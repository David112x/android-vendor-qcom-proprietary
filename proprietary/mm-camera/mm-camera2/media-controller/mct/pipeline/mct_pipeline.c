/* mct_pipeline.c
 *
 *Copyright (c) 2012-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "cam_ker_headers.h"
#include "cam_intf.h"
#include "mct_controller.h"
#include "camera_dbg.h"
#include <linux/videodev2.h>
#include <media/msmb_camera.h>
#include <math.h>
#include <sys/sysinfo.h>
#include "mct_profiler.h"
#include <cutils/properties.h>
#include "server_debug.h"
#include "supermct.h"

#define MCT_PIPELINE_STOP_SESSION_TIMEOUT 2
#define MCT_SORT_FPS_TABLE 0
#define MCT_PIPELINE_SLEEP_DURATION 5

#define FIXED_PIPELINE_DELAY 0
static cam_dimension_t default_preview_sizes[] = {
#ifndef CAMERA_FEATURE_LOW_RAM
  { 2048, 1536}, // QXGA
  { 2048, 1080}, // 4k/2
#endif
  { 1920, 1440},
  { 1920, 1080}, // 1080p
  { 2560, 800},  // 1280x800 stereo
  { 1440, 1080}, // Wide HD
  { 1280, 960},
  { 1280, 720},  // 720P, reserved
  { 1280, 480},  // VGA stereo
  { 1280, 400},  // 640x400 stereo
  { 864, 480},   // FWVGA
  { 800, 480},   // WVGA
  { 768, 432},
  { 720, 480},
  { 640, 640},
  { 640, 480},   // VGA
  { 480, 640},   // VGA portrait
  { 640, 360},
  { 576, 432},
  { 480, 360},   // HVGA for HFR support
  { 480, 320},   // HVGA
  { 384, 288},
  { 352, 288},   // CIF
  { 320, 240},   // QVGA
  { 240, 320},   // QVGA portrait
  { 240, 160},   // SQVGA
  { 176, 144},   // QCIF
  { 144, 176}   // QCIF portrait
//  { 160, 120}
};

static cam_dimension_t default_picture_sizes[] = {
#ifndef CAMERA_FEATURE_LOW_RAM
  { 5656, 4242}, // 24 MP
  { 5344, 4008}, // 21 MP
  { 5312, 2988}, // 16 MP with 16:9 aspect ratio.
  { 4608, 3456}, // 16MP
  { 4160, 3120}, // 13M
  { 4000, 3000}, // 12MP
#endif
#ifdef CAMERA_FEATURE_LOW_RAM
  { 3072, 3072},
#endif
  { 3840, 2160},
  { 3264, 2448}, // 8MP added for 8909
  { 3200, 2400}, // 8MP
  { 2976, 2976},
#ifdef _LE_CAMERA_
  { 2704, 2028}, //2.7K (4:3)
  { 2704, 1520}, // 2.7K
#endif
  { 2592, 1944}, // 5MP
  { 2688, 1512}, // 4MP wide
  { 2048, 1536}, // 3MP QXGA
#ifdef _LE_CAMERA_
  { 1920, 1440}, //1440p
#endif
  { 1920, 1080}, // HD1080
  { 2560, 800},  // 1280x800 stereo
  { 1600, 1200}, // 2MP UXGA
  { 1440, 1080}, // Wide HD
  { 1280, 960},  // 1.3MP UXGA
  { 1280, 768},  // WXGA
  { 1280, 720},  // HD720
  { 1200, 1200},
  { 1280, 480},  // VGA stereo
  { 1280, 400},  // 640x400 stereo
  { 1024, 768},  // 1MP XGA
  { 800, 600},   // SVGA
#ifdef _LE_CAMERA_
  { 848, 480},   //WVGA Wide
  { 858, 480},   //WVGA Wide
#endif
  { 864, 480},   // FWVGA
  { 800, 480},   // WVGA
  { 720, 480},   // 480p
  { 640, 640},
  { 640, 480},   // VGA
  { 640, 360},
  { 480, 640},   // Portrait VGA for VT App
  { 480, 360},   // HVGA for HFR support
  { 480, 320},   // HVGA
#ifdef _LE_CAMERA_
  { 432, 240},
#endif
  { 352, 288},   // CIF
  { 320, 240},   // QVGA
  { 240, 320},   // Portrait QVGA for VT App
  { 176, 144}   // QCIF
  //{ 160, 120}
};

static cam_dimension_t default_liveshot_sizes[] = {
#ifndef CAMERA_FEATURE_LOW_RAM
  { 5312, 2988}, // 16 MP with 16:9 aspect ratio.
  { 4128, 3096}, // 4:3
  { 4128, 2322}, // 16:9
  { 4000, 3000}, // 12MP
  /* This resolution is required for 4K recording
   * as we match video/liveshot aspect ratios*/
  { 4096, 2160}, // 8MP for 4kDCI
#endif
  { 3840, 2160}, // 8MP for 4KUHD
  { 3264, 2448}, // 8MP added for 8909
  { 3200, 2400}, // 8MP
#ifdef _LE_CAMERA_
  { 2704, 2028}, //2.7K(4:3)
  { 2704, 1520}, // 2.7K
#endif
  { 2592, 1944}, // 5MP
  { 2048, 1536}, // 3MP QXGA
  { 2048, 1080}, // 4k/2
#ifdef _LE_CAMERA_
  { 1920, 1440}, //1440p
#endif
  { 1920, 1080}, // HD1080
  { 2560, 800},  // 1280x800 stereo
  { 1600, 1200}, // 2MP UXGA
  { 1440, 1080}, // Wide HD
  { 1280, 960},  // 1.3MP UXGA
  { 1280, 768},  // WXGA
  { 1280, 720},  // HD720
  { 1280, 480},  // VGA stereo
  { 1280, 400},  // 640x400 stereo
  { 1024, 768},  // 1MP XGA
  { 800, 600},   // SVGA
  { 864, 480},   // FWVGA
#ifdef _LE_CAMERA_
  { 848, 480},   //WVGA Wide
  { 858, 480},   //WVGA Wide
#endif
  { 800, 480},   // WVGA
  { 720, 480},   // 480p
  { 640, 480},   // VGA
  { 640, 360},
#ifdef _LE_CAMERA_
  { 432, 240},
#endif
  { 352, 288},   // CIF
  { 320, 240},   // QVGA
  { 176, 144}   // QCIF
  //{ 160, 120}
};

static cam_dimension_t default_video_sizes[] = {
  { 4096, 2160},// true 4K
  { 3840, 2160},// 4K
#ifdef _LE_CAMERA_
  { 2704, 2028}, //2.7K ( 4:3)
  { 2704, 1520},// 2.7K
#endif
#ifdef _LE_CAMERA_
  { 1920, 1440}, //1440p
#endif
  { 1920, 1080},// 1080p
  { 2560, 800}, // 1280x800 stereo
  { 1280, 960},
  { 1280, 720}, // 720p
  { 1280, 480},  // VGA stereo
  { 1280, 400},  // 640x400 stereo
  { 864, 480},  // FWVGA
#ifdef _LE_CAMERA_
  { 848, 480},   //WVGA Wide
  { 858, 480},   //WVGA Wide
#endif
  { 800, 480},  // WVGA
  { 720, 480},  // 480p
  { 640, 480},  // VGA
  { 480, 640},  // VGA portrait
  { 640, 360},
  { 480, 360},  // HVGA for HFR support
  { 480, 320},  // HVGA
#ifdef _LE_CAMERA_
  { 432, 240},
#endif
  { 352, 288},  // CIF
  { 320, 240},  // QVGA
  { 240, 320},  // QVGA portrait
  { 176, 144},  // QCIF
  { 144, 176}  // QCIF portrait
  //{ 160, 120}
};

static cam_dimension_t default_hfr_video_sizes[] = {
  { 3840, 2160},// 4K
#ifdef _LE_CAMERA_
  { 2704, 2028}, //2.7K ( 4:3)
  { 2704, 1520},// 2.7K
#endif
#ifdef _LE_CAMERA_
  { 1920, 1440}, //1440p
#endif
  { 1920, 1080},// 1080p
  { 2560, 800},  // 1280x800 stereo
  { 1280, 720}, // 720p
  { 1280, 480},  // VGA stereo
  { 1280, 400},  // 640x400 stereo
  { 800, 480},  // WVGA
  { 720, 480},  // 480p
  { 640, 480},  // VGA
  { 480, 360},  // HVGA for HFR support
  { 480, 320},  // HVGA
  { 320, 240},  // QVGA
};

/* stall durations for each of the picture sizes
 * for front sensor.
 * maintain same order as default_picture_sizes table
 */
static int64_t default_jpeg_stall_durations_front[] = {
#ifndef CAMERA_FEATURE_LOW_RAM
  0.000 * SEC_TO_NS_FACTOR,  //24 MP
  0.000 * SEC_TO_NS_FACTOR,  //21 MP
  0.000 * SEC_TO_NS_FACTOR,  //16 MP with 16:9 aspect ratio.
  0.000 * SEC_TO_NS_FACTOR,  //16MP
  0.082 * SEC_TO_NS_FACTOR,  //13M
  0.080 * SEC_TO_NS_FACTOR,  //12MP
#endif
#ifdef CAMERA_FEATURE_LOW_RAM
  0.067 * SEC_TO_NS_FACTOR,
#endif
  0.076 * SEC_TO_NS_FACTOR,
  0.072 * SEC_TO_NS_FACTOR,  //8MP added for 8909
  0.070 * SEC_TO_NS_FACTOR,  //8MP
  0.076 * SEC_TO_NS_FACTOR,
#ifdef _LE_CAMERA_
  0.060 * SEC_TO_NS_FACTOR,  //2.7K ( 4:3)
  0.060 * SEC_TO_NS_FACTOR,  //2.7K
#endif
  0.057 * SEC_TO_NS_FACTOR,  //5MP
  0.053 * SEC_TO_NS_FACTOR,  //4MP wide
  0.050 * SEC_TO_NS_FACTOR,  //3MP QXGA
#ifdef _LE_CAMERA_
  0.049 * SEC_TO_NS_FACTOR,  //1440p
#endif
  0.049 * SEC_TO_NS_FACTOR,  //HD1080
  0.049 * SEC_TO_NS_FACTOR,  //1280x800 Stereo
  0.048 * SEC_TO_NS_FACTOR,  //2MP UXGA
  0.047 * SEC_TO_NS_FACTOR,  //Wide HD
  0.046 * SEC_TO_NS_FACTOR,  //1.3MP UXGA
  0.045 * SEC_TO_NS_FACTOR,  //WXGA
  0.045 * SEC_TO_NS_FACTOR,  //HD720
  0.045 * SEC_TO_NS_FACTOR,  //1200*1200
  0.045 * SEC_TO_NS_FACTOR,  //VGA Stereo
  0.045 * SEC_TO_NS_FACTOR,  //640x400 Stereo
  0.045 * SEC_TO_NS_FACTOR,  //1MP XGA
  0.045 * SEC_TO_NS_FACTOR,  //SVGA
  0.045 * SEC_TO_NS_FACTOR,  //FWVGA
#ifdef _LE_CAMERA_
  0.045 * SEC_TO_NS_FACTOR,  //WVGA
  0.045 * SEC_TO_NS_FACTOR,  //WVGA
#endif
  0.044 * SEC_TO_NS_FACTOR,  //WVGA
  0.044 * SEC_TO_NS_FACTOR,  //480p
  0.044 * SEC_TO_NS_FACTOR,  //VGA
  0.044 * SEC_TO_NS_FACTOR,
  0.043 * SEC_TO_NS_FACTOR,  //Portrait VGA for VT App
  0.043 * SEC_TO_NS_FACTOR,  //HVGA for HFR support
  0.042 * SEC_TO_NS_FACTOR,  //HVGA
#ifdef _LE_CAMERA_
  0.041 * SEC_TO_NS_FACTOR,
#endif
  0.040 * SEC_TO_NS_FACTOR,  //CIF
  0.040 * SEC_TO_NS_FACTOR,  //QVGA
  0.035 * SEC_TO_NS_FACTOR,  //Portrait QVGA for VT App
  0.030 * SEC_TO_NS_FACTOR,  //QCIF
  0.025 * SEC_TO_NS_FACTOR,
  0.018 * SEC_TO_NS_FACTOR,  // Portrait QCIF for VT App
};

/* stall durations for each of the picture sizes for
 * back sensor.
 * maintain same order as default_picture_sizes table
 */
static int64_t default_jpeg_stall_durations_back[] = {
#ifndef CAMERA_FEATURE_LOW_RAM
  0.096 * SEC_TO_NS_FACTOR,  //24 MP
  0.092 * SEC_TO_NS_FACTOR,  //21 MP
  0.086 * SEC_TO_NS_FACTOR,  //16 MP with 16:9 aspect ratio.
  0.085 * SEC_TO_NS_FACTOR,  //16MP
  0.082 * SEC_TO_NS_FACTOR,  //13M
  0.080 * SEC_TO_NS_FACTOR,  //12MP
#endif
#ifdef CAMERA_FEATURE_LOW_RAM
  0.067 * SEC_TO_NS_FACTOR,
#endif
  0.076 * SEC_TO_NS_FACTOR,
  0.072 * SEC_TO_NS_FACTOR,  //8MP added for 8909
  0.070 * SEC_TO_NS_FACTOR,  //8MP
  0.076 * SEC_TO_NS_FACTOR,
#ifdef _LE_CAMERA_
  0.060 * SEC_TO_NS_FACTOR,  //2.7K ( 4:3)
  0.060 * SEC_TO_NS_FACTOR,  //2.7K
#endif
  0.057 * SEC_TO_NS_FACTOR,  //5MP
  0.053 * SEC_TO_NS_FACTOR,  //4MP wide
  0.050 * SEC_TO_NS_FACTOR,  //3MP QXGA
#ifdef _LE_CAMERA_
  0.049 * SEC_TO_NS_FACTOR,  //1440p
#endif
  0.049 * SEC_TO_NS_FACTOR,  //HD1080
  0.049 * SEC_TO_NS_FACTOR,  //1280x800 Stereo
  0.048 * SEC_TO_NS_FACTOR,  //2MP UXGA
  0.047 * SEC_TO_NS_FACTOR,  //Wide HD
  0.046 * SEC_TO_NS_FACTOR,  //1.3MP UXGA
  0.045 * SEC_TO_NS_FACTOR,  //WXGA
  0.045 * SEC_TO_NS_FACTOR,  //HD720
  0.045 * SEC_TO_NS_FACTOR,  //1200*1200
  0.045 * SEC_TO_NS_FACTOR,  //VGA Stereo
  0.045 * SEC_TO_NS_FACTOR,  //640x400 Stereo
  0.045 * SEC_TO_NS_FACTOR,  //1MP XGA
  0.045 * SEC_TO_NS_FACTOR,  //SVGA
  0.045 * SEC_TO_NS_FACTOR,  //FWVGA
#ifdef _LE_CAMERA_
  0.045 * SEC_TO_NS_FACTOR,  //WVGA
  0.045 * SEC_TO_NS_FACTOR,  //WVGA
#endif
  0.044 * SEC_TO_NS_FACTOR,  //WVGA
  0.044 * SEC_TO_NS_FACTOR,  //480p
  0.044 * SEC_TO_NS_FACTOR,  //VGA
  0.044 * SEC_TO_NS_FACTOR,
  0.043 * SEC_TO_NS_FACTOR,  //Portrait VGA for VT App
  0.043 * SEC_TO_NS_FACTOR,  //HVGA for HFR support
  0.042 * SEC_TO_NS_FACTOR,  //HVGA
#ifdef _LE_CAMERA_
  0.041 * SEC_TO_NS_FACTOR,
#endif
  0.040 * SEC_TO_NS_FACTOR,  //CIF
  0.040 * SEC_TO_NS_FACTOR,  //QVGA
  0.035 * SEC_TO_NS_FACTOR,  //Portrait QVGA for VT App
  0.030 * SEC_TO_NS_FACTOR,  //QCIF
  0.025 * SEC_TO_NS_FACTOR,
  0.018 * SEC_TO_NS_FACTOR,  // Portrait QCIF for VT App
};

/*This should always be sorted from small to large
 * (first by maximum fps and then minimum fps): */
static cam_fps_range_t default_fps_ranges[] = {
  { 15.0, 15.0, 15.0, 15.0},
  { 20.0, 20.0, 20.0, 20.0},
  { 24.0, 24.0, 24.0, 24.0},
  { 30.0, 30.0, 30.0, 30.0},
  { 60.0, 60.0, 60.0, 60.0},
};

/** mct_pipeline_check_stream
 *    @d1: mct_stream_t* pointer to the streanm being checked
 *    @d2: mct_pipeline_get_stream_info_t* pointer info to check against
 *
 *  Check if the stream matches stream index or stream type.
 *
 *  Return: TRUE if stream matches.
 **/
static boolean mct_pipeline_check_stream(void *d1, void *d2)
{
  mct_stream_t                   *stream = (mct_stream_t *)d1;
  mct_pipeline_get_stream_info_t *info   =
    (mct_pipeline_get_stream_info_t *)d2;

  if (!info) {
    CLOGE(CAM_MCT_MODULE, "Stream Info null");
    return FALSE;
  }
  if (!stream) {
    CLOGE(CAM_MCT_MODULE, "Stream null");
    return FALSE;
  }

  if (info->check_type == CHECK_INDEX) {
    return (stream->streamid
      == info->stream_index ? TRUE : FALSE);
  } else if (info->check_type == CHECK_TYPE) {
      return (stream->streaminfo.stream_type
        == info->stream_type ? TRUE : FALSE);
  } else if (info->check_type == CHECK_SESSION) {
    return (((stream->streaminfo.identity & 0xFFFF0000) >> 16)
      == (unsigned int)info->session_index ? TRUE : FALSE);
  } else if (info->check_type == CHECK_FEATURE_MASK) {
   return ((stream->streaminfo.stream_type == info->stream_type ? TRUE : FALSE)
     && (stream->streaminfo.reprocess_config.pp_feature_config.feature_mask &
     info->feature_mask));
  }

  return FALSE;
}

/** mct_pipeline_get_stream
 *    @pipeline: mct_pipeline_t object
 *    @get_info: stream information to match
 *
 *  Retrieve the stream which matches stream information in get_info.
 *
 *  Reture stream object if the stream is found.
 **/
mct_stream_t* mct_pipeline_get_stream(mct_pipeline_t *pipeline,
  mct_pipeline_get_stream_info_t *get_info)
{
  mct_list_t *find_list = NULL;
  if (!MCT_PIPELINE_CHILDREN(pipeline)) {
    CLOGD(CAM_MCT_MODULE, "No children of pipeline present");
    return NULL;
  }

  MCT_OBJECT_LOCK(pipeline);
  find_list = mct_list_find_custom(MCT_PIPELINE_CHILDREN(pipeline),
    get_info, mct_pipeline_check_stream);
  MCT_OBJECT_UNLOCK(pipeline);

  if (!find_list) {
    CLOGD(CAM_MCT_MODULE, "Stream not found in the list");
    return NULL;
  }

  return MCT_STREAM_CAST(find_list->data);
}

/** mct_pipeline_find_stream:
 *    @module:
 *    @identity:
 *
 **/
mct_stream_t* mct_pipeline_find_stream
  (mct_module_t *module, unsigned int session_id)
{
  mct_list_t     *find_list = NULL;
  mct_pipeline_get_stream_info_t info;

  info.check_type    = CHECK_SESSION;
  info.session_index = session_id;

  MCT_MODULE_LOCK(module);
  find_list = mct_list_find_custom(MCT_MODULE_PARENT(module),
    (void *)&info, mct_pipeline_check_stream);
  MCT_MODULE_UNLOCK(module);

  if (!find_list)
    return NULL;

  return MCT_STREAM_CAST(find_list->data);
}

/** mct_pipeline_find_stream_from_stream_id:
 *    @module:
 *    @identity:
 *
 **/
mct_stream_t* mct_pipeline_find_stream_from_stream_id
  (mct_pipeline_t *pipeline, uint32_t stream_id)
{
  mct_stream_t     *stream = NULL;
  mct_pipeline_get_stream_info_t info;

  info.check_type    = CHECK_INDEX;
  info.stream_index  = stream_id;

  stream = mct_pipeline_get_stream(pipeline, &info);
  if (!stream) {
    CLOGE(CAM_MCT_MODULE, "Couldn't find stream");
    return NULL;
  }

  return stream;
}

/** mct_pipeline_find_buf:
 *    @stream: mct_stream_t object which to receive the event
 *    @event:  mct_event_t object to send to this stream
 *
 *  Used for matching metadata buffer in to stream list.
 *
 *  Return TRUE on success
 **/
static boolean mct_pipeline_find_buf(void *data, void *user_data)
{

  mct_stream_map_buf_t *map_buf = data;
  uint32_t *buf_index = user_data;

  if (CAM_MAPPING_BUF_TYPE_STREAM_BUF != map_buf->buf_type) {
    return FALSE;
  }

  return (map_buf->buf_index == *buf_index);
}

void *mct_pipeline_get_buffer_ptr(mct_pipeline_t *pipeline, uint32_t buf_idx,
  uint32_t stream_id)
{
  mct_stream_map_buf_t *current_buf;

  current_buf = mct_pipeline_get_buffer(pipeline, buf_idx, stream_id);

  return current_buf ? current_buf->buf_planes[0].buf : NULL;
}

/** mct_pipeline_get_buffer:
 *
 *  Used for matching frame buffer in to stream list.
 **/
mct_stream_map_buf_t *mct_pipeline_get_buffer(mct_pipeline_t *pipeline,
  uint32_t buf_idx, uint32_t stream_id)
{
  mct_list_t *current_buf_holder;
  mct_stream_map_buf_t *current_buf;
  mct_stream_t *stream = NULL;
  mct_pipeline_get_stream_info_t info;

  info.check_type   = CHECK_INDEX;
  info.stream_index = stream_id;
  stream = mct_pipeline_get_stream(pipeline, &info);
  if (!stream) {
    CLOGE(CAM_MCT_MODULE, "Couldn't find stream id %x", stream_id);
    return NULL;
  }

  pthread_mutex_lock(&stream->buffers.lock_img_buf);
  current_buf_holder = mct_list_find_custom(
    stream->buffers.img_buf,
    &buf_idx, mct_pipeline_find_buf);
  pthread_mutex_unlock(&stream->buffers.lock_img_buf);
  if (!current_buf_holder) {
    CLOGE(CAM_MCT_MODULE, "current_buf_holder is null");
    return NULL;
  }

  current_buf = current_buf_holder->data;
  return current_buf;
}

/** mct_pipeline_query_modules:
 *    @
 *    @
 *
 **/
static boolean mct_pipeline_query_modules(void *data, void *user_data)
{
  boolean ret = TRUE;
  mct_pipeline_t *pipeline = (mct_pipeline_t *)user_data;
  mct_module_t *module   = (mct_module_t *)data;

  if (!pipeline || !module)
    return FALSE;
  if (module->query_mod) {
    ret = module->query_mod(module, &pipeline->query_data, pipeline->session);
    if(FALSE == ret)
      CLOGE(CAM_MCT_MODULE, "Query mod failed on %s",
         MCT_MODULE_NAME(module));
  }
  return ret;
}

/** Name: mct_pipeline_set_session_data
 *
 *  Arguments/Fields:
 *    @user_data: Pointer to pipeline
 *    @frame_id: Pointer to a module in the linked list of modules
 *
 *  Return: TRUE on Success, FALSE on failure
 *
 *  Description:
 *  Sends session data (various frame delays) to all modules to
 *  synchronize their ouputs.
 *  This information is sent on the session-stream which has already
 *  established linking across all modules in the imaging pipe.
 **/

static boolean mct_pipeline_set_session_data(void *data, void *user_data)
{
  boolean rc = TRUE;
  mct_pipeline_t *pipeline = (mct_pipeline_t *)user_data;
  mct_module_t *module   = (mct_module_t *)data;

  if (!pipeline || !module)
    return FALSE;
  if (module->set_session_data) {
    rc = module->set_session_data(
      module, &pipeline->session_data, pipeline->session);
  }

  return rc;
}

/** Name: mct_pipeline_get_session_data
 *
 *  Arguments/Fields:
 *    @user_data: Pointer to pipeline
 *    @frame_id: Pointer to a module in the linked list of modules
 *
 *  Return: TRUE on Success, FALSE on failure
 *
 *  Description:
 *  Queries session data from all modules in pipeline.
 *
 **/
boolean mct_pipeline_get_session_data(void *data, void *user_data)
{
  mct_pipeline_t *pipeline = (mct_pipeline_t *)user_data;
  mct_module_t *module = (mct_module_t *)data;

  if (!pipeline || !module) {
    return FALSE;
  }
  if (module->get_session_data) {
    module->get_session_data(
    module, &pipeline->session_data, pipeline->session);
  }
  return TRUE;
}

/** Name: mct_pipeline_lookup_session_data
 *
 *  Arguments/Fields:
 *    @module: Pointer to module
 *    @session_id: Session ID
 *
 *  Return: Pointer to pipeline->session_data
 *
 *  Description:
 *  MCT API for modules to look up session data.
 *  Executes in the context of the caller.
 *
 **/
mct_pipeline_session_data_t const* mct_pipeline_lookup_session_data (
  mct_module_t *module, uint32_t sessionid)
{
  mct_stream_t *stream = NULL;
  mct_pipeline_get_stream_info_t info;
  mct_pipeline_t *pipeline = NULL;

  if (!module) {
    CLOGE(CAM_MCT_MODULE, "Invalid module pointer");
    return NULL;
  }

  stream = mct_pipeline_find_stream(module, sessionid);
  if (!stream) {
    CLOGW(CAM_MCT_MODULE, "Couldn't find stream for sessionid %d",
      sessionid);
    return NULL;
  }

  if (MCT_STREAM_PARENT(stream))
    pipeline = MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data);
  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Not valid pipeline for this stream 0x%x",
      stream->streaminfo.identity);
    return NULL;
  }

  return &(pipeline->session_data);
}

/** mct_pipeline_delete_stream:
 *    @
 *    @
 *
 **/
static boolean mct_pipeline_delete_stream(void *data, void *user_data __unused)
{
  mct_stream_t *stream = (mct_stream_t *)data;

  if (!stream) {
    return FALSE;
  }

  return mct_stream_destroy(stream);
}

/** mct_pipeline_add_stream
 *
 *    @ pipeline: the current pipeline; equivalent to "this"
 *    @ streamid: the stream id for the stream to be added
 *
 *  Return: TRUE on success, FALSE on failure
 *
 *  This function creates a new stream with the stream id
 *  passed in and adds it to the pipleine's list of children.
 *
 **/
static boolean mct_pipeline_add_stream(mct_pipeline_t *pipeline,
  uint32_t stream_id)
{
  mct_stream_t *stream = NULL;
  if (!pipeline)
      return FALSE;

  stream = mct_stream_new(stream_id);
  if (!stream)
      goto stream_failed;
  /* set pipeline as this stream's parent */
  if (!mct_object_set_parent(MCT_OBJECT_CAST(stream),
        MCT_OBJECT_CAST(pipeline))) {
    CLOGE(CAM_MCT_MODULE, "Set parent failed for stream id %d", stream_id);
    goto set_parent_failed;
  }

  return TRUE;

set_parent_failed:
  mct_stream_destroy(stream);
stream_failed:
  return FALSE;
}

/** mct_pipeline_remove_stream
 *
 *    @ pipeline: the current pipeline; equivalent to "this"
 *    @ stream: the stream to be removed
 *
 *  Return: TRUE on success, FALSE on failure
 *
 *  This function destroys the stream matching the stream id
 *  passed in and removes it from the pipleine's list of children.
 *
 **/
static boolean mct_pipeline_remove_stream(mct_pipeline_t *pipeline,
  mct_stream_t *stream)
{
  boolean ret = FALSE;
  if (!pipeline) {
    return ret;
  }
  if (MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data) != pipeline) {
    CLOGE(CAM_MCT_MODULE, "Stream 0x%x doesn't belong to pipeline %p",
       stream->streaminfo.identity, pipeline);
    return ret;
  }

  ret = mct_stream_destroy(stream);
  return ret;
}

/** mct_pipeline_send_event
 *    @pipeline:
 *    @stream_id:
 *    @event:
 *
 *  Return: TRUE on success, FALSE on failure
 **/
