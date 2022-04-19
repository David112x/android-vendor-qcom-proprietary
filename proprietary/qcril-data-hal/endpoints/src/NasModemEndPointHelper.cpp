/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include "NasModemEndPointHelper.h"
#include "modules/nas/NasSetPsAttachDetachMessage.h"
#include "qcril_reqlist.h"
#include "modules/nas/NasGetSysInfoMessage.h"

#include <pthread.h>

extern DDSSubIdInfo currentDDSSUB;
extern pthread_mutex_t ddsSubMutex;
timer_t NasModemEndPointHelper::set_detach_or_attach_timer_id = NULL;
const uint8_t  NasModemEndPointHelper::QCRIL_DATA_DETACH_ATTACH_IND_TIMEOUT = 77;
#define QCRIL_DATA_INVALID_TIMER_ID 0
pthread_mutex_t NasModemEndPointHelper::nasPSStateMutex = PTHREAD_MUTEX_INITIALIZER;
DataNasPsAttachDetachCallback* NasModemEndPointHelper::nasAttachDetachCb = NULL;
NasModemEndPointHelper::NasDomainPref NasModemEndPointHelper::currentState = NasModemEndPointHelper::NasDomainPref::QCRIL_NAS_IA_NONE;
void sendRequestResp(qcril_request_resp_params_type *resp_params, RIL_Errno e);
/*===========================================================================

  FUNCTION:  handleDetachRequest

===========================================================================*/
/*!
    @brief
    API to trigger detach through RIL common Nas module.

    @return
    None.
*/
/*=========================================================================*/
void NasModemEndPointHelper::handleDetachRequest(RIL_Token t, qcril_evt_e_type event_id)
{
  if( nasAttachDetachCb == NULL ) {
    nasAttachDetachCb = new DataNasPsAttachDetachCallback("set-cb-token");
  }
  auto msg = std::make_shared<NasSetPsAttachDetachMessage>(NasSetPsAttachDetachMessage::PsAttachAction::PS_DETACH, nasAttachDetachCb);

  LOCK_MUTEX(nasPSStateMutex);
  currentState = NasDomainPref::QCRIL_NAS_IA_DETACH_IN_PROGRESS;
  UNLOCK_MUTEX(nasPSStateMutex);

  msg->dispatch();
  Log::getInstance().d("[NasModemEndPointHelper] ::Detach triggered through NasModule.");
  qcril_data_start_detach_or_attach_ind_timer( t, event_id );
}

/*===========================================================================

  FUNCTION:  handleAttachRequest

===========================================================================*/
/*!
    @brief
    API to clear trigger attach through RIL common Nas module

    @return
    None.
*/
/*=========================================================================*/
void NasModemEndPointHelper::handleAttachRequest(RIL_Token t, qcril_evt_e_type event_id)
{
  if( nasAttachDetachCb == NULL ) {
    nasAttachDetachCb = new DataNasPsAttachDetachCallback("set-cb-token");
  }
  auto msg = std::make_shared<NasSetPsAttachDetachMessage>(NasSetPsAttachDetachMessage::PsAttachAction::PS_ATTACH, nasAttachDetachCb);

  LOCK_MUTEX(nasPSStateMutex);
  currentState = NasDomainPref::QCRIL_NAS_IA_ATTACH_IN_PROGRESS;
  UNLOCK_MUTEX(nasPSStateMutex);

  msg->dispatch();
  Log::getInstance().d("[NasModemEndPointHelper] ::Attach triggered through NasModule.");
  qcril_data_start_detach_or_attach_ind_timer(t, event_id);
}

