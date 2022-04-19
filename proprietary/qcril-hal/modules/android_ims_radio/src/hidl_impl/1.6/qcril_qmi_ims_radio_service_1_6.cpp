/******************************************************************************
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/
#define TAG "RILQ"

#include <interfaces/voice/QcRilRequestGetCallBarringMessage.h>
#include <interfaces/voice/QcRilRequestSetSupsServiceMessage.h>
#include <interfaces/voice/QcRilRequestQueryColpMessage.h>
#include <modules/android_ims_radio/hidl_impl/base/qcril_qmi_ims_radio_utils.h>
#include <modules/android_ims_radio/hidl_impl/1.5/qcril_qmi_ims_radio_service_1_5.h>
#include <modules/android_ims_radio/hidl_impl/1.6/qcril_qmi_ims_radio_service_1_6.h>
#include <modules/android_ims_radio/hidl_impl/1.6/qcril_qmi_ims_radio_utils_1_6.h>
#include <modules/voice/qcril_qmi_voice.h>
#include <modules/ims/ImsModule.h>
#include <string.h>

extern "C" {
#include "qcril_log.h"
#include "qcril_reqlist.h"
}

using ::android::hardware::Void;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_6 {
namespace implementation {

void ImsRadioImpl_1_6::setCallback_nolock(
    const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadioResponse>
        &imsRadioResponse,
    const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadioIndication>
        &imsRadioIndication) {
  QCRIL_LOG_DEBUG("ImsRadioImpl_1_6::setCallback_nolock");

  mImsRadioIndicationCbV1_6 =
      V1_6::IImsRadioIndication::castFrom(imsRadioIndication).withDefault(nullptr);
  mImsRadioResponseCbV1_6 =
      V1_6::IImsRadioResponse::castFrom(imsRadioResponse).withDefault(nullptr);

  if (mImsRadioResponseCbV1_6 == nullptr || mImsRadioIndicationCbV1_6 == nullptr) {
    mImsRadioResponseCbV1_6 = nullptr;
    mImsRadioIndicationCbV1_6 = nullptr;
  }

  if (mBaseImsRadioImpl) {
    return mBaseImsRadioImpl->setCallback_nolock(imsRadioResponse, imsRadioIndication);
  }
}
/*
 *   @brief
 *   Registers the callback for IImsRadio using the IImsRadioCallback object
 *   being passed in by the client as a parameter
 *
 */
::android::hardware::Return<void> ImsRadioImpl_1_6::setCallback(
    const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadioResponse>
        &imsRadioResponse,
    const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadioIndication>
        &imsRadioIndication) {
  QCRIL_LOG_DEBUG("ImsRadioImpl_1_6::setCallback");
  {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
    if (mImsRadioIndicationCbV1_6 != nullptr) {
      mImsRadioIndicationCbV1_6->unlinkToDeath(this);
    }
    setCallback_nolock(imsRadioResponse, imsRadioIndication);
    if (mImsRadioIndicationCbV1_6 != nullptr) {
      mImsRadioIndicationCbV1_6->linkToDeath(this, 0);
    }
  }
  notifyImsClientConnected();

  return Void();
}

sp<V1_6::IImsRadioIndication> ImsRadioImpl_1_6::getIndicationCallbackV1_6() {
  std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mImsRadioIndicationCbV1_6;
}

sp<V1_6::IImsRadioResponse> ImsRadioImpl_1_6::getResponseCallbackV1_6() {
  std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mImsRadioResponseCbV1_6;
}

void ImsRadioImpl_1_6::serviceDied(uint64_t,
                                   const ::android::wp<::android::hidl::base::V1_0::IBase> &) {
  QCRIL_LOG_DEBUG("ImsRadioImpl_1_6::serviceDied: Client died. Cleaning up callbacks");
  clearCallbacks();
}

ImsRadioImpl_1_6::ImsRadioImpl_1_6(qcril_instance_id_e_type instance) : ImsRadioImplBase(instance) {
  mBaseImsRadioImpl = new V1_5::implementation::ImsRadioImpl_1_5(instance);
}

ImsRadioImpl_1_6::~ImsRadioImpl_1_6() {}

