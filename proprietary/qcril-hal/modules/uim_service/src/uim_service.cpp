/******************************************************************************
@file    uim_service.cpp
@brief   qcril uim service

DESCRIPTION
Implements the server side of the IUim interface.
Handles RIL requests and responses and indications to be received
and sent to client respectively

---------------------------------------------------------------------------

Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
---------------------------------------------------------------------------
******************************************************************************/
#ifndef QMI_RIL_UTF

#include "framework/Log.h"
#include <framework/ModuleLooper.h>

#include "modules/uim/UimRemoteSimlockRequestMsg.h"
#include "modules/uim/UimGBAInitRequestMsg.h"
#include "modules/uim/UimGBAGetIMPIRequestMsg.h"

#include <vendor/qti/hardware/radio/uim/1.2/IUim.h>
#include <vendor/qti/hardware/radio/uim/1.2/IUimResponse.h>
#include <vendor/qti/hardware/radio/uim/1.2/types.h>
#include <vendor/qti/hardware/radio/uim/1.1/types.h>
#include <vendor/qti/hardware/radio/uim/1.0/types.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

#include "uim_service.h"

#include <cutils/properties.h>
#include <cstring>

using namespace vendor::qti::hardware::radio::uim;
using namespace vendor::qti::hardware::radio::uim::V1_0;
using namespace vendor::qti::hardware::radio::uim::V1_1;
using namespace vendor::qti::hardware::radio::uim::V1_2;
using ::android::hardware::hidl_death_recipient;
using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_string;

using ::android::hardware::Return;
using ::android::sp;
using ::android::wp;
using ::android::hardware::Void;
using ::android::hardware::Status;

#define TAG "UIM_SERVICE"

/* This is used to track HIDL client status which uses Remote simlock functionality */
#define UIM_REMOTE_SIMLOCK_COOKIE 0

/* This is used to track HIDL client status which uses GBA functionality*/
#define UIM_GBA_COOKIE 1

struct UimImpl;

static load_module<UimServiceModule>  uim_service_module;
static android::sp<UimImpl>           uimService;

struct UimImpl : public V1_2::IUim, public hidl_death_recipient {
  sp<V1_0::IUimResponse>    mResponseCb;
  sp<V1_1::IUimResponse>    mResponseCbV1_1;
  sp<V1_2::IUimResponse>    mResponseCbV1_2;
  sp<IUimIndication>        mIndicationCb;
  qtimutex::QtiSharedMutex  mCallbackLock;

  sp<V1_0::IUimResponse> getResponseCallback();
  sp<IUimIndication> getIndicationCallback();
  sp<V1_1::IUimResponse> getResponseCallbackV1_1();
  sp<V1_2::IUimResponse> getResponseCallbackV1_2();

  void clearCallbacks();

  void clearGbaCallbacks();

  // Functions from hidl_death_recipient
  void serviceDied( uint64_t, const ::android::wp<::android::hidl::base::V1_0::IBase> &);

  // Methods from IUimResponse
  Return<void> setCallback(const sp<V1_0::IUimResponse>& responseCallback,
                           const sp<IUimIndication>& indicationCallback);

  Return<void> setGbaCallback(const sp<V1_2::IUimResponse>& responseCallback);

  Return<void> UimRemoteSimlockRequest(int32_t token, V1_0::UimRemoteSimlockOperationType simlockOp,
                                       const hidl_vec<uint8_t>& simlockData);

  Return<void> uimRemoteSimlockRequest_1_1(int32_t token,
         V1_1::UimRemoteSimlockOperationType simlockOp, const hidl_vec<uint8_t>& simlockData);

  Return<void> uimGbaInit(uint32_t token, const hidl_vec<uint8_t>& securityProtocol,
                          const hidl_string& nafFullyQualifiedDomainName,
                          V1_2::UimApplicationType app_type,
                          bool forceBootStrapping, const hidl_string& apn);

  Return<void> uimGetImpi(uint32_t token, V1_2::UimApplicationType app_type, bool secure);

  void uimRemoteSimlockResponse
  (
    int                                   token,
    UimRemoteSimlockResponseType          response,
    V1_1::UimRemoteSimlockOperationType   op_type,
    std::vector<uint8_t>                & rsp_ptr,
    UimRemoteSimlockVersion               version,
    UimRemoteSimlockStatus                status,
    int                                   timer_value
  );

