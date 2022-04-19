/******************************************************************************
#  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once

#include <string>
#include <vector>
#include "interfaces/common.h"
#include "interfaces/ims/ims.h"

namespace qcril {
namespace interfaces {

enum class CallState {
  UNKNOWN,
  ACTIVE,
  HOLDING,
  DIALING,  /* MO call only */
  ALERTING, /* MO call only */
  INCOMING, /* MT call only */
  WAITING,  /* MT call only */
  END,
};
inline std::string toString(const CallState &o) {
  if (o == CallState::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == CallState::ACTIVE) {
    return "ACTIVE";
  }
  if (o == CallState::HOLDING) {
    return "HOLDING";
  }
  if (o == CallState::DIALING) {
    return "DIALING";
  }
  if (o == CallState::ALERTING) {
    return "ALERTING";
  }
  if (o == CallState::INCOMING) {
    return "INCOMING";
  }
  if (o == CallState::WAITING) {
    return "WAITING";
  }
  if (o == CallState::END) {
    return "END";
  }
  return "<invalid>";
}

enum class CallDomain {
  UNKNOWN,
  CS,        /* Circuit Switched Domain */
  PS,        /* Packet Switched Domain */
  AUTOMATIC, /* Automatic domain - domain for a new call should be selected by modem */
  NOT_SET
};

inline std::string toString(const CallDomain &o) {
  if (o == CallDomain::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == CallDomain::CS) {
    return "CS";
  }
  if (o == CallDomain::PS) {
    return "PS";
  }
  if (o == CallDomain::AUTOMATIC) {
    return "AUTOMATIC";
  }
  if (o == CallDomain::NOT_SET) {
    return "NOT_SET";
  }
  return "<invalid>";
}

enum class CallSubState {
  UNDEFINED = 0,
  AUDIO_CONNECTED_SUSPENDED = 1,
  VIDEO_CONNECTED_SUSPENDED = 2,
  AVP_RETRY = 4,
  MEDIA_PAUSED = 8,
};

enum class RIL_EccCategoryMask {
  UNKNOWN = 1 << 0,
  POLICE = 1 << 1,          /* Police */
  AMBULANCE = 1 << 2,       /* Ambulance */
  FIRE_BRIGADE = 1 << 3,    /*Fire brigade */
  MARINE_GUARD = 1 << 4,    /* Marine guard */
  MOUNTAIN_RESCUE = 1 << 5, /* Mountain rescue */
  MANUAL_ECALL = 1 << 6,    /* Manual ECALL */
  AUTO_ECALL   = 1 << 7     /* Auto ECALL */
};

enum class EmergencyCallRouting {
  UNKNOWN,
  EMERGENCY,       /* Dial call as emergency */
  NORMAL,          /* Dial call as normal voice call for fake ecc */
};
std::string toString(const EmergencyCallRouting &o);

inline std::string toString(const EmergencyCallRouting &o) {
  if (o == EmergencyCallRouting::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == EmergencyCallRouting::EMERGENCY) {
    return "EMERGENCY";
  }
  if (o == EmergencyCallRouting::NORMAL) {
    return "NORMAL";
  }
  return "<invalid>";
}

enum class Presentation {
  UNKNOWN,
  ALLOWED,    /* Presentation allowed */
  RESTRICTED, /* Presentation restricted */
  DEFAULT
};
inline std::string toString(const Presentation &o) {
  if (o == Presentation::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == Presentation::ALLOWED) {
    return "ALLOWED";
  }
  if (o == Presentation::RESTRICTED) {
    return "RESTRICTED";
  }
  if (o == Presentation::DEFAULT) {
    return "DEFAULT";
  }
  return "<invalid>";
}