/*===========================================================================

  FUNCTION:  qcril_data_stop_detach_or_attach_ind_timer

===========================================================================*/
/*!
    @brief
    API to clear  set_lte_attach_list_ind timer

    @return
    None.
*/
/*=========================================================================*/
void NasModemEndPointHelper::qcril_data_stop_detach_or_attach_ind_timer()
{
  Log::getInstance().d("[NasModemEndPointHelper]::"
     "qcril_data_stop_detach_or_attach_ind_timer: ENTRY");
  if ( !set_detach_or_attach_timer_id )
  {
    Log::getInstance().d( "timer_id is NULL" );
    return;
  }
  if (QCRIL_DATA_INVALID_TIMER_ID == set_detach_or_attach_timer_id)
  {
    Log::getInstance().d( "invalid timer_id" );
    return;
  }
  Log::getInstance().d( "clearing timer");
  timer_delete(set_detach_or_attach_timer_id);
  set_detach_or_attach_timer_id = QCRIL_DATA_INVALID_TIMER_ID;
}

/*===========================================================================

  FUNCTION:  qcril_data_detach_or_attach_ind_timeout_hdlr

===========================================================================*/
/*!
    @brief
    Handler which gets invoked when set_lte_attach_list_ind timer expires

    @return
    None.
*/
/*=========================================================================*/
void NasModemEndPointHelper::qcril_data_detach_or_attach_ind_timeout_hdlr(union sigval sval)
{
  Log::getInstance().d("[NasModemEndPointHelper]:"
                        "qcril_data_detach_or_attach_ind_timeout_hdlr ENTRY");

  IxErrnoType                     found_qcril_request;
  qcril_reqlist_public_type       qcril_req_info;

  qcril_request_resp_params_type *req = (qcril_request_resp_params_type*)(sval.sival_ptr);

  if(req)
  {
    found_qcril_request = qcril_reqlist_query_by_event(req->instance_id,
                                                QCRIL_DEFAULT_MODEM_ID,
                                                QCRIL_EVT_QMI_REQUEST_INIT_ATTACH_APN,
                                                &qcril_req_info);

    if( found_qcril_request == E_SUCCESS )
    {
      qcril_data_stop_detach_or_attach_ind_timer();
      LOCK_MUTEX(nasPSStateMutex);
      currentState = NasDomainPref::QCRIL_NAS_IA_NONE;
      UNLOCK_MUTEX(nasPSStateMutex);
      qcril_request_resp_params_type resp;
      resp.instance_id        = req->instance_id;
      resp.t                  = req->t;
      resp.request_id         = req->request_id;
      resp.request_id_android = RIL_REQUEST_SET_INITIAL_ATTACH_APN;
      resp.ril_err_no         = RIL_E_GENERIC_FAILURE;
      resp.resp_pkt           = NULL;
      resp.resp_len           = 0;
      resp.logstr             = NULL;
      resp.rild_sock_oem_req  = 0;
      Log::getInstance().d("sending IA request failure response");
      qcril_send_request_response( &resp );
    } else
    {
      Log::getInstance().d("qcril_data_detach_or_attach_ind_timeout_hdlr"
                               ":: ERROR: Failed to Send response");
    }
  } else
  {
      Log::getInstance().d("[ProfileHandler]qcril_data_nas_detach_attach_ind_hdlr"
                    "There is no QCRIL_EVT_QMI_REQUEST_INIT_ATTACH_APN"
                     "event to handle");
  }
  if(nasAttachDetachCb)
  {
    delete nasAttachDetachCb;
    nasAttachDetachCb = NULL;
  }
}
/*===========================================================================

  FUNCTION:  qcril_data_start_detach_or_attach_ind_timer

===========================================================================*/
/*!
    @brief
    Create & start a timer for the maximum time to wait for indication from modem

    @return
    None.
*/
void NasModemEndPointHelper::qcril_data_start_detach_or_attach_ind_timer
(
  RIL_Token t, qcril_evt_e_type event_id
)
{
  struct sigevent sigev;
  struct itimerspec itimers;
  Log::getInstance().d("qcril_data_start_detach_or_attach_ind_timer Entry");

  memset(&sigev, 0, sizeof(struct sigevent));
  memset(&itimers, 0,sizeof(struct itimerspec));

  /* The timer_id should be invalid, if not log an error and delete it */
  if (QCRIL_DATA_INVALID_TIMER_ID != set_detach_or_attach_timer_id)
  {
    Log::getInstance().d( "deleting stale retry_timer_id");
    qcril_data_stop_detach_or_attach_ind_timer();
  }
  qcril_request_resp_params_type *resp_params =
        (qcril_request_resp_params_type*)malloc(sizeof(qcril_request_resp_params_type));

  if(resp_params)
  {
    memset(resp_params, 0, sizeof(qcril_request_resp_params_type));
    resp_params->instance_id = static_cast<qcril_instance_id_e_type>(global_instance_id);
    resp_params->request_id =  event_id;
    resp_params->t = t;
  }

  sigev.sigev_notify            = SIGEV_THREAD;
  sigev.sigev_notify_attributes = NULL;
  sigev.sigev_value.sival_ptr   = resp_params;
  sigev.sigev_notify_function   = qcril_data_detach_or_attach_ind_timeout_hdlr;

  if (-1 == timer_create(CLOCK_MONOTONIC, &sigev, &set_detach_or_attach_timer_id) )
  {
    Log::getInstance().d( "failed to create timer ");
  }
  else
  {
    Log::getInstance().d( "timer creation success:");
  }
  /* assign default value before query property value */
  itimers.it_value.tv_sec = QCRIL_DATA_DETACH_ATTACH_IND_TIMEOUT;
  itimers.it_value.tv_nsec    = 0;
  itimers.it_interval.tv_sec  = 0;
  itimers.it_interval.tv_nsec = 0;

  /* Start the timer */
  if (-1 == timer_settime(set_detach_or_attach_timer_id, 0, &itimers, NULL))
  {
    Log::getInstance().d( "failed to start timer for timer_id , deleting... ");
    qcril_data_stop_detach_or_attach_ind_timer();
  }
}

