/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_0_TYPES_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_0_TYPES_H

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_0 {

// Forward declaration for forward reference support:
enum class IpPresentation : int32_t;
enum class CallType : int32_t;
enum class CallDomain : int32_t;
enum class CallFailCause : int32_t;
enum class RegState : int32_t;
enum class StatusType : int32_t;
enum class RadioTechType : int32_t;
enum class ConfigItem : int32_t;
enum class ConfigFailureCause : int32_t;
enum class ToneOperation : int32_t;
enum class HandoverType : int32_t;
enum class CallState : int32_t;
enum class ServiceType : int32_t;
enum class ExtraType : int32_t;
enum class RadioState : int32_t;
enum class ServiceClassStatus : int32_t;
enum class FacilityType : int32_t;
enum class ClipStatus : int32_t;
enum class TtyMode : int32_t;
enum class ConferenceCallState : int32_t;
enum class CallModifyFailCause : int32_t;
enum class NotificationType : int32_t;
enum class MessageType : int32_t;
enum class MessagePriority : int32_t;
enum class EctType : int32_t;
enum class ConfParticipantOperation : int32_t;
enum class BlockReasonType : int32_t;
enum class RegFailureReasonType : int32_t;
enum class RttMode : int32_t;
struct RegistrationInfo;
struct StatusForAccessTech;
struct ServiceStatusInfo;
struct CallDetails;
struct DialRequest;
struct SipErrorInfo;
struct CallFailCauseResponse;
struct HangupRequestInfo;
struct ConfigInfo;
struct Extra;
struct HandoverInfo;
struct CallInfo;
struct CbNumInfo;
struct CbNumListInfo;
struct SuppServiceStatus;
struct ClipProvisionStatus;
struct ClirInfo;
struct ColrInfo;
struct DtmfInfo;
struct TtyInfo;
struct ConferenceInfo;
struct ViceInfo;
struct CallModifyInfo;
struct CallFwdTimerInfo;
struct CallForwardInfo;
struct SuppServiceNotification;
struct ExplicitCallTransferInfo;
struct MessageSummary;
struct MessageDetails;
struct MessageWaitingIndication;
struct DeflectRequestInfo;
struct AddressInfo;
struct ImsSubConfigInfo;
struct ParticipantStatusInfo;
struct BlockReasonDetails;
struct BlockStatus;
enum class VoWiFiCallQualityInfo : int32_t;
enum class SsServiceType : int32_t;
enum class SsRequestType : int32_t;
enum class SsTeleserviceType : int32_t;
struct CfData;
struct SsInfoData;
struct StkCcUnsolSsResult;

enum class IpPresentation : int32_t {
    IP_PRESENTATION_NUM_ALLOWED = 0,
    IP_PRESENTATION_NUM_RESTRICTED = 1, // (::vendor::qti::hardware::radio::ims::V1_0::IpPresentation.IP_PRESENTATION_NUM_ALLOWED implicitly + 1)
    IP_PRESENTATION_NUM_DEFAULT = 2, // (::vendor::qti::hardware::radio::ims::V1_0::IpPresentation.IP_PRESENTATION_NUM_RESTRICTED implicitly + 1)
    IP_PRESENTATION_INVALID = 3, // (::vendor::qti::hardware::radio::ims::V1_0::IpPresentation.IP_PRESENTATION_NUM_DEFAULT implicitly + 1)
};

enum class CallType : int32_t {
    CALL_TYPE_VOICE = 0,
    CALL_TYPE_VT_TX = 1, // (::vendor::qti::hardware::radio::ims::V1_0::CallType.CALL_TYPE_VOICE implicitly + 1)
    CALL_TYPE_VT_RX = 2, // (::vendor::qti::hardware::radio::ims::V1_0::CallType.CALL_TYPE_VT_TX implicitly + 1)
    CALL_TYPE_VT = 3, // (::vendor::qti::hardware::radio::ims::V1_0::CallType.CALL_TYPE_VT_RX implicitly + 1)
    CALL_TYPE_VT_NODIR = 4, // (::vendor::qti::hardware::radio::ims::V1_0::CallType.CALL_TYPE_VT implicitly + 1)
    CALL_TYPE_CS_VS_TX = 5, // (::vendor::qti::hardware::radio::ims::V1_0::CallType.CALL_TYPE_VT_NODIR implicitly + 1)
    CALL_TYPE_CS_VS_RX = 6, // (::vendor::qti::hardware::radio::ims::V1_0::CallType.CALL_TYPE_CS_VS_TX implicitly + 1)
    CALL_TYPE_PS_VS_TX = 7, // (::vendor::qti::hardware::radio::ims::V1_0::CallType.CALL_TYPE_CS_VS_RX implicitly + 1)
    CALL_TYPE_PS_VS_RX = 8, // (::vendor::qti::hardware::radio::ims::V1_0::CallType.CALL_TYPE_PS_VS_TX implicitly + 1)
    CALL_TYPE_UNKNOWN = 9, // (::vendor::qti::hardware::radio::ims::V1_0::CallType.CALL_TYPE_PS_VS_RX implicitly + 1)
    CALL_TYPE_SMS = 10, // (::vendor::qti::hardware::radio::ims::V1_0::CallType.CALL_TYPE_UNKNOWN implicitly + 1)
    CALL_TYPE_UT = 11, // (::vendor::qti::hardware::radio::ims::V1_0::CallType.CALL_TYPE_SMS implicitly + 1)
    CALL_TYPE_INVALID = 12, // (::vendor::qti::hardware::radio::ims::V1_0::CallType.CALL_TYPE_UT implicitly + 1)
};

enum class CallDomain : int32_t {
    CALL_DOMAIN_UNKNOWN = 0,
    CALL_DOMAIN_CS = 1, // (::vendor::qti::hardware::radio::ims::V1_0::CallDomain.CALL_DOMAIN_UNKNOWN implicitly + 1)
    CALL_DOMAIN_PS = 2, // (::vendor::qti::hardware::radio::ims::V1_0::CallDomain.CALL_DOMAIN_CS implicitly + 1)
    CALL_DOMAIN_AUTOMATIC = 3, // (::vendor::qti::hardware::radio::ims::V1_0::CallDomain.CALL_DOMAIN_PS implicitly + 1)
    CALL_DOMAIN_NOT_SET = 4, // (::vendor::qti::hardware::radio::ims::V1_0::CallDomain.CALL_DOMAIN_AUTOMATIC implicitly + 1)
    CALL_DOMAIN_INVALID = 5, // (::vendor::qti::hardware::radio::ims::V1_0::CallDomain.CALL_DOMAIN_NOT_SET implicitly + 1)
};

enum class CallFailCause : int32_t {
    CALL_FAIL_UNOBTAINABLE_NUMBER = 0,
    CALL_FAIL_NORMAL = 1, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_UNOBTAINABLE_NUMBER implicitly + 1)
    CALL_FAIL_BUSY = 2, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_NORMAL implicitly + 1)
    CALL_FAIL_NORMAL_UNSPECIFIED = 3, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_BUSY implicitly + 1)
    CALL_FAIL_CONGESTION = 4, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_NORMAL_UNSPECIFIED implicitly + 1)
    CALL_FAIL_INCOMPATIBILITY_DESTINATION = 5, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_CONGESTION implicitly + 1)
    CALL_FAIL_CALL_BARRED = 6, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_INCOMPATIBILITY_DESTINATION implicitly + 1)
    CALL_FAIL_FDN_BLOCKED = 7, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_CALL_BARRED implicitly + 1)
    CALL_FAIL_IMEI_NOT_ACCEPTED = 8, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_FDN_BLOCKED implicitly + 1)
    CALL_FAIL_DIAL_MODIFIED_TO_USSD = 9, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_IMEI_NOT_ACCEPTED implicitly + 1)
    CALL_FAIL_DIAL_MODIFIED_TO_SS = 10, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_DIAL_MODIFIED_TO_USSD implicitly + 1)
    CALL_FAIL_DIAL_MODIFIED_TO_DIAL = 11, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_DIAL_MODIFIED_TO_SS implicitly + 1)
    CALL_FAIL_EMERGENCY_TEMP_FAILURE = 12, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_DIAL_MODIFIED_TO_DIAL implicitly + 1)
    CALL_FAIL_EMERGENCY_PERM_FAILURE = 13, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_EMERGENCY_TEMP_FAILURE implicitly + 1)
    CALL_FAIL_HO_NOT_FEASIBLE = 14, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_EMERGENCY_PERM_FAILURE implicitly + 1)
    CALL_FAIL_USER_BUSY = 501,
    CALL_FAIL_USER_REJECT = 502,
    CALL_FAIL_LOW_BATTERY = 503,
    CALL_FAIL_BLACKLISTED_CALL_ID = 504, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_LOW_BATTERY implicitly + 1)
    CALL_FAIL_CS_RETRY_REQUIRED = 505, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_BLACKLISTED_CALL_ID implicitly + 1)
    CALL_FAIL_NETWORK_UNAVAILABLE = 506, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_CS_RETRY_REQUIRED implicitly + 1)
    CALL_FAIL_FEATURE_UNAVAILABLE = 507, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_NETWORK_UNAVAILABLE implicitly + 1)
    CALL_FAIL_SIP_ERROR = 508, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_FEATURE_UNAVAILABLE implicitly + 1)
    CALL_FAIL_MISC = 509, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_ERROR implicitly + 1)
    CALL_FAIL_ANSWERED_ELSEWHERE = 510, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_MISC implicitly + 1)
    CALL_FAIL_PULL_OUT_OF_SYNC = 511, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_ANSWERED_ELSEWHERE implicitly + 1)
    CALL_FAIL_CAUSE_CALL_PULLED = 512, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_PULL_OUT_OF_SYNC implicitly + 1)
    CALL_FAIL_ACCESS_CLASS_BLOCKED = 513, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_CAUSE_CALL_PULLED implicitly + 1)
    CALL_FAIL_SIP_REDIRECTED = 514, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_ACCESS_CLASS_BLOCKED implicitly + 1)
    CALL_FAIL_SIP_BAD_REQUEST = 515, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_REDIRECTED implicitly + 1)
    CALL_FAIL_SIP_FORBIDDEN = 516, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_BAD_REQUEST implicitly + 1)
    CALL_FAIL_SIP_NOT_FOUND = 517, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_FORBIDDEN implicitly + 1)
    CALL_FAIL_SIP_NOT_SUPPORTED = 518, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_NOT_FOUND implicitly + 1)
    CALL_FAIL_SIP_REQUEST_TIMEOUT = 519, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_NOT_SUPPORTED implicitly + 1)
    CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE = 520, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_REQUEST_TIMEOUT implicitly + 1)
    CALL_FAIL_SIP_BAD_ADDRESS = 521, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE implicitly + 1)
    CALL_FAIL_SIP_BUSY = 522, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_BAD_ADDRESS implicitly + 1)
    CALL_FAIL_SIP_REQUEST_CANCELLED = 523, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_BUSY implicitly + 1)
    CALL_FAIL_SIP_NOT_ACCEPTABLE = 524, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_REQUEST_CANCELLED implicitly + 1)
    CALL_FAIL_SIP_NOT_REACHABLE = 525, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_NOT_ACCEPTABLE implicitly + 1)
    CALL_FAIL_SIP_SERVER_INTERNAL_ERROR = 526, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_NOT_REACHABLE implicitly + 1)
    CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED = 527, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_SERVER_INTERNAL_ERROR implicitly + 1)
    CALL_FAIL_SIP_SERVER_BAD_GATEWAY = 528, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED implicitly + 1)
    CALL_FAIL_SIP_SERVICE_UNAVAILABLE = 529, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_SERVER_BAD_GATEWAY implicitly + 1)
    CALL_FAIL_SIP_SERVER_TIMEOUT = 530, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_SERVICE_UNAVAILABLE implicitly + 1)
    CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED = 531, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_SERVER_TIMEOUT implicitly + 1)
    CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE = 532, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED implicitly + 1)
    CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE = 533, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE implicitly + 1)
    CALL_FAIL_SIP_USER_REJECTED = 534, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE implicitly + 1)
    CALL_FAIL_SIP_GLOBAL_ERROR = 535, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_USER_REJECTED implicitly + 1)
    CALL_FAIL_MEDIA_INIT_FAILED = 536, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_GLOBAL_ERROR implicitly + 1)
    CALL_FAIL_MEDIA_NO_DATA = 537, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_MEDIA_INIT_FAILED implicitly + 1)
    CALL_FAIL_MEDIA_NOT_ACCEPTABLE = 538, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_MEDIA_NO_DATA implicitly + 1)
    CALL_FAIL_MEDIA_UNSPECIFIED_ERROR = 539, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_MEDIA_NOT_ACCEPTABLE implicitly + 1)
    CALL_FAIL_HOLD_RESUME_FAILED = 540, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_MEDIA_UNSPECIFIED_ERROR implicitly + 1)
    CALL_FAIL_HOLD_RESUME_CANCELED = 541, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_HOLD_RESUME_FAILED implicitly + 1)
    CALL_FAIL_HOLD_REINVITE_COLLISION = 542, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_HOLD_RESUME_CANCELED implicitly + 1)
    CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO = 543, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_HOLD_REINVITE_COLLISION implicitly + 1)
    CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL = 544, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO implicitly + 1)
    CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO = 545, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL implicitly + 1)
    CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS = 546, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO implicitly + 1)
    CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD = 547, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS implicitly + 1)
    CALL_FAIL_ERROR_UNSPECIFIED = 548, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD implicitly + 1)
    CALL_FAIL_INVALID = 549, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_ERROR_UNSPECIFIED implicitly + 1)
};

enum class RegState : int32_t {
    REGISTERED = 0,
    NOT_REGISTERED = 1, // (::vendor::qti::hardware::radio::ims::V1_0::RegState.REGISTERED implicitly + 1)
    REGISTERING = 2, // (::vendor::qti::hardware::radio::ims::V1_0::RegState.NOT_REGISTERED implicitly + 1)
    INVALID = 3, // (::vendor::qti::hardware::radio::ims::V1_0::RegState.REGISTERING implicitly + 1)
};

enum class StatusType : int32_t {
    STATUS_DISABLED = 0,
    STATUS_PARTIALLY_ENABLED = 1, // (::vendor::qti::hardware::radio::ims::V1_0::StatusType.STATUS_DISABLED implicitly + 1)
    STATUS_ENABLED = 2, // (::vendor::qti::hardware::radio::ims::V1_0::StatusType.STATUS_PARTIALLY_ENABLED implicitly + 1)
    STATUS_NOT_SUPPORTED = 3, // (::vendor::qti::hardware::radio::ims::V1_0::StatusType.STATUS_ENABLED implicitly + 1)
    STATUS_INVALID = 4, // (::vendor::qti::hardware::radio::ims::V1_0::StatusType.STATUS_NOT_SUPPORTED implicitly + 1)
};

enum class RadioTechType : int32_t {
    RADIO_TECH_ANY = 0,
    RADIO_TECH_UNKNOWN = 1, // (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_ANY implicitly + 1)
    RADIO_TECH_GPRS = 2, // (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_UNKNOWN implicitly + 1)
    RADIO_TECH_EDGE = 3, // (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_GPRS implicitly + 1)
    RADIO_TECH_UMTS = 4, // (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_EDGE implicitly + 1)
    RADIO_TECH_IS95A = 5, // (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_UMTS implicitly + 1)
    RADIO_TECH_IS95B = 6, // (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_IS95A implicitly + 1)
    RADIO_TECH_1xRTT = 7, // (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_IS95B implicitly + 1)
    RADIO_TECH_EVDO_0 = 8, // (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_1xRTT implicitly + 1)
    RADIO_TECH_EVDO_A = 9, // (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_EVDO_0 implicitly + 1)
    RADIO_TECH_HSDPA = 10, // (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_EVDO_A implicitly + 1)
    RADIO_TECH_HSUPA = 11, // (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_HSDPA implicitly + 1)
    RADIO_TECH_HSPA = 12, // (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_HSUPA implicitly + 1)
    RADIO_TECH_EVDO_B = 13, // (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_HSPA implicitly + 1)
    RADIO_TECH_EHRPD = 14, // (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_EVDO_B implicitly + 1)
    RADIO_TECH_LTE = 15, // (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_EHRPD implicitly + 1)
    RADIO_TECH_HSPAP = 16, // (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_LTE implicitly + 1)
    RADIO_TECH_GSM = 17, // (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_HSPAP implicitly + 1)
    RADIO_TECH_TD_SCDMA = 18, // (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_GSM implicitly + 1)
    RADIO_TECH_WIFI = 19, // (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_TD_SCDMA implicitly + 1)
    RADIO_TECH_IWLAN = 20, // (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_WIFI implicitly + 1)
    RADIO_TECH_INVALID = 21, // (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_IWLAN implicitly + 1)
};

enum class ConfigItem : int32_t {
    CONFIG_ITEM_NONE = 0,
    CONFIG_ITEM_VOCODER_AMRMODESET = 1, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_NONE implicitly + 1)
    CONFIG_ITEM_VOCODER_AMRWBMODESET = 2, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VOCODER_AMRMODESET implicitly + 1)
    CONFIG_ITEM_SIP_SESSION_TIMER = 3, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VOCODER_AMRWBMODESET implicitly + 1)
    CONFIG_ITEM_MIN_SESSION_EXPIRY = 4, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_SESSION_TIMER implicitly + 1)
    CONFIG_ITEM_CANCELLATION_TIMER = 5, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_MIN_SESSION_EXPIRY implicitly + 1)
    CONFIG_ITEM_T_DELAY = 6, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_CANCELLATION_TIMER implicitly + 1)
    CONFIG_ITEM_SILENT_REDIAL_ENABLE = 7, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_T_DELAY implicitly + 1)
    CONFIG_ITEM_SIP_T1_TIMER = 8, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SILENT_REDIAL_ENABLE implicitly + 1)
    CONFIG_ITEM_SIP_T2_TIMER = 9, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_T1_TIMER implicitly + 1)
    CONFIG_ITEM_SIP_TF_TIMER = 10, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_T2_TIMER implicitly + 1)
    CONFIG_ITEM_VLT_SETTING_ENABLED = 11, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_TF_TIMER implicitly + 1)
    CONFIG_ITEM_LVC_SETTING_ENABLED = 12, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VLT_SETTING_ENABLED implicitly + 1)
    CONFIG_ITEM_DOMAIN_NAME = 13, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_LVC_SETTING_ENABLED implicitly + 1)
    CONFIG_ITEM_SMS_FORMAT = 14, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_DOMAIN_NAME implicitly + 1)
    CONFIG_ITEM_SMS_OVER_IP = 15, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SMS_FORMAT implicitly + 1)
    CONFIG_ITEM_PUBLISH_TIMER = 16, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SMS_OVER_IP implicitly + 1)
    CONFIG_ITEM_PUBLISH_TIMER_EXTENDED = 17, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_PUBLISH_TIMER implicitly + 1)
    CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION = 18, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_PUBLISH_TIMER_EXTENDED implicitly + 1)
    CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION = 19, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION implicitly + 1)
    CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL = 20, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION implicitly + 1)
    CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH = 21, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL implicitly + 1)
    CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL = 22, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH implicitly + 1)
    CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP = 23, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL implicitly + 1)
    CONFIG_ITEM_GZIP_FLAG = 24, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP implicitly + 1)
    CONFIG_ITEM_EAB_SETTING_ENABLED = 25, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_GZIP_FLAG implicitly + 1)
    CONFIG_ITEM_MOBILE_DATA_ENABLED = 26, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_EAB_SETTING_ENABLED implicitly + 1)
    CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED = 27, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_MOBILE_DATA_ENABLED implicitly + 1)
    CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING = 28, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED implicitly + 1)
    CONFIG_ITEM_VOICE_OVER_WIFI_MODE = 29, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING implicitly + 1)
    CONFIG_ITEM_CAPABILITY_DISCOVERY_ENABLED = 30, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VOICE_OVER_WIFI_MODE implicitly + 1)
    CONFIG_ITEM_EMERGENCY_CALL_TIMER = 31, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_CAPABILITY_DISCOVERY_ENABLED implicitly + 1)
    CONFIG_ITEM_SSAC_HYSTERESIS_TIMER = 32, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_EMERGENCY_CALL_TIMER implicitly + 1)
    CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS = 33, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SSAC_HYSTERESIS_TIMER implicitly + 1)
    CONFIG_ITEM_LBO_PCSCF_ADDRESS = 34, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS implicitly + 1)
    CONFIG_ITEM_KEEP_ALIVE_ENABLED = 35, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_LBO_PCSCF_ADDRESS implicitly + 1)
    CONFIG_ITEM_REGISTRATION_RETRY_BASE_TIME_SEC = 36, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_KEEP_ALIVE_ENABLED implicitly + 1)
    CONFIG_ITEM_REGISTRATION_RETRY_MAX_TIME_SEC = 37, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_REGISTRATION_RETRY_BASE_TIME_SEC implicitly + 1)
    CONFIG_ITEM_SPEECH_START_PORT = 38, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_REGISTRATION_RETRY_MAX_TIME_SEC implicitly + 1)
    CONFIG_ITEM_SPEECH_END_PORT = 39, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SPEECH_START_PORT implicitly + 1)
    CONFIG_ITEM_SIP_INVITE_REQ_RETX_INTERVAL_MSEC = 40, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SPEECH_END_PORT implicitly + 1)
    CONFIG_ITEM_SIP_INVITE_RSP_WAIT_TIME_MSEC = 41, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_INVITE_REQ_RETX_INTERVAL_MSEC implicitly + 1)
    CONFIG_ITEM_SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC = 42, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_INVITE_RSP_WAIT_TIME_MSEC implicitly + 1)
    CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC = 43, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC implicitly + 1)
    CONFIG_ITEM_SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC = 44, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC implicitly + 1)
    CONFIG_ITEM_SIP_INVITE_RSP_RETX_INTERVAL_MSEC = 45, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC implicitly + 1)
    CONFIG_ITEM_SIP_ACK_RECEIPT_WAIT_TIME_MSEC = 46, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_INVITE_RSP_RETX_INTERVAL_MSEC implicitly + 1)
    CONFIG_ITEM_SIP_ACK_RETX_WAIT_TIME_MSEC = 47, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_ACK_RECEIPT_WAIT_TIME_MSEC implicitly + 1)
    CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC = 48, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_ACK_RETX_WAIT_TIME_MSEC implicitly + 1)
    CONFIG_ITEM_SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC = 49, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC implicitly + 1)
    CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT = 50, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC implicitly + 1)
    CONFIG_ITEM_AMR_WB_BANDWIDTH_EFFICIENT_PT = 51, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT implicitly + 1)
    CONFIG_ITEM_AMR_OCTET_ALIGNED_PT = 52, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_AMR_WB_BANDWIDTH_EFFICIENT_PT implicitly + 1)
    CONFIG_ITEM_AMR_BANDWIDTH_EFFICIENT_PT = 53, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_AMR_OCTET_ALIGNED_PT implicitly + 1)
    CONFIG_ITEM_DTMF_WB_PT = 54, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_AMR_BANDWIDTH_EFFICIENT_PT implicitly + 1)
    CONFIG_ITEM_DTMF_NB_PT = 55, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_DTMF_WB_PT implicitly + 1)
    CONFIG_ITEM_AMR_DEFAULT_MODE = 56, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_DTMF_NB_PT implicitly + 1)
    CONFIG_ITEM_SMS_PSI = 57, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_AMR_DEFAULT_MODE implicitly + 1)
    CONFIG_ITEM_VIDEO_QUALITY = 58, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SMS_PSI implicitly + 1)
    CONFIG_ITEM_THRESHOLD_LTE1 = 59, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VIDEO_QUALITY implicitly + 1)
    CONFIG_ITEM_THRESHOLD_LTE2 = 60, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_THRESHOLD_LTE1 implicitly + 1)
    CONFIG_ITEM_THRESHOLD_LTE3 = 61, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_THRESHOLD_LTE2 implicitly + 1)
    CONFIG_ITEM_THRESHOLD_1x = 62, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_THRESHOLD_LTE3 implicitly + 1)
    CONFIG_ITEM_THRESHOLD_WIFI_A = 63, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_THRESHOLD_1x implicitly + 1)
    CONFIG_ITEM_THRESHOLD_WIFI_B = 64, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_THRESHOLD_WIFI_A implicitly + 1)
    CONFIG_ITEM_T_EPDG_LTE = 65, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_THRESHOLD_WIFI_B implicitly + 1)
    CONFIG_ITEM_T_EPDG_WIFI = 66, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_T_EPDG_LTE implicitly + 1)
    CONFIG_ITEM_T_EPDG_1x = 67, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_T_EPDG_WIFI implicitly + 1)
    CONFIG_ITEM_VWF_SETTING_ENABLED = 68, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_T_EPDG_1x implicitly + 1)
    CONFIG_ITEM_VCE_SETTING_ENABLED = 69, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VWF_SETTING_ENABLED implicitly + 1)
    CONFIG_ITEM_RTT_SETTING_ENABLED = 70, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VCE_SETTING_ENABLED implicitly + 1)
    CONFIG_ITEM_SMS_APP = 71, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_RTT_SETTING_ENABLED implicitly + 1)
    CONFIG_ITEM_VVM_APP = 72, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SMS_APP implicitly + 1)
    CONFIG_ITEM_INVALID = 73, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VVM_APP implicitly + 1)
};

