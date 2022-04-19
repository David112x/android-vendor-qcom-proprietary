/******************************************************************************
#  Copyright (c) 2017 - 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#  ---------------------------------------------------------------------------
#******************************************************************************/

#include "UimModule.h"
#include "framework/ModuleLooper.h"
#include "framework/QcrilInitMessage.h"
#include "modules/qmi/ModemEndPointFactory.h"
#include "modules/uim_service/UimGetRilPropertySyncMsg.h"
#include "modules/uim/UimInitMessage.h"

#include "UimTimerRequestMsg.h"
#include "qcril_uim.h"
#include "qcril_uim_card.h"
#include "qcril_uim_file.h"
#include "qcril_uim_security.h"
#include "qcril_uim_sap.h"
#include "qcril_uim_gba.h"
#include "qcril_uim_slot_mapping.h"
#include "qcril_uim_util.h"
#include "qcril_uim_restart.h"

#define TAG "UimModule"
#define MAX_QMI_UIM_ASYNC_MSG 10
#define QCRIL_UIM_LOG_ADB_ON     "persist.vendor.radio.adb_log_on"

bool qcril_uim_log_adb_on = false;

static load_module<UimModule> uim_module;

/*=========================================================================

  Function:  UimModule::UimModule

===========================================================================*/
UimModule::UimModule():uim_req_list("UimModule")
{
  mName = "UimModule";
  mReady = false;
  mRestart = false;

  mLooper = std::unique_ptr<ModuleLooper>(new ModuleLooper);

  QCRIL_LOG_INFO( "%s - Constructor\n", __FUNCTION__);

  using std::placeholders::_1;
  mMessageHandler = {
       HANDLER(QcrilInitMessage, UimModule::UimProcessRilInit),
       HANDLER(UimTimerRequestMsg, UimModule::UimHandleTimerRequest),
       HANDLER(UimTimerExpiryMsg, UimModule::UimHandleTimerExpiry),
       HANDLER(UimGetFacilityLockRequestMsg, UimModule::UimProcessGetFacilityLockStatusReq),
       HANDLER(UimSetFacilityLockRequestMsg, UimModule::UimProcessSetFacilityLockReq),
       HANDLER(UimGetAtrRequestMsg, UimModule::UimProcessGetAtrReq),
       HANDLER(UimGetGIDRequestMsg, UimModule::UimProcessGetGIDReq),
       HANDLER(UimGetMccMncRequestMsg, UimModule::handleUimGetMccMncRequestMsg),
       HANDLER(UimQmiUimResponseMsg, UimModule::UimProcessQmiCommandCallback),
       HANDLER(UimQmiUimIndicationMsg, UimModule::UimProcessQmiIndiaction),
       HANDLER(UimChangeSubscriptionReqMsg, UimModule::UimProcessChangeSubscription),
       HANDLER(UimGetImsiRequestSyncMsg, UimModule::UimProcessGetImsiReq),
       HANDLER(UimGetCardStatusRequestSyncMsg, UimModule::UimProcessGetCardStatusReq),
       HANDLER(UimGetIccIdRequestSyncMsg, UimModule::UimProcessGetIccIdSycnReq),
       HANDLER(UimGetCardStatusRequestMsg, UimModule::handleGetCardStatusRequestMsg),
       HANDLER(UimEnterSimPinRequestMsg, UimModule::handleEnterSimPinRequestMsg),
       HANDLER(UimEnterSimPukRequestMsg, UimModule::handleEnterSimPukRequestMsg),
       HANDLER(UimChangeSimPinRequestMsg, UimModule::handleChangeSimPinRequestMsg),
       HANDLER(UimEnterDePersoRequestMsg, UimModule::handleEnterDePersoRequestMsg),
       HANDLER(UimISIMAuthenticationRequestMsg, UimModule::handleISIMAuthenticationRequestMsg),
       HANDLER(UimSIMAuthenticationRequestMsg, UimModule::handleSIMAuthenticationRequestMsg),
       HANDLER(UimGetImsiRequestMsg, UimModule::handleGetImsiRequestMsg),
       HANDLER(UimSIMIORequestMsg, UimModule::handleSIMIORequestMsg),
       HANDLER(UimTransmitAPDURequestMsg, UimModule::handleTransmitAPDURequestMsg),
       HANDLER(UimSIMOpenChannelRequestMsg, UimModule::handleSIMOpenChannelRequestMsg),
       HANDLER(UimSIMCloseChannelRequestMsg, UimModule::handleSIMCloseChannelRequestMsg),
       HANDLER(UimCardPowerReqMsg, UimModule::handleUimCardPowerReqMsg),
       HANDLER(UimVoltageStatusRequestMsg, UimModule::handleUimVoltageStatusRequestMsg),
       HANDLER(UimPersoRequestMsg, UimModule::handleUimPersoRequestMsg),
       HANDLER(UimGetPersoStatusRequestMsg, UimModule::handleUimGetPersoStatusRequestMsg),
       HANDLER(UimGetLPATermialCapabilitesRequestMsg, UimModule::handleUimGetLPATermialCapabilitesRequestMsg),
       HANDLER(UimSetLPATermialCapabilitesRequestMsg, UimModule::handleUimSetLPATermialCapabilitesRequestMsg),
       HANDLER(UimSetAPDUBehaviorRequestMsg, UimModule::handleUimSetAPDUBehaviorRequestMsg),
       HANDLER(UimGBAGetIMPIRequestMsg, UimModule::handleUimGBAGetIMPIRequestMsg),
       HANDLER(UimGBAInitRequestMsg, UimModule::handleUimGBAInitRequestMsg),
       HANDLER(UimLpaReqMessage, UimModule::uim_process_lpa_request),
       HANDLER(UimSAPConnectionRequestMsg, UimModule::handleUimSAPConnectionRequestMsg),
       HANDLER(UimSAPRequestMsg, UimModule::handleUimSAPRequestMsg),
       HANDLER(UimRemoteSimlockRequestMsg, UimModule::handleUimRemoteSimlockRequestMsg),
       HANDLER(UimGetPhysicalSlotMappingRequestMsg, UimModule::handleUimGetPhysicalSlotMappingRequestMsg),
       HANDLER(UimSwitchSlotRequestMsg, UimModule::handleUimSwitchSlotRequestMsg),
       HANDLER(UimGetSlotStatusRequestMsg, UimModule::handleUimGetSlotStatusRequestMsg),
                    };
} /* UimModule::UimModule */


