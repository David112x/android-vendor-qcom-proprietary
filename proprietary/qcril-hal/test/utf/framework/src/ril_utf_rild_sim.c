/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_rild_sim.c
  @brief   RIL UTF RIL daemon simulation

  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#include "ril_utf_rild_sim.h"
#include "platform/common/platform.h"

/*========================================================
 * Global Variables
========================================================*/
extern pthread_t rild_id;
extern RIL_RadioFunctions qmi_ril_funcs;
extern struct timed_cb_t *timed_cb_list_head;
extern struct timed_cb_t *timed_cb_list_tail;
extern struct ril_utf_expect_t *ril_utf_rec_head;
extern struct ril_utf_expect_t *ril_utf_rec_tail;
extern int q_empty;
extern pthread_t mt_id;
extern pthread_mutex_t main_thread_q_access;
extern pthread_mutex_t main_thread_ril_q_access;
extern pthread_mutex_t reset_flag_mutex;
extern boolean reset_now_flag;
extern pthread_cond_t main_thread_q_ready;
extern struct ril_request_q_t ril_request_q;

/*========================================================
 * Local Global Variables
========================================================*/
#define MAX_MSG_PAIRS 200
struct token_msg_pair {
  int token_id;
  int msg_id;
  int valid;
};
pthread_mutex_t token_msg_mutex = PTHREAD_MUTEX_INITIALIZER;
struct token_msg_pair token_msg_list[MAX_MSG_PAIRS] = {0};

// MSIM RIL instance
int rild_instance = 1;

/*========================================================
 * Local Functions
========================================================*/
void insert_token_msg_pair(int token, int msg_id)
{
  pthread_mutex_lock(&token_msg_mutex);
  int x;

  for (x = 0; x < MAX_MSG_PAIRS; ++x)
  {
    if (token_msg_list[x].valid == 0)
    {
      token_msg_list[x].token_id = token;
      token_msg_list[x].msg_id = msg_id;
      token_msg_list[x].valid = 1;
      break;
    }
  }
  pthread_mutex_unlock(&token_msg_mutex);
}
int lookup_msg_id(int token_id)
{
  pthread_mutex_lock(&token_msg_mutex);
  int x;
  int msg_id = -1;

  for (x = 0; x < MAX_MSG_PAIRS; ++x)
  {
    if ( (token_msg_list[x].valid == 1) &&
         (token_msg_list[x].token_id == token_id) )
    {
      msg_id = token_msg_list[x].msg_id;
      token_msg_list[x].valid = 0;
      break;
    }
  }

  pthread_mutex_unlock(&token_msg_mutex);
  return msg_id;
}


/*========================================================
  - The below structure to sent as argument when calling
    RIL_init().
========================================================*/
static struct RIL_Env s_rilEnv = {
    RIL_onRequestComplete,
    RIL_onUnsolicitedResponse,
    RIL_requestTimedCallback
};

/*========================================================
  START. UTL "rild" thread implementation.

  FUNCTIONS:
   - rild_start
   - start_rild_thread
   - RIL_register
   - RIL_onRequestComplete
   - RIL_onUnsolicitedResponse
   - RIL_requestTimedCallback
   - process_callbacks
   - main_thread_ev_loop
   - start_main_thread

   UTL Global variable:
   -  RIL_Env s_rilEnv
========================================================*/

