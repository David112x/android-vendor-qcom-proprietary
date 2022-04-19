/**********************************************************************
*  Copyright (c) 2013-2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/
#include <linux/media.h>
#include "mct_module.h"
#include "module_cac.h"
#include "chromatix_swpostproc.h"

/* Note define to 1 for enabling this flag */
#if defined(CHROMATIX_308E) || defined(CHROMATIX_310E)
#define CUSTOM_ENABLE_LDS
#endif

#define RNR_ENABLED(p) (( NULL != chromatix) && \
  (((chromatix_RNR2_type *)& \
    (chromatix->chromatix_radial_noise2_reduction))->rnr_en))

/**
 * STATIC function declarations
 **/
static mct_port_t *module_cac_create_port(mct_module_t *p_mct_mod,
  mct_port_direction_t dir);

/**
 * Function: module_cac_get_stream_by_id
 *
 * Description: This method is used to find the client based
 *              on stream identity
 *
 * Arguments:
 *   @p_client: cac client
 *   @identity: input identity
 *
 * Return values:
 *     Index of the stream in the stream port mapping of the
 *     client
 *
 * Notes: none
 **/
int module_cac_get_stream_by_id(cac_client_t *p_client,
  uint32_t identity)
{
  int32_t i = 0;
  int s_idx = -1;
  if (p_client->stream_cnt > MAX_CAC_STREAMS) {
    IDBG_ERROR("%s %d invalid stream_cnt %d", __func__, __LINE__,
      p_client->stream_cnt);
    return s_idx;
  }
  for (i = 0; i < p_client->stream_cnt; i++) {
    IDBG_MED("%s %d stream_type %d identity %x stream_cnt %d i %d", __func__,
      __LINE__, p_client->stream[i].stream_info->stream_type,
      p_client->stream[i].identity, p_client->stream_cnt,
      i);
    if (identity == p_client->stream[i].identity) {
      s_idx = i;
      break;
    }
  }
  return s_idx;
}

/**
 * Function: module_cac_get_stream_by_type
 *
 * Description: This method is used to find the client based
 *              on stream type
 *
 * Arguments:
 *   @p_client: cac client
 *   @identity: input stream type
 *
 * Return values:
 *     Index of the stream in the stream port mapping of the
 *     client
 *
 * Notes: none
 **/
int module_cac_get_stream_idx_by_type(cac_client_t *p_client,
  uint32_t stream_type)
{
  int32_t i = 0;
  int s_idx = -1;
  if (p_client->stream_cnt > MAX_CAC_STREAMS) {
    IDBG_ERROR("%s %d invalid stream_cnt %d", __func__, __LINE__,
      p_client->stream_cnt);
    return s_idx;
  }
  for (i = 0; i < p_client->stream_cnt; i++) {
    IDBG_MED("%s %d stream_type %d identity %d stream_cnt %d i %d", __func__,
      __LINE__, p_client->stream[i].stream_info->stream_type,
      p_client->stream[i].identity, p_client->stream_cnt,
      i);
    if (stream_type == p_client->stream[i].stream_info->stream_type) {
      s_idx = i;
      break;
    }
  }
  return s_idx;
}

/**
 * Function: module_cac_get_stream_type_by_id
 *
 * Description: This method is used to find the client based
 *              on stream identity
 *
 * Arguments:
 *   @p_client: cac client
 *   @identity: input identity
 *
 * Return values:
 *     Stream type of the stream with the specified identity
 *     -1 is not found
 *
 * Notes: none
 **/
int module_cac_get_stream_type_by_id(cac_client_t *p_client,
  uint32_t identity)
{
  int32_t i = 0;
  int32_t stream_type = -1;
  if (p_client->stream_cnt > MAX_CAC_STREAMS) {
    IDBG_ERROR("%s %d invalid stream_cnt %d", __func__, __LINE__,
      p_client->stream_cnt);
    return stream_type;
  }
  for (i = 0; i < p_client->stream_cnt; i++) {
    IDBG_MED("%s %d stream_type %d identity %d stream_cnt %d i %d", __func__,
      __LINE__, p_client->stream[i].stream_info->stream_type,
      p_client->stream[i].identity, p_client->stream_cnt,
      i);
    if (identity == p_client->stream[i].identity) {
      stream_type = p_client->stream[i].stream_info->stream_type;
      break;
    }
  }
  return stream_type;
}

/**
 * Function: module_cac_find_client
 *
 * Description: This method is used to find the client
 *
 * Arguments:
 *   @p_fp_data: cac client
 *   @p_input: input data
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean module_cac_find_client(void *p_cac_data, void *p_input)
{
  cac_client_t *p_client = (cac_client_t *)p_cac_data;
  uint32_t identity = *((uint32_t *)p_input);
  boolean flag = FALSE;
  int i;

  for (i = 0; i < p_client->stream_cnt; i++) {
    if (p_client->stream[i].identity == identity) {
      flag = TRUE;
      break;
    }
  }
  return flag;
}

/**
 * Function: module_cac_find_adj_port
 *
 * Description: This method is used to find the adj port
 *
 * Arguments:
 *   @p_fp_data: cac client
 *   @p_input: port
 *
 * Return values:
 *     Adjacent port
 *
 * Notes: none
 **/
static void* module_cac_find_adj_port(void *p_cac_data, void *p_input)
{
  int i = 0;
  cac_client_t *p_client = (cac_client_t *)p_cac_data;
  mct_port_t *port = ((mct_port_t *)p_input);
  for (i = 0; i < p_client->stream_cnt; i++) {
    if (p_client->stream[i].p_sinkport == port) {
      return p_client->stream[i].p_srcport;
    } else if (p_client->stream[i].p_srcport == port) {
      return p_client->stream[i].p_sinkport;
    }
  }
  return NULL;
}

/**
 * Function: module_cac_find_identity
 *
 * Description: This method is used to find the client
 *
 * Arguments:
 *   @p_data: data in the mct list
 *   @p_input: input data to be seeked
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean __unused module_cac_find_identity(void *p_data, void *p_input)
{
  uint32_t *p_identity = (uint32_t *)p_data;
  uint32_t identity = *((uint32_t *)p_input);

  return (*p_identity == identity) ? TRUE : FALSE;
}

/**
 * Function: module_cac_find_session_params
 *
 * Description: This method is used to find the the session based parameters
 *
 * Arguments:
 *   @p_data: data in the mct list
 *   @p_input: input data to be seeked
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean module_cac_find_session_params(void *p_data, void *p_input)
{

  cac_session_params_t *stored_param = p_data;
  uint32_t session_id = *((uint32_t *)p_input);

  return (stored_param->sessionid == session_id) ? TRUE : FALSE;
}

/**
 * Function: module_cac_get_session_params
 *
 * Description: This method is used to get the session params by
 *              session id
 *
 * Arguments:
 *   @p_mod: cac module
 *   @session_id: Session id
 *
 * Return values:
 *     Return a pointer to the session params. Null if not
 *     found.
 *
 * Notes: none
 **/
void *module_cac_get_session_params(module_cac_t *p_mod, uint32_t session_id)
{
  mct_list_t *p_temp_list;
  cac_session_params_t *stored_param = NULL;

  if (!p_mod) {
    IDBG_ERROR("%s %d] Invalid input", __func__, __LINE__);
    return NULL;
  }

  /* Find settings per session id */
  p_temp_list = mct_list_find_custom(p_mod->session_params, &session_id,
    module_cac_find_session_params);
  if (!p_temp_list) {
    return NULL;
  }
  stored_param = p_temp_list->data;
  if (!stored_param) {
    return NULL;
  }
  return stored_param;
}

/**
 * Function: module_cac_set_mod
 *
 * Description: This function is used to set the cac module type
 *
 * Arguments:
 *   @module: mct module pointer
 *   @module_type: module type
 *   @identity: id of the stream/session
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
static void module_cac_set_mod(mct_module_t *module,
  unsigned int module_type,
  unsigned int identity)
{
  module_cac_t *p_mod;

  if (!(module && module->module_private)) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return;
  }

  /* Do not touch module mode since currently is only one instance module */
  p_mod = (module_cac_t *)module->module_private;
  p_mod->module_type = module_type;

  if (mct_module_find_type(module, identity) != MCT_MODULE_FLAG_INVALID) {
    mct_module_remove_type(module, identity);
  }
  mct_module_add_type(module, module_type, identity);

  return;
}

/**
 * Function: module_cac_fill_stream
 *
 * Description: This function is used to fill the stream details
 *
 * Arguments:
 *   @p_client: cac client
 *   @stream_info: pointer to stream info
 *   @port: port
 *
 * Return values:
 *     error values
 *
 * Notes: none
 **/