/*=========================================================================

  Function:  UimModule::~UimModule

===========================================================================*/
UimModule::~UimModule()
{
  QCRIL_LOG_INFO( "%s - Destructor \n", __FUNCTION__);
  mReady = false;
  mRestart = false;
  mUimModemEndPoint = NULL;
  mLooper = nullptr;
} /* UimModule::~UimModule */


/*=========================================================================

  Function:  UimModule::init

===========================================================================*/
void UimModule::init()
{
  QCRIL_LOG_INFO( "init\n");

  Module::init();

  /* Reset qcril_uim global structure*/
  qcril_uim_reset_state();

  mUimModemEndPoint = ModemEndPointFactory<UimModemEndPoint>::getInstance().buildEndPoint();
} /* UimModule::init() */


/*=========================================================================

  Function:  UimModule::UimProcessRilInit

===========================================================================*/
void UimModule::UimProcessRilInit(std::shared_ptr<Message> msg_ptr)
{
  /* Init QMI services.*/
  std::shared_ptr<std::string>  sharedResponse;
  auto      prop_ptr     = std::make_shared<UimGetRilPropertySyncMsg>
                            (QCRIL_UIM_LOG_ADB_ON);
  std::shared_ptr<std::string>  respPtr = NULL;

  (void) msg_ptr;

  if (prop_ptr != NULL &&
      (prop_ptr->dispatchSync(respPtr) == Message::Callback::Status::SUCCESS)&&
      respPtr != NULL &&
      strcmp(respPtr->c_str(), "1") == 0)
  {
    qcril_uim_log_adb_on = true;
  }
  else
  {
    qcril_uim_log_adb_on = false;
  }

  if (mUimModemEndPoint != NULL)
  {
    mUimModemEndPoint->requestSetupSync(sharedResponse);
  }
} /* UimModule::UimProcessRilInit */


/*=========================================================================

  Function:  UimModule::UimHandleTimerRequest

===========================================================================*/
void UimModule::UimHandleTimerRequest
(
  std::shared_ptr<Message> msg_ptr
)
{
  /* Dummy function, and don't send response, Dispatcher will call the callback
     on timer expiry */
  (void) msg_ptr;
} /* UimModule::UimHandleTimerRequest */


/*=========================================================================

  Function:  UimModule::UimHandleTimerExpiry

===========================================================================*/
void UimModule::UimHandleTimerExpiry
(
  std::shared_ptr<UimTimerExpiryMsg> msg_ptr
)
{
  UimTimerExpiryCallback  cb = NULL;

  if (msg_ptr == NULL)
  {
    return;
  }

  cb = msg_ptr->get_cb_func();

  if (cb != NULL)
  {
    cb(msg_ptr->get_user_data());
  }
} /* UimModule::UimHandleTimerExpiry */


