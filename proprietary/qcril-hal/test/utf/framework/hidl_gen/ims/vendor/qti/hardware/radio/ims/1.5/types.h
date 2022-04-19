/*===========================================================================

   Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
   All Rights Reserved.
   Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_5_TYPES_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_5_TYPES_H

#include <vendor/qti/hardware/radio/ims/1.0/types.h>
#include <vendor/qti/hardware/radio/ims/1.2/types.h>
#include <vendor/qti/hardware/radio/ims/1.3/types.h>
#include <vendor/qti/hardware/radio/ims/1.4/types.h>

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_5 {

// Forward declaration for forward reference support:
enum class CallFailCause : int32_t;
enum class EmergencyServiceCategory : int32_t;
enum class TirMode : int32_t;
enum class EmergencyCallRoute : int32_t;
enum class UssdModeType : int32_t;
enum class ImsSmsSendFailureReason : int32_t;
enum class ImsSmsDeliverStatusResult : int32_t;
struct CallFailCauseResponse;
struct CallInfo;
struct AutoCallRejectionInfo;
enum class ConfigItem : int32_t;
struct ConfigInfo;
enum class Result : int32_t;
struct CallForwardStatus;
struct CallForwardStatusInfo;

enum class CallFailCause : int32_t {
    CALL_FAIL_UNOBTAINABLE_NUMBER = 0,
    CALL_FAIL_NORMAL = 1 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_UNOBTAINABLE_NUMBER implicitly + 1) */,
    CALL_FAIL_BUSY = 2 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_NORMAL implicitly + 1) */,
    CALL_FAIL_NORMAL_UNSPECIFIED = 3 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_BUSY implicitly + 1) */,
    CALL_FAIL_CONGESTION = 4 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_NORMAL_UNSPECIFIED implicitly + 1) */,
    CALL_FAIL_INCOMPATIBILITY_DESTINATION = 5 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_CONGESTION implicitly + 1) */,
    CALL_FAIL_CALL_BARRED = 6 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_INCOMPATIBILITY_DESTINATION implicitly + 1) */,
    CALL_FAIL_FDN_BLOCKED = 7 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_CALL_BARRED implicitly + 1) */,
    CALL_FAIL_IMEI_NOT_ACCEPTED = 8 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_FDN_BLOCKED implicitly + 1) */,
    CALL_FAIL_DIAL_MODIFIED_TO_USSD = 9 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_IMEI_NOT_ACCEPTED implicitly + 1) */,
    CALL_FAIL_DIAL_MODIFIED_TO_SS = 10 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_DIAL_MODIFIED_TO_USSD implicitly + 1) */,
    CALL_FAIL_DIAL_MODIFIED_TO_DIAL = 11 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_DIAL_MODIFIED_TO_SS implicitly + 1) */,
    CALL_FAIL_EMERGENCY_TEMP_FAILURE = 12 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_DIAL_MODIFIED_TO_DIAL implicitly + 1) */,
    CALL_FAIL_EMERGENCY_PERM_FAILURE = 13 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_EMERGENCY_TEMP_FAILURE implicitly + 1) */,
    CALL_FAIL_HO_NOT_FEASIBLE = 14 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_EMERGENCY_PERM_FAILURE implicitly + 1) */,
    CALL_FAIL_USER_BUSY = 501,
    CALL_FAIL_USER_REJECT = 502,
    CALL_FAIL_LOW_BATTERY = 503,
    CALL_FAIL_BLACKLISTED_CALL_ID = 504 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_LOW_BATTERY implicitly + 1) */,
    CALL_FAIL_CS_RETRY_REQUIRED = 505 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_BLACKLISTED_CALL_ID implicitly + 1) */,
    CALL_FAIL_NETWORK_UNAVAILABLE = 506 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_CS_RETRY_REQUIRED implicitly + 1) */,
    CALL_FAIL_FEATURE_UNAVAILABLE = 507 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_NETWORK_UNAVAILABLE implicitly + 1) */,
    CALL_FAIL_SIP_ERROR = 508 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_FEATURE_UNAVAILABLE implicitly + 1) */,
    CALL_FAIL_MISC = 509 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_ERROR implicitly + 1) */,
    CALL_FAIL_ANSWERED_ELSEWHERE = 510 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_MISC implicitly + 1) */,
    CALL_FAIL_PULL_OUT_OF_SYNC = 511 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_ANSWERED_ELSEWHERE implicitly + 1) */,
    CALL_FAIL_CAUSE_CALL_PULLED = 512 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_PULL_OUT_OF_SYNC implicitly + 1) */,
    CALL_FAIL_ACCESS_CLASS_BLOCKED = 513 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_CAUSE_CALL_PULLED implicitly + 1) */,
    CALL_FAIL_SIP_REDIRECTED = 514 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_ACCESS_CLASS_BLOCKED implicitly + 1) */,
    CALL_FAIL_SIP_BAD_REQUEST = 515 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_REDIRECTED implicitly + 1) */,
    CALL_FAIL_SIP_FORBIDDEN = 516 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_BAD_REQUEST implicitly + 1) */,
    CALL_FAIL_SIP_NOT_FOUND = 517 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_FORBIDDEN implicitly + 1) */,
    CALL_FAIL_SIP_NOT_SUPPORTED = 518 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_NOT_FOUND implicitly + 1) */,
    CALL_FAIL_SIP_REQUEST_TIMEOUT = 519 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_NOT_SUPPORTED implicitly + 1) */,
    CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE = 520 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_REQUEST_TIMEOUT implicitly + 1) */,
    CALL_FAIL_SIP_BAD_ADDRESS = 521 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE implicitly + 1) */,
    CALL_FAIL_SIP_BUSY = 522 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_BAD_ADDRESS implicitly + 1) */,
    CALL_FAIL_SIP_REQUEST_CANCELLED = 523 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_BUSY implicitly + 1) */,
    CALL_FAIL_SIP_NOT_ACCEPTABLE = 524 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_REQUEST_CANCELLED implicitly + 1) */,
    CALL_FAIL_SIP_NOT_REACHABLE = 525 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_NOT_ACCEPTABLE implicitly + 1) */,
    CALL_FAIL_SIP_SERVER_INTERNAL_ERROR = 526 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_NOT_REACHABLE implicitly + 1) */,
    CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED = 527 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_SERVER_INTERNAL_ERROR implicitly + 1) */,
    CALL_FAIL_SIP_SERVER_BAD_GATEWAY = 528 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED implicitly + 1) */,
    CALL_FAIL_SIP_SERVICE_UNAVAILABLE = 529 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_SERVER_BAD_GATEWAY implicitly + 1) */,
    CALL_FAIL_SIP_SERVER_TIMEOUT = 530 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_SERVICE_UNAVAILABLE implicitly + 1) */,
    CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED = 531 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_SERVER_TIMEOUT implicitly + 1) */,
    CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE = 532 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED implicitly + 1) */,
    CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE = 533 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE implicitly + 1) */,
    CALL_FAIL_SIP_USER_REJECTED = 534 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE implicitly + 1) */,
    CALL_FAIL_SIP_GLOBAL_ERROR = 535 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_USER_REJECTED implicitly + 1) */,
    CALL_FAIL_MEDIA_INIT_FAILED = 536 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_SIP_GLOBAL_ERROR implicitly + 1) */,
    CALL_FAIL_MEDIA_NO_DATA = 537 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_MEDIA_INIT_FAILED implicitly + 1) */,
    CALL_FAIL_MEDIA_NOT_ACCEPTABLE = 538 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_MEDIA_NO_DATA implicitly + 1) */,
    CALL_FAIL_MEDIA_UNSPECIFIED_ERROR = 539 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_MEDIA_NOT_ACCEPTABLE implicitly + 1) */,
    CALL_FAIL_HOLD_RESUME_FAILED = 540 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_MEDIA_UNSPECIFIED_ERROR implicitly + 1) */,
    CALL_FAIL_HOLD_RESUME_CANCELED = 541 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_HOLD_RESUME_FAILED implicitly + 1) */,
    CALL_FAIL_HOLD_REINVITE_COLLISION = 542 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_HOLD_RESUME_CANCELED implicitly + 1) */,
    CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO = 543 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_HOLD_REINVITE_COLLISION implicitly + 1) */,
    CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL = 544 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO implicitly + 1) */,
    CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO = 545 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL implicitly + 1) */,
    CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS = 546 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO implicitly + 1) */,
    CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD = 547 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS implicitly + 1) */,
    CALL_FAIL_ERROR_UNSPECIFIED = 548 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD implicitly + 1) */,
    CALL_FAIL_INVALID = 549 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_ERROR_UNSPECIFIED implicitly + 1) */,
    CALL_FAIL_SIP_ALTERNATE_EMERGENCY_CALL = 550 /* (::vendor::qti::hardware::radio::ims::V1_0::CallFailCause.CALL_FAIL_INVALID implicitly + 1) */,
    CALL_FAIL_RADIO_OFF = 551 /* (::vendor::qti::hardware::radio::ims::V1_1::CallFailCause.CALL_FAIL_SIP_ALTERNATE_EMERGENCY_CALL implicitly + 1) */,
    CALL_FAIL_OUT_OF_SERVICE = 552 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_RADIO_OFF implicitly + 1) */,
    CALL_FAIL_NO_VALID_SIM = 553 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OUT_OF_SERVICE implicitly + 1) */,
    CALL_FAIL_RADIO_INTERNAL_ERROR = 554 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_NO_VALID_SIM implicitly + 1) */,
    CALL_FAIL_NETWORK_RESP_TIMEOUT = 555 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_RADIO_INTERNAL_ERROR implicitly + 1) */,
    CALL_FAIL_NETWORK_REJECT = 556 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_NETWORK_RESP_TIMEOUT implicitly + 1) */,
    CALL_FAIL_RADIO_ACCESS_FAILURE = 557 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_NETWORK_REJECT implicitly + 1) */,
    CALL_FAIL_RADIO_LINK_FAILURE = 558 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_RADIO_ACCESS_FAILURE implicitly + 1) */,
    CALL_FAIL_RADIO_LINK_LOST = 559 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_RADIO_LINK_FAILURE implicitly + 1) */,
    CALL_FAIL_RADIO_UPLINK_FAILURE = 560 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_RADIO_LINK_LOST implicitly + 1) */,
    CALL_FAIL_RADIO_SETUP_FAILURE = 561 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_RADIO_UPLINK_FAILURE implicitly + 1) */,
    CALL_FAIL_RADIO_RELEASE_NORMAL = 562 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_RADIO_SETUP_FAILURE implicitly + 1) */,
    CALL_FAIL_RADIO_RELEASE_ABNORMAL = 563 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_RADIO_RELEASE_NORMAL implicitly + 1) */,
    CALL_FAIL_NETWORK_DETACH = 564 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_RADIO_RELEASE_ABNORMAL implicitly + 1) */,
    CALL_FAIL_OEM_CAUSE_1 = 565 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_NETWORK_DETACH implicitly + 1) */,
    CALL_FAIL_OEM_CAUSE_2 = 566 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_1 implicitly + 1) */,
    CALL_FAIL_OEM_CAUSE_3 = 567 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_2 implicitly + 1) */,
    CALL_FAIL_OEM_CAUSE_4 = 568 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_3 implicitly + 1) */,
    CALL_FAIL_OEM_CAUSE_5 = 569 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_4 implicitly + 1) */,
    CALL_FAIL_OEM_CAUSE_6 = 570 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_5 implicitly + 1) */,
    CALL_FAIL_OEM_CAUSE_7 = 571 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_6 implicitly + 1) */,
    CALL_FAIL_OEM_CAUSE_8 = 572 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_7 implicitly + 1) */,
    CALL_FAIL_OEM_CAUSE_9 = 573 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_8 implicitly + 1) */,
    CALL_FAIL_OEM_CAUSE_10 = 574 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_9 implicitly + 1) */,
    CALL_FAIL_OEM_CAUSE_11 = 575 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_10 implicitly + 1) */,
    CALL_FAIL_OEM_CAUSE_12 = 576 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_11 implicitly + 1) */,
    CALL_FAIL_OEM_CAUSE_13 = 577 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_12 implicitly + 1) */,
    CALL_FAIL_OEM_CAUSE_14 = 578 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_13 implicitly + 1) */,
    CALL_FAIL_OEM_CAUSE_15 = 579 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_14 implicitly + 1) */,
    CALL_FAIL_NO_CSFB_IN_CS_ROAM = 580 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_OEM_CAUSE_15 implicitly + 1) */,
    CALL_FAIL_SRV_NOT_REGISTERED = 581 /* (::vendor::qti::hardware::radio::ims::V1_2::CallFailCause.CALL_FAIL_NO_CSFB_IN_CS_ROAM implicitly + 1) */,
    CALL_FAIL_CALL_TYPE_NOT_ALLOWED = 582 /* (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_SRV_NOT_REGISTERED implicitly + 1) */,
    CALL_FAIL_EMRG_CALL_ONGOING = 583 /* (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_CALL_TYPE_NOT_ALLOWED implicitly + 1) */,
    CALL_FAIL_CALL_SETUP_ONGOING = 584 /* (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_EMRG_CALL_ONGOING implicitly + 1) */,
    CALL_FAIL_MAX_CALL_LIMIT_REACHED = 585 /* (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_CALL_SETUP_ONGOING implicitly + 1) */,
    CALL_FAIL_UNSUPPORTED_SIP_HDRS = 586 /* (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_MAX_CALL_LIMIT_REACHED implicitly + 1) */,
    CALL_FAIL_CALL_TRANSFER_ONGOING = 587 /* (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_UNSUPPORTED_SIP_HDRS implicitly + 1) */,
    CALL_FAIL_PRACK_TIMEOUT = 588 /* (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_CALL_TRANSFER_ONGOING implicitly + 1) */,
    CALL_FAIL_QOS_FAILURE = 589 /* (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_PRACK_TIMEOUT implicitly + 1) */,
    CALL_FAIL_ONGOING_HANDOVER = 590 /* (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_QOS_FAILURE implicitly + 1) */,
    CALL_FAIL_VT_WITH_TTY_NOT_ALLOWED = 591 /* (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_ONGOING_HANDOVER implicitly + 1) */,
    CALL_FAIL_CALL_UPGRADE_ONGOING = 592 /* (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_VT_WITH_TTY_NOT_ALLOWED implicitly + 1) */,
    CALL_FAIL_CONFERENCE_WITH_TTY_NOT_ALLOWED = 593 /* (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_CALL_UPGRADE_ONGOING implicitly + 1) */,
    CALL_FAIL_CALL_CONFERENCE_ONGOING = 594 /* (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_CONFERENCE_WITH_TTY_NOT_ALLOWED implicitly + 1) */,
    CALL_FAIL_VT_WITH_AVPF_NOT_ALLOWED = 595 /* (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_CALL_CONFERENCE_ONGOING implicitly + 1) */,
    CALL_FAIL_ENCRYPTION_CALL_ONGOING = 596 /* (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_VT_WITH_AVPF_NOT_ALLOWED implicitly + 1) */,
    CALL_FAIL_CALL_ONGOING_CW_DISABLED = 597 /* (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_ENCRYPTION_CALL_ONGOING implicitly + 1) */,
    CALL_FAIL_CALL_ON_OTHER_SUB = 598 /* (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_CALL_ONGOING_CW_DISABLED implicitly + 1) */,
    CALL_FAIL_1X_COLLISION = 599 /* (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_CALL_ON_OTHER_SUB implicitly + 1) */,
    CALL_FAIL_UI_NOT_READY = 600 /* (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_1X_COLLISION implicitly + 1) */,
    CALL_FAIL_CS_CALL_ONGOING = 601 /* (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_UI_NOT_READY implicitly + 1) */,
    CALL_FAIL_SIP_USER_MARKED_UNWANTED = 602 /* (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_CS_CALL_ONGOING implicitly + 1) */,
    CALL_FAIL_REJECTED_ELSEWHERE = 603 /* (::vendor::qti::hardware::radio::ims::V1_3::CallFailCause.CALL_FAIL_SIP_USER_MARKED_UNWANTED implicitly + 1) */,
    CALL_FAIL_USER_REJECTED_SESSION_MODIFICATION = 604 /* (::vendor::qti::hardware::radio::ims::V1_5::CallFailCause.CALL_FAIL_REJECTED_ELSEWHERE implicitly + 1) */,
    CALL_FAIL_USER_CANCELLED_SESSION_MODIFICATION = 605 /* (::vendor::qti::hardware::radio::ims::V1_5::CallFailCause.CALL_FAIL_USER_REJECTED_SESSION_MODIFICATION implicitly + 1) */,
    CALL_FAIL_SESSION_MODIFICATION_FAILED = 606 /* (::vendor::qti::hardware::radio::ims::V1_5::CallFailCause.CALL_FAIL_USER_CANCELLED_SESSION_MODIFICATION implicitly + 1) */,
    CALL_FAIL_SIP_METHOD_NOT_ALLOWED = 607 /* (::vendor::qti::hardware::radio::ims::V1_5::CallFailCause.CALL_FAIL_SESSION_MODIFICATION_FAILED implicitly + 1) */,
    CALL_FAIL_SIP_PROXY_AUTHENTICATION_REQUIRED = 608 /* (::vendor::qti::hardware::radio::ims::V1_5::CallFailCause.CALL_FAIL_SIP_METHOD_NOT_ALLOWED implicitly + 1) */,
    CALL_FAIL_SIP_REQUEST_ENTITY_TOO_LARGE = 609 /* (::vendor::qti::hardware::radio::ims::V1_5::CallFailCause.CALL_FAIL_SIP_PROXY_AUTHENTICATION_REQUIRED implicitly + 1) */,
    CALL_FAIL_SIP_REQUEST_URI_TOO_LARGE = 610 /* (::vendor::qti::hardware::radio::ims::V1_5::CallFailCause.CALL_FAIL_SIP_REQUEST_ENTITY_TOO_LARGE implicitly + 1) */,
    CALL_FAIL_SIP_EXTENSION_REQUIRED = 611 /* (::vendor::qti::hardware::radio::ims::V1_5::CallFailCause.CALL_FAIL_SIP_REQUEST_URI_TOO_LARGE implicitly + 1) */,
    CALL_FAIL_SIP_INTERVAL_TOO_BRIEF = 612 /* (::vendor::qti::hardware::radio::ims::V1_5::CallFailCause.CALL_FAIL_SIP_EXTENSION_REQUIRED implicitly + 1) */,
    CALL_FAIL_SIP_CALL_OR_TRANS_DOES_NOT_EXIST = 613 /* (::vendor::qti::hardware::radio::ims::V1_5::CallFailCause.CALL_FAIL_SIP_INTERVAL_TOO_BRIEF implicitly + 1) */,
    CALL_FAIL_SIP_LOOP_DETECTED = 614 /* (::vendor::qti::hardware::radio::ims::V1_5::CallFailCause.CALL_FAIL_SIP_CALL_OR_TRANS_DOES_NOT_EXIST implicitly + 1) */,
    CALL_FAIL_SIP_TOO_MANY_HOPS = 615 /* (::vendor::qti::hardware::radio::ims::V1_5::CallFailCause.CALL_FAIL_SIP_LOOP_DETECTED implicitly + 1) */,
    CALL_FAIL_SIP_AMBIGUOUS = 616 /* (::vendor::qti::hardware::radio::ims::V1_5::CallFailCause.CALL_FAIL_SIP_TOO_MANY_HOPS implicitly + 1) */,
    CALL_FAIL_SIP_REQUEST_PENDING = 617 /* (::vendor::qti::hardware::radio::ims::V1_5::CallFailCause.CALL_FAIL_SIP_AMBIGUOUS implicitly + 1) */,
    CALL_FAIL_SIP_UNDECIPHERABLE = 618 /* (::vendor::qti::hardware::radio::ims::V1_5::CallFailCause.CALL_FAIL_SIP_REQUEST_PENDING implicitly + 1) */,
    CALL_FAIL_RETRY_ON_IMS_WITHOUT_RTT = 619 /* (::vendor::qti::hardware::radio::ims::V1_5::CallFailCause.CALL_FAIL_SIP_UNDECIPHERABLE implicitly + 1) */,
};

