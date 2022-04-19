/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_oem_sim.c
  @brief   RIL UTF OEM simulation and endpoint handling

  DESCRIPTION
   Go between RIL UTF test and QMI OEM socket
  ---------------------------------------------------------------------------
******************************************************************************/
#include "ril_utf_i_qti_oem_hook_sim.h"
#include "ril_utf_oem_sim.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
#include "qcril_qmi_lte_direct_disc_packing.h"
#include "qcril_qmi_lte_direct_disc_msg_meta.h"
#include "qcril_otheri.h"
#ifdef QMI_RIL_HAL_UTF
#include "qcril_qmi_oem_reqlist.h"
#endif
// Globals
extern struct ril_utf_expect_t *ril_utf_rec_head;
extern struct ril_utf_expect_t *ril_utf_rec_tail;
extern pthread_t oem_send_id;
extern pthread_t oem_recv_id;
extern int oem_inject_pending;
extern int oem_expect_ready;
extern pthread_cond_t wakeup_oem_send_thread;
extern pthread_cond_t wakeup_oem_recv_thread;
// MSIM RIL instance
extern int rild_instance;


typedef enum _Oem__MsgType {
    OEM__MSG_TYPE__RESPONSE       = 0,
    OEM__MSG_TYPE__UNSOL_RESPONSE = 1
} Oem_MsgType;

struct token_event {
  int oem_event_id;
  int token_id;
  int valid;
};

struct token_event oem_request_list[50];

// Local definitions
#define UTF_QMI_OEM_SOCKET_PATH "./oem_connect_socket0"
#define UTF_QMI_OEM_SECONDARY_SOCKET_PATH "./oem_connect_socket1"

#define UTF_OEM_SOCKET_CONNECT_MAX_ATTEMPTS 10
#define MAX_OEM_MESSAGES 10
#define RIL_UTF_OEM_MAX_BUFFER_BYTES_PER_LINE                16
#define RIL_UTF_OEM_MAX_OUTPUT_BUF_SIZE                      ( ( RIL_UTF_OEM_MAX_BUFFER_BYTES_PER_LINE * 5 ) + 2 )

uint8_t oem_recv_buffer[RIL_UTF_OEM_SOCKET_MAX_BUF_SIZE];
int oem_recv_byte_num;

int qmi_oem_sock_id;

OEMmsgQ *oem_inject_msg_q_head = NULL;
OEMmsgQ *oem_inject_msg_q_tail = NULL;

OEMmsgQ *oem_expect_msg_q_head = NULL;
OEMmsgQ *oem_expect_msg_q_tail = NULL;

/*========================================================
  FUNCTION: utf_start_oem_threads
    - starts the following threads:
      - utf_oem_send_start
      - utf_oem_recv_start
========================================================*/
void utf_start_oem_threads()
{
  pthread_attr_t oem_send_thrd_attr;
  pthread_attr_t oem_recv_thrd_attr;
  int ret;

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Connecting to QMI OEM socket...");
  if ( connect_to_ril_oem_socket() != TRUE )
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error: Could not connect to QMI OEM socket. check error msg");
  }

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Starting QMI OEM send thread...");
  pthread_attr_init(&oem_send_thrd_attr);
  ret = pthread_create(&oem_send_id, &oem_send_thrd_attr, utf_oem_send_start, NULL);

  if ( ret < 0 )
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Failed to start oem send thread. Exiting...");
    return;
  }

#if 0

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Starting QMI OEM recv thread...");
  pthread_attr_init(&oem_recv_thrd_attr);
  ret = pthread_create(&oem_recv_id, &oem_recv_thrd_attr, utf_oem_recv_start, NULL);

  if ( ret < 0 )
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Failed to start oem recv thread. Exiting...");
    return;
  }
#endif

}

