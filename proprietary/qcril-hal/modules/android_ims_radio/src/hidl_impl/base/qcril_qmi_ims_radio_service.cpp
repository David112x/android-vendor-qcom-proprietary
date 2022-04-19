/******************************************************************************
  @file    qcril_qmi_ims_radio_service.cpp
  @brief   qcril qmi - ims_radio_service

  DESCRIPTION
    Implements the server side of the IImsradio interface. Handles RIL
    requests and responses to be received and sent to client respectively

  ---------------------------------------------------------------------------

  Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/
#define TAG "RILQ"

#include <interfaces/ims/QcRilRequestImsGetRegStateMessage.h>
#include <interfaces/ims/QcRilRequestImsQueryServiceStatusMessage.h>
#include <interfaces/ims/QcRilRequestImsRegistrationChangeMessage.h>
#include <interfaces/ims/QcRilRequestImsGetRtpStatsMessage.h>
#include <interfaces/ims/QcRilRequestImsGetRtpErrorStatsMessage.h>
#include <interfaces/ims/QcRilRequestImsGetSubConfigMessage.h>
#include <interfaces/ims/QcRilRequestImsGeoLocationInfoMessage.h>
#include <interfaces/ims/QcRilRequestImsSetServiceStatusMessage.h>
#include <interfaces/ims/QcRilRequestImsSendRttMessage.h>
#include <interfaces/ims/QcRilRequestImsSetConfigMessage.h>
#include <interfaces/ims/QcRilRequestImsGetConfigMessage.h>

#include <interfaces/ims/ims.h>
#include <interfaces/voice/QcRilRequestImsAddParticipantMessage.h>
#include <interfaces/voice/QcRilRequestImsAnswerMessage.h>
#include <interfaces/voice/QcRilRequestImsCancelModifyCallMessage.h>
#include <interfaces/voice/QcRilRequestImsConferenceMessage.h>
#include <interfaces/voice/QcRilRequestImsDeflectCallMessage.h>
#include <interfaces/voice/QcRilRequestImsDialMessage.h>
#include <interfaces/voice/QcRilRequestImsExplicitCallTransferMessage.h>
#include <interfaces/voice/QcRilRequestGetCallWaitingMessage.h>
#include <interfaces/voice/QcRilRequestGetClipMessage.h>
#include <interfaces/voice/QcRilRequestGetClirMessage.h>
#include <interfaces/voice/QcRilRequestGetColrMessage.h>
#include <interfaces/voice/QcRilRequestImsHangupMessage.h>
#include <interfaces/voice/QcRilRequestImsHoldMessage.h>
#include <interfaces/voice/QcRilRequestImsModifyCallConfirmMessage.h>
#include <interfaces/voice/QcRilRequestImsModifyCallInitiateMessage.h>
#include <interfaces/voice/QcRilRequestQueryCallForwardMessage.h>
#include <interfaces/voice/QcRilRequestImsResumeMessage.h>
#include <interfaces/voice/QcRilRequestImsSendDtmfMessage.h>
#include <interfaces/voice/QcRilRequestSetCallForwardMessage.h>
#include <interfaces/voice/QcRilRequestSetCallWaitingMessage.h>
#include <interfaces/voice/QcRilRequestSetSuppSvcNotificationMessage.h>
#include <interfaces/voice/QcRilRequestSetSupsServiceMessage.h>
#include <interfaces/voice/QcRilRequestQueryColpMessage.h>
#include <interfaces/voice/QcRilRequestGetCallBarringMessage.h>
#include <interfaces/voice/QcRilRequestSetClirMessage.h>
#include <interfaces/voice/QcRilRequestSetColrMessage.h>
#include <interfaces/voice/QcRilRequestImsSetUiTtyModeMessage.h>
#include <interfaces/voice/QcRilRequestImsStartDtmfMessage.h>
#include <interfaces/voice/QcRilRequestImsStopDtmfMessage.h>
#include <interfaces/nas/RilRequestExitEmergencyCallbackMessage.h>
#include <modules/android_ims_radio/hidl_impl/base/qcril_qmi_ims_radio_service.h>
#include <modules/android_ims_radio/hidl_impl/base/qcril_qmi_ims_radio_utils.h>
#include <modules/android_ims_radio/hidl_impl/1.5/qcril_qmi_ims_radio_service_1_5.h>
#include <modules/android_ims_radio/hidl_impl/1.5/qcril_qmi_ims_radio_utils_1_5.h>
#include <modules/voice/qcril_qmi_voice.h>
#include <string.h>
#include <modules/sms/RilRequestImsSendSmsMessage.h>
#include <framework/QcrilImsClientConnected.h>

#include "modules/sms/qcril_qmi_sms.h"

extern "C" {
    #include "qcril_log.h"
    #include "qcril_reqlist.h"
}

#ifdef QMI_RIL_UTF
#include "unistd.h"
extern "C" uint32 qcril_get_time_milliseconds();
#endif

#if defined(FEATURE_TARGET_GLIBC_x86)  || defined(QMI_RIL_UTF)
    extern "C" size_t strlcat(char *, const char *, size_t);
    extern "C" size_t strlcpy(char *, const char *, size_t);
#endif

using namespace vendor::qti::hardware::radio::ims::V1_0::implementation;
using namespace vendor::qti::hardware::radio::ims::V1_0::utils;

using ::android::hardware::Void;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;

void ImsRadioImplBase::imsRadiolog(std::string dir, std::string logStr) {
  for (unsigned li = 0; li < logStr.length(); li += 150) {
    std::string subStr = logStr.substr(li, 150);
    QCRIL_LOG_INFO("%s [%zu/%zu] %s", dir.c_str(), logStr.length(),
                   li + subStr.length(), subStr.c_str());
  }
}

void ImsRadioImplBase::notifyImsClientConnected() {
  auto msg = std::make_shared<QcrilImsClientConnected>();
  if (msg != nullptr) {
    msg->broadcast();
  }
}

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_0 {
namespace implementation {

void ImsRadioImpl::setCallback_nolock(const sp<IImsRadioResponse> &imsRadioResponse,
                                      const sp<IImsRadioIndication> &imsRadioIndication) {
  QCRIL_LOG_DEBUG("ImsRadioImpl::setCallback_nolock");

  mImsRadioResponseCb = imsRadioResponse;
  mImsRadioIndicationCb = imsRadioIndication;
  mImsRadioIndicationCbV1_1 =
      V1_1::IImsRadioIndication::castFrom(imsRadioIndication).withDefault(nullptr);
  mImsRadioResponseCbV1_2 =
      V1_2::IImsRadioResponse::castFrom(mImsRadioResponseCb).withDefault(nullptr);
  mImsRadioIndicationCbV1_2 =
      V1_2::IImsRadioIndication::castFrom(mImsRadioIndicationCb).withDefault(nullptr);
  if (mImsRadioResponseCbV1_2 == nullptr || mImsRadioIndicationCbV1_2 == nullptr) {
    mImsRadioResponseCbV1_2 = nullptr;
    mImsRadioIndicationCbV1_2 = nullptr;
  }
  mImsRadioResponseCbV1_3 =
      V1_3::IImsRadioResponse::castFrom(mImsRadioResponseCb).withDefault(nullptr);
  mImsRadioIndicationCbV1_3 =
      V1_3::IImsRadioIndication::castFrom(mImsRadioIndicationCb).withDefault(nullptr);
  if (mImsRadioResponseCbV1_3 == nullptr || mImsRadioIndicationCbV1_3 == nullptr) {
    mImsRadioResponseCbV1_3 = nullptr;
    mImsRadioIndicationCbV1_3 = nullptr;
  }
}
/*
 *   @brief
 *   Registers the callback for IImsRadio using the IImsRadioCallback object
 *   being passed in by the client as a parameter
 *
 */
Return<void> ImsRadioImpl::setCallback(const sp<IImsRadioResponse> &imsRadioResponse,
                                       const sp<IImsRadioIndication> &imsRadioIndication) {
  QCRIL_LOG_DEBUG("ImsRadioImpl::setCallback");
  {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
    if (mImsRadioResponseCb != nullptr) {
      mImsRadioResponseCb->unlinkToDeath(this);
    }
    setCallback_nolock(imsRadioResponse, imsRadioIndication);
    if (mImsRadioResponseCb != nullptr) {
      mImsRadioResponseCb->linkToDeath(this, 0);
    }
  }
  notifyImsClientConnected();

  return Void();
}

sp<IImsRadioResponse> ImsRadioImpl::getResponseCallback() {
  std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mImsRadioResponseCb;
}

sp<IImsRadioIndication> ImsRadioImpl::getIndicationCallback() {
  std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mImsRadioIndicationCb;
}

sp<V1_1::IImsRadioIndication> ImsRadioImpl::getIndicationCallbackV1_1() {
  std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mImsRadioIndicationCbV1_1;
}

sp<V1_2::IImsRadioResponse> ImsRadioImpl::getResponseCallbackV1_2() {
  std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mImsRadioResponseCbV1_2;
}

sp<V1_2::IImsRadioIndication> ImsRadioImpl::getIndicationCallbackV1_2() {
  std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mImsRadioIndicationCbV1_2;
}

sp<V1_3::IImsRadioResponse> ImsRadioImpl::getResponseCallbackV1_3() {
  std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mImsRadioResponseCbV1_3;
}

sp<V1_3::IImsRadioIndication> ImsRadioImpl::getIndicationCallbackV1_3() {
  std::shared_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
  return mImsRadioIndicationCbV1_3;
}

void ImsRadioImpl::serviceDied(uint64_t,
                               const ::android::wp<::android::hidl::base::V1_0::IBase> &) {
  QCRIL_LOG_DEBUG("ImsRadioImpl::serviceDied: Client died. Cleaning up callbacks");
  clearCallbacks();
}

ImsRadioImpl::ImsRadioImpl(qcril_instance_id_e_type instance) : ImsRadioImplBase(instance) {}

ImsRadioImpl::~ImsRadioImpl() {}

void ImsRadioImpl::clearCallbacks() {
  QCRIL_LOG_ERROR("enter");
  {
    std::unique_lock<qtimutex::QtiSharedMutex> lock(mCallbackLock);
    mImsRadioResponseCb = nullptr;
    mImsRadioIndicationCb = nullptr;
    mImsRadioIndicationCbV1_1 = nullptr;
    mImsRadioResponseCbV1_2 = nullptr;
    mImsRadioIndicationCbV1_2 = nullptr;
    mImsRadioIndicationCbV1_3 = nullptr;
    mImsRadioResponseCbV1_3 = nullptr;
  }
  QCRIL_LOG_ERROR("exit");
}

void ImsRadioImpl::registerService() {
  std::string serviceName = "imsradio" + std::to_string(getInstanceId());
  android::status_t ret = registerAsService(serviceName);
  QCRIL_LOG_INFO("registerService: starting ImsRadioImpl %s status %d", serviceName.c_str(), ret);
}

void ImsRadioImpl::notifyOnRegistrationChanged(std::shared_ptr<QcRilUnsolImsRegStateMessage> msg) {
  if (msg) {
    std::shared_ptr<qcril::interfaces::Registration> reg = msg->getRegistration();
    if (reg != nullptr) {
      RegistrationInfo regInfo = {.state = RegState::INVALID,
                                  .radioTech = RadioTechType::RADIO_TECH_INVALID};
      convertRegistrationInfo(regInfo, *reg);
      sp<IImsRadioIndication> indCb = getIndicationCallback();
      if (indCb != nullptr) {
        Return<void> ret = indCb->onRegistrationChanged(regInfo);
        if (!ret.isOk()) {
          QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
        }
      }
    }
  }
}

