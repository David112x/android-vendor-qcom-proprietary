/******************************************************************************
#  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include "common_v01.h"
#include "user_identity_module_v01.h"
#include "framework/ModuleLooper.h"
#include "modules/qmi/QmiServiceUpIndMessage.h"
#include "modules/qmi/QmiServiceDownIndMessage.h"
#include "modules/uim/UimQmiUimIndicationMsg.h"
#include "modules/uim/UimQmiUimRequestMsg.h"
#include "modules/uim/UimQmiUimResponseMsg.h"
#include "UimModemEndPointModule.h"
#include "UimModemQcci.h"

#ifdef QMI_RIL_UTF
#include "ril_utf_qmi_sim.h"
#endif

#define TAG "UimModemEndPointModule"

/*===========================================================================

  FUNCTION  UimModemEndPointModule - Constructor

===========================================================================*/
UimModemEndPointModule::UimModemEndPointModule
(
  string         name,
  ModemEndPoint *owner
)
{
  mName = name;
  mOwner = owner;
  mServiceObject = nullptr;
  mQmiSvcClient = nullptr;
  mLooper = std::unique_ptr<ModuleLooper>(new ModuleLooper);

  using std::placeholders::_1;

  mMessageHandler = {
       HANDLER(QmiUimSetupRequestSync, UimModemEndPointModule::handleQmiClientSetup),
       HANDLER(QmiServiceUpIndMessage, UimModemEndPointModule::handleQmiServiceUp),
       HANDLER(QmiServiceDownIndMessage, UimModemEndPointModule::handleQmiServiceDown),
       HANDLER(UimQmiUimRequestMsg, UimModemEndPointModule::handleQcrilUimRequest),
  };
} /* UimModemEndPointModule::UimModemEndPointModule */


/*===========================================================================

  FUNCTION  UimModemEndPointModule - init

===========================================================================*/
void UimModemEndPointModule::init()
{
  QCRIL_LOG_INFO( "init\n");

  /* Call base init before doing any other stuff.*/
  Module::init();
} /* UimModemEndPointModule::init */


/*===========================================================================

  FUNCTION  UimModemEndPointModule::~UimModemEndPointModule

===========================================================================*/
UimModemEndPointModule::~UimModemEndPointModule()
{
  QCRIL_LOG_INFO( "%s", __FUNCTION__);

  if (mQmiNotifyHandle != nullptr)
  {
    (void)qmi_client_release(mQmiNotifyHandle);
    mQmiNotifyHandle = nullptr;
  }
  if (mQmiSvcClient != nullptr)
  {
    (void)qmi_client_release(mQmiSvcClient);
    mQmiSvcClient = nullptr;
  }
  mLooper = nullptr;
} /* UimModemEndPointModule::~UimModemEndPointModule */


/*===========================================================================

  FUNCTION  onQmiNotifyCb - UimModemEndPointModule QMI client notfier
                                    callback handler

===========================================================================*/
void UimModemEndPointModule::onQmiNotifyCb
(
  qmi_client_type                user_handle,
  qmi_idl_service_object_type    service_obj,
  qmi_client_notify_event_type   service_event,
  void                         * notify_cb_data
)
{
  UimModemEndPointModule *module = (UimModemEndPointModule *)notify_cb_data;

  if (module == NULL)
  {
    return;
  }

  (void)user_handle;
  (void)service_obj;

  switch (service_event)
  {
    case QMI_CLIENT_SERVICE_COUNT_INC:
      {
        QCRIL_LOG_INFO("qmiNotifyCb() Service up indication");
        module->dispatch(std::shared_ptr<Message>(new QmiServiceUpIndMessage(nullptr)));
      }
      break;

    case QMI_CLIENT_SERVICE_COUNT_DEC:
      {
        QCRIL_LOG_INFO("qmiNotifyCb() Service down indication");
        module->dispatch(std::shared_ptr<Message>(new QmiServiceDownIndMessage(QMI_SERVICE_ERR)));
      }
      break;

    default:
      QCRIL_LOG_INFO("qmiNotifyCb() unsupported service event %d", service_event);
      break;
  }
} /* UimModemEndPointModule::onQmiNotifyCb */


