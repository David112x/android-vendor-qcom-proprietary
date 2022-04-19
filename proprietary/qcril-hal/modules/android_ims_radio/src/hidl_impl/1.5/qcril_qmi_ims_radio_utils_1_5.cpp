/*===========================================================================
   Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
   All Rights Reserved.
   Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/
#define TAG "RILQ"

#include <framework/Log.h>
#include <modules/android_ims_radio/hidl_impl/1.5/qcril_qmi_ims_radio_utils_1_5.h>
#include <string.h>

using ::android::hardware::hidl_bitfield;

using qcril::interfaces::EmergencyCallRouting;
using qcril::interfaces::RIL_EccCategoryMask;

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_5 {
namespace utils {

V1_5::CallFailCause convertCallFailCause(const qcril::interfaces::CallFailCause &in) {
  V1_5::CallFailCause cfc = V1_5::CallFailCause::CALL_FAIL_INVALID;
  switch (in) {
  case qcril::interfaces::CallFailCause::REJECTED_ELSEWHERE:
    cfc = V1_5::CallFailCause::CALL_FAIL_REJECTED_ELSEWHERE;
    break;
  case qcril::interfaces::CallFailCause::USER_REJECTED_SESSION_MODIFICATION:
    cfc = V1_5::CallFailCause::CALL_FAIL_USER_REJECTED_SESSION_MODIFICATION;
    break;
  case qcril::interfaces::CallFailCause::USER_CANCELLED_SESSION_MODIFICATION:
    cfc = V1_5::CallFailCause::CALL_FAIL_USER_CANCELLED_SESSION_MODIFICATION;
    break;
  case qcril::interfaces::CallFailCause::SESSION_MODIFICATION_FAILED:
    cfc = V1_5::CallFailCause::CALL_FAIL_SESSION_MODIFICATION_FAILED;
    break;
  case qcril::interfaces::CallFailCause::SIP_METHOD_NOT_ALLOWED:
    cfc = V1_5::CallFailCause::CALL_FAIL_SIP_METHOD_NOT_ALLOWED;
    break;
  case qcril::interfaces::CallFailCause::SIP_PROXY_AUTHENTICATION_REQUIRED:
    cfc = V1_5::CallFailCause::CALL_FAIL_SIP_PROXY_AUTHENTICATION_REQUIRED;
    break;
  case qcril::interfaces::CallFailCause::SIP_REQUEST_ENTITY_TOO_LARGE:
    cfc = V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_ENTITY_TOO_LARGE;
    break;
  case qcril::interfaces::CallFailCause::SIP_REQUEST_URI_TOO_LARGE:
    cfc = V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_URI_TOO_LARGE;
    break;
  case qcril::interfaces::CallFailCause::SIP_EXTENSION_REQUIRED:
    cfc = V1_5::CallFailCause::CALL_FAIL_SIP_EXTENSION_REQUIRED;
    break;
  case qcril::interfaces::CallFailCause::SIP_INTERVAL_TOO_BRIEF:
    cfc = V1_5::CallFailCause::CALL_FAIL_SIP_INTERVAL_TOO_BRIEF;
    break;
  case qcril::interfaces::CallFailCause::SIP_CALL_OR_TRANS_DOES_NOT_EXIST:
    cfc = V1_5::CallFailCause::CALL_FAIL_SIP_CALL_OR_TRANS_DOES_NOT_EXIST;
    break;
  case qcril::interfaces::CallFailCause::SIP_LOOP_DETECTED:
    cfc = V1_5::CallFailCause::CALL_FAIL_SIP_LOOP_DETECTED;
    break;
  case qcril::interfaces::CallFailCause::SIP_TOO_MANY_HOPS:
    cfc = V1_5::CallFailCause::CALL_FAIL_SIP_TOO_MANY_HOPS;
    break;
  case qcril::interfaces::CallFailCause::SIP_AMBIGUOUS:
    cfc = V1_5::CallFailCause::CALL_FAIL_SIP_AMBIGUOUS;
    break;
  case qcril::interfaces::CallFailCause::SIP_REQUEST_PENDING:
    cfc = V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_PENDING;
    break;
  case qcril::interfaces::CallFailCause::SIP_UNDECIPHERABLE:
    cfc = V1_5::CallFailCause::CALL_FAIL_SIP_UNDECIPHERABLE;
    break;
  case qcril::interfaces::CallFailCause::RETRY_ON_IMS_WITHOUT_RTT:
    cfc = V1_5::CallFailCause::CALL_FAIL_RETRY_ON_IMS_WITHOUT_RTT;
    break;
  default:
    cfc = static_cast<V1_5::CallFailCause>(V1_0::utils::convertCallFailCause(in));
    break;
  }
  return cfc;
}
qcril::interfaces::CallFailCause convertCallFailCause(const V1_5::CallFailCause &in) {
  qcril::interfaces::CallFailCause cfc = qcril::interfaces::CallFailCause::UNKNOWN;
  switch (in) {
    case V1_5::CallFailCause::CALL_FAIL_REJECTED_ELSEWHERE:
      cfc = qcril::interfaces::CallFailCause::REJECTED_ELSEWHERE;
      break;
    case V1_5::CallFailCause::CALL_FAIL_USER_REJECTED_SESSION_MODIFICATION:
      cfc = qcril::interfaces::CallFailCause::USER_REJECTED_SESSION_MODIFICATION;
      break;
    case V1_5::CallFailCause::CALL_FAIL_USER_CANCELLED_SESSION_MODIFICATION:
      cfc = qcril::interfaces::CallFailCause::USER_CANCELLED_SESSION_MODIFICATION;
      break;
    case V1_5::CallFailCause::CALL_FAIL_SESSION_MODIFICATION_FAILED:
      cfc = qcril::interfaces::CallFailCause::SESSION_MODIFICATION_FAILED;
      break;
    case V1_5::CallFailCause::CALL_FAIL_SIP_METHOD_NOT_ALLOWED:
      cfc = qcril::interfaces::CallFailCause::SIP_METHOD_NOT_ALLOWED;
      break;
    case V1_5::CallFailCause::CALL_FAIL_SIP_PROXY_AUTHENTICATION_REQUIRED:
      cfc = qcril::interfaces::CallFailCause::SIP_PROXY_AUTHENTICATION_REQUIRED;
      break;
    case V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_ENTITY_TOO_LARGE:
      cfc = qcril::interfaces::CallFailCause::SIP_REQUEST_ENTITY_TOO_LARGE;
      break;
    case V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_URI_TOO_LARGE:
      cfc = qcril::interfaces::CallFailCause::SIP_REQUEST_URI_TOO_LARGE;
      break;
    case V1_5::CallFailCause::CALL_FAIL_SIP_EXTENSION_REQUIRED:
      cfc = qcril::interfaces::CallFailCause::SIP_EXTENSION_REQUIRED;
      break;
    case V1_5::CallFailCause::CALL_FAIL_SIP_INTERVAL_TOO_BRIEF:
      cfc = qcril::interfaces::CallFailCause::SIP_INTERVAL_TOO_BRIEF;
      break;
    case V1_5::CallFailCause::CALL_FAIL_SIP_CALL_OR_TRANS_DOES_NOT_EXIST:
      cfc = qcril::interfaces::CallFailCause::SIP_CALL_OR_TRANS_DOES_NOT_EXIST;
      break;
    case V1_5::CallFailCause::CALL_FAIL_SIP_LOOP_DETECTED:
      cfc = qcril::interfaces::CallFailCause::SIP_LOOP_DETECTED;
      break;
    case V1_5::CallFailCause::CALL_FAIL_SIP_TOO_MANY_HOPS:
      cfc = qcril::interfaces::CallFailCause::SIP_TOO_MANY_HOPS;
      break;
    case V1_5::CallFailCause::CALL_FAIL_SIP_AMBIGUOUS:
      cfc = qcril::interfaces::CallFailCause::SIP_AMBIGUOUS;
      break;
    case V1_5::CallFailCause::CALL_FAIL_SIP_REQUEST_PENDING:
      cfc = qcril::interfaces::CallFailCause::SIP_REQUEST_PENDING;
      break;
    case V1_5::CallFailCause::CALL_FAIL_SIP_UNDECIPHERABLE:
      cfc = qcril::interfaces::CallFailCause::SIP_UNDECIPHERABLE;
      break;
    case V1_5::CallFailCause::CALL_FAIL_RETRY_ON_IMS_WITHOUT_RTT:
      cfc = qcril::interfaces::CallFailCause::RETRY_ON_IMS_WITHOUT_RTT;
      break;
    default:
      cfc = V1_0::utils::convertCallFailCause(static_cast<V1_3::CallFailCause>(in));
      break;
  }
  return cfc;
}
bool convertCallFailCauseResponse(
    V1_5::CallFailCauseResponse &out,
    const std::shared_ptr<qcril::interfaces::CallFailCauseResponse> &in) {
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
    out.hasErrorDetails = true;
    V1_0::utils::convertSipErrorInfo(out.errorDetails, in->getErrorDetails());
  }
  return true;
}

bool convertCallInfo(V1_5::CallInfo &out, const qcril::interfaces::CallInfo &in) {
  if (in.hasCallState()) {
    out.state = V1_0::utils::convertCallState(in.getCallState());
  } else {
    out.state = V1_0::CallState::CALL_STATE_INVALID;
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
    out.callDetails.callType = V1_0::utils::convertCallType(in.getCallType());
  } else {
    out.callDetails.callType = V1_0::CallType::CALL_TYPE_INVALID;
  }

  if (in.hasCallDomain()) {
    out.callDetails.callDomain = V1_0::utils::convertCallDomain(in.getCallDomain());
  } else {
    out.callDetails.callDomain = V1_0::CallDomain::CALL_DOMAIN_INVALID;
  }

  uint32_t extrasLength = 0;
  std::vector<std::string> extras;
  if (in.hasCodec()) {
    std::string codec = V1_0::utils::getCodecStringExtra(in.getCodec());
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
  if (in.hasEmergencyServiceCategory()) {
    extrasLength++;
    extras.push_back("EmergencyServiceCategory=" +
                     std::to_string(in.getEmergencyServiceCategory()));
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

  V1_0::utils::convertServiceStatusInfoList(out.callDetails.localAbility, in.getLocalAbility());

  V1_0::utils::convertServiceStatusInfoList(out.callDetails.peerAbility, in.getPeerAbility());

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
    out.callDetails.rttMode = V1_0::utils::convertRttMode(in.getRttMode());
  } else {
    out.callDetails.rttMode = V1_0::RttMode::RTT_MODE_INVALID;
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
    V1_0::utils::convertVerstatInfo(out.verstatInfo, in.getVerstatInfo());
  } else {
    out.verstatInfo = {.canMarkUnwantedCall = false,
                       .verificationStatus = V1_2::VerificationStatus::STATUS_VALIDATION_NONE};
  }

  // Secondary Line Information
  if (in.hasTerminatingNumber() && in.getTerminatingNumber().size() > 0) {
    out.mtMultiLineInfo.msisdn = in.getTerminatingNumber().c_str();
    if (in.hasIsSecondary()) {
      out.mtMultiLineInfo.lineType = in.getIsSecondary()
                                         ? V1_4::MultiIdentityLineType::LINE_TYPE_SECONDARY
                                         : V1_4::MultiIdentityLineType::LINE_TYPE_PRIMARY;
    }
  }

  // TIR options allowed
  out.tirMode = V1_5::TirMode::INVALID;
  if (in.hasTirMode()) {
    out.tirMode = V1_5::utils::convertTirMode(in.getTirMode());
  }

  return true;
}

bool convertCallInfoList(hidl_vec<V1_5::CallInfo> &out,
                         const std::vector<qcril::interfaces::CallInfo> &in) {
  uint32_t count = in.size();
  out.resize(count);
  for (uint32_t i = 0; i < count; ++i) {
    out[i] = {};
    convertCallInfo(out[i], in[i]);
  }
  return true;
}

uint64_t convertToRilEmergencyServiceCategories(
    const hidl_bitfield<V1_5::EmergencyServiceCategory> categories) {
  //Categories are uninitialized.
  uint64_t rilCategories = 0;

  if (categories & V1_5::EmergencyServiceCategory::POLICE) {
    rilCategories |= (uint64_t)RIL_EccCategoryMask::POLICE;
  }
  if (categories & V1_5::EmergencyServiceCategory::AMBULANCE) {
    rilCategories |= (uint64_t)RIL_EccCategoryMask::AMBULANCE;
  }
  if (categories & V1_5::EmergencyServiceCategory::FIRE_BRIGADE) {
    rilCategories |= (uint64_t)RIL_EccCategoryMask::FIRE_BRIGADE;
  }
  if (categories & V1_5::EmergencyServiceCategory::MARINE_GUARD) {
    rilCategories |= (uint64_t)RIL_EccCategoryMask::MARINE_GUARD;
  }
  if (categories & V1_5::EmergencyServiceCategory::MOUNTAIN_RESCUE) {
    rilCategories |= (uint64_t)RIL_EccCategoryMask::MOUNTAIN_RESCUE;
  }
  if (categories & V1_5::EmergencyServiceCategory::MIEC) {
    rilCategories |= (uint64_t)RIL_EccCategoryMask::MANUAL_ECALL;
  }
  if (categories & V1_5::EmergencyServiceCategory::AIEC) {
    rilCategories |= (uint64_t)RIL_EccCategoryMask::AUTO_ECALL;
  }
  if (rilCategories == 0) {
    rilCategories = (uint64_t)RIL_EccCategoryMask::UNKNOWN;
  }
  return rilCategories;
}

EmergencyCallRouting convertToRilEmergencyCallRoute(
    const V1_5::EmergencyCallRoute route) {
  EmergencyCallRouting rilRoute = EmergencyCallRouting::UNKNOWN;

  if (route == V1_5::EmergencyCallRoute::EMERGENCY) {
    rilRoute = EmergencyCallRouting::EMERGENCY;
  } else if (route == V1_5::EmergencyCallRoute::NORMAL) {
    rilRoute = EmergencyCallRouting::NORMAL;
  }
  return rilRoute;
}

V1_5::TirMode convertTirMode(const qcril::interfaces::TirMode &in) {
  switch(in) {
    case qcril::interfaces::TirMode::TEMPORARY:
        return V1_5::TirMode::TEMPORARY;
    case qcril::interfaces::TirMode::PERMANENT:
        return V1_5::TirMode::PERMANENT;
    default:
        return V1_5::TirMode::INVALID;
  }
}

qcril::interfaces::ConfigItem convertConfigItem(const V1_5::ConfigItem in) {
  qcril::interfaces::ConfigItem outConfigItem;
  switch (in) {
    case V1_5::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE:
      outConfigItem = qcril::interfaces::ConfigItem::VOICE_OVER_WIFI_ROAMING_MODE;
      break;
    default:
      return V1_0::utils::convertConfigItem((V1_0::ConfigItem)in);
  }
  return outConfigItem;
}

void readConfigInfo(const V1_5::ConfigInfo &inConfig, qcril::interfaces::ConfigInfo &outConfig) {
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
  qcril::interfaces::ConfigFailureCause cause =
    V1_0::utils::convertConfigErrorCause(inConfig.errorCause);
  if(cause !=  qcril::interfaces::ConfigFailureCause::FAILURE_INVALID) {
    outConfig.setErrorCause(cause);
  }
}

V1_5::UssdModeType convertUssdModeType(const qcril::interfaces::UssdModeType &in) {
  switch(in) {
    case qcril::interfaces::UssdModeType::NOTIFY:
      return UssdModeType::NOTIFY;
    case qcril::interfaces::UssdModeType::REQUEST:
      return UssdModeType::REQUEST;
    case qcril::interfaces::UssdModeType::NW_RELEASE:
      return UssdModeType::NW_RELEASE;
    case qcril::interfaces::UssdModeType::LOCAL_CLIENT:
      return UssdModeType::LOCAL_CLIENT;
    case qcril::interfaces::UssdModeType::NOT_SUPPORTED:
      return UssdModeType::NOT_SUPPORTED;
    case qcril::interfaces::UssdModeType::NW_TIMEOUT:
      return UssdModeType::NW_TIMEOUT;
    default:
      return UssdModeType::NOTIFY;
  }
  return UssdModeType::NOTIFY;
}

bool convertSipErrorInfo(V1_0::SipErrorInfo &out,
                         const std::shared_ptr<qcril::interfaces::SipErrorInfo> &in) {
  out.errorCode = INT32_MAX;
  out.errorString = "";

  if (in == nullptr) {
    return false;
  }

  if (in->hasErrorCode()) {
    out.errorCode = in->getErrorCode();
  }
  if (in->hasErrorString()) {
    out.errorString = in->getErrorString();
  }

  return true;
}

bool convertCallForwardStatus(V1_5::CallForwardStatus &out,
                              const qcril::interfaces::SetCallForwardStatus &in) {
  out.reason = in.hasReason() ? in.getReason() : INT32_MAX;
  out.status = V1_5::Result::FAILURE;
  if (in.hasStatus()) {
    out.status = in.getStatus() ? V1_5::Result::SUCCESS : V1_5::Result::FAILURE;
  }
  convertSipErrorInfo(out.errorDetails, (in.hasErrorDetails() ? in.getErrorDetails() : nullptr));
  return true;
}

bool convertCallForwardStatus(hidl_vec<V1_5::CallForwardStatus> &out,
                              const std::vector<qcril::interfaces::SetCallForwardStatus> &in) {
  uint32_t count = in.size();
  out.resize(count);
  for (uint32_t i = 0; i < count; ++i) {
    convertCallForwardStatus(out[i], in[i]);
  }
  return true;
}

bool convertCallForwardStatusInfo(
    V1_5::CallForwardStatusInfo &out,
    const std::shared_ptr<qcril::interfaces::SetCallForwardRespData> &in) {
  out.errorDetails.errorCode = INT32_MAX;
  out.errorDetails.errorString = "";
  out.status.resize(0);
  if (in) {
    if (in->hasErrorDetails() && in->getErrorDetails() != nullptr) {
      convertSipErrorInfo(out.errorDetails, in->getErrorDetails());
    }
    if (!in->getSetCallForwardStatus().empty()) {
      convertCallForwardStatus(out.status, in->getSetCallForwardStatus());
    }
  }
  return true;
}

qcril::interfaces::DeliverStatus convertHidlToRilSmsAckResult(
        const V1_5::ImsSmsDeliverStatusResult result) {
    switch (result) {
    case ImsSmsDeliverStatusResult::DELIVER_STATUS_ERROR_NO_MEMORY:
        return qcril::interfaces::DeliverStatus::STATUS_NO_MEMORY;
    case ImsSmsDeliverStatusResult::DELIVER_STATUS_ERROR_REQUEST_NOT_SUPPORTED:
        return qcril::interfaces::DeliverStatus::STATUS_REQUEST_NOT_SUPPORTED;
    default:
        return V1_0::utils::convertHidlToRilSmsAckResult(
                 (V1_2::ImsSmsDeliverStatusResult)result);
    }
}

void convertRilImsResponseToHidlResponse(RIL_Errno rilErr, RIL_RadioTechnologyFamily tech,
        std::optional<bool> bLteOnlyReg,
        V1_2::ImsSmsSendStatusResult& statusReport, V1_5::ImsSmsSendFailureReason& failureReason) {
    statusReport = V1_2::ImsSmsSendStatusResult::SEND_STATUS_ERROR;
    switch (rilErr) {
    case RIL_E_SUCCESS:
        statusReport = V1_2::ImsSmsSendStatusResult::SEND_STATUS_OK;
        failureReason = V1_5::ImsSmsSendFailureReason::RESULT_ERROR_NONE;
        break;
    case RIL_E_SMS_SEND_FAIL_RETRY:
    {
        bool lteOnly = (bLteOnlyReg ? *bLteOnlyReg : false);
        statusReport = ((tech == RADIO_TECH_3GPP) && lteOnly) ?
                V1_2::ImsSmsSendStatusResult::SEND_STATUS_ERROR :
                V1_2::ImsSmsSendStatusResult::SEND_STATUS_ERROR_FALLBACK;
        failureReason = V1_5::ImsSmsSendFailureReason::RESULT_ERROR_GENERIC_FAILURE;
    }
        break;
    case RIL_E_NO_NETWORK_FOUND:
        failureReason = V1_5::ImsSmsSendFailureReason::RESULT_ERROR_NO_SERVICE;
        break;
    case RIL_E_FDN_CHECK_FAILURE:
        failureReason = V1_5::ImsSmsSendFailureReason::RESULT_ERROR_FDN_CHECK_FAILURE;
        break;
    case RIL_E_RADIO_NOT_AVAILABLE:
        failureReason = V1_5::ImsSmsSendFailureReason::RESULT_RADIO_NOT_AVAILABLE;
        break;
    case RIL_E_NETWORK_REJECT:
        failureReason = V1_5::ImsSmsSendFailureReason::RESULT_NETWORK_REJECT;
        break;
    case RIL_E_INVALID_ARGUMENTS:
        failureReason = V1_5::ImsSmsSendFailureReason::RESULT_INVALID_ARGUMENTS;
        break;
    case RIL_E_INVALID_STATE:
        failureReason = V1_5::ImsSmsSendFailureReason::RESULT_INVALID_STATE;
        break;
    case RIL_E_NO_MEMORY:
        failureReason = V1_5::ImsSmsSendFailureReason::RESULT_NO_MEMORY;
        break;
    case RIL_E_INVALID_SMS_FORMAT:
        failureReason = V1_5::ImsSmsSendFailureReason::RESULT_INVALID_SMS_FORMAT;
        break;
    case RIL_E_SYSTEM_ERR:
        failureReason = V1_5::ImsSmsSendFailureReason::RESULT_SYSTEM_ERROR;
        break;
    case RIL_E_MODEM_ERR:
        failureReason = V1_5::ImsSmsSendFailureReason::RESULT_MODEM_ERROR;
        break;
    case RIL_E_NETWORK_ERR:
        failureReason = V1_5::ImsSmsSendFailureReason::RESULT_NETWORK_ERROR;
        break;
    case RIL_E_ENCODING_ERR:
        failureReason = V1_5::ImsSmsSendFailureReason::RESULT_ENCODING_ERROR;
        break;
    case RIL_E_INVALID_SMSC_ADDRESS:
        failureReason = V1_5::ImsSmsSendFailureReason::RESULT_INVALID_SMSC_ADDRESS;
        break;
    case RIL_E_OPERATION_NOT_ALLOWED:
        failureReason = V1_5::ImsSmsSendFailureReason::RESULT_OPERATION_NOT_ALLOWED;
        break;
    case RIL_E_INTERNAL_ERR:
        failureReason = V1_5::ImsSmsSendFailureReason::RESULT_INTERNAL_ERROR;
        break;
    case RIL_E_NO_RESOURCES:
        failureReason = V1_5::ImsSmsSendFailureReason::RESULT_NO_RESOURCES;
        break;
    case RIL_E_CANCELLED:
        failureReason = V1_5::ImsSmsSendFailureReason::RESULT_CANCELLED;
        break;
    case RIL_E_REQUEST_NOT_SUPPORTED:
        failureReason = V1_5::ImsSmsSendFailureReason::RESULT_REQUEST_NOT_SUPPORTED;
        break;
    default:
        failureReason = V1_5::ImsSmsSendFailureReason::RESULT_ERROR_GENERIC_FAILURE;
        break;
    }
}

V1_2::ImsSmsSendFailureReason convertHidlSendSmsFailureReasonToV12(
        V1_5::ImsSmsSendFailureReason reason) {
    switch (reason) {
    case V1_5::ImsSmsSendFailureReason::RESULT_ERROR_NONE:
    case V1_5::ImsSmsSendFailureReason::RESULT_ERROR_NO_SERVICE:
        return (V1_2::ImsSmsSendFailureReason)reason;
    default:
        return V1_2::ImsSmsSendFailureReason::RESULT_ERROR_GENERIC_FAILURE;
    }
}

}  // namespace utils
}  // namespace V1_5
}  // namespace ims
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
