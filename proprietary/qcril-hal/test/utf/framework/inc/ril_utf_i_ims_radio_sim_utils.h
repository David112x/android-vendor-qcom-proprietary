/******************************************************************************
#  Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once

#include "qcril_log.h"
#include "imsIF.pb.h"
#include "ril_utf_ims_if.h"
#include <framework/legacy.h>
#include <qcrili.h>
#include <vendor/qti/hardware/radio/ims/1.3/types.h>
#include <vendor/qti/hardware/radio/ims/1.4/types.h>
#include <vendor/qti/hardware/radio/ims/1.5/types.h>
#include <vendor/qti/hardware/radio/ims/1.6/types.h>

using ::android::hardware::hidl_vec;
using ::android::hardware::hidl_string;

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_0 {
namespace utils {

    bool isError(ims_Error ret);

    template <typename T> uint32_t __attribute__((noinline)) getCount(T** ptr);

    //Functions to convert hidl to proto.
    //Struct conversions.
    ims_Error convertHidlToProtoDial(const DialRequest& inDial, ims_Dial& outDial);
    ims_Error convertHidlToProtoCallDetails(const CallDetails& inCallDetails,
        ims_CallDetails& outCallDetails);
    ims_Error convertHidlToProtoCallDetailsExtras(
        const android::hardware::hidl_vec<hidl_string>& inCallDetailsExtras,
        char** outCallDetailsExtras);
    ims_Error convertHidlToProtoServiceStatusInfoList(const hidl_vec<ServiceStatusInfo>& inList,
        ims_Info** outList);
    ims_Error convertHidlToProtoHangup(const V1_3::HangupRequestInfo& inHangup,
            ims_Hangup& outHangup);
    ims_Error convertHidlToProtoCallFailCauseResponse(
            const V1_3::CallFailCauseResponse& inFailCause,ims_CallFailCauseResponse& outFailCause);
    ims_Error convertHidlToProtoSipErrorInfo(const SipErrorInfo& inErrorInfo,
            ims_SipErrorInfo& outErrorInfo);
    ims_Error convertHidlToProtoServiceStatusInfo(const ServiceStatusInfo& inServiceInfo,
            ims_Info& outServiceInfo);
    ims_Error convertHidlToProtoStatusForAccessTech(const StatusForAccessTech& inAccTechStatus,
            ims_StatusForAccessTech& outAccTechStatus);
    ims_Error convertHidlToProtoRegistrationInfo(const RegistrationInfo& inRegistration,
            ims_Registration& outRegistration);
    ims_Error convertHidlToProtoDtmf(const DtmfInfo& inDtmf, ims_Dtmf& outDtmf);
    ims_Error convertHidlToProtoBinaryData(const hidl_vec<uint8_t>& inBinaryData,
            qcril_binary_data_type& outBinaryData);
    ims_Error convertHidlToProtoCallModifyInfo(const CallModifyInfo &in, ims_CallModify &out);
    ims_Error convertHidlToProtoCallForwardInfo(const CallForwardInfo& inCallForwardInfo,
        ims_CallForwardInfoList_CallForwardInfo& outCallForwardInfo);
    void convertHidlToProtoCallFwdTimerInfo(const CallFwdTimerInfo& inCallFwdTimerInfo,
         ims_CallFwdTimerInfo& outCallFwdTimerInfo);
    ims_Error convertHidlToProtoString(const hidl_string& inStr, char*& outStr);
    ims_Error convertHidlToProtoConfig(const ConfigInfo& inConfig, ims_ConfigMsg& outConfig);
    ims_Error convertHidlToProtoNumListInfo(const CbNumListInfo& inCbNumListInfo,
        ims_CbNumListType& outCbNumListInfo);
    ims_Error convertHidlToProtoEctInfo(const ExplicitCallTransferInfo &inEctRequest,
            ims_ExplicitCallTransfer &outEctRequest);
    ims_Error convertHidlToProtoNumInfo(const CbNumInfo& inCbNumInfo, ims_CbNumList& outCbNumInfo);
    ims_Error convertHidlToProtoDeflectRequestInfo(const DeflectRequestInfo &inDeflectRequest,
            ims_DeflectCall &outDeflectRequest);
    bool convertHidlToProtoClirInfo(const ClirInfo &in, ims_Clir &out);
    bool convertHidlToProtoSuppServiceStatus(const V1_3::SuppServiceStatus& in,
                                             ims_SuppSvcResponse& out);
    //enum conversions.
    ims_CallDomain convertHidlToProtoCallDomain(const CallDomain& inCallDomain);
    ims_CallType convertHidlToProtoCallType(const CallType& callType);
    ims_CallFailCause convertHidlToProtoCallFailCause(const V1_3::CallFailCause& failCause);
    ims_CallFailCause convertHidlToProtoCallFailCauseV1_2(const V1_2::CallFailCause& failCause);
    ims_CallFailCause convertHidlToProtoCallFailCauseV1_1(const V1_1::CallFailCause& failCause);
    ims_CallFailCause convertHidlToProtoCallFailCauseV1_0(const CallFailCause& failCause);
    ims_Registration_RegState convertHidlToProtoRegState(const RegState& state);
    ims_SrvType convertHidlToProtoServiceType(const ServiceType& type);
    ims_StatusType convertHidlToProtoStatusType(const StatusType& type);
    ims_IpPresentation convertHidlToProtoPresentation(const IpPresentation& inPresentation);
    ims_RadioTechType convertHidlToProtoRadioTech(const RadioTechType& radioTechType);
    ims_Tty_Mode_Type convertHidlToProtoTtyMode(TtyMode ttyMode);
    bool convertHidlToProtoCallModifyFailCause(const CallModifyFailCause &in, ims_Error &out);
    ims_ServiceClassStatus convertHidlToProtoServiceClassStatus(ServiceClassStatus status);
    ims_ServiceClassProvisionStatus convertHidlToProtoServiceClassProvisionStatus(
            V1_3::ServiceClassProvisionStatus status);
    ims_Error convertHidlToProtoConfigItem(const ConfigItem inConfigItem,
            ims_ConfigItem& outConfigItem);
    ims_Error convertHidlToProtoErrorCause(const ConfigFailureCause& inErrorCause,
            ims_ConfigFailureCause& outErrorCause);
    int convertHidlToProtoFacilityType(FacilityType facilityType);
    bool convertHidlToProtoEctType(const EctType &in, ims_EctType &out);
    ims_Error convertHidlToProtoGeolocation(double lat, double lon,
            const V1_2::AddressInfo& inAddress, ims_GeoLocationInfo& outAddress);
    bool copy(const hidl_string& in, char*& out, const char* log_tag);
    void freeGeolocationInfo(ims_GeoLocationInfo& geoInfo);
    void freeAddressInfo(ims_AddressInfo& addressInfo);
    ims_RttMode convertHidlToProtoRttMode(const RttMode& inRttMode);
    RttMode convertProtoToHidlRttMode(const ims_RttMode& inRttMode);
    ims_Handover_Msg_Type convertHidlToProtoHandoverType(const HandoverType &in);
    void convertHidlToProtoHandoverExtra(const Extra &in, ims_Extra &out);

    //Functions to convert proto to hidl
    //Struct conversions.
    ims_Error convertProtoToHidlHangup(const ims_Hangup &in, V1_3::HangupRequestInfo &out);
    ims_Error convertProtoToHidlGeolocation(const ims_GeoLocationInfo &in, V1_2::AddressInfo &out);
    void convertProtoToHidlRegistration(const ims_Registration& inReg, RegistrationInfo& outReg);
    ims_Error convertProtoToHidlSipErrorInfo(const ims_SipErrorInfo& inErrorInfo,
            SipErrorInfo& outErrorInfo);
    void convertProtoToHidlCallList(const ims_CallList& inCallList,
            hidl_vec<V1_3::CallInfo>& outCallList);
    void convertProtoToHidlCall(const ims_CallList_Call& inCall, V1_3::CallInfo& outCall);
    void convertProtoToHidlCallDetails(const ims_CallDetails& inCallDetails,
            CallDetails& outCallDetails);
    void convertProtoToHidlServiceInfoList(const ims_Info** const inServiceInfoList,
            hidl_vec<ServiceStatusInfo>& outServiceInfoList);
    void convertProtoToHidlServiceStatusInfo(const ims_Info& inServiceStatusInfo,
            ServiceStatusInfo& outServiceStatusInfo);
    void convertProtoToHidlStatusForAccessTech(const ims_StatusForAccessTech& inAccTechStatus,
            StatusForAccessTech& outAccTechStatus);
    void convertProtoToHidlCallFailCauseResponse(const ims_CallFailCauseResponse& inFailCause,
            V1_3::CallFailCauseResponse& outFailCause);
    void convertProtoToHidlHandoverInfo(const ims_Handover& inHandover, HandoverInfo& outHandover);
    void convertProtoToHidlHandoverExtra(const ims_Extra& inExtra, Extra& outExtra);
    void convertProtoToHidlSuppServiceStatus(const ims_SuppSvcResponse& inSuppSvcResponse,
        V1_3::SuppServiceStatus& outSuppServiceStatus);
    void convertProtoToHidlSuppServiceStatusV1_0(const ims_SuppSvcResponse& inSuppSvcResponse,
        SuppServiceStatus& outSuppServiceStatus);
    void convertProtoToHidlNumListInfo(const ims_CbNumListType& incbNumListType,
        CbNumListInfo& outCbNumListInfo);
    void convertProtoToHidlNumInfo(const ims_CbNumList& inCbNumList, CbNumInfo& outCbNumInfo);
    void convertProtoToHidlClipProvisionStatus(
        const ims_ClipProvisionStatus& inClipProvisionStatus,
        ClipProvisionStatus& outClipProvisionStatus);
    bool convertProtoToHidlClirInfo(const ims_Clir& inClir, ClirInfo& outClir);
    void convertProtoToHidlColrInfo(const ims_Colr& inColr, V1_3::ColrInfo& outColr);
    void convertProtoToHidlColrInfoV1_0(const ims_Colr& inColr, ColrInfo& outColr);
    void convertProtoToHidlBinaryData(const qcril_binary_data_type* inBinaryData,
        hidl_vec<uint8_t>& outBinaryData);
    bool convertProtoToHidlCallModifyInfo(const ims_CallModify &in, CallModifyInfo &out);
    void convertProtoToHidlCallForwardInfoList(
            const ims_CallForwardInfoList_CallForwardInfo** const inCfInfoList,
            hidl_vec<CallForwardInfo>& outCfInfoList);
    void convertProtoToHidlCallForwardInfo(
            const ims_CallForwardInfoList_CallForwardInfo& inCallForwardInfo,
            CallForwardInfo& outCallForwardInfo);
    void convertProtoToHidlCallForwardTimerInfo(const ims_CallFwdTimerInfo& inCallFwdTimerInfo,
            CallFwdTimerInfo& outCallFwdTimerInfo);
    void convertProtoToHidlSuppServiceNotification(
            const ims_SuppSvcNotification& inSuppServiceNotification,
            SuppServiceNotification& outSuppServiceNotification);
    NotificationType convertProtoToHidlNotificationType(ims_NotificationType inType) ;
    void setDefaultVerstatInfo(V1_3::VerstatInfo& verstatInfo);
    void convertToHidlVerstatInfo(
            const ims_VerstatInfo& inVerstatInfo,
            V1_3::VerstatInfo& outVerstatInfo);
    //enum conversions.
    RegState convertProtoToHidlRegState(const ims_Registration_RegState& state);
    RadioTechType convertProtoToHidlRadioTech(const ims_RadioTechType& radioTechType);
    CallState convertProtoToHidlCallState(const ims_CallState& inState);
    CallType convertProtoToHidlCallType(const ims_CallType& callType);
    CallDomain convertProtoToHidlCallDomain(const ims_CallDomain& callDomain);
    CallDomain convertProtoToHidlCallDomain(const ims_CallDomain& inCallDomain);
    ServiceType convertProtoToHidlServiceType(const ims_SrvType& type);
    StatusType convertProtoToHidlStatusType(const ims_StatusType& status);
    V1_3::CallFailCause convertProtoToHidlCallFailCause(const ims_CallFailCause& failCause);
    CallFailCause convertProtoToHidlCallFailCauseV1_0(const ims_CallFailCause& failCause);
    ToneOperation convertProtoToHidlTone(const ims_RingBackTone& inTone);
    ToneOperation convertProtoToHidlToneOperation(const ims_RingBackTone_ToneFlag& flag);
    HandoverType convertProtoToHidlHandoverType(const ims_Handover_Msg_Type& inType);
    ServiceClassStatus convertProtoToHidlServiceClassStatus(
            ims_ServiceClassStatus serviceClassStatus);
    V1_3::ServiceClassProvisionStatus convertProtoToHidlServiceClassProvisionStatus(
        ims_ServiceClassProvisionStatus provisionStatus);
    FacilityType convertProtoToHidlFacilityType(int facilityType);
    TtyMode convertProtoToHidlTtyMode(ims_Tty_Mode_Type ttyMode);
    ClipStatus convertProtoToHidlClipStatus(ims_ClipStatus clipStatus);
    IpPresentation convertProtoToHidlPresentation(ims_IpPresentation presentation);
    ConferenceCallState convertProtoToHidlConferenceCallState(
        ims_ConfCallState conferenceCallState);
    MessageType convertProtoToHidlMwiMessageType(ims_MwiMessageType mwiMessageType);
    MessagePriority convertProtoToHidlMwiMessagePriority(ims_MwiPriority mwiPriority);

    bool convertProtoToHidlCallModifyFailCause(const ims_Error &in, CallModifyFailCause &out);
    void convertProtoToHidlImsSubConfig(const ims_ImsSubConfig& subConfig,
            ImsSubConfigInfo& subConfigInfo);
    void convertProtoToHidlParticipantStatusInfo(const ims_ParticipantStatusInfo& in,
            ParticipantStatusInfo& out);
    void convertProtoToHidlConfParticipantOperation(const ims_ConfParticipantOperation& in,
            ConfParticipantOperation& out);
    void convertProtoToHidlBlockStatus(const ims_BlockStatus &inStatus, BlockStatus &outStatus);
    void convertProtoToHidlBlockReasonDetails(const ims_BlockReasonDetails &inDetails,
            BlockReasonDetails &outDetails);
    BlockReasonType convertProtoToHidlBlockReasonType(ims_BlockReasonType reason);
    RegFailureReasonType convertProtoToHidlRegFailureReasonType(ims_RegFailureReasonType reason);
    VoWiFiCallQualityInfo convertProtoToHidlVoWiFiCallQuality(const ims_VoWiFiCallQuality&
        voWiFiCallQuality);
    void convertToHidlStkCcUnsolSsResult(ims_ril_ss_ind_type &in, StkCcUnsolSsResult &out);
    void convertToHidlAutoCallRejectionInfo(const ims_AutoCallRejectionInfo& in,
        V1_3::AutoCallRejectionInfo& out);
    V1_3::VoiceInfo convertToHidlVoiceInfo(ims_VoiceInfo voiceInfo);

    V1_2::VerificationStatus convertToHidlVerstatVerificationStatus(
            ims_VerstatVerificationStatus status);

    RadioState convertProtoToHidlRadioState(const ims_RadioStateChanged& inState);
    ConfigFailureCause convertProtoToHidlErrorCause(const ims_ConfigFailureCause& inErrorCause);
    ConfigItem convertProtoToHidlConfigItem(const ims_ConfigItem& outConfigItem);
    void convertProtoToHidlConfig(const ims_ConfigMsg& inConfig, ConfigInfo& outConfig);
    void convertProtoToHidlMessageWaitingIndication(const ims_Mwi& mwi,
            MessageWaitingIndication& messageWaitingIndication);
    void convertProtoToHidlMessageSummary(const ims_MwiMessageSummary& mwiMessageSummary,
            MessageSummary& messageSummary);
    void convertProtoToHidlMessageDetails(const ims_MwiMessageDetails& mwiMessageDetails,
            MessageDetails& messageDetails);

    bool convertStkCcUnsolSsResult(StkCcUnsolSsResult &out, RIL_StkCcUnsolSsResponse &in);

    void convertProtoToHidlString(const char * inStr, hidl_string& outStr);

    SsServiceType convertToHidlSsServiceType(const int &serviceType);
    SsRequestType convertToHidlSsRequestType(const RIL_SsRequestType &type);
    SsTeleserviceType convertToHidlSsTeleserviceType(const RIL_SsTeleserviceType &type);

    bool convertHidlToProtoImsSubConfig(const ImsSubConfigInfo &in, ims_ImsSubConfig &out);
    ims_CallState convertHidlToProtoCallState(const CallState &inState);

    bool convertHidlToProtoServiceInfoList(const ::android::hardware::hidl_vec<ServiceStatusInfo> &in,
        ims_SrvStatusList &out);
    void convertHidlToProtoHandoverInfo(const HandoverInfo &in, ims_Handover &out);
    ims_RingBackTone_ToneFlag convertHidlToProtoToneOperation(const ToneOperation &in);
    ims_NotificationType convertHidlToProtoNotificationType(const NotificationType &in);
    void convertHidlToProtoSuppServiceNotification(const SuppServiceNotification &in,
        ims_SuppSvcNotification &out);
    void convertHidlToProtoConfParticipantOperation(const ConfParticipantOperation &in,
        ims_ConfParticipantOperation &out);
    void convertHidlToProtoParticipantStatusInfo(const ParticipantStatusInfo &in,
        ims_ParticipantStatusInfo &out);
    void convertHidlToProtoBlockStatus(const ::vendor::qti::hardware::radio::ims::V1_0::BlockStatus &in,
        ims_BlockStatus &out);
    ims_Error convertProtoToHidlEctInfo(ExplicitCallTransferInfo &out,
        const ims_ExplicitCallTransfer &in);
    ims_Error convertProtoToHidlDeflectRequestInfo(DeflectRequestInfo &out, const ims_DeflectCall &in);
    ims_Error convertHidlToInternalImsSmsResponse(V1_2::ImsSmsSendStatusResult smsStatus,
        V1_2::ImsSmsSendFailureReason reason);
    ims_Error convertHidlToInternalImsSmsResponse(V1_2::ImsSmsSendStatusResult smsStatus,
        V1_5::ImsSmsSendFailureReason reason);
    void convertHidlToInternalIncomingSmsResult(const V1_2::IncomingImsSms& in, utf_ims_mt_sms_t& out);
    void convertInternalToHidlImsSendSms(const RIL_IMS_SMS_Message& in, V1_2::ImsSmsMessage& out);
} // namespace utils
} // namespace V1_0
} // namespace ims
} // namespace radio
} // namespace hardware
} // namespace qti
} // namespace vendor

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_4 {
namespace utils {

// Functions to convert hidl to proto.
// Struct conversions.
ims_Error convertHidlToProtoDial(const V1_4::DialRequest &inDial, ims_Dial &outDial);
ims_Error convertProtoToHidlDial(const ims_Dial &in, V1_4::DialRequest &out);

// Functions to convert proto to hidl
// Struct conversions.
void convertProtoToHidlCallList(const ims_CallList &inCallList,
                                hidl_vec<V1_4::CallInfo> &outCallList);
void convertProtoToHidlCall(const ims_CallList_Call &inCall, V1_4::CallInfo &outCall);
bool convertHidlToProtoCall(const V1_4::CallInfo &in, ims_CallList_Call &out);
bool convertHidlToProtoCallList(const ::android::hardware::hidl_vec<V1_4::CallInfo> &in,
                                ims_CallList &out);
void convertMultiIdentityLineInfo(const ims_MultiIdentityStatus &in,
    hidl_vec<V1_4::MultiIdentityLineInfoHal> &out);
void convertMultiIdentityLineInfo(const hidl_vec<V1_4::MultiIdentityLineInfoHal> &in,
    ims_MultiIdentityStatus &out);

// Utility functions
}  // namespace utils
}  // namespace V1_4
}  // namespace ims
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_5 {
namespace utils {

void convertProtoToHidlCallList(const ims_CallList &inCallList,
                                hidl_vec<V1_5::CallInfo> &outCallList);
void convertProtoToHidlCall(const ims_CallList_Call &inCall, V1_5::CallInfo &outCall);
void convertProtoToHidlCallFailCauseResponse(const ims_CallFailCauseResponse &inFailCause,
                                             V1_5::CallFailCauseResponse &outFailCause);
V1_5::CallFailCause convertProtoToHidlCallFailCause(const ims_CallFailCause &failCause);
void convertToHidlAutoCallRejectionInfo(const ims_AutoCallRejectionInfo &in,
                                        V1_5::AutoCallRejectionInfo &out);
ims_Error convertHidlToProtoConfigItem(const V1_5::ConfigItem inConfigItem,
    ims_ConfigItem& outConfigItem);
V1_5::ConfigItem convertProtoToHidlConfigItem(const ims_ConfigItem& in);

// Functions to convert hidl to proto.
// Struct conversions.
V1_5::CallFailCause fromProtoToHidlCallFailCause_1_5(const ims_CallFailCause &failCause);
ims_CallFailCause convertHidlToProtoCallFailCause(const V1_5::CallFailCause &failCause);
ims_CallFailCause fromHidlToProtoCallFailCause(const V1_5::CallFailCause &failCause);
V1_3::CallFailCause toCallFailCauseV13(const V1_5::CallFailCause &from);
void toCallInfoV14(V1_5::CallInfo &from, V1_4::CallInfo &to);
void toCallListV14(hidl_vec<V1_5::CallInfo> &from, hidl_vec<V1_4::CallInfo> &to);
void toAutoCallRejectionV13(const V1_5::AutoCallRejectionInfo &from,
                            V1_3::AutoCallRejectionInfo &to);
void toVerstatInfoV13(const V1_3::VerstatInfo &from, V1_3::VerstatInfo &to);
void toMtMultiLineInfoV14(const V1_4::MultiIdentityLineInfoHal &from,
                          V1_4::MultiIdentityLineInfoHal &to);
ims_TirMode convertHidlToProtoTirMode(const V1_5::TirMode& tirMode);
bool convertHidlToProtoCallForwardStatusInfo(const V1_5::CallForwardStatusInfo &in,
                                         ims_CallForwardStatusInfo &out);
ims_Error convertHidlToProtoCallFailCauseResponse(const V1_5::CallFailCauseResponse& inFailCause,
                                                  ims_CallFailCauseResponse& outFailCause);
bool convertHidlToProtoCall(const V1_5::CallInfo &in, ims_CallList_Call &out);
bool convertHidlToProtoCallList(const ::android::hardware::hidl_vec<V1_5::CallInfo> &in,
                                ims_CallList &out);
bool convertHidlToProtoAutoCallRejectionInfo(const V1_5::AutoCallRejectionInfo& in,
                                             ims_AutoCallRejectionInfo& out);
}  // namespace utils
}  // namespace V1_5
}  // namespace ims
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_6 {
namespace utils {
ims_RadioTechType convertHidlToProtoRadioTech(const V1_6::RadioTechType& radioTechType);
ims_Error convertHidlToProtoCallDetails(const V1_6::CallDetails& inCallDetails,
                                        ims_CallDetails& outCallDetails);
ims_Error convertHidlToProtoServiceStatusInfoList(const hidl_vec<V1_6::ServiceStatusInfo>& inList,
        ims_Info** outList);
ims_Error convertHidlToProtoServiceStatusInfo(const V1_6::ServiceStatusInfo& inServiceInfo,
                                              ims_Info& outServiceInfo);
bool convertHidlToProtoServiceInfoList(
    const ::android::hardware::hidl_vec<V1_6::ServiceStatusInfo>& in, ims_SrvStatusList& out);
ims_Error convertHidlToProtoRegistrationInfo(const V1_6::RegistrationInfo& inRegistration,
    ims_Registration& outRegistration);
ims_Error convertHidlToProtoConfig(const V1_6::ConfigInfo& inConfig, ims_ConfigMsg& outConfig);
ims_Error convertHidlToProtoConfigItem(const V1_6::ConfigItem inConfigItem,
    ims_ConfigItem& outConfigItem);
V1_6::ConfigItem convertProtoToHidlConfigItem(const ims_ConfigItem& in);
void convertHidlToProtoHandoverInfo(const V1_6::HandoverInfo &in, ims_Handover &out);
void convertProtoToHidlConfig(const ims_ConfigMsg& inConfig, V1_6::ConfigInfo& outConfig);
bool convertHidlToProtoCall(const V1_6::CallInfo &in, ims_CallList_Call &out);
bool convertHidlToProtoCallList(const ::android::hardware::hidl_vec<V1_6::CallInfo> &in,
                                ims_CallList &out);
} // namespace utils
} // namespace V1_6
} // namespace ims
} // namespace radio
} // namespace hardware
} // namespace qti
} // namespace vendor
