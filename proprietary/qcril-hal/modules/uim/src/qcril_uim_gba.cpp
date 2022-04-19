/*===========================================================================

  Copyright (c) 2015 - 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "framework/Log.h"
#include "modules/uim/qcril_uim_srvc.h"
#include "qcril_uim.h"
#include "qcril_uim_util.h"
#include "qcril_uim_gba.h"

#define QCRIL_UIM_GBA_INIT_SEC_PROTOCOL_LEN      5

#define TAG "UIM_GBA"

/*===========================================================================

                               INTERNAL FUNCTIONS

===========================================================================*/

/*===========================================================================

  FUNCTION:  qcril_uim_gba_get_non_prov_session_type

 ==========================================================================*/
/*!
     @brief
      Fetches the sesstion enum type for the passed slot index.

     @return
      Session type enum
                                                                           */
/*=========================================================================*/
static qmi_uim_session_type qcril_uim_gba_get_non_prov_session_type
(
  uint8  slot_id
)
{
  qmi_uim_session_type session_type = QMI_UIM_SESSION_TYPE_NON_PROV_SLOT_1;

  switch (slot_id)
  {
    case 0:
      session_type = QMI_UIM_SESSION_TYPE_NON_PROV_SLOT_1;
      break;
    case 1:
      session_type = QMI_UIM_SESSION_TYPE_NON_PROV_SLOT_2;
      break;
    case 2:
      session_type = QMI_UIM_SESSION_TYPE_NON_PROV_SLOT_3;
      break;
    default:
      QCRIL_LOG_ERROR("Unsupported slot_id: 0x%x", slot_id);
      break;
  }

  QCRIL_LOG_INFO("session_type found: 0x%x", session_type);
  return session_type;
} /* qcril_uim_gba_get_non_prov_session_type */


/*===========================================================================

  FUNCTION:  qcril_uim_gba_get_gw_session_type_from_slot

 ==========================================================================*/
/*!
     @brief
      This is used to obtain GW session type corresponding to the slot ID passed in.
      This function is used to obtain only primary and secondary sessions.

     @return
      Associated Session Type.
      It is valid only when either it is primary or secondary GW session
                                                                           */
/*=========================================================================*/
static qmi_uim_session_type qcril_uim_gba_get_gw_session_type_from_slot
(
  uint8  slot_id
)
{
  unsigned short         session_index  = 0xFFFF;
  qmi_uim_session_type   session_type   = QMI_UIM_SESSION_TYPE_MAX;

  session_index = qcril_uim.card_status.index_gw_pri_prov;

  if (((session_index >> 8) & 0xFF) == slot_id)
  {
    session_type = QMI_UIM_SESSION_TYPE_PRI_GW_PROV;
  }
  else
  {
    session_index = qcril_uim.card_status.index_gw_sec_prov;

    if (((session_index >> 8) & 0xFF) == slot_id)
    {
      session_type = QMI_UIM_SESSION_TYPE_SEC_GW_PROV;
    }
  }

  return session_type;
} /* qcril_uim_gba_get_gw_session_type_from_slot */


/*=========================================================================

  FUNCTION:  qcril_uim_gba_get_impi_resp

===========================================================================*/
/*!
    @brief
    Handles the response of GBA get IMPI request.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_gba_get_impi_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
    const std::shared_ptr<UimGBAGetIMPIRequestMsg> req_ptr
)
{
  RIL_UIM_Errno  ril_err  = RIL_UIM_E_SUCCESS;
  auto           rsp_ptr  = std::make_shared<RIL_UIM_GBA_IMPI_Response>();

  if(params_ptr == NULL || req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("NULL params_ptr");
    return;
  }

  ril_err = qcril_uim_convert_err_value(params_ptr->qmi_rsp_data.qmi_err_code);

  QCRIL_LOG_DEBUG("qcril_uim_oem_get_impi_resp, qmi_err_code: 0x%x",
                  params_ptr->qmi_rsp_data.qmi_err_code);

  /* If successful update the response with encrypted IMSI data */
  if (rsp_ptr != NULL)
  {
    rsp_ptr->is_impi_encrypted        = params_ptr->qmi_rsp_data.rsp_data.gba_impi_rsp.is_encrypted;

    const qmi_uim_data_type *impi_ptr = &params_ptr->qmi_rsp_data.rsp_data.gba_impi_rsp.impi;

    if ((impi_ptr != NULL) &&
        (impi_ptr->data_len > 0) &&
        (impi_ptr->data_ptr != NULL))
    {
      rsp_ptr->impi.resize(impi_ptr->data_len);
      memcpy(&rsp_ptr->impi[0], impi_ptr->data_ptr, impi_ptr->data_len);
    }
    rsp_ptr->err = ril_err;
  }
  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_gba_get_impi_resp */