enum class CallFailCause {
  UNKNOWN = 0,
  UNOBTAINABLE_NUMBER = 1,
  NO_ROUTE_TO_DESTINATION = 3,
  CHANNEL_UNACCEPTABLE = 6,
  OPERATOR_DETERMINED_BARRING = 8,
  NORMAL = 16,
  BUSY = 17,
  NO_USER_RESPONDING = 18,
  NO_ANSWER_FROM_USER = 19,
  SUBSCRIBER_ABSENT = 20,
  CALL_REJECTED = 21,
  NUMBER_CHANGED = 22,
  PREEMPTION = 25,
  DESTINATION_OUT_OF_ORDER = 27,
  INVALID_NUMBER_FORMAT = 28,
  FACILITY_REJECTED = 29,
  RESP_TO_STATUS_ENQUIRY = 30,
  NORMAL_UNSPECIFIED = 31,
  CONGESTION = 34,
  NETWORK_OUT_OF_ORDER = 38,
  TEMPORARY_FAILURE = 41,
  SWITCHING_EQUIPMENT_CONGESTION = 42,
  ACCESS_INFORMATION_DISCARDED = 43,
  REQUESTED_CIRCUIT_OR_CHANNEL_NOT_AVAILABLE = 44,
  RESOURCES_UNAVAILABLE_OR_UNSPECIFIED = 47,
  QOS_UNAVAILABLE = 49,
  REQUESTED_FACILITY_NOT_SUBSCRIBED = 50,
  INCOMING_CALLS_BARRED_WITHIN_CUG = 55,
  BEARER_CAPABILITY_NOT_AUTHORIZED = 57,
  BEARER_CAPABILITY_UNAVAILABLE = 58,
  SERVICE_OPTION_NOT_AVAILABLE = 63,
  BEARER_SERVICE_NOT_IMPLEMENTED = 65,
  ACM_LIMIT_EXCEEDED = 68,
  REQUESTED_FACILITY_NOT_IMPLEMENTED = 69,
  ONLY_DIGITAL_INFORMATION_BEARER_AVAILABLE = 70,
  SERVICE_OR_OPTION_NOT_IMPLEMENTED = 79,
  INVALID_TRANSACTION_IDENTIFIER = 81,
  USER_NOT_MEMBER_OF_CUG = 87,
  INCOMPATIBLE_DESTINATION = 88,
  INVALID_TRANSIT_NW_SELECTION = 91,
  SEMANTICALLY_INCORRECT_MESSAGE = 95,
  INVALID_MANDATORY_INFORMATION = 96,
  MESSAGE_TYPE_NON_IMPLEMENTED = 97,
  MESSAGE_TYPE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE = 98,
  INFORMATION_ELEMENT_NON_EXISTENT = 99,
  CONDITIONAL_IE_ERROR = 100,
  MESSAGE_NOT_COMPATIBLE_WITH_PROTOCOL_STATE = 101,
  RECOVERY_ON_TIMER_EXPIRED = 102,
  PROTOCOL_ERROR_UNSPECIFIED = 111,
  INTERWORKING_UNSPECIFIED = 127,
  CALL_BARRED = 240,
  FDN_BLOCKED = 241,
  IMSI_UNKNOWN_IN_VLR = 242,
  IMEI_NOT_ACCEPTED = 243,
  DIAL_MODIFIED_TO_USSD = 244, /* STK Call Control */
  DIAL_MODIFIED_TO_SS = 245,   /* STK Call Control */
  DIAL_MODIFIED_TO_DIAL = 246, /* STK Call Control */
  RADIO_OFF = 247, /* Radio is OFF */
  OUT_OF_SERVICE = 248, /* No cellular coverage */
  NO_VALID_SIM = 249, /* No valid SIM is present */
  RADIO_INTERNAL_ERROR = 250, /* Internal error at Modem */
  NETWORK_RESP_TIMEOUT = 251, /* No response from network */
  NETWORK_REJECT = 252, /* Explicit network reject */
  RADIO_ACCESS_FAILURE = 253, /* RRC connection failure. Eg.RACH */
  RADIO_LINK_FAILURE = 254, /* Radio Link Failure */
  RADIO_LINK_LOST = 255, /* Radio link lost due to poor coverage */
  RADIO_UPLINK_FAILURE = 256, /* Radio uplink failure */
  RADIO_SETUP_FAILURE = 257, /* RRC connection setup failure */
  RADIO_RELEASE_NORMAL = 258, /* RRC connection release, normal */
  RADIO_RELEASE_ABNORMAL = 259, /* RRC connection release, abnormal */
  ACCESS_CLASS_BLOCKED = 260, /* Access class barring */
  NETWORK_DETACH = 261, /* Explicit network detach */
  EMERGENCY_TEMP_FAILURE = 325, /* CALL_END_CAUSE_TEMP_REDIAL_ALLOWED = 0x145 */
  EMERGENCY_PERM_FAILURE = 326, /* CALL_END_CAUSE_PERM_REDIAL_NOT_NEEDED = 0x146 */
  CDMA_LOCKED_UNTIL_POWER_CYCLE = 1000,
  CDMA_DROP = 1001,
  CDMA_INTERCEPT = 1002,
  CDMA_REORDER = 1003,
  CDMA_SO_REJECT = 1004,
  CDMA_RETRY_ORDER = 1005,
  CDMA_ACCESS_FAILURE = 1006,
  CDMA_PREEMPTED = 1007,
  CDMA_NOT_EMERGENCY = 1008, /* For non-emergency number dialed during emergency callback mode */
  CDMA_ACCESS_BLOCKED = 1009, /* CDMA network access probes blocked */
  INCOMPATIBILITY_DESTINATION,
  HO_NOT_FEASIBLE,     /* Call was ended due to LTE to 3G/2G handover not feasible*/
  USER_BUSY,           /* User ends a call.*/
  USER_REJECT,         /* User rejects incoming call.*/
  LOW_BATTERY,         /* Call end/reject due to low battery.*/
  BLACKLISTED_CALL_ID, /* Call end/reject due to blacklisted caller.*/
  CS_RETRY_REQUIRED,   /* Call end due to CS Fallback request from lower layers.*/
  NETWORK_UNAVAILABLE,
  FEATURE_UNAVAILABLE,
  SIP_ERROR,
  MISC,
  ANSWERED_ELSEWHERE,
  PULL_OUT_OF_SYNC,
  CAUSE_CALL_PULLED,
  SIP_REDIRECTED,
  SIP_BAD_REQUEST,
  SIP_FORBIDDEN,
  SIP_NOT_FOUND,
  SIP_NOT_SUPPORTED,
  SIP_REQUEST_TIMEOUT,
  SIP_TEMPORARILY_UNAVAILABLE,
  SIP_BAD_ADDRESS,
  SIP_BUSY,
  SIP_REQUEST_CANCELLED,
  SIP_NOT_ACCEPTABLE,
  SIP_NOT_REACHABLE,
  SIP_SERVER_INTERNAL_ERROR,
  SIP_SERVER_NOT_IMPLEMENTED,
  SIP_SERVER_BAD_GATEWAY,
  SIP_SERVICE_UNAVAILABLE,
  SIP_SERVER_TIMEOUT,
  SIP_SERVER_VERSION_UNSUPPORTED,
  SIP_SERVER_MESSAGE_TOOLARGE,
  SIP_SERVER_PRECONDITION_FAILURE,
  SIP_USER_REJECTED,
  SIP_GLOBAL_ERROR,
  MEDIA_INIT_FAILED,
  MEDIA_NO_DATA,
  MEDIA_NOT_ACCEPTABLE,
  MEDIA_UNSPECIFIED_ERROR,
  HOLD_RESUME_FAILED,
  HOLD_RESUME_CANCELED,
  HOLD_REINVITE_COLLISION,
  DIAL_MODIFIED_TO_DIAL_VIDEO,       /* STK Call Control */
  DIAL_VIDEO_MODIFIED_TO_DIAL,       /* STK Call Control */
  DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO, /* STK Call Control */
  DIAL_VIDEO_MODIFIED_TO_SS,         /* STK Call Control */
  DIAL_VIDEO_MODIFIED_TO_USSD,       /* STK Call Control */
  SIP_ALTERNATE_EMERGENCY_CALL,
  NO_CSFB_IN_CS_ROAM,
  SRV_NOT_REGISTERED,
  CALL_TYPE_NOT_ALLOWED,
  EMRG_CALL_ONGOING,
  CALL_SETUP_ONGOING,
  MAX_CALL_LIMIT_REACHED,
  UNSUPPORTED_SIP_HDRS,
  CALL_TRANSFER_ONGOING,
  PRACK_TIMEOUT,
  QOS_FAILURE,
  ONGOING_HANDOVER,
  VT_WITH_TTY_NOT_ALLOWED,
  CALL_UPGRADE_ONGOING,
  CONFERENCE_WITH_TTY_NOT_ALLOWED,
  CALL_CONFERENCE_ONGOING,
  VT_WITH_AVPF_NOT_ALLOWED,
  ENCRYPTION_CALL_ONGOING,
  CALL_ONGOING_CW_DISABLED,
  CALL_ON_OTHER_SUB,
  ONE_X_COLLISION,
  UI_NOT_READY,
  CS_CALL_ONGOING,
  SIP_USER_MARKED_UNWANTED,
  REJECTED_ELSEWHERE,
  USER_REJECTED_SESSION_MODIFICATION,
  USER_CANCELLED_SESSION_MODIFICATION,
  SESSION_MODIFICATION_FAILED,
  SIP_METHOD_NOT_ALLOWED,
  SIP_PROXY_AUTHENTICATION_REQUIRED,
  SIP_REQUEST_ENTITY_TOO_LARGE,
  SIP_REQUEST_URI_TOO_LARGE,
  SIP_EXTENSION_REQUIRED,
  SIP_INTERVAL_TOO_BRIEF,
  SIP_CALL_OR_TRANS_DOES_NOT_EXIST,
  SIP_LOOP_DETECTED,
  SIP_TOO_MANY_HOPS,
  SIP_AMBIGUOUS,
  SIP_REQUEST_PENDING,
  SIP_UNDECIPHERABLE,
  RETRY_ON_IMS_WITHOUT_RTT,