void ImsRadioImpl_1_6::clearCallbacks() {
  QCRIL_LOG_ERROR("enter");
  {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
    mImsRadioResponseCbV1_6 = nullptr;
    mImsRadioIndicationCbV1_6 = nullptr;
    if (mBaseImsRadioImpl) {
      mBaseImsRadioImpl->clearCallbacks();
    }
  }
  QCRIL_LOG_ERROR("exit");
}

void ImsRadioImpl_1_6::registerService() {
  std::string serviceName = "imsradio" + std::to_string(getInstanceId());
  android::status_t ret = registerAsService(serviceName);
  QCRIL_LOG_INFO("registerService: starting ImsRadioImpl_1_6 %s status %d", serviceName.c_str(),
                 ret);
}

void ImsRadioImpl_1_6::notifyOnRTTMessage(
    std::shared_ptr<QcRilUnsolImsRttMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnRTTMessage(msg);
  }
}

void ImsRadioImpl_1_6::notifyOnVowifiCallQuality(
    std::shared_ptr<QcRilUnsolImsVowifiCallQuality> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnVowifiCallQuality(msg);
  }
}

void ImsRadioImpl_1_6::notifyOnVopsChanged(
    std::shared_ptr<QcRilUnsolImsVopsIndication> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnVopsChanged(msg);
  }
}

void ImsRadioImpl_1_6::notifyOnSubConfigChanged(
    std::shared_ptr<QcRilUnsolImsSubConfigIndication> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnSubConfigChanged(msg);
  }
}

void ImsRadioImpl_1_6::notifyOnGeoLocationChange(
    std::shared_ptr<QcRilUnsolImsGeoLocationInfo> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnGeoLocationChange(msg);
  }
}

void ImsRadioImpl_1_6::notifyOnServiceStatusChange(
    std::shared_ptr<QcRilUnsolImsSrvStatusIndication> msg) {
  if (!msg) {
    QCRIL_LOG_ERROR("msg is nullptr");
    return;
  }
  sp<V1_6::IImsRadioIndication> indCb = getIndicationCallbackV1_6();
  if (!indCb) {
    QCRIL_LOG_DEBUG("fallback");
    if (mBaseImsRadioImpl) {
      mBaseImsRadioImpl->notifyOnServiceStatusChange(msg);
    }
    return;
  }
  std::shared_ptr<qcril::interfaces::ServiceStatusInfoList> data = msg->getServiceStatus();
  hidl_vec<ServiceStatusInfo> srvStatusList;
  if (data != nullptr) {
    V1_6::utils::convertServiceStatusInfoList(srvStatusList, *data);
    imsRadiolog("<", "onServiceStatusChanged_1_6: srvStatusList=" + toString(srvStatusList));
    Return<void> ret = indCb->onServiceStatusChanged_1_6(srvStatusList);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
  else {
    QCRIL_LOG_ERROR("data is nullptr");
  }
}

void ImsRadioImpl_1_6::notifyOnSsacInfoChange(
    std::shared_ptr<QcRilUnsolImsSsacInfoIndication> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnSsacInfoChange(msg);
  }
}

void ImsRadioImpl_1_6::notifyOnRegBlockStatusChange(
    std::shared_ptr<QcRilUnsolImsRegBlockStatusMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnRegBlockStatusChange(msg);
  }
}

void ImsRadioImpl_1_6::notifyOnRing(std::shared_ptr<QcRilUnsolCallRingingMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnRing(msg);
  }
}
void ImsRadioImpl_1_6::notifyOnRingbackTone(std::shared_ptr<QcRilUnsolRingbackToneMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnRingbackTone(msg);
  }
}

void ImsRadioImpl_1_6::notifyOnModifyCall(std::shared_ptr<QcRilUnsolImsModifyCallMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnModifyCall(msg);
  }
}

