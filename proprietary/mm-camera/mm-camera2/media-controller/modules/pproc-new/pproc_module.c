/****************************************************************************
* Copyright (c) 2013-2019 Qualcomm Technologies, Inc.                       *
* All Rights Reserved.                                                      *
* Confidential and Proprietary - Qualcomm Technologies, Inc.                *
****************************************************************************/

#include "camera_dbg.h"
#include "cam_intf.h"
#include "cam_types.h"
#include "modules.h"
#include "mct_list.h"
#include "mct_module.h"
#include "mct_port.h"
#include "eztune_diagnostics.h"
#include "pproc_module.h"
#include "pproc_port.h"
#include "mct_controller.h"
//#include "module_cac.h"
#include "c2d_module.h"
#include "cpp_module.h"
#ifdef CAMERA_FEATURE_VPU
#include "vpu_module.h"
#endif
#ifdef CAMERA_FEATURE_VPE
#include "vpe_module.h"
#endif
#ifdef CAMERA_FEATURE_WNR_SW
#include "wnr_module.h"
#endif
#include "module_imglib.h"
#include "base_module.h"
#include "eztune_module.h"
#include <media/msmb_isp.h>
#include <cutils/properties.h>

#undef PPROC_OFFLINE_USE_V4L2
//#define PPROC_OFFLINE_USE_V4L2

#define PPROC_MAX_PORTS 16


typedef struct {
    cam_denoise_param_t denoise2d;
    int32_t sharpness; /* 0 means no sharpness */
    int32_t effect;
    int32_t scene_mode;
    cam_rotation_t rotation;
    cam_rotation_t device_rotation;
    uint32_t flip;
    cam_denoise_param_t tnr;
    int32_t dsdn;
    cam_dyn_img_data_t dyn_img_data;
    cam_sensor_hdr_type_t hdr_mode;
} pproc_module_pp_config_t;

static mct_module_init_name_t pproc_modules_list[] = {
#ifdef CAMERA_FEATURE_CPP
  {"cpp", cpp_module_init, cpp_module_deinit, NULL},
#endif
#ifdef CAMERA_FEATURE_VPE
  {"vpe", vpe_module_init, vpe_module_deinit, NULL},
#endif
#ifdef CAMERA_FEATURE_C2D
  {"c2d", c2d_module_init, c2d_module_deinit, NULL},
#endif
#ifdef CAMERA_FEATURE_THIRDPARTYEIS
  {"thirdpartyeis_p", module_thirdpartyprevieweis_init, module_thirdpartyprevieweis_deinit, NULL},
  {"thirdpartyeis_v", module_thirdpartyvideoeis_init, module_thirdpartyvideoeis_deinit, NULL},
#endif
#ifdef CAMERA_FEATURE_PPEISCORE
  {"ppeiscore", module_ppeiscore_init, module_ppeiscore_deinit, NULL},
#endif
#ifdef CAMERA_FEATURE_VPU
  {"vpu", vpu_module_init, vpu_module_deinit, NULL},
#endif
#ifdef CAMERA_FEATURE_TMOD
  {"tmod", base_module_init, base_module_deinit, NULL},
  {"tmod2", base_module_init, base_module_deinit, NULL},
#endif
#ifdef CAMERA_FEATURE_AFS
  {"paaf", module_paaf_init, module_paaf_deinit, NULL},
#endif
#ifdef CAMERA_FEATURE_EZT
  {"ezt", eztune_module_init, eztune_module_deinit, NULL},
#endif
#ifdef CAMERA_FEATURE_LLVD
  {"llvd", module_llvd_init, module_llvd_deinit, NULL},
#endif
#ifdef CAMERA_FEATURE_SW_TNR
  {"sw_tnr", module_sw_tnr_init, module_sw_tnr_deinit, NULL},
#endif
#ifdef CAMERA_FEATURE_PP_DUMMY
  {"sample", module_sample_init, module_imgbase_deinit, NULL},
#endif
#ifdef CAMERA_FEATURE_CAC
  {"cac", module_cac_init, module_cac_deinit, NULL},
#else
#ifdef CAMERA_FEATURE_WNR_SW
  {"wnr", module_wnr_init, module_wnr_deinit, NULL},
#endif
#endif
#ifdef CAMERA_FEATURE_QUADRA_CFA
  {"quadracfa", module_quadracfa_init, module_imgbase_deinit, NULL},
#endif
#ifdef CAMERA_FEATURE_SVHDR
  {"bayer_proc_svhdr", module_svhdr_init, module_svhdr_deinit, NULL},
#endif
#ifdef CAMERA_FEATURE_SAT
  {"sat", module_qdc_common_init, module_qdc_common_deinit, NULL},
#endif
#ifdef CAMERA_FEATURE_BINCORR
  {"bayer_proc_bincorr", module_bincorr_init, module_bincorr_deinit, NULL},
#endif
#ifdef CAMERA_FEATURE_SAC
  {"sac", module_qdc_common_init, module_qdc_common_deinit, NULL},
#endif
#ifdef CAMERA_FEATURE_RTBDM
  {"rtbdm", module_qdc_common_init, module_qdc_common_deinit, NULL},
#endif
#ifdef CAMERA_FEATURE_RTB
  {"rtb", module_qdc_common_init, module_qdc_common_deinit, NULL},
#endif
#ifdef CAMERA_FEATURE_DEPTH_MAP
  {"depth_map", module_depth_map_init, module_depth_map_deinit, NULL},
#endif
#ifdef CAMERA_FEATURE_MFPROC_PRECPP
  {"mfproc_precpp", module_mfproc_precpp_init, module_mfproc_precpp_deinit, NULL},
#endif

};

/** CONVERT_GAMMA
 *    @type_in: type of input pointer
 *    @p_in: pointer to first entry of input gamma
 *    @input_entry_size: size of output gamma entry
 *    @input_bins: number of output bins
 *    @in_bits: number of input bits
 *    @p_output: pointer to first entry of output gamma
 *    @output_bins: number of output bins
 *
 *  Interpolates given gamma to specified output
 *
 *  Return: TRUE on success
 *          FALSE otherwise
 *
 */
#define CONVERT_GAMMA(type_in, p_in, input_entry_size, \
  input_bins, in_bits, p_output, output_bins) ({ \
  uint32_t i, j, k; \
  uint32_t val; \
  uint32_t delta = 0; \
  uint32_t bin_ratio; \
  uint32_t index; \
  int32_t shift_bits = in_bits - 8; \
  uint32_t mask; \
  boolean ret_val = TRUE; \
  type_in *p_orig = (type_in *)p_in; \
  \
  if ((0 != input_bins % output_bins) || \
    (0 != input_entry_size % 2) || \
    (0 == output_bins) || \
    (0 == input_bins) || \
    (0 == input_entry_size) || \
    (0 == input_bins / output_bins) || \
    (shift_bits < 0)) { \
    PP_ERR("Error"); \
    ret_val = FALSE; \
  } else { \
    bin_ratio = input_bins / output_bins; \
    mask = pow(2, in_bits) - 1; \
    PP_LOW("gamma bin_ratio %d %d %d", \
      bin_ratio, output_bins, input_entry_size); \
    \
    p_output[0] = ((p_orig[0] & mask) >> shift_bits); \
    for (i = 1; i < output_bins; i++) { \
      val = 0; \
      for (j = 0; j < bin_ratio; j++) { \
        val += p_orig[i * bin_ratio + j] & mask; \
      } \
      if(bin_ratio == 0) { \
        ret_val = FALSE; \
        break; \
      } \
      val = (val / bin_ratio) >> shift_bits; \
      delta = val - p_output[i-1]; \
      PP_LOW("gamma val[%d] %x %x %x", \
        i, val, delta, (type_in )p_orig[i * bin_ratio]); \
      p_output[i-1] |= (delta << 8); \
      p_output[i] = val; \
      PP_LOW("gamma_new val[%d] %x %x", \
        i, p_output[i], p_output[i-1]); \
    } \
  } \
  ret_val; \
})

/** _pproc_module_type:
 *    @identity:    identity (sessionid + streamid).
 *    @module_type: module type corresponding to this identity
 *
 * Store the module type per identity
 **/
typedef struct _pproc_module_type {
  uint32_t          identity;
  mct_module_type_t module_type;
} pproc_module_type_t;

/** _pproc_module_private:
 *    @sub_mods:         submodule array
 *    @num_sub_mods:     number of submodule
 *    @module_type_list: list to hold identity(stream) and
 *                       corresponding module type
 *
 * private object structure for pproc module
 **/
typedef struct _pproc_module_private {
  mct_module_t **sub_mods;
  uint32_t       num_sub_mods;
  mct_list_t    *module_type_list;
  int32_t       hal_version;
} pproc_module_private_t;

#define CALL_SUBMOD_FUNCTION(mod_private, function, args...) do { \
  uint32_t idx; \
  for (idx = 0; idx < mod_private->num_sub_mods; idx++) { \
    if (mod_private->sub_mods[idx] && \
      mod_private->sub_mods[idx]->function) { \
      mod_private->sub_mods[idx]->function(mod_private->sub_mods[idx], \
        ##args); \
    } \
  } \
} while(0)
volatile uint32_t gcam_pproc_loglevel = 0;

/** pproc_module_get_sub_mod
 *    @module: this pproc module object
 *    @name:   sub-module's name
 *
 *  To get a sub-module object from pproc module.
 *
 *  Return sub-module object if it is exists. Otherwise NULL.
 **/
mct_module_t* pproc_module_get_sub_mod(mct_module_t *module, const char *name)
{
  pproc_module_private_t *mod_private;
  uint32_t                i;

  PP_LOW("E\n");
  if (module == NULL) {
    PP_ERR("error module: %p\n", module);
    return NULL;
  }

  if (strcmp(MCT_OBJECT_NAME(module), "pproc")) {
    PP_ERR("error invalid module\n");
    return NULL;
  }

  mod_private = MCT_OBJECT_PRIVATE(module);

  for (i = 0; i < mod_private->num_sub_mods; i++) {
    if (mod_private->sub_mods[i]) {
      if (!strcmp(name, MCT_OBJECT_NAME(mod_private->sub_mods[i]))) {
        return mod_private->sub_mods[i];
      }
    }
  }
  PP_LOW("X\n");
  return NULL;
}

/** pproc_module_loglevel:
 *
 *  Args:
 *  Return:
 *    void
 **/
static void pproc_module_loglevel()
{
  char pproc_prop[PROPERTY_VALUE_MAX];
  memset(pproc_prop, 0, sizeof(pproc_prop));
  property_get("persist.vendor.camera.pproc.debug.mask", pproc_prop, "1");
  gcam_pproc_loglevel = atoi(pproc_prop);
}

/** pproc_module_free_port
 *    @data: port object to free
 *    @user_data: should be NULL
 *
 *  To free a sink or source port.
 *
 *  Return TRUE on success.
 **/
static boolean pproc_module_free_port(void *data, void *user_data __unused)
{
  mct_port_t *port = MCT_PORT_CAST(data);

  PP_LOW("E\n");
  if (!port) {
    PP_ERR("error because list data is null\n");
    return FALSE;
  }

  if (strncmp(MCT_OBJECT_NAME(port), "pproc_sink", strlen("pproc_sink")) &&
      strncmp(MCT_OBJECT_NAME(port), "pproc_source", strlen("pproc_source"))) {
    PP_ERR("error because port is invalid\n");
    return FALSE;
  }

  pproc_port_deinit(port);

  PP_LOW("X\n");
  return TRUE;
}

/** pproc_module_free_type_list
 *    @data: pproc_module_type_t object to free
 *    @user_data: should be NULL
 *
 *  To free module private's module_type_list element.
 *
 *  Return TRUE on success.
 **/
static boolean pproc_module_free_type_list(void *data, void *user_data __unused)
{
  pproc_module_type_t *type = (pproc_module_type_t *)data;

  PP_LOW("E\n");
  if (!type) {
    PP_ERR("error type: %p\n", type);
    return FALSE;
  }

  free(type);

  PP_LOW("X\n");
  return TRUE;
}

/** pproc_module_check_session
 *    @data1: pproc_module_type_t object from module private
 *    @data2: session id object
 *
 *  To check if session id has alreday been existing.
 *
 *  Return TRUE if the session id is existing.
 **/
static boolean pproc_module_check_session(void *data1, void *data2)
{
  pproc_module_type_t *type = (pproc_module_type_t *)data1;
  uint32_t            *sessionid = (uint32_t *)data2;

  return (((type->identity & 0xFFFF0000) >> 16) == *sessionid ?
    TRUE: FALSE);
}

/** pproc_module_check_identity
 *    @data1: pproc_module_type_t object from module private
 *    @data2: identity object
 *
 *  To check if identity has alreday been existing.
 *
 *  Return TRUE if the identity is existing.
 **/
static boolean pproc_module_check_identity(void *data1, void *data2)
{
  pproc_module_type_t *type = (pproc_module_type_t *)data1;
  uint32_t           *identity = (uint32_t *)data2;

  return ((type->identity == *identity) ?
    TRUE: FALSE);
}

/** pproc_module_reserve_compatible_port
 *    @data1: submods port
 *    @data2: stream attributes used to reserve this port;
 *
 *  To reserve port on module in stream.
 *
 *  Reserve status from submod
 **/
static boolean pproc_module_reserve_compatible_port(void *data1, void *data2)
{
  mct_port_t        *port = (mct_port_t *)data1;
  mct_stream_info_t *stream_info = (mct_stream_info_t *)data2;
  mct_port_caps_t    peer_caps;

  if (!port || !stream_info) {
    PP_ERR("error port: %p stream_info: %p\n", port, stream_info);
    return FALSE;
  }

  if (port->peer != NULL) {
    return FALSE;
  }

  return TRUE;
}

/** pproc_module_offline_streamon
 *    @module: pproc module
 *    @event: mct event to be handled
 *    @data: stream info
 *
 *  Handle stream on event for offline stream. Request pproc's
 *  own sink port that internally requests' sub module's sink
 *  port and link with pproc's sink port.
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_offline_streamon(mct_module_t *module,
  mct_event_t *event, void *data)
{
  boolean              rc = TRUE;
  mct_list_t          *lport = NULL;
  mct_port_t          *port = NULL;

  /* Validate input pameters */
  if (!module || strcmp(MCT_OBJECT_NAME(module), "pproc") || !event || !data) {
    PP_ERR("failed: data %p\n", data);
    rc = FALSE;
    goto ERROR;
  }

  /* Find pproc port for this identity */
  lport = mct_list_find_custom(MCT_MODULE_SINKPORTS(module), &event->identity,
    pproc_port_check_identity_in_port);

  if (!lport) {
    PP_ERR("failed: to caps reserve\n");
    rc = FALSE;
    goto ERROR;
  }

  /* Extract pproc port from mct list */
  port = (mct_port_t *)lport->data;
  if (!port) {
    PP_ERR("failed: reserved port NULL");
    rc = FALSE;
    goto ERROR;
  }

  /* Call stream on event on pproc port to get forward to sub module */
  rc = port->event_func(port, event);
  if (rc == FALSE) {
    PP_ERR("failed: to stream on sub module\n");
    rc = FALSE;
  }

ERROR:
  return rc;
}

/** pproc_module_send_output_dim_event
 *    @module: pproc module
 *
 *  Handle pproc_module_send_output_dim_event. Sends
 *  preview dimensions to the next module.
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_send_output_dim_event(mct_module_t *module,
  mct_port_t *port, uint32_t identity, cam_dimension_t *input_dim,
  cam_stream_buf_plane_info_t *buf_plane, cam_format_t input_fmt,
  uint32_t current_frame_id)
{
  boolean           rc = TRUE;
  mct_event_t       event;
  mct_stream_info_t stream_info;

  /* Validate input parameters */
  if (!module || !port || !input_dim) {
    PP_ERR("failed: module %p, port %p, input_dim %p", module, port, input_dim);
    rc = FALSE;
    goto ERROR;
  }

  /* Fill stream_info parameters */
  memset(&stream_info, 0, sizeof(mct_stream_info_t));
  stream_info.dim.width = input_dim->width;
  stream_info.dim.height = input_dim->height;
  memcpy(&stream_info.buf_planes,
    buf_plane, sizeof(stream_info.buf_planes));
  stream_info.fmt = input_fmt;

  /* Fill event parameters */
  event.identity  = identity;
  event.type      = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_OUTPUT_DIM;
  event.u.module_event.module_event_data = (void *)&stream_info;
  event.u.module_event.current_frame_id = current_frame_id;

  rc = port->event_func(port, &event);
  if (rc == FALSE) {
    PP_ERR("failed: send output dim event\n");
  }

