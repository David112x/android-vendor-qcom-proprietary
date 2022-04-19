/******************************************************************************
  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/
#define TAG "RILQ"

#include <interfaces/voice/QcRilRequestImsDialMessage.h>
#include <interfaces/voice/QcRilRequestSetCallForwardMessage.h>
#include <modules/android_ims_radio/hidl_impl/1.5/qcril_qmi_ims_radio_service_1_5.h>
#include <modules/android_ims_radio/hidl_impl/1.5/qcril_qmi_ims_radio_utils_1_5.h>
#include <modules/voice/qcril_qmi_voice.h>
#include <modules/ims/ImsModule.h>
#include <modules/sms/RilRequestImsSendSmsMessage.h>
#include <modules/sms/RilRequestAckImsSmsMessage.h>
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
namespace V1_5 {
namespace implementation {

void ImsRadioImpl_1_5::setCallback_nolock(
    const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadioResponse>
        &imsRadioResponse,
    const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadioIndication>
        &imsRadioIndication) {
  QCRIL_LOG_DEBUG("ImsRadioImpl_1_5::setCallback_nolock");

  mImsRadioIndicationCbV1_5 =
      V1_5::IImsRadioIndication::castFrom(imsRadioIndication).withDefault(nullptr);
  mImsRadioResponseCbV1_5 =
      V1_5::IImsRadioResponse::castFrom(imsRadioResponse).withDefault(nullptr);

  if (mImsRadioResponseCbV1_5 == nullptr || mImsRadioIndicationCbV1_5 == nullptr) {
    mImsRadioResponseCbV1_5 = nullptr;
    mImsRadioIndicationCbV1_5 = nullptr;
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
::android::hardware::Return<void> ImsRadioImpl_1_5::setCallback(
    const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadioResponse>
        &imsRadioResponse,
    const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadioIndication>
        &imsRadioIndication) {
  QCRIL_LOG_DEBUG("ImsRadioImpl_1_5::setCallback");
  {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
    if (mImsRadioIndicationCbV1_5 != nullptr) {
      mImsRadioIndicationCbV1_5->unlinkToDeath(this);
    }
    setCallback_nolock(imsRadioResponse, imsRadioIndication);
    if (mImsRadioIndicationCbV1_5 != nullptr) {
      mImsRadioIndicationCbV1_5->linkToDeath(this, 0);
    }
  }
  notifyImsClientConnected();

  return Void();
}

sp<V1_5::IImsRadioResponse> ImsRadioImpl_1_5::getResponseCallbackV1_5() {
  std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mImsRadioResponseCbV1_5;
}

sp<V1_5::IImsRadioIndication> ImsRadioImpl_1_5::getIndicationCallbackV1_5() {
  std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mImsRadioIndicationCbV1_5;
}

void ImsRadioImpl_1_5::serviceDied(uint64_t,
                                   const ::android::wp<::android::hidl::base::V1_0::IBase> &) {
  QCRIL_LOG_DEBUG("ImsRadioImpl_1_5::serviceDied: Client died. Cleaning up callbacks");
  clearCallbacks();
}

ImsRadioImpl_1_5::ImsRadioImpl_1_5(qcril_instance_id_e_type instance) : ImsRadioImplBase(instance) {
  mBaseImsRadioImpl = new V1_4::implementation::ImsRadioImpl_1_4(instance);
}

ImsRadioImpl_1_5::~ImsRadioImpl_1_5() {}

void ImsRadioImpl_1_5::clearCallbacks() {
  QCRIL_LOG_ERROR("enter");
  {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
    mImsRadioResponseCbV1_5 = nullptr;
    mImsRadioIndicationCbV1_5 = nullptr;
    if (mBaseImsRadioImpl) {
      mBaseImsRadioImpl->clearCallbacks();
    }
  }
  QCRIL_LOG_ERROR("exit");
}

void ImsRadioImpl_1_5::registerService() {
  std::string serviceName = "imsradio" + std::to_string(getInstanceId());
  android::status_t ret = registerAsService(serviceName);
  QCRIL_LOG_INFO("registerService: starting ImsRadioImpl_1_5 %s status %d", serviceName.c_str(),
                 ret);
}

/*
 *  @brief
 *  Server side implementtaion of the IImsradio service's emergencyDial API
 *
 */
