/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once

#include "interfaces/common.h"
#include <string>
#include <vector>

namespace qcril {
namespace interfaces {

enum class RegState {
  UNKNOWN,
  NOT_REGISTERED, /* Not registered */
  REGISTERING,    /* Registering */
  REGISTERED      /* Registered */
};
inline std::string toString(const RegState &o) {
  if (o == RegState::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == RegState::NOT_REGISTERED) {
    return "NOT_REGISTERED";
  }
  if (o == RegState::REGISTERING) {
    return "REGISTERING";
  }
  if (o == RegState::REGISTERED) {
    return "REGISTERED";
  }
  return "<invalid>";
}
enum class VoiceInfo {
  SILENT,
  SPEECH,
  UNKNOWN
};

enum class RadioState {
  STATE_OFF,
  STATE_UNAVAILABLE,
  STATE_ON,
  STATE_UNKNOWN
};

// enum class ServiceType { INVALID, SMS, VOIP, VT };

enum class CallType {
  UNKNOWN,
  VOICE,    /* Voice */
  VT_TX,    /* Transmit only Video */
  VT_RX,    /* Receive only Vide */
  VT,       /* Video */
  VT_NODIR, /* VT no direction */
  SMS,      /* SMS */
  UT        /* UT  - Supplementary services */
};
inline std::string toString(const CallType &o) {
  if (o == CallType::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == CallType::VOICE) {
    return "VOICE";
  }
  if (o == CallType::VT_TX) {
    return "VT_TX";
  }
  if (o == CallType::VT_RX) {
    return "VT_RX";
  }
  if (o == CallType::VT) {
    return "VT";
  }
  if (o == CallType::VT_NODIR) {
    return "VT_NODIR";
  }
  if (o == CallType::SMS) {
    return "SMS";
  }
  if (o == CallType::UT) {
    return "UT";
  }
  return "<invalid>";
}

enum class StatusType {
  UNKNOWN,
  DISABLED,          /* Disabled */
  PARTIALLY_ENABLED, /* Partially enabled */
  ENABLED,           /* Enabled */
  NOT_SUPPORTED      /* Not supported */
};
inline std::string toString(const StatusType &o) {
  if (o == StatusType::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == StatusType::DISABLED) {
    return "DISABLED";
  }
  if (o == StatusType::PARTIALLY_ENABLED) {
    return "PARTIALLY_ENABLED";
  }
  if (o == StatusType::ENABLED) {
    return "ENABLED";
  }
  if (o == StatusType::NOT_SUPPORTED) {
    return "NOT_SUPPORTED";
  }
  return "<invalid>";
}

enum class RttMode {
  UNKNOWN,
  DISABLED, /* non-RTT call */
  FULL      /* RTT call */
};
inline std::string toString(const RttMode &o) {
  if (o == RttMode::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == RttMode::DISABLED) {
    return "DISABLED";
  }
  if (o == RttMode::FULL) {
    return "FULL";
  }
  return "<invalid>";
}

enum class ServiceClassStatus {
  UNKNOWN,
  DISABLED, /* Disabled */
  ENABLED,  /* Enabled */
};

inline std::string toString(const ServiceClassStatus &o) {
  if (o == ServiceClassStatus::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == ServiceClassStatus::DISABLED) {
    return "DISABLED";
  }
  if (o == ServiceClassStatus::ENABLED) {
    return "ENABLED";
  }
  return "<invalid>";
}

enum class HandoverType {
  UNKNOWN,
  START,                         /* Handover triggered */
  COMPLETE_SUCCESS,              /* Handover completed successfully */
  COMPLETE_FAIL,                 /* Handover failed */
  CANCEL,                        /* Handover cancelled */
  NOT_TRIGGERED,                 /* Handover not triggered */
  NOT_TRIGGERED_MOBILE_DATA_OFF, /* Handover not triggered due to mobile data off */
};
inline std::string toString(const HandoverType &o) {
  if (o == HandoverType::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == HandoverType::START) {
    return "START";
  }
  if (o == HandoverType::COMPLETE_SUCCESS) {
    return "COMPLETE_SUCCESS";
  }
  if (o == HandoverType::COMPLETE_FAIL) {
    return "COMPLETE_FAIL";
  }
  if (o == HandoverType::CANCEL) {
    return "CANCEL";
  }
  if (o == HandoverType::NOT_TRIGGERED) {
    return "NOT_TRIGGERED";
  }
  if (o == HandoverType::NOT_TRIGGERED_MOBILE_DATA_OFF) {
    return "NOT_TRIGGERED_MOBILE_DATA_OFF";
  }
  return "<invalid>";
}

enum class VowifiQuality {
  UNKNOWN,
  NONE,      /* None */
  EXCELLENT, /* Excellent */
  FAIR,      /* Fair */
  BAD        /* Bad */
};

inline std::string toString(const VowifiQuality &o) {
  if (o == VowifiQuality::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == VowifiQuality::NONE) {
    return "NONE";
  }
  if (o == VowifiQuality::EXCELLENT) {
    return "EXCELLENT";
  }
  if (o == VowifiQuality::FAIR) {
    return "FAIR";
  }
  if (o == VowifiQuality::BAD) {
    return "BAD";
  }
  return "<invalid>";
}

enum class BlockReasonType {
  UNKNOWN,
  PDP_FAILURE,          /* Pdp failure */
  REGISTRATION_FAILURE, /* Registration failure */
  HANDOVER_FAILURE,     /* Handover failure */
  OTHER_FAILURE         /* Other failures */
};
inline std::string toString(const BlockReasonType &o) {
  if (o == BlockReasonType::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == BlockReasonType::PDP_FAILURE) {
    return "PDP_FAILURE";
  }
  if (o == BlockReasonType::REGISTRATION_FAILURE) {
    return "REGISTRATION_FAILURE";
  }
  if (o == BlockReasonType::HANDOVER_FAILURE) {
    return "HANDOVER_FAILURE";
  }
  if (o == BlockReasonType::OTHER_FAILURE) {
    return "OTHER_FAILURE";
  }
  return "<invalid>";
}

enum class RegFailureReasonType {
  UNKNOWN,
  UNSPECIFIED,          /* Unspecified */
  MOBILE_IP,            /*Mobile IP */
  INTERNAL,             /* Internal */
  CALL_MANAGER_DEFINED, /* Call manager defined  */
  SPEC_DEFINED,         /* specification defined */
  PPP,                  /* PPP */
  EHRPD,                /* EHRPD */
  IPV6,                 /* IPv6 */
  IWLAN,                /* IWLAN */
  HANDOFF               /* Handoff */
};
inline std::string toString(const RegFailureReasonType &o) {
  if (o == RegFailureReasonType::UNKNOWN) {
    return "UNKNOWN";
  }
  if (o == RegFailureReasonType::UNSPECIFIED) {
    return "UNSPECIFIED";
  }
  if (o == RegFailureReasonType::MOBILE_IP) {
    return "MOBILE_IP";
  }
  if (o == RegFailureReasonType::INTERNAL) {
    return "INTERNAL";
  }
  if (o == RegFailureReasonType::CALL_MANAGER_DEFINED) {
    return "CALL_MANAGER_DEFINED";
  }
  if (o == RegFailureReasonType::SPEC_DEFINED) {
    return "SPEC_DEFINED";
  }
  if (o == RegFailureReasonType::PPP) {
    return "PPP";
  }
  if (o == RegFailureReasonType::EHRPD) {
    return "EHRPD";
  }
  if (o == RegFailureReasonType::IPV6) {
    return "IPV6";
  }
  if (o == RegFailureReasonType::IWLAN) {
    return "IWLAN";
  }
  if (o == RegFailureReasonType::HANDOFF) {
    return "HANDOFF";
  }
  return "<invalid>";
}

enum class ConfigItem {
    NONE,

