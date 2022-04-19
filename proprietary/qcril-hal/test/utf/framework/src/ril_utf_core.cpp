/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_core.c
  @brief   RIL UTF core declarations

  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#include "ril_utf_common_includes.h"
#include "ril_utf_core.h"
extern "C" {
#include "ril_utf_proxy_sim.h"
#include "ril_utf_i_qti_oem_hook_sim.h"
#include "ril_utf_qmi_sim.h"
}
#include "ril_utf_i_ims_radio_sim.h"
#ifndef RIL_UTF_IOE
#include "qmi_idl_lib.h"
#endif

#ifdef RIL_UTF_IOE
#include "mcm_sms_v01.h"
#define assert(x)
#define UTF_IOE_SOCKET_CONNECT_MAX_ATTEMPTS 100
#define UTF_IOE_SERVER_SOCKET_PATH "./qmi_csi_sock1"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
extern int ioe_srv_sock_id;
extern pthread_t ioe_srv_thrd_id;
#endif

#ifdef QMI_RIL_HAL_UTF
#include "qcril_eventlist.h"
#endif
/*========================================================
  Globals
========================================================*/
extern int dsi_cb_thread_ready;
extern pthread_cond_t wakeup_dsi_cb_thread;
extern struct sig_expect_pool_t wakeup_dsi_sync_signals[MAX_SIGNALS];
extern pthread_mutex_t main_thread_q_access;
extern pthread_mutex_t main_thread_ril_q_access;
extern pthread_mutex_t reset_flag_mutex;
extern boolean reset_now_flag;
extern struct ril_utf_msg_t *ril_utf_rec_head;
extern struct ril_utf_msg_t *ril_utf_rec_tail;
extern struct ril_utf_msg_t *ril_utf_inj_head;
extern struct ril_utf_msg_t *ril_utf_inj_tail;
extern struct ril_utf_q_t *exec_list_start;
extern struct ril_utf_q_t *exec_list_end;
extern pthread_cond_t wakeup_qmi_cb_thread;
extern pthread_cond_t wakeup_ims_send_thread;
extern pthread_cond_t wakeup_proxy_send_thread;
extern pthread_cond_t wakeup_oem_send_thread;
extern pthread_cond_t wakeup_ioe_client_send_thread;
extern pthread_cond_t callback_thread_request;
extern int ims_inject_pending;
extern int proxy_inject_pending;
extern int ioe_client_inject_pending;
extern int qmi_cb_thread_ready;
extern int oem_inject_pending;
extern struct ril_request_q_t ril_request_q;
extern int q_empty; // main thread Q for cbs
extern pthread_cond_t main_thread_q_ready;
extern pthread_t mt_id;
extern pthread_t rild_id;
extern pthread_t ims_recv_id;
extern pthread_t rild_recv_id;
extern pthread_t msim_recv_id;
extern pthread_t oem_send_id;
extern pthread_t oem_recv_id;
extern pthread_t callback_thread;

extern "C" int utf_free_all_qmi_ril_heap();

/*========================================================
   START. UTL support functions implementation.

   Functions:
   - print_msg_in_hex
   - print_header
   - validate_expecatation
   - verify_ril_message
========================================================*/

/*========================================================
  FUNCTION:print_msg_in_hex
  - print the provided message as a hexadecimal byte stream
========================================================*/
void print_msg_in_hex( const void *response, size_t responselen )
{
  int newline = 0;
  int idx = 0;

  uint8_t *temp = (uint8_t *)response;
  RIL_UTF_HTML_LOG("---> ------- Begin of QMI_RIL_UTL: Message ---------------------------------");
  if (responselen == 0) {
    RIL_UTF_HTML_LOG("---> Response Length = 0, no message\n");
  }
  else {
    char buffer [HEX_MSG_BUFFER_LEN];
    char t_buffer [HEX_MSG_BUFFER_LEN];
    memset(buffer, 0x00, sizeof(buffer));
    strcat(buffer, "@@@@@@---> ");

    //RIL_UTF_HTML_LOG_ORIG("\n@@@@@@---> ");
    for( idx = 0; idx < responselen; idx++, newline++ )
    {
      if ( newline == HEX_MSG_NUMBER_PER_LINE )
      {
        newline = 0;
        if (idx != responselen) {
          RIL_UTF_HTML_LOG_ORIG("\n%s", buffer);
          memset(buffer, 0x00, sizeof(buffer));
          strcat(buffer, "@@@@@@---> ");
          //RIL_UTF_HTML_LOG_ORIG("\n@@@@@@---> ");
        }
      }
      snprintf(t_buffer, sizeof(t_buffer), "%02X ", temp[idx]);
      strcat(buffer, t_buffer);
      //RIL_UTF_HTML_LOG_ORIG("%02X ", temp[idx]);
    }
    RIL_UTF_HTML_LOG_ORIG("\n%s", buffer);
  }
  RIL_UTF_HTML_LOG("---> ------- End of QMI_RIL_UTL: Message -----------------------------------");
}