void ImsRadioImpl_1_6::notifyOnTtyNotification(
    std::shared_ptr<QcRilUnsolImsTtyNotificationMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnTtyNotification(msg);
  }
}
void ImsRadioImpl_1_6::notifyOnRefreshConferenceInfo(
    std::shared_ptr<QcRilUnsolImsConferenceInfoMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnRefreshConferenceInfo(msg);
  }
}
void ImsRadioImpl_1_6::notifyOnRefreshViceInfo(std::shared_ptr<QcRilUnsolImsViceInfoMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnRefreshViceInfo(msg);
  }
}
void ImsRadioImpl_1_6::notifyOnSuppServiceNotification(
    std::shared_ptr<QcRilUnsolSuppSvcNotificationMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnSuppServiceNotification(msg);
  }
}
void ImsRadioImpl_1_6::notifyOnParticipantStatusInfo(
    std::shared_ptr<QcRilUnsolConfParticipantStatusInfoMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnParticipantStatusInfo(msg);
  }
}
void ImsRadioImpl_1_6::notifyOnSupplementaryServiceIndication(
      std::shared_ptr<QcRilUnsolSupplementaryServiceMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnSupplementaryServiceIndication(msg);
  }
}

void ImsRadioImpl_1_6::notifyIncomingImsSms(std::shared_ptr<RilUnsolIncomingImsSMSMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyIncomingImsSms(msg);
  }
}

void ImsRadioImpl_1_6::notifyNewImsSmsStatusReport(
      std::shared_ptr<RilUnsolNewImsSmsStatusReportMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyNewImsSmsStatusReport(msg);
  }
}

void ImsRadioImpl_1_6::notifyOnCallStateChanged(
    std::shared_ptr<QcRilUnsolImsCallStatusMessage> msg) {
  if (!msg) {
    QCRIL_LOG_ERROR("msg is nullptr");
    return;
  }
  sp<V1_6::IImsRadioIndication> indCb = getIndicationCallbackV1_6();
  if (!indCb) {
    QCRIL_LOG_DEBUG("fallback");
    if (mBaseImsRadioImpl) {
      mBaseImsRadioImpl->notifyOnCallStateChanged(msg);
    }
    return;
  }
  std::vector<qcril::interfaces::CallInfo> callInfo = msg->getCallInfo();
  if (callInfo.empty()) {
    QCRIL_LOG_ERROR("empty callInfo");
    return;
  }
  hidl_vec<V1_6::CallInfo> callListV1_6 = {};
  bool result = V1_6::utils::convertCallInfoList(callListV1_6, callInfo);
  if (!result) {
    QCRIL_LOG_ERROR("CallInfo convertion failed");
    return;
  }
  imsRadiolog("<", "onCallStateChanged_1_6: callList = " + toString(callListV1_6));
  Return<void> ret = indCb->onCallStateChanged_1_6(callListV1_6);
  if (!ret.isOk()) {
    QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
  }
}

void ImsRadioImpl_1_6::notifyOnIncomingCallAutoRejected(
    std::shared_ptr<QcRilUnsolAutoCallRejectionMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnIncomingCallAutoRejected(msg);
  }
}

void ImsRadioImpl_1_6::notifyOnPendingMultiIdentityStatus(
    std::shared_ptr<QcRilUnsolImsPendingMultiLineStatus> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnPendingMultiIdentityStatus(msg);
  }
}

void ImsRadioImpl_1_6::notifyOnMultiIdentityLineStatus(
    std::shared_ptr<QcRilUnsolImsMultiIdentityStatusMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnMultiIdentityLineStatus(msg);
  }
}

void ImsRadioImpl_1_6::notifyOnVoiceInfoStatusChange(
    std::shared_ptr<QcRilUnsolImsVoiceInfo> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnVoiceInfoStatusChange(msg);
  }
}

void ImsRadioImpl_1_6::notifyOnExitEcbmIndication(
    std::shared_ptr<QcRilUnsolImsExitEcbmIndication> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnExitEcbmIndication(msg);
  }
}

void ImsRadioImpl_1_6::notifyOnEnterEcbmIndication(
    std::shared_ptr<QcRilUnsolImsEnterEcbmIndication> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnEnterEcbmIndication(msg);
  }
}

void ImsRadioImpl_1_6::notifyOnRadioStateChanged(
    std::shared_ptr<QcRilUnsolImsRadioStateIndication> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnRadioStateChanged(msg);
  }
}

void ImsRadioImpl_1_6::notifyOnMessageWaiting(
      std::shared_ptr<QcRilUnsolMessageWaitingInfoMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnMessageWaiting(msg);
  }
}