ERROR:
  return rc;
}

/** pproc_module_extract_crop_params
 *    @module: pproc module
 *
 *  Handle pproc_module_extract_crop_params
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_extract_crop_params(metadata_buffer_t *metadata,
  mct_stream_info_t *input_stream_info, uint32_t identity,
  uint32_t input_stream_id, mct_bus_msg_stream_crop_t *stream_crop)
{
  boolean rc = FALSE;
  cam_crop_data_t *crop_data = NULL;
  uint8_t num_of_streams = 0, index = 0;
  crop_data = POINTER_OF_META(CAM_INTF_META_CROP_DATA, metadata);

  /* Validate input parameters */
  if (!metadata || !input_stream_info || !stream_crop) {
    PP_ERR("failed: metadata %p input stream %p stream_crop %p\n",
      metadata, input_stream_info, stream_crop);
    rc = FALSE;
    goto ERROR;
  }

  memset(stream_crop, 0, sizeof(mct_bus_msg_stream_crop_t));
  stream_crop->session_id = PPROC_GET_SESSION_ID(identity);
  stream_crop->stream_id = PPROC_GET_STREAM_ID(identity);
  stream_crop->crop_out_x = input_stream_info->dim.width;
  stream_crop->crop_out_y = input_stream_info->dim.height;

  /*
   * HAL requested to ignore crop for the dual camera scenario.
   *
   * Hence, intentionally return a failure, so the caller of this function will
   * skip sending the MCT_EVENT_MODULE_STREAM_CROP event to CPP module.
   */
  PP_DBG("metadata %p ignore_crop=%d\n", metadata, crop_data->ignore_crop);
  if (crop_data->ignore_crop) {
    rc = FALSE;
    goto ERROR;
  }

  num_of_streams = crop_data->num_of_streams;
  for (index = 0; index < num_of_streams; index++) {
    if (crop_data->crop_info[index].stream_id == input_stream_id) {
      break;
    }
  }

  if ((index < num_of_streams) &&
    (crop_data->crop_info[index].crop.width > 0) &&
    (crop_data->crop_info[index].crop.height > 0)) {
    /* Update stream crop with input crop parameters present in stream info */
    stream_crop->x = crop_data->crop_info[index].crop.left;
    stream_crop->y = crop_data->crop_info[index].crop.top;
    stream_crop->crop_out_x = crop_data->crop_info[index].crop.width;
    stream_crop->crop_out_y = crop_data->crop_info[index].crop.height;
    stream_crop->x_map = crop_data->crop_info[index].roi_map.left;
    stream_crop->y_map = crop_data->crop_info[index].roi_map.top;
    stream_crop->width_map = crop_data->crop_info[index].roi_map.width;
    stream_crop->height_map = crop_data->crop_info[index].roi_map.height;
    stream_crop->scale_ratio = crop_data->crop_info[index].scale_ratio;
    PP_LOW("Scale ratio %f", stream_crop->scale_ratio);

    if (num_of_streams < MAX_NUM_STREAMS) {
      crop_data->crop_info[num_of_streams].stream_id = identity & 0x0000FFFF;
      crop_data->crop_info[num_of_streams].crop.left = stream_crop->x;
      crop_data->crop_info[num_of_streams].crop.top = stream_crop->y;
      crop_data->crop_info[num_of_streams].crop.width = stream_crop->crop_out_x;
      crop_data->crop_info[num_of_streams].crop.height = stream_crop->crop_out_y;
      crop_data->crop_info[num_of_streams].roi_map.left = 0;
      crop_data->crop_info[num_of_streams].roi_map.top = 0;
      crop_data->crop_info[num_of_streams].roi_map.width = input_stream_info->dim.width;
      crop_data->crop_info[num_of_streams].roi_map.height = input_stream_info->dim.height;
      crop_data->num_of_streams += 1;
    }
    rc = TRUE;
  }
  PP_DBG("crop %d %d %d %d\n", stream_crop->x, stream_crop->y,
    stream_crop->crop_out_x, stream_crop->crop_out_y);

ERROR:
  return rc;
}


/** pproc_module_send_stream_crop_event
 *    @module: pproc module
 *
 *  Handle pproc_module_send_stream_crop_event
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_send_stream_crop_event(mct_port_t *port,
  uint32_t identity, mct_bus_msg_stream_crop_t *stream_crop)
{
  boolean     rc = TRUE;
  mct_event_t event;

  /* Validate input parameters */
  if (!port || !stream_crop) {
    PP_ERR("failed: port %p stream_crop %p\n", port, stream_crop);
    rc = FALSE;
    goto ERROR;
  }

  /* Fill event parameters */
  event.identity  = identity;
  event.type      = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_STREAM_CROP;
  event.u.module_event.module_event_data = (void *)stream_crop;
  event.u.module_event.current_frame_id = stream_crop->frame_id;

  rc = port->event_func(port, &event);
  if (rc == FALSE) {
    PP_ERR("failed: stream crop event\n");
  }

ERROR:
  return rc;
}

/** pproc_module_send_dc_shift_offset_event
 *    @module: pproc module
 *
 *  Inform CPP to handle the dual camera output shift offset event.
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_send_dc_shift_offset_event(
  mct_port_t *port,
  uint32_t identity,
  uint32_t current_frame_id,
  cam_sac_output_info_t *dualcam_shift_offsets)
{
  boolean rc = TRUE;
  mct_event_t event;

  /* Validate input parameters */
  if (!port || !dualcam_shift_offsets) {
    PP_ERR("failed: port %p dc_shift_offset %p\n", port, dualcam_shift_offsets);
    return FALSE;
  }

  /* Fill event parameters */
  event.identity  = identity;
  event.type      = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_DUALCAM_SHIFT_OFFSET;
  event.u.module_event.module_event_data = (void *)dualcam_shift_offsets;
  event.u.module_event.current_frame_id = current_frame_id;

  rc = port->event_func(port, &event);
  if (rc == FALSE) {
    PP_ERR("failed: stream dc_shift_offset event\n");
  }

  return rc;
}

/** pproc_module_send_dc_shift_margins_event
 *    @module: pproc module
 *
 *  Inform CPP to handle the dual camera output shift margins event.
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_send_dc_shift_margins_event(
  mct_port_t *port,
  uint32_t identity,
  uint32_t current_frame_id,
  cam_crop_data_t *dualcam_crop_data)
{
  boolean rc = TRUE;
  mct_event_t event;

  /* Validate input parameters */
  if (!port || !dualcam_crop_data) {
    PP_ERR("failed: port %p dc_shift_margins %p\n", port, dualcam_crop_data);
    return FALSE;
  }

  /* Fill event parameters */
  event.identity  = identity;
  event.type      = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_DUALCAM_SHIFT_MARGINS;
  event.u.module_event.module_event_data = (void *)dualcam_crop_data;
  event.u.module_event.current_frame_id = current_frame_id;

  rc = port->event_func(port, &event);
  if (rc == FALSE) {
    PP_ERR("failed: stream dc_shift_margins event\n");
  }

  return rc;
}

/** pproc_module_send_isp_notify_lpm_event
 *    @module: pproc module
 *
 *  Handle pproc_module_send_isp_notify_lpm_event
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_send_isp_notify_lpm_event(mct_port_t *port,
  uint32_t identity, mct_event_inform_lpm_t *inform_lpm, uint32_t current_frame_id)
{
  boolean     rc = TRUE;
  mct_event_t event;

  /* Validate input parameters */
  if (!port || !inform_lpm) {
    PP_ERR("failed: port %p inform_lpm %p\n", port, inform_lpm);
    rc = FALSE;
    goto ERROR;
  }

  /* Fill event parameters */
  event.identity  = identity;
  event.type      = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_INFORM_LPM;
  event.u.module_event.module_event_data = (void *)inform_lpm;
  event.u.module_event.current_frame_id = current_frame_id;

  rc = port->event_func(port, &event);
  if (rc == FALSE) {
    PP_ERR("failed: send_isp_notify_lpm_event\n");
  }

ERROR:
  return rc;
}

static boolean pproc_module_send_hdr_curves(mct_port_t *port, uint32_t identity,
  mct_hdr_data * hdr_data, uint32_t current_frame_id)
{

  boolean rc = TRUE;
  mct_event_t event;

  /* Validate input parameters */
  if (!port || !hdr_data) {
    PP_ERR("failed: port %p hdr_data %p\n", port, hdr_data);
    rc = FALSE;
    goto ERROR;
  }

  /* Fill event parameters */
  event.identity  = identity;
  event.type      = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_POST_HDR_CURVES;
  event.u.module_event.module_event_data = (void *)hdr_data;
  event.u.module_event.current_frame_id = current_frame_id;

  rc = port->event_func(port, &event);
  if (rc == FALSE) {
    PP_ERR("failed: send hdr curves failed\n");
  }

ERROR:
  return rc;
}

/** pproc_module_send_aec_update_event
 *    @module: pproc module
 *
 *  Handle pproc_module_send_aec_update_event
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_send_aec_update_event(mct_port_t *port,
  uint32_t identity, stats_update_t *stats_update, uint32_t current_frame_id)
{
  boolean     rc = TRUE;
  mct_event_t event;

  /* Validate input parameters */
  if (!port || !stats_update) {
    PP_ERR("failed: port %p stats_update %p\n", port, stats_update);
    rc = FALSE;
    goto ERROR;
  }

  /* Fill event parameters */
  event.identity  = identity;
  event.type      = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_STATS_AEC_UPDATE;
  event.u.module_event.module_event_data = (void *)stats_update;
  event.u.module_event.current_frame_id = current_frame_id;

  rc = port->event_func(port, &event);
  if (rc == FALSE) {
    PP_ERR("failed: send aec update event\n");
  }

ERROR:
  return rc;
}

/** pproc_module_send_gamma_update_event
 *    @module: pproc module
 *
 *  Handle pproc_module_send_gamma_update_event
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_send_gamma_update_event(mct_port_t *port,
  uint32_t identity, uint16_t *p_gamma, uint32_t current_frame_id)
{
  boolean     rc = TRUE;
  mct_event_t event;

  /* Validate input parameters */
  if (!port || !p_gamma) {
    PP_ERR("failed: port %p p_gamma %p\n", port, p_gamma);
    rc = FALSE;
    goto ERROR;
  }

  /* Fill event parameters */
  event.identity  = identity;
  event.type      = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_GAMMA_UPDATE;
  event.u.module_event.module_event_data = (void *)p_gamma;
  event.u.module_event.current_frame_id = current_frame_id;

  rc = port->event_func(port, &event);
  if (rc == FALSE) {
    PP_ERR("failed: send gamma update event\n");
  }

ERROR:
  return rc;
}

/** pproc_module_send_awb_update_event
 *    @module: pproc module
 *
 *  Handle pproc_module_send_awb_update_event
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_send_awb_update_event(mct_port_t *port,
  uint32_t identity, awb_update_t *awb_update, uint32_t current_frame_id)
{
  boolean     rc = TRUE;
  mct_event_t event;

  /* Validate input parameters */
  if (!port || !awb_update) {
    PP_ERR("failed: port %p awb_update %p\n", port, awb_update);
    rc = FALSE;
    goto ERROR;
  }

  /* Fill event parameters */
  event.identity  = identity;
  event.type      = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_AWB_UPDATE;
  event.u.module_event.module_event_data = (void *)awb_update;
  event.u.module_event.current_frame_id = current_frame_id;

  rc = port->event_func(port, &event);
  if (rc == FALSE) {
    PP_ERR("failed: send awb update event\n");
  }

ERROR:
  return rc;
}

/** pproc_module_send_set_meta_event
 *    @module: pproc module
 *    @identity: identity
 *    @meta_data: meta data
 *
 *  Handle pproc_module_send_set_meta_event
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_send_set_meta_event(mct_port_t *port,
  uint32_t identity, void *meta_data)
{
  boolean     rc = TRUE;
  mct_event_t event;

  /* Validate input parameters */
  if (!port || !meta_data) {
    PP_ERR("failed: port %p meta_data %p\n", port, meta_data);
    rc = FALSE;
    goto ERROR;
  }

  /* Fill event parameters */
  event.identity  = identity;
  event.type      = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_SET_META_PTR;
  event.u.module_event.module_event_data = meta_data;

  rc = port->event_func(port, &event);
  if (rc == FALSE) {
    PP_ERR("failed: send set meta data ptr event\n");
  }

ERROR:
  return rc;
}

/** pproc_module_send_set_chromatix_event
 *    @module: pproc module
 *
 *  Handle pproc_module_send_set_chromatix_event
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_send_set_chromatix_event(mct_port_t *port,
  uint32_t identity, modulesChromatix_t *module_chromatix)
{
  boolean     rc = TRUE;
  mct_event_t event;

  /* Validate input parameters */
  if (!port || !module_chromatix) {
    PP_ERR("failed: port %p chromatix %p\n", port, module_chromatix);
    rc = FALSE;
    goto ERROR;
  }

  /* Fill event parameters */
  event.identity  = identity;
  event.type      = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_SET_CHROMATIX_PTR;
  event.u.module_event.module_event_data = (void *)module_chromatix;

  rc = port->event_func(port, &event);
  if (rc == FALSE) {
    PP_ERR("failed: send set chromatix event\n");
  }

ERROR:
  return rc;
}

/** pproc_module_send_fd_update_event
 *    @port: mct port
 *    @identity: stream identity
 *    @faces_data: fd info
 *    @gazes_data: gaze info
 *  Handle pproc_module_send_fd_update_event. Sends the FD
 *  info to next module
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_send_fd_update_event(mct_port_t *port,
  unsigned int identity, cam_face_detection_data_t *faces_data,
  cam_face_gaze_data_t *gazes_data)
{
  boolean     rc = TRUE;
  mct_event_t event;
  cam_faces_data_t comb_faces_data;

  /* Validate input parameters */
  if (!port || !faces_data || !gazes_data) {
    PP_ERR(" failed: port %p faces_data %p gazes_data %p\n",
      port, faces_data, gazes_data);
    rc = FALSE;
    goto ERROR;
  }

  memcpy(&(comb_faces_data.detection_data), faces_data,
    sizeof(comb_faces_data.detection_data));
  memcpy(&(comb_faces_data.gaze_data), gazes_data,
    sizeof(comb_faces_data.gaze_data));

  /* Fill event parameters */
  event.identity  = identity;
  event.type      = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_EXTENDED_FACE_INFO;
  event.u.module_event.module_event_data = (void *)(&comb_faces_data);

  rc = port->event_func(port, &event);
  if (rc == FALSE) {
    PP_ERR("failed: send fd update event\n");
  }

ERROR:
  return rc;
}

/** pproc_module_send_buf_divert_event
 *    @module: pproc module
 *
 *  Handle pproc_module_send_buf_divert_event
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_send_buf_divert_event(mct_module_t *module,
  mct_port_t *port, uint32_t identity, mct_stream_map_buf_t *buf_holder,
  cam_stream_parm_buffer_t *parm_buf, uint32_t buf_identity,
  metadata_buffer_t *mct_meta_data)
{
  boolean           rc = TRUE;
  uint32_t          i;
  isp_buf_divert_t  isp_buf;
  struct v4l2_plane plane[VIDEO_MAX_PLANES];
  mct_event_t       event;
  mct_stream_session_metadata_info *priv_metadata;
  pproc_module_private_t *pproc_mod_private;
  uint8_t           need_meta;

  /* Validate input parameters */
  if (!module || !port || !buf_holder || !parm_buf || !mct_meta_data) {
    PP_ERR("failed: module %p port %p buf_holder %p", module, port, buf_holder);
    rc = FALSE;
    goto ERROR;
  }

  //get pproc mod private
  pproc_mod_private =
   (pproc_module_private_t *)MCT_OBJECT_PRIVATE(module);
  if (NULL == pproc_mod_private) {
    PP_ERR("pproc_mod_private:%p\n", pproc_mod_private);
    return FALSE;
  }

  memset(&isp_buf, 0, sizeof(isp_buf_divert_t));
  memset(plane, 0, sizeof(plane));
  priv_metadata =
      (mct_stream_session_metadata_info *)POINTER_OF_META(
        CAM_INTF_META_PRIVATE_DATA, mct_meta_data);
  if (priv_metadata == NULL) {
    PP_ERR("No private metadata!");
    rc = FALSE;
  } else {
    if (pproc_mod_private->hal_version == CAM_HAL_V3) {
      isp_buf.is_uv_subsampled =
        priv_metadata->iface_metadata.is_uv_subsampled;
    } else {
      isp_buf.is_uv_subsampled = parm_buf->reprocess.is_uv_subsampled;
    }
  }
  isp_buf.identity = buf_identity;
