/*============================================================================

Copyright (c) 2014-2016 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
============================================================================*/
#include <sys/mman.h>
#include <pthread.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

#include <media/msmb_camera.h>
#include <media/msmb_pproc.h>
#include "mct_pipeline.h"
#include "mct_module.h"
#include "mct_stream.h"
#include "modules.h"
#include "camera_dbg.h"
#include "vfe_test_vector.h"
#include <dlfcn.h>
#include "chromatix.h"
#include "chromatix_common.h"
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION >= 0x306)
#include "chromatix_3a.h"
#endif
#include "isp_module.h"
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x310)
/* pdaf headers */
#include "pdaf_lib.h"
#endif
#if TARGET_ION_ABI_VERSION >=2
#include <ion/ion.h>
#endif
#define CHROMATIX_3A_LIB_GAIN   "libchromatix_bet_gain_3a.so"
#define CHROMATIX_3A_LIB_LUX    "libchromatix_bet_lux_3a.so"
#define CHROMATIX_COMMON_LIB_GAIN   "libchromatix_bet_gain_common.so"
#define CHROMATIX_PREVIEW_LIB_GAIN  "libchromatix_bet_gain_preview.so"
#define CHROMATIX_COMMON_LIB_LUX    "libchromatix_bet_lux_common.so"
#define CHROMATIX_PREVIEW_LIB_LUX   "libchromatix_bet_lux_preview.so"

#include "vfe_test_vector.h"
vfe_test_vector_t vfe_test_vector;
int show_mismatch = 0;
int show_match = 0;
int has_mismatch = 0;
int quit_upon_err = 0;

#define MAX_TEST_CASE_NUM 50

typedef struct {
  testcase_report_t lux[MAX_TEST_CASE_NUM];
  testcase_report_t gain[MAX_TEST_CASE_NUM];
  char *module_name[ISP_MOD_MAX_NUM];
}bet_report_t;

bet_report_t bet_report;


struct v4l2_frame_buffer
{
  struct v4l2_buffer buffer;
  unsigned long addr[VIDEO_MAX_PLANES];
  uint32_t size;
  struct ion_allocation_data ion_alloc[VIDEO_MAX_PLANES];
  struct ion_fd_data fd_data[VIDEO_MAX_PLANES];
};

typedef struct {
  mct_port_t *sink_port;
  mct_port_t *session_port;
  mct_port_t *source_port;
  mct_port_t *test_port;
  mct_stream_info_t stream_info;
  mct_stream_info_t session_based_stream_info;
} port_stream_config_t;

pthread_cond_t  frame_done_cond;
pthread_mutex_t mutex;
boolean         frame_pending = FALSE;

#define ISP_TEST_INPUT_WIDTH 1984
#define ISP_TEST_INPUT_HEIGHT 1508
#define PPROC_TEST_ALIGN_4K 4096

static boolean isp_test_port_event(mct_port_t *port, mct_event_t *event)
{
  if (!event) {
    CDBG_ERROR("failed:%s Null pointer event  %p",__func__, event);
    return FALSE;
  }
  if (!port) {
    CDBG("%s: failed: port %p\n", __func__, port);
  }
  switch (event->type)
  {
  case MCT_EVENT_MODULE_EVENT:
    {
      switch (event->u.module_event.type)
      {
      case MCT_EVENT_MODULE_STATS_AWB_UPDATE:
        CDBG_ERROR("received AWB event");
        break;
      case MCT_EVENT_MODULE_STATS_AEC_UPDATE:
        CDBG_ERROR("received AEC event");
        break;
      case MCT_EVENT_MODULE_SET_FLASH_MODE:
        CDBG_ERROR("received Flash event");
        break;
      default:
        break;
      }
      break;
    }
  default:
    break;
  }
  return TRUE;
}

static boolean isp_test_create_stream_info(
    camera_size_t     *mod,
    unsigned int      identity,
    mct_stream_info_t *stream_info)
{
  if (!stream_info) {
    CDBG_ERROR("%s: failed: Null Pointer %p\n", __func__, stream_info);
    return FALSE;
  }
  stream_info->identity       = identity;
  stream_info->dim.width      = mod->width;
  stream_info->dim.height     = mod->height;
  stream_info->fmt            = CAM_FORMAT_YUV_422_NV16;
  stream_info->stream_type    = CAM_STREAM_TYPE_PREVIEW;
  stream_info->streaming_mode = CAM_STREAMING_MODE_CONTINUOUS;

  stream_info->stream = mct_stream_new(identity & 0x0000FFFF);
  if (!stream_info->stream) {
    CDBG_ERROR("%s: failed: Null Pointer %p", __func__, stream_info->stream);
    return FALSE;
  }
  stream_info->stream->streaminfo = *stream_info;
  return TRUE;
}

static boolean isp_test_create_session_stream_info(unsigned int identity,
                                           mct_stream_info_t *stream_info)
{
  stream_info->identity = identity;
  stream_info->stream_type = CAM_STREAM_TYPE_PARM;
  stream_info->fmt = CAM_FORMAT_YUV_420_NV12;
  stream_info->dim.width = 0;
  stream_info->dim.height = 0;
  stream_info->streaming_mode = CAM_STREAMING_MODE_CONTINUOUS;
  stream_info->buf_planes.plane_info.num_planes= 0;
  stream_info->num_bufs = 0;
  stream_info->stream = mct_stream_new(identity);
  if (!stream_info->stream) {
    CDBG_ERROR("%s: Null pointer stream_info->stream %p\n", __func__,
      stream_info->stream);
    return FALSE;
  }
  stream_info->stream->streaminfo = *stream_info;
  return TRUE;
}

static boolean isp_test_destroy_stream_info(mct_stream_info_t *stream_info)
{
  pthread_mutex_destroy(MCT_OBJECT_GET_LOCK(stream_info->stream));
  free(stream_info->stream);
  return TRUE;
}

/** chromatix_load_lib:
 *
 **/
