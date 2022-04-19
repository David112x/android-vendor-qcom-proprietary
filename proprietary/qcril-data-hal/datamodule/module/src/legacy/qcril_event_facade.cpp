/**
* Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/


#include "legacy/qcril_data.h"
#include "legacy/qcril_event_facade.h"
#include "DataModuleMutex.h"
#include "framework/Dispatcher.h"
#include "qcrili.h"
#include "UnSolMessages/SetPrefDataTechMessage.h"
#include "UnSolMessages/DsdSysStatusMessage.h"
#include "UnSolMessages/DataRegistrationStateMessage.h"
#include "UnSolMessages/DataSysStatusMessage.h"
#include "UnSolMessages/CallStatusMessage.h"
#include "UnSolMessages/NewDDSInfoMessage.h"
#include "UnSolMessages/DDSStatusFollowupMessage.h"
#include "UnSolMessages/DataAllBearerTypeChangedMessage.h"
#include "UnSolMessages/DataBearerTypeChangedMessage.h"
#include "RilRequestEmbmsActivateTmgiMessage.h"
#include "RilRequestEmbmsDeactivateTmgiMessage.h"
#include "RilRequestEmbmsActivateDeactivateTmgiMessage.h"
#include "RilRequestEmbmsGetAvailTmgiMessage.h"
#include "RilRequestEmbmsGetActiveTmgiMessage.h"
#include "RilRequestEmbmsContentDescUpdateMessage.h"
#include "RilRequestEmbmsSendIntTmgiListMessage.h"

#include "modules/nas/NasEmbmsDisableDataMessage.h"
#include "modules/nas/NasEmbmsEnableDataConMessage.h"

#include "request/LinkCapIndMessage.h"

#define TAG "DataModule"

using namespace rildata;

DDSSubIdInfo currentDDSSUB = { QCRIL_INVALID_MODEM_STACK_ID, DSD_DDS_DURATION_PERMANANT_V01 };
pthread_mutex_t ddsSubMutex = PTHREAD_MUTEX_INITIALIZER;
QCRIL_DEFINE_LEGACY_EVENT(DATA_COMMAND_CALLBACK, qcril_data_command_hdlr, &QCRIL_GRP_DATA, QMI_RIL_ALLOWED_IN_ALL_STATES);
QCRIL_DEFINE_LEGACY_EVENT(DATA_EVENT_CALLBACK, qcril_data_event_hdlr, &QCRIL_GRP_DATA, QMI_RIL_ALLOWED_IN_ALL_STATES);
QCRIL_DEFINE_LEGACY_EVENT(DATA_ASYNC_CALLBACK, qcril_data_async_hdlr, &QCRIL_GRP_DATA, QMI_RIL_ALLOWED_IN_ALL_STATES);
QCRIL_DEFINE_LEGACY_EVENT(DATA_SET_RAT_PREF_CALLBACK, qcril_data_set_rat_pref_hdlr, &QCRIL_GRP_DATA, QMI_RIL_ALLOWED_IN_ALL_STATES);
QCRIL_DEFINE_LEGACY_EVENT(DATA_LQE_EVENT_CALLBACK, qcril_data_lqe_event_hdlr, &QCRIL_GRP_DATA, QMI_RIL_ALLOWED_IN_ALL_STATES);
QCRIL_DEFINE_LEGACY_EVENT(DATA_WDS_EVENT_CALLBACK, qcril_data_wds_event_hdlr, &QCRIL_GRP_DATA, QMI_RIL_ALLOWED_IN_ALL_STATES);
QCRIL_DEFINE_LEGACY_EVENT(DATA_DSD_EVENT_CALLBACK, qcril_data_dsd_event_hdlr, &QCRIL_GRP_DATA, QMI_RIL_ALLOWED_IN_ALL_STATES);
QCRIL_DEFINE_LEGACY_EVENT(DATA_WDS_SUSPEND_CALLBACK, qcril_data_qmi_wds_suspend_hdlr, &QCRIL_GRP_DATA, QMI_RIL_ALLOWED_IN_ALL_STATES);
QCRIL_DEFINE_LEGACY_EVENT(DATA_WDS_RESUME_CALLBACK, qcril_data_qmi_wds_resume_hdlr, &QCRIL_GRP_DATA, QMI_RIL_ALLOWED_IN_ALL_STATES);
QCRIL_DEFINE_LEGACY_EVENT(DATA_DSD_SUSPEND_CALLBACK, qcril_data_qmi_dsd_suspend_hdlr, &QCRIL_GRP_DATA, QMI_RIL_ALLOWED_IN_ALL_STATES);
QCRIL_DEFINE_LEGACY_EVENT(DATA_DSD_RESUME_CALLBACK, qcril_data_qmi_dsd_resume_hdlr, &QCRIL_GRP_DATA, QMI_RIL_ALLOWED_IN_ALL_STATES);
QCRIL_DEFINE_LEGACY_EVENT(DATA_KEEPALIVE_EVENT_CALLBACK, qcril_data_keepalive_event_hdlr, &QCRIL_GRP_DATA, QMI_RIL_ALLOWED_IN_ALL_STATES);

//QCRIL_DEFINE_LEGACY_EVENT(RIL_REQUEST_SETUP_DATA_CALL, NULL, &QCRIL_GRP_ANDROID_RIL_REQ, QMI_RIL_ALLOWED_IN_ALL_STATES);
QCRIL_DEFINE_LEGACY_EVENT(RIL_REQUEST_DEACTIVATE_DATA_CALL, NULL, &QCRIL_GRP_ANDROID_RIL_REQ, QMI_RIL_ALLOWED_IN_ALL_STATES);
QCRIL_DEFINE_LEGACY_EVENT(RIL_REQUEST_LAST_DATA_CALL_FAIL_CAUSE, NULL, &QCRIL_GRP_ANDROID_RIL_REQ, QMI_RIL_ALLOWED_IN_ALL_STATES);
QCRIL_DEFINE_LEGACY_EVENT(RIL_REQUEST_DATA_CALL_LIST, NULL, &QCRIL_GRP_ANDROID_RIL_REQ, QMI_RIL_ALLOWED_IN_ALL_STATES);
QCRIL_DEFINE_LEGACY_EVENT(RIL_REQUEST_GET_DATA_CALL_PROFILE, NULL, &QCRIL_GRP_ANDROID_RIL_REQ, QMI_RIL_ALLOWED_IN_ALL_STATES);
QCRIL_DEFINE_LEGACY_EVENT(RIL_REQUEST_SET_DATA_PROFILE, NULL, &QCRIL_GRP_ANDROID_RIL_REQ, QMI_RIL_ALLOWED_IN_ALL_STATES);
QCRIL_DEFINE_LEGACY_EVENT(RIL_REQUEST_START_LCE, NULL, &QCRIL_GRP_ANDROID_RIL_REQ, QMI_RIL_ALLOWED_IN_ALL_STATES);
QCRIL_DEFINE_LEGACY_EVENT(RIL_REQUEST_STOP_LCE, NULL, &QCRIL_GRP_ANDROID_RIL_REQ, QMI_RIL_ALLOWED_IN_ALL_STATES);
QCRIL_DEFINE_LEGACY_EVENT(RIL_REQUEST_PULL_LCEDATA, NULL, &QCRIL_GRP_ANDROID_RIL_REQ, QMI_RIL_ALLOWED_IN_ALL_STATES);
QCRIL_DEFINE_LEGACY_EVENT(RIL_REQUEST_START_KEEPALIVE, NULL, &QCRIL_GRP_ANDROID_RIL_REQ, QMI_RIL_ALLOWED_IN_ALL_STATES);
QCRIL_DEFINE_LEGACY_EVENT(RIL_REQUEST_STOP_KEEPALIVE, NULL, &QCRIL_GRP_ANDROID_RIL_REQ, QMI_RIL_ALLOWED_IN_ALL_STATES);
QCRIL_DEFINE_LEGACY_EVENT(RIL_REQUEST_SET_CARRIER_INFO_IMSI_ENCRYPTION, NULL, &QCRIL_GRP_ANDROID_RIL_REQ, QMI_RIL_ALLOWED_IN_ALL_STATES);

#ifdef QMI_RIL_UTF
QCRIL_DEFINE_LEGACY_EVENT(HOOK_EMBMS_ACTIVATE_TMGI                  , qcril_data_embms_activate_tmgi       , &QCRIL_GRP_OEM_HOOK , QMI_RIL_ALLOWED_IN_ALL_STATES);
QCRIL_DEFINE_LEGACY_EVENT(HOOK_EMBMS_DEACTIVATE_TMGI                , qcril_data_embms_deactivate_tmgi    , &QCRIL_GRP_OEM_HOOK , QMI_RIL_ALLOWED_IN_ALL_STATES);
QCRIL_DEFINE_LEGACY_EVENT(HOOK_EMBMS_ACTIVATE_DEACTIVATE_TMGI       , qcril_data_embms_activate_deactivate_tmgi, &QCRIL_GRP_OEM_HOOK , QMI_RIL_ALLOWED_IN_ALL_STATES);
#endif

/*=========================================================================
  FUNCTION:  qcril_data_set_pref_data_tech

===========================================================================*/
/*!
    @brief
    Set the preferred data technology used for data call setup.

    @return
    None
*/
/*=========================================================================*/
void qcril_data_set_pref_data_tech
(
  qcril_instance_id_e_type        instance_id,
  qcril_data_pref_data_tech_e_type pref_data_tech
)
{
   auto msg = std::make_shared<rildata::SetPrefDataTechMessage>
                                     (instance_id, pref_data_tech);
   msg->broadcast();
}