/*===========================================================================

  FUNCTION:  qcril_data_handle_detach_attach_ind

===========================================================================*/
/*!
    @brief
    API to handle detach & Attach indication received from NASModule in RIL
    common

    @return
    None.
*/
/*=========================================================================*/
void NasModemEndPointHelper::qcril_data_handle_detach_attach_ind
(
  qcril_request_resp_params_type &resp_params,
  uint8_t domainPrefValid,
  nas_srv_domain_pref_enum_type_v01 domainPref
)
{
  LOCK_MUTEX(nasPSStateMutex);

  if ( (NasDomainPref::QCRIL_NAS_IA_DETACH == currentState) && (((TRUE == domainPrefValid) &&
                 (QMI_SRV_DOMAIN_PREF_CS_ONLY_V01 == domainPref)) || (FALSE == domainPrefValid)) )
  {
    UNLOCK_MUTEX(nasPSStateMutex);

    Log::getInstance().d("[NasModemEndPointHelper]::Detach successful."
             "qcril_data_handle_detach_attach_ind:: Attach Triggered");

    qcril_data_stop_detach_or_attach_ind_timer();

    handleAttachRequest(resp_params.t,resp_params.request_id);
  }
  else if ( (NasDomainPref::QCRIL_NAS_IA_ATTACH == currentState) &&
             ((TRUE == domainPrefValid) &&
                 ((QMI_SRV_DOMAIN_PREF_CS_PS_V01 == domainPref)
                || (QMI_SRV_DOMAIN_PREF_PS_ONLY_V01 == domainPref)) ))
  {
    currentState = NasDomainPref::QCRIL_NAS_IA_NONE;
    UNLOCK_MUTEX(nasPSStateMutex);

    Log::getInstance().d("[NasModemEndPointHelper]::"
      "qcril_data_handle_detach_attach_ind: Attach Issued Successfully");

    qcril_data_stop_detach_or_attach_ind_timer();
    sendRequestResp(&resp_params, RIL_E_SUCCESS);

  }
  else if( NasDomainPref::QCRIL_NAS_IA_NONE == currentState )
  {
    //Received NAS IND is not corresponding to our IA request

    UNLOCK_MUTEX(nasPSStateMutex);
    Log::getInstance().d("ERROR!! [NasModemEndPointHelper]::"
      "qcril_data_handle_detach_attach_ind: NAS IND received "
      "is not for IA Request. No action needed ");
  }
  else
  {
    UNLOCK_MUTEX(nasPSStateMutex);
  }
}