#ifdef CACHE_PHASE2
  isp_buf.buffer_access = parm_buf->reprocess.flags;
#endif

#ifdef PPROC_OFFLINE_USE_V4L2
  /* TODO use v4l2 buffer later */
  isp_buf.native_buf = FALSE;
  isp_buf.buffer.sequence = parm_buf->reprocess.frame_idx;
  isp_buf.buffer.m.planes = plane;

  for (i = 0; i < buf_holder->num_planes; i++) {
    isp_buf.buffer.m.planes[i].m.userptr = buf_holder->buf_planes[i].fd;
    isp_buf.buffer.m.planes[i].data_offset = buf_holder->buf_planes[i].offset;
    isp_buf.buffer.m.planes[i].length = buf_holder->buf_planes[i].size;
  }

  isp_buf.buffer.length = buf_holder->num_planes;
  isp_buf.buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  isp_buf.buffer.index = buf_holder->buf_index;
  isp_buf.buffer.memory = V4L2_MEMORY_USERPTR;

  /* Fill timestamp */
  gettimeofday(&isp_buf.buffer.timestamp, NULL);

  /* Fill event parameters */
  event.identity  = identity;
  event.type      = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_STREAM_CROP;
  event.u.module_event.module_event_data = (void *)&isp_buf;
  event.u.module_event.current_frame_id = parm_buf->reprocess.frame_idx;
#else
  /* Use native buffer for now */
  isp_buf.native_buf = TRUE;
  isp_buf.fd = buf_holder->buf_planes[0].fd;
  isp_buf.vaddr = buf_holder->buf_planes[0].buf;
  PP_LOW("isp_buf.vaddr %p, divert prt %p", isp_buf.vaddr, &isp_buf);
  for (i = 0; i < buf_holder->num_planes; i++) {
    isp_buf.plane_vaddr[i]= (uint32_t)buf_holder->buf_planes[i].buf;
    PP_LOW("isp_buf plane %d, p_buf.plane_vaddr %p", i,
      (void *)isp_buf.plane_vaddr[i]);
  }

  isp_buf.buffer.sequence = parm_buf->reprocess.frame_idx;

  isp_buf.buffer.length = buf_holder->num_planes;
  isp_buf.buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
  isp_buf.buffer.index = buf_holder->buf_index;
  isp_buf.buffer.memory = V4L2_MEMORY_USERPTR;

  /* Fill timestamp */
  gettimeofday(&isp_buf.buffer.timestamp, NULL);

  /* Fill event parameters */
  event.identity  = identity;
  event.type      = MCT_EVENT_MODULE_EVENT;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_BUF_DIVERT;
  event.u.module_event.module_event_data = (void *)&isp_buf;
#endif

  need_meta = pproc_port_check_meta_data_dump(CAM_STREAM_TYPE_OFFLINE_PROC);
  if (!isp_buf.meta_data) {
    meta_data_container *md_container = NULL;
    md_container =
      (meta_data_container *)CAM_MALLOC(sizeof(meta_data_container));
    if (!md_container) {
      PP_ERR("alloc failed md_container\n");
    } else {
      memset(md_container, 0, sizeof(meta_data_container));
      if (need_meta) {
          md_container->pproc_meta_data =
            (pproc_meta_data_t *)CAM_MALLOC(sizeof(pproc_meta_data_t));
          if (!md_container->pproc_meta_data) {
            PP_ERR("alloc failed pproc_meta_data\n");
            free(md_container);
            rc = FALSE;
            goto ERROR;
          } else {
            memset(md_container->pproc_meta_data, 0, sizeof(pproc_meta_data_t));
          }
        }
        isp_buf.meta_data = md_container;
        md_container->mct_meta_data = mct_meta_data;
    }
  }
  rc = port->event_func(port, &event);
  if (rc == FALSE) {
    PP_ERR("failed: send output dim event\n");
  }

ERROR:
  return rc;
}

static boolean pproc_module_find_stream_by_streamid(void *data1, void *data2)
{
  mct_stream_t *stream = (void *)data1;
  uint32_t *input_stream_id = (uint32_t *)data2;
  if (!stream) {
    PP_ERR("failed stream %p", stream);
    return FALSE;
  }
  if (!input_stream_id) {
    PP_ERR("failed input_stream_id %p", input_stream_id);
    return FALSE;
  }
  if ((stream->streaminfo.identity & 0xFFFF) == *input_stream_id) {
    return TRUE;
  }
  return FALSE;
}

static boolean pproc_module_find_stream_by_sessionid(void *data1, void *data2)
{
  mct_stream_t *stream = (void *)data1;
  uint32_t *input_session_id = (uint32_t *)data2;

  if (!stream) {
    PP_ERR("failed stream %p", stream);
    return FALSE;
  }
  if (!input_session_id) {
    PP_ERR("failed input_session_id %p", input_session_id);
    return FALSE;
  }
  if (((stream->streaminfo.identity & 0xFFFF0000) >> 16) == *input_session_id) {
    return TRUE;
  }
  return FALSE;
}

static boolean pproc_module_find_online_input_buffer(void *data1, void *data2)
{
  mct_stream_map_buf_t *buf_holder = (mct_stream_map_buf_t *)data1;
  uint32_t *buf_index = (uint32_t *)data2;
  if (!buf_holder || !buf_index) {
    PP_ERR("failed buf_holder %p buf_index %p\n", buf_holder, buf_index);
    return FALSE;
  }
  if ((buf_holder->buf_index == *buf_index) &&
      (buf_holder->buf_type == CAM_MAPPING_BUF_TYPE_STREAM_BUF)) {
    return TRUE;
  }
  return FALSE;
}

static void *pproc_module_get_online_input_buffer(mct_module_t *module,
  mct_stream_info_t *stream_info, cam_stream_parm_buffer_t *parm_buf,
  uint32_t identity)
{
  uint32_t session_id = identity >> 16;
  uint32_t stream_id = identity & 0xFFFF;
  mct_pipeline_t *pipeline = NULL;
  mct_stream_t *stream = NULL;
  mct_list_t *stream_list = NULL, *buf_list = NULL;
  /* Validate input params */
  if (!module || !stream_info) {
    PP_ERR("failed module %p stream_info %p\n", module, stream_info);
    return NULL;
  }
  /* Get pproc module's parent - stream */
  stream_list = mct_list_find_custom(MCT_MODULE_PARENT(module), &session_id,
    pproc_module_find_stream_by_sessionid);
  if (!stream_list) {
    PP_ERR("failed stream_list %p\n", stream_list);
    return NULL;
  }
  if (!stream_list->data) {
    PP_ERR("failed stream_list->data %p", stream_list->data);
    return FALSE;
  }
  stream = (mct_stream_t *)stream_list->data;
  /* Get stream's parent - pipeline */
  pipeline = MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data);
  if (!pipeline) {
    PP_ERR("failed pipeline %p\n", pipeline);
    return NULL;
  }
  stream_list = mct_list_find_custom(MCT_PIPELINE_CHILDREN(pipeline),
    &stream_info->reprocess_config.online.input_stream_id,
    pproc_module_find_stream_by_streamid);
  if (!stream_list) {
    PP_ERR("failed stream_list %p", stream_list);
    return FALSE;
  }
  if (!stream_list->data) {
    PP_ERR("failed stream_list->data %p", stream_list->data);
    return FALSE;
  }
  stream = (mct_stream_t *)stream_list->data;

  if (!stream->streaminfo.img_buffer_list) {
    PP_ERR("failed img_buffer_list = NULL");
    return FALSE;
  }

  buf_list = mct_list_find_custom(stream->streaminfo.img_buffer_list,
    &parm_buf->reprocess.buf_index, pproc_module_find_online_input_buffer);
  if (!buf_list) {
    PP_ERR("failed buf_list %p", buf_list);
    return FALSE;
  }
  if (!buf_list->data) {
    PP_ERR("failed buf_list->data %p", buf_list->data);
    return FALSE;
  }
  return buf_list->data;
}

static boolean pproc_module_find_offline_input_buffer(void *data1, void *data2)
{
  mct_stream_map_buf_t *buf_holder = (mct_stream_map_buf_t *)data1;
  uint32_t *buf_index = (uint32_t *)data2;
  if (!buf_holder || !buf_index) {
    PP_ERR("failed buf_holder %p buf_index %p\n", buf_holder, buf_index);
    return FALSE;
  }
  if ((buf_holder->buf_index == *buf_index) &&
      (buf_holder->buf_type == CAM_MAPPING_BUF_TYPE_OFFLINE_INPUT_BUF)) {
    return TRUE;
  }
  return FALSE;
}

static void *pproc_module_get_offline_input_buffer(mct_module_t *module,
  cam_stream_parm_buffer_t *parm_buf, uint32_t identity)
{
  uint32_t session_id = identity >> 16;
  uint32_t stream_id = identity & 0xFFFF;
  mct_pipeline_t *pipeline = NULL;
  mct_stream_t *stream = NULL;
  mct_list_t *stream_list = NULL, *buf_list = NULL;
  /* Validate input params */
  if (!module) {
    PP_ERR("failed module %p\n", module);
    return NULL;
  }
  /* Get pproc module's parent - stream */
  stream_list = mct_list_find_custom(MCT_MODULE_PARENT(module), &session_id,
    pproc_module_find_stream_by_sessionid);
  if (!stream_list) {
    PP_ERR("failed stream_list %p\n", stream_list);
    return NULL;
  }
  if (!stream_list->data) {
    PP_ERR("failed stream_list->data %p", stream_list->data);
    return FALSE;
  }
  stream = (mct_stream_t *)stream_list->data;
  /* Get stream's parent - pipeline */
  pipeline = MCT_PIPELINE_CAST((MCT_STREAM_PARENT(stream))->data);
  if (!pipeline) {
    PP_ERR("failed pipeline %p\n", pipeline);
    return NULL;
  }
  stream_list = mct_list_find_custom(MCT_PIPELINE_CHILDREN(pipeline),
    &stream_id, pproc_module_find_stream_by_streamid);
  if (!stream_list) {
    PP_ERR("failed stream_list %p", stream_list);
    return FALSE;
  }
  if (!stream_list->data) {
    PP_ERR("failed stream_list->data %p", stream_list->data);
    return FALSE;
  }
  stream = (mct_stream_t *)stream_list->data;
  buf_list = mct_list_find_custom(stream->streaminfo.img_buffer_list,
    &parm_buf->reprocess.buf_index, pproc_module_find_offline_input_buffer);
  if (!buf_list) {
    PP_ERR("failed buf_list %p", buf_list);
    return FALSE;
  }
  if (!buf_list->data) {
    PP_ERR("failed buf_list->data %p", buf_list->data);
    return FALSE;
  }
  return buf_list->data;
}

/** pproc_module_send_set_param_event
 *    @module: pproc module
 *
 *  Handle pproc_module_send_set_param_event
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_send_set_param_event(mct_port_t *port,
  uint32_t identity, pproc_module_pp_config_t *frame_pp_config,
  cam_intf_parm_type_t type, uint32_t current_frame_id)
{
  boolean                  rc = TRUE;
  mct_event_t              event;
  mct_event_control_parm_t event_parm;
  cam_stream_parm_buffer_t stream_param;
  cam_rotation_info_t rotation_info;

  memset(&event, 0, sizeof(mct_event_t));
  memset(&event_parm, 0, sizeof(mct_event_control_parm_t));

  /* Validate input parameters */
  if (!port || !frame_pp_config) {
    PP_ERR("failed: port %p frame_pp_config %p\n", port, frame_pp_config);
    return FALSE;
  }

  /* Fill event parameters */
  event.identity  = identity;
  event.type      = MCT_EVENT_CONTROL_CMD;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.timestamp = 0;
  event.u.ctrl_event.type = MCT_EVENT_CONTROL_SET_PARM;
  event.u.ctrl_event.control_event_data = &event_parm;
  event.u.ctrl_event.current_frame_id = current_frame_id;
  event_parm.type = type;
  switch (type) {
  case CAM_INTF_PARM_WAVELET_DENOISE: {
    event_parm.parm_data = (void *)&frame_pp_config->denoise2d;
    break;
  }
  case CAM_INTF_PARM_SHARPNESS: {
    event_parm.parm_data = (void *)&frame_pp_config->sharpness;
    break;
  }
  case CAM_INTF_PARM_ROTATION: {
    rotation_info.rotation = frame_pp_config->rotation;
    rotation_info.device_rotation = frame_pp_config->device_rotation;
    rotation_info.streamId = PPROC_GET_STREAM_ID(identity);
    PP_HIGH("rotation:%d, strmId:%d\n", rotation_info.rotation,
      rotation_info.streamId);
    event_parm.parm_data = (void *)&rotation_info;
    break;
  }
  case CAM_INTF_PARM_EFFECT: {
    event_parm.parm_data = (void *)&frame_pp_config->effect;
    break;
  }
  case CAM_INTF_PARM_BESTSHOT_MODE: {
    event_parm.parm_data = (void *)&frame_pp_config->scene_mode;
    break;
  }
  case CAM_STREAM_PARAM_TYPE_SET_FLIP: {
    memset(&stream_param, 0, sizeof(cam_stream_parm_buffer_t));
    /*
     * Enumeration in the cam_stream_param_type_e picks the values
     * from cam_intf_parm_type_t
     */
    stream_param.type = (cam_stream_param_type_e)type;
    stream_param.flipInfo.flip_mask = frame_pp_config->flip;
    event.u.ctrl_event.type = MCT_EVENT_CONTROL_PARM_STREAM_BUF;
    event.u.ctrl_event.control_event_data = &stream_param;
    break;
  }
  case CAM_INTF_PARM_TEMPORAL_DENOISE: {
    event_parm.parm_data = (void *)&frame_pp_config->tnr;
    break;
  }
  case CAM_INTF_PARM_CDS_MODE: {
    event_parm.parm_data = (void *)&frame_pp_config->dsdn;
    break;
  }
  case CAM_INTF_META_IMG_DYN_FEAT:{
    event_parm.parm_data = (void *)&frame_pp_config->dyn_img_data;
    break;
  }
  case CAM_INTF_PARM_SENSOR_HDR: {
    event_parm.parm_data = (void *)&frame_pp_config->hdr_mode;
    break;
  }
  default:
    PP_ERR("error invalid type:%d\n", type);
    rc = FALSE;
    break;
  }

  if (rc == TRUE) {
     rc = port->event_func(port, &event);
     if (rc == FALSE) {
       PP_ERR("failed: send denoise setparam event\n");
     }
  }

  return rc;
}

/** pproc_module_send_parm_stream_buf_event
 *    @port: pproc port
 *    @identity: identity
 *    @stream_info: stream info
 *
 *  Handle pproc_module_send_parm_stream_buf_event
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_send_parm_stream_buf_event(mct_port_t *port,
  uint32_t identity, mct_stream_info_t *stream_info)
{
  boolean                  rc = TRUE;
  mct_event_t              cmd_event;
  mct_event_control_t      event_data;

  event_data.type = MCT_EVENT_CONTROL_PARM_STREAM_BUF;
  event_data.control_event_data = (void *)&stream_info->parm_buf;

  cmd_event.type = MCT_EVENT_CONTROL_CMD;
  cmd_event.identity = identity;
  cmd_event.direction = MCT_EVENT_DOWNSTREAM;
  cmd_event.timestamp = 0;
  cmd_event.u.ctrl_event = event_data;

   rc = port->event_func(port, &cmd_event);
   if (rc == FALSE) {
     PP_ERR("failed: send stream buf event\n");
   }

  return rc;
}

/** pproc_module_interpolate_gamma
 *    @p_out: pointer to first entry of output gamma
 *    @p_uin: pointer to first entry of oinput gamma
 *    @output_entry_size: size of output gamma entry
 *    @output_bins: number of output bins
 *    @input_entry_size: size of output gamma entry
 *    @input_bins: number of output bins
 *
 *  Interpolates given gamma to specified output
 *
 *  Return: TRUE on success
 *          FALSE otherwise
 *
 */
