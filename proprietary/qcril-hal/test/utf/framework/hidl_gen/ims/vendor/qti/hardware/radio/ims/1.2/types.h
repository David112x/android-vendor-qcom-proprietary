/*
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_2_TYPES_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_2_TYPES_H

#include <vendor/qti/hardware/radio/ims/1.0/types.h>
#include <vendor/qti/hardware/radio/ims/1.1/types.h>

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_2 {

// Forward declaration for forward reference support:
enum class CallFailCause : int32_t;
struct CallFailCauseResponse;
struct HangupRequestInfo;
struct CallInfo;
enum class ImsSmsSendStatusResult : int32_t;
enum class ImsSmsSendFailureReason : int32_t;
enum class ImsSmsDeliverStatusResult : int32_t;
enum class ImsSmsStatusReportResult : int32_t;
enum class VerificationStatus : int32_t;
struct ImsSmsMessage;
struct ImsSmsSendStatusReport;
struct IncomingImsSms;
struct AddressInfo;

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
    CALL_FAIL_SIP_ALTERNATE_EMERGENCY_CALL = 550, // (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_INVALID implicitly + 1)
    CALL_FAIL_RADIO_OFF = 551, // (::vendor::qti::hardware::radio::ims::V1_1::CallFailCause.CALL_FAIL_SIP_ALTERNATE_EMERGENCY_CALL implicitly + 1)
    CALL_FAIL_OUT_OF_SERVICE = 552, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_RADIO_OFF implicitly + 1)
    CALL_FAIL_NO_VALID_SIM = 553, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OUT_OF_SERVICE implicitly + 1)
    CALL_FAIL_RADIO_INTERNAL_ERROR = 554, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_NO_VALID_SIM implicitly + 1)
    CALL_FAIL_NETWORK_RESP_TIMEOUT = 555, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_RADIO_INTERNAL_ERROR implicitly + 1)
    CALL_FAIL_NETWORK_REJECT = 556, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_NETWORK_RESP_TIMEOUT implicitly + 1)
    CALL_FAIL_RADIO_ACCESS_FAILURE = 557, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_NETWORK_REJECT implicitly + 1)
    CALL_FAIL_RADIO_LINK_FAILURE = 558, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_RADIO_ACCESS_FAILURE implicitly + 1)
    CALL_FAIL_RADIO_LINK_LOST = 559, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_RADIO_LINK_FAILURE implicitly + 1)
    CALL_FAIL_RADIO_UPLINK_FAILURE = 560, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_RADIO_LINK_LOST implicitly + 1)
    CALL_FAIL_RADIO_SETUP_FAILURE = 561, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_RADIO_UPLINK_FAILURE implicitly + 1)
    CALL_FAIL_RADIO_RELEASE_NORMAL = 562, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_RADIO_SETUP_FAILURE implicitly + 1)
    CALL_FAIL_RADIO_RELEASE_ABNORMAL = 563, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_RADIO_RELEASE_NORMAL implicitly + 1)
    CALL_FAIL_NETWORK_DETACH = 564, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_RADIO_RELEASE_ABNORMAL implicitly + 1)
    CALL_FAIL_OEM_CAUSE_1 = 565, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_NETWORK_DETACH implicitly + 1)
    CALL_FAIL_OEM_CAUSE_2 = 566, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_1 implicitly + 1)
    CALL_FAIL_OEM_CAUSE_3 = 567, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_2 implicitly + 1)
    CALL_FAIL_OEM_CAUSE_4 = 568, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_3 implicitly + 1)
    CALL_FAIL_OEM_CAUSE_5 = 569, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_4 implicitly + 1)
    CALL_FAIL_OEM_CAUSE_6 = 570, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_5 implicitly + 1)
    CALL_FAIL_OEM_CAUSE_7 = 571, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_6 implicitly + 1)
    CALL_FAIL_OEM_CAUSE_8 = 572, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_7 implicitly + 1)
    CALL_FAIL_OEM_CAUSE_9 = 573, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_8 implicitly + 1)
    CALL_FAIL_OEM_CAUSE_10 = 574, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_9 implicitly + 1)
    CALL_FAIL_OEM_CAUSE_11 = 575, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_10 implicitly + 1)
    CALL_FAIL_OEM_CAUSE_12 = 576, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_11 implicitly + 1)
    CALL_FAIL_OEM_CAUSE_13 = 577, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_12 implicitly + 1)
    CALL_FAIL_OEM_CAUSE_14 = 578, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_13 implicitly + 1)
    CALL_FAIL_OEM_CAUSE_15 = 579, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_14 implicitly + 1)
    CALL_FAIL_NO_CSFB_IN_CS_ROAM = 580, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_15 implicitly + 1)
};

struct CallFailCauseResponse final {
    ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause failCause __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<uint8_t> errorinfo __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string networkErrorString __attribute__ ((aligned(8)));
    bool hasErrorDetails __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo errorDetails __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallFailCauseResponse, failCause) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallFailCauseResponse, errorinfo) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallFailCauseResponse, networkErrorString) == 24, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallFailCauseResponse, hasErrorDetails) == 40, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallFailCauseResponse, errorDetails) == 48, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_2::CallFailCauseResponse) == 72, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_2::CallFailCauseResponse) == 8, "wrong alignment");

struct HangupRequestInfo final {
    int32_t connIndex __attribute__ ((aligned(4)));
    bool hasMultiParty __attribute__ ((aligned(1)));
    bool multiParty __attribute__ ((aligned(1)));
    ::android::hardware::hidl_string connUri __attribute__ ((aligned(8)));
    uint32_t conf_id __attribute__ ((aligned(4)));
    bool hasFailCauseResponse __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_2::CallFailCauseResponse failCauseResponse __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::HangupRequestInfo, connIndex) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::HangupRequestInfo, hasMultiParty) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::HangupRequestInfo, multiParty) == 5, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::HangupRequestInfo, connUri) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::HangupRequestInfo, conf_id) == 24, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::HangupRequestInfo, hasFailCauseResponse) == 28, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::HangupRequestInfo, failCauseResponse) == 32, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_2::HangupRequestInfo) == 104, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_2::HangupRequestInfo) == 8, "wrong alignment");

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
    ::vendor::qti::hardware::radio::ims::V1_2::CallFailCauseResponse failCause __attribute__ ((aligned(8)));
    bool hasIsEncrypted __attribute__ ((aligned(1)));
    bool isEncrypted __attribute__ ((aligned(1)));
    bool hasIsCalledPartyRinging __attribute__ ((aligned(1)));
    bool isCalledPartyRinging __attribute__ ((aligned(1)));
    ::android::hardware::hidl_string historyInfo __attribute__ ((aligned(8)));
    bool hasIsVideoConfSupported __attribute__ ((aligned(1)));
    bool isVideoConfSupported __attribute__ ((aligned(1)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallInfo, state) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallInfo, index) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallInfo, toa) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallInfo, hasIsMpty) == 12, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallInfo, isMpty) == 13, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallInfo, hasIsMT) == 14, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallInfo, isMT) == 15, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallInfo, als) == 16, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallInfo, hasIsVoice) == 20, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallInfo, isVoice) == 21, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallInfo, hasIsVoicePrivacy) == 22, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallInfo, isVoicePrivacy) == 23, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallInfo, number) == 24, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallInfo, numberPresentation) == 40, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallInfo, name) == 48, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallInfo, namePresentation) == 64, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallInfo, hasCallDetails) == 68, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallInfo, callDetails) == 72, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallInfo, hasFailCause) == 168, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallInfo, failCause) == 176, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallInfo, hasIsEncrypted) == 248, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallInfo, isEncrypted) == 249, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallInfo, hasIsCalledPartyRinging) == 250, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallInfo, isCalledPartyRinging) == 251, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallInfo, historyInfo) == 256, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallInfo, hasIsVideoConfSupported) == 272, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::CallInfo, isVideoConfSupported) == 273, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_2::CallInfo) == 280, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_2::CallInfo) == 8, "wrong alignment");

enum class ImsSmsSendStatusResult : int32_t {
    /**
     * Message was sent successfully.  */
    SEND_STATUS_OK = 0,
    /**
     * IMS provider failed to send the message and platform
     * should not retry falling back to sending the message
     * using the radio.  */
    SEND_STATUS_ERROR = 1, // (::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult.SEND_STATUS_OK implicitly + 1)
    /**
     * IMS provider failed to send the message and platform
     * should retry again after setting TP-RD  */
    SEND_STATUS_ERROR_RETRY = 2, // (::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult.SEND_STATUS_ERROR implicitly + 1)
    /**
     * IMS provider failed to send the message and platform
     * should retry falling back to sending the message
     * using the radio.  */
    SEND_STATUS_ERROR_FALLBACK = 3, // (::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult.SEND_STATUS_ERROR_RETRY implicitly + 1)
};