/*=========================================================================
  FUNCTION:  qcril_data_set_dsd_sys_status

===========================================================================*/
/*!
    @brief
    Set the preferred data technology used for data call setup. This is
    called upon receipt of
    QMI_DSD_REPORT_SYSTEM_STATUS_IND_V01/QMI_DSD_GET_SYSTEM_STATUS_RESP_V01.

    @return
    None
*/
/*=========================================================================*/
void qcril_data_set_dsd_sys_status
(
  dsd_system_status_ind_msg_v01 *dsd_system_status
)
{
  if (dsd_system_status != nullptr && dsd_system_status->avail_sys_len >= 1)
  {
    //For Non-NR RAT, DsdSysStatusMessage will be used.
    if (dsd_system_status->avail_sys[0].rat_value != DSD_SYS_RAT_EX_3GPP_5G_V01)
    {
      auto msg = std::make_shared<rildata::DsdSysStatusMessage>
          (global_instance_id, *dsd_system_status);
      msg->broadcast();
    }
    else //For NR RAT, DataRegistrationStateMessage will be used.
    {
      //For NSA Mode
      if (dsd_system_status->avail_sys[0].so_mask & QMI_DSD_3GPP_SO_MASK_5G_NSA_V01)
      {
        if (dsd_system_status->avail_sys[0].so_mask & QMI_DSD_3GPP_SO_MASK_5G_MMWAVE_V01)
        {
          auto msg = std::make_shared<rildata::DataRegistrationStateMessage>
              (global_instance_id, NR_NSA_MMWV);
          msg->broadcast();
        }
        //NR_NSA is the default value for NSA Mode,
        //even if mask QMI_DSD_3GPP_SO_MASK_5G_SUB6_V01 is not set.
        else
        {
          auto msg = std::make_shared<rildata::DataRegistrationStateMessage>
              (global_instance_id, NR_NSA);
          msg->broadcast();
        }
      }
      //For SA Mode
      else if (dsd_system_status->avail_sys[0].so_mask & QMI_DSD_3GPP_SO_MASK_5G_SA_V01)
      {
        if (dsd_system_status->avail_sys[0].so_mask & QMI_DSD_3GPP_SO_MASK_5G_MMWAVE_V01)
        {
          auto msg = std::make_shared<rildata::DataRegistrationStateMessage>
              (global_instance_id, NR_SA_MMWV);
          msg->broadcast();
        }
        //NR_SA is the default value for SA Mode,
        //Even if mask QMI_DSD_3GPP_SO_MASK_5G_SUB6_V01 is not set.
        else
        {
          auto msg = std::make_shared<rildata::DataRegistrationStateMessage>
              (global_instance_id, NR_SA);
          msg->broadcast();
        }
      }
      else
      {
        QCRIL_LOG_ERROR("Invalid so_mask for 5G RAT %llu. so_mask should contain either"
                        "5G_NSA or 5G_SA", dsd_system_status->avail_sys[0].so_mask);
      }
    }
  }
  else
  {
    QCRIL_LOG_ERROR("dsd_system_status is NULL or no available system.");
  }
}