  /* OEM specific error codes. Used to distinguish error from
   * ERROR_UNSPECIFIED and help assist debugging */
  OEM_CAUSE_1 = 0xf001,
  OEM_CAUSE_2 = 0xf002,
  OEM_CAUSE_3 = 0xf003,
  OEM_CAUSE_4 = 0xf004,
  OEM_CAUSE_5 = 0xf005,
  OEM_CAUSE_6 = 0xf006,
  OEM_CAUSE_7 = 0xf007,
  OEM_CAUSE_8 = 0xf008,
  OEM_CAUSE_9 = 0xf009,
  OEM_CAUSE_10 = 0xf00a,
  OEM_CAUSE_11 = 0xf00b,
  OEM_CAUSE_12 = 0xf00c,
  OEM_CAUSE_13 = 0xf00d,
  OEM_CAUSE_14 = 0xf00e,
  OEM_CAUSE_15 = 0xf00f,
  ERROR_UNSPECIFIED = 0xffff /* This error will be deprecated soon,
                                vendor code should make sure to map error
                                code to specific error */
};
inline std::string toString(const CallFailCause &o) {
  // TODO: convert properly
  return std::string("CallFailCause:") + std::to_string(static_cast<uint32_t>(o));
}

enum class ClirAction {
  UNKNOWN,
  INVOCATION,  /* Invocation */
  SUPPRESSION, /* Suppression */
};
inline std::string toString(const ClirAction &o) {
  if (o == ClirAction::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == ClirAction::INVOCATION) {
    return "INVOCATION";
  }
  if (o == ClirAction::SUPPRESSION) {
    return "SUPPRESSION";
  }
  return "<invalid>";
}

enum class ClirPresentation {
  UNKNOWN,
  NOT_PROVISIONED,            /* Not provisioned */
  PROVISIONED_PERMANENT_MODE, /* Provisioned permanent */
  PRESENTATION_RESTRICTED,    /* Presenstation restricted */
  PRESENTATION_ALLOWED,       /* Presenation allowed */
};
inline std::string toString(const ClirPresentation &o) {
  if (o == ClirPresentation::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == ClirPresentation::NOT_PROVISIONED) {
    return "NOT_PROVISIONED";
  }
  if (o == ClirPresentation::PROVISIONED_PERMANENT_MODE) {
    return "PROVISIONED_PERMANENT_MODE";
  }
  if (o == ClirPresentation::PRESENTATION_RESTRICTED) {
    return "PRESENTATION_RESTRICTED";
  }
  if (o == ClirPresentation::PRESENTATION_ALLOWED) {
    return "PRESENTATION_ALLOWED";
  }
  return "<invalid>";
}

enum class ServiceClassProvisionStatus {
  UNKNOWN,
  NOT_PROVISIONED, /* Not privisioned */
  PROVISIONED,     /* Provisioned */
};
inline std::string toString(const ServiceClassProvisionStatus &o) {
  if (o == ServiceClassProvisionStatus::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == ServiceClassProvisionStatus::NOT_PROVISIONED) {
    return "NOT_PROVISIONED";
  }
  if (o == ServiceClassProvisionStatus::PROVISIONED) {
    return "PROVISIONED";
  }
  return "<invalid>";
}

enum class ClipStatus {
  UNKNOWN,
  NOT_PROVISIONED, /* Not privisioned */
  PROVISIONED,     /* Provisioned */
  STATUS_UNKNOWN,  /* Unknown */
};
inline std::string toString(const ClipStatus &o) {
  if (o == ClipStatus::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == ClipStatus::NOT_PROVISIONED) {
    return "NOT_PROVISIONED";
  }
  if (o == ClipStatus::PROVISIONED) {
    return "PROVISIONED";
  }
  if (o == ClipStatus::STATUS_UNKNOWN) {
    return "STATUS_UNKNOWN";
  }
  return "<invalid>";
}

