/******************************************************************************
#  Copyright (c) 2013, 2016-2017, 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
/******************************************************************************
  @file    qcril_qmi_imss_v02.c
  @brief   qcril qmi - IMS Setting

  DESCRIPTION
    Handles RIL requests, Callbacks, indications for QMI IMS Setting.

******************************************************************************/
#define TAG "QCRIL_IMSS"

#include "qcril_log.h"
#include <cutils/properties.h>
#include <string.h>
#include "qcrili.h"
#include "qcril_qmi_client.h"
#include "qcril_qmi_imss.h"
#include "qcril_qmi_imss_qmi.h"
#include "qcril_qmi_imss_v02.h"
#include "ip_multimedia_subsystem_settings_v01.h"
#include "qcril_reqlist.h"
#include "qcril_qmi_npb_utils.h"
#include "qcril_qmi_radio_config_imss.h"
#include "modules/nas/qcril_qmi_nas.h"
#include "modules/nas/qcril_arb.h"
#include <cutils/properties.h>
#include <framework/Log.h>

#include <interfaces/ims/ims.h>
#include <modules/ims/ImsModule.h>
#include <interfaces/ims/QcRilUnsolImsRttMessage.h>
#include "qcril_qmi_ims_utils.h"


extern struct ims_cached_info_type qcril_qmi_ims_cached_info;
extern boolean feature_voice_dom_pref_on_toggle_ims_cap;
extern boolean feature_disabled_modem_req;

void qcril_qmi_imss_ims_service_enable_config_ind_hdlr(void *ind_data_ptr);

RIL_Errno qcril_qmi_imss_send_set_ims_service_enable_config_req
(
 const ims_settings_set_ims_service_enable_config_req_msg_v01 *qmi_req,
 uint32 user_data,
 qmiAsyncCbType resp_cb
);

void qcril_qmi_imss_set_ims_service_enable_config_resp_ims_reg_change_hdlr
(
 unsigned int                 msg_id,
 std::shared_ptr<void>        resp_c_struct,
 unsigned int                 resp_c_struct_len,
 void                        *resp_cb_data,
 qmi_client_error_type        transp_err
);

qcril_ims_setting_wfc_status_type qcril_qmi_imss_convert_imss_to_ril_wfc_status_v02
(
  uint8_t wfc_status
);

qcril_ims_setting_wfc_preference_type qcril_qmi_imss_convert_imss_to_ril_wfc_preference_v02
(
  ims_settings_call_mode_option_enum_v01 wfc_preference
);