/*========================================================
  FUNCTION: connect_to_qmi_oem_socket
    - Attempts to make a connection to the exported
      socket from qmi oem
========================================================*/
int connect_to_ril_oem_socket(void)
{
  int sock_conn_length;
  int utf_oem_sock_conn_max_attempts = UTF_OEM_SOCKET_CONNECT_MAX_ATTEMPTS;

  struct sockaddr_un qmi_oem_addr;

  connect_to_i_qti_oem_hook();

  if ( ( qmi_oem_sock_id = socket(AF_UNIX, SOCK_STREAM, 0) ) < 0)
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: !!! Error !!! Could not create socket Exiting...");
    return FALSE;
  }

  memset(&qmi_oem_addr, 0x00, sizeof(qmi_oem_addr));

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Trying to connect to qmi oem socket %s",UTF_QMI_OEM_SOCKET_PATH);

  qmi_oem_addr.sun_family = AF_UNIX;
  #ifdef RIL_UTF_L_MASTER
  if (rild_instance == 1)
  #else
  if (rild_instance == 0)
  #endif
  {
    strlcpy(qmi_oem_addr.sun_path, UTF_QMI_OEM_SOCKET_PATH, sizeof(qmi_oem_addr.sun_path) - 1);
  }
  else
  {
    strlcpy(qmi_oem_addr.sun_path, UTF_QMI_OEM_SECONDARY_SOCKET_PATH, sizeof(qmi_oem_addr.sun_path) - 1);
  }

  sock_conn_length = strlen(qmi_oem_addr.sun_path) + sizeof(qmi_oem_addr.sun_family);


  while ( utf_oem_sock_conn_max_attempts )
  {
    if ( connect(qmi_oem_sock_id, (struct sockaddr*) &qmi_oem_addr, sock_conn_length) < 0 )
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: !!! Error !!!Failed to qmi oem socket.");
      utf_oem_sock_conn_max_attempts--;

      if ( !utf_oem_sock_conn_max_attempts )
      {
        RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: !!! Error !!!Failed to qmi oem socket after %d attempts.", UTF_OEM_SOCKET_CONNECT_MAX_ATTEMPTS);
        return FALSE;
      }
    }
    else
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Connected to qmi oem socket.");
      break;
    }
  }


  return TRUE;
}

/*========================================================
  FUNCTION: utf_oem_send_start
  - Starts the utf oem send thread.
  - will wait for signal from UTF main test exec
    for an "inject" to send a message to qmi oem.

  Critical section access:
  - exec_q_access locked and unlocked here.
  - Note: pthread_wait will release exec_q_access
    when waiting, and locks again upon re-entering.
========================================================*/
void* utf_oem_send_start( void *param)
{
  struct timeval e_timeout;
  struct timespec timeout;
  e_timeout.tv_sec = 0;
  e_timeout.tv_usec = 10;

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Starting QMI OEM send thread...");

  // Acquire Q lock
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: OEM send thread locking exec_q_access in %s", __func__);
  EXEC_Q_LOCK();

  while( EXEC_OEM_THREAD )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Exec...proxy thread");
    // Wait for items to be placed in Q
    while ( !oem_inject_pending )
    {
      calc_timeout(&timeout, &e_timeout);
      //RIL_UTF_DEBUG("\n QMI_RIL_UTL: Waiting for oem request message in %s", __func__);
      pthread_cond_timedwait(&wakeup_oem_send_thread, &exec_q_access, &timeout);

      // Check for reboot request
      if (ril_utf_check_reboot_state() == 1)
      {
        ril_utf_oem_send_thread_shutdown();
      }
    }
    EXEC_Q_UNLOCK();

    RIL_UTF_DEBUG("\n notify inject_complete_sig\n");

    // Inform test thread we are finished
    inject_complete = 1;
    pthread_cond_signal(&inject_complete_sig);

    process_oem_inject_queue();

    oem_inject_pending = FALSE;
  }

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: OEM send thread unlocking exec_q_access in %s", __func__);
  EXEC_Q_UNLOCK();

}

/*========================================================
  FUNCTION: process_oem_inject_queue
  - Processes the OEM inject queue.
  -   Take elements in the queue and send them to
      QMI OEM socket.

  Critical section access:
  - exec_q_access must be locked prior to calling this
    function.
========================================================*/
void process_oem_inject_queue(void)
{
  utf_oem_msg_q_payload *oem_msg_to_send = (utf_oem_msg_q_payload *) qcril_malloc(sizeof(utf_oem_msg_q_payload));
  int   oem_msg_to_send_len = 0;

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: processing QMI OEM inject queue.");

  while( oem_inject_msg_q_tail )
  {
    memset(oem_msg_to_send, 0x00, sizeof(utf_oem_msg_q_payload));

    RIL_UTF_DEBUG("\n QMI_RIL_UTL: getting message to send to QMI OEM socket.");
    if (get_oem_msg_to_send(oem_msg_to_send))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: getting message to send to QMI OEM socket.");
      ril_utf_send_oem_message(oem_msg_to_send);
    }
    else
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Warning! No QMI message to inject.");
    }
  }
}

