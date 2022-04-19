/***************************************************************************
 * Copyright (c) 2014, 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ***************************************************************************/


#include <dlfcn.h>

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

#include "mct_controller.h"
#include "modules.h"
#include "camera_dbg.h"
#include "base_module.h"
#include "base_port.h"
#include "eztune.h"
#include "eztune_interface.h"
#include "eztune_module.h"
#include "pp_log.h"

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "mm-camera-eztune"

/* Tuning library specific data structures */
static void *(*create_tuning_server)( void *, uint32_t);
static void  (*delete_tuning_server)( void *);
static void  (*notify_tuning_server)( void *, uint32_t, void *);
static void *tuning_control_handle = NULL;
static void *tuning_preview_handle = NULL;
static void *tuning_lib = NULL;
static mct_module_t *module_eztune = NULL;

void eztune_server_config(mct_module_t *module)
{
    /* Avoid multiple instances. If an instance is already
     * configured do not create a new one
     */
    if (!module || module_eztune || tuning_lib) {
        return;
    }
    module_eztune = module;
}

/** eztune_server_start
 * Opens eztune server dll and starts the server.
 * Server runs in a separate thread, hence no need to call
 * this API from a separate thread context
 *
 **/
void eztune_server_start ()
{
    if(!module_eztune) {
        goto end;
    }

    tuning_lib = dlopen("libmmcamera_tuning.so", RTLD_NOW);

    if (tuning_lib == NULL) {
        // not always an error, the absence of the lib may indicate a production build
        PP_HIGH("Tuning lib not opened, eztune disabled: %s", dlerror());
        goto end;
    }

    *(void **) &(create_tuning_server) = dlsym(tuning_lib, "eztune_create_server");
    *(void **) &(delete_tuning_server) = dlsym(tuning_lib, "eztune_delete_server");
    *(void **) &(notify_tuning_server) = dlsym(tuning_lib, "eztune_notify_server");

    if (!create_tuning_server || !delete_tuning_server || !notify_tuning_server) {
        PP_ERR("Error obtaining symbols in tuning libs");
        goto end;
    }

    tuning_control_handle = create_tuning_server(module_eztune, EZTUNE_SERVER_CONTROL);
    tuning_preview_handle = create_tuning_server(module_eztune, EZTUNE_SERVER_PREVIEW);

end:
    return;
}

/** eztune_server_stop
 *    @module: mct_module_t* pointer to module
 *
 * Stops eztune server and closes associated dll
 *
 **/
void eztune_server_stop(mct_module_t *module)
{
    if (!module_eztune || module_eztune != module) {
        goto end;
    }

    module_eztune = NULL;

    if (tuning_control_handle)
        delete_tuning_server(tuning_control_handle);

    if(tuning_preview_handle)
        delete_tuning_server(tuning_preview_handle);

    if (tuning_lib)
        dlclose(tuning_lib);

    tuning_control_handle = NULL;
    tuning_preview_handle = NULL;
    tuning_lib = NULL;

end:
    return;
}

static int isEztuneEnabled()
{
    int eztune_enable = 0;
#ifdef _LE_CAMERA_
    eztune_enable = 1;
#else
    char                      prop[PROPERTY_VALUE_MAX];
    memset(prop, 0, sizeof(prop));
    property_get("persist.vendor.camera.eztune.enable", prop, "0");
    eztune_enable = atoi(prop);
#endif

    return eztune_enable;
}

/** eztune_check_server
 *
 * Check if server is running and ready. Do delayed
 * start of server if needed.
 *
 **/
static boolean eztune_server_check(mct_module_t *module)
{
    if(!isEztuneEnabled()) {
        PP_HIGH("EZTune: Server not starting as eztune is disabled");
        return FALSE;
    }

    // check if eztune has been initialized
    if (!module_eztune || module_eztune != module) {
        return FALSE;
    }

    // delay start to avoid connections before pipeline is running,
    // this first gets called with the SET_CHROMATIX event
    if(!tuning_control_handle && !tuning_preview_handle) {
        mct_pipeline_t *pipeline = eztune_get_pipeline();
        mct_port_t *port = eztune_get_port_by_type(CAM_STREAM_TYPE_PREVIEW);

        // do not start server if:
        // Not connected to preview/snapshot stream (eztune disabled in feature-mask)
        // YUV sensors detected (these are not tuned)
        if(!port || !pipeline ||
           pipeline->query_data.sensor_cap.sensor_format == FORMAT_YCBCR) {
            PP_HIGH("EZTune: server not starting as eztune is disabled or sensor is YUV");
        } else {
            eztune_server_start();
            if(!(tuning_control_handle && tuning_preview_handle)) {
                PP_ERR("EZTune: error starting server");
            }
        }
    }

    return tuning_control_handle && tuning_preview_handle;
}