enum class ImsSmsSendFailureReason : int32_t {
    RESULT_ERROR_NONE = 0,
    /**
     * Generic failure cause  */
    RESULT_ERROR_GENERIC_FAILURE = 1, // (::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason.RESULT_ERROR_NONE implicitly + 1)
    /**
     * Failed because radio was explicitly turned off  */
    RESULT_ERROR_RADIO_OFF = 2, // (::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason.RESULT_ERROR_GENERIC_FAILURE implicitly + 1)
    /**
     * Failed because no pdu provided  */
    RESULT_ERROR_NULL_PDU = 3, // (::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason.RESULT_ERROR_RADIO_OFF implicitly + 1)
    /**
     * Failed because service is currently unavailable  */
    RESULT_ERROR_NO_SERVICE = 4, // (::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason.RESULT_ERROR_NULL_PDU implicitly + 1)
    /**
     * Failed because we reached the sending queue limit.  */
    RESULT_ERROR_LIMIT_EXCEEDED = 5, // (::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason.RESULT_ERROR_NO_SERVICE implicitly + 1)
    /**
     * Failed because user denied the sending of this short code.  */
    RESULT_ERROR_SHORT_CODE_NOT_ALLOWED = 6, // (::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason.RESULT_ERROR_LIMIT_EXCEEDED implicitly + 1)
    /**
     * Failed because the user has denied this app
     * ever send premium short codes.  */
    RESULT_ERROR_SHORT_CODE_NEVER_ALLOWED = 7, // (::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason.RESULT_ERROR_SHORT_CODE_NOT_ALLOWED implicitly + 1)
};

enum class ImsSmsDeliverStatusResult : int32_t {
    /**
     * Message was delivered successfully.  */
    DELIVER_STATUS_OK = 0,
    /**
     * Message was not delivered.  */
    DELIVER_STATUS_ERROR = 1, // (::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult.DELIVER_STATUS_OK implicitly + 1)
};

enum class ImsSmsStatusReportResult : int32_t {
    /**
     * Status Report was set successfully.  */
    STATUS_REPORT_STATUS_OK = 0,
    /**
     * Error while setting status report  */
    STATUS_REPORT_STATUS_ERROR = 1, // (::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult.STATUS_REPORT_STATUS_OK implicitly + 1)
};

enum class VerificationStatus : int32_t {
    /**
     * Telephone number is not validated.  */
    STATUS_VALIDATION_NONE = 0,
    /**
     * Telephone number validation passed.  */
    STATUS_VALIDATION_PASS = 1, // (::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus.STATUS_VALIDATION_NONE implicitly + 1)
    /**
     * Telephone number validation failed.  */
    STATUS_VALIDATION_FAIL = 2, // (::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus.STATUS_VALIDATION_PASS implicitly + 1)
};

