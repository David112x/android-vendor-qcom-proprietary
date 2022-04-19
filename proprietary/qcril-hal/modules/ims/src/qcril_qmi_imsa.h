/******************************************************************************
#  Copyright (c) 2013, 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
/******************************************************************************
  @file    qcril_qmi_imsa.h
  @brief   qcril qmi - IMS Application

  DESCRIPTION
    Handles RIL requests, Callbacks, indications for QMI IMS Application.

******************************************************************************/

#ifndef QCRIL_QMI_IMSA_H
#define QCRIL_QMI_IMSA_H

#include "qmi_client.h"
#include "qcrili.h"
#include "ip_multimedia_subsystem_application_v01.h"
#include "device_management_service_v01.h"

#ifdef __cplusplus
#include "modules/ims/ImsServiceStatusInd.h"
#include "modules/ims/ImsWfcSettingsStatusInd.h"
#include "interfaces/ims/QcRilRequestImsGetRegStateMessage.h"
#include "interfaces/ims/QcRilRequestImsQueryServiceStatusMessage.h"

#include "interfaces/ims/QcRilRequestImsGetRtpStatsMessage.h"
#include "interfaces/ims/QcRilRequestImsGetSubConfigMessage.h"
#include "interfaces/ims/QcRilRequestImsGetRtpErrorStatsMessage.h"
#include "interfaces/ims/QcRilRequestImsGeoLocationInfoMessage.h"
#include "interfaces/ims/QcRilRequestImsQueryVirtualLineInfo.h"

#include "modules/sms/SmsImsServiceStatusInd.h"
#endif

#define qcril_qmi_imsa_srv_status_type imsa_service_status_ind_msg_v01

#ifdef __cplusplus
typedef struct
{
   qtimutex::QtiRecursiveMutex imsa_info_lock_mutex;
   boolean inited;
   boolean ims_status_change_registered;
   uint8_t ims_registered_valid;
   imsa_ims_registration_status_enum_v01 ims_registered;
   uint8_t ims_registration_error_code_valid;
   uint16_t ims_registration_error_code;
   uint8_t ims_registration_error_string_valid;
   char *ims_registration_error_string;
   uint8_t ims_srv_status_valid;
   imsa_service_status_ind_msg_v01 ims_srv_status;
   uint8_t registration_network_valid;
   imsa_service_rat_enum_v01 registration_network;
   uint8_t uri_list_valid;
   uint32_t uri_list_len;
   p_associated_uri_v01 URI_List[IMSA_MAX_NUM_URIS_V01];
   uint32_t ims_capability_len;
   dms_ims_capability_type_v01 ims_capability[QMI_DMS_MAX_SUBSCRIPTION_LIST_LEN_V01];
   uint8_t max_ims_instances_valid;
   uint8_t max_ims_instances;
} qcril_qmi_imsa_info_type;
#endif

void qcril_qmi_imsa_pre_init(void);

void qcril_qmi_imsa_init();

void qcril_qmi_imsa_cleanup();

void qcril_qmi_imsa_set_init_state(boolean inited);

#ifdef __cplusplus
void qcril_qmi_imsa_request_ims_registration_state(
    std::shared_ptr<QcRilRequestImsGetRegStateMessage> msg);
void qcril_qmi_imsa_request_query_ims_srv_status(
    std::shared_ptr<QcRilRequestImsQueryServiceStatusMessage> msg);
void qcril_qmi_imsa_request_get_rtp_statistics(
    std::shared_ptr<QcRilRequestImsGetRtpStatsMessage> msg);
void qcril_qmi_imsa_request_get_rtp_error_statistics(
    std::shared_ptr<QcRilRequestImsGetRtpErrorStatsMessage> msg);
void qcril_qmi_imsa_request_get_ims_sub_config(
    std::shared_ptr<QcRilRequestImsGetSubConfigMessage> msg);
void qcril_qmi_imsa_set_geo_loc(
    std::shared_ptr<QcRilRequestImsGeoLocationInfoMessage> msg);
void qcril_qmi_imsa_request_query_virtual_line_info(
    std::shared_ptr<QcRilRequestImsQueryVirtualLineInfo> msg);
#endif




#ifdef __cplusplus
void qcril_qmi_imsa_command_cb
(
   unsigned int                 msg_id,
   std::shared_ptr<void>        resp_c_struct,
   unsigned int                 resp_c_struct_len,
   void                        *resp_cb_data,
   qmi_client_error_type        transp_err
);

void qcril_qmi_imsa_set_geo_loc_resp_hdlr
(
   unsigned int                 msg_id,
   std::shared_ptr<void>        resp_c_struct,
   unsigned int                 resp_c_struct_len,
   void                        *resp_cb_data,
   qmi_client_error_type        transp_err
);


struct VirtualLineInfo{
    std::string mMsisdn;
    const std::vector<std::string>& mVirtualLines;
    VirtualLineInfo(const std::string& msisdn, const std::vector<std::string>& lines) :
           mMsisdn(msisdn), mVirtualLines(lines) {
    }
};

std::shared_ptr<qcril::interfaces::RegistrationBlockStatus>
copy_qmi_blk_status_to_ims_registration_block_status
(
    imsa_ims_block_status_ind_msg_v01 *src
);
void qcril_qmi_imsa_sms_ims_service_status_hdlr(SmsImsServiceStatusInd::SmsImsServiceStatus &sms_status);
#endif

void qcril_qmi_imsa_unsol_ind_cb_helper
(
 unsigned int   msg_id,
 unsigned char *decoded_payload,
 uint32_t       decoded_payload_len
);

void qcril_qmi_imsa_ims_capability_ind_hdlr
(
  dms_ims_capability_ind_msg_v01 * payload
);

void qcril_qmi_imsa_store_ims_capability
(
    uint8_t max_ims_instances_valid,
    uint8_t max_ims_instances,
    uint8_t ims_capability_valid,
    uint32_t ims_capability_len,
    dms_ims_capability_type_v01 ims_capability[]
);


#ifdef __cplusplus
void qcril_qmi_imsa_broadcast_service_status
(
 const qcril_qmi_imsa_info_type& qcril_qmi_imsa_info
);
#endif

#endif
