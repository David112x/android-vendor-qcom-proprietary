/*===========================================================================
   Copyright (c) 2017,2019 Qualcomm Technologies, Inc.
   All Rights Reserved.
   Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#pragma once

#include <hidl/HidlSupport.h>
#include <interfaces/ims/ims.h>
#include <interfaces/sms/sms.h>
#include <interfaces/voice/voice.h>
#include <vendor/qti/hardware/radio/ims/1.3/types.h>

using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_0 {
namespace utils {

typedef enum {
  SUCCESS = 0,
  RADIO_NOT_AVAILABLE = 1,
  GENERIC_FAILURE = 2,
  REQUEST_NOT_SUPPORTED = 6,
  CANCELLED = 7,
  UNUSED = 16,
  INVALID_PARAMETER = 27,
  REJECTED_BY_REMOTE = 28,
  IMS_DEREGISTERED = 29,
  NETWORK_NOT_SUPPORTED = 30,
  HOLD_RESUME_FAILED = 31,
  HOLD_RESUME_CANCELED = 32,
  REINVITE_COLLISION = 33,
  FDN_CHECK_FAILURE = 34,
  SS_MODIFIED_TO_DIAL = 35,
  SS_MODIFIED_TO_USSD = 36,
  SS_MODIFIED_TO_SS = 37,
  SS_MODIFIED_TO_DIAL_VIDEO = 38,
  DIAL_MODIFIED_TO_USSD = 39,
  DIAL_MODIFIED_TO_SS = 40,
  DIAL_MODIFIED_TO_DIAL = 41,
  DIAL_MODIFIED_TO_DIAL_VIDEO = 42,
  DIAL_VIDEO_MODIFIED_TO_USSD = 43,
  DIAL_VIDEO_MODIFIED_TO_SS = 44,
  DIAL_VIDEO_MODIFIED_TO_DIAL = 45,
  DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO = 46
} ImsRadioErrorCode;

ImsRadioErrorCode qcril_qmi_ims_map_ril_error_to_ims_error(int ril_error);

bool convertRegistrationInfo(RegistrationInfo &out, const qcril::interfaces::Registration &in);
bool convertAccessTechnologyStatus(StatusForAccessTech &out,
                                   const qcril::interfaces::AccessTechnologyStatus &in);
bool convertServiceStatusInfo(ServiceStatusInfo &out,
                              const qcril::interfaces::ServiceStatusInfo &in);
bool convertServiceStatusInfoList(hidl_vec<ServiceStatusInfo> &out,
                                  const qcril::interfaces::ServiceStatusInfoList &in);
bool convertServiceStatusInfoList(hidl_vec<ServiceStatusInfo> &out,
                                  const std::vector<qcril::interfaces::ServiceStatusInfo> &in);
bool convertCallForwardInfo(CallForwardInfo &out, const qcril::interfaces::CallForwardInfo &in);
bool convertCallForwardInfoList(hidl_vec<CallForwardInfo> &out,
                                const std::vector<qcril::interfaces::CallForwardInfo> &in);
bool convertCallFailCauseResponse(std::shared_ptr<qcril::interfaces::CallFailCauseResponse> &out,
                                  const V1_3::CallFailCauseResponse &in);
bool convertCallFailCauseResponse(
    V1_3::CallFailCauseResponse &out,
    const std::shared_ptr<qcril::interfaces::CallFailCauseResponse> &in);
bool convertClirInfo(std::shared_ptr<qcril::interfaces::ClirInfo> &out, const ClirInfo &in);
bool convertClirInfo(ClirInfo &out, const std::shared_ptr<qcril::interfaces::ClirInfo> &in);
bool convertColrInfo(V1_3::ColrInfo &out, const std::shared_ptr<qcril::interfaces::ColrInfo> &in);
bool convertClipInfo(ClipProvisionStatus &out,
                     const std::shared_ptr<qcril::interfaces::ClipInfo> &in);
bool convertSipErrorInfo(SipErrorInfo &out,
                         const std::shared_ptr<qcril::interfaces::SipErrorInfo> &in);
bool convertSipErrorInfo(std::shared_ptr<qcril::interfaces::SipErrorInfo> &out,
                         const SipErrorInfo &in);
bool convertCallFwdTimerInfo(CallFwdTimerInfo &out,
                             const std::shared_ptr<qcril::interfaces::CallFwdTimerInfo> &in);
bool convertCallFwdTimerInfo(std::shared_ptr<qcril::interfaces::CallFwdTimerInfo> &out,
                             const CallFwdTimerInfo &in);
bool convertCallInfo(V1_3::CallInfo &out, const qcril::interfaces::CallInfo &in);
bool convertCallInfoList(hidl_vec<V1_3::CallInfo> &out,
                         const std::vector<qcril::interfaces::CallInfo> &in);
bool convertVerstatInfo(V1_3::VerstatInfo &out,
                        const std::shared_ptr<qcril::interfaces::VerstatInfo> &in);
bool convertNumInfo(CbNumInfo &out, const qcril::interfaces::CallBarringNumbersInfo &in);
bool convertCbNumListInfo(CbNumListInfo &out,
                          const qcril::interfaces::CallBarringNumbersListInfo &in);
bool convertSuppServiceStatus(V1_3::SuppServiceStatus &out,
                              const std::shared_ptr<qcril::interfaces::SuppServiceStatusInfo> &in);
bool convertImsSubConfigInfo(ImsSubConfigInfo &out, qcril::interfaces::ImsSubConfigInfo &in);
void convertVowifiCallQualityInfo(VoWiFiCallQualityInfo &out, qcril::interfaces::VowifiQuality in);
void convertRegBlockStatus(BlockStatus &outStatus, const qcril::interfaces::BlockStatus &in);
void convertConfigInfo(ConfigInfo &outStatus, const qcril::interfaces::ConfigInfo &in);

qcril::interfaces::CallFailCause convertCallFailCause(const V1_3::CallFailCause &in);
V1_3::CallFailCause convertCallFailCause(const qcril::interfaces::CallFailCause &in);
qcril::interfaces::RegState convertRegState(const RegState &in);
RegState convertRegState(const qcril::interfaces::RegState &in);
RIL_RadioTechnology convertRadioTechType(const RadioTechType &in);
RadioTechType convertRadioTechType(const RIL_RadioTechnology &in);
CallState convertCallState(const qcril::interfaces::CallState in);
qcril::interfaces::CallType convertCallType(const CallType &in);
CallType convertCallType(const qcril::interfaces::CallType &in);
qcril::interfaces::CallDomain convertCallDomain(const CallDomain &in);
CallDomain convertCallDomain(const qcril::interfaces::CallDomain &in);
qcril::interfaces::RttMode convertRttMode(const RttMode &in);
RttMode convertRttMode(const qcril::interfaces::RttMode &in);
qcril::interfaces::Presentation convertIpPresentation(const IpPresentation &in);
IpPresentation convertIpPresentation(const qcril::interfaces::Presentation &in);
CallModifyFailCause convertCallModifyFailCause(const qcril::interfaces::CallModifyFailCause &in);
qcril::interfaces::CallModifyFailCause convertCallModifyFailCause(const CallModifyFailCause &in);
ServiceClassStatus convertServiceClassStatus(const qcril::interfaces::ServiceClassStatus &in);
qcril::interfaces::ServiceClassStatus convertServiceClassStatus(const ServiceClassStatus &in);
V1_3::ServiceClassProvisionStatus convertServiceClassProvisionStatus(
    const qcril::interfaces::ServiceClassProvisionStatus &in);
qcril::interfaces::ServiceClassProvisionStatus convertServiceClassProvisionStatus(
    const V1_3::ServiceClassProvisionStatus &in);
ClipStatus convertClipStatus(const qcril::interfaces::ClipStatus &in);
qcril::interfaces::ClipStatus convertClipStatus(const ClipStatus &in);
FacilityType convertFacilityType(const qcril::interfaces::FacilityType &in);
qcril::interfaces::FacilityType convertFacilityType(const FacilityType &in);
EctType convertEctType(const qcril::interfaces::EctType &in);
qcril::interfaces::EctType convertEctType(const EctType &in);
TtyMode convertTtyMode(const qcril::interfaces::TtyMode &in);
qcril::interfaces::TtyMode convertTtyMode(const TtyMode &in);
V1_2::VerificationStatus convertVerificationStatus(const qcril::interfaces::VerificationStatus &in);
ToneOperation convertToneOperation(const qcril::interfaces::RingBackToneOperation &in);
HandoverType convertHandoverType(const qcril::interfaces::HandoverType &in);
ConferenceCallState convertConferenceCallState(const qcril::interfaces::ConferenceCallState &in);
ConfParticipantOperation convertConfParticipantOperation(
    const qcril::interfaces::ConfParticipantOperation &in);
NotificationType convertNotificationType(const qcril::interfaces::NotificationType &in);
BlockReasonType convertBlockReasonType(qcril::interfaces::BlockReasonType reason);
ConfigItem convertConfigItem(qcril::interfaces::ConfigItem in);
qcril::interfaces::ConfigItem convertConfigItem(const ConfigItem in);
ConfigFailureCause convertConfigErrorCause(qcril::interfaces::ConfigFailureCause in);
qcril::interfaces::ConfigFailureCause convertConfigErrorCause(const ConfigFailureCause in);
V1_3::VoiceInfo convertToHidlVoiceInfo(qcril::interfaces::VoiceInfo voiceInfo);
RadioState convertToHidlRadioState(qcril::interfaces::RadioState inState);
MessagePriority convertMessagePriority(qcril::interfaces::MessagePriority in);
MessageType convertMessageType(qcril::interfaces::MessageType in);
StatusType convertStatusType(const qcril::interfaces::StatusType &in);

/*Read from HIDL structures*/
void readGeoLocationInfo(double lat, double lon, const V1_2::AddressInfo &addressInfo,
                         qcril::interfaces::GeoLocationInfo &out);
