/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_qmi_sim.c
  @brief   RIL UTF QMI simulation

  DESCRIPTION
   Simulated QMI interfaces for RIL UTF
  ---------------------------------------------------------------------------
******************************************************************************/
#include "ril_utf_qmi_sim.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/select.h>
/*========================================================
 * Global Variables
========================================================*/
extern struct ril_utf_expect_t *ril_utf_rec_head;
extern struct ril_utf_expect_t *ril_utf_rec_tail;
extern struct ril_utf_msg_t *ril_utf_inj_head;
extern struct ril_utf_msg_t *ril_utf_inj_tail;
extern struct ril_utf_qmi_resp_pending_q_t *qmi_resp_pending_list_start;
extern struct ril_utf_qmi_resp_pending_q_t *qmi_resp_pending_list_end;
extern pthread_cond_t wakeup_qmi_cb_thread;
extern pthread_cond_t reset_flag_mutex;
extern pthread_cond_t qmi_thrd_complete_sig;
extern boolean reset_now_flag;
extern struct ril_request_q_t ril_request_q;
extern int global_timeout_max;
extern int suspend_timeouts;
extern int qmi_cb_thread_ready;
extern int qmi_start;
extern int qmi_thrd_complete;
extern qmi_client_error_cb qmi_error_cb[QCRIL_QMI_CLIENT_MAX];
extern qmi_client_notify_cb qmi_notify_cb[QCRIL_QMI_CLIENT_MAX];
#ifdef RIL_UTF_IOE
extern struct ril_utf_csi_data_t IoE_servers[MAX_IOE_SERVERS];
extern struct ril_utf_req_handle_table_t req_table[MAX_IOE_REQUESTS];
#endif

static int error_cb_count = 0;
static int notify_cb_count = 0;

// These tools are used to block qmi_client_init for SSR testing
pthread_mutex_t block_client_init_mutex = PTHREAD_MUTEX_INITIALIZER;
enum ril_utf_qmi_srv_id_t client_service_id;
int block_client_init;

static pthread_mutex_t ril_utf_qmi_client_list_mutex = PTHREAD_MUTEX_INITIALIZER;

#define QMI_NOTIFY_HANDLE (qmi_client_type)(uintptr_t)(-1)

/*========================================================
  START. UTL qmi functionality support

  FUNCTIONS:
    - remove_from_pending_qmi_resp_q
    - add_to_pending_qmi_resp_q
    - issue_qmi_response
    - issue_qmi_indication
    - process_qmi_callback
    - print_qmi_params
    - qmi_thrd_start
    - ril_utf_get_qmi_client_id
    - qmi_client_send_msg_async
    - qmi_client_send_msg_sync
    - qmi_client_get_service_version
    - qmi_client_init
    - qmi_init
    - qmi_release
    - qmi_client_message_decode
    -
========================================================*/
#ifdef RIL_UTF_IOE
/*Utility function to decode the buffer recieved from client and call respective IoE Functions*/
qmi_csi_error decode_recieved_buffer_ioe(uint8_t *recv_buffer, int buffer_len, int service_handle);
/*Utility funtion to get service_handle pointer to the table from the client_handle*/
int get_service_from_client_handle(int client_handle);
#endif

/*========================================================
  FUNCTION: clear_pending_qmi_resp_q
  - remove all the elements from the response queue

  Critical section access:
  - The entire function accesses UTL critical section (for
    execution queue).
  - It is expected that the exec_q_access mutex is
    locked prior to call to this function.
========================================================*/
void clear_pending_qmi_resp_q(void)
{
  struct ril_utf_qmi_resp_pending_q_t *node;
  while (qmi_resp_pending_list_start)
  {
    node = qmi_resp_pending_list_start;
    RIL_UTF_DEBUG("Removing stale qmi request %08x. cb_data: %08x\n",
            node,
            node->resp_cb_data);
    remove_from_pending_qmi_resp_q( qmi_resp_pending_list_start );
    free(node);
  }
}

/*========================================================
  FUNCTION: clear_qmi_clients
  - reset the is_used bit for qmi_clients array

  Keep the other fields for now
========================================================*/
void clear_qmi_clients(void)
{
   for (int cid = 0; cid < QCRIL_UNITTEST_MAX_QMI_CLIENT_ID; cid++)
       for (int sid = 0; sid < QCRIL_UNITTEST_MAX_QMI_CLIENTS; sid++)
          ril_utf_qmi_client_list[cid][sid].is_used = 0;
}

/*========================================================
  FUNCTION: remove_from_pending_qmi_resp_q
  - remove the provided element from the pending qmi resp
    queue.
  - memory is released by the caller

  Critical section access:
  - The entire function accesses UTL critical section (for
    execution queue).
  - It is expected that the exec_q_access mutex is
    locked prior to call to this function.
========================================================*/
void remove_from_pending_qmi_resp_q( struct ril_utf_qmi_resp_pending_q_t *node)
{
  if ( node == qmi_resp_pending_list_start )
  {
    qmi_resp_pending_list_start = node->next;
  }
  else
  {
    node->prev->next = node->next;
  }

  if ( node->next )
  {
    node->next->prev = node->prev;
  }
  else
  {
    qmi_resp_pending_list_end = node->prev;
  }

  node->prev = NULL;
  node->next = NULL;
}

/*========================================================
  FUNCTION: add_to_pending_qmi_resp_q
  - create a node in the pending qmi resp queue based on
    the supplied parameters.

  Critical section access:
  - The entire function accesses UTL critical section (for
    execution queue).
  - It is expected that the exec_q_access mutex is
    locked prior to call to this function.
========================================================*/
void add_to_pending_qmi_resp_q
(
    qmi_client_type                 user_handle,
    unsigned long                   msg_id,
    void                            *resp_c_struct,
    int                             resp_c_struct_len,
    qmi_client_recv_msg_async_cb    resp_cb,
    void                            *resp_cb_data,
    enum qmi_send_msg_t_e            qmi_send_msg_type
)
{

  // create pending response entry node
  struct ril_utf_qmi_resp_pending_q_t *node = NULL;

  node = (struct ril_utf_qmi_resp_pending_q_t*) malloc(sizeof(struct ril_utf_qmi_resp_pending_q_t));
  node->user_handle = user_handle;
  node->msg_id      = msg_id;
  node->resp_c_struct = resp_c_struct;
  node->resp_c_struct_len = resp_c_struct_len;
  node->resp_cb = resp_cb;
  node->resp_cb_data = resp_cb_data;
  node->qmi_send_msg_type = qmi_send_msg_type;
  node->next = NULL;
  node->prev = NULL;


  // add node to end of pending resp list
  if ( qmi_resp_pending_list_end )
  {
    node->prev = qmi_resp_pending_list_end;
    qmi_resp_pending_list_end->next = node;
  }
  else
  {
    // nothing in the pending resp list. update start of list
    qmi_resp_pending_list_start = node;
  }

  //update end of list
  qmi_resp_pending_list_end = node;
}