    /* Value for this item is: (String)
     * String consisting of 0-7 seperated
     * by comma "," e.g., "1,5,7"
     * Reference for 0-7 values:
     * Values:
     *   - 0  -- 4.75 kbps
     *   - 1  -- 5.15 kbps
     *   - 2  -- 5.9 kbps
     *   - 3  -- 6.17 kbps
     *   - 4 -- 7.4 kbps
     *   - 5 -- 7.95 kbps
     *   - 6 -- 10.2 kbps
     *   - 7 -- 12.2 kbps
     * Spec reference: RFC 4867
     */

    VOCODER_AMRMODESET,
    /* Value for this Item is: (String)
     * String consisting of 0-8 seperated
     * by comma "," e.g., "2,6,8"
     * Reference for 0-8 values:
     * Values:
     *   - 0  -- 6.60 kbps
     *   - 1  -- 8.85 kbps
     *   - 2  -- 12.65 kbps
     *   - 3  -- 14.25 kbps
     *   - 4 -- 15.85 kbps
     *   - 5 -- 18.25 kbps
     *   - 6 -- 19.85 kbps
     *   - 7 -- 23.05 kbps
     *   - 8 -- 23.85 kbps
     * Spec reference: RFC 4867
     */
    VOCODER_AMRWBMODESET,

    /* Value for this item is: (Int)
     * SIP Session duration, in seconds
     */
    SIP_SESSION_TIMER,

    /* Value for this item is: (Int)
     * Minimum allowed value for session timer, in seconds
     */
    MIN_SESSION_EXPIRY,

    /* Value for this item is: (Int)
     * SIP timer operator mode A, in seconds
     * Valid Range: 0 - 30
     * By default 6 seconds is used.
     */
    CANCELLATION_TIMER,

    /* Value for this item is: (Int)
     * Ims Registration wait time when iRAT transitions from eHRPD to LTE,
     * in seconds.
     */
    T_DELAY,

    /* Value for this item is: (Int)
     * Flag that allows a device to silently redial over 1xRTT.
     * If this is not included in the request, a value of TRUE
     * (i.e., enabled) is used.
     * Value:
     *   - 0 -- Disable
     *   - 1 -- Enable
     */
    SILENT_REDIAL_ENABLE,

    /* Value for this item is: (Int)
     * RTT estimate, in milliseconds.
     */
    SIP_T1_TIMER,

    /* Value for this item is: (Int)
     * Maximum retransmit interval, in milliseconds,
     * for non-invite requests
     * and invite responses.
     */
    SIP_T2_TIMER,

    /* Value for this item is: (Int)
     * Non-invite transaction timeout timer, in milliseconds.
     */
    SIP_TF_TIMER,

    /* Value for this item is: (Int)
     * Enable VoLTE Support through Client Provisioning
     * Values:
     *   - 0 -- Disable
     *   - 1 -- Enable
     */
    VLT_SETTING_ENABLED,

    /* Value for this item is: (Int)
     * Enable VT Support through Client Provisioning
     * Values:
     *   - 0 -- Disable
     *   - 1 -- Enable
     */
    LVC_SETTING_ENABLED,