void readServiceStatusInfo(const ServiceStatusInfo &in, qcril::interfaces::ServiceStatusInfo &out);
void readConfigInfo(const ConfigInfo &inConfig, qcril::interfaces::ConfigInfo &outConfig);
std::shared_ptr<qcril::interfaces::AccessTechnologyStatus> readAccessTechStatus(
    const StatusForAccessTech &in);
std::shared_ptr<qcril::interfaces::Registration> readRegistration(const RegistrationInfo &in);

std::string getCodecStringExtra(uint32_t codec);
bool isServiceTypeIcbQueryWithNumber(SsServiceType serType, SsRequestType reqType);
bool isServiceTypeCfQuery(SsServiceType serType, SsRequestType reqType);
qcril::interfaces::StatusType convertStatusType(const StatusType &in);

// Utility functions
void convertCallListV13ToV12(hidl_vec<V1_3::CallInfo> &callListV13,
                             hidl_vec<V1_2::CallInfo> &callListV12);
void convertCallV13ToV12(V1_3::CallInfo &callInfoV13, V1_2::CallInfo &callInfoV12);
V1_2::CallFailCause convertCallFailCauseV13ToV12(const V1_3::CallFailCause &failcause);
void convertCallListV12ToV11(hidl_vec<V1_2::CallInfo> &callListV12,
                             hidl_vec<V1_1::CallInfo> &callListV11);
