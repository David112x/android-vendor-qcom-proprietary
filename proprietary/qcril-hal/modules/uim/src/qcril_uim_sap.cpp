/*===========================================================================

  Copyright (c) 2014-2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

/*===========================================================================

                           INCLUDE FILES

===========================================================================*/

#include "framework/Log.h"
#include "modules/uim/qcril_uim_srvc.h"
#include "modules/uim/UimSapStatusIndMsg.h"
#include "qcril_uim.h"
#include "qcril_uim_sap.h"
#include "qcril_uim_util.h"

#define TAG "QCRIL_UIM_SAP"

static RIL_UIM_SAP_CardState   qcril_uim_sap_sim_state = RIL_UIM_SAP_CARD_STATE_NOT_ACCESSSIBLE;

/*===========================================================================

                               INTERNAL FUNCTIONS

===========================================================================*/

/*===========================================================================

                               QCRIL INTERFACE FUNCTIONS

===========================================================================*/
/*===========================================================================

FUNCTION:  qcril_uim_sap_connection_request

===========================================================================*/
/*!
   @brief
   This function sends SAP connection request

   @return
   int.

*/
/*=========================================================================*/
void qcril_uim_sap_connection_request
(
  const std::shared_ptr<UimSAPConnectionRequestMsg> req_ptr
)
{
  int                                   qmi_result         = 0;
  qmi_uim_sap_connection_params_type    sap_request_params = {};
  std::shared_ptr<RIL_UIM_SAP_ConnectionResponse> rsp_ptr  = NULL;

  if (req_ptr == NULL)
  {
    return;
  }

  /* Update QMI parameters from protobuf request & dispatch it to modem */
  sap_request_params.slot           = (qmi_uim_slot_type)((uint8_t) QMI_UIM_SLOT_1 + qmi_ril_get_process_instance_id());
  sap_request_params.conn_condition = QMI_UIM_SAP_CONN_COND_BLOCK_VOICE;

  if (req_ptr->get_req_type() == RIL_UIM_SAP_CONNECT)
  {
    sap_request_params.operation_type = QMI_UIM_SAP_CONNECTION_CONNECT;
  }
  else
  {
    /* Note - for disconnect req, there is no mode passed from client */
    sap_request_params.operation_type  = QMI_UIM_SAP_CONNECTION_DISCONNECT;
    sap_request_params.disconnect_mode = QMI_UIM_SAP_DISCONNECT_MODE_GRACEFUL;
  }

  /* Dispatch request to modem. In case of error, clean up */
  qmi_result = qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_SAP_CONNECTION,
                                            &sap_request_params,
                                            req_ptr);

  if (qmi_result >= 0)
  {
    return;
  }

  QCRIL_LOG_ERROR("Failed to send qmi request: %d", qmi_result);
  rsp_ptr = std::make_shared<RIL_UIM_SAP_ConnectionResponse>(RIL_UIM_SAP_CONNECTION_FAILURE);
  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_sap_connection_request */