int module_cac_fill_stream(cac_client_t *p_client,
  mct_stream_info_t *stream_info, mct_port_t *port)
{
  stream_port_mapping_t *p_stream;
  int32_t status = IMG_ERR_NOT_FOUND;

  IDBG_MED("%s:%d] Stream %d, port %s",  __func__,  __LINE__,
    stream_info->stream_type, MCT_PORT_NAME(port));
  if (MCT_PORT_IS_SINK(port) &&
    p_client->stream_cnt >= MAX_CAC_STREAMS) {
    IDBG_ERROR("%s:%d] Error max ports reached, %d",
      __func__,__LINE__, p_client->stream_cnt);
    return IMG_ERR_OUT_OF_BOUNDS;
  }

  if (MCT_PORT_IS_SINK(port)) {
    IDBG_MED("%s:%d] Port %s is sink port",  __func__,
      __LINE__, MCT_PORT_NAME(port));
    p_stream = &p_client->stream[p_client->stream_cnt];
    p_stream->stream_info = stream_info;
    p_stream->identity = stream_info->identity;
    p_stream->p_sinkport = port;
    status = IMG_SUCCESS;
    p_stream->p_srcport = NULL;
    p_client->stream_cnt++;
  } else { /* src port */
    IDBG_MED("%s:%d] Port %s is src port",  __func__,
      __LINE__, MCT_PORT_NAME(port));
    int32_t i = 0;
    for (i = 0; i < p_client->stream_cnt; i++) {
      IDBG_MED("%s:%d] Src %x %x", __func__, __LINE__,
        p_client->stream[i].identity, stream_info->identity);
      if (p_client->stream[i].identity ==
        stream_info->identity) {
        p_client->stream[i].p_srcport = port;
        status = IMG_SUCCESS;
        break;
      }
    }
  }
  return status;
}

/**
 * Function: module_cac_query_mod
 *
 * Description: This function is used to query the cac module
 * info
 *
 * Arguments:
 *   @module: mct module pointer
 *   @query_buf: pipeline capability
 *   @sessionid: session identity
 *
 * Return values:
 *     success/failure
 *
 * Notes: none
 **/
boolean module_cac_query_mod(mct_module_t *module, void *buf,
  unsigned int sessionid)
{
  IMG_UNUSED(sessionid);

  mct_pipeline_cap_t *p_mct_cap = (mct_pipeline_cap_t *)buf;
  mct_pipeline_pp_cap_t *p_cap;
  IDBG_MED("%s:%d: E", __func__, __LINE__);
  if (!p_mct_cap || !module) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }
  p_cap = &p_mct_cap->pp_cap;
  p_cap->feature_mask |= (cam_feature_mask_t)CAM_QCOM_FEATURE_CAC;
  p_cap->aberration_modes_count = 2;
  p_cap->aberration_modes[0] = CAM_COLOR_CORRECTION_ABERRATION_OFF;
  p_cap->aberration_modes[1] = CAM_COLOR_CORRECTION_ABERRATION_HIGH_QUALITY;

 return TRUE;

}

/**
 * Function: module_cac_forward_port_event
 *
 * Description: This method is used to forward an event
 * depending on the direction.
 *
 * Arguments:
 *   @mct_port: Port that recieved the event
 *   @event: Event recieved
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static boolean module_cac_forward_port_event(cac_client_t *p_client,
  mct_port_t *port, mct_event_t *event)
{
  boolean rc = FALSE;
  mct_port_t *p_adj_port = NULL;
  if (p_client == NULL)
  {
    IDBG_ERROR("%s:%d] p_client is NULL",__func__,__LINE__);
    return FALSE;
  }

  if (MCT_PORT_IS_SINK(port)) {
    p_adj_port = (mct_port_t*)module_cac_find_adj_port(p_client, port);
    if (NULL == p_adj_port) {
       IDBG_HIGH("%s:%d] Invalid port", __func__, __LINE__);
       return FALSE;
    }
    switch(event->direction) {
      case MCT_EVENT_UPSTREAM : {
        IDBG_ERROR("%s:%d] Error Upstream event on Sink port %d",
          __func__, __LINE__, event->type);
        break;
      }
      case MCT_EVENT_BOTH:
      case MCT_EVENT_DOWNSTREAM: {
       rc =  mct_port_send_event_to_peer(p_adj_port, event);
       if (rc == FALSE) {
         IDBG_ERROR("%s:%d] Fowarding event %d from sink port failed",
           __func__, __LINE__, event->type);
       }
       break;
     }
     default:
       IDBG_ERROR("%s:%d] Invalid port direction for event %d",
         __func__, __LINE__, event->type);
       break;
    }
  } else if (MCT_PORT_IS_SRC(port)) {
    p_adj_port = (mct_port_t*)module_cac_find_adj_port(p_client, port);
    if (NULL == p_adj_port) {
       IDBG_HIGH("%s:%d] Invalid port", __func__, __LINE__);
       return FALSE;
    }
    switch(event->direction) {
      case MCT_EVENT_DOWNSTREAM : {
        IDBG_ERROR("%s:%d] Error Downstream event on Src port %d",
          __func__, __LINE__, event->type);
        break;
      }
      case MCT_EVENT_BOTH:
      case MCT_EVENT_UPSTREAM: {
       rc =  mct_port_send_event_to_peer(p_adj_port, event);
       if (rc == FALSE) {
         IDBG_ERROR("%s:%d] Fowarding event %d from src port failed",
           __func__, __LINE__, event->type);
       }
       break;
     }
     default:
       IDBG_ERROR("%s:%d] Invalid port direction for event %d",
         __func__, __LINE__, event->type);
       break;
    }
  }
  return rc;
}

#ifdef CUSTOM_ENABLE_LDS
/**
 * Function: module_cac_set_lds
 *
 * Description: This function is to update the LDS parameters
 * for offline usecase for cac v3 from chromatix header
 *
 *   @p_client: cac client
 *   @chromatix: chromatix pointer
 *   @lux_idx: Lux value
 *   @gain: Gain value
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_cac_set_lds(cac_session_params_t *p_session_params,
  chromatix_sw_postproc_type *chromatix, float lux_idx, float gain)
{
  int rc = IMG_SUCCESS;
  float trigger;

  chromatix_LDS_type *chromatix_lds =
    &chromatix->chromatix_luma_down_scaling;
  if (!chromatix_lds) {
    IDBG_ERROR("%s %d: Chromatix LDS is NULL", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  /* LDS trigger points from Chromatix header say A & B
      Min-----A-----B-----Max */
  trigger = (chromatix_lds->control_LDS == 0) ? (float)lux_idx : (float)gain;
  if (chromatix_lds->control_LDS == 0) {

    IDBG_MED("%s %d: Trigger %f, Trig A %ld, Tig B %ld", __func__, __LINE__,
      trigger, chromatix_lds->lds_trigger.lux_index_start,
      chromatix_lds->lds_trigger.lux_index_end);

    if (trigger > chromatix_lds->lds_trigger.lux_index_end) {
      p_session_params->lds_enable_flag = TRUE;
    } else if (trigger < chromatix_lds->lds_trigger.lux_index_start) {
      p_session_params->lds_enable_flag = FALSE;
    }
  } else {
    if (trigger > chromatix_lds->lds_trigger.gain_end) {
      p_session_params->lds_enable_flag = TRUE;
    } else if (trigger < chromatix_lds->lds_trigger.gain_start) {
      p_session_params->lds_enable_flag = FALSE;
    }
  }

  return rc;
}

/**
 * Function: module_cac_set_skin_rnr
 *
 * Description: This function is to update the SKIN RNR
 * parameters for offline usecase for cac v3 from chromatix
 * header
 *
 *   @p_client: cac client
 *   @chromatix: chromatix pointer
 *   @lux_idx: Lux value
 *   @gain: Gain value
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_cac_set_skin_rnr(cac_session_params_t *p_session_params,
  chromatix_sw_postproc_type *chromatix, float lux_idx, float gain)
{
  int rc = IMG_SUCCESS;
  float trigger;


  chromatix_RNR2_type * chromatix_rnr =
    &(chromatix->chromatix_radial_noise2_reduction);
  if (!chromatix_rnr) {
    IDBG_ERROR("%s %d: Chromatix RNR is NULL", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  if (!RNR_ENABLED(chromatix)) {
    IDBG_ERROR("%s %d: RNR not enabled for skin RNR", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  /* LDS trigger points from Chromatix header say A & B
      Min-----A-----B-----Max */
  trigger = (chromatix_rnr->control_RNR == 0) ? lux_idx : gain;

  IDBG_MED("%s %d: chromatix_rnr->control_RNR %d,"
    "trigger %f, "
    "gain %lf, "
    "lux_idx %lf, "
    "prev_lux_value %lf, "
    "prev_gain_value %lf, "
    "prev_sampling_factor %d, "
    "old s_rnr_enabled %d, ",
    __func__, __LINE__,
    chromatix_rnr->control_RNR,
    trigger,
    p_session_params->hyst_info.gain,
    p_session_params->hyst_info.lux_idx,
    p_session_params->hyst_info.prev_lux_value,
    p_session_params->hyst_info.prev_gain_value,
    p_session_params->hyst_info.prev_sampling_factor,
    p_session_params->hyst_info.s_rnr_enabled
    );

  if (chromatix_rnr->control_RNR == 0) {
    //lux based
    if (trigger < chromatix_rnr->skin_nr_trigger.lux_index_start ) {
      p_session_params->hyst_info.s_rnr_enabled = FALSE;
      IDBG_MED("%s %d: lux < start s_rnr_enabled %d,", __func__, __LINE__,
        p_session_params->hyst_info.s_rnr_enabled);
    } else if (trigger >= chromatix_rnr->skin_nr_trigger.lux_index_end) {
      p_session_params->hyst_info.s_rnr_enabled = TRUE;
      IDBG_MED("%s %d: lux >= end s_rnr_enabled %d,", __func__, __LINE__,
        p_session_params->hyst_info.s_rnr_enabled);
    } else {
      IDBG_MED("%s %d: start > lux < end current s_rnr_enabled %d,", __func__,
        __LINE__, p_session_params->hyst_info.s_rnr_enabled);
    }
  } else {
    //gain based
    if (trigger < chromatix_rnr->skin_nr_trigger.gain_start ) {
      p_session_params->hyst_info.s_rnr_enabled = FALSE;
      IDBG_MED("%s %d: gain < start s_rnr_enabled %d,", __func__, __LINE__,
        p_session_params->hyst_info.s_rnr_enabled);
    } else if (trigger >= chromatix_rnr->skin_nr_trigger.gain_end) {
      p_session_params->hyst_info.s_rnr_enabled = TRUE;
      IDBG_MED("%s %d: gain >= end s_rnr_enabled %d,", __func__, __LINE__,
        p_session_params->hyst_info.s_rnr_enabled);
    } else {
      IDBG_MED("%s %d: start > gain < end current s_rnr_enabled %d,", __func__,
        __LINE__, p_session_params->hyst_info.s_rnr_enabled);
    }
  }

  return rc;
}
#endif


