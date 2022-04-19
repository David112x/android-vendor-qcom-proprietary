/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_rild_socket.c
  @brief   RIL UTF RILD emulation for communication via RILD socket

  DESCRIPTION
   Go between RIL UTF test and QMI RILD socket
  ---------------------------------------------------------------------------
******************************************************************************/
#include "ril_utf_rild_socket.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>

// Globals
extern struct ril_utf_expect_t *ril_utf_rec_head;
extern struct ril_utf_expect_t *ril_utf_rec_tail;
extern pthread_t rild_recv_id;
extern int rild_expect_ready;
extern pthread_cond_t wakeup_rild_recv_thread;


// Local definitions
#define UTF_QMI_RILD_SOCKET_PATH "/dev/socket/urild"
#define UTF_RILD_SOCKET_CONNECT_MAX_ATTEMPTS 10
#define MAX_RILD_MESSAGES 10
#define RIL_UTF_RILD_MAX_BUFFER_BYTES_PER_LINE                16
#define RIL_UTF_RILD_MAX_OUTPUT_BUF_SIZE                      ( ( RIL_UTF_RILD_MAX_BUFFER_BYTES_PER_LINE * 5 ) + 2 )


#define INT8_SIZE  1
#define INT32_SIZE 4

uint8_t rild_recv_buffer[RIL_UTF_RILD_SOCKET_MAX_BUF_SIZE];
int rild_recv_byte_num;

struct token_pair {
  int token_id;
  int msg_id;
  int valid;
};

struct token_pair token_list[50];

int qmi_rild_sock_id;

RILDmsgQ *rild_expect_msg_q_head = NULL;
RILDmsgQ *rild_expect_msg_q_tail = NULL;

/*========================================================
  FUNCTION: utf_start_rild_threads
    - starts the following threads:
      - utf_rild_send_start
      - utf_rild_recv_start
========================================================*/
void utf_start_rild_threads()
{
  pthread_attr_t rild_send_thrd_attr;
  pthread_attr_t rild_recv_thrd_attr;
  int ret;

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Connecting to QMI RILD socket...");
  if ( connect_to_ril_rild_socket() != TRUE )
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error: Could not connect to QMI RILD socket. check error msg");
    return;
  }


  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Starting QMI RILD recv thread...");
  pthread_attr_init(&rild_recv_thrd_attr);
  ret = pthread_create(&rild_recv_id, &rild_recv_thrd_attr, utf_rild_recv_start, NULL);

  if ( ret < 0 )
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Failed to start rild recv thread. Exiting...");
    return;
  }

}

/*========================================================
  FUNCTION: connect_to_qmi_rild_socket
    - Attempts to make a connection to the exported
      socket from qmi rild
========================================================*/
int connect_to_ril_rild_socket(void)
{
  int sock_conn_length;
  int utf_rild_sock_conn_max_attempts = UTF_RILD_SOCKET_CONNECT_MAX_ATTEMPTS;

  struct sockaddr_un qmi_rild_addr;

  if ( ( qmi_rild_sock_id = socket(AF_UNIX, SOCK_STREAM, 0) ) < 0)
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: !!! Error !!! Could not create socket Exiting...");
    return FALSE;
  }

  memset(&qmi_rild_addr, 0x00, sizeof(qmi_rild_addr));

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Trying to connect to qmi rild socket %s",UTF_QMI_RILD_SOCKET_PATH);

  qmi_rild_addr.sun_family = AF_UNIX;
  strlcpy(qmi_rild_addr.sun_path, UTF_QMI_RILD_SOCKET_PATH, sizeof(qmi_rild_addr.sun_path) - 1);

  sock_conn_length = strlen(qmi_rild_addr.sun_path) + sizeof(qmi_rild_addr.sun_family);


  while ( utf_rild_sock_conn_max_attempts )
  {
    if ( connect(qmi_rild_sock_id, (struct sockaddr*) &qmi_rild_addr, sock_conn_length) < 0 )
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: !!! Error !!!Failed to qmi rild socket.");
      utf_rild_sock_conn_max_attempts--;

      if ( !utf_rild_sock_conn_max_attempts )
      {
        RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: !!! Error !!!Failed to qmi rild socket after %d attempts.", UTF_RILD_SOCKET_CONNECT_MAX_ATTEMPTS);
        exit(1);
        return FALSE;
      }
    }
    else
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Connected to qmi rild socket.");
      break;
    }
  }


  return TRUE;
}

