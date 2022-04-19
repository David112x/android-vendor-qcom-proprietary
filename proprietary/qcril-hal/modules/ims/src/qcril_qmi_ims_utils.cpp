/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include <interfaces/ims/ims.h>
#include <map>
#include <memory>
#include "ip_multimedia_subsystem_application_v01.h"
#include "ip_multimedia_subsystem_settings_v01.h"
#include "qcril_log.h"
#include "qcril_qmi_ims_utils.h"

#define TAG "QCRIL_IMSS"

//===========================================================================
// qcril_qmi_ims_map_qmi_ims_reg_state_to_ims_reg_state
//===========================================================================
qcril::interfaces::RegState qcril_qmi_ims_map_qmi_ims_reg_state_to_ims_reg_state(
    uint8_t ims_registered) {
  qcril::interfaces::RegState ret = qcril::interfaces::RegState::UNKNOWN;
  switch (ims_registered) {
    case IMSA_STATUS_NOT_REGISTERED_V01:
      ret = qcril::interfaces::RegState::NOT_REGISTERED;
      break;
    case IMSA_STATUS_REGISTERING_V01:
      ret = qcril::interfaces::RegState::REGISTERING;
      break;
    case IMSA_STATUS_REGISTERED_V01:
      ret = qcril::interfaces::RegState::REGISTERED;
      break;
    default:
      ret = qcril::interfaces::RegState::UNKNOWN;
      break;
  }

  // QCRIL_LOG_INFO("qmi ims_reg_state %d mapped to ims ims_reg_state %d", ims_registered, ret);

  return ret;
}  // qcril_qmi_ims_map_qmi_ims_reg_state_to_ims_reg_state

//===========================================================================
// qcril_qmi_ims_map_imsa_rat_to_ims_radio_tech
//===========================================================================
RIL_RadioTechnology qcril_qmi_ims_map_imsa_rat_to_ims_radio_tech(
    imsa_service_rat_enum_v01 imsa_rat) {
  RIL_RadioTechnology ret = RADIO_TECH_UNKNOWN;
  switch (imsa_rat) {
    case IMSA_WLAN_V01:
      ret = RADIO_TECH_WIFI;
      break;
    case IMSA_IWLAN_V01:
      ret = RADIO_TECH_IWLAN;
      break;
    case IMSA_WWAN_V01:
      ret = RADIO_TECH_LTE;
      break;
    default:
      ret = RADIO_TECH_UNKNOWN;
      break;
  }

  // QCRIL_LOG_INFO("qmi imsa_rat %d mapped to ims radio_tech %d", imsa_rat, ret);

  return ret;
}  // qcril_qmi_ims_map_imsa_rat_to_ims_radio_tech

//===========================================================================
// qcril_qmi_ims_map_imsa_service_status_to_ims_status_type
//===========================================================================
qcril::interfaces::StatusType qcril_qmi_ims_map_imsa_service_status_to_ims_status_type(
    imsa_service_status_enum_v01 status) {
  qcril::interfaces::StatusType ret = qcril::interfaces::StatusType::UNKNOWN;
  switch (status) {
    case IMSA_NO_SERVICE_V01:
      ret = qcril::interfaces::StatusType::DISABLED;
      break;
    case IMSA_LIMITED_SERVICE_V01:
      ret = qcril::interfaces::StatusType::PARTIALLY_ENABLED;
      break;
    case IMSA_FULL_SERVICE_V01:
      ret = qcril::interfaces::StatusType::ENABLED;
      break;
    default:
      ret = qcril::interfaces::StatusType::UNKNOWN;
      break;
  }
  // QCRIL_LOG_INFO("qmi imsa_service %d mapped to ims service_type %d", status, ret);

  return ret;
}  // qcril_qmi_ims_map_imsa_service_status_to_ims_status_type

//===========================================================================
// qcril_qmi_ims_create_Registration
//===========================================================================
std::shared_ptr<qcril::interfaces::Registration> qcril_qmi_ims_create_Registration(
    uint8_t regStateValid, imsa_ims_registration_status_enum_v01 regState, uint8_t ratValid,
    imsa_service_rat_enum_v01 rat, uint8_t errorCodeValid, uint16_t errorCode, char *errorString,
    uint32_t uriListLen, p_associated_uri_v01 *uriList) {
  auto reg = std::make_shared<qcril::interfaces::Registration>();

  if (reg != nullptr) {
    if (regStateValid) {
      reg->setState(qcril_qmi_ims_map_qmi_ims_reg_state_to_ims_reg_state(regState));
    }
    if (ratValid) {
      reg->setRadioTechnology(qcril_qmi_ims_map_imsa_rat_to_ims_radio_tech(rat));
    }
    if (errorCodeValid) {
      reg->setErrorCode(errorCode);
    }
    if (errorString != nullptr) {
      reg->setErrorMessage(errorString);
    }
    if (uriListLen > 0 && uriList != nullptr) {
      std::string uri_list = uriList[0].uri;
      for (uint32_t i = 1; i < uriListLen; i++) {
        uri_list = uri_list + "|" + uriList[i].uri;
      }
      reg->setPAssociatedUris(uri_list);
    }
  }
  // TODO print reg
  return reg;
}  // qcril_qmi_ims_create_Registration

//===========================================================================
// qcril_qmi_ims_create_AccessTechnologyStatus
//===========================================================================
std::shared_ptr<qcril::interfaces::AccessTechnologyStatus>
qcril_qmi_ims_create_AccessTechnologyStatus(boolean rat_valid, imsa_service_rat_enum_v01 rat,
                                            imsa_service_status_enum_v01 status,
                                            bool restrictCause_valid, uint32_t restrictCause,
                                            std::shared_ptr<qcril::interfaces::Registration> reg) {
  auto ats = std::make_shared<qcril::interfaces::AccessTechnologyStatus>();
  if (ats != nullptr) {
    if (rat_valid) {
      ats->setNetworkMode(qcril_qmi_ims_map_imsa_rat_to_ims_radio_tech(rat));
    }
    ats->setStatusType(qcril_qmi_ims_map_imsa_service_status_to_ims_status_type(status));
    if (restrictCause_valid) {
      ats->setRestrictCause(restrictCause);
    }
    ats->setRegistration(reg);
  }
  return ats;
}  // qcril_qmi_ims_create_AccessTechnologyStatus