  void uimGbainitResponse
  (
    int token,
    V1_2::UimGbaStatus    gbaStatus,
    bool                  isKsNafEncrypted,
    std::vector<uint8_t>  ksNaf,
    std::string           bTid,
    std::string           lifeTime
  );

  void uimGetImpiResponse
  (
    int token,
    V1_2::UimGbaStatus    gbaStatus,
    bool                  is_impi_encrypted,
    std::vector<uint8_t>  impi
  );
};


/*===========================================================================

FUNCTION:  UimImpl::setCallback

===========================================================================*/
Return<void> UimImpl::setCallback
(
  const sp<V1_0::IUimResponse>& responseCallback,
  const sp<IUimIndication>& indicationCallback
)
{
  QCRIL_LOG_DEBUG("UimImpl::setCallback");

  std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  if (mResponseCb != nullptr)
  {
    mResponseCb->unlinkToDeath(this);
  }
  mIndicationCb = indicationCallback;
  mResponseCb = responseCallback;
  if (mResponseCb != nullptr)
  {
    mResponseCb->linkToDeath(this, UIM_REMOTE_SIMLOCK_COOKIE);
  }

  mResponseCbV1_1 =
    V1_1::IUimResponse::castFrom(responseCallback).withDefault(nullptr);

  return Status::ok();
} /* UimImpl::setCallback */

/*===========================================================================

FUNCTION:  UimImpl::setGbaCallback

===========================================================================*/
Return<void> UimImpl::setGbaCallback
(
  const sp<V1_2::IUimResponse>& responseCallback
)
{
  QCRIL_LOG_DEBUG("UimImpl::setGbaCallback");

  std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  if (mResponseCbV1_2 != nullptr)
  {
    mResponseCbV1_2->unlinkToDeath(this);
  }
  mResponseCbV1_2 = responseCallback;

  if (mResponseCbV1_2 != nullptr)
  {
    mResponseCbV1_2->linkToDeath(this, UIM_GBA_COOKIE);
  }

  return Status::ok();
} /* UimImpl::setGbaCallback */



/*===========================================================================

FUNCTION:  UimImpl::setCallback

===========================================================================*/
sp<V1_0::IUimResponse> UimImpl::getResponseCallback()
{
  std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mResponseCb;
} /* UimImpl::getResponseCallback */


/*===========================================================================

FUNCTION:  UimImpl::getIndicationCallback

===========================================================================*/
sp<IUimIndication> UimImpl::getIndicationCallback()
{
  std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mIndicationCb;
} /* UimImpl::getIndicationCallback */


/*===========================================================================

FUNCTION:  UimImpl::getResponseCallbackV1_1

===========================================================================*/
sp<V1_1::IUimResponse> UimImpl::getResponseCallbackV1_1()
{
  std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mResponseCbV1_1;
} /* UimImpl::getResponseCallbackV1_1 */


/*===========================================================================

FUNCTION:  UimImpl::getResponseCallbackV1_2

===========================================================================*/
sp<V1_2::IUimResponse> UimImpl::getResponseCallbackV1_2()
{
  std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mResponseCbV1_2;
} /* UimImpl::getResponseCallbackV1_2 */


/*===========================================================================

FUNCTION:  UimImpl::serviceDied

===========================================================================*/
void UimImpl::serviceDied
(
 uint64_t cookie,
 const ::android::wp<::android::hidl::base::V1_0::IBase> &
)
{
  QCRIL_LOG_DEBUG("UimImpl::serviceDied: Client died. Cleaning up callbacks");

  if(cookie == UIM_REMOTE_SIMLOCK_COOKIE)
  {
    clearCallbacks();
  }
  else if (cookie == UIM_GBA_COOKIE)
  {
    clearGbaCallbacks();
  }
} /* UimImpl::serviceDied */


/*===========================================================================

FUNCTION:  UimImpl::clearCallbacks

===========================================================================*/
void UimImpl::clearCallbacks()
{
  QCRIL_LOG_ERROR("UimImpl::clearCallbacks");

  std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  if (mResponseCb != nullptr)
  {
    mResponseCb->unlinkToDeath(this);
    mResponseCb = nullptr;
  }

  mResponseCbV1_1 = nullptr;
  mIndicationCb = nullptr;
} /* UimImpl::clearCallbacks() */