/*========================================================
  FUNCTION: ril_utf_send_oem_message
  - Send message to qmi oem socket

  Critical section access:
  - exec_q_access must be locked prior to calling this
    function.
========================================================*/
void ril_utf_send_oem_message(utf_oem_msg_q_payload *msg)
{
  RIL_UTF_ESSENTIAL("\n calling send_i_oemhook_request");
  send_i_oemhook_request(msg->token, msg->msg, msg->msg_len);
#if 0
  if (send(qmi_oem_sock_id, (void *) msg, len, 0) < 0)
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error! Failed to send mesage to qmi oem socket.");
  }
#endif
}

/*========================================================
  FUNCTION: get_oem_msg_to_send
  -  Get's the QMI OEM message to send from the
     OEM message queue
  - Returns true if there is a message to send
    and false if the queue is empty.

  Critical section access:
  - exec_q_access must be locked prior to calling this
    function.
========================================================*/
int get_oem_msg_to_send(utf_oem_msg_q_payload *msg)
{
  int ret_val = FALSE;

  if( oem_inject_msg_q_tail )
  {
    //copy the message and len to send.
    memcpy(msg, oem_inject_msg_q_tail->payload, oem_inject_msg_q_tail->payload_len);

    //Remove from queue
    OEMmsgQ *prev = oem_inject_msg_q_tail->prev;
    if ( prev )
    {
      prev->next = NULL;
    }
    free(oem_inject_msg_q_tail->payload);
    free(oem_inject_msg_q_tail);
    oem_inject_msg_q_tail = prev;
    if (oem_inject_msg_q_tail == NULL)
    {
      oem_inject_msg_q_head = NULL;
    }

    ret_val = TRUE;
  }
  else
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: OEM inject queue empty");
    ret_val = FALSE;
  }

  return ret_val;
}

/*========================================================
  FUNCTION: enqueue_oem_msg_to_send

  Critical section access:
  - exec_q_access must be locked prior to calling this
    function.
========================================================*/
void enqueue_oem_msg_to_send(void *msg, int len)
{
  OEMmsgQ *new_msg = (OEMmsgQ *) malloc(sizeof(OEMmsgQ));
  memset(new_msg, 0x00, sizeof(OEMmsgQ));
  new_msg->payload = msg;
  new_msg->payload_len = len;

  if( oem_inject_msg_q_head )
  {
    new_msg->prev = NULL;
    new_msg->next = oem_inject_msg_q_head;
    oem_inject_msg_q_head->prev = new_msg;
  }
  else
  {
    oem_inject_msg_q_tail = new_msg;
  }

  oem_inject_msg_q_head = new_msg;
}