/*========================================================
  FUNCTION: ril_utf_send_rild_message
  - Send message to qmi rild socket

  Critical section access:
  - exec_q_access must be locked prior to calling this
    function.
========================================================*/
void ril_utf_send_rild_message(char *msg, int len)
{
  if (send(qmi_rild_sock_id, (void *) msg, len, 0) < 0)
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error! Failed to send mesage to qmi rild socket.");
  }
}

/*========================================================
  FUNCTION: send_ril_utf_rild_socket_request

  Critical section access:
  - exec_q_access must be locked prior to calling this
    function.
========================================================*/
int send_ril_utf_rild_socket_request( struct ril_utf_msg_t *test_step )
{
  int x;
  int size = 0;
  char* data = (char*) malloc(MAX_PAYLOAD);
  char *message = (char*) malloc(MAX_PAYLOAD + 8);

  // Keep track of token and msg id pair
  for (x = 0; x < 50; ++x)
  {
    if (token_list[x].valid == 0)
    {
      token_list[x].valid = 1;
      token_list[x].token_id = test_step->token_id;
      token_list[x].msg_id = test_step->msg_id;
      break;
    }
  }

  RIL_UTF_DEBUG("\n Entering function %s", __func__);

  if (test_step->msg_id == RIL_REQUEST_DIAL) // use for messages with specific data
  {
    char* ptr = data;
    RIL_Dial *dial_req = (RIL_Dial*) test_step->payload;

    size += pack_string16(ptr, dial_req->address);
    size = ALIGNED(size);
    ptr += size;
    size += pack_int32(ptr, dial_req->clir);
    ptr += 4;

    // currently don't support uus info
    size += pack_int32(ptr, 0);

  }
  else
  {
    char* ptr = data;

    size += pack_int32(ptr, 1);
    ptr += 4;

    int value = *((int*)test_step->payload);
    size += pack_int32(ptr, value);
    ptr = data + size;
  }

   // clear buffer
   memset(message, 0xee, MAX_PAYLOAD);

   int msg_size = 8 + size;
   int pkt_size = msg_size + 4;
   ril_request* req = (ril_request*) message;

   char msg_size_pk[4] = {0};
   pack_int_bigendian(msg_size_pk, msg_size);

   // fill out request message
   memcpy(&req->length, msg_size_pk, 4);

   req->request_type = test_step->msg_id;
   req->token = test_step->token_id;

   if (size) {
      // copy payload
      memcpy(&req->data, data, size);
   }

   RIL_UTF_DEBUG("send_msg: %d token: %d, size %d\n", req->request_type,
            req->token, size);

  // Send message through RILD socket
  ril_utf_send_rild_message(message, pkt_size);

  // Inform test thread we are finished
  inject_complete = 1;

  RIL_UTF_DEBUG("\nExiting function %s", __func__);

  return 0;
}

