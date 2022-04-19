/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_ims_sim.c
  @brief   RIL UTF IMS simulation and endpoint handling

  DESCRIPTION
   Simulated IMS interfaces for RIL UTF
   Go between RIL UTF test and QMI RIL IMS
  ---------------------------------------------------------------------------
******************************************************************************/
#include "ril_utf_ims_sim.h"
extern "C" {
#include "ril_utf_message_printer.h"
}
#include "ril_utf_i_ims_radio_sim.h"
#include "qcril_qmi_ims_packing.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>

// Globals
extern struct ril_utf_msg_t *ril_utf_rec_head;
extern struct ril_utf_msg_t *ril_utf_rec_tail;
extern pthread_t ims_recv_id;
extern int ims_expect_ready;
extern pthread_cond_t wakeup_ims_recv_thread;
extern pthread_mutex_t reset_flag_mutex;
extern boolean reset_now_flag;
// MSIM RIL instance
extern int rild_instance;

// Local definitions
int ims_unpack_int_bigendian(char* data);
#ifdef UTF_TARGET_BUILD
#define UTF_QMI_RIL_IMS_SOCKET_PATH "/dev/socket/qmux_radio/utf_rild_ims"
#else
#define UTF_QMI_RIL_IMS_SOCKET_PATH "./rild_ims0"
#define UTF_QMI_RIL_IMS_SECONDARY_SOCKET_PATH "./rild_ims1"
#endif

#define UTF_IMS_SOCKET_CONNECT_MAX_ATTEMPTS 10
#define MAX_IMS_MESSAGES 10

uint8_t ims_recv_buffer[RIL_UTF_IMS_SOCKET_MAX_BUF_SIZE];
int ims_recv_byte_num;

int qmi_ril_ims_sock_id;

ImsmsgQ *ims_expect_msg_q_head = NULL;
ImsmsgQ *ims_expect_msg_q_tail = NULL;

/*========================================================
  FUNCTION: utf_start_ims_thread
    - starts the following threads:
      - utf_ims_send_start
      - utf_ims_recv_start
========================================================*/
void utf_start_ims_threads()
{
  pthread_attr_t ims_send_thrd_attr;
  pthread_attr_t ims_recv_thrd_attr;
  int ret;

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Connecting to QMI RIL IMS socket...");
  if ( connect_to_ril_ims_socket() != TRUE )
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error: Could not connect to QMI RIL IMS socket. check error msg");
    return;
  }


  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Starting IMS recv thread...");
  pthread_attr_init(&ims_recv_thrd_attr);
  ret = utf_pthread_create_handler(&ims_recv_id, &ims_recv_thrd_attr, utf_ims_recv_start, NULL);

  if ( ret < 0 )
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Failed to start ims recv thread. Exiting...");
    return;
  }
}

/*========================================================
  FUNCTION: connect_to_ril_ims_socket
    - Attempts to make a connection to the exported
      ims socket from qmi_ril
========================================================*/
int connect_to_ril_ims_socket(void)
{
  int sock_conn_length;
  int utf_ims_sock_conn_max_attempts = UTF_IMS_SOCKET_CONNECT_MAX_ATTEMPTS;

  struct sockaddr_un qmi_ril_ims_addr;

  if ( ( qmi_ril_ims_sock_id = socket(AF_UNIX, SOCK_STREAM, 0) ) < 0)
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: !!! Error !!!Could not create socket Exiting...");
    return FALSE;
  }

  memset(&qmi_ril_ims_addr, 0x00, sizeof(qmi_ril_ims_addr));

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Trying to connect to qmi ril ims socket %s",UTF_QMI_RIL_IMS_SOCKET_PATH);

  qmi_ril_ims_addr.sun_family = AF_UNIX;
  #ifdef RIL_UTF_L_MASTER
  if (rild_instance == 1)
  #else
  if (rild_instance == 0)
  #endif
  {
    strlcpy(qmi_ril_ims_addr.sun_path, UTF_QMI_RIL_IMS_SOCKET_PATH, sizeof(qmi_ril_ims_addr.sun_path) - 1);
  }
  else
  {
    strlcpy(qmi_ril_ims_addr.sun_path, UTF_QMI_RIL_IMS_SECONDARY_SOCKET_PATH, sizeof(qmi_ril_ims_addr.sun_path) - 1);
  }

  sock_conn_length = strlen(qmi_ril_ims_addr.sun_path) + sizeof(qmi_ril_ims_addr.sun_family);


  while ( utf_ims_sock_conn_max_attempts )
  {
    if ( connect(qmi_ril_ims_sock_id, (struct sockaddr*) &qmi_ril_ims_addr, sock_conn_length) < 0 )
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: !!! Error !!!Failed to qmi ril ims socket.");
      utf_ims_sock_conn_max_attempts--;

      if ( !utf_ims_sock_conn_max_attempts )
      {
        RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: !!! Error !!!Failed to qmi ril ims socket after %d attempts.", UTF_IMS_SOCKET_CONNECT_MAX_ATTEMPTS);
        return FALSE;
      }
    }
    else
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Connected to qmi ril ims socket.");
      break;
    }
  }


  return TRUE;
}


