/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once

#include <interfaces/ims/ims.h>
#include "ip_multimedia_subsystem_application_v01.h"
#include "ip_multimedia_subsystem_settings_v01.h"
#include "qcril_qmi_radio_config_meta.h"

// using namespace qcril::interfaces;

qcril::interfaces::RegState qcril_qmi_ims_map_qmi_ims_reg_state_to_ims_reg_state(
    uint8_t ims_registered);

RIL_RadioTechnology qcril_qmi_ims_map_imsa_rat_to_ims_radio_tech(
    imsa_service_rat_enum_v01 imsa_rat);

std::shared_ptr<qcril::interfaces::Registration> qcril_qmi_ims_create_Registration(
    uint8_t regStateValid, imsa_ims_registration_status_enum_v01 regState, uint8_t ratValid,
    imsa_service_rat_enum_v01 rat, uint8_t errorCodeValid, uint16_t errorCode, char *errorString,
    uint32_t uriListLen, p_associated_uri_v01 *uriList);

std::shared_ptr<qcril::interfaces::ServiceStatusInfoList>
qcril_qmi_ims_create_ServiceStatusInfoList(const imsa_service_status_ind_msg_v01 *qmi_data);

qcril::interfaces::VowifiQuality qcril_qmi_ims_map_qmi_wifi_quality_to_ims_wifi_quality(
 imsa_vowifiprofile_status_enum_v01 qmi_wifi_quality
);

qcril::interfaces::BlockReasonType qcril_qmi_ims_map_qmi_block_reason_to_ims_block_reason
(
  imsa_block_reason_type_mask_v01 qmi_block_reason
);

qcril::interfaces::RegFailureReasonType qcril_qmi_call_end_type_to_ims_reg_failure
(
  imsa_verbose_call_end_reason_type_enum_v01 qmi_block_reason
);
qcril_qmi_radio_config_item_type qcril_qmi_ims_map_ims_config_to_radio_config_item
(
   qcril::interfaces::ConfigItem ims_config_item
);

/* Maps the radio config item to the ims config item */
qcril::interfaces::ConfigItem qcril_qmi_ims_map_radio_config_to_ims_config_item
(
   qcril_qmi_radio_config_item_type ims_config_item
);
qcril::interfaces::ConfigFailureCause qcril_qmi_ims_map_radio_config_settings_resp_to_ims_config_failcause
(
  qcril_qmi_radio_config_settings_resp_type radio_config_error
);

qcril::interfaces::VoiceInfo qcril_qmi_ims_map_voice_info
(
 imsa_voice_info_enum_v01 qmi_voice_info
);