/*===========================================================================
  FUNCTION  handleQmiClientSetup -  UimModemEndPointModule QMI Client setup
                                    requet handler
===========================================================================*/
void UimModemEndPointModule::handleQmiClientSetup
(
  std::shared_ptr<QmiUimSetupRequestSync> msg
)
{
  QCRIL_LOG_INFO( "handleQmiClientSetup : %d", mOwner->getState());

  if (mServiceObject == nullptr)
  {
    mServiceObject = uim_get_service_object_v01();

    if (mServiceObject == nullptr)
    {
      QCRIL_LOG_INFO("Did not get uim_get_service_object");
      return;
    }

    qmi_client_error_type rc = qmi_client_notifier_init(
        mServiceObject, &mNotifierOsParams, &mQmiNotifyHandle);

    if (rc == QMI_NO_ERR)
    {
      rc = qmi_client_register_notify_cb(mQmiNotifyHandle, UimModemEndPointModule::onQmiNotifyCb, this);

      QCRIL_LOG_INFO("qmi_client_register_notify_cb: %d", rc);
    }
    else
    {
      QCRIL_LOG_INFO("qmi_client_notifier_init failed: %d", rc);
    }
  }
  msg->sendResponse(msg, Message::Callback::Status::SUCCESS, nullptr);
} /* UimModemEndPointModule::handleQmiClientSetup */


/*===========================================================================

  FUNCTION  handleQmiServiceUp -    UimModemEndPointModule QMI service up
                                    indication handler

===========================================================================*/
void UimModemEndPointModule::handleQmiServiceUp
(
  std::shared_ptr<Message> msg
)
{
  qmi_client_error_type client_err     = QMI_NO_ERR;
  qmi_client_type       qmi_svc_client = nullptr;
  qmi_service_info      info           = {};

  (void)msg;
  QCRIL_LOG_INFO("handleQmiServiceUp()");

  if (mQmiSvcClient != nullptr)
  {
    QCRIL_LOG_INFO( "Already Registered to service");
    return;
  }

  memset(&mOsParams, 0x00, sizeof(mOsParams));

  client_err = qmi_client_get_any_service(mServiceObject, &info);
  QCRIL_LOG_INFO("qmi_client_get_any_service %d", client_err);

  client_err = qmi_client_init(&info,
                               mServiceObject,
                               (qmi_client_ind_cb)qmi_client_indication_cb,
                               this,
                               &mOsParams,
                               &qmi_svc_client);

  QCRIL_LOG_INFO("qmi_client_init %d", client_err);

  if (client_err != QMI_NO_ERR)
  {
    QCRIL_LOG_INFO("Error in client init");
  }
  else
  {
    QCRIL_LOG_INFO( "Qmi service client : %p", qmi_svc_client);

    mQmiSvcClient = qmi_svc_client;
    mOwner->setState(ModemEndPoint::State::OPERATIONAL);

    auto ind_msg_ptr = std::make_shared<UimQmiUimIndicationMsg>(QMI_UIM_SRVC_UP_IND_MSG,
                                                                nullptr);
    if (ind_msg_ptr != nullptr)
    {
      ind_msg_ptr->broadcast();
    }
  }
} /* UimModemEndPointModule::handleQmiServiceUp */


