/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_msim_rild.c
  @brief   RIL UTF MSIM IPC simulation and endpoint handling

  DESCRIPTION
    This simulation file is used to simulate IPC from second RILD in multiple
    rild call flows
  ---------------------------------------------------------------------------
******************************************************************************/
#include "ril_utf_msim_rild.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>

// Globals
extern struct ril_utf_expect_t *ril_utf_rec_head;
extern struct ril_utf_expect_t *ril_utf_rec_tail;

extern pthread_t msim_recv_id;
extern pthread_cond_t wakeup_msim_recv_thread;
extern int msim_expect_ready;

extern pthread_mutex_t reset_flag_mutex;
extern boolean reset_now_flag;

// MSIM RIL instance
extern int rild_instance;


#define MAX_MSIM_MESSAGES 10
#define RIL_UTF_MSIM_MAX_BUFFER_BYTES_PER_LINE                16
#define RIL_UTF_MSIM_MAX_OUTPUT_BUF_SIZE                      ( ( RIL_UTF_MSIM_MAX_BUFFER_BYTES_PER_LINE * 5 ) + 2 )

#define RIL_UTF_MSIM_PRIMARY_SOCKET "./rild_sync_0"
#define RIL_UTF_MSIM_SECONDARY_SOCKET "./rild_sync_1"

int msim_local_sock_id;
struct sockaddr_un msim_local_addr;
struct sockaddr_un msim_remote_addr;


msimmsgQ *msim_expect_msg_q_head = NULL;
msimmsgQ *msim_expect_msg_q_tail = NULL;

/*========================================================
  FUNCTION: utf_start_msim_thread
    - starts the following threads:
      - utf_msim_send_start
      - utf_msim_recv_start
========================================================*/
void utf_start_msim_threads()
{
  pthread_attr_t msim_send_thrd_attr;
  pthread_attr_t msim_recv_thrd_attr;
  int ret;


  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Connecting to QMI RIL MSIM socket...");
  if ( setup_ril_msim_socket() != TRUE )
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error: Could not connect to QMI RIL MSIM socket. check error msg");
    return;
  }


  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Starting MSIM recv thread...");
  pthread_attr_init(&msim_recv_thrd_attr);
  ret = utf_pthread_create_handler(&msim_recv_id, &msim_recv_thrd_attr, utf_msim_recv_start, NULL);

  if ( ret < 0 )
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Failed to start msim recv thread. Exiting...");
    return;
  }
}

/*========================================================
  FUNCTION: setup_ril_msim_socket
    - Opens up IPC socet for emulated ril
========================================================*/
int setup_ril_msim_socket(void)
{
  int sock_len;

  unlink(RIL_UTF_MSIM_PRIMARY_SOCKET);
  unlink(RIL_UTF_MSIM_SECONDARY_SOCKET);

  if ( ( msim_local_sock_id = socket(AF_UNIX, SOCK_DGRAM, 0) ) < 0)
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: !!! Error !!!Could not create socket Exiting...");
    exit(1);
    return FALSE;
  }

// set up remote addr info
  memset(&msim_remote_addr, 0, sizeof(msim_remote_addr));
  msim_remote_addr.sun_family = AF_UNIX;

// set up local addr info
  memset(&msim_local_addr, 0x00, sizeof(msim_local_addr));
  msim_local_addr.sun_family = AF_UNIX;

  if (rild_instance == 1)
  {
    strlcpy(msim_local_addr.sun_path, RIL_UTF_MSIM_SECONDARY_SOCKET, sizeof(msim_local_addr.sun_path));
    strlcpy(msim_remote_addr.sun_path, RIL_UTF_MSIM_PRIMARY_SOCKET, sizeof(msim_remote_addr.sun_path));
  }
  else
  {
    strlcpy(msim_local_addr.sun_path, RIL_UTF_MSIM_PRIMARY_SOCKET, sizeof(msim_local_addr.sun_path));
    strlcpy(msim_remote_addr.sun_path, RIL_UTF_MSIM_SECONDARY_SOCKET, sizeof(msim_remote_addr.sun_path));
  }

  sock_len = strlen(msim_local_addr.sun_path) + sizeof(msim_local_addr.sun_family);

  if (bind(msim_local_sock_id, (struct sockaddr*)&msim_local_addr, sock_len) < 0)
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: !!! Error !!!Could not bind socket Exiting...");
    return FALSE;
  }

  return TRUE;
}


