/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_qmi_sim.h
  @brief   RIL UTF QMI simulation

  DESCRIPTION
   Simulated QMI interfaces for RIL UTF
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef _RIL_UTF_QMI_SIM_H_
#define _RIL_UTF_QMI_SIM_H_

#include "ril_utf_common_includes.h"
#include "ril_utf_defs.h"
#include "ril_utf_core.h"
#ifdef RIL_UTF_IOE
#include "qmi_csi.h"

#define QMI_CSI_MCM_CORE_SRV  "./qmi_csi_sock1"

typedef unsigned int qmi_service_instance;

typedef struct {
    uint32_t sig_set;
    uint32_t timed_out;
    pthread_cond_t cond;
    pthread_mutex_t mutex;
} qmi_cci_os_signal_type;

typedef qmi_cci_os_signal_type qmi_client_os_params;

#endif

void clear_pending_qmi_resp_q(void);
void clear_qmi_clients(void);
void remove_from_pending_qmi_resp_q( struct ril_utf_qmi_resp_pending_q_t *node);

void add_to_pending_qmi_resp_q
(
    qmi_client_type                 user_handle,
    unsigned long                   msg_id,
    void                            *resp_c_struct,
    int                             resp_c_struct_len,
    qmi_client_recv_msg_async_cb    resp_cb,
    void                            *resp_cb_data,
    enum qmi_send_msg_t_e            qmi_send_msg_type
);

void issue_qmi_response(struct ril_utf_msg_t *tmp_node);
void issue_qmi_indication(struct ril_utf_msg_t *tmp_node);
void process_qmi_callback();
void print_qmi_params
(
    qmi_client_type    user_handle,
    int                msg_id,
    void               *req_c_struct,
    int                req_c_struct_len,
    int                resp_c_struct_len,
    int                timeout_msecs
);

void *qmi_thrd_start( void *param );
int ril_utf_get_qmi_client_id(qmi_idl_service_object_type service_obj);
void enqueue_qmi_expect( void  *req_c_struct, int req_c_struct_len, unsigned long msg_id,
                     int service_id, int client_id, enum ril_utf_q_element_t type );
void ril_utf_qmi_thread_shutdown();
int send_ril_utf_ssr_error_cb(struct ril_utf_msg_t *test_step);
int send_ril_utf_ssr_notify_cb(struct ril_utf_msg_t *test_step);
void utf_wait_for_notify_reg_cb();

void block_qmi_client_init(enum ril_utf_qmi_srv_id_t service);
void unblock_qmi_client_init();
int check_block_client_init();
qmi_client_error_type
ril_utf_qmi_client_get_any_service
(
 qmi_idl_service_object_type service_obj,
 qmi_service_info *service_info
 );

#endif