static boolean mct_pipeline_send_event(mct_pipeline_t *pipeline,
  uint32_t stream_id, mct_event_t *event)
{
  boolean ret = TRUE;
  mct_stream_t *stream = NULL;
  mct_pipeline_get_stream_info_t info;

  if (!pipeline || !event)
    return FALSE;

  info.check_type   = CHECK_INDEX;
  info.stream_index = stream_id;

  stream = mct_pipeline_get_stream(pipeline, &info);
  if (!stream) {
    CLOGE(CAM_MCT_MODULE, "Couldn't find stream");
    return FALSE;
  }

  ret = stream->send_event(stream, event);
  return ret;
}

/** mct_pipeline_set_bus:
 *    @
 *    @
 *
 **/
boolean mct_pipeline_set_bus(mct_pipeline_t *pipeline, mct_bus_t *bus __unused)
{
  if (!pipeline)
    return FALSE;
  /* TODO */
  return TRUE;
}

/** mct_pipeline_get_bus:
 *    @pipeline:
 *
 **/
static mct_bus_t* mct_pipeline_get_bus(mct_pipeline_t *pipeline)
{
  if (!pipeline)
    return NULL;
  /* TODO */
  return pipeline->bus;
}

/** mct_pipeline_map_parm:
 *    @pipeline:
 *
 **/
static boolean mct_pipeline_map_parm(mct_pipeline_t *pipeline)
{
  boolean ret = FALSE;
  mct_stream_t  *stream = NULL;
  mct_pipeline_get_stream_info_t get_info;

  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Pipeline not present");
    return ret;
  }
  get_info.check_type   = CHECK_INDEX;
  get_info.stream_index = MCT_SESSION_STREAM_ID;
  if ((stream = mct_pipeline_get_stream(pipeline, &get_info))) {
    if (stream->map_parm) {
      ret = stream->map_parm(stream);
    }
    else {
      CLOGE(CAM_MCT_MODULE, "Map parm method not present");
      ret = FALSE;
    }
  }
  else {
    CLOGE(CAM_MCT_MODULE, "Cannot find session stream");
    return ret;
  }
  return ret;
}

/** mct_pipeline_map_parm:
 *    @pipeline:
 *
 **/
static boolean mct_pipeline_unmap_parm(mct_pipeline_t *pipeline)
{
  boolean ret = FALSE;
  mct_stream_t  *stream = NULL;
  mct_pipeline_get_stream_info_t get_info;

  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Pipeline not present");
    return ret;
  }

  get_info.check_type   = CHECK_INDEX;
  get_info.stream_index = MCT_SESSION_STREAM_ID;

  if ((stream = mct_pipeline_get_stream(pipeline, &get_info))) {
    if (stream->unmap_parm) {
      ret = stream->unmap_parm(stream);
    }
    else {
      CLOGE(CAM_MCT_MODULE, "Unmap parm method not present");
      ret = FALSE;
    }
  }
  else {
    CLOGE(CAM_MCT_MODULE, "Cannot find session stream");
    return ret;
  }
  return ret;
}

/** mct_pipeline_map_buf:
 *    @
 *    @
 *
 **/
static boolean mct_pipeline_map_buf(void *message, mct_pipeline_t *pipeline)
{
  boolean ret = FALSE;
  mct_serv_ds_msg_bundle_t *msg = (mct_serv_ds_msg_bundle_t *)message;
  mct_stream_t  *stream;
  uint32_t i = 0;

  if (!msg || !pipeline ||
      (msg->session != MCT_PIPELINE_SESSION(pipeline))) {
    CLOGE(CAM_MCT_MODULE, "Pipeline sanity failed msg =%p pipeline =%p",
      msg, pipeline);
    return ret;
  }
  for (i = 0; i < msg->num_bufs && i < CAM_MAX_NUM_BUFS_PER_STREAM; i++) {
    switch (msg->ds_buf[i].buf_type) {
    case CAM_MAPPING_BUF_TYPE_PARM_BUF: {
#ifndef DAEMON_PRESENT
      if (msg->ds_buf[i].buffer) {
        pipeline->config_parm = (parm_buffer_t *)msg->ds_buf[i].buffer;
      } else {
        ret = FALSE;
        CLOGE(CAM_MCT_MODULE, "PARM Buf buffer null");
        break;
      }
#else
      pipeline->config_parm = mmap(NULL, msg->ds_buf[i].size,
        PROT_READ | PROT_WRITE, MAP_SHARED, msg->ds_buf[i].fd, 0);

      if (pipeline->config_parm == MAP_FAILED) {
        ret = FALSE;
        CLOGE(CAM_MCT_MODULE, "Mapping failed for param buf with err =%s",
          strerror(errno));
        break;
      }
#endif
      pipeline->config_parm_size = msg->ds_buf[i].size;
      pipeline->config_parm_fd   = msg->ds_buf[i].fd;
      ret = TRUE;
    }
      break;

    case CAM_MAPPING_BUF_TYPE_DUAL_CAM_CMD_BUF: {
#ifndef DAEMON_PRESENT
      if (msg->ds_buf[i].buffer) {
        pipeline->related_sensors_sync_buf =
          (cam_dual_camera_cmd_info_t *)msg->ds_buf[i].buffer;
      } else {
        ret= FALSE;
        CLOGE(CAM_MCT_MODULE, "SYNC_RELATED_SENSORS buffer null");
        break;
      }
#else
      pipeline->related_sensors_sync_buf =
        mmap(NULL, msg->ds_buf[i].size, PROT_READ | PROT_WRITE, MAP_SHARED,
        msg->ds_buf[i].fd, 0);

      if (pipeline->related_sensors_sync_buf == MAP_FAILED) {
        ret = FALSE;
        CLOGE(CAM_MCT_MODULE, "Mapping failed for sync buf with err =%s",
          strerror(errno));
        break;
      }
#endif
      pipeline->related_sensors_sync_buf_size = msg->ds_buf[i].size;
      pipeline->related_sensors_sync_buf_fd = msg->ds_buf[i].fd;
      ret = TRUE;
    }
      break;

    case CAM_MAPPING_BUF_TYPE_CAPABILITY: {
#ifndef DAEMON_PRESENT
      if (msg->ds_buf[i].buffer) {
        pipeline->query_buf = (cam_capability_t *)msg->ds_buf[i].buffer;
      } else {
        ret= FALSE;
        CLOGE(CAM_MCT_MODULE, "CAPABILITY buffer null");
        break;
      }
#else
      pipeline->query_buf = mmap(NULL, msg->ds_buf[i].size,
        PROT_READ | PROT_WRITE, MAP_SHARED, msg->ds_buf[i].fd, 0);

      if (pipeline->query_buf == MAP_FAILED) {
        ret = FALSE;
        CLOGE(CAM_MCT_MODULE, "Mapping failed for capability buf with err =%s",
          strerror(errno));
        break;
      }
#endif
      pipeline->query_buf_size = msg->ds_buf[i].size;
      pipeline->query_buf_fd   = msg->ds_buf[i].fd;
      ret = TRUE;
    }
      break;

    /* Below messages are per Stream */
    case CAM_MAPPING_BUF_TYPE_MISC_BUF:
    case CAM_MAPPING_BUF_TYPE_OFFLINE_META_BUF:
    case CAM_MAPPING_BUF_TYPE_OFFLINE_INPUT_BUF:
    case CAM_MAPPING_BUF_TYPE_STREAM_INFO:
    case CAM_MAPPING_BUF_TYPE_STREAM_BUF:
    case CAM_MAPPING_BUF_TYPE_STREAM_USER_BUF: {
      mct_pipeline_get_stream_info_t get_info;
      get_info.check_type   = CHECK_INDEX;
      get_info.stream_index = msg->ds_buf[i].stream;

      if ((stream = mct_pipeline_get_stream(pipeline, &get_info))) {
        if (stream->map_buf) {
          ret = stream->map_buf(msg->ds_buf + i, stream);
          if (FALSE == ret) {
            CLOGE(CAM_MCT_MODULE, "Mapping failed for buf type %d",
               msg->ds_buf[i].buf_type);
            msg->num_bufs = msg->num_bufs - 1;
            if (msg->num_bufs) {
              ret = mct_pipeline_unmap_buf(msg, pipeline);
              CLOGE(CAM_MCT_MODULE, "Unmapping failed for buf type %d",
                msg->ds_buf[i].buf_type);
              ret = FALSE;
            }
          }
        } else {
          ret = FALSE;
          CLOGE(CAM_MCT_MODULE, "No map method available");
        }
      } else {
        ret = FALSE;
        CLOGE(CAM_MCT_MODULE, "Stream [%d] not found",
          get_info.stream_index);
      }
    }
      break;

    default:
      break;
    } /* switch (msg->buf_type) */
  }
  return ret;
}

/** mct_pipeline_unmap_buf:
 *    @
 *    @
 *
 **/
boolean mct_pipeline_unmap_buf(void *message, mct_pipeline_t *pipeline)
{
  boolean ret = FALSE;
  int32_t rc;
  mct_serv_ds_msg_bundle_t *msg = (mct_serv_ds_msg_bundle_t *)message;
  mct_stream_t  *stream;
  uint32_t i = 0;

  if (!msg || !pipeline ||
      (msg->session != MCT_PIPELINE_SESSION(pipeline))) {
    CLOGE(CAM_MCT_MODULE, "Pipeline sanity failed msg =%p pipeline =%p",
    msg, pipeline);
    return ret;
  }

  for (i = 0; i < msg->num_bufs && i < CAM_MAX_NUM_BUFS_PER_STREAM; i++) {
    switch (msg->ds_buf[i].buf_type) {
    case CAM_MAPPING_BUF_TYPE_PARM_BUF: {
#ifdef DAEMON_PRESENT
      rc = munmap(pipeline->config_parm, pipeline->config_parm_size);
      close(pipeline->config_parm_fd);
      if (rc < 0) {
        ret = FALSE;
        CLOGE(CAM_MCT_MODULE, "Un-Mapping failed for param buf err =%s",
          strerror(errno));
        break;
      }
#endif
      pipeline->config_parm = NULL;
      pipeline->config_parm_size = 0;
      ret = TRUE;
    }
      break;

    case CAM_MAPPING_BUF_TYPE_DUAL_CAM_CMD_BUF: {
#ifdef DAEMON_PRESENT
      rc = munmap(pipeline->related_sensors_sync_buf,
        pipeline->related_sensors_sync_buf_size);
      close(pipeline->related_sensors_sync_buf_fd);
      if (rc < 0) {
        ret = FALSE;
        CLOGE(CAM_MCT_MODULE, "Un-Mapping failed for sync buf err =%s",
          strerror(errno));
        break;
      }
#endif
      pipeline->related_sensors_sync_buf = NULL;
      pipeline->related_sensors_sync_buf_size = 0;
      ret = TRUE;
    }
      break;

    case CAM_MAPPING_BUF_TYPE_CAPABILITY: {
#ifdef DAEMON_PRESENT
      rc = munmap(pipeline->query_buf, pipeline->query_buf_size);
      close(pipeline->query_buf_fd);
      if (rc < 0) {
        ret = FALSE;
        CLOGE(CAM_MCT_MODULE, "Un-Mapping failed for cap buf err =%s",
          strerror(errno));
        break;
      }
#endif
      pipeline->query_buf = NULL;
      pipeline->query_buf_size = 0;
      ret = TRUE;
    }
      break;

    /* Below messages are per Stream */
    case CAM_MAPPING_BUF_TYPE_MISC_BUF:
    case CAM_MAPPING_BUF_TYPE_OFFLINE_META_BUF:
    case CAM_MAPPING_BUF_TYPE_OFFLINE_INPUT_BUF:
    case CAM_MAPPING_BUF_TYPE_STREAM_INFO:
    case CAM_MAPPING_BUF_TYPE_STREAM_BUF:
    case CAM_MAPPING_BUF_TYPE_STREAM_USER_BUF: {

      mct_pipeline_get_stream_info_t get_info;
      get_info.check_type   = CHECK_INDEX;
      get_info.stream_index = msg->ds_buf[i].stream;

      if ((stream = mct_pipeline_get_stream(pipeline, &get_info))) {
        if (stream && stream->unmap_buf) {
          ret = stream->unmap_buf(msg->ds_buf + i, stream);
          if (FALSE == ret) {
            CLOGE(CAM_MCT_MODULE, "Unmapping failed for buf type %d",
              msg->ds_buf[i].buf_type);
          }
        } else {
          ret = FALSE;
          CLOGE(CAM_MCT_MODULE, "No unmap method available");
        }
      } else {
        ret = FALSE;
        CLOGE(CAM_MCT_MODULE, "Stream [%d] not found", get_info.stream_index);
      }
    }
     break;

    default:
      break;
    }
  }
  return ret;
}

/** mct_pipeline_pack_event:
 *    @
 *    @
 *    @
 *    @
 *
 *  Description:
 *    Fills in the parameter event with payload and other information.
 *
 *  Return:
 *     boolean TRUE if sucess
 *             FALSE if event is null
 **/
boolean mct_pipeline_pack_event(mct_event_type type, uint32_t identity,
  mct_event_direction direction, void *payload, mct_event_t* event)
{
  if( event == NULL)
    return FALSE;

  event->type = type;
  event->identity = identity;
  event->direction = direction;
  event->timestamp = 0;

  if (type == MCT_EVENT_CONTROL_CMD)
    event->u.ctrl_event = *((mct_event_control_t *)payload);
  else if (type == MCT_EVENT_MODULE_EVENT)
    event->u.module_event = *((mct_event_module_t *)payload);

  return TRUE;
}

/** Name: mct_pipeline_decide_hw_wakeup
 *
 *  Arguments/Fields:
 *    @pipeline: Structure of mct_controller
 *
 *  Return:
 *    TRUE on success, FALSE on failure
 *
 *  Description:
 *    Decide whether to wake up HW modules.
 *
 **/
static void mct_pipeline_decide_hw_wakeup(mct_pipeline_t *pipeline,
  boolean send_immediately)
{
  uint8_t super_param_active_duration;
  mct_pipeline_get_stream_info_t info;
  mct_stream_t *parm_stream;

  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Invalid pipeline pointer");
    return;
  }

  info.check_type  = CHECK_INDEX;
  info.stream_index  = MCT_SESSION_STREAM_ID;
  parm_stream = mct_pipeline_get_stream(pipeline, &info);
  if (!parm_stream) {
    CLOGE(CAM_MCT_MODULE, "Couldn't find session stream");
    return;
  }

  /* Update hw_sleep timer */
  if (send_immediately && (parm_stream->hw_sleep_frame_id <
    parm_stream->current_frame_id + pipeline->sleep_duration)) {
    parm_stream->hw_sleep_frame_id =
    parm_stream->current_frame_id + pipeline->sleep_duration;
  }

  /*Increment the number of set_param batch counter */
  if (!send_immediately)
    pipeline->set_param_cnt++;

  /* Send wake-up signal to HW modules if required
  * NOTE: Involves switching context to let SOF thread
  * control hw_state in a synchronous fashion*/
  if (pipeline->parm_buf_count &&
    (MCT_MODULE_STATE_ASLEEP == pipeline->module_hw_state) ) {
    mct_bus_msg_t bus_msg;
    mct_bus_msg_ctrl_request_frame_t ctrl;
    ctrl.request_flag = TRUE;
    ctrl.req_mode = FRM_REQ_MODE_BURST;
    ctrl.lpm_req_bit = LPM_REQUEST_BIT_MCT;
    memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
    bus_msg.sessionid = pipeline->session;
    bus_msg.type = MCT_BUS_MSG_CONTROL_REQUEST_FRAME;
    bus_msg.msg = &ctrl;
    bus_msg.size = sizeof(mct_bus_msg_ctrl_request_frame_t);
    if (pipeline->bus) {
      pipeline->bus->post_msg_to_bus(pipeline->bus, &bus_msg);
    }
  }

}

/** Name: mct_pipeline_send_ctrl_events
 *
 *  Arguments/Fields:
 *    @pipeline: Structure of mct_controller
 *    @event_type: Type of control event
 *    @send_immediately: Switch to either send params right away
 *       or to store into super_param_queue (for achieving per-frame-control)
 *
 *  Return:
 *    TRUE on success, FALSE on failure
 *
 *  Description:
 *    Handles all control events sent by HAL.
 *    If send_immediately is set to TRUE, HAL config_parms are sent to
 *    downstream modules right away.
 *    If not, they are stored in a queue to be synchronized and sent to
 *    downstream modules in the subsequent SOF.
 *
 **/
boolean mct_pipeline_send_ctrl_events(mct_pipeline_t *pipeline,
  mct_event_control_type_t event_type, boolean send_immediately)
{
  boolean ret = TRUE;
  mct_event_t cmd_event;
  mct_event_super_control_parm_t *super_event = NULL;
  mct_event_control_t event_data;
  mct_event_control_parm_t event_parm;
  parm_buffer_t *p_table = NULL;
  mct_stream_t *parm_stream = NULL;
  mct_pipeline_get_stream_info_t info;
  uint32_t index = 0;
  uint32_t frame_num = UINT32_MAX;
  int current = 0;
  boolean rc = TRUE;
  uint32_t sync_param = 0;
  size_t parm_size = 0;

  info.check_type  = CHECK_INDEX;
  info.stream_index  = MCT_SESSION_STREAM_ID;
  parm_stream = mct_pipeline_get_stream(pipeline, &info);
  if (!parm_stream) {
    CLOGE(CAM_MCT_MODULE, "Couldn't find session stream");
    return FALSE;
  }
  if (parm_stream->state == MCT_ST_STATE_BAD) {
    CLOGE(CAM_MCT_MODULE, "Daemon already died, not processing set params");
    return FALSE;
  }

  if (NULL == pipeline->config_parm) {
    CLOGE(CAM_MCT_MODULE, "HAL Config_parm buffer not mapped!");
    return FALSE;
  }
  p_table = (parm_buffer_t *)pipeline->config_parm;

  /* Search through config buffer for valid entries */
  pipeline->parm_buf_count = 0;
  for (current = 0; current < CAM_INTF_PARM_MAX; current++) {
    if (p_table->is_valid[current]) {
      if(current == CAM_INTF_META_SEND_IMMEDIATELY && pipeline->hal_version == CAM_HAL_V1) {
        send_immediately = true;
      }
      pipeline->valid_parm_index_tbl[pipeline->parm_buf_count++] =
        current;
    }
  }
  /* Initialize super_event structure */
  if (!send_immediately) {
    super_event = malloc(sizeof(mct_event_super_control_parm_t));
    if (NULL == super_event) {
      CLOGE(CAM_MCT_MODULE, "super event malloc failed");
      return FALSE;
    }
    memset(super_event, 0, sizeof(mct_event_super_control_parm_t));
    super_event->parm_events = (mct_event_control_parm_t *)calloc (
      pipeline->parm_buf_count, sizeof (mct_event_control_parm_t));
    if (NULL == super_event->parm_events) {
      free(super_event);
      CLOGE(CAM_MCT_MODULE, "param events calloc failed");
      return FALSE;
    }
  }

  for(current = 0; current < pipeline->parm_buf_count; current++) {
    index = pipeline->valid_parm_index_tbl[current];
    event_parm.type = index;
    event_parm.parm_data = get_pointer_of(index, p_table);
    parm_size = (get_size_of(index) + sizeof(mct_event_control_parm_t));
    if (NULL == event_parm.parm_data) {
      CLOGW(CAM_MCT_MODULE, "WARNING: event_parm.parm_data = NULL for %d."
        "Update get_pointer_of and get_size_of functions",
         index);
      continue;
    }

    /* Some events are meant to be handled in MCT alone.
    Add special handling for such events */
    if (event_parm.type == CAM_INTF_PARM_HAL_VERSION) {
      pipeline->hal_version = *(int32_t *)event_parm.parm_data;
      if (pipeline->hal_version == CAM_HAL_V1) {
        /* Do not control hw_state in HAL1 case because
        set_params are not refreshed every single frame */
        if (pipeline->module_hw_state == MCT_MODULE_STATE_NONE) {
          pipeline->module_hw_state = MCT_MODULE_STATE_AWAKE;
          pipeline->updated_hw_state = MCT_MODULE_STATE_AWAKE;
        }
        pipeline->session_data.max_pipeline_meta_reporting_delay = 0;
        pipeline->session_data.set_session_mask |= META_REPORTING_DELAY_MASK;
        rc &= mct_list_traverse(pipeline->modules, mct_pipeline_set_session_data,
                pipeline);
      }
      else if (pipeline->hal_version == CAM_HAL_V3) {
        if (pipeline->lpm_mode.perf_mode == CAM_PERF_NONE) {
          pipeline->lpm_mode.perf_mode = CAM_PERF_SENSOR_SUSPEND;
        }
        if (pipeline->module_hw_state == MCT_MODULE_STATE_NONE) {
          pipeline->module_hw_state = MCT_MODULE_STATE_AWAKE;
        }
        if (!pipeline->session_data.max_pipeline_meta_reporting_delay) {
          pipeline->session_data.max_pipeline_meta_reporting_delay = 1;
          pipeline->session_data.set_session_mask |= META_REPORTING_DELAY_MASK;
          rc &= mct_list_traverse(pipeline->modules, mct_pipeline_set_session_data,
                  pipeline);
        }
      }
    }

    if (event_parm.type == CAM_INTF_PARM_ZSL_MODE) {
      pipeline->is_zsl_mode = *(int32_t *)event_parm.parm_data;
    }
    if (event_parm.type ==  CAM_INTF_META_FRAME_NUMBER) {
      frame_num = *(unsigned int *)(event_parm.parm_data);
    }
    if (event_parm.type ==  CAM_INTF_PARM_UPDATE_DEBUG_LEVEL) {
      cam_set_dbg_log_properties();
    }
    if (event_parm.type ==  CAM_INTF_META_STREAM_INFO) {
      pipeline->session_data.streamConfigInfo =
          *(cam_stream_size_info_t *)event_parm.parm_data;
    }
    if (event_parm.type ==  CAM_INTF_PARM_SYNC_DC_PARAMETERS) {
      sync_param = *(unsigned int *)(event_parm.parm_data);
    }

    /* Send immediately before stream on */
    if (send_immediately || (event_parm.type == CAM_INTF_PARM_INT_EVT) ||
      (event_parm.type == CAM_INTF_META_STREAM_INFO) ||
      event_parm.type == CAM_INTF_META_RAW) {

      if (pipeline->send_event) {
        event_data.type = event_type;
        event_data.size = parm_size;
        event_data.control_event_data = &event_parm;
        if (send_immediately) {
          event_data.current_frame_id = 0;
        } else {
          event_data.current_frame_id = parm_stream->current_frame_id;
        }
        ret = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
                                      parm_stream->streaminfo.identity,
                                      MCT_EVENT_DOWNSTREAM, &event_data,
                                      &cmd_event);
        if (ret == FALSE) {
          CLOGE(CAM_MCT_MODULE, "Error in packing event no. %d/%d: type %d",
             current+1, pipeline->parm_buf_count, index);
          break;
        }
        if ((event_parm.type == CAM_INTF_META_FRAME_NUMBER) &&
          (pipeline->hal_version == CAM_HAL_V3)) {
          /* If super-parms with valid frame_num get sent
             prior to stream-on, add to tracking map */
          mct_stream_map_frame_number(pipeline,
            frame_num, event_data.current_frame_id);
          if(pipeline->module_hw_state == MCT_MODULE_STATE_ASLEEP) {
            CLOGE(CAM_MCT_MODULE, "Receiving super_event with frame_num"
              " while lpm is on");
          }
        }
        ret = pipeline->send_event(pipeline, parm_stream->streamid,
                                   &cmd_event);
        if (ret == FALSE) {
          CLOGE(CAM_MCT_MODULE, "Error in sending event no. %d/%d: type %d",
             current+1, pipeline->parm_buf_count, index);
          break;
        }
      } else {
        break;
      }
    } else {

      /* Deep-copy parm_data when storing to queue
         because original param buffer goes out of scope
         at the end of the CAM_PRIV_PARM synchronous call.
      */
      if (super_event->parm_events) {
        uint32_t num_of_parm_events = super_event->num_of_parm_events;
        super_event->parm_events[num_of_parm_events].type = event_parm.type;

        uint32_t copysize = get_size_of(index);
        super_event->parm_events[num_of_parm_events].parm_data =
          malloc (copysize);
        if (super_event->parm_events[num_of_parm_events].parm_data) {
          memcpy(super_event->parm_events[num_of_parm_events].parm_data,
            event_parm.parm_data, copysize);
          super_event->num_of_parm_events++;
        } else {
          CLOGE(CAM_MCT_MODULE, "Allocating %d bytes for parm_data failed",
                copysize);
        }
      } else {
          CLOGE(CAM_MCT_MODULE,
                "Allocation for %d entries in parm_events failed",
                pipeline->parm_buf_count);
      }
    }
  }

  if ( !send_immediately && super_event ) {
    /* Store super event in super_param_queue */
    super_event->identity = parm_stream->streaminfo.identity;
    super_event->frame_number = frame_num;
    super_event->applying_frame_id = 0;
    super_event->is_sync_param = sync_param;
    CLOGD(CAM_MCT_MODULE, "Storing super_event [%p], frame_num [%d] is_sync_param=%d",
       super_event, super_event->frame_number, super_event->is_sync_param);
    pthread_mutex_lock(&pipeline->super_param_q_lock);
    mct_queue_push_tail(pipeline->super_param_queue, super_event);
    pthread_mutex_unlock(&pipeline->super_param_q_lock);
  }
  /*Checking if AUX module needs to wake up based on oncoming set_param request */
  if (pipeline->lpm_owner && pipeline->hal_version == CAM_HAL_V3) {
    mct_pipeline_decide_hw_wakeup(pipeline, send_immediately);
  }

  /* Clear reference table after use*/
  memset(pipeline->valid_parm_index_tbl, 0,
    (CAM_INTF_PARM_MAX * sizeof(uint16_t)));
  pipeline->parm_buf_count = 0;

  return ret;
}

/** Name: mct_pipeline_store_special_event
 *
 *  Arguments/Fields:
 *    @pipeline: MCT pipeline pointer
 *    @event: Event of type mct_event_control_t
 *
 *  Return:
 *    void
 *  Description:
 *  Method to queue new HAL command event. These could be one of the following:
 *    CAM_PRIV_DO_AUTO_FOCUS
 *    CAM_PRIV_CANCEL_AUTO_FOCUS
 *    CAM_PRIV_PREPARE_SNAPSHOT
 *    CAM_PRIV_STREAM_INFO_SYNC
 *    CAM_PRIV_STREAM_PARM
 *    CAM_PRIV_START_ZSL_SNAPSHOT
 *    CAM_PRIV_STOP_ZSL_SNAPSHOT
 **/
void mct_pipeline_store_special_event(mct_pipeline_t *pipeline, void *event)
{
  pthread_mutex_lock(&pipeline->param_q_lock);
  mct_queue_push_tail(pipeline->param_queue, event);
  pthread_mutex_unlock(&pipeline->param_q_lock);
}

/** Name: mct_pipeline_send_special_event
 *
 *  Arguments/Fields:
 *    @pipeline: MCT pipeline pointer
 *    @parm_stream: Session stream on which to send queued events
 *
 *  Return:
 *    TRUE on success, FALSE on failure
 *  Description:
 *    This method posts events stored in pipeline->param_queue at SOF.
 *    It subsequently frees these entries.
 **/
boolean mct_pipeline_send_special_event(mct_pipeline_t *pipeline,
  mct_stream_t *parm_stream)
{
  boolean ret = TRUE;
  mct_event_t *p_cmd_event;
  mct_event_t event;

  if (!pipeline || !parm_stream) {
    CLOGE(CAM_MCT_MODULE, "NULL ptr detected: pipeline [%p], parm_stream [%p]",
       pipeline, parm_stream);
    return FALSE;
  }
  /*Send all events in pipeline->param_queue */
  do {
    pthread_mutex_lock(&pipeline->param_q_lock);
    p_cmd_event = (mct_event_t *)mct_queue_pop_head(pipeline->param_queue);
    pthread_mutex_unlock(&pipeline->param_q_lock);
    if (p_cmd_event) {
      if (pipeline->send_event) {
        /* Some of the modules ovewrite event fields; send a copy */
        memcpy(&event, p_cmd_event, sizeof(event));
        ret &= pipeline->send_event(pipeline, parm_stream->streamid, &event);
        if (FALSE == ret) {
          CLOGE(CAM_MCT_MODULE, "Failure in sending event");
          return FALSE;
        }
      }
      /* Clear queue entry */
      ret &= mct_pipeline_free_param(p_cmd_event, NULL);
      if (FALSE == ret) {
        CLOGE(CAM_MCT_MODULE, "Failure in freeing event");
        return FALSE;
      }
    } else {
      break;
    }
  } while (1);

  return ret;
}

