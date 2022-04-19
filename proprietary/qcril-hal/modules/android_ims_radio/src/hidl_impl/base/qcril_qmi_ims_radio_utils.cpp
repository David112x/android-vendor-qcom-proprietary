/******************************************************************************
  @file    qcril_qmi_ims_radio_utils.cpp
  @brief   qcril qmi - ims_radio_utils

  DESCRIPTION
    Implements the utils to be used by the ImsRadioImpl class

  ---------------------------------------------------------------------------

  Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/

#define TAG "RILQ"

#include "modules/android_ims_radio/hidl_impl/base/qcril_qmi_ims_radio_utils.h"
#include <string.h>
#include <framework/Log.h>
#include "utils_standard.h"
#include <modules/nas/qcril_qmi_nas.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_0 {
namespace utils {

//===========================================================================
// qcril_qmi_ims_map_ril_error_to_ims_error
//===========================================================================
ImsRadioErrorCode qcril_qmi_ims_map_ril_error_to_ims_error(int ril_error)
{
  ImsRadioErrorCode ret;

  switch ( ril_error )
  {
    case RIL_E_SUCCESS:
      ret = ImsRadioErrorCode::SUCCESS;
      break;

    case RIL_E_RADIO_NOT_AVAILABLE:
      ret = ImsRadioErrorCode::RADIO_NOT_AVAILABLE;
      break;

    case RIL_E_GENERIC_FAILURE:
      ret = ImsRadioErrorCode::GENERIC_FAILURE;
      break;

    case RIL_E_REQUEST_NOT_SUPPORTED:
      ret = ImsRadioErrorCode::REQUEST_NOT_SUPPORTED;
      break;

    case RIL_E_CANCELLED:
      ret = ImsRadioErrorCode::CANCELLED;
      break;

    case RIL_E_HOLD_RESUME_FAILED:
      ret = ImsRadioErrorCode::HOLD_RESUME_FAILED;
      break;

    case RIL_E_HOLD_RESUME_CANCELED:
      ret = ImsRadioErrorCode::HOLD_RESUME_CANCELED;
      break;
    case RIL_E_REINVITE_COLLISION:
      ret = ImsRadioErrorCode::REINVITE_COLLISION;
      break;

    case RIL_E_FDN_CHECK_FAILURE:
      ret = ImsRadioErrorCode::FDN_CHECK_FAILURE;
      break;

    case RIL_E_DIAL_MODIFIED_TO_DIAL:
      ret = ImsRadioErrorCode::DIAL_MODIFIED_TO_DIAL;
      break;

    case RIL_E_DIAL_MODIFIED_TO_DIAL_VIDEO:
      ret = ImsRadioErrorCode::DIAL_MODIFIED_TO_DIAL_VIDEO;
      break;

    case RIL_E_DIAL_MODIFIED_TO_SS:
      ret = ImsRadioErrorCode::DIAL_MODIFIED_TO_SS;
      break;

    case RIL_E_DIAL_MODIFIED_TO_USSD:
      ret = ImsRadioErrorCode::DIAL_MODIFIED_TO_USSD;
      break;

    case RIL_E_DIAL_VIDEO_MODIFIED_TO_DIAL:
      ret = ImsRadioErrorCode::DIAL_VIDEO_MODIFIED_TO_DIAL;
      break;

    case RIL_E_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO:
      ret = ImsRadioErrorCode::DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO;
      break;

    case RIL_E_DIAL_VIDEO_MODIFIED_TO_SS:
      ret = ImsRadioErrorCode::DIAL_VIDEO_MODIFIED_TO_SS;
      break;

    case RIL_E_DIAL_VIDEO_MODIFIED_TO_USSD:
      ret = ImsRadioErrorCode::DIAL_VIDEO_MODIFIED_TO_USSD;
      break;

    case RIL_E_SS_MODIFIED_TO_DIAL:
      ret = ImsRadioErrorCode::SS_MODIFIED_TO_DIAL;
      break;

    case RIL_E_SS_MODIFIED_TO_DIAL_VIDEO:
      ret = ImsRadioErrorCode::SS_MODIFIED_TO_DIAL_VIDEO;
      break;

    case RIL_E_SS_MODIFIED_TO_SS:
      ret = ImsRadioErrorCode::SS_MODIFIED_TO_SS;
      break;

    case RIL_E_SS_MODIFIED_TO_USSD:
      ret = ImsRadioErrorCode::SS_MODIFIED_TO_USSD;
      break;

    case RIL_E_REJECTED_BY_REMOTE:
      ret = ImsRadioErrorCode::REJECTED_BY_REMOTE;
      break;

    case RIL_E_NETWORK_NOT_SUPPORTED:
      ret = ImsRadioErrorCode::NETWORK_NOT_SUPPORTED;
      break;

    case RIL_E_INVALID_ARGUMENTS:
      ret = ImsRadioErrorCode::INVALID_PARAMETER;
      break;

    default:
      ret = ImsRadioErrorCode::GENERIC_FAILURE;
  }

  QCRIL_LOG_INFO("ril error %d mapped to ims error %d", ril_error, ret);
  return ret;
} // qcril_qmi_ims_map_ril_error_to_ims_error

qcril::interfaces::RegState convertRegState(const RegState &in) {
  switch (in) {
  case RegState::REGISTERED:
    return qcril::interfaces::RegState::REGISTERED;
  case RegState::NOT_REGISTERED:
    return qcril::interfaces::RegState::NOT_REGISTERED;
  case RegState::REGISTERING:
    return qcril::interfaces::RegState::REGISTERING;
  case RegState::INVALID:
  default:
    return qcril::interfaces::RegState::UNKNOWN;
  }
}

RegState convertRegState(const qcril::interfaces::RegState &in) {
  switch (in) {
  case qcril::interfaces::RegState::REGISTERED:
    return RegState::REGISTERED;
  case qcril::interfaces::RegState::NOT_REGISTERED:
    return RegState::NOT_REGISTERED;
  case qcril::interfaces::RegState::REGISTERING:
    return RegState::REGISTERING;
  case qcril::interfaces::RegState::UNKNOWN:
  default:
    return RegState::INVALID;
  }
}

RIL_RadioTechnology convertRadioTechType(const RadioTechType &in) {
  switch (in) {
  case RadioTechType::RADIO_TECH_ANY:
    return RADIO_TECH_ANY;
  case RadioTechType::RADIO_TECH_UNKNOWN:
    return RADIO_TECH_UNKNOWN;
  case RadioTechType::RADIO_TECH_GPRS:
    return RADIO_TECH_GPRS;
  case RadioTechType::RADIO_TECH_EDGE:
    return RADIO_TECH_EDGE;
  case RadioTechType::RADIO_TECH_UMTS:
    return RADIO_TECH_UMTS;
  case RadioTechType::RADIO_TECH_IS95A:
    return RADIO_TECH_IS95A;
  case RadioTechType::RADIO_TECH_IS95B:
    return RADIO_TECH_IS95B;
  case RadioTechType::RADIO_TECH_1xRTT:
    return RADIO_TECH_1xRTT;
  case RadioTechType::RADIO_TECH_EVDO_0:
    return RADIO_TECH_EVDO_0;
  case RadioTechType::RADIO_TECH_EVDO_A:
    return RADIO_TECH_EVDO_A;
  case RadioTechType::RADIO_TECH_HSDPA:
    return RADIO_TECH_HSDPA;
  case RadioTechType::RADIO_TECH_HSUPA:
    return RADIO_TECH_HSUPA;
  case RadioTechType::RADIO_TECH_HSPA:
    return RADIO_TECH_HSPA;
  case RadioTechType::RADIO_TECH_EVDO_B:
    return RADIO_TECH_EVDO_B;
  case RadioTechType::RADIO_TECH_EHRPD:
    return RADIO_TECH_EHRPD;
  case RadioTechType::RADIO_TECH_LTE:
    return RADIO_TECH_LTE;
  case RadioTechType::RADIO_TECH_HSPAP:
    return RADIO_TECH_HSPAP;
  case RadioTechType::RADIO_TECH_GSM:
    return RADIO_TECH_GSM;
  case RadioTechType::RADIO_TECH_TD_SCDMA:
    return RADIO_TECH_TD_SCDMA;
  case RadioTechType::RADIO_TECH_WIFI:
    return RADIO_TECH_WIFI;
  case RadioTechType::RADIO_TECH_IWLAN:
    return RADIO_TECH_IWLAN;
  case RadioTechType::RADIO_TECH_INVALID:
  default:
    return RADIO_TECH_UNKNOWN;
  }
}

RadioTechType convertRadioTechType(const RIL_RadioTechnology &in) {
  switch (in) {
    case RADIO_TECH_ANY:
      return RadioTechType::RADIO_TECH_ANY;
    case RADIO_TECH_UNKNOWN:
      return RadioTechType::RADIO_TECH_UNKNOWN;
    case RADIO_TECH_GPRS:
      return RadioTechType::RADIO_TECH_GPRS;
    case RADIO_TECH_EDGE:
      return RadioTechType::RADIO_TECH_EDGE;
    case RADIO_TECH_UMTS:
      return RadioTechType::RADIO_TECH_UMTS;
    case RADIO_TECH_IS95A:
      return RadioTechType::RADIO_TECH_IS95A;
    case RADIO_TECH_IS95B:
      return RadioTechType::RADIO_TECH_IS95B;
    case RADIO_TECH_1xRTT:
      return RadioTechType::RADIO_TECH_1xRTT;
    case RADIO_TECH_EVDO_0:
      return RadioTechType::RADIO_TECH_EVDO_0;
    case RADIO_TECH_EVDO_A:
      return RadioTechType::RADIO_TECH_EVDO_A;
    case RADIO_TECH_HSDPA:
      return RadioTechType::RADIO_TECH_HSDPA;
    case RADIO_TECH_HSUPA:
      return RadioTechType::RADIO_TECH_HSUPA;
    case RADIO_TECH_HSPA:
      return RadioTechType::RADIO_TECH_HSPA;
    case RADIO_TECH_EVDO_B:
      return RadioTechType::RADIO_TECH_EVDO_B;
    case RADIO_TECH_EHRPD:
      return RadioTechType::RADIO_TECH_EHRPD;
    case RADIO_TECH_LTE:
      return RadioTechType::RADIO_TECH_LTE;
    case RADIO_TECH_HSPAP:
      return RadioTechType::RADIO_TECH_HSPAP;
    case RADIO_TECH_GSM:
      return RadioTechType::RADIO_TECH_GSM;
    case RADIO_TECH_TD_SCDMA:
      return RadioTechType::RADIO_TECH_TD_SCDMA;
    case RADIO_TECH_WIFI:
      return RadioTechType::RADIO_TECH_WIFI;
    case RADIO_TECH_IWLAN:
      return RadioTechType::RADIO_TECH_IWLAN;
//    case RADIO_TECH_UNKNOWN:
    default:
      return RadioTechType::RADIO_TECH_INVALID;
  }
}

CallState convertCallState(const qcril::interfaces::CallState in) {
  switch (in) {
  case qcril::interfaces::CallState::ACTIVE:
    return CallState::CALL_ACTIVE;
  case qcril::interfaces::CallState::HOLDING:
    return CallState::CALL_HOLDING;
  case qcril::interfaces::CallState::DIALING:
    return CallState::CALL_DIALING;
  case qcril::interfaces::CallState::ALERTING:
    return CallState::CALL_ALERTING;
  case qcril::interfaces::CallState::INCOMING:
    return CallState::CALL_INCOMING;
  case qcril::interfaces::CallState::WAITING:
    return CallState::CALL_WAITING;
  case qcril::interfaces::CallState::END:
    return CallState::CALL_END;
  default:
    return CallState::CALL_STATE_INVALID;
  }
}
qcril::interfaces::CallType convertCallType(const CallType &in) {
  switch (in) {
  case CallType::CALL_TYPE_VOICE:
    return qcril::interfaces::CallType::VOICE;
  case CallType::CALL_TYPE_VT_TX:
    return qcril::interfaces::CallType::VT_TX;
  case CallType::CALL_TYPE_VT_RX:
    return qcril::interfaces::CallType::VT_RX;
  case CallType::CALL_TYPE_VT:
    return qcril::interfaces::CallType::VT;
  case CallType::CALL_TYPE_VT_NODIR:
    return qcril::interfaces::CallType::VT_NODIR;
  case CallType::CALL_TYPE_SMS:
    return qcril::interfaces::CallType::SMS;
  case CallType::CALL_TYPE_UT:
    return qcril::interfaces::CallType::UT;
  case CallType::CALL_TYPE_CS_VS_TX:
  case CallType::CALL_TYPE_CS_VS_RX:
  case CallType::CALL_TYPE_PS_VS_TX:
  case CallType::CALL_TYPE_PS_VS_RX:
  case CallType::CALL_TYPE_UNKNOWN:
  default:
    return qcril::interfaces::CallType::UNKNOWN;
  }
}

CallType convertCallType(const qcril::interfaces::CallType &in) {
  switch (in) {
  case qcril::interfaces::CallType::VOICE:
    return CallType::CALL_TYPE_VOICE;
  case qcril::interfaces::CallType::VT_TX:
    return CallType::CALL_TYPE_VT_TX;
  case qcril::interfaces::CallType::VT_RX:
    return CallType::CALL_TYPE_VT_RX;
  case qcril::interfaces::CallType::VT:
    return CallType::CALL_TYPE_VT;
  case qcril::interfaces::CallType::VT_NODIR:
    return CallType::CALL_TYPE_VT_NODIR;
  case qcril::interfaces::CallType::SMS:
    return CallType::CALL_TYPE_SMS;
  case qcril::interfaces::CallType::UT:
    return CallType::CALL_TYPE_UT;
  case qcril::interfaces::CallType::UNKNOWN:
  default:
    return CallType::CALL_TYPE_UNKNOWN;
  }
}

qcril::interfaces::Presentation convertIpPresentation(const IpPresentation &in) {
  switch (in) {
    case IpPresentation::IP_PRESENTATION_NUM_ALLOWED:
      return qcril::interfaces::Presentation::ALLOWED;
    case IpPresentation::IP_PRESENTATION_NUM_RESTRICTED:
      return qcril::interfaces::Presentation::RESTRICTED;
    case IpPresentation::IP_PRESENTATION_NUM_DEFAULT:
    default:
      return qcril::interfaces::Presentation::DEFAULT;
  }
}

IpPresentation convertIpPresentation(const qcril::interfaces::Presentation &in) {
    switch (in) {
      case qcril::interfaces::Presentation::ALLOWED:
            return IpPresentation::IP_PRESENTATION_NUM_ALLOWED;
        case qcril::interfaces::Presentation::RESTRICTED:
            return IpPresentation::IP_PRESENTATION_NUM_RESTRICTED;
        case qcril::interfaces::Presentation::DEFAULT:
            return IpPresentation::IP_PRESENTATION_NUM_DEFAULT;
        default:
            return IpPresentation::IP_PRESENTATION_INVALID;
    }
}

qcril::interfaces::CallDomain convertCallDomain(const CallDomain &in) {
  switch (in) {
  case CallDomain::CALL_DOMAIN_UNKNOWN:
    return qcril::interfaces::CallDomain::UNKNOWN;
  case CallDomain::CALL_DOMAIN_CS:
    return qcril::interfaces::CallDomain::CS;
  case CallDomain::CALL_DOMAIN_PS:
    return qcril::interfaces::CallDomain::PS;
  case CallDomain::CALL_DOMAIN_AUTOMATIC:
    return qcril::interfaces::CallDomain::AUTOMATIC;
  case CallDomain::CALL_DOMAIN_NOT_SET:
  default:
    return qcril::interfaces::CallDomain::NOT_SET;
  }
}

CallDomain convertCallDomain(const qcril::interfaces::CallDomain &in) {
  switch (in) {
  case qcril::interfaces::CallDomain::UNKNOWN:
    return CallDomain::CALL_DOMAIN_UNKNOWN;
  case qcril::interfaces::CallDomain::CS:
    return CallDomain::CALL_DOMAIN_CS;
  case qcril::interfaces::CallDomain::PS:
    return CallDomain::CALL_DOMAIN_PS;
  case qcril::interfaces::CallDomain::AUTOMATIC:
    return CallDomain::CALL_DOMAIN_AUTOMATIC;
  case qcril::interfaces::CallDomain::NOT_SET:
  default:
    return CallDomain::CALL_DOMAIN_NOT_SET;
  }
}

qcril::interfaces::RttMode convertRttMode(const RttMode &in) {
  switch (in) {
  case RttMode::RTT_MODE_DISABLED:
    return qcril::interfaces::RttMode::DISABLED;
  case RttMode::RTT_MODE_FULL:
    return qcril::interfaces::RttMode::FULL;
  case RttMode::RTT_MODE_INVALID:
  default:
    return qcril::interfaces::RttMode::UNKNOWN;
  }
}

RttMode convertRttMode(const qcril::interfaces::RttMode &in) {
  switch (in) {
  case qcril::interfaces::RttMode::DISABLED:
    return RttMode::RTT_MODE_DISABLED;
  case qcril::interfaces::RttMode::FULL:
    return RttMode::RTT_MODE_FULL;
  case qcril::interfaces::RttMode::UNKNOWN:
  default:
    return RttMode::RTT_MODE_INVALID;
  }
}

StatusType convertStatusType(const qcril::interfaces::StatusType &in) {
  switch (in) {
  case qcril::interfaces::StatusType::DISABLED:
    return StatusType::STATUS_DISABLED;
  case qcril::interfaces::StatusType::PARTIALLY_ENABLED:
    return StatusType::STATUS_PARTIALLY_ENABLED;
  case qcril::interfaces::StatusType::ENABLED:
    return StatusType::STATUS_ENABLED;
  case qcril::interfaces::StatusType::NOT_SUPPORTED:
    return StatusType::STATUS_NOT_SUPPORTED;
  case qcril::interfaces::StatusType::UNKNOWN:
  default:
    return StatusType::STATUS_INVALID;
  }
}

V1_3::CallFailCause convertCallFailCause(const qcril::interfaces::CallFailCause &in) {
  switch (in) {
  case qcril::interfaces::CallFailCause::UNOBTAINABLE_NUMBER:
    return V1_3::CallFailCause::CALL_FAIL_UNOBTAINABLE_NUMBER;
  case qcril::interfaces::CallFailCause::NORMAL:
    return V1_3::CallFailCause::CALL_FAIL_NORMAL;
  case qcril::interfaces::CallFailCause::BUSY:
    return V1_3::CallFailCause::CALL_FAIL_BUSY;
  case qcril::interfaces::CallFailCause::NORMAL_UNSPECIFIED:
    return V1_3::CallFailCause::CALL_FAIL_NORMAL_UNSPECIFIED;
  case qcril::interfaces::CallFailCause::CONGESTION:
    return V1_3::CallFailCause::CALL_FAIL_CONGESTION;
  case qcril::interfaces::CallFailCause::INCOMPATIBILITY_DESTINATION:
    return V1_3::CallFailCause::CALL_FAIL_INCOMPATIBILITY_DESTINATION;
  case qcril::interfaces::CallFailCause::CALL_BARRED:
    return V1_3::CallFailCause::CALL_FAIL_CALL_BARRED;
  case qcril::interfaces::CallFailCause::FDN_BLOCKED:
    return V1_3::CallFailCause::CALL_FAIL_FDN_BLOCKED;
  case qcril::interfaces::CallFailCause::IMEI_NOT_ACCEPTED:
    return V1_3::CallFailCause::CALL_FAIL_IMEI_NOT_ACCEPTED;
  case qcril::interfaces::CallFailCause::DIAL_MODIFIED_TO_USSD:
    return V1_3::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_USSD;
  case qcril::interfaces::CallFailCause::DIAL_MODIFIED_TO_SS:
    return V1_3::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_SS;
  case qcril::interfaces::CallFailCause::DIAL_MODIFIED_TO_DIAL:
    return V1_3::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL;
  case qcril::interfaces::CallFailCause::EMERGENCY_TEMP_FAILURE:
    return V1_3::CallFailCause::CALL_FAIL_EMERGENCY_TEMP_FAILURE;
  case qcril::interfaces::CallFailCause::EMERGENCY_PERM_FAILURE:
    return V1_3::CallFailCause::CALL_FAIL_EMERGENCY_PERM_FAILURE;
  case qcril::interfaces::CallFailCause::HO_NOT_FEASIBLE:
    return V1_3::CallFailCause::CALL_FAIL_HO_NOT_FEASIBLE;
  case qcril::interfaces::CallFailCause::USER_BUSY:
    return V1_3::CallFailCause::CALL_FAIL_USER_BUSY;
  case qcril::interfaces::CallFailCause::USER_REJECT:
    return V1_3::CallFailCause::CALL_FAIL_USER_REJECT;
  case qcril::interfaces::CallFailCause::LOW_BATTERY:
    return V1_3::CallFailCause::CALL_FAIL_LOW_BATTERY;
  case qcril::interfaces::CallFailCause::BLACKLISTED_CALL_ID:
    return V1_3::CallFailCause::CALL_FAIL_BLACKLISTED_CALL_ID;
  case qcril::interfaces::CallFailCause::CS_RETRY_REQUIRED:
    return V1_3::CallFailCause::CALL_FAIL_CS_RETRY_REQUIRED;
  case qcril::interfaces::CallFailCause::NETWORK_UNAVAILABLE:
    return V1_3::CallFailCause::CALL_FAIL_NETWORK_UNAVAILABLE;
  case qcril::interfaces::CallFailCause::FEATURE_UNAVAILABLE:
    return V1_3::CallFailCause::CALL_FAIL_FEATURE_UNAVAILABLE;
  case qcril::interfaces::CallFailCause::SIP_ERROR:
    return V1_3::CallFailCause::CALL_FAIL_SIP_ERROR;
  case qcril::interfaces::CallFailCause::MISC:
    return V1_3::CallFailCause::CALL_FAIL_MISC;
  case qcril::interfaces::CallFailCause::ANSWERED_ELSEWHERE:
    return V1_3::CallFailCause::CALL_FAIL_ANSWERED_ELSEWHERE;
  case qcril::interfaces::CallFailCause::PULL_OUT_OF_SYNC:
    return V1_3::CallFailCause::CALL_FAIL_PULL_OUT_OF_SYNC;
  case qcril::interfaces::CallFailCause::CAUSE_CALL_PULLED:
    return V1_3::CallFailCause::CALL_FAIL_CAUSE_CALL_PULLED;
  case qcril::interfaces::CallFailCause::ACCESS_CLASS_BLOCKED:
    return V1_3::CallFailCause::CALL_FAIL_ACCESS_CLASS_BLOCKED;
  case qcril::interfaces::CallFailCause::SIP_REDIRECTED:
    return V1_3::CallFailCause::CALL_FAIL_SIP_REDIRECTED;
  case qcril::interfaces::CallFailCause::SIP_BAD_REQUEST:
    return V1_3::CallFailCause::CALL_FAIL_SIP_BAD_REQUEST;
  case qcril::interfaces::CallFailCause::SIP_FORBIDDEN:
    return V1_3::CallFailCause::CALL_FAIL_SIP_FORBIDDEN;
  case qcril::interfaces::CallFailCause::SIP_NOT_FOUND:
    return V1_3::CallFailCause::CALL_FAIL_SIP_NOT_FOUND;
  case qcril::interfaces::CallFailCause::SIP_NOT_SUPPORTED:
    return V1_3::CallFailCause::CALL_FAIL_SIP_NOT_SUPPORTED;
  case qcril::interfaces::CallFailCause::SIP_REQUEST_TIMEOUT:
    return V1_3::CallFailCause::CALL_FAIL_SIP_REQUEST_TIMEOUT;
  case qcril::interfaces::CallFailCause::SIP_TEMPORARILY_UNAVAILABLE:
    return V1_3::CallFailCause::CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE;
  case qcril::interfaces::CallFailCause::SIP_BAD_ADDRESS:
    return V1_3::CallFailCause::CALL_FAIL_SIP_BAD_ADDRESS;
  case qcril::interfaces::CallFailCause::SIP_BUSY:
    return V1_3::CallFailCause::CALL_FAIL_SIP_BUSY;
  case qcril::interfaces::CallFailCause::SIP_REQUEST_CANCELLED:
    return V1_3::CallFailCause::CALL_FAIL_SIP_REQUEST_CANCELLED;
  case qcril::interfaces::CallFailCause::SIP_NOT_ACCEPTABLE:
    return V1_3::CallFailCause::CALL_FAIL_SIP_NOT_ACCEPTABLE;
  case qcril::interfaces::CallFailCause::SIP_NOT_REACHABLE:
    return V1_3::CallFailCause::CALL_FAIL_SIP_NOT_REACHABLE;
  case qcril::interfaces::CallFailCause::SIP_SERVER_INTERNAL_ERROR:
    return V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_INTERNAL_ERROR;
  case qcril::interfaces::CallFailCause::SIP_SERVER_NOT_IMPLEMENTED:
    return V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED;
  case qcril::interfaces::CallFailCause::SIP_SERVER_BAD_GATEWAY:
    return V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_BAD_GATEWAY;
  case qcril::interfaces::CallFailCause::SIP_SERVICE_UNAVAILABLE:
    return V1_3::CallFailCause::CALL_FAIL_SIP_SERVICE_UNAVAILABLE;
  case qcril::interfaces::CallFailCause::SIP_SERVER_TIMEOUT:
    return V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_TIMEOUT;
  case qcril::interfaces::CallFailCause::SIP_SERVER_VERSION_UNSUPPORTED:
    return V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED;
  case qcril::interfaces::CallFailCause::SIP_SERVER_MESSAGE_TOOLARGE:
    return V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE;
  case qcril::interfaces::CallFailCause::SIP_SERVER_PRECONDITION_FAILURE:
    return V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE;
  case qcril::interfaces::CallFailCause::SIP_USER_REJECTED:
    return V1_3::CallFailCause::CALL_FAIL_SIP_USER_REJECTED;
  case qcril::interfaces::CallFailCause::SIP_GLOBAL_ERROR:
    return V1_3::CallFailCause::CALL_FAIL_SIP_GLOBAL_ERROR;
  case qcril::interfaces::CallFailCause::MEDIA_INIT_FAILED:
    return V1_3::CallFailCause::CALL_FAIL_MEDIA_INIT_FAILED;
  case qcril::interfaces::CallFailCause::MEDIA_NO_DATA:
    return V1_3::CallFailCause::CALL_FAIL_MEDIA_NO_DATA;
  case qcril::interfaces::CallFailCause::MEDIA_NOT_ACCEPTABLE:
    return V1_3::CallFailCause::CALL_FAIL_MEDIA_NOT_ACCEPTABLE;
  case qcril::interfaces::CallFailCause::MEDIA_UNSPECIFIED_ERROR:
    return V1_3::CallFailCause::CALL_FAIL_MEDIA_UNSPECIFIED_ERROR;
  case qcril::interfaces::CallFailCause::HOLD_RESUME_FAILED:
    return V1_3::CallFailCause::CALL_FAIL_HOLD_RESUME_FAILED;
  case qcril::interfaces::CallFailCause::HOLD_RESUME_CANCELED:
    return V1_3::CallFailCause::CALL_FAIL_HOLD_RESUME_CANCELED;
  case qcril::interfaces::CallFailCause::HOLD_REINVITE_COLLISION:
    return V1_3::CallFailCause::CALL_FAIL_HOLD_REINVITE_COLLISION;
  case qcril::interfaces::CallFailCause::DIAL_MODIFIED_TO_DIAL_VIDEO:
    return V1_3::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO;
  case qcril::interfaces::CallFailCause::DIAL_VIDEO_MODIFIED_TO_DIAL:
    return V1_3::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL;
  case qcril::interfaces::CallFailCause::DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO:
    return V1_3::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO;
  case qcril::interfaces::CallFailCause::DIAL_VIDEO_MODIFIED_TO_SS:
    return V1_3::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS;
  case qcril::interfaces::CallFailCause::DIAL_VIDEO_MODIFIED_TO_USSD:
    return V1_3::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD;
  case qcril::interfaces::CallFailCause::ERROR_UNSPECIFIED:
    return V1_3::CallFailCause::CALL_FAIL_ERROR_UNSPECIFIED;
  case qcril::interfaces::CallFailCause::SIP_ALTERNATE_EMERGENCY_CALL:
    return V1_3::CallFailCause::CALL_FAIL_SIP_ALTERNATE_EMERGENCY_CALL;
  case qcril::interfaces::CallFailCause::RADIO_OFF:
    return V1_3::CallFailCause::CALL_FAIL_RADIO_OFF;
  case qcril::interfaces::CallFailCause::OUT_OF_SERVICE:
    return V1_3::CallFailCause::CALL_FAIL_OUT_OF_SERVICE;
  case qcril::interfaces::CallFailCause::NO_VALID_SIM:
    return V1_3::CallFailCause::CALL_FAIL_NO_VALID_SIM;
  case qcril::interfaces::CallFailCause::RADIO_INTERNAL_ERROR:
    return V1_3::CallFailCause::CALL_FAIL_RADIO_INTERNAL_ERROR;
  case qcril::interfaces::CallFailCause::NETWORK_RESP_TIMEOUT:
    return V1_3::CallFailCause::CALL_FAIL_NETWORK_RESP_TIMEOUT;
  case qcril::interfaces::CallFailCause::NETWORK_REJECT:
    return V1_3::CallFailCause::CALL_FAIL_NETWORK_REJECT;
  case qcril::interfaces::CallFailCause::RADIO_ACCESS_FAILURE:
    return V1_3::CallFailCause::CALL_FAIL_RADIO_ACCESS_FAILURE;
  case qcril::interfaces::CallFailCause::RADIO_LINK_FAILURE:
    return V1_3::CallFailCause::CALL_FAIL_RADIO_LINK_FAILURE;
  case qcril::interfaces::CallFailCause::RADIO_LINK_LOST:
    return V1_3::CallFailCause::CALL_FAIL_RADIO_LINK_LOST;
  case qcril::interfaces::CallFailCause::RADIO_UPLINK_FAILURE:
    return V1_3::CallFailCause::CALL_FAIL_RADIO_UPLINK_FAILURE;
  case qcril::interfaces::CallFailCause::RADIO_SETUP_FAILURE:
    return V1_3::CallFailCause::CALL_FAIL_RADIO_SETUP_FAILURE;
  case qcril::interfaces::CallFailCause::RADIO_RELEASE_NORMAL:
    return V1_3::CallFailCause::CALL_FAIL_RADIO_RELEASE_NORMAL;
  case qcril::interfaces::CallFailCause::RADIO_RELEASE_ABNORMAL:
    return V1_3::CallFailCause::CALL_FAIL_RADIO_RELEASE_ABNORMAL;
  case qcril::interfaces::CallFailCause::NETWORK_DETACH:
    return V1_3::CallFailCause::CALL_FAIL_NETWORK_DETACH;
  case qcril::interfaces::CallFailCause::OEM_CAUSE_1:
    return V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_1;
  case qcril::interfaces::CallFailCause::OEM_CAUSE_2:
    return V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_2;
  case qcril::interfaces::CallFailCause::OEM_CAUSE_3:
    return V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_3;
  case qcril::interfaces::CallFailCause::OEM_CAUSE_4:
    return V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_4;
  case qcril::interfaces::CallFailCause::OEM_CAUSE_5:
    return V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_5;
  case qcril::interfaces::CallFailCause::OEM_CAUSE_6:
    return V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_6;
  case qcril::interfaces::CallFailCause::OEM_CAUSE_7:
    return V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_7;
  case qcril::interfaces::CallFailCause::OEM_CAUSE_8:
    return V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_8;
  case qcril::interfaces::CallFailCause::OEM_CAUSE_9:
    return V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_9;
  case qcril::interfaces::CallFailCause::OEM_CAUSE_10:
    return V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_10;
  case qcril::interfaces::CallFailCause::OEM_CAUSE_11:
    return V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_11;
  case qcril::interfaces::CallFailCause::OEM_CAUSE_12:
    return V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_12;
  case qcril::interfaces::CallFailCause::OEM_CAUSE_13:
    return V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_13;
  case qcril::interfaces::CallFailCause::OEM_CAUSE_14:
    return V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_14;
  case qcril::interfaces::CallFailCause::OEM_CAUSE_15:
    return V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_15;
  case qcril::interfaces::CallFailCause::NO_CSFB_IN_CS_ROAM:
    return V1_3::CallFailCause::CALL_FAIL_NO_CSFB_IN_CS_ROAM;
  case qcril::interfaces::CallFailCause::SRV_NOT_REGISTERED:
    return V1_3::CallFailCause::CALL_FAIL_SRV_NOT_REGISTERED;
  case qcril::interfaces::CallFailCause::CALL_TYPE_NOT_ALLOWED:
    return V1_3::CallFailCause::CALL_FAIL_CALL_TYPE_NOT_ALLOWED;
  case qcril::interfaces::CallFailCause::EMRG_CALL_ONGOING:
    return V1_3::CallFailCause::CALL_FAIL_EMRG_CALL_ONGOING;
  case qcril::interfaces::CallFailCause::CALL_SETUP_ONGOING:
    return V1_3::CallFailCause::CALL_FAIL_CALL_SETUP_ONGOING;
  case qcril::interfaces::CallFailCause::MAX_CALL_LIMIT_REACHED:
    return V1_3::CallFailCause::CALL_FAIL_MAX_CALL_LIMIT_REACHED;
  case qcril::interfaces::CallFailCause::UNSUPPORTED_SIP_HDRS:
    return V1_3::CallFailCause::CALL_FAIL_UNSUPPORTED_SIP_HDRS;
  case qcril::interfaces::CallFailCause::CALL_TRANSFER_ONGOING:
    return V1_3::CallFailCause::CALL_FAIL_CALL_TRANSFER_ONGOING;
  case qcril::interfaces::CallFailCause::PRACK_TIMEOUT:
    return V1_3::CallFailCause::CALL_FAIL_PRACK_TIMEOUT;
  case qcril::interfaces::CallFailCause::QOS_FAILURE:
    return V1_3::CallFailCause::CALL_FAIL_QOS_FAILURE;
  case qcril::interfaces::CallFailCause::ONGOING_HANDOVER:
    return V1_3::CallFailCause::CALL_FAIL_ONGOING_HANDOVER;
  case qcril::interfaces::CallFailCause::VT_WITH_TTY_NOT_ALLOWED:
    return V1_3::CallFailCause::CALL_FAIL_VT_WITH_TTY_NOT_ALLOWED;
  case qcril::interfaces::CallFailCause::CALL_UPGRADE_ONGOING:
    return V1_3::CallFailCause::CALL_FAIL_CALL_UPGRADE_ONGOING;
  case qcril::interfaces::CallFailCause::CONFERENCE_WITH_TTY_NOT_ALLOWED:
    return V1_3::CallFailCause::CALL_FAIL_CONFERENCE_WITH_TTY_NOT_ALLOWED;
  case qcril::interfaces::CallFailCause::CALL_CONFERENCE_ONGOING:
    return V1_3::CallFailCause::CALL_FAIL_CALL_CONFERENCE_ONGOING;
  case qcril::interfaces::CallFailCause::VT_WITH_AVPF_NOT_ALLOWED:
    return V1_3::CallFailCause::CALL_FAIL_VT_WITH_AVPF_NOT_ALLOWED;
  case qcril::interfaces::CallFailCause::ENCRYPTION_CALL_ONGOING:
    return V1_3::CallFailCause::CALL_FAIL_ENCRYPTION_CALL_ONGOING;
  case qcril::interfaces::CallFailCause::CALL_ONGOING_CW_DISABLED:
    return V1_3::CallFailCause::CALL_FAIL_CALL_ONGOING_CW_DISABLED;
  case qcril::interfaces::CallFailCause::CALL_ON_OTHER_SUB:
    return V1_3::CallFailCause::CALL_FAIL_CALL_ON_OTHER_SUB;
  case qcril::interfaces::CallFailCause::ONE_X_COLLISION:
    return V1_3::CallFailCause::CALL_FAIL_1X_COLLISION;
  case qcril::interfaces::CallFailCause::UI_NOT_READY:
    return V1_3::CallFailCause::CALL_FAIL_UI_NOT_READY;
  case qcril::interfaces::CallFailCause::CS_CALL_ONGOING:
    return V1_3::CallFailCause::CALL_FAIL_CS_CALL_ONGOING;
  case qcril::interfaces::CallFailCause::SIP_USER_MARKED_UNWANTED:
    return V1_3::CallFailCause::CALL_FAIL_SIP_USER_MARKED_UNWANTED;
  case qcril::interfaces::CallFailCause::UNKNOWN:
    return V1_3::CallFailCause::CALL_FAIL_INVALID;
  default:
    return static_cast<V1_3::CallFailCause>(in);
  }
}

qcril::interfaces::CallFailCause convertCallFailCause(const V1_3::CallFailCause &in) {
  switch (in) {
  case V1_3::CallFailCause::CALL_FAIL_UNOBTAINABLE_NUMBER:
    return qcril::interfaces::CallFailCause::UNOBTAINABLE_NUMBER;
  case V1_3::CallFailCause::CALL_FAIL_NORMAL:
    return qcril::interfaces::CallFailCause::NORMAL;
  case V1_3::CallFailCause::CALL_FAIL_BUSY:
    return qcril::interfaces::CallFailCause::BUSY;
  case V1_3::CallFailCause::CALL_FAIL_NORMAL_UNSPECIFIED:
    return qcril::interfaces::CallFailCause::NORMAL_UNSPECIFIED;
  case V1_3::CallFailCause::CALL_FAIL_CONGESTION:
    return qcril::interfaces::CallFailCause::CONGESTION;
  case V1_3::CallFailCause::CALL_FAIL_INCOMPATIBILITY_DESTINATION:
    return qcril::interfaces::CallFailCause::INCOMPATIBILITY_DESTINATION;
  case V1_3::CallFailCause::CALL_FAIL_CALL_BARRED:
    return qcril::interfaces::CallFailCause::CALL_BARRED;
  case V1_3::CallFailCause::CALL_FAIL_FDN_BLOCKED:
    return qcril::interfaces::CallFailCause::FDN_BLOCKED;
  case V1_3::CallFailCause::CALL_FAIL_IMEI_NOT_ACCEPTED:
    return qcril::interfaces::CallFailCause::IMEI_NOT_ACCEPTED;
  case V1_3::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_USSD:
    return qcril::interfaces::CallFailCause::DIAL_MODIFIED_TO_USSD;
  case V1_3::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_SS:
    return qcril::interfaces::CallFailCause::DIAL_MODIFIED_TO_SS;
  case V1_3::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL:
    return qcril::interfaces::CallFailCause::DIAL_MODIFIED_TO_DIAL;
  case V1_3::CallFailCause::CALL_FAIL_EMERGENCY_TEMP_FAILURE:
    return qcril::interfaces::CallFailCause::EMERGENCY_TEMP_FAILURE;
  case V1_3::CallFailCause::CALL_FAIL_EMERGENCY_PERM_FAILURE:
    return qcril::interfaces::CallFailCause::EMERGENCY_PERM_FAILURE;
  case V1_3::CallFailCause::CALL_FAIL_HO_NOT_FEASIBLE:
    return qcril::interfaces::CallFailCause::HO_NOT_FEASIBLE;
  case V1_3::CallFailCause::CALL_FAIL_USER_BUSY:
    return qcril::interfaces::CallFailCause::USER_BUSY;
  case V1_3::CallFailCause::CALL_FAIL_USER_REJECT:
    return qcril::interfaces::CallFailCause::USER_REJECT;
  case V1_3::CallFailCause::CALL_FAIL_LOW_BATTERY:
    return qcril::interfaces::CallFailCause::LOW_BATTERY;
  case V1_3::CallFailCause::CALL_FAIL_BLACKLISTED_CALL_ID:
    return qcril::interfaces::CallFailCause::BLACKLISTED_CALL_ID;
  case V1_3::CallFailCause::CALL_FAIL_CS_RETRY_REQUIRED:
    return qcril::interfaces::CallFailCause::CS_RETRY_REQUIRED;
  case V1_3::CallFailCause::CALL_FAIL_NETWORK_UNAVAILABLE:
    return qcril::interfaces::CallFailCause::NETWORK_UNAVAILABLE;
  case V1_3::CallFailCause::CALL_FAIL_FEATURE_UNAVAILABLE:
    return qcril::interfaces::CallFailCause::FEATURE_UNAVAILABLE;
  case V1_3::CallFailCause::CALL_FAIL_SIP_ERROR:
    return qcril::interfaces::CallFailCause::SIP_ERROR;
  case V1_3::CallFailCause::CALL_FAIL_MISC:
    return qcril::interfaces::CallFailCause::MISC;
  case V1_3::CallFailCause::CALL_FAIL_ANSWERED_ELSEWHERE:
    return qcril::interfaces::CallFailCause::ANSWERED_ELSEWHERE;
  case V1_3::CallFailCause::CALL_FAIL_PULL_OUT_OF_SYNC:
    return qcril::interfaces::CallFailCause::PULL_OUT_OF_SYNC;
  case V1_3::CallFailCause::CALL_FAIL_CAUSE_CALL_PULLED:
    return qcril::interfaces::CallFailCause::CAUSE_CALL_PULLED;
  case V1_3::CallFailCause::CALL_FAIL_ACCESS_CLASS_BLOCKED:
    return qcril::interfaces::CallFailCause::ACCESS_CLASS_BLOCKED;
  case V1_3::CallFailCause::CALL_FAIL_SIP_REDIRECTED:
    return qcril::interfaces::CallFailCause::SIP_REDIRECTED;
  case V1_3::CallFailCause::CALL_FAIL_SIP_BAD_REQUEST:
    return qcril::interfaces::CallFailCause::SIP_BAD_REQUEST;
  case V1_3::CallFailCause::CALL_FAIL_SIP_FORBIDDEN:
    return qcril::interfaces::CallFailCause::SIP_FORBIDDEN;
  case V1_3::CallFailCause::CALL_FAIL_SIP_NOT_FOUND:
    return qcril::interfaces::CallFailCause::SIP_NOT_FOUND;
  case V1_3::CallFailCause::CALL_FAIL_SIP_NOT_SUPPORTED:
    return qcril::interfaces::CallFailCause::SIP_NOT_SUPPORTED;
  case V1_3::CallFailCause::CALL_FAIL_SIP_REQUEST_TIMEOUT:
    return qcril::interfaces::CallFailCause::SIP_REQUEST_TIMEOUT;
  case V1_3::CallFailCause::CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE:
    return qcril::interfaces::CallFailCause::SIP_TEMPORARILY_UNAVAILABLE;
  case V1_3::CallFailCause::CALL_FAIL_SIP_BAD_ADDRESS:
    return qcril::interfaces::CallFailCause::SIP_BAD_ADDRESS;
  case V1_3::CallFailCause::CALL_FAIL_SIP_BUSY:
    return qcril::interfaces::CallFailCause::SIP_BUSY;
  case V1_3::CallFailCause::CALL_FAIL_SIP_REQUEST_CANCELLED:
    return qcril::interfaces::CallFailCause::SIP_REQUEST_CANCELLED;
  case V1_3::CallFailCause::CALL_FAIL_SIP_NOT_ACCEPTABLE:
    return qcril::interfaces::CallFailCause::SIP_NOT_ACCEPTABLE;
  case V1_3::CallFailCause::CALL_FAIL_SIP_NOT_REACHABLE:
    return qcril::interfaces::CallFailCause::SIP_NOT_REACHABLE;
  case V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_INTERNAL_ERROR:
    return qcril::interfaces::CallFailCause::SIP_SERVER_INTERNAL_ERROR;
  case V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED:
    return qcril::interfaces::CallFailCause::SIP_SERVER_NOT_IMPLEMENTED;
  case V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_BAD_GATEWAY:
    return qcril::interfaces::CallFailCause::SIP_SERVER_BAD_GATEWAY;
  case V1_3::CallFailCause::CALL_FAIL_SIP_SERVICE_UNAVAILABLE:
    return qcril::interfaces::CallFailCause::SIP_SERVICE_UNAVAILABLE;
  case V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_TIMEOUT:
    return qcril::interfaces::CallFailCause::SIP_SERVER_TIMEOUT;
  case V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED:
    return qcril::interfaces::CallFailCause::SIP_SERVER_VERSION_UNSUPPORTED;
  case V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE:
    return qcril::interfaces::CallFailCause::SIP_SERVER_MESSAGE_TOOLARGE;
  case V1_3::CallFailCause::CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE:
    return qcril::interfaces::CallFailCause::SIP_SERVER_PRECONDITION_FAILURE;
  case V1_3::CallFailCause::CALL_FAIL_SIP_USER_REJECTED:
    return qcril::interfaces::CallFailCause::SIP_USER_REJECTED;
  case V1_3::CallFailCause::CALL_FAIL_SIP_GLOBAL_ERROR:
    return qcril::interfaces::CallFailCause::SIP_GLOBAL_ERROR;
  case V1_3::CallFailCause::CALL_FAIL_MEDIA_INIT_FAILED:
    return qcril::interfaces::CallFailCause::MEDIA_INIT_FAILED;
  case V1_3::CallFailCause::CALL_FAIL_MEDIA_NO_DATA:
    return qcril::interfaces::CallFailCause::MEDIA_NO_DATA;
  case V1_3::CallFailCause::CALL_FAIL_MEDIA_NOT_ACCEPTABLE:
    return qcril::interfaces::CallFailCause::MEDIA_NOT_ACCEPTABLE;
  case V1_3::CallFailCause::CALL_FAIL_MEDIA_UNSPECIFIED_ERROR:
    return qcril::interfaces::CallFailCause::MEDIA_UNSPECIFIED_ERROR;
  case V1_3::CallFailCause::CALL_FAIL_HOLD_RESUME_FAILED:
    return qcril::interfaces::CallFailCause::HOLD_RESUME_FAILED;
  case V1_3::CallFailCause::CALL_FAIL_HOLD_RESUME_CANCELED:
    return qcril::interfaces::CallFailCause::HOLD_RESUME_CANCELED;
  case V1_3::CallFailCause::CALL_FAIL_HOLD_REINVITE_COLLISION:
    return qcril::interfaces::CallFailCause::HOLD_REINVITE_COLLISION;
  case V1_3::CallFailCause::CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO:
    return qcril::interfaces::CallFailCause::DIAL_MODIFIED_TO_DIAL_VIDEO;
  case V1_3::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL:
    return qcril::interfaces::CallFailCause::DIAL_VIDEO_MODIFIED_TO_DIAL;
  case V1_3::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO:
    return qcril::interfaces::CallFailCause::DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO;
  case V1_3::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS:
    return qcril::interfaces::CallFailCause::DIAL_VIDEO_MODIFIED_TO_SS;
  case V1_3::CallFailCause::CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD:
    return qcril::interfaces::CallFailCause::DIAL_VIDEO_MODIFIED_TO_USSD;
  case V1_3::CallFailCause::CALL_FAIL_ERROR_UNSPECIFIED:
    return qcril::interfaces::CallFailCause::ERROR_UNSPECIFIED;
  case V1_3::CallFailCause::CALL_FAIL_SIP_ALTERNATE_EMERGENCY_CALL:
    return qcril::interfaces::CallFailCause::SIP_ALTERNATE_EMERGENCY_CALL;
  case V1_3::CallFailCause::CALL_FAIL_RADIO_OFF:
    return qcril::interfaces::CallFailCause::RADIO_OFF;
  case V1_3::CallFailCause::CALL_FAIL_OUT_OF_SERVICE:
    return qcril::interfaces::CallFailCause::OUT_OF_SERVICE;
  case V1_3::CallFailCause::CALL_FAIL_NO_VALID_SIM:
    return qcril::interfaces::CallFailCause::NO_VALID_SIM;
  case V1_3::CallFailCause::CALL_FAIL_RADIO_INTERNAL_ERROR:
    return qcril::interfaces::CallFailCause::RADIO_INTERNAL_ERROR;
  case V1_3::CallFailCause::CALL_FAIL_NETWORK_RESP_TIMEOUT:
    return qcril::interfaces::CallFailCause::NETWORK_RESP_TIMEOUT;
  case V1_3::CallFailCause::CALL_FAIL_NETWORK_REJECT:
    return qcril::interfaces::CallFailCause::NETWORK_REJECT;
  case V1_3::CallFailCause::CALL_FAIL_RADIO_ACCESS_FAILURE:
    return qcril::interfaces::CallFailCause::RADIO_ACCESS_FAILURE;
  case V1_3::CallFailCause::CALL_FAIL_RADIO_LINK_FAILURE:
    return qcril::interfaces::CallFailCause::RADIO_LINK_FAILURE;
  case V1_3::CallFailCause::CALL_FAIL_RADIO_LINK_LOST:
    return qcril::interfaces::CallFailCause::RADIO_LINK_LOST;
  case V1_3::CallFailCause::CALL_FAIL_RADIO_UPLINK_FAILURE:
    return qcril::interfaces::CallFailCause::RADIO_UPLINK_FAILURE;
  case V1_3::CallFailCause::CALL_FAIL_RADIO_SETUP_FAILURE:
    return qcril::interfaces::CallFailCause::RADIO_SETUP_FAILURE;
  case V1_3::CallFailCause::CALL_FAIL_RADIO_RELEASE_NORMAL:
    return qcril::interfaces::CallFailCause::RADIO_RELEASE_NORMAL;
  case V1_3::CallFailCause::CALL_FAIL_RADIO_RELEASE_ABNORMAL:
    return qcril::interfaces::CallFailCause::RADIO_RELEASE_ABNORMAL;
  case V1_3::CallFailCause::CALL_FAIL_NETWORK_DETACH:
    return qcril::interfaces::CallFailCause::NETWORK_DETACH;
  case V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_1:
    return qcril::interfaces::CallFailCause::OEM_CAUSE_1;
  case V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_2:
    return qcril::interfaces::CallFailCause::OEM_CAUSE_2;
  case V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_3:
    return qcril::interfaces::CallFailCause::OEM_CAUSE_3;
  case V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_4:
    return qcril::interfaces::CallFailCause::OEM_CAUSE_4;
  case V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_5:
    return qcril::interfaces::CallFailCause::OEM_CAUSE_5;
  case V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_6:
    return qcril::interfaces::CallFailCause::OEM_CAUSE_6;
  case V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_7:
    return qcril::interfaces::CallFailCause::OEM_CAUSE_7;
  case V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_8:
    return qcril::interfaces::CallFailCause::OEM_CAUSE_8;
  case V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_9:
    return qcril::interfaces::CallFailCause::OEM_CAUSE_9;
  case V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_10:
    return qcril::interfaces::CallFailCause::OEM_CAUSE_10;
  case V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_11:
    return qcril::interfaces::CallFailCause::OEM_CAUSE_11;
  case V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_12:
    return qcril::interfaces::CallFailCause::OEM_CAUSE_12;
  case V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_13:
    return qcril::interfaces::CallFailCause::OEM_CAUSE_13;
  case V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_14:
    return qcril::interfaces::CallFailCause::OEM_CAUSE_14;
  case V1_3::CallFailCause::CALL_FAIL_OEM_CAUSE_15:
    return qcril::interfaces::CallFailCause::OEM_CAUSE_15;
  case V1_3::CallFailCause::CALL_FAIL_NO_CSFB_IN_CS_ROAM:
    return qcril::interfaces::CallFailCause::NO_CSFB_IN_CS_ROAM;
  case V1_3::CallFailCause::CALL_FAIL_SRV_NOT_REGISTERED:
    return qcril::interfaces::CallFailCause::SRV_NOT_REGISTERED;
  case V1_3::CallFailCause::CALL_FAIL_CALL_TYPE_NOT_ALLOWED:
    return qcril::interfaces::CallFailCause::CALL_TYPE_NOT_ALLOWED;
  case V1_3::CallFailCause::CALL_FAIL_EMRG_CALL_ONGOING:
    return qcril::interfaces::CallFailCause::EMRG_CALL_ONGOING;
  case V1_3::CallFailCause::CALL_FAIL_CALL_SETUP_ONGOING:
    return qcril::interfaces::CallFailCause::CALL_SETUP_ONGOING;
  case V1_3::CallFailCause::CALL_FAIL_MAX_CALL_LIMIT_REACHED:
    return qcril::interfaces::CallFailCause::MAX_CALL_LIMIT_REACHED;
  case V1_3::CallFailCause::CALL_FAIL_UNSUPPORTED_SIP_HDRS:
    return qcril::interfaces::CallFailCause::UNSUPPORTED_SIP_HDRS;
  case V1_3::CallFailCause::CALL_FAIL_CALL_TRANSFER_ONGOING:
    return qcril::interfaces::CallFailCause::CALL_TRANSFER_ONGOING;
  case V1_3::CallFailCause::CALL_FAIL_PRACK_TIMEOUT:
    return qcril::interfaces::CallFailCause::PRACK_TIMEOUT;
  case V1_3::CallFailCause::CALL_FAIL_QOS_FAILURE:
    return qcril::interfaces::CallFailCause::QOS_FAILURE;
  case V1_3::CallFailCause::CALL_FAIL_ONGOING_HANDOVER:
    return qcril::interfaces::CallFailCause::ONGOING_HANDOVER;
  case V1_3::CallFailCause::CALL_FAIL_VT_WITH_TTY_NOT_ALLOWED:
    return qcril::interfaces::CallFailCause::VT_WITH_TTY_NOT_ALLOWED;
  case V1_3::CallFailCause::CALL_FAIL_CALL_UPGRADE_ONGOING:
    return qcril::interfaces::CallFailCause::CALL_UPGRADE_ONGOING;
  case V1_3::CallFailCause::CALL_FAIL_CONFERENCE_WITH_TTY_NOT_ALLOWED:
    return qcril::interfaces::CallFailCause::CONFERENCE_WITH_TTY_NOT_ALLOWED;
  case V1_3::CallFailCause::CALL_FAIL_CALL_CONFERENCE_ONGOING:
    return qcril::interfaces::CallFailCause::CALL_CONFERENCE_ONGOING;
  case V1_3::CallFailCause::CALL_FAIL_VT_WITH_AVPF_NOT_ALLOWED:
    return qcril::interfaces::CallFailCause::VT_WITH_AVPF_NOT_ALLOWED;
  case V1_3::CallFailCause::CALL_FAIL_ENCRYPTION_CALL_ONGOING:
    return qcril::interfaces::CallFailCause::ENCRYPTION_CALL_ONGOING;
  case V1_3::CallFailCause::CALL_FAIL_CALL_ONGOING_CW_DISABLED:
    return qcril::interfaces::CallFailCause::CALL_ONGOING_CW_DISABLED;
  case V1_3::CallFailCause::CALL_FAIL_CALL_ON_OTHER_SUB:
    return qcril::interfaces::CallFailCause::CALL_ON_OTHER_SUB;
  case V1_3::CallFailCause::CALL_FAIL_1X_COLLISION:
    return qcril::interfaces::CallFailCause::ONE_X_COLLISION;
  case V1_3::CallFailCause::CALL_FAIL_UI_NOT_READY:
    return qcril::interfaces::CallFailCause::UI_NOT_READY;
  case V1_3::CallFailCause::CALL_FAIL_CS_CALL_ONGOING:
    return qcril::interfaces::CallFailCause::CS_CALL_ONGOING;
  case V1_3::CallFailCause::CALL_FAIL_SIP_USER_MARKED_UNWANTED:
    return qcril::interfaces::CallFailCause::SIP_USER_MARKED_UNWANTED;
  case V1_3::CallFailCause::CALL_FAIL_INVALID:
  default:
    return qcril::interfaces::CallFailCause::UNKNOWN;
  }
}

qcril::interfaces::CallModifyFailCause convertCallModifyFailCause(const CallModifyFailCause &in) {
  switch (in) {
  case CallModifyFailCause::E_SUCCESS:
    return qcril::interfaces::CallModifyFailCause::SUCCESS;
  case CallModifyFailCause::E_RADIO_NOT_AVAILABLE:
    return qcril::interfaces::CallModifyFailCause::RADIO_NOT_AVAILABLE;
  case CallModifyFailCause::E_GENERIC_FAILURE:
    return qcril::interfaces::CallModifyFailCause::GENERIC_FAILURE;
  case CallModifyFailCause::E_REQUEST_NOT_SUPPORTED:
    return qcril::interfaces::CallModifyFailCause::REQUEST_NOT_SUPPORTED;
  case CallModifyFailCause::E_CANCELLED:
    return qcril::interfaces::CallModifyFailCause::CANCELLED;
  case CallModifyFailCause::E_UNUSED:
    return qcril::interfaces::CallModifyFailCause::UNUSED;
  case CallModifyFailCause::E_INVALID_PARAMETER:
    return qcril::interfaces::CallModifyFailCause::INVALID_PARAMETER;
  case CallModifyFailCause::E_REJECTED_BY_REMOTE:
    return qcril::interfaces::CallModifyFailCause::REJECTED_BY_REMOTE;
  case CallModifyFailCause::E_IMS_DEREGISTERED:
    return qcril::interfaces::CallModifyFailCause::IMS_DEREGISTERED;
  case CallModifyFailCause::E_NETWORK_NOT_SUPPORTED:
    return qcril::interfaces::CallModifyFailCause::NETWORK_NOT_SUPPORTED;
  case CallModifyFailCause::E_HOLD_RESUME_FAILED:
    return qcril::interfaces::CallModifyFailCause::HOLD_RESUME_FAILED;
  case CallModifyFailCause::E_HOLD_RESUME_CANCELED:
    return qcril::interfaces::CallModifyFailCause::HOLD_RESUME_CANCELED;
  case CallModifyFailCause::E_REINVITE_COLLISION:
    return qcril::interfaces::CallModifyFailCause::REINVITE_COLLISION;
  case CallModifyFailCause::E_INVALID:
  default:
    return qcril::interfaces::CallModifyFailCause::UNKNOWN;
  }
}

CallModifyFailCause convertCallModifyFailCause(const qcril::interfaces::CallModifyFailCause &in) {
  switch (in) {
  case qcril::interfaces::CallModifyFailCause::SUCCESS:
    return CallModifyFailCause::E_SUCCESS;
  case qcril::interfaces::CallModifyFailCause::RADIO_NOT_AVAILABLE:
    return CallModifyFailCause::E_RADIO_NOT_AVAILABLE;
  case qcril::interfaces::CallModifyFailCause::GENERIC_FAILURE:
    return CallModifyFailCause::E_GENERIC_FAILURE;
  case qcril::interfaces::CallModifyFailCause::REQUEST_NOT_SUPPORTED:
    return CallModifyFailCause::E_REQUEST_NOT_SUPPORTED;
  case qcril::interfaces::CallModifyFailCause::CANCELLED:
    return CallModifyFailCause::E_CANCELLED;
  case qcril::interfaces::CallModifyFailCause::UNUSED:
    return CallModifyFailCause::E_UNUSED;
  case qcril::interfaces::CallModifyFailCause::INVALID_PARAMETER:
    return CallModifyFailCause::E_INVALID_PARAMETER;
  case qcril::interfaces::CallModifyFailCause::REJECTED_BY_REMOTE:
    return CallModifyFailCause::E_REJECTED_BY_REMOTE;
  case qcril::interfaces::CallModifyFailCause::IMS_DEREGISTERED:
    return CallModifyFailCause::E_IMS_DEREGISTERED;
  case qcril::interfaces::CallModifyFailCause::NETWORK_NOT_SUPPORTED:
    return CallModifyFailCause::E_NETWORK_NOT_SUPPORTED;
  case qcril::interfaces::CallModifyFailCause::HOLD_RESUME_FAILED:
    return CallModifyFailCause::E_HOLD_RESUME_FAILED;
  case qcril::interfaces::CallModifyFailCause::HOLD_RESUME_CANCELED:
    return CallModifyFailCause::E_HOLD_RESUME_CANCELED;
  case qcril::interfaces::CallModifyFailCause::REINVITE_COLLISION:
    return CallModifyFailCause::E_REINVITE_COLLISION;
  case qcril::interfaces::CallModifyFailCause::UNKNOWN:
  default:
    return CallModifyFailCause::E_INVALID;
  }
}

ToneOperation convertToneOperation(const qcril::interfaces::RingBackToneOperation &in) {
  switch (in) {
    case qcril::interfaces::RingBackToneOperation::STOP:
      return ToneOperation::STOP;
    case qcril::interfaces::RingBackToneOperation::START:
      return ToneOperation::START;
    default:
      return ToneOperation::INVALID;
  }
}

HandoverType convertHandoverType(const qcril::interfaces::HandoverType &in) {
  switch (in) {
    case qcril::interfaces::HandoverType::START:
      return HandoverType::START;
    case qcril::interfaces::HandoverType::COMPLETE_SUCCESS:
      return HandoverType::COMPLETE_SUCCESS;
    case qcril::interfaces::HandoverType::COMPLETE_FAIL:
      return HandoverType::COMPLETE_FAIL;
    case qcril::interfaces::HandoverType::CANCEL:
      return HandoverType::CANCEL;
    case qcril::interfaces::HandoverType::NOT_TRIGGERED:
      return HandoverType::NOT_TRIGGERED;
    case qcril::interfaces::HandoverType::NOT_TRIGGERED_MOBILE_DATA_OFF:
      return HandoverType::NOT_TRIGGERED_MOBILE_DATA_OFF;
    default:
      return HandoverType::INVALID;
  }
}

ConferenceCallState convertConferenceCallState(const qcril::interfaces::ConferenceCallState &in) {
  switch (in) {
    case qcril::interfaces::ConferenceCallState::RINGING:
      return ConferenceCallState::RINGING;
    case qcril::interfaces::ConferenceCallState::FOREGROUND:
      return ConferenceCallState::FOREGROUND;
    case qcril::interfaces::ConferenceCallState::BACKGROUND:
      return ConferenceCallState::BACKGROUND;
    default:
      return ConferenceCallState::INVALID;
  }
}

ServiceClassStatus convertServiceClassStatus(const qcril::interfaces::ServiceClassStatus &in) {
  switch (in) {
    case qcril::interfaces::ServiceClassStatus::DISABLED:
      return ServiceClassStatus::DISABLED;
    case qcril::interfaces::ServiceClassStatus::ENABLED:
      return ServiceClassStatus::ENABLED;
    case qcril::interfaces::ServiceClassStatus::UNKNOWN:
    default:
      return ServiceClassStatus::INVALID;
  }
}

qcril::interfaces::ServiceClassStatus convertServiceClassStatus(const ServiceClassStatus &in) {
  switch (in) {
    case ServiceClassStatus::DISABLED:
      return qcril::interfaces::ServiceClassStatus::DISABLED;
    case ServiceClassStatus::ENABLED:
      return qcril::interfaces::ServiceClassStatus::ENABLED;
    case ServiceClassStatus::INVALID:
    default:
      return qcril::interfaces::ServiceClassStatus::UNKNOWN;
  }
}

V1_3::ServiceClassProvisionStatus convertServiceClassProvisionStatus(const qcril::interfaces::ServiceClassProvisionStatus &in) {
  switch (in) {
    case qcril::interfaces::ServiceClassProvisionStatus::NOT_PROVISIONED:
      return V1_3::ServiceClassProvisionStatus::NOT_PROVISIONED;
    case qcril::interfaces::ServiceClassProvisionStatus::PROVISIONED:
      return V1_3::ServiceClassProvisionStatus::PROVISIONED;
    case qcril::interfaces::ServiceClassProvisionStatus::UNKNOWN:
    default:
      return V1_3::ServiceClassProvisionStatus::INVALID;
  }
}

qcril::interfaces::ServiceClassProvisionStatus convertServiceClassProvisionStatus(const V1_3::ServiceClassProvisionStatus &in) {
  switch (in) {
    case V1_3::ServiceClassProvisionStatus::NOT_PROVISIONED:
      return qcril::interfaces::ServiceClassProvisionStatus::NOT_PROVISIONED;
    case V1_3::ServiceClassProvisionStatus::PROVISIONED:
      return qcril::interfaces::ServiceClassProvisionStatus::PROVISIONED;
    case V1_3::ServiceClassProvisionStatus::INVALID:
    default:
      return qcril::interfaces::ServiceClassProvisionStatus::UNKNOWN;
  }
}

ClipStatus convertClipStatus(const qcril::interfaces::ClipStatus &in) {
  switch (in) {
    case qcril::interfaces::ClipStatus::NOT_PROVISIONED:
      return ClipStatus::NOT_PROVISIONED;
    case qcril::interfaces::ClipStatus::PROVISIONED:
      return ClipStatus::PROVISIONED;
    case qcril::interfaces::ClipStatus::STATUS_UNKNOWN:
      return ClipStatus::STATUS_UNKNOWN;
    case qcril::interfaces::ClipStatus::UNKNOWN:
    default:
      return ClipStatus::INVALID;
  }
}

qcril::interfaces::ClipStatus convertClipStatus(const ClipStatus &in) {
  switch (in) {
    case ClipStatus::NOT_PROVISIONED:
      return qcril::interfaces::ClipStatus::NOT_PROVISIONED;
    case ClipStatus::PROVISIONED:
      return qcril::interfaces::ClipStatus::PROVISIONED;
    case ClipStatus::STATUS_UNKNOWN:
      return qcril::interfaces::ClipStatus::STATUS_UNKNOWN;
    case ClipStatus::INVALID:
    default:
      return qcril::interfaces::ClipStatus::UNKNOWN;
  }
}

FacilityType convertFacilityType(const qcril::interfaces::FacilityType &in) {
  switch (in) {
    case qcril::interfaces::FacilityType::CLIP:
      return FacilityType::FACILITY_CLIP;
    case qcril::interfaces::FacilityType::COLP:
      return FacilityType::FACILITY_COLP;
    case qcril::interfaces::FacilityType::BAOC:
      return FacilityType::FACILITY_BAOC;
    case qcril::interfaces::FacilityType::BAOIC:
      return FacilityType::FACILITY_BAOIC;
    case qcril::interfaces::FacilityType::BAOICxH:
      return FacilityType::FACILITY_BAOICxH;
    case qcril::interfaces::FacilityType::BAIC:
      return FacilityType::FACILITY_BAIC;
    case qcril::interfaces::FacilityType::BAICr:
      return FacilityType::FACILITY_BAICr;
    case qcril::interfaces::FacilityType::BA_ALL:
      return FacilityType::FACILITY_BA_ALL;
    case qcril::interfaces::FacilityType::BA_MO:
      return FacilityType::FACILITY_BA_MO;
    case qcril::interfaces::FacilityType::BA_MT:
      return FacilityType::FACILITY_BA_MT;
    case qcril::interfaces::FacilityType::BS_MT:
      return FacilityType::FACILITY_BS_MT;
    case qcril::interfaces::FacilityType::BAICa:
      return FacilityType::FACILITY_BAICa;
    case qcril::interfaces::FacilityType::UNKNOWN:
    default:
      return FacilityType::FACILITY_INVALID;
  }
}

qcril::interfaces::FacilityType convertFacilityType(const FacilityType &in) {
  switch (in) {
    case FacilityType::FACILITY_CLIP:
      return qcril::interfaces::FacilityType::CLIP;
    case FacilityType::FACILITY_COLP:
      return qcril::interfaces::FacilityType::COLP;
    case FacilityType::FACILITY_BAOC:
      return qcril::interfaces::FacilityType::BAOC;
    case FacilityType::FACILITY_BAOIC:
      return qcril::interfaces::FacilityType::BAOIC;
    case FacilityType::FACILITY_BAOICxH:
      return qcril::interfaces::FacilityType::BAOICxH;
    case FacilityType::FACILITY_BAIC:
      return qcril::interfaces::FacilityType::BAIC;
    case FacilityType::FACILITY_BAICr:
      return qcril::interfaces::FacilityType::BAICr;
    case FacilityType::FACILITY_BA_ALL:
      return qcril::interfaces::FacilityType::BA_ALL;
    case FacilityType::FACILITY_BA_MO:
      return qcril::interfaces::FacilityType::BA_MO;
    case FacilityType::FACILITY_BA_MT:
      return qcril::interfaces::FacilityType::BA_MT;
    case FacilityType::FACILITY_BS_MT:
      return qcril::interfaces::FacilityType::BS_MT;
    case FacilityType::FACILITY_BAICa:
      return qcril::interfaces::FacilityType::BAICa;
    case FacilityType::FACILITY_INVALID:
    default:
      return qcril::interfaces::FacilityType::UNKNOWN;
  }
}

EctType convertEctType(const qcril::interfaces::EctType &in) {
  switch (in) {
    case qcril::interfaces::EctType::BLIND_TRANSFER:
      return EctType::BLIND_TRANSFER;
    case qcril::interfaces::EctType::ASSURED_TRANSFER:
      return EctType::ASSURED_TRANSFER;
    case qcril::interfaces::EctType::CONSULTATIVE_TRANSFER:
      return EctType::CONSULTATIVE_TRANSFER;
    case qcril::interfaces::EctType::UNKNOWN:
    default:
      return EctType::INVALID;
  }
}
qcril::interfaces::EctType convertEctType(const EctType &in) {
  switch (in) {
    case EctType::BLIND_TRANSFER:
      return qcril::interfaces::EctType::BLIND_TRANSFER;
    case EctType::ASSURED_TRANSFER:
      return qcril::interfaces::EctType::ASSURED_TRANSFER;
    case EctType::CONSULTATIVE_TRANSFER:
      return qcril::interfaces::EctType::CONSULTATIVE_TRANSFER;
    case EctType::INVALID:
    default:
      return qcril::interfaces::EctType::UNKNOWN;
  }
}

TtyMode convertTtyMode(const qcril::interfaces::TtyMode &in) {
  switch (in) {
    case qcril::interfaces::TtyMode::MODE_OFF:
      return TtyMode::TTY_MODE_OFF;
    case qcril::interfaces::TtyMode::FULL:
      return TtyMode::TTY_MODE_FULL;
    case qcril::interfaces::TtyMode::HCO:
      return TtyMode::TTY_MODE_HCO;
    case qcril::interfaces::TtyMode::VCO:
      return TtyMode::TTY_MODE_VCO;
    case qcril::interfaces::TtyMode::UNKNOWN:
    default:
      return TtyMode::TTY_MODE_INVALID;
  }
}
qcril::interfaces::TtyMode convertTtyMode(const TtyMode &in) {
  switch (in) {
    case TtyMode::TTY_MODE_OFF:
      return qcril::interfaces::TtyMode::MODE_OFF;
    case TtyMode::TTY_MODE_FULL:
      return qcril::interfaces::TtyMode::FULL;
    case TtyMode::TTY_MODE_HCO:
      return qcril::interfaces::TtyMode::HCO;
    case TtyMode::TTY_MODE_VCO:
      return qcril::interfaces::TtyMode::VCO;
    case TtyMode::TTY_MODE_INVALID:
    default:
      return qcril::interfaces::TtyMode::UNKNOWN;
  }
}
ConfParticipantOperation convertConfParticipantOperation(const qcril::interfaces::ConfParticipantOperation &in) {
  switch (in) {
    case qcril::interfaces::ConfParticipantOperation::ADD:
      return ConfParticipantOperation::ADD;
    case qcril::interfaces::ConfParticipantOperation::REMOVE:
      return ConfParticipantOperation::REMOVE;
    case qcril::interfaces::ConfParticipantOperation::UNKNOWN:
    default:
      return ConfParticipantOperation::INVALID;
  }
}
NotificationType convertNotificationType(const qcril::interfaces::NotificationType &in) {
  switch (in) {
    case qcril::interfaces::NotificationType::MO:
      return NotificationType::MO;
    case qcril::interfaces::NotificationType::MT:
      return NotificationType::MT;
    case qcril::interfaces::NotificationType::UNKNOWN:
    default:
      return NotificationType::INVALID;
  }
}

MessagePriority convertMessagePriority(qcril::interfaces::MessagePriority in) {
  switch (in) {
    case qcril::interfaces::MessagePriority::UNKNOWN:
      return MessagePriority::MWI_MSG_PRIORITY_UNKNOWN;
    case qcril::interfaces::MessagePriority::LOW:
      return MessagePriority::MWI_MSG_PRIORITY_LOW;
    case qcril::interfaces::MessagePriority::NORMAL:
      return MessagePriority::MWI_MSG_PRIORITY_NORMAL;
    case qcril::interfaces::MessagePriority::URGENT:
      return MessagePriority::MWI_MSG_PRIORITY_URGENT;
    default:
      return MessagePriority::MWI_MSG_PRIORITY_INVALID;
  }
}
MessageType convertMessageType(qcril::interfaces::MessageType in) {
  switch (in) {
    case qcril::interfaces::MessageType::UNKNOWN:
      return MessageType::MWI_MSG_INVALID;
    case qcril::interfaces::MessageType::NONE:
      return MessageType::MWI_MSG_NONE;
    case qcril::interfaces::MessageType::VOICE:
      return MessageType::MWI_MSG_VOICE;
    case qcril::interfaces::MessageType::VIDEO:
      return MessageType::MWI_MSG_VIDEO;
    case qcril::interfaces::MessageType::FAX:
      return MessageType::MWI_MSG_FAX;
    case qcril::interfaces::MessageType::PAGER:
      return MessageType::MWI_MSG_PAGER;
    case qcril::interfaces::MessageType::MULTIMEDIA:
      return MessageType::MWI_MSG_MULTIMEDIA;
    case qcril::interfaces::MessageType::TEXT:
      return MessageType::MWI_MSG_TEXT;
    default:
      return MessageType::MWI_MSG_INVALID;
  }
}

qcril::interfaces::StatusType convertStatusType(const StatusType &in) {
  switch (in) {
    case StatusType::STATUS_DISABLED:
      return qcril::interfaces::StatusType::DISABLED;
    case StatusType::STATUS_PARTIALLY_ENABLED:
      return qcril::interfaces::StatusType::PARTIALLY_ENABLED;
    case StatusType::STATUS_ENABLED:
      return qcril::interfaces::StatusType::ENABLED;
    case StatusType::STATUS_NOT_SUPPORTED:
      return qcril::interfaces::StatusType::NOT_SUPPORTED;
    case StatusType::STATUS_INVALID:
    default:
      return qcril::interfaces::StatusType::UNKNOWN;
  }
}

void convertVowifiCallQualityInfo(VoWiFiCallQualityInfo &out,
    qcril::interfaces::VowifiQuality in) {
  switch (in) {
    case qcril::interfaces::VowifiQuality::EXCELLENT:
      out = VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_EXCELLENT;
      break;
    case qcril::interfaces::VowifiQuality::FAIR:
      out = VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_FAIR;
      break;
    case qcril::interfaces::VowifiQuality::BAD:
      out = VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_BAD;
      break;
    case qcril::interfaces::VowifiQuality::NONE:
      out = VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_NONE;
      break;
    case qcril::interfaces::VowifiQuality::UNKNOWN:
    default:
      out = VoWiFiCallQualityInfo::VOWIFI_CALL_QUALITY_INVALID;
      break;
  }
}

BlockReasonType convertBlockReasonType(qcril::interfaces::BlockReasonType reason) {
  switch (reason) {
    case qcril::interfaces::BlockReasonType::PDP_FAILURE:
      return BlockReasonType::PDP_FAILURE;
    case qcril::interfaces::BlockReasonType::REGISTRATION_FAILURE:
      return BlockReasonType::REGISTRATION_FAILURE;
    case qcril::interfaces::BlockReasonType::HANDOVER_FAILURE:
      return BlockReasonType::HANDOVER_FAILURE;
    case qcril::interfaces::BlockReasonType::OTHER_FAILURE:
      return BlockReasonType::OTHER_FAILURE;
    default:
      return BlockReasonType::INVALID;
  }
  return BlockReasonType::INVALID;
}

RegFailureReasonType convertProtoToHidlRegFailureReasonType(
  qcril::interfaces::RegFailureReasonType reason) {
  switch (reason) {
    case qcril::interfaces::RegFailureReasonType::UNSPECIFIED:
      return RegFailureReasonType::UNSPECIFIED;
    case qcril::interfaces::RegFailureReasonType::MOBILE_IP:
      return RegFailureReasonType::MOBILE_IP;
    case qcril::interfaces::RegFailureReasonType::INTERNAL:
      return RegFailureReasonType::INTERNAL;
    case qcril::interfaces::RegFailureReasonType::CALL_MANAGER_DEFINED:
      return RegFailureReasonType::CALL_MANAGER_DEFINED;
    case qcril::interfaces::RegFailureReasonType::SPEC_DEFINED:
      return RegFailureReasonType::TYPE_3GPP_SPEC_DEFINED;
    case qcril::interfaces::RegFailureReasonType::PPP:
      return RegFailureReasonType::PPP;
    case qcril::interfaces::RegFailureReasonType::EHRPD:
      return RegFailureReasonType::EHRPD;
    case qcril::interfaces::RegFailureReasonType::IPV6:
      return RegFailureReasonType::IPV6;
    case qcril::interfaces::RegFailureReasonType::IWLAN:
      return RegFailureReasonType::IWLAN;
    case qcril::interfaces::RegFailureReasonType::HANDOFF:
      return RegFailureReasonType::HANDOFF;
    default:
      return RegFailureReasonType::INVALID;
  }
}

ConfigFailureCause convertConfigErrorCause(qcril::interfaces::ConfigFailureCause in) {
  switch (in) {
    case qcril::interfaces::ConfigFailureCause::NO_ERR:
      return ConfigFailureCause::CONFIG_NO_ERR;
    case qcril::interfaces::ConfigFailureCause::IMS_NOT_READY:
      return ConfigFailureCause::CONFIG_IMS_NOT_READY;
    case qcril::interfaces::ConfigFailureCause::FILE_NOT_AVAILABLE:
      return ConfigFailureCause::CONFIG_FILE_NOT_AVAILABLE;
    case qcril::interfaces::ConfigFailureCause::READ_FAILED:
      return ConfigFailureCause::CONFIG_READ_FAILED;
    case qcril::interfaces::ConfigFailureCause::WRITE_FAILED:
      return ConfigFailureCause::CONFIG_WRITE_FAILED;
    case qcril::interfaces::ConfigFailureCause::OTHER_INTERNAL_ERR:
      return ConfigFailureCause::CONFIG_OTHER_INTERNAL_ERR;
    default:
      return ConfigFailureCause::CONFIG_FAILURE_INVALID;
  }
}

qcril::interfaces::ConfigFailureCause convertConfigErrorCause(const ConfigFailureCause in) {
  qcril::interfaces::ConfigFailureCause outErrorCause;
  switch (in) {
    case ConfigFailureCause::CONFIG_NO_ERR:
      outErrorCause = qcril::interfaces::ConfigFailureCause::NO_ERR;
      break;
    case ConfigFailureCause::CONFIG_IMS_NOT_READY:
      outErrorCause = qcril::interfaces::ConfigFailureCause::IMS_NOT_READY;
      break;
    case ConfigFailureCause::CONFIG_FILE_NOT_AVAILABLE:
      outErrorCause = qcril::interfaces::ConfigFailureCause::FILE_NOT_AVAILABLE;
      break;
    case ConfigFailureCause::CONFIG_READ_FAILED:
      outErrorCause = qcril::interfaces::ConfigFailureCause::READ_FAILED;
      break;
    case ConfigFailureCause::CONFIG_WRITE_FAILED:
      outErrorCause = qcril::interfaces::ConfigFailureCause::WRITE_FAILED;
      break;
    case ConfigFailureCause::CONFIG_OTHER_INTERNAL_ERR:
      outErrorCause = qcril::interfaces::ConfigFailureCause::OTHER_INTERNAL_ERR;
      break;
    default:
      outErrorCause = qcril::interfaces::ConfigFailureCause::FAILURE_INVALID;
      break;
  }
  return outErrorCause;
}

qcril::interfaces::ConfigItem convertConfigItem(const ConfigItem in) {
  qcril::interfaces::ConfigItem outConfigItem;
  switch (in) {
    case ConfigItem::CONFIG_ITEM_NONE:
      outConfigItem = qcril::interfaces::ConfigItem::NONE;
      break;
    case ConfigItem::CONFIG_ITEM_VOCODER_AMRMODESET:
      outConfigItem = qcril::interfaces::ConfigItem::VOCODER_AMRMODESET;
      break;
    case ConfigItem::CONFIG_ITEM_VOCODER_AMRWBMODESET:
      outConfigItem = qcril::interfaces::ConfigItem::VOCODER_AMRWBMODESET;
      break;
    case ConfigItem::CONFIG_ITEM_SIP_SESSION_TIMER:
      outConfigItem = qcril::interfaces::ConfigItem::SIP_SESSION_TIMER;
      break;
    case ConfigItem::CONFIG_ITEM_MIN_SESSION_EXPIRY:
      outConfigItem = qcril::interfaces::ConfigItem::MIN_SESSION_EXPIRY;
      break;
    case ConfigItem::CONFIG_ITEM_CANCELLATION_TIMER:
      outConfigItem = qcril::interfaces::ConfigItem::CANCELLATION_TIMER;
      break;
    case ConfigItem::CONFIG_ITEM_T_DELAY:
      outConfigItem = qcril::interfaces::ConfigItem::T_DELAY;
      break;
    case ConfigItem::CONFIG_ITEM_SILENT_REDIAL_ENABLE:
      outConfigItem = qcril::interfaces::ConfigItem::SILENT_REDIAL_ENABLE;
      break;
    case ConfigItem::CONFIG_ITEM_SIP_T1_TIMER:
      outConfigItem = qcril::interfaces::ConfigItem::SIP_T1_TIMER;
      break;
    case ConfigItem::CONFIG_ITEM_SIP_T2_TIMER:
      outConfigItem = qcril::interfaces::ConfigItem::SIP_T2_TIMER;
      break;
    case ConfigItem::CONFIG_ITEM_SIP_TF_TIMER:
      outConfigItem = qcril::interfaces::ConfigItem::SIP_TF_TIMER;
      break;
    case ConfigItem::CONFIG_ITEM_VLT_SETTING_ENABLED:
      outConfigItem = qcril::interfaces::ConfigItem::VLT_SETTING_ENABLED;
      break;
    case ConfigItem::CONFIG_ITEM_LVC_SETTING_ENABLED:
      outConfigItem = qcril::interfaces::ConfigItem::LVC_SETTING_ENABLED;
      break;
    case ConfigItem::CONFIG_ITEM_DOMAIN_NAME:
      outConfigItem = qcril::interfaces::ConfigItem::DOMAIN_NAME;
      break;
    case ConfigItem::CONFIG_ITEM_SMS_FORMAT:
      outConfigItem = qcril::interfaces::ConfigItem::SMS_FORMAT;
      break;
    case ConfigItem::CONFIG_ITEM_SMS_OVER_IP:
      outConfigItem = qcril::interfaces::ConfigItem::SMS_OVER_IP;
      break;
    case ConfigItem::CONFIG_ITEM_PUBLISH_TIMER:
      outConfigItem = qcril::interfaces::ConfigItem::PUBLISH_TIMER;
      break;
    case ConfigItem::CONFIG_ITEM_PUBLISH_TIMER_EXTENDED:
      outConfigItem = qcril::interfaces::ConfigItem::PUBLISH_TIMER_EXTENDED;
      break;
    case ConfigItem::CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION:
      outConfigItem = qcril::interfaces::ConfigItem::CAPABILITIES_CACHE_EXPIRATION;
      break;
    case ConfigItem::CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION:
      outConfigItem = qcril::interfaces::ConfigItem::AVAILABILITY_CACHE_EXPIRATION;
      break;
    case ConfigItem::CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL:
      outConfigItem = qcril::interfaces::ConfigItem::CAPABILITIES_POLL_INTERVAL;
      break;
    case ConfigItem::CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH:
      outConfigItem = qcril::interfaces::ConfigItem::SOURCE_THROTTLE_PUBLISH;
      break;
    case ConfigItem::CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL:
      outConfigItem = qcril::interfaces::ConfigItem::MAX_NUM_ENTRIES_IN_RCL;
      break;
    case ConfigItem::CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP:
      outConfigItem = qcril::interfaces::ConfigItem::CAPAB_POLL_LIST_SUB_EXP;
      break;
    case ConfigItem::CONFIG_ITEM_GZIP_FLAG:
      outConfigItem = qcril::interfaces::ConfigItem::GZIP_FLAG;
      break;
    case ConfigItem::CONFIG_ITEM_EAB_SETTING_ENABLED:
      outConfigItem = qcril::interfaces::ConfigItem::EAB_SETTING_ENABLED;
      break;
    case ConfigItem::CONFIG_ITEM_MOBILE_DATA_ENABLED:
      outConfigItem = qcril::interfaces::ConfigItem::MOBILE_DATA_ENABLED;
      break;
    case ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED:
      outConfigItem = qcril::interfaces::ConfigItem::VOICE_OVER_WIFI_ENABLED;
      break;
    case ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING:
      outConfigItem = qcril::interfaces::ConfigItem::VOICE_OVER_WIFI_ROAMING;
      break;
    case ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_MODE:
      outConfigItem = qcril::interfaces::ConfigItem::VOICE_OVER_WIFI_MODE;
      break;
    case ConfigItem::CONFIG_ITEM_CAPABILITY_DISCOVERY_ENABLED:
      outConfigItem = qcril::interfaces::ConfigItem::CAPABILITY_DISCOVERY_ENABLED;
      break;
    case ConfigItem::CONFIG_ITEM_EMERGENCY_CALL_TIMER:
      outConfigItem = qcril::interfaces::ConfigItem::EMERGENCY_CALL_TIMER;
      break;
    case ConfigItem::CONFIG_ITEM_SSAC_HYSTERESIS_TIMER:
      outConfigItem = qcril::interfaces::ConfigItem::SSAC_HYSTERESIS_TIMER;
      break;
    case ConfigItem::CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS:
      outConfigItem = qcril::interfaces::ConfigItem::VOLTE_USER_OPT_IN_STATUS;
      break;
    case ConfigItem::CONFIG_ITEM_LBO_PCSCF_ADDRESS:
      outConfigItem = qcril::interfaces::ConfigItem::LBO_PCSCF_ADDRESS;
      break;
    case ConfigItem::CONFIG_ITEM_KEEP_ALIVE_ENABLED:
      outConfigItem = qcril::interfaces::ConfigItem::KEEP_ALIVE_ENABLED;
      break;
    case ConfigItem::CONFIG_ITEM_REGISTRATION_RETRY_BASE_TIME_SEC:
      outConfigItem = qcril::interfaces::ConfigItem::REGISTRATION_RETRY_BASE_TIME_SEC;
      break;
    case ConfigItem::CONFIG_ITEM_REGISTRATION_RETRY_MAX_TIME_SEC:
      outConfigItem = qcril::interfaces::ConfigItem::REGISTRATION_RETRY_MAX_TIME_SEC;
      break;
    case ConfigItem::CONFIG_ITEM_SPEECH_START_PORT:
      outConfigItem = qcril::interfaces::ConfigItem::SPEECH_START_PORT;
      break;
    case ConfigItem::CONFIG_ITEM_SPEECH_END_PORT:
      outConfigItem = qcril::interfaces::ConfigItem::SPEECH_END_PORT;
      break;
    case ConfigItem::CONFIG_ITEM_SIP_INVITE_REQ_RETX_INTERVAL_MSEC:
      outConfigItem = qcril::interfaces::ConfigItem::SIP_INVITE_REQ_RETX_INTERVAL_MSEC;
      break;
    case ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_WAIT_TIME_MSEC:
      outConfigItem = qcril::interfaces::ConfigItem::SIP_INVITE_RSP_WAIT_TIME_MSEC;
      break;
    case ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC:
      outConfigItem = qcril::interfaces::ConfigItem::SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC;
      break;
    case ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC:
      outConfigItem = qcril::interfaces::ConfigItem::SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC;
      break;
    case ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC:
      outConfigItem = qcril::interfaces::ConfigItem::SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC;
      break;
    case ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_RETX_INTERVAL_MSEC:
      outConfigItem = qcril::interfaces::ConfigItem::SIP_INVITE_RSP_RETX_INTERVAL_MSEC;
      break;
    case ConfigItem::CONFIG_ITEM_SIP_ACK_RECEIPT_WAIT_TIME_MSEC:
      outConfigItem = qcril::interfaces::ConfigItem::SIP_ACK_RECEIPT_WAIT_TIME_MSEC;
      break;
    case ConfigItem::CONFIG_ITEM_SIP_ACK_RETX_WAIT_TIME_MSEC:
      outConfigItem = qcril::interfaces::ConfigItem::SIP_ACK_RETX_WAIT_TIME_MSEC;
      break;
    case ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC:
      outConfigItem = qcril::interfaces::ConfigItem::SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC;
      break;
    case ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC:
      outConfigItem = qcril::interfaces::ConfigItem::SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC;
      break;
    case ConfigItem::CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT:
      outConfigItem = qcril::interfaces::ConfigItem::AMR_WB_OCTET_ALIGNED_PT;
      break;
    case ConfigItem::CONFIG_ITEM_AMR_WB_BANDWIDTH_EFFICIENT_PT:
      outConfigItem = qcril::interfaces::ConfigItem::AMR_WB_BANDWIDTH_EFFICIENT_PT;
      break;
    case ConfigItem::CONFIG_ITEM_AMR_OCTET_ALIGNED_PT:
      outConfigItem = qcril::interfaces::ConfigItem::AMR_OCTET_ALIGNED_PT;
      break;
    case ConfigItem::CONFIG_ITEM_AMR_BANDWIDTH_EFFICIENT_PT:
      outConfigItem = qcril::interfaces::ConfigItem::AMR_BANDWIDTH_EFFICIENT_PT;
      break;
    case ConfigItem::CONFIG_ITEM_DTMF_WB_PT:
      outConfigItem = qcril::interfaces::ConfigItem::DTMF_WB_PT;
      break;
    case ConfigItem::CONFIG_ITEM_DTMF_NB_PT:
      outConfigItem = qcril::interfaces::ConfigItem::DTMF_NB_PT;
      break;
    case ConfigItem::CONFIG_ITEM_AMR_DEFAULT_MODE:
      outConfigItem = qcril::interfaces::ConfigItem::AMR_DEFAULT_MODE;
      break;
    case ConfigItem::CONFIG_ITEM_SMS_PSI:
      outConfigItem = qcril::interfaces::ConfigItem::SMS_PSI;
      break;
    case ConfigItem::CONFIG_ITEM_VIDEO_QUALITY:
      outConfigItem = qcril::interfaces::ConfigItem::VIDEO_QUALITY;
      break;
    case ConfigItem::CONFIG_ITEM_THRESHOLD_LTE1:
      outConfigItem = qcril::interfaces::ConfigItem::THRESHOLD_LTE1;
      break;
    case ConfigItem::CONFIG_ITEM_THRESHOLD_LTE2:
      outConfigItem = qcril::interfaces::ConfigItem::THRESHOLD_LTE2;
      break;
    case ConfigItem::CONFIG_ITEM_THRESHOLD_LTE3:
      outConfigItem = qcril::interfaces::ConfigItem::THRESHOLD_LTE3;
      break;
    case ConfigItem::CONFIG_ITEM_THRESHOLD_1x:
      outConfigItem = qcril::interfaces::ConfigItem::THRESHOLD_1x;
      break;
    case ConfigItem::CONFIG_ITEM_THRESHOLD_WIFI_A:
      outConfigItem = qcril::interfaces::ConfigItem::THRESHOLD_WIFI_A;
      break;
    case ConfigItem::CONFIG_ITEM_THRESHOLD_WIFI_B:
      outConfigItem = qcril::interfaces::ConfigItem::THRESHOLD_WIFI_B;
      break;
    case ConfigItem::CONFIG_ITEM_T_EPDG_LTE:
      outConfigItem = qcril::interfaces::ConfigItem::T_EPDG_LTE;
      break;
    case ConfigItem::CONFIG_ITEM_T_EPDG_WIFI:
      outConfigItem = qcril::interfaces::ConfigItem::T_EPDG_WIFI;
      break;
    case ConfigItem::CONFIG_ITEM_T_EPDG_1x:
      outConfigItem = qcril::interfaces::ConfigItem::T_EPDG_1x;
      break;
    case ConfigItem::CONFIG_ITEM_VWF_SETTING_ENABLED:
      outConfigItem = qcril::interfaces::ConfigItem::VWF_SETTING_ENABLED;
      break;
    case ConfigItem::CONFIG_ITEM_VCE_SETTING_ENABLED:
      outConfigItem = qcril::interfaces::ConfigItem::VCE_SETTING_ENABLED;
      break;
    case ConfigItem::CONFIG_ITEM_RTT_SETTING_ENABLED:
      outConfigItem = qcril::interfaces::ConfigItem::RTT_SETTING_ENABLED;
      break;
    case ConfigItem::CONFIG_ITEM_SMS_APP:
      outConfigItem = qcril::interfaces::ConfigItem::SMS_APP;
      break;
    case ConfigItem::CONFIG_ITEM_VVM_APP:
      outConfigItem = qcril::interfaces::ConfigItem::VVM_APP;
      break;
    default:
      outConfigItem = qcril::interfaces::ConfigItem::INVALID_CONFIG;
      break;
  }
  return outConfigItem;
}

ConfigItem convertConfigItem(qcril::interfaces::ConfigItem inConfigItem) {
  switch (inConfigItem) {
    case qcril::interfaces::ConfigItem::NONE:
      return ConfigItem::CONFIG_ITEM_NONE;
    case qcril::interfaces::ConfigItem::VOCODER_AMRMODESET:
      return ConfigItem::CONFIG_ITEM_VOCODER_AMRMODESET;
    case qcril::interfaces::ConfigItem::VOCODER_AMRWBMODESET:
      return ConfigItem::CONFIG_ITEM_VOCODER_AMRWBMODESET;
    case qcril::interfaces::ConfigItem::SIP_SESSION_TIMER:
      return ConfigItem::CONFIG_ITEM_SIP_SESSION_TIMER;
    case qcril::interfaces::ConfigItem::MIN_SESSION_EXPIRY:
      return ConfigItem::CONFIG_ITEM_MIN_SESSION_EXPIRY;
    case qcril::interfaces::ConfigItem::CANCELLATION_TIMER:
      return ConfigItem::CONFIG_ITEM_CANCELLATION_TIMER;
    case qcril::interfaces::ConfigItem::T_DELAY:
      return ConfigItem::CONFIG_ITEM_T_DELAY;
    case qcril::interfaces::ConfigItem::SILENT_REDIAL_ENABLE:
      return ConfigItem::CONFIG_ITEM_SILENT_REDIAL_ENABLE;
    case qcril::interfaces::ConfigItem::SIP_T1_TIMER:
      return ConfigItem::CONFIG_ITEM_SIP_T1_TIMER;
    case qcril::interfaces::ConfigItem::SIP_T2_TIMER:
      return ConfigItem::CONFIG_ITEM_SIP_T2_TIMER;
    case qcril::interfaces::ConfigItem::SIP_TF_TIMER:
      return ConfigItem::CONFIG_ITEM_SIP_TF_TIMER;
    case qcril::interfaces::ConfigItem::VLT_SETTING_ENABLED:
      return ConfigItem::CONFIG_ITEM_VLT_SETTING_ENABLED;
    case qcril::interfaces::ConfigItem::LVC_SETTING_ENABLED:
      return ConfigItem::CONFIG_ITEM_LVC_SETTING_ENABLED;
    case qcril::interfaces::ConfigItem::DOMAIN_NAME:
      return ConfigItem::CONFIG_ITEM_DOMAIN_NAME;
    case qcril::interfaces::ConfigItem::SMS_FORMAT:
      return ConfigItem::CONFIG_ITEM_SMS_FORMAT;
    case qcril::interfaces::ConfigItem::SMS_OVER_IP:
      return ConfigItem::CONFIG_ITEM_SMS_OVER_IP;
    case qcril::interfaces::ConfigItem::PUBLISH_TIMER:
      return ConfigItem::CONFIG_ITEM_PUBLISH_TIMER;
    case qcril::interfaces::ConfigItem::PUBLISH_TIMER_EXTENDED:
      return ConfigItem::CONFIG_ITEM_PUBLISH_TIMER_EXTENDED;
    case qcril::interfaces::ConfigItem::CAPABILITIES_CACHE_EXPIRATION:
      return ConfigItem::CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION;
    case qcril::interfaces::ConfigItem::AVAILABILITY_CACHE_EXPIRATION:
      return ConfigItem::CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION;
    case qcril::interfaces::ConfigItem::CAPABILITIES_POLL_INTERVAL:
      return ConfigItem::CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL;
    case qcril::interfaces::ConfigItem::SOURCE_THROTTLE_PUBLISH:
      return ConfigItem::CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH;
    case qcril::interfaces::ConfigItem::MAX_NUM_ENTRIES_IN_RCL:
      return ConfigItem::CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL;
    case qcril::interfaces::ConfigItem::CAPAB_POLL_LIST_SUB_EXP:
      return ConfigItem::CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP;
    case qcril::interfaces::ConfigItem::GZIP_FLAG:
      return ConfigItem::CONFIG_ITEM_GZIP_FLAG;
    case qcril::interfaces::ConfigItem::EAB_SETTING_ENABLED:
      return ConfigItem::CONFIG_ITEM_EAB_SETTING_ENABLED;
    case qcril::interfaces::ConfigItem::MOBILE_DATA_ENABLED:
      return ConfigItem::CONFIG_ITEM_MOBILE_DATA_ENABLED;
    case qcril::interfaces::ConfigItem::VOICE_OVER_WIFI_ENABLED:
      return ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
    case qcril::interfaces::ConfigItem::VOICE_OVER_WIFI_ROAMING:
      return ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING;
    case qcril::interfaces::ConfigItem::VOICE_OVER_WIFI_MODE:
      return ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_MODE;
    case qcril::interfaces::ConfigItem::CAPABILITY_DISCOVERY_ENABLED:
      return ConfigItem::CONFIG_ITEM_CAPABILITY_DISCOVERY_ENABLED;
    case qcril::interfaces::ConfigItem::EMERGENCY_CALL_TIMER:
      return ConfigItem::CONFIG_ITEM_EMERGENCY_CALL_TIMER;
    case qcril::interfaces::ConfigItem::SSAC_HYSTERESIS_TIMER:
      return ConfigItem::CONFIG_ITEM_SSAC_HYSTERESIS_TIMER;
    case qcril::interfaces::ConfigItem::VOLTE_USER_OPT_IN_STATUS:
      return ConfigItem::CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS;
    case qcril::interfaces::ConfigItem::LBO_PCSCF_ADDRESS:
      return ConfigItem::CONFIG_ITEM_LBO_PCSCF_ADDRESS;
    case qcril::interfaces::ConfigItem::KEEP_ALIVE_ENABLED:
      return ConfigItem::CONFIG_ITEM_KEEP_ALIVE_ENABLED;
    case qcril::interfaces::ConfigItem::REGISTRATION_RETRY_BASE_TIME_SEC:
      return ConfigItem::CONFIG_ITEM_REGISTRATION_RETRY_BASE_TIME_SEC;
    case qcril::interfaces::ConfigItem::REGISTRATION_RETRY_MAX_TIME_SEC:
      return ConfigItem::CONFIG_ITEM_REGISTRATION_RETRY_MAX_TIME_SEC;
    case qcril::interfaces::ConfigItem::SPEECH_START_PORT:
      return ConfigItem::CONFIG_ITEM_SPEECH_START_PORT;
    case qcril::interfaces::ConfigItem::SPEECH_END_PORT:
      return ConfigItem::CONFIG_ITEM_SPEECH_END_PORT;
    case qcril::interfaces::ConfigItem::SIP_INVITE_REQ_RETX_INTERVAL_MSEC:
      return ConfigItem::CONFIG_ITEM_SIP_INVITE_REQ_RETX_INTERVAL_MSEC;
    case qcril::interfaces::ConfigItem::SIP_INVITE_RSP_WAIT_TIME_MSEC:
      return ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_WAIT_TIME_MSEC;
    case qcril::interfaces::ConfigItem::SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC:
      return ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC;
    case qcril::interfaces::ConfigItem::SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC:
      return ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC;
    case qcril::interfaces::ConfigItem::SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC:
      return ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC;
    case qcril::interfaces::ConfigItem::SIP_INVITE_RSP_RETX_INTERVAL_MSEC:
      return ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_RETX_INTERVAL_MSEC;
    case qcril::interfaces::ConfigItem::SIP_ACK_RECEIPT_WAIT_TIME_MSEC:
      return ConfigItem::CONFIG_ITEM_SIP_ACK_RECEIPT_WAIT_TIME_MSEC;
    case qcril::interfaces::ConfigItem::SIP_ACK_RETX_WAIT_TIME_MSEC:
      return ConfigItem::CONFIG_ITEM_SIP_ACK_RETX_WAIT_TIME_MSEC;
    case qcril::interfaces::ConfigItem::SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC:
      return ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC;
    case qcril::interfaces::ConfigItem::SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC:
      return ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC;
    case qcril::interfaces::ConfigItem::AMR_WB_OCTET_ALIGNED_PT:
      return ConfigItem::CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT;
    case qcril::interfaces::ConfigItem::AMR_WB_BANDWIDTH_EFFICIENT_PT:
      return ConfigItem::CONFIG_ITEM_AMR_WB_BANDWIDTH_EFFICIENT_PT;
    case qcril::interfaces::ConfigItem::AMR_OCTET_ALIGNED_PT:
      return ConfigItem::CONFIG_ITEM_AMR_OCTET_ALIGNED_PT;
    case qcril::interfaces::ConfigItem::AMR_BANDWIDTH_EFFICIENT_PT:
      return ConfigItem::CONFIG_ITEM_AMR_BANDWIDTH_EFFICIENT_PT;
    case qcril::interfaces::ConfigItem::DTMF_WB_PT:
      return ConfigItem::CONFIG_ITEM_DTMF_WB_PT;
    case qcril::interfaces::ConfigItem::DTMF_NB_PT:
      return ConfigItem::CONFIG_ITEM_DTMF_NB_PT;
    case qcril::interfaces::ConfigItem::AMR_DEFAULT_MODE:
      return ConfigItem::CONFIG_ITEM_AMR_DEFAULT_MODE;
    case qcril::interfaces::ConfigItem::SMS_PSI:
      return ConfigItem::CONFIG_ITEM_SMS_PSI;
    case qcril::interfaces::ConfigItem::VIDEO_QUALITY:
      return ConfigItem::CONFIG_ITEM_VIDEO_QUALITY;
    case qcril::interfaces::ConfigItem::THRESHOLD_LTE1:
      return ConfigItem::CONFIG_ITEM_THRESHOLD_LTE1;
    case qcril::interfaces::ConfigItem::THRESHOLD_LTE2:
      return ConfigItem::CONFIG_ITEM_THRESHOLD_LTE2;
    case qcril::interfaces::ConfigItem::THRESHOLD_LTE3:
      return ConfigItem::CONFIG_ITEM_THRESHOLD_LTE3;
    case qcril::interfaces::ConfigItem::THRESHOLD_1x:
      return ConfigItem::CONFIG_ITEM_THRESHOLD_1x;
    case qcril::interfaces::ConfigItem::THRESHOLD_WIFI_A:
      return ConfigItem::CONFIG_ITEM_THRESHOLD_WIFI_A;
    case qcril::interfaces::ConfigItem::THRESHOLD_WIFI_B:
      return ConfigItem::CONFIG_ITEM_THRESHOLD_WIFI_B;
    case qcril::interfaces::ConfigItem::T_EPDG_LTE:
      return ConfigItem::CONFIG_ITEM_T_EPDG_LTE;
    case qcril::interfaces::ConfigItem::T_EPDG_WIFI:
      return ConfigItem::CONFIG_ITEM_T_EPDG_WIFI;
    case qcril::interfaces::ConfigItem::T_EPDG_1x:
      return ConfigItem::CONFIG_ITEM_T_EPDG_1x;
    case qcril::interfaces::ConfigItem::VWF_SETTING_ENABLED:
      return ConfigItem::CONFIG_ITEM_VWF_SETTING_ENABLED;
    case qcril::interfaces::ConfigItem::VCE_SETTING_ENABLED:
      return ConfigItem::CONFIG_ITEM_VCE_SETTING_ENABLED;
    case qcril::interfaces::ConfigItem::RTT_SETTING_ENABLED:
      return ConfigItem::CONFIG_ITEM_RTT_SETTING_ENABLED;
    case qcril::interfaces::ConfigItem::SMS_APP:
      return ConfigItem::CONFIG_ITEM_SMS_APP;
    case qcril::interfaces::ConfigItem::VVM_APP:
      return ConfigItem::CONFIG_ITEM_VVM_APP;
    default:
      return ConfigItem::CONFIG_ITEM_INVALID;
    }
}

V1_3::VoiceInfo convertToHidlVoiceInfo(qcril::interfaces::VoiceInfo voiceInfo) {
    switch(voiceInfo) {
      case qcril::interfaces::VoiceInfo::SILENT:
        return V1_3::VoiceInfo::VOICE_INFO_SILENT;
      case qcril::interfaces::VoiceInfo::SPEECH:
        return V1_3::VoiceInfo::VOICE_INFO_SPEECH;
      default:
        return V1_3::VoiceInfo::VOICE_INFO_UNKNOWN;
    }
}

RadioState convertToHidlRadioState(qcril::interfaces::RadioState inState) {
    switch (inState) {
        case qcril::interfaces::RadioState::STATE_OFF:
            return RadioState::RADIO_STATE_OFF;
        case qcril::interfaces::RadioState::STATE_ON:
            return RadioState::RADIO_STATE_ON;
        case qcril::interfaces::RadioState::STATE_UNAVAILABLE:
            return RadioState::RADIO_STATE_UNAVAILABLE;
        default:
            return RadioState::RADIO_STATE_INVALID;
    }
    return RadioState::RADIO_STATE_INVALID;
}

bool convertRegistrationInfo(RegistrationInfo &out, const qcril::interfaces::Registration &in) {
  if (in.hasState()) {
    out.state = convertRegState(in.getState());
  } else {
    out.state = RegState::INVALID;
  }
  if (in.hasErrorCode()) {
    out.errorCode = in.getErrorCode();
  } else {
    out.errorCode = INT32_MAX;
  }
  if (in.hasErrorMessage()) { // && in.getErrorMessage() != nullptr)
    out.errorMessage = in.getErrorMessage().c_str();
  }
  if (in.hasRadioTechnology()) {
    out.radioTech = convertRadioTechType(in.getRadioTechnology());
  } else {
    out.radioTech = RadioTechType::RADIO_TECH_INVALID;
  }
  if (in.hasPAssociatedUris()) { // && in.getPAssociatedUris() != nullptr)
    out.pAssociatedUris = in.getPAssociatedUris().c_str();
  }
  return true;
}

bool convertAccessTechnologyStatus(StatusForAccessTech &out,
                                   const qcril::interfaces::AccessTechnologyStatus &in) {
  if (in.hasNetworkMode()) {
    out.networkMode = convertRadioTechType(in.getNetworkMode());
  } else {
    out.networkMode = RadioTechType::RADIO_TECH_INVALID;
  }

  if (in.hasStatusType()) {
    out.status = convertStatusType(in.getStatusType());
  } else {
    out.status = StatusType::STATUS_INVALID;
  }

  if (in.hasRestrictCause()) {
    out.restrictCause = in.getRestrictCause();
  } else {
    out.restrictCause = INT32_MAX;
  }

  out.hasRegistration = in.hasRegistration();
  if (in.hasRegistration()) {
    convertRegistrationInfo(out.registration, *(in.getRegistration()));
  }
  return true;
}

bool convertServiceStatusInfo(ServiceStatusInfo &out,
                              const qcril::interfaces::ServiceStatusInfo &in) {
  out.hasIsValid = true;
  out.isValid = true;

  out.type = ServiceType::INVALID;

  if (in.hasCallType()) {
    out.callType = convertCallType(in.getCallType());
  } else {
    out.callType = CallType::CALL_TYPE_INVALID;
  }

  if (in.hasStatusType()) {
    out.status = convertStatusType(in.getStatusType());
  } else {
    out.status = StatusType::STATUS_INVALID;
  }

  if (in.hasRestrictCause()) {
    out.restrictCause = in.getRestrictCause();
  } else {
    out.restrictCause = INT32_MAX;
  }

  if (in.hasRttMode()) {
    out.rttMode = convertRttMode(in.getRttMode());
  } else {
    out.rttMode = RttMode::RTT_MODE_INVALID;
  }

  if (in.hasAccessTechnologyStatus() && in.getAccessTechnologyStatus() != nullptr) {
    out.accTechStatus.resize(1);
    out.accTechStatus[0] = {};
    convertAccessTechnologyStatus(out.accTechStatus[0], *(in.getAccessTechnologyStatus()));
  }
  return true;
}

bool convertServiceStatusInfoList(hidl_vec<ServiceStatusInfo> &out,
                                  const qcril::interfaces::ServiceStatusInfoList &in) {
  uint32_t count = in.getServiceStatusInfo().size();
  out.resize(count);
  for (uint32_t i = 0; i < count; ++i) {
    out[i] = {};
    convertServiceStatusInfo(out[i], in.getServiceStatusInfo()[i]);
  }
  return true;
}

bool convertServiceStatusInfoList(hidl_vec<ServiceStatusInfo> &out,
                                  const std::vector<qcril::interfaces::ServiceStatusInfo> &in) {
  uint32_t count = in.size();
  out.resize(count);
  for (uint32_t i = 0; i < count; ++i) {
    out[i] = {};
    convertServiceStatusInfo(out[i], in[i]);
  }
  return true;
}

bool convertCallForwardInfo(CallForwardInfo &out, const qcril::interfaces::CallForwardInfo &in) {
  if (in.hasStatus()) {
    out.status = in.getStatus();
  }
  if (in.hasReason()) {
    out.reason = in.getReason();
  }
  if (in.hasServiceClass()) {
    out.serviceClass = in.getServiceClass();
  }
  if (in.hasToa()) {
    out.toa = in.getToa();
  }
  if (in.hasNumber() && !in.getNumber().empty()) {
    out.number = in.getNumber().c_str();
  }
  if (in.hasTimeSeconds()) {
    out.timeSeconds = in.getTimeSeconds();
  }
  if (in.hasCallFwdTimerStart() && in.getCallFwdTimerStart() != nullptr) {
    out.hasCallFwdTimerStart = true;
    convertCallFwdTimerInfo(out.callFwdTimerStart, in.getCallFwdTimerStart());
  }
  if (in.hasCallFwdTimerEnd() && in.getCallFwdTimerEnd() != nullptr) {
    out.hasCallFwdTimerEnd = true;
    convertCallFwdTimerInfo(out.callFwdTimerEnd, in.getCallFwdTimerEnd());
  }
  return true;
}

bool convertCallForwardInfoList(hidl_vec<CallForwardInfo> &out,
                                const std::vector<qcril::interfaces::CallForwardInfo> &in) {
  uint32_t count = in.size();
  out.resize(count);
  for (uint32_t i = 0; i < count; ++i) {
    out[i] = {.status = INT32_MAX,
              .reason = INT32_MAX,
              .serviceClass = INT32_MAX,
              .toa = INT32_MAX,
              .timeSeconds = INT32_MAX,
              .hasCallFwdTimerStart = false,
              .hasCallFwdTimerEnd = false};
    convertCallForwardInfo(out[i], in[i]);
  }
  return true;
}

bool convertSipErrorInfo(SipErrorInfo &out,
                         const std::shared_ptr<qcril::interfaces::SipErrorInfo> &in) {
  if (in == nullptr || !in->hasErrorString()) {
    return false;
  }
  out.errorCode = in->hasErrorCode() ? in->getErrorCode() : INT32_MAX;
  out.errorString = in->hasErrorString() ? in->getErrorString() : "";
  return true;
}

bool convertSipErrorInfo(std::shared_ptr<qcril::interfaces::SipErrorInfo> &out,
                         const SipErrorInfo &in) {
  if (out == nullptr) {
    return false;
  }
  if (in.errorCode != INT32_MAX) {
    out->setErrorCode(in.errorCode);
  }
  if (!in.errorString.empty()) {
    out->setErrorString(in.errorString.c_str());
  }
  return true;
}

bool convertCallFwdTimerInfo(CallFwdTimerInfo &out,
                             const std::shared_ptr<qcril::interfaces::CallFwdTimerInfo> &in) {
  if (in == nullptr) {
    return false;
  }
  out.year = (in->hasYear()) ? in->getYear() : INT32_MAX;
  out.month = (in->hasMonth()) ? in->getMonth() : INT32_MAX;
  out.day = (in->hasDay()) ? in->getDay() : INT32_MAX;
  out.hour = (in->hasHour()) ? in->getHour() : INT32_MAX;
  out.minute = (in->hasMinute()) ? in->getMinute() : INT32_MAX;
  out.second = (in->hasSecond()) ? in->getSecond() : INT32_MAX;
  out.timezone = (in->hasTimezone()) ? in->getTimezone() : INT32_MAX;

  return true;
}

bool convertCallFwdTimerInfo(std::shared_ptr<qcril::interfaces::CallFwdTimerInfo> &out,
                             const CallFwdTimerInfo &in) {
  if (out == nullptr) {
    return false;
  }
  if (in.year != INT32_MAX) {
    out->setYear(in.year);
  }
  if (in.month != INT32_MAX) {
    out->setMonth(in.month);
  }
  if (in.day != INT32_MAX) {
    out->setDay(in.day);
  }
  if (in.hour != INT32_MAX) {
    out->setHour(in.hour);
  }
  if (in.minute != INT32_MAX) {
    out->setMinute(in.minute);
  }
  if (in.second != INT32_MAX) {
    out->setSecond(in.second);
  }
  if (in.timezone != INT32_MAX) {
    out->setTimezone(in.timezone);
  }
  return true;
}

V1_2::VerificationStatus convertVerificationStatus(const qcril::interfaces::VerificationStatus &in) {
  switch (in) {
    case qcril::interfaces::VerificationStatus::PASS:
      return V1_2::VerificationStatus::STATUS_VALIDATION_PASS;
    case qcril::interfaces::VerificationStatus::FAIL:
      return V1_2::VerificationStatus::STATUS_VALIDATION_FAIL;
    default:
      return V1_2::VerificationStatus::STATUS_VALIDATION_NONE;
  }
}

bool convertVerstatInfo(V1_3::VerstatInfo &out,
                        const std::shared_ptr<qcril::interfaces::VerstatInfo> &in) {
  if (in == nullptr) {
    return false;
  }
  if (in->hasCanMarkUnwantedCall()) {
    out.canMarkUnwantedCall = in->getCanMarkUnwantedCall();
  }
  if (in->hasVerificationStatus()) {
    out.verificationStatus = convertVerificationStatus(in->getVerificationStatus());
  }
  return true;
}

std::string getCodecStringExtra(uint32_t codec) {
  std::string result = "";
  const char *codecstr[] = {
      "Codec=NONE",     /* 0x0000  None   */
      "Codec=QCELP13K", /* 0x0001  QCELP-13K   */
      "Codec=EVRC",     /* 0x0002  EVRC   */
      "Codec=EVRC_B",   /* 0x0003  EVRC-B   */
      "Codec=EVRC_WB",  /* 0x0004  EVRC wideband   */
      "Codec=EVRC_NW",  /* 0x0005  EVRC narrowband-wideband   */
      "Codec=AMR_NB",   /* 0x0006  AMR narrowband   */
      "Codec=AMR_WB",   /* 0x0007  AMR wideband   */
      "Codec=GSM_EFR",  /* 0x0008  GSM enhanced full rate   */
      "Codec=GSM_FR",   /* 0x0009  GSM full rate   */
      "Codec=GSM_HR",   /* 0x000A  GSM half rate   */
      "Codec=G711U",    /* 0x000B  G711U   */
      "Codec=G723",     /* 0x000C  G723   */
      "Codec=G711A",    /* 0x000D  G711A   */
      "Codec=G722",     /* 0x000E  G722   */
      "Codec=G711AB",   /* 0x000F  G711AB   */
      "Codec=G729",     /* 0x0010  G729   */
      "Codec=EVS_NB",   /* 0x0011  EVS narrowband   */
      "Codec=EVS_WB",   /* 0x0012  EVS_WB wideband   */
      "Codec=EVS_SWB",  /* 0x0013  EVS super-wideband   */
      "Codec=EVS_FB",   /* 0x0014  EVS fullband  */
  };
  if (codec < sizeof(codecstr) / sizeof(codecstr[0])) {
    result = codecstr[codec];
  }
  return result;
}

