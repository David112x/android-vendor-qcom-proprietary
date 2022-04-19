/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_ims_sim.c
  @brief   RIL UTF IMS simulation and endpoint handling

  DESCRIPTION
   Go between RIL UTF test and  ioe_client
  ---------------------------------------------------------------------------
******************************************************************************/
#include "ril_utf_ioe_client_sim.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>

// Globals
extern struct ril_utf_expect_t *ril_utf_rec_head;
extern struct ril_utf_expect_t *ril_utf_rec_tail;
extern pthread_t ioe_client_send_id;
extern pthread_t ioe_client_recv_id;
extern int ioe_client_inject_pending;
extern int ioe_client_expect_ready;
extern pthread_cond_t wakeup_ioe_client_send_thread;
extern pthread_cond_t wakeup_ioe_client_recv_thread;

// Local definitions
#define UTF_IOE_SERVER_SOCKET_PATH "./qmi_csi_sock1"
#define UTF_IOE_CLIENT_SOCKET_CONNECT_MAX_ATTEMPTS 200
#define MAX_IOE_CLIENT_MESSAGES 10
#define RIL_UTF_IOE_CLIENT_MAX_BUFFER_BYTES_PER_LINE                16
#define RIL_UTF_IOE_CLIENT_MAX_OUTPUT_BUF_SIZE                      ( ( RIL_UTF_IOE_CLIENT_MAX_BUFFER_BYTES_PER_LINE * 5 ) + 2 )

uint8_t recv_buffer[RIL_UTF_IOE_CLIENT_SOCKET_MAX_BUF_SIZE];
int recv_byte_num;

int ioe_srv_sock_id;

Ioe_client_msgQ *ioe_client_inject_msg_q_head = NULL;
Ioe_client_msgQ *ioe_client_inject_msg_q_tail = NULL;

Ioe_client_msgQ *ioe_client_expect_msg_q_head = NULL;
Ioe_client_msgQ *ioe_client_expect_msg_q_tail = NULL;

/*========================================================
  FUNCTION: utf_start_ioe_client_threads
    - starts the following threads:
      - utf_ims_send_start
      - utf_ims_recv_start
========================================================*/
void utf_start_ioe_client_threads()
{
  pthread_attr_t ioe_client_send_thrd_attr;
  pthread_attr_t ioe_client_recv_thrd_attr;
  int ret;
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Connecting to  IOE_CLIENT socket...");
  if ( connect_to_ril_ioe_client_socket(0) != TRUE )
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error: Could not connect to  IOE_CLIENT socket. check error msg");
    return;
  }

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Starting  IOE_CLIENT send thread...");
  pthread_attr_init(&ioe_client_send_thrd_attr);
  ret = pthread_create(&ioe_client_send_id, &ioe_client_send_thrd_attr, utf_ioe_client_send_start, NULL);

  if ( ret < 0 )
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Failed to start ioe_client send thread. Exiting...");
    return;
  }


  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Starting  IOE_CLIENT recv thread...");
  pthread_attr_init(&ioe_client_recv_thrd_attr);
  ret = pthread_create(&ioe_client_recv_id, &ioe_client_recv_thrd_attr, utf_ioe_client_recv_start, NULL);

  if ( ret < 0 )
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Failed to start ioe_client recv thread. Exiting...");
    return;
  }
}