enum class FacilityType {
  UNKNOWN,
  CLIP,    /* Calling Line Identification Presentation */
  COLP,    /* Connected Line Identification Presentation */
  BAOC,    /* Bar All Outgoing Calls */
  BAOIC,   /* Bar All Outgoing International Calls */
  BAOICxH, /* Bar all Outgoing International Calls except those directed to home PLMN country */
  BAIC,    /* Bar All Incoming Calls */
  BAICr,   /* Bar All Incoming Calls when Roaming outside the home PLMN country */
  BA_ALL,  /* Bar All incoming & outgoing Calls */
  BA_MO,   /* Bar All Outgoing Calls */
  BA_MT,   /* Bar All Incming Calls */
  BS_MT,   /* Bar Incoming Calls from specific DN */
  BAICa,   /* Bar All Incoming Calls from Anonymous numbers*/
};
inline std::string toString(const FacilityType &o) {
  if (o == FacilityType::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == FacilityType::CLIP) {
    return "CLIP";
  }
  if (o == FacilityType::COLP) {
    return "COLP";
  }
  if (o == FacilityType::BAOC) {
    return "BAOC";
  }
  if (o == FacilityType::BAOIC) {
    return "BAOIC";
  }
  if (o == FacilityType::BAOICxH) {
    return "BAOICxH";
  }
  if (o == FacilityType::BAIC) {
    return "BAIC";
  }
  if (o == FacilityType::BAICr) {
    return "BAICr";
  }
  if (o == FacilityType::BA_ALL) {
    return "BA_ALL";
  }
  if (o == FacilityType::BA_MO) {
    return "BA_MO";
  }
  if (o == FacilityType::BA_MT) {
    return "BA_MT";
  }
  if (o == FacilityType::BS_MT) {
    return "BS_MT";
  }
  if (o == FacilityType::BAICa) {
    return "BAICa";
  }
  return "<invalid>";
}

enum class CallModifyFailCause {
  UNKNOWN,
  SUCCESS,
  RADIO_NOT_AVAILABLE,
  GENERIC_FAILURE,
  REQUEST_NOT_SUPPORTED,
  CANCELLED, /* Cancelled */
  UNUSED,
  INVALID_PARAMETER,
  REJECTED_BY_REMOTE,
  IMS_DEREGISTERED,
  NETWORK_NOT_SUPPORTED,
  HOLD_RESUME_FAILED,
  HOLD_RESUME_CANCELED,
  REINVITE_COLLISION,
};
inline std::string toString(const CallModifyFailCause &o) {
  if (o == CallModifyFailCause::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == CallModifyFailCause::SUCCESS) {
    return "SUCCESS";
  }
  if (o == CallModifyFailCause::RADIO_NOT_AVAILABLE) {
    return "RADIO_NOT_AVAILABLE";
  }
  if (o == CallModifyFailCause::GENERIC_FAILURE) {
    return "GENERIC_FAILURE";
  }
  if (o == CallModifyFailCause::REQUEST_NOT_SUPPORTED) {
    return "REQUEST_NOT_SUPPORTED";
  }
  if (o == CallModifyFailCause::CANCELLED) {
    return "CANCELLED";
  }
  if (o == CallModifyFailCause::UNUSED) {
    return "UNUSED";
  }
  if (o == CallModifyFailCause::INVALID_PARAMETER) {
    return "INVALID_PARAMETER";
  }
  if (o == CallModifyFailCause::REJECTED_BY_REMOTE) {
    return "REJECTED_BY_REMOTE";
  }
  if (o == CallModifyFailCause::IMS_DEREGISTERED) {
    return "IMS_DEREGISTERED";
  }
  if (o == CallModifyFailCause::NETWORK_NOT_SUPPORTED) {
    return "NETWORK_NOT_SUPPORTED";
  }
  if (o == CallModifyFailCause::HOLD_RESUME_FAILED) {
    return "HOLD_RESUME_FAILED";
  }
  if (o == CallModifyFailCause::HOLD_RESUME_CANCELED) {
    return "HOLD_RESUME_CANCELED";
  }
  if (o == CallModifyFailCause::REINVITE_COLLISION) {
    return "REINVITE_COLLISION";
  }
  return "<invalid>";
}

enum class RingBackToneOperation {
  UNKNOWN,
  START, /*  Start Ringback tone */
  STOP,  /* Stop Ringback tone  */
};
inline std::string toString(const RingBackToneOperation &o) {
  if (o == RingBackToneOperation::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == RingBackToneOperation::START) {
    return "START";
  }
  if (o == RingBackToneOperation::STOP) {
    return "STOP";
  }
  return "<invalid>";
}

enum class EctType {
  UNKNOWN,
  BLIND_TRANSFER,        /*  Blind transfer */
  ASSURED_TRANSFER,      /* Assured transfer  */
  CONSULTATIVE_TRANSFER, /* Consultative   */
};
inline std::string toString(const EctType &o) {
  if (o == EctType::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == EctType::BLIND_TRANSFER) {
    return "BLIND_TRANSFER";
  }
  if (o == EctType::ASSURED_TRANSFER) {
    return "ASSURED_TRANSFER";
  }
  if (o == EctType::CONSULTATIVE_TRANSFER) {
    return "CONSULTATIVE_TRANSFER";
  }
  return "<invalid>";
}

enum class PrivacyMode {
  UNKNOWN,
  STANDARD, /* Standard privacy */
  ENHANCED, /* Enhanced privacy */
};
inline std::string toString(const PrivacyMode &o) {
  if (o == PrivacyMode::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == PrivacyMode::STANDARD) {
    return "STANDARD";
  }
  if (o == PrivacyMode::ENHANCED) {
    return "ENHANCED";
  }
  return "<invalid>";
}

enum class TtyMode {
  UNKNOWN,
  MODE_OFF, /* Off */
  FULL,     /* Full */
  HCO,      /* Hearing carry over */
  VCO,      /* Voice carry over */
};
inline std::string toString(const TtyMode &o) {
  if (o == TtyMode::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == TtyMode::MODE_OFF) {
    return "MODE_OFF";
  }
  if (o == TtyMode::FULL) {
    return "FULL";
  }
  if (o == TtyMode::HCO) {
    return "HCO";
  }
  if (o == TtyMode::VCO) {
    return "VCO";
  }
  return "<invalid>";
}

enum class VerificationStatus {
  UNKNOWN,
  NONE, /* Telephone number is not validated. */
  PASS, /* Telephone number validation passed. */
  FAIL, /* Telephone number validation failed. */
};
inline std::string toString(const VerificationStatus &o) {
  if (o == VerificationStatus::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == VerificationStatus::NONE) {
    return "NONE";
  }
  if (o == VerificationStatus::PASS) {
    return "PASS";
  }
  if (o == VerificationStatus::FAIL) {
    return "FAIL";
  }
  return "<invalid>";
}