/*===========================================================================

  FUNCTION:  qcril_uim_gba_resp

===========================================================================*/
/*!
    @brief
    Handles the response of GBA initialization request.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_gba_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimGBAInitRequestMsg> req_ptr
)
{
  RIL_UIM_Errno   ril_err = RIL_UIM_E_SUCCESS;
  auto            rsp_ptr = std::make_shared<RIL_UIM_GBA_InitResponse>();

  /* Sanity check */
  if(params_ptr == NULL || req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("NULL params_ptr");
    return;
  }

  ril_err = qcril_uim_convert_err_value(params_ptr->qmi_rsp_data.qmi_err_code);

  QCRIL_LOG_DEBUG("qmi_err_code: 0x%x",
                   params_ptr->qmi_rsp_data.qmi_err_code);

  if (rsp_ptr != NULL)
  {
    /* Check for appropriate NAF response types & update */
    rsp_ptr->naf_type = (params_ptr->qmi_rsp_data.rsp_data.gba_rsp.is_ks_naf_encrypted == TRUE) ?
                          RIL_UIM_GBA_KS_NAF_TYPE_ENCRYPTED : RIL_UIM_GBA_KS_NAF_TYPE_PLAIN;

    if ((params_ptr->qmi_rsp_data.rsp_data.gba_rsp.ks_naf.data_ptr != NULL) &&
        (params_ptr->qmi_rsp_data.rsp_data.gba_rsp.ks_naf.data_len > 0))
    {
      uint8_t  * naf_response = NULL;
      uint32_t   len          = 0;

      /* If plaintext, convert to base64 format */
      if (rsp_ptr->naf_type == RIL_UIM_GBA_KS_NAF_TYPE_PLAIN)
      {

        naf_response = (uint8_t *)qcril_uim_alloc_bin_to_base64string(
                              params_ptr->qmi_rsp_data.rsp_data.gba_rsp.ks_naf.data_ptr,
                              params_ptr->qmi_rsp_data.rsp_data.gba_rsp.ks_naf.data_len);
        len = (naf_response == NULL) ? 0 : strlen((const char *)naf_response);
      }
      /* else send data as is */
      else
      {
        len  = params_ptr->qmi_rsp_data.rsp_data.gba_rsp.ks_naf.data_len;
        naf_response = params_ptr->qmi_rsp_data.rsp_data.gba_rsp.ks_naf.data_ptr;
      }

      if (naf_response != NULL && len != 0)
      {
        rsp_ptr->naf_response.resize(len);
        memcpy(&rsp_ptr->naf_response[0], naf_response, len);

        if (rsp_ptr->naf_type == RIL_UIM_GBA_KS_NAF_TYPE_PLAIN)
        {
          qcril_free(naf_response);
          naf_response = NULL;
        }
      }
    }

    /* Update the B-TId */
    if ((params_ptr->qmi_rsp_data.rsp_data.gba_rsp.b_tid.data_ptr != NULL) &&
        (params_ptr->qmi_rsp_data.rsp_data.gba_rsp.b_tid.data_len > 0))
    {
      rsp_ptr->bootstrap_tid.assign((char *)params_ptr->qmi_rsp_data.rsp_data.gba_rsp.b_tid.data_ptr,
                                    (size_t)params_ptr->qmi_rsp_data.rsp_data.gba_rsp.b_tid.data_len);
    }

    QCRIL_LOG_INFO("B-TID: %s", rsp_ptr->bootstrap_tid.c_str());

    /* Update the Key Lifetime */
    if ((params_ptr->qmi_rsp_data.rsp_data.gba_rsp.key_lifetime.data_ptr != NULL) &&
        (params_ptr->qmi_rsp_data.rsp_data.gba_rsp.key_lifetime.data_len > 0))
    {
      rsp_ptr->life_time.assign((char *)params_ptr->qmi_rsp_data.rsp_data.gba_rsp.key_lifetime.data_ptr,
                                (size_t)params_ptr->qmi_rsp_data.rsp_data.gba_rsp.key_lifetime.data_len);
    }

    QCRIL_LOG_INFO("life_time: %s", rsp_ptr->life_time.c_str());

    rsp_ptr->err = ril_err;
  }
  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_gba_resp */


/*===========================================================================

                               QCRIL INTERFACE FUNCTIONS

===========================================================================*/