void ImsRadioImpl_1_6::notifyOnModemSupportsWfcRoamingModeConfiguration(
    std::shared_ptr<QcRilUnsolImsWfcRoamingConfigIndication> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnModemSupportsWfcRoamingModeConfiguration(msg);
  }
}
void ImsRadioImpl_1_6::notifyOnUssdMessageFailed(std::shared_ptr<QcRilUnsolOnUssdMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnUssdMessageFailed(msg);
  }
}

Return<void> ImsRadioImpl_1_6::setConfig_1_6(int32_t token, const V1_6::ConfigInfo& inConfig) {
  imsRadiolog(">", "setConfig_1_6: token = " + std::to_string(token) + toString(inConfig));
  bool sendFailure = false;
  auto ctx = getContext(token);
  do {
    std::shared_ptr<QcRilRequestImsSetConfigMessage> msg =
      std::make_shared<QcRilRequestImsSetConfigMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    V1_6::utils::readConfigInfo(inConfig, msg->getConfigInfo());
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                  std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = V1_0::utils::ImsRadioErrorCode::GENERIC_FAILURE;
          (void)msg;
          std::shared_ptr<qcril::interfaces::ConfigInfo> respData = nullptr;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = V1_0::utils::qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
            respData = std::static_pointer_cast<qcril::interfaces::ConfigInfo>(resp->data);
          }
          sendSetConfigMessageResponse(token, errorCode, respData);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendSetConfigMessageResponse(token, RIL_E_GENERIC_FAILURE, nullptr);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

void ImsRadioImpl_1_6::sendSetConfigMessageResponse(uint32_t token, uint32_t errorCode,
  std::shared_ptr<qcril::interfaces::ConfigInfo> data) {
  ConfigInfo config = ConfigInfo{ .item=V1_6::ConfigItem::CONFIG_ITEM_INVALID,
                                  .intValue=INT32_MAX,
                                  .errorCause=V1_0::ConfigFailureCause::CONFIG_FAILURE_INVALID };
  if (data) {
    V1_6::utils::convertConfigInfo(config, *(data));
  }
  sp<V1_6::IImsRadioResponse> respCb = getResponseCallbackV1_6();
  if (respCb != nullptr) {
    imsRadiolog("<", "setConfigResponse_1_6: token = " + std::to_string(token) + " errorCode = " +
            std::to_string(errorCode) + " config = " + toString(config));
    Return<void> retStatus = respCb->setConfigResponse_1_6(token, errorCode, config);
    if (!retStatus.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s",
          retStatus.description().c_str());
    }
  } else {
    QCRIL_LOG_ERROR("V1_6 respCb is null");
  }
}