/*========================================================
  FUNCTION: ril_utf_send_ims_message
  - Send message to qmi ril ims socket

  Critical section access:
  - exec_q_access must be locked prior to calling this
    function.
========================================================*/
void ril_utf_send_ims_message(char *msg, int len)
{
  RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: %s: IMS sending %d bytes... dumping hex", __func__, len);
  ril_utf_print_hex((unsigned char *)msg, len);

  if (send(qmi_ril_ims_sock_id, (void *) msg, len, 0) < 0)
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error! Failed to send mesage to qmi ril ims socket.");
  }
}

/*========================================================
  FUNCTION: utf_ims_recv_start
  - Starts the utf ims recv thread.
  - will wait for signal from UTF main test exec
    for an "expect" to send a message to qmi ril.

  Critical section access:
  - exec_q_access locked and unlocked here.
  - Note: pthread_wait will release exec_q_access
    when waiting, and locks again upon re-entering.
========================================================*/
void* utf_ims_recv_start( void *param)
{
  fd_set rfds;
  int ret_val;
  int i, my_expect_slot, x;
  struct ims_buff_t msg;
  struct timeval timeout;
  struct timeval e_timeout;
  struct timespec thread_timeout;
  e_timeout.tv_sec = 0;
  e_timeout.tv_usec = EXPECT_TIMEOUT;

  while( EXEC_IMS_THREAD )
  {
    memset(&msg, 0, sizeof(msg));

    FD_ZERO(&rfds);
    FD_SET(qmi_ril_ims_sock_id, &rfds);
    timeout.tv_sec = 0;
    timeout.tv_usec = 10;

    ret_val = select(qmi_ril_ims_sock_id+1, &rfds, NULL, NULL, &timeout);

    if ( ret_val == -1 )
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: IMS recv thread error in select in func %s", __func__);
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Select returns error %d", ret_val);
      return NULL;
    }
    else if ( ret_val )
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: %s: IMS recv thread receiving message", __func__);
      utf_ims_recv_msg(&msg);

      // acquire mutex
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: IMS recv thread locking exec_q_access in %s", __func__);
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

      // verify that we are allowed to proceed
      while ( expect_table[my_expect_slot].expect_ready != 1 )
      {
        calc_timeout(&thread_timeout, &e_timeout);
        pthread_cond_timedwait(&expect_ready_sig, &exec_q_access, &thread_timeout);
      }

      // Decode message and enqueue
      handle_ims_msg(msg.size, msg.payload);

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
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: IMS recv thread error in select (should not get here) in %s", __func__);
    }

    // Check for reboot request
    if (ril_utf_check_reboot_state() == 1)
    {
      ril_utf_ims_recv_thread_shutdown();
    }
  } // while( EXEC_IMS_THREAD )

}

/*========================================================
  FUNCTION: utf_ims_recv_msg
    - Recieves all outstanding messages on the socket
      and modifies "msgs" to contain the size and ptr
      to messages in buffer received
  Critical section access:
    - none
========================================================*/
int utf_ims_recv_msg(struct ims_buff_t *msg)
{
  int ret = TRUE;
  char buff[30];
  int length = 0;

  uint8_t temp_buffer[RIL_UTF_IMS_SOCKET_MAX_BUF_SIZE];
  int temp_byte_num;

  ims_recv_byte_num = 0;
  memset(ims_recv_buffer, 0, sizeof(ims_recv_buffer));
  memset(temp_buffer, 0, sizeof(temp_buffer));

  // read message size
  recv(qmi_ril_ims_sock_id, temp_buffer, 4, 0);
  memcpy(buff, temp_buffer, 4);
  length = ims_unpack_int_bigendian(buff);

  // Keep calling recv() until all bytes arrive
  do {
    if ((temp_byte_num = recv(qmi_ril_ims_sock_id, temp_buffer, length, 0)) <= 0)
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Error: IMS could not read bytes in %s", __func__);
      ret = FALSE;
      return ret;
    }

    // Add data to final buffer
    memcpy(ims_recv_buffer+ims_recv_byte_num, temp_buffer, temp_byte_num);
    ims_recv_byte_num += temp_byte_num;

    RIL_UTF_DEBUG("\n QMI_RIL_UTF: READ %d BYTES FROM IMS", temp_byte_num);

  } while (length > ims_recv_byte_num);

  RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: %s: IMS received %d bytes... dumping hex", __func__, ims_recv_byte_num);
  ril_utf_print_hex(ims_recv_buffer, ims_recv_byte_num);

  // Assume only one message
  msg->payload = ims_recv_buffer;
  msg->size = ims_recv_byte_num;

  return ret;

}