/*=========================================================================
  FUNCTION:  qcril_data_set_data_sys_status

===========================================================================*/
/*!
    @brief
    Set the preferred data technology used for data call setup. This is
    newer version of qcril_arb_set_pref_data_tech, we pass over all info
    returned from modem to upper layer

    @return
    None
*/
/*=========================================================================*/
void qcril_data_set_data_sys_status
(
  qcril_instance_id_e_type          instance_id,
  wds_data_system_status_type_v01 * data_sys_status
)
{
  if (data_sys_status != nullptr)
  {
    auto msg = std::make_shared<rildata::DataSysStatusMessage>
                           (instance_id, *data_sys_status);
    msg->broadcast();
  }
}

/*===========================================================================

  FUNCTION:  qcril_data_client_notify

===========================================================================*/
/*!
    @brief
    Notify data call status

    @return
    None
*/
/*=========================================================================*/
#define CALL_ID_INVALID (-1)
void qcril_data_client_notify
(
  qcril_data_net_evt_t      evt,
  int cid
)
{
  unsigned int num_calls = 0;
  int i;

  QCRIL_LOG_VERBOSE("%s : ENTRY", __func__ );

  QCRIL_DATA_MUTEX_LOCK( INFO_TBL_MUTEX );
  for (i = 0; i < MAX_CONCURRENT_UMTS_DATA_CALLS; i++)
  {
    if (CALL_ID_INVALID != info_tbl[i].cid)
    {
      num_calls++;
    }
  }
  QCRIL_DATA_MUTEX_UNLOCK( INFO_TBL_MUTEX );

  QCRIL_LOG_INFO( "Notify cid [%d], ev [%d] and [%d] call records", cid, evt, num_calls);

  auto msg = std::make_shared<rildata::CallStatusMessage>
                         (global_instance_id, cid, evt, num_calls);
  msg->broadcast();

  QCRIL_LOG_VERBOSE("%s : EXIT", __func__ );
}

