/* isp_resource.c
 *
 * Copyright (c) 2012-2020 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

/* std headers */
#include <stdio.h>
#include <dlfcn.h>
#include <pthread.h>
#include <linux/media.h>

#ifdef _ANDROID_
#include <cutils/properties.h>
#endif

/* kernel headers */
#include <media/msmb_ispif.h>

/* mctl headers */
#include "mtype.h"
#include "mct_stream.h"

/* isp headers */
#include "isp_module.h"
#include "isp_handler.h"
#include "isp_util.h"
#include "isp_log.h"
#include "isp_defs.h"

#undef ISP_DBG
#define ISP_DBG(fmt, args...) \
  ISP_DBG_MOD(ISP_LOG_COMMON, fmt, ##args)
#undef ISP_HIGH
#define ISP_HIGH(fmt, args...) \
  ISP_HIGH_MOD(ISP_LOG_COMMON, fmt, ##args)

/** isp_resource_get_vfe_version:
 *
 *  @module: module handle
 *  @port: port handle
 *  @event: IFACE request output resource event
 *
 *  Update IFACe request output resource event based for
 *  current session
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_resource_get_vfe_version(isp_resource_t *isp_resource,
  uint32_t vfe_id, uint32_t * version)
{
  boolean              ret = TRUE;
  isp_resource_info_t *res_info = NULL;
  isp_pipeline_t      *isp_pipeline = NULL;

  res_info = &isp_resource->isp_resource_info[vfe_id];
  isp_pipeline = res_info->isp_pipeline;
  *version = isp_pipeline->isp_version;
  return ret;
}

/** isp_resource_query_mod:
 *
 *  @isp_resource: handle to isp_resource_t
 *  @query_buf: handle to query_buf
 *  @session_id: session id of current session
 *
 *  Call query_mod on all sub_modules
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_resource_query_mod(isp_resource_t *isp_resource,
  void *query_buf, unsigned int session_id)
{
  boolean              ret = TRUE;
  isp_resource_info_t *res_info = NULL;
  isp_hw_id_t          res_isp = 0;
  int32_t              hw_stream_id = 0;
  isp_pipeline_t      *isp_pipeline = NULL;
  isp_submod_info_t   *submod_info = NULL;
  int32_t              hw_size = 0, submod_id = 0;
  mct_module_t        *mod = NULL;
  uint8_t              available_vfe = 0;
  mct_pipeline_isp_cap_t *isp_cap = NULL;
  mct_pipeline_cap_t     *cap_buf = (mct_pipeline_cap_t *)query_buf;

  isp_cap = &cap_buf->isp_cap;

  if (!isp_resource || !query_buf) {
    ISP_ERR("failed: %p %p", isp_resource, query_buf);
    return FALSE;
  }

  /* Iterate through each ISP resource and call query mod */
  for (res_isp = 0; res_isp < isp_resource->num_isp &&
         res_isp < ISP_HW_MAX; res_isp++) {
    res_info = &isp_resource->isp_resource_info[res_isp];
    isp_pipeline = res_info->isp_pipeline;

    if (res_info->resource_alloc.state == ISP_RESOURCE_FREE)
      available_vfe++;

    submod_info = res_info->common_hw_info;
    hw_size = (int32_t)res_info->common_hw_size;

    for (submod_id = 0; submod_id < hw_size; submod_id++) {
      if (!submod_info[submod_id].isp_submod) {
        continue;
      }
      mod = submod_info[submod_id].isp_submod;
      /* Call query_mod */
      if (mod->query_mod) {
        ISP_DBG("call query_mod on mod %s %p", MCT_MODULE_NAME(mod), mod);
        ret = mod->query_mod(mod, query_buf, session_id);
        if (ret == FALSE) {
          ISP_ERR("failed: start_session %s", MCT_MODULE_NAME(mod));
        }
      }
    }

    for (hw_stream_id = 0; hw_stream_id < (int32_t)isp_pipeline->num_hw_streams;
         hw_stream_id++) {

      submod_info = res_info->stream_hw_info[hw_stream_id];
      hw_size = (int32_t)res_info->stream_hw_size[hw_stream_id];

      for (submod_id = 0; submod_id < hw_size; submod_id++) {
        if (!submod_info[submod_id].isp_submod) {
          continue;
        }
        mod = submod_info[submod_id].isp_submod;
        /* Call query_mod */
        if (mod->query_mod) {
          ISP_DBG("call query_mod on mod %s %p", MCT_MODULE_NAME(mod), mod);
          ret = mod->query_mod(mod, query_buf, session_id);
          if (ret == FALSE) {
            ISP_ERR("failed: start_session %s", MCT_MODULE_NAME(mod));
          }
        }
      }
    }
  }
  if (isp_pipeline) {
    /* post HVX hw present or not*/
    isp_cap->is_hvx_present = isp_pipeline->is_hvx_present;
  }

  /* Post dual_vfe_enabled to mct to indicate no isp resource available*/
  if (!available_vfe || ((isp_resource->num_session_opened > 1) &&
    (isp_resource->num_isp_preallocate >= ISP_HW_MAX)))
    isp_cap->dual_vfe_enabled = TRUE;

  ISP_DBG("num_isp_preallocate %d num_session_opened %d dual_vfe_enabled %d",
    isp_resource->num_isp_preallocate, isp_resource->num_session_opened,
    isp_cap->dual_vfe_enabled);
  return ret;
}

/** isp_resource_pipeline_parse:
 *
 *  @isp_resource: isp resource handle
 *  @hw_id: hw id
 *  @stats_type: stats type
 *  @input_buf: input buf
 *  @output: output buf
 *  @stats_params_isp0: stats params for ISP 0
 *  @stats_params_isp1: stats params for ISP 1
 *
 *  Call pipeline parse to parse stats buffer
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_resource_pipeline_parse(isp_resource_t *isp_resource,
  isp_hw_id_t hw_id, enum msm_isp_stats_type stats_type,
  uint32_t raw_buf_len, void *input_buf,
  mct_event_stats_isp_t *output,
  isp_saved_stats_params_t *stats_params_isp0,
  isp_saved_stats_params_t *stats_params_isp1,
  isp_parser_session_params_t *parser_session_params)
{
  boolean                          ret = FALSE;
  isp_pipeline_stats_func_table_t *func_table;

  if (!isp_resource || !input_buf || !output ||
    !stats_params_isp0 || !parser_session_params) {
    ISP_ERR("failed: %p %p %p %p %p", isp_resource, input_buf, output,
      stats_params_isp0, parser_session_params);
    return FALSE;
  }

  if (stats_type >= MSM_ISP_STATS_MAX) {
    ISP_ERR("failed: invalid stats type %d", stats_type);
    return FALSE;
  }

  func_table = isp_resource->isp_resource_info[hw_id].isp_pipeline->func_table;
  if (!func_table) {
    ISP_ERR("failed: func_table %p", func_table);
    return FALSE;
  }

  if (!func_table->pipeline_stats_parse[stats_type]) {
    ISP_HIGH("failed: pipeline_stats_parse NULL");
    return FALSE;
  }

  if (stats_params_isp0 &&
      stats_params_isp0->rgns_stats[stats_type].is_valid == TRUE) {
    ret = func_table->pipeline_stats_parse[stats_type](input_buf, output,
    stats_params_isp0, parser_session_params);
    if (ret == FALSE) {
      ISP_ERR("failed: stats_type %d", stats_type);
    }
  }

  if (stats_params_isp1 &&
      stats_params_isp1->rgns_stats[stats_type].is_valid == TRUE) {
     /* right stripe VFE parsed from the middle of raw buffer
        the raw buffer len is counted by bytes, so cast to 8 bit*/
    ret = func_table->pipeline_stats_parse[stats_type](
      (uint8_t *)input_buf + (raw_buf_len / 2),
      output, stats_params_isp1, parser_session_params);
    if (ret == FALSE) {
      ISP_ERR("failed: stats_type %d", stats_type);
    }
  }

  return ret;
}

/** isp_resource_calculate_roi_map:
 *
 *  @isp_resource: ISP resource handle
 *  @hw_id: hw id
 *  @saved_params: ISP saved params
 *  @stream_crop: stream crop params
 *  @identity: event identity
 *
 *  Call pipieline specific calculate_roi_map function
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_resource_calculate_roi_map(isp_resource_t *isp_resource,
  isp_hw_id_t hw_id, isp_zoom_params_t *zoom_params_arr,
  mct_bus_msg_stream_crop_t *stream_crop, uint32_t identity)
{
  boolean                          ret = TRUE;
  isp_pipeline_stats_func_table_t *func_table;

  if (!isp_resource || !zoom_params_arr || !stream_crop) {
    ISP_ERR("failed: %p %p %p", isp_resource, zoom_params_arr, stream_crop);
    return FALSE;
  }

  if ((hw_id >= ISP_HW_MAX) || !identity) {
    ISP_ERR("failed: hw_id %d identity %x", hw_id, identity);
    return FALSE;
  }

  func_table = isp_resource->isp_resource_info[hw_id].isp_pipeline->func_table;
  if (!func_table) {
    ISP_ERR("failed: func_table %p", func_table);
    return FALSE;
  }

  if (!func_table->pipeline_calculate_roi_map) {
    ISP_ERR("failed: pipeline_calculate_roi_map NULL");
    return FALSE;
  }

  ret = func_table->pipeline_calculate_roi_map(zoom_params_arr,
    stream_crop, identity);
  if (ret == FALSE) {
    ISP_ERR("failed: pipeline_calculate_roi_map %d", ret);
  }

  return ret;
}

boolean isp_resource_update_module_cfg(isp_resource_t *isp_resource,
  isp_hw_id_t hw_id, int32_t fd, isp_module_enable_info_t *enable_bit_info,
  cam_format_t sensor_fmt)
{
  boolean         ret = TRUE;
  isp_pipeline_t *isp_pipeline = NULL;
  uint32_t        i;

  if (!isp_resource || (hw_id >= ISP_HW_MAX) || (fd <= 0) || !enable_bit_info) {
    ISP_ERR("failed: %p hw_id %d fd %d %p", isp_resource, hw_id, fd,
      enable_bit_info);
    return FALSE;
  }

  /* Lock resource manager params, unlock this on every return path */
  PTHREAD_MUTEX_LOCK(&isp_resource->mutex);

  isp_pipeline = isp_resource->isp_resource_info[hw_id].isp_pipeline;
  if (!isp_pipeline) {
    ISP_ERR("failed: isp_pipeline %p", isp_pipeline);
    goto ERROR;
  }

  /* update current submod_enable table */
  for (i = 0; i < ISP_MOD_MAX_NUM; i++) {
    if (enable_bit_info->submod_mask[i]) {
      isp_pipeline->submod_enable[i] = enable_bit_info->submod_enable[i];
    }
  }

  if (isp_pipeline->func_table &&
    isp_pipeline->func_table->pipeline_update_module_cfg) {
    ret = isp_pipeline->func_table->pipeline_update_module_cfg(fd,
      isp_pipeline->submod_enable, enable_bit_info,
      sensor_fmt);
    if (ret == FALSE) {
      ISP_ERR("failed: pipeline_update_module_cfg");
      goto ERROR;
    }
  } else {
    ISP_ERR("func_table %p or pipeline_update_module_cfg NULL",
      isp_pipeline->func_table);
    goto ERROR;
  }

  PTHREAD_MUTEX_UNLOCK(&isp_resource->mutex);
  return TRUE;

ERROR:
  PTHREAD_MUTEX_UNLOCK(&isp_resource->mutex);
  return FALSE;

}

/** isp_resource_get_hw_streams
 *
 *  @isp_resource: isp resource handle
 *  @num_hw_streams: number of hw streams to be returned
 *  @isp_id: ISP hw id for which number of hw streams needs
 *         to be retrieved
 *
 *  Return number of hw stream and HW stream IDs for the ISP id requested
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_resource_get_hw_streams(isp_resource_t *isp_resource,
  isp_submod_hw_streams_info_t *hw_streams, uint32_t isp_id)
{
  isp_resource_info_t *res_info = NULL;
  uint32_t i;

  if (!isp_resource || !hw_streams || isp_id >= ISP_HW_MAX) {
    ISP_ERR("failed: isp_resource %p hw_streams %p isp_id %d",
      isp_resource, hw_streams, isp_id);
    return FALSE;
  }

  /* Lock resource manager params, unlock this on every return path */
  PTHREAD_MUTEX_LOCK(&isp_resource->mutex);

  res_info = &isp_resource->isp_resource_info[isp_id];
  if (!res_info->isp_pipeline || !res_info->isp_pipeline->stream_hw_params[0]) {
    ISP_ERR("failed: NULL ptr");
    PTHREAD_MUTEX_UNLOCK(&isp_resource->mutex);
    return FALSE;
  }

  hw_streams->num_hw_streams = res_info->isp_pipeline->num_hw_streams;
  for (i = 0; i < res_info->isp_pipeline->num_hw_streams; i++) {
    hw_streams->hw_stream_ids[i] = res_info->isp_pipeline->hw_stream_ids[i];
  }

  PTHREAD_MUTEX_UNLOCK(&isp_resource->mutex);

  return TRUE;
}

/** isp_resource_get_hw_limitations
 *
 *  @isp_resource: isp resource handle
 *  @hw_limit: HW limitation container
 *  @isp_id: ISP hw id for which number of hw streams needs
 *         to be retrieved
 *
 *  Return ISP HW limitations
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_resource_get_hw_limitations(isp_resource_t *isp_resource,
  isp_submod_hw_limit_t *hw_limit, uint32_t isp_num, isp_hw_id_t *hw_ids)
{
  isp_resource_info_t           *res_info = NULL;
  isp_hw_streamid_t              hw_stream_id;
  isp_resource_hw_stream_info_t *hw_stream_info;
  isp_submod_hw_limit_desc_t    *hw_limits;
  isp_hw_id_t                    hw_id, j;
  uint32_t i;

  if (!isp_resource || !hw_limit || !isp_num || isp_num > ISP_HW_MAX ||
    !hw_ids) {
    ISP_ERR("failed: isp_resource %p hw_limit %p isp_num %d hw_ids %p",
      isp_resource, hw_limit, isp_num, hw_ids);
    return FALSE;
  }

  memset(hw_limit->hw_limits, 0x7F, sizeof(hw_limit->hw_limits));

  /* Lock resource manager params, unlock this on every return path */
  PTHREAD_MUTEX_LOCK(&isp_resource->mutex);

  for (j = 0; j < isp_num; j++) {
    hw_id = hw_ids[j];
    res_info = &isp_resource->isp_resource_info[hw_id];
    if (!res_info->isp_pipeline || !res_info->isp_pipeline->stream_hw_params[0]) {
      ISP_ERR("failed: NULL ptr");
      PTHREAD_MUTEX_UNLOCK(&isp_resource->mutex);
      return FALSE;
    }

    hw_limit->num_hw_streams = res_info->isp_pipeline->num_hw_streams;
    for(i = 0; i < res_info->isp_pipeline->num_hw_streams; i++) {
      hw_stream_id = res_info->isp_pipeline->hw_stream_ids[i];
      hw_stream_info = &res_info->isp_pipeline->hw_stream_info[hw_stream_id];
      hw_limits = &hw_limit->hw_limits[i];
      hw_limits->hw_stream_id = hw_stream_id;
      if (hw_limits->max_width > (unsigned int)hw_stream_info->max_width)
        hw_limits->max_width = hw_stream_info->max_width;
      if (hw_limits->max_height > (unsigned int)hw_stream_info->max_height)
        hw_limits->max_height = hw_stream_info->max_height;
      if (hw_limits->max_scale_ratio > res_info->isp_pipeline->max_scale_ratio)
        hw_limits->max_scale_ratio = res_info->isp_pipeline->max_scale_ratio;
    }
  }

  for (i = 0; i < hw_limit->num_hw_streams; i++) {
    ISP_DBG("i %d hw stream id %d max w %d h %d scale ratio %d", i,
      hw_limit->hw_limits[i].hw_stream_id, hw_limit->hw_limits[i].max_width,
      hw_limit->hw_limits[i].max_height,
      hw_limit->hw_limits[i].max_scale_ratio);
  }
  PTHREAD_MUTEX_UNLOCK(&isp_resource->mutex);

  return TRUE;
}

/** isp_resource_request_cds_cap:
 *
 *  @cds_cap: pointer to cds handle
 *  @session_param: session parameters
 *  @hw_id: hardware id
 *
 *  requests for cds capabilities for current session
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_resource_request_cds_cap(
  uint32_t            *cds_cap,
  isp_session_param_t *session_param,
  uint32_t             hw_id,
  uint32_t             identity)
{
  boolean                   ret = TRUE;
  mct_event_t               event;
  isp_private_event_t       private_event;

  /*request Scalar to fill CDS capability */
  memset(&event, 0, sizeof(event));
  memset(&private_event, 0, sizeof(private_event));
  private_event.type = ISP_PRIVATE_REQUEST_CDS_CAP;
  private_event.data = cds_cap;

  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
  event.u.module_event.module_event_data = &private_event;

  ret = isp_util_forward_event_to_internal_pipeline(
    session_param, &event, hw_id);
  if (ret == FALSE) {
    ISP_ERR("failed: ISP_PRIVATE_REQUEST_CDS_CAP hw_id %d",
     hw_id);
    return ret;
  }
  return ret;
}

/** isp_resource_update_mapped_stream_info:
 *
 *  @data: handle to identity
 *  @user_data: handle to isp_mapped_stream_info_t
 *
 *  Update mapped stream info based on identity
 *
 *  Return TRUE
 **/