/*========================================================
  FUNCTION: handle_ims_msg
    - decode incoming ims message
  Critical section access:
  - exec_q_access is expected to be locked prior to calling
    this function.
========================================================*/
void handle_ims_msg(int size, uint8_t *payload)
{
  RIL_UTF_DEBUG("\nENTERING FUNCTION %s", __func__);
  enum ril_utf_q_element_t type;
  ims_MsgTag *msg_tag_ptr = NULL;
  void *msg_data_ptr = NULL;

  size_t tag_size = payload[0];
  size_t packed_msg_size = size - 1 - tag_size;
  size_t unpacked_msg_size = 0;

  msg_tag_ptr = ril_utf_ims_unpack_msg_tag(payload);

  ril_utf_ims_parse_packed_msg(msg_tag_ptr->type, msg_tag_ptr->id, payload+tag_size+1, packed_msg_size, &msg_data_ptr, &unpacked_msg_size);

  if ( msg_tag_ptr->type == ims_MsgType_RESPONSE )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: received ims_MsgType_RESPONSE");
    type = ril_utf_ims_response;
  }
  else if ( msg_tag_ptr->type == ims_MsgType_UNSOL_RESPONSE )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: received ims_MsgType_UNSOL_RESPONSE");
    type = ril_utf_ims_indication;
  }
  else
  {
    RIL_UTF_ESSENTIAL("\nError: Received a message of unkown type in %s", __func__);
    return;
  }

  RIL_UTF_ESSENTIAL("\nQMI_RIL_UTF: received IMS MESSAGE ID %d TOKEN ID %d", msg_tag_ptr->id, msg_tag_ptr->token);
  RIL_UTF_ESSENTIAL("\nQMI_RIL_UTF: message payload len is %d", unpacked_msg_size);

  // Add message to queue for test thread
  enqueue_ims_expect(unpacked_msg_size, msg_data_ptr, msg_tag_ptr->id, msg_tag_ptr->token, type, msg_tag_ptr->error);

  free(msg_tag_ptr);

  RIL_UTF_DEBUG("\nEXITING FUNCTION %s", __func__);
}

size_t ril_utf_ims_pack_msg(const void *msg, ims_MsgType msg_type, ims_MsgId msg_id, uint8_t *buf, size_t max_buf_size)
{
  size_t ret = 0;

  if (NULL == buf)
  {
      RIL_UTF_DEBUG("buf is NULL");
      return ret;
  }
  RIL_UTF_DEBUG("message id: %d, type: %d", msg_id, msg_type);
  if (NULL == msg)
  {
      RIL_UTF_DEBUG("msg is NULL");
      return ret;
  }

  pb_field_t* fields = (pb_field_t *)qcril_qmi_ims_get_msg_pb_fields(msg_id, msg_type);

  if (fields)
  {
    pb_ostream_t os;
    os = pb_ostream_from_buffer(buf, max_buf_size);
    if (!qcril_qmi_encode_npb(&os, fields, msg))
    {
      RIL_UTF_ESSENTIAL("pb_encode failed");
      return ret;
    }
    ret = os.bytes_written;
  }
  else
  {
    RIL_UTF_DEBUG("no payload for message %s", qcril_qmi_ims_get_msg_log_str(msg_id, msg_type));
  }

  return ret;
}

int ril_utf_ims_parse_packed_msg(ims_MsgType msg_type, ims_MsgId msg_id, const uint8_t *byte_stream, size_t byte_stream_size,
                                     void** ims_msg_dptr, size_t* ims_msg_size_ptr)
{
  *ims_msg_dptr = NULL;
  *ims_msg_size_ptr = 0;

  if ( byte_stream == NULL )
  {
      RIL_UTF_ESSENTIAL("packed message is null");
      return -1;
  }

  const pb_field_t* fields = qcril_qmi_ims_get_msg_pb_fields(msg_id, msg_type);
  if (fields && qcril_qmi_ims_get_msg_size(msg_id, msg_type))
     {
         *ims_msg_dptr = qcril_malloc(qcril_qmi_ims_get_msg_size(msg_id, msg_type));
         if (*ims_msg_dptr)
         {
          pb_istream_t istream;
          istream = pb_istream_from_buffer((uint8_t *)byte_stream, byte_stream_size);
          if (qcril_qmi_decode_npb(&istream, fields, *ims_msg_dptr))
          {
            *ims_msg_size_ptr = qcril_qmi_ims_get_msg_size(msg_id, msg_type);
          }
         }
  }
  return 0;
}