/*========================================================
  FUNCTION: ril_utf_send_msim_message
  - Send message to qmi ril msim socket

  Critical section access:
  - exec_q_access must be locked prior to calling this
    function.
========================================================*/
void ril_utf_send_msim_message(char *msg, int len)
{
  //TODO: add proper send mechanics
/*
  if (send(qmi_ril_msim_sock_id, (void *) msg, len, 0) < 0)
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error! Failed to send mesage to qmi ril msim socket.");
  }
*/
}

/*========================================================
  FUNCTION: utf_msim_recv_start
  - Starts the utf msim recv thread.
  - will wait for signal from UTF main test exec
    for an "expect" to send a message to qmi ril.

  Critical section access:
  - exec_q_access locked and unlocked here.
  - Note: pthread_wait will release exec_q_access
    when waiting, and locks again upon re-entering.
========================================================*/
void* utf_msim_recv_start( void *param)
{
  fd_set rfds;
  int ret_val;

  struct ipc_send_recv_data_info msim_recv_buffer;
  int msim_recv_byte_num;

  int i, my_expect_slot, x;
  struct timespec timeout;
  struct timeval e_timeout;
  struct timespec thread_timeout;
  e_timeout.tv_sec = 0;
  e_timeout.tv_usec = EXPECT_TIMEOUT;

  while( EXEC_MSIM_THREAD )
  {

    FD_ZERO(&rfds);
    FD_SET(msim_local_sock_id, &rfds);
    timeout.tv_sec = 0;
    timeout.tv_nsec = 10;


    ret_val = select(msim_local_sock_id+1, &rfds, NULL, NULL, &timeout);

    if ( ret_val == -1 )
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: MSIM recv thread error in select in func %s", __func__);
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Select returns error %d", ret_val);
      return NULL;
    }
    else if ( ret_val )
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: %s: MSIM recv thread receiving message", __func__);
      utf_msim_recv_msg(&msim_recv_buffer, &msim_recv_byte_num);

      // acquire mutex
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: MSIM recv thread locking exec_q_access in %s", __func__);
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
      handle_msim_msg(msim_recv_buffer, msim_recv_byte_num);

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
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: MSIM recv thread error in select (should not get here) in %s", __func__);
    }

    // Check for reboot request
    if (ril_utf_check_reboot_state() == 1)
    {
      ril_utf_msim_recv_thread_shutdown();
    }
  } // while( EXEC_MSIM_THREAD )

}

/*========================================================
  FUNCTION: utf_msim_recv_msg
    - Recieves all outstanding messages on the socket
      and modifies "msgs" to contain the size and ptr
      to messages in buffer received
  Critical section access:
    - none
========================================================*/
int utf_msim_recv_msg(struct ipc_send_recv_data_info* msim_recv_buffer_ptr,
                      int* msim_recv_byte_num_ptr)
{
  int ret = TRUE;
  char buff[30];
  int length = 0;

  struct sockaddr_un tmp_remote_addr;
  int source_addr_len = sizeof(tmp_remote_addr);

  (*msim_recv_byte_num_ptr) = 0;
  memset(&tmp_remote_addr, 0, sizeof(tmp_remote_addr));
  memset(msim_recv_buffer_ptr, 0, sizeof(*msim_recv_buffer_ptr));

  (*msim_recv_byte_num_ptr) = recvfrom(msim_local_sock_id, msim_recv_buffer_ptr, sizeof(*msim_recv_buffer_ptr),
                                0, (struct sockaddr*)&tmp_remote_addr, &source_addr_len);

  RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: %s: MSIM received %d bytes... dumping hex", __func__, *msim_recv_byte_num_ptr);
  ril_utf_print_hex(msim_recv_buffer_ptr, (*msim_recv_byte_num_ptr));
  if ( *msim_recv_byte_num_ptr == -1)
  {
    char buf[128];
    strerror_r(errno, buf, sizeof(buf));
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: %s: error receiving. errno: %d. str: %s", __func__, errno, buf);
    //while(1);
  }

  return ret;

}