/*===========================================================================

  FUNCTION:  qcril_data_new_dds_info

===========================================================================*/
/*!
    @brief
    Notify new DDS information

    @return
    None
*/
/*=========================================================================*/
void qcril_data_new_dds_info
(
  DDSSubIdInfo sub_info
)
{
  LOCK_MUTEX(ddsSubMutex);
  currentDDSSUB.dds_sub_id = sub_info.dds_sub_id;
  currentDDSSUB.switch_type = sub_info.switch_type;
  UNLOCK_MUTEX(ddsSubMutex);
  QCRIL_LOG_INFO( "qcril_data_new_dds_info:DDS switched to SUB = [%d], switch_type = [%d]",
                  currentDDSSUB.dds_sub_id, currentDDSSUB.switch_type);

  auto msg = std::make_shared<rildata::NewDDSInfoMessage>
                                (global_instance_id, sub_info);
  msg->broadcast();
}

/*===========================================================================

  FUNCTION:  qcril_data_dds_status_followup

===========================================================================*/
/*!
    @brief
    Notify DDS switch followup status

    @return
    None
*/
/*=========================================================================*/
void qcril_data_dds_status_followup
(
  DDSStatusInfo * status
)
{
  if (status != nullptr)
  {
    auto msg = std::make_shared<rildata::DDSStatusFollowupMessage>
                                         (global_instance_id, *status);
    msg->broadcast();
  }
}

/*===========================================================================

  FUNCTION:  qcril_data_embms_enable_data_con

===========================================================================*/
/*!
    @brief
    Notify embms enable data response

    @return
    None
*/
/*=========================================================================*/
void qcril_data_embms_enable_data_con
(
  qcril_embms_enable_response_payload_type *payload
)
{
  auto msg = std::make_shared<NasEmbmsEnableDataConMessage>(*payload);
  msg->dispatch();
}

/*===========================================================================

  FUNCTION:  qcril_data_embms_disable_data_ind

===========================================================================*/
/*!
    @brief
    Notify embms disable indication

    @return
    None
*/
/*=========================================================================*/
void qcril_data_embms_disable_data_ind
(
  qcril_embms_disable_indication_payload_type *payload
)
{
  auto msg = std::make_shared<NasEmbmsDisableDataMessage>(*payload);
  msg->dispatch();
}

void qcril_data_lce_report_ind
(uint32_t uplink_capacity, uint32_t downlink_capacity) {
  LinkCapEstimate_t capacity = {downlink_capacity, uplink_capacity};
  auto msg = std::make_shared<LinkCapIndMessage>(capacity);
  msg->broadcast();
}