bool convertCallInfo(V1_3::CallInfo &out, const qcril::interfaces::CallInfo &in) {
  if (in.hasCallState()) {
    out.state = convertCallState(in.getCallState());
  } else {
    out.state = CallState::CALL_STATE_INVALID;
  }

  if (in.hasIndex()) {
    out.index = in.getIndex();
  } else {
    out.index = INT32_MAX;
  }

  if (in.hasToa()) {
    out.toa = in.getToa();
  } else {
    out.toa = INT32_MAX;
  }

  out.hasIsMpty = in.hasIsMpty();
  if (in.hasIsMpty()) {
    out.isMpty = in.getIsMpty();
  }

  out.hasIsMT = in.hasIsMt();
  if (in.hasIsMt()) {
    out.isMT = in.getIsMt();
  }

  if (in.hasAls()) {
    out.als = in.getAls();
  } else {
    out.als = INT32_MAX;
  }

  out.hasIsVoice = in.hasIsVoice();
  if (in.hasIsVoice()) {
    out.isVoice = in.getIsVoice();
  }

  out.hasIsVoicePrivacy = in.hasIsVoicePrivacy();
  if (in.hasIsVoicePrivacy()) {
    out.isVoicePrivacy = in.getIsVoicePrivacy();
  }

  if (in.hasNumber() && in.getNumber().size() > 0) {
    out.number = in.getNumber().c_str();
  }

  if (in.hasNumberPresentation()) {
    out.numberPresentation = in.getNumberPresentation();
  } else {
    out.numberPresentation = INT32_MAX;
  }

  if (in.hasName() && in.getName().size() > 0) {
    out.name = in.getName().c_str();
  }

  if (in.hasNamePresentation()) {
    out.namePresentation = in.getNamePresentation();
  } else {
    out.namePresentation = INT32_MAX;
  }

  // Call Details
  out.hasCallDetails = true;

  if (in.hasCallType()) {
    out.callDetails.callType = convertCallType(in.getCallType());
  } else {
    out.callDetails.callType = CallType::CALL_TYPE_INVALID;
  }

  if (in.hasCallDomain()) {
    out.callDetails.callDomain = convertCallDomain(in.getCallDomain());
  } else {
    out.callDetails.callDomain = CallDomain::CALL_DOMAIN_INVALID;
  }

  uint32_t extrasLength = 0;
  std::vector<std::string> extras;
  if (in.hasCodec()) {
    std::string codec = getCodecStringExtra(in.getCodec());
    if (codec.size()) {
      extrasLength++;
      extras.push_back(codec);
    }
  }
  if (in.hasDisplayText() && in.getDisplayText().size()) {
    extrasLength++;
    extras.push_back("DisplayText=" + in.getDisplayText());
  }
  if (in.hasAdditionalCallInfo() && in.getAdditionalCallInfo().size()) {
    extrasLength++;
    extras.push_back("AdditionalCallInfo=" + in.getAdditionalCallInfo());
  }
  if (in.hasChildNumber() && in.getChildNumber().size()) {
    extrasLength++;
    extras.push_back("ChildNum=" + in.getChildNumber());
  }
  if (extras.size()) {
    out.callDetails.extrasLength = extras.size();
  } else {
    out.callDetails.extrasLength = INT32_MAX;
  }
  out.callDetails.extras.resize(0);
  if (extras.size()) {
    out.callDetails.extras.resize(extras.size());
    for (uint32_t i = 0; i < extras.size(); ++i) {
      out.callDetails.extras[i] = extras[i];
    }
  }

  convertServiceStatusInfoList(out.callDetails.localAbility, in.getLocalAbility());

  convertServiceStatusInfoList(out.callDetails.peerAbility, in.getPeerAbility());

  if (in.hasCallSubState()) {
    out.callDetails.callSubstate = in.getCallSubState();
  } else {
    out.callDetails.callSubstate = INT32_MAX;
  }

  if (in.hasMediaId()) {
    out.callDetails.mediaId = in.getMediaId();
  } else {
    out.callDetails.mediaId = INT32_MAX;
  }

  if (in.hasCauseCode()) {
    // CallModifiedCause enum values are in line with telephony expectation
    out.callDetails.causeCode = static_cast<uint32_t>(in.getCauseCode());
  } else {
    out.callDetails.causeCode = INT32_MAX;
  }

  if (in.hasRttMode()) {
    out.callDetails.rttMode = convertRttMode(in.getRttMode());
  } else {
    out.callDetails.rttMode = RttMode::RTT_MODE_INVALID;
  }

  if (in.hasSipAlternateUri() && in.getSipAlternateUri().size() > 0) {
    out.callDetails.sipAlternateUri = in.getSipAlternateUri();
  }

  //=====

  out.hasFailCause = in.hasCallFailCauseResponse();
  if (in.hasCallFailCauseResponse()) {
    convertCallFailCauseResponse(out.failCause, in.getCallFailCauseResponse());
  }

  out.hasIsEncrypted = in.hasIsEncrypted();
  if (in.hasIsEncrypted()) {
    out.isEncrypted = in.getIsEncrypted();
  }

  out.hasIsCalledPartyRinging = in.hasIsCalledPartyRinging();
  if (in.hasIsCalledPartyRinging()) {
    out.isCalledPartyRinging = in.getIsCalledPartyRinging();
  }

  out.hasIsVideoConfSupported = in.hasIsVideoConfSupported();
  if (in.hasIsVideoConfSupported()) {
    out.isVideoConfSupported = in.getIsVideoConfSupported();
  }

  if (in.hasHistoryInfo() && in.getHistoryInfo().size() > 0) {
    out.historyInfo = in.getHistoryInfo().c_str();
  }

  if (in.hasVerstatInfo()) {
    convertVerstatInfo(out.verstatInfo, in.getVerstatInfo());
  } else {
    out.verstatInfo = {.canMarkUnwantedCall = false,
                       .verificationStatus = V1_2::VerificationStatus::STATUS_VALIDATION_NONE};
  }
  return true;
}