static void* chromatix_load_lib(void **chromatix_handle,
                                void *chromatix_name)
{
  char *new_chromatix = (char *)chromatix_name;
  void* (*open_lib)(void);

  if (!chromatix_handle) {
    CDBG_ERROR("failed: invalid params chromatix_handle %p", chromatix_handle);
    goto ERROR;
  }

  /* Open new chromatix library */
  *chromatix_handle = dlopen((const char *)new_chromatix, RTLD_NOW);
  if (!(*chromatix_handle)) {
    CDBG_ERROR("Failed to dlopen %s: %s", (char *)new_chromatix, dlerror());
    goto ERROR;
  }

  /* Find function pointer to open */
  *(void **)&open_lib = dlsym(*chromatix_handle, "load_chromatix");
  if (!open_lib) {
    CDBG_ERROR("Failed to find symbol: %s: :%s", (char *)new_chromatix, dlerror());
    goto ERROR;
  }

  /* Call open on chromatix library and get chromatix pointer */
  return (open_lib());

ERROR:
  return NULL;
}

/** get_sensor_fmt:
 *
 *  The test vectors only provide bayer_pattern. So convert
 *  it to sensor format. This is more like a workaround
 *  as the test vectors don't provide the sensor format that
 *  ISP internally uses to determine the pattern. For now only
 *  using 10BPP format.
 *
 **/
static cam_format_t get_sensor_fmt(uint8_t bayer_pattern)
{
  cam_format_t fmt = CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR;
  switch (bayer_pattern) {
  case ISP_BAYER_RGRGRG:
    fmt = CAM_FORMAT_BAYER_MIPI_RAW_10BPP_RGGB;
    break;
  case ISP_BAYER_GRGRGR:
    fmt = CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GRBG;
    break;
  case ISP_BAYER_BGBGBG:
    fmt = CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR;
    break;
  case ISP_BAYER_GBGBGB:
    fmt = CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GBRG;
    break;
  default:
    CDBG_TV("Bad bayer_pattern provided : %d\n", bayer_pattern);
    /* Just return default */
  }
  return fmt;
}

/** sensor_event_cfg:
 *
 *  @sensor_out_info: sensor out configure
 *  @sink_port: isp sink port
 *
 **/
static boolean sensor_event_cfg(sensor_out_info_t *sensor_out_info,
  mct_port_t *sink_port)
{
  mct_event_t event;
  unsigned int identity;
  if (!sensor_out_info || !sink_port) {
    CDBG_TV("failed: NULL POINTER %p %p\n", sensor_out_info, sink_port);
    return FALSE;
  }

  memset(sensor_out_info, 0, sizeof(sensor_out_info_t));
  //sensor_out_info_t *sensor_info = sensor_out_info;
  sensor_out_info->dim_output.height = vfe_test_vector.full_size.height;
  sensor_out_info->dim_output.width = vfe_test_vector.full_size.width;
  sensor_out_info->fmt = get_sensor_fmt(vfe_test_vector.bayer_pattern);
  sensor_out_info->mode = CAMERA_MODE_2D_B;
  sensor_out_info->request_crop.first_line = 0;
  sensor_out_info->request_crop.last_line = vfe_test_vector.camif_size.height - 1;
  sensor_out_info->request_crop.first_pixel = 0;
  sensor_out_info->request_crop.last_pixel = vfe_test_vector.camif_size.width - 1;
  sensor_out_info->op_pixel_clk = 264000000; /* HZ */
  sensor_out_info->num_frames_skip = 1;
  sensor_out_info->max_gain = 16;
  sensor_out_info->max_linecount = 57888;
  sensor_out_info->vt_pixel_clk = 330000000;
  sensor_out_info->ll_pck = 4480;
  sensor_out_info->fl_lines = 2412;
  sensor_out_info->pixel_sum_factor = 1;
  //new values
  sensor_out_info->sensor_rolloff_config.enable = TRUE;
  sensor_out_info->sensor_rolloff_config.scale_factor = vfe_test_vector.sensor_scaling;
  sensor_out_info->sensor_rolloff_config.full_width = vfe_test_vector.full_size.width;
  sensor_out_info->sensor_rolloff_config.full_height = vfe_test_vector.full_size.height;
  sensor_out_info->sensor_rolloff_config.output_width = vfe_test_vector.camif_size.width;
  sensor_out_info->sensor_rolloff_config.output_height = vfe_test_vector.camif_size.height;
  sensor_out_info->sensor_rolloff_config.offset_x = vfe_test_vector.x_offset;
  sensor_out_info->sensor_rolloff_config.offset_y = vfe_test_vector.y_offset;

  identity = pack_identity(1, 1);

  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_SET_STREAM_CONFIG;
  event.u.module_event.module_event_data = (void *)sensor_out_info;

  if (FALSE == sink_port->event_func(sink_port, &event)) {
    return FALSE;
  }

  return TRUE;
}

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x310)
/** pdaf_pattern_config:
 *
 *  @pdaf_config: pdaf pattern config
 *  @sink_port: isp sink port
 *
 **/
static boolean pdaf_pattern_config(pdaf_sensor_native_info_t *pdaf_config,
  mct_port_t *sink_port)
{
  mct_event_t event;
  unsigned int identity;
  if (!pdaf_config || !sink_port) {
    CDBG_ERROR("failed: NULL POINTER %p %p\n", pdaf_config, sink_port);
    return FALSE;
  }

  memset(pdaf_config, 0, sizeof(pdaf_sensor_native_info_t));

  pdaf_config->block_pattern.pd_offset_horizontal = vfe_test_vector.pdaf_xOffset;
  pdaf_config->block_pattern.pd_offset_vertical = vfe_test_vector.pdaf_yOffset;
  pdaf_config->block_pattern.block_dim.width = vfe_test_vector.camif_size.width;
  pdaf_config->block_pattern.block_dim.height = vfe_test_vector.camif_size.height;
  pdaf_config->block_pattern.pix_count = vfe_test_vector.pdaf_pix_cnt;

  identity = pack_identity(1, 1);

  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_SENSOR_PDAF_CONFIG;
  event.u.module_event.module_event_data = (void *)pdaf_config;

  if (FALSE == sink_port->event_func(sink_port, &event)) {
    return FALSE;
  }

  return TRUE;
}
#endif
/** isp_test_vector_fetch_params:
 *
 *  @aec: aec update stats
 *  @awb: awb update stats
 *
 **/
