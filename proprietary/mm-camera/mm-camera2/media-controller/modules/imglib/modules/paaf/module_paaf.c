/***************************************************************************
* Copyright (c) 2016 Qualcomm Technologies, Inc.                           *
* All Rights Reserved.                                                     *
* Confidential and Proprietary - Qualcomm Technologies, Inc.               *
***************************************************************************/
#include "module_paaf.h"

/** PAAF_FILTER_TYPE:
 *
 *  @in_type: af_sw_filter_type
 *
 *   Returns corresponding PAAF filter type
 */
#define PAAF_FILTER_TYPE(in_type) ({ \
  paaf_sw_filter_type_t filter; \
  if (in_type == AFS_ON_FIR)  \
    filter = PAAF_ON_FIR; \
  else if (in_type == AFS_ON_IIR) \
    filter = PAAF_ON_IIR; \
  else  \
    filter = PAAF_OFF; \
  filter; \
}) \

/**
 * PAAF_INCREASE_ROI_SIZE
 *
 *   @roi - PAAF ROI coordinates
 *   @length - New ROI's length and width
 *
 * Increase the size of the ROI
 **/
#define PAAF_INCREASE_ROI_SIZE(roi, length) ({\
  (roi)->pos.x = (roi)->pos.x - ((length - (roi)->size.width) / 2); \
  (roi)->pos.y = (roi)->pos.y - ((length - (roi)->size.height) / 2); \
  (roi)->size.width = length; \
  (roi)->size.height = length; \
})\

/**
 *  Static functions
 **/
static int32_t module_paaf_client_created(imgbase_client_t *p_client);
static int32_t module_paaf_client_destroy(imgbase_client_t *p_client);
static boolean module_paaf_query_mod(mct_pipeline_cap_t *p_mct_cap,
  void* p_mod,unsigned int sessionid);
static int32_t module_paaf_update_meta(imgbase_client_t *p_client,
  img_meta_t *p_meta);
static boolean module_paaf_handle_set_stream_config(uint32_t identity,
  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_core_ops, boolean *is_evt_handled);
static boolean module_paaf_handle_af_config(uint32_t identity,
  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_core_ops, boolean *is_evt_handled);
static boolean module_paaf_handle_stream_crop_event(uint32_t identity,
  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_core_ops, boolean *is_evt_handled);
static boolean module_paaf_handle_af_tune_ptr_event(uint32_t identity,
  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_core_ops, boolean *is_evt_handled);
static int32_t module_paaf_handle_meta_stream_info(mct_event_control_parm_t
  *p_ctrl_parm, imgbase_client_t *p_client, img_core_ops_t *p_core_ops);
static int module_paaf_event_handler(imgbase_client_t *p_client,
  img_event_t *p_event);


/**
 * img_paaf_defs
 *
 *  PAAF stream config definition. Streams to process
 *  are chosen based on present streams described in streams
 *  Arrays.
 **/
static const img_paaf_defs_t paaf_defs[] = {
  {
    .streams = {CAM_STREAM_TYPE_PREVIEW, CAM_STREAM_TYPE_MAX},
    .processing_stream = CAM_STREAM_TYPE_PREVIEW,
    .streams_to_process = IMG_2_MASK(CAM_STREAM_TYPE_PREVIEW),
  },
  {
    .streams = {CAM_STREAM_TYPE_ANALYSIS, CAM_STREAM_TYPE_MAX},
    .processing_stream = CAM_STREAM_TYPE_ANALYSIS,
    .streams_to_process = IMG_2_MASK(CAM_STREAM_TYPE_ANALYSIS),
  },
  {
    .streams = {CAM_STREAM_TYPE_CALLBACK, CAM_STREAM_TYPE_MAX},
    .processing_stream = CAM_STREAM_TYPE_CALLBACK,
    .streams_to_process = IMG_2_MASK(CAM_STREAM_TYPE_CALLBACK),
  },
  {
    .streams = {CAM_STREAM_TYPE_PREVIEW, CAM_STREAM_TYPE_VIDEO,
      CAM_STREAM_TYPE_MAX},
    .processing_stream = CAM_STREAM_TYPE_PREVIEW,
    .streams_to_process = IMG_2_MASK(CAM_STREAM_TYPE_PREVIEW) |
                          IMG_2_MASK(CAM_STREAM_TYPE_VIDEO),
  },
  {
    .streams = {CAM_STREAM_TYPE_PREVIEW, CAM_STREAM_TYPE_ANALYSIS,
      CAM_STREAM_TYPE_MAX},
    .processing_stream = CAM_STREAM_TYPE_ANALYSIS,
    .streams_to_process = IMG_2_MASK(CAM_STREAM_TYPE_ANALYSIS),
  },
  {
    .streams = {CAM_STREAM_TYPE_PREVIEW, CAM_STREAM_TYPE_CALLBACK,
      CAM_STREAM_TYPE_MAX},
    .processing_stream = CAM_STREAM_TYPE_PREVIEW,
    .streams_to_process = IMG_2_MASK(CAM_STREAM_TYPE_PREVIEW),
  },
};

/** g_caps:
 *
 *  Set the capabilities for PAAF module
*/
static img_caps_t g_caps = {
  .num_input = 1,
  .num_output = 0,
  .num_meta = 1,
  .inplace_algo = 0,
  .num_release_buf = 0,
  .preload_mode = IMG_PRELOAD_PER_SESSION_MODE,
};

/** g_params:
 *
 *  imgbase parameters
 **/