/*===========================================================================

  FUNCTION:  qcril_data_bearer_type_info_ind_update

===========================================================================*/
/*!
    @brief
    Update a bearer type info using qmi indication

    @return
    None
*/
/*=========================================================================*/
void qcril_data_bearer_type_info_ind_update
(
  int32_t cid,
  wds_data_bearer_type_info_v01 *bearer_type_info
)
{
  // Adding one bearer with BearerInfo_t
  BearerInfo_t b;
  b.bearerId = (int32_t)bearer_type_info->bearer_id;
  b.uplink = (RatType_t)bearer_type_info->ul_direction;
  b.downlink = (RatType_t)bearer_type_info->dl_direction;

  auto msg = std::make_shared<DataBearerTypeChangedMessage>(cid, b);
  msg->broadcast();
}


void activate_embms_request_msg(qcril_request_params_type const *request)
{
  QCRIL_LOG_DEBUG( "%s", "activate_embms_request_msg: ENTRY" );

  if(request != nullptr )
  {
    auto msg = std::make_shared<rildata::RilRequestEmbmsActivateTmgiMessage>(*request);
    msg->dispatch();
  }
}

void deactivate_embms_request_msg(qcril_request_params_type const *request)
{
  QCRIL_LOG_DEBUG( "%s", "deactivate_embms_request_msg: ENTRY" );
  if(request != nullptr)
  {
    auto msg = std::make_shared<rildata::RilRequestEmbmsDeactivateTmgiMessage>(*request);
    msg->dispatch();
  }
}

void activate_deactivate_embms_request_msg(qcril_request_params_type const *request)
{
  QCRIL_LOG_DEBUG( "%s", "activate_deactivate_embms_request_msg: ENTRY" );
  if(request != nullptr)
  {
    auto msg = std::make_shared<rildata::RilRequestEmbmsActivateDeactivateTmgiMessage>(*request);
    msg->dispatch();
  }
}

void embms_get_available_tmgi_msg(qcril_request_params_type const *request)
{
  QCRIL_LOG_DEBUG( "%s", "embms_get_available_tmgi_msg: ENTRY" );
  if(request != nullptr)
  {
    auto msg = std::make_shared<rildata::RilRequestEmbmsGetAvailTmgiMessage>(*request);
    msg->dispatch();
  }
}

void embms_get_active_tmgi_msg(qcril_request_params_type const *request)
{
  QCRIL_LOG_DEBUG( "%s", "embms_get_active_tmgi_msg: ENTRY" );
  if(request != nullptr)
  {
    auto msg = std::make_shared<rildata::RilRequestEmbmsGetActiveTmgiMessage>(*request);
    msg->dispatch();
  }
}

void embms_content_desc_update_msg(qcril_request_params_type const *request)
{
  QCRIL_LOG_DEBUG( "%s", "embms_content_desc_update_msg: ENTRY" );
  if(request != nullptr)
  {
    auto msg = std::make_shared<rildata::RilRequestEmbmsContentDescUpdateMessage>(*request);
    msg->dispatch();
  }
}

void embms_send_interested_list(qcril_request_params_type const *request)
{
  QCRIL_LOG_DEBUG( "%s", "embms_send_interested_list: ENTRY" );
  if(request != nullptr)
  {
    auto msg = std::make_shared<rildata::RilRequestEmbmsSendIntTmgiListMessage>(*request);
    msg->dispatch();
  }
}
/*===========================================================================

  FUNCTION:  qcril_data_bearer_type_info_response_update

===========================================================================*/
/*!
    @brief
    Update bearer type info using qmi response

    @return
    None
*/
/*=========================================================================*/
void qcril_data_bearer_type_info_response_update
(
  int32_t cid,
  char * apnname,
  char * ifacename,
  wds_get_data_bearer_type_resp_msg_v01 *resp
)
{
  AllocatedBearer_t bearerInfo;
  bearerInfo.cid = cid;
  bearerInfo.apn.assign(apnname);
  bearerInfo.ifaceName.assign(ifacename);
  // Adding bearers with BearerInfo_t
  if(resp->bearer_type_info_valid) {
    for(uint32_t i=0;i<resp->bearer_type_info_len;i++) {
      BearerInfo_t b;
      b.bearerId = (int32_t)resp->bearer_type_info[i].bearer_id;
      b.uplink = (RatType_t)resp->bearer_type_info[i].ul_direction;
      b.downlink = (RatType_t)resp->bearer_type_info[i].dl_direction;
      bearerInfo.bearers.push_back(b);
    }
    auto msg = std::make_shared<DataAllBearerTypeChangedMessage>(bearerInfo);
    msg->broadcast();
  }
}