/*========================================================
  FUNCTION:print_msg_in_tlv
  - print function to print QMI messages in TLVs
========================================================*/
void print_msg_in_tlv(int msg_id, enum ril_utf_qmi_srv_id_t qmi_srv_id, int client_id, enum ril_utf_q_element_t payload_type,
 const void *message, size_t message_len)
{
  //get service id and msg id
  void *dest;
  uint32_t tlv_stream_len, dst_encode_len;
  int32_t idl_err;
  qmi_idl_type_of_message_type message_type;

  if (payload_type == ril_utf_qmi_indication)
  {
    message_type = QMI_IDL_INDICATION;
  }
  else if (payload_type == ril_utf_qmi_request)
  {
    message_type = QMI_IDL_REQUEST;
  }
  else if (payload_type == ril_utf_qmi_response)
  {
    message_type = QMI_IDL_RESPONSE;
  }
  else
  {
    RIL_UTF_HTML_LOG("ERROR: MESSAGE IS NOT A QMI MESSAGE");
    return;
  }


  if (ril_utf_qmi_client_list[client_id][qmi_srv_id].user_handle == NULL)
  {
    RIL_UTF_HTML_LOG("ERROR: qmi client not yet initialized via qmi_client_init()");
    // this is acceptable due to this only happening for one message during boot
    return;
  }

  idl_err = qmi_idl_get_max_message_len( ril_utf_qmi_client_list[client_id][qmi_srv_id].user_handle->p_service,
                                         message_type,
                                         msg_id,
                                         &tlv_stream_len  );

  if (idl_err != QMI_IDL_LIB_NO_ERR)
  {
    RIL_UTF_HTML_LOG("ERROR: qmi_idl_get_max_message_len failed with error %d", idl_err);
    return;
  }

  dest = malloc(tlv_stream_len);
  idl_err = qmi_idl_message_encode( ril_utf_qmi_client_list[client_id][qmi_srv_id].user_handle->p_service,
                                    message_type,
                                    msg_id,
                                    message,
                                    message_len,
                                    dest,
                                    tlv_stream_len,
                                    (uint32_t*)&dst_encode_len );

  if (idl_err != QMI_IDL_LIB_NO_ERR)
  {
    RIL_UTF_HTML_LOG("ERROR: qmi_idl_message_encode failed with error %d", idl_err);
  }
  else
  {
    print_msg_in_hex( dest, dst_encode_len);
  }

  free(dest);
}

/*========================================================
  FUNCTION: print_db_payload
  - print payload for DB
========================================================*/
void print_db_msg(void* payload, int payload_len)
{
  db_node_t* ptr = (db_node_t*) payload;
  if (ptr->mcc)
  {
    RIL_UTF_HTML_LOG(" mcc %s", ptr->mcc);
  }
  if (ptr->mnc)
  {
    RIL_UTF_HTML_LOG(" mnc %s", ptr->mnc);
  }
  if (ptr->emergency_num)
  {
    RIL_UTF_HTML_LOG(" emergency_num %s", ptr->emergency_num);
  }
  if (ptr->service_state)
  {
    RIL_UTF_HTML_LOG(" service_state %s", ptr->service_state);
  }
  if (ptr->service)
  {
    RIL_UTF_HTML_LOG(" service %s", ptr->service);
  }
  if (ptr->ims_address)
  {
    RIL_UTF_HTML_LOG(" ims_address %s", ptr->ims_address);
  }
  if (ptr->iin)
  {
    RIL_UTF_HTML_LOG(" iin %s", ptr->iin);
  }
  if (ptr->roam)
  {
    RIL_UTF_HTML_LOG(" roam %s", ptr->roam);
  }
  if (ptr->property_name)
  {
    RIL_UTF_HTML_LOG(" property_name %s", ptr->property_name);
  }
  if (ptr->value)
  {
    RIL_UTF_HTML_LOG(" value %s", ptr->value);
  }
  if (strcmp(ptr->operator_type, ""))
  {
    RIL_UTF_HTML_LOG(" operator_type %s", ptr->operator_type);
  }
  RIL_UTF_HTML_LOG(" source %d", ptr->source);
  RIL_UTF_HTML_LOG(" action %d", ptr->action);
  RIL_UTF_HTML_LOG(" expected_ret %d", ptr->expected_ret);
}

/*========================================================
  FUNCTION: print_header
  - print a header.
========================================================*/
void print_header(void)
{
  RIL_UTF_ESSENTIAL("\n==================================================================================");
}


/*========================================================
   END. UTL support functions implementation.
========================================================*/

/*========================================================
  START. UTL core implementation

  FUNCTIONS:
    - ril_utf_qmi_init
    - ril_utf_init_mutexes
    - add_to_execute_list
    - inject
    - expect
    - is_terminate_test_on_expectation_failure
    - calc_timeout
    - check_expectation_validity
    - print_expectation
    - handle_expect_step
    - check_inject_validity
    - send_ril_utf_ril_request
    - send_ril_utf_qmi_response
    - send_ril_utf_qmi_indication
    - handle_inject_step
    - clear_exec_list
    - execute
    - get_exec_list_node
========================================================*/

/*========================================================
  FUNCTION: ril_utf_qmi_init
  - qmi globals init

  Critical section access:
  - exec_q_access must be locked prior to calling this
    function for access to qmi globals.
========================================================*/
#ifndef UTF_TARGET_BUILD
// REMOVE FEATURE FLAGS WHEN ADDING IN QMI SIDE TO TARGET BUILD
void ril_utf_qmi_init()
{
 qmi_client_info.next = NULL;
 qmi_client_info.init_client_handle = QCRIL_UNITTEST_DEFAULT_QMI_CLIENT;
 qmi_client_info.sys_event_rx_hdlr = NULL;
 qmi_client_info.sys_event_user_data = NULL;

 ril_utf_qmi_client_list[0][QMI_CTL].user_handle = (qmi_client_type) malloc(sizeof(struct qmi_client_struct));
 ril_utf_qmi_client_list[0][QMI_CTL].user_handle->service_user_handle = 0;
 ril_utf_qmi_client_list[0][QMI_CTL].user_handle->p_service = ctl_get_service_object_v01();

 ril_utf_qmi_client_list[1][QMI_CTL].user_handle = (qmi_client_type) malloc(sizeof(struct qmi_client_struct));
 ril_utf_qmi_client_list[1][QMI_CTL].user_handle->service_user_handle = 1;
 ril_utf_qmi_client_list[1][QMI_CTL].user_handle->p_service = ctl_get_service_object_v01();

 ril_utf_qmi_client_list[0][QMI_VOICE].user_handle = (qmi_client_type) malloc(sizeof(struct qmi_client_struct));
 ril_utf_qmi_client_list[0][QMI_VOICE].user_handle->service_user_handle = 0;
 ril_utf_qmi_client_list[0][QMI_VOICE].user_handle->p_service = voice_get_service_object_v02();
}
#endif