static module_imgbase_params_t g_params = {
  .imgbase_client_created = module_paaf_client_created,
  .imgbase_client_destroy = module_paaf_client_destroy,
  .imgbase_query_mod = module_paaf_query_mod,
  .imgbase_client_update_meta = module_paaf_update_meta,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_SET_STREAM_CONFIG] =
    module_paaf_handle_set_stream_config,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_IMGLIB_AF_CONFIG] =
    module_paaf_handle_af_config,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_STREAM_CROP] =
    module_paaf_handle_stream_crop_event,
  .imgbase_handle_module_event[MCT_EVENT_MODULE_SET_AF_TUNE_PTR] =
    module_paaf_handle_af_tune_ptr_event,
  .imgbase_handle_ctrl_parm[CAM_INTF_META_STREAM_INFO] =
    module_paaf_handle_meta_stream_info,
  .imgbase_client_event_handler[QIMG_EVT_META_BUF_DONE] =
    module_paaf_event_handler,
  .exec_mode = IMG_EXECUTION_SW,
  .access_mode = IMG_ACCESS_READ,
  .force_cache_op = FALSE,
};

/**
 * Function: module_paaf_default_roi_to_center
 *
 * Description: In error case, default PAAF ROI to center of
 *              output frame
 *
 * Arguments:
 *   @roi - PAAF ROI coords
 *   @width - Processing stream width
 *   @height - Processing stream height
 *   @h_scale - af tuning h_clip ratio
 *   @v_scale - af tuning v_clip ratio
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
static inline void module_paaf_default_roi_to_center(
  img_rect_t *roi, int width, int height, float h_scale, float v_scale)
{
  roi->size.width = (int)((float)width * h_scale);
  roi->size.height = (int)((float)width * v_scale);
  roi->pos.x = (width - roi->size.width) / 2;
  roi->pos.y = (height - roi->size.height) / 2;

  IDBG_MED("[PAAF_DBG] center roi (%d %d %d %d), "
    "h_scale %f, v_scale %f", roi->pos.x, roi->pos.y, roi->size.width,
    roi->size.height, h_scale, v_scale);
}

/**
 * Function: module_paaf_client_fix_out_of_boundary
 *
 * Description: If the ROI is partly out of boundary,
 *      translate x and y coords to within the processing frame.
 *      Will still need to do a boundary check after this
 *      tranlsation and reject the ROI if it
 *      cannot be contained in the processing frame.
 *
 * Arguments:
 *   @p_rect - PAAF input ROI cordinates
 *   @width - Processing frame width
 *   @height - Processing frame height
 *
 * Return values:
 *   TRUE/FALSE
 *
 * Notes: none
 **/
static inline boolean module_paaf_fix_out_of_boundary(
  img_rect_t *p_rect, int width, int height)
{
  if ((p_rect->pos.x > width) || (p_rect->pos.y > height) ||
    ((p_rect->pos.x + p_rect->size.width) < 0) ||
    ((p_rect->pos.y + p_rect->size.height) < 0)) {
    //ROI window completely out of boundary
    return FALSE;
  }

  if (p_rect->pos.x < 0) {
    p_rect->pos.x = 0;
  }

  if (p_rect->pos.y < 0) {
    p_rect->pos.y = 0;
  }

  if ((p_rect->pos.x + p_rect->size.width) >= width) {
    p_rect->pos.x = width - p_rect->size.width - 1;
  }

  if ((p_rect->pos.y + p_rect->size.height) >= height) {
    p_rect->pos.y = height - p_rect->size.height - 1;
  }
  return TRUE;
}

/**
 * Function: module_paaf_get_skip
 *
 * Description: This function is to get the skip count
 *
 * Arguments:
 *   @width: image width
 *   @height: image height
 *   @coeff_len: determines filter order
 *
 * Return values:
 *     skip count
 *
 * Notes: none
 **/
int32_t module_paaf_get_skip(uint32_t width, uint32_t height,
  uint32_t coeff_len)
{
  /* if coeff_len is 3, lower order filter is used
     and pixel skip is not needed*/
  if (coeff_len == 3) {
    return 1;
  }
  float threshold = 304.0;
  int32_t num_skip = (int32_t)(MAX(ceil((float)width / threshold),
    ceil((float)height / threshold)));

  return num_skip;
}

/**
 * Function: module_paaf_set_frame_crop
 *
 * Description: Set the roi for frame crop
 *
 * Arguments:
 *   @cropped_window - to store the cropped coords
 *   @p_orig_roi - PAAF roi cordinates
 *   @width - Processing frame width
 *   @height - Processing frame height
 *   @pixel_skip - number of pixels to skip. 1 = no skip
 *
 * Return values:
 *   TRUE/FALSE
 *
 * Notes: none
 **/
static inline boolean module_paaf_set_frame_crop(
  img_rect_t *cropped_window, img_rect_t *p_orig_roi,
  int width, int height, int pixel_skip)
{
  if (p_orig_roi->size.width == 0 || p_orig_roi->size.height == 0) {
    return FALSE;
  }

  cropped_window->pos.x = p_orig_roi->pos.x - PAAF_ROI_BOUNDARY_PIXELS;
  cropped_window->pos.y = p_orig_roi->pos.y - PAAF_ROI_BOUNDARY_PIXELS;
  cropped_window->size.width =
    p_orig_roi->size.width + (2 * PAAF_ROI_BOUNDARY_PIXELS);
  cropped_window->size.height =
    p_orig_roi->size.height + (2 * PAAF_ROI_BOUNDARY_PIXELS);

  p_orig_roi->pos.x = PAAF_ROI_BOUNDARY_PIXELS / pixel_skip;
  p_orig_roi->pos.y = PAAF_ROI_BOUNDARY_PIXELS / pixel_skip;
  p_orig_roi->size.width = p_orig_roi->size.width / pixel_skip;
  p_orig_roi->size.height = p_orig_roi->size.height / pixel_skip;

  if (!module_paaf_fix_out_of_boundary(cropped_window, width, height)) {
    return FALSE;
  }

  return TRUE;
}