/**
 * Function: module_cac_handle_aec_update
 *
 * Description: This method is used to handle aec update
 *
 * Arguments:
 *   @aec_update: AEC update structure
 *   @p_mod: CAC module
 *   @session_id: session ID
 *
 * Return values:
 *  None
 *
 * Notes: none
 **/
static void module_cac_handle_aec_update(aec_update_t *aec_update,
  module_cac_t *p_mod, uint32_t session_id)
{
  cac_session_params_t *session_params;

  if (NULL == aec_update) {
    IDBG_ERROR("%s %d: aec_update is NULL", __func__, __LINE__);
    return;
  }

  session_params = (cac_session_params_t *)
    module_cac_get_session_params(p_mod, session_id);
  if (!session_params) {
    IDBG_ERROR("%s:%d: Session params are null. Cannot post msg", __func__,
      __LINE__);
    return;
  }
  IDBG_MED("%s %d: session_params %0x,"
    "session_params->hyst_info.lux_idx %lf, "
    "aec_update->lux_idx %lf, ",
    __func__, __LINE__,
    (uint32_t)session_params,
    session_params->hyst_info.lux_idx,
    aec_update->lux_idx);

  /*Save previous lux/gain values*/
  session_params->hyst_info.prev_lux_value =
    session_params->hyst_info.lux_idx;
  session_params->hyst_info.prev_gain_value =
    session_params->hyst_info.gain;
  /*Update current lux/gain values*/
  session_params->hyst_info.lux_idx =
    aec_update->lux_idx;
  session_params->hyst_info.gain = aec_update->real_gain;

  session_params->lds_enable_flag = FALSE;

#ifdef CUSTOM_ENABLE_LDS
  /*Check if chromatix ptr is valid*/
  if (!session_params->chromatix) {
    IDBG_ERROR("Invalid chromatix ptr");
    return;
  }

  /*Run Skin RNR Hysteresis*/
  module_cac_set_skin_rnr(session_params,
    session_params->chromatix,
    session_params->hyst_info.lux_idx,
    session_params->hyst_info.gain);

  if ((aec_update->lds_Flag == 0)||(aec_update->lds_Flag == 4)) {
    /*Set LDS on/off*/
    module_cac_set_lds(session_params,
      session_params->chromatix,
      session_params->hyst_info.lux_idx,
      session_params->hyst_info.gain);
  } else {
    /* LDS always off in lls mode */
    session_params->lds_enable_flag = FALSE;
  }
#endif

  IDBG_MED("lux %f, gain %f, prev_lux %f, prev_gain %f, lls_flag %d lds %d",
    aec_update->lux_idx, aec_update->real_gain,
    session_params->hyst_info.prev_lux_value,
    session_params->hyst_info.prev_gain_value,
    aec_update->lls_flag,
    session_params->lds_enable_flag);

  return;
}


/**
 * Function: module_cac_handle_sof
 *
 * Description: Handle SOF event
 *
 * Arguments:
 *   @p_mod: CAC module
 *   @identity: Identity of the stream
 *
 * Return values:
 *     None
 *
 * Notes: none
 **/
void module_cac_handle_sof(module_cac_t *p_mod,
  uint32_t identity)
{
  mct_module_t *p_mct_mod;
  mct_bus_msg_t bus_msg;
  uint32_t session_id;
  cam_aberration_mode_t cac_mode;
  cac_session_params_t *session_params;
  cam_img_hysterisis_info_t img_msg;

  if (!p_mod) {
    IDBG_ERROR("%s %d]: Invalid input", __func__, __LINE__);
    return;
  }
  session_id = IMGLIB_SESSIONID(identity);
  session_params = (cac_session_params_t *)
    module_cac_get_session_params(p_mod, session_id);
  if (!session_params) {
    IDBG_ERROR("%s:%d: Session params are null. Cannot post msg", __func__,
      __LINE__);
    return;
  }

  cac_mode = session_params->cac_mode;
  p_mct_mod = p_mod->parent_mod;

  memset(&bus_msg, 0, sizeof(mct_bus_msg_t));
  bus_msg.type = MCT_BUS_MSG_SET_CAC_MODE;
  bus_msg.msg = &cac_mode;
  bus_msg.size = sizeof(cam_aberration_mode_t);
  bus_msg.sessionid = session_id;
  IDBG_MED("%s:%d] Posting cac_mode %d for session id %d", __func__, __LINE__,
    cac_mode, bus_msg.sessionid);
  mct_module_post_bus_msg(p_mct_mod, &bus_msg);

  img_msg.lds_enabled = session_params->lds_enable_flag;

  bus_msg.type = MCT_BUS_MSG_IMG_HYSTERSIS_INFO;
  bus_msg.msg = (void *)&img_msg;
  bus_msg.size = sizeof(cam_img_hysterisis_info_t);
  bus_msg.sessionid = session_id;
  IDBG_MED("%s:%d] session id %d mct_mod %p", __func__, __LINE__,
    bus_msg.sessionid, p_mct_mod);
  mct_module_post_bus_msg(p_mct_mod, &bus_msg);
}

/**
 * Function: module_cac_is_stream_valid
 *
 * Description: Check if the stream needs to be processed by cac
 *
 * Arguments:
 *   @p_client: CAC client
 *   @p_mod: CAC module
 *   @identity: Identity of the stream
 *   @frameid: Current Frameid
 *
 * Return values:
 *     True/False
 *
 * Notes: none
 **/
boolean module_cac_is_stream_valid(cac_client_t *p_client,
  module_cac_t *p_mod, uint32_t identity, uint32_t frameid)
{
  int32_t j = 0;
  int stream_type = 0;
  boolean stream_valid = FALSE;
  uint32_t session_id;
  cac_session_params_t *session_params;
  uint32_t frame_idx = frameid % MAX_PER_FRAME_LIST_COUNT;

  stream_type = module_cac_get_stream_type_by_id(p_client, identity);
  if (stream_type < 0) {
    IDBG_ERROR("%s %d] Invalid stream type", __func__, __LINE__);
    return FALSE;
  }

  //Always process buffers on the offline stream
  if ((cam_stream_type_t)stream_type == CAM_STREAM_TYPE_OFFLINE_PROC) {
    return TRUE;
  }

  //process only snapshot and offline stream for now
  if ((cam_stream_type_t)stream_type != CAM_STREAM_TYPE_SNAPSHOT) {
    return FALSE;
  }

  session_id = IMGLIB_SESSIONID(identity);
  session_params = (cac_session_params_t *)
    module_cac_get_session_params(p_mod, session_id);
  if (!session_params) {
    IDBG_ERROR("%s:%d: Session params are null. Cannot post msg", __func__,
      __LINE__);
    return FALSE;
  }

  //Find the frame in the list
  if (session_params->fpc_list[frame_idx].frameid == frameid) {
    int num_streams = session_params->fpc_list[frame_idx].valid_str_list.num_streams;
    for (j = 0; j < num_streams; j++) {
      if ((uint32_t)IMGLIB_STREAMID(identity) ==
        (cam_stream_type_t)session_params->fpc_list[frame_idx].
          valid_str_list.stream_request[j].streamID) {
        stream_valid = TRUE;
        IDBG_HIGH("%s %d]: stream id %d is valid for frameid %d, index=%d",
            __func__, __LINE__, IMGLIB_STREAMID(identity), frameid, frame_idx);
      }
    }
  }

  return stream_valid;
}

/**
 * Function: module_cac_update_streamlist
 *
 * Description: Update per frame stream info for each frame
 *
 * Arguments:
 *   @stream_list: List of valid streams for current frame
 *   @p_mod: CAC module
 *   @frameid: Current frame id
 *   @identity: Stream identity
 *
 * Return values:
 *     True/False
 *
 * Notes: none
 **/
void module_cac_update_streamlist(cam_stream_ID_t *stream_list,
  module_cac_t *p_mod, uint32_t frameid, uint32_t identity)
{
  uint32_t future_frame = 0;
  uint32_t session_id;
  cac_session_params_t *session_params;
  uint32_t frame_idx = 0;

  if (!stream_list || !p_mod) {
    IDBG_ERROR("%s %d] : Invalid input", __func__, __LINE__);
    return;
  }

  session_id = IMGLIB_SESSIONID(identity);
  session_params = (cac_session_params_t *)
    module_cac_get_session_params(p_mod, session_id);
  if (!session_params) {
    IDBG_ERROR("%s:%d: Session params are null. Cannot post msg", __func__,
      __LINE__);
    return;
  }

  //Calculate future frameid using pipeline delay
  future_frame = frameid +
    session_params->session_data.max_pipeline_frame_applying_delay;

  // Store the frame information in the array at index future_frame_id % array_size
  frame_idx = future_frame % MAX_PER_FRAME_LIST_COUNT;

  IDBG_MED("%s %d: pipeline delay %d, future frameid %d, array_indx=%d",
    __func__, __LINE__,
    session_params->session_data.max_pipeline_frame_applying_delay,
    future_frame, frame_idx);

  //Update the frame id and associated stream list
  session_params->fpc_list[frame_idx].frameid =
    future_frame;
  session_params->fpc_list[frame_idx].valid_str_list =
    *stream_list;

  return;
}

