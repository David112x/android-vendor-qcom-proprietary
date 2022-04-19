/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_msim_rild.h
  @brief   RIL UTF MSIM IPC rild simulation

  DESCRIPTION
    This simulation file is used to simulate the second rild in msim call flows
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef _RIL_UTF_MSIM_RILD_H_
#define _RIL_UTF_MSIM_RILD_H_

#include "ril_utf_common_includes.h"
#include "ril_utf_defs.h"
#include "ril_utf_core.h"

#define PACKED_MSIM_MSG_MAX_LEN 1000

struct utf_msim_msg_q_t {
  void *payload;
  int payload_len;

  struct utf_msim_msg_q_t *next;
  struct utf_msim_msg_q_t *prev;
};

#define QCRIL_MAX_IPC_PAYLOAD_SIZE            600
typedef struct ipc_send_recv_data_info
{
  qcril_instance_id_e_type rild_instance_id;
  ipc_message_id_type message_id;
  char payload[QCRIL_MAX_IPC_PAYLOAD_SIZE];
  int payload_length;
}ipc_send_recv_data_info_type;

typedef struct utf_msim_msg_q_t msimmsgQ;

int setup_ril_msim_socket(void);
void start_msim_endpoint(void);
void ril_utf_send_msim_message(char *msg, int len);
int get_msim_msg_to_send(char *msg, int *len);
int send_ril_utf_msim_request( struct ril_utf_msg_t *test_step );
void ril_utf_recv_msim_message(void);
void process_msim_inject_queue(void);
void utf_msim_start(void *param);
int utf_msim_recv_msg(struct ipc_send_recv_data_info* msim_recv_buffer_ptr,
                      int* msim_recv_byte_num_ptr);
void handle_msim_msg(struct ipc_send_recv_data_info msim_recv_buffer,
                      int msim_recv_byte_num);
void enqueue_msim_expect(int size, void *payload, int msg_id,
                         int rild_instance_id, enum ril_utf_q_element_t type);
void ril_utf_msim_recv_thread_shutdown();
void* utf_msim_recv_start( void *param);
void* utf_msim_send_start( void *param);

#endif