bool convertCallInfoList(hidl_vec<V1_3::CallInfo> &out,
                         const std::vector<qcril::interfaces::CallInfo> &in) {
  uint32_t count = in.size();
  out.resize(count);
  for (uint32_t i = 0; i < count; ++i) {
    out[i] = {};
    convertCallInfo(out[i], in[i]);
  }
  return true;
}

bool convertCallFailCauseResponse(std::shared_ptr<qcril::interfaces::CallFailCauseResponse> &out,
                                  const V1_3::CallFailCauseResponse &in) {
  if (out == nullptr) {
    return false;
  }

  if (in.failCause != V1_3::CallFailCause::CALL_FAIL_INVALID) {
    out->setFailCause(convertCallFailCause(in.failCause));
  }
  // --FR47445-TODO--: check this?
  const uint32_t errorInfoLen = in.errorinfo.size();
  if (errorInfoLen > 0) {
    std::string errorInfo((const char *)in.errorinfo.data(), errorInfoLen);
    out->setExtendedFailCause(std::stoul(errorInfo));
  }
  const uint32_t errorStringLen = in.networkErrorString.size();
  if (errorStringLen > 0) {
    out->setNetworkErrorString(in.networkErrorString.c_str());
  }
  if (in.hasErrorDetails) {
    std::shared_ptr<qcril::interfaces::SipErrorInfo> errorDetails =
        std::shared_ptr<qcril::interfaces::SipErrorInfo>();
    bool ret = convertSipErrorInfo(errorDetails, in.errorDetails);
    if (ret) {
      out->setErrorDetails(errorDetails);
    }
  }

  return true;
}