void isp_test_vector_fetch_params(
  stats_update_t *aec, stats_update_t *awb, iface_raw_stats_buf_info_t *raw_stats,
  vfe_test_vector_t *vfe_test_vector)
{
  aec->aec_update.lux_idx = vfe_test_vector->params.lux_idx;
  if (vfe_test_vector->params.led_enable) {
    aec->aec_update.flash_sensitivity = vfe_test_vector->params.sensitivity_led;
    aec->aec_update.strobe_enable = FALSE;
  } else if (vfe_test_vector->params.strobe_enable) {
    aec->aec_update.flash_sensitivity = vfe_test_vector->params.sensitivity_strobe;
    aec->aec_update.strobe_enable = TRUE;
  }
  aec->aec_update.dig_gain = vfe_test_vector->params.digital_gain;
  aec->aec_update.real_gain = vfe_test_vector->params.cur_real_gain;
  aec->aec_update.settled = 1;

  /* pre-ADRC modules(Pre GTM) should work on sensor gain
     post-ADRC modules(Post GTM) should work on real gain
     ADRC on: real_gain = sensor_gain * adrc_gain
     ADRC off: adrc gain = 1, ==> real_gain = sensor_gain */
  if (!vfe_test_vector->adrc_enable) {
    aec->aec_update.sensor_gain = vfe_test_vector->params.cur_real_gain;
    vfe_test_vector->adrc_total_gain = -1.0f;
  }
  else {
    aec->aec_update.sensor_gain = vfe_test_vector->params.cur_real_gain /
      ((vfe_test_vector->adrc_total_gain < 1.0f)?
        1.0f : vfe_test_vector->adrc_total_gain);
    if (aec->aec_update.sensor_gain < 1.0f) {
      CDBG_ERROR("Something is wrong! real_gain/adrc_gain should not be < 1");
      aec->aec_update.sensor_gain = 1.0f;
    }
  }

  aec->aec_update.hdr_sensitivity_ratio = vfe_test_vector->exp_ratio;
  aec->aec_update.hdr_exp_time_ratio = vfe_test_vector->exp_time_ratio;

  aec->aec_update.total_drc_gain = vfe_test_vector->adrc_total_gain;
  aec->aec_update.color_drc_gain = vfe_test_vector->adrc_color_gain;
  aec->aec_update.gamma_ratio = vfe_test_vector->adrc_gamma_ratio;
  aec->aec_update.ltm_ratio = vfe_test_vector->adrc_ltm_ratio;
  aec->aec_update.la_ratio = vfe_test_vector->adrc_la_ratio;
  aec->aec_update.gtm_ratio = vfe_test_vector->adrc_gtm_ratio;
  aec->aec_update.hdr_sensitivity_ratio      = vfe_test_vector->exp_ratio;

  awb->awb_update.gain.r_gain = vfe_test_vector->params.awb_gains.r_gain;
  awb->awb_update.gain.b_gain = vfe_test_vector->params.awb_gains.b_gain;
  awb->awb_update.gain.g_gain = vfe_test_vector->params.awb_gains.g_gain;
  awb->awb_update.color_temp = vfe_test_vector->params.color_temp;
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x309)
  awb->awb_update.dual_led_setting.is_valid = true;
  awb->awb_update.dual_led_setting.led1_high_setting = vfe_test_vector->params.led1_current;
  awb->awb_update.dual_led_setting.led2_high_setting = vfe_test_vector->params.led2_current;
#else
  aec->aec_update.dual_led_setting.is_valid = true;
  aec->aec_update.dual_led_setting.led1_high_setting = vfe_test_vector->params.led1_current;
  aec->aec_update.dual_led_setting.led2_high_setting = vfe_test_vector->params.led2_current;
#endif

  memset(raw_stats, 0, sizeof(*raw_stats));
  raw_stats->session_id = 1;
  raw_stats->stats_mask = 1 << MSM_ISP_STATS_BHIST;
  raw_stats->raw_stats_buffer[MSM_ISP_STATS_BHIST] =
    (void *)vfe_test_vector->params.raw_bhist_stats;
}

boolean isp2_find_src_port(void *data1, void *data2)
{
  mct_port_t *port = (mct_port_t *)data1;
  mct_stream_info_t *stream_info = (mct_stream_info_t *)data2;

  if (!port || !stream_info) {
    CDBG_ERROR("failed port %p stream_info %p", data1,
               data2);
    return FALSE;
  }
  CDBG_ERROR("src port type %d", port->caps.port_caps_type);

  if (port->caps.port_caps_type == MCT_PORT_CAPS_FRAME) {
    CDBG_ERROR("%s%d, return", __func__, __LINE__);
    return FALSE;
  }

  if (port->check_caps_reserve(port, NULL, stream_info) == TRUE) {
    return TRUE;
  }

  return FALSE;
}


boolean isp2_find_sink_port(void *data1, void *data2)
{
  mct_port_t *port = (mct_port_t *)data1;
  mct_stream_info_t *stream_info = (mct_stream_info_t *)data2;

  if (!port || !stream_info) {
    CDBG_ERROR("failed port %p stream_info %p", data1,
               data2);
    return FALSE;
  }

  if (port->check_caps_reserve(port, NULL, stream_info) == TRUE) {
    return TRUE;
  }

  return FALSE;
}

/** port_stream_cfg:
 *
 *  @port_stream_config: configure ports and stream
 *  @isp: isp module
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean port_stream_cfg(
  vfe_test_vector_t    *mod,
  port_stream_config_t *port_stream_config,
  mct_module_t         *isp)
{
  unsigned int identity;
  int rc;
  mct_list_t *s_list;

  identity = pack_identity(1, 1);

  /* Create stream info */
  memset(&port_stream_config->stream_info, 0, sizeof(mct_stream_info_t));
  rc = isp_test_create_stream_info(&mod->output_size, identity, &port_stream_config->stream_info);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error creating stream info\n", __func__, __LINE__);
    return FALSE;
  }