enum class EmergencyServiceCategory : int32_t {
    /**
     * General emergency call, all categories
     */
    UNSPECIFIED = 0,
    POLICE = 1 /* (1 << 0) */,
    AMBULANCE = 2 /* (1 << 1) */,
    FIRE_BRIGADE = 4 /* (1 << 2) */,
    MARINE_GUARD = 8 /* (1 << 3) */,
    MOUNTAIN_RESCUE = 16 /* (1 << 4) */,
    /**
     * Manually Initiated eCall (MIeC)
     */
    MIEC = 32 /* (1 << 5) */,
    /**
     * Automatically Initiated eCall (AIeC)
     */
    AIEC = 64 /* (1 << 6) */,
};

enum class TirMode : int32_t {
    INVALID = 0,
    TEMPORARY = 1 /* (::vendor::qti::hardware::radio::ims::V1_5::TirMode.INVALID implicitly + 1) */,
    PERMANENT = 2 /* (::vendor::qti::hardware::radio::ims::V1_5::TirMode.TEMPORARY implicitly + 1) */,
};

enum class EmergencyCallRoute : int32_t {
    UNKNOWN = 0,
    EMERGENCY = 1,
    NORMAL = 2,
};

enum class UssdModeType : int32_t {
    NOTIFY = 0,
    REQUEST = 1 /* (::vendor::qti::hardware::radio::ims::V1_5::UssdModeType.NOTIFY implicitly + 1) */,
    NW_RELEASE = 2 /* (::vendor::qti::hardware::radio::ims::V1_5::UssdModeType.REQUEST implicitly + 1) */,
    LOCAL_CLIENT = 3 /* (::vendor::qti::hardware::radio::ims::V1_5::UssdModeType.NW_RELEASE implicitly + 1) */,
    NOT_SUPPORTED = 4 /* (::vendor::qti::hardware::radio::ims::V1_5::UssdModeType.LOCAL_CLIENT implicitly + 1) */,
    NW_TIMEOUT = 5 /* (::vendor::qti::hardware::radio::ims::V1_5::UssdModeType.NOT_SUPPORTED implicitly + 1) */,
};

enum class ImsSmsSendFailureReason : int32_t {
    RESULT_ERROR_NONE = 0,
    /**
     * Generic failure cause
     */
    RESULT_ERROR_GENERIC_FAILURE = 1 /* (::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason.RESULT_ERROR_NONE implicitly + 1) */,
    /**
     * Failed because radio was explicitly turned off
     */
    RESULT_ERROR_RADIO_OFF = 2 /* (::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason.RESULT_ERROR_GENERIC_FAILURE implicitly + 1) */,
    /**
     * Failed because no pdu provided
     */
    RESULT_ERROR_NULL_PDU = 3 /* (::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason.RESULT_ERROR_RADIO_OFF implicitly + 1) */,
    /**
     * Failed because service is currently unavailable
     */
    RESULT_ERROR_NO_SERVICE = 4 /* (::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason.RESULT_ERROR_NULL_PDU implicitly + 1) */,
    /**
     * Failed because we reached the sending queue limit.
     */
    RESULT_ERROR_LIMIT_EXCEEDED = 5 /* (::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason.RESULT_ERROR_NO_SERVICE implicitly + 1) */,
    /**
     * Failed because user denied the sending of this short code.
     */
    RESULT_ERROR_SHORT_CODE_NOT_ALLOWED = 6 /* (::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason.RESULT_ERROR_LIMIT_EXCEEDED implicitly + 1) */,
    /**
     * Failed because the user has denied this app
     * ever send premium short codes.
     */
    RESULT_ERROR_SHORT_CODE_NEVER_ALLOWED = 7 /* (::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason.RESULT_ERROR_SHORT_CODE_NOT_ALLOWED implicitly + 1) */,
    /**
     * Failed because FDN is enabled.
     */
    RESULT_ERROR_FDN_CHECK_FAILURE = 8 /* (::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason.RESULT_ERROR_SHORT_CODE_NEVER_ALLOWED implicitly + 1) */,
    /**
     * Failed because the radio was not available.
     */
    RESULT_RADIO_NOT_AVAILABLE = 9 /* (::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason.RESULT_ERROR_FDN_CHECK_FAILURE implicitly + 1) */,
    /**
     * Failed because of network rejection.
     */
    RESULT_NETWORK_REJECT = 10 /* (::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason.RESULT_RADIO_NOT_AVAILABLE implicitly + 1) */,
    /**
     * Failed because of invalid arguments.
     */
    RESULT_INVALID_ARGUMENTS = 11 /* (::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason.RESULT_NETWORK_REJECT implicitly + 1) */,
    /**
     * Failed because of an invalid state.
     */
    RESULT_INVALID_STATE = 12 /* (::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason.RESULT_INVALID_ARGUMENTS implicitly + 1) */,
    /**
     * Failed because there is no memory.
     */
    RESULT_NO_MEMORY = 13 /* (::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason.RESULT_INVALID_STATE implicitly + 1) */,
    /**
     * Failed because the sms format is not valid.
     */
    RESULT_INVALID_SMS_FORMAT = 14 /* (::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason.RESULT_NO_MEMORY implicitly + 1) */,
    /**
     * Failed because of a system error.
     */
    RESULT_SYSTEM_ERROR = 15 /* (::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason.RESULT_INVALID_SMS_FORMAT implicitly + 1) */,
    /**
     * Failed because of a modem error.
     */
    RESULT_MODEM_ERROR = 16 /* (::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason.RESULT_SYSTEM_ERROR implicitly + 1) */,
    /**
     * Failed because of a network error.
     */
    RESULT_NETWORK_ERROR = 17 /* (::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason.RESULT_MODEM_ERROR implicitly + 1) */,
    /**
     * Failed because of an encoding error.
     */
    RESULT_ENCODING_ERROR = 18 /* (::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason.RESULT_NETWORK_ERROR implicitly + 1) */,
    /**
     * Failed because of an invalid smsc address.
     */
    RESULT_INVALID_SMSC_ADDRESS = 19 /* (::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason.RESULT_ENCODING_ERROR implicitly + 1) */,
    /**
     * Failed because the operation is not allowed.
     */
    RESULT_OPERATION_NOT_ALLOWED = 20 /* (::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason.RESULT_INVALID_SMSC_ADDRESS implicitly + 1) */,
    RESULT_INTERNAL_ERROR = 21 /* (::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason.RESULT_OPERATION_NOT_ALLOWED implicitly + 1) */,
    /**
     * Failed because there are no resources.
     */
    RESULT_NO_RESOURCES = 22 /* (::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason.RESULT_INTERNAL_ERROR implicitly + 1) */,
    /**
     * Failed because the operation was cancelled.
     */
    RESULT_CANCELLED = 23 /* (::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason.RESULT_NO_RESOURCES implicitly + 1) */,
    /**
     * Failed because the request is not supported.
     */
    RESULT_REQUEST_NOT_SUPPORTED = 24 /* (::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason.RESULT_CANCELLED implicitly + 1) */,
};

enum class ImsSmsDeliverStatusResult : int32_t {
    /**
     * Message was delivered successfully.
     */
    DELIVER_STATUS_OK = 0,
    /**
     * Message was not delivered.
     */
    DELIVER_STATUS_ERROR = 1 /* (::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult.DELIVER_STATUS_OK implicitly + 1) */,
    /**
     * Message was not delivered due to lack of memory.
     */
    DELIVER_STATUS_ERROR_NO_MEMORY = 2 /* (::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult.DELIVER_STATUS_ERROR implicitly + 1) */,
    /**
     * Message was not delivered as the request is not supported.
     */
    DELIVER_STATUS_ERROR_REQUEST_NOT_SUPPORTED = 3 /* (::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult.DELIVER_STATUS_ERROR_NO_MEMORY implicitly + 1) */,
};

struct CallFailCauseResponse final {
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause failCause __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<uint8_t> errorinfo __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string networkErrorString __attribute__ ((aligned(8)));
    bool hasErrorDetails __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo errorDetails __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallFailCauseResponse, failCause) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallFailCauseResponse, errorinfo) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallFailCauseResponse, networkErrorString) == 24, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallFailCauseResponse, hasErrorDetails) == 40, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallFailCauseResponse, errorDetails) == 48, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_5::CallFailCauseResponse) == 72, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_5::CallFailCauseResponse) == 8, "wrong alignment");