/*========================================================
  FUNCTION: utf_rild_recv_start
  - Starts the utf rild recv thread.

  Critical section access:
  - exec_q_access locked and unlocked here.
  - Note: pthread_wait will release exec_q_access
    when waiting, and locks again upon re-entering.
========================================================*/
void* utf_rild_recv_start( void *param)
{
  fd_set rfds;
  int ret_val;
  int i, my_expect_slot, x;
  struct rild_buff_t msg;
  struct timespec timeout;

  while( EXEC_RILD_THREAD )
  {
    memset(&msg, 0, sizeof(msg));

    FD_ZERO(&rfds);
    FD_SET(qmi_rild_sock_id, &rfds);
    timeout.tv_sec = 0;
    timeout.tv_nsec = 10;

    ret_val = select(qmi_rild_sock_id+1, &rfds, NULL, NULL, NULL);

    if ( ret_val == -1 )
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: RILD recv thread error in select in func %s", __func__);
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Select returns error %d", ret_val);
      return NULL;
    }
    else if ( ret_val )
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: %s: RILD recv thread receiving message", __func__);
      utf_rild_recv_msg(&msg);

      // acquire mutex
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: RILD recv thread locking exec_q_access in %s", __func__);
      EXEC_Q_LOCK();

      // Grab next available slot in expect table
      for ( i = 0; i < MAX_EXPECTS; ++i )
      {
        if (expect_table[i].in_use == 0)
        {
          my_expect_slot = i;
          expect_table[my_expect_slot].in_use = 1;
          break;
        }
      }

  // currently not used on target as all expects are queued and should be handled in order received
      // verify that we are allowed to proceed
      struct timeval e_timeout;
      e_timeout.tv_sec = 0;
      e_timeout.tv_usec = EXPECT_TIMEOUT;
      while ( expect_table[my_expect_slot].expect_ready != 1 )
      {
        calc_timeout(&timeout, &e_timeout);
        pthread_cond_timedwait(&expect_ready_sig, &exec_q_access, &timeout);
      }

      // Decode message and enqueue
      handle_rild_msg(msg.size, msg.payload);

      // signal test thread.
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Setting expect_received to true @ %s %s %d ",__FILE__, __func__, __LINE__);
      expect_table[my_expect_slot].expect_received = 1;
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Signalling Expectation");
      pthread_cond_signal(&signal_exptectation);

      // release mutex
      EXEC_Q_UNLOCK();
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: %s. released exec_q_mutex.", __func__);

    }
    else if (ret_val == 0)
    {
      // timeout
    }
    else
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: RILD recv thread error in select (should not get here) in %s", __func__);
    }

    // Check for reboot request
    if (ril_utf_check_reboot_state() == 1)
    {
      ril_utf_rild_recv_thread_shutdown();
    }
  } // while( EXEC_RILD_THREAD )

}

/*========================================================
  FUNCTION: utf_rild_recv_msg
    - Recieves all outstanding messages on the socket
      and modifies "msgs" to contain the size and ptr
      to messages in buffer received
    - Note:  On target messages often arrive fragmented
      from qcril.  As a result we must verify that all
      data predicted by the size field is received before
      proceeding
  Critical section access:
    - none
========================================================*/
int utf_rild_recv_msg(struct rild_buff_t *msg)
{
  int ret = TRUE;
  char buff[30];
  int length = 0;

  uint8_t temp_buffer[RIL_UTF_RILD_SOCKET_MAX_BUF_SIZE];
  int temp_byte_num;

  // init buffers
  rild_recv_byte_num = 0;
  memset(rild_recv_buffer, 0, sizeof(rild_recv_buffer));
  memset(temp_buffer, 0, sizeof(temp_buffer));

  // read message size
  recv(qmi_rild_sock_id, temp_buffer, 4, 0);
  memcpy(buff, temp_buffer, 4);
  length = unpack_int_bigendian(buff);

  RIL_UTF_DEBUG("\n QMI_RIL_UTF: NEED TO READ %d BYTES FROM RILD", length);

  // Add data to final buffer
  memcpy(rild_recv_buffer+rild_recv_byte_num, temp_buffer, 4);
  rild_recv_byte_num += 4;


  // Keep calling recv() until all bytes arrive
  do {
    if ((temp_byte_num = recv(qmi_rild_sock_id, temp_buffer, length, 0)) <= 0)
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Error: RILD could not read bytes in %s", __func__);
      ret = FALSE;
      return ret;
    }

    // Add data to final buffer
    memcpy(rild_recv_buffer+rild_recv_byte_num, temp_buffer, temp_byte_num);
    rild_recv_byte_num += temp_byte_num;

    RIL_UTF_DEBUG("\n QMI_RIL_UTF: READ %d BYTES FROM RILD", temp_byte_num);

  } while ((length+4) > rild_recv_byte_num);




  RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: %s: RILD received %d bytes... dumping hex", __func__, rild_recv_byte_num);
  ril_utf_print_hex(rild_recv_buffer, rild_recv_byte_num);

  // Assume only one message
  msg->payload = rild_recv_buffer;
  msg->size = rild_recv_byte_num;

  return ret;
}