    /* Value for this item is: (String)
     * Ims Domain Name
     */
    DOMAIN_NAME,

    /* Value for this item is: (Int)
     * Sms Format.
     * Values:
     *   - 0 -- 3GPP2
     *   - 1 -- 3GPP
     */
    SMS_FORMAT,

    /* Value for this item is: (Int)
     * SMS over IP network indication flag
     * Values:
     *    - 0 -- Turn off MO SMS
     *    - 1 -- Turn on MO SMS
     */
    SMS_OVER_IP,

    /* Value for this item is: (Int)
     * Publish timer, in seconds, when publish is sent on
     * an IMS network using 4G Radio access technology
     */
    PUBLISH_TIMER,

    /* Value for this item is: (Int)
     * Publish extended timer, in seconds,
     * when publish is sent on an ims
     * network in non-4G RAT or
     * when in Airplane powerdown mode in a 4G RAT
     */
    PUBLISH_TIMER_EXTENDED,

    /* Value for this item is: (Int)
     * Duration of time, in seconds,
     * for which the retrieved capability
     * is considered valid
     */
    CAPABILITIES_CACHE_EXPIRATION,

    /* Value for this item is: (Int)
     * Duration of time, in seconds,
     * for which the retrieved availability
     * is considered valid
     */
    AVAILABILITY_CACHE_EXPIRATION,

    /* Value for this item is: (Int)
     * Duration of time, in seconds,
     * between successive capability polling
     */
    CAPABILITIES_POLL_INTERVAL,

    /* Value for this item is: (Int)
     * Duration, in seconds,
     * between successive publish requests
     */
    SOURCE_THROTTLE_PUBLISH,

    /* Value for this item is: (Int)
     * Maximum number of entries that
     * can be kept in the list subscription
     */
    MAX_NUM_ENTRIES_IN_RCL,

    /* Value for this item is: (Int)
     * Expiry timer value, in seconds,
     * for the list subscription req
     */
    CAPAB_POLL_LIST_SUB_EXP,

    /* Value for this item is: (Int)
     * Gzip compression enable flag
     * Values:
     *   - 0 -- Disable
     *   - 1 -- Enable
     */
    GZIP_FLAG,

    /* Value for this item is: (Int)
     * Enable presence support through client provisioning
     * Values:
     *   - 0 -- Disable
     *   - 1 -- Enable
     */
    EAB_SETTING_ENABLED,

    /* Value for this item is: (Int)
     * Mobile Data Status
     * Values:
     *   - 0 -- Disable
     *   - 1 -- Enable
     */
    MOBILE_DATA_ENABLED,

    /* Value for this item is: (int)
     * Wi-Fi Calling Status
     * Values:
     *   - 0 -- NOT_SUPPORTED
     *   - 1 -- WIFI_CALLING_ON
     *   - 2 -- WIFI_CALLING_OFF
     */
    VOICE_OVER_WIFI_ENABLED,

    /* Value for this item is: (Int)
     * Wi-Fi Call Roaming status.
     * Values:
     *   - 1 -- ROAMING_ENABLED
     *   - 0 -- ROAMING_DISABLED
     *
     * Note: Generic error will be returned
     * is the value is ROAMING_NOT_SUPPORTED
     */
    VOICE_OVER_WIFI_ROAMING,

    /* Value for this item is: (Int)
     * Wi-Fi Call Mode Preference
     * Values:
     *   - 0 -- WIFI_ONLY/WLAN_ONLY
     *   - 1 -- CELLULAR_PREFERRED
     *   - 2 -- WIFI_PREFERRED/WLAN_PREFERRED
     *   - 10 -- IMS_PREFERRED
     *
     * Note: Generic error will be returned
     * if the value is not any of the above.
     */
    VOICE_OVER_WIFI_MODE,

    /* Value for this item is: (Int)
     * Flag indicating whether Discovery
     * Capability is enabled
     * Values:
     *   - 0 -- Disable
     *          Presence doesnot publish/subscribe
     *          and ignores any notification received
     *
     *   - 1 -- Enable
     *          Presence publishes/subscribes and
     *          processes any notification received
     *
     * NOTE: Not supported currently
     */
    CAPABILITY_DISCOVERY_ENABLED,

    /* Value for this item is: (Int)
     * Emergenciy call timer
     *
     * NOTE: Not supported currently
     */
    EMERGENCY_CALL_TIMER,

    /* Value for this item is: (Int)
     * When UE moves from non-Voice area to Voice
     * area, the device shall start a hysteresis timer
     * and domain selection will not notify IMS about
     * voice preference until the expiry of hysteresis
     * timer
     *
     * NOTE: Not supported currently
     */
    SSAC_HYSTERESIS_TIMER,

    /* VoLTE user opted in status.
     * Value is in Integer format.
     * Opted-in (1) Opted-out (0).
     */
    VOLTE_USER_OPT_IN_STATUS,

    /* Proxy for Call Session Control Function(P-CSCF)
     * address for Local-BreakOut(LBO).
     * Value is in String format.
     */
    LBO_PCSCF_ADDRESS,

    /* Keep Alive Enabled for SIP.
     * Value is in Integer format. On(1), OFF(0).
     */
    KEEP_ALIVE_ENABLED,

    /* Registration retry Base Time value
     * in seconds.
     * Value is in Integer format.
     */
    REGISTRATION_RETRY_BASE_TIME_SEC,

    /* Registration retry Max Time value
     * in seconds.
     * Value is in Integer format.
     */
    REGISTRATION_RETRY_MAX_TIME_SEC,