struct CallInfo final {
    ::vendor::qti::hardware::radio::ims::V1_0::CallState state __attribute__ ((aligned(4)));
    uint32_t index __attribute__ ((aligned(4)));
    uint32_t toa __attribute__ ((aligned(4)));
    bool hasIsMpty __attribute__ ((aligned(1)));
    bool isMpty __attribute__ ((aligned(1)));
    bool hasIsMT __attribute__ ((aligned(1)));
    bool isMT __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineInfoHal mtMultiLineInfo __attribute__ ((aligned(8)));
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
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCauseResponse failCause __attribute__ ((aligned(8)));
    bool hasIsEncrypted __attribute__ ((aligned(1)));
    bool isEncrypted __attribute__ ((aligned(1)));
    bool hasIsCalledPartyRinging __attribute__ ((aligned(1)));
    bool isCalledPartyRinging __attribute__ ((aligned(1)));
    ::android::hardware::hidl_string historyInfo __attribute__ ((aligned(8)));
    bool hasIsVideoConfSupported __attribute__ ((aligned(1)));
    bool isVideoConfSupported __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_3::VerstatInfo verstatInfo __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_5::TirMode tirMode __attribute__ ((aligned(4)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, state) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, index) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, toa) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, hasIsMpty) == 12, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, isMpty) == 13, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, hasIsMT) == 14, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, isMT) == 15, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, mtMultiLineInfo) == 16, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, als) == 40, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, hasIsVoice) == 44, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, isVoice) == 45, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, hasIsVoicePrivacy) == 46, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, isVoicePrivacy) == 47, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, number) == 48, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, numberPresentation) == 64, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, name) == 72, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, namePresentation) == 88, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, hasCallDetails) == 92, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, callDetails) == 96, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, hasFailCause) == 192, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, failCause) == 200, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, hasIsEncrypted) == 272, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, isEncrypted) == 273, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, hasIsCalledPartyRinging) == 274, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, isCalledPartyRinging) == 275, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, historyInfo) == 280, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, hasIsVideoConfSupported) == 296, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, isVideoConfSupported) == 297, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, verstatInfo) == 300, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo, tirMode) == 308, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo) == 312, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_5::CallInfo) == 8, "wrong alignment");

struct AutoCallRejectionInfo final {
    ::vendor::qti::hardware::radio::ims::V1_0::CallType callType __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause autoRejectionCause __attribute__ ((aligned(4)));
    uint16_t sipErrorCode __attribute__ ((aligned(2)));
    ::android::hardware::hidl_string number __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::AutoCallRejectionInfo, callType) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::AutoCallRejectionInfo, autoRejectionCause) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::AutoCallRejectionInfo, sipErrorCode) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::AutoCallRejectionInfo, number) == 16, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_5::AutoCallRejectionInfo) == 32, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_5::AutoCallRejectionInfo) == 8, "wrong alignment");

enum class ConfigItem : int32_t {
    CONFIG_ITEM_NONE = 0,
    CONFIG_ITEM_VOCODER_AMRMODESET = 1 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_NONE implicitly + 1) */,
    CONFIG_ITEM_VOCODER_AMRWBMODESET = 2 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VOCODER_AMRMODESET implicitly + 1) */,
    CONFIG_ITEM_SIP_SESSION_TIMER = 3 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VOCODER_AMRWBMODESET implicitly + 1) */,
    CONFIG_ITEM_MIN_SESSION_EXPIRY = 4 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_SESSION_TIMER implicitly + 1) */,
    CONFIG_ITEM_CANCELLATION_TIMER = 5 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_MIN_SESSION_EXPIRY implicitly + 1) */,
    CONFIG_ITEM_T_DELAY = 6 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_CANCELLATION_TIMER implicitly + 1) */,
    CONFIG_ITEM_SILENT_REDIAL_ENABLE = 7 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_T_DELAY implicitly + 1) */,
    CONFIG_ITEM_SIP_T1_TIMER = 8 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SILENT_REDIAL_ENABLE implicitly + 1) */,
    CONFIG_ITEM_SIP_T2_TIMER = 9 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_T1_TIMER implicitly + 1) */,
    CONFIG_ITEM_SIP_TF_TIMER = 10 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_T2_TIMER implicitly + 1) */,
    CONFIG_ITEM_VLT_SETTING_ENABLED = 11 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_TF_TIMER implicitly + 1) */,
    CONFIG_ITEM_LVC_SETTING_ENABLED = 12 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VLT_SETTING_ENABLED implicitly + 1) */,
    CONFIG_ITEM_DOMAIN_NAME = 13 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_LVC_SETTING_ENABLED implicitly + 1) */,
    CONFIG_ITEM_SMS_FORMAT = 14 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_DOMAIN_NAME implicitly + 1) */,
    CONFIG_ITEM_SMS_OVER_IP = 15 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SMS_FORMAT implicitly + 1) */,
    CONFIG_ITEM_PUBLISH_TIMER = 16 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SMS_OVER_IP implicitly + 1) */,
    CONFIG_ITEM_PUBLISH_TIMER_EXTENDED = 17 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_PUBLISH_TIMER implicitly + 1) */,
    CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION = 18 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_PUBLISH_TIMER_EXTENDED implicitly + 1) */,
    CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION = 19 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION implicitly + 1) */,
    CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL = 20 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION implicitly + 1) */,
    CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH = 21 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL implicitly + 1) */,
    CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL = 22 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH implicitly + 1) */,
    CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP = 23 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL implicitly + 1) */,
    CONFIG_ITEM_GZIP_FLAG = 24 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP implicitly + 1) */,
    CONFIG_ITEM_EAB_SETTING_ENABLED = 25 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_GZIP_FLAG implicitly + 1) */,
    CONFIG_ITEM_MOBILE_DATA_ENABLED = 26 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_EAB_SETTING_ENABLED implicitly + 1) */,
    CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED = 27 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_MOBILE_DATA_ENABLED implicitly + 1) */,
    CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING = 28 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED implicitly + 1) */,
    CONFIG_ITEM_VOICE_OVER_WIFI_MODE = 29 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING implicitly + 1) */,
    CONFIG_ITEM_CAPABILITY_DISCOVERY_ENABLED = 30 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VOICE_OVER_WIFI_MODE implicitly + 1) */,
    CONFIG_ITEM_EMERGENCY_CALL_TIMER = 31 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_CAPABILITY_DISCOVERY_ENABLED implicitly + 1) */,
    CONFIG_ITEM_SSAC_HYSTERESIS_TIMER = 32 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_EMERGENCY_CALL_TIMER implicitly + 1) */,
    CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS = 33 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SSAC_HYSTERESIS_TIMER implicitly + 1) */,
    CONFIG_ITEM_LBO_PCSCF_ADDRESS = 34 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS implicitly + 1) */,
    CONFIG_ITEM_KEEP_ALIVE_ENABLED = 35 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_LBO_PCSCF_ADDRESS implicitly + 1) */,
    CONFIG_ITEM_REGISTRATION_RETRY_BASE_TIME_SEC = 36 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_KEEP_ALIVE_ENABLED implicitly + 1) */,
    CONFIG_ITEM_REGISTRATION_RETRY_MAX_TIME_SEC = 37 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_REGISTRATION_RETRY_BASE_TIME_SEC implicitly + 1) */,
    CONFIG_ITEM_SPEECH_START_PORT = 38 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_REGISTRATION_RETRY_MAX_TIME_SEC implicitly + 1) */,
    CONFIG_ITEM_SPEECH_END_PORT = 39 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SPEECH_START_PORT implicitly + 1) */,
    CONFIG_ITEM_SIP_INVITE_REQ_RETX_INTERVAL_MSEC = 40 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SPEECH_END_PORT implicitly + 1) */,
    CONFIG_ITEM_SIP_INVITE_RSP_WAIT_TIME_MSEC = 41 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_INVITE_REQ_RETX_INTERVAL_MSEC implicitly + 1) */,
    CONFIG_ITEM_SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC = 42 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_INVITE_RSP_WAIT_TIME_MSEC implicitly + 1) */,
    CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC = 43 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC implicitly + 1) */,
    CONFIG_ITEM_SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC = 44 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC implicitly + 1) */,
    CONFIG_ITEM_SIP_INVITE_RSP_RETX_INTERVAL_MSEC = 45 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC implicitly + 1) */,
    CONFIG_ITEM_SIP_ACK_RECEIPT_WAIT_TIME_MSEC = 46 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_INVITE_RSP_RETX_INTERVAL_MSEC implicitly + 1) */,
    CONFIG_ITEM_SIP_ACK_RETX_WAIT_TIME_MSEC = 47 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_ACK_RECEIPT_WAIT_TIME_MSEC implicitly + 1) */,
    CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC = 48 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_ACK_RETX_WAIT_TIME_MSEC implicitly + 1) */,
    CONFIG_ITEM_SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC = 49 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC implicitly + 1) */,
    CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT = 50 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC implicitly + 1) */,
    CONFIG_ITEM_AMR_WB_BANDWIDTH_EFFICIENT_PT = 51 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT implicitly + 1) */,
    CONFIG_ITEM_AMR_OCTET_ALIGNED_PT = 52 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_AMR_WB_BANDWIDTH_EFFICIENT_PT implicitly + 1) */,
    CONFIG_ITEM_AMR_BANDWIDTH_EFFICIENT_PT = 53 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_AMR_OCTET_ALIGNED_PT implicitly + 1) */,
    CONFIG_ITEM_DTMF_WB_PT = 54 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_AMR_BANDWIDTH_EFFICIENT_PT implicitly + 1) */,
    CONFIG_ITEM_DTMF_NB_PT = 55 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_DTMF_WB_PT implicitly + 1) */,
    CONFIG_ITEM_AMR_DEFAULT_MODE = 56 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_DTMF_NB_PT implicitly + 1) */,
    CONFIG_ITEM_SMS_PSI = 57 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_AMR_DEFAULT_MODE implicitly + 1) */,
    CONFIG_ITEM_VIDEO_QUALITY = 58 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SMS_PSI implicitly + 1) */,
    CONFIG_ITEM_THRESHOLD_LTE1 = 59 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VIDEO_QUALITY implicitly + 1) */,
    CONFIG_ITEM_THRESHOLD_LTE2 = 60 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_THRESHOLD_LTE1 implicitly + 1) */,
    CONFIG_ITEM_THRESHOLD_LTE3 = 61 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_THRESHOLD_LTE2 implicitly + 1) */,
    CONFIG_ITEM_THRESHOLD_1x = 62 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_THRESHOLD_LTE3 implicitly + 1) */,
    CONFIG_ITEM_THRESHOLD_WIFI_A = 63 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_THRESHOLD_1x implicitly + 1) */,
    CONFIG_ITEM_THRESHOLD_WIFI_B = 64 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_THRESHOLD_WIFI_A implicitly + 1) */,
    CONFIG_ITEM_T_EPDG_LTE = 65 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_THRESHOLD_WIFI_B implicitly + 1) */,
    CONFIG_ITEM_T_EPDG_WIFI = 66 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_T_EPDG_LTE implicitly + 1) */,
    CONFIG_ITEM_T_EPDG_1x = 67 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_T_EPDG_WIFI implicitly + 1) */,
    CONFIG_ITEM_VWF_SETTING_ENABLED = 68 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_T_EPDG_1x implicitly + 1) */,
    CONFIG_ITEM_VCE_SETTING_ENABLED = 69 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VWF_SETTING_ENABLED implicitly + 1) */,
    CONFIG_ITEM_RTT_SETTING_ENABLED = 70 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VCE_SETTING_ENABLED implicitly + 1) */,
    CONFIG_ITEM_SMS_APP = 71 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_RTT_SETTING_ENABLED implicitly + 1) */,
    CONFIG_ITEM_VVM_APP = 72 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SMS_APP implicitly + 1) */,
    CONFIG_ITEM_INVALID = 73 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VVM_APP implicitly + 1) */,
    CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE = 74 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_INVALID implicitly + 1) */,
};

struct ConfigInfo final {
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem item __attribute__ ((aligned(4)));
    bool hasBoolValue __attribute__ ((aligned(1)));
    bool boolValue __attribute__ ((aligned(1)));
    uint32_t intValue __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string stringValue __attribute__ ((aligned(8)));
    ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause errorCause __attribute__ ((aligned(4)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::ConfigInfo, item) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::ConfigInfo, hasBoolValue) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::ConfigInfo, boolValue) == 5, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::ConfigInfo, intValue) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::ConfigInfo, stringValue) == 16, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::ConfigInfo, errorCause) == 32, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_5::ConfigInfo) == 40, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_5::ConfigInfo) == 8, "wrong alignment");

enum class Result : int32_t {
    SUCCESS = 0,
    FAILURE = 1 /* (::vendor::qti::hardware::radio::ims::V1_5::Result.SUCCESS implicitly + 1) */,
};

struct CallForwardStatus final {
    uint32_t reason __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_5::Result status __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo errorDetails __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatus, reason) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatus, status) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatus, errorDetails) == 8, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatus) == 32, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatus) == 8, "wrong alignment");

struct CallForwardStatusInfo final {
    ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo errorDetails __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatus> status __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatusInfo, errorDetails) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatusInfo, status) == 24, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatusInfo) == 40, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatusInfo) == 8, "wrong alignment");

//
// type declarations for package
//

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause o);

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause lhs, const ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause lhs, const ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory o);

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory lhs, const ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory lhs, const ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_5::TirMode o);

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_5::TirMode lhs, const ::vendor::qti::hardware::radio::ims::V1_5::TirMode rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_5::TirMode rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_5::TirMode lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_5::TirMode lhs, const ::vendor::qti::hardware::radio::ims::V1_5::TirMode rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_5::TirMode rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_5::TirMode lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_5::TirMode e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_5::TirMode e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute o);

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute lhs, const ::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute lhs, const ::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_5::UssdModeType o);

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType lhs, const ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType lhs, const ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason o);

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason lhs, const ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason lhs, const ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult o);

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult lhs, const ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult lhs, const ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult e) {
    v &= static_cast<int32_t>(e);
    return v;
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_5::CallFailCauseResponse& o);
static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_5::CallFailCauseResponse& lhs, const ::vendor::qti::hardware::radio::ims::V1_5::CallFailCauseResponse& rhs);
static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_5::CallFailCauseResponse& lhs, const ::vendor::qti::hardware::radio::ims::V1_5::CallFailCauseResponse& rhs);

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_5::CallInfo& o);
static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_5::CallInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_5::CallInfo& rhs);
static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_5::CallInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_5::CallInfo& rhs);

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_5::AutoCallRejectionInfo& o);
static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_5::AutoCallRejectionInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_5::AutoCallRejectionInfo& rhs);
static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_5::AutoCallRejectionInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_5::AutoCallRejectionInfo& rhs);

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem o);

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem lhs, const ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem lhs, const ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem e) {
    v &= static_cast<int32_t>(e);
    return v;
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_5::ConfigInfo& o);
static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_5::ConfigInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_5::ConfigInfo& rhs);
static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_5::ConfigInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_5::ConfigInfo& rhs);

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_5::Result o);

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_5::Result lhs, const ::vendor::qti::hardware::radio::ims::V1_5::Result rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_5::Result rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_5::Result lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_5::Result lhs, const ::vendor::qti::hardware::radio::ims::V1_5::Result rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_5::Result rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_5::Result lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_5::Result e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_5::Result e) {
    v &= static_cast<int32_t>(e);
    return v;
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatus& o);
static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatus& lhs, const ::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatus& rhs);
static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatus& lhs, const ::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatus& rhs);

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatusInfo& o);
static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatusInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatusInfo& rhs);
static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatusInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatusInfo& rhs);