static boolean eztune_validate_buff_index(void *data1, void *data2)
{
    mct_stream_map_buf_t *list_buf = (mct_stream_map_buf_t *)data1;
    uint32_t *buf_idx = (uint32_t *)data2;

    assert(list_buf && buf_idx);

    if (list_buf->buf_index == *buf_idx)
        return TRUE;
    else
        return FALSE;
}

//#define EZTUNE_FILEDUMP

#ifdef EZTUNE_FILEDUMP
static void eztune_dump_frame(uint8_t *buffer __unused, cam_frame_len_offset_t plane_info __unused)
{
    int32_t file_fd = open("/data/test_img.yuv", O_RDWR | O_CREAT, 0777);

    int i;
    void *data;
    uint32_t written_len = 0;

    for (i = 0; i < plane_info.num_planes; i++) {
        uint32_t index = plane_info.mp[i].offset;
        if (i > 0) {
            index += plane_info.mp[i - 1].len;
        }
        int j;
        for (j = 0; j <  plane_info.mp[i].height; j++) {
            data = (void *)((uint8_t *)buffer + index);
            written_len += write(file_fd, data,  plane_info.mp[i].width);
            index +=  plane_info.mp[i].stride;
        }
    }
    close(file_fd);
}
#endif

void eztune_notify_preview_frame(mct_module_t *module,
                                 mct_stream_t *stream,
                                 isp_buf_divert_t *p_buf_divert)
{
    uint8_t *buffer = NULL;
    mct_list_t *img_buf_list = NULL;
    mct_stream_map_buf_t *img_buf = NULL;

    if (!eztune_server_check(module)) {
        return;
    }

    if(p_buf_divert->native_buf) {
        buffer = p_buf_divert->vaddr;
    } else {
        img_buf_list = mct_list_find_custom(stream->streaminfo.img_buffer_list,
                                            &p_buf_divert->buffer.index, eztune_validate_buff_index);

        if (img_buf_list)
            img_buf = (mct_stream_map_buf_t *)img_buf_list->data;

        if (img_buf)
            buffer = (uint8_t *)img_buf->buf_planes[0].buf;
    }

#ifdef EZTUNE_FILEDUMP
    //for debug
    eztune_dump_frame(buffer, stream->streaminfo.buf_planes.plane_info);
    p_buf_divert->buffer_access |= CPU_HAS_READ;
#endif

    if (buffer) {
        mct_stream_info_t streaminfo;

        //make copy of stream info and override img_buffer_list with the
        //actual buffer pointer. Intent is to send stream info and buffer
        //pointer in one call to tuning server
        streaminfo = stream->streaminfo;
        streaminfo.img_buffer_list = (mct_list_t*)buffer;
        notify_tuning_server(tuning_preview_handle, EZTUNE_PREVIEW_NOTIFY, &streaminfo);
        p_buf_divert->buffer_access |= CPU_HAS_READ;
    }
}

void eztune_notify_snapshot_frame(mct_module_t *module, void *params)
{
    if (!eztune_server_check(module)) {
        return;
    }

    notify_tuning_server(tuning_preview_handle, EZTUNE_SNAPSHOT_NOTIFY, params);
}

void eztune_notify_metadata_frame(mct_module_t *module,
                                  metadata_buffer_t *metadata)
{
    if (!eztune_server_check(module)) {
        return;
    }

    notify_tuning_server(tuning_control_handle, EZTUNE_METADATA_NOTIFY, metadata);

    //we are notifying metadata to image port also as 3A Live will need metadata to fill 3A packet
    //this is the simplest way of achieving the goal, else one Image instance of Process and Adaptor
    //will have to pull metadata from Control instance of Process and Adaptor
    notify_tuning_server(tuning_preview_handle, EZTUNE_METADATA_NOTIFY, metadata);
}

void eztune_notify_streamon(mct_module_t *module, mct_event_t *event)
{
    if (!eztune_server_check(module)) {
        return;
    }

    mct_stream_t *stream = eztune_get_stream_by_id(event->identity);
    notify_tuning_server(tuning_control_handle, EZTUNE_STREAMON_NOTIFY, stream);
    notify_tuning_server(tuning_preview_handle, EZTUNE_STREAMON_NOTIFY, stream);
}

void eztune_notify_streamoff(mct_module_t *module, mct_event_t *event)
{
    if (!eztune_server_check(module)) {
        return;
    }

    mct_stream_t *stream = eztune_get_stream_by_id(event->identity);
    notify_tuning_server(tuning_control_handle, EZTUNE_STREAMOFF_NOTIFY, stream);
    notify_tuning_server(tuning_preview_handle, EZTUNE_STREAMOFF_NOTIFY, stream);
}

void eztune_notify_set_chromatix_ptr(mct_module_t *module, mct_event_t *event)
{
    if (!eztune_server_check(module)) {
        return;
    }

    modulesChromatix_t *chromatix_param = (modulesChromatix_t *)
        event->u.module_event.module_event_data;
    notify_tuning_server(tuning_control_handle, EZTUNE_SET_CHROMATIX_NOTIFY, chromatix_param);
    notify_tuning_server(tuning_preview_handle, EZTUNE_SET_CHROMATIX_NOTIFY, chromatix_param);
}