/*===========================================================================

  FUNCTION  handleQmiServiceDown -  UimModemEndPointModule QMI service down
                                    indication handler

===========================================================================*/
void UimModemEndPointModule::handleQmiServiceDown
(
  std::shared_ptr<Message> msg
)
{
  (void)msg;

  if (mQmiSvcClient != NULL)
  {
    (void)qmi_client_release(mQmiSvcClient);
    mQmiSvcClient = nullptr;
  }

  QCRIL_LOG_INFO( "handleQmiServiceDown : %d", mOwner->getState());

  mOwner->setState(ModemEndPoint::State::NON_OPERATIONAL);

  auto ind_msg_ptr = std::make_shared<UimQmiUimIndicationMsg>(QMI_UIM_SRVC_DOWN_IND_MSG,
                                                              nullptr);
  if (ind_msg_ptr != nullptr)
  {
    ind_msg_ptr->broadcast();
  }
} /* UimModemEndPointModule::handleQmiServiceDown */


/*===========================================================================
  FUNCTION  qmi_uim_process_qcril_uim_request - Process QCRIL UIM requests
===========================================================================*/
int UimModemEndPointModule::processQcrilUimRequest
(
  qmi_client_type                mQmiSvcClient,
  uint32_t                       request_type,
  const void                   * param_data_ptr,
  qmi_uim_user_async_cb_type     callback_function_ptr,
  void                         * user_data,
  qmi_uim_rsp_data_type        * rsp_data_ptr
)
{
  int ret = -1;

 QCRIL_LOG_INFO( "%s, request: %d", __FUNCTION__, request_type);

  switch(request_type)
  {
    case QCRIL_UIM_REQUEST_READ_TRANSPARENT:
      ret = qcril_qmi_uim_read_transparent(mQmiSvcClient,
                                          (const qmi_uim_read_transparent_params_type*)param_data_ptr,
                                          callback_function_ptr,
                                          user_data,
                                          rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_READ_RECORD:
      ret = qcril_qmi_uim_read_record(mQmiSvcClient,
                                     (const qmi_uim_read_record_params_type*)param_data_ptr,
                                     callback_function_ptr,
                                     user_data,
                                     rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_WRITE_TRANSPARENT:
      ret = qcril_qmi_uim_write_transparent(mQmiSvcClient,
                                           (const qmi_uim_write_transparent_params_type*)param_data_ptr,
                                           callback_function_ptr,
                                           user_data,
                                           rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_WRITE_RECORD:
      ret = qcril_qmi_uim_write_record(mQmiSvcClient,
                                      (const qmi_uim_write_record_params_type*)param_data_ptr,
                                      callback_function_ptr,
                                      user_data,
                                      rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_GET_RESPONSE:
      ret = qcril_qmi_uim_get_file_attributes(mQmiSvcClient,
                                             (const qmi_uim_get_file_attributes_params_type*)param_data_ptr,
                                             callback_function_ptr,
                                             user_data,
                                             rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_REFRESH_REGISTER:
      ret = qcril_qmi_uim_refresh_register(mQmiSvcClient,
                                          (const qmi_uim_refresh_register_params_type*)param_data_ptr,
                                          callback_function_ptr,
                                          user_data,
                                          rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_GET_FDN:
      ret = qcril_qmi_uim_get_service_status(mQmiSvcClient,
                                            (const qmi_uim_get_service_status_params_type*)param_data_ptr,
                                            callback_function_ptr,
                                            user_data,
                                            rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_SET_FDN:
      ret = qcril_qmi_uim_set_service_status(mQmiSvcClient,
                                            (const qmi_uim_set_service_status_params_type*)param_data_ptr,
                                            callback_function_ptr,
                                            user_data,
                                            rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_VERIFY_PIN2:
    case QCRIL_UIM_REQUEST_VERIFY_PIN:
      ret = qcril_qmi_uim_verify_pin(mQmiSvcClient,
                                    (const qmi_uim_verify_pin_params_type*)param_data_ptr,
                                    callback_function_ptr,
                                    user_data,
                                    rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_UNBLOCK_PIN:
      ret = qcril_qmi_uim_unblock_pin(mQmiSvcClient,
                                     (const qmi_uim_unblock_pin_params_type*)param_data_ptr,
                                     callback_function_ptr,
                                     user_data,
                                     rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_CHANGE_PIN:
      ret = qcril_qmi_uim_change_pin(mQmiSvcClient,
                                    (const qmi_uim_change_pin_params_type*)param_data_ptr,
                                    callback_function_ptr,
                                    user_data,
                                    rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_SET_PIN:
      ret = qcril_qmi_uim_set_pin_protection(mQmiSvcClient,
                                            (const qmi_uim_set_pin_protection_params_type*)param_data_ptr,
                                            callback_function_ptr,
                                            user_data,
                                            rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_DEPERSO:
      ret = qcril_qmi_uim_depersonalization(mQmiSvcClient,
                                           (const qmi_uim_depersonalization_params_type*)param_data_ptr,
                                           callback_function_ptr,
                                           user_data,
                                           rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_POWER_UP:
      ret = qcril_qmi_uim_power_up(mQmiSvcClient,
                                  (const qmi_uim_power_up_params_type*)param_data_ptr,
                                  callback_function_ptr,
                                  user_data,
                                  rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_POWER_DOWN:
      ret = qcril_qmi_uim_power_down(mQmiSvcClient,
                                    (const qmi_uim_power_down_params_type*)param_data_ptr,
                                    callback_function_ptr,
                                    user_data,
                                    rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_CHANGE_PROV_SESSION:
      ret = qcril_qmi_uim_change_provisioning_session(mQmiSvcClient,
                                                     (const qmi_uim_change_prov_session_params_type*)param_data_ptr,
                                                     callback_function_ptr,
                                                     user_data,
                                                     rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_AUTHENTICATE:
      ret = qcril_qmi_uim_authenticate(mQmiSvcClient,
                                       (const qmi_uim_authenticate_params_type*)param_data_ptr,
                                       callback_function_ptr,
                                       user_data,
                                       rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_SAP_CONNECTION:
      ret = qcril_qmi_uim_sap_connection(mQmiSvcClient,
                                         (const qmi_uim_sap_connection_params_type*)param_data_ptr,
                                         callback_function_ptr,
                                         user_data,
                                         rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_SAP_REQUEST:
      ret = qcril_qmi_uim_sap_request(mQmiSvcClient,
                                      (const qmi_uim_sap_request_params_type*)param_data_ptr,
                                      callback_function_ptr,
                                      user_data,
                                      rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_LOGICAL_CHANNEL:
      ret = qcril_qmi_uim_logical_channel(mQmiSvcClient,
                                         (const qmi_uim_logical_channel_params_type*)param_data_ptr,
                                         callback_function_ptr,
                                         user_data,
                                         rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_OPEN_LOGICAL_CHANNEL:
      ret = qcril_qmi_uim_open_logical_channel(mQmiSvcClient,
                                               (const qmi_uim_open_logical_channel_params_type*)param_data_ptr,
                                               callback_function_ptr,
                                               user_data,
                                               rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_SEND_APDU:
      ret = qcril_qmi_uim_send_apdu(mQmiSvcClient,
                                   (const qmi_uim_send_apdu_params_type*)param_data_ptr,
                                   callback_function_ptr,
                                   user_data,
                                   rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_GET_ATR:
      ret = qcril_qmi_uim_get_atr(mQmiSvcClient,
                                  (const qmi_uim_get_atr_params_type*)param_data_ptr,
                                  callback_function_ptr,
                                  user_data,
                                  rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_SEND_STATUS:
      ret = qcril_qmi_uim_send_status(mQmiSvcClient,
                                      (const qmi_uim_status_cmd_params_type*)param_data_ptr,
                                      callback_function_ptr,
                                      user_data,
                                      rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_RESELECT:
      ret = qcril_qmi_uim_reselect(mQmiSvcClient,
                                   (const qmi_uim_reselect_params_type*)param_data_ptr,
                                   callback_function_ptr,
                                   user_data,
                                   rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_SUPPLY_VOLTAGE:
      ret = qcril_qmi_uim_supply_voltage(mQmiSvcClient,
                                         (const qmi_uim_supply_voltage_params_type*)param_data_ptr,
                                         callback_function_ptr,
                                         user_data,
                                         rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_GBA:
      ret = qcril_qmi_uim_gba_request(mQmiSvcClient,
                                      (const qmi_uim_gba_params_type*)param_data_ptr,
                                      callback_function_ptr,
                                      user_data,
                                      rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_GBA_IMPI:
      ret = qcril_qmi_uim_gba_impi(mQmiSvcClient,
                                   (const qmi_uim_gba_impi_params_type*)param_data_ptr,
                                   callback_function_ptr,
                                   user_data,
                                   rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_PERSO:
      ret = qcril_qmi_uim_personalization(mQmiSvcClient,
                                         (const qmi_uim_personalization_params_type*)param_data_ptr,
                                           callback_function_ptr,
                                           user_data,
                                           rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_GET_PHYSICAL_SLOT_MAPPING:
      ret = qcril_qmi_uim_get_physical_slot_mapping(mQmiSvcClient,
                                                   callback_function_ptr,
                                                   user_data,
                                                   rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_SWITCH_SLOT_EXT:
      ret = qcril_qmi_uim_switch_slot_ext(mQmiSvcClient,
                                          (const qmi_uim_physical_slot_mapping_type*)param_data_ptr,
                                          callback_function_ptr,
                                          user_data,
                                          rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_SET_APDU_BEHAVIOR:
      ret = qcril_qmi_uim_set_apdu_behavior(mQmiSvcClient,
                                            (const qmi_uim_set_apdu_behavior_params_type*)param_data_ptr,
                                            callback_function_ptr,
                                            user_data,
                                            rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_EVENT_REG:
      ret = qcril_qmi_uim_event_reg(mQmiSvcClient,
                                    (qmi_uim_event_reg_params_type *)param_data_ptr,
                                    rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_GET_LABEL:
      ret = qcril_qmi_uim_get_label(mQmiSvcClient,
                                    (qmi_uim_get_label_params_type *)param_data_ptr,
                                    callback_function_ptr,
                                    user_data,
                                    rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_GET_CONFIGURATION:
      ret = qcril_qmi_uim_get_configuration(mQmiSvcClient,
                                            (qmi_uim_get_configuration_params_type *)param_data_ptr,
                                            callback_function_ptr,
                                            user_data,
                                            rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_REFRESH_OK:
      ret = qcril_qmi_uim_refresh_ok(mQmiSvcClient,
                                     (qmi_uim_refresh_ok_params_type *)param_data_ptr,
                                     rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_REFRESH_COMPLETE:
      ret = qcril_qmi_uim_refresh_complete(mQmiSvcClient,
                                           (qmi_uim_refresh_complete_params_type *)param_data_ptr,
                                           rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_GET_CARD_STATUS:
      ret = qcril_qmi_uim_get_card_status(mQmiSvcClient,
                                          callback_function_ptr,
                                          user_data,
                                          rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_CLOSE_SESSION:
      ret = qcril_qmi_uim_close_session(mQmiSvcClient,
                                        (qmi_uim_close_session_params_type *)param_data_ptr,
                                        rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_GET_TERMINAL_CAPABILITY:
      ret = qcril_qmi_uim_get_terminal_capability(mQmiSvcClient,
                                                  (qmi_uim_terminal_capability_params_type*)param_data_ptr,
                                                  callback_function_ptr,
                                                  user_data,
                                                  rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_SET_TERMINAL_CAPABILITY:
      ret = qcril_qmi_uim_set_terminal_capability(mQmiSvcClient,
                                                  (qmi_uim_terminal_capability_params_type*)param_data_ptr,
                                                  callback_function_ptr,
                                                  user_data,
                                                  rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_GET_SLOTS_STATUS:
      ret = qcril_qmi_uim_get_slots_status(mQmiSvcClient,
                                           callback_function_ptr,
                                           user_data,
                                           rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_REMOTE_UNLOCK:
      ret = qcril_qmi_uim_remote_unlock(mQmiSvcClient,
                                        (const qmi_uim_remote_unlock_params_type *)param_data_ptr,
                                        callback_function_ptr,
                                        user_data,
                                        rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_ADD_PROFILE:
      ret = qcril_qmi_uim_add_profile(mQmiSvcClient,
                                      (const qmi_uim_add_profile_req_type*)param_data_ptr,
                                      callback_function_ptr,
                                      user_data,
                                      rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_SET_SIM_PROFILE:
      ret = qcril_qmi_uim_set_sim_profile(mQmiSvcClient,
                                          (const qmi_uim_set_sim_profile_req_type*)param_data_ptr,
                                          callback_function_ptr,
                                          user_data,
                                          rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_GET_SIM_PROFILES:
      ret = qcril_qmi_uim_get_sim_profiles(mQmiSvcClient,
                                           (const qmi_uim_get_sim_profiles_type*)param_data_ptr,
                                           callback_function_ptr,
                                           user_data,
                                           rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_DELETE_PROFILE:
      ret = qcril_qmi_uim_delete_profile(mQmiSvcClient,
                                         (const qmi_uim_delete_profile_req_type*)param_data_ptr,
                                         callback_function_ptr,
                                         user_data,
                                         rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_UPDATE_PROFILE_NICKNAME:
      ret = qcril_qmi_uim_update_profile_nickname(mQmiSvcClient,
                                                  (const qmi_uim_update_profile_nickname_req_type*)param_data_ptr,
                                                  callback_function_ptr,
                                                  user_data,
                                                  rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_EUICC_MEMORY_RESET:
      ret = qcril_qmi_uim_euicc_memory_reset(mQmiSvcClient,
                                             (const qmi_uim_euicc_memory_reset_req_type*)param_data_ptr,
                                             callback_function_ptr,
                                             user_data,
                                             rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_USER_CONSENT:
      ret = qcril_qmi_uim_user_consent(mQmiSvcClient,
                                       (const qmi_uim_profile_user_consent_req_type*)param_data_ptr,
                                       callback_function_ptr,
                                       user_data,
                                       rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_GET_EID:
      ret = qcril_qmi_uim_get_eid(mQmiSvcClient,
                                  (const qmi_uim_get_eid_req_type*)param_data_ptr,
                                  callback_function_ptr,
                                  user_data,
                                  rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_GET_SET_SERVER_ADDRESS:
      ret = qcril_qmi_uim_get_set_server_address(mQmiSvcClient,
                                                 (const qmi_uim_euicc_default_server_address_req_type*)param_data_ptr,
                                                 callback_function_ptr,
                                                 user_data,
                                                 rsp_data_ptr);
      break;

    case QCRIL_UIM_REQUEST_GET_PROFILE_INFO:
      ret = qcril_qmi_uim_get_profile_info(mQmiSvcClient,
                                           (const qmi_uim_get_profile_info_req_type*)param_data_ptr,
                                           callback_function_ptr,
                                           user_data,
                                           rsp_data_ptr);
      break;

    default:
      ret = -1;
      break;
  }
  return ret;
} /* qmi_uim_process_qcril_uim_request */


/*===========================================================================
  FUNCTION  onQmiUnsolIndCb - UimModemEndPointModule QMI UIM unsol
                                    message handler
===========================================================================*/
void UimModemEndPointModule::onQmiUnsolIndCb
(
  void * user_data,
  qmi_uim_indication_id_type     ind_id,
  qmi_uim_indication_data_type * ind_data_ptr
)
{
  std::shared_ptr<UimQmiUimIndicationMsg> ind_msg_ptr =
       std::make_shared<UimQmiUimIndicationMsg>((uint32_t)ind_id,
                                                (void *)ind_data_ptr);
  (void) user_data;

  if (ind_msg_ptr != nullptr)
  {
    ind_msg_ptr->broadcast();
  }
} /* UimModemEndPointModule::onQmiUnsolIndCb */


/*===========================================================================
  FUNCTION  handleQcrilUimRequest - UimModemEndPointModule QCRIL sync
                                        request handler
===========================================================================*/
void UimModemEndPointModule::handleQcrilUimRequest
(
  std::shared_ptr<UimQmiUimRequestMsg> uim_qmi_req_msg_ptr
)
{
  std::shared_ptr<int>              respPtr   = std::make_shared<int>(-1);
  qmi_uim_user_async_cb_type        cb_ptr    = NULL;
  Message::Callback::Status         status    = Message::Callback::Status::FAILURE;

  QCRIL_LOG_INFO("%s, State: %d", __FUNCTION__, mOwner->getState());

  if (uim_qmi_req_msg_ptr == nullptr)
  {
    return;
  }

  do
  {
    if (mOwner->getState() != ModemEndPoint::State::OPERATIONAL ||
        respPtr == NULL ||
        (uim_qmi_req_msg_ptr->get_rsp_data() == NULL &&
         uim_qmi_req_msg_ptr->get_user_data() == NULL))
    {
      QCRIL_LOG_ERROR(" Failed to handle request resp: %p, req: 0x%x", respPtr.get(), uim_qmi_req_msg_ptr.get());
      break;
    }

    if (uim_qmi_req_msg_ptr->get_rsp_data() == NULL &&
        uim_qmi_req_msg_ptr->get_user_data() != NULL)
    {
      cb_ptr = UimModemEndPointModule::UimQmiAsyncCb;
    }

    status = Message::Callback::Status::SUCCESS;
    *respPtr = processQcrilUimRequest(mQmiSvcClient,
                                      uim_qmi_req_msg_ptr->get_msg_id(),
                                      uim_qmi_req_msg_ptr->get_message(),
                                      cb_ptr,
                                      (void *)uim_qmi_req_msg_ptr->get_user_data(),
                                      (qmi_uim_rsp_data_type *)uim_qmi_req_msg_ptr->get_rsp_data());
  } while(0);

  uim_qmi_req_msg_ptr->sendResponse(uim_qmi_req_msg_ptr,
                                    status,
                                    respPtr);
} /* UimModemEndPointModule::handleQcrilUimRequest */


/*===========================================================================
  FUNCTION  UimModemEndPointModule::UimQmiAsyncCb - QMI callback function
===========================================================================*/
void UimModemEndPointModule::UimQmiAsyncCb
(
  qmi_uim_rsp_data_type        * rsp_data,
  void                         * user_data
)
{
  std::shared_ptr<UimQmiUimResponseMsg>  resp_msg_ptr =
   std::make_shared<UimQmiUimResponseMsg>(rsp_data, (void *)user_data);

  if (resp_msg_ptr != nullptr)
  {
    resp_msg_ptr->dispatch();
  }
} /* UimQmiAsyncCb */


/*===========================================================================

  FUNCTION  cleanUpQmiSvcClient -  CleanUp QMI UIM Service

===========================================================================*/
void UimModemEndPointModule::cleanUpQmiSvcClient()
{
  if (mQmiNotifyHandle != nullptr)
  {
    (void)qmi_client_release(mQmiNotifyHandle);
    mQmiNotifyHandle = nullptr;
  }

  if(nullptr != mQmiSvcClient)
  {
    qmi_client_release(mQmiSvcClient);
    mQmiSvcClient = nullptr;
    mServiceObject = nullptr;
  }
} /* UimModemEndPointModule::cleanUpQmiSvcClient */