//
// type header definitions for package
//

template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_5::CallFailCause>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_5::CallFailCause> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_UNOBTAINABLE_NUMBER) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_UNOBTAINABLE_NUMBER)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_UNOBTAINABLE_NUMBER";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_UNOBTAINABLE_NUMBER;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NORMAL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NORMAL)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_NORMAL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NORMAL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_BUSY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_BUSY)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_BUSY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_BUSY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NORMAL_UNSPECIFIED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NORMAL_UNSPECIFIED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_NORMAL_UNSPECIFIED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NORMAL_UNSPECIFIED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CONGESTION) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CONGESTION)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CONGESTION";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CONGESTION;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_INCOMPATIBILITY_DESTINATION) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_INCOMPATIBILITY_DESTINATION)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_INCOMPATIBILITY_DESTINATION";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_INCOMPATIBILITY_DESTINATION;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_BARRED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_BARRED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CALL_BARRED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_BARRED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_FDN_BLOCKED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_FDN_BLOCKED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_FDN_BLOCKED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_FDN_BLOCKED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_IMEI_NOT_ACCEPTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_IMEI_NOT_ACCEPTED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_IMEI_NOT_ACCEPTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_IMEI_NOT_ACCEPTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_USSD) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_USSD)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_MODIFIED_TO_USSD";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_USSD;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_SS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_SS)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_MODIFIED_TO_SS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_SS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_MODIFIED_TO_DIAL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_EMERGENCY_TEMP_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_EMERGENCY_TEMP_FAILURE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_EMERGENCY_TEMP_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_EMERGENCY_TEMP_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_EMERGENCY_PERM_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_EMERGENCY_PERM_FAILURE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_EMERGENCY_PERM_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_EMERGENCY_PERM_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_HO_NOT_FEASIBLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_HO_NOT_FEASIBLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_HO_NOT_FEASIBLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_HO_NOT_FEASIBLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_USER_BUSY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_USER_BUSY)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_USER_BUSY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_USER_BUSY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_USER_REJECT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_USER_REJECT)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_USER_REJECT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_USER_REJECT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_LOW_BATTERY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_LOW_BATTERY)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_LOW_BATTERY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_LOW_BATTERY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_BLACKLISTED_CALL_ID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_BLACKLISTED_CALL_ID)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_BLACKLISTED_CALL_ID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_BLACKLISTED_CALL_ID;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CS_RETRY_REQUIRED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CS_RETRY_REQUIRED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CS_RETRY_REQUIRED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CS_RETRY_REQUIRED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NETWORK_UNAVAILABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NETWORK_UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_NETWORK_UNAVAILABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NETWORK_UNAVAILABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_FEATURE_UNAVAILABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_FEATURE_UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_FEATURE_UNAVAILABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_FEATURE_UNAVAILABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_ERROR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_ERROR)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_ERROR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_ERROR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MISC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MISC)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_MISC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MISC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_ANSWERED_ELSEWHERE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_ANSWERED_ELSEWHERE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_ANSWERED_ELSEWHERE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_ANSWERED_ELSEWHERE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_PULL_OUT_OF_SYNC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_PULL_OUT_OF_SYNC)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_PULL_OUT_OF_SYNC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_PULL_OUT_OF_SYNC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CAUSE_CALL_PULLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CAUSE_CALL_PULLED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CAUSE_CALL_PULLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CAUSE_CALL_PULLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_ACCESS_CLASS_BLOCKED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_ACCESS_CLASS_BLOCKED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_ACCESS_CLASS_BLOCKED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_ACCESS_CLASS_BLOCKED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REDIRECTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REDIRECTED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_REDIRECTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REDIRECTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_BAD_REQUEST) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_BAD_REQUEST)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_BAD_REQUEST";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_BAD_REQUEST;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_FORBIDDEN) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_FORBIDDEN)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_FORBIDDEN";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_FORBIDDEN;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_NOT_FOUND) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_NOT_FOUND)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_NOT_FOUND";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_NOT_FOUND;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_NOT_SUPPORTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_NOT_SUPPORTED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_NOT_SUPPORTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_NOT_SUPPORTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_TIMEOUT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_TIMEOUT)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_REQUEST_TIMEOUT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_TIMEOUT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_BAD_ADDRESS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_BAD_ADDRESS)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_BAD_ADDRESS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_BAD_ADDRESS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_BUSY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_BUSY)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_BUSY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_BUSY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_CANCELLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_CANCELLED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_REQUEST_CANCELLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_CANCELLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_NOT_ACCEPTABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_NOT_ACCEPTABLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_NOT_ACCEPTABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_NOT_ACCEPTABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_NOT_REACHABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_NOT_REACHABLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_NOT_REACHABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_NOT_REACHABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_INTERNAL_ERROR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_INTERNAL_ERROR)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVER_INTERNAL_ERROR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_INTERNAL_ERROR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_BAD_GATEWAY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_BAD_GATEWAY)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVER_BAD_GATEWAY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_BAD_GATEWAY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVICE_UNAVAILABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVICE_UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVICE_UNAVAILABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVICE_UNAVAILABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_TIMEOUT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_TIMEOUT)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVER_TIMEOUT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_TIMEOUT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_USER_REJECTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_USER_REJECTED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_USER_REJECTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_USER_REJECTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_GLOBAL_ERROR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_GLOBAL_ERROR)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_GLOBAL_ERROR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_GLOBAL_ERROR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MEDIA_INIT_FAILED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MEDIA_INIT_FAILED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_MEDIA_INIT_FAILED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MEDIA_INIT_FAILED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MEDIA_NO_DATA) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MEDIA_NO_DATA)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_MEDIA_NO_DATA";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MEDIA_NO_DATA;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MEDIA_NOT_ACCEPTABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MEDIA_NOT_ACCEPTABLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_MEDIA_NOT_ACCEPTABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MEDIA_NOT_ACCEPTABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MEDIA_UNSPECIFIED_ERROR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MEDIA_UNSPECIFIED_ERROR)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_MEDIA_UNSPECIFIED_ERROR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MEDIA_UNSPECIFIED_ERROR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_HOLD_RESUME_FAILED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_HOLD_RESUME_FAILED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_HOLD_RESUME_FAILED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_HOLD_RESUME_FAILED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_HOLD_RESUME_CANCELED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_HOLD_RESUME_CANCELED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_HOLD_RESUME_CANCELED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_HOLD_RESUME_CANCELED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_HOLD_REINVITE_COLLISION) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_HOLD_REINVITE_COLLISION)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_HOLD_REINVITE_COLLISION";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_HOLD_REINVITE_COLLISION;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_ERROR_UNSPECIFIED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_ERROR_UNSPECIFIED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_ERROR_UNSPECIFIED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_ERROR_UNSPECIFIED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_INVALID)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_INVALID;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_ALTERNATE_EMERGENCY_CALL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_ALTERNATE_EMERGENCY_CALL)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_ALTERNATE_EMERGENCY_CALL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_ALTERNATE_EMERGENCY_CALL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_OFF) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_OFF)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_RADIO_OFF";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_OFF;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OUT_OF_SERVICE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OUT_OF_SERVICE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OUT_OF_SERVICE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OUT_OF_SERVICE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NO_VALID_SIM) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NO_VALID_SIM)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_NO_VALID_SIM";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NO_VALID_SIM;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_INTERNAL_ERROR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_INTERNAL_ERROR)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_RADIO_INTERNAL_ERROR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_INTERNAL_ERROR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NETWORK_RESP_TIMEOUT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NETWORK_RESP_TIMEOUT)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_NETWORK_RESP_TIMEOUT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NETWORK_RESP_TIMEOUT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NETWORK_REJECT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NETWORK_REJECT)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_NETWORK_REJECT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NETWORK_REJECT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_ACCESS_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_ACCESS_FAILURE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_RADIO_ACCESS_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_ACCESS_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_LINK_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_LINK_FAILURE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_RADIO_LINK_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_LINK_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_LINK_LOST) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_LINK_LOST)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_RADIO_LINK_LOST";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_LINK_LOST;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_UPLINK_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_UPLINK_FAILURE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_RADIO_UPLINK_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_UPLINK_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_SETUP_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_SETUP_FAILURE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_RADIO_SETUP_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_SETUP_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_RELEASE_NORMAL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_RELEASE_NORMAL)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_RADIO_RELEASE_NORMAL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_RELEASE_NORMAL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_RELEASE_ABNORMAL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_RELEASE_ABNORMAL)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_RADIO_RELEASE_ABNORMAL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_RELEASE_ABNORMAL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NETWORK_DETACH) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NETWORK_DETACH)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_NETWORK_DETACH";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NETWORK_DETACH;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_1) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_1)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_1";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_1;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_2) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_2)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_2";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_2;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_3) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_3)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_3";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_3;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_4) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_4)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_4";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_4;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_5) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_5)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_5";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_5;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_6) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_6)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_6";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_6;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_7) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_7)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_7";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_7;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_8) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_8)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_8";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_8;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_9) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_9)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_9";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_9;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_10) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_10)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_10";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_10;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_11) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_11)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_11";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_11;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_12) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_12)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_12";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_12;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_13) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_13)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_13";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_13;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_14) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_14)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_14";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_14;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_15) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_15)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_OEM_CAUSE_15";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_15;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NO_CSFB_IN_CS_ROAM) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NO_CSFB_IN_CS_ROAM)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_NO_CSFB_IN_CS_ROAM";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NO_CSFB_IN_CS_ROAM;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SRV_NOT_REGISTERED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SRV_NOT_REGISTERED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SRV_NOT_REGISTERED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SRV_NOT_REGISTERED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_TYPE_NOT_ALLOWED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_TYPE_NOT_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CALL_TYPE_NOT_ALLOWED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_TYPE_NOT_ALLOWED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_EMRG_CALL_ONGOING) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_EMRG_CALL_ONGOING)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_EMRG_CALL_ONGOING";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_EMRG_CALL_ONGOING;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_SETUP_ONGOING) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_SETUP_ONGOING)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CALL_SETUP_ONGOING";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_SETUP_ONGOING;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MAX_CALL_LIMIT_REACHED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MAX_CALL_LIMIT_REACHED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_MAX_CALL_LIMIT_REACHED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MAX_CALL_LIMIT_REACHED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_UNSUPPORTED_SIP_HDRS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_UNSUPPORTED_SIP_HDRS)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_UNSUPPORTED_SIP_HDRS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_UNSUPPORTED_SIP_HDRS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_TRANSFER_ONGOING) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_TRANSFER_ONGOING)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CALL_TRANSFER_ONGOING";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_TRANSFER_ONGOING;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_PRACK_TIMEOUT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_PRACK_TIMEOUT)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_PRACK_TIMEOUT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_PRACK_TIMEOUT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_QOS_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_QOS_FAILURE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_QOS_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_QOS_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_ONGOING_HANDOVER) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_ONGOING_HANDOVER)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_ONGOING_HANDOVER";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_ONGOING_HANDOVER;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_VT_WITH_TTY_NOT_ALLOWED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_VT_WITH_TTY_NOT_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_VT_WITH_TTY_NOT_ALLOWED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_VT_WITH_TTY_NOT_ALLOWED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_UPGRADE_ONGOING) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_UPGRADE_ONGOING)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CALL_UPGRADE_ONGOING";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_UPGRADE_ONGOING;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CONFERENCE_WITH_TTY_NOT_ALLOWED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CONFERENCE_WITH_TTY_NOT_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CONFERENCE_WITH_TTY_NOT_ALLOWED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CONFERENCE_WITH_TTY_NOT_ALLOWED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_CONFERENCE_ONGOING) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_CONFERENCE_ONGOING)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CALL_CONFERENCE_ONGOING";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_CONFERENCE_ONGOING;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_VT_WITH_AVPF_NOT_ALLOWED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_VT_WITH_AVPF_NOT_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_VT_WITH_AVPF_NOT_ALLOWED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_VT_WITH_AVPF_NOT_ALLOWED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_ENCRYPTION_CALL_ONGOING) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_ENCRYPTION_CALL_ONGOING)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_ENCRYPTION_CALL_ONGOING";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_ENCRYPTION_CALL_ONGOING;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_ONGOING_CW_DISABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_ONGOING_CW_DISABLED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CALL_ONGOING_CW_DISABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_ONGOING_CW_DISABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_ON_OTHER_SUB) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_ON_OTHER_SUB)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CALL_ON_OTHER_SUB";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_ON_OTHER_SUB;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_1X_COLLISION) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_1X_COLLISION)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_1X_COLLISION";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_1X_COLLISION;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_UI_NOT_READY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_UI_NOT_READY)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_UI_NOT_READY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_UI_NOT_READY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CS_CALL_ONGOING) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CS_CALL_ONGOING)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_CS_CALL_ONGOING";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CS_CALL_ONGOING;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_USER_MARKED_UNWANTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_USER_MARKED_UNWANTED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_USER_MARKED_UNWANTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_USER_MARKED_UNWANTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_REJECTED_ELSEWHERE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_REJECTED_ELSEWHERE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_REJECTED_ELSEWHERE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_REJECTED_ELSEWHERE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_USER_REJECTED_SESSION_MODIFICATION) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_USER_REJECTED_SESSION_MODIFICATION)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_USER_REJECTED_SESSION_MODIFICATION";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_USER_REJECTED_SESSION_MODIFICATION;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_USER_CANCELLED_SESSION_MODIFICATION) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_USER_CANCELLED_SESSION_MODIFICATION)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_USER_CANCELLED_SESSION_MODIFICATION";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_USER_CANCELLED_SESSION_MODIFICATION;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SESSION_MODIFICATION_FAILED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SESSION_MODIFICATION_FAILED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SESSION_MODIFICATION_FAILED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SESSION_MODIFICATION_FAILED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_METHOD_NOT_ALLOWED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_METHOD_NOT_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_METHOD_NOT_ALLOWED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_METHOD_NOT_ALLOWED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_PROXY_AUTHENTICATION_REQUIRED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_PROXY_AUTHENTICATION_REQUIRED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_PROXY_AUTHENTICATION_REQUIRED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_PROXY_AUTHENTICATION_REQUIRED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_ENTITY_TOO_LARGE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_ENTITY_TOO_LARGE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_REQUEST_ENTITY_TOO_LARGE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_ENTITY_TOO_LARGE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_URI_TOO_LARGE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_URI_TOO_LARGE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_REQUEST_URI_TOO_LARGE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_URI_TOO_LARGE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_EXTENSION_REQUIRED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_EXTENSION_REQUIRED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_EXTENSION_REQUIRED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_EXTENSION_REQUIRED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_INTERVAL_TOO_BRIEF) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_INTERVAL_TOO_BRIEF)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_INTERVAL_TOO_BRIEF";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_INTERVAL_TOO_BRIEF;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_CALL_OR_TRANS_DOES_NOT_EXIST) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_CALL_OR_TRANS_DOES_NOT_EXIST)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_CALL_OR_TRANS_DOES_NOT_EXIST";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_CALL_OR_TRANS_DOES_NOT_EXIST;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_LOOP_DETECTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_LOOP_DETECTED)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_LOOP_DETECTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_LOOP_DETECTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_TOO_MANY_HOPS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_TOO_MANY_HOPS)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_TOO_MANY_HOPS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_TOO_MANY_HOPS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_AMBIGUOUS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_AMBIGUOUS)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_AMBIGUOUS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_AMBIGUOUS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_PENDING) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_PENDING)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_REQUEST_PENDING";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_PENDING;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_UNDECIPHERABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_UNDECIPHERABLE)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_SIP_UNDECIPHERABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_UNDECIPHERABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RETRY_ON_IMS_WITHOUT_RTT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RETRY_ON_IMS_WITHOUT_RTT)) {
        os += (first ? "" : " | ");
        os += "CALL_FAIL_RETRY_ON_IMS_WITHOUT_RTT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RETRY_ON_IMS_WITHOUT_RTT;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_5::CallFailCause o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_UNOBTAINABLE_NUMBER) {
        return "CALL_FAIL_UNOBTAINABLE_NUMBER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NORMAL) {
        return "CALL_FAIL_NORMAL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_BUSY) {
        return "CALL_FAIL_BUSY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NORMAL_UNSPECIFIED) {
        return "CALL_FAIL_NORMAL_UNSPECIFIED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CONGESTION) {
        return "CALL_FAIL_CONGESTION";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_INCOMPATIBILITY_DESTINATION) {
        return "CALL_FAIL_INCOMPATIBILITY_DESTINATION";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_BARRED) {
        return "CALL_FAIL_CALL_BARRED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_FDN_BLOCKED) {
        return "CALL_FAIL_FDN_BLOCKED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_IMEI_NOT_ACCEPTED) {
        return "CALL_FAIL_IMEI_NOT_ACCEPTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_USSD) {
        return "CALL_FAIL_DIAL_MODIFIED_TO_USSD";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_SS) {
        return "CALL_FAIL_DIAL_MODIFIED_TO_SS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL) {
        return "CALL_FAIL_DIAL_MODIFIED_TO_DIAL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_EMERGENCY_TEMP_FAILURE) {
        return "CALL_FAIL_EMERGENCY_TEMP_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_EMERGENCY_PERM_FAILURE) {
        return "CALL_FAIL_EMERGENCY_PERM_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_HO_NOT_FEASIBLE) {
        return "CALL_FAIL_HO_NOT_FEASIBLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_USER_BUSY) {
        return "CALL_FAIL_USER_BUSY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_USER_REJECT) {
        return "CALL_FAIL_USER_REJECT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_LOW_BATTERY) {
        return "CALL_FAIL_LOW_BATTERY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_BLACKLISTED_CALL_ID) {
        return "CALL_FAIL_BLACKLISTED_CALL_ID";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CS_RETRY_REQUIRED) {
        return "CALL_FAIL_CS_RETRY_REQUIRED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NETWORK_UNAVAILABLE) {
        return "CALL_FAIL_NETWORK_UNAVAILABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_FEATURE_UNAVAILABLE) {
        return "CALL_FAIL_FEATURE_UNAVAILABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_ERROR) {
        return "CALL_FAIL_SIP_ERROR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MISC) {
        return "CALL_FAIL_MISC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_ANSWERED_ELSEWHERE) {
        return "CALL_FAIL_ANSWERED_ELSEWHERE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_PULL_OUT_OF_SYNC) {
        return "CALL_FAIL_PULL_OUT_OF_SYNC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CAUSE_CALL_PULLED) {
        return "CALL_FAIL_CAUSE_CALL_PULLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_ACCESS_CLASS_BLOCKED) {
        return "CALL_FAIL_ACCESS_CLASS_BLOCKED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REDIRECTED) {
        return "CALL_FAIL_SIP_REDIRECTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_BAD_REQUEST) {
        return "CALL_FAIL_SIP_BAD_REQUEST";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_FORBIDDEN) {
        return "CALL_FAIL_SIP_FORBIDDEN";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_NOT_FOUND) {
        return "CALL_FAIL_SIP_NOT_FOUND";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_NOT_SUPPORTED) {
        return "CALL_FAIL_SIP_NOT_SUPPORTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_TIMEOUT) {
        return "CALL_FAIL_SIP_REQUEST_TIMEOUT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE) {
        return "CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_BAD_ADDRESS) {
        return "CALL_FAIL_SIP_BAD_ADDRESS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_BUSY) {
        return "CALL_FAIL_SIP_BUSY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_CANCELLED) {
        return "CALL_FAIL_SIP_REQUEST_CANCELLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_NOT_ACCEPTABLE) {
        return "CALL_FAIL_SIP_NOT_ACCEPTABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_NOT_REACHABLE) {
        return "CALL_FAIL_SIP_NOT_REACHABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_INTERNAL_ERROR) {
        return "CALL_FAIL_SIP_SERVER_INTERNAL_ERROR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED) {
        return "CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_BAD_GATEWAY) {
        return "CALL_FAIL_SIP_SERVER_BAD_GATEWAY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVICE_UNAVAILABLE) {
        return "CALL_FAIL_SIP_SERVICE_UNAVAILABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_TIMEOUT) {
        return "CALL_FAIL_SIP_SERVER_TIMEOUT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED) {
        return "CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE) {
        return "CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE) {
        return "CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_USER_REJECTED) {
        return "CALL_FAIL_SIP_USER_REJECTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_GLOBAL_ERROR) {
        return "CALL_FAIL_SIP_GLOBAL_ERROR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MEDIA_INIT_FAILED) {
        return "CALL_FAIL_MEDIA_INIT_FAILED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MEDIA_NO_DATA) {
        return "CALL_FAIL_MEDIA_NO_DATA";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MEDIA_NOT_ACCEPTABLE) {
        return "CALL_FAIL_MEDIA_NOT_ACCEPTABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MEDIA_UNSPECIFIED_ERROR) {
        return "CALL_FAIL_MEDIA_UNSPECIFIED_ERROR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_HOLD_RESUME_FAILED) {
        return "CALL_FAIL_HOLD_RESUME_FAILED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_HOLD_RESUME_CANCELED) {
        return "CALL_FAIL_HOLD_RESUME_CANCELED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_HOLD_REINVITE_COLLISION) {
        return "CALL_FAIL_HOLD_REINVITE_COLLISION";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO) {
        return "CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL) {
        return "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO) {
        return "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS) {
        return "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD) {
        return "CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_ERROR_UNSPECIFIED) {
        return "CALL_FAIL_ERROR_UNSPECIFIED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_INVALID) {
        return "CALL_FAIL_INVALID";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_ALTERNATE_EMERGENCY_CALL) {
        return "CALL_FAIL_SIP_ALTERNATE_EMERGENCY_CALL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_OFF) {
        return "CALL_FAIL_RADIO_OFF";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OUT_OF_SERVICE) {
        return "CALL_FAIL_OUT_OF_SERVICE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NO_VALID_SIM) {
        return "CALL_FAIL_NO_VALID_SIM";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_INTERNAL_ERROR) {
        return "CALL_FAIL_RADIO_INTERNAL_ERROR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NETWORK_RESP_TIMEOUT) {
        return "CALL_FAIL_NETWORK_RESP_TIMEOUT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NETWORK_REJECT) {
        return "CALL_FAIL_NETWORK_REJECT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_ACCESS_FAILURE) {
        return "CALL_FAIL_RADIO_ACCESS_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_LINK_FAILURE) {
        return "CALL_FAIL_RADIO_LINK_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_LINK_LOST) {
        return "CALL_FAIL_RADIO_LINK_LOST";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_UPLINK_FAILURE) {
        return "CALL_FAIL_RADIO_UPLINK_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_SETUP_FAILURE) {
        return "CALL_FAIL_RADIO_SETUP_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_RELEASE_NORMAL) {
        return "CALL_FAIL_RADIO_RELEASE_NORMAL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_RELEASE_ABNORMAL) {
        return "CALL_FAIL_RADIO_RELEASE_ABNORMAL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NETWORK_DETACH) {
        return "CALL_FAIL_NETWORK_DETACH";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_1) {
        return "CALL_FAIL_OEM_CAUSE_1";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_2) {
        return "CALL_FAIL_OEM_CAUSE_2";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_3) {
        return "CALL_FAIL_OEM_CAUSE_3";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_4) {
        return "CALL_FAIL_OEM_CAUSE_4";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_5) {
        return "CALL_FAIL_OEM_CAUSE_5";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_6) {
        return "CALL_FAIL_OEM_CAUSE_6";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_7) {
        return "CALL_FAIL_OEM_CAUSE_7";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_8) {
        return "CALL_FAIL_OEM_CAUSE_8";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_9) {
        return "CALL_FAIL_OEM_CAUSE_9";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_10) {
        return "CALL_FAIL_OEM_CAUSE_10";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_11) {
        return "CALL_FAIL_OEM_CAUSE_11";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_12) {
        return "CALL_FAIL_OEM_CAUSE_12";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_13) {
        return "CALL_FAIL_OEM_CAUSE_13";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_14) {
        return "CALL_FAIL_OEM_CAUSE_14";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_15) {
        return "CALL_FAIL_OEM_CAUSE_15";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NO_CSFB_IN_CS_ROAM) {
        return "CALL_FAIL_NO_CSFB_IN_CS_ROAM";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SRV_NOT_REGISTERED) {
        return "CALL_FAIL_SRV_NOT_REGISTERED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_TYPE_NOT_ALLOWED) {
        return "CALL_FAIL_CALL_TYPE_NOT_ALLOWED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_EMRG_CALL_ONGOING) {
        return "CALL_FAIL_EMRG_CALL_ONGOING";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_SETUP_ONGOING) {
        return "CALL_FAIL_CALL_SETUP_ONGOING";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MAX_CALL_LIMIT_REACHED) {
        return "CALL_FAIL_MAX_CALL_LIMIT_REACHED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_UNSUPPORTED_SIP_HDRS) {
        return "CALL_FAIL_UNSUPPORTED_SIP_HDRS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_TRANSFER_ONGOING) {
        return "CALL_FAIL_CALL_TRANSFER_ONGOING";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_PRACK_TIMEOUT) {
        return "CALL_FAIL_PRACK_TIMEOUT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_QOS_FAILURE) {
        return "CALL_FAIL_QOS_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_ONGOING_HANDOVER) {
        return "CALL_FAIL_ONGOING_HANDOVER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_VT_WITH_TTY_NOT_ALLOWED) {
        return "CALL_FAIL_VT_WITH_TTY_NOT_ALLOWED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_UPGRADE_ONGOING) {
        return "CALL_FAIL_CALL_UPGRADE_ONGOING";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CONFERENCE_WITH_TTY_NOT_ALLOWED) {
        return "CALL_FAIL_CONFERENCE_WITH_TTY_NOT_ALLOWED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_CONFERENCE_ONGOING) {
        return "CALL_FAIL_CALL_CONFERENCE_ONGOING";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_VT_WITH_AVPF_NOT_ALLOWED) {
        return "CALL_FAIL_VT_WITH_AVPF_NOT_ALLOWED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_ENCRYPTION_CALL_ONGOING) {
        return "CALL_FAIL_ENCRYPTION_CALL_ONGOING";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_ONGOING_CW_DISABLED) {
        return "CALL_FAIL_CALL_ONGOING_CW_DISABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_ON_OTHER_SUB) {
        return "CALL_FAIL_CALL_ON_OTHER_SUB";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_1X_COLLISION) {
        return "CALL_FAIL_1X_COLLISION";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_UI_NOT_READY) {
        return "CALL_FAIL_UI_NOT_READY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CS_CALL_ONGOING) {
        return "CALL_FAIL_CS_CALL_ONGOING";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_USER_MARKED_UNWANTED) {
        return "CALL_FAIL_SIP_USER_MARKED_UNWANTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_REJECTED_ELSEWHERE) {
        return "CALL_FAIL_REJECTED_ELSEWHERE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_USER_REJECTED_SESSION_MODIFICATION) {
        return "CALL_FAIL_USER_REJECTED_SESSION_MODIFICATION";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_USER_CANCELLED_SESSION_MODIFICATION) {
        return "CALL_FAIL_USER_CANCELLED_SESSION_MODIFICATION";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SESSION_MODIFICATION_FAILED) {
        return "CALL_FAIL_SESSION_MODIFICATION_FAILED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_METHOD_NOT_ALLOWED) {
        return "CALL_FAIL_SIP_METHOD_NOT_ALLOWED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_PROXY_AUTHENTICATION_REQUIRED) {
        return "CALL_FAIL_SIP_PROXY_AUTHENTICATION_REQUIRED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_ENTITY_TOO_LARGE) {
        return "CALL_FAIL_SIP_REQUEST_ENTITY_TOO_LARGE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_URI_TOO_LARGE) {
        return "CALL_FAIL_SIP_REQUEST_URI_TOO_LARGE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_EXTENSION_REQUIRED) {
        return "CALL_FAIL_SIP_EXTENSION_REQUIRED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_INTERVAL_TOO_BRIEF) {
        return "CALL_FAIL_SIP_INTERVAL_TOO_BRIEF";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_CALL_OR_TRANS_DOES_NOT_EXIST) {
        return "CALL_FAIL_SIP_CALL_OR_TRANS_DOES_NOT_EXIST";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_LOOP_DETECTED) {
        return "CALL_FAIL_SIP_LOOP_DETECTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_TOO_MANY_HOPS) {
        return "CALL_FAIL_SIP_TOO_MANY_HOPS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_AMBIGUOUS) {
        return "CALL_FAIL_SIP_AMBIGUOUS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_PENDING) {
        return "CALL_FAIL_SIP_REQUEST_PENDING";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_UNDECIPHERABLE) {
        return "CALL_FAIL_SIP_UNDECIPHERABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RETRY_ON_IMS_WITHOUT_RTT) {
        return "CALL_FAIL_RETRY_ON_IMS_WITHOUT_RTT";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::UNSPECIFIED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::UNSPECIFIED)) {
        os += (first ? "" : " | ");
        os += "UNSPECIFIED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::UNSPECIFIED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::POLICE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::POLICE)) {
        os += (first ? "" : " | ");
        os += "POLICE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::POLICE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::AMBULANCE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::AMBULANCE)) {
        os += (first ? "" : " | ");
        os += "AMBULANCE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::AMBULANCE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::FIRE_BRIGADE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::FIRE_BRIGADE)) {
        os += (first ? "" : " | ");
        os += "FIRE_BRIGADE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::FIRE_BRIGADE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::MARINE_GUARD) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::MARINE_GUARD)) {
        os += (first ? "" : " | ");
        os += "MARINE_GUARD";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::MARINE_GUARD;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::MOUNTAIN_RESCUE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::MOUNTAIN_RESCUE)) {
        os += (first ? "" : " | ");
        os += "MOUNTAIN_RESCUE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::MOUNTAIN_RESCUE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::MIEC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::MIEC)) {
        os += (first ? "" : " | ");
        os += "MIEC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::MIEC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::AIEC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::AIEC)) {
        os += (first ? "" : " | ");
        os += "AIEC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::AIEC;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::UNSPECIFIED) {
        return "UNSPECIFIED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::POLICE) {
        return "POLICE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::AMBULANCE) {
        return "AMBULANCE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::FIRE_BRIGADE) {
        return "FIRE_BRIGADE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::MARINE_GUARD) {
        return "MARINE_GUARD";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::MOUNTAIN_RESCUE) {
        return "MOUNTAIN_RESCUE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::MIEC) {
        return "MIEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::AIEC) {
        return "AIEC";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_5::TirMode>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_5::TirMode> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::TirMode::INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::TirMode::INVALID)) {
        os += (first ? "" : " | ");
        os += "INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::TirMode::INVALID;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::TirMode::TEMPORARY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::TirMode::TEMPORARY)) {
        os += (first ? "" : " | ");
        os += "TEMPORARY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::TirMode::TEMPORARY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::TirMode::PERMANENT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::TirMode::PERMANENT)) {
        os += (first ? "" : " | ");
        os += "PERMANENT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::TirMode::PERMANENT;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_5::TirMode o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::TirMode::INVALID) {
        return "INVALID";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::TirMode::TEMPORARY) {
        return "TEMPORARY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::TirMode::PERMANENT) {
        return "PERMANENT";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute::UNKNOWN) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute::UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute::UNKNOWN;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute::EMERGENCY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute::EMERGENCY)) {
        os += (first ? "" : " | ");
        os += "EMERGENCY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute::EMERGENCY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute::NORMAL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute::NORMAL)) {
        os += (first ? "" : " | ");
        os += "NORMAL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute::NORMAL;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute::UNKNOWN) {
        return "UNKNOWN";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute::EMERGENCY) {
        return "EMERGENCY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute::NORMAL) {
        return "NORMAL";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_5::UssdModeType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_5::UssdModeType> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::NOTIFY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::NOTIFY)) {
        os += (first ? "" : " | ");
        os += "NOTIFY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::NOTIFY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::REQUEST) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::REQUEST)) {
        os += (first ? "" : " | ");
        os += "REQUEST";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::REQUEST;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::NW_RELEASE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::NW_RELEASE)) {
        os += (first ? "" : " | ");
        os += "NW_RELEASE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::NW_RELEASE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::LOCAL_CLIENT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::LOCAL_CLIENT)) {
        os += (first ? "" : " | ");
        os += "LOCAL_CLIENT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::LOCAL_CLIENT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::NOT_SUPPORTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::NOT_SUPPORTED)) {
        os += (first ? "" : " | ");
        os += "NOT_SUPPORTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::NOT_SUPPORTED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::NW_TIMEOUT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::NW_TIMEOUT)) {
        os += (first ? "" : " | ");
        os += "NW_TIMEOUT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::NW_TIMEOUT;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_5::UssdModeType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::NOTIFY) {
        return "NOTIFY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::REQUEST) {
        return "REQUEST";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::NW_RELEASE) {
        return "NW_RELEASE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::LOCAL_CLIENT) {
        return "LOCAL_CLIENT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::NOT_SUPPORTED) {
        return "NOT_SUPPORTED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::NW_TIMEOUT) {
        return "NW_TIMEOUT";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_NONE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_NONE)) {
        os += (first ? "" : " | ");
        os += "RESULT_ERROR_NONE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_NONE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_GENERIC_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_GENERIC_FAILURE)) {
        os += (first ? "" : " | ");
        os += "RESULT_ERROR_GENERIC_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_GENERIC_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_RADIO_OFF) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_RADIO_OFF)) {
        os += (first ? "" : " | ");
        os += "RESULT_ERROR_RADIO_OFF";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_RADIO_OFF;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_NULL_PDU) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_NULL_PDU)) {
        os += (first ? "" : " | ");
        os += "RESULT_ERROR_NULL_PDU";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_NULL_PDU;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_NO_SERVICE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_NO_SERVICE)) {
        os += (first ? "" : " | ");
        os += "RESULT_ERROR_NO_SERVICE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_NO_SERVICE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_LIMIT_EXCEEDED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_LIMIT_EXCEEDED)) {
        os += (first ? "" : " | ");
        os += "RESULT_ERROR_LIMIT_EXCEEDED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_LIMIT_EXCEEDED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_SHORT_CODE_NOT_ALLOWED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_SHORT_CODE_NOT_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "RESULT_ERROR_SHORT_CODE_NOT_ALLOWED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_SHORT_CODE_NOT_ALLOWED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_SHORT_CODE_NEVER_ALLOWED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_SHORT_CODE_NEVER_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "RESULT_ERROR_SHORT_CODE_NEVER_ALLOWED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_SHORT_CODE_NEVER_ALLOWED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_FDN_CHECK_FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_FDN_CHECK_FAILURE)) {
        os += (first ? "" : " | ");
        os += "RESULT_ERROR_FDN_CHECK_FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_FDN_CHECK_FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_RADIO_NOT_AVAILABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_RADIO_NOT_AVAILABLE)) {
        os += (first ? "" : " | ");
        os += "RESULT_RADIO_NOT_AVAILABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_RADIO_NOT_AVAILABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_NETWORK_REJECT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_NETWORK_REJECT)) {
        os += (first ? "" : " | ");
        os += "RESULT_NETWORK_REJECT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_NETWORK_REJECT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_INVALID_ARGUMENTS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_INVALID_ARGUMENTS)) {
        os += (first ? "" : " | ");
        os += "RESULT_INVALID_ARGUMENTS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_INVALID_ARGUMENTS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_INVALID_STATE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_INVALID_STATE)) {
        os += (first ? "" : " | ");
        os += "RESULT_INVALID_STATE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_INVALID_STATE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_NO_MEMORY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_NO_MEMORY)) {
        os += (first ? "" : " | ");
        os += "RESULT_NO_MEMORY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_NO_MEMORY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_INVALID_SMS_FORMAT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_INVALID_SMS_FORMAT)) {
        os += (first ? "" : " | ");
        os += "RESULT_INVALID_SMS_FORMAT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_INVALID_SMS_FORMAT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_SYSTEM_ERROR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_SYSTEM_ERROR)) {
        os += (first ? "" : " | ");
        os += "RESULT_SYSTEM_ERROR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_SYSTEM_ERROR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_MODEM_ERROR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_MODEM_ERROR)) {
        os += (first ? "" : " | ");
        os += "RESULT_MODEM_ERROR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_MODEM_ERROR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_NETWORK_ERROR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_NETWORK_ERROR)) {
        os += (first ? "" : " | ");
        os += "RESULT_NETWORK_ERROR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_NETWORK_ERROR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ENCODING_ERROR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ENCODING_ERROR)) {
        os += (first ? "" : " | ");
        os += "RESULT_ENCODING_ERROR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ENCODING_ERROR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_INVALID_SMSC_ADDRESS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_INVALID_SMSC_ADDRESS)) {
        os += (first ? "" : " | ");
        os += "RESULT_INVALID_SMSC_ADDRESS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_INVALID_SMSC_ADDRESS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_OPERATION_NOT_ALLOWED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_OPERATION_NOT_ALLOWED)) {
        os += (first ? "" : " | ");
        os += "RESULT_OPERATION_NOT_ALLOWED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_OPERATION_NOT_ALLOWED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_INTERNAL_ERROR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_INTERNAL_ERROR)) {
        os += (first ? "" : " | ");
        os += "RESULT_INTERNAL_ERROR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_INTERNAL_ERROR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_NO_RESOURCES) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_NO_RESOURCES)) {
        os += (first ? "" : " | ");
        os += "RESULT_NO_RESOURCES";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_NO_RESOURCES;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_CANCELLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_CANCELLED)) {
        os += (first ? "" : " | ");
        os += "RESULT_CANCELLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_CANCELLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_REQUEST_NOT_SUPPORTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_REQUEST_NOT_SUPPORTED)) {
        os += (first ? "" : " | ");
        os += "RESULT_REQUEST_NOT_SUPPORTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_REQUEST_NOT_SUPPORTED;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_NONE) {
        return "RESULT_ERROR_NONE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_GENERIC_FAILURE) {
        return "RESULT_ERROR_GENERIC_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_RADIO_OFF) {
        return "RESULT_ERROR_RADIO_OFF";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_NULL_PDU) {
        return "RESULT_ERROR_NULL_PDU";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_NO_SERVICE) {
        return "RESULT_ERROR_NO_SERVICE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_LIMIT_EXCEEDED) {
        return "RESULT_ERROR_LIMIT_EXCEEDED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_SHORT_CODE_NOT_ALLOWED) {
        return "RESULT_ERROR_SHORT_CODE_NOT_ALLOWED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_SHORT_CODE_NEVER_ALLOWED) {
        return "RESULT_ERROR_SHORT_CODE_NEVER_ALLOWED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_FDN_CHECK_FAILURE) {
        return "RESULT_ERROR_FDN_CHECK_FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_RADIO_NOT_AVAILABLE) {
        return "RESULT_RADIO_NOT_AVAILABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_NETWORK_REJECT) {
        return "RESULT_NETWORK_REJECT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_INVALID_ARGUMENTS) {
        return "RESULT_INVALID_ARGUMENTS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_INVALID_STATE) {
        return "RESULT_INVALID_STATE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_NO_MEMORY) {
        return "RESULT_NO_MEMORY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_INVALID_SMS_FORMAT) {
        return "RESULT_INVALID_SMS_FORMAT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_SYSTEM_ERROR) {
        return "RESULT_SYSTEM_ERROR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_MODEM_ERROR) {
        return "RESULT_MODEM_ERROR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_NETWORK_ERROR) {
        return "RESULT_NETWORK_ERROR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ENCODING_ERROR) {
        return "RESULT_ENCODING_ERROR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_INVALID_SMSC_ADDRESS) {
        return "RESULT_INVALID_SMSC_ADDRESS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_OPERATION_NOT_ALLOWED) {
        return "RESULT_OPERATION_NOT_ALLOWED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_INTERNAL_ERROR) {
        return "RESULT_INTERNAL_ERROR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_NO_RESOURCES) {
        return "RESULT_NO_RESOURCES";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_CANCELLED) {
        return "RESULT_CANCELLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_REQUEST_NOT_SUPPORTED) {
        return "RESULT_REQUEST_NOT_SUPPORTED";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult::DELIVER_STATUS_OK) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult::DELIVER_STATUS_OK)) {
        os += (first ? "" : " | ");
        os += "DELIVER_STATUS_OK";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult::DELIVER_STATUS_OK;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult::DELIVER_STATUS_ERROR) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult::DELIVER_STATUS_ERROR)) {
        os += (first ? "" : " | ");
        os += "DELIVER_STATUS_ERROR";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult::DELIVER_STATUS_ERROR;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult::DELIVER_STATUS_ERROR_NO_MEMORY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult::DELIVER_STATUS_ERROR_NO_MEMORY)) {
        os += (first ? "" : " | ");
        os += "DELIVER_STATUS_ERROR_NO_MEMORY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult::DELIVER_STATUS_ERROR_NO_MEMORY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult::DELIVER_STATUS_ERROR_REQUEST_NOT_SUPPORTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult::DELIVER_STATUS_ERROR_REQUEST_NOT_SUPPORTED)) {
        os += (first ? "" : " | ");
        os += "DELIVER_STATUS_ERROR_REQUEST_NOT_SUPPORTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult::DELIVER_STATUS_ERROR_REQUEST_NOT_SUPPORTED;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult::DELIVER_STATUS_OK) {
        return "DELIVER_STATUS_OK";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult::DELIVER_STATUS_ERROR) {
        return "DELIVER_STATUS_ERROR";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult::DELIVER_STATUS_ERROR_NO_MEMORY) {
        return "DELIVER_STATUS_ERROR_NO_MEMORY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult::DELIVER_STATUS_ERROR_REQUEST_NOT_SUPPORTED) {
        return "DELIVER_STATUS_ERROR_REQUEST_NOT_SUPPORTED";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_5::CallFailCauseResponse& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".failCause = ";
    os += ::vendor::qti::hardware::radio::ims::V1_5::toString(o.failCause);
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

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_5::CallFailCauseResponse& lhs, const ::vendor::qti::hardware::radio::ims::V1_5::CallFailCauseResponse& rhs) {
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

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_5::CallFailCauseResponse& lhs, const ::vendor::qti::hardware::radio::ims::V1_5::CallFailCauseResponse& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_5::CallInfo& o) {
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
    os += ", .mtMultiLineInfo = ";
    os += ::vendor::qti::hardware::radio::ims::V1_4::toString(o.mtMultiLineInfo);
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
    os += ::vendor::qti::hardware::radio::ims::V1_5::toString(o.failCause);
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
    os += ", .tirMode = ";
    os += ::vendor::qti::hardware::radio::ims::V1_5::toString(o.tirMode);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_5::CallInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_5::CallInfo& rhs) {
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
    if (lhs.mtMultiLineInfo != rhs.mtMultiLineInfo) {
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
    if (lhs.tirMode != rhs.tirMode) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_5::CallInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_5::CallInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_5::AutoCallRejectionInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".callType = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.callType);
    os += ", .autoRejectionCause = ";
    os += ::vendor::qti::hardware::radio::ims::V1_5::toString(o.autoRejectionCause);
    os += ", .sipErrorCode = ";
    os += ::android::hardware::toString(o.sipErrorCode);
    os += ", .number = ";
    os += ::android::hardware::toString(o.number);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_5::AutoCallRejectionInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_5::AutoCallRejectionInfo& rhs) {
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

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_5::AutoCallRejectionInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_5::AutoCallRejectionInfo& rhs){
    return !(lhs == rhs);
}

template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_5::ConfigItem>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_5::ConfigItem> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_NONE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_NONE)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_NONE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_NONE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOCODER_AMRMODESET) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOCODER_AMRMODESET)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_VOCODER_AMRMODESET";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOCODER_AMRMODESET;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOCODER_AMRWBMODESET) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOCODER_AMRWBMODESET)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_VOCODER_AMRWBMODESET";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOCODER_AMRWBMODESET;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_SESSION_TIMER) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_SESSION_TIMER)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SIP_SESSION_TIMER";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_SESSION_TIMER;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_MIN_SESSION_EXPIRY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_MIN_SESSION_EXPIRY)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_MIN_SESSION_EXPIRY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_MIN_SESSION_EXPIRY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_CANCELLATION_TIMER) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_CANCELLATION_TIMER)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_CANCELLATION_TIMER";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_CANCELLATION_TIMER;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_T_DELAY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_T_DELAY)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_T_DELAY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_T_DELAY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SILENT_REDIAL_ENABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SILENT_REDIAL_ENABLE)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SILENT_REDIAL_ENABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SILENT_REDIAL_ENABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_T1_TIMER) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_T1_TIMER)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SIP_T1_TIMER";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_T1_TIMER;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_T2_TIMER) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_T2_TIMER)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SIP_T2_TIMER";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_T2_TIMER;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_TF_TIMER) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_TF_TIMER)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SIP_TF_TIMER";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_TF_TIMER;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VLT_SETTING_ENABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VLT_SETTING_ENABLED)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_VLT_SETTING_ENABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VLT_SETTING_ENABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_LVC_SETTING_ENABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_LVC_SETTING_ENABLED)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_LVC_SETTING_ENABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_LVC_SETTING_ENABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_DOMAIN_NAME) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_DOMAIN_NAME)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_DOMAIN_NAME";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_DOMAIN_NAME;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SMS_FORMAT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SMS_FORMAT)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SMS_FORMAT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SMS_FORMAT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SMS_OVER_IP) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SMS_OVER_IP)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SMS_OVER_IP";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SMS_OVER_IP;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_PUBLISH_TIMER) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_PUBLISH_TIMER)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_PUBLISH_TIMER";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_PUBLISH_TIMER;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_PUBLISH_TIMER_EXTENDED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_PUBLISH_TIMER_EXTENDED)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_PUBLISH_TIMER_EXTENDED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_PUBLISH_TIMER_EXTENDED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_GZIP_FLAG) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_GZIP_FLAG)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_GZIP_FLAG";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_GZIP_FLAG;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_EAB_SETTING_ENABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_EAB_SETTING_ENABLED)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_EAB_SETTING_ENABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_EAB_SETTING_ENABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_MOBILE_DATA_ENABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_MOBILE_DATA_ENABLED)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_MOBILE_DATA_ENABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_MOBILE_DATA_ENABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_MODE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_MODE)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_VOICE_OVER_WIFI_MODE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_MODE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_CAPABILITY_DISCOVERY_ENABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_CAPABILITY_DISCOVERY_ENABLED)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_CAPABILITY_DISCOVERY_ENABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_CAPABILITY_DISCOVERY_ENABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_EMERGENCY_CALL_TIMER) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_EMERGENCY_CALL_TIMER)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_EMERGENCY_CALL_TIMER";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_EMERGENCY_CALL_TIMER;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SSAC_HYSTERESIS_TIMER) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SSAC_HYSTERESIS_TIMER)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SSAC_HYSTERESIS_TIMER";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SSAC_HYSTERESIS_TIMER;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_LBO_PCSCF_ADDRESS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_LBO_PCSCF_ADDRESS)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_LBO_PCSCF_ADDRESS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_LBO_PCSCF_ADDRESS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_KEEP_ALIVE_ENABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_KEEP_ALIVE_ENABLED)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_KEEP_ALIVE_ENABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_KEEP_ALIVE_ENABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_REGISTRATION_RETRY_BASE_TIME_SEC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_REGISTRATION_RETRY_BASE_TIME_SEC)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_REGISTRATION_RETRY_BASE_TIME_SEC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_REGISTRATION_RETRY_BASE_TIME_SEC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_REGISTRATION_RETRY_MAX_TIME_SEC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_REGISTRATION_RETRY_MAX_TIME_SEC)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_REGISTRATION_RETRY_MAX_TIME_SEC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_REGISTRATION_RETRY_MAX_TIME_SEC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SPEECH_START_PORT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SPEECH_START_PORT)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SPEECH_START_PORT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SPEECH_START_PORT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SPEECH_END_PORT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SPEECH_END_PORT)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SPEECH_END_PORT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SPEECH_END_PORT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_INVITE_REQ_RETX_INTERVAL_MSEC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_INVITE_REQ_RETX_INTERVAL_MSEC)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SIP_INVITE_REQ_RETX_INTERVAL_MSEC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_INVITE_REQ_RETX_INTERVAL_MSEC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_WAIT_TIME_MSEC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_WAIT_TIME_MSEC)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SIP_INVITE_RSP_WAIT_TIME_MSEC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_WAIT_TIME_MSEC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_RETX_INTERVAL_MSEC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_RETX_INTERVAL_MSEC)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SIP_INVITE_RSP_RETX_INTERVAL_MSEC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_RETX_INTERVAL_MSEC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_ACK_RECEIPT_WAIT_TIME_MSEC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_ACK_RECEIPT_WAIT_TIME_MSEC)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SIP_ACK_RECEIPT_WAIT_TIME_MSEC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_ACK_RECEIPT_WAIT_TIME_MSEC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_ACK_RETX_WAIT_TIME_MSEC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_ACK_RETX_WAIT_TIME_MSEC)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SIP_ACK_RETX_WAIT_TIME_MSEC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_ACK_RETX_WAIT_TIME_MSEC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AMR_WB_BANDWIDTH_EFFICIENT_PT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AMR_WB_BANDWIDTH_EFFICIENT_PT)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_AMR_WB_BANDWIDTH_EFFICIENT_PT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AMR_WB_BANDWIDTH_EFFICIENT_PT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AMR_OCTET_ALIGNED_PT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AMR_OCTET_ALIGNED_PT)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_AMR_OCTET_ALIGNED_PT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AMR_OCTET_ALIGNED_PT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AMR_BANDWIDTH_EFFICIENT_PT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AMR_BANDWIDTH_EFFICIENT_PT)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_AMR_BANDWIDTH_EFFICIENT_PT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AMR_BANDWIDTH_EFFICIENT_PT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_DTMF_WB_PT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_DTMF_WB_PT)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_DTMF_WB_PT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_DTMF_WB_PT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_DTMF_NB_PT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_DTMF_NB_PT)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_DTMF_NB_PT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_DTMF_NB_PT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AMR_DEFAULT_MODE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AMR_DEFAULT_MODE)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_AMR_DEFAULT_MODE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AMR_DEFAULT_MODE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SMS_PSI) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SMS_PSI)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SMS_PSI";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SMS_PSI;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VIDEO_QUALITY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VIDEO_QUALITY)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_VIDEO_QUALITY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VIDEO_QUALITY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE1) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE1)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_THRESHOLD_LTE1";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE1;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE2) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE2)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_THRESHOLD_LTE2";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE2;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE3) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE3)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_THRESHOLD_LTE3";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE3;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_1x) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_1x)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_THRESHOLD_1x";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_1x;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_WIFI_A) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_WIFI_A)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_THRESHOLD_WIFI_A";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_WIFI_A;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_WIFI_B) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_WIFI_B)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_THRESHOLD_WIFI_B";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_WIFI_B;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_T_EPDG_LTE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_T_EPDG_LTE)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_T_EPDG_LTE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_T_EPDG_LTE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_T_EPDG_WIFI) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_T_EPDG_WIFI)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_T_EPDG_WIFI";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_T_EPDG_WIFI;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_T_EPDG_1x) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_T_EPDG_1x)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_T_EPDG_1x";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_T_EPDG_1x;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VWF_SETTING_ENABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VWF_SETTING_ENABLED)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_VWF_SETTING_ENABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VWF_SETTING_ENABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VCE_SETTING_ENABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VCE_SETTING_ENABLED)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_VCE_SETTING_ENABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VCE_SETTING_ENABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_RTT_SETTING_ENABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_RTT_SETTING_ENABLED)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_RTT_SETTING_ENABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_RTT_SETTING_ENABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SMS_APP) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SMS_APP)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_SMS_APP";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SMS_APP;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VVM_APP) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VVM_APP)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_VVM_APP";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VVM_APP;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_INVALID)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_INVALID;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE)) {
        os += (first ? "" : " | ");
        os += "CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_5::ConfigItem o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_NONE) {
        return "CONFIG_ITEM_NONE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOCODER_AMRMODESET) {
        return "CONFIG_ITEM_VOCODER_AMRMODESET";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOCODER_AMRWBMODESET) {
        return "CONFIG_ITEM_VOCODER_AMRWBMODESET";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_SESSION_TIMER) {
        return "CONFIG_ITEM_SIP_SESSION_TIMER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_MIN_SESSION_EXPIRY) {
        return "CONFIG_ITEM_MIN_SESSION_EXPIRY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_CANCELLATION_TIMER) {
        return "CONFIG_ITEM_CANCELLATION_TIMER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_T_DELAY) {
        return "CONFIG_ITEM_T_DELAY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SILENT_REDIAL_ENABLE) {
        return "CONFIG_ITEM_SILENT_REDIAL_ENABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_T1_TIMER) {
        return "CONFIG_ITEM_SIP_T1_TIMER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_T2_TIMER) {
        return "CONFIG_ITEM_SIP_T2_TIMER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_TF_TIMER) {
        return "CONFIG_ITEM_SIP_TF_TIMER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VLT_SETTING_ENABLED) {
        return "CONFIG_ITEM_VLT_SETTING_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_LVC_SETTING_ENABLED) {
        return "CONFIG_ITEM_LVC_SETTING_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_DOMAIN_NAME) {
        return "CONFIG_ITEM_DOMAIN_NAME";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SMS_FORMAT) {
        return "CONFIG_ITEM_SMS_FORMAT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SMS_OVER_IP) {
        return "CONFIG_ITEM_SMS_OVER_IP";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_PUBLISH_TIMER) {
        return "CONFIG_ITEM_PUBLISH_TIMER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_PUBLISH_TIMER_EXTENDED) {
        return "CONFIG_ITEM_PUBLISH_TIMER_EXTENDED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION) {
        return "CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION) {
        return "CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL) {
        return "CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH) {
        return "CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL) {
        return "CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP) {
        return "CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_GZIP_FLAG) {
        return "CONFIG_ITEM_GZIP_FLAG";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_EAB_SETTING_ENABLED) {
        return "CONFIG_ITEM_EAB_SETTING_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_MOBILE_DATA_ENABLED) {
        return "CONFIG_ITEM_MOBILE_DATA_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED) {
        return "CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING) {
        return "CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_MODE) {
        return "CONFIG_ITEM_VOICE_OVER_WIFI_MODE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_CAPABILITY_DISCOVERY_ENABLED) {
        return "CONFIG_ITEM_CAPABILITY_DISCOVERY_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_EMERGENCY_CALL_TIMER) {
        return "CONFIG_ITEM_EMERGENCY_CALL_TIMER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SSAC_HYSTERESIS_TIMER) {
        return "CONFIG_ITEM_SSAC_HYSTERESIS_TIMER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS) {
        return "CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_LBO_PCSCF_ADDRESS) {
        return "CONFIG_ITEM_LBO_PCSCF_ADDRESS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_KEEP_ALIVE_ENABLED) {
        return "CONFIG_ITEM_KEEP_ALIVE_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_REGISTRATION_RETRY_BASE_TIME_SEC) {
        return "CONFIG_ITEM_REGISTRATION_RETRY_BASE_TIME_SEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_REGISTRATION_RETRY_MAX_TIME_SEC) {
        return "CONFIG_ITEM_REGISTRATION_RETRY_MAX_TIME_SEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SPEECH_START_PORT) {
        return "CONFIG_ITEM_SPEECH_START_PORT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SPEECH_END_PORT) {
        return "CONFIG_ITEM_SPEECH_END_PORT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_INVITE_REQ_RETX_INTERVAL_MSEC) {
        return "CONFIG_ITEM_SIP_INVITE_REQ_RETX_INTERVAL_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_WAIT_TIME_MSEC) {
        return "CONFIG_ITEM_SIP_INVITE_RSP_WAIT_TIME_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC) {
        return "CONFIG_ITEM_SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC) {
        return "CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC) {
        return "CONFIG_ITEM_SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_RETX_INTERVAL_MSEC) {
        return "CONFIG_ITEM_SIP_INVITE_RSP_RETX_INTERVAL_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_ACK_RECEIPT_WAIT_TIME_MSEC) {
        return "CONFIG_ITEM_SIP_ACK_RECEIPT_WAIT_TIME_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_ACK_RETX_WAIT_TIME_MSEC) {
        return "CONFIG_ITEM_SIP_ACK_RETX_WAIT_TIME_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC) {
        return "CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC) {
        return "CONFIG_ITEM_SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT) {
        return "CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AMR_WB_BANDWIDTH_EFFICIENT_PT) {
        return "CONFIG_ITEM_AMR_WB_BANDWIDTH_EFFICIENT_PT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AMR_OCTET_ALIGNED_PT) {
        return "CONFIG_ITEM_AMR_OCTET_ALIGNED_PT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AMR_BANDWIDTH_EFFICIENT_PT) {
        return "CONFIG_ITEM_AMR_BANDWIDTH_EFFICIENT_PT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_DTMF_WB_PT) {
        return "CONFIG_ITEM_DTMF_WB_PT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_DTMF_NB_PT) {
        return "CONFIG_ITEM_DTMF_NB_PT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AMR_DEFAULT_MODE) {
        return "CONFIG_ITEM_AMR_DEFAULT_MODE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SMS_PSI) {
        return "CONFIG_ITEM_SMS_PSI";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VIDEO_QUALITY) {
        return "CONFIG_ITEM_VIDEO_QUALITY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE1) {
        return "CONFIG_ITEM_THRESHOLD_LTE1";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE2) {
        return "CONFIG_ITEM_THRESHOLD_LTE2";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE3) {
        return "CONFIG_ITEM_THRESHOLD_LTE3";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_1x) {
        return "CONFIG_ITEM_THRESHOLD_1x";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_WIFI_A) {
        return "CONFIG_ITEM_THRESHOLD_WIFI_A";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_WIFI_B) {
        return "CONFIG_ITEM_THRESHOLD_WIFI_B";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_T_EPDG_LTE) {
        return "CONFIG_ITEM_T_EPDG_LTE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_T_EPDG_WIFI) {
        return "CONFIG_ITEM_T_EPDG_WIFI";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_T_EPDG_1x) {
        return "CONFIG_ITEM_T_EPDG_1x";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VWF_SETTING_ENABLED) {
        return "CONFIG_ITEM_VWF_SETTING_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VCE_SETTING_ENABLED) {
        return "CONFIG_ITEM_VCE_SETTING_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_RTT_SETTING_ENABLED) {
        return "CONFIG_ITEM_RTT_SETTING_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SMS_APP) {
        return "CONFIG_ITEM_SMS_APP";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VVM_APP) {
        return "CONFIG_ITEM_VVM_APP";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_INVALID) {
        return "CONFIG_ITEM_INVALID";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE) {
        return "CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_5::ConfigInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".item = ";
    os += ::vendor::qti::hardware::radio::ims::V1_5::toString(o.item);
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

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_5::ConfigInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_5::ConfigInfo& rhs) {
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

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_5::ConfigInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_5::ConfigInfo& rhs){
    return !(lhs == rhs);
}