//===========================================================================
// qcril_qmi_ims_create_ServiceStatusInfo
//===========================================================================
qcril::interfaces::ServiceStatusInfo qcril_qmi_ims_create_ServiceStatusInfo(
    qcril::interfaces::CallType callType, imsa_service_status_enum_v01 status, boolean rat_valid,
    imsa_service_rat_enum_v01 rat) {
  qcril::interfaces::ServiceStatusInfo srvStatInfo = {};
  srvStatInfo.setCallType(callType);
  srvStatInfo.setStatusType(qcril_qmi_ims_map_imsa_service_status_to_ims_status_type(status));
  // srvStatInfo.setRestrictCause(callType);
  std::shared_ptr<qcril::interfaces::Registration> reg = qcril_qmi_ims_create_Registration(
      TRUE,
      (status == IMSA_NO_SERVICE_V01 ? IMSA_STATUS_NOT_REGISTERED_V01 : IMSA_STATUS_REGISTERED_V01),
      rat_valid, rat, FALSE, 0, nullptr, 0, nullptr);
  srvStatInfo.setAccessTechnologyStatus(
      qcril_qmi_ims_create_AccessTechnologyStatus(rat_valid, rat, status, FALSE, 0, reg));
  // srvStatInfo.setRttMode(callType);
  return srvStatInfo;
}  // qcril_qmi_ims_create_ServiceStatusInfo

//===========================================================================
// qcril_qmi_ims_create_ServiceStatusInfoList
//===========================================================================
std::shared_ptr<qcril::interfaces::ServiceStatusInfoList>
qcril_qmi_ims_create_ServiceStatusInfoList(const imsa_service_status_ind_msg_v01 *qmi_data) {
  auto srvStatInfoList = std::make_shared<qcril::interfaces::ServiceStatusInfoList>();

  if (srvStatInfoList != nullptr) {
    if (qmi_data->sms_service_status_valid) {
      srvStatInfoList->addToServiceStatusInfo(qcril_qmi_ims_create_ServiceStatusInfo(
          qcril::interfaces::CallType::SMS, qmi_data->sms_service_status,
          qmi_data->sms_service_rat_valid, qmi_data->sms_service_rat));
    }
    if (qmi_data->voip_service_status_valid) {
      srvStatInfoList->addToServiceStatusInfo(qcril_qmi_ims_create_ServiceStatusInfo(
          qcril::interfaces::CallType::VOICE, qmi_data->voip_service_status,
          qmi_data->voip_service_rat_valid, qmi_data->voip_service_rat));
    }
    if (qmi_data->vt_service_status_valid) {
      srvStatInfoList->addToServiceStatusInfo(qcril_qmi_ims_create_ServiceStatusInfo(
          qcril::interfaces::CallType::VT, qmi_data->vt_service_status,
          qmi_data->vt_service_rat_valid, qmi_data->vt_service_rat));
      srvStatInfoList->addToServiceStatusInfo(qcril_qmi_ims_create_ServiceStatusInfo(
          qcril::interfaces::CallType::VT_TX, qmi_data->vt_service_status,
          qmi_data->vt_service_rat_valid, qmi_data->vt_service_rat));
      srvStatInfoList->addToServiceStatusInfo(qcril_qmi_ims_create_ServiceStatusInfo(
          qcril::interfaces::CallType::VT_RX, qmi_data->vt_service_status,
          qmi_data->vt_service_rat_valid, qmi_data->vt_service_rat));
    }
    if (qmi_data->ut_service_status_valid) {
      srvStatInfoList->addToServiceStatusInfo(qcril_qmi_ims_create_ServiceStatusInfo(
          qcril::interfaces::CallType::UT, qmi_data->ut_service_status,
          qmi_data->ut_service_rat_valid, qmi_data->ut_service_rat));
    }
  }

  return srvStatInfoList;
}  // qcril_qmi_ims_create_ServiceStatusInfoList

//===========================================================================
// qcril_qmi_ims_map_qmi_wifi_quality_to_ims_wifi_quality
//===========================================================================
qcril::interfaces::VowifiQuality qcril_qmi_ims_map_qmi_wifi_quality_to_ims_wifi_quality
(
 imsa_vowifiprofile_status_enum_v01 qmi_wifi_quality
)
{
    switch (qmi_wifi_quality)
    {
      case IMSA_SERVICE_VOWIFI_QUALITIY_EXCELLENT_V01:
        return qcril::interfaces::VowifiQuality::EXCELLENT;
        break;
      case IMSA_SERVICE_VOWIFI_QUALITIY_FAIR_V01:
        return qcril::interfaces::VowifiQuality::FAIR;
        break;
      case IMSA_SERVICE_VOWIFI_QUALITIY_BAD_V01:
        return qcril::interfaces::VowifiQuality::BAD;
        break;
      default:
        return qcril::interfaces::VowifiQuality::UNKNOWN;
        break;
    }
    return qcril::interfaces::VowifiQuality::UNKNOWN;
}

//===========================================================================
// qcril_qmi_ims_map_qmi_block_reason_to_ims_block_reason
//===========================================================================
qcril::interfaces::BlockReasonType qcril_qmi_ims_map_qmi_block_reason_to_ims_block_reason
(
 imsa_block_reason_type_mask_v01 qmi_block_reason
)
{
    using qcril::interfaces::BlockReasonType;
    BlockReasonType failure = BlockReasonType::OTHER_FAILURE;
    switch (qmi_block_reason)
    {
      case IMSA_BLOCK_REASON_PDP_FAILURE_BIT_MASK_V01:
        failure = BlockReasonType::PDP_FAILURE;
      break;
      case IMSA_BLOCK_REASON_REGISTRATION_FAILURE_BIT_MASK_V01:
        failure = BlockReasonType::REGISTRATION_FAILURE;
      break;
      case IMSA_BLOCK_REASON_HANDOVER_FAILURE_BIT_MASK_V01:
        failure = BlockReasonType::HANDOVER_FAILURE;
      break;
      case IMSA_BLOCK_REASON_OTHER_FAILURE_BIT_MASK_V01:
      default:
        failure = BlockReasonType::OTHER_FAILURE;
        break;
    }
    return failure;
}

