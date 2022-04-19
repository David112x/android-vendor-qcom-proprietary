/******************************************************************************
#  Copyright (c) 2013-2015, 2017, 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
/******************************************************************************
  @file    qcril_qmi_imss.c
  @brief   qcril qmi - IMS Setting

  DESCRIPTION
    Handles RIL requests, Callbacks, indications for QMI IMS Setting.

******************************************************************************/

#include <cutils/properties.h>

#define TAG "QCRIL_IMSS"

#include "qcril_log.h"
#include <framework/Log.h>
#include "qcril_qmi_client.h"
#include "qcril_qmi_imss.h"
#include "ip_multimedia_subsystem_settings_v01.h"
#include "qcril_reqlist.h"
#include "qcril_qmi_npb_utils.h"
#include "qcril_qmi_radio_config_imss.h"
#include "modules/nas/qcril_qmi_nas.h"
#include "qcril_qmi_imss_v02.h"
#include "modules/nas/qcril_arb.h"

#include "qcril_qmi_imss_qmi.h"
#include "modules/nas/NasSetVoiceDomainPreferenceRequest.h"
#include "SetRatPrefMessage.h"

#include <interfaces/ims/ims.h>
#include "modules/ims/ImsModule.h"
#include "qcril_qmi_ims_utils.h"

//using namespace qcril::interfaces;

#define BIT_POS_IMS_SERVICE_ENABLE_CONFIG_MSG_ID 0x80
#define BYTE_POS_IMS_SERVICE_ENABLE_CONFIG_MSG_ID 17

//WFC roaming config TLV is 0x24 (36). Fields 0 to 15 (decimal) are mandatory by definition
//Starting with the LSB, bit 0 represents field ID 16. So bit 20 represents field id 36
#define BIT_POS_WFC_ROAMING_MODE_PREFERENCE  20

// Modem supports PREFERENCE_IMS with call_mode_preference_ext TLV (0x26)
#define BIT_POS_WFC_PREFERENCE_IMS           22

//===========================================================================
//                     GLOBALS
//===========================================================================
struct ims_cached_info_type   qcril_qmi_ims_cached_info;

static void qcril_qmi_imss_update_modem_version(void);
static void qcril_qmi_imss_update_modem_features(void);
static RIL_Errno qcril_qmi_ims_map_radio_config_error_to_ril_error
(
 qcril_qmi_radio_config_error_type radio_config_error
);

boolean feature_voice_dom_pref_on_toggle_ims_cap = FALSE;
boolean feature_disabled_modem_req               = FALSE;
static boolean modem_supports_wfc_roaming_config        = FALSE;
static boolean modem_supports_wfc_preferred_ims         = FALSE;

static uint8_t modem_version = 0; // 0 for old 1 for new modem versions