Return<void> ImsRadioImpl_1_5::emergencyDial(int32_t token,
    const ::vendor::qti::hardware::radio::ims::V1_4::DialRequest &dialRequest,
    hidl_bitfield<V1_5::EmergencyServiceCategory> categories,
    const hidl_vec<hidl_string> &urns, V1_5::EmergencyCallRoute route,
    bool hasKnownUserIntentEmergency, bool isTesting) {
  imsRadiolog(">", "emergencyDial: token = " + std::to_string(token) + " dialRequest = " +
                       toString(dialRequest) + " categories = " + std::to_string((int)categories) +
                       " route = " + std::to_string((int)route) +
                       " hasKnownUserIntentEmergency = " +
                       (hasKnownUserIntentEmergency ? "true" : "false") +
                       " isTesting = " + (isTesting ? "true" : "false"));
  QCRIL_LOG_FUNC_ENTRY();
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);

  auto msg = std::make_shared<QcRilRequestImsDialMessage>(ctx);
  if (msg == nullptr) {
    QCRIL_LOG_ERROR("msg is nullptr");
    mBaseImsRadioImpl->sendDialResponse(token, RIL_E_GENERIC_FAILURE);
    return Void();
  }
  // Set parameters
  if (!dialRequest.address.empty()) {
    msg->setAddress(dialRequest.address.c_str());
  } else {
    // Empty dial string is allowed for conference uri call
    if (!dialRequest.hasIsConferenceUri || !dialRequest.isConferenceUri) {
      QCRIL_LOG_ERROR("msg is nullptr");
      mBaseImsRadioImpl->sendDialResponse(token, RIL_E_GENERIC_FAILURE);
      return Void();
    }
  }
  if (dialRequest.clirMode != INT32_MAX) {
    msg->setClir(dialRequest.clirMode);
  }
  if (dialRequest.hasCallDetails) {
    auto& callDetails = dialRequest.callDetails;
    msg->setCallType(V1_0::utils::convertCallType(callDetails.callType));
    msg->setCallDomain(V1_0::utils::convertCallDomain(callDetails.callDomain));
    msg->setRttMode(V1_0::utils::convertRttMode(callDetails.rttMode));
    if (callDetails.extras.size()) {
      std::string displayText = V1_0::utils::getExtra("DisplayText", callDetails.extras);
      if (!displayText.empty()) {
        msg->setDisplayText(displayText);
      }
      std::string retryCallFailReason =
          V1_0::utils::getExtra("RetryCallFailReason", callDetails.extras);
      if (!retryCallFailReason.empty()) {
        msg->setRetryCallFailReason(V1_5::utils::convertCallFailCause(
            static_cast<V1_5::CallFailCause>(std::stoi(retryCallFailReason))));
      }
      std::string retryCallFailMode =
          V1_0::utils::getExtra("RetryCallFailRadioTech", callDetails.extras);
      if (!retryCallFailMode.empty()) {
        msg->setRetryCallFailMode(V1_0::utils::convertRadioTechType(
            static_cast<V1_0::RadioTechType>(std::stoi(retryCallFailMode))));
      }
    }
  }
  if (dialRequest.hasIsConferenceUri) {
    msg->setIsConferenceUri(dialRequest.isConferenceUri);
  }
  if (dialRequest.hasIsCallPull) {
    msg->setIsCallPull(dialRequest.isCallPull);
  }
  if (dialRequest.hasIsEncrypted) {
    msg->setIsEncrypted(dialRequest.isEncrypted);
  }
  if (!dialRequest.multiLineInfo.msisdn.empty()) {
    msg->setOriginatingNumber(dialRequest.multiLineInfo.msisdn.c_str());
    msg->setIsSecondary(
        (dialRequest.multiLineInfo.lineType == V1_4::MultiIdentityLineType::LINE_TYPE_SECONDARY));
  }
  msg->setIsEmergency(true);
  msg->setCategories(V1_5::utils::convertToRilEmergencyServiceCategories(categories));
  msg->setRouting(V1_5::utils::convertToRilEmergencyCallRoute(route));
  msg->setIsForEccTesting(isTesting);
  msg->setIsIntentionEcc(hasKnownUserIntentEmergency);

  std::vector<std::string> Urns;
  for(size_t j = 0; j < urns.size(); j++) {
    Urns.push_back(urns[j]);
  }

  GenericCallback<QcRilRequestMessageCallbackPayload> cb(
      [this, token](std::shared_ptr<Message> /*msg*/, Message::Callback::Status status,
                    std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
        uint32_t errorCode = RIL_E_GENERIC_FAILURE;
        if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
          errorCode = V1_0::utils::qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
        }
        mBaseImsRadioImpl->sendDialResponse(token, errorCode);
      });
  msg->setCallback(&cb);
  msg->dispatch();
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> ImsRadioImpl_1_5::setCallForwardStatus(
    int32_t token,
    const ::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo &callForwardInfo) {
  if (getResponseCallbackV1_5() == nullptr) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->setCallForwardStatus(token, callForwardInfo);
    }
  }

  imsRadiolog(">", "setCallForwardStatus: token = " + std::to_string(token) +
                       " callForwardInfo = " + toString(callForwardInfo));
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestSetCallForwardMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    msg->setIsImsRequest(true);
    // Set parameters
    if (callForwardInfo.status != INT32_MAX) {
      msg->setStatus(callForwardInfo.status);
    }
    if (callForwardInfo.reason != INT32_MAX) {
      msg->setReason(callForwardInfo.reason);
    }
    if (callForwardInfo.serviceClass != INT32_MAX) {
      msg->setServiceClass(callForwardInfo.serviceClass);
    }
    if (callForwardInfo.toa != INT32_MAX) {
      msg->setToa(callForwardInfo.toa);
    }
    if (!callForwardInfo.number.empty()) {
      msg->setNumber(callForwardInfo.number.c_str());
    }
    if (callForwardInfo.timeSeconds != INT32_MAX) {
      msg->setTimeSeconds(callForwardInfo.timeSeconds);
    }
    if (callForwardInfo.hasCallFwdTimerStart) {
      std::shared_ptr<qcril::interfaces::CallFwdTimerInfo> callFwdTimerStart =
          std::make_shared<qcril::interfaces::CallFwdTimerInfo>();
      V1_0::utils::convertCallFwdTimerInfo(callFwdTimerStart, callForwardInfo.callFwdTimerStart);
      msg->setCallFwdTimerStart(callFwdTimerStart);
    }

    if (callForwardInfo.hasCallFwdTimerEnd) {
      std::shared_ptr<qcril::interfaces::CallFwdTimerInfo> callFwdTimerEnd =
          std::make_shared<qcril::interfaces::CallFwdTimerInfo>();
      V1_0::utils::convertCallFwdTimerInfo(callFwdTimerEnd, callForwardInfo.callFwdTimerEnd);
      msg->setCallFwdTimerEnd(callFwdTimerEnd);
    }

    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          std::shared_ptr<qcril::interfaces::SetCallForwardRespData> data = nullptr;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = V1_0::utils::qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
            data = std::static_pointer_cast<qcril::interfaces::SetCallForwardRespData>(resp->data);
          }
          sendSetCallForwardStatusResponse(token, errorCode, data);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendSetCallForwardStatusResponse(token, RIL_E_GENERIC_FAILURE, nullptr);
  }
  return Void();
}