/*========================================================
  FUNCTION: ril_utf_init_mutexes
  - init mutexes "exec_q_access" and "main_thread_q_access"
  - locking protocol:
      - make sure the exec_q_access is always locked first.

  Critical section access:
  - None
========================================================*/
void ril_utf_init_mutexes()
{
  pthread_mutex_init( &exec_q_access, NULL);
  pthread_mutex_init( &main_thread_q_access, NULL);
  pthread_mutex_init( &main_thread_ril_q_access, NULL);
  pthread_mutex_init( &reset_flag_mutex, NULL);
  #ifdef RIL_UTF_IOE
  pthread_mutex_init(&ioe_table_access, NULL );
  #endif
}

void ril_utf_init_sig_pool()
{
  int i;

  for ( i = 0; i < MAX_SIGNALS; ++i )
  {
    wakeup_qmi_sync_signals[i].msg_id = -1;
    wakeup_qmi_sync_signals[i].service_id = -1;
    wakeup_qmi_sync_signals[i].client_id = -1;
    wakeup_qmi_sync_signals[i].in_use = 0;
    pthread_cond_init(&(wakeup_qmi_sync_signals[i].signal_exptectation), NULL);
  }
}

/*========================================================
  FUNCTION: add_to_execute_list
  - Get a node in the execution list and add it
    to the end of the execution list

  Critical section access:
  - exec_q_access must be locked prior to calling this
    function for access to qmi globals.
========================================================*/
void add_to_execute_list(struct ril_utf_q_t *node)
{
  if ( exec_list_end )
  {
    exec_list_end->next = node;
    node->prev = exec_list_end;
    exec_list_end = node;
  }
  else
  {
    exec_list_end = exec_list_start = node;
  }
}


/*========================================================
  FUNCTION: calc_timeout
  - Calculate timeout based on provided "struct timeval" based
    timeout.
  - gets the current time of day, and adds the input timeout
    to obtain timeout in the "struct timespec" format.

  Critical section access:
  - None
========================================================*/
void calc_timeout(struct timespec *wait_timeout, struct timeval *timeout)
{
  struct timeval    ctime;
  gettimeofday(&ctime, NULL);

  int seconds = (ctime.tv_usec + timeout->tv_usec ) / MICROSECONDS;

  if ( seconds >= 1)
  {
    wait_timeout->tv_sec = ctime.tv_sec + timeout->tv_sec + seconds;
    wait_timeout->tv_nsec = ((ctime.tv_usec + timeout->tv_usec ) - (seconds * MICROSECONDS)) * 1000;
  }
  else
  {
    wait_timeout->tv_sec  = ctime.tv_sec + timeout->tv_sec;
    wait_timeout->tv_nsec = (ctime.tv_usec + timeout->tv_usec) * 1000;
  }
}

/*========================================================
  FUNCTION: send_ril_utf_ril_request
  - Add a ril request to the main thread dispatch q
  - send signal to the main thread to wake up.

  Critical section access:
  - exec_q_access is both locked and unlocked here.
========================================================*/
int send_ril_utf_ril_request(struct ril_utf_msg_t *test_step)
{
  int err_val = QCRIL_UNITTTEST_NO_ERR;
  int sig_err = QCRIL_UNITTTEST_NO_ERR;

  //========================================================
  //   acquire lock for main_thread
  //   ril_request_q_ptr is shared between this function
  //   and the main thread that dispatches the requests
  //========================================================
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: locking main_thread_q_access");
  MAIN_THRD_RIL_REQUEST_Q_LOCK();

  //========================================================
  //   update ril_request_q_ptr
  //   We can do the below assignment safely, since we expect
  //   exec_q_access to have been locked prior to this function
  //   call.
  //========================================================
  if (ril_request_q.head == NULL)
  {
    ril_request_q.head = test_step;
    ril_request_q.tail = test_step;
    test_step->next = NULL;
    test_step->prev = NULL;
  }
  else
  {
    ril_request_q.tail->next = test_step;
    test_step->next = NULL;
    test_step->prev = ril_request_q.tail;
    ril_request_q.tail = test_step;
  }

  // Set flag for pending messages
  q_empty = FALSE;

  //========================================================
  //  signal the "main thread"
  //  Although signalled here, the "main thread" will block on
  //  exec_q_access mutex, which will be released when the
  //  execution queue waits for the next expect.
  //========================================================
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Waking up main thread");
  sig_err = pthread_cond_signal(&main_thread_q_ready);

  if ( sig_err != QCRIL_UNITTTEST_NO_ERR )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Failed to wake up main thread.");
  }

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: unlocking main_thread_q_access");
  MAIN_THRD_RIL_REQUEST_Q_UNLOCK();

  return err_val;
}