/*========================================================
  FUNCTION: handle_msim_msg
    - decode incoming msim message
  Critical section access:
  - exec_q_access is expected to be locked prior to calling
    this function.
========================================================*/
void handle_msim_msg(struct ipc_send_recv_data_info msim_recv_buffer,
                      int msim_recv_byte_num)
{
  RIL_UTF_DEBUG("\nENTERING FUNCTION %s", __func__);
  enum ril_utf_q_element_t type;

  int payload_len = msim_recv_byte_num - sizeof(msim_recv_buffer.message_id) -
                    sizeof(msim_recv_buffer.rild_instance_id);;

  if (msim_recv_byte_num > 0)
  {
    char *tmp_payload = (char*) malloc(QCRIL_MAX_IPC_PAYLOAD_SIZE);
    memcpy(tmp_payload, msim_recv_buffer.payload, payload_len);

    // Add message to queue for test thread
    enqueue_msim_expect(payload_len, tmp_payload, msim_recv_buffer.message_id,
                        msim_recv_buffer.rild_instance_id, ril_utf_msim_recv_msg);
  }
  else
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: %s: Payload length received: %d\n",
        __func__,
        msim_recv_byte_num);
  }

  RIL_UTF_DEBUG("\nEXITING FUNCTION %s", __func__);
}


/*========================================================
  FUNCTION: enqueue_msim_expect
    - Add received msim message to queue for test thread
  Critical section access:
  - exec_q_access is expected to be locked prior to calling
    this function.
========================================================*/
void enqueue_msim_expect(int size, void *payload, int msg_id,
                         int rild_instance_id, enum ril_utf_q_element_t type)
{
  struct ril_utf_msg_t *expect_node =
    (struct ril_utf_msg_t *) malloc(sizeof(struct ril_utf_msg_t));
  expect_node->payload_len = size;
  expect_node->payload = payload;
  expect_node->msg_id = msg_id;
  expect_node->rild_instance_id = rild_instance_id;
  expect_node->type = type;

  enqueue_ril_utf_q_node(expect_node, &ril_utf_rec_head, &ril_utf_rec_tail);
}

void ril_utf_msim_recv_thread_shutdown()
{
  RIL_UTF_DEBUG("\nQMI_RIL_UTF: Executing msim recv thread shutdown");
  int sock_len = strlen(msim_remote_addr.sun_path) + sizeof(msim_remote_addr.sun_family);
  int err = sendto(msim_local_sock_id, "reset", (strlen("reset")+1), 0,
         (struct sockaddr *)&msim_remote_addr, sock_len);
  close(msim_local_sock_id);
  pthread_exit(NULL);
}


int send_ril_utf_msim_request( struct ril_utf_msg_t *test_step )
{
  int err_val = 0;
  struct ipc_send_recv_data_info msim_send_buffer;

  memset(&msim_send_buffer, 0, sizeof(msim_send_buffer));

  msim_send_buffer.rild_instance_id = test_step->rild_instance_id;
  msim_send_buffer.message_id = test_step->msg_id;
  memcpy(msim_send_buffer.payload, test_step->payload, test_step->payload_len);
  msim_send_buffer.payload_length = test_step->payload_len;
  msim_send_buffer.payload_length += sizeof(msim_send_buffer.message_id) + sizeof(msim_send_buffer.rild_instance_id);

  int sock_len = strlen(msim_remote_addr.sun_path) + sizeof(msim_remote_addr.sun_family);

  // Send message
  int err = sendto(msim_local_sock_id, &msim_send_buffer, msim_send_buffer.payload_length, 0,
         (struct sockaddr *)&msim_remote_addr, sock_len);


  // Inform test thread we are finished
  RIL_UTF_DEBUG("\n QMI_RIL_UTF: Setting inject_complete to TRUE in %s", __func__);
  inject_complete = 1;
  pthread_cond_signal(&inject_complete_sig);

  if (test_step) {
    if (test_step->payload) {
      free(test_step->payload);
      test_step->payload = NULL;
    }
    free(test_step);
  }
  return err_val;
}