/*========================================================
  FUNCTION: handle_rild_msg
    - decode incoming rild message
  Critical section access:
  - exec_q_access is expected to be locked prior to calling
    this function.
========================================================*/
void handle_rild_msg(int size, uint8_t *payload)
{
  int x;
  int length;
  int message_id = 0;
  int token = 0;
  int error = 0;
  char buff[30];
  enum ril_utf_q_element_t type;
  int payload_len;
  void *message_payload;
  ril_unsol_response* unsol_msg;
  ril_response* response_msg;

  RIL_UTF_DEBUG("\n QMI_RIL_UTF: Received RILD message in function %s", __func__);

  memcpy(buff, payload, 4);
  length = unpack_int_bigendian(buff);

  RIL_UTF_DEBUG("\n QMI_RIL_UTF: RILD TOTAL MESSAGE LEN IS %d", length);


  ril_any_message *msg = (ril_any_message*) payload;

  // Switch on message type
  switch (msg->type) {
    case 0x0:
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: RECEIVED RILD RESPONSE MESSAGE");
      response_msg = (ril_response*) payload;
      type = ril_utf_ril_response;
      token = response_msg->token;
      error = response_msg->error;

      // retrive corresponding msg id
      for (x = 0; x < 50; ++x)
      {
        if ( (token_list[x].valid == 1) &&
             (token_list[x].token_id = token) )
        {
          message_id = token_list[x].msg_id;\
          token_list[x].valid = 0;
          break;
        }
      }

      if (length <= 12) // no payload only error and token
      {
        payload_len = 0;
        message_payload = NULL;
      }
      else
      {
        unpack_ril_response(message_id, response_msg->data_length, &(response_msg->data), &payload_len, &message_payload);
      }
      break;
    case 0x01:
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: RECEIVED RILD UNSOL RESPONSE MESSAGE");
      unsol_msg = (ril_unsol_response*) payload;
      type = ril_utf_ril_unsol_response;
      token = -1;
      message_id = unsol_msg->unsol_response_type;
      payload_len = unsol_msg->data_length;
      message_payload = unsol_msg->data;
      break;
  }

  RIL_UTF_DEBUG("\n QMI_RIL_UTF: RILD message payload length is %d", payload_len);

  RIL_UTF_DEBUG("\nQMI_RIL_UTF: RILD MESSAGE RECEIVED WITH MESSAGE ID %d AND TOKEN ID %d", message_id, token);

  // Add message to queue for test thread
  enqueue_rild_expect(message_id, token, type, message_payload, payload_len, error);
}

/*========================================================
  FUNCTION: enqueue_rild_expect
    - Add received rild message to queue for test thread
  Critical section access:
  - exec_q_access is expected to be locked prior to calling
    this function.
========================================================*/
void enqueue_rild_expect(int msg_id, int token_id, enum ril_utf_q_element_t type,
  void *payload, int payload_len, RIL_Errno error)
{
  RIL_UTF_DEBUG("\n QMI_RIL_UTF: Entering function %s", __func__);
  struct ril_utf_msg_t *expect_node =
    (struct ril_utf_msg_t *) malloc(sizeof(struct ril_utf_msg_t));

  // prep the node for insertion
  expect_node->msg_id = msg_id;
  expect_node->token_id = token_id;
  expect_node->type = type;
  expect_node->error_val = error;
  expect_node->payload = payload;
  expect_node->payload_len = payload_len;

  enqueue_ril_utf_q_node(expect_node, &ril_utf_rec_head, &ril_utf_rec_tail);

  RIL_UTF_DEBUG("\n QMI_RIL_UTF: Exiting function %s", __func__);
}

void ril_utf_rild_send_thread_shutdown()
{
  RIL_UTF_DEBUG("\nQMI_RIL_UTF: UNLOCKING exec_q_mutex");
  EXEC_Q_UNLOCK();

  pthread_exit(NULL);
}
void ril_utf_rild_recv_thread_shutdown()
{
  RIL_UTF_DEBUG("\nQMI_RIL_UTF: Executing rild recv thread shutdown");
  fclose(qmi_rild_sock_id);
  pthread_exit(NULL);
}


