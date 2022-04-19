/*===========================================================================
   Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
   All Rights Reserved.
   Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#pragma once

extern "C" {
#include "qcril_log.h"
}

#include <framework/legacy.h>
#include <interfaces/ims/QcRilUnsolImsRegStateMessage.h>
#include <interfaces/ims/QcRilUnsolImsRttMessage.h>
#include <interfaces/ims/QcRilUnsolImsVowifiCallQuality.h>
#include <interfaces/ims/QcRilUnsolImsVopsIndication.h>
#include <interfaces/ims/QcRilUnsolImsSubConfigIndication.h>
#include <interfaces/ims/QcRilUnsolImsGeoLocationInfo.h>
#include <interfaces/ims/QcRilUnsolImsSrvStatusIndication.h>
#include <interfaces/ims/QcRilUnsolImsSsacInfoIndication.h>
#include <interfaces/ims/QcRilUnsolImsRegBlockStatusMessage.h>
#include <interfaces/ims/QcRilUnsolImsPendingMultiLineStatus.h>
#include <interfaces/ims/QcRilUnsolImsMultiIdentityStatusMessage.h>
#include <interfaces/ims/QcRilUnsolImsVoiceInfo.h>
#include <interfaces/ims/QcRilUnsolImsExitEcbmIndication.h>
#include <interfaces/ims/QcRilUnsolImsEnterEcbmIndication.h>
#include <interfaces/ims/QcRilUnsolImsRadioStateIndication.h>
#include <interfaces/ims/QcRilUnsolImsWfcRoamingConfigIndication.h>

#include <QtiMutex.h>
#include <ImsRadioContext.h>
#include "comdef.h"
#include "telephony/ril.h"

#include <ImsRadioContext.h>
#include <framework/legacy.h>
#include <hidl/HidlSupport.h>
#include <interfaces/ims/QcRilUnsolImsRegStateMessage.h>
#include <interfaces/voice/QcRilUnsolAutoCallRejectionMessage.h>
#include <interfaces/voice/QcRilUnsolSupplementaryServiceMessage.h>
#include <interfaces/voice/QcRilUnsolConfParticipantStatusInfoMessage.h>
#include <interfaces/voice/QcRilUnsolCallRingingMessage.h>
#include <interfaces/voice/QcRilUnsolImsCallStatusMessage.h>
#include <interfaces/voice/QcRilUnsolImsConferenceInfoMessage.h>
#include <interfaces/voice/QcRilUnsolImsHandoverMessage.h>
#include <interfaces/voice/QcRilUnsolImsModifyCallMessage.h>
#include <interfaces/voice/QcRilUnsolRingbackToneMessage.h>
#include <interfaces/voice/QcRilUnsolImsTtyNotificationMessage.h>
#include <interfaces/voice/QcRilUnsolImsViceInfoMessage.h>
#include <interfaces/voice/QcRilUnsolSuppSvcNotificationMessage.h>
#include <interfaces/sms/QcRilUnsolMessageWaitingInfoMessage.h>
#include <interfaces/voice/QcRilUnsolOnUssdMessage.h>
#include <modules/sms/RilUnsolIncomingImsSMSMessage.h>
#include <modules/sms/RilUnsolNewImsSmsStatusReportMessage.h>
#include <vendor/qti/hardware/radio/ims/1.3/IImsRadio.h>
#include <vendor/qti/hardware/radio/ims/1.3/IImsRadioIndication.h>
#include <vendor/qti/hardware/radio/ims/1.3/IImsRadioResponse.h>
#include <vendor/qti/hardware/radio/ims/1.3/types.h>

#include <utils/RefBase.h>

using ::android::hardware::Return;
using ::android::hardware::hidl_death_recipient;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::sp;

class ImsRadioImplBase : virtual public ::android::RefBase {
 private:
  qcril_instance_id_e_type mInstanceId;

 public:
  ImsRadioImplBase(qcril_instance_id_e_type instance) : mInstanceId(instance) {}
  virtual ~ImsRadioImplBase() {}
  qcril_instance_id_e_type getInstanceId() { return mInstanceId; }
  std::shared_ptr<ImsRadioContext> getContext(uint32_t serial) {
    std::shared_ptr<ImsRadioContext> ctx = std::make_shared<ImsRadioContext>(mInstanceId, serial);
    return ctx;
  }
  virtual void registerService() = 0;
  virtual void setCallback_nolock(
      const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadioResponse> &resp,
      const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadioIndication> &ind) = 0;
  virtual void clearCallbacks() = 0;
  void imsRadiolog(std::string dir, std::string logStr);
  void notifyImsClientConnected();

  virtual void notifyOnRegistrationChanged(std::shared_ptr<QcRilUnsolImsRegStateMessage> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyOnRTTMessage(std::shared_ptr<QcRilUnsolImsRttMessage> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyOnVowifiCallQuality(std::shared_ptr<QcRilUnsolImsVowifiCallQuality> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyOnVopsChanged(std::shared_ptr<QcRilUnsolImsVopsIndication> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyOnSubConfigChanged(std::shared_ptr<QcRilUnsolImsSubConfigIndication> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyOnGeoLocationChange(std::shared_ptr<QcRilUnsolImsGeoLocationInfo> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyOnServiceStatusChange(
      std::shared_ptr<QcRilUnsolImsSrvStatusIndication> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyOnSsacInfoChange(std::shared_ptr<QcRilUnsolImsSsacInfoIndication> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyOnRegBlockStatusChange(
      std::shared_ptr<QcRilUnsolImsRegBlockStatusMessage> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyOnCallStateChanged(std::shared_ptr<QcRilUnsolImsCallStatusMessage> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyOnModifyCall(std::shared_ptr<QcRilUnsolImsModifyCallMessage> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyOnRing(std::shared_ptr<QcRilUnsolCallRingingMessage> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyOnRingbackTone(std::shared_ptr<QcRilUnsolRingbackToneMessage> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyOnHandover(std::shared_ptr<QcRilUnsolImsHandoverMessage> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyOnTtyNotification(
      std::shared_ptr<QcRilUnsolImsTtyNotificationMessage> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyOnRefreshConferenceInfo(
      std::shared_ptr<QcRilUnsolImsConferenceInfoMessage> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyOnRefreshViceInfo(std::shared_ptr<QcRilUnsolImsViceInfoMessage> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyOnSuppServiceNotification(
      std::shared_ptr<QcRilUnsolSuppSvcNotificationMessage> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyOnParticipantStatusInfo(
      std::shared_ptr<QcRilUnsolConfParticipantStatusInfoMessage> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyOnIncomingCallAutoRejected(
      std::shared_ptr<QcRilUnsolAutoCallRejectionMessage> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyOnSupplementaryServiceIndication(
      std::shared_ptr<QcRilUnsolSupplementaryServiceMessage> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyIncomingImsSms(std::shared_ptr<RilUnsolIncomingImsSMSMessage> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyNewImsSmsStatusReport(
      std::shared_ptr<RilUnsolNewImsSmsStatusReportMessage> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyOnPendingMultiIdentityStatus(
      std::shared_ptr<QcRilUnsolImsPendingMultiLineStatus> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyOnMultiIdentityLineStatus(
      std::shared_ptr<QcRilUnsolImsMultiIdentityStatusMessage> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyOnVoiceInfoStatusChange(std::shared_ptr<QcRilUnsolImsVoiceInfo> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyOnExitEcbmIndication(
      std::shared_ptr<QcRilUnsolImsExitEcbmIndication> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyOnEnterEcbmIndication(
      std::shared_ptr<QcRilUnsolImsEnterEcbmIndication> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyOnRadioStateChanged(
      std::shared_ptr<QcRilUnsolImsRadioStateIndication> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyOnMessageWaiting(
      std::shared_ptr<QcRilUnsolMessageWaitingInfoMessage> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyOnModemSupportsWfcRoamingModeConfiguration(
      std::shared_ptr<QcRilUnsolImsWfcRoamingConfigIndication> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
  virtual void notifyOnUssdMessageFailed(
      std::shared_ptr<QcRilUnsolOnUssdMessage> /*msg*/) {
    QCRIL_LOG_ERROR("Dummy base class implementation: this method should not be invoked!");
  }
};

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_0 {
namespace implementation {

/** Class implementing the IImsRadio server implementation for all requests coming in
 * from Ims to RIL. Also registers the callback of type IImsRadioResponse to send the
 * solicited response back and IImsRadioIndication to send unsolicited responses.
 */
class ImsRadioImpl : public ImsRadioImplBase, public V1_3::IImsRadio, public hidl_death_recipient {
 private:
  sp<IImsRadioResponse> mImsRadioResponseCb;
  sp<IImsRadioIndication> mImsRadioIndicationCb;
  sp<V1_1::IImsRadioIndication> mImsRadioIndicationCbV1_1;
  sp<V1_2::IImsRadioResponse> mImsRadioResponseCbV1_2;
  sp<V1_2::IImsRadioIndication> mImsRadioIndicationCbV1_2;
  sp<V1_3::IImsRadioResponse> mImsRadioResponseCbV1_3;
  sp<V1_3::IImsRadioIndication> mImsRadioIndicationCbV1_3;

 protected:
  qtimutex::QtiSharedMutex mCallbackLock;
  sp<IImsRadioIndication> getIndicationCallback();
  sp<IImsRadioResponse> getResponseCallback();
  sp<V1_1::IImsRadioIndication> getIndicationCallbackV1_1();
  sp<V1_2::IImsRadioResponse> getResponseCallbackV1_2();
  sp<V1_2::IImsRadioIndication> getIndicationCallbackV1_2();
  sp<V1_3::IImsRadioResponse> getResponseCallbackV1_3();
  sp<V1_3::IImsRadioIndication> getIndicationCallbackV1_3();

  void serviceDied(uint64_t, const ::android::wp<::android::hidl::base::V1_0::IBase> &);

 public:
  ImsRadioImpl(qcril_instance_id_e_type instance);
  ~ImsRadioImpl();

  void registerService();

  void setCallback_nolock(const sp<IImsRadioResponse> &imsRadioResponse,
                          const sp<IImsRadioIndication> &imsRadioIndication);
  void clearCallbacks();

  // HIDL APIs
  Return<void> setCallback(const sp<IImsRadioResponse> &mImsRadioResponseCbParam,
                           const sp<IImsRadioIndication> &mImsRadioIndicationCbParam);
  Return<void> dial(int32_t token, const DialRequest &dialRequest);
  Return<void> getImsRegistrationState(int32_t token);
  Return<void> answer(int32_t token, CallType callType, IpPresentation presentation,
                      RttMode rttMode);
  Return<void> hangup_1_3(int32_t token, const V1_3::HangupRequestInfo &hangup);
  Return<void> hangup_1_2(int32_t token, const V1_2::HangupRequestInfo &hangup);
  Return<void> hangup_1_1(int32_t token, const V1_1::HangupRequestInfo &hangup);
  Return<void> hangup(int32_t token, const HangupRequestInfo &hangup);
  Return<void> requestRegistrationChange(int32_t token, RegState state);
  Return<void> queryServiceStatus(int32_t token);
  Return<void> setServiceStatus(int32_t token, const ServiceStatusInfo &srvStatusInfo);
  Return<void> hold(int32_t token, uint32_t callId);
  Return<void> resume(int32_t token, uint32_t callId);
  Return<void> setConfig(int32_t token, const ConfigInfo &config);
  Return<void> getConfig(int32_t token, const ConfigInfo &config);
  Return<void> conference(int32_t token);
  Return<void> getClip(int32_t token);
  Return<void> getClir(int32_t token);
  Return<void> setClir(int32_t token, const ClirInfo &clirInfo);
  Return<void> getColr(int32_t token);
  Return<void> setColr_1_3(int32_t token, const V1_3::ColrInfo &colrInfo);
  Return<void> setColr(int32_t token, const ColrInfo &colrInfo);
  Return<void> exitEmergencyCallbackMode(int32_t token);
  Return<void> sendDtmf(int32_t token, const DtmfInfo &dtmfInfo);
  Return<void> startDtmf(int32_t token, const DtmfInfo &dtmfInfo);
  Return<void> stopDtmf(int32_t token);
  Return<void> setUiTtyMode(int32_t token, const TtyInfo &ttyInfo);
  Return<void> modifyCallInitiate(int32_t token, const CallModifyInfo &callModifyInfo);
  Return<void> cancelModifyCall(int32_t token, uint32_t callId);
  Return<void> modifyCallConfirm(int32_t token, const CallModifyInfo &callModifyInfo);
  Return<void> getCallWaiting(int32_t token, uint32_t serviceClass);
  Return<void> setSuppServiceNotification(int32_t token, ServiceClassStatus status);
  Return<void> queryCallForwardStatus(int32_t token, const CallForwardInfo &callForwardInfo);
  Return<void> setCallForwardStatus(int32_t token, const CallForwardInfo &callForwardInfo);
  Return<void> setCallWaiting(int32_t token, ServiceClassStatus inStatus, uint32_t inServiceClass);
  Return<void> explicitCallTransfer(int32_t token, const ExplicitCallTransferInfo &ectInfo);
  Return<void> suppServiceStatus(int32_t token, int32_t operationType, FacilityType facilityType,
                                 const CbNumListInfo &cbNumListInfo);
  Return<void> deflectCall(int32_t token, const DeflectRequestInfo &deflectRequestInfo);
  Return<void> getRtpStatistics(int32_t token);
  Return<void> getRtpErrorStatistics(int32_t token);
  Return<void> addParticipant(int32_t token, const DialRequest &dialRequest);
  Return<void> sendGeolocationInfo_1_2(int32_t token, double lat, double lon,
                                       const V1_2::AddressInfo &addressInfo);
  Return<void> sendGeolocationInfo(int32_t token, double lat, double lon,
                                   const AddressInfo &addressInfo);
  Return<void> getImsSubConfig(int32_t token);
  Return<void> sendImsSms(int32_t token, const V1_2::ImsSmsMessage &imsSms);
  Return<void> acknowledgeSms(int32_t token, uint32_t messageRef,
                              V1_2::ImsSmsDeliverStatusResult smsdeliverResult);
  Return<void> acknowledgeSmsReport(int32_t token, uint32_t messageRef,
                                    V1_2::ImsSmsStatusReportResult smsstatusreport);
  Return<void> getSmsFormat(getSmsFormat_cb _hidl_cb);
  Return<void> sendRttMessage(int32_t token, const hidl_string &rttMessage);

  // Unsol message handlers
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
  void notifyOnPendingMultiIdentityStatus(std::shared_ptr<QcRilUnsolImsPendingMultiLineStatus> msg);
  void notifyOnMultiIdentityLineStatus(std::shared_ptr<QcRilUnsolImsMultiIdentityStatusMessage> msg);
  void notifyOnVoiceInfoStatusChange(std::shared_ptr<QcRilUnsolImsVoiceInfo> msg);
  void notifyOnExitEcbmIndication(std::shared_ptr<QcRilUnsolImsExitEcbmIndication> msg);
  void notifyOnEnterEcbmIndication(std::shared_ptr<QcRilUnsolImsEnterEcbmIndication> msg);
  void notifyOnRadioStateChanged(std::shared_ptr<QcRilUnsolImsRadioStateIndication> msg);
  void notifyOnMessageWaiting(std::shared_ptr<QcRilUnsolMessageWaitingInfoMessage> msg);

  void sendSetConfigMessageResponse(uint32_t token, uint32_t errorCode,
    std::shared_ptr<qcril::interfaces::ConfigInfo> data);
  void sendSuppServiceStatusResponse(
      int32_t token, uint32_t errorCode,
      const std::shared_ptr<qcril::interfaces::SuppServiceStatusInfo> data);

private:
  // Send response on IImsRadioResponse
  void sendRequestRegistrationChangeResponse(int32_t token, uint32_t errorCode,
                                             std::shared_ptr<qcril::interfaces::BasePayload> data);
  void sendGetRegistrationResponse(int32_t token, uint32_t errorCode,
                                   std::shared_ptr<qcril::interfaces::Registration> data);
  void sendQueryServiceStatusResponse(
      int32_t token, uint32_t errorCode,
      std::shared_ptr<qcril::interfaces::ServiceStatusInfoList> data);
  void sendDialResponse(int32_t token, uint32_t errorCode);
  void sendAnswerResponse(int32_t token, uint32_t errorCode);
  void sendHangupResponse(int32_t token, uint32_t errorCode);
  void sendHoldResponse(int32_t token, uint32_t errorCode,
                        std::shared_ptr<qcril::interfaces::SipErrorInfo> data);
  void sendResumeResponse(int32_t token, uint32_t errorCode,
                          std::shared_ptr<qcril::interfaces::SipErrorInfo> data);
  void sendQueryCallForwardStatusResponse(
      int32_t token, uint32_t errorCode,
      const std::shared_ptr<qcril::interfaces::GetCallForwardRespData> data);
  void sendGetCallWaitingResponse(int32_t token, uint32_t errorCode,
                                  const std::shared_ptr<qcril::interfaces::CallWaitingInfo> data);
  void sendConferenceResponse(int32_t token, uint32_t errorCode,
                              const std::shared_ptr<qcril::interfaces::SipErrorInfo> data);
  void sendGetClipResponse(int32_t token, uint32_t errorCode,
                           const std::shared_ptr<qcril::interfaces::ClipInfo> data);
  void sendGetClirResponse(int32_t token, uint32_t errorCode,
                           const std::shared_ptr<qcril::interfaces::ClirInfo> data);
  void sendSetClirResponse(int32_t token, uint32_t errorCode);
  void sendGetColrResponse(int32_t token, uint32_t errorCode,
                           const std::shared_ptr<qcril::interfaces::ColrInfo> data);
  void sendDtmfResponse(int32_t token, uint32_t errorCode);
  void sendStartDtmfResponse(int32_t token, uint32_t errorCode);
  void sendStopDtmfResponse(int32_t token, uint32_t errorCode);
  void sendExitEmergencyCallbackModeResponse(int32_t token, uint32_t errorCode);
  void sendSetUiTTYModeResponse(int32_t token, uint32_t errorCode);
  void sendExplicitCallTransferResponse(int32_t token, uint32_t errorCode);
  void sendModifyCallInitiateResponse(int32_t token, uint32_t errorCode);
  void sendCancelModifyCallResponse(int32_t token, uint32_t errorCode);
  void sendModifyCallConfirmResponse(int32_t token, uint32_t errorCode);
  void sendAddParticipantResponse(int32_t token, uint32_t errorCode);
  void sendDeflectCallResponse(int32_t token, uint32_t errorCode);
  void sendSetSuppServiceNotificationResponse(int32_t token, uint32_t errorCode);
  void sendGetRtpStatsMessageResponse(uint32_t token, uint32_t errorCode,
    std::shared_ptr<qcril::interfaces::RtpStatisticsData> data);
  void sendGetRtpErrorStatsMessageResponse(uint32_t token, uint32_t errorCode,
    std::shared_ptr<qcril::interfaces::RtpStatisticsData> data);
  void sendGetImsSubConfigMessageResponse(uint32_t token, uint32_t errorCode,
    std::shared_ptr<qcril::interfaces::ImsSubConfigInfo> data);
  void sendRttMessageResponse(uint32_t token, uint32_t errorCode,
    std::shared_ptr<qcril::interfaces::BasePayload> data);
  void sendSetServiceStatusMessageResponse(uint32_t token, uint32_t errorCode,
    std::shared_ptr<qcril::interfaces::BasePayload> data);
  void sendGeoLocationResponse(uint32_t token, uint32_t errorCode,
    std::shared_ptr<qcril::interfaces::BasePayload> data);
  void sendGetConfigMessageResponse(uint32_t token, uint32_t errorCode,
    std::shared_ptr<qcril::interfaces::ConfigInfo> data);
};
}  // namespace implementation
}  // namespace V1_0
}  // namespace ims
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