/**
 * Function: module_paaf_client_created
 *
 * Description: function called after client creation
 *
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t module_paaf_client_created(imgbase_client_t *p_client)
{
  module_imgbase_t *p_mod = (module_imgbase_t *)p_client->p_mod;
  img_paaf_module_t *p_paafmod = (img_paaf_module_t *)p_mod->mod_private;
  img_paaf_client_t *p_paaf_client;

  // Create PAAF private client and set default params
  p_paaf_client = calloc(1, sizeof(img_paaf_client_t));
  if (!p_paaf_client) {
    IDBG_ERROR("Failed, PAAf client is NULL");
    return IMG_ERR_NO_MEMORY;
  }

  p_paaf_client->p_session_data =
    &p_paafmod->session_data[p_client->session_id - 1];
  p_client->p_private_data = p_paaf_client;
  p_paaf_client->p_client = p_client;
  p_paaf_client->use_af_tuning_trans = FALSE;

  // Set default params for imglib base client
  p_client->before_cpp = TRUE;
  p_client->processing_disabled = TRUE;
  p_client->feature_mask = CAM_QCOM_FEATURE_PAAF;
  p_client->process_all_frames = TRUE;

  // Update streams_to_process mask in the client
  p_client->streams_to_process =
    p_paaf_client->p_session_data->streams_to_process;

  return IMG_SUCCESS;
}

/**
 * Function: module_paaf_client_destroy
 *
 * Description: function called before client is destroyed
 *
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_GENERAL
 *
 * Notes: none
 **/
int32_t module_paaf_client_destroy(imgbase_client_t *p_client)
{
  IDBG_MED(":E");
  if (p_client->p_private_data) {
    free(p_client->p_private_data);
    p_client->p_private_data = NULL;
  }
  return IMG_SUCCESS;
}

/**
 * Function: module_paaf_query_mod
 *
 * Description: This function is used to query PAAF caps
 *
 * Arguments:
 *   @p_mct_cap - capababilities
 *   @p_mod - pointer to the module
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
boolean module_paaf_query_mod(mct_pipeline_cap_t *p_mct_cap, void* p_mod,unsigned int sessionid)
{
  mct_pipeline_imaging_cap_t *p_cap = NULL;
  mct_pipeline_sensor_cap_t *p_sensor_cap = NULL;
  cam_analysis_info_t *p_analysis_info;

  IMG_UNUSED(p_mod);
  IMG_UNUSED(sessionid);
  if (!p_mct_cap) {
    IDBG_ERROR("Error Input");
    return FALSE;
  }

  p_cap = &p_mct_cap->imaging_cap;
  p_sensor_cap = &p_mct_cap->sensor_cap;
  p_cap->feature_mask |= CAM_QCOM_FEATURE_PAAF;
  p_analysis_info = &p_cap->analysis_info[CAM_ANALYSIS_INFO_PAAF];

  // Fill Analysis information for PAAF
  if (p_sensor_cap->color_arrangement == CAM_FILTER_ARRANGEMENT_Y) {
    p_analysis_info->analysis_max_res.width =
      PAAF_RECOMMENDED_ANALYSIS_WIDTH;
    p_analysis_info->analysis_max_res.height =
      PAAF_RECOMMENDED_ANALYSIS_HEIGHT;
    p_analysis_info->analysis_recommended_res.width =
      PAAF_RECOMMENDED_ANALYSIS_WIDTH;
    p_analysis_info->analysis_recommended_res.height =
      PAAF_RECOMMENDED_ANALYSIS_HEIGHT;
    p_analysis_info->analysis_padding_info.width_padding = CAM_PAD_TO_16;
    p_analysis_info->analysis_padding_info.height_padding = CAM_PAD_TO_8;
    p_analysis_info->analysis_padding_info.plane_padding = CAM_PAD_TO_16;
    p_analysis_info->analysis_padding_info.min_stride =
      PAAF_RECOMMENDED_ANALYSIS_WIDTH;
    p_analysis_info->analysis_padding_info.min_scanline =
      PAAF_RECOMMENDED_ANALYSIS_HEIGHT;
    p_analysis_info->analysis_padding_info.offset_info.offset_x = 0;
    p_analysis_info->analysis_padding_info.offset_info.offset_y = 0;
    p_analysis_info->hw_analysis_supported = FALSE;
    p_analysis_info->analysis_format = CAM_FORMAT_Y_ONLY;
    p_analysis_info->valid = TRUE;
  } else {
    p_analysis_info->valid = FALSE;
  }

  return TRUE;
}

/**
 * Function: module_paaf_handle_stream_crop_event
 *
 * Description: This function handles Af config module event
 *
 * Arguments:
 *   p_mod_event - pointer of module event
 *   p_client - pointer to imgbase client
 *   p_core_ops - pointer to imgbase module ops
 *
 * Return values:
 *     TRUE/FALSE
 *
 * Notes: none
 **/