//===========================================================================
// qcril_qmi_imss_request_set_ims_registration_v02
//===========================================================================
void qcril_qmi_imss_request_set_ims_registration_v02(
    std::shared_ptr<QcRilRequestImsRegistrationChangeMessage> msg)
{
  qcril::interfaces::RegState imsRegState = qcril::interfaces::RegState::UNKNOWN;
  ims_settings_set_ims_service_enable_config_req_msg_v01 qmi_req;
  uint32 user_data;
  bool sendResponse = false;
  RIL_Errno errorCode = RIL_E_SUCCESS;
  uint16_t req_id = -1;
  bool doCleanup = false;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    imsRegState = msg->getRegState();
    //QCRIL_LOG_INFO("state: %d", imsRegState);

    if(feature_disabled_modem_req)
    {
      QCRIL_LOG_INFO("Modem IMS config is disabled. Respond to Telephony with success");
      errorCode = RIL_E_SUCCESS;
      sendResponse = true;
      break;
    }

    if(imsRegState == qcril::interfaces::RegState::UNKNOWN)
    {
      QCRIL_LOG_ERROR("Invalid parameters: state is not present; Send failure");
      errorCode = RIL_E_GENERIC_FAILURE;
      sendResponse = true;
      break;
    }

    memset(&qmi_req, 0, sizeof(qmi_req));
    qmi_req.ims_service_enabled_valid = TRUE;
    qmi_req.ims_service_enabled =
        (imsRegState == qcril::interfaces::RegState::REGISTERED ? TRUE : FALSE);

    QCRIL_LOG_INFO("ims_service_enabled = %s", qmi_req.ims_service_enabled ? "TRUE" : "FALSE");
    auto pendingMsgStatus = getImsModule()->getPendingMessageList().insert(msg);
    if (pendingMsgStatus.second != true) {
      break;
    }
    req_id = pendingMsgStatus.first;
    user_data = QCRIL_COMPOSE_USER_DATA(QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, req_id);

    errorCode = qcril_qmi_imss_send_set_ims_service_enable_config_req(&qmi_req, user_data,
        qcril_qmi_imss_set_ims_service_enable_config_resp_ims_reg_change_hdlr);
    if (errorCode != RIL_E_SUCCESS)
    {
      sendResponse = true;
      doCleanup = true;
    }
  } while (FALSE);

  if (sendResponse)
  {
    auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(errorCode, nullptr);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    if (doCleanup) {
      getImsModule()->getPendingMessageList().erase(req_id);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imss_request_set_ims_registration_v02

//===========================================================================
// qcril_qmi_imss_send_set_ims_service_enable_config_req
//===========================================================================
RIL_Errno qcril_qmi_imss_send_set_ims_service_enable_config_req
(
 const ims_settings_set_ims_service_enable_config_req_msg_v01 *qmi_req,
 uint32 user_data,
 qmiAsyncCbType resp_cb
)
{
  qmi_client_error_type qmi_client_error = QMI_NO_ERR;
  RIL_Errno res = RIL_E_GENERIC_FAILURE;

  QCRIL_LOG_FUNC_ENTRY();

  qmi_client_error = qmi_client_imss_send_async(
      QMI_IMS_SETTINGS_SET_IMS_SERVICE_ENABLE_CONFIG_REQ_V01,
      (void *)qmi_req,
      sizeof(ims_settings_set_ims_service_enable_config_req_msg_v01),
      sizeof(ims_settings_set_ims_service_enable_config_rsp_msg_v01),
      resp_cb,
      (void*)(uintptr_t)user_data );

  QCRIL_LOG_INFO(".. qmi send async res %d", (int) qmi_client_error);

  if (qmi_client_error == QMI_NO_ERR)
  {
    res = RIL_E_SUCCESS;
  }

  QCRIL_LOG_FUNC_RETURN();

  return res;
} // qcril_qmi_imss_send_set_ims_service_enable_config_req

//===========================================================================
// qcril_qmi_imss_request_set_ims_srv_status_v02
//===========================================================================
void qcril_qmi_imss_request_set_ims_srv_status_v02(
    std::shared_ptr<QcRilRequestImsSetServiceStatusMessage> msg) {
  RIL_Errno res = RIL_E_GENERIC_FAILURE;
  QCRIL_LOG_FUNC_ENTRY();
  if (msg != nullptr) {
    do {
      if (feature_disabled_modem_req) {
        QCRIL_LOG_INFO("Modem IMS config is disabled. Respond to Telephony with success");
        res = RIL_E_SUCCESS;
        break;
      }
      ims_settings_set_ims_service_enable_config_req_msg_v01 qmi_req = {};
      if (msg->hasVolteEnabled()) {
        qmi_req.volte_enabled_valid = TRUE;
        qmi_req.volte_enabled = msg->getVolteEnabled();
      }
      if (msg->hasVideoTelephonyEnabled()) {
        qmi_req.videotelephony_enabled_valid = TRUE;
        qmi_req.videotelephony_enabled = msg->getVideoTelephonyEnabled();
      }
      if (msg->hasWifiCallingEnabled()) {
        qmi_req.wifi_calling_enabled_valid = TRUE;
        qmi_req.wifi_calling_enabled = msg->getWifiCallingEnabled();
      }
      if (msg->hasUTEnabled()) {
        qmi_req.ut_enabled_valid = TRUE;
        qmi_req.ut_enabled = msg->getUTEnabled();
      }
      if (!qmi_req.volte_enabled_valid && !qmi_req.videotelephony_enabled_valid &&
          !qmi_req.wifi_calling_enabled_valid && !qmi_req.ut_enabled_valid) {
        QCRIL_LOG_INFO("Invalid parameters");
        res = RIL_E_INVALID_ARGUMENTS;
        break;
      }
      auto pendingMsgStatus = getImsModule()->getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        break;
      }
      uint32 user_data = QCRIL_COMPOSE_USER_DATA(QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID,
                                                 pendingMsgStatus.first);
      if (feature_voice_dom_pref_on_toggle_ims_cap && qmi_req.volte_enabled_valid &&
          qmi_req.volte_enabled) {
        // Make sure to send the response only when we set voice_domain_preference and volte_enabled.
        auto setVoiceDomainPrefMsg = std::make_shared<NasSetVoiceDomainPreferenceRequest>(
            NAS_VOICE_DOMAIN_PREF_PS_PREF_V01,
            [msg, qmi_req, user_data](std::shared_ptr<Message> solicitedMsg,
                                      Message::Callback::Status status,
                                      std::shared_ptr<RIL_Errno> result) -> void {
              RIL_Errno res = RIL_E_GENERIC_FAILURE;
              if (status == Message::Callback::Status::SUCCESS && result) {
                res = *result;
              }
              QCRIL_LOG_DEBUG("%s: result = %d", solicitedMsg->dump().c_str(), res);
              if (res == RIL_E_SUCCESS) {
                res = qcril_qmi_imss_send_set_ims_service_enable_config_req(
                    &qmi_req, user_data, qcril_qmi_imss_set_ims_service_enable_config_resp_hdlr);
              } else {
                msg->setPendingMessageState(PendingMessageState::COMPLETED_FAILURE);
              }
              if (res != RIL_E_SUCCESS) {
                msg->sendResponse(
                    msg, Message::Callback::Status::SUCCESS,
                    std::make_shared<QcRilRequestMessageCallbackPayload>(res, nullptr));
                getImsModule()->getPendingMessageList().erase(msg);
              }
            });
        if (setVoiceDomainPrefMsg) {
          setVoiceDomainPrefMsg->dispatch();
          res = RIL_E_SUCCESS;
        }
        break;
      }
      res = qcril_qmi_imss_send_set_ims_service_enable_config_req(
          &qmi_req, user_data, qcril_qmi_imss_set_ims_service_enable_config_resp_hdlr);
    } while (FALSE);
    if (feature_disabled_modem_req || RIL_E_SUCCESS != res) {
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS,
                        std::make_shared<QcRilRequestMessageCallbackPayload>(res, nullptr));
      getImsModule()->getPendingMessageList().erase(msg);
    }
  }

  QCRIL_LOG_FUNC_RETURN();
}  // qcril_qmi_imss_request_set_ims_srv_status_v02