//===========================================================================
// qcril_qmi_imss_pre_init
//===========================================================================
void qcril_qmi_imss_pre_init(void)
{
  char prop_str[PROPERTY_VALUE_MAX];

  QCRIL_LOG_FUNC_ENTRY();

  memset(prop_str, 0, sizeof(prop_str));
  property_get(QMI_RIL_IMSS_VOICE_DOMAIN_PREF_ON_IMS_TOGGLE, prop_str, "");
  QCRIL_LOG_INFO("Property: %s, value: %s", QMI_RIL_IMSS_VOICE_DOMAIN_PREF_ON_IMS_TOGGLE, prop_str);
  feature_voice_dom_pref_on_toggle_ims_cap =
    ((strcmp(prop_str, "true") == 0) || (strcmp(prop_str, "1") == 0));

  memset(prop_str, 0, sizeof(prop_str));
  property_get(QMI_RIL_DISABLE_MODEM_CONFIG, prop_str, "");
  QCRIL_LOG_INFO("Property: %s, value: %s", QMI_RIL_DISABLE_MODEM_CONFIG, prop_str);
  feature_disabled_modem_req = (strncmp(prop_str, "true", QMI_RIL_DISABLE_MODEM_CONFIG_LENGTH)==0);

  qcril_qmi_imss_cleanup();

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// qcril_qmi_imss_cleanup
//===========================================================================
void qcril_qmi_imss_cleanup(void)
{
  QCRIL_LOG_FUNC_ENTRY();

  qcril_qmi_imss_info_lock();

  qcril_qmi_ims_cached_info.wifi_calling_enabled_valid = FALSE;
  qcril_qmi_ims_cached_info.wifi_calling_enabled =
    qcril_qmi_imss_convert_imss_to_ril_wfc_status(IMS_SETTINGS_WFC_STATUS_NOT_SUPPORTED_V01);
  qcril_qmi_ims_cached_info.call_mode_preference_valid = FALSE;
  qcril_qmi_ims_cached_info.call_mode_preference =
    qcril_qmi_imss_convert_imss_to_ril_wfc_preference(IMS_SETTINGS_WFC_CALL_PREF_NONE_V01);

  qcril_qmi_imss_info_unlock();

  qcril_qmi_imss_broadcast_wfc_settings(qcril_qmi_ims_cached_info);

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// qcril_qmi_ims_send_unsol_wfc_roaming_config
//===========================================================================
void qcril_qmi_ims_send_unsol_wfc_roaming_config()
{
  QCRIL_LOG_FUNC_ENTRY();

  auto msg = std::make_shared<QcRilUnsolImsWfcRoamingConfigIndication>(
      modem_supports_wfc_roaming_config);
  if (msg != nullptr)
  {
    Dispatcher::getInstance().dispatchSync(msg);
  }

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// qcril_qmi_imss_init
//===========================================================================
void qcril_qmi_imss_init(void)
{
   QCRIL_LOG_FUNC_ENTRY();

   qcril_qmi_imss_update_modem_version();
   qcril_qmi_radio_config_update_meta_table(modem_version);
   qcril_qmi_imss_update_modem_features();

   // register for indication
   qmi_client_error_type ret = QMI_NO_ERR;
   ims_settings_config_ind_reg_req_msg_v01 ind_reg_req;

   memset(&ind_reg_req, 0, sizeof(ind_reg_req));
   ind_reg_req.client_provisioning_config_valid = TRUE;
   ind_reg_req.client_provisioning_config = 1;  // enable

   ind_reg_req.reg_mgr_config_valid = TRUE;
   ind_reg_req.reg_mgr_config = 1;  // enable

   ind_reg_req.ims_service_enable_config_enabled_valid = TRUE;
   ind_reg_req.ims_service_enable_config_enabled = 1;

   ind_reg_req.rtt_config_enabled_valid = TRUE;
   ind_reg_req.rtt_config_enabled = 1; //enabled

   ret = qmi_client_imss_send_async(QMI_IMS_SETTINGS_CONFIG_IND_REG_REQ_V01,
       (void*) &ind_reg_req,
       sizeof(ims_settings_config_ind_reg_req_msg_v01),
       sizeof(ims_settings_config_ind_reg_rsp_msg_v01),
       qcril_qmi_imss_command_cb,
       (void*)0
       );
   if (QMI_NO_ERR != ret)
   {
     QCRIL_LOG_ERROR("registration for indication failed %d", ret);
   }

   //Get the intial config values
   //Fetch the WFC status
   if(1 == modem_version)
   {
     qcril_qmi_imss_get_ims_service_enable_config();
   }
   else
   {
     qcril_qmi_imss_get_client_provisioning_config();
   }

   QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imss_init

//===========================================================================
// qcril_qmi_imss_info_lock
//===========================================================================
void qcril_qmi_imss_info_lock()
{
    qcril_qmi_ims_cached_info.imss_info_lock_mutex.lock();
} // qcril_qmi_imss_info_lock

//===========================================================================
// qcril_qmi_imss_info_unlock
//===========================================================================
void qcril_qmi_imss_info_unlock()
{
    qcril_qmi_ims_cached_info.imss_info_lock_mutex.unlock();
} // qcril_qmi_imss_info_unlock

//===========================================================================
// qcril_qmi_imss_set_reg_mgr_config_resp_hdlr
//===========================================================================
void qcril_qmi_imss_set_reg_mgr_config_resp_hdlr
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
  ims_settings_set_reg_mgr_config_rsp_msg_v01 *resp;
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
    resp = (ims_settings_set_reg_mgr_config_rsp_msg_v01*)(resp_c_struct.get());
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
} // qcril_qmi_imss_set_reg_mgr_config_resp_hdlr

//===========================================================================
// qcril_qmi_imss_request_set_ims_registration
//===========================================================================
void qcril_qmi_imss_request_set_ims_registration(
    std::shared_ptr<QcRilRequestImsRegistrationChangeMessage> msg)
{
  qcril::interfaces::RegState imsRegState = qcril::interfaces::RegState::UNKNOWN;
  ims_settings_set_reg_mgr_config_req_msg_v01 qmi_req;
  bool sendResponse = false;
  uint32 user_data;
  qmi_client_error_type qmi_client_error = QMI_NO_ERR;
  RIL_Errno errorCode = RIL_E_SUCCESS;
  uint16_t req_id = -1;
  bool doCleanup = false;

  QCRIL_LOG_FUNC_ENTRY();

  do
  {
    imsRegState = msg->getRegState();
    //QCRIL_LOG_INFO("state: %d", imsRegState);

    if (feature_disabled_modem_req)
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

    getImsModule()->getPendingMessageList().print();

    auto pendingMsgStatus = getImsModule()->getPendingMessageList().insert(msg);
    if (pendingMsgStatus.second != true) {
      break;
    }
    req_id = pendingMsgStatus.first;
    user_data = QCRIL_COMPOSE_USER_DATA(QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID, req_id);

    // Call QMI IMSS API to set the ims test mode.
    memset(&qmi_req, 0, sizeof(qmi_req));
    qmi_req.ims_test_mode_enabled_valid = TRUE;
    qmi_req.ims_test_mode_enabled =
        (imsRegState == qcril::interfaces::RegState::REGISTERED) ? FALSE : TRUE;
    QCRIL_LOG_INFO("ims_test_mode_enabled = %s", qmi_req.ims_test_mode_enabled ? "TRUE" : "FALSE");

    qmi_client_error = qmi_client_imss_send_async(
        QMI_IMS_SETTINGS_SET_REG_MGR_CONFIG_REQ_V01,
        &qmi_req,
        sizeof(ims_settings_set_reg_mgr_config_req_msg_v01),
        sizeof(ims_settings_set_reg_mgr_config_rsp_msg_v01),
        qcril_qmi_imss_set_reg_mgr_config_resp_hdlr,
        (void*)(uintptr_t)user_data);
    QCRIL_LOG_INFO(".. qmi send async res %d", (int) qmi_client_error);

    if (qmi_client_error != QMI_NO_ERR)
    {
      errorCode = RIL_E_GENERIC_FAILURE;
      sendResponse = true;
      doCleanup = true;
      break;
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
} // qcril_qmi_imss_request_set_ims_registration

//===========================================================================
// qcril_qmi_imss_request_set_ims_srv_status
//===========================================================================
void qcril_qmi_imss_request_set_ims_srv_status(
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
      if (msg->hasVolteEnabled() && msg->hasVideoTelephonyEnabled() &&
          msg->hasWifiCallingEnabled()) {
        QCRIL_LOG_ERROR("Not supported settings all volte/vt/wfc in a single request");
        res = RIL_E_REQUEST_NOT_SUPPORTED;
        break;
      }
      auto pendingMsgStatus = getImsModule()->getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        break;
      }
      uint32 user_data = QCRIL_COMPOSE_USER_DATA(QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID,
                                                 pendingMsgStatus.first);
      if (msg->hasWifiCallingEnabled()) {
        // Set the wifi calling setting through client provisioning settings.
        // If the radio tech is WLAN. So declare the variable.
        ims_settings_set_client_provisioning_config_req_msg_v01 qmi_client_provisioning_req = {};
        qmi_client_provisioning_req.wifi_call_valid = TRUE;
        qmi_client_provisioning_req.wifi_call = msg->getWifiCallingEnabled()
                                                    ? IMS_SETTINGS_WFC_STATUS_ON_V01
                                                    : IMS_SETTINGS_WFC_STATUS_OFF_V01;
        QCRIL_LOG_INFO("Sending wifi call setting through set_service, value: %d",
                       qmi_client_provisioning_req.wifi_call);

        qmi_client_error_type qmi_client_error = qmi_client_imss_send_async(
            QMI_IMS_SETTINGS_SET_CLIENT_PROVISIONING_CONFIG_REQ_V01, &qmi_client_provisioning_req,
            sizeof(ims_settings_set_client_provisioning_config_req_msg_v01),
            sizeof(ims_settings_set_client_provisioning_config_rsp_msg_v01),
            qcril_qmi_imss_set_client_provisioning_config_resp_hdlr, (void*)(uintptr_t)user_data);
        QCRIL_LOG_INFO(".. qmi send async res %d", (int)qmi_client_error);
        if (qmi_client_error == E_SUCCESS) {
          res = RIL_E_SUCCESS;
        }
      }
      ims_settings_set_qipcall_config_req_msg_v01 qmi_req = {};
      if (msg->hasVolteEnabled()) {
        qmi_req.volte_enabled_valid = TRUE;
        qmi_req.volte_enabled = msg->getVolteEnabled();
      }
      if (msg->hasVideoTelephonyEnabled()) {
        qmi_req.vt_calling_enabled_valid = TRUE;
        qmi_req.vt_calling_enabled = msg->getVideoTelephonyEnabled();
      }
      if (feature_voice_dom_pref_on_toggle_ims_cap && qmi_req.volte_enabled_valid &&
          qmi_req.volte_enabled) {
        // Make sure to send the response only when we set voice_domain_preference and
        // volte_enabled.
        auto setVoiceDomainPrefMsg = std::make_shared<NasSetVoiceDomainPreferenceRequest>(
            NAS_VOICE_DOMAIN_PREF_PS_PREF_V01,
            [msg, qmi_req, user_data](std::shared_ptr<Message> /*msg*/,
                                      Message::Callback::Status status,
                                      std::shared_ptr<RIL_Errno> result) -> void {
              RIL_Errno res = RIL_E_GENERIC_FAILURE;
              if (status == Message::Callback::Status::SUCCESS && result) {
                res = *result;
              }
              QCRIL_LOG_DEBUG("NasSetVoiceDomainPreferenceRequest: result = %d", res);
              if (res == RIL_E_SUCCESS) {
                res = qcril_qmi_imss_send_set_qipcall_config_req(&qmi_req, user_data);
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
      if (qmi_req.volte_enabled_valid || qmi_req.vt_calling_enabled_valid) {
        res = qcril_qmi_imss_send_set_qipcall_config_req(&qmi_req, user_data);
      } else {
        QCRIL_LOG_ERROR("request misses some necessary information");
        break;
      }
    } while (FALSE);
    if (feature_disabled_modem_req || RIL_E_SUCCESS != res) {
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(res, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
      getImsModule()->getPendingMessageList().erase(msg);
    }
  }

  QCRIL_LOG_FUNC_RETURN();
}  // qcril_qmi_imss_request_set_ims_srv_status

//===========================================================================
// converToQmiMultiIdentityLineType
//===========================================================================
ims_settings_uri_line_type_v01 converToQmiMultiIdentityLineType(int lineType) {
    switch(lineType) {
        case qcril::interfaces::MultiIdentityInfo::LINE_TYPE_PRIMARY:
            return IMS_SETTINGS_URI_LINE_PRIMARY_V01;
        default:
            return IMS_SETTINGS_URI_LINE_SECONDARY_V01;
    }
} // converToQmiMultiIdentityLineType

//===========================================================================
// qcril_qmi_imss_request_register_multi_identity_lines
//===========================================================================
void qcril_qmi_imss_request_register_multi_identity_lines
(
  std::shared_ptr<QcRilRequestImsRegisterMultiIdentityMessage> msg
)
{
  QCRIL_LOG_FUNC_ENTRY();
  RIL_Errno res = RIL_E_GENERIC_FAILURE;
  uint32 user_data;
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  bool sendResponse = false;
  //cleanup request from pendingmsglist
  bool doCleanup = false;
  uint16_t req_id = 0;
  if (msg != nullptr) {
    do {
      auto& lines = msg->getLineInfo();
      int n_lines = lines.size();
      if (n_lines == 0) {
        QCRIL_LOG_ERROR("Registration request for zero lines");
        sendResponse = true;
        doCleanup = false;
        break;
      }
      ims_settings_set_digits_req_msg_v01 qmi_req;
      memset(&qmi_req, 0, sizeof(qmi_req));

      qmi_req.is_digits_enabled = IMS_SETTINGS_DIGITS_FEATURE_ENABLED_V01;
      qmi_req.no_of_lines_valid = 1;
      qmi_req.no_of_lines = n_lines;
      //Below two fields are the same as above two
      qmi_req.line_info_list_valid = 1;
      qmi_req.line_info_list_len = n_lines;
      int index = 0;
      for (auto &in_line : lines) {
        auto& out_line = qmi_req.line_info_list[index];
        strlcpy(out_line.msisdn, in_line.msisdn.c_str(), sizeof(out_line.msisdn));
        //we won't fill token as it is not required
        out_line.uri_line_type = converToQmiMultiIdentityLineType(in_line.lineType);
        QCRIL_LOG_DEBUG("MultiIdentity line[%d] : msisdn = %s , line type = %d", index,
                out_line.msisdn, out_line.uri_line_type);
        index++;
      }
      auto pendingMsgStatus = getImsModule()->getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true)
      {
          sendResponse = true;
          doCleanup = false;
          break;
      }
      req_id = pendingMsgStatus.first;
      user_data = QCRIL_COMPOSE_USER_DATA( instance_id,
                                           QCRIL_DEFAULT_MODEM_ID,
                                           req_id );

      qmi_client_error_type qmi_client_error = qmi_client_imss_send_async(
                                        QMI_IMS_SETTINGS_SET_DIGITS_REQ_V01,
                                        &qmi_req,
                                        sizeof(ims_settings_set_digits_req_msg_v01),
                                        sizeof(ims_settings_set_digits_req_rsp_msg_v01),
                                        qcril_qmi_imss_register_multi_identity_lines_resp_hdlr,
                                        (void*)(uintptr_t)user_data);
      QCRIL_LOG_INFO(".. qmi send async res %d", (int) qmi_client_error );
      if (QMI_NO_ERR != qmi_client_error)
      {
        sendResponse = true;
        doCleanup = true;
      }
    } while(FALSE);

    if (sendResponse == true) {
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(res, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
      if (doCleanup) {
        getImsModule()->getPendingMessageList().erase(req_id);
      }
    }
  }
  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imss_request_register_multi_identity_lines

//===========================================================================
// qcril_qmi_imss_request_set_ims_config
//
// Functionality:
// 1. Reads config item from the ims request
// 2. creates a reqlist entry (store the config item id here)
// 3. Depending on the type of item's value, process the data
// 4. and Store the processed data in Config params
// 5. gets the set request handler for the config item
// 6. Call the handler with config params,
//    to send down the appropriate req
//===========================================================================
void qcril_qmi_imss_request_set_ims_config
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
  uint32_t intVal = 0;
  std::string strVal;

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
        config_params.config_item_value_len = strlen(strVal.c_str());
        QCRIL_LOG_INFO("Config item received is a string, value: %s", strVal.c_str());
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
    }while (FALSE);
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
} // qcril_qmi_imss_request_set_ims_config

//===========================================================================
// qcril_qmi_imss_request_get_ims_config
//
// Functionality:
// 1. Reads config item from the ims request
// 2. creates a reqlist entry (store the config item id here)
// 3. Store the processed data in Config params
// 5. gets the get request handler for the config item
// 6. Call the handler with config params,
//    to send down the appropriate req
//===========================================================================
void qcril_qmi_imss_request_get_ims_config
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
} //qcril_qmi_imss_request_get_ims_config

//===========================================================================
// qcril_qmi_imss_handle_ims_set_service_status_event_resp
//===========================================================================
void qcril_qmi_imss_handle_ims_set_service_status_event_resp(uint16 req_id, RIL_Errno ril_err) {
  boolean send_response = TRUE;

  QCRIL_LOG_FUNC_ENTRY();
  do {
    auto ril_msg = getImsModule()->getPendingMessageList().extract(req_id);
    if (ril_msg == nullptr) {
      QCRIL_LOG_ERROR("message is null");
      break;
    }
    if (ril_msg->get_message_id() == QcRilRequestImsSetServiceStatusMessage::get_class_message_id()) {
      auto pendingMsg = std::static_pointer_cast<QcRilRequestImsSetServiceStatusMessage>(ril_msg);
      if (pendingMsg->getPendingMessageState() == PendingMessageState::COMPLETED_FAILURE) {
        QCRIL_LOG_INFO("state is QCRIL_REQ_COMPLETED_FAILURE!!");
        ril_err = RIL_E_GENERIC_FAILURE;
        break;
      }
      if (ril_err != RIL_E_SUCCESS) {
        pendingMsg->setPendingMessageState(PendingMessageState::COMPLETED_FAILURE);
      }
      if (feature_voice_dom_pref_on_toggle_ims_cap && pendingMsg->hasVolteEnabled()) {
        bool volte_enabled = pendingMsg->getVolteEnabled();
        // 1. Set voice domain pref to CS_ONLY if the SET_SERVICE_STATUS is to enable VoLTE
        //    and modem returns success for VoLTE enabled request.
        // 2. Revert voice domain pref to CS_ONLY if the SET_SERVICE_STATUS is to disable VoLTE
        //    and modem returns failure for VoLTE disable request.
        if ((!volte_enabled && (ril_err == RIL_E_SUCCESS)) ||
            (volte_enabled && (ril_err != RIL_E_SUCCESS))) {
          // set voice domain pref
          auto setVoiceDomainPrefMsg = std::make_shared<NasSetVoiceDomainPreferenceRequest>(
              NAS_VOICE_DOMAIN_PREF_CS_ONLY_V01,
              [pendingMsg, ril_err](std::shared_ptr<Message> solicitedMsg,
                                    Message::Callback::Status status,
                                    std::shared_ptr<RIL_Errno> result) -> void {
                RIL_Errno res = ril_err;
                if (status == Message::Callback::Status::SUCCESS && result) {
                  if (*result != RIL_E_SUCCESS) {
                    res = *result;
                  }
                }
                QCRIL_LOG_DEBUG("%s: result = %d", solicitedMsg->dump().c_str(), res);
                pendingMsg->sendResponse(
                    pendingMsg, Message::Callback::Status::SUCCESS,
                    std::make_shared<QcRilRequestMessageCallbackPayload>(res, nullptr));
              });
          if (setVoiceDomainPrefMsg) {
            setVoiceDomainPrefMsg->dispatch();
            send_response = false;
          }
        }
      }
    }
    if (send_response) {
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(ril_err, nullptr);
      auto respMsg = std::static_pointer_cast<QcRilRequestImsSetServiceStatusMessage>(ril_msg);
      respMsg->sendResponse(respMsg, Message::Callback::Status::SUCCESS, respPayload);
      // erase the message
      getImsModule()->getPendingMessageList().erase(ril_msg);
    }
  } while (FALSE);

  QCRIL_LOG_FUNC_RETURN();
}  // qcril_qmi_imss_handle_ims_set_service_status_event_resp

//===========================================================================
// qcril_qmi_imss_set_qipcall_config_resp_hdlr
//===========================================================================
void qcril_qmi_imss_set_qipcall_config_resp_hdlr
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
  ims_settings_set_qipcall_config_rsp_msg_v01 *resp;
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
    resp = (ims_settings_set_qipcall_config_rsp_msg_v01*)(resp_c_struct.get());
    if (resp == nullptr)
    {
      QCRIL_LOG_ERROR("resp is null");
      break;
    }
    ril_err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(QMI_NO_ERR, &(resp->resp));
    QCRIL_LOG_ESSENTIAL("ril_err: %d, qmi res: %d", (int) ril_err, (int)resp->resp.error);
    if (pendingMsg->get_message_id() ==
        QcRilRequestImsSetServiceStatusMessage::get_class_message_id()) {
        //TODO
        qcril_qmi_imss_handle_ims_set_service_status_event_resp(req_id, ril_err);
    } else {
      //TODO enable based on request
      //auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(ril_err, nullptr);
      //pendingMsg->sendResponse(pendingMsg, Message::Callback::Status::SUCCESS, respPayload);
      //erase the message
      //getImsModule()->getPendingMessageList().erase(pendingMsg);

    }
  } while(FALSE);

  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imss_set_qipcall_config_resp_hdlr

//===========================================================================
// qcril_qmi_imss_set_client_provisioning_config_resp_hdlr
//===========================================================================
void qcril_qmi_imss_set_client_provisioning_config_resp_hdlr
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
    ims_settings_set_client_provisioning_config_rsp_msg_v01 *resp;
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
        resp = (ims_settings_set_client_provisioning_config_rsp_msg_v01*)(resp_c_struct.get());
        if (resp == nullptr)
        {
            QCRIL_LOG_ERROR("resp is null");
            break;
        }
        ril_err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(
                QMI_NO_ERR, &(resp->resp));
        QCRIL_LOG_ESSENTIAL("ril_err: %d, qmi res: %d", (int) ril_err, (int)resp->resp.error);

    } while (FALSE);

    if (pendingMsg) {
        auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(ril_err, nullptr);
        auto ril_msg = std::static_pointer_cast<QcRilRequestImsSetServiceStatusMessage>(pendingMsg);
        ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, respPayload);
    }

    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imss_set_client_provisioning_config_resp_hdlr

//===========================================================================
// qcril_qmi_imss_register_multi_identity_lines_resp_hdlr
//===========================================================================
void qcril_qmi_imss_register_multi_identity_lines_resp_hdlr
(
  unsigned int                 msg_id,
  std::shared_ptr<void>        resp_c_struct,
  unsigned int                 resp_c_struct_len,
  void                        *resp_cb_data,
  qmi_client_error_type        transp_err
)
{
  QCRIL_LOG_FUNC_ENTRY();
  (void)msg_id;
  (void)resp_c_struct_len;
  RIL_Errno ril_err = RIL_E_SUCCESS;
  ims_settings_set_digits_req_rsp_msg_v01 *resp_msg_ptr = NULL;

  uint32 user_data = (uint32)(uintptr_t) resp_cb_data;
  uint16_t req_id = QCRIL_EXTRACT_USER_ID_FROM_USER_DATA(user_data);

  auto pendingMsg = getImsModule()->getPendingMessageList().extract(req_id);

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
    resp_msg_ptr = (ims_settings_set_digits_req_rsp_msg_v01*)(resp_c_struct.get());
    if (resp_msg_ptr == nullptr)
    {
      QCRIL_LOG_ERROR("resp_msg_ptr is null");
      ril_err = RIL_E_GENERIC_FAILURE;
      break;
    }
    ril_err =
      qcril_qmi_util_convert_qmi_response_codes_to_ril_result(QMI_NO_ERR, &(resp_msg_ptr->resp));

    if( RIL_E_SUCCESS != ril_err )
    {
      QCRIL_LOG_INFO(".. Failed to change IMS handover config");
    }
  } while (FALSE);
  if (pendingMsg) {
    auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(ril_err, nullptr);
    auto ril_msg =
        std::static_pointer_cast<QcRilRequestImsRegisterMultiIdentityMessage>(pendingMsg);
    ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, respPayload);
  }

   QCRIL_LOG_FUNC_RETURN();
} //qcril_qmi_imss_register_multi_identity_lines_resp_hdlr

