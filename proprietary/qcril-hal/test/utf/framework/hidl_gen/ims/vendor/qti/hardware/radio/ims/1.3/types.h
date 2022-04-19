/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_3_TYPES_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_3_TYPES_H

#include <vendor/qti/hardware/radio/ims/1.0/types.h>
#include <vendor/qti/hardware/radio/ims/1.2/types.h>

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_3 {

// Forward declaration for forward reference support:
enum class ServiceClassProvisionStatus : int32_t;
struct SuppServiceStatus;
struct ColrInfo;
enum class CallFailCause : int32_t;
struct CallFailCauseResponse;
struct HangupRequestInfo;
struct VerstatInfo;
struct CallInfo;
struct AutoCallRejectionInfo;
enum class VoiceInfo : int32_t;

// Order of inner types was changed for forward reference support.

enum class ServiceClassProvisionStatus : int32_t {
    NOT_PROVISIONED = 0,
    PROVISIONED = 1, // (::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus.NOT_PROVISIONED implicitly + 1)
    INVALID = 2, // (::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus.PROVISIONED implicitly + 1)
};

struct SuppServiceStatus final {
    ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus status __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus provisionStatus __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_0::FacilityType facilityType __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string failureCause __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_0::CbNumListInfo> cbNumListInfo __attribute__ ((aligned(8)));
    bool hasErrorDetails __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo errorDetails __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::SuppServiceStatus, status) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::SuppServiceStatus, provisionStatus) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::SuppServiceStatus, facilityType) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::SuppServiceStatus, failureCause) == 16, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::SuppServiceStatus, cbNumListInfo) == 32, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::SuppServiceStatus, hasErrorDetails) == 48, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::SuppServiceStatus, errorDetails) == 56, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_3::SuppServiceStatus) == 80, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_3::SuppServiceStatus) == 8, "wrong alignment");

struct ColrInfo final {
    ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus status __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus provisionStatus __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation presentation __attribute__ ((aligned(4)));
    bool hasErrorDetails __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo errorDetails __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::ColrInfo, status) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::ColrInfo, provisionStatus) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::ColrInfo, presentation) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::ColrInfo, hasErrorDetails) == 12, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::ColrInfo, errorDetails) == 16, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_3::ColrInfo) == 40, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_3::ColrInfo) == 8, "wrong alignment");

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
    CALL_FAIL_SRV_NOT_REGISTERED = 581, // (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_NO_CSFB_IN_CS_ROAM implicitly + 1)
    CALL_FAIL_CALL_TYPE_NOT_ALLOWED = 582, // (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_SRV_NOT_REGISTERED implicitly + 1)
    CALL_FAIL_EMRG_CALL_ONGOING = 583, // (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_CALL_TYPE_NOT_ALLOWED implicitly + 1)
    CALL_FAIL_CALL_SETUP_ONGOING = 584, // (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_EMRG_CALL_ONGOING implicitly + 1)
    CALL_FAIL_MAX_CALL_LIMIT_REACHED = 585, // (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_CALL_SETUP_ONGOING implicitly + 1)
    CALL_FAIL_UNSUPPORTED_SIP_HDRS = 586, // (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_MAX_CALL_LIMIT_REACHED implicitly + 1)
    CALL_FAIL_CALL_TRANSFER_ONGOING = 587, // (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_UNSUPPORTED_SIP_HDRS implicitly + 1)
    CALL_FAIL_PRACK_TIMEOUT = 588, // (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_CALL_TRANSFER_ONGOING implicitly + 1)
    CALL_FAIL_QOS_FAILURE = 589, // (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_PRACK_TIMEOUT implicitly + 1)
    CALL_FAIL_ONGOING_HANDOVER = 590, // (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_QOS_FAILURE implicitly + 1)
    CALL_FAIL_VT_WITH_TTY_NOT_ALLOWED = 591, // (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_ONGOING_HANDOVER implicitly + 1)
    CALL_FAIL_CALL_UPGRADE_ONGOING = 592, // (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_VT_WITH_TTY_NOT_ALLOWED implicitly + 1)
    CALL_FAIL_CONFERENCE_WITH_TTY_NOT_ALLOWED = 593, // (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_CALL_UPGRADE_ONGOING implicitly + 1)
    CALL_FAIL_CALL_CONFERENCE_ONGOING = 594, // (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_CONFERENCE_WITH_TTY_NOT_ALLOWED implicitly + 1)
    CALL_FAIL_VT_WITH_AVPF_NOT_ALLOWED = 595, // (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_CALL_CONFERENCE_ONGOING implicitly + 1)
    CALL_FAIL_ENCRYPTION_CALL_ONGOING = 596, // (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_VT_WITH_AVPF_NOT_ALLOWED implicitly + 1)
    CALL_FAIL_CALL_ONGOING_CW_DISABLED = 597, // (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_ENCRYPTION_CALL_ONGOING implicitly + 1)
    CALL_FAIL_CALL_ON_OTHER_SUB = 598, // (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_CALL_ONGOING_CW_DISABLED implicitly + 1)
    CALL_FAIL_1X_COLLISION = 599, // (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_CALL_ON_OTHER_SUB implicitly + 1)
    CALL_FAIL_UI_NOT_READY = 600, // (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_1X_COLLISION implicitly + 1)
    CALL_FAIL_CS_CALL_ONGOING = 601, // (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_UI_NOT_READY implicitly + 1)
    CALL_FAIL_SIP_USER_MARKED_UNWANTED = 602, // (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_CS_CALL_ONGOING implicitly + 1)
};

