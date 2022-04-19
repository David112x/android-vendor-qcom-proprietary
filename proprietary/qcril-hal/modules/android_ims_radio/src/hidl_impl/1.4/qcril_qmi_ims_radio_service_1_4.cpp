/******************************************************************************
  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/
#define TAG "RILQ"

#include <interfaces/voice/QcRilRequestImsAddParticipantMessage.h>
#include <interfaces/voice/QcRilRequestImsDialMessage.h>
#include <modules/android_ims_radio/hidl_impl/1.4/qcril_qmi_ims_radio_service_1_4.h>
#include <modules/android_ims_radio/hidl_impl/1.4/qcril_qmi_ims_radio_utils_1_4.h>
#include <modules/android_ims_radio/hidl_impl/1.5/qcril_qmi_ims_radio_utils_1_5.h>
#include <modules/voice/qcril_qmi_voice.h>
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
namespace V1_4 {
namespace implementation {

void ImsRadioImpl_1_4::setCallback_nolock(
    const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadioResponse>
        &imsRadioResponse,
    const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadioIndication>
        &imsRadioIndication) {
  QCRIL_LOG_DEBUG("ImsRadioImpl_1_4::setCallback_nolock");

  mImsRadioResponseCbV1_4 =
      V1_4::IImsRadioResponse::castFrom(imsRadioResponse).withDefault(nullptr);
  mImsRadioIndicationCbV1_4 =
      V1_4::IImsRadioIndication::castFrom(imsRadioIndication).withDefault(nullptr);
  if (mImsRadioResponseCbV1_4 == nullptr || mImsRadioIndicationCbV1_4 == nullptr) {
    mImsRadioResponseCbV1_4 = nullptr;
    mImsRadioIndicationCbV1_4 = nullptr;
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
::android::hardware::Return<void> ImsRadioImpl_1_4::setCallback(
    const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadioResponse>
        &imsRadioResponse,
    const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadioIndication>
        &imsRadioIndication) {
  QCRIL_LOG_DEBUG("ImsRadioImpl_1_4::setCallback");
  {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
    if (mImsRadioResponseCbV1_4 != nullptr) {
      mImsRadioResponseCbV1_4->unlinkToDeath(this);
    }
    setCallback_nolock(imsRadioResponse, imsRadioIndication);
    if (mImsRadioResponseCbV1_4 != nullptr) {
      mImsRadioResponseCbV1_4->linkToDeath(this, 0);
    }
  }
  notifyImsClientConnected();

  return Void();
}

sp<V1_4::IImsRadioResponse> ImsRadioImpl_1_4::getResponseCallbackV1_4() {
  std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mImsRadioResponseCbV1_4;
}

sp<V1_4::IImsRadioIndication> ImsRadioImpl_1_4::getIndicationCallbackV1_4() {
  std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mImsRadioIndicationCbV1_4;
}

void ImsRadioImpl_1_4::serviceDied(uint64_t,
                                   const ::android::wp<::android::hidl::base::V1_0::IBase> &) {
  QCRIL_LOG_DEBUG("ImsRadioImpl_1_4::serviceDied: Client died. Cleaning up callbacks");
  clearCallbacks();
}

ImsRadioImpl_1_4::ImsRadioImpl_1_4(qcril_instance_id_e_type instance) : ImsRadioImplBase(instance) {
  mBaseImsRadioImpl = new V1_0::implementation::ImsRadioImpl(instance);
}

ImsRadioImpl_1_4::~ImsRadioImpl_1_4() {}

void ImsRadioImpl_1_4::clearCallbacks() {
  QCRIL_LOG_ERROR("enter");
  {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
    mImsRadioIndicationCbV1_4 = nullptr;
    mImsRadioResponseCbV1_4 = nullptr;
    if (mBaseImsRadioImpl) {
      mBaseImsRadioImpl->clearCallbacks();
    }
  }
  QCRIL_LOG_ERROR("exit");
}

void ImsRadioImpl_1_4::registerService() {
  std::string serviceName = "imsradio" + std::to_string(getInstanceId());
  android::status_t ret = registerAsService(serviceName);
  QCRIL_LOG_INFO("registerService: starting ImsRadioImpl_1_4 %s status %d", serviceName.c_str(),
                 ret);
}

void ImsRadioImpl_1_4::notifyOnRegistrationChanged(
    std::shared_ptr<QcRilUnsolImsRegStateMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnRegistrationChanged(msg);
  }
}

void ImsRadioImpl_1_4::notifyOnRTTMessage(
    std::shared_ptr<QcRilUnsolImsRttMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnRTTMessage(msg);
  }
}

void ImsRadioImpl_1_4::notifyOnVowifiCallQuality(
    std::shared_ptr<QcRilUnsolImsVowifiCallQuality> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnVowifiCallQuality(msg);
  }
}

void ImsRadioImpl_1_4::notifyOnVopsChanged(
    std::shared_ptr<QcRilUnsolImsVopsIndication> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnVopsChanged(msg);
  }
}

void ImsRadioImpl_1_4::notifyOnSubConfigChanged(
    std::shared_ptr<QcRilUnsolImsSubConfigIndication> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnSubConfigChanged(msg);
  }
}

void ImsRadioImpl_1_4::notifyOnGeoLocationChange(
    std::shared_ptr<QcRilUnsolImsGeoLocationInfo> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnGeoLocationChange(msg);
  }
}

void ImsRadioImpl_1_4::notifyOnServiceStatusChange(
    std::shared_ptr<QcRilUnsolImsSrvStatusIndication> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnServiceStatusChange(msg);
  }
}

void ImsRadioImpl_1_4::notifyOnSsacInfoChange(
    std::shared_ptr<QcRilUnsolImsSsacInfoIndication> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnSsacInfoChange(msg);
  }
}

void ImsRadioImpl_1_4::notifyOnRegBlockStatusChange(
    std::shared_ptr<QcRilUnsolImsRegBlockStatusMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnRegBlockStatusChange(msg);
  }
}

void ImsRadioImpl_1_4::notifyOnModifyCall(std::shared_ptr<QcRilUnsolImsModifyCallMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnModifyCall(msg);
  }
}
void ImsRadioImpl_1_4::notifyOnRing(std::shared_ptr<QcRilUnsolCallRingingMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnRing(msg);
  }
}
void ImsRadioImpl_1_4::notifyOnRingbackTone(std::shared_ptr<QcRilUnsolRingbackToneMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnRingbackTone(msg);
  }
}
void ImsRadioImpl_1_4::notifyOnHandover(std::shared_ptr<QcRilUnsolImsHandoverMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnHandover(msg);
  }
}
void ImsRadioImpl_1_4::notifyOnTtyNotification(
    std::shared_ptr<QcRilUnsolImsTtyNotificationMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnTtyNotification(msg);
  }
}
void ImsRadioImpl_1_4::notifyOnRefreshConferenceInfo(
    std::shared_ptr<QcRilUnsolImsConferenceInfoMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnRefreshConferenceInfo(msg);
  }
}
void ImsRadioImpl_1_4::notifyOnRefreshViceInfo(std::shared_ptr<QcRilUnsolImsViceInfoMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnRefreshViceInfo(msg);
  }
}
void ImsRadioImpl_1_4::notifyOnSuppServiceNotification(
    std::shared_ptr<QcRilUnsolSuppSvcNotificationMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnSuppServiceNotification(msg);
  }
}
void ImsRadioImpl_1_4::notifyOnParticipantStatusInfo(
    std::shared_ptr<QcRilUnsolConfParticipantStatusInfoMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnParticipantStatusInfo(msg);
  }
}
void ImsRadioImpl_1_4::notifyOnIncomingCallAutoRejected(
    std::shared_ptr<QcRilUnsolAutoCallRejectionMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnIncomingCallAutoRejected(msg);
  }
}

void ImsRadioImpl_1_4::notifyOnSupplementaryServiceIndication(
      std::shared_ptr<QcRilUnsolSupplementaryServiceMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnSupplementaryServiceIndication(msg);
  }
}

void ImsRadioImpl_1_4::notifyIncomingImsSms(std::shared_ptr<RilUnsolIncomingImsSMSMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyIncomingImsSms(msg);
  }
}

void ImsRadioImpl_1_4::notifyNewImsSmsStatusReport(
      std::shared_ptr<RilUnsolNewImsSmsStatusReportMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyNewImsSmsStatusReport(msg);
  }
}

void ImsRadioImpl_1_4::notifyOnCallStateChanged(
    std::shared_ptr<QcRilUnsolImsCallStatusMessage> msg) {
  if (!msg) {
    QCRIL_LOG_ERROR("msg is nullptr");
    return;
  }
  sp<V1_4::IImsRadioIndication> indCb = getIndicationCallbackV1_4();
  if (!indCb) {
    QCRIL_LOG_ERROR("fallback");
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
  hidl_vec<V1_4::CallInfo> callListV1_4 = {};
  bool result = V1_4::utils::convertCallInfoList(callListV1_4, callInfo);
  if (!result) {
    QCRIL_LOG_ERROR("CallInfo convertion failed");
    return;
  }
  imsRadiolog("<", "onCallStateChanged_1_4: callList = " + toString(callListV1_4));
  Return<void> ret = indCb->onCallStateChanged_1_4(callListV1_4);
  if (!ret.isOk()) {
    QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
  }
}

void ImsRadioImpl_1_4::notifyOnPendingMultiIdentityStatus(
    std::shared_ptr<QcRilUnsolImsPendingMultiLineStatus> msg) {
  QCRIL_LOG_FUNC_ENTRY();
  if (msg) {
    sp<IImsRadioIndication> indCb = getIndicationCallbackV1_4();
    if (indCb) {
      imsRadiolog("<", "onMultiIdentityInfoPending");
      Return<void> ret = indCb->onMultiIdentityInfoPending();
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsRadioImpl_1_4::notifyOnMultiIdentityLineStatus(
    std::shared_ptr<QcRilUnsolImsMultiIdentityStatusMessage> msg) {
  QCRIL_LOG_FUNC_ENTRY();
  if (msg) {
    hidl_vec<V1_4::MultiIdentityLineInfoHal> hal_info;
    sp<IImsRadioIndication> indCb = getIndicationCallbackV1_4();
    std::vector<qcril::interfaces::MultiIdentityInfo> info = msg->getLineInfo();
    V1_4::utils::convertToHidlMultiIdentityRegistrationStatus(info, hal_info);
    if (indCb) {
      imsRadiolog("<", "onMultiIdentityRegistrationStatusChange: info=" + toString(hal_info));
      Return<void> ret = indCb->onMultiIdentityRegistrationStatusChange(hal_info);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }
  QCRIL_LOG_FUNC_RETURN();
}

void ImsRadioImpl_1_4::notifyOnVoiceInfoStatusChange(
    std::shared_ptr<QcRilUnsolImsVoiceInfo> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnVoiceInfoStatusChange(msg);
  }
}

void ImsRadioImpl_1_4::notifyOnExitEcbmIndication(
    std::shared_ptr<QcRilUnsolImsExitEcbmIndication> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnExitEcbmIndication(msg);
  }
}

void ImsRadioImpl_1_4::notifyOnEnterEcbmIndication(
    std::shared_ptr<QcRilUnsolImsEnterEcbmIndication> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnEnterEcbmIndication(msg);
  }
}

void ImsRadioImpl_1_4::notifyOnRadioStateChanged(
    std::shared_ptr<QcRilUnsolImsRadioStateIndication> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnRadioStateChanged(msg);
  }
}

void ImsRadioImpl_1_4::notifyOnMessageWaiting(
      std::shared_ptr<QcRilUnsolMessageWaitingInfoMessage> msg) {
  if (mBaseImsRadioImpl) {
    mBaseImsRadioImpl->notifyOnMessageWaiting(msg);
  }
}

/*
 *  @brief
 *  Server side implementtaion of the IImsradio service's dial API
 *
 */