/*========================================================
  FUNCTION: connect_to_ril_ioe_client_socket
    - Attempts to make a connection to the exported
      socket from  ioe_client
========================================================*/
int connect_to_ril_ioe_client_socket(uint8_t client_handle)
{
  int sock_conn_length;
  int utf_ioe_client_sock_conn_max_attempts = UTF_IOE_CLIENT_SOCKET_CONNECT_MAX_ATTEMPTS;

  struct sockaddr_un ioe_srv_addr;

  if(ioe_srv_sock_id == UTF_IOE_SERVER_SOCKET_ID)
  {
      if ( ( ioe_srv_sock_id = socket(AF_UNIX, SOCK_STREAM, 0) ) < 0)
      {
          RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: !!! Error !!! Could not create socket connection to IOE server failed. Exiting...");
          return FALSE;
      }

      memset(&ioe_srv_addr, 0x00, sizeof(ioe_srv_addr));

      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Trying to connect to IOE server socket %s",UTF_IOE_SERVER_SOCKET_PATH);

      ioe_srv_addr.sun_family = AF_UNIX;
      strlcpy(ioe_srv_addr.sun_path, UTF_IOE_SERVER_SOCKET_PATH, sizeof(ioe_srv_addr.sun_path) - 1);

      sock_conn_length = strlen(ioe_srv_addr.sun_path) + sizeof(ioe_srv_addr.sun_family);


      while ( utf_ioe_client_sock_conn_max_attempts )
      {
        if ( connect(ioe_srv_sock_id, (struct sockaddr*) &ioe_srv_addr, sock_conn_length) < 0 )
        {
          RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: !!! Error !!!Failed to  IoE Server socket.");
          utf_ioe_client_sock_conn_max_attempts--;

          if ( !utf_ioe_client_sock_conn_max_attempts )
          {
            RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: !!! Error !!!Failed to  ioe_client socket after %d attempts.", UTF_IOE_CLIENT_SOCKET_CONNECT_MAX_ATTEMPTS);
            return FALSE;
          }
        }
        else
        {
          RIL_UTF_DEBUG("\n QMI_RIL_UTL: Connected to  IoE Server socket.");
          break;
        }
      }
  }
  //Connect the client to th Server (client needs to register for IoE server)
  uint8_t *send_message_buffer;
  int buffer_len = UTF_IOE_CONNECT_CB_BUFFER_FIXED_LEN;
  send_message_buffer = (uint8_t*) malloc(buffer_len);
  memset(send_message_buffer, 0x00, buffer_len);
  int i = 0;
  send_message_buffer[i++] = UTF_IOE_CLIENT_CONNECT_CB_ID; //id to determine Connect_cb at the other end
  send_message_buffer[i] = client_handle;

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: IOE_CLIENT connecting to the server by sending connect_cb in %s", __func__);
//EXEC_Q_LOCK();
  if (send(ioe_srv_sock_id, (void *) send_message_buffer, buffer_len, 0) < 0)
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error! Failed to send connect mesage to  ioe_client socket.");
  }
//EXEC_Q_UNLOCK();
 return TRUE;
}
/*========================================================
  FUNCTION: utf_ioe_client_send_start
  - Starts the utf ioe_client send thread.
  - will wait for signal from UTF main test exec
    for an "inject" to send a message to  ioe_client.

  Critical section access:
  - exec_q_access locked and unlocked here.
  - Note: pthread_wait will release exec_q_access
    when waiting, and locks again upon re-entering.
========================================================*/
void* utf_ioe_client_send_start( void *param)
{
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: IOE_CLIENT send thread entry...");

  // Acquire Q lock
//RIL_UTF_DEBUG("\n QMI_RIL_UTL: IOE_CLIENT send thread locking exec_q_access in %s", __func__);
  EXEC_Q_LOCK();

  while( EXEC_IOE_CLIENT_THREAD )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Exec...ioe_client thread");
    // Wait for items to be placed in Q
    while ( !ioe_client_inject_pending )
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Waiting for ioe_client request message in %s", __func__);
      pthread_cond_wait(&wakeup_ioe_client_send_thread, &exec_q_access);
    }
 RIL_UTF_DEBUG("\n QMI_RIL_UTL: processing  IOE_CLIENT inject queue1111.");
    process_ioe_client_inject_queue();

    ioe_client_inject_pending = FALSE;

    // Inform test thread we are finished
    inject_complete = 1;
    pthread_cond_signal(&inject_complete_sig);
  }

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: IOE_CLIENT send thread unlocking exec_q_access in %s", __func__);
  EXEC_Q_UNLOCK();

}