/* session based stream */
  identity = 15;
  identity = pack_identity(1, 15);
  memset(&port_stream_config->session_based_stream_info, 0,
    sizeof(mct_stream_info_t));
  rc = isp_test_create_session_stream_info(identity,
    &port_stream_config->session_based_stream_info);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error creating stream info\n", __func__, __LINE__);
    return FALSE;
  }

  /*find sink port in isp module*/
  s_list = mct_list_find_custom(MCT_MODULE_SINKPORTS(isp),
    &port_stream_config->session_based_stream_info, isp2_find_sink_port);
  if (!s_list) {
    CDBG_ERROR("%s:%d] error finding sink port\n", __func__, __LINE__);
    return FALSE;
  }
  port_stream_config->session_port = MCT_PORT_CAST(s_list->data);
  rc = mct_port_add_child(identity, port_stream_config->session_port);
  if (rc == FALSE) {
    CDBG_ERROR("failed to add child");
    return FALSE;
  }

  mct_event_t event;
  mct_port_t *session_port = port_stream_config->session_port;

  sensor_set_dim_t dim;
  memset(&dim, 0, sizeof(dim));
  dim.op_pixel_clk      = 264000000;
  dim.output_format     = CAM_FORMAT_BAYER_MIPI_RAW_10BPP_BGGR;
  dim.dim_output.height = mod->camif_size.height;
  dim.dim_output.width  = mod->camif_size.width;
  memset(&event, 0, sizeof(event));

  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_SET_SENSOR_OUTPUT_INFO;
  event.u.module_event.module_event_data = (void *)&dim;
  if (FALSE == session_port->event_func(session_port, &event)) {
    CDBG_ERROR("%s:%d] error sending sensor output info\n", __func__,
      __LINE__);
    return FALSE;
  }

  cam_stream_size_info_t streaminfo;
  memset(&streaminfo, 0, sizeof(streaminfo));

  streaminfo.stream_sizes[0].width  = mod->output_size.width;
  streaminfo.stream_sizes[0].height = mod->output_size.height;
  streaminfo.num_streams            = 1;
  streaminfo.type[0]                = CAM_STREAM_TYPE_PREVIEW;

  mct_event_control_parm_t parm_data;
  memset(&parm_data, 0, sizeof(parm_data));

  parm_data.type = CAM_INTF_META_STREAM_INFO;
  parm_data.parm_data = &streaminfo;

  memset(&event, 0, sizeof(event));
  event.type =  MCT_EVENT_CONTROL_CMD;
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_SET_PARM;
  event.u.ctrl_event.control_event_data = (void *)&parm_data;
  if (FALSE == session_port->event_func(session_port, &event)) {
    CDBG_ERROR("%s:%d] error sending sensor meta stream info\n", __func__,
      __LINE__);
    return FALSE;
  }

  identity = pack_identity(1, 1);

  /* Create a test port and set function handles */
  port_stream_config->test_port = mct_port_create("test_port");
  if (!port_stream_config->test_port) {
    CDBG_ERROR("%s:%d] error creating test port", __func__, __LINE__);
    return FALSE;
  }
  mct_port_set_event_func(port_stream_config->test_port, isp_test_port_event);



  s_list = mct_list_find_custom(MCT_MODULE_SINKPORTS(isp),
                                &port_stream_config->stream_info, isp2_find_sink_port);
  if (!s_list) {
    CDBG_ERROR("%s:%d] error finding sink port\n", __func__, __LINE__);
    return FALSE;
  }
  port_stream_config->sink_port = MCT_PORT_CAST(s_list->data);
  rc = mct_port_add_child(identity, port_stream_config->sink_port);
  if (rc == FALSE) {
    CDBG_ERROR("failed to add child");
    return FALSE;
  }

  /* Ext link on isp port */
  rc = port_stream_config->sink_port->ext_link(identity, port_stream_config->sink_port, port_stream_config->test_port);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error linking isp port\n", __func__, __LINE__);
    return FALSE;
  }

  rc = mct_port_add_child(identity, port_stream_config->sink_port);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error adding identity to port\n", __func__, __LINE__);
    return FALSE;
  }

  /****************************************/
  /*find source port in isp module*/
  s_list = mct_list_find_custom(MCT_MODULE_SRCPORTS(isp),
                                &port_stream_config->stream_info, isp2_find_src_port);
  if (!s_list) {
    CDBG_ERROR("%s:%d] error finding port\n", __func__, __LINE__);
    return FALSE;
  }
  port_stream_config->source_port = MCT_PORT_CAST(s_list->data);
  rc = mct_port_add_child(identity, port_stream_config->source_port);
  if (rc == FALSE) {
    CDBG_ERROR("failed to add child");
    return FALSE;
  }
  CDBG_ERROR("source port type %d", port_stream_config->source_port->caps.port_caps_type);
  /* Ext link on source port */
  rc = port_stream_config->source_port->ext_link(identity, port_stream_config->source_port, port_stream_config->test_port);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error linking isp port\n", __func__, __LINE__);
    return FALSE;
  }

  rc = mct_object_set_parent(MCT_OBJECT_CAST(isp),
                             MCT_OBJECT_CAST(port_stream_config->stream_info.stream));
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error adding isp to stream\n", __func__, __LINE__);
    return FALSE;
  }

  return TRUE;
}

static int run_test(char *datafile, char* chromatix_file_common,
  char* chromatix_file_preview, char* chromatix_file_3a,
  testcase_report_t* testcase_report)
{
  boolean                  rc = FALSE;
  mct_module_t            *isp;
  mct_port_t              *isp_port = NULL,*test_port = NULL;
  mct_port_t              *sink_port = NULL,*source_port = NULL;
  mct_stream_info_t        stream_info;
  unsigned int             identity;
  mct_event_t              event;
  mct_pipeline_t          *pipeline = NULL;
  mct_bus_msg_isp_sof_t sof_event;

  void *chromatix_3a_handle = NULL;
  void *chromatix_handle = NULL;
  void *chromatix_common_handle = NULL;

  void *chromatix_3a_ptr;
  void *chromatix_ptr;
  void *chromatix_common_ptr;
  modulesChromatix_t module_chromatix;
  iface_resource_request_t isp_resource_request;
  uint32_t frame_id = 0;

  iface_raw_stats_buf_info_t raw_stats;
  sensor_out_info_t sensor_out_info;
  stats_update_t    awb_update_data;
  stats_update_t    aec_update_data;
  cam_flash_mode_t mode;
  isp_hdr_mode_t    isp_hdr_mode;
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x310)
  pdaf_sensor_native_info_t pdaf_config;