/** Name: mct_pipeline_is_valid_lpm_req_bit
 *
 *  Arguments/Fields:
 *    @lpm_req_bit: input lpm request bit
 *
 *  Return:
 *    TRUE on success, FALSE on failure
 *  Description:
 *    This method checks if the lpm request bit is valid
**/
static boolean mct_pipeline_is_valid_lpm_req_bit(
  mct_lpm_request_bit_t lpm_req_bit)
{
    /* lpm_req_bit can contain priority bit ORed with any one of the other valid module bit. */
    if (lpm_req_bit & LPM_REQUEST_BIT_PRIORITY) {
        /* If priority bit is set, remove it for module bit comparison. */
        lpm_req_bit &= ~LPM_REQUEST_BIT_PRIORITY;
    }

    if ((lpm_req_bit == LPM_REQUEST_BIT_MCT)         ||
            (lpm_req_bit == LPM_REQUEST_BIT_SENSOR)  ||
            (lpm_req_bit == LPM_REQUEST_BIT_ISP)     ||
            (lpm_req_bit == LPM_REQUEST_BIT_PPROC)   ||
            (lpm_req_bit == LPM_REQUEST_BIT_STATS)   ||
            (lpm_req_bit == LPM_REQUEST_BIT_STATS_IS)) {
        return true;
    } else {
        return false;
    }
}

/** Name: mct_pipeline_send_module_wakeup
 *
 *  Arguments/Fields:
 *    @pipeline: MCT pipeline pointer
 *
 *  Return:
 *    TRUE on success, FALSE on failure
 *  Description:
 *    This method sends a wake-up signal to HW modules.
 *    It also updates the latest frame id until which hw can remain awake.
**/
boolean mct_pipeline_send_module_wakeup(mct_pipeline_t *pipeline)
{
  boolean ret = TRUE;
  mct_event_t cmd_event;
  mct_event_control_t event_data;
  mct_pipeline_get_stream_info_t info;
  mct_stream_t *parm_stream;
  cam_dual_camera_perf_control_t perf_lpm_mode;

  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Invalid pipeline pointer");
    return FALSE;
  }

  info.check_type  = CHECK_INDEX;
  info.stream_index  = MCT_SESSION_STREAM_ID;
  parm_stream = mct_pipeline_get_stream(pipeline, &info);
  if (!parm_stream) {
    CLOGE(CAM_MCT_MODULE, "Couldn't find session stream");
    return FALSE;
  }

  if (pipeline->updated_hw_state != pipeline->module_hw_state) {
    event_data.type = MCT_EVENT_CONTROL_HW_WAKEUP;
    pipeline->lpm_mode.enable = FALSE;
    perf_lpm_mode = pipeline->lpm_mode;
    event_data.control_event_data = &perf_lpm_mode;
    event_data.size = sizeof(cam_dual_camera_perf_control_t);
    ret = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
      (pack_identity(MCT_PIPELINE_SESSION(pipeline), parm_stream->streamid)),
      MCT_EVENT_DOWNSTREAM, &event_data, &cmd_event);
    if (ret == FALSE) {
      CLOGE(CAM_MCT_MODULE, "Error in packing event");
      return ret;
    }
    if (pipeline->send_event) {
      CLOGI(CAM_MCT_MODULE, "Sending HW_WAKEUP at frame id %d",
        parm_stream->current_frame_id);
      pipeline->module_hw_state = MCT_MODULE_STATE_AWAKE;
      ret = pipeline->send_event(pipeline, parm_stream->streamid,
                                   &cmd_event);
      if (TRUE == ret) {
       CLOGI(CAM_MCT_MODULE, "Sending HW_WAKEUP at frame id %d done",
       parm_stream->current_frame_id);
        pipeline->state_change = TRUE;
        if(pipeline->state_update_count > 0) {
          pipeline->state_update_count--;
        }
        if (!pipeline->bus) {
          CLOGE(CAM_MCT_MODULE,"Bus pointer not initialized");
          return FALSE;
        }
        /* Awaken SOF timer */
        if(!pipeline->bus->thread_run) {
          mct_bus_msg_t bus_msg;
          uint32_t sof_timeout_interval;
          memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
          bus_msg.sessionid = pipeline->session;
          bus_msg.type = MCT_BUS_MSG_SENSOR_STARTING;
          sof_timeout_interval = MCT_BUS_DEFAULT_SOF_TIMEOUT;
          bus_msg.msg = &sof_timeout_interval;
          if (pipeline->bus) {
            pipeline->bus->post_msg_to_bus(pipeline->bus, &bus_msg);
          }
        }
      } else {
        pipeline->module_hw_state = MCT_MODULE_STATE_ASLEEP;
        CLOGE(CAM_MCT_MODULE, "Failure in setting hw_state to WAKE at frame %d",
          parm_stream->current_frame_id);
      }
    } else {
      CLOGE(CAM_MCT_MODULE, "send_event not registered");
      ret = FALSE;
    }
  }

  return ret;
}

/** Name: mct_pipeline_send_module_sleep
 *
 *  Arguments/Fields:
 *    @pipeline: MCT pipeline pointer
 *
 *  Return:
 *    TRUE on success, FALSE on failure
 *  Description:
 *    This method sends a sleep signal to HW modules to put them into
 *    a power saving mode at the expiry of the hw_awake period.
**/
boolean mct_pipeline_send_module_sleep(mct_pipeline_t *pipeline)
{
  boolean ret = TRUE;
  mct_event_t cmd_event;
  mct_event_control_t event_data;
  mct_pipeline_get_stream_info_t info;
  mct_stream_t *parm_stream;
  cam_dual_camera_perf_control_t perf_lpm_mode;

  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Invalid pipeline ptr");
    return FALSE;
  }

  info.check_type  = CHECK_INDEX;
  info.stream_index  = MCT_SESSION_STREAM_ID;
  parm_stream = mct_pipeline_get_stream(pipeline, &info);
  if (!parm_stream) {
    CLOGE(CAM_MCT_MODULE, "Couldn't find session stream");
    return FALSE;
  }

  if ((pipeline->updated_hw_state != pipeline->module_hw_state) &&
    (FRM_REQ_MODE_CONTINUOUS != pipeline->wake_mode)) {
    event_data.type = MCT_EVENT_CONTROL_HW_SLEEP;
    pipeline->lpm_mode.enable = TRUE;
    perf_lpm_mode = pipeline->lpm_mode;
    event_data.control_event_data = &perf_lpm_mode;
    event_data.size = sizeof(cam_dual_camera_perf_control_t);
    ret = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
      (pack_identity(MCT_PIPELINE_SESSION(pipeline), parm_stream->streamid)),
      MCT_EVENT_DOWNSTREAM, &event_data, &cmd_event);
    if (ret == FALSE) {
      CLOGE(CAM_MCT_MODULE, "Error in packing event");
      return ret;
    }
    if (pipeline->send_event && !(pipeline->frame_num_idx_list)) {
      CLOGI(CAM_MCT_MODULE, "Sending HW_SLEEP at frame id %d session =%d",
        parm_stream->current_frame_id, pipeline->session);
      ret = pipeline->send_event(pipeline, parm_stream->streamid,
                                 &cmd_event);
      if (TRUE == ret) {
        pipeline->module_hw_state = MCT_MODULE_STATE_ASLEEP;
        pipeline->state_change = FALSE;
        if(pipeline->state_update_count > 0) {
          pipeline->state_update_count--;
        }
        /* Stop SOF timer */
        if (pipeline->lpm_mode.perf_mode == CAM_PERF_SENSOR_SUSPEND ||
            pipeline->lpm_mode.perf_mode == CAM_PERF_ISPIF_FRAME_DROP){
          mct_bus_msg_t bus_msg;
          memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
          bus_msg.sessionid = pipeline->session;
          bus_msg.type = MCT_BUS_MSG_SENSOR_STOPPING;
          bus_msg.msg = NULL;
          if (pipeline->bus) {
            pipeline->bus->post_msg_to_bus(pipeline->bus, &bus_msg);
          }
        }
      } else {
        CLOGE(CAM_MCT_MODULE, "Failure in setting hw_state to SLEEP at frame %d",
          parm_stream->current_frame_id);
      }
    } else {
      CLOGE(CAM_MCT_MODULE, "send_event not registered or metadata still present");
      ret = FALSE;
    }
  }

  return ret;
}

/** Name: mct_pipeline_control_hw_state
 *
 *  Arguments/Fields:
 *    @pipeline: MCT pipeline pointer
 *    @bus_msg: Control value passed as argument.
 *       bus_msg->request_flag: TRUE: START, FALSE: STOP
 *
 *  Return:
 *    TRUE on success, FALSE on failure
 *  Description:
 *    This method controls the state of the hardware modules
 *    based on the type of request.
**/
boolean mct_pipeline_control_hw_state(mct_pipeline_t *pipeline,
  mct_bus_msg_ctrl_request_frame_t *ctrl)
{
  boolean ret  = TRUE;

  if (!pipeline || !ctrl) {
    CLOGE(CAM_MCT_MODULE, "NULL ptr: pipeline = %p, bus_msg = %p",
      pipeline, ctrl);
    return FALSE;
  }

  if (!mct_pipeline_is_valid_lpm_req_bit(ctrl->lpm_req_bit)) {
    CLOGH(CAM_MCT_MODULE, "Invalid LPM request bit. Ignoring Sleep and Awake commands");
    return TRUE;
  }

  if (ctrl->request_flag) {
    pipeline->wakeup_bit_mask |= ctrl->lpm_req_bit;
  } else {
    pipeline->wakeup_bit_mask &= ~ctrl->lpm_req_bit;
  }

  if (pipeline->wakeup_bit_mask) {
    pipeline->updated_hw_state = MCT_MODULE_STATE_AWAKE;
  } else {
    pipeline->updated_hw_state = MCT_MODULE_STATE_ASLEEP;
  }

  /*Checking if priority bit is set in any request */
  if (pipeline->wakeup_bit_mask & LPM_REQUEST_BIT_PRIORITY) {
    pipeline->updated_hw_state = MCT_MODULE_STATE_AWAKE;
    pipeline->wakeup_bit_mask = 0;
  }

  if (pipeline->updated_hw_state == MCT_MODULE_STATE_AWAKE) {
    ret = mct_pipeline_send_module_wakeup(pipeline);
    if (!ret) {
      CLOGE(CAM_MCT_MODULE, "Failed to set HW state for session %d",
        pipeline->session);
      return ret;
    }
  }
  return ret;
}

/** Name: mct_pipeline_process_dual_cam_cmd
 *
 *  Arguments/Fields:
 *    @pipeline: MCT pipeline pointer
 *
 *  Return:
 *    TRUE on success, FALSE on failure
 *  Description:
 *    This method handles dual camera related events
 * from HAL.
**/

boolean mct_pipeline_process_dual_cam_cmd(mct_pipeline_t *pipeline)
{
  boolean ret  = TRUE;
  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Invalid pipeline ptr");
    return FALSE;
  }

  /*start reading the sync buf from HAL*/
  cam_dual_camera_cmd_info_t *dual_cam_info = (cam_dual_camera_cmd_info_t *)
    pipeline->related_sensors_sync_buf;
  if (dual_cam_info == NULL) {
    CLOGE(CAM_MCT_MODULE, "Dual camera cmd buffer is not mapped yet");
    return FALSE;
  }

  switch(dual_cam_info->cmd_type) {
    case CAM_DUAL_CAMERA_BUNDLE_INFO: {
      cam_dual_camera_bundle_info_t* related_cam_info =
        &dual_cam_info->bundle_info;
      if (related_cam_info == NULL) {
        ret = FALSE;
        CLOGE(CAM_MCT_MODULE, "Invalid input for BUNDLE Info");
        break;
      }
      pipeline->lpm_mode.perf_mode = related_cam_info->perf_mode;
      mct_event_t cmd_event;
      mct_event_control_t relatedcam_event_data;

      if(related_cam_info->sync_control == CAM_SYNC_RELATED_SENSORS_ON) {
        relatedcam_event_data.type = MCT_EVENT_CONTROL_LINK_INTRA_SESSION;
      } else {
        relatedcam_event_data.type = MCT_EVENT_CONTROL_UNLINK_INTRA_SESSION;
      }
      related_cam_info->related_sensor_session_id =
          pack_identity(related_cam_info->related_sensor_session_id,
          MCT_SESSION_STREAM_ID);
      CLOGI(CAM_MCT_MODULE,
          "Sending Dual-Cam Event %d on session %d to connect to \
          session %x dualcam sensor info %d", relatedcam_event_data.type,
          MCT_PIPELINE_SESSION(pipeline),
          related_cam_info->related_sensor_session_id,
          related_cam_info->type);

       /* Create Supermct instance only for HAL-1*/
       if (pipeline->hal_version == CAM_HAL_V1) {
         if (FALSE == is_supermct_exist(pipeline)) {
           supermct_new(pipeline);
         }
       }

       /* Set lpm Owner in pipeline */
       pipeline->lpm_owner = !(related_cam_info->hal_lpm_control);

       relatedcam_event_data.control_event_data =
           related_cam_info;
       relatedcam_event_data.size = sizeof(cam_dual_camera_bundle_info_t);

      MCT_PROF_LOG_BEG(PROF_MCT_CAM_COMMAND, "command", data->command);

      ret = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
          pack_identity(MCT_PIPELINE_SESSION(pipeline),
          MCT_SESSION_STREAM_ID),
          MCT_EVENT_DOWNSTREAM, &relatedcam_event_data, &cmd_event);
      if (ret == FALSE) {
        CLOGE(CAM_MCT_MODULE, "Error in packing event");
        break;
      }

      /*Set aux_cam flag to TRUE for aux_camera*/
      if (related_cam_info->mode == CAM_MODE_SECONDARY) {
        pipeline->aux_cam = TRUE;
      } else {
        pipeline->aux_cam = FALSE;
      }
      ret = pipeline->send_event(pipeline, MCT_SESSION_STREAM_ID, &cmd_event);

      MCT_PROF_LOG_END();
    }
      break;

    case CAM_DUAL_CAMERA_LOW_POWER_MODE: {
      cam_dual_camera_perf_control_t *dual_cam_lpm = &dual_cam_info->value;
      if(dual_cam_lpm == NULL) {
        ret = FALSE;
        CLOGE(CAM_MCT_MODULE, "Invalid input for PERF Info");
        break;
      }
      /*Caching the value to use it later */
      pipeline->lpm_mode = *dual_cam_lpm;

      /*Sending bus msg to enable/disable LPM */
      mct_bus_msg_t bus_msg;
      mct_bus_msg_ctrl_request_frame_t ctrl;
      memset(&ctrl, 0, sizeof(ctrl));
      memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
      bus_msg.sessionid = pipeline->session;
      bus_msg.type = MCT_BUS_MSG_CONTROL_REQUEST_FRAME;
      bus_msg.msg = &ctrl;
      bus_msg.size = sizeof(mct_bus_msg_ctrl_request_frame_t);

      if (dual_cam_lpm->enable) {
        ctrl.request_flag = FALSE;
        ctrl.req_mode = FRM_REQ_MODE_NONE;
        ctrl.lpm_req_bit = 0;
        if (dual_cam_lpm->priority) {
          ctrl.lpm_req_bit |= LPM_REQUEST_BIT_MCT | LPM_REQUEST_BIT_PRIORITY;
        } else {
          ctrl.lpm_req_bit |= LPM_REQUEST_BIT_MCT;
        }
        pipeline->current_hw_state_requested = MCT_MODULE_STATE_ASLEEP;
        pipeline->state_change = true;
        CLOGI(CAM_MCT_MODULE, "Suspend camera %d type = %d priority = %d",
          MCT_PIPELINE_SESSION(pipeline), dual_cam_info->cmd_type,
          dual_cam_lpm->priority);
      } else {
        ctrl.request_flag = TRUE;
        ctrl.req_mode = FRM_REQ_MODE_CONTINUOUS;
        ctrl.lpm_req_bit |= LPM_REQUEST_BIT_MCT;
        pipeline->current_hw_state_requested = MCT_MODULE_STATE_AWAKE;
        CLOGI(CAM_MCT_MODULE, "Wake-up camera %d type = %d priority %d",
          MCT_PIPELINE_SESSION(pipeline), dual_cam_info->cmd_type,
          dual_cam_lpm->priority);
      }
      if (pipeline->bus) {
        pipeline->state_update_count++;
        pipeline->bus->post_msg_to_bus(pipeline->bus, &bus_msg);
      }
    }
      break;

    case CAM_DUAL_CAMERA_MASTER_INFO: {
      cam_dual_camera_master_info_t *dual_master_info = &dual_cam_info->mode;
      if(dual_master_info == NULL) {
        ret = FALSE;
        CLOGE(CAM_MCT_MODULE, "Invalid input for MASTER Info");
        break;
      }

      mct_event_t cmd_event;
      mct_event_control_t event_data;
      CLOGI(CAM_MCT_MODULE, "Master Switch: session %d Mode %d",
          MCT_PIPELINE_SESSION(pipeline), dual_master_info->mode);
      event_data.type = MCT_EVENT_CONTROL_MASTER_INFO;
      event_data.control_event_data = dual_master_info;
      event_data.size = sizeof(cam_dual_camera_master_info_t);
      MCT_PROF_LOG_BEG(PROF_MCT_CAM_COMMAND, "command", data->command);
      ret = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
          pack_identity(MCT_PIPELINE_SESSION(pipeline), MCT_SESSION_STREAM_ID),
          MCT_EVENT_DOWNSTREAM, &event_data, &cmd_event);
      if (ret == FALSE) {
        CLOGE(CAM_MCT_MODULE, "Error in packing event");
        break;
      }
      /*Set aux_cam flag to TRUE for aux_camera*/
      if (dual_master_info->mode == CAM_MODE_SECONDARY) {
        pipeline->aux_cam = TRUE;
      } else {
        pipeline->aux_cam = FALSE;
      }
      ret = pipeline->send_event(pipeline, MCT_SESSION_STREAM_ID, &cmd_event);
      MCT_PROF_LOG_END();
    }
      break;

    case CAM_DUAL_CAMERA_FALLBACK_INFO: {
      cam_dual_camera_fallback_info_t *dual_fallback_info = &dual_cam_info->fallback;
      if(dual_fallback_info == NULL) {
        ret = FALSE;
        CLOGE(CAM_MCT_MODULE, "Invalid input for FALLBACK Info");
        break;
      }

      mct_event_t cmd_event;
      mct_event_control_t event_data;
      CLOGI(CAM_MCT_MODULE, "Fallback: session %d fallback %d",
          MCT_PIPELINE_SESSION(pipeline), dual_fallback_info->fallback);
      event_data.type = MCT_EVENT_CONTROL_FALLBACK_INFO;
      event_data.control_event_data = dual_fallback_info;
      event_data.size = sizeof(cam_dual_camera_fallback_info_t);
      ret = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
          pack_identity(MCT_PIPELINE_SESSION(pipeline), MCT_SESSION_STREAM_ID),
          MCT_EVENT_DOWNSTREAM, &event_data, &cmd_event);
      if (ret == FALSE) {
        CLOGE(CAM_MCT_MODULE, "Error in packing event");
        break;
      }
      ret = pipeline->send_event(pipeline, MCT_SESSION_STREAM_ID, &cmd_event);
    }
      break;

    case CAM_DUAL_CAMERA_DEFER_INFO: {
      cam_dual_camera_defer_cmd_t cam_defer_cmd = dual_cam_info->defer_cmd;

      if (cam_defer_cmd == pipeline->deferred_state) {
          /*No op*/
          break;
      }

      switch(cam_defer_cmd) {
        case (CAM_DEFER_START): {
          CLOGI(CAM_MCT_MODULE, "Received the command on session %d to defer cmds",
            pipeline->session);

          if (pipeline->deferred_state != CAM_DEFER_DEFAULT) {
              CLOGI(CAM_MCT_MODULE, "ignore defer cmd in state:%d", pipeline->deferred_state);
              break;
          }
          /*Initializing the defer queue */
          pthread_mutex_init(&pipeline->defer_q_lock, NULL);
          pipeline->defer_queue = (mct_queue_t *)mct_queue_new;
          if (!pipeline->defer_queue) {
            pthread_mutex_destroy(&pipeline->defer_q_lock);
            CLOGE(CAM_MCT_MODULE, "Failed to create defer queue");
            ret = FALSE;
            break;
          }
          memset(pipeline->defer_queue, 0, sizeof(mct_queue_t));
          mct_queue_init(pipeline->defer_queue);
          pipeline->deferred_state = CAM_DEFER_START;
        }
        break;

        case (CAM_DEFER_PROCESS): {
          if (pipeline->deferred_state == CAM_DEFER_START) {
            CLOGI(CAM_MCT_MODULE, "Received command to process defer q on session %d",
              pipeline->session);
            ret = mct_pipeline_process_defer_queue(pipeline);
            if (ret == FALSE) {
              CLOGE(CAM_MCT_MODULE, "Failed to process defer commands");
            }
          }
        }
        break;

        case (CAM_DEFER_FLUSH): {
          CLOGI(CAM_MCT_MODULE, "Received command to flush the defer queue on session %d",
            pipeline->session);
          mct_pipeline_flush_defer_queue(pipeline);
        }
        break;

        default: {
          CLOGE(CAM_MCT_MODULE, "Unrecognized command");
          break;
        }
      }
    }
      break;
    default:
      break;
  }
  return ret;
}

/** mct_pipeline_check_matching_dim:
 *    @src: input dimensions
 *    @ref_table: pointer to reference dim table for matching
 *    @num_entries: number of entries in reference table
 *
 *  Return: index of closest matching entry from reference dimension table
 **/
static int8_t mct_pipeline_check_matching_dim (
  cam_dimension_t src_dim, cam_dimension_t* ref_table, uint8_t num_entries)
{
  uint8_t index;
  int8_t matching_index = -1;
  uint32_t score = UINT_MAX;
  uint32_t deviation = 0;

  if(!ref_table || (0 == num_entries))
  {
    CLOGE(CAM_MCT_MODULE, "Invalid reference table");
    return matching_index;
  }
  for (index = 0; (index < num_entries && index < MAX_SIZES_CNT); index++) {
      if (ref_table[index].width <= src_dim.width &&
          ref_table[index].height <= src_dim.height)
        break;
  }
  matching_index = index;
  if(matching_index != -1) {
    /* Valid match has been found */
    CLOGD(CAM_MCT_MODULE, "Closest matching entry: ref_table[%d] = [%d x %d]",
      matching_index,
      ref_table[matching_index].width, ref_table[matching_index].height);
  }
  else {
    CLOGD(CAM_MCT_MODULE, "No match found for src dim [%d X %d]",
       src_dim.width, src_dim.height);
  }
  return matching_index;
}

/** mct_pipeline_find_matching_hfr_dim:
 *    @mode: HFR mode (60/90/120/240...)
 *    @sensor_dim: sensor dimensions for a particular FPS mode
 *    @hfr_sensor_width, hfr_sensor_height: local HFR dimension arrays
 *
 *  Return: NONE
 **/
 static void mct_pipeline_find_matching_hfr_dim (
  cam_hfr_mode_t mode, cam_dimension_t sensor_dim,
  int32_t *hfr_sensor_width, int32_t *hfr_sensor_height )
{
  int8_t matching_index = -1;
  if (!hfr_sensor_width || !hfr_sensor_height) {
    CLOGE(CAM_MCT_MODULE, "NULL ptr: hfr_sensor_width=%p,hfr_sensor_height=%p",
      hfr_sensor_width, hfr_sensor_height);
    return;
  }

  if (sensor_dim.width > hfr_sensor_width[mode] &&
    sensor_dim.height > hfr_sensor_height[mode]) {
    matching_index = mct_pipeline_check_matching_dim (
      sensor_dim, default_hfr_video_sizes,
      sizeof(default_hfr_video_sizes) / sizeof(cam_dimension_t));

    if ( -1 != matching_index && matching_index <
      (int8_t)(sizeof(default_hfr_video_sizes) / sizeof(cam_dimension_t)) ) {
      hfr_sensor_width[mode]  =
        default_hfr_video_sizes[matching_index].width;
      hfr_sensor_height[mode] =
        default_hfr_video_sizes[matching_index].height;
    } else {
      hfr_sensor_width[mode] = sensor_dim.width;
      hfr_sensor_height[mode] = sensor_dim.height;
    }
  }
}

/** mct_pipeline_pick_highest_fps:
 *    @src: input dimensions
 *    @ref_table: pointer to reference dim-fps table for matching
 *    @num_entries: number of entries in reference table
 *
 *  Description:
 *    Picks the sensor entry with the highest frame rate among
 *    entries with dimensions >= requested dimension.
 *
 *  Return: index of matching entry from reference table
 **/
static int8_t mct_pipeline_pick_highest_fps ( cam_dimension_t src_dim,
  mct_pipeline_sensor_res_table_t* ref_table, uint8_t num_entries )
{
  uint8_t index;
  int8_t matching_index = -1;
  float max_fps = 0.0;
  if(!ref_table || (0 == num_entries))
  {
    CLOGE(CAM_MCT_MODULE, "Invalid reference table");
    return matching_index;
  }

  for (index = 0; (index < num_entries && index < SENSOR_MAX_RESOLUTION);
    index++) {
    if ( ref_table[index].dim.width >= src_dim.width &&
      ref_table[index].dim.height >= src_dim.height ) {
      if (max_fps < ref_table[index].fps.max_fps) {
        max_fps = ref_table[index].fps.max_fps;
        matching_index = index;
      }
    }
  }

  if(matching_index != -1) {
    /* Valid match has been found */
    CLOGD(CAM_MCT_MODULE,
      "Found ref_table[%d]: res (%d x %d), fps (%f, %f) closest to (%d x %d)",
      matching_index,
      ref_table[matching_index].dim.width,
      ref_table[matching_index].dim.height,
      ref_table[matching_index].fps.min_fps,
      ref_table[matching_index].fps.max_fps,
      src_dim.width, src_dim.height);
  }
  else {
    CLOGD(CAM_MCT_MODULE, "No match found for src_dim [%d x %d]",
      src_dim.width, src_dim.height);
  }

  return matching_index;
}


/** mct_pipeline_append_fps_entry:
 *    @fps_entry: fps_range entry to be added to HAL table
 *    @hal_data: Top level ptr consisting of hal_fps_tbl
 *
 *  Description:
 *    Appends fps_entry to hal_fps_tbl ensuring no duplication.
 *    It also appends a constant-range fps entry corresponding
 *    to fps_entry ensuring no duplication.
 *
 *  Return: NONE
 *
 **/
static void mct_pipeline_append_fps_entry(const cam_fps_range_t fps_entry,
  cam_capability_t *hal_data)
{
  if (!hal_data) {
    CLOGE(CAM_MCT_MODULE, "Invalid hal_data ptr");
    return;
  }
  cam_fps_range_t *hal_fps_table = hal_data->fps_ranges_tbl;
  cam_fps_range_t const_fps;
  memset (&const_fps, 0, sizeof(cam_fps_range_t));

  /* Add non-duplicate entry into HAL table */
  if ( (hal_data->fps_ranges_tbl_cnt < MAX_SIZES_CNT) &&
    ( FALSE == mct_util_check_matching_fps (
    fps_entry, hal_data->fps_ranges_tbl,
    hal_data->fps_ranges_tbl_cnt) ) ) {
    hal_fps_table[hal_data->fps_ranges_tbl_cnt++] =
      fps_entry;
    CLOGI( CAM_MCT_MODULE, " Added hal_table[%d]: (%f, %f)",
      hal_data->fps_ranges_tbl_cnt - 1,
      hal_fps_table[hal_data->fps_ranges_tbl_cnt-1].min_fps,
      hal_fps_table[hal_data->fps_ranges_tbl_cnt-1].max_fps );

    /* Add non-duplicate constant-range fps entry if needed */
    const_fps.min_fps = const_fps.max_fps =
        (uint32_t) fps_entry.max_fps;
    if ( (hal_data->fps_ranges_tbl_cnt < MAX_SIZES_CNT) &&
      ( FALSE == mct_util_check_matching_fps (
      const_fps, hal_data->fps_ranges_tbl,
      hal_data->fps_ranges_tbl_cnt) ) ) {
      hal_fps_table[hal_data->fps_ranges_tbl_cnt++] = const_fps;
      CLOGI( CAM_MCT_MODULE, " Added hal_table[%d]: (%f, %f)",
        hal_data->fps_ranges_tbl_cnt - 1,
        hal_fps_table[hal_data->fps_ranges_tbl_cnt-1].min_fps,
        hal_fps_table[hal_data->fps_ranges_tbl_cnt-1].max_fps );
    }
  }
}