boolean module_paaf_handle_stream_crop_event(uint32_t identity,
  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_core_ops, boolean *is_evt_handled)
{
  img_paaf_client_t *p_paaf_client;
  mct_bus_msg_stream_crop_t *p_s_crop;
  img_rect_t *p_crop_info;
  img_trans_info_t *p_camif_trans_info;
  mct_stream_info_t *isp_out_dim;
  boolean fwd_event = TRUE;

  if (!p_mod_event || !p_client || !p_core_ops) {
    IDBG_ERROR("Error input");
    return fwd_event;
  }

  p_s_crop = (mct_bus_msg_stream_crop_t *)p_mod_event->module_event_data;
  if (NULL == p_s_crop) {
    IDBG_ERROR("Error Stream crop is NULL");
    return fwd_event;
  }

  p_paaf_client = (img_paaf_client_t *)p_client->p_private_data;
  p_crop_info = &p_paaf_client->crop_info;
  p_camif_trans_info = &p_paaf_client->camif_trans_info;
  isp_out_dim = &p_client->isp_output_dim_stream_info;


  //Update crop Info
  if (!p_s_crop->crop_out_x || !p_s_crop->crop_out_y) {
    p_crop_info->pos.x = 0;
    p_crop_info->pos.y = 0;
    p_crop_info->size.width = isp_out_dim->dim.width;
    p_crop_info->size.height = isp_out_dim->dim.height;
  } else {
    p_crop_info->pos.x = (int32_t)p_s_crop->x;
    p_crop_info->pos.y = (int32_t)p_s_crop->y;
    p_crop_info->size.width = (int32_t)p_s_crop->crop_out_x;
    p_crop_info->size.height = (int32_t)p_s_crop->crop_out_y;
  }

  IDBG_MED("Updated Crop Info (%d %d %d %d)",
    p_paaf_client->crop_info.pos.x, p_paaf_client->crop_info.pos.y,
    p_paaf_client->crop_info.size.width, p_paaf_client->crop_info.size.height);

  //Update translation info
  IDBG_MED("Recieved Map(%d %d %d %d)",
    p_s_crop->width_map, p_s_crop->height_map,
    p_s_crop->x_map, p_s_crop->y_map);

  if ((p_s_crop->width_map > 0) && (p_s_crop->height_map > 0)) {
    p_paaf_client->use_af_tuning_trans = FALSE;
    p_camif_trans_info->h_scale = (float)isp_out_dim->dim.width /
      (float)p_s_crop->width_map;
    p_camif_trans_info->v_scale = (float)isp_out_dim->dim.height /
      (float)p_s_crop->height_map;
  } else {
    p_paaf_client->use_af_tuning_trans = TRUE;
  }
  p_camif_trans_info->h_offset = p_s_crop->x_map;
  p_camif_trans_info->v_offset = p_s_crop->y_map;

  IDBG_MED("Updated Camif translation h_scale %f, v_scale %f,"
    "h_offset %d, v_offset %d", p_camif_trans_info->h_scale,
    p_camif_trans_info->v_scale, p_camif_trans_info->h_offset,
    p_camif_trans_info->v_offset);

  return fwd_event;
}

/**
 * Function: module_paaf_get_paaf_streams
 *
 * Description: Helper function to find streams that have PAAF
 * enabled
 *
 * Arguments:
 * @meta_stream_info - pointer to meta stream info
 * @paaf_streams - Streams on which PAAF feature mask is set
 *
 *
 * Return values:
 *     Number of streams with PAAF enabled
 *
 * Notes: none
 **/
uint32_t module_paaf_get_paaf_streams(cam_stream_size_info_t *meta_stream_info,
  cam_stream_type_t *paaf_streams)
{
  uint32_t meta_idx, i = 0;

  IDBG_LOW("Meta str info: num of streams %d", meta_stream_info->num_streams);
  for (meta_idx = 0; meta_idx < meta_stream_info->num_streams; meta_idx++) {
    if (meta_stream_info->postprocess_mask[meta_idx] & CAM_QCOM_FEATURE_PAAF) {
      paaf_streams[i] = meta_stream_info->type[meta_idx];
      i++;
    }
  }
  //Append CAM_STREAM_TYPE_MAX
  paaf_streams[i] = CAM_STREAM_TYPE_MAX;

  return i+1;
}
/**
 * Function: module_paaf_handle_meta_stream_info
 *
 * Description: This function handles the Meta stream Info Ctrl
 * param
 *
 * Arguments:
 *   p_mod_event - pointer of module event
 *   p_client - pointer to imgbase client
 *   p_core_ops - pointer to imgbase module ops
 *
 * Return values:
 *     TRUE/FALSE
 *
 * Notes: none
 **/
static int32_t module_paaf_handle_meta_stream_info(mct_event_control_parm_t
  *p_ctrl_parm, imgbase_client_t *p_client, img_core_ops_t *p_core_ops)
{
  img_paaf_client_t *p_paaf_client;
  cam_stream_size_info_t *meta_stream_info;
  uint32_t def_idx, paaf_idx, chosen_idx = 0, paaf_str_cnt;
  int current_chosen_cnt = 0, stream_match_cnt, i;
  cam_stream_type_t paaf_streams[CAM_STREAM_TYPE_MAX];
  cam_stream_type_t *p_paaf_streams;
  int rc = IMG_ERR_EAGAIN;

  if (!p_ctrl_parm || !p_client || !p_core_ops) {
    IDBG_ERROR(" Error input");
    return IMG_ERR_INVALID_INPUT;
  }

  meta_stream_info = (cam_stream_size_info_t *)(p_ctrl_parm->parm_data);
  if (!meta_stream_info) {
    IDBG_ERROR("Meta stream info is NULL");
    return IMG_ERR_INVALID_INPUT;
  }

  p_paaf_client = (img_paaf_client_t *)p_client->p_private_data;

  IDBG_MED("Meta str info: num of streams %d", meta_stream_info->num_streams);

  //Get PAAF enabled streams
  p_paaf_streams = &paaf_streams[0];
  paaf_str_cnt = module_paaf_get_paaf_streams(meta_stream_info,
    p_paaf_streams);

  //Find the streams config matching from the PAAF defs
  for (def_idx = 0; def_idx < IMGLIB_ARRAY_SIZE(paaf_defs); def_idx++) {
    stream_match_cnt = 0;
    for (paaf_idx = 0; paaf_idx < paaf_str_cnt; paaf_idx++) {
      for (i = 0; i < CAM_STREAM_TYPE_MAX; i++) {
        if (paaf_defs->streams[i] == CAM_STREAM_TYPE_MAX) {
          break;
        }
        if (paaf_defs[def_idx].streams[i] == paaf_streams[paaf_idx]){
          stream_match_cnt++;
          break;
        }
      }
    }
    if (current_chosen_cnt < stream_match_cnt) {
      current_chosen_cnt = stream_match_cnt;
      chosen_idx = def_idx;
    }
  }

  //Update streams to process and processing stream
  if (current_chosen_cnt) {
    p_paaf_client->p_session_data->processing_stream =
      paaf_defs[chosen_idx].processing_stream;
    p_paaf_client->p_session_data->streams_to_process =
      paaf_defs[chosen_idx].streams_to_process;

    //Override the streams_to_process in the p_client since the session stream
    //client is created before meta stream info is recieved
    p_client->streams_to_process = paaf_defs[chosen_idx].streams_to_process;
  } else {
    //If PAAF is not enabled on any stream, set processing stream to
    //CAM_STREAM_TYPE_MAX
    p_paaf_client->p_session_data->processing_stream =
      CAM_STREAM_TYPE_MAX;
    p_paaf_client->p_session_data->streams_to_process =
      IMG_2_MASK(CAM_STREAM_TYPE_MAX);
    p_client->streams_to_process =
      p_paaf_client->p_session_data->streams_to_process;
    if (meta_stream_info->num_streams > 0) {
      IDBG_MED("PAAf not enabled on any streams");
    }
  }

  IDBG_MED("Processing stream type %d",
    p_paaf_client->p_session_data->processing_stream,
    p_client->streams_to_process);

  return rc;
}

