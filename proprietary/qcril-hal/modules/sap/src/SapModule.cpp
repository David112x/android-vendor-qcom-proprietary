/******************************************************************************
#  Copyright (c) 2018 - 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#ifndef QMI_RIL_UTF
#define LOG_TAG "RIL_SAP"

#include <android/hardware/radio/1.2/ISap.h>

#include <hwbinder/IPCThreadState.h>
#include <hwbinder/ProcessState.h>

#include <framework/ModuleLooper.h>

#include "modules/uim/UimSAPRequestMsg.h"
#include "modules/uim/UimSAPConnectionRequestMsg.h"
#include "modules/uim/UimSapStatusIndMsg.h"
#include "SapModule.h"

using namespace android::hardware::radio::V1_0;
using ::android::hardware::Return;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_array;
using ::android::hardware::Void;
using ::android::hardware::hidl_death_recipient;
using android::sp;

#define SAP_HIDL_SEND_RESPONSE(func, ...) \
  { \
    sp<ISapCallback>  callback   = (sapService != nullptr) ? sapService->getCallback() : nullptr; \
    Return<void>      retStatus  = Void(); \
    if (sapService != nullptr && callback != nullptr) \
    { \
      retStatus = callback->func(__VA_ARGS__); \
      sapService->checkReturnStatus(retStatus); \
    } \
  }

struct SapImpl;

static load_module<SapModule>    sap_module;
static android::sp<SapImpl>      sapService;


struct SapImpl : public android::hardware::radio::V1_2::ISap, public hidl_death_recipient
{
  sp<ISapCallback> sapCallback;

  qtimutex::QtiSharedMutex mCallbackLock;

  sp<ISapCallback> getCallback();

  void clearCallbacks();

  void serviceDied( uint64_t, const ::android::wp<::android::hidl::base::V1_0::IBase> &);

  Return<void> setCallback(const ::android::sp<ISapCallback>& sapCallbackParam);

  Return<void> connectReq(int32_t token, int32_t maxMsgSize);

  Return<void> disconnectReq(int32_t token);

  Return<void> apduReq(int32_t token, SapApduType type, const hidl_vec<uint8_t>& command);

  Return<void> transferAtrReq(int32_t token);

  Return<void> powerReq(int32_t token, bool state);

  Return<void> resetSimReq(int32_t token);

  Return<void> transferCardReaderStatusReq(int32_t token);

  Return<void> setTransferProtocolReq(int32_t token, SapTransferProtocol transferProtocol);

  void checkReturnStatus(Return<void>& ret);
};


/*===========================================================================

FUNCTION:  SapImpl::clearCallbacks

===========================================================================*/
void SapImpl::clearCallbacks()
{
  RLOGD("SecureElementImpl::clearCallbacks");

  std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  if (sapCallback != nullptr)
  {
    sapCallback->unlinkToDeath(this);
    sapCallback = nullptr;
  }
} /* SapImpl::clearCallbacks */


/*===========================================================================

FUNCTION:  SapImpl::serviceDied

===========================================================================*/
void SapImpl::serviceDied
(
  uint64_t,
  const ::android::wp<::android::hidl::base::V1_0::IBase> &
)
{
  RLOGD("SapImpl::serviceDied: Client died. Cleaning up callbacks");
  clearCallbacks();
} /* SapImpl::serviceDied */


/*===========================================================================

FUNCTION:  SapImpl::checkReturnStatus

===========================================================================*/
void SapImpl::checkReturnStatus
(
  Return<void>& ret
)
{
  if (ret.isOk() == false)
  {
    RLOGE("checkReturnStatus: unable to call response/indication callback: %s",
            ret.description().c_str());
    // Remote process (SapRilReceiver.java) hosting the callback must be dead. Reset the
    // callback object; there's no other recovery to be done here. When the client process is
    // back up, it will call setCallback()
    sapCallback = nullptr;
  }
} /* SapImpl::checkReturnStatus */