/*===========================================================================

  FUNCTION:  qcril_uim_gba_request_get_impi

===========================================================================*/
/*!
    @brief
    Handles MessageId_RIL_OEM_IMPI request from the client. This is sent
    from RIL via QCRIL_EVT_OEM_MSG_GET_IMPI.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_gba_request_get_impi
(
  const std::shared_ptr<UimGBAGetIMPIRequestMsg> req_ptr
)
{
  qmi_uim_gba_impi_params_type    gba_impi_params = {};
  RIL_UIM_Errno                   err          = RIL_UIM_E_INTERNAL_ERR;
  qmi_uim_session_type            session_type = QMI_UIM_SESSION_TYPE_PRI_GW_PROV;
  RIL_UIM_GBA_IMPI_Params       * in_ptr       = NULL;
  uint8_t                         app_index    = QMI_UIM_MAX_APP_PER_CARD_COUNT;
  std::shared_ptr<RIL_UIM_GBA_IMPI_Response> rsp_ptr = NULL;

  /* Sanity check */
  if(req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot process response");
    return;
  }

  in_ptr = &req_ptr->get_params();
  /* Parse input info */
  QCRIL_LOG_INFO("secure: %d, slotId:0x%x, applicationType: 0x%x",
                 in_ptr->secure,
                 in_ptr->slot_id,
                 in_ptr->app_type);

  if (in_ptr->slot_id >= QMI_UIM_MAX_CARD_COUNT)
  {
    QCRIL_LOG_ERROR("Invalid slot_id: 0x%x", in_ptr->slot_id);
    err = RIL_UIM_E_INVALID_ARGUMENTS;
    goto report_error;
  }

  /* Fetch session info */
  switch (in_ptr->app_type)
  {
    case RIL_UIM_APPTYPE_USIM:
      session_type = qcril_uim_gba_get_gw_session_type_from_slot(in_ptr->slot_id);

      if(session_type != QMI_UIM_SESSION_TYPE_PRI_GW_PROV &&
         session_type != QMI_UIM_SESSION_TYPE_SEC_GW_PROV)
      {
        QCRIL_LOG_ERROR("%s","Invalid session type");
        goto report_error;
      }
      gba_impi_params.session_info.session_type = session_type;
      break;

    case RIL_UIM_APPTYPE_ISIM:
      err = qcril_uim_extract_isim_index(&app_index, in_ptr->slot_id);
      if ((err != RIL_UIM_E_SUCCESS) || (app_index >= QMI_UIM_MAX_APP_PER_CARD_COUNT))
      {
        QCRIL_LOG_ERROR("ISIM app not found in slot_id: 0x%x", in_ptr->slot_id);
        goto report_error;
      }
      /* Copy AID */
      gba_impi_params.session_info.aid.data_len = qcril_uim.card_status.card[in_ptr->slot_id].application[app_index].aid_len;
      gba_impi_params.session_info.aid.data_ptr = (unsigned char *)qcril_uim.card_status.card[in_ptr->slot_id].application[app_index].aid_value;
      gba_impi_params.session_info.session_type = qcril_uim_gba_get_non_prov_session_type(in_ptr->slot_id);
      break;

    default:
      QCRIL_LOG_ERROR("Unsupported app type: 0x%x", in_ptr->app_type);
      err = RIL_UIM_E_INVALID_ARGUMENTS;
      goto report_error;
  }

  QCRIL_LOG_INFO("session_type: %d\n", gba_impi_params.session_info.session_type);

  /* Also set encryption flag if client needs it */
  if (in_ptr->secure)
  {
    gba_impi_params.encrypt_data = QMI_UIM_TRUE;
  }

  /* Proceed with read transparent  */
  if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_GBA_IMPI,
                                   &gba_impi_params,
                                   req_ptr) >= 0)
  {
    return;
  }
  else
  {
    err = RIL_UIM_E_MODEM_ERR;
  }

report_error:
  rsp_ptr = std::shared_ptr<RIL_UIM_GBA_IMPI_Response>();

  if (rsp_ptr != NULL)
  {
    rsp_ptr->err = err;
  }
  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_gba_request_get_impi */