/** mct_pipeline_fill_fps_ranges:
 *    @pipeline: Pipeline pointer
 *    @mct_fps_table: MCT default fps entries table
 *    @mct_tbl_size: Size of MCT fps table
 *    @sensor_fps_table: A pre-filtered fps table published by sensor
 *    @sensor_tbl_size: Size of sensor fps table
 *    @cpp_fps_table: Constrained fps entries published by CPP
 *    @cpp_tbl_size: Size of CPP fps table
 *
 *  Description:
 *    This method does a 3-way merge of sorted input tables
 *    into the final hal fps table with sorting.
 *
 *  Return: TRUE upon successful completion of operations.
 *             FALSE for intermittent failures.
 **/

static boolean mct_pipeline_fill_fps_ranges (
  mct_pipeline_t *pipeline,
  cam_fps_range_t *mct_fps_table, uint8_t mct_tbl_size,
  cam_fps_range_t* sensor_fps_table, uint8_t sensor_tbl_size,
  cam_fps_range_t *cpp_fps_table, uint8_t cpp_tbl_size )
{
  cam_fps_range_t *hal_fps_table;
  cam_fps_range_t crucible[3]; /* Placeholder for sorting */
  /* Array size '3' represents a 3-way merge */
  boolean valid_match = FALSE;
  int8_t matching_index = -1;
  /* Index represents the positions of tables in crucible */
  /* 0->MCT, 1->Sensor, 2->CPP */
  uint8_t m = 0;  /* MCT table index */
  uint8_t s = 0;  /* Sensor table index */
  uint8_t c = 0;  /* CPP table index */

  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "NULL pipeline ptr");
    return FALSE;
  }
  cam_capability_t   *hal_data   = pipeline->query_buf;
  if (!hal_data) {
    CLOGE(CAM_MCT_MODULE, "NULL query_buf!");
    return FALSE;
  }
  hal_fps_table = hal_data->fps_ranges_tbl;
  hal_data->fps_ranges_tbl_cnt = 0;

  if (mct_fps_table) {
    crucible[0] = mct_fps_table[0];
  } else {
    memset (&crucible[0], 0, sizeof (cam_fps_range_t));
  }
  if (sensor_fps_table) {
    crucible[1] = sensor_fps_table[0];
  } else {
    memset (&crucible[1], 0, sizeof (cam_fps_range_t));
  }
  if (cpp_fps_table) {
    crucible[2] = cpp_fps_table[0];
  } else {
    memset (&crucible[2], 0, sizeof (cam_fps_range_t));
  }

#ifdef MCT_SORT_FPS_TABLE
  mct_util_sort_fps_entries(mct_fps_table, mct_tbl_size);
  if (sensor_tbl_size < MAX_FPS_RANGE_ENTRIES)
    mct_util_sort_fps_entries(sensor_fps_table, sensor_tbl_size);
#endif
  if (cpp_tbl_size < MAX_FPS_RANGE_ENTRIES)
    mct_util_sort_fps_entries(cpp_fps_table, cpp_tbl_size);

  do {
    valid_match = FALSE;
    /* Find the best lower fps range among the three table entries */
    matching_index = mct_util_pick_lowest_fps(crucible,
      sizeof(crucible) / sizeof (cam_fps_range_t));
    /* Add this entry into HAL fps table */
    if (matching_index > -1 && matching_index < FPS_SEL_MAX) {
      mct_pipeline_append_fps_entry (crucible[matching_index], hal_data);
    }

    switch (matching_index) {
      case FPS_SEL_MCT: {
        if ( (mct_fps_table) && (++m < mct_tbl_size) ) {
          crucible[matching_index] = mct_fps_table[m];
        } else {
          crucible[matching_index].min_fps = 0;
          crucible[matching_index].max_fps = 0;
        }
        valid_match = TRUE;
        break;
      }
      case FPS_SEL_SENSOR: {
        if ( (sensor_fps_table) && (++s < sensor_tbl_size) ) {
          crucible[matching_index] = sensor_fps_table[s];
        } else {
          crucible[matching_index].min_fps = 0;
          crucible[matching_index].max_fps = 0;
        }
        valid_match = TRUE;
        break;
      }
      case FPS_SEL_CPP: {
        if ( (cpp_fps_table) && (++c < cpp_tbl_size) ) {
          crucible[matching_index] = cpp_fps_table[c];
        } else {
          crucible[matching_index].min_fps = 0;
          crucible[matching_index].max_fps = 0;
        }
        valid_match = TRUE;
        break;
      }
      default:
        break;
    }
  } while (valid_match);

  return TRUE;
}

/** mct_pipeline_fill_caps_sensor:
 *    @pipeline: Pipeline pointer
 *
 *  Description:
 *    Populates capabilities queried from the sensor module
 *    into HAL query_buf.
 *
 *  Return: TRUE upon successful completion of operations.
 *             FALSE for intermittent failures.
 **/
static boolean mct_pipeline_fill_caps_sensor(mct_pipeline_t *pipeline)
{
  uint32_t i = 0, j = 0, k = 0;
  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Invalid pipeline ptr");
    return FALSE;
  }
  mct_pipeline_cap_t *local_data = &pipeline->query_data;
  cam_capability_t   *hal_data   = pipeline->query_buf;
  if (!hal_data) {
    CLOGE(CAM_MCT_MODULE, "NULL query_buf!");
    return FALSE;
  }

  hal_data->modes_supported     = local_data->sensor_cap.modes_supported;
  hal_data->sensor_mount_angle  = local_data->sensor_cap.sensor_mount_angle;
  hal_data->focal_length        = local_data->sensor_cap.focal_length;
  hal_data->hor_view_angle      = local_data->sensor_cap.hor_view_angle;
  hal_data->ver_view_angle      = local_data->sensor_cap.ver_view_angle;
  hal_data->max_roll_degrees    = local_data->sensor_cap.max_roll_degree;
  hal_data->max_pitch_degrees   = local_data->sensor_cap.max_pitch_degree;
  hal_data->max_yaw_degrees     = local_data->sensor_cap.max_yaw_degree;
  hal_data->pixel_pitch_um      = local_data->sensor_cap.pix_size;

  /* Set scale picture size */
  size_t scale_picture_sizes_cnt =
    MIN(local_data->sensor_cap.scale_picture_sizes_cnt, MAX_SCALE_SIZES_CNT);
  hal_data->scale_picture_sizes_cnt = scale_picture_sizes_cnt;
  memcpy(hal_data->scale_picture_sizes,
    local_data->sensor_cap.scale_picture_sizes,
    scale_picture_sizes_cnt * sizeof (cam_dimension_t));

  /* Set RAW formats */
  size_t supported_raw_fmt_cnt =
    MIN(local_data->sensor_cap.supported_raw_fmts_cnt, CAM_FORMAT_MAX);
  hal_data->supported_raw_fmt_cnt = supported_raw_fmt_cnt;
  memcpy(hal_data->supported_raw_fmts,
    local_data->sensor_cap.supported_raw_fmts,
    supported_raw_fmt_cnt * sizeof (cam_format_t));

  /* Set preview formats */
  size_t supported_preview_fmt_cnt =
    MIN(local_data->sensor_cap.supported_preview_fmt_cnt, CAM_FORMAT_MAX);
  hal_data->supported_preview_fmt_cnt = supported_preview_fmt_cnt;
  memcpy(hal_data->supported_preview_fmts,
    local_data->sensor_cap.supported_preview_fmts,
    supported_preview_fmt_cnt * sizeof (cam_format_t));

  /* Flash settings */
  if (local_data->sensor_cap.is_flash_supported) {
    hal_data->supported_flash_modes_cnt =
      local_data->stats_cap.supported_flash_modes_cnt;
    memcpy(hal_data->supported_flash_modes,
      local_data->stats_cap.supported_flash_modes,
       sizeof(cam_flash_mode_t) *
       MIN(hal_data->supported_flash_modes_cnt, CAM_FLASH_MODE_MAX));
    strlcpy((char *)&hal_data->flash_dev_name,
      local_data->sensor_cap.flash_dev_name,
      sizeof(hal_data->flash_dev_name));
  }
  hal_data->flash_available = local_data->sensor_cap.is_flash_supported;

  /* Support 8Bit RAW format by default */
  hal_data->rdi_mode_stream_fmt = local_data->sensor_cap.supported_raw_fmts[0];

  memcpy (&(hal_data->brightness_ctrl),
    &(local_data->sensor_cap.brightness_ctrl), sizeof (cam_control_range_t));
  memcpy (&(hal_data->sharpness_ctrl),
    &(local_data->sensor_cap.sharpness_ctrl), sizeof (cam_control_range_t));
  memcpy (&(hal_data->contrast_ctrl),
    &(local_data->sensor_cap.contrast_ctrl), sizeof (cam_control_range_t));
  memcpy (&(hal_data->saturation_ctrl),
    &(local_data->sensor_cap.saturation_ctrl), sizeof (cam_control_range_t));
  memcpy (&(hal_data->sce_ctrl),
    &(local_data->sensor_cap.sce_ctrl), sizeof (cam_control_range_t));

  /*Specific Capabilities for HAL 3*/
  hal_data->hyper_focal_distance  =
    local_data->sensor_cap.hyper_focal_distance;
  hal_data->focus_dist_calibrated =
    local_data->sensor_cap.focus_dist_calibrated;

  uint8_t focal_lengths_count =
    MIN(local_data->sensor_cap.focal_lengths_count, CAM_FOCAL_LENGTHS_MAX);
  hal_data->focal_lengths_count = focal_lengths_count;
  memcpy (hal_data->focal_lengths, local_data->sensor_cap.focal_lengths,
    focal_lengths_count * sizeof (float));

  uint8_t apertures_count =
    MIN(local_data->sensor_cap.apertures_count, CAM_APERTURES_MAX);
  hal_data->apertures_count = apertures_count;
  memcpy (hal_data->apertures, local_data->sensor_cap.apertures,
    apertures_count * sizeof (float));

  uint8_t filter_densities_count =
    MIN(local_data->sensor_cap.filter_densities_count, CAM_FILTER_DENSITIES_MAX);
  hal_data->filter_densities_count = filter_densities_count;
  memcpy (hal_data->filter_densities, local_data->sensor_cap.filter_densities,
    filter_densities_count * sizeof (float));

  uint8_t optical_stab_modes_count =
    MIN(local_data->sensor_cap.optical_stab_modes_count, CAM_OPT_STAB_MAX);
  hal_data->optical_stab_modes_count = optical_stab_modes_count;
  memcpy (hal_data->optical_stab_modes,
    local_data->sensor_cap.optical_stab_modes,
    optical_stab_modes_count * sizeof(cam_optical_stab_modes_t));

  hal_data->exposure_time_range[0] = local_data->sensor_cap.min_exp_time;
  hal_data->exposure_time_range[1] = local_data->sensor_cap.max_exp_time;
  hal_data->max_analog_sensitivity = local_data->sensor_cap.max_analog_gain;
  hal_data->sensitivity_range.max_sensitivity =
    local_data->sensor_cap.max_gain;
  hal_data->sensitivity_range.min_sensitivity =
    local_data->sensor_cap.min_gain;

  hal_data->max_frame_duration = local_data->sensor_cap.max_frame_duration;
  hal_data->color_arrangement = local_data->sensor_cap.color_arrangement;
  hal_data->num_color_channels = local_data->sensor_cap.num_color_channels;

  memcpy (hal_data->sensor_physical_size,
    local_data->sensor_cap.sensor_physical_size,
    SENSOR_PHYSICAL_SIZE_CNT * sizeof (float));
  hal_data->pixel_array_size = local_data->sensor_cap.pixel_array_size;

  hal_data->active_array_size = local_data->sensor_cap.active_array_size;

  hal_data->max_depth_points =
    local_data->sensor_cap.active_array_size.width *
    local_data->sensor_cap.active_array_size.height;

  hal_data->white_level = local_data->sensor_cap.white_level;

  memcpy(hal_data->black_level_pattern,
    local_data->sensor_cap.black_level_pattern,
    BLACK_LEVEL_PATTERN_CNT * sizeof (uint32_t));

  hal_data->flash_charge_duration =
    local_data->sensor_cap.flash_charge_duration;

  switch (local_data->sensor_cap.sensor_format ) {
    case FORMAT_BAYER:
      hal_data->sensor_type.sens_type = CAM_SENSOR_RAW;
      break;
    case FORMAT_YCBCR:
      hal_data->sensor_type.sens_type = CAM_SENSOR_YUV;
      break;
    case FORMAT_GREY:
      hal_data->sensor_type.sens_type = CAM_SENSOR_MONO;
      break;
    default:
      break;
  }

  /* Add supported flash firing levels */
  hal_data->supported_flash_firing_level_cnt = CAM_FLASH_FIRING_LEVEL_MAX;
  for (i = 0; i < CAM_FLASH_FIRING_LEVEL_MAX; i++)
    hal_data->supported_firing_levels[i] = i;

  hal_data->auto_hdr_supported = local_data->sensor_cap.sensor_format ==
    FORMAT_BAYER;

  hal_data->reference_illuminant1 =
    local_data->sensor_cap.reference_illuminant1;
  hal_data->reference_illuminant2 =
    local_data->sensor_cap.reference_illuminant2;

  /* DNG data */
  memcpy(hal_data->forward_matrix1,
    local_data->sensor_cap.forward_transform_D65,
    sizeof(cam_color_correct_matrix_t));
  memcpy(hal_data->forward_matrix2,
    local_data->sensor_cap.forward_transform_A,
    sizeof(cam_color_correct_matrix_t));

  memcpy(hal_data->color_transform1,
    local_data->sensor_cap.color_transform_D65,
    sizeof(cam_color_correct_matrix_t));
  memcpy(hal_data->color_transform2,
    local_data->sensor_cap.color_transform_A,
    sizeof(cam_color_correct_matrix_t));

  memcpy(hal_data->calibration_transform1,
    local_data->sensor_cap.calibration_transform_D65,
    sizeof(cam_color_correct_matrix_t));
  memcpy(hal_data->calibration_transform2,
    local_data->sensor_cap.calibration_transform_A,
    sizeof(cam_color_correct_matrix_t));
  memcpy(&hal_data->base_gain_factor,
    &local_data->sensor_cap.base_gain_factor,
    sizeof(cam_rational_type_t));
  memcpy(&hal_data->related_cam_calibration,
    &local_data->sensor_cap.dualcam_tune_data,
    sizeof(cam_related_system_calibration_data_t));
  /* Populate supported test pattern */
  uint8_t supported_test_pattern_modes_cnt =
    MIN(local_data->sensor_cap.supported_test_pattern_modes_cnt,
    MAX_TEST_PATTERN_CNT);
  hal_data->supported_test_pattern_modes_cnt=supported_test_pattern_modes_cnt;
  memcpy(hal_data->supported_test_pattern_modes,
    local_data->sensor_cap.supported_test_pattern_modes,
    supported_test_pattern_modes_cnt * sizeof (cam_test_pattern_mode_t));

  /* noise profile co-efficient */
  hal_data->gradient_S = local_data->sensor_cap.gradient_S;
  hal_data->offset_S = local_data->sensor_cap.offset_S;
  hal_data->gradient_O = local_data->sensor_cap.gradient_O;
  hal_data->offset_O = local_data->sensor_cap.offset_O;

  /* copy black regions data */
  hal_data->optical_black_region_count =
    local_data->sensor_cap.sensor_optical_black_region_count;
  if (hal_data->optical_black_region_count > MAX_OPTICAL_BLACK_REGIONS) {
      CLOGW(CAM_MCT_MODULE, "limiting sensor black_region_count to max bound");
      hal_data->optical_black_region_count = MAX_OPTICAL_BLACK_REGIONS;
  }
  for (i = 0, j = 0; i < hal_data->optical_black_region_count; i++) {
    hal_data->optical_black_regions[j++] =
      local_data->sensor_cap.sensor_optical_black_region[i].left;
    hal_data->optical_black_regions[j++] =
      local_data->sensor_cap.sensor_optical_black_region[i].top;
    hal_data->optical_black_regions[j++] =
      local_data->sensor_cap.sensor_optical_black_region[i].width;
    hal_data->optical_black_regions[j++] =
      local_data->sensor_cap.sensor_optical_black_region[i].height;
  }
  /*Updating capabilities for Raw Meta channel */
  hal_data->meta_raw_channel_count = local_data->sensor_cap.num_meta_channels;
  for (i = 0; i < hal_data->meta_raw_channel_count; i++) {
    hal_data->supported_meta_raw_fmts[i] =
      local_data->sensor_cap.meta_chnl_info[i].fmt;
    hal_data->raw_meta_dim[i].height =
      local_data->sensor_cap.meta_chnl_info[i].height;
    hal_data->raw_meta_dim[i].width =
      local_data->sensor_cap.meta_chnl_info[i].width;
    hal_data->vc[i] = local_data->sensor_cap.meta_chnl_info[i].vc;
    hal_data->dt[i] = local_data->sensor_cap.meta_chnl_info[i].dt;
    hal_data->sub_fmt[i] =
      local_data->sensor_cap.meta_chnl_info[i].sub_fmt;
  }

  hal_data->sensor_rotation = local_data->sensor_cap.sensor_rotation;

  hal_data->is_depth_sensor = local_data->sensor_cap.is_depth_sensor;

  return TRUE;
}

/** mct_pipeline_fill_caps_isp:
 *    @pipeline: Pipeline pointer
 *
 *  Description:
 *    Populates capabilities queried from the isp module
 *    into HAL query_buf.
 *
 *  Return: TRUE upon successful completion of operations.
 *             FALSE for intermittent failures.
 **/
static boolean mct_pipeline_fill_caps_isp(mct_pipeline_t *pipeline)
{
 uint32_t i = 0, j = 0;
 bool entry_available;
  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Invalid pipeline ptr");
    return FALSE;
  }
  mct_pipeline_cap_t *local_data = &pipeline->query_data;
  cam_capability_t   *hal_data   = pipeline->query_buf;
  if (!hal_data) {
    CLOGE(CAM_MCT_MODULE, "NULL query_buf!");
    return FALSE;
  }
  hal_data->max_downscale_factor = local_data->isp_cap.max_downscale_factor;

  /*Zoom support*/
  if (local_data->isp_cap.zoom_ratio_tbl_cnt > 0) {
    hal_data->zoom_supported = TRUE;
    hal_data->zoom_ratio_tbl_cnt = local_data->isp_cap.zoom_ratio_tbl_cnt;
    for (i = 0; (i < (local_data->isp_cap.zoom_ratio_tbl_cnt)
      && i < MAX_ZOOMS_CNT); i++) {
      hal_data->zoom_ratio_tbl[i] = local_data->isp_cap.zoom_ratio_tbl[i];
    }
  } else {
    hal_data->zoom_supported = FALSE;
  }

  hal_data->isp_sensitivity_range.min_sensitivity =
    local_data->isp_cap.isp_sensitivity_range.min_sensitivity;

  hal_data->isp_sensitivity_range.max_sensitivity =
    local_data->isp_cap.isp_sensitivity_range.max_sensitivity;

  hal_data->lens_shading_map_size = local_data->isp_cap.lens_shading_map_size;

  hal_data->max_tone_map_curve_points =
    local_data->isp_cap.max_gamma_curve_points;

  hal_data->max_viewfinder_size =
    local_data->isp_cap.max_viewfinder_size;


  // If dual camera, calculate optimal max snapshot size such that
  // HAL requested snapshot size < sensor mode < vfe max to ensure
  // single VFE usage per each session.

  CLOGI(CAM_MCT_MODULE, "isp max encoder size : %d X %d",
    local_data->isp_cap.max_encoder_size.width,local_data->isp_cap.max_encoder_size.height);
  for (i = 0; (i < local_data->sensor_cap.dim_fps_table_count)
    && (i < SENSOR_MAX_RESOLUTION); i++) {
    if (local_data->sensor_cap.dim_fps_table[i].dim.width <= local_data->isp_cap.max_encoder_size.width) {
      hal_data->single_isp_max_size.width = local_data->sensor_cap.dim_fps_table[i].dim.width;
      hal_data->single_isp_max_size.height= local_data->sensor_cap.dim_fps_table[i].dim.height;
      CLOGI(CAM_MCT_MODULE, "single isp max size : %d X %d",
        hal_data->single_isp_max_size.width,hal_data->single_isp_max_size.height);
      break;
    }
  }

  for (i = 0;(i < (sizeof(default_picture_sizes) / sizeof(cam_dimension_t))
    && i < MAX_SIZES_CNT);  i++ ) {
    if (default_picture_sizes[i].width <= hal_data->single_isp_max_size.width &&
      default_picture_sizes[i].height <= hal_data->single_isp_max_size.height) {
      hal_data->single_isp_max_size = default_picture_sizes[i];
      CLOGI(CAM_MCT_MODULE, "HAL max snapshot size in dual cam mode : %d X %d",
        hal_data->single_isp_max_size.width,hal_data->single_isp_max_size.height);
    break;
    }
  }

  hal_data->histogram_size = local_data->isp_cap.histogram_size;
  hal_data->max_histogram_count = local_data->isp_cap.max_histogram_count;
  /*Histogram ONLY support the Bayer sensor*/
  hal_data->histogram_supported = local_data->sensor_cap.sensor_format ==
    FORMAT_BAYER;
  hal_data->is_mono_stats_suport = local_data->isp_cap.is_mono_stats_support;
  /*Add Aberration modes. Step 1: PPROC modes */
  hal_data->aberration_modes_count = local_data->pp_cap.aberration_modes_count;
  memcpy(hal_data->aberration_modes, local_data->pp_cap.aberration_modes,
    sizeof(cam_aberration_mode_t) *
    MIN(hal_data->aberration_modes_count,
       CAM_COLOR_CORRECTION_ABERRATION_MAX));

  /*Copy non-duplicated Aberration modes from ISP */
  for (i= 0; (i < (local_data->isp_cap.aberration_modes_count)
    && i < CAM_COLOR_CORRECTION_ABERRATION_MAX); i++ ) {
      entry_available = FALSE;
      for (j =0; (j < (hal_data->aberration_modes_count) &&
        j < CAM_COLOR_CORRECTION_ABERRATION_MAX); j++ ) {
        if (local_data->isp_cap.aberration_modes[i] ==
          hal_data->aberration_modes[j]) {
          entry_available = TRUE;
          break;
        }
      }
      if (!entry_available) {
        hal_data->aberration_modes[hal_data->aberration_modes_count++] =
          local_data->isp_cap.aberration_modes[i];
      }
  }
  return TRUE;
}

/** mct_pipeline_fill_caps_stats:
 *    @pipeline: Pipeline pointer
 *
 *  Description:
 *    Populates capabilities queried from the stats module
 *    into HAL query_buf.
 *
 *  Return: TRUE upon successful completion of operations.
 *             FALSE for intermittent failures.
 **/
static boolean mct_pipeline_fill_caps_stats(mct_pipeline_t *pipeline)
{
  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Invalid pipeline ptr");
    return FALSE;
  }
  mct_pipeline_cap_t *local_data = &pipeline->query_data;
  cam_capability_t   *hal_data   = pipeline->query_buf;
  if (!hal_data) {
    CLOGE(CAM_MCT_MODULE, "NULL query_buf!");
    return FALSE;
  }

  /* 3A version info*/
  hal_data->q3a_version = local_data->stats_cap.q3a_version;
  /* EC info */
  hal_data->exposure_compensation_min =
    local_data->stats_cap.exposure_compensation_min;
  hal_data->exposure_compensation_max =
    local_data->stats_cap.exposure_compensation_max;
  hal_data->exposure_compensation_default =
    local_data->stats_cap.exposure_compensation_default;
  hal_data->exposure_compensation_step =
    local_data->stats_cap.exposure_compensation_step;
  hal_data->exp_compensation_step.numerator =
    local_data->stats_cap.exp_compensation_step.numerator;
  hal_data->exp_compensation_step.denominator =
    local_data->stats_cap.exp_compensation_step.denominator;

  /* Antibanding info */
  hal_data->supported_antibandings_cnt =
    local_data->stats_cap.supported_antibandings_cnt;
  memcpy(hal_data->supported_antibandings,
    local_data->stats_cap.supported_antibandings,
    sizeof(cam_antibanding_mode_type) *
    MIN(hal_data->supported_antibandings_cnt, CAM_ANTIBANDING_MODE_MAX));

  if (local_data->sensor_cap.ae_lock_supported)
    hal_data->auto_exposure_lock_supported =
      local_data->stats_cap.auto_exposure_lock_supported;

  if (local_data->sensor_cap.wb_lock_supported)
    hal_data->auto_wb_lock_supported =
      local_data->stats_cap.auto_wb_lock_supported;

  hal_data->supported_iso_modes_cnt =
    local_data->stats_cap.supported_iso_modes_cnt;
  memcpy(hal_data->supported_iso_modes,
    local_data->stats_cap.supported_iso_modes,
    sizeof(cam_iso_mode_type) *
    MIN(hal_data->supported_iso_modes_cnt, CAM_ISO_MODE_MAX));

  hal_data->supported_aec_modes_cnt =
    local_data->stats_cap.supported_aec_modes_cnt;
  memcpy(hal_data->supported_aec_modes,
    local_data->stats_cap.supported_aec_modes,
     sizeof(cam_auto_exposure_mode_type) *
     MIN(hal_data->supported_aec_modes_cnt, CAM_AEC_MODE_MAX));

  hal_data->supported_instant_aec_modes_cnt =
    local_data->stats_cap.supported_instant_aec_modes_cnt;
  memcpy(hal_data->supported_instant_aec_modes,
    local_data->stats_cap.supported_instant_aec_modes,
     sizeof(cam_aec_convergence_type) *
     MIN(hal_data->supported_instant_aec_modes_cnt, CAM_AEC_CONVERGENCE_MAX));

  hal_data->supported_ae_modes_cnt =
    local_data->stats_cap.supported_ae_modes_cnt;
  memcpy(hal_data->supported_ae_modes,
    local_data->stats_cap.supported_ae_modes,
    sizeof(hal_data->supported_ae_modes));

  if (!local_data->sensor_cap.af_supported) {
    hal_data->supported_focus_modes_cnt = 1;
    hal_data->supported_focus_modes[0] = CAM_FOCUS_MODE_FIXED;
    hal_data->max_num_focus_areas = 0;
    hal_data->min_focus_distance = 0;
  } else {
    hal_data->supported_focus_modes_cnt =
      local_data->stats_cap.supported_focus_modes_cnt;
    memcpy(hal_data->supported_focus_modes,
      local_data->stats_cap.supported_focus_modes,
      sizeof(cam_focus_mode_type) * MIN(hal_data->supported_focus_modes_cnt,
      CAM_FOCUS_MODE_MAX));
    hal_data->max_num_focus_areas =
    local_data->stats_cap.max_num_focus_areas;
    hal_data->min_focus_distance = local_data->sensor_cap.min_focus_distance;
  }

  hal_data->supported_focus_algos_cnt =
    local_data->stats_cap.supported_focus_algos_cnt;
  memcpy(hal_data->supported_focus_algos,
    local_data->stats_cap.supported_focus_algos,
     sizeof(cam_focus_algorithm_type) *
     MIN(hal_data->supported_focus_algos_cnt,CAM_FOCUS_ALGO_MAX));

  hal_data->supported_white_balances_cnt =
    local_data->stats_cap.supported_white_balances_cnt;
  memcpy(hal_data->supported_white_balances,
    local_data->stats_cap.supported_white_balances,
     sizeof(cam_wb_mode_type) *
     MIN(hal_data->supported_white_balances_cnt,CAM_WB_MODE_MAX));

  hal_data->max_num_metering_areas =
    local_data->stats_cap.max_num_metering_areas;

  /* IS */
  hal_data->supported_is_types_cnt =
    local_data->stats_cap.supported_is_types_cnt;
  memcpy(hal_data->supported_is_types,
    local_data->stats_cap.supported_is_types,
    sizeof(uint32_t) * MIN(hal_data->supported_is_types_cnt, IS_TYPE_MAX));
  memcpy(hal_data->supported_is_type_margins,
    local_data->stats_cap.supported_is_type_margins,
    sizeof(float) * MIN(hal_data->supported_is_types_cnt, IS_TYPE_MAX));
  return TRUE;
}

/** mct_pipeline_fill_caps_pproc:
 *    @pipeline: Pipeline pointer
 *
 *  Description:
 *    Populates capabilities queried from the pproc module
 *    into HAL query_buf.
 *
 *  Return: TRUE upon successful completion of operations.
 *             FALSE for intermittent failures.
 **/
