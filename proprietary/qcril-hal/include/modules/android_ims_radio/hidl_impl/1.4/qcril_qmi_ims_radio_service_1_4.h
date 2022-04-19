/*===========================================================================
   Copyright (c) 2018 Qualcomm Technologies, Inc.
   All Rights Reserved.
   Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#pragma once

#include "comdef.h"

#include <QtiMutex.h>
#include <hidl/HidlSupport.h>
#include <modules/android_ims_radio/hidl_impl/base/qcril_qmi_ims_radio_service.h>
#include <vendor/qti/hardware/radio/ims/1.4/IImsRadio.h>
#include <vendor/qti/hardware/radio/ims/1.4/IImsRadioIndication.h>
#include <vendor/qti/hardware/radio/ims/1.4/IImsRadioResponse.h>
#include <vendor/qti/hardware/radio/ims/1.4/types.h>
#include <interfaces/ims/QcRilRequestImsRegisterMultiIdentityMessage.h>
#include <interfaces/ims/QcRilRequestImsQueryVirtualLineInfo.h>
#include <interfaces/ims/QcRilUnsolImsPendingMultiLineStatus.h>
#include <interfaces/ims/QcRilUnsolImsMultiIdentityStatusMessage.h>

using ::android::hardware::Return;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::sp;

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_4 {
namespace implementation {

/*
 * Class implementing the IImsRadio server implementation for all requests coming in
 * from Ims to RIL. Also registers the callback of type IImsRadioResponse to send the
 * solicited response back and IImsRadioIndication to send unsolicited responses.
 */