static boolean pproc_module_interpolate_gamma(uint8_t *p_out,  uint8_t *p_in,
  uint32_t output_entry_size, uint32_t output_bins, uint32_t input_entry_size,
  uint32_t input_bins)
{
  uint32_t i, j, k;
  uint32_t val;
  uint32_t old_val;
  uint32_t delta;
  uint32_t bin_ratio;
  uint32_t index;

  if ((0 != input_bins % output_bins) ||
      (0 != output_entry_size % 2) ||
      (0 != input_entry_size % 2) ||
      (0 == output_bins) ||
      (0 == input_bins) ||
      (0 == output_entry_size) ||
      (0 == input_entry_size) ||
      (0 == input_bins / output_bins)) {
    return FALSE;
  }

  bin_ratio = input_bins / output_bins;
  if(bin_ratio == 0) {
    return FALSE;
  }
  old_val = 0;

  for (i=0; i<output_bins; i++) {
    val = 0;
    for (j=0; j<bin_ratio; j++) {
      for (k=0; k<input_entry_size / 2; k++) {
        index = i * bin_ratio * input_entry_size + j * input_entry_size + k;
        val += ((uint16_t)(p_in[index]) << (8 * k));
      }
    }

    val /= bin_ratio;
    delta = val - old_val;
    old_val = val;

    for (k=0; k<output_entry_size / 2; k++) {
      index = i * output_entry_size + k;
      p_out[index] = val & 0xff;
      val >>= 8;

      index = i * output_entry_size + output_entry_size / 2 + k;
      p_out[index] = delta & 0xff;
      delta >>= 8;
    }
  }

  return TRUE;
}

/** pproc_module_extract_gamma_from_metadata
 *    @gamma_update: pointer to first entry of output gamma
 *    @output_entry_size: size of output gamma entry
 *    @output_bins: number of output bins
 *    @isp_gama: metadata gamma data
 *
 *  Extracts gamma from metadata
 *
 *  Return: TRUE on success
 *          FALSE otherwise
 *
 */
static boolean pproc_module_extract_gamma_from_metadata(uint16_t *gamma_update,
  uint32_t output_entry_size, uint32_t output_bins,
  mct_bus_msg_isp_gamma_t *isp_gama)
{
  boolean ret_val = FALSE;
  uint32_t output_size = output_entry_size * output_bins;
  uint32_t gamma_offset;

  switch (isp_gama->num_tables) {
  case 1:
    gamma_offset = 0;
    break;
  case 3:
    // Green gamma is used, which is second table
    gamma_offset = isp_gama->entry_size * isp_gama->num_entries;
    break;
  default:
    return ret_val;
    break;
  }

  switch (isp_gama->isp_version) {
  case ISP_VERSION_32:
  case ISP_VERSION_40:
    if (isp_gama->entry_size * isp_gama->num_entries == output_size) {
      memcpy(gamma_update, &isp_gama->private_data[gamma_offset], output_size);
      ret_val = TRUE;
    }
    break;
  case ISP_VERSION_44:
  case ISP_VERSION_47:
  case ISP_VERSION_48:
  case ISP_VERSION_46: {
    switch (isp_gama->entry_size) {
    case 4:
      ret_val = CONVERT_GAMMA(uint32_t ,
        (uint8_t*)&isp_gama->private_data[gamma_offset],
        isp_gama->entry_size,
        isp_gama->num_entries,
        isp_gama->pack_bits,
        gamma_update, output_bins);
      break;
    default:
      ret_val = CONVERT_GAMMA(uint16_t ,
        (uint8_t*)&isp_gama->private_data[gamma_offset],
        isp_gama->entry_size,
        isp_gama->num_entries,
        isp_gama->pack_bits,
        gamma_update, output_bins);
    }
    break;
  }
  default:
    break;
  }

  return ret_val;
}

/** pproc_module_send_face_info_event
 *    @module: pproc module
 *
 *  Send face info event down stream
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_send_face_info_event(mct_port_t *port,
  unsigned int identity, cam_face_detection_data_t *faces_data, int frame_idx)
{

  int rc = 0;
  mct_event_t                       face_info_event;
  mct_face_info_t                   face_info;

#ifndef FACE_CPP
  memset(&face_info, 0, sizeof(face_info));
  memset(&face_info_event, 0, sizeof(face_info_event));
  if (faces_data->num_faces_detected) {
    face_info.face_count = faces_data->num_faces_detected;
  }
  face_info.frame_id = frame_idx;
  face_info_event.u.module_event.type = MCT_EVENT_MODULE_FACE_INFO;
  face_info_event.u.module_event.module_event_data = (void *)&face_info;
  face_info_event.type = MCT_EVENT_MODULE_EVENT;
  face_info_event.identity = identity;
  face_info_event.direction = MCT_EVENT_DOWNSTREAM;
  rc = port->event_func(port, &face_info_event);
  if (rc == FALSE) {
    PP_ERR("failed: face info event\n");
  }
#endif
  return rc;
}

static int pproc_module_update_raw_processing_chromatix(
  cam_pp_feature_config_t *pp_feature_config,
  modulesChromatix_t *module_chromatix,
  mct_stream_session_metadata_info *priv_metadata)
{

  if (!pp_feature_config || !module_chromatix || !priv_metadata) {
    PP_ERR("pp_feature_config %p,  mod_chromatix %p, priv_meta %p invalid",
      pp_feature_config, module_chromatix, priv_metadata);
    return -EINVAL;
  }

#ifdef FEATURE_RAW_PROCESSING
  if (pp_feature_config->feature_mask & CAM_QCOM_FEATURE_RAW_PROCESSING) {
    if (priv_metadata->sensor_data.offline_cpp_chromatix_ptr)
      module_chromatix->chromatixSnapCppPtr =
      priv_metadata->sensor_data.offline_cpp_chromatix_ptr;
    if (priv_metadata->sensor_data.offline_us_cpp_chromatix_ptr)
      module_chromatix->chromatixUsCppPtr =
      priv_metadata->sensor_data.offline_us_cpp_chromatix_ptr;
    if (priv_metadata->sensor_data.offline_ds_cpp_chromatix_ptr)
      module_chromatix->chromatixDsCppPtr =
      priv_metadata->sensor_data.offline_ds_cpp_chromatix_ptr;
    if (priv_metadata->sensor_data.offline_postproc_chromatix_ptr)
      module_chromatix->chromatixPostProcPtr =
      priv_metadata->sensor_data.offline_postproc_chromatix_ptr;
  }
#endif

  return 0;
}

/** pproc_module_handle_reprocess_online
 *    @module: pproc module
 *
 *  Handle pproc_module_handle_reprocess_online
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_handle_reprocess_online(mct_module_t *module,
  mct_port_t *port, mct_stream_info_t *stream_info,
  cam_stream_parm_buffer_t *parm_buf, uint32_t identity)
{
  boolean                           rc = TRUE;
  uint32_t                          session_id = 0;
  uint32_t                          stream_id = 0;
  metadata_buffer_t                *metadata = NULL;
  mct_stream_info_t                *input_stream_info = NULL;
  mct_bus_msg_stream_crop_t         stream_crop;
  mct_event_inform_lpm_t            inform_lpm;
  mct_stream_map_buf_t             *buf_holder = NULL;
  cam_pp_feature_config_t          *pp_feature_config;
  modulesChromatix_t                module_chromatix;
  mct_stream_session_metadata_info *priv_metadata;
  stats_get_data_t                 *stats_get = NULL;
  stats_update_t                    stats_update;
  aec_update_t                     *aec_update = NULL;
  awb_update_t                      awb_update;
  uint16_t                          gamma_update[64];
  void                              *meta_crop_data = NULL;
  pproc_module_pp_config_t          frame_pp_config;
  mct_hdr_data *hdr_data = NULL;

  /* Validate input parameters */
  if (!module || !port || !stream_info || !parm_buf) {
    PP_ERR("failed: module %p port %p stream_info %p parm_buf %p\n",
      module, port, stream_info, parm_buf);
    rc = FALSE;
    goto ERROR;
  }

  session_id = PPROC_GET_SESSION_ID(identity);
  stream_id = PPROC_GET_STREAM_ID(identity);
  mct_module_type_t mod_type = mct_module_find_type(module,
    identity);

  /* Get input stream info */
  input_stream_info =
    (mct_stream_info_t *)mct_module_get_stream_info(module, session_id,
    stream_info->reprocess_config.online.input_stream_id);
  if (!input_stream_info) {
    PP_ERR("stream_info NULL\n");
    rc = FALSE;
    goto ERROR;
  }

  /* Validate number of planes */
  if (input_stream_info->buf_planes.plane_info.num_planes == 0) {
    PP_ERR("input stream info num planes 0\n");
    rc = FALSE;
    goto ERROR;
  }

  //stream_info->isp_fmt = stream_info->fmt;

  if (mod_type == MCT_MODULE_FLAG_SOURCE ||
    mod_type == MCT_MODULE_FLAG_PEERLESS) {
    /* Pass output dim event */
    rc = pproc_module_send_output_dim_event(module, port, identity,
      &input_stream_info->dim,
      &input_stream_info->buf_planes,
      input_stream_info->fmt, parm_buf->reprocess.frame_idx);
  }

  /* TODO: when HAL implements single place holder for all the pp_config
     we will switch to using this. For now use the other pp_feature_config
     pp_feature_config = &input_stream_info->pp_config; */
  pp_feature_config = &stream_info->reprocess_config.pp_feature_config;

  frame_pp_config.denoise2d.denoise_enable = false;
  frame_pp_config.sharpness = 0;
  frame_pp_config.effect = CAM_EFFECT_MODE_OFF;
  frame_pp_config.rotation = ROTATE_0;
  frame_pp_config.flip = FLIP_NONE;
  frame_pp_config.scene_mode = CAM_SCENE_MODE_OFF;
  frame_pp_config.tnr.denoise_enable = false;
  frame_pp_config.dsdn = false;
  frame_pp_config.dyn_img_data.dyn_feature_mask = 0;

  /* check for CROP in feature mask */
  /* TODO: enable this only when HAL enables it */
  //if (pp_feature_config->feature_mask & CAM_QCOM_FEATURE_CROP) {
    /* Check if metadata is present */
    if (parm_buf->reprocess.meta_present == 1) {
      /* Extract meta buffer */
      metadata = (metadata_buffer_t *)mct_module_get_buffer_ptr(
        parm_buf->reprocess.meta_buf_index, module, session_id,
        parm_buf->reprocess.meta_stream_handle);
      if (!metadata) {
        PP_ERR("failed: metadata NULL\n");
        rc = FALSE;
        goto ERROR;
      }

      rc = pproc_module_send_set_meta_event(port, identity, metadata);
      if (rc == FALSE) {
        PP_ERR("failed: send set meta data ptr event\n");
        rc = FALSE;
        goto ERROR;
      }

      if (mod_type == MCT_MODULE_FLAG_SOURCE ||
        mod_type == MCT_MODULE_FLAG_PEERLESS) {
        /* Extract crop params from meta data */
        rc = pproc_module_extract_crop_params(metadata, input_stream_info,
          identity, stream_info->reprocess_config.online.input_stream_id,
          &stream_crop);
        if (rc == TRUE) {
          stream_crop.frame_id = parm_buf->reprocess.frame_idx;
          rc = pproc_module_send_stream_crop_event(port, identity, &stream_crop);
          if (rc == FALSE) {
            PP_ERR("failed: send stream crop event\n");
            rc = FALSE;
            goto ERROR;
          }
        }
      }

      /* Extract chromatix params from meta data */
      memset(&module_chromatix, 0, sizeof(modulesChromatix_t));
      priv_metadata =
          (mct_stream_session_metadata_info *)POINTER_OF_META(CAM_INTF_META_PRIVATE_DATA, metadata);
      module_chromatix.chromatixComPtr =
        priv_metadata->sensor_data.common_chromatix_ptr;
      module_chromatix.chromatixPtr =
        priv_metadata->sensor_data.chromatix_ptr;
      module_chromatix.chromatixCppPtr =
        priv_metadata->sensor_data.cpp_chromatix_ptr;
      module_chromatix.chromatixSnapCppPtr =
        priv_metadata->sensor_data.cpp_snapchromatix_ptr;
      module_chromatix.chromatixVideoCppPtr =
        priv_metadata->sensor_data.cpp_videochromatix_ptr;
      module_chromatix.chromatixPostProcPtr =
        priv_metadata->sensor_data.postproc_chromatix_ptr;
      module_chromatix.chromatixUsCppPtr =
        priv_metadata->sensor_data.cpp_us_chromatix_ptr;
      module_chromatix.chromatixDsCppPtr =
        priv_metadata->sensor_data.cpp_ds_chromatix_ptr;
      module_chromatix.chromatixOisSnapCppPtr =
        priv_metadata->sensor_data.cpp_ois_snapchromatix_ptr;
      module_chromatix.chromatixOisUsCppPtr =
        priv_metadata->sensor_data.cpp_ois_us_chromatix_ptr;
      module_chromatix.chromatixOisDsCppPtr =
        priv_metadata->sensor_data.cpp_ois_ds_chromatix_ptr;

      /* Update chromatix for offline raw processing if it is valid */
      pproc_module_update_raw_processing_chromatix(pp_feature_config,
        &module_chromatix, priv_metadata);

      module_chromatix.modules_reloaded =  CAMERA_CHROMATIX_MODULE_ALL;
      /* Send chromatix pointer downstream */
      rc = pproc_module_send_set_chromatix_event(port, identity,
        &module_chromatix);
      if (rc == FALSE) {
        PP_ERR("failed: send set chromatix event\n");
        rc = FALSE;
        goto ERROR;
      }

      IF_META_AVAILABLE(cam_edge_application_t, edge,
            CAM_INTF_META_EDGE_MODE, metadata) {
        if (edge->edge_mode != CAM_EDGE_MODE_OFF &&
          edge->edge_mode != CAM_EDGE_MODE_ZERO_SHUTTER_LAG) {
          frame_pp_config.sharpness = edge->sharpness;
        }
      } else {
        PP_WARN("No sharpness value in metadata! Use default!!!\n");
      }

      IF_META_AVAILABLE(uint32_t, noise_mode,
          CAM_INTF_META_NOISE_REDUCTION_MODE, metadata) {
        if ((*noise_mode != CAM_NOISE_REDUCTION_MODE_OFF) &&
          (*noise_mode != CAM_NOISE_REDUCTION_MODE_MINIMAL) &&
          (*noise_mode != CAM_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG)) {
          frame_pp_config.denoise2d.denoise_enable = true;
          frame_pp_config.denoise2d.process_plates =
            CAM_WAVELET_DENOISE_YCBCR_PLANE;
        }
      } else {
        PP_WARN("No denoise value in metadata! Use default!!!\n");
      }

      IF_META_AVAILABLE(cam_rotation_info_t, rotation_info,
        CAM_INTF_PARM_ROTATION, metadata) {
        if (rotation_info->rotation != ROTATE_0) {
          frame_pp_config.rotation = rotation_info->rotation;
          frame_pp_config.device_rotation = rotation_info->device_rotation;
        }
      } else {
        PP_WARN("No rotation value in metadata! Use default!!!\n");
      }

      IF_META_AVAILABLE(int32_t, flip, CAM_INTF_PARM_FLIP, metadata) {
        if (*flip != FLIP_NONE) {
          frame_pp_config.flip = (int32_t) *flip;
        }
      } else {
        PP_WARN("No flip value in metadata! Use default!!!\n");
      }

      IF_META_AVAILABLE(cam_denoise_param_t, tnr_mode,
          CAM_INTF_PARM_TEMPORAL_DENOISE, metadata) {
        frame_pp_config.tnr.denoise_enable = tnr_mode->denoise_enable;
        frame_pp_config.tnr.process_plates =
          tnr_mode->process_plates;
      } else {
        PP_HIGH("No tnr value in metadata! Use default!!!\n");
      }

      IF_META_AVAILABLE(uint32_t, effect_mode, CAM_INTF_PARM_EFFECT, metadata) {
        if (*effect_mode != CAM_EFFECT_MODE_OFF) {
          frame_pp_config.effect = (int32_t) *effect_mode;
        }
      } else {
        PP_WARN("No effect value in metadata! Use default!!!\n");
      }

      IF_META_AVAILABLE(uint32_t, scene_mode, CAM_INTF_PARM_BESTSHOT_MODE,
        metadata) {
        if (*scene_mode != CAM_SCENE_MODE_OFF) {
          frame_pp_config.scene_mode = (int32_t) *scene_mode;
        }
      }

      IF_META_AVAILABLE(int32_t, dsdn, CAM_INTF_PARM_CDS_MODE, metadata) {
          frame_pp_config.dsdn = (int32_t) *dsdn;
      } else {
        PP_WARN("No cpp cpp cds value in metadata! Use default!!!\n");
      }

      IF_META_AVAILABLE(cam_dyn_img_data_t, dyn_mask, CAM_INTF_META_IMG_DYN_FEAT, metadata) {
        frame_pp_config.dyn_img_data.dyn_feature_mask =
          dyn_mask->dyn_feature_mask;
        frame_pp_config.dyn_img_data.input_count =
          dyn_mask->input_count;
        rc = pproc_module_send_set_param_event(port, identity,
          &frame_pp_config, CAM_INTF_META_IMG_DYN_FEAT,
          parm_buf->reprocess.frame_idx);
      } else {
        PP_HIGH("No Dynamic feature mask in metadata! Use default!!!\n");
      }

      IF_META_AVAILABLE (cam_sensor_hdr_type_t, p_hdr_mode,
        CAM_INTF_PARM_SENSOR_HDR, metadata) {
        frame_pp_config.hdr_mode = *p_hdr_mode;
        rc = pproc_module_send_set_param_event(port, identity,
          &frame_pp_config, CAM_INTF_PARM_SENSOR_HDR,
          parm_buf->reprocess.frame_idx);
      } else {
        PP_HIGH("No HDR mode in metadata\n", __func__, __LINE__);
      }

      /* Extract aec update from meta data */
      stats_get =
        (stats_get_data_t *)&priv_metadata->stats_aec_data.private_data;
      memset(&stats_update, 0, sizeof(stats_update_t));
      stats_update.flag = STATS_UPDATE_AEC;
      aec_update = &stats_update.aec_update;
      if (stats_get) {
        aec_update->lux_idx = stats_get->aec_get.lux_idx;
        aec_update->real_gain = stats_get->aec_get.real_gain[0];
        aec_update->linecount = stats_get->aec_get.linecount[0];
        aec_update->exp_time = stats_get->aec_get.exp_time;
        aec_update->exif_iso = stats_get->aec_get.exif_iso;
        aec_update->hdr_sensitivity_ratio = stats_get->aec_get.hdr_sensitivity_ratio;
        aec_update->hdr_exp_time_ratio = stats_get->aec_get.hdr_exp_time_ratio;
        aec_update->total_drc_gain = stats_get->aec_get.total_drc_gain;
        aec_update->color_drc_gain = stats_get->aec_get.color_drc_gain;
        /* Send AEC_UPDATE event */
        rc = pproc_module_send_aec_update_event(port, identity,
          &stats_update, parm_buf->reprocess.frame_idx);
        if (rc == FALSE) {
          PP_ERR("failed: send aec update event\n");
          rc = FALSE;
          goto ERROR;
        }
      }

      rc = pproc_module_extract_gamma_from_metadata(&gamma_update[0],
          sizeof(gamma_update[0]), sizeof(gamma_update)/sizeof(gamma_update[0]),
          (mct_bus_msg_isp_gamma_t *)&priv_metadata->isp_gamma_data);
      if (rc == FALSE) {
        PP_ERR("failed: extract gamma from metadata\n");
      } else {
        rc = pproc_module_send_gamma_update_event(port, identity,
          (uint16_t *)&gamma_update[0], parm_buf->reprocess.frame_idx);
        if (rc == FALSE) {
           PP_ERR("failed: send gamma update event\n");
          rc = FALSE;
          goto ERROR;
        }
      }
      /* Extract awb update from meta data and send downstream */
      memcpy(&awb_update, &priv_metadata->isp_stats_awb_data,
        sizeof(awb_update_t));
      rc = pproc_module_send_awb_update_event(port, identity, &awb_update,
        parm_buf->reprocess.frame_idx);
      if (rc == FALSE) {
        PP_ERR("failed: send awb update event\n");
        rc = FALSE;
        goto ERROR;
      }

      /*Extract LPM event data*/
      inform_lpm.is_lpm_enabled = priv_metadata->isp_lpm_data.is_lpm_enabled;
      rc = pproc_module_send_isp_notify_lpm_event(port, identity, &inform_lpm,
        parm_buf->reprocess.frame_idx);
      if (rc == FALSE) {
        PP_ERR("send inform LPM failed:\n");
      }

       hdr_data = (mct_hdr_data *)&priv_metadata->hdr_data;
       rc = pproc_module_send_hdr_curves(port, identity, hdr_data,
         parm_buf->reprocess.frame_idx);
       if (rc == FALSE) {
         PP_ERR("send HDR curves failed: %d\n", rc);
       }

      // Send face detect data
      IF_META_AVAILABLE(cam_face_detection_data_t, faces_data,
          CAM_INTF_META_FACE_DETECTION, metadata) {

        rc = pproc_module_send_face_info_event(port, identity, faces_data,
          parm_buf->reprocess.frame_idx);
        if (rc == FALSE) {
          PP_ERR("failed: send face info event - ignore error\n");
        }

        // populate gazes_data for True portrait use cases
        IF_META_AVAILABLE(cam_face_gaze_data_t, gazes_data,
          CAM_INTF_META_FACE_GAZE, metadata) {
          rc = pproc_module_send_fd_update_event(port, identity, faces_data,
            gazes_data);
          if (rc == FALSE) {
            PP_ERR("failed: send fd event\n");
            rc = FALSE;
            goto ERROR;
          }
        }
      }
    }
  //}

  /* check for denoise in feature mask */
  if (pp_feature_config->feature_mask & CAM_QCOM_FEATURE_DENOISE2D) {
    rc = pproc_module_send_set_param_event(port, identity,
      &frame_pp_config, CAM_INTF_PARM_WAVELET_DENOISE,
      parm_buf->reprocess.frame_idx);
    if (rc == FALSE) {
      PP_ERR("failed: send stream wnr params event\n");
      goto ERROR;
    }
  }

  /* check for tnr in feature mask */
  if (pp_feature_config->feature_mask & CAM_QCOM_FEATURE_CPP_TNR) {
    rc = pproc_module_send_set_param_event(port, identity,
      &frame_pp_config, CAM_INTF_PARM_TEMPORAL_DENOISE,
      parm_buf->reprocess.frame_idx);
    if (rc == FALSE) {
      PP_ERR("failed: send tnr param crop event\n");
      goto ERROR;
    }
  }

  PP_INFO("input buf index %d input stream id %d, frame id %d, identity %x\n",
    parm_buf->reprocess.buf_index,
    stream_info->reprocess_config.online.input_stream_id,
    parm_buf->reprocess.frame_idx, identity);

  /* check for effect in feature mask */
  if(pp_feature_config->feature_mask & CAM_QCOM_FEATURE_EFFECT) {
    rc = pproc_module_send_set_param_event(port, identity,
      &frame_pp_config, CAM_INTF_PARM_EFFECT,
      parm_buf->reprocess.frame_idx);
    if (rc == FALSE) {
     PP_ERR("failed: send stream effect event\n");
     goto ERROR;
    }
  }
  /* check for sharpness in feature mask */
  if (pp_feature_config->feature_mask & CAM_QCOM_FEATURE_SHARPNESS) {
    rc = pproc_module_send_set_param_event(port, identity,
      &frame_pp_config, CAM_INTF_PARM_SHARPNESS,
      parm_buf->reprocess.frame_idx);
    if (rc == FALSE) {
     PP_ERR("failed: send set param sharpness event\n");
     goto ERROR;
    }
  }

  /* Set scene mode */
  rc = pproc_module_send_set_param_event(port, identity, &frame_pp_config,
    CAM_INTF_PARM_BESTSHOT_MODE, parm_buf->reprocess.frame_idx);
  if (rc == FALSE) {
    PP_ERR("failed: send best shotmode event\n");
  }

  /* check for flip in feature mask */
  if (pp_feature_config->feature_mask & CAM_QCOM_FEATURE_FLIP) {
    rc = pproc_module_send_set_param_event(port, identity,
      &frame_pp_config,
      CAM_INTF_PARM_STREAM_FLIP,
      parm_buf->reprocess.frame_idx);
    if (rc == FALSE) {
     PP_ERR("failed: send set param flip event\n");
     goto ERROR;
    }
  }

  /* check for rotation in feature mask */
  if (pp_feature_config->feature_mask & CAM_QCOM_FEATURE_ROTATION) {
    rc = pproc_module_send_set_param_event(port, identity,
      &frame_pp_config, CAM_INTF_PARM_ROTATION,
      parm_buf->reprocess.frame_idx);
    if (rc == FALSE) {
       PP_ERR("failed: send stream rotation event\n");
       goto ERROR;
    }
  }

  /* check for cpp cds in feature mask */
  if(pp_feature_config->feature_mask & CAM_QCOM_FEATURE_DSDN) {
    rc = pproc_module_send_set_param_event(port, identity,
      &frame_pp_config, CAM_INTF_PARM_CDS_MODE,
      parm_buf->reprocess.frame_idx);
    if (rc == FALSE) {
     PP_ERR("failed: send CDS event\n");
     goto ERROR;
    }
  }

  /* Pick Input buffer from different stream */
  buf_holder = pproc_module_get_online_input_buffer(module, stream_info,
    parm_buf, identity);
  if (!buf_holder) {
    PP_ERR("failed: buf_holder NULL\n");
    rc = FALSE;
    goto ERROR;
  }

  ATRACE_INT("Pproc:MCT", 1);
  ATRACE_INT("Pproc:MCT", 0);
  PP_HIGH("input buf fd %d type %d size %d\n", buf_holder->buf_planes[0].fd,
    buf_holder->buf_type, buf_holder->buf_size);
  /* Send buf divert downstream */
  if (mod_type == MCT_MODULE_FLAG_SOURCE ||
      mod_type == MCT_MODULE_FLAG_PEERLESS) {
    rc = pproc_module_send_buf_divert_event(module, port, identity, buf_holder,
      parm_buf, input_stream_info->identity, metadata);
    if (rc == FALSE) {
      PP_ERR("failed: send buf divert event\n");
      rc = FALSE;
      goto ERROR;
    }
  }