template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_5::Result>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_5::Result> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::Result::SUCCESS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::Result::SUCCESS)) {
        os += (first ? "" : " | ");
        os += "SUCCESS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::Result::SUCCESS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_5::Result::FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_5::Result::FAILURE)) {
        os += (first ? "" : " | ");
        os += "FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_5::Result::FAILURE;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_5::Result o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::Result::SUCCESS) {
        return "SUCCESS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_5::Result::FAILURE) {
        return "FAILURE";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatus& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".reason = ";
    os += ::android::hardware::toString(o.reason);
    os += ", .status = ";
    os += ::vendor::qti::hardware::radio::ims::V1_5::toString(o.status);
    os += ", .errorDetails = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.errorDetails);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatus& lhs, const ::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatus& rhs) {
    if (lhs.reason != rhs.reason) {
        return false;
    }
    if (lhs.status != rhs.status) {
        return false;
    }
    if (lhs.errorDetails != rhs.errorDetails) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatus& lhs, const ::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatus& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatusInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".errorDetails = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.errorDetails);
    os += ", .status = ";
    os += ::android::hardware::toString(o.status);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatusInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatusInfo& rhs) {
    if (lhs.errorDetails != rhs.errorDetails) {
        return false;
    }
    if (lhs.status != rhs.status) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatusInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatusInfo& rhs){
    return !(lhs == rhs);
}


}  // namespace V1_5
}  // namespace ims
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