bool convertCallFailCauseResponse(
    V1_3::CallFailCauseResponse &out,
    const std::shared_ptr<qcril::interfaces::CallFailCauseResponse> &in) {
  if (in == nullptr) {
    return false;
  }
  if (in->hasFailCause()) {
    out.failCause = convertCallFailCause(in->getFailCause());
  }
  out.errorinfo.resize(0);
  if (in->hasExtendedFailCause()) {
    std::string extFailCauseStr = std::to_string(in->getExtendedFailCause());
    if (!extFailCauseStr.empty()) {
      out.errorinfo.resize(extFailCauseStr.size());
      for (size_t i = 0; i < extFailCauseStr.size(); i++) {
        out.errorinfo[i] = extFailCauseStr[i];
      }
    }
  }
  if (in->hasNetworkErrorString() && !in->getNetworkErrorString().empty()) {
    out.networkErrorString = in->getNetworkErrorString().c_str();
  }
  if (in->hasErrorDetails() && in->getErrorDetails() != nullptr) {
    out.hasErrorDetails = convertSipErrorInfo(out.errorDetails, in->getErrorDetails());
  }
  return true;
}

bool convertClirInfo(ClirInfo &out, const std::shared_ptr<qcril::interfaces::ClirInfo> &in) {
  if (in == nullptr) {
    return false;
  }
  out.paramM = in->hasPresentation() ? in->getPresentation() : INT32_MAX;
  out.paramN = in->hasAction() ? in->getAction() : INT32_MAX;
  return true;
}