void ImsRadioImpl_1_5::sendSetCallForwardStatusResponse(
    int32_t token, uint32_t errorCode,
    const std::shared_ptr<qcril::interfaces::SetCallForwardRespData> data) {
  Return<void> ret = Void();
  V1_5::CallForwardStatusInfo callForwardStatus = {};

  V1_5::utils::convertCallForwardStatusInfo(callForwardStatus, data);

  sp<V1_5::IImsRadioResponse> respCb = getResponseCallbackV1_5();
  if (respCb != nullptr) {
    imsRadiolog("<", "setCallForwardStatusResponse: token = " + std::to_string(token) +
                         " errorCode = " + std::to_string(errorCode) +
                         " callForwardStatus = " + toString(callForwardStatus));
    ret = respCb->setCallForwardStatusResponse(token, errorCode, callForwardStatus);
  }
  if (!ret.isOk()) {
    QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
  }
}

void ImsRadioImpl_1_5::notifyOnRegistrationChanged(
    std::shared_ptr<QcRilUnsolImsRegStateMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnRegistrationChanged(msg);
  }
}

void ImsRadioImpl_1_5::notifyOnRTTMessage(
    std::shared_ptr<QcRilUnsolImsRttMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnRTTMessage(msg);
  }
}

void ImsRadioImpl_1_5::notifyOnVowifiCallQuality(
    std::shared_ptr<QcRilUnsolImsVowifiCallQuality> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnVowifiCallQuality(msg);
  }
}