void ImsRadioImpl::notifyOnCallStateChanged(std::shared_ptr<QcRilUnsolImsCallStatusMessage> msg) {
  if (msg) {
    std::vector<qcril::interfaces::CallInfo> callInfo = msg->getCallInfo();
    hidl_vec<V1_3::CallInfo> callListV1_3 = {};
    Return<void> ret;

    do {
      if (callInfo.empty()) {
        break;
      }
      bool result = convertCallInfoList(callListV1_3, callInfo);
      if (!result) {
        break;
      }
      sp<V1_3::IImsRadioIndication> indCbV1_3 = getIndicationCallbackV1_3();
      if (indCbV1_3 != nullptr) {
        imsRadiolog("<", "onCallStateChanged_1_3: callList = " + toString(callListV1_3));
        ret = indCbV1_3->onCallStateChanged_1_3(callListV1_3);
        break;
      }
      // indCbV1_3 is a nullptr. Trying to use indCbV1_2
      QCRIL_LOG_DEBUG("indCbV1_3 is nullptr. Falling back to V1.2");
      sp<V1_2::IImsRadioIndication> indCbV1_2 = getIndicationCallbackV1_2();
      hidl_vec<V1_2::CallInfo> callListV1_2;
      convertCallListV13ToV12(callListV1_3, callListV1_2);
      if (indCbV1_2 != nullptr) {
        imsRadiolog("<", "onCallStateChanged_1_2: callList = " + toString(callListV1_2));
        ret = indCbV1_2->onCallStateChanged_1_2(callListV1_2);
        break;
      }
      // indCbV1_2 is a nullptr. Trying to use indCbV1_1
      QCRIL_LOG_DEBUG("indCbV1_2 is nullptr. Falling back to V1.1");
      sp<V1_1::IImsRadioIndication> indCbV1_1 = getIndicationCallbackV1_1();
      hidl_vec<V1_1::CallInfo> callListV1_1;
      convertCallListV12ToV11(callListV1_2, callListV1_1);
      if (indCbV1_1 != nullptr) {
        imsRadiolog("<", "onCallStateChanged_1_1: callList = " + toString(callListV1_1));
        ret = indCbV1_1->onCallStateChanged_1_1(callListV1_1);
        break;
      }
      // indCbV1_1 is a nullptr. Trying to use indCb
      QCRIL_LOG_DEBUG("indCbV1_1 is nullptr. Falling back to V1.0");
      sp<IImsRadioIndication> indCb = getIndicationCallback();
      hidl_vec<V1_0::CallInfo> callListV1_0;
      convertCallListV11ToV10(callListV1_1, callListV1_0);
      if (indCb != nullptr) {
        imsRadiolog("<", "onCallStateChanged: callList = " + toString(callListV1_0));
        ret = indCb->onCallStateChanged(callListV1_0);
        break;
      }
    } while (FALSE);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}
void ImsRadioImpl::notifyOnModifyCall(std::shared_ptr<QcRilUnsolImsModifyCallMessage> msg) {
  if (msg) {
    CallModifyInfo modifyInfo = {.callIndex = INT32_MAX,
                                 .hasCallDetails = false,
                                 .failCause = CallModifyFailCause::E_INVALID};
    if (msg->hasCallIndex()) {
      modifyInfo.callIndex = msg->getCallIndex();
    }

    if (msg->hasCallType()) {
      modifyInfo.hasCallDetails = true;
      modifyInfo.callDetails.callType = convertCallType(msg->getCallType());
    }

    if (msg->hasCallDomain()) {
      modifyInfo.hasCallDetails = true;
      modifyInfo.callDetails.callDomain = convertCallDomain(msg->getCallDomain());
    }

    if (msg->hasRttMode()) {
      modifyInfo.hasCallDetails = true;
      modifyInfo.callDetails.rttMode = convertRttMode(msg->getRttMode());
    }

    if (msg->hasCallModifyFailCause()) {
      modifyInfo.failCause = convertCallModifyFailCause(msg->getCallModifyFailCause());
    }
    sp<IImsRadioIndication> indCb = getIndicationCallback();
    if (indCb != nullptr) {
      imsRadiolog("<", "onModifyCall: modifyInfo = " + toString(modifyInfo));
      Return<void> ret = indCb->onModifyCall(modifyInfo);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }
}
void ImsRadioImpl::notifyOnRing(std::shared_ptr<QcRilUnsolCallRingingMessage> msg) {
  if (msg) {
    sp<IImsRadioIndication> indCb = getIndicationCallback();
    if (indCb != nullptr) {
      imsRadiolog("<", "onRing");
      Return<void> ret = indCb->onRing();
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }
}
void ImsRadioImpl::notifyOnRingbackTone(std::shared_ptr<QcRilUnsolRingbackToneMessage> msg) {
  if (msg) {
    ToneOperation tone = convertToneOperation(msg->getRingBackToneOperation());
    sp<IImsRadioIndication> indCb = getIndicationCallback();
    if (indCb != nullptr) {
      imsRadiolog("<", "onRingbackTone: tone = " + toString(tone));
      Return<void> ret = indCb->onRingbackTone(tone);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }
}

#define WLAN_HANDOVER_NO_LTE_FAILURE_CODE_STRING "CD-04"

void ImsRadioImpl::notifyOnHandover(std::shared_ptr<QcRilUnsolImsHandoverMessage> msg) {
  if (msg) {
    HandoverInfo handoverInfo = {.type = HandoverType::INVALID,
                                 .srcTech = RadioTechType::RADIO_TECH_INVALID,
                                 .targetTech = RadioTechType::RADIO_TECH_INVALID,
                                 .hasHoExtra = false};
    if (msg->hasHandoverType()) {
      handoverInfo.type = convertHandoverType(msg->getHandoverType());
    }
    if (msg->hasSourceTech()) {
      handoverInfo.srcTech = convertRadioTechType(msg->getSourceTech());
    }
    if (msg->hasTargetTech()) {
      handoverInfo.targetTech = convertRadioTechType(msg->getTargetTech());
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
          (msg->getCauseCode().find(WLAN_HANDOVER_NO_LTE_FAILURE_CODE_STRING) == 0)) {
        handoverInfo.errorCode = WLAN_HANDOVER_NO_LTE_FAILURE_CODE_STRING;
        // --debug--: TODO a better c++ way?
        size_t errormessage_start = strlen(WLAN_HANDOVER_NO_LTE_FAILURE_CODE_STRING);
        // Check for the delimeter ":" after the errorcode
        // and discard any extra spaces
        while (errormessage_start < msg->getCauseCode().size()) {
          if (msg->getCauseCode()[errormessage_start] == ':') {
            errormessage_start++;
            while (msg->getCauseCode()[errormessage_start] == ' ') {
              errormessage_start++;
            }
            break;
          }
        }
        std::string errorMessage = msg->getCauseCode().substr(errormessage_start);
        handoverInfo.errorMessage = errorMessage;
      } else {
        handoverInfo.hasHoExtra = true;
        handoverInfo.hoExtra.type = ExtraType::LTE_TO_IWLAN_HO_FAIL;
        uint8_t *buffer = new uint8_t[msg->getCauseCode().length()];
        for (size_t idx = 0; idx < msg->getCauseCode().length(); idx++) {
          buffer[idx] = msg->getCauseCode()[idx];
        }
        handoverInfo.hoExtra.extraInfo.setToExternal(buffer, msg->getCauseCode().length(), true);
      }
    }

    sp<IImsRadioIndication> indCb = getIndicationCallback();
    if (indCb != nullptr) {
      imsRadiolog("<", "onHandover: handoverInfo = " + toString(handoverInfo));
      Return<void> ret = indCb->onHandover(handoverInfo);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }
}

void ImsRadioImpl::notifyOnTtyNotification(
    std::shared_ptr<QcRilUnsolImsTtyNotificationMessage> msg) {
  if (msg) {
    TtyInfo ttyInfo = {.mode=TtyMode::TTY_MODE_INVALID};
    if (msg->hasTtyMode()) {
      ttyInfo.mode = convertTtyMode(msg->getTtyMode());
    }
    sp<IImsRadioIndication> indCb = getIndicationCallback();
    if (indCb != nullptr) {
      imsRadiolog("<", "onTtyNotification: ttyInfo = " + toString(ttyInfo));
      Return<void> ret = indCb->onTtyNotification(ttyInfo);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }
}

void ImsRadioImpl::notifyOnRefreshConferenceInfo(std::shared_ptr<QcRilUnsolImsConferenceInfoMessage> msg) {
  if (msg) {
    ConferenceInfo conferenceInfo = {.conferenceCallState = ConferenceCallState::INVALID};
    if (msg->hasConferenceCallState()) {
      conferenceInfo.conferenceCallState =
          convertConferenceCallState(msg->getConferenceCallState());
    }
    if (msg->hasConferenceInfoUri() && msg->getConferenceInfoUri().size()) {
      uint8_t *buffer = new uint8_t[msg->getConferenceInfoUri().size()];
      for (size_t idx = 0; idx < msg->getConferenceInfoUri().size(); idx++) {
        buffer[idx] = msg->getConferenceInfoUri()[idx];
      }
      conferenceInfo.confInfoUri.setToExternal(buffer, msg->getConferenceInfoUri().size(), true);
    }

    sp<IImsRadioIndication> indCb = getIndicationCallback();
    if (indCb != nullptr) {
      imsRadiolog("<", "onRefreshConferenceInfo: conferenceInfo = " + toString(conferenceInfo));
      Return<void> ret = indCb->onRefreshConferenceInfo(conferenceInfo);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }
}

void ImsRadioImpl::notifyOnRefreshViceInfo(std::shared_ptr<QcRilUnsolImsViceInfoMessage> msg) {
  if (msg) {
    ViceInfo viceInfo = {};
    if (msg->hasViceInfoUri() && msg->getViceInfoUri().size()) {
      uint8_t *buffer = new uint8_t[msg->getViceInfoUri().size()];
      for (size_t idx = 0; idx < msg->getViceInfoUri().size(); idx++) {
        buffer[idx] = msg->getViceInfoUri()[idx];
      }
      viceInfo.viceInfoUri.setToExternal(buffer, msg->getViceInfoUri().size(), true);
    }

    sp<IImsRadioIndication> indCb = getIndicationCallback();
    if (indCb != nullptr) {
      imsRadiolog("<", "onRefreshViceInfo: viceInfo = " + toString(viceInfo));
      Return<void> ret = indCb->onRefreshViceInfo(viceInfo);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }
}

void ImsRadioImpl::notifyOnSuppServiceNotification(
    std::shared_ptr<QcRilUnsolSuppSvcNotificationMessage> msg) {
  if (msg) {
    SuppServiceNotification data = {
        .notificationType = NotificationType::INVALID,
        .code = INT32_MAX,
        .index = INT32_MAX,
        .type = INT32_MAX,
        .connId = INT32_MAX,
        .hasHoldTone = false};

    if (msg->hasNotificationType()) {
      data.notificationType = convertNotificationType(msg->getNotificationType());
    }
    if (msg->hasCode()) {
      data.code = msg->getCode();
    }
    if (msg->hasIndex()) {
      data.index = msg->getIndex();
    }
    if (msg->hasType()) {
      data.type = msg->getType();
    }
    if (msg->hasNumber() && !msg->getNumber().empty()) {
      data.number = msg->getNumber().c_str();
    }
    if (msg->hasConnectionId()) {
      data.connId = msg->getConnectionId();
    }
    if (msg->hasHistoryInfo() && !msg->getHistoryInfo().empty()) {
      data.historyInfo = msg->getHistoryInfo().c_str();
    }
    if (msg->hasHoldTone()) {
      data.hasHoldTone = true;
      data.holdTone = msg->getHoldTone();
    }
    sp<IImsRadioIndication> indCb = getIndicationCallback();
    if (indCb != nullptr) {
      imsRadiolog("<", "onSuppServiceNotification: suppServiceNotification = " + toString(data));
      Return<void> ret = indCb->onSuppServiceNotification(data);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }
}

void ImsRadioImpl::notifyOnParticipantStatusInfo(
    std::shared_ptr<QcRilUnsolConfParticipantStatusInfoMessage> msg) {
  if (msg) {
    ParticipantStatusInfo participantStatusInfo = {.callId = INT32_MAX,
                                                   .operation = ConfParticipantOperation::INVALID,
                                                   .sipStatus = INT32_MAX,
                                                   .hasIsEct = false};
    if (msg->hasCallId()) {
      participantStatusInfo.callId = msg->getCallId();
    }
    if (msg->hasOperation()) {
      participantStatusInfo.operation = convertConfParticipantOperation(msg->getOperation());
    }
    if (msg->hasSipStatus()) {
      participantStatusInfo.sipStatus = msg->getSipStatus();
    }
    if (msg->hasParticipantUri() && !msg->getParticipantUri().empty()) {
      participantStatusInfo.participantUri = msg->getParticipantUri().c_str();
    }
    if (msg->hasIsEct()) {
      participantStatusInfo.hasIsEct = true;
      participantStatusInfo.isEct = msg->getIsEct();
    }

    sp<IImsRadioIndication> indCb = getIndicationCallback();
    if (indCb != nullptr) {
      imsRadiolog("<", "onParticipantStatusInfo: participantStatusInfo = " +
                           toString(participantStatusInfo));
      Return<void> ret = indCb->onParticipantStatusInfo(participantStatusInfo);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }
}
void ImsRadioImpl::notifyOnIncomingCallAutoRejected(
    std::shared_ptr<QcRilUnsolAutoCallRejectionMessage> msg) {
  if (msg) {
    V1_3::AutoCallRejectionInfo info = {
        .callType = CallType::CALL_TYPE_INVALID,
        .autoRejectionCause = V1_3::CallFailCause::CALL_FAIL_INVALID};

    if (msg->hasCallType()) {
      info.callType = convertCallType(msg->getCallType());
    }
    if (msg->hasCallFailCause()) {
      info.autoRejectionCause = convertCallFailCause(msg->getCallFailCause());
    }
    if (msg->hasSipErrorCode()) {
      info.sipErrorCode = msg->getSipErrorCode();
    }
    if (msg->hasNumber()) {
      info.number = msg->getNumber().c_str();
    }
    sp<V1_3::IImsRadioIndication> indCbV1_3 = getIndicationCallbackV1_3();
    if (indCbV1_3 != nullptr) {
      imsRadiolog("<", "onIncomingCallAutoRejected: autoCallRejectionInfo = " + toString(info));
      Return<void> ret = indCbV1_3->onIncomingCallAutoRejected(info);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }
}

void ImsRadioImpl::notifyOnSupplementaryServiceIndication(
    std::shared_ptr<QcRilUnsolSupplementaryServiceMessage> msg) {
  if (msg) {
    StkCcUnsolSsResult ss = {.serviceType = SsServiceType::SS_INVALID,
                             .requestType = SsRequestType::SS_INVALID,
                             .teleserviceType = SsTeleserviceType::SS_INVALID,
                             .serviceClass = INT32_MAX,
                             .result = INT32_MAX};

    if (msg->hasServiceType()) {
      ss.serviceType = (SsServiceType)msg->getServiceType();
    }
    if (msg->hasRequestType()) {
      ss.requestType = (SsRequestType)msg->getRequestType();
    }
    if (msg->hasTeleserviceType()) {
      ss.teleserviceType = (SsTeleserviceType)msg->getTeleserviceType();
    }
    if (msg->hasServiceClass()) {
      ss.serviceClass = msg->getServiceClass();
    }
    if (msg->hasResult()) {
      ss.result = msg->getResult();
    }

    if (isServiceTypeCfQuery(ss.serviceType, ss.requestType) &&
        msg->hasCallForwardInfoList() && !msg->getCallForwardInfoList().empty()) {
      std::vector<qcril::interfaces::CallForwardInfo> rilCfInfo = msg->getCallForwardInfoList();
      ss.cfData.resize(1);
      ss.ssInfoData.resize(0);

      /* number of call info's */
      ss.cfData[0].cfInfo.resize(rilCfInfo.size());

      for (int i = 0; i < rilCfInfo.size() && i < NUM_SERVICE_CLASSES; i++) {
        qcril::interfaces::CallForwardInfo cf = rilCfInfo[i];
        CallForwardInfo *cfInfo = &ss.cfData[0].cfInfo[i];

        if (cf.hasStatus()) {
          cfInfo->status = cf.getStatus();
        }
        if (cf.hasReason()) {
          cfInfo->reason = cf.getReason();
        }
        if (cf.hasServiceClass()) {
          cfInfo->serviceClass = cf.getServiceClass();
        }
        if (cf.hasToa()) {
          cfInfo->toa = cf.getToa();
        }
        if (!cf.getNumber().empty()) {
          cfInfo->number = cf.getNumber().c_str();
        }
        if (cf.hasTimeSeconds()) {
          cfInfo->timeSeconds = cf.getTimeSeconds();
        }
#if 0
          RLOGD(
              "onSupplementaryServiceIndicationInd: "
              "Data: %d,reason=%d,cls=%d,toa=%d,num=%s,tout=%d],",
              cf.status, cf.reason, cf.serviceClass, cf.toa, (char *)cf.number, cf.timeSeconds);
#endif
      }
    } else {
      ss.ssInfoData.resize(1);
      ss.cfData.resize(0);

      if (msg->hasSuppSrvInfoList() && !msg->getSuppSrvInfoList().empty()) {
        /* each int */
        ss.ssInfoData[0].ssInfo.resize(SS_INFO_MAX);
        for (int i = 0; i < msg->getSuppSrvInfoList().size() && i < SS_INFO_MAX; i++) {
          ss.ssInfoData[0].ssInfo[i] = msg->getSuppSrvInfoList()[i];
        }
      }
    }

    if (msg->hasBarredNumberList()) {
      std::vector<std::string> cbNumList = msg->getBarredNumberList();
      if (isServiceTypeIcbQueryWithNumber(ss.serviceType, ss.requestType) && cbNumList.size()) {
        ss.cbNumInfo.resize(cbNumList.size());
        for (int i = 0; i < cbNumList.size(); i++) {
          ss.cbNumInfo[i].status = ServiceClassStatus::ENABLED;
          if (!cbNumList[i].empty()) {
            ss.cbNumInfo[i].number = cbNumList[i].c_str();
          }
        }
      }
    }
    sp<IImsRadioIndication> indCb = getIndicationCallback();
    if (indCb != nullptr) {
      imsRadiolog("<", "onSupplementaryServiceIndication: ss = " + toString(ss));
      Return<void> ret = indCb->onSupplementaryServiceIndication(ss);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }
}

void ImsRadioImpl::notifyOnRTTMessage(std::shared_ptr<QcRilUnsolImsRttMessage> msg) {
  if (msg) {
    string rttMsg = msg->getRttMessage();
    hidl_string out;

    if (rttMsg.empty()) {
      QCRIL_LOG_ERROR("Invalid rtt message");
      return;
    }
    out = rttMsg.c_str();
    sp<IImsRadioIndication> indCb = getIndicationCallback();
    if (indCb) {
      imsRadiolog("<", "onRttMessageReceived: message=" + rttMsg);
      Return<void> ret = indCb->onRttMessageReceived(out);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  } else {
    QCRIL_LOG_ERROR("Invalid rtt message");
  }
}

void ImsRadioImpl::notifyOnVowifiCallQuality(
    std::shared_ptr<QcRilUnsolImsVowifiCallQuality> msg) {
  if (msg) {
    VoWiFiCallQualityInfo voWiFiCallQuality;
    convertVowifiCallQualityInfo(voWiFiCallQuality, msg->getCallQuality());
    sp<IImsRadioIndication> indCb = getIndicationCallback();
    if (indCb) {
      imsRadiolog("<", "onVoWiFiCallQuality: quality=" + toString(voWiFiCallQuality));
      Return<void> ret = indCb->onVoWiFiCallQuality(voWiFiCallQuality);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }
}

void ImsRadioImpl::notifyOnVopsChanged(std::shared_ptr<QcRilUnsolImsVopsIndication> msg) {
  if (msg) {
    sp<V1_2::IImsRadioIndication> indCb = getIndicationCallbackV1_2();
    if (indCb) {
      imsRadiolog("<", "onVopsChanged: status=" + std::to_string(msg->getVopsStatus()));
      Return<void> ret = indCb->onVopsChanged(msg->getVopsStatus());
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }
}

void ImsRadioImpl::notifyOnSubConfigChanged(
    std::shared_ptr<QcRilUnsolImsSubConfigIndication> msg) {
  if (msg) {
    ImsSubConfigInfo config = ImsSubConfigInfo{};
    std::shared_ptr<qcril::interfaces::ImsSubConfigInfo> data = msg->getImsSubConfig();
    if (data != nullptr) {
      convertImsSubConfigInfo(config, *data);
    }
    sp<IImsRadioIndication> indCb = getIndicationCallback();
    if (indCb) {
      imsRadiolog("<", "notifyOnSubConfigChanged: status=" + toString(config));
      Return<void> ret = indCb->onImsSubConfigChanged(config);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }
}

void ImsRadioImpl::notifyOnGeoLocationChange(
    std::shared_ptr<QcRilUnsolImsGeoLocationInfo> msg) {
  if (msg) {
    if (!msg->hasLatitude() || !msg->hasLongitude()) {
      QCRIL_LOG_ERROR("Geolocation indication has no coordinates!");
      return;
    }
    sp<IImsRadioIndication> indCb = getIndicationCallback();
    if (indCb) {
      imsRadiolog("<", "onGeolocationInfoRequested: lat = " + std::to_string(msg->getLatitude()) +
                  "lon = " + std::to_string(msg->getLongitude()));
      Return<void> ret = indCb->onGeolocationInfoRequested(msg->getLatitude(), msg->getLongitude());
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }
}

void ImsRadioImpl::notifyOnServiceStatusChange(
    std::shared_ptr<QcRilUnsolImsSrvStatusIndication> msg) {
  if (msg) {
    std::shared_ptr<qcril::interfaces::ServiceStatusInfoList> data = msg->getServiceStatus();
    hidl_vec<ServiceStatusInfo> srvStatusList;
    if (data != nullptr) {
      convertServiceStatusInfoList(srvStatusList, *data);
      sp<IImsRadioIndication> indCb = getIndicationCallback();
      if (indCb) {
        imsRadiolog("<", "onServiceStatusChanged: srvStatusList=" + toString(srvStatusList));
        Return<void> ret = indCb->onServiceStatusChanged(srvStatusList);
        if (!ret.isOk()) {
          QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
        }
      }
    }
  }
}

void ImsRadioImpl::notifyOnRegBlockStatusChange(
    std::shared_ptr<QcRilUnsolImsRegBlockStatusMessage> msg) {
  if (msg) {
    std::shared_ptr<qcril::interfaces::RegistrationBlockStatus> data =
        msg->getBlockStatus();
    if (data == nullptr) {
        QCRIL_LOG_ERROR("indication is null.");
        return;
    }
    bool hasBlockStatusOnWwan = false;
    BlockStatus blockStatusOnWwan = {};
    bool hasBlockStatusOnWlan = false;
    BlockStatus blockStatusOnWlan = {};

    if (data->hasBlockStatusOnWwan()) {
      hasBlockStatusOnWwan = true;
      convertRegBlockStatus(blockStatusOnWwan, *(data->getBlockStatusOnWwan()));
    }
    if (data->hasBlockStatusOnWlan()) {
      hasBlockStatusOnWlan = true;
      convertRegBlockStatus(blockStatusOnWlan, *(data->getBlockStatusOnWlan()));
    }

    sp<IImsRadioIndication> indCb = getIndicationCallback();
    if (indCb) {
      imsRadiolog("<", std::string("onRegistrationBlockStatus: wwan(") +
                           (hasBlockStatusOnWwan ? "true" : "false") +
                           ")=" + toString(blockStatusOnWwan) + "wlan(" +
                           (hasBlockStatusOnWlan ? "true" : "false") +
                           ")=" + toString(blockStatusOnWlan));
      Return<void> ret = indCb->onRegistrationBlockStatus(hasBlockStatusOnWwan, blockStatusOnWwan,
                                                          hasBlockStatusOnWlan, blockStatusOnWlan);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }
}

void ImsRadioImpl::notifyOnSsacInfoChange(
    std::shared_ptr<QcRilUnsolImsSsacInfoIndication> msg) {
  if (msg) {
    //TODO not using this message
  }
}

void ImsRadioImpl::notifyIncomingImsSms(std::shared_ptr<RilUnsolIncomingImsSMSMessage> msg) {
    if (msg) {
        sp<V1_2::IImsRadioIndication> indCb1_2 = getIndicationCallbackV1_2();
        if (indCb1_2) {
            V1_2::IncomingImsSms sms;
            auto tech = msg->getTech();
            if (tech == RADIO_TECH_3GPP) {
                auto& payload = msg->getGsmPayload();
                sms = {"3gpp", payload, convertVerificationStatus(
                        msg->getVerificationStatus())};
            } else {
                std::vector<uint8_t> payload;
                (void)convertCdmaFormatToPseudoPdu(msg->getCdmaPayload(), payload);
                sms = {"3gpp2", payload, V1_2::VerificationStatus::STATUS_VALIDATION_NONE};
            }
            imsRadiolog("<", "onIncomingImsSms: sms = " + toString(sms));
            Return<void> ret = indCb1_2->onIncomingImsSms(sms);
            if (!ret.isOk()) {
                QCRIL_LOG_ERROR("Unable to send response. Exception : %s",
                        ret.description().c_str());
            }
        } else {
            QCRIL_LOG_ERROR("V1_2 indication cb is null");
        }
    }
}

void ImsRadioImpl::notifyNewImsSmsStatusReport(
        std::shared_ptr<RilUnsolNewImsSmsStatusReportMessage> msg) {
    if (msg) {
        sp<V1_2::IImsRadioIndication> indCb1_2 = getIndicationCallbackV1_2();
        if (indCb1_2) {
            V1_2::ImsSmsSendStatusReport report;
            auto tech = msg->getRadioTech();
            if (tech == RADIO_TECH_3GPP) {
                auto& payload = msg->getGsmPayload();
                report = {msg->getMessageRef(), "3gpp", payload};
            } else {
                std::vector<uint8_t> payload;
                (void)convertCdmaFormatToPseudoPdu(msg->getCdmaPayload(), payload);
                report = {msg->getMessageRef(), "3gpp2", payload};
            }
            imsRadiolog("<", "onImsSmsStatusReport: report = " + toString(report));
            Return<void> ret = indCb1_2->onImsSmsStatusReport(report);
            if (!ret.isOk()) {
                QCRIL_LOG_ERROR("Unable to send response. Exception : %s",
                        ret.description().c_str());
            }
        } else {
            QCRIL_LOG_ERROR("V1_2 indication cb is null");
        }
    }
}

void ImsRadioImpl::notifyOnPendingMultiIdentityStatus(
    std::shared_ptr<QcRilUnsolImsPendingMultiLineStatus> msg) {
  (void)msg;
  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_LOG_ERROR("should not be called on this version");
  QCRIL_LOG_FUNC_RETURN();
}

void ImsRadioImpl::notifyOnMultiIdentityLineStatus(
    std::shared_ptr<QcRilUnsolImsMultiIdentityStatusMessage> msg) {
  (void)msg;
  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_LOG_ERROR("should not be called on this version");
  QCRIL_LOG_FUNC_RETURN();
}

void ImsRadioImpl::notifyOnVoiceInfoStatusChange(
    std::shared_ptr<QcRilUnsolImsVoiceInfo> msg) {
  if (msg) {
    V1_3::VoiceInfo voiceInfo = convertToHidlVoiceInfo(msg->getVoiceInfo());
    sp<V1_3::IImsRadioIndication> indCb = getIndicationCallbackV1_3();
    if (indCb) {
      imsRadiolog("<", "onVoiceInfoChanged: voiceInfo" + toString(voiceInfo));
      Return<void> ret = indCb->onVoiceInfoChanged(voiceInfo);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }
}

void ImsRadioImpl::notifyOnExitEcbmIndication(
    std::shared_ptr<QcRilUnsolImsExitEcbmIndication> msg) {
  if (msg) {
    sp<IImsRadioIndication> indCb = getIndicationCallback();
    if (indCb) {
      imsRadiolog("<", "onExitEmergencyCallBackMode");
      Return<void> ret = indCb->onExitEmergencyCallBackMode();
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }
}

void ImsRadioImpl::notifyOnEnterEcbmIndication(
    std::shared_ptr<QcRilUnsolImsEnterEcbmIndication> msg) {
  if (msg) {
    sp<IImsRadioIndication> indCb = getIndicationCallback();
    if (indCb) {
      imsRadiolog("<", "onEnterEmergencyCallBackMode");
      Return<void> ret = indCb->onEnterEmergencyCallBackMode();
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }
}

void ImsRadioImpl::notifyOnRadioStateChanged(
    std::shared_ptr<QcRilUnsolImsRadioStateIndication> msg) {
  if (msg) {
    RadioState state = convertToHidlRadioState(msg->getRadioState());
    sp<IImsRadioIndication> indCb = getIndicationCallback();
    if (indCb) {
      imsRadiolog("<", "onRadioStateChanged: state=" + toString(state));
      Return<void> ret = indCb->onRadioStateChanged(state);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }
}

void ImsRadioImpl::notifyOnMessageWaiting(
    std::shared_ptr<QcRilUnsolMessageWaitingInfoMessage> msg) {
  if (msg) {
    MessageWaitingIndication mwi = {};

    if (msg->hasMessageSummary() && !msg->getMessageSummary().empty()) {
      auto msgSummary = msg->getMessageSummary();
      mwi.messageSummary.resize(msgSummary.size());
      for (int i = 0; i < msgSummary.size(); i++) {
        mwi.messageSummary[i].type = msgSummary[i].hasMessageType()
                                         ? convertMessageType(msgSummary[i].getMessageType())
                                         : MessageType::MWI_MSG_INVALID;
        mwi.messageSummary[i].type = msgSummary[i].hasMessageType()
                                         ? convertMessageType(msgSummary[i].getMessageType())
                                         : MessageType::MWI_MSG_INVALID;
        mwi.messageSummary[i].newMessageCount =
            msgSummary[i].hasNewMessageCount() ? msgSummary[i].getNewMessageCount() : INT32_MAX;
        mwi.messageSummary[i].oldMessageCount =
            msgSummary[i].hasOldMessageCount() ? msgSummary[i].getOldMessageCount() : INT32_MAX;
        mwi.messageSummary[i].newUrgentMessageCount = msgSummary[i].hasNewUrgentMessageCount()
                                                          ? msgSummary[i].getNewUrgentMessageCount()
                                                          : INT32_MAX;
        mwi.messageSummary[i].oldUrgentMessageCount = msgSummary[i].hasOldUrgentMessageCount()
                                                          ? msgSummary[i].getOldUrgentMessageCount()
                                                          : INT32_MAX;
      }
    }
    if (msg->hasUeAddress() && !msg->getUeAddress().empty()) {
      mwi.ueAddress = msg->getUeAddress().c_str();
    }
    if (msg->hasMessageDetails() && !msg->getMessageDetails().empty()) {
      auto msgDetails = msg->getMessageDetails();
      mwi.messageDetails.resize(msgDetails.size());
      for (int i = 0; i < msgDetails.size(); i++) {
        if (msgDetails[i].hasToAddress() && !msgDetails[i].getToAddress().empty()) {
          mwi.messageDetails[i].toAddress = msgDetails[i].getToAddress().c_str();
        }
        if (msgDetails[i].hasFromAddress() && !msgDetails[i].getFromAddress().empty()) {
          mwi.messageDetails[i].fromAddress = msgDetails[i].getFromAddress().c_str();
        }
        if (msgDetails[i].hasSubject() && !msgDetails[i].getSubject().empty()) {
          mwi.messageDetails[i].subject = msgDetails[i].getSubject().c_str();
        }
        if (msgDetails[i].hasDate() && !msgDetails[i].getDate().empty()) {
          mwi.messageDetails[i].date = msgDetails[i].getDate().c_str();
        }
        mwi.messageDetails[i].priority = msgDetails[i].hasPriority()
                                             ? convertMessagePriority(msgDetails[i].getPriority())
                                             : MessagePriority::MWI_MSG_PRIORITY_INVALID;
        if (msgDetails[i].hasMessageId() && !msgDetails[i].getMessageId().empty()) {
          mwi.messageDetails[i].id = msgDetails[i].getMessageId().c_str();
        }
        mwi.messageDetails[i].type = msgDetails[i].hasMessageType()
                                         ? convertMessageType(msgDetails[i].getMessageType())
                                         : MessageType::MWI_MSG_INVALID;
      }
    }

    sp<IImsRadioIndication> indCb = getIndicationCallback();
    if (indCb != nullptr) {
      imsRadiolog("<", "onMessageWaiting: mwi = " + toString(mwi));
      Return<void> ret = indCb->onMessageWaiting(mwi);
      if (!ret.isOk()) {
        QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
      }
    }
  }
}

Return<void> ImsRadioImpl::getImsRegistrationState(int32_t token) {
  imsRadiolog(">", "getImsRegistrationState: token = " + std::to_string(token));
  QCRIL_LOG_FUNC_ENTRY();
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
      [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                    std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
        uint32_t errorCode = RIL_E_GENERIC_FAILURE;
        std::shared_ptr<qcril::interfaces::Registration> respData = nullptr;
        (void)msg;
        if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
          errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
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
void ImsRadioImpl::sendGetRegistrationResponse(
    int32_t token, uint32_t errorCode, std::shared_ptr<qcril::interfaces::Registration> data) {
  RegistrationInfo reg = {.state = RegState::INVALID, .errorCode=INT32_MAX,
                          .radioTech = RadioTechType::RADIO_TECH_INVALID};
  if (data != nullptr) {
    convertRegistrationInfo(reg, *data);
  } else {
    QCRIL_LOG_INFO("data is nullptr");
  }
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "getRegistrationResponse: token = " + std::to_string(token) +
                         " errorCode = " + std::to_string(errorCode) + " reg = " + toString(reg));
    Return<void> ret = respCb->getRegistrationResponse(token, errorCode, reg);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

/*
 *  @brief
 *  Server side implementtaion of the IImsradio service's dial API
 *
 */
Return<void> ImsRadioImpl::dial(int32_t token, const DialRequest &dialRequest) {
  imsRadiolog(">",
              "dial: token = " + std::to_string(token) + " dialRequest = " + toString(dialRequest));
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestImsDialMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    msg->setIsImsRequest(true);
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
      msg->setCallType(convertCallType(dialRequest.callDetails.callType));
      msg->setCallDomain(convertCallDomain(dialRequest.callDetails.callDomain));
      msg->setRttMode(convertRttMode(dialRequest.callDetails.rttMode));
      if (dialRequest.callDetails.extras.size()) {
        std::string displayText = getExtra("DisplayText", dialRequest.callDetails.extras);
        if (!displayText.empty()) {
          msg->setDisplayText(displayText);
        }
        std::string retryCallFailReason =
            getExtra("RetryCallFailReason", dialRequest.callDetails.extras);
        if (!retryCallFailReason.empty()) {
          msg->setRetryCallFailReason(convertCallFailCause(
              static_cast<V1_3::CallFailCause>(std::stoi(retryCallFailReason))));
        }
        std::string retryCallFailMode =
            getExtra("RetryCallFailRadioTech", dialRequest.callDetails.extras);
        if (!retryCallFailMode.empty()) {
          msg->setRetryCallFailMode(
              convertRadioTechType(static_cast<RadioTechType>(std::stoi(retryCallFailMode))));
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

    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
          }
          sendDialResponse(token, errorCode);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendDialResponse(token, RIL_E_GENERIC_FAILURE);
  }
  return Void();
}

void ImsRadioImpl::sendDialResponse(int32_t token, uint32_t errorCode) {
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "dialResponse: token = " + std::to_string(token) +
                         " errorCode = " + std::to_string(errorCode));
    Return<void> ret = respCb->dialResponse(token, errorCode);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

Return<void> ImsRadioImpl::answer(int32_t token, CallType callType, IpPresentation presentation,
        RttMode rttMode) {
  imsRadiolog(">", "answer: token = " + std::to_string(token) + " callType = " +
                       toString(callType) + " presentation = " + toString(presentation));
  QCRIL_LOG_FUNC_ENTRY();
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestImsAnswerMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    // Set parameters
    msg->setCallType(convertCallType(callType));
    if (presentation != IpPresentation::IP_PRESENTATION_INVALID) {
      msg->setPresentation(convertIpPresentation(presentation));
    }
    msg->setRttMode(convertRttMode(rttMode));

    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
          }
          sendAnswerResponse(token, errorCode);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);

  if (sendFailure) {
    sendAnswerResponse(token, RIL_E_GENERIC_FAILURE);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

void ImsRadioImpl::sendAnswerResponse(int32_t token, uint32_t errorCode) {
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "answerResponse: token = " + std::to_string(token) +
                         " errorCode = " + std::to_string(errorCode));
    Return<void> ret = respCb->answerResponse(token, errorCode);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

Return<void> ImsRadioImpl::hangup(int32_t token, const HangupRequestInfo &hangup) {
  imsRadiolog(">", "hangup: token = " + std::to_string(token) + " hangup = " + toString(hangup));
  V1_1::HangupRequestInfo v11Hangup;
  migrateHangup(hangup, v11Hangup);
  return hangup_1_1(token, v11Hangup);
}

Return<void> ImsRadioImpl::hangup_1_1(int32_t token, const V1_1::HangupRequestInfo &hangup) {
  imsRadiolog(">",
              "hangup_1_1: token = " + std::to_string(token) + " hangup = " + toString(hangup));
  V1_2::HangupRequestInfo v12Hangup;
  migrateHangup(hangup, v12Hangup);
  return hangup_1_2(token, v12Hangup);
}

Return<void> ImsRadioImpl::hangup_1_2(int32_t token, const V1_2::HangupRequestInfo &hangup) {
  imsRadiolog(">",
              "hangup_1_2: token = " + std::to_string(token) + " hangup = " + toString(hangup));
  V1_3::HangupRequestInfo v13Hangup;
  migrateHangup(hangup, v13Hangup);
  return hangup_1_3(token, v13Hangup);
}

Return<void> ImsRadioImpl::hangup_1_3(int32_t token, const V1_3::HangupRequestInfo &hangup) {
  QCRIL_LOG_FUNC_ENTRY();
  imsRadiolog(">",
              "hangup_1_3: token = " + std::to_string(token) + " hangup = " + toString(hangup));
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestImsHangupMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    // Set parameters
    if (hangup.connIndex != INT32_MAX) {
      msg->setCallIndex(hangup.connIndex);
    }
    if (hangup.hasMultiParty) {
      msg->setIsMultiParty(hangup.multiParty);
    }
    if (hangup.connUri.size()) {
      msg->setConnectionUri(hangup.connUri.c_str());
    }
    if (hangup.conf_id != INT32_MAX) {
      msg->setConferenceId(hangup.conf_id);
    }
    if (hangup.hasFailCauseResponse) {
      if (hangup.failCauseResponse.failCause != V1_3::CallFailCause::CALL_FAIL_INVALID) {
        msg->setRejectCause(convertCallFailCause(hangup.failCauseResponse.failCause));
      }
      const uint32_t errorInfoLen = hangup.failCauseResponse.errorinfo.size();
      if (errorInfoLen > 0) {
        std::string errorInfo((const char *)hangup.failCauseResponse.errorinfo.data(),
                              errorInfoLen);
        msg->setRejectCauseRaw(std::stoul(errorInfo));
      }
    }

    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
          }
          sendHangupResponse(token, errorCode);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);

  if (sendFailure) {
    sendHangupResponse(token, RIL_E_GENERIC_FAILURE);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}
void ImsRadioImpl::sendHangupResponse(int32_t token, uint32_t errorCode) {
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "hangupResponse: token = " + std::to_string(token) +
                         " errorCode = " + std::to_string(errorCode));
    Return<void> ret = respCb->hangupResponse(token, errorCode);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

Return<void> ImsRadioImpl::requestRegistrationChange(int32_t token, RegState state) {
  imsRadiolog(">", "requestRegistrationChange: token = " + std::to_string(token) +
                       " state = " + toString(state));
  QCRIL_LOG_FUNC_ENTRY();
  bool sendFailure = false;
  auto ctx = getContext(token);
  qcril::interfaces::RegState regState = convertRegState(state);
  do {
    if (regState == qcril::interfaces::RegState::UNKNOWN) {
      QCRIL_LOG_ERROR("Invalid parameters");
      sendFailure = true;
      break;
    }
    std::shared_ptr<QcRilRequestImsRegistrationChangeMessage> msg =
        std::make_shared<QcRilRequestImsRegistrationChangeMessage>(ctx, convertRegState(state));
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
          }
          sendRequestRegistrationChangeResponse(token, errorCode, nullptr);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);

  if (sendFailure) {
    sendRequestRegistrationChangeResponse(token, RIL_E_GENERIC_FAILURE, nullptr);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

void ImsRadioImpl::sendRequestRegistrationChangeResponse(int32_t token, uint32_t errorCode,
        std::shared_ptr<qcril::interfaces::BasePayload> data) {
  (void)data;
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "requestRegistrationChangeResponse: token = " + std::to_string(token) +
                         " errorCode = " + std::to_string(errorCode));
    Return<void> ret = respCb->requestRegistrationChangeResponse(token, errorCode);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

Return<void> ImsRadioImpl::queryServiceStatus(int32_t token) {
  imsRadiolog(">", "queryServiceStatus: token = " + std::to_string(token));
  QCRIL_LOG_FUNC_ENTRY();
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
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          std::shared_ptr<qcril::interfaces::ServiceStatusInfoList> respData = nullptr;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
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

void ImsRadioImpl::sendQueryServiceStatusResponse(
    int32_t token, uint32_t errorCode,
    std::shared_ptr<qcril::interfaces::ServiceStatusInfoList> data) {
  hidl_vec<ServiceStatusInfo> srvStatusList;
  if (data) {
    convertServiceStatusInfoList(srvStatusList, *data);
  }
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "queryServiceStatusResponse: token = " + std::to_string(token) +
                         " errorCode = " + std::to_string(errorCode) +
                         " srvStatusList = " + toString(srvStatusList));
    Return<void> ret = respCb->queryServiceStatusResponse(token, errorCode, srvStatusList);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

void ImsRadioImpl::sendSetServiceStatusMessageResponse(uint32_t token, uint32_t errorCode,
  std::shared_ptr<qcril::interfaces::BasePayload> data) {
  (void)data;
  sp<IImsRadioResponse> respCb = getResponseCallback();
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

Return<void> ImsRadioImpl::setServiceStatus(int32_t token,
        const ServiceStatusInfo& srvStatusInfo) {
  imsRadiolog(">", "setServiceStatus: token = " + std::to_string(token) +
                       " srvStatusInfo=" + toString(srvStatusInfo));
  bool sendFailure = false;
  auto ctx = getContext(token);

  do {
    std::shared_ptr<QcRilRequestImsSetServiceStatusMessage> msg =
      std::make_shared<QcRilRequestImsSetServiceStatusMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    auto networkMode = RadioTechType::RADIO_TECH_INVALID;
    auto status = StatusType::STATUS_INVALID;
    if (srvStatusInfo.accTechStatus.size()) {
      networkMode = srvStatusInfo.accTechStatus[0].networkMode;
      status = srvStatusInfo.accTechStatus[0].status;
    }
    if ((srvStatusInfo.callType == CallType::CALL_TYPE_VOICE) &&
        (networkMode == RadioTechType::RADIO_TECH_LTE)) {
      msg->setVolteEnabled(status != StatusType::STATUS_DISABLED);
    }
    if ((srvStatusInfo.callType == CallType::CALL_TYPE_VOICE) &&
        ((networkMode == RadioTechType::RADIO_TECH_IWLAN) ||
         (networkMode == RadioTechType::RADIO_TECH_WIFI))) {
      msg->setWifiCallingEnabled(status != StatusType::STATUS_DISABLED);
    }
    if (srvStatusInfo.callType == CallType::CALL_TYPE_VT) {
      msg->setVideoTelephonyEnabled(status != StatusType::STATUS_DISABLED);
    }
    if (srvStatusInfo.callType == CallType::CALL_TYPE_UT) {
      msg->setUTEnabled(status != StatusType::STATUS_DISABLED);
    }

    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
      [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                    std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
        uint32_t errorCode = RIL_E_GENERIC_FAILURE;
        (void)msg;
        if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
          errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
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

Return<void> ImsRadioImpl::hold(int32_t token, uint32_t callId) {
  imsRadiolog(">",
              "hold: token = " + std::to_string(token) + " callId = " + std::to_string(callId));
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestImsHoldMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    msg->setCallId(callId);
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          std::shared_ptr<qcril::interfaces::SipErrorInfo> errorDetails = nullptr;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
            if (resp->data != nullptr) {
              errorDetails = std::static_pointer_cast<qcril::interfaces::SipErrorInfo>(resp->data);
            }
          }
          sendHoldResponse(token, errorCode, errorDetails);
        });

    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendHoldResponse(token, RIL_E_GENERIC_FAILURE, nullptr);
  }
  return Void();
}

void ImsRadioImpl::sendHoldResponse(int32_t token, uint32_t errorCode,
                                    std::shared_ptr<qcril::interfaces::SipErrorInfo> data) {
  SipErrorInfo errorDetails = {.errorCode = INT32_MAX};
  if (data) {
    convertSipErrorInfo(errorDetails, data);
  }
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "holdResponse: token = " + std::to_string(token) + " errorCode = " +
                         std::to_string(errorCode) + " errorDetails = " + toString(errorDetails));
    Return<void> ret = respCb->holdResponse(token, errorCode, errorDetails);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

Return<void> ImsRadioImpl::resume(int32_t token, uint32_t callId) {
  imsRadiolog(">",
              "resume: token = " + std::to_string(token) + " callId = " + std::to_string(callId));
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestImsResumeMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    msg->setCallId(callId);
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          std::shared_ptr<qcril::interfaces::SipErrorInfo> errorDetails = nullptr;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
            if (resp->data != nullptr) {
              errorDetails = std::static_pointer_cast<qcril::interfaces::SipErrorInfo>(resp->data);
            }
          }
          sendResumeResponse(token, errorCode, errorDetails);
        });

    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendResumeResponse(token, RIL_E_GENERIC_FAILURE, nullptr);
  }
  return Void();
}

void ImsRadioImpl::sendResumeResponse(int32_t token, uint32_t errorCode,
                                      std::shared_ptr<qcril::interfaces::SipErrorInfo> data) {
  SipErrorInfo errorDetails = {.errorCode = INT32_MAX};
  if (data) {
    convertSipErrorInfo(errorDetails, data);
  }
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "resumeResponse: token = " + std::to_string(token) + " errorCode = " +
                         std::to_string(errorCode) + " errorDetails = " + toString(errorDetails));
    Return<void> ret = respCb->resumeResponse(token, errorCode, errorDetails);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

void ImsRadioImpl::sendSetConfigMessageResponse(uint32_t token, uint32_t errorCode,
  std::shared_ptr<qcril::interfaces::ConfigInfo> data) {
  ConfigInfo config = ConfigInfo{ .item=V1_0::ConfigItem::CONFIG_ITEM_INVALID,
                                  .intValue=INT32_MAX,
                                  .errorCause=V1_0::ConfigFailureCause::CONFIG_FAILURE_INVALID };
  if (data) {
      convertConfigInfo(config, *(data));
  }
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "setConfigResponse: token = " + std::to_string(token) + " errorCode = " +
                         std::to_string(errorCode) + " config = " + toString(config));
    Return<void> retStatus = respCb->setConfigResponse(token, errorCode, config);
    if (!retStatus.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s",
            retStatus.description().c_str());
    }
  }
}

Return<void> ImsRadioImpl::setConfig(int32_t token, const ConfigInfo& inConfig) {
  imsRadiolog(">", "setConfig: token = " + std::to_string(token) + " config=" + toString(inConfig));
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
    readConfigInfo(inConfig, msg->getConfigInfo());
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
      [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                    std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
        uint32_t errorCode = RIL_E_GENERIC_FAILURE;
        (void)msg;
        std::shared_ptr<qcril::interfaces::ConfigInfo> respData = nullptr;
        if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
          errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
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
void ImsRadioImpl::sendGetConfigMessageResponse(uint32_t token, uint32_t errorCode,
  std::shared_ptr<qcril::interfaces::ConfigInfo> data) {
  ConfigInfo config = ConfigInfo{ .item=V1_0::ConfigItem::CONFIG_ITEM_INVALID,
                                  .intValue=INT32_MAX,
                                  .errorCause=V1_0::ConfigFailureCause::CONFIG_FAILURE_INVALID };
  if (data) {
    convertConfigInfo(config, (*data));
  }
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "getConfigResponse: token = " + std::to_string(token) + " errorCode = " +
                         std::to_string(errorCode) + " config = " + toString(config));
    Return<void> retStatus = respCb->getConfigResponse(token, errorCode, config);
    if (!retStatus.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s",
              retStatus.description().c_str());
    }
  }
}

Return<void> ImsRadioImpl::getConfig(int32_t token, const ConfigInfo& inConfig) {
  imsRadiolog(">", "getConfig: token=" + std::to_string(token) + "config=" + toString(inConfig));
  bool sendFailure = false;
  auto ctx = getContext(token);
  do {
    std::shared_ptr<QcRilRequestImsGetConfigMessage> msg =
        std::make_shared<QcRilRequestImsGetConfigMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    readConfigInfo(inConfig, msg->getConfigInfo());
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
      [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                    std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
        uint32_t errorCode = RIL_E_GENERIC_FAILURE;
        (void)msg;
        std::shared_ptr<qcril::interfaces::ConfigInfo> respData = nullptr;
        if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
          errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
          respData = std::static_pointer_cast<qcril::interfaces::ConfigInfo>(resp->data);
        }
        sendGetConfigMessageResponse(token, errorCode, respData);
      });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendGetConfigMessageResponse(token, RIL_E_GENERIC_FAILURE, nullptr);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> ImsRadioImpl::queryCallForwardStatus(int32_t token,
                                                  const CallForwardInfo &callForwardInfo) {
  imsRadiolog(">", "queryCallForwardStatus: token = " + std::to_string(token) +
                       " callForwardInfo = " + toString(callForwardInfo));
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestQueryCallForwardMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    msg->setIsImsRequest(true);
    // Set parameters
    if (callForwardInfo.reason != INT32_MAX) {
      msg->setReason(callForwardInfo.reason);
    }
    if (callForwardInfo.serviceClass != INT32_MAX) {
      msg->setServiceClass(callForwardInfo.serviceClass);
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          std::shared_ptr<qcril::interfaces::GetCallForwardRespData> data = nullptr;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
            data = std::static_pointer_cast<qcril::interfaces::GetCallForwardRespData>(resp->data);
          }
          sendQueryCallForwardStatusResponse(token, errorCode, data);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendQueryCallForwardStatusResponse(token, RIL_E_GENERIC_FAILURE, nullptr);
  }
  return Void();
}

void ImsRadioImpl::sendQueryCallForwardStatusResponse(
    int32_t token, uint32_t errorCode,
    const std::shared_ptr<qcril::interfaces::GetCallForwardRespData> data) {
  hidl_vec<CallForwardInfo> outCfInfoList;
  SipErrorInfo errorDetails = {.errorCode = INT32_MAX};

  if (data) {
    if (!data->getCallForwardInfo().empty()) {
      convertCallForwardInfoList(outCfInfoList, data->getCallForwardInfo());
    }
    if (data->hasErrorDetails()) {
      convertSipErrorInfo(errorDetails, data->getErrorDetails());
    }
  }

  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "queryCallForwardStatusResponse: token = " + std::to_string(token) +
                         " errorCode = " + std::to_string(errorCode) + " outCfInfoList = " +
                         toString(outCfInfoList) + " errorDetails = " + toString(errorDetails));
    Return<void> ret =
        respCb->queryCallForwardStatusResponse(token, errorCode, outCfInfoList, errorDetails);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

Return<void> ImsRadioImpl::setCallForwardStatus(int32_t token,
                                                const CallForwardInfo &callForwardInfo) {
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
      convertCallFwdTimerInfo(callFwdTimerStart, callForwardInfo.callFwdTimerStart);
      msg->setCallFwdTimerStart(callFwdTimerStart);
    }

    if (callForwardInfo.hasCallFwdTimerEnd) {
      std::shared_ptr<qcril::interfaces::CallFwdTimerInfo> callFwdTimerEnd =
          std::make_shared<qcril::interfaces::CallFwdTimerInfo>();
      convertCallFwdTimerInfo(callFwdTimerEnd, callForwardInfo.callFwdTimerEnd);
      msg->setCallFwdTimerEnd(callFwdTimerEnd);
    }

    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          std::shared_ptr<qcril::interfaces::SetCallForwardRespData> setCfResp = nullptr;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
            setCfResp = std::static_pointer_cast<qcril::interfaces::SetCallForwardRespData>(resp->data);
          }
          auto data = std::make_shared<qcril::interfaces::SuppServiceStatusInfo>();
          if (data && setCfResp && setCfResp->hasErrorDetails() && setCfResp->getErrorDetails()) {
            data->setErrorDetails(setCfResp->getErrorDetails());
          }
          sendSuppServiceStatusResponse(token, errorCode, data);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendSuppServiceStatusResponse(token, RIL_E_GENERIC_FAILURE, nullptr);
  }
  return Void();
}
void ImsRadioImpl::sendSuppServiceStatusResponse(
    int32_t token, uint32_t errorCode,
    const std::shared_ptr<qcril::interfaces::SuppServiceStatusInfo> data) {
  Return<void> ret = Void();
  V1_3::SuppServiceStatus suppServiceStatusV1_3 = {
      .status = ServiceClassStatus::INVALID,
      .provisionStatus = V1_3::ServiceClassProvisionStatus::INVALID,
      .facilityType = FacilityType::FACILITY_INVALID,
      .hasErrorDetails = false};

  if (data) {
    convertSuppServiceStatus(suppServiceStatusV1_3, data);
  }

  sp<V1_3::IImsRadioResponse> respCbV1_3 = getResponseCallbackV1_3();
  if (respCbV1_3 != nullptr) {
    imsRadiolog("<", "suppServiceStatusResponse_1_3: token = " + std::to_string(token) +
                         " errorCode = " + std::to_string(errorCode) +
                         " suppServiceStatus = " + toString(suppServiceStatusV1_3));
    ret = respCbV1_3->suppServiceStatusResponse_1_3(token, errorCode, suppServiceStatusV1_3);
  } else {
    sp<IImsRadioResponse> respCb = getResponseCallback();
    if (respCb != nullptr) {
      SuppServiceStatus suppServiceStatusV1_0 = {};
      convertSuppServiceStatusV13ToV10(suppServiceStatusV1_3, suppServiceStatusV1_0);
      imsRadiolog("<", "suppServiceStatusResponse: token = " + std::to_string(token) +
                           " errorCode = " + std::to_string(errorCode) +
                           " suppServiceStatus = " + toString(suppServiceStatusV1_0));
      ret = respCb->suppServiceStatusResponse(token, errorCode, suppServiceStatusV1_0);
    }
  }
  if (!ret.isOk()) {
    QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
  }
}

Return<void> ImsRadioImpl::getCallWaiting(int32_t token, uint32_t inServiceClass) {
  imsRadiolog(">", "getCallWaiting: token = " + std::to_string(token) +
                       " serviceClass = " + std::to_string(inServiceClass));
  QCRIL_LOG_FUNC_ENTRY();
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestGetCallWaitingMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    msg->setIsImsRequest(true);
    // Set parameters
    if (inServiceClass != INT32_MAX) {
      msg->setServiceClass(inServiceClass);
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          std::shared_ptr<qcril::interfaces::CallWaitingInfo> cwInfo = nullptr;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
            cwInfo = std::static_pointer_cast<qcril::interfaces::CallWaitingInfo>(resp->data);
          }
          sendGetCallWaitingResponse(token, errorCode, cwInfo);

        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendGetCallWaitingResponse(token, RIL_E_GENERIC_FAILURE, nullptr);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}
void ImsRadioImpl::sendGetCallWaitingResponse(
    int32_t token, uint32_t errorCode,
    const std::shared_ptr<qcril::interfaces::CallWaitingInfo> data) {
  SipErrorInfo errorDetails = {.errorCode = INT32_MAX};
  ServiceClassStatus serviceClassStatus = ServiceClassStatus::INVALID;
  uint32_t serviceClass = INT32_MAX;

  if (data) {
    if (data->hasStatus()) {
      serviceClassStatus = convertServiceClassStatus(data->getStatus());
    }
    if (data->hasErrorDetails()) {
      convertSipErrorInfo(errorDetails, data->getErrorDetails());
    }
    if (data->hasServiceClass()) {
      serviceClass = data->getServiceClass();
    }
  }

  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "getCallWaitingResponse: token = " + std::to_string(token) + " errorCode = " +
                         std::to_string(errorCode) + " status = " + toString(serviceClassStatus) +
                         " serviceClass = " + std::to_string(serviceClass) +
                         " errorDetails = " + toString(errorDetails));
    Return<void> ret = respCb->getCallWaitingResponse(token, errorCode, serviceClassStatus,
                                                            serviceClass, errorDetails);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

Return<void> ImsRadioImpl::setCallWaiting(int32_t token, ServiceClassStatus inStatus,
                                          uint32_t inServiceClass) {
  imsRadiolog(">", "setCallWaiting: token = " + std::to_string(token) +
                       " serviceClass = " + std::to_string(inServiceClass));
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestSetCallWaitingMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    msg->setIsImsRequest(true);
    // Set parameters
    if (inStatus != ServiceClassStatus::INVALID) {
      msg->setServiceStatus(convertServiceClassStatus(inStatus));
    }
    if (inServiceClass != INT32_MAX) {
      msg->setServiceClass(inServiceClass);
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          std::shared_ptr<qcril::interfaces::SipErrorInfo> errorDetails = nullptr;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
            errorDetails = std::static_pointer_cast<qcril::interfaces::SipErrorInfo>(resp->data);
          }
          auto data = std::make_shared<qcril::interfaces::SuppServiceStatusInfo>();
          if (data) {
            data->setErrorDetails(errorDetails);
          }
          sendSuppServiceStatusResponse(token, errorCode, data);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendSuppServiceStatusResponse(token, RIL_E_GENERIC_FAILURE, nullptr);
  }
  return Void();
}

Return<void> ImsRadioImpl::suppServiceStatus(int32_t token, int32_t operationType,
                                             FacilityType facilityType,
                                             const CbNumListInfo &cbNumListInfo) {
  imsRadiolog(">", "suppServiceStatus: token = " + std::to_string(token) + " operationType = " +
                       std::to_string(operationType) + " facilityType = " + toString(facilityType) +
                       " cbNumListInfo = " + toString(cbNumListInfo));
  std::shared_ptr<QcRilRequestMessage> msg = nullptr;
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    switch (operationType) {
      case 4:  // REGISTER
      case 5:  // ERASURE
        if (facilityType != FacilityType::FACILITY_BS_MT) {
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
        if (facilityType != FacilityType::FACILITY_INVALID) {
          setSupsMsg->setFacilityType(convertFacilityType(facilityType));
        }
        if (cbNumListInfo.serviceClass != INT32_MAX) {
          setSupsMsg->setServiceClass(cbNumListInfo.serviceClass);
        }
        if (facilityType == FacilityType::FACILITY_BS_MT) {
          if (cbNumListInfo.cbNumInfo.size() > 0) {
            std::vector<std::string> cbNumList;
            for (uint32_t i = 0; i < cbNumListInfo.cbNumInfo.size(); ++i) {
              std::string num = cbNumListInfo.cbNumInfo[i].number.c_str();
              cbNumList.push_back(num);
            }
            setSupsMsg->setCallBarringNumberList(cbNumList);
          }
        }
        GenericCallback<QcRilRequestMessageCallbackPayload> cb(
            [this, token](std::shared_ptr<Message> setSupsMsg, Message::Callback::Status status,
                          std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
              uint32_t errorCode = RIL_E_GENERIC_FAILURE;
              std::shared_ptr<qcril::interfaces::SipErrorInfo> errorDetails = nullptr;
              (void)setSupsMsg;
              if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
                errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
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
      } break;
      case 3:  // QUERY
        if (facilityType == FacilityType::FACILITY_COLP) {
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
                  errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
                  data = std::static_pointer_cast<qcril::interfaces::SuppServiceStatusInfo>(
                      resp->data);
                }
                sendSuppServiceStatusResponse(token, errorCode, data);
              });
          queryColp->setCallback(&cb);
          msg = queryColp;
        } else if (facilityType != FacilityType::FACILITY_CLIP) {
          // QcRilRequestGetCallBarringMessage
          auto queryMsg = std::make_shared<QcRilRequestGetCallBarringMessage>(ctx);
          if (queryMsg == nullptr) {
            QCRIL_LOG_ERROR("queryMsg is nullptr");
            sendFailure = true;
            break;
          }
          // Set parameters
          if (facilityType != FacilityType::FACILITY_INVALID) {
            queryMsg->setFacilityType(convertFacilityType(facilityType));
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
                  errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
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

Return<void> ImsRadioImpl::setSuppServiceNotification(int32_t token, ServiceClassStatus status) {
  imsRadiolog(">", "setSuppServiceNotification: token = " + std::to_string(token) +
                       " status = " + toString(status));
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestSetSuppSvcNotificationMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    // Set parameters
    if (status != ServiceClassStatus::INVALID) {
      msg->setStatus((status == ServiceClassStatus::ENABLED) ? true : false);
    } else {
      // TODO error code
      QCRIL_LOG_ERROR("Invalid parameter");
      sendFailure = true;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
          }
          sendSetSuppServiceNotificationResponse(token, errorCode);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendSetSuppServiceNotificationResponse(token, RIL_E_GENERIC_FAILURE);
  }
  return Void();
}
void ImsRadioImpl::sendSetSuppServiceNotificationResponse(int32_t token, uint32_t errorCode) {
  ServiceClassStatus status = ServiceClassStatus::INVALID;
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "setSuppServiceNotificationResponse: token = " + std::to_string(token) +
                         " errorCode = " + std::to_string(errorCode) +
                         " status = " + toString(status));
    Return<void> ret = respCb->setSuppServiceNotificationResponse(token, errorCode, status);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

Return<void> ImsRadioImpl::conference(int32_t token) {
  imsRadiolog(">", "conference: token = " + std::to_string(token));
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestImsConferenceMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          std::shared_ptr<qcril::interfaces::SipErrorInfo> rilErrorInfo = nullptr;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
            rilErrorInfo = std::static_pointer_cast<qcril::interfaces::SipErrorInfo>(resp->data);
          }
          sendConferenceResponse(token, errorCode, rilErrorInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendConferenceResponse(token, RIL_E_GENERIC_FAILURE, nullptr);
  }
  return Void();
}

void ImsRadioImpl::sendConferenceResponse(
    int32_t token, uint32_t errorCode,
    const std::shared_ptr<qcril::interfaces::SipErrorInfo> data) {
  SipErrorInfo errorInfo = {.errorCode = INT32_MAX};
  if (data) {
    convertSipErrorInfo(errorInfo, data);
  }
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "conferenceResponse: token = " + std::to_string(token) + " errorCode = " +
                         std::to_string(errorCode) + " errorInfo = " + toString(errorInfo));
    Return<void> ret = respCb->conferenceResponse(token, errorCode, errorInfo);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

Return<void> ImsRadioImpl::exitEmergencyCallbackMode(int32_t token) {
  imsRadiolog(">", "exitEmergencyCallbackMode: token = " + std::to_string(token));
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<RilRequestExitEmergencyCallbackMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
          }
          sendExitEmergencyCallbackModeResponse(token, errorCode);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendExitEmergencyCallbackModeResponse(token, RIL_E_GENERIC_FAILURE);
  }
  return Void();
}

void ImsRadioImpl::sendExitEmergencyCallbackModeResponse(int32_t token, uint32_t errorCode) {
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "exitEmergencyCallbackModeResponse: token = " + std::to_string(token) +
                         " errorCode = " + std::to_string(errorCode));
    Return<void> ret = respCb->exitEmergencyCallbackModeResponse(token, errorCode);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}


Return<void> ImsRadioImpl::getClip(int32_t token) {
  imsRadiolog(">", "getClip: token = " + std::to_string(token));
  QCRIL_LOG_FUNC_ENTRY();
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestGetClipMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    msg->setIsImsRequest(true);
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          std::shared_ptr<qcril::interfaces::ClipInfo> clipInfo = nullptr;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
            clipInfo = std::static_pointer_cast<qcril::interfaces::ClipInfo>(resp->data);
          }
          sendGetClipResponse(token, errorCode, clipInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendGetClipResponse(token, RIL_E_GENERIC_FAILURE, nullptr);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}
void ImsRadioImpl::sendGetClipResponse(int32_t token, uint32_t errorCode,
                                       const std::shared_ptr<qcril::interfaces::ClipInfo> data) {
  ClipProvisionStatus clipProvisionStatus = {};
  bool hasClipInfo = false;
  if (data) {
    hasClipInfo = convertClipInfo(clipProvisionStatus, data);
  }
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "getClipResponse: token = " + std::to_string(token) +
                         " errorCode = " + std::to_string(errorCode) +
                         " clipProvisionStatus = " + toString(clipProvisionStatus));
    Return<void> ret = respCb->getClipResponse(token, errorCode, clipProvisionStatus);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

Return<void> ImsRadioImpl::getClir(int32_t token) {
  imsRadiolog(">", "getClir: token = " + std::to_string(token));
  QCRIL_LOG_FUNC_ENTRY();
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestGetClirMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    msg->setIsImsRequest(true);
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          std::shared_ptr<qcril::interfaces::ClirInfo> clirInfo = nullptr;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
            clirInfo = std::static_pointer_cast<qcril::interfaces::ClirInfo>(resp->data);
          }
          sendGetClirResponse(token, errorCode, clirInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendGetClirResponse(token, RIL_E_GENERIC_FAILURE, nullptr);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}
void ImsRadioImpl::sendGetClirResponse(int32_t token, uint32_t errorCode,
                                       const std::shared_ptr<qcril::interfaces::ClirInfo> data) {
  ClirInfo clirInfo = {};
  bool hasClirInfo = false;
  if (data) {
    hasClirInfo = convertClirInfo(clirInfo, data);
  }
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "getClirResponse: token = " + std::to_string(token) + " errorCode = " +
                         std::to_string(errorCode) + " clirInfo = " + toString(clirInfo) +
                         " hasClirInfo = " + std::to_string(hasClirInfo));
    Return<void> ret = respCb->getClirResponse(token, errorCode, clirInfo, hasClirInfo);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

Return<void> ImsRadioImpl::setClir(int32_t token, const ClirInfo &clirInfo) {
  imsRadiolog(">",
              "setClir: token = " + std::to_string(token) + " clirInfo = " + toString(clirInfo));
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestSetClirMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    msg->setIsImsRequest(true);
    msg->setParamN(clirInfo.paramN);
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
          }
          sendSetClirResponse(token, errorCode);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendSetClirResponse(token, RIL_E_GENERIC_FAILURE);
  }
  return Void();
}
void ImsRadioImpl::sendSetClirResponse(int32_t token, uint32_t errorCode) {
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "setClirResponse: token = " + std::to_string(token) +
                         " errorCode = " + std::to_string(errorCode));
    Return<void> ret = respCb->setClirResponse(token, errorCode);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

Return<void> ImsRadioImpl::getColr(int32_t token) {
  imsRadiolog(">", "getColr: token = " + std::to_string(token));
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestGetColrMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    msg->setIsImsRequest(true);
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          std::shared_ptr<qcril::interfaces::ColrInfo> colrInfo = nullptr;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
            colrInfo = std::static_pointer_cast<qcril::interfaces::ColrInfo>(resp->data);
          }
          sendGetColrResponse(token, errorCode, colrInfo);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendGetColrResponse(token, RIL_E_GENERIC_FAILURE, nullptr);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

void ImsRadioImpl::sendGetColrResponse(int32_t token, uint32_t errorCode,
                                       const std::shared_ptr<qcril::interfaces::ColrInfo> data) {
  V1_3::ColrInfo colrInfoV1_3 = {};
  Return<void> ret = Void();
  bool hasColrInfo = false;
  if (data) {
    hasColrInfo = convertColrInfo(colrInfoV1_3, data);
  }
  sp<V1_3::IImsRadioResponse> respCbV1_3 = getResponseCallbackV1_3();
  if (respCbV1_3 != nullptr) {
    imsRadiolog("<", "getColrResponse_1_3: token = " + std::to_string(token) + " errorCode = " +
                         std::to_string(errorCode) + " colrInfo = " + toString(colrInfoV1_3));
    ret = respCbV1_3->getColrResponse_1_3(token, errorCode, colrInfoV1_3);
  } else {
    sp<IImsRadioResponse> respCb = getResponseCallback();
    if (respCb != nullptr) {
      ColrInfo colrInfoV1_0 = {};
      convertColrInfoV13ToV10(colrInfoV1_3, colrInfoV1_0);
      imsRadiolog("<", "getColrResponse: token = " + std::to_string(token) + " errorCode = " +
                           std::to_string(errorCode) + " colrInfo = " + toString(colrInfoV1_0));
      ret = respCb->getColrResponse(token, errorCode, colrInfoV1_0);
    }
  }

  if (!ret.isOk()) {
    QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
  }
}

Return<void> ImsRadioImpl::setColr(int32_t token, const ColrInfo &colrInfo) {
  imsRadiolog(">",
              "setColr: token = " + std::to_string(token) + " colrInfo = " + toString(colrInfo));
  V1_3::ColrInfo v13ColrInfo;
  migrateColrInfo(colrInfo, v13ColrInfo);
  return setColr_1_3(token, v13ColrInfo);
}

Return<void> ImsRadioImpl::setColr_1_3(int32_t token, const V1_3::ColrInfo& colrInfo) {
  imsRadiolog(">", "setColr_1_3: token = " + std::to_string(token) +
                   " colrInfo = " + toString(colrInfo));
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestSetColrMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    msg->setIsImsRequest(true);
    // Set parameters
    if (colrInfo.presentation != IpPresentation::IP_PRESENTATION_INVALID) {
      msg->setPresentation(convertIpPresentation(colrInfo.presentation));
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          std::shared_ptr<qcril::interfaces::SipErrorInfo> errorDetails = nullptr;
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
            errorDetails = std::static_pointer_cast<qcril::interfaces::SipErrorInfo>(resp->data);
          }
          auto data = std::make_shared<qcril::interfaces::SuppServiceStatusInfo>();
          if (data) {
            data->setErrorDetails(errorDetails);
          }
          sendSuppServiceStatusResponse(token, errorCode, data);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendSuppServiceStatusResponse(token, RIL_E_GENERIC_FAILURE, nullptr);
  }
  return Void();
}

Return<void> ImsRadioImpl::sendDtmf(int32_t token, const DtmfInfo &dtmfInfo) {
  imsRadiolog(">",
              "sendDtmf: token = " + std::to_string(token) + " dtmfInfo = " + toString(dtmfInfo));
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestImsSendDtmfMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    // Set parameters
    if (!dtmfInfo.dtmf.empty()) {
      // Only one digit expected.
      msg->setDigit(dtmfInfo.dtmf.c_str()[0]);
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
          }
          sendDtmfResponse(token, errorCode);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendDtmfResponse(token, RIL_E_GENERIC_FAILURE);
  }
  return Void();
}
void ImsRadioImpl::sendDtmfResponse(int32_t token, uint32_t errorCode) {
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "sendDtmfResponse: token = " + std::to_string(token) +
                         " errorCode = " + std::to_string(errorCode));
    Return<void> ret = respCb->sendDtmfResponse(token, errorCode);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

Return<void> ImsRadioImpl::startDtmf(int32_t token, const DtmfInfo &dtmfInfo) {
  imsRadiolog(">",
              "startDtmf: token = " + std::to_string(token) + " dtmfInfo = " + toString(dtmfInfo));
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestImsStartDtmfMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    // Set parameters
    if (!dtmfInfo.dtmf.empty()) {
      // Only one digit expected.
      msg->setDigit(dtmfInfo.dtmf.c_str()[0]);
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
          }
          sendStartDtmfResponse(token, errorCode);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendStartDtmfResponse(token, RIL_E_GENERIC_FAILURE);
  }
  return Void();
}
void ImsRadioImpl::sendStartDtmfResponse(int32_t token, uint32_t errorCode) {
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "startDtmfResponse: token = " + std::to_string(token) +
                         " errorCode = " + std::to_string(errorCode));
    Return<void> ret = respCb->startDtmfResponse(token, errorCode);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

Return<void> ImsRadioImpl::stopDtmf(int32_t token) {
  imsRadiolog(">", "stopDtmf: token = " + std::to_string(token));
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestImsStopDtmfMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
          }
          sendStopDtmfResponse(token, errorCode);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendStopDtmfResponse(token, RIL_E_GENERIC_FAILURE);
  }
  return Void();
}
void ImsRadioImpl::sendStopDtmfResponse(int32_t token, uint32_t errorCode) {
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "stopDtmfResponse: token = " + std::to_string(token) +
                         " errorCode = " + std::to_string(errorCode));
    Return<void> ret = respCb->stopDtmfResponse(token, errorCode);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

Return<void> ImsRadioImpl::setUiTtyMode(int32_t token, const TtyInfo &ttyInfo) {
  imsRadiolog(">",
              "setUiTtyMode: token = " + std::to_string(token) + " ttyInfo = " + toString(ttyInfo));
  QCRIL_LOG_FUNC_ENTRY();
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestImsSetUiTtyModeMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    // Set parameters
    if (ttyInfo.mode != TtyMode::TTY_MODE_INVALID) {
      msg->setTtyMode(convertTtyMode(ttyInfo.mode));
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
          }
          sendSetUiTTYModeResponse(token, errorCode);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendSetUiTTYModeResponse(token, RIL_E_GENERIC_FAILURE);
  }
  return Void();
}
void ImsRadioImpl::sendSetUiTTYModeResponse(int32_t token, uint32_t errorCode) {
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "setUiTTYModeResponse: token = " + std::to_string(token) +
                         " errorCode = " + std::to_string(errorCode));
    Return<void> ret = respCb->setUiTTYModeResponse(token, errorCode);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

Return<void> ImsRadioImpl::explicitCallTransfer(int32_t token,
                                                const ExplicitCallTransferInfo &ectInfo) {
  imsRadiolog(">", "explicitCallTransfer: token = " + std::to_string(token) +
                       " ectInfo = " + toString(ectInfo));
  QCRIL_LOG_FUNC_ENTRY();
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestImsExplicitCallTransferMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    // Set parameters
    if (ectInfo.callId != INT32_MAX) {
      msg->setCallId(ectInfo.callId);
    }
    msg->setEctType(convertEctType(ectInfo.ectType));

    if (!ectInfo.targetAddress.empty()) {
      msg->setTargetAddress(ectInfo.targetAddress.c_str());
    }
    if (ectInfo.targetCallId != INT32_MAX) {
      msg->setTargetCallId(ectInfo.targetCallId);
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
          }
          sendExplicitCallTransferResponse(token, errorCode);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendExplicitCallTransferResponse(token, RIL_E_GENERIC_FAILURE);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}
void ImsRadioImpl::sendExplicitCallTransferResponse(int32_t token, uint32_t errorCode) {
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "explicitCallTransferResponse: token = " + std::to_string(token) +
                         " errorCode = " + std::to_string(errorCode));
    Return<void> ret = respCb->explicitCallTransferResponse(token, errorCode);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

Return<void> ImsRadioImpl::modifyCallInitiate(int32_t token, const CallModifyInfo &callModifyInfo) {
  imsRadiolog(">", "modifyCallInitiate: token = " + std::to_string(token) +
                       " callModifyInfo = " + toString(callModifyInfo));
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestImsModifyCallInitiateMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    // Set parameters
    msg->setCallId(callModifyInfo.callIndex);
    if (callModifyInfo.hasCallDetails) {
      msg->setCallType(convertCallType(callModifyInfo.callDetails.callType));
      msg->setCallDomain(convertCallDomain(callModifyInfo.callDetails.callDomain));
      msg->setRttMode(convertRttMode(callModifyInfo.callDetails.rttMode));
    }

    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
          }
          sendModifyCallInitiateResponse(token, errorCode);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendModifyCallInitiateResponse(token, RIL_E_GENERIC_FAILURE);
  }
  return Void();
}
void ImsRadioImpl::sendModifyCallInitiateResponse(int32_t token, uint32_t errorCode) {
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "modifyCallInitiateResponse: token = " + std::to_string(token) +
                         " errorCode = " + std::to_string(errorCode));
    Return<void> ret = respCb->modifyCallInitiateResponse(token, errorCode);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