enum class MultiIdentityRegistrationStatus {
  UNKNOWN,
  DISABLE, /* Disabled. */
  ENABLE,  /* Enabled. */
};
inline std::string toString(const MultiIdentityRegistrationStatus &o) {
  if (o == MultiIdentityRegistrationStatus::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == MultiIdentityRegistrationStatus::DISABLE) {
    return "DISABLE";
  }
  if (o == MultiIdentityRegistrationStatus::ENABLE) {
    return "ENABLE";
  }
  return "<invalid>";
}

enum class MultiIdentityLineType {
  UNKNOWN,
  PRIMARY,   /* Primary. */
  SECONDARY, /* Secondary. */
};
inline std::string toString(const MultiIdentityLineType &o) {
  if (o == MultiIdentityLineType::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == MultiIdentityLineType::PRIMARY) {
    return "PRIMARY";
  }
  if (o == MultiIdentityLineType::SECONDARY) {
    return "SECONDARY";
  }
  return "<invalid>";
}


enum class ConferenceCallState {
  UNKNOWN,
  FOREGROUND, /* Conference call is in forground */
  BACKGROUND, /* Conference call is in backgroudn */
  RINGING,    /* Conference call is in ringing state */
};
inline std::string toString(const ConferenceCallState &o) {
  if (o == ConferenceCallState::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == ConferenceCallState::FOREGROUND) {
    return "FOREGROUND";
  }
  if (o == ConferenceCallState::BACKGROUND) {
    return "BACKGROUND";
  }
  if (o == ConferenceCallState::RINGING) {
    return "RINGING";
  }
  return "<invalid>";
}

enum class ConfParticipantOperation {
  UNKNOWN,
  ADD,    /* Add */
  REMOVE, /* Remove */
};
inline std::string toString(const ConfParticipantOperation &o) {
  if (o == ConfParticipantOperation::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == ConfParticipantOperation::ADD) {
    return "ADD";
  }
  if (o == ConfParticipantOperation::REMOVE) {
    return "REMOVE";
  }
  return "<invalid>";
}

enum class NotificationType {
  UNKNOWN,
  MO, /* MO intermediate result code */
  MT, /* MT unsolicited result code */
};
inline std::string toString(const NotificationType &o) {
  if (o == NotificationType::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == NotificationType::MO) {
    return "MO";
  }
  if (o == NotificationType::MT) {
    return "MT";
  }
  return "<invalid>";
}

enum class UssdModeType {
  UNKNOWN,
  NOTIFY,         // USSD-Notify
  REQUEST,        // USSD-Request
  NW_RELEASE,     // Session terminated by network
  LOCAL_CLIENT,   // other local client (eg, SIM Toolkit) has responded
  NOT_SUPPORTED,  // Operation not supported
  NW_TIMEOUT,     // Network timeout
};
inline std::string toString(const UssdModeType &o) {
  if (o == UssdModeType::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == UssdModeType::NOTIFY) {
    return "NOTIFY";
  }
  if (o == UssdModeType::REQUEST) {
    return "REQUEST";
  }
  if (o == UssdModeType::NW_RELEASE) {
    return "NW_RELEASE";
  }
  if (o == UssdModeType::LOCAL_CLIENT) {
    return "LOCAL_CLIENT";
  }
  if (o == UssdModeType::NOT_SUPPORTED) {
    return "NOT_SUPPORTED";
  }
  if (o == UssdModeType::NW_TIMEOUT) {
    return "NW_TIMEOUT";
  }
  return "<invalid>";
}

enum class DtmfEvent {
  UNKNOWN,
  FWD_BURST,      /*  Received a CDMA-burst DTMF message */
  FWD_START_CONT, /*  Received a start-continuous DTMF tone order */
  FWD_STOP_CONT,  /*  Received a stop-continuous DTMF tone order */
};
inline std::string toString(const DtmfEvent &o) {
  if (o == DtmfEvent::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == DtmfEvent::FWD_BURST) {
    return "FWD_BURST";
  }
  if (o == DtmfEvent::FWD_START_CONT) {
    return "FWD_START_CONT";
  }
  if (o == DtmfEvent::FWD_STOP_CONT) {
    return "FWD_STOP_CONT";
  }
  return "<invalid>";
}

enum class DtmfOnLength {
  UNKNOWN,
  ONLENGTH_95MS,
  ONLENGTH_150MS,
  ONLENGTH_200MS,
  ONLENGTH_250MS,
  ONLENGTH_300MS,
  ONLENGTH_350MS,
  ONLENGTH_SMS,
};
inline std::string toString(const DtmfOnLength &o) {
  if (o == DtmfOnLength::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == DtmfOnLength::ONLENGTH_95MS) {
    return "ONLENGTH_95MS";
  }
  if (o == DtmfOnLength::ONLENGTH_150MS) {
    return "ONLENGTH_150MS";
  }
  if (o == DtmfOnLength::ONLENGTH_200MS) {
    return "ONLENGTH_200MS";
  }
  if (o == DtmfOnLength::ONLENGTH_250MS) {
    return "ONLENGTH_250MS";
  }
  if (o == DtmfOnLength::ONLENGTH_300MS) {
    return "ONLENGTH_300MS";
  }
  if (o == DtmfOnLength::ONLENGTH_350MS) {
    return "ONLENGTH_350MS";
  }
  if (o == DtmfOnLength::ONLENGTH_SMS) {
    return "ONLENGTH_SMS";
  }
  return "<invalid>";
}

enum class DtmfOffLength {
  UNKNOWN,
  OFFLENGTH_60MS,
  OFFLENGTH_100MS,
  OFFLENGTH_150MS,
  OFFLENGTH_200MS,
};
inline std::string toString(const DtmfOffLength &o) {
  if (o == DtmfOffLength::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == DtmfOffLength::OFFLENGTH_60MS) {
    return "OFFLENGTH_60MS";
  }
  if (o == DtmfOffLength::OFFLENGTH_100MS) {
    return "OFFLENGTH_100MS";
  }
  if (o == DtmfOffLength::OFFLENGTH_150MS) {
    return "OFFLENGTH_150MS";
  }
  if (o == DtmfOffLength::OFFLENGTH_200MS) {
    return "OFFLENGTH_200MS";
  }
  return "<invalid>";
}