/*===========================================================================

  FUNCTION:  sendRequestResp

===========================================================================*/
/*!
    @brief
    API to send response to telephony

    @return
    None.
*/
/*=========================================================================*/
void sendRequestResp(qcril_request_resp_params_type *resp_params, RIL_Errno e)
{
  Log::getInstance().d("[NasModemEndPointHelper]: Sending respons to RIL Common");
  qcril_request_resp_params_type ril_resp;
  memset(&ril_resp,0,sizeof(qcril_request_resp_params_type));
  qcril_default_request_resp_params(resp_params->instance_id,
                                    resp_params->t,
                                    resp_params->request_id,
                                    e,
                                    &ril_resp);
  qcril_send_request_response(&ril_resp);

}

/*===========================================================================

  FUNCTION:  retrieveLTEPSAttachStatus

===========================================================================*/
/*!
    @brief
    API to retrieve the current SYS info & determine if LTE PS service domain
    is available or not.

    @return
    TRUE: If LTE PS service domain is available
    FALSE: If LTE PS service domain is not available
*/
/*=========================================================================*/
boolean NasModemEndPointHelper::retrieveLTEPSAttachStatus()
{
  Log::getInstance().d("[NasModemEndPointHelper]:In retrieveLTEPSAttachStatus");

  auto msg = std::make_shared<NasGetSysInfoMessage>(nullptr);
  std::shared_ptr<nas_get_sys_info_resp_msg_v01> get_sys_info_resp_msg = nullptr;
  Message::Callback::Status apiStatus;
  apiStatus = msg->dispatchSync(get_sys_info_resp_msg);

  std::ostringstream ss;
  ss << "[NasModemEndPointHelper::retrieveLTEPSAttachStatus] status = " << (int) apiStatus;
  Log::getInstance().d(ss.str());

  if ((apiStatus != Message::Callback::Status::SUCCESS) || !get_sys_info_resp_msg)
  {
    Log::getInstance().d("[NasModemEndPointHelper] failed to get sys info or"
                         "get_sys_info_resp_msg is NULL, result="
                         + std::to_string((int) apiStatus));
    return FALSE;
  }
  if ( get_sys_info_resp_msg->lte_sys_info.common_sys_info.srv_domain_valid &&
        ((get_sys_info_resp_msg->lte_sys_info.common_sys_info.srv_domain == SYS_SRV_DOMAIN_PS_ONLY_V01) ||
         (get_sys_info_resp_msg->lte_sys_info.common_sys_info.srv_domain == SYS_SRV_DOMAIN_CS_PS_V01)))
  {
    Log::getInstance().d("[NasModemEndPointHelper] PS service domain in available");
    return TRUE;
  }
  Log::getInstance().d("[NasModemEndPointHelper] PS service domain in unavailable");
  return FALSE;
}