/*========================================================
  FUNCTION: issue_qmi_response
  - processes the qmi response queue, and issues responses
  - called from the qmi_cb thread

  Critical section access:
    - This function locks and unlocks exec_q_mutex
========================================================*/
void issue_qmi_response(struct ril_utf_msg_t *tmp_node)
{
  RIL_UTF_DEBUG("\n QMI_RIL_UTF: Entering issue_qmi_response");

  //get service id, msg id and client_id
  int service_id = tmp_node->service_id;
  int msg_id     = tmp_node->msg_id;
  int client_id  = tmp_node->client_id;

  // use above to search for first pending resp match
  struct ril_utf_qmi_resp_pending_q_t *current_item = qmi_resp_pending_list_start;
  int    found_matching_req = 0;

  while ( current_item && !found_matching_req )
  {
    if ( current_item->user_handle->p_service->service_id == service_id
        && current_item->user_handle->service_user_handle == client_id
        && current_item->msg_id == msg_id )
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Found matching request for service %d, msg %d ", service_id, msg_id );
      found_matching_req = TRUE;

      // remove item from qmi_resp_pending_list
      remove_from_pending_qmi_resp_q( current_item );

      //update response
      if ( current_item->resp_c_struct_len == tmp_node->payload_len )
      {
        memcpy(current_item->resp_c_struct, tmp_node->payload, current_item->resp_c_struct_len);
      }
      else
      {
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error. mismatch between supplied payload len (%d) and buffer (%d)", tmp_node->payload_len, current_item->resp_c_struct_len);
      }

      break; // located match
    }
    else
    {
      current_item = current_item->next;
    }
  }


  if( !found_matching_req )
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error. did not find corresponding req mesg: srv id: %d, msg_id:%d", service_id, msg_id);
  }
  else
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: unlocking exec_q_access in %s", __func__);
    EXEC_Q_UNLOCK();

    //issue resp callback
    current_item->resp_cb(
                         current_item->user_handle,
                         current_item->msg_id,
                         current_item->resp_c_struct,
                         current_item->resp_c_struct_len,
                         current_item->resp_cb_data,
                         QMI_NO_ERR );
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: locking exec_q_access in %s", __func__);
    EXEC_Q_LOCK();

    //release memory
    free( current_item );
  }

  RIL_UTF_DEBUG("\n QMI_RIL_UTF: Exiting issue_qmi_response");

}

/*========================================================
  FUNCTION: issue_qmi_indication
  - issues a qmi indication based on the info in the
    global qmi_resp_q_ptr (which would have been updated
    prior to this function call).
  - called from the qmi_cb thread

  Critical section access:
  - The entire function accesses UTL critical section (for
    execution queue).
  - It is expected that the exec_q_access mutex is
    locked prior to call to this function.
========================================================*/
void issue_qmi_indication(struct ril_utf_msg_t *tmp_node)
{
  //get service id and msg id
  int client_id  = tmp_node->client_id;
  int service_id = tmp_node->service_id;
  int ind_msg_id = tmp_node->msg_id;
  void *dest;
  int tlv_stream_len, dst_encode_len;
  int32_t idl_err;

  RIL_UTF_DEBUG("\n QMI_RIL_UTF: Entering issue_qmi_indication");
  // Release exec_q_mutex for safe use of call back functions
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: unlocking exec_q_access in %s", __func__);
  EXEC_Q_UNLOCK();


  idl_err = qmi_idl_get_max_message_len( ril_utf_qmi_client_list[client_id][service_id].user_handle->p_service,
                                         QMI_IDL_INDICATION,
                                         ind_msg_id,
                                         &tlv_stream_len  );

  if (idl_err != 0)
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: ERROR qmi_idl_get_message_len returned error code %d\n", idl_err);
  }

  dest = malloc(tlv_stream_len);


  idl_err = qmi_idl_message_encode( ril_utf_qmi_client_list[client_id][service_id].user_handle->p_service,
                                    QMI_IDL_INDICATION,
                                    ind_msg_id,
                                    tmp_node->payload,
                                    tmp_node->payload_len,
                                    dest,
                                    tlv_stream_len,
                                    (uint32_t*)&dst_encode_len );

  if (idl_err != 0)
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: ERROR qmi_idl_message_encode returned error code %d\n", idl_err);
  }

  // Free C struct payload and add in encoded payload
//  free(tmp_node->payload);
//  tmp_node->payload = dest;
//  tmp_node->payload_len = dst_encode_len;

  //obtain indication register, previously registered
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: sending indication: service_id: %d, msg_id: %d", service_id, ind_msg_id);
  ril_utf_qmi_client_list[client_id][service_id].ind_cb( ril_utf_qmi_client_list[client_id][service_id].user_handle,
                                                         tmp_node->msg_id,
                                                         dest,
                                                         dst_encode_len,
                                                         ril_utf_qmi_client_list[client_id][service_id].ind_cb_data);

  RIL_UTF_DEBUG("\n QMI_RIL_UTF: Exiting issue_qmi_indication");
  // Acquire Q lock as that was the state at entry of function
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: locking exec_q_access in %s", __func__);
  EXEC_Q_LOCK();

  if (dest) {
    free(dest);
  }
}

/*========================================================
  FUNCTION: process_qmi_callback
  - check the current qmi_resp_q_ptr and issue either
    a qmi response or an indication, as appropriate.
  - called from the qmi_cb thread

  Critical section access:
  - The entire function accesses UTL critical section (for
    execution queue).
  - It is expected that the exec_q_access mutex is
    locked prior to call to this function.
========================================================*/
void process_qmi_callback()
{
  struct ril_utf_msg_t *tmp_node;

  RIL_UTF_DEBUG("\nEntering process_qmi_callback()\n");

  if ( ril_utf_inj_head != NULL )
  {
    for ( tmp_node = ril_utf_inj_head; tmp_node != NULL; tmp_node = tmp_node->next )
    {
      if ( (tmp_node->type == ril_utf_qmi_response) ||
           (tmp_node->type == ril_utf_qmi_indication))
      {
        remove_ril_utf_q_node(tmp_node, &ril_utf_inj_head, &ril_utf_inj_tail);
        break;
      }
    }

    // Inform test thread we are finished
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: Setting inject_complete to TRUE in %s", __func__);
    inject_complete = 1;
    pthread_cond_signal(&inject_complete_sig);

    if ( tmp_node != NULL )
    {
      if( tmp_node->type == ril_utf_qmi_indication )
      {
        // if indication, invoke appropriate handler
        issue_qmi_indication(tmp_node);
      }
      else if( tmp_node->type == ril_utf_qmi_response )
      {
        // if response, then issue callback
        issue_qmi_response(tmp_node);
      }
      else
      {
        RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error. expected either a response or an indication here...");
      }
    }
    else
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Warning! expected a message (qmi_resp or ind) here.");
    }
  }
  else
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Warning! expected a message (qmi_resp or ind) here.");
  }

  // tmp_node 'consumed'
  // the qmi response has been "consumed", so
  // release memory here.
  if (tmp_node) {
    if (tmp_node->payload) {
      free(tmp_node->payload);
    }
    free(tmp_node);
  }
  tmp_node = NULL;

  RIL_UTF_DEBUG("\nExiting process_qmi_callback()\n");
}

