/*===========================================================================

  Copyright (c) 2017 - 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
/*===========================================================================

                           INCLUDE FILES

===========================================================================*/
#include <string.h>
#include "framework/Log.h"
#include "modules/uim/UimSimlockTempUnlockExpireInd.h"
#include "modules/uim_service/UimSetRilPropertySyncMsg.h"
#include "qcril_uim.h"
#include "qcril_uim_util.h"
#include "qcril_uim_restart.h"
#include "qcril_uim_card.h"
#include "qcril_uim_security.h"

#define TAG "QCRIL_UIM_SECURITY"

/*===========================================================================

                               INTERNAL FUNCTIONS

===========================================================================*/

/*===========================================================================

  FUNCTION:  qcril_uim_fetch_retries

===========================================================================*/
/*!
    @brief
    Finds the app related to the passed session type & fetches its remaining
    retry attempts depending upon the request type - PIN1/PIN2/PUK1/PUK2.

    @return
    RIL_UIM_E_SUCCESS if successfully found, else RIL_UIM_E_INTERNAL_ERR
*/
/*=========================================================================*/
static RIL_UIM_Errno qcril_uim_fetch_retries
(
  RIL_UIM_PIN_Type              pin_type,
  qmi_uim_session_type          session_type,
  int                         * num_retries_ptr
)
{
  uint8                  index     = 0;
  uint8                  slot      = 0;
  qmi_uim_app_state_type app_state = QMI_UIM_APP_STATE_UNKNOWN;
  RIL_UIM_Errno          err       = RIL_UIM_E_INTERNAL_ERR;

  if (num_retries_ptr == NULL)
  {
    return RIL_UIM_E_INTERNAL_ERR;
  }

  /* Fetch approriate App index to extract the retries */
  err = qcril_uim_extract_index(&index, &slot, session_type);
  if (err != RIL_UIM_E_SUCCESS)
  {
    return err;
  }

  if ((index >= QMI_UIM_MAX_APP_PER_CARD_COUNT) || (slot >= QMI_UIM_MAX_CARD_COUNT))
  {
    QCRIL_LOG_INFO("Invalid indexes for retries: slot_index: 0x%x, app_index: 0x%x",
                   slot, index);
    return RIL_UIM_E_INTERNAL_ERR;
  }

  app_state = qcril_uim.card_status.card[slot].application[index].app_state;

  /* Also check if the retry info is relevant based on the App state */
  if ((app_state == QMI_UIM_APP_STATE_UNKNOWN) || (app_state == QMI_UIM_APP_STATE_DETECTED))
  {
    QCRIL_LOG_ERROR("Invalid App state for retries: 0x%x, slot_index: 0x%x, app_index: 0x%x",
                    app_state, slot, index);
    return RIL_UIM_E_INVALID_SIM_STATE;
  }

  /* Fetch retries based on the request type */
  switch(pin_type)
  {
    case RIL_UIM_SIM_PIN1:
      *num_retries_ptr = qcril_uim.card_status.card[slot].application[index].pin1_num_retries;
      break;
    case RIL_UIM_SIM_PIN2:
      *num_retries_ptr = qcril_uim.card_status.card[slot].application[index].pin2_num_retries;
      break;
    case RIL_UIM_SIM_PUK1:
      *num_retries_ptr = qcril_uim.card_status.card[slot].application[index].puk1_num_retries;
      break;
    case RIL_UIM_SIM_PUK2:
      *num_retries_ptr = qcril_uim.card_status.card[slot].application[index].puk2_num_retries;
      break;
    default:
      QCRIL_LOG_ERROR( "Unsupported Request ID 0x%x\n", pin_type);
      return RIL_UIM_E_INTERNAL_ERR;
  }

  QCRIL_LOG_INFO( "Remaining retries: 0x%x, slot: 0x%x, app_index: 0x%x)\n",
                 *num_retries_ptr, slot, index);

  return RIL_UIM_E_SUCCESS;
} /* qcril_uim_fetch_retries */


/*===========================================================================

  FUNCTION:  qcril_uim_is_pin_disabled

===========================================================================*/
/*!
    @brief
    Checks if the passed app's PIN1 is disabled

    @return
    TRUE if disabled, else FALSE
*/
/*=========================================================================*/
static boolean qcril_uim_is_pin_disabled
(
  qmi_uim_session_type              session_type
)
{
  uint8   index   = 0;
  uint8   slot    = 0;

  if (qcril_uim_extract_index(&index, &slot, session_type) == RIL_UIM_E_SUCCESS)
  {
    if ((index < QMI_UIM_MAX_APP_PER_CARD_COUNT) && (slot < QMI_UIM_MAX_CARD_COUNT))
    {
      /* Check appropriate pin state */
      if (qcril_uim.card_status.card[slot].application[index].univ_pin ==
          QCRIL_UIM_UPIN_STATE_REPLACES_PIN1)
      {
        if (qcril_uim.card_status.card[slot].upin_state ==
            QMI_UIM_PIN_STATE_DISABLED)
        {
          return TRUE;
        }
      }
      else if (qcril_uim.card_status.card[slot].application[index].pin1_state ==
               QMI_UIM_PIN_STATE_DISABLED)
      {
        return TRUE;
      }
    }
  }
  return FALSE;
} /* qcril_uim_is_pin_disabled */


