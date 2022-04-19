/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_globals.c
  @brief   RIL UTF global variables

  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#include "ril_utf_core.h"

/*========================================================
 * Global Variables
========================================================*/
int q_empty = TRUE; // main thread Q for cbs

/*
   access controlled through "exec_q_access" mutex
*/
boolean reset_now_flag = 0;
pthread_mutex_t reset_flag_mutex;
struct ril_utf_q_t *exec_list_start = NULL;
struct ril_utf_q_t *exec_list_end   = NULL;
struct ril_utf_msg_t *ril_utf_rec_head = NULL;
struct ril_utf_msg_t *ril_utf_rec_tail = NULL;
struct ril_utf_msg_t *ril_utf_inj_head = NULL;
struct ril_utf_msg_t *ril_utf_inj_tail = NULL;
pthread_mutex_t exec_q_access;
struct expect_timing_t expect_table[MAX_EXPECTS];
int inject_complete = FALSE;
int qmi_cb_thread_ready = FALSE;
int dsi_cb_thread_ready = FALSE;
int qmi_thrd_complete = FALSE;
int dsi_thrd_complete = FALSE;
pthread_cond_t signal_exptectation  = PTHREAD_COND_INITIALIZER;
pthread_cond_t inject_complete_sig = PTHREAD_COND_INITIALIZER;
pthread_cond_t expect_ready_sig = PTHREAD_COND_INITIALIZER;

pthread_t dsit_id;
pthread_t qmit_id;

qmi_client_error_cb qmi_error_cb[QCRIL_QMI_CLIENT_MAX];
qmi_client_notify_cb qmi_notify_cb[QCRIL_QMI_CLIENT_MAX];
int qmi_start = FALSE;
struct ril_utf_qmi_resp_pending_q_t *qmi_resp_pending_list_start = NULL;
struct ril_utf_qmi_resp_pending_q_t *qmi_resp_pending_list_end   = NULL;
pthread_cond_t wakeup_qmi_cb_thread = PTHREAD_COND_INITIALIZER;
pthread_cond_t dsi_thrd_complete_sig = PTHREAD_COND_INITIALIZER;
pthread_cond_t wakeup_qmi_sync_req  = PTHREAD_COND_INITIALIZER;
pthread_cond_t qmi_thrd_complete_sig = PTHREAD_COND_INITIALIZER;
pthread_cond_t wakeup_dsi_cb_thread = PTHREAD_COND_INITIALIZER;
struct sig_expect_pool_t wakeup_qmi_sync_signals[MAX_SIGNALS];
struct sig_expect_pool_t wakeup_dsi_sync_signals[MAX_SIGNALS];

pthread_t rild_id;
#ifndef RIL_UTF_IOE
RIL_RadioFunctions qmi_ril_funcs;
#endif
pthread_t mt_id;
pthread_t callback_thread;
pthread_cond_t main_thread_q_ready = PTHREAD_COND_INITIALIZER;
pthread_mutex_t main_thread_q_access;
pthread_mutex_t main_thread_ril_q_access;
pthread_cond_t callback_thread_request = PTHREAD_COND_INITIALIZER;

#ifdef RIL_UTF_IOE
pthread_mutex_t ioe_table_access;
#endif

struct timed_cb_t *timed_cb_list_head = NULL;
struct timed_cb_t *timed_cb_list_tail = NULL;

FILE* utf_log_file;
ril_utf_thread_name_info_type ril_utf_thread_name_info[RIL_UTF_THREAD_INFO_MAX_SIZE];
pthread_mutex_t ril_utf_log_lock_mutex;
char ril_utf_thread_name[ RIL_UTF_THREAD_NAME_MAX_SIZE ];

pthread_t ims_recv_id;
pthread_cond_t wakeup_ims_recv_thread = PTHREAD_COND_INITIALIZER;
int ims_expect_ready   = FALSE;

pthread_t msim_recv_id;
pthread_cond_t wakeup_msim_recv_thread = PTHREAD_COND_INITIALIZER;
int msim_expect_ready   = FALSE;

pthread_t qmi_proxy_id;
pthread_t proxy_send_id;
pthread_t proxy_recv_id;
pthread_t ioe_client_send_id;
pthread_t ioe_client_recv_id;
pthread_cond_t wakeup_proxy_send_thread = PTHREAD_COND_INITIALIZER;
pthread_cond_t wakeup_proxy_recv_thread = PTHREAD_COND_INITIALIZER;
pthread_cond_t wakeup_ioe_client_send_thread = PTHREAD_COND_INITIALIZER;
pthread_cond_t wakeup_ioe_client_recv_thread = PTHREAD_COND_INITIALIZER;
int proxy_inject_pending = FALSE;
int proxy_expect_ready   = FALSE;
int ioe_client_inject_pending = FALSE;
int ioe_client_expect_ready   = FALSE;

pthread_t qmi_oem_id;
pthread_t oem_send_id;
pthread_t oem_recv_id;
pthread_cond_t wakeup_oem_send_thread = PTHREAD_COND_INITIALIZER;
pthread_cond_t wakeup_oem_recv_thread = PTHREAD_COND_INITIALIZER;
int oem_inject_pending = FALSE;
int oem_expect_ready   = FALSE;

pthread_t qmi_rild_id;
pthread_t rild_recv_id;
pthread_cond_t wakeup_rild_recv_thread = PTHREAD_COND_INITIALIZER;
int rild_expect_ready   = FALSE;
struct ril_request_q_t ril_request_q = {NULL, NULL};

int global_timeout_max = 5; //seconds
int suspend_timeouts = TRUE;

int GEN_CALL_FLOW = FALSE;
#ifdef RIL_UTF_IOE
// QCSI specific changes
struct ril_utf_csi_data_t IoE_servers[MAX_IOE_SERVERS];
struct ril_utf_req_handle_table_t req_table[MAX_IOE_REQUESTS];
int ioe_srv_sock_id = UTF_IOE_SERVER_SOCKET_ID;
pthread_t ioe_srv_thrd_id;
#endif

int shutdown_after_execution = 1;
int is_boot_up = 0;
//===============================================================
// QMI globals
//===============================================================
/*
  Below array referenced based on the client id returned by QMI service obj
  example: NAS = 3, Voice = 9
  Note: There are no conflict for services like IMS or IMS VT right now, but may
  arise in the future
*/
struct qmi_client_init_t ril_utf_qmi_client_list[QCRIL_UNITTEST_MAX_QMI_CLIENT_ID][QCRIL_UNITTEST_MAX_QMI_CLIENTS]; // used in qmi_client_init
qmi_sys_event_client_data_type qmi_client_info; //used in qmi_init