enum class TirMode {
  UNKNOWN,
  TEMPORARY,
  PERMANENT,
};

enum class CallModifiedCause {
  NONE = 0, /* No call modify cause information \n  */
  UPGRADE_DUE_TO_LOCAL_REQ = 1, /* Upgrade due to a local request \n  */
  UPGRADE_DUE_TO_REMOTE_REQ = 2, /* Upgrade due to a remote request \n  */
  DOWNGRADE_DUE_TO_LOCAL_REQ = 3, /* Downgrade due to a local request \n  */
  DOWNGRADE_DUE_TO_REMOTE_REQ = 4, /* Downgrade due to a remote request \n  */
  DOWNGRADE_DUE_TO_RTP_TIMEOUT = 5, /* Downgrade due to an RTP timeout \n  */
  DOWNGRADE_DUE_TO_QOS = 6, /* Downgrade due to QOS \n  */
  DOWNGRADE_DUE_TO_PACKET_LOSS = 7, /* Downgrade due to a packet loss \n  */
  DOWNGRADE_DUE_TO_LOW_THRPUT = 8, /* Downgrade due to low throughput \n  */
  DOWNGRADE_DUE_TO_THERM_MITIGATION = 9, /* Downgrade due to thermal mitigation \n  */
  DOWNGRADE_DUE_TO_LIPSYNC = 10, /* Downgrade due to lipsync  */
  DOWNGRADE_DUE_TO_GENERIC_ERROR = 11, /* Downgrade due to a general error \n  */
};
inline std::string toString(const CallModifiedCause &o) {
  if (o == CallModifiedCause::NONE) {
    return "NONE";
  }
  if (o == CallModifiedCause::UPGRADE_DUE_TO_LOCAL_REQ) {
    return "UPGRADE_DUE_TO_LOCAL_REQ";
  }
  if (o == CallModifiedCause::UPGRADE_DUE_TO_REMOTE_REQ) {
    return "UPGRADE_DUE_TO_REMOTE_REQ";
  }
  if (o == CallModifiedCause::DOWNGRADE_DUE_TO_LOCAL_REQ) {
    return "DOWNGRADE_DUE_TO_LOCAL_REQ";
  }
  if (o == CallModifiedCause::DOWNGRADE_DUE_TO_REMOTE_REQ) {
    return "DOWNGRADE_DUE_TO_REMOTE_REQ";
  }
  if (o == CallModifiedCause::DOWNGRADE_DUE_TO_RTP_TIMEOUT) {
    return "DOWNGRADE_DUE_TO_RTP_TIMEOUT";
  }
  if (o == CallModifiedCause::DOWNGRADE_DUE_TO_QOS) {
    return "DOWNGRADE_DUE_TO_QOS";
  }
  if (o == CallModifiedCause::DOWNGRADE_DUE_TO_PACKET_LOSS) {
    return "DOWNGRADE_DUE_TO_PACKET_LOSS";
  }
  if (o == CallModifiedCause::DOWNGRADE_DUE_TO_LOW_THRPUT) {
    return "DOWNGRADE_DUE_TO_LOW_THRPUT";
  }
  if (o == CallModifiedCause::DOWNGRADE_DUE_TO_THERM_MITIGATION) {
    return "DOWNGRADE_DUE_TO_THERM_MITIGATION";
  }
  if (o == CallModifiedCause::DOWNGRADE_DUE_TO_LIPSYNC) {
    return "DOWNGRADE_DUE_TO_LIPSYNC";
  }
  if (o == CallModifiedCause::DOWNGRADE_DUE_TO_GENERIC_ERROR) {
    return "DOWNGRADE_DUE_TO_GENERIC_ERROR";
  }
  return "<invalid>";
}


class CallFailCauseResponse {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(CallFailCause, FailCause);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, ExtendedFailCause);
  // QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_VECTOR(uint8_t, ErrorInfo);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, NetworkErrorString);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_SHARED_PTR(SipErrorInfo, ErrorDetails);

 public:
  CallFailCauseResponse() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(FailCause);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(ExtendedFailCause);
    // QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR_VECTOR(ErrorInfo);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(NetworkErrorString);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR_SHARED_PTR(ErrorDetails);
  }
  ~CallFailCauseResponse() {}
  CallFailCauseResponse(const CallFailCauseResponse &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, FailCause);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, ExtendedFailCause);
    // QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR_VECTOR(from, ErrorInfo);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, NetworkErrorString);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR_SHARED_PTR(from, ErrorDetails);
  }
};

class VerstatInfo {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(bool, CanMarkUnwantedCall);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(VerificationStatus, VerificationStatus);

 public:
  VerstatInfo() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(CanMarkUnwantedCall);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(VerificationStatus);
  }
  ~VerstatInfo() {}
  VerstatInfo(const VerstatInfo &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, CanMarkUnwantedCall);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, VerificationStatus);
  }
};

class UusInfo {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(RIL_UUS_Type, Type);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(RIL_UUS_DCS, Dcs);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, Data);

 public:
  UusInfo() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Type);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Dcs);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Data);
  }
  ~UusInfo() {}
  UusInfo(const UusInfo &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Type);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Dcs);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Data);
  }
};

