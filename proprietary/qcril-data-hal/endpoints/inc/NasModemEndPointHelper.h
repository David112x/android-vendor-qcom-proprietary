/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "module/IDataModule.h"
#include "network_access_service_v01.h"
#include "legacy/qcril_data.h"
#include "DataNasPsAttachDetachCallback.h"

#include <pthread.h>

class NasModemEndPointHelper
{
public:
   enum class NasDomainPref
   {
    QCRIL_NAS_IA_NONE = 0,
    QCRIL_NAS_IA_DETACH_IN_PROGRESS,
    QCRIL_NAS_IA_DETACH,
    QCRIL_NAS_IA_ATTACH_IN_PROGRESS,
    QCRIL_NAS_IA_ATTACH,
   };
   static NasDomainPref currentState;
   static pthread_mutex_t nasPSStateMutex;
   NasModemEndPointHelper() {}
   ~NasModemEndPointHelper() {
     if(nasAttachDetachCb)
       {
         delete nasAttachDetachCb;
         nasAttachDetachCb = NULL;
       }
   }
   static void handleDetachRequest(RIL_Token t, qcril_evt_e_type event_id);
   static void handleAttachRequest(RIL_Token t, qcril_evt_e_type event_id);
   static void qcril_data_stop_detach_or_attach_ind_timer();
   static void qcril_data_detach_or_attach_ind_timeout_hdlr(union sigval sval);
   static void qcril_data_start_detach_or_attach_ind_timer(RIL_Token t, qcril_evt_e_type event_id);
   static void qcril_data_handle_detach_attach_ind(qcril_request_resp_params_type &resp_params,
               uint8_t domainPrefValid, nas_srv_domain_pref_enum_type_v01 domainPref );
   void processDetachAttachResp(qcril_request_resp_params_type &resp_params, RIL_Errno *ret);
   void init();
   static boolean retrieveLTEPSAttachStatus();

private:
   static timer_t set_detach_or_attach_timer_id;
   static const  uint8_t QCRIL_DATA_DETACH_ATTACH_IND_TIMEOUT;
   static const uint8_t QCRIL_DATA_INVALID_TIMER_ID;
   static DataNasPsAttachDetachCallback *nasAttachDetachCb;
};