ims_MsgTag* ril_utf_ims_unpack_msg_tag(uint8_t *buf)
{
  ims_MsgTag* msg_tag_ptr = NULL;

  if (NULL == buf)
  {
    RIL_UTF_ESSENTIAL("buf is NULL");
  }

  msg_tag_ptr = (ims_MsgTag *)malloc(sizeof(*msg_tag_ptr));
  if (msg_tag_ptr)
  {
    size_t msg_tag_size = 0;
    msg_tag_size = buf[0];
    pb_istream_t is;
    is = pb_istream_from_buffer(&buf[1], msg_tag_size);
    if (!qcril_qmi_decode_npb(&is, ims_MsgTag_fields, msg_tag_ptr))
    {
      RIL_UTF_ESSENTIAL("pb_decode failed");
      free(msg_tag_ptr);
      return NULL;
    }
  }

  return msg_tag_ptr;
}

/*
size_t ril_utf_ims_pack_msg(const void *msg, Ims__MsgType type, Ims__MsgId message_id, uint8_t *buf, size_t max_buf_size)
{
  size_t ret = 0;

  if (NULL == buf)
  {
      RIL_UTF_DEBUG("buf is NULL");
      return ret;
  }

  RIL_UTF_DEBUG("message id: %d, type: %d", message_id, type);
  if (NULL == msg)
  {
    RIL_UTF_DEBUG("msg is NULL");
    return ret;
  }

  if (ims_MsgType_REQUEST == type)
  {
    if (IMS__MSG_ID__REQUEST_DIAL == message_id)
    {
      ret = ims__dial__get_packed_size((Ims__Dial *)msg);

      if (ims__dial__pack((Ims__Dial*)msg, buf) != ret)
      {
          RIL_UTF_DEBUG("get_packed_size is different from pack size");
      }
    }
    else if (IMS__MSG_ID__REQUEST_ANSWER == message_id)
    {
      ret = ims__answer__get_packed_size((Ims__Answer*)msg);

      if (ims__answer__pack((Ims__Answer*)msg, buf) != ret)
      {
          RIL_UTF_DEBUG("get_packed_size is different from pack size");
      }
    }
    else if (IMS__MSG_ID__REQUEST_HANGUP == message_id)
    {
      ret = ims__hangup__get_packed_size((Ims__Hangup*)msg);

      if (ims__hangup__pack((Ims__Hangup*)msg, buf) != ret)
      {
          RIL_UTF_DEBUG("get_packed_size is different from pack size");
      }
    }
    else if (IMS__MSG_ID__REQUEST_DTMF == message_id)
    {
      ret = ims__dtmf__get_packed_size((Ims__Dtmf*)msg);

      if (ims__dtmf__pack((Ims__Dtmf*)msg, buf) != ret)
      {
          RIL_UTF_DEBUG("get_packed_size is different from pack size");
      }
    }
    else if ( IMS__MSG_ID__REQUEST_DTMF_START == message_id)
    {
      ret = ims__dtmf__get_packed_size((Ims__Dtmf*)msg);

      if (ims__dtmf__pack((Ims__Dtmf*)msg, buf) != ret)
      {
          RIL_UTF_DEBUG("get_packed_size is different from pack size");
      }
    }
    else if (IMS__MSG_ID__REQUEST_MODIFY_CALL_INITIATE == message_id)
    {
      ret = ims__call_modify__get_packed_size((Ims__CallModify*)msg);

      if (ims__call_modify__pack((Ims__CallModify*)msg, buf) != ret)
      {
          RIL_UTF_DEBUG("get_packed_size is different from pack size");
      }
    }
    else if (IMS__MSG_ID__REQUEST_MODIFY_CALL_CONFIRM == message_id)
    {
      ret = ims__call_modify__get_packed_size((Ims__CallModify*)msg);

      if (ims__call_modify__pack((Ims__CallModify*)msg, buf) != ret)
      {
          RIL_UTF_DEBUG("get_packed_size is different from pack size");
      }
    }
    else if (IMS__MSG_ID__REQUEST_SET_CLIR  == message_id)
    {
      ret = ims__clir__get_packed_size((Ims__Clir*)msg);

      if (ims__clir__pack((Ims__Clir*)msg, buf) != ret)
      {
          RIL_UTF_DEBUG("get_packed_size is different from pack size");
      }
    }
    else if (IMS__MSG_ID__REQUEST_SET_CALL_FORWARD_STATUS == message_id ||
             IMS__MSG_ID__REQUEST_QUERY_CALL_FORWARD_STATUS == message_id)
    {
      ret = ims__call_forward_info_list__get_packed_size((Ims__CallForwardInfoList*)msg);

      if (ims__call_forward_info_list__pack((Ims__CallForwardInfoList*)msg, buf) != ret)
      {
          RIL_UTF_DEBUG("get_packed_size is different from pack size");
      }
    }
    else if (IMS__MSG_ID__REQUEST_QUERY_CALL_WAITING == message_id)
    {
      ret = ims__service_class__get_packed_size((Ims__ServiceClass*)msg);

      if (ims__service_class__pack((Ims__ServiceClass*)msg, buf) != ret)
      {
          RIL_UTF_DEBUG("get_packed_size is different from pack size");
      }
    }
    else if (IMS__MSG_ID__REQUEST_SET_CALL_WAITING == message_id)
    {
      ret = ims__call_waiting_info__get_packed_size((Ims__CallWaitingInfo*)msg);

      if (ims__call_waiting_info__pack((Ims__CallWaitingInfo*)msg, buf) != ret)
      {
          RIL_UTF_DEBUG("get_packed_size is different from pack size");
      }
    }
    else if (IMS__MSG_ID__REQUEST_SET_SUPP_SVC_NOTIFICATION == message_id)
    {
      ret = ims__supp_svc_status__get_packed_size((Ims__SuppSvcStatus*)msg);

      if (ims__supp_svc_status__pack((Ims__SuppSvcStatus*)msg, buf) != ret)
      {
          RIL_UTF_DEBUG("get_packed_size is different from pack size");
      }
    }
    else if (IMS__MSG_ID__REQUEST_IMS_REG_STATE_CHANGE == message_id)
    {
      ret = ims__registration__get_packed_size((Ims__Registration*)msg);

      if (ims__registration__pack((Ims__Registration*)msg, buf) != ret)
      {
          RIL_UTF_DEBUG("get_packed_size is different from pack size");
      }
    }
    else if (IMS__MSG_ID__REQUEST_ADD_PARTICIPANT == message_id)
    {
      ret = ims__dial__get_packed_size((Ims__Dial *)msg);

      if (ims__dial__pack((Ims__Dial*)msg, buf) != ret)
      {
          RIL_UTF_DEBUG("get_packed_size is different from pack size");
      }
    }
    else if (IMS__MSG_ID__REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE == message_id)
    {
      ret = ims__switch_waiting_or_holding_and_active__get_packed_size((Ims__Dial *)msg);

      if (ims__switch_waiting_or_holding_and_active__pack((Ims__Dial*)msg, buf) != ret)
      {
          RIL_UTF_DEBUG("get_packed_size is different from pack size");
      }
    }
    else if (IMS__MSG_ID__REQUEST_SET_SERVICE_STATUS == message_id)
    {
      ret = ims__info__get_packed_size((Ims__Info *)msg);

      if (ims__info__pack((Ims__Info*)msg, buf) != ret)
      {
          RIL_UTF_DEBUG("get_packed_size is different from pack size");
      }
    }
    else if (IMS__MSG_ID__REQUEST_SUPP_SVC_STATUS == message_id)
    {
      ret = ims__supp_svc_request__get_packed_size((Ims__SuppSvcRequest *)msg);

      if (ims__supp_svc_request__pack((Ims__SuppSvcRequest*)msg, buf) != ret)
      {
          RIL_UTF_DEBUG("get_packed_size is different from pack size");
      }
    }
#ifndef UTF_TARGET_BUILD
    else if (IMS__MSG_ID__REQUEST_SET_VT_CALL_QUALITY == message_id)
    {
      ret = ims__video_call_quality__get_packed_size((Ims__VideoCallQuality *)msg);

      if (ims__video_call_quality__pack((Ims__VideoCallQuality *)msg, buf) != ret)
      {
          RIL_UTF_DEBUG("get_packed_size is different from pack size");
      }
    }
    else if (IMS__MSG_ID__REQUEST_SET_COLR == message_id)
    {
      ret = ims__colr__get_packed_size((Ims__Colr *)msg);

      if (ims__colr__pack((Ims__Colr *)msg, buf) != ret)
      {
          RIL_UTF_DEBUG("get_packed_size is different from pack size");
      }
    }
#endif
#ifdef RIL_UTF_L_MASTER
    else if (IMS__MSG_ID__REQUEST_SEND_UI_TTY_MODE == message_id)
#else
    else if (IMS__MSG_ID__REQUEST_SET_TTY_MODE == message_id)
#endif
    {
      ret = ims__tty_notify__get_packed_size((Ims__TtyNotify *)msg);

      if (ims__tty_notify__pack((Ims__TtyNotify *)msg, buf) != ret)
      {
          RIL_UTF_DEBUG("get_packed_size is different from pack size");
      }
    }
#ifdef RIL_UTF_L_MASTER
    else if (IMS__MSG_ID__REQUEST_HOLD == message_id)
    {
      ret = ims__hold__get_packed_size((Ims__Hold *)msg);

      if (ims__hold__pack((Ims__Hold *)msg, buf) != ret)
      {
          RIL_UTF_DEBUG("get_packed_size is different from pack size");
      }
    }
    else if (IMS__MSG_ID__REQUEST_RESUME == message_id)
    {
      ret = ims__resume__get_packed_size((Ims__Resume *)msg);

      if (ims__resume__pack((Ims__Resume *)msg, buf) != ret)
      {
          RIL_UTF_DEBUG("get_packed_size is different from pack size");
      }
    }
    else if (IMS__MSG_ID__REQUEST_SET_WIFI_CALLING_STATUS == message_id)
    {
      ret = ims__wifi_calling_info__get_packed_size((Ims__WifiCallingInfo *)msg);

      if (ims__wifi_calling_info__pack((Ims__WifiCallingInfo *)msg, buf) != ret)
      {
          RIL_UTF_DEBUG("get_packed_size is different from pack size");
      }
    }
    else if (ims_MsgId_REQUEST_SET_IMS_CONFIG == message_id ||
             ims_MsgId_REQUEST_GET_IMS_CONFIG == message_id)
    {
      ret = ims__config_msg__get_packed_size((ims_ConfigMsg *)msg);

      if (ims__config_msg__pack((ims_ConfigMsg *)msg, buf) != ret)
      {
          RIL_UTF_DEBUG("get_packed_size is different from pack size");
      }
    }
#endif
  }

  return ret;
}

ims_MsgTag* ril_utf_ims_unpack_msg_tag(const uint8_t *buf)
{
    ims_MsgTag* msg_tag_ptr = NULL;
    size_t msg_tag_size = 0;

    if (NULL == buf)
    {
        RIL_UTF_ESSENTIAL("buf is NULL in %s",__func__);
    }
    else
    {
        msg_tag_size = buf[0];
        msg_tag_ptr = qcril_qmi_ims__msg_tag__unpack (NULL, msg_tag_size, &buf[1]);
    }

    return msg_tag_ptr;
} // ril_utf_ims_unpack_msg_tag

void ril_utf_ims_parse_packed_msg(Ims__MsgType type, Ims__MsgId message_id, const uint8_t *packed_msg, size_t packed_msg_size,
                                     void** unpacked_msg, size_t*unpacked_msg_size_ptr)
{
    *unpacked_msg = NULL;
    *unpacked_msg_size_ptr = 0;


    if (packed_msg_size == 0)
    {
        RIL_UTF_ESSENTIAL("packed msg size is 0");
    }
    else
    {
        RIL_UTF_DEBUG("message id: %d, type: %d", message_id, type);

        if ( ims_MsgType_RESPONSE == type || ims_MsgType_UNSOL_RESPONSE == type)
        {
#ifdef RIL_UTF_L_MASTER
          if (IMS__MSG_ID__UNSOL_RESPONSE_CALL_STATE_CHANGED == message_id)
          {
              *unpacked_msg = (void*)ims__call_list__unpack(NULL, packed_msg_size, packed_msg);
              *unpacked_msg_size_ptr = sizeof(Ims__CallList);
          }
#else
          if (IMS__MSG_ID__REQUEST_GET_CURRENT_CALLS == message_id)
          {
              *unpacked_msg = (void*)ims__call_list__unpack(NULL, packed_msg_size, packed_msg);
              *unpacked_msg_size_ptr = sizeof(Ims__CallList);
          }
#endif
          else if (IMS__MSG_ID__REQUEST_IMS_REGISTRATION_STATE == message_id)
          {
              *unpacked_msg = (void*)ims__registration__unpack(NULL, packed_msg_size, packed_msg);
              *unpacked_msg_size_ptr = sizeof(Ims__Registration);
          }
          else if (IMS__MSG_ID__REQUEST_LAST_CALL_FAIL_CAUSE == message_id)
          {
              *unpacked_msg = (void*)ims__call_fail_cause_response__unpack(NULL, packed_msg_size, packed_msg);
              *unpacked_msg_size_ptr = sizeof(Ims__CallFailCauseResponse);
          }
          else if (IMS__MSG_ID__REQUEST_GET_CLIR == message_id)
          {
              *unpacked_msg = (void*)ims__clir__unpack(NULL, packed_msg_size, packed_msg);
              *unpacked_msg_size_ptr = sizeof(Ims__Clir);
          }
          else if (IMS__MSG_ID__REQUEST_QUERY_CLIP == message_id)
          {
              *unpacked_msg = (void*)ims__clip_provision_status__unpack
                (NULL, packed_msg_size, packed_msg);
              *unpacked_msg_size_ptr = sizeof(Ims__ClipProvisionStatus);
          }
          else if (IMS__MSG_ID__REQUEST_QUERY_CALL_FORWARD_STATUS == message_id)
          {
              *unpacked_msg = (void*)ims__call_forward_info_list__unpack(NULL, packed_msg_size, packed_msg);
              *unpacked_msg_size_ptr = sizeof(Ims__CallForwardInfoList);
          }
          else if (IMS__MSG_ID__REQUEST_QUERY_CALL_WAITING == message_id)
          {
              *unpacked_msg = (void*)ims__call_waiting_info__unpack(NULL, packed_msg_size, packed_msg);
              *unpacked_msg_size_ptr = sizeof(Ims__CallWaitingInfo);
          }
          else if (IMS__MSG_ID__REQUEST_SET_SUPP_SVC_NOTIFICATION  == message_id)
          {
              *unpacked_msg = (void*)ims__supp_svc_notification__unpack(NULL, packed_msg_size, packed_msg);
              *unpacked_msg_size_ptr = sizeof(Ims__SuppSvcNotification);
          }
          else if (IMS__MSG_ID__UNSOL_RINGBACK_TONE == message_id)
          {
              *unpacked_msg = (void*)ims__ring_back_tone__unpack(NULL, packed_msg_size, packed_msg);
              *unpacked_msg_size_ptr = sizeof(Ims__RingBackTone);
          }
          else if (IMS__MSG_ID__UNSOL_SUPP_SVC_NOTIFICATION == message_id)
          {
              *unpacked_msg = (void*)ims__supp_svc_notification__unpack(NULL, packed_msg_size, packed_msg);
              *unpacked_msg_size_ptr = sizeof(Ims__SuppSvcNotification);
          }
          else if (IMS__MSG_ID__UNSOL_MODIFY_CALL == message_id)
          {
              *unpacked_msg = (void*)ims__call_modify__unpack(NULL, packed_msg_size, packed_msg);
              *unpacked_msg_size_ptr = sizeof(Ims__CallModify);
          }
          else if (IMS__MSG_ID__UNSOL_RESPONSE_HANDOVER == message_id)
          {
              *unpacked_msg = (void*)ims__handover__unpack(NULL, packed_msg_size, packed_msg);
              *unpacked_msg_size_ptr = sizeof(Ims__Handover);
          }
          else if (IMS__MSG_ID__UNSOL_REFRESH_CONF_INFO == message_id)
          {
              *unpacked_msg = (void*)ims__conf_info__unpack(NULL, packed_msg_size, packed_msg);
              *unpacked_msg_size_ptr = sizeof(Ims__ConfInfo);
          }
          else if (IMS__MSG_ID__UNSOL_SRV_STATUS_UPDATE == message_id)
          {
              *unpacked_msg = (void*)ims__srv_status_list__unpack(NULL, packed_msg_size, packed_msg);
              *unpacked_msg_size_ptr = sizeof(Ims__SrvStatusList);
          }
          #ifndef UTF_TARGET_BUILD
          else if (IMS__MSG_ID__UNSOL_MWI == message_id)
          {
              *unpacked_msg = (void*)ims__mwi__unpack(NULL, packed_msg_size, packed_msg);
              *unpacked_msg_size_ptr = sizeof(Ims__Mwi);
          }
          #endif
          else if (IMS__MSG_ID__REQUEST_QUERY_SERVICE_STATUS == message_id)
          {
              *unpacked_msg = (void*)ims__srv_status_list__unpack(NULL, packed_msg_size, packed_msg);
              *unpacked_msg_size_ptr = sizeof(Ims__SrvStatusList);
          }
          else if (IMS__MSG_ID__REQUEST_SUPP_SVC_STATUS == message_id)
          {
              *unpacked_msg = (void*)ims__supp_svc_response__unpack(
                      NULL, packed_msg_size, packed_msg);
              *unpacked_msg_size_ptr = sizeof(Ims__SuppSvcResponse);
          }
#ifdef RIL_UTF_L_MASTER
          else if (IMS__MSG_ID__REQUEST_GET_WIFI_CALLING_STATUS == message_id)
          {
              *unpacked_msg = (void*)ims__wifi_calling_info__unpack(NULL, packed_msg_size, packed_msg);
              *unpacked_msg_size_ptr = sizeof(Ims__WifiCallingInfo);
          }
#endif
#ifndef UTF_TARGET_BUILD
          else if (IMS__MSG_ID__REQUEST_QUERY_VT_CALL_QUALITY == message_id)
          {
              *unpacked_msg = (void*)ims__video_call_quality__unpack(NULL, packed_msg_size, packed_msg);
              *unpacked_msg_size_ptr = sizeof(Ims__VideoCallQuality);
          }
          else if (IMS__MSG_ID__REQUEST_GET_COLR == message_id)
          {
              *unpacked_msg = (void*)ims__colr__unpack(NULL, packed_msg_size, packed_msg);
              *unpacked_msg_size_ptr = sizeof(Ims__Colr);
          }
          else if (ims_MsgId_REQUEST_SET_IMS_CONFIG == message_id ||
                   ims_MsgId_REQUEST_GET_IMS_CONFIG == message_id)
          {
              *unpacked_msg = (void*)ims__config_msg__unpack(NULL, packed_msg_size, packed_msg);
              *unpacked_msg_size_ptr = sizeof(ims_ConfigMsg);
          }
          else
          {
            // should never reach
            RIL_UTF_ESSENTIAL("\nERROR: Received payload but is of unknown type");
          }
#endif
        }
    }
}
*/

