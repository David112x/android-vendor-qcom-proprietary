/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_ioe_client_sim.h
  @brief   RIL UTF IOE CLIENT simulation

  DESCRIPTION
   Go between RIL UTF test and IOE SERVER
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef _RIL_UTF_IOE_CLIENT_SIM_H_
#define _RIL_UTF_IOE_CLIENT_SIM_H_

#include "ril_utf_common_includes.h"
#include "ril_utf_defs.h"
#include "ril_utf_core.h"

#define PACKED_IOE_CLIENT_MSG_MAX_LEN (1024*8)

struct utf_ioe_client_msg_q_t {
  void *payload;
  int payload_len;

  struct utf_ioe_client_msg_q_t *next;
  struct utf_ioe_client_msg_q_t *prev;
};

struct ioe_client_buff_t {
  int size;
  uint8_t *payload;
};

typedef struct utf_ioe_client_msg_q_t Ioe_client_msgQ;

int connect_to_ril_ioe_client_socket(uint8_t client_handle);
void start_ioe_client_endpoint(void);
void ril_utf_send_ioe_client_message(char *msg, int len);
int get_ioe_client_msg_to_send(char *msg, int *len);
void enqueue_ioe_client_msg_to_send(void *msg, int len);
void ril_utf_recv_ioe_client_message(void);
void process_ioe_client_inject_queue(void);
void utf_ioe_client_start(void *param);
int utf_ioe_client_recv_msg(struct ioe_client_buff_t msgs[]);
void handle_ioe_client_msg(int size, uint8_t *payload);
void enqueue_ioe_client_expect();
void* utf_ioe_client_recv_start( void *param);
void* utf_ioe_client_send_start( void *param);

#endif