/*========================================================
  FUNCTION: utf_oem_recv_start
  - Starts the utf oem recv thread.

  Critical section access:
  - exec_q_access locked and unlocked here.
  - Note: pthread_wait will release exec_q_access
    when waiting, and locks again upon re-entering.
========================================================*/
void* utf_oem_recv_start( void *param)
{
  fd_set rfds;
  int ret_val;
  int i, my_expect_slot, x;
  struct oem_buff_t msg;
  struct timespec timeout;
  struct timeval e_timeout;
  struct timespec thread_timeout;
  e_timeout.tv_sec = 0;
  e_timeout.tv_usec = EXPECT_TIMEOUT;

  while( EXEC_OEM_THREAD )
  {
    memset(&msg, 0, sizeof(msg));
    //RIL_UTF_DEBUG("\n QMI_RIL_UTL: Exec...oem recv thread");

    FD_ZERO(&rfds);
    FD_SET(qmi_oem_sock_id, &rfds);
    timeout.tv_sec = 0;
    timeout.tv_nsec = 10;

    ret_val = select(qmi_oem_sock_id+1, &rfds, NULL, NULL, &timeout);

    if ( ret_val == -1 )
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: OEM recv thread error in select in func %s", __func__);
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Select returns error %d", ret_val);
      return NULL;
    }
    else if ( ret_val )
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: %s: OEM recv thread receiving message", __func__);
      utf_oem_recv_msg(&msg);

      // acquire mutex
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: OEM recv thread locking exec_q_access in %s", __func__);
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
        if (ril_utf_check_reboot_state() == 1)
        {
          EXEC_Q_UNLOCK();
          pthread_exit(NULL);
        }
      }

      // Decode message and enqueue
      handle_oem_msg(msg.size, msg.payload, 0, 0, 0);

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
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: OEM recv thread error in select (should not get here) in %s", __func__);
    }

    // Check for reboot request
    if (ril_utf_check_reboot_state() == 1)
    {
      ril_utf_oem_recv_thread_shutdown();
    }
  } // while( EXEC_OEM_THREAD )

}

/*========================================================
  FUNCTION: utf_oem_recv_msg
    - Recieves all outstanding messages on the socket
      and modifies "msgs" to contain the size and ptr
      to messages in buffer received
  Critical section access:
    - none
========================================================*/
int utf_oem_recv_msg(struct oem_buff_t *msg)
{
  int ret = TRUE;
  char buff[30];
  int length = 0;

  uint8_t tmp_buf[RIL_UTF_OEM_SOCKET_MAX_BUF_SIZE];
  int tmp_byte_num;

  oem_recv_byte_num = 0;
  memset(tmp_buf, 0, sizeof(tmp_buf));
  memset(oem_recv_buffer, 0, sizeof(oem_recv_buffer));

  // read message size
  recv(qmi_oem_sock_id, tmp_buf, 4, 0);
  length = oem_unpack_int_bigendian(tmp_buf);

  // keep reading until whole message read
  do {
    memset(tmp_buf, 0, sizeof(tmp_buf));
    if ((tmp_byte_num = recv(qmi_oem_sock_id, tmp_buf, length , 0)) <= 0)
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error: OEM could not read bytes in %s", __func__);
      ret = FALSE;
      return ret;
    }

    // add data to final buffer
    memcpy(oem_recv_buffer+oem_recv_byte_num, tmp_buf, tmp_byte_num);
    oem_recv_byte_num += tmp_byte_num;

  } while (length > oem_recv_byte_num);


  RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: %s: OEM received %d bytes... dumping hex", __func__, oem_recv_byte_num);
  ril_utf_print_hex(oem_recv_buffer, oem_recv_byte_num);

  // Assume only one message
  //TODO: replace with intelligent separation of qmi msgs
  msg->payload = oem_recv_buffer;
  msg->size = oem_recv_byte_num;

  return ret;
}