/**
 * Function: module_cac_port_event_func
 *
 * Description: Event handler function for the dummy port
 *
 * Arguments:
 *   @port: mct port pointer
 *   @event: mct event
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
boolean module_cac_port_event_func(mct_port_t *port,
  mct_event_t *event)
{
  int rc = IMG_SUCCESS;
  mct_module_t *p_mct_mod = NULL;
  module_cac_t *p_mod = NULL;
  cac_client_t *p_client;
  boolean fwd_event = TRUE;
  uint32_t session_id;
  int session_idx = 0;
  int str_idx;
  cac_session_params_t *session_params;
  int i;

  if (!port || !event) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }
  IDBG_LOW("%s:%d] port %p E", __func__, __LINE__, port);
  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    IDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_cac_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] CAC module NULL", __func__, __LINE__);
    return FALSE;
  }

  p_client = (cac_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("%s:%d] CAC client NULL", __func__, __LINE__);
    return FALSE;
  }

  str_idx = module_cac_get_stream_by_id(p_client, event->identity);
  if (str_idx < 0) {
    IDBG_ERROR("%s:%d] Stream with Identity 0x%x not mapped to client",
      __func__, __LINE__, event->identity);
    return FALSE;
  }

  session_id = IMGLIB_SESSIONID(p_client->stream[str_idx].identity);
  session_params = (cac_session_params_t *)
    module_cac_get_session_params(p_mod, session_id);
  if (!session_params) {
    IDBG_ERROR("%s:%d: Session params are null", __func__, __LINE__);
    return FALSE;
  }

  p_client->event_identity = event->identity;

  IDBG_LOW("%s:%d] type %d", __func__, __LINE__, event->type);
  switch (event->type) {
  case MCT_EVENT_CONTROL_CMD: {
    mct_event_control_t *p_ctrl_event = &event->u.ctrl_event;
    IDBG_MED("%s:%d] Ctrl type %d", __func__, __LINE__, p_ctrl_event->type);
    switch (p_ctrl_event->type) {
    case MCT_EVENT_CONTROL_STREAMON: {
      IDBG_HIGH("%s:%d] CAC STREAMON identity 0x%x", __func__, __LINE__,
        event->identity);
      pthread_mutex_lock(&p_client->mutex);
      p_client->stream[str_idx].stream_off = FALSE;
      pthread_mutex_unlock(&p_client->mutex);
      break;
    }
    case MCT_EVENT_CONTROL_STREAMOFF: {
      if (p_client->state == IMGLIB_STATE_STARTED) {
        IDBG_LOW("%s:%d] CAC client abort", __func__, __LINE__);
        module_cac_client_stop(p_client);
      }
      IDBG_HIGH("%s:%d] CAC STREAMOFF identity 0x%x", __func__, __LINE__,
        event->identity);
      pthread_mutex_lock(&p_client->mutex);
      p_client->stream[str_idx].stream_off = TRUE;
      pthread_mutex_unlock(&p_client->mutex);
      img_q_flush(&session_params->msg_thread.msg_q);
      break;
    }
    case MCT_EVENT_CONTROL_SET_SUPER_PARM: {
      uint16_t i;
      mct_event_super_control_parm_t *super_param =
        (mct_event_super_control_parm_t *)event->u.ctrl_event.control_event_data;
      for (i = 0; i < super_param->num_of_parm_events; i++) {
        switch(super_param->parm_events[i].type) {
        case CAM_INTF_META_STREAM_ID: {
          IDBG_MED("%s:%d] SET_SUPER_PARM: CAM_INTF_META_STREAM_ID",
            __func__, __LINE__);
          cam_stream_ID_t *stream_list =
            (cam_stream_ID_t *)(super_param->parm_events[i].parm_data);
          if (stream_list) {
            module_cac_update_streamlist(stream_list, p_mod,
              event->u.ctrl_event.current_frame_id, event->identity);
          } else {
            IDBG_ERROR("%s %d: CAM_INTF_META_STREAM_ID is null", __func__,
              __LINE__);
          }
          break;
        }
        case CAM_INTF_PARM_HAL_VERSION: {
          IDBG_MED("%s:%d] SET_SUPER_PARM: CAM_INTF_PARM_HAL_VERSION",
            __func__, __LINE__);
          int32_t *hal_version = (super_param->parm_events[i].parm_data);
          if (NULL == hal_version) {
            IDBG_ERROR("%s:%d] NULL invalid data", __func__, __LINE__);
          } else {
            session_params->hal_version = *hal_version;
            IDBG_MED("%s:%d] hal_version %d", __func__, __LINE__,
              session_params->hal_version);
          }
          break;
        }
        case CAM_INTF_META_STREAM_INFO: {
          IDBG_MED("%s:%d] SET_PARM: CAM_INTF_PARM_HAL_VERSION",
            __func__, __LINE__);
          /*reset the per frame stream list */
          memset(&session_params->fpc_list, 0,
            sizeof(cac_per_frame_params_t) *MAX_PER_FRAME_LIST_COUNT);
          break;
        }
        case CAM_INTF_PARM_CAC: {
          cam_aberration_mode_t *cac_mode =
            (cam_aberration_mode_t *)(super_param->parm_events[i].parm_data);
          if (cac_mode) {
            pthread_mutex_lock(&p_mod->mutex);
            session_params->cac_mode = *cac_mode;
            pthread_mutex_unlock(&p_mod->mutex);
          }
          IDBG_MED("%s:%d] SET_PARM: CAM_INTF_PARAM_CAC, cac_mode %d",
            __func__, __LINE__, session_params->cac_mode);
          break;
        }
        default:
          break;
        }
      }
      break;
    }
    case MCT_EVENT_CONTROL_SET_PARM: {
      mct_event_control_parm_t *param =
        event->u.ctrl_event.control_event_data;
      switch(param->type) {
      case CAM_INTF_META_STREAM_ID:{
        IDBG_MED("%s:%d] SET_PARM: CAM_INTF_META_STREAM_ID",
            __func__, __LINE__);
        cam_stream_ID_t *stream_list = (cam_stream_ID_t *)param->parm_data;
        if (stream_list) {
          module_cac_update_streamlist(stream_list, p_mod,
            event->u.ctrl_event.current_frame_id, event->identity);
        } else {
          IDBG_ERROR("%s %d: CAM_INTF_META_STREAM_ID is null",
            __func__, __LINE__);
        }
        break;
      }
      case CAM_INTF_PARM_HAL_VERSION: {
        IDBG_MED("%s:%d] SET_PARM: CAM_INTF_PARM_HAL_VERSION",
          __func__, __LINE__);
        int32_t *hal_version = (int32_t *)param->parm_data;
        if (NULL == hal_version) {
          IDBG_ERROR("%s:%d] NULL invalid data", __func__, __LINE__);
          break;
        }
        session_params->hal_version = *hal_version;
        IDBG_MED("%s:%d] hal_version %d", __func__, __LINE__,
          session_params->hal_version);
        break;
      }
      case CAM_INTF_META_STREAM_INFO: {
        IDBG_MED("%s:%d] SET_PARM: CAM_INTF_PARM_HAL_VERSION",
          __func__, __LINE__);
        /*reset the per frame stream list */
        memset(&session_params->fpc_list, 0,
          sizeof(cac_per_frame_params_t) *MAX_PER_FRAME_LIST_COUNT);
        break;
      }
      case CAM_INTF_PARM_CAC: {
        cam_aberration_mode_t *cac_mode =
          (cam_aberration_mode_t *)param->parm_data;
        if (cac_mode) {
          pthread_mutex_lock(&p_mod->mutex);
          session_params->cac_mode = *cac_mode;
          pthread_mutex_unlock(&p_mod->mutex);
        }
        IDBG_MED("%s:%d] SET_PARM: CAM_INTF_PARAM_CAC, cac_mode %d", __func__,
          __LINE__, session_params->cac_mode);
        break;
      }
      default:
        break;
      }
    break;
    }
    case MCT_EVENT_CONTROL_SOF: {
      IDBG_MED("%s:%d] MCT_EVENT_CONTROL_SOF stream type %d", __func__,
        __LINE__, p_client->stream[str_idx].stream_info->stream_type);
      module_cac_handle_sof(p_mod, event->identity);
      break;
    }
    default:
      break;
    }
    break;
  }
  case MCT_EVENT_MODULE_EVENT: {
    mct_event_module_t *p_mod_event = &event->u.module_event;
    IDBG_MED("%s:%d] Mod type %d", __func__, __LINE__, p_mod_event->type);
    switch (p_mod_event->type) {
    case MCT_EVENT_MODULE_BUF_DIVERT: {
      mod_img_msg_t msg;
      boolean is_stream_valid = TRUE;
      isp_buf_divert_t *p_buf_divert =
        (isp_buf_divert_t *)p_mod_event->module_event_data;

      IDBG_MED("%s:%d] MCT_EVENT_MODULE_BUF_DIVERT for event_identity 0x%x"
        "buf_divert_identity 0x%x", __func__, __LINE__, event->identity,
         p_buf_divert->identity);
      if (session_params->hal_version == CAM_HAL_V3) {
        IDBG_MED("%s %d]: BUF_DIVERT for frame id %d ", __func__, __LINE__,
                      p_buf_divert->buffer.sequence);
        is_stream_valid = module_cac_is_stream_valid(p_client, p_mod,
          event->identity, p_buf_divert->buffer.sequence);
        if (session_params->cac_mode !=
          CAM_COLOR_CORRECTION_ABERRATION_HIGH_QUALITY) {
          is_stream_valid = FALSE;
        }
      }
      if (p_client->stream[str_idx].stream_info->stream_type ==
        CAM_STREAM_TYPE_PREVIEW ||
        p_client->stream[str_idx].stream_info->stream_type ==
        CAM_STREAM_TYPE_VIDEO) {
          is_stream_valid = FALSE;
      }
      if (is_stream_valid) {
        memset(&msg, 0x0, sizeof(mod_img_msg_t));
        msg.port = port;
        msg.type = MOD_IMG_MSG_DIVERT_BUF;
        msg.data.buf_divert.buf_divert = *p_buf_divert;
        msg.data.buf_divert.identity = event->identity;
        msg.data.buf_divert.p_exec = module_cac_client_divert_exec;
        msg.data.buf_divert.userdata = (void *)p_client;
        module_imglib_send_msg(&session_params[session_idx].msg_thread,
          &msg);

        /* indicate that the buffer is consumed */
        p_buf_divert->is_locked = FALSE;
        p_buf_divert->ack_flag = FALSE;
        fwd_event = FALSE;
      } else {
        IDBG_MED("%s %d] Not processing MCT_EVENT_MODULE_BUF_DIVERT for "
          "identity 0x%x is_valid %d, cac_mode %d", __func__, __LINE__,
          p_buf_divert->identity, is_stream_valid, session_params->cac_mode);
        p_buf_divert->is_locked = FALSE;
        p_buf_divert->ack_flag = FALSE;
        fwd_event = TRUE;
      }
      break;
    }

    case MCT_EVENT_MODULE_STATS_AWB_UPDATE: {
      stats_update_t *stats_update = (stats_update_t *)
        p_mod_event->module_event_data;

      if (stats_update->awb_update.awb_dst == STATS_UPDATE_DST_ALL ||
          stats_update->awb_update.awb_dst & STATS_UPDATE_DST_CAC) {
        p_client->cac_cfg_info.cac_3a_data.awb_gr_gain =
          (float)FLOAT_TO_Q(7, stats_update->awb_update.gain.g_gain);
        p_client->cac_cfg_info.cac_3a_data.awb_gb_gain =
          (float)FLOAT_TO_Q(7, stats_update->awb_update.gain.g_gain);
        IDBG_MED("%s: abw gr = %f, awb gb = %f", __func__,
          p_client->cac_cfg_info.cac_3a_data.awb_gr_gain,
          p_client->cac_cfg_info.cac_3a_data.awb_gb_gain);
      } else {
        IDBG_ERROR("%s skipping AWB update due to the dst flag", __func__);
      }
      break;
    }
    case MCT_EVENT_MODULE_ISP_AWB_UPDATE: {
      awb_update_t *awb_update = (awb_update_t *)p_mod_event->module_event_data;

      p_client->cac_cfg_info.cac_3a_data.awb_gr_gain =
        (float)FLOAT_TO_Q(7, awb_update->gain.g_gain);
      p_client->cac_cfg_info.cac_3a_data.awb_gb_gain =
        (float)FLOAT_TO_Q(7, awb_update->gain.g_gain);
      IDBG_MED("%s: abw gr = %f, awb gb = %f", __func__,
        p_client->cac_cfg_info.cac_3a_data.awb_gr_gain,
        p_client->cac_cfg_info.cac_3a_data.awb_gb_gain);
      break;
    }
    case MCT_EVENT_MODULE_ISP_GAMMA_UPDATE: {
      uint16_t *p_gamma_in = (uint16_t *)p_mod_event->module_event_data;
      img_gamma_t *p_gamma = &p_client->cac_cfg_info.r_gamma;

      for (i = 0; i < GAMMA_TABLE_ENTRIES; i++) {
        p_gamma->table[i] = (uint16_t)p_gamma_in[i];
        IDBG_MED("gamma[%d] 0x%x", i, p_client->cac_cfg_info.r_gamma.table[i]);
      }
      p_client->cac_cfg_info.b_gamma = p_client->cac_cfg_info.r_gamma;
      p_client->cac_cfg_info.g_gamma = p_client->cac_cfg_info.r_gamma;
      break;
    }
    case MCT_EVENT_MODULE_SET_CHROMATIX_PTR: {
      modulesChromatix_t *chromatix_param =
        (modulesChromatix_t*)p_mod_event->module_event_data;
      if (chromatix_param) {
        session_params[session_idx].chromatix =
          chromatix_param->chromatixPostProcPtr;
        p_client->cac_cfg_info.chromatix_ptr =
          chromatix_param->chromatixPostProcPtr;
        IDBG_MED("%s %d: MCT_EVENT_MODULE_SET_CHROMATIX_PTR %p", __func__,
          __LINE__, session_params[session_idx].chromatix);
      } else {
        IDBG_ERROR("%s %d: MCT_EVENT_MODULE_SET_CHROMATIX_PTR"
         "chromatix param is null", __func__, __LINE__);
      }
      break;
    }
    case MCT_EVENT_MODULE_STATS_AEC_UPDATE: {
      IDBG_MED("%s:%d] MCT_EVENT_MODULE_STATS_AEC_UPDATE", __func__, __LINE__);
      uint32_t stream_type = module_cac_get_stream_type_by_id(p_client,
        p_client->stream[str_idx].identity);
      stats_update_t *stats_update = (stats_update_t *)
        p_mod_event->module_event_data;
      IDBG_MED("%s:%d] MCT_EVENT_MODULE_STATS_AEC_UPDATE, stream_type %d",
        __func__, __LINE__, stream_type);
      if (stats_update) {
        aec_update_t *aec_update = &stats_update->aec_update;
        /* Check new capture mode and destination */
        if (aec_update->aec_dst &&
          !(aec_update->aec_dst & STATS_UPDATE_DST_CAC)) {
          IDBG_HIGH("[newcapture]%s:%d, no need to update AEC", __func__,
            __LINE__);
          break;
        }
        /*Todo:  aec_update_t change for LDS & LLS */
        module_cac_handle_aec_update(aec_update, p_mod, session_id);
        p_client->cac_cfg_info.cac_3a_data.lux_idx =
          stats_update->aec_update.lux_idx;
        p_client->cac_cfg_info.cac_3a_data.gain =
          stats_update->aec_update.real_gain;
        IDBG_MED("%s: lux_idx = %f, gain = %f", __func__,
          p_client->cac_cfg_info.cac_3a_data.lux_idx,
          p_client->cac_cfg_info.cac_3a_data.gain);

      } else {
        IDBG_ERROR("%s %d:stats_update from MCT_EVENT_MODULE_STATS_AEC_UPDATE"
          "is NULL", __func__, __LINE__);
      }
      break;
    }
    case MCT_EVENT_MODULE_SET_STREAM_CONFIG: {
      IDBG_MED("%s:%d] MCT_EVENT_MODULE_SET_STREAM_CONFIG", __func__, __LINE__);
      sensor_out_info_t *sensor_out_info = (sensor_out_info_t*)
        p_mod_event->module_event_data;
      if (sensor_out_info) {
        session_params[session_idx].camif_output.width =
          sensor_out_info->dim_output.width;
        session_params[session_idx].camif_output.height =
          sensor_out_info->dim_output.height;
      } else {
        IDBG_ERROR("%s %d: Camif o/p from MCT_EVENT_MODULE_SET_STREAM_CONFIG"
          "is NULL", __func__, __LINE__);
      }
      break;
    }
    case MCT_EVENT_MODULE_ISP_OUTPUT_DIM: {
      mct_stream_info_t *stream_info =
        (mct_stream_info_t *)(event->u.module_event.module_event_data);
      if (!stream_info) {
        IDBG_ERROR("%s:%d, failed\n", __func__, __LINE__);
        return -EFAULT;
      }
      IDBG_MED("%s:%d] MCT_EVENT_MODULE_ISP_OUTPUT_DIM stream info %dx%d "
        "identity 0x%x, format %d", __func__, __LINE__, stream_info->dim.width,
        stream_info->dim.height, event->identity, stream_info->fmt);
      int str_idx = module_cac_get_stream_by_id(p_client,  event->identity);
      if (str_idx < 0) {
        IDBG_ERROR("%s %d: Cannot find stream with identity 0x%x mapped to the"
          "client", __func__, __LINE__, p_client->p_buf_divert_data->identity);
        return IMG_ERR_GENERAL;
      }
      //Update Stream info dimensions
      p_client->stream[str_idx].in_dim.width = stream_info->dim.width;
      p_client->stream[str_idx].in_dim.height = stream_info->dim.height;
      p_client->stream[str_idx].in_dim.scanline =
        stream_info->buf_planes.plane_info.mp[0].scanline;
      p_client->stream[str_idx].in_dim.stride =
        stream_info->buf_planes.plane_info.mp[0].stride;
      break;
    }
    default:
      break;
    }
    break;
  }
  default:
   break;
  }

  if (fwd_event) {
    boolean brc = module_cac_forward_port_event(p_client, port, event);
    rc = (brc) ? IMG_SUCCESS : IMG_ERR_GENERAL;
  }

  return GET_STATUS(rc);
}