/*========================================================
  FUNCTION: print_qmi_params
  - prints the params a qmi send (sync/ async) function is
    called with.

  Critical section access:
  - None
========================================================*/
void print_qmi_params
(
    qmi_client_type    user_handle,
    int                msg_id,
    void               *req_c_struct,
    int                req_c_struct_len,
    int                resp_c_struct_len,
    int                timeout_msecs
)
{
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: service_id: %02x, msg_id: %04x, client_id: %d, request_struct_len: %d, resp_struct_len: %d, timeout:%d",
                               user_handle->p_service->service_id,
                               msg_id,
                               user_handle->service_user_handle,
                               req_c_struct_len,
                               resp_c_struct_len,
                               timeout_msecs);

  // TODO: print the message

}


/*========================================================
  FUNCTION: qmi_thrd_start
  - qmi thread entry function
  - locks the execution queue
  - waits (blocking/indefinite) for cond wakeup_qmi_cb_thread
  - when woken up, processes all the items in the pending qmi
    response queue
  - The pending qmi response queue will consist of either
    async qmi responses or qmi indications.

  Critical section access:
  - The entire function accesses UTL critical section (for
    execution queue).
  - It is expected that the exec_q_access mutex is
    locked prior to call to this function.
========================================================*/
void *qmi_thrd_start( void *param )
{
  struct timespec timeout;
  struct timeval e_timeout;
  e_timeout.tv_sec = 0;
  e_timeout.tv_usec = 10;

  // initalize predicate
  qmi_cb_thread_ready = 0;

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: setting qmi_thrd_complete to 1");
  qmi_thrd_complete = 1;
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: broadcasting qmi_thrd_complete_sig");
  pthread_cond_broadcast(&qmi_thrd_complete_sig);

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Starting QMI thread...");

  // Acquire Q lock
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: locking exec_q_access in %s", __func__);
  EXEC_Q_LOCK();
  error_cb_count = 0;
  notify_cb_count = 0;

  while ( EXEC_QMI_THREAD )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Exec...qmi thrd");

    // Wait for items to be placed in Q
    while ( (ril_utf_inj_head == NULL) || (qmi_cb_thread_ready == 0) )
    {
      calc_timeout(&timeout, &e_timeout);
      pthread_cond_timedwait(&wakeup_qmi_cb_thread, &exec_q_access, &timeout);

      #ifndef RIL_UTF_IOE
      // Check for reboot request
      if (ril_utf_check_reboot_state() == 1)
      {
        ril_utf_qmi_thread_shutdown();
      }
      #endif
    }
    // initalize predicate
    qmi_cb_thread_ready = 0;
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: qmi_cb thread waking up!");

    // read Q's and act
    process_qmi_callback();

  }

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: unlocking exec_q_access in %s", __func__);
  EXEC_Q_UNLOCK();
}

/*========================================================
  FUNCTION: ril_utf_get_qmi_client_id
  - checks the global client list (an array) and returns
    the index of the first unused location.

  Note:
   - The array size is 2 for now. only QMI_NAS initializes
     a second client in one QMI RIL instance.
   - TODO: if expanding UTL to include multiple RILs, then
     consider a linked list for the clients.

  Critical section access:
  - None
========================================================*/
int ril_utf_get_qmi_client_id(qmi_idl_service_object_type service_obj)
{
  int qmi_svc_idx = (int) service_obj->service_id;
  int ret_val = -1;
  int i;

  pthread_mutex_lock(&ril_utf_qmi_client_list_mutex);
  for ( i=0; i<QCRIL_UNITTEST_MAX_QMI_CLIENT_ID; i++ )
  {
    if( !ril_utf_qmi_client_list[i][qmi_svc_idx].is_used )
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Found new client id %d for service %d", i, qmi_svc_idx);
      ril_utf_qmi_client_list[i][qmi_svc_idx].is_used = 1;
      ret_val = i;
      break;
    }
  }
  pthread_mutex_unlock(&ril_utf_qmi_client_list_mutex);

  if ( ret_val == -1 )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Warning! exceeded QCRIL_UNITTEST_MAX_QMI_CLIENT_ID for qmi svc %d", qmi_svc_idx);
  }

  return ret_val;
}

void ril_utf_release_qmi_client_id(qmi_client_type user_handle)
{
    if (!user_handle || !user_handle->p_service) return;
    int qmi_svc_idx = (int)user_handle->p_service->service_id;
    int client_id = user_handle->service_user_handle;

    assert(client_id < QCRIL_UNITTEST_MAX_QMI_CLIENT_ID);
    assert(qmi_svc_idx < QCRIL_UNITTEST_MAX_QMI_CLIENTS);
    pthread_mutex_lock(&ril_utf_qmi_client_list_mutex);
    ril_utf_qmi_client_list[client_id][qmi_svc_idx].is_used = 0;
    pthread_mutex_unlock(&ril_utf_qmi_client_list_mutex);
}


/*========================================================
 * UTF redefinitions
 *   The below functions are all declared in qmi services
 *   include headers. They are redefined below for local
 *   stubbed implementation.
========================================================*/
qmi_client_error_type
ril_utf_qmi_client_send_msg_async
#ifdef RIL_UTF_IOE
(
    qmi_client_type                 user_handle,
    unsigned long                   msg_id,
    void                            *req_c_struct,
    int                             req_c_struct_len,
    void                            *resp_c_struct,
    int                             resp_c_struct_len,
    qmi_client_recv_msg_async_cb    resp_cb,
    void                            *resp_cb_data,
    qmi_txn_handle                  *txn_handle
)
#else
(
 qmi_client_type                 user_handle,
 unsigned int                    msg_id,
 void                            *req_c_struct,
 unsigned int                    req_c_struct_len,
 void                            *resp_c_struct,
 unsigned int                    resp_c_struct_len,
 qmi_client_recv_msg_async_cb    resp_cb,
 void                            *resp_cb_data,
 qmi_txn_handle                  *txn_handle
)
#endif
{
  int i, my_expect_slot;
  struct timespec timeout;
  struct timeval e_timeout;
  e_timeout.tv_sec = 0;
  e_timeout.tv_usec = EXPECT_TIMEOUT;

  if (!user_handle)
  {
      return QMI_SERVICE_ERR_INVALID_HANDLE;
  }

  RIL_UTF_ESSENTIAL("\n@@@@@@@@@@@@@@@@@@@@ UTF Testpoint invocation. QMI:recv'd Message ID: %d, Type: %s", msg_id, ril_utf_q_element_t_name[ril_utf_qmi_request].name);
  print_qmi_params(user_handle, msg_id, req_c_struct, req_c_struct_len, resp_c_struct_len, 0);
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: %s. waiting for exec_q_mutex.", __func__);
  // acquire mutex
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
    calc_timeout(&timeout, &e_timeout);
    pthread_cond_timedwait(&expect_ready_sig, &exec_q_access, &timeout);
  }

  enqueue_qmi_expect( req_c_struct, req_c_struct_len, msg_id,
    user_handle->p_service->service_id, user_handle->service_user_handle,
    ril_utf_qmi_request);

  // Add req to pending resp q.
  add_to_pending_qmi_resp_q ( user_handle,
                              msg_id,
                              resp_c_struct,
                              resp_c_struct_len,
                              resp_cb,
                              resp_cb_data,
                              async_req
                              );

  // signal test thread. Resp will be sent from qmi_cb thread.
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Setting expect_received to true @ %s %s %d ",__FILE__, __func__, __LINE__);
  expect_table[my_expect_slot].expect_received = 1;
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Signalling Expectation");
  pthread_cond_signal(&signal_exptectation);

  // release mutex
  EXEC_Q_UNLOCK();
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: %s. released exec_q_mutex.", __func__);

  // return success
  // TODO: add feature to return failure
  return QMI_NO_ERR;

}