class CallInfo {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(CallState, CallState);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, Index);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, Toa);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(bool, IsMpty);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(bool, IsMt);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(bool, Als);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(bool, IsVoice);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(bool, IsVoicePrivacy);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, Number);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, NumberPresentation);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, Name);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, NamePresentation);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(CallType, CallType);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(CallDomain, CallDomain);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, CallSubState);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(bool, IsEncrypted);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(bool, IsCalledPartyRinging);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(bool, IsVideoConfSupported);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, HistoryInfo);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, MediaId);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(CallModifiedCause, CauseCode);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(RttMode, RttMode);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, SipAlternateUri);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_VECTOR(ServiceStatusInfo, LocalAbility);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_VECTOR(ServiceStatusInfo, PeerAbility);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_SHARED_PTR(CallFailCauseResponse, CallFailCauseResponse);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, TerminatingNumber);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(bool, IsSecondary);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_SHARED_PTR(VerstatInfo, VerstatInfo);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_SHARED_PTR(UusInfo, UusInfo);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(RIL_AudioQuality, AudioQuality);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, Codec);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, DisplayText);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, AdditionalCallInfo);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, ChildNumber);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, EmergencyServiceCategory);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(TirMode, TirMode);

 public:
  CallInfo() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(CallState);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Index);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Toa);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(IsMpty);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(IsMt);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Als);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(IsVoice);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(IsVoicePrivacy);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Number);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(NumberPresentation);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Name);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(NamePresentation);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(CallType);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(CallDomain);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(CallSubState);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(IsEncrypted);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(IsCalledPartyRinging);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(IsVideoConfSupported);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(HistoryInfo);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(MediaId);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(CauseCode);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(RttMode);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(SipAlternateUri);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR_VECTOR(LocalAbility);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR_VECTOR(PeerAbility);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR_SHARED_PTR(CallFailCauseResponse);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(TerminatingNumber);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(IsSecondary);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(VerstatInfo);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(UusInfo);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(AudioQuality);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Codec);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(DisplayText);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(AdditionalCallInfo);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(ChildNumber);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(EmergencyServiceCategory);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(TirMode);
  }
  ~CallInfo() {}
  CallInfo(const CallInfo &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, CallState);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Index);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Toa);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, IsMpty);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, IsMt);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Als);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, IsVoice);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, IsVoicePrivacy);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Number);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, NumberPresentation);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Name);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, NamePresentation);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, CallType);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, CallDomain);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, CallSubState);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, IsEncrypted);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, IsCalledPartyRinging);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, IsVideoConfSupported);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, HistoryInfo);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, MediaId);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, CauseCode);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, RttMode);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, SipAlternateUri);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR_VECTOR(from, LocalAbility);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR_VECTOR(from, PeerAbility);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR_SHARED_PTR(from, CallFailCauseResponse);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, TerminatingNumber);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, IsSecondary);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR_SHARED_PTR(from, VerstatInfo);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR_SHARED_PTR(from, UusInfo);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, AudioQuality);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Codec);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, DisplayText);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, AdditionalCallInfo);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, ChildNumber);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, EmergencyServiceCategory);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, TirMode);
  }
};

class TtyModeResp : public qcril::interfaces::BasePayload {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(TtyMode, TtyMode);

 public:
  TtyModeResp() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(TtyMode);
  }
  ~TtyModeResp() {}
  TtyModeResp(const TtyModeResp &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, TtyMode);
  }
};

class PrivacyModeResp : public qcril::interfaces::BasePayload {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(PrivacyMode, PrivacyMode);

 public:
  PrivacyModeResp() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(PrivacyMode);
  }
  ~PrivacyModeResp() {}
  PrivacyModeResp(const PrivacyModeResp &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, PrivacyMode);
  }
};

class LastCallFailCauseInfo : public qcril::interfaces::BasePayload {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(CallFailCause, CallFailCause);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, CallFailCauseDescription);

 public:
  LastCallFailCauseInfo() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(CallFailCause);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(CallFailCauseDescription);
  }
  ~LastCallFailCauseInfo() {}
  LastCallFailCauseInfo(const LastCallFailCauseInfo &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, CallFailCause);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, CallFailCauseDescription);
  }
};

class ColrInfo : public qcril::interfaces::BasePayload {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(ServiceClassStatus, Status);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(ServiceClassProvisionStatus, ProvisionStatus);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(Presentation, Presentation);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_SHARED_PTR(SipErrorInfo, ErrorDetails);

 public:
  ColrInfo() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Status);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(ProvisionStatus);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Presentation);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR_SHARED_PTR(ErrorDetails);
  }
  ~ColrInfo() {}
  ColrInfo(const ColrInfo &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Status);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, ProvisionStatus);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Presentation);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR_SHARED_PTR(from, ErrorDetails);
  }
};

class ClirInfo : public qcril::interfaces::BasePayload {
  // QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(ClirAction, action);              // Param N
  // QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(ClirPresentation, presentation);  // Param M
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, Action);        // Param N
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, Presentation);  // Param M

 public:
  ClirInfo() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Action);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Presentation);
  }
  ~ClirInfo() {}
  ClirInfo(const ClirInfo &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Action);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Presentation);
  }
};

class ClipInfo : public qcril::interfaces::BasePayload {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(ClipStatus, ClipStatus);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_SHARED_PTR(SipErrorInfo, ErrorDetails);

 public:
  ClipInfo() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(ClipStatus);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR_SHARED_PTR(ErrorDetails);
  }
  ~ClipInfo() {}
  ClipInfo(const ClipInfo &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, ClipStatus);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR_SHARED_PTR(from, ErrorDetails);
  }
};

class CallFwdTimerInfo {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, Year);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, Month);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, Day);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, Hour);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, Minute);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, Second);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, Timezone);

 public:
  CallFwdTimerInfo() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Year);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Month);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Day);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Hour);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Minute);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Second);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Timezone);
  }

  ~CallFwdTimerInfo() {}

  CallFwdTimerInfo(const CallFwdTimerInfo &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Year);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Month);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Day);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Hour);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Minute);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Second);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Timezone);
  }
};

inline std::string toString(const CallFwdTimerInfo &o) {
  std::string os;
  os += "{";
  os += " .mYear = ";
  os += o.hasYear() ? std::to_string(o.getYear()) : "<invalid>";
  os += " .mMonth = ";
  os += o.hasMonth() ? std::to_string(o.getMonth()) : "<invalid>";
  os += " .mDay = ";
  os += o.hasDay() ? std::to_string(o.getDay()) : "<invalid>";
  os += " .mHour = ";
  os += o.hasHour() ? std::to_string(o.getHour()) : "<invalid>";
  os += " .mMinute = ";
  os += o.hasMinute() ? std::to_string(o.getMinute()) : "<invalid>";
  os += " .mSecond = ";
  os += o.hasSecond() ? std::to_string(o.getSecond()) : "<invalid>";
  os += " .mTimezone = ";
  os += o.hasTimezone() ? std::to_string(o.getTimezone()) : "<invalid>";
  os += "}";
  return os;
}