//===========================================================================
// qcril_qmi_call_end_type_to_ims_reg_failure
//===========================================================================
qcril::interfaces::RegFailureReasonType qcril_qmi_call_end_type_to_ims_reg_failure
(
 imsa_verbose_call_end_reason_type_enum_v01 qmi_block_reason
)
{
  using qcril::interfaces::RegFailureReasonType;

  RegFailureReasonType result = RegFailureReasonType::UNSPECIFIED;
    switch (qmi_block_reason)
    {
      case IMSA_VCER_TYPE_UNSPECIFIED_V01:
        result = RegFailureReasonType::UNSPECIFIED;
        break;
      case IMSA_VCER_TYPE_MOBILE_IP_V01:
        result = RegFailureReasonType::MOBILE_IP;
        break;
      case IMSA_VCER_TYPE_INTERNAL_V01:
        result = RegFailureReasonType::INTERNAL;
        break;
      case IMSA_VCER_TYPE_CALL_MANAGER_DEFINED_V01:
        result = RegFailureReasonType::CALL_MANAGER_DEFINED;
        break;
      case IMSA_VCER_TYPE_3GPP_SPEC_DEFINED_V01:
        result = RegFailureReasonType::SPEC_DEFINED;
        break;
      case IMSA_VCER_TYPE_PPP_V01:
        result = RegFailureReasonType::PPP;
        break;
      case IMSA_VCER_TYPE_EHRPD_V01:
        result = RegFailureReasonType::EHRPD;
        break;
      case IMSA_VCER_TYPE_IPV6_V01:
        result = RegFailureReasonType::IPV6;
        break;
      case IMSA_VCER_TYPE_IWLAN_V01:
        result = RegFailureReasonType::IWLAN;
        break;
      case IMSA_VCER_TYPE_HANDOFF_V01:
        result = RegFailureReasonType::HANDOFF;
        break;
      default:
        break;
    }
  return result;
}