//
// global type declarations for package
//

namespace android {
namespace hardware {
namespace details {
template<> constexpr std::array<::vendor::qti::hardware::radio::ims::V1_5::CallFailCause, 134> hidl_enum_values<::vendor::qti::hardware::radio::ims::V1_5::CallFailCause> = {
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_UNOBTAINABLE_NUMBER,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NORMAL,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_BUSY,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NORMAL_UNSPECIFIED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CONGESTION,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_INCOMPATIBILITY_DESTINATION,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_BARRED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_FDN_BLOCKED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_IMEI_NOT_ACCEPTED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_USSD,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_SS,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_EMERGENCY_TEMP_FAILURE,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_EMERGENCY_PERM_FAILURE,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_HO_NOT_FEASIBLE,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_USER_BUSY,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_USER_REJECT,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_LOW_BATTERY,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_BLACKLISTED_CALL_ID,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CS_RETRY_REQUIRED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NETWORK_UNAVAILABLE,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_FEATURE_UNAVAILABLE,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_ERROR,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MISC,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_ANSWERED_ELSEWHERE,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_PULL_OUT_OF_SYNC,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CAUSE_CALL_PULLED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_ACCESS_CLASS_BLOCKED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REDIRECTED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_BAD_REQUEST,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_FORBIDDEN,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_NOT_FOUND,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_NOT_SUPPORTED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_TIMEOUT,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_BAD_ADDRESS,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_BUSY,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_CANCELLED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_NOT_ACCEPTABLE,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_NOT_REACHABLE,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_INTERNAL_ERROR,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_BAD_GATEWAY,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVICE_UNAVAILABLE,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_TIMEOUT,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_USER_REJECTED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_GLOBAL_ERROR,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MEDIA_INIT_FAILED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MEDIA_NO_DATA,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MEDIA_NOT_ACCEPTABLE,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MEDIA_UNSPECIFIED_ERROR,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_HOLD_RESUME_FAILED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_HOLD_RESUME_CANCELED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_HOLD_REINVITE_COLLISION,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_ERROR_UNSPECIFIED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_INVALID,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_ALTERNATE_EMERGENCY_CALL,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_OFF,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OUT_OF_SERVICE,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NO_VALID_SIM,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_INTERNAL_ERROR,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NETWORK_RESP_TIMEOUT,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NETWORK_REJECT,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_ACCESS_FAILURE,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_LINK_FAILURE,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_LINK_LOST,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_UPLINK_FAILURE,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_SETUP_FAILURE,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_RELEASE_NORMAL,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RADIO_RELEASE_ABNORMAL,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NETWORK_DETACH,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_1,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_2,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_3,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_4,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_5,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_6,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_7,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_8,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_9,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_10,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_11,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_12,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_13,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_14,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_OEM_CAUSE_15,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_NO_CSFB_IN_CS_ROAM,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SRV_NOT_REGISTERED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_TYPE_NOT_ALLOWED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_EMRG_CALL_ONGOING,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_SETUP_ONGOING,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_MAX_CALL_LIMIT_REACHED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_UNSUPPORTED_SIP_HDRS,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_TRANSFER_ONGOING,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_PRACK_TIMEOUT,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_QOS_FAILURE,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_ONGOING_HANDOVER,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_VT_WITH_TTY_NOT_ALLOWED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_UPGRADE_ONGOING,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CONFERENCE_WITH_TTY_NOT_ALLOWED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_CONFERENCE_ONGOING,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_VT_WITH_AVPF_NOT_ALLOWED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_ENCRYPTION_CALL_ONGOING,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_ONGOING_CW_DISABLED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CALL_ON_OTHER_SUB,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_1X_COLLISION,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_UI_NOT_READY,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_CS_CALL_ONGOING,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_USER_MARKED_UNWANTED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_REJECTED_ELSEWHERE,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_USER_REJECTED_SESSION_MODIFICATION,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_USER_CANCELLED_SESSION_MODIFICATION,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SESSION_MODIFICATION_FAILED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_METHOD_NOT_ALLOWED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_PROXY_AUTHENTICATION_REQUIRED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_ENTITY_TOO_LARGE,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_URI_TOO_LARGE,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_EXTENSION_REQUIRED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_INTERVAL_TOO_BRIEF,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_CALL_OR_TRANS_DOES_NOT_EXIST,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_LOOP_DETECTED,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_TOO_MANY_HOPS,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_AMBIGUOUS,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_PENDING,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_SIP_UNDECIPHERABLE,
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCause::CALL_FAIL_RETRY_ON_IMS_WITHOUT_RTT,
};
}  // namespace details
}  // namespace hardware
}  // namespace android