#endif
  port_stream_config_t port_strm_config;

  if (!chromatix_file_common || !chromatix_file_3a ||
      !chromatix_file_preview || !testcase_report) {
    CDBG_ERROR("%s: null pointer!  EXIT!!!!\n", __func__);
    goto EXIT;
  }

  CDBG_ERROR("bet_dbg module isp init");
  isp = module_isp_init("isp");
  if (!isp) {
    CDBG_ERROR("%s:%d] error getting isp module\n", __func__, __LINE__);
    goto EXIT;
  }

  identity = pack_identity(1, 1);
  isp->set_mod(isp, MCT_MODULE_FLAG_INDEXABLE, identity);

  CDBG_ERROR("bet_dbg module start session");
  if (isp->start_session) {
    rc = isp->start_session(isp, 1);
    if (rc == FALSE) {
      CDBG_ERROR("%s:%d] error starting session in isp\n", __func__, __LINE__);
      goto EXIT;
    }
  }

  /*load libchromatix lib and fill the structures*/
  CDBG_ERROR("bet_dbg load libchromatix lib");
  chromatix_common_ptr =
     chromatix_load_lib(&chromatix_common_handle, chromatix_file_common);
  if (!chromatix_common_ptr) {
    CDBG_ERROR("Failed to dlopen %s: %s", chromatix_file_common, dlerror());
    return 0;
  }

  chromatix_ptr =
     chromatix_load_lib(&chromatix_handle, chromatix_file_preview);
  if (!chromatix_handle) {
    CDBG_ERROR("Failed to dlopen %s: %s", chromatix_file_preview, dlerror());
    return 0;
  }
  if (!chromatix_ptr) {
    CDBG_ERROR("%s: failed: chromatix_ptr %p\n", __func__, chromatix_ptr);
    return 0;
  }


  module_chromatix.chromatixPtr = chromatix_ptr;
  module_chromatix.chromatixComPtr = chromatix_common_ptr;
  CDBG_ERROR("bet_dbg init");
  memset(&vfe_test_vector, 0, sizeof(vfe_test_vector_t));
  vfe_test_vector.chromatix_data = (chromatix_parms_type *)chromatix_ptr;
  vfe_test_vector_init(&vfe_test_vector, datafile);

  CDBG_ERROR("bet_dbg port_stream_cfg");
  /*source,sink,test port and stream config*/
  if (!port_stream_cfg(&vfe_test_vector, &port_strm_config, isp)) {
    CDBG_ERROR("%s:%d] error configuring port and stream\n", __func__, __LINE__);
    goto EXIT;
  }

    /*Sensor config event*/
  CDBG_ERROR("bet_dbg sensor_event_cfg");
  if (!sensor_event_cfg(&sensor_out_info, port_strm_config.sink_port)) {
    CDBG_ERROR("%s: error in stream config\n", __func__);
    goto EXIT;
  }

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x310)
    /*PDAF config */
  if (!pdaf_pattern_config(&pdaf_config, port_strm_config.sink_port)) {
    CDBG_ERROR("%s: error in pdaf config\n", __func__);
    goto EXIT;
  }
#endif

#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION >= 0x306)
  chromatix_3a_ptr =
     chromatix_load_lib(&chromatix_handle, chromatix_file_3a);
  if (!chromatix_handle) {
    CDBG_ERROR("Failed to dlopen %s: %s", chromatix_file_3a, dlerror());
    return 0;
  }
  if (!chromatix_3a_ptr) {
    CDBG_ERROR("%s: Null pointer %p\n", __func__, chromatix_3a_ptr);
    return 0;
  }

  module_chromatix.chromatix3APtr = chromatix_3a_ptr;
#endif

  sink_port = port_strm_config.sink_port;
  /*send chromatix ptr event*/
  CDBG_ERROR("bet_dbg chromatix ptr event, %p, %p, %p", module_chromatix.chromatixPtr,
    module_chromatix.chromatixComPtr, module_chromatix.chromatix3APtr);
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_SET_CHROMATIX_PTR;
  event.u.module_event.module_event_data = (void *)&module_chromatix;
  if (FALSE == sink_port->event_func(sink_port, &event)) {
    CDBG_ERROR("%s: error in set chromatix\n", __func__);
    goto EXIT;
  }

  CDBG_ERROR("bet_dbg set hdr mode");
  memset(&event, 0, sizeof(mct_event_t));
  memset(&isp_hdr_mode , 0 , sizeof(isp_hdr_mode_t));
  if (vfe_test_vector.hdr_mode == VFE_TEST_NON_HDR){
    isp_hdr_mode.hdr_mode = CAM_SENSOR_HDR_OFF;
  } else if (vfe_test_vector.hdr_mode == VFE_TEST_HDR){
    isp_hdr_mode.hdr_mode = CAM_SENSOR_HDR_ZIGZAG;
  } else if (vfe_test_vector.hdr_mode == VFE_TEST_MODE_SENSOR_HDR){
    isp_hdr_mode.hdr_mode = CAM_SENSOR_HDR_IN_SENSOR;
  }
  isp_hdr_mode.use_gtm_fix_curve  = FALSE;

  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_HDR;
  event.u.module_event.module_event_data = (void *)&isp_hdr_mode;
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  if (FALSE == sink_port->event_func(sink_port, &event)) {
    CDBG_ERROR("%s: error in set chromatix\n", __func__);
    goto EXIT;
  }

  CDBG_ERROR("bet_dbg fetch awb aec params and raw stats");
  isp_test_vector_fetch_params(&aec_update_data, &awb_update_data, &raw_stats,
    &vfe_test_vector);

  /* send AWB update event */
  CDBG_ERROR("bet_dbg awb update event");
  source_port = port_strm_config.source_port;
  event.identity = identity;
  event.direction = MCT_EVENT_UPSTREAM;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_AWB_UPDATE;
  event.u.module_event.module_event_data = (void *)&awb_update_data;
  rc = source_port->event_func(source_port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error sending AWB update event\n", __func__,
               __LINE__);
    goto EXIT;
  }

  /* iface resource request event */
  memset(&isp_resource_request, 0, sizeof(iface_resource_request_t));
  CDBG_ERROR("bet_dbg iface_resource request");

  memset(&event, 0, sizeof(mct_event_t));
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_IFACE_REQUEST_OUTPUT_RESOURCE ;
  event.u.module_event.module_event_data = (void *)&isp_resource_request;
  rc = sink_port->event_func(sink_port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error in set_stream config\n", __func__, __LINE__);
    goto EXIT;
  }

  /* Stream on event */
  CDBG_ERROR("bet_dbg stream on");
  memset(&event, 0, sizeof(mct_event_t));
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_CONTROL_CMD;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_STREAMON;
  event.u.ctrl_event.control_event_data = (void *)&stream_info;
  rc = sink_port->event_func(sink_port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error in streaming on\n", __func__, __LINE__);
    goto EXIT;
  }

  usleep(1000);

  /* send AEC update event */
  CDBG_ERROR("bet_dbg AEC update event");
  event.identity = identity;
  event.direction = MCT_EVENT_UPSTREAM;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_AEC_UPDATE;
  event.u.module_event.module_event_data = (void *)&aec_update_data;
  rc = source_port->event_func(source_port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error sending AEC update event\n", __func__,
               __LINE__);
    goto EXIT;
  }

  usleep(1000);

  if (vfe_test_vector.params.strobe_enable
      ||  vfe_test_vector.params.led_enable) {
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x309)
    mode = CAM_FLASH_MODE_ON;
