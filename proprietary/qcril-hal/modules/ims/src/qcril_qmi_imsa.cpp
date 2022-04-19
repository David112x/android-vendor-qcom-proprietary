/******************************************************************************
#  Copyright (c) 2013, 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
/******************************************************************************
  @file    qcril_qmi_imsa.c
  @brief   qcril qmi - IMS Application

  DESCRIPTION
    Handles RIL requests, Callbacks, indications for QMI IMS Application.

******************************************************************************/

#define TAG "QCRIL_IMSA"

#include "qcril_qmi_imsa.h"
#include "qcril_qmi_client.h"
#include "ip_multimedia_subsystem_application_v01.h"
#include "ip_multimedia_subsystem_settings_v01.h"
#include "qcril_reqlist.h"
#include "qcril_qmi_imss.h"
#include "modules/nas/qcril_arb.h"
#include "qcril_cm_ss.h"
#include "qcril_qmi_imsa_qmi.h"
#include "qcril_qmi_ims_utils.h"
#include "qcril_am.h"
#include "modules/ims/ImsPdpStatusInd.h"
#include "modules/sms/SmsGetImsServiceStatusMessage.h"

#include <framework/Dispatcher.h>
#include <framework/Log.h>
#include <interfaces/ims/QcRilUnsolImsGeoLocationInfo.h>
#include <interfaces/ims/QcRilUnsolImsMultiIdentityStatusMessage.h>
#include <interfaces/ims/QcRilUnsolImsPendingMultiLineStatus.h>
#include <interfaces/ims/QcRilUnsolImsRegBlockStatusMessage.h>
#include <interfaces/ims/QcRilUnsolImsRegStateMessage.h>
#include <interfaces/ims/QcRilUnsolImsSrvStatusIndication.h>
#include <interfaces/ims/QcRilUnsolImsSubConfigIndication.h>
#include <interfaces/ims/QcRilUnsolImsVoiceInfo.h>
#include <interfaces/ims/QcRilUnsolImsVowifiCallQuality.h>
#include <interfaces/ims/ims.h>
#include <interfaces/voice/QcRilUnsolImsHandoverMessage.h>
#include <modules/ims/ImsModule.h>
#include <modules/ims/ImsPdpStatusInd.h>

using namespace qcril::interfaces;

qcril_qmi_imsa_info_type qcril_qmi_imsa_info;

static void qcril_qmi_imsa_store_service_status_cl(
    uint8_t sms_service_status_valid,
    imsa_service_status_enum_v01 sms_service_status,
    uint8_t voip_service_status_valid,
    imsa_service_status_enum_v01 voip_service_status,
    uint8_t vt_service_status_valid,
    imsa_service_status_enum_v01 vt_service_status,
    uint8_t sms_service_rat_valid,
    imsa_service_rat_enum_v01 sms_service_rat,
    uint8_t voip_service_rat_valid,
    imsa_service_rat_enum_v01 voip_service_rat,
    uint8_t vt_service_rat_valid,
    imsa_service_rat_enum_v01 vt_service_rat,
    uint8_t ut_service_status_valid,
    imsa_service_status_enum_v01 ut_service_status,
    uint8_t ut_service_rat_valid,
    imsa_service_rat_enum_v01 ut_service_rat
);

static void qcril_qmi_imsa_store_registration_status(
    uint8_t ims_registered_valid,
    imsa_ims_registration_status_enum_v01 ims_registered,
    uint8_t ims_registration_error_code_valid,
    uint16_t ims_registration_error_code,
    uint8_t ims_registration_error_string_valid,
    char *ims_registration_error_string,
    uint8_t registration_network_valid,
    imsa_service_rat_enum_v01 registration_network,
    uint8_t uri_list_valid,
    p_associated_uri_v01 *URI_List,
    uint32_t uri_list_len
);

static void qcril_qmi_imsa_get_ims_sub_configs(uint64_t* ims_sub_configs, uint8_t max_cap_len);

std::shared_ptr<qcril::interfaces::BlockStatus>
qcril_qmi_imsa_map_imsa_block_status_to_ims_block_status
(
    imsa_ims_failure_reason_type_v01* imsa_block_status
);

//===========================================================================
// qcril_qmi_imsa_pre_init
//===========================================================================
void qcril_qmi_imsa_pre_init(void)
{
} // qcril_qmi_imsa_pre_init

//===========================================================================
// qcril_qmi_imsa_info_lock
//===========================================================================
void qcril_qmi_imsa_info_lock()
{
  qcril_qmi_imsa_info.imsa_info_lock_mutex.lock();
} // qcril_qmi_imsa_info_lock

//===========================================================================
// qcril_qmi_imsa_info_unlock
//===========================================================================
void qcril_qmi_imsa_info_unlock()
{
  qcril_qmi_imsa_info.imsa_info_lock_mutex.unlock();
} // qcril_qmi_imsa_info_unlock

//===========================================================================
// qcril_qmi_imsa_set_init_state
//===========================================================================
void qcril_qmi_imsa_set_init_state(boolean inited)
{
   qcril_qmi_imsa_info_lock();
   qcril_qmi_imsa_info.inited = inited;
   qcril_qmi_imsa_info_unlock();
} // qcril_qmi_imsa_set_init_state