Return<void> ImsRadioImpl_1_6::getImsRegistrationState(int32_t token) {
  imsRadiolog(">", "getImsRegistrationState: token = " + std::to_string(token));
  QCRIL_LOG_FUNC_ENTRY();
  if (getResponseCallbackV1_6() == nullptr) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->getImsRegistrationState(token);
    }
  }
  bool sendFailure = false;
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  do {
    auto msg = std::make_shared<QcRilRequestImsGetRegStateMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
      [this, token](std::shared_ptr<Message>/*msg*/, Message::Callback::Status status,
                    std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
        uint32_t errorCode = RIL_E_GENERIC_FAILURE;
        std::shared_ptr<qcril::interfaces::Registration> respData = nullptr;
        if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
          errorCode = V1_0::utils::qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
          respData = std::static_pointer_cast<qcril::interfaces::Registration>(resp->data);
        }
        sendGetRegistrationResponse(token, errorCode, respData);
      });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);

  if (sendFailure) {
    sendGetRegistrationResponse(token, RIL_E_GENERIC_FAILURE, nullptr);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

void ImsRadioImpl_1_6::sendGetRegistrationResponse(
    int32_t token, uint32_t errorCode, std::shared_ptr<qcril::interfaces::Registration> data) {

  V1_6::RegistrationInfo reg = {.state = V1_0::RegState::INVALID, .errorCode=INT32_MAX,
                          .radioTech = V1_6::RadioTechType::RADIO_TECH_INVALID};
  if (data != nullptr) {
    V1_6::utils::convertRegistrationInfo(reg, *data);
  } else {
    QCRIL_LOG_ERROR("data is nullptr");
  }
  sp<V1_6::IImsRadioResponse> respCb = getResponseCallbackV1_6();
  if (respCb != nullptr) {
    imsRadiolog("<", "getRegistrationResponse_1_6: token = " + std::to_string(token) +
                         " errorCode = " + std::to_string(errorCode) + " reg = " + toString(reg));
    Return<void> ret = respCb->getRegistrationResponse_1_6(token, errorCode, reg);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

Return<void> ImsRadioImpl_1_6::queryServiceStatus(int32_t token) {
  imsRadiolog(">", "queryServiceStatus: token = " + std::to_string(token));
  QCRIL_LOG_FUNC_ENTRY();
  if (getResponseCallbackV1_6() == nullptr) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->queryServiceStatus(token);
    }
  }
  bool sendFailure = false;
  auto ctx = getContext(token);
  do {
    std::shared_ptr<QcRilRequestImsQueryServiceStatusMessage> msg =
        std::make_shared<QcRilRequestImsQueryServiceStatusMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message>/*msg*/, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          std::shared_ptr<qcril::interfaces::ServiceStatusInfoList> respData = nullptr;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = V1_0::utils::qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
            respData =
                std::static_pointer_cast<qcril::interfaces::ServiceStatusInfoList>(resp->data);
          }
          sendQueryServiceStatusResponse(token, errorCode, respData);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);

  if (sendFailure) {
    sendQueryServiceStatusResponse(token, RIL_E_GENERIC_FAILURE, nullptr);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

void ImsRadioImpl_1_6::sendQueryServiceStatusResponse(
    int32_t token, uint32_t errorCode,
    std::shared_ptr<qcril::interfaces::ServiceStatusInfoList> data) {
  hidl_vec<V1_6::ServiceStatusInfo> srvStatusList;
  if (data != nullptr) {
    V1_6::utils::convertServiceStatusInfoList(srvStatusList, *data);
  } else {
    QCRIL_LOG_ERROR("data is nullptr");
  }
  sp<V1_6::IImsRadioResponse> respCb = getResponseCallbackV1_6();
  if (respCb != nullptr) {
    imsRadiolog("<", "queryServiceStatusResponseV1_6: token = " + std::to_string(token) +
                         " errorCode = " + std::to_string(errorCode) +
                         " srvStatusList = " + toString(srvStatusList));
    Return<void> ret = respCb->queryServiceStatusResponse_1_6(token, errorCode, srvStatusList);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

void ImsRadioImpl_1_6::notifyOnRegistrationChanged(std::shared_ptr<QcRilUnsolImsRegStateMessage> msg) {
  if (msg == nullptr) {
    QCRIL_LOG_ERROR("data is nullptr");
    return;
  }
  sp<V1_6::IImsRadioIndication> indCb = getIndicationCallbackV1_6();
  if (!indCb) {
    QCRIL_LOG_DEBUG("fallback");
    if (mBaseImsRadioImpl) {
      mBaseImsRadioImpl->notifyOnRegistrationChanged(msg);
    }
    return;
  }
  std::shared_ptr<qcril::interfaces::Registration> reg = msg->getRegistration();
  if (reg != nullptr) {
    V1_6::RegistrationInfo regInfo = {.state = V1_0::RegState::INVALID,
                                .radioTech = V1_6::RadioTechType::RADIO_TECH_INVALID};
    V1_6::utils::convertRegistrationInfo(regInfo, *reg);
    if (indCb != nullptr) {
      imsRadiolog("<", "onRegistrationChanged_1_6: regInfo = " + toString(regInfo));
      Return<void> ret = indCb->onRegistrationChanged_1_6(regInfo);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }
}

void ImsRadioImpl_1_6::notifyOnHandover(std::shared_ptr<QcRilUnsolImsHandoverMessage> msg) {
  if (msg == nullptr) {
    QCRIL_LOG_ERROR("data is nullptr");
    return;
  }
  sp<V1_6::IImsRadioIndication> indCb = getIndicationCallbackV1_6();
  if (!indCb) {
    QCRIL_LOG_DEBUG("fallback");
    if (mBaseImsRadioImpl) {
      mBaseImsRadioImpl->notifyOnHandover(msg);
    }
    return;
  }
  V1_6::HandoverInfo handoverInfo = {.type = V1_0::HandoverType::INVALID,
                               .srcTech = V1_6::RadioTechType::RADIO_TECH_INVALID,
                               .targetTech = V1_6::RadioTechType::RADIO_TECH_INVALID,
                               .hasHoExtra = false};
  if (msg->hasHandoverType()) {
    handoverInfo.type = V1_0::utils::convertHandoverType(msg->getHandoverType());
  }
  if (msg->hasSourceTech()) {
    handoverInfo.srcTech = V1_6::utils::convertRadioTechType(msg->getSourceTech());
  }
  if (msg->hasTargetTech()) {
    handoverInfo.targetTech = V1_6::utils::convertRadioTechType(msg->getTargetTech());
  }
  if (msg->hasCauseCode() && !msg->getCauseCode().empty()) {
    /* Error is reported when the handover is NOT_TRIGGERED while the device is on active
     * Wifi call and the wifi Rssi is nearing threshold roveout (-85dbm) and there is
     * no qualified LTE network to handover to. Modem sends "CD-04:No Available qualified
     * mobile network". Here it is decoded and sent as errorcode(CD-04) and errormessage
     * to telephony.
     */
    if (msg->hasHandoverType() &&
        (msg->getHandoverType() == qcril::interfaces::HandoverType::NOT_TRIGGERED) &&
        (msg->getCauseCode().find(
            QcRilUnsolImsHandoverMessage::WLAN_HANDOVER_NO_LTE_FAILURE_CODE_STRING) == 0)) {
      handoverInfo.errorCode =
              QcRilUnsolImsHandoverMessage::WLAN_HANDOVER_NO_LTE_FAILURE_CODE_STRING;

      std::string causeCode = msg->getCauseCode();
      std::string delim = ":";
      size_t pos = causeCode.find(delim);
      if (pos != std::string::npos) {
        std::string errorCode = causeCode.substr(0, pos);
        std::string errorMsg = causeCode.substr(pos + delim.length());
        //remove whitespaces at the beginning
        if (!errorMsg.empty()) {
          size_t start = errorMsg.find_first_not_of(" ");
          errorMsg = (start != std::string::npos) ? errorMsg.substr(start) : "";
          handoverInfo.errorMessage = errorMsg;
        }
      }
    } else {
      handoverInfo.hasHoExtra = true;
      handoverInfo.hoExtra.type = V1_0::ExtraType::LTE_TO_IWLAN_HO_FAIL;
      uint8_t *buffer = new uint8_t[msg->getCauseCode().length()];
      if (buffer == nullptr) {
        QCRIL_LOG_ERROR("buffer is null. returning");
        return;
      }
      for (size_t idx = 0; idx < msg->getCauseCode().length(); idx++) {
        buffer[idx] = msg->getCauseCode()[idx];
      }
      handoverInfo.hoExtra.extraInfo.setToExternal(buffer, msg->getCauseCode().length(), true);
    }
  }
  imsRadiolog("<", "onHandover_1_6: handoverInfo = " + toString(handoverInfo));
  Return<void> ret = indCb->onHandover_1_6(handoverInfo);
  if (!ret.isOk()) {
    QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
  }
}

Return<void> ImsRadioImpl_1_6::setServiceStatus_1_6(int32_t token,
        const hidl_vec<ServiceStatusInfo>& srvStatusInfoList) {
  imsRadiolog(">", "setServiceStatus_1_6: token = " + std::to_string(token) +
                       " srvStatusInfoList=" + toString(srvStatusInfoList));
  bool sendFailure = false;
  auto ctx = getContext(token);

  do {
    auto msg = std::make_shared<QcRilRequestImsSetServiceStatusMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    for (const auto &srvStatusInfo : srvStatusInfoList) {
      auto networkMode = V1_6::RadioTechType::RADIO_TECH_INVALID;
      auto status = V1_0::StatusType::STATUS_INVALID;
      if (srvStatusInfo.accTechStatus.size()) {
        networkMode = srvStatusInfo.accTechStatus[0].networkMode;
        status = srvStatusInfo.accTechStatus[0].status;
      }

      if ((srvStatusInfo.callType == V1_0::CallType::CALL_TYPE_VOICE) &&
          (networkMode == V1_6::RadioTechType::RADIO_TECH_LTE)) {
        msg->setVolteEnabled(status != V1_0::StatusType::STATUS_DISABLED);
      }
      if ((srvStatusInfo.callType == V1_0::CallType::CALL_TYPE_VOICE) &&
          ((networkMode == V1_6::RadioTechType::RADIO_TECH_IWLAN) ||
           (networkMode == V1_6::RadioTechType::RADIO_TECH_WIFI))) {
        msg->setWifiCallingEnabled(status != V1_0::StatusType::STATUS_DISABLED);
      }
      if (srvStatusInfo.callType == V1_0::CallType::CALL_TYPE_VT) {
        msg->setVideoTelephonyEnabled(status != V1_0::StatusType::STATUS_DISABLED);
      }
      if (srvStatusInfo.callType == V1_0::CallType::CALL_TYPE_UT) {
        msg->setUTEnabled(status != V1_0::StatusType::STATUS_DISABLED);
      }
    }

    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
      [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                    std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
        uint32_t errorCode = RIL_E_GENERIC_FAILURE;
        (void)msg;
        if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
          errorCode = V1_0::utils::qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
        }
        sendSetServiceStatusMessageResponse(token, errorCode, nullptr);
      });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);

  if (sendFailure) {
    sendSetServiceStatusMessageResponse(token, RIL_E_GENERIC_FAILURE, nullptr);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

void ImsRadioImpl_1_6::sendSetServiceStatusMessageResponse(uint32_t token, uint32_t errorCode,
  std::shared_ptr<qcril::interfaces::BasePayload> data) {
  (void)data;
  sp<V1_6::IImsRadioResponse> respCb = getResponseCallbackV1_6();
  if (respCb != nullptr) {
    imsRadiolog("<", "setServiceStatusResponse: token = " + std::to_string(token) +
                         " errorCode = " + std::to_string(errorCode));
    Return<void> retStatus = respCb->setServiceStatusResponse(token, errorCode);
    if (!retStatus.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s",
             retStatus.description().c_str());
    }
  }
}

Return<void> ImsRadioImpl_1_6::suppServiceStatus_1_6(int32_t token, int32_t operationType,
        V1_0::FacilityType facilityType, const V1_0::CbNumListInfo &cbNumListInfo,
        const hidl_string &password) {
  imsRadiolog(">", "suppServiceStatus_1_6: token = " + std::to_string(token) + " operationType = " +
              std::to_string(operationType) + " facilityType = " + toString(facilityType) +
              " cbNumListInfo = " + toString(cbNumListInfo));
  std::shared_ptr<QcRilRequestMessage> msg = nullptr;
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    switch (operationType) {
      case 4:  // REGISTER
      case 5:  // ERASURE
        if (facilityType != V1_0::FacilityType::FACILITY_BS_MT) {
          QCRIL_LOG_INFO("Unsupported facility type %d for reg or erase", facilityType);
          break;
        }
        // fallthrough; use QcRilRequestSetSupsServiceMessage for REGISTER/ERASURE of BS_MT
        [[fallthrough]];
      case 1:  // ACTIVATE
      case 2:  // DEACTIVATE
      {
        auto setSupsMsg = std::make_shared<QcRilRequestSetSupsServiceMessage>(ctx);
        if (setSupsMsg == nullptr) {
          QCRIL_LOG_ERROR("setSupsMsg is nullptr");
          sendFailure = true;
          break;
        }
        // Set parameters
        if (operationType != INT32_MAX) {
          setSupsMsg->setOperationType(operationType);
        }
        if (facilityType != V1_0::FacilityType::FACILITY_INVALID) {
          setSupsMsg->setFacilityType(V1_0::utils::convertFacilityType(facilityType));
        }
        if (cbNumListInfo.serviceClass != INT32_MAX) {
          setSupsMsg->setServiceClass(cbNumListInfo.serviceClass);
        }
        if (facilityType == V1_0::FacilityType::FACILITY_BS_MT) {
          if (cbNumListInfo.cbNumInfo.size() > 0) {
            std::vector<std::string> cbNumList;
            for (uint32_t i = 0; i < cbNumListInfo.cbNumInfo.size(); ++i) {
              std::string num = cbNumListInfo.cbNumInfo[i].number.c_str();
              cbNumList.push_back(num);
            }
            setSupsMsg->setCallBarringNumberList(cbNumList);
          }
        }
        if (password.size()) {
          setSupsMsg->setPassword(password.c_str());
        }
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, token](std::shared_ptr<Message> setSupsMsg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
              uint32_t errorCode = RIL_E_GENERIC_FAILURE;
              std::shared_ptr<qcril::interfaces::SipErrorInfo> errorDetails = nullptr;
              (void)setSupsMsg;
              if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
                errorCode = V1_0::utils::qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
                errorDetails =
                    std::static_pointer_cast<qcril::interfaces::SipErrorInfo>(resp->data);
              }
              auto data = std::make_shared<qcril::interfaces::SuppServiceStatusInfo>();
              if (data) {
                data->setErrorDetails(errorDetails);
              }
              sendSuppServiceStatusResponse(token, errorCode, data);
            });
        setSupsMsg->setCallback(&cb);
        msg = setSupsMsg;
      }
      break;
      case 3:  // QUERY
        if (facilityType == V1_0::FacilityType::FACILITY_COLP) {
          // QcRilRequestQueryColpMessage
          auto queryColp = std::make_shared<QcRilRequestQueryColpMessage>(ctx);
          if (queryColp == nullptr) {
            QCRIL_LOG_ERROR("queryColp is nullptr");
            sendFailure = true;
            break;
          }
          GenericCallback<QcRilRequestMessageCallbackPayload> cb(
              [this, token](std::shared_ptr<Message> queryColp, Message::Callback::Status status,
                            std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
                uint32_t errorCode = RIL_E_GENERIC_FAILURE;
                std::shared_ptr<qcril::interfaces::SuppServiceStatusInfo> data = nullptr;
                (void)queryColp;
                if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
                  errorCode = V1_0::utils::qcril_qmi_ims_map_ril_error_to_ims_error(
                      resp->errorCode);
                  data = std::static_pointer_cast<qcril::interfaces::SuppServiceStatusInfo>(
                      resp->data);
                }
                sendSuppServiceStatusResponse(token, errorCode, data);
              });
          queryColp->setCallback(&cb);
          msg = queryColp;
        } else if (facilityType != V1_0::FacilityType::FACILITY_CLIP) {
          // QcRilRequestGetCallBarringMessage
          auto queryMsg = std::make_shared<QcRilRequestGetCallBarringMessage>(ctx);
          if (queryMsg == nullptr) {
            QCRIL_LOG_ERROR("queryMsg is nullptr");
            sendFailure = true;
            break;
          }
          // Set parameters
          if (facilityType != V1_0::FacilityType::FACILITY_INVALID) {
            queryMsg->setFacilityType(V1_0::utils::convertFacilityType(facilityType));
          }
          if (cbNumListInfo.serviceClass != INT32_MAX) {
            queryMsg->setServiceClass(cbNumListInfo.serviceClass);
          }
          GenericCallback<QcRilRequestMessageCallbackPayload> cb(
              [this, token](std::shared_ptr<Message> queryMsg, Message::Callback::Status status,
                            std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
                uint32_t errorCode = RIL_E_GENERIC_FAILURE;
                std::shared_ptr<qcril::interfaces::SuppServiceStatusInfo> data = nullptr;
                (void)queryMsg;
                if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
                  errorCode = V1_0::utils::qcril_qmi_ims_map_ril_error_to_ims_error(
                      resp->errorCode);
                  data = std::static_pointer_cast<qcril::interfaces::SuppServiceStatusInfo>(
                      resp->data);
                }
                sendSuppServiceStatusResponse(token, errorCode, data);
              });
          queryMsg->setCallback(&cb);
          msg = queryMsg;
        }
        break;
      default:
        break;
    }
    if (msg) {
      msg->setIsImsRequest(true);
      msg->dispatch();
    }
  } while (FALSE);
  if (sendFailure) {
    sendSuppServiceStatusResponse(token, RIL_E_GENERIC_FAILURE, nullptr);
  }
  return Void();
}

void ImsRadioImpl_1_6::sendSuppServiceStatusResponse(int32_t token, uint32_t errorCode,
    const std::shared_ptr<qcril::interfaces::SuppServiceStatusInfo> data) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->sendSuppServiceStatusResponse(token, errorCode, data);
  }
}
}  // namespace implementation
}  // namespace V1_6
}  // namespace ims
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