qmi_client_error_type
ril_utf_qmi_client_send_msg_sync
#ifdef RIL_UTF_IOE
(
    qmi_client_type    user_handle,
    int                msg_id,
    void               *req_c_struct,
    int                req_c_struct_len,
    void               *resp_c_struct,
    int                resp_c_struct_len,
    int                timeout_msecs
)
#else
(
    qmi_client_type    user_handle,
    unsigned int       msg_id,
    void               *req_c_struct,
    unsigned int       req_c_struct_len,
    void               *resp_c_struct,
    unsigned int       resp_c_struct_len,
    unsigned int       timeout_msecs
)
#endif
{

  if (!user_handle)
  {
      return QMI_SERVICE_ERR_INVALID_HANDLE;
  }

  RIL_UTF_ESSENTIAL("\n@@@@@@@@@@@@@@@@@@@@ UTF Testpoint invocation. QMI:Message ID: %d, Type: %s", msg_id, ril_utf_q_element_t_name[ril_utf_qmi_request].name);
  print_qmi_params(user_handle, msg_id, req_c_struct, req_c_struct_len, resp_c_struct_len, timeout_msecs);
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: %s. waiting for exec_q_mutex.", __func__);
  int ret_val = QMI_NO_ERR;
  int i;
  int my_expect_slot;
  struct timeval sync_wait_time;
  int err_val = QCRIL_UNITTTEST_NO_ERR;
  struct ril_utf_msg_t *tmp_node;
  struct timeval e_timeout;
  struct timespec timeout;
  e_timeout.tv_sec = 0;
  e_timeout.tv_usec = EXPECT_TIMEOUT;

  // acquire mutex
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
    calc_timeout(&timeout, &e_timeout);
    pthread_cond_timedwait(&expect_ready_sig, &exec_q_access, &timeout);
  }

  // add msg to queue for test thread
  enqueue_qmi_expect( req_c_struct, req_c_struct_len, msg_id,
    user_handle->p_service->service_id, user_handle->service_user_handle,
    ril_utf_qmi_request);

  // signal test thread. Resp will be sent from qmi_cb thread.
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Setting expect_received to true @ %s %s %d ",__FILE__, __func__, __LINE__);
  expect_table[my_expect_slot].in_use = 1;
  expect_table[my_expect_slot].expect_received = 1;
  expect_table[my_expect_slot].is_sync = 1;
  expect_table[my_expect_slot].service_id = user_handle->p_service->service_id;
  expect_table[my_expect_slot].client_id = user_handle->service_user_handle;
  expect_table[my_expect_slot].msg_id = msg_id;
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: signalling signal_exptectation@ %s %s %d ",__FILE__, __func__, __LINE__);
  pthread_cond_signal(&signal_exptectation);

  if ( !suspend_timeouts )
  {
    sync_wait_time.tv_sec = 0;
    sync_wait_time.tv_usec = timeout_msecs * 1000; // milli to micro sec
  }
  else
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: suspending timeouts",__FILE__, __func__, __LINE__);
    sync_wait_time.tv_sec = global_timeout_max; //seconds
    sync_wait_time.tv_usec = 0; // milli to micro sec
  }
  calc_timeout(&timeout, &sync_wait_time);

  for ( i = 0; i < MAX_SIGNALS; ++i )
  {
    if ( wakeup_qmi_sync_signals[i].in_use == 0 )
    {
      break;
    }
  }
  // Set signal from pool to wait
  wakeup_qmi_sync_signals[i].in_use = 1;
  wakeup_qmi_sync_signals[i].msg_id = msg_id;
  wakeup_qmi_sync_signals[i].service_id = user_handle->p_service->service_id;
  wakeup_qmi_sync_signals[i].client_id = user_handle->service_user_handle;
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: waiting for qmi sync req response @ %s signal slot %d", __func__, i);
  err_val = pthread_cond_timedwait(&(wakeup_qmi_sync_signals[i].signal_exptectation), &exec_q_access, &timeout);

  // Clear signal for use by someone else
  wakeup_qmi_sync_signals[i].in_use = 0;

  // find message sent to us
  for ( tmp_node = ril_utf_inj_head; tmp_node  != NULL; tmp_node = tmp_node->next )
  {
    if ( (tmp_node->msg_id == msg_id) &&
         (tmp_node->service_id == user_handle->p_service->service_id) )
    {
      // Found corresponding expectation
      // Remove from list
      remove_ril_utf_q_node(tmp_node, &ril_utf_inj_head, &ril_utf_inj_tail);
      break;
    }
  } // end for


  if ( err_val == ETIMEDOUT )
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: TEST REPORT: Error. timed out @ %s....", __func__);
    ret_val = QMI_TIMEOUT_ERR;
  }
  else
  {
    if( tmp_node )
    {
      memcpy(resp_c_struct, tmp_node->payload, resp_c_struct_len);
    }
    else
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: TEST REPORT: Error(%d). no response msg found", ret_val);
    }
  }

  // Inform test thread we are finished
  RIL_UTF_DEBUG("\n QMI_RIL_UTF: Setting inject_complete to TRUE in %s", __func__);
  inject_complete = 1;
  pthread_cond_signal(&inject_complete_sig);

  // qmi_resp_q_ptr 'consumed'
  // the qmi response has been "consumed", so
  // release memory here.
  if (tmp_node) {
    if (tmp_node->payload) {
      free(tmp_node->payload);
      tmp_node->payload = NULL;
    }
    free(tmp_node);
    tmp_node = NULL;
  }

  // release mutex
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: %s. released exec_q_mutex.", __func__);
  EXEC_Q_UNLOCK();

  // return success if we reached here
  // TODO: add feature to return failure
  return ret_val;
}

/*========================================================
  FUNCTION: enqueue_qmi_expect
    - Add received qmi message to queue for test thread
  Critical section access:
  - exec_q_access is expected to be locked prior to calling
    this function.
========================================================*/
void enqueue_qmi_expect( void  *req_c_struct, int req_c_struct_len, unsigned long msg_id,
                     int service_id, int client_id, enum ril_utf_q_element_t type )
{
  struct ril_utf_msg_t *expect_node =
    (struct ril_utf_msg_t *) malloc(sizeof(struct ril_utf_msg_t));
  expect_node->payload = malloc(req_c_struct_len);
  memcpy(expect_node->payload, req_c_struct, req_c_struct_len);
  expect_node->payload_len = req_c_struct_len;
  expect_node->msg_id = msg_id;
  expect_node->service_id = service_id;
  expect_node->client_id = client_id;
  expect_node->type = type;
  // check for expectation
  enqueue_ril_utf_q_node(expect_node, &ril_utf_rec_head, &ril_utf_rec_tail);
}