static boolean find_port_with_identity_find_func(void * data, void * userdata)
{
    mct_port_t *port = (mct_port_t *)data;
    uint32_t identity = *(uint32_t *)userdata;

    if (base_port_get_stream_data(port, identity) == NULL) {
        return false;
    }
    return true;
}

mct_pipeline_t *eztune_get_pipeline()
{
    mct_module_t *module = NULL;
    mct_stream_t *stream = NULL;
    mct_pipeline_t *pipeline = NULL;

    if (!module_eztune || !MCT_OBJECT_PRIVATE(module_eztune)) {
        PP_ERR("module is NULL, pipeline not open %p", module_eztune);
        return NULL;
    }

    // get the parent PPROC module, the local module is not actually
    // connected to the pipeline or a "real" stream
    module = ((base_module_ctrl_t*)MCT_OBJECT_PRIVATE(module_eztune))->parent_mod;
    if (!module || !MCT_MODULE_PARENT(module)) {
        PP_ERR("module/stream is NULL, pipeline not yet initialized %p", module);
        return NULL;
    }
    stream = MCT_STREAM_CAST((MCT_MODULE_PARENT(module))->data);
    if(!stream || !MCT_STREAM_PARENT(stream)) {
        PP_ERR("stream/pipeline is NULL, pipeline not yet initialized %p", stream);
        return NULL;
    }
    pipeline = MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data);
    if(!pipeline) {
        PP_ERR("pipeline is NULL, pipeline not yet initialized %p", pipeline);
        return NULL;
    }

    return pipeline;
}

mct_stream_t *eztune_get_stream_by_id(uint32_t id)
{
    mct_pipeline_t *pipeline = NULL;
    mct_stream_t *stream = NULL;
    mct_pipeline_get_stream_info_t info;

    pipeline = eztune_get_pipeline();
    if(!pipeline) {
        return NULL;
    }

    info.check_type = CHECK_INDEX;
    info.stream_index = id & 0xFFFF;

    stream = mct_pipeline_get_stream(pipeline, &info);
    return stream;
}

mct_stream_t *eztune_get_stream_by_type(cam_stream_type_t stream_type)
{
    mct_pipeline_t *pipeline = NULL;
    mct_stream_t *stream = NULL;
    mct_pipeline_get_stream_info_t info;

    pipeline = eztune_get_pipeline();
    if(!pipeline) {
        return NULL;
    }

    info.check_type = CHECK_TYPE;
    info.stream_type = stream_type;

    stream = mct_pipeline_get_stream(pipeline, &info);
    return stream;
}

mct_port_t *eztune_get_port_by_type(cam_stream_type_t stream_type)
{
    mct_pipeline_t *pipeline = NULL;
    mct_stream_t *stream = NULL;
    mct_port_t *port = NULL;
    mct_list_t *list = NULL;
    uint32_t id;

    pipeline = eztune_get_pipeline();
    stream = eztune_get_stream_by_type(stream_type);

    if(!pipeline || !stream) {
        return NULL;
    }
    id = pack_identity(MCT_PIPELINE_SESSION(pipeline), stream->streamid);
    list = mct_list_find_custom(MCT_MODULE_SINKPORTS(module_eztune),
        &id, find_port_with_identity_find_func);

    if (list) {
        port = (mct_port_t *)(list->data);
    }
    return port;
}

boolean eztune_post_msg_to_bus(mct_bus_msg_type_t type, int32_t size, void *msg)
{
    mct_pipeline_t *pipeline = NULL;
    mct_bus_t *bus = NULL;
    mct_bus_msg_t bus_msg;

    pipeline = eztune_get_pipeline();
    if(!pipeline) {
      return FALSE;
    }

    memset(&bus_msg, 0x0, sizeof(mct_bus_msg_t));
    bus = pipeline->bus;
    bus_msg.type = type;
    bus_msg.size = size;
    bus_msg.msg = msg;
    bus_msg.sessionid = bus->session_id;
    bus->post_msg_to_bus(bus, &bus_msg);

    return TRUE;
}

boolean eztune_send_event(mct_event_module_type_t type, uint32_t identity, void *data)
{
  boolean rv = FALSE;
  mct_list_t *templist = NULL;
  mct_port_t *port = NULL;
  mct_event_t ezt_event;

  ezt_event.type = MCT_EVENT_MODULE_EVENT;
  ezt_event.identity = identity;
  ezt_event.direction = MCT_EVENT_UPSTREAM;
  ezt_event.u.module_event.type = type;
  ezt_event.u.module_event.module_event_data = data;

  templist = mct_list_find_custom(MCT_MODULE_SINKPORTS(module_eztune),
                                  &identity, find_port_with_identity_find_func);
  if (templist) {
    port = (mct_port_t *)(templist->data);
    rv = port->event_func(port, &ezt_event);
  }

  return rv;
}