/*========================================================
  FUNCTION: send_ril_utf_ril_request
  - Add a ril request to the main thread dispatch q
  - send signal to the main thread to wake up.
  - TODO: implement this function

  Critical section access:
  - exec_q_access must be locked prior to calling this func
========================================================*/
int send_ril_utf_qmi_response(struct ril_utf_msg_t *test_step)
{
  int err_val = QCRIL_UNITTTEST_NO_ERR;
  int sig_err = QCRIL_UNITTTEST_NO_ERR;
  int i;
  int is_sync_response = 0;

  /*========================================================
     update inject queue
     We can do the below assignment safely, since we expect
     exec_q_access to have been locked prior to this function
     call.
  ==========================================================*/
  enqueue_ril_utf_q_node(test_step, &ril_utf_inj_head, &ril_utf_inj_tail);

  /*========================================================
    signal the qmi_cb_thread.
    Although signalled here, the qmi_cb thread will block on
    exec_q_access mutex, which will be released when the
    execution queue waits for the next expect.
  ==========================================================*/

  // find corresponding expect
  for ( i = 0; i < MAX_EXPECTS; ++i )
  {
    if (expect_table[i].in_use == 0)
    {
      break;
    }
    if ( (expect_table[i].service_id == test_step->service_id) &&
         (expect_table[i].msg_id == test_step->msg_id) &&
         (expect_table[i].client_id == test_step->client_id) &&
         (expect_table[i].is_sync == 1) )
    {
      is_sync_response = 1;
      break;
    }
  }

  if ( is_sync_response == 1 )
  {
    for ( i = 0; i < MAX_SIGNALS; ++i )
    {
      if ( (wakeup_qmi_sync_signals[i].in_use == 1) &&
           (wakeup_qmi_sync_signals[i].msg_id == test_step->msg_id) &&
           (wakeup_qmi_sync_signals[i].service_id == test_step->service_id) &&
           (wakeup_qmi_sync_signals[i].client_id == test_step->client_id) )
      {
        break;
      }
    }
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Waking up sync request with signal slot %d\n", i);
    sig_err = pthread_cond_signal(&(wakeup_qmi_sync_signals[i].signal_exptectation));
  }
  else
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Waking up qmi_cb thread.");
    // Set predicate to signal wakeup
    qmi_cb_thread_ready = 1;
    sig_err = pthread_cond_signal(&wakeup_qmi_cb_thread);
  }

  if ( sig_err != QCRIL_UNITTTEST_NO_ERR )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Failed to wake up qmi_cb thread.");
  }

  return err_val;
}

#ifdef RIL_UTF_IOE
/*========================================================
  FUNCTION: send_ril_utf_ioe_request
  - Send an ioe request to the IOE Server thread.

  Critical section access:
  - exec_q_access must be locked prior to calling this func
========================================================*/
int send_ril_utf_ioe_request(struct ril_utf_msg_t *test_step)
{

  int err_val = QCRIL_UNITTTEST_NO_ERR;
  int sig_err = QCRIL_UNITTTEST_NO_ERR;
  uint8_t *send_message_buffer;

  //Calculate the buffer len
  // req_cb id(1)+ client_handle(1) + masg_id(4) + payload_len(4)
  int buffer_len = UTF_IOE_REQ_CB_BUFFER_MIN_FIXED_LEN + test_step->payload_len /*req_str_payload*/;

  send_message_buffer = (uint8_t*) malloc(buffer_len);
  RIL_UTF_DEBUG("mallocccccccccc buff len: %d\n",buffer_len);
  //Form the buffer
  int i = 0;
  send_message_buffer[i++] = UTF_IOE_CLIENT_REQ_CB_ID;
  send_message_buffer[i++] = UTF_IOE_DEFAULT_CLIENT_HANDLE; //client handle is 0 for now
  uint32_t temp = (uint32_t) test_step->msg_id;
  memcpy(send_message_buffer + i, &temp, sizeof(uint32_t));
  i+= sizeof(uint32_t);
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: msg id and size %d %d", test_step->msg_id, sizeof(test_step->msg_id));

   temp = (uint32_t)test_step->payload_len;
  RIL_UTF_DEBUG("TEMP!!! %d\n",temp);

   if(temp != 0)
   {
  memcpy(send_message_buffer + i, &temp, sizeof(uint32_t));
  i+= sizeof(uint32_t);
  uint8_t *payld;
  payld = (uint8_t*) malloc(test_step->payload_len);
  payld  = (uint8_t*)test_step->payload;
  memcpy(send_message_buffer + i, payld, test_step->payload_len);
  }
  else
  {
  int j = 0;
  for(j = 0; j< 4; j++)
  send_message_buffer[i++] = 0;
  }

  if (test_step) {
    if (test_step->payload) {
      free(test_step->payload);
      test_step->payload = NULL;
    }
    free(test_step);
  }
  enqueue_ioe_client_msg_to_send(send_message_buffer, buffer_len);
  ioe_client_inject_pending = TRUE;
  sig_err = pthread_cond_signal(&wakeup_ioe_client_send_thread);

  if(sig_err != 0)
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Failed to wake up ioe_client send thread");
  }

  return err_val;
}
#endif

int send_ril_utf_qmi_indication(struct ril_utf_msg_t *test_step)
{
  int err_val = QCRIL_UNITTTEST_NO_ERR;
  int sig_err = QCRIL_UNITTTEST_NO_ERR;
  /*========================================================
     update qmi_sync_resp queue
     We can do the below assignment safely, since we expect
     exec_q_access to have been locked prior to this function
     call.
  ==========================================================*/
  enqueue_ril_utf_q_node(test_step, &ril_utf_inj_head, &ril_utf_inj_tail);

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Waking up qmi_cb thread for qmi indication.");
  // Set predicate to signal wakeup
  qmi_cb_thread_ready = 1;
  pthread_cond_signal(&wakeup_qmi_cb_thread);

  if ( sig_err != QCRIL_UNITTTEST_NO_ERR )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Failed to wake up qmi_cb thread.");
  }

  return err_val;
}