/*========================================================
  FUNCTION: enqueue_ims_expect
    - Add received ims message to queue for test thread
  Critical section access:
  - exec_q_access is expected to be locked prior to calling
    this function.
========================================================*/
void enqueue_ims_expect(int size, void *payload, int msg_id,
                        int token, enum ril_utf_q_element_t type, ims_Error error)
{
  struct ril_utf_msg_t *expect_node =
    (struct ril_utf_msg_t *) malloc(sizeof(struct ril_utf_msg_t));
  expect_node->payload_len = size;
  expect_node->payload = payload;
  expect_node->msg_id = msg_id;
  expect_node->token_id = token;
  expect_node->type = type;
  expect_node->ims_error = error;
  enqueue_ril_utf_q_node(expect_node, &ril_utf_rec_head, &ril_utf_rec_tail);
}

void ril_utf_ims_recv_thread_shutdown()
{
  RIL_UTF_DEBUG("\nQMI_RIL_UTF: Executing ims recv thread shutdown");
  send(qmi_ril_ims_sock_id, "reset", (strlen("reset")+1), 0);
  close(qmi_ril_ims_sock_id);
  pthread_exit(NULL);
}

static void ril_utf_free_ims_payload_memory(struct ril_utf_msg_t *test_step)
{
  if (!test_step)
    return;
  if (test_step->payload) {
    switch ((ims_MsgId)(test_step->msg_id)) {
    case ims_MsgId_REQUEST_SEND_SMS:
    {
        auto ims_sms = static_cast<RIL_IMS_SMS_Message*>(test_step->payload);
        if (ims_sms->tech == RADIO_TECH_3GPP) {
            if (ims_sms->message.gsmMessage) {
                if (ims_sms->message.gsmMessage[0])
                    free(ims_sms->message.gsmMessage[0]);
                if (ims_sms->message.gsmMessage[1])
                    free(ims_sms->message.gsmMessage[1]);
                free(ims_sms->message.gsmMessage);
            }
        } else if (ims_sms->tech == RADIO_TECH_3GPP2) {
            if (ims_sms->message.cdmaMessage)
                free(ims_sms->message.cdmaMessage);
        }
    }
    break;
    default:
        if (test_step->payload) {
            free(test_step->payload);
            test_step->payload = NULL;
        }
    }
  }
  free(test_step);
}

int send_ril_utf_ims_request( struct ril_utf_msg_t *test_step )
{
  char send_buf1[RIL_UTF_IMS_SOCKET_MAX_BUF_SIZE];
  int err_val = QCRIL_UNITTTEST_NO_ERR;
  int sig_err;

  send_i_ims_radio_request(ims_MsgType_REQUEST,
      (ims_MsgId)(test_step->msg_id),
      test_step->token_id,
      test_step->payload);

  // Inform test thread we are finished
  RIL_UTF_DEBUG("\n QMI_RIL_UTF: Setting inject_complete to TRUE in %s", __func__);
  inject_complete = 1;
  pthread_cond_signal(&inject_complete_sig);


  ril_utf_free_ims_payload_memory(test_step);
  return err_val;
}

//=========================================================================
// FUNCTION: ims_unpack_int_bigendian
//
// DESCRIPTION: unpack a 4 byte value in big-endian
//
// RETURN: unpacked data as an int
//=========================================================================
//
int ims_unpack_int_bigendian(char* data) {

   int value = 0;
   value += ((unsigned char)data[0]) << 24;
   value += ((unsigned char)data[1]) << 16;
   value += ((unsigned char)data[2]) << 8;
   value += ((unsigned char)data[3]);

   return value;
}