void ril_utf_qmi_thread_shutdown()
{
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: UNLOCKING exec_q_mutex");
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Executing qmi_cb_thread shutdown");
  EXEC_Q_UNLOCK();
  qmi_start = FALSE;
  pthread_exit(NULL);
}

qmi_client_error_type
qmi_client_get_service_version
(
    const char                   *conn_id,
    qmi_idl_service_object_type   service_obj,
    qmi_service_version_info     *service_version_info
)
{
  return QMI_NO_ERROR;
}

#ifdef RIL_UTF_IOE
qmi_client_error_type ril_utf_qmi_client_init_instance
(
 qmi_idl_service_object_type service_obj,
 qmi_service_instance        instance_id,
 qmi_client_ind_cb           ind_cb,
 void                        *ind_cb_data,
 qmi_client_os_params        *os_params,
 uint32_t                    timeout,
 qmi_client_type             *user_handle
)
{
  ril_utf_qmi_client_init(NULL,
                  service_obj,
                  ind_cb,
                  ind_cb_data,
                  user_handle);
  //ignore timeout
}

qmi_client_error_type
ril_utf_qmi_client_init
(
  const char                                *conn_id,
  qmi_idl_service_object_type               service_obj,/* Defined in the  generated header file */
  qmi_client_ind_cb                         ind_cb,
  void                                      *ind_cb_data,
  qmi_client_type                           *user_handle
)
#else
qmi_client_error_type
ril_utf_qmi_client_init
(
 qmi_service_info                          *service_info,
 qmi_idl_service_object_type               service_obj,
 qmi_client_ind_cb                         ind_cb,
 void                                      *ind_cb_data,
 qmi_client_os_params                      *os_params,
 qmi_client_type                           *user_handle
)
#endif
{

  #ifndef RIL_UTF_IOE
    // ensure that on first call we start up qmi threads
  pthread_attr_t qmi_thrd_attr;
  int ret;
  int return_now = 0;
  struct timeval e_timeout;
  struct timeval timeout;
  e_timeout.tv_sec = 0;
  e_timeout.tv_usec = EXPECT_TIMEOUT;

  if (qmi_start != TRUE)
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: %s. waiting for exec_q_mutex.", __func__);
    EXEC_Q_LOCK();

    qmi_start = TRUE;
    //start qmi thread
    pthread_attr_init(&qmi_thrd_attr);
    ret = pthread_create(&qmit_id, &qmi_thrd_attr, qmi_thrd_start, NULL);

    if ( ret < 0 )
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Failed to start qmi thread. Exiting...");
      return -1;
    }
    else
    {
      while (qmi_thrd_complete != 1)
      {
        calc_timeout(&timeout, &e_timeout);
        pthread_cond_timedwait(&qmi_thrd_complete_sig, &exec_q_access, &timeout);
      }
    }

    EXEC_Q_UNLOCK();
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: %s. released exec_q_mutex.", __func__);
  }
  #endif
 //ignore conn_id for now
  qmi_client_error_type ret_val = QMI_INTERNAL_ERR;
  int qmi_svc_idx = (int) service_obj->service_id;
  int qmi_client_idx = ril_utf_get_qmi_client_id(service_obj);

  // Block client init until we are unblocked by tester
  while ((check_block_client_init() == 1) && (qmi_svc_idx == client_service_id))
  {
    sleep(1);
    return_now = 1;
    ret = QMI_TIMEOUT_ERROR;
  }
  if (return_now == 1)
  {
    return ret;
  }

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: svc %d, client %d", qmi_svc_idx,  qmi_client_idx);

  if ( qmi_client_idx != -1)
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: %s for svc %d, client %d", __func__, qmi_svc_idx,  qmi_client_idx);

    ril_utf_qmi_client_list[qmi_client_idx][qmi_svc_idx].service_obj = service_obj;
    ril_utf_qmi_client_list[qmi_client_idx][qmi_svc_idx].ind_cb = ind_cb;
    ril_utf_qmi_client_list[qmi_client_idx][qmi_svc_idx].ind_cb_data = ind_cb_data;

    ril_utf_qmi_client_list[qmi_client_idx][qmi_svc_idx].user_handle = (qmi_client_type) malloc(sizeof(struct qmi_client_struct));
    ril_utf_qmi_client_list[qmi_client_idx][qmi_svc_idx].user_handle->service_user_handle = qmi_client_idx;
    ril_utf_qmi_client_list[qmi_client_idx][qmi_svc_idx].user_handle->p_service = service_obj;

    *user_handle = ril_utf_qmi_client_list[qmi_client_idx][qmi_svc_idx].user_handle;

    ret_val = QMI_NO_ERROR;
  }

  return ret_val;
}

#ifndef RIL_UTF_IOE
/*===========================================================================
  FUNCTION  ril_utf_qmi_client_init_instance
===========================================================================*/
qmi_client_error_type
ril_utf_qmi_client_init_instance
(
 qmi_idl_service_object_type service_obj,
 qmi_service_instance        instance_id,
 qmi_client_ind_cb           ind_cb,
 void                        *ind_cb_data,
 qmi_client_os_params        *os_params,
 uint32_t                    timeout,
 qmi_client_type             *user_handle
)
{
  int rc;
  qmi_service_info info;

  rc = ril_utf_qmi_client_init(&info, service_obj, ind_cb, ind_cb_data, os_params, user_handle);

  return rc;
}
#endif
int qmi_init
(
  qmi_sys_event_rx_hdlr   event_rx_hdlr,
  void                    *event_user_data
)
{
  pthread_attr_t qmi_thrd_attr;
  int ret;

  qmi_client_info.sys_event_rx_hdlr = event_rx_hdlr;
  qmi_client_info.sys_event_user_data = event_user_data;

  //memset(ril_utf_qmi_client_list, 0x00, sizeof(struct ril_utf_q_t) * QCRIL_UNITTEST_MAX_QMI_CLIENT_ID * QCRIL_UNITTEST_MAX_QMI_CLIENTS);

  //start qmi thread
  pthread_attr_init(&qmi_thrd_attr);
  ret = pthread_create(&qmit_id, &qmi_thrd_attr, qmi_thrd_start, NULL);

  if ( ret < 0 )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Failed to start qmi thread. Exiting...");
    return -1; //TODO: update error reporting
  }

  return qmi_client_info.init_client_handle;
}

int qmi_release
(
  int init_handle
)
{
  return 0;
}

qmi_client_error_type
ril_utf_qmi_client_release
(
  qmi_client_type     user_handle
)
{
  if (user_handle != QMI_NOTIFY_HANDLE)
      ril_utf_release_qmi_client_id(user_handle);
  return QMI_NO_ERROR;
}

qmi_client_error_type
ril_utf_qmi_client_get_service_list
(
 qmi_idl_service_object_type   service_obj,
 qmi_service_info       *service_info_array,
 unsigned int *num_entries,
 unsigned int *num_services
 )
{
  return QMI_SERVICE_ERR;
}