/**
 * Function: module_cac_acquire_port
 *
 * Description: This function is used to acquire the port
 *
 * Arguments:
 *   @p_mct_mod: mct module pointer
 *   @port: mct port pointer
 *   @stream_info: stream information
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
boolean module_cac_port_acquire(mct_module_t *p_mct_mod,
  mct_port_t *port,
  mct_stream_info_t *stream_info)
{
  int rc = IMG_SUCCESS, i = 0;
  unsigned int p_identity ;
  mct_list_t *p_temp_list = NULL;
  cac_client_t *p_client = NULL;
  module_cac_t *p_mod = NULL;
  boolean is_compatible = TRUE;
  uint32_t port_sess_id;

  IDBG_MED("%s:%d] E", __func__, __LINE__);

  p_mod = (module_cac_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] cac module NULL", __func__, __LINE__);
    return FALSE;
  }
  p_identity =  stream_info->identity;

  /* check if its sink port*/
  if (MCT_PORT_IS_SINK(port)) {
    /* create cac client */
    rc = module_cac_client_create(p_mct_mod, port, stream_info);
    if (IMG_SUCCEEDED(rc)) {
      p_client = (cac_client_t *) port->port_private;
      p_client->stream_mask |= (1 << IMGLIB_STREAMID(p_identity));
      IDBG_LOW("%s:%d] client %p %x type %d", __func__, __LINE__,
        p_client, p_client->stream_mask,
        stream_info->stream_type);
      rc = module_cac_fill_stream(p_client, stream_info, port);
    }
  } else {
    if (port->object.refcount > 0) {
      cac_client_t *p_temp_client = port->port_private;
      if (p_temp_client) {
        IDBG_LOW("%s:%d] port %s found client %p %x type %d", __func__,
          __LINE__,
          MCT_PORT_NAME(port), p_temp_client, p_temp_client->stream_mask,
          stream_info->stream_type);

        /*check if port is already connected in a  different session*/
        for (i = 0; i < p_temp_client->stream_cnt; i++) {
          port_sess_id = IMGLIB_SESSIONID(p_temp_client->stream[i].identity);
          if (port_sess_id != IMGLIB_SESSIONID(stream_info->identity)) {
            IDBG_MED("%s %d]: Port already connected on sessionid %d,"
              "Current sessionid %d", __func__, __LINE__, port_sess_id,
               IMGLIB_SESSIONID(stream_info->identity));
            goto error;
          }
        }
        is_compatible = p_temp_client->stream_mask &
          (1 << IMGLIB_STREAMID(stream_info->identity));
      } else {
        IDBG_ERROR("%s:%d] cannot find the client", __func__, __LINE__);
        goto error;
      }
    }
    if (is_compatible) {
       /* update the internal connection with source port */
       p_temp_list = mct_list_find_custom(p_mod->cac_client, &p_identity,
         module_cac_find_client);
       if (NULL != p_temp_list) {
         p_client = p_temp_list->data;
         port->port_private = p_client;
         IDBG_MED("%s:%d] found client %p", __func__, __LINE__, p_client);
         rc = module_cac_fill_stream(p_client, stream_info, port);
       } else {
         IDBG_ERROR("%s:%d] cannot find the client", __func__, __LINE__);
         goto error;
      }
    } else {
      IDBG_MED("%s:%d] Port not compatible",__func__, __LINE__);
      rc = IMG_ERR_GENERAL;
    }
  }
  if (!IMG_SUCCEEDED(rc)) {
    goto error;
  }
  IDBG_MED("%s:%d] port %p port_private %p X", __func__, __LINE__,
    port, port->port_private);
  return GET_STATUS(rc);

