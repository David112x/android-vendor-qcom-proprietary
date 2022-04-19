/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_globals.h
  @brief   RIL UTF global variables

  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef _RIL_UTF_GLOBALS_H
#define _RIL_UTF_GLOBALS_H

#include "ril_utf_common_includes.h"

extern int q_empty = TRUE; // main thread Q for cbs

/*
   access controlled through "exec_q_access" mutex
*/
extern struct ril_utf_q_t *exec_list_start = NULL;
extern struct ril_utf_q_t *exec_list_end   = NULL;
extern struct ril_utf_msg_t *ril_utf_rec_head = NULL;
extern struct ril_utf_msg_t *ril_utf_rec_tail = NULL;
extern struct ril_utf_msg_t *ril_utf_inj_head = NULL;
extern struct ril_utf_msg_t *ril_utf_inj_tail = NULL;

extern pthread_mutex_t exec_q_access;
extern struct expect_timing_t expect_table[MAX_EXPECTS];
extern int inject_complete;
extern pthread_cond_t signal_exptectation  = PTHREAD_COND_INITIALIZER;
extern pthread_cond_t inject_complete_sig = PTHREAD_COND_INITIALIZER;
extern pthread_cond_t expect_ready_sig = PTHREAD_COND_INITIALIZER;

extern pthread_t qmit_id;
extern pthread_t dsit_id;

extern int qmi_start;

extern struct ril_utf_qmi_resp_pending_q_t *qmi_resp_pending_list_start = NULL;
extern struct ril_utf_qmi_resp_pending_q_t *qmi_resp_pending_list_end   = NULL;
extern struct ril_request_q_t ril_request_q = {NULL, NULL};
extern pthread_cond_t wakeup_qmi_cb_thread = PTHREAD_COND_INITIALIZER;
extern pthread_cond_t wakeup_qmi_sync_req  = PTHREAD_COND_INITIALIZER;
extern pthread_cond_t qmi_thrd_complete_sig = PTHREAD_COND_INITIALIZER;
extern pthread_cond_t dsi_thrd_complete_sig;
extern struct sig_expect_pool_t wakeup_qmi_sync_signals[MAX_SIGNALS];
extern struct sig_expect_pool_t wakeup_dsi_sync_signals[MAX_SIGNALS];

extern pthread_t rild_id;
extern pthread_t ims_recv_id;
extern pthread_cond_t wakeup_ims_recv_thread = PTHREAD_COND_INITIALIZER;
extern int ims_expect_ready   = FALSE;
extern RIL_RadioFunctions qmi_ril_funcs;
extern pthread_t mt_id;
extern pthread_cond_t main_thread_q_ready = PTHREAD_COND_INITIALIZER;
extern pthread_cond_t callback_thread_request = PTHREAD_COND_INITIALIZER;
extern pthread_mutex_t main_thread_q_access;
extern pthread_mutex_t main_thread_ril_q_access;

extern pthread_cond_t wakeup_dsi_cb_thread = PTHREAD_COND_INITIALIZER;
extern int dsi_cb_thread_ready = FALSE;

#ifdef RIL_UTF_IOE
extern pthread_mutex_t ioe_table_access;
#endif

#ifndef RIL_UTF_IOE
extern struct timed_cb_t *timed_cb_list_head = NULL;
extern struct timed_cb_t *timed_cb_list_tail = NULL;
#endif

extern int GEN_CALL_FLOW;

//===============================================================
// QMI globals
//===============================================================
/*
  Below array referenced based on the client id returned by QMI service obj
  example: NAS = 3, Voice = 9
  Note: There are no conflict for services like IMS or IMS VT right now, but may
  arise in the future
*/
extern struct qmi_client_init_t ril_utf_qmi_client_list[QCRIL_UNITTEST_MAX_QMI_CLIENT_ID][QCRIL_UNITTEST_MAX_QMI_CLIENTS]; // used in qmi_client_init
extern qmi_sys_event_client_data_type qmi_client_info; //used in qmi_init
#endif //_RIL_UTF_GLOBALS_H