/**
 * Function: module_paaf_handle_af_tune_ptr_event
 *
 * Description: This function handles Af tune ptr module event
 *
 * Arguments:
 *   p_mod_event - pointer of module event
 *   p_client - pointer to imgbase client
 *   p_core_ops - pointer to imgbase module ops
 *
 * Return values:
 *     TRUE/FALSE
 *
 * Notes: none
 **/
boolean module_paaf_handle_af_tune_ptr_event(uint32_t identity,
  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_core_ops, boolean *is_evt_handled)
{
  img_paaf_client_t *p_paaf_client;
  af_algo_tune_parms_t *p_tuning_info;
  img_paaf_session_data_t *p_session_data;
  boolean fwd_event = TRUE;

  if (!p_mod_event || !p_client || !p_core_ops) {
    IDBG_ERROR("Error input");
    return fwd_event;
  }

  p_tuning_info = (af_algo_tune_parms_t *)p_mod_event->module_event_data;
  if (NULL == p_tuning_info) {
    IDBG_ERROR("Error AF Tuning PTR is NULL");
    return fwd_event;
  }

  //Get Session Data
  p_paaf_client = (img_paaf_client_t *)p_client->p_private_data;
  p_session_data = p_paaf_client->p_session_data;

  p_session_data->af_tuning_trans_info.h_scale =
    p_tuning_info->af_vfe[0].config.h_clip_ratio_normal_light;
  p_session_data->af_tuning_trans_info.v_scale =
    p_tuning_info->af_vfe[0].config.v_clip_ratio_normal_light;

  IDBG_LOW("AF Turning PTR update event, h_clip_ratio %f, v_clip_ratio %f",
    p_tuning_info->af_vfe[0].config.h_clip_ratio_normal_light,
    p_tuning_info->af_vfe[0].config.v_clip_ratio_normal_light);

  return fwd_event;

}

/**
 * Function: module_paaf_update_af_config
 *
 * Description: This function updates the latest AF config
 *
 * Arguments:
 *   p_client - pointer to imgbase client
  *
 * Return values:
 *     None
 *
 * Notes: none
 **/