ERROR:
  return rc;
}

/**
* Function: pproc_module_get_meta_buffer
*
* Description: Function used as callback to find
*   metadata buffer wiht corresponding index
*
* Input parameters:
*   @data - MCT stream buffer list
*   @user_data - Pointer to searched buffer index
*
* Return values:
*     true/false
*
* Notes: none
**/
static boolean pproc_module_get_meta_buffer(void *data, void *user_data)
{
  mct_stream_map_buf_t *buf = (mct_stream_map_buf_t *)data;
  uint8_t *buf_index = (uint8_t *)user_data;

  if (!buf || !buf_index) {
    PP_ERR("failed buf %p, buf_index %p", buf, buf_index);
    return FALSE;
  }

  PP_HIGH("buf type %d buff index %d search index %d",
   buf->buf_type, buf->buf_index, *buf_index);

  /* For face detection is used stream buff type */
  if (buf->buf_type != CAM_MAPPING_BUF_TYPE_OFFLINE_META_BUF)
    return FALSE;

  return ((uint8_t)buf->buf_index == *buf_index);
}


/** pproc_module_handle_reprocess_offline
 *    @module: pproc module
 *
 *  Handle pproc_module_handle_reprocess_offline
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_handle_reprocess_offline(mct_module_t *module,
  mct_port_t *port, mct_stream_info_t *input_stream_info,
  cam_stream_parm_buffer_t *parm_buf, uint32_t identity)
{
  boolean                      rc = FALSE;
  uint32_t                     session_id = 0;
  uint32_t                     stream_id = 0;
  cam_pp_offline_src_config_t *offline_src_cfg;
  mct_bus_msg_stream_crop_t    stream_crop;
  mct_event_inform_lpm_t       inform_lpm;
  cam_pp_feature_config_t     *pp_feature_config;
  mct_stream_map_buf_t        *buf_holder = NULL;
  cam_crop_param_t            *crop = NULL;
  metadata_buffer_t           *metadata = NULL;
  void                        *meta_crop_data = NULL;
  pproc_module_pp_config_t     frame_pp_config;
  mct_hdr_data *hdr_data = NULL;

  /* Validate input parameters */
  if (!module || !input_stream_info || !parm_buf) {
    PP_ERR(" failed: module %p input_stream_info %p parm_buf %p\n",
      module, input_stream_info, parm_buf);
    goto ERROR;
  }

  session_id = PPROC_GET_SESSION_ID(identity);
  stream_id = PPROC_GET_STREAM_ID(identity);
  mct_module_type_t mod_type = mct_module_find_type(module,
    identity);

  /* Validate number of planes */
  if (input_stream_info->buf_planes.plane_info.num_planes == 0) {
    PP_ERR("input stream info num planes 0\n");
    rc = FALSE;
    goto ERROR;
  }

  if (mod_type == MCT_MODULE_FLAG_SOURCE ||
    mod_type == MCT_MODULE_FLAG_PEERLESS) {
    /* Pass output dim event */
    offline_src_cfg = &input_stream_info->reprocess_config.offline;
    rc = pproc_module_send_output_dim_event(module, port, identity,
      &offline_src_cfg->input_dim,
      &offline_src_cfg->input_buf_planes,
      offline_src_cfg->input_fmt, parm_buf->reprocess.frame_idx);
    if (rc == FALSE) {
      PP_ERR("failed: send stream crop event\n");
      goto ERROR;
    }
  }

  pp_feature_config = &input_stream_info->reprocess_config.pp_feature_config;

  frame_pp_config.denoise2d.denoise_enable = false;
  frame_pp_config.sharpness = 0;
  frame_pp_config.effect = CAM_EFFECT_MODE_OFF;
  frame_pp_config.rotation = ROTATE_0;
  frame_pp_config.device_rotation = ROTATE_0;
  frame_pp_config.flip = FLIP_NONE;
  frame_pp_config.scene_mode = CAM_SCENE_MODE_OFF;
  frame_pp_config.tnr.denoise_enable = false;
  frame_pp_config.dsdn = 0;
  frame_pp_config.dyn_img_data.dyn_feature_mask = 0;

  if (parm_buf->reprocess.meta_present == 1) {
    /* Extract meta buffer */
    mct_list_t  *temp_list = NULL;

     temp_list = mct_list_find_custom(input_stream_info->img_buffer_list,
       &parm_buf->reprocess.meta_buf_index,pproc_module_get_meta_buffer);
     if (temp_list && temp_list->data) {
       mct_stream_map_buf_t *buff_holder = temp_list->data;
       metadata = buff_holder->buf_planes[0].buf;
     }
     if (metadata) {
       modulesChromatix_t                module_chromatix;
       mct_stream_session_metadata_info *priv_metadata;
       stats_get_data_t                 *stats_get = NULL;
       stats_update_t                    stats_update;
       aec_update_t                     *aec_update = NULL;
       cam_awb_params_t                 *awb_meta = NULL;
       mct_bus_msg_awb_immediate_t      *awb_msg = NULL;
       awb_update_t                      awb_update;
       uint16_t                          gamma_update[64];

       rc = pproc_module_send_set_meta_event(port, identity, metadata);
       if (rc == FALSE) {
         PP_ERR("failed: send set meta data ptr event\n");
         rc = FALSE;
         goto ERROR;
       }

      /* Send dual camera (Main/wide & Aux/tele) shift offset data */
      IF_META_AVAILABLE(cam_sac_output_info_t,
        dc_sac_output_data,
        CAM_INTF_META_DC_SAC_OUTPUT_INFO,
        metadata) {
        if (dc_sac_output_data && (dc_sac_output_data->is_output_shift_valid)) {
          rc = pproc_module_send_dc_shift_offset_event(
                 port,
                 identity,
                 parm_buf->reprocess.frame_idx,
                 dc_sac_output_data);
          if (rc == FALSE) {
            PP_ERR("failed: Main & Aux shift offset info event\n");
          }
        }
      }

      /* Send dual camera (Main/wide & Aux/tele) margin data */
      IF_META_AVAILABLE(cam_crop_data_t,
        crop_data,
        CAM_INTF_META_CROP_DATA,
        metadata) {
        if (crop_data) {
          PP_DBG("b4 pproc: crop data margins:{wm=%f,hm=%f} ",
            crop_data->margins.widthMargins,
            crop_data->margins.heightMargins);

          /*
          ** Margins should to be populated appropriately by the respective
          ** control modules (e.g., FOV Control in HAL, for both QC-SAC/SAT
          ** features) so that CPP can apply them in a streamlined fashion.
          */
          /*/
          crop_data->margins.widthMargins  = 0.0f;
          crop_data->margins.heightMargins = 0.0f;
          /*/

          PP_DBG("l8r pproc: crop data margins:{wm=%f,hm=%f} ",
            crop_data->margins.widthMargins,
            crop_data->margins.heightMargins);
          rc = pproc_module_send_dc_shift_margins_event(
                 port,
                 identity,
                 parm_buf->reprocess.frame_idx,
                 crop_data);
          if (rc == FALSE) {
            PP_ERR("failed: send dual cam margins info event\n");
          }
        }
      }

      if (mod_type == MCT_MODULE_FLAG_SOURCE ||
        mod_type == MCT_MODULE_FLAG_PEERLESS) {
        PP_DBG("stream_crop.frame_id(reprocess)=%u", parm_buf->reprocess.frame_idx);
        /* Extract crop params from meta data */
        rc = pproc_module_extract_crop_params(metadata, input_stream_info,
          identity, PPROC_GET_STREAM_ID(identity), &stream_crop);
        if (rc == TRUE) {
          stream_crop.frame_id = parm_buf->reprocess.frame_idx;
          rc = pproc_module_send_stream_crop_event(port, identity, &stream_crop);
          if (rc == FALSE) {
            PP_ERR("failed: send stream crop event\n");
            rc = FALSE;
            goto ERROR;
          }
        }
      }

       /* Extract chromatix params from meta data */
       memset(&module_chromatix, 0, sizeof(modulesChromatix_t));
       priv_metadata =
         (mct_stream_session_metadata_info *)
         POINTER_OF_META(CAM_INTF_META_PRIVATE_DATA, metadata);
       module_chromatix.chromatixComPtr =
         priv_metadata->sensor_data.common_chromatix_ptr;
       module_chromatix.chromatixPtr =
         priv_metadata->sensor_data.chromatix_ptr;
       module_chromatix.chromatixCppPtr =
         priv_metadata->sensor_data.cpp_chromatix_ptr;
       module_chromatix.chromatixSnapCppPtr =
         priv_metadata->sensor_data.cpp_snapchromatix_ptr;
       module_chromatix.chromatixVideoCppPtr =
         priv_metadata->sensor_data.cpp_videochromatix_ptr;
       module_chromatix.chromatixPostProcPtr =
         priv_metadata->sensor_data.postproc_chromatix_ptr;
       module_chromatix.chromatixUsCppPtr =
         priv_metadata->sensor_data.cpp_us_chromatix_ptr;
       module_chromatix.chromatixDsCppPtr =
         priv_metadata->sensor_data.cpp_ds_chromatix_ptr;
       module_chromatix.chromatixOisSnapCppPtr =
         priv_metadata->sensor_data.cpp_ois_snapchromatix_ptr;
       module_chromatix.chromatixOisUsCppPtr =
         priv_metadata->sensor_data.cpp_ois_us_chromatix_ptr;
       module_chromatix.chromatixOisDsCppPtr =
         priv_metadata->sensor_data.cpp_ois_ds_chromatix_ptr;

      /* Update chromatix for offline raw processing if it is valid */
      pproc_module_update_raw_processing_chromatix(pp_feature_config,
        &module_chromatix, priv_metadata);

      module_chromatix.modules_reloaded =  CAMERA_CHROMATIX_MODULE_ALL;
       /* Send chromatix pointer downstream */
       rc = pproc_module_send_set_chromatix_event(port, identity,
         &module_chromatix);
       if (rc == FALSE) {
         PP_ERR("failed: send chromatix pointer event\n");
       }

      IF_META_AVAILABLE(cam_edge_application_t, edge,
            CAM_INTF_META_EDGE_MODE, metadata) {
        if (edge->edge_mode != CAM_EDGE_MODE_OFF &&
          edge->edge_mode != CAM_EDGE_MODE_ZERO_SHUTTER_LAG) {
          frame_pp_config.sharpness = edge->sharpness;
        }
      } else {
        PP_WARN("No sharpness value in metadata! Use default!!!\n");
      }

      IF_META_AVAILABLE(uint32_t, noise_mode,
            CAM_INTF_META_NOISE_REDUCTION_MODE, metadata) {
        if ((*noise_mode != CAM_NOISE_REDUCTION_MODE_OFF) &&
          (*noise_mode != CAM_NOISE_REDUCTION_MODE_MINIMAL) &&
          (*noise_mode != CAM_NOISE_REDUCTION_MODE_ZERO_SHUTTER_LAG)) {
          frame_pp_config.denoise2d.denoise_enable = true;
          frame_pp_config.denoise2d.process_plates =
            CAM_WAVELET_DENOISE_YCBCR_PLANE;
        }
      } else {
        PP_WARN("No denoise value in metadata! Use default!!!\n");
      }

      IF_META_AVAILABLE(cam_rotation_info_t, rotation_info,
        CAM_INTF_PARM_ROTATION, metadata) {
        if (rotation_info->rotation != ROTATE_0) {
          frame_pp_config.rotation = rotation_info->rotation;
          frame_pp_config.device_rotation = rotation_info->device_rotation;
        }
      } else {
        PP_WARN("No rotation value in metadata! Use default!!!\n");
      }

      IF_META_AVAILABLE(int32_t, flip, CAM_INTF_PARM_FLIP, metadata) {
        if (flip != FLIP_NONE) {
          frame_pp_config.flip = (int32_t) *flip;
        }
      } else {
        PP_WARN("No flip value in metadata! Use default!!!\n");
      }

      IF_META_AVAILABLE(cam_denoise_param_t, tnr_mode,
          CAM_INTF_PARM_TEMPORAL_DENOISE, metadata) {
        frame_pp_config.tnr.denoise_enable = tnr_mode->denoise_enable;
        frame_pp_config.tnr.process_plates =
          tnr_mode->process_plates;
      } else {
        PP_HIGH("No tnr value in metadata! Use default!!!\n");
      }


      IF_META_AVAILABLE(uint32_t, effect_mode, CAM_INTF_PARM_EFFECT, metadata) {
        if (*effect_mode != CAM_EFFECT_MODE_OFF) {
          frame_pp_config.effect = (int32_t)*effect_mode;
        }
      } else {
        PP_WARN("No effect value in metadata! Use default!!!\n");
      }

      IF_META_AVAILABLE(uint32_t, scene_mode, CAM_INTF_PARM_BESTSHOT_MODE,
        metadata) {
        if (*scene_mode != CAM_SCENE_MODE_OFF) {
          frame_pp_config.scene_mode = (int32_t) *scene_mode;
        }
      }

      IF_META_AVAILABLE(int32_t, dsdn, CAM_INTF_PARM_CDS_MODE, metadata) {
          frame_pp_config.dsdn = (int32_t) *dsdn;
      } else {
        PP_WARN("No cpp cds value in metadata! Use default!!!\n");
      }

      IF_META_AVAILABLE(cam_dyn_img_data_t, dyn_mask,
        CAM_INTF_META_IMG_DYN_FEAT, metadata) {
        frame_pp_config.dyn_img_data.dyn_feature_mask =
          dyn_mask->dyn_feature_mask;
        frame_pp_config.dyn_img_data.input_count =
          dyn_mask->input_count;
        rc = pproc_module_send_set_param_event(port, identity,
          &frame_pp_config, CAM_INTF_META_IMG_DYN_FEAT,
          parm_buf->reprocess.frame_idx);
      } else {
        PP_HIGH("No Dynamic feature mask in metadata! Use default!!!\n");
      }

      IF_META_AVAILABLE (cam_sensor_hdr_type_t, p_hdr_mode,
        CAM_INTF_PARM_SENSOR_HDR, metadata) {
        frame_pp_config.hdr_mode = *p_hdr_mode;
        rc = pproc_module_send_set_param_event(port, identity,
          &frame_pp_config, CAM_INTF_PARM_SENSOR_HDR,
          parm_buf->reprocess.frame_idx);
      } else {
        PP_HIGH("No HDR mode in metadata\n", __func__, __LINE__);
      }

       /* Extract aec update from meta data */
       stats_get =
         (stats_get_data_t *)&priv_metadata->stats_aec_data.private_data;
       memset(&stats_update, 0, sizeof(stats_update_t));
       stats_update.flag = STATS_UPDATE_AEC;
       aec_update = &stats_update.aec_update;
       if (stats_get) {
         aec_update->lux_idx = stats_get->aec_get.lux_idx;
         aec_update->real_gain = stats_get->aec_get.real_gain[0];
         aec_update->exif_iso = stats_get->aec_get.exif_iso;
         aec_update->hdr_sensitivity_ratio = stats_get->aec_get.hdr_sensitivity_ratio;
         aec_update->hdr_exp_time_ratio = stats_get->aec_get.hdr_exp_time_ratio;
         aec_update->total_drc_gain = stats_get->aec_get.total_drc_gain;
         aec_update->color_drc_gain = stats_get->aec_get.color_drc_gain;
         /* Send AEC_UPDATE event */
         rc = pproc_module_send_aec_update_event(port, identity,
           &stats_update, parm_buf->reprocess.frame_idx);
         if (rc == FALSE) {
           PP_ERR("failed: send aec update event\n");
         }
       }

      /* Extract gamma update from meta data and send downstream */
      rc = pproc_module_extract_gamma_from_metadata(&gamma_update[0],
          sizeof(gamma_update[0]), sizeof(gamma_update)/sizeof(gamma_update[0]),
          (mct_bus_msg_isp_gamma_t *)&priv_metadata->isp_gamma_data);
      if (rc == FALSE) {
        PP_ERR("failed: extract gamma from metadata\n");
      } else {
        rc = pproc_module_send_gamma_update_event(port, identity,
          (uint16_t *)&gamma_update[0], parm_buf->reprocess.frame_idx);
        if (rc == FALSE) {
          PP_ERR("failed: send gamma update event\n");
        }
      }

      if (pp_feature_config->feature_mask & CAM_QCOM_FEATURE_QUADRA_CFA) {
        /* Extract awb update from meta data */
        awb_msg = (mct_bus_msg_awb_immediate_t *)
          priv_metadata->stats_awb_data.private_data;
        if (!awb_msg) {
          PP_ERR("failed: extract awb update event awb_msg %p",
            awb_msg);
          rc = FALSE;
          goto ERROR;
        }
        awb_meta = (cam_awb_params_t *)&(awb_msg->awb_info);
        if (!awb_meta) {
          PP_ERR("failed: extract awb update event awb_meta %p\n",
            awb_meta);
        } else {
          memcpy(&awb_update, &priv_metadata->isp_stats_awb_data,
            sizeof(awb_update_t));

          awb_update.color_temp = awb_meta->cct_value;
          awb_update.gain.g_gain = awb_meta->rgb_gains.g_gain;
          awb_update.gain.b_gain = awb_meta->rgb_gains.b_gain;
          awb_update.gain.r_gain = awb_meta->rgb_gains.r_gain;

          awb_update.ccm_update.awb_ccm_enable =
            awb_meta->ccm_update.awb_ccm_enable;
          awb_update.ccm_update.hard_awb_ccm_flag=
            awb_meta->ccm_update.hard_awb_ccm_flag;
          awb_update.ccm_update.ccm_update_flag =
            awb_meta->ccm_update.ccm_update_flag;
          memcpy(awb_update.ccm_update.ccm,
            awb_meta->ccm_update.ccm,
            sizeof(float)*AWB_NUM_CCM_ROWS*AWB_NUM_CCM_COLS);
          memcpy(awb_update.ccm_update.ccm_offset,
            awb_meta->ccm_update.ccm_offset,
            sizeof(float)*AWB_NUM_CCM_ROWS);

          rc = pproc_module_send_awb_update_event(port, identity, &awb_update,
            parm_buf->reprocess.frame_idx);
          if (rc == FALSE) {
            PP_ERR("failed: send awb update event\n");
          }
        }
      } else {
        /* Extract awb update from meta data and send downstream */
        memcpy(&awb_update, &priv_metadata->isp_stats_awb_data,
          sizeof(awb_update_t));
        rc = pproc_module_send_awb_update_event(port, identity, &awb_update,
          parm_buf->reprocess.frame_idx);
        if (rc == FALSE) {
          PP_ERR("failed: send awb update event\n");
        }
      }
       /*Extract LPM event data*/
       inform_lpm.is_lpm_enabled = priv_metadata->isp_lpm_data.is_lpm_enabled;
       rc = pproc_module_send_isp_notify_lpm_event(port, identity, &inform_lpm,
        parm_buf->reprocess.frame_idx);
       if (rc == FALSE) {
         PP_ERR("send inform LPM failed: \n");
       }

       hdr_data = (mct_hdr_data *)&priv_metadata->hdr_data;
       rc = pproc_module_send_hdr_curves(port, identity, hdr_data,
         parm_buf->reprocess.frame_idx);
       if (rc == FALSE) {
         PP_ERR("send HDR curves failed: %d\n", rc);
       }

      // Send face detect data
      IF_META_AVAILABLE(cam_face_detection_data_t, faces_data,
          CAM_INTF_META_FACE_DETECTION, metadata) {

        rc = pproc_module_send_face_info_event(port, identity, faces_data,
          parm_buf->reprocess.frame_idx);
        if (rc == FALSE) {
          PP_ERR("failed: send face info event - ignore error\n");
        }

        // populate gazes_data for True portrait use cases
        IF_META_AVAILABLE(cam_face_gaze_data_t, gazes_data,
          CAM_INTF_META_FACE_GAZE, metadata) {
          rc = pproc_module_send_fd_update_event(port, identity, faces_data,
            gazes_data);
          if (rc == FALSE) {
            PP_ERR("failed: send fd event\n");
            rc = FALSE;
            goto ERROR;
          }
        }
      }
    } else {
      PP_ERR("Metadata buffer idx %d is not available",
        parm_buf->reprocess.meta_buf_index);
    }
  }
  /* check for denoise in feature mask */
  if (pp_feature_config->feature_mask & CAM_QCOM_FEATURE_DENOISE2D) {
    rc = pproc_module_send_set_param_event(port, identity,
      &frame_pp_config, CAM_INTF_PARM_WAVELET_DENOISE,
      parm_buf->reprocess.frame_idx);
    if (rc == FALSE) {
      PP_ERR("failed: send stream crop event\n");
      goto ERROR;
    }
  }

  /* check for tnr in feature mask */
  if (pp_feature_config->feature_mask & CAM_QCOM_FEATURE_CPP_TNR) {
    rc = pproc_module_send_set_param_event(port, identity,
      &frame_pp_config, CAM_INTF_PARM_TEMPORAL_DENOISE,
      parm_buf->reprocess.frame_idx);
    if (rc == FALSE) {
      PP_ERR("failed: send tnr param event\n");
      goto ERROR;
    }
  }
  /* check for effect in feature mask */
  if(pp_feature_config->feature_mask & CAM_QCOM_FEATURE_EFFECT) {
    rc = pproc_module_send_set_param_event(port, identity,
      &frame_pp_config, CAM_INTF_PARM_EFFECT,
      parm_buf->reprocess.frame_idx);
    if (rc == FALSE) {
     PP_ERR("failed: send stream effect event\n");
     goto ERROR;
    }
  }

  /* check for sharpness in feature mask */
  if (pp_feature_config->feature_mask & CAM_QCOM_FEATURE_SHARPNESS) {
    rc = pproc_module_send_set_param_event(port, identity,
      &frame_pp_config, CAM_INTF_PARM_SHARPNESS,
      parm_buf->reprocess.frame_idx);
    if (rc == FALSE) {
      PP_ERR("failed: send set param sharpness event\n");
      goto ERROR;
    }
  }

  /* check for flip in feature mask */
  if (pp_feature_config->feature_mask & CAM_QCOM_FEATURE_FLIP) {
    rc = pproc_module_send_set_param_event(port, identity,
      &frame_pp_config,
      CAM_INTF_PARM_STREAM_FLIP,
      parm_buf->reprocess.frame_idx);
    if (rc == FALSE) {
     PP_ERR("failed: send set param flip event\n");
     goto ERROR;
    }
  }

  /* check for rotation in feature mask */
  if (pp_feature_config->feature_mask & CAM_QCOM_FEATURE_ROTATION) {
    rc = pproc_module_send_set_param_event(port, identity,
      &frame_pp_config, CAM_INTF_PARM_ROTATION,
      parm_buf->reprocess.frame_idx);
    if (rc == FALSE) {
      PP_ERR("failed: send stream rotation event\n");
      goto ERROR;
    }
  }

  /* check for cpp cds in feature mask */
  if(pp_feature_config->feature_mask & CAM_QCOM_FEATURE_DSDN) {
    rc = pproc_module_send_set_param_event(port, identity,
      &frame_pp_config, CAM_INTF_PARM_CDS_MODE,
      parm_buf->reprocess.frame_idx);
    if (rc == FALSE) {
     PP_ERR("failed: send CDS event\n");
     goto ERROR;
    }
  }

  /* Set scene mode */
  rc = pproc_module_send_set_param_event(port, identity, &frame_pp_config,
    CAM_INTF_PARM_BESTSHOT_MODE, parm_buf->reprocess.frame_idx);
  if (rc == FALSE) {
    PP_ERR("failed: send best shotmode event\n");
  }

  /* Pick Input buffer from different stream */
  buf_holder = pproc_module_get_offline_input_buffer(module, parm_buf,
    identity);
  if (!buf_holder) {
    PP_ERR("failed: buf_holder NULL\n");
    rc = FALSE;
    goto ERROR;
  }

  PP_INFO("input buf fd %d type %d size %d identity %x, frame_id %d\n",
    buf_holder->buf_planes[0].fd, buf_holder->buf_type, buf_holder->buf_size,
    identity, parm_buf->reprocess.frame_idx);
  /* Send buf divert downstream */
  if (mod_type == MCT_MODULE_FLAG_SOURCE ||
      mod_type == MCT_MODULE_FLAG_PEERLESS) {
    rc = pproc_module_send_buf_divert_event(module, port, identity, buf_holder,
      parm_buf, identity, metadata);
    if (rc == FALSE) {
      PP_ERR("failed: send buf divert event\n");
      goto ERROR;
    }
  }

  return TRUE;