//===========================================================================
// qcril_qmi_ims_map_ims_config_to_radio_config_item
//===========================================================================
qcril_qmi_radio_config_item_type qcril_qmi_ims_map_ims_config_to_radio_config_item
(
   qcril::interfaces::ConfigItem ims_config_item
)
{
  QCRIL_LOG_FUNC_ENTRY();
  qcril_qmi_radio_config_item_type ret = QCRIL_QMI_RADIO_CONFIG_INVALID;

  switch(ims_config_item)
  {
    case qcril::interfaces::ConfigItem::VOCODER_AMRMODESET:
      ret = QCRIL_QMI_RADIO_CONFIG_VOIP_AMR_MODE;
      break;

    case qcril::interfaces::ConfigItem::VOCODER_AMRWBMODESET:
      ret = QCRIL_QMI_RADIO_CONFIG_VOIP_AMR_WB_MODE;
      break;

    case qcril::interfaces::ConfigItem::SIP_SESSION_TIMER:
      ret = QCRIL_QMI_RADIO_CONFIG_VOIP_SESSION_EXPIRY_TIMER;
      break;

    case qcril::interfaces::ConfigItem::MIN_SESSION_EXPIRY:
      ret = QCRIL_QMI_RADIO_CONFIG_VOIP_MIN_SESSION_EXPIRY;
      break;

    case qcril::interfaces::ConfigItem::CANCELLATION_TIMER:
      ret = QCRIL_QMI_RADIO_CONFIG_SIP_TIMER_OPERATOR_MODE_A;
      break;

    case qcril::interfaces::ConfigItem::T_DELAY:
      ret = QCRIL_QMI_RADIO_CONFIG_REG_MGR_EXTENDED_T_DELAY;
      break;

    case qcril::interfaces::ConfigItem::SILENT_REDIAL_ENABLE:
      ret = QCRIL_QMI_RADIO_CONFIG_VOIP_SILENT_REDIAL_ENABLED;
      break;

    case qcril::interfaces::ConfigItem::SIP_T1_TIMER:
      ret = QCRIL_QMI_RADIO_CONFIG_SIP_TIMER_T1;
      break;

    case qcril::interfaces::ConfigItem::SIP_T2_TIMER:
      ret = QCRIL_QMI_RADIO_CONFIG_SIP_TIMER_T2;
      break;

    case qcril::interfaces::ConfigItem::SIP_TF_TIMER:
      ret = QCRIL_QMI_RADIO_CONFIG_SIP_TIMER_TF;
      break;

    case qcril::interfaces::ConfigItem::VLT_SETTING_ENABLED:
      ret = QCRIL_QMI_RADIO_CONFIG_CLIENT_PROVISIONING_ENABLE_VOLTE;
      break;

    case qcril::interfaces::ConfigItem::LVC_SETTING_ENABLED:
      ret = QCRIL_QMI_RADIO_CONFIG_CLIENT_PROVISIONING_ENABLE_VT;
      break;

    case qcril::interfaces::ConfigItem::DOMAIN_NAME:
      ret = QCRIL_QMI_RADIO_CONFIG_USER_IMS_DOMAIN_NAME;
      break;

    case qcril::interfaces::ConfigItem::SMS_FORMAT:
      ret = QCRIL_QMI_RADIO_CONFIG_SMS_FORMAT;
      break;

    case qcril::interfaces::ConfigItem::SMS_OVER_IP:
      ret = QCRIL_QMI_RADIO_CONFIG_SMS_OVER_IP;
      break;

    case qcril::interfaces::ConfigItem::PUBLISH_TIMER:
      ret = QCRIL_QMI_RADIO_CONFIG_PRESENCE_PUBLISH_EXPIRY_TIMER;
      break;

    case qcril::interfaces::ConfigItem::PUBLISH_TIMER_EXTENDED:
      ret = QCRIL_QMI_RADIO_CONFIG_PRESENCE_PUBLISH_EXTENDED_EXPIRY_TIMER;
      break;

    case qcril::interfaces::ConfigItem::CAPABILITIES_CACHE_EXPIRATION:
      ret = QCRIL_QMI_RADIO_CONFIG_PRESENCE_CAPABILITIES_CACHE_EXPIRATION;
      break;

    case qcril::interfaces::ConfigItem::AVAILABILITY_CACHE_EXPIRATION:
      ret = QCRIL_QMI_RADIO_CONFIG_PRESENCE_AVAILABILITY_CACHE_EXPIRATION;
      break;

    case qcril::interfaces::ConfigItem::CAPABILITIES_POLL_INTERVAL:
      ret = QCRIL_QMI_RADIO_CONFIG_PRESENCE_CAPABILITY_POLL_INTERVAL;
      break;

    case qcril::interfaces::ConfigItem::SOURCE_THROTTLE_PUBLISH:
      ret = QCRIL_QMI_RADIO_CONFIG_PRESENCE_MINIMUM_PUBLISH_INTERVAL;
      break;

    case qcril::interfaces::ConfigItem::MAX_NUM_ENTRIES_IN_RCL:
      ret = QCRIL_QMI_RADIO_CONFIG_PRESENCE_MAXIMUM_SUBSCRIPTION_LIST_ENTRIES;
      break;

    case qcril::interfaces::ConfigItem::CAPAB_POLL_LIST_SUB_EXP:
      ret = QCRIL_QMI_RADIO_CONFIG_PRESENCE_CAPABILITY_POLL_LIST_SUBSCRIPTION_EXPIRY_TIMER;
      break;

    case qcril::interfaces::ConfigItem::GZIP_FLAG:
      ret = QCRIL_QMI_RADIO_CONFIG_PRESENCE_GZIP_ENABLED;
      break;

    case qcril::interfaces::ConfigItem::EAB_SETTING_ENABLED:
      ret = QCRIL_QMI_RADIO_CONFIG_CLIENT_PROVISIONING_ENABLE_PRESENCE;
      break;

    case qcril::interfaces::ConfigItem::MOBILE_DATA_ENABLED:
      ret = QCRIL_QMI_RADIO_CONFIG_QIPCALL_MOBILE_DATA_ENABLED;
      break;

    case qcril::interfaces::ConfigItem::VOICE_OVER_WIFI_ENABLED:
      ret = QCRIL_QMI_RADIO_CONFIG_CLIENT_PROVISIONING_WIFI_CALL;
      break;

    case qcril::interfaces::ConfigItem::VOICE_OVER_WIFI_ROAMING:
      ret = QCRIL_QMI_RADIO_CONFIG_CLIENT_PROVISIONING_WIFI_CALL_ROAMING;
      break;

    case qcril::interfaces::ConfigItem::VOICE_OVER_WIFI_MODE:
      ret = QCRIL_QMI_RADIO_CONFIG_CLIENT_PROVISIONING_WIFI_CALL_PREFERENCE;
      break;

    case qcril::interfaces::ConfigItem::SIP_INVITE_REQ_RETX_INTERVAL_MSEC:
      ret = QCRIL_QMI_RADIO_CONFIG_SIP_TIMER_TA_VALUE;
      break;

    case qcril::interfaces::ConfigItem::SIP_INVITE_RSP_WAIT_TIME_MSEC:
      ret = QCRIL_QMI_RADIO_CONFIG_SIP_TIMER_TB_VALUE;
      break;

    case qcril::interfaces::ConfigItem::SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC:
      ret = QCRIL_QMI_RADIO_CONFIG_SIP_TIMER_TD_VALUE;
      break;

    case qcril::interfaces::ConfigItem::SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC:
      ret = QCRIL_QMI_RADIO_CONFIG_SIP_TIMER_TE_VALUE;
      break;

    case qcril::interfaces::ConfigItem::SIP_INVITE_RSP_RETX_INTERVAL_MSEC:
      ret = QCRIL_QMI_RADIO_CONFIG_SIP_TIMER_TG_VALUE;
      break;

    case qcril::interfaces::ConfigItem::SIP_ACK_RECEIPT_WAIT_TIME_MSEC:
      ret = QCRIL_QMI_RADIO_CONFIG_SIP_TIMER_TH_VALUE;
      break;

    case qcril::interfaces::ConfigItem::SIP_ACK_RETX_WAIT_TIME_MSEC:
      ret = QCRIL_QMI_RADIO_CONFIG_SIP_TIMER_TI_VALUE;
      break;

    case qcril::interfaces::ConfigItem::SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC:
      ret = QCRIL_QMI_RADIO_CONFIG_SIP_TIMER_TJ;
      break;

    case qcril::interfaces::ConfigItem::SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC:
      ret = QCRIL_QMI_RADIO_CONFIG_SIP_TIMER_TK_VALUE;
      break;

    case qcril::interfaces::ConfigItem::KEEP_ALIVE_ENABLED:
      ret = QCRIL_QMI_RADIO_CONFIG_SIP_KEEPALIVE_ENABLED;
      break;

    case qcril::interfaces::ConfigItem::REGISTRATION_RETRY_BASE_TIME_SEC:
      ret = QCRIL_QMI_RADIO_CONFIG_REG_MGR_EXTENDED_REG_RETRY_BASE_TIME;
      break;

    case qcril::interfaces::ConfigItem::REGISTRATION_RETRY_MAX_TIME_SEC:
      ret = QCRIL_QMI_RADIO_CONFIG_REG_MGR_EXTENDED_REG_RETRY_MAX_TIME;
      break;

    case qcril::interfaces::ConfigItem::VOLTE_USER_OPT_IN_STATUS:
      ret = QCRIL_QMI_RADIO_CONFIG_PRESENCE_VOLTE_USER_OPTED_IN_STATUS;
      break;

    case qcril::interfaces::ConfigItem::SMS_PSI:
      ret = QCRIL_QMI_RADIO_CONFIG_SMS_PSI;
      break;

    case qcril::interfaces::ConfigItem::SPEECH_START_PORT:
      ret = QCRIL_QMI_RADIO_CONFIG_QIPCALL_SPEECH_START_PORT;
      break;

    case qcril::interfaces::ConfigItem::SPEECH_END_PORT:
      ret = QCRIL_QMI_RADIO_CONFIG_QIPCALL_SPEECH_END_PORT;
      break;

    case qcril::interfaces::ConfigItem::AMR_WB_OCTET_ALIGNED_PT:
      ret = QCRIL_QMI_RADIO_CONFIG_QIPCALL_AMR_WB_OCTET_ALIGNED_DYNAMIC_PT;
      break;

    case qcril::interfaces::ConfigItem::AMR_WB_BANDWIDTH_EFFICIENT_PT:
      ret = QCRIL_QMI_RADIO_CONFIG_QIPCALL_AMR_WB_BANDWIDTH_EFFICIENT_DYNAMIC_PT;
      break;

    case qcril::interfaces::ConfigItem::AMR_OCTET_ALIGNED_PT:
      ret = QCRIL_QMI_RADIO_CONFIG_QIPCALL_AMR_OCTET_ALIGNED_DYNAMIC_PT;
      break;

    case qcril::interfaces::ConfigItem::AMR_BANDWIDTH_EFFICIENT_PT:
      ret = QCRIL_QMI_RADIO_CONFIG_QIPCALL_AMR_BANDWIDTH_EFFICIENT_DYNAMIC_PT;
      break;

    case qcril::interfaces::ConfigItem::DTMF_WB_PT:
      ret = QCRIL_QMI_RADIO_CONFIG_QIPCALL_DTMF_WB_DYNAMIC_POINT;
      break;

    case qcril::interfaces::ConfigItem::DTMF_NB_PT:
      ret = QCRIL_QMI_RADIO_CONFIG_QIPCALL_DTMF_NB_DYNAMIC_PT;
      break;

    case qcril::interfaces::ConfigItem::AMR_DEFAULT_MODE:
      ret = QCRIL_QMI_RADIO_CONFIG_QIPCALL_AMR_DEFAULT_MODE;
      break;

    case qcril::interfaces::ConfigItem::LBO_PCSCF_ADDRESS:
      ret = QCRIL_QMI_RADIO_CONFIG_REG_MGR_CONFIG_REG_MGR_PRIMARY_CSCF;
      break;

    case qcril::interfaces::ConfigItem::EMERGENCY_CALL_TIMER:
      ret = QCRIL_QMI_RADIO_CONFIG_QIPCALL_EMERG_CALL_TIMER;
      break;

    case qcril::interfaces::ConfigItem::CAPABILITY_DISCOVERY_ENABLED:
      ret = QCRIL_QMI_RADIO_CONFIG_PRESENCE_CAPABILITY_DISCOVERY_ENABLE;
      break;

    case qcril::interfaces::ConfigItem::VIDEO_QUALITY:
      ret = QCRIL_QMI_RADIO_CONFIG_QIPCALL_VIDEO_QUALITY;
      break;

    case qcril::interfaces::ConfigItem::THRESHOLD_LTE1:
      ret = QCRIL_QMI_RADIO_CONFIG_HANDOVER_CONFIG_LTE_THRESHOLD1;
      break;

    case qcril::interfaces::ConfigItem::THRESHOLD_LTE2:
      ret = QCRIL_QMI_RADIO_CONFIG_HANDOVER_CONFIG_LTE_THRESHOLD2;
      break;

    case qcril::interfaces::ConfigItem::THRESHOLD_LTE3:
      ret = QCRIL_QMI_RADIO_CONFIG_HANDOVER_CONFIG_LTE_THRESHOLD3;
      break;

    case qcril::interfaces::ConfigItem::THRESHOLD_1x:
      ret = QCRIL_QMI_RADIO_CONFIG_HANDOVER_CONFIG_1X_THRESHOLD;
      break;

    case qcril::interfaces::ConfigItem::THRESHOLD_WIFI_A:
      ret = QCRIL_QMI_RADIO_CONFIG_HANDOVER_CONFIG_WIFI_THRESHOLDA;
      break;

    case qcril::interfaces::ConfigItem::THRESHOLD_WIFI_B:
      ret = QCRIL_QMI_RADIO_CONFIG_HANDOVER_CONFIG_WIFI_THRESHOLDB;
      break;

    case qcril::interfaces::ConfigItem::T_EPDG_LTE:
      ret = QCRIL_QMI_RADIO_CONFIG_HANDOVER_CONFIG_WLAN_TO_WWAN_HYS_TIMER;
      break;

    case qcril::interfaces::ConfigItem::T_EPDG_1x:
      ret = QCRIL_QMI_RADIO_CONFIG_HANDOVER_CONFIG_WLAN_TO_1X_HYS_TIMER;
      break;

    case qcril::interfaces::ConfigItem::T_EPDG_WIFI:
      ret = QCRIL_QMI_RADIO_CONFIG_HANDOVER_CONFIG_WWAN_TO_WLAN_HYS_TIMER;
      break;

    case qcril::interfaces::ConfigItem::VWF_SETTING_ENABLED:
      ret = QCRIL_QMI_RADIO_CONFIG_CLIENT_PROVISIONING_ENABLE_VOWIFI;
      break;

    case qcril::interfaces::ConfigItem::VCE_SETTING_ENABLED:
      ret = QCRIL_QMI_RADIO_CONFIG_QIPCALL_VICE_ENABLED;
      break;

    case qcril::interfaces::ConfigItem::RTT_SETTING_ENABLED:
      ret = QCRIL_QMI_RADIO_CONFIG_CLIENT_PROVISIONING_ENABLE_RTT;
      break;

    case qcril::interfaces::ConfigItem::SMS_APP:
      ret = QCRIL_QMI_RADIO_CONFIG_RCS_SMS_APP;
      break;

    case qcril::interfaces::ConfigItem::VVM_APP:
      ret = QCRIL_QMI_RADIO_CONFIG_RCS_VVM_APP;
      break;

    case qcril::interfaces::ConfigItem::VOICE_OVER_WIFI_ROAMING_MODE:
      ret = QCRIL_QMI_RADIO_CONFIG_WIFI_CALL_ROAMING_PREFERENCE;
      break;

    case qcril::interfaces::ConfigItem::AUTO_REJECT_CALL_ENABLED:
      ret = QCRIL_QMI_RADIO_CONFIG_SET_CONCURRENCY_CONFIG;
      break;

    default:
      ret = QCRIL_QMI_RADIO_CONFIG_INVALID;
      break;

  }
  QCRIL_LOG_INFO("Mapped ims config: %d to radio config: %d", ims_config_item, ret);
  QCRIL_LOG_FUNC_RETURN();
  return ret;
} // qcril_qmi_ims_map_ims_config_to_radio_config_item


