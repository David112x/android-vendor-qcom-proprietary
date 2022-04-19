/******************************************************************************
@file    uim_remote_remote_service.cpp
@brief   qcril uim remote server service

DESCRIPTION
Implements the server side of the IUimRemoteServiceServer interface.
Handles RIL requests and responses and indications to be received
and sent to client respectively

---------------------------------------------------------------------------

Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
---------------------------------------------------------------------------
******************************************************************************/

#include "framework/Log.h"
#include <framework/ModuleLooper.h>

#include <vendor/qti/hardware/radio/uim_remote_server/1.0/IUimRemoteServiceServer.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <telephony/ril.h>
#include "modules/uim/UimSAPRequestMsg.h"
#include "modules/uim/UimSAPConnectionRequestMsg.h"
#include "uim_remote_server_service.h"

using namespace vendor::qti::hardware::radio::uim_remote_server::V1_0;

using ::android::hardware::Status;
using ::android::hardware::Return;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_array;
using ::android::hardware::Void;
using ::android::hardware::hidl_death_recipient;
using android::sp;

#define TAG "UIM_REMOTE_SERVER_SERVICE"

#define UIM_RMT_SERVER_HIDL_SEND_RESPONSE(func, ...) \
  { \
    sp<IUimRemoteServiceServerResponse>  callback   = (uimRemoteServerService != nullptr) ? uimRemoteServerService->getResponseCallback() : nullptr; \
    Return<void>                         retStatus  = Void(); \
    if (uimRemoteServerService != nullptr && callback != nullptr) \
    { \
      retStatus = callback->func(__VA_ARGS__); \
      uimRemoteServerService->checkReturnStatus(retStatus); \
    } \
  }

struct UimRemoteServerImpl;

static load_module<UimRemoteServerModule>  uim_remote_server_module;
static android::sp<UimRemoteServerImpl>    uimRemoteServerService;


struct UimRemoteServerImpl : public IUimRemoteServiceServer, public hidl_death_recipient
{
  sp<IUimRemoteServiceServerResponse> mResponseCb;

  sp<IUimRemoteServiceServerIndication> mIndicationCb;

  qtimutex::QtiSharedMutex mCallbackLock;

  sp<IUimRemoteServiceServerResponse> getResponseCallback();

  sp<IUimRemoteServiceServerIndication> getIndicationCallback();

  void clearCallbacks();

  void serviceDied( uint64_t, const ::android::wp<::android::hidl::base::V1_0::IBase> &);

  Return<void> setCallback(const sp<IUimRemoteServiceServerResponse>& responseCallback, const sp<IUimRemoteServiceServerIndication>& indicationCallback);

  Return<void> uimRemoteServiceServerConnectReq(int32_t token, int32_t maxMsgSize);

  Return<void> uimRemoteServiceServerDisconnectReq(int32_t token);

  Return<void> uimRemoteServiceServerApduReq(int32_t token, UimRemoteServiceServerApduType type, const hidl_vec<uint8_t>& command);

  Return<void> uimRemoteServiceServerTransferAtrReq(int32_t token);

  Return<void> uimRemoteServiceServerPowerReq(int32_t token, bool state);

  Return<void> uimRemoteServiceServerResetSimReq(int32_t token);

  Return<void> uimRemoteServiceServerTransferCardReaderStatusReq(int32_t token);

  Return<void> uimRemoteServiceServerSetTransferProtocolReq(int32_t token, UimRemoteServiceServerTransferProtocol transferProtocol);

  void checkReturnStatus(Return<void>& ret);
};


/*===========================================================================

FUNCTION:  UimRemoteServerImpl::setCallback

===========================================================================*/
Return<void> UimRemoteServerImpl::setCallback
(
  const sp<IUimRemoteServiceServerResponse>& responseCallback,
  const sp<IUimRemoteServiceServerIndication>& indicationCallback
)
{
  QCRIL_LOG_INFO("UimRemoteServerImpl::setCallback");

  std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  if (mResponseCb != nullptr)
  {
    mResponseCb->unlinkToDeath(this);
  }
  mIndicationCb = indicationCallback;
  mResponseCb = responseCallback;
  if (mResponseCb != nullptr)
  {
    mResponseCb->linkToDeath(this, 0);
  }
  return Status::ok();
} /* UimRemoteServerImpl::setCallback */