/*=========================================================================

  Function:  UimModule::UimProcessQmiCommandCallback

===========================================================================*/
void UimModule::UimProcessQmiCommandCallback(std::shared_ptr<UimQmiUimResponseMsg> resp_msg_ptr)
{
  if (mQmiAsyncMsgCount > 0)
  {
    mQmiAsyncMsgCount--;
  }

  /* Received one response, check the queue for pending messages */
  while (!mQmiPendingList.empty())
  {
    std::shared_ptr<int>                 respPtr    = nullptr;
    std::shared_ptr<UimQmiUimRequestMsg> pendingMsg = mQmiPendingList.front();

    mQmiPendingList.pop();

    if (pendingMsg != nullptr)
    {
      Message::Callback::Status status = pendingMsg->dispatchSync(respPtr);

      QCRIL_LOG_INFO("Processing the message in the QMI Queue : %s", pendingMsg->dump().c_str());

      if (status == Message::Callback::Status::SUCCESS &&
          respPtr != nullptr && *respPtr == 0)
      {
        /* Queuing async request to modem success */
        mQmiAsyncMsgCount++;
        break;
      }
      else
      {
        /* QMI UIM request failed, send error response back */
        qmi_uim_rsp_data_type                  rsp_data = {};
        std::shared_ptr<UimQmiUimResponseMsg>  resp_ptr = NULL;

        rsp_data.sys_err_code = (respPtr != nullptr) ? *respPtr : QMI_UIM_SERVICE_ERR;
        rsp_data.qmi_err_code = QMI_UIM_SERVICE_ERR_INTERNAL;
        rsp_data.rsp_id = qmi_uim_convert_req_type_to_rsp_id(pendingMsg->get_msg_id());

        resp_ptr = std::make_shared<UimQmiUimResponseMsg>(&rsp_data, (void *)pendingMsg->get_user_data());
        if (resp_ptr != nullptr)
        {
          resp_ptr->dispatch();
        }
      }
    }
  }

  if (resp_msg_ptr != NULL)
  {
    std::shared_ptr<Message>        req_ptr      = NULL;
    qcril_uim_user_data             user_data    = {};
    qcril_uim_callback_params_type *rsp_data_ptr = (qcril_uim_callback_params_type *)resp_msg_ptr->get_message();
    
    user_data.user_data = resp_msg_ptr->get_user_data();

    req_ptr = uim_req_list.extract(user_data.token);

    if (req_ptr == NULL)
    {
      return;
    }

    qcril_uim_process_qmi_callback(rsp_data_ptr, req_ptr);
  }
} /* UimProcessQmiCommandCallback */


/*=========================================================================

  Function:  UimModule::UimProcessQmiIndiaction

===========================================================================*/
void UimModule::UimProcessQmiIndiaction(std::shared_ptr<UimQmiUimIndicationMsg> ind_ptr)
{
  std::shared_ptr<UimInitMessage>         uim_init_msg_ptr = nullptr;

  if (ind_ptr != NULL)
  {
    switch(ind_ptr->get_ind_id())
    {
      case QMI_UIM_SRVC_UP_IND_MSG:
        if (mRestart == true)
        {
          qcril_uim_process_modem_restart_complete();
        }
        if (qcril_uim_init_state(&mReady) != 0)
        {
          QCRIL_LOG_INFO("Init failed");
          break;
        }

        /* Broadcast UIM ready message */
        uim_init_msg_ptr = std::make_shared<UimInitMessage>();
        if (uim_init_msg_ptr)
        {
          uim_init_msg_ptr->broadcast();
        }
        break;
      case QMI_UIM_SRVC_DOWN_IND_MSG:
        mRestart = true;
        mReady = false;
        qcril_uim_process_modem_restart_start();
        break;
      default:
        qcril_uim_process_qmi_indication(ind_ptr);
        break;
    }
  }
} /* UimProcessQmiIndiaction */


/*=========================================================================

  Function:  UimModule::UimProcessChangeSubscription

===========================================================================*/
void UimModule::UimProcessChangeSubscription(std::shared_ptr<UimChangeSubscriptionReqMsg> sub_info_ptr)
{
  if (sub_info_ptr == nullptr || mReady == false)
  {
    return;
  }

  qcril_uim_process_change_subscription(sub_info_ptr);
} /* UimProcessChangeSubscription */


/*=========================================================================

  Function:  UimModule::UimProcessGetIccIdSycnReq

===========================================================================*/
void UimModule::UimProcessGetIccIdSycnReq(std::shared_ptr<UimGetIccIdRequestSyncMsg> req_ptr)
{
  std::shared_ptr<string>       respPtr                        = nullptr;
  Message::Callback::Status     status                         = Message::Callback::Status::FAILURE;
  RIL_UIM_Errno                 ret                            = RIL_UIM_E_RADIO_NOT_AVAILABLE;
  uint8_t                       iccid_raw[QCRIL_UIM_ICCID_LEN] = {};
  uint8_t                       iccid_len                      = QCRIL_UIM_ICCID_LEN;
  char                          *iccid_str                     = NULL;

  if (req_ptr == nullptr)
  {
    return;
  }

  if (mReady)
  {
    ret = qcril_uim_read_iccid(req_ptr->get_slot(),
                               iccid_raw,
                               &iccid_len);

    iccid_str = qcril_uim_util_iccid_to_string(iccid_raw, iccid_len);

    if (iccid_str != NULL)
    {
      respPtr = std::make_shared<string>(iccid_str);
      if (respPtr != nullptr)
      {
        status = Message::Callback::Status::SUCCESS;
      }
      qcril_free(iccid_str);
    }
  }
  req_ptr->sendResponse(req_ptr, status, respPtr);
} /* UimProcessGetIccIdSycnReq */