/*===========================================================================

  FUNCTION:  qcril_uim_sap_send_request

===========================================================================*/
/*!
    @brief
    This function sends SAP request to qcril_uim_qcci

    @return
    int.

*/
/*=========================================================================*/
void qcril_uim_sap_send_request
(
  const std::shared_ptr<UimSAPRequestMsg>  req_ptr
)
{
  int                                 qmi_result         = 0;
  qmi_uim_sap_request_params_type     sap_request_params = {};
  RIL_UIM_SAP_ResultCode              result             = RIL_UIM_SAP_GENERIC_FAILURE;
  std::shared_ptr<RIL_UIM_SAP_Response> rsp_ptr          = NULL;
  RIL_UIM_SAP_RequestType             req_id             = RIL_UIM_SAP_APDU_REQ;
  uint8_t                             slot               = qmi_ril_get_process_instance_id();

  if (req_ptr == NULL)
  {
    return;
  }

  req_id = req_ptr->get_req_type();

  if (slot >= QMI_UIM_MAX_CARD_COUNT)
  {
    goto send_response;
  }

  if(qcril_uim_sap_sim_state == RIL_UIM_SAP_CARD_STATE_INSERTED)
  {
    if (req_id == RIL_UIM_SAP_POWER_REQ &&
        req_ptr->get_power_state())
    {
      result = RIL_UIM_SAP_CARD_ALREADY_POWERED_ON;
      goto send_response;
    }
  }
  else if (qcril_uim_sap_sim_state == RIL_UIM_SAP_CARD_STATE_POWER_DOWN)
  {
    if (req_id == RIL_UIM_SAP_POWER_REQ &&
        !req_ptr->get_power_state())
    {
      result = RIL_UIM_SAP_CARD_ALREADY_POWERED_OFF;
      goto send_response;
    }
  }
  else if (qcril_uim_sap_sim_state == RIL_UIM_SAP_CARD_STATE_REMOVED)
  {
    result = RIL_UIM_SAP_CARD_REMOVED;
    goto send_response;
  }
  else
  {
    result = RIL_UIM_SAP_CARD_NOT_ACCESSSIBLE;
    goto send_response;
  }

  sap_request_params.slot = (qmi_uim_slot_type)((uint8_t)QMI_UIM_SLOT_1 + slot);

  switch (req_id)
  {
    case RIL_UIM_SAP_APDU_REQ:
      sap_request_params.request_type = QMI_UIM_SAP_REQUEST_OP_SEND_APDU;
      sap_request_params.apdu.data_len = req_ptr->get_apdu().size();
      sap_request_params.apdu.data_ptr = (uint8_t *)req_ptr->get_apdu().data();
      break;

    case RIL_UIM_SAP_POWER_REQ:
      if (req_ptr->get_power_state())
      {
        sap_request_params.request_type = QMI_UIM_SAP_REQUEST_OP_POWER_SIM_ON;
      }
      else
      {
        sap_request_params.request_type = QMI_UIM_SAP_REQUEST_OP_POWER_SIM_OFF;
      }
      break;

    case RIL_UIM_SAP_RESET_SIM_REQ:
      sap_request_params.request_type = QMI_UIM_SAP_REQUEST_OP_RESET_SIM;
      break;

    case RIL_UIM_SAP_TRANSFER_ATR_REQ:
      sap_request_params.request_type = QMI_UIM_SAP_REQUEST_OP_GET_ATR;
      break;

    case RIL_UIM_SAP_TRANSFER_CARD_READER_STATUS_REQ:
      sap_request_params.request_type = QMI_UIM_SAP_REQUEST_OP_READER_STATUS;
      break;

    default:
      QCRIL_LOG_ERROR( "Unspported message_id: 0x%x\n", req_id);
      goto send_response;
  }

  if (req_id == RIL_UIM_SAP_POWER_REQ)
  {
    int                    qmi_err_code = 0;
    qmi_uim_rsp_data_type  rsp_data     = {};

    qmi_err_code = qcril_uim_send_qmi_sync_msg(QCRIL_UIM_REQUEST_SAP_REQUEST,
                                               &sap_request_params,
                                               &rsp_data);

    if (qmi_err_code == 0 && rsp_data.qmi_err_code == 0)
    {
       result = RIL_UIM_SAP_SUCCESS;

       if (req_ptr->get_power_state())
       {
         qcril_uim_sap_sim_state = RIL_UIM_SAP_CARD_STATE_INSERTED;
       }
       else
       {
         qcril_uim_sap_sim_state = RIL_UIM_SAP_CARD_STATE_POWER_DOWN;
       }
    }
    else
    {
      QCRIL_LOG_ERROR("Sap Power request failed, qmi_err_code: 0x%x, result_code: 0x%x\n",
                      qmi_err_code, rsp_data.qmi_err_code);
    }
  }
  else
  {
    /* Dispatch request to modem. In case of error, clean up */
    qmi_result = qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_SAP_REQUEST,
                                              &sap_request_params,
                                              req_ptr);
    if (qmi_result >= 0)
    {
      return;
    }

    QCRIL_LOG_ERROR("SAP request failed, qmi_result: 0x%x", qmi_result);
  }

send_response:
  rsp_ptr = std::make_shared<RIL_UIM_SAP_Response>();

  if(rsp_ptr != NULL)
  {
    rsp_ptr->result = result;
  }
  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_sap_send_request */