bool convertClirInfo(std::shared_ptr<qcril::interfaces::ClirInfo> &out, const ClirInfo &in) {
  if (out == nullptr) {
    return false;
  }
  if (in.paramM != INT32_MAX) {
    out->setPresentation(in.paramM);
  }
  if (in.paramN != INT32_MAX) {
    out->setAction(in.paramN);
  }
  return true;
}

void convertSuppServiceStatusV13ToV10(const V1_3::SuppServiceStatus& suppServiceStatusV13,
    V1_0::SuppServiceStatus& suppServiceStatusV10) {
    suppServiceStatusV10.status = suppServiceStatusV13.status;
    suppServiceStatusV10.facilityType = suppServiceStatusV13.facilityType;
    suppServiceStatusV10.failureCause = suppServiceStatusV13.failureCause;

    uint32_t count = suppServiceStatusV13.cbNumListInfo.size();
    suppServiceStatusV10.cbNumListInfo.resize(count);
    for (uint32_t i = 0; i < count; i++) {
        suppServiceStatusV10.cbNumListInfo[i] = {};
        auto &in = suppServiceStatusV13.cbNumListInfo[i];
        auto &out = suppServiceStatusV10.cbNumListInfo[i];
        out.serviceClass = in.serviceClass;
        uint32_t num_count = in.cbNumInfo.size();
        out.cbNumInfo.resize(num_count);
        for (uint32_t j = 0; j < num_count; j++) {
            out.cbNumInfo[j] = {};
            out.cbNumInfo[j].status = in.cbNumInfo[j].status;
            out.cbNumInfo[j].number = in.cbNumInfo[j].number;
        }
    }

    suppServiceStatusV10.hasErrorDetails = suppServiceStatusV13.hasErrorDetails;
    suppServiceStatusV10.errorDetails.errorCode =
            suppServiceStatusV13.errorDetails.errorCode;
    suppServiceStatusV10.errorDetails.errorString =
            suppServiceStatusV13.errorDetails.errorString;
}