void module_paaf_update_af_config(imgbase_client_t *p_client,
  mct_imglib_swaf_config_t *p_cfg)
{
  mct_stream_info_t *isp_out_dim;
  img_paaf_client_t *p_paaf_client;
  img_paaf_session_data_t *p_session_data;
  img_paaf_cfg_t *p_paaf_cfg;
  img_trans_info_t *p_camif_trans;
  boolean rc;
  int i = 0;

  //Get client Info
  p_paaf_client = (img_paaf_client_t *)p_client->p_private_data;
  p_paaf_cfg = &p_paaf_client->paaf_config;
  p_session_data = p_paaf_client->p_session_data;

  //Update all the PAAF cfg params except ROI
  for (i = 0; i < 11; i++)
    p_paaf_cfg->coeff_fir[i] = p_cfg->coeff_fir[i];
  for (i = 0; i < 6; i++) {
    p_paaf_cfg->coeffa[i] = p_cfg->coeffa[i];
    p_paaf_cfg->coeffb[i] = p_cfg->coeffb[i];
  }
  p_paaf_cfg->coeff_len = p_cfg->coeff_len;
  p_paaf_cfg->enable = p_cfg->enable;
  p_paaf_cfg->filter_type = PAAF_FILTER_TYPE(p_cfg->filter_type);
  p_paaf_cfg->frame_id = p_cfg->frame_id;
  p_paaf_cfg->FV_min = p_cfg->FV_min;

  //Translate and update ROI
  //Get ISP OUT DIM and stream crop for the processing stream
  isp_out_dim = &p_client->isp_output_dim_stream_info;
  p_camif_trans = &p_paaf_client->camif_trans_info;

  if (!p_paaf_client->use_af_tuning_trans) {
    //Calculate PAAF ROI position based on vfe map scale
    p_paaf_cfg->roi.pos.x = (int32_t)IMG_TRANSLATE(p_cfg->roi.left,
      p_camif_trans->h_scale, p_camif_trans->h_offset);
    p_paaf_cfg->roi.pos.y = (int32_t)IMG_TRANSLATE(p_cfg->roi.top,
      p_camif_trans->v_scale, p_camif_trans->v_offset);
    IDBG_MED("[PAAF_DBG] roi_x %d, h_scale %f, h_off %d. "
      "roi_y %d, v_scale %f, v_off %d",
      p_paaf_cfg->roi.pos.x, p_camif_trans->h_scale, p_camif_trans->h_offset,
      p_paaf_cfg->roi.pos.y, p_camif_trans->v_scale, p_camif_trans->v_offset);

    //Calculate PAAF client ROI size
    p_paaf_cfg->roi.size.width = (int32_t)IMG_TRANSLATE2(p_cfg->roi.width,
      p_camif_trans->h_scale, 0);
    p_paaf_cfg->roi.size.height = (int32_t)IMG_TRANSLATE2(p_cfg->roi.height,
      p_camif_trans->v_scale, 0);
    IDBG_MED("[PAAF_DBG] roi_w %d roi_h %d", p_paaf_cfg->roi.size.width,
      p_paaf_cfg->roi.size.height);
  } else {
    //Vfe map is invalid, default to center ROI based on af_tuning ratio
    IDBG_ERROR("[PAAF_DBG] Invalid VFE w and h map. "
      "Set to center ROI");
    module_paaf_default_roi_to_center(&p_paaf_cfg->roi, isp_out_dim->dim.width,
      isp_out_dim->dim.height, p_session_data->af_tuning_trans_info.h_scale,
      p_session_data->af_tuning_trans_info.v_scale);
  }

  //Grow ROI window if ROI is too small
  if (p_paaf_cfg->roi.size.width < PAAF_ROI_MIN_SIZE ||
    p_paaf_cfg->roi.size.height < PAAF_ROI_MIN_SIZE) {
    PAAF_INCREASE_ROI_SIZE(&p_paaf_cfg->roi, PAAF_ROI_MIN_SIZE);
  }

  //Checka nd FIX ROI boundary if required
  if (!module_paaf_fix_out_of_boundary(&p_paaf_cfg->roi,
    isp_out_dim->dim.width, isp_out_dim->dim.height)) {
    IDBG_ERROR("[PAAF_DBG] ROI window completely out of boundary. "
      "Set to center ROI");
    //Use default scale params from AF tuning header
    module_paaf_default_roi_to_center(&p_paaf_cfg->roi,
      isp_out_dim->dim.width, isp_out_dim->dim.height,
      p_session_data->af_tuning_trans_info.h_scale,
      p_session_data->af_tuning_trans_info.v_scale);
  }

  //If ROI is out of boundary, default to center ROI
  if (!IMG_RECT_IS_VALID(&p_paaf_cfg->roi,
    isp_out_dim->dim.width, isp_out_dim->dim.height)) {
    IDBG_ERROR("[PAAF_DBG] ROI out of boundary. Set to center ROI");
    module_paaf_default_roi_to_center(&p_paaf_cfg->roi,
      isp_out_dim->dim.width, isp_out_dim->dim.height,
      p_session_data->af_tuning_trans_info.h_scale,
      p_session_data->af_tuning_trans_info.v_scale);
  }

  //Ensure Even width and Height
  p_paaf_cfg->roi.size.width &= (~3);
  p_paaf_cfg->roi.size.height &= (~3);

  //Get num of pixels skip for filtering
  p_paaf_cfg->pixel_skip_cnt = module_paaf_get_skip(
    (uint32_t)p_paaf_cfg->roi.size.width,
    (uint32_t)p_paaf_cfg->roi.size.height, p_cfg->coeff_len);
  if (!p_paaf_cfg->pixel_skip_cnt)
    p_paaf_cfg->pixel_skip_cnt = 1;

  //Set Crop Window
  rc = module_paaf_set_frame_crop(&p_paaf_cfg->cropped_roi, &p_paaf_cfg->roi,
    isp_out_dim->dim.width, isp_out_dim->dim.height,
    p_paaf_cfg->pixel_skip_cnt);
  if (!rc) {
    IDBG_ERROR("[PAAF_DBG] frame crop out of boundary, "
      "set to center ROI");
    module_paaf_default_roi_to_center(&p_paaf_cfg->roi,
      isp_out_dim->dim.width, isp_out_dim->dim.height,
      p_session_data->af_tuning_trans_info.h_scale,
      p_session_data->af_tuning_trans_info.v_scale);
    module_paaf_set_frame_crop(&p_paaf_cfg->cropped_roi,
      &p_paaf_cfg->roi, isp_out_dim->dim.width, isp_out_dim->dim.height,
      p_paaf_cfg->pixel_skip_cnt);
  }

  IDBG_HIGH("[PAAF_DBG] ROI(%d %d %d %d), cropped roi"
    "(%d %d %d %d), coe_len %d pixel skip  count %d",
    p_paaf_cfg->roi.pos.x,
    p_paaf_cfg->roi.pos.y,
    p_paaf_cfg->roi.size.width,
    p_paaf_cfg->roi.size.height,
    p_paaf_cfg->cropped_roi.pos.x,
    p_paaf_cfg->cropped_roi.pos.y,
    p_paaf_cfg->cropped_roi.size.width,
    p_paaf_cfg->cropped_roi.size.height,
    p_paaf_cfg->coeff_len,
    p_paaf_cfg->pixel_skip_cnt);
}

/**
 * Function: module_paaf_update_meta
 *
 * Description: This function is used to called when the base
 *                       module updates the metadata
 *
 * Arguments:
 *   @p_client - pointer to imgbase client
 *   @p_meta: pointer to the image meta
 *
 * Return values:
 *     error values
 *
 * Notes: none
 **/