int send_ril_utf_dsi_indication(struct ril_utf_msg_t *test_step)
{
  int err_val = QCRIL_UNITTTEST_NO_ERR;
  int sig_err = QCRIL_UNITTTEST_NO_ERR;
  /*========================================================
     update qmi_sync_resp queue
     We can do the below assignment safely, since we expect
     exec_q_access to have been locked prior to this function
     call.
  ==========================================================*/
  enqueue_ril_utf_q_node(test_step, &ril_utf_inj_head, &ril_utf_inj_tail);

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Waking up qmi_cb thread for qmi indication.");
  // Set predicate to signal wakeup
  dsi_cb_thread_ready = 1;
  pthread_cond_signal(&wakeup_dsi_cb_thread);

  if ( sig_err != QCRIL_UNITTTEST_NO_ERR )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Failed to wake up qmi_cb thread.");
  }

  return err_val;
}

int send_ril_utf_dsi_response(struct ril_utf_msg_t *test_step)
{
  int err_val = QCRIL_UNITTTEST_NO_ERR;
  int sig_err = QCRIL_UNITTTEST_NO_ERR;
  int i;
  int is_sync_response = 0;

  /*========================================================
     update inject queue
     We can do the below assignment safely, since we expect
     exec_q_access to have been locked prior to this function
     call.
  ==========================================================*/
  enqueue_ril_utf_q_node(test_step, &ril_utf_inj_head, &ril_utf_inj_tail);

  // find corresponding expect
  for ( i = 0; i < MAX_EXPECTS; ++i )
  {
    if (expect_table[i].in_use == 0)
    {
      break;
    }
    if ( (expect_table[i].msg_id == test_step->msg_id) &&
         (expect_table[i].is_sync == 1) )
    {
      is_sync_response = 1;
      break;
    }
  }

  if ( is_sync_response == 1 )
  {
    for ( i = 0; i < MAX_SIGNALS; ++i )
    {
      if ( (wakeup_dsi_sync_signals[i].in_use == 1) &&
           (wakeup_dsi_sync_signals[i].msg_id == test_step->msg_id) )
      {
        break;
      }
    }
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Waking up sync request with signal slot %d\n", i);
    sig_err = pthread_cond_signal(&(wakeup_dsi_sync_signals[i].signal_exptectation));
  }

  if ( sig_err != QCRIL_UNITTTEST_NO_ERR )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Failed to wake up qmi_cb thread.");
  }

  return err_val;
}

int send_ril_utf_proxy_qmi_request( struct ril_utf_msg_t *test_step )
{
  int err_val = QCRIL_UNITTTEST_NO_ERR;
  int sig_err = QCRIL_UNITTTEST_NO_ERR;
  int header_size;
  int encoded_tlv_stream_len = 0;
  uint32_t max_msg_len = 0;
  int wire_format_msg_len = 0;

  //get service id and msg id
  int client_id  = test_step->client_id;
  int service_id = test_step->service_id;
  int msg_id = test_step->msg_id;
  uint8_t *send_msg_buf;
  int32_t idl_err;
  int err;

  header_size = ( service_id == (int)QMI_CTL) ? QMI_HEADER_SIZE_CTL : QMI_HEADER_SIZE_DEFAULT ;

  if ( service_id == QMI_CTL )
  {
    encoded_tlv_stream_len = test_step->payload_len;

    send_msg_buf = (uint8_t*) malloc(encoded_tlv_stream_len + header_size);
    memset(send_msg_buf, 0x00, encoded_tlv_stream_len + header_size);

    memcpy((void*)(send_msg_buf+header_size), test_step->payload, encoded_tlv_stream_len);
  }
  else
  {
    err = qmi_idl_get_max_message_len( ril_utf_qmi_client_list[client_id][service_id].user_handle->p_service,
                                           QMI_IDL_REQUEST,
                                           msg_id,
                                           &max_msg_len  );


    send_msg_buf = (uint8_t*) malloc(max_msg_len + header_size);

    memset(send_msg_buf, 0x00, max_msg_len + header_size);

    err = qmi_idl_message_encode( ril_utf_qmi_client_list[client_id][service_id].user_handle->p_service,
                                      QMI_IDL_REQUEST,
                                      msg_id,
                                      test_step->payload,
                                      test_step->payload_len, //TBD:use max msg len
                                      (void*)(send_msg_buf+header_size),
                                      max_msg_len,
                                      (uint32_t*)&encoded_tlv_stream_len );
  }

  wire_format_msg_len = encoded_tlv_stream_len + header_size;

  pack_qmi_msg_header(send_msg_buf, test_step, encoded_tlv_stream_len);

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: sending request: service_id: %d, msg_id: %d", service_id, msg_id);

  print_msg_in_hex(send_msg_buf, wire_format_msg_len );

#ifndef RIL_UTF_IOE
  enqueue_proxy_msg_to_send(send_msg_buf, wire_format_msg_len );
#endif

  proxy_inject_pending = TRUE;
  sig_err = pthread_cond_signal(&wakeup_proxy_send_thread);

  if (sig_err != 0)
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Failed to wake up qmi_cb thread.");
  }

  // Free C struct payload and add in encoded payload
  if (test_step) {
    if (test_step->payload) {
      free (test_step->payload);
      test_step->payload = NULL;
    }
    free (test_step);
  }

  return err_val;
}