/*===========================================================================

FUNCTION:  UimImpl::clearGbaCallbacks

===========================================================================*/
void UimImpl::clearGbaCallbacks()
{
  QCRIL_LOG_ERROR("UimImpl::clearGbaCallbacks");

  std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  if (mResponseCbV1_2 != nullptr)
  {
    mResponseCbV1_2->unlinkToDeath(this);
    mResponseCbV1_2 = nullptr;
  }

  mResponseCbV1_2 = nullptr;
} /* UimImpl::clearGbaCallbacks() */


/*===========================================================================

FUNCTION:  UimImpl::uimRemoteSimlockResponse

===========================================================================*/
void UimImpl::uimRemoteSimlockResponse
(
  int                                   token,
  UimRemoteSimlockResponseType          response,
  V1_1::UimRemoteSimlockOperationType   op_type,
  std::vector<uint8_t>                & simlock_data,
  UimRemoteSimlockVersion               version,
  UimRemoteSimlockStatus                status,
  int32_t                               timer_value
)
{
  sp<V1_0::IUimResponse>  respCb       = getResponseCallback();
  sp<V1_1::IUimResponse>  respCbV1_1   = getResponseCallbackV1_1();
  Return<void>            ret          = Status::ok();

  if (respCb == nullptr && respCbV1_1 == nullptr)
  {
    QCRIL_LOG_ERROR("uimRemoteSimlockResponse responseCb is null");
    return;
  }
  QCRIL_LOG_INFO("uimRemoteSimlockResponse token=%d", token);

  QCRIL_LOG_INFO("op_type:%d,  status : %d", op_type, status);

  if (respCbV1_1 != nullptr)
  {
    switch(op_type)
    {
      case V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_UNLOCK_TIMER_START:
      case V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_UNLOCK_TIMER_STOP:
        ret = respCbV1_1->uimRemoteSimlockTimerResponse_1_1(token, response, timer_value);
        break;

      default:
        ret = respCbV1_1->uimRemoteSimlockResponse_1_1(token,
                                                       response,
                                                       op_type,
                                                       simlock_data,
                                                       version,
                                                       status);
    }
  }
  else if (respCb != nullptr)
  {
    ret = respCb->UimRemoteSimlockResponse(token,
                                           response,
                                           (V1_0::UimRemoteSimlockOperationType)op_type,
                                           simlock_data,
                                           version,
                                           status);
  }

  if (!ret.isOk())
  {
    QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
  }
}


/*===========================================================================

FUNCTION:  UimImpl::UimRemoteSimlockRequest

===========================================================================*/
Return<void> UimImpl::UimRemoteSimlockRequest
(
  int32_t token,
  V1_0::UimRemoteSimlockOperationType simlockOp,
  const hidl_vec<uint8_t>& simlockData
)
{
  V1_1::UimRemoteSimlockOperationType  simlock_op =
                             (V1_1::UimRemoteSimlockOperationType)simlockOp;

  QCRIL_LOG_DEBUG("UimRemoteSimlockRequest");

  switch(simlockOp)
  {
    case V1_0::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_GENERATE_ENCRYPTED_KEY:
    case V1_0::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_PROCESS_SIMLOCK_DATA:
    case V1_0::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_GENERATE_HMAC:
    case V1_0::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_GET_MAX_SUPPORTED_VERSION:
    case V1_0::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_GET_STATUS:
      return uimRemoteSimlockRequest_1_1(token, simlock_op, simlockData);

    default:
      return Status::fromExceptionCode(Status::EX_ILLEGAL_ARGUMENT);
  }
} /* UimImpl::UimRemoteSimlockRequest */