struct CallFailCauseResponse final {
    ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause failCause __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<uint8_t> errorinfo __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string networkErrorString __attribute__ ((aligned(8)));
    bool hasErrorDetails __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo errorDetails __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallFailCauseResponse, failCause) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallFailCauseResponse, errorinfo) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallFailCauseResponse, networkErrorString) == 24, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallFailCauseResponse, hasErrorDetails) == 40, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallFailCauseResponse, errorDetails) == 48, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_3::CallFailCauseResponse) == 72, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_3::CallFailCauseResponse) == 8, "wrong alignment");

struct HangupRequestInfo final {
    int32_t connIndex __attribute__ ((aligned(4)));
    bool hasMultiParty __attribute__ ((aligned(1)));
    bool multiParty __attribute__ ((aligned(1)));
    ::android::hardware::hidl_string connUri __attribute__ ((aligned(8)));
    uint32_t conf_id __attribute__ ((aligned(4)));
    bool hasFailCauseResponse __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_3::CallFailCauseResponse failCauseResponse __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::HangupRequestInfo, connIndex) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::HangupRequestInfo, hasMultiParty) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::HangupRequestInfo, multiParty) == 5, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::HangupRequestInfo, connUri) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::HangupRequestInfo, conf_id) == 24, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::HangupRequestInfo, hasFailCauseResponse) == 28, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::HangupRequestInfo, failCauseResponse) == 32, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_3::HangupRequestInfo) == 104, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_3::HangupRequestInfo) == 8, "wrong alignment");

struct VerstatInfo final {
    bool canMarkUnwantedCall __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_2::VerificationStatus verificationStatus __attribute__ ((aligned(4)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::VerstatInfo, canMarkUnwantedCall) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::VerstatInfo, verificationStatus) == 4, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_3::VerstatInfo) == 8, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_3::VerstatInfo) == 4, "wrong alignment");

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
    ::vendor::qti::hardware::radio::ims::V1_3::CallFailCauseResponse failCause __attribute__ ((aligned(8)));
    bool hasIsEncrypted __attribute__ ((aligned(1)));
    bool isEncrypted __attribute__ ((aligned(1)));
    bool hasIsCalledPartyRinging __attribute__ ((aligned(1)));
    bool isCalledPartyRinging __attribute__ ((aligned(1)));
    ::android::hardware::hidl_string historyInfo __attribute__ ((aligned(8)));
    bool hasIsVideoConfSupported __attribute__ ((aligned(1)));
    bool isVideoConfSupported __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_3::VerstatInfo verstatInfo __attribute__ ((aligned(4)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo, state) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo, index) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo, toa) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo, hasIsMpty) == 12, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo, isMpty) == 13, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo, hasIsMT) == 14, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo, isMT) == 15, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo, als) == 16, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo, hasIsVoice) == 20, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo, isVoice) == 21, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo, hasIsVoicePrivacy) == 22, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo, isVoicePrivacy) == 23, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo, number) == 24, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo, numberPresentation) == 40, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo, name) == 48, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo, namePresentation) == 64, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo, hasCallDetails) == 68, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo, callDetails) == 72, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo, hasFailCause) == 168, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo, failCause) == 176, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo, hasIsEncrypted) == 248, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo, isEncrypted) == 249, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo, hasIsCalledPartyRinging) == 250, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo, isCalledPartyRinging) == 251, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo, historyInfo) == 256, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo, hasIsVideoConfSupported) == 272, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo, isVideoConfSupported) == 273, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo, verstatInfo) == 276, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo) == 288, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_3::CallInfo) == 8, "wrong alignment");

struct AutoCallRejectionInfo final {
    ::vendor::qti::hardware::radio::ims::V1_0::CallType callType __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause autoRejectionCause __attribute__ ((aligned(4)));
    uint16_t sipErrorCode __attribute__ ((aligned(2)));
    ::android::hardware::hidl_string number __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::AutoCallRejectionInfo, callType) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::AutoCallRejectionInfo, autoRejectionCause) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::AutoCallRejectionInfo, sipErrorCode) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_3::AutoCallRejectionInfo, number) == 16, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_3::AutoCallRejectionInfo) == 32, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_3::AutoCallRejectionInfo) == 8, "wrong alignment");