void ImsRadioImpl_1_5::notifyOnVopsChanged(
    std::shared_ptr<QcRilUnsolImsVopsIndication> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnVopsChanged(msg);
  }
}

void ImsRadioImpl_1_5::notifyOnSubConfigChanged(
    std::shared_ptr<QcRilUnsolImsSubConfigIndication> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnSubConfigChanged(msg);
  }
}

void ImsRadioImpl_1_5::notifyOnGeoLocationChange(
    std::shared_ptr<QcRilUnsolImsGeoLocationInfo> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnGeoLocationChange(msg);
  }
}

void ImsRadioImpl_1_5::notifyOnServiceStatusChange(
    std::shared_ptr<QcRilUnsolImsSrvStatusIndication> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnServiceStatusChange(msg);
  }
}

void ImsRadioImpl_1_5::notifyOnSsacInfoChange(
    std::shared_ptr<QcRilUnsolImsSsacInfoIndication> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnSsacInfoChange(msg);
  }
}

void ImsRadioImpl_1_5::notifyOnRegBlockStatusChange(
    std::shared_ptr<QcRilUnsolImsRegBlockStatusMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnRegBlockStatusChange(msg);
  }
}

void ImsRadioImpl_1_5::notifyOnModifyCall(std::shared_ptr<QcRilUnsolImsModifyCallMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnModifyCall(msg);
  }
}
void ImsRadioImpl_1_5::notifyOnRing(std::shared_ptr<QcRilUnsolCallRingingMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnRing(msg);
  }
}
void ImsRadioImpl_1_5::notifyOnRingbackTone(std::shared_ptr<QcRilUnsolRingbackToneMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnRingbackTone(msg);
  }
}
void ImsRadioImpl_1_5::notifyOnHandover(std::shared_ptr<QcRilUnsolImsHandoverMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnHandover(msg);
  }
}
void ImsRadioImpl_1_5::notifyOnTtyNotification(
    std::shared_ptr<QcRilUnsolImsTtyNotificationMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnTtyNotification(msg);
  }
}
void ImsRadioImpl_1_5::notifyOnRefreshConferenceInfo(
    std::shared_ptr<QcRilUnsolImsConferenceInfoMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnRefreshConferenceInfo(msg);
  }
}
void ImsRadioImpl_1_5::notifyOnRefreshViceInfo(std::shared_ptr<QcRilUnsolImsViceInfoMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnRefreshViceInfo(msg);
  }
}
void ImsRadioImpl_1_5::notifyOnSuppServiceNotification(
    std::shared_ptr<QcRilUnsolSuppSvcNotificationMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnSuppServiceNotification(msg);
  }
}
void ImsRadioImpl_1_5::notifyOnParticipantStatusInfo(
    std::shared_ptr<QcRilUnsolConfParticipantStatusInfoMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnParticipantStatusInfo(msg);
  }
}
void ImsRadioImpl_1_5::notifyOnSupplementaryServiceIndication(
      std::shared_ptr<QcRilUnsolSupplementaryServiceMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnSupplementaryServiceIndication(msg);
  }
}