/*========================================================
  FUNCTION: handle_oem_msg
    - decode incoming oem message
  Critical section access:
  - exec_q_access is expected to be locked prior to calling
    this function.
========================================================*/
void handle_oem_msg(int size, uint8_t *payload, int msg_type, int msg_token, uint32_t oem_error)
{
  enum ril_utf_q_element_t type;
  int length = 0;
  int is_tunneled = 1;
  int x;
  uint32_t event_id;
  uint32_t payload_len;
  int32_t oem_payload_len;
  uint32_t token_id = 0;
  uint16_t error_val = 0;
  int message_id = 0;
  uint16_t service_id = 0;
  int tlv_encoded_len = 0;
  int idl_err;
  uint32_t c_struct_len = 0;
  uint8_t *temp = payload;
  void *c_struct_ptr;
  qmi_idl_type_of_message_type idl_type;
  qmi_idl_service_object_type qmi_idl_tunneling_service_object;

   QCRIL_LOG_INFO("handle_oem_msg: size: %d msg_type: %d, msg_token: %d, oem_error: %d",size, msg_type, msg_token, oem_error);
  // Don't process empty messages
  /*if (size == 0)
    return;*/

  if (msg_type == OEM__MSG_TYPE__RESPONSE)
  {
    idl_type = QMI_IDL_RESPONSE;
    type = ril_utf_oem_response;

    oem_payload_len = size;

    // Search for matching no tunneled message
    for (x = 0; x < 50; ++x)
    {
      if ( (oem_request_list[x].valid == 1) &&
           (oem_request_list[x].token_id == msg_token) )
      {
        oem_request_list[x].valid = 0;
        event_id = oem_request_list[x].oem_event_id;
        is_tunneled = 0;
        break;
      }
    }

    if (is_tunneled == 0)
    {
      // Pointer for non tunnelled OEM msgs
      if (oem_payload_len > 0)
      {
        c_struct_ptr = malloc(oem_payload_len);
        memcpy(c_struct_ptr, temp, oem_payload_len);
        c_struct_len = oem_payload_len;
      }
      else
      {
        c_struct_ptr = NULL;
      }
    }
    else
    {
      // Event id
      memcpy(&event_id, temp, 4);
      temp = temp + 4;
      // payload len
      memcpy(&payload_len, temp, 4);
      temp = temp + 4;
      // Service id
      memcpy(&service_id, temp, 2);
      temp = temp + 2;
      // message id
      memcpy(&message_id, temp, 2);
      temp = temp + 2;
      // Error val
      memcpy(&error_val, temp, 2);
      temp = temp + 2;
      // calculate tlv encoded length
      tlv_encoded_len = payload_len - 6; // 2 - service_id, 2 - message_id, 2 - err_val
    }

  }
  else if (msg_type == OEM__MSG_TYPE__UNSOL_RESPONSE)
  {
    idl_type = QMI_IDL_INDICATION;
    type = ril_utf_oem_indication;

    // RIL_UNSOL_OEM_HOOK_RAW
    temp = temp + 4;

    /////// OEM HEADER //////////
    // OEM NAME
    temp = temp + 8;
    // Event id
    memcpy(&event_id, temp, 4);
    temp = temp + 4;
    // Payload length
    memcpy(&oem_payload_len, temp, 4);
    temp = temp + 4;

#ifdef QMI_RIL_HAL_UTF
    if(event_id != QCRIL_REQ_HOOK_UNSOL_GENERIC)
#else
    if (event_id != QCRIL_EVT_HOOK_UNSOL_GENERIC)
#endif
    {
      // Pointer for non tunnelled OEM msgs
      if (oem_payload_len > 0)
      {
        c_struct_ptr = malloc(oem_payload_len);
        memcpy(c_struct_ptr, temp, oem_payload_len);
        c_struct_len = oem_payload_len;
      }
      else
      {
        c_struct_ptr = NULL;
      }
    }
    else
    {
      // Service id
      memcpy(&service_id, temp, 2);
      temp = temp + 2;
      // message id
      memcpy(&message_id, temp, 2);
      temp = temp + 2;

      // calculate tlv encoded length
      tlv_encoded_len = oem_payload_len - 4; // 2 - service_id, 2 - message_id
    }

  }
  else
  {
    RIL_UTF_ESSENTIAL("\nQMI_RIL_UTF: ERROR: received unknown OEM message type");
  }

  RIL_UTF_DEBUG("\nQMI_RIL_UTF: event_id = %d", event_id);

  // Decode with IDL lib if tunneled message
#ifdef QMI_RIL_HAL_UTF
  if ( (event_id == QCRIL_REQ_HOOK_REQ_GENERIC) ||
     (event_id == QCRIL_REQ_HOOK_UNSOL_GENERIC) )
#else
  if ( (event_id == QCRIL_EVT_HOOK_REQ_GENERIC) ||
     (event_id == QCRIL_EVT_HOOK_UNSOL_GENERIC) )
#endif
  {
    // Get service object based on service_id
    qmi_idl_tunneling_service_object = qmi_ril_oem_hook_qmi_idl_tunneling_get_service_object( (qmi_ril_oem_hook_qmi_tunneling_service_id_type) service_id);

    // Find c struct size
    idl_err = qmi_idl_get_message_c_struct_len( qmi_idl_tunneling_service_object, idl_type, message_id, &c_struct_len  );

    // Malloc destination for decode
    c_struct_ptr = malloc(c_struct_len);

    // Decode message payload
    idl_err = qmi_idl_message_decode( qmi_idl_tunneling_service_object,
                            idl_type,
                            message_id,
                            temp,
                            tlv_encoded_len,
                            c_struct_ptr,
                            c_struct_len );
    if (idl_err != 0)
    {
      RIL_UTF_ESSENTIAL("\nQMI_RIL_UTF: ERROR: idl decode failed with error %d", idl_err);
    }
  }
#ifdef QMI_RIL_HAL_UTF
  else if (event_id == QCRIL_REQ_HOOK_LTE_DIRECT_DISC_REQ ||
           event_id == QCRIL_REQ_HOOK_UNSOL_LTE_DIRECT_DISC)
#else
  else if (event_id == QCRIL_EVT_HOOK_LTE_DIRECT_DISC_REQ ||
           event_id == QCRIL_EVT_HOOK_UNSOL_LTE_DIRECT_DISC)
#endif
  {
    size_t unpacked_msg_size = 0;
    LteDirectDiscovery_MsgHeader *msg_tag_ptr = NULL;
    void *msg_data_ptr = NULL;
    qcril_evt_e_type rnd_req;

    msg_tag_ptr = qcril_qmi_lte_direct_disc_unpack_msg_tag((uint8_t*)temp, oem_payload_len);

    if(msg_tag_ptr)
    {
        message_id = msg_tag_ptr->id;
#ifndef QMI_RIL_HAL_UTF
        QCRIL_LOG_INFO("msg: %s, type: %d, message_id: %d, error: %d",
                       qcril_qmi_lte_direct_disc_get_msg_log_str(msg_tag_ptr->id, msg_tag_ptr->type),
                       msg_tag_ptr->type,
                       msg_tag_ptr->id,
                       msg_tag_ptr->error);
#endif
    }

    c_struct_ptr = msg_tag_ptr;
    c_struct_len = oem_payload_len;
  }


  RIL_UTF_DEBUG("\nQMI_RIL_UTF: OEM MESSAGE RECEIVED WITH SERVICE %x AND MESSAGE ID %x", service_id, message_id);
  // Add message to queue for test thread
  enqueue_oem_expect(event_id, message_id, service_id, token_id, type, c_struct_ptr, c_struct_len);
}

