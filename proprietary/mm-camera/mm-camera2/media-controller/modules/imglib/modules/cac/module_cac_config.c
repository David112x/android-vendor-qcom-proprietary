/**********************************************************************
* Copyright (c) 2013-2016 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include <linux/media.h>
#include "mct_module.h"
#include "module_cac.h"
#include "mct_stream.h"
#include "chromatix.h"
#include "chromatix_swpostproc.h"

/**
 * Function: module_cac_update_offline_params
 *
 * Description: This function is to update the CAC parameters
 * for offline usecase for cac.
 *
 * Arguments:
 *   @p_client: cac client
 *   @pframe: frame pointer
 *   @native_buf: flag to indicate if its a native buffer
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
static int module_cac_config_update_offline_params(cac_client_t *p_client)
{
  int status = IMG_SUCCESS;
  metadata_buffer_t *metadata_buff;
  mct_stream_session_metadata_info *session_meta;
  int str_idx = 0;
  uint32_t session_id;
  module_cac_t *p_mod;
  cac_session_params_t *session_params = NULL;
  int i = 0;

  IDBG_MED("%s:%d] ", __func__, __LINE__);
  str_idx = module_cac_get_stream_by_id(p_client,
    p_client->event_identity);
  if (str_idx < 0) {
    IDBG_ERROR("%s %d: Cannot find stream with identity 0x%x mapped to the"
      "client", __func__, __LINE__, p_client->p_buf_divert_data->identity);
    return IMG_ERR_GENERAL;
  }

  meta_data_container *meta_data_container =
    p_client->p_buf_divert_data->meta_data;
  metadata_buff = (metadata_buffer_t *)meta_data_container->mct_meta_data;

  if (!metadata_buff) {
    IDBG_ERROR("%s:%d] Invalid metadata buffer", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  /*Face detection results*/
  IF_META_AVAILABLE(cam_face_detection_data_t, fd_result,
      CAM_INTF_META_FACE_DETECTION, metadata_buff) {
    IDBG_MED("%s %d: Face detction result num of face %d", __func__, __LINE__,
      fd_result->num_faces_detected);
     p_client->cac_cfg_info.fd_data.num_faces_detected = fd_result->num_faces_detected;
     for (i = 0; i < fd_result->num_faces_detected; i++) {
       p_client->cac_cfg_info.fd_data.faces[i].x = fd_result->faces[i].face_boundary.left;
       p_client->cac_cfg_info.fd_data.faces[i].y = fd_result->faces[i].face_boundary.top;
       p_client->cac_cfg_info.fd_data.faces[i].dx= fd_result->faces[i].face_boundary.width;
       p_client->cac_cfg_info.fd_data.faces[i].dy= fd_result->faces[i].face_boundary.height;
     }
  }

  /*Get LDS */
  IF_META_AVAILABLE(cam_img_hysterisis_info_t, hysterisis_result,
      CAM_INTF_META_IMG_HYST_INFO, metadata_buff) {
    p_client->cac_cfg_info.lds_enable_flag = hysterisis_result->lds_enabled;
  } else {
    IDBG_MED("%s %d: hyst metadata invalid", __func__, __LINE__);
    p_client->cac_cfg_info.lds_enable_flag = 0;
  }

  /*Todo: Update LDS flag in buf divert*/
  //p_client->p_buf_divert_data->lds_enabled = p_client->cac_cfg_info.lds_enable_flag;
  IDBG_MED("%s %d: From metatdata lds %d", __func__, __LINE__,
    p_client->cac_cfg_info.lds_enable_flag);

  session_meta = (mct_stream_session_metadata_info *)
    POINTER_OF_META(CAM_INTF_META_PRIVATE_DATA, metadata_buff);
  if (!(session_meta)) {
    IDBG_ERROR("%s:%d] session_meta pointer", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }


  //Get Data From Session params
  p_mod = (module_cac_t *)p_client->p_mod;
  session_id = IMGLIB_SESSIONID(p_client->stream[str_idx].identity);
  session_params = (cac_session_params_t *)
    module_cac_get_session_params(p_mod, session_id);
  if (!session_params) {
    IDBG_ERROR("%s:%d: Session params are null", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }
  p_client->cac_cfg_info.img_3A_info.lux_idx =
    session_params->hyst_info.lux_idx;
  p_client->cac_cfg_info.img_3A_info.prev_lux_value =
    session_params->hyst_info.prev_lux_value;
  p_client->cac_cfg_info.img_3A_info.gain = session_params->hyst_info.gain;
  p_client->cac_cfg_info.img_3A_info.prev_gain_value =
    session_params->hyst_info.prev_gain_value;
  p_client->cac_cfg_info.img_3A_info.s_rnr_enabled =
    session_params->hyst_info.s_rnr_enabled;
  IDBG_HIGH("%s %d:lux_idx %f, gain %f  prev_lux %f prev_gain %f "
    "s_rnr_enabled= %d ",
    __func__, __LINE__,
    p_client->cac_cfg_info.img_3A_info.lux_idx,
    p_client->cac_cfg_info.img_3A_info.gain,
    p_client->cac_cfg_info.img_3A_info.prev_lux_value,
    p_client->cac_cfg_info.img_3A_info.prev_gain_value,
    p_client->cac_cfg_info.img_3A_info.s_rnr_enabled );

  return status;
}

/**
 * Function: module_cac_config_get_params
 *
 * Description: Get CAC config in non zsl mode
 *
 * Arguments:
 *   @p_client: cac client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
static int module_cac_config_get_params(cac_client_t *p_client)
{
  mct_event_t get_gtbl_event;
  mct_isp_table_t gammatbl;
  uint32_t session_id;
  int str_idx = 0;
  int rc = IMG_SUCCESS;
  module_cac_t *p_mod;
  cac_session_params_t *session_params;

  str_idx = module_cac_get_stream_by_id(p_client,
    p_client->event_identity);
  if (str_idx < 0) {
    IDBG_ERROR("%s %d: Cannot find stream with identity 0x%x mapped to the"
      "client", __func__, __LINE__, p_client->event_identity);
    return IMG_ERR_GENERAL;
  }

  memset(&get_gtbl_event, 0x0, sizeof(mct_event_t));
  memset(&gammatbl, 0x0, sizeof(mct_isp_table_t));

  gammatbl.gamma_num_entries = GAMMA_TABLE_ENTRIES;
  gammatbl.gamma_table = &p_client->cac_cfg_info.r_gamma.table[0];
  get_gtbl_event.type = MCT_EVENT_MODULE_EVENT;
  get_gtbl_event.identity = p_client->event_identity;
  get_gtbl_event.direction = MCT_EVENT_UPSTREAM;
  get_gtbl_event.u.module_event.type = MCT_EVENT_MODULE_GET_ISP_TABLES;
  get_gtbl_event.u.module_event.module_event_data = &gammatbl;

  rc =  mct_port_send_event_to_peer(p_client->stream[str_idx].p_sinkport,
    &get_gtbl_event);
  if (!rc) {
    IDBG_ERROR("%s:%d] MCT_EVENT_MODULE_GET_ISP_TABLES failed : rc = %d",
      __func__, __LINE__, rc);
    return IMG_ERR_GENERAL;
  }
  rc = IMG_SUCCESS;
  IDBG_MED("%s:%d] MCT_EVENT_MODULE_GET_ISP_TABLES Successful", __func__,
    __LINE__);
  p_client->cac_cfg_info.b_gamma = p_client->cac_cfg_info.r_gamma;
  p_client->cac_cfg_info.g_gamma = p_client->cac_cfg_info.r_gamma;

  //Get Data From Session params
  p_mod = (module_cac_t *)p_client->p_mod;
  session_id = IMGLIB_SESSIONID(p_client->stream[str_idx].identity);
  session_params = (cac_session_params_t *)
    module_cac_get_session_params(p_mod, session_id);
  if (!session_params) {
    IDBG_ERROR("%s:%d: Session params are null", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }
  p_client->cac_cfg_info.chromatix_ptr =
    session_params->chromatix;
  p_client->cac_cfg_info.cac_3a_data.awb_gb_gain = 127;
  p_client->cac_cfg_info.cac_3a_data.awb_gr_gain = 127;

#ifdef DEBUG_GAMMA_TBL
  int i = 0;
  for (i = 0; i < GAMMA_TABLE_ENTRIES; i++) {
    IDBG_INFO("gamma[%d] %d", i, p_client->cac_cfg_info.g_gamma.table[i]);
  }
#endif
  return rc;
}

/**
 * Function: module_cac_config_client
 *
 * Description: This function configures the cac component
 *
 * Arguments:
 *   @p_client: cac client
 *
 * Return values:
 *     imaging error values
 *
 * Notes: none
 **/
int module_cac_config_client(cac_client_t *p_client)
{
  int rc = IMG_SUCCESS;
  img_component_ops_t *p_comp = &p_client->comp;
  int str_idx = 0;

  str_idx = module_cac_get_stream_by_id(p_client,
    p_client->event_identity);
  if (str_idx < 0) {
    IDBG_ERROR("%s %d: Cannot find stream with identity 0x%x mapped to the"
      "client", __func__, __LINE__, p_client->event_identity);
    return IMG_ERR_GENERAL;
  }

  IDBG_MED("%s:%d] type %d", __func__, __LINE__,
    p_client->stream[str_idx].stream_info->stream_type);

  if (CAM_STREAM_TYPE_OFFLINE_PROC ==
    p_client->stream[str_idx].stream_info->stream_type) {
    rc = module_cac_config_update_offline_params(p_client);
  } else {
    rc = module_cac_config_get_params(p_client);
  }

  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s %d: Error configuring cac params", __func__, __LINE__);
    return IMG_ERR_GENERAL;
  }

  //Set the component to be executed in syncronous mode
  p_client->mode = IMG_SYNC_MODE;

  rc = IMG_COMP_SET_PARAM(p_comp, QCAC_CHROMATIX_INFO,
    (void *)(p_client->cac_cfg_info.chromatix_ptr));
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] QCAC_CHROMATIX_INFO rc %d", __func__, __LINE__, rc);
    return rc;
  }
  rc = IMG_COMP_SET_PARAM(p_comp, QCAC_RGAMMA_TABLE,
    (void*)&(p_client->cac_cfg_info.r_gamma));
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] Cannot set rGamma tables rc %d", __func__, __LINE__, rc);
    return rc;
  }
  rc = IMG_COMP_SET_PARAM(p_comp, QCAC_GGAMMA_TABLE,
    (void*)&(p_client->cac_cfg_info.g_gamma));
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] Cannot set gGamma tables rc %d", __func__, __LINE__, rc);
    return rc;
  }
  rc = IMG_COMP_SET_PARAM(p_comp, QCAC_BGAMMA_TABLE,
    (void*)&(p_client->cac_cfg_info.b_gamma));
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] Cannot set bGamma tables rc %d", __func__, __LINE__, rc);
    return rc;
  }