/*=========================================================================

  Function:  UimModule::UimProcessGetImsiReq

===========================================================================*/
void UimModule::UimProcessGetImsiReq(std::shared_ptr<UimGetImsiRequestSyncMsg> imsi_req_ptr)
{
  RIL_UIM_Errno                             ret           = RIL_UIM_E_RADIO_NOT_AVAILABLE;
  std::shared_ptr<RIL_UIM_IMSI_Response>    imsi_rsp_ptr  = nullptr;

  if (imsi_req_ptr == nullptr)
  {
    return;
  }

  if (mReady)
  {
    char * imsi_ptr = NULL;

    ret = qcril_uim_direct_get_imsi(imsi_req_ptr->get_slot(),
                                    imsi_req_ptr->get_aid().c_str(),
                                    &imsi_ptr);

    if (imsi_ptr != nullptr)
    {
      imsi_rsp_ptr = std::make_shared<RIL_UIM_IMSI_Response>();
      if (imsi_rsp_ptr != nullptr)
      {
        imsi_rsp_ptr->err = ret;
        imsi_rsp_ptr->IMSI = imsi_ptr;
        qcril_free(imsi_ptr);
      }
    }
  }
  imsi_req_ptr->sendResponse(imsi_req_ptr, Message::Callback::Status::SUCCESS, imsi_rsp_ptr);
} /* UimProcessGetImsiReq */


/*=========================================================================

  Function:  UimModule::UimProcessGetCardStatusReq

===========================================================================*/
void UimModule::UimProcessGetCardStatusReq(std::shared_ptr<UimGetCardStatusRequestSyncMsg> req_ptr)
{
  Message::Callback::Status                status          = Message::Callback::Status::FAILURE;
  RIL_UIM_Errno                            ret             = RIL_UIM_E_RADIO_NOT_AVAILABLE;
  std::shared_ptr<RIL_UIM_CardStatus>      respPtr         = nullptr;

  if (req_ptr == nullptr)
  {
    return;
  }

  if (mReady)
  {
    respPtr = std::make_shared<RIL_UIM_CardStatus>();

    if (respPtr != nullptr)
    {
      ret = qcril_uim_direct_get_card_status(req_ptr->get_slot(),
                                             respPtr);

      if (ret == RIL_UIM_E_SUCCESS)
      {
        status = Message::Callback::Status::SUCCESS;
      }
    }
  }

  req_ptr->sendResponse(req_ptr, status, respPtr);
} /* UimProcessGetCardStatusReq */


/*=========================================================================

  Function:  UimModule::UimProcessGetFacilityLockStatusReq

===========================================================================*/
void UimModule::UimProcessGetFacilityLockStatusReq(std::shared_ptr<UimGetFacilityLockRequestMsg> req_ptr)
{
  if (req_ptr == nullptr)
  {
    return;
  }

  if (mReady == false)
  {
    req_ptr->sendResponse(req_ptr, Message::Callback::Status::FAILURE, nullptr);
    return;
  }

  if (req_ptr->get_facility_lock_type() == FDN)
  {
    qcril_uim_request_get_fdn_status(req_ptr);
  }
  else
  {
    qcril_uim_request_get_pin_status(req_ptr);
  }
} /* UimProcessGetFacilityLockStatusReq */


/*=========================================================================

  Function:  UimModule::UimProcessSetFacilityLockReq

===========================================================================*/
void UimModule::UimProcessSetFacilityLockReq(std::shared_ptr<UimSetFacilityLockRequestMsg> req_ptr)
{
  if (req_ptr == nullptr)
  {
    return;
  }

  if (mReady == false)
  {
    req_ptr->sendResponse(req_ptr, Message::Callback::Status::FAILURE, nullptr);
    return;
  }

  if (req_ptr->get_facility_lock_type() == FDN)
  {
    qcril_uim_request_set_fdn_status(req_ptr);
  }
  else
  {
    qcril_uim_request_set_pin_status(req_ptr);
  }
} /* UimProcessSetFacilityLockReq */


/*=========================================================================

  Function:  UimModule::UimProcessGetAtrReq

===========================================================================*/
void UimModule::UimProcessGetAtrReq(std::shared_ptr<UimGetAtrRequestMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  QCRIL_LOG_INFO( "mReady :%d", mReady);

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<UimAtrRspParam>();

    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = RIL_UIM_E_RADIO_NOT_AVAILABLE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_uim_request_get_atr(req_ptr);
} /* UimProcessGetAtrReq */