enum class ConfigFailureCause : int32_t {
    CONFIG_NO_ERR = 0,
    CONFIG_IMS_NOT_READY = 1, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause.CONFIG_NO_ERR implicitly + 1)
    CONFIG_FILE_NOT_AVAILABLE = 2, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause.CONFIG_IMS_NOT_READY implicitly + 1)
    CONFIG_READ_FAILED = 3, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause.CONFIG_FILE_NOT_AVAILABLE implicitly + 1)
    CONFIG_WRITE_FAILED = 4, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause.CONFIG_READ_FAILED implicitly + 1)
    CONFIG_OTHER_INTERNAL_ERR = 5, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause.CONFIG_WRITE_FAILED implicitly + 1)
    CONFIG_FAILURE_INVALID = 6, // (::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause.CONFIG_OTHER_INTERNAL_ERR implicitly + 1)
};

enum class ToneOperation : int32_t {
    STOP = 0,
    START = 1, // (::vendor::qti::hardware::radio::ims::V1_0::ToneOperation.STOP implicitly + 1)
    INVALID = 2, // (::vendor::qti::hardware::radio::ims::V1_0::ToneOperation.START implicitly + 1)
};

enum class HandoverType : int32_t {
    START = 0,
    COMPLETE_SUCCESS = 1, // (::vendor::qti::hardware::radio::ims::V1_0::HandoverType.START implicitly + 1)
    COMPLETE_FAIL = 2, // (::vendor::qti::hardware::radio::ims::V1_0::HandoverType.COMPLETE_SUCCESS implicitly + 1)
    CANCEL = 3, // (::vendor::qti::hardware::radio::ims::V1_0::HandoverType.COMPLETE_FAIL implicitly + 1)
    NOT_TRIGGERED = 4, // (::vendor::qti::hardware::radio::ims::V1_0::HandoverType.CANCEL implicitly + 1)
    NOT_TRIGGERED_MOBILE_DATA_OFF = 5, // (::vendor::qti::hardware::radio::ims::V1_0::HandoverType.NOT_TRIGGERED implicitly + 1)
    INVALID = 6, // (::vendor::qti::hardware::radio::ims::V1_0::HandoverType.NOT_TRIGGERED_MOBILE_DATA_OFF implicitly + 1)
};

enum class CallState : int32_t {
    CALL_ACTIVE = 0,
    CALL_HOLDING = 1, // (::vendor::qti::hardware::radio::ims::V1_0::CallState.CALL_ACTIVE implicitly + 1)
    CALL_DIALING = 2, // (::vendor::qti::hardware::radio::ims::V1_0::CallState.CALL_HOLDING implicitly + 1)
    CALL_ALERTING = 3, // (::vendor::qti::hardware::radio::ims::V1_0::CallState.CALL_DIALING implicitly + 1)
    CALL_INCOMING = 4, // (::vendor::qti::hardware::radio::ims::V1_0::CallState.CALL_ALERTING implicitly + 1)
    CALL_WAITING = 5, // (::vendor::qti::hardware::radio::ims::V1_0::CallState.CALL_INCOMING implicitly + 1)
    CALL_END = 6, // (::vendor::qti::hardware::radio::ims::V1_0::CallState.CALL_WAITING implicitly + 1)
    CALL_STATE_INVALID = 7, // (::vendor::qti::hardware::radio::ims::V1_0::CallState.CALL_END implicitly + 1)
};

enum class ServiceType : int32_t {
    SMS = 0,
    VOIP = 1, // (::vendor::qti::hardware::radio::ims::V1_0::ServiceType.SMS implicitly + 1)
    VT = 2, // (::vendor::qti::hardware::radio::ims::V1_0::ServiceType.VOIP implicitly + 1)
    INVALID = 3, // (::vendor::qti::hardware::radio::ims::V1_0::ServiceType.VT implicitly + 1)
};

enum class ExtraType : int32_t {
    LTE_TO_IWLAN_HO_FAIL = 0,
    INVALID = 1, // (::vendor::qti::hardware::radio::ims::V1_0::ExtraType.LTE_TO_IWLAN_HO_FAIL implicitly + 1)
};

enum class RadioState : int32_t {
    RADIO_STATE_OFF = 0,
    RADIO_STATE_UNAVAILABLE = 1, // (::vendor::qti::hardware::radio::ims::V1_0::RadioState.RADIO_STATE_OFF implicitly + 1)
    RADIO_STATE_ON = 2, // (::vendor::qti::hardware::radio::ims::V1_0::RadioState.RADIO_STATE_UNAVAILABLE implicitly + 1)
    RADIO_STATE_INVALID = 3, // (::vendor::qti::hardware::radio::ims::V1_0::RadioState.RADIO_STATE_ON implicitly + 1)
};

enum class ServiceClassStatus : int32_t {
    DISABLED = 0,
    ENABLED = 1, // (::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus.DISABLED implicitly + 1)
    INVALID = 2, // (::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus.ENABLED implicitly + 1)
};

enum class FacilityType : int32_t {
    FACILITY_CLIP = 0,
    FACILITY_COLP = 1, // (::vendor::qti::hardware::radio::ims::V1_0::FacilityType.FACILITY_CLIP implicitly + 1)
    FACILITY_BAOC = 2, // (::vendor::qti::hardware::radio::ims::V1_0::FacilityType.FACILITY_COLP implicitly + 1)
    FACILITY_BAOIC = 3, // (::vendor::qti::hardware::radio::ims::V1_0::FacilityType.FACILITY_BAOC implicitly + 1)
    FACILITY_BAOICxH = 4, // (::vendor::qti::hardware::radio::ims::V1_0::FacilityType.FACILITY_BAOIC implicitly + 1)
    FACILITY_BAIC = 5, // (::vendor::qti::hardware::radio::ims::V1_0::FacilityType.FACILITY_BAOICxH implicitly + 1)
    FACILITY_BAICr = 6, // (::vendor::qti::hardware::radio::ims::V1_0::FacilityType.FACILITY_BAIC implicitly + 1)
    FACILITY_BA_ALL = 7, // (::vendor::qti::hardware::radio::ims::V1_0::FacilityType.FACILITY_BAICr implicitly + 1)
    FACILITY_BA_MO = 8, // (::vendor::qti::hardware::radio::ims::V1_0::FacilityType.FACILITY_BA_ALL implicitly + 1)
    FACILITY_BA_MT = 9, // (::vendor::qti::hardware::radio::ims::V1_0::FacilityType.FACILITY_BA_MO implicitly + 1)
    FACILITY_BS_MT = 10, // (::vendor::qti::hardware::radio::ims::V1_0::FacilityType.FACILITY_BA_MT implicitly + 1)
    FACILITY_BAICa = 11, // (::vendor::qti::hardware::radio::ims::V1_0::FacilityType.FACILITY_BS_MT implicitly + 1)
    FACILITY_INVALID = 12, // (::vendor::qti::hardware::radio::ims::V1_0::FacilityType.FACILITY_BAICa implicitly + 1)
};

enum class ClipStatus : int32_t {
    NOT_PROVISIONED = 0,
    PROVISIONED = 1, // (::vendor::qti::hardware::radio::ims::V1_0::ClipStatus.NOT_PROVISIONED implicitly + 1)
    STATUS_UNKNOWN = 2, // (::vendor::qti::hardware::radio::ims::V1_0::ClipStatus.PROVISIONED implicitly + 1)
    INVALID = 3, // (::vendor::qti::hardware::radio::ims::V1_0::ClipStatus.STATUS_UNKNOWN implicitly + 1)
};

enum class TtyMode : int32_t {
    TTY_MODE_OFF = 0,
    TTY_MODE_FULL = 1, // (::vendor::qti::hardware::radio::ims::V1_0::TtyMode.TTY_MODE_OFF implicitly + 1)
    TTY_MODE_HCO = 2, // (::vendor::qti::hardware::radio::ims::V1_0::TtyMode.TTY_MODE_FULL implicitly + 1)
    TTY_MODE_VCO = 3, // (::vendor::qti::hardware::radio::ims::V1_0::TtyMode.TTY_MODE_HCO implicitly + 1)
    TTY_MODE_INVALID = 4, // (::vendor::qti::hardware::radio::ims::V1_0::TtyMode.TTY_MODE_VCO implicitly + 1)
};

enum class ConferenceCallState : int32_t {
    RINGING = 0,
    FOREGROUND = 1, // (::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState.RINGING implicitly + 1)
    BACKGROUND = 2, // (::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState.FOREGROUND implicitly + 1)
    INVALID = 3, // (::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState.BACKGROUND implicitly + 1)
};

enum class CallModifyFailCause : int32_t {
    E_SUCCESS = 0,
    E_RADIO_NOT_AVAILABLE = 1, // (::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause.E_SUCCESS implicitly + 1)
    E_GENERIC_FAILURE = 2, // (::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause.E_RADIO_NOT_AVAILABLE implicitly + 1)
    E_REQUEST_NOT_SUPPORTED = 3, // (::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause.E_GENERIC_FAILURE implicitly + 1)
    E_CANCELLED = 4, // (::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause.E_REQUEST_NOT_SUPPORTED implicitly + 1)
    E_UNUSED = 5, // (::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause.E_CANCELLED implicitly + 1)
    E_INVALID_PARAMETER = 6, // (::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause.E_UNUSED implicitly + 1)
    E_REJECTED_BY_REMOTE = 7, // (::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause.E_INVALID_PARAMETER implicitly + 1)
    E_IMS_DEREGISTERED = 8, // (::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause.E_REJECTED_BY_REMOTE implicitly + 1)
    E_NETWORK_NOT_SUPPORTED = 9, // (::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause.E_IMS_DEREGISTERED implicitly + 1)
    E_HOLD_RESUME_FAILED = 10, // (::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause.E_NETWORK_NOT_SUPPORTED implicitly + 1)
    E_HOLD_RESUME_CANCELED = 11, // (::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause.E_HOLD_RESUME_FAILED implicitly + 1)
    E_REINVITE_COLLISION = 12, // (::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause.E_HOLD_RESUME_CANCELED implicitly + 1)
    E_INVALID = 13, // (::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause.E_REINVITE_COLLISION implicitly + 1)
};

enum class NotificationType : int32_t {
    MO = 0,
    MT = 1, // (::vendor::qti::hardware::radio::ims::V1_0::NotificationType.MO implicitly + 1)
    INVALID = 2, // (::vendor::qti::hardware::radio::ims::V1_0::NotificationType.MT implicitly + 1)
};

enum class MessageType : int32_t {
    MWI_MSG_NONE = 0,
    MWI_MSG_VOICE = 1, // (::vendor::qti::hardware::radio::ims::V1_0::MessageType.MWI_MSG_NONE implicitly + 1)
    MWI_MSG_VIDEO = 2, // (::vendor::qti::hardware::radio::ims::V1_0::MessageType.MWI_MSG_VOICE implicitly + 1)
    MWI_MSG_FAX = 3, // (::vendor::qti::hardware::radio::ims::V1_0::MessageType.MWI_MSG_VIDEO implicitly + 1)
    MWI_MSG_PAGER = 4, // (::vendor::qti::hardware::radio::ims::V1_0::MessageType.MWI_MSG_FAX implicitly + 1)
    MWI_MSG_MULTIMEDIA = 5, // (::vendor::qti::hardware::radio::ims::V1_0::MessageType.MWI_MSG_PAGER implicitly + 1)
    MWI_MSG_TEXT = 6, // (::vendor::qti::hardware::radio::ims::V1_0::MessageType.MWI_MSG_MULTIMEDIA implicitly + 1)
    MWI_MSG_INVALID = 7, // (::vendor::qti::hardware::radio::ims::V1_0::MessageType.MWI_MSG_TEXT implicitly + 1)
};

enum class MessagePriority : int32_t {
    MWI_MSG_PRIORITY_UNKNOWN = 0,
    MWI_MSG_PRIORITY_LOW = 1, // (::vendor::qti::hardware::radio::ims::V1_0::MessagePriority.MWI_MSG_PRIORITY_UNKNOWN implicitly + 1)
    MWI_MSG_PRIORITY_NORMAL = 2, // (::vendor::qti::hardware::radio::ims::V1_0::MessagePriority.MWI_MSG_PRIORITY_LOW implicitly + 1)
    MWI_MSG_PRIORITY_URGENT = 3, // (::vendor::qti::hardware::radio::ims::V1_0::MessagePriority.MWI_MSG_PRIORITY_NORMAL implicitly + 1)
    MWI_MSG_PRIORITY_INVALID = 4, // (::vendor::qti::hardware::radio::ims::V1_0::MessagePriority.MWI_MSG_PRIORITY_URGENT implicitly + 1)
};

enum class EctType : int32_t {
    BLIND_TRANSFER = 0,
    ASSURED_TRANSFER = 1, // (::vendor::qti::hardware::radio::ims::V1_0::EctType.BLIND_TRANSFER implicitly + 1)
    CONSULTATIVE_TRANSFER = 2, // (::vendor::qti::hardware::radio::ims::V1_0::EctType.ASSURED_TRANSFER implicitly + 1)
    INVALID = 3, // (::vendor::qti::hardware::radio::ims::V1_0::EctType.CONSULTATIVE_TRANSFER implicitly + 1)
};

enum class ConfParticipantOperation : int32_t {
    ADD = 0,
    REMOVE = 1, // (::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation.ADD implicitly + 1)
    INVALID = 2, // (::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation.REMOVE implicitly + 1)
};

enum class BlockReasonType : int32_t {
    PDP_FAILURE = 0,
    REGISTRATION_FAILURE = 1, // (::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType.PDP_FAILURE implicitly + 1)
    HANDOVER_FAILURE = 2, // (::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType.REGISTRATION_FAILURE implicitly + 1)
    OTHER_FAILURE = 3, // (::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType.HANDOVER_FAILURE implicitly + 1)
    INVALID = 4, // (::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType.OTHER_FAILURE implicitly + 1)
};

enum class RegFailureReasonType : int32_t {
    UNSPECIFIED = 0,
    MOBILE_IP = 1, // (::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType.UNSPECIFIED implicitly + 1)
    INTERNAL = 2, // (::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType.MOBILE_IP implicitly + 1)
    CALL_MANAGER_DEFINED = 3, // (::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType.INTERNAL implicitly + 1)
    TYPE_3GPP_SPEC_DEFINED = 4, // (::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType.CALL_MANAGER_DEFINED implicitly + 1)
    PPP = 5, // (::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType.TYPE_3GPP_SPEC_DEFINED implicitly + 1)
    EHRPD = 6, // (::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType.PPP implicitly + 1)
    IPV6 = 7, // (::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType.EHRPD implicitly + 1)
    IWLAN = 8, // (::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType.IPV6 implicitly + 1)
    HANDOFF = 9, // (::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType.IWLAN implicitly + 1)
    INVALID = 10, // (::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType.HANDOFF implicitly + 1)
};

enum class RttMode : int32_t {
    RTT_MODE_DISABLED = 0,
    RTT_MODE_FULL = 1, // (::vendor::qti::hardware::radio::ims::V1_0::RttMode.RTT_MODE_DISABLED implicitly + 1)
    RTT_MODE_INVALID = 2, // (::vendor::qti::hardware::radio::ims::V1_0::RttMode.RTT_MODE_FULL implicitly + 1)
};

struct RegistrationInfo final {
    ::vendor::qti::hardware::radio::ims::V1_0::RegState state __attribute__ ((aligned(4)));
    uint32_t errorCode __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string errorMessage __attribute__ ((aligned(8)));
    ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType radioTech __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string pAssociatedUris __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::RegistrationInfo, state) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::RegistrationInfo, errorCode) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::RegistrationInfo, errorMessage) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::RegistrationInfo, radioTech) == 24, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::RegistrationInfo, pAssociatedUris) == 32, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::RegistrationInfo) == 48, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::RegistrationInfo) == 8, "wrong alignment");

struct StatusForAccessTech final {
    ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType networkMode __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_0::StatusType status __attribute__ ((aligned(4)));
    uint32_t restrictCause __attribute__ ((aligned(4)));
    bool hasRegistration __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_0::RegistrationInfo registration __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::StatusForAccessTech, networkMode) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::StatusForAccessTech, status) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::StatusForAccessTech, restrictCause) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::StatusForAccessTech, hasRegistration) == 12, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::StatusForAccessTech, registration) == 16, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::StatusForAccessTech) == 64, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::StatusForAccessTech) == 8, "wrong alignment");

struct ServiceStatusInfo final {
    bool hasIsValid __attribute__ ((aligned(1)));
    bool isValid __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_0::ServiceType type __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_0::CallType callType __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_0::StatusType status __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<uint8_t> userdata __attribute__ ((aligned(8)));
    uint32_t restrictCause __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_0::StatusForAccessTech> accTechStatus __attribute__ ((aligned(8)));
    ::vendor::qti::hardware::radio::ims::V1_0::RttMode rttMode __attribute__ ((aligned(4)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ServiceStatusInfo, hasIsValid) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ServiceStatusInfo, isValid) == 1, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ServiceStatusInfo, type) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ServiceStatusInfo, callType) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ServiceStatusInfo, status) == 12, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ServiceStatusInfo, userdata) == 16, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ServiceStatusInfo, restrictCause) == 32, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ServiceStatusInfo, accTechStatus) == 40, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ServiceStatusInfo, rttMode) == 56, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::ServiceStatusInfo) == 64, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::ServiceStatusInfo) == 8, "wrong alignment");

struct CallDetails final {
    ::vendor::qti::hardware::radio::ims::V1_0::CallType callType __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_0::CallDomain callDomain __attribute__ ((aligned(4)));
    uint32_t extrasLength __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<::android::hardware::hidl_string> extras __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_0::ServiceStatusInfo> localAbility __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_0::ServiceStatusInfo> peerAbility __attribute__ ((aligned(8)));
    uint32_t callSubstate __attribute__ ((aligned(4)));
    uint32_t mediaId __attribute__ ((aligned(4)));
    uint32_t causeCode __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_0::RttMode rttMode __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string sipAlternateUri __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallDetails, callType) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallDetails, callDomain) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallDetails, extrasLength) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallDetails, extras) == 16, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallDetails, localAbility) == 32, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallDetails, peerAbility) == 48, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallDetails, callSubstate) == 64, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallDetails, mediaId) == 68, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallDetails, causeCode) == 72, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallDetails, rttMode) == 76, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallDetails, sipAlternateUri) == 80, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::CallDetails) == 96, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::CallDetails) == 8, "wrong alignment");

struct DialRequest final {
    ::android::hardware::hidl_string address __attribute__ ((aligned(8)));
    uint32_t clirMode __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation presentation __attribute__ ((aligned(4)));
    bool hasCallDetails __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_0::CallDetails callDetails __attribute__ ((aligned(8)));
    bool hasIsConferenceUri __attribute__ ((aligned(1)));
    bool isConferenceUri __attribute__ ((aligned(1)));
    bool hasIsCallPull __attribute__ ((aligned(1)));
    bool isCallPull __attribute__ ((aligned(1)));
    bool hasIsEncrypted __attribute__ ((aligned(1)));
    bool isEncrypted __attribute__ ((aligned(1)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::DialRequest, address) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::DialRequest, clirMode) == 16, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::DialRequest, presentation) == 20, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::DialRequest, hasCallDetails) == 24, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::DialRequest, callDetails) == 32, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::DialRequest, hasIsConferenceUri) == 128, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::DialRequest, isConferenceUri) == 129, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::DialRequest, hasIsCallPull) == 130, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::DialRequest, isCallPull) == 131, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::DialRequest, hasIsEncrypted) == 132, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::DialRequest, isEncrypted) == 133, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::DialRequest) == 136, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::DialRequest) == 8, "wrong alignment");

struct SipErrorInfo final {
    uint32_t errorCode __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string errorString __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo, errorCode) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo, errorString) == 8, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo) == 24, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo) == 8, "wrong alignment");

struct CallFailCauseResponse final {
    ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause failCause __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<uint8_t> errorinfo __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string networkErrorString __attribute__ ((aligned(8)));
    bool hasErrorDetails __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo errorDetails __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallFailCauseResponse, failCause) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallFailCauseResponse, errorinfo) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallFailCauseResponse, networkErrorString) == 24, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallFailCauseResponse, hasErrorDetails) == 40, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallFailCauseResponse, errorDetails) == 48, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::CallFailCauseResponse) == 72, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::CallFailCauseResponse) == 8, "wrong alignment");

struct HangupRequestInfo final {
    uint32_t connIndex __attribute__ ((aligned(4)));
    bool hasMultiParty __attribute__ ((aligned(1)));
    bool multiParty __attribute__ ((aligned(1)));
    ::android::hardware::hidl_string connUri __attribute__ ((aligned(8)));
    uint32_t conf_id __attribute__ ((aligned(4)));
    bool hasFailCauseResponse __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_0::CallFailCauseResponse failCauseResponse __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::HangupRequestInfo, connIndex) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::HangupRequestInfo, hasMultiParty) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::HangupRequestInfo, multiParty) == 5, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::HangupRequestInfo, connUri) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::HangupRequestInfo, conf_id) == 24, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::HangupRequestInfo, hasFailCauseResponse) == 28, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::HangupRequestInfo, failCauseResponse) == 32, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::HangupRequestInfo) == 104, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::HangupRequestInfo) == 8, "wrong alignment");

struct ConfigInfo final {
    ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem item __attribute__ ((aligned(4)));
    bool hasBoolValue __attribute__ ((aligned(1)));
    bool boolValue __attribute__ ((aligned(1)));
    uint32_t intValue __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string stringValue __attribute__ ((aligned(8)));
    ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause errorCause __attribute__ ((aligned(4)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ConfigInfo, item) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ConfigInfo, hasBoolValue) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ConfigInfo, boolValue) == 5, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ConfigInfo, intValue) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ConfigInfo, stringValue) == 16, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ConfigInfo, errorCause) == 32, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::ConfigInfo) == 40, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::ConfigInfo) == 8, "wrong alignment");

struct Extra final {
    ::vendor::qti::hardware::radio::ims::V1_0::ExtraType type __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<uint8_t> extraInfo __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::Extra, type) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::Extra, extraInfo) == 8, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::Extra) == 24, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::Extra) == 8, "wrong alignment");

struct HandoverInfo final {
    ::vendor::qti::hardware::radio::ims::V1_0::HandoverType type __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType srcTech __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType targetTech __attribute__ ((aligned(4)));
    bool hasHoExtra __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_0::Extra hoExtra __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string errorCode __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string errorMessage __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::HandoverInfo, type) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::HandoverInfo, srcTech) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::HandoverInfo, targetTech) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::HandoverInfo, hasHoExtra) == 12, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::HandoverInfo, hoExtra) == 16, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::HandoverInfo, errorCode) == 40, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::HandoverInfo, errorMessage) == 56, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::HandoverInfo) == 72, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::HandoverInfo) == 8, "wrong alignment");

struct CallInfo final {
    ::vendor::qti::hardware::radio::ims::V1_0::CallState state __attribute__ ((aligned(4)));
    uint32_t index __attribute__ ((aligned(4)));
    uint32_t toa __attribute__ ((aligned(4)));
    bool hasIsMpty __attribute__ ((aligned(1)));
    bool isMpty __attribute__ ((aligned(1)));
    bool hasIsMT __attribute__ ((aligned(1)));
    bool isMT __attribute__ ((aligned(1)));
    uint32_t als __attribute__ ((aligned(4)));
    bool hasIsVoice __attribute__ ((aligned(1)));
    bool isVoice __attribute__ ((aligned(1)));
    bool hasIsVoicePrivacy __attribute__ ((aligned(1)));
    bool isVoicePrivacy __attribute__ ((aligned(1)));
    ::android::hardware::hidl_string number __attribute__ ((aligned(8)));
    uint32_t numberPresentation __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string name __attribute__ ((aligned(8)));
    uint32_t namePresentation __attribute__ ((aligned(4)));
    bool hasCallDetails __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_0::CallDetails callDetails __attribute__ ((aligned(8)));
    bool hasFailCause __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_0::CallFailCauseResponse failCause __attribute__ ((aligned(8)));
    bool hasIsEncrypted __attribute__ ((aligned(1)));
    bool isEncrypted __attribute__ ((aligned(1)));
    bool hasIsCalledPartyRinging __attribute__ ((aligned(1)));
    bool isCalledPartyRinging __attribute__ ((aligned(1)));
    ::android::hardware::hidl_string historyInfo __attribute__ ((aligned(8)));
    bool hasIsVideoConfSupported __attribute__ ((aligned(1)));
    bool isVideoConfSupported __attribute__ ((aligned(1)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallInfo, state) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallInfo, index) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallInfo, toa) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallInfo, hasIsMpty) == 12, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallInfo, isMpty) == 13, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallInfo, hasIsMT) == 14, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallInfo, isMT) == 15, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallInfo, als) == 16, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallInfo, hasIsVoice) == 20, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallInfo, isVoice) == 21, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallInfo, hasIsVoicePrivacy) == 22, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallInfo, isVoicePrivacy) == 23, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallInfo, number) == 24, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallInfo, numberPresentation) == 40, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallInfo, name) == 48, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallInfo, namePresentation) == 64, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallInfo, hasCallDetails) == 68, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallInfo, callDetails) == 72, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallInfo, hasFailCause) == 168, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallInfo, failCause) == 176, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallInfo, hasIsEncrypted) == 248, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallInfo, isEncrypted) == 249, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallInfo, hasIsCalledPartyRinging) == 250, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallInfo, isCalledPartyRinging) == 251, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallInfo, historyInfo) == 256, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallInfo, hasIsVideoConfSupported) == 272, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallInfo, isVideoConfSupported) == 273, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::CallInfo) == 280, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::CallInfo) == 8, "wrong alignment");