error:

  IDBG_MED("%s:%d] Error X", __func__, __LINE__);
  return FALSE;

}

/**
 * Function: module_cac_port_check_caps_reserve
 *
 * Description: This function is used to reserve the port
 *
 * Arguments:
 *   @port: mct port pointer
 *   @caps: pointer to peer capabilities
 *   @stream_info: stream information
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
boolean module_cac_port_check_caps_reserve(mct_port_t *port, void *caps,
  void *vstream_info)
{
  boolean rc = FALSE;
  mct_port_caps_t *peer_caps = caps;
  mct_module_t *p_mct_mod = NULL;
  module_cac_t *p_mod = NULL;
  mct_stream_info_t *stream_info = (mct_stream_info_t *)vstream_info;
  cac_client_t *p_client = NULL;
  uint32_t port_sess_id;
  int i = 0;

  IDBG_MED("%s:%d] E", __func__, __LINE__);
  if (!port || !stream_info) {
    CDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  if (peer_caps && (peer_caps->port_caps_type != port->caps.port_caps_type)) {
    CDBG_ERROR("%s:%d peer caps missmatch", __func__, __LINE__);
    return FALSE;
  }

  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    CDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_cac_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    CDBG_ERROR("%s:%d] cac module NULL", __func__, __LINE__);
    return FALSE;
  }

  /* lock the module */
  pthread_mutex_lock(&p_mod->mutex);

  IDBG_HIGH("%s %d: on Port %s for Stream type %d, Identity 0x%x", __func__,
    __LINE__, MCT_PORT_NAME(port), stream_info->stream_type,
    stream_info->identity);

  if ((port->object.refcount > 0) && (MCT_PORT_IS_SINK(port))) {
    p_client = port->port_private;
    if (!p_client) {
      IDBG_ERROR("%s:%d] error, no client ptr", __func__, __LINE__);
      pthread_mutex_unlock(&p_mod->mutex);
      return FALSE;
    }
    for (i = 0; i < p_client->stream_cnt; i++) {
      port_sess_id = IMGLIB_SESSIONID(p_client->stream[i].identity);
      if (port_sess_id != IMGLIB_SESSIONID(stream_info->identity)) {
      IDBG_ERROR("%s %d]: Port %s already connected on sessionid %d,"
        "Current sessionid %d", __func__, __LINE__, MCT_PORT_NAME(port),
        port_sess_id, IMGLIB_SESSIONID(stream_info->identity));
      pthread_mutex_unlock(&p_mod->mutex);
      return FALSE;
      }
    }
    rc = module_cac_fill_stream(p_client, stream_info, port);
    if (IMG_SUCCEEDED(rc)) {
      rc = TRUE;
      p_client->stream_mask |= (1 << IMGLIB_STREAMID(stream_info->identity));
    }
  } else {
    rc = module_cac_port_acquire(p_mct_mod, port, stream_info);
  }
  if (FALSE == rc) {
    IDBG_MED("%s:%d] Error acquiring sink port", __func__, __LINE__);
    pthread_mutex_unlock(&p_mod->mutex);
    return FALSE;
  }
  port->object.refcount++;

  IDBG_HIGH("%s:%d] Port %s for stream_type %d, identity 0x%x ref_cnt %d",
    __func__, __LINE__, MCT_PORT_NAME(port), stream_info->stream_type,
    stream_info->identity,
    port->object.refcount);

  pthread_mutex_unlock(&p_mod->mutex);
  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return TRUE;

}

/**
 * Function: module_cac_port_release_client
 *
 * Description: This method is used to release the client after
 *  all the ports are destroyed
 *
 * Arguments:
 *   @p_mod: pointer to the CAC module
 *   @identity: stream/session id
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_cac_port_release_client(module_cac_t *p_mod,
  mct_port_t *port,
  cac_client_t *p_client,
  unsigned int identity)
{
  IMG_UNUSED(port);

  mct_list_t *p_temp_list = NULL;
  int idx = 0;
  p_temp_list = mct_list_find_custom(p_mod->cac_client, &identity,
    module_cac_find_client);
  if (NULL != p_temp_list) {
    IDBG_MED("%s:%d] ", __func__, __LINE__);
    p_mod->cac_client = mct_list_remove(p_mod->cac_client,
      p_temp_list->data);
  }
  if (MCT_PORT_IS_SINK(port)) {
    idx = module_cac_get_stream_by_id(p_client, identity);
    module_cac_client_destroy(p_client);
  }
}

/**
 * Function: module_cac_port_check_caps_unreserve
 *
 * Description: This method is used to unreserve the port
 *
 * Arguments:
 *   @identity: identitity for the session and stream
 *   @port: mct port pointer
 *   @peer: peer mct port pointer
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
boolean module_cac_port_check_caps_unreserve(mct_port_t *port,
  unsigned int identity)
{
  mct_module_t *p_mct_mod = NULL;
  module_cac_t *p_mod = NULL;
  cac_client_t *p_client = NULL;

  if (!port) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  IDBG_HIGH("%s:%d] E port %s, identity 0x%x, dir %d", __func__, __LINE__,
    MCT_PORT_NAME(port), identity, MCT_PORT_DIRECTION(port));

  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    IDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_cac_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] cac module NULL", __func__, __LINE__);
    return FALSE;
  }

  p_client = (cac_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("%s:%d] cac client NULL", __func__, __LINE__);
    return FALSE;
  }

  /* lock the module */
  pthread_mutex_lock(&p_mod->mutex);
  port->object.refcount --;

  if (port->object.refcount == 0) {
    module_cac_port_release_client(p_mod, port, p_client, identity);
    port->port_private = NULL;
  }

  IDBG_HIGH("%s:%d] Port %s for identity 0x%x ref_cnt %d",
    __func__, __LINE__, MCT_PORT_NAME(port), identity,
    MCT_OBJECT_REFCOUNT(port));
  pthread_mutex_unlock(&p_mod->mutex);

  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return TRUE;
}