/*=========================================================================

  Function:  UimModule::UimProcessGetGIDReq

===========================================================================*/
void UimModule::UimProcessGetGIDReq(std::shared_ptr<UimGetGIDRequestMsg> req_ptr)
{
  if (req_ptr == nullptr)
  {
    return;
  }

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_UIM_GIDResponse>();

    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = RIL_UIM_E_RADIO_NOT_AVAILABLE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_uim_request_get_gid(req_ptr);
} /* UimModule::UimProcessGetGIDReq */


/*=========================================================================

  Function:  UimModule::handleUimGetMccMncRequestMsg

===========================================================================*/
void UimModule::handleUimGetMccMncRequestMsg(std::shared_ptr<UimGetMccMncRequestMsg> req_ptr)
{
  if (req_ptr == nullptr)
  {
    return;
  }

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<qcril_uim_mcc_mnc_info_type>();

    if (rsp_ptr != NULL)
    {
      rsp_ptr->err_code = RIL_UIM_E_RADIO_NOT_AVAILABLE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_uim_request_get_mcc_mnc(req_ptr);
} /* UimModule::handleUimGetMccMncRequestMsg */


/*=========================================================================

  Function:  UimModule::handleGetCardStatusRequestMsg

===========================================================================*/
void UimModule::handleGetCardStatusRequestMsg(std::shared_ptr<UimGetCardStatusRequestMsg> req_ptr)
{
  if (req_ptr == nullptr)
  {
    return;
  }

  qcril_uim_request_get_sim_status(req_ptr);
} /* UimModule::handleGetCardStatusRequestMsg */


/*=========================================================================

  Function:  UimModule::handleEnterSimPinRequestMsg

===========================================================================*/
void UimModule::handleEnterSimPinRequestMsg(std::shared_ptr<UimEnterSimPinRequestMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  QCRIL_LOG_INFO( "mReady :%d", mReady);

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_UIM_SIM_PIN_Response>();

    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = RIL_UIM_E_RADIO_NOT_AVAILABLE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_uim_request_enter_pin(req_ptr);
} /* handleEnterSimPinRequestMsg */


/*=========================================================================

  Function:  UimModule::handleEnterSimPukRequestMsg

===========================================================================*/
void UimModule::handleEnterSimPukRequestMsg(std::shared_ptr<UimEnterSimPukRequestMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  QCRIL_LOG_INFO( "mReady :%d", mReady);

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_UIM_SIM_PIN_Response>();

    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = RIL_UIM_E_RADIO_NOT_AVAILABLE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_uim_request_enter_puk(req_ptr);
} /* handleEnterSimPukRequestMsg */


/*=========================================================================

  Function:  UimModule::handleChangeSimPinRequestMsg

===========================================================================*/
void UimModule::handleChangeSimPinRequestMsg(std::shared_ptr<UimChangeSimPinRequestMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  QCRIL_LOG_INFO( "mReady :%d", mReady);

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_UIM_SIM_PIN_Response>();

    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = RIL_UIM_E_RADIO_NOT_AVAILABLE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_uim_request_change_pin(req_ptr);
} /* handleChangeSimPinRequestMsg */


/*=========================================================================

  Function:  UimModule::handleEnterDePersoRequestMsg

===========================================================================*/
void UimModule::handleEnterDePersoRequestMsg(std::shared_ptr<UimEnterDePersoRequestMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  QCRIL_LOG_INFO( "mReady :%d", mReady);

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_UIM_PersoResponse>();

    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = RIL_UIM_E_RADIO_NOT_AVAILABLE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_uim_request_enter_perso_key(req_ptr);
} /* handleEnterDePersoRequestMsg */


/*=========================================================================

  Function:  UimModule::handleISIMAuthenticationRequestMsg

===========================================================================*/
void UimModule::handleISIMAuthenticationRequestMsg(std::shared_ptr<UimISIMAuthenticationRequestMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  QCRIL_LOG_INFO( "mReady :%d", mReady);

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_UIM_SIM_IO_Response>();

    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = RIL_UIM_E_RADIO_NOT_AVAILABLE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_uim_request_isim_authenticate(req_ptr);
} /* handleISIMAuthenticationRequestMsg */


/*=========================================================================

  Function:  UimModule::handleSIMAuthenticationRequestMsg

===========================================================================*/
void UimModule::handleSIMAuthenticationRequestMsg(std::shared_ptr<UimSIMAuthenticationRequestMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  QCRIL_LOG_INFO( "mReady :%d", mReady);

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_UIM_SIM_IO_Response>();

    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = RIL_UIM_E_RADIO_NOT_AVAILABLE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_uim_request_sim_authenticate(req_ptr);
} /* handleSIMAuthenticationRequestMsg */


/*=========================================================================

  Function:  UimModule::handleGetImsiRequestMsg

===========================================================================*/
void UimModule::handleGetImsiRequestMsg(std::shared_ptr<UimGetImsiRequestMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  QCRIL_LOG_INFO( "mReady :%d", mReady);

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_UIM_IMSI_Response>();

    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = RIL_UIM_E_RADIO_NOT_AVAILABLE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_uim_request_get_imsi(req_ptr);
} /* handleGetImsiRequestMsg */


/*=========================================================================

  Function:  UimModule::handleSIMIORequestMsg

===========================================================================*/
void UimModule::handleSIMIORequestMsg(std::shared_ptr<UimSIMIORequestMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  QCRIL_LOG_INFO( "mReady :%d", mReady);

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_UIM_SIM_IO_Response>();

    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = RIL_UIM_E_RADIO_NOT_AVAILABLE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_uim_request_sim_io(req_ptr);
} /* handleSIMIORequestMsg */


/*=========================================================================

  Function:  UimModule::handleTransmitAPDURequestMsg

===========================================================================*/
void UimModule::handleTransmitAPDURequestMsg(std::shared_ptr<UimTransmitAPDURequestMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  QCRIL_LOG_INFO( "mReady :%d", mReady);

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_UIM_SIM_IO_Response>();

    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = RIL_UIM_E_RADIO_NOT_AVAILABLE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_uim_request_send_apdu(req_ptr);
} /* handleTransmitAPDURequestMsg */


/*=========================================================================

  Function:  UimModule::handleSIMOpenChannelRequestMsg

===========================================================================*/
void UimModule::handleSIMOpenChannelRequestMsg(std::shared_ptr<UimSIMOpenChannelRequestMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  QCRIL_LOG_INFO( "mReady :%d", mReady);

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_UIM_OpenChannelResponse>();

    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = RIL_UIM_E_RADIO_NOT_AVAILABLE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_uim_open_logical_ch_req(req_ptr);
} /* handleSIMOpenChannelRequestMsg */


/*=========================================================================

  Function:  UimModule::handleSIMCloseChannelRequestMsg

===========================================================================*/
void UimModule::handleSIMCloseChannelRequestMsg(std::shared_ptr<UimSIMCloseChannelRequestMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  QCRIL_LOG_INFO( "mReady :%d", mReady);

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_UIM_Errno>();

    if (rsp_ptr != NULL)
    {
      *rsp_ptr = RIL_UIM_E_RADIO_NOT_AVAILABLE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_uim_close_logical_ch_req(req_ptr);
} /* handleSIMCloseChannelRequestMsg */


/*=========================================================================

  Function:  UimModule::handleUimCardPowerReqMsg

===========================================================================*/
void UimModule::handleUimCardPowerReqMsg(std::shared_ptr<UimCardPowerReqMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  QCRIL_LOG_INFO( "mReady :%d", mReady);

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_UIM_Errno>();

    if (rsp_ptr != NULL)
    {
      *rsp_ptr = RIL_UIM_E_RADIO_NOT_AVAILABLE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_uim_request_uicc_power(req_ptr);
} /* handleUimCardPowerReqMsg */


/*=========================================================================

  Function:  UimModule::handleUimVoltageStatusRequestMsg

===========================================================================*/
void UimModule::handleUimVoltageStatusRequestMsg(std::shared_ptr<UimVoltageStatusRequestMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  QCRIL_LOG_INFO( "mReady :%d", mReady);

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_UIM_Errno>();

    if (rsp_ptr != NULL)
    {
      *rsp_ptr = RIL_UIM_E_RADIO_NOT_AVAILABLE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_uim_request_voltage_status(req_ptr);
} /* handleUimVoltageStatusRequestMsg */


/*=========================================================================

  Function:  UimModule::handleUimPersoRequestMsg

===========================================================================*/
void UimModule::handleUimPersoRequestMsg(std::shared_ptr<UimPersoRequestMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  QCRIL_LOG_INFO( "mReady :%d", mReady);

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_UIM_PersoResponse>();

    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = RIL_UIM_E_RADIO_NOT_AVAILABLE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_uim_request_perso_reactivate(req_ptr);
} /* handleUimPersoRequestMsg */


/*=========================================================================

  Function:  UimModule::handleUimGetPersoStatusRequestMsg

===========================================================================*/
void UimModule::handleUimGetPersoStatusRequestMsg(std::shared_ptr<UimGetPersoStatusRequestMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  QCRIL_LOG_INFO( "mReady :%d", mReady);

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_UIM_PersonalizationStatusResp>();

    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = RIL_UIM_E_RADIO_NOT_AVAILABLE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_uim_request_perso_status(req_ptr);
} /* handleUimGetPersoStatusRequestMsg */


/*=========================================================================

  Function:  UimModule::handleUimGetLPATermialCapabilitesRequestMsg

===========================================================================*/
void UimModule::handleUimGetLPATermialCapabilitesRequestMsg(std::shared_ptr<UimGetLPATermialCapabilitesRequestMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  QCRIL_LOG_INFO( "mReady :%d", mReady);

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_UIM_TerminalCapabilityLPATLV>();

    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = RIL_UIM_E_RADIO_NOT_AVAILABLE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_uim_request_get_terminal_capability(req_ptr);
} /* handleUimGetLPATermialCapabilitesRequestMsg */


/*=========================================================================

  Function:  UimModule::handleUimSetLPATermialCapabilitesRequestMsg

===========================================================================*/
void UimModule::handleUimSetLPATermialCapabilitesRequestMsg(std::shared_ptr<UimSetLPATermialCapabilitesRequestMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  QCRIL_LOG_INFO( "mReady :%d", mReady);

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_UIM_Errno>();

    if (rsp_ptr != NULL)
    {
      *rsp_ptr = RIL_UIM_E_RADIO_NOT_AVAILABLE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_uim_request_set_terminal_capability(req_ptr);
} /* handleUimSetLPATermialCapabilitesRequestMsg */


/*=========================================================================

  Function:  UimModule::handleUimSetAPDUBehaviorRequestMsg

===========================================================================*/
void UimModule::handleUimSetAPDUBehaviorRequestMsg(std::shared_ptr<UimSetAPDUBehaviorRequestMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  QCRIL_LOG_INFO( "mReady :%d", mReady);

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_UIM_Errno>();

    if (rsp_ptr != NULL)
    {
      *rsp_ptr = RIL_UIM_E_RADIO_NOT_AVAILABLE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_uim_request_set_apdu_behavior(req_ptr);
} /* handleUimSetAPDUBehaviorRequestMsg */


/*=========================================================================

  Function:  UimModule::handleUimGBAGetIMPIRequestMsg

===========================================================================*/
void UimModule::handleUimGBAGetIMPIRequestMsg(std::shared_ptr<UimGBAGetIMPIRequestMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  QCRIL_LOG_INFO( "mReady :%d", mReady);

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_UIM_GBA_IMPI_Response>();

    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = RIL_UIM_E_RADIO_NOT_AVAILABLE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_uim_gba_request_get_impi(req_ptr);
} /* handleUimGBAGetIMPIRequestMsg */


/*=========================================================================

  Function:  UimModule::uim_process_lpa_request

===========================================================================*/
void UimModule::uim_process_lpa_request(std::shared_ptr<UimLpaReqMessage> req_ptr)
{
  boolean  ret = FALSE;

  if (req_ptr == nullptr)
  {
    return;
  }

  switch(req_ptr->get_req_id())
  {
    case UIM_LPA_USER_REQUEST_ID:
      ret = qcril_uim_lpa_user_request(req_ptr);
      break;
    default:
      break;
  }

  if (ret == FALSE)
  {
    req_ptr->sendResponse(req_ptr, Message::Callback::Status::FAILURE, nullptr);
  }
} /* uim_process_lpa_request */


/*=========================================================================

  Function:  UimModule::handleUimGBAInitRequestMsg

===========================================================================*/
void UimModule::handleUimGBAInitRequestMsg(std::shared_ptr<UimGBAInitRequestMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  QCRIL_LOG_INFO( "mReady :%d", mReady);

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_UIM_GBA_InitResponse>();

    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = RIL_UIM_E_RADIO_NOT_AVAILABLE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_uim_request_gba_init(req_ptr);
} /* handleUimGBAInitRequestMsg */


/*=========================================================================

  Function:  UimModule::handleUimSAPConnectionRequestMsg

===========================================================================*/
void UimModule::handleUimSAPConnectionRequestMsg(std::shared_ptr<UimSAPConnectionRequestMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  QCRIL_LOG_INFO( "mReady :%d", mReady);

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_UIM_SAP_ConnectionResponse>();

    if (rsp_ptr != NULL)
    {
      *rsp_ptr = RIL_UIM_SAP_CONNECTION_FAILURE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_uim_sap_connection_request(req_ptr);
} /* handleUimSAPConnectionRequestMsg */


/*=========================================================================

  Function:  UimModule::handleUimSAPRequestMsg

===========================================================================*/
void UimModule::handleUimSAPRequestMsg(std::shared_ptr<UimSAPRequestMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  QCRIL_LOG_INFO( "mReady :%d", mReady);

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_UIM_SAP_Response>();

    if (rsp_ptr != NULL)
    {
      rsp_ptr->result = RIL_UIM_SAP_GENERIC_FAILURE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_uim_sap_send_request(req_ptr);
} /* handleUimSAPRequestMsg */


/*=========================================================================

  Function:  UimModule::handleUimRemoteSimlockRequestMsg

===========================================================================*/
void UimModule::handleUimRemoteSimlockRequestMsg(std::shared_ptr<UimRemoteSimlockRequestMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  QCRIL_LOG_INFO( "mReady :%d", mReady);

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_UIM_RemoteSimlockResponse>();

    if (rsp_ptr != NULL)
    {
      rsp_ptr->rsp_type = RIL_UIM_REMOTE_SIMLOCK_RESP_FAILURE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_uim_request_remote_sim_lock_unlock(req_ptr);
} /* handleUimRemoteSimlockRequestMsg */


/*=========================================================================

  Function:  UimModule::handleUimGetPhysicalSlotMappingRequestMsg

===========================================================================*/
void UimModule::handleUimGetPhysicalSlotMappingRequestMsg(std::shared_ptr<UimGetPhysicalSlotMappingRequestMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  QCRIL_LOG_INFO( "mReady :%d", mReady);

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_UIM_PhySlotMap>();

    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = RIL_UIM_E_RADIO_NOT_AVAILABLE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_uim_request_get_physical_slot_mapping(req_ptr);
} /* handleUimGetPhysicalSlotMappingRequestMsg */


/*=========================================================================

  Function:  UimModule::handleUimSwitchSlotRequestMsg

===========================================================================*/
void UimModule::handleUimSwitchSlotRequestMsg(std::shared_ptr<UimSwitchSlotRequestMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  QCRIL_LOG_INFO( "mReady :%d", mReady);

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_UIM_Errno>();

    if (rsp_ptr != NULL)
    {
      *rsp_ptr = RIL_UIM_E_RADIO_NOT_AVAILABLE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_uim_request_switch_slot(req_ptr);
} /* handleUimSwitchSlotRequestMsg */


/*=========================================================================

  Function:  UimModule::handleUimGetSlotStatusRequestMsg

===========================================================================*/
void UimModule::handleUimGetSlotStatusRequestMsg(std::shared_ptr<UimGetSlotStatusRequestMsg> req_ptr)
{
  QCRIL_LOG_INFO( "%s\n", __FUNCTION__);

  if (req_ptr == nullptr)
  {
    return;
  }

  QCRIL_LOG_INFO( "mReady :%d", mReady);

  if (mReady == false)
  {
    auto rsp_ptr = std::make_shared<RIL_UIM_SlotsStatusInfo>();

    if (rsp_ptr != NULL)
    {
      rsp_ptr->err = RIL_UIM_E_RADIO_NOT_AVAILABLE;
    }

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
    return;
  }

  qcril_uim_request_get_slots_status(req_ptr);
} /* handleUimGetSlotStatusRequestMsg */


/*=========================================================================

  Function:  getUimModule

===========================================================================*/
UimModule *getUimModule()
{
  return &(uim_module.get_module());
} /* getUimModule */


/*=========================================================================

  Function:  UimSendQmiUimRequest

===========================================================================*/
int UimModule::UimSendQmiUimRequest
(
  uint32                        msg_id,
  const void                  * params,
  std::shared_ptr<Message>      req_ptr
)
{
  std::shared_ptr<UimQmiUimRequestMsg> req_msg_ptr    = nullptr;
  int                                  ret            = 0;
  qcril_uim_user_data                  user_data      = {};
  std::pair<uint16_t, bool>            token          = {0, false};

  token = uim_req_list.insert(req_ptr);

  if (!token.second)
  {
    return -1;
  }

  user_data.token = token.first;

  req_msg_ptr = std::make_shared<UimQmiUimRequestMsg>(msg_id,
                                                      params,
                                                      nullptr,
                                                      user_data.user_data);

  if (req_msg_ptr == nullptr)
  {
    return -1;
  }

  /* QMI async message count less than MAX_QMI_UIM_ASYNC_MSG */
  if (mQmiAsyncMsgCount < MAX_QMI_UIM_ASYNC_MSG)
  {
    std::shared_ptr<int>                 respPtr        = nullptr;

    if (req_msg_ptr->dispatchSync(respPtr) == Message::Callback::Status::SUCCESS)
    {
      if (respPtr == nullptr)
      {
        ret = -1;
      }
      else
      {
        ret = *respPtr;
      }

      /* Request is qmi async increase QMI msg count */
      if (ret == 0)
      {
        mQmiAsyncMsgCount++;
      }
    }
    else
    {
      /*In case of dispatchSync failure from other Modules due to SSR etc,
      return -1 to sendResponse back*/

      ret = -1;
    }
  }
  else
  {
    /* Async message and waiting for 10 QMI async responses */
    QCRIL_LOG_INFO("Waiting for response from QMI UIM putting message in the QMI Queue");
    mQmiPendingList.push(req_msg_ptr);
  }

  return ret;
} /* UimSendQmiUimRequest */


/*=========================================================================

  Function:  UimModule::qcrilHalUimModuleCleanup

===========================================================================*/
void UimModule::qcrilHalUimModuleCleanup()
{
  if (mUimModemEndPoint != NULL)
  {
    mUimModemEndPoint->cleanUp();
  }
  mReady = false;
  mRestart = false;
} /* UimModule::qcrilHalUimModuleCleanup() */