struct CbNumInfo final {
    ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus status __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string number __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CbNumInfo, status) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CbNumInfo, number) == 8, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::CbNumInfo) == 24, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::CbNumInfo) == 8, "wrong alignment");

struct CbNumListInfo final {
    uint32_t serviceClass __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_0::CbNumInfo> cbNumInfo __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CbNumListInfo, serviceClass) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CbNumListInfo, cbNumInfo) == 8, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::CbNumListInfo) == 24, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::CbNumListInfo) == 8, "wrong alignment");

struct SuppServiceStatus final {
    ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus status __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_0::FacilityType facilityType __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string failureCause __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_0::CbNumListInfo> cbNumListInfo __attribute__ ((aligned(8)));
    bool hasErrorDetails __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo errorDetails __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::SuppServiceStatus, status) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::SuppServiceStatus, facilityType) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::SuppServiceStatus, failureCause) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::SuppServiceStatus, cbNumListInfo) == 24, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::SuppServiceStatus, hasErrorDetails) == 40, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::SuppServiceStatus, errorDetails) == 48, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::SuppServiceStatus) == 72, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::SuppServiceStatus) == 8, "wrong alignment");

struct ClipProvisionStatus final {
    ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus clipStatus __attribute__ ((aligned(4)));
    bool hasErrorDetails __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo errorDetails __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ClipProvisionStatus, clipStatus) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ClipProvisionStatus, hasErrorDetails) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ClipProvisionStatus, errorDetails) == 8, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::ClipProvisionStatus) == 32, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::ClipProvisionStatus) == 8, "wrong alignment");

struct ClirInfo final {
    uint32_t paramM __attribute__ ((aligned(4)));
    uint32_t paramN __attribute__ ((aligned(4)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ClirInfo, paramM) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ClirInfo, paramN) == 4, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::ClirInfo) == 8, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::ClirInfo) == 4, "wrong alignment");

struct ColrInfo final {
    ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation presentation __attribute__ ((aligned(4)));
    bool hasErrorDetails __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo errorDetails __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ColrInfo, presentation) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ColrInfo, hasErrorDetails) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ColrInfo, errorDetails) == 8, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::ColrInfo) == 32, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::ColrInfo) == 8, "wrong alignment");

struct DtmfInfo final {
    ::android::hardware::hidl_string dtmf __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::DtmfInfo, dtmf) == 0, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::DtmfInfo) == 16, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::DtmfInfo) == 8, "wrong alignment");

struct TtyInfo final {
    ::vendor::qti::hardware::radio::ims::V1_0::TtyMode mode __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<uint8_t> userData __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::TtyInfo, mode) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::TtyInfo, userData) == 8, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::TtyInfo) == 24, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::TtyInfo) == 8, "wrong alignment");

struct ConferenceInfo final {
    ::android::hardware::hidl_vec<uint8_t> confInfoUri __attribute__ ((aligned(8)));
    ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState conferenceCallState __attribute__ ((aligned(4)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ConferenceInfo, confInfoUri) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ConferenceInfo, conferenceCallState) == 16, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::ConferenceInfo) == 24, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::ConferenceInfo) == 8, "wrong alignment");

struct ViceInfo final {
    ::android::hardware::hidl_vec<uint8_t> viceInfoUri __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ViceInfo, viceInfoUri) == 0, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::ViceInfo) == 16, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::ViceInfo) == 8, "wrong alignment");

struct CallModifyInfo final {
    uint32_t callIndex __attribute__ ((aligned(4)));
    bool hasCallDetails __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_0::CallDetails callDetails __attribute__ ((aligned(8)));
    ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause failCause __attribute__ ((aligned(4)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallModifyInfo, callIndex) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallModifyInfo, hasCallDetails) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallModifyInfo, callDetails) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallModifyInfo, failCause) == 104, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::CallModifyInfo) == 112, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::CallModifyInfo) == 8, "wrong alignment");

struct CallFwdTimerInfo final {
    uint32_t year __attribute__ ((aligned(4)));
    uint32_t month __attribute__ ((aligned(4)));
    uint32_t day __attribute__ ((aligned(4)));
    uint32_t hour __attribute__ ((aligned(4)));
    uint32_t minute __attribute__ ((aligned(4)));
    uint32_t second __attribute__ ((aligned(4)));
    uint32_t timezone __attribute__ ((aligned(4)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallFwdTimerInfo, year) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallFwdTimerInfo, month) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallFwdTimerInfo, day) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallFwdTimerInfo, hour) == 12, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallFwdTimerInfo, minute) == 16, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallFwdTimerInfo, second) == 20, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallFwdTimerInfo, timezone) == 24, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::CallFwdTimerInfo) == 28, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::CallFwdTimerInfo) == 4, "wrong alignment");

struct CallForwardInfo final {
    uint32_t status __attribute__ ((aligned(4)));
    uint32_t reason __attribute__ ((aligned(4)));
    uint32_t serviceClass __attribute__ ((aligned(4)));
    uint32_t toa __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string number __attribute__ ((aligned(8)));
    uint32_t timeSeconds __attribute__ ((aligned(4)));
    bool hasCallFwdTimerStart __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_0::CallFwdTimerInfo callFwdTimerStart __attribute__ ((aligned(4)));
    bool hasCallFwdTimerEnd __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_0::CallFwdTimerInfo callFwdTimerEnd __attribute__ ((aligned(4)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo, status) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo, reason) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo, serviceClass) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo, toa) == 12, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo, number) == 16, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo, timeSeconds) == 32, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo, hasCallFwdTimerStart) == 36, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo, callFwdTimerStart) == 40, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo, hasCallFwdTimerEnd) == 68, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo, callFwdTimerEnd) == 72, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo) == 104, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo) == 8, "wrong alignment");

struct SuppServiceNotification final {
    ::vendor::qti::hardware::radio::ims::V1_0::NotificationType notificationType __attribute__ ((aligned(4)));
    uint32_t code __attribute__ ((aligned(4)));
    uint32_t index __attribute__ ((aligned(4)));
    uint32_t type __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string number __attribute__ ((aligned(8)));
    uint32_t connId __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string historyInfo __attribute__ ((aligned(8)));
    bool hasHoldTone __attribute__ ((aligned(1)));
    bool holdTone __attribute__ ((aligned(1)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::SuppServiceNotification, notificationType) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::SuppServiceNotification, code) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::SuppServiceNotification, index) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::SuppServiceNotification, type) == 12, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::SuppServiceNotification, number) == 16, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::SuppServiceNotification, connId) == 32, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::SuppServiceNotification, historyInfo) == 40, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::SuppServiceNotification, hasHoldTone) == 56, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::SuppServiceNotification, holdTone) == 57, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::SuppServiceNotification) == 64, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::SuppServiceNotification) == 8, "wrong alignment");

struct ExplicitCallTransferInfo final {
    uint32_t callId __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_0::EctType ectType __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string targetAddress __attribute__ ((aligned(8)));
    uint32_t targetCallId __attribute__ ((aligned(4)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ExplicitCallTransferInfo, callId) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ExplicitCallTransferInfo, ectType) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ExplicitCallTransferInfo, targetAddress) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ExplicitCallTransferInfo, targetCallId) == 24, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::ExplicitCallTransferInfo) == 32, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::ExplicitCallTransferInfo) == 8, "wrong alignment");

struct MessageSummary final {
    ::vendor::qti::hardware::radio::ims::V1_0::MessageType type __attribute__ ((aligned(4)));
    uint32_t newMessageCount __attribute__ ((aligned(4)));
    uint32_t oldMessageCount __attribute__ ((aligned(4)));
    uint32_t newUrgentMessageCount __attribute__ ((aligned(4)));
    uint32_t oldUrgentMessageCount __attribute__ ((aligned(4)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::MessageSummary, type) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::MessageSummary, newMessageCount) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::MessageSummary, oldMessageCount) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::MessageSummary, newUrgentMessageCount) == 12, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::MessageSummary, oldUrgentMessageCount) == 16, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::MessageSummary) == 20, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::MessageSummary) == 4, "wrong alignment");

struct MessageDetails final {
    ::android::hardware::hidl_string toAddress __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string fromAddress __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string subject __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string date __attribute__ ((aligned(8)));
    ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority priority __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string id __attribute__ ((aligned(8)));
    ::vendor::qti::hardware::radio::ims::V1_0::MessageType type __attribute__ ((aligned(4)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::MessageDetails, toAddress) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::MessageDetails, fromAddress) == 16, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::MessageDetails, subject) == 32, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::MessageDetails, date) == 48, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::MessageDetails, priority) == 64, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::MessageDetails, id) == 72, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::MessageDetails, type) == 88, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::MessageDetails) == 96, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::MessageDetails) == 8, "wrong alignment");

struct MessageWaitingIndication final {
    ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_0::MessageSummary> messageSummary __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string ueAddress __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_0::MessageDetails> messageDetails __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::MessageWaitingIndication, messageSummary) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::MessageWaitingIndication, ueAddress) == 16, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::MessageWaitingIndication, messageDetails) == 32, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::MessageWaitingIndication) == 48, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::MessageWaitingIndication) == 8, "wrong alignment");

struct DeflectRequestInfo final {
    uint32_t connIndex __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string number __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::DeflectRequestInfo, connIndex) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::DeflectRequestInfo, number) == 8, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::DeflectRequestInfo) == 24, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::DeflectRequestInfo) == 8, "wrong alignment");

struct AddressInfo final {
    ::android::hardware::hidl_string city __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string state __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string country __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string postalCode __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string countryCode __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::AddressInfo, city) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::AddressInfo, state) == 16, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::AddressInfo, country) == 32, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::AddressInfo, postalCode) == 48, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::AddressInfo, countryCode) == 64, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::AddressInfo) == 80, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::AddressInfo) == 8, "wrong alignment");

struct ImsSubConfigInfo final {
    uint32_t simultStackCount __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<bool> imsStackEnabled __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ImsSubConfigInfo, simultStackCount) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ImsSubConfigInfo, imsStackEnabled) == 8, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::ImsSubConfigInfo) == 24, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::ImsSubConfigInfo) == 8, "wrong alignment");

struct ParticipantStatusInfo final {
    uint32_t callId __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation operation __attribute__ ((aligned(4)));
    uint32_t sipStatus __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string participantUri __attribute__ ((aligned(8)));
    bool hasIsEct __attribute__ ((aligned(1)));
    bool isEct __attribute__ ((aligned(1)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ParticipantStatusInfo, callId) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ParticipantStatusInfo, operation) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ParticipantStatusInfo, sipStatus) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ParticipantStatusInfo, participantUri) == 16, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ParticipantStatusInfo, hasIsEct) == 32, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::ParticipantStatusInfo, isEct) == 33, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::ParticipantStatusInfo) == 40, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::ParticipantStatusInfo) == 8, "wrong alignment");

struct BlockReasonDetails final {
    ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType regFailureReasonType __attribute__ ((aligned(4)));
    uint32_t regFailureReason __attribute__ ((aligned(4)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::BlockReasonDetails, regFailureReasonType) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::BlockReasonDetails, regFailureReason) == 4, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::BlockReasonDetails) == 8, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::BlockReasonDetails) == 4, "wrong alignment");

struct BlockStatus final {
    ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType blockReason __attribute__ ((aligned(4)));
    bool hasBlockReasonDetails __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonDetails blockReasonDetails __attribute__ ((aligned(4)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::BlockStatus, blockReason) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::BlockStatus, hasBlockReasonDetails) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::BlockStatus, blockReasonDetails) == 8, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::BlockStatus) == 16, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::BlockStatus) == 4, "wrong alignment");

enum class VoWiFiCallQualityInfo : int32_t {
    VOWIFI_CALL_QUALITY_NONE = 0,
    VOWIFI_CALL_QUALITY_EXCELLENT = 1, // (::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo.VOWIFI_CALL_QUALITY_NONE implicitly + 1)
    VOWIFI_CALL_QUALITY_FAIR = 2, // (::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo.VOWIFI_CALL_QUALITY_EXCELLENT implicitly + 1)
    VOWIFI_CALL_QUALITY_BAD = 3, // (::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo.VOWIFI_CALL_QUALITY_FAIR implicitly + 1)
    VOWIFI_CALL_QUALITY_INVALID = 4, // (::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo.VOWIFI_CALL_QUALITY_BAD implicitly + 1)
};

enum class SsServiceType : int32_t {
    SS_CFU = 0,
    SS_CF_BUSY = 1, // (::vendor::qti::hardware::radio::ims::V1_0::SsServiceType.SS_CFU implicitly + 1)
    SS_CF_NO_REPLY = 2, // (::vendor::qti::hardware::radio::ims::V1_0::SsServiceType.SS_CF_BUSY implicitly + 1)
    SS_CF_NOT_REACHABLE = 3, // (::vendor::qti::hardware::radio::ims::V1_0::SsServiceType.SS_CF_NO_REPLY implicitly + 1)
    SS_CF_ALL = 4, // (::vendor::qti::hardware::radio::ims::V1_0::SsServiceType.SS_CF_NOT_REACHABLE implicitly + 1)
    SS_CF_ALL_CONDITIONAL = 5, // (::vendor::qti::hardware::radio::ims::V1_0::SsServiceType.SS_CF_ALL implicitly + 1)
    SS_CFUT = 6, // (::vendor::qti::hardware::radio::ims::V1_0::SsServiceType.SS_CF_ALL_CONDITIONAL implicitly + 1)
    SS_CLIP = 7, // (::vendor::qti::hardware::radio::ims::V1_0::SsServiceType.SS_CFUT implicitly + 1)
    SS_CLIR = 8, // (::vendor::qti::hardware::radio::ims::V1_0::SsServiceType.SS_CLIP implicitly + 1)
    SS_COLP = 9, // (::vendor::qti::hardware::radio::ims::V1_0::SsServiceType.SS_CLIR implicitly + 1)
    SS_COLR = 10, // (::vendor::qti::hardware::radio::ims::V1_0::SsServiceType.SS_COLP implicitly + 1)
    SS_CNAP = 11, // (::vendor::qti::hardware::radio::ims::V1_0::SsServiceType.SS_COLR implicitly + 1)
    SS_WAIT = 12, // (::vendor::qti::hardware::radio::ims::V1_0::SsServiceType.SS_CNAP implicitly + 1)
    SS_BAOC = 13, // (::vendor::qti::hardware::radio::ims::V1_0::SsServiceType.SS_WAIT implicitly + 1)
    SS_BAOIC = 14, // (::vendor::qti::hardware::radio::ims::V1_0::SsServiceType.SS_BAOC implicitly + 1)
    SS_BAOIC_EXC_HOME = 15, // (::vendor::qti::hardware::radio::ims::V1_0::SsServiceType.SS_BAOIC implicitly + 1)
    SS_BAIC = 16, // (::vendor::qti::hardware::radio::ims::V1_0::SsServiceType.SS_BAOIC_EXC_HOME implicitly + 1)
    SS_BAIC_ROAMING = 17, // (::vendor::qti::hardware::radio::ims::V1_0::SsServiceType.SS_BAIC implicitly + 1)
    SS_ALL_BARRING = 18, // (::vendor::qti::hardware::radio::ims::V1_0::SsServiceType.SS_BAIC_ROAMING implicitly + 1)
    SS_OUTGOING_BARRING = 19, // (::vendor::qti::hardware::radio::ims::V1_0::SsServiceType.SS_ALL_BARRING implicitly + 1)
    SS_INCOMING_BARRING = 20, // (::vendor::qti::hardware::radio::ims::V1_0::SsServiceType.SS_OUTGOING_BARRING implicitly + 1)
    SS_INCOMING_BARRING_DN = 21, // (::vendor::qti::hardware::radio::ims::V1_0::SsServiceType.SS_INCOMING_BARRING implicitly + 1)
    SS_INCOMING_BARRING_ANONYMOUS = 22, // (::vendor::qti::hardware::radio::ims::V1_0::SsServiceType.SS_INCOMING_BARRING_DN implicitly + 1)
    SS_INVALID = 23, // (::vendor::qti::hardware::radio::ims::V1_0::SsServiceType.SS_INCOMING_BARRING_ANONYMOUS implicitly + 1)
};

enum class SsRequestType : int32_t {
    SS_ACTIVATION = 0,
    SS_DEACTIVATION = 1, // (::vendor::qti::hardware::radio::ims::V1_0::SsRequestType.SS_ACTIVATION implicitly + 1)
    SS_INTERROGATION = 2, // (::vendor::qti::hardware::radio::ims::V1_0::SsRequestType.SS_DEACTIVATION implicitly + 1)
    SS_REGISTRATION = 3, // (::vendor::qti::hardware::radio::ims::V1_0::SsRequestType.SS_INTERROGATION implicitly + 1)
    SS_ERASURE = 4, // (::vendor::qti::hardware::radio::ims::V1_0::SsRequestType.SS_REGISTRATION implicitly + 1)
    SS_INVALID = 5, // (::vendor::qti::hardware::radio::ims::V1_0::SsRequestType.SS_ERASURE implicitly + 1)
};

enum class SsTeleserviceType : int32_t {
    SS_ALL_TELE_AND_BEARER_SERVICES = 0,
    SS_ALL_TELESEVICES = 1, // (::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType.SS_ALL_TELE_AND_BEARER_SERVICES implicitly + 1)
    SS_TELEPHONY = 2, // (::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType.SS_ALL_TELESEVICES implicitly + 1)
    SS_ALL_DATA_TELESERVICES = 3, // (::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType.SS_TELEPHONY implicitly + 1)
    SS_SMS_SERVICES = 4, // (::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType.SS_ALL_DATA_TELESERVICES implicitly + 1)
    SS_ALL_TELESERVICES_EXCEPT_SMS = 5, // (::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType.SS_SMS_SERVICES implicitly + 1)
    SS_INVALID = 6, // (::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType.SS_ALL_TELESERVICES_EXCEPT_SMS implicitly + 1)
};

struct CfData final {
    ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo> cfInfo __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::CfData, cfInfo) == 0, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::CfData) == 16, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::CfData) == 8, "wrong alignment");

struct SsInfoData final {
    ::android::hardware::hidl_vec<uint32_t> ssInfo __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::SsInfoData, ssInfo) == 0, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::SsInfoData) == 16, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::SsInfoData) == 8, "wrong alignment");