struct ImsSmsMessage final {
    uint32_t messageRef __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string format __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string smsc __attribute__ ((aligned(8)));
    bool shallRetry __attribute__ ((aligned(1)));
    ::android::hardware::hidl_vec<uint8_t> pdu __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsMessage, messageRef) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsMessage, format) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsMessage, smsc) == 24, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsMessage, shallRetry) == 40, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsMessage, pdu) == 48, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsMessage) == 64, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsMessage) == 8, "wrong alignment");

struct ImsSmsSendStatusReport final {
    uint32_t messageRef __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string format __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<uint8_t> pdu __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusReport, messageRef) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusReport, format) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusReport, pdu) == 24, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusReport) == 40, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusReport) == 8, "wrong alignment");

struct IncomingImsSms final {
    ::android::hardware::hidl_string format __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<uint8_t> pdu __attribute__ ((aligned(8)));
    ::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus verstat __attribute__ ((aligned(4)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::IncomingImsSms, format) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::IncomingImsSms, pdu) == 16, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::IncomingImsSms, verstat) == 32, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_2::IncomingImsSms) == 40, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_2::IncomingImsSms) == 8, "wrong alignment");

struct AddressInfo final {
    ::android::hardware::hidl_string city __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string state __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string country __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string postalCode __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string countryCode __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string street __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string houseNumber __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::AddressInfo, city) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::AddressInfo, state) == 16, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::AddressInfo, country) == 32, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::AddressInfo, postalCode) == 48, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::AddressInfo, countryCode) == 64, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::AddressInfo, street) == 80, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_2::AddressInfo, houseNumber) == 96, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_2::AddressInfo) == 112, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_2::AddressInfo) == 8, "wrong alignment");

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause lhs, const ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause lhs, const ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_2::CallFailCause>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_2::CallFailCause> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_UNOBTAINABLE_NUMBER) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_UNOBTAINABLE_NUMBER)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_UNOBTAINABLE_NUMBER";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_UNOBTAINABLE_NUMBER;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NORMAL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NORMAL)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_NORMAL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NORMAL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_BUSY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_BUSY)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_BUSY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_BUSY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NORMAL_UNSPECIFIED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NORMAL_UNSPECIFIED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_NORMAL_UNSPECIFIED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NORMAL_UNSPECIFIED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_CONGESTION) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_CONGESTION)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CONGESTION";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_CONGESTION;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_INCOMPATIBILITY_DESTINATION) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_INCOMPATIBILITY_DESTINATION)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_INCOMPATIBILITY_DESTINATION";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_INCOMPATIBILITY_DESTINATION;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_CALL_BARRED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_CALL_BARRED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CALL_BARRED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_CALL_BARRED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_FDN_BLOCKED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_FDN_BLOCKED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_FDN_BLOCKED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_FDN_BLOCKED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_IMEI_NOT_ACCEPTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_IMEI_NOT_ACCEPTED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_IMEI_NOT_ACCEPTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_IMEI_NOT_ACCEPTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_USSD) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_USSD)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_MODIFIED_TO_USSD";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_USSD;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_SS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_SS)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_MODIFIED_TO_SS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_SS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_MODIFIED_TO_DIAL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_EMERGENCY_TEMP_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_EMERGENCY_TEMP_FAILURE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_EMERGENCY_TEMP_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_EMERGENCY_TEMP_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_EMERGENCY_PERM_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_EMERGENCY_PERM_FAILURE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_EMERGENCY_PERM_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_EMERGENCY_PERM_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_HO_NOT_FEASIBLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_HO_NOT_FEASIBLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_HO_NOT_FEASIBLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_HO_NOT_FEASIBLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_USER_BUSY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_USER_BUSY)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_USER_BUSY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_USER_BUSY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_USER_REJECT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_USER_REJECT)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_USER_REJECT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_USER_REJECT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_LOW_BATTERY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_LOW_BATTERY)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_LOW_BATTERY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_LOW_BATTERY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_BLACKLISTED_CALL_ID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_BLACKLISTED_CALL_ID)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_BLACKLISTED_CALL_ID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_BLACKLISTED_CALL_ID;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_CS_RETRY_REQUIRED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_CS_RETRY_REQUIRED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CS_RETRY_REQUIRED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_CS_RETRY_REQUIRED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NETWORK_UNAVAILABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NETWORK_UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_NETWORK_UNAVAILABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NETWORK_UNAVAILABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_FEATURE_UNAVAILABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_FEATURE_UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_FEATURE_UNAVAILABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_FEATURE_UNAVAILABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_ERROR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_ERROR)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_ERROR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_ERROR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_MISC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_MISC)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_MISC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_MISC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_ANSWERED_ELSEWHERE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_ANSWERED_ELSEWHERE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_ANSWERED_ELSEWHERE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_ANSWERED_ELSEWHERE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_PULL_OUT_OF_SYNC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_PULL_OUT_OF_SYNC)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_PULL_OUT_OF_SYNC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_PULL_OUT_OF_SYNC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_CAUSE_CALL_PULLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_CAUSE_CALL_PULLED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CAUSE_CALL_PULLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_CAUSE_CALL_PULLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_ACCESS_CLASS_BLOCKED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_ACCESS_CLASS_BLOCKED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_ACCESS_CLASS_BLOCKED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_ACCESS_CLASS_BLOCKED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_REDIRECTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_REDIRECTED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_REDIRECTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_REDIRECTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_BAD_REQUEST) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_BAD_REQUEST)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_BAD_REQUEST";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_BAD_REQUEST;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_FORBIDDEN) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_FORBIDDEN)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_FORBIDDEN";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_FORBIDDEN;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_NOT_FOUND) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_NOT_FOUND)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_NOT_FOUND";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_NOT_FOUND;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_NOT_SUPPORTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_NOT_SUPPORTED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_NOT_SUPPORTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_NOT_SUPPORTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_REQUEST_TIMEOUT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_REQUEST_TIMEOUT)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_REQUEST_TIMEOUT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_REQUEST_TIMEOUT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_BAD_ADDRESS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_BAD_ADDRESS)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_BAD_ADDRESS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_BAD_ADDRESS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_BUSY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_BUSY)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_BUSY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_BUSY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_REQUEST_CANCELLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_REQUEST_CANCELLED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_REQUEST_CANCELLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_REQUEST_CANCELLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_NOT_ACCEPTABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_NOT_ACCEPTABLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_NOT_ACCEPTABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_NOT_ACCEPTABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_NOT_REACHABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_NOT_REACHABLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_NOT_REACHABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_NOT_REACHABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_INTERNAL_ERROR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_INTERNAL_ERROR)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVER_INTERNAL_ERROR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_INTERNAL_ERROR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_BAD_GATEWAY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_BAD_GATEWAY)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVER_BAD_GATEWAY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_BAD_GATEWAY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVICE_UNAVAILABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVICE_UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVICE_UNAVAILABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVICE_UNAVAILABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_TIMEOUT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_TIMEOUT)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVER_TIMEOUT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_TIMEOUT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_USER_REJECTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_USER_REJECTED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_USER_REJECTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_USER_REJECTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_GLOBAL_ERROR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_GLOBAL_ERROR)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_GLOBAL_ERROR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_GLOBAL_ERROR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_MEDIA_INIT_FAILED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_MEDIA_INIT_FAILED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_MEDIA_INIT_FAILED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_MEDIA_INIT_FAILED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_MEDIA_NO_DATA) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_MEDIA_NO_DATA)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_MEDIA_NO_DATA";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_MEDIA_NO_DATA;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_MEDIA_NOT_ACCEPTABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_MEDIA_NOT_ACCEPTABLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_MEDIA_NOT_ACCEPTABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_MEDIA_NOT_ACCEPTABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_MEDIA_UNSPECIFIED_ERROR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_MEDIA_UNSPECIFIED_ERROR)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_MEDIA_UNSPECIFIED_ERROR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_MEDIA_UNSPECIFIED_ERROR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_HOLD_RESUME_FAILED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_HOLD_RESUME_FAILED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_HOLD_RESUME_FAILED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_HOLD_RESUME_FAILED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_HOLD_RESUME_CANCELED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_HOLD_RESUME_CANCELED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_HOLD_RESUME_CANCELED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_HOLD_RESUME_CANCELED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_HOLD_REINVITE_COLLISION) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_HOLD_REINVITE_COLLISION)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_HOLD_REINVITE_COLLISION";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_HOLD_REINVITE_COLLISION;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_ERROR_UNSPECIFIED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_ERROR_UNSPECIFIED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_ERROR_UNSPECIFIED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_ERROR_UNSPECIFIED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_INVALID)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_INVALID;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_ALTERNATE_EMERGENCY_CALL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_ALTERNATE_EMERGENCY_CALL)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_ALTERNATE_EMERGENCY_CALL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_ALTERNATE_EMERGENCY_CALL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_OFF) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_OFF)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_RADIO_OFF";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_OFF;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OUT_OF_SERVICE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OUT_OF_SERVICE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OUT_OF_SERVICE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OUT_OF_SERVICE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NO_VALID_SIM) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NO_VALID_SIM)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_NO_VALID_SIM";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NO_VALID_SIM;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_INTERNAL_ERROR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_INTERNAL_ERROR)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_RADIO_INTERNAL_ERROR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_INTERNAL_ERROR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NETWORK_RESP_TIMEOUT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NETWORK_RESP_TIMEOUT)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_NETWORK_RESP_TIMEOUT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NETWORK_RESP_TIMEOUT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NETWORK_REJECT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NETWORK_REJECT)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_NETWORK_REJECT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NETWORK_REJECT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_ACCESS_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_ACCESS_FAILURE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_RADIO_ACCESS_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_ACCESS_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_LINK_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_LINK_FAILURE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_RADIO_LINK_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_LINK_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_LINK_LOST) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_LINK_LOST)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_RADIO_LINK_LOST";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_LINK_LOST;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_UPLINK_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_UPLINK_FAILURE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_RADIO_UPLINK_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_UPLINK_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_SETUP_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_SETUP_FAILURE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_RADIO_SETUP_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_SETUP_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_RELEASE_NORMAL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_RELEASE_NORMAL)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_RADIO_RELEASE_NORMAL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_RELEASE_NORMAL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_RELEASE_ABNORMAL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_RELEASE_ABNORMAL)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_RADIO_RELEASE_ABNORMAL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_RELEASE_ABNORMAL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NETWORK_DETACH) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NETWORK_DETACH)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_NETWORK_DETACH";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NETWORK_DETACH;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_1) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_1)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_1";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_1;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_2) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_2)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_2";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_2;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_3) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_3)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_3";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_3;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_4) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_4)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_4";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_4;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_5) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_5)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_5";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_5;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_6) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_6)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_6";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_6;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_7) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_7)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_7";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_7;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_8) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_8)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_8";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_8;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_9) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_9)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_9";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_9;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_10) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_10)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_10";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_10;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_11) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_11)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_11";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_11;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_12) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_12)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_12";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_12;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_13) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_13)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_13";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_13;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_14) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_14)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_14";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_14;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_15) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_15)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_15";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_15;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NO_CSFB_IN_CS_ROAM) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NO_CSFB_IN_CS_ROAM)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_NO_CSFB_IN_CS_ROAM";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NO_CSFB_IN_CS_ROAM;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_2::CallFailCause o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_UNOBTAINABLE_NUMBER) {
        return "CALL_FAIL_UNOBTAINABLE_NUMBER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NORMAL) {
        return "CALL_FAIL_NORMAL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_BUSY) {
        return "CALL_FAIL_BUSY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NORMAL_UNSPECIFIED) {
        return "CALL_FAIL_NORMAL_UNSPECIFIED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_CONGESTION) {
        return "CALL_FAIL_CONGESTION";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_INCOMPATIBILITY_DESTINATION) {
        return "CALL_FAIL_INCOMPATIBILITY_DESTINATION";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_CALL_BARRED) {
        return "CALL_FAIL_CALL_BARRED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_FDN_BLOCKED) {
        return "CALL_FAIL_FDN_BLOCKED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_IMEI_NOT_ACCEPTED) {
        return "CALL_FAIL_IMEI_NOT_ACCEPTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_USSD) {
        return "CALL_FAIL_DIAL_MODIFIED_TO_USSD";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_SS) {
        return "CALL_FAIL_DIAL_MODIFIED_TO_SS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL) {
        return "CALL_FAIL_DIAL_MODIFIED_TO_DIAL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_EMERGENCY_TEMP_FAILURE) {
        return "CALL_FAIL_EMERGENCY_TEMP_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_EMERGENCY_PERM_FAILURE) {
        return "CALL_FAIL_EMERGENCY_PERM_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_HO_NOT_FEASIBLE) {
        return "CALL_FAIL_HO_NOT_FEASIBLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_USER_BUSY) {
        return "CALL_FAIL_USER_BUSY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_USER_REJECT) {
        return "CALL_FAIL_USER_REJECT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_LOW_BATTERY) {
        return "CALL_FAIL_LOW_BATTERY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_BLACKLISTED_CALL_ID) {
        return "CALL_FAIL_BLACKLISTED_CALL_ID";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_CS_RETRY_REQUIRED) {
        return "CALL_FAIL_CS_RETRY_REQUIRED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NETWORK_UNAVAILABLE) {
        return "CALL_FAIL_NETWORK_UNAVAILABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_FEATURE_UNAVAILABLE) {
        return "CALL_FAIL_FEATURE_UNAVAILABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_ERROR) {
        return "CALL_FAIL_SIP_ERROR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_MISC) {
        return "CALL_FAIL_MISC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_ANSWERED_ELSEWHERE) {
        return "CALL_FAIL_ANSWERED_ELSEWHERE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_PULL_OUT_OF_SYNC) {
        return "CALL_FAIL_PULL_OUT_OF_SYNC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_CAUSE_CALL_PULLED) {
        return "CALL_FAIL_CAUSE_CALL_PULLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_ACCESS_CLASS_BLOCKED) {
        return "CALL_FAIL_ACCESS_CLASS_BLOCKED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_REDIRECTED) {
        return "CALL_FAIL_SIP_REDIRECTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_BAD_REQUEST) {
        return "CALL_FAIL_SIP_BAD_REQUEST";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_FORBIDDEN) {
        return "CALL_FAIL_SIP_FORBIDDEN";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_NOT_FOUND) {
        return "CALL_FAIL_SIP_NOT_FOUND";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_NOT_SUPPORTED) {
        return "CALL_FAIL_SIP_NOT_SUPPORTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_REQUEST_TIMEOUT) {
        return "CALL_FAIL_SIP_REQUEST_TIMEOUT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE) {
        return "CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_BAD_ADDRESS) {
        return "CALL_FAIL_SIP_BAD_ADDRESS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_BUSY) {
        return "CALL_FAIL_SIP_BUSY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_REQUEST_CANCELLED) {
        return "CALL_FAIL_SIP_REQUEST_CANCELLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_NOT_ACCEPTABLE) {
        return "CALL_FAIL_SIP_NOT_ACCEPTABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_NOT_REACHABLE) {
        return "CALL_FAIL_SIP_NOT_REACHABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_INTERNAL_ERROR) {
        return "CALL_FAIL_SIP_SERVER_INTERNAL_ERROR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED) {
        return "CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_BAD_GATEWAY) {
        return "CALL_FAIL_SIP_SERVER_BAD_GATEWAY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVICE_UNAVAILABLE) {
        return "CALL_FAIL_SIP_SERVICE_UNAVAILABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_TIMEOUT) {
        return "CALL_FAIL_SIP_SERVER_TIMEOUT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED) {
        return "CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE) {
        return "CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE) {
        return "CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_USER_REJECTED) {
        return "CALL_FAIL_SIP_USER_REJECTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_GLOBAL_ERROR) {
        return "CALL_FAIL_SIP_GLOBAL_ERROR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_MEDIA_INIT_FAILED) {
        return "CALL_FAIL_MEDIA_INIT_FAILED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_MEDIA_NO_DATA) {
        return "CALL_FAIL_MEDIA_NO_DATA";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_MEDIA_NOT_ACCEPTABLE) {
        return "CALL_FAIL_MEDIA_NOT_ACCEPTABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_MEDIA_UNSPECIFIED_ERROR) {
        return "CALL_FAIL_MEDIA_UNSPECIFIED_ERROR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_HOLD_RESUME_FAILED) {
        return "CALL_FAIL_HOLD_RESUME_FAILED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_HOLD_RESUME_CANCELED) {
        return "CALL_FAIL_HOLD_RESUME_CANCELED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_HOLD_REINVITE_COLLISION) {
        return "CALL_FAIL_HOLD_REINVITE_COLLISION";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO) {
        return "CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL) {
        return "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO) {
        return "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS) {
        return "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD) {
        return "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_ERROR_UNSPECIFIED) {
        return "CALL_FAIL_ERROR_UNSPECIFIED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_INVALID) {
        return "CALL_FAIL_INVALID";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_ALTERNATE_EMERGENCY_CALL) {
        return "CALL_FAIL_SIP_ALTERNATE_EMERGENCY_CALL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_OFF) {
        return "CALL_FAIL_RADIO_OFF";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OUT_OF_SERVICE) {
        return "CALL_FAIL_OUT_OF_SERVICE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NO_VALID_SIM) {
        return "CALL_FAIL_NO_VALID_SIM";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_INTERNAL_ERROR) {
        return "CALL_FAIL_RADIO_INTERNAL_ERROR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NETWORK_RESP_TIMEOUT) {
        return "CALL_FAIL_NETWORK_RESP_TIMEOUT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NETWORK_REJECT) {
        return "CALL_FAIL_NETWORK_REJECT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_ACCESS_FAILURE) {
        return "CALL_FAIL_RADIO_ACCESS_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_LINK_FAILURE) {
        return "CALL_FAIL_RADIO_LINK_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_LINK_LOST) {
        return "CALL_FAIL_RADIO_LINK_LOST";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_UPLINK_FAILURE) {
        return "CALL_FAIL_RADIO_UPLINK_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_SETUP_FAILURE) {
        return "CALL_FAIL_RADIO_SETUP_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_RELEASE_NORMAL) {
        return "CALL_FAIL_RADIO_RELEASE_NORMAL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_RELEASE_ABNORMAL) {
        return "CALL_FAIL_RADIO_RELEASE_ABNORMAL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NETWORK_DETACH) {
        return "CALL_FAIL_NETWORK_DETACH";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_1) {
        return "CALL_FAIL_OEM_CAUSE_1";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_2) {
        return "CALL_FAIL_OEM_CAUSE_2";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_3) {
        return "CALL_FAIL_OEM_CAUSE_3";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_4) {
        return "CALL_FAIL_OEM_CAUSE_4";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_5) {
        return "CALL_FAIL_OEM_CAUSE_5";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_6) {
        return "CALL_FAIL_OEM_CAUSE_6";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_7) {
        return "CALL_FAIL_OEM_CAUSE_7";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_8) {
        return "CALL_FAIL_OEM_CAUSE_8";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_9) {
        return "CALL_FAIL_OEM_CAUSE_9";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_10) {
        return "CALL_FAIL_OEM_CAUSE_10";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_11) {
        return "CALL_FAIL_OEM_CAUSE_11";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_12) {
        return "CALL_FAIL_OEM_CAUSE_12";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_13) {
        return "CALL_FAIL_OEM_CAUSE_13";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_14) {
        return "CALL_FAIL_OEM_CAUSE_14";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_15) {
        return "CALL_FAIL_OEM_CAUSE_15";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NO_CSFB_IN_CS_ROAM) {
        return "CALL_FAIL_NO_CSFB_IN_CS_ROAM";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_2::CallFailCauseResponse& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".failCause = ";
    os += ::vendor::qti::hardware::radio::ims::V1_2::toString(o.failCause);
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

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_2::CallFailCauseResponse& lhs, const ::vendor::qti::hardware::radio::ims::V1_2::CallFailCauseResponse& rhs) {
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

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_2::CallFailCauseResponse& lhs,const ::vendor::qti::hardware::radio::ims::V1_2::CallFailCauseResponse& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_2::HangupRequestInfo& o) {
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
    os += ::vendor::qti::hardware::radio::ims::V1_2::toString(o.failCauseResponse);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_2::HangupRequestInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_2::HangupRequestInfo& rhs) {
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

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_2::HangupRequestInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_2::HangupRequestInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_2::CallInfo& o) {
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
    os += ::vendor::qti::hardware::radio::ims::V1_2::toString(o.failCause);
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

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_2::CallInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_2::CallInfo& rhs) {
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

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_2::CallInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_2::CallInfo& rhs){
    return !(lhs == rhs);
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult lhs, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult lhs, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult::SEND_STATUS_OK) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult::SEND_STATUS_OK)) {
        os += (first ? "" : " | ");
        os += "SEND_STATUS_OK";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult::SEND_STATUS_OK;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult::SEND_STATUS_ERROR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult::SEND_STATUS_ERROR)) {
        os += (first ? "" : " | ");
        os += "SEND_STATUS_ERROR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult::SEND_STATUS_ERROR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult::SEND_STATUS_ERROR_RETRY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult::SEND_STATUS_ERROR_RETRY)) {
        os += (first ? "" : " | ");
        os += "SEND_STATUS_ERROR_RETRY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult::SEND_STATUS_ERROR_RETRY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult::SEND_STATUS_ERROR_FALLBACK) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult::SEND_STATUS_ERROR_FALLBACK)) {
        os += (first ? "" : " | ");
        os += "SEND_STATUS_ERROR_FALLBACK";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult::SEND_STATUS_ERROR_FALLBACK;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult::SEND_STATUS_OK) {
        return "SEND_STATUS_OK";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult::SEND_STATUS_ERROR) {
        return "SEND_STATUS_ERROR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult::SEND_STATUS_ERROR_RETRY) {
        return "SEND_STATUS_ERROR_RETRY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult::SEND_STATUS_ERROR_FALLBACK) {
        return "SEND_STATUS_ERROR_FALLBACK";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason lhs, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason lhs, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_NONE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_NONE)) {
        os += (first ? "" : " | ");
        os += "RESULT_ERROR_NONE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_NONE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_GENERIC_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_GENERIC_FAILURE)) {
        os += (first ? "" : " | ");
        os += "RESULT_ERROR_GENERIC_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_GENERIC_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_RADIO_OFF) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_RADIO_OFF)) {
        os += (first ? "" : " | ");
        os += "RESULT_ERROR_RADIO_OFF";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_RADIO_OFF;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_NULL_PDU) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_NULL_PDU)) {
        os += (first ? "" : " | ");
        os += "RESULT_ERROR_NULL_PDU";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_NULL_PDU;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_NO_SERVICE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_NO_SERVICE)) {
        os += (first ? "" : " | ");
        os += "RESULT_ERROR_NO_SERVICE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_NO_SERVICE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_LIMIT_EXCEEDED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_LIMIT_EXCEEDED)) {
        os += (first ? "" : " | ");
        os += "RESULT_ERROR_LIMIT_EXCEEDED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_LIMIT_EXCEEDED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_SHORT_CODE_NOT_ALLOWED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_SHORT_CODE_NOT_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "RESULT_ERROR_SHORT_CODE_NOT_ALLOWED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_SHORT_CODE_NOT_ALLOWED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_SHORT_CODE_NEVER_ALLOWED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_SHORT_CODE_NEVER_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "RESULT_ERROR_SHORT_CODE_NEVER_ALLOWED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_SHORT_CODE_NEVER_ALLOWED;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_NONE) {
        return "RESULT_ERROR_NONE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_GENERIC_FAILURE) {
        return "RESULT_ERROR_GENERIC_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_RADIO_OFF) {
        return "RESULT_ERROR_RADIO_OFF";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_NULL_PDU) {
        return "RESULT_ERROR_NULL_PDU";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_NO_SERVICE) {
        return "RESULT_ERROR_NO_SERVICE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_LIMIT_EXCEEDED) {
        return "RESULT_ERROR_LIMIT_EXCEEDED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_SHORT_CODE_NOT_ALLOWED) {
        return "RESULT_ERROR_SHORT_CODE_NOT_ALLOWED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_SHORT_CODE_NEVER_ALLOWED) {
        return "RESULT_ERROR_SHORT_CODE_NEVER_ALLOWED";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult lhs, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult lhs, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult::DELIVER_STATUS_OK) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult::DELIVER_STATUS_OK)) {
        os += (first ? "" : " | ");
        os += "DELIVER_STATUS_OK";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult::DELIVER_STATUS_OK;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult::DELIVER_STATUS_ERROR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult::DELIVER_STATUS_ERROR)) {
        os += (first ? "" : " | ");
        os += "DELIVER_STATUS_ERROR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult::DELIVER_STATUS_ERROR;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult::DELIVER_STATUS_OK) {
        return "DELIVER_STATUS_OK";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult::DELIVER_STATUS_ERROR) {
        return "DELIVER_STATUS_ERROR";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult lhs, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult lhs, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult::STATUS_REPORT_STATUS_OK) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult::STATUS_REPORT_STATUS_OK)) {
        os += (first ? "" : " | ");
        os += "STATUS_REPORT_STATUS_OK";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult::STATUS_REPORT_STATUS_OK;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult::STATUS_REPORT_STATUS_ERROR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult::STATUS_REPORT_STATUS_ERROR)) {
        os += (first ? "" : " | ");
        os += "STATUS_REPORT_STATUS_ERROR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult::STATUS_REPORT_STATUS_ERROR;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult::STATUS_REPORT_STATUS_OK) {
        return "STATUS_REPORT_STATUS_OK";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult::STATUS_REPORT_STATUS_ERROR) {
        return "STATUS_REPORT_STATUS_ERROR";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus lhs, const ::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus lhs, const ::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus::STATUS_VALIDATION_NONE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus::STATUS_VALIDATION_NONE)) {
        os += (first ? "" : " | ");
        os += "STATUS_VALIDATION_NONE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus::STATUS_VALIDATION_NONE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus::STATUS_VALIDATION_PASS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus::STATUS_VALIDATION_PASS)) {
        os += (first ? "" : " | ");
        os += "STATUS_VALIDATION_PASS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus::STATUS_VALIDATION_PASS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus::STATUS_VALIDATION_FAIL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus::STATUS_VALIDATION_FAIL)) {
        os += (first ? "" : " | ");
        os += "STATUS_VALIDATION_FAIL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus::STATUS_VALIDATION_FAIL;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus::STATUS_VALIDATION_NONE) {
        return "STATUS_VALIDATION_NONE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus::STATUS_VALIDATION_PASS) {
        return "STATUS_VALIDATION_PASS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus::STATUS_VALIDATION_FAIL) {
        return "STATUS_VALIDATION_FAIL";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsMessage& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".messageRef = ";
    os += ::android::hardware::toString(o.messageRef);
    os += ", .format = ";
    os += ::android::hardware::toString(o.format);
    os += ", .smsc = ";
    os += ::android::hardware::toString(o.smsc);
    os += ", .shallRetry = ";
    os += ::android::hardware::toString(o.shallRetry);
    os += ", .pdu = ";
    os += ::android::hardware::toString(o.pdu);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsMessage& lhs, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsMessage& rhs) {
    if (lhs.messageRef != rhs.messageRef) {
        return false;
    }
    if (lhs.format != rhs.format) {
        return false;
    }
    if (lhs.smsc != rhs.smsc) {
        return false;
    }
    if (lhs.shallRetry != rhs.shallRetry) {
        return false;
    }
    if (lhs.pdu != rhs.pdu) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsMessage& lhs,const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsMessage& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusReport& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".messageRef = ";
    os += ::android::hardware::toString(o.messageRef);
    os += ", .format = ";
    os += ::android::hardware::toString(o.format);
    os += ", .pdu = ";
    os += ::android::hardware::toString(o.pdu);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusReport& lhs, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusReport& rhs) {
    if (lhs.messageRef != rhs.messageRef) {
        return false;
    }
    if (lhs.format != rhs.format) {
        return false;
    }
    if (lhs.pdu != rhs.pdu) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusReport& lhs,const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusReport& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_2::IncomingImsSms& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".format = ";
    os += ::android::hardware::toString(o.format);
    os += ", .pdu = ";
    os += ::android::hardware::toString(o.pdu);
    os += ", .verstat = ";
    os += ::vendor::qti::hardware::radio::ims::V1_2::toString(o.verstat);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_2::IncomingImsSms& lhs, const ::vendor::qti::hardware::radio::ims::V1_2::IncomingImsSms& rhs) {
    if (lhs.format != rhs.format) {
        return false;
    }
    if (lhs.pdu != rhs.pdu) {
        return false;
    }
    if (lhs.verstat != rhs.verstat) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_2::IncomingImsSms& lhs,const ::vendor::qti::hardware::radio::ims::V1_2::IncomingImsSms& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_2::AddressInfo& o) {
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
    os += ", .street = ";
    os += ::android::hardware::toString(o.street);
    os += ", .houseNumber = ";
    os += ::android::hardware::toString(o.houseNumber);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_2::AddressInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_2::AddressInfo& rhs) {
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
    if (lhs.street != rhs.street) {
        return false;
    }
    if (lhs.houseNumber != rhs.houseNumber) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_2::AddressInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_2::AddressInfo& rhs){
    return !(lhs == rhs);
}


}  // namespace V1_2
}  // namespace ims
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_2::CallFailCause>
{
    const ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause* end() { return begin() + 95; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause kVals[95] {
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_UNOBTAINABLE_NUMBER,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NORMAL,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_BUSY,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NORMAL_UNSPECIFIED,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_CONGESTION,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_INCOMPATIBILITY_DESTINATION,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_CALL_BARRED,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_FDN_BLOCKED,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_IMEI_NOT_ACCEPTED,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_USSD,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_SS,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_EMERGENCY_TEMP_FAILURE,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_EMERGENCY_PERM_FAILURE,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_HO_NOT_FEASIBLE,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_USER_BUSY,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_USER_REJECT,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_LOW_BATTERY,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_BLACKLISTED_CALL_ID,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_CS_RETRY_REQUIRED,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NETWORK_UNAVAILABLE,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_FEATURE_UNAVAILABLE,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_ERROR,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_MISC,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_ANSWERED_ELSEWHERE,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_PULL_OUT_OF_SYNC,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_CAUSE_CALL_PULLED,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_ACCESS_CLASS_BLOCKED,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_REDIRECTED,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_BAD_REQUEST,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_FORBIDDEN,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_NOT_FOUND,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_NOT_SUPPORTED,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_REQUEST_TIMEOUT,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_BAD_ADDRESS,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_BUSY,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_REQUEST_CANCELLED,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_NOT_ACCEPTABLE,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_NOT_REACHABLE,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_INTERNAL_ERROR,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_BAD_GATEWAY,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVICE_UNAVAILABLE,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_TIMEOUT,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_USER_REJECTED,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_GLOBAL_ERROR,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_MEDIA_INIT_FAILED,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_MEDIA_NO_DATA,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_MEDIA_NOT_ACCEPTABLE,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_MEDIA_UNSPECIFIED_ERROR,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_HOLD_RESUME_FAILED,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_HOLD_RESUME_CANCELED,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_HOLD_REINVITE_COLLISION,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_ERROR_UNSPECIFIED,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_INVALID,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_SIP_ALTERNATE_EMERGENCY_CALL,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_OFF,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OUT_OF_SERVICE,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NO_VALID_SIM,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_INTERNAL_ERROR,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NETWORK_RESP_TIMEOUT,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NETWORK_REJECT,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_ACCESS_FAILURE,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_LINK_FAILURE,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_LINK_LOST,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_UPLINK_FAILURE,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_SETUP_FAILURE,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_RELEASE_NORMAL,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_RADIO_RELEASE_ABNORMAL,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NETWORK_DETACH,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_1,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_2,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_3,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_4,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_5,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_6,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_7,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_8,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_9,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_10,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_11,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_12,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_13,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_14,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_OEM_CAUSE_15,
            ::vendor::qti::hardware::radio::ims::V1_2::CallFailCause::CALL_FAIL_NO_CSFB_IN_CS_ROAM,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult>
{
    const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult* end() { return begin() + 4; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult kVals[4] {
            ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult::SEND_STATUS_OK,
            ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult::SEND_STATUS_ERROR,
            ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult::SEND_STATUS_ERROR_RETRY,
            ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult::SEND_STATUS_ERROR_FALLBACK,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason>
{
    const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason* end() { return begin() + 8; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason kVals[8] {
            ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_NONE,
            ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_GENERIC_FAILURE,
            ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_RADIO_OFF,
            ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_NULL_PDU,
            ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_NO_SERVICE,
            ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_LIMIT_EXCEEDED,
            ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_SHORT_CODE_NOT_ALLOWED,
            ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason::RESULT_ERROR_SHORT_CODE_NEVER_ALLOWED,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult>
{
    const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult* end() { return begin() + 2; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult kVals[2] {
            ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult::DELIVER_STATUS_OK,
            ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult::DELIVER_STATUS_ERROR,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult>
{
    const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult* end() { return begin() + 2; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult kVals[2] {
            ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult::STATUS_REPORT_STATUS_OK,
            ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult::STATUS_REPORT_STATUS_ERROR,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus>
{
    const ::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus* end() { return begin() + 3; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus kVals[3] {
            ::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus::STATUS_VALIDATION_NONE,
            ::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus::STATUS_VALIDATION_PASS,
            ::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus::STATUS_VALIDATION_FAIL,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android

#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_2_TYPES_H