qmi_client_error_type
ril_utf_qmi_client_message_decode
#ifdef RIL_UTF_IOE
(
    qmi_client_type                         user_handle,
    qmi_idl_type_of_message_type            req_resp_ind,
    int                                     message_id,
    const void                              *p_src,
    int                                     src_len,
    void                                    *p_dst,
    int                                     dst_len
)
#else
(
 qmi_client_type                         user_handle,
 qmi_idl_type_of_message_type            req_resp_ind,
 unsigned int                            message_id,
 const void                              *p_src,
 unsigned int                            src_len,
 void                                    *p_dst,
 unsigned int                            dst_len
)
#endif
{
    qmi_client_error_type rc;
    rc = qmi_idl_message_decode(user_handle->p_service,
                                req_resp_ind,
                                message_id,
                                p_src,
                                src_len,
                                p_dst,
                                dst_len);
    return rc;
}

#ifdef RIL_UTF_IOE
qmi_csi_error
qmi_csi_register
(
   qmi_idl_service_object_type               service_obj,
   qmi_csi_connect                           service_connect,
   qmi_csi_disconnect                        service_disconnect,
   qmi_csi_process_req                       service_process_req,
   void                                      *service_cookie,
   qmi_csi_os_params                         *os_params,
   qmi_csi_service_handle                    *service_provider
)
{
    struct sockaddr_un servaddr;
    int sock_fd;
    int i=0;

    RIL_UTF_DEBUG("\nEnter: %s", __func__);
    RIL_UTF_DEBUG("\nFinding first open server slot");

    // cycle through servers list to id an open slot
    while (IoE_servers[i].is_used && i < MAX_IOE_SERVERS) {
        i++;
    }
    if (i == MAX_IOE_SERVERS)
    {
        // no open slot;
        // solution will be to increase hardcoded MAX_IOE_SERVERS value
        RIL_UTF_DEBUG("\nNo open slots for new servers. Reached limit %d", MAX_IOE_SERVERS);
        return QMI_CSI_INTERNAL_ERR;
    }
    else
    {
        RIL_UTF_DEBUG("\nAdding new server at slot %d", i);
    }

    /* creating socket */
    if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        RIL_UTF_ESSENTIAL("\nsocket creation failed");
        return QMI_CSI_INTERNAL_ERR;
    }

    memset(&servaddr, 0x00, sizeof(servaddr));
    servaddr.sun_family      = AF_UNIX;
    strlcpy(servaddr.sun_path, QMI_CSI_MCM_CORE_SRV, sizeof(servaddr.sun_path)-1);
    unlink(servaddr.sun_path);

    RIL_UTF_ESSENTIAL("\nbind ...%s", servaddr.sun_path);

    /* binding our socket to the service port */
    if (bind(sock_fd, (struct sockaddr*) &servaddr, sizeof(servaddr)) < 0)
    {
       RIL_UTF_ESSENTIAL("\nCould not bind socket to socket path");
       return QMI_CSI_INTERNAL_ERR;
    }

    /* convert our socket to a listening socket */
    if (listen(sock_fd, 5) < 0)
    {
        RIL_UTF_ESSENTIAL("\nCould not make socket passive");
        return 1;
    }

    RIL_UTF_ESSENTIAL("\nIoE server listening on socket %d",QMI_CSI_MCM_CORE_SRV );

    IoE_servers[i].service_obj     = service_obj;
    IoE_servers[i].service_connect = service_connect;
    IoE_servers[i].service_disconnect = service_disconnect;
    IoE_servers[i].service_process_req = service_process_req;
    IoE_servers[i].service_cookie = service_cookie;
    IoE_servers[i].client_array_size = 0;
    FD_ZERO(&((*os_params).fds));
    FD_SET(sock_fd, &((*os_params).fds));
    (*os_params).max_fd = sock_fd;
    IoE_servers[i].os_params = os_params;
    IoE_servers[i].client_sd = -1;
    // sp is an opaque handle and we only need the index
    // into IoE_servers to know which cb functions to
    // associate a particular csi call with
    RIL_UTF_DEBUG("\nService provide assigned to %d", i);
    *service_provider=i;

    //mark slot as taken up
    IoE_servers[i].is_used = 1;

    RIL_UTF_DEBUG("\nExit: %s", __func__);
}


qmi_csi_error
qmi_csi_handle_event
(
   qmi_csi_service_handle                    service_provider,
   qmi_csi_os_params                         *os_params
)
{
  int service_handle = service_provider;
  qmi_csi_error err_val = QMI_CSI_NO_ERR;
  if(service_handle >= MAX_IOE_SERVERS)
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Incorrect Service provider!!!");
    err_val = QMI_CSI_INTERNAL_ERR;
  }
  else if(!IoE_servers[service_handle].is_used)
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Servicenot been registered yet!!!");
    err_val = QMI_CSI_INTERNAL_ERR;
  }
  else
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Found the service in service table");
    uint8_t recv_buffer[RIL_UTF_IOE_CLIENT_SOCKET_MAX_BUF_SIZE];
    int buffer_len = 0;
    //Recieve the msg from socket
    int j = 0;
    int new_sd;
    if(IoE_servers[service_handle].client_sd == -1)
    {
        /*Get the address of FD set into the client_sd field of the table*/
//      EXEC_Q_LOCK();
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Client trying to connect to the server");
        new_sd = accept((*os_params).max_fd, NULL, NULL);
        if(new_sd < 0)
        {
            RIL_UTF_DEBUG("\n QMI_RIL_UTL: Client Connect Failed!");
        }
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: New Client Connection established! %d", new_sd);
       // FD_SET(new_sd, (*os_params).fds);
        if(new_sd > (*os_params).max_fd)
            (*os_params).max_fd = new_sd;
        IoE_servers[service_handle].client_sd = new_sd;
    }
    do
    {
        RIL_UTF_DEBUG("\nQMI_RIL_UTL: recieve data from the socket %d",IoE_servers[service_handle].client_sd );
        buffer_len = recv(IoE_servers[service_handle].client_sd, recv_buffer, sizeof(recv_buffer), 0);
        if(buffer_len <= 0)
        {
            RIL_UTF_DEBUG("\n QMI_RIL_UTL: Couldnot recieve data from socket at the server end %d", buffer_len);
            err_val = QMI_CSI_INTERNAL_ERR;
            break;
        }
        RIL_UTF_DEBUG("\nQMI_RIL_UTL: recieved data, Processing the commands from client");
        err_val = decode_recieved_buffer_ioe(recv_buffer, buffer_len, service_handle);
    }while(buffer_len > 0);
//    EXEC_Q_UNLOCK();

  }
  return err_val;
}