/*===========================================================================

FUNCTION:  UimRemoteServerImpl::getResponseCallback

===========================================================================*/
sp<IUimRemoteServiceServerResponse> UimRemoteServerImpl::getResponseCallback()
{
  std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mResponseCb;
} /* UimRemoteServerImpl::getResponseCallback */


/*===========================================================================

FUNCTION:  UimRemoteServerImpl::getIndicationCallback

===========================================================================*/
sp<IUimRemoteServiceServerIndication> UimRemoteServerImpl::getIndicationCallback()
{
  std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mIndicationCb;
} /* UimRemoteServerImpl::getIndicationCallback */


/*===========================================================================

FUNCTION:  UimRemoteServerImpl::serviceDied

===========================================================================*/
void UimRemoteServerImpl::serviceDied
(
 uint64_t,
 const ::android::wp<::android::hidl::base::V1_0::IBase> &
)
{
  QCRIL_LOG_INFO("UimRemoteServerImpl::serviceDied: Client died. Cleaning up callbacks");
  clearCallbacks();
} /* UimRemoteServerImpl::serviceDied */


/*===========================================================================

FUNCTION:  UimRemoteServerImpl::clearCallbacks

===========================================================================*/
void UimRemoteServerImpl::clearCallbacks()
{
  QCRIL_LOG_ERROR("UimRemoteServerImpl::clearCallbacks");

  std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  if (mResponseCb != nullptr)
  {
    mResponseCb = nullptr;
  }
  if (mIndicationCb != nullptr)
  {
    mIndicationCb = nullptr;
  }
} /* UimRemoteServerImpl::clearCallbacks() */


/*===========================================================================

FUNCTION:  UimRemoteServerImpl::checkReturnStatus

===========================================================================*/
void UimRemoteServerImpl::checkReturnStatus
(
  Return<void>& ret
)
{
  if (ret.isOk() == false)
  {
    QCRIL_LOG_ERROR("checkReturnStatus: unable to call response/indication callback: %s",
            ret.description().c_str());
    mResponseCb = nullptr;
  }
} /* UimRemoteServerImpl::checkReturnStatus */