namespace android {
namespace hardware {
namespace details {
template<> constexpr std::array<::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory, 8> hidl_enum_values<::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory> = {
    ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::UNSPECIFIED,
    ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::POLICE,
    ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::AMBULANCE,
    ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::FIRE_BRIGADE,
    ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::MARINE_GUARD,
    ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::MOUNTAIN_RESCUE,
    ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::MIEC,
    ::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory::AIEC,
};
}  // namespace details
}  // namespace hardware
}  // namespace android

namespace android {
namespace hardware {
namespace details {
template<> constexpr std::array<::vendor::qti::hardware::radio::ims::V1_5::TirMode, 3> hidl_enum_values<::vendor::qti::hardware::radio::ims::V1_5::TirMode> = {
    ::vendor::qti::hardware::radio::ims::V1_5::TirMode::INVALID,
    ::vendor::qti::hardware::radio::ims::V1_5::TirMode::TEMPORARY,
    ::vendor::qti::hardware::radio::ims::V1_5::TirMode::PERMANENT,
};
}  // namespace details
}  // namespace hardware
}  // namespace android

namespace android {
namespace hardware {
namespace details {
template<> constexpr std::array<::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute, 3> hidl_enum_values<::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute> = {
    ::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute::UNKNOWN,
    ::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute::EMERGENCY,
    ::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute::NORMAL,
};
}  // namespace details
}  // namespace hardware
}  // namespace android