int send_ril_utf_oem_request( struct ril_utf_msg_t *test_step )
{
  int err_val = QCRIL_UNITTTEST_NO_ERR;
  int x;
  int32_t max_msg_size;
  qmi_idl_service_object_type qmi_idl_tunneling_service_object;
  int sig_err;
  int len;
  int message_type = 2; // OEM request (not in enum for some reason)
  int token = test_step->token_id;
  int command_id = test_step->oem_command_id;
  uint16_t service_id = test_step->service_id;
  uint16_t message_id = test_step->msg_id;
  char *buf;
  void *tlv_stream;
  uint32_t payload_len = test_step->payload_len;
  static uint8_t rnd_buffer[8 * 1024];

  RIL_UTF_DEBUG("\n command_id = %d\n", command_id);

#ifdef QMI_RIL_HAL_UTF
  if (command_id == QCRIL_REQ_HOOK_REQ_GENERIC)
#else
  if (command_id == QCRIL_EVT_HOOK_REQ_GENERIC)
#endif
  {
#ifdef QMI_RIL_HAL_UTF
    RIL_UTF_DEBUG("\n command_id = QCRIL_REQ_HOOK_REQ_GENERIC\n");
#else
    RIL_UTF_DEBUG("\n command_id = QCRIL_EVT_HOOK_REQ_GENERIC\n");
#endif

    RIL_UTF_DEBUG("\n service_id %d\n", service_id);
    ////////////// OEM IDL TUNNELING MESSAGE PREP ///////////////////
    // Get service object for encode
    qmi_idl_tunneling_service_object = qmi_ril_oem_hook_qmi_idl_tunneling_get_service_object( (qmi_ril_oem_hook_qmi_tunneling_service_id_type) service_id);

    if(NULL == qmi_idl_tunneling_service_object)
    {
        RIL_UTF_DEBUG("\n null qmi idl tunnel\n");
    }

    // find encode size
    int err = qmi_idl_get_max_message_len ( qmi_idl_tunneling_service_object, QMI_IDL_REQUEST, message_id, &max_msg_size);
    if ( err != QMI_NO_ERR )
    {
      RIL_UTF_ESSENTIAL("\nQMI_RIL_UTF: Error: could not find max encode size with error %d", err);
      err_val = QCRIL_UNITTEST_GEN_ERR;
      return err_val;
    }
    // malloc dest buffer
    tlv_stream = malloc(max_msg_size);
    // Encode message
    err = qmi_idl_message_encode
        ( qmi_idl_tunneling_service_object ,
          QMI_IDL_REQUEST,
          message_id,
          test_step->payload,
          test_step->payload_len,
          tlv_stream,
          max_msg_size,
          &payload_len);
    if ( err != QMI_NO_ERR )
    {
      RIL_UTF_ESSENTIAL("\nQMI_RIL_UTF: Encode OEM message failed with error %d", err);
      err_val = QCRIL_UNITTEST_GEN_ERR;
      return err_val;
    }
  } // end tunneling
  else
  {
#ifdef QMI_RIL_HAL_UTF
    if (command_id == QCRIL_REQ_HOOK_LTE_DIRECT_DISC_REQ)
#else
    if (command_id == QCRIL_EVT_HOOK_LTE_DIRECT_DISC_REQ)
#endif
    {
      memset(rnd_buffer, 0, sizeof(rnd_buffer));

      payload_len = qcril_qmi_lte_direct_disc_pack_msg_tag(
          test_step->payload,
          test_step->payload_len,
          token, // TODO get token per LteDirectDiscovery
          LteDirectDiscovery_MsgType_REQUEST,
          message_id,
          LteDirectDiscovery_Error_E_SUCCESS,
          rnd_buffer,
          sizeof(rnd_buffer));
    }

    // Add non tunneled message to list to mark it's token id with its command id
    for (x = 0; x < 50; ++x)
    {
      if (oem_request_list[x].valid == 0)
      {
        oem_request_list[x].valid = 1;
        oem_request_list[x].oem_event_id = command_id;
        oem_request_list[x].token_id = token;
        break;
      }
    }
  } // end non tunneling

  // Calculate header size and prep buffer
  int header_len = 4 + 4 + 4 + 4 + 8 + 4 + 4 + 8 + 2 + 2;
  len = header_len + payload_len;
  buf = (char*) malloc(len);
  memset(buf, 0, len);

  // Load message
  // Message length
  buf[0] = (0xff000000 & len) >> 24;
  buf[1] = (0xff0000 & len) >> 16;
  buf[2] = (0xff00 & len) >> 8;
  buf[3] = 0xff & len;

  // Message type
  buf[4] = 0xff & message_type;
  buf[5] = (0xff00 & message_type) >> 8;
  buf[6] = (0xff0000 & message_type) >> 16;
  buf[7] = (0xff000000 & message_type) >> 24;

  // Message token
  buf[8] = 0xff & token;
  buf[9] = (0xff00 & token) >> 8;
  buf[10] = (0xff0000 & token) >> 16;
  buf[11] = (0xff000000 & token) >> 24;

  // Message data len
  int oem_data_len = payload_len + 8 + 4 + 4 + 8 + 2 + 2;
  buf[12] = 0xff & oem_data_len;
  buf[13] = (0xff00 & oem_data_len) >> 8;
  buf[14] = (0xff0000 & oem_data_len) >> 16;
  buf[15] = (0xff000000 & oem_data_len) >> 24;

  ////////OEM message header///////////
  // OEM name
  char *oem_name = "QOEMHOOK";
  memcpy(buf+16, oem_name, 8);
  // command_id
  memcpy(buf+24, &command_id, 4);
#ifdef QMI_RIL_HAL_UTF
  if (command_id == QCRIL_REQ_HOOK_REQ_GENERIC)
#else
  if (command_id == QCRIL_EVT_HOOK_REQ_GENERIC)
#endif
  {
    ////////////// OEM IDL TUNNELING MESSAGE PREP ///////////////////
    // data length
    int remaining_data_len = payload_len + 8 + 2 + 2;
    memcpy(buf+28, &remaining_data_len, 4);
    // Skip 8 bytes that are reserved and not used
    // Service id
    memcpy(buf+40, &service_id, 2);
    // Message id
    memcpy(buf+42, &message_id, 2);

    // tlv data
    memcpy(buf+44, tlv_stream, payload_len);

    // Clean up resources
    free(tlv_stream);
  }
#ifdef QMI_RIL_HAL_UTF
  else if (command_id == QCRIL_REQ_HOOK_LTE_DIRECT_DISC_REQ)
#else
  else if (command_id == QCRIL_EVT_HOOK_LTE_DIRECT_DISC_REQ)
#endif
  {
    memcpy(buf+28, &(payload_len), 4);
    memcpy(buf+32, rnd_buffer, payload_len);
  }
  else
  {
    ////////////// OEM NON TUNNELING MESSAGE PREP ///////////////////
    // data length
    memcpy(buf+28, &(test_step->payload_len), 4);
    if (test_step->payload != NULL)
      memcpy(buf+32, test_step->payload, test_step->payload_len);
  }

  RIL_UTF_DEBUG("\n enqueuing");
  utf_oem_msg_q_payload *oem_payload = (utf_oem_msg_q_payload *)qcril_malloc(sizeof(utf_oem_msg_q_payload));
  if (oem_payload) {
    oem_payload->token = token;
    oem_payload->msg_len = len-16;
    oem_payload->msg = (char *)qcril_malloc(oem_payload->msg_len);
    if (oem_payload->msg) {
      memcpy(oem_payload->msg, buf+16, oem_payload->msg_len);
    }
  }
  // Add message to dispatch queue
  enqueue_oem_msg_to_send(oem_payload, sizeof(utf_oem_msg_q_payload));
  oem_inject_pending = TRUE;
  sig_err = pthread_cond_signal(&wakeup_oem_send_thread);

  if (sig_err != 0)
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Failed to wake up oem send thread.");
  }

  if (test_step) {
    if (test_step->payload) {
      free (test_step->payload);
      test_step->payload = NULL;
    }
    free (test_step);
  }

  return err_val;
}