static boolean mct_pipeline_fill_caps_pproc(mct_pipeline_t *pipeline)
{
  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Invalid pipeline ptr");
    return FALSE;
  }
  mct_pipeline_cap_t *local_data = &pipeline->query_data;
  cam_capability_t   *hal_data   = pipeline->query_buf;
  if (!hal_data) {
    CLOGE(CAM_MCT_MODULE, "NULL query_buf!");
    return FALSE;
  }
  hal_data->padding_info.width_padding  = local_data->pp_cap.width_padding;
  hal_data->padding_info.height_padding = local_data->pp_cap.height_padding;
  CLOGD(CAM_MCT_MODULE,
    "[PADDING]  w_p %d, h_p %d", hal_data->padding_info.width_padding,
    hal_data->padding_info.height_padding);

  hal_data->min_num_pp_bufs =
    local_data->pp_cap.min_num_pp_bufs;
  hal_data->max_batch_bufs_supported =
    local_data->pp_cap.max_supported_pp_batch_size;

  hal_data->sharpness_map_size  = local_data->pp_cap.sharpness_map_size;
  hal_data->max_sharpness_map_value =
    local_data->pp_cap.max_sharpness_map_value;
  hal_data->quadra_cfa_format = local_data->pp_cap.quadra_cfa_format;

  hal_data->supported_binning_correction_mode_cnt =
    local_data->pp_cap.supported_binning_correction_mode_cnt;
  memcpy(hal_data->supported_binning_modes,
    local_data->pp_cap.bincorr_modes,
    sizeof(cam_aberration_mode_t) *
    MIN(hal_data->supported_binning_correction_mode_cnt,
      CAM_BINNING_CORRECTION_MODE_MAX));

  return TRUE;
}

/** mct_pipeline_fill_caps_imaging:
 *    @pipeline: Pipeline pointer
 *
 *  Description:
 *    Populates capabilities queried from the imglib module
 *    into HAL query_buf.
 *
 *  Return: TRUE upon successful completion of operations.
 *             FALSE for intermittent failures.
 **/
static boolean mct_pipeline_fill_caps_imaging(mct_pipeline_t *pipeline)
{
  int index;

  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Invalid pipeline ptr");
    return FALSE;
  }
  mct_pipeline_cap_t *local_data = &pipeline->query_data;
  cam_capability_t   *hal_data   = pipeline->query_buf;
  if (!hal_data) {
    CLOGE(CAM_MCT_MODULE, "NULL query_buf!");
    return FALSE;
  }

  /* add face ROI to HAL API */
  hal_data->max_num_roi = local_data->imaging_cap.max_num_roi;

  /* add BSGC availability to HAL */
  hal_data->face_bsgc = local_data->imaging_cap.face_bsgc;

  /* add HDR capability details to HAL */
  memcpy(&hal_data->hdr_bracketing_setting,
    &local_data->imaging_cap.hdr_bracketing_setting,
    sizeof(cam_hdr_bracketing_info_t));

  /* add Ubifocus capability details to HAL */
  memcpy(&hal_data->ubifocus_af_bracketing_need,
    &local_data->imaging_cap.ubifocus_af_bracketing_need,
    sizeof(cam_af_bracketing_t));
  /* add refocus capability details to HAL */
  memcpy(&hal_data->refocus_af_bracketing_need,
    &local_data->imaging_cap.refocus_af_bracketing_need,
    sizeof(cam_af_bracketing_t));

  /* add Optizoom capability details to HAL */
  memcpy(&hal_data->opti_zoom_settings_need,
    &local_data->imaging_cap.opti_zoom_settings,
    sizeof(cam_opti_zoom_t));

  /* add Chroma flash capability details to HAL */
  memcpy(&hal_data->chroma_flash_settings_need,
    &local_data->imaging_cap.chroma_flash_settings_need,
    sizeof(cam_chroma_flash_t));

  /* add trueportrait capability details to HAL */
  memcpy(&hal_data->true_portrait_settings_need,
    &local_data->imaging_cap.true_portrait_settings,
    sizeof(cam_true_portrait_t));

  /* add StillMore capability details to HAL */
  memcpy(&hal_data->stillmore_settings_need,
    &local_data->imaging_cap.stillmore_settings,
    sizeof(cam_still_more_t));

  hal_data->max_face_detection_count =
    local_data->imaging_cap.max_face_detection_count;

  /*Add analysis stream info*/
  for (index = 0; index < CAM_ANALYSIS_INFO_MAX; index++) {
    hal_data->analysis_info[index] =
      local_data->imaging_cap.analysis_info[index];
  }

  /* Add Spatial Alignment soln type */
  hal_data->avail_spatial_align_solns =
    local_data->imaging_cap.avail_spatial_align_solns;
  return TRUE;
}

/** mct_pipeline_fill_caps_common:
 *    @pipeline: Pipeline pointer
 *
 *  Description:
 *    Populates some common capabilities queried across modules
 *    into HAL query_buf.
 *
 *  Return: TRUE upon successful completion of operations.
 *             FALSE for intermittent failures.
 **/
static boolean mct_pipeline_fill_caps_common(mct_pipeline_t *pipeline)
{
  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Invalid pipeline ptr");
    return FALSE;
  }
  mct_pipeline_cap_t *local_data = &pipeline->query_data;
  cam_capability_t   *hal_data   = pipeline->query_buf;
  if (!hal_data) {
    CLOGE(CAM_MCT_MODULE, "NULL query_buf!");
    return FALSE;
  }

  /* Update the offset info */
  hal_data->padding_info.offset_info.offset_x =
    local_data->common_cap.offset_info.offset_x;
  hal_data->padding_info.offset_info.offset_y =
    local_data->common_cap.offset_info.offset_y;
  hal_data->padding_info.plane_padding =
    local_data->common_cap.plane_padding;

  CLOGD(CAM_MCT_MODULE, "QUERY_CAP - offset [x,y]: [%d,%d] plane %d",
    local_data->common_cap.offset_info.offset_x,
    local_data->common_cap.offset_info.offset_y,
    local_data->common_cap.plane_padding);

  /* Update stride and scanline info */
  hal_data->min_stride    = local_data->common_cap.min_stride;
  hal_data->min_scanline  = local_data->common_cap.min_scanline;
  hal_data->buf_alignment = local_data->common_cap.buf_alignment;

  CLOGD(CAM_MCT_MODULE, "buf_alignment=%d stride X scan=%dx%d",
    hal_data->buf_alignment, hal_data->min_stride, hal_data->min_scanline);
  return TRUE;
}

/** mct_pipeline_fill_supported_effects:
 *    @pipeline: Pipeline pointer
 *
 *  Description:
 *    Populates supported effects field in HAL query_buf
 *    from different back-end modules.
 *
 *  Return: TRUE upon successful completion of operations.
 *             FALSE for intermittent failures.
 **/
static boolean mct_pipeline_fill_supported_effects(mct_pipeline_t *pipeline)
{
  uint32_t i = 0, j = 0;
  bool common = FALSE;
  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Invalid pipeline ptr");
    return FALSE;
  }
  mct_pipeline_cap_t *local_data = &pipeline->query_data;
  cam_capability_t   *hal_data   = pipeline->query_buf;
  if (!hal_data) {
    CLOGE(CAM_MCT_MODULE, "NULL query_buf!");
    return FALSE;
  }
  hal_data->supported_effects_cnt = 0;
  if (local_data->sensor_cap.sensor_format == FORMAT_BAYER) {
    /* Use supported_effects from ISP for Bayer sensor */
    for (i = 0; (i < (local_data->isp_cap.supported_effects_cnt)
      && i < CAM_EFFECT_MODE_MAX); i++) {
      hal_data->supported_effects[hal_data->supported_effects_cnt] =
        local_data->isp_cap.supported_effects[i];
         hal_data->supported_effects_cnt++;
    }
  } else {
    /* Use supported_effects from sensor for YUV sensor */
    for (i = 0; i < CAM_EFFECT_MODE_MAX; ++i) {
      if (local_data->sensor_cap.sensor_supported_effect_modes & (1 << i)) {
        hal_data->supported_effects[hal_data->supported_effects_cnt] = i;
        hal_data->supported_effects_cnt ++;
      }
    }
  }
  /* Use unique supported_effects from CPP */
  for (j = 0; j < local_data->pp_cap.supported_effects_cnt; j++) {
    common = FALSE;
    for (i = 0; i < hal_data->supported_effects_cnt; i++) {
      if (hal_data->supported_effects[i] ==
        local_data->pp_cap.supported_effects[j]) {
        common = TRUE;
        break;
      }
    }
    if (common == TRUE)
      continue;

    if (hal_data->supported_effects_cnt < CAM_EFFECT_MODE_MAX) {
      hal_data->supported_effects[hal_data->supported_effects_cnt] =
        local_data->pp_cap.supported_effects[j];
         hal_data->supported_effects_cnt++;
    } else {
      break;    /* Reached limit on supported_effects[] */
    }
  }
  return TRUE;
}

/** mct_pipeline_fill_supported_scene_modes:
 *    @pipeline: Pipeline pointer
 *
 *  Description:
 *    Populates supported scene modes in HAL query_buf
 *    from different back-end modules.
 *
 *  Return: TRUE upon successful completion of operations.
 *             FALSE for intermittent failures.
 **/
static boolean mct_pipeline_fill_supported_scene_modes(mct_pipeline_t *pipeline)
{
  uint32_t i = 0, j = 0;
    if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Invalid pipeline ptr");
    return FALSE;
  }
  mct_pipeline_cap_t *local_data = &pipeline->query_data;
  cam_capability_t   *hal_data   = pipeline->query_buf;
  if (!hal_data) {
    CLOGE(CAM_MCT_MODULE, "NULL query_buf!");
    return FALSE;
  }

  /* Add supported scene modes */
  if (local_data->sensor_cap.scene_mode_supported) {
    hal_data->supported_scene_modes_cnt =
      local_data->stats_cap.supported_scene_modes_cnt;
    memcpy(hal_data->supported_scene_modes,
      local_data->stats_cap.supported_scene_modes,
      sizeof(cam_scene_mode_type) *
      MIN(hal_data->supported_scene_modes_cnt, CAM_SCENE_MODE_MAX));
    /*add scene mode overrides*/
    memcpy(hal_data->scene_mode_overrides,
      local_data->stats_cap.scene_mode_overrides,
      sizeof(cam_scene_mode_overrides_t) *
      MIN(hal_data->supported_scene_modes_cnt, CAM_SCENE_MODE_MAX));

    if ((local_data->imaging_cap.supported_scene_modes_cnt > 0) &&
      ((hal_data->supported_scene_modes_cnt +
        local_data->imaging_cap.supported_scene_modes_cnt) <
        CAM_SCENE_MODE_MAX)) {
      for (i = 0; i < local_data->imaging_cap.supported_scene_modes_cnt; i++) {
        j = hal_data->supported_scene_modes_cnt + i;
        hal_data->supported_scene_modes[j] =
          local_data->imaging_cap.supported_scene_modes[i];
        hal_data->scene_mode_overrides[j] =
          local_data->imaging_cap.scene_mode_overrides[i];
      }
      hal_data->supported_scene_modes_cnt +=
        local_data->imaging_cap.supported_scene_modes_cnt;
    }
  } else {
    for (i = 0; i < CAM_SCENE_MODE_MAX; ++i) {
      if (local_data->sensor_cap.sensor_supported_scene_modes & (1 << i)) {
        hal_data->supported_scene_modes[hal_data->supported_scene_modes_cnt]
           = i;
        hal_data->supported_scene_modes_cnt ++;
      }
    }
    memcpy(hal_data->scene_mode_overrides,
      local_data->stats_cap.scene_mode_overrides,
      sizeof(local_data->stats_cap.scene_mode_overrides));
  }

  return TRUE;
}

/** mct_pipeline_fill_dimensions_preview:
 *    @pipeline: Pipeline pointer
 *    @prev_sensor_width, prev_sensor_height: Max preview dimensions
 *    @snap_sensor_width, snap_sensor_height: Max snapshot dimensions
 *
 *  Description:
 *    Populates video dimensions in HAL query_buf
 *
 *  Return: TRUE upon successful completion of operations.
 *             FALSE for intermittent failures.
 **/
static boolean mct_pipeline_fill_dimensions_preview (
  mct_pipeline_t *pipeline,
  const int32_t prev_sensor_width, const int32_t prev_sensor_height,
  const int32_t snap_sensor_width, const int32_t snap_sensor_height )
{
  uint32_t i = 0;
  int enable_max_preview = 0;
  char value[PROPERTY_VALUE_MAX];
  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Invalid pipeline ptr");
    return FALSE;
  }
  cam_capability_t   *hal_data   = pipeline->query_buf;
  if (!hal_data) {
    CLOGE(CAM_MCT_MODULE, "NULL query_buf!");
    return FALSE;
  }
#ifdef _ANDROID_
    property_get("persist.vendor.camera.max_prev.enable", value, "0");
    enable_max_preview = atoi(value);
    if (enable_max_preview) {
      CLOGI(CAM_MCT_MODULE,
        "camera max preview enabled (persist.vendor.camera.max_prev.enable = 1)");
    }
#endif

  hal_data->preview_sizes_tbl_cnt = 0;
  if(enable_max_preview) {
    int32_t max_preview_width = 0;
    int32_t max_preview_height = 0;

    for(i = 0;
        i < (sizeof(default_picture_sizes) / sizeof(cam_dimension_t));
        i++) {
      if((default_picture_sizes[i].width > max_preview_width ||
          default_picture_sizes[i].height > max_preview_height) &&
         (default_picture_sizes[i].width <= snap_sensor_width &&
          default_picture_sizes[i].height <= snap_sensor_height)) {
        max_preview_width = default_picture_sizes[i].width;
        max_preview_height = default_picture_sizes[i].height;
      }
    }

    hal_data->preview_sizes_tbl[hal_data->preview_sizes_tbl_cnt].width =
        max_preview_width;
    hal_data->preview_sizes_tbl[hal_data->preview_sizes_tbl_cnt].height =
        max_preview_height;
    hal_data->preview_sizes_tbl_cnt++;
  }
  for (i = hal_data->preview_sizes_tbl_cnt;
       (i < (sizeof(default_preview_sizes) / sizeof(cam_dimension_t))
          && i < MAX_SIZES_CNT);
       i++) {
    if (default_preview_sizes[i].width <= prev_sensor_width &&
      default_preview_sizes[i].height <= prev_sensor_height) {
      hal_data->preview_sizes_tbl[hal_data->preview_sizes_tbl_cnt] =
        default_preview_sizes[i];
      hal_data->preview_sizes_tbl_cnt++;
    }
  }
  return TRUE;
}

/** mct_pipeline_fill_dimensions_snapshot:
 *    @pipeline: Pipeline pointer
 *    @snap_sensor_width, snap_sensor_height: Snapshot dimensions
 *
 *  Description:
 *    Populates snapshot dimensions in HAL query_buf
 *    Also fills jpeg stall durations and min frame_durations
 *    for each of these picture sizes.
 *    This method also fills FPS entries from the intermediate computations.
 *
 *  Return: TRUE upon successful completion of operations.
 *             FALSE for intermittent failures.
 **/
static boolean mct_pipeline_fill_dimensions_snapshot (
  mct_pipeline_t *pipeline,
  const int32_t snap_sensor_width, const int32_t snap_sensor_height )
{
  uint32_t i = 0;
  int8_t matching_index = -1;
  int8_t  matching_mct_index = -1;
  uint8_t default_fps_tbl_size = 0;
  mct_pipeline_sensor_res_table_t sensor_dim_fps_table[SENSOR_MAX_RESOLUTION];
  cam_fps_range_t mct_fps_table[MAX_FPS_RANGE_ENTRIES];
  uint8_t mct_table_index = 0;
  cam_fps_range_t cpp_fps_table[MAX_FPS_RANGE_ENTRIES];
  uint8_t cpp_table_index = 0;
  cam_fps_range_t temp_cpp_fps;
  cam_fps_range_t sensor_fps_table[MAX_FPS_RANGE_ENTRIES];
  uint8_t sensor_table_index = 0;
  char fps_str[PROPERTY_VALUE_MAX];
  float preview_max_fps = 0.0;
  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Invalid pipeline ptr");
    return FALSE;
  }
  mct_pipeline_cap_t *local_data = &pipeline->query_data;
  cam_capability_t   *hal_data   = pipeline->query_buf;
  if (!hal_data) {
    CLOGE(CAM_MCT_MODULE, "NULL query_buf!");
    return FALSE;
  }
  if (local_data->sensor_cap.dim_fps_table_count > SENSOR_MAX_RESOLUTION) {
    CLOGE(CAM_MCT_MODULE, "dim_fps_table_count %d > %d",
      local_data->sensor_cap.dim_fps_table_count, SENSOR_MAX_RESOLUTION);
    return FALSE;
  }

  /*Determine max preview fps supported on device */
  memset(fps_str, 0, PROPERTY_VALUE_MAX);
  property_get("persist.vendor.camera.max.previewfps", fps_str, "30");
  preview_max_fps = (float)atoi(fps_str);
  CLOGD(CAM_MCT_MODULE, "preview_max_fps = %f", preview_max_fps);

  /*Fill picture sizes and correponding frame durations */
  hal_data->picture_sizes_tbl_cnt = 0;
  memset(sensor_dim_fps_table, 0,
    SENSOR_MAX_RESOLUTION * sizeof(mct_pipeline_sensor_res_table_t));
  /* Extract sensor_caps dimensions into sensor_cap_dim_table */
  for (i = 0; i < local_data->sensor_cap.dim_fps_table_count; i++) {
    memcpy(&sensor_dim_fps_table[i],
    &local_data->sensor_cap.dim_fps_table[i],
    sizeof(mct_pipeline_sensor_res_table_t));
  }
  memset(cpp_fps_table, 0, MAX_FPS_RANGE_ENTRIES * sizeof(cam_fps_range_t));
  default_fps_tbl_size = sizeof(default_fps_ranges) / sizeof(cam_fps_range_t);
  memset (&temp_cpp_fps, 0, sizeof(cam_fps_range_t));
  for (i = 0;
       (i < (sizeof(default_picture_sizes) / sizeof(cam_dimension_t))
          && i < MAX_SIZES_CNT);
       i++) {
    /* Ignore picture sizes not supported by sensor */
    if (snap_sensor_width <
      default_picture_sizes[i].width ||
      snap_sensor_height <
      default_picture_sizes[i].height)
      continue;

    /* Fill picture_sizes_tbl entry */
    hal_data->picture_sizes_tbl[hal_data->picture_sizes_tbl_cnt] =
      default_picture_sizes[i];

    /* Also fill jpeg_stall_durations */
    hal_data->jpeg_stall_durations[hal_data->picture_sizes_tbl_cnt] =
      (hal_data->position) ?
      default_jpeg_stall_durations_front[i]:
      default_jpeg_stall_durations_back[i];

    /* Now select an appropriate picture_min_duration
       for this resolution */
    matching_index = mct_pipeline_pick_highest_fps (
      default_picture_sizes[i],
      sensor_dim_fps_table,
      local_data->sensor_cap.dim_fps_table_count );
    if (-1 == matching_index || matching_index >= SENSOR_MAX_RESOLUTION) {
      CLOGE(CAM_MCT_MODULE, "Unexpected failure. No match found for [%d x %d]",
        default_picture_sizes[i].width, default_picture_sizes[i].height);
      /* TODO: Use default frame duration for this entry */
    } else {
      float picture_max_fps = 1.0;
      /* Check this resolution with CPP B/W limits on given fps */
      /* Assuming NV12_420 YUV as the default streaming format */
      /* Also include up to 1080P stream to be running
         simultaneously at this published fps */
      float cpp_max_fps = ( (1.0 * local_data->pp_cap.max_pixel_bandwidth) /
        (((default_picture_sizes[i].width * default_picture_sizes[i].height) +
          (1920 * 1080) ) *
        DEFAULT_FORMAT_NV12_420) );
      if (cpp_max_fps >=
        local_data->sensor_cap.dim_fps_table[matching_index].fps.max_fps) {
        /* Select max sensor fps for this resolution */
        picture_max_fps = (uint32_t)
          (local_data->sensor_cap.dim_fps_table[matching_index].fps.max_fps);
      } else if ( ( cpp_max_fps <
        local_data->sensor_cap.dim_fps_table[matching_index].fps.max_fps) &&
        ( cpp_max_fps >
        local_data->sensor_cap.dim_fps_table[matching_index].fps.min_fps) ) {
        /* Select fps based on cpp BW for this resolution */
        matching_mct_index = mct_util_find_closest_fps (
          cpp_max_fps, default_fps_ranges,default_fps_tbl_size, COMPARE_LOWER);
        if ( (-1 != matching_mct_index &&
          matching_mct_index < default_fps_tbl_size) ) {
          picture_max_fps = (uint32_t)
            (default_fps_ranges[matching_mct_index].max_fps);
          temp_cpp_fps.min_fps = (uint32_t)
            (local_data->sensor_cap.dim_fps_table[matching_index].fps.min_fps)<=
            preview_max_fps ?
            (local_data->sensor_cap.dim_fps_table[matching_index].fps.min_fps) :
            preview_max_fps;
          temp_cpp_fps.max_fps = (uint32_t)picture_max_fps < preview_max_fps ?
            (uint32_t)picture_max_fps : preview_max_fps;

          /* Store cpp_fps into table */
          if (cpp_table_index < MAX_FPS_RANGE_ENTRIES &&
            FALSE == mct_util_check_matching_fps (
            temp_cpp_fps, cpp_fps_table, cpp_table_index)) {
            cpp_fps_table[cpp_table_index++] = temp_cpp_fps;
          } else if (cpp_table_index == MAX_FPS_RANGE_ENTRIES) {
            CLOGW(CAM_MCT_MODULE,
              "Running out of limit for cpp_fps_table. Consider increasing \
              MAX_FPS_RANGE_ENTRIES");
          }
        } else {
          /* CPP fps < lowest default fps. Choose a low fps */
          picture_max_fps = (uint32_t)
            (local_data->sensor_cap.dim_fps_table[matching_index].fps.min_fps);
          temp_cpp_fps.min_fps = temp_cpp_fps.max_fps =
            (uint32_t)(local_data->sensor_cap.dim_fps_table[matching_index].\
              fps.min_fps) <= preview_max_fps ?
              (uint32_t)(local_data->sensor_cap.dim_fps_table[matching_index].\
                              fps.min_fps) : preview_max_fps;
          if (cpp_table_index < MAX_FPS_RANGE_ENTRIES &&
            FALSE == mct_util_check_matching_fps (
            temp_cpp_fps, cpp_fps_table, cpp_table_index)) {
              cpp_fps_table[cpp_table_index++] = temp_cpp_fps;
          } else if (cpp_table_index == MAX_FPS_RANGE_ENTRIES) {
            CLOGW(CAM_MCT_MODULE,
              "Running out of limit for cpp_fps_table. Consider increasing \
              MAX_FPS_RANGE_ENTRIES");
          }
        }
      } else {
        /* Select min sensor fps for this resolution */
        picture_max_fps = (uint32_t)
          (local_data->sensor_cap.dim_fps_table[matching_index].fps.min_fps);
        temp_cpp_fps.min_fps = temp_cpp_fps.max_fps =
          (uint32_t)(local_data->sensor_cap.dim_fps_table[matching_index].\
            fps.min_fps) <= preview_max_fps ?
            (uint32_t)(local_data->sensor_cap.dim_fps_table[matching_index].\
                            fps.min_fps) : preview_max_fps;
        if (cpp_table_index < MAX_FPS_RANGE_ENTRIES &&
          FALSE == mct_util_check_matching_fps (
          temp_cpp_fps, cpp_fps_table, cpp_table_index)) {
          cpp_fps_table[cpp_table_index++] = temp_cpp_fps;
        } else if (cpp_table_index == MAX_FPS_RANGE_ENTRIES) {
          CLOGW(CAM_MCT_MODULE,
            "Running out of limit for cpp_fps_table. Consider increasing \
            MAX_FPS_RANGE_ENTRIES");
        }
      }
      /* Fix lower bound on min_durations */
      if (picture_max_fps > preview_max_fps) {
        picture_max_fps = preview_max_fps;
      }
      if (picture_max_fps > 0) {
        hal_data->picture_min_duration[hal_data->picture_sizes_tbl_cnt] =
          (1e9 / picture_max_fps);
        CLOGD(CAM_MCT_MODULE,
            "Cam %d: Res (%d x %d): fps = %f", hal_data->position,
          default_picture_sizes[i].width, default_picture_sizes[i].height,
          picture_max_fps);
      } else {
        CLOGE(CAM_MCT_MODULE, "Invalid picture_max_fps %f", picture_max_fps);
      }
    }

    hal_data->picture_sizes_tbl_cnt++;
  }

  /* Filter sensor table for fps range selection */
  memset(sensor_fps_table, 0, MAX_FPS_RANGE_ENTRIES * sizeof(cam_fps_range_t));
  for (i = 0; i < local_data->sensor_cap.dim_fps_table_count; i++) {
    /* Allow only integer, non-duplicate, non-HFR entries */
    if ((uint32_t)(local_data->sensor_cap.dim_fps_table[i].fps.max_fps) <=
      preview_max_fps) {
      if ( (sensor_table_index < MAX_FPS_RANGE_ENTRIES) &&
        FALSE == mct_util_check_matching_fps (
        local_data->sensor_cap.dim_fps_table[i].fps,
        sensor_fps_table, sensor_table_index) ) {
        sensor_fps_table[sensor_table_index].min_fps =
          (uint32_t)(local_data->sensor_cap.dim_fps_table[i].fps.min_fps);
        sensor_fps_table[sensor_table_index].max_fps =
          (uint32_t)(local_data->sensor_cap.dim_fps_table[i].fps.max_fps);
        sensor_fps_table[sensor_table_index].video_min_fps =
          (uint32_t)(local_data->sensor_cap.dim_fps_table[i].fps.video_min_fps);
        sensor_fps_table[sensor_table_index].video_max_fps =
          (uint32_t)(local_data->sensor_cap.dim_fps_table[i].fps.video_max_fps);
        sensor_table_index++;
      } else if (sensor_table_index == MAX_FPS_RANGE_ENTRIES) {
        CLOGW(CAM_MCT_MODULE,
          "Running out of limit for sensor_fps_table. Consider increasing \
          MAX_FPS_RANGE_ENTRIES");
      }
    }
  }
  /* Filter mct table for fps range selection */
  memset(mct_fps_table, 0, MAX_FPS_RANGE_ENTRIES * sizeof(cam_fps_range_t));
  for (i = 0; i < default_fps_tbl_size; i++) {
    /* Allow only integer, non-duplicate, non-HFR entries
       that satisfy sensor capabilities (coming from sensor_fps_table) */
    if ((uint32_t)(default_fps_ranges[i].max_fps) <= preview_max_fps) {
      if ( (mct_table_index < MAX_FPS_RANGE_ENTRIES) &&
        FALSE == mct_util_check_matching_fps (
          default_fps_ranges[i], mct_fps_table, mct_table_index) &&
        (-1 != mct_util_find_closest_fps(default_fps_ranges[i].max_fps,
          sensor_fps_table, sensor_table_index, COMPARE_HIGHER) ) ) {
        mct_fps_table[mct_table_index++] = default_fps_ranges[i];
      } else {
        CLOGI(CAM_MCT_MODULE,
          "Running out of table limit for mct_fps_table. Consider increasing \
          MAX_FPS_RANGE_ENTRIES ");
      }
    }
  }

  /* Populate fps ranges */
  if (!mct_pipeline_fill_fps_ranges(pipeline,
    mct_fps_table, mct_table_index,
    sensor_fps_table, sensor_table_index,
    cpp_fps_table, cpp_table_index) ) {
    CLOGE(CAM_MCT_MODULE, "Fatal error in filling up fps ranges.");
  } else {
    CLOGI(CAM_MCT_MODULE,
      "Cam %d: FPS range selection complete. Publishing %d ranges.",
      hal_data->position, hal_data->fps_ranges_tbl_cnt);
  }
  return TRUE;
}

/** mct_pipeline_fill_dimensions_raw:
 *    @pipeline: Pipeline pointer
 *    @snap_sensor_width, snap_sensor_height: Snapshot dimensions
 *    @snap_sensor_min_duration: Min frame_duration for a given resolution
 *
 *  Description:
 *    Populates raw dimensions in HAL query_buf.
 *    These fields are filled with static values for now.
 *
 *  Return: TRUE upon successful completion of operations.
 *             FALSE for intermittent failures.
 **/
static boolean mct_pipeline_fill_dimensions_raw (
  mct_pipeline_t *pipeline,
  const int32_t snap_sensor_width, const int32_t snap_sensor_height,
  const int64_t snap_sensor_min_duration )
{
  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Invalid pipeline ptr");
    return FALSE;
  }
  cam_capability_t   *hal_data   = pipeline->query_buf;
  if (!hal_data) {
    CLOGE(CAM_MCT_MODULE, "NULL query_buf!");
    return FALSE;
  }
  hal_data->raw16_stall_durations[0] = ((hal_data->position)? 0.050 : 0.100)
    * SEC_TO_NS_FACTOR;
  hal_data->supported_raw_dim_cnt = 1;
  hal_data->raw_dim[0].width = snap_sensor_width;
  hal_data->raw_dim[0].height = snap_sensor_height;
  hal_data->raw_min_duration[0] = snap_sensor_min_duration;
  return TRUE;
}