/**
 * Function: module_cac_port_ext_link
 *
 * Description: This method is called when the user establishes
 *              link.
 *
 * Arguments:
 *   @identity: identitity for the session and stream
 *   @port: mct port pointer
 *   @peer: peer mct port pointer
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
boolean module_cac_port_ext_link(unsigned int identity,
  mct_port_t* port, mct_port_t *peer)
{
  IMG_UNUSED(identity);

  mct_module_t *p_mct_mod = NULL;
  module_cac_t *p_mod = NULL;
  cac_client_t *p_client = NULL;

  if (!port || !peer) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return FALSE;
  }

  IDBG_MED("%s:%d] port %p E", __func__, __LINE__, port);
  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    IDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_cac_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] cac module NULL", __func__, __LINE__);
    return FALSE;
  }

  p_client = (cac_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("%s:%d] invalid client", __func__, __LINE__);
    return FALSE;
  }

  if (MCT_PORT_PEER(port)) {
    IDBG_ERROR("%s:%d] link already established", __func__, __LINE__);
    return TRUE;
  }

  MCT_PORT_PEER(port) = peer;

  /* check if its sink port*/
  if (MCT_PORT_IS_SINK(port)) {
    /* start cac client in case of dynamic module */
  } else {
    /* do nothing for source port */
  }
  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return TRUE;
}

/**
 * Function: module_cac_port_unlink
 *
 * Description: This method is called when the user disconnects
 *              the link.
 *
 * Arguments:
 *   @identity: identitity for the session and stream
 *   @port: mct port pointer
 *   @peer: peer mct port pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_cac_port_unlink(unsigned int identity,
  mct_port_t *port, mct_port_t *peer)
{
  IMG_UNUSED(identity);

  mct_module_t *p_mct_mod = NULL;
  module_cac_t *p_mod = NULL;
  cac_client_t *p_client = NULL;

  if (!port || !peer) {
    IDBG_ERROR("%s:%d invalid input", __func__, __LINE__);
    return;
  }

  IDBG_MED("%s:%d] E", __func__, __LINE__);
  p_mct_mod = MCT_MODULE_CAST((MCT_PORT_PARENT(port))->data);
  if (!p_mct_mod) {
    IDBG_ERROR("%s:%d invalid module", __func__, __LINE__);
    return;
  }

  p_mod = (module_cac_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] cac module NULL", __func__, __LINE__);
    return;
  }

  p_client = (cac_client_t *)port->port_private;
  if (NULL == p_client) {
    IDBG_ERROR("%s:%d] cac client NULL", __func__, __LINE__);
    return;
  }

  if (MCT_PORT_IS_SINK(port)) {
    /* stop the client in case of dynamic module */
  } else {
    /* do nothing for source port*/
  }

  if (port->object.refcount == 1) {
    MCT_PORT_PEER(port) = NULL;
  }

  IDBG_MED("%s:%d] X", __func__, __LINE__);
  return;
}

/**
 * Function: module_cac_port_set_caps
 *
 * Description: This method is used to set the capabilities
 *
 * Arguments:
 *   @port: mct port pointer
 *   @caps: mct port capabilities
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
boolean module_cac_port_set_caps(mct_port_t *port,
  mct_port_caps_t *caps)
{
  IMG_UNUSED(port);
  IMG_UNUSED(caps);

  return TRUE;
}


/**
 * Function: module_cac_free_port
 *
 * Description: This function is used to free the cac ports
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
static boolean module_cac_free_port(void *data, void *user_data)
{
  mct_port_t *p_port = (mct_port_t *)data;
  mct_module_t *p_mct_mod = (mct_module_t *)user_data;
  boolean rc = FALSE;

  if (!p_port || !p_mct_mod) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return TRUE;
  }
  IDBG_MED("%s:%d port %p p_mct_mod %p", __func__, __LINE__, p_port,
    p_mct_mod);

  rc = mct_module_remove_port(p_mct_mod, p_port);
  if (rc == FALSE) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
  }
  mct_port_destroy(p_port);
  return TRUE;
}

/**
 * Function: module_cac_create_port
 *
 * Description: This function is used to create a port and link with the
 *              module
 *
 * Arguments:
 *   none
 *
 * Return values:
 *     MCTL port pointer
 *
 * Notes: none
 **/
mct_port_t *module_cac_create_port(mct_module_t *p_mct_mod,
  mct_port_direction_t dir)
{
  char portname[PORT_NAME_LEN];
  mct_port_t *p_port = NULL;
  int status = IMG_SUCCESS;
  int index = 0;

  if (!p_mct_mod || (MCT_PORT_UNKNOWN == dir)) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return NULL;
  }

  index = (MCT_PORT_SINK == dir) ? p_mct_mod->numsinkports :
    p_mct_mod->numsrcports;
  /*portname <mod_name>_direction_portIndex*/
  snprintf(portname, sizeof(portname), "%s_d%d_i%d",
    MCT_MODULE_NAME(p_mct_mod), dir, index);
  p_port = mct_port_create(portname);
  if (NULL == p_port) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return NULL;
  }
  IDBG_MED("%s:%d portname %s", __func__, __LINE__, portname);

  p_port->direction = dir;
  p_port->port_private = NULL;
  p_port->caps.port_caps_type = MCT_PORT_CAPS_FRAME;

  /* override the function pointers */
  p_port->check_caps_reserve    = module_cac_port_check_caps_reserve;
  p_port->check_caps_unreserve  = module_cac_port_check_caps_unreserve;
  p_port->ext_link              = module_cac_port_ext_link;
  p_port->un_link               = module_cac_port_unlink;
  p_port->set_caps              = module_cac_port_set_caps;
  p_port->event_func            = module_cac_port_event_func;
  p_mct_mod->query_mod          = module_cac_query_mod;
   /* add port to the module */
  if (!mct_module_add_port(p_mct_mod, p_port)) {
    IDBG_ERROR("%s: Set parent failed", __func__);
    status = IMG_ERR_GENERAL;
    goto error;
  }

  if (MCT_PORT_SRC == dir) {
    p_mct_mod->numsrcports++;
  } else {
    p_mct_mod->numsinkports++;
  }

  IDBG_MED("%s:%d ", __func__, __LINE__);
  return p_port;

error:

  IDBG_ERROR("%s:%d] failed", __func__, __LINE__);
  if (p_port) {
    mct_port_destroy(p_port);
    p_port = NULL;
  }
  return NULL;
}

/**
 * Function: module_cac_free_mod
 *
 * Description: This function is used to free the CAC module
 *
 * Arguments:
 *   p_mct_mod - MCTL module instance pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_cac_deinit(mct_module_t *p_mct_mod)
{
  module_cac_t *p_mod = NULL;
  img_core_ops_t *p_core_ops = NULL;
  mct_list_t* p_list;

  if (NULL == p_mct_mod) {
    IDBG_ERROR("%s:%d] MCTL module NULL", __func__, __LINE__);
    return;
  }

  p_mod = (module_cac_t *)p_mct_mod->module_private;
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d] cac module NULL", __func__, __LINE__);
    return;
  }

  do {
    p_list = mct_list_find_custom(MCT_MODULE_SINKPORTS(p_mct_mod), p_mct_mod,
      module_imglib_get_next_from_list);
    if (p_list)
      module_cac_free_port(p_list->data, p_mct_mod);
  } while (p_list);

  do {
    p_list = mct_list_find_custom(MCT_MODULE_SRCPORTS(p_mct_mod), p_mct_mod,
      module_imglib_get_next_from_list);
    if (p_list)
      module_cac_free_port(p_list->data, p_mct_mod);
  } while (p_list);


  p_core_ops = &p_mod->core_ops;
  IDBG_MED("%s:%d lib_ref_cnt %d", __func__, __LINE__, p_mod->lib_ref_count);
  if (p_mod->lib_ref_count) {
    IMG_COMP_UNLOAD(p_core_ops);
  }
  p_mod->cac_client_cnt = 0;
  pthread_mutex_destroy(&p_mod->mutex);
  pthread_cond_destroy(&p_mod->cond);

  mct_module_destroy(p_mct_mod);
  p_mct_mod = NULL;
}


/**
 * Function: module_cac_create_session_param
 *
 * Description: This method is used to create session parameters
 * it will add new list with parameters session based
 *
 * Arguments:
 *   @p_mod: Cac Module
 *   @session_id Session id
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static int module_cac_create_session_param(module_cac_t *p_mod,
  uint32_t session_id)
{
  cac_session_params_t *stored_param;
  int rc = IMG_SUCCESS;

  IDBG_LOW("E");

  if (!p_mod) {
    return IMG_ERR_INVALID_INPUT;
  }

  stored_param = malloc(sizeof(*stored_param));
  if (NULL == stored_param) {
    return IMG_ERR_NO_MEMORY;
  }

  memset(stored_param, 0x0, sizeof(cac_session_params_t));
  stored_param->sessionid = session_id;
  //stored_param->valid_params = FALSE;

  /* create message thread */
  rc = module_imglib_create_msg_thread(&stored_param->msg_thread);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d failed to create msg thread for session %d", __func__,
      __LINE__, session_id);
    free(stored_param);
    return IMG_ERR_GENERAL;
  }

  p_mod->session_params = mct_list_append(p_mod->session_params,
    stored_param, NULL, NULL);

  if (IMG_SUCCEEDED(rc)) {
    mod_img_msg_t img_msg;
    memset(&img_msg, 0x0, sizeof(mod_img_msg_t));
    img_msg.type = MOD_IMG_MSG_EXEC_INFO;
    img_msg.data.exec_info.data = (void*)p_mod;
    img_msg.data.exec_info.p_exec = module_cac_client_allocate_cac_buffer;
    img_msg.data.exec_info.p_userdata = (void *)&stored_param->sessionid;
    module_imglib_send_msg(&stored_param->msg_thread, &img_msg);
  }

  IDBG_LOW("X");

  return (p_mod->session_params) ? IMG_SUCCESS : IMG_ERR_GENERAL;
}