static boolean isp_resource_update_mapped_stream_info(void *data,
  void *user_data)
{
  boolean                   ret = TRUE;
  uint32_t                 *identity = (uint32_t *)data;
  isp_mapped_stream_info_t *mapped_stream_info =
    (isp_mapped_stream_info_t *)user_data;
  isp_pix_out_info_t       *isp_pix_output = NULL;
  isp_session_param_t      *session_param = NULL;
  isp_stream_param_t       *stream_param = NULL;
  uint32_t                  index = 0;
  uint32_t                  i = 0;
  mct_event_t               event;
  isp_private_event_t       private_event;
  isp_hw_stream_info_t      hw_stream_info;
  isp_port_data_t          *port_data = NULL;
  mct_list_t               *l_identity = NULL;
  mct_port_t               *out_port = NULL;
  isp_hw_streamid_t         hw_stream = 0;
  isp_stream_port_map_info_t *streams = NULL;
  enum msm_vfe_axi_stream_src axi_src = VFE_AXI_SRC_MAX;
  iface_resource_request_t *resource_request = NULL;

  if (!data || !user_data) {
    ISP_ERR("failed: data %p user_data %p", data, user_data);
    return TRUE;
  }

  if (!mapped_stream_info->module || !mapped_stream_info->port ||
      !mapped_stream_info->isp_pix_output ||
      !mapped_stream_info->session_param) {
    ISP_ERR("failed: %p %p %p %p", mapped_stream_info->module,
      mapped_stream_info->port, mapped_stream_info->isp_pix_output,
      mapped_stream_info->session_param);
    return TRUE;
  }

  isp_pix_output = mapped_stream_info->isp_pix_output;
  session_param = mapped_stream_info->session_param;
  resource_request = mapped_stream_info->resource_request;

  ISP_HIGH("trying to mapped_mct_stream_id %d",
    ISP_GET_STREAM_ID(*identity));

  ret = isp_util_get_stream_params(session_param, *identity,
    &stream_param);
  if (ret == FALSE || !stream_param) {
    ISP_ERR("failed: isp_util_get_session_stream_params ret %d %p",
      ret, stream_param);
    /* return TRUE so that mct list traverse doesn't terminate */
    return TRUE;
  }

  /* Do not iterate for session based stream */
  if ((stream_param->is_pipeline_supported == FALSE) ||
    (stream_param->stream_info.stream_type == CAM_STREAM_TYPE_PARM)) {
    ISP_DBG("stream identity %x is not supported in ISP pipeline",
      stream_param->stream_info.identity);
    return TRUE;
  }

  ret = isp_resource_request_cds_cap(
    &resource_request->cds_capable[resource_request->num_pix_stream],
    session_param, mapped_stream_info->hw_id,
    stream_param->stream_info.identity);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_resource_request_cds_cap");
  }

  /* HAL stream should have only one hw stream */
  if (stream_param->num_hw_stream == 1) {
    axi_src =
      isp_util_get_axi_src_type(stream_param->stream_info.fmt,
      stream_param->hw_stream[0]);
  } else {
    ISP_ERR("failed: invalid num_hw_stream %d for stream type %d",
      stream_param->num_hw_stream,
      stream_param->stream_info.stream_type);
    return TRUE;
  }
  ISP_DBG("dual_dbg stream type %d, AXI_SRC = %d (ENC 0/ View 1/Camif 2/Ideal 3/Rdi 456)",
    stream_param->stream_info.stream_type ,axi_src);

  streams = session_param->stream_port_map.streams;
  for (i = 0; i < session_param->stream_port_map.num_streams; i++) {
    if (stream_param->stream_info.stream_type == streams[i].stream_type &&
      stream_param->stream_info.dim.width == streams[i].orig_stream_sizes.width &&
      stream_param->stream_info.dim.height == streams[i].orig_stream_sizes.height &&
      stream_param->hw_stream[0] == streams[i].hw_stream) {
      break;
    }
  }
  if (i == session_param->stream_port_map.num_streams) {
    ISP_ERR("failed: Cannot find corresponding stream");
    return TRUE;
  }

  switch (axi_src) {
  case IDEAL_RAW: {
     mapped_stream_info->num_pix_stream++;
     isp_pix_output->axi_path = axi_src;
     isp_pix_output->fmt = stream_param->stream_info.fmt;
     isp_pix_output->dim.width = stream_param->stream_info.dim.width;
     isp_pix_output->dim.height = stream_param->stream_info.dim.height;
     isp_pix_output->streaming_mode =
       stream_param->stream_info.streaming_mode;
     isp_pix_output->mapped_mct_stream_id[isp_pix_output->num_mct_stream_mapped]
       = ISP_GET_STREAM_ID(*identity);
     isp_pix_output->use_native_buffer = streams[i].native_buffer;
     isp_pix_output->isp_frame_skip = NO_SKIP;
     isp_pix_output->num_mct_stream_mapped++;
  }
     break;

  case PIX_VIEWFINDER:
  case PIX_ENCODER:
  case PIX_VIDEO: {
    /* ENC/VIEW/VIDEO stream, fo through full pipeline to query dimention
      Get super dim from scaler module */
    memset(&event, 0, sizeof(event));
    memset(&private_event, 0, sizeof(private_event));
    memset(&hw_stream_info, 0, sizeof(hw_stream_info));

    private_event.type = ISP_PRIVATE_FETCH_SCALER_HW_STREAM_INFO;
    private_event.data = &hw_stream_info;

    event.type = MCT_EVENT_MODULE_EVENT;
    event.identity = *identity;
    event.direction = MCT_EVENT_DOWNSTREAM;
    event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
    event.u.module_event.module_event_data = &private_event;

    ret = isp_util_forward_event_to_internal_pipeline(session_param, &event,
       mapped_stream_info->hw_id);
    if (ret == FALSE) {
      ISP_ERR("failed: ISP_PRIVATE_FETCH_SCALER_HW_STREAM_INFO hw_id %d",
        mapped_stream_info->hw_id);
    } else {
      mapped_stream_info->num_pix_stream++;
      isp_pix_output->axi_path = axi_src;
      isp_pix_output->fmt = hw_stream_info.fmt;
      isp_pix_output->dim.width = hw_stream_info.width;
      isp_pix_output->dim.height = hw_stream_info.height;
      isp_pix_output->streaming_mode = hw_stream_info.streaming_mode;
      isp_pix_output->
        mapped_mct_stream_id[isp_pix_output->num_mct_stream_mapped]
        = ISP_GET_STREAM_ID(*identity);
      isp_pix_output->use_native_buffer = streams[i].native_buffer;
      isp_pix_output->controlable_output = streams[i].controlable_output;
      isp_pix_output->shared_output = streams[i].shared_output;
      isp_pix_output->num_mct_stream_mapped++;
    }

    /*split case fill in isp split info base on ispif stripe info*/
    if (session_param->num_isp > 1) {
       if (stream_param->is_pipeline_supported == TRUE) {
         isp_util_set_split_output_info_per_stream(session_param,
           &isp_pix_output->isp_split_output_info,
           mapped_stream_info->hw_id, *identity);
       }
    }

    /* Check frame skip request from each module */
    if (stream_param->is_pipeline_supported == TRUE) {
      isp_util_set_isp_frame_skip(mapped_stream_info->module,
        session_param,
        &isp_pix_output->isp_frame_skip,
        mapped_stream_info->hw_id, *identity);
    }
  }
    break;

  default:
     ISP_ERR("Warning: ISP_PIPELINE is not used for this stream: fmt %d,"
             " axi_path %d (ENC 0/ View 1/Camif_raw 2/Ideal raw 3/Rdi 456)",
       stream_param->stream_info.fmt, axi_src);
    return FALSE;
  }

  ISP_HIGH("pix_output mapp mct stream id %d, num_mct_stream_mapped %d",
    ISP_GET_STREAM_ID(*identity), isp_pix_output->num_mct_stream_mapped);
  ISP_HIGH("pix_output axi_path %d", isp_pix_output->axi_path);
  ISP_HIGH("pix_output->fmt %d", isp_pix_output->fmt);
  ISP_HIGH("pix_output->dim w %d h %d", isp_pix_output->dim.width,
    isp_pix_output->dim.height);
  ISP_HIGH("pix_output->streaming_mode %d",
    isp_pix_output->streaming_mode);
  ISP_HIGH("pix_output->num_mct_stream_mapped %d",
    isp_pix_output->num_mct_stream_mapped);
  ISP_HIGH("pix_output->use_native_buffer %d",
    isp_pix_output->use_native_buffer);
  ISP_HIGH("pix_output->controlable_output %d",
    isp_pix_output->controlable_output);
  ISP_HIGH("pix_output->shared_output %d", isp_pix_output->shared_output);

  return TRUE;
}

/** isp_resource_request_stripe_info:
 *
 *  @isp_resource: isp resource handle
 *  @module: module handle
 *  @port: port handle
 *  @event: IFACE request output resource event
 *
 *  Update IFACe request output resource event based for
 *  current session
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_resource_request_stripe_info(isp_resource_t *isp_resource,
  iface_resource_request_t *resource_request,
  isp_session_param_t  *session_param, uint32_t hw_id)
{
  boolean                   ret = TRUE;
  mct_event_t               event;
  isp_private_event_t       private_event;
  isp_stripe_request_t      isp_stripe_request;
  memset(&isp_stripe_request, 0, sizeof(isp_stripe_request_t));

  ISP_DBG(" DUAL ISP!");

  if (!resource_request || !session_param) {
    ISP_ERR("failed: resource_request %p session_param %p",
      resource_request, session_param);
    return FALSE;
  }

  /*request all module for overlap limitation*/
  memset(&event, 0, sizeof(event));
  memset(&private_event, 0, sizeof(private_event));
  private_event.type = ISP_PRIVATE_REQUEST_STRIPE_LIMITATION;
  private_event.data = &isp_stripe_request.stripe_limit;

  isp_stripe_request.stripe_limit.max_left_split =
    isp_resource->isp_resource_info[ISP_HW_0].isp_pipeline->max_width;
  isp_stripe_request.stripe_limit.max_right_split =
    isp_resource->isp_resource_info[ISP_HW_1].isp_pipeline->max_width;

  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = 0;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
  event.u.module_event.module_event_data = &private_event;

  if (session_param->session_based_ide) {
    ret = isp_util_forward_event_to_session_based_stream_all_int_pipelines(
      session_param, &event);
    if (ret == FALSE) {
      ISP_ERR("failed: ISP_PRIVATE_REQUEST_MIN_STRIPE_OVERLAP hw_id %d",
       hw_id);
      memset(&isp_stripe_request, 0, sizeof(isp_stripe_request_t));
      return ret;
    }
  } else {
    ret = isp_util_forward_event_to_all_streams_all_internal_pipelines(
      session_param, &event);
    if (ret == FALSE) {
      ISP_ERR("failed: ISP_PRIVATE_REQUEST_MIN_STRIPE_OVERLAP hw_id %d",
       hw_id);
      memset(&isp_stripe_request, 0, sizeof(isp_stripe_request_t));
      return ret;
    }
  }

  /*request scaler to fill in stripe info*/
  memset(&event, 0, sizeof(event));
  memset(&private_event, 0, sizeof(private_event));
  isp_stripe_request.offline_mode = FALSE;
  private_event.type = ISP_PRIVATE_REQUEST_STRIPE_OFFSET;
  private_event.data = &isp_stripe_request;

  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = 0;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
  event.u.module_event.module_event_data = &private_event;

  if (session_param->session_based_ide) {
    ret = isp_util_forward_event_to_session_based_stream_all_int_pipelines(
      session_param, &event);
    if (ret == FALSE) {
      ISP_ERR("failed: ISP_PRIVATE_REQUEST_STRIPE_OFFSET hw_id %d",
       hw_id);
       memset(&isp_stripe_request, 0, sizeof(isp_stripe_request_t));
      return ret;
    }
  } else {
    ret = isp_util_forward_event_to_all_streams_all_internal_pipelines(
      session_param, &event);
    if (ret == FALSE) {
      ISP_ERR("failed: ISP_PRIVATE_REQUEST_STRIPE_OFFSET hw_id %d",
       hw_id);
       memset(&isp_stripe_request, 0, sizeof(isp_stripe_request_t));
      return ret;
    }
  }

  /*fill in resource request*/
  resource_request->ispif_split_output_info.overlap =
    isp_stripe_request.ispif_out_info.overlap;
  resource_request->ispif_split_output_info.right_stripe_offset =
    isp_stripe_request.ispif_out_info.right_stripe_offset;
  resource_request->ispif_split_output_info.split_point =
    isp_stripe_request.ispif_out_info.split_point;
  if (session_param->num_isp > 1) {
    resource_request->ispif_split_output_info.is_split = 1;
  } else {
    resource_request->ispif_split_output_info.is_split = 0;
  }

  ISP_DBG("stripe is_split %d, min overlap = %d, max stripe offset = %d",
    resource_request->ispif_split_output_info.is_split,
    resource_request->ispif_split_output_info.overlap,
   resource_request->ispif_split_output_info.right_stripe_offset);

  memset(&event, 0, sizeof(event));
  memset(&private_event, 0, sizeof(private_event));
  private_event.type = ISP_PRIVATE_SET_STRIPE_INFO;
  private_event.data = &resource_request->ispif_split_output_info;

  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = 0;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
  event.u.module_event.module_event_data = &private_event;

  if (session_param->session_based_ide) {
    ret = isp_util_forward_event_to_session_based_stream_all_int_pipelines(
      session_param, &event);
    if (ret == FALSE) {
      ISP_ERR("failed: ISP_PRIVATE_SET_STRIPE_INFO hw_id %d",
       hw_id);
      memset(&isp_stripe_request, 0, sizeof(isp_stripe_request_t));
      return ret;
    }
  } else {
    ret = isp_util_forward_event_to_all_streams_all_internal_pipelines(
      session_param, &event);
    if (ret == FALSE) {
      ISP_ERR("failed: ISP_PRIVATE_SET_STRIPE_INFO hw_id %d",
       hw_id);
      memset(&isp_stripe_request, 0, sizeof(isp_stripe_request_t));
      return ret;
    }
  }

  return ret;
}

/** isp_resource_request_offline_stripe_info:
 *
 *  @isp_resource: isp resource handle
 *  @module: module handle
 *  @port: port handle
 *  @event: IFACE request output resource event
 *
 *  Update IFACe request output resource event based for
 *  current session
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_resource_request_offline_stripe_info(isp_resource_t *isp_resource,
  ispif_out_info_t *ispif_split_output_info,
  void  *param, uint32_t hw_id, uint32_t identity)
{
  boolean                   ret = TRUE;
  mct_event_t               event;
  isp_private_event_t       private_event;
  isp_stripe_request_t      isp_stripe_request;
  isp_session_param_t       *session_param = param;
  memset(&isp_stripe_request, 0, sizeof(isp_stripe_request_t));

  ISP_ERR(" OFFLINE MULTI PASS! identity 0x%X", identity);

  if (!ispif_split_output_info || !session_param) {
    ISP_ERR("failed: resource_request %p session_param %p",
      ispif_split_output_info, session_param);
    return FALSE;
  }

  /*request all module for overlap limitation*/
  memset(&event, 0, sizeof(event));
  memset(&private_event, 0, sizeof(private_event));
  private_event.type = ISP_PRIVATE_REQUEST_STRIPE_LIMITATION;
  private_event.data = &isp_stripe_request.stripe_limit;

  isp_stripe_request.stripe_limit.max_left_split =
    isp_resource->isp_resource_info[ISP_HW_0].isp_pipeline->max_width;
  isp_stripe_request.stripe_limit.max_right_split =
    isp_resource->isp_resource_info[ISP_HW_1].isp_pipeline->max_width;

  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
  event.u.module_event.module_event_data = &private_event;

  ret = isp_util_forward_event_to_internal_pipeline(session_param, &event,
    hw_id);
  if (ret == FALSE) {
    CDBG_ERROR("failed: isp_util_forward_event_to_internal_pipeline\n");
  }

  /*request scaler to fill in stripe info*/
  memset(&event, 0, sizeof(event));
  memset(&private_event, 0, sizeof(private_event));
  isp_stripe_request.offline_mode = TRUE;
  private_event.type = ISP_PRIVATE_REQUEST_STRIPE_OFFSET;
  private_event.data = &isp_stripe_request;

  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
  event.u.module_event.module_event_data = &private_event;

  ret = isp_util_forward_event_to_internal_pipeline(session_param, &event,
    hw_id);
  if (ret == FALSE) {
    CDBG_ERROR("failed: isp_util_forward_event_to_internal_pipeline\n");
  }
  /*fill in resource request*/
  ispif_split_output_info->overlap =
    isp_stripe_request.ispif_out_info.overlap;
  ispif_split_output_info->right_stripe_offset =
    isp_stripe_request.ispif_out_info.right_stripe_offset;
  ispif_split_output_info->split_point =
    isp_stripe_request.ispif_out_info.split_point;
  if (session_param->multi_pass) {
    ispif_split_output_info->is_split = 1;
  } else {
    ispif_split_output_info->is_split = 0;
  }

  ISP_INFO("stripe is_split %d, min overlap = %d, max stripe offset = %d",
    ispif_split_output_info->is_split,
    ispif_split_output_info->overlap,
    ispif_split_output_info->right_stripe_offset);

  memset(&event, 0, sizeof(event));
  memset(&private_event, 0, sizeof(private_event));
  private_event.type = ISP_PRIVATE_SET_STRIPE_INFO;
  private_event.data = ispif_split_output_info;

  event.type = MCT_EVENT_MODULE_EVENT;
  event.identity = identity;
  event.direction = MCT_EVENT_DOWNSTREAM;
  event.u.module_event.type = MCT_EVENT_MODULE_ISP_PRIVATE_EVENT;
  event.u.module_event.module_event_data = &private_event;

  ret = isp_util_forward_event_to_internal_pipeline(session_param, &event,
    hw_id);
  return ret;
}

/** isp_check_reserve
 *
 *  @isp_resource: pointer to isp resource
 *  @count:        Number of isp resource to reserve
 *  @hw_ids:       Hardware id of the isp unit that can be reserved, this is out parameter.
 *  @in_w:         The output width that the isp should be able to support
 *  @in_h:         The output height that the isp should be able to support
 *  @full_w:       The max width that the sensor can request
 *  @full_h:       The max height which sensor can request
 *  @pix_clk:      The clock rate which the isp should be able to support
 *  @max_level:    Out parameter in which the highest cx level to be used for the isp
 *                 units is set
 *
 *  Check if isp units can be reserved for the given dimension and clock
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_check_reserve(isp_resource_t *isp_resource, uint32_t count,
                           isp_hw_id_t *hw_ids,
                           uint32_t in_w, uint32_t in_h,
                           uint32_t full_w, uint32_t full_h,
                           uint32_t pix_clk,
                           isp_cx_levels_t *max_level)
{
  isp_cx_levels_t               cx_level = 0;
  uint32_t                      isp = 0, i;
  isp_resource_info_t          *res_info;
  isp_resource_alloc_t         *res_alloc;
  isp_hw_id_t                   hw_id;
  boolean                       sensor_fit = FALSE;

  *max_level = 0;

  /*
   * Check if there are "count(argument passed)" number of vfe available that
   * can support the sensor max res
   */
  sensor_fit = FALSE;
  for (i = 0 ; i < isp_resource->num_isp ; i++) {
    hw_id = isp_resource->sorted_hw_ids[i];
    res_info = &isp_resource->isp_resource_info[hw_id];
    res_alloc = &res_info->resource_alloc;
    if (res_alloc->state == ISP_RESOURCE_RESERVED)
      continue;
    if ((res_info->isp_pipeline->max_width >= full_w) &&
      (res_info->isp_pipeline->max_height >= full_h)) {
      isp++;
      if (isp == count) {
        sensor_fit = TRUE;
        break;
      }
    }
  }

  /*
   * Select the isp that can support the requested width and height,
   * also if the sensor_fit is set then select the vfe that can also
   * support the sensor max resolution, sensor_fit set means that
   * there is a vfe available that can support this max resolution
   */
  isp = 0;
  for (i = 0; i < isp_resource->num_isp; i++) {
    res_info = &isp_resource->isp_resource_info[isp_resource->sorted_hw_ids[i]];
    res_alloc = &res_info->resource_alloc;
    if (res_alloc->state == ISP_RESOURCE_RESERVED)
      continue;
    if (res_info->isp_pipeline->max_width < in_w ||
        res_info->isp_pipeline->max_height < in_h)
      continue;
    /*
     * There is sensor fit and the current vfe is smaller then the
     * sensor max res, then do not select this vfe, instead go for
     * the one whose max res can support the sensor max res as well
     */
    if (sensor_fit && (res_info->isp_pipeline->max_height < full_h ||
                      res_info->isp_pipeline->max_width < full_w))
      continue;

    for (cx_level = 0; cx_level < ISP_CX_LEVEL_MAX; cx_level++) {
      if (res_info->isp_pipeline->max_pix_clk_rate[cx_level] >= pix_clk)
        break;
    }
    if (cx_level == ISP_CX_LEVEL_MAX)
      continue;

    hw_ids[isp] = isp_resource->sorted_hw_ids[i];
    isp++;
    if (*max_level < cx_level)
      *max_level = cx_level;
    if (isp == count)
      break;
  }
  if (isp != count) {
    for (i = 0; i < isp; i++)
      hw_ids[i] = 0;
    *max_level = ISP_CX_LEVEL_MAX;
    return FALSE;
  }

  return TRUE;
}

