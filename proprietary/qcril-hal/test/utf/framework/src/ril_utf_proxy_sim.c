/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
  @file    ril_utf_ims_sim.c
  @brief   RIL UTF IMS simulation and endpoint handling

  DESCRIPTION
   Go between RIL UTF test and QMI PROXY
  ---------------------------------------------------------------------------
******************************************************************************/
#include "ril_utf_proxy_sim.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>

// Globals
extern struct ril_utf_expect_t *ril_utf_rec_head;
extern struct ril_utf_expect_t *ril_utf_rec_tail;
extern pthread_t proxy_send_id;
extern pthread_t proxy_recv_id;
extern int proxy_inject_pending;
extern int proxy_expect_ready;
extern pthread_cond_t wakeup_proxy_send_thread;
extern pthread_cond_t wakeup_proxy_recv_thread;

// Local definitions
#define UTF_QMI_PROXY_SOCKET_PATH "./proxy_qmux_connect_socket"
#define UTF_PROXY_SOCKET_CONNECT_MAX_ATTEMPTS 10
#define MAX_PROXY_MESSAGES 10
#define RIL_UTF_PROXY_MAX_BUFFER_BYTES_PER_LINE                16
#define RIL_UTF_PROXY_MAX_OUTPUT_BUF_SIZE                      ( ( RIL_UTF_PROXY_MAX_BUFFER_BYTES_PER_LINE * 5 ) + 2 )

uint8_t recv_buffer[RIL_UTF_PROXY_SOCKET_MAX_BUF_SIZE];
int recv_byte_num;

int qmi_proxy_sock_id;

ProxymsgQ *proxy_inject_msg_q_head = NULL;
ProxymsgQ *proxy_inject_msg_q_tail = NULL;

ProxymsgQ *proxy_expect_msg_q_head = NULL;
ProxymsgQ *proxy_expect_msg_q_tail = NULL;

/*========================================================
  FUNCTION: utf_start_proxy_threads
    - starts the following threads:
      - utf_ims_send_start
      - utf_ims_recv_start
========================================================*/
void utf_start_proxy_threads()
{
  pthread_attr_t proxy_send_thrd_attr;
  pthread_attr_t proxy_recv_thrd_attr;
  int ret;

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Connecting to QMI PROXY socket...");
  if ( connect_to_ril_proxy_socket() != TRUE )
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error: Could not connect to QMI PROXY socket. check error msg");
    return;
  }

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Starting QMI PROXY send thread...");
  pthread_attr_init(&proxy_send_thrd_attr);
  ret = pthread_create(&proxy_send_id, &proxy_send_thrd_attr, utf_proxy_send_start, NULL);

  if ( ret < 0 )
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Failed to start proxy send thread. Exiting...");
    return;
  }


  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Starting QMI PROXY recv thread...");
  pthread_attr_init(&proxy_recv_thrd_attr);
  ret = pthread_create(&proxy_recv_id, &proxy_recv_thrd_attr, utf_proxy_recv_start, NULL);

  if ( ret < 0 )
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Failed to start proxy recv thread. Exiting...");
    return;
  }
}