/*===========================================================================

  FUNCTION:  qcril_uim_pin_resp

===========================================================================*/
/*!
    @brief
    Handle PIN operation confirmations

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_pin_resp
(
  qcril_uim_callback_params_type  * params_ptr,
  const std::shared_ptr<UimVerifyPINMsg> msg_ptr
)
{
  RIL_UIM_Errno                       RIL_UIM_Err          = RIL_UIM_E_SUCCESS;
  qmi_uim_pin_id_type                 pin_id               = QMI_UIM_PIN_ID_PIN1;
  int                                 num_retries          = 0;
  qcril_uim_indication_params_type  * card_status_ind_ptr  = NULL;
  qmi_uim_rsp_data_type               card_status_rsp      = {};
  int                                 qmi_err_code         = 0;
  auto                                rsp_ptr              = std::make_shared<RIL_UIM_SIM_PIN_Response>();     

  if(msg_ptr == NULL || params_ptr == NULL)
  {
    QCRIL_LOG_ERROR("NULL pointer");
    QCRIL_ASSERT(0);
    return;
  }

  const std::shared_ptr<Message> req_ptr = msg_ptr->get_request_msg();

  if(req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("NULL pointer");
    return;
  }

  /* Query card status before sending the response. This is needed to comply to certain
     race condition scenarios. An exmaple is when the get sim status is called soon after the
     PIN response - the PIN status is actually changed but QCRIL hasnt gotten the indication yet */
  card_status_ind_ptr = (qcril_uim_indication_params_type *)qcril_malloc(sizeof(qcril_uim_indication_params_type));
  if (card_status_ind_ptr != NULL)
  {
    memset(card_status_ind_ptr, 0x00, sizeof(qcril_uim_indication_params_type));
    memset(&card_status_rsp, 0x00, sizeof(qmi_uim_rsp_data_type));
    qmi_err_code = qcril_uim_send_qmi_sync_msg(QCRIL_UIM_REQUEST_GET_CARD_STATUS, nullptr, &card_status_rsp);
    if (qmi_err_code < 0)
    {
      QCRIL_LOG_ERROR( "Get card status failed, qmi_err_code: 0x%x\n", qmi_err_code);
    }
    else
    {
      /* Proceed with updating the global using the common function.
         It takes care of sending the UNSOL before the PIN response */
      card_status_ind_ptr->ind_id      = QMI_UIM_SRVC_STATUS_CHANGE_IND_MSG;
      memcpy(&card_status_ind_ptr->ind_data.status_change_ind,
             &card_status_rsp.rsp_data.get_card_status_rsp.card_status,
             sizeof(qmi_uim_card_status_type));
      qcril_uim_process_status_change_ind(card_status_ind_ptr);
    }

    /* Clean up the allocated memory */
    qcril_free(card_status_ind_ptr);
    card_status_ind_ptr = NULL;
  }

  QCRIL_LOG_DEBUG( "qcril_uim_pin_resp: qmi_err_code=%d \n",
                    params_ptr->qmi_rsp_data.qmi_err_code);

  /* Retrieve the PIN id */
  if (RIL_UIM_SIM_PIN2 == msg_ptr->get_pin_type() ||
      RIL_UIM_SIM_PUK2 == msg_ptr->get_pin_type())
  {
    pin_id = QMI_UIM_PIN_ID_PIN2;
  }

  /* Response code for RIL */
  switch(params_ptr->qmi_rsp_data.qmi_err_code)
  {
    case QMI_UIM_SERVICE_ERR_NONE:
      RIL_UIM_Err = RIL_UIM_E_SUCCESS;
      break;

    case QMI_UIM_SERVICE_ERR_INCORRECT_PIN:
    case QMI_UIM_SERVICE_ERR_PIN_PERM_BLOCKED:
      RIL_UIM_Err = RIL_UIM_E_PASSWORD_INCORRECT;
      break;

    case QMI_UIM_SERVICE_ERR_PIN_BLOCKED:
      RIL_UIM_Err = RIL_UIM_E_PASSWORD_INCORRECT;
      if (pin_id == QMI_UIM_PIN_ID_PIN2)
      {
        RIL_UIM_Err = RIL_UIM_E_SIM_PUK2;
      }
      break;

    default:
      RIL_UIM_Err = RIL_UIM_E_INTERNAL_ERR;
      /* Pass the correct error on "change PIN1 when PIN1 is disabled" case */
      if ((params_ptr->qmi_rsp_data.rsp_id == QMI_UIM_SRVC_CHANGE_PIN_RSP_MSG) &&
          (RIL_UIM_SIM_PIN1 == msg_ptr->get_pin_type()) &&
          qcril_uim_is_pin_disabled(msg_ptr->get_session_info().session_type))
      {
        RIL_UIM_Err = RIL_UIM_E_REQUEST_NOT_SUPPORTED;
        QCRIL_LOG_DEBUG( "PIN disabled, change sim pin not supported, qmi_err_code:0x%x \n",
                         params_ptr->qmi_rsp_data.qmi_err_code);
      }
      break;
  }

  /* Compose number of retries */
  switch(params_ptr->qmi_rsp_data.rsp_id)
  {
    case QMI_UIM_SRVC_SET_PIN_PROTECTION_RSP_MSG:
      num_retries = params_ptr->qmi_rsp_data.rsp_data.set_pin_protection_rsp.num_retries;
      break;
    case QMI_UIM_SRVC_VERIFY_PIN_RSP_MSG:
      num_retries = params_ptr->qmi_rsp_data.rsp_data.verify_pin_rsp.num_retries;
      break;
    case QMI_UIM_SRVC_UNBLOCK_PIN_RSP_MSG:
      num_retries = params_ptr->qmi_rsp_data.rsp_data.unblock_pin_rsp.num_unblock_retries;
      break;
    case QMI_UIM_SRVC_CHANGE_PIN_RSP_MSG:
      num_retries = params_ptr->qmi_rsp_data.rsp_data.change_pin_rsp.num_retries;
      break;
    default:
      /* This case should never happen!! */
      RIL_UIM_Err = RIL_UIM_E_INTERNAL_ERR;
      num_retries = 0;
      break;
  }

  if(pin_id == QMI_UIM_PIN_ID_PIN1)
  {
    if (RIL_UIM_Err == RIL_UIM_E_SUCCESS)
    {
      qcril_uim_store_encrypted_pin(&params_ptr->qmi_rsp_data,
                                    msg_ptr->get_session_info().session_type);
    }
    else
    {
      /* Reset PIN1 data if PIN1 operation was unsuccessful */
      qcril_uim_clear_encrypted_pin(msg_ptr->get_session_info().session_type);
    }
  }

  if (rsp_ptr != NULL)
  {
    rsp_ptr->err = RIL_UIM_Err;
    rsp_ptr->no_of_retries = num_retries;
  }

  if (string(UimEnterSimPinRequestMsg::MESSAGE_NAME) == req_ptr->get_message_name())
  {
    (std::static_pointer_cast<UimEnterSimPinRequestMsg>(req_ptr))->sendResponse
                         (req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
  }
  else if (string(UimEnterSimPukRequestMsg::MESSAGE_NAME) == req_ptr->get_message_name())
  {
    (std::static_pointer_cast<UimEnterSimPukRequestMsg>(req_ptr))->sendResponse
                         (req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
  }
  else if (string(UimChangeSimPinRequestMsg::MESSAGE_NAME) == req_ptr->get_message_name())
  {
    (std::static_pointer_cast<UimChangeSimPinRequestMsg>(req_ptr))->sendResponse
                         (req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
  }
  else if (string(UimSetFacilityLockRequestMsg::MESSAGE_NAME) == req_ptr->get_message_name())
  {
    (std::static_pointer_cast<UimSetFacilityLockRequestMsg>(req_ptr))->sendResponse
                     (req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
  }
  else
  {
    QCRIL_LOG_ERROR("Invalid request %s", req_ptr->get_message_name().c_str());
  }
} /* qcril_uim_pin_resp */


/*===========================================================================

  FUNCTION:  qcril_uim_deperso_resp

===========================================================================*/
/*!
    @brief
    Handle deperso operation confirmation

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_deperso_resp
(
  qcril_uim_callback_params_type                   * params_ptr,
  const std::shared_ptr<UimEnterDePersoRequestMsg>   req_ptr
)
{
  RIL_UIM_Errno  RIL_UIM_Err  = RIL_UIM_E_SUCCESS;
  auto           rsp_ptr      = std::make_shared<RIL_UIM_PersoResponse>();

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if(params_ptr == NULL || req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("NULL pointer");
    QCRIL_ASSERT(0);
    return;
  }

  RIL_UIM_Err = qcril_uim_convert_err_value(params_ptr->qmi_rsp_data.qmi_err_code);

  QCRIL_LOG_DEBUG( "qcril_uim_deperso_resp: qmi_err_code=%d \n",
                    params_ptr->qmi_rsp_data.qmi_err_code);

  if (rsp_ptr != NULL)
  {
    rsp_ptr->err = RIL_UIM_Err;
    rsp_ptr->no_of_retries = params_ptr->qmi_rsp_data.rsp_data.depersonalization_rsp.num_retries;
  }
  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_deperso_resp */


/*===========================================================================

  FUNCTION:  qcril_uim_perso_resp

===========================================================================*/
/*!
    @brief
    Handle perso operation confirmation

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_perso_resp
(
  qcril_uim_callback_params_type            * params_ptr,
  const std::shared_ptr<UimPersoRequestMsg>   req_ptr
)
{
  RIL_UIM_Errno                                 RIL_UIM_Err  = RIL_UIM_E_SUCCESS;
  int                                           num_retries  = -1;
  std::shared_ptr<RIL_UIM_PersoResponse>        rsp_ptr      = NULL;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if(params_ptr == NULL || req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("NULL pointer");
    return;
  }

  /* Update response error code */
  switch(params_ptr->qmi_rsp_data.qmi_err_code)
  {
    case 0:  /* QMI_ERR_NONE */
      RIL_UIM_Err = RIL_UIM_E_SUCCESS;
      break;

    case 12: /* QMI_ERR_INCORRECT_PIN */
    case 36: /* QMI_ERR_PIN_PERM_BLOCKED */
      RIL_UIM_Err = RIL_UIM_E_PASSWORD_INCORRECT;
      break;

    case 35: /* QMI_ERR_PIN_BLOCKED */
      RIL_UIM_Err = RIL_UIM_E_PASSWORD_INCORRECT;
      break;

    default:
      RIL_UIM_Err = RIL_UIM_E_INTERNAL_ERR;
      break;
  }

  QCRIL_LOG_DEBUG( "qcril_uim_perso_resp: qmi_err_code=%d \n",
                    params_ptr->qmi_rsp_data.qmi_err_code);

  /* Update number of retries */
  if (params_ptr->qmi_rsp_data.rsp_data.personalization_rsp.num_retries_valid)
  {
    num_retries = params_ptr->qmi_rsp_data.rsp_data.personalization_rsp.num_retries;
  }

  rsp_ptr = std::make_shared<RIL_UIM_PersoResponse>();

  if (rsp_ptr != NULL)
  {
    rsp_ptr->err = RIL_UIM_Err;
    rsp_ptr->no_of_retries = num_retries;
  }

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_perso_resp */


/*===========================================================================
  FUNCTION:  qcril_uim_remote_unlock_resp
===========================================================================*/
/*!
    @brief
    Handle remote unlock response
    @return
    None
*/
/*=========================================================================*/
void qcril_uim_remote_unlock_resp
(
  qcril_uim_callback_params_type  * params_ptr,
  const std::shared_ptr<UimRemoteSimlockRequestMsg> req_ptr
)
{
  RIL_UIM_RemoteSimlockVersion             version       = {};
  RIL_UIM_RemoteSimlockStatus              status        = {};
  RIL_UIM_RemoteSimlockResponseType        response      = RIL_UIM_REMOTE_SIMLOCK_RESP_SUCCESS;
  int                                      timer_value   = 0;
  uint8_t                                * data_ptr      = NULL;
  uint32_t                                 data_len      = 0;
  std::shared_ptr<RIL_UIM_RemoteSimlockResponse> rsp_ptr = NULL;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if(params_ptr == NULL || req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("NULL pointer");
    return;
  }

  QCRIL_LOG_DEBUG( "qcril_uim_perso_resp: qmi_err_code=%d \n",
                    params_ptr->qmi_rsp_data.qmi_err_code);

  if (params_ptr->qmi_rsp_data.qmi_err_code == QMI_UIM_SERVICE_ERR_NONE ||
      params_ptr->qmi_rsp_data.qmi_err_code == QMI_UIM_SERVICE_ERR_NO_EFFECT)
  {
    response = RIL_UIM_REMOTE_SIMLOCK_RESP_SUCCESS;
  }
  else if (params_ptr->qmi_rsp_data.qmi_err_code == QMI_UIM_SERVICE_ERR_INCOMPATIBLE_STATE)
  {
    response = RIL_UIM_REMOTE_SIMLOCK_RESP_TIMER_EXPIRED;
  }
  else
  {
    response = RIL_UIM_REMOTE_SIMLOCK_RESP_FAILURE;
  }

  switch(req_ptr->get_op_type())
  {
    case RIL_UIM_REMOTE_SIMLOCK_GENERATE_ENCRYPTED_KEY:
      data_ptr = params_ptr->qmi_rsp_data.rsp_data.remote_unlock_rsp.remote_unlock_rsp.data_ptr;
      data_len = params_ptr->qmi_rsp_data.rsp_data.remote_unlock_rsp.remote_unlock_rsp.data_len;
      break;
    case RIL_UIM_REMOTE_SIMLOCK_PROCESS_SIMLOCK_DATA:
      data_ptr = params_ptr->qmi_rsp_data.rsp_data.remote_unlock_rsp.remote_unlock_rsp.data_ptr;
      data_len = params_ptr->qmi_rsp_data.rsp_data.remote_unlock_rsp.remote_unlock_rsp.data_len;
      break;
    case RIL_UIM_REMOTE_SIMLOCK_GENERATE_HMAC:
      data_ptr = params_ptr->qmi_rsp_data.rsp_data.remote_unlock_rsp.remote_unlock_rsp.data_ptr;
      data_len = params_ptr->qmi_rsp_data.rsp_data.remote_unlock_rsp.remote_unlock_rsp.data_len;
      break;
    case RIL_UIM_REMOTE_SIMLOCK_GET_MAX_SUPPORTED_VERSION:
      version.majorVersion =
        params_ptr->qmi_rsp_data.rsp_data.remote_unlock_rsp.remote_simlock_version.major_version;
      version.minorVersion =
        params_ptr->qmi_rsp_data.rsp_data.remote_unlock_rsp.remote_simlock_version.minor_version;
      break;
    case RIL_UIM_REMOTE_SIMLOCK_GENERATE_BLOB:
      data_ptr = params_ptr->qmi_rsp_data.rsp_data.remote_unlock_rsp.remote_unlock_rsp.data_ptr;
      data_len = params_ptr->qmi_rsp_data.rsp_data.remote_unlock_rsp.remote_unlock_rsp.data_len;
      break;
    case RIL_UIM_REMOTE_SIMLOCK_START_UNLOCK_TIMER:
      timer_value = params_ptr->qmi_rsp_data.rsp_data.remote_unlock_rsp.timer_value;
      break;
    case RIL_UIM_REMOTE_SIMLOCK_STOP_UNLOCK_TIMER:
      break;
    default:
      response = RIL_UIM_REMOTE_SIMLOCK_RESP_FAILURE;
      break;
  }

  rsp_ptr = std::make_shared<RIL_UIM_RemoteSimlockResponse>();

  if (rsp_ptr != NULL)
  {
    rsp_ptr->rsp_type = response;
    rsp_ptr->op_type = req_ptr->get_op_type();
    rsp_ptr->version = version;
    rsp_ptr->timer_value = timer_value;
    rsp_ptr->status = status;
    if (data_ptr != NULL && data_len != 0)
    {
      rsp_ptr->rsp_data.resize(data_len);
      memcpy(&rsp_ptr->rsp_data[0], data_ptr, data_len);
    }
  }

  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_remote_unlock_resp */


/*===========================================================================

                               QCRIL INTERFACE FUNCTIONS

===========================================================================*/

/*===========================================================================

  FUNCTION:  qcril_uim_request_enter_pin

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_ENTER_SIM_PIN/RIL_REQUEST_ENTER_SIM_PIN2 requests
    from the framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_enter_pin
(
  const std::shared_ptr<UimEnterSimPinRequestMsg> msg_ptr
)
{
  RIL_UIM_Errno                             err                  = RIL_UIM_E_SUCCESS;
  uint16                                    first_level_df_path  = 0;
  uint8                                     slot                 = qmi_ril_get_process_instance_id();
#ifdef FEATURE_QCRIL_UIM_QMI_UPIN
  uint8                                     index                = 0;
  uint8                                     prov_slot            = 0;
#endif /* FEATURE_QCRIL_UIM_QMI_UPIN */
  qmi_uim_verify_pin_params_type            pin_params           = {};
  std::shared_ptr<RIL_UIM_SIM_PIN_Response> resp_ptr             = std::make_shared<RIL_UIM_SIM_PIN_Response>();
  std::shared_ptr<UimVerifyPINMsg>          msg                  = NULL;

  if(msg_ptr == NULL || resp_ptr == NULL || slot >= QMI_UIM_MAX_CARD_COUNT)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot process response");
    QCRIL_ASSERT(0);
    return;
  }

  /* Update the file path based on passed aid pointe */
  first_level_df_path = (msg_ptr->get_aid().empty()) ?
                         QCRIL_UIM_FILEID_DF_CDMA : QCRIL_UIM_FILEID_ADF_USIM_CSIM;

  QCRIL_ASSERT(slot < QMI_UIM_MAX_CARD_COUNT);

  do
  {
    /* Extract session type, we need a provisioning session for pin operations */
    err = qcril_uim_extract_session_type(slot,
                                         (const char *)msg_ptr->get_aid().c_str(),
                                         first_level_df_path,
                                         &pin_params.session_info,
                                         NULL,
                                         0);
    if ((err != RIL_UIM_E_SUCCESS) ||
        (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
        (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
        (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
    {
      if (first_level_df_path == QCRIL_UIM_FILEID_DF_CDMA)
      {
        err = qcril_uim_extract_session_type(slot,
                                             (const char *)msg_ptr->get_aid().c_str(),
                                             QCRIL_UIM_FILEID_DF_GSM,
                                             &pin_params.session_info,
                                             NULL,
                                             0);
      }
      if ((err != RIL_UIM_E_SUCCESS) ||
          (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
          (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
          (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
      {
        resp_ptr->err = RIL_UIM_E_INVALID_ARGUMENTS;
        break;
      }
    }

    QCRIL_LOG_INFO( "Session type found: %d", pin_params.session_info.session_type);

    /* PIN id */
    if (msg_ptr->get_pin_type() == RIL_UIM_SIM_PIN1)
    {
      pin_params.pin_id = QMI_UIM_PIN_ID_PIN1;
    }
    else if (msg_ptr->get_pin_type() == RIL_UIM_SIM_PIN2)
    {
      pin_params.pin_id = QMI_UIM_PIN_ID_PIN2;
    }
    else
    {
      /* Invalid PIN ID */
      QCRIL_LOG_ERROR("Invalid Pin from RIL Request ID\n");
      resp_ptr->err = RIL_UIM_E_INVALID_ARGUMENTS;
      break;
    }

  #ifdef FEATURE_QCRIL_UIM_QMI_UPIN
    /* Check if PIN1 is replaced by UPIN for the specified App */
    if ((pin_params.pin_id == QMI_UIM_PIN_ID_PIN1) &&
        (qcril_uim_extract_index(&index,
                                 &prov_slot,
                                 pin_params.session_info.session_type) == RIL_UIM_E_SUCCESS))
    {
      if ((index >= 0) && (index < QMI_UIM_MAX_APP_PER_CARD_COUNT) &&
          (prov_slot == slot) &&
          (prov_slot >= 0) && (prov_slot < QMI_UIM_MAX_CARD_COUNT))
      {
        if (qcril_uim.card_status.card[prov_slot].application[index].univ_pin ==
            QCRIL_UIM_UPIN_STATE_REPLACES_PIN1)
        {
          pin_params.pin_id = QMI_UIM_PIN_ID_UPIN;
          QCRIL_LOG_INFO("PIN1 replaced by UPIN for card[%d].application[%d]",
                         prov_slot, index);
        }
      }
    }
  #endif /* FEATURE_QCRIL_UIM_QMI_UPIN */

    /* Special case for empty PIN string. In this case the PIN retries left is returned
       immediately from the global card status */
    if (msg_ptr->get_pin().empty())
    {
      QCRIL_LOG_INFO( "Empty PIN string, fetching retries.");

      err = qcril_uim_fetch_retries(msg_ptr->get_pin_type(),
                                    pin_params.session_info.session_type,
                                    &resp_ptr->no_of_retries);

      resp_ptr->err = err;
      break;
    }

    /* PIN value */
    pin_params.pin_data.data_ptr = (uint8_t *)msg_ptr->get_pin().c_str();
    pin_params.pin_data.data_len = msg_ptr->get_pin().length();

    msg = std::make_shared<UimVerifyPINMsg>(msg_ptr, pin_params.session_info,
                                            msg_ptr->get_pin_type());

    if (msg == NULL)
    {
      resp_ptr->err = RIL_UIM_E_NO_MEMORY;
      break;
    }

    /* Proceed with verify pin */
    if(qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_VERIFY_PIN,
                                    (void*)&pin_params,
                                     msg) == 0)
    {
      return;
    }
    resp_ptr->err = RIL_UIM_E_MODEM_ERR;
  } while(0);

  msg_ptr->sendResponse(msg_ptr, Message::Callback::Status::SUCCESS, resp_ptr);
} /* qcril_uim_request_enter_pin */


/*===========================================================================

  FUNCTION:  qcril_uim_request_enter_puk

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_ENTER_SIM_PUK/RIL_REQUEST_ENTER_SIM_PUK2 requests
    from the framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_enter_puk
(
  const std::shared_ptr<UimEnterSimPukRequestMsg> msg_ptr
)
{
  int                                       res                  = -1;
  RIL_UIM_Errno                             err                  = RIL_UIM_E_SUCCESS;
  uint16                                    first_level_df_path  = 0;
  uint8                                     slot                 = qmi_ril_get_process_instance_id();
#ifdef FEATURE_QCRIL_UIM_QMI_UPIN
  uint8                                     index                = 0;
  uint8                                     prov_slot            = 0;
#endif /* FEATURE_QCRIL_UIM_QMI_UPIN */
  qmi_uim_unblock_pin_params_type           pin_params           = {};
  std::shared_ptr<RIL_UIM_SIM_PIN_Response> rsp_ptr              = std::make_shared<RIL_UIM_SIM_PIN_Response>();
  std::shared_ptr<UimVerifyPINMsg>          msg                  = NULL;

  if(msg_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot process response");
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_LOG_INFO( "qcril_uim_request_enter_puk()\n");

  memset(&pin_params, 0, sizeof(qmi_uim_unblock_pin_params_type));

  /* Update the file path based on passed aid pointer */
  first_level_df_path = (msg_ptr->get_aid().length() == 0) ?
                         QCRIL_UIM_FILEID_DF_CDMA : QCRIL_UIM_FILEID_ADF_USIM_CSIM;

  /* Extract session type, we need a provisioning session for pin operations */
  err = qcril_uim_extract_session_type(slot,
                                       msg_ptr->get_aid().c_str(),
                                       first_level_df_path,
                                       &pin_params.session_info,
                                       NULL,
                                       0);
  if ((err != RIL_UIM_E_SUCCESS) ||
      (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
      (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
      (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
  {
    if (first_level_df_path == QCRIL_UIM_FILEID_DF_CDMA)
    {
      err = qcril_uim_extract_session_type(slot,
                                           msg_ptr->get_aid().c_str(),
                                           QCRIL_UIM_FILEID_DF_GSM,
                                           &pin_params.session_info,
                                           NULL,
                                           0);
    }
    if ((err != RIL_UIM_E_SUCCESS) ||
        (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
        (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
        (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
    {
      if (rsp_ptr != NULL)
      {
        rsp_ptr->err = RIL_UIM_E_INVALID_ARGUMENTS;
      }
      msg_ptr->sendResponse(msg_ptr, Message::Callback::Status::SUCCESS, rsp_ptr); 
      return;
    }
  }

  QCRIL_LOG_INFO( "Session type found: %d", pin_params.session_info.session_type);

  /* PIN id */
  switch (msg_ptr->get_pin_type())
  {
    case RIL_UIM_SIM_PUK1:
      pin_params.pin_id = QMI_UIM_PIN_ID_PIN1;
      break;
    case RIL_UIM_SIM_PUK2:
      pin_params.pin_id = QMI_UIM_PIN_ID_PIN2;
      break;
    default:
      /* Invalid PIN ID */
      QCRIL_LOG_ERROR( " Invalid Pin from RIL Request ID\n");
      if (rsp_ptr != NULL)
      {
        rsp_ptr->err = RIL_UIM_E_INVALID_ARGUMENTS;
      }
      msg_ptr->sendResponse(msg_ptr, Message::Callback::Status::SUCCESS, rsp_ptr); 
      return;
  }

#ifdef FEATURE_QCRIL_UIM_QMI_UPIN
  /* Check if PIN1 is replaced by UPIN for the specified App */
  if ((pin_params.pin_id == QMI_UIM_PIN_ID_PIN1) &&
      (qcril_uim_extract_index(&index,
                               &prov_slot,
                               pin_params.session_info.session_type) == RIL_UIM_E_SUCCESS))
  {
    if ((index >= 0) && (index < QMI_UIM_MAX_APP_PER_CARD_COUNT) &&
        (prov_slot == slot) &&
        (prov_slot >= 0) && (prov_slot < QMI_UIM_MAX_CARD_COUNT))
    {
      if (qcril_uim.card_status.card[prov_slot].application[index].univ_pin ==
          QCRIL_UIM_UPIN_STATE_REPLACES_PIN1)
      {
        pin_params.pin_id = QMI_UIM_PIN_ID_UPIN;
        QCRIL_LOG_INFO("PIN1 replaced by UPIN for card[%d].application[%d]",
                       prov_slot, index);
      }
    }
  }
#endif /* FEATURE_QCRIL_UIM_QMI_UPIN */

  /* Special case for empty PUK string. In this case the PUK retries left is returned
     immediately from the global card status */
  if (msg_ptr->get_pin().empty())
  {
    QCRIL_LOG_INFO( "Empty PUK string, fetching retries.");

    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = qcril_uim_fetch_retries(msg_ptr->get_pin_type(),
                                    pin_params.session_info.session_type,
                                    &rsp_ptr->no_of_retries);
    }

    /* Generate response */
    msg_ptr->sendResponse(msg_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  /* PUK value */
  pin_params.puk_data.data_ptr = (uint8_t *)msg_ptr->get_puk().c_str();
  pin_params.puk_data.data_len = msg_ptr->get_puk().length();

  /* New PIN value */
  pin_params.new_pin_data.data_ptr = (uint8_t *)msg_ptr->get_pin().c_str();
  pin_params.new_pin_data.data_len = msg_ptr->get_pin().length();

  msg = std::make_shared<UimVerifyPINMsg>(msg_ptr, pin_params.session_info,
                                          msg_ptr->get_pin_type());

  if (msg != NULL)
  {
    /* Proceed with unblock pin */
    res = qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_UNBLOCK_PIN,
                                       (void*)&pin_params,
                                       msg);
  }

  if (res < 0)
  {
    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = RIL_UIM_E_MODEM_ERR;
    }
    msg_ptr->sendResponse(msg_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
  }
} /* qcril_uim_request_enter_puk */


/*===========================================================================

  FUNCTION:  qcril_uim_request_change_pin

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_CHANGE_SIM_PIN/RIL_REQUEST_CHANGE_SIM_PIN2 requests
    from the framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_change_pin
(
  const std::shared_ptr<UimChangeSimPinRequestMsg> msg_ptr
)
{
  int                                       res                  = -1;
  RIL_UIM_Errno                             err                  = RIL_UIM_E_SUCCESS;
  uint16                                    first_level_df_path  = 0;
  uint8                                     slot                 = qmi_ril_get_process_instance_id();
#ifdef FEATURE_QCRIL_UIM_QMI_UPIN
  uint8                                     index                = 0;
  uint8                                     prov_slot            = 0;
#endif /* FEATURE_QCRIL_UIM_QMI_UPIN */
  qmi_uim_change_pin_params_type            pin_params           = {};
  std::shared_ptr<RIL_UIM_SIM_PIN_Response> rsp_ptr              = std::make_shared<RIL_UIM_SIM_PIN_Response>();
  std::shared_ptr<UimVerifyPINMsg>          msg                  = NULL;

  if(msg_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot process response");
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_LOG_INFO( "qcril_uim_request_change_pin()\n");

  /* Update the file path based on passed aid pointer */
  first_level_df_path = (msg_ptr->get_aid().length() == 0) ?
                         QCRIL_UIM_FILEID_DF_CDMA : QCRIL_UIM_FILEID_ADF_USIM_CSIM;

  /* Extract session type, we need a provisioning session for pin operations */
  err = qcril_uim_extract_session_type(slot,
                                       msg_ptr->get_aid().c_str(),
                                       first_level_df_path,
                                       &pin_params.session_info,
                                       NULL,
                                       0);
  if ((err != RIL_UIM_E_SUCCESS) ||
      (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
      (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
      (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
  {
    if (first_level_df_path == QCRIL_UIM_FILEID_DF_CDMA)
    {
      err = qcril_uim_extract_session_type(slot,
                                           msg_ptr->get_aid().c_str(),
                                           QCRIL_UIM_FILEID_DF_GSM,
                                           &pin_params.session_info,
                                           NULL,
                                           0);
    }
    if ((err != RIL_UIM_E_SUCCESS) ||
        (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
        (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
        (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
    {
      if (rsp_ptr != NULL)
      {
        rsp_ptr->err = RIL_UIM_E_INVALID_ARGUMENTS;
      }
      msg_ptr->sendResponse(msg_ptr, Message::Callback::Status::SUCCESS, rsp_ptr); 
      return;
    }
  }

  QCRIL_LOG_INFO( "Session type found: %d", pin_params.session_info.session_type);

  /* PIN id */
  switch (msg_ptr->get_pin_type())
  {
    case RIL_UIM_SIM_PIN1:
      pin_params.pin_id = QMI_UIM_PIN_ID_PIN1;
      break;
    case RIL_UIM_SIM_PIN2:
      pin_params.pin_id = QMI_UIM_PIN_ID_PIN2;
      break;
    default:
      /* Invalid PIN ID */
      QCRIL_LOG_ERROR( " Invalid Pin from RIL Request ID\n");
      if (rsp_ptr != NULL)
      {
        rsp_ptr->err = RIL_UIM_E_INVALID_ARGUMENTS;
      }
      msg_ptr->sendResponse(msg_ptr, Message::Callback::Status::SUCCESS, rsp_ptr); 
      return;
  }

#ifdef FEATURE_QCRIL_UIM_QMI_UPIN
  /* Check if PIN1 is replaced by UPIN for the specified App */
  if ((pin_params.pin_id == QMI_UIM_PIN_ID_PIN1) &&
      (qcril_uim_extract_index(&index,
                               &prov_slot,
                               pin_params.session_info.session_type) == RIL_UIM_E_SUCCESS))
  {
    if ((index >= 0) && (index < QMI_UIM_MAX_APP_PER_CARD_COUNT) &&
        (prov_slot == slot) &&
        (prov_slot >= 0) && (prov_slot < QMI_UIM_MAX_CARD_COUNT))
    {
      if (qcril_uim.card_status.card[prov_slot].application[index].univ_pin ==
          QCRIL_UIM_UPIN_STATE_REPLACES_PIN1)
      {
        pin_params.pin_id = QMI_UIM_PIN_ID_UPIN;
        QCRIL_LOG_INFO("PIN1 replaced by UPIN for card[%d].application[%d]",
                       prov_slot, index);
      }
    }
  }
#endif /* FEATURE_QCRIL_UIM_QMI_UPIN*/

  /* Old PIN value */
  pin_params.old_pin_data.data_ptr = (uint8_t *)msg_ptr->get_old_pin().c_str();
  pin_params.old_pin_data.data_len = msg_ptr->get_old_pin().length();

  /* New PIN value */
  pin_params.new_pin_data.data_ptr = (uint8_t *)msg_ptr->get_new_pin().c_str();
  pin_params.new_pin_data.data_len = msg_ptr->get_new_pin().length();

  msg = std::make_shared<UimVerifyPINMsg>(msg_ptr, pin_params.session_info,
                                          msg_ptr->get_pin_type());

  if (msg != NULL)
  {
    /* Proceed with change pin */
    res = qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_CHANGE_PIN,
                                       (void*)&pin_params,
                                       msg);
  }

  if (res < 0)
  {
    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = RIL_UIM_E_MODEM_ERR;
    }
    msg_ptr->sendResponse(msg_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
  }
} /* qcril_uim_request_change_pin */


/*===========================================================================

  FUNCTION:  qcril_uim_request_set_pin_status

===========================================================================*/
/*!
    @brief
    Handles QCRIL_UIM_EVT_INTERNAL_MMGSDI_SET_PIN1_STATUS request from
    the framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_set_pin_status
(
  const std::shared_ptr<UimSetFacilityLockRequestMsg> req_ptr
)
{
  int                                       res        = -1;
  RIL_UIM_Errno                             err        = RIL_UIM_E_SUCCESS;
  uint16                                    first_level_df_path = 0;
#ifdef FEATURE_QCRIL_UIM_QMI_UPIN
  uint8                                     index      = 0;
  uint8                                     prov_slot  = 0;
#endif /* FEATURE_QCRIL_UIM_QMI_UPIN */
  qmi_uim_set_pin_protection_params_type    pin_params      = {};
  auto rsp_ptr  = std::make_shared<RIL_UIM_SIM_PIN_Response>();
  std::shared_ptr<UimVerifyPINMsg>          msg_ptr         = NULL;

  if(req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot process response");
    QCRIL_ASSERT(0);
    return;
  }
  QCRIL_LOG_INFO( "qcril_uim_request_set_pin_status\n");

  /* Update the file path based on passed aid pointer */
  first_level_df_path = (req_ptr->get_aid().length() == 0) ?
                         QCRIL_UIM_FILEID_DF_CDMA : QCRIL_UIM_FILEID_ADF_USIM_CSIM;

  /* Extract session type, we need a provisioning session for pin operations */
  err = qcril_uim_extract_session_type(qmi_ril_get_process_instance_id(),
                                       req_ptr->get_aid().c_str(),
                                       first_level_df_path,
                                       &pin_params.session_info,
                                       NULL,
                                       0);
  if ((err != RIL_UIM_E_SUCCESS) ||
      (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
      (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
      (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
  {
    if (first_level_df_path == QCRIL_UIM_FILEID_DF_CDMA)
    {
      err = qcril_uim_extract_session_type(qmi_ril_get_process_instance_id(),
                                           req_ptr->get_aid().c_str(),
                                           QCRIL_UIM_FILEID_DF_GSM,
                                           &pin_params.session_info,
                                           NULL,
                                           0);
    }
    if ((err != RIL_UIM_E_SUCCESS) ||
        (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_1) ||
        (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_2) ||
        (pin_params.session_info.session_type == QMI_UIM_SESSION_TYPE_CARD_SLOT_3))
    {
      if (rsp_ptr != NULL)
      {
        rsp_ptr->err = RIL_UIM_E_INVALID_ARGUMENTS;
      }

      req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
      return;
    }
  }

  QCRIL_LOG_INFO( "Session type found: %d", pin_params.session_info.session_type);

  /* pin parameters */
  if (req_ptr->get_status())
  {
    pin_params.pin_operation = QMI_UIM_PIN_OP_ENABLE;
  }
  else
  {
    pin_params.pin_operation = QMI_UIM_PIN_OP_DISABLE;
  }

  pin_params.pin_id = QMI_UIM_PIN_ID_PIN1;
  pin_params.pin_data.data_ptr = (uint8_t *)req_ptr->get_password().c_str();
  pin_params.pin_data.data_len = req_ptr->get_password().length();

#ifdef FEATURE_QCRIL_UIM_QMI_UPIN
  /* In case of UPIN, only disable is currently supported */
  if ((pin_params.pin_operation == QMI_UIM_PIN_OP_DISABLE) &&
      (qcril_uim_extract_index(&index,
                               &prov_slot,
                               pin_params.session_info.session_type) == RIL_UIM_E_SUCCESS))
  {
    if ((index >= 0) && (index < QMI_UIM_MAX_APP_PER_CARD_COUNT) &&
        (prov_slot == slot) &&
        (prov_slot >= 0) && (prov_slot < QMI_UIM_MAX_CARD_COUNT))
    {
      if (qcril_uim.card_status.card[prov_slot].application[index].univ_pin ==
          QCRIL_UIM_UPIN_STATE_REPLACES_PIN1)
      {
        pin_params.pin_id = QMI_UIM_PIN_ID_UPIN;
        QCRIL_LOG_INFO("PIN1 replaced by UPIN for card[%d].application[%d]",
                       prov_slot, index);
      }
    }
  }
#endif /* FEATURE_QCRIL_UIM_QMI_UPIN */

  msg_ptr = std::make_shared<UimVerifyPINMsg>(req_ptr, pin_params.session_info,
                                              RIL_UIM_SIM_PIN1);

  if (msg_ptr != NULL)
  {
    /* Proceed with set pin protection */
    res = qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_SET_PIN,
                                       (void*)&pin_params,
                                       msg_ptr);
  }
  if (res < 0)
  {
    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = RIL_UIM_E_MODEM_ERR;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
  }
} /* qcril_uim_request_set_pin_status */


/*===========================================================================

  FUNCTION:  qcril_uim_request_get_pin_status

===========================================================================*/
/*!
    @brief
    Handles QCRIL_UIM_EVT_INTERNAL_MMGSDI_GET_PIN1_STATUS requests
    from the framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_get_pin_status
(
  const std::shared_ptr<UimGetFacilityLockRequestMsg> req_ptr
)
{
  RIL_UIM_Errno             err         = RIL_UIM_E_SUCCESS;
  int                       ret_value   = 0;
  qmi_uim_pin_status_type   pin1_status = QMI_UIM_PIN_STATE_UNKNOWN;
  auto                      rsp_ptr     = std::make_shared<RIL_UIM_GetFacilityLockResponseMsg>();

  if(req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot process response");
    QCRIL_ASSERT(0);
    return;
  }

  QCRIL_LOG_INFO( "qcril_uim_request_get_pin_status\n");

  /* Fetch PIN1 or UPIN status for the specified App */
  err = qcril_uim_extract_pin1_status(qmi_ril_get_process_instance_id(),
                                      req_ptr->get_aid().c_str(),
                                      &pin1_status);
  if (err != RIL_UIM_E_SUCCESS)
  {
    if (rsp_ptr != NULL)
    {
      rsp_ptr->ril_err = err;
    }
    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  switch(pin1_status)
  {
    case QMI_UIM_PIN_STATE_ENABLED_NOT_VERIFIED:
    case QMI_UIM_PIN_STATE_ENABLED_VERIFIED:
    case QMI_UIM_PIN_STATE_BLOCKED:
    case QMI_UIM_PIN_STATE_PERM_BLOCKED:
      QCRIL_LOG_INFO( "PIN enabled\n" );
      ret_value = 1;
      break;
    case QMI_UIM_PIN_STATE_DISABLED:
      QCRIL_LOG_INFO( "PIN disabled\n" );
      ret_value = 0;
      break;
    default:
      QCRIL_LOG_INFO( "Unknown pin status 0x%x \n", pin1_status);
      err = RIL_UIM_E_INVALID_SIM_STATE;
      break;
  }

  if (rsp_ptr != NULL)
  {
    rsp_ptr->ril_err = err;
    rsp_ptr->lock_status = ret_value;
  }
  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_request_get_pin_status */


/*===========================================================================

  FUNCTION:  qcril_uim_convert_perso_feature_id_to_perso_feature_status

===========================================================================*/
/*!
    @brief
    Converts personalization feature ID to personalization feature status

    @return
    None
*/
/*=========================================================================*/
static boolean qcril_uim_convert_perso_feature_id_to_perso_feature_status
(
  qmi_uim_perso_feature_id_type      perso_feature_id,
  qmi_uim_perso_feature_status_type *perso_feature_status_ptr
)
{
  boolean status = TRUE;

  if(perso_feature_status_ptr == NULL)
  {
    return FALSE;
  }

  switch(perso_feature_id)
  {
    case QMI_UIM_PERSO_FEATURE_GW_NW:
      *perso_feature_status_ptr = QMI_UIM_PERSO_FEATURE_STATUS_GW_NW;
      break;
    case QMI_UIM_PERSO_FEATURE_GW_NS:
      *perso_feature_status_ptr = QMI_UIM_PERSO_FEATURE_STATUS_GW_NS;
      break;
    case QMI_UIM_PERSO_FEATURE_GW_SP:
      *perso_feature_status_ptr = QMI_UIM_PERSO_FEATURE_STATUS_GW_SP;
      break;
    case QMI_UIM_PERSO_FEATURE_GW_CP:
      *perso_feature_status_ptr = QMI_UIM_PERSO_FEATURE_STATUS_GW_CP;
      break;
    case QMI_UIM_PERSO_FEATURE_GW_SIM:
      *perso_feature_status_ptr = QMI_UIM_PERSO_FEATURE_STATUS_GW_SIM;
      break;
    case QMI_UIM_PERSO_FEATURE_1X_NW1:
      *perso_feature_status_ptr = QMI_UIM_PERSO_FEATURE_STATUS_1X_NW1;
      break;
    case QMI_UIM_PERSO_FEATURE_1X_NW2:
      *perso_feature_status_ptr = QMI_UIM_PERSO_FEATURE_STATUS_1X_NW2;
      break;
    case QMI_UIM_PERSO_FEATURE_1X_HRPD:
      *perso_feature_status_ptr = QMI_UIM_PERSO_FEATURE_STATUS_1X_HRPD;
      break;
    case QMI_UIM_PERSO_FEATURE_1X_SP:
      *perso_feature_status_ptr = QMI_UIM_PERSO_FEATURE_STATUS_1X_SP;
      break;
    case QMI_UIM_PERSO_FEATURE_1X_CP:
      *perso_feature_status_ptr = QMI_UIM_PERSO_FEATURE_STATUS_1X_CP;
      break;
     case QMI_UIM_PERSO_FEATURE_1X_RUIM:
      *perso_feature_status_ptr = QMI_UIM_PERSO_FEATURE_STATUS_1X_RUIM;
      break;
    case QMI_UIM_PERSO_FEATURE_GW_SPN:
      *perso_feature_status_ptr = QMI_UIM_PERSO_FEATURE_STATUS_GW_SPN;
      break;
    case QMI_UIM_PERSO_FEATURE_GW_SP_EHPLMN:
      *perso_feature_status_ptr = QMI_UIM_PERSO_FEATURE_STATUS_GW_SP_EHPLMN;
      break;
    case QMI_UIM_PERSO_FEATURE_GW_ICCID:
      *perso_feature_status_ptr = QMI_UIM_PERSO_FEATURE_STATUS_GW_ICCID;
      break;
    case QMI_UIM_PERSO_FEATURE_GW_IMPI:
      *perso_feature_status_ptr = QMI_UIM_PERSO_FEATURE_STATUS_GW_IMPI;
      break;
    case QMI_UIM_PERSO_FEATURE_GW_NS_SP:
      *perso_feature_status_ptr = QMI_UIM_PERSO_FEATURE_STATUS_GW_NS_SP;
      break;
    default:
      status = FALSE;
      break;
  }

  return status;
} /* qcril_uim_convert_perso_feature_id_to_perso_feature_status */


/*===========================================================================

  FUNCTION:  qcril_uim_get_perso_retries

===========================================================================*/
/*!
    @brief
    Gets the personalization retries for the feature by sending
    QMI_UIM_GET_CONFIGURATION command to modem.

    @return
    None
*/
/*=========================================================================*/
static RIL_UIM_Errno qcril_uim_get_perso_retries
(
  qmi_uim_perso_feature_id_type    perso_feature,
  int                            * num_retries_ptr
)
{
  int                                    ret    = 0;
  uint8                                  i      = 0;
  RIL_UIM_Errno                              errval = RIL_UIM_E_INTERNAL_ERR;
  qmi_uim_get_configuration_params_type  get_config_params;
  qmi_uim_rsp_data_type                  rsp_data;

  if (num_retries_ptr == NULL)
  {
    return RIL_UIM_E_INTERNAL_ERR;
  }

  *num_retries_ptr = 0;

  memset(&get_config_params, 0x00, sizeof(get_config_params));
  memset(&rsp_data, 0x00, sizeof(rsp_data));

  get_config_params.perso_status = QMI_UIM_TRUE;
  ret = qcril_uim_send_qmi_sync_msg(QCRIL_UIM_REQUEST_GET_CONFIGURATION,
                                    &get_config_params,
                                    &rsp_data);

  if (ret != 0)
  {
    return qcril_uim_convert_err_value(ret);
  }

  for (i = 0;
       i < rsp_data.rsp_data.get_configuration_rsp.perso_status_len && i < QMI_UIM_MAX_PERSO_FEATURES;
       i++)
  {
    if (rsp_data.rsp_data.get_configuration_rsp.perso_status[i].feature == perso_feature)
    {
      *num_retries_ptr = rsp_data.rsp_data.get_configuration_rsp.perso_status[i].verify_left;
      errval = RIL_UIM_E_SUCCESS;
      break;
    }
  }

  return errval;
} /* qcril_uim_get_perso_retries */


/*===========================================================================

  FUNCTION:  qcril_uim_request_enter_perso_key

===========================================================================*/
/*!
    @brief
    Handles RIL_REQUEST_ENTER_NETWORK_DEPERSONALIZATION or
    QCRIL_UIM_EVT_HOOK_ENTER_DEPERSONALIZATION_REQ requests from the framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_enter_perso_key
(
  const std::shared_ptr<UimEnterDePersoRequestMsg> msg_ptr
)
{
  int                                     res                    = -1;
  qmi_uim_depersonalization_params_type   perso_params           = {};
  auto rsp_ptr = std::make_shared<RIL_UIM_PersoResponse>();

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if(msg_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot process response");
    QCRIL_ASSERT(0);
    return;
  }

  /* Update the feature type */
  switch (msg_ptr->get_perso_type())
  {
    case RIL_UIM_PERSOSUBSTATE_SIM_NETWORK:
      perso_params.perso_feature = QMI_UIM_PERSO_FEATURE_GW_NW;
      break;
    case RIL_UIM_PERSOSUBSTATE_SIM_NETWORK_SUBSET:
      perso_params.perso_feature = QMI_UIM_PERSO_FEATURE_GW_NS;
      break;
    case RIL_UIM_PERSOSUBSTATE_SIM_CORPORATE:
      perso_params.perso_feature = QMI_UIM_PERSO_FEATURE_GW_CP;
      break;
    case RIL_UIM_PERSOSUBSTATE_SIM_SERVICE_PROVIDER:
      perso_params.perso_feature = QMI_UIM_PERSO_FEATURE_GW_SP;
      break;
    case RIL_UIM_PERSOSUBSTATE_SIM_SPN:
      perso_params.perso_feature = QMI_UIM_PERSO_FEATURE_GW_SPN;
      break;
    case RIL_UIM_PERSOSUBSTATE_SIM_SP_EHPLMN:
      perso_params.perso_feature = QMI_UIM_PERSO_FEATURE_GW_SP_EHPLMN;
      break;
    case RIL_UIM_PERSOSUBSTATE_SIM_ICCID:
      perso_params.perso_feature = QMI_UIM_PERSO_FEATURE_GW_ICCID;
      break;
    case RIL_UIM_PERSOSUBSTATE_SIM_IMPI:
      perso_params.perso_feature = QMI_UIM_PERSO_FEATURE_GW_IMPI;
      break;
    case RIL_UIM_PERSOSUBSTATE_SIM_SIM:
      perso_params.perso_feature = QMI_UIM_PERSO_FEATURE_GW_SIM;
      break;
    case RIL_UIM_PERSOSUBSTATE_SIM_NS_SP:
      perso_params.perso_feature = QMI_UIM_PERSO_FEATURE_GW_NS_SP;
      break;
    case RIL_UIM_PERSOSUBSTATE_RUIM_NETWORK1:
      perso_params.perso_feature = QMI_UIM_PERSO_FEATURE_1X_NW1;
      break;
    case RIL_UIM_PERSOSUBSTATE_RUIM_NETWORK2:
      perso_params.perso_feature = QMI_UIM_PERSO_FEATURE_1X_NW2;
      break;
    case RIL_UIM_PERSOSUBSTATE_RUIM_HRPD:
      perso_params.perso_feature = QMI_UIM_PERSO_FEATURE_1X_HRPD;
      break;
    case RIL_UIM_PERSOSUBSTATE_RUIM_CORPORATE:
      perso_params.perso_feature = QMI_UIM_PERSO_FEATURE_1X_CP;
      break;
    case RIL_UIM_PERSOSUBSTATE_RUIM_SERVICE_PROVIDER:
      perso_params.perso_feature = QMI_UIM_PERSO_FEATURE_1X_SP;
      break;
    case RIL_UIM_PERSOSUBSTATE_RUIM_RUIM:
      perso_params.perso_feature = QMI_UIM_PERSO_FEATURE_1X_RUIM;
      break;
    default:
      /* Invalid feature ID */
      QCRIL_LOG_ERROR( " Invalid perso feature from RIL Request ID %d\n",
                       msg_ptr->get_perso_type());
      if (rsp_ptr != NULL)
      {
        rsp_ptr->err = RIL_UIM_E_INVALID_ARGUMENTS;
      }
      msg_ptr->sendResponse(msg_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
      return;
  }

  /* Return the number of remaining retries if perso key is empty */
  if(msg_ptr->get_unlock_code().length() == 0)
  {
    uint8                             slot                 = qmi_ril_get_process_instance_id();
    int                               num_retries          = 0;
    uint8                             i                    = 0;
    boolean                           perso_convert_status = FALSE;
    qmi_uim_perso_feature_status_type perso_feature_status = QMI_UIM_PERSO_FEATURE_STATUS_UNKNOWN;

    perso_convert_status
      = qcril_uim_convert_perso_feature_id_to_perso_feature_status(perso_params.perso_feature,
                                                                   &perso_feature_status);
    if(perso_convert_status && slot < QMI_UIM_MAX_CARD_COUNT)
    {
      for(i = 0; i < qcril_uim.card_status.card[slot].num_app; i++)
      {
        if((i != QCRIL_UIM_INVALID_APP_INDEX_VALUE) &&
           (qcril_uim.card_status.card[slot].application[i].app_state == QMI_UIM_APP_STATE_PERSO) &&
           (qcril_uim.card_status.card[slot].application[i].perso_feature == perso_feature_status) &&
           (qcril_uim.card_status.card[slot].application[i].perso_state == QMI_UIM_PERSO_STATE_CODE_REQUIRED))
        {
          if (rsp_ptr != NULL)
          {
            rsp_ptr->err = RIL_UIM_E_SUCCESS;
            rsp_ptr->no_of_retries = qcril_uim.card_status.card[slot].application[i].perso_retries;
          }
          msg_ptr->sendResponse(msg_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
          return;
        }
      }
    }

    /* If perso retries are not available in QCRIL, retrieve them from modem
       using Get Configuration command. */
    if (qcril_uim_get_perso_retries(perso_params.perso_feature, &num_retries) == RIL_UIM_E_SUCCESS)
    {
      if (rsp_ptr != NULL)
      {
        rsp_ptr->err = RIL_UIM_E_SUCCESS;
        rsp_ptr->no_of_retries = num_retries;
      }
      msg_ptr->sendResponse(msg_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
      return;
    }

    QCRIL_LOG_ERROR("Unable to find app that requires key for perso feature 0x%x",
                     perso_params.perso_feature);

    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = RIL_UIM_E_INTERNAL_ERR;
    }
    msg_ptr->sendResponse(msg_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  /* Perso operation */
  perso_params.perso_operation = QMI_UIM_PERSO_OP_DEACTIVATE;

  /* New PIN value */
  perso_params.ck_data.data_ptr = (uint8_t *)msg_ptr->get_unlock_code().c_str();
  perso_params.ck_data.data_len = msg_ptr->get_unlock_code().length();

   /* Proceed with depersonalization */
  res = qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_DEPERSO,
                                     (void*)&perso_params,
                                     msg_ptr);
  if (res < 0)
  {
    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = RIL_UIM_E_MODEM_ERR;
    }
    msg_ptr->sendResponse(msg_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
  }
} /* qcril_uim_request_enter_perso_key */


/*===========================================================================

  FUNCTION:  qcril_uim_request_perso_reactivate

===========================================================================*/
/*!
    @brief
    Handles QCRIL_UIM_EVT_HOOK_PERSONALIZATION_REACTIVATE_REQ requests
    from the framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_perso_reactivate
(
  const std::shared_ptr<UimPersoRequestMsg> req_ptr
)
{
  int                                    res                  = 0;
  qmi_uim_personalization_params_type    perso_params         = {};
  RIL_UIM_PersonalizationReq           * reactivate_req_ptr   = NULL;
  RIL_UIM_Errno                          ril_err              = RIL_UIM_E_INTERNAL_ERR;
  std::shared_ptr<RIL_UIM_PersoResponse> rsp_ptr              = NULL;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  /* Sanity checks */
  if (req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot process response");
    return;
  }

  reactivate_req_ptr = req_ptr->get_data();

  do
  {
    if (reactivate_req_ptr == nullptr)
    {
      break;
    }

    QCRIL_LOG_INFO( "qcril_uim_request_perso_reactivate for persoType (0x%x)\n",
                    reactivate_req_ptr->persoType );

    /* Reactivate Perso feature */
    switch (reactivate_req_ptr->persoType)
    {
      case RIL_UIM_PERSOSUBSTATE_SIM_NETWORK:
        perso_params.reactivate_feature = QMI_UIM_PERSO_FEATURE_GW_NW;
        break;
      case RIL_UIM_PERSOSUBSTATE_SIM_NETWORK_SUBSET:
        perso_params.reactivate_feature = QMI_UIM_PERSO_FEATURE_GW_NS;
        break;
      case RIL_UIM_PERSOSUBSTATE_SIM_CORPORATE:
        perso_params.reactivate_feature = QMI_UIM_PERSO_FEATURE_GW_CP;
        break;
      case RIL_UIM_PERSOSUBSTATE_SIM_SERVICE_PROVIDER:
        perso_params.reactivate_feature = QMI_UIM_PERSO_FEATURE_GW_SP;
        break;
      case RIL_UIM_PERSOSUBSTATE_SIM_SPN:
        perso_params.reactivate_feature = QMI_UIM_PERSO_FEATURE_GW_SPN;
        break;
      case RIL_UIM_PERSOSUBSTATE_SIM_SP_EHPLMN:
        perso_params.reactivate_feature = QMI_UIM_PERSO_FEATURE_GW_SP_EHPLMN;
        break;
      case RIL_UIM_PERSOSUBSTATE_SIM_ICCID:
        perso_params.reactivate_feature = QMI_UIM_PERSO_FEATURE_GW_ICCID;
        break;
      case RIL_UIM_PERSOSUBSTATE_SIM_IMPI:
        perso_params.reactivate_feature = QMI_UIM_PERSO_FEATURE_GW_IMPI;
        break;
      case RIL_UIM_PERSOSUBSTATE_SIM_SIM:
        perso_params.reactivate_feature = QMI_UIM_PERSO_FEATURE_GW_SIM;
        break;
      case RIL_UIM_PERSOSUBSTATE_SIM_NS_SP:
        perso_params.reactivate_feature = QMI_UIM_PERSO_FEATURE_GW_NS_SP;
        break;
      case RIL_UIM_PERSOSUBSTATE_RUIM_NETWORK1:
        perso_params.reactivate_feature = QMI_UIM_PERSO_FEATURE_1X_NW1;
        break;
      case RIL_UIM_PERSOSUBSTATE_RUIM_NETWORK2:
        perso_params.reactivate_feature = QMI_UIM_PERSO_FEATURE_1X_NW2;
        break;
      case RIL_UIM_PERSOSUBSTATE_RUIM_HRPD:
        perso_params.reactivate_feature = QMI_UIM_PERSO_FEATURE_1X_HRPD;
        break;
      case RIL_UIM_PERSOSUBSTATE_RUIM_CORPORATE:
        perso_params.reactivate_feature = QMI_UIM_PERSO_FEATURE_1X_CP;
        break;
      case RIL_UIM_PERSOSUBSTATE_RUIM_SERVICE_PROVIDER:
        perso_params.reactivate_feature = QMI_UIM_PERSO_FEATURE_1X_SP;
        break;
      case RIL_UIM_PERSOSUBSTATE_RUIM_RUIM:
        perso_params.reactivate_feature = QMI_UIM_PERSO_FEATURE_1X_RUIM;
        break;
      default:
        /* Invalid feature ID */
        ril_err = RIL_UIM_E_REQUEST_NOT_SUPPORTED;
        break;
    }

    /* CK value */
    perso_params.ck_data.data_ptr = (uint8_t *)reactivate_req_ptr->controlKey.c_str();
    perso_params.ck_data.data_len = reactivate_req_ptr->controlKey.length();

    /* Proceed with depersonalization */
    res = qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_PERSO,
                                       (void*)&perso_params,
                                       req_ptr);
    if (res >= 0)
    {
      return;
    }
    ril_err = RIL_UIM_E_MODEM_ERR;
  } while(0);

  rsp_ptr = std::make_shared<RIL_UIM_PersoResponse>();

  if (rsp_ptr != NULL)
  {
    rsp_ptr->err = ril_err;
  }
  
  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_request_perso_reactivate */


/*===========================================================================

  FUNCTION:  qcril_uim_request_perso_status

===========================================================================*/
/*!
    @brief
    Handles QCRIL_UIM_EVT_HOOK_PERSONALIZATION_STATUS_REQ requests
    from the framework

    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_perso_status
(
  const std::shared_ptr<UimGetPersoStatusRequestMsg> req_ptr
)
{
  int                                    ret               = 0;
  uint8                                  i                 = 0;
  qmi_uim_rsp_data_type                  rsp_data          = {};
  qmi_uim_get_configuration_params_type  get_config_params = {};
  auto                                   rsp_ptr           =
          std::make_shared<RIL_UIM_PersonalizationStatusResp>();

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  /* Sanity checks */
  if (req_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid input, cannot process response");
    return;
  }

  if (rsp_ptr == NULL)
  {
    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  get_config_params.perso_status = QMI_UIM_TRUE;
  ret = qcril_uim_send_qmi_sync_msg(QCRIL_UIM_REQUEST_GET_CONFIGURATION,
                                    &get_config_params,
                                    &rsp_data);

  if ((ret != 0) || (!rsp_data.rsp_data.get_configuration_rsp.perso_status_valid))
  {
    rsp_ptr->err = RIL_UIM_E_INTERNAL_ERR;

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  for (i = 0;
       i < rsp_data.rsp_data.get_configuration_rsp.perso_status_len && i < QMI_UIM_MAX_PERSO_FEATURES;
       i++)
  {
    switch (rsp_data.rsp_data.get_configuration_rsp.perso_status[i].feature)
    {
      case QMI_UIM_PERSO_FEATURE_GW_NW:
        rsp_ptr->has_gwNWPersoStatus = TRUE;
        rsp_ptr->gwNWPersoStatus.verifyAttempts =
          rsp_data.rsp_data.get_configuration_rsp.perso_status[i].verify_left;
        break;

      case QMI_UIM_PERSO_FEATURE_GW_NS:
        rsp_ptr->has_gwNWSubsetPersoStatus = TRUE;
        rsp_ptr->gwNWSubsetPersoStatus.verifyAttempts =
          rsp_data.rsp_data.get_configuration_rsp.perso_status[i].verify_left;
        break;

      case QMI_UIM_PERSO_FEATURE_GW_SP:
        rsp_ptr->has_gwSPPersoStatus = TRUE;
        rsp_ptr->gwSPPersoStatus.verifyAttempts =
          rsp_data.rsp_data.get_configuration_rsp.perso_status[i].verify_left;
        break;

      case QMI_UIM_PERSO_FEATURE_GW_CP:
        rsp_ptr->has_gwCPPersoStatus = TRUE;
        rsp_ptr->gwCPPersoStatus.verifyAttempts =
          rsp_data.rsp_data.get_configuration_rsp.perso_status[i].verify_left;
        break;

      case QMI_UIM_PERSO_FEATURE_GW_SPN:
        rsp_ptr->has_gwSPNPersoStatus = TRUE;
        rsp_ptr->gwSPNPersoStatus.verifyAttempts =
          rsp_data.rsp_data.get_configuration_rsp.perso_status[i].verify_left;
        break;

      case QMI_UIM_PERSO_FEATURE_GW_SP_EHPLMN:
        rsp_ptr->has_gwSPEhplmnPersoStatus = TRUE;
        rsp_ptr->gwSPEhplmnPersoStatus.verifyAttempts =
          rsp_data.rsp_data.get_configuration_rsp.perso_status[i].verify_left;
        break;

      case QMI_UIM_PERSO_FEATURE_GW_ICCID:
        rsp_ptr->has_gwICCIDPersoStatus = TRUE;
        rsp_ptr->gwICCIDPersoStatus.verifyAttempts =
          rsp_data.rsp_data.get_configuration_rsp.perso_status[i].verify_left;
        break;

      case QMI_UIM_PERSO_FEATURE_GW_IMPI:
        rsp_ptr->has_gwIMPIPersoStatus = TRUE;
        rsp_ptr->gwIMPIPersoStatus.verifyAttempts =
          rsp_data.rsp_data.get_configuration_rsp.perso_status[i].verify_left;
        break;

      case QMI_UIM_PERSO_FEATURE_GW_SIM:
        rsp_ptr->has_gwSIMPersoStatus = TRUE;
        rsp_ptr->gwSIMPersoStatus.verifyAttempts =
          rsp_data.rsp_data.get_configuration_rsp.perso_status[i].verify_left;
        break;
      case QMI_UIM_PERSO_FEATURE_GW_NS_SP:
        rsp_ptr->has_gwNSSPPersoStatus = TRUE;
        rsp_ptr->gwNSSPPersoStatus.verifyAttempts =
          rsp_data.rsp_data.get_configuration_rsp.perso_status[i].verify_left;
        break;

      case QMI_UIM_PERSO_FEATURE_1X_NW1:
        rsp_ptr->has_gw1xNWType1PersoStatus = TRUE;
        rsp_ptr->gw1xNWType1PersoStatus.verifyAttempts =
          rsp_data.rsp_data.get_configuration_rsp.perso_status[i].verify_left;
        break;

      case QMI_UIM_PERSO_FEATURE_1X_NW2:
        rsp_ptr->has_gw1xNWType2PersoStatus = TRUE;
        rsp_ptr->gw1xNWType2PersoStatus.verifyAttempts =
          rsp_data.rsp_data.get_configuration_rsp.perso_status[i].verify_left;
        break;

      case QMI_UIM_PERSO_FEATURE_1X_RUIM:
        rsp_ptr->has_gw1xRUIMPersoStatus = TRUE;
        rsp_ptr->gw1xRUIMPersoStatus.verifyAttempts =
          rsp_data.rsp_data.get_configuration_rsp.perso_status[i].verify_left;
        break;

      default:
        QCRIL_LOG_INFO("Unsupported perso feature, skipping: 0x%x",
                         rsp_data.rsp_data.get_configuration_rsp.perso_status[i].feature);
        break;
    }
  }

  rsp_ptr->err = RIL_UIM_E_SUCCESS;
  req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
} /* qcril_uim_request_perso_status */


/*=========================================================================
  FUNCTION:  qcril_uim_request_remote_sim_lock_unlock
===========================================================================*/
/*!
    @brief
    Handles QCRIL_UIM_EVT_HOOK_REMOTE_SIM_LOCK_UNLOCK request from QCRIL.
    @return
    None
*/
/*=========================================================================*/
void qcril_uim_request_remote_sim_lock_unlock
(
  const std::shared_ptr<UimRemoteSimlockRequestMsg> req_ptr
)
{
  qmi_uim_remote_unlock_params_type      remote_unlock_params  = {};
  int                                    ret                   = -1;
  std::shared_ptr<RIL_UIM_RemoteSimlockResponse> rsp_ptr       = std::make_shared<RIL_UIM_RemoteSimlockResponse>();
  RIL_UIM_RemoteSimlockResponseType      response              = RIL_UIM_REMOTE_SIMLOCK_RESP_FAILURE;

  if(req_ptr == NULL)
  {
    return;
  }

  QCRIL_LOG_INFO("qcril_uim_remote_lock_unlock operation type (0x%x)\n",
                  req_ptr->get_op_type());

  switch(req_ptr->get_op_type())
  {
    case RIL_UIM_REMOTE_SIMLOCK_GENERATE_ENCRYPTED_KEY:
      remote_unlock_params.operation = QMI_UIM_REMOTE_UNLOCK_RETRIEVE_ENCRYPTED_KEY;
      break;
    case RIL_UIM_REMOTE_SIMLOCK_PROCESS_SIMLOCK_DATA:
      remote_unlock_params.operation = QMI_UIM_REMOTE_UNLOCK_PROCESS_SIMLOCK_DATA;
      break;
    case RIL_UIM_REMOTE_SIMLOCK_GENERATE_HMAC:
      remote_unlock_params.operation = QMI_UIM_REMOTE_UNLOCK_GENERATE_MAC;
      break;
    case RIL_UIM_REMOTE_SIMLOCK_GET_MAX_SUPPORTED_VERSION:
      remote_unlock_params.operation = QMI_UIM_REMOTE_UNLOCK_GET_MAX_SUPPORTED_VERSION;
      break;
    case RIL_UIM_REMOTE_SIMLOCK_GENERATE_BLOB:
      remote_unlock_params.operation = QMI_UIM_REMOTE_UNLOCK_GENERATE_BLOB;
      break;
    case RIL_UIM_REMOTE_SIMLOCK_START_UNLOCK_TIMER:
      remote_unlock_params.operation = QMI_UIM_REMOTE_UNLOCK_START_TIMER;
      break;
    case RIL_UIM_REMOTE_SIMLOCK_STOP_UNLOCK_TIMER:
      remote_unlock_params.operation = QMI_UIM_REMOTE_UNLOCK_STOP_TIMER;
      break;
    case RIL_UIM_REMOTE_SIMLOCK_GET_STATUS:
      {
        qmi_uim_get_configuration_params_type  get_config_params = {};
        qmi_uim_rsp_data_type                  rsp_data          = {};
        RIL_UIM_RemoteSimlockStatus            status            = {};

        get_config_params.perso_status = QMI_UIM_TRUE;

        ret = qcril_uim_send_qmi_sync_msg(QCRIL_UIM_REQUEST_GET_CONFIGURATION,
                                          &get_config_params,
                                          &rsp_data);
        if (ret == 0)
        {
          uint32  i = 0;

          status.state = RIL_UIM_REMOTE_SIMLOCK_STATE_PERMANENT_UNLOCK;
          if (rsp_data.rsp_data.get_configuration_rsp.perso_status_valid)
          {
            for(i = 0; i < rsp_data.rsp_data.get_configuration_rsp.perso_status_len &&
                       i < QMI_UIM_MAX_PERSO_FEATURES; i++)
            {
              if(rsp_data.rsp_data.get_configuration_rsp.perso_status[i].feature == QMI_UIM_PERSO_FEATURE_GW_SP ||
                 rsp_data.rsp_data.get_configuration_rsp.perso_status[i].feature == QMI_UIM_PERSO_FEATURE_GW_NW ||
                 rsp_data.rsp_data.get_configuration_rsp.perso_status[i].feature == QMI_UIM_PERSO_FEATURE_GW_NS ||
                 rsp_data.rsp_data.get_configuration_rsp.perso_status[i].feature == QMI_UIM_PERSO_FEATURE_GW_CP ||
                 rsp_data.rsp_data.get_configuration_rsp.perso_status[i].feature == QMI_UIM_PERSO_FEATURE_GW_SPN ||
                 rsp_data.rsp_data.get_configuration_rsp.perso_status[i].feature == QMI_UIM_PERSO_FEATURE_GW_SP_EHPLMN ||
                 rsp_data.rsp_data.get_configuration_rsp.perso_status[i].feature == QMI_UIM_PERSO_FEATURE_GW_ICCID ||
                 rsp_data.rsp_data.get_configuration_rsp.perso_status[i].feature == QMI_UIM_PERSO_FEATURE_GW_IMPI)
              {
                status.state = RIL_UIM_REMOTE_SIMLOCK_STATE_LOCKED;
                break;
              }
            }
          }
          if (rsp_data.rsp_data.get_configuration_rsp.temp_perso_status_len != 0)
          {
            for(i = 0; i < rsp_data.rsp_data.get_configuration_rsp.temp_perso_status_len &&
                       i < QMI_UIM_MAX_PERSO_FEATURES; i++)
            {
              if(rsp_data.rsp_data.get_configuration_rsp.temp_perso_status[i].feature == QMI_UIM_PERSO_FEATURE_GW_SP ||
                 rsp_data.rsp_data.get_configuration_rsp.temp_perso_status[i].feature == QMI_UIM_PERSO_FEATURE_GW_NW ||
                 rsp_data.rsp_data.get_configuration_rsp.temp_perso_status[i].feature == QMI_UIM_PERSO_FEATURE_GW_NS ||
                 rsp_data.rsp_data.get_configuration_rsp.temp_perso_status[i].feature == QMI_UIM_PERSO_FEATURE_GW_CP ||
                 rsp_data.rsp_data.get_configuration_rsp.temp_perso_status[i].feature == QMI_UIM_PERSO_FEATURE_GW_SPN ||
                 rsp_data.rsp_data.get_configuration_rsp.temp_perso_status[i].feature == QMI_UIM_PERSO_FEATURE_GW_SP_EHPLMN ||
                 rsp_data.rsp_data.get_configuration_rsp.temp_perso_status[i].feature == QMI_UIM_PERSO_FEATURE_GW_ICCID ||
                 rsp_data.rsp_data.get_configuration_rsp.temp_perso_status[i].feature == QMI_UIM_PERSO_FEATURE_GW_IMPI)
              {
                status.state = RIL_UIM_REMOTE_SIMLOCK_STATE_TEMPERORY_UNLOCK;
                status.unlockTime =
                  rsp_data.rsp_data.get_configuration_rsp.temp_perso_status[i].unlock_time_left;
                break;
              }
            }
          }
          response = RIL_UIM_REMOTE_SIMLOCK_RESP_SUCCESS;
        }

        if (rsp_ptr != NULL)
        {
          rsp_ptr->rsp_type = response;
          rsp_ptr->op_type = req_ptr->get_op_type();
          rsp_ptr->status = status;
        }
        req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
      }
      return;
    default:
      if (rsp_ptr != NULL)
      {
        rsp_ptr->rsp_type = response;
        rsp_ptr->op_type = req_ptr->get_op_type();
      }
      req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
      return;
  }

  if (!req_ptr->get_simlock_data().empty())
  {
    remote_unlock_params.simlock_data.data_ptr = (uint8_t *)req_ptr->get_simlock_data().data();
    remote_unlock_params.simlock_data.data_len = req_ptr->get_simlock_data().size();
  }

  ret = qcril_uim_send_qmi_async_msg(QCRIL_UIM_REQUEST_REMOTE_UNLOCK,
                                     &remote_unlock_params,
                                     req_ptr);

  if (ret < 0)
  {
    if (rsp_ptr != NULL)
    {
      rsp_ptr->rsp_type = response;
      rsp_ptr->op_type = req_ptr->get_op_type();
    }
    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
  }
} /* qcril_uim_request_remote_sim_lock_unlock */


/*===========================================================================

  FUNCTION:  qcril_uim_process_simlock_temp_unlock_ind

===========================================================================*/
/*!
    @brief
    Main function for processing QMI SIMLOCK temporary unlock indication.

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_process_simlock_temp_unlock_ind
(
  const qcril_uim_indication_params_type  * ind_param_ptr
)
{
  uint8               slot = 0;
  char                temp_buff[100] = {0};

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (ind_param_ptr == NULL)
  {
    QCRIL_LOG_ERROR("Invalid inputs, cannot proceed");
    QCRIL_ASSERT(0);
    return;
  }

  slot = qmi_ril_get_process_instance_id();
  if( slot >= QMI_UIM_MAX_CARD_COUNT )
  {
    QCRIL_LOG_ERROR("Invalid slot 0x%x for 0x%x", slot);
    return;
  }

  (void)snprintf(temp_buff, 99, "%s_%d", QCRIL_UIM_PROP_TEMPARORY_UNLOCK_STATUS, slot);

  QCRIL_LOG_INFO( "RSU status:0x%x and received status:0x%x \n",
           qcril_uim.temp_unlock_status,
           ind_param_ptr->ind_data.simlock_temp_unlock_ind.temporay_unlock_status[slot]);

  if (!ind_param_ptr->ind_data.simlock_temp_unlock_ind.temporay_unlock_status[slot] &&
      qcril_uim.temp_unlock_status)
  {
    auto ind_ptr = std::make_shared<UimSimlockTempUnlockExpireInd>();

    if (ind_ptr != NULL)
    {
      ind_ptr->broadcast();
    }
    QCRIL_LOG_INFO( "QCRIL_UIM_EVT_HOOK_UNSOL_SIMLOCK_TEMP_UNLOCK_EXPIRED is sent \n");
  }
  if (ind_param_ptr->ind_data.simlock_temp_unlock_ind.temporay_unlock_status[slot] !=
      qcril_uim.temp_unlock_status)
  {
    std::shared_ptr<UimSetRilPropertySyncMsg> prop_ptr = NULL;
    std::shared_ptr<void>                     rsp_ptr  = NULL;

    qcril_uim.temp_unlock_status =
      ind_param_ptr->ind_data.simlock_temp_unlock_ind.temporay_unlock_status[slot];
    switch(qcril_uim.temp_unlock_status)
    {
      case TRUE:
        prop_ptr = std::make_shared<UimSetRilPropertySyncMsg>
          (temp_buff, QCRIL_UIM_PROP_TEMPARORY_UNLOCK_STATUS_TRUE);

        if (prop_ptr != NULL)
        {
          prop_ptr->dispatchSync(rsp_ptr);
        }
        break;
      case FALSE:
        prop_ptr = std::make_shared<UimSetRilPropertySyncMsg>
          (temp_buff, QCRIL_UIM_PROP_TEMPARORY_UNLOCK_STATUS_FALSE);

        if (prop_ptr != NULL)
        {
          prop_ptr->dispatchSync(rsp_ptr);
        }
        break;
    }
  }
} /* qcril_uim_process_simlock_temp_unlock_ind */


/*===========================================================================

  FUNCTION:  qcril_uim_check_and_send_temp_unlock_expiry_ind

===========================================================================*/
/*!
    @brief
    Function to check the temperory unlock status and send expiry indication

    @return
    None.
*/
/*=========================================================================*/
void qcril_uim_check_and_send_temp_unlock_expiry_ind
(
  void
)
{

  int                                       ret            = 0;
  qmi_uim_get_configuration_params_type     get_config_params = {};
  qmi_uim_rsp_data_type                     rsp_data       = {};
  uint8                                     slot           =  (uint8)qmi_ril_get_sim_slot();
  char                                      temp_buff[100] = {0};
  std::shared_ptr<UimSetRilPropertySyncMsg> prop_ptr       = NULL;
  std::shared_ptr<void>                     rsp_ptr        = NULL;

  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if( slot >= QMI_UIM_MAX_CARD_COUNT )
  {
    QCRIL_LOG_ERROR("Invalid slot 0x%x ", slot);
    return;
  }

  (void)snprintf(temp_buff, 99, "%s_%d", QCRIL_UIM_PROP_TEMPARORY_UNLOCK_STATUS, slot);

  get_config_params.perso_status = QMI_UIM_TRUE;
  ret = qcril_uim_send_qmi_sync_msg(QCRIL_UIM_REQUEST_GET_CONFIGURATION,
                                    &get_config_params,
                                    &rsp_data);

  if (ret != 0)
  {
    return;
  }

  QCRIL_LOG_INFO( "RSU status:0x%x and received status:0x%x \n",
           qcril_uim.temp_unlock_status,
           rsp_data.rsp_data.get_configuration_rsp.temp_unlock_status[slot]);

  if (!rsp_data.rsp_data.get_configuration_rsp.temp_unlock_status[slot] &&
      qcril_uim.temp_unlock_status)
  {
    auto ind_ptr = std::make_shared<UimSimlockTempUnlockExpireInd>();

    if (ind_ptr != NULL)
    {
      ind_ptr->broadcast();
    }
    QCRIL_LOG_INFO( "QCRIL_UIM_EVT_HOOK_UNSOL_SIMLOCK_TEMP_UNLOCK_EXPIRED is sent \n");
    qcril_uim.temp_unlock_status = QMI_UIM_FALSE;

    prop_ptr = std::make_shared<UimSetRilPropertySyncMsg>
      (temp_buff, QCRIL_UIM_PROP_TEMPARORY_UNLOCK_STATUS_FALSE);

    if (prop_ptr != NULL)
    {
      prop_ptr->dispatchSync(rsp_ptr);
    }
  }
  else if (!qcril_uim.temp_unlock_status &&
           rsp_data.rsp_data.get_configuration_rsp.temp_unlock_status[slot])
  {
    qcril_uim.temp_unlock_status = QMI_UIM_TRUE;

    prop_ptr = std::make_shared<UimSetRilPropertySyncMsg>
      (temp_buff, QCRIL_UIM_PROP_TEMPARORY_UNLOCK_STATUS_FALSE);

    if (prop_ptr != NULL)
    {
      prop_ptr->dispatchSync(rsp_ptr);
    }
  }
} /* qcril_uim_check_and_send_temp_unlock_expiry_ind */