/*=========================================================================

  FUNCTION:  qcril_uim_sap_qmi_handle_sap_connection_resp

===========================================================================*/
/*!
    @brief
    Handles the SAP connection request callback. Based on the response
    received from the modem, respective packed response types are constructed
    and the onRequestComplete is called. This completes the original request
    called by SAP client.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_sap_qmi_handle_sap_connection_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimSAPConnectionRequestMsg> req_ptr
)
{
  std::shared_ptr<RIL_UIM_SAP_ConnectionResponse> rsp_ptr  = NULL;

  /* Sanity check */
  if(params_ptr == NULL || req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("NULL params_ptr");
    return;
  }

  QCRIL_LOG_DEBUG("handle_sap_request_resp, qmi_err_code: 0x%x",
                  params_ptr->qmi_rsp_data.qmi_err_code);

  rsp_ptr = std::make_shared<RIL_UIM_SAP_ConnectionResponse>(
        (params_ptr->qmi_rsp_data.qmi_err_code == 0) ?
                 RIL_UIM_SAP_CONNECTION_SUCCESS : RIL_UIM_SAP_CONNECTION_FAILURE);
  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);

  /* Now send UNSOL response if needed */
  if ((params_ptr->qmi_rsp_data.qmi_err_code == 0) &&
      (qcril_uim_sap_sim_state == RIL_UIM_SAP_CARD_STATE_INSERTED))
  {
    auto ind_ptr = std::make_shared<UimSapStatusIndMsg>(RIL_UIM_SAP_STATUS_CARD_RESET);

    if (ind_ptr != NULL)
    {
      ind_ptr->broadcast();
    }
  }
} /* qcril_uim_sap_qmi_handle_sap_connection_resp */


/*=========================================================================

  FUNCTION:  qcril_uim_sap_qmi_handle_sap_request_resp

===========================================================================*/
/*!
    @brief
    Handles the SAP request callback. Based on the response
    received from the modem, respective packed response types are constructed
    and the onRequestComplete is called. This completes the original request
    called on the RIL SAP socket.

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_sap_qmi_handle_sap_request_resp
(
  const qcril_uim_callback_params_type * const params_ptr,
  const std::shared_ptr<UimSAPRequestMsg>  req_ptr
)
{
  std::shared_ptr<RIL_UIM_SAP_Response> rsp_ptr          = std::make_shared<RIL_UIM_SAP_Response>();
  RIL_UIM_SAP_RequestType               req_id           = RIL_UIM_SAP_APDU_REQ;

  /* Sanity check */
  if(params_ptr == NULL || req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("NULL params_ptr");
    return;
  }

  req_id = req_ptr->get_req_type();

  QCRIL_LOG_DEBUG("MsgId: 0x%x, qmi_err_code: 0x%x",
                  req_id,
                  params_ptr->qmi_rsp_data.qmi_err_code);

  if (rsp_ptr != NULL)
  {
    rsp_ptr->result = (params_ptr->qmi_rsp_data.qmi_err_code == 0) ?
                 RIL_UIM_SAP_SUCCESS : RIL_UIM_SAP_GENERIC_FAILURE;

    if (req_id == RIL_UIM_SAP_APDU_REQ ||
        req_id == RIL_UIM_SAP_TRANSFER_ATR_REQ)
    {
      uint8_t *data_ptr = params_ptr->qmi_rsp_data.rsp_data.sap_response_rsp.sap_response.data_ptr;
      uint32_t len      = params_ptr->qmi_rsp_data.rsp_data.sap_response_rsp.sap_response.data_len;

      if (data_ptr != NULL && len != 0)
      {
        rsp_ptr->atr_apdu_data.resize(len);
        memcpy(&rsp_ptr->atr_apdu_data[0], data_ptr, len);
      }
    }
    else if (req_id == RIL_UIM_SAP_TRANSFER_CARD_READER_STATUS_REQ)
    {
      if (params_ptr->qmi_rsp_data.rsp_data.sap_response_rsp.sap_response.data_ptr != NULL)
      {
        rsp_ptr->reader_status = *((int32_t *)params_ptr->qmi_rsp_data.rsp_data.sap_response_rsp.sap_response.data_ptr);
      }
    }
  }

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_sap_qmi_handle_sap_request_resp */