#else
    if (vfe_test_vector.params.led_enable)
      mode = CAM_FLASH_MODE_ON;
#if defined(CHROMATIX_VERSION) && (CHROMATIX_VERSION == 0x310)
    else
      mode = CAM_FLASH_MODE_TORCH;
#endif
#endif
    /* send Flash update event, for CC and rolloff */
    CDBG_ERROR("bet_dbg flash update event led en %d, strobe en %d", vfe_test_vector.params.strobe_enable,
      vfe_test_vector.params.led_enable);
    event.identity = identity;
    event.direction = MCT_EVENT_UPSTREAM;
    event.type = MCT_EVENT_MODULE_EVENT;
    event.u.module_event.type = MCT_EVENT_MODULE_SET_FLASH_MODE;
    event.u.module_event.module_event_data = (void *)&mode;
    rc = source_port->event_func(source_port, &event);
    if (rc == FALSE) {
      CDBG_ERROR("%s:%d] error sending Flash update event\n", __func__,
                 __LINE__);
      goto EXIT;
    }
  }
  stream_info = port_strm_config.stream_info;


  stream_info = port_strm_config.stream_info;
   /*create reg update event*/
  CDBG_ERROR("bet_dbg REG UPDATE 1......");
  frame_id = 1;
  memset(&event, 0, sizeof(mct_event_t));
  event.identity = identity;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_REG_UPDATE_NOTIFY;
  event.u.module_event.module_event_data =
    (void *)&frame_id;
  event.u.module_event.current_frame_id = frame_id;
  sink_port->event_func(sink_port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error sending module SOF event\n", __func__,
      __LINE__);
    goto EXIT;
  }

  /*Create a SOF (control sof + mct sof):
    control sof: trigger update thread: set new algo parm, update enable bit
                 hw update thread
    moduel sof: reset hw busy bit;*/
  CDBG_ERROR("bet_dbg SOF1......");
  memset(&event, 0, sizeof(mct_event_t));
  memset(&sof_event, 0, sizeof(mct_bus_msg_isp_sof_t));
  sof_event.frame_id = frame_id;
  event.u.module_event.type = MCT_EVENT_MODULE_SOF_NOTIFY;
  event.u.module_event.module_event_data = (void *)&sof_event;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  sink_port->event_func(sink_port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error sending module SOF event\n", __func__,
      __LINE__);
    goto EXIT;
  }

  /*Create a SOF: trigger update thread: set new algo parm */
  CDBG_ERROR("bet_dbg SOF1");
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_CONTROL_CMD;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_SOF;
  event.u.ctrl_event.control_event_data = (void *)&sof_event;
  rc = sink_port->event_func(sink_port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error sending control SOF event\n", __func__,
               __LINE__);
    goto EXIT;
  }

  /* wait for 100 ms to make sure trigger updates are done */
  usleep(100000);

   /*create reg update event*/
  CDBG_ERROR("bet_dbg REG UPDATE 2......");
  frame_id = 2;
  memset(&event, 0, sizeof(mct_event_t));
  event.identity = identity;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_REG_UPDATE_NOTIFY;
  event.u.module_event.module_event_data =
    (void *)&frame_id;
  event.u.module_event.current_frame_id = frame_id;
  sink_port->event_func(sink_port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error sending module SOF event\n", __func__,
      __LINE__);
    goto EXIT;
  }

  /*Create a SOF: trigger update thread: set new algo parm */
  CDBG_ERROR("bet_dbg SOF2.......");

  memset(&event, 0, sizeof(mct_event_t));
  memset(&sof_event, 0, sizeof(mct_bus_msg_isp_sof_t));
  sof_event.frame_id = frame_id;
  event.u.module_event.type = MCT_EVENT_MODULE_SOF_NOTIFY;
  event.u.module_event.module_event_data = (void *)&sof_event;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  rc = sink_port->event_func(sink_port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error sending module SOF event\n", __func__,
      __LINE__);
    goto EXIT;
  }

  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_CONTROL_CMD;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_SOF;
  event.u.ctrl_event.control_event_data = (void *)&sof_event;
  rc = sink_port->event_func(sink_port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error sending SOF event\n", __func__,
               __LINE__);
    goto EXIT;
  }

  /* wait for 100 ms to make sure trigger updates are done */
  usleep(100000);

  /* send raw stats event */
  raw_stats.frame_id = frame_id;
  CDBG_ERROR("bet_dbg Raw stats event frame id = %d\n", raw_stats.frame_id);
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.u.module_event.type = MCT_EVENT_MODULE_RAW_STATS_DIVERT;
  event.u.module_event.module_event_data = (void *)&raw_stats;
  rc = sink_port->event_func(sink_port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error sending raw stats event\n", __func__,
               __LINE__);
    goto EXIT;
  }

  /* wait for 100 ms to make sure parser thread is done */
  usleep(100000);


  /*create reg update event*/
  CDBG_ERROR("bet_dbg REG UPDATE 3......");
  frame_id = 3;
  memset(&event, 0, sizeof(mct_event_t));
  event.identity = identity;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_REG_UPDATE_NOTIFY;
  event.u.module_event.module_event_data =
    (void *)&frame_id;
  event.u.module_event.current_frame_id = frame_id;
  sink_port->event_func(sink_port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error sending module SOF event\n", __func__,
      __LINE__);
    goto EXIT;
  }

  /*Create a SOF: let trigger update takes the new algo output */
  CDBG_ERROR("bet_dbg SOF3.......");

  memset(&event, 0, sizeof(mct_event_t));
  memset(&sof_event, 0, sizeof(mct_bus_msg_isp_sof_t));

  sof_event.frame_id = frame_id;
  event.u.module_event.type = MCT_EVENT_MODULE_SOF_NOTIFY;
  event.u.module_event.module_event_data = (void *)&sof_event;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  rc = sink_port->event_func(sink_port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error sending module SOF event\n", __func__,
      __LINE__);
    goto EXIT;
  }
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_CONTROL_CMD;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_SOF;
  event.u.ctrl_event.control_event_data = (void *)&sof_event;
  rc = sink_port->event_func(sink_port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error sending SOF event\n", __func__,
               __LINE__);
    goto EXIT;
  }

  /* wait for 100 ms to ensure all trigger updates are done */
  usleep(100000);

  /*create reg update event*/
  CDBG_ERROR("bet_dbg REG UPDATE 4......");
  frame_id = 4;
  memset(&event, 0, sizeof(mct_event_t));
  event.identity = identity;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_REG_UPDATE_NOTIFY;
  event.u.module_event.module_event_data =
    (void *)&frame_id;
  event.u.module_event.current_frame_id = frame_id;
  sink_port->event_func(sink_port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error sending module SOF event\n", __func__,
      __LINE__);
    goto EXIT;
  }
  /*Create a SOF: HW update thread*/
  CDBG_ERROR("bet_dbg SOF4.......");

  memset(&event, 0, sizeof(mct_event_t));
  memset(&sof_event, 0, sizeof(mct_bus_msg_isp_sof_t));

  sof_event.frame_id = frame_id;
  event.u.module_event.type = MCT_EVENT_MODULE_SOF_NOTIFY;
  event.u.module_event.module_event_data = (void *)&sof_event;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  rc = sink_port->event_func(sink_port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error sending module SOF event\n", __func__,
      __LINE__);
    goto EXIT;
  }
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_CONTROL_CMD;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_SOF;
  event.u.ctrl_event.control_event_data = (void *)&sof_event;
  rc = sink_port->event_func(sink_port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error sending SOF event\n", __func__,
               __LINE__);
    goto EXIT;
  }

  /* wait for 100 ms to ensure all trigger updates are done */
  usleep(100000);

  /*create reg update event*/
  CDBG_ERROR("bet_dbg REG UPDATE 5......");
  frame_id = 5;
  memset(&event, 0, sizeof(mct_event_t));
  event.identity = identity;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_REG_UPDATE_NOTIFY;
  event.u.module_event.module_event_data =
    (void *)&frame_id;
  event.u.module_event.current_frame_id = frame_id;
  sink_port->event_func(sink_port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error sending module SOF event\n", __func__,
      __LINE__);
    goto EXIT;
  }
  /*Create a SOF: HW update thread*/
  CDBG_ERROR("bet_dbg SOF5.......");

  memset(&event, 0, sizeof(mct_event_t));
  memset(&sof_event, 0, sizeof(mct_bus_msg_isp_sof_t));

  sof_event.frame_id = frame_id;
  event.u.module_event.type = MCT_EVENT_MODULE_SOF_NOTIFY;
  event.u.module_event.module_event_data = (void *)&sof_event;
  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  rc = sink_port->event_func(sink_port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error sending module SOF event\n", __func__,
      __LINE__);
    goto EXIT;
  }
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_CONTROL_CMD;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_SOF;
  event.u.ctrl_event.control_event_data = (void *)&sof_event;
  rc = sink_port->event_func(sink_port, &event);
  if (rc == FALSE) {
    CDBG_ERROR("%s:%d] error sending SOF event\n", __func__,
               __LINE__);
    goto EXIT;
  }
  /* wait for 100 ms to ensure all hw updates are done */
  usleep(100000);

  /* execute the test vector matching */
  CDBG_ERROR("bet_dbg compare");
  vfe_test_vector_execute(&vfe_test_vector, testcase_report, &bet_report.module_name[0]);

  /* Stream off event */
  CDBG_ERROR("bet_dbg stream off");
  memset(&event, 0, sizeof(mct_event_t));
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.type = MCT_EVENT_CONTROL_CMD;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_STREAMOFF;
  event.u.ctrl_event.control_event_data = (void *)&stream_info;
  sink_port->event_func(sink_port, &event);

  test_port = port_strm_config.test_port;

  CDBG_ERROR("module unlink sink");
  sink_port->un_link(identity, sink_port, test_port);

  CDBG_ERROR("module unlink source");
  source_port->un_link(identity, source_port, test_port);

  CDBG_ERROR("module check caps unreserve");
  sink_port->check_caps_unreserve(sink_port, identity);

  CDBG_ERROR("module check caps unreserve source");
  source_port->check_caps_unreserve(source_port, identity);

  CDBG_ERROR("stop session");
  if (isp->stop_session) {
    isp->stop_session(isp, 1);
  }

  CDBG_ERROR("module deinit");
  module_isp_deinit(isp);

  CDBG_ERROR("vfe test vector deinit");
  if (0 != (rc = vfe_test_vector_deinit(&vfe_test_vector)))
    CDBG_ERROR("%s: vfe_test_vector_deinit err = %d", __func__, rc);

  usleep(100000);