static boolean isp_resource_reserve_isp(isp_resource_t *isp_resource,
  isp_session_param_t *session_param, uint32_t *num_isp,
  isp_hw_id_t *hw_ids, uint32_t request_dual_isp)
{
  boolean                        reserved;
  isp_resource_info_t           *res_info = NULL;
  isp_resource_alloc_t          *res_alloc = NULL;
  isp_hw_id_t                    i, j, hw_id;
  uint32_t                       in_w, in_h, pix_clk;
  uint32_t                       full_w, full_h;
  char                           value[PROPERTY_VALUE_MAX];
  uint32_t                       dual_vfe_enabled;
  uint8_t                        vfe_fuse_check = FALSE;
  isp_cx_levels_t                single_cx_level, dual_cx_level = 0;
  isp_hw_id_t                    dual_hw_id[2] = {0};
  char                           buf[10] = {0};
  int                            fd, soc_id = 0;

  in_w = session_param->sensor_output_info.dim_output.width;
  in_h = session_param->sensor_output_info.dim_output.height;
  full_w = session_param->sensor_output_info.full_res.width;
  full_h = session_param->sensor_output_info.full_res.height;
  pix_clk = session_param->sensor_output_info.op_pixel_clk;

  fd = open("/sys/devices/soc0/soc_id", O_RDONLY);
  if (fd >= 0) {
     if (read(fd, buf, sizeof(buf) - 1) == -1) {
        ISP_ERR("Unable to read soc_id");
     } else {
        soc_id = atoi(buf);
     }
     close(fd);
   }

  if (isp_resource->isp_hw_limt.hw_limit != NO_LIMIT) {
      switch (isp_resource->isp_resource_info->isp_pipeline->hw_version) {
      case ISP_SDM660:
      /* If eFuse is set to limit VFE resolution, check for the
         Dual Camera usecase and if the sensor resolution for any
         VFE exceeds supported value, return error e.g. in case of
         SDM630 limit dualcamera usecase to 13MP+13MP. Both SDM660
         and SDM630 use the same VFE h/w version */
          if (session_param->is_dualCam)
              vfe_fuse_check = TRUE;

      /* For trinket, dual camera limatation needs to be checked*/
         if (soc_id == SOC_ID_TRINKET) {
             vfe_fuse_check = TRUE;
             if (session_param->is_dualCam) {
              switch (isp_resource->isp_hw_limt.hw_limit) {
                 case MAX_8MP_DUAL:
                  isp_resource->isp_hw_limt.max_resolution = 7990272;
                 break;
                 case MAX_13MP_DUAL:
                  isp_resource->isp_hw_limt.max_resolution = 13500000;
                 break;
                 case MAX_16MP_DUAL:
                  isp_resource->isp_hw_limt.max_resolution = 16384000;
                 break;
                 case NO_LIMIT_DUAL:
                 default:
                  break;
                }
             }
          }
          break;
      case ISP_MSM8953:
          vfe_fuse_check = TRUE;
          break;
      default:
          break;
      }
      if (vfe_fuse_check && isp_resource->isp_hw_limt.max_resolution < (in_w * in_h)) {
          ISP_ERR("Sensor ouput resolution exceeds max supported max_res = %d, in_wxh = %dx%d",
                  isp_resource->isp_hw_limt.max_resolution, in_w, in_h);
          return FALSE;
      }
  }

  ISP_DBG("full_wx x full_h %d x %d ,  WxH %d x %d ", full_w, full_h, in_w, in_h);
  /*enforce dual vfe enable by set property*/
  property_get("persist.vendor.camera.isp.dualisp", value, "0");
  dual_vfe_enabled = atoi(value);

  /* Limiting sensor op_clk to TURBO, Forcing each session to single ISP in dual camera scenario */
  if (session_param->is_dualCam) {
      dual_cx_level = ISP_CX_LEVEL_MAX;
      if (pix_clk > isp_resource->isp_resource_info->
        isp_pipeline->max_pix_clk_rate[ISP_CX_LEVEL_TURBO]) {
          ISP_INFO("Limiting sensor op_clk %d to TURBO %d \n", pix_clk,
            isp_resource->isp_resource_info->isp_pipeline->
              max_pix_clk_rate[ISP_CX_LEVEL_TURBO]);
          pix_clk = isp_resource->isp_resource_info->
            isp_pipeline->max_pix_clk_rate[ISP_CX_LEVEL_TURBO];
      }
  } else {
      if (session_param->stream_port_map.raw_stream_exists) {
        dual_cx_level = ISP_CX_LEVEL_MAX;
      } else {
        /* For dual vfe the req width and height is divide by 2 + 20% overlap, hence mult by 3/5 */
        isp_check_reserve(isp_resource, 2, dual_hw_id,
                          ((in_w * 3) / 5), in_h,
                          ((full_w * 3) / 5), full_h,
                          (pix_clk / 2),
                          &dual_cx_level);
      }
  }

  if (dual_vfe_enabled && dual_cx_level != ISP_CX_LEVEL_MAX)
    single_cx_level = ISP_CX_LEVEL_MAX;
  else
    /* get the cx level to use in case of single isp */
    isp_check_reserve(isp_resource, 1, &hw_id, in_w, in_h, full_w, full_h, pix_clk, &single_cx_level);
  if (single_cx_level != ISP_CX_LEVEL_MAX &&
      !request_dual_isp) {
    res_info = &isp_resource->isp_resource_info[hw_id];
    if (single_cx_level <= res_info->isp_pipeline->single_vfe_cx_level ||
        single_cx_level <= dual_cx_level) {
      res_alloc = &res_info->resource_alloc;
      res_alloc->state = ISP_RESOURCE_RESERVED;
      res_alloc->session_id = session_param->session_id;
      *num_isp = 1;
      hw_ids[0] = hw_id;
      ISP_INFO("Allocate num_isp %d for session %d request_dual_isp %d\n",
            *num_isp, session_param->session_id,request_dual_isp);
      return TRUE;
    }
  }

  if (dual_cx_level != ISP_CX_LEVEL_MAX ||
      request_dual_isp) {
    isp_hw_id_t k;

    for (i = 0; i < 2; i++) {
      res_info = &isp_resource->isp_resource_info[dual_hw_id[i]];
      res_alloc = &res_info->resource_alloc;
      res_alloc->state = ISP_RESOURCE_RESERVED;
      res_alloc->session_id = session_param->session_id;

      /* place the hw id in sorted order of the id since the caller
       * of this function assumes it is that way
       */
      for (j = 0; j < i; j++) {
        if (hw_ids[j] > dual_hw_id[i])
          break;
      }
      for (k = i; k > j; k--)
        hw_ids[k] = dual_hw_id[k - 1];

      hw_ids[j] = dual_hw_id[i];
    }
    *num_isp = 2;
    ISP_INFO("Allocate num_isp %d for session %d request_dual_isp %d\n",
        *num_isp, session_param->session_id, request_dual_isp);
    return TRUE;
  }

  /* No isp available */
  ISP_ERR("Failed to allocate isp resource for session %d\n",
          session_param->session_id);
  return FALSE;
}

static boolean isp_resource_reserve_offline_isp(isp_resource_t *isp_resource,
  isp_session_param_t *session_param, uint32_t *num_isp,
  isp_hw_id_t *hw_ids)
{
  boolean                        reserved;
  isp_resource_info_t           *res_info = NULL;
  isp_resource_alloc_t          *res_alloc = NULL;
  isp_hw_id_t                    i, j, hw_id;
  uint32_t                       in_w, in_h;

  in_w = session_param->offline_input_cfg.dim_output.width;
  in_h = session_param->offline_input_cfg.dim_output.height;

  /* Check witch ISP fit best. ISP with the smallest input is first. */
  reserved = FALSE;
  for (i = 0; i < isp_resource->num_isp; i++) {
    in_w = session_param->offline_input_cfg.dim_output.width;
    hw_id = isp_resource->sorted_hw_ids[i];
    res_info = &isp_resource->isp_resource_info[hw_id];
    res_alloc = &res_info->resource_alloc;
    if ((res_alloc->state == ISP_RESOURCE_FREE) &&
      (res_info->isp_pipeline->max_width >= in_w) &&
      (res_info->isp_pipeline->max_height >= in_h)) {
      hw_ids[(*num_isp)++] = hw_id;
      res_alloc->state = ISP_RESOURCE_RESERVED;
      res_alloc->session_id = session_param->session_id;
      reserved = TRUE;
      session_param->multi_pass = FALSE;
      break;
    } else {
      in_w = ((in_w / 2) * 6) / 5;
      if ((res_alloc->state == ISP_RESOURCE_FREE) &&
        (res_info->isp_pipeline->max_width >= in_w) &&
        (res_info->isp_pipeline->max_height >= in_h)) {
        hw_ids[(*num_isp)++] = hw_id;
        res_alloc->state = ISP_RESOURCE_RESERVED;
        res_alloc->session_id = session_param->session_id;
        reserved = TRUE;
        session_param->multi_pass = TRUE;
	break;
      }
   }
  }

  ISP_DBG("Reserved %d\n", reserved);
  return reserved;
}

/** isp_resource_is_mono_format:
 *
 *  @format: sesnor_fromat
 *
 *  Check if format is mono
 *
 *  Return TRUE if format is mono
 **/
static boolean isp_resource_is_mono_format(cam_format_t format)
{
  switch(format) {
    case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GREY:
    case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GREY:
    case CAM_FORMAT_BAYER_MIPI_RAW_14BPP_GREY:
    case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GREY:
    case CAM_FORMAT_Y_ONLY:
    case CAM_FORMAT_Y_ONLY_10_BPP:
    case CAM_FORMAT_Y_ONLY_12_BPP:
    case CAM_FORMAT_Y_ONLY_14_BPP:
      return TRUE;
    default:
      return FALSE;
  }
}

/** isp_resource_get_stats_type_mask:
 *
 *  @module: module handle
 *  @isp: isp handle
 *  @session_param: session param handle
 *  @stats_type_mask: stats to enable for this mode
 *
 *  Send event to 3A and isp pipeline to get stats to be
 *  enabled from ISP
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_resource_get_stats_type_mask(mct_module_t *module,
  isp_t *isp, isp_session_param_t *session_param,
  isp_stats_mask_ctrl_t *stats_mask_ctrl, boolean is_offline)
{
  boolean                       ret = TRUE;
  uint32_t                      i = 0;
  isp_hw_id_t                   hw_id = ISP_HW_MAX;
  mct_event_request_stats_type  stats_type;
  uint8_t                      *submod_enable = NULL;
  isp_pipeline_t               *isp_pipeline = NULL;
  isp_hw_params_t              *common_hw_params = NULL;
  isp_hw_module_id_t            isp_hw = ISP_MOD_MAX_NUM;
  isp_hw_info_t                *hw_info_table = NULL;
  mct_event_t                   mct_event;

  if (!module || !isp || !session_param || !stats_mask_ctrl) {
    ISP_ERR("failed: %p %p %p %p", module, isp, session_param, stats_mask_ctrl);
    return FALSE;
  }

  if (!session_param->num_isp) {
    ISP_ERR("failed: num_isp %d", session_param->num_isp);
    return FALSE;
  }

  memset(stats_mask_ctrl, 0, sizeof(isp_stats_mask_ctrl_t));
  memset(&stats_type, 0, sizeof(stats_type));

  /* Get hw_id used by left VFE */
  hw_id = session_param->hw_id[0];

  isp_pipeline = isp->isp_resource.isp_resource_info[hw_id].isp_pipeline;
  if (!isp_pipeline) {
    ISP_ERR("failed; pipeline %p", isp_pipeline);
    return FALSE;
  }

  submod_enable = isp_pipeline->submod_enable;
  common_hw_params = isp_pipeline->common_hw_params;
  if (!submod_enable || !common_hw_params) {
    ISP_ERR("failed; sumod_enable %p common_hw_params %p", submod_enable,
      common_hw_params);
    return FALSE;
  }

  hw_info_table = common_hw_params->hw_info_table;
  if (!hw_info_table) {
    ISP_ERR("failed: %p", hw_info_table);
    return FALSE;
  }

  /* Prepare supported stats type mask */
  for (i = 0; i < common_hw_params->num_hw; i++) {
    if (hw_info_table[i].stats_type != MSM_ISP_STATS_MAX) {
      isp_hw = hw_info_table[i].hw_module_id;
      if (submod_enable[isp_hw] == TRUE) {
        stats_type.supported_stats_mask |= (1 << hw_info_table[i].stats_type);
      }
    }
  }
  stats_type.num_isp_stat_comp_grp = isp_pipeline->num_stats_comp_grp;
  ISP_DBG("supported_stats_mask %x", stats_type.supported_stats_mask);

  /* Fill supported stats tap location */
  if (isp_pipeline->supported_stats_tap_location) {
    for (i = 0; i < MSM_ISP_STATS_MAX; ++i) {
      stats_type.supported_tap_location[i] =
        isp_pipeline->supported_stats_tap_location[i];
    }
  }

  if (is_offline)
    stats_type.isp_streaming_type = ISP_STREAMING_OFFLINE;
  else
    stats_type.isp_streaming_type = ISP_STREAMING_ONLINE;

  switch(session_param->sensor_fmt) {
  case CAM_FORMAT_BAYER_MIPI_RAW_8BPP_GREY:
  case CAM_FORMAT_BAYER_MIPI_RAW_10BPP_GREY:
  case CAM_FORMAT_BAYER_MIPI_RAW_12BPP_GREY:
    isp_pipeline->func_table->pipeline_update_stats_mask_for_mono_sensor(
      &stats_type.supported_stats_mask);
    break;
  default:
    break;
  }

  /* Send event to 3A to get stats needed by 3A */
  memset(&mct_event, 0, sizeof(mct_event));

  mct_event.direction = MCT_EVENT_DOWNSTREAM;
  mct_event.type = MCT_EVENT_MODULE_EVENT;
  mct_event.identity = session_param->session_based_ide;
  mct_event.u.module_event.type = MCT_EVENT_MODULE_REQUEST_STATS_TYPE;
  mct_event.u.module_event.module_event_data = (void *)&stats_type;

  ret = isp_util_forward_event_downstream_to_type(module, &mct_event,
    MCT_PORT_CAPS_STATS);
  if (ret == FALSE) {
    ISP_ERR("failed: MCT_EVENT_MODULE_REQUEST_STATS_TYPE to 3A");
  }

  /* Send event to isp pipeline */
  ret = isp_util_forward_event_to_session_based_stream_all_int_pipelines(
    session_param, &mct_event);
  if (ret == FALSE) {
    ISP_ERR("failed: MCT_EVENT_MODULE_REQUEST_STATS_TYPE to isp pipeline");
  }
  /* 3A controls the stats enable/disable, Parsing of given stats &  *
   * grouping those stats typically we have 2 composite groups in    *
   * stats. 3A can request ISP to group any of these given ISP       *
   * COMPOSITE STATS GROUP.                                          */

   /* Pass enable stats mask to ISP pipeline to map to hw mask */
   if (isp_pipeline->func_table->pipeline_map_stats_mask_to_hw) {
     ret = isp_pipeline->func_table->pipeline_map_stats_mask_to_hw(
       &stats_type.enable_stats_mask);
     if (ret == FALSE) {
       ISP_ERR("failed: pipeline_map_stats_mask_to_hw");
     }
   }

   /* Pass enable stats mask to ISP pipeline to map to hw mask */
   if (isp_pipeline->func_table->pipeline_map_stats_mask_to_hw) {
     ret = isp_pipeline->func_table->pipeline_map_stats_mask_to_hw(
       &stats_type.enable_stats_parse_mask);
     if (ret == FALSE) {
       ISP_ERR("failed: pipeline_map_stats_mask_to_hw");
     }
   }

   stats_mask_ctrl->stats_mask = stats_type.enable_stats_mask;
   stats_mask_ctrl->parse_mask = stats_type.enable_stats_parse_mask;
   stats_mask_ctrl->num_stats_comp_grp = stats_type.num_isp_stat_comp_grp;
   for (i=0; i<stats_type.num_isp_stat_comp_grp; i++)
     stats_mask_ctrl->stats_comp_grp_mask[i] =
       stats_type.stats_comp_grp_mask[i];

  if (!is_offline) {
    if (!isp_resource_is_mono_format(session_param->sensor_fmt) ||
        isp_pipeline->is_mono_stats_supported) {
      if (!(stats_mask_ctrl->stats_mask & (1 << MSM_ISP_STATS_BHIST))) {
        stats_mask_ctrl->stats_mask |= (1 << MSM_ISP_STATS_BHIST);
        /*Enable stats parsing by default for BHIST*/
        stats_mask_ctrl->parse_mask |= (1 << MSM_ISP_STATS_BHIST);
        stats_mask_ctrl->stats_comp_grp_mask[0] |= (1 << MSM_ISP_STATS_BHIST);
      }
    }
  }

  ISP_DBG("stats_mask %x parqse mask %x",
    stats_mask_ctrl->stats_mask , stats_mask_ctrl->parse_mask);

  return TRUE;

}

/** isp_resource_iface_request_offline_isp:
 *
 *  @module: module handle
 *  @port: port handle
 *  @event: IFACE request output resource event
 *
 *  Allocate ISP resource for offline reprocessing
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_resource_iface_request_offline_isp(mct_module_t *module,
  mct_port_t *port, mct_event_t *event)
{
  boolean                          ret = TRUE;
  isp_t                           *isp = NULL;
  isp_resource_t                  *resource = NULL;
  isp_session_param_t             *session_param = NULL;
  boolean                          reserved;
  isp_pipeline_stats_func_table_t *func_table = NULL;
  iface_resource_request_t        *resource_request = NULL;
  uint32_t                         num_isp;
  isp_hw_id_t                      offline_hw_id;

  if (!module || !port || !event) {
    ISP_ERR("failed: module %p port %p event %p", module, port, event);
    return FALSE;
  }

  isp = (isp_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("faild: isp %p", isp);
    return FALSE;
  }

  resource = &isp->isp_resource;

  resource_request =
    (iface_resource_request_t *)event->u.module_event.module_event_data;
  if (!resource_request) {
    ISP_ERR("failed: resource_request %p", resource_request);
    return FALSE;
  }

  memset(resource_request, 0, sizeof(*resource_request));
  ret = isp_util_get_session_params(module,
    ISP_GET_SESSION_ID(event->identity),&session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  /* Get stats mask, parse mask, stats composite group mask*/
  ret = isp_resource_get_stats_type_mask(module, isp, session_param,
    &session_param->parser_params.stats_ctrl, TRUE);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_get_stats_type_mask");
    return FALSE;
  }

  /* Check if there is available ISP for offline reprocessing */
  if (session_param->num_isp >= resource->num_isp) {
    /* TODO dual isp offline */
    ISP_ERR("failed: Offline dual isp not implemented!");
    return FALSE;
  }

  for (offline_hw_id = 0; offline_hw_id < ISP_HW_MAX; offline_hw_id++) {
      session_param->offline_hw_id[offline_hw_id] = ISP_HW_MAX;
  }
  reserved = isp_resource_allocate(module, resource, session_param->session_id,
    &session_param->offline_num_isp, session_param->offline_hw_id, 1, 0);
  if (!reserved) {
    ISP_ERR("No free isp -  not supported");
    return FALSE;
  }
  ISP_DBG("off_dbg num offline isp %d offline id %d",
          session_param->offline_num_isp, session_param->offline_hw_id[0]);

  for (num_isp = 0; num_isp < session_param->offline_num_isp; num_isp++) {
    offline_hw_id = session_param->offline_hw_id[num_isp];
    func_table =
      resource->isp_resource_info[offline_hw_id].isp_pipeline->func_table;
    if (!func_table || !func_table->pipeline_fill_stats_info) {
      ISP_ERR("failed: func_table %p", func_table);
      return FALSE;
    }
    if (func_table->pipeline_fill_stats_info && !session_param->multi_pass) {
      ret = func_table->pipeline_fill_stats_info(session_param->session_id,
        resource_request,
        &session_param->parser_params.stats_ctrl,
        session_param->fast_aec_mode, session_param->num_isp,
        &session_param->parser_params.parser_session_params);
      if (ret == FALSE) {
        ISP_ERR("failed: pipeline_fill_stats_info");
      }
    }
    ISP_DBG("off_dbg num stats stream %d", resource_request->num_stats_stream);
  }

  if (ret == TRUE) {
    /* Take copy of resource_request struct */
    memcpy(&session_param->parser_params.resource_request[ISP_STREAMING_OFFLINE],
      resource_request,
      sizeof(session_param->parser_params.resource_request[ISP_STREAMING_OFFLINE]));
  }

  return ret;
}