int unpack_ril_response(int message_id, uint32 len, char *data, int *dest_len, void **dest_payload)
{
  if (message_id == RIL_REQUEST_GET_CURRENT_CALLS)
  {
    int x;
    int num_calls = len;
    RIL_Call **list = (RIL_Call**)malloc(num_calls * sizeof(RIL_Call*));

    for (x = 0; x < num_calls; ++x)
    {
      list[x] = (RIL_Call*)malloc(sizeof(RIL_Call));
      memset(list[x], 0, sizeof(RIL_Call));

      list[x]->state = *(uint32*)(data); data += 4;
      list[x]->index = *(uint32*)(data); data += 4;
      list[x]->toa = *(uint32*)(data); data += 4;
      list[x]->isMpty = *(uint32*)(data); data += 4;
      list[x]->isMT = *(uint32*)(data); data += 4;
      list[x]->als = *(uint32*)(data); data += 4;
      list[x]->isVoice = *(uint32*)(data); data += 4;
      list[x]->isVoicePrivacy = *(uint32*)(data); data += 4;

      int number_len = *(uint32*)(data); data += 4;
      if (number_len != -1)
      {
        if (number_len > 0)
          --number_len; // no \0 from RILD
        list[x]->number = (char*)malloc(sizeof(char) * number_len);
        int bytes = unpack_16string(list[x]->number, data, number_len);
        data += bytes;
      }

      list[x]->numberPresentation = *(uint32*)(data); data += 4;

      int name_len = *(uint32*)(data); data += 4;
      if (name_len != -1)
      {
        if (name_len > 0)
          --name_len; // no \0 from RILD
        list[x]->name = (char*)malloc(sizeof(char) * name_len);
        int bytes = unpack_16string(list[x]->name, data, name_len);
        data += bytes;
      }

      list[x]->namePresentation = *(uint32*)(data); data += 4;

      int uusInfoPresent = *(uint32*)(data); data += 4;
      if (uusInfoPresent)
      {
        printf("error uus not currently supported for unpacking");
        exit(1);
      }
    } // end for

    // Set output length
    (*dest_len) = num_calls*sizeof(RIL_Call*);
    // Set output payload
    (*dest_payload) = (void*)list;
  }
  else
  {
    (*dest_len) = len;
    (*dest_payload) = (void*)data;
  }

  return 0;
}


////////////////////////////HELPER FUNCTIONS////////////////////////////////
// The following functions are used to pack messages for use in RILD socket
////////////////////////////////////////////////////////////////////////////


//=========================================================================
// FUNCTION: pack_int_bigendian
//
// DESCRIPTION: unpack a 4 byte value in big-endian
//
// RETURN: unpacked data as an int
//=========================================================================
//
void pack_int_bigendian(char* data, int value)
{
  int i;

  for (i = 3; i > -1; i--) {
    data[i] = (byte) value;
    value >>= 8;
  }
}


//=========================================================================
// FUNCTION: unpack_int_bigendian
//
// DESCRIPTION: unpack a 4 byte value in big-endian
//
// RETURN: unpacked data as an int
//=========================================================================
//
int unpack_int_bigendian(char* data) {

   int value = 0;
   value += ((unsigned char)data[0]) << 24;
   value += ((unsigned char)data[1]) << 16;
   value += ((unsigned char)data[2]) << 8;
   value += ((unsigned char)data[3]);

   return value;
}

//=========================================================================
// FUNCTION: pack_int8 <dest> <src>
//
// DESCRIPTION:
/// pack an byte for ril message
//
// RETURN:
// number of bytes in <dest>
//=========================================================================
//
int pack_int8(char* dest, const char src) {

   *dest = src;
   return ALIGNED(INT8_SIZE);
}

//=========================================================================
// FUNCTION: pack_int8_array <dest> <src>
//
// DESCRIPTION:
/// pack an byte for ril message
//
// RETURN:
// number of bytes in <dest>
//=========================================================================
//
int pack_int8_array(char* dest, const unsigned char num_bytes,
                                const char* src) {

   char* base = dest;
   int size = pack_int8(dest, num_bytes);
   dest += size;

   int i = 0;
   while(i < num_bytes) {
      size += pack_int8(dest, src[i++]);
      dest = base + size;
   }
//   size += 36;
   return ALIGNED(size);
}

//=========================================================================
// FUNCTION: pack_int32 <dest> <src>
//
// DESCRIPTION:
// pack a single int32 in java serialized format
//
// RETURN:
// number of bytes in <dest>
//=========================================================================
//
int pack_int32(char* dest, const int src) {

   // pack the int
   memcpy(dest, &src, INT32_SIZE);

   return INT32_SIZE;
}