ERROR:
  return rc;
}

/** pproc_module_offline_stream_param_buf
 *    @module: pproc module
 *    @event: mct event to be handled
 *    @data: stream info
 *
 *  Handle stream param buf event for offline stream. Use input
 *  buffer index sent as part of this call for performing post
 *  processing. Use crop information, chromatix pointers and AEC
 *  trigger from stream info and meta data respectively.
 *
 *  Return: TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_offline_stream_param_buf(mct_module_t *module,
  mct_event_t *event, void *data)
{
  boolean                     rc = TRUE;
  uint32_t                    i = 0;
  mct_port_t                 *port = NULL;
  mct_stream_info_t          *stream_info = NULL;
  cam_stream_parm_buffer_t   *parm_buf = NULL;
  mct_module_type_t           mod_type;

  /* Validate input pameters */
  if (!module || strcmp(MCT_OBJECT_NAME(module), "pproc") || !event || !data) {
    PP_ERR("failed: data %p\n", data);
    rc = FALSE;
    goto ERROR;
  }

  parm_buf = (cam_stream_parm_buffer_t *)data;
  mod_type = mct_module_find_type(module,
      event->identity);

  /* Validate parm buf type */
  if (parm_buf->type != CAM_STREAM_PARAM_TYPE_DO_REPROCESS
    && parm_buf->type != CAM_STREAM_PARAM_TYPE_GET_OUTPUT_CROP
    && parm_buf->type != CAM_STREAM_PARAM_TYPE_REQUEST_OPS_MODE
    && parm_buf->type != CAM_STREAM_PARAM_TYPE_GET_IMG_PROP) {
    PP_HIGH("failed: parm_buf type is invalid %d\n", parm_buf->type);
    rc = TRUE;
    goto ERROR;
  }

  /* Get reserved pproc sink port for this identity */
  port = pproc_port_get_reserved_port(module, event->identity);
  if (!port) {
    PP_ERR("failed: reserved port NULL");
    rc = FALSE;
    goto ERROR;
  }

  /* Get stream info for pproc sink port */
  stream_info = pproc_port_get_attached_stream_info(port, event->identity);
  if (!stream_info) {
    PP_ERR("failed: stream info NULL\n");
    rc = FALSE;
    goto ERROR;
  }

  /*There is possibility for downstream modules (like sw-wnr) to expect this
  event*/
  if (mod_type == MCT_MODULE_FLAG_SOURCE ||
    mod_type == MCT_MODULE_FLAG_PEERLESS) {
    rc = port->event_func(port, event);
    if (FALSE == rc) {
      PP_ERR("Error sending stream param buf event downstream\n");
      goto ERROR;
    }
  }
  if (CAM_STREAM_PARAM_TYPE_DO_REPROCESS == parm_buf->type) {
    /* Find the type of reprocess, offline / online */
    if (stream_info->reprocess_config.pp_type == CAM_ONLINE_REPROCESS_TYPE) {
      rc = pproc_module_handle_reprocess_online(module, port, stream_info,
        parm_buf, event->identity);
      if (rc == FALSE) {
        PP_ERR("failed: handle reprocess online\n");
        rc = FALSE;
        goto ERROR;
      }
    } else if (stream_info->reprocess_config.pp_type ==
      CAM_OFFLINE_REPROCESS_TYPE) {
      rc = pproc_module_handle_reprocess_offline(module, port, stream_info,
        parm_buf, event->identity);
      if (rc == FALSE) {
        PP_ERR("failed: handle reprocess offline\n");
        rc = FALSE;
        goto ERROR;
      }
    }
  }