qmi_csi_error decode_recieved_buffer_ioe(uint8_t *recv_buffer, int buffer_len, int service_handle)
{
    qmi_csi_error err_val = QMI_CSI_NO_ERR;
    RIL_UTF_DEBUG("\n QMI_UTF_RIL: Processing the recieved data from client");
    int i = 0;
    int j = 0;
    qmi_client_handle client_handle;
    void *connection_handle;
    int req_handle = 0;
    uint32_t msg_id, temp;
    uint32_t req_c_struct_len;
    void *req_c_struct;
    do
    {
        switch(recv_buffer[i++])
        {
            case UTF_IOE_CLIENT_CONNECT_CB_ID:
                RIL_UTF_DEBUG("\n QMI_RIL_UTL: Calling connect_cb from handle event!!! client_handle = %d", recv_buffer[i]);
                //Store the client hancle for the setrver in the client array
                if(IoE_servers[service_handle].client_array_size == 0)
                {
                    //First client
                    IoE_servers[service_handle].client_array = (int *)malloc(UTF_IOE_MAX_CLIENTS * sizeof(int));
                }
                if((IoE_servers[service_handle].client_array_size+1) > UTF_IOE_MAX_CLIENTS)
                {
                    RIL_UTF_DEBUG("\n QMI_UTF_RIL: Max client limit has reached!");
                }
                else
                {
                    client_handle = recv_buffer[i++];
                    IoE_servers[service_handle].client_array[(IoE_servers[service_handle].client_array_size++)] = client_handle;
                    ((*IoE_servers[service_handle].service_connect)(client_handle,
                                                                    &IoE_servers[service_handle].service_cookie,
                                                                    &connection_handle));
                    IoE_servers[service_handle].connection_handle = connection_handle;
                }
                break;
            case UTF_IOE_CLIENT_DISCONNECT_CB_ID:
                RIL_UTF_DEBUG("QMI_RIL_UTL: Disconnect CB in qmi-csi-handle event");
                break;
            case UTF_IOE_CLIENT_REQ_CB_ID:
                RIL_UTF_DEBUG("\nQMI_RIL_UTL: Calling Req_cb with recieved data: %d",recv_buffer);
                /*Code to enter the req into the req_table*/
                while(req_table[req_handle].is_used && req_handle < MAX_IOE_REQUESTS)
                {
                    req_handle++;
                }
                if(req_handle >= MAX_IOE_REQUESTS)
                {
                    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Request table is full!");
                    //send_error reponse or inc the req table capacity?
                }
                else
                {
                    client_handle = recv_buffer[i++];
                    req_table[req_handle].is_used = 1;
                    req_table[req_handle].service_handle = service_handle;
                    req_table[req_handle].client_handle = client_handle;
                    msg_id = 0x000;
                    req_c_struct_len = 0x000;
                    for(j = 0; j < (sizeof(uint32_t)/sizeof(uint8_t)) ; j++)
                    {
                        temp = (uint32_t)recv_buffer[i++];
                        temp = temp << (j * 8);
                        msg_id = msg_id | temp;
                    }
                    RIL_UTF_DEBUG("\nQMI_RIL_UTL: decoded msg_id %d", msg_id);
                    for(j = 0; j < (sizeof(uint32_t)/sizeof(uint8_t)); j++)
                    {
                        temp = (uint32_t)recv_buffer[i++];
                        temp = temp << (j * 8);
                        req_c_struct_len = req_c_struct_len | temp;
                    }
                    RIL_UTF_DEBUG("\nQMI_RIL_UTL: decoded paylen %d", req_c_struct_len);
                    if(req_c_struct_len == 0)
                        req_c_struct = NULL;
                    else
                    {
                        req_c_struct = malloc(req_c_struct_len);
                        memcpy(req_c_struct, recv_buffer + i, req_c_struct_len);
                    }
                    /*Debugging purpose can uncomment if needed
                    mcm_sms_send_mo_msg_req_msg_v01 *ptr =
                        (mcm_sms_send_mo_msg_req_msg_v01*) req_c_struct;
                    RIL_UTF_DEBUG("\nQMI_RIL_UTL: Req cb struct details \n (UTF)Format: %d\n(UTF)messgae_content: %s\n (UTF)destination: %s\n(UTF)sv_valid:%d", ptr->message_format,ptr->message_content, ptr->destination,ptr->size_validation_valid);*/
                    ((*IoE_servers[service_handle].service_process_req)(IoE_servers[service_handle].connection_handle,
                                                                        req_handle,
                                                                        msg_id,
                                                                        req_c_struct,
                                                                        req_c_struct_len,
                                                                        &IoE_servers[service_handle].service_cookie));
                    i+=req_c_struct_len;
                }
                break;
            default:
                RIL_UTF_DEBUG("\n QMI_RIL_UTL: Qmi csi handle event..event unknown");
        }
    }while(i < buffer_len);
}

qmi_csi_error
qmi_csi_send_resp
(
    qmi_req_handle     req_handle,
 unsigned int       msg_id,
    void               *resp_c_struct,
 unsigned int       resp_c_struct_len
)
{
    qmi_csi_error err_val = QMI_CSI_NO_ERR;
    int req_handle_int = (int) req_handle;
    if(req_handle_int >= MAX_IOE_REQUESTS)
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Invalid req_handle!!!");
      err_val = QMI_CSI_INTERNAL_ERR;
    }
    else if(!req_table[req_handle_int].is_used)
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Request has not been sent through to send the response!!");
      err_val = QMI_CSI_INTERNAL_ERR;
    }
    else
    {
        int service_handle = req_table[req_handle_int].service_handle;
        //Calculate the buffer len
        // resp_cb id(1) + req_handle(1) + masg_id(4) + payload_len(4)
        int buffer_len = UTF_IOE_RESP_IND_BUFFER_MIN_FIXED_LEN + resp_c_struct_len;
        uint8_t *send_message_buffer;
        send_message_buffer = (uint8_t*) malloc(buffer_len);

        //Form the buffer
        int i = 0;
        send_message_buffer[i++] = UTF_IOE_CLIENT_RESP_CB_ID;
        send_message_buffer[i++] =  req_table[req_handle_int].client_handle;
        send_message_buffer[i++] = IoE_servers[service_handle].service_obj->service_id;

        uint32_t temp = (uint32_t) msg_id;
        memcpy(send_message_buffer + i, &temp, sizeof(uint32_t));
        i+= sizeof(uint32_t);

        temp = (uint32_t)resp_c_struct_len;
        memcpy(send_message_buffer + i, &temp, sizeof(uint32_t));
        i+= sizeof(uint32_t);
        uint8_t *payld;
        payld = (uint8_t*) malloc(resp_c_struct_len);
        payld  = (uint8_t*)resp_c_struct;
        memcpy(send_message_buffer + i, payld, resp_c_struct_len);
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Sending Response from server!");
        if (send(IoE_servers[service_handle].client_sd, (void *) send_message_buffer, buffer_len, 0) < 0)
        {
            RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error! Failed to send response to  ioe_client socket.");
            err_val = QMI_CSI_INTERNAL_ERR;
        }
    }

}

