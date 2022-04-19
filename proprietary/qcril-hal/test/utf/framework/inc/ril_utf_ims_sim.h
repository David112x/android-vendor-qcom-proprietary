/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_ims_sim.h
  @brief   RIL UTF IMS simulation

  DESCRIPTION
   Simulated IMS interfaces for RIL UTF
   Go between RIL UTF test and QMI RIL IMS
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef _RIL_UTF_IMS_SIM_H_
#define _RIL_UTF_IMS_SIM_H_

#include "ril_utf_common_includes.h"
#include "ril_utf_defs.h"
#include "ril_utf_core.h"
#include "qcril_qmi_npb_decode.h"
#include "qcril_qmi_npb_encode.h"
#include "qcril_qmi_ims_msg_meta.h"

#define PACKED_IMS_MSG_MAX_LEN 1000

struct utf_ims_msg_q_t {
  void *payload;
  int payload_len;

  struct utf_ims_msg_q_t *next;
  struct utf_ims_msg_q_t *prev;
};

struct ims_buff_t {
  int size;
  uint8_t *payload;
};

typedef struct utf_ims_msg_q_t ImsmsgQ;

int connect_to_ril_ims_socket(void);
void start_ims_endpoint(void);
void ril_utf_send_ims_message(char *msg, int len);
int get_ims_msg_to_send(char *msg, int *len);
int send_ril_utf_ims_request( struct ril_utf_msg_t *test_step );
void ril_utf_recv_ims_message(void);
void process_ims_inject_queue(void);
void utf_ims_start(void *param);
int utf_ims_recv_msg(struct ims_buff_t *msg);
void handle_ims_msg(int size, uint8_t *payload);
ims_MsgTag* ril_utf_ims_unpack_msg_tag(uint8_t *buf);
int ril_utf_ims_parse_packed_msg(ims_MsgType msg_type, ims_MsgId msg_id, const uint8_t *byte_stream, size_t byte_stream_size,
                                     void** ims_msg_dptr, size_t* ims_msg_size_ptr);
void enqueue_ims_expect(int size, void *payload, int msg_id,
                        int token, enum ril_utf_q_element_t type, ims_Error error);
void ril_utf_ims_send_thread_shutdown();
void ril_utf_ims_recv_thread_shutdown();
void* utf_ims_recv_start( void *param);
void* utf_ims_send_start( void *param);

#endif