ERROR:
  return rc;
}

/** pproc_module_offline_streamoff
 *    @module: pproc module
 *    @event: mct event to be handled
 *    @data: stream info
 *
 *  Handle stream off event for offline stream.
 *
 *  Return: TRUE TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_offline_streamoff(mct_module_t *module,
  mct_event_t *event, void *data)
{
  boolean     rc = TRUE;
  mct_list_t *lport = NULL;
  mct_port_t *port = NULL;

  PP_LOW("data %p\n", data);
  ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_PPROC_STREAMOFF);
  /* Validate input pameters */
  if (!module || strcmp(MCT_OBJECT_NAME(module), "pproc") || !event || !data) {
    PP_ERR("failed: data %p or module %p, event %p or module not pproc\n",
      data, module, event);
    rc = FALSE;
    goto ERROR;
  }

  /* Find pproc port for this identity */
  lport = mct_list_find_custom(MCT_MODULE_SINKPORTS(module), &event->identity,
    pproc_port_check_identity_in_port);
  if (!lport) {
    PP_ERR("failed: to find pproc port\n");
    rc = FALSE;
    goto ERROR;
  }

  /* Extract pproc port from mct list */
  port = (mct_port_t *)lport->data;
  if (!port) {
    PP_ERR("failed: reserved port NULL");
    rc = FALSE;
    goto ERROR;
  }

  /* Call streamoff event on pproc port to get forwarded to sub module */
  rc = port->event_func(port, event);
  if (rc == FALSE) {
    PP_ERR("failed: to stream off sub module\n");
    rc = FALSE;
  }

ERROR:
  ATRACE_CAMSCOPE_END(CAMSCOPE_PPROC_STREAMOFF);
  return rc;
}

/** pproc_module_offline_deletestream
 *    @module: pproc module
 *    @event: mct event to be handled
 *    @data: stream info
 *
 *  Call unlink and caps unreserve on submodule's sink port that is used for
 *  offline stream.
 *
 *  Return: TRUE TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_offline_deletestream(mct_module_t *module,
  mct_event_t *event, void *data)
{
  boolean     rc = TRUE;
  mct_list_t *lport = NULL;
  mct_port_t *port = NULL;

  PP_LOW("data %p\n", data);
  /* Validate input pameters */
  if (!module || strcmp(MCT_OBJECT_NAME(module), "pproc") || !event || !data) {
    PP_ERR("failed: data %p\n", data);
    rc = FALSE;
    goto ERROR;
  }

  /* Find pproc port for this identity */
  lport = mct_list_find_custom(MCT_MODULE_SINKPORTS(module), &event->identity,
    pproc_port_check_identity_in_port);
  if (!lport) {
    PP_ERR("failed: to find pproc port\n");
    rc = FALSE;
    goto ERROR;
  }

  /* Extract pproc port from mct list */
  port = (mct_port_t *)lport->data;
  if (!port) {
    PP_ERR("failed: reserved port NULL");
    rc = FALSE;
    goto ERROR;
  }

  rc = port->check_caps_unreserve(port, event->identity);
  if (rc == FALSE) {
    PP_ERR("failed: to caps unreserve\n");
    rc = FALSE;
  }
  mct_port_remove_child(event->identity, port);
ERROR:
  return rc;
}

/** pproc_module_handle_set_param
 *    @module: pproc module
 *    @event: mct event to be handled
 *
 *  Handle set param event sent through module interface.
 *
 *  Return: TRUE TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_handle_set_param(mct_module_t *module,
  mct_event_t *event)
{
  boolean                 rc = TRUE;
  pproc_module_private_t *mod_private;
  mct_event_control_parm_t *ctrl_parm;
  mct_module_t           *c2d_module;

  mod_private = (pproc_module_private_t *)MCT_OBJECT_PRIVATE(module);
  if (NULL == mod_private) {
    PP_ERR("mod_private:%p\n", mod_private);
    return FALSE;
  }

  ctrl_parm = (mct_event_control_parm_t *)
    event->u.ctrl_event.control_event_data;
  if(!ctrl_parm || !ctrl_parm->parm_data) {
    PP_ERR("invalid ctrl_parm, failed");
    return FALSE;
  }

  //get hal version data
  if (ctrl_parm->type == CAM_INTF_PARM_HAL_VERSION) {
    mod_private->hal_version = *((int32_t *)ctrl_parm->parm_data);
    PP_LOW("hal_version = %d", mod_private->hal_version);
  }

  c2d_module = pproc_module_get_sub_mod(module, "c2d");
  if (c2d_module && c2d_module->process_event) {
    rc = c2d_module->process_event(c2d_module, event);
  }
  return rc;
}

/** pproc_module_forward_event
 *    @module: pproc module
 *    @event: mct event to be handled
 *
 *  Handle event sent through module interface.
 *
 *  Return: TRUE TRUE on success
 *          FALSE otherwise **/
static boolean pproc_module_forward_event(mct_module_t *module,
  mct_event_t *event)
{
  boolean            rc = TRUE;
  uint32_t           i = 0;
  mct_port_t        *port = NULL;
  mct_stream_info_t *stream_info = NULL;

  /* Validate input pameters */
  if (!module || strcmp(MCT_OBJECT_NAME(module), "pproc") || !event) {
    PP_ERR("failed: module:%p, event:%p\n", module, event);
    rc = FALSE;
    goto PPROC_MOD_FWD_EVENT_ERROR;
  }

  /* Get reserved pproc sink port for this identity */
  port = pproc_port_get_reserved_port(module, event->identity);
  if (!port) {
    PP_ERR("failed: reserved port NULL");
    rc = FALSE;
    goto PPROC_MOD_FWD_EVENT_ERROR;
  }

  /* Get stream info for pproc sink port */
  stream_info = pproc_port_get_attached_stream_info(port, event->identity);
  if (!stream_info) {
    PP_ERR("failed: stream info NULL\n");
    rc = FALSE;
    goto PPROC_MOD_FWD_EVENT_ERROR;
  }

  rc = port->event_func(port, event);
  if (FALSE == rc) {
    PP_ERR("Error sending event downstream\n");
    goto PPROC_MOD_FWD_EVENT_ERROR;
  }

PPROC_MOD_FWD_EVENT_ERROR:
  return rc;
}

/** pproc_module_process_event
 *    @module: pproc module
 *    @event: mct event to be handled
 *
 *  Handle event set on this module. As per current
 *  architecture, his path is exercised only for reprocessing
 *
 *  Return: TRUE if event is handled successfully
 *          FALSE otherwise **/
static boolean pproc_module_process_event(mct_module_t *module,
  mct_event_t *event)
{
  boolean              rc = TRUE;
  mct_event_control_t *ctrl_event = NULL;

  /* Validate input parameters */
  if (!module || !event || strcmp(MCT_OBJECT_NAME(module), "pproc")) {
    PP_ERR("module %p event %p\n", module, event);
    rc = FALSE;
    goto ERROR;
  }

  /* Check whether event's type is MCT_EVENT_CONTROL_CMD */
  if (event->type == MCT_EVENT_CONTROL_CMD) {
    ctrl_event = &event->u.ctrl_event;
    PP_LOW("event %d", ctrl_event->type);
    switch (ctrl_event->type) {
    case MCT_EVENT_CONTROL_STREAMON:
        ATRACE_CAMSCOPE_BEGIN(CAMSCOPE_CPP_STREAMON);
        rc = pproc_module_offline_streamon(module, event,
        ctrl_event->control_event_data);
        ATRACE_CAMSCOPE_END(CAMSCOPE_CPP_STREAMON);
      break;
    case MCT_EVENT_CONTROL_PARM_STREAM_BUF:
      rc = pproc_module_offline_stream_param_buf(module, event,
        ctrl_event->control_event_data);
      break;
    case MCT_EVENT_CONTROL_STREAMOFF:
      rc = pproc_module_offline_streamoff(module, event,
        ctrl_event->control_event_data);
      break;
    case MCT_EVENT_CONTROL_SET_PARM:
      rc = pproc_module_handle_set_param(module, event);
      break;
    case MCT_EVENT_CONTROL_DEL_OFFLINE_STREAM:
      rc = pproc_module_offline_deletestream(module, event,
        ctrl_event->control_event_data);
      break;
    default:
      rc = pproc_module_forward_event(module, event);
      break;
    }
  } else {
    rc = pproc_module_forward_event(module, event);
  }
ERROR:
  return rc;
}

/** pproc_module_start_session
 *    @module:   pproc module
 *    @identity: stream|session identity
 *
 *  Call submodule start session function.
 *
 *  Return TRUE on success.
 **/
static boolean pproc_module_start_session(mct_module_t *module,
  uint32_t sessionid)
{
  pproc_module_private_t  *mod_private;
  boolean                  rc = FALSE;

  pproc_module_loglevel(); //dynamic logging
  PP_LOW("E\n");
  /* Sanity check */
  if (!module || !MCT_OBJECT_NAME(module) || strcmp(MCT_OBJECT_NAME(module), "pproc")) {
    PP_ERR("error module: %p\n", module);
    return rc;
  }

  MCT_OBJECT_LOCK(module);
  mod_private = MCT_OBJECT_PRIVATE(module);
  if (!mod_private) {
    PP_ERR("error module private is NULL\n");
    goto start_session_done;
  }

#if 0
  /* TODO: set_mod() is not called during removing the stream */
  /* check to see if the session has already been existing */
  if (mct_list_find_custom(mod_private->module_type_list,
        &sessionid, pproc_module_check_session) != NULL) {
    rc = TRUE;
    goto start_session_done;
  }
#endif

  CALL_SUBMOD_FUNCTION(mod_private, start_session, sessionid);

  rc = TRUE;

start_session_done:
  MCT_OBJECT_UNLOCK(module);
  PP_LOW("X\n");
  return rc;
}

/** pproc_module_stop_session
 *    @module: PPROC module
 *    @identity: stream|session identity
 *
 *  Call submodule stop session function
 *
 *  Return TRUE on success.
 **/