/*========================================================
  FUNCTION: process_ioe_client_inject_queue
  - Processes the IOE_CLIENT inject queue.
  -   Take elements in the queue and send them to
       IOE_CLIENT socket.

  Critical section access:
  - exec_q_access must be locked prior to calling this
    function.
========================================================*/
void process_ioe_client_inject_queue(void)
{
     RIL_UTF_DEBUG("\n QMI_RIL_UTL: processing  IOE_CLIENT inject queue.2");
  char *ioe_client_msg_to_send = (char *) malloc(PACKED_IOE_CLIENT_MSG_MAX_LEN);
   RIL_UTF_DEBUG("\n QMI_RIL_UTL: processing  IOE_CLIENT inject queue3.");
  int   ioe_client_msg_to_send_len = 0;

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: processing  IOE_CLIENT inject queue.%d",ioe_client_inject_msg_q_tail);

  while( ioe_client_inject_msg_q_tail )
  {
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: processing  IOE_CLIENT inject queue.before memset");
    memset(ioe_client_msg_to_send, 0x00, PACKED_IOE_CLIENT_MSG_MAX_LEN);
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: processing  IOE_CLIENT inject queue.before memset after");
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: getting message to send to  IOE_CLIENT socket.");
    if (get_ioe_client_msg_to_send(ioe_client_msg_to_send, &ioe_client_msg_to_send_len))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: getting message to send to  IOE_CLIENT socket.");
      ril_utf_send_ioe_client_message(ioe_client_msg_to_send, ioe_client_msg_to_send_len);
    }
    else
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Warning! No  message to inject.");
    }
  }
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: no while loop :(");
}

/*========================================================
  FUNCTION: ril_utf_send_ioe_client_message
  - Send message to  ioe_client socket

  Critical section access:
  - exec_q_access must be locked prior to calling this
    function.
========================================================*/
void ril_utf_send_ioe_client_message(char *msg, int len)
{
  if (send(ioe_srv_sock_id, (void *) msg, len, 0) < 0)
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error! Failed to send mesage to  ioe_client socket.");
  }
}

/*========================================================
  FUNCTION: get_ioe_client_msg_to_send
  -  Get's the QMI IOE_CLIENT message to send from the
     IOE_CLIENT message queue
  - Returns true if there is a message to send
    and false if the queue is empty.

  Critical section access:
  - exec_q_access must be locked prior to calling this
    function.
========================================================*/
int get_ioe_client_msg_to_send(char *msg, int *len)
{
  int ret_val = FALSE;

  if( ioe_client_inject_msg_q_tail )
  {
    //copy the message and len to send.
    memcpy(msg, ioe_client_inject_msg_q_tail->payload, ioe_client_inject_msg_q_tail->payload_len);
    *len = ioe_client_inject_msg_q_tail->payload_len;

    //Remove from queue
    Ioe_client_msgQ *prev = ioe_client_inject_msg_q_tail->prev;
    if ( prev )
    {
      prev->next = NULL;
    }
    else
        ioe_client_inject_msg_q_head = NULL;
    free(ioe_client_inject_msg_q_tail);
    ioe_client_inject_msg_q_tail = prev;

    ret_val = TRUE;
  }
  else
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: IOE_CLIENT inject queue empty");
    ret_val = FALSE;
  }

  return ret_val;
}

/*========================================================
  FUNCTION: enqueue_ioe_client_msg_to_send

  Critical section access:
  - exec_q_access must be locked prior to calling this
    function.
========================================================*/
void enqueue_ioe_client_msg_to_send(void *msg, int len)
{
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: enqueue message %d --- %d", msg, len);
    Ioe_client_msgQ *new_msg = (Ioe_client_msgQ *) malloc(sizeof(Ioe_client_msgQ));

  memset(new_msg, 0x00, sizeof(Ioe_client_msgQ));
  new_msg->payload = msg;
  new_msg->payload_len = len;

  if( ioe_client_inject_msg_q_head )
  {
    new_msg->prev = NULL;
    new_msg->next = ioe_client_inject_msg_q_head;
    ioe_client_inject_msg_q_head->prev = new_msg;
  ioe_client_inject_msg_q_tail = new_msg;
  }
  else
  {
    ioe_client_inject_msg_q_tail = new_msg;
  }

  ioe_client_inject_msg_q_head = new_msg;
}
/*========================================================
  FUNCTION: utf_ioe_client_recv_start
  - Starts the utf ioe_client recv thread.

  Critical section access:
  - exec_q_access locked and unlocked here.
  - Note: pthread_wait will release exec_q_access
    when waiting, and locks again upon re-entering.
========================================================*/
void* utf_ioe_client_recv_start( void *param)
{
  fd_set rfds;
  int ret_val;
  int i, my_expect_slot, x;
  struct ioe_client_buff_t msgs[MAX_IOE_CLIENT_MESSAGES];

  while( EXEC_IOE_CLIENT_THREAD )
  {
    memset(msgs, 0, sizeof(msgs));
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Exec...ioe_client recv thread");

    FD_ZERO(&rfds);
    FD_SET(ioe_srv_sock_id, &rfds);

    ret_val = select(ioe_srv_sock_id+1, &rfds, NULL, NULL, NULL);

    if ( ret_val == -1 )
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: IOE_CLIENT recv thread error in select in func %s", __func__);
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Select returns error %d", ret_val);
      return NULL;
    }
    else if ( ret_val )
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: %s: IOE_CLIENT recv thread receiving message", __func__);
      utf_ioe_client_recv_msg(msgs);

      for ( x = 0; msgs[x].payload != NULL; ++x ) // handle all received msgs
      {
        // acquire mutex
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: IOE_CLIENT recv thread locking exec_q_access in %s", __func__);
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
        handle_ioe_client_msg(msgs[x].size, msgs[x].payload);

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
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: IOE_CLIENT recv thread error in select (should not get here) in %s", __func__);
    }
  } // while( EXEC_IOE_CLIENT_THREAD )

}