/*===========================================================================

FUNCTION:  UimImpl::UimRemoteSimlockRequest

===========================================================================*/
Return<void> UimImpl::uimRemoteSimlockRequest_1_1
(
  int32_t token,
  V1_1::UimRemoteSimlockOperationType simlockOp,
  const hidl_vec<uint8_t>& simlockData
)
{
  auto msg = std::make_shared<UimRemoteSimlockRequestMsg>(
              (RIL_UIM_RemoteSimlockOperationType)simlockOp, simlockData);

  QCRIL_LOG_DEBUG("uimRemoteSimlockRequest_1_1");

  if (msg)
  {
    GenericCallback<RIL_UIM_RemoteSimlockResponse> cb(
        ([this, token, simlockOp](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status ret_status,
                       std::shared_ptr<RIL_UIM_RemoteSimlockResponse> rsp_ptr) -> void
    {
      V1_1::UimRemoteSimlockOperationType  op_type  = simlockOp;
      UimRemoteSimlockResponseType         rsp_type = UimRemoteSimlockResponseType::UIM_REMOTE_SIMLOCK_RESP_FAILURE;
      std::vector<uint8_t>                 data     = {};
      UimRemoteSimlockVersion              version  = {};
      UimRemoteSimlockStatus               status   = {};
      int32_t                              timer    = -1;

      if (solicitedMsg && rsp_ptr &&
          ret_status == Message::Callback::Status::SUCCESS)
      {
        op_type = (V1_1::UimRemoteSimlockOperationType)rsp_ptr->op_type;
        rsp_type = (UimRemoteSimlockResponseType)rsp_ptr->rsp_type;
        data = rsp_ptr->rsp_data;
        version.majorVersion = rsp_ptr->version.majorVersion;
        version.minorVersion = rsp_ptr->version.minorVersion;
        status.status = (UimRemoteSimlockStatusType)rsp_ptr->status.state;
        status.unlockTime = rsp_ptr->status.unlockTime;
        timer = rsp_ptr->timer_value;
      }

      uimRemoteSimlockResponse(token, rsp_type, op_type, data, version, status, timer);
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    std::vector<uint8_t>                 data     = {};
    UimRemoteSimlockVersion              version  = {};
    UimRemoteSimlockStatus               status   = {};

    uimRemoteSimlockResponse(token, UimRemoteSimlockResponseType::UIM_REMOTE_SIMLOCK_RESP_FAILURE, simlockOp, data, version, status, -1);
  }

  return Void();
} /* UimImpl::uimRemoteSimlockRequest_1_1 */


/*===========================================================================

FUNCTION:  UimImpl::uimGbainitResponse

===========================================================================*/

void UimImpl::uimGbainitResponse
(
  int token,
  V1_2::UimGbaStatus    gbaStatus,
  bool                  isKsNafEncrypted,
  std::vector<uint8_t>  ksNaf,
  std::string           bTid,
  std::string           lifeTime
)
{

  QCRIL_LOG_INFO("uimGbainitResponse token=%d", token);

  sp<V1_2::IUimResponse>  respCbV1_2      = getResponseCallbackV1_2();
  Return<void>            ret             = Status::ok();
  hidl_string             bootstrappingId = {};
  hidl_string             keyLifeTime     = {};

  if (respCbV1_2 == nullptr)
  {
    QCRIL_LOG_ERROR("uimGbainitResponse respCbV1_2 is null");
    return;
  }

  if(!bTid.empty())
  {
    bootstrappingId.setToExternal(bTid.c_str(), bTid.length());
  }

  if(!lifeTime.empty())
  {
    keyLifeTime.setToExternal(lifeTime.c_str(), lifeTime.length());
  }

  ret = respCbV1_2->uimGbaInitResponse(token,
                                       gbaStatus,
                                       isKsNafEncrypted,
                                       ksNaf,
                                       bootstrappingId,
                                       keyLifeTime);
  if (!ret.isOk())
  {
    QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
  }

}/* uimGbainitResponse */


/*===========================================================================

FUNCTION:  UimImpl::uimGetImpiResponse

===========================================================================*/

void UimImpl::uimGetImpiResponse
(
int                   token,
V1_2::UimGbaStatus    gbaStatus,
bool                  is_impi_encrypted,
std::vector<uint8_t>  impi
)
{

  QCRIL_LOG_INFO("uimGetImpiResponse token=%d", token);

  sp<V1_2::IUimResponse>  respCbV1_2   = getResponseCallbackV1_2();
  Return<void>            ret          = Status::ok();

  if (respCbV1_2 == nullptr)
  {
    QCRIL_LOG_ERROR("uimGetImpiResponse respCbV1_2 is null");
    return;
  }

  ret = respCbV1_2->uimGbaGetImpiResponse(token,
                                          gbaStatus,
                                          is_impi_encrypted,
                                          impi);
  if (!ret.isOk())
  {
    QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
  }

}/* uimGetImpiResponse */


/*===========================================================================

FUNCTION:  UimImpl::uimGbaInit

===========================================================================*/
Return<void> UimImpl::uimGbaInit
(
  uint32_t                      token,
  const hidl_vec < uint8_t >  & securityProtocol,
  const hidl_string           & nafFullyQualifiedDomainName,
  V1_2 :: UimApplicationType    app_type,
  bool                          forceBootStrapping,
  const hidl_string           & apn
)
{
  QCRIL_LOG_DEBUG("uimGbaInit");

  RIL_UIM_GBA_InitParams gba_init_params = {};

  gba_init_params.slot_id             = qmi_ril_get_process_instance_id();
  gba_init_params.force_bootstrapping = forceBootStrapping;
  gba_init_params.domain_name         = nafFullyQualifiedDomainName;
  gba_init_params.protocol            = securityProtocol;
  gba_init_params.app_type            = (RIL_UIM_AppType)app_type;

  if(!apn.empty())
  {
    gba_init_params.apn = apn;
  }

  auto msg = std::make_shared<UimGBAInitRequestMsg>(gba_init_params);

  if (msg)
  {
    GenericCallback<RIL_UIM_GBA_InitResponse> cb(
        ([this, token](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status ret_status,
                       std::shared_ptr<RIL_UIM_GBA_InitResponse> rsp_ptr) -> void
    {
      V1_2::UimGbaStatus    gbaStatus         = V1_2::UimGbaStatus::UIM_GBA_GENERIC_FAILURE;
      bool                  isKsNafEncrypted  = false;
      std::vector<uint8_t>  ksNaf             = {};
      std::string           bTid              = "";
      std::string           lifeTime          = "";

      if (solicitedMsg && rsp_ptr &&
          ret_status == Message::Callback::Status::SUCCESS)
      {
        if(rsp_ptr->err == RIL_UIM_E_SUCCESS)
        {
          gbaStatus       = V1_2::UimGbaStatus::UIM_GBA_SUCCESS;
        }

        isKsNafEncrypted  = (bool)rsp_ptr->naf_type;
        ksNaf             = rsp_ptr->naf_response;
        bTid              = rsp_ptr->bootstrap_tid;
        lifeTime          = rsp_ptr->life_time;
      }

      uimGbainitResponse(token, gbaStatus, isKsNafEncrypted, ksNaf, bTid, lifeTime);
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    V1_2::UimGbaStatus    gbaStatus         = V1_2::UimGbaStatus::UIM_GBA_GENERIC_FAILURE;
    bool                  isKsNafEncrypted  = false;
    std::vector<uint8_t>  ksNaf             = {};
    std::string           bTid              = "";
    std::string           lifeTime          = "";

    uimGbainitResponse(token, gbaStatus, isKsNafEncrypted, ksNaf, bTid, lifeTime);
  }

  return Void();

}/* uimGbaInit */


/*===========================================================================

FUNCTION:  UimImpl::uimGetImpi

===========================================================================*/
Return<void> UimImpl::uimGetImpi
(
uint32_t                     token,
V1_2 :: UimApplicationType   app_type,
bool                         secure
)
{

  QCRIL_LOG_DEBUG("uimGetImpi");

  RIL_UIM_GBA_IMPI_Params gba_get_impi_params = {};

  gba_get_impi_params.slot_id  = qmi_ril_get_process_instance_id();
  gba_get_impi_params.app_type = (RIL_UIM_AppType)app_type;
  gba_get_impi_params.secure   = secure;

  auto msg = std::make_shared<UimGBAGetIMPIRequestMsg>(gba_get_impi_params);

  if (msg)
  {
    GenericCallback<RIL_UIM_GBA_IMPI_Response> cb(
        ([this, token](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status ret_status,
                       std::shared_ptr<RIL_UIM_GBA_IMPI_Response> rsp_ptr) -> void
    {
      V1_2::UimGbaStatus    gbaStatus         = V1_2::UimGbaStatus::UIM_GBA_GENERIC_FAILURE;
      std::vector<uint8_t>  impi              = {};
      bool                  is_impi_encrypted = false;

      if (solicitedMsg && rsp_ptr &&
          ret_status == Message::Callback::Status::SUCCESS)
      {
        if(rsp_ptr->err == RIL_UIM_E_SUCCESS)
        {
          gbaStatus = V1_2::UimGbaStatus::UIM_GBA_SUCCESS;
        }
        is_impi_encrypted = rsp_ptr->is_impi_encrypted;
        impi              = rsp_ptr->impi;
      }

      uimGetImpiResponse(token, gbaStatus, is_impi_encrypted, impi);
    }));
    msg->setCallback(&cb);
    msg->dispatch();
  }
  else
  {
    V1_2::UimGbaStatus    gbaStatus         = V1_2::UimGbaStatus::UIM_GBA_GENERIC_FAILURE;
    std::vector<uint8_t>  impi              = {};
    bool                  is_impi_encrypted = false;

    uimGetImpiResponse(token, gbaStatus, is_impi_encrypted, impi);
  }

  return Void();

}/* uimGetImpi */


/*===========================================================================

FUNCTION:  UimServiceModule::UimServiceModule

===========================================================================*/
UimServiceModule::UimServiceModule()
{
  mName  = "UimServiceModule";
  mLooper = std::unique_ptr<ModuleLooper>(new ModuleLooper);

  using std::placeholders::_1;
  mMessageHandler = {
      HANDLER(QcrilInitMessage, UimServiceModule::handleQcrilInitMessage),
      HANDLER(UimGetRilPropertySyncMsg, UimServiceModule::handleQcrilUimGetProperty),
      HANDLER(UimSetRilPropertySyncMsg, UimServiceModule::handleQcrilUimSetProperty)
  };
} /* UimServiceModule::UimServiceModule */


/*===========================================================================

FUNCTION:  UimServiceModule::~UimServiceModule

===========================================================================*/
UimServiceModule::~UimServiceModule()
{
  mLooper = nullptr;
} /* UimServiceModule::~UimServiceModule */


/*===========================================================================

FUNCTION:  UimServiceModule::init

===========================================================================*/
void UimServiceModule::init()
{
  Module::init();
} /* UimServiceModule::init */


/*===========================================================================

FUNCTION:  UimServiceModule::handleQcrilInitMessage

===========================================================================*/
void UimServiceModule::handleQcrilInitMessage
(
  std::shared_ptr<QcrilInitMessage> msg
)
{
  android::status_t          ret = android::OK;

  (void)msg;

  uimService = new UimImpl{};

  if (uimService != NULL)
  {
    ret = uimService->registerAsService("Uim" + std::to_string(qmi_ril_get_process_instance_id()));
  }

  QCRIL_LOG_DEBUG("uim_service status=%d", ret);
} /* UimServiceModule::handleQcrilInitMessage */


/*===========================================================================

FUNCTION:  UimServiceModule::handleQcrilUimGetProperty

===========================================================================*/
void UimServiceModule::handleQcrilUimGetProperty
(
  std::shared_ptr<UimGetRilPropertySyncMsg> req_ptr
)
{
  auto rsp_ptr = std::make_shared<std::string>();
  char prop_value[PROPERTY_VALUE_MAX] = {};

  if (req_ptr != NULL)
  {
    property_get(req_ptr->get_property().c_str(), prop_value, "");

    QCRIL_LOG_INFO("%s - %s", req_ptr->get_property().c_str(), prop_value);

    if (rsp_ptr != NULL)
    {
      rsp_ptr->assign(prop_value);
    }
    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, rsp_ptr);
  }
} /* UimServiceModule::handleQcrilUimGetProperty */


/*===========================================================================

FUNCTION:  UimServiceModule::handleQcrilUimSetProperty

===========================================================================*/
void UimServiceModule::handleQcrilUimSetProperty
(
  std::shared_ptr<UimSetRilPropertySyncMsg> req_ptr
)
{
  if (req_ptr != NULL)
  {
    property_set(req_ptr->get_property().c_str(), req_ptr->get_value().c_str());

    req_ptr->sendResponse(req_ptr, Message::Callback::Status::SUCCESS, NULL);
  }
} /* UimServiceModule::handleQcrilUimSetProperty */

#endif