class CallForwardInfo {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(int, Status);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(int, Reason);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(int, ServiceClass);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(int, Toa);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, Number);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(int, TimeSeconds);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_SHARED_PTR(CallFwdTimerInfo, CallFwdTimerStart);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_SHARED_PTR(CallFwdTimerInfo, CallFwdTimerEnd);

 public:
  CallForwardInfo() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Status);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Reason);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(ServiceClass);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Toa);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Number);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(TimeSeconds);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR_SHARED_PTR(CallFwdTimerStart);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR_SHARED_PTR(CallFwdTimerEnd);
  }

  CallForwardInfo(const CallForwardInfo &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Status);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Reason);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, ServiceClass);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Toa);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Number);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, TimeSeconds);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR_SHARED_PTR(from, CallFwdTimerStart);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR_SHARED_PTR(from, CallFwdTimerEnd);
  }
};

class CallBarringNumbersInfo {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(ServiceClassStatus, Status);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, Number);

 public:
  CallBarringNumbersInfo() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Status);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Number);
  }
  CallBarringNumbersInfo(const CallBarringNumbersInfo &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Status);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Number);
  }
};

class CallBarringNumbersListInfo {
  /* Only voice class i.e 1 is supported */
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, ServiceClass);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_VECTOR(CallBarringNumbersInfo, CallBarringNumbersInfo); /* every element in CbNumInfo corresponds to each serviceClass */

 public:
  CallBarringNumbersListInfo() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(ServiceClass);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR_VECTOR(CallBarringNumbersInfo);
  }

  CallBarringNumbersListInfo(const CallBarringNumbersListInfo &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, ServiceClass);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR_VECTOR(from, CallBarringNumbersInfo);
  }
};

class SuppServiceStatusInfo : public qcril::interfaces::BasePayload {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(ServiceClassStatus, Status);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, ServiceClass);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(ServiceClassProvisionStatus, ProvisionStatus);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(FacilityType, FacilityType);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_VECTOR(CallBarringNumbersListInfo, CallBarringNumbersListInfo);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_SHARED_PTR(SipErrorInfo, ErrorDetails);

 public:
  SuppServiceStatusInfo() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Status);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(ServiceClass);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(ProvisionStatus);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(FacilityType);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR_VECTOR(CallBarringNumbersListInfo);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR_SHARED_PTR(ErrorDetails);
  }

  SuppServiceStatusInfo(const SuppServiceStatusInfo &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Status);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, ServiceClass);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, ProvisionStatus);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, FacilityType);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR_VECTOR(from, CallBarringNumbersListInfo);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR_SHARED_PTR(from, ErrorDetails);
  }
};

class CallWaitingInfo : public qcril::interfaces::BasePayload {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(ServiceClassStatus, Status);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, ServiceClass);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_SHARED_PTR(SipErrorInfo, ErrorDetails);

 public:
  CallWaitingInfo() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Status);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(ServiceClass);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR_SHARED_PTR(ErrorDetails);
  }

  CallWaitingInfo(const CallWaitingInfo &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Status);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, ServiceClass);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR_SHARED_PTR(from, ErrorDetails);
  }
};

class GetCallForwardRespData : public qcril::interfaces::BasePayload {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_VECTOR(CallForwardInfo, CallForwardInfo);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_SHARED_PTR(SipErrorInfo, ErrorDetails);

 public:
  GetCallForwardRespData() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR_VECTOR(CallForwardInfo);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR_SHARED_PTR(ErrorDetails);
  }

  GetCallForwardRespData(const GetCallForwardRespData &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR_VECTOR(from, CallForwardInfo);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR_SHARED_PTR(from, ErrorDetails);
  }
};

class SetCallForwardStatus {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, Reason);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(bool, Status);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_SHARED_PTR(SipErrorInfo, ErrorDetails);

 public:
  SetCallForwardStatus() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Reason);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Status);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR_SHARED_PTR(ErrorDetails);
  }

  SetCallForwardStatus(const SetCallForwardStatus &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Reason);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Status);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR_SHARED_PTR(from, ErrorDetails);
  }
};

class SetCallForwardRespData : public qcril::interfaces::BasePayload {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_VECTOR(SetCallForwardStatus, SetCallForwardStatus);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_SHARED_PTR(SipErrorInfo, ErrorDetails);

 public:
  SetCallForwardRespData() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR_VECTOR(SetCallForwardStatus);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR_SHARED_PTR(ErrorDetails);
  }

  SetCallForwardRespData(const SetCallForwardRespData &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR_VECTOR(from, SetCallForwardStatus);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR_SHARED_PTR(from, ErrorDetails);
  }
};

class GetCurrentCallsRespData : public qcril::interfaces::BasePayload {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_VECTOR(CallInfo, CallInfoList);

 public:
  GetCurrentCallsRespData() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR_VECTOR(CallInfoList);
  }

  GetCurrentCallsRespData(const GetCurrentCallsRespData &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR_VECTOR(from, CallInfoList);
  }
};

class GetCurrentSetupCallsRespData : public qcril::interfaces::BasePayload {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, Index);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, Toa);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(bool, Als);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(bool, IsVoice);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, Number);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, NumberPresentation);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, Name);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, NamePresentation);

 public:
  GetCurrentSetupCallsRespData() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Index);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Toa);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Als);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(IsVoice);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Number);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(NumberPresentation);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Name);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(NamePresentation);
  }

  GetCurrentSetupCallsRespData(const GetCurrentSetupCallsRespData &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Index);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Toa);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Als);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, IsVoice);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Number);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, NumberPresentation);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Name);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, NamePresentation);
  }
};

class MuteInfo : public qcril::interfaces::BasePayload {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(bool, Enable);

 public:
  MuteInfo() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Enable);
  }
  ~MuteInfo() {}
  MuteInfo(const MuteInfo &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Enable);
  }
};

}  // namespace interfaces
}  // namespace qcril