//=========================================================================
// FUNCTION: unpack_int32 <src> <return value>
//
// DESCRIPTION:
// pack a single int32 in java serialized format
//
// RETURN:
//    1 - success
//    0 - failure
//=========================================================================
//
int unpack_int32(const byte* src, int* value) {

   assert(src);

   int* int_array = (int*) src;

   if (int_array[0] != 1) {
      printf("Error: unexpected payload num elmts %d\n", int_array[0]);
      return 0;
   }

   *value = int_array[1];

   return 1;
}

//=========================================================================
// FUNCTION: pack_int32_array <dest> <src>
//
// DESCRIPTION:
// pack an array of int32s in java serialized format
// packs a count of ints, followed by each int.
//
// RETURN:
// number of bytes in <dest>
//=========================================================================
//
int pack_int32_array(char* dest, const int count, const int* src) {

   bzero(dest, INT32_SIZE + (INT32_SIZE * count) );

   // pack size
   dest[0] = (byte)count;
   dest += INT32_SIZE;

   // pack the int
   int cnt = 0;
   while(cnt < count) {
      memcpy(dest, &src[cnt++], INT32_SIZE);
   }

   return INT32_SIZE + (INT32_SIZE * count);
}

//=========================================================================
// FUNCTION: pack_string <dest> <len> <src>
//
// DESCRIPTION:
// pack a 'C' ascii string in java serialized ascii String format
// assumes <src> is a null terminate string
// -1 length indicates a null string
//
// RETURN:
// number of bytes in <dest>
//=========================================================================
//
int pack_string(char* dest, const int length, const char* src) {

   // pkt size is (int size + string)
   int size = 0;

   // Pack length
   memcpy(dest, &length, INT32_SIZE);
   dest += sizeof(int);
   size += INT32_SIZE;

   // length is -1 or src is null, no string to copy to data
   if (!src || length == -1) { return size; } // no need to align an int

   // Note: we copy entire string regardless or length passed
   // this can lead to invalid pkts, but thats needed for fuzzing
   // for a good packet strlen(src) == length
   int str_len = strlen(src);
   memcpy(dest, src, str_len);
   dest += str_len;
   size += str_len;

   printf("pack_string: length %d, actual strlen %d, pkt size %d\n",
           length, str_len, size);
   if (str_len != length) {
      printf("Warning: invalid pkt, length != strlen\n");
   }

   return ALIGNED(size);
}

//=========================================================================
// FUNCTION: pack_string16 <dest> <src>
//
// DESCRIPTION:
// pack a 'C' ascii string in java serialized UTF16 String format
// assumes <src> is a null terminate string
//
// RETURN:
// number of bytes in <dest>
//=========================================================================
//
int pack_string16(char* dest, const char* src) {

   // data size is an (int + UTF16 + NULL terminator)
   int size = 0;

   int str_len = -1; // Java writes -1 for NULL arrays
   if (src) {
      str_len = strlen(src);
   }

   // Pack length of string
   memcpy(dest, &str_len, INT32_SIZE);
   dest += INT32_SIZE;
   size += INT32_SIZE;

   if (!src) { return size; } // NULL string, no need to align size is int

   // Convert ascii string to UTF16 and pack
   while(*src) {
      *dest++ = *src++;
      *dest++ = 0;
   }
   // include NULL terminator
   *dest++ = 0;
   *dest = 0;
   size += (2 * (str_len + 1));

   return ALIGNED(size);
}

//=========================================================================
// FUNCTION: unpack_16string <dest> <src> <length>
//
// DESCRIPTION: unpack a UTF16 string to 8 bit ascii
//
// RETURN:
// bytes consumed from src
// unpacked ascii string in <dest>
//=========================================================================
//
int unpack_16string(char* dest, const byte* src, const int length) {

   assert(dest);
   assert(src);
   assert(length >= 0);

   int cnt = 0;
   while(cnt <= length + 1) { // add one for trailing null
      *dest++ = *src++;
       src++; // strip high order byte
       cnt++;
   }

   return ALIGNED(cnt * 2);
}