/*===========================================================================

FUNCTION:  SapImpl::setCallback

===========================================================================*/
Return<void> SapImpl::setCallback
(
  const ::android::sp<ISapCallback>& sapCallbackParam
)
{
  RLOGD("SapImpl::setCallback");

  {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
    if (sapCallback != nullptr)
    {
      sapCallback->unlinkToDeath(this);
    }
    sapCallback = sapCallbackParam;
    sapCallback->linkToDeath(this, 0);
  }

  return Void();
} /* SapImpl::setCallback */


/*===========================================================================

FUNCTION:  SapImpl::getCallback

===========================================================================*/
sp<ISapCallback> SapImpl::getCallback()
{
  std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return sapCallback;
} /* SapImpl::getCallback */


/*===========================================================================

FUNCTION:  SapImpl::connectReq

===========================================================================*/
Return<void> SapImpl::connectReq
(
  int32_t token,
  int32_t maxMsgSize
)
{
  auto msg = std::make_shared<UimSAPConnectionRequestMsg>(RIL_UIM_SAP_CONNECT);

  RLOGD("SapImpl::connectReq");

  (void) maxMsgSize;

  if (msg)
  {
    GenericCallback<RIL_UIM_SAP_ConnectionResponse> cb(
        ([token](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<RIL_UIM_SAP_ConnectionResponse> responseDataPtr) -> void
    {
      SapConnectRsp     resp       = SapConnectRsp::CONNECT_FAILURE;

      if (solicitedMsg && responseDataPtr &&
          status == Message::Callback::Status::SUCCESS &&
          *responseDataPtr == RIL_UIM_SAP_CONNECTION_SUCCESS)
      {
        resp = SapConnectRsp::SUCCESS;
      }

      SAP_HIDL_SEND_RESPONSE(connectResponse, token, resp, 0);
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    SAP_HIDL_SEND_RESPONSE(connectResponse, token, SapConnectRsp::CONNECT_FAILURE, 0);
  }

  return Void();
} /* SapImpl::connectReq */


/*===========================================================================

FUNCTION:  SapImpl::disconnectReq

===========================================================================*/
Return<void> SapImpl::disconnectReq
(
  int32_t token
)
{
  auto msg = std::make_shared<UimSAPConnectionRequestMsg>(RIL_UIM_SAP_DISCONNECT);

  RLOGD("SapImpl::disconnectReq");

  if (msg)
  {
    GenericCallback<RIL_UIM_SAP_ConnectionResponse> cb(
        ([token](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<RIL_UIM_SAP_ConnectionResponse> responseDataPtr) -> void
    {
      (void) status;
      (void) responseDataPtr;
      (void) solicitedMsg;

      SAP_HIDL_SEND_RESPONSE(disconnectResponse, token);
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    SAP_HIDL_SEND_RESPONSE(disconnectResponse, token);
  }

  return Void();
} /* SapImpl::disconnectReq */


/*===========================================================================

FUNCTION:  SapImpl::apduReq

===========================================================================*/
Return<void> SapImpl::apduReq
(
  int32_t                  token,
  SapApduType              type,
  const hidl_vec<uint8_t>& command)
{
  auto msg = std::make_shared<UimSAPRequestMsg>(RIL_UIM_SAP_APDU_REQ, command);

  RLOGD("SapImpl::apduReq");

  (void) type;

  if (msg)
  {
    GenericCallback<RIL_UIM_SAP_Response> cb(
        ([token](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<RIL_UIM_SAP_Response> responseDataPtr) -> void
    {
      SapResultCode         resp       = SapResultCode::GENERIC_FAILURE;
      std::vector<uint8_t>  apdu_data  = {};

      if (solicitedMsg && responseDataPtr &&
          status == Message::Callback::Status::SUCCESS)
      {
        apdu_data = responseDataPtr->atr_apdu_data;

        switch(responseDataPtr->result)
        {
          case RIL_UIM_SAP_SUCCESS:
            resp = SapResultCode::SUCCESS;
            break;
          case RIL_UIM_SAP_CARD_NOT_ACCESSSIBLE:
            resp = SapResultCode::CARD_NOT_ACCESSSIBLE;
            break;
          case RIL_UIM_SAP_CARD_ALREADY_POWERED_OFF:
            resp = SapResultCode::CARD_ALREADY_POWERED_OFF;
            break;
          case RIL_UIM_SAP_CARD_REMOVED:
            resp = SapResultCode::CARD_REMOVED;
            break;
          default:
            resp = SapResultCode::GENERIC_FAILURE;
        }
      }

      SAP_HIDL_SEND_RESPONSE(apduResponse, token, resp, apdu_data);
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    std::vector<uint8_t>  apdu_data  = {};
    SAP_HIDL_SEND_RESPONSE(apduResponse, token, SapResultCode::GENERIC_FAILURE, apdu_data);
  }

  return Void();
} /* SapImpl::apduReq */


/*===========================================================================

FUNCTION:  SapImpl::transferAtrReq

===========================================================================*/
Return<void> SapImpl::transferAtrReq
(
  int32_t token
)
{
  auto msg = std::make_shared<UimSAPRequestMsg>(RIL_UIM_SAP_TRANSFER_ATR_REQ);

  RLOGD("SapImpl::transferAtrReq");

  if (msg)
  {
    GenericCallback<RIL_UIM_SAP_Response> cb(
        ([token](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<RIL_UIM_SAP_Response> responseDataPtr) -> void
    {
      SapResultCode         resp      = SapResultCode::GENERIC_FAILURE;
      std::vector<uint8_t>  atr_data  = {};

      if (solicitedMsg && responseDataPtr &&
          status == Message::Callback::Status::SUCCESS)
      {
        atr_data = responseDataPtr->atr_apdu_data;

        switch(responseDataPtr->result)
        {
          case RIL_UIM_SAP_SUCCESS:
            resp = SapResultCode::SUCCESS;
            break;
          case RIL_UIM_SAP_CARD_NOT_ACCESSSIBLE:
            resp = SapResultCode::DATA_NOT_AVAILABLE;
            break;
          case RIL_UIM_SAP_CARD_ALREADY_POWERED_OFF:
            resp = SapResultCode::CARD_ALREADY_POWERED_OFF;
            break;
          case RIL_UIM_SAP_CARD_REMOVED:
            resp = SapResultCode::CARD_REMOVED;
            break;
          default:
            resp = SapResultCode::GENERIC_FAILURE;
        }
      }

      SAP_HIDL_SEND_RESPONSE(transferAtrResponse, token, resp, atr_data);
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    std::vector<uint8_t>  atr_data  = {};
    SAP_HIDL_SEND_RESPONSE(transferAtrResponse, token, SapResultCode::GENERIC_FAILURE, atr_data);
  }

  return Void();
} /* SapImpl::transferAtrReq */


/*===========================================================================

FUNCTION:  SapImpl::powerReq

===========================================================================*/
Return<void> SapImpl::powerReq
(
  int32_t token,
  bool state
)
{
  auto msg = std::make_shared<UimSAPRequestMsg>(RIL_UIM_SAP_POWER_REQ, state);

  RLOGD("SapImpl::powerReq");

  if (msg)
  {
    GenericCallback<RIL_UIM_SAP_Response> cb(
        ([token](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<RIL_UIM_SAP_Response> responseDataPtr) -> void
    {
      SapResultCode         resp      = SapResultCode::GENERIC_FAILURE;

      if (solicitedMsg && responseDataPtr &&
          status == Message::Callback::Status::SUCCESS)
      {
        switch(responseDataPtr->result)
        {
          case RIL_UIM_SAP_SUCCESS:
            resp = SapResultCode::SUCCESS;
            break;
          case RIL_UIM_SAP_CARD_NOT_ACCESSSIBLE:
            resp = SapResultCode::CARD_NOT_ACCESSSIBLE;
            break;
          case RIL_UIM_SAP_CARD_ALREADY_POWERED_OFF:
            resp = SapResultCode::CARD_ALREADY_POWERED_OFF;
            break;
          case RIL_UIM_SAP_CARD_ALREADY_POWERED_ON:
            resp = SapResultCode::CARD_ALREADY_POWERED_ON;
            break;
          case RIL_UIM_SAP_CARD_REMOVED:
            resp = SapResultCode::CARD_REMOVED;
            break;
          default:
            resp = SapResultCode::GENERIC_FAILURE;
        }
      }

      SAP_HIDL_SEND_RESPONSE(powerResponse, token, resp);
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    SAP_HIDL_SEND_RESPONSE(powerResponse, token, SapResultCode::GENERIC_FAILURE);
  }

  return Void();
} /* SapImpl::powerReq */


/*===========================================================================

FUNCTION:  SapImpl::resetSimReq

===========================================================================*/
Return<void> SapImpl::resetSimReq
(
  int32_t token
)
{
  auto msg = std::make_shared<UimSAPRequestMsg>(RIL_UIM_SAP_RESET_SIM_REQ);

  RLOGD("SapImpl::resetSimReq");

  if (msg)
  {
    GenericCallback<RIL_UIM_SAP_Response> cb(
        ([token](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<RIL_UIM_SAP_Response> responseDataPtr) -> void
    {
      SapResultCode         resp      = SapResultCode::GENERIC_FAILURE;

      if (solicitedMsg && responseDataPtr &&
          status == Message::Callback::Status::SUCCESS)
      {
        switch(responseDataPtr->result)
        {
          case RIL_UIM_SAP_SUCCESS:
            resp = SapResultCode::SUCCESS;
            break;
          case RIL_UIM_SAP_CARD_NOT_ACCESSSIBLE:
            resp = SapResultCode::CARD_NOT_ACCESSSIBLE;
            break;
          case RIL_UIM_SAP_CARD_ALREADY_POWERED_OFF:
            resp = SapResultCode::CARD_ALREADY_POWERED_OFF;
            break;
          case RIL_UIM_SAP_CARD_REMOVED:
            resp = SapResultCode::CARD_REMOVED;
            break;
          default:
            resp = SapResultCode::GENERIC_FAILURE;
        }
      }

      SAP_HIDL_SEND_RESPONSE(resetSimResponse, token, resp);
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    SAP_HIDL_SEND_RESPONSE(resetSimResponse, token, SapResultCode::GENERIC_FAILURE);
  }

  return Void();
} /* SapImpl::resetSimReq */


/*===========================================================================

FUNCTION:  SapImpl::transferCardReaderStatusReq

===========================================================================*/
Return<void> SapImpl::transferCardReaderStatusReq
(
  int32_t token
)
{
  auto msg = std::make_shared<UimSAPRequestMsg>(RIL_UIM_SAP_TRANSFER_CARD_READER_STATUS_REQ);

  RLOGD("SapImpl::transferCardReaderStatusReq");

  if (msg)
  {
    GenericCallback<RIL_UIM_SAP_Response> cb(
        ([token](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status ret_status,
                       std::shared_ptr<RIL_UIM_SAP_Response> responseDataPtr) -> void
    {
      SapResultCode         resp      = SapResultCode::GENERIC_FAILURE;
      int32_t               status    = -1;

      if (solicitedMsg && responseDataPtr &&
          ret_status == Message::Callback::Status::SUCCESS)
      {
        status = responseDataPtr->reader_status;
        switch(responseDataPtr->result)
        {
          case RIL_UIM_SAP_SUCCESS:
            resp = SapResultCode::SUCCESS;
            break;
          case RIL_UIM_SAP_CARD_NOT_ACCESSSIBLE:
          case RIL_UIM_SAP_CARD_REMOVED:
            resp = SapResultCode::DATA_NOT_AVAILABLE;
            break;
          default:
            resp = SapResultCode::GENERIC_FAILURE;
        }
      }

      SAP_HIDL_SEND_RESPONSE(transferCardReaderStatusResponse, token, resp, status);
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    int32_t               status    = -1;

    SAP_HIDL_SEND_RESPONSE(transferCardReaderStatusResponse, token, SapResultCode::GENERIC_FAILURE, status);
  }

  return Void();
} /* SapImpl::transferCardReaderStatusReq */


/*===========================================================================

FUNCTION:  SapImpl::setTransferProtocolReq

===========================================================================*/
Return<void> SapImpl::setTransferProtocolReq
(
  int32_t token,
  SapTransferProtocol transferProtocol
)
{
  RLOGD("SapImpl::setTransferProtocolReq");

  (void)transferProtocol;

  SAP_HIDL_SEND_RESPONSE(transferProtocolResponse, token, SapResultCode::NOT_SUPPORTED);

  return Void();
} /* SapImpl::setTransferProtocolReq */


/*===========================================================================

FUNCTION:  SapModule::SapModule

===========================================================================*/
SapModule::SapModule()
{
  mName  = "SapModule";
  mLooper = std::unique_ptr<ModuleLooper>(new ModuleLooper);

  using std::placeholders::_1;
  mMessageHandler = {
      HANDLER(QcrilInitMessage, SapModule::handleQcrilInitMessage),
      HANDLER(UimSapStatusIndMsg, SapModule::handleSAPInd)
  };
} /* SapModule::SapModule */


/*===========================================================================

FUNCTION:  SapModule::~SapModule

===========================================================================*/
SapModule::~SapModule()
{
  if (sapService) {
    sapService->sapCallback = nullptr;

    sapService = nullptr;
  }
} /* SapModule::~SapModule */


/*===========================================================================

FUNCTION:  SapModule::init

===========================================================================*/
void SapModule::init()
{
  Module::init();
} /* SapModule::init */


/*===========================================================================

FUNCTION:  SapModule::handleQcrilInitMessage

===========================================================================*/
void SapModule::handleQcrilInitMessage
(
  std::shared_ptr<QcrilInitMessage> msg
)
{
  android::status_t          ret = android::OK;

  (void)msg;

  sapService = new SapImpl{};

  if (sapService != NULL)
  {
    switch(qmi_ril_get_process_instance_id())
    {
      case 0:
        ret = sapService->registerAsService("slot1");
        break;
      case 1:
        ret = sapService->registerAsService("slot2");
        break;
      default:
        return;
    }
  }
  RLOGD("sap_register_service status=%d", ret);
} /* SapModule::handleQcrilInitMessage */


/*=========================================================================

FUNCTION:  SapModule::handleSAPInd

===========================================================================*/
void SapModule::handleSAPInd
(
  std::shared_ptr<UimSapStatusIndMsg> msg
)
{
  sp<ISapCallback>  callback   =
    (sapService != nullptr) ? sapService->getCallback() : nullptr;
  Return<void>      retStatus  = Void();

  /* Only 2 unsol responses are send from the QMI IND & it depends on the sap_status:
  1. RIL_SIM_SAP_STATUS_IND, upon status for connection establishment/disconnection
  2. RIL_SIM_SAP_DISCONNECT_IND, upon disconnection request only */

  if (sapService == nullptr || callback == nullptr || msg == NULL)
  {
    RLOGE("NULL sap_service or Null callback");
    return;
  }

  switch (msg->get_status())
  {
    case RIL_UIM_SAP_DISCONNECT_GRACEFUL:
      retStatus = callback->disconnectIndication(0, SapDisconnectType::GRACEFUL);
      break;

    case RIL_UIM_SAP_DISCONNECT_IMMEDIATE:
      retStatus = callback->disconnectIndication(0, SapDisconnectType::IMMEDIATE);
      break;

    case RIL_UIM_SAP_STATUS_CARD_RESET:
      retStatus = callback->statusIndication(0, SapStatus ::CARD_RESET);
      break;

    case RIL_UIM_SAP_STATUS_NOT_ACCESSIABLE:
      retStatus = callback->statusIndication(0, SapStatus::CARD_NOT_ACCESSIBLE);
      break;

    case RIL_UIM_SAP_STATUS_CARD_REMOVED:
      retStatus = callback->statusIndication(0, SapStatus::CARD_REMOVED);
      break;

    case RIL_UIM_SAP_STATUS_CARD_INSERTED:
      retStatus = callback->statusIndication(0, SapStatus::CARD_INSERTED);
      break;

    case RIL_UIM_SAP_STATUS_RECOVERED:
      retStatus = callback->statusIndication(0, SapStatus::RECOVERED);
      break;

    default:
      /* Note that for a succesfully connected case, if the connection response
      isnt sent yet, the STATUS_IND is sent after response callback comes */
      RLOGD("Unknown Status: 0x%x", msg->get_status());
      return;
  }

  sapService->checkReturnStatus(retStatus);
} /* SapModule::handleSAPInd */

#endif