//===========================================================================
// qcril_qmi_imsa_init
//===========================================================================
void qcril_qmi_imsa_init()
{
   QCRIL_LOG_FUNC_ENTRY();

   qcril_qmi_imsa_info_lock();
   qcril_qmi_imsa_info.inited = TRUE;
   qcril_qmi_imsa_info.ims_registered_valid = FALSE;
   qcril_qmi_imsa_info.registration_network_valid = FALSE;
   qcril_qmi_imsa_info.ims_status_change_registered = FALSE;
   qcril_qmi_imsa_info.ims_srv_status_valid = FALSE;
   qcril_qmi_imsa_info.uri_list_valid = FALSE;
   qcril_qmi_imsa_info_unlock();

   // register for indication
   qmi_client_error_type ret = QMI_NO_ERR;
   imsa_ind_reg_req_msg_v01 ind_reg_req;

   memset(&ind_reg_req, 0, sizeof(ind_reg_req));
   ind_reg_req.reg_status_config_valid = TRUE;
   ind_reg_req.reg_status_config = 1;  // enable
   ind_reg_req.service_status_config_valid = TRUE;
   ind_reg_req.service_status_config = 1;  // enable
   ind_reg_req.rat_handover_config_valid = TRUE;
   ind_reg_req.rat_handover_config = 1;  // enable
   ind_reg_req.geo_location_status_config_valid = TRUE;
   ind_reg_req.geo_location_status_config = 1; // enable
   ind_reg_req.vowifiprofile_status_config_valid = TRUE;
   ind_reg_req.vowifiprofile_status_config = 1; // enable
   ind_reg_req.ims_block_status_config_valid = TRUE;
   ind_reg_req.ims_block_status_config = 1;
   ind_reg_req.voice_config_valid = TRUE;
   ind_reg_req.voice_config = 1;
   ind_reg_req.pdp_status_config_valid = TRUE;
   ind_reg_req.pdp_status_config = 1;

   ret = qmi_client_imsa_send_async(QMI_IMSA_IND_REG_REQ_V01,
       (void*) &ind_reg_req,
       sizeof(imsa_ind_reg_req_msg_v01),
       sizeof(imsa_ind_reg_rsp_msg_v01),
       qcril_qmi_imsa_command_cb,
       (void*)0);

   QCRIL_LOG_DEBUG("QMI_IMSA_IND_REG_REQ ret = %d", ret);
   if (QMI_NO_ERR != ret)
   {
     QCRIL_LOG_ERROR("registration for indication failed %d", ret);
   }

   QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imsa_init

//===========================================================================
// qcril_qmi_imsa_cleanup
//===========================================================================
void qcril_qmi_imsa_cleanup()
{
   QCRIL_LOG_FUNC_ENTRY();

   qcril_qmi_imsa_info_lock();

   qcril_qmi_imsa_store_registration_status(
       TRUE, IMSA_STATUS_NOT_REGISTERED_V01,        // ims_registered
       FALSE, 0,                                    // ims_registration_error_code
       FALSE, NULL,                                 // ims_registration_error_string
       FALSE, IMSA_WWAN_V01,                        // registration_network
       FALSE, NULL, 0);                             // URI_List

   qcril_qmi_imsa_store_service_status_cl(
       TRUE, IMSA_NO_SERVICE_V01,  // SMS SERVICE_STATUS
       TRUE, IMSA_NO_SERVICE_V01,  // VOIP SERVICE_STATUS
       TRUE, IMSA_NO_SERVICE_V01,  // VT SERVICE_STATUS
       FALSE, IMSA_WWAN_V01,       // SMS SERVICE_RAT
       FALSE, IMSA_WWAN_V01,       // VOIP SERVICE_RAT
       FALSE, IMSA_WWAN_V01,       // VT SERVICE_RAT
       TRUE, IMSA_NO_SERVICE_V01,  // UT SERVICE_STATUS
       FALSE, IMSA_WWAN_V01);      // UT SERVICE_RAT

   qcril_qmi_imsa_info.ims_registered_valid = FALSE;
   qcril_qmi_imsa_info.ims_registration_error_code_valid = FALSE;
   qcril_qmi_imsa_info.ims_registration_error_string_valid = FALSE;
   qcril_qmi_imsa_info.registration_network_valid = FALSE;
   qcril_qmi_imsa_info.ims_status_change_registered = FALSE;
   qcril_qmi_imsa_info.uri_list_valid = FALSE;
   qcril_qmi_imsa_info.uri_list_len = 0;
   qcril_qmi_imsa_info.ims_capability_len = 0;
   qcril_qmi_imsa_info.max_ims_instances_valid = FALSE;

   qcril_qmi_imsa_info.ims_srv_status_valid = FALSE;
   qcril_qmi_imsa_info.ims_srv_status.sms_service_status_valid = FALSE;
   qcril_qmi_imsa_info.ims_srv_status.voip_service_status_valid = FALSE;
   qcril_qmi_imsa_info.ims_srv_status.vt_service_status_valid = FALSE;
   qcril_qmi_imsa_info.ims_srv_status.ut_service_status_valid = FALSE;
   qcril_qmi_imsa_info.ims_srv_status.sms_service_rat_valid = FALSE;
   qcril_qmi_imsa_info.ims_srv_status.voip_service_rat_valid = FALSE;
   qcril_qmi_imsa_info.ims_srv_status.vt_service_rat_valid = FALSE;
   qcril_qmi_imsa_info.ims_srv_status.ut_service_rat_valid = FALSE;

   qcril_qmi_imsa_info_unlock();

   QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imsa_cleanup

//===========================================================================
// qcril_qmi_imsa_store_registration_status
//===========================================================================
void qcril_qmi_imsa_store_registration_status(
      uint8_t ims_registered_valid,
      imsa_ims_registration_status_enum_v01 ims_registered,
      uint8_t ims_registration_error_code_valid,
      uint16_t ims_registration_error_code,
      uint8_t ims_registration_error_string_valid,
      char *ims_registration_error_string,
      uint8_t registration_network_valid,
      imsa_service_rat_enum_v01 registration_network,
      uint8_t uri_list_valid,
      p_associated_uri_v01 *URI_List,
      uint32_t uri_list_len
      )
{
      QCRIL_LOG_FUNC_ENTRY();
      size_t len;
      qcril_qmi_imsa_info.ims_registered_valid = ims_registered_valid;
      qcril_qmi_imsa_info.ims_registered = ims_registered;

      qcril_qmi_imsa_info.registration_network_valid = registration_network_valid;
      qcril_qmi_imsa_info.registration_network = registration_network;

      qcril_qmi_imsa_info.ims_registration_error_code_valid = ims_registration_error_code_valid;
      qcril_qmi_imsa_info.ims_registration_error_code = ims_registration_error_code;

      if(qcril_qmi_imsa_info.ims_registration_error_string != NULL)
      {
        qcril_free(qcril_qmi_imsa_info.ims_registration_error_string);
        qcril_qmi_imsa_info.ims_registration_error_string = NULL;
      }
      qcril_qmi_imsa_info.ims_registration_error_string_valid = ims_registration_error_string_valid;
      if (ims_registration_error_string_valid && ims_registration_error_string)
      {
        len = strlen(ims_registration_error_string) + 1;
        qcril_qmi_imsa_info.ims_registration_error_string = (char*)qcril_malloc(len);
        if(qcril_qmi_imsa_info.ims_registration_error_string != NULL)
        {
          strlcpy(qcril_qmi_imsa_info.ims_registration_error_string, ims_registration_error_string,
              len);
        }
      }

      qcril_qmi_imsa_info.uri_list_valid = uri_list_valid;
      if(uri_list_valid == FALSE)
      {
        qcril_qmi_imsa_info.uri_list_len = 0;
      }
      else
      {
        memset(qcril_qmi_imsa_info.URI_List, 0,
              sizeof(p_associated_uri_v01) * IMSA_MAX_NUM_URIS_V01);
        qcril_qmi_imsa_info.uri_list_len = uri_list_len;
        if(uri_list_len > 0)
        {
          memcpy(qcril_qmi_imsa_info.URI_List, URI_List,
                  uri_list_len * sizeof(p_associated_uri_v01));
        }
      }
      // IMSA REFACTOR TEST
      // TODO: <IMS_REFACTOR> move out of mutex lock
      qcril_qmi_imsa_broadcast_service_status(qcril_qmi_imsa_info);
      QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
// qcril_qmi_imsa_reg_status_ind_hdlr
//===========================================================================
void qcril_qmi_imsa_reg_status_ind_hdlr(void *ind_data_ptr)
{
   imsa_registration_status_ind_msg_v01 *reg_ind_msg_ptr =
     (imsa_registration_status_ind_msg_v01*) ind_data_ptr;

   QCRIL_LOG_FUNC_ENTRY();

   if (NULL != reg_ind_msg_ptr)
   {
      qcril_qmi_imsa_info_lock();
      imsa_ims_registration_status_enum_v01 ims_reg_status = reg_ind_msg_ptr->ims_reg_status;

      if (!reg_ind_msg_ptr->ims_reg_status_valid)
      {
          ims_reg_status = (reg_ind_msg_ptr->ims_registered ?
                            IMSA_STATUS_REGISTERED_V01 :
                            IMSA_STATUS_NOT_REGISTERED_V01);
      }

      qcril_qmi_imsa_store_registration_status(TRUE,
                                               ims_reg_status,
                                               reg_ind_msg_ptr->ims_registration_failure_error_code_valid,
                                               reg_ind_msg_ptr->ims_registration_failure_error_code,
                                               reg_ind_msg_ptr->registration_error_string_valid,
                                               reg_ind_msg_ptr->registration_error_string,
                                               reg_ind_msg_ptr->registration_network_valid,
                                               reg_ind_msg_ptr->registration_network,
                                               reg_ind_msg_ptr->uri_list_valid,
                                               reg_ind_msg_ptr->uri_list,
                                               reg_ind_msg_ptr->uri_list_len);

      QCRIL_LOG_INFO("here: string gotten: %s", reg_ind_msg_ptr->registration_error_string);
      QCRIL_LOG_INFO("ims_registered: %d", qcril_qmi_imsa_info.ims_registered);

      auto reg = qcril_qmi_ims_create_Registration(
          qcril_qmi_imsa_info.ims_registered_valid, qcril_qmi_imsa_info.ims_registered,
          qcril_qmi_imsa_info.registration_network_valid, qcril_qmi_imsa_info.registration_network,
          qcril_qmi_imsa_info.ims_registration_error_code_valid,
          qcril_qmi_imsa_info.ims_registration_error_code,
          (qcril_qmi_imsa_info.ims_registration_error_string_valid
               ? qcril_qmi_imsa_info.ims_registration_error_string
               : nullptr),
          (qcril_qmi_imsa_info.uri_list_valid ? qcril_qmi_imsa_info.uri_list_len : 0),
          qcril_qmi_imsa_info.URI_List);
      qcril_qmi_imsa_info_unlock();

      //auto msg = std::make_shared<QcRilUnsolImsRegStateMessage>(std::make_shared<Registration>(reg));
      auto msg = std::make_shared<QcRilUnsolImsRegStateMessage>(reg);
      if (msg != nullptr)
      {
        Dispatcher::getInstance().dispatchSync(msg);
      }
   }
   else
   {
      QCRIL_LOG_ERROR("ind_data_ptr is NULL");
   }
   QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imsa_reg_status_ind_hdlr

//===========================================================================
// qcril_qmi_imsa_vowifi_status_ind_hdlr
//===========================================================================
void qcril_qmi_imsa_vowifi_status_ind_hdlr(void *ind_data_ptr)
{
   imsa_vowifiprofile_status_ind_msg_v01 *ind_msg_ptr = NULL;

   QCRIL_LOG_FUNC_ENTRY();

   ind_msg_ptr = (imsa_vowifiprofile_status_ind_msg_v01 *) ind_data_ptr;
   if (NULL != ind_msg_ptr)
   {
     QCRIL_LOG_INFO("vowifiprofile_status: %d ",
                    ind_msg_ptr->vowifiprofile_status);

     qcril::interfaces::VowifiQuality quality =
        qcril_qmi_ims_map_qmi_wifi_quality_to_ims_wifi_quality(ind_msg_ptr->vowifiprofile_status);
     auto msg = std::make_shared<QcRilUnsolImsVowifiCallQuality>(quality);
     if (msg != nullptr)
     {
       Dispatcher::getInstance().dispatchSync(msg);
     }
   }
   else
   {
     QCRIL_LOG_ERROR("ind_data_ptr is NULL");
   }

   QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imsa_vowifi_status_ind_hdlr

//===========================================================================
// qcril_qmi_imsa_block_status_ind_hdlr
//===========================================================================
void qcril_qmi_imsa_block_status_ind_hdlr(void *ind_data_ptr)
{
    imsa_ims_block_status_ind_msg_v01 *block_ind_msg_ptr =
                        (imsa_ims_block_status_ind_msg_v01*) ind_data_ptr;

    QCRIL_LOG_FUNC_ENTRY();
    if (block_ind_msg_ptr) {
        std::shared_ptr<qcril::interfaces::RegistrationBlockStatus> blockStatus;
        blockStatus = copy_qmi_blk_status_to_ims_registration_block_status(block_ind_msg_ptr);

        if (blockStatus != nullptr) {
            auto msg = std::make_shared<QcRilUnsolImsRegBlockStatusMessage>(blockStatus);
            if (msg != nullptr)
            {
                Dispatcher::getInstance().dispatchSync(msg);
            }
        }
        else
        {
            QCRIL_LOG_ERROR("Block status  is NULL");
        }
    }
   QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imsa_block_status_ind_hdlr

//===========================================================================
// qcril_qmi_imsa_geo_loc_status_ind_hdlr
//===========================================================================
void qcril_qmi_imsa_geo_loc_status_ind_hdlr(void *ind_data_ptr)
{
   imsa_geo_loc_status_ind_msg_v01 *ind_msg_ptr = (imsa_geo_loc_status_ind_msg_v01 *) ind_data_ptr;

   QCRIL_LOG_FUNC_ENTRY();

   if (NULL != ind_msg_ptr)
   {
       QCRIL_LOG_INFO("lat_v: %c lat: %g long_v: %c long: %g",
                      ind_msg_ptr->latitude_valid ? 'Y' : 'N',
                      ind_msg_ptr->latitude,
                      ind_msg_ptr->longitude_valid ? 'Y' : 'N',
                      ind_msg_ptr->longitude);

       auto msg = std::make_shared<QcRilUnsolImsGeoLocationInfo>();
       if (msg != nullptr) {
           if (ind_msg_ptr->latitude_valid) {
              msg->setLatitude(ind_msg_ptr->latitude);
           }
           if (ind_msg_ptr->longitude_valid) {
              msg->setLongitude(ind_msg_ptr->longitude);
           }
           Dispatcher::getInstance().dispatchSync(msg);
       } else {
           QCRIL_LOG_INFO("memory failure");
       }
       //TODO handle failure case


#if 0
       if (err)
       {
           memset(&set_geo, 0, sizeof(set_geo));
           memset(&set_geo_rsp, 0, sizeof(set_geo_rsp));
           set_geo.latitude_valid = ind_msg_ptr->latitude_valid;
           set_geo.latitude = ind_msg_ptr->latitude;
           set_geo.longitude_valid = ind_msg_ptr->longitude_valid;
           set_geo.longitude = ind_msg_ptr->longitude;
           qmi_client_error_type qmi_error = qmi_client_imsa_send_sync(
               QMI_IMSA_SET_GEO_LOC_REQ_V01,
               &set_geo, sizeof(set_geo),
               &set_geo_rsp, sizeof(set_geo_rsp));
           err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(qmi_error,
               &set_geo_rsp.resp);
           QCRIL_LOG_INFO(".. transport error %d, ril_error %d", (int)qmi_error, (int)err);
       }
#endif

   }
   else
   {
      QCRIL_LOG_ERROR("ind_data_ptr is NULL");
   }
   QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imsa_geo_loc_status_ind_hdlr

//===========================================================================
// qcril_qmi_imsa_voice_info_ind_hdlr
//===========================================================================
void qcril_qmi_imsa_voice_info_ind_hdlr(void *ind_data_ptr) {
    imsa_voice_info_ind_msg_v01 *ind_msg_ptr = (imsa_voice_info_ind_msg_v01 *)
        ind_data_ptr;

    QCRIL_LOG_FUNC_ENTRY();
    if (NULL != ind_msg_ptr) {
        QCRIL_LOG_INFO("Voice info data: %d", ind_msg_ptr->voice_info);
        qcril::interfaces::VoiceInfo info =
            qcril_qmi_ims_map_voice_info(ind_msg_ptr->voice_info);
        auto msg = std::make_shared<QcRilUnsolImsVoiceInfo>(info);
        if (msg != nullptr)
        {
            Dispatcher::getInstance().dispatchSync(msg);
        }
    }
    QCRIL_LOG_FUNC_RETURN();
} //qcril_qmi_imsa_voice_info_ind_hdlr

//===========================================================================
// qcril_qmi_imsa_multi_identity_registration_status_change_ind_hdlr
//===========================================================================
void qcril_qmi_imsa_multi_identity_registration_status_change_ind_hdlr(void *ind_data_ptr) {
    QCRIL_LOG_FUNC_ENTRY();

    do {
        if(ind_data_ptr == nullptr) {
            QCRIL_LOG_ERROR("ind_msg_ptr is NULL");
            break;
        }
        auto registration_info = static_cast<imsa_digits_update_ind_msg_v01*>(ind_data_ptr);
        if (registration_info == nullptr) {
            QCRIL_LOG_ERROR("ind_msg_ptr is NULL");
            break;
        }
        if (registration_info->line_info_list_valid) {
            std::vector<qcril::interfaces::MultiIdentityInfo> lines_info;
            for (uint32_t i =0; i < registration_info->line_info_list_len; i++) {
                auto& qmi_line_info = registration_info->line_info_list[i];
                qcril::interfaces::MultiIdentityInfo line_info(qmi_line_info.msisdn,
                                qmi_line_info.uri_line_status);
                lines_info.push_back(line_info);
            }
            auto msg = std::make_shared<QcRilUnsolImsMultiIdentityStatusMessage>(lines_info);
            if (msg != nullptr)
            {
               Dispatcher::getInstance().dispatchSync(msg);
            }
        }
    } while(FALSE);

    QCRIL_LOG_FUNC_RETURN();
} //qcril_qmi_imsa_multi_identity_registration_status_change_ind_hdlr

//===========================================================================
// qcril_qmi_imsa_multi_identity_info_pending_ind_hdlr
//===========================================================================
void qcril_qmi_imsa_multi_identity_info_pending_ind_hdlr(void *ind_data_ptr) {
    QCRIL_LOG_FUNC_ENTRY();

    //ind_data_ptr just has a placeholder to prevent the declaration of
    //an empty struct. The field should not be used.
    QCRIL_NOTUSED(ind_data_ptr);
    auto msg = std::make_shared<QcRilUnsolImsPendingMultiLineStatus>();
    if (msg != nullptr)
    {
        Dispatcher::getInstance().dispatchSync(msg);
    }
}

//===========================================================================
// qcril_qmi_imsa_pdp_status_ind_hdlr
//===========================================================================
void qcril_qmi_imsa_pdp_status_ind_hdlr(void *ind_data_ptr) {
  QCRIL_LOG_FUNC_ENTRY();
  auto pdp_status_ind_msg = static_cast<imsa_pdp_status_ind_msg_v01*>(ind_data_ptr);
  if (pdp_status_ind_msg)
  {
    auto msg = std::make_shared<ImsPdpStatusInd>(pdp_status_ind_msg->is_ims_pdp_connected);
    Dispatcher::getInstance().broadcast(msg);
  }
  else
  {
    QCRIL_LOG_ERROR("ind_data_ptr is NULL");
  }
  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imsa_pdp_status_ind_hdlr

// qcril_qmi_imsa_set_geo_loc
void qcril_qmi_imsa_set_geo_loc(std::shared_ptr<QcRilRequestImsGeoLocationInfoMessage> msg) {
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  uint32 user_data;
  imsa_set_geo_loc_req_msg_v01 req_msg;
  RIL_Errno err = RIL_E_GENERIC_FAILURE;
  uint16_t req_id = -1;
  bool doCleanup = false;

  QCRIL_LOG_FUNC_ENTRY();

  if (msg) {
    do {
      memset(&req_msg, 0, sizeof(req_msg));
      if (msg->hasLatitude()) {
        req_msg.latitude_valid = TRUE;
        req_msg.latitude = msg->getLatitude();
      }
      if (msg->hasLongitude()) {
        req_msg.longitude_valid = TRUE;
        req_msg.longitude = msg->getLongitude();
      }

      if (msg->hasCity() && !msg->getCity().empty()) {
        req_msg.city_valid = TRUE;
        strlcpy((char *)req_msg.city, msg->getCity().c_str(), sizeof(req_msg.city));
      }
      if (msg->hasPostalCode() && !msg->getPostalCode().empty()) {
        req_msg.zipCode_valid = TRUE;
        strlcpy((char *)req_msg.zipCode, (char *)msg->getPostalCode().c_str(),
                sizeof(req_msg.zipCode));
      }
      if (msg->hasState() && !msg->getState().empty()) {
        req_msg.state_valid = TRUE;
        strlcpy((char *)req_msg.state, msg->getState().c_str(), sizeof(req_msg.state));
      }
      if (msg->hasCountry() && !msg->getCountry().empty()) {
        req_msg.countryName_valid = TRUE;
        strlcpy((char *)req_msg.countryName, msg->getCountry().c_str(),
                sizeof(req_msg.countryName));
      }
      if (msg->hasCountryCode() && !msg->getCountryCode().empty()) {
        req_msg.countryCode_valid = TRUE;
        strlcpy((char *)req_msg.countryCode, msg->getCountryCode().c_str(),
                sizeof(req_msg.countryCode));
      }
      if (msg->hasStreet() && !msg->getStreet().empty()) {
        req_msg.street_valid = TRUE;
        strlcpy((char *)req_msg.street, msg->getStreet().c_str(), sizeof(req_msg.street));
      }
      if (msg->hasHouseNumber() && !msg->getHouseNumber().empty()) {
        req_msg.housenumber_valid = TRUE;
        strlcpy((char *)req_msg.housenumber, msg->getHouseNumber().c_str(),
                sizeof(req_msg.housenumber));
      }

      auto pendingMsgStatus = getImsModule()->getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true) {
        doCleanup = false;
        break;
      }
      req_id = pendingMsgStatus.first;

      user_data = QCRIL_COMPOSE_USER_DATA(instance_id, QCRIL_DEFAULT_MODEM_ID, req_id);

      qmi_client_error_type qmi_error = qmi_client_imsa_send_async(
          QMI_IMSA_SET_GEO_LOC_REQ_V01, &req_msg,
          sizeof(imsa_set_geo_loc_req_msg_v01),
          sizeof(imsa_set_geo_loc_rsp_msg_v01),
          qcril_qmi_imsa_set_geo_loc_resp_hdlr,
          (void *)(uintptr_t)user_data);

      err = qcril_qmi_util_convert_qmi_response_codes_to_ril_result(qmi_error, NULL);
      QCRIL_LOG_INFO(".. transport error %d, ril_error %d", (int)qmi_error, (int)err);
      if (err != RIL_E_SUCCESS) {
        doCleanup = true;
      }
    } while (FALSE);
    if (err != RIL_E_SUCCESS) {
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(err, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
      if (doCleanup) {
        getImsModule()->getPendingMessageList().erase(req_id);
      }
    }
  }
} // qcril_qmi_imsa_set_geo_loc

//===========================================================================
// qcril_qmi_imsa_store_service_status_cl
//===========================================================================
void qcril_qmi_imsa_store_service_status_cl(
    uint8_t sms_service_status_valid,
    imsa_service_status_enum_v01 sms_service_status,
    uint8_t voip_service_status_valid,
    imsa_service_status_enum_v01 voip_service_status,
    uint8_t vt_service_status_valid,
    imsa_service_status_enum_v01 vt_service_status,
    uint8_t sms_service_rat_valid,
    imsa_service_rat_enum_v01 sms_service_rat,
    uint8_t voip_service_rat_valid,
    imsa_service_rat_enum_v01 voip_service_rat,
    uint8_t vt_service_rat_valid,
    imsa_service_rat_enum_v01 vt_service_rat,
    uint8_t ut_service_status_valid,
    imsa_service_status_enum_v01 ut_service_status,
    uint8_t ut_service_rat_valid,
    imsa_service_rat_enum_v01 ut_service_rat
)
{
    if (sms_service_status_valid)
    {
      qcril_qmi_imsa_info.ims_srv_status_valid = TRUE;

      qcril_qmi_imsa_info.ims_srv_status.sms_service_status_valid = sms_service_status_valid;
      qcril_qmi_imsa_info.ims_srv_status.sms_service_status = sms_service_status;
    }
    else if (!qcril_qmi_imsa_info.ims_srv_status.sms_service_status_valid)
    {
      imsa_service_status_enum_v01 sms_service_status;
      auto msg = std::make_shared<SmsGetImsServiceStatusMessage>();
      std::shared_ptr<SmsImsServiceStatusInd::SmsImsServiceStatusInfo> sms_svc_info = nullptr;
      Message::Callback::Status apiStatus;
      apiStatus = msg->dispatchSync(sms_svc_info);

      if ((apiStatus != Message::Callback::Status::SUCCESS) || !sms_svc_info)
      {
        QCRIL_LOG_DEBUG("Failed to get sms over ims status apiStatus = %d", apiStatus);
      }
      else
      {
        if (SmsImsServiceStatusInd::SmsImsServiceStatus::NO_SERVICE ==
                    sms_svc_info->sms_ims_service_status)
        {
          sms_service_status = IMSA_NO_SERVICE_V01;
        }
        else if (SmsImsServiceStatusInd::SmsImsServiceStatus::LIMITED_SERVICE ==
                    sms_svc_info->sms_ims_service_status)
        {
          sms_service_status = IMSA_LIMITED_SERVICE_V01;
        }
        else if (SmsImsServiceStatusInd::SmsImsServiceStatus::FULL_SERVICE ==
                    sms_svc_info->sms_ims_service_status)
        {
          sms_service_status = IMSA_FULL_SERVICE_V01;
        }
        else
        {
          sms_service_status = IMSA_NO_SERVICE_V01;
        }
        qcril_qmi_imsa_info.ims_srv_status_valid = TRUE;
        qcril_qmi_imsa_info.ims_srv_status.sms_service_status_valid = TRUE;
        qcril_qmi_imsa_info.ims_srv_status.sms_service_status = sms_service_status;
      }
    }

    if (sms_service_rat_valid)
    {
      qcril_qmi_imsa_info.ims_srv_status.sms_service_rat_valid = sms_service_rat_valid;
      qcril_qmi_imsa_info.ims_srv_status.sms_service_rat = sms_service_rat;
    }

    if (voip_service_status_valid)
    {
      qcril_qmi_imsa_info.ims_srv_status_valid = TRUE;

      qcril_qmi_imsa_info.ims_srv_status.voip_service_status_valid = voip_service_status_valid;
      qcril_qmi_imsa_info.ims_srv_status.voip_service_status = voip_service_status;
    }

    if (voip_service_rat_valid)
    {
      qcril_qmi_imsa_info.ims_srv_status.voip_service_rat_valid = voip_service_rat_valid;
      qcril_qmi_imsa_info.ims_srv_status.voip_service_rat = voip_service_rat;
    }

    if (vt_service_status_valid)
    {
      qcril_qmi_imsa_info.ims_srv_status_valid = TRUE;

      qcril_qmi_imsa_info.ims_srv_status.vt_service_status_valid = vt_service_status_valid;
      qcril_qmi_imsa_info.ims_srv_status.vt_service_status = vt_service_status;
    }

    if (vt_service_rat_valid)
    {
      qcril_qmi_imsa_info.ims_srv_status.vt_service_rat_valid = vt_service_rat_valid;
      qcril_qmi_imsa_info.ims_srv_status.vt_service_rat = vt_service_rat;
    }

    if (ut_service_status_valid)
    {
      qcril_qmi_imsa_info.ims_srv_status_valid = TRUE;

      qcril_qmi_imsa_info.ims_srv_status.ut_service_status_valid = ut_service_status_valid;
      qcril_qmi_imsa_info.ims_srv_status.ut_service_status = ut_service_status;
    }

    if (ut_service_rat_valid)
    {
      qcril_qmi_imsa_info.ims_srv_status.ut_service_rat_valid = ut_service_rat_valid;
      qcril_qmi_imsa_info.ims_srv_status.ut_service_rat = ut_service_rat;
    }
    // IMSA REFACTOR TEST
    qcril_qmi_imsa_broadcast_service_status(qcril_qmi_imsa_info);

} // qcril_qmi_imsa_store_service_status_cl

//===========================================================================
// qcril_qmi_imsa_service_status_ind_hdlr
//===========================================================================
void qcril_qmi_imsa_service_status_ind_hdlr(void *ind_data_ptr)
{
    imsa_service_status_ind_msg_v01 *service_ind_msg_ptr = (imsa_service_status_ind_msg_v01*) ind_data_ptr;

    QCRIL_LOG_FUNC_ENTRY();
    if (NULL != service_ind_msg_ptr)
    {
        QCRIL_LOG_ESSENTIAL("VOIP: service_status(%s): %d, rat(%s): %d",
            service_ind_msg_ptr->voip_service_status_valid ? "valid": "not valid",
            service_ind_msg_ptr->voip_service_status,
            service_ind_msg_ptr->voip_service_rat_valid ? "valid": "not valid",
            service_ind_msg_ptr->voip_service_rat);
        QCRIL_LOG_ESSENTIAL("VT: service_status(%s): %d, rat(%s): %d",
            service_ind_msg_ptr->vt_service_status_valid ? "valid": "not valid",
            service_ind_msg_ptr->vt_service_status,
            service_ind_msg_ptr->vt_service_rat_valid ? "valid": "not valid",
            service_ind_msg_ptr->vt_service_rat);
        QCRIL_LOG_ESSENTIAL("UT: service_status(%s): %d, rat(%s): %d",
            service_ind_msg_ptr->ut_service_status_valid ? "valid": "not valid",
            service_ind_msg_ptr->ut_service_status,
            service_ind_msg_ptr->ut_service_rat_valid ? "valid": "not valid",
            service_ind_msg_ptr->ut_service_rat);

        qcril_qmi_imsa_info_lock();

        qcril_qmi_imsa_store_service_status_cl(
            FALSE,
            IMSA_NO_SERVICE_V01,
            service_ind_msg_ptr->voip_service_status_valid,
            service_ind_msg_ptr->voip_service_status,
            service_ind_msg_ptr->vt_service_status_valid,
            service_ind_msg_ptr->vt_service_status,
            service_ind_msg_ptr->sms_service_rat_valid,
            service_ind_msg_ptr->sms_service_rat,
            service_ind_msg_ptr->voip_service_rat_valid,
            service_ind_msg_ptr->voip_service_rat,
            service_ind_msg_ptr->vt_service_rat_valid,
            service_ind_msg_ptr->vt_service_rat,
            service_ind_msg_ptr->ut_service_status_valid,
            service_ind_msg_ptr->ut_service_status,
            service_ind_msg_ptr->ut_service_rat_valid,
            service_ind_msg_ptr->ut_service_rat );

        std::shared_ptr<qcril::interfaces::ServiceStatusInfoList> srvStatus = nullptr;
            srvStatus = qcril_qmi_ims_create_ServiceStatusInfoList(&qcril_qmi_imsa_info.ims_srv_status);

        qcril_qmi_imsa_info_unlock();

        if (srvStatus != nullptr)
        {
            auto msg = std::make_shared<QcRilUnsolImsSrvStatusIndication>(srvStatus);
            if (msg != nullptr)
            {
               Dispatcher::getInstance().dispatchSync(msg);
            }

            // Trigger QCRIL_AM_EVENT_IMS_SRV_CHANGED only if the VoIP/VT RAT is valid    
            if (service_ind_msg_ptr->voip_service_rat_valid ||    
                     service_ind_msg_ptr->vt_service_rat_valid)   
            {   
              // TODO: <IMS_REFACTOR> use AM message
                qcril_am_handle_event(QCRIL_AM_EVENT_IMS_SRV_CHANGED, NULL);    
            }   
        }
    }
    else
    {
        QCRIL_LOG_ERROR("ind_data_ptr is NULL");
    }
    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imsa_service_status_ind_hdlr

//=========================================================================
//  qcril_qmi_imsa_sms_ims_service_status_hdlr
//=========================================================================
void qcril_qmi_imsa_sms_ims_service_status_hdlr
(
 SmsImsServiceStatusInd::SmsImsServiceStatus &sms_status
)
{
  imsa_service_status_enum_v01 sms_service_status = IMSA_NO_SERVICE_V01;
  QCRIL_LOG_FUNC_ENTRY();

  QCRIL_LOG_ESSENTIAL("SMS IMS service_status: %d", sms_status);

  if(SmsImsServiceStatusInd::SmsImsServiceStatus::NO_SERVICE == sms_status)
  {
    sms_service_status = IMSA_NO_SERVICE_V01;
  }
  else if(SmsImsServiceStatusInd::SmsImsServiceStatus::LIMITED_SERVICE == sms_status)
  {
    sms_service_status = IMSA_LIMITED_SERVICE_V01;
  }
  else if(SmsImsServiceStatusInd::SmsImsServiceStatus::FULL_SERVICE == sms_status)
  {
    sms_service_status = IMSA_FULL_SERVICE_V01;
  }

  qcril_qmi_imsa_info_lock();
  qcril_qmi_imsa_info.ims_srv_status.sms_service_status_valid = TRUE;
  qcril_qmi_imsa_info.ims_srv_status.sms_service_status = sms_service_status;

  std::shared_ptr<qcril::interfaces::ServiceStatusInfoList> srvStatus = nullptr;
  srvStatus = qcril_qmi_ims_create_ServiceStatusInfoList(&qcril_qmi_imsa_info.ims_srv_status);

  qcril_qmi_imsa_info_unlock();

  if (srvStatus != nullptr)
  {
    auto msg = std::make_shared<QcRilUnsolImsSrvStatusIndication>(srvStatus);
    if (msg != nullptr)
    {
      Dispatcher::getInstance().dispatchSync(msg);
    }
  }
  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imsa_sms_ims_service_status_hdlr

//=========================================================================
//  convert_imsa_handover_type
//=========================================================================
qcril::interfaces::HandoverType convert_imsa_handover_type(imsa_rat_handover_status_enum_v01 in)
{
  qcril::interfaces::HandoverType result = qcril::interfaces::HandoverType::UNKNOWN;
  switch (in) {
    case IMSA_STATUS_RAT_HO_SUCCESS_V01:
      result = qcril::interfaces::HandoverType::COMPLETE_SUCCESS;
      break;
    case IMSA_STATUS_RAT_HO_FAILURE_V01:
      result = qcril::interfaces::HandoverType::COMPLETE_FAIL;
      break;
    case IMSA_STATUS_RAT_HO_NOT_TRIGGERED_V01:
      result = qcril::interfaces::HandoverType::NOT_TRIGGERED;
      break;
    case IMSA_STATUS_RAT_HO_NOT_TRIGGERED_MOBILE_DATA_OFF_V01:
      result = qcril::interfaces::HandoverType::NOT_TRIGGERED_MOBILE_DATA_OFF;
      break;
    default:
      result = qcril::interfaces::HandoverType::UNKNOWN;
      break;
  }
  return result;
}

//=========================================================================
//  convert_imsa_rat_type
//=========================================================================
RIL_RadioTechnology convert_imsa_rat_type(imsa_service_rat_enum_v01 in)
{
  RIL_RadioTechnology result = RADIO_TECH_UNKNOWN;
  switch (in) {
    case IMSA_WLAN_V01:
      result = RADIO_TECH_WIFI;
      break;
    case IMSA_IWLAN_V01:
      result = RADIO_TECH_IWLAN;
      break;
    case IMSA_WWAN_V01:
      result = RADIO_TECH_LTE;
      break;
    default:
      result = RADIO_TECH_UNKNOWN;
      break;
  }
  return result;
}

//===========================================================================
// qcril_qmi_imsa_rat_handover_status_ind_hdlr
//===========================================================================
static void qcril_qmi_imsa_rat_handover_status_ind_hdlr(void *ind_data_ptr)
{
  QCRIL_LOG_FUNC_ENTRY();
  imsa_rat_handover_status_ind_msg_v01 *rat_ho_ind_msg_ptr = (imsa_rat_handover_status_ind_msg_v01*) ind_data_ptr;
  if (NULL != rat_ho_ind_msg_ptr)
  {
    if (rat_ho_ind_msg_ptr->rat_ho_status_info_valid)
    {
      auto msg = std::make_shared<QcRilUnsolImsHandoverMessage>();
      if (msg != nullptr)
      {
        msg->setHandoverType(
            convert_imsa_handover_type(rat_ho_ind_msg_ptr->rat_ho_status_info.rat_ho_status));
        msg->setSourceTech(
            convert_imsa_rat_type(rat_ho_ind_msg_ptr->rat_ho_status_info.source_rat));
        msg->setTargetTech(
            convert_imsa_rat_type(rat_ho_ind_msg_ptr->rat_ho_status_info.target_rat));
        msg->setCauseCode(rat_ho_ind_msg_ptr->rat_ho_status_info.cause_code);
        Dispatcher::getInstance().dispatchSync(msg);
      }
      if (rat_ho_ind_msg_ptr->rat_ho_status_info.rat_ho_status == IMSA_STATUS_RAT_HO_SUCCESS_V01)
      {
        // TODO: <IMS_REFACTOR> use AM message
        qcril_am_handle_event(QCRIL_AM_EVENT_IMS_HANDOVER,
            &rat_ho_ind_msg_ptr->rat_ho_status_info.target_rat);
      }
    }
  }
  else
  {
    QCRIL_LOG_ERROR("ind_data_ptr is NULL");
  }
  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imsa_rat_handover_status_ind_hdlr

//===========================================================================
// qcril_qmi_imsa_rtp_statistics_ind_hdlr
//===========================================================================
static void qcril_qmi_imsa_rtp_statistics_ind_hdlr(void *ind_data_ptr)
{
    QCRIL_LOG_FUNC_ENTRY();
    imsa_rtp_statistics_ind_msg_v01 *rtp_statistics_ind_msg_ptr =
        (imsa_rtp_statistics_ind_msg_v01*) ind_data_ptr;
    if (NULL != rtp_statistics_ind_msg_ptr)
    {
        std::shared_ptr<qcril::interfaces::RtpStatisticsData> respPayload =
            std::make_shared<qcril::interfaces::RtpStatisticsData>();

        QCRIL_LOG_DEBUG("rx_expected_rtp_pkt = %d, rx_rtp_pkt_loss = %d\n",
                 rtp_statistics_ind_msg_ptr->total_rx_expected_rtp_pkt_count,
                 rtp_statistics_ind_msg_ptr->total_rx_rtp_pkt_loss_count);

        /* check for RTP stats pending request */
        auto rtpMsg = getImsModule()->getPendingMessageList().find(
                QcRilRequestImsGetRtpStatsMessage::get_class_message_id());
        if (rtpMsg) {
            if(respPayload) {
                if (rtp_statistics_ind_msg_ptr->total_rx_expected_rtp_pkt_count_valid) {
                    respPayload->setCount(
                        rtp_statistics_ind_msg_ptr->total_rx_expected_rtp_pkt_count);
                } else {
                    respPayload->setCount(0);
                }
            }
            auto resp =
                std::make_shared<QcRilRequestMessageCallbackPayload>(RIL_E_SUCCESS, respPayload);
            auto ril_msg = std::static_pointer_cast<QcRilRequestImsGetRtpStatsMessage>(rtpMsg);
            ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, resp);
            //erase the message
            getImsModule()->getPendingMessageList().erase(rtpMsg);
            return;
        }

        /* check for RTP error stats pending request */
        auto rtpErrorMsg = getImsModule()->getPendingMessageList().find(
            QcRilRequestImsGetRtpErrorStatsMessage::get_class_message_id());
        if (rtpErrorMsg) {
            if(respPayload) {
                if (rtp_statistics_ind_msg_ptr->total_rx_rtp_pkt_loss_count_valid) {
                    respPayload->setCount(rtp_statistics_ind_msg_ptr->total_rx_rtp_pkt_loss_count);
                } else {
                    respPayload->setCount(0);
                }
            }
            auto resp =
                std::make_shared<QcRilRequestMessageCallbackPayload>(RIL_E_SUCCESS, respPayload);
            auto error_msg =
                std::static_pointer_cast<QcRilRequestImsGetRtpStatsMessage>(rtpErrorMsg);
            error_msg->sendResponse(error_msg, Message::Callback::Status::SUCCESS, resp);
            //erase the message
            getImsModule()->getPendingMessageList().erase(rtpErrorMsg);
            return;
        }
    }
    else
    {
        QCRIL_LOG_ERROR("ind_data_ptr is NULL");
    }
    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imsa_rtp_statistics_ind_hdlr

//===========================================================================
// qcril_qmi_imsa_unsol_ind_cb_helper
//===========================================================================
void qcril_qmi_imsa_unsol_ind_cb_helper
(
 unsigned int   msg_id,
 unsigned char *decoded_payload,
 uint32_t       decoded_payload_len
)
{
  QCRIL_LOG_FUNC_ENTRY();
  QCRIL_LOG_ESSENTIAL("qcril_qmi_imsa_unsol_ind_cb_helper");

  if (decoded_payload || !decoded_payload_len)
  {
    switch(msg_id)
    {
      case QMI_IMSA_REGISTRATION_STATUS_IND_V01:
        qcril_qmi_imsa_reg_status_ind_hdlr(decoded_payload);
        break;

      case QMI_IMSA_SERVICE_STATUS_IND_V01:
        qcril_qmi_imsa_service_status_ind_hdlr(decoded_payload);
        break;

      case QMI_IMSA_RAT_HANDOVER_STATUS_IND_V01:
        qcril_qmi_imsa_rat_handover_status_ind_hdlr(decoded_payload);
        break;

      case QMI_IMSA_RTP_STATISTICS_IND_V01:
        qcril_qmi_imsa_rtp_statistics_ind_hdlr(decoded_payload);
        break;

      case QMI_IMSA_GEO_LOC_STATUS_IND_V01:
        qcril_qmi_imsa_geo_loc_status_ind_hdlr(decoded_payload);
        break;

      case QMI_IMSA_VoWIFI_STATUS_IND_V01:
        qcril_qmi_imsa_vowifi_status_ind_hdlr(decoded_payload);
        break;

      case QMI_IMSA_IMS_BLOCK_STATUS_IND_V01:
        qcril_qmi_imsa_block_status_ind_hdlr(decoded_payload);
        break;

      case QMI_IMSA_VOICE_INFO_IND_V01:
        qcril_qmi_imsa_voice_info_ind_hdlr(decoded_payload);
        break;

      case QMI_IMSA_DIGITS_UPDATE_IND_V01:
        qcril_qmi_imsa_multi_identity_registration_status_change_ind_hdlr(decoded_payload);
        break;

      case QMI_IMSA_DIGITS_INFO_PENDING_IND_V01:
        qcril_qmi_imsa_multi_identity_info_pending_ind_hdlr(decoded_payload);
        break;

      case QMI_IMSA_PDP_STATUS_IND_V01:
        qcril_qmi_imsa_pdp_status_ind_hdlr(decoded_payload);
        break;

      default:
        QCRIL_LOG_INFO("Unknown QMI IMSA indication %d", msg_id);
        break;
    }
  }

  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imsa_unsol_ind_cb_helper

//===========================================================================
// qcril_qmi_imsa_get_reg_status_resp_hdlr
//===========================================================================
void qcril_qmi_imsa_get_reg_status_resp_hdlr
(
   unsigned int                 msg_id,
   std::shared_ptr<void>        resp_c_struct,
   unsigned int                 resp_c_struct_len,
   void                        *resp_cb_data,
   qmi_client_error_type        transp_err
)
{
  (void)msg_id;
  (void)resp_c_struct_len;
  imsa_get_registration_status_resp_msg_v01 *resp_msg_ptr = NULL;
  //Registration reg = {};
  uint32 user_data = (uint32)(uintptr_t) resp_cb_data;
  uint16_t req_id = QCRIL_EXTRACT_USER_ID_FROM_USER_DATA(user_data);

  QCRIL_LOG_FUNC_ENTRY();
  getImsModule()->getPendingMessageList().print();
  auto pendingMsg = getImsModule()->getPendingMessageList().extract(req_id);
  getImsModule()->getPendingMessageList().print();
  RIL_Errno errorCode = RIL_E_SUCCESS;
  std::shared_ptr<qcril::interfaces::Registration> reg = nullptr;

  do {
    if (pendingMsg == nullptr)
    {
      QCRIL_LOG_ERROR("pendingMsg is null");
      break;
    }
    if (transp_err != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Transp error (%d) recieved from QMI", transp_err);
      errorCode = RIL_E_GENERIC_FAILURE;
      break;
    }
    resp_msg_ptr = (imsa_get_registration_status_resp_msg_v01*)(resp_c_struct.get());
    if (resp_msg_ptr == nullptr)
    {
      QCRIL_LOG_ERROR("resp_msg_ptr is null");
      errorCode = RIL_E_GENERIC_FAILURE;
      break;
    }

    if (resp_msg_ptr->resp.result == QMI_RESULT_SUCCESS_V01 &&
        (resp_msg_ptr->ims_reg_status_valid || resp_msg_ptr->ims_registered_valid))
    {
      qcril_qmi_imsa_info_lock();
      imsa_ims_registration_status_enum_v01 ims_reg_status = resp_msg_ptr->ims_reg_status;

      if (!resp_msg_ptr->ims_reg_status_valid)
      {
        ims_reg_status = (resp_msg_ptr->ims_registered ? IMSA_STATUS_REGISTERED_V01
                                                       : IMSA_STATUS_NOT_REGISTERED_V01);
      }

      qcril_qmi_imsa_store_registration_status(TRUE,
          ims_reg_status,
          resp_msg_ptr->ims_registration_failure_error_code_valid,
          resp_msg_ptr->ims_registration_failure_error_code,
          resp_msg_ptr->registration_error_string_valid,
          resp_msg_ptr->registration_error_string,
          resp_msg_ptr->registration_network_valid,
          resp_msg_ptr->registration_network,
          resp_msg_ptr->uri_list_valid,
          resp_msg_ptr->uri_list,
          resp_msg_ptr->uri_list_len);

      reg = qcril_qmi_ims_create_Registration(
          qcril_qmi_imsa_info.ims_registered_valid, qcril_qmi_imsa_info.ims_registered,
          qcril_qmi_imsa_info.registration_network_valid, qcril_qmi_imsa_info.registration_network,
          qcril_qmi_imsa_info.ims_registration_error_code_valid,
          qcril_qmi_imsa_info.ims_registration_error_code,
          (qcril_qmi_imsa_info.ims_registration_error_string_valid
               ? qcril_qmi_imsa_info.ims_registration_error_string
               : nullptr),
          (qcril_qmi_imsa_info.uri_list_valid ? qcril_qmi_imsa_info.uri_list_len : 0),
          qcril_qmi_imsa_info.URI_List);
      qcril_qmi_imsa_info_unlock();

      if (reg != nullptr) {
        errorCode = RIL_E_SUCCESS;
      } else {
        errorCode = RIL_E_GENERIC_FAILURE;
      }
    }
    else
    {
      QCRIL_LOG_INFO("error: %d", resp_msg_ptr->resp.error);
      qcril_qmi_imsa_info_lock();
      qcril_qmi_imsa_info.ims_registered_valid = FALSE;
      qcril_qmi_imsa_info.registration_network_valid = FALSE;
      qcril_qmi_imsa_info.ims_registration_error_code_valid = FALSE;
      qcril_qmi_imsa_info.ims_registration_error_string_valid = FALSE;
      qcril_qmi_imsa_info.uri_list_valid = FALSE;
      qcril_qmi_imsa_info_unlock();

      errorCode = RIL_E_GENERIC_FAILURE;
    }
  } while (FALSE);

  if (pendingMsg != nullptr)
  {
    auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(errorCode, reg);
    auto msg(std::static_pointer_cast<QcRilRequestImsGetRegStateMessage>(pendingMsg));
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
  }
  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imsa_get_reg_status_resp_hdlr

//===========================================================================
// qcril_qmi_imsa_ind_reg_resp_hdlr
//===========================================================================
void qcril_qmi_imsa_ind_reg_resp_hdlr
(
  const qcril_request_params_type *const params_ptr
)
{
   imsa_ind_reg_rsp_msg_v01 *resp_msg_ptr = NULL;
   QCRIL_LOG_FUNC_ENTRY();
   if( NULL != params_ptr && NULL != params_ptr->data )
   {
      resp_msg_ptr = (imsa_ind_reg_rsp_msg_v01 *)params_ptr->data;
      QCRIL_LOG_INFO("result: %d, error: %d", resp_msg_ptr->resp.result, resp_msg_ptr->resp.error);
      if (QMI_RESULT_SUCCESS_V01 == resp_msg_ptr->resp.result)
      {
         qcril_qmi_imsa_info_lock();
         qcril_qmi_imsa_info.ims_status_change_registered = TRUE;
         qcril_qmi_imsa_info_unlock();
      }
   }
   else
   {
      QCRIL_LOG_ERROR("params_ptr or params_ptr->data is NULL");
   }
   QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imsa_ind_reg_resp_hdlr

//===========================================================================
// qcril_qmi_imsa_get_service_status_resp_hdlr
//===========================================================================
void qcril_qmi_imsa_get_service_status_resp_hdlr
(
   unsigned int                 msg_id,
   std::shared_ptr<void>        resp_c_struct,
   unsigned int                 resp_c_struct_len,
   void                        *resp_cb_data,
   qmi_client_error_type        transp_err
)
{
  (void)msg_id;
  (void)resp_c_struct_len;
  imsa_get_service_status_resp_msg_v01 *resp_msg_ptr = NULL;
  uint32 user_data = (uint32)(uintptr_t) resp_cb_data;
  uint16_t req_id = QCRIL_EXTRACT_USER_ID_FROM_USER_DATA(user_data);

  QCRIL_LOG_FUNC_ENTRY();
  getImsModule()->getPendingMessageList().print();
  auto pendingMsg = getImsModule()->getPendingMessageList().extract(req_id);
  getImsModule()->getPendingMessageList().print();

  RIL_Errno errorCode = RIL_E_SUCCESS;
  std::shared_ptr<qcril::interfaces::ServiceStatusInfoList> srvStatus = nullptr;

  do {
    if (pendingMsg == nullptr)
    {
      QCRIL_LOG_ERROR("pendingMsg is null");
      break;
    }
    if (transp_err != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Transp error (%d) recieved from QMI", transp_err);
      errorCode = RIL_E_GENERIC_FAILURE;
      break;
    }
    resp_msg_ptr = (imsa_get_service_status_resp_msg_v01*)(resp_c_struct.get());
    if (resp_msg_ptr == nullptr)
    {
      QCRIL_LOG_ERROR("resp_msg_ptr is null");
      errorCode = RIL_E_GENERIC_FAILURE;
      break;
    }

    if (resp_msg_ptr->resp.result == QMI_RESULT_SUCCESS_V01)
    {
      QCRIL_LOG_ESSENTIAL("VOIP: service_status(%s): %d, rat(%s): %d",
          resp_msg_ptr->voip_service_status_valid ? "valid": "not valid",
          resp_msg_ptr->voip_service_status,
          resp_msg_ptr->voip_service_rat_valid ? "valid": "not valid",
          resp_msg_ptr->voip_service_rat);
      QCRIL_LOG_ESSENTIAL("VT: service_status(%s): %d, rat(%s): %d",
          resp_msg_ptr->vt_service_status_valid ? "valid": "not valid",
          resp_msg_ptr->vt_service_status,
          resp_msg_ptr->vt_service_rat_valid ? "valid": "not valid",
          resp_msg_ptr->vt_service_rat);
      QCRIL_LOG_ESSENTIAL("UT: service_status(%s): %d, rat(%s): %d",
          resp_msg_ptr->ut_service_status_valid ? "valid": "not valid",
          resp_msg_ptr->ut_service_status,
          resp_msg_ptr->ut_service_rat_valid ? "valid": "not valid",
          resp_msg_ptr->ut_service_rat);

      qcril_qmi_imsa_info_lock();
      qcril_qmi_imsa_store_service_status_cl(
          FALSE,
          IMSA_NO_SERVICE_V01,
          resp_msg_ptr->voip_service_status_valid,
          resp_msg_ptr->voip_service_status,
          resp_msg_ptr->vt_service_status_valid,
          resp_msg_ptr->vt_service_status,
          resp_msg_ptr->sms_service_rat_valid,
          resp_msg_ptr->sms_service_rat,
          resp_msg_ptr->voip_service_rat_valid,
          resp_msg_ptr->voip_service_rat,
          resp_msg_ptr->vt_service_rat_valid,
          resp_msg_ptr->vt_service_rat,
          resp_msg_ptr->ut_service_status_valid,
          resp_msg_ptr->ut_service_status,
          resp_msg_ptr->ut_service_rat_valid,
          resp_msg_ptr->ut_service_rat );

      srvStatus = qcril_qmi_ims_create_ServiceStatusInfoList(&qcril_qmi_imsa_info.ims_srv_status);

      qcril_qmi_imsa_info_unlock();

      errorCode = RIL_E_SUCCESS;

      // Trigger QCRIL_AM_EVENT_IMS_SRV_CHANGED only if the VoIP/VT RAT is valid
      if (resp_msg_ptr->voip_service_rat_valid || resp_msg_ptr->vt_service_rat_valid)
      {
        // TODO: <IMS_REFACTOR> use AM message
        qcril_am_handle_event(QCRIL_AM_EVENT_IMS_SRV_CHANGED, NULL);
      }
    }
    else
    {
      QCRIL_LOG_INFO("error: %d", resp_msg_ptr->resp.error);
      qcril_qmi_imsa_info_lock();
      qcril_qmi_imsa_info.ims_srv_status_valid = FALSE;
      qcril_qmi_imsa_info_unlock();

      errorCode = RIL_E_GENERIC_FAILURE;
    }
  } while (FALSE);

  if (pendingMsg != nullptr)
  {
    auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(errorCode, srvStatus);
    auto msg(std::static_pointer_cast<QcRilRequestImsQueryServiceStatusMessage>(pendingMsg));
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
  }
  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imsa_get_service_status_resp_hdlr

//===========================================================================
// qcril_qmi_imsa_get_rtp_statistics_resp_hdlr
//===========================================================================
void qcril_qmi_imsa_get_rtp_statistics_resp_hdlr
(
   unsigned int                 msg_id,
   std::shared_ptr<void>        resp_c_struct,
   unsigned int                 resp_c_struct_len,
   void                        *resp_cb_data,
   qmi_client_error_type        transp_err
)
{
    (void)msg_id;
    (void)resp_c_struct_len;
    imsa_get_rtp_statistics_resp_msg_v01 *resp_msg_ptr = NULL;
    uint32 user_data = (uint32)(uintptr_t) resp_cb_data;
    uint16_t req_id = QCRIL_EXTRACT_USER_ID_FROM_USER_DATA(user_data);
    auto pendingMsg = getImsModule()->getPendingMessageList().find(req_id);
    RIL_Errno errorCode = RIL_E_GENERIC_FAILURE;
    boolean sendResponse = false;
    QCRIL_LOG_FUNC_ENTRY();

    std::shared_ptr<qcril::interfaces::RtpStatisticsData> rtpStats =
        std::make_shared<qcril::interfaces::RtpStatisticsData>();

    do {
        if (pendingMsg == nullptr)
        {
            QCRIL_LOG_ERROR("pendingMsg is null");
            break;
        }
        if (transp_err != QMI_NO_ERR)
        {
            QCRIL_LOG_ERROR("Transp error (%d) recieved from QMI", transp_err);
            errorCode = RIL_E_GENERIC_FAILURE;
            sendResponse = true;
            break;
        }
        resp_msg_ptr = (imsa_get_rtp_statistics_resp_msg_v01*)(resp_c_struct.get());
        if (resp_msg_ptr == nullptr)
        {
            QCRIL_LOG_ERROR("resp_msg_ptr is null");
            errorCode = RIL_E_GENERIC_FAILURE;
            sendResponse = true;
            break;
        }
        if (QMI_RESULT_SUCCESS_V01 != resp_msg_ptr->resp.result)
        {
            QCRIL_LOG_INFO("error: %d", resp_msg_ptr->resp.error);
            errorCode = RIL_E_GENERIC_FAILURE;
            sendResponse = true;
        }
    } while(0);

    //send response only in error scenario
    if(sendResponse && pendingMsg) {
        auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(errorCode, nullptr);
        auto ril_msg = std::static_pointer_cast<QcRilRequestMessage>(pendingMsg);
        ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, respPayload);
        getImsModule()->getPendingMessageList().erase(req_id);
    }

    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imsa_get_rtp_statistics_resp_hdlr

//===========================================================================
// qcril_qmi_imsa_set_geo_loc_resp_hdlr
//===========================================================================
void qcril_qmi_imsa_set_geo_loc_resp_hdlr
(
   unsigned int                 msg_id,
   std::shared_ptr<void>        resp_c_struct,
   unsigned int                 resp_c_struct_len,
   void                        *resp_cb_data,
   qmi_client_error_type        transp_err
)
{
    (void)msg_id;
    (void)resp_c_struct_len;
    imsa_set_geo_loc_rsp_msg_v01 *resp = NULL;
    RIL_Errno ril_err = RIL_E_SUCCESS;

    uint32 user_data = (uint32)(uintptr_t) resp_cb_data;
    uint16_t req_id = QCRIL_EXTRACT_USER_ID_FROM_USER_DATA(user_data);
    auto pendingMsg = getImsModule()->getPendingMessageList().extract(req_id);

    QCRIL_LOG_FUNC_ENTRY();
    do {
        if (pendingMsg == nullptr)
        {
          QCRIL_LOG_ERROR("pendingMsg is null");
          break;
        }
        //TODO revisit this
        if (transp_err != QMI_NO_ERR)
        {
          QCRIL_LOG_ERROR("Transp error (%d) recieved from QMI", transp_err);
          break;
        }
        resp = (imsa_set_geo_loc_rsp_msg_v01*)(resp_c_struct.get());
        if (resp == nullptr)
        {
          QCRIL_LOG_ERROR("resp is null");
          break;
        }
        if (QMI_RESULT_SUCCESS_V01 != resp->resp.result)
        {
            QCRIL_LOG_INFO("error: %d", resp->resp.error);
            ril_err = RIL_E_GENERIC_FAILURE;
        }
        QCRIL_LOG_INFO("ims_error: %d", ril_err);
    } while(FALSE);

    if (pendingMsg) {
        auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(ril_err, nullptr);
        auto ril_msg = std::static_pointer_cast<QcRilRequestImsGeoLocationInfoMessage>(pendingMsg);
        ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, respPayload);
    }

    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imsa_set_geo_loc_resp_hdlr

//===========================================================================
// qcril_qmi_imsa_get_virtual_line_info_resp_hdlr
//===========================================================================
void qcril_qmi_imsa_get_virtual_line_info_resp_hdlr
(
  unsigned int                 msg_id,
  std::shared_ptr<void>        resp_c_struct,
  unsigned int                 resp_c_struct_len,
  void                        *resp_cb_data,
  qmi_client_error_type        transp_err
)
{
  QCRIL_LOG_FUNC_ENTRY();
  (void)msg_id;
  (void)resp_c_struct_len;
  RIL_Errno ril_err = RIL_E_GENERIC_FAILURE;
  imsa_get_virtual_line_rsp_msg_v01 *resp_msg_ptr = NULL;
  uint32 user_data = (uint32)(uintptr_t) resp_cb_data;
  uint16_t req_id = QCRIL_EXTRACT_USER_ID_FROM_USER_DATA(user_data);

  auto pendingMsg = getImsModule()->getPendingMessageList().extract(req_id);
  std::shared_ptr<qcril::interfaces::VirtualLineInfo> info = nullptr;
  std::shared_ptr<QcRilRequestImsQueryVirtualLineInfo> ril_msg = nullptr;
  do {
    if (pendingMsg == nullptr)
    {
      QCRIL_LOG_ERROR("pendingMsg is null");
      break;
    }
    ril_msg =
      std::static_pointer_cast<QcRilRequestImsQueryVirtualLineInfo>(pendingMsg);

    if (transp_err != QMI_NO_ERR)
    {
      QCRIL_LOG_ERROR("Transp error (%d) recieved from QMI", transp_err);
      ril_err = RIL_E_GENERIC_FAILURE;
      break;
    }
    resp_msg_ptr = (imsa_get_virtual_line_rsp_msg_v01*)(resp_c_struct.get());
    if (resp_msg_ptr == nullptr)
    {
      QCRIL_LOG_ERROR("resp_msg_ptr is null");
      ril_err = RIL_E_GENERIC_FAILURE;
      break;
    }
    if (QMI_RESULT_SUCCESS_V01 == resp_msg_ptr->resp.result &&
        resp_msg_ptr->virtual_line_info_valid)
    {
      std::vector<std::string> virtual_lines;
      if (resp_msg_ptr->virtual_line_info.uri_list_len > 0)
      {
        for (uint32_t i = 0;
             (i < resp_msg_ptr->virtual_line_info.uri_list_len) && (i < IMSA_DIGITS_MAX_URIS_V01);
             i++)
        {
          virtual_lines.push_back(std::string(resp_msg_ptr->virtual_line_info.uri_list[i].uri));
        }
        info = std::make_shared<qcril::interfaces::VirtualLineInfo>(ril_msg->getMsisdn(),
                                                                    virtual_lines);
        ril_err = RIL_E_SUCCESS;
      }
    } else {
      QCRIL_LOG_INFO("error: %d", resp_msg_ptr->resp.error);
      ril_err = RIL_E_GENERIC_FAILURE;
    }
  } while(FALSE);
  if (pendingMsg && ril_msg) {
    auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(ril_err, info);
    ril_msg->sendResponse(ril_msg, Message::Callback::Status::SUCCESS, respPayload);
  }

  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imsa_get_virtual_line_info_resp_hdlr

//===========================================================================
// qcril_qmi_imsa_command_cb
//===========================================================================
void qcril_qmi_imsa_command_cb
(
   unsigned int                 msg_id,
   std::shared_ptr<void>        resp_c_struct,
   unsigned int                 resp_c_struct_len,
   void                        *resp_cb_data,
   qmi_client_error_type        transp_err
)
{
  qcril_instance_id_e_type instance_id;
  uint32 user_data;
  uint16 req_id;
  qcril_reqlist_public_type req_info;
  qcril_request_params_type req_data;

  QCRIL_LOG_FUNC_ENTRY();

  memset(&req_data, 0, sizeof(req_data));
  req_data.datalen = resp_c_struct_len;
  req_data.data = resp_c_struct.get();

  QCRIL_LOG_INFO("msg_id 0x%.2x", msg_id);

  if (QMI_IMSA_IND_REG_RSP_V01 == msg_id)
  {
    if( transp_err != QMI_NO_ERR )
    {
      QCRIL_LOG_INFO("Transp error (%d) recieved", transp_err);
    }
    else
    {
      qcril_qmi_imsa_ind_reg_resp_hdlr(&req_data);
    }
  }
  else
  {
    user_data = ( uint32 )(uintptr_t) resp_cb_data;
    instance_id = (qcril_instance_id_e_type)QCRIL_EXTRACT_INSTANCE_ID_FROM_USER_DATA( user_data );
    req_id = QCRIL_EXTRACT_USER_ID_FROM_USER_DATA( user_data );
    req_data.instance_id = instance_id;
    req_data.modem_id = QCRIL_DEFAULT_MODEM_ID;
    /* Lookup the Token ID */
    if ( qcril_reqlist_query_by_req_id( req_id, &instance_id, &req_info ) == E_SUCCESS )
    {
      if( transp_err != QMI_NO_ERR )
      {
        QCRIL_LOG_INFO("Transp error (%d) recieved from QMI for RIL request %d",
            transp_err, req_info.request);
        qcril_send_empty_payload_request_response(instance_id, req_info.t,
            req_info.request, RIL_E_GENERIC_FAILURE ); // Send GENERIC_FAILURE response
      }
      else
      {
        req_data.t = req_info.t;
        req_data.event_id = req_info.request;
        switch(msg_id)
        {
          case QMI_IMSA_GET_VIRTUAL_LINE_RSP_V01:
          {
            //retrieve the stored msisdn information
            const char* msisdn = (const char*)req_info.req_data;
            if (!msisdn)
            {
              QCRIL_LOG_ERROR("QMI_IMSA_GET_VIRTUAL_LINE_RSP_V01 :: stored msisdn is null.");
              return;
            }
            //qcril_qmi_imsa_get_virtual_line_info_resp_hdlr(&req_data, std::string(msisdn));
            break;
          }

          default:
            QCRIL_LOG_INFO("Unsupported QMI IMSA message %d", msg_id);
            break;
        }
      }
    }
    else
    {
      QCRIL_LOG_ERROR( "Req ID: %d not found", req_id );
    }
  }

  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imsa_command_cb


//===========================================================================
// qcril_qmi_imsa_request_query_virtual_line_info
//===========================================================================
void qcril_qmi_imsa_request_query_virtual_line_info
(
    std::shared_ptr<QcRilRequestImsQueryVirtualLineInfo> msg
)
{
  QCRIL_LOG_FUNC_ENTRY();
  RIL_Errno res = RIL_E_GENERIC_FAILURE;

  bool imsa_inited  = FALSE;
  qcril_qmi_imsa_info_lock();
  imsa_inited = qcril_qmi_imsa_info.inited ;
  qcril_qmi_imsa_info_unlock();

  uint32 user_data;
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  bool sendResponse = false;
  //cleanup request from pendingmsglist
  bool doCleanup = true;
  uint16_t req_id = 0;
  if (msg != nullptr) {
    do {
      if (!imsa_inited) {
        sendResponse = true;
        doCleanup = false;
        break;
      }
      if(msg->getMsisdn().empty()) {
        sendResponse = true;
        doCleanup = false;
        break;
      }
      std::string& msisdn = msg->getMsisdn();
      QCRIL_LOG_INFO(" Query Line MSISDN : %s ", msisdn.c_str());
      imsa_get_virtual_line_req_msg_v01 qmi_req;
      memset(&qmi_req, 0, sizeof(qmi_req));
      strlcpy(qmi_req.msisdn, msisdn.c_str(), sizeof(qmi_req.msisdn));
      auto pendingMsgStatus = getImsModule()->getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true)
      {
          sendResponse = true;
          doCleanup = false;
          break;
      }
      req_id = pendingMsgStatus.first;
      user_data = QCRIL_COMPOSE_USER_DATA( instance_id,
                                           QCRIL_DEFAULT_MODEM_ID,
                                           req_id );
      qmi_client_error_type qmi_client_error = qmi_client_imsa_send_async(
                                        QMI_IMSA_GET_VIRTUAL_LINE_REQ_V01,
                                        &qmi_req,
                                        sizeof(imsa_get_virtual_line_req_msg_v01),
                                        sizeof(imsa_get_virtual_line_rsp_msg_v01),
                                        qcril_qmi_imsa_get_virtual_line_info_resp_hdlr,
                                        (void*)(uintptr_t)user_data);
      QCRIL_LOG_INFO(".. qmi send async res %d", (int) qmi_client_error );
      if (QMI_NO_ERR != qmi_client_error)
      {
        sendResponse = true;
        doCleanup = true;
      }
    } while(FALSE);
    if (sendResponse == true) {
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(res, nullptr);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
      if (doCleanup) {
        getImsModule()->getPendingMessageList().erase(req_id);
      }
    }
  }
  QCRIL_LOG_FUNC_RETURN();
} //qcril_qmi_imsa_request_query_virtual_line_info

//===========================================================================
// qcril_qmi_imsa_request_ims_registration_state
//===========================================================================
void qcril_qmi_imsa_request_ims_registration_state(
    std::shared_ptr<QcRilRequestImsGetRegStateMessage> msg)
{
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  uint32 user_data;
  RIL_Errno errorCode = RIL_E_SUCCESS;
  std::shared_ptr<qcril::interfaces::Registration> reg = nullptr;
  bool sendResponse = false;
  uint16_t req_id = -1;
  bool doCleanup = false;

  QCRIL_LOG_FUNC_ENTRY();

  qcril_qmi_imsa_info_lock();

  if (msg != nullptr)
  {
    do
    {
      if (!qcril_qmi_imsa_info.inited)
      {
        errorCode = RIL_E_GENERIC_FAILURE;
        sendResponse = true;
        break;
      }

      if (qcril_qmi_imsa_info.ims_registered_valid ||
          qcril_qmi_imsa_info.ims_registration_error_code_valid ||
          qcril_qmi_imsa_info.ims_registration_error_string_valid)
      {
        reg = qcril_qmi_ims_create_Registration(
            qcril_qmi_imsa_info.ims_registered_valid, qcril_qmi_imsa_info.ims_registered,
            qcril_qmi_imsa_info.registration_network_valid,
            qcril_qmi_imsa_info.registration_network,
            qcril_qmi_imsa_info.ims_registration_error_code_valid,
            qcril_qmi_imsa_info.ims_registration_error_code,
            (qcril_qmi_imsa_info.ims_registration_error_string_valid
             ? qcril_qmi_imsa_info.ims_registration_error_string
             : nullptr),
            (qcril_qmi_imsa_info.uri_list_valid ? qcril_qmi_imsa_info.uri_list_len : 0),
            qcril_qmi_imsa_info.URI_List);
        errorCode = RIL_E_SUCCESS;
        sendResponse = true;
        break;
      }

      auto pendingMsgStatus = getImsModule()->getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true)
      {
        errorCode = RIL_E_GENERIC_FAILURE;
        sendResponse = true;
        break;
      }
      req_id = pendingMsgStatus.first;

      user_data = QCRIL_COMPOSE_USER_DATA(instance_id, QCRIL_DEFAULT_MODEM_ID, req_id);

      if (qmi_client_imsa_send_async(QMI_IMSA_GET_REGISTRATION_STATUS_REQ_V01,
            NULL,
            0,
            sizeof(imsa_get_registration_status_resp_msg_v01),
            qcril_qmi_imsa_get_reg_status_resp_hdlr,
            (void *)(uintptr_t) user_data) != QMI_NO_ERR)
      {
        errorCode = RIL_E_GENERIC_FAILURE;
        sendResponse = true;
        doCleanup = true;
        break;
      }
    } while(FALSE);

    if (sendResponse)
    {
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(errorCode, reg);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
      if (doCleanup) {
        getImsModule()->getPendingMessageList().erase(req_id);
      }
    }
  }
  else
  {
    QCRIL_LOG_ERROR("msg is NULL");
  }
  qcril_qmi_imsa_info_unlock();

  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imsa_request_ims_registration_state

//===========================================================================
// qcril_qmi_imsa_request_query_ims_srv_status
//===========================================================================
void qcril_qmi_imsa_request_query_ims_srv_status(
    std::shared_ptr<QcRilRequestImsQueryServiceStatusMessage> msg)
{
  qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
  uint32 user_data;
  bool sendResponse = false;
  RIL_Errno errorCode = RIL_E_SUCCESS;
  std::shared_ptr<qcril::interfaces::ServiceStatusInfoList> srvStatus = nullptr;
  QCRIL_LOG_FUNC_ENTRY();
  uint16_t req_id = -1;
  bool doCleanup = false;

  if (msg != nullptr)
  {
    qcril_qmi_imsa_info_lock();
    do
    {
      if (!qcril_qmi_imsa_info.inited)
      {
        errorCode = RIL_E_GENERIC_FAILURE;
        sendResponse = true;
        break;
      }

      if (qcril_qmi_imsa_info.ims_srv_status_valid &&
          qcril_qmi_imsa_info.ims_status_change_registered)
      {
        // Use cached info to reply back
        errorCode = RIL_E_SUCCESS;
        srvStatus = qcril_qmi_ims_create_ServiceStatusInfoList(&qcril_qmi_imsa_info.ims_srv_status);
        sendResponse = true;
        break;
      }

      auto pendingMsgStatus = getImsModule()->getPendingMessageList().insert(msg);
      if (pendingMsgStatus.second != true)
      {
        errorCode = RIL_E_GENERIC_FAILURE;
        sendResponse = true;
        break;
      }
      req_id = pendingMsgStatus.first;

      user_data = QCRIL_COMPOSE_USER_DATA(instance_id, QCRIL_DEFAULT_MODEM_ID, req_id);

      if (qmi_client_imsa_send_async(QMI_IMSA_GET_SERVICE_STATUS_REQ_V01, NULL, 0,
            sizeof(imsa_get_service_status_resp_msg_v01),
            qcril_qmi_imsa_get_service_status_resp_hdlr,
            (void *)(uintptr_t)user_data) != QMI_NO_ERR)
      {
        errorCode = RIL_E_GENERIC_FAILURE;
        sendResponse = true;
        doCleanup = true;
        break;
      }
    } while (0);
    qcril_qmi_imsa_info_unlock();
    if (sendResponse)
    {
      auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(errorCode, srvStatus);
      msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
      if (doCleanup) {
        getImsModule()->getPendingMessageList().erase(req_id);
      }
    }
  } else {
    QCRIL_LOG_ERROR("msg is NULL");
  }

  QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imsa_request_query_ims_srv_status

//===========================================================================
// qcril_qmi_imsa_request_get_rtp_statistics
//===========================================================================
void qcril_qmi_imsa_request_get_rtp_statistics
(
    std::shared_ptr<QcRilRequestImsGetRtpStatsMessage> msg
)
{
    qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
    uint32 user_data;
    RIL_Errno errorCode = RIL_E_SUCCESS;
    boolean failed = FALSE;
    std::shared_ptr<qcril::interfaces::RtpStatisticsData> rtpStats =
        std::make_shared<qcril::interfaces::RtpStatisticsData>();
    uint16_t req_id = -1;
    bool doCleanup = false;

    QCRIL_LOG_FUNC_ENTRY();

    if( NULL != msg )
    {
        qcril_qmi_imsa_info_lock();
        if (qcril_qmi_imsa_info.inited)
        {
            do
            {
                auto pendingMsgStatus = getImsModule()->getPendingMessageList().insert(msg);
                if (pendingMsgStatus.second != true)
                {
                    errorCode = RIL_E_GENERIC_FAILURE;
                    failed = true;
                    break;
                  }
                req_id = pendingMsgStatus.first;

                user_data = QCRIL_COMPOSE_USER_DATA( instance_id,
                                                     QCRIL_DEFAULT_MODEM_ID,
                                                     req_id );

                if (qmi_client_imsa_send_async(QMI_IMSA_GET_RTP_STATISTICS_REQ_V01,
                      NULL,
                      0,
                      sizeof(imsa_get_rtp_statistics_resp_msg_v01),
                      qcril_qmi_imsa_get_rtp_statistics_resp_hdlr,
                      (void *)(uintptr_t) user_data) != QMI_NO_ERR)
                {
                    errorCode = RIL_E_GENERIC_FAILURE;
                    failed = TRUE;
                    doCleanup = true;
                    break;
                }
            } while(0);
        }
        qcril_qmi_imsa_info_unlock();
        if (failed)
        {
            auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(errorCode,
                                nullptr);
            msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
            if (doCleanup) {
                getImsModule()->getPendingMessageList().erase(req_id);
            }
        }
    }
    else
    {
        QCRIL_LOG_ERROR("msg is NULL");
    }

    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imsa_request_get_rtp_statistics

//===========================================================================
// qcril_qmi_imsa_request_get_rtp_error_statistics
//===========================================================================
void qcril_qmi_imsa_request_get_rtp_error_statistics
(
    std::shared_ptr<QcRilRequestImsGetRtpErrorStatsMessage> msg
)
{
    qcril_instance_id_e_type instance_id = QCRIL_DEFAULT_INSTANCE_ID;
    uint32 user_data;
    RIL_Errno errorCode = RIL_E_SUCCESS;
    boolean failed = FALSE;
    std::shared_ptr<qcril::interfaces::RtpStatisticsData> rtpStats =
        std::make_shared<qcril::interfaces::RtpStatisticsData>();
    uint16_t req_id = -1;
    bool doCleanup = false;
    QCRIL_LOG_FUNC_ENTRY();

    if( NULL != msg )
    {
        qcril_qmi_imsa_info_lock();
        if (qcril_qmi_imsa_info.inited)
        {
            do
            {
                auto pendingMsgStatus = getImsModule()->getPendingMessageList().insert(msg);
                if (pendingMsgStatus.second != true)
                {
                    errorCode = RIL_E_GENERIC_FAILURE;
                    failed = true;
                    break;
                  }
                req_id = pendingMsgStatus.first;

                user_data = QCRIL_COMPOSE_USER_DATA( instance_id,
                                                     QCRIL_DEFAULT_MODEM_ID,
                                                     req_id );

                if (qmi_client_imsa_send_async(QMI_IMSA_GET_RTP_STATISTICS_REQ_V01,
                      NULL,
                      0,
                      sizeof(imsa_get_rtp_statistics_resp_msg_v01),
                      qcril_qmi_imsa_get_rtp_statistics_resp_hdlr,
                      (void *)(uintptr_t) user_data) != QMI_NO_ERR)
                {
                    errorCode = RIL_E_GENERIC_FAILURE;
                    failed = TRUE;
                    doCleanup = true;
                    break;
                }
            } while(0);
        }
        qcril_qmi_imsa_info_unlock();
        if (failed)
        {
            auto respPayload = std::make_shared<QcRilRequestMessageCallbackPayload>(errorCode,
                                nullptr);
            msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
            if (doCleanup) {
                getImsModule()->getPendingMessageList().erase(req_id);
            }
        }
    }
    else
    {
        QCRIL_LOG_ERROR("msg is NULL");
    }

    QCRIL_LOG_FUNC_RETURN();
} // qcril_qmi_imsa_request_get_rtp_statistics

static ImsServiceStatusInd::ServiceStatus qcril_qmi_imsa_convert_qmi_ss_to_imsa_ss (
    imsa_service_status_enum_v01 qmi_ss) {
  switch (qmi_ss) {
    case IMSA_NO_SERVICE_V01:
      return ImsServiceStatusInd::ServiceStatus::NO_SERVICE;
      break;
    case IMSA_LIMITED_SERVICE_V01:
      return ImsServiceStatusInd::ServiceStatus::LIMITED_SERVICE;
      break;
    case IMSA_FULL_SERVICE_V01:
      return ImsServiceStatusInd::ServiceStatus::FULL_SERVICE;
      break;
    default:
      return ImsServiceStatusInd::ServiceStatus::INVALID;
      break;
  }
}

static ImsServiceStatusInd::ServiceRat qcril_qmi_imsa_convert_qmi_rat_to_imsa_rat (
    imsa_service_rat_enum_v01 qmi_rat) {
  switch (qmi_rat) {
    case IMSA_WLAN_V01:
      return ImsServiceStatusInd::ServiceRat::WLAN;
      break;
    case IMSA_WWAN_V01:
      return ImsServiceStatusInd::ServiceRat::WWAN;
      break;
    case IMSA_IWLAN_V01:
      return ImsServiceStatusInd::ServiceRat::IWLAN;
      break;
    default:
      return ImsServiceStatusInd::ServiceRat::INVALID;
      break;
  }
}


static ImsServiceStatusInd::RegistrationStatus qcril_qmi_imsa_convert_qmi_reg_to_imsa_reg (
    imsa_ims_registration_status_enum_v01 reg_status) {
  switch (reg_status) {
    case IMSA_STATUS_NOT_REGISTERED_V01:
      return ImsServiceStatusInd::RegistrationStatus::NOT_REGISTERED;
      break;
    case IMSA_STATUS_REGISTERING_V01:
      return ImsServiceStatusInd::RegistrationStatus::REGISTERING;
      break;
    case IMSA_STATUS_REGISTERED_V01:
      return ImsServiceStatusInd::RegistrationStatus::REGISTERED;
      break;
    case IMSA_STATUS_LIMITED_REGISTERED_V01:
      return ImsServiceStatusInd::RegistrationStatus::LIMITED_REGISTERED;
      break;
    default:
      return ImsServiceStatusInd::RegistrationStatus::INVALID;
      break;
  }
}

void qcril_qmi_imsa_broadcast_service_status (const qcril_qmi_imsa_info_type& qcril_qmi_imsa_info) {
  ImsServiceStatusInd::ServiceStatusInfo imsServiceStatus;

  QCRIL_LOG_FUNC_ENTRY();

  imsServiceStatus.reg_status =
    qcril_qmi_imsa_info.ims_registered_valid ?
    qcril_qmi_imsa_convert_qmi_reg_to_imsa_reg(
        (imsa_ims_registration_status_enum_v01) qcril_qmi_imsa_info.ims_registered) :
    ImsServiceStatusInd::RegistrationStatus::INVALID;

  imsServiceStatus.voip_service_status =
    qcril_qmi_imsa_info.ims_srv_status.voip_service_status_valid ?
    qcril_qmi_imsa_convert_qmi_ss_to_imsa_ss(
        qcril_qmi_imsa_info.ims_srv_status.voip_service_status) :
    ImsServiceStatusInd::ServiceStatus::INVALID;
  imsServiceStatus.voip_service_rat =
    qcril_qmi_imsa_info.ims_srv_status.voip_service_rat_valid ?
    qcril_qmi_imsa_convert_qmi_rat_to_imsa_rat(
        qcril_qmi_imsa_info.ims_srv_status.voip_service_rat) :
    ImsServiceStatusInd::ServiceRat::INVALID;

  imsServiceStatus.vt_service_status =
    qcril_qmi_imsa_info.ims_srv_status.vt_service_status_valid ?
    qcril_qmi_imsa_convert_qmi_ss_to_imsa_ss(
        qcril_qmi_imsa_info.ims_srv_status.vt_service_status) :
    ImsServiceStatusInd::ServiceStatus::INVALID;
  imsServiceStatus.vt_service_rat =
    qcril_qmi_imsa_info.ims_srv_status.vt_service_rat_valid ?
    qcril_qmi_imsa_convert_qmi_rat_to_imsa_rat(
        qcril_qmi_imsa_info.ims_srv_status.vt_service_rat) :
    ImsServiceStatusInd::ServiceRat::INVALID;

  QCRIL_LOG_DEBUG("Broadcasting ImsServiceStatusInd");

  std::shared_ptr<ImsServiceStatusInd> msg =
    std::make_shared<ImsServiceStatusInd>(imsServiceStatus);
  Dispatcher::getInstance().broadcast(msg);

  QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
//@function qcril_qmi_imsa_ims_capability_ind_hdlr
//
//@brief Handler for ims capability indication
//===========================================================================
void qcril_qmi_imsa_ims_capability_ind_hdlr
(
    dms_ims_capability_ind_msg_v01 * payload
)
{
    uint64_t ims_sub_config[QMI_DMS_MAX_SUBSCRIPTION_LIST_LEN_V01];
    int i;
    std::shared_ptr<qcril::interfaces::ImsSubConfigInfo> subConfig =
        std::make_shared<qcril::interfaces::ImsSubConfigInfo>();

    QCRIL_LOG_FUNC_ENTRY();
    do {
        if (subConfig == nullptr) {
            break;
        }

        if(payload == NULL) {
            QCRIL_LOG_ERROR("Indication payload is null.");
            break;
        }

        qcril_qmi_imsa_info_lock();
        qcril_qmi_imsa_info.ims_capability_len = payload->ims_capability_len;
        memcpy(qcril_qmi_imsa_info.ims_capability, payload->ims_capability,
               sizeof(qcril_qmi_imsa_info.ims_capability));

        if(qcril_qmi_imsa_info.max_ims_instances_valid)
        {
            subConfig->setSimultStackCount(qcril_qmi_imsa_info.max_ims_instances);
        }

        memset(&ims_sub_config, 0, sizeof(ims_sub_config));
        qcril_qmi_imsa_get_ims_sub_configs(ims_sub_config, QMI_DMS_MAX_SUBSCRIPTION_LIST_LEN_V01);
        subConfig->setCount(QMI_DMS_MAX_SUBSCRIPTION_LIST_LEN_V01);
        for(i = 0; i < QMI_DMS_MAX_SUBSCRIPTION_LIST_LEN_V01; ++i) {
            subConfig->addToImsStackEnabled(ims_sub_config[i]);
        }

        qcril_qmi_imsa_info_unlock();

        auto msg = std::make_shared<QcRilUnsolImsSubConfigIndication>(subConfig);
        if (msg != nullptr)
        {
            Dispatcher::getInstance().dispatchSync(msg);
        }

    } while(FALSE);

    QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
//@function qcril_qmi_imsa_request_get_ims_sub_config
//
//@brief Handle event QCRIL_EVT_IMS_SOCKET_REQ_GET_IMS_SUB_CONFIG
//===========================================================================
void qcril_qmi_imsa_request_get_ims_sub_config
(
    std::shared_ptr<QcRilRequestImsGetSubConfigMessage> msg
)
{
    RIL_Errno res = RIL_E_GENERIC_FAILURE;
    uint64_t ims_sub_config[QMI_DMS_MAX_SUBSCRIPTION_LIST_LEN_V01];
    int i;
    bool sendResponse = false;
    RIL_Errno errorCode = RIL_E_SUCCESS;
    QCRIL_LOG_FUNC_ENTRY();


    std::shared_ptr<qcril::interfaces::ImsSubConfigInfo> subConfig =
        std::make_shared<qcril::interfaces::ImsSubConfigInfo>();

    if (msg) {
        do {

            if (subConfig == nullptr) {
                QCRIL_LOG_INFO("memory failure");
                errorCode = RIL_E_GENERIC_FAILURE;
                sendResponse = true;
                break;
            }

            res = qcril_qmi_nas_get_device_capability(FALSE, FALSE, FALSE, TRUE);
            if(res == RIL_E_SUCCESS)
            {
                qcril_qmi_imsa_info_lock();
                if(qcril_qmi_imsa_info.max_ims_instances_valid)
                {
                    subConfig->setSimultStackCount(qcril_qmi_imsa_info.max_ims_instances);
                }
                else
                {
                    qcril_qmi_imsa_info_unlock();
                    errorCode = RIL_E_REQUEST_NOT_SUPPORTED;
                    sendResponse = true;
                    QCRIL_LOG_INFO("max_ims_instances is not valid, sending unsupported error.");
                    break;
                }

                if(qcril_qmi_imsa_info.ims_capability_len > 0)
                {
                    memset(&ims_sub_config, 0, sizeof(ims_sub_config));
                    qcril_qmi_imsa_get_ims_sub_configs(ims_sub_config,
                        QMI_DMS_MAX_SUBSCRIPTION_LIST_LEN_V01);
                    subConfig->setCount(QMI_DMS_MAX_SUBSCRIPTION_LIST_LEN_V01);
                    for(i = 0; i < QMI_DMS_MAX_SUBSCRIPTION_LIST_LEN_V01; ++i) {
                        subConfig->addToImsStackEnabled(ims_sub_config[i]);
                    }
                }
                qcril_qmi_imsa_info_unlock();
                errorCode = RIL_E_SUCCESS;
                sendResponse = true;
                break;
            }
            else
            {
                errorCode = RIL_E_GENERIC_FAILURE;
                sendResponse = true;
                break;
            }
        } while (FALSE);
        if (sendResponse) {
            auto respPayload =
                std::make_shared<QcRilRequestMessageCallbackPayload>(errorCode, subConfig);
            msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
        }
    }
    QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
//@function qcril_qmi_imsa_store_ims_capability
//
//@brief Utility function to store the ims capability in the cache.
//===========================================================================
void qcril_qmi_imsa_store_ims_capability
(
    uint8_t max_ims_instances_valid,
    uint8_t max_ims_instances,
    uint8_t ims_capability_valid,
    uint32_t ims_capability_len,
    dms_ims_capability_type_v01* ims_capability
)
{
    QCRIL_LOG_FUNC_ENTRY();
    qcril_qmi_imsa_info_lock();
    qcril_qmi_imsa_info.max_ims_instances_valid = max_ims_instances_valid;
    qcril_qmi_imsa_info.max_ims_instances = max_ims_instances;

    memset(&qcril_qmi_imsa_info.ims_capability, 0,
           sizeof(qcril_qmi_imsa_info.ims_capability));

    if( ims_capability_valid )
    {
        qcril_qmi_imsa_info.ims_capability_len = ims_capability_len;
        memcpy(qcril_qmi_imsa_info.ims_capability, ims_capability,
               sizeof(qcril_qmi_imsa_info.ims_capability));
    }
    else
    {
        qcril_qmi_imsa_info.ims_capability_len = 0;
    }

    qcril_qmi_imsa_info_unlock();
    QCRIL_LOG_FUNC_RETURN();
}

//===========================================================================
//@function qcril_qmi_imsa_get_ims_sub_configs
//
//@brief Utility function to convert qmi response to telephony ims response.
//===========================================================================
void qcril_qmi_imsa_get_ims_sub_configs(uint64_t* ims_sub_configs, uint8_t max_cap_len)
{
    uint32_t i;
    uint32_t ims_cap_len;
    dms_ims_capability_type_v01 ims_cap[QMI_DMS_MAX_SUBSCRIPTION_LIST_LEN_V01];

    QCRIL_LOG_FUNC_ENTRY();
    ims_cap_len = qcril_qmi_imsa_info.ims_capability_len;
    memset(&ims_cap, 0, sizeof(ims_cap));
    memcpy(ims_cap, qcril_qmi_imsa_info.ims_capability, sizeof(ims_cap));

    for(i = 0; i < ims_cap_len && ims_cap_len <= max_cap_len; ++i)
    {
        if(ims_cap[i].subscription == DMS_PRIMARY_SUBS_V01)
        {
            ims_sub_configs[RIL_SUBSCRIPTION_1] = ims_cap[i].enabled;
            QCRIL_LOG_INFO("Primary sub ims_cap:%d", ims_cap[i].enabled);
        }
        if(ims_cap[i].subscription == DMS_SECONDARY_SUBS_V01)
        {
            ims_sub_configs[RIL_SUBSCRIPTION_2] = ims_cap[i].enabled;
            QCRIL_LOG_INFO("Secondary sub ims_cap:%d", ims_cap[i].enabled);
        }
        if(ims_cap[i].subscription == DMS_TERTIARY_SUBS_V01)
        {
            ims_sub_configs[RIL_SUBSCRIPTION_3] = ims_cap[i].enabled;
            QCRIL_LOG_INFO("Tertiary sub ims_cap:%d", ims_cap[i].enabled);
        }
    }
    QCRIL_LOG_FUNC_RETURN();
}

/*=========================================================================
  FUNCTION:   copy_qmi_blk_status_to_ims_registration_block_status

===========================================================================*/
/*!
    @brief
    Copy Block Status Indication information.

    @return
    If copied information, return TRUE
    Else return FALSE
*/
/*=========================================================================*/
std::shared_ptr<qcril::interfaces::RegistrationBlockStatus>
copy_qmi_blk_status_to_ims_registration_block_status
(
    imsa_ims_block_status_ind_msg_v01 *src
)
{
  auto status = std::make_shared<qcril::interfaces::RegistrationBlockStatus>();
  if (NULL != src && status != nullptr)
  {
    //Wlan Failures
    if(src->ims_failure_on_wlan_valid)
    {
      auto wlanstatus = qcril_qmi_imsa_map_imsa_block_status_to_ims_block_status(
              &src->ims_failure_on_wlan);
      if (wlanstatus != nullptr) {
        status->setBlockStatusOnWlan(wlanstatus);
      }
    }

    //Wwan Failures
    if(src->ims_failure_on_wwan_valid)
    {
      auto wwanstatus = qcril_qmi_imsa_map_imsa_block_status_to_ims_block_status(
              &src->ims_failure_on_wwan);
      if (wwanstatus != nullptr) {
        status->setBlockStatusOnWwan(wwanstatus);
      }
    }
  }
  else
  {
    QCRIL_LOG_ERROR("ims_error: Empty indication for block status!");
  }
  return status;
}

//=========================================================================
//  FUNCTION:   qcril_qmi_imsa_map_imsa_block_status_to_ims_block_status
//=========================================================================
std::shared_ptr<qcril::interfaces::BlockStatus>
qcril_qmi_imsa_map_imsa_block_status_to_ims_block_status
(
    imsa_ims_failure_reason_type_v01* imsa_block_status
)
{
  auto status = std::make_shared<qcril::interfaces::BlockStatus>();
  if (status)  {
     status->setBlockReason(qcril_qmi_ims_map_qmi_block_reason_to_ims_block_reason(
        imsa_block_status->block_reason_mask));

    auto reason = std::make_shared<qcril::interfaces::BlockReasonDetails>();
    switch(imsa_block_status->block_reason_mask)
    {
      case IMSA_BLOCK_REASON_PDP_FAILURE_BIT_MASK_V01:
        reason->setRegFailureReasonType(qcril_qmi_call_end_type_to_ims_reg_failure(
            imsa_block_status->pdp_failure_reason.call_end_reason_type));
        reason->setRegFailureReason(imsa_block_status->pdp_failure_reason.call_end_reason);
        status->setBlockReasonDetails(reason);
        break;
      case IMSA_BLOCK_REASON_REGISTRATION_FAILURE_BIT_MASK_V01:
        reason->setRegFailureReasonType(qcril_qmi_call_end_type_to_ims_reg_failure(
            imsa_block_status->pdp_failure_reason.call_end_reason_type));
        reason->setRegFailureReason(imsa_block_status->registration_failure_reason);
        status->setBlockReasonDetails(reason);
      break;
      case IMSA_BLOCK_REASON_HANDOVER_FAILURE_BIT_MASK_V01:
        reason->setRegFailureReasonType(qcril_qmi_call_end_type_to_ims_reg_failure(
            imsa_block_status->handover_failure_reason.call_end_reason_type));
        reason->setRegFailureReason(imsa_block_status->handover_failure_reason.call_end_reason);
        status->setBlockReasonDetails(reason);
      break;
      default:
        QCRIL_LOG_INFO("ims_error: Unknwon  registration block status mask: %d",
              imsa_block_status->block_reason_mask);
      break;
    }
  }
  return status;
}