#ifdef USE_CAC_V3
  IDBG_MED("%s:%d] sending QCAC_3A_INFO", __func__, __LINE__);
  rc = IMG_COMP_SET_PARAM(p_comp, QCAC_3A_INFO,
    (void *)&(p_client->cac_cfg_info.img_3A_info));
#else
  IDBG_MED("%s:%d] QCAC_3A_INFO 3a data", __func__, __LINE__);
  rc = IMG_COMP_SET_PARAM(p_comp, QCAC_3A_INFO,
    (void *)&(p_client->cac_cfg_info.cac_3a_data));
#endif
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] Cannot set QCAC_3A_INFO rc %d", __func__, __LINE__, rc);
    return rc;
  }
  rc = IMG_COMP_SET_PARAM(p_comp, QCDS_ENABLED,
    (void *)&(p_client->cac_cfg_info.cds_enable_flag));
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] QCDS_ENABLED rc %d", __func__, __LINE__, rc);
    return rc;
  }

  rc = IMG_COMP_SET_PARAM(p_comp, QCDS_PROCESS,
    (void *)&(p_client->use_sw_cds));
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] QCDS_PROCESS rc %d", __func__, __LINE__, rc);
  }

  rc = IMG_COMP_SET_PARAM(p_comp, QIMG_PARAM_MODE,
    (void*)&(p_client->mode));
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] QIMG_PARAM_MODE rc %d", __func__, __LINE__, rc);
    return rc;
  }
  p_client->cac_cfg_info.chroma_order = CAC_CHROMA_ORDER_CRCB;
  rc = IMG_COMP_SET_PARAM(p_comp, QCAC_CHROMA_ORDER,
    (void *)&(p_client->cac_cfg_info.chroma_order));
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] Cannot set Chroma Order rc %d", __func__, __LINE__, rc);
    return rc;
  }
  rc = IMG_COMP_SET_PARAM(p_comp, QFD_RESULT,
    (void *)&(p_client->cac_cfg_info.fd_data));
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] Cannot set QFD_RESULT rc %d", __func__, __LINE__, rc);
    return rc;
  }
  rc = IMG_COMP_SET_PARAM(p_comp, QIMG_CAMERA_DUMP,
    (void *)&p_client->debug_meta_info);
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s %d] QIMG_CAMERA_DUMP rc %d", __func__, __LINE__, rc);
    return rc;
  }
  rc = IMG_COMP_SET_PARAM(p_comp, QLDS_ENABLED,
    (void *)&(p_client->cac_cfg_info.lds_enable_flag));
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] QLDS_ENABLED rc %d", __func__, __LINE__, rc);
    return rc;
  }
  rc = IMG_COMP_SET_PARAM(p_comp, QYUYV422_TO_420_ENABLED,
    (void *)&(p_client->cac_cfg_info.yuyv422_to_420_enabled));
  if (rc != IMG_SUCCESS) {
    IDBG_ERROR("%s:%d] QYUYV422_TO_420_ENABLED rc %d", __func__, __LINE__, rc);
    return rc;
  }
  IDBG_MED("%s:%d] ", __func__, __LINE__);
  return rc;
}