qmi_csi_error
qmi_csi_send_ind
(
    qmi_client_handle  client_handle,
    unsigned int       msg_id,
    void               *ind_c_struct,
    unsigned int       ind_c_struct_len
)
{
    int service_handle = get_service_from_client_handle(client_handle);
    qmi_csi_error err_val = QMI_CSI_NO_ERR;
    if(service_handle == -1)
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Incorrect Client. Client never registered to a server!!! Serv_handle: %d , Client_hdle: %d", service_handle, client_handle);
      err_val = QMI_CSI_INTERNAL_ERR;
    }
    else
    {
        //Calculate the buffer len
        // resp_cb id(1) + client_handle(1) + masg_id(4) + payload_len(4)
        int buffer_len = UTF_IOE_RESP_IND_BUFFER_MIN_FIXED_LEN + ind_c_struct_len;
        uint8_t *send_message_buffer;
        send_message_buffer = (uint8_t*) malloc(buffer_len);

        //Form the buffer
        int i = 0;
        send_message_buffer[i++] = UTF_IOE_CLIENT_IND_CB_ID;
        send_message_buffer[i++] = client_handle;
        send_message_buffer[i++] = IoE_servers[service_handle].service_obj->service_id;

        uint32_t temp = (uint32_t) msg_id;
        memcpy(send_message_buffer + i, &temp, sizeof(uint32_t));
        i+= sizeof(uint32_t);

        temp = (uint32_t)ind_c_struct_len;
        memcpy(send_message_buffer + i, &temp, sizeof(uint32_t));
        i+= sizeof(uint32_t);
        uint8_t *payld;
        payld = (uint8_t*) malloc(ind_c_struct_len);
        payld  = (uint8_t*)ind_c_struct;
        memcpy(send_message_buffer + i, payld, ind_c_struct_len);
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Sending Response from server!");
        if (send(IoE_servers[service_handle].client_sd, (void *) send_message_buffer, buffer_len, 0) < 0)
        {
            RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error! Failed to send response to  ioe_client socket.");
            err_val = QMI_CSI_INTERNAL_ERR;
        }
    }

}

int get_service_from_client_handle(int client_handle)
{
    int i, j, service_handle;
    service_handle = -1;
    for(i = 0; i <  MAX_IOE_SERVERS; i++)
    {
        if(IoE_servers[i].is_used && (IoE_servers[i].client_array_size > 0) && (service_handle == -1))
        {
            for(j = 0;j < IoE_servers[i].client_array_size; j++)
            {
                if(IoE_servers[i].client_array[j] == client_handle)
                {
                    service_handle = i;
                    break;
                }
            }
        }
    }
    return service_handle;
}
#endif
/*===========================================================================
  FUNCTION  ril_utf_qmi_client_message_encode
===========================================================================*/
/*!
@brief
  Encodes a message

@return
  QMI_NO_ERROR if function is successful, error code otherwise.

@note

  - Dependencies
    - None

  - Side Effects
   - None
*/
/*=========================================================================*/
qmi_client_error_type
ril_utf_qmi_client_message_encode
#ifdef RIL_UTF_IOE
(
    qmi_client_type                   user_handle,
    qmi_idl_type_of_message_type      req_resp_ind,
    int                               message_id,
    const void                        *p_src,
    int                               src_len,
    void                              *p_dst,
    int                               dst_len,
    int                               *dst_encoded_len
)
#else
(
 qmi_client_type                      user_handle,
 qmi_idl_type_of_message_type         req_resp_ind,
 unsigned int                         message_id,
 const void                           *p_src,
 unsigned int                         src_len,
 void                                 *p_dst,
 unsigned int                         dst_len,
 unsigned int                         *dst_encoded_len
)
#endif
{
    qmi_client_error_type rc;
    rc = qmi_idl_message_encode(user_handle->p_service,
                                req_resp_ind,
                                message_id,
                                p_src,
                                src_len,
                                p_dst,
                                dst_len,
                                (uint32_t*)dst_encoded_len);
    return rc;
}

/*===========================================================================
  FUNCTION  ril_utf_qmi_client_register_error_cb
===========================================================================*/
qmi_client_error_type ril_utf_qmi_client_register_error_cb
(
 qmi_client_type user_handle,
 qmi_client_error_cb err_cb,
 void *err_cb_data
 )
{
  int client = (int)err_cb_data;
#if 0 // TODO: revist
  qmi_error_cb[client] = err_cb;
#endif
  if(client != QCRIL_QMI_CLIENT_IMS_SETTING &&
     client != QCRIL_QMI_CLIENT_IMSA)
  {
    ++error_cb_count;
  }
  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  ril_utf_qmi_client_register_notify_cb
===========================================================================*/
qmi_client_error_type ril_utf_qmi_client_register_notify_cb
(
 qmi_client_type user_handle,
 qmi_client_notify_cb notify_cb,
 void *notify_cb_data
)
{
/*
  int client = (int)notify_cb_data;
  if(client != QCRIL_QMI_CLIENT_IMS_SETTING &&
     client != QCRIL_QMI_CLIENT_IMSA)
  {
    ++notify_cb_count;
  }
  qmi_notify_cb[client] = notify_cb;
*/
  notify_cb(user_handle, NULL, QMI_CLIENT_SERVICE_COUNT_INC, notify_cb_data);
  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  ril_utf_qmi_client_notifier_init
===========================================================================*/
qmi_client_error_type
ril_utf_qmi_client_notifier_init
(
 qmi_idl_service_object_type               service_obj,
 qmi_client_os_params                      *os_params,
 qmi_client_type                           *user_handle
 )
{
  // Pass through as data is not used
  *user_handle = QMI_NOTIFY_HANDLE;
  return QMI_NO_ERR;
}

/*===========================================================================
  FUNCTION  ril_utf_qmi_client_get_service_instance
===========================================================================*/
qmi_client_error_type
ril_utf_qmi_client_get_service_instance
(
 qmi_idl_service_object_type service_obj,
 qmi_service_instance instance_id,
 qmi_service_info *service_info
 )
{

  return QMI_NO_ERROR;
}

int send_ril_utf_ssr_error_cb(struct ril_utf_msg_t *test_step)
{
  qcril_qmi_client_e_type client_num = *((qcril_qmi_client_e_type*)test_step->payload);

  // Callback with error for specified client
  qmi_error_cb[client_num](NULL, 0, (void*)client_num);

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

  return 0;
}

int send_ril_utf_ssr_notify_cb(struct ril_utf_msg_t *test_step)
{
  qcril_qmi_client_e_type client_num = *((qcril_qmi_client_e_type*)test_step->payload);

  // Callback with client that is back up
  qmi_notify_cb[client_num](NULL, 0, QMI_CLIENT_SERVICE_COUNT_INC, (void*)client_num);

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

  return 0;
}

void utf_wait_for_notify_reg_cb()
{
  // lock till notify cbs have been reg
  while ( notify_cb_count < (error_cb_count-2) ) sleep(1); // we don't care about IMSA and IMSS *shrugs*

  notify_cb_count = 0;
  error_cb_count = 0;
}


void block_qmi_client_init(enum ril_utf_qmi_srv_id_t service)
{
  pthread_mutex_lock(&block_client_init_mutex);

  client_service_id = service;
  block_client_init = 1;

  pthread_mutex_unlock(&block_client_init_mutex);
}

void unblock_qmi_client_init()
{
  pthread_mutex_lock(&block_client_init_mutex);

  block_client_init = 0;

  pthread_mutex_unlock(&block_client_init_mutex);
}

int check_block_client_init()
{
  pthread_mutex_lock(&block_client_init_mutex);

  int tmp = block_client_init;

  pthread_mutex_unlock(&block_client_init_mutex);
  return tmp;
}

/*===========================================================================
  FUNCTION  ril_utf_qmi_client_get_any_service
===========================================================================*/
qmi_client_error_type
ril_utf_qmi_client_get_any_service
(
 qmi_idl_service_object_type service_obj,
 qmi_service_info *service_info
 )
{

  return QMI_NO_ERROR;
}

/*========================================================
  END. UTF qmi support implementation.
========================================================*/