static boolean pproc_module_stop_session(mct_module_t *module,
  uint32_t sessionid)
{
  pproc_module_private_t  *mod_private;
  boolean                  rc = FALSE;

  PP_LOW("E\n");
  /* Sanity check */
  if (!module || !MCT_OBJECT_NAME(module) || strcmp(MCT_OBJECT_NAME(module), "pproc")) {
    PP_ERR("error module: %p\n", module);
    return rc;
  }

  MCT_OBJECT_LOCK(module);
  mod_private = MCT_OBJECT_PRIVATE(module);
  if (!mod_private) {
    PP_ERR("error module private is NULL\n");
    goto stop_session_done;
  }

#if 0
  /* TODO: set_mod() is not called during removing the stream */
  /* check to see if the session has already been existing */
  if (mct_list_find_custom(mod_private->module_type_list,
        &sessionid, pproc_module_check_session) != NULL) {
    rc = TRUE;
    goto stop_session_done;
  }
#endif

  CALL_SUBMOD_FUNCTION(mod_private, stop_session, sessionid);

  rc = TRUE;

stop_session_done:
  MCT_OBJECT_UNLOCK(module);
  PP_LOW("X\n");
  return rc;
}

/** pproc_module_request_new_port
 *    @stream_info:
 *    @direction:
 *    @module:
 *    @peer_caps:
 *
 *    Create new port dynamically due to lack of support from
 *    existing ports.
 **/
static mct_port_t *pproc_module_request_new_port(void *stream_info __unused,
  mct_port_direction_t direction __unused, mct_module_t *module __unused,
  void *peer_caps __unused)
{
  /* TODO:
   *
   *   This function may be triggered after LINK fails due to lack of port
   *   support or when pproc is a source port but MCT needs a sink port to
   *   connect to it and communicate through this port.
   *
   *   Dependes on the nature of stream_info and checking with sub-modules,
   *   i.e, CPP/VPE/C2D to see whether or not new port can be created. If
   *   yes, create a corresponding new port.
   */
  return NULL;
}

/** pproc_module_set_mod
*     @module:      pproc module itself ("pproc")
 *    @module_type: module type to set
 *    @identity:    stream|session identity
 *
 *  Set this module the type for the stream(from identity).
 *
 *  Return TRUE on success
 **/
static void pproc_module_set_mod(mct_module_t *module,
  mct_module_type_t module_type, uint32_t identity)
{
  mct_module_type_t       type = module_type;
  pproc_module_private_t *mod_private;
  pproc_module_type_t    *pproc_type;
  mct_list_t             *list;

  PP_LOW("E\n");
  if (!module || strcmp(MCT_OBJECT_NAME(module), "pproc")) {
    PP_ERR("error module: %p\n", module);
    return;
  }

  MCT_OBJECT_LOCK(module);
  mod_private = (pproc_module_private_t *)MCT_OBJECT_PRIVATE(module);

  /* If set_mod for same identity then remove the existing entry */
  list = mct_list_find_custom(mod_private->module_type_list, &identity,
    pproc_module_check_identity);

  if (list) {
    pproc_type = list->data;
    mod_private->module_type_list =
      mct_list_remove(mod_private->module_type_list, pproc_type);
    free(pproc_type);
  }

  if (MCT_MODULE_FLAG_INVALID == module_type) {
    MCT_OBJECT_UNLOCK(module);
    return;
  }

  pproc_type = CAM_MALLOC(sizeof(pproc_module_type_t));
  if (!pproc_type) {
    PP_ERR("error in memory allocation\n");
    MCT_OBJECT_UNLOCK(module);
    return;
  }

  pproc_type->identity = identity;
  pproc_type->module_type = module_type;

  list = mct_list_append(mod_private->module_type_list, pproc_type, NULL, NULL);
  if (!list) {
    PP_ERR("error appending in module type list\n");
    free(pproc_type);
    MCT_OBJECT_UNLOCK(module);
    return;
  }

  mod_private->module_type_list = list;
  mct_module_add_type(module, module_type, identity);
  MCT_OBJECT_UNLOCK(module);

  PP_LOW("X\n");
  return;
}

/** pproc_module_query_mod
 *     @module:     pproc module itself ("pproc")
 *     @query_buf:  media controller's query information buffer
 *     @sessionid:  session and stream identity
 *
 *  pproc module's capability is based on the sub-modules and
 *  independant of the session id. This function is used to
 *  query submodule capability by simply pass the query buffer
 *  to sub-modules so that the can be filled out accordingly.
 *
 *  Return Sub-module's query result
 **/
static boolean pproc_module_query_mod(mct_module_t *module,
  void *buf, uint32_t sessionid)
{
  pproc_module_private_t *mod_private;
  boolean                 rc = FALSE;
  mct_pipeline_cap_t     *query_buf = (mct_pipeline_cap_t *)buf;

  PP_LOW("E\n");
  if (!module || !query_buf || strcmp(MCT_OBJECT_NAME(module), "pproc")) {
    PP_ERR("error module: %p query_buf: %p\n", module, query_buf);
    return rc;
  }

  MCT_OBJECT_LOCK(module);
  mod_private = (pproc_module_private_t *)MCT_OBJECT_PRIVATE(module);

  /* TODO: We should probably independently store the sub-mods caps and give
     the aggregate caps to media-controller */

  memset(&query_buf->pp_cap, 0, sizeof(mct_pipeline_pp_cap_t));

  CALL_SUBMOD_FUNCTION(mod_private, query_mod, query_buf, sessionid);

  rc = TRUE;

query_mod_done:
  MCT_OBJECT_UNLOCK(module);
  PP_LOW("feature_mask 0x%llx X\n", query_buf->pp_cap.feature_mask);
  return rc;
}

/** pproc_module_deinit
 *    @mod: PPROC module object
 *
 *  Function for MCT to deinit PPROC module. This will remove
 *  all the ports of this module. Also call deinit for the
 *  submods.
 *
 *  Return: NULL
 **/
void pproc_module_deinit(mct_module_t *module)
{
  pproc_module_private_t *mod_private;
  uint32_t                i;
  uint32_t                j, list_cnt;

  PP_LOW("E\n");
  if (!module || strcmp(MCT_OBJECT_NAME(module), "pproc"))
    return;

  mod_private = (pproc_module_private_t *)MCT_OBJECT_PRIVATE(module);
  list_cnt = sizeof(pproc_modules_list) / sizeof(pproc_modules_list[0]);
  for (i = 0; i < mod_private->num_sub_mods; i++) {
    if (mod_private->sub_mods[i]) {
      for (j = 0; j < list_cnt; j++) {
        if (!strncmp(MCT_OBJECT_NAME(mod_private->sub_mods[i]),
          pproc_modules_list[j].name,
          strlen(pproc_modules_list[j].name))) {
          pproc_modules_list[j].deinit_mod(mod_private->sub_mods[i]);
        }
      }
      mod_private->sub_mods[i] = NULL;
    }
  }
  free(mod_private->sub_mods);

  mct_list_free_all(MCT_MODULE_SRCPORTS(module), pproc_module_free_port);
  mct_list_free_all(MCT_MODULE_SINKPORTS(module),pproc_module_free_port);
  /* TODO: Modules children is port which is deleted above ! Is there a
     consideration to make the streams as children of PPROC */
  //mct_list_free_list(MCT_MODULE_CHILDREN(module));

  mct_list_free_all(mod_private->module_type_list, pproc_module_free_type_list);

  free(mod_private);
  mct_module_destroy(module);

  PP_LOW("X\n");
  return;
}

/** pproc_module_create_default_ports
 *    @pproc: this pproc module object
 *
 *  By default, create 2 sink ports and 2 source ports, because considering
 *  extreme use case - live snapshot, in which there are 2 streaming inputs
 *  (with one buffer) goes into one port and 1 streaming input
 *  goes into another port per session.
 *
 *  These ports should map to internal CPP/VPE/C2D sub-modules'
 *  correspoinding ports.
 *
 *  Return TRUE on success.
 **/
static boolean pproc_module_create_default_ports(mct_module_t *module)
{
  boolean     rc = TRUE;
  mct_port_t *port;
  int         i, j;
  char        port_name[32];
  char        port_direction[2][32] = {{"pproc_sink_"}, {"pproc_source_"}};

  PP_LOW("E\n");

  for (j = 0; j < 2; j++) {
    for (i = 0; i < PPROC_MAX_PORTS; i++) {
      snprintf(port_name, sizeof(port_name), "%s%d", port_direction[j], i);
      port = pproc_port_init(port_name);
      if (port) {
        if (mct_module_add_port(module, port) == FALSE) {
          PP_ERR("%s add failed\n", port_name);
          goto create_error;
        }
      } else {
        PP_ERR("%s init failed\n", port_name);
        goto create_error;
      }
    }
  }

  PP_LOW("X\n");
  return TRUE;

create_error:
  mct_list_free_all(MCT_MODULE_SRCPORTS(module), pproc_module_free_port);
  mct_list_free_all(MCT_MODULE_SINKPORTS(module),pproc_module_free_port);
  PP_LOW("X\n");
  return FALSE;
}

static boolean pproc_module_set_session_data(mct_module_t *module,
  void *set_buf, uint32_t sessionid)
{
  pproc_module_private_t *mod_private;
  boolean                 rc = FALSE;

  PP_LOW("E\n");
  if (!module || strcmp(MCT_OBJECT_NAME(module), "pproc")) {
    PP_ERR("error module: %p \n", module);
    return rc;
  }

  MCT_OBJECT_LOCK(module);
  mod_private = (pproc_module_private_t *)MCT_OBJECT_PRIVATE(module);


  CALL_SUBMOD_FUNCTION(mod_private, set_session_data, set_buf, sessionid);

  rc = TRUE;

set_session_data_done:
  MCT_OBJECT_UNLOCK(module);
  PP_LOW("X\n");
  return rc;
}

/** pproc_module_init:
 *    @name: name of this pproc interface module("pproc").
 *
 *  pproc interface module initializtion entry point, it only
 *  creates pproc module and initialize its sub-modules(cpp/vpe,
 *  c2d, s/w scaling modules etc.). pproc should also initialize
 *  its sink and source ports which map to its sub-modules ports.
 *
 *  Return: pproc module object if success
 **/
mct_module_t* pproc_module_init(const char *name)
{
  mct_module_t           *pproc;
  mct_port_t             *port;
  pproc_module_private_t *mod_private;
  uint32_t                 i, j, list_cnt;
  uint32_t                num_sub_mods = 0;

  PP_LOW("E\n");
  if (strcmp(name, "pproc")) {
    PP_ERR("invalid module name\n");
    return NULL;
  }

  pproc = mct_module_create("pproc");
  if (!pproc) {
    PP_ERR("error module create failed\n");
    return NULL;
  }

  mod_private = CAM_MALLOC(sizeof(pproc_module_private_t));
  if (mod_private == NULL) {
    PP_ERR("error because private data alloc failed\n");
    goto private_error;
  }

  memset(mod_private, 0 ,sizeof(pproc_module_private_t));

  list_cnt = sizeof(pproc_modules_list) / sizeof(pproc_modules_list[0]);
  mod_private->sub_mods =
    (mct_module_t **)CAM_MALLOC(list_cnt * sizeof(mct_module_t *));
  if (!mod_private->sub_mods) {
    PP_ERR("error allocating sub_mods array\n");
    goto sub_modue_error;
  }
  memset(mod_private->sub_mods, 0 ,list_cnt * sizeof(mct_module_t *));
  for (i = 0; i < list_cnt; i++) {
    mod_private->sub_mods[num_sub_mods] =
      pproc_modules_list[i].init_mod(pproc_modules_list[i].name);
    if (mod_private->sub_mods[num_sub_mods]) {
      /* MCT can provide a generic container/parent module interface */
      if (!strcmp(MCT_OBJECT_NAME(mod_private->sub_mods[num_sub_mods]),
        "wnr")) {
        module_wnr_set_parent(mod_private->sub_mods[num_sub_mods], pproc);
      } else if (!strcmp(MCT_OBJECT_NAME(mod_private->sub_mods[num_sub_mods]),
        "cac")) {
        module_cac_set_parent(mod_private->sub_mods[num_sub_mods], pproc);
      } else if (!strcmp(MCT_OBJECT_NAME(mod_private->sub_mods[num_sub_mods]),
        "paaf")) {
         module_paaf_set_parent(mod_private->sub_mods[num_sub_mods], pproc);
      } else if (!strcmp(MCT_OBJECT_NAME(mod_private->sub_mods[num_sub_mods]),
        "tmod")) {
         base_module_set_parent(mod_private->sub_mods[num_sub_mods], pproc);
      } else if (!strcmp(MCT_OBJECT_NAME(mod_private->sub_mods[num_sub_mods]),
        "ezt")) {
         base_module_set_parent(mod_private->sub_mods[num_sub_mods], pproc);
      } else if (!strcmp(MCT_OBJECT_NAME(mod_private->sub_mods[num_sub_mods]),
         "llvd")) {
         module_llvd_set_parent(mod_private->sub_mods[num_sub_mods], pproc);
      } else if (!strcmp(MCT_OBJECT_NAME(mod_private->sub_mods[num_sub_mods]),
        "cpp")) {
        module_cpp_set_parent(mod_private->sub_mods[num_sub_mods], pproc);
      } else if  (!strcmp(MCT_OBJECT_NAME(mod_private->sub_mods[num_sub_mods]),
        "sw_tnr")) {
        module_sw_tnr_set_parent(mod_private->sub_mods[num_sub_mods], pproc);
      } else if  (!strcmp(MCT_OBJECT_NAME(mod_private->sub_mods[num_sub_mods]),
        "c2d") || !strcmp(MCT_OBJECT_NAME(mod_private->sub_mods[num_sub_mods]),
        "vpe") || !strcmp(MCT_OBJECT_NAME(mod_private->sub_mods[num_sub_mods]),
        "vpu")) {
        /* Skip set parent update */
      } else if (!strcmp(MCT_OBJECT_NAME(mod_private->sub_mods[num_sub_mods]),
        "ppeiscore")) {
        module_ppeiscore_set_parent(mod_private->sub_mods[num_sub_mods], pproc);
      } else if (!strcmp(MCT_OBJECT_NAME(mod_private->sub_mods[num_sub_mods]),
        "depth_map")) {
        module_depth_map_set_parent(mod_private->sub_mods[num_sub_mods], pproc);
      } else if (!strcmp(MCT_OBJECT_NAME(mod_private->sub_mods[num_sub_mods]),
        "mfproc_precpp")) {
        module_mfproc_precpp_set_parent(mod_private->sub_mods[num_sub_mods], pproc);
      } else { /* common for all sub-modules derived from imgbase */
        module_imgbase_set_parent(mod_private->sub_mods[num_sub_mods], pproc);
      }
      num_sub_mods++;
    }
  }
  mod_private->num_sub_mods = num_sub_mods;
  PP_HIGH("num_sub_mods:%d\n", mod_private->num_sub_mods);
  MCT_OBJECT_PRIVATE(pproc) = mod_private;

  /* create pproc's stream/capture sinkports and source ports */
  if (pproc_module_create_default_ports(pproc) == FALSE) {
    PP_ERR("error in default port creation\n");
    goto port_create_error;
  }

  mct_module_set_set_mod_func(pproc, pproc_module_set_mod);
  mct_module_set_query_mod_func(pproc, pproc_module_query_mod);
  mct_module_set_start_session_func(pproc, pproc_module_start_session);
  mct_module_set_stop_session_func(pproc, pproc_module_stop_session);
  mct_module_set_request_new_port_func(pproc, pproc_module_request_new_port);
  mct_module_set_process_event_func(pproc, pproc_module_process_event);

  pproc->set_session_data = pproc_module_set_session_data;

  PP_LOW("X\n");
  return pproc;

port_create_error:

  for (i = 0; i < mod_private->num_sub_mods; i++) {
    if (mod_private->sub_mods[i]) {
      for (j = 0; j < list_cnt; j++) {
        if (!strncmp(MCT_OBJECT_NAME(mod_private->sub_mods[i]),
          pproc_modules_list[j].name,
          strlen(pproc_modules_list[j].name))) {
          pproc_modules_list[j].deinit_mod(mod_private->sub_mods[i]);
        }
      }
      mod_private->sub_mods[i] = NULL;
    }
  }
  free(mod_private->sub_mods);

  mct_list_free_all(MCT_MODULE_SRCPORTS(pproc), pproc_module_free_port);
  mct_list_free_all(MCT_MODULE_SINKPORTS(pproc),pproc_module_free_port);
  /* TODO: Modules children is port which is deleted above !*/
  //mct_list_free_list(MCT_MODULE_CHILDREN(pproc));
sub_modue_error:
  free(mod_private);
private_error:
  mct_module_destroy(pproc);
  PP_LOW("X\n");
  return NULL;
}