Return<void> ImsRadioImpl::cancelModifyCall(int32_t token, uint32_t callId) {
  imsRadiolog(">", "cancelModifyCall: token = " + std::to_string(token) +
                       " callId = " + std::to_string(callId));
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestImsCancelModifyCallMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    // Set parameters
    msg->setCallId(callId);

    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
          }
          sendCancelModifyCallResponse(token, errorCode);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendCancelModifyCallResponse(token, RIL_E_GENERIC_FAILURE);
  }
  return Void();
}
void ImsRadioImpl::sendCancelModifyCallResponse(int32_t token, uint32_t errorCode) {
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "cancelModifyCallResponse: token = " + std::to_string(token) +
                         " errorCode = " + std::to_string(errorCode));
    Return<void> ret = respCb->cancelModifyCallResponse(token, errorCode);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

Return<void> ImsRadioImpl::modifyCallConfirm(int32_t token, const CallModifyInfo &callModifyInfo) {
  imsRadiolog(">", "modifyCallConfirm: token = " + std::to_string(token) +
                       " callModifyInfo = " + toString(callModifyInfo));
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestImsModifyCallConfirmMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    // Set parameters
    msg->setCallId(callModifyInfo.callIndex);
    if (callModifyInfo.hasCallDetails) {
      msg->setCallType(convertCallType(callModifyInfo.callDetails.callType));
      msg->setCallDomain(convertCallDomain(callModifyInfo.callDetails.callDomain));
      msg->setRttMode(convertRttMode(callModifyInfo.callDetails.rttMode));
    }

    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
          }
          sendModifyCallConfirmResponse(token, errorCode);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendModifyCallConfirmResponse(token, RIL_E_GENERIC_FAILURE);
  }
  return Void();
}
void ImsRadioImpl::sendModifyCallConfirmResponse(int32_t token, uint32_t errorCode) {
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "modifyCallConfirmResponse: token = " + std::to_string(token) +
                         " errorCode = " + std::to_string(errorCode));
    Return<void> ret = respCb->modifyCallConfirmResponse(token, errorCode);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

