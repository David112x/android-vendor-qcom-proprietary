/*===========================================================================
    Copyright (c) 2019 Qualcomm Technologies, Inc.
    All Rights Reserved.
    Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/
#define TAG "RILQ"

#include <framework/Log.h>
#include <modules/android_ims_radio/hidl_impl/1.6/qcril_qmi_ims_radio_utils_1_6.h>
#include <string.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_6 {
namespace utils {

qcril::interfaces::ConfigItem convertConfigItem(const V1_6::ConfigItem in) {
    qcril::interfaces::ConfigItem outConfigItem;
    switch(in) {
        case V1_6::ConfigItem::CONFIG_ITEM_SET_AUTO_REJECT_CALL_MODE_CONFIG:
            outConfigItem = qcril::interfaces::ConfigItem::AUTO_REJECT_CALL_ENABLED;
            break;
        default:
            return V1_5::utils::convertConfigItem((V1_5::ConfigItem)in);
    }
    return outConfigItem;
}

void readConfigInfo(const V1_6::ConfigInfo &inConfig, qcril::interfaces::ConfigInfo &outConfig) {
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

V1_6::ConfigItem convertConfigItem(qcril::interfaces::ConfigItem in) {
    switch(in) {
        case qcril::interfaces::ConfigItem::AUTO_REJECT_CALL_ENABLED:
            return ConfigItem::CONFIG_ITEM_SET_AUTO_REJECT_CALL_MODE_CONFIG;
        case qcril::interfaces::ConfigItem::VOICE_OVER_WIFI_ROAMING_MODE:
            return ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE;
        default:
            return (V1_6::ConfigItem) V1_0::utils::convertConfigItem(in);
    }
}

void convertConfigInfo(V1_6::ConfigInfo &outConfig, const qcril::interfaces::ConfigInfo &inConfig) {
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
            V1_0::utils::convertConfigErrorCause(inConfig.getErrorCause()) :
            V1_0::ConfigFailureCause::CONFIG_FAILURE_INVALID;

}

bool convertCallInfoList(hidl_vec<V1_6::CallInfo> &out,
                         const std::vector<qcril::interfaces::CallInfo> &in) {
  uint32_t count = in.size();
  out.resize(count);
  for (uint32_t i = 0; i < count; ++i) {
    out[i] = {};
    convertCallInfo(out[i], in[i]);
  }
  return true;
}

bool convertCallInfo(V1_6::CallInfo &out, const qcril::interfaces::CallInfo &in) {
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
      V1_5::utils::convertCallFailCauseResponse(out.failCause, in.getCallFailCauseResponse());
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

bool convertServiceStatusInfo(V1_6::ServiceStatusInfo &out,
                              const qcril::interfaces::ServiceStatusInfo &in) {
  out.hasIsValid = true;
  out.isValid = true;

  out.type = V1_0::ServiceType::INVALID;

  if (in.hasCallType()) {
    out.callType = V1_0::utils::convertCallType(in.getCallType());
  } else {
    out.callType = V1_0::CallType::CALL_TYPE_INVALID;
  }

  if (in.hasStatusType()) {
    out.status = V1_0::utils::convertStatusType(in.getStatusType());
  } else {
    out.status = V1_0::StatusType::STATUS_INVALID;
  }

  if (in.hasRestrictCause()) {
    out.restrictCause = in.getRestrictCause();
  } else {
    out.restrictCause = INT32_MAX;
  }

  if (in.hasRttMode()) {
    out.rttMode = V1_0::utils::convertRttMode(in.getRttMode());
  } else {
    out.rttMode = V1_0::RttMode::RTT_MODE_INVALID;
  }

  if (in.hasAccessTechnologyStatus() && in.getAccessTechnologyStatus() != nullptr) {
    out.accTechStatus.resize(1);
    out.accTechStatus[0] = {};
    V1_6::utils::convertAccessTechnologyStatus(out.accTechStatus[0], *(in.getAccessTechnologyStatus()));
  }
  return true;
}

bool convertServiceStatusInfoList(hidl_vec<V1_6::ServiceStatusInfo> &out,
                                  const qcril::interfaces::ServiceStatusInfoList &in) {
  uint32_t count = in.getServiceStatusInfo().size();
  out.resize(count);
  for (uint32_t i = 0; i < count; ++i) {
    out[i] = {};
    convertServiceStatusInfo(out[i], in.getServiceStatusInfo()[i]);
  }
  return true;
}

bool convertServiceStatusInfoList(hidl_vec<V1_6::ServiceStatusInfo> &out,
                                  const std::vector<qcril::interfaces::ServiceStatusInfo> &in) {
  uint32_t count = in.size();
  out.resize(count);
  for (uint32_t i = 0; i < count; ++i) {
    out[i] = {};
    convertServiceStatusInfo(out[i], in[i]);
  }
  return true;
}

bool convertRegistrationInfo(V1_6::RegistrationInfo &out, const qcril::interfaces::Registration &in) {
  if (in.hasState()) {
    out.state = V1_0::utils::convertRegState(in.getState());
  } else {
    out.state = V1_0::RegState::INVALID;
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
    out.radioTech = V1_6::RadioTechType::RADIO_TECH_INVALID;
  }
  if (in.hasPAssociatedUris()) { // && in.getPAssociatedUris() != nullptr)
    out.pAssociatedUris = in.getPAssociatedUris().c_str();
  }
  return true;
}

bool convertAccessTechnologyStatus(V1_6::StatusForAccessTech &out,
                                   const qcril::interfaces::AccessTechnologyStatus &in) {
  if (in.hasNetworkMode()) {
    out.networkMode = convertRadioTechType(in.getNetworkMode());
  } else {
    out.networkMode = V1_6::RadioTechType::RADIO_TECH_INVALID;
  }

  if (in.hasStatusType()) {
    out.status = V1_0::utils::convertStatusType(in.getStatusType());
  } else {
    out.status = V1_0::StatusType::STATUS_INVALID;
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

V1_6::RadioTechType convertRadioTechType(const RIL_RadioTechnology &in) {
  switch (in) {
    case RADIO_TECH_5G:
      return V1_6::RadioTechType::RADIO_TECH_NR5G;
    default:
      return static_cast<V1_6::RadioTechType>(V1_0::utils::convertRadioTechType(in));
  }
}

void readServiceStatusInfo(const V1_6::ServiceStatusInfo &in,
                                qcril::interfaces::ServiceStatusInfo &out) {
  if (in.callType != V1_0::CallType::CALL_TYPE_INVALID) {
    out.setCallType(V1_0::utils::convertCallType(in.callType));
  }
  if (in.status != V1_0::StatusType::STATUS_INVALID) {
    out.setStatusType(V1_0::utils::convertStatusType(in.status));
  }

  //TODO set userdata

  if (in.restrictCause != INT32_MAX) {
    out.setRestrictCause(in.restrictCause);
  }

  const uint32_t accTechStatusLen = in.accTechStatus.size();
  //read first value

  if (accTechStatusLen > 0) {
    out.setAccessTechnologyStatus(V1_6::utils::readAccessTechStatus(in.accTechStatus[0]));
  }

  if (in.rttMode != V1_0::RttMode::RTT_MODE_INVALID) {
    out.setRttMode(V1_0::utils::convertRttMode(in.rttMode));
  }
}

std::shared_ptr<qcril::interfaces::AccessTechnologyStatus> readAccessTechStatus(
    const StatusForAccessTech& in) {
  std::shared_ptr<qcril::interfaces::AccessTechnologyStatus> out =
    std::make_shared<qcril::interfaces::AccessTechnologyStatus>();

  if (out != nullptr) {
    if (in.networkMode != RadioTechType::RADIO_TECH_INVALID) {
      out->setNetworkMode(V1_6::utils::convertRadioTechType(in.networkMode));
    }
    if (in.status != V1_0::StatusType::STATUS_INVALID) {
      out->setStatusType(V1_0::utils::convertStatusType(in.status));
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

std::shared_ptr<qcril::interfaces::Registration> readRegistration(const RegistrationInfo& in) {
  std::shared_ptr<qcril::interfaces::Registration> out =
  std::make_shared<qcril::interfaces::Registration>();
  if (out) {
    if (in.state != V1_0::RegState::INVALID) {
      out->setState(V1_0::utils::convertRegState(in.state));
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

RIL_RadioTechnology convertRadioTechType(const RadioTechType &in) {
  switch (in) {
    case RadioTechType::RADIO_TECH_NR5G:
      return RADIO_TECH_5G;
    default:
      return V1_0::utils::convertRadioTechType(static_cast<V1_0::RadioTechType>(in));
  }
}

}  // namespace utils
}  // namespace V1_6
}  // namespace ims
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