//===========================================================================
// qcril_qmi_imss_get_ims_config_log_and_send_response
//
// Functionality:
// 1. Reads config item and data from the config params
// 2. If the error is SUCCESS
//    Depending on the type of the items value,
//    process and log the data and generate the
//    response to be sent
// 3. If error reports a failure then check for settings error
//    form the config response accordingly
// 4. And send the reponse with the formed structure
// 5. If there are no config params then send empty response
//    with failure
//===========================================================================
void qcril_qmi_imss_get_ims_config_log_and_send_response
(
   uint16_t req_id,
   const qcril_qmi_radio_config_params_type * const config_params,
   qcril_qmi_radio_config_error_type radio_config_error,
   qcril_qmi_radio_config_settings_resp_type settings_resp
)
{
  QCRIL_LOG_FUNC_ENTRY();

  std::shared_ptr<qcril::interfaces::ConfigInfo> config =
  std::make_shared<qcril::interfaces::ConfigInfo>();

  qcril_qmi_radio_config_item_value_type item_value_type;

  bool send_response = FALSE;
  if (config == nullptr) {
    QCRIL_LOG_INFO("memory allocation failure");
    radio_config_error = QCRIL_QMI_RADIO_CONFIG_ERROR_GENERIC_FAILURE;
  }

  if(config_params != NULL && config != nullptr)
  {

    config->setItem(
      qcril_qmi_ims_map_radio_config_to_ims_config_item(config_params->config_item));
    QCRIL_LOG_INFO("Config item: %s",
                    qcril_qmi_radio_config_get_item_log_str(config_params->config_item));

    /* Get the item value type */
    item_value_type = qcril_qmi_radio_config_get_item_value_type(config_params->config_item);

    /* If the config_item_value exists in config params and error is SUCCESS then
       Process(and log) the config_params data accordingly */
    if(radio_config_error == QCRIL_QMI_RADIO_CONFIG_ERROR_SUCCESS &&
       config_params->config_item_value != NULL &&
       config_params->config_item_value_len >0 &&
       config_params->config_item_value_type == item_value_type)
    {
      /* Evaluate and then assign value correctly */
      switch(item_value_type)
      {
        case QCRIL_QMI_RADIO_CONFIG_ITEM_IS_INT: {
          // If the type is integer
          config->setIntValue(*((uint32_t *)config_params->config_item_value));
          send_response = TRUE;
          QCRIL_LOG_INFO("Config value for %s: %d",
                          qcril_qmi_radio_config_get_item_log_str(config_params->config_item),
                          config->getIntValue());
          break;
        }
        case QCRIL_QMI_RADIO_CONFIG_ITEM_IS_BOOLEAN: {
          // If the type is boolean
          config->setIntValue(*((bool *) config_params->config_item_value));
          send_response = TRUE;
          QCRIL_LOG_INFO("Config value for %s: %d",
                          qcril_qmi_radio_config_get_item_log_str(config_params->config_item),
                          config->getIntValue());
          break;
        }
        case QCRIL_QMI_RADIO_CONFIG_ITEM_IS_STRING: {
          // If the type is String
          char *strVal = (char *)qcril_malloc(config_params->config_item_value_len + 1);
          if(strVal)
          {
            strlcpy(strVal,(char *)config_params->config_item_value,
                    config_params->config_item_value_len + 1);
            config->setStringValue(strVal);
            QCRIL_LOG_INFO("Config value for %s: %s",
                          qcril_qmi_radio_config_get_item_log_str(config_params->config_item),
                          config->getStringValue().c_str());
            send_response = TRUE;
            qcril_free(strVal);
          }
          else
          {
            QCRIL_LOG_ERROR("Malloc failure for string value, send empty failure resp");
            radio_config_error = QCRIL_QMI_RADIO_CONFIG_ERROR_GENERIC_FAILURE;
            send_response = FALSE;
          }
          break;
        }
        default: {
          QCRIL_LOG_ERROR("Unknown data type");
          send_response = FALSE;
          radio_config_error = QCRIL_QMI_RADIO_CONFIG_ERROR_GENERIC_FAILURE;
          break;
        }
      }
    }
    /* If the value is not sent or it is a failure
       then send error cause in the response. */
    else
    {
      config->setErrorCause(
         qcril_qmi_ims_map_radio_config_settings_resp_to_ims_config_failcause(settings_resp));
      QCRIL_LOG_INFO("sending ConfigFailureCause..%d", config->getErrorCause());
      send_response = TRUE;
    }
    if(send_response)
    {
      auto pendingMsg = getImsModule()->getPendingMessageList().extract(req_id);
      if (pendingMsg) {
        RIL_Errno res =
          qcril_qmi_ims_map_radio_config_error_to_ril_error(radio_config_error);
        auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(res, config);
        auto msg(std::static_pointer_cast<QcRilRequestImsGetConfigMessage>(pendingMsg));
        msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
      }
    }
  }
  if(radio_config_error != QCRIL_QMI_RADIO_CONFIG_ERROR_SUCCESS &&
     !send_response)
  {
    QCRIL_LOG_INFO("..sending NULL response");
    auto pendingMsg = getImsModule()->getPendingMessageList().extract(req_id);
    RIL_Errno res =
      qcril_qmi_ims_map_radio_config_error_to_ril_error(radio_config_error);
    auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(res, nullptr);
    if (pendingMsg) {
      if (pendingMsg->get_message_id() ==
        QcRilRequestImsGetConfigMessage::get_class_message_id()) {
        auto msg(std::static_pointer_cast<QcRilRequestImsGetConfigMessage>(pendingMsg));
        msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
      }
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}// qcril_qmi_imss_get_ims_config_log_and_send_response

//===========================================================================
// qcril_qmi_imss_set_ims_config_log_and_send_response
//
// Functionality:
// 1. Get and log the config item from the params
// 2. If the error is FAILURE then put the settings error
//    in the response structure
// 3. Send the response with response structure formed
//    accordingly
// 5. If there are no config params then send empty response
//    with failure
//===========================================================================
void qcril_qmi_imss_set_ims_config_log_and_send_response
(
   uint16_t req_id,
   const qcril_qmi_radio_config_params_type * const config_params,
   qcril_qmi_radio_config_error_type radio_config_error,
   qcril_qmi_radio_config_settings_resp_type settings_resp
)
{
  QCRIL_LOG_FUNC_ENTRY();

  std::shared_ptr<qcril::interfaces::ConfigInfo> config =
  std::make_shared<qcril::interfaces::ConfigInfo>();

  if (config == nullptr) {
    QCRIL_LOG_INFO("memory allocation failure");
    radio_config_error = QCRIL_QMI_RADIO_CONFIG_ERROR_GENERIC_FAILURE;
  }

  if(config_params != NULL && config != nullptr)
  {
    /* Get Config item */
    config->setItem(qcril_qmi_ims_map_radio_config_to_ims_config_item(config_params->config_item));
    QCRIL_LOG_INFO("Config item: %s",
                    qcril_qmi_radio_config_get_item_log_str(config_params->config_item));

    /* If error reported then get error */
    if(radio_config_error != QCRIL_QMI_RADIO_CONFIG_ERROR_SUCCESS)
    {
      config->setErrorCause(
        qcril_qmi_ims_map_radio_config_settings_resp_to_ims_config_failcause(settings_resp));
      QCRIL_LOG_INFO("sending ConfigFailureCause..%d", config->getErrorCause());
    }
    auto pendingMsg = getImsModule()->getPendingMessageList().extract(req_id);
    if (pendingMsg) {
      RIL_Errno res =
        qcril_qmi_ims_map_radio_config_error_to_ril_error(radio_config_error);
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(res, config);
      auto msg(std::static_pointer_cast<QcRilRequestImsSetConfigMessage>(pendingMsg));
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
    }
  }
  else
  {
    if(radio_config_error != QCRIL_QMI_RADIO_CONFIG_ERROR_SUCCESS)
    {
      auto pendingMsg = getImsModule()->getPendingMessageList().extract(req_id);
      RIL_Errno res =
        qcril_qmi_ims_map_radio_config_error_to_ril_error(radio_config_error);
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(res, nullptr);
      if (pendingMsg) {
        if (pendingMsg->get_message_id() ==
          QcRilRequestImsSetConfigMessage::get_class_message_id()) {
          auto msg(std::static_pointer_cast<QcRilRequestImsSetConfigMessage>(pendingMsg));
          msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
        }
      }
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}// qcril_qmi_imss_set_ims_config_log_and_send_response


//===========================================================================
// qcril_qmi_imss_dispatch_config_response_helper
//
// Functionality:
// 1. Depending on req event id gets the specific handler
//    i.e. get - get_resp_handler and set - set_resp_handler
// 2. Stores the response params and config item
//    in the config params and calls the handler
//    with config params
//===========================================================================
void qcril_qmi_imss_dispatch_config_response_helper
(
   const qcril_qmi_radio_config_resp_data_type *const resp_ptr,
   int config_item,
   unsigned long msg_id,
   uint16_t req_id
)
{
  QCRIL_LOG_FUNC_ENTRY();
  qcril_qmi_radio_config_handler_type* resp_handler = NULL;
  qcril_qmi_radio_config_params_type config_params;
  memset(&config_params, 0, sizeof(config_params));


  bool send_failure_resp = TRUE;
  if(resp_ptr != NULL)
  {
    auto pendingMsg = getImsModule()->getPendingMessageList().find(req_id);
    if (pendingMsg == nullptr) {
        QCRIL_LOG_INFO("No pending request");
        return;
    }
    if(pendingMsg->get_message_id() == QcRilRequestImsGetConfigMessage::get_class_message_id())
    {
      if(config_item > QCRIL_QMI_RADIO_CONFIG_INVALID)
      {
        QCRIL_LOG_INFO("get");
        resp_handler = qcril_qmi_radio_config_validate_and_find_get_config_resp_handler(
                        (qcril_qmi_radio_config_item_type)config_item, msg_id);
        //form up the function parameter with config item and other data in extra data
        config_params.config_item = (qcril_qmi_radio_config_item_type)config_item;
        config_params.config_item_value_len = 0;
        config_params.config_item_value = NULL;
        config_params.extra_data_len = resp_ptr->data_len;
        config_params.extra_data = (void *)resp_ptr->data;

        if(resp_handler != NULL )
        {
          (resp_handler)(&config_params, req_id);
          send_failure_resp = FALSE;
        }
      }
      if(send_failure_resp) {
        qcril_qmi_imss_get_ims_config_log_and_send_response(req_id,
                                                NULL,
                                                QCRIL_QMI_RADIO_CONFIG_ERROR_GENERIC_FAILURE,
                                                QCRIL_QMI_RADIO_CONFIG_SETTINGS_RESP_NO_ERR);

      }
    }
    else if(pendingMsg->get_message_id() ==
        QcRilRequestImsSetConfigMessage::get_class_message_id())
    {
      if(config_item > QCRIL_QMI_RADIO_CONFIG_INVALID) {
        QCRIL_LOG_INFO("set");
        resp_handler = qcril_qmi_radio_config_validate_and_find_set_config_resp_handler(
            (qcril_qmi_radio_config_item_type)config_item, msg_id);
        //form up the function parameter with config item and other data in extra data
        config_params.config_item = (qcril_qmi_radio_config_item_type)config_item;
        config_params.config_item_value_len = 0;
        config_params.config_item_value = NULL;
        config_params.extra_data_len = resp_ptr->data_len;
        config_params.extra_data = (void *)resp_ptr->data;

        if(resp_handler != NULL )
        {
          (resp_handler)(&config_params, req_id);
          send_failure_resp = FALSE;
        }
      }
      if(send_failure_resp) {
        qcril_qmi_imss_set_ims_config_log_and_send_response(req_id,
                                            NULL,
                                            QCRIL_QMI_RADIO_CONFIG_ERROR_GENERIC_FAILURE,
                                            QCRIL_QMI_RADIO_CONFIG_SETTINGS_RESP_NO_ERR);
      }
    }
  }
  else
  {
    QCRIL_LOG_ERROR("params are null, cannot send response without token");
  }
  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// qcril_qmi_imss_config_resp_cb
//===========================================================================
void qcril_qmi_imss_config_resp_cb
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
// qcril_qmi_imss_command_cb
//===========================================================================
void qcril_qmi_imss_command_cb
(
   unsigned int                 msg_id,
   std::shared_ptr<void>        resp_c_struct,
   unsigned int                 resp_c_struct_len,
   void                        *resp_cb_data,
   qmi_client_error_type        transp_err
)
{
  qcril_instance_id_e_type instance_id;
  uint32 user_data;
  uint16 req_id;
  qcril_reqlist_public_type req_info;
  qcril_request_params_type req_data;

  QCRIL_LOG_FUNC_ENTRY();

  user_data = ( uint32 )(uintptr_t) resp_cb_data;
  instance_id = (qcril_instance_id_e_type)QCRIL_EXTRACT_INSTANCE_ID_FROM_USER_DATA( user_data );
  req_id = QCRIL_EXTRACT_USER_ID_FROM_USER_DATA( user_data );

  memset(&req_data, 0, sizeof(req_data));
  req_data.modem_id = QCRIL_DEFAULT_MODEM_ID;
  req_data.instance_id = instance_id;
  req_data.datalen = resp_c_struct_len;
  req_data.data = resp_c_struct.get();

  /* Lookup the Token ID */
  if ( qcril_reqlist_query_by_req_id( req_id, &instance_id, &req_info ) == E_SUCCESS )
  {
    if( transp_err != QMI_NO_ERR )
    {
      QCRIL_LOG_INFO("Transp error (%d) recieved from QMI for RIL request %d",
          transp_err, req_info.request);
      /* Send GENERIC_FAILURE response */
      qcril_send_empty_payload_request_response(instance_id, req_info.t, req_info.request,
          RIL_E_GENERIC_FAILURE);
    }
    else
    {
      req_data.t = req_info.t;
      req_data.event_id = req_info.request;
      switch(msg_id)
      {
        default:
          QCRIL_LOG_INFO("Unsupported QMI IMSS message %d", msg_id);
          break;
      }
    }
  }
  else
  {
    QCRIL_LOG_ERROR( "Req ID: %d not found", req_id );
  }

  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imss_command_cb

//===========================================================================
// qcril_qmi_imss_get_client_provisioning_config
//===========================================================================
void qcril_qmi_imss_get_client_provisioning_config()
{
   qmi_client_error_type qmi_client_error = QMI_NO_ERR;
   ims_settings_get_client_provisioning_config_rsp_msg_v01 qmi_ims_get_client_provisioning_config_resp;
   uint8_t wifi_call_valid = FALSE;
   ims_settings_wfc_status_enum_v01 wifi_call;
   uint8_t wifi_call_preference_valid = FALSE;
   ims_settings_wfc_preference_v01 wifi_call_preference;
   bool send_update_to_data = false;

   QCRIL_LOG_FUNC_ENTRY();

   qcril_qmi_imss_info_lock();
   wifi_call_valid = qcril_qmi_ims_cached_info.wifi_calling_enabled_valid;
   wifi_call = qcril_qmi_imss_convert_ril_to_imss_wfc_status(
           qcril_qmi_ims_cached_info.wifi_calling_enabled);
   wifi_call_preference_valid = qcril_qmi_ims_cached_info.call_mode_preference_valid;
   wifi_call_preference = qcril_qmi_imss_convert_ril_to_imss_wfc_preference(
           qcril_qmi_ims_cached_info.call_mode_preference);
   qcril_qmi_imss_info_unlock();

   if(wifi_call_preference_valid && wifi_call_valid)
   {
      QCRIL_LOG_INFO("wifi_call status %d, wifi_call_preference: %d", wifi_call, wifi_call_preference);
      return;
   }

   memset(&qmi_ims_get_client_provisioning_config_resp, 0x0,
       sizeof(qmi_ims_get_client_provisioning_config_resp));

   qmi_client_error = qmi_client_imss_send_sync(
       QMI_IMS_SETTINGS_GET_CLIENT_PROVISIONING_CONFIG_REQ_V01,
       NULL,
       0,
       &qmi_ims_get_client_provisioning_config_resp,
       sizeof(qmi_ims_get_client_provisioning_config_resp));
   QCRIL_LOG_INFO(".. qmi send sync res %d", (int) qmi_client_error );

   if (qmi_client_error == QMI_NO_ERR)
   {
      if( qmi_ims_get_client_provisioning_config_resp.resp.result == QMI_RESULT_SUCCESS_V01 )
      {
          qcril_qmi_imss_info_lock();
          qcril_qmi_ims_cached_info.wifi_calling_enabled_valid = qmi_ims_get_client_provisioning_config_resp.wifi_call_valid;
          qcril_ims_setting_wfc_status_type old_value_status = qcril_qmi_ims_cached_info.wifi_calling_enabled;
          qcril_qmi_ims_cached_info.wifi_calling_enabled = qcril_qmi_imss_convert_imss_to_ril_wfc_status(
            qmi_ims_get_client_provisioning_config_resp.wifi_call);
          if(old_value_status != qcril_qmi_ims_cached_info.wifi_calling_enabled)
          {
            send_update_to_data = true;
          }
          qcril_qmi_ims_cached_info.call_mode_preference_valid = qmi_ims_get_client_provisioning_config_resp.wifi_call_preference_valid;
          qcril_ims_setting_wfc_preference_type old_value_pref = qcril_qmi_ims_cached_info.call_mode_preference;
          qcril_qmi_ims_cached_info.call_mode_preference = qcril_qmi_imss_convert_imss_to_ril_wfc_preference(
                qmi_ims_get_client_provisioning_config_resp.wifi_call_preference);
          if(old_value_pref != qcril_qmi_ims_cached_info.call_mode_preference)
          {
            send_update_to_data = true;
          }

          QCRIL_LOG_INFO(".. client_prov_enabled_valid: %d, client_prov_enabled: %d",
            qmi_ims_get_client_provisioning_config_resp.enable_client_provisioning_valid,
            qmi_ims_get_client_provisioning_config_resp.enable_client_provisioning);
          QCRIL_LOG_INFO(".. wifi_call_valid: %d, wifi_call: %d", qcril_qmi_ims_cached_info.wifi_calling_enabled_valid,
            qcril_qmi_ims_cached_info.wifi_calling_enabled);
          QCRIL_LOG_INFO(".. wifi_call_preference_valid: %d, wifi_call_preference: %d", qcril_qmi_ims_cached_info.call_mode_preference_valid,
            qcril_qmi_ims_cached_info.call_mode_preference);

          qcril_qmi_imss_info_unlock();
      }
   }

   if (send_update_to_data)
   {
     qcril_qmi_imss_update_wifi_pref_from_ind_to_qcril_data();
   }

   /* IMSS REFACTOR: TEST */
   qcril_qmi_imss_broadcast_wfc_settings(qcril_qmi_ims_cached_info);

   QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imss_get_client_provisioning_config

//===========================================================================
// qcril_qmi_imss_client_provisioning_config_ind_hdlr
//===========================================================================
void qcril_qmi_imss_client_provisioning_config_ind_hdlr(void *ind_data_ptr)
{
  ims_settings_client_provisioning_config_ind_msg_v01 *reg_ind_msg_ptr =
      (ims_settings_client_provisioning_config_ind_msg_v01 *)ind_data_ptr;
  bool send_update_to_data = false;

  QCRIL_LOG_FUNC_ENTRY();

  if (NULL != reg_ind_msg_ptr)
  {
    qcril_qmi_imss_info_lock();
    if (reg_ind_msg_ptr->wifi_call_valid)
    {
      qcril_qmi_ims_cached_info.wifi_calling_enabled_valid = reg_ind_msg_ptr->wifi_call_valid;
      qcril_ims_setting_wfc_status_type old_value_status =
          qcril_qmi_ims_cached_info.wifi_calling_enabled;
      qcril_qmi_ims_cached_info.wifi_calling_enabled =
          qcril_qmi_imss_convert_imss_to_ril_wfc_status(reg_ind_msg_ptr->wifi_call);
      if (old_value_status != qcril_qmi_ims_cached_info.wifi_calling_enabled)
      {
        send_update_to_data = true;
      }
    }
    if (reg_ind_msg_ptr->wifi_call_preference_valid)
    {
      qcril_qmi_ims_cached_info.call_mode_preference_valid =
          reg_ind_msg_ptr->wifi_call_preference_valid;
      qcril_ims_setting_wfc_preference_type old_value_pref =
          qcril_qmi_ims_cached_info.call_mode_preference;
      qcril_qmi_ims_cached_info.call_mode_preference =
          qcril_qmi_imss_convert_imss_to_ril_wfc_preference(reg_ind_msg_ptr->wifi_call_preference);
      if (old_value_pref != qcril_qmi_ims_cached_info.call_mode_preference)
      {
        send_update_to_data = true;
      }
    }
    QCRIL_LOG_INFO(".. client_prov_enabled_valid: %d, client_prov_enabled: %d",
                   reg_ind_msg_ptr->client_provisioning_enabled_valid,
                   reg_ind_msg_ptr->client_provisioning_enabled);
    QCRIL_LOG_INFO(".. wifi_call_valid: %d, wifi_call: %d",
                   qcril_qmi_ims_cached_info.wifi_calling_enabled_valid,
                   qcril_qmi_ims_cached_info.wifi_calling_enabled);
    QCRIL_LOG_INFO(".. wifi_call_preference_valid: %d, wifi_call_preference: %d",
                   qcril_qmi_ims_cached_info.call_mode_preference_valid,
                   qcril_qmi_ims_cached_info.call_mode_preference);

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
// qcril_qmi_imss_unsol_ind_cb_helper
//===========================================================================
void qcril_qmi_imss_unsol_ind_cb_helper
(
 unsigned int   msg_id,
 unsigned char *decoded_payload,
 uint32_t       decoded_payload_len
)
{
  QCRIL_LOG_FUNC_ENTRY();

  if (decoded_payload || !decoded_payload_len)
  {
    switch(msg_id)
    {
      case QMI_IMS_SETTINGS_CLIENT_PROVISIONING_CONFIG_IND_V01:
        qcril_qmi_imss_client_provisioning_config_ind_hdlr(decoded_payload);
        break;

      default:
        QCRIL_LOG_INFO("Unknown QMI IMSA indication %d", msg_id);
        break;
    }
  }

  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imss_unsol_ind_cb_helper

//===========================================================================
// qcril_qmi_imss_send_set_qipcall_config_req
//===========================================================================
RIL_Errno qcril_qmi_imss_send_set_qipcall_config_req
(
 const ims_settings_set_qipcall_config_req_msg_v01 *qmi_req,
 uint32 user_data
)
{
  qmi_client_error_type qmi_client_error = QMI_NO_ERR;
  RIL_Errno res = RIL_E_GENERIC_FAILURE;

  QCRIL_LOG_FUNC_ENTRY();

  qmi_client_error = qmi_client_imss_send_async(
      QMI_IMS_SETTINGS_SET_QIPCALL_CONFIG_REQ_V01,
      (void *)qmi_req,
      sizeof(ims_settings_set_qipcall_config_req_msg_v01),
      sizeof(ims_settings_set_qipcall_config_rsp_msg_v01),
      qcril_qmi_imss_set_qipcall_config_resp_hdlr,
      (void*)(uintptr_t)user_data);


  QCRIL_LOG_INFO(".. qmi send async res %d", (int) qmi_client_error);

  if (qmi_client_error == QMI_NO_ERR)
  {
    res = RIL_E_SUCCESS;
  }

  QCRIL_LOG_FUNC_RETURN();

  return res;
} // qcril_qmi_imss_send_set_qipcall_config_req

//===========================================================================
//  qcril_qmi_imss_get_modem_version
//  @brief
//    modem version values:
//    0 - Old IMSS APIs
//    1 - New IMSS APIs
//===========================================================================
uint8_t qcril_qmi_imss_get_modem_version(void)
{
  return modem_version;
}

uint8_t qcril_qmi_imss_is_modem_supports_wfc_preferred_ims(void)
{
  return modem_supports_wfc_preferred_ims;
}

//===========================================================================
// qcril_qmi_ims_is_msg_supported
//===========================================================================
boolean qcril_qmi_ims_is_msg_supported
(
  int message_id, uint8_t supported_msgs[], uint32_t supported_msgs_len
)
{
  uint32_t index = message_id/8;
  uint8_t bit_position = message_id - index * 8;
  uint8_t bit_position_mask = 0x01 << bit_position;

  if(index < supported_msgs_len)
  {
    return ((supported_msgs[index] & bit_position_mask) == bit_position_mask);
  }
  else
  {
    return FALSE;
  }
}

//===========================================================================
// qcril_qmi_ims_is_field_supported
//===========================================================================
boolean qcril_qmi_ims_is_field_supported
(
  int field_id, uint8_t response_fields[], uint32_t response_fields_len
)
{
  uint32_t index = field_id/8;
  uint8_t bit_position = field_id - index * 8;
  uint8_t bit_position_mask = 0x01 << bit_position;

  if(index < response_fields_len)
  {
    return ((response_fields[index] & bit_position_mask) == bit_position_mask);
  }
  else
  {
    return FALSE;
  }
}


void qcril_qmi_imss_update_modem_version(void)
{
    qmi_client_error_type qmi_client_error = QMI_NO_ERR;
    qmi_get_supported_msgs_resp_v01 qmi_resp;

    QCRIL_LOG_FUNC_ENTRY();

    memset(&qmi_resp, 0x0, sizeof(qmi_resp));
    qmi_client_error = qmi_client_imss_send_sync(
            QMI_IMS_SETTINGS_GET_SUPPORTED_MSGS_REQ_V01,
            NULL,
            0,
            &qmi_resp,
            sizeof(qmi_resp));
    QCRIL_LOG_INFO(".. qmi send sync res %d", (int) qmi_client_error);

    if (qmi_client_error == QMI_NO_ERR)
    {
        if(qmi_resp.supported_msgs_valid)
        {
          //Use new modem version if ims service enable config request is supported.
          modem_version = qcril_qmi_ims_is_msg_supported(
            QMI_IMS_SETTINGS_SET_IMS_SERVICE_ENABLE_CONFIG_REQ_V01, qmi_resp.supported_msgs,
            qmi_resp.supported_msgs_len);
        }
    }
    QCRIL_LOG_INFO("modem_version = %d", modem_version);
}

void qcril_qmi_imss_update_modem_features(void)
{
    qmi_client_error_type qmi_client_error = QMI_NO_ERR;
    qmi_get_supported_fields_req_v01 qmi_req;
    qmi_get_supported_fields_resp_v01 qmi_resp;
    uint32_t i = 0;

    QCRIL_LOG_FUNC_ENTRY();

    memset(&qmi_req, 0x0, sizeof(qmi_req));
    memset(&qmi_resp, 0x0, sizeof(qmi_resp));

    qmi_req.msg_id = QMI_IMS_SETTINGS_SET_IMS_SERVICE_ENABLE_CONFIG_REQ_V01;
    qmi_client_error = qmi_client_imss_send_sync(
            QMI_IMS_SETTINGS_GET_SUPPORTED_FIELDS_REQ_V01,
            &qmi_req,
            sizeof(qmi_req),
            &qmi_resp,
            sizeof(qmi_resp));
    QCRIL_LOG_INFO(".. qmi send sync res %d", (int) qmi_client_error);

    if (qmi_client_error == QMI_NO_ERR)
    {
        if(qmi_resp.request_fields_valid)
        {
          for (i=0; i<qmi_resp.request_fields_len;i++)
          {
              QCRIL_LOG_INFO("request field = %d, value = 0x%X", i, qmi_resp.request_fields[i]);
          }
          modem_supports_wfc_roaming_config = qcril_qmi_ims_is_field_supported(
            BIT_POS_WFC_ROAMING_MODE_PREFERENCE, qmi_resp.request_fields,
            qmi_resp.request_fields_len);
          modem_supports_wfc_preferred_ims = qcril_qmi_ims_is_field_supported(
            BIT_POS_WFC_PREFERENCE_IMS, qmi_resp.request_fields,
            qmi_resp.request_fields_len);
        }
    }
    QCRIL_LOG_INFO("modem_supports_wfc_roaming_config = %d, modem_supports_wfc_preferred_ims = %d",
                   modem_supports_wfc_roaming_config, modem_supports_wfc_preferred_ims);
    qcril_qmi_ims_send_unsol_wfc_roaming_config();
}

//===========================================================================
//  qcril_qmi_imss_convert_ril_to_imss_wfc_status
//===========================================================================
ims_settings_wfc_status_enum_v01 qcril_qmi_imss_convert_ril_to_imss_wfc_status
(
  qcril_ims_setting_wfc_status_type wfc_status
)
{
  ims_settings_wfc_status_enum_v01 res;

  switch(wfc_status)
  {
    case QCRIL_IMS_SETTING_WFC_NOT_SUPPORTED:
      res = IMS_SETTINGS_WFC_STATUS_NOT_SUPPORTED_V01;
      break;

    case QCRIL_IMS_SETTING_WFC_OFF:
      res = IMS_SETTINGS_WFC_STATUS_OFF_V01;
      break;

    case QCRIL_IMS_SETTING_WFC_ON:
      res = IMS_SETTINGS_WFC_STATUS_ON_V01;
      break;

    default:
      res = IMS_SETTINGS_WFC_STATUS_NOT_SUPPORTED_V01;
  }
  return res;
}

//===========================================================================
//  qcril_qmi_imss_convert_ril_to_imss_wfc_preference
//===========================================================================
ims_settings_wfc_preference_v01 qcril_qmi_imss_convert_ril_to_imss_wfc_preference
(
  qcril_ims_setting_wfc_preference_type wfc_preference
)
{
  ims_settings_wfc_preference_v01 res;

  switch(wfc_preference)
  {
    case QCRIL_IMS_SETTING_WFC_WLAN_PREFERRED:
      res = IMS_SETTINGS_WFC_WLAN_PREFERRED_V01;
      break;

    case QCRIL_IMS_SETTING_WFC_WLAN_ONLY:
      res = IMS_SETTINGS_WFC_WLAN_ONLY_V01;
      break;

    case QCRIL_IMS_SETTING_WFC_CELLULAR_PREFERRED:
      res = IMS_SETTINGS_WFC_CELLULAR_PREFERRED_V01;
      break;

    case QCRIL_IMS_SETTING_WFC_CELLULAR_ONLY:
      res = IMS_SETTINGS_WFC_CELLULAR_ONLY_V01;
      break;

    default:
      res = IMS_SETTINGS_WFC_CALL_PREF_NONE_V01;
  }
  return res;
}

//===========================================================================
//  qcril_qmi_imss_convert_imss_to_ril_wfc_status
//===========================================================================
qcril_ims_setting_wfc_status_type qcril_qmi_imss_convert_imss_to_ril_wfc_status
(
  ims_settings_wfc_status_enum_v01 wfc_status
)
{
  qcril_ims_setting_wfc_status_type res;

  switch(wfc_status)
  {
    case IMS_SETTINGS_WFC_STATUS_NOT_SUPPORTED_V01:
      res = QCRIL_IMS_SETTING_WFC_NOT_SUPPORTED;
      break;

    case IMS_SETTINGS_WFC_STATUS_OFF_V01:
      res = QCRIL_IMS_SETTING_WFC_OFF;
      break;

    case IMS_SETTINGS_WFC_STATUS_ON_V01:
      res = QCRIL_IMS_SETTING_WFC_ON;
      break;

    default:
      res = QCRIL_IMS_SETTING_WFC_NOT_SUPPORTED;
  }
  return res;
}
//===========================================================================
//  qcril_qmi_imss_convert_imss_to_ril_wfc_preference
//===========================================================================
qcril_ims_setting_wfc_preference_type qcril_qmi_imss_convert_imss_to_ril_wfc_preference
(
  ims_settings_wfc_preference_v01 wfc_preference
)
{
  qcril_ims_setting_wfc_preference_type res;

  switch(wfc_preference)
  {
    case IMS_SETTINGS_WFC_WLAN_PREFERRED_V01:
      res = QCRIL_IMS_SETTING_WFC_WLAN_PREFERRED;
      break;

    case IMS_SETTINGS_WFC_WLAN_ONLY_V01:
      res = QCRIL_IMS_SETTING_WFC_WLAN_ONLY;
      break;

    case IMS_SETTINGS_WFC_CELLULAR_PREFERRED_V01:
      res = QCRIL_IMS_SETTING_WFC_CELLULAR_PREFERRED;
      break;

    case IMS_SETTINGS_WFC_CELLULAR_ONLY_V01:
      res = QCRIL_IMS_SETTING_WFC_CELLULAR_ONLY;
      break;

    default:
      res = QCRIL_IMS_SETTING_WFC_PREF_NONE;
  }
  return res;
}

static ImsWfcSettingsStatusInd::Status qcril_qmi_imss_convert_qmi_status_to_imss_status (
    qcril_ims_setting_wfc_status_type wfcStatus) {
  switch (wfcStatus) {
    case QCRIL_IMS_SETTING_WFC_NOT_SUPPORTED:
      return ImsWfcSettingsStatusInd::Status::WFC_NOT_SUPPORTED;
      break;

    case QCRIL_IMS_SETTING_WFC_OFF:
      return ImsWfcSettingsStatusInd::Status::WFC_OFF;
      break;

    case QCRIL_IMS_SETTING_WFC_ON:
      return ImsWfcSettingsStatusInd::Status::WFC_ON;
      break;

    default:
      return ImsWfcSettingsStatusInd::Status::INVALID;
      break;
  }
}

static ImsWfcSettingsStatusInd::Preference qcril_qmi_imss_convert_qmi_pref_to_imss_pref (
    qcril_ims_setting_wfc_preference_type wfcPref) {
  switch (wfcPref) {
    case QCRIL_IMS_SETTING_WFC_WLAN_PREFERRED:
      return ImsWfcSettingsStatusInd::Preference::WLAN_PREFERRED;
      break;
    case QCRIL_IMS_SETTING_WFC_WLAN_ONLY:
      return ImsWfcSettingsStatusInd::Preference::WLAN_ONLY;
      break;
    case QCRIL_IMS_SETTING_WFC_CELLULAR_PREFERRED:
      return ImsWfcSettingsStatusInd::Preference::CELLULAR_PREFERRED;
      break;
    case QCRIL_IMS_SETTING_WFC_CELLULAR_ONLY:
      return ImsWfcSettingsStatusInd::Preference::CELLULAR_ONLY;
      break;
    case QCRIL_IMS_SETTING_WFC_PREF_NONE:
      return ImsWfcSettingsStatusInd::Preference::NONE;
      break;
    default:
      return ImsWfcSettingsStatusInd::Preference::INVALID;
      break;
  }
}

void qcril_qmi_imss_broadcast_wfc_settings (const struct ims_cached_info_type& ims_info) {
  ImsWfcSettingsStatusInd::StatusInfo imssWfcStatus;

  QCRIL_LOG_FUNC_ENTRY();

  imssWfcStatus.status = ims_info.wifi_calling_enabled ?
    qcril_qmi_imss_convert_qmi_status_to_imss_status(ims_info.wifi_calling_enabled) :
    ImsWfcSettingsStatusInd::Status::INVALID;
  imssWfcStatus.preference = ims_info.call_mode_preference_valid ?
    qcril_qmi_imss_convert_qmi_pref_to_imss_pref(ims_info.call_mode_preference) :
    ImsWfcSettingsStatusInd::Preference::INVALID;

  QCRIL_LOG_DEBUG("Broadcasting ImsWfcSettingsStatusInd");
  std::shared_ptr<ImsWfcSettingsStatusInd> msg =
    std::make_shared<ImsWfcSettingsStatusInd>(imssWfcStatus);
  Dispatcher::getInstance().broadcast(msg);

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// qcril_qmi_ims_map_radio_config_error_to_ims_error
//===========================================================================
static RIL_Errno qcril_qmi_ims_map_radio_config_error_to_ril_error
(
 qcril_qmi_radio_config_error_type radio_config_error
)
{
  RIL_Errno ret;

  switch ( radio_config_error )
  {
    case QCRIL_QMI_RADIO_CONFIG_ERROR_SUCCESS:
      ret = RIL_E_SUCCESS;
      break;

    case QCRIL_QMI_RADIO_CONFIG_ERROR_REQUEST_NOT_SUPPORTED:
      ret = RIL_E_REQUEST_NOT_SUPPORTED;
      break;

    default:
      ret = RIL_E_GENERIC_FAILURE;
      break;
  }

  QCRIL_LOG_INFO("radio config error %d mapped to ims error %d", radio_config_error, ret);
  return ret;
}// qcril_qmi_ims_map_radio_config_error_to_ims_error

//===========================================================================
// qcril_qmi_ims_map_ims_settings_wfc_preference_to_dsd_rat_preference
//===========================================================================
static boolean qcril_qmi_ims_map_ims_settings_wfc_preference_to_dsd_rat_preference
(
 const qcril_ims_setting_wfc_preference_type wifi_call_preference,
 rildata::RatPreference &dsd_rat_preference
)
{
  boolean result = TRUE;
  switch (wifi_call_preference)
  {
    case QCRIL_IMS_SETTING_WFC_PREF_NONE:
      dsd_rat_preference = rildata::RatPreference::Inactive;
      break;
    case QCRIL_IMS_SETTING_WFC_WLAN_PREFERRED:
      dsd_rat_preference = rildata::RatPreference::WifiPreferred;
      break;
    case QCRIL_IMS_SETTING_WFC_WLAN_ONLY:
      dsd_rat_preference = rildata::RatPreference::WifiOnly;
      break;
    case QCRIL_IMS_SETTING_WFC_CELLULAR_PREFERRED:
      dsd_rat_preference = rildata::RatPreference::CellularPreferred;
      break;
    case QCRIL_IMS_SETTING_WFC_CELLULAR_ONLY:
      dsd_rat_preference = rildata::RatPreference::CellularOnly;
      break;
    case QCRIL_IMS_SETTING_WFC_IMS_PREFERRED:
      dsd_rat_preference = rildata::RatPreference::ImsPreferred;
      break;
    default:
      result = FALSE;
      break;
  }
  return result;
}
//===========================================================================
// qcril_qmi_imss_update_wifi_pref_from_ind_to_qcril_data
//===========================================================================
void qcril_qmi_imss_update_wifi_pref_from_ind_to_qcril_data()
{
  QCRIL_LOG_FUNC_ENTRY();
  qcril_ims_setting_wfc_preference_type wfc_pref = QCRIL_IMS_SETTING_WFC_PREF_NONE;
  RIL_Errno ril_err = RIL_E_GENERIC_FAILURE;
  rildata::RatPreference dsd_rat_preference = rildata::RatPreference::Inactive;

  qcril_qmi_imss_info_lock();
  if (qcril_qmi_ims_cached_info.call_mode_preference_valid)
  {
    wfc_pref = qcril_qmi_ims_cached_info.call_mode_preference;
  }
  if (qcril_qmi_ims_cached_info.wifi_calling_enabled_valid &&
      qcril_qmi_ims_cached_info.wifi_calling_enabled == QCRIL_IMS_SETTING_WFC_OFF)
  {
    // if WFC is off - force the preference to be CELLULAR ONLY
    // as data expects it to be CELLULAR ONLY when WFC is OFF
    wfc_pref = QCRIL_IMS_SETTING_WFC_CELLULAR_ONLY;
  }
  qcril_qmi_imss_info_unlock();

  if (qcril_qmi_ims_map_ims_settings_wfc_preference_to_dsd_rat_preference(wfc_pref,
                                                                          dsd_rat_preference))
  {
#ifndef QMI_RIL_UTF
    // Set RAT preference from QCRIL DATA;
    std::shared_ptr<rildata::SetRatPrefMessage> msg =
        std::make_shared<rildata::SetRatPrefMessage>(dsd_rat_preference);
    if (msg)
    {
      msg->dispatch();
      QCRIL_LOG_INFO("Delivered rat preference to Data: %d", dsd_rat_preference);
      /* We donot process the error from Data - as the intention is only
         to inform Data about the wifi settings */
      ril_err = RIL_E_SUCCESS;
    }
#endif
  }
  else
  {
    QCRIL_LOG_ERROR("failed to map preference to send it to data");
  }
  QCRIL_LOG_FUNC_RETURN();
}