/*========================================================
  FUNCTION: utf_ioe_client_recv_msg
    - Recieves all outstanding messages on the socket
      and modifies "msgs" to contain the size and ptr
      to messages in buffer received
  Critical section access:
    - none
========================================================*/
int utf_ioe_client_recv_msg(struct ioe_client_buff_t msgs[])
{
  int ret = TRUE;
  int msg_count = 0;
  int curr_msg = 0;

  if ((recv_byte_num = recv(ioe_srv_sock_id, recv_buffer, sizeof(recv_buffer) , 0)) <= 0)
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error: IOE_CLIENT could not read bytes in %s", __func__);
    ret = FALSE;
    return ret;
  }

  RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: %s: IOE_CLIENT received %d bytes... dumping hex", __func__, recv_byte_num);
  //ril_utf_print_hex(recv_buffer, recv_byte_num);

  // Assume only one message
  //TODO: replace with intelligent separation of qmi msgs
  msgs[msg_count].payload = recv_buffer;
  msgs[msg_count++].size = recv_byte_num;


  return ret;
}

/*========================================================
  FUNCTION: handle_ioe_client_msg
    - decode incoming ioe_client message
  Critical section access:
  - exec_q_access is expected to be locked prior to calling
    this function.
========================================================*/
void handle_ioe_client_msg(int size, uint8_t *payload)
{
  enum ril_utf_q_element_t type;

  //TODO: decode qmi idl byte stream here

  // Add message to queue for test thread
  enqueue_ioe_client_expect(payload);


}

/*========================================================
  FUNCTION: enqueue_ioe_client_expect
    - Add received ioe_client message to queue for test thread
  Critical section access:
  - exec_q_access is expected to be locked prior to calling
    this function.
========================================================*/
void enqueue_ioe_client_expect(uint8_t *payload)
{
  struct ril_utf_msg_t *expect_node =
    (struct ril_utf_msg_t *) malloc(sizeof(struct ril_utf_msg_t));
  int i = 0;
  int j = 0;
  uint32_t temp;
  switch( payload[i++] )
  {
    case UTF_IOE_CLIENT_RESP_CB_ID:
      expect_node->type = ril_utf_ioe_response;
      break;
    case UTF_IOE_CLIENT_IND_CB_ID:
      expect_node->type = ril_utf_ioe_indication;
      break;
  }

  //TODO: add necessary data to node for expect handle
    expect_node->client_id  = payload[i++];
    expect_node->service_id = payload[i++];
    expect_node->msg_id = 0x000;
    expect_node->payload_len = 0x000;
    for(j = 0; j < (sizeof(uint32_t)/sizeof(uint8_t)) ; j++)
    {
        temp = (uint32_t)payload[i++];
        temp = temp << (j * 8);
        expect_node->msg_id = expect_node->msg_id | temp;
    }
    for(j = 0; j < (sizeof(uint32_t)/sizeof(uint8_t)) ; j++)
    {
        temp = (uint32_t)payload[i++];
        temp = temp << (j * 8);
        expect_node->payload_len = expect_node->payload_len | temp;
    }
    expect_node->payload = (void *) malloc(expect_node->payload_len);
    memcpy(expect_node->payload, &payload[i], expect_node->payload_len);
    RIL_UTF_DEBUG("\n ENQUEED NODE WITH TYPE: %d",expect_node->type);
  enqueue_ril_utf_q_node(expect_node, &ril_utf_rec_head, &ril_utf_rec_tail);
}