/*========================================================
  FUNCTION: rild_start
  - Entry function for rild thread
  - Call Ril_Init() and supply the necessary function ptrs
    - The implementation for these function ptrs will be in
       UTL, but the functions themselves will be called from
       QMI RIL
  - Store the returned function function pointers.
    - These functions will be implemented by QMI RIL, and
      called by UTL. (rild would be calling them on target)
  - The function (and by extension the "rild" thread, will
    block indefinitely. (same behavior as on target).
========================================================*/
#ifndef UTF_TARGET_BUILD
void *rild_start(void *param)
{
  char* t_argv[MAX_LIB_ARGS];
  int   t_argc;
  const RIL_RadioFunctions *funcs_inst=NULL;

  // Configure arguments
  memset(t_argv, 0, sizeof(char *) * MAX_LIB_ARGS);
  t_argv[0] = (char*)malloc(sizeof(char) * 20);
  t_argv[1] = (char*)malloc(sizeof(char) * 20);
  t_argv[2] = (char*)malloc(sizeof(char) * 20);
  strlcpy(t_argv[0], "./rild", 20);
  strlcpy(t_argv[1], "-c", 20);
  snprintf(t_argv[2], 20, "%d", rild_instance);
  t_argc = 3;

  // Call RilInit
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Initiating Ril_Init");
  funcs_inst = RIL_Init( (const struct RIL_Env *) &s_rilEnv, t_argc, t_argv);

  // Free arguments
  free(t_argv[0]);
  free(t_argv[1]);
  free(t_argv[2]);

  if ( funcs_inst )
  {
    RIL_UTF_DEBUG("\n Calling RIL_register");
    RIL_register(funcs_inst);
  }
  else
  {
    RIL_UTF_DEBUG("\n Failed!!! not calling RIL_register");
    //TODO: Fail and exit test here
  }

  //check for exit condition
  while (1)
  {
    if (ril_utf_check_reboot_state() == 1)
    {
      ril_utf_rild_thread_shutdown();
    }
    usleep(1);
  }
}
#endif

/*========================================================
  FUNCTION: start_rild_thread
  - Starts the rild thread.
========================================================*/
#ifndef UTF_TARGET_BUILD
void start_rild_thread()
{
  pthread_attr_t rild_thrd_attr;
  int ret;

  pthread_attr_init(&rild_thrd_attr);
  ret = pthread_create(&rild_id, &rild_thrd_attr, rild_start, NULL);

  if ( ret < 0 )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Failed to start rild thread. Exiting...");
    return;
  }
}
#endif
/*========================================================
  FUNCTION: RIL_register
  - The RIL_RadioFunctions (refer ril.h) will be stored
    by UTL
========================================================*/
void RIL_register (const RIL_RadioFunctions *callbacks)
{
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Ril instance %d", (int) callbacks);
  memcpy(&qmi_ril_funcs, callbacks, sizeof(RIL_RadioFunctions));
  platform_register(&qmi_ril_funcs);
}

/*========================================================
  FUNCTION: RIL_onRequestComplete
  - UTL implementation of a RIL function (on target, this
    function implementation would be provided by rild)
  - Compare the provided message and len with the
    expecation set by the test.
  - UTL considers RIL_onRequestComplete an "expectation", and
    the test would be blocked until this expecatation is met.
  - Once verification is complete, the condition
    "signal_exptectation" is signalled.

  Critical section access:
  - exec_q_access is expected to be locked prior to calling
    this function.
     - For access to ril_utf_exp_ptr
     - mutex lock and unlocked in this function
========================================================*/
void RIL_onRequestComplete(RIL_Token t, RIL_Errno e, void *response, size_t responselen)
{
  int msg_id;
  void *data_local;

  int my_expect_slot = acquire_expectation_table();

  // Extract token
  int token_id = *((int*)t);
  free(t);

  msg_id = lookup_msg_id(token_id);
  RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Received RIL response in %s msg_id %d, token id %d", __func__, msg_id, token_id);

  // Ok here we need to switch on message ID to do the correct deep copy
  // This means we need an additional feature that keeps track of message ids based on token ids (OH JOY)
  if (msg_id == RIL_REQUEST_GET_CURRENT_CALLS)
  {
    RIL_Call** tmp = copy_ril_request_get_current_calls_resp((RIL_Call**) response, responselen);
    data_local = (void*) tmp;
  }
  else if (msg_id == RIL_REQUEST_LAST_CALL_FAIL_CAUSE)
  {
    RIL_LastCallFailCauseInfo* tmp = copy_ril_request_last_call_fail_cause_resp((RIL_LastCallFailCauseInfo*) response);
    data_local = (void*) tmp;
  }
  else if (msg_id == RIL_REQUEST_OPERATOR)
  {
    ril_request_operator_resp_from_qcril_t* tmp = copy_ril_request_operator_resp((char**) response);
    data_local = (void*) tmp;
  }
  else if (msg_id == RIL_REQUEST_QUERY_AVAILABLE_NETWORKS)
  {
    ril_request_query_available_networks_resp_t* tmp = copy_ril_request_query_available_networks_resp((char**) response, responselen);
    data_local = (void*) tmp;
  }
  else if (msg_id == RIL_REQUEST_QUERY_CALL_FORWARD_STATUS)
  {
    RIL_CallForwardInfo **tmp = copy_ril_request_query_call_forward_status_resp((RIL_CallForwardInfo**) response, responselen);
    data_local = (void*) tmp;
  }
  else if ((msg_id == RIL_REQUEST_GET_SIM_STATUS))
  {
     RIL_CardStatus_v6 *tmp = copy_ril_request_get_sim_status_resp((RIL_CardStatus_v6 *) response, responselen);
     data_local = (void*) tmp;
  }
  else
  {
    data_local = (void *)malloc(responselen);
    memcpy(data_local, response, responselen);
  }

  enqueue_ril_expect(data_local, responselen, msg_id, token_id, e, ril_utf_ril_response);

  release_expectation_table(my_expect_slot);
}