//===========================================================================
// qcril_qmi_imss_request_set_ims_config_v02
//===========================================================================
void qcril_qmi_imss_request_set_ims_config_v02
(
  std::shared_ptr<QcRilRequestImsSetConfigMessage> msg
)
{
  uint32 user_data;
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  /* Radio config mapping related types.
   Needed for processing the request*/
  qcril_qmi_radio_config_params_type config_params;
  qcril_qmi_radio_config_item_value_type config_item_type;
  qcril_qmi_radio_config_handler_type* req_handler = NULL;
  qcril_qmi_radio_config_error_type radio_config_error =
          QCRIL_QMI_RADIO_CONFIG_ERROR_GENERIC_FAILURE;

  /* To Convert the int value to bool  */
  bool bool_value;
  bool sendResponse = false;
  uint16_t req_id = 0;
  std::string strVal;
  uint32_t intVal = 0;

  QCRIL_LOG_FUNC_ENTRY();
  if (msg != nullptr) {
    do {
      if(feature_disabled_modem_req)
      {
        QCRIL_LOG_INFO("Modem IMS config is disabled. Respond to Telephony with success");
        radio_config_error = QCRIL_QMI_RADIO_CONFIG_ERROR_SUCCESS;
        auto config = std::make_shared<qcril::interfaces::ConfigInfo>();
        if (config) {
          config->setItem(msg->getConfigInfo().getItem());
        }
        auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(
                                RIL_E_SUCCESS, config);
        msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
        break;
      }

      auto pendingMsgStatus = getImsModule()->getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true)
      {
          sendResponse = true;
          break;
      }
      req_id = pendingMsgStatus.first;

      qcril::interfaces::ConfigInfo &configInfo = msg->getConfigInfo();
      if (!configInfo.hasItem()) {
        sendResponse = true;
        QCRIL_LOG_ERROR("Config Item is not present in the request, ..sending failure");
        break;
      }

      config_params.config_item =
        qcril_qmi_ims_map_ims_config_to_radio_config_item(configInfo.getItem());

      if(config_params.config_item == QCRIL_QMI_RADIO_CONFIG_INVALID)
      {
        QCRIL_LOG_ERROR("Invalid config item: %d. Doesnt exist in radio config",
                         configInfo.getItem());
        sendResponse = true;
        break;
      }

      user_data = QCRIL_COMPOSE_USER_DATA( instance_id,
                                           QCRIL_DEFAULT_MODEM_ID,
                                           req_id );

      QCRIL_LOG_INFO("processing request - config item: %s",
                   qcril_qmi_radio_config_get_item_log_str(config_params.config_item));
      config_item_type = qcril_qmi_radio_config_get_item_value_type(config_params.config_item);
      // If item value type is boolean and input params has int val
      // As per imsproto for now the items coming in Boolean are placed
      // in intValue as there is no corresponding API in ImsConfig.java
      if((config_item_type == QCRIL_QMI_RADIO_CONFIG_ITEM_IS_BOOLEAN)
          && (configInfo.hasIntValue()))
      {
        QCRIL_LOG_INFO("Config item received is of boolean type in intValue, value: %d",
                        configInfo.getIntValue());
        bool_value = (configInfo.getIntValue() ? TRUE : FALSE );
        config_params.config_item_value_len = sizeof(bool_value);
        config_params.config_item_value = (void *)(&bool_value);
      }
      else if ((config_item_type == QCRIL_QMI_RADIO_CONFIG_ITEM_IS_INT)
                           && configInfo.hasIntValue()) {
        QCRIL_LOG_INFO("Config item received is an integer, value: %d",
                        configInfo.getIntValue());
        intVal = configInfo.getIntValue();
        config_params.config_item_value_len = sizeof(intVal);
        config_params.config_item_value = (void *)(&intVal);
      }
      else if((config_item_type == QCRIL_QMI_RADIO_CONFIG_ITEM_IS_STRING)
                          && (configInfo.hasStringValue())) {
        strVal = configInfo.getStringValue();
        QCRIL_LOG_INFO("Config item received is a string, value: %s", strVal.c_str());
        config_params.config_item_value_len = strlen(strVal.c_str());
        config_params.config_item_value = (void *) strVal.c_str();
        //TODO check string
      }
      else
      {
         QCRIL_LOG_ERROR("..invalid parameters for the config items value");
         radio_config_error = QCRIL_QMI_RADIO_CONFIG_ERROR_INVALID_PARAMETER;
         sendResponse = true;
         break;
      }
      config_params.extra_data_len = sizeof(user_data);
      config_params.extra_data = (void *)(uintptr_t)user_data;
      config_params.config_item_value_type = config_item_type;
      /* Get the set request handler and call it with config params */
      req_handler = qcril_qmi_radio_config_find_set_config_req_handler(config_params.config_item);
      if(req_handler == NULL)
      {
        QCRIL_LOG_ERROR("NULL req handler for the item");
        sendResponse = true;
        break;
      }
      radio_config_error = (req_handler)(&config_params, req_id);
      QCRIL_LOG_INFO("Returned from req handler with radio_config_error: %d", radio_config_error);
    } while(FALSE);

    if(radio_config_error != QCRIL_QMI_RADIO_CONFIG_ERROR_SUCCESS)
    {
      QCRIL_LOG_ERROR("radio config error: %d..sending empty response", radio_config_error);
      qcril_qmi_imss_set_ims_config_log_and_send_response(req_id,
                                                    NULL,
                                                    radio_config_error,
                                                    QCRIL_QMI_RADIO_CONFIG_SETTINGS_RESP_NO_ERR);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imss_request_set_ims_config_v02

//===========================================================================
// qcril_qmi_imss_request_get_ims_config_v02
//===========================================================================
void qcril_qmi_imss_request_get_ims_config_v02
(
    std::shared_ptr<QcRilRequestImsGetConfigMessage> msg
)
{
  uint32 user_data;
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;

  /* Radio config mapping related types. */
  qcril_qmi_radio_config_params_type config_params;
  qcril_qmi_radio_config_handler_type* req_handler = NULL;
  qcril_qmi_radio_config_error_type radio_config_error =
    QCRIL_QMI_RADIO_CONFIG_ERROR_GENERIC_FAILURE;
  uint16_t req_id = 0;

  QCRIL_LOG_FUNC_ENTRY();
  if (msg != nullptr) {
    do {
      auto pendingMsgStatus = getImsModule()->getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true)
      {
          break;
      }
      req_id = pendingMsgStatus.first;

      qcril::interfaces::ConfigInfo &configInfo = msg->getConfigInfo();
      if (!configInfo.hasItem()) {
        QCRIL_LOG_ERROR("Config Item is not present in the request, ..sending failure");
        break;
      }
      config_params.config_item =
        qcril_qmi_ims_map_ims_config_to_radio_config_item(configInfo.getItem());

      if(config_params.config_item == QCRIL_QMI_RADIO_CONFIG_INVALID)
      {
        QCRIL_LOG_ERROR("Invalid config item: %d. Doesnt exist in radio config",
                         configInfo.getItem());
        break;
      }

      user_data = QCRIL_COMPOSE_USER_DATA( instance_id,
                                           QCRIL_DEFAULT_MODEM_ID,
                                           req_id );
      QCRIL_LOG_INFO("processing request - config item: %s",
                   qcril_qmi_radio_config_get_item_log_str(config_params.config_item));
      config_params.config_item_value_len = 0;
      config_params.config_item_value = NULL;
      config_params.extra_data_len = sizeof(user_data);
      config_params.extra_data = (void *)(uintptr_t)user_data;

      /* Get the get request handler and call it with config params */
      req_handler = qcril_qmi_radio_config_find_get_config_req_handler(config_params.config_item);
      if(req_handler == NULL)
      {
        QCRIL_LOG_ERROR("NULL req handler for the item");
        break;
      }

      radio_config_error = (req_handler)(&config_params, req_id);

      QCRIL_LOG_INFO("Returned from req handler with radio_config_error: %d",
                      radio_config_error);
    } while(FALSE);
    if(radio_config_error != QCRIL_QMI_RADIO_CONFIG_ERROR_SUCCESS)
    {
      QCRIL_LOG_ERROR("radio config error: %d..sending empty response", radio_config_error);
      qcril_qmi_imss_get_ims_config_log_and_send_response(req_id,
                                                      NULL,
                                                      radio_config_error,
                                                      QCRIL_QMI_RADIO_CONFIG_SETTINGS_RESP_NO_ERR);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
} //qcril_qmi_imss_request_get_ims_config_v02

//===========================================================================
// qcril_qmi_imss_config_resp_cb
//===========================================================================
void qcril_qmi_imss_config_resp_cb_v02
(
   unsigned int                 msg_id,
   std::shared_ptr<void>        resp_c_struct,
   unsigned int                 resp_c_struct_len,
   void                        *resp_cb_data,
   qmi_client_error_type        transp_err
)
{
 (void)msg_id;
 (void)resp_c_struct_len;
 int config_item;

 uint32 user_data = (uint32)(uintptr_t) resp_cb_data;
 uint16_t req_id = QCRIL_EXTRACT_USER_ID_FROM_USER_DATA(user_data);
 auto pendingMsg = getImsModule()->getPendingMessageList().find(req_id);

 qcril_qmi_radio_config_resp_data_type req_data;
 QCRIL_LOG_FUNC_ENTRY();
  do {
    if (pendingMsg == nullptr)
    {
      QCRIL_LOG_ERROR("pendingMsg is null");
      break;
    }
    if (transp_err != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Transp error (%d) recieved from QMI", transp_err);
      break;
    }
    req_data.data_len = resp_c_struct_len;
    req_data.data = resp_c_struct.get();
    if (pendingMsg->get_message_id() == QcRilRequestImsSetConfigMessage::get_class_message_id()) {
      auto msg(std::static_pointer_cast<QcRilRequestImsSetConfigMessage>(pendingMsg));
      qcril::interfaces::ConfigInfo &configInfo = msg->getConfigInfo();
      config_item =
        qcril_qmi_ims_map_ims_config_to_radio_config_item(configInfo.getItem());
      QCRIL_LOG_INFO("Calling config dispatch response helper, Config_item: %d", config_item);
      qcril_qmi_imss_dispatch_config_response_helper(&req_data, config_item, msg_id, req_id);
    }
    if (pendingMsg->get_message_id() == QcRilRequestImsGetConfigMessage::get_class_message_id())
    {
      auto msg(std::static_pointer_cast<QcRilRequestImsGetConfigMessage>(pendingMsg));
      qcril::interfaces::ConfigInfo &configInfo = msg->getConfigInfo();
      config_item =
         qcril_qmi_ims_map_ims_config_to_radio_config_item(configInfo.getItem());
      QCRIL_LOG_INFO("Calling config dispatch response helper, Config_item: %d", config_item);
      qcril_qmi_imss_dispatch_config_response_helper(&req_data, config_item, msg_id, req_id);
    }
  } while (FALSE);

  QCRIL_LOG_FUNC_RETURN();

}

//===========================================================================
// qcril_qmi_imss_unsol_ind_cb_helper_v02
//===========================================================================
void qcril_qmi_imss_unsol_ind_cb_helper_v02
(
 unsigned int   msg_id,
 unsigned char *decoded_payload,
 uint32_t       decoded_payload_len
)
{

  QCRIL_LOG_FUNC_ENTRY();

  if (decoded_payload && decoded_payload_len)
  {
    switch(msg_id)
    {
      case QMI_IMS_SETTINGS_IMS_SERVICE_ENABLE_CONFIG_IND_V01:
        qcril_qmi_imss_ims_service_enable_config_ind_hdlr(decoded_payload);
        break;

      case QMI_IMS_SETTINGS_RTT_RCVD_DATA_IND_V01:
        qcril_qmi_imss_rtt_rcvd_data_ind_hdlr(decoded_payload);
        break;
      default:
        QCRIL_LOG_INFO("Unknown QMI IMSA indication %d", msg_id);
        break;
    }
  }

  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imss_unsol_ind_cb_helper_v02

//===========================================================================
// qcril_qmi_imss_set_ims_service_enable_config_resp_hdlr
//===========================================================================
void qcril_qmi_imss_set_ims_service_enable_config_resp_hdlr
(
   unsigned int                 msg_id,
   std::shared_ptr<void>        resp_c_struct,
   unsigned int                 resp_c_struct_len,
   void                        *resp_cb_data,
   qmi_client_error_type        transp_err
)
{
  (void)msg_id;
  (void)resp_c_struct_len;

  ims_settings_set_ims_service_enable_config_rsp_msg_v01 *resp;
  RIL_Errno ril_err = RIL_E_GENERIC_FAILURE;

  uint32 user_data = (uint32)(uintptr_t) resp_cb_data;
  uint16_t req_id = QCRIL_EXTRACT_USER_ID_FROM_USER_DATA(user_data);
  auto pendingMsg = getImsModule()->getPendingMessageList().find(req_id);

  QCRIL_LOG_FUNC_ENTRY();

  do {
    if (pendingMsg == nullptr)
    {
      QCRIL_LOG_ERROR("pendingMsg is null");
      break;
    }
    if (transp_err != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Transp error (%d) recieved from QMI", transp_err);
      break;
    }
    resp = (ims_settings_set_ims_service_enable_config_rsp_msg_v01*)(resp_c_struct.get());
    if (resp != nullptr)
    {
      ril_err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(QMI_NO_ERR, &(resp->resp));
      QCRIL_LOG_ESSENTIAL("ril_err: %d, qmi res: %d", (int) ril_err, (int)resp->resp.error);
    } else {
      QCRIL_LOG_ERROR("resp is null");
      ril_err = RIL_E_GENERIC_FAILURE;
    }

    if (pendingMsg->get_message_id() ==
        QcRilRequestImsSetServiceStatusMessage::get_class_message_id()) {
      qcril_qmi_imss_handle_ims_set_service_status_event_resp(req_id, ril_err);
    }
  } while(FALSE);

  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imss_set_ims_service_enable_config_resp_hdlr

//===========================================================================
// qcril_qmi_imss_set_ims_service_enable_config_resp_ims_reg_change_hdlr
//===========================================================================
void qcril_qmi_imss_set_ims_service_enable_config_resp_ims_reg_change_hdlr
(
 unsigned int                 msg_id,
 std::shared_ptr<void>        resp_c_struct,
 unsigned int                 resp_c_struct_len,
 void                        *resp_cb_data,
 qmi_client_error_type        transp_err
)
{
  (void)msg_id;
  (void)resp_c_struct_len;
  ims_settings_set_ims_service_enable_config_rsp_msg_v01 *resp;
  RIL_Errno ril_err = RIL_E_GENERIC_FAILURE;
  uint32 user_data = (uint32)(uintptr_t) resp_cb_data;
  uint16_t req_id = QCRIL_EXTRACT_USER_ID_FROM_USER_DATA(user_data);

  QCRIL_LOG_FUNC_ENTRY();

  getImsModule()->getPendingMessageList().print();
  auto pendingMsg = getImsModule()->getPendingMessageList().extract(req_id);
  getImsModule()->getPendingMessageList().print();

  do {
    if (pendingMsg == nullptr)
    {
      QCRIL_LOG_ERROR("pendingMsg is null");
      break;
    }
    if (transp_err != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Transp error (%d) recieved from QMI", transp_err);
      ril_err = RIL_E_GENERIC_FAILURE;
      break;
    }
    resp = (ims_settings_set_ims_service_enable_config_rsp_msg_v01*)(resp_c_struct.get());
    if (resp == nullptr)
    {
      QCRIL_LOG_ERROR("resp is null");
      ril_err = RIL_E_GENERIC_FAILURE;
      break;
    }
    ril_err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(QMI_NO_ERR, &(resp->resp));
    QCRIL_LOG_ESSENTIAL("ril_err: %d, qmi res: %d", (int) ril_err, (int)resp->resp.error);
  } while (FALSE);

  if (pendingMsg != nullptr)
  {
    auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(ril_err, nullptr);
    auto msg(std::static_pointer_cast<QcRilRequestImsRegistrationChangeMessage>(pendingMsg));
    msg->sendResponse(pendingMsg, Message::Callback::Status::SUCCESS, respPayload);
  }

  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imss_set_ims_service_enable_config_resp_ims_reg_change_hdlr

//===========================================================================
// qcril_qmi_imss_client_ims_service_enable_config_ind_hdlr
//===========================================================================
void qcril_qmi_imss_ims_service_enable_config_ind_hdlr(void *ind_data_ptr)
{
  ims_settings_ims_service_enable_config_ind_msg_v01 *reg_ind_msg_ptr =
      (ims_settings_ims_service_enable_config_ind_msg_v01 *)ind_data_ptr;
  bool send_update_to_data = false;

  QCRIL_LOG_FUNC_ENTRY();

  if (NULL != reg_ind_msg_ptr)
  {
    qcril_qmi_imss_info_lock();

    if (reg_ind_msg_ptr->wifi_calling_enabled_valid)
    {
      qcril_qmi_ims_cached_info.wifi_calling_enabled_valid =
          reg_ind_msg_ptr->wifi_calling_enabled_valid;
      qcril_ims_setting_wfc_status_type old_value_status =
          qcril_qmi_ims_cached_info.wifi_calling_enabled;
      qcril_qmi_ims_cached_info.wifi_calling_enabled =
          qcril_qmi_imss_convert_imss_to_ril_wfc_status_v02(reg_ind_msg_ptr->wifi_calling_enabled);

      if (old_value_status != qcril_qmi_ims_cached_info.wifi_calling_enabled)
      {
        send_update_to_data = true;
      }
    }
    if (reg_ind_msg_ptr->call_mode_preference_ext_valid)
    {
      qcril_qmi_ims_cached_info.call_mode_preference_valid =
          reg_ind_msg_ptr->call_mode_preference_ext_valid;
      qcril_ims_setting_wfc_preference_type old_value_pref =
          qcril_qmi_ims_cached_info.call_mode_preference;
      qcril_qmi_ims_cached_info.call_mode_preference =
          qcril_qmi_imss_convert_imss_to_ril_wfc_preference_v02(
              reg_ind_msg_ptr->call_mode_preference_ext);

      if (old_value_pref != qcril_qmi_ims_cached_info.call_mode_preference)
      {
        send_update_to_data = true;
      }
    }
    else if (reg_ind_msg_ptr->call_mode_preference_valid)
    {
      qcril_qmi_ims_cached_info.call_mode_preference_valid =
          reg_ind_msg_ptr->call_mode_preference_valid;
      qcril_ims_setting_wfc_preference_type old_value_pref =
          qcril_qmi_ims_cached_info.call_mode_preference;
      qcril_qmi_ims_cached_info.call_mode_preference =
          qcril_qmi_imss_convert_imss_to_ril_wfc_preference_v02(
              reg_ind_msg_ptr->call_mode_preference);

      if (old_value_pref != qcril_qmi_ims_cached_info.call_mode_preference)
      {
        send_update_to_data = true;
      }
    }
    qcril_qmi_ims_cached_info.service_mask_by_network_enabled_valid =
        reg_ind_msg_ptr->service_mask_by_network_enabled_valid;
    qcril_qmi_ims_cached_info.service_mask_by_network_enabled =
        reg_ind_msg_ptr->service_mask_by_network_enabled;

    QCRIL_LOG_INFO(".. wifi_call:(%d, %d), wifi_call_preference:(%d, %d), "
                   "servce_mask_by_network_enabled:(%d, %d)",
                   qcril_qmi_ims_cached_info.wifi_calling_enabled_valid,
                   qcril_qmi_ims_cached_info.wifi_calling_enabled,
                   qcril_qmi_ims_cached_info.call_mode_preference_valid,
                   qcril_qmi_ims_cached_info.call_mode_preference,
                   qcril_qmi_ims_cached_info.service_mask_by_network_enabled_valid,
                   qcril_qmi_ims_cached_info.service_mask_by_network_enabled);

    /* IMSA refactoring */
    qcril_qmi_imss_broadcast_wfc_settings(qcril_qmi_ims_cached_info);

    qcril_qmi_imss_info_unlock();

    if (send_update_to_data)
    {
      qcril_qmi_imss_update_wifi_pref_from_ind_to_qcril_data();
    }
  }
  else
  {
    QCRIL_LOG_ERROR("ind_data_ptr is NULL");
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
//  qcril_qmi_imss_convert_imss_to_ril_wfc_status_v02
//===========================================================================
qcril_ims_setting_wfc_status_type qcril_qmi_imss_convert_imss_to_ril_wfc_status_v02
(
  uint8_t wfc_status
)
{
  qcril_ims_setting_wfc_status_type res;

  switch(wfc_status)
  {
    case FALSE:
      res = QCRIL_IMS_SETTING_WFC_OFF;
      break;

    case TRUE:
      res = QCRIL_IMS_SETTING_WFC_ON;
      break;

    default:
      res = QCRIL_IMS_SETTING_WFC_NOT_SUPPORTED;
  }
  return res;
}

//===========================================================================
//  qcril_qmi_imss_convert_imss_to_ril_wfc_preference_v02
//===========================================================================
qcril_ims_setting_wfc_preference_type qcril_qmi_imss_convert_imss_to_ril_wfc_preference_v02
(
  ims_settings_call_mode_option_enum_v01 wfc_preference
)
{
  qcril_ims_setting_wfc_preference_type res;

  switch(wfc_preference)
  {
    case IMS_SETTINGS_CALL_MODE_PREFERRED_WLAN_ONLY_V01:
      res = QCRIL_IMS_SETTING_WFC_WLAN_ONLY;
      break;

    case IMS_SETTINGS_CALL_MODE_PREFERRED_WIFI_V01:
      res = QCRIL_IMS_SETTING_WFC_WLAN_PREFERRED;
      break;

    case IMS_SETTINGS_CALL_MODE_PREFERRED_CELLULAR_V01:
      res = QCRIL_IMS_SETTING_WFC_CELLULAR_PREFERRED;
      break;

    case IMS_SETTINGS_CALL_MODE_PREFERRED_IMS_V01:
      res = QCRIL_IMS_SETTING_WFC_IMS_PREFERRED;
      break;

    default:
      res = QCRIL_IMS_SETTING_WFC_PREF_NONE;
  }
  return res;
}

//===========================================================================
//  qcril_qmi_imss_get_ims_service_enable_config
//===========================================================================
void qcril_qmi_imss_get_ims_service_enable_config()
{
   qmi_client_error_type qmi_client_error = QMI_NO_ERR;
   ims_settings_get_ims_service_enable_config_rsp_msg_v01
     qmi_ims_get_ims_service_enable_config_resp;

   QCRIL_LOG_FUNC_ENTRY();

   memset(&qmi_ims_get_ims_service_enable_config_resp, 0x0,
     sizeof(qmi_ims_get_ims_service_enable_config_resp));

   qmi_client_error = qmi_client_imss_send_sync(
     QMI_IMS_SETTINGS_GET_IMS_SERVICE_ENABLE_CONFIG_REQ_V01,
     NULL,
     0,
     &qmi_ims_get_ims_service_enable_config_resp,
     sizeof(qmi_ims_get_ims_service_enable_config_resp) );
   QCRIL_LOG_INFO(".. qmi send sync res %d", (int) qmi_client_error );

   if (qmi_client_error == QMI_NO_ERR)
   {
      if( qmi_ims_get_ims_service_enable_config_resp.resp.result == QMI_RESULT_SUCCESS_V01 )
      {
        qcril_qmi_imss_store_get_ims_service_enable_resp(
          &qmi_ims_get_ims_service_enable_config_resp);
      }
   }

   qcril_qmi_imss_broadcast_wfc_settings(qcril_qmi_ims_cached_info);

   QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imss_get_ims_service_enable_config

//===========================================================================
//  qcril_qmi_imss_store_get_ims_service_enable_resp
//===========================================================================
void qcril_qmi_imss_store_get_ims_service_enable_resp
(
 ims_settings_get_ims_service_enable_config_rsp_msg_v01 *qmi_resp
)
{
  bool send_update_to_data = false;

  qcril_qmi_imss_info_lock();
  QCRIL_LOG_INFO(".. IMS has_state: %d, state: %d", qmi_resp->ims_service_enabled_valid,
                 qmi_resp->ims_service_enabled);
  qcril_qmi_ims_cached_info.wifi_calling_enabled_valid = qmi_resp->wifi_calling_enabled_valid;
  qcril_ims_setting_wfc_status_type old_value_status =
      qcril_qmi_ims_cached_info.wifi_calling_enabled;
  qcril_qmi_ims_cached_info.wifi_calling_enabled =
      qcril_qmi_imss_convert_imss_to_ril_wfc_status_v02(qmi_resp->wifi_calling_enabled);
  if (old_value_status != qcril_qmi_ims_cached_info.wifi_calling_enabled)
  {
    send_update_to_data = true;
  }
  if (qmi_resp->call_mode_preference_ext_valid)
  {
    qcril_qmi_ims_cached_info.call_mode_preference_valid = qmi_resp->call_mode_preference_ext_valid;
    qcril_ims_setting_wfc_preference_type old_value_pref = qcril_qmi_ims_cached_info.call_mode_preference;
    qcril_qmi_ims_cached_info.call_mode_preference =
        qcril_qmi_imss_convert_imss_to_ril_wfc_preference_v02(qmi_resp->call_mode_preference_ext);
    if (old_value_pref != qcril_qmi_ims_cached_info.call_mode_preference)
    {
      send_update_to_data = true;
    }
  }
  else if (qmi_resp->call_mode_preference_valid)
  {
    qcril_qmi_ims_cached_info.call_mode_preference_valid = qmi_resp->call_mode_preference_valid;
    qcril_ims_setting_wfc_preference_type old_value_pref = qcril_qmi_ims_cached_info.call_mode_preference;
    qcril_qmi_ims_cached_info.call_mode_preference =
        qcril_qmi_imss_convert_imss_to_ril_wfc_preference_v02(qmi_resp->call_mode_preference);
    if (old_value_pref != qcril_qmi_ims_cached_info.call_mode_preference)
    {
      send_update_to_data = true;
    }
  }
  qcril_qmi_ims_cached_info.service_mask_by_network_enabled_valid =
      qmi_resp->service_mask_by_network_enabled_valid;
  qcril_qmi_ims_cached_info.service_mask_by_network_enabled =
      qmi_resp->service_mask_by_network_enabled;

  QCRIL_LOG_INFO(".. wifi_call:(%d, %d), wifi_call_preference:(%d, %d), "
                 "servce_mask_by_network_enabled:(%d, %d)",
                 qcril_qmi_ims_cached_info.wifi_calling_enabled_valid,
                 qcril_qmi_ims_cached_info.wifi_calling_enabled,
                 qcril_qmi_ims_cached_info.call_mode_preference_valid,
                 qcril_qmi_ims_cached_info.call_mode_preference,
                 qcril_qmi_ims_cached_info.service_mask_by_network_enabled_valid,
                 qcril_qmi_ims_cached_info.service_mask_by_network_enabled);
  qcril_qmi_imss_info_unlock();

  if (send_update_to_data)
  {
    qcril_qmi_imss_update_wifi_pref_from_ind_to_qcril_data();
  }
}

//===========================================================================
//  qcril_qmi_imss_get_service_mask_by_network_enabled
//===========================================================================
ims_settings_omadm_services_type_mask_v01 qcril_qmi_imss_get_service_mask_by_network_enabled()
{
  ims_settings_omadm_services_type_mask_v01 service_mask =
    IMS_SETTINGS_OMADM_SERVICES_BIT_MASK_NONE_V01;

  qcril_qmi_imss_info_lock();
  if(qcril_qmi_ims_cached_info.service_mask_by_network_enabled_valid)
  {
    service_mask = qcril_qmi_ims_cached_info.service_mask_by_network_enabled;
  }
  qcril_qmi_imss_info_unlock();

  return service_mask;
}

//===========================================================================
// qcril_qmi_imss_request_send_rtt_msg
//===========================================================================
void qcril_qmi_imss_request_send_rtt_msg
(
    std::shared_ptr<QcRilRequestImsSendRttMessage> msg
)
{
  ims_settings_send_rtt_req_msg_v01  qmi_req;
  uint32 user_data;
  qmi_client_error_type qmi_client_error = QMI_NO_ERR;
  RIL_Errno res = RIL_E_GENERIC_FAILURE;

  memset(&qmi_req, 0, sizeof(qmi_req));
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  uint16_t req_id = -1;
  bool doCleanup = false;

  QCRIL_LOG_FUNC_ENTRY();

  if (msg != nullptr) {
    do {
      string rttMessage = msg->getRttMessage();
      if (rttMessage.empty()) {
        QCRIL_LOG_ERROR("Invalid Arguments for RTT Message");
        break;
      }

      if (IMS_SETTINGS_RTT_DATA_LEN_MAX_V01 <= rttMessage.length())
      {
        QCRIL_LOG_ERROR("Invalid rtt string length %d", rttMessage.length());
        break;
      }

      qmi_req.rtt_data_len = rttMessage.length();

      strlcpy((char*)qmi_req.rtt_data,
              (char*)rttMessage.c_str(),
              IMS_SETTINGS_RTT_DATA_LEN_MAX_V01);
      QCRIL_LOG_DEBUG("Rtt Message : %s ",qmi_req.rtt_data);

      auto pendingMsgStatus = getImsModule()->getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true)
      {
        break;
      }
      req_id = pendingMsgStatus.first;
      user_data = QCRIL_COMPOSE_USER_DATA(instance_id, QCRIL_DEFAULT_MODEM_ID, req_id);

      qmi_client_error = qmi_client_imss_send_async(QMI_IMS_SETTINGS_SEND_RTT_REQ_V01,
                                                    &qmi_req,
                                                    sizeof(ims_settings_send_rtt_req_msg_v01),
                                                    sizeof(ims_settings_send_rtt_resp_msg_v01),
                                                    qcril_qmi_imss_request_send_rtt_msg_resp_hdlr,
                                                    (void*)(uintptr_t)user_data );
      QCRIL_LOG_INFO(".. qmi send async res %d", (int) qmi_client_error );
      if (E_SUCCESS == qmi_client_error)
      {
        res = RIL_E_SUCCESS;
      } else {
        doCleanup = true;
      }
    } while(FALSE);
    if (RIL_E_SUCCESS != res)
    {
        auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(res, nullptr);
        msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
        if (doCleanup) {
            getImsModule()->getPendingMessageList().erase(req_id);
        }
    }
  }

  QCRIL_LOG_FUNC_RETURN();
}  // qcril_qmi_imss_request_send_rtt_msg

//===========================================================================
// qcril_qmi_imss_request_send_rtt_msg_resp_hdlr
//===========================================================================
void qcril_qmi_imss_request_send_rtt_msg_resp_hdlr
(
   unsigned int                 msg_id,
   std::shared_ptr<void>        resp_c_struct,
   unsigned int                 resp_c_struct_len,
   void                        *resp_cb_data,
   qmi_client_error_type        transp_err
)
{
  (void)msg_id;
  (void)resp_c_struct_len;
  ims_settings_send_rtt_resp_msg_v01 *resp = nullptr;
  RIL_Errno ril_err = RIL_E_GENERIC_FAILURE;

  uint32 user_data = (uint32)(uintptr_t) resp_cb_data;
  uint16_t req_id = QCRIL_EXTRACT_USER_ID_FROM_USER_DATA(user_data);
  auto pendingMsg = getImsModule()->getPendingMessageList().extract(req_id);

  QCRIL_LOG_FUNC_ENTRY();

  do {
    if (pendingMsg == nullptr)
    {
      QCRIL_LOG_ERROR("pendingMsg is null");
      break;
    }
    if (transp_err != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Transp error (%d) recieved from QMI", transp_err);
      break;
    }
    resp = (ims_settings_send_rtt_resp_msg_v01*)(resp_c_struct.get());
    if (resp == nullptr)
    {
      QCRIL_LOG_ERROR("resp is null");
      break;
    }
    ril_err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(QMI_NO_ERR,
                                                                     &(resp->resp));
    QCRIL_LOG_ESSENTIAL("ril_err: %d, qmi res: %d", (int) ril_err, (int)resp->resp.error);
  } while(FALSE);

  if (pendingMsg) {
    auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(ril_err, nullptr);
    auto ril_msg = std::static_pointer_cast<QcRilRequestImsSendRttMessage>(pendingMsg);
    ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, respPayload);
  }

  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imss_request_send_rtt_msg_resp_hdlr

//===========================================================================
// qcril_qmi_imss_rtt_rcvd_data_ind_hdlr
//===========================================================================
void qcril_qmi_imss_rtt_rcvd_data_ind_hdlr(void *ind_data_ptr)
{
  ims_settings_rtt_ind_msg_v01 *rtt_message_incoming = (ims_settings_rtt_ind_msg_v01 *)ind_data_ptr;
  int rtt_msg_len;

  QCRIL_LOG_FUNC_ENTRY();

  if( rtt_message_incoming != NULL)
  {
    rtt_msg_len =  (int)rtt_message_incoming->rtt_data_len + 1;
    char *rtt_message = nullptr;
    rtt_message = (char *) qcril_malloc(sizeof(char) * rtt_msg_len);
    if (rtt_message) {
      strlcpy(rtt_message, (char*)rtt_message_incoming->rtt_data,
                rtt_msg_len);
      auto msg = std::make_shared<QcRilUnsolImsRttMessage>(rtt_message);
      if (msg != nullptr)
      {
        Dispatcher::getInstance().dispatchSync(msg);
      }
    } else {
      QCRIL_LOG_ERROR("memory failure");
    }
    if (rtt_message) {
        qcril_free(rtt_message);
    }
  }
  else
  {
    QCRIL_LOG_ERROR("ind_data_ptr is NULL");
  }

  QCRIL_LOG_FUNC_RETURN();
}
