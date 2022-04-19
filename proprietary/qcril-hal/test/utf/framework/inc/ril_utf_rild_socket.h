/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_rild_socket.h
  @brief   RIL UTF interface to rild socket

  DESCRIPTION
   Go between RIL UTF test and QMI RILD socket
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef _RIL_UTF_RILD_SOCKET_H_
#define _RIL_UTF_RILD_SOCKET_H_

#include "ril_utf_common_includes.h"
#include "ril_utf_defs.h"
#include "ril_utf_core.h"

#define PACKED_RILD_MSG_MAX_LEN 1000
#define ALIGNED(s) (((s)+3)&~3)
#define MAX_PAYLOAD (512 * 4)

typedef struct {
   uint32 length;
   uint32 type;
} ril_any_message;

typedef struct {
   uint32 length;
   uint32 request_type;
   uint32 token;
   byte   data; // start of data if present -or- 0 (4bytes) to indicate zero size
} ril_request;

typedef struct {
   uint32 length;
   uint32 response_type;
   uint32 token;
   uint32 error;
   uint32 data_length;
   byte   data; // start of data if present
} ril_response;

typedef struct {
   uint32 length;
   uint32 response_type;
   uint32 unsol_response_type;
   uint32 data_length;
   byte   data; // start of data if present
} ril_unsol_response;

struct utf_rild_msg_q_t {
  void *payload;
  int payload_len;

  struct utf_rild_msg_q_t *next;
  struct utf_rild_msg_q_t *prev;
};

typedef struct {
   int length;
   char* string;
} char_array;

struct rild_buff_t {
  int size;
  uint8_t *payload;
};

typedef struct utf_rild_msg_q_t RILDmsgQ;

int connect_to_ril_rild_socket(void);
void start_rild_endpoint(void);
void ril_utf_send_rild_message(char *msg, int len);
void ril_utf_recv_rild_message(void);
void utf_rild_start(void *param);
int utf_rild_recv_msg(struct rild_buff_t *msg);
void handle_rild_msg(int size, uint8_t *payload);
void enqueue_rild_expect(int msg_id, int token_id, enum ril_utf_q_element_t type,
  void *payload, int payload_len, RIL_Errno error);
int unpack_ril_response(int message_id, uint32 len, char *data, int *dest_len, void **dest_payload);
void* utf_rild_recv_start( void *param);

void ril_utf_rild_send_thread_shutdown();
void ril_utf_rild_recv_thread_shutdown();


/// pack an int32 in bigendian format
void pack_int_bigendian(char* data, int value);
int unpack_int_bigendian(char* data);

/// pack a regualr ascii string
int pack_string(char* dest, const int length, const char* src);

/// pack an ascii string in java serialized UTF16 format
int pack_string16(char* dest, const char* src);
int unpack_16string(char* dest, const byte* src, const int length);

/// pack an byte for ril message
int pack_int8(char* dest, const char src);
int pack_int8_array(char* dest, const unsigned char num_bytes,
                                const char* src);

/// pack an int in java serialzed format
int pack_int32(char* dest, const int src);
int unpack_int32(const byte* src, int* value);

/// pack an int array in java serialzed format
int pack_int32_array(char* dest, const int count, const int* src);
#endif