Return<void> ImsRadioImpl::getRtpStatistics(int32_t token) {
  imsRadiolog(">", "getRtpStatistics: token = " + std::to_string(token));

  bool sendFailure = false;
  auto ctx = getContext(token);

  do {
    std::shared_ptr<QcRilRequestImsGetRtpStatsMessage> msg =
          std::make_shared<QcRilRequestImsGetRtpStatsMessage>(ctx);
    if (msg == nullptr) {
      sendFailure = true;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
      [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                    std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
        uint32_t errorCode = RIL_E_GENERIC_FAILURE;
        std::shared_ptr<qcril::interfaces::RtpStatisticsData> respData = nullptr;
        (void)msg;
        if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
          respData = std::static_pointer_cast<qcril::interfaces::RtpStatisticsData>(resp->data);
          errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
        }
        sendGetRtpStatsMessageResponse(token, errorCode, respData);
    });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);

  if (sendFailure) {
    sendGetRtpStatsMessageResponse(token, RIL_E_GENERIC_FAILURE, nullptr);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

void ImsRadioImpl::sendGetRtpStatsMessageResponse(uint32_t token, uint32_t errorCode,
  std::shared_ptr<qcril::interfaces::RtpStatisticsData> data) {
  uint64_t count = 0;
  if (data != nullptr) {
    count = data->getCount();
  }
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "sendGetRtpStatsMessageResponse: token = " + std::to_string(token) +
                     " errorCode = " + std::to_string(errorCode) +
                     " count = " + std::to_string(count));
    Return<void> ret = respCb->getRtpStatisticsResponse(token, errorCode, count);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

void ImsRadioImpl::sendGetRtpErrorStatsMessageResponse(uint32_t token, uint32_t errorCode,
  std::shared_ptr<qcril::interfaces::RtpStatisticsData> data) {
  uint64_t count = 0;
  if (data != nullptr) {
    count = data->getCount();
  }
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "sendGetRtpErrorStatsMessageResponse: token = " +
                       std::to_string(token) +
                       " errorCode = " + std::to_string(errorCode) +
                       " count = " + std::to_string(count));
    Return<void> ret = respCb->getRtpErrorStatisticsResponse(token, errorCode, count);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}