int send_ril_utf_db_request (struct ril_utf_msg_t *test_step)
{
  RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Handling DB inject");
  inject_complete = 1;
  pthread_cond_signal(&inject_complete_sig);
  //TODO
  db_node_t* ptr = (db_node_t*) test_step->payload;
  int ret = 0;
  switch (ptr->action)
  {
#ifdef RIL_UTF_L_MASTER
    case UTF_DB_INSERT_OPERATOR_TYPE:
    {
      ret = qcril_db_insert_operator_type(ptr->mcc, ptr->mnc, ptr->operator_type);
      RIL_UTF_ESSENTIAL("\n return value for qcril_db_insert_operator_type(%s, %s, %s): %d", ptr->mcc, ptr->mnc, ptr->operator_type, ret);
      if (ret == ptr->expected_ret)
      {
        ret = 0;
      }
    }
      break;
    case UTF_DB_INSERT_PROPERTIES_TABLE:
    {
      ret = qcril_db_insert_properties_table(ptr->property_name, ptr->value);
      RIL_UTF_ESSENTIAL("\n return value for qcril_db_insert_property_table(%s, %s): %d", ptr->property_name, ptr->value, ret);
      if (ret == ptr->expected_ret)
      {
        ret = 0;
      }
    }
      break;
#endif
    default:
      break;
  }

  if (test_step) {
    if (test_step->payload) {
      free(test_step->payload);
      test_step->payload = NULL;
    }
    free(test_step);
  }

  return ret;
}

int send_ril_utf_db_response (void* payload, int payload_len)
{
  db_node_t* ptr = (db_node_t*) payload;
  int ret = 0;
  char emergency_num[QCRIL_MAX_EMERGENCY_NUMBERS_LEN] = {0};
  switch (ptr->action)
  {
    case UTF_DB_IS_MCC_PART_OF_EMERGENCY_NUMBER_TABLE:
    {
      ret = qcril_db_is_mcc_part_of_emergency_numbers_table(ptr->source, TRUE, ptr->mcc, FALSE, ptr->mnc, emergency_num);
      RIL_UTF_ESSENTIAL("\n return value for qcril_db_is_mcc_part_of_emergency_numbers_table(%d, %s, %s): %d", ptr->source, ptr->mcc, ptr->mnc, ret);
      if (ret == ptr->expected_ret)
      {
        ret = 0;
      }
      break;
    }
#ifdef RIL_UTF_L_MASTER
    case UTF_DB_QUERY_OPERATOR_TYPE:
    {
      char operator_type[QCRIL_DB_MAX_OPERATOR_TYPE_LEN];
      memset(operator_type, 0, QCRIL_DB_MAX_OPERATOR_TYPE_LEN);
      RIL_UTF_ESSENTIAL("\n operator_type for qcril_db_query_operator_type(%s, %s, %s): %s", ptr->mcc, ptr->mnc, ptr->operator_type, operator_type);
      qcril_db_query_operator_type(ptr->mcc, ptr->mnc, operator_type);
      RIL_UTF_ESSENTIAL("\n operator_type for qcril_db_query_operator_type(%s, %s, %s): %s", ptr->mcc, ptr->mnc, ptr->operator_type, operator_type);
      if (strcmp(ptr->operator_type, operator_type))
      {
        ret = 1;
      }
      break;
    }
#endif
    default:
      break;
  }
  return ret;
}

int send_ril_utf_property_request (struct ril_utf_msg_t *test_step)
{
  int ret;

  property_node_t* ptr = (property_node_t*) test_step->payload;
  ret = property_set(ptr->key, ptr->value);

  inject_complete = 1;
  pthread_cond_signal(&inject_complete_sig);

  if (test_step) {
    if (test_step->payload) {
      free(test_step->payload);
      test_step->payload = NULL;
    }
    free(test_step);
  }

  return ret;
}

int send_ril_utf_property_response (void* payload, int payload_len)
{
  int ret = 0;
  property_node_t* ptr = (property_node_t*) payload;
  char value[QCRIL_TEST_PROPERTY_VALUE_LEN];
  property_get(ptr->key, value, "None");
  if (strncmp(ptr->value, value, sizeof(ptr->value)))
  {
    ret = 1;
  }
  return ret;
}

int pack_qmi_msg_header(uint8_t *send_buf, struct ril_utf_msg_t *test_step, int tlv_stream_len)
{
  int header_size =  ( test_step->service_id == (int)QMI_CTL) ? QMI_HEADER_SIZE_CTL : QMI_HEADER_SIZE_DEFAULT ;
  int qmux_pdu_len = tlv_stream_len + header_size - UTF_QMI_QMUX_IF_LEN; // len + header - I/F
  int idx=0;

  //=====================================
  //  QMI Msg header: PDU
  //=====================================
  send_buf[0] = 1; // I/F: Always 1
  send_buf[1] = qmux_pdu_len & 0xff;
  send_buf[2] = (qmux_pdu_len & 0xff00) >> 8;
  send_buf[3] = UTF_QMI_PDU_CONTROL_POINT; // TBD:Will be srv point for resp/ind
  send_buf[4] = test_step->service_id; //QMUX type = srv id
  send_buf[5] = test_step->client_id;  //client id

  //=====================================
  //  QMI Msg header: SDU
  //=====================================
  send_buf[6] = UTF_QMI_SDU_CTL_REQUEST;  // TBD: also add resp/ind
  send_buf[7] = 0; // tx id byte 1: hardcoded value for UTF

  if ( test_step->service_id == QMI_CTL )
  {
    // QMI_CTL SDU header has a 1 byte tx id
    idx = 8; //resume from 9th byte
  }
  else
  {
    // All other SDU headers have a 2 byte tx id
    send_buf[8] = 0; // tx id  byte 2
    idx = 9; //resume from 10th byte
  }

    send_buf[idx++] = test_step->msg_id & 0xff;
    send_buf[idx++] = (test_step->msg_id & 0xff00) >> 8;
    send_buf[idx++] = tlv_stream_len & 0xff;
    send_buf[idx++] = (tlv_stream_len & 0xff00) >> 8;

    return idx; //returns the number of bytes packed
}