    /* Smallest RTP port for speech codec.
     * Value is in integer format.
     */
    SPEECH_START_PORT,

    /* Largest RTP port for speech code.
     * Value is in Integer format.
     */
    SPEECH_END_PORT,

    /* SIP Timer A's value in msec.
     * Timer A is the INVITE request
     * retransmit interval, UDP only.
     * Value is in Integer format.
     */
    SIP_INVITE_REQ_RETX_INTERVAL_MSEC,

    /* SIP Timer B's value in msec.
     * Timer B is the wait time for
     * INVITE message to be acknowledged.
     * Value is in Integer format.
     */
    SIP_INVITE_RSP_WAIT_TIME_MSEC,

    /* SIP Timer D's value in msec.
     * Timer D is the wait time for
     * response retransmits of the
     * invite client transactions.
     * Value is in Integer format.
     */
    SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC,

    /* SIP Timer E's value in msec.
     * Timer E is the value
     * Non-INVITE request
     * retransmit interval, for
     * UDP only.
     * Value in Integer format.
     */
    SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC,

    /* SIP Timer F's value in msec
     * Timer F is the value of
     * Non-INVITE transaction
     * timeout timer.
     * Value is in Integer format.
     */
    SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC,

    /* SIP Timer G's value in msec.
     * Timer G is the value of INVITE
     * response retransmit interval.
     * Value is in Integer format.
     */
    SIP_INVITE_RSP_RETX_INTERVAL_MSEC,

    /* SIP Timer H's value in msec.
     * Timer H is value of wait time
     * for ACK receipt.
     * Value is in Integer format.
     */
    SIP_ACK_RECEIPT_WAIT_TIME_MSEC,

    /* SIP Timer I's value in msec.
     * Timer I is value of wait time for
     * ACK retransmits.
     * Value is in Integer format.
     */
    SIP_ACK_RETX_WAIT_TIME_MSEC,

    /* SIP Timer J's value in
     * msec. Timer J is the
     * value of wait time for
     * non-invite request
     * retransmission.
     * Value is in Integer
     * format.
     */
    SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC,

    /* SIP Timer K's value in
     * msec. Timer K is the
     * value of wait time for
     * non-invite response
     * retransmits.
     * Value is in Integer
     * format.
     */
    SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC,

    /* AMR WB octet aligned dynamic payload
     * type.
     * Value is in Integer format.
     */
    AMR_WB_OCTET_ALIGNED_PT,

    /* AMR WB bandwidth efficient
     * payload type.
     * Value is in Integer format.
     */
    AMR_WB_BANDWIDTH_EFFICIENT_PT,

    /* AMR octet aligned dynamic payload type.
     * Value is in Integer format.
     */
    AMR_OCTET_ALIGNED_PT,

    /* AMR bandwidth efficient payload
     * type.
     * Value is in Integer format.
     */
    AMR_BANDWIDTH_EFFICIENT_PT,


    /* DTMF WB payload type.
     * Value is in Integer format.
     */
    DTMF_WB_PT,

    /* DTMF NB payload type.
     * Value is in Integer format.
     */
    DTMF_NB_PT,

    /* AMR Default encoding mode.
     * Value is in Integer format.
     */
    AMR_DEFAULT_MODE,

    /* SMS Public Service Identity.
     * Value is in String format.
     */
    SMS_PSI,

    /* Value for this item is: (Int)
     * Video Quality values are
     *  - 0 -- Low Quality
     *  - 1 -- Medium Quality
     *  - 2 -- High Quality
     */
    VIDEO_QUALITY,

    /* Value for this item is: (Int)
     * Threshold for LTE to WiFi Handover
     */
    THRESHOLD_LTE1,

    /* Value for this item is: (Int)
     * Threshold for WiFi to LTE Handover
     */
    THRESHOLD_LTE2,

    /* Value for this item is: (Int)
     * Threshold for LTE to WiFi Handover
     * and WiFi to LTE Handover
     */
    THRESHOLD_LTE3,

    /* Value for this item is: (Int)
     * 1x Threshold value for comparing
     * averaged signal strength.
     */
    THRESHOLD_1x,

    /* Value for this item is: (Int)
     * Threshold for LTE to WiFi Handover
     */
    THRESHOLD_WIFI_A,

    /* Value for this item is: (Int)
     * Threshold for WiFi to LTE Handover
     */
    THRESHOLD_WIFI_B,

    /* Value for this item is: (Int)
     * Ims handover hysteresis timer for
     * WLAN to WWAN in seconds.
     */
    T_EPDG_LTE,

    /* Value for this item is: (Int)
     * Ims handover hysteresis timer for
     * WWAN to WLAN in seconds.
     */
    T_EPDG_WIFI,

    /* Value for this item is: (Int)
     * Ims handover hysteresis timer for
     * WLAN to 1x in seconds.
     */
    T_EPDG_1x,


    /* Value for this item is: (Int)
     * Enable VoWiFi through client
     * provisioning.
     * Values
     *   - 0 -- DISABLED
     *   - 1 -- ENABLED
     */
    VWF_SETTING_ENABLED,

    /* Value for this item is: (Int)
     * Enable VICE through client
     * provisioning.
     * Values
     *   - 0 -- DISABLED
     *   - 1 -- ENABLED
     */
    VCE_SETTING_ENABLED,
    /* Value for this item is: (Int)
     * Enable RTT through client
     * provisioning.
     * Values
     *   - 0 -- DISABLED
     *   - 1 -- ENABLED
     */
    RTT_SETTING_ENABLED,