EXIT:
  return 0;
}

static int execute_bet_test(boolean is_lux, int testnum, char* chromatix_common,
  char* chromatix_preview, char* chromatix_3a)
{
  DIR           *d = NULL;
  struct dirent *dir;
  char testfile[64];
  char* testdir = NULL;
  boolean mismatch[ISP_MOD_MAX_NUM];
  testcase_report_t *testcase_report = NULL;
  int i = 0;
  uint32_t num_test_case = 0;

  if (testnum > MAX_TEST_CASE_NUM) {
    printf("testcase %d go over max can support %d",
      testnum, MAX_TEST_CASE_NUM);
    exit(0);
  }

  if (testnum == -1) {
    for (i = 1; i < MAX_TEST_CASE_NUM + 1; i++) {
      if (is_lux == TRUE) {
        testdir = "/system/etc/lux";
        testcase_report = &bet_report.lux[i];
      } else {
        testdir = "/system/etc/gain";
        testcase_report = &bet_report.gain[i];
      }
      snprintf(testfile, sizeof(testfile), "%s/output%d.txt", testdir, i);
      if (access(testfile, F_OK) != -1) {
        printf("Running test with data file: %s\n", testfile);
        run_test(testfile, chromatix_common, chromatix_preview, chromatix_3a, testcase_report);
        if (has_mismatch && quit_upon_err)
           exit(1);
      } else {
        printf("%s does not exist finish.......\n", testfile);
        return 0;
      }
    }
  } else { /* specific data file requested */
    if (is_lux == TRUE) {
      testdir = "/system/etc/lux";
      testcase_report = &bet_report.lux[testnum];
    } else {
      testdir = "/system/etc/gain";
      testcase_report = &bet_report.gain[testnum];
    }
    snprintf(testfile, sizeof(testfile), "%s/output%d.txt", testdir, testnum);
    if (access(testfile, F_OK) != -1) {
      printf("Running test with data file: %s\n", testfile);
      run_test(testfile, chromatix_common, chromatix_preview, chromatix_3a, testcase_report);
      if (has_mismatch && quit_upon_err)
         exit(1);
    } else {
      printf("%s does not exist\n", testfile);
    }
  }

  return 0;
}