/** mct_pipeline_fill_dimensions_HFR:
 *    @pipeline: Pipeline pointer
 *    @hfr_sensor_width, hfr_sensor_height: HFR dimension arrays
 *
 *  Description:
 *    Populates HFR dimensions and corresponding liveshot dimensions
 *    in HAL query_buf
 *
 *  Return: TRUE upon successful completion of operations.
 *             FALSE for intermittent failures.
 **/
static boolean mct_pipeline_fill_dimensions_HFR (
  mct_pipeline_t *pipeline,
  const int32_t *hfr_sensor_width, const int32_t *hfr_sensor_height )
{
  uint32_t i = 0, j = 0, idx = 0;
  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Invalid pipeline ptr");
    return FALSE;
  }
  cam_capability_t   *hal_data   = pipeline->query_buf;
  if (!hal_data) {
    CLOGE(CAM_MCT_MODULE, "NULL query_buf!");
    return FALSE;
  }
  if (!hfr_sensor_width || !hfr_sensor_height) {
    CLOGE(CAM_MCT_MODULE, "NULL ptr: hfr_sensor_width=%p,hfr_sensor_height=%p",
      hfr_sensor_width, hfr_sensor_height);
    return FALSE;
  }

  for (i=0; i < CAM_HFR_MODE_MAX; i++) {
    if (hfr_sensor_width[i] != 0 && hfr_sensor_height[i] != 0) {
      hal_data->hfr_tbl[idx].mode = i;

      /* Add HFR dimensions for this mode */
      hal_data->hfr_tbl[idx].dim_cnt = 0;
      for (j = 0;
          (j < (sizeof(default_hfr_video_sizes) / sizeof(cam_dimension_t))
            && j < MAX_SIZES_CNT); j++) {
        if (default_hfr_video_sizes[j].width <= hfr_sensor_width[i] &&
          default_hfr_video_sizes[j].height <= hfr_sensor_height[i]) {
          hal_data->hfr_tbl[idx].dim
            [hal_data->hfr_tbl[idx].dim_cnt++] =
            default_hfr_video_sizes[j];
        }
      }
      /* Add liveshot dimensions for this HFR mode */
      hal_data->hfr_tbl[idx].livesnapshot_sizes_tbl_cnt = 0;
      for (j = 0;
          (j < (sizeof(default_liveshot_sizes) / sizeof(cam_dimension_t))
            && j < MAX_SIZES_CNT); j++) {
        if (default_liveshot_sizes[j].width <= hfr_sensor_width[i] &&
          default_liveshot_sizes[j].height <= hfr_sensor_height[i]) {
          hal_data->hfr_tbl[idx].livesnapshot_sizes_tbl
            [hal_data->hfr_tbl[idx].livesnapshot_sizes_tbl_cnt++] =
            default_liveshot_sizes[j];
        }
      }
      idx++;
    }
  }
  hal_data->hfr_tbl_cnt = idx;
  return TRUE;
}

/** mct_pipeline_fill_dimensions_video:
 *    @pipeline: Pipeline pointer
 *    @video_sensor_width, video_sensor_height: Video dimensions
 *
 *  Description:
 *    Populates video dimensions in HAL query_buf
 *
 *  Return: TRUE upon successful completion of operations.
 *             FALSE for intermittent failures.
 **/
static boolean mct_pipeline_fill_dimensions_video (
  mct_pipeline_t *pipeline,
  const int32_t video_sensor_width, const int32_t video_sensor_height )
{
  uint32_t i = 0;
  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Invalid pipeline ptr");
    return FALSE;
  }
  cam_capability_t   *hal_data   = pipeline->query_buf;
  if (!hal_data) {
    CLOGE(CAM_MCT_MODULE, "NULL query_buf!");
    return FALSE;
  }
  hal_data->video_sizes_tbl_cnt = 0;
  for (i = 0;
       (i < (sizeof(default_video_sizes) / sizeof(cam_dimension_t))
          && i < MAX_SIZES_CNT);
       i++) {
    if (default_video_sizes[i].width <= video_sensor_width &&
      default_video_sizes[i].height <= video_sensor_height) {
      hal_data->video_sizes_tbl[hal_data->video_sizes_tbl_cnt++] =
        default_video_sizes[i];
    }
  }
  return TRUE;
}

/** mct_pipeline_fill_dimensions_livesnapshot:
 *    @pipeline: Pipeline pointer
 *    @liveshot_sensor_width, liveshot_sensor_height: Live snapshot dimensions
 *
 *  Description:
 *    Populates livesnapshot dimensions in HAL query_buf
 *
 *  Return: TRUE upon successful completion of operations.
 *             FALSE for intermittent failures.
 **/
static boolean mct_pipeline_fill_dimensions_livesnapshot (
  mct_pipeline_t *pipeline,
  const int32_t liveshot_sensor_width, const int32_t liveshot_sensor_height )
{
  uint32_t i = 0;
  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Invalid pipeline ptr");
    return FALSE;
  }
  cam_capability_t   *hal_data   = pipeline->query_buf;
  if (!hal_data) {
    CLOGE(CAM_MCT_MODULE, "NULL query_buf!");
    return FALSE;
  }
  hal_data->video_snapshot_supported = 1;

  hal_data->livesnapshot_sizes_tbl_cnt = 0;
  for (i = 0;
       (i < (sizeof(default_liveshot_sizes) / sizeof(cam_dimension_t))
          && i < MAX_SIZES_CNT);
       i++) {
    if (default_liveshot_sizes[i].width <= liveshot_sensor_width &&
      default_liveshot_sizes[i].height <= liveshot_sensor_height) {
      hal_data->livesnapshot_sizes_tbl[hal_data->livesnapshot_sizes_tbl_cnt++]=
        default_liveshot_sizes[i];
    }
  }
  return TRUE;
}

/** mct_pipeline_fill_dimensions_VHDR_Liveshot:
 *    @pipeline: Pipeline pointer
 *    @vhdr_liveshot_width, vhdr_liveshot_height: VHDR liveshot dimensions
 *
 *  Description:
 *    Populates VHDR liveshot dimensions in HAL query_buf
 *
 *  Return: TRUE upon successful completion of operations.
 *             FALSE for intermittent failures.
 **/
static boolean mct_pipeline_fill_dimensions_VHDR_Liveshot (
  mct_pipeline_t *pipeline,
  const int32_t vhdr_liveshot_width, const int32_t vhdr_liveshot_height )
{
  uint32_t i = 0;
  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Invalid pipeline ptr");
    return FALSE;
  }
  cam_capability_t   *hal_data   = pipeline->query_buf;
  if (!hal_data) {
    CLOGE(CAM_MCT_MODULE, "NULL query_buf!");
    return FALSE;
  }
  hal_data->vhdr_livesnapshot_sizes_tbl_cnt = 0;
  for (i = 0;
       (i < (sizeof(default_liveshot_sizes) / sizeof(cam_dimension_t))
          && i < MAX_SIZES_CNT);
       i++) {
    if (default_liveshot_sizes[i].width <= vhdr_liveshot_width &&
      default_liveshot_sizes[i].height <= vhdr_liveshot_height) {
      hal_data->vhdr_livesnapshot_sizes_tbl
        [hal_data->vhdr_livesnapshot_sizes_tbl_cnt++] =
          default_liveshot_sizes[i];
    }
  }
  return TRUE;
}

/** mct_pipeline_fill_dimensions_ZZHDR:
 *    @pipeline: Pipeline pointer
 *    @zzhdr_sensor_width, zzhdr_sensor_height: ZZHDR dimensions
 *
 *  Description:
 *    Populates ZZHDR dimensions in HAL query_buf
 *
 *  Return: TRUE upon successful completion of operations.
 *             FALSE for intermittent failures.
 **/
static boolean mct_pipeline_fill_dimensions_ZZHDR(mct_pipeline_t *pipeline,
  const int32_t zzhdr_sensor_width, const int32_t zzhdr_sensor_height )
{
  uint32_t i = 0;
  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Invalid pipeline ptr");
    return FALSE;
  }
  mct_pipeline_cap_t *local_data = &pipeline->query_data;
  cam_capability_t   *hal_data   = pipeline->query_buf;
  if (!hal_data) {
    CLOGE(CAM_MCT_MODULE, "NULL query_buf!");
    return FALSE;
  }

  if (local_data->isp_cap.is_hvx_present) {
    hal_data->zzhdr_sizes_tbl_cnt = 0;
    for (i = 0;
      (i < (sizeof(default_picture_sizes) / sizeof(cam_dimension_t))
        && i < MAX_SIZES_CNT);
        i++ ) {
      if (default_picture_sizes[i].width <= zzhdr_sensor_width &&
        default_picture_sizes[i].height <= zzhdr_sensor_height) {
        hal_data->zzhdr_sizes_tbl[hal_data->zzhdr_sizes_tbl_cnt++] =
          default_picture_sizes[i];
      }
    }
  }
  return TRUE;
}

/** mct_pipeline_fill_dimensions_quadra_cfa
 *    @pipeline: Pipeline pointer
 *    @snap_sensor_width, snap_sensor_height: Snapshot dimensions
 *  Description:
 *    Populates quara CFA dimensions in HAL query_buf.
 *    These fields are filled with static values for now.
 *
 *  Return: TRUE upon successful completion of operations.
 *             FALSE for intermittent failures.
 **/
static boolean mct_pipeline_fill_dimensions_quadra_cfa(
  mct_pipeline_t *pipeline, const int32_t quadra_cfa_width,
  const int32_t quadra_cfa_height)
{
  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Invalid pipeline ptr");
    return FALSE;
  }
  cam_capability_t *hal_data = pipeline->query_buf;
  if (!hal_data) {
    CLOGE(CAM_MCT_MODULE, "NULL query_buf!");
    return FALSE;
  }
  hal_data->supported_quadra_cfa_dim_cnt = 1;
  hal_data->quadra_cfa_dim[0].width = quadra_cfa_width;
  hal_data->quadra_cfa_dim[0].height = quadra_cfa_height;
  CLOGH(CAM_MCT_MODULE,"Quadra CFA Mode: Width =%d height =%d",
    quadra_cfa_width, quadra_cfa_height);
  return TRUE;
}

/** mct_pipeline_populate_query_cap_buffer:
 *    @pipeline: Pipeline pointer
 *
 *
 *  Description:
 *    Populates capabilities queried from modules
 *    into HAL query_buf.
 *
 *  Return: TRUE upon successful completion of operations.
 *             FALSE for intermittent failures.
 **/
static boolean mct_pipeline_populate_query_cap_buffer(mct_pipeline_t *pipeline)
{
  mct_pipeline_cap_t *local_data = &pipeline->query_data;
  cam_capability_t   *hal_data   = pipeline->query_buf;
  boolean ret = TRUE;
  uint32_t i = 0, j = 0;
  int32_t prev_sensor_width = 0, prev_sensor_height = 0;
  int32_t snap_sensor_width = 0, snap_sensor_height = 0;
  int32_t quadra_cfa_width = 0, quadra_cfa_height = 0;
  int32_t remosaic_lib_present = 0;
  int32_t quadracfa_insensor = 0;
  int64_t snap_sensor_min_duration = 0;
  int32_t max_snap_width = 0, max_snap_height = 0;
  int32_t vid_sensor_width = 0, vid_sensor_height = 0;
  int32_t liveshot_sensor_width = 0, liveshot_sensor_height = 0;
  int32_t vhdr_liveshot_sensor_width = 0, vhdr_liveshot_sensor_height = 0;
  int32_t zzhdr_sensor_width = 0, zzhdr_sensor_height = 0;
  int32_t hfr_sensor_width[CAM_HFR_MODE_MAX];
  int32_t hfr_sensor_height[CAM_HFR_MODE_MAX];
  int8_t matching_index = -1;
  struct sysinfo info;
  int set_max_snap = 0;
  char value[PROPERTY_VALUE_MAX];

  if (!hal_data || !local_data) {
    CLOGE(CAM_MCT_MODULE,
          "NULL ptr detected! query_buf = [%p] local_data = [%p]",
          hal_data, local_data);
    return FALSE;
  }
  /* Camera ID */
  hal_data->position = local_data->sensor_cap.position;
  CLOGD(CAM_MCT_MODULE, "Filling capabilities for cam[%d]",hal_data->position);

  /* Fill feature mask */
  hal_data->qcom_supported_feature_mask =
    local_data->sensor_cap.feature_mask | local_data->isp_cap.feature_mask |
    local_data->stats_cap.feature_mask | local_data->pp_cap.feature_mask |
    local_data->imaging_cap.feature_mask;
  if (local_data->sensor_cap.sensor_format != FORMAT_BAYER) {
    hal_data->qcom_supported_feature_mask &= ~CAM_QCOM_FEATURE_SHARPNESS;
    hal_data->qcom_supported_feature_mask &= ~CAM_QCOM_FEATURE_EFFECT;
  }

  memset(hfr_sensor_width, 0, sizeof(int32_t)*CAM_HFR_MODE_MAX);
  memset(hfr_sensor_height, 0, sizeof(int32_t)*CAM_HFR_MODE_MAX);

  for (i = 0; i < local_data->sensor_cap.dim_fps_table_count; i++) {
    if (local_data->sensor_cap.dim_fps_table[i].fps.max_fps >= 29.0 &&
        local_data->sensor_cap.dim_fps_table[i].fps.max_fps < 60.0) {
      if (local_data->sensor_cap.dim_fps_table[i].dim.width >
        liveshot_sensor_width) {
        if (local_data->sensor_cap.dim_fps_table[i].dim.height >
          liveshot_sensor_height) {
          liveshot_sensor_width  =
            local_data->sensor_cap.dim_fps_table[i].dim.width;
          liveshot_sensor_height =
            local_data->sensor_cap.dim_fps_table[i].dim.height;
        } else { /* check if the resolution is 16:9 */
          if (fabs(((float)local_data->sensor_cap.dim_fps_table[i].dim.width) /
             ((float)local_data->sensor_cap.dim_fps_table[i].dim.height)-1.778)
              <= 0.1) {
            liveshot_sensor_width  =
              local_data->sensor_cap.dim_fps_table[i].dim.width;
            liveshot_sensor_height =
              local_data->sensor_cap.dim_fps_table[i].dim.height;
          }
        }
      }
    }
    if (local_data->sensor_cap.dim_fps_table[i].fps.max_fps >= 15.0 &&
        local_data->sensor_cap.dim_fps_table[i].fps.max_fps < 60.0) {
      if (local_data->sensor_cap.dim_fps_table[i].dim.width >
          prev_sensor_width) {
        if (local_data->sensor_cap.dim_fps_table[i].dim.height >
          prev_sensor_height) {
          prev_sensor_width  =
            local_data->sensor_cap.dim_fps_table[i].dim.width;
          prev_sensor_height =
            local_data->sensor_cap.dim_fps_table[i].dim.height;
          vid_sensor_width  =
            local_data->sensor_cap.dim_fps_table[i].dim.width;
          vid_sensor_height =
            local_data->sensor_cap.dim_fps_table[i].dim.height;
          if ( (local_data->sensor_cap.dim_fps_table[i].fps.max_fps >= 29.0) &&
            (local_data->sensor_cap.dim_fps_table[i].fps.max_fps < 30.0) ) {
            /* Tweak max_fps to satisfy 30fps video record */
             local_data->sensor_cap.dim_fps_table[i].fps.max_fps = 30.0;
          }
        } else { /* check if the resolution is 16:9 */
          if (fabs(((float)local_data->sensor_cap.dim_fps_table[i].dim.width) /
             ((float)local_data->sensor_cap.dim_fps_table[i].dim.height)-1.778)
              <= 0.1) {
            vid_sensor_width =
              local_data->sensor_cap.dim_fps_table[i].dim.width;
            vid_sensor_height =
              local_data->sensor_cap.dim_fps_table[i].dim.height;
          }
        }
      }
    } else if ( (local_data->sensor_cap.dim_fps_table[i].fps.max_fps >=
      (60.0 - FPS_VARIANCE_LIMIT)) &&
      (local_data->sensor_cap.dim_fps_table[i].fps.max_fps <
      (90.0 - FPS_VARIANCE_LIMIT)) ) {
      /*HFR mode 60fps */
      mct_pipeline_find_matching_hfr_dim(CAM_HFR_MODE_60FPS,
        local_data->sensor_cap.dim_fps_table[i].dim,
        hfr_sensor_width, hfr_sensor_height);

    } else if ( (local_data->sensor_cap.dim_fps_table[i].fps.max_fps >=
      (90.0 - FPS_VARIANCE_LIMIT)) &&
      (local_data->sensor_cap.dim_fps_table[i].fps.max_fps <
      (120.0 - FPS_VARIANCE_LIMIT)) ) {
        /*HFR mode 90fps*/
        mct_pipeline_find_matching_hfr_dim(CAM_HFR_MODE_90FPS,
         local_data->sensor_cap.dim_fps_table[i].dim,
         hfr_sensor_width, hfr_sensor_height);

    } else if ( (local_data->sensor_cap.dim_fps_table[i].fps.max_fps >=
      (120.0 - FPS_VARIANCE_LIMIT)) &&
      (local_data->sensor_cap.dim_fps_table[i].fps.max_fps <
      (150.0 - FPS_VARIANCE_LIMIT)) ) {
        /*HFR mode 120 fps*/
        mct_pipeline_find_matching_hfr_dim(CAM_HFR_MODE_120FPS,
         local_data->sensor_cap.dim_fps_table[i].dim,
         hfr_sensor_width, hfr_sensor_height);

    } else if ( (local_data->sensor_cap.dim_fps_table[i].fps.max_fps >=
      (150.0 - FPS_VARIANCE_LIMIT)) &&
      (local_data->sensor_cap.dim_fps_table[i].fps.max_fps <
      (240.0 - FPS_VARIANCE_LIMIT)) ) {
        /*HFR mode 150 fps */
        mct_pipeline_find_matching_hfr_dim(CAM_HFR_MODE_150FPS,
         local_data->sensor_cap.dim_fps_table[i].dim,
         hfr_sensor_width, hfr_sensor_height);

    } else if ( local_data->sensor_cap.dim_fps_table[i].fps.max_fps >=
      (240.0 - FPS_VARIANCE_LIMIT) ) {
      /*HFR mode 240 fps */
      mct_pipeline_find_matching_hfr_dim(CAM_HFR_MODE_240FPS,
       local_data->sensor_cap.dim_fps_table[i].dim,
       hfr_sensor_width, hfr_sensor_height);
    }

    /*SENSOR_HDR_MODE */
    if (local_data->sensor_cap.dim_fps_table[i].mode == (1 << 2)) {
      if (local_data->sensor_cap.dim_fps_table[i].dim.width >
        vhdr_liveshot_sensor_width &&
          local_data->sensor_cap.dim_fps_table[i].dim.height >
          vhdr_liveshot_sensor_height) {
        vhdr_liveshot_sensor_width =
          local_data->sensor_cap.dim_fps_table[i].dim.width;
        vhdr_liveshot_sensor_height =
          local_data->sensor_cap.dim_fps_table[i].dim.height;
      }
    }
    /*SENSOR_RAW_HDR_MODE */
    else if (local_data->sensor_cap.dim_fps_table[i].mode == (1 << 3)
      && local_data->isp_cap.is_hvx_present) {
      if (local_data->sensor_cap.dim_fps_table[i].dim.width >=
        zzhdr_sensor_width &&
        local_data->sensor_cap.dim_fps_table[i].dim.height >=
        zzhdr_sensor_height) {
          zzhdr_sensor_width =
            local_data->sensor_cap.dim_fps_table[i].dim.width;
          zzhdr_sensor_height =
            local_data->sensor_cap.dim_fps_table[i].dim.height;
      }
      if (local_data->sensor_cap.dim_fps_table[i].dim.width >
        vhdr_liveshot_sensor_width &&
          local_data->sensor_cap.dim_fps_table[i].dim.height >
            vhdr_liveshot_sensor_height) {
          vhdr_liveshot_sensor_width =
            local_data->sensor_cap.dim_fps_table[i].dim.width;
          vhdr_liveshot_sensor_height =
            local_data->sensor_cap.dim_fps_table[i].dim.height;
      }
    }

    /*QUADRA_CFA_MODE */
    else if (local_data->sensor_cap.dim_fps_table[i].mode & (1 << 4)) {
      if(local_data->pp_cap.is_remosaic_lib_present ||
        local_data->sensor_cap.dim_fps_table[i].mode & (1 << 5)) {
          if (local_data->sensor_cap.dim_fps_table[i].dim.width >=
            quadra_cfa_width &&
            local_data->sensor_cap.dim_fps_table[i].dim.height >=
            quadra_cfa_height) {
            remosaic_lib_present = local_data->pp_cap.is_remosaic_lib_present;
            quadracfa_insensor =
              (local_data->sensor_cap.dim_fps_table[i].mode & (1 << 5)) ? TRUE : FALSE;
            quadra_cfa_width =
              local_data->sensor_cap.dim_fps_table[i].dim.width;
            quadra_cfa_height =
              local_data->sensor_cap.dim_fps_table[i].dim.height;
          }
      }
    }

    if (local_data->sensor_cap.dim_fps_table[i].dim.width >
        snap_sensor_width
        && local_data->sensor_cap.dim_fps_table[i].dim.height >
        snap_sensor_height) {
      if (!((local_data->sensor_cap.dim_fps_table[i].mode & (1 << 4)) != 0)) {
        snap_sensor_width  =
          local_data->sensor_cap.dim_fps_table[i].dim.width;
        snap_sensor_height =
          local_data->sensor_cap.dim_fps_table[i].dim.height;
        snap_sensor_min_duration =
          1e9 / local_data->sensor_cap.dim_fps_table[i].fps.max_fps;
      }
    }
  } /*for loop*/
  sysinfo(&info);
  CLOGD(CAM_MCT_MODULE, "totalram = %ld, freeram = %ld",
    info.totalram, info.freeram);
  if (info.totalram <= TOTAL_RAM_SIZE_512MB) {
    max_snap_width = PICTURE_SIZE_5MP_WIDTH;
    max_snap_height = PICTURE_SIZE_5MP_HEIGHT;
  }
  memset(value, 0, PROPERTY_VALUE_MAX);
  property_get("persist.vendor.camera.max.snapshot", value, "");
  if (strlen(value) > 0) {
      set_max_snap = 1;
      char *saveptr = NULL;
      char *token = strtok_r(value, "x", &saveptr);
      if (token) {
        max_snap_width = atoi(token);
      }
      token = strtok_r(NULL, "x", &saveptr);
      if (token) {
        max_snap_height = atoi(token);
      }
  }

  if (set_max_snap) {
    if (info.totalram <= TOTAL_RAM_SIZE_512MB) {
      if (max_snap_width > PICTURE_SIZE_5MP_WIDTH ||
          max_snap_height > PICTURE_SIZE_5MP_HEIGHT) {
        max_snap_width = PICTURE_SIZE_5MP_WIDTH;
        max_snap_height = PICTURE_SIZE_5MP_HEIGHT;
      }
    }
    if (prev_sensor_width > max_snap_width && prev_sensor_height >
        max_snap_height) {
      prev_sensor_width = max_snap_width;
      prev_sensor_height = max_snap_height;
    } else if (prev_sensor_width > max_snap_width ||
               prev_sensor_height > max_snap_height) {
        for (i = 0;
             (i < (sizeof(default_preview_sizes) / sizeof(cam_dimension_t))
                && i < MAX_SIZES_CNT);
             i++) {
          if (default_preview_sizes[i].width <= max_snap_width &&
              default_preview_sizes[i].height <= max_snap_height) {
            prev_sensor_width = default_preview_sizes[i].width;
            prev_sensor_height = default_preview_sizes[i].height;
            break;
          }
        }
    }
    if (snap_sensor_width > max_snap_width &&
        snap_sensor_height > max_snap_height) {
      snap_sensor_width = max_snap_width;
      snap_sensor_height = max_snap_height;
    } else if (snap_sensor_width > max_snap_width ||
               snap_sensor_height > max_snap_height) {
        for (i = 0;
             (i < (sizeof(default_picture_sizes) / sizeof(cam_dimension_t))
                && i < MAX_SIZES_CNT);
             i++) {
          if (default_picture_sizes[i].width <= max_snap_width &&
              default_picture_sizes[i].height <= max_snap_height) {
            snap_sensor_width = default_picture_sizes[i].width;
            snap_sensor_height = default_picture_sizes[i].height;
            break;
          }
        }
    }
    if (liveshot_sensor_width > max_snap_width &&
        liveshot_sensor_height > max_snap_height) {
        liveshot_sensor_width = max_snap_width;
        liveshot_sensor_height = max_snap_height;
    } else if (liveshot_sensor_width > max_snap_width ||
                 liveshot_sensor_height > max_snap_height) {
        for (i = 0;
             (i < (sizeof(default_liveshot_sizes) / sizeof(cam_dimension_t))
                && i < MAX_SIZES_CNT);
             i++) {
          if (default_liveshot_sizes[i].width <= max_snap_width &&
              default_liveshot_sizes[i].height <= max_snap_height) {
            liveshot_sensor_width = default_liveshot_sizes[i].width;
            liveshot_sensor_height = default_liveshot_sizes[i].height;
            break;
          }
        }
    }
  }

  /* Populate dimension tables */
  ret &= mct_pipeline_fill_dimensions_preview(pipeline,
    prev_sensor_width, prev_sensor_height,
    snap_sensor_width,snap_sensor_height);

  if (remosaic_lib_present || quadracfa_insensor) {
    CLOGH(CAM_MCT_MODULE,"Quadra CFA Mode supported");
    ret &= mct_pipeline_fill_dimensions_snapshot(pipeline,
      quadra_cfa_width, quadra_cfa_height);
    ret &= mct_pipeline_fill_dimensions_quadra_cfa(pipeline,
      quadra_cfa_width, quadra_cfa_height);
    hal_data->is_remosaic_lib_present = remosaic_lib_present;
    hal_data->is_quadracfa_insensor = quadracfa_insensor;
  }
  else {
    ret &= mct_pipeline_fill_dimensions_snapshot(pipeline,
      snap_sensor_width, snap_sensor_height);
  }

  ret &= mct_pipeline_fill_dimensions_raw(pipeline,
    snap_sensor_width, snap_sensor_height, snap_sensor_min_duration);

  ret &= mct_pipeline_fill_dimensions_HFR (pipeline,
   hfr_sensor_width, hfr_sensor_height);

  ret &= mct_pipeline_fill_dimensions_video (pipeline,
   vid_sensor_width, vid_sensor_height);

  ret &= mct_pipeline_fill_dimensions_livesnapshot (pipeline,
   liveshot_sensor_width, liveshot_sensor_height);

  ret &= mct_pipeline_fill_dimensions_VHDR_Liveshot (pipeline,
    vhdr_liveshot_sensor_width, vhdr_liveshot_sensor_height);

  ret &= mct_pipeline_fill_dimensions_ZZHDR (pipeline,
    zzhdr_sensor_width, zzhdr_sensor_height);

  /* Populate module-specific capabilities */
  ret &= mct_pipeline_fill_supported_effects(pipeline);
  ret &= mct_pipeline_fill_supported_scene_modes(pipeline);
  ret &= mct_pipeline_fill_caps_sensor(pipeline);
  ret &= mct_pipeline_fill_caps_isp(pipeline);
  ret &= mct_pipeline_fill_caps_stats(pipeline);
  ret &= mct_pipeline_fill_caps_pproc(pipeline);
  ret &= mct_pipeline_fill_caps_imaging(pipeline);
  ret &= mct_pipeline_fill_caps_common(pipeline);

  hal_data->supported_ir_mode_cnt = 0;
  if (local_data->sensor_cap.supported_ir_modes[0]) {
    hal_data->supported_ir_modes[hal_data->supported_ir_mode_cnt++] =
      CAM_IR_MODE_OFF;
  }
  if (local_data->sensor_cap.supported_ir_modes[1]) {
    hal_data->supported_ir_modes[hal_data->supported_ir_mode_cnt++] =
      CAM_IR_MODE_ON;
  }

  return ret;
}

/** mct_pipeline_process_set:
 *    @
 *    @
 *
 **/