int32_t module_paaf_update_meta(imgbase_client_t *p_client,
  img_meta_t *p_meta)
{
  int rc;
  img_paaf_client_t *p_paaf_client;
  img_paaf_session_data_t *p_session_data;

  if (!p_client || !p_meta) {
    IDBG_ERROR("Invalid input %p %p", p_client, p_meta);
    rc = IMG_ERR_INVALID_INPUT;
    goto error;
  }

  //Get Session Data
  p_paaf_client = (img_paaf_client_t *)p_client->p_private_data;
  p_session_data = p_paaf_client->p_session_data;

  //If the AF config was updated, update meta data
  pthread_mutex_lock(&p_session_data->session_mutex);
  if (p_session_data->paaf_cfg_updated) {
    module_paaf_update_af_config(p_client, &p_session_data->paaf_cfg);
    p_session_data->paaf_cfg_updated = 0;
  }
  pthread_mutex_unlock(&p_session_data->session_mutex);

  rc = img_set_meta(p_meta, IMG_META_PAAF_CFG, &p_paaf_client->paaf_config);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("Error rc %d", rc);
    goto error;
  }

  IDBG_HIGH("Success");
  return IMG_SUCCESS;

error:
  return rc;
}

/**
 * Function: module_paaf_disable_all_clients
 *
 * Description: Callback function provided in mct_list_traverse used
 *  for updating multi clients in the same session
 *
 * Arguments:
 *   @p_data: pointer to client structure.
 *   @p_user: Pointer to multi session flag.
 *
 * Return values:
 *     TRUE/FALSE
 *
 * Notes: This function holds client mutex.
 **/
static boolean module_paaf_disable_all_clients(void *p_data, void *p_user)
{
  img_paaf_multi_client_data_t *p_client_data =
    (img_paaf_multi_client_data_t *)p_user;
  imgbase_client_t *p_client = (imgbase_client_t *)p_data;

  if (!p_client_data || !p_client) {
    return FALSE;
  }

  if ((p_client_data->session_id == p_client->session_id) &&
    IMG_CLIENT_HAS_STREAM(p_client, p_client_data->processing_stream)) {
    pthread_mutex_lock(&p_client->mutex);
    p_client->processing_disabled = p_client_data->processing_disable;
    pthread_mutex_unlock(&p_client->mutex);
  }
  return TRUE;
}

/**
 * Function: module_paaf_handle_af_config
 *
 * Description: This function handles Af config module event
 *
 * Arguments:
 *   p_mod_event - pointer of module event
 *   p_client - pointer to imgbase client
 *   p_core_ops - pointer to imgbase module ops
 *
 * Return values:
 *     TRUE/FALSE
 *
 * Notes: none
 **/
boolean module_paaf_handle_af_config(uint32_t identity,
  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_core_ops, boolean *is_evt_handled)
{
  img_paaf_client_t *p_paaf_client;
  img_paaf_session_data_t *p_session_data;
  module_imgbase_t *p_mod;
  mct_imglib_af_config_t *p_af_cfg;
  mct_imglib_swaf_config_t *p_swaf_cfg;
  img_paaf_multi_client_data_t client_data;
  boolean fwd_event = TRUE;

  if (!p_mod_event || !p_client || !p_core_ops) {
    IDBG_ERROR("Error input");
    return fwd_event;
  }

  p_af_cfg = (mct_imglib_af_config_t *)p_mod_event->module_event_data;
  if (NULL == p_af_cfg || p_af_cfg->type != AF_CFG_SWAF) {
    return fwd_event;
  }

  p_swaf_cfg = &(p_af_cfg->u.swaf);
  if (NULL == p_swaf_cfg) {
    IDBG_ERROR("Error SW AF CFG is NULL");
    return fwd_event;
  }

  p_mod = (module_imgbase_t *)p_client->p_mod;
  p_paaf_client = (img_paaf_client_t *)p_client->p_private_data;
  p_session_data = p_paaf_client->p_session_data;
  memset(&client_data, 0, sizeof(img_paaf_multi_client_data_t));


  //Update processing disabled/enabled flag
  if (p_swaf_cfg->enable)
    client_data.processing_disable = FALSE;
  else
    client_data.processing_disable = TRUE;

  //Update all clients in the current session
   client_data.session_id = p_client->session_id;
   client_data.processing_stream = p_session_data->processing_stream;
   mct_list_traverse(p_mod->imgbase_client,
      module_paaf_disable_all_clients, &client_data);

  //Update PAAF cfg saved in session data
  pthread_mutex_lock(&p_session_data->session_mutex);
  p_session_data->paaf_cfg = *p_swaf_cfg;
  p_session_data->paaf_cfg_updated = TRUE;
  pthread_mutex_unlock(&p_session_data->session_mutex);

  IDBG_MED("PAAF enable %d filter_type %d session %x",
    p_paaf_client->p_session_data->paaf_cfg.enable,
    p_paaf_client->p_session_data->paaf_cfg.filter_type,
    p_client->session_id);

  return fwd_event;

}

/**
 * Function: module_paaf_handle_event_done
 *
 * Description: Function to handle PAAF done event
 *
 * Arguments:
 *   @p_client - IMGLIB_BASE client
 *
 * Return values:
 *   none
 *
 * Notes: none
 **/