/*========================================================
  FUNCTION: enqueue_oem_expect
    - Add received oem message to queue for test thread
  Critical section access:
  - exec_q_access is expected to be locked prior to calling
    this function.
========================================================*/
void enqueue_oem_expect(int command_id, int msg_id, int service_id, int token_id, enum ril_utf_q_element_t type,
  void *payload, int payload_len)
{
  struct ril_utf_msg_t *expect_node =
    (struct ril_utf_msg_t *) malloc(sizeof(struct ril_utf_msg_t));

  RIL_UTF_DEBUG("\nQMI_RIL_UTF: Received OEM Hook message %x with service %x", msg_id, service_id);

  // prep the node for insertion
  expect_node->oem_command_id = command_id;
  expect_node->msg_id = msg_id;
  expect_node->service_id = service_id;
  expect_node->token_id = token_id;
  expect_node->type = type;
  expect_node->payload = payload;
  expect_node->payload_len = payload_len;

  enqueue_ril_utf_q_node(expect_node, &ril_utf_rec_head, &ril_utf_rec_tail);
}

void ril_utf_oem_send_thread_shutdown()
{
  RIL_UTF_DEBUG("\nQMI_RIL_UTF: UNLOCKING exec_q_mutex");
  EXEC_Q_UNLOCK();

  send(qmi_oem_sock_id, "rs", (strlen("rs")+1), 0);
  close(qmi_oem_sock_id);

  pthread_exit(NULL);
}
void ril_utf_oem_recv_thread_shutdown()
{
  RIL_UTF_DEBUG("\nQMI_RIL_UTF: Executing oem recv thread shutdown");
  pthread_exit(NULL);
}


//=========================================================================
// FUNCTION: oem_unpack_int_bigendian
//
// DESCRIPTION: unpack a 4 byte value in big-endian
//
// RETURN: unpacked data as an int
//=========================================================================
//
int oem_unpack_int_bigendian(char* data) {

   int value = 0;
   value += ((unsigned char)data[0]) << 24;
   value += ((unsigned char)data[1]) << 16;
   value += ((unsigned char)data[2]) << 8;
   value += ((unsigned char)data[3]);

   return value;
}