/** isp_resource_iface_request_offline_output_resource:
 *
 *  @module: module handle
 *  @port: port handle
 *  @event: IFACE request output resource event
 *
 *  Update IFACE request output resource event for offline stream
 *  based for current session
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_resource_iface_request_offline_output_resource(mct_module_t *module,
  mct_port_t *port, mct_event_t *event)
{
  boolean                          ret = TRUE;
  isp_t                           *isp = NULL;
  isp_session_param_t             *session_param = NULL;
  iface_resource_request_t        *resource_request = NULL;
  uint32_t                         num_isp = 0;
  uint32_t                         i = 0;
  isp_resource_t                  *resource = NULL;
  isp_resource_info_t             *resource_info = NULL;
  isp_resource_alloc_t            *resource_alloc = NULL;
  isp_pix_out_info_t              *isp_pix_output = NULL;
  isp_mapped_stream_info_t         mapped_stream_info;
  enum msm_ispif_vfe_intf          vfe_id = 0;
  isp_hw_id_t                      hw_id = 0;
  mct_list_t                      *l_stream_params = NULL;
  isp_stream_param_t              *stream_param = NULL;

  if (!module || !port || !event) {
    ISP_ERR("failed: module %p port %p event %p", module, port, event);
    return FALSE;
  }

  isp = (isp_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("faild: isp %p", isp);
    return FALSE;
  }

  resource = &isp->isp_resource;

  ret = isp_util_get_session_params(module,
    ISP_GET_SESSION_ID(event->identity),&session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  /* This stream should be started from HAL. This is offline ISP output stream.*/
  l_stream_params = mct_list_find_custom(session_param->l_stream_params,
    &event->identity, isp_util_compare_identity_from_stream_param);
  if (l_stream_params) {
    stream_param = (isp_stream_param_t *)l_stream_params->data;
    ISP_HIGH("Found stream param ide %x type %d",
      stream_param->stream_info.identity,
      stream_param->stream_info.stream_type);
  } else {
    ISP_ERR("Can not find stream param ide %x ",
      event->identity);
    return FALSE;
  }

  resource_request =
    (iface_resource_request_t *)event->u.module_event.module_event_data;
  if (!resource_request) {
    ISP_ERR("failed: resource_request %p", resource_request);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&session_param->mutex);
  PTHREAD_MUTEX_LOCK(&resource->mutex);

  /* there is free ISP - use it for offline processing */
  resource_request->num_isps = session_param->offline_num_isp;
  resource_request->max_nominal_clk =
    resource->isp_resource_info[session_param->offline_hw_id[0]].
    isp_pipeline->max_pix_clk_rate[ISP_CX_LEVEL_NOMINAL];

  for (num_isp = 0; num_isp < session_param->offline_num_isp; num_isp++) {
    hw_id = session_param->offline_hw_id[num_isp];
    resource_info = &resource->isp_resource_info[hw_id];
    resource_alloc = &resource_info->resource_alloc;
    if (resource_alloc->session_id != ISP_GET_SESSION_ID(event->identity)) {
      ISP_ERR("invalid resource session id %d event session id %d",
        resource_alloc->session_id, ISP_GET_SESSION_ID(event->identity));
      continue;
    }

    vfe_id = isp_util_get_vfe_id(hw_id);
    if (vfe_id >= VFE_MAX) {
      ISP_ERR("invalid vfe_id %d", vfe_id);
      goto ERROR;
    }
    resource_request->isp_id_mask |= 1 << vfe_id;

    resource_request->request_pix_op_clk =
      resource_info->isp_pipeline->max_pix_clk_rate[ISP_CX_LEVEL_NOMINAL];

    isp_pix_output =
      &resource_request->isp_pix_output[resource_request->num_pix_stream];

    ret = isp_util_get_stream_params(session_param, event->identity,
      &stream_param);
    if (!ret) {
      ISP_ERR("isp_util_get_stream_params failed for identity %d",
        event->identity);
      goto ERROR;
    }
    /* Hard code to use ENCODER stream for offline */
    stream_param->hw_stream[stream_param->num_hw_stream++] =
      ISP_HW_STREAM_ENCODER;
    memset(&mapped_stream_info, 0, sizeof(mapped_stream_info));
    mapped_stream_info.module = module;
    mapped_stream_info.port = port;
    mapped_stream_info.isp_pix_output = isp_pix_output;
    mapped_stream_info.num_pix_stream = 0;
    mapped_stream_info.session_param = session_param;
    mapped_stream_info.hw_id = hw_id;
    mapped_stream_info.resource_request = resource_request;
    ret = isp_resource_update_mapped_stream_info(&event->identity,
      &mapped_stream_info);
    if (ret == FALSE) {
      goto ERROR;
    }
    isp_pix_out_info_t *isp_pix_output = mapped_stream_info.isp_pix_output;
    mapped_stream_info.num_pix_stream++;
    isp_pix_output->fmt = stream_param->stream_info.fmt;
    isp_pix_output->dim.width = stream_param->stream_info.dim.width;
    isp_pix_output->dim.height = stream_param->stream_info.dim.height;
    isp_pix_output->streaming_mode = stream_param->stream_info.streaming_mode;
    isp_pix_output->mapped_mct_stream_id[isp_pix_output->num_mct_stream_mapped]
      = ISP_GET_STREAM_ID(event->identity);
    isp_pix_output->use_native_buffer = 0;
    isp_pix_output->num_mct_stream_mapped++;

    if (mapped_stream_info.num_pix_stream > 0) {
      resource_request->num_pix_stream++;
    } else {
      memset(isp_pix_output, 0 , sizeof(isp_pix_out_info_t));
    }

    ISP_HIGH("resource_request->num_pix_stream %d",
      resource_request->num_pix_stream);
    ISP_HIGH("splitinfo: is_split = %d, stripe id = %d, stripe_offset = %d",
      isp_pix_output->isp_split_output_info.stripe_id,
      isp_pix_output->isp_split_output_info.is_split,
      isp_pix_output->isp_split_output_info.right_stripe_offset);
    ISP_HIGH("splitinfo: left_output = %d, right_output = %d",
      isp_pix_output->isp_split_output_info.left_output_width,
      isp_pix_output->isp_split_output_info.right_output_width);
  }


  PTHREAD_MUTEX_UNLOCK(&resource->mutex);
  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);

  return ret;
ERROR:
  PTHREAD_MUTEX_UNLOCK(&resource->mutex);
  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
  return FALSE;
}

/** isp_resource_iface_request_output_resource:
 *
 *  @module: module handle
 *  @port: port handle
 *  @event: IFACE request output resource event
 *
 *  Update IFACe request output resource event based for
 *  current session
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_resource_iface_request_output_resource(mct_module_t *module,
  mct_port_t *port, mct_event_t *event)
{
  boolean                          ret = TRUE;
  isp_t                           *isp = NULL;
  isp_session_param_t             *session_param = NULL;
  iface_resource_request_t        *resource_request = NULL;
  uint32_t                         num_isp = 0;
  uint32_t                         i = 0;
  isp_resource_t                  *resource = NULL;
  isp_resource_info_t             *resource_info = NULL;
  isp_resource_alloc_t            *resource_alloc = NULL;
  isp_hw_streamid_t                hw_stream = 0;
  isp_pix_out_info_t              *isp_pix_output = NULL;
  isp_mapped_stream_info_t         mapped_stream_info;
  enum msm_ispif_vfe_intf          vfe_id = 0;
  isp_hw_id_t                      hw_id = 0;
  isp_pipeline_stats_func_table_t *func_table = NULL;
  uint32_t                         stats_type_mask = 0;
  isp_pipeline_t                  *isp_pipeline = NULL;

  if (!module || !port || !event) {
    ISP_ERR("failed: module %p port %p event %p", module, port, event);
    return FALSE;
  }

  isp = (isp_t *)MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("faild: isp %p", isp);
    return FALSE;
  }

  resource = &isp->isp_resource;

  ret = isp_util_get_session_params(module, ISP_GET_SESSION_ID(event->identity),
    &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_params ret %d %p", ret,
      session_param);
    return FALSE;
  }

  /* Get stats mask, parse mask, stats composite group mask*/
  ret = isp_resource_get_stats_type_mask(module, isp, session_param,
    &session_param->parser_params.stats_ctrl, FALSE);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_util_get_stats_type_mask");
  }

  resource_request =
    (iface_resource_request_t *)event->u.module_event.module_event_data;
  if (!resource_request) {
    ISP_ERR("failed: resource_request %p", resource_request);
    return FALSE;
  }

  /* Get hw_id used by left VFE */
  hw_id = session_param->hw_id[0];
  isp_pipeline = isp->isp_resource.isp_resource_info[hw_id].isp_pipeline;
  if (!isp_pipeline || (hw_id >= ISP_HW_MAX)) {
    ISP_ERR("failed; pipeline %p", isp_pipeline);
     return FALSE;
  }
  hw_id = 0;

  PTHREAD_MUTEX_LOCK(&session_param->mutex);
  PTHREAD_MUTEX_LOCK(&resource->mutex);
  memset(resource_request, 0, sizeof(*resource_request));
  resource_request->num_isps = session_param->num_isp;
  resource_request->request_pix_op_clk = resource->request_op_clk;
  resource_request->hw_version = isp_pipeline->hw_version;
  resource_request->camif_cap.is_camif_raw_crop_supported=
    isp_pipeline->is_camif_crop_supported;
  resource_request->camif_cap.is_camif_raw_op_fmt_supported =
    isp_pipeline->is_camif_raw_op_fmt_supported;

  ISP_HIGH("num_isps %d", resource_request->num_isps);

  /*request ispif stripe info*/
  if (session_param->num_isp > 1) {
    ret = isp_resource_request_stripe_info(resource,
      resource_request, session_param, hw_id);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_resource_request_stripe_info");
    }
  }

  for (num_isp = 0; num_isp < session_param->num_isp; num_isp++) {
    hw_id = session_param->hw_id[num_isp];
    resource_info = &resource->isp_resource_info[hw_id];
    resource_alloc = &resource_info->resource_alloc;
    if (resource_alloc->session_id != ISP_GET_SESSION_ID(event->identity)) {
      ISP_ERR("invalid resource session id %d event session id %d",
        resource_alloc->session_id, ISP_GET_SESSION_ID(event->identity));
      continue;
    }

    func_table = resource->isp_resource_info[hw_id].isp_pipeline->func_table;
    if (!func_table || !func_table->pipeline_fill_stats_info) {
      ISP_ERR("failed: func_table %p", func_table);
      goto ERROR;
    }

    vfe_id = isp_util_get_vfe_id(hw_id);
    if (vfe_id >= VFE_MAX) {
      ISP_ERR("invalid vfe_id %d", vfe_id);
      goto ERROR;
    }
    resource_request->isp_id_mask |= 1 << vfe_id;
    resource_request->max_nominal_clk =
      resource->isp_resource_info[vfe_id].isp_pipeline->max_pix_clk_rate[ISP_CX_LEVEL_NOMINAL];

    /*loop through per hw stream*/
    for (hw_stream = 0; hw_stream < ISP_HW_STREAM_MAX; hw_stream++) {
      if (!resource_alloc->l_identity[hw_stream]) {
        continue;
      }
      isp_pix_output =
        &resource_request->isp_pix_output[resource_request->num_pix_stream];

      memset(&mapped_stream_info, 0, sizeof(mapped_stream_info));
      mapped_stream_info.module = module;
      mapped_stream_info.port = port;
      mapped_stream_info.isp_pix_output = isp_pix_output;
      mapped_stream_info.num_pix_stream = 0;
      mapped_stream_info.session_param = session_param;
      mapped_stream_info.hw_id = hw_id;
      mapped_stream_info.resource_request = resource_request;
      mct_list_traverse(resource_alloc->l_identity[hw_stream],
        isp_resource_update_mapped_stream_info, &mapped_stream_info);

      /*if split, two ISP axi output is sharing the same hw stream*/
      if (resource_request->ispif_split_output_info.is_split == TRUE) {
         for (i = 0; i < resource_request->num_pix_stream; i++) {
           if (resource_request->isp_pix_output[i].axi_path ==
               isp_pix_output->axi_path) {
              mapped_stream_info.num_pix_stream = 0;
           }
         }
      }

      if (mapped_stream_info.num_pix_stream > 0) {
        resource_request->num_pix_stream++;
      } else {
        memset(isp_pix_output, 0 , sizeof(isp_pix_out_info_t));
      }

      ISP_HIGH("resource_request->num_pix_stream %d",
        resource_request->num_pix_stream);
      ISP_DBG("split info: is_split = %d, stripe id = %d, stripe_offset = %d",
        isp_pix_output->isp_split_output_info.is_split,
        isp_pix_output->isp_split_output_info.stripe_id,
        isp_pix_output->isp_split_output_info.right_stripe_offset);
      ISP_DBG("split info: left_output = %d, right_output = %d",
        isp_pix_output->isp_split_output_info.left_output_width,
        isp_pix_output->isp_split_output_info.right_output_width);
    }

    /*fill in nstats need to happen after stripe info decided,
      to decide stats buf shared or private*/
    if (func_table->pipeline_fill_stats_info) {
      ret = func_table->pipeline_fill_stats_info(session_param->session_id,
        resource_request, &session_param->parser_params.stats_ctrl,
        session_param->fast_aec_mode, session_param->num_isp,
         &session_param->parser_params.parser_session_params);
      if (ret == FALSE) {
        ISP_ERR("failed: pipeline_fill_stats_info");
      }
    } else {
      ISP_ERR("failed: pipeline_fill_stats_info NULL");
    }
  }

  if (ret == TRUE) {
    /* Take copy of resource_request struct */
    memcpy(&session_param->parser_params.resource_request[ISP_STREAMING_ONLINE],
      resource_request,
      sizeof(session_param->parser_params.resource_request[ISP_STREAMING_ONLINE]));
  }

  PTHREAD_MUTEX_UNLOCK(&resource->mutex);
  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);

  return ret;
ERROR:
  PTHREAD_MUTEX_UNLOCK(&resource->mutex);
  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
  return FALSE;
}

/** isp_resource_update_superdim_in_resource_info:
 *
 *  @data: handle to identity
 *  @user_data: handle to isp_update_dim_t
 *
 *  Update dim struct present in resource info based on
 *  stream param for current identity
 *
 *  Return TRUE
 **/
static boolean isp_resource_update_superdim_in_resource_info(void *data,
  void *user_data)
{
  boolean             ret = TRUE;
  uint32_t           *identity = (uint32_t *)data;
  isp_update_dim_t   *update_dim = (isp_update_dim_t *)user_data;
  isp_stream_param_t *stream_param = NULL;
  cam_dimension_t    *stream_dim = NULL;
  isp_dim_t          *res_dim = NULL;

  if (!data || !user_data) {
    ISP_ERR("failed: data %p user_data %p", data, user_data);
    return TRUE;
  }

  if (!update_dim->session_param || !update_dim->dim) {
    ISP_ERR("failed: session_param %p dim %p", update_dim->session_param,
      update_dim->dim);
    return TRUE;
  }

  res_dim = update_dim->dim;

  ret = isp_util_get_stream_params(update_dim->session_param, *identity,
    &stream_param);
  if (ret == FALSE || !stream_param) {
    ISP_ERR("failed: isp_util_get_stream_params ret %d stream param %p", ret,
      stream_param);
    return TRUE;
  }

  stream_dim = &stream_param->stream_info.dim;
  if (stream_dim->width > res_dim->width) {
    res_dim->width = stream_dim->width;
  }
  if (stream_dim->height > res_dim->height) {
    res_dim->height = stream_dim->height;
  }

  return TRUE;
}

/** isp_resource_is_right_stripe_offset_usable:
 *
 *  @M: N
 *  @N: N
 *  @offset: offset
 *
 *  Return TRUE if right stripe offset is usable, FALSE
 *  otherwise
 **/
static boolean isp_resource_is_right_stripe_offset_usable(
  uint32_t M, uint32_t N, uint32_t offset)
{
  uint32_t mn_init, step;
  uint32_t ratio = N / M;
  uint32_t interp_reso = 3;
  if (ratio >= 16) interp_reso = 0;
  else if (ratio >= 8) interp_reso = 1;
  else if (ratio >= 4) interp_reso = 2;

  /* upscaling */
  if (N < M)
    return TRUE;

  mn_init = offset * M % N;
  step = mn_init * (1 << (13 + interp_reso)) / M >> 13;
  if (step == 0)
    return TRUE;

  mn_init = (offset + 1) * M % N;
  step = mn_init * (1 << (13 + interp_reso)) / M >> 13;

  return (step != 0 && mn_init < M);
}

static void isp_resource_free_isp(isp_resource_t *isp_resource,
  isp_session_param_t *session_param, isp_hw_id_t hw_id)
{
  isp_hw_update_list_params_t   *hw_update_list_params = NULL;
  isp_resource_info_t           *res_info = NULL;
  isp_resource_alloc_t          *res_alloc = NULL;

  res_info = &isp_resource->isp_resource_info[hw_id];
  res_alloc = &res_info->resource_alloc;
  res_alloc->state = ISP_RESOURCE_FREE;
  res_alloc->session_id = 0;

  ISP_DBG("session %d hw_id %d ", session_param->session_id, hw_id);

  hw_update_list_params =
    &session_param->hw_update_params.hw_update_list_params[hw_id];
  /* close fd */
  if (hw_update_list_params->fd > 0) {
#ifdef VIDIOC_MSM_ISP_AHB_CLK_CFG
    isp_pipeline_t *isp_pipeline = isp_resource->isp_resource_info[hw_id].isp_pipeline;
    if (isp_pipeline->is_ahb_clk_cfg_supported == TRUE) {
      struct msm_isp_ahb_clk_cfg ahb_param;
      if (MSM_ISP_CAMERA_AHB_SVS_VOTE != hw_update_list_params->ahb_clk_vote) {
        ahb_param.vote = MSM_ISP_CAMERA_AHB_SVS_VOTE;
        ioctl(hw_update_list_params->fd, VIDIOC_MSM_ISP_AHB_CLK_CFG, &ahb_param);
      }
      hw_update_list_params->ahb_clk_vote = MSM_ISP_CAMERA_AHB_SVS_VOTE;
    }
#endif
    close(hw_update_list_params->fd);
    hw_update_list_params->fd = 0;
  }
}

/** isp_resource_deallocate:
 *
 *  @module: module handle
 *  @isp_resource: ISP resource handle
 *  @session_id: session id
 *
 *  Release ISP resource
 *
 *  Return TRUE on successful and FALSE on failure
 **/
boolean isp_resource_deallocate(mct_module_t *module,
  isp_resource_t *isp_resource, uint32_t session_id,
  uint32_t *num_isp, isp_hw_id_t *hw_ids)
{
  isp_session_param_t *session_param;
  uint32_t             i;
  boolean              ret = TRUE;

  ret = isp_util_get_session_params(module, session_id, &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_params ret %d %p",
      ret, session_param);
    return FALSE;
  }

  for (i = 0; i < *num_isp; i++) {
    isp_resource_free_isp(isp_resource, session_param, hw_ids[i]);
    session_param->isp_ports.isp_submod_ports[hw_ids[i]] = NULL;
    hw_ids[i] = ISP_HW_MAX;
  }
  *num_isp = 0;

  return ret;
}

/** isp_stream_resource_deallocate:
 *
 *  @module: module handle
 *  @session_param: session specific params
 *  @stream_param: stream specific params
 *  @isp_resource: ISP resource handle
 *
 *  Deallocate ISP resource based on request
 *
 *  Return void
 **/