/*===========================================================================

  FUNCTION:  qcril_uim_request_gba_init

===========================================================================*/
/*!
    @brief
    Handles MessageId_RIL_OEM_GBA_INIT request from the client. This is
    sent from RIL via QCRIL_EVT_OEM_MSG_GBA_INIT.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_gba_init
(
  const std::shared_ptr<UimGBAInitRequestMsg> req_ptr
)
{
  qmi_uim_gba_params_type              gba_init_params = {};
  RIL_UIM_GBA_InitParams             * in_ptr       = NULL;
  uint8_t                              app_index    = QCRIL_UIM_INVALID_APP_INDEX_VALUE;
  RIL_UIM_Errno                        err          = RIL_UIM_E_INTERNAL_ERR;
  qmi_uim_session_type                 session_type = QMI_UIM_SESSION_TYPE_PRI_GW_PROV;
  std::shared_ptr<RIL_UIM_GBA_InitResponse> rsp_ptr = NULL;

  /* Sanity check */
  if(req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot process response");
    return;
  }

  in_ptr = &req_ptr->get_params();

  QCRIL_LOG_INFO("slotId:0x%x, applicationType: 0x%x",
                 in_ptr->slot_id,
                 in_ptr->app_type);

  if (in_ptr->slot_id >= QMI_UIM_MAX_CARD_COUNT)
  {
    QCRIL_LOG_ERROR("Invalid slot_id: 0x%x", in_ptr->slot_id);
    err = RIL_UIM_E_INVALID_ARGUMENTS;
    goto report_error;
  }

  /* Fetch session info */
  switch (in_ptr->app_type)
  {
    case RIL_UIM_APPTYPE_USIM:
      session_type = qcril_uim_gba_get_gw_session_type_from_slot(in_ptr->slot_id);

      if(session_type != QMI_UIM_SESSION_TYPE_PRI_GW_PROV &&
         session_type != QMI_UIM_SESSION_TYPE_SEC_GW_PROV)
      {
        QCRIL_LOG_ERROR("%s","Invalid session type");
        goto report_error;
      }
      gba_init_params.session_info.session_type = session_type;
      break;
    case RIL_UIM_APPTYPE_ISIM:
      err = qcril_uim_extract_isim_index(&app_index, in_ptr->slot_id);
      if ((err != RIL_UIM_E_SUCCESS) || (app_index >= QMI_UIM_MAX_APP_PER_CARD_COUNT))
      {
        QCRIL_LOG_ERROR("ISIM app not found in slot_id: 0x%x", in_ptr->slot_id);
        goto report_error;
      }
      /* Copy AID */
      gba_init_params.session_info.aid.data_len = qcril_uim.card_status.card[in_ptr->slot_id].application[app_index].aid_len;
      gba_init_params.session_info.aid.data_ptr = (unsigned char *)qcril_uim.card_status.card[in_ptr->slot_id].application[app_index].aid_value;
      gba_init_params.session_info.session_type = qcril_uim_gba_get_non_prov_session_type(in_ptr->slot_id);
      break;
    default:
      QCRIL_LOG_ERROR("Unsupported app type: 0x%x", in_ptr->app_type);
      err = RIL_UIM_E_INVALID_ARGUMENTS;
      goto report_error;
  }

  QCRIL_LOG_INFO( "session_type: %d\n", gba_init_params.session_info.session_type );

  QCRIL_LOG_INFO("FQDN length: %d, FQDN: %s",
                 in_ptr->domain_name.length(),
                 in_ptr->domain_name.c_str());

  /* Return error if FQDN isnt sent, note that tmp_data_len == 0 means
     FQDN pointer is invalid due to the above check above */
  if (in_ptr->domain_name.empty())
  {
    QCRIL_LOG_ERROR("No FQDN in request !");
    err = RIL_UIM_E_INVALID_ARGUMENTS;
    goto report_error;
  }

  /* Add FQDN */
  gba_init_params.naf_id.fqdn.data_len = in_ptr->domain_name.length();
  gba_init_params.naf_id.fqdn.data_ptr = (uint8_t *)in_ptr->domain_name.c_str();

  /* Return error if security protocol is invalid */
  if (in_ptr->protocol.size() != QCRIL_UIM_GBA_INIT_SEC_PROTOCOL_LEN)
  {
    QCRIL_LOG_ERROR("Invalid securityProtocol in request !");
    err = RIL_UIM_E_INVALID_ARGUMENTS;
    goto report_error;
  }

  /* Add security protocol */
  memcpy(gba_init_params.naf_id.protocol_id, in_ptr->protocol.data(), in_ptr->protocol.size());

  /* Add bootstrapping info */
  gba_init_params.force_bootstrapping = in_ptr->force_bootstrapping ?
                                          QMI_UIM_TRUE : QMI_UIM_FALSE;
  QCRIL_LOG_INFO("FQDN forceBootstrapping: %s",
                 in_ptr->force_bootstrapping ? "TRUE" : "FALSE");

  if(!in_ptr->apn.empty())
  {
    QCRIL_LOG_INFO("APN passed is %s", in_ptr->apn.c_str());
    gba_init_params.apn.data_len = in_ptr->apn.length();
    gba_init_params.apn.data_ptr = (uint8_t *) in_ptr->apn.c_str();
  }

  /* Proceed with the GBA request */
  if (qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_GBA,
                                   &gba_init_params,
                                   req_ptr) >= 0)
  {
    return;
  }
  else
  {
    err = RIL_UIM_E_MODEM_ERR;
  }

report_error:
  rsp_ptr = std::shared_ptr<RIL_UIM_GBA_InitResponse>();

  if (rsp_ptr != NULL)
  {
    rsp_ptr->err = err;
  }
  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_request_gba_init */