    /* Value for this item is: (Int)
     * Report sms app selection through
     * client provisioning.
     * Values
     *   - 0 -- SMS app selection not allowed.
     *   - 1 -- RCS app is default sms app.
     *   - 2 -- RCS app is not default sms app.
     */
    SMS_APP,

    /* Value for this item is: (Int)
     * Report vvm app selection through
     * client provisioning.
     * Values
     *   - 0 -- RCS app is default vvm app.
     *   - 1 -- RCS app is not default vvm app.
     */
    VVM_APP,

    /* Value for this item is: (Int)
     * Wi-Fi Call Mode Roaming Preference
     * Values:
     *   - 0 -- WIFI_ONLY/WLAN_ONLY
     *   - 1 -- CELLULAR_PREFERRED
     *   - 2 -- WIFI_PREFERRED/WLAN_PREFERRED
     *   - 10 -- IMS_PREFERRED
     *
     * Note: Generic error will be returned
     * if the value is not any of the above.
     */
    VOICE_OVER_WIFI_ROAMING_MODE,

     /* Value for this item is: (Int)
      * Auto Reject Call Preference
      * Values:
      *   - 0 -- DISABLED
      *   - 1 -- ENABLED
      *
      * Note: Generic error will be returned
      * if the value is not any of the above.
      */
    AUTO_REJECT_CALL_ENABLED,