void module_paaf_handle_event_done(imgbase_client_t *p_client,
  img_event_t *p_event)
{
  img_meta_t *p_meta;
  img_paaf_result_t *p_paaf_result;
  imgbase_stream_t *p_stream = NULL;
  module_imgbase_t *p_mod;
  mct_list_t *p_client_list;
  imgbase_client_t *p_session_client;

  boolean ret;

  p_meta = p_event->d.p_meta;

  if (!p_meta) {
    IDBG_ERROR("PAAF result is null");
    return;
  }

  p_paaf_result = (img_paaf_result_t *)img_get_meta(p_meta,
    IMG_META_PAAF_RESULT);
  if (!p_paaf_result) {
    IDBG_ERROR("PAAF result is null");
    return;
  }

  IDBG_MED("PAAF result fv %f, framed %d", p_paaf_result->fV,
    p_paaf_result->frame_id);

  //Send event to 3A
  if (p_paaf_result->fV > 0) {
    ATRACE_BEGIN_SNPRINTF(40, " IMGLIB:PAAF EVENT result fv %f, frameid %d",
      p_paaf_result->fV, p_paaf_result->frame_id);

    mct_imglib_af_output_t paaf_output;
    memset(&paaf_output, 0x0, sizeof(mct_imglib_af_output_t));

    //Get Session stream Client
    p_mod = (module_imgbase_t *)p_client->p_mod;
    p_client_list = mct_list_find_custom(p_mod->imgbase_client, &p_client->session_id,
      module_imgbase_find_session_str_client);
    if (!p_client_list) {
      IDBG_ERROR("Cannot find session str client. Cannot send PAAF"
        "output to 3A");
      ATRACE_END();
      return;
    }

    p_session_client = (imgbase_client_t *)p_client_list->data;
    p_stream = &p_session_client->stream[0];
    IDBG_MED("Session identity %x, session id %d", p_stream->identity,
      p_session_client->session_id);
    if (NULL == p_stream) {
      IDBG_ERROR("Cannot find stream");
      ATRACE_END();
      return;
    }

    //Fill Event Data
    paaf_output.frame_id = p_paaf_result->frame_id;
    paaf_output.fV = p_paaf_result->fV;

    //Compose and send MCt event to 3A
    ret = mod_imgbase_send_event(p_stream->identity, TRUE,
      MCT_EVENT_MODULE_IMGLIB_AF_OUTPUT, paaf_output);
    if (!ret) {
      IDBG_ERROR("Send MCT_EVENT_MODULE_IMGLIB_AF_OUTPUT failed");
      ATRACE_END();
      return;
    }
    ATRACE_END();
  }
}

/**
 * Function: module_paaf_event_handler
 *
 * Description: event handler for paaf module
 *
 * Arguments:
 *   @p_client - pointer to IMGLIB_BASE client
 *   @p_event - pointer to the event
 *
 * Return values:
 *   IMG_SUCCESS
 *   IMG_ERR_INVALID_INPUT
 *
 * Notes: none
 **/
int module_paaf_event_handler(imgbase_client_t *p_client,
  img_event_t *p_event)
{
  int rc = IMG_SUCCESS;

  if ((NULL == p_event) || (NULL == p_client)) {
    IDBG_ERROR("Error Input");
    return IMG_ERR_INVALID_INPUT;
  }
  switch (p_event->type) {
  case QIMG_EVT_META_BUF_DONE: {
    module_paaf_handle_event_done(p_client, p_event);
    rc = IMG_ERR_EAGAIN;
    break;
  }
  default:
    break;
  }

  IDBG_LOW("%s:%d] type %d X", __func__, __LINE__, p_event->type);
  return rc;
}
/**
 * Function: module_paaf_handle_set_stream_config
 *
 * Description: This function handles module event
 *
 * Arguments:
 *   p_mod_event - pointer of module event
 *   p_client - pointer to imgbase client
 *   p_core_ops - pointer to imgbase module ops
 *
 * Return values:
 *     TRUE/FALSE
 *
 * Notes: none
 **/
boolean module_paaf_handle_set_stream_config(uint32_t identity,
  mct_event_module_t *p_mod_event, imgbase_client_t *p_client,
  img_core_ops_t *p_core_ops, boolean *is_evt_handled)
{
  module_imgbase_t *p_mod;
  img_paaf_session_data_t *session_data;
  sensor_out_info_t *p_sensor_info;
  img_paaf_client_t *p_paaf_client;

  boolean fwd_event = TRUE;

  if (!p_mod_event || !p_client || !p_core_ops) {
    IDBG_ERROR("Error input");
    return fwd_event;
  }

  p_sensor_info = (sensor_out_info_t *)p_mod_event->module_event_data;
  if (!p_sensor_info) {
    IDBG_ERROR("Error input %p", p_sensor_info);
    return fwd_event;
  }

  p_mod = (module_imgbase_t *)p_client->p_mod;
  p_paaf_client = (img_paaf_client_t *)p_client->p_private_data;
  session_data = p_paaf_client->p_session_data;

  IDBG_MED("MCT_EVENT_MODULE_SET_STREAM_CONFIG, w = %u, h = %u",
    p_sensor_info->dim_output.width, p_sensor_info->dim_output.height);

  session_data->max_dim.width = p_sensor_info->dim_output.width;
  session_data->max_dim.height = p_sensor_info->dim_output.height;

  return fwd_event;
}

/**
 * Function: module_paaf_deinit
 *
 * Description: Function used to deinit PAAF module
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_paaf_deinit(mct_module_t *p_mct_mod)
{
  IDBG_MED(":E");
  img_paaf_module_t *p_paafmod = p_mct_mod->module_private;
  pthread_mutex_destroy(&p_paafmod->session_data->session_mutex);
  module_imgbase_deinit(p_mct_mod);
  IDBG_MED(":X");
}

/** module_paaf_init:
 *
 * Description: Function used to initialize the PAAF module
 *
 *  Arguments:
 *  @name - name of the module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_paaf_init(const char *name)
{
  img_paaf_module_t *p_paafmod = calloc(1, sizeof(img_paaf_module_t));
  if (!p_paafmod) {
    IDBG_ERROR("%s:%d] Failed", __func__, __LINE__);
    return NULL;
  }
  pthread_mutex_init(&p_paafmod->session_data->session_mutex, NULL);

  return module_imgbase_init(name,
    IMG_COMP_GEN_FRAME_PROC,
    "qcom.gen_frameproc",
    p_paafmod,
    &g_caps,
    "libmmcamera_paaf_lib.so",
    CAM_QCOM_FEATURE_PAAF,
    &g_params);
}


/** module_paaf_set_parent:
 *
 * Description: This function is used to set the parent pointer
 *              of the PAAF module
 *
 *  Arguments:
 *  @p_parent - parent module pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_paaf_set_parent(mct_module_t *p_mct_mod, mct_module_t *p_parent)
{
  return module_imgbase_set_parent(p_mct_mod, p_parent);
}