Return<void> ImsRadioImpl::getRtpErrorStatistics(int32_t token) {
  imsRadiolog(">", "getRtpErrorStatistics: token = " + std::to_string(token));

  bool sendFailure = false;
  auto ctx = getContext(token);

  do {
    std::shared_ptr<QcRilRequestImsGetRtpErrorStatsMessage> msg =
    std::make_shared<QcRilRequestImsGetRtpErrorStatsMessage>(ctx);
    if (msg == nullptr) {
      sendFailure = true;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
      [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                  std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
        uint32_t errorCode = RIL_E_GENERIC_FAILURE;
        std::shared_ptr<qcril::interfaces::RtpStatisticsData> respData = nullptr;
        (void)msg;
        if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
          respData = std::static_pointer_cast<qcril::interfaces::RtpStatisticsData>(resp->data);
          errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
        }
        sendGetRtpErrorStatsMessageResponse(token, errorCode, respData);
      });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);

  if (sendFailure) {
    sendGetRtpErrorStatsMessageResponse(token, RIL_E_GENERIC_FAILURE, nullptr);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

/*
 *  @brief
 *  Server side implementtaion of the IImsradio service's add participant API
 *
 */
Return<void> ImsRadioImpl::addParticipant(int32_t token, const DialRequest &dialRequest) {
  imsRadiolog(">", "addParticipant: token = " + std::to_string(token) +
                       " dialRequest = " + toString(dialRequest));
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
            errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
          }
          sendAddParticipantResponse(token, errorCode);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendAddParticipantResponse(token, RIL_E_GENERIC_FAILURE);
  }
  return Void();
}
void ImsRadioImpl::sendAddParticipantResponse(int32_t token, uint32_t errorCode) {
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "addParticipantResponse: token = " + std::to_string(token) +
                         " errorCode = " + std::to_string(errorCode));
    Return<void> ret = respCb->addParticipantResponse(token, errorCode);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