/*===========================================================================

FUNCTION:  UimRemoteServerImpl::uimRemoteServiceServerConnectReq

===========================================================================*/
Return<void> UimRemoteServerImpl::uimRemoteServiceServerConnectReq
(
  int32_t token,
  int32_t maxMsgSize
)
{
  auto msg = std::make_shared<UimSAPConnectionRequestMsg>(RIL_UIM_SAP_CONNECT);

  QCRIL_LOG_DEBUG("UimRemoteServerImpl::uimRemoteServiceServerConnectReq");

  (void) maxMsgSize;

  if (msg)
  {
    GenericCallback<RIL_UIM_SAP_ConnectionResponse> cb(
        ([token](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<RIL_UIM_SAP_ConnectionResponse> responseDataPtr) -> void
    {
      UimRemoteServiceServerConnectRsp  resp  = UimRemoteServiceServerConnectRsp::UIM_REMOTE_SERVICE_SERVER_CONNECT_FAILURE;

      if (solicitedMsg && responseDataPtr &&
          status == Message::Callback::Status::SUCCESS &&
          *responseDataPtr == RIL_UIM_SAP_CONNECTION_SUCCESS)
      {
        resp = UimRemoteServiceServerConnectRsp::UIM_REMOTE_SERVICE_SERVER_SUCCESS;
      }

      UIM_RMT_SERVER_HIDL_SEND_RESPONSE(uimRemoteServiceServerConnectResponse, token, resp, 0);
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    UIM_RMT_SERVER_HIDL_SEND_RESPONSE(uimRemoteServiceServerConnectResponse,
                                      token,
                                      UimRemoteServiceServerConnectRsp::UIM_REMOTE_SERVICE_SERVER_CONNECT_FAILURE,
                                      0);
  }

  return Void();
} /* UimRemoteServerImpl::uimRemoteServiceServerConnectReq */


/*===========================================================================

FUNCTION:  UimRemoteServerImpl::uimRemoteServiceServerDisconnectReq

===========================================================================*/
Return<void> UimRemoteServerImpl::uimRemoteServiceServerDisconnectReq
(
  int32_t token
)
{
  auto msg = std::make_shared<UimSAPConnectionRequestMsg>(RIL_UIM_SAP_DISCONNECT);

  QCRIL_LOG_DEBUG("UimRemoteServerImpl::uimRemoteServiceServerDisconnectReq");

  if (msg)
  {
    GenericCallback<RIL_UIM_SAP_ConnectionResponse> cb(
        ([token](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<RIL_UIM_SAP_ConnectionResponse> responseDataPtr) -> void
    {
      (void) status;
      (void) responseDataPtr;
      (void) solicitedMsg;

      UIM_RMT_SERVER_HIDL_SEND_RESPONSE(uimRemoteServiceServerDisconnectResponse, token);
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    UIM_RMT_SERVER_HIDL_SEND_RESPONSE(uimRemoteServiceServerDisconnectResponse, token);
  }

  return Void();
} /* UimRemoteServerImpl::uimRemoteServiceServerDisconnectReq */


/*===========================================================================

FUNCTION:  UimRemoteServerImpl::uimRemoteServiceServerApduReq

===========================================================================*/
Return<void> UimRemoteServerImpl::uimRemoteServiceServerApduReq
(
  int32_t                        token,
  UimRemoteServiceServerApduType type,
  const hidl_vec<uint8_t>&       command)
{
  auto msg = std::make_shared<UimSAPRequestMsg>(RIL_UIM_SAP_APDU_REQ, command);

  QCRIL_LOG_DEBUG("UimRemoteServerImpl::uimRemoteServiceServerApduReq");

  (void) type;

  if (msg)
  {
    GenericCallback<RIL_UIM_SAP_Response> cb(
        ([token](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<RIL_UIM_SAP_Response> responseDataPtr) -> void
    {
      UimRemoteServiceServerResultCode  resp       = UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_GENERIC_FAILURE;
      std::vector<uint8_t>              apdu_data  = {};

      if (solicitedMsg && responseDataPtr &&
          status == Message::Callback::Status::SUCCESS)
      {
        apdu_data = responseDataPtr->atr_apdu_data;

        switch(responseDataPtr->result)
        {
          case RIL_UIM_SAP_SUCCESS:
            resp = UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_SUCCESS;
            break;
          case RIL_UIM_SAP_CARD_NOT_ACCESSSIBLE:
            resp = UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_CARD_NOT_ACCESSSIBLE;
            break;
          case RIL_UIM_SAP_CARD_ALREADY_POWERED_OFF:
            resp = UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_CARD_ALREADY_POWERED_OFF;
            break;
          case RIL_UIM_SAP_CARD_REMOVED:
            resp = UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_CARD_REMOVED;
            break;
          default:
            resp = UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_GENERIC_FAILURE;
        }
      }

      UIM_RMT_SERVER_HIDL_SEND_RESPONSE(uimRemoteServiceServerApduResponse, token, resp, apdu_data);
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    std::vector<uint8_t>  apdu_data  = {};
    UIM_RMT_SERVER_HIDL_SEND_RESPONSE(uimRemoteServiceServerApduResponse,
                                      token,
                                      UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_GENERIC_FAILURE,
                                      apdu_data);
  }

  return Void();
} /* UimRemoteServerImpl::uimRemoteServiceServerApduReq */


/*===========================================================================

FUNCTION:  UimRemoteServerImpl::uimRemoteServiceServerTransferAtrReq

===========================================================================*/
Return<void> UimRemoteServerImpl::uimRemoteServiceServerTransferAtrReq
(
  int32_t token
)
{
  auto msg = std::make_shared<UimSAPRequestMsg>(RIL_UIM_SAP_TRANSFER_ATR_REQ);

  QCRIL_LOG_DEBUG("UimRemoteServerImpl::uimRemoteServiceServerTransferAtrReq");

  if (msg)
  {
    GenericCallback<RIL_UIM_SAP_Response> cb(
        ([token](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<RIL_UIM_SAP_Response> responseDataPtr) -> void
    {
      UimRemoteServiceServerResultCode  resp      = UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_GENERIC_FAILURE;
      std::vector<uint8_t>              atr_data  = {};

      if (solicitedMsg && responseDataPtr &&
          status == Message::Callback::Status::SUCCESS)
      {
        atr_data = responseDataPtr->atr_apdu_data;

        switch(responseDataPtr->result)
        {
          case RIL_UIM_SAP_SUCCESS:
            resp = UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_SUCCESS;
            break;
          case RIL_UIM_SAP_CARD_NOT_ACCESSSIBLE:
            resp = UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_DATA_NOT_AVAILABLE;
            break;
          case RIL_UIM_SAP_CARD_ALREADY_POWERED_OFF:
            resp = UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_CARD_ALREADY_POWERED_OFF;
            break;
          case RIL_UIM_SAP_CARD_REMOVED:
            resp = UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_CARD_REMOVED;
            break;
          default:
            resp = UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_GENERIC_FAILURE;
        }
      }

      UIM_RMT_SERVER_HIDL_SEND_RESPONSE(uimRemoteServiceServerTransferAtrResponse, token, resp, atr_data);
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    std::vector<uint8_t>  atr_data  = {};
    UIM_RMT_SERVER_HIDL_SEND_RESPONSE(uimRemoteServiceServerTransferAtrResponse,
                                      token,
                                      UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_GENERIC_FAILURE,
                                      atr_data);
  }

  return Void();
} /* UimRemoteServerImpl::uimRemoteServiceServerTransferAtrReq */


/*===========================================================================

FUNCTION:  UimRemoteServerImpl::uimRemoteServiceServerPowerReq

===========================================================================*/
Return<void> UimRemoteServerImpl::uimRemoteServiceServerPowerReq
(
  int32_t token,
  bool    state
)
{
  auto msg = std::make_shared<UimSAPRequestMsg>(RIL_UIM_SAP_POWER_REQ, state);

  QCRIL_LOG_DEBUG("UimRemoteServerImpl::uimRemoteServiceServerPowerReq");

  if (msg)
  {
    GenericCallback<RIL_UIM_SAP_Response> cb(
        ([token](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<RIL_UIM_SAP_Response> responseDataPtr) -> void
    {
      UimRemoteServiceServerResultCode  resp = UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_GENERIC_FAILURE;

      if (solicitedMsg && responseDataPtr &&
          status == Message::Callback::Status::SUCCESS)
      {
        switch(responseDataPtr->result)
        {
          case RIL_UIM_SAP_SUCCESS:
            resp = UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_SUCCESS;
            break;
          case RIL_UIM_SAP_CARD_NOT_ACCESSSIBLE:
            resp = UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_CARD_NOT_ACCESSSIBLE;
            break;
          case RIL_UIM_SAP_CARD_ALREADY_POWERED_OFF:
            resp = UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_CARD_ALREADY_POWERED_OFF;
            break;
          case RIL_UIM_SAP_CARD_ALREADY_POWERED_ON:
            resp = UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_CARD_ALREADY_POWERED_ON;
            break;
          case RIL_UIM_SAP_CARD_REMOVED:
            resp = UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_CARD_REMOVED;
            break;
          default:
            resp = UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_GENERIC_FAILURE;
        }
      }

      UIM_RMT_SERVER_HIDL_SEND_RESPONSE(uimRemoteServiceServerPowerResponse, token, resp);
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    UIM_RMT_SERVER_HIDL_SEND_RESPONSE(uimRemoteServiceServerPowerResponse,
                                      token,
                                      UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_GENERIC_FAILURE);
  }

  return Void();
} /* UimRemoteServerImpl::uimRemoteServiceServerPowerReq */


/*===========================================================================

FUNCTION:  UimRemoteServerImpl::uimRemoteServiceServerResetSimReq

===========================================================================*/
Return<void> UimRemoteServerImpl::uimRemoteServiceServerResetSimReq
(
  int32_t token
)
{
  auto msg = std::make_shared<UimSAPRequestMsg>(RIL_UIM_SAP_RESET_SIM_REQ);

  QCRIL_LOG_DEBUG("UimRemoteServerImpl::uimRemoteServiceServerResetSimReq");

  if (msg)
  {
    GenericCallback<RIL_UIM_SAP_Response> cb(
        ([token](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                       std::shared_ptr<RIL_UIM_SAP_Response> responseDataPtr) -> void
    {
      UimRemoteServiceServerResultCode  resp  = UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_GENERIC_FAILURE;

      if (solicitedMsg && responseDataPtr &&
          status == Message::Callback::Status::SUCCESS)
      {
        switch(responseDataPtr->result)
        {
          case RIL_UIM_SAP_SUCCESS:
            resp = UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_SUCCESS;
            break;
          case RIL_UIM_SAP_CARD_NOT_ACCESSSIBLE:
            resp = UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_CARD_NOT_ACCESSSIBLE;
            break;
          case RIL_UIM_SAP_CARD_ALREADY_POWERED_OFF:
            resp = UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_CARD_ALREADY_POWERED_OFF;
            break;
          case RIL_UIM_SAP_CARD_REMOVED:
            resp = UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_CARD_REMOVED;
            break;
          default:
            resp = UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_GENERIC_FAILURE;
        }
      }

      UIM_RMT_SERVER_HIDL_SEND_RESPONSE(uimRemoteServiceServerResetSimResponse, token, resp);
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    UIM_RMT_SERVER_HIDL_SEND_RESPONSE(uimRemoteServiceServerResetSimResponse,
                                      token,
                                      UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_GENERIC_FAILURE);
  }

  return Void();
} /* UimRemoteServerImpl::uimRemoteServiceServerResetSimReq */


/*===========================================================================

FUNCTION:  UimRemoteServerImpl::uimRemoteServiceServerTransferCardReaderStatusReq

===========================================================================*/
Return<void> UimRemoteServerImpl::uimRemoteServiceServerTransferCardReaderStatusReq
(
  int32_t token
)
{
  auto msg = std::make_shared<UimSAPRequestMsg>(RIL_UIM_SAP_TRANSFER_CARD_READER_STATUS_REQ);

  QCRIL_LOG_DEBUG("UimRemoteServerImpl::uimRemoteServiceServerTransferCardReaderStatusReq");

  if (msg)
  {
    GenericCallback<RIL_UIM_SAP_Response> cb(
        ([token](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status ret_status,
                       std::shared_ptr<RIL_UIM_SAP_Response> responseDataPtr) -> void
    {
      UimRemoteServiceServerResultCode  resp    = UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_GENERIC_FAILURE;
      int32_t                           status  = -1;

      if (solicitedMsg && responseDataPtr &&
          ret_status == Message::Callback::Status::SUCCESS)
      {
        status = responseDataPtr->reader_status;
        switch(responseDataPtr->result)
        {
          case RIL_UIM_SAP_SUCCESS:
            resp = UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_SUCCESS;
            break;
          case RIL_UIM_SAP_CARD_NOT_ACCESSSIBLE:
          case RIL_UIM_SAP_CARD_REMOVED:
            resp = UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_DATA_NOT_AVAILABLE;
            break;
          default:
            resp = UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_GENERIC_FAILURE;
        }
      }

      UIM_RMT_SERVER_HIDL_SEND_RESPONSE(uimRemoteServiceServerTransferCardReaderStatusResponse, token, resp, status);
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    int32_t               status    = -1;

    UIM_RMT_SERVER_HIDL_SEND_RESPONSE(uimRemoteServiceServerTransferCardReaderStatusResponse,
                                      token,
                                      UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_GENERIC_FAILURE,
                                      status);
  }

  return Void();
} /* UimRemoteServerImpl::uimRemoteServiceServerTransferCardReaderStatusReq */


/*===========================================================================

FUNCTION:  UimRemoteServerImpl::uimRemoteServiceServerSetTransferProtocolReq

===========================================================================*/
Return<void> UimRemoteServerImpl::uimRemoteServiceServerSetTransferProtocolReq
(
  int32_t token,
  UimRemoteServiceServerTransferProtocol transferProtocol
)
{
  QCRIL_LOG_DEBUG("UimRemoteServerImpl::uimRemoteServiceServerSetTransferProtocolReq");

  (void)transferProtocol;

  UIM_RMT_SERVER_HIDL_SEND_RESPONSE(uimRemoteServiceServerTransferProtocolResponse,
                                    token,
                                    UimRemoteServiceServerResultCode::UIM_REMOTE_SERVICE_SERVER_NOT_SUPPORTED);

  return Void();
} /* UimRemoteServerImpl::uimRemoteServiceServerSetTransferProtocolReq */


/*===========================================================================

FUNCTION:  UimRemoteServerModule::UimRemoteServerModule

===========================================================================*/
UimRemoteServerModule::UimRemoteServerModule()
{
  mName  = "UimRemoteServerModule";
  mLooper = std::unique_ptr<ModuleLooper>(new ModuleLooper);

  using std::placeholders::_1;
  mMessageHandler = {
      HANDLER(QcrilInitMessage, UimRemoteServerModule::handleQcrilInitMessage),
      HANDLER(UimSapStatusIndMsg, UimRemoteServerModule::handleSAPInd)
  };
} /* UimRemoteServerModule::UimRemoteServerModule */


/*===========================================================================

FUNCTION:  UimRemoteServerModule::~UimRemoteServerModule

===========================================================================*/
UimRemoteServerModule::~UimRemoteServerModule()
{
  mLooper = nullptr;
} /* UimRemoteServerModule::~UimRemoteServerModule */


/*===========================================================================

FUNCTION:  UimRemoteServerModule::init

===========================================================================*/
void UimRemoteServerModule::init()
{
  Module::init();
} /* UimRemoteServerModule::init */


/*===========================================================================

FUNCTION:  UimRemoteServerModule::handleQcrilInitMessage

===========================================================================*/
void UimRemoteServerModule::handleQcrilInitMessage
(
  std::shared_ptr<QcrilInitMessage> msg
)
{
  android::status_t          ret = android::OK;

  (void)msg;

  uimRemoteServerService = new UimRemoteServerImpl{};

  if (uimRemoteServerService != NULL)
  {
    ret = uimRemoteServerService->registerAsService("uimRemoteServer" + std::to_string(qmi_ril_get_process_instance_id()));
  }

  QCRIL_LOG_DEBUG("uim_remote_server_register_service status=%d", ret);
} /* UimRemoteServerModule::handleQcrilInitMessage */


/*=========================================================================

FUNCTION:  UimRemoteServerModule::handleSAPInd

===========================================================================*/
void UimRemoteServerModule::handleSAPInd
(
  std::shared_ptr<UimSapStatusIndMsg> msg
)
{
  sp<IUimRemoteServiceServerIndication>  callback   =
    (uimRemoteServerService != nullptr) ? uimRemoteServerService->getIndicationCallback() : nullptr;
  Return<void>                           retStatus  = Void();

  /* Only 2 unsol responses are send from the QMI IND & it depends on the sap_status:
  1. RIL_SIM_SAP_STATUS_IND, upon status for connection establishment/disconnection
  2. RIL_SIM_SAP_DISCONNECT_IND, upon disconnection request only */

  if (uimRemoteServerService == nullptr || callback == nullptr || msg == NULL)
  {
    QCRIL_LOG_ERROR("NULL sap_service or Null callback");
    return;
  }

  switch (msg->get_status())
  {
    case RIL_UIM_SAP_DISCONNECT_GRACEFUL:
      retStatus = callback->uimRemoteServiceServerDisconnectIndication(
         UimRemoteServiceServerDisconnectType::UIM_REMOTE_SERVICE_SERVER_DISCONNECT_GRACEFUL);
      break;

    case RIL_UIM_SAP_DISCONNECT_IMMEDIATE:
      retStatus = callback->uimRemoteServiceServerDisconnectIndication(
         UimRemoteServiceServerDisconnectType::UIM_REMOTE_SERVICE_SERVER_DISCONNECT_IMMEDIATE);
      break;

    case RIL_UIM_SAP_STATUS_CARD_RESET:
      retStatus = callback->uimRemoteServiceServerStatusIndication(
         UimRemoteServiceServerStatus::UIM_REMOTE_SERVICE_SERVER_CARD_RESET);
      break;

    case RIL_UIM_SAP_STATUS_NOT_ACCESSIABLE:
      retStatus = callback->uimRemoteServiceServerStatusIndication(
         UimRemoteServiceServerStatus::UIM_REMOTE_SERVICE_SERVER_CARD_NOT_ACCESSIBLE);
      break;

    case RIL_UIM_SAP_STATUS_CARD_REMOVED:
      retStatus = callback->uimRemoteServiceServerStatusIndication(
         UimRemoteServiceServerStatus::UIM_REMOTE_SERVICE_SERVER_CARD_REMOVED);
      break;

    case RIL_UIM_SAP_STATUS_CARD_INSERTED:
      retStatus = callback->uimRemoteServiceServerStatusIndication(
         UimRemoteServiceServerStatus::UIM_REMOTE_SERVICE_SERVER_CARD_INSERTED);
      break;

    case RIL_UIM_SAP_STATUS_RECOVERED:
      retStatus = callback->uimRemoteServiceServerStatusIndication(
         UimRemoteServiceServerStatus::UIM_REMOTE_SERVICE_SERVER_RECOVERED);
      break;

    default:
      /* Note that for a succesfully connected case, if the connection response
      isnt sent yet, the STATUS_IND is sent after response callback comes */
      QCRIL_LOG_DEBUG("Unknown Status: 0x%x", msg->get_status());
      return;
  }

  if (retStatus.isOk() == false)
  {
    QCRIL_LOG_ERROR("checkReturnStatus: unable to call response/indication callback: %s",
            retStatus.description().c_str());

    if (uimRemoteServerService != NULL)
    {
      uimRemoteServerService->mIndicationCb = nullptr;
    }
  }
} /* UimRemoteServerModule::handleSAPInd */