void clear_exec_list()
{
  //loop through the exec list and delete all test points
}


elnptr get_exec_list_node()
{
  elnptr node;

  node = (elnptr) malloc(sizeof(struct ril_utf_q_t));

  memset((void *)node, 0x00,sizeof(struct ril_utf_q_t));

  //set default timeout (40 msec)
  node->e.status = not_met;
  node->e.timeout.tv_usec = 40000; //40 milli seconds

  return node;
}


/*========================================================
  FUNCTION:
   qcril_get_time_milliseconds
   - Gets prints the current time
=========================================================*/
uint32_t qcril_get_time_milliseconds()
{
  struct timeval current_time;
  gettimeofday(&current_time, NULL);
  return ( current_time.tv_sec * 1000 + current_time.tv_usec / 1000);
}

/*========================================================
  FUNCTION:
   remove_ril_utf_q_node
   - deletes a node from the qmi sync/async
    pending expect queue
=========================================================*/
void remove_ril_utf_q_node( struct ril_utf_msg_t* node,
                            struct ril_utf_msg_t** head,
                            struct ril_utf_msg_t** tail)
{
  if ( node->next == NULL )
  {
    *tail = node->prev;
  }
  else
  {
    node->next->prev = node->prev;
  }

  if ( node->prev == NULL )
  {
    *head = node->next;
  }
  else
  {
    node->prev->next = node->next;
  }
}

/*========================================================
  FUNCTION:
   enqueue_ril_utf_q_node
   - enqueues a node from the qmi sync/async
    pending expect queue
=========================================================*/
void enqueue_ril_utf_q_node( struct ril_utf_msg_t* node,
                             struct ril_utf_msg_t** head,
                             struct ril_utf_msg_t** tail)
{
    node->prev = NULL;
    node->next = NULL;
    if ( *head != NULL)
    {
      (*tail)->next = node;
      node->prev = *tail;
      *tail = node;
    }
    else
    {
      *head = node;
      *tail = node;
    }
}

/*========================================================
  FUNCTION:
   pop_front_ril_utf_q_node
   - enqueues a node from the qmi sync/async
    pending expect queue
=========================================================*/
struct ril_utf_msg_t* pop_front_ril_utf_q_node
  ( struct ril_utf_msg_t** head,
    struct ril_utf_msg_t** tail )
{
  if ( *head == NULL )
  {
    return NULL;
  }
  else
  {
    struct ril_utf_msg_t* tmp = *head;
    *head = tmp->next;
    if (*head == NULL)
    {
      *tail = NULL;
    }
    return tmp;
  }
}

int clear_inject_expect_queues()
{
  struct ril_utf_msg_t *temp;

  while (ril_utf_rec_head != NULL)
  {
    temp = ril_utf_rec_head;
    ril_utf_rec_head = ril_utf_rec_head->next;
    if (temp->payload)
    {
      free(temp->payload);
      temp->payload = NULL;
    }
    free(temp);
  }
  ril_utf_rec_head = NULL;
  ril_utf_rec_tail = NULL;

  while (ril_utf_inj_head != NULL)
  {
    temp = ril_utf_inj_head;
    ril_utf_inj_head = ril_utf_inj_head->next;
    if (temp->payload)
    {
      free(temp->payload);
      temp->payload = NULL;
    }
    free(temp);
  }
  ril_utf_inj_head = NULL;
  ril_utf_inj_tail = NULL;

  return 0;
}

/*========================================================
  FUNCTION:
   ril_utf_init
   - print headers
   - initialize UTL mutexes
   - initialize UTL QMI globals
=========================================================*/
void ril_utf_init()
{
  uint32_t t_milliseconds = qcril_get_time_milliseconds();
  RIL_UTF_DEBUG("\n start: %d\n", (int)t_milliseconds);
  print_header();
  RIL_UTF_HTML_LOG_ORIG("\n     ******QMI RIL Unit Test*******");
  print_header();

  ril_utf_init_mutexes();
  ril_utf_init_sig_pool();
#ifndef UTF_TARGET_BUILD
  // Initialize test specific entities
  ril_utf_qmi_init();
#endif
}

/*========================================================
  FUNCTION:
   qmi_proxy_utf_init
   - print headers
   - initialize UTL mutexes
   - initialize UTL QMI globals
=========================================================*/
void qmi_proxy_utf_init()
{
  uint32_t t_milliseconds = qcril_get_time_milliseconds();
  RIL_UTF_DEBUG("\n start: %d\n", (int)t_milliseconds);
  print_header();
  RIL_UTF_HTML_LOG_ORIG("\n     ******QMI PROXY Unit Test*******");
  print_header();

  ril_utf_init_mutexes();
  ril_utf_init_sig_pool();
  // Initialize test specific entities
  ril_utf_qmi_init();
}
/*========================================================
  FUNCTION:
   ioe_utf_init
   - print headers
   - initialize UTL mutexes
   - initialize UTL QMI globals
=========================================================*/
void ioe_utf_init()
{
  uint32_t t_milliseconds = qcril_get_time_milliseconds();
  RIL_UTF_DEBUG("\n start: %d\n", (int)t_milliseconds);
  print_header();
  RIL_UTF_HTML_LOG_ORIG("\n     ******IoE Unit Test*******");
  print_header();

  ril_utf_init_mutexes();
  ril_utf_init_sig_pool();

  // Initialize test specific entities
  ril_utf_qmi_init();
}