void ImsRadioImpl::sendRttMessageResponse(uint32_t token, uint32_t errorCode,
  std::shared_ptr<qcril::interfaces::BasePayload> /*data*/) {
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "sendRttMessageResponse: token = " + std::to_string(token) +
                         " errorCode = " + std::to_string(errorCode));
    Return<void> retStatus = respCb->sendRttMessageResponse(token, errorCode);
    if (!retStatus.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s",
                retStatus.description().c_str());
    }
  }
}

Return<void> ImsRadioImpl::sendRttMessage(int32_t token, const hidl_string& rttMessage) {
  imsRadiolog(">",
              "sendRttMessage: token=" + std::to_string(token) + " rttmsg=" + toString(rttMessage));

  bool sendFailure = false;
  auto ctx = getContext(token);

  do {
    string rttStr;
    if(!rttMessage.empty()) {
      rttStr = rttMessage.c_str();
    } else {
      sendFailure = true;
      break;
    }

    std::shared_ptr<QcRilRequestImsSendRttMessage> msg =
          std::make_shared<QcRilRequestImsSendRttMessage>(ctx, rttStr);
    if (msg == nullptr) {
      sendFailure = true;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
      [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                 std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
        uint32_t errorCode = RIL_E_GENERIC_FAILURE;
        (void)msg;
        if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
          errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
        }
        sendRttMessageResponse(token, errorCode, nullptr);
    });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);

  if (sendFailure) {
     sendRttMessageResponse(token, RIL_E_GENERIC_FAILURE, nullptr);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> ImsRadioImpl::deflectCall(int32_t token,
                                       const DeflectRequestInfo &deflectRequestInfo) {
  imsRadiolog(">", "deflectCall: token = " + std::to_string(token) +
                       " deflectRequestInfo = " + toString(deflectRequestInfo));
  QCRIL_LOG_FUNC_ENTRY();
  std::shared_ptr<ImsRadioContext> ctx = getContext(token);
  bool sendFailure = false;
  do {
    auto msg = std::make_shared<QcRilRequestImsDeflectCallMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    // Set parameters
    if (deflectRequestInfo.connIndex != INT32_MAX) {
      msg->setCallId(deflectRequestInfo.connIndex);
    }
    if (!deflectRequestInfo.number.empty()) {
      msg->setNumber(deflectRequestInfo.number.c_str());
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
        [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                      std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
          uint32_t errorCode = RIL_E_GENERIC_FAILURE;
          (void)msg;
          if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
            errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
          }
          sendDeflectCallResponse(token, errorCode);
        });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);
  if (sendFailure) {
    sendDeflectCallResponse(token, RIL_E_GENERIC_FAILURE);
  }
  QCRIL_LOG_FUNC_RETURN();
  return Void();
}
void ImsRadioImpl::sendDeflectCallResponse(int32_t token, uint32_t errorCode) {
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "deflectCallResponse: token = " + std::to_string(token) +
                         " errorCode = " + std::to_string(errorCode));
    Return<void> ret = respCb->deflectCallResponse(token, errorCode);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