namespace android {
namespace hardware {
namespace details {
template<> constexpr std::array<::vendor::qti::hardware::radio::ims::V1_5::UssdModeType, 6> hidl_enum_values<::vendor::qti::hardware::radio::ims::V1_5::UssdModeType> = {
    ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::NOTIFY,
    ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::REQUEST,
    ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::NW_RELEASE,
    ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::LOCAL_CLIENT,
    ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::NOT_SUPPORTED,
    ::vendor::qti::hardware::radio::ims::V1_5::UssdModeType::NW_TIMEOUT,
};
}  // namespace details
}  // namespace hardware
}  // namespace android

namespace android {
namespace hardware {
namespace details {
template<> constexpr std::array<::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason, 25> hidl_enum_values<::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason> = {
    ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_NONE,
    ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_GENERIC_FAILURE,
    ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_RADIO_OFF,
    ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_NULL_PDU,
    ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_NO_SERVICE,
    ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_LIMIT_EXCEEDED,
    ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_SHORT_CODE_NOT_ALLOWED,
    ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_SHORT_CODE_NEVER_ALLOWED,
    ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ERROR_FDN_CHECK_FAILURE,
    ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_RADIO_NOT_AVAILABLE,
    ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_NETWORK_REJECT,
    ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_INVALID_ARGUMENTS,
    ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_INVALID_STATE,
    ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_NO_MEMORY,
    ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_INVALID_SMS_FORMAT,
    ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_SYSTEM_ERROR,
    ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_MODEM_ERROR,
    ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_NETWORK_ERROR,
    ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_ENCODING_ERROR,
    ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_INVALID_SMSC_ADDRESS,
    ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_OPERATION_NOT_ALLOWED,
    ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_INTERNAL_ERROR,
    ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_NO_RESOURCES,
    ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_CANCELLED,
    ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason::RESULT_REQUEST_NOT_SUPPORTED,
};
}  // namespace details
}  // namespace hardware
}  // namespace android

namespace android {
namespace hardware {
namespace details {
template<> constexpr std::array<::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult, 4> hidl_enum_values<::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult> = {
    ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult::DELIVER_STATUS_OK,
    ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult::DELIVER_STATUS_ERROR,
    ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult::DELIVER_STATUS_ERROR_NO_MEMORY,
    ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult::DELIVER_STATUS_ERROR_REQUEST_NOT_SUPPORTED,
};
}  // namespace details
}  // namespace hardware
}  // namespace android

namespace android {
namespace hardware {
namespace details {
template<> constexpr std::array<::vendor::qti::hardware::radio::ims::V1_5::ConfigItem, 75> hidl_enum_values<::vendor::qti::hardware::radio::ims::V1_5::ConfigItem> = {
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_NONE,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOCODER_AMRMODESET,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOCODER_AMRWBMODESET,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_SESSION_TIMER,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_MIN_SESSION_EXPIRY,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_CANCELLATION_TIMER,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_T_DELAY,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SILENT_REDIAL_ENABLE,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_T1_TIMER,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_T2_TIMER,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_TF_TIMER,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VLT_SETTING_ENABLED,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_LVC_SETTING_ENABLED,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_DOMAIN_NAME,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SMS_FORMAT,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SMS_OVER_IP,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_PUBLISH_TIMER,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_PUBLISH_TIMER_EXTENDED,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_GZIP_FLAG,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_EAB_SETTING_ENABLED,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_MOBILE_DATA_ENABLED,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_MODE,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_CAPABILITY_DISCOVERY_ENABLED,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_EMERGENCY_CALL_TIMER,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SSAC_HYSTERESIS_TIMER,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_LBO_PCSCF_ADDRESS,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_KEEP_ALIVE_ENABLED,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_REGISTRATION_RETRY_BASE_TIME_SEC,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_REGISTRATION_RETRY_MAX_TIME_SEC,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SPEECH_START_PORT,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SPEECH_END_PORT,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_INVITE_REQ_RETX_INTERVAL_MSEC,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_WAIT_TIME_MSEC,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_RETX_INTERVAL_MSEC,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_ACK_RECEIPT_WAIT_TIME_MSEC,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_ACK_RETX_WAIT_TIME_MSEC,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AMR_WB_BANDWIDTH_EFFICIENT_PT,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AMR_OCTET_ALIGNED_PT,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AMR_BANDWIDTH_EFFICIENT_PT,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_DTMF_WB_PT,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_DTMF_NB_PT,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_AMR_DEFAULT_MODE,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SMS_PSI,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VIDEO_QUALITY,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE1,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE2,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE3,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_1x,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_WIFI_A,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_THRESHOLD_WIFI_B,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_T_EPDG_LTE,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_T_EPDG_WIFI,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_T_EPDG_1x,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VWF_SETTING_ENABLED,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VCE_SETTING_ENABLED,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_RTT_SETTING_ENABLED,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_SMS_APP,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VVM_APP,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_INVALID,
    ::vendor::qti::hardware::radio::ims::V1_5::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE,
};
}  // namespace details
}  // namespace hardware
}  // namespace android

namespace android {
namespace hardware {
namespace details {
template<> constexpr std::array<::vendor::qti::hardware::radio::ims::V1_5::Result, 2> hidl_enum_values<::vendor::qti::hardware::radio::ims::V1_5::Result> = {
    ::vendor::qti::hardware::radio::ims::V1_5::Result::SUCCESS,
    ::vendor::qti::hardware::radio::ims::V1_5::Result::FAILURE,
};
}  // namespace details
}  // namespace hardware
}  // namespace android


#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_5_TYPES_H