static void isp_stream_resource_deallocate(
    mct_module_t        *module,
    isp_session_param_t *session_param,
    isp_stream_param_t  *stream_param,
    isp_resource_t      *isp_resource)
{
  isp_hw_id_t                    hw_id = 0;
  isp_resource_info_t           *res_info = NULL;
  isp_resource_alloc_t          *res_alloc = NULL;
  isp_hw_streamid_t              hw_stream_id;
  mct_list_t                    *l_identity = NULL;
  void                          *temp_data = NULL;
  if (!module || !session_param || !stream_param || !isp_resource) {
    ISP_ERR("failed %p %p %p %p", module, session_param, stream_param, isp_resource);
    return;
  }

  /* Lock resource manager params, unlock this on every return path */
  PTHREAD_MUTEX_LOCK(&isp_resource->mutex);

  /* Find which ISP resource is assigned for this stream */
  for (hw_id = 0; hw_id < isp_resource->num_isp; hw_id++) {
    res_info = &isp_resource->isp_resource_info[hw_id];
    res_alloc = &res_info->resource_alloc;
    if ((res_alloc->state != ISP_RESOURCE_FREE) ||
        (res_alloc->session_id == session_param->session_id)) {
      /* Find the hw stream that is assigned for current stream */
      for (hw_stream_id = 0; hw_stream_id < ISP_HW_STREAM_MAX; hw_stream_id++) {
        l_identity = mct_list_find_custom(res_alloc->l_identity[hw_stream_id],
          &stream_param->stream_info.identity, isp_util_compare_identity);
        if (l_identity) {
          /* Remove identity from resource */
          temp_data = l_identity->data;
          res_alloc->l_identity[hw_stream_id] = mct_list_remove(
            res_alloc->l_identity[hw_stream_id], l_identity->data);
          free(temp_data);
          res_alloc->num_streams--;
          ISP_DBG("res_alloc->num_streams %d", res_alloc->num_streams);
        }
      }
    }
  }

  PTHREAD_MUTEX_UNLOCK(&isp_resource->mutex);

}

/** isp_resource_unlink_modules:
 *
 *  @stream: new stream
 *  @port: ISP sink port
 *  @isp_resource_info: ISP resource info handle
 *  @session_param: session param handle
 *  @stream_param: stream param handle
 *  @hw_id: ISP hw id
 *
 *  Caps reserve and ext link sumoodules and add to stream
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_resource_unlink_modules(mct_stream_t *stream,
  mct_port_t *port, isp_resource_info_t *isp_resource_info,
  isp_session_param_t *session_param, isp_stream_param_t *stream_param,
  isp_hw_id_t hw_id)
{
  boolean            ret = TRUE;
  uint32_t           submod_id = 0;
  mct_module_t      *mod1 = NULL, *mod2 = NULL, *store_common_mod = NULL;
  mct_port_t        *submod_port = NULL;
  isp_port_data_t   *port_data = NULL;
  isp_submod_info_t *submod_info = NULL;
  uint32_t           hw_size = 0;
  isp_hw_streamid_t  hw_stream = ISP_HW_STREAM_MAX;
  uint32_t           i = 0;
  mct_module_t      *port_module = NULL;

  if (!stream || !port || !isp_resource_info || !session_param ||
      !stream_param) {
    ISP_ERR("failed: %p %p %p %p %p", stream, port, isp_resource_info,
            session_param, stream_param);
    return FALSE;
  }

  if (stream_param->num_hw_stream > ISP_HW_STREAM_MAX) {
    ISP_ERR("failed: stream_param->num_hw_stream %d",
      stream_param->num_hw_stream);
    return FALSE;
  }

  port_data = MCT_OBJECT_PRIVATE(port);
  if (!port_data) {
    ISP_ERR("failed: port_data %p", port_data);
    return FALSE;
  }

  ISP_DBG("type %d", stream_param->stream_info.stream_type);

  submod_info = isp_resource_info->common_hw_info;
  hw_size = isp_resource_info->common_hw_size;

  for (submod_id = 0; submod_id < hw_size; submod_id++) {
    if (submod_info[submod_id].isp_submod) {
      mod1 = submod_info[submod_id].isp_submod;
      break;
    }
  }

  if (!mod1) {
    ISP_ERR("no modules found");
    return FALSE;
  }
  submod_port = session_param->isp_ports.isp_submod_ports[hw_id];
  if (!submod_port) {
    ISP_ERR("submod_port %p ide %x", submod_port, stream->streaminfo.identity);
    return FALSE;
  }
  submod_port->un_link(stream->streaminfo.identity, submod_port, port);
  ret = submod_port->check_caps_unreserve(submod_port,
    stream->streaminfo.identity);

  mct_port_remove_child(stream->streaminfo.identity, submod_port);

  port_module = (mct_module_t *)(MCT_PORT_PARENT(submod_port)->data);
  /* Update port private data */
  session_param->isp_ports.isp_submod_ports[hw_id] = submod_port;
  ISP_DBG("vfe %d isp_submod_ports %p port parent %s", hw_id,
    session_param->isp_ports.isp_submod_ports[hw_id],
    MCT_MODULE_NAME(port_module));

  for (submod_id++; submod_id < hw_size; submod_id++) {
    if (!submod_info[submod_id].isp_submod) {
      continue;
    }
    mod2 = submod_info[submod_id].isp_submod;

    ISP_DBG("unlink %s %s", MCT_MODULE_NAME(mod1), MCT_MODULE_NAME(mod2));
    stream->unlink(stream, mod1, mod2);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_stream_link_modules %s %s", MCT_MODULE_NAME(mod1),
        MCT_MODULE_NAME(mod2));
      goto ERROR;
    }
    mod1 = mod2;
    store_common_mod = mod1;
  }

  if (!store_common_mod) {
    ISP_ERR("failed: no modules found in common hw stream");
    return FALSE;
  }

  for (i = 0; i < stream_param->num_hw_stream; i++) {
    hw_stream = stream_param->hw_stream[i];
    mod1 = store_common_mod;

    submod_info = isp_resource_info->stream_hw_info[hw_stream];
    hw_size = isp_resource_info->stream_hw_size[hw_stream];

    for (submod_id = 0; submod_id < hw_size; submod_id++) {
      if (!submod_info[submod_id].isp_submod) {
        continue;
      }
      mod2 = submod_info[submod_id].isp_submod;

      ISP_DBG("unlink %s %s", MCT_MODULE_NAME(mod1), MCT_MODULE_NAME(mod2));
      stream->unlink(stream, mod1, mod2);
      if (ret == FALSE) {
        ISP_ERR("failed: mct_stream_link_modules %s %s", MCT_MODULE_NAME(mod1),
          MCT_MODULE_NAME(mod2));
        goto ERROR;
      }
      mod1 = mod2;
    }
  }

  mct_list_free_all_on_data(MCT_OBJECT_CHILDREN(stream),
    mct_stream_remove_stream_from_module, stream);
  MCT_OBJECT_CHILDREN(stream) = NULL;
  MCT_STREAM_NUM_CHILDREN(stream) = 0;
  free(stream);

  return ret;

ERROR:
  /* TODO handle error here */
  return ret;
}

/** isp_stream_resource_deallocate:
 *
 *  @module: module handle
 *  @session_param: session specific params
 *  @stream_param: stream specific params
 *  @isp_resource: ISP resource handle
 *
 *  Deallocate ISP resource based on request
 *
 *  Return void
 **/
static void isp_stream_resource_deallocate_hw_id(mct_module_t *module,
  isp_session_param_t *session_param, isp_stream_param_t *stream_param,
  isp_resource_t *isp_resource, isp_hw_id_t hw_id)
{
  isp_resource_info_t           *res_info = NULL;
  isp_resource_alloc_t          *res_alloc = NULL;
  isp_hw_streamid_t              hw_stream_id;
  mct_list_t                    *l_identity = NULL;
  void                          *temp_data = NULL;
  if (!session_param || !stream_param || !isp_resource) {
    ISP_ERR("failed %p %p %p", session_param, stream_param, isp_resource);
    return;
  }

  /* Lock resource manager params, unlock this on every return path */
  PTHREAD_MUTEX_LOCK(&isp_resource->mutex);

  /* Find which ISP resource is assigned for this stream */
  res_info = &isp_resource->isp_resource_info[hw_id];
  res_alloc = &res_info->resource_alloc;
  if ((res_alloc->state != ISP_RESOURCE_FREE) ||
      (res_alloc->session_id == session_param->session_id)) {
    /* Find the hw stream that is assigned for current stream */
    for (hw_stream_id = 0; hw_stream_id < ISP_HW_STREAM_MAX; hw_stream_id++) {
      l_identity = mct_list_find_custom(res_alloc->l_identity[hw_stream_id],
        &stream_param->stream_info.identity, isp_util_compare_identity);
      if (l_identity) {
        /* Remove identity from resource */
        temp_data = l_identity->data;
        res_alloc->l_identity[hw_stream_id] = mct_list_remove(
          res_alloc->l_identity[hw_stream_id], l_identity->data);
        free(temp_data);
        res_alloc->num_streams--;
      }
    }
    if (!res_alloc->num_streams) {
      isp_resource_deallocate(module, isp_resource, session_param->session_id,
        &session_param->offline_num_isp, session_param->offline_hw_id);
    }
  }

  PTHREAD_MUTEX_UNLOCK(&isp_resource->mutex);
}

/** isp_resource_destroy_internal_link_hw_id:
 *
 *  @port: port
 *  @port_data: port private data
 *  @identity: identity
 *
 *  Destroy internal link by unlink and caps unreserve
 *
 *  Return void
 **/
void isp_resource_destroy_internal_link_hw_id(mct_port_t *port,
  isp_port_data_t *port_data, uint32_t identity, isp_hw_id_t hw_id)
{
  int32_t              rc = 0;
  boolean              ret = TRUE;
  mct_stream_t        *int_stream = NULL;
  mct_module_t        *module = NULL;
  isp_t               *isp = NULL;
  isp_session_param_t *session_param = NULL;
  isp_stream_param_t  *stream_param = NULL;
  isp_resource_t      *isp_resource = NULL;
  mct_port_t          *submod_port = NULL;

  if (!port || !port_data) {
    ISP_ERR("failed: %p %p", port, port_data);
    return;
  }

  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed");
    return;
  }

  isp = MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed");
    return;
  }

  isp_resource = &isp->isp_resource;

  ret = isp_util_get_session_stream_params(module, identity, &session_param,
    &stream_param);
  if (ret == FALSE || !session_param || !stream_param) {
    ISP_ERR("failed: isp_util_get_session_stream_params ret %d %p %p",
      ret, session_param, stream_param);
    return;
  }

  /* Lock ISP session params, unlock this on every return path */
  PTHREAD_MUTEX_LOCK(&session_param->mutex);

  int_stream = stream_param->int_stream[hw_id];
  stream_param->int_stream[hw_id] = NULL;
  if (!int_stream) {
    PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
    return;
  }
  submod_port = session_param->isp_ports.isp_submod_ports[hw_id];
  if (!submod_port) {
    PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
    return;
  }
  submod_port->un_link(identity, submod_port, port);

  /* Check if this stream is present in this submod port or destroyed already
        for offline */
  if (mct_list_find_custom(MCT_PORT_CHILDREN(submod_port), &identity,
  isp_util_compare_identity) == NULL) {
    ISP_HIGH("ide %x is already removed from submod_port", identity);
    PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
    return;
  }
  rc = submod_port->check_caps_unreserve(submod_port, identity);
  if (rc == FALSE) {
    ISP_ERR("failed");
  }

  mct_port_remove_child(identity, submod_port);

  if (MCT_OBJECT_CHILDREN(int_stream)) {
    mct_list_operate_nodes(MCT_OBJECT_CHILDREN(int_stream),
      mct_stream_operate_unlink, int_stream);
    mct_list_free_all_on_data(MCT_OBJECT_CHILDREN(int_stream),
      mct_stream_remove_stream_from_module, int_stream);
    MCT_OBJECT_CHILDREN(int_stream) = NULL;
    MCT_STREAM_NUM_CHILDREN(int_stream) = 0;
  }
  free(int_stream);

  isp_stream_resource_deallocate_hw_id(module, session_param, stream_param,
    isp_resource, hw_id);

  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
  return;
}

/** isp_resource_destroy_internal_link:
 *
 *  @port: port
 *  @identity: identity
 *
 *  Destroy internal link by unlink and caps unreserve
 *
 * Return TRUE on success and FALSE if streams could not be mapped
 **/
boolean isp_resource_destroy_internal_link(mct_port_t *port, uint32_t identity)
{
  int32_t              rc = 0;
  boolean              ret = TRUE;
  mct_stream_t        *int_stream = NULL;
  mct_module_t        *module = NULL;
  isp_t               *isp = NULL;
  isp_session_param_t *session_param = NULL;
  isp_stream_param_t  *stream_param = NULL;
  isp_hw_id_t          hw_id;
  isp_resource_t      *isp_resource = NULL;
  mct_port_t          *submod_port = NULL;
  uint32_t             common_hw_size = 0, sub_mod = 0;
  mct_module_t        *mod1 = NULL, *mod2 = NULL;

  if (!port) {
    ISP_ERR("failed: port %p", port);
    return FALSE;
  }

  module = (mct_module_t *)(MCT_PORT_PARENT(port)->data);
  if (!module) {
    ISP_ERR("failed");
    return FALSE;
  }

  isp = MCT_OBJECT_PRIVATE(module);
  if (!isp) {
    ISP_ERR("failed");
    return FALSE;
  }

  isp_resource = &isp->isp_resource;

  ret = isp_util_get_session_stream_params(module, identity, &session_param,
    &stream_param);
  if (ret == FALSE || !session_param || !stream_param) {
    ISP_ERR("failed: isp_util_get_session_stream_params ret %d %p %p",
      ret, session_param, stream_param);
    return FALSE;
  }

   /* clean up stream mapping */
  if (session_param->stream_port_map.num_streams){
    session_param->stream_port_map.num_streams = 0;
    memset(&session_param->preferred_mapping, 0,
          sizeof(session_param->preferred_mapping));
    memset(session_param->stream_port_map.streams, 0,
          sizeof(session_param->stream_port_map.streams));
  }

  /* Lock ISP session params, unlock this on every return path */
  PTHREAD_MUTEX_LOCK(&session_param->mutex);

  for (hw_id = 0; hw_id < ISP_HW_MAX; hw_id++) {
    int_stream = stream_param->int_stream[hw_id];
    stream_param->int_stream[hw_id] = NULL;
    if (!int_stream) {
      continue;
    }
    isp_resource_unlink_modules(int_stream, port,
      &isp_resource->isp_resource_info[hw_id], session_param, stream_param,
      hw_id);
  }

  isp_stream_resource_deallocate(module, session_param, stream_param,
    isp_resource);

  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);

  return TRUE;
}

/** sort_isp_hw_by_size
 *
 *  @res_info: array of ISP resources info
 *  @num_isp: number of ISP HWs
 *  @sorted_ids: array of sorted IDs
 *
 *  Sort array of IDs by supported input size
 *
 * Return: none
 **/
static void sort_isp_hw_by_size(isp_resource_info_t *res_info, uint32_t num_isp,
  uint32_t *sorted_ids)
{
  isp_pipeline_t *isp_pipeline_cur, *isp_pipeline_prv;
  uint32_t i, tmp;
  int32_t j;

  for(i = 0; i < num_isp; i++) {
    sorted_ids[i] = i;
  }

  for(i = 0; i < num_isp; i++) {
    for (j = i; j > 0; j--) {
      isp_pipeline_cur = res_info[sorted_ids[j]].isp_pipeline;
      isp_pipeline_prv = res_info[sorted_ids[j - 1]].isp_pipeline;

      /* Sort by size. The smallest first */
      if ((isp_pipeline_cur->max_width * isp_pipeline_cur->max_height) <=
          (isp_pipeline_prv->max_width * isp_pipeline_prv->max_height)) {
        tmp = sorted_ids[j];
        sorted_ids[j] = sorted_ids[j - 1];
        sorted_ids[j - 1] = tmp;
      }
    }
  }
}

/** isp_resource_allocate:
 *
 *  @module: module handle
 *  @isp_resource: ISP resource handle
 *  @session_id: session id
 *  @num_isp: number of reserved ISPs
 *  @hw_ids: which ISP are reserved
 *
 *  Acquire ISP resource
 *
 *  Return TRUE on successful and FALSE on failure
 **/
boolean isp_resource_allocate(mct_module_t *module,
  isp_resource_t *isp_resource, uint32_t session_id, uint32_t *num_isp,
  isp_hw_id_t *hw_ids, uint8_t offline, uint32_t request_dual_isp)
{
  isp_session_param_t           *session_param = NULL;
  isp_hw_update_list_params_t   *hw_update_list_params = NULL;
  isp_resource_info_t           *res_info = NULL;
  isp_resource_alloc_t          *res_alloc = NULL;
  isp_hw_id_t                    i, j, hw_id;
  boolean                        ret, reserved;

  ret = isp_util_get_session_params(module, session_id, &session_param);
  if (ret == FALSE || !session_param) {
    ISP_ERR("failed: isp_util_get_session_params ret %d %p",
      ret, session_param);
    return FALSE;
  }

  PTHREAD_MUTEX_LOCK(&isp_resource->mutex);

  if (!offline) {
    reserved = isp_resource_reserve_isp(isp_resource, session_param, num_isp,
      hw_ids, request_dual_isp);
  } else {
    reserved = isp_resource_reserve_offline_isp(isp_resource, session_param,
      num_isp, hw_ids);
  }

  /* open fd */
  if (reserved) {
    for (i = 0; i < *num_isp; i++) {
      hw_id = hw_ids[i];
      hw_update_list_params =
        &session_param->hw_update_params.hw_update_list_params[hw_id];
      res_info = &isp_resource->isp_resource_info[hw_id];
      hw_update_list_params->fd = open(res_info->subdev_name,
        O_RDWR | O_NONBLOCK);
      /* Retry opening the device */
      if (hw_update_list_params->fd < 0) {
        ISP_ERR("failed: retrying!");
        usleep(100 * 1000);
        hw_update_list_params->fd = open(res_info->subdev_name,
          O_RDWR | O_NONBLOCK);
      }
      if (hw_update_list_params->fd < 0) {
        ISP_ERR("failed: fd %d", hw_update_list_params->fd);
        reserved = FALSE;
#ifndef VIDIOC_MSM_ISP_AHB_CLK_CFG
      }
#else
      } else {
          isp_pipeline_t *isp_pipeline = res_info->isp_pipeline;
          if (isp_pipeline->is_ahb_clk_cfg_supported == TRUE) {
            struct msm_isp_ahb_clk_cfg ahb_param;
            ahb_param.vote = MSM_ISP_CAMERA_AHB_TURBO_VOTE;
            /* set the ahb clk to max */
            if (ioctl(hw_update_list_params->fd, VIDIOC_MSM_ISP_AHB_CLK_CFG, &ahb_param)) {
              ISP_ERR("failed: VIDIOC_MSM_ISP_AHB_CFG %d\n", errno);
              reserved = FALSE;
              hw_update_list_params->ahb_clk_vote = MSM_ISP_CAMERA_AHB_SVS_VOTE;
            } else {
              hw_update_list_params->ahb_clk_vote = MSM_ISP_CAMERA_AHB_TURBO_VOTE;
            }
        }
     }
#endif
    }
  }

  /* Release resources if reservation fails */
  if (!reserved) {
    for (i = 0; i < *num_isp; i++) {
      isp_resource_free_isp(isp_resource, session_param, hw_ids[i]);
      hw_ids[i] = ISP_HW_MAX;
    }
    *num_isp = 0;
    ISP_ERR("failed: ISP resource can not be acquired");
  } else {
    ISP_INFO("INFO: ISP resource acquired: session %d num_isp %d",
      session_param->session_id, *num_isp);
    for (i = 0; i < *num_isp; i++) {
      ISP_INFO("INFO: ISP resource acquired: hw_id %d", hw_ids[i]);
    }
  }

  PTHREAD_MUTEX_UNLOCK(&isp_resource->mutex);

  return reserved;
}