/*========================================================
  FUNCTION:
   ril_utf_connect_to_ioe_server
   - Attempt to establish a connection to the IOE server
     socket
=========================================================*/
#ifdef RIL_UTF_IOE
void ril_utf_connect_to_ioe_server(void)
{
  utf_start_ioe_client_threads();
}

void utf_start_ioe_server(void)
{
  pthread_attr_t ioe_srv_thread_attr;
  int ret;

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Starting IOE server thread...");
  pthread_attr_init(&ioe_srv_thread_attr);
  ret = pthread_create(&ioe_srv_thrd_id, &ioe_srv_thread_attr, start_ioe_server, NULL);
  int cl = qmi_init(NULL, NULL);
  RIL_UTF_DEBUG("ioe server started!!!");
  if ( ret < 0 )
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Failed to start IOE server thread. Exiting...");
    return;
  }
}
#endif

/*========================================================
  FUNCTION:
   ril_utf_format_log_msg
   - formats the log message to the format desired..
=========================================================*/
void ril_utf_format_log_msg
(
  char *buf_ptr,
  int buf_size,
  char *fmt,
  ...
)
{
  va_list ap;

  /*-----------------------------------------------------------------------*/

  assert( buf_ptr != NULL );
  assert( buf_size > 0 );

  /*-----------------------------------------------------------------------*/
  va_start( ap, fmt );

  vsnprintf( buf_ptr, buf_size, fmt, ap);

  va_start(ap, fmt);

  va_end( ap );

} /* ril_utf_format_log_msg */

/*========================================================
  FUNCTION:
   ril_utf_format_log_msg
   - if thread exists with the thread_id, points the name to thread_name
=========================================================*/
int ril_utf_get_thread_name(pthread_t thread_id, char *ril_utf_thread_name)
{
    int iter_i = 0,res = FALSE;

    for(iter_i = 0; iter_i < RIL_UTF_THREAD_INFO_MAX_SIZE; iter_i++)
    {
        if(TRUE == ril_utf_thread_name_info[iter_i].is_valid && thread_id == ril_utf_thread_name_info[iter_i].thread_id)
        {
            strlcpy(ril_utf_thread_name, ril_utf_thread_name_info[iter_i].ril_utf_thread_name, RIL_UTF_THREAD_NAME_MAX_SIZE);
            res = TRUE;
            break;
        }
    }

    return res;
}
#ifndef RIL_UTF_IOE
typedef struct
{
    int is_valid;
    pthread_t thread_id;
    char thread_name[QMI_RIL_THREAD_NAME_MAX_SIZE];
} qmi_ril_thread_name_info_type;

extern qmi_ril_thread_name_info_type qmi_ril_thread_name_info[QMI_RIL_THREAD_INFO_MAX_SIZE];

/*========================================================
  FUNCTION:
   ril_utf_reboot
    - Kills all threads associated with tested code
      and resets all global data variables
=========================================================*/
int qmi_ril_utf_shutdown()
{
  int i;
  int ret_val;

  // Set reset state
  pthread_mutex_lock(&reset_flag_mutex);
  reset_now_flag = 1;
  pthread_mutex_unlock(&reset_flag_mutex);

  reset_i_ims_radio();
  reset_i_qti_oem_hook();

#ifndef UTF_TARGET_BUILD
#ifndef QMI_RIL_HAL_UTF
  // Request event thread shutdown
  qcril_event_queue( QCRIL_DEFAULT_INSTANCE_ID, QCRIL_DEFAULT_MODEM_ID,
                     QCRIL_DATA_NOT_ON_STACK, -1,
                     NULL, 0,
                     (RIL_Token) QCRIL_TOKEN_ID_INTERNAL );
#endif

  //qmi_ril_threads_shutdown();

  // Wake up cb thread to shutdown
  pthread_cond_signal(&callback_thread_request);

  // Join all qcril threads made with UTF thread handler
  utf_join_threads();

  if (callback_thread) {
    pthread_join(callback_thread, NULL);
    callback_thread = 0;
  }
  if (oem_send_id) {
    pthread_join(oem_send_id, NULL);
    oem_send_id = 0;
  }
  if (oem_recv_id) {
    pthread_join(oem_recv_id, NULL);
    oem_recv_id = 0;
  }
  if (qmit_id) {
    pthread_join(qmit_id, NULL);
    qmit_id = 0;
  }
  if (rild_id) {
    pthread_join(rild_id, NULL);
    rild_id = 0;
  }
  if (mt_id) {
    pthread_join(mt_id, NULL);
    mt_id = 0;
  }

#endif

#ifndef UTF_TARGET_BUILD
  // Clean up global variables and resources
  //qmi_ril_reboot_cleanup();

  // Check that all heap allocation is freed
  utf_free_all_qmi_ril_heap();

  // Reset properties
  utf_property_init();
#endif

  // Clean up queues
  clear_inject_expect_queues();
  clear_pending_qmi_resp_q();
  clear_qmi_clients();

  // Set reset state to off
  pthread_mutex_lock(&reset_flag_mutex);
  reset_now_flag = 0;
  pthread_mutex_unlock(&reset_flag_mutex);

  return 0;
}

int ril_utf_check_reboot_state()
{
  int ret_val = 0;
  // Check for reset and quit if flag is set
  pthread_mutex_lock(&reset_flag_mutex);
  if (reset_now_flag != 0)
  {
    ret_val = 1;
  }
  pthread_mutex_unlock(&reset_flag_mutex);

  return ret_val;
}

#endif
