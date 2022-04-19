 /* iface_hvx.c
 *
 * Copyright (c) 2015-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/* std headers */
#include <stdio.h>
#include <dlfcn.h>

/* iface headers */
#include "iface.h"
#include "iface_hvx.h"
#include "iface_util.h"
#include <inttypes.h>

/* #define ILOG_DEBUG */
#ifdef ILOG_DEBUG
  #define ILOG(fmt, args...) \
    CDBG_ERROR("%s:%d hvx_debug "fmt"\n", __func__, __LINE__, ##args)
#else
   #define ILOG(fmt, args...) do{}while(0)
#endif

#define IERR(fmt, args...) \
  CDBG_ERROR("%s:%d hvx_debug "fmt"\n", __func__, __LINE__, ##args)


/* macro */
#define HVX_STUB_LIB_NAME               "libadsp_hvx_stub.so"
#define HVX_STUB_EVENT                  "hvx_stub_event"
#define HVX_LIB_FILL_FUNCTION_TABLE     "hvx_lib_fill_function_table"
#define DEFAULT_RX_LINES                10
#define DEFAULT_TX_LINES                7
#define STRIDE_BOUNDARY(width)          (((((width)*2) + 127) >> 7) << 7)
#define HVX_NUM_BUFFERS                 4
#define HVX_NUM_FRAME_BUFFERS           1
#define HVX_NUM_STATS_BUFFERS           4
#define MAX_SESSION                     2
static iface_hvx_handle_session_map_t  adsp_handle_map[MAX_SESSION];
/**************** Buffer manager implementation Start ***************/


/** iface_hvx_clear_mapped_buffers:
 *
 *  @hvx: hvx handle
 *  free filled buffer
 *
 *  Return 0 on success and negative error on failure
 **/
static int32_t iface_hvx_clear_mapped_buffers(iface_hvx_t *hvx)
{
  int                         i = 0;
  hvx_stub_request_buffers_t *request_buffers = NULL;
  hvx_stub_request_buffers_t *request_dump_buffers = NULL;

  if (!hvx) {
    IERR("failed: hvx %p", hvx);
    return -EINVAL;
  }

  request_buffers = &hvx->request_buffers;
  request_dump_buffers = &hvx->request_dump_buffers;
  for (i = 0 ; i < MAX_HVX_BUFFERS; i++){
    if (hvx->hvx_info.is_stats_needed)
      request_buffers->buf_label[i] = 0;
    if (hvx->hvx_info.is_dump_frame_needed)
      request_dump_buffers->buf_label[i] = 0;
  }
  if (hvx->hvx_info.is_stats_needed)
    request_buffers->num_buffers = 0;
  if (hvx->hvx_info.is_dump_frame_needed)
    request_dump_buffers->num_buffers = 0;
  return 0;
}


/** iface_hvx_get_free_buffer:
 *
 *  @hvx: hvx handle
 *  @adsp_buffers: pointer to fill free buffer
 *  @num_isp:
 *
 *  Fills the free buffer
 *
 *  Return 0 on success and negative error on failure
 **/
static int32_t iface_hvx_get_free_buffer(iface_hvx_t *hvx,
  iface_hvx_adsp_buffers_t *adsp_buffers, uint32_t num_isp)
{
  int                         i = 0;
  hvx_stub_request_buffers_t *request_buffers = NULL;
  uint32_t                    isp_id = 0;
  if (!hvx || !adsp_buffers || !num_isp || (num_isp > MAX_HVX_VFE)) {
    IERR("failed: hvx %p adsp_buffers %p", hvx, adsp_buffers);
    return -EINVAL;
  }

  adsp_buffers[PING].buffer = 0x0;
  adsp_buffers[PONG].buffer = 0x0;
  request_buffers = &hvx->request_buffers;
  for (i = 0, isp_id = 0; ((i < request_buffers->num_buffers) &&
    (isp_id < num_isp)); i++) {
    ILOG("state %d  label %d ", request_buffers->state[i], request_buffers->buf_label[i]);
    if ( hvx->hvx_info.is_stats_needed && request_buffers->state[i] == HVX_STUB_STATE_IFACE &&
        (1 & request_buffers->buf_label[i] >> (4 + isp_id))) {
      adsp_buffers[isp_id].buffer = request_buffers->buffers[i];
      adsp_buffers[isp_id].buf_idx = i;
      adsp_buffers[isp_id].buf_label = request_buffers->buf_label[i];
      ILOG("get_buf[%d] %llx  label %d state %d isp_id %d", i, adsp_buffers[isp_id].buffer,
        adsp_buffers[isp_id].buf_label, request_buffers->state[i], isp_id);
      isp_id++;
    }
  }

  if (i > request_buffers->num_buffers) {
    return -1;
  }
  return 0;
}

/** iface_hvx_get_free_dump_buffer:
 *
 *  @hvx: hvx handle
 *  @adsp_buffers: pointer to fill free buffer
 *  @num_isp:
 *
 *  Fills the free buffer
 *
 *  Return 0 on success and negative error on failure
 **/

static int32_t iface_hvx_get_free_frame_dump_buffer(iface_hvx_t *hvx,
  iface_hvx_adsp_buffers_t *adsp_buffers, uint32_t num_isp)
{
  int                         i = 0;
  hvx_stub_request_buffers_t *request_buffers = NULL;
  uint32_t                    isp_id = 0;
  if (!hvx || !adsp_buffers || !num_isp || (num_isp > MAX_HVX_VFE)) {
    IERR("failed: hvx %p adsp_buffers %p", hvx, adsp_buffers);
    return -EINVAL;
  }

  adsp_buffers[PING].buffer = 0x0;
  adsp_buffers[PONG].buffer = 0x0;
  request_buffers = &hvx->request_dump_buffers;

  for (i = 0, isp_id = 0; ((i < request_buffers->num_buffers) &&
    (isp_id < num_isp)); i++) {
    if ( hvx->hvx_info.is_dump_frame_needed && request_buffers->state[i] == HVX_STUB_STATE_IFACE &&
      (request_buffers->buf_label[i]  >= 0x40 ) &&
      (1 & (request_buffers->buf_label[i]-0x40) >> (4 + isp_id))) {
      adsp_buffers[isp_id].buffer = request_buffers->buffers[i];
      adsp_buffers[isp_id].buf_idx = i;
      adsp_buffers[isp_id].buf_label = request_buffers->buf_label[i];
      ILOG("get_buf[%d] %llx  label %x state %d isp_id %d", i, adsp_buffers[isp_id].buffer,
        adsp_buffers[isp_id].buf_label, request_buffers->state[i], isp_id);
      isp_id++;
    }
  }
  if (i > request_buffers->num_buffers) {
    return -1;
  }
  return 0;
}
/** iface_hvx_divert_buffer:
 *
 *  @hvx: hvx handle
 *  @adsp_buffers: pointer for the buffer
 *  @num_isp:
 *
 *  diverts the buffer
 *
 *  Return 0 on success and negative error on failure
 **/
static int32_t iface_hvx_divert_buffer(iface_hvx_t *hvx,
  iface_hvx_adsp_buffers_t *adsp_buffers, uint32_t num_isp)
{
  int                         i = 0;
  uint32_t                    isp_id = 0;
  hvx_stub_request_buffers_t *request_buffers = NULL;

  if (!hvx || !adsp_buffers || !num_isp || (num_isp > MAX_HVX_VFE)) {
    IERR("failed: hvx %p adsp_buffers %p", hvx, adsp_buffers);
    return -EINVAL;
  }

  request_buffers = &hvx->request_buffers;
  for (i = 0, isp_id = 0; ((i < request_buffers->num_buffers) &&
    (isp_id < num_isp)); i++) {
    if (request_buffers->buffers[i] == adsp_buffers[isp_id].buffer) {
      if (request_buffers->state[i] != HVX_STUB_STATE_ADSP) {
        IERR("invalid state %d exp %d for buf[%d] %llu",
          request_buffers->state[i], HVX_STUB_STATE_ADSP, i,
          adsp_buffers[isp_id].buffer);
      }
      request_buffers->state[i] = HVX_STUB_STATE_3A;
      ILOG("divert_buf[%d] %llu state %d", i, adsp_buffers[isp_id].buffer,
        request_buffers->state[i]);
      isp_id++;
    }
  }

  if (i > request_buffers->num_buffers) {
    return -1;
  }
  return 0;
}

/** iface_hvx_return_buffer:
 *
 *  @hvx: hvx handle
 *  @adsp_buffers: buffer pointer
 *  @num_isp:
 *
 *  return the buffer that is used
 *
 *  Return 0 on success and negative error on failure
 **/
static int32_t iface_hvx_return_buffer(iface_hvx_t *hvx,
  unsigned char buf_label, uint32_t num_isp)
{
  int                         i = 0;
  uint32_t                    isp_id = 0;
  uint32_t                    ping_pong = 0;
  hvx_stub_request_buffers_t *request_buffers = NULL;

  if (!hvx || !num_isp || (num_isp > MAX_HVX_VFE)) {
    IERR("failed: hvx %p ", hvx);
    return -EINVAL;
  }

  if (  buf_label < 0x40 ) {
    //stats buffer
    request_buffers = &hvx->request_buffers;
  } else {
    request_buffers = &hvx->request_dump_buffers;
  }

  for (i = 0, isp_id = 0; i < request_buffers->num_buffers; i++) {
      if (request_buffers->buf_label[i] == buf_label){
        if (request_buffers->state[i] != HVX_STUB_STATE_3A) {
           IERR("invalid state %d exp %d for buf_label[%d] %x",
             request_buffers->state[i], HVX_STUB_STATE_ADSP, i,
             request_buffers->buf_label[i]);
        }
        request_buffers->state[i] = HVX_STUB_STATE_IFACE;
        ILOG("iface_hvx_return_buffer[%d] state %d label %x buffer %llx", i,
          request_buffers->state[i], request_buffers->buf_label[i],
          request_buffers->buffers[i]);
      }
  }

  if (i > request_buffers->num_buffers) {
    return -1;
  }
  return 0;
}
/**************** Buffer manager implementation End ***************/

/** iface_hvx_oem_dynamic_config:
 *
 *  @adsp_data: data to be passed to Q6
 *  @adsp_data_size: data size
 *  @caller_data: Caller data
 *
 *  Call HVX dynamic config to Q6
 *
 *  Return 0 on success and negative error on failure
 **/
static hvx_ret_type_t iface_hvx_oem_dynamic_config(void *adsp_data,
 unsigned int adsp_data_size, void *caller_data)
{
  int32_t         rc = 0;
  hvx_ret_type_t  ret_type = HVX_RET_SUCCESS;
  iface_hvx_t    *hvx = NULL;

  if (!caller_data || !adsp_data || !adsp_data_size) {
    IERR("failed caller_data %p adsp_config %p data_size %d\n",
      caller_data, adsp_data, adsp_data_size);
    return HVX_RET_FAILURE;
  }

  hvx = (iface_hvx_t *)caller_data;

  ILOG("HVX_STUB_EVENT_DYNAMIC_CONFIG E %d", hvx->adsp_handle);
  /* Call hvx_dynamic_config */
  if (hvx->hvx_stub_event) {
     rc = hvx->hvx_stub_event(hvx->adsp_handle, HVX_STUB_EVENT_DYNAMIC_CONFIG,
       adsp_data, adsp_data_size);
     if (rc < 0) {
       IERR("failed: HVX_STUB_EVENT_DYNAMIC_CONFIG rc %d\n", rc);
       ret_type = HVX_RET_FAILURE;
     }
  } else {
    IERR("failed hvx->hvx_stub_event %p", hvx->hvx_stub_event);
    ret_type = HVX_RET_FAILURE;
  }
  ILOG("HVX_STUB_EVENT_DYNAMIC_CONFIG X %d rc %d", hvx->adsp_handle, rc);

  return ret_type;
}

/** iface_hvx_update:
 *
 *  @adsp_data: data to be passed to Q6
 *  @adsp_data_size: data size
 *  @caller_data: Caller data
 *
 *  Call HVX update to Q6
 *
 *  Return 0 on success and negative error on failure
 **/
static hvx_ret_type_t iface_hvx_update(void *adsp_data,
  unsigned int adsp_data_size, void *caller_data)
{
  int32_t         rc = 0;
  hvx_ret_type_t  ret_type = HVX_RET_SUCCESS;
  iface_hvx_t    *hvx = NULL;

  if (!caller_data || !adsp_data || !adsp_data_size) {
    IERR("failed adsp_config %p data %p data_size %d\n", caller_data,
      adsp_data, adsp_data_size);
    return HVX_RET_FAILURE;
  }

  hvx = (iface_hvx_t *)caller_data;

  if (hvx->hvx_stub_event) {
    ILOG("HVX_STUB_EVENT_UPDATE E %d", hvx->adsp_handle);
    /* Call hvx_dynamic_config */
     rc = hvx->hvx_stub_event(hvx->adsp_handle, HVX_STUB_EVENT_UPDATE,
       adsp_data, adsp_data_size);
     ILOG("rc %d", rc);
     if (rc < 0) {
       IERR("failed: HVX_STUB_EVENT_UPDATE rc %d\n", rc);
       ret_type = HVX_RET_FAILURE;
     }
  } else {
    IERR("failed hvx->hvx_stub_event %p", hvx->hvx_stub_event);
    ret_type = HVX_RET_FAILURE;
  }
  ILOG("HVX_STUB_EVENT_UPDATE X %d rc %d", hvx->adsp_handle, rc);

  return ret_type;
}

/** iface_hvx_start_session:
 *
 *  @hvx_singleton: HVX singleton handle
 *  @hvx: hvx handle
 *
 *  Initialize hvx params
 *
 *  Return 0 on success and negative error on failure
 **/
int32_t iface_hvx_start_session(iface_hvx_singleton_t *hvx_singleton,
  iface_hvx_t *hvx)
{
  if (!hvx_singleton || !hvx) {
    IERR("failed: %p %p\n", hvx_singleton, hvx);
    return -1;
  }
  pthread_mutex_init(&hvx->mutex, NULL);
  pthread_mutex_init(&hvx->buf_mutex, NULL);
  pthread_mutex_init(&hvx->dump_mutex, NULL);
  return 0;
}

/** iface_hvx_stop_session:
 *
 *  @hvx_singleton: HVX singleton handle
 *  @hvx: hvx handle
 *
 *  De-Initialize hvx params
 *
 *  Return 0 on success and negative error on failure
 **/
int32_t iface_hvx_stop_session(iface_hvx_singleton_t *hvx_singleton,
  iface_hvx_t *hvx)
{
  if (!hvx_singleton || !hvx) {
    IERR("failed: %p %p\n", hvx_singleton, hvx);
    return -1;
  }
  if (hvx->state >= IFACE_STATE_OPENED &&
      hvx->state <= IFACE_STATE_MAX){
    ILOG("state %d ", hvx->state);
    iface_hvx_close(hvx_singleton, hvx);
  }
  pthread_mutex_destroy(&hvx->mutex);
  pthread_mutex_destroy(&hvx->buf_mutex);
  pthread_mutex_destroy(&hvx->dump_mutex);
  return 0;
}

/** iface_hvx_get_bayer_format:
 *
 *  @fmt: sensor output format
 *  @bayer_format: handle to return bayer_format
 *  @bits_per_pixel: handle to return bits per pixel
 *
 *  Retrieve bayer format and bits per pixel
 *
 *  Return 0 on success and negative error on failure
 **/
static int32_t iface_hvx_get_bayer_format(cam_format_t fmt,
  hvx_stub_pixel_format_t *bayer_format, int *bits_per_pixel)
{
  if (!bayer_format || !bits_per_pixel) {
    IERR("failed: bayer_format %p bits_per_pixel %p\n", bayer_format,
      bits_per_pixel);
    return -1;
  }

  switch (fmt) {
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GBRG:
    *bayer_format = HVX_STUB_BAYER_GBRG;
    *bits_per_pixel = 8;
    break;

  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG:
    *bayer_format = HVX_STUB_BAYER_GBRG;
    *bits_per_pixel = 10;
    break;

  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GBRG:
    *bayer_format = HVX_STUB_BAYER_GBRG;
    *bits_per_pixel = 12;
    break;

  case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GBRG:
    *bayer_format = HVX_STUB_BAYER_GBRG;
    *bits_per_pixel = 14;
    break;

  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GRBG:
    *bayer_format = HVX_STUB_BAYER_GRBG;
    *bits_per_pixel = 8;
    break;

  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG:
    *bayer_format = HVX_STUB_BAYER_GRBG;
    *bits_per_pixel = 10;
    break;

  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GRBG:
    *bayer_format = HVX_STUB_BAYER_GRBG;
    *bits_per_pixel = 12;
    break;

  case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GRBG:
    *bayer_format = HVX_STUB_BAYER_GRBG;
    *bits_per_pixel = 14;
    break;

  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_RGGB:
    *bayer_format = HVX_STUB_BAYER_RGGB;
    *bits_per_pixel = 8;
    break;

  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB:
    *bayer_format = HVX_STUB_BAYER_RGGB;
    *bits_per_pixel = 10;
    break;

  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_RGGB:
    *bayer_format = HVX_STUB_BAYER_RGGB;
    *bits_per_pixel = 12;
    break;

  case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_RGGB:
    *bayer_format = HVX_STUB_BAYER_RGGB;
    *bits_per_pixel = 14;
    break;

  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_BGGR:
    *bayer_format = HVX_STUB_BAYER_BGGR;
    *bits_per_pixel = 8;
    break;

  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR:
    *bayer_format = HVX_STUB_BAYER_BGGR;
    *bits_per_pixel = 10;
    break;

  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_BGGR:
    *bayer_format = HVX_STUB_BAYER_BGGR;
    *bits_per_pixel = 12;
    break;

  case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_BGGR:
    *bayer_format = HVX_STUB_BAYER_BGGR;
    *bits_per_pixel = 14;
    break;

  case CAM_FORMAT_YUV_RAW_8BIT_YUYV:
    *bayer_format = HVX_STUB_BAYER_YUYV;
    *bits_per_pixel = 14;
    break;

  case CAM_FORMAT_YUV_RAW_8BIT_YVYU:
    *bayer_format = HVX_STUB_BAYER_YVYU;
    *bits_per_pixel = 14;
    break;

  case CAM_FORMAT_YUV_RAW_8BIT_UYVY:
    *bayer_format = HVX_STUB_BAYER_UYVY;
    *bits_per_pixel = 14;
    break;

  case CAM_FORMAT_YUV_RAW_8BIT_VYUY:
    *bayer_format = HVX_STUB_BAYER_VYUY;
    *bits_per_pixel = 14;
    break;

  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GREY:
    *bayer_format = HVX_STUB_BAYER_GRBG;
    *bits_per_pixel = 10;
    break;

  default:
    *bayer_format = HVX_STUB_BAYER_MAX;
    *bits_per_pixel = -1;
    break;
  }

  ILOG("sens fmt %d bayer_format %d bits_per_pixel %d", fmt, *bayer_format,
    *bits_per_pixel);

  return 0;
}

/** iface_hvx_init_default:
 *
 *  @hvx: hvx handle
 *
 *  Initialize default
 *
 *  Return NONE
 **/
static void iface_hvx_init_default(iface_hvx_t *hvx)
{
  if (!hvx) {
    IERR("failed hvx %p\n", hvx);
    return;
  }
  hvx->enabled = FALSE;
  hvx->state = IFACE_STATE_INVALID;
  hvx->dump_state = IFACE_DUMP_STATE_INVALID;
  ILOG("hvx_state %d", hvx->state);
  memset(&hvx->hvx_info, 0, sizeof(hvx->hvx_info));
  hvx->adsp_handle = 0;
  hvx->isp_id_mask = 0;
  hvx->hvx_stub_lib_handle = NULL;
  hvx->hvx_stub_event = NULL;
  hvx->hvx_oem_lib_handle = NULL;
  hvx->hvx_oem_lib_fill_function_table = NULL;
  memset(&hvx->hvx_oem_lib_func_table, 0, sizeof(hvx->hvx_oem_lib_func_table));
  hvx->hvx_oem_lib_data = NULL;
  hvx->left_dump_id = -1;
  hvx->right_dump_id = -1;
}

/** iface_hvx_oem_lib_open:
 *
 *  @hvx_singleton: hvx singleton handle
 *  @hvx: hvx handle
 *
 *  Read setprop and open OEM lib
 *
 *  Return 0 on success and negative error on failure
 **/
static int32_t iface_hvx_oem_lib_open(iface_hvx_singleton_t *hvx_singleton,
  iface_hvx_t *hvx)
{
  int32_t                 rc = 0;
  hvx_ret_type_t          hvx_rc = HVX_RET_SUCCESS;
  char                    back_lib_name[PROPERTY_VALUE_MAX],
                          front_lib_name[PROPERTY_VALUE_MAX];
  int32_t                 back_lib_len = 0,
                          front_lib_len = 0;
  boolean                 open_back = FALSE;
  char                    libcam[PROPERTY_VALUE_MAX] = {"libmmcamera_"};

  if (!hvx_singleton || !hvx) {
    IERR("failed: hvx_singleton %p hvx %p\n", hvx_singleton, hvx);
    return -1;
  }

  /* Initialize default */
  iface_hvx_init_default(hvx);

  /* Call dlerror() once to flush out previous dl error */
  dlerror();

  if(!(hvx->stub_lib_name)){
    back_lib_len = iface_read_setprop_string("persist.vendor.camera.hvx_lib_1",
      back_lib_name, NULL);
    if (back_lib_len > 0) {
      ILOG("back_lib_len %d back_lib_name %s\n", back_lib_len, back_lib_name);
    } else {
      ILOG("back_lib_len %d\n", back_lib_len);
    }
    front_lib_len = iface_read_setprop_string("persist.vendor.camera.hvx_lib_2",
      front_lib_name, NULL);
    if (front_lib_len > 0) {
      ILOG("front_lib_len %d front_lib_name %s\n", front_lib_len,
        front_lib_name);
    } else {
      ILOG("front_lib_len %d\n", front_lib_len);
    }
    if (!back_lib_len && !front_lib_len) {
      hvx->enabled = FALSE;
      return 0;
    }
    /*  Increment num_session */
    if (!(hvx_singleton->num_session++)) {
      /* First session, try to use back camera lib name */
      if (back_lib_len > 0) {
        open_back = TRUE;
      }
    } else {
      /* Once session already open, try to use front camera lib name */
      if (!front_lib_len) {
        open_back = TRUE;
      }
    }
    ILOG("open_back %d", open_back);


    if (open_back == TRUE) {
      /* Open HVX library */
      hvx->hvx_oem_lib_handle = dlopen(back_lib_name, RTLD_NOW);
    } else {
      /* Open HVX library */
      hvx->hvx_oem_lib_handle = dlopen(front_lib_name, RTLD_NOW);
    }
  } else {
    ILOG("stub_lib_name %s ", hvx->stub_lib_name);
    strlcat(libcam, hvx->stub_lib_name, PROPERTY_VALUE_MAX);
    strlcat(libcam, ".so", PROPERTY_VALUE_MAX);
    ILOG("stub_lib_name %s ",libcam);
    hvx->hvx_oem_lib_handle = dlopen(libcam, RTLD_NOW);
  }

  if (!hvx->hvx_oem_lib_handle) {
    IERR("failed dlopen for hvx OEM lib %s error %s\n", back_lib_name,
      dlerror());
    hvx->enabled = FALSE;
    return 0;
  }

  /* Call dlsym */
  hvx->hvx_oem_lib_fill_function_table = dlsym(hvx->hvx_oem_lib_handle,
    HVX_LIB_FILL_FUNCTION_TABLE);
  if (!hvx->hvx_oem_lib_fill_function_table) {
    IERR("failed hvx_oem_lib_func_table NULL\n");
    rc = -1;
  }

  if (!rc) {
    /* Call hvx_lib_fill_function_table to fill function pointers */
    hvx_rc = hvx->hvx_oem_lib_fill_function_table(&hvx->hvx_oem_lib_func_table);
    if (hvx_rc != HVX_RET_SUCCESS) {
      IERR("failed fill func table rc %d\n", hvx_rc);
      rc = -1;
    }
  }

  /* Call hvx_lib_open */
  if (!rc && hvx->hvx_oem_lib_func_table.hvx_lib_open) {
    hvx->hvx_oem_lib_func_table.hvx_lib_open(&hvx->hvx_oem_lib_data);
    if (hvx_rc != HVX_RET_SUCCESS) {
      IERR("failed hvx_lib_open rc %d\n", hvx_rc);
      rc = -1;
    }
  }

  if (!rc)
    hvx->enabled = TRUE;

  return rc;
}

/** iface_hvx_stub_lib_open:
 *
 *  @hvx_singleton: hvx singleton handle
 *  @hvx: hvx handle
 *
 *  Open stub lib and query caps(first time)
 *
 *  Return 0 on success and negative error on failure
 **/
static int32_t iface_hvx_stub_lib_open(iface_hvx_singleton_t *hvx_singleton,
  iface_hvx_t *hvx)
{
  int32_t                 rc = 0;
  hvx_stub_query_caps_t   query_caps;

  if (!hvx_singleton || !hvx) {
    IERR("failed: hvx_singleton %p hvx %p\n", hvx_singleton, hvx);
    return -1;
  }

  /* Call dlerror() once to flush out previous dl error */
  dlerror();

  if (!hvx->hvx_stub_lib_handle) {
    /* Open HVX stub lib */
    IERR("dlopen for hvx stub lib %s\n", HVX_STUB_LIB_NAME);
    hvx->hvx_stub_lib_handle = dlopen(HVX_STUB_LIB_NAME, RTLD_NOW);
    if (!hvx->hvx_stub_lib_handle) {
      IERR("failed dlopen for hvx stub lib %s error %s\n", HVX_STUB_LIB_NAME,
        dlerror());
      hvx->enabled = FALSE;
      rc = -1;
    }
  }

  if (hvx->hvx_stub_lib_handle) {
    /* Call dlsym */
    hvx->hvx_stub_event = dlsym(hvx->hvx_stub_lib_handle, HVX_STUB_EVENT);
    if (!hvx->hvx_stub_event) {
      IERR("failed hvx_stub_event NULL\n");
      hvx->enabled = FALSE;
      rc = -1;
    }
  }

  /* Query caps */
  pthread_mutex_lock(&hvx_singleton->mutex);
  if (!rc && (!hvx_singleton->max_hvx_units ||
    !hvx_singleton->max_vector_mode)) {
    memset(&query_caps, 0, sizeof(query_caps));
    ILOG("HVX_STUB_EVENT_QUERY_CAPS E %d", hvx->adsp_handle);
    if (hvx->hvx_stub_event) {
      rc = hvx->hvx_stub_event(0, HVX_STUB_EVENT_QUERY_CAPS, &query_caps,
        sizeof(query_caps));
      if (rc < 0) {
        IERR("failed: HVX_STUB_EVENT_QUERY_CAPS rc %d\n", rc);
        hvx->enabled = FALSE;
        rc = -1;
      } else {
        hvx_singleton->max_vector_mode =
          hvx_singleton->available_vector_mode[0] =
          hvx_singleton->available_vector_mode[1] =
          query_caps.hvx_stub_vector_mode;
        hvx_singleton->max_hvx_units = hvx_singleton->available_hvx_units =
          query_caps.max_hvx_unit;
        ILOG("max vector mode %d max units %d", hvx_singleton->max_vector_mode,
          hvx_singleton->max_hvx_units);
      }
    }
    ILOG("HVX_STUB_EVENT_QUERY_CAPS X %d rc %d", hvx->adsp_handle, rc);
  }

  pthread_mutex_unlock(&hvx_singleton->mutex);

  return rc;
}

/** iface_hvx_oem_lib_get_hvx_info:
 *
 *  @hvx_singleton: hvx singleton handle
 *  @hvx: hvx handle
 *  @set_dim: set_dim handle
 *
 *  Get HVX info from OEM lib
 *
 *  Return 0 on success and negative error on failure
 **/
static int32_t iface_hvx_oem_lib_get_hvx_info(
  iface_hvx_singleton_t *hvx_singleton, iface_hvx_t *hvx,
  sensor_set_dim_t *set_dim)
{
  hvx_ret_type_t          hvx_rc = HVX_RET_SUCCESS;
  int32_t                 rc = 0;
  hvx_lib_get_hvx_info_t *hvx_info = NULL;

  if (!hvx_singleton || !hvx || !set_dim) {
    IERR("failed: hvx_singleton %p hvx %p set_dim %p\n", hvx_singleton,
      hvx, set_dim);
    return -1;
  }

  pthread_mutex_lock(&hvx_singleton->mutex);
  if (hvx->hvx_oem_lib_func_table.hvx_lib_get_hvx_info) {
    hvx_info = &hvx->hvx_info;
    memset(hvx_info, 0, sizeof(*hvx_info));
    hvx_info->sensor_width = set_dim->dim_output.width;
    hvx_info->sensor_height = set_dim->dim_output.height;
    hvx_info->output_format = set_dim->output_format;
    hvx_info->available_hvx_units = hvx_singleton->max_hvx_units;
    hvx_info->available_hvx_vector_mode =
      (hvx_lib_vector_mode_t) hvx_singleton->max_vector_mode;
    hvx_rc = hvx->hvx_oem_lib_func_table.hvx_lib_get_hvx_info(
      hvx->hvx_oem_lib_data, hvx_info);
    if (hvx_rc != HVX_RET_SUCCESS) {
      IERR("failed hvx_lib_get_hvx_info rc %d\n", hvx_rc);
      /* Disable hvx */
      hvx->enabled = FALSE;
      rc = -1;
    } else if ((hvx_info->hvx_enable == HVX_TRUE) &&
      (hvx_info->is_pix_intf_needed == HVX_FALSE) &&
      (hvx_info->is_stats_needed == HVX_FALSE)) {
      IERR("failed hvx_enable %d pix %d stats %d\n", hvx_info->hvx_enable,
        hvx_info->is_pix_intf_needed, hvx_info->is_stats_needed);
      rc = -1;
    } else if ((hvx_info->hvx_enable == HVX_TRUE) &&
      (hvx_info->is_stats_needed == HVX_TRUE) && !hvx_info->stats_data_size) {
      IERR("failed hvx_enable %d stats enable %d size %d\n",
        hvx_info->hvx_enable, hvx_info->is_stats_needed,
        hvx_info->stats_data_size);
      rc = -1;
    } else if (hvx_info->hvx_enable == HVX_TRUE) {
      hvx->enabled = TRUE;
      if (!hvx_info->hvx_out_width) {
        /* Fill default */
        hvx_info->hvx_out_width = hvx_info->sensor_width;
      }
      if (!hvx_info->hvx_out_height) {
        /* Fill default */
        hvx_info->hvx_out_height = hvx_info->sensor_height;
      }
      if (!hvx_info->request_hvx_units ||
        (hvx_info->request_hvx_units > hvx_info->available_hvx_units)) {
        IERR("error: request_hvx_units %d avalable %d\n",
          hvx_info->request_hvx_units, hvx_info->available_hvx_units);
        /* Fill default */
        hvx_info->request_hvx_units = 1;
      }
      if ((hvx_info->request_hvx_vector_mode == HVX_LIB_VECTOR_NULL) ||
        (hvx_info->request_hvx_vector_mode == HVX_LIB_VECTOR_INVALID) ||
        (hvx_info->request_hvx_vector_mode >= HVX_LIB_VECTOR_MAX)) {
        IERR("error: request_hvx_vector_mode %d avalable %d\n",
          hvx_info->request_hvx_vector_mode,
          hvx_info->available_hvx_vector_mode);
        /* Fill default */
        hvx_info->request_hvx_vector_mode = HVX_LIB_VECTOR_32;
      }
      /* Modify sensor output */
      set_dim->dim_output.width = hvx_info->hvx_out_width;
      set_dim->dim_output.height = hvx_info->hvx_out_height;
      ILOG("get_hvx_info enable %d algo %s dim %d %d",
        hvx_info->hvx_enable, hvx_info->algo_name, hvx_info->hvx_out_width,
        hvx_info->hvx_out_height);
      ILOG("get_hvx_info request units %d mode %d pix %d stats %d %d",
        hvx_info->request_hvx_units, hvx_info->request_hvx_vector_mode,
        hvx_info->is_pix_intf_needed, hvx_info->is_stats_needed,
        hvx_info->stats_data_size);
    }
  }
  pthread_mutex_unlock(&hvx_singleton->mutex);

  return rc;
}

/** iface_stub_lib_close:
 *
 *  @hvx: hvx handle
 *
 *  Close stub library
 *
 **/
static void iface_stub_lib_close(iface_hvx_t *hvx)
{
  if (!hvx) {
    return;
  }

  /* Close HVX stub lib */
  if (hvx->hvx_stub_lib_handle) {
    dlclose(hvx->hvx_stub_lib_handle);
    ILOG("hvx_state %d", hvx->state);
    hvx->hvx_stub_lib_handle = 0;
    hvx->hvx_stub_event = NULL;
  }
}

/** iface_hvx_oem_lib_close:
 *
 *  @hvx: hvx handle
 *
 *  Close OEM library
 *
 **/
static void iface_hvx_oem_lib_close(iface_hvx_t *hvx)
{
  hvx_ret_type_t hvx_rc = HVX_RET_SUCCESS;

  if (!hvx) {
    return;
  }

  if ((hvx->state >= IFACE_STATE_OPENED) &&
    hvx->hvx_oem_lib_func_table.hvx_lib_close) {
    ILOG("hvx_state %d", hvx->state);
    /* Call hvx_lib_close*/
    hvx_rc = hvx->hvx_oem_lib_func_table.hvx_lib_close(hvx->hvx_oem_lib_data);
    if (hvx_rc != HVX_RET_SUCCESS) {
      IERR("failed hvx_lib_close rc %d\n", hvx_rc);
    }
  }
  hvx->hvx_oem_lib_data = NULL;

  if (hvx->hvx_oem_lib_handle) {
    /* close hvx lib */
    dlclose(hvx->hvx_oem_lib_handle);
    hvx->hvx_oem_lib_handle = NULL;
    hvx->hvx_oem_lib_fill_function_table = NULL;
    memset(&hvx->hvx_oem_lib_func_table, 0,
      sizeof(hvx->hvx_oem_lib_func_table));
  }
  hvx->state = IFACE_STATE_INVALID;
  hvx->enabled = FALSE;

  return;
}


/** iface_hvx_send module_event:
 *
 *  @iface:
 *  @session: iface_session
 *  @stream_id:
 *
 *  Send disable mask to ISP
 *  Return 0 on success and negative error on failure
 **/

int32_t iface_hvx_send_module_event_isp(iface_t *iface,
  iface_session_t *session, uint32_t stream_id)
{
  hvx_lib_get_hvx_info_t     *hvx_info = NULL;
  iface_stream_t        *user_stream = NULL;
  mct_port_t            *isp_sink_port = NULL;
  mct_event_t            event;

  if (!iface || !session ){
    CDBG_ERROR("iface  %p session %p ", iface, session);
    return -1;
  }

  hvx_info = &session->hvx.hvx_info;

  user_stream = iface_util_find_stream_in_sink_port_list(iface,
    session->session_id, stream_id);
  if (!user_stream || !user_stream->src_port) {
    CDBG_ERROR("%s:%d NULL pointer  rc = -1\n",
      __func__, __LINE__);
    return -1;
   }


   CDBG("%s mask %lld \n ", __func__, hvx_info->disable_module_mask);
   event.type = MCT_EVENT_MODULE_EVENT;
   event.u.module_event.type = MCT_EVENT_MODULE_ISP_DISABLE_MODULE;
   event.u.module_event.module_event_data = (void *)&hvx_info->disable_module_mask;
   event.identity = pack_identity(session->session_id, user_stream->stream_id);
   event.direction = MCT_EVENT_DOWNSTREAM;
   mct_port_send_event_to_peer(user_stream->src_port, &event);

   return 0;
}

/** iface_hvx_open:
 *
 *  @hvx_singleton: hvx singleton handle
 *  @hvx: hvx handle
 *  @set_dim: sensor output handle
 *
 *  Pass sensor output to OEM stub library and get output of
 *  HVX along with stats
 *
 *  Return 0 on success and negative error on failure
 **/
int32_t iface_hvx_open(iface_hvx_singleton_t *hvx_singleton,
  iface_hvx_t *hvx, sensor_set_dim_t *set_dim)
{
  hvx_ret_type_t          hvx_rc = HVX_RET_SUCCESS;
  int32_t                 rc = 0;
  char                    back_lib_name[PROPERTY_VALUE_MAX],
                          front_lib_name[PROPERTY_VALUE_MAX];
  int32_t                 back_lib_len = 0,
                          front_lib_len = 0;
  boolean                 open_back = FALSE;
  hvx_stub_query_caps_t   query_caps;
  hvx_lib_get_hvx_info_t *hvx_info = NULL;

  if (!hvx_singleton || !hvx || !set_dim) {
    IERR("failed: hvx_singleton %p hvx %p set_dim %p\n", hvx_singleton,
      hvx, set_dim);
    return -1;
  }

  if (hvx->state != IFACE_STATE_INVALID) {
    ILOG("hvx_state %d", hvx->state);
    return 0;
  }

  /* Lock HVX mutex */
  pthread_mutex_lock(&hvx->mutex);

  rc = iface_hvx_oem_lib_open(hvx_singleton, hvx);
  if (rc < 0) {
    IERR("failed: iface_hvx_oem_lib_open %d", rc);
  } else  if (hvx->enabled == TRUE) {

    /* Change hvx state */
    hvx->state = IFACE_STATE_OPENED;
    ILOG("hvx_state %d", hvx->state);

    rc = iface_hvx_stub_lib_open(hvx_singleton, hvx);
    if (rc < 0) {
      IERR("failed: iface_hvx_stub_lib_open rc %d", rc);
    }
  }

  if (!rc) {
    rc = iface_hvx_oem_lib_get_hvx_info(hvx_singleton, hvx, set_dim);
  }

  if ((rc < 0) || hvx->enabled == FALSE) {
    iface_stub_lib_close(hvx);
  }

  if (rc < 0) {
    iface_hvx_oem_lib_close(hvx);
  }

  /* Unlock mutex */
  pthread_mutex_unlock(&hvx->mutex);

  return rc;
}

/** iface_hvx_close:
 *
 *  @hvx_singleton: hvx singleton handle
 *  @hvx: hvx handle
 *
 *  Close HVX lib
 *
 *  Return 0 on success and negative error on failure
 **/
int32_t iface_hvx_close(iface_hvx_singleton_t *hvx_singleton,
  iface_hvx_t *hvx)
{
  hvx_ret_type_t          hvx_rc = HVX_RET_SUCCESS;
  int32_t                 rc = 0;
  hvx_lib_get_hvx_info_t *hvx_info = NULL;

  if (!hvx_singleton || !hvx) {
    IERR("failed: hvx_singleton %p hvx %p\n", hvx_singleton, hvx);
    return -1;
  }

  /* Lock HVX mutex */
  pthread_mutex_lock(&hvx->mutex);

  if (hvx->state >= IFACE_STATE_MAX) {
    IERR("invalid state hvx %d\n", hvx->state);
    hvx->state = IFACE_STATE_OPENED;
  }

  /* Call hvx_lib_close*/
  if ((hvx->state >= IFACE_STATE_OPENED) &&
    hvx->hvx_oem_lib_func_table.hvx_lib_close) {
    pthread_mutex_lock(&hvx_singleton->mutex);
     memset(&hvx->current_sof_params, 0, sizeof(hvx_lib_sof_params_t));
     memset(&hvx->new_sof_params, 0, sizeof(hvx_lib_sof_params_t));
    hvx_info = &hvx->hvx_info;
    if ((hvx->isp_id_mask & (1 << VFE0)) &&
      (hvx->isp_id_mask & (1 << VFE1))) {
      hvx_singleton->available_hvx_units += (2 * hvx_info->request_hvx_units);
    } else {
      hvx_singleton->available_hvx_units += hvx_info->request_hvx_units;
    }
    pthread_mutex_unlock(&hvx_singleton->mutex);
  }

  iface_hvx_clear_mapped_buffers(hvx);
  /* Close OEM library */
  iface_hvx_oem_lib_close(hvx);

  if (hvx->hvx_stub_event) {
    ILOG("HVX_STUB_EVENT_CLOSE E %d", hvx->adsp_handle);
    rc = hvx->hvx_stub_event(hvx->adsp_handle, HVX_STUB_EVENT_CLOSE, NULL, 0);
    if (rc < 0) {
      IERR("failed: HVX_STUB_EVENT_CLOSE rc %d\n", rc);
      rc = -1;
    }
    ILOG("HVX_STUB_EVENT_CLOSE X %d rc %d", hvx->adsp_handle, rc);
  }

  iface_stub_lib_close(hvx);

  hvx->state = IFACE_STATE_INVALID;
  ILOG("hvx_state %d", hvx->state);

  /* Unlock mutex */
  pthread_mutex_unlock(&hvx->mutex);

  return rc;
}

/** iface_hvx_adsp_lib_open:
 *
 *  @hvx_singleton: hvx singleton handle
 *  @hvx: hvx handle
 *  @sensor_out_info: sensor out info
 *  @session_resource: session resource handle
 *  @handle: ADSP handle
 *
 *  Call open on ADSP and get back handle
 *
 *  Return 0 on success and negative error on failure
 **/
static int32_t iface_hvx_adsp_lib_open(
  iface_hvx_singleton_t *hvx_singleton, iface_hvx_t *hvx,
  sensor_out_info_t *sensor_out_info, iface_resource_t *session_resource,
  int *handle)
{
  int32_t                    rc = 0;
  hvx_lib_get_hvx_info_t    *hvx_info = NULL;
  hvx_stub_open_t            hvx_stub_open;
  boolean                    dual_vfe = FALSE;
  uint32_t                   num_units = 1, i = 0;
  enum msm_ispif_vfe_intf    vfe_id = 0;

  if (!hvx || !sensor_out_info || !session_resource || !handle) {
    IERR("failed %p %p %p %p\n", hvx, sensor_out_info, session_resource,
      handle);
    return -1;
  }

  hvx_info = &hvx->hvx_info;

  /* call hvx_lib_open */
  memset(&hvx_stub_open, 0, sizeof(hvx_stub_open));
  strlcpy(hvx_stub_open.name, hvx_info->algo_name, sizeof(hvx_stub_open.name));

  dual_vfe = IS_DUAL_VFE(session_resource->num_isps);
  if (dual_vfe == TRUE) {
    hvx_stub_open.vfe_id = HVX_STUB_VFE_BOTH;
    num_units = 2;
  } else {
    vfe_id = WHICH_VFE(session_resource->isp_id_mask);
    if (vfe_id == VFE0) {
      hvx_stub_open.vfe_id = HVX_STUB_VFE0;
    } else if (vfe_id == VFE1) {
      hvx_stub_open.vfe_id = HVX_STUB_VFE1;
    } else {
      IERR("failed invalid isp_id_mask %x\n",
        session_resource->isp_id_mask);
      rc = -1;
    }
    num_units = 1;
  }

  pthread_mutex_lock(&hvx_singleton->mutex);

  if ((num_units * hvx_info->request_hvx_units) >
    hvx_singleton->available_hvx_units) {
    IERR("insufficient hvx units %d available %d\n",
      (num_units * hvx_info->request_hvx_units),
      hvx_singleton->available_hvx_units);
    rc = -1;
  }

  pthread_mutex_unlock(&hvx_singleton->mutex);

  if (!rc) {
    ILOG("open algo %s vfe_id %d handle %d",
      hvx_stub_open.name, hvx_stub_open.vfe_id, hvx_stub_open.handle);
    ILOG("HVX_STUB_EVENT_OPEN E %d", hvx->adsp_handle);
    /* Call hvx_stub_open */
    rc = hvx->hvx_stub_event(0, HVX_STUB_EVENT_OPEN, &hvx_stub_open,
      sizeof(hvx_stub_open));
    if (rc < 0) {
      IERR("failed: HVX_STUB_EVENT_OPEN rc %d\n", rc);
      return rc;
    }
    ILOG("HVX_STUB_EVENT_OPEN X %d rc %d", hvx->adsp_handle, rc);

    *handle = hvx_stub_open.handle;
    for (i = 0; i < MAX_SESSION ; i++) {
      if (!adsp_handle_map[i].iface_hvx) {
        adsp_handle_map[i].iface_hvx =  hvx;
        adsp_handle_map[i].adsp_handle = *handle;
        break;
      }
    }
  }

  if (rc < 0) {
    hvx->enabled = FALSE;
  }

  if (!rc ) {
    hvx->enabled = TRUE;
  }

  return rc;
}

/** iface_hvx_adsp_lib_reset:
 *
 *  @hvx: hvx handle
 *
 *  Call reset on ADSP
 *
 *  Return 0 on success and negative error on failure
 **/
static int32_t iface_hvx_adsp_lib_reset(iface_hvx_t *hvx)
{
  int32_t rc = 0;

  ILOG("HVX_STUB_EVENT_RESET E");
  /* Call reset */
  rc = hvx->hvx_stub_event(hvx->adsp_handle, HVX_STUB_EVENT_RESET, NULL, 0);
  if (rc < 0) {
    IERR("failed: HVX_STUB_EVENT_RESET rc %d\n", rc);
    return rc;
  }
  ILOG("HVX_STUB_EVENT_RESET X %d rc %d", hvx->adsp_handle, rc);

  return 0;
}

/** iface_hvx_adsp_lib_static_config:
 *
 *  @hvx_singleton: hvx singleton handle
 *  @hvx: hvx handle
 *  @sensor_out_info: sensor out info
 *  @session_resource: session resource handle
 *  @bayer_format: bayer format
 *  @bits_per_pixel: bits per pixel
 *  @vfe_clk: vfe clk frequency
 *
 *  Call static_config on ADSP
 *
 *  Return 0 on success and negative error on failure
 **/
static int32_t iface_hvx_adsp_lib_static_config(
  iface_hvx_singleton_t *hvx_singleton, iface_hvx_t *hvx,
  sensor_out_info_t *sensor_out_info, iface_resource_t *session_resource,
  hvx_stub_pixel_format_t bayer_format, int32_t bits_per_pixel,
  uint32_t *vfe_clk)
{
  int32_t                    rc = 0;
  hvx_stub_static_config_t   stub_static_config;
  uint32_t                   full_camif_w = 0,
                             full_camif_h = 0,
                             vfe_width[2],
                             num_isp = 0,
                             i = 0;
  hvx_lib_get_hvx_info_t    *hvx_info = NULL;
  boolean                    dual_vfe = FALSE;

  if (!hvx || !sensor_out_info || !session_resource || !vfe_clk) {
    IERR("failed %p %p %p %p\n", hvx, sensor_out_info, session_resource,
      vfe_clk);
    return -1;
  }

  hvx_info = &hvx->hvx_info;

  dual_vfe = IS_DUAL_VFE(session_resource->num_isps);
  if (dual_vfe == TRUE) {
    /* Calculate left vfe width */
    vfe_width[0] = session_resource->ispif_split_info.right_stripe_offset +
      session_resource->ispif_split_info.overlap -
      sensor_out_info->request_crop.first_pixel;
    /* Calculate right vfe width */
    vfe_width[1] = sensor_out_info->request_crop.last_pixel -
      session_resource->ispif_split_info.right_stripe_offset + 1;
    hvx->stub_vfe_type = HVX_STUB_VFE_BOTH;
    num_isp = 2;
  } else {
    /* Calculate left vfe width */
    vfe_width[0] = sensor_out_info->request_crop.last_pixel -
      sensor_out_info->request_crop.first_pixel + 1;
    /* Calculate right vfe width */
    vfe_width[1] = 0;
    if (WHICH_VFE(session_resource->isp_id_mask) == VFE0) {
      hvx->stub_vfe_type = HVX_STUB_VFE0;
    } else {
      hvx->stub_vfe_type = HVX_STUB_VFE1;
    }
    num_isp = 1;
  }

  /* Calculate full size camif dimension from sensor output */
  full_camif_w = sensor_out_info->request_crop.last_pixel -
    sensor_out_info->request_crop.first_pixel + 1;
  full_camif_h = sensor_out_info->request_crop.last_line -
    sensor_out_info->request_crop.first_line + 1;

  /* HVX Static config */
  memset(&stub_static_config, 0, sizeof(stub_static_config));
  stub_static_config.handle = hvx->adsp_handle;

  /* Update common information */
  stub_static_config.vfe_id = hvx->stub_vfe_type;
  stub_static_config.hvx_stub_vector_mode =
    (hvx_stub_vector_mode_t) hvx_info->request_hvx_vector_mode;
  stub_static_config.width = full_camif_w;
  stub_static_config.height = full_camif_h;
  stub_static_config.pixel_format = bayer_format;
  stub_static_config.bits_per_pixel = bits_per_pixel;

  if (dual_vfe == TRUE) {
    stub_static_config.image_overlap =
      session_resource->ispif_split_info.overlap;
    stub_static_config.right_image_offset =
      session_resource->ispif_split_info.right_stripe_offset;
  } else {
    stub_static_config.image_overlap = 0;
    stub_static_config.right_image_offset = 0;
  }

  for (i = 0; i < num_isp; i++) {
    stub_static_config.hvx_unit_no[i] = hvx_info->request_hvx_units;
    stub_static_config.vfe_clk_freq[i] = vfe_clk[i];
    stub_static_config.rx_lines[i] = DEFAULT_RX_LINES;
    stub_static_config.tx_lines[i] = DEFAULT_TX_LINES;
    stub_static_config.rx_line_width[i] = stub_static_config.tx_line_width[i] =
      vfe_width[i];
    stub_static_config.rx_line_stride[i] =
      stub_static_config.tx_line_stride[i] = STRIDE_BOUNDARY(vfe_width[i]);
  }

  /* Update resource manager */
  hvx_singleton->available_hvx_units =
    hvx_singleton->max_hvx_units - (num_isp * hvx_info->request_hvx_units);
  hvx_singleton->available_vector_mode[0] =
    hvx_singleton->max_vector_mode - hvx_info->request_hvx_vector_mode;

  /* Print static_config */
  ILOG("********* dump_hvx_static_config ***********");
  ILOG("dump_hvx_static_config handle %x", stub_static_config.handle);
  ILOG("dump_hvx_static_config vfe_id %x", stub_static_config.vfe_id);
  ILOG("dump_hvx_static_config full_size dim %d x %d", stub_static_config.width,
    stub_static_config.height);
  ILOG("dump_hvx_static_config dual_vfe overlap %d",
    stub_static_config.image_overlap);
  ILOG("dump_hvx_static_config right_vfe offset %d",
    stub_static_config.right_image_offset);
  ILOG("dump_hvx_static_config pixel_format %d",
    stub_static_config.pixel_format);
  ILOG("dump_hvx_static_config bits_per_pixel %d",
    stub_static_config.bits_per_pixel);
  ILOG("dump_hvx_static_config vector mode %d",
    stub_static_config.hvx_stub_vector_mode);
  for (i = 0; i < num_isp; i++) {
    ILOG("dump_hvx_static_config vfe[%d].vfe_clk_freq %d", i,
      stub_static_config.vfe_clk_freq[i]);
    ILOG("dump_hvx_static_config vfe[%d].hvx units %d", i,
      stub_static_config.hvx_unit_no[i]);
    ILOG("dump_hvx_static_config vfe[%d].rx w %d strid %d, tx width %d stride %d",
      i, stub_static_config.rx_line_width[i],
      stub_static_config.rx_line_stride[i],
      stub_static_config.tx_line_width[i],
      stub_static_config.tx_line_stride[i]);
    ILOG("dump_hvx_static_config vfe[%d].lines rx %d tx %d", i,
      stub_static_config.rx_lines[i],
      stub_static_config.tx_lines[i]);
  }

  ILOG("HVX_STUB_EVENT_STATIC_CONFIG E");
  /* Call hvx_stub_static_config */
  rc = hvx->hvx_stub_event(hvx->adsp_handle, HVX_STUB_EVENT_STATIC_CONFIG,
    &stub_static_config, sizeof(stub_static_config));
  if (rc < 0) {
    IERR("failed: HVX_STUB_EVENT_STATIC_CONFIG rc %d\n", rc);
  }
  ILOG("HVX_STUB_EVENT_STATIC_CONFIG X %d rc %d", hvx->adsp_handle, rc);

  return 0;
}

/** iface_hvx_oem_set_config:
 *
 *  @hvx: hvx handle
 *  @session_resource: session resource handle
 *  @sensor_out_info: sensor out info handle
 *  @bayer_format: bayer format
 *  @bits_per_pixel: bits per pixel
 *
 *  Call set_config on OEM lib
 *
 *  Return 0 on success and negative error on failure
 **/
static int32_t iface_hvx_oem_set_config(iface_hvx_t *hvx,
  iface_resource_t *session_resource, sensor_out_info_t *sensor_out_info,
  hvx_stub_pixel_format_t bayer_format, int32_t bits_per_pixel)
{
  hvx_ret_type_t             hvx_rc = HVX_RET_SUCCESS;
  int32_t                    rc = 0;
  hvx_lib_config_t           hvx_lib_config;
  hvx_lib_sensor_info_t     *sensor_info = NULL;
  hvx_lib_single_isp_info_t *single_isp_info = NULL;
  hvx_lib_dual_isp_info_t   *dual_isp_info = NULL;
  hvx_lib_adsp_config_t      adsp_config;
  uint32_t                   full_camif_w = 0,
                             full_camif_h = 0,
                             vfe0_width = 0,
                             vfe1_width = 0;
  boolean                    dual_vfe = FALSE;

  if (!hvx->hvx_oem_lib_func_table.hvx_lib_set_config) {
    return 0;
  }

  /* Call dynamic config */
  memset(&hvx_lib_config, 0, sizeof(hvx_lib_config));

  /* Calculate camif window from sensor out info */
  full_camif_w = sensor_out_info->request_crop.last_pixel -
    sensor_out_info->request_crop.first_pixel + 1;
  full_camif_h = sensor_out_info->request_crop.last_line -
    sensor_out_info->request_crop.first_line + 1;

  dual_vfe = IS_DUAL_VFE(session_resource->num_isps);
  if (dual_vfe == TRUE) {
    vfe0_width = session_resource->ispif_split_info.right_stripe_offset +
      session_resource->ispif_split_info.overlap -
      sensor_out_info->request_crop.first_pixel;
    vfe1_width = sensor_out_info->request_crop.last_pixel -
      session_resource->ispif_split_info.right_stripe_offset + 1;
    hvx_lib_config.isp_type = HVX_DUAL_ISP;
  } else {
    hvx_lib_config.isp_type = HVX_SINGLE_ISP;
  }

  /* HVX input */
  sensor_info = &hvx_lib_config.sensor_info;
  switch (bayer_format) {
  case HVX_STUB_BAYER_RGGB:
    sensor_info->bayer_format = HVX_SENSOR_RGGB;
    break;

  case HVX_STUB_BAYER_BGGR:
    sensor_info->bayer_format = HVX_SENSOR_BGGR;
    break;

  case HVX_STUB_BAYER_GRBG:
    sensor_info->bayer_format = HVX_SENSOR_GRBG;
    break;

  case HVX_STUB_BAYER_GBRG:
    sensor_info->bayer_format = HVX_SENSOR_GBRG;
    break;

  default:
    sensor_info->bayer_format = HVX_SENSOR_MAX;
    break;
  }
  sensor_info->bits_per_pixel = bits_per_pixel;
  if (dual_vfe == FALSE) {
    single_isp_info = &hvx_lib_config.u.single_isp_info;
    single_isp_info->isp_info.camif_width = full_camif_w;
    single_isp_info->isp_info.camif_height = full_camif_h;
    single_isp_info->isp_info.sensor_offset = 0;
  } else {
    dual_isp_info = &hvx_lib_config.u.dual_isp_info;
    dual_isp_info->isp_info[0].camif_width = vfe0_width;
    dual_isp_info->isp_info[0].camif_height = full_camif_h;
    dual_isp_info->isp_info[0].sensor_offset = 0;
    dual_isp_info->isp_info[1].camif_width = vfe1_width;
    dual_isp_info->isp_info[1].camif_height = full_camif_h;
    dual_isp_info->isp_info[1].sensor_offset =
      session_resource->ispif_split_info.right_stripe_offset;
  }
  adsp_config.adsp_config_call = &iface_hvx_oem_dynamic_config;
  /* Call hvx_lib_set_config on OEM stub */
  hvx_rc = hvx->hvx_oem_lib_func_table.hvx_lib_set_config(
    hvx->hvx_oem_lib_data, &hvx_lib_config, &adsp_config, hvx);
  if (hvx_rc != HVX_RET_SUCCESS) {
    IERR("failed hvx_lib_set_config rc %d\n", hvx_rc);
    return rc;
  }

  return 0;
}

/** iface_hvx_stats_callback_func:
 *
 *  @hvx: hvx handle
 *  @stub_vfe_type: hvx_stub_type
 *  @buf_label: buffer label
 *
 *  Callback function from adsp after they fill the buffer
 *
 *  Return 0 on success and negative error on failure
 **/
static int iface_hvx_callback_func(
  int                      handle,
  enum hvx_stub_vfe_type_t stub_vfe_type __unused,
  unsigned char          buf_label)
{
  uint32_t                      *data = NULL;
  char                           value[PROPERTY_VALUE_MAX];
  char                           str_buf[32];
  FILE                          *fptr = NULL;
  uint32_t                       isp_id = 0;
  int                            i = 0;
  int                            rc = 0;
  iface_hvx_t                    *iface_hvx = NULL;
  hvx_stub_request_buffers_t     *request_buffers = NULL;
  hvx_lib_adsp_config_t          adsp_config;
  hvx_lib_stats_t                stats_data;
  int                            p = 0;
  boolean                        stats_flag = FALSE;
  boolean                        dump_flag = FALSE;
  int                            dump_id = 0;
  ILOG("received buffer handle %d vfe %d buf_label %d ", handle,
    stub_vfe_type, buf_label);

  do {
    memset(&stats_data, 0, sizeof(hvx_lib_stats_t));
    adsp_config.adsp_config_call = &iface_hvx_oem_dynamic_config;
    for (i = 0; i < MAX_SESSION; i++) {
       if (adsp_handle_map[i].adsp_handle == handle) {
         iface_hvx = adsp_handle_map[i].iface_hvx;
         break;
       }
    }

    if (iface_hvx != NULL) {
      pthread_mutex_lock(&iface_hvx->buf_mutex);
      if (  buf_label < 0x40 ) {
        //stats buffer
        request_buffers = &iface_hvx->request_buffers;
        stats_flag = TRUE;
      } else  {
        request_buffers = &iface_hvx->request_dump_buffers;
        dump_flag = TRUE;
      }
      for (i = 0; i < request_buffers->num_buffers; i++) {
        ILOG("buffers addr[%d] %llx  lable %d ", i,
          request_buffers->buffers[i], request_buffers->buf_label[i]);
        if (request_buffers->buf_label[i] == buf_label) {
          request_buffers->state[i] = HVX_STUB_STATE_3A;
          if (iface_hvx->hvx_info.is_stats_needed && (buf_label < 0x40) ) {
            if (1 & (buf_label >> (4 + VFE0))) {
              ILOG("VFE0", __func__, __LINE__);
              stats_data.stats_left =  (void*)((uint32_t)(uintptr_t)request_buffers->buffers[i]);
              stats_data.stats_left_size = request_buffers->buffer_size;
            } else if (1 & (buf_label >> (4 + VFE1))) {
              ILOG("VFE1", __func__, __LINE__);
              stats_data.stats_right =  (void*)((uint32_t)(uintptr_t)request_buffers->buffers[i]);
              stats_data.stats_right_size = request_buffers->buffer_size;
            }
          } else if (iface_hvx->hvx_info.is_dump_frame_needed && (buf_label >= 0x40) ) {
            if (1 & ( (buf_label-0x40) >> (4 + VFE0))) {
              stats_data.stats_left =  (void*)((uint32_t)(uintptr_t)request_buffers->buffers[i]);
              stats_data.stats_left_size = request_buffers->buffer_size;
              iface_hvx->left_dump_id ++;
              dump_id = iface_hvx->left_dump_id;
            } else if (1 & ( (buf_label-0x40) >> (4 + VFE1))) {
              stats_data.stats_right =  (void*)((uint32_t)(uintptr_t)request_buffers->buffers[i]);
              stats_data.stats_right_size = request_buffers->buffer_size;
              iface_hvx->right_dump_id ++;
              dump_id = iface_hvx->right_dump_id;
            }
          }
          ILOG ("Buffer state Set to 3A");
        } else {
          ILOG("else buf_label %d request_buffers->buf_label[i] %d", buf_label,
            request_buffers->buf_label[i]);
        }
      }
      if (iface_hvx->hvx_oem_lib_func_table.hvx_lib_consume_stats && stats_flag ) {
        iface_hvx->hvx_oem_lib_func_table.hvx_lib_consume_stats(
        iface_hvx->hvx_oem_lib_data, iface_hvx->frame_id, &stats_data,
          &adsp_config, iface_hvx);
        if (rc != HVX_RET_SUCCESS) {
          IERR("failed hvx_lib_consume_stats rc %d\n", rc);
          rc = -1;
          pthread_mutex_unlock(&iface_hvx->buf_mutex);
          break;
        }
      }
      if (iface_hvx->hvx_oem_lib_func_table.hvx_lib_consume_dump && dump_flag ) {
        ILOG("zm: dump consume, buf lable %x, \n", buf_label);
        iface_hvx->hvx_oem_lib_func_table.hvx_lib_consume_dump(
          iface_hvx->hvx_oem_lib_data, dump_id, &stats_data,
          &adsp_config, iface_hvx);
        if (rc != HVX_RET_SUCCESS) {
          IERR("failed hvx_lib_consume_stats rc %d\n", rc);
          pthread_mutex_unlock(&iface_hvx->buf_mutex);
          rc = -1;
          break;
        }
        pthread_mutex_lock(&iface_hvx->dump_mutex);
        if ( iface_hvx->dump_state != IFACE_DUMP_STATE_FINISHED) {
          iface_hvx->num_dumped_frame ++;
        }
        pthread_mutex_unlock(&iface_hvx->dump_mutex);
      }
      rc = iface_hvx_return_buffer(iface_hvx,
        buf_label, iface_hvx->num_isp);
      if (rc < 0) {
        IERR("failed hvx_return_buffer rc %d\n", rc);
        pthread_mutex_unlock(&iface_hvx->buf_mutex);
        break;
      }
      pthread_mutex_unlock(&iface_hvx->buf_mutex);
    }
  } while (FALSE);

  return rc;
}

/** iface_hvx_request_buffers:
 *
 *  @hvx: hvx handle
 *
 *  request and send the buffer to ADSP
 *
 *  Return 0 on success and negative error on failure
 **/

static int32_t iface_hvx_request_buffers(iface_hvx_t *hvx)
{
  int32_t                      rc = 0;
  hvx_stub_request_buffers_t  *request_buffers = NULL;
  uint32_t                     i = 0;
  uint32_t                     j = 0, k = 0;
  int                          p = 0;
  uint32_t                     ping_pong = 0;
  uint32_t                     num_free_buf = 0;
  hvx_stub_send_buffer_t       send_buffer;
  iface_hvx_adsp_buffers_t   adsp_buffers[MAX_HVX_VFE];
  if (!hvx) {
    IERR("failed: hvx %p", hvx);
    return -EINVAL;
  }

  /* Set callback function */
  memset(&send_buffer, 0, sizeof(hvx_stub_send_buffer_t));
  /* mutex lock to prevent thread from SOF, hvx thread to handle
   * SOF thread -> which we used to get the buffer during rotation
   */
  pthread_mutex_lock(&hvx->buf_mutex);
  if (hvx->request_buffers.num_buffers == 0) {

    request_buffers = &hvx->request_buffers;

    /* Fill num buffers and pass call to ADSP to allocate buffers */
    memset(request_buffers, 0, sizeof(hvx_stub_request_buffers_t));

    request_buffers->num_buffers = HVX_NUM_STATS_BUFFERS * hvx->num_isp;
    request_buffers->buffer_size = hvx->hvx_info.stats_data_size;
    request_buffers->buf_type = HVX_STUB_STATS;
    ILOG("assign buffer num %d, and size %d", request_buffers->num_buffers, request_buffers->buffer_size);

    /* Print stats request buffer configuration */
    ILOG("********** dump_hvx_request_buffers_config *********");
    ILOG("dump_hvx_request_buffers_config num_bufs %d size %d sizeof(request_buffers->buffers) %d",
      request_buffers->num_buffers, request_buffers->buffer_size, sizeof(request_buffers->buffers));

    ILOG("HVX_STUB_REQUEST_BUFFERS E");
    rc = hvx->hvx_stub_event(hvx->adsp_handle, HVX_STUB_REQUEST_BUFFERS,
      request_buffers, sizeof(hvx_stub_request_buffers_t));
    if (rc < 0) {
      IERR("failed: HVX_STUB_REQUEST_BUFFERS rc %d\n", rc);
    }
    ILOG("HVX_STUB_REQUEST_BUFFERS X");

    /* Initialize buffer state */
    for (p = 0; p < request_buffers->num_buffers; p++) {
      request_buffers->state[p] = HVX_STUB_STATE_IFACE;
    }

    // lable buffer according to dump frame or stats

    for (i = 0, k = 0; i < hvx->num_isp; i++) {
      for (j = 0; j < HVX_NUM_BUFFERS; j++) {
        request_buffers->buf_label[k++] =  (1 << (i + 4)) | j ;
      }
    }

    /* Print buffers information */
    ILOG("********** dump_hvx_request_buffers *********");
    for (p = 0; p < request_buffers->num_buffers; p++) {
      ILOG("dump_hvx_request_buffers addr[%d] buf label = %d %llx", p,
        request_buffers->buf_label[p], request_buffers->buffers[p]);
    }

  } else {
    /* Send ping / pong buffer to ADSP */
    for (ping_pong = 0; (ping_pong < NUM_PING_PONG); ping_pong++) {
      /* Get free buffer */
      rc = iface_hvx_get_free_buffer(hvx, adsp_buffers,
        hvx->num_isp);
      if ((rc < 0)) {
        IERR("failed: get_buf rc %d");
        break;
      }
      send_buffer.vfe_type = hvx->stub_vfe_type;
      send_buffer.buf_type = HVX_STUB_STATS;
      for (j = 0; j < hvx->num_isp; j++) {
        if (adsp_buffers[j].buffer){
          num_free_buf++;
          send_buffer.buffer_idx[j] = adsp_buffers[j].buf_idx;
          send_buffer.buf_label[j] = adsp_buffers[j].buf_label;
          send_buffer.addr[j] = (void*)((uint32_t)(uintptr_t)adsp_buffers[j].buffer);
          send_buffer.addr_Len[j] = hvx->request_buffers.buffer_size;
          hvx->adsp_buffers[j][ping_pong].buf_idx = adsp_buffers[j].buf_idx;
          hvx->adsp_buffers[j][ping_pong].buffer = adsp_buffers[j].buffer;
        } else{
          IERR("failed: adsp_buffers is null");
          break;
        }
      }
      if (num_free_buf < hvx->num_isp) {
        IERR("failed: No free buffers num_free_buf %d hvx->num_isp %d",
          num_free_buf, hvx->num_isp);
        rc =  -1;
        break;
      }
      ILOG("ping_pong %d hvx->hvx_stub_event %p hvx->adsp_handle %x"
        "send_buffer.addr[%d] %p %p", ping_pong, hvx->hvx_stub_event,
        hvx->adsp_handle, ping_pong, send_buffer.addr[0], send_buffer.addr[1]);

      ILOG("HVX_STUB_SEND_BUFFER E");
      /* Send free buffer to ADSP */
      for (k = 0; k < hvx->num_isp; k++) {
        for (p = 0; p < hvx->request_buffers.num_buffers; p++) {
          if (hvx->request_buffers.buf_label[p] == send_buffer.buf_label[k]){
            hvx->request_buffers.state[p] = HVX_STUB_STATE_ADSP;
            ILOG("state %d  label_tag %d ", hvx->request_buffers.state[p],
              hvx->request_buffers.buf_label[p]);
          }
        }
      }
      if (hvx->hvx_info.is_stats_needed) {
        rc = hvx->hvx_stub_event(hvx->adsp_handle, HVX_STUB_SEND_BUFFER,
          &send_buffer, sizeof(send_buffer));
      }
      if (rc < 0) {
        IERR("failed: HVX_STUB_SEND_BUFFER rc %d\n", rc);
      }
      num_free_buf = 0;
      ILOG("HVX_STUB_SEND_BUFFER X");
    }
  }
  pthread_mutex_unlock(&hvx->buf_mutex);
  return rc;
}

/** iface_hvx_request_frame_dump_buffers:
 *
 *  @hvx: hvx handle
 *
 *  request and send the Frame dump buffer to ADSP
 *
 *  Return 0 on success and negative error on failure
 **/

static int32_t iface_hvx_request_frame_dump_buffers(iface_hvx_t *hvx)
{
  int32_t                      rc = 0;
  hvx_stub_request_buffers_t  *request_buffers = NULL;
  uint32_t                     i = 0;
  uint32_t                     j = 0, k = 0;
  uint32_t                     ping_pong = 0;
  uint32_t                     num_free_buf = 0;
  int                          p = 0;
  hvx_stub_send_buffer_t       send_buffer;
  hvx_stub_set_callback_func_t set_callback_func;
  iface_hvx_adsp_buffers_t   adsp_buffers[MAX_HVX_VFE];
  if (!hvx) {
    IERR("failed: hvx %p", hvx);
    return -EINVAL;
  }

  /* Set callback function */
  memset(&send_buffer, 0, sizeof(hvx_stub_send_buffer_t));
  /* mutex lock to prevent thread from SOF, hvx thread to handle
   * SOF thread -> which we used to get the buffer during rotation
   */
  pthread_mutex_lock(&hvx->buf_mutex);
  if (hvx->request_dump_buffers.num_buffers == 0) {

    request_buffers = &hvx->request_dump_buffers;

    /* Fill num buffers and pass call to ADSP to allocate buffers */
    memset(request_buffers, 0, sizeof(hvx_stub_request_buffers_t));

    request_buffers->num_buffers = HVX_NUM_BUFFERS * hvx->num_isp;
    request_buffers->buffer_size = hvx->dump_frame_size;
    request_buffers->buf_type = HVX_STUB_FRAME_DUMP;
    ILOG("request buffer num %d, and size %d", request_buffers->num_buffers, request_buffers->buffer_size);

    ILOG("HVX_STUB_REQUEST_BUFFERS E");
    rc = hvx->hvx_stub_event(hvx->adsp_handle, HVX_STUB_REQUEST_BUFFERS,
      request_buffers, sizeof(hvx_stub_request_buffers_t));
    if (rc < 0) {
      IERR("failed: HVX_STUB_REQUEST_BUFFERS rc %d\n", rc);
    }
    ILOG("HVX_STUB_REQUEST_BUFFERS X");

    /* Initialize buffer state */
    for (p = 0; p < request_buffers->num_buffers; p++) {
      request_buffers->state[p] = HVX_STUB_STATE_IFACE;
    }

    // lable buffer according to dump frame or stats
    // the labeling sequency here for each isp
    // provides logic to get free buffer in get free buffer func.
    k = 0;
    for (i = 0; i < hvx->num_isp; i++) {
      for (j = 0; j < HVX_NUM_BUFFERS; j++) {
        request_buffers->buf_label[k++] =  0x40 + ( (1 << (i + 4)) | j) ;
      }
    }
    /* Print buffers information */
    ILOG("********** dump_hvx_request_buffers *********");
    for (p = 0; p < request_buffers->num_buffers; p++) {
      ILOG("dump_hvx_request_buffers addr[%d] buf label = %x %llx", p,
        request_buffers->buf_label[p], request_buffers->buffers[p]);
    }

  } else {
    /* Send ping / pong buffer to ADSP */
    for (ping_pong = 0; (ping_pong < NUM_PING_PONG); ping_pong++) {
      /* Get free buffer */
      rc = iface_hvx_get_free_frame_dump_buffer(hvx, adsp_buffers,
        hvx->num_isp);
      if ((rc < 0)) {
        IERR("failed: iface_hvx_get_free_dump_buffer\n");
        break;
      }
      send_buffer.vfe_type = hvx->stub_vfe_type;
      send_buffer.buf_type = HVX_STUB_FRAME_DUMP;

      for (j = 0; j < hvx->num_isp; j++) {
        if (adsp_buffers[j].buffer){
          num_free_buf++;
          send_buffer.buffer_idx[j] = adsp_buffers[j].buf_idx;
          send_buffer.buf_label[j] = adsp_buffers[j].buf_label;
          send_buffer.addr[j] = (void*)((uint32_t)
                                        (uintptr_t)adsp_buffers[j].buffer);
          send_buffer.addr_Len[j] = hvx->request_dump_buffers.buffer_size;
        } else{
          IERR("failed: adsp_buffers is null");
         pthread_mutex_unlock(&hvx->buf_mutex);
         return rc;
        }
      }
      if (num_free_buf < hvx->num_isp) {
        IERR("failed: No free buffers num_free_buf %d hvx->num_isp %d",
          num_free_buf, hvx->num_isp);
         rc =  -1;
        pthread_mutex_unlock(&hvx->buf_mutex);
        return rc;
      }
      ILOG("send_buffer.addr %p %p", send_buffer.addr[0], send_buffer.addr[1]);
      ILOG("HVX_STUB_SEND_BUFFER E");
      // Send free buffer to ADSP
      for (k = 0; k < hvx->num_isp; k++) {
        for (p = 0; p < hvx->request_dump_buffers.num_buffers; p++) {
          if (hvx->request_dump_buffers.buf_label[p] == send_buffer.buf_label[k]){
            hvx->request_dump_buffers.state[p] = HVX_STUB_STATE_ADSP;
            ILOG("state %d  label_tag %d ", hvx->request_dump_buffers.state[p],
              hvx->request_dump_buffers.buf_label[p]);
          }
        }
      }

      rc = hvx->hvx_stub_event(hvx->adsp_handle, HVX_STUB_SEND_BUFFER,
        &send_buffer, sizeof(send_buffer));

      if (rc < 0) {
        IERR("failed: HVX_STUB_SEND_BUFFER rc %d\n", rc);
      }
      num_free_buf = 0;
      ILOG("HVX_STUB_SEND_BUFFER X");
    }
  }
  pthread_mutex_unlock(&hvx->buf_mutex);
  return rc;
}

/** iface_hvx_handle_set_stream_config:
 *
 *  @hvx_singleton: hvx singleton handle
 *  @hvx: hvx handle
 *  @sensor_out_info: sensor out info handle
 *  @isp_resource_request: isp output resource information
 *
 *  Call hvx_stub_open and initialize hvx params
 *
 *  Return 0 on success and negative error on failure
 **/
int32_t iface_hvx_handle_set_stream_config(iface_hvx_singleton_t *hvx_singleton,
  iface_hvx_t *hvx, sensor_out_info_t *sensor_out_info,
  iface_resource_t *session_resource, uint32_t *vfe_clk)
{
  hvx_ret_type_t             hvx_rc = HVX_RET_SUCCESS;
  uint32_t                   i = 0;
  int32_t                    rc = 0;
  hvx_stub_open_t            hvx_stub_open;
  hvx_stub_static_config_t   stub_static_config;
  uint32_t                   full_camif_w = 0,
                             full_camif_h = 0,
                             vfe0_width = 0,
                             vfe1_width = 0;
  hvx_stub_pixel_format_t    bayer_format = HVX_STUB_BAYER_MAX;
  int32_t                    bits_per_pixel = -1;
  boolean                    dual_vfe = FALSE;
  hvx_lib_config_t           hvx_lib_config;
  hvx_lib_sensor_info_t     *sensor_info = NULL;
  hvx_lib_single_isp_info_t *single_isp_info = NULL;
  hvx_lib_dual_isp_info_t   *dual_isp_info = NULL;
  hvx_lib_adsp_config_t      adsp_config;
  int                        frame_size = 0;

  if (!hvx || !sensor_out_info || !session_resource || !vfe_clk) {
    IERR("failed %p %p %p %p\n", hvx, sensor_out_info, session_resource,
      vfe_clk);
    return -1;
  }

  if (hvx->state >= IFACE_STATE_CONFIGURED && hvx->state < IFACE_STATE_MAX ){
    ILOG("hvx is already configured hvx state %d\n", hvx->state);
    return 0;
  }

  ILOG("num_isps %d isp_id_mask %x", session_resource->num_isps,
    session_resource->isp_id_mask);
  /* Validate num_isps */
  if ((session_resource->num_isps == 0) || (session_resource->num_isps > 2)) {
    IERR("failed: invalid num_isps %x\n", session_resource->num_isps);
    return -1;
  }

  if (!hvx->hvx_stub_event) {
    IERR("failed: hvx->hvx_stub_event %p", hvx->hvx_stub_event);
    return -1;
  }

  /* Validate fmt */
  rc = iface_hvx_get_bayer_format(sensor_out_info->fmt, &bayer_format,
    &bits_per_pixel);
  if ((rc < 0) || (bayer_format >= HVX_STUB_BAYER_MAX ) ||
    (bits_per_pixel <= 0)) {
    IERR("failed: invalid sensor output fmt %d\n",sensor_out_info->fmt);
    return -1;
  }

  /* Lock HVX mutex */
  pthread_mutex_lock(&hvx->mutex);

  hvx->isp_id_mask = session_resource->isp_id_mask;
  hvx->num_isp = session_resource->num_isps;

  /* Call lib open */
  rc = iface_hvx_adsp_lib_open(hvx_singleton, hvx, sensor_out_info,
    session_resource, &hvx->adsp_handle);
  if (rc < 0) {
    IERR("failed: iface_hvx_adsp_lib_open rc %d", rc);

    /* Unlock mutex */
    pthread_mutex_unlock(&hvx->mutex);

    return rc;
  }

  /* Call reset */
  rc = iface_hvx_adsp_lib_reset(hvx);
  if (rc < 0) {
    IERR("failed: iface_hvx_adsp_lib_reset rc %d\n", rc);

    /* Unlock mutex */
    pthread_mutex_unlock(&hvx->mutex);

    return rc;
  }

  /* Call static_config */
  rc = iface_hvx_adsp_lib_static_config(hvx_singleton, hvx, sensor_out_info,
    session_resource, bayer_format, bits_per_pixel, vfe_clk);
  if (rc < 0) {
    IERR("failed: iface_hvx_adsp_lib_static_config rc %d\n", rc);

    /* Unlock mutex */
    pthread_mutex_unlock(&hvx->mutex);

    return rc;
  }

  /* Call set_config on ADSP */
  rc = iface_hvx_oem_set_config(hvx, session_resource, sensor_out_info,
    bayer_format, bits_per_pixel);
  if (rc < 0) {
    IERR("failed: iface_hvx_oem_set_config rc %d\n", rc);

    /* Unlock mutex */
    pthread_mutex_unlock(&hvx->mutex);

    return rc;
  }
  hvx->dump_state = IFACE_DUMP_STATE_INVALID;
  hvx->num_dump_frame = 0;
  hvx->num_dumped_frame = 0;
  hvx->left_dump_id = -1;
  hvx->right_dump_id = -1;
  /* Check if stats is required */
  if (hvx->hvx_info.is_stats_needed == TRUE) {
    rc = iface_hvx_request_buffers(hvx);
    if (rc < 0) {
      IERR("failed: iface_hvx_request_buffers rc %d\n", rc);
    }
  }

  if (hvx->hvx_info.is_dump_frame_needed == TRUE &&
      hvx->hvx_oem_lib_func_table.hvx_lib_get_dump_buffer_size){
    hvx_rc = hvx->hvx_oem_lib_func_table.hvx_lib_get_dump_buffer_size(
    (int)hvx->hvx_info.hvx_out_width,
    (int)hvx->hvx_info.hvx_out_height,
    (int)session_resource->ispif_split_info.right_stripe_offset,
    (int)session_resource->ispif_split_info.overlap,
    &(frame_size));
    if (hvx_rc != HVX_RET_SUCCESS) {
      IERR("failed: hvx_lib_get_dump_buffer_size hvx_rc %d\n", hvx_rc);
    }
    hvx->dump_frame_size = (unsigned int)frame_size;
  }

  if ( hvx->hvx_info.is_dump_frame_needed == TRUE) {
    rc = iface_hvx_request_frame_dump_buffers(hvx);
    if (rc < 0) {
      IERR("failed: iface_hvx_request_dump_buffers rc %d\n", rc);
      pthread_mutex_unlock(&hvx->mutex);
      return rc;
    } else {
      hvx->dump_state = IFACE_DUMP_STATE_ALLOCATED;
      ILOG("dump buffer allocated\n");
    }
  }

  /* Register Error callback from ADSP*/
  hvx->set_callback_func.callback_func = &iface_hvx_callback_func;
  hvx->set_callback_func.Error_callback = &iface_hvx_callback_error;
  rc = hvx->hvx_stub_event(hvx->adsp_handle, HVX_STUB_SET_CALLBACK_FUNC,
      &hvx->set_callback_func, sizeof(hvx->set_callback_func));
  if (rc < 0) {
    IERR("failed: HVX_STUB_SET_CALLBACK_FUNC rc %d\n", rc);
  }

  if (rc == 0) {
    /* Change HVX state to configured */
    hvx->state = IFACE_STATE_CONFIGURED;
    ILOG("hvx_state %d", hvx->state);
  }

  /* Unlock mutex */
  pthread_mutex_unlock(&hvx->mutex);

  return rc;
}

/** iface_hvx_streamon:
 *
 *  @hvx: hvx handle
 *
 *  Start HVX
 *
 *  Return 0 on success and negative error on failure
 **/
int32_t iface_hvx_streamon(iface_hvx_t *hvx)
{
  int32_t            rc = 0;

  if (!hvx) {
    IERR("failed: %p\n", hvx);
    return -1;
  }

  /* Lock mutex */
  pthread_mutex_lock(&hvx->mutex);

  if (hvx->state != IFACE_STATE_CONFIGURED) {
    IERR("failed invalid state %d expected %d\n", hvx->state,
      IFACE_STATE_CONFIGURED);
    ILOG("hvx_state %d", hvx->state);
    /* Unlock mutex */
    pthread_mutex_unlock(&hvx->mutex);
    return -1;
  }

  ILOG("HVX_STUB_EVENT_START E %d", hvx->adsp_handle);
  /* Call hvx_streamon */
  if (hvx->hvx_stub_event) {
    rc = hvx->hvx_stub_event(hvx->adsp_handle, HVX_STUB_EVENT_START, NULL, 0);
    if (rc < 0) {
      IERR("failed: HVX_STUB_EVENT_START rc %d\n", rc);
    } else {
      /* Change HVX state to STREAMING */
      hvx->state = IFACE_STATE_STREAMING;
      ILOG("hvx_state %d", hvx->state);
    }
  }
  ILOG("HVX_STUB_EVENT_START X %d rc %d", hvx->adsp_handle, rc);

  /* Unlock mutex */
  pthread_mutex_unlock(&hvx->mutex);

  return rc;
}

/** iface_hvx_streamoff:
 *
 *  @hvx: hvx handle
 *
 *  Stop HVX
 *
 *  Return 0 on success and negative error on failure
 **/
int32_t iface_hvx_streamoff(iface_hvx_t *hvx)
{
  int32_t            rc = 0;

  if (!hvx) {
    IERR("failed: %p\n", hvx);
    return -1;
  }

  /* Lock mutex */
  pthread_mutex_lock(&hvx->mutex);

  if (hvx->state != IFACE_STATE_STREAMING) {
    IERR("failed invalid state %d expected %d\n", hvx->state,
      IFACE_STATE_STREAMING);
    /* Unlock mutex */
    pthread_mutex_unlock(&hvx->mutex);
    return -1;
  }

  ILOG("HVX_STUB_EVENT_STOP E %d", hvx->adsp_handle);
  if (hvx->hvx_stub_event) {
    /* Call hvx_streamoff */
    rc = hvx->hvx_stub_event(hvx->adsp_handle, HVX_STUB_EVENT_STOP, NULL, 0);
    if (rc < 0) {
      IERR("failed: HVX_STUB_EVENT_STOP rc %d\n", rc);
    }
  }
  ILOG("HVX_STUB_EVENT_STOP X %d rc %d", hvx->adsp_handle, rc);

  hvx->state = IFACE_STATE_CONFIGURED;
  ILOG("hvx_state %d", hvx->state);

  /* Unlock mutex */
  pthread_mutex_unlock(&hvx->mutex);

  return rc;
}

/** iface_hvx_sof:
 *
 *  @hvx: hvx handle
 *  @frame_id: frame id
 *
 *  Send SOF to OEM Lib
 *
 *  Return 0 on success and negative error on failure
 **/
int32_t iface_hvx_sof(iface_hvx_t *hvx, uint32_t frame_id)
{
  hvx_ret_type_t        hvx_rc = HVX_RET_SUCCESS;
  int32_t               rc = 0;
  hvx_lib_adsp_config_t adsp_config;
  int32_t dump_cmd_len = 0;
  static int32_t dump_cmd_value = 0;
  char    dump_cmd_str[PROPERTY_VALUE_MAX];

  if (!hvx) {
    IERR("failed: %p\n", hvx);
    return -1;
  }

  /* Lock mutex */
  pthread_mutex_lock(&hvx->mutex);

  hvx->current_sof_params = hvx->new_sof_params;
  hvx->current_sof_params.frame_id = frame_id;

  if (hvx->state != IFACE_STATE_STREAMING) {
    ILOG("hvx_state %d", hvx->state);
    /* Unlock mutex */
    pthread_mutex_unlock(&hvx->mutex);
    return 0;
  }
  hvx->frame_id = frame_id;
  if (hvx->hvx_info.is_stats_needed == TRUE) {
    rc = iface_hvx_request_buffers(hvx);
  }

  if (hvx->hvx_oem_lib_func_table.hvx_lib_sof) {
    /* Pass SOF to OEM lib */
    adsp_config.adsp_config_call = &iface_hvx_update;
    hvx_rc = hvx->hvx_oem_lib_func_table.hvx_lib_sof(hvx->hvx_oem_lib_data,
      &hvx->current_sof_params, &adsp_config, hvx);
    if (hvx_rc != HVX_RET_SUCCESS) {
      pthread_mutex_unlock(&hvx->mutex);
      IERR("failed hvx_lib_sof rc %d\n", hvx_rc);
      rc = -1;
    }
  }

  if (!dump_cmd_value){
    dump_cmd_len = iface_read_setprop_string("persist.vendor.camera.hvx_fdump_en", dump_cmd_str, NULL);
    if (dump_cmd_len != 0) {
      IERR("Invalid string dump_cmd_len %d\n");
    }
    dump_cmd_value = atoi(dump_cmd_str);
  }

  ILOG("dump_cmd_value %d\n", dump_cmd_value);

  ILOG("dump states %d E\n", hvx->dump_state);
  if ( (hvx->dump_state == IFACE_DUMP_STATE_ALLOCATED||
    hvx->dump_state == IFACE_DUMP_STATE_RESET) && (dump_cmd_value)) {
    hvx->dump_state = IFACE_DUMP_STATE_PROCEEDING;
    hvx->num_dump_frame = dump_cmd_value * hvx->num_isp; //TBD: due VFE

    ILOG("enable dump, num of isp %d  \n", hvx->num_isp);

    rc = hvx->hvx_stub_event(hvx->adsp_handle, HVX_STUB_EVENT_ENABLE_DUMP, NULL, 0);
    if (rc < 0) {
        IERR("failed: HVX_STUB_EVENT_ENABLE_DUMP rc %d\n", rc);
        pthread_mutex_unlock(&hvx->mutex);
        return rc;
    }

    /* Unlock mutex */
    pthread_mutex_unlock(&hvx->mutex);
    return rc;
  }

  if (hvx->dump_state == IFACE_DUMP_STATE_PROCEEDING && (hvx->num_dump_frame <=
    hvx->num_dumped_frame )) {
    ILOG("disable dump\n");
    pthread_mutex_lock(&hvx->dump_mutex);
    hvx->dump_state = IFACE_DUMP_STATE_FINISHED;
    hvx->num_dump_frame = 0;
    hvx->num_dumped_frame = 0;
    hvx->left_dump_id = -1;
    hvx->right_dump_id = -1;
    dump_cmd_value = 0;

    pthread_mutex_unlock(&hvx->dump_mutex);
    rc = hvx->hvx_stub_event(hvx->adsp_handle, HVX_STUB_EVENT_DISABLE_DUMP, NULL, 0);
    if (rc < 0) {
        IERR("failed: HVX_STUB_EVENT_DISABLE_DUMP rc %d\n", rc);
        pthread_mutex_unlock(&hvx->mutex);
        return rc;
    }
    /* Unlock mutex */
    pthread_mutex_unlock(&hvx->mutex);
    return rc;
  }

  if (hvx->dump_state == IFACE_DUMP_STATE_FINISHED && (dump_cmd_value == 0)) {
    ILOG("dump finished\n");
    hvx->dump_state = IFACE_DUMP_STATE_RESET;
  }
  if (hvx->dump_state == IFACE_DUMP_STATE_PROCEEDING) {
    ILOG("dump in process,  at frame id %d", frame_id);
    rc = iface_hvx_request_frame_dump_buffers(hvx);
  }

  ILOG("dump states %d hvx->num_dump_frame %d hvx->num_dumped_frame %d X\n",
       hvx->dump_state, hvx->num_dump_frame, hvx->num_dumped_frame);

  /* Unlock mutex */
  pthread_mutex_unlock(&hvx->mutex);

  return rc;
}

/** iface_hvx_aec_update:
 *
 *  @hvx: hvx handle
 *  @stats_update: stats_update
 *
 *  Send SOF to OEM Lib
 *
 *  Return 0 on success and negative error on failure
 **/
int32_t iface_hvx_aec_update(iface_hvx_t *hvx, stats_update_t *stats_update) {

  hvx_lib_sof_params_t  *sof_params;
  if (!hvx || !stats_update){
    IERR("failed hvx %p stats_update %p", hvx, stats_update);
    return -1;
  }

  sof_params = &hvx->new_sof_params;

  sof_params->real_gain = stats_update->aec_update.real_gain;
  sof_params->exposure_time = stats_update->aec_update.exp_time;
  sof_params->short_real_gain = stats_update->aec_update.s_real_gain;
  sof_params->target_luma = stats_update->aec_update.target_luma;
  sof_params->cur_luma = stats_update->aec_update.cur_luma;
  sof_params->avg_luma = stats_update->aec_update.avg_luma;
  sof_params->lux_idx = stats_update->aec_update.lux_idx;
  sof_params->hdr_exp_time_ratio = stats_update->aec_update.hdr_exp_time_ratio;
  sof_params->hdr_sensitivity_ratio = stats_update->aec_update.hdr_sensitivity_ratio;
  return 0;
}


/** iface_hvx_awb_update:
 *
 *  @hvx: hvx handle
 *  @stats_update: stats_update
 *
 *  Send SOF to OEM Lib
 *
 *  Return 0 on success and negative error on failure
 **/
int32_t iface_hvx_awb_update(iface_hvx_t *hvx, stats_update_t *stats_update){
  hvx_lib_sof_params_t  *sof_params;
  if (!hvx || !stats_update){
    IERR("failed hvx %p stats_update %p", hvx, stats_update);
    return -1;
  }

  sof_params = &hvx->new_sof_params;

  sof_params->r_gain = stats_update->awb_update.gain.r_gain;
  sof_params->g_gain = stats_update->awb_update.gain.g_gain;
  sof_params->b_gain = stats_update->awb_update.gain.b_gain;
  sof_params->color_temp = stats_update->awb_update.color_temp;

  return 0;
}

/** iface_hvx_callback_error:
 *
 *  @hvx: hvx handle
 *  @stub_vfe_type: hvx_stub_type
 *  @error_msg: error message
 *  @error_msgLen: message length
 *  @frame_id: frame id
 *  Callback function from adsp for error case
 *
 *  Return 0 on success and negative error on failure
 **/

int32_t iface_hvx_callback_error(int handle, hvx_stub_vfe_type_t vfe_type,
   const char* error_msg, int error_msgLen, int frame_id) {
  IERR("%s:%d handle %d vfe %d msg %s error_msgLen %d frame_id %d \n", __func__, __LINE__, handle,
     vfe_type,  error_msg, error_msgLen, frame_id );
 return 0;
}