Return<void> ImsRadioImpl_1_4::dial_1_4(int32_t token, const V1_4::DialRequest &dialRequest) {
  imsRadiolog(">", "dial_1_4: token = " + std::to_string(token) +
                       " dialRequest = " + toString(dialRequest));
  QCRIL_LOG_FUNC_ENTRY();
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestImsDialMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    // Set parameters
    if (!dialRequest.address.empty()) {
      msg->setAddress(dialRequest.address.c_str());
    } else {
      // Empty dial string is allowed for conference uri call
      if (!dialRequest.hasIsConferenceUri || !dialRequest.isConferenceUri) {
        QCRIL_LOG_ERROR("msg is nullptr");
        sendFailure = true;
        break;
      }
    }
    if (dialRequest.clirMode != INT32_MAX) {
      msg->setClir(dialRequest.clirMode);
    }
    if (dialRequest.hasCallDetails) {
      msg->setCallType(V1_0::utils::convertCallType(dialRequest.callDetails.callType));
      msg->setCallDomain(V1_0::utils::convertCallDomain(dialRequest.callDetails.callDomain));
      msg->setRttMode(V1_0::utils::convertRttMode(dialRequest.callDetails.rttMode));
      if (dialRequest.callDetails.extras.size()) {
        std::string displayText =
            V1_0::utils::getExtra("DisplayText", dialRequest.callDetails.extras);
        if (!displayText.empty()) {
          msg->setDisplayText(displayText);
        }
        std::string retryCallFailReason =
            V1_0::utils::getExtra("RetryCallFailReason", dialRequest.callDetails.extras);
        if (!retryCallFailReason.empty()) {
          msg->setRetryCallFailReason(V1_5::utils::convertCallFailCause(
              static_cast<V1_5::CallFailCause>(std::stoi(retryCallFailReason))));
        }
        std::string retryCallFailMode =
            V1_0::utils::getExtra("RetryCallFailRadioTech", dialRequest.callDetails.extras);
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

    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = V1_0::utils::qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
          }
          sendDialResponse(token, errorCode);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendDialResponse(token, RIL_E_GENERIC_FAILURE);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}
void ImsRadioImpl_1_4::sendDialResponse(int32_t token, uint32_t errorCode) {
  sp<V1_4::IImsRadioResponse> respCb = getResponseCallbackV1_4();
  if (respCb != nullptr) {
    imsRadiolog("<", "dialResponse: token = " + std::to_string(token) +
                         " errorCode = " + std::to_string(errorCode));
    Return<void> ret = respCb->dialResponse(token, errorCode);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

/*
 *  @brief
 *  Server side implementtaion of the IImsradio service's add participant API
 *
 */
Return<void> ImsRadioImpl_1_4::addParticipant_1_4(int32_t token,
                                                  const V1_4::DialRequest &dialRequest) {
  imsRadiolog(">", "addParticipant_1_4: token = " + std::to_string(token) +
                       " dialRequest = " + toString(dialRequest));
  QCRIL_LOG_FUNC_ENTRY();
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestImsAddParticipantMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    // Set parameters
    if (!dialRequest.address.empty()) {
      msg->setAddress(dialRequest.address.c_str());
    }

    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = V1_0::utils::qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
          }
          sendAddParticipantResponse(token, errorCode);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendAddParticipantResponse(token, RIL_E_GENERIC_FAILURE);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}
void ImsRadioImpl_1_4::sendAddParticipantResponse(int32_t token, uint32_t errorCode) {
  sp<V1_4::IImsRadioResponse> respCb = getResponseCallbackV1_4();
  if (respCb != nullptr) {
    imsRadiolog("<", "addParticipantResponse: token=" + std::to_string(token) +
                         " errorCode=" + std::to_string(errorCode));
    Return<void> ret = respCb->addParticipantResponse(token, errorCode);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

void ImsRadioImpl_1_4::sendRegisterMultiIdentityLinesResponse(uint32_t token, uint32_t errorCode,
  std::shared_ptr<qcril::interfaces::BasePayload> /*data*/) {
  sp<IImsRadioResponse> respCb = getResponseCallbackV1_4();
  if (respCb != nullptr) {
    imsRadiolog("<", "registerMultiIdentityLinesResponse: token=" + std::to_string(token) +
                         " errorCode=" + std::to_string(errorCode));
    Return<void> retStatus =
      respCb->registerMultiIdentityLinesResponse(token, errorCode);
    if (!retStatus.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s",
         retStatus.description().c_str());
    }
  }
}

Return<void> ImsRadioImpl_1_4::registerMultiIdentityLines(
    int32_t token, const hidl_vec<V1_4::MultiIdentityLineInfoHal> &lines) {
  imsRadiolog(">", "registerMultiIdentityLines: token=" + std::to_string(token) +
                       " info=" + toString(lines));
  bool sendFailure = false;
  auto ctx = getContext(token);

  do {
    std::shared_ptr<QcRilRequestImsRegisterMultiIdentityMessage> msg =
      std::make_shared<QcRilRequestImsRegisterMultiIdentityMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    std::vector<qcril::interfaces::MultiIdentityInfo> &out_lines = msg->getLineInfo();
    for (auto &in_line : lines) {
      qcril::interfaces::MultiIdentityInfo out_line;
      // from here only activation requests will be sent to modem
      V1_4::utils::convertToRilMultiIdentityInfo(in_line, out_line);
      out_lines.push_back(out_line);
    }

    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
      [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                    std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
        uint32_t errorCode = RIL_E_GENERIC_FAILURE;
        (void)msg;
        if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
          errorCode = V1_0::utils::qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
        }
        sendRegisterMultiIdentityLinesResponse(token, errorCode, nullptr);
      });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);

  if (sendFailure) {
    sendRegisterMultiIdentityLinesResponse(token, RIL_E_GENERIC_FAILURE, nullptr);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

void ImsRadioImpl_1_4::sendQueryVirtualLinesResponse(uint32_t token, uint32_t /*errorCode*/,
  std::shared_ptr<qcril::interfaces::VirtualLineInfo> data) {
  hidl_string msisdn;
  hidl_vec<hidl_string> lines;
  if (data != nullptr) {
    msisdn = data->getMsisdn();
    V1_4::utils::convertToHidlVirtualLines(lines, data->getVirtualLines());
  }
  sp<IImsRadioResponse> respCb = getResponseCallbackV1_4();

  if (respCb != nullptr) {
    imsRadiolog("<", "sendQueryVirtualLinesResponse: token=" + std::to_string(token) +
                         " msisdn=" + toString(msisdn) + " lines=" + toString(lines));
    Return<void> retStatus =
      respCb->queryVirtualLineInfoResponse(token, msisdn, lines);
    if (!retStatus.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s",
         retStatus.description().c_str());
    }
  }
}

Return<void> ImsRadioImpl_1_4::queryVirtualLineInfo(int32_t token, const hidl_string &msisdn) {
  imsRadiolog(">",
      "queryVirtualLineInfo: token=" + std::to_string(token) + " msisdn=" + toString(msisdn));
  bool sendFailure = false;
  auto ctx = getContext(token);
  do {
    if (msisdn.empty()) {
      QCRIL_LOG_ERROR("invalid input");
      sendFailure = true;
      break;
    }
    std::shared_ptr<QcRilRequestImsQueryVirtualLineInfo> msg =
      std::make_shared<QcRilRequestImsQueryVirtualLineInfo>(ctx, msisdn);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
      [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                    std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
        uint32_t errorCode = RIL_E_GENERIC_FAILURE;
        std::shared_ptr<qcril::interfaces::VirtualLineInfo> respData = nullptr;
        (void)msg;
        if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
          errorCode = V1_0::utils::qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
          respData = std::static_pointer_cast<qcril::interfaces::VirtualLineInfo>(resp->data);
        }
        sendQueryVirtualLinesResponse(token, errorCode, respData);
      });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);

  if (sendFailure) {
    sendQueryVirtualLinesResponse(token, RIL_E_GENERIC_FAILURE, nullptr);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}


}  // namespace implementation
}  // namespace V1_4
}  // namespace ims
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