static boolean mct_pipeline_process_set(struct msm_v4l2_event_data *data,
  mct_pipeline_t *pipeline)
{
  boolean      ret = TRUE;
  mct_stream_t *stream = NULL;
  mct_pipeline_get_stream_info_t info;

  if (data->command != CAM_PRIV_PARM)
    CLOGI(CAM_MCT_MODULE, "command=%x", data->command);

  /* First find correct stream; for some commands find based on index,
   * for others (session based commands) find the appropriate stream
   * based on stream_type */
  switch (data->command) {
  case CAM_PRIV_STREAM_INFO_SYNC:
  case MSM_CAMERA_PRIV_STREAM_ON:
  case MSM_CAMERA_PRIV_STREAM_OFF:
  case MSM_CAMERA_PRIV_DEL_STREAM:
  case CAM_PRIV_STREAM_PARM: {
    info.check_type   = CHECK_INDEX;
    info.stream_index = data->stream_id;

    CLOGL(CAM_MCT_MODULE, "stream id: %x", data->stream_id);

    stream = mct_pipeline_get_stream(pipeline, &info);
    if (!stream) {
      CLOGE(CAM_MCT_MODULE, "Couldn't find stream id: %x",
          data->stream_id);
      return FALSE;
    }

    if (MSM_CAMERA_PRIV_STREAM_ON == data->command &&
      MCT_ST_STATE_RUNNING == stream->state) {
      CLOGI(CAM_MCT_MODULE,
            "Stream 0x%x, stream type=%d already in running state.",
            stream->streaminfo.identity, stream->streaminfo.stream_type);
      return TRUE;
    }
    else if (MSM_CAMERA_PRIV_STREAM_OFF == data->command &&
      MCT_ST_STATE_IDLE == stream->state) {
      CLOGI(CAM_MCT_MODULE,
            "Stream 0x%x stream type=%d is already streamed off.",
            stream->streaminfo.identity, stream->streaminfo.stream_type);
      return TRUE;
    }
  }
    break;

  case CAM_PRIV_PARM:
  case CAM_PRIV_PREPARE_SNAPSHOT:
  case CAM_PRIV_START_ZSL_SNAPSHOT:
  case CAM_PRIV_STOP_ZSL_SNAPSHOT:
  case CAM_PRIV_CANCEL_AUTO_FOCUS:
  case CAM_PRIV_DO_AUTO_FOCUS:
  case CAM_PRIV_DUAL_CAM_CMD: {
    info.check_type   = CHECK_INDEX;
    info.stream_index = MCT_SESSION_STREAM_ID;
    stream = mct_pipeline_get_stream(pipeline, &info);
    if (!stream) {
      CLOGE(CAM_MCT_MODULE, "Couldn't find stream id: %x",
          data->stream_id);
      return FALSE;
    }
  }
    break;

  default:
    break;
  }

  /* Now process the set ctrl command on the appropriate stream */
  switch (data->command) {
  case MSM_CAMERA_PRIV_NEW_STREAM: {

    CLOGD(CAM_MCT_MODULE, "Adding new stream: id [0%x]", data->stream_id);
    if (pipeline->add_stream) {
      MCT_PROF_LOG_BEG(PROF_MCT_STREAMADD, "id:", data->stream_id);

      ret = pipeline->add_stream(pipeline, data->stream_id);
      if(FALSE == ret)
        CLOGE(CAM_MCT_MODULE, "add stream failed for stream [0%x]",
          data->stream_id);

        MCT_PROF_LOG_END();
    } else {
      ret = FALSE;
    }
  }
    break;

  case CAM_PRIV_STREAM_INFO_SYNC: {
  /* This will trigger module linking*/
    if (!MCT_STREAM_STREAMINFO(stream)) {
      ret = FALSE;
    } else {
      MCT_PROF_LOG_BEG(PROF_MCT_STREAM_LINK, "id", stream->streamid,
                    "type", stream->streaminfo.stream_type);
      (MCT_STREAM_LINK(stream)) ?
        (ret = (MCT_STREAM_LINK(stream))(stream)) : (ret = FALSE);
      if (FALSE == ret) {
        CLOGE(CAM_MCT_MODULE, "Linking failed for stream 0x%x, stream type=%d",
           stream->streaminfo.identity,
          stream->streaminfo.stream_type);
      } else {
        CLOGI(CAM_MCT_MODULE,
              "Linking successful for stream 0x%x stream type=%d",
              stream->streaminfo.identity, stream->streaminfo.stream_type);
      }
      MCT_PROF_LOG_END();
    }
  }
    break;

  case MSM_CAMERA_PRIV_STREAM_ON:
  case MSM_CAMERA_PRIV_STREAM_OFF:{
    mct_event_t         cmd_event;
    mct_event_control_t event_data;
    char command[20];
    boolean ret = TRUE;

    if (data->command == MSM_CAMERA_PRIV_STREAM_ON) {
      MCT_PROF_LOG_BEG(PROF_MCT_STREAMON, "id", stream->streamid,
                    "type", stream->streaminfo.stream_type);
      event_data.type = MCT_EVENT_CONTROL_STREAMON;
      strlcpy (command, "STREAM-ON", sizeof(command));
    } else {
      MCT_PROF_LOG_BEG(PROF_MCT_STREAMOFF, "id", stream->streamid,
                    "type", stream->streaminfo.stream_type);
      event_data.type = MCT_EVENT_CONTROL_STREAMOFF;
      strlcpy (command, "STREAM-OFF", sizeof(command));
    }
    /* Clear super-param queue if this is the last stream to be streamed-off */
    if ( (1 == pipeline->stream_on_cnt) &&
      (data->command == MSM_CAMERA_PRIV_STREAM_OFF) ) {
      MCT_OBJECT_LOCK(pipeline);

      mct_list_free_all(pipeline->frame_num_idx_list,
        mct_stream_free_frame_num_list);
      pipeline->frame_num_idx_list = NULL;
      MCT_OBJECT_UNLOCK(pipeline);
      if(is_supermct_exist(pipeline))
      {
        supermct_lock_sof_update();
      }
      if (!MCT_QUEUE_IS_EMPTY(pipeline->super_param_queue)) {
        mct_pipeline_flush_super_param_queue(pipeline);
      }
      if(is_supermct_exist(pipeline))
      {
        supermct_unlock_sof_update();
      }
      pipeline->max_ack_id = 0;
      if(is_supermct_exist(pipeline) == TRUE)
      {
        supermct_update_ack_id(0);
      }

      mct_pipeline_get_stream_info_t param_info;
      mct_stream_t *parm_stream = NULL;
      param_info.check_type  = CHECK_INDEX;
      param_info.stream_index  = MCT_SESSION_STREAM_ID;
      parm_stream = mct_pipeline_get_stream(pipeline, &param_info);
      if (!parm_stream) {
         CLOGE(CAM_MCT_MODULE, "Couldn't find session stream");
         return FALSE;
      }
      parm_stream->state = MCT_ST_STATE_IDLE;
    }
    event_data.size = 0;
    event_data.control_event_data = (void *)&stream->streaminfo;
    CLOGI(CAM_MCT_MODULE, " %s on stream 0x%x stream type=%d",
      command, stream->streaminfo.identity, stream->streaminfo.stream_type);

    ret = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
      (pack_identity(MCT_PIPELINE_SESSION(pipeline), data->stream_id)),
       MCT_EVENT_DOWNSTREAM, &event_data, &cmd_event);
    if (ret == FALSE) {
          CLOGE(CAM_MCT_MODULE, "Error in packing event");
          break;
    }

    if (pipeline->send_event) {
      pthread_mutex_lock(&pipeline->stream_ops_lock);
      ret = pipeline->send_event(pipeline, data->stream_id, &cmd_event);
      pthread_mutex_unlock(&pipeline->stream_ops_lock);
      if (TRUE == ret) {
        CLOGI(CAM_MCT_MODULE, " %s success for stream 0x%x, stream type=%d",
           command, stream->streaminfo.identity,
          stream->streaminfo.stream_type);
        if (data->command == MSM_CAMERA_PRIV_STREAM_ON) {
          stream->state = MCT_ST_STATE_RUNNING;
          if ((stream->streaminfo.stream_type != CAM_STREAM_TYPE_METADATA) &&
            (stream->streaminfo.stream_type != CAM_STREAM_TYPE_OFFLINE_PROC)) {
              pipeline->stream_on_cnt++;
          }
        } else {
          stream->state = MCT_ST_STATE_IDLE;
          if ((stream->streaminfo.stream_type != CAM_STREAM_TYPE_METADATA) &&
            (stream->streaminfo.stream_type != CAM_STREAM_TYPE_OFFLINE_PROC)) {
            pipeline->stream_on_cnt--;
            if (0 == pipeline->stream_on_cnt) {
              if (pipeline->bus) {
                pipeline->bus->prev_sof_id = 0;  /* Reset SOF tracker */
              }
            }
          }
        }
      } else {
        CLOGE(CAM_MCT_MODULE, " %s failed for stream 0x%x, stream type=%d",
         command, stream->streaminfo.identity,
        stream->streaminfo.stream_type);

        /* if stream on fails send stream off command */
        if (data->command == MSM_CAMERA_PRIV_STREAM_ON) {
        stream->state = MCT_ST_STATE_RUNNING;
        pipeline->stream_on_cnt++;

        event_data.type = MCT_EVENT_CONTROL_STREAMOFF;
        strlcpy (command, "STREAM-OFF", sizeof(command));
        event_data.size = 0;
        event_data.control_event_data = (void *)&stream->streaminfo;
        CDBG_HIGH("%s: %s on stream 0x%x stream type=%d",
        __func__, command, stream->streaminfo.identity,
        stream->streaminfo.stream_type);

        ret = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
        (pack_identity(MCT_PIPELINE_SESSION(pipeline), data->stream_id)),
         MCT_EVENT_DOWNSTREAM, &event_data,&cmd_event);

        if (ret == FALSE) {
              CLOGE(CAM_MCT_MODULE, "Error in packing event");
              break;
        }

        ret = pipeline->send_event(pipeline, data->stream_id, &cmd_event);
        if (ret == FALSE) {
              CLOGE(CAM_MCT_MODULE, "Failure in sending event");
              return FALSE;
        }
        }
      }
    }
    else {
      CLOGE(CAM_MCT_MODULE, "send_event not registered");
      ret = FALSE;
    }
    MCT_PROF_LOG_END();
  }
    break;

  case MSM_CAMERA_PRIV_DEL_STREAM: {
    CLOGI(CAM_MCT_MODULE,
          "Issuing DEL_STREAM on stream 0x%x and stream type=%d",
          stream->streaminfo.identity, stream->streaminfo.stream_type);
    if (pipeline->deferred_state != CAM_DEFER_DEFAULT &&
       stream->streaminfo.stream_type == CAM_STREAM_TYPE_METADATA) {
        CLOGI(CAM_MCT_MODULE, "In defer state, flush the queue.");
        ret = mct_pipeline_flush_defer_queue(pipeline);
        pipeline->deferred_state = CAM_DEFER_DEFAULT;
    }

    if (stream->streaminfo.stream_type == CAM_STREAM_TYPE_METADATA &&
      pipeline->hal_version == CAM_HAL_V3) {
      pipeline->module_hw_state = MCT_MODULE_STATE_NONE;
    }

    if (pipeline->remove_stream) {
      cam_stream_type_t stream_type = stream->streaminfo.stream_type;
      unsigned int identity =  stream->streaminfo.identity;

      MCT_PROF_LOG_BEG(PROF_MCT_STREAMDEL, "id", stream->streamid,
                    "type", stream->streaminfo.stream_type);
      ret = pipeline->remove_stream(pipeline, stream);
      if (TRUE == ret) {
        CLOGI(CAM_MCT_MODULE,
              "Stream 0x%x and stream type=%d, successfully deleted",
              identity, stream_type);
      } else {
        CLOGI(CAM_MCT_MODULE,
              "Failure in deleting stream 0x%x, stream type=%d",
              identity, stream_type);
        ret = FALSE;
      }
      MCT_PROF_LOG_END();
    } else {
      CLOGE(CAM_MCT_MODULE, "remove_stream not registered");
      ret = FALSE;
    }
  }
    break;

  case CAM_PRIV_PARM: {
    MCT_PROF_LOG_BEG(PROF_MCT_CAM_PRIV_PARAM);
    if ((pipeline->stream_on_cnt && pipeline->hal_version == CAM_HAL_V1 &&
      (((pipeline->module_hw_state == MCT_MODULE_STATE_AWAKE) &&
      (pipeline->current_hw_state_requested != MCT_MODULE_STATE_ASLEEP))||
      ((pipeline->state_update_count > 0) &&
      (pipeline->current_hw_state_requested == MCT_MODULE_STATE_AWAKE )))) ||
      (pipeline->stream_on_cnt && pipeline->hal_version == CAM_HAL_V3)) {
        ret = mct_pipeline_send_ctrl_events(pipeline,
                                 MCT_EVENT_CONTROL_SET_PARM, 0);
    } else {
      ret = mct_pipeline_send_ctrl_events(pipeline,
                                 MCT_EVENT_CONTROL_SET_PARM, 1);
    }
    MCT_PROF_LOG_END();
  }
    break;

  case CAM_PRIV_STREAM_PARM: {
    mct_event_t cmd_event;
    mct_event_control_t event_data;
    cam_stream_info_t *stream_info_buf;

    MCT_PROF_LOG_BEG(PROF_MCT_CAM_STREAM_PARAM, "stream type",
                 stream->streaminfo.stream_type);

    ret = FALSE;
    stream_info_buf = MCT_STREAM_STREAMINFO(stream);
    if (!stream_info_buf) {
      CLOGE(CAM_MCT_MODULE, "Stream Info buffer is missing");
      break;
    }

    event_data.type = MCT_EVENT_CONTROL_PARM_STREAM_BUF;
    event_data.control_event_data = (void *)&stream->streaminfo.parm_buf;
    event_data.size = sizeof(cam_stream_parm_buffer_t);
    stream->streaminfo.parm_buf = stream_info_buf->parm_buf;
    ret = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
      (pack_identity(MCT_PIPELINE_SESSION(pipeline), stream->streamid)),
      MCT_EVENT_DOWNSTREAM, &event_data, &cmd_event);
    if (ret == FALSE) {
          CLOGE(CAM_MCT_MODULE, "Error in packing event");
          break;
    }

    if (pipeline->send_event)
      ret = pipeline->send_event(pipeline, data->stream_id, &cmd_event);
    if (TRUE == ret)
      stream_info_buf->parm_buf.reprocess.ret_val =
          stream->streaminfo.parm_buf.reprocess.ret_val;
    MCT_PROF_LOG_END();
  }
  break;

  case CAM_PRIV_PREPARE_SNAPSHOT:
  case CAM_PRIV_START_ZSL_SNAPSHOT:
  case CAM_PRIV_STOP_ZSL_SNAPSHOT:
  case CAM_PRIV_DO_AUTO_FOCUS:
  case CAM_PRIV_CANCEL_AUTO_FOCUS: {
    mct_event_t cmd_event, *p_cmd_event;
    mct_event_control_t event_data;
    boolean send_immediately = TRUE;
    void *p_arg_data;

    p_cmd_event = &cmd_event;
    event_data.control_event_data = &(data->arg_value);
    event_data.size = sizeof(unsigned int);

    if (data->command == CAM_PRIV_DO_AUTO_FOCUS) {
      event_data.type = MCT_EVENT_CONTROL_DO_AF;
    } else if (data->command == CAM_PRIV_CANCEL_AUTO_FOCUS) {
      event_data.type = MCT_EVENT_CONTROL_CANCEL_AF;
    } else if (data->command == CAM_PRIV_PREPARE_SNAPSHOT) {
      event_data.type = MCT_EVENT_CONTROL_PREPARE_SNAPSHOT;
    } else if (data->command == CAM_PRIV_START_ZSL_SNAPSHOT) {
      event_data.type = MCT_EVENT_CONTROL_START_ZSL_SNAPSHOT;
    } else if (data->command == CAM_PRIV_STOP_ZSL_SNAPSHOT) {
      event_data.type = MCT_EVENT_CONTROL_STOP_ZSL_SNAPSHOT;
    }

    p_cmd_event = malloc(sizeof(mct_event_t));
    p_arg_data = malloc(sizeof(data->arg_value));
    if(p_cmd_event && p_arg_data) {
      send_immediately = FALSE;
      memcpy(p_arg_data, &data->arg_value, sizeof(data->arg_value));
      event_data.control_event_data = p_arg_data;
      event_data.size = 0;
    } else {
      if (p_cmd_event) {
        free(p_cmd_event);
      }
      if (p_arg_data) {
        free(p_arg_data);
      }
      /* if cannot allocate - send parameter immediately */
      p_cmd_event = &cmd_event;
    }

    MCT_PROF_LOG_BEG(PROF_MCT_CAM_COMMAND, "command", data->command);

    ret = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
      (pack_identity(MCT_PIPELINE_SESSION(pipeline), stream->streamid)),
      MCT_EVENT_DOWNSTREAM, &event_data, p_cmd_event);

    if (ret == FALSE) {
      CLOGE(CAM_MCT_MODULE, "Error in packing event");
      break;
    }


    if(send_immediately == TRUE) {
      if (pipeline->send_event)
        ret = pipeline->send_event(pipeline, stream->streamid, p_cmd_event);
      else
        ret = FALSE;
    } else {
      CLOGI(CAM_MCT_MODULE, "store param event: 0x%x", event_data.type);
      mct_pipeline_store_special_event(pipeline, p_cmd_event);
    }

    data->ret_value = *((uint32_t *)event_data.control_event_data);

    MCT_PROF_LOG_END();

  }
    break;

  case CAM_PRIV_DUAL_CAM_CMD:{
    ret = mct_pipeline_process_dual_cam_cmd(pipeline);
  }
    break;
  default:
    ret = TRUE;
    break;
  }

  return ret;
}

/** mct_pipeline_process_get:
 *    @
 *    @
 *
 **/
static boolean mct_pipeline_process_get(struct msm_v4l2_event_data *data,
  mct_pipeline_t *pipeline)
{
  boolean      ret = TRUE;
  mct_stream_t *stream = NULL;
  mct_pipeline_get_stream_info_t info;

  switch (data->command) {
  case MSM_CAMERA_PRIV_QUERY_CAP: {
    /* for queryBuf */
    ret &= mct_list_traverse(pipeline->modules, mct_pipeline_query_modules,
             pipeline);
    if (!pipeline->query_buf || !pipeline->modules) {
      CLOGE(CAM_MCT_MODULE,
            "NULL ptr detected! query_buf = [%p] module list = [%p]",
            pipeline->query_buf, pipeline->modules);
      ret = FALSE;
    } else {
      /* fill up HAL's query buffer with query_data
         extracted during start_session */
        ret = mct_pipeline_populate_query_cap_buffer(pipeline);
    }
  }
    break;

  case CAM_PRIV_PARM: {
    if (pipeline->deferred_state == CAM_DEFER_START) {
      CLOGE(CAM_MCT_MODULE, "Not processing get_params in defer state");
      ret = FALSE;
      break;
    }
    ret = mct_pipeline_send_ctrl_events(pipeline,
                            MCT_EVENT_CONTROL_GET_PARM, 1);
  }
    break;

  case CAM_PRIV_STREAM_PARM: {
    mct_event_t cmd_event;
    mct_event_control_t event_data;
    cam_stream_info_t *stream_info_buf;
    info.check_type   = CHECK_INDEX;
    info.stream_index = data->stream_id;
    stream = mct_pipeline_get_stream(pipeline, &info);
    if (!stream) {
      CLOGE(CAM_MCT_MODULE, "Couldn't find stream with id %d",
         info.stream_index);
      return FALSE;
    }
    stream_info_buf = MCT_STREAM_STREAMINFO(stream);
    if (!stream_info_buf) {
      CLOGE(CAM_MCT_MODULE, "Stream Info buffer is missing");
      break;
    }

    event_data.type = MCT_EVENT_CONTROL_PARM_STREAM_BUF;
    event_data.control_event_data = (void *)&stream->streaminfo.parm_buf;
    event_data.size = sizeof(cam_stream_parm_buffer_t);

    stream->streaminfo.parm_buf = stream_info_buf->parm_buf;
    ret = mct_pipeline_pack_event(MCT_EVENT_CONTROL_CMD,
      (pack_identity(MCT_PIPELINE_SESSION(pipeline), stream->streamid)),
      MCT_EVENT_DOWNSTREAM, &event_data, &cmd_event);
    if (ret == FALSE) {
      CLOGE(CAM_MCT_MODULE, "Error in packing event");
      break;
    }

    if (pipeline->send_event)
      ret = pipeline->send_event(pipeline, data->stream_id, &cmd_event);
    if (TRUE == ret)
      stream_info_buf->parm_buf =
          stream->streaminfo.parm_buf;
  }
    break;

  default:
    ret = FALSE;
    break;
  }

  return ret;
}

/** mct_pipeline_process_serv_msg:
 *    @
 *    @
 *
 **/
static boolean mct_pipeline_process_serv_msg(void *message,
  mct_pipeline_t *pipeline)
{
  struct v4l2_event *msg = (struct v4l2_event *)message;
  boolean           ret = TRUE;
  struct msm_v4l2_event_data *data =
    (struct msm_v4l2_event_data *)(msg->u.data);

  if (!message || !pipeline || data->session_id != pipeline->session)
    return FALSE;

  switch (msg->id) {
  case MSM_CAMERA_SET_PARM:
    ret = mct_pipeline_process_set(data, pipeline);
    break;

  case MSM_CAMERA_GET_PARM:
    /* process config_w  */
    ret = mct_pipeline_process_get(data, pipeline);
    break;

  case MSM_CAMERA_DEL_SESSION: {
    /* for session ending:
     * a session has ONLY one child */
    if (MCT_PIPELINE_CHILDREN(pipeline)) {
      MCT_OBJECT_LOCK(pipeline);

      /* Delete streams */
      mct_list_free_all(MCT_PIPELINE_CHILDREN(pipeline),
        mct_pipeline_delete_stream);

      MCT_PIPELINE_CHILDREN(pipeline) = NULL;
      MCT_PIPELINE_NUM_CHILDREN(pipeline) = 0;

      MCT_OBJECT_UNLOCK(pipeline);

    }
  }
    break;

  default:
    /* something wrong */
    ret = FALSE;
    break;
  } /* switch (msg->msg_type) */

  return ret;
}

boolean mct_pipeline_free_defer_queue(void *data,
  void *user_data __unused)
{
  mct_pipeline_defer_cmd_t *payload = (mct_pipeline_defer_cmd_t *)data;
  if (payload == NULL) {
    return TRUE;
  }

  if (payload->event_data.u.ctrl_event.size) {
    free(payload->event_data.u.ctrl_event.control_event_data);
    payload->event_data.u.ctrl_event.control_event_data = NULL;
  }

  if (payload) {
    free(payload);
    payload = NULL;
  }

  return TRUE;
}


/** mct_pipeline_flush_defer_queue:
 *    @pipeline: MCT Pipeline Object
 *
 * Flush deferred commands.
 * Return: TRUE/FALSE
 *
 * This function executes in Media Controller's thread context
 **/

boolean mct_pipeline_flush_defer_queue(mct_pipeline_t *pipeline)
{
  mct_pipeline_defer_cmd_t *payload = NULL;

  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Pipeline not present");
    return FALSE;
  }

  pthread_mutex_lock(&pipeline->defer_q_lock);
  if (pipeline->defer_queue &&
    !MCT_QUEUE_IS_EMPTY(pipeline->defer_queue)) {
    mct_queue_flush(pipeline->defer_queue, mct_pipeline_free_defer_queue);
  }
  pipeline->deferred_state = CAM_DEFER_FLUSH;
  pthread_mutex_unlock(&pipeline->defer_q_lock);
  return TRUE;
}


/** mct_pipeline_process_defer_queue:
 *    @pipeline: MCT Pipeline Object
 *    @msg: message object from imaging server/HAL
 *
 * Processes deferred commands.
 * Return: TRUE/FALSE
 *
 * This function executes in Media Controller's thread context
 **/

boolean mct_pipeline_process_defer_queue(mct_pipeline_t
  *pipeline)
{

  boolean ret = TRUE;
  mct_pipeline_defer_cmd_t *payload = NULL;
  mct_stream_t *stream = NULL;

  CLOGI(CAM_MCT_MODULE, "Processing deferred queue");

  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Pipeline NULL");
    return FALSE;
  }

  pthread_mutex_lock(&pipeline->defer_q_lock);
  pipeline->deferred_state = CAM_DEFER_PROCESS;
  while (1) {
    payload = (mct_pipeline_defer_cmd_t *)
      mct_queue_pop_head(pipeline->defer_queue);

    if (!payload) {
      CLOGI(CAM_MCT_MODULE, "All defer commands processed");
      break;
    }
    stream = payload->stream;
    if ((payload->event_data.type == MCT_EVENT_CONTROL_CMD)
        && (payload->event_data.u.ctrl_event.type == MCT_EVENT_CONTROL_START_LINK)) {
      ret = mct_stream_link_module_array(stream,
          payload->event_data.u.ctrl_event.control_event_data);
    } else if (stream && stream->send_event) {
      ret = stream->send_event(stream, &(payload->event_data));
    }

    mct_pipeline_free_defer_queue(payload, NULL);

    if (ret == FALSE) {
      CLOGE(CAM_MCT_MODULE, "Failed to process a defer command, sending error");
      mct_controller_send_cb(pipeline, MCT_BUS_MSG_SEND_HW_ERROR);
      break;
    }
  }
  pthread_mutex_unlock(&pipeline->defer_q_lock);
  mct_pipeline_flush_defer_queue(pipeline);
  return ret;
}

/** mct_controller_proc_defer_cmd:
 *    @pipeline: MCT Pipeline Object
 *    @msg: message object from imaging server/HAL
 *
 * Caches deferred commands in queue to be processed later.
 * Return: TRUE/FALSE
 *
 * This function executes in Media Controller's thread context
 **/
boolean mct_pipeline_push_defer_cmd(mct_pipeline_t * pipeline,
  mct_event_t *event, mct_stream_t *stream)
{
  boolean ret = TRUE;
  mct_pipeline_defer_cmd_t *payload = NULL;
  void *control_event_data = NULL;

  if (!event || event->type != MCT_EVENT_CONTROL_CMD) {
    return FALSE;
  }
  /*Send all commands to defer queue */
  payload = (mct_pipeline_defer_cmd_t *)malloc(sizeof(mct_pipeline_defer_cmd_t));
  if (!payload) {
    CLOGE(CAM_MCT_MODULE, "Malloc failed for defer command");
    return FALSE;
  }

  memset(payload, 0, sizeof(mct_pipeline_defer_cmd_t));
  payload->event_data = *event;
  payload->stream = stream;
  if (event->u.ctrl_event.size) {
    payload->event_data.u.ctrl_event.control_event_data
      = malloc(event->u.ctrl_event.size);
   memcpy(payload->event_data.u.ctrl_event.control_event_data,
     event->u.ctrl_event.control_event_data, event->u.ctrl_event.size);
  } else {
   payload->event_data.u.ctrl_event.control_event_data =
     event->u.ctrl_event.control_event_data;
  }
  pthread_mutex_lock(&pipeline->defer_q_lock);
  mct_queue_push_tail(pipeline->defer_queue, payload);
  pthread_mutex_unlock(&pipeline->defer_q_lock);
  return ret;
}

/** mct_pipeline_process_bus_msg:
 *    @
 *    @
 *
 **/
static boolean mct_pipeline_process_bus_msg(void *msg,
  mct_pipeline_t *pipeline)

{
  boolean ret = TRUE;
  mct_bus_msg_t *bus_msg = (mct_bus_msg_t *)msg;
  mct_stream_t *stream = NULL;
  mct_pipeline_get_stream_info_t info;

  info.check_type   = CHECK_TYPE;
  info.stream_type  = CAM_STREAM_TYPE_METADATA;
  stream = mct_pipeline_get_stream(pipeline, &info);
  if (stream && stream->state == MCT_ST_STATE_RUNNING) {
    ret = mct_stream_process_bus_msg(stream, bus_msg);
  } else {
    CLOGD(CAM_MCT_MODULE, "Can't find stream");
  }
  return ret;
}

/** mct_pipeline_init_mutex:
 *    @
 *
 **/
static void mct_pipeline_init_mutex(pthread_mutex_t *mutex)
{
  pthread_mutexattr_t attr;

  pthread_mutexattr_init(&attr);
  pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
  pthread_mutex_init(mutex, &attr);
  pthread_mutexattr_destroy(&attr);
}

/** mct_pipeline_get_module_num:
 *    @data1: void* pointer to the module being processed
 *    @data2: void* pointer to the pipeline object
 *
 *  Traverse module list to get the module number.
 *
 *  Return: Ture.
 **/