void generate_bet_report() {
  int i = 0;
  int j = 0;
  testcase_report_t *testcase_report = NULL;
  boolean is_mod_mismatch = FALSE;

  printf("\n\n\n   ============== BET TEST Report =================\n");
  for (i = 0; i < ISP_MOD_MAX_NUM; i++) {
    is_mod_mismatch = FALSE;
    if (bet_report.module_name[i] == NULL) {
      continue;
    }
    printf("\tModule name: %s ", bet_report.module_name[i]);
    for (j = 0; j < MAX_TEST_CASE_NUM; j++) {
      testcase_report = &bet_report.lux[j];
      if (testcase_report->mismatch[i] == 1) {
        if (is_mod_mismatch == FALSE) {
          printf("==> Mismatch\n \t mismatch on: ");
        }
        is_mod_mismatch = TRUE;
        printf(" Lux %d, ",j);
      }
    }
    for (j = 0; j < MAX_TEST_CASE_NUM; j++) {
      testcase_report = &bet_report.gain[j];
      if (testcase_report->mismatch[i] == 1) {
        if (is_mod_mismatch == FALSE) {
          printf("==> Mismatch\n \t mismatch on: ");
        }
        is_mod_mismatch = TRUE;
        printf(" Gain %d,",j);
      }
    }
    if (is_mod_mismatch == FALSE) {
      printf("(O) \n");
    } else {
      printf("\n\t **** FAIL ****\n\n");
    }
  }
}

int main(int argc, char *argv[])
{
  int run_gain = 0;
  int run_lux = 0;
  int c;
  int gain_file = -1;
  int lux_file = -1;
  boolean is_lux = FALSE;

/* STEP To perform BET test:
   1. Compile whole system image(only first time)
      or  compile mm-camera (after first time)
   2. 'make snod' in root
   3. Push system.img and reboot
   4. run following adb command:
      ---adb root;
      ---adb remount;
      ---adb shell /system/bin/test_bet_8084 -l4' for example

   Test executable command line options:

   usage: test_bet [-a(all)] [-g(gain) testnum] [-l(lux) testnum]
                   [-s(show mismatch details)]
                   [-q(quit upon first encountered error]
   example:
           test_bet -a     Test all vectors (both gain and lux_index)
           test_bet -g2 -s Test gain-based vector in output2.txt,
                           showing details for any mismatch
           test_bet -l4    Test lux-based vector in output4.txt */
  if (argc == 1) {
    /* no argument */
    printf("usage: test_bet [-a(all)] [-g(gain) testnum] "
           "[-l(lux) testnum] [-s(show mismatch details)] "
           "[-q(quit upon first encountered error]\n");
    printf("example:\n\ttest_bet -a\tTest all vectors (both gain and lux_index)\n");
    printf("\ttest_bet -g2 -s\tTest gain-based vector in output2.txt, "
           "showing details for any mismatch\n");
    printf("\ttest_bet -l4\tTest lux-based vector in output4.txt\n");
    return 0;
  } else {
    opterr = 0;
    while ((c = getopt (argc, argv, "asg:l:q")) != -1) {
      switch (c) {
        case 'a':
          run_gain = 1;
          run_lux = 1;
          break;
        case 'g':
          run_gain = 1;
          gain_file = atoi(optarg);
          break;
        case 'l':
          run_lux = 1;
          lux_file = atoi(optarg);
          break;
        case 's':
          show_mismatch = 1;
          break;
        case 'q':
          quit_upon_err = 1;
          break;
        case '?':
          if (isprint (optopt))
            fprintf (stderr, "Unknown option `-%c'.\n", optopt);
          else
            fprintf (stderr,
                     "Unknown option character `\\x%x'.\n",
                     optopt);
          return 1;
        default:
          abort ();
      }
    }
  }

  if (run_gain) {
    is_lux = FALSE;
    execute_bet_test(is_lux, gain_file, CHROMATIX_COMMON_LIB_GAIN,
      CHROMATIX_PREVIEW_LIB_GAIN, CHROMATIX_3A_LIB_GAIN);
  }
  if (run_lux) {
    is_lux = TRUE;
    execute_bet_test(is_lux, lux_file, CHROMATIX_COMMON_LIB_LUX,
      CHROMATIX_PREVIEW_LIB_LUX, CHROMATIX_3A_LIB_LUX);
  }

  generate_bet_report();
  return has_mismatch;
}