    INVALID_CONFIG
};

inline std::string toString(const ConfigItem &o) {
  if (o == ConfigItem::NONE) {
    return "NONE";
  }
  if (o == ConfigItem::VOCODER_AMRMODESET) {
    return "VOCODER_AMRMODESET";
  }
  if (o == ConfigItem::VOCODER_AMRWBMODESET) {
    return "VOCODER_AMRWBMODESET";
  }
  if (o == ConfigItem::SIP_SESSION_TIMER) {
    return "SIP_SESSION_TIMER";
  }
  if (o == ConfigItem::MIN_SESSION_EXPIRY) {
    return "MIN_SESSION_EXPIRY";
  }
  if (o == ConfigItem::CANCELLATION_TIMER) {
    return "CANCELLATION_TIMER";
  }
  if (o == ConfigItem::T_DELAY) {
    return "T_DELAY";
  }
  if (o == ConfigItem::SILENT_REDIAL_ENABLE) {
    return "SILENT_REDIAL_ENABLE";
  }
  if (o == ConfigItem::SIP_T1_TIMER) {
    return "SIP_T1_TIMER";
  }
  if (o == ConfigItem::SIP_T2_TIMER) {
    return "SIP_T2_TIMER";
  }
  if (o == ConfigItem::SIP_TF_TIMER) {
    return "SIP_TF_TIMER";
  }
  if (o == ConfigItem::VLT_SETTING_ENABLED) {
    return "VLT_SETTING_ENABLED";
  }
  if (o == ConfigItem::LVC_SETTING_ENABLED) {
    return "LVC_SETTING_ENABLED";
  }
  if (o == ConfigItem::DOMAIN_NAME) {
    return "DOMAIN_NAME";
  }
  if (o == ConfigItem::SMS_FORMAT) {
    return "SMS_FORMAT";
  }
  if (o == ConfigItem::SMS_OVER_IP) {
    return "SMS_OVER_IP";
  }
  if (o == ConfigItem::PUBLISH_TIMER) {
    return "PUBLISH_TIMER";
  }
  if (o == ConfigItem::PUBLISH_TIMER_EXTENDED) {
    return "PUBLISH_TIMER_EXTENDED";
  }
  if (o == ConfigItem::CAPABILITIES_CACHE_EXPIRATION) {
    return "CAPABILITIES_CACHE_EXPIRATION";
  }
  if (o == ConfigItem::AVAILABILITY_CACHE_EXPIRATION) {
    return "AVAILABILITY_CACHE_EXPIRATION";
  }
  if (o == ConfigItem::CAPABILITIES_POLL_INTERVAL) {
    return "CAPABILITIES_POLL_INTERVAL";
  }
  if (o == ConfigItem::SOURCE_THROTTLE_PUBLISH) {
    return "SOURCE_THROTTLE_PUBLISH";
  }
  if (o == ConfigItem::MAX_NUM_ENTRIES_IN_RCL) {
    return "MAX_NUM_ENTRIES_IN_RCL";
  }
  if (o == ConfigItem::CAPAB_POLL_LIST_SUB_EXP) {
    return "CAPAB_POLL_LIST_SUB_EXP";
  }
  if (o == ConfigItem::GZIP_FLAG) {
    return "GZIP_FLAG";
  }
  if (o == ConfigItem::EAB_SETTING_ENABLED) {
    return "EAB_SETTING_ENABLED";
  }
  if (o == ConfigItem::MOBILE_DATA_ENABLED) {
    return "MOBILE_DATA_ENABLED";
  }
  if (o == ConfigItem::VOICE_OVER_WIFI_ENABLED) {
    return "VOICE_OVER_WIFI_ENABLED";
  }
  if (o == ConfigItem::VOICE_OVER_WIFI_ROAMING) {
    return "VOICE_OVER_WIFI_ROAMING";
  }
  if (o == ConfigItem::VOICE_OVER_WIFI_MODE) {
    return "VOICE_OVER_WIFI_MODE";
  }
  if (o == ConfigItem::CAPABILITY_DISCOVERY_ENABLED) {
    return "CAPABILITY_DISCOVERY_ENABLED";
  }
  if (o == ConfigItem::EMERGENCY_CALL_TIMER) {
    return "EMERGENCY_CALL_TIMER";
  }
  if (o == ConfigItem::SSAC_HYSTERESIS_TIMER) {
    return "SSAC_HYSTERESIS_TIMER";
  }
  if (o == ConfigItem::VOLTE_USER_OPT_IN_STATUS) {
    return "VOLTE_USER_OPT_IN_STATUS";
  }
  if (o == ConfigItem::LBO_PCSCF_ADDRESS) {
    return "LBO_PCSCF_ADDRESS";
  }
  if (o == ConfigItem::KEEP_ALIVE_ENABLED) {
    return "KEEP_ALIVE_ENABLED";
  }
  if (o == ConfigItem::REGISTRATION_RETRY_BASE_TIME_SEC) {
    return "REGISTRATION_RETRY_BASE_TIME_SEC";
  }
  if (o == ConfigItem::REGISTRATION_RETRY_MAX_TIME_SEC) {
    return "REGISTRATION_RETRY_MAX_TIME_SEC";
  }
  if (o == ConfigItem::SPEECH_START_PORT) {
    return "SPEECH_START_PORT";
  }
  if (o == ConfigItem::SPEECH_END_PORT) {
    return "SPEECH_END_PORT";
  }
  if (o == ConfigItem::SIP_INVITE_REQ_RETX_INTERVAL_MSEC) {
    return "SIP_INVITE_REQ_RETX_INTERVAL_MSEC";
  }
  if (o == ConfigItem::SIP_INVITE_RSP_WAIT_TIME_MSEC) {
    return "SIP_INVITE_RSP_WAIT_TIME_MSEC";
  }
  if (o == ConfigItem::SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC) {
    return "SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC";
  }
  if (o == ConfigItem::SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC) {
    return "SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC";
  }
  if (o == ConfigItem::SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC) {
    return "SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC";
  }
  if (o == ConfigItem::SIP_INVITE_RSP_RETX_INTERVAL_MSEC) {
    return "SIP_INVITE_RSP_RETX_INTERVAL_MSEC";
  }
  if (o == ConfigItem::SIP_ACK_RECEIPT_WAIT_TIME_MSEC) {
    return "SIP_ACK_RECEIPT_WAIT_TIME_MSEC";
  }
  if (o == ConfigItem::SIP_ACK_RETX_WAIT_TIME_MSEC) {
    return "SIP_ACK_RETX_WAIT_TIME_MSEC";
  }
  if (o == ConfigItem::SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC) {
    return "SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC";
  }
  if (o == ConfigItem::SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC) {
    return "SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC";
  }
  if (o == ConfigItem::AMR_WB_OCTET_ALIGNED_PT) {
    return "AMR_WB_OCTET_ALIGNED_PT";
  }
  if (o == ConfigItem::AMR_WB_BANDWIDTH_EFFICIENT_PT) {
    return "AMR_WB_BANDWIDTH_EFFICIENT_PT";
  }
  if (o == ConfigItem::AMR_OCTET_ALIGNED_PT) {
    return "AMR_OCTET_ALIGNED_PT";
  }
  if (o == ConfigItem::AMR_BANDWIDTH_EFFICIENT_PT) {
    return "AMR_BANDWIDTH_EFFICIENT_PT";
  }
  if (o == ConfigItem::DTMF_WB_PT) {
    return "DTMF_WB_PT";
  }
  if (o == ConfigItem::DTMF_NB_PT) {
    return "DTMF_NB_PT";
  }
  if (o == ConfigItem::AMR_DEFAULT_MODE) {
    return "AMR_DEFAULT_MODE";
  }
  if (o == ConfigItem::SMS_PSI) {
    return "SMS_PSI";
  }
  if (o == ConfigItem::VIDEO_QUALITY) {
    return "VIDEO_QUALITY";
  }
  if (o == ConfigItem::THRESHOLD_LTE1) {
    return "THRESHOLD_LTE1";
  }
  if (o == ConfigItem::THRESHOLD_LTE2) {
    return "THRESHOLD_LTE2";
  }
  if (o == ConfigItem::THRESHOLD_LTE3) {
    return "THRESHOLD_LTE3";
  }
  if (o == ConfigItem::THRESHOLD_1x) {
    return "THRESHOLD_1x";
  }
  if (o == ConfigItem::THRESHOLD_WIFI_A) {
    return "THRESHOLD_WIFI_A";
  }
  if (o == ConfigItem::THRESHOLD_WIFI_B) {
    return "THRESHOLD_WIFI_B";
  }
  if (o == ConfigItem::T_EPDG_LTE) {
    return "T_EPDG_LTE";
  }
  if (o == ConfigItem::T_EPDG_WIFI) {
    return "T_EPDG_WIFI";
  }
  if (o == ConfigItem::T_EPDG_1x) {
    return "T_EPDG_1x";
  }
  if (o == ConfigItem::VWF_SETTING_ENABLED) {
    return "VWF_SETTING_ENABLED";
  }
  if (o == ConfigItem::VCE_SETTING_ENABLED) {
    return "VCE_SETTING_ENABLED";
  }
  if (o == ConfigItem::RTT_SETTING_ENABLED) {
    return "RTT_SETTING_ENABLED";
  }
  if (o == ConfigItem::SMS_APP) {
    return "SMS_APP";
  }
  if (o == ConfigItem::VVM_APP) {
    return "VVM_APP";
  }
  if (o == ConfigItem::VOICE_OVER_WIFI_ROAMING_MODE) {
    return "VOICE_OVER_WIFI_ROAMING_MODE";
  }
  if (o == ConfigItem::AUTO_REJECT_CALL_ENABLED) {
    return "AUTO_REJECT_CALL_ENABLED";
  }
  if (o == ConfigItem::INVALID_CONFIG) {
    return "INVALID_CONFIG";
  }
  return "<invalid>";
}

enum class ConfigFailureCause {
    NO_ERR,
    IMS_NOT_READY,
    FILE_NOT_AVAILABLE,
    READ_FAILED,
    WRITE_FAILED,
    OTHER_INTERNAL_ERR,
    FAILURE_INVALID
};
inline std::string toString(const ConfigFailureCause &o) {
  if (o == ConfigFailureCause::NO_ERR) {
    return "NO_ERR";
  }
  if (o == ConfigFailureCause::IMS_NOT_READY) {
    return "IMS_NOT_READY";
  }
  if (o == ConfigFailureCause::FILE_NOT_AVAILABLE) {
    return "FILE_NOT_AVAILABLE";
  }
  if (o == ConfigFailureCause::READ_FAILED) {
    return "READ_FAILED";
  }
  if (o == ConfigFailureCause::WRITE_FAILED) {
    return "WRITE_FAILED";
  }
  if (o == ConfigFailureCause::OTHER_INTERNAL_ERR) {
    return "OTHER_INTERNAL_ERR";
  }
  if (o == ConfigFailureCause::FAILURE_INVALID) {
    return "FAILURE_INVALID";
  }
  return "<invalid>";
}

class Registration : public qcril::interfaces::BasePayload {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(RegState, State);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, ErrorCode);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, ErrorMessage);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(RIL_RadioTechnology, RadioTechnology);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, PAssociatedUris);

 public:
  Registration() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(State);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(ErrorCode);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(ErrorMessage);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(RadioTechnology);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(PAssociatedUris);
  }
  Registration(const Registration &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, State);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, ErrorCode);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, ErrorMessage);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, RadioTechnology);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, PAssociatedUris);
  }
  virtual ~Registration() {}
};