void convertCallV12ToV11(V1_2::CallInfo &callInfoV12, V1_1::CallInfo &callInfoV11);
V1_1::CallFailCause convertCallFailCauseV12ToV11(const V1_2::CallFailCause &failcause);
void convertCallListV11ToV10(hidl_vec<V1_1::CallInfo> &callListV11,
                             hidl_vec<V1_0::CallInfo> &callListV10);
void convertCallV11ToV10(V1_1::CallInfo &callInfoV11, V1_0::CallInfo &callInfoV10);
V1_0::CallFailCause convertCallFailCauseToV10(const V1_1::CallFailCause &failcause);
void convertSuppServiceStatusV13ToV10(const V1_3::SuppServiceStatus &suppServiceStatusV13,
                                      V1_0::SuppServiceStatus &suppServiceStatusV10);
void convertColrInfoV13ToV10(const V1_3::ColrInfo &colrInfoV13, V1_0::ColrInfo &colrInfoV10);

void migrateHangup(const V1_0::HangupRequestInfo &from, V1_1::HangupRequestInfo &to);
void migrateHangup(const V1_1::HangupRequestInfo &from, V1_2::HangupRequestInfo &to);
void migrateHangup(const V1_2::HangupRequestInfo &from, V1_3::HangupRequestInfo &to);
void migrateColrInfo(const V1_0::ColrInfo &from, V1_3::ColrInfo &to);
void migrateAddressInfo(const V1_0::AddressInfo &from, V1_2::AddressInfo &to);

bool convertCdmaFormatToPseudoPdu(RIL_CDMA_SMS_Message &cdmaMsg, std::vector<uint8_t> &payload);
void rilErrorConvertToImsError(const RIL_Errno ril_err, const RIL_RadioTechnologyFamily tech,
                               V1_2::ImsSmsSendStatusResult &sms_status_report,
                               V1_2::ImsSmsSendFailureReason &failure_reason);
bool convertBytesToHexString(const uint8_t *bytePayload, size_t size, std::string &str);
bool convertPayloadToCdmaFormat(const uint8_t *payload, size_t size, RIL_CDMA_SMS_Message &cdmaMsg);

qcril::interfaces::DeliverStatus convertHidlToRilSmsAckResult(
    V1_2::ImsSmsDeliverStatusResult smsdeliverResult);

std::string getExtra(std::string key, const hidl_vec<hidl_string> &extras);

}  // namespace utils
}  // namespace V1_0
}  // namespace ims
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