/** isp_stream_resource_allocate:
 *
 *  @session_param: session specific params
 *  @stream_param: stream specific params
 *  @isp_resource: ISP resource handle
 *  @stream_info: mct stream info
 *
 *  Update session params and resource
 *
 *  Return TRUE on successful and FALSE on failure
 **/
static boolean isp_stream_resource_allocate(isp_session_param_t *session_param,
  isp_stream_param_t *stream_param, isp_resource_t *isp_resource,
  mct_stream_info_t *stream_info)
{
  boolean                        ret = TRUE;
  uint32_t                      *identity = NULL;
  isp_hw_id_t                    res_isp;
  isp_hw_id_t                    hw_id = 0, session_isp = 0;
  isp_resource_info_t           *res_info = NULL;
  isp_resource_alloc_t          *res_alloc = NULL;
  mct_list_t                    *l_identity = NULL;
  boolean                        hw_stream_id[ISP_HW_STREAM_MAX] = {FALSE};
  uint32_t                       num_hw_stream = 0;
  uint32_t                       session_id = 0;
  isp_resource_hw_stream_info_t *hw_stream_info = NULL;
  boolean                        reserved = FALSE;
  isp_stream_port_map_info_t    *streams = NULL;
  uint32_t                       i;
  char                           value[PROPERTY_VALUE_MAX];
  uint32_t                       dual_vfe_enabled;
  int32_t                        width, height;

  if (!session_param || !isp_resource || !stream_info) {
    ISP_ERR("failed: session_param %p isp_resource %p", session_param,
      isp_resource);
    return FALSE;
  }

  ISP_HIGH("stream_port_map num streams %d",
    session_param->stream_port_map.num_streams);
  for (i = 0; i < session_param->stream_port_map.num_streams; i++) {
    ISP_HIGH("INFO: type %d resolution %dx%d hw_stream %d native_buff %d ",
      session_param->stream_port_map.streams[i].stream_type,
      session_param->stream_port_map.streams[i].stream_sizes.width,
      session_param->stream_port_map.streams[i].stream_sizes.height,
      session_param->stream_port_map.streams[i].hw_stream,
      session_param->stream_port_map.streams[i].native_buffer);
  }
  /* Lock resource manager params, unlock this on every return path */
  PTHREAD_MUTEX_LOCK(&isp_resource->mutex);

  /*enforce dual vfe enable by set property*/
  property_get("persist.vendor.camera.isp.dualisp", value, "0");
  dual_vfe_enabled = atoi(value);
  if (dual_vfe_enabled == 1 && !session_param->stream_port_map.raw_stream_exists) {
    session_param->num_isp = 2;
    ISP_INFO("enforce DUAL VFE mode! session_parm num isp = 2");
  } else if(dual_vfe_enabled == 1) {
    ISP_INFO("enforce DUAL VFE mode! fail, RAW stream exists, num isp = 1");
  }

  session_id = ISP_GET_SESSION_ID(stream_info->identity);
  stream_param->num_hw_stream = 0;

  /* Iterate through each ISP resource and allocate resource */
  for (res_isp = 0; res_isp < session_param->num_isp &&
         res_isp < ISP_HW_MAX; res_isp++) {
    hw_id = session_param->hw_id[res_isp];
    res_info = &isp_resource->isp_resource_info[hw_id];
    res_alloc = &res_info->resource_alloc;

    streams = session_param->stream_port_map.streams;
    if ((stream_param->stream_info.stream_type != CAM_STREAM_TYPE_PARM) &&
      (stream_param->stream_info.stream_type != CAM_STREAM_TYPE_RAW)) {
      for (i = 0; i < session_param->stream_port_map.num_streams; i++) {
        /* Look for the stream that needs to be linked, and also make
           sure we are not picking already mapped stream.
           This can happen when streams are of same type and same
           dimension too */
        if (((stream_param->stream_info.stream_type ==
          streams[i].stream_type) &&
          (stream_param->stream_info.dim.width ==
          streams[i].orig_stream_sizes.width) &&
          (stream_param->stream_info.dim.height ==
          streams[i].orig_stream_sizes.height)) &&
          !(streams[i].mapped_mask & 1 << hw_id)) {

          streams[i].mapped_mask |= 1 << hw_id;
          hw_stream_id[streams[i].hw_stream] = TRUE;
          ISP_DBG("i %d hw_stream %d", streams[i].hw_stream,
            hw_stream_id[streams[i].hw_stream]);

          hw_stream_info =
            &res_info->isp_pipeline->hw_stream_info[streams[i].hw_stream];
          ISP_DBG("hw_id %d hw_stream_id %d stream_type %d",
            hw_id, hw_stream_id[streams[i].hw_stream],
            stream_param->stream_info.stream_type);
          stream_info->dim.width = streams[i].stream_sizes.width;
          stream_info->dim.height = streams[i].stream_sizes.height;
          stream_info->original_dim.width = streams[i].orig_stream_sizes.width;
          stream_info->original_dim.height = streams[i].orig_stream_sizes.height;
          stream_param->stream_info.original_dim = stream_info->original_dim;
          if (streams[i].is_changed) {
            stream_info->dim.width = streams[i].changed_stream_sizes.width;
            stream_info->dim.height = streams[i].changed_stream_sizes.height;
          }
          if((stream_info->pp_config.rotation == ROTATE_90) ||
             (stream_info->pp_config.rotation == ROTATE_270)) {
              height    = stream_info->dim.width;
              width     = stream_info->dim.height;
          } else {
              width     = stream_info->dim.width;
              height    = stream_info->dim.height;
          }
          /*need to consider dual vfe can take more width*/
          if ((hw_stream_info->max_width <
            (int32_t)(width / session_param->num_isp)) ||
              (hw_stream_info->max_height < height)) {
            ISP_ERR("failed: error wrong stream mapping");
            ISP_ERR("HW stream: %d HW max: %dx%d stream resolution: %dx%d",
              hw_stream_id[streams[i].hw_stream], hw_stream_info->max_width,
              hw_stream_info->max_height, stream_info->dim.width,
              stream_info->dim.height);
            goto ERROR;
          }
          break;
        }
      }
    } else if (stream_param->stream_info.stream_type == CAM_STREAM_TYPE_PARM) {
      for (i = 0; i < session_param->stream_port_map.num_streams; i++) {
        hw_stream_id[streams[i].hw_stream] = TRUE;
        ISP_DBG("i %d hw_stream %d", streams[i].hw_stream,
          hw_stream_id[streams[i].hw_stream]);
      }
    }

    for (i = 0; i < ISP_HW_STREAM_MAX; i++) {
      ISP_DBG("for i %d hw_stream %d", i, hw_stream_id[i]);
      if (hw_stream_id[i] == FALSE) {
        continue;
      }
      ISP_DBG("for loop hw stream %d", hw_stream_id[i]);
      /* Reserve resource for this stream */
      identity = (uint32_t *)CAM_MALLOC(sizeof(*identity));
      if (!identity) {
        ISP_ERR("failed: identity %p", identity);
        goto ERROR;
      }
      memset(identity, 0, sizeof(*identity));
      *identity = stream_info->identity;
      l_identity = mct_list_append(res_alloc->l_identity[i],
        (void *)identity, NULL, NULL);
      if (!l_identity) {
        ISP_ERR("failed: l_identity %p", l_identity);
        free(identity);
        goto ERROR;
      }
      res_alloc->l_identity[i] = l_identity;
      res_alloc->num_streams++;
    }
    reserved = TRUE;
  }

  for (i = 0; i < ISP_HW_STREAM_MAX; i++) {
       ISP_DBG("for i %d hw_stream %d", i, hw_stream_id[i]);
       if (hw_stream_id[i] == TRUE) {
           /* Update resource info in stream params */
           ISP_DBG("num_hw_stream %d hw stream %d", stream_param->num_hw_stream, i);
           stream_param->hw_stream[stream_param->num_hw_stream++] = i;
       }
  }

  if (reserved == FALSE) {
    /* Resource is assigned to other session */
    /* Check whether there are more than one ISP and any ISP is reserved but
       not streaming */
    if (isp_resource->num_isp > 1) {
      for (res_isp = 0; res_isp < isp_resource->num_isp &&
             res_isp < ISP_HW_MAX; res_isp++) {
        hw_id = session_param->hw_id[res_isp];
        res_info = &isp_resource->isp_resource_info[hw_id];
        res_alloc = &res_info->resource_alloc;
        if ((res_alloc->state == ISP_RESOURCE_RESERVED) &&
            (res_alloc->session_id != session_id)) {
          /* Break the chain and assign resource to current session */
        }
      }
    }
  }

  /* Print acquired resource */
  for (i = 0; i < stream_param->num_hw_stream; i++) {
    ISP_DBG("ide %x type %d hw stream %d",
      stream_param->stream_info.identity,
      stream_param->stream_info.stream_type,
      stream_param->hw_stream[i]);
  }

  PTHREAD_MUTEX_UNLOCK(&isp_resource->mutex);
  return TRUE;

ERROR:
  ISP_ERR("failed");
  PTHREAD_MUTEX_UNLOCK(&isp_resource->mutex);
  return FALSE;
}

/** isp_resource_caps_reserve:
 *
 *  @data1: submodule port for which caps reserve shall be
 *        called
 *  @data2: submodule reserve struct containing stream info
 *        and isp port
 *
 *  Validate whether current submod port's peer is different
 *  from isp port. call caps reserve on submod port
 *
 *  Return TRUE on successful caps reserve and FALSE on
 *  failure
 **/
static boolean isp_resource_caps_reserve(void *data1, void *data2)
{
  boolean               ret = TRUE;
  mct_port_t           *submod_port = (mct_port_t *)data1;
  isp_submod_reserve_t *submod_reserve = (isp_submod_reserve_t *)data2;

  if (!data1 || !data2) {
    ISP_ERR("failed: %p %p", data1, data2);
    return FALSE;
  }

  if (submod_port->peer && (submod_port->peer != submod_reserve->isp_port)) {
    return FALSE;
  }

  return submod_port->check_caps_reserve(submod_port, NULL,
    submod_reserve->stream_info);
}

/** isp_resource_reserve_port:
 *
 *  @module: module handle
 *  @direction: port direction
 *  @stream_info: stream info
 *  @isp_port: isp port handle
 *
 *  Reserve source / sink port of module
 *
 *  Return submodule's reserved port on success and NULL on
 *  failure
 **/
static mct_port_t *isp_resource_reserve_port(mct_module_t *module,
  mct_port_direction_t direction, mct_stream_info_t *stream_info,
  mct_port_t *isp_port)
{
  mct_list_t           *submod_ports_l = NULL;
  mct_list_t           *submod_port_l = NULL;
  isp_submod_reserve_t  submod_reserve;
  mct_port_t           *submod_port = NULL;

  if (!module || !stream_info || !isp_port) {
    ISP_ERR("failed: %p %p %p", module, stream_info, isp_port);
    return NULL;
  }

  if (direction == MCT_PORT_SRC) {
    submod_ports_l = MCT_MODULE_SRCPORTS(module);
  } else if (direction == MCT_PORT_SINK) {
    submod_ports_l = MCT_MODULE_SINKPORTS(module);
  } else {
    ISP_ERR("failed: invalid direction %d", direction);
    return NULL;
  }

  submod_reserve.stream_info = stream_info;
  submod_reserve.isp_port = isp_port;
  submod_port_l = mct_list_find_custom(submod_ports_l, &submod_reserve,
    isp_resource_caps_reserve);

  if (!submod_port_l) {
    if (module->request_new_port) {
      submod_port = module->request_new_port(stream_info, direction, module,
        NULL);
      if (!submod_port) {
        ISP_ERR("error creating submod sink port");
      }
    }
  } else {
    submod_port = (mct_port_t *)submod_port_l->data;
  }

  return submod_port;
}

/** isp_resource_link_modules:
 *
 *  @stream: new stream
 *  @port: ISP sink port
 *  @isp_resource_info: ISP resource info handle
 *  @session_param: session param handle
 *  @stream_param: stream param handle
 *  @hw_id: ISP hw id
 *
 *  Caps reserve and ext link sumoodules and add to stream
 *
 *  Return TRUE on success and FALSE on failure
 **/
static boolean isp_resource_link_modules(mct_stream_t *stream, mct_port_t *port,
  isp_resource_info_t *isp_resource_info, isp_session_param_t *session_param,
  isp_stream_param_t *stream_param, isp_hw_id_t hw_id)
{
  boolean            ret = TRUE;
  uint32_t           submod_id = 0;
  mct_module_t      *mod1 = NULL, *mod2 = NULL, *store_common_mod = NULL;
  mct_port_t        *submod_port = NULL;
  isp_port_data_t   *port_data = NULL;
  isp_submod_info_t *submod_info = NULL;
  uint32_t           hw_size = 0;
  isp_hw_streamid_t  hw_stream = ISP_HW_STREAM_MAX;
  uint32_t           i = 0;
  mct_module_t      *port_module = NULL;

  if (!stream || !port || !isp_resource_info || !session_param ||
      !stream_param) {
    ISP_ERR("failed: %p %p %p %p %p", stream, port, isp_resource_info,
            session_param, stream_param);
    return FALSE;
  }

  if (stream_param->num_hw_stream > ISP_HW_STREAM_MAX) {
    ISP_ERR("failed: stream_param->num_hw_stream %d",
      stream_param->num_hw_stream);
    return FALSE;
  }

  port_data = MCT_OBJECT_PRIVATE(port);
  if (!port_data) {
    ISP_ERR("failed: port_data %p", port_data);
    return FALSE;
  }

  ISP_HIGH("type %d", stream_param->stream_info.stream_type);

  submod_info = isp_resource_info->common_hw_info;
  hw_size = isp_resource_info->common_hw_size;

  for (submod_id = 0; submod_id < hw_size; submod_id++) {
    if (submod_info[submod_id].isp_submod) {
      mod1 = submod_info[submod_id].isp_submod;
      break;
    }
  }

  if (!mod1) {
    ISP_ERR("no modules found");
    return FALSE;
  }
  mod1->set_mod(mod1, MCT_MODULE_FLAG_INDEXABLE, stream->streaminfo.identity);
  submod_port = isp_resource_reserve_port(mod1, MCT_PORT_SINK,
    &stream->streaminfo, port);
  if (!submod_port || !submod_port->ext_link) {
    ISP_ERR("failed: submod_port %p", submod_port);
    return FALSE;
  }

  submod_port->ext_link(stream->streaminfo.identity, submod_port, port);

  port_module = (mct_module_t *)(MCT_PORT_PARENT(submod_port)->data);
  /* Update port private data */
  session_param->isp_ports.isp_submod_ports[hw_id] = submod_port;
  ISP_DBG("vfe %d isp_submod_ports %p port parent %s", hw_id,
    session_param->isp_ports.isp_submod_ports[hw_id],
    MCT_MODULE_NAME(port_module));

  ret = mct_port_add_child(stream->streaminfo.identity, submod_port);
  if (ret == FALSE) {
    ISP_ERR("failed: mct_port_add_child port %s identity %d",
      MCT_PORT_NAME(submod_port), stream->streaminfo.identity);
    goto ERROR;
  }
  ret = mct_object_set_parent(MCT_OBJECT_CAST(mod1), MCT_OBJECT_CAST(stream));
  if (ret == FALSE) {
    ISP_ERR("failed: mct_object_set_parent mod1 %s identity %d",
      MCT_PORT_NAME(mod1), stream->streaminfo.identity);
    goto ERROR;
  }

  for (submod_id++; submod_id < hw_size; submod_id++) {
    if (!submod_info[submod_id].isp_submod) {
      continue;
    }
    mod2 = submod_info[submod_id].isp_submod;
    mod2->set_mod(mod2, MCT_MODULE_FLAG_INDEXABLE, stream->streaminfo.identity);

    ISP_DBG("link %s %s", MCT_MODULE_NAME(mod1), MCT_MODULE_NAME(mod2));
    ret = mct_stream_link_modules(stream, mod1, mod2, NULL);
    if (ret == FALSE) {
      ISP_ERR("failed: mct_stream_link_modules %s %s", MCT_MODULE_NAME(mod1),
        MCT_MODULE_NAME(mod2));
      goto ERROR;
    }
    mod1 = mod2;
    store_common_mod = mod1;
  }

  if (!store_common_mod) {
    ISP_ERR("failed: no modules found in common hw stream");
    return FALSE;
  }

  for (i = 0; i < stream_param->num_hw_stream; i++) {
    hw_stream = stream_param->hw_stream[i];
    mod1 = store_common_mod;

    submod_info = isp_resource_info->stream_hw_info[hw_stream];
    hw_size = isp_resource_info->stream_hw_size[hw_stream];

    for (submod_id = 0; submod_id < hw_size; submod_id++) {
      if (!submod_info[submod_id].isp_submod) {
        continue;
      }
      mod2 = submod_info[submod_id].isp_submod;
      mod2->set_mod(mod2, MCT_MODULE_FLAG_INDEXABLE,
        stream->streaminfo.identity);

      ISP_DBG("link %s %s", MCT_MODULE_NAME(mod1), MCT_MODULE_NAME(mod2));
      ret = mct_stream_link_modules(stream, mod1, mod2, NULL);
      if (ret == FALSE) {
        ISP_ERR("failed: mct_stream_link_modules %s %s", MCT_MODULE_NAME(mod1),
          MCT_MODULE_NAME(mod2));
        goto ERROR;
      }
      mod1 = mod2;
    }
  }

  return ret;

ERROR:
  /* TODO handle error here */
  return ret;
}

/** isp_resource_create_link:
 *
 *  @module: module handle
 *  @port: port handle
 *  @isp_resource: ISP resource handle
 *  @stream_info: mct stream info
 *
 *  1) Allocate resource
 *
 *  2) Caps reserve on sub module ports
 *
 *  3) Ext link sub module ports
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_resource_create_link_without_alloc(mct_module_t *module,
  mct_port_t *port, isp_resource_t *isp_resource,
  mct_stream_info_t *stream_info, isp_hw_id_t hw_id)
{
  boolean                      ret = TRUE;
  isp_session_param_t         *session_param = NULL;
  isp_stream_param_t          *stream_param = NULL;
  uint32_t                     session_id = 0;
  mct_stream_t                *stream = NULL;
  isp_resource_info_t         *isp_resource_info = NULL;

  if (!module || !port || !isp_resource || !stream_info) {
    ISP_ERR("failed: %p %p %p %p", module, port, isp_resource, stream_info);
    return FALSE;
  }

  session_id = ISP_GET_SESSION_ID(stream_info->identity);

  ret = isp_util_get_session_stream_params(module, stream_info->identity,
    &session_param, &stream_param);
  if (ret == FALSE || !session_param || !stream_param) {
    ISP_ERR("failed: isp_util_get_session_stream_params ret %d %p %p",
      ret, session_param, stream_param);
    return FALSE;
  }

  /* Lock ISP session params, unlock this on every return path */
  PTHREAD_MUTEX_LOCK(&session_param->mutex);

  isp_resource_info = &isp_resource->isp_resource_info[hw_id];

  /* Create new stream */
  stream = mct_stream_new(session_id);
  if (!stream) {
    ISP_ERR("failed: stream %p", stream);
    goto ERROR;
  }
  stream->streaminfo = *stream_info;

  ret = isp_resource_link_modules(stream, port, isp_resource_info,
    session_param, stream_param, hw_id);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_resource_link_modules");
    goto ERROR;
  }

  /* Store new stream in stream params */
  stream_param->int_stream[hw_id] = stream;

  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
  return ret;

ERROR:
  ISP_ERR("failed: isp_resource_create_link");
  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
  return ret;
}