/*========================================================
  FUNCTION: connect_to_qmi_proxy_socket
    - Attempts to make a connection to the exported
      socket from qmi proxy
========================================================*/
int connect_to_ril_proxy_socket(void)
{
  int sock_conn_length;
  int utf_proxy_sock_conn_max_attempts = UTF_PROXY_SOCKET_CONNECT_MAX_ATTEMPTS;

  struct sockaddr_un qmi_proxy_addr;

  if ( ( qmi_proxy_sock_id = socket(AF_UNIX, SOCK_STREAM, 0) ) < 0)
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: !!! Error !!! Could not create socket Exiting...");
    return FALSE;
  }

  memset(&qmi_proxy_addr, 0x00, sizeof(qmi_proxy_addr));

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Trying to connect to qmi proxy socket %s",UTF_QMI_PROXY_SOCKET_PATH);

  qmi_proxy_addr.sun_family = AF_UNIX;
  strlcpy(qmi_proxy_addr.sun_path, UTF_QMI_PROXY_SOCKET_PATH, sizeof(qmi_proxy_addr.sun_path) - 1);

  sock_conn_length = strlen(qmi_proxy_addr.sun_path) + sizeof(qmi_proxy_addr.sun_family);


  while ( utf_proxy_sock_conn_max_attempts )
  {
    if ( connect(qmi_proxy_sock_id, (struct sockaddr*) &qmi_proxy_addr, sock_conn_length) < 0 )
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: !!! Error !!!Failed to qmi proxy socket.");
      utf_proxy_sock_conn_max_attempts--;

      if ( !utf_proxy_sock_conn_max_attempts )
      {
        RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: !!! Error !!!Failed to qmi proxy socket after %d attempts.", UTF_PROXY_SOCKET_CONNECT_MAX_ATTEMPTS);
        return FALSE;
      }
    }
    else
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Connected to qmi proxy socket.");
      break;
    }
  }


  return TRUE;
}

/*========================================================
  FUNCTION: utf_proxy_send_start
  - Starts the utf proxy send thread.
  - will wait for signal from UTF main test exec
    for an "inject" to send a message to qmi proxy.

  Critical section access:
  - exec_q_access locked and unlocked here.
  - Note: pthread_wait will release exec_q_access
    when waiting, and locks again upon re-entering.
========================================================*/
void* utf_proxy_send_start( void *param)
{
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Starting QMI PROXY send thread...");

  // Acquire Q lock
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: PROXY send thread locking exec_q_access in %s", __func__);
  EXEC_Q_LOCK();

  while( EXEC_IMS_THREAD )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Exec...proxy thread");
    // Wait for items to be placed in Q
    while ( !proxy_inject_pending )
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Waiting for proxy request message in %s", __func__);
      pthread_cond_wait(&wakeup_proxy_send_thread, &exec_q_access);
    }

    process_proxy_inject_queue();

    proxy_inject_pending = FALSE;

    // Inform test thread we are finished
    inject_complete = 1;
    pthread_cond_signal(&inject_complete_sig);
  }

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: PROXY send thread unlocking exec_q_access in %s", __func__);
  EXEC_Q_UNLOCK();

}

/*========================================================
  FUNCTION: process_proxy_inject_queue
  - Processes the PROXY inject queue.
  -   Take elements in the queue and send them to
      QMI PROXY socket.

  Critical section access:
  - exec_q_access must be locked prior to calling this
    function.
========================================================*/
void process_proxy_inject_queue(void)
{
  char *proxy_msg_to_send = (char *) malloc(PACKED_PROXY_MSG_MAX_LEN);
  int   proxy_msg_to_send_len = 0;

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: processing QMI PROXY inject queue.");

  while( proxy_inject_msg_q_tail )
  {
    memset(proxy_msg_to_send, 0x00, PACKED_PROXY_MSG_MAX_LEN);

    RIL_UTF_DEBUG("\n QMI_RIL_UTL: getting message to send to QMI PROXY socket.");
    if (get_proxy_msg_to_send(proxy_msg_to_send, &proxy_msg_to_send_len))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: getting message to send to QMI PROXY socket.");
      ril_utf_send_proxy_message(proxy_msg_to_send, proxy_msg_to_send_len);
    }
    else
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Warning! No QMI message to inject.");
    }
  }

  if (proxy_msg_to_send) {
    free(proxy_msg_to_send);
  }
}

/*========================================================
  FUNCTION: ril_utf_send_proxy_message
  - Send message to qmi proxy socket

  Critical section access:
  - exec_q_access must be locked prior to calling this
    function.
========================================================*/
void ril_utf_send_proxy_message(char *msg, int len)
{
  if (send(qmi_proxy_sock_id, (void *) msg, len, 0) < 0)
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error! Failed to send mesage to qmi proxy socket.");
  }
}