//===========================================================================
// qcril_qmi_ims_map_radio_config_to_ims_config_item
//===========================================================================
qcril::interfaces::ConfigItem qcril_qmi_ims_map_radio_config_to_ims_config_item
(
   qcril_qmi_radio_config_item_type radio_config_item
)
{
  QCRIL_LOG_FUNC_ENTRY();
  qcril::interfaces::ConfigItem ret = qcril::interfaces::ConfigItem::NONE;

  switch(radio_config_item)
  {
    case QCRIL_QMI_RADIO_CONFIG_VOIP_AMR_MODE:
      ret = qcril::interfaces::ConfigItem::VOCODER_AMRMODESET;
      break;

    case QCRIL_QMI_RADIO_CONFIG_VOIP_AMR_WB_MODE:
      ret = qcril::interfaces::ConfigItem::VOCODER_AMRWBMODESET;
      break;

    case QCRIL_QMI_RADIO_CONFIG_VOIP_SESSION_EXPIRY_TIMER:
      ret = qcril::interfaces::ConfigItem::SIP_SESSION_TIMER;
      break;

    case QCRIL_QMI_RADIO_CONFIG_VOIP_MIN_SESSION_EXPIRY:
      ret = qcril::interfaces::ConfigItem::MIN_SESSION_EXPIRY;
      break;

    case QCRIL_QMI_RADIO_CONFIG_SIP_TIMER_OPERATOR_MODE_A:
      ret = qcril::interfaces::ConfigItem::CANCELLATION_TIMER;
      break;

    case QCRIL_QMI_RADIO_CONFIG_REG_MGR_EXTENDED_T_DELAY:
      ret = qcril::interfaces::ConfigItem::T_DELAY;
      break;

    case QCRIL_QMI_RADIO_CONFIG_REG_MGR_EXTENDED_REG_RETRY_BASE_TIME:
      ret = qcril::interfaces::ConfigItem::REGISTRATION_RETRY_BASE_TIME_SEC;
      break;

    case QCRIL_QMI_RADIO_CONFIG_REG_MGR_EXTENDED_REG_RETRY_MAX_TIME:
      ret = qcril::interfaces::ConfigItem::REGISTRATION_RETRY_MAX_TIME_SEC;
      break;

    case QCRIL_QMI_RADIO_CONFIG_VOIP_SILENT_REDIAL_ENABLED:
      ret = qcril::interfaces::ConfigItem::SILENT_REDIAL_ENABLE;
      break;

    case QCRIL_QMI_RADIO_CONFIG_SIP_TIMER_T1:
      ret = qcril::interfaces::ConfigItem::SIP_T1_TIMER;
      break;

    case QCRIL_QMI_RADIO_CONFIG_SIP_TIMER_T2:
      ret = qcril::interfaces::ConfigItem::SIP_T2_TIMER;
      break;

    case QCRIL_QMI_RADIO_CONFIG_SIP_TIMER_TF:
      ret = qcril::interfaces::ConfigItem::SIP_TF_TIMER;
      break;

    case QCRIL_QMI_RADIO_CONFIG_SIP_TIMER_TA_VALUE:
      ret = qcril::interfaces::ConfigItem::SIP_INVITE_REQ_RETX_INTERVAL_MSEC;
      break;

    case QCRIL_QMI_RADIO_CONFIG_SIP_TIMER_TB_VALUE:
      ret = qcril::interfaces::ConfigItem::SIP_INVITE_RSP_WAIT_TIME_MSEC;
      break;

    case QCRIL_QMI_RADIO_CONFIG_SIP_TIMER_TD_VALUE:
      ret = qcril::interfaces::ConfigItem::SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC;
      break;

    case QCRIL_QMI_RADIO_CONFIG_SIP_TIMER_TE_VALUE:
      ret = qcril::interfaces::ConfigItem::SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC;
      break;

    case QCRIL_QMI_RADIO_CONFIG_SIP_TIMER_TG_VALUE:
      ret = qcril::interfaces::ConfigItem::SIP_INVITE_RSP_RETX_INTERVAL_MSEC;
      break;

    case QCRIL_QMI_RADIO_CONFIG_SIP_TIMER_TH_VALUE:
      ret = qcril::interfaces::ConfigItem::SIP_ACK_RECEIPT_WAIT_TIME_MSEC;
      break;

    case QCRIL_QMI_RADIO_CONFIG_SIP_TIMER_TI_VALUE:
      ret = qcril::interfaces::ConfigItem::SIP_ACK_RETX_WAIT_TIME_MSEC;
      break;

    case QCRIL_QMI_RADIO_CONFIG_SIP_TIMER_TJ:
      ret = qcril::interfaces::ConfigItem::SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC;
      break;

    case QCRIL_QMI_RADIO_CONFIG_SIP_TIMER_TK_VALUE:
      ret = qcril::interfaces::ConfigItem::SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC;
      break;

    case QCRIL_QMI_RADIO_CONFIG_SIP_KEEPALIVE_ENABLED:
      ret = qcril::interfaces::ConfigItem::KEEP_ALIVE_ENABLED;
      break;

    case QCRIL_QMI_RADIO_CONFIG_CLIENT_PROVISIONING_ENABLE_VOLTE:
      ret = qcril::interfaces::ConfigItem::VLT_SETTING_ENABLED;
      break;

    case QCRIL_QMI_RADIO_CONFIG_CLIENT_PROVISIONING_ENABLE_VT:
      ret = qcril::interfaces::ConfigItem::LVC_SETTING_ENABLED;
      break;

    case QCRIL_QMI_RADIO_CONFIG_USER_IMS_DOMAIN_NAME:
      ret = qcril::interfaces::ConfigItem::DOMAIN_NAME;
      break;

    case QCRIL_QMI_RADIO_CONFIG_SMS_FORMAT:
      ret = qcril::interfaces::ConfigItem::SMS_FORMAT;
      break;

    case QCRIL_QMI_RADIO_CONFIG_SMS_OVER_IP:
      ret = qcril::interfaces::ConfigItem::SMS_OVER_IP;
      break;

    case QCRIL_QMI_RADIO_CONFIG_SMS_PSI:
      ret = qcril::interfaces::ConfigItem::SMS_PSI;
      break;

    case QCRIL_QMI_RADIO_CONFIG_PRESENCE_PUBLISH_EXPIRY_TIMER:
      ret = qcril::interfaces::ConfigItem::PUBLISH_TIMER;
      break;

    case QCRIL_QMI_RADIO_CONFIG_PRESENCE_PUBLISH_EXTENDED_EXPIRY_TIMER:
      ret = qcril::interfaces::ConfigItem::PUBLISH_TIMER_EXTENDED;
      break;

    case QCRIL_QMI_RADIO_CONFIG_PRESENCE_CAPABILITIES_CACHE_EXPIRATION:
      ret = qcril::interfaces::ConfigItem::CAPABILITIES_CACHE_EXPIRATION;
      break;

    case QCRIL_QMI_RADIO_CONFIG_PRESENCE_AVAILABILITY_CACHE_EXPIRATION:
      ret = qcril::interfaces::ConfigItem::AVAILABILITY_CACHE_EXPIRATION;
      break;

    case QCRIL_QMI_RADIO_CONFIG_PRESENCE_CAPABILITY_POLL_INTERVAL:
      ret = qcril::interfaces::ConfigItem::CAPABILITIES_POLL_INTERVAL;
      break;

    case QCRIL_QMI_RADIO_CONFIG_PRESENCE_MINIMUM_PUBLISH_INTERVAL:
      ret = qcril::interfaces::ConfigItem::SOURCE_THROTTLE_PUBLISH;
      break;

    case QCRIL_QMI_RADIO_CONFIG_PRESENCE_MAXIMUM_SUBSCRIPTION_LIST_ENTRIES:
      ret = qcril::interfaces::ConfigItem::MAX_NUM_ENTRIES_IN_RCL;
      break;

    case QCRIL_QMI_RADIO_CONFIG_PRESENCE_CAPABILITY_POLL_LIST_SUBSCRIPTION_EXPIRY_TIMER:
      ret = qcril::interfaces::ConfigItem::CAPAB_POLL_LIST_SUB_EXP;
      break;

    case QCRIL_QMI_RADIO_CONFIG_PRESENCE_GZIP_ENABLED:
      ret = qcril::interfaces::ConfigItem::GZIP_FLAG;
      break;

    case QCRIL_QMI_RADIO_CONFIG_PRESENCE_VOLTE_USER_OPTED_IN_STATUS:
      ret = qcril::interfaces::ConfigItem::VOLTE_USER_OPT_IN_STATUS;
      break;

    case QCRIL_QMI_RADIO_CONFIG_PRESENCE_CAPABILITY_DISCOVERY_ENABLE:
      ret = qcril::interfaces::ConfigItem::CAPABILITY_DISCOVERY_ENABLED;
      break;

    case QCRIL_QMI_RADIO_CONFIG_CLIENT_PROVISIONING_ENABLE_PRESENCE:
      ret = qcril::interfaces::ConfigItem::EAB_SETTING_ENABLED;
      break;

    case QCRIL_QMI_RADIO_CONFIG_QIPCALL_MOBILE_DATA_ENABLED:
      ret = qcril::interfaces::ConfigItem::MOBILE_DATA_ENABLED;
      break;

    case QCRIL_QMI_RADIO_CONFIG_QIPCALL_SPEECH_START_PORT:
      ret = qcril::interfaces::ConfigItem::SPEECH_START_PORT;
      break;

    case QCRIL_QMI_RADIO_CONFIG_QIPCALL_SPEECH_END_PORT:
      ret = qcril::interfaces::ConfigItem::SPEECH_END_PORT;
      break;

    case QCRIL_QMI_RADIO_CONFIG_QIPCALL_AMR_WB_OCTET_ALIGNED_DYNAMIC_PT:
      ret = qcril::interfaces::ConfigItem::AMR_WB_OCTET_ALIGNED_PT;
      break;

    case QCRIL_QMI_RADIO_CONFIG_QIPCALL_AMR_WB_BANDWIDTH_EFFICIENT_DYNAMIC_PT:
      ret = qcril::interfaces::ConfigItem::AMR_WB_BANDWIDTH_EFFICIENT_PT;
      break;

    case QCRIL_QMI_RADIO_CONFIG_QIPCALL_AMR_OCTET_ALIGNED_DYNAMIC_PT:
      ret = qcril::interfaces::ConfigItem::AMR_OCTET_ALIGNED_PT;
      break;

    case QCRIL_QMI_RADIO_CONFIG_QIPCALL_AMR_BANDWIDTH_EFFICIENT_DYNAMIC_PT:
      ret = qcril::interfaces::ConfigItem::AMR_BANDWIDTH_EFFICIENT_PT;
      break;

    case QCRIL_QMI_RADIO_CONFIG_QIPCALL_DTMF_WB_DYNAMIC_POINT:
      ret = qcril::interfaces::ConfigItem::DTMF_WB_PT;
      break;

    case QCRIL_QMI_RADIO_CONFIG_QIPCALL_DTMF_NB_DYNAMIC_PT:
      ret = qcril::interfaces::ConfigItem::DTMF_NB_PT;
      break;

    case QCRIL_QMI_RADIO_CONFIG_QIPCALL_AMR_DEFAULT_MODE:
      ret = qcril::interfaces::ConfigItem::AMR_DEFAULT_MODE;
      break;

    case QCRIL_QMI_RADIO_CONFIG_QIPCALL_EMERG_CALL_TIMER:
      ret = qcril::interfaces::ConfigItem::EMERGENCY_CALL_TIMER;
      break;

    case QCRIL_QMI_RADIO_CONFIG_CLIENT_PROVISIONING_WIFI_CALL:
      ret = qcril::interfaces::ConfigItem::VOICE_OVER_WIFI_ENABLED;
      break;

    case QCRIL_QMI_RADIO_CONFIG_CLIENT_PROVISIONING_WIFI_CALL_ROAMING:
      ret = qcril::interfaces::ConfigItem::VOICE_OVER_WIFI_ROAMING;
      break;

    case QCRIL_QMI_RADIO_CONFIG_CLIENT_PROVISIONING_WIFI_CALL_PREFERENCE:
      ret = qcril::interfaces::ConfigItem::VOICE_OVER_WIFI_MODE;
      break;

    case QCRIL_QMI_RADIO_CONFIG_REG_MGR_CONFIG_REG_MGR_PRIMARY_CSCF:
      ret = qcril::interfaces::ConfigItem::LBO_PCSCF_ADDRESS;
      break;

    case QCRIL_QMI_RADIO_CONFIG_QIPCALL_VIDEO_QUALITY:
      ret = qcril::interfaces::ConfigItem::VIDEO_QUALITY;
      break;

    case QCRIL_QMI_RADIO_CONFIG_HANDOVER_CONFIG_LTE_THRESHOLD1:
      ret = qcril::interfaces::ConfigItem::THRESHOLD_LTE1;
      break;

    case QCRIL_QMI_RADIO_CONFIG_HANDOVER_CONFIG_LTE_THRESHOLD2:
      ret = qcril::interfaces::ConfigItem::THRESHOLD_LTE2;
      break;

    case QCRIL_QMI_RADIO_CONFIG_HANDOVER_CONFIG_LTE_THRESHOLD3:
      ret = qcril::interfaces::ConfigItem::THRESHOLD_LTE3;
      break;

    case QCRIL_QMI_RADIO_CONFIG_HANDOVER_CONFIG_1X_THRESHOLD:
      ret = qcril::interfaces::ConfigItem::THRESHOLD_1x;
      break;

    case QCRIL_QMI_RADIO_CONFIG_HANDOVER_CONFIG_WIFI_THRESHOLDA:
      ret = qcril::interfaces::ConfigItem::THRESHOLD_WIFI_A;
      break;

    case QCRIL_QMI_RADIO_CONFIG_HANDOVER_CONFIG_WIFI_THRESHOLDB:
      ret = qcril::interfaces::ConfigItem::THRESHOLD_WIFI_B;
      break;

    case QCRIL_QMI_RADIO_CONFIG_HANDOVER_CONFIG_WLAN_TO_WWAN_HYS_TIMER:
      ret = qcril::interfaces::ConfigItem::T_EPDG_LTE;
      break;

    case QCRIL_QMI_RADIO_CONFIG_HANDOVER_CONFIG_WLAN_TO_1X_HYS_TIMER:
      ret = qcril::interfaces::ConfigItem::T_EPDG_1x;
      break;

    case QCRIL_QMI_RADIO_CONFIG_HANDOVER_CONFIG_WWAN_TO_WLAN_HYS_TIMER:
      ret = qcril::interfaces::ConfigItem::T_EPDG_WIFI;
      break;

    case QCRIL_QMI_RADIO_CONFIG_CLIENT_PROVISIONING_ENABLE_VOWIFI:
      ret = qcril::interfaces::ConfigItem::VWF_SETTING_ENABLED;
      break;

    case QCRIL_QMI_RADIO_CONFIG_QIPCALL_VICE_ENABLED:
      ret = qcril::interfaces::ConfigItem::VCE_SETTING_ENABLED;
      break;

    case QCRIL_QMI_RADIO_CONFIG_CLIENT_PROVISIONING_ENABLE_RTT:
      ret = qcril::interfaces::ConfigItem::RTT_SETTING_ENABLED;
      break;

    case QCRIL_QMI_RADIO_CONFIG_RCS_SMS_APP:
      ret = qcril::interfaces::ConfigItem::SMS_APP;
      break;

    case QCRIL_QMI_RADIO_CONFIG_RCS_VVM_APP:
      ret = qcril::interfaces::ConfigItem::VVM_APP;
      break;

    case QCRIL_QMI_RADIO_CONFIG_WIFI_CALL_ROAMING_PREFERENCE:
      ret = qcril::interfaces::ConfigItem::VOICE_OVER_WIFI_ROAMING_MODE;
      break;

    case QCRIL_QMI_RADIO_CONFIG_SET_CONCURRENCY_CONFIG:
      ret = qcril::interfaces::ConfigItem::AUTO_REJECT_CALL_ENABLED;
      break;

    default:
      ret = qcril::interfaces::ConfigItem::NONE;
      break;

  }
  QCRIL_LOG_INFO("Mapped radio config item: %d to ims config item: %d", radio_config_item, ret);
  QCRIL_LOG_FUNC_RETURN();
  return ret;
} // qcril_qmi_ims_map_radio_config_to_ims_config_item