/*========================================================
  FUNCTION: RIL_onUnsolicitedResponse
  - UTL implementation of a RIL function (on target, this
    function implementation would be provided by rild)
  - Handle an unsol response from QMI RIL.
  - Special case:
    - In the case of RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED
      a call to the "onStateRequest" call should be made
      before returing from RIL_onUnsolicitedResponse.
    - This is to emulate on target behavior.
  - UTL considers RIL_onUnsolicitedResponse an "expectation", and
    the test would be blocked until this expecatation is met.
  - Once verification is complete, the condition
    "signal_exptectation" is signalled.

  Critical section access:
  - exec_q_access is expected to be locked prior to calling
    this function.
     - For access to ril_utf_exp_ptr
     - mutex lock and unlocked in this function

  NOTE:
  - In case of DSDS two unsol functions are needed,
    corresponding to each of the commands interface.
========================================================*/
void RIL_onUnsolicitedResponse(int unsolResponse, const void *data, size_t datalen)
{
  void *data_local = (void *)malloc(datalen);
  memcpy(data_local, data, datalen);
  size_t len_local = datalen;

  struct PACKED ril_unsol_resp_radio_state_changed_t *data_temp;

  // if message is UNSOL_RADIO_STATE_CHANGED, then issue onStateRequest
  // We are simulating rild behaviour here, and using the payload for the
  // state variable.
  if ( unsolResponse == RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED )
  {
    data_temp = (struct PACKED_STRUCT ril_unsol_resp_radio_state_changed_t*) malloc(sizeof(struct PACKED_STRUCT ril_unsol_resp_radio_state_changed_t));
    if (qmi_ril_funcs.onStateRequest != NULL) {
        data_temp->state = qmi_ril_funcs.onStateRequest();
    }
    if (data_local) {
      free(data_local);
    }
    data_local = data_temp;
    datalen = sizeof(struct PACKED_STRUCT ril_unsol_resp_radio_state_changed_t);
  }
  else if ( unsolResponse == RIL_UNSOL_ON_USSD )
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: RIL_UNSOL_ON_USSD copy the data");
    char** tmp = copy_ril_unsol_on_ussd((char**)data, datalen);
    if (data_local) {
      free(data_local);
    }
    data_local = (void*) tmp;
  }

  int my_expect_slot = acquire_expectation_table();

#if 0
  // if message is UNSOL_RADIO_STATE_CHANGED, then issue onStateRequest
  // We are simulating rild behaviour here, and using the payload for the
  // state variable.
  if ( unsolResponse == RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED )
  {
    data_temp = (struct PACKED_STRUCT ril_unsol_resp_radio_state_changed_t*) malloc(sizeof(struct PACKED_STRUCT ril_unsol_resp_radio_state_changed_t));
    data_temp->state = qmi_ril_funcs.onStateRequest();
    data_local = data_temp;
    datalen = sizeof(struct PACKED_STRUCT ril_unsol_resp_radio_state_changed_t);
  }
  else if ( unsolResponse == RIL_UNSOL_ON_USSD )
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: RIL_UNSOL_ON_USSD copy the data");
    char** tmp = copy_ril_unsol_on_ussd((char**)data, datalen);
    data_local = (void*) tmp;
  }
#endif

  RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Received unsolResponse in %s", __func__);
  enqueue_ril_expect(data_local, len_local, unsolResponse, -1, 0, ril_utf_ril_unsol_response);

  release_expectation_table(my_expect_slot);