void convertColrInfoV13ToV10(const V1_3::ColrInfo& colrInfoV13,
    V1_0::ColrInfo& colrInfoV10) {
    colrInfoV10.presentation = colrInfoV13.presentation;
    colrInfoV10.hasErrorDetails = colrInfoV13.hasErrorDetails;
    colrInfoV10.errorDetails.errorCode = colrInfoV13.errorDetails.errorCode;
    colrInfoV10.errorDetails.errorString.setToExternal(
            colrInfoV13.errorDetails.errorString.c_str(),
            colrInfoV13.errorDetails.errorString.size());
}

bool convertColrInfo(V1_3::ColrInfo &out, const std::shared_ptr<qcril::interfaces::ColrInfo> &in) {
  if (in == nullptr) {
    return false;
  }
  out.status = convertServiceClassStatus(in->getStatus());
  out.provisionStatus = convertServiceClassProvisionStatus(in->getProvisionStatus());
  out.presentation = convertIpPresentation(in->getPresentation());
  if (in->hasErrorDetails() && in->getErrorDetails() != nullptr) {
    out.hasErrorDetails = convertSipErrorInfo(out.errorDetails, in->getErrorDetails());
  }
  return true;
}

bool convertClipInfo(ClipProvisionStatus &out, const std::shared_ptr<qcril::interfaces::ClipInfo> &in) {
  if (in == nullptr) {
    return false;
  }
  out.clipStatus = convertClipStatus(in->getClipStatus());
  if (in->hasErrorDetails() && in->getErrorDetails() != nullptr) {
    out.hasErrorDetails = convertSipErrorInfo(out.errorDetails, in->getErrorDetails());
  }
  return true;
}

bool convertNumInfo(CbNumInfo &out, const qcril::interfaces::CallBarringNumbersInfo &in) {
  out.status = ServiceClassStatus::INVALID;
  if (in.hasStatus()) {
    out.status = convertServiceClassStatus(in.getStatus());
  }
  if (in.hasNumber() && in.getNumber().size() > 0) {
    out.number = in.getNumber().c_str();
  }
  return true;
}
bool convertCbNumListInfo(CbNumListInfo &out,
                          const qcril::interfaces::CallBarringNumbersListInfo &in) {
  out.serviceClass = INT32_MAX;
  if (in.hasServiceClass()) {
    out.serviceClass = in.getServiceClass();
  }
  if (in.getCallBarringNumbersInfo().size() > 0) {
    uint32_t size = in.getCallBarringNumbersInfo().size();
    out.cbNumInfo.resize(size);
    for (uint32_t i = 0; i < size; i++) {
      convertNumInfo(out.cbNumInfo[i], in.getCallBarringNumbersInfo()[i]);
    }
  }
  return true;
}
bool convertSuppServiceStatus(V1_3::SuppServiceStatus &out,
                              const std::shared_ptr<qcril::interfaces::SuppServiceStatusInfo> &in) {
  if (in == nullptr) {
    return false;
  }
  out.status = ServiceClassStatus::INVALID;
  if (in->hasStatus()) {
    out.status = convertServiceClassStatus(in->getStatus());
  }
  out.provisionStatus = V1_3::ServiceClassProvisionStatus::INVALID;
  if (in->hasProvisionStatus()) {
    out.provisionStatus = convertServiceClassProvisionStatus(in->getProvisionStatus());
  }
  out.facilityType = FacilityType::FACILITY_INVALID;
  if (in->hasFacilityType()) {
    out.facilityType = convertFacilityType(in->getFacilityType());
  }
  //in->hasCallBarringNumbersListInfo()
  if (in->getCallBarringNumbersListInfo().size() > 0) {
    uint32_t size = in->getCallBarringNumbersListInfo().size();
    out.cbNumListInfo.resize(size);
    for (uint32_t i = 0; i < size; i++) {
      convertCbNumListInfo(out.cbNumListInfo[i], in->getCallBarringNumbersListInfo()[i]);
    }
  }
  if (in->hasErrorDetails() && in->getErrorDetails() != nullptr) {
    out.hasErrorDetails = convertSipErrorInfo(out.errorDetails, in->getErrorDetails());
  }
  return true;
}

bool isServiceTypeCfQuery(SsServiceType serType, SsRequestType reqType) {
  if ((reqType == SsRequestType::SS_INTERROGATION) &&
      (serType == SsServiceType::SS_CFU ||
       serType == SsServiceType::SS_CF_BUSY ||
       serType == SsServiceType::SS_CF_NO_REPLY ||
       serType == SsServiceType::SS_CF_NOT_REACHABLE ||
       serType == SsServiceType::SS_CF_ALL ||
       serType == SsServiceType::SS_CF_ALL_CONDITIONAL ||
       serType == SsServiceType::SS_CFUT)) {
    return true;
  }
  return false;
}

bool isServiceTypeIcbQueryWithNumber(SsServiceType serType, SsRequestType reqType) {
  if ((reqType == SsRequestType::SS_INTERROGATION) &&
      (serType == SsServiceType::SS_INCOMING_BARRING_DN ||
       serType == SsServiceType::SS_INCOMING_BARRING_ANONYMOUS)) {
    return true;
  }
  return false;
}

void convertCallListV13ToV12(hidl_vec<V1_3::CallInfo>& callListV13,
        hidl_vec<V1_2::CallInfo>& callListV12) {
    uint32_t count = callListV13.size();
    callListV12.resize(count);
    for (uint32_t i = 0; i < count; i++) {
        callListV12[i] = {};
        convertCallV13ToV12(callListV13[i], callListV12[i]);
    }
}

