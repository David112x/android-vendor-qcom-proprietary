/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_proxy_sim.h
  @brief   RIL UTF PROXY simulation

  DESCRIPTION
   Go between RIL UTF test and QMI PROXY
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef _RIL_UTF_PROXY_SIM_H_
#define _RIL_UTF_PROXY_SIM_H_

#include "ril_utf_common_includes.h"
#include "ril_utf_defs.h"
#include "ril_utf_core.h"

#define PACKED_PROXY_MSG_MAX_LEN 1000

struct utf_proxy_msg_q_t {
  void *payload;
  int payload_len;

  struct utf_proxy_msg_q_t *next;
  struct utf_proxy_msg_q_t *prev;
};

struct proxy_buff_t {
  int size;
  uint8_t *payload;
};

typedef struct utf_proxy_msg_q_t ProxymsgQ;

int connect_to_ril_proxy_socket(void);
void start_proxy_endpoint(void);
void ril_utf_send_proxy_message(char *msg, int len);
int get_proxy_msg_to_send(char *msg, int *len);
void enqueue_proxy_msg_to_send(void *msg, int len);
void ril_utf_recv_proxy_message(void);
void process_proxy_inject_queue(void);
void utf_proxy_start(void *param);
int utf_proxy_recv_msg(struct proxy_buff_t msgs[]);
void handle_proxy_msg(int size, uint8_t *payload);
void enqueue_proxy_expect();
void* utf_proxy_recv_start( void *param);
void* utf_proxy_send_start( void *param);

#endif