/** isp_resource_create_link:
 *
 *  @module: module handle
 *  @port: port handle
 *  @isp_resource: ISP resource handle
 *  @stream_info: mct stream info
 *
 *  1) Allocate resource
 *
 *  2) Caps reserve on sub module ports
 *
 *  3) Ext link sub module ports
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_resource_create_link(mct_module_t *module, mct_port_t *port,
  isp_resource_t *isp_resource, mct_stream_info_t *stream_info)
{
  boolean                      ret = TRUE;
  isp_session_param_t         *session_param = NULL;
  isp_stream_param_t          *stream_param = NULL;
  uint32_t                     session_id = 0;
  mct_stream_t                *stream = NULL;
  isp_hw_id_t                  isp_hw;
  isp_resource_info_t         *isp_resource_info = NULL;
  isp_resource_alloc_t        *resource_alloc = NULL;
  isp_hw_id_t                  hw_id;
  mct_stream_info_t            l_stream_info;
  enum msm_vfe_axi_stream_src axi_src = VFE_AXI_SRC_MAX;

  if (!module || !port || !isp_resource || !stream_info) {
    ISP_ERR("failed: %p %p %p %p", module, port, isp_resource, stream_info);
    return FALSE;
  }

  session_id = ISP_GET_SESSION_ID(stream_info->identity);

  ret = isp_util_get_session_stream_params(module, stream_info->identity,
    &session_param, &stream_param);
  if (ret == FALSE || !session_param || !stream_param) {
    ISP_ERR("failed: isp_util_get_session_stream_params ret %d %p %p",
      ret, session_param, stream_param);
    return FALSE;
  }

  if (stream_param->is_pipeline_supported == FALSE) {
    ISP_WARN(" stream identity %x not supported by ISP pipeline!",
      stream_info->identity);
    return TRUE;
  }

  /* Lock ISP session params, unlock this on every return path */
  PTHREAD_MUTEX_LOCK(&session_param->mutex);

  l_stream_info = *stream_info;
  ret = isp_stream_resource_allocate(session_param, stream_param,
     isp_resource, &l_stream_info);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_stream_resource_allocate");
    goto ERROR;
  }

  for (isp_hw = ISP_HW_0; isp_hw < session_param->num_isp; isp_hw++) {
    hw_id = session_param->hw_id[isp_hw];
    isp_resource_info = &isp_resource->isp_resource_info[hw_id];
    resource_alloc = &isp_resource_info->resource_alloc;
    if (resource_alloc->session_id == session_id) {

      ISP_DBG("type %d vfe %d", stream_param->stream_info.stream_type,
        hw_id);

      /* Create new stream */
      stream = mct_stream_new(session_id);
      if (!stream) {
        ISP_ERR("failed: stream %p", stream);
        goto ERROR;
      }
      stream->streaminfo = l_stream_info;

      ret = isp_resource_link_modules(stream, port, isp_resource_info,
        session_param, stream_param, hw_id);
      if (ret == FALSE) {
        ISP_ERR("failed: isp_resource_link_modules");
        goto ERROR;
      }

      /* Store new stream in stream params */
      stream_param->int_stream[hw_id] = stream;

    }
  }

  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
  return ret;

ERROR:
  ISP_ERR("failed: isp_resource_create_link");
  PTHREAD_MUTEX_UNLOCK(&session_param->mutex);
  return ret;
}

/** isp_resource_start_session:
 *
 *  @isp_resource: ISP resource handle
 *  @session_id: session id for which start session needs to be
 *             called
 *
 *  Call start session on all submodules and of all ISP for
 *  given session_id
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_resource_start_session(isp_resource_t *isp_resource,
  uint32_t session_id)
{
  boolean              ret = TRUE;
  isp_resource_info_t *res_info = NULL;
  isp_hw_id_t          res_isp = 0;
  int32_t              hw_stream_id = 0;
  isp_pipeline_t      *isp_pipeline = NULL;
  isp_submod_info_t   *submod_info = NULL;
  int32_t              hw_size = 0, submod_id = 0;
  mct_module_t        *mod = NULL;
  uint32_t             i = 0;

  if (!isp_resource) {
    ISP_ERR("failed: isp_resource %p", isp_resource);
    return FALSE;
  }

  /* Iterate through each ISP resource and call start session */
  for (res_isp = 0; res_isp < isp_resource->num_isp &&
         res_isp < ISP_HW_MAX; res_isp++) {
    res_info = &isp_resource->isp_resource_info[res_isp];
    isp_pipeline = res_info->isp_pipeline;

    submod_info = res_info->common_hw_info;
    hw_size = (int32_t)res_info->common_hw_size;

    for (submod_id = 0; submod_id < hw_size; submod_id++) {
      if (!submod_info[submod_id].isp_submod) {
        continue;
      }
      mod = submod_info[submod_id].isp_submod;
      ISP_DBG("call start_session on mod %s %p", MCT_MODULE_NAME(mod), mod);
      /* Call start_session */
      if (mod->start_session) {
        ret = mod->start_session(mod, session_id);
        if (ret == FALSE) {
          ISP_ERR("failed: start_session %s", MCT_MODULE_NAME(mod));
          goto ERROR_COMMON;
        }
      }
    }

    for (hw_stream_id = 0; hw_stream_id < (int32_t)isp_pipeline->num_hw_streams;
         hw_stream_id++) {

      submod_info = res_info->stream_hw_info[hw_stream_id];
      hw_size = (int32_t)res_info->stream_hw_size[hw_stream_id];

      for (submod_id = 0; submod_id < hw_size; submod_id++) {
        if (!submod_info[submod_id].isp_submod) {
          continue;
        }
        mod = submod_info[submod_id].isp_submod;
        ISP_DBG("call start_session on mod %s %p", MCT_MODULE_NAME(mod), mod);
        /* Call start_session */
        if (mod->start_session) {
          ret = mod->start_session(mod, session_id);
          if (ret == FALSE) {
            ISP_ERR("failed: start_session %s", MCT_MODULE_NAME(mod));
            goto ERROR_HW_STREAM;
          }
        }
      }
    }
  }

  return TRUE;

ERROR_HW_STREAM:
  for (; hw_stream_id >= 0; --hw_stream_id) {

    for (--submod_id; submod_id >= 0; --submod_id) {
      if (!submod_info[submod_id].isp_submod) {
        continue;
      }
      mod = submod_info[submod_id].isp_submod;
      ISP_ERR("call stop_session on mod %s", MCT_MODULE_NAME(mod));
      /* Call start_session */
      if (mod->stop_session) {
        ret = mod->stop_session(mod, session_id);
        if (ret == FALSE) {
          ISP_ERR("failed: stop_session %s", MCT_MODULE_NAME(mod));
        }
      }
    }
  }
ERROR_COMMON:
  for (--submod_id; submod_id >= 0; submod_id++) {
    if (!submod_info[submod_id].isp_submod) {
      continue;
    }
    mod = submod_info[submod_id].isp_submod;
    ISP_DBG("call stop_session on mod %s", MCT_MODULE_NAME(mod));
    /* Call stop_session */
    if (mod->stop_session) {
      ret = mod->stop_session(mod, session_id);
      if (ret == FALSE) {
        ISP_ERR("failed: stop_session %s", MCT_MODULE_NAME(mod));
      }
    }
  }
  for (res_isp; res_isp > 0; --res_isp) {
    res_info = &isp_resource->isp_resource_info[res_isp - 1];
    isp_pipeline = res_info->isp_pipeline;

    submod_info = res_info->common_hw_info;
    hw_size = res_info->common_hw_size;

    for (submod_id = 0; submod_id < hw_size; submod_id++) {
      if (!submod_info[submod_id].isp_submod) {
        continue;
      }
      mod = submod_info[submod_id].isp_submod;
      ISP_DBG("call stop_session on mod %s", MCT_MODULE_NAME(mod));
      /* Call stop_session */
      if (mod->stop_session) {
        ret = mod->stop_session(mod, session_id);
        if (ret == FALSE) {
          ISP_ERR("failed: stop_session %s", MCT_MODULE_NAME(mod));
        }
      }
    }

    for (hw_stream_id = 0; hw_stream_id < (int32_t)isp_pipeline->num_hw_streams;
         hw_stream_id++) {

      submod_info = res_info->stream_hw_info[hw_stream_id];
      hw_size = res_info->stream_hw_size[hw_stream_id];

      for (submod_id = 0; submod_id < hw_size; submod_id++) {
        if (!submod_info[submod_id].isp_submod) {
          continue;
        }
        mod = submod_info[submod_id].isp_submod;
        ISP_DBG("call stop_session on mod %s", MCT_MODULE_NAME(mod));
        /* Call start_session */
        if (mod->stop_session) {
          ret = mod->stop_session(mod, session_id);
          if (ret == FALSE) {
            ISP_ERR("failed: stop_session %s", MCT_MODULE_NAME(mod));
          }
        }
      }
    }
  }
  return FALSE;
}

/** isp_resource_stop_session:
 *
 *  @isp_resource: ISP resource handle
 *  @session_id: session id for which start session needs to be
 *             called
 *
 *  Call start session on all submodules and of all ISP for
 *  given session_id
 *
 *  Return TRUE on success and FALSE on failure
 **/
boolean isp_resource_stop_session(isp_resource_t *isp_resource,
  uint32_t session_id)
{
  boolean              ret = TRUE, func_ret = TRUE;
  isp_resource_info_t *res_info = NULL;
  isp_hw_id_t          res_isp = 0;
  isp_hw_streamid_t    hw_stream_id = 0;
  isp_pipeline_t      *isp_pipeline = NULL;
  isp_submod_info_t   *submod_info = NULL;
  int32_t              hw_size = 0, submod_id = 0;
  mct_module_t        *mod = NULL;

  if (!isp_resource) {
    ISP_ERR("failed: isp_resource %p", isp_resource);
    return FALSE;
  }

  /* Iterate through each ISP resource and call start session */
  for (res_isp = 0; res_isp < isp_resource->num_isp &&
         res_isp < ISP_HW_MAX; res_isp++) {
    res_info = &isp_resource->isp_resource_info[res_isp];
    isp_pipeline = res_info->isp_pipeline;

    submod_info = res_info->common_hw_info;
    hw_size = (int32_t)res_info->common_hw_size;

    for (submod_id = 0; submod_id < hw_size; submod_id++) {
      if (!submod_info[submod_id].isp_submod) {
        continue;
      }
      mod = submod_info[submod_id].isp_submod;
      ISP_DBG("call stop_session on mod %s", MCT_MODULE_NAME(mod));
      /* Call stop_session */
      if (mod->stop_session) {
        ret = mod->stop_session(mod, session_id);
        if (ret == FALSE) {
          ISP_ERR("failed: stop_session %s", MCT_MODULE_NAME(mod));
          func_ret = FALSE;
        }
      }
    }

    for (hw_stream_id = 0; hw_stream_id < isp_pipeline->num_hw_streams;
         hw_stream_id++) {

      submod_info = res_info->stream_hw_info[hw_stream_id];
      hw_size = (int32_t)res_info->stream_hw_size[hw_stream_id];

      for (submod_id = 0; submod_id < hw_size; submod_id++) {
        if (!submod_info[submod_id].isp_submod) {
          continue;
        }
        mod = submod_info[submod_id].isp_submod;
        ISP_DBG("call stop_session on mod %s", MCT_MODULE_NAME(mod));
        /* Call stop_session */
        if (mod->stop_session) {
          ret = mod->stop_session(mod, session_id);
          if (ret == FALSE) {
            ISP_ERR("failed: stop_session %s", MCT_MODULE_NAME(mod));
            func_ret = FALSE;
          }
        }
      }
    }
  }

  return func_ret;
}

/** isp_resource_init_sub_mod_table:
 *
 *  @hw_params: ISP hw params
 *  @submod_info: submod info
 *  @size: size
 *  @isp_version: ISP version
 *
 *  Open submod library and initialize submods
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean isp_resource_init_sub_mod_table(
    isp_hw_params_t   *hw_params,
    isp_pipeline_t    *isp_pipeline,
    isp_submod_info_t *submod_info,
    uint32_t          size,
    uint32_t          isp_version __unused)
{
  uint32_t                  num_submod = 0;
  isp_hw_info_t            *isp_hw_info = NULL;
  char                      lib_name[ISP_SUBMOD_LIB_NAME];
  char                      submod_name[ISP_SUBMOD_NAME];
  isp_submod_init_table_t  *(*open_lib)(void) = NULL;

  if (!hw_params || !submod_info || !size) {
    ISP_ERR("failed: %p %p %d", hw_params, submod_info, size);
    return FALSE;
  }

  for (num_submod = 0; num_submod < size; num_submod++) {
    isp_hw_info = &hw_params->hw_info_table[num_submod];
    if (!strlen(isp_hw_info->hw_name)) {
      continue;
    }

    snprintf(lib_name, sizeof(lib_name), "%s", isp_hw_info->hw_name);
    snprintf(submod_name, sizeof(submod_name), "libmmcamera_isp_%s.so",
      isp_hw_info->hw_name);


    /* Open library */
    submod_info[num_submod].lib_handle = dlopen(submod_name, RTLD_NOW);
    if (!submod_info[num_submod].lib_handle) {
      ISP_ERR("failed: opening %s", submod_name);
      continue;
    }

    /* dlsym open_lib() */
    open_lib = dlsym(submod_info[num_submod].lib_handle, "module_open");
    if (!open_lib) {
      ISP_ERR("failed: to find open_lib() for %s", lib_name);
      dlclose(submod_info[num_submod].lib_handle);
      submod_info[num_submod].lib_handle = NULL;
      continue;
    }

    /* call function to get init / deinit func pointers */
    submod_info[num_submod].isp_submode_init_table = open_lib();
    if (!submod_info[num_submod].isp_submode_init_table) {
      ISP_ERR("failed: open_lib() returned NULL");
      dlclose(submod_info[num_submod].lib_handle);
      submod_info[num_submod].lib_handle = NULL;
      continue;
    }

    /* call init function */
    submod_info[num_submod].isp_submod =
      submod_info[num_submod].isp_submode_init_table->module_init(lib_name);
    if (!submod_info[num_submod].isp_submod) {
      ISP_ERR("failed: module_init for %s returned NULL", lib_name);
      dlclose(submod_info[num_submod].lib_handle);
      submod_info[num_submod].lib_handle = NULL;
    }
    isp_pipeline->submod_enable[isp_hw_info->hw_module_id] = TRUE;
    ISP_DBG("lib_name %s mod %p hw_id %u enable %d", lib_name,
      submod_info[num_submod].isp_submod, isp_hw_info->hw_module_id,
      isp_pipeline->submod_enable[isp_hw_info->hw_module_id]);
  }

  return TRUE;
}

/** isp_resource_init_sub_mods:
 *
 *  @isp_submod_info: ISP sumod info
 *  @isp_resource: ISP pipeline handle
 *  @isp_version: ISP harware version
 *
 *  Open submod library and initialize submods
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean isp_resource_init_sub_mods(
  isp_resource_info_t *isp_resource_info, uint32_t isp_version)
{
  boolean                        ret = TRUE;
  uint32_t                       isp_main_version;
  isp_pipeline_t                *isp_pipeline = NULL;
  isp_hw_params_t               *hw_params = NULL;
  isp_hw_streamid_t              hw_stream, num_hw_stream;

  if (!isp_resource_info) {
    ISP_ERR("failed: %p", isp_resource_info);
    return FALSE;
  }

  isp_pipeline = isp_resource_info->isp_pipeline;
  if (!isp_pipeline) {
    ISP_ERR("failed");
    return FALSE;
  }

  hw_params = isp_pipeline->common_hw_params;
  if (!hw_params) {
    ISP_ERR("failed");
    return FALSE;
  }

  isp_resource_info->common_hw_size = hw_params->num_hw;
  isp_resource_info->common_hw_info = CAM_MALLOC(isp_resource_info->common_hw_size *
    sizeof(*isp_resource_info->common_hw_info));
  if (!isp_resource_info->common_hw_info) {
    ISP_ERR("failed");
    return FALSE;
  }
  memset(isp_resource_info->common_hw_info, 0,
    isp_resource_info->common_hw_size * sizeof(*isp_resource_info->common_hw_info));

  ret = isp_resource_init_sub_mod_table(hw_params, isp_pipeline,
    isp_resource_info->common_hw_info, isp_resource_info->common_hw_size,
    isp_version);
  if (ret == FALSE) {
    ISP_ERR("failed");
    return FALSE;
  }

  num_hw_stream = isp_resource_info->isp_pipeline->num_hw_streams;

  for (hw_stream = 0; hw_stream < num_hw_stream; hw_stream++) {
    hw_params = isp_pipeline->stream_hw_params[hw_stream];
    if (!hw_params) {
      ISP_ERR("failed, hw_params NULL for hw_stream %d", hw_stream);
      continue;
    }
    ISP_HIGH("num hw streams %d num hw %d", num_hw_stream,
      hw_params->num_hw);
    isp_resource_info->stream_hw_size[hw_stream] = hw_params->num_hw;
    isp_resource_info->stream_hw_info[hw_stream] =
      CAM_MALLOC(isp_resource_info->stream_hw_size[hw_stream] *
      sizeof(*isp_resource_info->stream_hw_info[hw_stream]));
    if (!isp_resource_info->stream_hw_info[hw_stream]) {
      ISP_ERR("failed");
      return FALSE;
    }
    memset(isp_resource_info->stream_hw_info[hw_stream], 0,
      isp_resource_info->stream_hw_size[hw_stream] *
      sizeof(*isp_resource_info->stream_hw_info[hw_stream]));

    ret = isp_resource_init_sub_mod_table(hw_params, isp_pipeline,
      isp_resource_info->stream_hw_info[hw_stream],
      isp_resource_info->stream_hw_size[hw_stream], isp_version);
    if (ret == FALSE) {
      ISP_ERR("failed");
      return FALSE;
    }
  }

  return ret;
}

/** isp_resource_deinit_sub_mods:
 *
 *  @isp_submod_info: ISP submodule info where deinit and lib
 *                  handle is stored
 *  @size: size of submod list
 *
 *  1) Call deinit
 *
 *  2) Close submod lib handle
 *
 *  Returns: void
 **/
static void isp_resource_deinit_sub_mods(isp_submod_info_t *isp_submod_info,
  uint32_t size)
{
  uint32_t           num_submod = 0;
  isp_submod_info_t *submod_info = NULL;

  if (!isp_submod_info) {
    ISP_ERR("failed: %p", isp_submod_info);
    return;
  }

  for (num_submod = 0; num_submod < size; num_submod++) {
    submod_info = &isp_submod_info[num_submod];

    /* Call deinit */
    if (submod_info->isp_submod && submod_info->isp_submode_init_table &&
        submod_info->isp_submode_init_table->module_deinit) {
      submod_info->isp_submode_init_table->module_deinit(
        submod_info->isp_submod);
    }

    /* Close dynamically loaded library */
    if (submod_info->lib_handle) {
      dlclose(submod_info->lib_handle);
      submod_info->lib_handle = NULL;
    }
  }
}

/** isp_resource_discover_subdev_nodes
 *
 *  @isp_subdev_names: handle to store subdev names
 *  @num_of_isp: handle to return total number of ISP hardware
 *
 *  Discover ISP subdevs
 *
 *  Returns: TRUE on success and FALSE on failure
 **/