void ImsRadioImpl_1_5::notifyIncomingImsSms(std::shared_ptr<RilUnsolIncomingImsSMSMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyIncomingImsSms(msg);
  }
}

void ImsRadioImpl_1_5::notifyNewImsSmsStatusReport(
      std::shared_ptr<RilUnsolNewImsSmsStatusReportMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyNewImsSmsStatusReport(msg);
  }
}

void ImsRadioImpl_1_5::notifyOnCallStateChanged(
    std::shared_ptr<QcRilUnsolImsCallStatusMessage> msg) {
  do {
    if (!msg) {
      QCRIL_LOG_ERROR("msg is nullptr");
      break;
    }
    sp<V1_5::IImsRadioIndication> indCb = getIndicationCallbackV1_5();
    if (!indCb) {
      QCRIL_LOG_ERROR("fallback");
      if (mBaseImsRadioImpl) {
        mBaseImsRadioImpl->notifyOnCallStateChanged(msg);
      }
      break;
    }
    std::vector<qcril::interfaces::CallInfo> callInfo = msg->getCallInfo();
    if (callInfo.size() <= 0) {
      QCRIL_LOG_ERROR("empty callInfo");
      break;
    }
    hidl_vec<V1_5::CallInfo> callListV1_5 = {};
    bool result = V1_5::utils::convertCallInfoList(callListV1_5, callInfo);
    if (!result) {
      QCRIL_LOG_ERROR("CallInfo convertion failed");
      break;
    }
    imsRadiolog("<", "onCallStateChanged_1_5: callList = " + toString(callListV1_5));
    Return<void> ret = indCb->onCallStateChanged_1_5(callListV1_5);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  } while (FALSE);
}
void ImsRadioImpl_1_5::notifyOnIncomingCallAutoRejected(
    std::shared_ptr<QcRilUnsolAutoCallRejectionMessage> msg) {
  do {
    if (!msg) {
      QCRIL_LOG_ERROR("msg is nullptr");
      break;
    }
    sp<V1_5::IImsRadioIndication> indCb = getIndicationCallbackV1_5();
    if (!indCb) {
      QCRIL_LOG_ERROR("fallback");
      if (mBaseImsRadioImpl) {
        mBaseImsRadioImpl->notifyOnIncomingCallAutoRejected(msg);
      }
      break;
    }
    V1_5::AutoCallRejectionInfo info = {
        .callType = V1_0::CallType::CALL_TYPE_INVALID,
        .autoRejectionCause = V1_5::CallFailCause::CALL_FAIL_INVALID};

    if (msg->hasCallType()) {
      info.callType = V1_0::utils::convertCallType(msg->getCallType());
    }
    if (msg->hasCallFailCause()) {
      info.autoRejectionCause = V1_5::utils::convertCallFailCause(msg->getCallFailCause());
    }
    if (msg->hasSipErrorCode()) {
      info.sipErrorCode = msg->getSipErrorCode();
    }
    if (msg->hasNumber()) {
      info.number = msg->getNumber().c_str();
    }
    imsRadiolog("<", "onIncomingCallAutoRejected_1_5: autoCallRejectionInfo = " + toString(info));
    Return<void> ret = indCb->onIncomingCallAutoRejected_1_5(info);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  } while (FALSE);
}
void ImsRadioImpl_1_5::notifyOnPendingMultiIdentityStatus(
    std::shared_ptr<QcRilUnsolImsPendingMultiLineStatus> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnPendingMultiIdentityStatus(msg);
  }
}

void ImsRadioImpl_1_5::notifyOnMultiIdentityLineStatus(
    std::shared_ptr<QcRilUnsolImsMultiIdentityStatusMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnMultiIdentityLineStatus(msg);
  }
}