class AccessTechnologyStatus : public qcril::interfaces::BasePayload {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(RIL_RadioTechnology, NetworkMode);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(StatusType, StatusType);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, RestrictCause);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_SHARED_PTR(Registration, Registration);

 public:
  AccessTechnologyStatus() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(NetworkMode);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(StatusType);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(RestrictCause);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Registration);
  }
  AccessTechnologyStatus(const AccessTechnologyStatus &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, NetworkMode);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, StatusType);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, RestrictCause);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Registration);
  }
  virtual ~AccessTechnologyStatus() {}
};

class ServiceStatusInfo : public qcril::interfaces::BasePayload {
  // QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(bool, IsValid);
  // QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(ServiceType, ServiceType);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(CallType, CallType);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(StatusType, StatusType);
  // QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_VECTOR(uint8_t, UserData);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint8_t, RestrictCause);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_SHARED_PTR(AccessTechnologyStatus, AccessTechnologyStatus);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(RttMode, RttMode);

 public:
  ServiceStatusInfo() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(CallType);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(StatusType);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(RestrictCause);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(AccessTechnologyStatus);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(RttMode);
  }
  ServiceStatusInfo(const ServiceStatusInfo &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, CallType);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, StatusType);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, RestrictCause);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, AccessTechnologyStatus);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, RttMode);
  }
  virtual ~ServiceStatusInfo() {}
};

class ServiceStatusInfoList : public qcril::interfaces::BasePayload {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_VECTOR(ServiceStatusInfo, ServiceStatusInfo);

 public:
  ServiceStatusInfoList() { QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR_VECTOR(ServiceStatusInfo); }
  ServiceStatusInfoList(const ServiceStatusInfoList &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR_VECTOR(from, ServiceStatusInfo);
  }
  virtual ~ServiceStatusInfoList() {}
};

class SipErrorInfo : public qcril::interfaces::BasePayload {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, ErrorCode);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, ErrorString);

 public:
  SipErrorInfo() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(ErrorCode);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(ErrorString);
  }

  SipErrorInfo(const SipErrorInfo &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, ErrorCode);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, ErrorString);
  }
};

class RtpStatisticsData : public qcril::interfaces::BasePayload {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint64_t, Count);

public:
  RtpStatisticsData() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Count);
  }
  RtpStatisticsData(uint64_t count) {
    setCount(count);
  }
  ~RtpStatisticsData() {};
};

class ImsSubConfigInfo : public qcril::interfaces::BasePayload {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, SimultStackCount);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, Count);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_VECTOR(uint8_t, ImsStackEnabled);

public:
  ImsSubConfigInfo() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(SimultStackCount);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Count);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR_VECTOR(ImsStackEnabled);
  }
  ~ImsSubConfigInfo() {};
};

class AddressInfo {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, City);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, State);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, Country);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, PostalCode);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, CountryCode);
public:
  AddressInfo() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(City);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(State);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Country);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(PostalCode);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(CountryCode);
  }
  AddressInfo(const AddressInfo &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, City);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, State);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Country);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, PostalCode);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, CountryCode);
  }
  ~AddressInfo() { };
};


class GeoLocationInfo : public qcril::interfaces::BasePayload {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(double, Latitude);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(double, Longitude);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(AddressInfo, AddressInfo);
public:
  GeoLocationInfo() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Latitude);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Longitude);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(AddressInfo);
  }
  GeoLocationInfo(const GeoLocationInfo &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Latitude);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Longitude);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, AddressInfo);
  }
  ~GeoLocationInfo() {};
};