/*===========================================================================

  FUNCTION: processDetachAttachResp

===========================================================================*/
/*!
    @brief
    API to process response received after
    sending request -'NasSetPsAttachDetachMessage' to Nas Module

    @return
    None
*/
/*=========================================================================*/
void NasModemEndPointHelper::processDetachAttachResp(qcril_request_resp_params_type &resp_params, RIL_Errno *ret)
{
  IxErrnoType                     found_qcril_request;
  qcril_reqlist_public_type       qcril_req_info;

  if(ret == NULL)
    return;

  found_qcril_request = qcril_reqlist_query_by_event(resp_params.instance_id,
                                              QCRIL_DEFAULT_MODEM_ID,
                                              QCRIL_EVT_QMI_REQUEST_INIT_ATTACH_APN,
                                              &qcril_req_info);

  if( found_qcril_request == E_SUCCESS )
  {
    qcril_instance_id_e_type rilInstanceId = qmi_ril_get_process_instance_id();

    LOCK_MUTEX(ddsSubMutex);
    if( currentDDSSUB.dds_sub_id == QCRIL_INVALID_MODEM_STACK_ID )
    {
      UNLOCK_MUTEX(ddsSubMutex);
      Log::getInstance().d("Current DDS is invalid. Querying the current DDS from modem");
      DDSSubIdInfo ddsInfo = qcril_data_get_dds_sub_info();

      LOCK_MUTEX(ddsSubMutex);
      currentDDSSUB.dds_sub_id = ddsInfo.dds_sub_id;
      currentDDSSUB.switch_type = ddsInfo.switch_type;
      UNLOCK_MUTEX(ddsSubMutex);
      Log::getInstance().d("Current DDS is on SUB ="+std::to_string(currentDDSSUB.dds_sub_id)+
                           "with switch type="+std::to_string(currentDDSSUB.switch_type));
      LOCK_MUTEX(ddsSubMutex);
    }

    /* Check for non-DDS sub */
    if((currentDDSSUB.dds_sub_id != rilInstanceId) &&
                      (DSD_DDS_DURATION_PERMANANT_V01 == currentDDSSUB.switch_type))
    {
      UNLOCK_MUTEX(ddsSubMutex);

      /* For non DDS sub, We will not wait for PS_DETACH indication */

      Log::getInstance().d("[NasModemEndPointHelper]::processDetachAttachResp : Current DDS SUB="+std::to_string(currentDDSSUB.dds_sub_id));
      Log::getInstance().d("Since SUB= "+std::to_string(rilInstanceId) +"is non-DDS SUB, we do not wait for PS_DETACH Indication.");

      qcril_data_stop_detach_or_attach_ind_timer();

      LOCK_MUTEX(nasPSStateMutex);
      if(currentState == NasDomainPref::QCRIL_NAS_IA_DETACH_IN_PROGRESS)
      {
        currentState = NasDomainPref::QCRIL_NAS_IA_NONE;
        UNLOCK_MUTEX(nasPSStateMutex);

        sendRequestResp(&resp_params, *ret);
      }
      else
      {
        UNLOCK_MUTEX(nasPSStateMutex);
        Log::getInstance().d("ERROR!! [NasModemEndPointHelper]::processDetachAttachResp"
                    "Non-DDS SUB, Error: Improper current State seen!!!"
                    "Device not in DETACH_IN_PROGRESS State");
      }
    }
    else
    {
      /* DDS Sub Case : So we'll wait for PS_DETACH indication */
      UNLOCK_MUTEX(ddsSubMutex);
      if( *ret == RIL_E_SUCCESS )
      {
        LOCK_MUTEX(nasPSStateMutex);
        if( currentState == NasDomainPref::QCRIL_NAS_IA_DETACH_IN_PROGRESS)
        {
          currentState = NasDomainPref::QCRIL_NAS_IA_DETACH;
        }
        else if(currentState == NasDomainPref::QCRIL_NAS_IA_ATTACH_IN_PROGRESS)
        {
          currentState = NasDomainPref::QCRIL_NAS_IA_ATTACH;
        }
        UNLOCK_MUTEX(nasPSStateMutex);
      }
      else
      {
        Log::getInstance().d("[NasModemEndPointHelper]::processDetachAttachResp"
                             "Received Error Response");
        qcril_data_stop_detach_or_attach_ind_timer();

        LOCK_MUTEX(nasPSStateMutex);
        currentState =  NasDomainPref::QCRIL_NAS_IA_NONE;
        UNLOCK_MUTEX(nasPSStateMutex);

        sendRequestResp(&resp_params, *ret);
      }
    }
  }
  else
  {
    Log::getInstance().d("ERROR!! [NasModemEndPointHelper]::processDetachAttachResp"
                 "There is no QCRIL_EVT_QMI_REQUEST_INIT_ATTACH_APN request to handle");
  }
  if(nasAttachDetachCb)
  {
    delete nasAttachDetachCb;
    nasAttachDetachCb = NULL;
  }
}