static boolean mct_pipeline_get_module_num(void *data1 __unused, void *data2)
{
  mct_pipeline_t *pipeline = (mct_pipeline_t *)data2;
  pipeline->thread_data.modules_num++;

  return TRUE;
}

/** mct_pipeline_sync_init:
 *    @sync_data: mct_sync_data_t*
 *
 **/
int mct_pipeline_sync_init(mct_sync_data_t *sync_data)
{
  int rc = 0;
  pthread_mutex_init(&sync_data->mutex, NULL);
  rc = pthread_cond_init(&sync_data->cond, NULL);
  sync_data->cond_posted = 0;
  return rc;
}

/** mct_pipeline_sync_post:
 *    @sync_data: mct_sync_data_t*
 *
 **/
int mct_pipeline_sync_post(mct_sync_data_t *sync_data)
{
  int rc = 0;
  pthread_mutex_lock(&sync_data->mutex);
  sync_data->cond_posted++;
  rc = pthread_cond_signal(&sync_data->cond);
  pthread_mutex_unlock(&sync_data->mutex);
  return rc;
}

/** mct_pipeline_sync_pend:
 *    @sync_data: mct_sync_data_t*
 *
 **/
int mct_pipeline_sync_pend(mct_sync_data_t *sync_data)
{
  int rc = 0;

  pthread_mutex_lock(&sync_data->mutex);
  if (0 == sync_data->cond_posted) {
    rc = pthread_cond_wait(&sync_data->cond, &sync_data->mutex);
  }
  sync_data->cond_posted--;
  pthread_mutex_unlock(&sync_data->mutex);
  return rc;
}

/** mct_pipeline_sync_destroy:
 *    @sync_data: mct_sync_data_t*
 *
 **/
int mct_pipeline_sync_destroy(mct_sync_data_t *sync_data)
{
  int rc = 0;
  rc = pthread_mutex_destroy(&sync_data->mutex);
  rc |= pthread_cond_destroy(&sync_data->cond);
  sync_data->cond_posted = 0;
  return rc;
}

/** mct_pipeline_stop_session_thread:
 *    @data: void* pointer to the mct_pipeline_thread_data
 *
 *  Thread implementation to stop camera module.
 *
 *  Return: NULL
 **/
static void* mct_pipeline_stop_session_thread(void *data)
{
  mct_pipeline_thread_data_t *thread_data = (mct_pipeline_thread_data_t*)data;
  mct_module_t *module = thread_data->module;
  unsigned int session_id = thread_data->session_id;
  boolean rc = TRUE;

  ATRACE_BEGIN_SNPRINTF(30, "Camera:MCTModStop:%s",
             MCT_MODULE_NAME(thread_data->module));

  pthread_mutex_lock(&thread_data->mutex);
  pthread_cond_signal(&thread_data->cond_v);
  pthread_mutex_unlock(&thread_data->mutex);

  CLOGI(CAM_MCT_MODULE, "Stop module name: %s - E",
     MCT_MODULE_NAME(module));
  if (module->stop_session) {
    rc = module->stop_session(module, session_id);
  }
  if(rc == FALSE) {
    CLOGE(CAM_MCT_MODULE, "Failed to stop module name: %s",
       MCT_MODULE_NAME(module));
  }
  CLOGI(CAM_MCT_MODULE, "Stop module name: %s - X", MCT_MODULE_NAME(module));

  rc = mct_pipeline_sync_post(&thread_data->sync);
  if (rc) {
    CLOGE(CAM_MCT_MODULE, "Failed to post condition!");
  }

  ATRACE_END();

  return NULL;
}

/** mct_pipeline_modules_stop:
 *    @data1: void* pointer to the module being processed
 *    @data2: void* pointer to the pipeline object
 *
 *  Create thread for each module for stop session.
 *
 *  Return: True on success
 **/
static boolean mct_pipeline_modules_stop(void *data1, void *data2)
{
  int rc = 0;
  pthread_attr_t attr;
  char thread_name[16];
  mct_pipeline_t *pipeline = (mct_pipeline_t *)data2;
  mct_pipeline_thread_data_t *thread_data = &(pipeline->thread_data);
  thread_data->module = (mct_module_t *)data1;
  thread_data->session_id = pipeline->session;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_mutex_lock(&thread_data->mutex);
  rc = pthread_create(&pipeline->thread_data.pid, &attr,
    &mct_pipeline_stop_session_thread, (void *)thread_data);
  snprintf(thread_name, sizeof(thread_name), "CAM_stop%s",
           MCT_MODULE_NAME(thread_data->module));
  if(!rc) {
    pthread_setname_np(pipeline->thread_data.pid,thread_name);
    pthread_cond_wait(&thread_data->cond_v, &thread_data->mutex);
  }
  pthread_mutex_unlock(&thread_data->mutex);

  return TRUE;
}

/** mct_pipeline_stop_stream_internal:
 *    @data1: void* pointer to the pipeline object
 *
 *  Delete Session based stream.
 *
 *  Return: NONE
 **/
static void mct_pipeline_stop_stream_internal(mct_pipeline_t *pipeline)
{
  struct v4l2_event msg;
  struct msm_v4l2_event_data *data =
          (struct msm_v4l2_event_data*)&msg.u.data[0];

  if (pipeline->unmap_parm(pipeline) == FALSE) {
    CLOGE(CAM_MCT_MODULE, "stream unmap_parm failed");
    return;
  }
  if (pipeline->session_stream.session_stream_info) {
    free(pipeline->session_stream.session_stream_info);
    pipeline->session_stream.session_stream_info = NULL;
  }

  msg.id = MSM_CAMERA_SET_PARM;
  data->command =   MSM_CAMERA_PRIV_DEL_STREAM;
  data->session_id = pipeline->session;
  data->stream_id = MCT_SESSION_STREAM_ID;
  if (pipeline->process_serv_msg(&msg, pipeline) == FALSE) {
    CLOGE(CAM_MCT_MODULE, "Delete session stream failed");
    return;
  }
}

/** mct_pipeline_start_stream_internal:
 *    @data1: pointer to pipeline object
 *
 *  start Session based stream.
 *
 *  Return: TRUE on success, FALSE on failure
 **/
static boolean mct_pipeline_start_stream_internal(mct_pipeline_t *pipeline)
{
  boolean rc = TRUE;
  mct_stream_t *stream = NULL;
  mct_pipeline_get_stream_info_t info;
  struct v4l2_event msg;
  struct msm_v4l2_event_data *data =
          (struct msm_v4l2_event_data*)&msg.u.data[0];
  cam_stream_info_t *stream_info;

  /*create new session based stream */
  data->session_id = pipeline->session;
  data->stream_id = MCT_SESSION_STREAM_ID;
  if (pipeline->add_stream) {
    CLOGI(CAM_MCT_MODULE,
          "Adding session stream streamid= 0x%x for session=%d",
          data->stream_id, pipeline->session);
    rc  = pipeline->add_stream(pipeline, data->stream_id);
  }
  if (FALSE == rc) {
    CLOGE(CAM_MCT_MODULE, "session stream creation failed");
    return FALSE;
  }

  CLOGD(CAM_MCT_MODULE, "Allocating stream buffer");
  pipeline->session_stream.session_stream_info =
                                  malloc(sizeof(cam_stream_info_t));
  if (!pipeline->session_stream.session_stream_info) {
    CLOGE(CAM_MCT_MODULE, "Session_stream_info allocation failed");
    return FALSE;
  }

  CLOGD(CAM_MCT_MODULE, "Mapping stream buffer");
  if (pipeline->map_parm(pipeline) == FALSE) {
    CLOGE(CAM_MCT_MODULE, "stream map_param failed");
    return FALSE;
  }

  stream_info =
     (cam_stream_info_t *)pipeline->session_stream.session_stream_info;
  memset(stream_info, 0, sizeof(cam_stream_info_t));
  stream_info->stream_type = CAM_STREAM_TYPE_PARM;
  stream_info->fmt = CAM_FORMAT_YUV_420_NV12;
  stream_info->dim.width = 0;
  stream_info->dim.height = 0;
  stream_info->streaming_mode = CAM_STREAMING_MODE_CONTINUOUS;
  stream_info->buf_planes.plane_info.num_planes= 0;
  stream_info->num_bufs = 0;

  data->session_id = pipeline->session;
  data->stream_id = MCT_SESSION_STREAM_ID;

  info.check_type = CHECK_INDEX;
  info.stream_index = data->stream_id;

  /*Getting stream pointer based on stream id */
  stream = mct_pipeline_get_stream(pipeline, &info);

  if (!stream) {
    CLOGE(CAM_MCT_MODULE, "Session stream not present");
    return FALSE;
  }

  CLOGI(CAM_MCT_MODULE, "Linking session stream for session %d",
     pipeline->session);
  (MCT_STREAM_LINK(stream)) ?
  (rc = (MCT_STREAM_LINK(stream))(stream)) : (rc = FALSE);
  if (FALSE == rc) {
    CLOGE(CAM_MCT_MODULE, "Session stream linking failed for session %d",
       pipeline->session);
    return FALSE;
  }
  CLOGI(CAM_MCT_MODULE, "Session stream linked successfully session %d",
     pipeline->session);

  return TRUE;
}

/** mct_pipeline_start_session_thread:
 *    @data: void* pointer to the mct_pipeline_thread_data
 *
 *  Thread implementation to start camera module.
 *
 *  Return: NULL
 **/
static void* mct_pipeline_start_session_thread(void *data)
{
  mct_pipeline_thread_data_t *thread_data = (mct_pipeline_thread_data_t*)data;
  mct_module_t *module = thread_data->module;
  unsigned int session_id = thread_data->session_id;
  boolean rc = FALSE;

  ATRACE_BEGIN_SNPRINTF(30, "Camera:MCTModStart:%s",
             MCT_MODULE_NAME(thread_data->module));

  CLOGI(CAM_MCT_MODULE, "E %s" , MCT_MODULE_NAME(thread_data->module));

  pthread_mutex_lock(&thread_data->mutex);
  pthread_cond_signal(&thread_data->cond_v);
  pthread_mutex_unlock(&thread_data->mutex);
  if (module->start_session) {
    CLOGI(CAM_MCT_MODULE, "Calling start_session on Module %s",
          MCT_MODULE_NAME(module));
    rc = module->start_session(module, session_id);
    CLOGI(CAM_MCT_MODULE, "Module %s start_session rc = %d",
       MCT_MODULE_NAME(module), rc);
  }
  pthread_mutex_lock(&thread_data->mutex);
  thread_data->started_num++;
  if (rc == TRUE)
    thread_data->started_num_success++;

  CLOGI(CAM_MCT_MODULE, "started_num = %d, success = %d",
     thread_data->started_num, thread_data->started_num_success);

  if(thread_data->started_num == thread_data->modules_num)
    pthread_cond_signal(&thread_data->cond_v);
  pthread_mutex_unlock(&thread_data->mutex);

  ATRACE_END();
  CLOGI(CAM_MCT_MODULE, "X %s" , MCT_MODULE_NAME(module));

  return NULL;
}
/** mct_pipeline_modules_start:
 *    @data1: void* pointer to the module being processed
 *    @data2: void* pointer to the pipeline object
 *
 *  Create thread for each module for start session.
 *
 *  Return: True on success
 **/
static boolean mct_pipeline_modules_start(void *data1, void *data2)
{
  int rc = 0;
  pthread_attr_t attr;
  char thread_name[20];
  mct_pipeline_t *pipeline = (mct_pipeline_t *)data2;
  mct_pipeline_thread_data_t *thread_data = &(pipeline->thread_data);
  thread_data->module = (mct_module_t *)data1;
  thread_data->session_id = pipeline->session;

  pthread_attr_init(&attr);
  pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
  pthread_mutex_lock(&thread_data->mutex);
  rc = pthread_create(&pipeline->thread_data.pid, &attr,
    &mct_pipeline_start_session_thread, (void *)thread_data);
  snprintf(thread_name, sizeof(thread_name), "CAM_start%s",
           MCT_MODULE_NAME(thread_data->module));
  if(!rc) {
    pthread_setname_np(pipeline->thread_data.pid,thread_name);
    pthread_cond_wait(&thread_data->cond_v, &thread_data->mutex);
  }
  pthread_mutex_unlock(&thread_data->mutex);

  return TRUE;
}

/** mct_pipeline_stop_session:
 *    @pipeline: mct_pipeline_t object
 *
 *  Pipeline stop session, stop each camera module.
 *
 *  Return: no return value
 **/
void mct_pipeline_stop_session(mct_pipeline_t *pipeline)
{

  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "NULL pipeline ptr");
    return;
  }
  CLOGI(CAM_MCT_MODULE, "Initiating stop_session on session %d",
    pipeline->session);

  mct_pipeline_stop_stream_internal(pipeline);
  pipeline->session_stream.state == MCT_ST_STATE_IDLE;

  if (mct_pipeline_sync_init(&pipeline->thread_data.sync)) {
    CLOGE(CAM_MCT_MODULE, "Error on creating sync data!");
    return;
  }

  ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_MCT_STOP_SESSION);
  mct_list_traverse(pipeline->modules, mct_pipeline_modules_stop,
    pipeline);
  while (pipeline->thread_data.started_num) {
    if (mct_pipeline_sync_pend(&pipeline->thread_data.sync)) {
      CLOGE(CAM_MCT_MODULE, "Condition pend error - left modules: %d",
        pipeline->thread_data.started_num);
      break;
    }

    pipeline->thread_data.started_num--;
    CLOGI(CAM_MCT_MODULE,
          "Modules left: %d", pipeline->thread_data.started_num);

  }
  pipeline->session_data.max_pipeline_frame_applying_delay = 0;
  pipeline->session_data.max_pipeline_meta_reporting_delay = 0;
  pthread_mutex_destroy(&pipeline->thread_data.mutex);
  pthread_cond_destroy(&pipeline->thread_data.cond_v);
  pthread_condattr_destroy(&pipeline->thread_data.condattr);
  mct_pipeline_sync_destroy(&pipeline->thread_data.sync);

  CLOGI(CAM_MCT_MODULE, "Stopped session %d successfully", pipeline->session);
  ATRACE_CAMSCOPE_END(CAMSCOPE_MCT_STOP_SESSION);
  return;
}

/** mct_pipeline_start_session:
 *    @pipeline: mct_pipeline_t object
 *
 *  Description:
 *    Starts a session on each module spawning individual threads.
 *    Also queries module capabilities and publishes session data
 *    back to modules.
 *
 *  Return: Status of start_session
 **/
cam_status_t mct_pipeline_start_session(mct_pipeline_t *pipeline)
{
  boolean rc;
  int ret;
  int rdi_standalone_streams = 0;
  struct timespec timeToWait;
  char prop[PROPERTY_VALUE_MAX];

  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "NULL pipeline ptr");
    return CAM_STATUS_FAILED;
  }

  property_get("persist.vendor.camera.mct.multirdi", prop, "0");
  rdi_standalone_streams = atoi (prop);

  ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_MCT_START_SESSION);
  pthread_mutex_init(&pipeline->thread_data.mutex, NULL);
  pthread_condattr_init(&pipeline->thread_data.condattr);
  pthread_condattr_setclock(&pipeline->thread_data.condattr, CLOCK_MONOTONIC);
  pthread_cond_init(&pipeline->thread_data.cond_v, &pipeline->thread_data.condattr);

  pipeline->thread_data.started_num = 0;
  pipeline->thread_data.modules_num = 0;
  pipeline->thread_data.started_num_success = 0;
  rc = mct_list_traverse(pipeline->modules, mct_pipeline_get_module_num,
    pipeline);
  rc &= mct_list_traverse(pipeline->modules, mct_pipeline_modules_start,
    pipeline);
  rc = mct_util_get_timeout(MCT_THREAD_TIMEOUT, &timeToWait);
  if (!rc) {
    CLOGE(CAM_MCT_MODULE, "Failed to obtain timeout interval");
    return CAM_STATUS_FAILED;
  }

  pthread_mutex_lock(&pipeline->thread_data.mutex);
  if (pipeline->thread_data.started_num!=pipeline->thread_data.modules_num) {
    ret = pthread_cond_timedwait(&pipeline->thread_data.cond_v,
      &pipeline->thread_data.mutex, &timeToWait);

    if (ETIMEDOUT == ret) {
      pthread_mutex_unlock(&pipeline->thread_data.mutex);
      CLOGE(CAM_MCT_MODULE, "Timeout in starting session on all modules");
      return CAM_STATUS_FAILED;
    }
  }
  pthread_mutex_unlock(&pipeline->thread_data.mutex);
  if (pipeline->thread_data.started_num_success !=
    pipeline->thread_data.modules_num) {
    CLOGE(CAM_MCT_MODULE, "start_session did not succeed on all modules");
      return CAM_STATUS_FAILED;
  }

  /*Getting data from module */
  rc &= mct_list_traverse(pipeline->modules, mct_pipeline_get_session_data,
          pipeline);

  /* Check feasibility of session based on current resource availability */
  if ( !pipeline->session_data.is_aux_camera &&
    (0 != pipeline->session_data.dual_vfe_enabled) &&
    (FORMAT_YCBCR != pipeline->session_data.sensor_format) ) {
    CLOGE(CAM_MCT_MODULE, "No VFE HW available for another pixel-stream");
    if (!rdi_standalone_streams) {
      return CAM_STATUS_BUSY;
    }
    else {
      CLOGI(CAM_MCT_MODULE, "Ignoring HW availability errors in multi-rdi mode.");
    }
  }

#if FIXED_PIPELINE_DELAY
  pipeline->session_data.max_pipeline_frame_applying_delay = 2;
  pipeline->session_data.max_pipeline_meta_reporting_delay = 1;
#endif
  pipeline->session_data.set_session_mask |= SESSION_DATA_SUPERSET;
  rc &= mct_list_traverse(pipeline->modules, mct_pipeline_set_session_data,
                          pipeline);
  pipeline->session_data.set_session_mask = 0;

  /*Updating the session stream state to starting */
  pipeline->session_stream.state = MCT_ST_STATE_STARTING;
  rc &= mct_pipeline_start_stream_internal(pipeline);

  ATRACE_CAMSCOPE_END(CAMSCOPE_MCT_START_SESSION);
  if (TRUE == rc)
    return CAM_STATUS_SUCCESS;
  else
    return CAM_STATUS_FAILED;
}

/** Name: mct_pipeline_new
 *
 *  Arguments/Fields:
 *    @session_idx: Index of new session
 *    @pController: Context of mct_controller object
 *
 *  Return:
 *    Pointer to newly created pipeline on Success, NULL on failure
 *  Description:
 *    Constructor method for mct_pipeline.
 *    Initializes internal fields. Also creates a new bus structure
 *    for this session
 **/
mct_pipeline_t* mct_pipeline_new (unsigned int session_idx,
  mct_controller_t *pController)
{
  mct_pipeline_t *pipeline;
  if (!pController) {
    CLOGE(CAM_MCT_MODULE, "NULL mct_controller object");
    return NULL;
  }
  pipeline = malloc(sizeof(mct_pipeline_t));
  if (!pipeline)
    return NULL;
  memset(pipeline, 0, sizeof(mct_pipeline_t));


  /* Initialize a recursive mutex for pipeline */
  mct_pipeline_init_mutex(MCT_OBJECT_GET_LOCK(pipeline));
  pipeline->session = session_idx;
  pipeline->controller = pController;

  pthread_mutex_init(&pipeline->stream_ops_lock, NULL);

  mct_object_t *obj = MCT_OBJECT_CAST(pipeline);
  obj->children = NULL;
  obj->childrennum = 0;

  pthread_mutex_init(&pipeline->super_param_q_lock, NULL);
  pipeline->super_param_queue = (mct_queue_t *)mct_queue_new;
  if (!pipeline->super_param_queue)
  {
    CLOGE(CAM_MCT_MODULE, "failed to create super_param queue");
    pthread_mutex_destroy(&pipeline->super_param_q_lock);
    free(pipeline);
    pipeline = NULL;
    return NULL;
  }
  memset(pipeline->super_param_queue, 0, sizeof(mct_queue_t));
  mct_queue_init(pipeline->super_param_queue);

  pthread_mutex_init(&pipeline->param_q_lock, NULL);
  pipeline->param_queue = (mct_queue_t *)mct_queue_new;
  if (!pipeline->param_queue)
  {
    CLOGE(CAM_MCT_MODULE, "failed to create param queue");
    free(pipeline->super_param_queue);
    pipeline->super_param_queue = NULL;
    free(pipeline);
    pipeline = NULL;
    return NULL;
  }
  memset(pipeline->param_queue, 0, sizeof(mct_queue_t));
  mct_queue_init(pipeline->param_queue);

  /* Create pipeline->bus */
  pipeline->bus = mct_bus_create(session_idx);
  if (!pipeline->bus) {
    CLOGE(CAM_MCT_MODULE, "Bus creation failed");
    free(pipeline->param_queue);
    pipeline->param_queue = NULL;
    free(pipeline->super_param_queue);
    pipeline->super_param_queue = NULL;
    free(pipeline);
    pipeline = NULL;
    return NULL;
  }
  pipeline->sleep_duration = MCT_PIPELINE_SLEEP_DURATION;
  pipeline->lpm_owner = 1;

  /* For SERV_MSG_BUF_MAPPING */
  pipeline->map_buf   = mct_pipeline_map_buf;

  /*For SERV_MSG_BUF_UNMAPPING */
  pipeline->unmap_buf = mct_pipeline_unmap_buf;

  /* For SERV_MSG_BUF_MAPPING */
  pipeline->map_parm   = mct_pipeline_map_parm;

  /*For SERV_MSG_BUF_UNMAPPING */
  pipeline->unmap_parm = mct_pipeline_unmap_parm;

  /* For case SERV_MSG_SET,SERV_MSG_GET, SERV_MSG_STREAMON, SERV_MSG_STREAMOFF,
    SERV_MSG_QUERY,SERV_MSG_CLOSE_SESSION */
  pipeline->process_serv_msg= mct_pipeline_process_serv_msg;
  pipeline->process_bus_msg = mct_pipeline_process_bus_msg;

  pipeline->add_stream    = mct_pipeline_add_stream;
  pipeline->remove_stream = mct_pipeline_remove_stream;
  pipeline->send_event    = mct_pipeline_send_event;
  pipeline->set_bus       = mct_pipeline_set_bus;
  pipeline->get_bus       = mct_pipeline_get_bus;
  pipeline->hal_version   = CAM_HAL_V1;

  return pipeline;
}

/** Name: mct_pipeline_flush_super_param_queue:
 *  Arguments/Fields:
 *    @pipeline: structure of mct_controller_t
 *  Description:
 *     Routine to flush any super events remaining in pipeline queue.
 *     Additionally sends super-events to downstream modules before clearing.
 **/
boolean mct_pipeline_flush_super_param_queue(mct_pipeline_t *pipeline)
{
  boolean ret = TRUE;
  mct_pipeline_get_stream_info_t info;
  mct_event_super_control_parm_t *super_event = NULL;
  mct_stream_t *parm_stream;
  uint32_t i;

  if (!pipeline) {
    CLOGE(CAM_MCT_MODULE, "Invalid pipeline ptr");
    return FALSE;
  }

  info.check_type   = CHECK_INDEX;
  info.stream_index  = MCT_SESSION_STREAM_ID;
  parm_stream = mct_pipeline_get_stream(pipeline, &info);
  if (!parm_stream) {
    CLOGE(CAM_MCT_MODULE, "FATAL: No session stream found");
    return FALSE;
  }

  /* First send out super param settings if any */
  do {
    pthread_mutex_lock(&pipeline->super_param_q_lock);
    super_event = (mct_event_super_control_parm_t *)
      mct_queue_pop_head(pipeline->super_param_queue);
    pthread_mutex_unlock(&pipeline->super_param_q_lock);

    if (super_event) {
      /*Apply super param settings to all modules */
      CLOGD(CAM_MCT_MODULE, "Dequeued super_event = [%p], frame_num [%d]",
         super_event, super_event->frame_number);
      ret = mct_stream_send_super_event(parm_stream, super_event);
      if (FALSE == ret) {
        CLOGE(CAM_MCT_MODULE, "Failed to send super-event [%p]", super_event);
      }

     /* Now clear super-event entries */
      for (i = 0; i < super_event->num_of_parm_events; i++) {
        if (super_event->parm_events[i].parm_data) {
          free(super_event->parm_events[i].parm_data);
          super_event->parm_events[i].parm_data = NULL;
        }
      }
      if (super_event->parm_events) {
        free(super_event->parm_events);
        super_event->parm_events = NULL;
      }
      free(super_event);
      super_event = NULL;
    } else {
      break;
    }
  } while (1);

  return ret;
}


/** mct_pipeline_free_param:
 *    @
 *    Description: Utility function to free data in queue entries
 **/
boolean mct_pipeline_free_param(void *data, void *user_data __unused)
{
  mct_event_t *pdata = (mct_event_t *)data;
  if (!pdata) {
    CLOGE(CAM_MCT_MODULE, "Invalid ptr");
    return FALSE;
  }
  if (pdata->type == MCT_EVENT_CONTROL_CMD) {
    free(pdata->u.ctrl_event.control_event_data);
  }

  free(data);
  return TRUE;
}


/** mct_pipeline_destroy:
 *    @
 *
 **/
void mct_pipeline_destroy(mct_pipeline_t *pipeline)
{
  CLOGI(CAM_MCT_MODULE, "E: session:%d", pipeline->session);
  MCT_OBJECT_LOCK(pipeline);

  /* Clear frame-num mapping table */
  mct_list_free_all(pipeline->frame_num_idx_list,
    mct_stream_free_frame_num_list);
  pipeline->frame_num_idx_list = NULL;

  /*Destroy param queue if it exists */
  pthread_mutex_lock(&pipeline->param_q_lock);
  if (!MCT_QUEUE_IS_EMPTY(pipeline->param_queue))
    mct_queue_free_all(pipeline->param_queue,
      mct_pipeline_free_param);
  else {
    free(pipeline->param_queue);
    pipeline->param_queue = NULL;
  }
  pthread_mutex_unlock(&pipeline->param_q_lock);
  pthread_mutex_destroy(&pipeline->param_q_lock);

  /*Destroy defer queue if it exists */
  pthread_mutex_lock(&pipeline->defer_q_lock);
  if (pipeline->defer_queue &&
    !MCT_QUEUE_IS_EMPTY(pipeline->defer_queue))
    mct_queue_free_all(pipeline->defer_queue,
      mct_pipeline_free_defer_queue);
  else {
    free(pipeline->defer_queue);
    pipeline->defer_queue= NULL;
  }
  pipeline->deferred_state = CAM_DEFER_DEFAULT;
  pthread_mutex_unlock(&pipeline->defer_q_lock);
  pthread_mutex_destroy(&pipeline->defer_q_lock);

  /* Delete all streams in pipeline */
  if (MCT_PIPELINE_CHILDREN(pipeline)) {
    mct_list_free_all(MCT_PIPELINE_CHILDREN(pipeline),
      mct_pipeline_delete_stream);
  }
  pipeline->stream_on_cnt = 0;
  MCT_PIPELINE_CHILDREN(pipeline) = NULL;
  MCT_PIPELINE_NUM_CHILDREN(pipeline) = 0;
  pthread_mutex_destroy(MCT_OBJECT_GET_LOCK(pipeline));

  /*unmap buffers if they still exist*/
  if (pipeline->config_parm) {
#ifdef DAEMON_PRESENT
    munmap(pipeline->config_parm, pipeline->config_parm_size);
    close(pipeline->config_parm_fd);
#endif
    pipeline->config_parm = NULL;
    pipeline->config_parm_size = 0;
  }
  if (pipeline->query_buf) {
#ifdef DAEMON_PRESENT
    munmap(pipeline->query_buf, pipeline->query_buf_size);
    close(pipeline->query_buf_fd);
#endif
    pipeline->query_buf = NULL;
    pipeline->query_buf_size = 0;
  }
  if (pipeline->related_sensors_sync_buf) {
#ifdef DAEMON_PRESENT
    munmap(pipeline->related_sensors_sync_buf,
      pipeline->related_sensors_sync_buf_size);
    close(pipeline->related_sensors_sync_buf_fd);
#endif
    pipeline->related_sensors_sync_buf = NULL;
    pipeline->related_sensors_sync_buf_size = 0;
  }

  pthread_mutex_destroy(&pipeline->stream_ops_lock);
  /* Destroy MCT bus */
  mct_bus_destroy(pipeline->bus);

  MCT_OBJECT_UNLOCK(pipeline);

  CLOGI(CAM_MCT_MODULE, "X: Pipeline destroyed successfully session =%d",
    pipeline->session);
  free(pipeline);
  pipeline = NULL;

  return;
}