class SsacInfo : public qcril::interfaces::BasePayload {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, BarringFactorVoice);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, BarringTimeVoice);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, BarringFactorVideo);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, BarringTimeVideo);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, BarringFactorVoiceSib);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, BarringTimeVoiceSib);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, BarringFactorVideoSib);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, BarringTimeVideoSib);
public:
  SsacInfo() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(BarringFactorVoice);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(BarringTimeVoice);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(BarringFactorVideo);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(BarringTimeVideo);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(BarringFactorVoiceSib);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(BarringTimeVoiceSib);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(BarringFactorVideoSib);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(BarringTimeVideoSib);
  }
  SsacInfo(const SsacInfo &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, BarringFactorVoice);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, BarringTimeVoice);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, BarringFactorVideo);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, BarringTimeVideo);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, BarringFactorVoiceSib);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, BarringTimeVoiceSib);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, BarringFactorVideoSib);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, BarringTimeVideoSib);
  }
  ~SsacInfo() {}
};

class BlockReasonDetails {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(RegFailureReasonType, RegFailureReasonType);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, RegFailureReason);
public:
  BlockReasonDetails() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(RegFailureReasonType);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(RegFailureReason);
  }
  BlockReasonDetails(const BlockReasonDetails &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, RegFailureReasonType);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, RegFailureReason);
  }
  ~BlockReasonDetails() {}

};

class BlockStatus {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(BlockReasonType, BlockReason);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_SHARED_PTR(BlockReasonDetails, BlockReasonDetails);
public:
  BlockStatus() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(BlockReason);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(BlockReasonDetails);
  }
  BlockStatus(const BlockStatus &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, BlockReason);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, BlockReasonDetails);
  }
  ~BlockStatus() {}
};

class RegistrationBlockStatus : public qcril::interfaces::BasePayload {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_SHARED_PTR(BlockStatus, BlockStatusOnWwan);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR_SHARED_PTR(BlockStatus, BlockStatusOnWlan);
public:
  RegistrationBlockStatus() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(BlockStatusOnWwan);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(BlockStatusOnWlan);
  }
  RegistrationBlockStatus(const RegistrationBlockStatus &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, BlockStatusOnWwan);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, BlockStatusOnWlan);
  }
  ~RegistrationBlockStatus() {}
};

class ConfigInfo : public qcril::interfaces::BasePayload {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(ConfigItem,Item);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(bool, BoolValue);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(uint32_t, IntValue);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, StringValue);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(ConfigFailureCause, ErrorCause);
public:
  ConfigInfo() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Item);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(BoolValue);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(IntValue);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(StringValue);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(ErrorCause);
  }
  ConfigInfo(const ConfigInfo &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Item);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, BoolValue);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, IntValue);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, StringValue);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, ErrorCause);
  }

  ~ConfigInfo() {}
};

inline std::string toString(const ConfigInfo &o) {
  std::string os;
  os += "{";
  os += ".Item=";
  os += o.hasItem() ? toString(o.getItem()) : "<invalid>";
  os += ",.BoolValue=";
  os += o.hasBoolValue() ? (o.getBoolValue() ? "true" : "false") : "<invalid>";
  os += ",.IntValue=";
  os += o.hasIntValue() ? std::to_string(o.getIntValue()) : "<invalid>";
  os += ",.StringValue=";
  os += o.hasStringValue() ? o.getStringValue() : "<invalid>";
  os += ",.ErrorCause=";
  os += o.hasErrorCause() ? toString(o.getErrorCause()) : "<invalid>";
  os += "}";
  return os;
}

struct MultiIdentityInfo{
    static const int LINE_TYPE_UNKNOWN = 0;
    static const int LINE_TYPE_PRIMARY = 1;
    static const int LINE_TYPE_SECONDARY = 2;

    static const int REGISTRATION_STATUS_UNKNOWN = 0;
    static const int REGISTRATION_STATUS_ACTIVE = 1;
    static const int REGISTRATION_STATUS_INACTIVE = 2;

    std::string msisdn;
    int registrationStatus;
    int lineType;

    MultiIdentityInfo(std::string msisdn = std::string(),
            int registrationStatus = REGISTRATION_STATUS_UNKNOWN,
            int lineType = LINE_TYPE_UNKNOWN) {
        this->msisdn = msisdn;
        this->registrationStatus = registrationStatus;
        this->lineType = lineType;
    }
    MultiIdentityInfo(const MultiIdentityInfo &from) {
        msisdn = from.msisdn;
        registrationStatus = from.registrationStatus;
        lineType = from.lineType;
    }
};

class VirtualLineInfo : public qcril::interfaces::BasePayload {
private:
  std::string mMsisdn;
  std::vector<std::string> mVirtualLines;
public:
  VirtualLineInfo() {};
  VirtualLineInfo(const std::string& msisdn, const std::vector<std::string> lines) :
    mMsisdn(msisdn), mVirtualLines(lines) {
  };
  VirtualLineInfo(const VirtualLineInfo &from) {
    mMsisdn = from.mMsisdn;
    mVirtualLines = from.mVirtualLines;
  }
  ~VirtualLineInfo() {};

  std::string getMsisdn() { return mMsisdn; }
  std::vector<std::string>& getVirtualLines() { return mVirtualLines; };
};

} // namespace interfaces
} // namespace qcril