/**
 * @brief
 * Server side implementation of the IImsRadio service's sendGeolocation API.
 *
 */
Return<void> ImsRadioImpl::sendGeolocationInfo(int32_t token, double lat, double lon,
                                               const AddressInfo &addressInfo) {
  imsRadiolog(">", "sendGeolocationInfo: token=" + std::to_string(token) +
                       " lat=" + std::to_string(lat) + " lon=" + std::to_string(lon) +
                       " addressInfo=" + toString(addressInfo));
  V1_2::AddressInfo v12AddressInfo = {};
  migrateAddressInfo(addressInfo, v12AddressInfo);
  return sendGeolocationInfo_1_2(token, lat, lon, v12AddressInfo);
}

Return<void> ImsRadioImpl::sendGeolocationInfo_1_2(int32_t token, double lat, double lon,
            const V1_2::AddressInfo& addressInfo) {
  imsRadiolog(">", "sendGeolocationInfo_1_2: token=" + std::to_string(token) +
                       " lat=" + std::to_string(lat) + " lon=" + std::to_string(lon) +
                       " addressInfo=" + toString(addressInfo));
  bool sendFailure = false;
  auto ctx = getContext(token);
  do {
    std::shared_ptr<QcRilRequestImsGeoLocationInfoMessage> msg =
        std::make_shared<QcRilRequestImsGeoLocationInfoMessage>(ctx);
    if (msg == nullptr) {
      QCRIL_LOG_ERROR("msg is nullptr");
      sendFailure = true;
      break;
    }
    msg->setLatitude(lat);
    msg->setLongitude(lon);
    if (!addressInfo.city.empty()) {
      msg->setCity(addressInfo.city.c_str());
    }
    if (!addressInfo.state.empty()) {
      msg->setState(addressInfo.state.c_str());
    }
    if (!addressInfo.country.empty()) {
      msg->setCountry(addressInfo.country.c_str());
    }
    if (!addressInfo.postalCode.empty()) {
      msg->setPostalCode(addressInfo.postalCode.c_str());
    }
    if (!addressInfo.countryCode.empty()) {
      msg->setCountryCode(addressInfo.countryCode.c_str());
    }
    if (!addressInfo.street.empty()) {
      msg->setStreet(addressInfo.street.c_str());
    }
    if (!addressInfo.houseNumber.empty()) {
      msg->setHouseNumber(addressInfo.houseNumber.c_str());
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
      [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
                    std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
        uint32_t errorCode = RIL_E_GENERIC_FAILURE;
        (void)msg;
        if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
          errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
        }
        sendGeoLocationResponse(token, errorCode, nullptr);
    });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);

  if (sendFailure) {
    sendGeoLocationResponse(token, RIL_E_GENERIC_FAILURE, nullptr);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

void ImsRadioImpl::sendGeoLocationResponse(uint32_t token, uint32_t errorCode,
  std::shared_ptr<qcril::interfaces::BasePayload> /*data*/) {
  sp<IImsRadioResponse> respCb = getResponseCallback();
  if (respCb != nullptr) {
    imsRadiolog("<", "sendGeolocationInfoResponse: token = " + std::to_string(token) +
                         " errorCode = " + std::to_string(errorCode));
    Return<void> retStatus = respCb->sendGeolocationInfoResponse(token,
                                  errorCode);
    if (!retStatus.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s",
             retStatus.description().c_str());
    }
  }
}

