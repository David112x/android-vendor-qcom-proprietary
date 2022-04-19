/******************************************************************************
  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
******************************************************************************/

#define TAG "RILQ"

#include <framework/Log.h>
#include <modules/android_ims_radio/hidl_impl/1.4/qcril_qmi_ims_radio_utils_1_4.h>
#include <string.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_4 {
namespace utils {

bool convertCallInfo(V1_4::CallInfo &out, const qcril::interfaces::CallInfo &in) {
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
    V1_0::utils::convertCallFailCauseResponse(out.failCause, in.getCallFailCauseResponse());
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
  return true;
}

bool convertCallInfoList(hidl_vec<V1_4::CallInfo> &out,
                         const std::vector<qcril::interfaces::CallInfo> &in) {
  uint32_t count = in.size();
  out.resize(count);
  for (uint32_t i = 0; i < count; ++i) {
    out[i] = {};
    convertCallInfo(out[i], in[i]);
  }
  return true;
}

// ==============================

void convertToHidlVirtualLines(hidl_vec<hidl_string> &out, const std::vector<std::string> &in) {
  int size = in.size();
  out.resize(size);
  for (int i = 0; i < size; i++) {
    out[i] = hidl_string(in[i]);
#ifndef QMI_RIL_UTF
    QCRIL_LOG_INFO("Virtual Line[%d] : %s", i, out[i].c_str());
#endif
  }
}

void convertToHidlMultiIdentityRegistrationStatus(
    const std::vector<qcril::interfaces::MultiIdentityInfo> &in_lines,
    hidl_vec<V1_4::MultiIdentityLineInfoHal> &out_lines) {
  out_lines.resize(in_lines.size());
  for (unsigned i = 0; i < in_lines.size(); i++) {
    auto &in_line = in_lines[i];
    V1_4::MultiIdentityLineInfoHal out_line = {};
    out_line.msisdn = in_line.msisdn;
    out_line.registrationStatus =
        convertToHIdlMultiIdentityRegistrationStatus(in_line.registrationStatus);
#ifndef QMI_RIL_UTF
    std::string logStr = toString(out_line);
    QCRIL_LOG_INFO("MultiIdentity Registration Info for line[%d] : %s", i, logStr.c_str());
#endif
    out_lines[i] = out_line;
  }
}

V1_4::MultiIdentityRegistrationStatus convertToHIdlMultiIdentityRegistrationStatus(int status) {
  switch (status) {
    case qcril::interfaces::MultiIdentityInfo::REGISTRATION_STATUS_ACTIVE:
      return V1_4::MultiIdentityRegistrationStatus::STATUS_ENABLE;
    case qcril::interfaces::MultiIdentityInfo::REGISTRATION_STATUS_INACTIVE:
      return V1_4::MultiIdentityRegistrationStatus::STATUS_DISABLE;
    default:
      return V1_4::MultiIdentityRegistrationStatus::STATUS_UNKNOWN;
  }
}

void convertToRilMultiIdentityInfo(const V1_4::MultiIdentityLineInfoHal &in_line,
                                   qcril::interfaces::MultiIdentityInfo &out_line) {
  out_line.msisdn = in_line.msisdn;
  out_line.registrationStatus =
      convertToRilMultiIdentityRegistrationStatus(in_line.registrationStatus);
  out_line.lineType = convertToRilMultiIdentityLineType(in_line.lineType);
}

int convertToRilMultiIdentityRegistrationStatus(V1_4::MultiIdentityRegistrationStatus status) {
  switch (status) {
    case V1_4::MultiIdentityRegistrationStatus::STATUS_ENABLE:
      return qcril::interfaces::MultiIdentityInfo::REGISTRATION_STATUS_ACTIVE;
    case V1_4::MultiIdentityRegistrationStatus::STATUS_DISABLE:
      return qcril::interfaces::MultiIdentityInfo::REGISTRATION_STATUS_INACTIVE;
    default:
      return qcril::interfaces::MultiIdentityInfo::REGISTRATION_STATUS_UNKNOWN;
  }
}

int convertToRilMultiIdentityLineType(V1_4::MultiIdentityLineType type) {
  switch (type) {
    case V1_4::MultiIdentityLineType::LINE_TYPE_PRIMARY:
      return qcril::interfaces::MultiIdentityInfo::LINE_TYPE_PRIMARY;
    case V1_4::MultiIdentityLineType::LINE_TYPE_SECONDARY:
      return qcril::interfaces::MultiIdentityInfo::LINE_TYPE_SECONDARY;
    default:
      return qcril::interfaces::MultiIdentityInfo::LINE_TYPE_UNKNOWN;
  }
}

}  // namespace utils
}  // namespace V1_4
}  // namespace ims
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