/*========================================================
  FUNCTION: get_proxy_msg_to_send
  -  Get's the QMI PROXY message to send from the
     PROXY message queue
  - Returns true if there is a message to send
    and false if the queue is empty.

  Critical section access:
  - exec_q_access must be locked prior to calling this
    function.
========================================================*/
int get_proxy_msg_to_send(char *msg, int *len)
{
  int ret_val = FALSE;

  if( proxy_inject_msg_q_tail )
  {
    //copy the message and len to send.
    memcpy(msg, proxy_inject_msg_q_tail->payload, proxy_inject_msg_q_tail->payload_len);
    *len = proxy_inject_msg_q_tail->payload_len;

    //Remove from queue
    ProxymsgQ *prev = proxy_inject_msg_q_tail->prev;
    if ( prev )
    {
      prev->next = NULL;
    }
    if (proxy_inject_msg_q_tail) {
      if (proxy_inject_msg_q_tail->payload) {
        free(proxy_inject_msg_q_tail->payload);
      }
      free(proxy_inject_msg_q_tail);
    }
    proxy_inject_msg_q_tail = prev;
    if (proxy_inject_msg_q_tail == NULL)
    {
      proxy_inject_msg_q_head = NULL;
    }

    ret_val = TRUE;
  }
  else
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: PROXY inject queue empty");
    ret_val = FALSE;
  }

  return ret_val;
}

/*========================================================
  FUNCTION: enqueue_proxy_msg_to_send

  Critical section access:
  - exec_q_access must be locked prior to calling this
    function.
========================================================*/
void enqueue_proxy_msg_to_send(void *msg, int len)
{
  ProxymsgQ *new_msg = (ProxymsgQ *) malloc(sizeof(ProxymsgQ));
  memset(new_msg, 0x00, sizeof(ProxymsgQ));
  new_msg->payload = msg;
  new_msg->payload_len = len;

  if( proxy_inject_msg_q_head )
  {
    new_msg->prev = NULL;
    new_msg->next = proxy_inject_msg_q_head;
    proxy_inject_msg_q_head->prev = new_msg;
  }
  else
  {
    proxy_inject_msg_q_tail = new_msg;
  }

  proxy_inject_msg_q_head = new_msg;
}

/*========================================================
  FUNCTION: utf_proxy_recv_start
  - Starts the utf proxy recv thread.

  Critical section access:
  - exec_q_access locked and unlocked here.
  - Note: pthread_wait will release exec_q_access
    when waiting, and locks again upon re-entering.
========================================================*/
void* utf_proxy_recv_start( void *param)
{
  fd_set rfds;
  int ret_val;
  int i, my_expect_slot, x;
  struct proxy_buff_t msgs[MAX_PROXY_MESSAGES];

  while( EXEC_PROXY_THREAD )
  {
    memset(msgs, 0, sizeof(msgs));
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Exec...proxy recv thread");

    FD_ZERO(&rfds);
    FD_SET(qmi_proxy_sock_id, &rfds);

    ret_val = select(qmi_proxy_sock_id+1, &rfds, NULL, NULL, NULL);

    if ( ret_val == -1 )
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: PROXY recv thread error in select in func %s", __func__);
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Select returns error %d", ret_val);
      return NULL;
    }
    else if ( ret_val )
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: %s: PROXY recv thread receiving message", __func__);
      utf_proxy_recv_msg(msgs);

      for ( x = 0; msgs[x].payload != NULL; ++x ) // handle all received msgs
      {
        // acquire mutex
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: PROXY recv thread locking exec_q_access in %s", __func__);
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
        struct timespec expect_timeout;
        while ( expect_table[my_expect_slot].expect_ready != 1 )
        {
          expect_timeout.tv_sec = 0;
          expect_timeout.tv_nsec = EXPECT_TIMEOUT * 1000;
          pthread_cond_timedwait(&expect_ready_sig, &exec_q_access, &expect_timeout);
        }

        // Decode message and enqueue
        handle_proxy_msg(msgs[x].size, msgs[x].payload);

        // signal test thread.
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Setting expect_received to true @ %s %s %d ",__FILE__, __func__, __LINE__);
        expect_table[my_expect_slot].expect_received = 1;
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Signalling Expectation");
        pthread_cond_signal(&signal_exptectation);

        // release mutex
        EXEC_Q_UNLOCK();
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: %s. released exec_q_mutex.", __func__);

      } //end for

    }
    else
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: PROXY recv thread error in select (should not get here) in %s", __func__);
    }
  } // while( EXEC_PROXY_THREAD )

}