void ImsRadioImpl::sendGetImsSubConfigMessageResponse(uint32_t token, uint32_t errorCode,
  std::shared_ptr<qcril::interfaces::ImsSubConfigInfo> data) {
  sp<IImsRadioResponse> respCb = mImsRadioResponseCb;
  if (respCb != nullptr) {
    ImsSubConfigInfo subConfigInfo = ImsSubConfigInfo{};
    if(data != nullptr) {
      convertImsSubConfigInfo(subConfigInfo, *data);
    }
    imsRadiolog("<", "sendGetImsSubConfigMessageResponse: token=" + std::to_string(token) +
                         " errorCode=" + std::to_string(errorCode) +
                         " subConfigInfo=" + toString(subConfigInfo));
    Return<void> ret = respCb->getImsSubConfigResponse(token, errorCode, subConfigInfo);
    if (!ret.isOk()) {
      QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
    }
  }
}

Return<void> ImsRadioImpl::getImsSubConfig(int32_t token) {
  imsRadiolog(">", "getImsSubConfig: token = " + std::to_string(token));

  bool sendFailure = false;
  auto ctx = getContext(token);

  do {
    std::shared_ptr<QcRilRequestImsGetSubConfigMessage> msg =
          std::make_shared<QcRilRequestImsGetSubConfigMessage>(ctx);
    if (msg == nullptr) {
      sendFailure = true;
      break;
    }
    GenericCallback<QcRilRequestMessageCallbackPayload> cb(
      [this, token](std::shared_ptr<Message> msg, Message::Callback::Status status,
               std::shared_ptr<QcRilRequestMessageCallbackPayload> resp) -> void {
        uint32_t errorCode = RIL_E_GENERIC_FAILURE;
        std::shared_ptr<qcril::interfaces::ImsSubConfigInfo> respData = nullptr;
        (void)msg;
        if (status == Message::Callback::Status::SUCCESS && resp != nullptr) {
          respData = std::static_pointer_cast<qcril::interfaces::ImsSubConfigInfo>(resp->data);
          errorCode = qcril_qmi_ims_map_ril_error_to_ims_error(resp->errorCode);
        }
        sendGetImsSubConfigMessageResponse(token, errorCode, respData);
    });
    msg->setCallback(&cb);
    msg->dispatch();
  } while (FALSE);

  if (sendFailure) {
    sendGetImsSubConfigMessageResponse(token, RIL_E_GENERIC_FAILURE, nullptr);
  }

  QCRIL_LOG_FUNC_RETURN();
  return Void();
}

Return<void> ImsRadioImpl::sendImsSms(int32_t token, const V1_2::ImsSmsMessage& imsSms) {
    imsRadiolog(">", "sendImsSms: token=" + std::to_string(token) + " imsSms=" + toString(imsSms));
    errno_enum_type ret = E_FAILURE;

    RIL_RadioTechnologyFamily tech = (imsSms.format == "3gpp") ? RADIO_TECH_3GPP :
            RADIO_TECH_3GPP2;
    auto msg = std::make_shared<RilRequestImsSendSmsMessage>(imsSms.messageRef,
            tech, imsSms.shallRetry);
    if (msg) {
        if (tech == RADIO_TECH_3GPP) {
            std::string payload;
            QCRIL_LOG_INFO("sendImsSms: pdu size = %d", imsSms.pdu.size());
            if (convertBytesToHexString(imsSms.pdu.data(), imsSms.pdu.size(), payload)) {
                msg->setGsmPayload(imsSms.smsc, payload);
                ret = E_SUCCESS;
            }
        } else {
            RIL_CDMA_SMS_Message cdmaMsg;
            if (convertPayloadToCdmaFormat(imsSms.pdu.data(),
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
                (void)status;
                if (solicitedMsg && responseDataPtr) {
                    sp<V1_2::IImsRadioResponse> respCbV1_2 = getResponseCallbackV1_2();
                    V1_2::ImsSmsSendStatusResult statusReport;
                    V1_5::ImsSmsSendFailureReason v15Reason;
                    V1_5::utils::convertRilImsResponseToHidlResponse(responseDataPtr->rilErr,
                            tech, responseDataPtr->bLteOnlyReg, statusReport, v15Reason);
                    auto reason = V1_5::utils::convertHidlSendSmsFailureReasonToV12(v15Reason);
                    imsRadiolog("<", "sendImsSmsResponse: token=" + std::to_string(token) +
                      " msgeRef=" + std::to_string(responseDataPtr->messageRef) +
                      " statusReport=" + toString(statusReport) +
                      " reason=" + toString(reason));
                    Return<void> ret = respCbV1_2->sendImsSmsResponse(token,
                        responseDataPtr->messageRef, statusReport, reason);
                    if (!ret.isOk()) {
                      QCRIL_LOG_ERROR("Unable to send response. Exception : %s",
                                      ret.description().c_str());
                    }
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    } else {
        sp<V1_2::IImsRadioResponse> respCbV1_2 = getResponseCallbackV1_2();
        imsRadiolog("<", "sendImsSmsResponse: token=" + std::to_string(token) +
            " msgeRef=-1 statusReport=SEND_STATUS_ERROR reason=RESULT_ERROR_GENERIC_FAILURE");
        Return<void> ret = respCbV1_2->sendImsSmsResponse(token, -1,
                V1_2::ImsSmsSendStatusResult::SEND_STATUS_ERROR,
                V1_2::ImsSmsSendFailureReason::RESULT_ERROR_GENERIC_FAILURE);
        if (!ret.isOk()) {
          QCRIL_LOG_ERROR("Unable to send response. Exception : %s", ret.description().c_str());
        }
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> ImsRadioImpl::acknowledgeSms(int32_t token, uint32_t messageRef,
        V1_2::ImsSmsDeliverStatusResult smsdeliverResult) {
    imsRadiolog(">", "acknowledgeSms: token=" + std::to_string(token) +
                     " smsResult=" + toString(smsdeliverResult));

    // currently no respCb defined for ims sms ack
    auto msg = std::make_shared<RilRequestAckImsSmsMessage>(messageRef,
            convertHidlToRilSmsAckResult(smsdeliverResult));
    if (msg) {
        GenericCallback<RIL_Errno> cb(
            ([token](std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
                        std::shared_ptr<RIL_Errno> responseDataPtr) -> void {
                (void)status;
                (void)solicitedMsg;
                (void)token;
                if (responseDataPtr) {
                    QCRIL_LOG_INFO("acknowledgeSms cb, result: %d", *responseDataPtr);
                }
        }));
        msg->setCallback(&cb);
        msg->dispatch();
    }

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

Return<void> ImsRadioImpl::acknowledgeSmsReport(int32_t token, uint32_t messageRef,
        V1_2::ImsSmsStatusReportResult smsstatusreport) {
    imsRadiolog(">", "acknowledgeSmsReport: token=" + std::to_string(token) +
                     " messageRef=" +std::to_string(messageRef) +
                     " smsReport=" + toString(smsstatusreport));
    return Void();
}

Return<void> ImsRadioImpl::getSmsFormat(getSmsFormat_cb _hidl_cb) {
    imsRadiolog(">", "getSmsFormat");
    const char* format = "unknown";
    switch (qcril_qmi_sms_get_ims_sms_format()) {
    case QMI_RIL_SMS_FORMAT_UNKNOWN:
        format = "unknown";
        break;
    case QMI_RIL_SMS_FORMAT_3GPP:
        format = "3gpp";
        break;
    case QMI_RIL_SMS_FORMAT_3GPP2:
        format = "3gpp2";
        break;
    default:
        break;
    }

    imsRadiolog("<", std::string("getSmsFormat: format=") + format);
    _hidl_cb(format);

    QCRIL_LOG_FUNC_RETURN();
    return Void();
}

} // namespace implementation
} // namespace V1_0
} // namespace ims
} // namespace radio
} // namespace hardware
} // namespace qti
} // namespace vendor