/**
 * Function: module_cac_destroy_session_param
 *
 * Description: This method is used to destroy session parameters
 * it will add new list with parameters session based
 *
 * Arguments:
 *   @p_mod: CAC Module
 *   @session_id Session id
 *
 * Return values:
 *     true/false
 *
 * Notes: none
 **/
static int module_cac_destroy_session_param(module_cac_t *p_mod,
  uint32_t session_id)
{
  cac_session_params_t *stored_param;
  mct_list_t *p_temp_list;
  int rc = IMG_SUCCESS;

  if (!p_mod) {
    return IMG_ERR_INVALID_INPUT;
  }

  /* Find paramters per session id */
  p_temp_list = mct_list_find_custom(p_mod->session_params, &session_id,
    module_cac_find_session_params);
  if (!p_temp_list) {
    return IMG_ERR_INVALID_INPUT;
  }

  stored_param = ( cac_session_params_t *)p_temp_list->data;

  stored_param->prealloc_mem_rel =  module_imglib_common_get_prop(
    "persist.vendor.camera.cac.memopt", "0");

  IDBG_MED("release cac prealloc buffers %d",
    stored_param->prealloc_mem_rel);

  //release buffers when memory saving is preferred
  if (stored_param->prealloc_mem_rel) {
    module_cac_client_release_cac_buffer(&session_id, p_mod);
  }

    /* destroy message thread */
  rc = module_imglib_destroy_msg_thread(&stored_param->msg_thread);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d failed to destroy msg thread for session %d", __func__,
      __LINE__, session_id);
    return IMG_ERR_GENERAL;
  }

  p_mod->session_params = mct_list_remove(p_mod->session_params, stored_param);
  free(stored_param);

  return rc;
}

/**
 * Function: module_cac_start_session
 *
 * Description: This function is called when a new camera
 *              session is started
 *
 * Arguments:
 *   @module: mct module pointer
 *   @sessionid: session id
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
static boolean module_cac_start_session(mct_module_t *module,
  unsigned int sessionid)
{
  int rc = IMG_SUCCESS;
  module_cac_t *p_mod;

  IDBG_LOW("E");

  if (!module) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_cac_t *)module->module_private;
  if (!p_mod) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  /* Add session settings */
  rc = module_cac_create_session_param(p_mod, sessionid);

  IDBG_LOW("X %d", rc);

  return GET_STATUS(rc);
}

/**
 * Function: module_cac_stop_session
 *
 * Description: This function is called when the camera
 *              session is stopped
 *
 * Arguments:
 *   @module: mct module pointer
 *   @sessionid: session id
 *
 * Return values:
 *     error/success
 *
 * Notes: none
 **/
static boolean module_cac_stop_session(mct_module_t *module,
  unsigned int sessionid)
{
  int rc = IMG_SUCCESS;
  module_cac_t *p_mod;

  IDBG_LOW("E");

  if (!module) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  p_mod = (module_cac_t *)module->module_private;
  if (!p_mod) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    return FALSE;
  }

  rc = module_cac_destroy_session_param(p_mod, sessionid);

  IDBG_LOW("X %d", rc);

  return GET_STATUS(rc);
}

/** module_cac_set_session_data
 *
 *  @module: cac module handle
 *  @set_buf: set buffer handle that has session data
 *  @sessionid: session id for which session data shall be
 *            applied
 *
 *  This function provides session data that has per frame
 *  control parameters
 *
 *  Return: TRUE on success and FALSE on failure
 **/
static boolean module_cac_set_session_data(mct_module_t *module,
  void *set_buf, unsigned int sessionid)
{
  mct_pipeline_session_data_t *frame_ctrl_data;
  module_cac_t *p_mod;
  cac_session_params_t *session_params;

  if (!(module && module->module_private) || !set_buf) {
    IDBG_ERROR("%s:%d] Error: invalid params %p %p",
      __func__, __LINE__, module, set_buf);
    return FALSE;
  }

  p_mod = (module_cac_t *)module->module_private;

  session_params = (cac_session_params_t *)
    module_cac_get_session_params(p_mod, sessionid);
  if (!session_params) {
    IDBG_ERROR("%s:%d: Session params are null", __func__, __LINE__);
    return FALSE;
  }

  frame_ctrl_data = (mct_pipeline_session_data_t *)set_buf;
  IDBG_MED("%s:%d] Per frame control %d %d", __func__, __LINE__,
    frame_ctrl_data->max_pipeline_frame_applying_delay,
    frame_ctrl_data->max_pipeline_meta_reporting_delay);

  session_params->session_data = *frame_ctrl_data;

  return TRUE;

}

/** module_cac_set_parent:
 *
 *  Arguments:
 *  @p_parent - parent module pointer
 *
 * Description: This function is used to set the parent pointer
 *
 * Return values:
 *     none
 *
 * Notes: none
 **/
void module_cac_set_parent(mct_module_t *p_mct_mod, mct_module_t *p_parent)
{
  module_cac_t *p_mod = NULL;

  p_mod = (module_cac_t *)p_mct_mod->module_private;
  p_mod->parent_mod = p_parent;
}

/** module_cac_init:
 *
 *  Arguments:
 *  @name - name of the module
 *
 * Description: This function is used to initialize the cac
 * module
 *
 * Return values:
 *     MCTL module instance pointer
 *
 * Notes: none
 **/
mct_module_t *module_cac_init(const char *name)
{
  mct_module_t *p_mct_mod = NULL;
  module_cac_t *p_mod = NULL;
  img_core_ops_t *p_core_ops = NULL;
  mct_port_t *p_sinkport = NULL, *p_sourceport = NULL;
  int rc = 0;
  int i = 0;
  int32_t is_cac_enabled = 0;

  IDBG_LOW("E");

  is_cac_enabled = module_imglib_common_get_prop(
    "persist.vendor.camera.feature.cac", "0");

  IDBG_HIGH("Is CAC enabled %d", is_cac_enabled);

  if (!is_cac_enabled) {
    IDBG_HIGH("CAC disabled %d, init deferred",
      is_cac_enabled);
    goto error;
  }

  p_mct_mod = mct_module_create(name);
  if (NULL == p_mct_mod) {
    IDBG_ERROR("%s:%d cannot allocate mct module", __func__, __LINE__);
    return NULL;
  }

  p_mod = malloc(sizeof(module_cac_t));
  if (NULL == p_mod) {
    IDBG_ERROR("%s:%d failed", __func__, __LINE__);
    goto error;
  }

  p_mct_mod->module_private = (void *)p_mod;
  memset(p_mod, 0, sizeof(module_cac_t));

  pthread_mutex_init(&p_mod->mutex, NULL);
  pthread_cond_init(&p_mod->cond, NULL);
  p_core_ops = &p_mod->core_ops;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  /* check if the cac module is present */
#ifdef USE_CAC_V1
  rc = img_core_get_comp(IMG_COMP_CAC, "qcom.cac1", p_core_ops);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, rc);
    goto error;
  }
#elif defined(USE_CAC_V2)
    rc = img_core_get_comp(IMG_COMP_CAC, "qcom.cac2", p_core_ops);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, rc);
      goto error;
    }
#elif defined(USE_CAC_V3)
    rc = img_core_get_comp(IMG_COMP_CAC, "qcom.cac3", p_core_ops);
    if (IMG_ERROR(rc)) {
      IDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, rc);
      goto error;
    }
#endif
 /* try to load the component */
  rc = IMG_COMP_LOAD(p_core_ops, NULL);
  if (IMG_ERROR(rc)) {
    IDBG_ERROR("%s:%d] Error rc %d", __func__, __LINE__, rc);
    goto error;
  }
  p_mod->lib_ref_count++;
  p_mod->cac_client = NULL;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  /* create static ports */
  for (i = 0; i < MAX_CAC_STATIC_PORTS; i++) {
    p_sinkport = module_cac_create_port(p_mct_mod, MCT_PORT_SINK);
    if (NULL == p_sinkport) {
      IDBG_ERROR("%s:%d] create SINK port failed", __func__, __LINE__);
      goto error;
    }
    p_sourceport = module_cac_create_port(p_mct_mod, MCT_PORT_SRC);
    if (NULL == p_sourceport) {
      IDBG_ERROR("%s:%d] create SINK port failed", __func__, __LINE__);
      goto error;
    }
  }

  p_mct_mod->set_mod          = module_cac_set_mod;
  p_mct_mod->start_session    = module_cac_start_session;
  p_mct_mod->stop_session     = module_cac_stop_session;
  p_mct_mod->set_session_data = module_cac_set_session_data;
  IDBG_LOW("X %p", p_mct_mod);
  return p_mct_mod;

error:
  if (p_mod) {
    module_cac_deinit(p_mct_mod);
  } else if (p_mct_mod) {
    mct_module_destroy(p_mct_mod);
    p_mct_mod = NULL;
  }
  IDBG_LOW("X %p", p_mct_mod);
  return NULL;

}