/*========================================================
  FUNCTION: utf_proxy_recv_msg
    - Recieves all outstanding messages on the socket
      and modifies "msgs" to contain the size and ptr
      to messages in buffer received
  Critical section access:
    - none
========================================================*/
int utf_proxy_recv_msg(struct proxy_buff_t msgs[])
{
  int ret = TRUE;
  int msg_count = 0;
  int curr_msg = 0;

  if ((recv_byte_num = recv(qmi_proxy_sock_id, recv_buffer, sizeof(recv_buffer) , 0)) <= 0)
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error: PROXY could not read bytes in %s", __func__);
    ret = FALSE;
    return ret;
  }

  RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: %s: PROXY received %d bytes... dumping hex", __func__, recv_byte_num);
  ril_utf_print_hex(recv_buffer, recv_byte_num);

  // Assume only one message
  //TODO: replace with intelligent separation of qmi msgs
  msgs[msg_count].payload = recv_buffer;
  msgs[msg_count++].size = recv_byte_num;


  return ret;
}

/*========================================================
  FUNCTION: handle_proxy_msg
    - decode incoming proxy message
  Critical section access:
  - exec_q_access is expected to be locked prior to calling
    this function.
========================================================*/
void handle_proxy_msg(int size, uint8_t *payload)
{
  enum ril_utf_q_element_t type;

  //TODO: decode qmi idl byte stream here

  // Add message to queue for test thread
  enqueue_proxy_expect(payload);


}

/*========================================================
  FUNCTION: enqueue_proxy_expect
    - Add received proxy message to queue for test thread
  Critical section access:
  - exec_q_access is expected to be locked prior to calling
    this function.
========================================================*/
void enqueue_proxy_expect(uint8_t *payload)
{
  struct ril_utf_msg_t *expect_node =
    (struct ril_utf_msg_t *) malloc(sizeof(struct ril_utf_msg_t));
//TODO: add necessary data to node for expect handle
  expect_node->service_id = payload[4];
  expect_node->client_id  = payload[5];

  switch( payload[6] )
  {
    case 0:
      expect_node->type = ril_utf_qmi_request;
      break;
    case 1:
      expect_node->type = ril_utf_qmi_response;
      break;
    case 4:
      expect_node->type = ril_utf_qmi_indication;
      break;
  }

  if( expect_node->service_id == QMI_CTL )
  {
    expect_node->msg_id = payload[8];
    expect_node->payload_len = payload[10]; //TBD: decode this..!
    expect_node->payload = (void *) malloc(expect_node->payload_len);
    memcpy(expect_node->payload, &payload[12], expect_node->payload_len);
  }
  else
  {
    expect_node->msg_id = payload[9];
    expect_node->payload_len = payload[11];
    expect_node->payload = (void *) malloc(expect_node->payload_len);
    memcpy(expect_node->payload, &payload[13], expect_node->payload_len);
  }

  enqueue_ril_utf_q_node(expect_node, &ril_utf_rec_head, &ril_utf_rec_tail);
}