/*
 *   @brief
 *   Utility to convert CallInfo from Vesion 1.1
 *   to Version 1.0 in case of Radio HAL fallback
 *
 *   This conversion is based on the assumption that callInfoV12 does
 *   not go out of scope before callInfoV11
 */
void convertCallV13ToV12(V1_3::CallInfo& callInfoV13,
        V1_2::CallInfo& callInfoV12) {
    callInfoV12.state = callInfoV13.state;
    callInfoV12.index = callInfoV13.index;
    callInfoV12.toa = callInfoV13.toa;
    callInfoV12.hasIsMpty = callInfoV13.hasIsMpty;
    callInfoV12.isMpty = callInfoV13.isMpty;
    callInfoV12.hasIsMT = callInfoV13.hasIsMT;
    callInfoV12.isMT = callInfoV13.isMT;
    callInfoV12.als = callInfoV13.als;
    callInfoV12.hasIsVoice = callInfoV13.hasIsVoice;
    callInfoV12.isVoice = callInfoV13.isVoice;
    callInfoV12.hasIsVoicePrivacy = callInfoV13.hasIsVoicePrivacy;
    callInfoV12.isVoicePrivacy = callInfoV13.isVoicePrivacy;
    callInfoV12.number.setToExternal(callInfoV13.number.c_str(),
            callInfoV13.number.size());
    callInfoV12.numberPresentation = callInfoV13.numberPresentation;
    callInfoV12.name.setToExternal(callInfoV13.name.c_str(),
            callInfoV13.name.size());
    callInfoV12.namePresentation = callInfoV13.namePresentation;

    callInfoV12.hasCallDetails = callInfoV13.hasCallDetails;
    CallDetails& callDetailsV13 = callInfoV13.callDetails;
    CallDetails& callDetailsV12 = callInfoV12.callDetails;
    callDetailsV12.callType = callDetailsV13.callType;
    callDetailsV12.callDomain = callDetailsV13.callDomain;
    callDetailsV12.extrasLength = callDetailsV13.extrasLength;
    callDetailsV12.extras.setToExternal(callDetailsV13.extras.data(),
            callDetailsV13.extras.size());
    callDetailsV12.localAbility.setToExternal(
            callDetailsV13.localAbility.data(),
            callDetailsV13.localAbility.size());
    callDetailsV12.peerAbility.setToExternal(
            callDetailsV13.peerAbility.data(),
            callDetailsV13.peerAbility.size());
    callDetailsV12.callSubstate = callDetailsV13.callSubstate;
    callDetailsV12.mediaId = callDetailsV13.mediaId;
    callDetailsV12.causeCode = callDetailsV13.causeCode;
    callDetailsV12.rttMode = callDetailsV13.rttMode;
    callDetailsV12.sipAlternateUri.setToExternal(
           callDetailsV13.sipAlternateUri.c_str(),
           callDetailsV13.sipAlternateUri.size());

    callInfoV12.hasFailCause = callInfoV13.hasFailCause;
    V1_3::CallFailCauseResponse& callFailV13 = callInfoV13.failCause;
    V1_2::CallFailCauseResponse& callFailV12 = callInfoV12.failCause;
    callFailV12.failCause = convertCallFailCauseV13ToV12(callFailV13.failCause);
    callFailV12.errorinfo.setToExternal(callFailV13.errorinfo.data(),
            callFailV13.errorinfo.size());
    callFailV12.networkErrorString.setToExternal(callFailV13.networkErrorString.c_str(),
        callFailV13.networkErrorString.size());
    callFailV12.hasErrorDetails = callFailV13.hasErrorDetails;
    callFailV12.errorDetails.errorCode = callFailV13.errorDetails.errorCode;
    callFailV12.errorDetails.errorString.setToExternal(
        callFailV13.errorDetails.errorString.c_str(),
        callFailV13.errorDetails.errorString.size());

    callInfoV12.hasIsEncrypted = callInfoV13.hasIsEncrypted;
    callInfoV12.isEncrypted = callInfoV13.isEncrypted;
    callInfoV12.hasIsCalledPartyRinging = callInfoV13.hasIsCalledPartyRinging;
    callInfoV12.isCalledPartyRinging = callInfoV13.isCalledPartyRinging;
    callInfoV12.hasIsVideoConfSupported = callInfoV13.hasIsVideoConfSupported;
    callInfoV12.isVideoConfSupported = callInfoV13.isVideoConfSupported;
    callInfoV12.historyInfo.setToExternal(callInfoV13.historyInfo.c_str(),
        callInfoV12.historyInfo.size());
}

V1_2::CallFailCause convertCallFailCauseV13ToV12(const V1_3::CallFailCause& failCause) {
    switch (failCause) {
        //return V1.2 CALL_FAIL_ERROR_UNSPECIFIED for all the new fail causes
        //added in V1.3
        case V1_3::CallFailCause::CALL_FAIL_RADIO_OFF:
        case V1_3::CallFailCause::CALL_FAIL_OUT_OF_SERVICE:
        case V1_3::CallFailCause::CALL_FAIL_NO_VALID_SIM:
        case V1_3::CallFailCause::CALL_FAIL_RADIO_INTERNAL_ERROR:
        case V1_3::CallFailCause::CALL_FAIL_NETWORK_RESP_TIMEOUT:
        case V1_3::CallFailCause::CALL_FAIL_NETWORK_REJECT:
        case V1_3::CallFailCause::CALL_FAIL_RADIO_ACCESS_FAILURE:
        case V1_3::CallFailCause::CALL_FAIL_RADIO_LINK_FAILURE:
        case V1_3::CallFailCause::CALL_FAIL_RADIO_LINK_LOST:
        case V1_3::CallFailCause::CALL_FAIL_RADIO_UPLINK_FAILURE:
        case V1_3::CallFailCause::CALL_FAIL_RADIO_SETUP_FAILURE:
        case V1_3::CallFailCause::CALL_FAIL_RADIO_RELEASE_NORMAL:
        case V1_3::CallFailCause::CALL_FAIL_RADIO_RELEASE_ABNORMAL:
        case V1_3::CallFailCause::CALL_FAIL_NETWORK_DETACH:
            return V1_2::CallFailCause::CALL_FAIL_ERROR_UNSPECIFIED;
        default:
            return (V1_2::CallFailCause)failCause;
    }
}

void convertCallListV12ToV11(hidl_vec<V1_2::CallInfo>& callListV12,
        hidl_vec<V1_1::CallInfo>& callListV11) {
    uint32_t count = callListV12.size();
    callListV11.resize(count);
    for (uint32_t i = 0; i < count; i++) {
        callListV11[i] = {};
        convertCallV12ToV11(callListV12[i], callListV11[i]);
    }
}

/*
 *   @brief
 *   Utility to convert CallInfo from Vesion 1.1
 *   to Version 1.0 in case of Radio HAL fallback
 *
 *   This conversion is based on the assumption that callInfoV12 does
 *   not go out of scope before callInfoV11
 */
void convertCallV12ToV11(V1_2::CallInfo& callInfoV12,
        V1_1::CallInfo& callInfoV11) {
    callInfoV11.state = callInfoV12.state;
    callInfoV11.index = callInfoV12.index;
    callInfoV11.toa = callInfoV12.toa;
    callInfoV11.hasIsMpty = callInfoV12.hasIsMpty;
    callInfoV11.isMpty = callInfoV12.isMpty;
    callInfoV11.hasIsMT = callInfoV12.hasIsMT;
    callInfoV11.isMT = callInfoV12.isMT;
    callInfoV11.als = callInfoV12.als;
    callInfoV11.hasIsVoice = callInfoV12.hasIsVoice;
    callInfoV11.isVoice = callInfoV12.isVoice;
    callInfoV11.hasIsVoicePrivacy = callInfoV12.hasIsVoicePrivacy;
    callInfoV11.isVoicePrivacy = callInfoV12.isVoicePrivacy;
    callInfoV11.number.setToExternal(callInfoV12.number.c_str(),
            callInfoV12.number.size());
    callInfoV11.numberPresentation = callInfoV12.numberPresentation;
    callInfoV11.name.setToExternal(callInfoV12.name.c_str(),
            callInfoV12.name.size());
    callInfoV11.namePresentation = callInfoV12.namePresentation;

    callInfoV11.hasCallDetails = callInfoV12.hasCallDetails;
    CallDetails& callDetailsV12 = callInfoV12.callDetails;
    CallDetails& callDetailsV11 = callInfoV11.callDetails;
    callDetailsV11.callType = callDetailsV12.callType;
    callDetailsV11.callDomain = callDetailsV12.callDomain;
    callDetailsV11.extrasLength = callDetailsV12.extrasLength;
    callDetailsV11.extras.setToExternal(callDetailsV12.extras.data(),
            callDetailsV12.extras.size());
    callDetailsV11.localAbility.setToExternal(
            callDetailsV12.localAbility.data(),
            callDetailsV12.localAbility.size());
    callDetailsV11.peerAbility.setToExternal(
            callDetailsV12.peerAbility.data(),
            callDetailsV12.peerAbility.size());
    callDetailsV11.callSubstate = callDetailsV12.callSubstate;
    callDetailsV11.mediaId = callDetailsV12.mediaId;
    callDetailsV11.causeCode = callDetailsV12.causeCode;
    callDetailsV11.rttMode = callDetailsV12.rttMode;
    callDetailsV11.sipAlternateUri.setToExternal(
           callDetailsV12.sipAlternateUri.c_str(),
           callDetailsV12.sipAlternateUri.size());

    callInfoV11.hasFailCause = callInfoV12.hasFailCause;
    V1_2::CallFailCauseResponse& callFailV12 = callInfoV12.failCause;
    V1_1::CallFailCauseResponse& callFailV11 = callInfoV11.failCause;
    callFailV11.failCause = convertCallFailCauseV12ToV11(callFailV12.failCause);
    callFailV11.errorinfo.setToExternal(callFailV12.errorinfo.data(),
            callFailV12.errorinfo.size());
    callFailV11.networkErrorString.setToExternal(callFailV12.networkErrorString.c_str(),
        callFailV12.networkErrorString.size());
    callFailV11.hasErrorDetails = callFailV12.hasErrorDetails;
    callFailV11.errorDetails.errorCode = callFailV12.errorDetails.errorCode;
    callFailV11.errorDetails.errorString.setToExternal(
        callFailV12.errorDetails.errorString.c_str(),
        callFailV12.errorDetails.errorString.size());

    callInfoV11.hasIsEncrypted = callInfoV12.hasIsEncrypted;
    callInfoV11.isEncrypted = callInfoV12.isEncrypted;
    callInfoV11.hasIsCalledPartyRinging = callInfoV12.hasIsCalledPartyRinging;
    callInfoV11.isCalledPartyRinging = callInfoV12.isCalledPartyRinging;
    callInfoV11.hasIsVideoConfSupported = callInfoV12.hasIsVideoConfSupported;
    callInfoV11.isVideoConfSupported = callInfoV12.isVideoConfSupported;
    callInfoV11.historyInfo.setToExternal(callInfoV12.historyInfo.c_str(),
        callInfoV12.historyInfo.size());
}

V1_1::CallFailCause convertCallFailCauseV12ToV11(const V1_2::CallFailCause& failCause) {
    switch (failCause) {
        //return V1.1 CALL_FAIL_ERROR_UNSPECIFIED for all the new fail causes
        //added in V1.2
        case V1_2::CallFailCause::CALL_FAIL_RADIO_OFF:
        case V1_2::CallFailCause::CALL_FAIL_OUT_OF_SERVICE:
        case V1_2::CallFailCause::CALL_FAIL_NO_VALID_SIM:
        case V1_2::CallFailCause::CALL_FAIL_RADIO_INTERNAL_ERROR:
        case V1_2::CallFailCause::CALL_FAIL_NETWORK_RESP_TIMEOUT:
        case V1_2::CallFailCause::CALL_FAIL_NETWORK_REJECT:
        case V1_2::CallFailCause::CALL_FAIL_RADIO_ACCESS_FAILURE:
        case V1_2::CallFailCause::CALL_FAIL_RADIO_LINK_FAILURE:
        case V1_2::CallFailCause::CALL_FAIL_RADIO_LINK_LOST:
        case V1_2::CallFailCause::CALL_FAIL_RADIO_UPLINK_FAILURE:
        case V1_2::CallFailCause::CALL_FAIL_RADIO_SETUP_FAILURE:
        case V1_2::CallFailCause::CALL_FAIL_RADIO_RELEASE_NORMAL:
        case V1_2::CallFailCause::CALL_FAIL_RADIO_RELEASE_ABNORMAL:
        case V1_2::CallFailCause::CALL_FAIL_NETWORK_DETACH:
        case V1_2::CallFailCause::CALL_FAIL_NO_CSFB_IN_CS_ROAM:
            return V1_1::CallFailCause::CALL_FAIL_ERROR_UNSPECIFIED;
        default:
            return (V1_1::CallFailCause)failCause;
    }
}

void convertCallListV11ToV10(hidl_vec<V1_1::CallInfo>& callListV11,
        hidl_vec<V1_0::CallInfo>& callListV10) {
    uint32_t count = callListV11.size();
    callListV10.resize(count);
    for (uint32_t i = 0; i < count; i++) {
        callListV10[i] = {};
        convertCallV11ToV10(callListV11[i], callListV10[i]);
    }
 }

/*
 *   @brief
 *   Utility to convert CallInfo from Vesion 1.1
 *   to Version 1.0 in case of Radio HAL fallback
 *
 *   This conversion is based on the assumption that callInfoV11 does
 *   not go out of scope before callInfoV10
 */
void convertCallV11ToV10(V1_1::CallInfo& callInfoV11,
        V1_0::CallInfo& callInfoV10) {
    callInfoV10.state = callInfoV11.state;
    callInfoV10.index = callInfoV11.index;
    callInfoV10.toa = callInfoV11.toa;
    callInfoV10.hasIsMpty = callInfoV11.hasIsMpty;
    callInfoV10.isMpty = callInfoV11.isMpty;
    callInfoV10.hasIsMT = callInfoV11.hasIsMT;
    callInfoV10.isMT = callInfoV11.isMT;
    callInfoV10.als = callInfoV11.als;
    callInfoV10.hasIsVoice = callInfoV11.hasIsVoice;
    callInfoV10.isVoice = callInfoV11.isVoice;
    callInfoV10.hasIsVoicePrivacy = callInfoV11.hasIsVoicePrivacy;
    callInfoV10.isVoicePrivacy = callInfoV11.isVoicePrivacy;
    callInfoV10.number.setToExternal(callInfoV11.number.c_str(),
            callInfoV11.number.size());
    callInfoV10.numberPresentation = callInfoV11.numberPresentation;
    callInfoV10.name.setToExternal(callInfoV11.name.c_str(),
            callInfoV11.name.size());
    callInfoV10.namePresentation = callInfoV11.namePresentation;

    callInfoV10.hasCallDetails = callInfoV11.hasCallDetails;
    CallDetails& callDetailsV11 = callInfoV11.callDetails;
    CallDetails& callDetailsV10 = callInfoV10.callDetails;
    callDetailsV10.callType = callDetailsV11.callType;
    callDetailsV10.callDomain = callDetailsV11.callDomain;
    callDetailsV10.extrasLength = callDetailsV11.extrasLength;
    callDetailsV10.extras.setToExternal(callDetailsV11.extras.data(),
            callDetailsV11.extras.size());
    callDetailsV10.localAbility.setToExternal(
            callDetailsV11.localAbility.data(),
            callDetailsV11.localAbility.size());
    callDetailsV10.peerAbility.setToExternal(
            callDetailsV11.peerAbility.data(),
            callDetailsV11.peerAbility.size());
    callDetailsV10.callSubstate = callDetailsV11.callSubstate;
    callDetailsV10.mediaId = callDetailsV11.mediaId;
    callDetailsV10.causeCode = callDetailsV11.causeCode;
    callDetailsV10.rttMode = callDetailsV11.rttMode;
    callDetailsV10.sipAlternateUri.setToExternal(
           callDetailsV11.sipAlternateUri.c_str(),
           callDetailsV11.sipAlternateUri.size());

    callInfoV10.hasFailCause = callInfoV11.hasFailCause;
    V1_1::CallFailCauseResponse& callFailV11 = callInfoV11.failCause;
    V1_0::CallFailCauseResponse& callFailV10 = callInfoV10.failCause;
    callFailV10.failCause = convertCallFailCauseToV10(callFailV11.failCause);
    callFailV10.errorinfo.setToExternal(callFailV11.errorinfo.data(),
            callFailV11.errorinfo.size());
    callFailV10.networkErrorString.setToExternal(callFailV11.networkErrorString.c_str(),
        callFailV11.networkErrorString.size());
    callFailV10.hasErrorDetails = callFailV11.hasErrorDetails;
    callFailV10.errorDetails.errorCode = callFailV11.errorDetails.errorCode;
    callFailV10.errorDetails.errorString.setToExternal(
        callFailV11.errorDetails.errorString.c_str(),
        callFailV11.errorDetails.errorString.size());

    callInfoV10.hasIsEncrypted = callInfoV11.hasIsEncrypted;
    callInfoV10.isEncrypted = callInfoV11.isEncrypted;
    callInfoV10.hasIsCalledPartyRinging = callInfoV11.hasIsCalledPartyRinging;
    callInfoV10.isCalledPartyRinging = callInfoV11.isCalledPartyRinging;
    callInfoV10.hasIsVideoConfSupported = callInfoV11.hasIsVideoConfSupported;
    callInfoV10.isVideoConfSupported = callInfoV11.isVideoConfSupported;
    callInfoV10.historyInfo.setToExternal(callInfoV11.historyInfo.c_str(),
        callInfoV11.historyInfo.size());
}

V1_0::CallFailCause convertCallFailCauseToV10(const V1_1::CallFailCause& failCause) {
    return (failCause == V1_1::CallFailCause::CALL_FAIL_SIP_ALTERNATE_EMERGENCY_CALL)?
        V1_0::CallFailCause::CALL_FAIL_ERROR_UNSPECIFIED :
            (V1_0::CallFailCause)failCause;
}

V1_1::CallFailCause migrateCallFailCauseToV11(CallFailCause failCause) {
    //We need this check to make sure that an invalid call fail cause in
    //V10 gets mapped to an invalid call fail cause in V11. Otherwise it
    //might get mapped to a valid fail cause.
    return (failCause == V1_0::CallFailCause::CALL_FAIL_INVALID) ?
        V1_1::CallFailCause::CALL_FAIL_INVALID :
            static_cast<V1_1::CallFailCause>(failCause);
}


void migrateHangup(const V1_0::HangupRequestInfo& from,
        V1_1::HangupRequestInfo& to) {
    to.connIndex = from.connIndex;
    to.hasMultiParty = from.hasMultiParty;
    to.multiParty = from.multiParty;
    to.connUri = from.connUri;
    to.conf_id = from.conf_id;
    to.hasFailCauseResponse = from.hasFailCauseResponse;
    to.failCauseResponse.failCause =
            migrateCallFailCauseToV11(from.failCauseResponse.failCause);
    to.failCauseResponse.errorinfo = from.failCauseResponse.errorinfo;
    to.failCauseResponse.networkErrorString = from.failCauseResponse.networkErrorString;
    to.failCauseResponse.hasErrorDetails = from.failCauseResponse.hasErrorDetails;
    to.failCauseResponse.errorDetails.errorCode =
            from.failCauseResponse.errorDetails.errorCode;
    to.failCauseResponse.errorDetails.errorString =
            from.failCauseResponse.errorDetails.errorString;
}

V1_2::CallFailCause migrateCallFailCauseToV12(V1_1::CallFailCause failCause) {
    //We need this check to make sure that an invalid call fail cause in
    //V11 gets mapped to an invalid call fail cause in V12. Otherwise it
    //might get mapped to a valid fail cause.
    return (failCause == V1_1::CallFailCause::CALL_FAIL_INVALID) ?
        V1_2::CallFailCause::CALL_FAIL_INVALID :
            static_cast<V1_2::CallFailCause>(failCause);
}

void  migrateHangup(const V1_1::HangupRequestInfo& from,
        V1_2::HangupRequestInfo& to) {
    to.connIndex = from.connIndex;
    to.hasMultiParty = from.hasMultiParty;
    to.multiParty = from.multiParty;
    to.connUri = from.connUri;
    to.conf_id = from.conf_id;
    to.hasFailCauseResponse = from.hasFailCauseResponse;
    to.failCauseResponse.failCause =
            migrateCallFailCauseToV12(from.failCauseResponse.failCause);
    to.failCauseResponse.errorinfo = from.failCauseResponse.errorinfo;
    to.failCauseResponse.networkErrorString = from.failCauseResponse.networkErrorString;
    to.failCauseResponse.hasErrorDetails = from.failCauseResponse.hasErrorDetails;
    to.failCauseResponse.errorDetails.errorCode =
            from.failCauseResponse.errorDetails.errorCode;
    to.failCauseResponse.errorDetails.errorString =
            from.failCauseResponse.errorDetails.errorString;
}

V1_3::CallFailCause migrateCallFailCauseToV13(V1_2::CallFailCause failCause) {
    //We need this check to make sure that an invalid call fail cause in
    //V12 gets mapped to an invalid call fail cause in V13. Otherwise it
    //might get mapped to a valid fail cause.
    return (failCause == V1_2::CallFailCause::CALL_FAIL_INVALID) ?
        V1_3::CallFailCause::CALL_FAIL_INVALID :
            static_cast<V1_3::CallFailCause>(failCause);
}

void  migrateHangup(const V1_2::HangupRequestInfo& from,
        V1_3::HangupRequestInfo& to) {
    to.connIndex = from.connIndex;
    to.hasMultiParty = from.hasMultiParty;
    to.multiParty = from.multiParty;
    to.connUri = from.connUri;
    to.conf_id = from.conf_id;
    to.hasFailCauseResponse = from.hasFailCauseResponse;
    to.failCauseResponse.failCause =
            migrateCallFailCauseToV13(from.failCauseResponse.failCause);
    to.failCauseResponse.errorinfo = from.failCauseResponse.errorinfo;
    to.failCauseResponse.networkErrorString = from.failCauseResponse.networkErrorString;
    to.failCauseResponse.hasErrorDetails = from.failCauseResponse.hasErrorDetails;
    to.failCauseResponse.errorDetails.errorCode =
            from.failCauseResponse.errorDetails.errorCode;
    to.failCauseResponse.errorDetails.errorString =
            from.failCauseResponse.errorDetails.errorString;
}

