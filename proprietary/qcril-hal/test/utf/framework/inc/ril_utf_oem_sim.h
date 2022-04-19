/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_oem_sim.h
  @brief   RIL UTF OEM simulation

  DESCRIPTION
   Go between RIL UTF test and QMI OEM socket
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef _RIL_UTF_OEM_SIM_H_
#define _RIL_UTF_OEM_SIM_H_

#include "ril_utf_common_includes.h"
#include "ril_utf_defs.h"
#include "ril_utf_core.h"
#include "LteDirectDiscovery.pb.h"

#define PACKED_OEM_MSG_MAX_LEN 1000

typedef struct utf_oem_msg_q_payload_t {
  int   token;
  char *msg;
  int   msg_len;
} utf_oem_msg_q_payload;

struct utf_oem_msg_q_t {
  void *payload;
  int payload_len;

  struct utf_oem_msg_q_t *next;
  struct utf_oem_msg_q_t *prev;
};

struct oem_buff_t {
  int size;
  uint8_t *payload;
};

typedef struct utf_oem_msg_q_t OEMmsgQ;

int connect_to_ril_oem_socket(void);
void start_oem_endpoint(void);
void ril_utf_send_oem_message(utf_oem_msg_q_payload *msg);
int get_oem_msg_to_send(utf_oem_msg_q_payload *msg);
void enqueue_oem_msg_to_send(void *msg, int len);
void ril_utf_recv_oem_message(void);
void process_oem_inject_queue(void);
void utf_oem_start(void *param);
int utf_oem_recv_msg(struct oem_buff_t msgs[]);
void handle_oem_msg(int size, uint8_t *payload, int msg_type, int msg_token, uint32_t oem_error);
int send_ril_utf_oem_request( struct ril_utf_msg_t *test_step );

void enqueue_oem_expect(int command_id, int msg_id, int service_id, int token_id, enum ril_utf_q_element_t type,
  void *payload, int payload_len);
void* utf_oem_recv_start( void *param);
void* utf_oem_send_start( void *param);
int oem_unpack_int_bigendian(char* data);

void ril_utf_oem_send_thread_shutdown();
void ril_utf_oem_recv_thread_shutdown();
#endif