//===========================================================================
// qcril_qmi_ims_map_radio_config_settings_resp_to_ims_config_failcause
//===========================================================================
qcril::interfaces::ConfigFailureCause qcril_qmi_ims_map_radio_config_settings_resp_to_ims_config_failcause
(
  qcril_qmi_radio_config_settings_resp_type radio_config_error
)
{
  qcril::interfaces::ConfigFailureCause ret = qcril::interfaces::ConfigFailureCause::OTHER_INTERNAL_ERR;

  switch(radio_config_error)
  {
    case QCRIL_QMI_RADIO_CONFIG_SETTINGS_RESP_NO_ERR:
      ret = qcril::interfaces::ConfigFailureCause::NO_ERR;
      break;

    case QCRIL_QMI_RADIO_CONFIG_SETTINGS_RESP_IMS_NOT_READY:
      ret = qcril::interfaces::ConfigFailureCause::IMS_NOT_READY;
      break;

    case QCRIL_QMI_RADIO_CONFIG_SETTINGS_RESP_FILE_NOT_AVAILABLE:
      ret = qcril::interfaces::ConfigFailureCause::FILE_NOT_AVAILABLE;
      break;

    case QCRIL_QMI_RADIO_CONFIG_SETTINGS_RESP_READ_FAILED:
      ret = qcril::interfaces::ConfigFailureCause::READ_FAILED;
      break;

    case QCRIL_QMI_RADIO_CONFIG_SETTINGS_RESP_WRITE_FAILED:
      ret = qcril::interfaces::ConfigFailureCause::WRITE_FAILED;
      break;

    case QCRIL_QMI_RADIO_CONFIG_SETTINGS_RESP_OTHER_INTERNAL_ERR:
      ret = qcril::interfaces::ConfigFailureCause::OTHER_INTERNAL_ERR;
      break;

    default:
      ret = qcril::interfaces::ConfigFailureCause::OTHER_INTERNAL_ERR;
      break;
  }

  QCRIL_LOG_INFO("radio config error: %d mapped to ims config failcause %d",
                 radio_config_error, ret);

  return ret;
}//qcril_qmi_ims_map_radio_config_settings_resp_to_ims_config_failcause

qcril::interfaces::VoiceInfo qcril_qmi_ims_map_voice_info
(
 imsa_voice_info_enum_v01 qmi_voice_info
)
{
    switch (qmi_voice_info)
    {
      case IMSA_VOICE_INFO_SILENT_V01:
        return qcril::interfaces::VoiceInfo::SILENT;
      case IMSA_VOICE_INFO_SPEECH_V01:
        return qcril::interfaces::VoiceInfo::SPEECH;
      default:
        return qcril::interfaces::VoiceInfo::UNKNOWN;
    }
    return qcril::interfaces::VoiceInfo::UNKNOWN;
}