class ImsRadioImpl_1_4 : public ImsRadioImplBase,
                         public V1_4::IImsRadio,
                         public hidl_death_recipient {
 private:
  sp<V1_0::implementation::ImsRadioImpl> mBaseImsRadioImpl;
  qtimutex::QtiSharedMutex mCallbackLock;
  sp<V1_4::IImsRadioResponse> mImsRadioResponseCbV1_4;
  sp<V1_4::IImsRadioIndication> mImsRadioIndicationCbV1_4;

  sp<V1_4::IImsRadioResponse> getResponseCallbackV1_4();
  sp<V1_4::IImsRadioIndication> getIndicationCallbackV1_4();

 public:
  ImsRadioImpl_1_4(qcril_instance_id_e_type instance);
  ~ImsRadioImpl_1_4();

  void serviceDied(uint64_t, const ::android::wp<::android::hidl::base::V1_0::IBase> &);
  void registerService();
  void setCallback_nolock(
      const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadioResponse>
          &imsRadioResponse,
      const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadioIndication>
          &imsRadioIndication);
  void clearCallbacks();

  // Unsol indications handlers
  void notifyOnRegistrationChanged(std::shared_ptr<QcRilUnsolImsRegStateMessage> msg);
  void notifyOnRTTMessage(std::shared_ptr<QcRilUnsolImsRttMessage> msg);
  void notifyOnVowifiCallQuality(std::shared_ptr<QcRilUnsolImsVowifiCallQuality> msg);
  void notifyOnVopsChanged(std::shared_ptr<QcRilUnsolImsVopsIndication> msg);
  void notifyOnSubConfigChanged(std::shared_ptr<QcRilUnsolImsSubConfigIndication> msg);
  void notifyOnGeoLocationChange(std::shared_ptr<QcRilUnsolImsGeoLocationInfo> msg);
  void notifyOnServiceStatusChange(std::shared_ptr<QcRilUnsolImsSrvStatusIndication> msg);
  void notifyOnSsacInfoChange(std::shared_ptr<QcRilUnsolImsSsacInfoIndication> msg);
  void notifyOnRegBlockStatusChange(std::shared_ptr<QcRilUnsolImsRegBlockStatusMessage> msg);
  void notifyOnCallStateChanged(std::shared_ptr<QcRilUnsolImsCallStatusMessage> msg);
  void notifyOnModifyCall(std::shared_ptr<QcRilUnsolImsModifyCallMessage> msg);
  void notifyOnRing(std::shared_ptr<QcRilUnsolCallRingingMessage> msg);
  void notifyOnRingbackTone(std::shared_ptr<QcRilUnsolRingbackToneMessage> msg);
  void notifyOnHandover(std::shared_ptr<QcRilUnsolImsHandoverMessage> msg);
  void notifyOnTtyNotification(std::shared_ptr<QcRilUnsolImsTtyNotificationMessage> msg);
  void notifyOnRefreshConferenceInfo(std::shared_ptr<QcRilUnsolImsConferenceInfoMessage> msg);
  void notifyOnRefreshViceInfo(std::shared_ptr<QcRilUnsolImsViceInfoMessage> msg);
  void notifyOnSuppServiceNotification(std::shared_ptr<QcRilUnsolSuppSvcNotificationMessage> msg);
  void notifyOnParticipantStatusInfo(
      std::shared_ptr<QcRilUnsolConfParticipantStatusInfoMessage> msg);
  void notifyOnIncomingCallAutoRejected(
      std::shared_ptr<QcRilUnsolAutoCallRejectionMessage> msg);
  void notifyOnSupplementaryServiceIndication(
      std::shared_ptr<QcRilUnsolSupplementaryServiceMessage> msg);
  void notifyIncomingImsSms(std::shared_ptr<RilUnsolIncomingImsSMSMessage> msg);
  void notifyNewImsSmsStatusReport(std::shared_ptr<RilUnsolNewImsSmsStatusReportMessage> msg);
  void notifyOnMessageWaiting(std::shared_ptr<QcRilUnsolMessageWaitingInfoMessage> msg);

  // Send response on IImsRadioResponse
  void sendDialResponse(int32_t token, uint32_t errorCode);
  void sendAddParticipantResponse(int32_t token, uint32_t errorCode);
  void notifyOnPendingMultiIdentityStatus(std::shared_ptr<QcRilUnsolImsPendingMultiLineStatus> msg);
  void notifyOnMultiIdentityLineStatus(std::shared_ptr<QcRilUnsolImsMultiIdentityStatusMessage> msg);

  void sendRegisterMultiIdentityLinesResponse(uint32_t token, uint32_t errorCode,
    std::shared_ptr<qcril::interfaces::BasePayload> data);
  void sendQueryVirtualLinesResponse(uint32_t token, uint32_t errorCode,
    std::shared_ptr<qcril::interfaces::VirtualLineInfo> data);

  void notifyOnVoiceInfoStatusChange(std::shared_ptr<QcRilUnsolImsVoiceInfo> msg);
  void notifyOnExitEcbmIndication(std::shared_ptr<QcRilUnsolImsExitEcbmIndication> msg);
  void notifyOnEnterEcbmIndication(std::shared_ptr<QcRilUnsolImsEnterEcbmIndication> msg);
  void notifyOnRadioStateChanged(std::shared_ptr<QcRilUnsolImsRadioStateIndication> msg);

  // HIDL APIs
  ::android::hardware::Return<void> setCallback(
      const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadioResponse>
          &imsRadioResponse,
      const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadioIndication>
          &imsRadioIndication);

  Return<void> dial_1_4(int32_t token, const V1_4::DialRequest &dialRequest);
  Return<void> addParticipant_1_4(int32_t token, const V1_4::DialRequest &dialRequest);
  Return<void> registerMultiIdentityLines(int32_t token,
                                          const hidl_vec<V1_4::MultiIdentityLineInfoHal> &info);
  Return<void> queryVirtualLineInfo(int32_t token, const hidl_string &msisdn);

  // Forward these to RadioImpl
  ::android::hardware::Return<void> dial(
      int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::DialRequest &dialRequest) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->dial(token, dialRequest);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> addParticipant(
      int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::DialRequest &dialRequest) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->addParticipant(token, dialRequest);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> getImsRegistrationState(int32_t token) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->getImsRegistrationState(token);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> answer(
      int32_t token, ::vendor::qti::hardware::radio::ims::V1_0::CallType callType,
      ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation presentation,
      ::vendor::qti::hardware::radio::ims::V1_0::RttMode mode) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->answer(token, callType, presentation, mode);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> hangup(
      int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::HangupRequestInfo &hangup) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->hangup(token, hangup);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> requestRegistrationChange(
      int32_t token, ::vendor::qti::hardware::radio::ims::V1_0::RegState state) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->requestRegistrationChange(token, state);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> queryServiceStatus(int32_t token) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->queryServiceStatus(token);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> setServiceStatus(
      int32_t token,
      const ::vendor::qti::hardware::radio::ims::V1_0::ServiceStatusInfo &srvStatusInfo) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->setServiceStatus(token, srvStatusInfo);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> hangup_1_1(
      int32_t token, const ::vendor::qti::hardware::radio::ims::V1_1::HangupRequestInfo &hangup) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->hangup_1_1(token, hangup);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> hangup_1_2(
      int32_t token, const ::vendor::qti::hardware::radio::ims::V1_2::HangupRequestInfo &hangup) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->hangup_1_2(token, hangup);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> sendImsSms(
      int32_t token, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsMessage &imsSms) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->sendImsSms(token, imsSms);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> acknowledgeSms(
      int32_t token, uint32_t messageRef,
      ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult smsResult) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->acknowledgeSms(token, messageRef, smsResult);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> acknowledgeSmsReport(
      int32_t token, uint32_t messageRef,
      ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult smsReport) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->acknowledgeSmsReport(token, messageRef, smsReport);
    }
    return ::android::hardware::Void();
  }
  using getSmsFormat_cb = std::function<void(const ::android::hardware::hidl_string &format)>;
  ::android::hardware::Return<void> getSmsFormat(getSmsFormat_cb _hidl_cb) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->getSmsFormat(_hidl_cb);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> sendGeolocationInfo_1_2(
      int32_t token, double lat, double lon,
      const ::vendor::qti::hardware::radio::ims::V1_2::AddressInfo &addressInfo) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->sendGeolocationInfo_1_2(token, lat, lon, addressInfo);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> hangup_1_3(
      int32_t token, const ::vendor::qti::hardware::radio::ims::V1_3::HangupRequestInfo &hangup) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->hangup_1_3(token, hangup);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> hold(int32_t token, uint32_t callId) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->hold(token, callId);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> resume(int32_t token, uint32_t callId) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->resume(token, callId);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> setConfig(
      int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::ConfigInfo &config) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->setConfig(token, config);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> getConfig(
      int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::ConfigInfo &config) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->getConfig(token, config);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> conference(int32_t token) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->conference(token);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> getClip(int32_t token) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->getClip(token);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> getClir(int32_t token) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->getClir(token);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> setClir(
      int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::ClirInfo &clirInfo) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->setClir(token, clirInfo);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> getColr(int32_t token) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->getColr(token);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> setColr(
      int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::ColrInfo &colrInfo) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->setColr(token, colrInfo);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> setColr_1_3(
      int32_t token, const ::vendor::qti::hardware::radio::ims::V1_3::ColrInfo &colrInfo) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->setColr_1_3(token, colrInfo);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> exitEmergencyCallbackMode(int32_t token) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->exitEmergencyCallbackMode(token);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> sendDtmf(
      int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::DtmfInfo &dtmfInfo) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->sendDtmf(token, dtmfInfo);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> startDtmf(
      int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::DtmfInfo &dtmfInfo) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->startDtmf(token, dtmfInfo);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> stopDtmf(int32_t token) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->stopDtmf(token);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> setUiTtyMode(
      int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::TtyInfo &ttyInfo) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->setUiTtyMode(token, ttyInfo);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> modifyCallInitiate(
      int32_t token,
      const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyInfo &callModifyInfo) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->modifyCallInitiate(token, callModifyInfo);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> modifyCallConfirm(
      int32_t token,
      const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyInfo &callModifyInfo) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->modifyCallConfirm(token, callModifyInfo);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> queryCallForwardStatus(
      int32_t token,
      const ::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo &callForwardInfo) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->queryCallForwardStatus(token, callForwardInfo);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> setCallForwardStatus(
      int32_t token,
      const ::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo &callForwardInfo) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->setCallForwardStatus(token, callForwardInfo);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> getCallWaiting(int32_t token, uint32_t serviceClass) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->getCallWaiting(token, serviceClass);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> setCallWaiting(
      int32_t token, ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus serviceStatus,
      uint32_t serviceClass) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->setCallWaiting(token, serviceStatus, serviceClass);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> setSuppServiceNotification(
      int32_t token, ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus status) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->setSuppServiceNotification(token, status);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> explicitCallTransfer(
      int32_t token,
      const ::vendor::qti::hardware::radio::ims::V1_0::ExplicitCallTransferInfo &ectInfo) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->explicitCallTransfer(token, ectInfo);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> suppServiceStatus(
      int32_t token, int32_t operationType,
      ::vendor::qti::hardware::radio::ims::V1_0::FacilityType facilityType,
      const ::vendor::qti::hardware::radio::ims::V1_0::CbNumListInfo &cbNumListInfo) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->suppServiceStatus(token, operationType, facilityType,
                                                  cbNumListInfo);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> getRtpStatistics(int32_t token) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->getRtpStatistics(token);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> getRtpErrorStatistics(int32_t token) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->getRtpErrorStatistics(token);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> deflectCall(
      int32_t token,
      const ::vendor::qti::hardware::radio::ims::V1_0::DeflectRequestInfo &deflectRequestInfo) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->deflectCall(token, deflectRequestInfo);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> sendGeolocationInfo(
      int32_t token, double lat, double lon,
      const ::vendor::qti::hardware::radio::ims::V1_0::AddressInfo &addressInfo) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->sendGeolocationInfo(token, lat, lon, addressInfo);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> getImsSubConfig(int32_t token) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->getImsSubConfig(token);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> sendRttMessage(
      int32_t token, const ::android::hardware::hidl_string &message) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->sendRttMessage(token, message);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> cancelModifyCall(int32_t token, uint32_t callId) {
    if (mBaseImsRadioImpl) {
      return mBaseImsRadioImpl->cancelModifyCall(token, callId);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> sendSetConfigMessageResponse(
      uint32_t token, uint32_t errorCode, std::shared_ptr<qcril::interfaces::ConfigInfo> data) {
    if (mBaseImsRadioImpl) {
      mBaseImsRadioImpl->sendSetConfigMessageResponse(token, errorCode, data);
    }
    return ::android::hardware::Void();
  }
  ::android::hardware::Return<void> sendSuppServiceStatusResponse(int32_t token, uint32_t errorCode,
      const std::shared_ptr<qcril::interfaces::SuppServiceStatusInfo> data) {
    if (mBaseImsRadioImpl) {
      mBaseImsRadioImpl->sendSuppServiceStatusResponse(token, errorCode, data);
    }
    return ::android::hardware::Void();
  }
};
}  // namespace implementation
}  // namespace V1_4
}  // namespace ims
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