#if 0
  if ( unsolResponse == RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED )
  {
    if ( data_temp )
    {
      /*
          other 'data" payload is not allocated here. However in the case of
          RIL_UNSOL_RESPONSE_RADIO_STATE_CHANGED since we are allocating
          memory, it should be freed here.
      */
      free(data_temp);
    }
  }
#endif

}

/*========================================================
  FUNCTION: enqueue_ril_expect
    - Add received qmi message to queue for test thread
  Critical section access:
  - exec_q_access is expected to be locked prior to calling
    this function.
========================================================*/
void enqueue_ril_expect( void  *payload, int payload_len, unsigned long msg_id,
                     int token_id, RIL_Errno e, enum ril_utf_q_element_t type )
{
  struct ril_utf_msg_t *expect_node =
    (struct ril_utf_msg_t *) malloc(sizeof(struct ril_utf_msg_t));
  expect_node->payload = payload;
  expect_node->payload_len = payload_len;
  expect_node->msg_id = msg_id;
  expect_node->token_id = token_id;
  expect_node->error_val = e;
  expect_node->type = type;
  // check for expectation
  enqueue_ril_utf_q_node(expect_node, &ril_utf_rec_head, &ril_utf_rec_tail);
}

/*========================================================
  FUNCTION: RIL_requestTimedCallback
  - UTL implementation of a RIL function (on target, this
    function implementation would be provided by rild)
  - adds the provided call back to the UTL "timed_cb_list"
    if no timeout specified.
  - otherwise, starts a timer.

  Critical section access:
  - None
========================================================*/
void RIL_requestTimedCallback (RIL_TimedCallback callback, void *param, const struct timeval *relativeTime)
{
    // start the "timer"
    start_callback_timer( callback, param, relativeTime);
}

int acquire_expectation_table()
{
    int i, expect_slot;
    struct timeval e_timeout = {0, EXPECT_TIMEOUT};
    struct timespec thread_timeout;

    RIL_UTF_DEBUG("\n QMI_RIL_UTL: %s. waiting for exec_q_mutex.", __func__);
    // acquire mutex
    EXEC_Q_LOCK();
    // Grab next available slot in expect table
    for ( i = 0; i < MAX_EXPECTS; ++i )
    {
        if (expect_table[i].in_use == 0)
        {
            expect_slot = i;
            expect_table[expect_slot].in_use = 1;
            break;
        }
    }

    // verify that we are allowed to proceed
    while ( expect_table[expect_slot].expect_ready != 1 )
    {
        calc_timeout(&thread_timeout, &e_timeout);
        pthread_cond_timedwait(&expect_ready_sig, &exec_q_access, &thread_timeout);
    }
    return expect_slot;
}

void release_expectation_table(int expect_slot)
{
    // signal the test execution thread
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Setting expect_received to true @ %s %s %d ",__FILE__, __func__, __LINE__);
    expect_table[expect_slot].expect_received = 1;
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Signaling expectation");
    pthread_cond_signal(&signal_exptectation);

    // release Q lock
    EXEC_Q_UNLOCK();
}