/*===========================================================================

  FUNCTION:  qcril_uim_sap_ind

===========================================================================*/
/*!
    @brief
    This is the SAP indication callback implementation for the QMI interface.

    @return
    None.

*/
/*=========================================================================*/
void qcril_uim_sap_ind
(
  qmi_uim_sap_indication_params_type   * ind_data_ptr
)
{
  std::shared_ptr<UimSapStatusIndMsg> ind_ptr = NULL;
  RIL_UIM_SAP_StatusType              status  = RIL_UIM_SAP_STATUS_UNKNOWN;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (ind_data_ptr == NULL)
  {
    QCRIL_LOG_ERROR("NULL ind_data_ptr");
    return;
  }

  if ((ind_data_ptr->slot - (uint8_t)QMI_UIM_SLOT_1) != qmi_ril_get_process_instance_id())
  {
    QCRIL_LOG_ERROR("Wrong slot info %d", ind_data_ptr->slot);
    return;
  }

  switch (ind_data_ptr->sap_state)
  {
    case QMI_UIM_SAP_STATE_DISCONNECTED_SUCCESSFULLY:
      status = RIL_UIM_SAP_DISCONNECT_GRACEFUL;
      qcril_uim_sap_sim_state = RIL_UIM_SAP_CARD_STATE_NOT_ACCESSSIBLE;
      break;

    case QMI_UIM_SAP_STATE_CONNECTION_ERROR:
    case QMI_UIM_SAP_STATE_NOT_ENABLED:
      status = RIL_UIM_SAP_STATUS_NOT_ACCESSIABLE;
      qcril_uim_sap_sim_state = RIL_UIM_SAP_CARD_STATE_NOT_ACCESSSIBLE;
      break;

    case QMI_UIM_SAP_STATE_CONNECTED_SUCCESSFULLY:
      qcril_uim_sap_sim_state = RIL_UIM_SAP_CARD_STATE_INSERTED;
      QCRIL_LOG_INFO("Skipping SAP UNSOL response for sap_state: 0x%x", qcril_uim_sap_sim_state);
      return;

    case QMI_UIM_SAP_STATE_CONNECTING:
    case QMI_UIM_SAP_STATE_DISCONNECTING:
    default:
      /* Note that for a succesfully connected case, if the connection response
      isnt sent yet, the STATUS_IND is sent after response callback comes */
      QCRIL_LOG_INFO("Skipping SAP UNSOL response for sap_state: 0x%x", qcril_uim_sap_sim_state);
      return;
  }

  ind_ptr = std::make_shared<UimSapStatusIndMsg>(status);

  if (ind_ptr != NULL)
  {
    ind_ptr->broadcast();
  }
} /* qcril_qmi_sap_ind_hdlr */


/*===========================================================================

FUNCTION:  qcril_uim_sap_process_card_status

===========================================================================*/
/*!
    @brief
    This function handles the card state change indication.

    @return
    None.

*/
/*=========================================================================*/
void qcril_uim_sap_process_card_status
(
  qmi_uim_card_state_type   card_state,
  qmi_uim_card_error_type   err_type,
  uint8_t                   slot
)
{
  std::shared_ptr<UimSapStatusIndMsg> ind_ptr = NULL;

  QCRIL_LOG_DEBUG("SAP card state: %d, err_type: %d, slot: %d",
    card_state, err_type, slot);

  if (qmi_ril_get_process_instance_id() != slot)
  {
    return;
  }

  if (card_state == QMI_UIM_CARD_STATE_PRESENT)
  {
    if (qcril_uim_sap_sim_state == RIL_UIM_SAP_CARD_STATE_INSERTED)
    {
      /* card state is present means sap is disconnectd */
      ind_ptr = std::make_shared<UimSapStatusIndMsg>(RIL_UIM_SAP_DISCONNECT_GRACEFUL);

      if (ind_ptr != NULL)
      {
        ind_ptr->broadcast();
      }
      qcril_uim_sap_sim_state = RIL_UIM_SAP_CARD_STATE_NOT_ACCESSSIBLE;
    }
  }
  else if (card_state == QMI_UIM_CARD_STATE_ERROR)
  {
    switch(err_type)
    {
      case QMI_UIM_CARD_ERROR_POWER_DOWN:
        qcril_uim_sap_sim_state = RIL_UIM_SAP_CARD_STATE_POWER_DOWN;
        break;
      case QMI_UIM_CARD_ERROR_SAP_CONNECTED:
        qcril_uim_sap_sim_state = RIL_UIM_SAP_CARD_STATE_INSERTED;
        break;
      case QMI_UIM_CARD_ERROR_UNKNOWN_REMOVED:
        qcril_uim_sap_sim_state = RIL_UIM_SAP_CARD_STATE_REMOVED;
        break;
      default:
        qcril_uim_sap_sim_state = RIL_UIM_SAP_CARD_STATE_NOT_ACCESSSIBLE;
        break;
    }
  }
  else if (card_state == QMI_UIM_CARD_STATE_ABSENT)
  {
    qcril_uim_sap_sim_state = RIL_UIM_SAP_CARD_STATE_REMOVED;
  }
  else
  {
    qcril_uim_sap_sim_state = RIL_UIM_SAP_CARD_STATE_NOT_ACCESSSIBLE;
  }

  QCRIL_LOG_DEBUG("SAP state: %d", qcril_uim_sap_sim_state);
} /* qcril_uim_sap_process_card_status */