static boolean isp_resource_discover_subdev_nodes(
  isp_resource_subdev_names_t *isp_subdev_names, uint32_t *num_of_isp)
{
  struct media_device_info  mdev_info;
  int                       num_media_devices = 0;
  char                      dev_name[32];
  int                       rc = 0, dev_fd = 0;
  int                       num_isps = 0;
  int                       num_entities;

  if (!isp_subdev_names || !num_of_isp) {
    ISP_ERR("failed: %p %p", isp_subdev_names, num_of_isp);
    return FALSE;
  }

  while (1) {
    snprintf(dev_name, sizeof(dev_name), "/dev/media%d", num_media_devices);
    ISP_DBG("dev_name %s", dev_name);
    dev_fd = open(dev_name, O_RDWR | O_NONBLOCK);
    ISP_DBG("dev_fd %d", dev_fd);
    if (dev_fd < 0) {
      ISP_DBG("Done discovering media devices\n");
      break;
    }

    num_media_devices++;
    rc = ioctl(dev_fd, MEDIA_IOC_DEVICE_INFO, &mdev_info);
    if (rc < 0) {
      ISP_ERR("Error: ioctl media_dev failed: %s\n", strerror(errno));
      close(dev_fd);
      break;
    }

    if (strncmp(mdev_info.model, "msm_config", sizeof(mdev_info.model)) != 0) {
      close(dev_fd);
      continue;
    }

    num_entities = 0;

    while (1) {
      struct media_entity_desc entity;
      memset(&entity, 0, sizeof(entity));
      entity.id = num_entities | MEDIA_ENT_ID_FLAG_NEXT;
      rc = ioctl(dev_fd, MEDIA_IOC_ENUM_ENTITIES, &entity);
      if (rc < 0) {
        ISP_DBG("Done enumerating media entities\n");
        break;
      }
      num_entities = entity.id;
      rc = mct_util_match_subdev_type(entity, MSM_CAMERA_SUBDEV_VFE,
        MEDIA_ENT_T_V4L2_SUBDEV);
      if (rc && *num_of_isp < ISP_HW_MAX) {
        snprintf(isp_subdev_names[*num_of_isp].subdev_name,
          sizeof(isp_subdev_names[*num_of_isp].subdev_name), "/dev/%s",
          entity.name);
        ISP_DBG("subdev name %s", isp_subdev_names[*num_of_isp].subdev_name);
        (*num_of_isp)++;
      }
    }
    close(dev_fd);
  }
  return TRUE;
}

/** isp_resource_query_caps:
 *
 *  @isp_resource: handle to isp pipeline
 *
 *  1) open subdev
 *
 *  2) read ISP version
 *
 *  3) initilize caps
 *
 *  4) initialize ISP pipeline
 *
 *  returns TRUE on success and FALSE on failure
 **/
static boolean isp_resource_query_caps(isp_resource_info_t *isp_resource_info,
  uint32_t isp_id)
{
  boolean                    ret = TRUE;
  int                        rc = 0;
  int                        fd = 0;
  uint32_t                   ver = 0;
  struct msm_isp_clk_rates   clk_rate;
  struct msm_vfe_cfg_cmd2    cmd2;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd;
  char                           value[PROPERTY_VALUE_MAX];
  uint32_t                       turbo_clk_mode;

  if (!isp_resource_info) {
    ISP_ERR("failed: invalid params %p", isp_resource_info);
    return FALSE;
  }

  if (isp_id >= ISP_HW_MAX) {
    ISP_ERR("failed: isp_id %d", isp_id);
    return FALSE;
  }

  if (!strlen(isp_resource_info->subdev_name)) {
    ISP_ERR("failed: subdev name len = 0!!");
    return FALSE;
  }

  fd = open(isp_resource_info->subdev_name, O_RDWR | O_NONBLOCK);
  if (fd < 0) {
    ISP_ERR("cannot open '%s'", isp_resource_info->subdev_name);
    return FALSE;
  }

  /* Get ISP hw version */
  memset(&cmd2, 0, sizeof(cmd2));
  memset(&reg_cfg_cmd, 0, sizeof(reg_cfg_cmd));

  cmd2.cfg_cmd = (void *)&reg_cfg_cmd;
  cmd2.cfg_data = (void *)&ver;
  cmd2.cmd_len = sizeof(ver);
  cmd2.num_cfg = 1;

  reg_cfg_cmd.u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd.cmd_type = VFE_READ;
  reg_cfg_cmd.u.rw_info.len = sizeof(ver);
  reg_cfg_cmd.u.rw_info.reg_offset = 0;

  rc = ioctl(fd, VIDIOC_MSM_VFE_REG_CFG, &cmd2);
  if (rc < 0) {
    ISP_ERR("isp version query error = %d\n", rc);
    ret = FALSE;
    goto end;
  }

  if (isp_resource_info->isp_pipeline) {
    free(isp_resource_info->isp_pipeline);
    isp_resource_info->isp_pipeline = NULL;
  }

  /* Create .so */
  if (ver >= ISP_MSM8998) {
    isp_resource_info->isp_pipeline = isp_pipeline48_open(ver, isp_id);
    if (!(isp_resource_info->isp_pipeline)) {
      ISP_ERR("failed *isp_resource %p", isp_resource_info->isp_pipeline);
      ret = FALSE;
      goto end;
    }
  } else if (ver >= ISP_MSM8996_V1) {
    isp_resource_info->isp_pipeline = isp_pipeline47_open(ver, isp_id);
    if (!(isp_resource_info->isp_pipeline)) {
      ISP_ERR("failed *isp_resource %p", isp_resource_info->isp_pipeline);
      ret = FALSE;
      goto end;
    }
  } else if (ver >= ISP_MSM8994_V1) {
    isp_resource_info->isp_pipeline = isp_pipeline46_open(ver, isp_id);
    if (!(isp_resource_info->isp_pipeline)) {
      ISP_ERR("failed *isp_resource %p", isp_resource_info->isp_pipeline);
      ret = FALSE;
      goto end;
    }
  } else if (ver >= ISP_MSM8084_V1) {
    isp_resource_info->isp_pipeline = isp_pipeline44_open(ver, isp_id);
    if (!(isp_resource_info->isp_pipeline)) {
      ISP_ERR("failed *isp_resource %p", isp_resource_info->isp_pipeline);
      ret = FALSE;
      goto end;
    }
  } else if (ver >= ISP_MSM8937) {
    isp_resource_info->isp_pipeline = isp_pipeline42_open(ver, isp_id);
    if (!(isp_resource_info->isp_pipeline)) {
      ISP_ERR("failed *isp_resource %p", isp_resource_info->isp_pipeline);
      ret = FALSE;
      goto end;
    }
  } else if (ver >= ISP_MSM8956) {
    isp_resource_info->isp_pipeline = isp_pipeline41_open(ver, isp_id);
    if (!(isp_resource_info->isp_pipeline)) {
      ISP_ERR("failed *isp_resource %p", isp_resource_info->isp_pipeline);
      ret = FALSE;
      goto end;
    }
  } else if (ver >= ISP_MSM8974_V1) {
    isp_resource_info->isp_pipeline = isp_pipeline40_open(ver, isp_id);
    if (!(isp_resource_info->isp_pipeline)) {
      ISP_ERR("failed *isp_resource %p", isp_resource_info->isp_pipeline);
      ret = FALSE;
      goto end;
    }
  } else if (ver >= ISP_MSM8960V1) {
    isp_resource_info->isp_pipeline = isp_pipeline32_open(ver, isp_id);
    if (!(isp_resource_info->isp_pipeline)) {
      ISP_ERR("failed *isp_resource %p", isp_resource_info->isp_pipeline);
      ret = FALSE;
      goto end;
    }
  } else {
    ISP_ERR("failed: *isp_resource %p", isp_resource_info->isp_pipeline);
    ret = FALSE;
    goto end;
  }

  cmd2.cfg_cmd = (void *)&reg_cfg_cmd;
  cmd2.cfg_data = (void *)&clk_rate;
  cmd2.cmd_len = sizeof(clk_rate);
  cmd2.num_cfg = 1;

  reg_cfg_cmd.u.rw_info.cmd_data_offset = 0;
  reg_cfg_cmd.cmd_type = GET_CLK_RATES;
  reg_cfg_cmd.u.rw_info.len = sizeof(clk_rate);
  reg_cfg_cmd.u.rw_info.reg_offset = 0;

  rc = ioctl(fd, VIDIOC_MSM_VFE_REG_CFG, &cmd2);
  if (rc < 0) {
    ISP_ERR("isp max clock query error = %d\n", rc);
    ret = FALSE;
    goto end;
  }

  isp_resource_info->isp_pipeline->max_pix_clk_rate[ISP_CX_LEVEL_SVS] = clk_rate.svs_rate;
  isp_resource_info->isp_pipeline->max_pix_clk_rate[ISP_CX_LEVEL_TURBO] = clk_rate.high_rate;
  isp_resource_info->isp_pipeline->max_pix_clk_rate[ISP_CX_LEVEL_NOMINAL] = clk_rate.nominal_rate;


#ifdef _ANDROID_
  property_get("persist.vendor.camera.isp.turbo", value, "0");
#endif
  turbo_clk_mode = atoi(value);
  if (turbo_clk_mode == 1) {
    isp_resource_info->isp_pipeline->max_pix_clk_rate[ISP_CX_LEVEL_NOMINAL] = clk_rate.high_rate;
    isp_resource_info->isp_pipeline->max_pix_clk_rate[ISP_CX_LEVEL_SVS] = clk_rate.high_rate;
    ISP_HIGH("turbo clk %d enabled for User", isp_resource_info->isp_pipeline->max_pix_clk_rate[ISP_CX_LEVEL_NOMINAL]);
  }

  ISP_DBG("max turbo clk rate  -> %d ",isp_resource_info->isp_pipeline->max_pix_clk_rate[ISP_CX_LEVEL_TURBO]);
  ISP_DBG("max nominal clk rate -> %d ",isp_resource_info->isp_pipeline->max_pix_clk_rate[ISP_CX_LEVEL_NOMINAL]);
  ISP_DBG("max svs clk rate -> %d ",isp_resource_info->isp_pipeline->max_pix_clk_rate[ISP_CX_LEVEL_SVS]);
end:
  close(fd);
  return ret;
}

static boolean isp_resource_get_isp_id(
  isp_resource_subdev_names_t *isp_subdev_names, uint32_t *isp_id)
{
  boolean                    ret = TRUE;
  int                        rc = 0;
  int                        fd = 0;
  struct msm_vfe_cfg_cmd2    cmd2;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd;

  if (!isp_subdev_names || !isp_id) {
    ISP_ERR("failed: invalid params %p %p", isp_subdev_names, isp_id);
    return FALSE;
  }

  /* Initialize isp_id */
  *isp_id = ISP_HW_MAX;

  if (!strlen(isp_subdev_names->subdev_name)) {
    ISP_ERR("failed: invalid subdev name %s", isp_subdev_names->subdev_name);
    return FALSE;
  }

  fd = open(isp_subdev_names->subdev_name, O_RDWR | O_NONBLOCK);
  if (fd < 0) {
    ISP_ERR("cannot open '%s'", isp_subdev_names->subdev_name);
    return FALSE;
  }
  /* Get ISP ID */
  memset(&cmd2, 0, sizeof(cmd2));
  memset(&reg_cfg_cmd, 0, sizeof(reg_cfg_cmd));

  cmd2.cfg_cmd = (void *)&reg_cfg_cmd;
  cmd2.cfg_data = (void *)isp_id;
  cmd2.cmd_len = sizeof(*isp_id);
  cmd2.num_cfg = 1;

  reg_cfg_cmd.cmd_type = GET_ISP_ID;

  rc = ioctl(fd, VIDIOC_MSM_VFE_REG_CFG, &cmd2);
  if (rc < 0) {
    ISP_ERR("isp version query error = %d\n", rc);
    goto ERROR;
  }

  if (*isp_id >= ISP_HW_MAX) {
    ISP_ERR("failed: invalid hw id %d rc %d", *isp_id, rc);
  }
  ISP_DBG("isp_id %d", *isp_id);

  close(fd);
  return TRUE;

ERROR:
  close(fd);
  return FALSE;
}

static boolean isp_resource_get_isp_hw_limit(
  isp_resource_info_t *isp_resource_info, uint32_t isp_id,
  uint32_t *hw_limit)
{
  int                        rc = 0;
  int                        fd = 0;
  struct msm_vfe_cfg_cmd2    cmd2;
  struct msm_vfe_reg_cfg_cmd reg_cfg_cmd;

   if (!isp_resource_info || !hw_limit) {
    ISP_ERR("failed: invalid params %p", isp_resource_info, hw_limit);
    return FALSE;
  }

  if (isp_id >= ISP_HW_MAX) {
    ISP_ERR("failed: isp_id %d", isp_id);
    return FALSE;
  }

  if (!strlen(isp_resource_info->subdev_name)) {
    ISP_ERR("failed: subdev name len = 0!!");
    return FALSE;
  }

#ifdef VFE_HW_LIMIT
  fd = open(isp_resource_info->subdev_name, O_RDWR | O_NONBLOCK);
  if (fd < 0) {
    ISP_ERR("cannot open '%s'", isp_resource_info->subdev_name);
    return FALSE;
  }
  /* Get ISP HW limit */
  memset(&cmd2, 0, sizeof(cmd2));
  memset(&reg_cfg_cmd, 0, sizeof(reg_cfg_cmd));

  cmd2.cfg_cmd = (void *)&reg_cfg_cmd;
  cmd2.cfg_data = (void *)hw_limit;
  cmd2.cmd_len = sizeof(*hw_limit);
  cmd2.num_cfg = 1;

  reg_cfg_cmd.cmd_type = GET_VFE_HW_LIMIT;

  rc = ioctl(fd, VIDIOC_MSM_VFE_REG_CFG, &cmd2);
  if (rc < 0) {
    ISP_ERR("isp hw limit error = %d\n", rc);
    goto CLOSE_FD;
  }
  ISP_INFO("isp hw limit %d", *hw_limit);

  close(fd);
#endif
  return TRUE;

CLOSE_FD:
  close(fd);
ERROR:
  return FALSE;
}

/** isp_resource_gen_hws_caps
 *
 *  @isp_resource: ISP resource handle
 *
 *  1) Discover ISP subdevs
 *
 *  2) Query hw caps
 *
 *  Returns TRUE on success and FALSE on failure
 **/
static boolean isp_resource_gen_hws_caps(isp_resource_t *isp_resource)
{
  isp_hw_id_t                  sd_num = 0;
  boolean                      ret = TRUE;
  isp_resource_info_t         *isp_resource_info = NULL;
  isp_hw_id_t                  isp_id = 0;
  isp_resource_subdev_names_t  isp_subdev_names[ISP_HW_MAX];
  uint32_t                     num_of_isp = 0;
  uint32_t                     isp_hw_limit;

  if (!isp_resource) {
    ISP_ERR("failed: isp_resource %p", isp_resource);
    return FALSE;
  }

  memset(&isp_subdev_names[0], 0,
    ISP_HW_MAX * sizeof(isp_resource_subdev_names_t));
  ret = isp_resource_discover_subdev_nodes(isp_subdev_names, &num_of_isp);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_resource_discover_subdev_nodes");
    return FALSE;
  }

  if (!num_of_isp || (num_of_isp > ISP_HW_MAX)) {
    ISP_ERR("failed: num_of_isp %d", num_of_isp);
    return FALSE;
  }

  ISP_DBG("DUALVFE num isp %d", num_of_isp);
  for (sd_num = ISP_HW_0; sd_num < num_of_isp; sd_num++) {

    isp_id = ISP_HW_MAX;

    ret = isp_resource_get_isp_id(&isp_subdev_names[sd_num], &isp_id);
    if ((isp_id >= ISP_HW_MAX) || (ret == FALSE)) {
      ISP_ERR("failed: subdev name %s sd_num %d invalid isp_id %d, ret = %d",
        isp_subdev_names[sd_num].subdev_name, sd_num, isp_id, ret);
      return FALSE;
    }

    isp_resource_info = &isp_resource->isp_resource_info[isp_id];
    strlcpy(isp_resource_info->subdev_name,
      isp_subdev_names[sd_num].subdev_name,
      sizeof(isp_resource_info->subdev_name));

    /* Initialize mutex */
    pthread_mutex_init(&isp_resource->mutex, NULL);
    isp_hw_limit = 0;
    isp_resource->isp_hw_limt.vfe1_reserved_rdi = -1;
    isp_resource->isp_hw_limt.hw_limit = NO_LIMIT;
    ret = isp_resource_get_isp_hw_limit(isp_resource_info, isp_id,
      &isp_hw_limit);
    if (ret == TRUE) {
     switch (isp_hw_limit) {
       case MAX_13MP:
         isp_resource->isp_hw_limt.hw_limit = MAX_13MP;
         isp_resource->isp_hw_limt.max_resolution = 13500000;
       break;
       case MAX_16MP:
         isp_resource->isp_hw_limt.hw_limit = MAX_16MP;
         isp_resource->isp_hw_limt.max_resolution = 16384000;
       break;
       case MAX_21MP:
         isp_resource->isp_hw_limt.hw_limit = MAX_21MP;
         isp_resource->isp_hw_limt.max_resolution = 21889024;
       break;
       case NO_LIMIT:
       default:
          isp_resource->isp_hw_limt.hw_limit = NO_LIMIT;
          break;
       }
    }
    ret = isp_resource_query_caps(isp_resource_info, isp_id);
    if (ret == FALSE) {
      ISP_ERR("failed: isp_hw_query_caps");
      return ret;
    }
    /* Initialize sub mods */
    ret = isp_resource_init_sub_mods(isp_resource_info,
      GET_ISP_MAIN_VERSION(isp_resource_info->isp_pipeline->isp_version));
    if (ret == FALSE) {
      ISP_ERR("failed: isp_resource_init_sub_mods");
      return ret;
    }
    isp_resource->num_isp++;
  }
  sort_isp_hw_by_size(isp_resource->isp_resource_info, isp_resource->num_isp,
    isp_resource->sorted_hw_ids);

  for (sd_num = ISP_HW_0; sd_num < isp_resource->num_isp; sd_num++) {
    ISP_DBG("i %d isp id %d", sd_num,
      isp_resource->isp_resource_info[sd_num].isp_pipeline->isp_id);
  }
  return ret;
}

/** isp_resource_init:
 *
 *  @isp_resource: ISP resource handle
 *
 *  1) Initlialize ISP pipeline
 *
 *  2) Initialize sub mods
 *
 *  return TRUE on success and FALSE on failure
 **/
boolean isp_resource_init(isp_resource_t *isp_resource)
{
  boolean ret = TRUE;

  if (!isp_resource) {
    ISP_ERR("failed: isp_resource %p", isp_resource);
    return FALSE;
  }

  /* Generate hw caps */
  ret = isp_resource_gen_hws_caps(isp_resource);
  if (ret == FALSE) {
    ISP_ERR("failed: isp_resource_gen_hws_caps");
    return FALSE;
  }

  return TRUE;
}

/** isp_resource_destroy:
 *
 *  @isp_resource: isp resource handle
 *
 *  Destroy isp pipeline data
 *
 *  Returns: void
 **/
void isp_resource_destroy(isp_resource_t *isp_resource)
{
  isp_hw_id_t            sd_num = 0;
  isp_hw_streamid_t      hw_stream = 0;
  isp_resource_info_t   *isp_resource_info = NULL;
  isp_hw_streamid_t      num_hw_stream = 0;

  if (!isp_resource) {
    ISP_ERR("failed: isp_resource %p", isp_resource);
    return;
  }

  for (sd_num = 0; sd_num < isp_resource->num_isp; sd_num++) {
    isp_resource_info = &isp_resource->isp_resource_info[sd_num];

    isp_resource_deinit_sub_mods(isp_resource_info->common_hw_info,
      isp_resource_info->common_hw_size);

    num_hw_stream = isp_resource_info->isp_pipeline->num_hw_streams;

    for (hw_stream = 0; hw_stream < num_hw_stream; hw_stream++) {
      isp_resource_deinit_sub_mods(isp_resource_info->stream_hw_info[hw_stream],
        isp_resource_info->stream_hw_size[hw_stream]);
    }
    free(isp_resource_info->isp_pipeline);
  }
}