/*========================================================
  FUNCTION:process_callbacks
  - Process all elements in the main thread dispatch queue
     - The dispatch queue can contain either calbacks (through
       call to RIL_setuptimedcallback) or RIL requests from
       Android telephony
  - when returning the "q_empy" cond is set to true.

  Mutexes:
  - none in this function

  Critical section access:
  - The entire function accesses UTL critical section (for
    main thread dispatch queue).
  - It is expected that the main_thread_q_access mutex is
    locked prior to call to this function.
  - Exec_q_mutex is locked to inform test thread of completed
    inject condition
========================================================*/
void process_callbacks()
{

  // Process Timed Callbacks
  //----------------------------------------------
  struct timed_cb_t *node =  timed_cb_list_head;
  struct timed_cb_t *tmp_for_free;

  while ( node )
  {
    tmp_for_free = node;

  // RELEASE LOCK TO AVOID DEADLOCK WITH CALLED CALLBACK
    MAIN_THRD_DISPATCH_Q_UNLOCK();
    tmp_for_free->callback( tmp_for_free->param );
    MAIN_THRD_DISPATCH_Q_LOCK();

    node = node->next;
    free(tmp_for_free);

  }

  timed_cb_list_head = timed_cb_list_tail = NULL;
  //----------------------------------------------


  MAIN_THRD_DISPATCH_Q_UNLOCK();

  // Process Ril requests DOES NOT NEED MAIN THRD LOCK
  //----------------------------------------------
  // Lock mutext for ril_request_q
  MAIN_THRD_RIL_REQUEST_Q_LOCK();
  // Process RIL_Requests
  struct ril_utf_msg_t *r_node = ril_request_q.head;
  struct ril_utf_msg_t *tmp;
  while ( r_node )
  {
    int *tmp_token = (int*)malloc(sizeof(int));
    *tmp_token = r_node->token_id;

    EXEC_Q_LOCK();
    // Inform test thread we are finished
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: Setting inject_complete to TRUE in %s", __func__);
    inject_complete = 1;
    pthread_cond_signal(&inject_complete_sig);
    EXEC_Q_UNLOCK();

    insert_token_msg_pair(r_node->token_id, r_node->msg_id);
    if (platform_onRequest(r_node->msg_id, r_node->payload,
            r_node->payload_len, tmp_token)) {
        qmi_ril_funcs.onRequest(r_node->msg_id, r_node->payload,
                r_node->payload_len, tmp_token);
    } else {
        free(tmp_token);
    }
    tmp = r_node;
    r_node = r_node->next;

    if (tmp) {
      if (tmp->payload) {
        free (tmp->payload);
      }
      free(tmp);
    }
  }
  ril_request_q.head = ril_request_q.tail = NULL;
  MAIN_THRD_RIL_REQUEST_Q_UNLOCK();
  //----------------------------------------------

  // Return to calling thread lock state
  MAIN_THRD_DISPATCH_Q_LOCK();
  q_empty = TRUE;
}

/*========================================================
  FUNCTION:main_thread_ev_loop
  - Called when starting the main thread.
  - Contains the main thread event loop
  - Waits (blocks) indefinitely on the "main_thread_q_ready"
    condition.

  Mutexes used:
  - main_thread_q_access
    - locks upon startup, and release when waiting for cond
    - locked again when procesing the q

  Critical section access:
  - None in this function, but in utility functions called
    from here.
========================================================*/
void* main_thread_ev_loop( void *param )
{
  // Acquire Q lock
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: locking main_thread_q_access");
  MAIN_THRD_DISPATCH_Q_LOCK();

  struct timeval e_timeout;
  struct timespec timeout;
  e_timeout.tv_sec = 0;
  e_timeout.tv_usec = 10;

  while( EXEC_MAIN_THREAD )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Exec...main thrd");
    // Wait for items to be placed in Q
    while ( q_empty  )
    {
      calc_timeout(&timeout, &e_timeout);
      pthread_cond_timedwait( &main_thread_q_ready, &main_thread_q_access, &timeout );

      // Check for reboot request
      if (ril_utf_check_reboot_state() == 1)
      {
        MAIN_THRD_DISPATCH_Q_UNLOCK();
        ril_utf_main_thread_shutdown();
      }
    }


    // read Q's and act
    process_callbacks();
  }

  // release Q lock
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: unlocking main_thread_q_access");
  MAIN_THRD_DISPATCH_Q_UNLOCK();
}

/*========================================================
  FUNCTION: start_main_thread
    - Starts the rild "main" thread
========================================================*/
void start_main_thread()
{
  pthread_attr_t mt_thrd_attr;
  int ret;

  pthread_attr_init(&mt_thrd_attr);
  ret = pthread_create(&mt_id, &mt_thrd_attr, main_thread_ev_loop, NULL);

  if ( ret < 0 )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Failed to start main thread. Exiting...");
    return;
  }
}

void ril_utf_rild_thread_shutdown()
{
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Executing rild thread shutdown");
  pthread_exit(NULL);
}

void ril_utf_main_thread_shutdown()
{
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Executing main thread shutdown");
  pthread_exit(NULL);
}

void ril_utf_set_primary_rild()
{
  #ifdef RIL_UTF_L_MASTER
  rild_instance = 1;
  #else
  rild_instance = 0;
  #endif
}
void ril_utf_set_secondary_rild()
{
  #ifdef RIL_UTF_L_MASTER
  rild_instance = 2;
  #else
  rild_instance = 1;
  #endif
}

/*========================================================
  END. UTL rild thread implementation.
========================================================*/