void ImsRadioImpl_1_5::notifyOnVoiceInfoStatusChange(
    std::shared_ptr<QcRilUnsolImsVoiceInfo> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnVoiceInfoStatusChange(msg);
  }
}

void ImsRadioImpl_1_5::notifyOnExitEcbmIndication(
    std::shared_ptr<QcRilUnsolImsExitEcbmIndication> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnExitEcbmIndication(msg);
  }
}

void ImsRadioImpl_1_5::notifyOnEnterEcbmIndication(
    std::shared_ptr<QcRilUnsolImsEnterEcbmIndication> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnEnterEcbmIndication(msg);
  }
}

void ImsRadioImpl_1_5::notifyOnRadioStateChanged(
    std::shared_ptr<QcRilUnsolImsRadioStateIndication> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnRadioStateChanged(msg);
  }
}

void ImsRadioImpl_1_5::notifyOnMessageWaiting(
      std::shared_ptr<QcRilUnsolMessageWaitingInfoMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnMessageWaiting(msg);
  }
}

void ImsRadioImpl_1_5::notifyOnModemSupportsWfcRoamingModeConfiguration(
    std::shared_ptr<QcRilUnsolImsWfcRoamingConfigIndication> msg) {
  if (msg) {
    sp<V1_5::IImsRadioIndication> indCb = getIndicationCallbackV1_5();
    if (indCb) {
      imsRadiolog("<", "onModemSupportsWfcRoamingModeConfiguration: status=" +
          std::to_string(msg->getWfcRoamingConfigSupport()));
      Return<void> ret = indCb->onModemSupportsWfcRoamingModeConfiguration(
          msg->getWfcRoamingConfigSupport());
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }
}

void ImsRadioImpl_1_5::notifyOnUssdMessageFailed(std::shared_ptr<QcRilUnsolOnUssdMessage> msg) {
  if (msg && msg->hasErrorDetails()) {
    sp<V1_5::IImsRadioIndication> indCb = getIndicationCallbackV1_5();
    if (indCb) {
      UssdModeType modeType = utils::convertUssdModeType(msg->getMode());
      V1_0::SipErrorInfo errorDetails;
      utils::convertSipErrorInfo(errorDetails, msg->getErrorDetails());
      imsRadiolog("<", "onUssdMessageFailed: type=" +
          toString(modeType) + toString(errorDetails));
      Return<void> ret = indCb->onUssdMessageFailed(
          modeType, errorDetails);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }
}

Return<void> ImsRadioImpl_1_5::setConfig_1_5(int32_t token, const V1_5::ConfigInfo& inConfig) {
  imsRadiolog(">", "setConfig_1_5: token = " + std::to_string(token) + toString(inConfig));
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
    V1_5::utils::readConfigInfo(inConfig, msg->getConfigInfo());
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
      [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                    std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
        uint32_t errorCode = RIL_E_GENERIC_FAILURE;
        (void)msg;
        std::shared_ptr<qcril::interfaces::ConfigInfo> respData = nullptr;
        if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
          errorCode = V1_0::utils::qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
          respData = std::static_pointer_cast<qcril::interfaces::ConfigInfo>(resp->data);
        }
        mBaseImsRadioImpl->sendSetConfigMessageResponse(token, errorCode, respData);
      });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    mBaseImsRadioImpl->sendSetConfigMessageResponse(token, RIL_E_GENERIC_FAILURE, nullptr);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> ImsRadioImpl_1_5::sendImsSms(int32_t token, const V1_2::ImsSmsMessage &imsSms) {
  imsRadiolog(">", "sendImsSms: token=" + std::to_string(token) + " imsSms=" + toString(imsSms));

  auto respCb = getResponseCallbackV1_5();
  if (respCb == nullptr) {
    if (mBaseImsRadioImpl) {
        return mBaseImsRadioImpl->sendImsSms(token, imsSms);
    }
  } else {
    int ret = E_FAILURE;
    RIL_RadioTechnologyFamily tech = (imsSms.format == "3gpp") ? RADIO_TECH_3GPP :
            RADIO_TECH_3GPP2;
    auto msg = std::make_shared<RilRequestImsSendSmsMessage>(imsSms.messageRef,
            tech, imsSms.shallRetry);
    if (msg) {
        if (tech == RADIO_TECH_3GPP) {
            std::string payload;
            QCRIL_LOG_INFO("sendImsSms: pdu size = %d", imsSms.pdu.size());
            if (V1_0::utils::convertBytesToHexString(imsSms.pdu.data(), imsSms.pdu.size(), payload)) {
                msg->setGsmPayload(imsSms.smsc, payload);
                ret = E_SUCCESS;
            }
        } else {
            RIL_CDMA_SMS_Message cdmaMsg;
            if (V1_0::utils::convertPayloadToCdmaFormat(imsSms.pdu.data(),
                    imsSms.pdu.size(), cdmaMsg)) {
                msg->setCdmaPayload(cdmaMsg);
                ret = E_SUCCESS;
            }
        }
    }

    if (ret == E_SUCCESS) {
        GenericCallback<RilSendSmsResult_t> cb(
            ([this, token, tech](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                            std::shared_ptr<RilSendSmsResult_t> responseDataPtr) -> void {
                (void)status; (void)solicitedMsg;
                sp<V1_5::IImsRadioResponse> respCb = getResponseCallbackV1_5();
                if (respCb == nullptr) return;
                Return<void> retStatus;
                if (responseDataPtr) {
                    V1_5::ImsSmsSendFailureReason failureReason;
                    V1_2::ImsSmsSendStatusResult statusReport;
                    V1_5::utils::convertRilImsResponseToHidlResponse(responseDataPtr->rilErr,
                            tech, responseDataPtr->bLteOnlyReg, statusReport, failureReason);
                    imsRadiolog("<", "sendImsSmsResponse: token=" + std::to_string(token) +
                        " msgeRef=" + std::to_string(responseDataPtr->messageRef) +
                        " statusReport=" + toString(statusReport) +
                        " reason=" + toString(failureReason));
                    retStatus = respCb->sendImsSmsResponse_1_5(token,
                            responseDataPtr->messageRef, statusReport, failureReason);
                } else {
                    retStatus = respCb->sendImsSmsResponse_1_5(token, -1,
                            V1_2::ImsSmsSendStatusResult::SEND_STATUS_ERROR,
                            V1_5::ImsSmsSendFailureReason::RESULT_ERROR_GENERIC_FAILURE);
                }
                if (!retStatus.isOk()) {
                    QCRIL_LOG_ERROR("Unable to send response. Exception : %s",
                                    retStatus.description().c_str());
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        Return<void> retStatus = respCb->sendImsSmsResponse_1_5(token, -1,
                V1_2::ImsSmsSendStatusResult::SEND_STATUS_ERROR,
                V1_5::ImsSmsSendFailureReason::RESULT_ERROR_GENERIC_FAILURE);
        if (!retStatus.isOk()) {
            QCRIL_LOG_ERROR("Unable to send response. Exception : %s", retStatus.description().c_str());
        }
    }
  }
  return Void();
}

Return<void> ImsRadioImpl_1_5::acknowledgeSms_1_5(int32_t token, uint32_t messageRef,
        V1_5::ImsSmsDeliverStatusResult smsResult) {
    // currently no respCb defined for ims sms ack
    auto msg = std::make_shared<RilRequestAckImsSmsMessage>(messageRef,
            V1_5::utils::convertHidlToRilSmsAckResult(smsResult));
    if (msg) {
        GenericCallback<RIL_Errno> cb(
            ([token](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_Errno> responseDataPtr) -> void {
                (void)status;
                (void)solicitedMsg;
                (void)token;
                if (responseDataPtr) {
                    QCRIL_LOG_INFO("acknowledgeSms_1_5 cb, result: %d", *responseDataPtr);
                }
            }));
            msg->setCallback(&cb);
            msg->dispatch();
    }
    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

}  // namespace implementation
}  // namespace V1_5
}  // namespace ims
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