enum class VoiceInfo : int32_t {
    /**
     * Voice Info silent is sent when remote party is silent on a RTT call  */
    VOICE_INFO_SILENT = 0,
    /**
     * Voice Info speech is sent when remote party starts speaking on a RTT call  */
    VOICE_INFO_SPEECH = 1, // (::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo.VOICE_INFO_SILENT implicitly + 1)
    VOICE_INFO_UNKNOWN = 2, // (::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo.VOICE_INFO_SPEECH implicitly + 1)
};

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus lhs, const ::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus lhs, const ::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus::NOT_PROVISIONED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus::NOT_PROVISIONED)) {
        os += (first ? "" : " | ");
        os += "NOT_PROVISIONED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus::NOT_PROVISIONED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus::PROVISIONED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus::PROVISIONED)) {
        os += (first ? "" : " | ");
        os += "PROVISIONED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus::PROVISIONED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus::INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus::INVALID)) {
        os += (first ? "" : " | ");
        os += "INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus::INVALID;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus::NOT_PROVISIONED) {
        return "NOT_PROVISIONED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus::PROVISIONED) {
        return "PROVISIONED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus::INVALID) {
        return "INVALID";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_3::SuppServiceStatus& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".status = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.status);
    os += ", .provisionStatus = ";
    os += ::vendor::qti::hardware::radio::ims::V1_3::toString(o.provisionStatus);
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

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_3::SuppServiceStatus& lhs, const ::vendor::qti::hardware::radio::ims::V1_3::SuppServiceStatus& rhs) {
    if (lhs.status != rhs.status) {
        return false;
    }
    if (lhs.provisionStatus != rhs.provisionStatus) {
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

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_3::SuppServiceStatus& lhs,const ::vendor::qti::hardware::radio::ims::V1_3::SuppServiceStatus& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_3::ColrInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".status = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.status);
    os += ", .provisionStatus = ";
    os += ::vendor::qti::hardware::radio::ims::V1_3::toString(o.provisionStatus);
    os += ", .presentation = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.presentation);
    os += ", .hasErrorDetails = ";
    os += ::android::hardware::toString(o.hasErrorDetails);
    os += ", .errorDetails = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.errorDetails);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_3::ColrInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_3::ColrInfo& rhs) {
    if (lhs.status != rhs.status) {
        return false;
    }
    if (lhs.provisionStatus != rhs.provisionStatus) {
        return false;
    }
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

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_3::ColrInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_3::ColrInfo& rhs){
    return !(lhs == rhs);
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause lhs, const ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause lhs, const ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_3::CallFailCause>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_3::CallFailCause> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_UNOBTAINABLE_NUMBER) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_UNOBTAINABLE_NUMBER)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_UNOBTAINABLE_NUMBER";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_UNOBTAINABLE_NUMBER;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NORMAL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NORMAL)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_NORMAL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NORMAL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_BUSY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_BUSY)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_BUSY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_BUSY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NORMAL_UNSPECIFIED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NORMAL_UNSPECIFIED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_NORMAL_UNSPECIFIED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NORMAL_UNSPECIFIED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CONGESTION) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CONGESTION)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CONGESTION";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CONGESTION;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_INCOMPATIBILITY_DESTINATION) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_INCOMPATIBILITY_DESTINATION)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_INCOMPATIBILITY_DESTINATION";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_INCOMPATIBILITY_DESTINATION;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_BARRED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_BARRED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CALL_BARRED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_BARRED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_FDN_BLOCKED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_FDN_BLOCKED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_FDN_BLOCKED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_FDN_BLOCKED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_IMEI_NOT_ACCEPTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_IMEI_NOT_ACCEPTED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_IMEI_NOT_ACCEPTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_IMEI_NOT_ACCEPTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_USSD) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_USSD)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_MODIFIED_TO_USSD";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_USSD;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_SS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_SS)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_MODIFIED_TO_SS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_SS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_MODIFIED_TO_DIAL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_EMERGENCY_TEMP_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_EMERGENCY_TEMP_FAILURE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_EMERGENCY_TEMP_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_EMERGENCY_TEMP_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_EMERGENCY_PERM_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_EMERGENCY_PERM_FAILURE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_EMERGENCY_PERM_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_EMERGENCY_PERM_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_HO_NOT_FEASIBLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_HO_NOT_FEASIBLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_HO_NOT_FEASIBLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_HO_NOT_FEASIBLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_USER_BUSY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_USER_BUSY)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_USER_BUSY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_USER_BUSY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_USER_REJECT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_USER_REJECT)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_USER_REJECT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_USER_REJECT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_LOW_BATTERY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_LOW_BATTERY)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_LOW_BATTERY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_LOW_BATTERY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_BLACKLISTED_CALL_ID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_BLACKLISTED_CALL_ID)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_BLACKLISTED_CALL_ID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_BLACKLISTED_CALL_ID;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CS_RETRY_REQUIRED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CS_RETRY_REQUIRED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CS_RETRY_REQUIRED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CS_RETRY_REQUIRED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NETWORK_UNAVAILABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NETWORK_UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_NETWORK_UNAVAILABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NETWORK_UNAVAILABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_FEATURE_UNAVAILABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_FEATURE_UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_FEATURE_UNAVAILABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_FEATURE_UNAVAILABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_ERROR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_ERROR)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_ERROR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_ERROR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MISC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MISC)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_MISC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MISC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_ANSWERED_ELSEWHERE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_ANSWERED_ELSEWHERE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_ANSWERED_ELSEWHERE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_ANSWERED_ELSEWHERE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_PULL_OUT_OF_SYNC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_PULL_OUT_OF_SYNC)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_PULL_OUT_OF_SYNC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_PULL_OUT_OF_SYNC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CAUSE_CALL_PULLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CAUSE_CALL_PULLED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CAUSE_CALL_PULLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CAUSE_CALL_PULLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_ACCESS_CLASS_BLOCKED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_ACCESS_CLASS_BLOCKED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_ACCESS_CLASS_BLOCKED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_ACCESS_CLASS_BLOCKED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_REDIRECTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_REDIRECTED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_REDIRECTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_REDIRECTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_BAD_REQUEST) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_BAD_REQUEST)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_BAD_REQUEST";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_BAD_REQUEST;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_FORBIDDEN) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_FORBIDDEN)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_FORBIDDEN";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_FORBIDDEN;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_NOT_FOUND) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_NOT_FOUND)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_NOT_FOUND";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_NOT_FOUND;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_NOT_SUPPORTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_NOT_SUPPORTED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_NOT_SUPPORTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_NOT_SUPPORTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_REQUEST_TIMEOUT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_REQUEST_TIMEOUT)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_REQUEST_TIMEOUT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_REQUEST_TIMEOUT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_BAD_ADDRESS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_BAD_ADDRESS)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_BAD_ADDRESS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_BAD_ADDRESS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_BUSY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_BUSY)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_BUSY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_BUSY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_REQUEST_CANCELLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_REQUEST_CANCELLED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_REQUEST_CANCELLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_REQUEST_CANCELLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_NOT_ACCEPTABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_NOT_ACCEPTABLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_NOT_ACCEPTABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_NOT_ACCEPTABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_NOT_REACHABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_NOT_REACHABLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_NOT_REACHABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_NOT_REACHABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_INTERNAL_ERROR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_INTERNAL_ERROR)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVER_INTERNAL_ERROR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_INTERNAL_ERROR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_BAD_GATEWAY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_BAD_GATEWAY)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVER_BAD_GATEWAY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_BAD_GATEWAY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVICE_UNAVAILABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVICE_UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVICE_UNAVAILABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVICE_UNAVAILABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_TIMEOUT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_TIMEOUT)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVER_TIMEOUT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_TIMEOUT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_USER_REJECTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_USER_REJECTED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_USER_REJECTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_USER_REJECTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_GLOBAL_ERROR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_GLOBAL_ERROR)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_GLOBAL_ERROR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_GLOBAL_ERROR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MEDIA_INIT_FAILED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MEDIA_INIT_FAILED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_MEDIA_INIT_FAILED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MEDIA_INIT_FAILED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MEDIA_NO_DATA) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MEDIA_NO_DATA)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_MEDIA_NO_DATA";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MEDIA_NO_DATA;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MEDIA_NOT_ACCEPTABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MEDIA_NOT_ACCEPTABLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_MEDIA_NOT_ACCEPTABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MEDIA_NOT_ACCEPTABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MEDIA_UNSPECIFIED_ERROR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MEDIA_UNSPECIFIED_ERROR)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_MEDIA_UNSPECIFIED_ERROR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MEDIA_UNSPECIFIED_ERROR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_HOLD_RESUME_FAILED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_HOLD_RESUME_FAILED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_HOLD_RESUME_FAILED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_HOLD_RESUME_FAILED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_HOLD_RESUME_CANCELED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_HOLD_RESUME_CANCELED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_HOLD_RESUME_CANCELED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_HOLD_RESUME_CANCELED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_HOLD_REINVITE_COLLISION) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_HOLD_REINVITE_COLLISION)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_HOLD_REINVITE_COLLISION";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_HOLD_REINVITE_COLLISION;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_ERROR_UNSPECIFIED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_ERROR_UNSPECIFIED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_ERROR_UNSPECIFIED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_ERROR_UNSPECIFIED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_INVALID)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_INVALID;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_ALTERNATE_EMERGENCY_CALL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_ALTERNATE_EMERGENCY_CALL)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_ALTERNATE_EMERGENCY_CALL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_ALTERNATE_EMERGENCY_CALL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_OFF) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_OFF)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_RADIO_OFF";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_OFF;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OUT_OF_SERVICE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OUT_OF_SERVICE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OUT_OF_SERVICE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OUT_OF_SERVICE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NO_VALID_SIM) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NO_VALID_SIM)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_NO_VALID_SIM";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NO_VALID_SIM;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_INTERNAL_ERROR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_INTERNAL_ERROR)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_RADIO_INTERNAL_ERROR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_INTERNAL_ERROR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NETWORK_RESP_TIMEOUT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NETWORK_RESP_TIMEOUT)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_NETWORK_RESP_TIMEOUT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NETWORK_RESP_TIMEOUT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NETWORK_REJECT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NETWORK_REJECT)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_NETWORK_REJECT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NETWORK_REJECT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_ACCESS_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_ACCESS_FAILURE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_RADIO_ACCESS_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_ACCESS_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_LINK_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_LINK_FAILURE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_RADIO_LINK_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_LINK_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_LINK_LOST) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_LINK_LOST)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_RADIO_LINK_LOST";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_LINK_LOST;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_UPLINK_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_UPLINK_FAILURE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_RADIO_UPLINK_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_UPLINK_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_SETUP_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_SETUP_FAILURE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_RADIO_SETUP_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_SETUP_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_RELEASE_NORMAL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_RELEASE_NORMAL)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_RADIO_RELEASE_NORMAL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_RELEASE_NORMAL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_RELEASE_ABNORMAL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_RELEASE_ABNORMAL)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_RADIO_RELEASE_ABNORMAL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_RELEASE_ABNORMAL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NETWORK_DETACH) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NETWORK_DETACH)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_NETWORK_DETACH";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NETWORK_DETACH;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_1) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_1)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_1";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_1;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_2) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_2)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_2";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_2;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_3) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_3)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_3";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_3;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_4) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_4)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_4";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_4;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_5) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_5)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_5";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_5;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_6) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_6)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_6";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_6;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_7) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_7)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_7";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_7;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_8) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_8)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_8";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_8;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_9) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_9)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_9";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_9;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_10) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_10)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_10";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_10;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_11) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_11)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_11";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_11;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_12) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_12)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_12";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_12;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_13) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_13)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_13";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_13;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_14) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_14)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_14";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_14;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_15) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_15)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_15";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_15;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NO_CSFB_IN_CS_ROAM) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NO_CSFB_IN_CS_ROAM)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_NO_CSFB_IN_CS_ROAM";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NO_CSFB_IN_CS_ROAM;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SRV_NOT_REGISTERED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SRV_NOT_REGISTERED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SRV_NOT_REGISTERED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SRV_NOT_REGISTERED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_TYPE_NOT_ALLOWED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_TYPE_NOT_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CALL_TYPE_NOT_ALLOWED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_TYPE_NOT_ALLOWED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_EMRG_CALL_ONGOING) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_EMRG_CALL_ONGOING)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_EMRG_CALL_ONGOING";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_EMRG_CALL_ONGOING;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_SETUP_ONGOING) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_SETUP_ONGOING)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CALL_SETUP_ONGOING";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_SETUP_ONGOING;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MAX_CALL_LIMIT_REACHED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MAX_CALL_LIMIT_REACHED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_MAX_CALL_LIMIT_REACHED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MAX_CALL_LIMIT_REACHED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_UNSUPPORTED_SIP_HDRS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_UNSUPPORTED_SIP_HDRS)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_UNSUPPORTED_SIP_HDRS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_UNSUPPORTED_SIP_HDRS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_TRANSFER_ONGOING) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_TRANSFER_ONGOING)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CALL_TRANSFER_ONGOING";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_TRANSFER_ONGOING;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_PRACK_TIMEOUT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_PRACK_TIMEOUT)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_PRACK_TIMEOUT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_PRACK_TIMEOUT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_QOS_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_QOS_FAILURE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_QOS_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_QOS_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_ONGOING_HANDOVER) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_ONGOING_HANDOVER)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_ONGOING_HANDOVER";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_ONGOING_HANDOVER;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_VT_WITH_TTY_NOT_ALLOWED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_VT_WITH_TTY_NOT_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_VT_WITH_TTY_NOT_ALLOWED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_VT_WITH_TTY_NOT_ALLOWED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_UPGRADE_ONGOING) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_UPGRADE_ONGOING)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CALL_UPGRADE_ONGOING";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_UPGRADE_ONGOING;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CONFERENCE_WITH_TTY_NOT_ALLOWED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CONFERENCE_WITH_TTY_NOT_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CONFERENCE_WITH_TTY_NOT_ALLOWED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CONFERENCE_WITH_TTY_NOT_ALLOWED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_CONFERENCE_ONGOING) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_CONFERENCE_ONGOING)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CALL_CONFERENCE_ONGOING";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_CONFERENCE_ONGOING;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_VT_WITH_AVPF_NOT_ALLOWED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_VT_WITH_AVPF_NOT_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_VT_WITH_AVPF_NOT_ALLOWED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_VT_WITH_AVPF_NOT_ALLOWED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_ENCRYPTION_CALL_ONGOING) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_ENCRYPTION_CALL_ONGOING)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_ENCRYPTION_CALL_ONGOING";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_ENCRYPTION_CALL_ONGOING;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_ONGOING_CW_DISABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_ONGOING_CW_DISABLED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CALL_ONGOING_CW_DISABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_ONGOING_CW_DISABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_ON_OTHER_SUB) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_ON_OTHER_SUB)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CALL_ON_OTHER_SUB";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_ON_OTHER_SUB;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_1X_COLLISION) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_1X_COLLISION)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_1X_COLLISION";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_1X_COLLISION;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_UI_NOT_READY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_UI_NOT_READY)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_UI_NOT_READY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_UI_NOT_READY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CS_CALL_ONGOING) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CS_CALL_ONGOING)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CS_CALL_ONGOING";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CS_CALL_ONGOING;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_USER_MARKED_UNWANTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_USER_MARKED_UNWANTED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_USER_MARKED_UNWANTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_USER_MARKED_UNWANTED;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_3::CallFailCause o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_UNOBTAINABLE_NUMBER) {
        return "CALL_FAIL_UNOBTAINABLE_NUMBER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NORMAL) {
        return "CALL_FAIL_NORMAL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_BUSY) {
        return "CALL_FAIL_BUSY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NORMAL_UNSPECIFIED) {
        return "CALL_FAIL_NORMAL_UNSPECIFIED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CONGESTION) {
        return "CALL_FAIL_CONGESTION";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_INCOMPATIBILITY_DESTINATION) {
        return "CALL_FAIL_INCOMPATIBILITY_DESTINATION";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_BARRED) {
        return "CALL_FAIL_CALL_BARRED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_FDN_BLOCKED) {
        return "CALL_FAIL_FDN_BLOCKED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_IMEI_NOT_ACCEPTED) {
        return "CALL_FAIL_IMEI_NOT_ACCEPTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_USSD) {
        return "CALL_FAIL_DIAL_MODIFIED_TO_USSD";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_SS) {
        return "CALL_FAIL_DIAL_MODIFIED_TO_SS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL) {
        return "CALL_FAIL_DIAL_MODIFIED_TO_DIAL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_EMERGENCY_TEMP_FAILURE) {
        return "CALL_FAIL_EMERGENCY_TEMP_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_EMERGENCY_PERM_FAILURE) {
        return "CALL_FAIL_EMERGENCY_PERM_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_HO_NOT_FEASIBLE) {
        return "CALL_FAIL_HO_NOT_FEASIBLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_USER_BUSY) {
        return "CALL_FAIL_USER_BUSY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_USER_REJECT) {
        return "CALL_FAIL_USER_REJECT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_LOW_BATTERY) {
        return "CALL_FAIL_LOW_BATTERY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_BLACKLISTED_CALL_ID) {
        return "CALL_FAIL_BLACKLISTED_CALL_ID";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CS_RETRY_REQUIRED) {
        return "CALL_FAIL_CS_RETRY_REQUIRED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NETWORK_UNAVAILABLE) {
        return "CALL_FAIL_NETWORK_UNAVAILABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_FEATURE_UNAVAILABLE) {
        return "CALL_FAIL_FEATURE_UNAVAILABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_ERROR) {
        return "CALL_FAIL_SIP_ERROR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MISC) {
        return "CALL_FAIL_MISC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_ANSWERED_ELSEWHERE) {
        return "CALL_FAIL_ANSWERED_ELSEWHERE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_PULL_OUT_OF_SYNC) {
        return "CALL_FAIL_PULL_OUT_OF_SYNC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CAUSE_CALL_PULLED) {
        return "CALL_FAIL_CAUSE_CALL_PULLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_ACCESS_CLASS_BLOCKED) {
        return "CALL_FAIL_ACCESS_CLASS_BLOCKED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_REDIRECTED) {
        return "CALL_FAIL_SIP_REDIRECTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_BAD_REQUEST) {
        return "CALL_FAIL_SIP_BAD_REQUEST";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_FORBIDDEN) {
        return "CALL_FAIL_SIP_FORBIDDEN";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_NOT_FOUND) {
        return "CALL_FAIL_SIP_NOT_FOUND";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_NOT_SUPPORTED) {
        return "CALL_FAIL_SIP_NOT_SUPPORTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_REQUEST_TIMEOUT) {
        return "CALL_FAIL_SIP_REQUEST_TIMEOUT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE) {
        return "CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_BAD_ADDRESS) {
        return "CALL_FAIL_SIP_BAD_ADDRESS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_BUSY) {
        return "CALL_FAIL_SIP_BUSY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_REQUEST_CANCELLED) {
        return "CALL_FAIL_SIP_REQUEST_CANCELLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_NOT_ACCEPTABLE) {
        return "CALL_FAIL_SIP_NOT_ACCEPTABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_NOT_REACHABLE) {
        return "CALL_FAIL_SIP_NOT_REACHABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_INTERNAL_ERROR) {
        return "CALL_FAIL_SIP_SERVER_INTERNAL_ERROR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED) {
        return "CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_BAD_GATEWAY) {
        return "CALL_FAIL_SIP_SERVER_BAD_GATEWAY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVICE_UNAVAILABLE) {
        return "CALL_FAIL_SIP_SERVICE_UNAVAILABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_TIMEOUT) {
        return "CALL_FAIL_SIP_SERVER_TIMEOUT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED) {
        return "CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE) {
        return "CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE) {
        return "CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_USER_REJECTED) {
        return "CALL_FAIL_SIP_USER_REJECTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_GLOBAL_ERROR) {
        return "CALL_FAIL_SIP_GLOBAL_ERROR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MEDIA_INIT_FAILED) {
        return "CALL_FAIL_MEDIA_INIT_FAILED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MEDIA_NO_DATA) {
        return "CALL_FAIL_MEDIA_NO_DATA";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MEDIA_NOT_ACCEPTABLE) {
        return "CALL_FAIL_MEDIA_NOT_ACCEPTABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MEDIA_UNSPECIFIED_ERROR) {
        return "CALL_FAIL_MEDIA_UNSPECIFIED_ERROR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_HOLD_RESUME_FAILED) {
        return "CALL_FAIL_HOLD_RESUME_FAILED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_HOLD_RESUME_CANCELED) {
        return "CALL_FAIL_HOLD_RESUME_CANCELED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_HOLD_REINVITE_COLLISION) {
        return "CALL_FAIL_HOLD_REINVITE_COLLISION";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO) {
        return "CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL) {
        return "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO) {
        return "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS) {
        return "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD) {
        return "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_ERROR_UNSPECIFIED) {
        return "CALL_FAIL_ERROR_UNSPECIFIED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_INVALID) {
        return "CALL_FAIL_INVALID";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_ALTERNATE_EMERGENCY_CALL) {
        return "CALL_FAIL_SIP_ALTERNATE_EMERGENCY_CALL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_OFF) {
        return "CALL_FAIL_RADIO_OFF";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OUT_OF_SERVICE) {
        return "CALL_FAIL_OUT_OF_SERVICE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NO_VALID_SIM) {
        return "CALL_FAIL_NO_VALID_SIM";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_INTERNAL_ERROR) {
        return "CALL_FAIL_RADIO_INTERNAL_ERROR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NETWORK_RESP_TIMEOUT) {
        return "CALL_FAIL_NETWORK_RESP_TIMEOUT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NETWORK_REJECT) {
        return "CALL_FAIL_NETWORK_REJECT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_ACCESS_FAILURE) {
        return "CALL_FAIL_RADIO_ACCESS_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_LINK_FAILURE) {
        return "CALL_FAIL_RADIO_LINK_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_LINK_LOST) {
        return "CALL_FAIL_RADIO_LINK_LOST";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_UPLINK_FAILURE) {
        return "CALL_FAIL_RADIO_UPLINK_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_SETUP_FAILURE) {
        return "CALL_FAIL_RADIO_SETUP_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_RELEASE_NORMAL) {
        return "CALL_FAIL_RADIO_RELEASE_NORMAL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_RELEASE_ABNORMAL) {
        return "CALL_FAIL_RADIO_RELEASE_ABNORMAL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NETWORK_DETACH) {
        return "CALL_FAIL_NETWORK_DETACH";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_1) {
        return "CALL_FAIL_OEM_CAUSE_1";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_2) {
        return "CALL_FAIL_OEM_CAUSE_2";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_3) {
        return "CALL_FAIL_OEM_CAUSE_3";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_4) {
        return "CALL_FAIL_OEM_CAUSE_4";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_5) {
        return "CALL_FAIL_OEM_CAUSE_5";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_6) {
        return "CALL_FAIL_OEM_CAUSE_6";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_7) {
        return "CALL_FAIL_OEM_CAUSE_7";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_8) {
        return "CALL_FAIL_OEM_CAUSE_8";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_9) {
        return "CALL_FAIL_OEM_CAUSE_9";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_10) {
        return "CALL_FAIL_OEM_CAUSE_10";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_11) {
        return "CALL_FAIL_OEM_CAUSE_11";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_12) {
        return "CALL_FAIL_OEM_CAUSE_12";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_13) {
        return "CALL_FAIL_OEM_CAUSE_13";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_14) {
        return "CALL_FAIL_OEM_CAUSE_14";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_15) {
        return "CALL_FAIL_OEM_CAUSE_15";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NO_CSFB_IN_CS_ROAM) {
        return "CALL_FAIL_NO_CSFB_IN_CS_ROAM";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SRV_NOT_REGISTERED) {
        return "CALL_FAIL_SRV_NOT_REGISTERED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_TYPE_NOT_ALLOWED) {
        return "CALL_FAIL_CALL_TYPE_NOT_ALLOWED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_EMRG_CALL_ONGOING) {
        return "CALL_FAIL_EMRG_CALL_ONGOING";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_SETUP_ONGOING) {
        return "CALL_FAIL_CALL_SETUP_ONGOING";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MAX_CALL_LIMIT_REACHED) {
        return "CALL_FAIL_MAX_CALL_LIMIT_REACHED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_UNSUPPORTED_SIP_HDRS) {
        return "CALL_FAIL_UNSUPPORTED_SIP_HDRS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_TRANSFER_ONGOING) {
        return "CALL_FAIL_CALL_TRANSFER_ONGOING";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_PRACK_TIMEOUT) {
        return "CALL_FAIL_PRACK_TIMEOUT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_QOS_FAILURE) {
        return "CALL_FAIL_QOS_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_ONGOING_HANDOVER) {
        return "CALL_FAIL_ONGOING_HANDOVER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_VT_WITH_TTY_NOT_ALLOWED) {
        return "CALL_FAIL_VT_WITH_TTY_NOT_ALLOWED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_UPGRADE_ONGOING) {
        return "CALL_FAIL_CALL_UPGRADE_ONGOING";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CONFERENCE_WITH_TTY_NOT_ALLOWED) {
        return "CALL_FAIL_CONFERENCE_WITH_TTY_NOT_ALLOWED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_CONFERENCE_ONGOING) {
        return "CALL_FAIL_CALL_CONFERENCE_ONGOING";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_VT_WITH_AVPF_NOT_ALLOWED) {
        return "CALL_FAIL_VT_WITH_AVPF_NOT_ALLOWED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_ENCRYPTION_CALL_ONGOING) {
        return "CALL_FAIL_ENCRYPTION_CALL_ONGOING";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_ONGOING_CW_DISABLED) {
        return "CALL_FAIL_CALL_ONGOING_CW_DISABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_ON_OTHER_SUB) {
        return "CALL_FAIL_CALL_ON_OTHER_SUB";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_1X_COLLISION) {
        return "CALL_FAIL_1X_COLLISION";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_UI_NOT_READY) {
        return "CALL_FAIL_UI_NOT_READY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CS_CALL_ONGOING) {
        return "CALL_FAIL_CS_CALL_ONGOING";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_USER_MARKED_UNWANTED) {
        return "CALL_FAIL_SIP_USER_MARKED_UNWANTED";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_3::CallFailCauseResponse& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".failCause = ";
    os += ::vendor::qti::hardware::radio::ims::V1_3::toString(o.failCause);
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

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_3::CallFailCauseResponse& lhs, const ::vendor::qti::hardware::radio::ims::V1_3::CallFailCauseResponse& rhs) {
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

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_3::CallFailCauseResponse& lhs,const ::vendor::qti::hardware::radio::ims::V1_3::CallFailCauseResponse& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_3::HangupRequestInfo& o) {
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
    os += ::vendor::qti::hardware::radio::ims::V1_3::toString(o.failCauseResponse);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_3::HangupRequestInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_3::HangupRequestInfo& rhs) {
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

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_3::HangupRequestInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_3::HangupRequestInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_3::VerstatInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".canMarkUnwantedCall = ";
    os += ::android::hardware::toString(o.canMarkUnwantedCall);
    os += ", .verificationStatus = ";
    os += ::vendor::qti::hardware::radio::ims::V1_2::toString(o.verificationStatus);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_3::VerstatInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_3::VerstatInfo& rhs) {
    if (lhs.canMarkUnwantedCall != rhs.canMarkUnwantedCall) {
        return false;
    }
    if (lhs.verificationStatus != rhs.verificationStatus) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_3::VerstatInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_3::VerstatInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_3::CallInfo& o) {
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
    os += ::vendor::qti::hardware::radio::ims::V1_3::toString(o.failCause);
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
    os += ", .verstatInfo = ";
    os += ::vendor::qti::hardware::radio::ims::V1_3::toString(o.verstatInfo);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_3::CallInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_3::CallInfo& rhs) {
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
    if (lhs.verstatInfo != rhs.verstatInfo) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_3::CallInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_3::CallInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_3::AutoCallRejectionInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".callType = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.callType);
    os += ", .autoRejectionCause = ";
    os += ::vendor::qti::hardware::radio::ims::V1_3::toString(o.autoRejectionCause);
    os += ", .sipErrorCode = ";
    os += ::android::hardware::toString(o.sipErrorCode);
    os += ", .number = ";
    os += ::android::hardware::toString(o.number);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_3::AutoCallRejectionInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_3::AutoCallRejectionInfo& rhs) {
    if (lhs.callType != rhs.callType) {
        return false;
    }
    if (lhs.autoRejectionCause != rhs.autoRejectionCause) {
        return false;
    }
    if (lhs.sipErrorCode != rhs.sipErrorCode) {
        return false;
    }
    if (lhs.number != rhs.number) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_3::AutoCallRejectionInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_3::AutoCallRejectionInfo& rhs){
    return !(lhs == rhs);
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo lhs, const ::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo lhs, const ::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo::VOICE_INFO_SILENT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo::VOICE_INFO_SILENT)) {
        os += (first ? "" : " | ");
        os += "VOICE_INFO_SILENT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo::VOICE_INFO_SILENT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo::VOICE_INFO_SPEECH) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo::VOICE_INFO_SPEECH)) {
        os += (first ? "" : " | ");
        os += "VOICE_INFO_SPEECH";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo::VOICE_INFO_SPEECH;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo::VOICE_INFO_UNKNOWN) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo::VOICE_INFO_UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "VOICE_INFO_UNKNOWN";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo::VOICE_INFO_UNKNOWN;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo::VOICE_INFO_SILENT) {
        return "VOICE_INFO_SILENT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo::VOICE_INFO_SPEECH) {
        return "VOICE_INFO_SPEECH";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo::VOICE_INFO_UNKNOWN) {
        return "VOICE_INFO_UNKNOWN";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}


}  // namespace V1_3
}  // namespace ims
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus>
{
    const ::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus* end() { return begin() + 3; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus kVals[3] {
            ::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus::NOT_PROVISIONED,
            ::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus::PROVISIONED,
            ::vendor::qti::hardware::radio::ims::V1_3::ServiceClassProvisionStatus::INVALID,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_3::CallFailCause>
{
    const ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause* end() { return begin() + 117; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause kVals[117] {
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_UNOBTAINABLE_NUMBER,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NORMAL,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_BUSY,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NORMAL_UNSPECIFIED,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CONGESTION,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_INCOMPATIBILITY_DESTINATION,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_BARRED,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_FDN_BLOCKED,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_IMEI_NOT_ACCEPTED,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_USSD,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_SS,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_EMERGENCY_TEMP_FAILURE,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_EMERGENCY_PERM_FAILURE,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_HO_NOT_FEASIBLE,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_USER_BUSY,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_USER_REJECT,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_LOW_BATTERY,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_BLACKLISTED_CALL_ID,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CS_RETRY_REQUIRED,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NETWORK_UNAVAILABLE,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_FEATURE_UNAVAILABLE,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_ERROR,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MISC,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_ANSWERED_ELSEWHERE,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_PULL_OUT_OF_SYNC,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CAUSE_CALL_PULLED,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_ACCESS_CLASS_BLOCKED,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_REDIRECTED,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_BAD_REQUEST,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_FORBIDDEN,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_NOT_FOUND,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_NOT_SUPPORTED,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_REQUEST_TIMEOUT,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_BAD_ADDRESS,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_BUSY,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_REQUEST_CANCELLED,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_NOT_ACCEPTABLE,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_NOT_REACHABLE,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_INTERNAL_ERROR,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_BAD_GATEWAY,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVICE_UNAVAILABLE,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_TIMEOUT,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_USER_REJECTED,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_GLOBAL_ERROR,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MEDIA_INIT_FAILED,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MEDIA_NO_DATA,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MEDIA_NOT_ACCEPTABLE,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MEDIA_UNSPECIFIED_ERROR,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_HOLD_RESUME_FAILED,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_HOLD_RESUME_CANCELED,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_HOLD_REINVITE_COLLISION,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_ERROR_UNSPECIFIED,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_INVALID,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_ALTERNATE_EMERGENCY_CALL,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_OFF,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OUT_OF_SERVICE,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NO_VALID_SIM,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_INTERNAL_ERROR,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NETWORK_RESP_TIMEOUT,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NETWORK_REJECT,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_ACCESS_FAILURE,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_LINK_FAILURE,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_LINK_LOST,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_UPLINK_FAILURE,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_SETUP_FAILURE,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_RELEASE_NORMAL,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_RADIO_RELEASE_ABNORMAL,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NETWORK_DETACH,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_1,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_2,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_3,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_4,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_5,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_6,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_7,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_8,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_9,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_10,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_11,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_12,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_13,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_14,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_15,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_NO_CSFB_IN_CS_ROAM,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SRV_NOT_REGISTERED,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_TYPE_NOT_ALLOWED,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_EMRG_CALL_ONGOING,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_SETUP_ONGOING,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_MAX_CALL_LIMIT_REACHED,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_UNSUPPORTED_SIP_HDRS,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_TRANSFER_ONGOING,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_PRACK_TIMEOUT,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_QOS_FAILURE,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_ONGOING_HANDOVER,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_VT_WITH_TTY_NOT_ALLOWED,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_UPGRADE_ONGOING,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CONFERENCE_WITH_TTY_NOT_ALLOWED,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_CONFERENCE_ONGOING,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_VT_WITH_AVPF_NOT_ALLOWED,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_ENCRYPTION_CALL_ONGOING,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_ONGOING_CW_DISABLED,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CALL_ON_OTHER_SUB,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_1X_COLLISION,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_UI_NOT_READY,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_CS_CALL_ONGOING,
            ::vendor::qti::hardware::radio::ims::V1_3::CallFailCause::CALL_FAIL_SIP_USER_MARKED_UNWANTED,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo>
{
    const ::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo* end() { return begin() + 3; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo kVals[3] {
            ::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo::VOICE_INFO_SILENT,
            ::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo::VOICE_INFO_SPEECH,
            ::vendor::qti::hardware::radio::ims::V1_3::VoiceInfo::VOICE_INFO_UNKNOWN,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android

#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_3_TYPES_H