void  migrateAddressInfo(const V1_0::AddressInfo& from,
        V1_2::AddressInfo& to) {
    to.city = from.city;
    to.state = from.state;
    to.country = from.country;
    to.postalCode = from.postalCode;
    to.countryCode = from.countryCode;
    // not filling street and houseNumber fields so that they contain default values.
}

void migrateColrInfo(const V1_0::ColrInfo& from,
            V1_3::ColrInfo& to) {
    to.status = ServiceClassStatus::INVALID;
    to.provisionStatus = V1_3::ServiceClassProvisionStatus::INVALID;
    to.presentation = from.presentation;
    to.hasErrorDetails = from.hasErrorDetails;
    to.errorDetails.errorCode = from.errorDetails.errorCode;
    to.errorDetails.errorString = from.errorDetails.errorString;
}


bool convertImsSubConfigInfo(ImsSubConfigInfo& subConfigInfo,
  qcril::interfaces::ImsSubConfigInfo &in) {
  if (in.hasSimultStackCount()) {
    subConfigInfo.simultStackCount = in.getSimultStackCount();
  } else {
    subConfigInfo.simultStackCount = INT32_MAX;
  }
  uint32_t count = 0;
  if (in.hasCount()) {
    count = in.getCount();
  }
  subConfigInfo.imsStackEnabled.resize(count);
  vector<uint8_t> subStatus = in.getImsStackEnabled();
  for(uint32_t i = 0; i < count; ++i) {
     subConfigInfo.imsStackEnabled[i] = static_cast<bool>(subStatus[i]);
  }
#ifndef QMI_RIL_UTF
  std::string logStr = toString(subConfigInfo);
  QCRIL_LOG_INFO("subConfigInfo = %s", logStr.c_str());
#endif
  return true;
}

void readGeoLocationInfo(double lat, double lon, const V1_2::AddressInfo& in,
  qcril::interfaces::GeoLocationInfo &out) {

  out.setLatitude(lat);
  out.setLongitude(lon);
  qcril::interfaces::AddressInfo &addInfo = out.getAddressInfoRef();
  if (!in.city.empty()) {
    addInfo.setCity(in.city.c_str());
  }
  if (!in.state.empty()) {
    addInfo.setState(in.state.c_str());
  }
  if (!in.country.empty()) {
    addInfo.setCountry(in.country.c_str());
  }
  if (!in.postalCode.empty()) {
    addInfo.setPostalCode(in.postalCode.c_str());
  }
  if (!in.countryCode.empty()) {
    addInfo.setCountryCode(in.countryCode.c_str());
  }
}

std::shared_ptr<qcril::interfaces::Registration> readRegistration(const RegistrationInfo& in) {
  std::shared_ptr<qcril::interfaces::Registration> out =
  std::make_shared<qcril::interfaces::Registration>();
  if (out) {
    if (in.state != RegState::INVALID) {
      out->setState(convertRegState(in.state));
    }
    if (in.errorCode != INT32_MAX) {
      out->setErrorCode(in.errorCode);
    }

    const uint32_t errorMessageLen = in.errorMessage.size();
    //TODO copy error message
    if (errorMessageLen > 0) {
        out->setErrorMessage(in.errorMessage.c_str());
     }

    if (in.radioTech != RadioTechType::RADIO_TECH_INVALID) {
      out->setRadioTechnology(convertRadioTechType(in.radioTech));
    }

    //TODO copy PAssociatedUris
    const uint32_t pAssociatedUrisLen = in.pAssociatedUris.size();
    if (pAssociatedUrisLen > 0) {
      out->setPAssociatedUris(in.pAssociatedUris.c_str());
    }
  }

  return out;
}

std::shared_ptr<qcril::interfaces::AccessTechnologyStatus> readAccessTechStatus(
    const StatusForAccessTech& in) {
  std::shared_ptr<qcril::interfaces::AccessTechnologyStatus> out =
    std::make_shared<qcril::interfaces::AccessTechnologyStatus>();

  if (out != nullptr) {
    if (in.networkMode != RadioTechType::RADIO_TECH_INVALID) {
      out->setNetworkMode(convertRadioTechType(in.networkMode));
    }
    if (in.status != StatusType::STATUS_INVALID) {
      out->setStatusType(convertStatusType(in.status));
    }
    if (in.restrictCause != INT32_MAX) {
      out->setRestrictCause(in.restrictCause);
    }
    if (in.hasRegistration) {
      out->setRegistration(readRegistration(in.registration));
    }
  }
  return out;
}

void readServiceStatusInfo(const ServiceStatusInfo &in,
                                qcril::interfaces::ServiceStatusInfo &out) {
  if (in.callType != CallType::CALL_TYPE_INVALID) {
    out.setCallType(convertCallType(in.callType));
  }
  if (in.status != StatusType::STATUS_INVALID) {
    out.setStatusType(convertStatusType(in.status));
  }

  //TODO set userdata

  if (in.restrictCause != INT32_MAX) {
    out.setRestrictCause(in.restrictCause);
  }

  const uint32_t accTechStatusLen = in.accTechStatus.size();
  //read first value

  if (accTechStatusLen > 0) {
    out.setAccessTechnologyStatus(readAccessTechStatus(in.accTechStatus[0]));
  }

  if (in.rttMode != RttMode::RTT_MODE_INVALID) {
    out.setRttMode(convertRttMode(in.rttMode));
  }
}

void convertBlockReasonDetails(BlockReasonDetails &outDetails,
  const qcril::interfaces::BlockReasonDetails &in) {
  outDetails.regFailureReasonType = RegFailureReasonType::INVALID;
  if (in.hasRegFailureReasonType()) {
    outDetails.regFailureReasonType = convertProtoToHidlRegFailureReasonType(
         in.getRegFailureReasonType());
    outDetails.regFailureReason = in.hasRegFailureReason() ?
         in.getRegFailureReason() : INT32_MAX;
  }
}

void convertRegBlockStatus(BlockStatus &outStatus,
  const qcril::interfaces::BlockStatus &in) {
  outStatus.blockReason = BlockReasonType::INVALID;
  if (in.hasBlockReason()) {
    outStatus.blockReason = convertBlockReasonType(in.getBlockReason());
  }
  if (in.hasBlockReasonDetails()) {
    outStatus.hasBlockReasonDetails = true;
    convertBlockReasonDetails(outStatus.blockReasonDetails, *(in.getBlockReasonDetails()));
  }
}

void readConfigInfo(const ConfigInfo &inConfig, qcril::interfaces::ConfigInfo &outConfig) {
  //config_item
  qcril::interfaces::ConfigItem item = convertConfigItem(inConfig.item);
  if (item != qcril::interfaces::ConfigItem::INVALID_CONFIG) {
    outConfig.setItem(item);
  }

  if (inConfig.hasBoolValue) {
    outConfig.setBoolValue(inConfig.boolValue);
  }
  if (inConfig.intValue != INT32_MAX) {
    outConfig.setIntValue(inConfig.intValue);
  }
  if (!inConfig.stringValue.empty()) {
    outConfig.setStringValue(inConfig.stringValue.c_str());
  }
  qcril::interfaces::ConfigFailureCause cause = convertConfigErrorCause(inConfig.errorCause);
  if(cause !=  qcril::interfaces::ConfigFailureCause::FAILURE_INVALID) {
    outConfig.setErrorCause(cause);
  }
}

void convertConfigInfo(ConfigInfo &outConfig, const qcril::interfaces::ConfigInfo &inConfig) {
  if (inConfig.hasItem()) {
    outConfig.item = convertConfigItem(inConfig.getItem());
  } else {
    outConfig.item = ConfigItem::CONFIG_ITEM_INVALID;
  }
  outConfig.hasBoolValue = inConfig.hasBoolValue();
  if (inConfig.hasBoolValue()) {
    outConfig.boolValue = inConfig.getBoolValue();
  }
  outConfig.intValue = (inConfig.hasIntValue()) ? inConfig.getIntValue() : INT32_MAX;
  if (inConfig.hasStringValue()) {
    outConfig.stringValue = inConfig.getStringValue().c_str();
  }
  outConfig.errorCause = (inConfig.hasErrorCause()) ?
           convertConfigErrorCause(inConfig.getErrorCause()) :
           ConfigFailureCause::CONFIG_FAILURE_INVALID;

}



#define HIGHER_4BIT(data)   (((data) & 0xf0) >> 4)
#define LOWER_4BIT(data)    ((data) & 0x0f)
#define INVALID_HEX_CHAR    0

static inline uint32_t revertEndian(uint32_t data)
{
    uint32_t result = (data & 0xff000000) >> 24;

    result |= ((data & 0x00ff0000) >> 8);
    result |= ((data & 0x0000ff00) << 8);
    result |= ((data & 0x000000ff) << 24);

    return result;
}

static uint32_t convertCurrentToBigEndian(uint32_t data)
{
    if (!IS_BIG_ENDIAN())
        return revertEndian(data);
    else
        return data;
}

static uint32_t convertBigEndianToCurrent(uint32_t data)
{
    if (!IS_BIG_ENDIAN())
        return revertEndian(data);
    else
        return data;
}

static void getBigEndianArrayFrom32(uint8_t* array32, uint32_t value)
{
    if (!array32) return;

    array32[0] = (value & 0xff000000) >> 24;
    array32[1] = (value & 0x00ff0000) >> 16;
    array32[2] = (value & 0x0000ff00) >> 8;
    array32[3] = (value & 0x000000ff);
}

static inline char IntToHexChar(uint8_t byte_data)
{
    if (byte_data >= 0 && byte_data <= 9)
        return ('0' + byte_data);
    if (byte_data > 9 && byte_data <= 15)
        return ('a' + byte_data - 10);
    return INVALID_HEX_CHAR;
}

bool convertBytesToHexString(const uint8_t* bytePayload, size_t size,
    std::string& str) {

    std::string out;
    for (unsigned int byte_idx = 0; byte_idx < size; byte_idx++)
    {
        char tmp;
        // higher bit is in the earlier index
        if ((tmp = IntToHexChar(HIGHER_4BIT(bytePayload[byte_idx]))) == INVALID_HEX_CHAR)
        {
            return false;
        }
        out.push_back(tmp);
        if ((tmp = IntToHexChar(LOWER_4BIT(bytePayload[byte_idx]))) == INVALID_HEX_CHAR)
        {
            return false;
        }
        out.push_back(tmp);
    }
    str = std::move(out);
    return true;
}

#if 0
static char* convertBytesToHexString(const uint8_t* bytePayload, size_t size)
{
    size_t hex_len = (size << 1) + 1;
    char* hex_buf;

    if ((hex_buf = new char[hex_len]) == nullptr)
        return nullptr;

    unsigned int hex_idx = 0;
    for (unsigned int byte_idx = 0; byte_idx < size; byte_idx++)
    {
        uint8_t tmp;
        // higher bit is in the earlier index
        if ((tmp = IntToHexChar(HIGHER_4BIT(bytePayload[byte_idx]))) == INVALID_HEX_CHAR)
        {
            delete []hex_buf;
            return nullptr;
        }
        hex_buf[hex_idx++] = tmp;
        if ((tmp = IntToHexChar(LOWER_4BIT(bytePayload[byte_idx]))) == INVALID_HEX_CHAR)
        {
            delete []hex_buf;
            return nullptr;
        }
        hex_buf[hex_idx++] = tmp;
    }
    hex_buf[hex_idx] = 0;

    return hex_buf;
}
#endif

static uint8_t convertDtmfToAscii(uint8_t dtmfDigit)
{
    uint8_t asciiDigit;

    switch (dtmfDigit) {
    case  0: asciiDigit = 'D'; break;
    case  1: asciiDigit = '1'; break;
    case  2: asciiDigit = '2'; break;
    case  3: asciiDigit = '3'; break;
    case  4: asciiDigit = '4'; break;
    case  5: asciiDigit = '5'; break;
    case  6: asciiDigit = '6'; break;
    case  7: asciiDigit = '7'; break;
    case  8: asciiDigit = '8'; break;
    case  9: asciiDigit = '9'; break;
    case 10: asciiDigit = '0'; break;
    case 11: asciiDigit = '*'; break;
    case 12: asciiDigit = '#'; break;
    case 13: asciiDigit = 'A'; break;
    case 14: asciiDigit = 'B'; break;
    case 15: asciiDigit = 'C'; break;
    default:
        asciiDigit = 32; // Invalid DTMF code
        break;
    }

    return asciiDigit;
}

bool convertPayloadToCdmaFormat(const uint8_t* payload, size_t size,
    RIL_CDMA_SMS_Message& cdmaMsg)
{
    if (!payload || !size) return false;
    // XXX take care alignment!
    // How to implement data stream in c++?
    uint32_t index = 0;
    // read the first 3 int
    index +=  sizeof(int) * 3;
    if (index > size) return false;
    auto int_buf = reinterpret_cast<const int*>(payload);
    cdmaMsg.uTeleserviceID = convertBigEndianToCurrent(int_buf[0]);
    cdmaMsg.bIsServicePresent = (convertBigEndianToCurrent(int_buf[1]) == 0) ? 0 : 1;
    cdmaMsg.uServicecategory = convertBigEndianToCurrent(int_buf[2]);

    // read bytes to fill sAddress
    if (index + 5 > size) return false;
    cdmaMsg.sAddress.digit_mode = static_cast<RIL_CDMA_SMS_DigitMode>(payload[index++]);
    cdmaMsg.sAddress.number_mode = static_cast<RIL_CDMA_SMS_NumberMode>(payload[index++]);
    cdmaMsg.sAddress.number_type = static_cast<RIL_CDMA_SMS_NumberType>(payload[index++]);
    cdmaMsg.sAddress.number_plan = static_cast<RIL_CDMA_SMS_NumberPlan>(payload[index++]);
    cdmaMsg.sAddress.number_of_digits = payload[index++];
    if (index + cdmaMsg.sAddress.number_of_digits > size)
        return false;
    memcpy(cdmaMsg.sAddress.digits, payload + index,
            cdmaMsg.sAddress.number_of_digits);
    index += cdmaMsg.sAddress.number_of_digits;

    // read bytes to fill sSubAddress
    if (index + 3 > size) return false;
    cdmaMsg.sSubAddress.subaddressType =
        static_cast<RIL_CDMA_SMS_SubaddressType>(payload[index++]);
    cdmaMsg.sSubAddress.odd = (payload[index++] == 0) ? 0 : 1;
    cdmaMsg.sSubAddress.number_of_digits = payload[index++];
    if (index + cdmaMsg.sSubAddress.number_of_digits > size)
        return false;
    memcpy(cdmaMsg.sSubAddress.digits, payload + index,
            cdmaMsg.sSubAddress.number_of_digits);
    index += cdmaMsg.sSubAddress.number_of_digits;

    // read bytes for bear data
    if (index + 1 > size) return false;
    cdmaMsg.uBearerDataLen = payload[index++];
    if (index + cdmaMsg.uBearerDataLen > size) return false;
    memcpy(cdmaMsg.aBearerData, payload + index, cdmaMsg.uBearerDataLen);

    QCRIL_LOG_INFO("payload size = %d, read size = %d", size,
        index + cdmaMsg.uBearerDataLen);

    return true;
}

bool convertCdmaFormatToPseudoPdu(RIL_CDMA_SMS_Message& cdmaMsg, std::vector<uint8_t>& payload)
{
    const uint32_t MESSAGE_TYPE_POINT_TO_POINT = 0x00;
    const uint32_t MESSAGE_TYPE_BROADCAST      = 0x01;
    const uint32_t MESSAGE_TYPE_ACKNOWLEDGE    = 0x02;
    uint32_t messageType;

    if (cdmaMsg.bIsServicePresent) {
        messageType = MESSAGE_TYPE_BROADCAST;
    } else {
        if (cdmaMsg.uTeleserviceID == 0x00) {
            messageType = MESSAGE_TYPE_ACKNOWLEDGE;
        } else {
            messageType = MESSAGE_TYPE_POINT_TO_POINT;
        }
    }
    // fill the first three int
    uint32_t item = convertCurrentToBigEndian(messageType);
    uint8_t* base = reinterpret_cast<uint8_t*>(&item);
    payload.insert(payload.end(), base, base + 4);

    item = convertCurrentToBigEndian(cdmaMsg.uTeleserviceID);
    payload.insert(payload.end(), base, base + 4);

    item = convertCurrentToBigEndian(cdmaMsg.uServicecategory);
    payload.insert(payload.end(), base, base + 4);

    // fill the sAddress
    payload.push_back(cdmaMsg.sAddress.digit_mode);
    payload.push_back(cdmaMsg.sAddress.number_mode);
    payload.push_back(cdmaMsg.sAddress.number_type);
    payload.push_back(cdmaMsg.sAddress.number_plan);
    payload.push_back(cdmaMsg.sAddress.number_of_digits);
    // 4-bit DTMF
    if (cdmaMsg.sAddress.digit_mode == 0) {
        for (int index = 0; index < cdmaMsg.sAddress.number_of_digits; index++)
            payload.push_back(convertDtmfToAscii(cdmaMsg.sAddress.digits[index]));
    } else {
        payload.insert(payload.end(), cdmaMsg.sAddress.digits,
            cdmaMsg.sAddress.digits + cdmaMsg.sAddress.number_of_digits);
    }

    uint8_t array32[4];
    // bearerReply
    getBigEndianArrayFrom32(array32, 1);
    payload.insert(payload.end(), array32, array32 + 4);
    // replySeqNo (not supported)
    payload.push_back(0);
    // errorClass (not supported)
    payload.push_back(0);
    // causeCode (not supported)
    payload.push_back(0);

    // bear Data length
    getBigEndianArrayFrom32(array32, cdmaMsg.uBearerDataLen);
    payload.insert(payload.end(), array32, array32 + 4);
    payload.insert(payload.end(), cdmaMsg.aBearerData,
        cdmaMsg.aBearerData + cdmaMsg.uBearerDataLen);

    return true;
}

#if 0
void freeRILImsSmsFormatPayload(RIL_IMS_SMS_Message* ril_ims_msg)
{
    if (ril_ims_msg) {
        if (ril_ims_msg->tech == RADIO_TECH_3GPP2) {
            if (ril_ims_msg->message.cdmaMessage) {
                delete ril_ims_msg->message.cdmaMessage;
                ril_ims_msg->message.cdmaMessage = nullptr;
            }
        }
        if (ril_ims_msg->tech == RADIO_TECH_3GPP) {
            if (ril_ims_msg->message.gsmMessage) {
                if (ril_ims_msg->message.gsmMessage[0])
                    free(ril_ims_msg->message.gsmMessage[0]); // release strdup memory
                if (ril_ims_msg->message.gsmMessage[1])
                    delete []ril_ims_msg->message.gsmMessage[1];
                delete []ril_ims_msg->message.gsmMessage;
                ril_ims_msg->message.gsmMessage = nullptr;
            }
        }
        delete ril_ims_msg;
    }
}


RIL_IMS_SMS_Message* getRILImsSmsFormatPayload(const V1_2::ImsSmsMessage& imsSms)
{
    auto ims_ril_msg = new RIL_IMS_SMS_Message();
    if (ims_ril_msg == nullptr)
        return nullptr;

    memset(ims_ril_msg, 0, sizeof(*ims_ril_msg));
    ims_ril_msg->retry = (imsSms.shallRetry) ? 1 : 0;
    ims_ril_msg->messageRef = imsSms.messageRef;

    if (imsSms.format == "3gpp") {
        ims_ril_msg->tech = RADIO_TECH_3GPP;
        char** gsm_message = new char*[2];
        if (gsm_message) {
            if (imsSms.smsc.size() != 0)
                // use "free()" to release the memory
                gsm_message[0] = strdup(imsSms.smsc.c_str());
            else
                gsm_message[0] = nullptr;

            gsm_message[1] = convertBytesToHexString(imsSms.pdu.data(), imsSms.pdu.size());
        }
        ims_ril_msg->message.gsmMessage = gsm_message;
        // if no message payload, release the memory
        if (!gsm_message || !gsm_message[1]) {
            freeRILImsSmsFormatPayload(ims_ril_msg);
            return nullptr;
        }
    } else if (imsSms.format == "3gpp2") {
        ims_ril_msg->tech = RADIO_TECH_3GPP2;
        auto cdma_msg = new RIL_CDMA_SMS_Message;
        if (cdma_msg) {
            bool result = convertPayloadToCdmaFormat(imsSms.pdu.data(), imsSms.pdu.size(), *cdma_msg);
            if (!result) {
                QCRIL_LOG_ERROR("Failed to parse CDMA payload");
                delete cdma_msg;
                cdma_msg = nullptr;
            }
        }
        ims_ril_msg->message.cdmaMessage = cdma_msg;
        if (!cdma_msg) {
            freeRILImsSmsFormatPayload(ims_ril_msg);
            return nullptr;
        }
    } else {
        delete ims_ril_msg;
        return nullptr;
    }

    return ims_ril_msg;
}
#endif

qcril::interfaces::DeliverStatus convertHidlToRilSmsAckResult(
    V1_2::ImsSmsDeliverStatusResult smsdeliverResult) {
    switch (smsdeliverResult) {
    case V1_2::ImsSmsDeliverStatusResult::DELIVER_STATUS_OK:
        return qcril::interfaces::DeliverStatus::STATUS_OK;
    case V1_2::ImsSmsDeliverStatusResult::DELIVER_STATUS_ERROR:
        return qcril::interfaces::DeliverStatus::STATUS_ERROR;
    default:
        return qcril::interfaces::DeliverStatus::STATUS_ERROR;
    }
}

std::string getExtra(std::string key, const hidl_vec<hidl_string> &extras) {
  std::string value;
  if (!key.empty()) {
    for (size_t i = 0; i < extras.size(); i++) {
      std::string extra = extras[i];
      if (extra.find(key) != std::string::npos &&
          extra.find("=") != std::string::npos) {
        value = extra.substr(extra.find("=") + 1);
      }
    }
  }
  return value;
}


} // namespace utils
} // namespace V1_0
} // namespace ims
} // namespace radio
} // namespace hardware
} // namespace qti
} // namespace vendor