struct StkCcUnsolSsResult final {
    ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType serviceType __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType requestType __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType teleserviceType __attribute__ ((aligned(4)));
    uint32_t serviceClass __attribute__ ((aligned(4)));
    uint32_t result __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_0::SsInfoData> ssInfoData __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_0::CfData> cfData __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_0::CbNumInfo> cbNumInfo __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::StkCcUnsolSsResult, serviceType) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::StkCcUnsolSsResult, requestType) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::StkCcUnsolSsResult, teleserviceType) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::StkCcUnsolSsResult, serviceClass) == 12, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::StkCcUnsolSsResult, result) == 16, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::StkCcUnsolSsResult, ssInfoData) == 24, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::StkCcUnsolSsResult, cfData) == 40, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_0::StkCcUnsolSsResult, cbNumInfo) == 56, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_0::StkCcUnsolSsResult) == 72, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_0::StkCcUnsolSsResult) == 8, "wrong alignment");

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation lhs, const ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation lhs, const ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::IpPresentation>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::IpPresentation> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation::IP_PRESENTATION_NUM_ALLOWED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::IpPresentation::IP_PRESENTATION_NUM_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "IP_PRESENTATION_NUM_ALLOWED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation::IP_PRESENTATION_NUM_ALLOWED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation::IP_PRESENTATION_NUM_RESTRICTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::IpPresentation::IP_PRESENTATION_NUM_RESTRICTED)) {
        os += (first ? "" : " | ");
        os += "IP_PRESENTATION_NUM_RESTRICTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation::IP_PRESENTATION_NUM_RESTRICTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation::IP_PRESENTATION_NUM_DEFAULT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::IpPresentation::IP_PRESENTATION_NUM_DEFAULT)) {
        os += (first ? "" : " | ");
        os += "IP_PRESENTATION_NUM_DEFAULT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation::IP_PRESENTATION_NUM_DEFAULT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation::IP_PRESENTATION_INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::IpPresentation::IP_PRESENTATION_INVALID)) {
        os += (first ? "" : " | ");
        os += "IP_PRESENTATION_INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation::IP_PRESENTATION_INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::IpPresentation o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation::IP_PRESENTATION_NUM_ALLOWED) {
        return "IP_PRESENTATION_NUM_ALLOWED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation::IP_PRESENTATION_NUM_RESTRICTED) {
        return "IP_PRESENTATION_NUM_RESTRICTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation::IP_PRESENTATION_NUM_DEFAULT) {
        return "IP_PRESENTATION_NUM_DEFAULT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation::IP_PRESENTATION_INVALID) {
        return "IP_PRESENTATION_INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::CallType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::CallType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::CallType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::CallType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::CallType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::CallType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::CallType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::CallType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::CallType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::CallType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::CallType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::CallType> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_VOICE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_VOICE)) {
        os += (first ? "" : " | ");
        os += "CALL_TYPE_VOICE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_VOICE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_VT_TX) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_VT_TX)) {
        os += (first ? "" : " | ");
        os += "CALL_TYPE_VT_TX";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_VT_TX;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_VT_RX) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_VT_RX)) {
        os += (first ? "" : " | ");
        os += "CALL_TYPE_VT_RX";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_VT_RX;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_VT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_VT)) {
        os += (first ? "" : " | ");
        os += "CALL_TYPE_VT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_VT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_VT_NODIR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_VT_NODIR)) {
        os += (first ? "" : " | ");
        os += "CALL_TYPE_VT_NODIR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_VT_NODIR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_CS_VS_TX) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_CS_VS_TX)) {
        os += (first ? "" : " | ");
        os += "CALL_TYPE_CS_VS_TX";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_CS_VS_TX;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_CS_VS_RX) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_CS_VS_RX)) {
        os += (first ? "" : " | ");
        os += "CALL_TYPE_CS_VS_RX";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_CS_VS_RX;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_PS_VS_TX) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_PS_VS_TX)) {
        os += (first ? "" : " | ");
        os += "CALL_TYPE_PS_VS_TX";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_PS_VS_TX;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_PS_VS_RX) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_PS_VS_RX)) {
        os += (first ? "" : " | ");
        os += "CALL_TYPE_PS_VS_RX";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_PS_VS_RX;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_UNKNOWN) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "CALL_TYPE_UNKNOWN";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_UNKNOWN;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_SMS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_SMS)) {
        os += (first ? "" : " | ");
        os += "CALL_TYPE_SMS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_SMS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_UT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_UT)) {
        os += (first ? "" : " | ");
        os += "CALL_TYPE_UT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_UT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_INVALID)) {
        os += (first ? "" : " | ");
        os += "CALL_TYPE_INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::CallType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_VOICE) {
        return "CALL_TYPE_VOICE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_VT_TX) {
        return "CALL_TYPE_VT_TX";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_VT_RX) {
        return "CALL_TYPE_VT_RX";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_VT) {
        return "CALL_TYPE_VT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_VT_NODIR) {
        return "CALL_TYPE_VT_NODIR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_CS_VS_TX) {
        return "CALL_TYPE_CS_VS_TX";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_CS_VS_RX) {
        return "CALL_TYPE_CS_VS_RX";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_PS_VS_TX) {
        return "CALL_TYPE_PS_VS_TX";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_PS_VS_RX) {
        return "CALL_TYPE_PS_VS_RX";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_UNKNOWN) {
        return "CALL_TYPE_UNKNOWN";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_SMS) {
        return "CALL_TYPE_SMS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_UT) {
        return "CALL_TYPE_UT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_INVALID) {
        return "CALL_TYPE_INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::CallDomain lhs, const ::vendor::qti::hardware::radio::ims::V1_0::CallDomain rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::CallDomain rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::CallDomain lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::CallDomain lhs, const ::vendor::qti::hardware::radio::ims::V1_0::CallDomain rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::CallDomain rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::CallDomain lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::CallDomain e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::CallDomain e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::CallDomain>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::CallDomain> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_UNKNOWN) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "CALL_DOMAIN_UNKNOWN";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_UNKNOWN;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_CS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_CS)) {
        os += (first ? "" : " | ");
        os += "CALL_DOMAIN_CS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_CS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_PS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_PS)) {
        os += (first ? "" : " | ");
        os += "CALL_DOMAIN_PS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_PS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_AUTOMATIC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_AUTOMATIC)) {
        os += (first ? "" : " | ");
        os += "CALL_DOMAIN_AUTOMATIC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_AUTOMATIC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_NOT_SET) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_NOT_SET)) {
        os += (first ? "" : " | ");
        os += "CALL_DOMAIN_NOT_SET";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_NOT_SET;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_INVALID)) {
        os += (first ? "" : " | ");
        os += "CALL_DOMAIN_INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::CallDomain o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_UNKNOWN) {
        return "CALL_DOMAIN_UNKNOWN";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_CS) {
        return "CALL_DOMAIN_CS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_PS) {
        return "CALL_DOMAIN_PS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_AUTOMATIC) {
        return "CALL_DOMAIN_AUTOMATIC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_NOT_SET) {
        return "CALL_DOMAIN_NOT_SET";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_INVALID) {
        return "CALL_DOMAIN_INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause lhs, const ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause lhs, const ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::CallFailCause>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::CallFailCause> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_UNOBTAINABLE_NUMBER) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_UNOBTAINABLE_NUMBER)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_UNOBTAINABLE_NUMBER";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_UNOBTAINABLE_NUMBER;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_NORMAL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_NORMAL)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_NORMAL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_NORMAL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_BUSY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_BUSY)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_BUSY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_BUSY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_NORMAL_UNSPECIFIED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_NORMAL_UNSPECIFIED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_NORMAL_UNSPECIFIED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_NORMAL_UNSPECIFIED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_CONGESTION) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_CONGESTION)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CONGESTION";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_CONGESTION;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_INCOMPATIBILITY_DESTINATION) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_INCOMPATIBILITY_DESTINATION)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_INCOMPATIBILITY_DESTINATION";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_INCOMPATIBILITY_DESTINATION;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_CALL_BARRED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_CALL_BARRED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CALL_BARRED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_CALL_BARRED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_FDN_BLOCKED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_FDN_BLOCKED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_FDN_BLOCKED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_FDN_BLOCKED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_IMEI_NOT_ACCEPTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_IMEI_NOT_ACCEPTED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_IMEI_NOT_ACCEPTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_IMEI_NOT_ACCEPTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_USSD) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_USSD)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_MODIFIED_TO_USSD";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_USSD;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_SS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_SS)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_MODIFIED_TO_SS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_SS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_MODIFIED_TO_DIAL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_EMERGENCY_TEMP_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_EMERGENCY_TEMP_FAILURE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_EMERGENCY_TEMP_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_EMERGENCY_TEMP_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_EMERGENCY_PERM_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_EMERGENCY_PERM_FAILURE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_EMERGENCY_PERM_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_EMERGENCY_PERM_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_HO_NOT_FEASIBLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_HO_NOT_FEASIBLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_HO_NOT_FEASIBLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_HO_NOT_FEASIBLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_USER_BUSY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_USER_BUSY)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_USER_BUSY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_USER_BUSY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_USER_REJECT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_USER_REJECT)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_USER_REJECT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_USER_REJECT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_LOW_BATTERY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_LOW_BATTERY)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_LOW_BATTERY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_LOW_BATTERY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_BLACKLISTED_CALL_ID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_BLACKLISTED_CALL_ID)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_BLACKLISTED_CALL_ID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_BLACKLISTED_CALL_ID;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_CS_RETRY_REQUIRED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_CS_RETRY_REQUIRED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CS_RETRY_REQUIRED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_CS_RETRY_REQUIRED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_NETWORK_UNAVAILABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_NETWORK_UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_NETWORK_UNAVAILABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_NETWORK_UNAVAILABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_FEATURE_UNAVAILABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_FEATURE_UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_FEATURE_UNAVAILABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_FEATURE_UNAVAILABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_ERROR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_ERROR)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_ERROR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_ERROR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_MISC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_MISC)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_MISC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_MISC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_ANSWERED_ELSEWHERE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_ANSWERED_ELSEWHERE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_ANSWERED_ELSEWHERE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_ANSWERED_ELSEWHERE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_PULL_OUT_OF_SYNC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_PULL_OUT_OF_SYNC)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_PULL_OUT_OF_SYNC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_PULL_OUT_OF_SYNC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_CAUSE_CALL_PULLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_CAUSE_CALL_PULLED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CAUSE_CALL_PULLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_CAUSE_CALL_PULLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_ACCESS_CLASS_BLOCKED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_ACCESS_CLASS_BLOCKED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_ACCESS_CLASS_BLOCKED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_ACCESS_CLASS_BLOCKED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_REDIRECTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_REDIRECTED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_REDIRECTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_REDIRECTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_BAD_REQUEST) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_BAD_REQUEST)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_BAD_REQUEST";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_BAD_REQUEST;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_FORBIDDEN) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_FORBIDDEN)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_FORBIDDEN";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_FORBIDDEN;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_NOT_FOUND) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_NOT_FOUND)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_NOT_FOUND";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_NOT_FOUND;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_NOT_SUPPORTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_NOT_SUPPORTED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_NOT_SUPPORTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_NOT_SUPPORTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_REQUEST_TIMEOUT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_REQUEST_TIMEOUT)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_REQUEST_TIMEOUT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_REQUEST_TIMEOUT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_BAD_ADDRESS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_BAD_ADDRESS)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_BAD_ADDRESS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_BAD_ADDRESS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_BUSY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_BUSY)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_BUSY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_BUSY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_REQUEST_CANCELLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_REQUEST_CANCELLED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_REQUEST_CANCELLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_REQUEST_CANCELLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_NOT_ACCEPTABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_NOT_ACCEPTABLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_NOT_ACCEPTABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_NOT_ACCEPTABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_NOT_REACHABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_NOT_REACHABLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_NOT_REACHABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_NOT_REACHABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_INTERNAL_ERROR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_INTERNAL_ERROR)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVER_INTERNAL_ERROR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_INTERNAL_ERROR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_BAD_GATEWAY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_BAD_GATEWAY)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVER_BAD_GATEWAY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_BAD_GATEWAY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVICE_UNAVAILABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVICE_UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVICE_UNAVAILABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVICE_UNAVAILABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_TIMEOUT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_TIMEOUT)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVER_TIMEOUT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_TIMEOUT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_USER_REJECTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_USER_REJECTED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_USER_REJECTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_USER_REJECTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_GLOBAL_ERROR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_GLOBAL_ERROR)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_GLOBAL_ERROR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_GLOBAL_ERROR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_MEDIA_INIT_FAILED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_MEDIA_INIT_FAILED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_MEDIA_INIT_FAILED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_MEDIA_INIT_FAILED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_MEDIA_NO_DATA) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_MEDIA_NO_DATA)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_MEDIA_NO_DATA";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_MEDIA_NO_DATA;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_MEDIA_NOT_ACCEPTABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_MEDIA_NOT_ACCEPTABLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_MEDIA_NOT_ACCEPTABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_MEDIA_NOT_ACCEPTABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_MEDIA_UNSPECIFIED_ERROR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_MEDIA_UNSPECIFIED_ERROR)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_MEDIA_UNSPECIFIED_ERROR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_MEDIA_UNSPECIFIED_ERROR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_HOLD_RESUME_FAILED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_HOLD_RESUME_FAILED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_HOLD_RESUME_FAILED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_HOLD_RESUME_FAILED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_HOLD_RESUME_CANCELED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_HOLD_RESUME_CANCELED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_HOLD_RESUME_CANCELED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_HOLD_RESUME_CANCELED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_HOLD_REINVITE_COLLISION) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_HOLD_REINVITE_COLLISION)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_HOLD_REINVITE_COLLISION";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_HOLD_REINVITE_COLLISION;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_ERROR_UNSPECIFIED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_ERROR_UNSPECIFIED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_ERROR_UNSPECIFIED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_ERROR_UNSPECIFIED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_INVALID)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::CallFailCause o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_UNOBTAINABLE_NUMBER) {
        return "CALL_FAIL_UNOBTAINABLE_NUMBER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_NORMAL) {
        return "CALL_FAIL_NORMAL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_BUSY) {
        return "CALL_FAIL_BUSY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_NORMAL_UNSPECIFIED) {
        return "CALL_FAIL_NORMAL_UNSPECIFIED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_CONGESTION) {
        return "CALL_FAIL_CONGESTION";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_INCOMPATIBILITY_DESTINATION) {
        return "CALL_FAIL_INCOMPATIBILITY_DESTINATION";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_CALL_BARRED) {
        return "CALL_FAIL_CALL_BARRED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_FDN_BLOCKED) {
        return "CALL_FAIL_FDN_BLOCKED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_IMEI_NOT_ACCEPTED) {
        return "CALL_FAIL_IMEI_NOT_ACCEPTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_USSD) {
        return "CALL_FAIL_DIAL_MODIFIED_TO_USSD";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_SS) {
        return "CALL_FAIL_DIAL_MODIFIED_TO_SS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL) {
        return "CALL_FAIL_DIAL_MODIFIED_TO_DIAL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_EMERGENCY_TEMP_FAILURE) {
        return "CALL_FAIL_EMERGENCY_TEMP_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_EMERGENCY_PERM_FAILURE) {
        return "CALL_FAIL_EMERGENCY_PERM_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_HO_NOT_FEASIBLE) {
        return "CALL_FAIL_HO_NOT_FEASIBLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_USER_BUSY) {
        return "CALL_FAIL_USER_BUSY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_USER_REJECT) {
        return "CALL_FAIL_USER_REJECT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_LOW_BATTERY) {
        return "CALL_FAIL_LOW_BATTERY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_BLACKLISTED_CALL_ID) {
        return "CALL_FAIL_BLACKLISTED_CALL_ID";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_CS_RETRY_REQUIRED) {
        return "CALL_FAIL_CS_RETRY_REQUIRED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_NETWORK_UNAVAILABLE) {
        return "CALL_FAIL_NETWORK_UNAVAILABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_FEATURE_UNAVAILABLE) {
        return "CALL_FAIL_FEATURE_UNAVAILABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_ERROR) {
        return "CALL_FAIL_SIP_ERROR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_MISC) {
        return "CALL_FAIL_MISC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_ANSWERED_ELSEWHERE) {
        return "CALL_FAIL_ANSWERED_ELSEWHERE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_PULL_OUT_OF_SYNC) {
        return "CALL_FAIL_PULL_OUT_OF_SYNC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_CAUSE_CALL_PULLED) {
        return "CALL_FAIL_CAUSE_CALL_PULLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_ACCESS_CLASS_BLOCKED) {
        return "CALL_FAIL_ACCESS_CLASS_BLOCKED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_REDIRECTED) {
        return "CALL_FAIL_SIP_REDIRECTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_BAD_REQUEST) {
        return "CALL_FAIL_SIP_BAD_REQUEST";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_FORBIDDEN) {
        return "CALL_FAIL_SIP_FORBIDDEN";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_NOT_FOUND) {
        return "CALL_FAIL_SIP_NOT_FOUND";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_NOT_SUPPORTED) {
        return "CALL_FAIL_SIP_NOT_SUPPORTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_REQUEST_TIMEOUT) {
        return "CALL_FAIL_SIP_REQUEST_TIMEOUT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE) {
        return "CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_BAD_ADDRESS) {
        return "CALL_FAIL_SIP_BAD_ADDRESS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_BUSY) {
        return "CALL_FAIL_SIP_BUSY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_REQUEST_CANCELLED) {
        return "CALL_FAIL_SIP_REQUEST_CANCELLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_NOT_ACCEPTABLE) {
        return "CALL_FAIL_SIP_NOT_ACCEPTABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_NOT_REACHABLE) {
        return "CALL_FAIL_SIP_NOT_REACHABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_INTERNAL_ERROR) {
        return "CALL_FAIL_SIP_SERVER_INTERNAL_ERROR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED) {
        return "CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_BAD_GATEWAY) {
        return "CALL_FAIL_SIP_SERVER_BAD_GATEWAY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVICE_UNAVAILABLE) {
        return "CALL_FAIL_SIP_SERVICE_UNAVAILABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_TIMEOUT) {
        return "CALL_FAIL_SIP_SERVER_TIMEOUT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED) {
        return "CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE) {
        return "CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE) {
        return "CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_USER_REJECTED) {
        return "CALL_FAIL_SIP_USER_REJECTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_GLOBAL_ERROR) {
        return "CALL_FAIL_SIP_GLOBAL_ERROR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_MEDIA_INIT_FAILED) {
        return "CALL_FAIL_MEDIA_INIT_FAILED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_MEDIA_NO_DATA) {
        return "CALL_FAIL_MEDIA_NO_DATA";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_MEDIA_NOT_ACCEPTABLE) {
        return "CALL_FAIL_MEDIA_NOT_ACCEPTABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_MEDIA_UNSPECIFIED_ERROR) {
        return "CALL_FAIL_MEDIA_UNSPECIFIED_ERROR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_HOLD_RESUME_FAILED) {
        return "CALL_FAIL_HOLD_RESUME_FAILED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_HOLD_RESUME_CANCELED) {
        return "CALL_FAIL_HOLD_RESUME_CANCELED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_HOLD_REINVITE_COLLISION) {
        return "CALL_FAIL_HOLD_REINVITE_COLLISION";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO) {
        return "CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL) {
        return "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO) {
        return "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS) {
        return "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD) {
        return "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_ERROR_UNSPECIFIED) {
        return "CALL_FAIL_ERROR_UNSPECIFIED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_INVALID) {
        return "CALL_FAIL_INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::RegState lhs, const ::vendor::qti::hardware::radio::ims::V1_0::RegState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::RegState rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::RegState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::RegState lhs, const ::vendor::qti::hardware::radio::ims::V1_0::RegState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::RegState rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::RegState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::RegState e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::RegState e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::RegState>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::RegState> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RegState::REGISTERED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RegState::REGISTERED)) {
        os += (first ? "" : " | ");
        os += "REGISTERED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RegState::REGISTERED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RegState::NOT_REGISTERED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RegState::NOT_REGISTERED)) {
        os += (first ? "" : " | ");
        os += "NOT_REGISTERED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RegState::NOT_REGISTERED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RegState::REGISTERING) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RegState::REGISTERING)) {
        os += (first ? "" : " | ");
        os += "REGISTERING";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RegState::REGISTERING;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RegState::INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RegState::INVALID)) {
        os += (first ? "" : " | ");
        os += "INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RegState::INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::RegState o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RegState::REGISTERED) {
        return "REGISTERED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RegState::NOT_REGISTERED) {
        return "NOT_REGISTERED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RegState::REGISTERING) {
        return "REGISTERING";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RegState::INVALID) {
        return "INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::StatusType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::StatusType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::StatusType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::StatusType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::StatusType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::StatusType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::StatusType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::StatusType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::StatusType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::StatusType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::StatusType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::StatusType> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::StatusType::STATUS_DISABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::StatusType::STATUS_DISABLED)) {
        os += (first ? "" : " | ");
        os += "STATUS_DISABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::StatusType::STATUS_DISABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::StatusType::STATUS_PARTIALLY_ENABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::StatusType::STATUS_PARTIALLY_ENABLED)) {
        os += (first ? "" : " | ");
        os += "STATUS_PARTIALLY_ENABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::StatusType::STATUS_PARTIALLY_ENABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::StatusType::STATUS_ENABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::StatusType::STATUS_ENABLED)) {
        os += (first ? "" : " | ");
        os += "STATUS_ENABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::StatusType::STATUS_ENABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::StatusType::STATUS_NOT_SUPPORTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::StatusType::STATUS_NOT_SUPPORTED)) {
        os += (first ? "" : " | ");
        os += "STATUS_NOT_SUPPORTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::StatusType::STATUS_NOT_SUPPORTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::StatusType::STATUS_INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::StatusType::STATUS_INVALID)) {
        os += (first ? "" : " | ");
        os += "STATUS_INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::StatusType::STATUS_INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::StatusType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::StatusType::STATUS_DISABLED) {
        return "STATUS_DISABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::StatusType::STATUS_PARTIALLY_ENABLED) {
        return "STATUS_PARTIALLY_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::StatusType::STATUS_ENABLED) {
        return "STATUS_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::StatusType::STATUS_NOT_SUPPORTED) {
        return "STATUS_NOT_SUPPORTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::StatusType::STATUS_INVALID) {
        return "STATUS_INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::RadioTechType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::RadioTechType> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_ANY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_ANY)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_ANY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_ANY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_UNKNOWN) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_UNKNOWN";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_UNKNOWN;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_GPRS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_GPRS)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_GPRS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_GPRS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_EDGE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_EDGE)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_EDGE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_EDGE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_UMTS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_UMTS)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_UMTS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_UMTS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_IS95A) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_IS95A)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_IS95A";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_IS95A;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_IS95B) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_IS95B)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_IS95B";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_IS95B;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_1xRTT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_1xRTT)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_1xRTT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_1xRTT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_EVDO_0) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_EVDO_0)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_EVDO_0";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_EVDO_0;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_EVDO_A) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_EVDO_A)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_EVDO_A";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_EVDO_A;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_HSDPA) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_HSDPA)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_HSDPA";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_HSDPA;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_HSUPA) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_HSUPA)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_HSUPA";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_HSUPA;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_HSPA) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_HSPA)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_HSPA";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_HSPA;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_EVDO_B) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_EVDO_B)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_EVDO_B";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_EVDO_B;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_EHRPD) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_EHRPD)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_EHRPD";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_EHRPD;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_LTE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_LTE)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_LTE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_LTE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_HSPAP) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_HSPAP)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_HSPAP";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_HSPAP;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_GSM) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_GSM)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_GSM";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_GSM;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_TD_SCDMA) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_TD_SCDMA)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_TD_SCDMA";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_TD_SCDMA;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_WIFI) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_WIFI)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_WIFI";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_WIFI;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_IWLAN) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_IWLAN)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_IWLAN";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_IWLAN;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_INVALID)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::RadioTechType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_ANY) {
        return "RADIO_TECH_ANY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_UNKNOWN) {
        return "RADIO_TECH_UNKNOWN";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_GPRS) {
        return "RADIO_TECH_GPRS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_EDGE) {
        return "RADIO_TECH_EDGE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_UMTS) {
        return "RADIO_TECH_UMTS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_IS95A) {
        return "RADIO_TECH_IS95A";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_IS95B) {
        return "RADIO_TECH_IS95B";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_1xRTT) {
        return "RADIO_TECH_1xRTT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_EVDO_0) {
        return "RADIO_TECH_EVDO_0";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_EVDO_A) {
        return "RADIO_TECH_EVDO_A";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_HSDPA) {
        return "RADIO_TECH_HSDPA";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_HSUPA) {
        return "RADIO_TECH_HSUPA";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_HSPA) {
        return "RADIO_TECH_HSPA";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_EVDO_B) {
        return "RADIO_TECH_EVDO_B";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_EHRPD) {
        return "RADIO_TECH_EHRPD";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_LTE) {
        return "RADIO_TECH_LTE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_HSPAP) {
        return "RADIO_TECH_HSPAP";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_GSM) {
        return "RADIO_TECH_GSM";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_TD_SCDMA) {
        return "RADIO_TECH_TD_SCDMA";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_WIFI) {
        return "RADIO_TECH_WIFI";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_IWLAN) {
        return "RADIO_TECH_IWLAN";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_INVALID) {
        return "RADIO_TECH_INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::ConfigItem>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::ConfigItem> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_NONE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_NONE)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_NONE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_NONE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOCODER_AMRMODESET) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOCODER_AMRMODESET)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_VOCODER_AMRMODESET";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOCODER_AMRMODESET;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOCODER_AMRWBMODESET) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOCODER_AMRWBMODESET)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_VOCODER_AMRWBMODESET";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOCODER_AMRWBMODESET;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_SESSION_TIMER) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_SESSION_TIMER)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SIP_SESSION_TIMER";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_SESSION_TIMER;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_MIN_SESSION_EXPIRY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_MIN_SESSION_EXPIRY)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_MIN_SESSION_EXPIRY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_MIN_SESSION_EXPIRY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_CANCELLATION_TIMER) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_CANCELLATION_TIMER)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_CANCELLATION_TIMER";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_CANCELLATION_TIMER;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_T_DELAY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_T_DELAY)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_T_DELAY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_T_DELAY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SILENT_REDIAL_ENABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SILENT_REDIAL_ENABLE)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SILENT_REDIAL_ENABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SILENT_REDIAL_ENABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_T1_TIMER) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_T1_TIMER)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SIP_T1_TIMER";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_T1_TIMER;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_T2_TIMER) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_T2_TIMER)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SIP_T2_TIMER";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_T2_TIMER;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_TF_TIMER) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_TF_TIMER)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SIP_TF_TIMER";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_TF_TIMER;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VLT_SETTING_ENABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VLT_SETTING_ENABLED)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_VLT_SETTING_ENABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VLT_SETTING_ENABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_LVC_SETTING_ENABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_LVC_SETTING_ENABLED)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_LVC_SETTING_ENABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_LVC_SETTING_ENABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_DOMAIN_NAME) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_DOMAIN_NAME)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_DOMAIN_NAME";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_DOMAIN_NAME;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SMS_FORMAT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SMS_FORMAT)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SMS_FORMAT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SMS_FORMAT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SMS_OVER_IP) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SMS_OVER_IP)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SMS_OVER_IP";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SMS_OVER_IP;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_PUBLISH_TIMER) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_PUBLISH_TIMER)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_PUBLISH_TIMER";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_PUBLISH_TIMER;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_PUBLISH_TIMER_EXTENDED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_PUBLISH_TIMER_EXTENDED)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_PUBLISH_TIMER_EXTENDED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_PUBLISH_TIMER_EXTENDED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_GZIP_FLAG) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_GZIP_FLAG)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_GZIP_FLAG";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_GZIP_FLAG;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_EAB_SETTING_ENABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_EAB_SETTING_ENABLED)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_EAB_SETTING_ENABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_EAB_SETTING_ENABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_MOBILE_DATA_ENABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_MOBILE_DATA_ENABLED)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_MOBILE_DATA_ENABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_MOBILE_DATA_ENABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_MODE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_MODE)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_VOICE_OVER_WIFI_MODE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_MODE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_CAPABILITY_DISCOVERY_ENABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_CAPABILITY_DISCOVERY_ENABLED)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_CAPABILITY_DISCOVERY_ENABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_CAPABILITY_DISCOVERY_ENABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_EMERGENCY_CALL_TIMER) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_EMERGENCY_CALL_TIMER)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_EMERGENCY_CALL_TIMER";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_EMERGENCY_CALL_TIMER;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SSAC_HYSTERESIS_TIMER) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SSAC_HYSTERESIS_TIMER)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SSAC_HYSTERESIS_TIMER";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SSAC_HYSTERESIS_TIMER;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_LBO_PCSCF_ADDRESS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_LBO_PCSCF_ADDRESS)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_LBO_PCSCF_ADDRESS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_LBO_PCSCF_ADDRESS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_KEEP_ALIVE_ENABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_KEEP_ALIVE_ENABLED)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_KEEP_ALIVE_ENABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_KEEP_ALIVE_ENABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_REGISTRATION_RETRY_BASE_TIME_SEC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_REGISTRATION_RETRY_BASE_TIME_SEC)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_REGISTRATION_RETRY_BASE_TIME_SEC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_REGISTRATION_RETRY_BASE_TIME_SEC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_REGISTRATION_RETRY_MAX_TIME_SEC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_REGISTRATION_RETRY_MAX_TIME_SEC)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_REGISTRATION_RETRY_MAX_TIME_SEC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_REGISTRATION_RETRY_MAX_TIME_SEC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SPEECH_START_PORT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SPEECH_START_PORT)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SPEECH_START_PORT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SPEECH_START_PORT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SPEECH_END_PORT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SPEECH_END_PORT)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SPEECH_END_PORT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SPEECH_END_PORT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_INVITE_REQ_RETX_INTERVAL_MSEC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_INVITE_REQ_RETX_INTERVAL_MSEC)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SIP_INVITE_REQ_RETX_INTERVAL_MSEC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_INVITE_REQ_RETX_INTERVAL_MSEC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_WAIT_TIME_MSEC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_WAIT_TIME_MSEC)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SIP_INVITE_RSP_WAIT_TIME_MSEC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_WAIT_TIME_MSEC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_RETX_INTERVAL_MSEC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_RETX_INTERVAL_MSEC)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SIP_INVITE_RSP_RETX_INTERVAL_MSEC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_RETX_INTERVAL_MSEC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_ACK_RECEIPT_WAIT_TIME_MSEC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_ACK_RECEIPT_WAIT_TIME_MSEC)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SIP_ACK_RECEIPT_WAIT_TIME_MSEC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_ACK_RECEIPT_WAIT_TIME_MSEC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_ACK_RETX_WAIT_TIME_MSEC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_ACK_RETX_WAIT_TIME_MSEC)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SIP_ACK_RETX_WAIT_TIME_MSEC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_ACK_RETX_WAIT_TIME_MSEC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AMR_WB_BANDWIDTH_EFFICIENT_PT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AMR_WB_BANDWIDTH_EFFICIENT_PT)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_AMR_WB_BANDWIDTH_EFFICIENT_PT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AMR_WB_BANDWIDTH_EFFICIENT_PT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AMR_OCTET_ALIGNED_PT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AMR_OCTET_ALIGNED_PT)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_AMR_OCTET_ALIGNED_PT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AMR_OCTET_ALIGNED_PT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AMR_BANDWIDTH_EFFICIENT_PT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AMR_BANDWIDTH_EFFICIENT_PT)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_AMR_BANDWIDTH_EFFICIENT_PT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AMR_BANDWIDTH_EFFICIENT_PT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_DTMF_WB_PT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_DTMF_WB_PT)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_DTMF_WB_PT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_DTMF_WB_PT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_DTMF_NB_PT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_DTMF_NB_PT)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_DTMF_NB_PT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_DTMF_NB_PT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AMR_DEFAULT_MODE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AMR_DEFAULT_MODE)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_AMR_DEFAULT_MODE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AMR_DEFAULT_MODE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SMS_PSI) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SMS_PSI)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SMS_PSI";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SMS_PSI;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VIDEO_QUALITY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VIDEO_QUALITY)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_VIDEO_QUALITY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VIDEO_QUALITY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE1) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE1)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_THRESHOLD_LTE1";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE1;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE2) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE2)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_THRESHOLD_LTE2";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE2;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE3) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE3)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_THRESHOLD_LTE3";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE3;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_1x) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_1x)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_THRESHOLD_1x";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_1x;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_WIFI_A) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_WIFI_A)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_THRESHOLD_WIFI_A";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_WIFI_A;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_WIFI_B) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_WIFI_B)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_THRESHOLD_WIFI_B";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_WIFI_B;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_T_EPDG_LTE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_T_EPDG_LTE)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_T_EPDG_LTE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_T_EPDG_LTE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_T_EPDG_WIFI) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_T_EPDG_WIFI)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_T_EPDG_WIFI";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_T_EPDG_WIFI;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_T_EPDG_1x) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_T_EPDG_1x)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_T_EPDG_1x";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_T_EPDG_1x;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VWF_SETTING_ENABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VWF_SETTING_ENABLED)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_VWF_SETTING_ENABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VWF_SETTING_ENABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VCE_SETTING_ENABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VCE_SETTING_ENABLED)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_VCE_SETTING_ENABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VCE_SETTING_ENABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_RTT_SETTING_ENABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_RTT_SETTING_ENABLED)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_RTT_SETTING_ENABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_RTT_SETTING_ENABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SMS_APP) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SMS_APP)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SMS_APP";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SMS_APP;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VVM_APP) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VVM_APP)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_VVM_APP";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VVM_APP;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_INVALID)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::ConfigItem o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_NONE) {
        return "CONFIG_ITEM_NONE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOCODER_AMRMODESET) {
        return "CONFIG_ITEM_VOCODER_AMRMODESET";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOCODER_AMRWBMODESET) {
        return "CONFIG_ITEM_VOCODER_AMRWBMODESET";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_SESSION_TIMER) {
        return "CONFIG_ITEM_SIP_SESSION_TIMER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_MIN_SESSION_EXPIRY) {
        return "CONFIG_ITEM_MIN_SESSION_EXPIRY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_CANCELLATION_TIMER) {
        return "CONFIG_ITEM_CANCELLATION_TIMER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_T_DELAY) {
        return "CONFIG_ITEM_T_DELAY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SILENT_REDIAL_ENABLE) {
        return "CONFIG_ITEM_SILENT_REDIAL_ENABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_T1_TIMER) {
        return "CONFIG_ITEM_SIP_T1_TIMER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_T2_TIMER) {
        return "CONFIG_ITEM_SIP_T2_TIMER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_TF_TIMER) {
        return "CONFIG_ITEM_SIP_TF_TIMER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VLT_SETTING_ENABLED) {
        return "CONFIG_ITEM_VLT_SETTING_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_LVC_SETTING_ENABLED) {
        return "CONFIG_ITEM_LVC_SETTING_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_DOMAIN_NAME) {
        return "CONFIG_ITEM_DOMAIN_NAME";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SMS_FORMAT) {
        return "CONFIG_ITEM_SMS_FORMAT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SMS_OVER_IP) {
        return "CONFIG_ITEM_SMS_OVER_IP";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_PUBLISH_TIMER) {
        return "CONFIG_ITEM_PUBLISH_TIMER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_PUBLISH_TIMER_EXTENDED) {
        return "CONFIG_ITEM_PUBLISH_TIMER_EXTENDED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION) {
        return "CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION) {
        return "CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL) {
        return "CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH) {
        return "CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL) {
        return "CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP) {
        return "CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_GZIP_FLAG) {
        return "CONFIG_ITEM_GZIP_FLAG";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_EAB_SETTING_ENABLED) {
        return "CONFIG_ITEM_EAB_SETTING_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_MOBILE_DATA_ENABLED) {
        return "CONFIG_ITEM_MOBILE_DATA_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED) {
        return "CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING) {
        return "CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_MODE) {
        return "CONFIG_ITEM_VOICE_OVER_WIFI_MODE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_CAPABILITY_DISCOVERY_ENABLED) {
        return "CONFIG_ITEM_CAPABILITY_DISCOVERY_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_EMERGENCY_CALL_TIMER) {
        return "CONFIG_ITEM_EMERGENCY_CALL_TIMER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SSAC_HYSTERESIS_TIMER) {
        return "CONFIG_ITEM_SSAC_HYSTERESIS_TIMER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS) {
        return "CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_LBO_PCSCF_ADDRESS) {
        return "CONFIG_ITEM_LBO_PCSCF_ADDRESS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_KEEP_ALIVE_ENABLED) {
        return "CONFIG_ITEM_KEEP_ALIVE_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_REGISTRATION_RETRY_BASE_TIME_SEC) {
        return "CONFIG_ITEM_REGISTRATION_RETRY_BASE_TIME_SEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_REGISTRATION_RETRY_MAX_TIME_SEC) {
        return "CONFIG_ITEM_REGISTRATION_RETRY_MAX_TIME_SEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SPEECH_START_PORT) {
        return "CONFIG_ITEM_SPEECH_START_PORT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SPEECH_END_PORT) {
        return "CONFIG_ITEM_SPEECH_END_PORT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_INVITE_REQ_RETX_INTERVAL_MSEC) {
        return "CONFIG_ITEM_SIP_INVITE_REQ_RETX_INTERVAL_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_WAIT_TIME_MSEC) {
        return "CONFIG_ITEM_SIP_INVITE_RSP_WAIT_TIME_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC) {
        return "CONFIG_ITEM_SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC) {
        return "CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC) {
        return "CONFIG_ITEM_SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_RETX_INTERVAL_MSEC) {
        return "CONFIG_ITEM_SIP_INVITE_RSP_RETX_INTERVAL_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_ACK_RECEIPT_WAIT_TIME_MSEC) {
        return "CONFIG_ITEM_SIP_ACK_RECEIPT_WAIT_TIME_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_ACK_RETX_WAIT_TIME_MSEC) {
        return "CONFIG_ITEM_SIP_ACK_RETX_WAIT_TIME_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC) {
        return "CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC) {
        return "CONFIG_ITEM_SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT) {
        return "CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AMR_WB_BANDWIDTH_EFFICIENT_PT) {
        return "CONFIG_ITEM_AMR_WB_BANDWIDTH_EFFICIENT_PT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AMR_OCTET_ALIGNED_PT) {
        return "CONFIG_ITEM_AMR_OCTET_ALIGNED_PT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AMR_BANDWIDTH_EFFICIENT_PT) {
        return "CONFIG_ITEM_AMR_BANDWIDTH_EFFICIENT_PT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_DTMF_WB_PT) {
        return "CONFIG_ITEM_DTMF_WB_PT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_DTMF_NB_PT) {
        return "CONFIG_ITEM_DTMF_NB_PT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AMR_DEFAULT_MODE) {
        return "CONFIG_ITEM_AMR_DEFAULT_MODE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SMS_PSI) {
        return "CONFIG_ITEM_SMS_PSI";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VIDEO_QUALITY) {
        return "CONFIG_ITEM_VIDEO_QUALITY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE1) {
        return "CONFIG_ITEM_THRESHOLD_LTE1";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE2) {
        return "CONFIG_ITEM_THRESHOLD_LTE2";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE3) {
        return "CONFIG_ITEM_THRESHOLD_LTE3";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_1x) {
        return "CONFIG_ITEM_THRESHOLD_1x";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_WIFI_A) {
        return "CONFIG_ITEM_THRESHOLD_WIFI_A";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_WIFI_B) {
        return "CONFIG_ITEM_THRESHOLD_WIFI_B";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_T_EPDG_LTE) {
        return "CONFIG_ITEM_T_EPDG_LTE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_T_EPDG_WIFI) {
        return "CONFIG_ITEM_T_EPDG_WIFI";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_T_EPDG_1x) {
        return "CONFIG_ITEM_T_EPDG_1x";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VWF_SETTING_ENABLED) {
        return "CONFIG_ITEM_VWF_SETTING_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VCE_SETTING_ENABLED) {
        return "CONFIG_ITEM_VCE_SETTING_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_RTT_SETTING_ENABLED) {
        return "CONFIG_ITEM_RTT_SETTING_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SMS_APP) {
        return "CONFIG_ITEM_SMS_APP";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VVM_APP) {
        return "CONFIG_ITEM_VVM_APP";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_INVALID) {
        return "CONFIG_ITEM_INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_NO_ERR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_NO_ERR)) {
        os += (first ? "" : " | ");
        os += "CONFIG_NO_ERR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_NO_ERR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_IMS_NOT_READY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_IMS_NOT_READY)) {
        os += (first ? "" : " | ");
        os += "CONFIG_IMS_NOT_READY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_IMS_NOT_READY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_FILE_NOT_AVAILABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_FILE_NOT_AVAILABLE)) {
        os += (first ? "" : " | ");
        os += "CONFIG_FILE_NOT_AVAILABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_FILE_NOT_AVAILABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_READ_FAILED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_READ_FAILED)) {
        os += (first ? "" : " | ");
        os += "CONFIG_READ_FAILED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_READ_FAILED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_WRITE_FAILED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_WRITE_FAILED)) {
        os += (first ? "" : " | ");
        os += "CONFIG_WRITE_FAILED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_WRITE_FAILED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_OTHER_INTERNAL_ERR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_OTHER_INTERNAL_ERR)) {
        os += (first ? "" : " | ");
        os += "CONFIG_OTHER_INTERNAL_ERR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_OTHER_INTERNAL_ERR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_FAILURE_INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_FAILURE_INVALID)) {
        os += (first ? "" : " | ");
        os += "CONFIG_FAILURE_INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_FAILURE_INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_NO_ERR) {
        return "CONFIG_NO_ERR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_IMS_NOT_READY) {
        return "CONFIG_IMS_NOT_READY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_FILE_NOT_AVAILABLE) {
        return "CONFIG_FILE_NOT_AVAILABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_READ_FAILED) {
        return "CONFIG_READ_FAILED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_WRITE_FAILED) {
        return "CONFIG_WRITE_FAILED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_OTHER_INTERNAL_ERR) {
        return "CONFIG_OTHER_INTERNAL_ERR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_FAILURE_INVALID) {
        return "CONFIG_FAILURE_INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::ToneOperation lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ToneOperation rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ToneOperation rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::ToneOperation lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::ToneOperation lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ToneOperation rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ToneOperation rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::ToneOperation lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::ToneOperation e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::ToneOperation e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::ToneOperation>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::ToneOperation> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ToneOperation::STOP) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ToneOperation::STOP)) {
        os += (first ? "" : " | ");
        os += "STOP";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ToneOperation::STOP;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ToneOperation::START) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ToneOperation::START)) {
        os += (first ? "" : " | ");
        os += "START";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ToneOperation::START;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ToneOperation::INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ToneOperation::INVALID)) {
        os += (first ? "" : " | ");
        os += "INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ToneOperation::INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::ToneOperation o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ToneOperation::STOP) {
        return "STOP";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ToneOperation::START) {
        return "START";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ToneOperation::INVALID) {
        return "INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::HandoverType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::HandoverType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::HandoverType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::HandoverType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::HandoverType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::HandoverType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::HandoverType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::HandoverType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::HandoverType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::HandoverType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::HandoverType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::HandoverType> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::HandoverType::START) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::HandoverType::START)) {
        os += (first ? "" : " | ");
        os += "START";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::HandoverType::START;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::HandoverType::COMPLETE_SUCCESS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::HandoverType::COMPLETE_SUCCESS)) {
        os += (first ? "" : " | ");
        os += "COMPLETE_SUCCESS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::HandoverType::COMPLETE_SUCCESS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::HandoverType::COMPLETE_FAIL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::HandoverType::COMPLETE_FAIL)) {
        os += (first ? "" : " | ");
        os += "COMPLETE_FAIL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::HandoverType::COMPLETE_FAIL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::HandoverType::CANCEL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::HandoverType::CANCEL)) {
        os += (first ? "" : " | ");
        os += "CANCEL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::HandoverType::CANCEL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::HandoverType::NOT_TRIGGERED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::HandoverType::NOT_TRIGGERED)) {
        os += (first ? "" : " | ");
        os += "NOT_TRIGGERED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::HandoverType::NOT_TRIGGERED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::HandoverType::NOT_TRIGGERED_MOBILE_DATA_OFF) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::HandoverType::NOT_TRIGGERED_MOBILE_DATA_OFF)) {
        os += (first ? "" : " | ");
        os += "NOT_TRIGGERED_MOBILE_DATA_OFF";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::HandoverType::NOT_TRIGGERED_MOBILE_DATA_OFF;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::HandoverType::INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::HandoverType::INVALID)) {
        os += (first ? "" : " | ");
        os += "INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::HandoverType::INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::HandoverType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::HandoverType::START) {
        return "START";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::HandoverType::COMPLETE_SUCCESS) {
        return "COMPLETE_SUCCESS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::HandoverType::COMPLETE_FAIL) {
        return "COMPLETE_FAIL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::HandoverType::CANCEL) {
        return "CANCEL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::HandoverType::NOT_TRIGGERED) {
        return "NOT_TRIGGERED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::HandoverType::NOT_TRIGGERED_MOBILE_DATA_OFF) {
        return "NOT_TRIGGERED_MOBILE_DATA_OFF";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::HandoverType::INVALID) {
        return "INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::CallState lhs, const ::vendor::qti::hardware::radio::ims::V1_0::CallState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::CallState rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::CallState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::CallState lhs, const ::vendor::qti::hardware::radio::ims::V1_0::CallState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::CallState rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::CallState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::CallState e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::CallState e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::CallState>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::CallState> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_ACTIVE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_ACTIVE)) {
        os += (first ? "" : " | ");
        os += "CALL_ACTIVE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_ACTIVE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_HOLDING) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_HOLDING)) {
        os += (first ? "" : " | ");
        os += "CALL_HOLDING";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_HOLDING;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_DIALING) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_DIALING)) {
        os += (first ? "" : " | ");
        os += "CALL_DIALING";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_DIALING;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_ALERTING) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_ALERTING)) {
        os += (first ? "" : " | ");
        os += "CALL_ALERTING";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_ALERTING;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_INCOMING) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_INCOMING)) {
        os += (first ? "" : " | ");
        os += "CALL_INCOMING";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_INCOMING;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_WAITING) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_WAITING)) {
        os += (first ? "" : " | ");
        os += "CALL_WAITING";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_WAITING;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_END) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_END)) {
        os += (first ? "" : " | ");
        os += "CALL_END";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_END;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_STATE_INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_STATE_INVALID)) {
        os += (first ? "" : " | ");
        os += "CALL_STATE_INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_STATE_INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::CallState o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_ACTIVE) {
        return "CALL_ACTIVE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_HOLDING) {
        return "CALL_HOLDING";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_DIALING) {
        return "CALL_DIALING";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_ALERTING) {
        return "CALL_ALERTING";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_INCOMING) {
        return "CALL_INCOMING";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_WAITING) {
        return "CALL_WAITING";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_END) {
        return "CALL_END";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_STATE_INVALID) {
        return "CALL_STATE_INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::ServiceType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ServiceType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ServiceType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::ServiceType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::ServiceType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ServiceType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ServiceType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::ServiceType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::ServiceType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::ServiceType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::ServiceType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::ServiceType> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ServiceType::SMS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ServiceType::SMS)) {
        os += (first ? "" : " | ");
        os += "SMS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ServiceType::SMS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ServiceType::VOIP) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ServiceType::VOIP)) {
        os += (first ? "" : " | ");
        os += "VOIP";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ServiceType::VOIP;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ServiceType::VT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ServiceType::VT)) {
        os += (first ? "" : " | ");
        os += "VT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ServiceType::VT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ServiceType::INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ServiceType::INVALID)) {
        os += (first ? "" : " | ");
        os += "INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ServiceType::INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::ServiceType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ServiceType::SMS) {
        return "SMS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ServiceType::VOIP) {
        return "VOIP";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ServiceType::VT) {
        return "VT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ServiceType::INVALID) {
        return "INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::ExtraType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ExtraType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ExtraType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::ExtraType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::ExtraType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ExtraType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ExtraType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::ExtraType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::ExtraType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::ExtraType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::ExtraType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::ExtraType> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ExtraType::LTE_TO_IWLAN_HO_FAIL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ExtraType::LTE_TO_IWLAN_HO_FAIL)) {
        os += (first ? "" : " | ");
        os += "LTE_TO_IWLAN_HO_FAIL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ExtraType::LTE_TO_IWLAN_HO_FAIL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ExtraType::INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ExtraType::INVALID)) {
        os += (first ? "" : " | ");
        os += "INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ExtraType::INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::ExtraType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ExtraType::LTE_TO_IWLAN_HO_FAIL) {
        return "LTE_TO_IWLAN_HO_FAIL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ExtraType::INVALID) {
        return "INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::RadioState lhs, const ::vendor::qti::hardware::radio::ims::V1_0::RadioState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::RadioState rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::RadioState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::RadioState lhs, const ::vendor::qti::hardware::radio::ims::V1_0::RadioState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::RadioState rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::RadioState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::RadioState e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::RadioState e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::RadioState>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::RadioState> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RadioState::RADIO_STATE_OFF) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RadioState::RADIO_STATE_OFF)) {
        os += (first ? "" : " | ");
        os += "RADIO_STATE_OFF";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RadioState::RADIO_STATE_OFF;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RadioState::RADIO_STATE_UNAVAILABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RadioState::RADIO_STATE_UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "RADIO_STATE_UNAVAILABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RadioState::RADIO_STATE_UNAVAILABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RadioState::RADIO_STATE_ON) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RadioState::RADIO_STATE_ON)) {
        os += (first ? "" : " | ");
        os += "RADIO_STATE_ON";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RadioState::RADIO_STATE_ON;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RadioState::RADIO_STATE_INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RadioState::RADIO_STATE_INVALID)) {
        os += (first ? "" : " | ");
        os += "RADIO_STATE_INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RadioState::RADIO_STATE_INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::RadioState o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RadioState::RADIO_STATE_OFF) {
        return "RADIO_STATE_OFF";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RadioState::RADIO_STATE_UNAVAILABLE) {
        return "RADIO_STATE_UNAVAILABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RadioState::RADIO_STATE_ON) {
        return "RADIO_STATE_ON";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RadioState::RADIO_STATE_INVALID) {
        return "RADIO_STATE_INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus::DISABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus::DISABLED)) {
        os += (first ? "" : " | ");
        os += "DISABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus::DISABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus::ENABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus::ENABLED)) {
        os += (first ? "" : " | ");
        os += "ENABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus::ENABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus::INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus::INVALID)) {
        os += (first ? "" : " | ");
        os += "INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus::INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus::DISABLED) {
        return "DISABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus::ENABLED) {
        return "ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus::INVALID) {
        return "INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::FacilityType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::FacilityType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::FacilityType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::FacilityType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::FacilityType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::FacilityType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::FacilityType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::FacilityType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::FacilityType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::FacilityType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::FacilityType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::FacilityType> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_CLIP) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_CLIP)) {
        os += (first ? "" : " | ");
        os += "FACILITY_CLIP";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_CLIP;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_COLP) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_COLP)) {
        os += (first ? "" : " | ");
        os += "FACILITY_COLP";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_COLP;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAOC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAOC)) {
        os += (first ? "" : " | ");
        os += "FACILITY_BAOC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAOC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAOIC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAOIC)) {
        os += (first ? "" : " | ");
        os += "FACILITY_BAOIC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAOIC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAOICxH) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAOICxH)) {
        os += (first ? "" : " | ");
        os += "FACILITY_BAOICxH";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAOICxH;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAIC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAIC)) {
        os += (first ? "" : " | ");
        os += "FACILITY_BAIC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAIC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAICr) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAICr)) {
        os += (first ? "" : " | ");
        os += "FACILITY_BAICr";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAICr;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BA_ALL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BA_ALL)) {
        os += (first ? "" : " | ");
        os += "FACILITY_BA_ALL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BA_ALL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BA_MO) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BA_MO)) {
        os += (first ? "" : " | ");
        os += "FACILITY_BA_MO";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BA_MO;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BA_MT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BA_MT)) {
        os += (first ? "" : " | ");
        os += "FACILITY_BA_MT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BA_MT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BS_MT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BS_MT)) {
        os += (first ? "" : " | ");
        os += "FACILITY_BS_MT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BS_MT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAICa) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAICa)) {
        os += (first ? "" : " | ");
        os += "FACILITY_BAICa";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAICa;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_INVALID)) {
        os += (first ? "" : " | ");
        os += "FACILITY_INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::FacilityType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_CLIP) {
        return "FACILITY_CLIP";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_COLP) {
        return "FACILITY_COLP";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAOC) {
        return "FACILITY_BAOC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAOIC) {
        return "FACILITY_BAOIC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAOICxH) {
        return "FACILITY_BAOICxH";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAIC) {
        return "FACILITY_BAIC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAICr) {
        return "FACILITY_BAICr";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BA_ALL) {
        return "FACILITY_BA_ALL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BA_MO) {
        return "FACILITY_BA_MO";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BA_MT) {
        return "FACILITY_BA_MT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BS_MT) {
        return "FACILITY_BS_MT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAICa) {
        return "FACILITY_BAICa";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_INVALID) {
        return "FACILITY_INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::ClipStatus>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::ClipStatus> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus::NOT_PROVISIONED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ClipStatus::NOT_PROVISIONED)) {
        os += (first ? "" : " | ");
        os += "NOT_PROVISIONED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus::NOT_PROVISIONED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus::PROVISIONED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ClipStatus::PROVISIONED)) {
        os += (first ? "" : " | ");
        os += "PROVISIONED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus::PROVISIONED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus::STATUS_UNKNOWN) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ClipStatus::STATUS_UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "STATUS_UNKNOWN";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus::STATUS_UNKNOWN;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus::INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ClipStatus::INVALID)) {
        os += (first ? "" : " | ");
        os += "INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus::INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::ClipStatus o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus::NOT_PROVISIONED) {
        return "NOT_PROVISIONED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus::PROVISIONED) {
        return "PROVISIONED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus::STATUS_UNKNOWN) {
        return "STATUS_UNKNOWN";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus::INVALID) {
        return "INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::TtyMode lhs, const ::vendor::qti::hardware::radio::ims::V1_0::TtyMode rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::TtyMode rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::TtyMode lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::TtyMode lhs, const ::vendor::qti::hardware::radio::ims::V1_0::TtyMode rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::TtyMode rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::TtyMode lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::TtyMode e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::TtyMode e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::TtyMode>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::TtyMode> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::TtyMode::TTY_MODE_OFF) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::TtyMode::TTY_MODE_OFF)) {
        os += (first ? "" : " | ");
        os += "TTY_MODE_OFF";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::TtyMode::TTY_MODE_OFF;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::TtyMode::TTY_MODE_FULL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::TtyMode::TTY_MODE_FULL)) {
        os += (first ? "" : " | ");
        os += "TTY_MODE_FULL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::TtyMode::TTY_MODE_FULL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::TtyMode::TTY_MODE_HCO) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::TtyMode::TTY_MODE_HCO)) {
        os += (first ? "" : " | ");
        os += "TTY_MODE_HCO";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::TtyMode::TTY_MODE_HCO;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::TtyMode::TTY_MODE_VCO) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::TtyMode::TTY_MODE_VCO)) {
        os += (first ? "" : " | ");
        os += "TTY_MODE_VCO";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::TtyMode::TTY_MODE_VCO;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::TtyMode::TTY_MODE_INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::TtyMode::TTY_MODE_INVALID)) {
        os += (first ? "" : " | ");
        os += "TTY_MODE_INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::TtyMode::TTY_MODE_INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::TtyMode o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::TtyMode::TTY_MODE_OFF) {
        return "TTY_MODE_OFF";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::TtyMode::TTY_MODE_FULL) {
        return "TTY_MODE_FULL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::TtyMode::TTY_MODE_HCO) {
        return "TTY_MODE_HCO";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::TtyMode::TTY_MODE_VCO) {
        return "TTY_MODE_VCO";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::TtyMode::TTY_MODE_INVALID) {
        return "TTY_MODE_INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState::RINGING) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState::RINGING)) {
        os += (first ? "" : " | ");
        os += "RINGING";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState::RINGING;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState::FOREGROUND) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState::FOREGROUND)) {
        os += (first ? "" : " | ");
        os += "FOREGROUND";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState::FOREGROUND;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState::BACKGROUND) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState::BACKGROUND)) {
        os += (first ? "" : " | ");
        os += "BACKGROUND";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState::BACKGROUND;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState::INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState::INVALID)) {
        os += (first ? "" : " | ");
        os += "INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState::INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState::RINGING) {
        return "RINGING";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState::FOREGROUND) {
        return "FOREGROUND";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState::BACKGROUND) {
        return "BACKGROUND";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState::INVALID) {
        return "INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause lhs, const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause lhs, const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_SUCCESS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_SUCCESS)) {
        os += (first ? "" : " | ");
        os += "E_SUCCESS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_SUCCESS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_RADIO_NOT_AVAILABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_RADIO_NOT_AVAILABLE)) {
        os += (first ? "" : " | ");
        os += "E_RADIO_NOT_AVAILABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_RADIO_NOT_AVAILABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_GENERIC_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_GENERIC_FAILURE)) {
        os += (first ? "" : " | ");
        os += "E_GENERIC_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_GENERIC_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_REQUEST_NOT_SUPPORTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_REQUEST_NOT_SUPPORTED)) {
        os += (first ? "" : " | ");
        os += "E_REQUEST_NOT_SUPPORTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_REQUEST_NOT_SUPPORTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_CANCELLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_CANCELLED)) {
        os += (first ? "" : " | ");
        os += "E_CANCELLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_CANCELLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_UNUSED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_UNUSED)) {
        os += (first ? "" : " | ");
        os += "E_UNUSED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_UNUSED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_INVALID_PARAMETER) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_INVALID_PARAMETER)) {
        os += (first ? "" : " | ");
        os += "E_INVALID_PARAMETER";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_INVALID_PARAMETER;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_REJECTED_BY_REMOTE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_REJECTED_BY_REMOTE)) {
        os += (first ? "" : " | ");
        os += "E_REJECTED_BY_REMOTE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_REJECTED_BY_REMOTE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_IMS_DEREGISTERED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_IMS_DEREGISTERED)) {
        os += (first ? "" : " | ");
        os += "E_IMS_DEREGISTERED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_IMS_DEREGISTERED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_NETWORK_NOT_SUPPORTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_NETWORK_NOT_SUPPORTED)) {
        os += (first ? "" : " | ");
        os += "E_NETWORK_NOT_SUPPORTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_NETWORK_NOT_SUPPORTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_HOLD_RESUME_FAILED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_HOLD_RESUME_FAILED)) {
        os += (first ? "" : " | ");
        os += "E_HOLD_RESUME_FAILED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_HOLD_RESUME_FAILED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_HOLD_RESUME_CANCELED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_HOLD_RESUME_CANCELED)) {
        os += (first ? "" : " | ");
        os += "E_HOLD_RESUME_CANCELED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_HOLD_RESUME_CANCELED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_REINVITE_COLLISION) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_REINVITE_COLLISION)) {
        os += (first ? "" : " | ");
        os += "E_REINVITE_COLLISION";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_REINVITE_COLLISION;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_INVALID)) {
        os += (first ? "" : " | ");
        os += "E_INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_SUCCESS) {
        return "E_SUCCESS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_RADIO_NOT_AVAILABLE) {
        return "E_RADIO_NOT_AVAILABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_GENERIC_FAILURE) {
        return "E_GENERIC_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_REQUEST_NOT_SUPPORTED) {
        return "E_REQUEST_NOT_SUPPORTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_CANCELLED) {
        return "E_CANCELLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_UNUSED) {
        return "E_UNUSED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_INVALID_PARAMETER) {
        return "E_INVALID_PARAMETER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_REJECTED_BY_REMOTE) {
        return "E_REJECTED_BY_REMOTE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_IMS_DEREGISTERED) {
        return "E_IMS_DEREGISTERED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_NETWORK_NOT_SUPPORTED) {
        return "E_NETWORK_NOT_SUPPORTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_HOLD_RESUME_FAILED) {
        return "E_HOLD_RESUME_FAILED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_HOLD_RESUME_CANCELED) {
        return "E_HOLD_RESUME_CANCELED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_REINVITE_COLLISION) {
        return "E_REINVITE_COLLISION";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_INVALID) {
        return "E_INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::NotificationType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::NotificationType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::NotificationType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::NotificationType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::NotificationType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::NotificationType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::NotificationType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::NotificationType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::NotificationType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::NotificationType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::NotificationType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::NotificationType> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::NotificationType::MO) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::NotificationType::MO)) {
        os += (first ? "" : " | ");
        os += "MO";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::NotificationType::MO;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::NotificationType::MT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::NotificationType::MT)) {
        os += (first ? "" : " | ");
        os += "MT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::NotificationType::MT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::NotificationType::INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::NotificationType::INVALID)) {
        os += (first ? "" : " | ");
        os += "INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::NotificationType::INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::NotificationType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::NotificationType::MO) {
        return "MO";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::NotificationType::MT) {
        return "MT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::NotificationType::INVALID) {
        return "INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::MessageType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::MessageType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::MessageType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::MessageType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::MessageType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::MessageType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::MessageType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::MessageType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::MessageType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::MessageType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::MessageType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::MessageType> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_NONE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_NONE)) {
        os += (first ? "" : " | ");
        os += "MWI_MSG_NONE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_NONE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_VOICE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_VOICE)) {
        os += (first ? "" : " | ");
        os += "MWI_MSG_VOICE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_VOICE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_VIDEO) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_VIDEO)) {
        os += (first ? "" : " | ");
        os += "MWI_MSG_VIDEO";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_VIDEO;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_FAX) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_FAX)) {
        os += (first ? "" : " | ");
        os += "MWI_MSG_FAX";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_FAX;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_PAGER) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_PAGER)) {
        os += (first ? "" : " | ");
        os += "MWI_MSG_PAGER";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_PAGER;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_MULTIMEDIA) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_MULTIMEDIA)) {
        os += (first ? "" : " | ");
        os += "MWI_MSG_MULTIMEDIA";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_MULTIMEDIA;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_TEXT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_TEXT)) {
        os += (first ? "" : " | ");
        os += "MWI_MSG_TEXT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_TEXT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_INVALID)) {
        os += (first ? "" : " | ");
        os += "MWI_MSG_INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::MessageType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_NONE) {
        return "MWI_MSG_NONE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_VOICE) {
        return "MWI_MSG_VOICE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_VIDEO) {
        return "MWI_MSG_VIDEO";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_FAX) {
        return "MWI_MSG_FAX";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_PAGER) {
        return "MWI_MSG_PAGER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_MULTIMEDIA) {
        return "MWI_MSG_MULTIMEDIA";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_TEXT) {
        return "MWI_MSG_TEXT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_INVALID) {
        return "MWI_MSG_INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority lhs, const ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority lhs, const ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::MessagePriority>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::MessagePriority> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority::MWI_MSG_PRIORITY_UNKNOWN) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::MessagePriority::MWI_MSG_PRIORITY_UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "MWI_MSG_PRIORITY_UNKNOWN";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority::MWI_MSG_PRIORITY_UNKNOWN;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority::MWI_MSG_PRIORITY_LOW) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::MessagePriority::MWI_MSG_PRIORITY_LOW)) {
        os += (first ? "" : " | ");
        os += "MWI_MSG_PRIORITY_LOW";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority::MWI_MSG_PRIORITY_LOW;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority::MWI_MSG_PRIORITY_NORMAL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::MessagePriority::MWI_MSG_PRIORITY_NORMAL)) {
        os += (first ? "" : " | ");
        os += "MWI_MSG_PRIORITY_NORMAL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority::MWI_MSG_PRIORITY_NORMAL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority::MWI_MSG_PRIORITY_URGENT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::MessagePriority::MWI_MSG_PRIORITY_URGENT)) {
        os += (first ? "" : " | ");
        os += "MWI_MSG_PRIORITY_URGENT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority::MWI_MSG_PRIORITY_URGENT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority::MWI_MSG_PRIORITY_INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::MessagePriority::MWI_MSG_PRIORITY_INVALID)) {
        os += (first ? "" : " | ");
        os += "MWI_MSG_PRIORITY_INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority::MWI_MSG_PRIORITY_INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::MessagePriority o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority::MWI_MSG_PRIORITY_UNKNOWN) {
        return "MWI_MSG_PRIORITY_UNKNOWN";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority::MWI_MSG_PRIORITY_LOW) {
        return "MWI_MSG_PRIORITY_LOW";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority::MWI_MSG_PRIORITY_NORMAL) {
        return "MWI_MSG_PRIORITY_NORMAL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority::MWI_MSG_PRIORITY_URGENT) {
        return "MWI_MSG_PRIORITY_URGENT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority::MWI_MSG_PRIORITY_INVALID) {
        return "MWI_MSG_PRIORITY_INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::EctType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::EctType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::EctType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::EctType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::EctType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::EctType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::EctType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::EctType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::EctType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::EctType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::EctType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::EctType> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::EctType::BLIND_TRANSFER) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::EctType::BLIND_TRANSFER)) {
        os += (first ? "" : " | ");
        os += "BLIND_TRANSFER";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::EctType::BLIND_TRANSFER;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::EctType::ASSURED_TRANSFER) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::EctType::ASSURED_TRANSFER)) {
        os += (first ? "" : " | ");
        os += "ASSURED_TRANSFER";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::EctType::ASSURED_TRANSFER;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::EctType::CONSULTATIVE_TRANSFER) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::EctType::CONSULTATIVE_TRANSFER)) {
        os += (first ? "" : " | ");
        os += "CONSULTATIVE_TRANSFER";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::EctType::CONSULTATIVE_TRANSFER;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::EctType::INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::EctType::INVALID)) {
        os += (first ? "" : " | ");
        os += "INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::EctType::INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::EctType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::EctType::BLIND_TRANSFER) {
        return "BLIND_TRANSFER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::EctType::ASSURED_TRANSFER) {
        return "ASSURED_TRANSFER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::EctType::CONSULTATIVE_TRANSFER) {
        return "CONSULTATIVE_TRANSFER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::EctType::INVALID) {
        return "INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation::ADD) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation::ADD)) {
        os += (first ? "" : " | ");
        os += "ADD";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation::ADD;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation::REMOVE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation::REMOVE)) {
        os += (first ? "" : " | ");
        os += "REMOVE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation::REMOVE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation::INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation::INVALID)) {
        os += (first ? "" : " | ");
        os += "INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation::INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation::ADD) {
        return "ADD";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation::REMOVE) {
        return "REMOVE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation::INVALID) {
        return "INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType::PDP_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType::PDP_FAILURE)) {
        os += (first ? "" : " | ");
        os += "PDP_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType::PDP_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType::REGISTRATION_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType::REGISTRATION_FAILURE)) {
        os += (first ? "" : " | ");
        os += "REGISTRATION_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType::REGISTRATION_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType::HANDOVER_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType::HANDOVER_FAILURE)) {
        os += (first ? "" : " | ");
        os += "HANDOVER_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType::HANDOVER_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType::OTHER_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType::OTHER_FAILURE)) {
        os += (first ? "" : " | ");
        os += "OTHER_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType::OTHER_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType::INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType::INVALID)) {
        os += (first ? "" : " | ");
        os += "INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType::INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType::PDP_FAILURE) {
        return "PDP_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType::REGISTRATION_FAILURE) {
        return "REGISTRATION_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType::HANDOVER_FAILURE) {
        return "HANDOVER_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType::OTHER_FAILURE) {
        return "OTHER_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType::INVALID) {
        return "INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::UNSPECIFIED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::UNSPECIFIED)) {
        os += (first ? "" : " | ");
        os += "UNSPECIFIED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::UNSPECIFIED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::MOBILE_IP) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::MOBILE_IP)) {
        os += (first ? "" : " | ");
        os += "MOBILE_IP";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::MOBILE_IP;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::INTERNAL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::INTERNAL)) {
        os += (first ? "" : " | ");
        os += "INTERNAL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::INTERNAL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::CALL_MANAGER_DEFINED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::CALL_MANAGER_DEFINED)) {
        os += (first ? "" : " | ");
        os += "CALL_MANAGER_DEFINED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::CALL_MANAGER_DEFINED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::TYPE_3GPP_SPEC_DEFINED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::TYPE_3GPP_SPEC_DEFINED)) {
        os += (first ? "" : " | ");
        os += "TYPE_3GPP_SPEC_DEFINED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::TYPE_3GPP_SPEC_DEFINED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::PPP) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::PPP)) {
        os += (first ? "" : " | ");
        os += "PPP";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::PPP;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::EHRPD) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::EHRPD)) {
        os += (first ? "" : " | ");
        os += "EHRPD";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::EHRPD;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::IPV6) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::IPV6)) {
        os += (first ? "" : " | ");
        os += "IPV6";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::IPV6;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::IWLAN) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::IWLAN)) {
        os += (first ? "" : " | ");
        os += "IWLAN";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::IWLAN;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::HANDOFF) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::HANDOFF)) {
        os += (first ? "" : " | ");
        os += "HANDOFF";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::HANDOFF;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::INVALID)) {
        os += (first ? "" : " | ");
        os += "INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::UNSPECIFIED) {
        return "UNSPECIFIED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::MOBILE_IP) {
        return "MOBILE_IP";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::INTERNAL) {
        return "INTERNAL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::CALL_MANAGER_DEFINED) {
        return "CALL_MANAGER_DEFINED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::TYPE_3GPP_SPEC_DEFINED) {
        return "TYPE_3GPP_SPEC_DEFINED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::PPP) {
        return "PPP";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::EHRPD) {
        return "EHRPD";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::IPV6) {
        return "IPV6";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::IWLAN) {
        return "IWLAN";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::HANDOFF) {
        return "HANDOFF";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::INVALID) {
        return "INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::RttMode lhs, const ::vendor::qti::hardware::radio::ims::V1_0::RttMode rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::RttMode rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::RttMode lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::RttMode lhs, const ::vendor::qti::hardware::radio::ims::V1_0::RttMode rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::RttMode rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::RttMode lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::RttMode e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::RttMode e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::RttMode>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::RttMode> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RttMode::RTT_MODE_DISABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RttMode::RTT_MODE_DISABLED)) {
        os += (first ? "" : " | ");
        os += "RTT_MODE_DISABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RttMode::RTT_MODE_DISABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RttMode::RTT_MODE_FULL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RttMode::RTT_MODE_FULL)) {
        os += (first ? "" : " | ");
        os += "RTT_MODE_FULL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RttMode::RTT_MODE_FULL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::RttMode::RTT_MODE_INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::RttMode::RTT_MODE_INVALID)) {
        os += (first ? "" : " | ");
        os += "RTT_MODE_INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::RttMode::RTT_MODE_INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::RttMode o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RttMode::RTT_MODE_DISABLED) {
        return "RTT_MODE_DISABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RttMode::RTT_MODE_FULL) {
        return "RTT_MODE_FULL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::RttMode::RTT_MODE_INVALID) {
        return "RTT_MODE_INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::RegistrationInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".state = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.state);
    os += ", .errorCode = ";
    os += ::android::hardware::toString(o.errorCode);
    os += ", .errorMessage = ";
    os += ::android::hardware::toString(o.errorMessage);
    os += ", .radioTech = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.radioTech);
    os += ", .pAssociatedUris = ";
    os += ::android::hardware::toString(o.pAssociatedUris);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::RegistrationInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::RegistrationInfo& rhs) {
    if (lhs.state != rhs.state) {
        return false;
    }
    if (lhs.errorCode != rhs.errorCode) {
        return false;
    }
    if (lhs.errorMessage != rhs.errorMessage) {
        return false;
    }
    if (lhs.radioTech != rhs.radioTech) {
        return false;
    }
    if (lhs.pAssociatedUris != rhs.pAssociatedUris) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::RegistrationInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::RegistrationInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::StatusForAccessTech& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".networkMode = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.networkMode);
    os += ", .status = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.status);
    os += ", .restrictCause = ";
    os += ::android::hardware::toString(o.restrictCause);
    os += ", .hasRegistration = ";
    os += ::android::hardware::toString(o.hasRegistration);
    os += ", .registration = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.registration);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::StatusForAccessTech& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::StatusForAccessTech& rhs) {
    if (lhs.networkMode != rhs.networkMode) {
        return false;
    }
    if (lhs.status != rhs.status) {
        return false;
    }
    if (lhs.restrictCause != rhs.restrictCause) {
        return false;
    }
    if (lhs.hasRegistration != rhs.hasRegistration) {
        return false;
    }
    if (lhs.registration != rhs.registration) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::StatusForAccessTech& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::StatusForAccessTech& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::ServiceStatusInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".hasIsValid = ";
    os += ::android::hardware::toString(o.hasIsValid);
    os += ", .isValid = ";
    os += ::android::hardware::toString(o.isValid);
    os += ", .type = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.type);
    os += ", .callType = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.callType);
    os += ", .status = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.status);
    os += ", .userdata = ";
    os += ::android::hardware::toString(o.userdata);
    os += ", .restrictCause = ";
    os += ::android::hardware::toString(o.restrictCause);
    os += ", .accTechStatus = ";
    os += ::android::hardware::toString(o.accTechStatus);
    os += ", .rttMode = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.rttMode);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::ServiceStatusInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ServiceStatusInfo& rhs) {
    if (lhs.hasIsValid != rhs.hasIsValid) {
        return false;
    }
    if (lhs.isValid != rhs.isValid) {
        return false;
    }
    if (lhs.type != rhs.type) {
        return false;
    }
    if (lhs.callType != rhs.callType) {
        return false;
    }
    if (lhs.status != rhs.status) {
        return false;
    }
    if (lhs.userdata != rhs.userdata) {
        return false;
    }
    if (lhs.restrictCause != rhs.restrictCause) {
        return false;
    }
    if (lhs.accTechStatus != rhs.accTechStatus) {
        return false;
    }
    if (lhs.rttMode != rhs.rttMode) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::ServiceStatusInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::ServiceStatusInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::CallDetails& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".callType = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.callType);
    os += ", .callDomain = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.callDomain);
    os += ", .extrasLength = ";
    os += ::android::hardware::toString(o.extrasLength);
    os += ", .extras = ";
    os += ::android::hardware::toString(o.extras);
    os += ", .localAbility = ";
    os += ::android::hardware::toString(o.localAbility);
    os += ", .peerAbility = ";
    os += ::android::hardware::toString(o.peerAbility);
    os += ", .callSubstate = ";
    os += ::android::hardware::toString(o.callSubstate);
    os += ", .mediaId = ";
    os += ::android::hardware::toString(o.mediaId);
    os += ", .causeCode = ";
    os += ::android::hardware::toString(o.causeCode);
    os += ", .rttMode = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.rttMode);
    os += ", .sipAlternateUri = ";
    os += ::android::hardware::toString(o.sipAlternateUri);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::CallDetails& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::CallDetails& rhs) {
    if (lhs.callType != rhs.callType) {
        return false;
    }
    if (lhs.callDomain != rhs.callDomain) {
        return false;
    }
    if (lhs.extrasLength != rhs.extrasLength) {
        return false;
    }
    if (lhs.extras != rhs.extras) {
        return false;
    }
    if (lhs.localAbility != rhs.localAbility) {
        return false;
    }
    if (lhs.peerAbility != rhs.peerAbility) {
        return false;
    }
    if (lhs.callSubstate != rhs.callSubstate) {
        return false;
    }
    if (lhs.mediaId != rhs.mediaId) {
        return false;
    }
    if (lhs.causeCode != rhs.causeCode) {
        return false;
    }
    if (lhs.rttMode != rhs.rttMode) {
        return false;
    }
    if (lhs.sipAlternateUri != rhs.sipAlternateUri) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::CallDetails& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::CallDetails& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::DialRequest& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".address = ";
    os += ::android::hardware::toString(o.address);
    os += ", .clirMode = ";
    os += ::android::hardware::toString(o.clirMode);
    os += ", .presentation = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.presentation);
    os += ", .hasCallDetails = ";
    os += ::android::hardware::toString(o.hasCallDetails);
    os += ", .callDetails = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.callDetails);
    os += ", .hasIsConferenceUri = ";
    os += ::android::hardware::toString(o.hasIsConferenceUri);
    os += ", .isConferenceUri = ";
    os += ::android::hardware::toString(o.isConferenceUri);
    os += ", .hasIsCallPull = ";
    os += ::android::hardware::toString(o.hasIsCallPull);
    os += ", .isCallPull = ";
    os += ::android::hardware::toString(o.isCallPull);
    os += ", .hasIsEncrypted = ";
    os += ::android::hardware::toString(o.hasIsEncrypted);
    os += ", .isEncrypted = ";
    os += ::android::hardware::toString(o.isEncrypted);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::DialRequest& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::DialRequest& rhs) {
    if (lhs.address != rhs.address) {
        return false;
    }
    if (lhs.clirMode != rhs.clirMode) {
        return false;
    }
    if (lhs.presentation != rhs.presentation) {
        return false;
    }
    if (lhs.hasCallDetails != rhs.hasCallDetails) {
        return false;
    }
    if (lhs.callDetails != rhs.callDetails) {
        return false;
    }
    if (lhs.hasIsConferenceUri != rhs.hasIsConferenceUri) {
        return false;
    }
    if (lhs.isConferenceUri != rhs.isConferenceUri) {
        return false;
    }
    if (lhs.hasIsCallPull != rhs.hasIsCallPull) {
        return false;
    }
    if (lhs.isCallPull != rhs.isCallPull) {
        return false;
    }
    if (lhs.hasIsEncrypted != rhs.hasIsEncrypted) {
        return false;
    }
    if (lhs.isEncrypted != rhs.isEncrypted) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::DialRequest& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::DialRequest& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".errorCode = ";
    os += ::android::hardware::toString(o.errorCode);
    os += ", .errorString = ";
    os += ::android::hardware::toString(o.errorString);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo& rhs) {
    if (lhs.errorCode != rhs.errorCode) {
        return false;
    }
    if (lhs.errorString != rhs.errorString) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::CallFailCauseResponse& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".failCause = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.failCause);
    os += ", .errorinfo = ";
    os += ::android::hardware::toString(o.errorinfo);
    os += ", .networkErrorString = ";
    os += ::android::hardware::toString(o.networkErrorString);
    os += ", .hasErrorDetails = ";
    os += ::android::hardware::toString(o.hasErrorDetails);
    os += ", .errorDetails = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.errorDetails);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::CallFailCauseResponse& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::CallFailCauseResponse& rhs) {
    if (lhs.failCause != rhs.failCause) {
        return false;
    }
    if (lhs.errorinfo != rhs.errorinfo) {
        return false;
    }
    if (lhs.networkErrorString != rhs.networkErrorString) {
        return false;
    }
    if (lhs.hasErrorDetails != rhs.hasErrorDetails) {
        return false;
    }
    if (lhs.errorDetails != rhs.errorDetails) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::CallFailCauseResponse& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::CallFailCauseResponse& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::HangupRequestInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".connIndex = ";
    os += ::android::hardware::toString(o.connIndex);
    os += ", .hasMultiParty = ";
    os += ::android::hardware::toString(o.hasMultiParty);
    os += ", .multiParty = ";
    os += ::android::hardware::toString(o.multiParty);
    os += ", .connUri = ";
    os += ::android::hardware::toString(o.connUri);
    os += ", .conf_id = ";
    os += ::android::hardware::toString(o.conf_id);
    os += ", .hasFailCauseResponse = ";
    os += ::android::hardware::toString(o.hasFailCauseResponse);
    os += ", .failCauseResponse = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.failCauseResponse);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::HangupRequestInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::HangupRequestInfo& rhs) {
    if (lhs.connIndex != rhs.connIndex) {
        return false;
    }
    if (lhs.hasMultiParty != rhs.hasMultiParty) {
        return false;
    }
    if (lhs.multiParty != rhs.multiParty) {
        return false;
    }
    if (lhs.connUri != rhs.connUri) {
        return false;
    }
    if (lhs.conf_id != rhs.conf_id) {
        return false;
    }
    if (lhs.hasFailCauseResponse != rhs.hasFailCauseResponse) {
        return false;
    }
    if (lhs.failCauseResponse != rhs.failCauseResponse) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::HangupRequestInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::HangupRequestInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::ConfigInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".item = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.item);
    os += ", .hasBoolValue = ";
    os += ::android::hardware::toString(o.hasBoolValue);
    os += ", .boolValue = ";
    os += ::android::hardware::toString(o.boolValue);
    os += ", .intValue = ";
    os += ::android::hardware::toString(o.intValue);
    os += ", .stringValue = ";
    os += ::android::hardware::toString(o.stringValue);
    os += ", .errorCause = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.errorCause);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::ConfigInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ConfigInfo& rhs) {
    if (lhs.item != rhs.item) {
        return false;
    }
    if (lhs.hasBoolValue != rhs.hasBoolValue) {
        return false;
    }
    if (lhs.boolValue != rhs.boolValue) {
        return false;
    }
    if (lhs.intValue != rhs.intValue) {
        return false;
    }
    if (lhs.stringValue != rhs.stringValue) {
        return false;
    }
    if (lhs.errorCause != rhs.errorCause) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::ConfigInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::ConfigInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::Extra& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".type = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.type);
    os += ", .extraInfo = ";
    os += ::android::hardware::toString(o.extraInfo);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::Extra& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::Extra& rhs) {
    if (lhs.type != rhs.type) {
        return false;
    }
    if (lhs.extraInfo != rhs.extraInfo) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::Extra& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::Extra& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::HandoverInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".type = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.type);
    os += ", .srcTech = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.srcTech);
    os += ", .targetTech = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.targetTech);
    os += ", .hasHoExtra = ";
    os += ::android::hardware::toString(o.hasHoExtra);
    os += ", .hoExtra = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.hoExtra);
    os += ", .errorCode = ";
    os += ::android::hardware::toString(o.errorCode);
    os += ", .errorMessage = ";
    os += ::android::hardware::toString(o.errorMessage);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::HandoverInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::HandoverInfo& rhs) {
    if (lhs.type != rhs.type) {
        return false;
    }
    if (lhs.srcTech != rhs.srcTech) {
        return false;
    }
    if (lhs.targetTech != rhs.targetTech) {
        return false;
    }
    if (lhs.hasHoExtra != rhs.hasHoExtra) {
        return false;
    }
    if (lhs.hoExtra != rhs.hoExtra) {
        return false;
    }
    if (lhs.errorCode != rhs.errorCode) {
        return false;
    }
    if (lhs.errorMessage != rhs.errorMessage) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::HandoverInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::HandoverInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::CallInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".state = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.state);
    os += ", .index = ";
    os += ::android::hardware::toString(o.index);
    os += ", .toa = ";
    os += ::android::hardware::toString(o.toa);
    os += ", .hasIsMpty = ";
    os += ::android::hardware::toString(o.hasIsMpty);
    os += ", .isMpty = ";
    os += ::android::hardware::toString(o.isMpty);
    os += ", .hasIsMT = ";
    os += ::android::hardware::toString(o.hasIsMT);
    os += ", .isMT = ";
    os += ::android::hardware::toString(o.isMT);
    os += ", .als = ";
    os += ::android::hardware::toString(o.als);
    os += ", .hasIsVoice = ";
    os += ::android::hardware::toString(o.hasIsVoice);
    os += ", .isVoice = ";
    os += ::android::hardware::toString(o.isVoice);
    os += ", .hasIsVoicePrivacy = ";
    os += ::android::hardware::toString(o.hasIsVoicePrivacy);
    os += ", .isVoicePrivacy = ";
    os += ::android::hardware::toString(o.isVoicePrivacy);
    os += ", .number = ";
    os += ::android::hardware::toString(o.number);
    os += ", .numberPresentation = ";
    os += ::android::hardware::toString(o.numberPresentation);
    os += ", .name = ";
    os += ::android::hardware::toString(o.name);
    os += ", .namePresentation = ";
    os += ::android::hardware::toString(o.namePresentation);
    os += ", .hasCallDetails = ";
    os += ::android::hardware::toString(o.hasCallDetails);
    os += ", .callDetails = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.callDetails);
    os += ", .hasFailCause = ";
    os += ::android::hardware::toString(o.hasFailCause);
    os += ", .failCause = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.failCause);
    os += ", .hasIsEncrypted = ";
    os += ::android::hardware::toString(o.hasIsEncrypted);
    os += ", .isEncrypted = ";
    os += ::android::hardware::toString(o.isEncrypted);
    os += ", .hasIsCalledPartyRinging = ";
    os += ::android::hardware::toString(o.hasIsCalledPartyRinging);
    os += ", .isCalledPartyRinging = ";
    os += ::android::hardware::toString(o.isCalledPartyRinging);
    os += ", .historyInfo = ";
    os += ::android::hardware::toString(o.historyInfo);
    os += ", .hasIsVideoConfSupported = ";
    os += ::android::hardware::toString(o.hasIsVideoConfSupported);
    os += ", .isVideoConfSupported = ";
    os += ::android::hardware::toString(o.isVideoConfSupported);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::CallInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::CallInfo& rhs) {
    if (lhs.state != rhs.state) {
        return false;
    }
    if (lhs.index != rhs.index) {
        return false;
    }
    if (lhs.toa != rhs.toa) {
        return false;
    }
    if (lhs.hasIsMpty != rhs.hasIsMpty) {
        return false;
    }
    if (lhs.isMpty != rhs.isMpty) {
        return false;
    }
    if (lhs.hasIsMT != rhs.hasIsMT) {
        return false;
    }
    if (lhs.isMT != rhs.isMT) {
        return false;
    }
    if (lhs.als != rhs.als) {
        return false;
    }
    if (lhs.hasIsVoice != rhs.hasIsVoice) {
        return false;
    }
    if (lhs.isVoice != rhs.isVoice) {
        return false;
    }
    if (lhs.hasIsVoicePrivacy != rhs.hasIsVoicePrivacy) {
        return false;
    }
    if (lhs.isVoicePrivacy != rhs.isVoicePrivacy) {
        return false;
    }
    if (lhs.number != rhs.number) {
        return false;
    }
    if (lhs.numberPresentation != rhs.numberPresentation) {
        return false;
    }
    if (lhs.name != rhs.name) {
        return false;
    }
    if (lhs.namePresentation != rhs.namePresentation) {
        return false;
    }
    if (lhs.hasCallDetails != rhs.hasCallDetails) {
        return false;
    }
    if (lhs.callDetails != rhs.callDetails) {
        return false;
    }
    if (lhs.hasFailCause != rhs.hasFailCause) {
        return false;
    }
    if (lhs.failCause != rhs.failCause) {
        return false;
    }
    if (lhs.hasIsEncrypted != rhs.hasIsEncrypted) {
        return false;
    }
    if (lhs.isEncrypted != rhs.isEncrypted) {
        return false;
    }
    if (lhs.hasIsCalledPartyRinging != rhs.hasIsCalledPartyRinging) {
        return false;
    }
    if (lhs.isCalledPartyRinging != rhs.isCalledPartyRinging) {
        return false;
    }
    if (lhs.historyInfo != rhs.historyInfo) {
        return false;
    }
    if (lhs.hasIsVideoConfSupported != rhs.hasIsVideoConfSupported) {
        return false;
    }
    if (lhs.isVideoConfSupported != rhs.isVideoConfSupported) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::CallInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::CallInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::CbNumInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".status = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.status);
    os += ", .number = ";
    os += ::android::hardware::toString(o.number);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::CbNumInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::CbNumInfo& rhs) {
    if (lhs.status != rhs.status) {
        return false;
    }
    if (lhs.number != rhs.number) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::CbNumInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::CbNumInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::CbNumListInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".serviceClass = ";
    os += ::android::hardware::toString(o.serviceClass);
    os += ", .cbNumInfo = ";
    os += ::android::hardware::toString(o.cbNumInfo);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::CbNumListInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::CbNumListInfo& rhs) {
    if (lhs.serviceClass != rhs.serviceClass) {
        return false;
    }
    if (lhs.cbNumInfo != rhs.cbNumInfo) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::CbNumListInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::CbNumListInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::SuppServiceStatus& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".status = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.status);
    os += ", .facilityType = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.facilityType);
    os += ", .failureCause = ";
    os += ::android::hardware::toString(o.failureCause);
    os += ", .cbNumListInfo = ";
    os += ::android::hardware::toString(o.cbNumListInfo);
    os += ", .hasErrorDetails = ";
    os += ::android::hardware::toString(o.hasErrorDetails);
    os += ", .errorDetails = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.errorDetails);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::SuppServiceStatus& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::SuppServiceStatus& rhs) {
    if (lhs.status != rhs.status) {
        return false;
    }
    if (lhs.facilityType != rhs.facilityType) {
        return false;
    }
    if (lhs.failureCause != rhs.failureCause) {
        return false;
    }
    if (lhs.cbNumListInfo != rhs.cbNumListInfo) {
        return false;
    }
    if (lhs.hasErrorDetails != rhs.hasErrorDetails) {
        return false;
    }
    if (lhs.errorDetails != rhs.errorDetails) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::SuppServiceStatus& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::SuppServiceStatus& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::ClipProvisionStatus& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".clipStatus = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.clipStatus);
    os += ", .hasErrorDetails = ";
    os += ::android::hardware::toString(o.hasErrorDetails);
    os += ", .errorDetails = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.errorDetails);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::ClipProvisionStatus& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ClipProvisionStatus& rhs) {
    if (lhs.clipStatus != rhs.clipStatus) {
        return false;
    }
    if (lhs.hasErrorDetails != rhs.hasErrorDetails) {
        return false;
    }
    if (lhs.errorDetails != rhs.errorDetails) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::ClipProvisionStatus& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::ClipProvisionStatus& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::ClirInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".paramM = ";
    os += ::android::hardware::toString(o.paramM);
    os += ", .paramN = ";
    os += ::android::hardware::toString(o.paramN);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::ClirInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ClirInfo& rhs) {
    if (lhs.paramM != rhs.paramM) {
        return false;
    }
    if (lhs.paramN != rhs.paramN) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::ClirInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::ClirInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::ColrInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".presentation = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.presentation);
    os += ", .hasErrorDetails = ";
    os += ::android::hardware::toString(o.hasErrorDetails);
    os += ", .errorDetails = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.errorDetails);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::ColrInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ColrInfo& rhs) {
    if (lhs.presentation != rhs.presentation) {
        return false;
    }
    if (lhs.hasErrorDetails != rhs.hasErrorDetails) {
        return false;
    }
    if (lhs.errorDetails != rhs.errorDetails) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::ColrInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::ColrInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::DtmfInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".dtmf = ";
    os += ::android::hardware::toString(o.dtmf);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::DtmfInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::DtmfInfo& rhs) {
    if (lhs.dtmf != rhs.dtmf) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::DtmfInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::DtmfInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::TtyInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".mode = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.mode);
    os += ", .userData = ";
    os += ::android::hardware::toString(o.userData);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::TtyInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::TtyInfo& rhs) {
    if (lhs.mode != rhs.mode) {
        return false;
    }
    if (lhs.userData != rhs.userData) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::TtyInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::TtyInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::ConferenceInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".confInfoUri = ";
    os += ::android::hardware::toString(o.confInfoUri);
    os += ", .conferenceCallState = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.conferenceCallState);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::ConferenceInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ConferenceInfo& rhs) {
    if (lhs.confInfoUri != rhs.confInfoUri) {
        return false;
    }
    if (lhs.conferenceCallState != rhs.conferenceCallState) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::ConferenceInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::ConferenceInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::ViceInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".viceInfoUri = ";
    os += ::android::hardware::toString(o.viceInfoUri);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::ViceInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ViceInfo& rhs) {
    if (lhs.viceInfoUri != rhs.viceInfoUri) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::ViceInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::ViceInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".callIndex = ";
    os += ::android::hardware::toString(o.callIndex);
    os += ", .hasCallDetails = ";
    os += ::android::hardware::toString(o.hasCallDetails);
    os += ", .callDetails = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.callDetails);
    os += ", .failCause = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.failCause);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyInfo& rhs) {
    if (lhs.callIndex != rhs.callIndex) {
        return false;
    }
    if (lhs.hasCallDetails != rhs.hasCallDetails) {
        return false;
    }
    if (lhs.callDetails != rhs.callDetails) {
        return false;
    }
    if (lhs.failCause != rhs.failCause) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::CallFwdTimerInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".year = ";
    os += ::android::hardware::toString(o.year);
    os += ", .month = ";
    os += ::android::hardware::toString(o.month);
    os += ", .day = ";
    os += ::android::hardware::toString(o.day);
    os += ", .hour = ";
    os += ::android::hardware::toString(o.hour);
    os += ", .minute = ";
    os += ::android::hardware::toString(o.minute);
    os += ", .second = ";
    os += ::android::hardware::toString(o.second);
    os += ", .timezone = ";
    os += ::android::hardware::toString(o.timezone);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::CallFwdTimerInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::CallFwdTimerInfo& rhs) {
    if (lhs.year != rhs.year) {
        return false;
    }
    if (lhs.month != rhs.month) {
        return false;
    }
    if (lhs.day != rhs.day) {
        return false;
    }
    if (lhs.hour != rhs.hour) {
        return false;
    }
    if (lhs.minute != rhs.minute) {
        return false;
    }
    if (lhs.second != rhs.second) {
        return false;
    }
    if (lhs.timezone != rhs.timezone) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::CallFwdTimerInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::CallFwdTimerInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".status = ";
    os += ::android::hardware::toString(o.status);
    os += ", .reason = ";
    os += ::android::hardware::toString(o.reason);
    os += ", .serviceClass = ";
    os += ::android::hardware::toString(o.serviceClass);
    os += ", .toa = ";
    os += ::android::hardware::toString(o.toa);
    os += ", .number = ";
    os += ::android::hardware::toString(o.number);
    os += ", .timeSeconds = ";
    os += ::android::hardware::toString(o.timeSeconds);
    os += ", .hasCallFwdTimerStart = ";
    os += ::android::hardware::toString(o.hasCallFwdTimerStart);
    os += ", .callFwdTimerStart = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.callFwdTimerStart);
    os += ", .hasCallFwdTimerEnd = ";
    os += ::android::hardware::toString(o.hasCallFwdTimerEnd);
    os += ", .callFwdTimerEnd = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.callFwdTimerEnd);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo& rhs) {
    if (lhs.status != rhs.status) {
        return false;
    }
    if (lhs.reason != rhs.reason) {
        return false;
    }
    if (lhs.serviceClass != rhs.serviceClass) {
        return false;
    }
    if (lhs.toa != rhs.toa) {
        return false;
    }
    if (lhs.number != rhs.number) {
        return false;
    }
    if (lhs.timeSeconds != rhs.timeSeconds) {
        return false;
    }
    if (lhs.hasCallFwdTimerStart != rhs.hasCallFwdTimerStart) {
        return false;
    }
    if (lhs.callFwdTimerStart != rhs.callFwdTimerStart) {
        return false;
    }
    if (lhs.hasCallFwdTimerEnd != rhs.hasCallFwdTimerEnd) {
        return false;
    }
    if (lhs.callFwdTimerEnd != rhs.callFwdTimerEnd) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::SuppServiceNotification& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".notificationType = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.notificationType);
    os += ", .code = ";
    os += ::android::hardware::toString(o.code);
    os += ", .index = ";
    os += ::android::hardware::toString(o.index);
    os += ", .type = ";
    os += ::android::hardware::toString(o.type);
    os += ", .number = ";
    os += ::android::hardware::toString(o.number);
    os += ", .connId = ";
    os += ::android::hardware::toString(o.connId);
    os += ", .historyInfo = ";
    os += ::android::hardware::toString(o.historyInfo);
    os += ", .hasHoldTone = ";
    os += ::android::hardware::toString(o.hasHoldTone);
    os += ", .holdTone = ";
    os += ::android::hardware::toString(o.holdTone);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::SuppServiceNotification& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::SuppServiceNotification& rhs) {
    if (lhs.notificationType != rhs.notificationType) {
        return false;
    }
    if (lhs.code != rhs.code) {
        return false;
    }
    if (lhs.index != rhs.index) {
        return false;
    }
    if (lhs.type != rhs.type) {
        return false;
    }
    if (lhs.number != rhs.number) {
        return false;
    }
    if (lhs.connId != rhs.connId) {
        return false;
    }
    if (lhs.historyInfo != rhs.historyInfo) {
        return false;
    }
    if (lhs.hasHoldTone != rhs.hasHoldTone) {
        return false;
    }
    if (lhs.holdTone != rhs.holdTone) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::SuppServiceNotification& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::SuppServiceNotification& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::ExplicitCallTransferInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".callId = ";
    os += ::android::hardware::toString(o.callId);
    os += ", .ectType = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.ectType);
    os += ", .targetAddress = ";
    os += ::android::hardware::toString(o.targetAddress);
    os += ", .targetCallId = ";
    os += ::android::hardware::toString(o.targetCallId);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::ExplicitCallTransferInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ExplicitCallTransferInfo& rhs) {
    if (lhs.callId != rhs.callId) {
        return false;
    }
    if (lhs.ectType != rhs.ectType) {
        return false;
    }
    if (lhs.targetAddress != rhs.targetAddress) {
        return false;
    }
    if (lhs.targetCallId != rhs.targetCallId) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::ExplicitCallTransferInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::ExplicitCallTransferInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::MessageSummary& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".type = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.type);
    os += ", .newMessageCount = ";
    os += ::android::hardware::toString(o.newMessageCount);
    os += ", .oldMessageCount = ";
    os += ::android::hardware::toString(o.oldMessageCount);
    os += ", .newUrgentMessageCount = ";
    os += ::android::hardware::toString(o.newUrgentMessageCount);
    os += ", .oldUrgentMessageCount = ";
    os += ::android::hardware::toString(o.oldUrgentMessageCount);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::MessageSummary& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::MessageSummary& rhs) {
    if (lhs.type != rhs.type) {
        return false;
    }
    if (lhs.newMessageCount != rhs.newMessageCount) {
        return false;
    }
    if (lhs.oldMessageCount != rhs.oldMessageCount) {
        return false;
    }
    if (lhs.newUrgentMessageCount != rhs.newUrgentMessageCount) {
        return false;
    }
    if (lhs.oldUrgentMessageCount != rhs.oldUrgentMessageCount) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::MessageSummary& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::MessageSummary& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::MessageDetails& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".toAddress = ";
    os += ::android::hardware::toString(o.toAddress);
    os += ", .fromAddress = ";
    os += ::android::hardware::toString(o.fromAddress);
    os += ", .subject = ";
    os += ::android::hardware::toString(o.subject);
    os += ", .date = ";
    os += ::android::hardware::toString(o.date);
    os += ", .priority = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.priority);
    os += ", .id = ";
    os += ::android::hardware::toString(o.id);
    os += ", .type = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.type);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::MessageDetails& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::MessageDetails& rhs) {
    if (lhs.toAddress != rhs.toAddress) {
        return false;
    }
    if (lhs.fromAddress != rhs.fromAddress) {
        return false;
    }
    if (lhs.subject != rhs.subject) {
        return false;
    }
    if (lhs.date != rhs.date) {
        return false;
    }
    if (lhs.priority != rhs.priority) {
        return false;
    }
    if (lhs.id != rhs.id) {
        return false;
    }
    if (lhs.type != rhs.type) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::MessageDetails& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::MessageDetails& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::MessageWaitingIndication& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".messageSummary = ";
    os += ::android::hardware::toString(o.messageSummary);
    os += ", .ueAddress = ";
    os += ::android::hardware::toString(o.ueAddress);
    os += ", .messageDetails = ";
    os += ::android::hardware::toString(o.messageDetails);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::MessageWaitingIndication& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::MessageWaitingIndication& rhs) {
    if (lhs.messageSummary != rhs.messageSummary) {
        return false;
    }
    if (lhs.ueAddress != rhs.ueAddress) {
        return false;
    }
    if (lhs.messageDetails != rhs.messageDetails) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::MessageWaitingIndication& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::MessageWaitingIndication& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::DeflectRequestInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".connIndex = ";
    os += ::android::hardware::toString(o.connIndex);
    os += ", .number = ";
    os += ::android::hardware::toString(o.number);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::DeflectRequestInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::DeflectRequestInfo& rhs) {
    if (lhs.connIndex != rhs.connIndex) {
        return false;
    }
    if (lhs.number != rhs.number) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::DeflectRequestInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::DeflectRequestInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::AddressInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".city = ";
    os += ::android::hardware::toString(o.city);
    os += ", .state = ";
    os += ::android::hardware::toString(o.state);
    os += ", .country = ";
    os += ::android::hardware::toString(o.country);
    os += ", .postalCode = ";
    os += ::android::hardware::toString(o.postalCode);
    os += ", .countryCode = ";
    os += ::android::hardware::toString(o.countryCode);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::AddressInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::AddressInfo& rhs) {
    if (lhs.city != rhs.city) {
        return false;
    }
    if (lhs.state != rhs.state) {
        return false;
    }
    if (lhs.country != rhs.country) {
        return false;
    }
    if (lhs.postalCode != rhs.postalCode) {
        return false;
    }
    if (lhs.countryCode != rhs.countryCode) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::AddressInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::AddressInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::ImsSubConfigInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".simultStackCount = ";
    os += ::android::hardware::toString(o.simultStackCount);
    os += ", .imsStackEnabled = ";
    os += ::android::hardware::toString(o.imsStackEnabled);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::ImsSubConfigInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ImsSubConfigInfo& rhs) {
    if (lhs.simultStackCount != rhs.simultStackCount) {
        return false;
    }
    if (lhs.imsStackEnabled != rhs.imsStackEnabled) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::ImsSubConfigInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::ImsSubConfigInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::ParticipantStatusInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".callId = ";
    os += ::android::hardware::toString(o.callId);
    os += ", .operation = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.operation);
    os += ", .sipStatus = ";
    os += ::android::hardware::toString(o.sipStatus);
    os += ", .participantUri = ";
    os += ::android::hardware::toString(o.participantUri);
    os += ", .hasIsEct = ";
    os += ::android::hardware::toString(o.hasIsEct);
    os += ", .isEct = ";
    os += ::android::hardware::toString(o.isEct);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::ParticipantStatusInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::ParticipantStatusInfo& rhs) {
    if (lhs.callId != rhs.callId) {
        return false;
    }
    if (lhs.operation != rhs.operation) {
        return false;
    }
    if (lhs.sipStatus != rhs.sipStatus) {
        return false;
    }
    if (lhs.participantUri != rhs.participantUri) {
        return false;
    }
    if (lhs.hasIsEct != rhs.hasIsEct) {
        return false;
    }
    if (lhs.isEct != rhs.isEct) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::ParticipantStatusInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::ParticipantStatusInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonDetails& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".regFailureReasonType = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.regFailureReasonType);
    os += ", .regFailureReason = ";
    os += ::android::hardware::toString(o.regFailureReason);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonDetails& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonDetails& rhs) {
    if (lhs.regFailureReasonType != rhs.regFailureReasonType) {
        return false;
    }
    if (lhs.regFailureReason != rhs.regFailureReason) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonDetails& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonDetails& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::BlockStatus& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".blockReason = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.blockReason);
    os += ", .hasBlockReasonDetails = ";
    os += ::android::hardware::toString(o.hasBlockReasonDetails);
    os += ", .blockReasonDetails = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.blockReasonDetails);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::BlockStatus& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::BlockStatus& rhs) {
    if (lhs.blockReason != rhs.blockReason) {
        return false;
    }
    if (lhs.hasBlockReasonDetails != rhs.hasBlockReasonDetails) {
        return false;
    }
    if (lhs.blockReasonDetails != rhs.blockReasonDetails) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::BlockStatus& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::BlockStatus& rhs){
    return !(lhs == rhs);
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo lhs, const ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo lhs, const ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_NONE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_NONE)) {
        os += (first ? "" : " | ");
        os += "VOWIFI_CALL_QUALITY_NONE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_NONE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_EXCELLENT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_EXCELLENT)) {
        os += (first ? "" : " | ");
        os += "VOWIFI_CALL_QUALITY_EXCELLENT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_EXCELLENT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_FAIR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_FAIR)) {
        os += (first ? "" : " | ");
        os += "VOWIFI_CALL_QUALITY_FAIR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_FAIR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_BAD) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_BAD)) {
        os += (first ? "" : " | ");
        os += "VOWIFI_CALL_QUALITY_BAD";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_BAD;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_INVALID)) {
        os += (first ? "" : " | ");
        os += "VOWIFI_CALL_QUALITY_INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_NONE) {
        return "VOWIFI_CALL_QUALITY_NONE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_EXCELLENT) {
        return "VOWIFI_CALL_QUALITY_EXCELLENT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_FAIR) {
        return "VOWIFI_CALL_QUALITY_FAIR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_BAD) {
        return "VOWIFI_CALL_QUALITY_BAD";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_INVALID) {
        return "VOWIFI_CALL_QUALITY_INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::SsServiceType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::SsServiceType> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CFU) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CFU)) {
        os += (first ? "" : " | ");
        os += "SS_CFU";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CFU;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CF_BUSY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CF_BUSY)) {
        os += (first ? "" : " | ");
        os += "SS_CF_BUSY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CF_BUSY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CF_NO_REPLY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CF_NO_REPLY)) {
        os += (first ? "" : " | ");
        os += "SS_CF_NO_REPLY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CF_NO_REPLY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CF_NOT_REACHABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CF_NOT_REACHABLE)) {
        os += (first ? "" : " | ");
        os += "SS_CF_NOT_REACHABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CF_NOT_REACHABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CF_ALL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CF_ALL)) {
        os += (first ? "" : " | ");
        os += "SS_CF_ALL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CF_ALL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CF_ALL_CONDITIONAL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CF_ALL_CONDITIONAL)) {
        os += (first ? "" : " | ");
        os += "SS_CF_ALL_CONDITIONAL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CF_ALL_CONDITIONAL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CFUT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CFUT)) {
        os += (first ? "" : " | ");
        os += "SS_CFUT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CFUT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CLIP) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CLIP)) {
        os += (first ? "" : " | ");
        os += "SS_CLIP";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CLIP;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CLIR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CLIR)) {
        os += (first ? "" : " | ");
        os += "SS_CLIR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CLIR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_COLP) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_COLP)) {
        os += (first ? "" : " | ");
        os += "SS_COLP";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_COLP;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_COLR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_COLR)) {
        os += (first ? "" : " | ");
        os += "SS_COLR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_COLR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CNAP) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CNAP)) {
        os += (first ? "" : " | ");
        os += "SS_CNAP";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CNAP;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_WAIT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_WAIT)) {
        os += (first ? "" : " | ");
        os += "SS_WAIT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_WAIT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_BAOC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_BAOC)) {
        os += (first ? "" : " | ");
        os += "SS_BAOC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_BAOC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_BAOIC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_BAOIC)) {
        os += (first ? "" : " | ");
        os += "SS_BAOIC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_BAOIC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_BAOIC_EXC_HOME) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_BAOIC_EXC_HOME)) {
        os += (first ? "" : " | ");
        os += "SS_BAOIC_EXC_HOME";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_BAOIC_EXC_HOME;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_BAIC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_BAIC)) {
        os += (first ? "" : " | ");
        os += "SS_BAIC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_BAIC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_BAIC_ROAMING) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_BAIC_ROAMING)) {
        os += (first ? "" : " | ");
        os += "SS_BAIC_ROAMING";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_BAIC_ROAMING;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_ALL_BARRING) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_ALL_BARRING)) {
        os += (first ? "" : " | ");
        os += "SS_ALL_BARRING";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_ALL_BARRING;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_OUTGOING_BARRING) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_OUTGOING_BARRING)) {
        os += (first ? "" : " | ");
        os += "SS_OUTGOING_BARRING";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_OUTGOING_BARRING;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_INCOMING_BARRING) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_INCOMING_BARRING)) {
        os += (first ? "" : " | ");
        os += "SS_INCOMING_BARRING";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_INCOMING_BARRING;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_INCOMING_BARRING_DN) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_INCOMING_BARRING_DN)) {
        os += (first ? "" : " | ");
        os += "SS_INCOMING_BARRING_DN";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_INCOMING_BARRING_DN;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_INCOMING_BARRING_ANONYMOUS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_INCOMING_BARRING_ANONYMOUS)) {
        os += (first ? "" : " | ");
        os += "SS_INCOMING_BARRING_ANONYMOUS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_INCOMING_BARRING_ANONYMOUS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_INVALID)) {
        os += (first ? "" : " | ");
        os += "SS_INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::SsServiceType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CFU) {
        return "SS_CFU";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CF_BUSY) {
        return "SS_CF_BUSY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CF_NO_REPLY) {
        return "SS_CF_NO_REPLY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CF_NOT_REACHABLE) {
        return "SS_CF_NOT_REACHABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CF_ALL) {
        return "SS_CF_ALL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CF_ALL_CONDITIONAL) {
        return "SS_CF_ALL_CONDITIONAL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CFUT) {
        return "SS_CFUT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CLIP) {
        return "SS_CLIP";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CLIR) {
        return "SS_CLIR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_COLP) {
        return "SS_COLP";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_COLR) {
        return "SS_COLR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CNAP) {
        return "SS_CNAP";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_WAIT) {
        return "SS_WAIT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_BAOC) {
        return "SS_BAOC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_BAOIC) {
        return "SS_BAOIC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_BAOIC_EXC_HOME) {
        return "SS_BAOIC_EXC_HOME";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_BAIC) {
        return "SS_BAIC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_BAIC_ROAMING) {
        return "SS_BAIC_ROAMING";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_ALL_BARRING) {
        return "SS_ALL_BARRING";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_OUTGOING_BARRING) {
        return "SS_OUTGOING_BARRING";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_INCOMING_BARRING) {
        return "SS_INCOMING_BARRING";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_INCOMING_BARRING_DN) {
        return "SS_INCOMING_BARRING_DN";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_INCOMING_BARRING_ANONYMOUS) {
        return "SS_INCOMING_BARRING_ANONYMOUS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_INVALID) {
        return "SS_INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::SsRequestType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::SsRequestType> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_ACTIVATION) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_ACTIVATION)) {
        os += (first ? "" : " | ");
        os += "SS_ACTIVATION";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_ACTIVATION;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_DEACTIVATION) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_DEACTIVATION)) {
        os += (first ? "" : " | ");
        os += "SS_DEACTIVATION";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_DEACTIVATION;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_INTERROGATION) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_INTERROGATION)) {
        os += (first ? "" : " | ");
        os += "SS_INTERROGATION";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_INTERROGATION;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_REGISTRATION) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_REGISTRATION)) {
        os += (first ? "" : " | ");
        os += "SS_REGISTRATION";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_REGISTRATION;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_ERASURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_ERASURE)) {
        os += (first ? "" : " | ");
        os += "SS_ERASURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_ERASURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_INVALID)) {
        os += (first ? "" : " | ");
        os += "SS_INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::SsRequestType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_ACTIVATION) {
        return "SS_ACTIVATION";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_DEACTIVATION) {
        return "SS_DEACTIVATION";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_INTERROGATION) {
        return "SS_INTERROGATION";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_REGISTRATION) {
        return "SS_REGISTRATION";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_ERASURE) {
        return "SS_ERASURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_INVALID) {
        return "SS_INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType lhs, const ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_ALL_TELE_AND_BEARER_SERVICES) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_ALL_TELE_AND_BEARER_SERVICES)) {
        os += (first ? "" : " | ");
        os += "SS_ALL_TELE_AND_BEARER_SERVICES";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_ALL_TELE_AND_BEARER_SERVICES;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_ALL_TELESEVICES) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_ALL_TELESEVICES)) {
        os += (first ? "" : " | ");
        os += "SS_ALL_TELESEVICES";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_ALL_TELESEVICES;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_TELEPHONY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_TELEPHONY)) {
        os += (first ? "" : " | ");
        os += "SS_TELEPHONY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_TELEPHONY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_ALL_DATA_TELESERVICES) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_ALL_DATA_TELESERVICES)) {
        os += (first ? "" : " | ");
        os += "SS_ALL_DATA_TELESERVICES";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_ALL_DATA_TELESERVICES;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_SMS_SERVICES) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_SMS_SERVICES)) {
        os += (first ? "" : " | ");
        os += "SS_SMS_SERVICES";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_SMS_SERVICES;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_ALL_TELESERVICES_EXCEPT_SMS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_ALL_TELESERVICES_EXCEPT_SMS)) {
        os += (first ? "" : " | ");
        os += "SS_ALL_TELESERVICES_EXCEPT_SMS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_ALL_TELESERVICES_EXCEPT_SMS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_INVALID)) {
        os += (first ? "" : " | ");
        os += "SS_INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_ALL_TELE_AND_BEARER_SERVICES) {
        return "SS_ALL_TELE_AND_BEARER_SERVICES";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_ALL_TELESEVICES) {
        return "SS_ALL_TELESEVICES";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_TELEPHONY) {
        return "SS_TELEPHONY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_ALL_DATA_TELESERVICES) {
        return "SS_ALL_DATA_TELESERVICES";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_SMS_SERVICES) {
        return "SS_SMS_SERVICES";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_ALL_TELESERVICES_EXCEPT_SMS) {
        return "SS_ALL_TELESERVICES_EXCEPT_SMS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_INVALID) {
        return "SS_INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::CfData& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".cfInfo = ";
    os += ::android::hardware::toString(o.cfInfo);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::CfData& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::CfData& rhs) {
    if (lhs.cfInfo != rhs.cfInfo) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::CfData& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::CfData& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::SsInfoData& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".ssInfo = ";
    os += ::android::hardware::toString(o.ssInfo);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::SsInfoData& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::SsInfoData& rhs) {
    if (lhs.ssInfo != rhs.ssInfo) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::SsInfoData& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::SsInfoData& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_0::StkCcUnsolSsResult& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".serviceType = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.serviceType);
    os += ", .requestType = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.requestType);
    os += ", .teleserviceType = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.teleserviceType);
    os += ", .serviceClass = ";
    os += ::android::hardware::toString(o.serviceClass);
    os += ", .result = ";
    os += ::android::hardware::toString(o.result);
    os += ", .ssInfoData = ";
    os += ::android::hardware::toString(o.ssInfoData);
    os += ", .cfData = ";
    os += ::android::hardware::toString(o.cfData);
    os += ", .cbNumInfo = ";
    os += ::android::hardware::toString(o.cbNumInfo);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_0::StkCcUnsolSsResult& lhs, const ::vendor::qti::hardware::radio::ims::V1_0::StkCcUnsolSsResult& rhs) {
    if (lhs.serviceType != rhs.serviceType) {
        return false;
    }
    if (lhs.requestType != rhs.requestType) {
        return false;
    }
    if (lhs.teleserviceType != rhs.teleserviceType) {
        return false;
    }
    if (lhs.serviceClass != rhs.serviceClass) {
        return false;
    }
    if (lhs.result != rhs.result) {
        return false;
    }
    if (lhs.ssInfoData != rhs.ssInfoData) {
        return false;
    }
    if (lhs.cfData != rhs.cfData) {
        return false;
    }
    if (lhs.cbNumInfo != rhs.cbNumInfo) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_0::StkCcUnsolSsResult& lhs,const ::vendor::qti::hardware::radio::ims::V1_0::StkCcUnsolSsResult& rhs){
    return !(lhs == rhs);
}


}  // namespace V1_0
}  // namespace ims
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::IpPresentation>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation* end() { return begin() + 4; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation kVals[4] {
            ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation::IP_PRESENTATION_NUM_ALLOWED,
            ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation::IP_PRESENTATION_NUM_RESTRICTED,
            ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation::IP_PRESENTATION_NUM_DEFAULT,
            ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation::IP_PRESENTATION_INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::CallType>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::CallType* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::CallType* end() { return begin() + 13; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::CallType* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::CallType kVals[13] {
            ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_VOICE,
            ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_VT_TX,
            ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_VT_RX,
            ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_VT,
            ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_VT_NODIR,
            ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_CS_VS_TX,
            ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_CS_VS_RX,
            ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_PS_VS_TX,
            ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_PS_VS_RX,
            ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_UNKNOWN,
            ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_SMS,
            ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_UT,
            ::vendor::qti::hardware::radio::ims::V1_0::CallType::CALL_TYPE_INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::CallDomain>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::CallDomain* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::CallDomain* end() { return begin() + 6; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::CallDomain* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::CallDomain kVals[6] {
            ::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_UNKNOWN,
            ::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_CS,
            ::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_PS,
            ::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_AUTOMATIC,
            ::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_NOT_SET,
            ::vendor::qti::hardware::radio::ims::V1_0::CallDomain::CALL_DOMAIN_INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::CallFailCause>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause* end() { return begin() + 64; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause kVals[64] {
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_UNOBTAINABLE_NUMBER,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_NORMAL,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_BUSY,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_NORMAL_UNSPECIFIED,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_CONGESTION,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_INCOMPATIBILITY_DESTINATION,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_CALL_BARRED,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_FDN_BLOCKED,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_IMEI_NOT_ACCEPTED,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_USSD,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_SS,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_EMERGENCY_TEMP_FAILURE,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_EMERGENCY_PERM_FAILURE,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_HO_NOT_FEASIBLE,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_USER_BUSY,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_USER_REJECT,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_LOW_BATTERY,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_BLACKLISTED_CALL_ID,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_CS_RETRY_REQUIRED,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_NETWORK_UNAVAILABLE,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_FEATURE_UNAVAILABLE,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_ERROR,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_MISC,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_ANSWERED_ELSEWHERE,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_PULL_OUT_OF_SYNC,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_CAUSE_CALL_PULLED,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_ACCESS_CLASS_BLOCKED,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_REDIRECTED,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_BAD_REQUEST,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_FORBIDDEN,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_NOT_FOUND,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_NOT_SUPPORTED,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_REQUEST_TIMEOUT,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_BAD_ADDRESS,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_BUSY,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_REQUEST_CANCELLED,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_NOT_ACCEPTABLE,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_NOT_REACHABLE,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_INTERNAL_ERROR,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_BAD_GATEWAY,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVICE_UNAVAILABLE,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_TIMEOUT,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_USER_REJECTED,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_SIP_GLOBAL_ERROR,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_MEDIA_INIT_FAILED,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_MEDIA_NO_DATA,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_MEDIA_NOT_ACCEPTABLE,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_MEDIA_UNSPECIFIED_ERROR,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_HOLD_RESUME_FAILED,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_HOLD_RESUME_CANCELED,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_HOLD_REINVITE_COLLISION,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_ERROR_UNSPECIFIED,
            ::vendor::qti::hardware::radio::ims::V1_0::CallFailCause::CALL_FAIL_INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::RegState>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::RegState* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::RegState* end() { return begin() + 4; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::RegState* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::RegState kVals[4] {
            ::vendor::qti::hardware::radio::ims::V1_0::RegState::REGISTERED,
            ::vendor::qti::hardware::radio::ims::V1_0::RegState::NOT_REGISTERED,
            ::vendor::qti::hardware::radio::ims::V1_0::RegState::REGISTERING,
            ::vendor::qti::hardware::radio::ims::V1_0::RegState::INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::StatusType>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::StatusType* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::StatusType* end() { return begin() + 5; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::StatusType* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::StatusType kVals[5] {
            ::vendor::qti::hardware::radio::ims::V1_0::StatusType::STATUS_DISABLED,
            ::vendor::qti::hardware::radio::ims::V1_0::StatusType::STATUS_PARTIALLY_ENABLED,
            ::vendor::qti::hardware::radio::ims::V1_0::StatusType::STATUS_ENABLED,
            ::vendor::qti::hardware::radio::ims::V1_0::StatusType::STATUS_NOT_SUPPORTED,
            ::vendor::qti::hardware::radio::ims::V1_0::StatusType::STATUS_INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::RadioTechType>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType* end() { return begin() + 22; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType kVals[22] {
            ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_ANY,
            ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_UNKNOWN,
            ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_GPRS,
            ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_EDGE,
            ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_UMTS,
            ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_IS95A,
            ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_IS95B,
            ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_1xRTT,
            ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_EVDO_0,
            ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_EVDO_A,
            ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_HSDPA,
            ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_HSUPA,
            ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_HSPA,
            ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_EVDO_B,
            ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_EHRPD,
            ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_LTE,
            ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_HSPAP,
            ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_GSM,
            ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_TD_SCDMA,
            ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_WIFI,
            ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_IWLAN,
            ::vendor::qti::hardware::radio::ims::V1_0::RadioTechType::RADIO_TECH_INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::ConfigItem>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem* end() { return begin() + 74; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem kVals[74] {
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_NONE,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOCODER_AMRMODESET,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOCODER_AMRWBMODESET,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_SESSION_TIMER,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_MIN_SESSION_EXPIRY,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_CANCELLATION_TIMER,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_T_DELAY,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SILENT_REDIAL_ENABLE,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_T1_TIMER,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_T2_TIMER,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_TF_TIMER,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VLT_SETTING_ENABLED,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_LVC_SETTING_ENABLED,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_DOMAIN_NAME,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SMS_FORMAT,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SMS_OVER_IP,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_PUBLISH_TIMER,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_PUBLISH_TIMER_EXTENDED,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_GZIP_FLAG,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_EAB_SETTING_ENABLED,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_MOBILE_DATA_ENABLED,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_MODE,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_CAPABILITY_DISCOVERY_ENABLED,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_EMERGENCY_CALL_TIMER,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SSAC_HYSTERESIS_TIMER,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_LBO_PCSCF_ADDRESS,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_KEEP_ALIVE_ENABLED,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_REGISTRATION_RETRY_BASE_TIME_SEC,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_REGISTRATION_RETRY_MAX_TIME_SEC,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SPEECH_START_PORT,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SPEECH_END_PORT,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_INVITE_REQ_RETX_INTERVAL_MSEC,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_WAIT_TIME_MSEC,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_RETX_INTERVAL_MSEC,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_ACK_RECEIPT_WAIT_TIME_MSEC,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_ACK_RETX_WAIT_TIME_MSEC,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AMR_WB_BANDWIDTH_EFFICIENT_PT,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AMR_OCTET_ALIGNED_PT,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AMR_BANDWIDTH_EFFICIENT_PT,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_DTMF_WB_PT,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_DTMF_NB_PT,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_AMR_DEFAULT_MODE,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SMS_PSI,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VIDEO_QUALITY,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE1,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE2,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE3,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_1x,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_WIFI_A,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_THRESHOLD_WIFI_B,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_T_EPDG_LTE,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_T_EPDG_WIFI,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_T_EPDG_1x,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VWF_SETTING_ENABLED,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VCE_SETTING_ENABLED,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_RTT_SETTING_ENABLED,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_SMS_APP,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_VVM_APP,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigItem::CONFIG_ITEM_INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause* end() { return begin() + 7; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause kVals[7] {
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_NO_ERR,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_IMS_NOT_READY,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_FILE_NOT_AVAILABLE,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_READ_FAILED,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_WRITE_FAILED,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_OTHER_INTERNAL_ERR,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause::CONFIG_FAILURE_INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::ToneOperation>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::ToneOperation* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::ToneOperation* end() { return begin() + 3; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::ToneOperation* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::ToneOperation kVals[3] {
            ::vendor::qti::hardware::radio::ims::V1_0::ToneOperation::STOP,
            ::vendor::qti::hardware::radio::ims::V1_0::ToneOperation::START,
            ::vendor::qti::hardware::radio::ims::V1_0::ToneOperation::INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::HandoverType>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::HandoverType* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::HandoverType* end() { return begin() + 7; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::HandoverType* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::HandoverType kVals[7] {
            ::vendor::qti::hardware::radio::ims::V1_0::HandoverType::START,
            ::vendor::qti::hardware::radio::ims::V1_0::HandoverType::COMPLETE_SUCCESS,
            ::vendor::qti::hardware::radio::ims::V1_0::HandoverType::COMPLETE_FAIL,
            ::vendor::qti::hardware::radio::ims::V1_0::HandoverType::CANCEL,
            ::vendor::qti::hardware::radio::ims::V1_0::HandoverType::NOT_TRIGGERED,
            ::vendor::qti::hardware::radio::ims::V1_0::HandoverType::NOT_TRIGGERED_MOBILE_DATA_OFF,
            ::vendor::qti::hardware::radio::ims::V1_0::HandoverType::INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::CallState>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::CallState* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::CallState* end() { return begin() + 8; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::CallState* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::CallState kVals[8] {
            ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_ACTIVE,
            ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_HOLDING,
            ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_DIALING,
            ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_ALERTING,
            ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_INCOMING,
            ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_WAITING,
            ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_END,
            ::vendor::qti::hardware::radio::ims::V1_0::CallState::CALL_STATE_INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::ServiceType>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::ServiceType* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::ServiceType* end() { return begin() + 4; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::ServiceType* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::ServiceType kVals[4] {
            ::vendor::qti::hardware::radio::ims::V1_0::ServiceType::SMS,
            ::vendor::qti::hardware::radio::ims::V1_0::ServiceType::VOIP,
            ::vendor::qti::hardware::radio::ims::V1_0::ServiceType::VT,
            ::vendor::qti::hardware::radio::ims::V1_0::ServiceType::INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::ExtraType>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::ExtraType* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::ExtraType* end() { return begin() + 2; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::ExtraType* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::ExtraType kVals[2] {
            ::vendor::qti::hardware::radio::ims::V1_0::ExtraType::LTE_TO_IWLAN_HO_FAIL,
            ::vendor::qti::hardware::radio::ims::V1_0::ExtraType::INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::RadioState>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::RadioState* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::RadioState* end() { return begin() + 4; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::RadioState* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::RadioState kVals[4] {
            ::vendor::qti::hardware::radio::ims::V1_0::RadioState::RADIO_STATE_OFF,
            ::vendor::qti::hardware::radio::ims::V1_0::RadioState::RADIO_STATE_UNAVAILABLE,
            ::vendor::qti::hardware::radio::ims::V1_0::RadioState::RADIO_STATE_ON,
            ::vendor::qti::hardware::radio::ims::V1_0::RadioState::RADIO_STATE_INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus* end() { return begin() + 3; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus kVals[3] {
            ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus::DISABLED,
            ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus::ENABLED,
            ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus::INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::FacilityType>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::FacilityType* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::FacilityType* end() { return begin() + 13; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::FacilityType* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::FacilityType kVals[13] {
            ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_CLIP,
            ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_COLP,
            ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAOC,
            ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAOIC,
            ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAOICxH,
            ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAIC,
            ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAICr,
            ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BA_ALL,
            ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BA_MO,
            ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BA_MT,
            ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BS_MT,
            ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_BAICa,
            ::vendor::qti::hardware::radio::ims::V1_0::FacilityType::FACILITY_INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::ClipStatus>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus* end() { return begin() + 4; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus kVals[4] {
            ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus::NOT_PROVISIONED,
            ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus::PROVISIONED,
            ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus::STATUS_UNKNOWN,
            ::vendor::qti::hardware::radio::ims::V1_0::ClipStatus::INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::TtyMode>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::TtyMode* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::TtyMode* end() { return begin() + 5; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::TtyMode* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::TtyMode kVals[5] {
            ::vendor::qti::hardware::radio::ims::V1_0::TtyMode::TTY_MODE_OFF,
            ::vendor::qti::hardware::radio::ims::V1_0::TtyMode::TTY_MODE_FULL,
            ::vendor::qti::hardware::radio::ims::V1_0::TtyMode::TTY_MODE_HCO,
            ::vendor::qti::hardware::radio::ims::V1_0::TtyMode::TTY_MODE_VCO,
            ::vendor::qti::hardware::radio::ims::V1_0::TtyMode::TTY_MODE_INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState* end() { return begin() + 4; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState kVals[4] {
            ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState::RINGING,
            ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState::FOREGROUND,
            ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState::BACKGROUND,
            ::vendor::qti::hardware::radio::ims::V1_0::ConferenceCallState::INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause* end() { return begin() + 14; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause kVals[14] {
            ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_SUCCESS,
            ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_RADIO_NOT_AVAILABLE,
            ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_GENERIC_FAILURE,
            ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_REQUEST_NOT_SUPPORTED,
            ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_CANCELLED,
            ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_UNUSED,
            ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_INVALID_PARAMETER,
            ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_REJECTED_BY_REMOTE,
            ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_IMS_DEREGISTERED,
            ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_NETWORK_NOT_SUPPORTED,
            ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_HOLD_RESUME_FAILED,
            ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_HOLD_RESUME_CANCELED,
            ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_REINVITE_COLLISION,
            ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause::E_INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::NotificationType>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::NotificationType* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::NotificationType* end() { return begin() + 3; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::NotificationType* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::NotificationType kVals[3] {
            ::vendor::qti::hardware::radio::ims::V1_0::NotificationType::MO,
            ::vendor::qti::hardware::radio::ims::V1_0::NotificationType::MT,
            ::vendor::qti::hardware::radio::ims::V1_0::NotificationType::INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::MessageType>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::MessageType* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::MessageType* end() { return begin() + 8; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::MessageType* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::MessageType kVals[8] {
            ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_NONE,
            ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_VOICE,
            ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_VIDEO,
            ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_FAX,
            ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_PAGER,
            ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_MULTIMEDIA,
            ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_TEXT,
            ::vendor::qti::hardware::radio::ims::V1_0::MessageType::MWI_MSG_INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::MessagePriority>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority* end() { return begin() + 5; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority kVals[5] {
            ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority::MWI_MSG_PRIORITY_UNKNOWN,
            ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority::MWI_MSG_PRIORITY_LOW,
            ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority::MWI_MSG_PRIORITY_NORMAL,
            ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority::MWI_MSG_PRIORITY_URGENT,
            ::vendor::qti::hardware::radio::ims::V1_0::MessagePriority::MWI_MSG_PRIORITY_INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::EctType>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::EctType* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::EctType* end() { return begin() + 4; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::EctType* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::EctType kVals[4] {
            ::vendor::qti::hardware::radio::ims::V1_0::EctType::BLIND_TRANSFER,
            ::vendor::qti::hardware::radio::ims::V1_0::EctType::ASSURED_TRANSFER,
            ::vendor::qti::hardware::radio::ims::V1_0::EctType::CONSULTATIVE_TRANSFER,
            ::vendor::qti::hardware::radio::ims::V1_0::EctType::INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation* end() { return begin() + 3; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation kVals[3] {
            ::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation::ADD,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation::REMOVE,
            ::vendor::qti::hardware::radio::ims::V1_0::ConfParticipantOperation::INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType* end() { return begin() + 5; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType kVals[5] {
            ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType::PDP_FAILURE,
            ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType::REGISTRATION_FAILURE,
            ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType::HANDOVER_FAILURE,
            ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType::OTHER_FAILURE,
            ::vendor::qti::hardware::radio::ims::V1_0::BlockReasonType::INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType* end() { return begin() + 11; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType kVals[11] {
            ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::UNSPECIFIED,
            ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::MOBILE_IP,
            ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::INTERNAL,
            ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::CALL_MANAGER_DEFINED,
            ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::TYPE_3GPP_SPEC_DEFINED,
            ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::PPP,
            ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::EHRPD,
            ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::IPV6,
            ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::IWLAN,
            ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::HANDOFF,
            ::vendor::qti::hardware::radio::ims::V1_0::RegFailureReasonType::INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::RttMode>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::RttMode* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::RttMode* end() { return begin() + 3; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::RttMode* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::RttMode kVals[3] {
            ::vendor::qti::hardware::radio::ims::V1_0::RttMode::RTT_MODE_DISABLED,
            ::vendor::qti::hardware::radio::ims::V1_0::RttMode::RTT_MODE_FULL,
            ::vendor::qti::hardware::radio::ims::V1_0::RttMode::RTT_MODE_INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo* end() { return begin() + 5; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo kVals[5] {
            ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_NONE,
            ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_EXCELLENT,
            ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_FAIR,
            ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_BAD,
            ::vendor::qti::hardware::radio::ims::V1_0::VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::SsServiceType>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType* end() { return begin() + 24; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType kVals[24] {
            ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CFU,
            ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CF_BUSY,
            ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CF_NO_REPLY,
            ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CF_NOT_REACHABLE,
            ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CF_ALL,
            ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CF_ALL_CONDITIONAL,
            ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CFUT,
            ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CLIP,
            ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CLIR,
            ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_COLP,
            ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_COLR,
            ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_CNAP,
            ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_WAIT,
            ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_BAOC,
            ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_BAOIC,
            ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_BAOIC_EXC_HOME,
            ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_BAIC,
            ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_BAIC_ROAMING,
            ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_ALL_BARRING,
            ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_OUTGOING_BARRING,
            ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_INCOMING_BARRING,
            ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_INCOMING_BARRING_DN,
            ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_INCOMING_BARRING_ANONYMOUS,
            ::vendor::qti::hardware::radio::ims::V1_0::SsServiceType::SS_INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::SsRequestType>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType* end() { return begin() + 6; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType kVals[6] {
            ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_ACTIVATION,
            ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_DEACTIVATION,
            ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_INTERROGATION,
            ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_REGISTRATION,
            ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_ERASURE,
            ::vendor::qti::hardware::radio::ims::V1_0::SsRequestType::SS_INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType>
{
    const ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType* end() { return begin() + 7; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType kVals[7] {
            ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_ALL_TELE_AND_BEARER_SERVICES,
            ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_ALL_TELESEVICES,
            ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_TELEPHONY,
            ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_ALL_DATA_TELESERVICES,
            ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_SMS_SERVICES,
            ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_ALL_TELESERVICES_EXCEPT_SMS,
            ::vendor::qti::hardware::radio::ims::V1_0::SsTeleserviceType::SS_INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android

#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_0_TYPES_H
