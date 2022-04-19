/******************************************************************************
#  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include "ril_utf_test_if.h"
#include "ril_utf_ims_sim.h"
#include "qcril_qmi_lte_direct_disc_msg_meta.h"
#ifdef UTF_TARGET_BUILD
#include "ril_utf_target_test_qmi_ril.h"
#endif
#ifndef RIL_UTF_IOE
#include "ril_utf_test_qmi_ril.h"
#endif
#ifdef QMI_RIL_HAL_UTF
#include "qcril_qmi_oem_reqlist.h"
#endif
extern "C"{
#ifndef RIL_UTF_IOE
#include "ril_utf_androidTel_payload.h"
#include "ril_utf_ims_payload.h"
#include "qcril_qmi_lte_direct_disc_packing.h"
#endif
extern struct ril_utf_q_t *ril_utf_rec_exp_head;
extern struct ril_utf_q_t *ril_utf_rec_exp_tail;
extern struct ril_utf_msg_t *ril_utf_rec_head;
extern struct ril_utf_msg_t *ril_utf_rec_tail;
extern pthread_mutex_t exec_q_access;
extern pthread_cond_t signal_exptectation;
extern pthread_cond_t inject_complete_sig;
extern pthread_cond_t expect_ready_sig;
extern struct expect_timing_t expect_table[MAX_EXPECTS];
extern int inject_complete;
extern int GEN_CALL_FLOW;
extern int qmi_thrd_complete;
}

#define NOTUSED(x) (void)x
int curr_ril_id = 1;
int curr_ims_id = 1;

using namespace std;

extern int shutdown_after_execution;
extern int is_boot_up;
/*========================================================
  FUNCTION: ril_utf_test_t
  - default constructor
========================================================*/
ril_utf_test_t::ril_utf_test_t()
{
  head = NULL;
  tail = NULL;
  in_exp_set = false;
  in_callflow_summary = false;

  in_loop_set = false;
  start_loop = NULL;
  loop_fail = false;

  curr_oem_id = 1;
  curr_ioe_id = 2;
  points_passed = 0;
  points_failed = 0;

  clear_db = false;
  testPointNum = 0;

  // Default off target boot case
  boot_case = qmi_ril_test_bootup_001;
  cleanup_case = qmi_ril_reboot_cleanup;
}

/*========================================================
  FUNCTION: ril_utf_test_t
  - assignment operator overload
========================================================*/
ril_utf_test_t& ril_utf_test_t::operator=(const ril_utf_test_t& t)
{
  clear_exec_q();

  ril_utf_node_base_t *prev    = NULL;
  ril_utf_node_base_t *src_p   = t.head;

  append_exec_q(prev, src_p);

  type = t.type;
  attr = t.attr;
  result = t.result;
  return *this;
}

/*========================================================
  FUNCTION: ril_utf_test_t
  - + operator overload
========================================================*/
ril_utf_test_t ril_utf_test_t::operator+(ril_utf_test_t& t)
{
  ril_utf_node_base_t *prev    = tail;
  ril_utf_node_base_t *src_p   = t.head;

  append_exec_q(prev, src_p);

  ril_utf_test_t result = *this;

  return result;
}

/*========================================================
  FUNCTION: ~ril_utf_test_t
  - default constructor
========================================================*/
ril_utf_test_t::~ril_utf_test_t()
{
  clear_exec_q();
}

/*========================================================
  FUNCTION: append_exec_q
  - append a list starting from p_src to prev.
  - if prev == NULL, that the start of the new list
    is the start of the list
========================================================*/
void ril_utf_test_t::append_exec_q(ril_utf_node_base_t *prev, ril_utf_node_base_t *src_p)
{
  ril_utf_node_base_t *current = NULL;
  while ( src_p )
  {
    current = new ril_utf_node_base_t;
    *current = *src_p;
    if( prev )
    {
      prev->set_next(current);
    }
    else
    {
      head = current;
    }
    prev = current;
    src_p = src_p->get_next();
  }
  tail = current;
}
/*========================================================
  FUNCTION: point_pass
  - mark a success test point
========================================================*/
void ril_utf_test_t::point_pass(void)
{
  RIL_UTF_HTML_LOG_ORIG("\n QMI_RIL_UTL: TEST POINT PASS", __func__);
  points_passed++;
}
/*========================================================
  FUNCTION: clear_exec_q
  - mark a fail test point
========================================================*/
void ril_utf_test_t::point_fail(void)
{
  RIL_UTF_HTML_LOG_ORIG("\n QMI_RIL_UTL: TEST POINT FAIL", __func__);
  points_failed++;
}
/*========================================================
  FUNCTION: clear_exec_q
  - mark a fail test point
========================================================*/
void ril_utf_test_t::print_summary(void)
{
  if (GEN_CALL_FLOW == TRUE)
  {
    return;
  }

  // Log results to log file
  RIL_UTF_HTML_LOG("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
  RIL_UTF_HTML_LOG("     TEST RESULTS");
  RIL_UTF_HTML_LOG("");
  RIL_UTF_HTML_LOG("     Test Name: %s", result.test_id.c_str());
  RIL_UTF_HTML_LOG("");
  if ( points_failed == 0 )
  {
    RIL_UTF_HTML_LOG("     Test PASSED with:");
  }
  else
  {
    RIL_UTF_HTML_LOG("     Test FAILED with:");
  }

  RIL_UTF_HTML_LOG("     %3d test points passed", points_passed);
  RIL_UTF_HTML_LOG("     %3d test points failed", points_failed);
  RIL_UTF_HTML_LOG("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
  fflush(stdout);


  // print pass or fail to screen
  if ( points_failed == 0 )
  {
    fprintf(stdout, "     Test PASSED\n");
  }
  else
  {
    fprintf(stdout, "     Test FAILED\n");
  }
  fprintf(stdout, "--------\n");
  fflush(stdout);
}

/*========================================================
  FUNCTION: clear_exec_q
  - clear the execution q
========================================================*/
void ril_utf_test_t::clear_exec_q(void)
{
  ril_utf_node_base_t *temp = NULL;

  while( head )
  {
    temp = head;
    head = head->get_next();
    temp->clear_payload();
    delete(temp);
  }

  head = tail = NULL;
}
/*========================================================
  FUNCTION: add_node
  - appends a node to the end of the execution list

  Critical section access:
  - Although the exec Q is updated as part of this step
    it is expected that all inj/exp will be added prior
    to calling execute. Therefore, no need to acquire
    qxec_q_access here.
========================================================*/
void ril_utf_test_t::add_node( ril_utf_node_base_t& node)
{
  ril_utf_node_base_t *node_t = new ril_utf_node_base_t;
  *node_t = node;

  if( tail )
  {
    tail->set_next(node_t);
    node_t->set_prev(tail);
    tail = node_t;
  }
  else
  {
    head = tail = node_t;
  }
}
/*========================================================
  FUNCTION: remove_list_node
  - removes node from generic double linked list

  Critical section access:
  - Although the exec Q is updated as part of this step
    it is expected that all inj/exp will be added prior
    to calling execute. Therefore, no need to acquire
    qxec_q_access here.
========================================================*/
int ril_utf_test_t::remove_list_node(struct ril_utf_expect_set_q_t* i,
                                     struct ril_utf_expect_set_q_t* &head_ptr,
                                     struct ril_utf_expect_set_q_t* &tail_ptr)
{
  // Remove from list
  if ( i->next == NULL )
  {
    tail_ptr = i->prev;
  }
  else
  {
    i->next->prev = i->prev;
  }
  if ( i->prev == NULL )
  {
    head_ptr = i->next;
  }
  else
  {
    i->prev->next = i->next;
  }

  return 0;
}
/*========================================================
  FUNCTION: clear_list
  - removes all nodes from generic double linked list

  Critical section access:
  - Although the exec Q is updated as part of this step
    it is expected that all inj/exp will be added prior
    to calling execute. Therefore, no need to acquire
    qxec_q_access here.
========================================================*/
int ril_utf_test_t::clear_list(struct ril_utf_expect_set_q_t* &head_ptr,
                               struct ril_utf_expect_set_q_t* &tail_ptr)
{
  struct ril_utf_expect_set_q_t *i;
  for ( i = head_ptr; i != NULL;)
  {
    struct ril_utf_expect_set_q_t *tmp = i;
    i = i->next;
    free(tmp);
  }

  head_ptr = NULL;
  tail_ptr = NULL;

  return 0;
}

/*========================================================
  FUNCTION: inject
  - "inject" a node to the execution list

  Critical section access:
  - Although the exec Q is updated as part of this step
    it is expected that all inject will be added prior
    to calling execute. Therefore, no need to acquire
    qxec_q_access here.
========================================================*/
void ril_utf_test_t::_inject( ril_utf_node_base_t& node, const source_location_t &location )
{
  node.set_node_type(INJECT);
  node.set_in_exp_set(in_exp_set);
  node.set_in_loop_set(in_loop_set);
  if ( node.get_payload_type() == ril_utf_oem_request )
  {
    node.set_token_id(curr_oem_id);
  }
  else if(node.get_payload_type() == ril_utf_ioe_request )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: set token id");
   node.set_token_id(curr_ioe_id);
  }
  node.set_in_callflow_summary(in_callflow_summary);
  node.set_source_location(location);
  add_node(node);
  node.clear_payload();

}

/*========================================================
  FUNCTION: expect
  - "expect" a node to the execution list

  Critical section access:
  - Although the exec Q is updated as part of this step
    it is expected that all inject will be added prior
    to calling execute. Therefore, no need to acquire
    qxec_q_access here.
========================================================*/
void ril_utf_test_t::_expect( ril_utf_node_base_t& node, const source_location_t &location )
{
  if ( node.get_payload_type() == ril_utf_oem_request )
  {
    node.set_token_id(curr_oem_id);
    ++curr_oem_id;
  }
  else if ( node.get_payload_type() == ril_utf_ims_indication )
  {
    node.set_token_id(-1);
  }

  node.set_node_type(EXPECT);
  node.set_in_exp_set(in_exp_set);
  node.set_in_loop_set(in_loop_set);
  node.set_in_callflow_summary(in_callflow_summary);
  node.set_source_location(location);
  add_node(node);
  node.clear_payload();
}

/*========================================================
  FUNCTION: callflow_summary
  - allows portions of test cases to be omitted from
    call flow diagrams and replaced with a single
    description

  Critical section access:
  - Although the exec Q is updated as part of this step
    it is expected that all inject will be added prior
    to calling execute. Therefore, no need to acquire
    qxec_q_access here.
========================================================*/
void ril_utf_test_t::callflow_summary_start()
{
  in_callflow_summary = true;
}
void ril_utf_test_t::callflow_summary_end(char *summary)
{
  in_callflow_summary = false;

  CallflowSummary cf;
  cf.set_description(summary);
  cf.set_node_type(CALLFLOW_SUMMARY);
  add_node(cf);
}

/*========================================================
  FUNCTION: sleep
  - Add a sleep delay to the execution sequence

  Critical section access:
  - Although the exec Q is updated as part of this step
    it is expected that all inject will be added prior
    to calling execute. Therefore, no need to acquire
    qxec_q_access here.
========================================================*/
void ril_utf_test_t::sleep( int milli_sec )
{
  TimeoutNode tn;
  tn.set_timeout_value( milli_sec );
  tn.set_node_type(TIMEOUT_NODE);
  add_node(tn);
}
/*========================================================
  FUNCTION: block_client_init
  - block_client_init for a service
========================================================*/
void ril_utf_test_t::block_client_init(enum ril_utf_qmi_srv_id_t service)
{
  BlockClientInitNode bn;
  bn.set_block_state(1);
  bn.set_blocked_service(service);
  bn.set_node_type(BLOCK_CLIENT_INIT);
  add_node(bn);
}

/*========================================================
  FUNCTION: unblock_block_client_init
  - block_client_init for a service
========================================================*/
void ril_utf_test_t::unblock_client_init()
{
  BlockClientInitNode bn;
  bn.set_block_state(0);
  bn.set_node_type(BLOCK_CLIENT_INIT);
  add_node(bn);
}


/*========================================================
  FUNCTION: add_canary
  - add timeout to test execution sequence

  Critical section access:
  - Although the exec Q is updated as part of this step
    it is expected that all inject will be added prior
    to calling execute. Therefore, no need to acquire
    qxec_q_access here.
========================================================*/
void ril_utf_test_t::add_canary( ril_utf_canary_if& node )
{
  node.set_node_type(CANARY_NODE);
  add_node(node);
  node.clear_payload();
}

/*========================================================
  FUNCTION: add_func_exec
  - add arbitrary function execution to test

  Critical section access:
  - Although the exec Q is updated as part of this step
    it is expected that all inject will be added prior
    to calling execute. Therefore, no need to acquire
    qxec_q_access here.
========================================================*/
void ril_utf_test_t::add_func_exec( void(*func)())
{
  FuncExecnode fn;
  fn.update_default_func_exec_node(func);
  fn.set_node_type(FUNC_EXEC_NODE);
  add_node(fn);
}

/*========================================================
  FUNCTION: set_start
  - mark the beginning of an unorderd set of nodes

  Critical section access:
  - Although the exec Q is updated as part of this step
    it is expected that all inject will be added prior
    to calling execute. Therefore, no need to acquire
    qxec_q_access here.
========================================================*/
void ril_utf_test_t::set_start(void)
{
  in_exp_set = true;
}
/*========================================================
  FUNCTION: set_end
  - mark the end of an unorderd set of nodes

  Critical section access:
  - Although the exec Q is updated as part of this step
    it is expected that all inject will be added prior
    to calling execute. Therefore, no need to acquire
    qxec_q_access here.
========================================================*/
void ril_utf_test_t::set_end(void)
{
  in_exp_set = false;
}

void ril_utf_test_t::set_test_type(enum ril_utf_test_type_t t)
{
}

/*========================================================
  FUNCTION: set_loop_start/end

  Critical section access:
  - Although the exec Q is updated as part of this step
    it is expected that all inject will be added prior
    to calling execute. Therefore, no need to acquire
    qxec_q_access here.
========================================================*/
void ril_utf_test_t::loop_start(void)
{
  in_loop_set = true;
}
void ril_utf_test_t::loop_end(void)
{
  in_loop_set = false;
}

/*========================================================
 FUNCTION: execute
 - executes the test case.
 - goes through each node in the execution q one by one.

 Critical section access:
 The mutex exec_q_access is locked at the start of the execution queue.
 When  the execution queue is waiting on an expectation, as part
 of the call to pthread_cond_timedwait, the mutex exec_q_access is
 unlocked.

 During the time the mutex remains unlocked by the execution queue, the
 following entities may acquire the lock:
  - a call to qmi_client_send_sync (in the context of the thread that is
    calling the function, in QMI RIL).
  - a call to qmi_client_send_async (in the context of the thread that is
    calling the function, in QMI RIL).
  - by the "qmi_cb" thread, when it is signalled by the execution queue to
    wake up and process (either an indication or a qmi_callback)

 There are two versions of execute() function.
 execute(void) is for normal UTF use and it takes no argument
 execute(void(*update_globals)(void)) is for the execution of auto generated
 test case, which takes a function pointer as an input to update qcril global
 variables during execution

========================================================*/
void ril_utf_test_t::execute(void)
{
  // Just pass an empty pointer to the execute function with argument
  execute(NULL);
}

void ril_utf_test_t::execute(void(*update_globals)(void))
{
  Testnode *test_step =  head;
  char outfile_name[200] = "";
  int expect_ret = QCRIL_UNITTTEST_NO_ERR;
  qmi_thrd_complete = 0;

  // print name to the screen
  fprintf(stdout, "--------\n");
  fprintf(stdout, "     Test Name: %s\n", result.test_id.c_str());
  // Generate documentation instead of executing test
  if (GEN_CALL_FLOW == TRUE)
  {
    gen_call_flow_diagram(test_step);
    return;
  }

#ifndef RIL_UTF_IOE
 /**************************************************/
 if (boot_case != NULL)
 {
    // Do pre-initialization in qcril-hal like pre-init Modules as static Module objects will be
    // created only once in case of UTF
    // No need to call qmi_ril_reboot_cleanup for first test case.
    static boolean cleanup_request = FALSE;
    if (cleanup_request) {
      if (cleanup_case != NULL) {
        (*cleanup_case)();
      }
    } else {
      cleanup_request = TRUE;
    }

#ifndef UTF_TARGET_BUILD
    // Open log file for test execution
    pthread_mutex_lock(&ril_utf_log_lock_mutex);
    snprintf(outfile_name, 200, "log/%s.txt", result.test_id.c_str());
    utf_log_file = fopen(outfile_name, "w");
    if (utf_log_file == NULL)
    {
      fprintf(stderr, "Error cannot open log file in ./log");
      exit(1);
    }
    pthread_mutex_unlock(&ril_utf_log_lock_mutex);
#endif
    /* The func pointer update_globals != NULL indicates that it's an auto
       generated testcase. In this case we need to choose whether to bootup or
       not based on global flags is_boot_up, and update global variables after
       that. Otherwise, bootup between each test case by default */

    /* Bootup UTF unless
       1) it is an auto generated test case and
       2) UTF is already been booted up */
    if (!(update_globals && is_boot_up))
    {
      // Boot test software
      ril_utf_init();
      init_timed_cb_emulation();
      start_main_thread();
      if (clear_db)
      {
        sqlite3* qcril_db_handle = NULL;
        sqlite3_open_v2("qcril.db", &qcril_db_handle, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
        char *zErrMsg = NULL;
        sqlite3_exec(qcril_db_handle, "delete from qcril_operator_type_table", NULL, NULL, &zErrMsg);
        sqlite3_exec(qcril_db_handle, "delete from qcril_properties_table", NULL, NULL, &zErrMsg);
        sqlite3_exec(qcril_db_handle, "delete from qcril_hw_mbn_file_type_table", NULL, NULL, &zErrMsg);
        sqlite3_exec(qcril_db_handle, "delete from qcril_sw_mbn_file_type_table", NULL, NULL, &zErrMsg);
        sqlite3_exec(qcril_db_handle, "delete from qcril_sw_mbn_iin_table", NULL, NULL, &zErrMsg);
        sqlite3_exec(qcril_db_handle, "delete from qcril_sw_mbn_mcc_mnc_table", NULL, NULL, &zErrMsg);
        sqlite3_exec(qcril_db_handle, "vacuum", NULL, NULL, &zErrMsg);
        sqlite3_close(qcril_db_handle);
      }
#ifdef UTF_TARGET_BUILD
      utf_start_rild_threads();
      (*boot_case)();
#else
      (*boot_case)();
#endif
      is_boot_up = 1;
    }

    /* For auto generated test case
       1) do not shutdown UTF after that
       2) update global variables before execution */
    if (update_globals)
    {
      shutdown_after_execution = 0;
      EXEC_Q_LOCK();
      (*update_globals)();
      EXEC_Q_UNLOCK();
    }
  }
  else
  {
    // Start up qcril once we are running boot test case
    start_rild_thread();
  }
  /**************************************************/
#endif


#ifndef UTF_TARGET_BUILD
  // Add canary node for this test (makes sure no expects are left unhandled at end of test)
  CanaryNode cn;
  add_canary(cn);
#endif

  // Initialize expect table (used for enforcing correct ordering of expect nodes)
  memset(&expect_table, 0, sizeof(expect_table));
  int current_expect_slot = 0;

  RIL_UTF_HTML_LOG_ORIG("\n==================================================================================");
  RIL_UTF_HTML_LOG_ORIG("\nTEST NAME: %s", result.test_id.c_str());
  RIL_UTF_HTML_LOG_ORIG("\n==================================================================================");

  // Check that all unordered expect sets are closed
  if ( in_exp_set != false )
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error: test has unordered expect set that isn't ended %s", __func__);
    return;
  }

  // Acquire Q lock
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: locking exec_q_access in %s\n", __func__);
  EXEC_Q_LOCK();

  // Print the first test number
  print_test_point();

  while ( test_step )
  {
    if ( test_step->get_node_type() == EXPECT )
    {
        //Expect
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Handling expect @ %s %s %d ",__FILE__, __func__, __LINE__);
        expect_ret = handle_expect_step( test_step, current_expect_slot );
        if ( expect_ret < QCRIL_UNITTTEST_NO_ERR )
        {
          if (in_loop_set == true)
          {
            loop_fail = true;
            test_step->set_token_id(-1);
          }

          if( test_step->is_fatal_on_failure() == true )
          {
            print_header();
            RIL_UTF_DEBUG("\n QMI_RIL_UTL: Expect failure fatal. Exiting test case...@ %s %s %d ",__FILE__, __func__, __LINE__);
            print_header();
            break;
          }
        }
        if (test_step->get_next()->get_node_type() != CANARY_NODE)
        {
          print_test_point();
        }
    }
    else if ( test_step->get_node_type() == INJECT )
    {
        //Inject
        RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Handling inject @ %s %s %d",__FILE__, __func__, __LINE__);
        if ( handle_inject_step( test_step ) != QCRIL_UNITTTEST_NO_ERR )
        {
          if( test_step->is_fatal_on_failure() )
          {
            print_header();
            RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Inject failure fatal. Exiting test case...@ %s %s %d ",__FILE__, __func__, __LINE__);
            print_header();
            break;
          }
        }
    }
    else if (test_step->get_node_type() == TIMEOUT_NODE )
    {
      int ms = ((TimeoutNode*)test_step)->get_timeout_value();
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Running timeout node for %d milliseconds", ms);
      usleep(ms * 1000);
    }
    else if (test_step->get_node_type() == CANARY_NODE )
    {
      int ms = ((CanaryNode*)test_step)->get_timeout_micro();
      handle_canary_timeout(ms, current_expect_slot);
    }
    else if (test_step->get_node_type() == FUNC_EXEC_NODE )
    {
      void (*tmp)();
      tmp = ((FuncExecnode*)test_step)->get_func_exec_node();
      // execute callback function
      tmp();
    }
    else if (test_step->get_node_type() == BLOCK_CLIENT_INIT )
    {
      int block = ((BlockClientInitNode*)test_step)->get_block_state();
      enum ril_utf_qmi_srv_id_t service = ((BlockClientInitNode*)test_step)->get_blocked_service();
      if (block == 1)
      {
        block_qmi_client_init(service);
      }
      else
      {
        unblock_qmi_client_init();
      }
    }
    else if (test_step->get_node_type() == CALLFLOW_SUMMARY )
    {
      // not used here
    }
    else
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Invalid case. exec list item should be inject or expect. Exiting test case...");
      break;
    }

    // Do not advance to next expect if target receives unexpected message
    if (expect_ret == QCRIL_UNITTEST_TARGET_UNEXPECTED_MESSAGE)
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: TARGET UNEXPECTED MESSAGE");
    }
    else
    {
      // move execution to next element.
      test_step = test_step->get_next();
    }

    // Handle loop admin where we repeat nodes until they pass
    handle_loop_set(test_step);

  } // end while

  // Release  lock
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: unlocking exec_q_access in %s", __func__);
  EXEC_Q_UNLOCK();
  result.num_test_points = points_passed + points_failed;
  result.test_points_passed = points_passed;
  result.test_points_failed = points_failed;
  result.status = STATUS_COMPLETED;
}

/*========================================================
  FUNCTION: handle_inject_step
  - injects the message based on the node type.

  Critical section access:
   - exec_q_access should be locked prior to calling this
    function
========================================================*/
int ril_utf_test_t::handle_inject_step( struct ril_utf_node_base_t* &test_step )
{
  int err_val = QCRIL_UNITTTEST_NO_ERR;
  inject_complete = 0;

  if ((test_step->get_payload_type() == ril_utf_ril_request) &&
      (test_step->get_token_id() == -1))
  {
    test_step->set_token_id(curr_ril_id);
    outstanding_ril_tokens[test_step->get_msg_id()] = curr_ril_id;
  }

  if ((test_step->get_payload_type() == ril_utf_ims_request) &&
      (test_step->get_token_id() == -1))
  {
    test_step->set_token_id(curr_ims_id);
    outstanding_ims_tokens[test_step->get_msg_id()] = curr_ims_id;
  }

  RIL_UTF_HTML_LOG_ORIG("\n");
  RIL_UTF_HTML_LOG("@@@@@@@@@@@@@@@@@@@ Begin of Inject @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
  RIL_UTF_HTML_LOG("@@@@@@@@@@@@@@@@@@@ Function: %s", test_step->get_source_location().function);
  RIL_UTF_HTML_LOG("@@@@@@@@@@@@@@@@@@@ File: %s Line: %d", test_step->get_source_location().file,
                                                            test_step->get_source_location().line_number);
  test_step->print_injection();
  RIL_UTF_HTML_LOG("@@@@@@@@@@@@@@@@@@@ End of Inject @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");

  // Set inject to be sent
  struct ril_utf_msg_t *inject_ptr = update_inj_ptr(test_step);

  switch( test_step->get_payload_type() )
  {
    case ril_utf_ril_request:
#ifdef UTF_TARGET_BUILD
      err_val = send_ril_utf_rild_socket_request( inject_ptr );
#else
      err_val =  send_ril_utf_ril_request( inject_ptr );
#endif
      break;

    case ril_utf_qmi_response:
      err_val = send_ril_utf_qmi_response( inject_ptr );
      break;

    case ril_utf_qmi_indication:
      err_val = send_ril_utf_qmi_indication( inject_ptr );
      break;

    case ril_utf_ims_request:
#ifndef RIL_UTF_IOE
      err_val = send_ril_utf_ims_request( inject_ptr );
#endif
      break;

    case ril_utf_oem_request:
#ifndef RIL_UTF_IOE
      err_val = send_ril_utf_oem_request( inject_ptr );
#endif
      break;

    case ril_utf_msim_send_msg:
#ifndef RIL_UTF_IOE
      err_val = send_ril_utf_msim_request( inject_ptr );
#endif
      break;

    case ril_utf_qmi_request:
      err_val = send_ril_utf_proxy_qmi_request( inject_ptr );
      break;

#ifdef RIL_UTF_IOE
    case ril_utf_ioe_request:
      err_val = send_ril_utf_ioe_request( inject_ptr );
      break;
#endif
  case ril_utf_ssr_error_cb:
      err_val = send_ril_utf_ssr_error_cb( inject_ptr );
      break;

    case ril_utf_db_request:
      err_val = send_ril_utf_db_request (inject_ptr);
      break;

  case ril_utf_ssr_notify_cb:
      err_val = send_ril_utf_ssr_notify_cb( inject_ptr );
      break;

  case ril_utf_property_msg:
      err_val = send_ril_utf_property_request (inject_ptr);
      break;

  case ril_utf_dsi_indication:
      err_val = send_ril_utf_dsi_indication( inject_ptr );
      break;

  case ril_utf_dsi_response:
      err_val = send_ril_utf_dsi_response( inject_ptr );
      break;

  case ril_utf_ril_response:
  default:
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: %s %s %d. Must be ril_request, ims_request, qmi_resp or qmi_ind:", __FILE__, __func__, __LINE__);
    //do nothing
    break;
  }

  // Wait for inject to report complete
  struct timespec timeout;
  struct timeval e_timeout;
  e_timeout.tv_sec = 0;
  e_timeout.tv_usec = INJECT_TIMEOUT;
  RIL_UTF_DEBUG("\n QMI_RIL_UTF: Releasing exec_q_mutex");
  while ( inject_complete == 0 )
  {
    calc_timeout(&timeout, &e_timeout);
    pthread_cond_timedwait(&inject_complete_sig, &exec_q_access, &timeout);
  }

  RIL_UTF_DEBUG("\n QMI_RIL_UTF: Received inject confirmed\n");

  return err_val;
}

/*========================================================
  FUNCTION: gen_call_flow_diagram
    - this function runs instead of actual test execution
    - creates .png images representing call flows and
      places them in the ./doc folder

  Critical section access:
   - exec_q_access should be locked prior to calling this
    function
========================================================*/
int ril_utf_test_t::gen_call_flow_diagram( ril_utf_node_base_t* &test_step)
{
  FILE *text_out;
  string test_id = get_test_id();
  string test_desc = get_test_description();
  char desc_path[200];
  char client[30];
  char RILserver[30];
  #ifdef RIL_UTF_IOE
    snprintf(client, 30, "IoEClient");
    snprintf(RILserver, 30, "IoERIL");
  #else
    snprintf(client, 30, "AndroidTelephony");
    snprintf(RILserver, 30, "QMIRIL");
  #endif
  // Open correct file for description used to generate diagram
  snprintf(desc_path, 200, "./doc/%s", test_id.c_str());
  text_out = fopen(desc_path, "w");

  // Diagram header
  fprintf(text_out, "msc {\n");
  fprintf(text_out, "hscale = \"2\";\n\n");
  fprintf(text_out, "%s,%s,Modem;\n\n",client,RILserver);
  fprintf(text_out, "%s abox Modem [ label = \"%s\" ];\n", client, test_desc.c_str());

  while (test_step != NULL)
  {
    const char *msg_name;
    const char *tag = test_step->get_callflow_tag();
    int print_tag = 0;

    // check if summary node
    if (test_step->get_node_type() == CALLFLOW_SUMMARY)
    {
      fprintf(text_out, "%s note Modem [ label = \"%s\", textbgcolour=\"#ffff00\" ];\n",
        client,
        ((CallflowSummary*)test_step)->get_description());
      test_step = test_step->get_next();
      continue;
    }

    // check for skip printing
    if (test_step->get_in_callflow_summary() == true)
    {
      test_step = test_step->get_next();
      continue;
    }

    // check for node tag
    if (strcmp(tag, "") != 0)
      print_tag = 1;

    switch ( test_step->get_payload_type() )
    {
      case ril_utf_qmi_request:
        msg_name = get_qmi_message_name(test_step->get_service_id(), test_step->get_msg_id());
        if (test_step->get_node_type() == INJECT)
        {
          fprintf(text_out, "%s<-Modem [ label = \"%sREQ\" ];\n",RILserver, msg_name);
        }
        else if (test_step->get_node_type() == EXPECT)
        {
          fprintf(text_out, "%s->Modem [ label = \"%sREQ\" ];\n",RILserver, msg_name);
        }
        else
        {
          // Pass we only print expects and injects
        }
        if (print_tag == 1)
          fprintf(text_out,"%s note Modem [ label = \"%s\", textbgcolour = \"#7f7fff\" ];\n",RILserver, tag);
        break;
      case ril_utf_qmi_response:
        msg_name = get_qmi_message_name(test_step->get_service_id(), test_step->get_msg_id());
        if (test_step->get_node_type() == INJECT)
        {
          fprintf(text_out, "%s<-Modem [ label = \"%sRESP\" ];\n",RILserver, msg_name);
        }
        else if (test_step->get_node_type() == EXPECT)
        {
          fprintf(text_out, "%s->Modem [ label = \"%sRESP\" ];\n",RILserver, msg_name);
        }
        else
        {
          // Pass we only print expects and injects
        }
        if (print_tag == 1)
          fprintf(text_out,"%s note Modem [ label = \"%s\", textbgcolour = \"#7f7fff\" ];\n",RILserver, tag);
        break;
      case ril_utf_qmi_indication:
        msg_name = get_qmi_message_name(test_step->get_service_id(), test_step->get_msg_id());
        if (test_step->get_node_type() == INJECT)
        {
          fprintf(text_out, "%s<-Modem [ label = \"%sIND\" ];\n",RILserver, msg_name);
        }
        else if (test_step->get_node_type() == EXPECT)
        {
          fprintf(text_out, "%s->Modem [ label = \"%sIND\" ];\n",RILserver, msg_name);
        }
        else
        {
          // Pass we only print expects and injects
        }
        if (print_tag == 1)
          fprintf(text_out,"%s note Modem [ label = \"%s\", textbgcolour = \"#7f7fff\" ];\n",RILserver, tag);
        break;
 #ifdef RIL_UTF_IOE
     case ril_utf_ioe_request:
         msg_name =    get_ioe_message_name(test_step->get_msg_id());
         if (test_step->get_node_type() == INJECT)
        {
          fprintf(text_out, "%s->%s [ label = \"%sREQ\" ];\n",client,RILserver, msg_name);
        }
        else if (test_step->get_node_type() == EXPECT)
        {
          fprintf(text_out, "%s<-%s [ label = \"%sREQ\" ];\n",client, RILserver, msg_name);
        }
        if (print_tag == 1)
           fprintf(text_out,"%s note %s [ label = \"%s\", textbgcolour = \"#7f7fff\" ];\n",client,RILserver, tag);
        break;
     case ril_utf_ioe_response:
       msg_name =    get_ioe_message_name(test_step->get_msg_id());
         if (test_step->get_node_type() == INJECT)
        {
          fprintf(text_out, "%s->%s [ label = \"%sRESP\" ];\n",client,RILserver, msg_name);
        }
        else if (test_step->get_node_type() == EXPECT)
        {
          fprintf(text_out, "%s<-%s [ label = \"%sRESP\" ];\n",client, RILserver, msg_name);
        }
        if (print_tag == 1)
           fprintf(text_out,"%s note %s [ label = \"%s\", textbgcolour = \"#7f7fff\" ];\n",client,RILserver, tag);
        break;
     case ril_utf_ioe_indication:
        msg_name =    get_ioe_message_name(test_step->get_msg_id());
         if (test_step->get_node_type() == INJECT)
        {
          fprintf(text_out, "%s->%s [ label = \"%s\" ];\n",client,RILserver, msg_name);
        }
        else if (test_step->get_node_type() == EXPECT)
        {
          fprintf(text_out, "%s<-%s [ label = \"%s\" ];\n",client, RILserver, msg_name);
        }
        if (print_tag == 1)
           fprintf(text_out,"%s note %s [ label = \"%s\", textbgcolour = \"#7f7fff\" ];\n",client,RILserver, tag);
       break;
 #else
       case ril_utf_ril_request:
      case ril_utf_ril_unsol_response:
      case ril_utf_ril_response:
        msg_name = get_ril_message_name(test_step->get_msg_id());
        if (test_step->get_node_type() == INJECT)
        {
          fprintf(text_out, "%s->%s [ label = \"%s\" ];\n",client, RILserver, msg_name);
        }
        else if (test_step->get_node_type() == EXPECT)
        {
          fprintf(text_out,"%s<-%s [label = \"%s\" ];\n",client, RILserver, msg_name);
        }
        else
        {
          // Pass we only print expects and injects
        }
        if (print_tag == 1)
          fprintf(text_out,"%s note %s [ label = \"%s\", textbgcolour = \"#7f7fff\" ];\n",client,RILserver, tag);
        break;
      case ril_utf_ims_request:
      case ril_utf_ims_response:
    case ril_utf_ims_indication:
        msg_name = get_ims_message_name(test_step->get_msg_id());
        if (test_step->get_node_type() == INJECT)
        {
          fprintf(text_out, "%s->%s [ label = \"%s\" ];\n",client, RILserver, msg_name);
        }
        else if (test_step->get_node_type() == EXPECT)
        {
          fprintf(text_out, "%s<-%s [ label = \"%s\" ];\n",client, RILserver, msg_name);
        }
        else
        {
          // Pass we only print expects and injects
        }
        if (print_tag == 1)
          fprintf(text_out,"%s note %s [ label = \"%s\", textbgcolour = \"#7f7fff\" ];\n",client,RILserver, tag);
        break;

      case ril_utf_oem_request:
      case ril_utf_oem_response:
    case ril_utf_oem_indication:
        msg_name = get_oem_message_name(test_step->get_service_id(), test_step->get_msg_id());
        if (test_step->get_node_type() == INJECT)
        {
          fprintf(text_out, "%s->%s [ label = \"%s\" ];\n",client,RILserver, msg_name);
        }
        else if (test_step->get_node_type() == EXPECT)
        {
          fprintf(text_out, "%s<-%s [ label = \"%s\" ];\n",client, RILserver, msg_name);
        }
        else
        {
          // Pass we only print expects and injects
        }
        if (print_tag == 1)
          fprintf(text_out,"%s note %s [ label = \"%s\", textbgcolour = \"#7f7fff\" ];\n",client, RILserver, tag);
        break;
#endif
    }

    test_step = test_step->get_next();
  }


  // Diagram footer
  fprintf(text_out, "}\n");
  fclose(text_out);

  char command[300] = "";
  snprintf(command, 300, "./doc/mscgen -T png -i %s -o %s.png", desc_path, desc_path);

  system(command);
  remove(desc_path);

  return 0;

}

/*========================================================
  FUNCTION: handle_loop_set
   - handles looping mechanism that repeats a set of
     nodes until all pass

  Critical section access:
   - exec_q_access should be locked prior to calling this
    function
========================================================*/
int ril_utf_test_t::handle_loop_set(struct ril_utf_node_base_t* &test_step)
{
  // We are at the end of loop and either need to pass through or repeat
  if ( (test_step == NULL) || (test_step->get_in_loop_set() == false) )
  {
    if ( (in_loop_set == true) &&
         (loop_fail == true) )
    {
      test_step = start_loop; // reset to beginnng of loop
    }
    in_loop_set = false;
    return 0;
  }

  // We are at the beginning of loop and need to set up globals
  if ( (test_step->get_in_loop_set() == true) &&
       (in_loop_set == false) )
  {
    start_loop = test_step;
    in_loop_set = true;
    loop_fail = false;
  }

  return 0;
}

/*========================================================
  FUNCTION: handle_expect_step
   - performs basic verification on the expect test_step,
      (TBD)
   - blocks on condition "signal_exptectation" or
     until timeout.

  Critical section access:
   - exec_q_access should be locked prior to calling this
    function
========================================================*/
int ril_utf_test_t::handle_expect_step( struct ril_utf_node_base_t* &test_step,
                                        int &current_expect_slot)
{
  int err_val = QCRIL_UNITTTEST_NO_ERR;
  struct timespec   timeout;
  struct ril_utf_msg_t *ril_utf_exp_ptr;

  // Assign ril token id
  if ( (test_step->get_payload_type() == ril_utf_ril_response ) &&
       (test_step->get_token_id() == -1) )
  {
    int temp_token = outstanding_ril_tokens[test_step->get_msg_id()];
    outstanding_ril_tokens.erase(test_step->get_msg_id());
    test_step->set_token_id(temp_token);
    ++curr_ril_id;
  }
  else if ( ( test_step->get_payload_type() == ril_utf_ims_response ) &&
           (test_step->get_token_id() == -1) )
  {
    int temp_token = outstanding_ims_tokens[test_step->get_msg_id()];
    outstanding_ims_tokens.erase(test_step->get_msg_id());
    test_step->set_token_id(temp_token);
    ++curr_ims_id;
  }

  // Wait on expect if we don't already have one in queue
  if ( ril_utf_rec_head == NULL )
  {
    // calculate timeout for expect
    struct timeval e_timeout;
    e_timeout = test_step->get_expect_timeout();

    // Signal that expectation is ready
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Set expect_ready to true @ %s %s %d ",__FILE__, __func__, __LINE__);
    expect_table[current_expect_slot].expect_ready = 1;
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Broadcast expect_ready_sig @ %s %s %d ",__FILE__, __func__, __LINE__);
    pthread_cond_broadcast(&expect_ready_sig);

    // Print expected message
    RIL_UTF_HTML_LOG_ORIG("\n");
    RIL_UTF_HTML_LOG("@@@@@@@@@@@@@@@@@@@ Begin of Expect @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    RIL_UTF_HTML_LOG("@@@@@@@@@@@@@@@@@@@ Function: %s", test_step->get_source_location().function);
    RIL_UTF_HTML_LOG("@@@@@@@@@@@@@@@@@@@ File: %s Line: %d", test_step->get_source_location().file,
                                                            test_step->get_source_location().line_number);
    RIL_UTF_HTML_LOG("============ Begin of Expected Message =====================================");
    test_step->print_expectation();
    RIL_UTF_HTML_LOG("============ End of Expected Message =======================================");

    // Wait for message to be received
    if ((test_step->get_payload_type() == ril_utf_db_response) || (test_step->get_payload_type() == ril_utf_property_msg))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: start waiting for expecatation @ %s %s %d ",__FILE__, __func__, __LINE__);
      expect_table[current_expect_slot].in_use = 1;
      expect_table[current_expect_slot].expect_ready = 1;
      expect_table[current_expect_slot].expect_received = 1;

      char exit_msg[10];
      memset(exit_msg, 0, sizeof(exit_msg));
      if (test_step->get_payload_type() == ril_utf_property_msg)
      {
        err_val = send_ril_utf_property_response(test_step->get_payload(), test_step->get_payload_len());
        strlcpy(exit_msg, "Property", 9);
      }
      else {
        err_val = send_ril_utf_db_response(test_step->get_payload(), test_step->get_payload_len());
        strlcpy(exit_msg, "DB", 3);
      }
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: end waiting for expectation @ %s %s %d ",__FILE__, __func__, __LINE__);

      RIL_UTF_HTML_LOG("@@@@@@@@@@@@@@@@@@@ TEST REPORT @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
      if (err_val != 0)
      {
        RIL_UTF_HTML_LOG(" QMI_RIL_UTL: TEST REPORT: Error @%s %s %d Value from %s does not match", __FILE__, __func__, __LINE__, exit_msg);
        RIL_UTF_HTML_LOG("@@@@@@@@@@@@@@@@@@@ End of TEST REPORT @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
        err_val = QCRIL_UNITTEST_GEN_ERR;
        point_fail();
        ++current_expect_slot;
        return err_val;
      }
      else
      {
        RIL_UTF_HTML_LOG(" QMI_RIL_UTL: TEST REPORT: Pass. Value from %s matches.", exit_msg);
        RIL_UTF_HTML_LOG("@@@@@@@@@@@@@@@@@@@ End of TEST REPORT @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
        err_val = QCRIL_UNITTTEST_NO_ERR;
        point_pass();
        ++current_expect_slot;
        return err_val;
      }
    }
    else
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: start waiting for expecatation @ %s %s %d ",__FILE__, __func__, __LINE__);
      while ( 1 )
      {
        calc_timeout(&timeout, &e_timeout);
        err_val = pthread_cond_timedwait( &signal_exptectation, &exec_q_access, &timeout );
        if ( (err_val == ETIMEDOUT) || (expect_table[current_expect_slot].expect_received == 1) )
        {
          break;
        }
      }
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: end waiting for expectation @ %s %s %d ",__FILE__, __func__, __LINE__);
    }
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Increment current_expect_slot @ %s %s %d ",__FILE__, __func__, __LINE__);
    ++current_expect_slot;

  }

  // check for wait timeout
  if ( err_val == ETIMEDOUT )
  {
    RIL_UTF_HTML_LOG_ORIG("\n");
    RIL_UTF_HTML_LOG("@@@@@@@@@@@@@@@@@@@ TEST REPORT @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
    RIL_UTF_HTML_LOG("     QMI_RIL_UTL: TEST REPORT: Error: %d @ %s %s %d TIMED OUT.", err_val, __FILE__, __func__, __LINE__);
    RIL_UTF_HTML_LOG("@@@@@@@@@@@@@@@@@@@ End of TEST REPORT @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    err_val = QCRIL_UNITTEST_TIMOUT_ERR;
    point_fail();
    return err_val;
  }

  // Pop current message sent by sync/async/ril_unsol
  ril_utf_exp_ptr = pop_front_ril_utf_q_node(&ril_utf_rec_head, &ril_utf_rec_tail);
  if ( ril_utf_exp_ptr == NULL )
  {
    RIL_UTF_HTML_LOG("     Error: No message sent. \n");
    point_fail();
    err_val = QCRIL_UNITTEST_GEN_ERR;
    return err_val;
  }


#ifdef UTF_TARGET_BUILD
  // Special case pass for Target
  //   any message with incorrect message id is nominal pass
  // For unsol messages only match on msg id
  if ( (test_step->get_payload_type() == ril_utf_ril_unsol_response) &&
       (test_step->get_msg_id() != ril_utf_exp_ptr->msg_id) )
  {
    return QCRIL_UNITTEST_TARGET_UNEXPECTED_MESSAGE;
  }
  // For response messages only match on token id
  else if ( (test_step->get_payload_type() == ril_utf_ril_response) &&
            (test_step->get_msg_id() != ril_utf_exp_ptr->msg_id) )
  {
    return QCRIL_UNITTEST_TARGET_UNEXPECTED_MESSAGE;
  }
  else if( (test_step->get_payload_type() == ril_utf_ims_indication) &&
           (test_step->get_msg_id() != ril_utf_exp_ptr->msg_id) )
  {
    return QCRIL_UNITTEST_TARGET_UNEXPECTED_MESSAGE;
  }
  else if( (test_step->get_payload_type() == ril_utf_ims_response) &&
           (test_step->get_msg_id() != ril_utf_exp_ptr->msg_id) )
  {
    return QCRIL_UNITTEST_TARGET_UNEXPECTED_MESSAGE;
  }
#endif


  if (test_step->get_in_exp_set() == true)
  {
    // Check for and handle unordered expect set
    if ( handle_unordered_exp_set( test_step, ril_utf_exp_ptr) == false )
    {
      RIL_UTF_HTML_LOG(" Error: no message in expect set matched!");
      //TODO: fail out of test case at this point
    }
  }

  RIL_UTF_HTML_LOG("");
  RIL_UTF_HTML_LOG("============ Begin of Received Message =====================================");
  RIL_UTF_HTML_LOG("     Received Message:");
  #ifndef RIL_UTF_IOE
  print_received_message(ril_utf_exp_ptr);
  #endif
  RIL_UTF_HTML_LOG("============ End of Received Message =======================================");
  RIL_UTF_HTML_LOG("@@@@@@@@@@@@@@@@@@@ End of Expect @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
  // Verify message is correct
  validate_expect(test_step, ril_utf_exp_ptr);

  RIL_UTF_HTML_LOG_ORIG("\n");
  RIL_UTF_HTML_LOG("@@@@@@@@@@@@@@@@@@@ TEST REPORT @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");

  if( test_step->get_expect_status() != met )
  {
    RIL_UTF_HTML_LOG(" QMI_RIL_UTL: TEST REPORT: Error @%s %s %d expectation not met", __FILE__, __func__, __LINE__);
    RIL_UTF_HTML_LOG("@@@@@@@@@@@@@@@@@@@ End of TEST REPORT @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    err_val = QCRIL_UNITTEST_GEN_ERR;
    point_fail();
  }
  else if( test_step->get_expect_status() == met )
  {
    RIL_UTF_HTML_LOG(" QMI_RIL_UTL: TEST REPORT: Pass. Expectation met.");
    RIL_UTF_HTML_LOG("@@@@@@@@@@@@@@@@@@@ End of TEST REPORT @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    err_val = QCRIL_UNITTTEST_NO_ERR;
    point_pass();
  }

  // Free received message
  if (ril_utf_exp_ptr->payload != NULL)
  {
    //free(ril_utf_exp_ptr->payload);
  }
  free(ril_utf_exp_ptr);

  return err_val;
}

/*========================================================
  FUNCTION: check_unordered_exp_set
     -If we are in an expect set...
      look through all nodes starting with test_node
      and see if any of the marked nodes match....
      If the node does match, move it and all immediately
      following injects before test_step in the linked
      list.  This allows for nodes to arrive in any
      order and still have the correct injects follow it

  Critical section access:
   - exec_q_access should be locked prior to calling this
    function
========================================================*/
bool ril_utf_test_t::handle_unordered_exp_set( struct ril_utf_node_base_t* &test_step,
                                              struct ril_utf_msg_t *ril_utf_exp_ptr)
{
  bool ret_val = true;
  struct ril_utf_node_base_t *curr_node = test_step;
  bool found_match = false;

  find_unordered_match( curr_node, ril_utf_exp_ptr, found_match);

  if ( (found_match == true) &&
       (test_step != curr_node) )
  {
    move_unordered_exp_set( test_step, curr_node);
  }

  if ( (found_match == false) &&
       (test_step->get_in_exp_set() == true) )
  {
    ret_val = false;
  }

  return ret_val;

}
void ril_utf_test_t::find_unordered_match( struct ril_utf_node_base_t* &curr_node,
                                              struct ril_utf_msg_t *ril_utf_exp_ptr,
                                              bool &found_match)
{
  int i = 1;
  while ( (curr_node != NULL) &&
          (curr_node->get_in_exp_set() == true) )
  {
    RIL_UTF_HTML_LOG("     Expecting Following Message #%d:", i);
    curr_node->print_expectation();
    i++;
    if ( (curr_node->get_node_type() == EXPECT) &&
         (curr_node->get_payload_type() == ril_utf_exp_ptr->type) )
    {
      // verification for different message types
      switch ( curr_node->get_payload_type() )
      {
      case ril_utf_ril_response:
        // check to see if ril response has current unmet inject
        // if an inject matching this response has already been sent
        // then we update our token to match
        if ((outstanding_ril_tokens.count(curr_node->get_msg_id()) == 1) &&
            (curr_node->get_token_id() == -1))
        {
          int temp_token = outstanding_ril_tokens[curr_node->get_msg_id()];
          outstanding_ril_tokens.erase(curr_node->get_msg_id());
          curr_node->set_token_id(temp_token);
          ++curr_ril_id;
        }
      case ril_utf_ril_unsol_response:
        if ( (curr_node->get_token_id() == ril_utf_exp_ptr->token_id) &&
           (curr_node->get_msg_id() == ril_utf_exp_ptr->msg_id) )
        {
          found_match = true;
        }
        break;
      case ril_utf_ril_request:
      case ril_utf_ioe_response:
      case ril_utf_ioe_indication:
        if (curr_node->get_msg_id() == ril_utf_exp_ptr->msg_id)
        {
          found_match = true;
        }
        break;
      case ril_utf_qmi_request:
      case ril_utf_qmi_response:
      case ril_utf_qmi_indication:
        if ( (curr_node->get_msg_id() == ril_utf_exp_ptr->msg_id) &&
             (curr_node->get_service_id() == ril_utf_exp_ptr->service_id) )
        {
          found_match = true;
        }
        break;
      case ril_utf_ims_request:
      case ril_utf_ims_response:
      case ril_utf_ims_indication:
        if ( (curr_node->get_msg_id() == ril_utf_exp_ptr->msg_id) &&
             (curr_node->get_token_id() == ril_utf_exp_ptr->token_id) )
        {
          found_match = true;
        }
        break;
      case ril_utf_oem_request:
      case ril_utf_oem_response:
      case ril_utf_oem_indication:
        if (curr_node->get_oem_command_id() == ril_utf_exp_ptr->oem_command_id)
        {
          found_match = true;
        }
        break;
      case ril_utf_msim_recv_msg:
        if (curr_node->get_msg_id() == ril_utf_exp_ptr->msg_id)
        {
          found_match = true;
        }
      }
    } // end same type

    if ( found_match == true )
    {
      break;
    }
    // increment
    curr_node = curr_node->get_next();
  } // end while
}
void ril_utf_test_t::move_unordered_exp_set( struct ril_utf_node_base_t* &test_step,
                                             struct ril_utf_node_base_t* &curr_node)
{
  struct ril_utf_node_base_t *end_of_injects;
  // mark expect that matched and all
  // injects that follow
  end_of_injects = curr_node;
  while ( (end_of_injects->get_next() != NULL) &&
          (end_of_injects->get_next()->get_in_exp_set() == true) &&
          ((end_of_injects->get_next()->get_node_type() != EXPECT) && (end_of_injects->get_next()->get_node_type() != CANARY_NODE) ) )
  {
    end_of_injects = end_of_injects->get_next();
  }

  // remove set of nodes
  if (curr_node->get_prev()) {
    curr_node->get_prev()->set_next(end_of_injects->get_next());
  }
  if ( end_of_injects->get_next() != NULL)
  {
    end_of_injects->get_next()->set_prev(curr_node->get_prev());
  }

  // insert set of nodes to just prior to current test node
  if (test_step->get_prev()) {
    test_step->get_prev()->set_next(curr_node);
  }
  curr_node->set_prev(test_step->get_prev());
  end_of_injects->set_next(test_step);
  test_step->set_prev(end_of_injects);

  // set test step to found expect
  test_step = curr_node;
}

/*========================================================
  FUNCTION: handle_canary_timeout
    - Handles canary node and prints any message that
      that shows up when none are being expected

  Critical section access:
   - exec_q_access should be locked prior to calling this
    function
========================================================*/
void ril_utf_test_t::handle_canary_timeout(int micro_sec,
                                           int &current_expect_slot)
{
  struct ril_utf_msg_t *ril_utf_exp_ptr;
  int err_val;
  struct timespec   timeout;
  struct timeval e_timeout;

  e_timeout.tv_sec = 0;
  e_timeout.tv_usec = micro_sec;

  while (1)
  {
    // Signal that expectation is ready
    expect_table[current_expect_slot].expect_ready = 1;
    pthread_cond_broadcast(&expect_ready_sig);

    calc_timeout(&timeout, &e_timeout);
    err_val = pthread_cond_timedwait( &signal_exptectation, &exec_q_access, &timeout );

    if ( err_val != ETIMEDOUT )
    {
      // Pop current message sent by sync/async/ril_unsol
      ril_utf_exp_ptr = pop_front_ril_utf_q_node(&ril_utf_rec_head, &ril_utf_rec_tail);
      if ( ril_utf_exp_ptr != NULL )
      {
        RIL_UTF_HTML_LOG("");
        RIL_UTF_HTML_LOG("============ Error!! Message received when none was expected ===============");
        RIL_UTF_HTML_LOG("============ Begin of Received Message =====================================");
        RIL_UTF_HTML_LOG("     Received Message:");
        print_received_message(ril_utf_exp_ptr);
        RIL_UTF_HTML_LOG("============ End of Received Message =======================================");

        // Having an unhandled expect constitues test failure
        point_fail();
        if (ril_utf_exp_ptr->payload) {
          free(ril_utf_exp_ptr->payload);
          ril_utf_exp_ptr->payload = NULL;
        }
        free(ril_utf_exp_ptr);
      }
    }
    else
    {
      break;
    }
    ++current_expect_slot;
  } // end while
}

/*========================================================
  FUNCTION: validate_expect
    - verifies message match between what was expected
      and what was actually received

  Critical section access:
   - exec_q_access should be locked prior to calling this
    function
========================================================*/
int ril_utf_test_t::validate_expect ( ril_utf_node_base_t *exp_node,
                                       struct ril_utf_msg_t *data )
{
  int err_val;

  switch ( data->type )
  {
    case ril_utf_ril_request:
    case ril_utf_ril_unsol_response:
#ifndef RIL_UTF_IOE
      validate_ril_unsol( exp_node, data);
#endif
      if (data && data->payload) {
        free(data->payload);
        data->payload = NULL;
      }
      break;
    case ril_utf_ril_response:
#ifndef RIL_UTF_IOE
      validate_ril_resp( exp_node, data);
#endif
      if (data && data->payload) {
        free(data->payload);
        data->payload = NULL;
      }
      break;
    case ril_utf_msim_recv_msg:
#ifndef RIL_UTF_IOE
      validate_msim_recv( exp_node, data);
#endif
      break;
    case ril_utf_qmi_request:
      validate_qmi_request( exp_node, data);
      if (data && data->payload) {
        free(data->payload);
        data->payload = NULL;
      }
      break;
    case ril_utf_ims_response:
    case ril_utf_ims_indication:
#ifndef RIL_UTF_IOE
      validate_ims( exp_node, data);
#endif
      if (data && data->payload) {
        free(data->payload);
        data->payload = NULL;
      }
      break;
    case ril_utf_qmi_response:
      validate_qmi_response( exp_node, data);
      break;
    case ril_utf_qmi_indication:
      validate_qmi_indication( exp_node, data);
      break;
    case ril_utf_oem_response:
    case ril_utf_oem_indication:
#ifndef RIL_UTF_IOE
      validate_oem( exp_node, data);
#endif
      break;
    case ril_utf_ims_request:
      RIL_UTF_ESSENTIAL("ERROR: message type is not expect");
      break;
  case ril_utf_ioe_response:
#ifdef RIL_UTF_IOE
      validate_ioe_response( exp_node, data);
#endif
      break;
  case ril_utf_ioe_indication:
#ifdef RIL_UTF_IOE
      validate_ioe_indication( exp_node, data);
#endif
      break;

  case ril_utf_dsi_request:
  case ril_utf_dsi_response:
      validate_dsi_request_response(exp_node, data);
      break;
  
    default:
      RIL_UTF_ESSENTIAL("ERROR: expect not of known type");
  }

  return err_val;

}
#ifndef RIL_UTF_IOE
void ril_utf_test_t::validate_oem( ril_utf_node_base_t *exp_node,
                                   struct ril_utf_msg_t *data)
{
  RIL_UTF_DEBUG("\n QMI_RIL_UTF: Verifying OEM message");
#ifdef QMI_RIL_HAL_UTF
  if (data->oem_command_id == QCRIL_REQ_HOOK_LTE_DIRECT_DISC_REQ ||
          data->oem_command_id == QCRIL_REQ_HOOK_UNSOL_LTE_DIRECT_DISC)
#else
  if (data->oem_command_id == QCRIL_EVT_HOOK_LTE_DIRECT_DISC_REQ ||
          data->oem_command_id == QCRIL_EVT_HOOK_UNSOL_LTE_DIRECT_DISC)
#endif
  {
    RIL_UTF_DEBUG("\nQMI_RIL_UTF: Received LTE_DIRECT_DISC message");
    validate_lte_direct_disc_message(exp_node, data);
  }
  else
  {

  if ( (exp_node->get_msg_id() != data->msg_id) ||
       (exp_node->get_service_id() != data->service_id) ||
       (exp_node->get_oem_command_id() != data->oem_command_id) )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: expect msg id %x got msg id %x\n",
      exp_node->get_msg_id(), data->msg_id);
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: expect service id %x got service id %x\n",
      exp_node->get_service_id(), data->service_id);
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: expect oem command id %d got oem command id %d\n",
      exp_node->get_oem_command_id(), data->oem_command_id);
    exp_node->set_expect_status(not_met);
    return;
  }

  if ( (exp_node->get_expect_skip_verification_status() == true) &&
       (data->payload != NULL) )
  {
    if ( memcmp(exp_node->get_payload(), data->payload, data->payload_len) != 0 )
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Expected");
      print_msg_in_hex( exp_node->get_payload(), exp_node->get_payload_len() );
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Recvd");
      print_msg_in_hex( data->payload, data->payload_len);
      exp_node->set_expect_status(not_met);
      return;
    }
  }

  exp_node->set_expect_status(met);
  }
}

void ril_utf_test_t::validate_lte_direct_disc_message( ril_utf_node_base_t *exp_node,
    struct ril_utf_msg_t *data)
{
  size_t unpacked_msg_size = 0;
  LteDirectDiscovery_MsgHeader *msg_tag_ptr = NULL;
  void *msg_data_ptr = NULL;
  qcril_evt_e_type rnd_req;

  RIL_UTF_DEBUG("\nQMI_RIL_UTL: data->payload = 0x%x, data->payload_len = %d\n",
      data->payload, data->payload_len);

  do {
    if(data->payload && data->payload_len)
    {
      msg_tag_ptr = (LteDirectDiscovery_MsgHeader *)data->payload;
      int msg_tag_ptr_payload_len = data->payload_len;

      QCRIL_LOG_INFO("QMI_RIL_UTL: msg: %s, type: %d, message_id: %d, error: %d",
          qcril_qmi_lte_direct_disc_get_msg_log_str(msg_tag_ptr->id, msg_tag_ptr->type),
          msg_tag_ptr->type,
          msg_tag_ptr->id,
          msg_tag_ptr->error);

      qcril_qmi_lte_direct_disc_parse_packed_msg(msg_tag_ptr->type, msg_tag_ptr->id,
          msg_tag_ptr->payload, msg_tag_ptr_payload_len, &msg_data_ptr,
          &unpacked_msg_size, &rnd_req);
    }
    else
    {
      RIL_UTF_DEBUG("\nQMI_RIL_UTL: Invalid payload");
      exp_node->set_expect_status(not_met);
      break;
    }

    if (exp_node->get_oem_command_id() != data->oem_command_id)
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: expect oem command id 0x%x got oem command id 0x%x\n",
          exp_node->get_oem_command_id(), data->oem_command_id);
      exp_node->set_expect_status(not_met);
      break;
    }
    if (msg_tag_ptr->type == LteDirectDiscovery_MsgType_RESPONSE &&
        msg_tag_ptr->error != exp_node->get_ril_error_value())
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: expected error value %d got error value %d",
          exp_node->get_ril_error_value(), msg_tag_ptr->error);
      exp_node->set_expect_status(not_met);
      break;
    }
    if (exp_node->get_msg_id() != msg_tag_ptr->id)
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: expect msg id 0x%x got msg id 0x%x\n",
          exp_node->get_msg_id(), msg_tag_ptr->id);
      exp_node->set_expect_status(not_met);
      break;
    }
    // TODO validate token?
    //
    if (exp_node->get_msg_id() == LteDirectDiscovery_MsgId_REQUEST_TERMINATE ||
        exp_node->get_msg_id() == LteDirectDiscovery_MsgId_REQUEST_PUBLISH ||
        exp_node->get_msg_id() == LteDirectDiscovery_MsgId_REQUEST_CANCEL_PUBLISH ||
        exp_node->get_msg_id() == LteDirectDiscovery_MsgId_REQUEST_SUBSCRIBE ||
        exp_node->get_msg_id() == LteDirectDiscovery_MsgId_REQUEST_CANCEL_SUBSCRIBE)
    {
      exp_node->set_expect_status(met);
    }
    if (exp_node->get_msg_id() == LteDirectDiscovery_MsgId_REQUEST_GET_DEVICE_CAPABILITY ||
        exp_node->get_msg_id() == LteDirectDiscovery_MsgId_UNSOL_RESPONSE_DEVICE_CAPABILITY_CHANGED)
    {
      exp_node->set_expect_status(
              validate_lte_direct_disc_device_capability(exp_node->get_payload(), msg_data_ptr));
    }
    else if (exp_node->get_msg_id() == LteDirectDiscovery_MsgId_REQUEST_GET_SERVICE_STATUS ||
             exp_node->get_msg_id() == LteDirectDiscovery_MsgId_UNSOL_RESPONSE_SERVICE_STATUS)
    {
      exp_node->set_expect_status(
          validate_lte_direct_disc_service_status(exp_node->get_payload(), msg_data_ptr));
    }
    else if (exp_node->get_msg_id() == LteDirectDiscovery_MsgId_UNSOL_RESPONSE_EXPRESSION_STATUS)
    {
      exp_node->set_expect_status(
              validate_lte_direct_disc_unsol_expression_status(exp_node->get_payload(), msg_data_ptr));
    }
    else if (exp_node->get_msg_id() == LteDirectDiscovery_MsgId_UNSOL_RESPONSE_AUTHORIZATION_RESULT)
    {
      exp_node->set_expect_status(
              validate_lte_direct_disc_unsol_authorization_result(exp_node->get_payload(), msg_data_ptr));
    }
    else if (exp_node->get_msg_id() == LteDirectDiscovery_MsgId_UNSOL_RESPONSE_TRANSMISSION_STATUS)
    {
      exp_node->set_expect_status(
              validate_lte_direct_disc_unsol_transmission_status(exp_node->get_payload(), msg_data_ptr));
    }
    else if (exp_node->get_msg_id() == LteDirectDiscovery_MsgId_UNSOL_RESPONSE_RECEPTION_STATUS)
    {
      exp_node->set_expect_status(
              validate_lte_direct_disc_unsol_reception_status(exp_node->get_payload(), msg_data_ptr));
    }
    else if (exp_node->get_msg_id() == LteDirectDiscovery_MsgId_UNSOL_RESPONSE_MATCH_EVENT)
    {
      exp_node->set_expect_status(
              validate_lte_direct_disc_unsol_match_event(exp_node->get_payload(), msg_data_ptr));
    }

  } while (FALSE);
}

enum ril_utf_expectation_t ril_utf_test_t::validate_lte_direct_disc_device_capability(void *exp, void *recv)
{
  ril_utf_expectation_t ret = met;
  LteDirectDiscovery_DeviceCapability *e = (LteDirectDiscovery_DeviceCapability*)exp;
  LteDirectDiscovery_DeviceCapability *r = (LteDirectDiscovery_DeviceCapability*)recv;

  do {
    if ((e->has_capability != r->has_capability) ||
            (e->capability != r->capability))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: expected has_capability = \"%d\" but received \"%d\"\n",
              e->has_capability, r->has_capability);
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: expected capability = \"%d\" but received \"%d\"\n",
              e->capability, r->capability);
      ret = not_met;
    }
  } while (FALSE);

  return ret;
}

enum ril_utf_expectation_t ril_utf_test_t::validate_lte_direct_disc_service_status(void *exp, void *recv)
{
  ril_utf_expectation_t ret = met;
  LteDirectDiscovery_ServiceStatus *e = (LteDirectDiscovery_ServiceStatus*)exp;
  LteDirectDiscovery_ServiceStatus *r = (LteDirectDiscovery_ServiceStatus*)recv;

  do {
    if ((e->has_publishAllowed != r->has_publishAllowed) ||
            (e->publishAllowed != r->publishAllowed))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: expected has_publishAllowed = \"%d\" but received \"%d\"\n",
              e->has_publishAllowed, r->has_publishAllowed);
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: expected publishAllowed = \"%d\" but received \"%d\"\n",
              e->publishAllowed, r->publishAllowed);
      ret = not_met;
    }
    if ((e->has_subscribeAllowed != r->has_subscribeAllowed) ||
            (e->subscribeAllowed != r->subscribeAllowed))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: expected has_subscribeAllowed = \"%d\" but received \"%d\"\n",
              e->has_subscribeAllowed, r->has_subscribeAllowed);
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: expected subscribeAllowed = \"%d\" but received \"%d\"\n",
              e->subscribeAllowed, r->subscribeAllowed);
      ret = not_met;
    }
  } while (FALSE);
  return ret;
}

enum ril_utf_expectation_t ril_utf_test_t::validate_lte_direct_disc_unsol_expression_status(void *exp, void *recv)
{
  ril_utf_expectation_t ret = met;
  LteDirectDiscovery_ExpressionStatus *e = (LteDirectDiscovery_ExpressionStatus*)exp;
  LteDirectDiscovery_ExpressionStatus *r = (LteDirectDiscovery_ExpressionStatus*)recv;

  do {
    char *e_str = (char *)e->osAppId.arg;
    char *r_str = (char *)r->osAppId.arg;
    if (e_str != NULL && r_str != NULL &&
        (strcmp(e_str, r_str) != 0))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: mismatched osAppId; expected = \"%s\" but received = \"%s\"\n",
              e_str, r_str);
      ret = not_met;
    }

    e_str = (char *)e->expression.arg;
    r_str = (char *)r->expression.arg;
    if (e_str != NULL && r_str != NULL &&
        (strcmp(e_str, r_str) != 0))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: mismatched expression; expected = \"%s\" but received \"%s\"\n",
              e_str, r_str);
      ret = not_met;
    }

    if ((e->has_result != r->has_result) ||
            (e->result != r->result))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: expected has_result = \"%d\" but received \"%d\"\n",
              e->has_result, r->has_result);
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: expected result = \"%d\" but received \"%d\"\n",
              e->result, r->result);
      ret = not_met;
    }
  } while (FALSE);

  return ret;
}

enum ril_utf_expectation_t ril_utf_test_t::validate_lte_direct_disc_unsol_authorization_result(void *exp, void *recv)
{
  ril_utf_expectation_t ret = met;
  LteDirectDiscovery_AuthorizationResult *e =
        (LteDirectDiscovery_AuthorizationResult*)exp;
  LteDirectDiscovery_AuthorizationResult *r =
        (LteDirectDiscovery_AuthorizationResult*)recv;

  do {
    char *e_str = (char *)e->osAppId.arg;
    char *r_str = (char *)r->osAppId.arg;
    if (e_str != NULL && r_str != NULL &&
        (strcmp(e_str, r_str) != 0))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: mismatched osAppId; expected = \"%s\" but received = \"%s\"\n",
              e_str, r_str);
      ret = not_met;
    }

    if ((e->has_result != r->has_result) ||
            (e->result != r->result))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: expected has_result = \"%d\" but received \"%d\"\n",
              e->has_result, r->has_result);
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: expected result = \"%d\" but received \"%d\"\n",
              e->result, r->result);
      ret = not_met;
    }
  } while (FALSE);

  return ret;
}

enum ril_utf_expectation_t ril_utf_test_t::validate_lte_direct_disc_unsol_transmission_status(void *exp, void *recv)
{
  ril_utf_expectation_t ret = met;
  LteDirectDiscovery_TransmissionStatus *e = (LteDirectDiscovery_TransmissionStatus*)exp;
  LteDirectDiscovery_TransmissionStatus *r = (LteDirectDiscovery_TransmissionStatus*)recv;

  do {
    char *e_str = (char *)e->osAppId.arg;
    char *r_str = (char *)r->osAppId.arg;
    if (e_str != NULL && r_str != NULL &&
        (strcmp(e_str, r_str) != 0))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: mismatched osAppId; expected = \"%s\" but received = \"%s\"\n",
              e_str, r_str);
      ret = not_met;
    }

    e_str = (char *)e->expression.arg;
    r_str = (char *)r->expression.arg;
    if (e_str != NULL && r_str != NULL &&
        (strcmp(e_str, r_str) != 0))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: mismatched expression; expected = \"%s\" but received \"%s\"\n",
              e_str, r_str);
      ret = not_met;
    }

    if ((e->has_status != r->has_status) ||
            (e->status != r->status))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: expected has_status = \"%d\" but received \"%d\"\n",
              e->has_status, r->has_status);
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: expected status = \"%d\" but received \"%d\"\n",
              e->status, r->status);
      ret = not_met;
    }
  } while (FALSE);

  return ret;
}

enum ril_utf_expectation_t ril_utf_test_t::validate_lte_direct_disc_unsol_reception_status(void *exp, void *recv)
{
  ril_utf_expectation_t ret = met;
  LteDirectDiscovery_ReceptionStatus *e = (LteDirectDiscovery_ReceptionStatus*)exp;
  LteDirectDiscovery_ReceptionStatus *r = (LteDirectDiscovery_ReceptionStatus*)recv;

  do {
    char *e_str = (char *)e->osAppId.arg;
    char *r_str = (char *)r->osAppId.arg;
    if (e_str != NULL && r_str != NULL &&
        (strcmp(e_str, r_str) != 0))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: mismatched osAppId; expected = \"%s\" but received = \"%s\"\n",
              e_str, r_str);
      ret = not_met;
    }

    e_str = (char *)e->expression.arg;
    r_str = (char *)r->expression.arg;
    if (e_str != NULL && r_str != NULL &&
        (strcmp(e_str, r_str) != 0))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: mismatched expression; expected = \"%s\" but received \"%s\"\n",
              e_str, r_str);
      ret = not_met;
    }

    if ((e->has_status != r->has_status) ||
            (e->status != r->status))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: expected has_status = \"%d\" but received \"%d\"\n",
              e->has_status, r->has_status);
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: expected status = \"%d\" but received \"%d\"\n",
              e->status, r->status);
      ret = not_met;
    }
  } while (FALSE);

  return ret;
}

enum ril_utf_expectation_t ril_utf_test_t::validate_lte_direct_disc_unsol_match_event(void *exp, void *recv)
{
  ril_utf_expectation_t ret = met;
  LteDirectDiscovery_MatchEvent *e = (LteDirectDiscovery_MatchEvent*)exp;
  LteDirectDiscovery_MatchEvent *r = (LteDirectDiscovery_MatchEvent*)recv;

  do {
    char *e_str = (char *)e->osAppId.arg;
    char *r_str = (char *)r->osAppId.arg;
    if (e_str != NULL && r_str != NULL &&
        (strcmp(e_str, r_str) != 0))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: mismatched osAppId; expected = \"%s\" but received = \"%s\"\n",
              e_str, r_str);
      ret = not_met;
    }

    e_str = (char *)e->expression.arg;
    r_str = (char *)r->expression.arg;
    if (e_str != NULL && r_str != NULL &&
        (strcmp(e_str, r_str) != 0))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: mismatched expression; expected = \"%s\" but received \"%s\"\n",
              e_str, r_str);
      ret = not_met;
    }

    if ((e->has_state != r->has_state) ||
            (e->state != r->state))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: expected has_state = \"%d\" but received \"%d\"\n",
              e->has_state, r->has_state);
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: expected state = \"%d\" but received \"%d\"\n",
              e->state, r->state);
      ret = not_met;
    }

    e_str = (char *)e->metaData.arg;
    r_str = (char *)r->metaData.arg;
    if (e_str != NULL && r_str != NULL &&
        (strcmp(e_str, r_str) != 0))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: mismatched metadata; expected = \"%s\" but received \"%s\"\n",
              e_str, r_str);
      ret = not_met;
    }
  } while (FALSE);

  return ret;
}


void ril_utf_test_t::validate_ims( ril_utf_node_base_t *exp_node,
                                   struct ril_utf_msg_t *data)
{
  RIL_UTF_DEBUG("\n QMI_RIL_UTF: Verifying IMS message");
  if ( (exp_node->get_msg_id() != data->msg_id) ||
       (exp_node->get_token_id() != data->token_id) )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: expect msg id 0x%x got msg id 0x%x\n",
      exp_node->get_msg_id(), data->msg_id);
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: expect token id 0x%x got token id 0x%x\n",
      exp_node->get_token_id(), data->token_id);
    exp_node->set_expect_status(not_met);
    return;
  }

  // Confirm error value
  if ( (data->type == ril_utf_ims_response) &&
       (data->ims_error != exp_node->get_ims_error_value()) )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: expected error value %d got error value %d",
                  exp_node->get_ims_error_value(), data->ims_error);
    exp_node->set_expect_status(not_met);
    return;
  }

  if (exp_node->get_expect_skip_verification_status() == false) {
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: skip verification");
    exp_node->set_expect_status(met);
    return;
  }

#ifdef RIL_UTF_L_MASTER
  if ( exp_node->get_msg_id() == ims_MsgId_UNSOL_RESPONSE_CALL_STATE_CHANGED)
  {
    validate_ims_get_current_calls( exp_node, data);
  }
#else
  if ( exp_node->get_msg_id() == ims_MsgId_REQUEST_GET_CURRENT_CALLS )
  {
    validate_ims_get_current_calls( exp_node, data);
  }
#endif
  else if ( exp_node->get_msg_id() == ims_MsgId_REQUEST_LAST_CALL_FAIL_CAUSE )
  {
    validate_ims_last_call_fail_cause( exp_node, data );
  }
  else if ( exp_node->get_msg_id() == ims_MsgId_REQUEST_IMS_REGISTRATION_STATE )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: ims_MsgId_REQUEST_IMS_REGISTRATION_STATE verification");
    validate_ims_registration_state( exp_node, data );
  }
  else if ( exp_node->get_msg_id() == ims_MsgId_REQUEST_QUERY_CALL_FORWARD_STATUS )
  {
    validate_ims_query_call_forward_status ( exp_node, data );
  }
  else if ( exp_node->get_msg_id() == ims_MsgId_REQUEST_QUERY_CALL_WAITING )
  {
    validate_ims_query_call_waiting ( exp_node, data );
  }
  else if (exp_node->get_msg_id() == ims_MsgId_REQUEST_GET_IMS_SUB_CONFIG ||
           exp_node->get_msg_id() == ims_MsgId_UNSOL_IMS_SUB_CONFIG_CHANGED)
  {
    validate_ims_sub_config(exp_node, data);
  }
  else if (exp_node->get_msg_id() == ims_MsgId_REQUEST_QUERY_VIRTUAL_LINE_INFO)
  {
    validate_ims_query_virtual_line_info(exp_node, data);
  }
  else if (exp_node->get_msg_id() == ims_MsgId_UNSOL_VOICE_INFO)
  {
    validate_ims_unsol_voice_info(exp_node, data);
  }
  else if (exp_node->get_msg_id() == ims_MsgId_UNSOL_MULTI_IDENTITY_REGISTRATION_STATUS)
  {
    validate_ims_unsol_multi_identity_registration_status(exp_node, data);
  }
  else if ( exp_node->get_msg_id() == ims_MsgId_UNSOL_SRV_STATUS_UPDATE ||
      exp_node->get_msg_id() == ims_MsgId_REQUEST_QUERY_SERVICE_STATUS)
  {
    validate_ims_unsol_srv_status_update ( exp_node, data );
  }
  else if ( exp_node->get_msg_id() == ims_MsgId_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED )
  {
    validate_ims_unsol_network_state_changed(exp_node, data);
  }
  else if ( exp_node->get_msg_id() == ims_MsgId_UNSOL_RADIO_STATE_CHANGED )
  {
    validate_ims_unsol_radio_state_changed(exp_node, data);
  }
#ifndef UTF_TARGET_BUILD
  else if ( exp_node->get_msg_id() == ims_MsgId_UNSOL_MWI )
  {
    validate_ims_unsol_mwi ( exp_node, data );
  }
#endif
  else if ( exp_node->get_msg_id() == ims_MsgId_REQUEST_SUPP_SVC_STATUS ||
            exp_node->get_msg_id() == ims_MsgId_REQUEST_SET_COLR ||
            exp_node->get_msg_id() == ims_MsgId_REQUEST_SET_CALL_WAITING )
  {
    validate_ims_request_supp_svc_status_resp ( exp_node, data );
  }
  else if ( exp_node->get_msg_id() == ims_MsgId_REQUEST_QUERY_CLIP )
  {
    validate_ims_request_query_clip_resp ( exp_node, data );
  }
  else if ( exp_node->get_msg_id() == ims_MsgId_REQUEST_SET_CALL_FORWARD_STATUS)
  {
    validate_ims_set_call_forward_status_resp( exp_node, data );
  }
  else if ( exp_node->get_msg_id() == ims_MsgId_REQUEST_EXPLICIT_CALL_TRANSFER )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: ims_MsgId_REQUEST_EXPLICIT_CALL_TRANSFER verification");
    exp_node->set_expect_status(met);
  }
  else if ( exp_node->get_msg_id() == ims_MsgId_REQUEST_DEFLECT_CALL )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: ims_MsgId_REQUEST_DEFLECT_CALL verification");
    exp_node->set_expect_status(met);
  }
  else if ( exp_node->get_msg_id() == ims_MsgId_REQUEST_SET_SUPP_SVC_NOTIFICATION )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: ims_MsgId_REQUEST_SET_SUPP_SVC_NOTIFICATION verification");
    exp_node->set_expect_status(met);
    //validate_set_supp_svc_notification_resp(exp_node, data);
  }
  else if ( exp_node->get_msg_id() == ims_MsgId_UNSOL_REFRESH_CONF_INFO )
  {
    validate_ims_unsol_refresh_conf_info ( exp_node, data );
  }
  else if ( exp_node->get_msg_id() == ims_MsgId_UNSOL_REFRESH_VICE_INFO )
  {
    validate_ims_unsol_refresh_vice_info ( exp_node, data );
  }
  else if ( exp_node->get_msg_id() == ims_MsgId_UNSOL_MODIFY_CALL )
  {
    validate_ims_unsol_modify_call ( exp_node, data );
  }
  else if ( exp_node->get_msg_id() == ims_MsgId_UNSOL_SUPP_SVC_NOTIFICATION )
  {
    validate_ims_unsol_supp_svc_notification ( exp_node, data );
  }
  else if ( exp_node->get_msg_id() == ims_MsgId_UNSOL_RESPONSE_HANDOVER )
  {
    validate_ims_unsol_response_handover ( exp_node, data );
  }
  else if ( exp_node->get_msg_id() == ims_MsgId_REQUEST_SET_IMS_CONFIG ||
            exp_node->get_msg_id() == ims_MsgId_REQUEST_GET_IMS_CONFIG)
  {
    validate_ims_config_msg( exp_node, data );
  }
  else if ( exp_node->get_msg_id() == ims_MsgId_UNSOL_REQUEST_GEOLOCATION )
  {
    validate_imsa_geo_loc( exp_node, data);
  }
  else if ( exp_node->get_msg_id() == ims_MsgId_UNSOL_VOWIFI_CALL_QUALITY ||
            exp_node->get_msg_id() == ims_MsgId_REQUEST_GET_VOWIFI_CALL_QUALITY )
  {
    validate_imsa_vowifi_call_quality(exp_node, data);
  }
  else if ( exp_node->get_msg_id() == ims_MsgId_UNSOL_PARTICIPANT_STATUS_INFO )
  {
    validate_unsol_participant_status_info( exp_node, data);
  }
  else if ( exp_node->get_msg_id() == ims_MsgId_UNSOL_ON_SS )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: ims_MsgId_UNSOL_ON_SS verification");
    exp_node->set_expect_status(met);
    //validate_unsol_on_ss( exp_node, data);
  }
  else if ( exp_node->get_msg_id() == ims_MsgId_UNSOL_INCOMING_IMS_SMS )
  {
    validate_unsol_incoming_ims_sms( exp_node, data);
  }
  else if ( exp_node->get_msg_id() == ims_MsgId_REQUEST_GET_COLR )
  {
    validate_ims_get_colr( exp_node, data);
  }
  else if ( exp_node->get_msg_id() == ims_MsgId_REQUEST_ADD_PARTICIPANT ||
            exp_node->get_msg_id() == ims_MsgId_REQUEST_CONFERENCE ||
            exp_node->get_msg_id() == ims_MsgId_REQUEST_HOLD ||
            exp_node->get_msg_id() == ims_MsgId_REQUEST_RESUME)
  {
    if (exp_node->get_ims_error_value() != ims_Error_E_SUCCESS) {
      validate_ims_sip_error_info_msg(exp_node, data);
    } else {
      exp_node->set_expect_status(met);
    }
  }
  else if ( exp_node->get_msg_id() == ims_MsgId_UNSOL_WFC_ROAMING_MODE_CONFIG_SUPPORT)
  {
    validate_ims_wfc_roaming_config_support(exp_node, data);
  }
  else if ( exp_node->get_msg_id() == ims_MsgId_UNSOL_USSD_FAILED)
  {
    validate_ims_ussd_failed(exp_node, data);
  }
  else if ( exp_node->get_msg_id() == ims_MsgId_UNSOL_RESPONSE_RTT_MSG_RECEIVED )
  {
    validate_imss_rtt_message(exp_node, data);
  }
  else if (exp_node->get_msg_id() == ims_MsgId_UNSOL_RESPONSE_REGISTRATION_BLOCK_STATUS)
  {
    validate_ims_unsol_registration_block_status(exp_node, data);
  }
  else if ( exp_node->get_msg_id() == ims_MsgId_UNSOL_AUTO_CALL_REJECTION_IND )
  {
    validate_ims_unsol_auto_call_rejection_ind ( exp_node, data );
  }
  else
  {
    if ( memcmp(data->payload, exp_node->get_payload(), data->payload_len) == 0 )
    {
      exp_node->set_expect_status(met);
    }
    else
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Expected");
      print_msg_in_hex( exp_node->get_payload(), exp_node->get_payload_len() );
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Recvd");
      print_msg_in_hex( data->payload, data->payload_len);
      exp_node->set_expect_status(not_met);
    }
  }
}

void ril_utf_test_t::validate_ims_unsol_radio_state_changed
(
 ril_utf_node_base_t *exp_node,
 struct ril_utf_msg_t *data
)
{
  ims_RadioStateChanged *exp = (ims_RadioStateChanged *) exp_node->get_payload();
  ims_RadioStateChanged *recv = (ims_RadioStateChanged *) data->payload;

  exp_node->set_expect_status(met); // start with met and mark unmet if failure

  if (exp && recv)
  {
    if ((exp->has_state != recv->has_state) ||
        (exp->state != recv->state))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Expected has_state = %d, state = %d",
          exp->has_state, exp->state);
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Received has_state = %d, state = %d",
          recv->has_state, recv->state);
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match");
      exp_node->set_expect_status(not_met);
      return;
    }
  }
}

void ril_utf_test_t::validate_ims_unsol_network_state_changed
(
 ril_utf_node_base_t *exp_node,
 struct ril_utf_msg_t *data
)
{
  ims_Registration *exp = (ims_Registration *) exp_node->get_payload();
  ims_Registration *recv = (ims_Registration *) data->payload;

  exp_node->set_expect_status(met); // start with met and mark unmet if failure

  if (exp && recv)
  {
    if ((exp->has_state != recv->has_state) ||
        (exp->state != recv->state))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Expected has_state = %d, state = %d",
          exp->has_state, exp->state);
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Received has_state = %d, state = %d",
          recv->has_state, recv->state);
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match");
      exp_node->set_expect_status(not_met);
      return;
    }
  }
}
void ril_utf_test_t::validate_ims_config_msg( ril_utf_node_base_t *exp_node,
                                                     struct ril_utf_msg_t *data)
{
  int i;
  exp_node->set_expect_status(met); // start with met and mark unmet if failure

  if(exp_node->get_payload() != NULL)
  {
      ims_ConfigMsg *exp = (ims_ConfigMsg*)exp_node->get_payload();
      ims_ConfigMsg *recv = (ims_ConfigMsg*)data->payload;

      if( (exp->has_item != recv->has_item) ||
          (exp->item != recv->item) )
      {
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Expected config item: %d, %d Received config item: %d, %d",
                       exp->has_item, exp->item,
                       recv->has_item, recv->item);
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match(config_item)");
        exp_node->set_expect_status(not_met);
        return;
      }
      if ( (exp->has_boolValue != recv->has_boolValue) ||
           (exp->boolValue != recv->boolValue) )
      {
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Expected boolValue: %d, %d Received boolValue: %d, %d",
                       exp->has_boolValue, exp->boolValue,
                       recv->has_boolValue, recv->boolValue);
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match(boolValue)");
        exp_node->set_expect_status(not_met);
        return;
      }
      if ( (exp->has_intValue != recv->has_intValue) ||
           (exp->intValue != recv->intValue) )
      {
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Expected intValue: %d, %d Received intValue: %d, %d",
                       exp->has_intValue, exp->intValue,
                       recv->has_intValue, recv->intValue);
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match(intValue)");
        exp_node->set_expect_status(not_met);
        return;
      }
      /*
      if ( (exp->stringValue) &&
              (recv->stringValue))
      {
          if (strcmp(exp->stringValue, recv->stringValue) != 0)
          {
            RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected stringValue : %s", exp->stringValue);
            RIL_UTF_DEBUG("\n QMI_RIL_UTF: Recvd stringValue : %s", recv->stringValue);
            exp_node->set_expect_status(not_met);
            return;
          }
      }
      */
      if ( (exp->has_errorCause != recv->has_errorCause) ||
           (exp->errorCause != recv->errorCause) )
      {
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Expected errorCause: %d, %d Received errorCause: %d, %d",
                       exp->has_errorCause, exp->errorCause,
                       recv->has_errorCause, recv->errorCause);
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match(errorCause)");
        exp_node->set_expect_status(not_met);
        return;
      }
  }
  else
  {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Null payload received must be error");
  }
}

void ril_utf_test_t::validate_ims_last_call_fail_cause( ril_utf_node_base_t *exp_node,
                                                     struct ril_utf_msg_t *data)
{
  int i;
  exp_node->set_expect_status(met); // start with met and mark unmet if failure

  ims_CallFailCauseResponse *exp = (ims_CallFailCauseResponse*)exp_node->get_payload();
  ims_CallFailCauseResponse *recv = (ims_CallFailCauseResponse*)data->payload;

  if ( (exp->has_failcause != recv->has_failcause) ||
       (exp->failcause != recv->failcause) )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Expected fail cause %d", exp->failcause);
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Received fail cause %d", recv->failcause);
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match");
    exp_node->set_expect_status(not_met);
    return;
  }
#ifdef RIL_UTF_L_MASTER
  if ( (exp->networkErrorString.arg) &&
          (recv->networkErrorString.arg))
  {
      if (strcmp( ((char*)exp->networkErrorString.arg), ((char*)recv->networkErrorString.arg) ) != 0)
      {
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: For call %d", i);
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected networkerrorstring : %s", exp->networkErrorString);
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: Recvd networkerrorstring : %s", recv->networkErrorString);
        exp_node->set_expect_status(not_met);
        return;
      }
  }
#endif
}
void ril_utf_test_t::validate_ims_registration_state(ril_utf_node_base_t *exp_node,
                                                     struct ril_utf_msg_t *data)
{
  int validation_success = TRUE;
  exp_node->set_expect_status(met); // start with met and mark unmet if failure

  ims_Registration *exp = (ims_Registration*)exp_node->get_payload();
  ims_Registration *recv = (ims_Registration*)data->payload;

  do {
    if (data->ims_error == ims_Error_E_SUCCESS)
    {
      if ((exp->has_state != recv->has_state) || (exp->state != recv->state))
      {
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, state do not match");
        validation_success = FALSE;
      }
      if ((exp->has_errorCode != recv->has_errorCode) || (exp->errorCode != recv->errorCode))
      {
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, errorCode do not match");
        validation_success = FALSE;
      }
      if (exp->errorMessage.arg && recv->errorMessage.arg)
      {
        if (strcmp((char *)exp->errorMessage.arg, (char *)recv->errorMessage.arg) != 0)
        {
          RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, errorMessage do not match");
          validation_success = FALSE;
        }
      }
      if ((exp->has_radioTech != recv->has_radioTech) || (exp->radioTech != recv->radioTech))
      {
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, radioTech do not match");
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: radioTech exp %d/%d", exp->has_radioTech, exp->radioTech);
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: radioTech recv %d/%d", recv->has_radioTech, recv->radioTech);
        validation_success = FALSE;
      }
      if (exp->pAssociatedUris.arg && recv->pAssociatedUris.arg)
      {
        if (strcmp((char *)exp->pAssociatedUris.arg, (char *)recv->pAssociatedUris.arg) != 0)
        {
          RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, pAssociatedUris do not match");
          validation_success = FALSE;
        }
      }
    }
  } while (FALSE);

  if (validation_success)
  {
    exp_node->set_expect_status(met);
  }
  else
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match");
    exp_node->set_expect_status(not_met);
  }
}
void ril_utf_test_t::validate_ims_query_call_forward_status(ril_utf_node_base_t *exp_node,
                                                            struct ril_utf_msg_t *data)
{
  int validation_success = TRUE;
  ims_CallForwardInfoList *exp = (ims_CallForwardInfoList *)exp_node->get_payload();
  ims_CallForwardInfoList *recv = (ims_CallForwardInfoList *)data->payload;

  do {
    if (data->ims_error == ims_Error_E_SUCCESS) {
      ims_CallForwardInfoList_CallForwardInfo **exp_info =
          (ims_CallForwardInfoList_CallForwardInfo **)exp->info.arg;
      ims_CallForwardInfoList_CallForwardInfo **recv_info =
          (ims_CallForwardInfoList_CallForwardInfo **)recv->info.arg;

      while (*exp_info && *recv_info) {
        if ((*exp_info)->has_status != (*recv_info)->has_status ||
            (*exp_info)->status != (*recv_info)->status) {
          RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match for status");
          validation_success = FALSE;
          break;
        }
        if ((*exp_info)->has_reason != (*recv_info)->has_reason ||
            (*exp_info)->reason != (*recv_info)->reason) {
          RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match for reason");
          validation_success = FALSE;
          break;
        }
        if ((*exp_info)->has_service_class != (*recv_info)->has_service_class ||
            (*exp_info)->service_class != (*recv_info)->service_class) {
          RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match for service_class");
          validation_success = FALSE;
          break;
        }
        if ((*exp_info)->has_toa != (*recv_info)->has_toa ||
            (*exp_info)->toa != (*recv_info)->toa) {
          RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match for toa");
          validation_success = FALSE;
          break;
        }
        if ((*exp_info)->has_time_seconds != (*recv_info)->has_time_seconds ||
            (*exp_info)->time_seconds != (*recv_info)->time_seconds) {
          RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match for time_seconds");
          validation_success = FALSE;
          break;
        }
        if ((*exp_info)->has_callFwdTimerStart != (*recv_info)->has_callFwdTimerStart ||
            (*exp_info)->has_callFwdTimerEnd != (*recv_info)->has_callFwdTimerEnd) {
          RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match for callFwdTimer start/end");
          validation_success = FALSE;
          break;
        }

#ifdef RIL_UTF_L_MASTER
        if ((*exp_info)->has_callFwdTimerStart) {
          // validate timer
          if (((*exp_info)->callFwdTimerStart.has_year !=
               (*recv_info)->callFwdTimerStart.has_year) ||
              ((*exp_info)->callFwdTimerStart.year != (*recv_info)->callFwdTimerStart.year) ||
              ((*exp_info)->callFwdTimerStart.has_month !=
               (*recv_info)->callFwdTimerStart.has_month) ||
              ((*exp_info)->callFwdTimerStart.month != (*recv_info)->callFwdTimerStart.month) ||
              ((*exp_info)->callFwdTimerStart.has_day != (*recv_info)->callFwdTimerStart.has_day) ||
              ((*exp_info)->callFwdTimerStart.day != (*recv_info)->callFwdTimerStart.day) ||
              ((*exp_info)->callFwdTimerStart.has_hour !=
               (*recv_info)->callFwdTimerStart.has_hour) ||
              ((*exp_info)->callFwdTimerStart.hour != (*recv_info)->callFwdTimerStart.hour) ||
              ((*exp_info)->callFwdTimerStart.has_minute !=
               (*recv_info)->callFwdTimerStart.has_minute) ||
              ((*exp_info)->callFwdTimerStart.minute != (*recv_info)->callFwdTimerStart.minute) ||
              ((*exp_info)->callFwdTimerStart.has_second !=
               (*recv_info)->callFwdTimerStart.has_second) ||
              ((*exp_info)->callFwdTimerStart.second != (*recv_info)->callFwdTimerStart.second) ||
              ((*exp_info)->callFwdTimerStart.has_timezone !=
               (*recv_info)->callFwdTimerStart.has_timezone) ||
              ((*exp_info)->callFwdTimerStart.timezone !=
               (*recv_info)->callFwdTimerStart.timezone)) {
            RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match for callFwdTimerStart");
            validation_success = FALSE;
            break;
          }
        }
        if ((*exp_info)->has_callFwdTimerEnd) {
          // validate timer
          if (((*exp_info)->callFwdTimerEnd.has_year != (*recv_info)->callFwdTimerEnd.has_year) ||
              ((*exp_info)->callFwdTimerEnd.year != (*recv_info)->callFwdTimerEnd.year) ||
              ((*exp_info)->callFwdTimerEnd.has_month != (*recv_info)->callFwdTimerEnd.has_month) ||
              ((*exp_info)->callFwdTimerEnd.month != (*recv_info)->callFwdTimerEnd.month) ||
              ((*exp_info)->callFwdTimerEnd.has_day != (*recv_info)->callFwdTimerEnd.has_day) ||
              ((*exp_info)->callFwdTimerEnd.day != (*recv_info)->callFwdTimerEnd.day) ||
              ((*exp_info)->callFwdTimerEnd.has_hour != (*recv_info)->callFwdTimerEnd.has_hour) ||
              ((*exp_info)->callFwdTimerEnd.hour != (*recv_info)->callFwdTimerEnd.hour) ||
              ((*exp_info)->callFwdTimerEnd.has_minute !=
               (*recv_info)->callFwdTimerEnd.has_minute) ||
              ((*exp_info)->callFwdTimerEnd.minute != (*recv_info)->callFwdTimerEnd.minute) ||
              ((*exp_info)->callFwdTimerEnd.has_second !=
               (*recv_info)->callFwdTimerEnd.has_second) ||
              ((*exp_info)->callFwdTimerEnd.second != (*recv_info)->callFwdTimerEnd.second) ||
              ((*exp_info)->callFwdTimerEnd.has_timezone !=
               (*recv_info)->callFwdTimerEnd.has_timezone) ||
              ((*exp_info)->callFwdTimerEnd.timezone != (*recv_info)->callFwdTimerEnd.timezone)) {
            RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match for callFwdTimerEnd");
            validation_success = FALSE;
            break;
          }
        }
#endif
        exp_info++;
        recv_info++;
      }
    }
    if (data->ims_error != ims_Error_E_SUCCESS && exp->has_errorDetails) {
      if (exp->has_errorDetails != recv->has_errorDetails) {
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match for has_errorDetails");
        validation_success = FALSE;
        break;
      }
      if (exp->has_errorDetails) {
        if (!validate_ims_sip_error_info_util(exp->errorDetails, recv->errorDetails)) {
          RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match for errorDetails");
          validation_success = FALSE;
          break;
        }
      }
    }
  } while (FALSE);

  if (validation_success) {
    exp_node->set_expect_status(met);
  } else {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match");
    exp_node->set_expect_status(not_met);
  }
}

void ril_utf_test_t::validate_ims_query_call_waiting(ril_utf_node_base_t *exp_node,
                                                     struct ril_utf_msg_t *data) {
  //      exp_node->set_expect_status(met);
  int validation_success = TRUE;
  ims_CallWaitingInfo *exp = (ims_CallWaitingInfo *)exp_node->get_payload();
  ims_CallWaitingInfo *recv = (ims_CallWaitingInfo *)data->payload;

  do {
    if (data->ims_error == ims_Error_E_SUCCESS) {
      if ((exp->has_service_status != recv->has_service_status) ||
          (exp->service_status != recv->service_status)) {
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match for service_status");
        validation_success = FALSE;
        break;
      }
      if (exp->has_service_class != recv->has_service_class) {
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match for has_service_class");
        validation_success = FALSE;
        break;
      }
      if (exp->has_service_class) {
        if ((exp->service_class.has_service_Class != recv->service_class.has_service_Class) ||
            (exp->service_class.service_Class != recv->service_class.service_Class)) {
          RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match for service_Class");
          validation_success = FALSE;
          break;
        }
      }
    }
    if (data->ims_error != ims_Error_E_SUCCESS && exp->has_errorDetails) {
      if (exp->has_errorDetails != recv->has_errorDetails) {
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match for has_errorDetails");
        validation_success = FALSE;
        break;
      }
      if (exp->has_errorDetails) {
        if (!validate_ims_sip_error_info_util(exp->errorDetails, recv->errorDetails)) {
          RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match for errorDetails");
          validation_success = FALSE;
          break;
        }
      }
    }
  } while (FALSE);

  if (validation_success) {
    exp_node->set_expect_status(met);
  } else {
    exp_node->set_expect_status(not_met);
  }
}

void ril_utf_test_t::validate_ims_sub_config(ril_utf_node_base_t *exp_node,
                                             struct ril_utf_msg_t *data) {
  int validation_success = TRUE;
  ims_ImsSubConfig *exp = (ims_ImsSubConfig *)exp_node->get_payload();
  ims_ImsSubConfig *recv = (ims_ImsSubConfig *)data->payload;

  do {
    if (data->ims_error == ims_Error_E_SUCCESS) {
      if ((exp->has_simultStackCount != recv->has_simultStackCount) ||
          (exp->simultStackCount != recv->simultStackCount)) {
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, simultStackCount: exp: %d, %d, recv: %d, %d",
                      exp->has_simultStackCount, exp->simultStackCount,
                      recv->has_simultStackCount, recv->simultStackCount);
        validation_success = FALSE;
        break;
      }
      if (exp->imsStackEnabled.arg && recv->imsStackEnabled.arg) {
        uint64_t **exp_imsEnabled = (uint64_t **)exp->imsStackEnabled.arg;
        uint64_t **recv_imsEnabled = (uint64_t **)recv->imsStackEnabled.arg;
        while (exp_imsEnabled && recv_imsEnabled && *exp_imsEnabled && *recv_imsEnabled) {
          uint64_t *exp_Enabled = *exp_imsEnabled;
          uint64_t *recv_Enabled = *recv_imsEnabled;
          if (exp_Enabled && recv_Enabled) {
            if (*exp_Enabled != *recv_Enabled) {
              RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match for service_Class");
              validation_success = FALSE;
              break;
            }
          }
          exp_imsEnabled++;
          recv_imsEnabled++;
        }
      }
    }
  } while (FALSE);

  if (validation_success) {
    exp_node->set_expect_status(met);
  } else {
    exp_node->set_expect_status(not_met);
  }
}

void ril_utf_test_t::validate_ims_query_virtual_line_info(ril_utf_node_base_t *exp_node,
                                                          struct ril_utf_msg_t *data) {
  int validation_success = TRUE;
  ims_VirtualLineInfoResp *exp = (ims_VirtualLineInfoResp *)exp_node->get_payload();
  ims_VirtualLineInfoResp *recv = (ims_VirtualLineInfoResp *)data->payload;

  do {
    if (data->ims_error == ims_Error_E_SUCCESS) {
      if (strcmp(exp->msisdn, recv->msisdn) != 0) {
        RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error, msisdn: exp: %s, recv: %s",
                          exp->msisdn, recv->msisdn);
        validation_success = FALSE;
        break;
      }
      if (exp->lines_len != recv->lines_len) {
        RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error, lines_len: exp: %d, recv: %d",
                      exp->lines_len, recv->lines_len);
        validation_success = FALSE;
        break;
      }
      for (size_t i = 0; i < exp->lines_len && i < 10; i++) {
        if (strcmp(exp->lines[i], recv->lines[i]) != 0) {
          RIL_UTF_ESSENTIAL("\nQMI_RIL_UTL: Error, lines[%d]: exp: %s, recv: %s",
                        i, exp->lines[i], recv->lines[i]);
          validation_success = FALSE;
          break;
        }
      }
    }
  } while (FALSE);

  if (validation_success) {
    exp_node->set_expect_status(met);
  } else {
    exp_node->set_expect_status(not_met);
  }
}

void ril_utf_test_t::validate_ims_unsol_voice_info(
    ril_utf_node_base_t *exp_node, struct ril_utf_msg_t *data) {
  int validation_success = TRUE;
  ims_VoiceInfoData *exp = (ims_VoiceInfoData *)exp_node->get_payload();
  ims_VoiceInfoData *recv = (ims_VoiceInfoData *)data->payload;

  do {
    if (data->ims_error == ims_Error_E_SUCCESS) {
      if (exp->voiceInfo != recv->voiceInfo) {
        RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error, voiceInfo: exp: %d, recv: %d",
                      exp->voiceInfo, recv->voiceInfo);
        validation_success = FALSE;
        break;
      }
    }
  } while (FALSE);

  if (validation_success) {
    exp_node->set_expect_status(met);
  } else {
    exp_node->set_expect_status(not_met);
  }
}

void ril_utf_test_t::validate_ims_unsol_multi_identity_registration_status(
    ril_utf_node_base_t *exp_node, struct ril_utf_msg_t *data) {
  int validation_success = TRUE;
  ims_MultiIdentityStatus *exp = (ims_MultiIdentityStatus *)exp_node->get_payload();
  ims_MultiIdentityStatus *recv = (ims_MultiIdentityStatus *)data->payload;

  do {
    if (data->ims_error == ims_Error_E_SUCCESS) {
      if (exp->lineInfo_len != recv->lineInfo_len) {
        RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error, lineInfo_len: exp: %d, recv: %d",
                      exp->lineInfo_len, recv->lineInfo_len);
        validation_success = FALSE;
        break;
      }
      for (uint32_t i = 0; i < recv->lineInfo_len && i < 10; i++) {
        if (strcmp(exp->lineInfo[i].msisdn, recv->lineInfo[i].msisdn) != 0) {
          RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error, lineInfo[%d].msisdn: exp: %s, recv: %s",
              i, exp->lineInfo[i].msisdn, recv->lineInfo[i].msisdn);
          validation_success = FALSE;
          break;
        }
        if (exp->lineInfo[i].registrationStatus != recv->lineInfo[i].registrationStatus) {
          RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error, lineInfo[%d].registrationStatus: exp: %d, recv: %d",
              i, exp->lineInfo[i].registrationStatus, recv->lineInfo[i].registrationStatus);
          validation_success = FALSE;
          break;
        }
        if (exp->lineInfo[i].lineType != recv->lineInfo[i].lineType) {
          RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error, lineInfo[%d].lineType: exp: %d, recv: %d",
              i, exp->lineInfo[i].lineType, recv->lineInfo[i].lineType);
          validation_success = FALSE;
          break;
        }
      }
    }
  } while (FALSE);

  if (validation_success) {
    exp_node->set_expect_status(met);
  } else {
    exp_node->set_expect_status(not_met);
  }
}

/*========================================================
  FUNCTION: validate_ims_request_query_clip_resp
    - Verifies the contents of ims_MsgId_REQUEST_QUERY_CLIP
      message that is received match what was expected.

  Critical section access:
   - None
========================================================*/
void ril_utf_test_t::validate_ims_request_query_clip_resp
(
   ril_utf_node_base_t *exp_node,
   struct ril_utf_msg_t *data
)
{
  int i;
  exp_node->set_expect_status(met); // start with met and mark unmet if failure

  if (data->ims_error != exp_node->get_ims_error_value())
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, error value do not match");
    exp_node->set_expect_status(not_met);
    return;
  }
  if (data->ims_error == ims_Error_E_SUCCESS)
  {
    ims_ClipProvisionStatus *exp = (ims_ClipProvisionStatus*)exp_node->get_payload();
    ims_ClipProvisionStatus *recv = (ims_ClipProvisionStatus*)data->payload;

    if ( (exp->has_clip_status != recv->has_clip_status) ||
         (exp->clip_status != recv->clip_status) )
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match for clip_status");
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Expected clip_status %d, %d", exp->has_clip_status,
                    exp->clip_status);
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Received clip_status %d, %d", recv->has_clip_status,
                    recv->clip_status);
      exp_node->set_expect_status(not_met);
      return;
    }
  }
}

/*========================================================
  FUNCTION: validate_ims_set_call_forward_status_resp
    - Verifies the contents of ims_MsgId_REQUEST_SET_CALL_FORWARD_STATUS
      message that is received match what was expected.

  Critical section access:
   - None
========================================================*/
void ril_utf_test_t::validate_ims_set_call_forward_status_resp
(
   ril_utf_node_base_t *exp_node,
   struct ril_utf_msg_t *data
)
{
  int validation_success = TRUE;

  ims_CallForwardStatusInfo *exp = (ims_CallForwardStatusInfo *) exp_node->get_payload();
  ims_CallForwardStatusInfo *recv = (ims_CallForwardStatusInfo *) data->payload;

  do {
    if (data->ims_error != exp_node->get_ims_error_value()) {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, error value do not match");
      validation_success = FALSE;
      break;
    }
    if (exp && recv) {
      if (data->ims_error != ims_Error_E_SUCCESS && exp->has_errorDetails) {
        if (exp->has_errorDetails != recv->has_errorDetails) {
          RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match for has_errorDetails");
          validation_success = FALSE;
          break;
        }
        if (exp->has_errorDetails) {
          if (!validate_ims_sip_error_info_util(exp->errorDetails, recv->errorDetails)) {
            RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match for errorDetails");
            validation_success = FALSE;
            break;
          }
        }
      }
      if (data->ims_error == ims_Error_E_SUCCESS) {
        if (exp->status_len != recv->status_len) {
          RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match for status_len");
          validation_success = FALSE;
          break;
        }
        if (exp->status_len) {
          uint32_t i = 0;;
          while(i < exp->status_len) {
            if (exp->status[i].reason != recv->status[i].reason) {
              RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match for status[%d].reason", i);
              validation_success = FALSE;
              break;
            }
            if (exp->status[i].status != recv->status[i].status) {
              RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match for status[%d].status", i);
              validation_success = FALSE;
              break;
            }
            if (!validate_ims_sip_error_info_util(exp->status[i].errorDetails, recv->status[i].errorDetails)) {
              RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match for status[%d].errorDetails", i);
              validation_success = FALSE;
              break;
            }
            i++;
          }
          break;
        }
      }
    }
  } while (FALSE);
  if (validation_success) {
    exp_node->set_expect_status(met);
  } else {
    exp_node->set_expect_status(not_met);
  }
}

/*========================================================
  FUNCTION: validate_ims_unsol_refresh_conf_info
    - Verifies the contents of ims_MsgId_UNSOL_REFRESH_CONF_INFO
      message that is received match what was expected.

  Critical section access:
   - None
========================================================*/
void ril_utf_test_t::validate_ims_unsol_refresh_conf_info
(
   ril_utf_node_base_t *exp_node,
   struct ril_utf_msg_t *data
)
{
  int validation_success = TRUE;
  ims_ConfInfo *exp = (ims_ConfInfo *) exp_node->get_payload();
  ims_ConfInfo *recv = (ims_ConfInfo *) data->payload;

  do
  {
    if (!exp || !recv) {
      break;
    }
    qcril_binary_data_type *exp_uri = (qcril_binary_data_type *)exp->conf_info_uri.arg;
    qcril_binary_data_type *recv_uri = (qcril_binary_data_type *)recv->conf_info_uri.arg;
    if (!exp_uri && !recv_uri) {
      break;
    }
    if (exp_uri->len != recv_uri->len) {
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: len exp_uri = %d and recv_uri = %d\n",
          exp_uri->len, recv_uri->len);
      validation_success = FALSE;
      break;
    }
    if (exp_uri->len && exp_uri->data && recv_uri->data) {
      if ((memcmp(exp_uri->data, recv_uri->data, exp_uri->len) != 0)) {
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: data exp and recv");
        validation_success = FALSE;
        break;
      }
    }
    if ((exp->has_confCallState != recv->has_confCallState) ||
        (recv->has_confCallState &&
         (exp->confCallState != recv->confCallState))) {
      validation_success = FALSE;
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: confCallState exp->= %d, %d and recv = %d, %d\n",
          exp->has_confCallState, exp->confCallState,
          recv->has_confCallState, recv->confCallState);
      break;
    }
  } while (0);

  if (validation_success) {
      exp_node->set_expect_status(met);
  } else {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match");
      exp_node->set_expect_status(not_met);
  }
}

/*========================================================
  FUNCTION: validate_ims_unsol_refresh_vice_info
    - Verifies the contents of ims_MsgId_UNSOL_REFRESH_VICE_INFO
      message that is received match what was expected.

  Critical section access:
   - None
========================================================*/
void ril_utf_test_t::validate_ims_unsol_refresh_vice_info
(
   ril_utf_node_base_t *exp_node,
   struct ril_utf_msg_t *data
)
{
  int validation_success = TRUE;
  ims_ViceInfo *exp = (ims_ViceInfo *) exp_node->get_payload();
  ims_ViceInfo *recv = (ims_ViceInfo *) data->payload;

  do
  {
    if (!exp || !recv) {
      break;
    }
    qcril_binary_data_type *exp_uri = (qcril_binary_data_type *)exp->vice_info_uri.arg;
    qcril_binary_data_type *recv_uri = (qcril_binary_data_type *)recv->vice_info_uri.arg;
    if (!exp_uri && !recv_uri) {
      break;
    }
    if (exp_uri->len != recv_uri->len) {
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: len exp_uri = %d and recv_uri = %d\n",
          exp_uri->len, recv_uri->len);
      validation_success = FALSE;
      break;
    }
    if (exp_uri->len && exp_uri->data && recv_uri->data) {
      if ((memcmp(exp_uri->data, recv_uri->data, exp_uri->len) != 0)) {
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: data exp and recv");
        validation_success = FALSE;
        break;
      }
    }
  } while (0);

  if (validation_success) {
      exp_node->set_expect_status(met);
  } else {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match");
      exp_node->set_expect_status(not_met);
  }
}

void ril_utf_test_t::validate_imsa_geo_loc( ril_utf_node_base_t *exp_node,
                                                     struct ril_utf_msg_t *data)
{
      exp_node->set_expect_status(not_met);
  int i;
  int validation_success = TRUE;
  ims_GeoLocationInfo *exp = (ims_GeoLocationInfo*)exp_node->get_payload();
  ims_GeoLocationInfo *recv = (ims_GeoLocationInfo*)data->payload;

  do
  {
      if ((exp->has_lat != recv->has_lat) ||
              (exp->lat != recv->lat) ||
              (exp->has_lon != recv->has_lon) ||
              (exp->lon != recv->lon))
      {
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, lat/lon don't match\n");
        validation_success = FALSE;
        break;
      }

      if (exp->has_addressInfo)
      {
          if (!recv->has_addressInfo)
          {
              validation_success = FALSE;
              break;
          }
          if (!exp->addressInfo.city.arg != !recv->addressInfo.city.arg ||
                  !exp->addressInfo.postalCode.arg != !recv->addressInfo.postalCode.arg ||
                  !exp->addressInfo.country.arg != !recv->addressInfo.country.arg ||
                  !exp->addressInfo.countryCode.arg != !recv->addressInfo.countryCode.arg )
          {
              validation_success = FALSE;
              break;
          }
      }
  } while (FALSE);

  if (validation_success)
  {
      exp_node->set_expect_status(met);
  }
  else
  {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match");
      exp_node->set_expect_status(not_met);
  }
}
  void ril_utf_test_t::validate_imsa_vowifi_call_quality
(
 ril_utf_node_base_t *exp_node,
 struct ril_utf_msg_t *data
 )
{
  int i;
  exp_node->set_expect_status(met); // start with met and mark unmet if failure

  ims_VoWiFiCallQuality *exp = (ims_VoWiFiCallQuality *) exp_node->get_payload();
  ims_VoWiFiCallQuality *recv = (ims_VoWiFiCallQuality *) data->payload;

  if (exp && recv)
  {
    if ((exp->has_quality != recv->has_quality) ||
        (exp->quality != recv->quality))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: quality exp = %d,%d and recv = %d,%d\n",
          exp->has_quality, exp->quality, recv->has_quality, recv->quality);
      exp_node->set_expect_status(not_met);
      return;
    }
  }
}

void ril_utf_test_t::validate_unsol_participant_status_info
(
  ril_utf_node_base_t *exp_node,
  struct ril_utf_msg_t *data
)
{
  int i;
  exp_node->set_expect_status(met); // start with met and mark unmet if failure

  ims_ParticipantStatusInfo *exp = (ims_ParticipantStatusInfo *) exp_node->get_payload();
  ims_ParticipantStatusInfo *recv = (ims_ParticipantStatusInfo *) data->payload;

  if (exp && recv)
  {
    if ((exp->has_callId != recv->has_callId) ||
        (exp->callId != recv->callId))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, callId don't match\n");
      RIL_UTF_DEBUG("\n Expected: has_callId = %d, callId = %d\n", exp->has_callId, exp->callId);
      RIL_UTF_DEBUG("\n Received: has_callId = %d, callId = %d\n", recv->has_callId, recv->callId);
      exp_node->set_expect_status(not_met);
    }
    if ((exp->has_operation != recv->has_operation) ||
        (exp->operation != recv->operation))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, operation don't match\n");
      RIL_UTF_DEBUG("\n Expected: has_operation = %d, operation = %d\n", exp->has_operation, exp->operation);
      RIL_UTF_DEBUG("\n Received: has_operation = %d, operation = %d\n", recv->has_operation, recv->operation);
      exp_node->set_expect_status(not_met);
    }
    if ((exp->has_sipStatus != recv->has_sipStatus) ||
        (exp->sipStatus != recv->sipStatus))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, sipStatus don't match\n");
      RIL_UTF_DEBUG("\n Expected: has_sipStatus = %d, sipStatus = %d\n", exp->has_sipStatus, exp->sipStatus);
      RIL_UTF_DEBUG("\n Received: has_sipStatus = %d, sipStatus = %d\n", recv->has_sipStatus, recv->sipStatus);
      exp_node->set_expect_status(not_met);
    }
    if (exp->participantUri.arg && recv->participantUri.arg)
    {
      if (strcmp((char *)exp->participantUri.arg, (char *)recv->participantUri.arg) != 0)
      {
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, participantUri don't match\n");
        RIL_UTF_DEBUG("\n Expected: participantUri.arg = %s\n", exp->participantUri.arg);
        RIL_UTF_DEBUG("\n Received: participantUri.arg = %s\n", recv->participantUri.arg);
        exp_node->set_expect_status(not_met);
      }
    }
    if ((exp->has_isEct != recv->has_isEct) ||
        (exp->isEct != recv->isEct))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, isEct don't match\n");
      RIL_UTF_DEBUG("\n Expected: has_isEct = %d, isEct = %d\n", exp->has_isEct, exp->isEct);
      RIL_UTF_DEBUG("\n Received: has_isEct = %d, isEct = %d\n", recv->has_isEct, recv->isEct);
      exp_node->set_expect_status(not_met);
    }
  }
}
void ril_utf_test_t::validate_ims_unsol_modify_call( ril_utf_node_base_t *exp_node,
    struct ril_utf_msg_t *data)
{
  int i;
  int validation_success = TRUE;
  ims_CallModify *exp = (ims_CallModify*)exp_node->get_payload();
  ims_CallModify *recv = (ims_CallModify*)data->payload;

  do
  {
    if ((exp->has_callIndex != recv->has_callIndex) ||
        (exp->callIndex != recv->callIndex))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, call index doesnot match\n");
      validation_success = FALSE;
      break;
    }
    if ((exp->has_error != recv->has_error) || (exp->error != recv->error))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, error doesnot match\n");
      validation_success = FALSE;
      break;
    }

    if (exp->has_callDetails != recv->has_callDetails)
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, has_callDetails doesnot match\n");
      validation_success = FALSE;
      break;
    }
    if (exp->has_callDetails)
    {
      if (exp->callDetails.has_callType != recv->callDetails.has_callType ||
          exp->callDetails.callType != recv->callDetails.callType)
      {
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, callType doesnot match\n");
        validation_success = FALSE;
        break;
      }
      if (exp->callDetails.has_callDomain != recv->callDetails.has_callDomain ||
          exp->callDetails.callDomain != recv->callDetails.callDomain)
      {
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, callType doesnot match\n");
        validation_success = FALSE;
        break;
      }
    }
  } while (FALSE);

  if (validation_success)
  {
    exp_node->set_expect_status(met);
  }
  else
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match");
    exp_node->set_expect_status(not_met);
  }
}
void ril_utf_test_t::validate_ims_unsol_supp_svc_notification( ril_utf_node_base_t *exp_node,
                                                     struct ril_utf_msg_t *data)
{
  int i;
  exp_node->set_expect_status(met); // start with met and mark unmet if failure

  ims_SuppSvcNotification *exp = (ims_SuppSvcNotification*)exp_node->get_payload();
  ims_SuppSvcNotification *recv = (ims_SuppSvcNotification*)data->payload;

  if ( (exp->has_notificationType != recv->has_notificationType) ||
       (exp->notificationType != recv->notificationType) )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Expected notificationtype %d, %d",
                  exp->has_notificationType, exp->notificationType);
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Received notificationtype %d, %d",
                  recv->has_notificationType, recv->notificationType);
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match");
    exp_node->set_expect_status(not_met);
    return;
  }
  if ( (exp->has_code != recv->has_code) ||
       (exp->code != recv->code) )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Expected code %d, %d", exp->has_code, exp->code);
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Received code %d, %d", recv->has_code, recv->code);
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match");
    exp_node->set_expect_status(not_met);
    return;
  }
  if ( (exp->has_index != recv->has_index) ||
       (exp->index != recv->index) )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Expected index %d, %d", exp->has_index, exp->index);
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Received index %d, %d", recv->has_index, recv->index);
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match");
    exp_node->set_expect_status(not_met);
    return;
  }
  if ( (exp->has_type != recv->has_type) ||
       (exp->type != recv->type) )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Expected type %d, %d", exp->has_type, exp->type);
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Received type %d, %d", recv->has_type, recv->type);
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match");
    exp_node->set_expect_status(not_met);
    return;
  }
  if ( exp->number.arg )
  {
      if ( !(recv->number.arg) ||
           (strcmp((char*)exp->number.arg, (char*)recv->number.arg) != 0) )
      {
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: For call %d", i);
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected number : %s", (char*)exp->number.arg);
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: Recvd number : %s", (char*)recv->number.arg);
        exp_node->set_expect_status(not_met);
        return;
      }
  }
  if ( (exp->has_connId != recv->has_connId) ||
       (exp->connId != recv->connId) )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Expected connid %d, %d", exp->has_connId, exp->connId);
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Received connid %d, %d", recv->has_connId, recv->connId);
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match");
    exp_node->set_expect_status(not_met);
    return;
  }
  if ( exp->history_info.arg )
  {
      if ( !(recv->history_info.arg) ||
           (strcmp((char*)exp->history_info.arg, (char*)recv->history_info.arg) != 0) )
      {
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: For call %d", i);
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected history_info : %s", (char*)exp->history_info.arg);
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: Recvd history_info : %s", (char*)recv->history_info.arg);
        exp_node->set_expect_status(not_met);
        return;
      }
  }
  if ( (exp->has_hold_tone != recv->has_hold_tone) ||
       (exp->hold_tone != recv->hold_tone) )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Expected hold_tone %d, %d", exp->has_hold_tone, exp->hold_tone);
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Received hold_tone %d, %d", recv->has_hold_tone, recv->hold_tone);
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match");
    exp_node->set_expect_status(not_met);
    return;
  }
}

void ril_utf_test_t::validate_ims_unsol_response_handover( ril_utf_node_base_t *exp_node,
                                                     struct ril_utf_msg_t *data)
{
  int i;
  exp_node->set_expect_status(met); // start with met and mark unmet if failure

  ims_Handover *exp = (ims_Handover*)exp_node->get_payload();
  ims_Handover *recv = (ims_Handover*)data->payload;

  if ( (exp->has_type != recv->has_type) ||
       (exp->type != recv->type) )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Expected has_type %d; type %d", exp->has_type, exp->type);
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Received has_type %d; type %d", recv->has_type, recv->type);
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match");
    exp_node->set_expect_status(not_met);
    return;
  }
  if ( (exp->has_srcTech != recv->has_srcTech) ||
       (exp->srcTech != recv->srcTech) )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Expected has_srcTech %d; srcTech %d",
            exp->has_srcTech, exp->srcTech);
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Received has_srcTech %d; srcTech %d",
            recv->has_srcTech, recv->srcTech);
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match");
    exp_node->set_expect_status(not_met);
    return;
  }
  if ( (exp->has_targetTech != recv->has_targetTech) ||
       (exp->targetTech != recv->targetTech) )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Expected has_targetTech %d; targetTech %d",
            exp->has_targetTech, exp->targetTech);
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Received has_targetTech %d; targetTech %d",
            recv->has_targetTech, recv->targetTech);
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match");
    exp_node->set_expect_status(not_met);
    return;
  }
}

/*========================================================
  FUNCTION: validate_ims_request_supp_svc_status_resp
    - Verifies the contents of ims_MsgId_REQUEST_SUPP_SVC_STATUS
      message that is received match what was expected.

  Critical section access:
   - None
========================================================*/
void ril_utf_test_t::validate_ims_request_supp_svc_status_resp
(
   ril_utf_node_base_t *exp_node,
   struct ril_utf_msg_t *data
)
{
  int i;
  exp_node->set_expect_status(met); // start with met and mark unmet if failure

  ims_SuppSvcResponse *exp = (ims_SuppSvcResponse*)exp_node->get_payload();
  ims_SuppSvcResponse *recv = (ims_SuppSvcResponse*)data->payload;

  if (data->ims_error != exp_node->get_ims_error_value())
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, error value do not match");
    exp_node->set_expect_status(not_met);
    return;
  }
  if (exp && recv)
  {
    if (recv->has_status &&
        ((exp->has_status != recv->has_status) || (exp->status != recv->status)))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected status = %d, %d, but Received = %d, %d",
              exp->has_status, exp->status, recv->has_status, recv->status);
      exp_node->set_expect_status(not_met);
      return;
    }

    if (recv->has_provisionStatus && ((exp->has_provisionStatus != recv->has_provisionStatus) ||
                                      (exp->provisionStatus != recv->provisionStatus))) {
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected provisionStatus = %d, %d, but Received = %d, %d",
                    exp->has_provisionStatus, exp->provisionStatus, recv->has_provisionStatus,
                    recv->provisionStatus);
      exp_node->set_expect_status(not_met);
      return;
    }

    if (recv->has_facilityType && ((exp->has_facilityType != recv->has_facilityType) ||
                                   (exp->facilityType != recv->facilityType)))
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected facilitytype = %d, %d, but Received = %d, %d",
              exp->has_facilityType, exp->facilityType, recv->has_facilityType, recv->facilityType);
      exp_node->set_expect_status(not_met);
      return;
    }
    if (exp->cbNumListType.arg && recv->cbNumListType.arg)
    {
      ims_CbNumListType** exp_cbnlt = (ims_CbNumListType**)(exp->cbNumListType.arg);
      ims_CbNumListType** recv_cbnlt = (ims_CbNumListType**)(recv->cbNumListType.arg);

      while ((exp_cbnlt && recv_cbnlt) && (*exp_cbnlt && *recv_cbnlt))
      {
        if ((*recv_cbnlt)->has_serviceClass &&
            (((*exp_cbnlt)->has_serviceClass != (*recv_cbnlt)->has_serviceClass) ||
             (*recv_cbnlt)->serviceClass.has_service_Class &&
                 (((*exp_cbnlt)->serviceClass.has_service_Class !=
                   (*recv_cbnlt)->serviceClass.has_service_Class) ||
                  ((*exp_cbnlt)->serviceClass.service_Class !=
                   (*recv_cbnlt)->serviceClass.service_Class))))
        {
          RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected service_Class = %d, %d, %d, but Received = %d, %d, %d",
              (*exp_cbnlt)->has_serviceClass,
              (*exp_cbnlt)->serviceClass.has_service_Class,
              (*exp_cbnlt)->serviceClass.service_Class,
              (*recv_cbnlt)->has_serviceClass,
              (*recv_cbnlt)->serviceClass.has_service_Class,
              (*recv_cbnlt)->serviceClass.service_Class);
          exp_node->set_expect_status(not_met);
          return;
        }
        if ((*exp_cbnlt)->cb_num_list.arg && (*recv_cbnlt)->cb_num_list.arg)
        {
          ims_CbNumList **exp_cbnl = (ims_CbNumList **)((*exp_cbnlt)->cb_num_list.arg);
          ims_CbNumList **recv_cbnl = (ims_CbNumList **)((*recv_cbnlt)->cb_num_list.arg);

          while ((exp_cbnl && recv_cbnl) && (*exp_cbnl && *recv_cbnl))
          {
            if ((*recv_cbnl)->has_status &&
                (((*exp_cbnl)->has_status != (*recv_cbnl)->has_status) ||
                 ((*exp_cbnl)->status != (*recv_cbnl)->status)))
            {
              RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected status = %d, %d, but Received = %d, %d",
                  (*exp_cbnl)->has_status, (*exp_cbnl)->status,
                  (*recv_cbnl)->has_status, (*recv_cbnl)->status);
              exp_node->set_expect_status(not_met);
              return;
            }
            if ((*exp_cbnl)->number.arg && (*recv_cbnl)->number.arg)
            {
              if (strcmp((char *)(*exp_cbnl)->number.arg, (char *)(*recv_cbnl)->number.arg) != 0)
              {
                RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected number = %s, but Received = %s",
                    (char *)(*exp_cbnl)->number.arg,
                    (char *)(*recv_cbnl)->number.arg);
                exp_node->set_expect_status(not_met);
                return;
              }
            }
            exp_cbnl++;
            recv_cbnl++;
          }
        }

        exp_cbnlt++;
        recv_cbnlt++;
      }
    }

    if (exp->has_errorDetails == recv->has_errorDetails)
    {
      if(!validate_ims_sip_error_info_util(exp->errorDetails, recv->errorDetails)){
        exp_node->set_expect_status(not_met);
        return;
      }
    } else {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Expected has_errorDetails: %d Received has_errorDetails: %d",
                     exp->has_errorDetails,
                     recv->has_errorDetails);
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error, messages do not match(has_errorDetails)");
      exp_node->set_expect_status(not_met);
      return;
    }
  }
}

void ril_utf_test_t::validate_unsol_incoming_ims_sms(ril_utf_node_base_t *exp_node,
        struct ril_utf_msg_t *data)
{
    auto exp = static_cast<utf_ims_mt_sms_t*>(exp_node->get_payload());
    auto recv = static_cast<utf_ims_mt_sms_t*>(data->payload);

    if (exp->tech != recv->tech)
    {
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Expected radio tech %d", exp->tech);
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Received radio tech %d", recv->tech);
        exp_node->set_expect_status(not_met);
        return;
    }
    if (exp->size != recv->size)
    {
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Expected pdu size %d", exp->size);
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Received pdu size %d", recv->size);
        exp_node->set_expect_status(not_met);
        return;
    }

    if (memcmp(exp->pdu, recv->pdu, recv->size))
    {
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: pdu mismatch");
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: PDU expected:");
        print_msg_in_hex(exp->pdu, exp->size);
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: PDU received:");
        print_msg_in_hex(recv->pdu, recv->size);
        exp_node->set_expect_status(not_met);
        return;
    }
    exp_node->set_expect_status(met);
}

#ifndef UTF_TARGET_BUILD
void ril_utf_test_t::validate_ims_unsol_mwi( ril_utf_node_base_t *exp_node,
                                                     struct ril_utf_msg_t *data)
{
  int validation_success = TRUE;
  ims_Mwi *exp = (ims_Mwi *) exp_node->get_payload();
  ims_Mwi *recv = (ims_Mwi *) data->payload;

  do
  {
    ims_MwiMessageSummary **exp_mwimsgsummary  = (ims_MwiMessageSummary **)exp->mwiMsgSummary.arg;
    ims_MwiMessageSummary **recv_mwimsgsummary = (ims_MwiMessageSummary **)recv->mwiMsgSummary.arg;

    while (exp_mwimsgsummary && *exp_mwimsgsummary && recv_mwimsgsummary && *recv_mwimsgsummary) {
      if ((*exp_mwimsgsummary)->has_messageType != (*recv_mwimsgsummary)->has_messageType ||
          ((*recv_mwimsgsummary)->has_messageType &&
          ((*exp_mwimsgsummary)->messageType != (*recv_mwimsgsummary)->messageType))) {
        validation_success = FALSE;
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: messageType exp = %d, %d and recv = %d, %d\n",
            (*exp_mwimsgsummary)->has_messageType, (*exp_mwimsgsummary)->messageType,
            (*recv_mwimsgsummary)->has_messageType, (*recv_mwimsgsummary)->messageType);
        break;
      }
      if ((*exp_mwimsgsummary)->has_newMessage != (*recv_mwimsgsummary)->has_newMessage ||
          ((*recv_mwimsgsummary)->has_newMessage &&
          ((*exp_mwimsgsummary)->newMessage != (*recv_mwimsgsummary)->newMessage))) {
        validation_success = FALSE;
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: newMessage exp = %d, %d and recv = %d, %d\n",
            (*exp_mwimsgsummary)->has_newMessage, (*exp_mwimsgsummary)->newMessage,
            (*recv_mwimsgsummary)->has_newMessage, (*recv_mwimsgsummary)->newMessage);
        break;
      }
      if ((*exp_mwimsgsummary)->has_oldMessage != (*recv_mwimsgsummary)->has_oldMessage ||
          ((*recv_mwimsgsummary)->has_oldMessage &&
          ((*exp_mwimsgsummary)->oldMessage != (*recv_mwimsgsummary)->oldMessage))) {
        validation_success = FALSE;
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: oldMessage exp = %d, %d and recv = %d, %d\n",
            (*exp_mwimsgsummary)->has_oldMessage, (*exp_mwimsgsummary)->oldMessage,
            (*recv_mwimsgsummary)->has_oldMessage, (*recv_mwimsgsummary)->oldMessage);
        break;
      }
      if ((*exp_mwimsgsummary)->has_newUrgent != (*recv_mwimsgsummary)->has_newUrgent ||
          ((*recv_mwimsgsummary)->has_newUrgent &&
          ((*exp_mwimsgsummary)->newUrgent != (*recv_mwimsgsummary)->newUrgent))) {
        validation_success = FALSE;
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: newUrgent exp = %d, %d and recv = %d, %d\n",
            (*exp_mwimsgsummary)->has_newUrgent, (*exp_mwimsgsummary)->newUrgent,
            (*recv_mwimsgsummary)->has_newUrgent, (*recv_mwimsgsummary)->newUrgent);
        break;
      }
      if ((*exp_mwimsgsummary)->has_oldUrgent != (*recv_mwimsgsummary)->has_oldUrgent ||
          ((*recv_mwimsgsummary)->has_oldUrgent &&
          ((*exp_mwimsgsummary)->oldUrgent != (*recv_mwimsgsummary)->oldUrgent))) {
        validation_success = FALSE;
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: oldUrgent exp = %d, %d and recv = %d, %d\n",
            (*exp_mwimsgsummary)->has_oldUrgent, (*exp_mwimsgsummary)->oldUrgent,
            (*recv_mwimsgsummary)->has_oldUrgent, (*recv_mwimsgsummary)->oldUrgent);
        break;
      }
      exp_mwimsgsummary++;
      recv_mwimsgsummary++;
    }

    if (!validation_success) {
      break;
    }

    char *exp_ueAddress = (char *)exp->ueAddress.arg;
    char *recv_ueAddress = (char *)recv->ueAddress.arg;

    if (exp_ueAddress != NULL && recv_ueAddress != NULL &&
        (strcmp(exp_ueAddress, recv_ueAddress) != 0)) {
      validation_success = FALSE;
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: ueAddress exp = %s and recv = %s\n",
          exp_ueAddress, recv_ueAddress);
      break;
    }

    ims_MwiMessageDetails **exp_mwiMsgDetail  = (ims_MwiMessageDetails **)exp->mwiMsgDetail.arg;
    ims_MwiMessageDetails **recv_mwiMsgDetail  = (ims_MwiMessageDetails **)recv->mwiMsgDetail.arg;

    while (exp_mwiMsgDetail && *exp_mwiMsgDetail && recv_mwiMsgDetail && *recv_mwiMsgDetail) {
      char *exp_string = (char *)(*exp_mwiMsgDetail)->toAddress.arg;
      char *recv_string = (char *)(*recv_mwiMsgDetail)->toAddress.arg;
      if (exp_string != NULL && recv_string != NULL &&
          (strcmp(exp_string, recv_string) != 0)) {
        validation_success = FALSE;
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: toAddress exp = %s and recv = %s\n",
            exp_string, recv_string);
        break;
      }
      exp_string = (char *)(*exp_mwiMsgDetail)->fromAddress.arg;
      recv_string = (char *)(*recv_mwiMsgDetail)->fromAddress.arg;
      if (exp_string != NULL && recv_string != NULL &&
          (strcmp(exp_string, recv_string) != 0)) {
        validation_success = FALSE;
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: fromAddress exp = %s and recv = %s\n",
            exp_string, recv_string);
        break;
      }
      exp_string = (char *)(*exp_mwiMsgDetail)->subject.arg;
      recv_string = (char *)(*recv_mwiMsgDetail)->subject.arg;
      if (exp_string != NULL && recv_string != NULL &&
          (strcmp(exp_string, recv_string) != 0)) {
        validation_success = FALSE;
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: subject exp = %s and recv = %s\n",
            exp_string, recv_string);
        break;
      }
      exp_string = (char *)(*exp_mwiMsgDetail)->date.arg;
      recv_string = (char *)(*recv_mwiMsgDetail)->date.arg;
      if (exp_string != NULL && recv_string != NULL &&
          (strcmp(exp_string, recv_string) != 0)) {
        validation_success = FALSE;
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: date exp = %s and recv = %s\n",
            exp_string, recv_string);
        break;
      }
      if (((*exp_mwiMsgDetail)->has_priority != (*recv_mwiMsgDetail)->has_priority) ||
          ((*recv_mwiMsgDetail)->has_priority &&
          ((*exp_mwiMsgDetail)->priority != (*recv_mwiMsgDetail)->priority))) {
        validation_success = FALSE;
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: priority exp = %d, %d and recv = %d, %d\n",
            (*exp_mwiMsgDetail)->has_priority, (*exp_mwiMsgDetail)->priority,
            (*recv_mwiMsgDetail)->has_priority, (*recv_mwiMsgDetail)->priority);
        break;
      }
      exp_string = (char *)(*exp_mwiMsgDetail)->messageId.arg;
      recv_string = (char *)(*recv_mwiMsgDetail)->messageId.arg;
      if (exp_string != NULL && recv_string != NULL &&
          (strcmp(exp_string, recv_string) != 0)) {
        validation_success = FALSE;
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: messageId exp = %s and recv = %s\n",
            exp_string, recv_string);
        break;
      }
      if (((*exp_mwiMsgDetail)->has_messageType != (*recv_mwiMsgDetail)->has_messageType) ||
          ((*recv_mwiMsgDetail)->has_messageType &&
          ((*exp_mwiMsgDetail)->messageType != (*recv_mwiMsgDetail)->messageType))) {
        validation_success = FALSE;
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: messageType exp = %d, %d and recv = %d, %d\n",
            (*exp_mwiMsgDetail)->has_messageType, (*exp_mwiMsgDetail)->messageType,
            (*recv_mwiMsgDetail)->has_messageType, (*recv_mwiMsgDetail)->messageType);
        break;
      }
      exp_mwiMsgDetail++;
      recv_mwiMsgDetail++;
    }
    if (!validation_success) {
      break;
    }
  } while (0);

  if (validation_success) {
      exp_node->set_expect_status(met);
  } else {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match");
      exp_node->set_expect_status(not_met);
  }
}
#endif
void ril_utf_test_t::validate_ims_unsol_srv_status_update( ril_utf_node_base_t *exp_node,
                                                     struct ril_utf_msg_t *data)
{
  int info_idx = 0;
  int validation_success = TRUE;
  ims_SrvStatusList *exp_list = (ims_SrvStatusList *) exp_node->get_payload();
  ims_SrvStatusList *recv_list = (ims_SrvStatusList *) data->payload;

  exp_node->set_expect_status(met);

  ims_Info **exp_info = (ims_Info **) exp_list->SrvStatusInfo.arg;
  ims_Info **recv_info = (ims_Info **) recv_list->SrvStatusInfo.arg;

  if (validate_ims_info_struct_list(exp_info, recv_info) != true)
  {
    exp_node->set_expect_status(not_met);
  }
}
void ril_utf_test_t::validate_ims_get_colr( ril_utf_node_base_t *exp_node,
                                            struct ril_utf_msg_t *data)
{
  exp_node->set_expect_status(met); // start with met and mark unmet if failure

  if(exp_node->get_payload() != NULL)
  {
    ims_Colr *expect = (ims_Colr *)exp_node->get_payload();
    ims_Colr *recv = (ims_Colr *)data->payload;

    if (expect->has_presentation != recv->has_presentation ||
        expect->presentation != recv->presentation)
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Expected presentation: %d Received presentation: %d",
                     expect->presentation,
                     recv->presentation);
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error, messages do not match(presentation)");
      exp_node->set_expect_status(not_met);
      return;
    }

    if (expect->has_errorDetails == recv->has_errorDetails)
    {
      if(!validate_ims_sip_error_info_util(expect->errorDetails, recv->errorDetails)){
        exp_node->set_expect_status(not_met);
        return;
      }
    } else {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Expect has_errorDetail: %d Receive has_errorDetail: %d ",
                        expect->has_errorDetails, recv->has_errorDetails);
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error, messages do not match(has_errorDetails)");
      exp_node->set_expect_status(not_met);
      return;
    }
  }
  else
  {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error, No payload");
      exp_node->set_expect_status(not_met);
      return;
  }
}

void ril_utf_test_t::validate_ims_wfc_roaming_config_support( ril_utf_node_base_t *exp_node,
                                            struct ril_utf_msg_t *data)
{
  exp_node->set_expect_status(met); // start with met and mark unmet if failure

  if(exp_node->get_payload() != NULL)
  {
    ims_WfcRoamingConfigSupportInfo *expect = (ims_WfcRoamingConfigSupportInfo *)exp_node->get_payload();
    ims_WfcRoamingConfigSupportInfo *recv = (ims_WfcRoamingConfigSupportInfo *)data->payload;

    if (expect->wfcRoamingConfigSupport != recv->wfcRoamingConfigSupport)
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Expected WfcRoamingModeSupport: %d Received WfcRoamingModeSupport: %d",
                     expect->wfcRoamingConfigSupport,
                     recv->wfcRoamingConfigSupport);
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error, messages do not match(wfcRoamingConfigSupport)");
      exp_node->set_expect_status(not_met);
      return;
    }
  }
  else
  {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error, No payload");
      exp_node->set_expect_status(not_met);
      return;
  }
}

void ril_utf_test_t::validate_ims_ussd_failed( ril_utf_node_base_t *exp_node,
                                            struct ril_utf_msg_t *data)
{
  exp_node->set_expect_status(met); // start with met and mark unmet if failure

  if(exp_node->get_payload() != NULL)
  {
    ims_UssdMessageFailedInfo *expect = (ims_UssdMessageFailedInfo *)exp_node->get_payload();
    ims_UssdMessageFailedInfo *recv = (ims_UssdMessageFailedInfo *)data->payload;

    if (expect->type != recv->type)
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Expected ussdModeType: %d Received ussdModeType: %d",
                     expect->type,
                     recv->type);
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error, messages do not match(ussdModeType)");
      exp_node->set_expect_status(not_met);
      return;
    }
    if (!validate_ims_sip_error_info_util(expect->errorDetails, recv->errorDetails)) {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match for errorDetails");
      exp_node->set_expect_status(not_met);
      return;
    }
  }
  else
  {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error, No payload");
      exp_node->set_expect_status(not_met);
      return;
  }
}

bool ril_utf_test_t::validate_ims_sip_error_info_util(ims_SipErrorInfo &expect,
                                                    ims_SipErrorInfo &recv)
{
  RIL_UTF_ESSENTIAL("\n Expect hascode %d code %d string %s",
                    expect.has_sipErrorCode, expect.sipErrorCode,
                    (char *)expect.sipErrorString.arg);
  RIL_UTF_ESSENTIAL("\n Recv hascode %d code %d string %s",
                    recv.has_sipErrorCode, recv.sipErrorCode,
                    (char *)recv.sipErrorString.arg);
  if (expect.has_sipErrorCode != recv.has_sipErrorCode)
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Expect has_sipErrorCode: %d Receive has_sipErrorCode: %d",
                   expect.has_sipErrorCode,
                   recv.has_sipErrorCode);
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error, messages do not match (has_sipErrorCode)");
    return false;
  }
  if (expect.sipErrorCode != recv.sipErrorCode)
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Expected sipErrorCode: %d Received sipErrorCode: %d",
                   expect.sipErrorCode,
                   recv.sipErrorCode);
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error, messages do not match (sipErrorCode)");
    return false;
  }
  if (expect.sipErrorString.arg && recv.sipErrorString.arg &&
      strcmp((char*)expect.sipErrorString.arg, (char*)recv.sipErrorString.arg) != 0)
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Expected sipErrorString: %s Received sipErrorString: %s",
                   (char*)expect.sipErrorString.arg,
                   (char*)recv.sipErrorString.arg);
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error, messages do not match (sipErrorString)");
    return false;
  }
  return true;
}

void ril_utf_test_t::validate_ims_sip_error_info_msg( ril_utf_node_base_t *exp_node,
                                            struct ril_utf_msg_t *data)
{
  exp_node->set_expect_status(met); // start with met and mark unmet if failure

  if(exp_node->get_payload() != NULL)
  {
    ims_SipErrorInfo *expect = (ims_SipErrorInfo *)exp_node->get_payload();
    ims_SipErrorInfo *recv = (ims_SipErrorInfo *)data->payload;
    if(!validate_ims_sip_error_info_util(*expect, *recv)){
      exp_node->set_expect_status(not_met);
      return;
    }
  }
  else
  {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Error, No payload");
      exp_node->set_expect_status(not_met);
      return;
  }
}

#ifdef QMI_RIL_HAL_UTF
extern "C" void print_ims_CallList_Call (ims_CallList_Call *call);
#endif

void ril_utf_test_t::validate_ims_get_current_calls( ril_utf_node_base_t *exp_node,
                                                     struct ril_utf_msg_t *data)
{
  int i;
  bool validation_status = true;
  exp_node->set_expect_status(met); // start with met and mark unmet if failure

  ims_CallList *expect_list = (ims_CallList*)exp_node->get_payload();
  ims_CallList *recv_list = (ims_CallList*)data->payload;

  if ( (expect_list == NULL) ||
       (recv_list == NULL) )
  {
    if (exp_node->get_payload_len() == data->payload_len)
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Received no calls SUCCESS");
      exp_node->set_expect_status(met);
    }
    else
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: Received no calls FAILURE");
      exp_node->set_expect_status(not_met);
    }
    return;
  }

#ifndef UTF_TARGET_BUILD
  ims_CallList_Call **expect = (ims_CallList_Call **)expect_list->callAttributes.arg;
  ims_CallList_Call **recv = (ims_CallList_Call **) recv_list->callAttributes.arg;

  if (recv == NULL) {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTL: recv is NULL, returning!!\n");
    return;
  }

  // For each call
  for (i = 0; recv[i] != NULL; ++i)
  {
    if ( (expect[i]->als != recv[i]->als) ||
         (expect[i]->has_als != recv[i]->has_als) ||
         (expect[i]->index != recv[i]->index) ||
         (expect[i]->has_index != recv[i]->has_index) ||
         (expect[i]->isMT != recv[i]->isMT) ||
         (expect[i]->has_isMT != recv[i]->has_isMT) ||
         (expect[i]->isMpty != recv[i]->isMpty) ||
         (expect[i]->has_isMpty != recv[i]->has_isMpty) ||
         (expect[i]->isVoice != recv[i]->isVoice) ||
         (expect[i]->has_isVoice != recv[i]->has_isVoice) ||
         (expect[i]->isVoicePrivacy != recv[i]->isVoicePrivacy) ||
         (expect[i]->has_isVoicePrivacy != recv[i]->has_isVoicePrivacy) ||
         (expect[i]->state != recv[i]->state) ||
         (expect[i]->has_state != recv[i]->has_state) ||
         (expect[i]->toa != recv[i]->toa) ||
         (expect[i]->has_toa != recv[i]->has_toa) ||
         (expect[i]->has_isCalledPartyRinging != recv[i]->has_isCalledPartyRinging) ||
         (expect[i]->isCalledPartyRinging != recv[i]->isCalledPartyRinging) ||
         (expect[i]->has_isEncrypted != recv[i]->has_isEncrypted) ||
         (expect[i]->isEncrypted != recv[i]->isEncrypted) ||
         (expect[i]->failCause.errorDetails.sipErrorCode !=
           recv[i]->failCause.errorDetails.sipErrorCode) ||
         (expect[i]->tirMode != recv[i]->tirMode) )
    {
      validation_status = false;
      RIL_UTF_ESSENTIAL("QMI_RIL_UTF: For call %d: failed basic parameters check", i);
      break;
    }

    if (recv[i]->state == ims_CallState_CALL_END)
    {
      exp_node->set_expect_status(met);

      if (expect[i]->has_failCause != recv[i]->has_failCause)
      {
        validation_status = false;
        RIL_UTF_ESSENTIAL("QMI_RIL_UTF: For call %d: failed has_failCause check", i);
      }

      if (expect[i]->has_failCause)
      {
        if ((expect[i]->failCause.has_failcause != recv[i]->failCause.has_failcause) ||
            (expect[i]->failCause.has_failcause &&
             (expect[i]->failCause.failcause != recv[i]->failCause.failcause)))
        {
          validation_status = false;
          RIL_UTF_ESSENTIAL("QMI_RIL_UTF: For call %d: failed failCause check", i);
        }
        if ((expect[i]->failCause.networkErrorString.arg) &&
            (recv[i]->failCause.networkErrorString.arg))
        {
          if (strcmp((char*)expect[i]->failCause.networkErrorString.arg, (char*)recv[i]->failCause.networkErrorString.arg) != 0)
          {
            RIL_UTF_ESSENTIAL("QMI_RIL_UTF: For call %d: failed networkErrorString check", i);

            RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: For call %d", i);
            RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Expected networkerrorstring : %s", (char*)expect[i]->failCause.networkErrorString.arg);
            RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Recvd networkerrorstring : %s", (char*)recv[i]->failCause.networkErrorString.arg);
            validation_status = false;
          }
        }
      }
    }

    if ( (expect[i]->failCause.errorDetails.sipErrorString.arg) &&
              (recv[i]->failCause.errorDetails.sipErrorString.arg))
    {
      if (strcmp((char*)expect[i]->failCause.errorDetails.sipErrorString.arg,
          (char*)recv[i]->failCause.errorDetails.sipErrorString.arg) != 0)
      {
        validation_status = false;
        RIL_UTF_ESSENTIAL("QMI_RIL_UTF: For call %d: failed sipErrorString check", i);

        RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: For call %d", i);
        RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Expected siperrorstring : %s",
                  (char*)expect[i]->failCause.errorDetails.sipErrorString.arg);
        RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Recvd siperrorstring : %s",
                  (char*)recv[i]->failCause.errorDetails.sipErrorString.arg);
        exp_node->set_expect_status(not_met);
        break;
      }
    }

    // Peer information about the support for add participant
    if ((expect[i]->has_peerConfAbility != recv[i]->has_peerConfAbility) ||
        ((expect[i]->has_peerConfAbility &&
          ((expect[i]->peerConfAbility.has_isVideoConfSupported != recv[i]->peerConfAbility.has_isVideoConfSupported) ||
           (expect[i]->peerConfAbility.has_isVideoConfSupported &&
            (expect[i]->peerConfAbility.isVideoConfSupported != recv[i]->peerConfAbility.isVideoConfSupported))))))
    {
      RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: For call %d: failed peerConfAbility check", i);
      validation_status = false;
    }

    // VerstatInfo
    if (recv[i]->hasVerstatInfo && expect[i]->hasVerstatInfo)
    {
      if ((recv[i]->verstatInfo.canMarkUnwantedCall != expect[i]->verstatInfo.canMarkUnwantedCall) ||
          (recv[i]->verstatInfo.verificationStatus != expect[i]->verstatInfo.verificationStatus))
      {
        RIL_UTF_ESSENTIAL("QMI_RIL_UTF: For call %d: failed verstatInfo check", i);
        validation_status = false;
        break;
      }
    }

    // historyInfo
    if (recv[i]->historyInfo.arg != NULL)
    {
      if (expect[i]->historyInfo.arg == NULL)
      {
        RIL_UTF_ESSENTIAL("QMI_RIL_UTF: For call %d: failed historyInfo (null) check", i);
        validation_status = false;
        break;
      }
      char *recv_historyInfo = (char *)(recv[i]->historyInfo.arg);
      char *exp_historyInfo  = (char *)(expect[i]->historyInfo.arg);
      if (strcmp(recv_historyInfo, exp_historyInfo) != 0)
      {
        RIL_UTF_ESSENTIAL("QMI_RIL_UTF: For call %d: failed historyInfo check", i);
        validation_status = false;
        break;
      }
    }
    // terminatingNumber
    if (expect[i]->terminatingNumber[0] &&
        strcmp(recv[i]->terminatingNumber, expect[i]->terminatingNumber) != 0)
    {
      RIL_UTF_ESSENTIAL("QMI_RIL_UTF: For call %d: failed terminatingNumber check: exp %s, recv %s",
                    i, expect[i]->terminatingNumber, recv[i]->terminatingNumber);
      validation_status = false;
      break;
    }
    if (recv[i]->isSecondary != expect[i]->isSecondary)
    {
      RIL_UTF_ESSENTIAL("QMI_RIL_UTF: For call %d: failed isSecondary check: exp %d, recv %d",
                    i, expect[i]->isSecondary, recv[i]->isSecondary);
      validation_status = false;
      break;
    }
    // Evaluate call details
    if (recv[i]->has_callDetails && expect[i]->has_callDetails)
    {
      do {
        if ((expect[i]->callDetails.has_callType != recv[i]->callDetails.has_callType) ||
            (expect[i]->callDetails.callType != recv[i]->callDetails.callType) ||
            (expect[i]->callDetails.has_callDomain != recv[i]->callDetails.has_callDomain) ||
            (expect[i]->callDetails.callDomain != recv[i]->callDetails.callDomain) ||
            (expect[i]->callDetails.has_callSubstate != recv[i]->callDetails.has_callSubstate) ||
            (expect[i]->callDetails.callSubstate != recv[i]->callDetails.callSubstate) ||
            (expect[i]->callDetails.has_mediaId != recv[i]->callDetails.has_mediaId) ||
            (expect[i]->callDetails.mediaId != recv[i]->callDetails.mediaId))
        {
          validation_status = false;
          RIL_UTF_ESSENTIAL("QMI_RIL_UTF: For call %d: failed basic callDetails check", i);
          break;
        }
        if ((expect[i]->callDetails.has_causeCode != recv[i]->callDetails.has_causeCode) ||
            (expect[i]->callDetails.has_causeCode &&
             ((expect[i]->callDetails.causeCode != recv[i]->callDetails.causeCode))))
        {
          validation_status = false;
          RIL_UTF_ESSENTIAL("QMI_RIL_UTF: For call %d: callDetails.causeCode: exp: %d,%d recv: %d,%d",
              i, expect[i]->callDetails.has_causeCode, expect[i]->callDetails.causeCode,
              recv[i]->callDetails.has_causeCode, recv[i]->callDetails.causeCode);
          break;
        }
        // Local Ability
        if (recv[i]->callDetails.has_localAbility && expect[i]->callDetails.has_localAbility)
        {
          ims_Info **exp_localSrvStatusInfo = (ims_Info **)expect[i]->callDetails.localAbility.SrvStatusInfo.arg;
          ims_Info **recv_localSrvStatusInfo = (ims_Info **)recv[i]->callDetails.localAbility.SrvStatusInfo.arg;

          if (validate_ims_info_struct_list(exp_localSrvStatusInfo, recv_localSrvStatusInfo) != true)
          {
            RIL_UTF_ESSENTIAL("QMI_RIL_UTF: For call %d: failed localAbility check", i);
            validation_status = false;
            break;
          }
        }
        // Peer Ability
        if (recv[i]->callDetails.has_peerAbility && expect[i]->callDetails.has_peerAbility)
        {
          ims_Info **exp_peerSrvStatusInfo = (ims_Info **)expect[i]->callDetails.peerAbility.SrvStatusInfo.arg;
          ims_Info **recv_peerSrvStatusInfo = (ims_Info **)recv[i]->callDetails.peerAbility.SrvStatusInfo.arg;

          if (validate_ims_info_struct_list(exp_peerSrvStatusInfo, recv_peerSrvStatusInfo) != true)
          {
            RIL_UTF_ESSENTIAL("QMI_RIL_UTF: For call %d: failed peerAbility check", i);
            validation_status = false;
            break;
          }
        }
        // Extras
        if (recv[i]->callDetails.extras.arg != NULL)
        {
          if (expect[i]->callDetails.extras.arg == NULL)
          {
            RIL_UTF_ESSENTIAL("QMI_RIL_UTF: For call %d: failed extras (null) check", i);
            validation_status = false;
            break;
          }
          char **recv_extras = (char **)(recv[i]->callDetails.extras.arg);
          char **exp_extras  = (char **)(expect[i]->callDetails.extras.arg);
          while (*recv_extras && *exp_extras)
          {
            if (strcmp(*recv_extras, *exp_extras) != 0)
            {
              RIL_UTF_ESSENTIAL("QMI_RIL_UTF: For call %d: failed extras check", i);
              validation_status = false;
              break;
            }
            recv_extras++;
            exp_extras++;
          }
          if (validation_status == false)
          {
            break;
          }
        }
        // sipAlternateUri
        if (recv[i]->callDetails.sipAlternateUri.arg != NULL)
        {
          if (expect[i]->callDetails.sipAlternateUri.arg == NULL)
          {
            RIL_UTF_ESSENTIAL("QMI_RIL_UTF: For call %d: failed sipAlternateUri (null) check", i);
            validation_status = false;
            break;
          }
          char *recv_historyInfo = (char *)(recv[i]->callDetails.sipAlternateUri.arg);
          char *exp_historyInfo  = (char *)(expect[i]->callDetails.sipAlternateUri.arg);
          if (strcmp(recv_historyInfo, exp_historyInfo) != 0)
          {
            RIL_UTF_ESSENTIAL("QMI_RIL_UTF: For call %d: failed sipAlternateUri check", i);
            validation_status = false;
            break;
          }
        }
      } while (FALSE);
    }
    else
    {
      validation_status = false;
    }
    if (validation_status != true)
    {
      break;
    }
  } // end for
  if (validation_status != true)
  {
    exp_node->set_expect_status(not_met);

    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: For call %d", i);
    RIL_UTF_ESSENTIAL("\n\n QMI_RIL_UTF: Expected");
    print_ims_CallList_Call((ims_CallList_Call *)expect[i]);
    RIL_UTF_ESSENTIAL("\n\n QMI_RIL_UTF: Recvd");
    print_ims_CallList_Call((ims_CallList_Call *)recv[i]);
  }
#endif
}
#endif
bool ril_utf_test_t::validate_ims_info_struct_list(ims_Info **exp_info, ims_Info **recv_info)
{
  int info_idx = 0;
  bool validation_status = true;

  while ((exp_info && recv_info) &&
      (*exp_info && *recv_info))
  {
    if ((*exp_info)->has_isValid != (*recv_info)->has_isValid ||
        ((*exp_info)->has_isValid &&
         (*exp_info)->isValid != (*recv_info)->isValid))
    {
      validation_status = false;
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: info_idx %d, isValid exp: %d, %d recv: %d, %d\n",
          info_idx, (*exp_info)->has_isValid, (*exp_info)->isValid,
          (*recv_info)->has_isValid, (*recv_info)->isValid);
      break;
    }
    if ((*exp_info)->has_callType != (*recv_info)->has_callType ||
        ((*exp_info)->has_callType &&
         (*exp_info)->callType != (*recv_info)->callType))
    {
      validation_status = false;
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: info_idx %d, callType exp: %d, %d recv: %d, %d\n",
          info_idx, (*exp_info)->has_callType, (*exp_info)->callType,
          (*recv_info)->has_callType, (*recv_info)->callType);
      break;
    }

    ims_StatusForAccessTech **exp_acc_tech = (ims_StatusForAccessTech **)(*exp_info)->accTechStatus.arg;
    ims_StatusForAccessTech **recv_acc_tech = (ims_StatusForAccessTech **)(*recv_info)->accTechStatus.arg;
    int acc_tech_idx = 0;

    while ((exp_acc_tech && recv_acc_tech) &&
        (*exp_acc_tech && *recv_acc_tech))
    {
      if ((*exp_acc_tech)->has_networkMode != (*recv_acc_tech)->has_networkMode ||
          ((*exp_acc_tech)->has_networkMode &&
           (*exp_acc_tech)->networkMode != (*recv_acc_tech)->networkMode))
      {
        validation_status = false;
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: info_idx %d, acc_tech_idx %d"
                      " networkMode exp: %d, %d recv: %d, %d\n",
                      info_idx, acc_tech_idx,
                      (*exp_acc_tech)->has_networkMode, (*exp_acc_tech)->networkMode,
                      (*recv_acc_tech)->has_networkMode, (*recv_acc_tech)->networkMode);
        break;
      }
      if ((*exp_acc_tech)->has_status != (*recv_acc_tech)->has_status ||
          ((*exp_acc_tech)->has_status &&
           (*exp_acc_tech)->status != (*recv_acc_tech)->status))
      {
        validation_status = false;
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: info_idx %d, acc_tech_idx %d"
                      " status exp: %d, %d recv: %d, %d\n",
                      info_idx, acc_tech_idx,
                      (*exp_acc_tech)->has_status, (*exp_acc_tech)->status,
                      (*recv_acc_tech)->has_status, (*recv_acc_tech)->status);
        break;
      }
      if ((*exp_acc_tech)->has_registered != (*recv_acc_tech)->has_registered ||
          ((*exp_acc_tech)->has_registered &&
           ((*exp_acc_tech)->registered.has_state != (*recv_acc_tech)->registered.has_state ||
            ((*exp_acc_tech)->registered.has_state &&
             (*exp_acc_tech)->registered.state != (*recv_acc_tech)->registered.state))))
      {
        validation_status = false;
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: info_idx %d, acc_tech_idx %d"
                      " registered.state exp: %d, %d, %d recv: %d, %d, %d\n",
                      info_idx, acc_tech_idx,
                      (*exp_acc_tech)->has_registered, (*exp_acc_tech)->registered.has_state,
                      (*exp_acc_tech)->registered.state,
                      (*recv_acc_tech)->has_registered, (*recv_acc_tech)->registered.has_state,
                      (*recv_acc_tech)->registered.state);
        break;
      }
      acc_tech_idx++;
      exp_acc_tech++;
      recv_acc_tech++;
    }

    if (validation_status != true)
    {
      break;
    }

    exp_info++;
    recv_info++;
    info_idx++;
  }

  if (validation_status != true)
  {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match");
  }
  return validation_status;
}

#ifdef RIL_UTF_IOE
void ril_utf_test_t::validate_ioe_response( ril_utf_node_base_t *exp_node,
                                            struct ril_utf_msg_t *data)
{
   RIL_UTF_DEBUG("\n QMI_RIL_UTF: Verifying IoE response");
   if((data->msg_id != exp_node->get_msg_id()) ||
       /*(data->service_id != exp_node->get_qmi_srv_id()) ||*/
       ( (exp_node->is_verify_payload() == true && data->payload_len != exp_node->get_payload_len())))
   {
     RIL_UTF_DEBUG("\n QMI_RIL_UTL: expect msg id %04x got msg id %04x\n",
      exp_node->get_msg_id(), data->msg_id);
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: expect service id %02x got service id %02x\n",
      exp_node->get_service_id(), data->service_id);
    exp_node->set_expect_status(not_met);
    return;
   }
     if ( (exp_node->get_expect_skip_verification_status() == true) &&
       (data->payload != NULL) )
  {
    if ( memcmp(exp_node->get_payload(), data->payload, data->payload_len) != 0 )
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Expected");
      print_msg_in_hex( exp_node->get_payload(), exp_node->get_payload_len() );
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Recvd");
      print_msg_in_hex( data->payload, data->payload_len);
      exp_node->set_expect_status(not_met);
      return;
    }
  }

  exp_node->set_expect_status(met);
}
void ril_utf_test_t::validate_ioe_indication( ril_utf_node_base_t *exp_node,
                                              struct ril_utf_msg_t *data)
{
 RIL_UTF_DEBUG("\n QMI_RIL_UTF: Verifying IoE response");
   if((data->msg_id != exp_node->get_msg_id()) ||
       /*(data->service_id != exp_node->get_qmi_srv_id()) ||*/
       ( (exp_node->is_verify_payload() == true && data->payload_len != exp_node->get_payload_len())))
   {
     RIL_UTF_DEBUG("\n QMI_RIL_UTL: expect msg id %04x got msg id %04x\n",
      exp_node->get_msg_id(), data->msg_id);
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: expect service id %02x got service id %02x\n",
      exp_node->get_service_id(), data->service_id);
    exp_node->set_expect_status(not_met);
    return;
   }
     if ( (exp_node->get_expect_skip_verification_status() == true) &&
       (data->payload != NULL) )
  {
    if ( memcmp(exp_node->get_payload(), data->payload, data->payload_len) != 0 )
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Expected");
      print_msg_in_hex( exp_node->get_payload(), exp_node->get_payload_len() );
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Recvd");
      print_msg_in_hex( data->payload, data->payload_len);
      exp_node->set_expect_status(not_met);
      return;
    }
  }

  exp_node->set_expect_status(met);
}
#endif

void ril_utf_test_t::validate_qmi_request( ril_utf_node_base_t *exp_node,
                                   struct ril_utf_msg_t *data)
{
  RIL_UTF_DEBUG("\n QMI_RIL_UTF: Verifying QMI request");
  if ( (data->msg_id != exp_node->get_msg_id()) ||
       (data->service_id != exp_node->get_service_id()) ||
       (data->client_id != exp_node->get_client_id()) ||
       ( (exp_node->is_verify_payload() == true && data->payload_len != exp_node->get_payload_len())) )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: msg_id/service_id/client not matching\n");
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: expected msg id: %d, received msg id: %d\n",
            exp_node->get_msg_id(), data->msg_id);
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: expected service id: %d, received service id: %d\n",
            exp_node->get_service_id(), data->service_id);
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: expected client id: %d, received client id: %d\n",
            exp_node->get_client_id(), data->client_id);
    exp_node->set_expect_status(not_met);
    return;
  }

  if ( (exp_node->get_expect_skip_verification_status() == true) &&
       (data->payload != NULL) )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: compare payload\n");
    if ( memcmp(exp_node->get_payload(), data->payload, data->payload_len) != 0 )
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: payload not matching\n");
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: exp_node->get_payload_len() = %d", exp_node->get_payload_len());
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: data->payload_len = %d", data->payload_len);
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: exp_node->get_payload()\n");
      print_msg_in_hex( exp_node->get_payload(), exp_node->get_payload_len());
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: data->payload\n");
      print_msg_in_hex( data->payload, data->payload_len);

      exp_node->set_expect_status(not_met);
      return;
    }
  }

  exp_node->set_expect_status(met);
}

void ril_utf_test_t::validate_qmi_response( ril_utf_node_base_t *exp_node,
                                   struct ril_utf_msg_t *data)
{
  RIL_UTF_DEBUG("\n QMI_RIL_UTF: Verifying QMI response");
  if ( (data->msg_id != exp_node->get_msg_id()) ||
       (data->service_id != exp_node->get_service_id())
      )
  {
    //TBD: add checking of (data->payload_len != exp_node->get_payload_len())
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: expect msg id %04x got msg id %04x\n",
      exp_node->get_msg_id(), data->msg_id);
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: expect service id %02x got service id %02x\n",
      exp_node->get_service_id(), data->service_id);
    exp_node->set_expect_status(not_met);
    return;
  }

  /*
     TBD: validate_qmi_response comparison code
  */
  exp_node->set_expect_status(met);
}

void ril_utf_test_t::validate_qmi_indication( ril_utf_node_base_t *exp_node,
                                   struct ril_utf_msg_t *data)
{
  RIL_UTF_DEBUG("\n QMI_RIL_UTF: Verifying QMI indication");
  if ( (data->msg_id != exp_node->get_msg_id()) ||
       (data->service_id != exp_node->get_service_id())
      )
  {
    //TBD: add checking of (data->payload_len != exp_node->get_payload_len())
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: expect msg id %04x got msg id %04x\n",
      exp_node->get_msg_id(), data->msg_id);
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: expect service id %02x got service id %02x\n",
      exp_node->get_service_id(), data->service_id);
    exp_node->set_expect_status(not_met);
    return;
  }
  /*
     TBD: validate_qmi_indication comparison code
  */
  exp_node->set_expect_status(met);
}

#ifndef RIL_UTF_IOE
void ril_utf_test_t::validate_ril_unsol( ril_utf_node_base_t *exp_node,
                                   struct ril_utf_msg_t *data)
{
  exp_node->set_expect_status(met); // start with met

  RIL_UTF_DEBUG("\n QMI_RIL_UTF: Verifying RIL Unsol message");
  if ( data->msg_id != exp_node->get_msg_id() )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: expect msg id %d got msg id %d\n",
      exp_node->get_msg_id(), data->msg_id);
    exp_node->set_expect_status(not_met);
    return;
  }
  if ( (exp_node->get_expect_skip_verification_status() == true) &&
       (exp_node->get_payload() != NULL) )
  {
    if ( (exp_node->get_msg_id() == RIL_UNSOL_RESPONSE_NEW_SMS) ||
            (exp_node->get_msg_id() == RIL_UNSOL_RESPONSE_NEW_SMS_STATUS_REPORT) ||
            (exp_node->get_msg_id() == RIL_UNSOL_SIGNAL_STRENGTH))
    {
      if (memcmp(exp_node->get_payload(), data->payload, data->payload_len))
      {
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected %s", exp_node->get_payload());
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: Recvd %s", data->payload);
        exp_node->set_expect_status(not_met);
        return;
      }
    }
    else if ( exp_node->get_msg_id() == RIL_UNSOL_ON_USSD )
    {
        exp_node->set_expect_status(verify_ril_unsol_on_ussd(exp_node->get_payload(), data->payload));
    }
    else if ( exp_node->get_msg_id() == RIL_UNSOL_ON_SS )
    {
        exp_node->set_expect_status(verify_ril_unsol_on_ss(exp_node->get_payload(), data->payload));
    }
    else if ( exp_node->get_msg_id() == RIL_UNSOL_SUPP_SVC_NOTIFICATION )
    {
        exp_node->set_expect_status(verify_ril_unsol_supp_svc_notification(exp_node->get_payload(), data->payload));
    }
    else if (exp_node->get_msg_id() == RIL_UNSOL_CDMA_CALL_WAITING)
    {
        exp_node->set_expect_status(verify_ril_unsol_cdma_call_waiting(exp_node->get_payload(), data->payload));
    }
    else if (exp_node->get_msg_id() == RIL_UNSOL_CDMA_OTA_PROVISION_STATUS)
    {
        exp_node->set_expect_status(verify_ril_unsol_cdma_ota_provision_status(exp_node->get_payload(), data->payload));
    }
    else if (exp_node->get_msg_id() == RIL_UNSOL_CDMA_INFO_REC)
    {
        exp_node->set_expect_status(verify_ril_unsol_cdma_info_rec(exp_node->get_payload(), data->payload));
    }
    else if (exp_node->get_msg_id() == RIL_UNSOL_RINGBACK_TONE)
    {
        exp_node->set_expect_status(verify_ril_unsol_ringback_tone(exp_node->get_payload(), data->payload));
    }
    else if (exp_node->get_msg_id() == RIL_UNSOL_NETWORK_SCAN_RESULT)
    {
        exp_node->set_expect_status(verify_ril_unsol_network_scan_result(exp_node->get_payload(), data->payload));
    }
    else
    {
      if ( memcmp(exp_node->get_payload(), data->payload, data->payload_len) != 0 )
      {
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Expected");
        print_msg_in_hex( exp_node->get_payload(), exp_node->get_payload_len() );
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Recvd");
        print_msg_in_hex( data->payload, data->payload_len);
        exp_node->set_expect_status(not_met);
        return;
      }
    }

  } // skip verification

}
void ril_utf_test_t::validate_msim_recv( ril_utf_node_base_t *exp_node,
                         struct ril_utf_msg_t *data)
{
  RIL_UTF_DEBUG("\n QMI_RIL_UTF: Verifying MSIM recv message");
  exp_node->set_expect_status(met);

  if ( data->rild_instance_id != exp_node->get_rild_instance_id() )
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Error RILD IPC wrong instance id expected %d recieved %d",
                          exp_node->get_rild_instance_id(), data->rild_instance_id);
    exp_node->set_expect_status(not_met);
    return;
  }

  // Do default memcmp for all static allocated msgs
  if ( (exp_node->get_expect_skip_verification_status() == true) &&
       (data->payload != NULL) )
  {
    if (data->payload_len != exp_node->get_payload_len())
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: expect payload len %d got payload len %d\n",
        exp_node->get_payload_len(), data->payload_len);
      exp_node->set_expect_status(not_met);
    }
    if ( memcmp(exp_node->get_payload(), data->payload, data->payload_len) != 0 )
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Expected");
      print_msg_in_hex( exp_node->get_payload(), exp_node->get_payload_len() );
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Recvd");
      print_msg_in_hex( data->payload, data->payload_len);
      exp_node->set_expect_status(not_met);
      return;
    }
  }
}
void ril_utf_test_t::validate_ril_resp( ril_utf_node_base_t *exp_node,
                                        struct ril_utf_msg_t *data)
{
  RIL_UTF_DEBUG("\n QMI_RIL_UTF: Verifying RIL resp message");
  exp_node->set_expect_status(met);

  //TODO: need to add to test node so we can specify fail cases
  if ( data->error_val != exp_node->get_ril_error_value())
  {
    RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Error RIL Request returned wrong fail cause %d %d", exp_node->get_ril_error_value(), data->error_val);
    exp_node->set_expect_status(not_met);
    return;
  }

  if (data->token_id != exp_node->get_token_id())
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: expect token id %d got token id %d",
      exp_node->get_token_id(), data->token_id);
    exp_node->set_expect_status(not_met);
    return;
  }

  if ( (exp_node->get_payload_len() == 0) &&
       (data->payload_len == 0) )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTL: empty payload");
    return;
  }

  if (exp_node->get_expect_skip_verification_status() == true)
  {
  if ( exp_node->get_msg_id() == RIL_REQUEST_GET_CURRENT_CALLS )
  {
    exp_node->set_expect_status(verify_ril_get_current_calls(exp_node->get_payload(), exp_node->get_payload_len(), data->payload, data->payload_len));
  }
  else if ( (exp_node->get_msg_id() == RIL_REQUEST_SEND_SMS) || ( exp_node->get_msg_id() == RIL_REQUEST_IMS_SEND_SMS ) )
  {
    exp_node->set_expect_status(verify_ril_send_sms(exp_node->get_payload(), data->payload));
  }
  else if (exp_node->get_msg_id() == RIL_REQUEST_VOICE_REGISTRATION_STATE)
  {
    verify_ril_voice_registration_state(exp_node, data);
  }
  else if (exp_node->get_msg_id() == RIL_REQUEST_DATA_REGISTRATION_STATE)
  {
    verify_ril_data_registration_state(exp_node, data);
  }
  else if ( exp_node->get_msg_id() == RIL_REQUEST_OPERATOR )
  {
    exp_node->set_expect_status(verify_ril_operator_name(exp_node->get_payload(), data->payload));
  }
  else if ( exp_node->get_msg_id() == RIL_REQUEST_QUERY_AVAILABLE_NETWORKS )
  {
    exp_node->set_expect_status(verify_ril_query_available_network(exp_node->get_payload(), data->payload));
  }
  else if (exp_node->get_msg_id() == RIL_REQUEST_IMS_REGISTRATION_STATE)
  {
    verify_ril_ims_registration_state(exp_node, data);
  }
  else if (exp_node->get_msg_id() == RIL_REQUEST_QUERY_CALL_FORWARD_STATUS)
  {
    exp_node->set_expect_status(
            verify_ril_query_call_forward_status(exp_node->get_payload(),
                exp_node->get_payload_len(),
                data->payload,
                data->payload_len));
  }
  else if (exp_node->get_msg_id() == RIL_REQUEST_GET_SIM_STATUS )
  {
    exp_node->set_expect_status(
            verify_ril_get_sim_status(exp_node->get_payload(),
                                      exp_node->get_payload_len(),
                                      data->payload,
                                      data->payload_len));
  }
  else if (exp_node->get_msg_id() == RIL_REQUEST_SIM_OPEN_CHANNEL)
  {
    exp_node->set_expect_status(
            verify_ril_open_channel_response(exp_node->get_payload(),
                                      exp_node->get_payload_len(),
                                      data->payload,
                                      data->payload_len));
  }
  else if (exp_node->get_msg_id() == RIL_REQUEST_QUERY_TTY_MODE)
  {
    if (data->error_val == RIL_E_SUCCESS) {
      exp_node->set_expect_status(
              verify_ril_query_tty_mode(exp_node->get_payload(),
                                        exp_node->get_payload_len(),
                                        data->payload,
                                        data->payload_len));
    }
  }
  else if (exp_node->get_msg_id() == RIL_REQUEST_CDMA_QUERY_PREFERRED_VOICE_PRIVACY_MODE)
  {
    if (data->error_val == RIL_E_SUCCESS) {
      exp_node->set_expect_status(
              verify_ril_query_preferred_voice_privacy_mode(exp_node->get_payload(),
                                        exp_node->get_payload_len(),
                                        data->payload,
                                        data->payload_len));
    }
  }
  else if ( exp_node->get_msg_id() == RIL_REQUEST_GET_NEIGHBORING_CELL_IDS )
  {
    exp_node->set_expect_status(
            verify_ril_get_neighboring_cell_ids(
                exp_node->get_payload(),
                exp_node->get_payload_len(),
                data->payload,
                data->payload_len));
  }
  else if ( exp_node->get_msg_id() == RIL_REQUEST_LAST_CALL_FAIL_CAUSE )
  {
    exp_node->set_expect_status(
            verify_ril_last_call_fail_cause(
                exp_node->get_payload(),
                exp_node->get_payload_len(),
                data->payload,
                data->payload_len));
  }
  else if (exp_node->get_msg_id() == RIL_REQUEST_GET_MUTE)
  {
    exp_node->set_expect_status(verify_ril_get_mute(exp_node->get_payload(),
                                                    exp_node->get_payload_len(),
                                                    data->payload,
                                                    data->payload_len));
  }
  else if (exp_node->get_msg_id() == RIL_REQUEST_CDMA_QUERY_ROAMING_PREFERENCE)
  {
    exp_node->set_expect_status(verify_ril_cdma_query_roaming_preference(exp_node->get_payload(),
                                                    exp_node->get_payload_len(),
                                                    data->payload,
                                                    data->payload_len));
  }
  else if (exp_node->get_msg_id() == RIL_REQUEST_CDMA_GET_SUBSCRIPTION_SOURCE)
  {
    exp_node->set_expect_status(verify_ril_cdma_get_subscription_source(exp_node->get_payload(),
                                                    exp_node->get_payload_len(),
                                                    data->payload,
                                                    data->payload_len));
  }
  //TODO: Add more special case verification blocks here
  else
  {
    // Do default memcmp for all static allocated msgs
    if ( (exp_node->get_expect_skip_verification_status() == true) &&
         (data->payload != NULL) )
    {
      if (data->payload_len != exp_node->get_payload_len())
      {
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: expect payload len %d got payload len %d\n",
          exp_node->get_payload_len(), data->payload_len);
        exp_node->set_expect_status(not_met);
      }
      if ( memcmp(exp_node->get_payload(), data->payload, data->payload_len) != 0 )
      {
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Expected");
        print_msg_in_hex( exp_node->get_payload(), exp_node->get_payload_len() );
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Recvd");
        print_msg_in_hex( data->payload, data->payload_len);
        exp_node->set_expect_status(not_met);
        return;
      }
    }
  } //end msg if
  }

}

void ril_utf_test_t::validate_dsi_request_response( ril_utf_node_base_t *exp_node,
                                                    struct ril_utf_msg_t *data)
{
  RIL_UTF_DEBUG("\n QMI_RIL_UTF: Verifying DSI request");
  if ( (data->msg_id != exp_node->get_msg_id())||( (exp_node->is_verify_payload() == true && data->payload_len != exp_node->get_payload_len())) )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: msg_id/payload_len not matching\n");
    exp_node->set_expect_status(not_met);
    return;
  }

  if ( (exp_node->get_expect_skip_verification_status() == true) &&
       (data->payload != NULL) )
  {
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: compare payload\n");
    if ( memcmp(exp_node->get_payload(), data->payload, data->payload_len) != 0 )
    {
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: payload not matching\n");
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: exp_node->get_payload_len() = %d", exp_node->get_payload_len());
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: data->payload_len = %d", data->payload_len);
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: exp_node->get_payload()\n");
      print_msg_in_hex( exp_node->get_payload(), exp_node->get_payload_len());
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: data->payload\n");
      print_msg_in_hex( data->payload, data->payload_len);

      exp_node->set_expect_status(not_met);
      return;
    }
  }

  exp_node->set_expect_status(met);
}



void ril_utf_test_t::verify_ril_ims_registration_state(ril_utf_node_base_t *exp_node,
                                  struct ril_utf_msg_t *data)
{
  exp_node->set_expect_status(met); // start with met and mark unmet if failure
  int *expect = (int *) exp_node->get_payload();
  int *recv = (int *) data->payload;

  if( (NULL == expect) || (NULL == recv) )
    return;

  if( ( expect[0] != recv[0] ) || ( expect[1] != recv[1] ) )
  {
    exp_node->set_expect_status(not_met);
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected reg state %d received reg state %d", expect[0], recv[0]);
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected tech %d received tech %d", expect[1], recv[1]);
  }
}

void ril_utf_test_t::verify_ril_voice_registration_state(ril_utf_node_base_t *exp_node,
                                  struct ril_utf_msg_t *data)
{
  exp_node->set_expect_status(met); // start with met and mark unmet if failure
  RIL_VoiceRegistrationStateResponse *expect = (RIL_VoiceRegistrationStateResponse *) exp_node->get_payload();
  RIL_VoiceRegistrationStateResponse *recv = (RIL_VoiceRegistrationStateResponse *) data->payload;

  if( (NULL == expect) || (NULL == recv) )
    return;

  if( ( expect->regState != recv->regState ) || ( expect->rat != recv->rat ) )
  {
    exp_node->set_expect_status(not_met);
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected voice reg state %d received voice reg state %d", expect->regState, recv->regState);
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected voice tech %d received voice tech %d", expect->rat, recv->rat);
  }
}

void ril_utf_test_t::verify_ril_data_registration_state(ril_utf_node_base_t *exp_node,
                                  struct ril_utf_msg_t *data)
{
  exp_node->set_expect_status(met); // start with met and mark unmet if failure
  RIL_DataRegistrationStateResponse *expect = (RIL_DataRegistrationStateResponse *) exp_node->get_payload();
  RIL_DataRegistrationStateResponse *recv = (RIL_DataRegistrationStateResponse *) data->payload;

  if( (NULL == expect) || (NULL == recv) )
    return;

  if( ( expect->regState != recv->regState ) || ( expect->rat != recv->rat ) )
  {
    exp_node->set_expect_status(not_met);
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected data reg state %d received data reg state %d", expect->regState, recv->regState);
    RIL_UTF_DEBUG("\n QMI_RIL_UTF: Expected data tech %d data received tech %d", expect->rat, recv->rat);
  }
}

#endif

/*========================================================
  FUNCTION: update_exp_ptr
  - updates the expectation pointer to be passed to the
    verifying entity.

  Critical section access:
  - exec_q_access should be locked prior to calling this
    function
========================================================*/
struct ril_utf_msg_t * ril_utf_test_t::update_exp_ptr( ril_utf_node_base_t *test_step )
{
  struct ril_utf_msg_t *ptr = NULL;

  ptr = (struct ril_utf_msg_t *) malloc(sizeof(struct ril_utf_msg_t));

  ptr->next = NULL;
  ptr->prev = NULL;

  ptr->type = test_step->get_payload_type();

  ptr->msg_id = test_step->get_msg_id();
  ptr->token_id = test_step->get_token_id();
  ptr->service_id = test_step->get_service_id();

  ptr->payload_len = test_step->get_payload_len();
  ptr->payload = (void *) malloc(ptr->payload_len);
  memcpy(ptr->payload, test_step->get_payload(), test_step->get_payload_len());

  return ptr;
}

/*========================================================
  FUNCTION: update_inj_ptr
  - updates the inject pointer to be passed to the
    inject handling entity.

  Critical section access:
  - exec_q_access should be locked prior to calling this
    function
========================================================*/
struct ril_utf_msg_t* ril_utf_test_t::update_inj_ptr(ril_utf_node_base_t *test_step )
{
  struct ril_utf_msg_t *ptr = NULL;

  ptr = (struct ril_utf_msg_t *) malloc(sizeof(struct ril_utf_msg_t));

  ptr->next = NULL;
  ptr->prev = NULL;

  ptr->client_id = test_step->get_client_id();
  ptr->msg_id    = test_step->get_msg_id();
  ptr->service_id = test_step->get_service_id();
  ptr->token_id = test_step->get_token_id();
  ptr->type = test_step->get_payload_type();
  #ifndef RIL_UTF_IOE
  ptr->oem_command_id = test_step->get_oem_command_id();
  #endif
  if(test_step->get_payload_len() == 0 || test_step->get_payload() == NULL)
  {
    ptr->payload_len = 0;
    ptr->payload = NULL;
  }
  else
  {
    ptr->payload_len = test_step->get_payload_len();
    ptr->payload = (void *) malloc(ptr->payload_len);
    memcpy(ptr->payload, test_step->get_payload(), ptr->payload_len);
  }

  return ptr;

}

/*========================================================
  FUNCTION: set_test_id
  - set the test id from test writer's input
========================================================*/
void ril_utf_test_t::set_test_id(string id)
{
  result.test_id = id;
  attr.setID(id);
}
string ril_utf_test_t::get_test_id(void)
{
  return result.test_id;
}

/*========================================================
  FUNCTION: set_boot_case
  - set the boot test to be run before this test case
  - NOTE: THIS SHOULD BE NULL FOR BOOT TEST
========================================================*/
void ril_utf_test_t::set_boot_case(test_result (*func)() )
{
  boot_case = func;
}

/*========================================================
  FUNCTION: set_cleanup_case
  - set the cleanup test to be run after this test case
========================================================*/
void ril_utf_test_t::set_cleanup_case(int (*func)() )
{
  cleanup_case = func;
}

/*========================================================
  FUNCTION: set_test_description
  - set the test description from test writer's input
========================================================*/
void ril_utf_test_t::set_test_description(string desc)
{
  result.test_description = desc;
}
string ril_utf_test_t::get_test_description(void)
{
  return result.test_description;
}

/*========================================================
  FUNCTION: set_test_attribute
  - set the test attribute from test writer's input
========================================================*/
void ril_utf_test_t::set_test_attribute(std::string attribute)
{
  attr.setAttribute(attribute);
}

/*========================================================
  FUNCTION: isAttributeMatch
  - Used to see if the attributes or ID in a test case match
    any of the user specified test cases
========================================================*/
bool ril_utf_test_t::isAttributeMatch (user_specified_attrib_t newattr)
{
  return attr.isAttributeMatch(newattr);
}

/*========================================================
  FUNCTION: get_test_result
  - get the test_result class from the test case as return value
========================================================*/
test_result ril_utf_test_t::get_test_result(void) {
  return result;
}


/*========================================================
  FUNCTION: print_received_message
  - print received expectation with
========================================================*/
void ril_utf_test_t::print_received_message(ril_utf_msg_t* msg_ptr)
{
  switch ( msg_ptr->type )
  {
    case ril_utf_ril_request:
    case ril_utf_ril_unsol_response:
    case ril_utf_ril_response:
      print_ril_message_name(msg_ptr->msg_id);
      RIL_UTF_HTML_LOG("     QMI_RIL_UTL: Message ID: %d, Token ID: %d, Type: %s", msg_ptr->msg_id, msg_ptr->token_id, ril_utf_q_element_t_name[msg_ptr->type].name);
      print_msg_in_hex(msg_ptr->payload, msg_ptr->payload_len);
      break;

    case ril_utf_qmi_request:
    case ril_utf_qmi_response:
    case ril_utf_qmi_indication:
      print_qmi_message_name(msg_ptr->service_id, msg_ptr->msg_id, msg_ptr->type);
      RIL_UTF_HTML_LOG("     QMI_RIL_UTL: Message ID: 0x%04x, Type: %s", msg_ptr->msg_id, ril_utf_q_element_t_name[msg_ptr->type].name);
      print_msg_in_tlv(msg_ptr->msg_id, (ril_utf_qmi_srv_id_t)msg_ptr->service_id, 0, msg_ptr->type, msg_ptr->payload, msg_ptr->payload_len);
      break;
    case ril_utf_oem_request:
    case ril_utf_oem_response:
    case ril_utf_oem_indication:
       RIL_UTF_HTML_LOG("     QMI_RIL_UTL: Message ID: 0x%04x, Type: %s", msg_ptr->msg_id, ril_utf_q_element_t_name[msg_ptr->type].name);
      #ifndef RIL_UTF_IOE
      print_oem_message_name(msg_ptr->service_id, msg_ptr->msg_id, msg_ptr->oem_command_id, msg_ptr->type);
      #endif
      break;
    case ril_utf_ims_request:
    case ril_utf_ims_response:
    case ril_utf_ims_indication:
      #ifndef RIL_UTF_IOE
      print_ims_message_name(msg_ptr->msg_id);
      #endif
      RIL_UTF_HTML_LOG("     QMI_RIL_UTL: Message ID: %d, Type: %s", msg_ptr->msg_id, ril_utf_q_element_t_name[msg_ptr->type].name);
      break;
   case ril_utf_ioe_response:
   case ril_utf_ioe_indication:
      RIL_UTF_HTML_LOG("     QMI_RIL_UTL: Message ID: %d, Type: %s", msg_ptr->msg_id, ril_utf_q_element_t_name[msg_ptr->type].name);
      print_msg_in_hex(msg_ptr->payload, msg_ptr->payload_len);
      break;
   case ril_utf_db_response:
       print_db_msg(msg_ptr->payload, msg_ptr->payload_len);
       break;
   case ril_utf_msim_send_msg:
   case ril_utf_msim_recv_msg:
      #ifndef RIL_UTF_IOE
      RIL_UTF_HTML_LOG("     QMI_RIL_UTL: Message ID: %d, Type: %s", msg_ptr->msg_id, ril_utf_q_element_t_name[msg_ptr->type].name);
      print_msim_message_name(msg_ptr->msg_id);
      print_msg_in_hex(msg_ptr->payload, msg_ptr->payload_len);
      #endif
   default:
      RIL_UTF_HTML_LOG("     Unsupported Message Type %d", msg_ptr->type);
      break;
  }
}

/*========================================================
  FUNCTION: print_test_point
  - print the current test point number in each test case
  - also print the uid for further tracking purpose
  - start from 1
========================================================*/
void ril_utf_test_t::print_test_point()
{
  RIL_UTF_HTML_LOG_ORIG("\n");
  RIL_UTF_HTML_LOG_ORIG("\n==================================================================================");
  ++testPointNum;
  static const char alphabet[] = "0123456789!@#$%^&*ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  int alphabet_len =  sizeof(alphabet) - 1;
  int i;
  std::string str(16, ' ');
  for (i = 0; i < 16; ++i) {
    str[i] = alphabet[rand() % alphabet_len];
  }
  RIL_UTF_HTML_LOG_ORIG("\nTest Point #%d, UID = %s", testPointNum, str.c_str());
  RIL_UTF_HTML_LOG_ORIG("\n==================================================================================");
}

/*========================================================
  FUNCTION: clear_db_before_execution
  - if called, UTF will remove qcril.db before execution
========================================================*/
void ril_utf_test_t::clear_db_before_execution(void)
{
  clear_db = true;
}

void ril_utf_test_t::reset_mbn_sim_info()
{
  ril_utf_reset_sim_info();
}

void ril_utf_test_t::set_mbn_sim_info(char *iccid, char *mcc, char *mnc, int slot)
{
  ril_utf_set_sim_info(iccid, mcc, mnc, slot);
}

void ril_utf_test_t::validate_imss_rtt_message( ril_utf_node_base_t *exp_node,
                                                     struct ril_utf_msg_t *data)
{
  exp_node->set_expect_status(not_met);
  int i;
  int validation_success = TRUE;
  ims_RttMessage *exp = (ims_RttMessage*)exp_node->get_payload();
  ims_RttMessage *recv = (ims_RttMessage*)data->payload;

  do
  {
      if (strcmp( ((char*)exp->rttMessage.arg), ((char*)recv->rttMessage.arg) ) != 0) {
        RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, message don't match\n");
        validation_success = FALSE;
        break;
      }
  } while (FALSE);

  if (validation_success)
  {
      exp_node->set_expect_status(met);
  }
  else
  {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match");
      exp_node->set_expect_status(not_met);
  }
}

int validate_block_reason_details(ims_BlockReasonDetails &exp, ims_BlockReasonDetails &recv) {
  int validation_success = TRUE;
  do
  {
    if (exp.has_regFailureReasonType != recv.has_regFailureReasonType ||
        (recv.has_regFailureReasonType &&
         (exp.regFailureReasonType != recv.regFailureReasonType))) {
      validation_success = FALSE;
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: regFailureReasonType exp = %d, %d and recv = %d, %d\n",
          exp.has_regFailureReasonType, exp.regFailureReasonType,
          recv.has_regFailureReasonType, recv.regFailureReasonType)
      break;
    }
    if (exp.has_RegFailureReason != recv.has_RegFailureReason ||
        (recv.has_RegFailureReason &&
         (exp.RegFailureReason != recv.RegFailureReason))) {
      validation_success = FALSE;
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: RegFailureReason exp = %d, %d and recv = %d, %d\n",
          exp.has_RegFailureReason, exp.RegFailureReason,
          recv.has_RegFailureReason, recv.RegFailureReason)
      break;
    }
  } while (0);
  return validation_success;
}
int validate_block_status(ims_BlockStatus &exp, ims_BlockStatus &recv) {
  int validation_success = TRUE;
  do
  {
    if ((exp.has_blockReason != recv.has_blockReason) ||
        (recv.has_blockReason &&
         (exp.blockReason != recv.blockReason))) {
      validation_success = FALSE;
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: blockReason exp = %d, %d and recv = %d, %d\n",
          exp.has_blockReason, exp.blockReason,
          recv.has_blockReason, recv.blockReason);
      break;
    }
    if ((exp.has_blockReasonDetails == recv.has_blockReasonDetails) &&
        recv.has_blockReasonDetails) {
      validation_success = validate_block_reason_details(exp.blockReasonDetails,
          recv.blockReasonDetails);
    }
  } while (0);
  return validation_success;
}

void ril_utf_test_t::validate_ims_unsol_registration_block_status( ril_utf_node_base_t *exp_node,
                                                     struct ril_utf_msg_t *data)
{
  int validation_success = TRUE;
  ims_RegistrationBlockStatus *exp = (ims_RegistrationBlockStatus *) exp_node->get_payload();
  ims_RegistrationBlockStatus *recv = (ims_RegistrationBlockStatus *) data->payload;

  do
  {
    if ((exp->has_imsBlockStatusOnWwan == recv->has_imsBlockStatusOnWwan) &&
        recv->has_imsBlockStatusOnWwan) {
      validation_success = validate_block_status(exp->imsBlockStatusOnWwan,
          recv->imsBlockStatusOnWwan);
    }
    if (!validation_success) {
      break;
    }
    if ((exp->has_imsBlockStatusOnWlan == recv->has_imsBlockStatusOnWlan) &&
        recv->has_imsBlockStatusOnWlan) {
      validation_success = validate_block_status(exp->imsBlockStatusOnWlan,
          recv->imsBlockStatusOnWlan);
    }
  } while (0);

  if (validation_success) {
      exp_node->set_expect_status(met);
  } else {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match");
      exp_node->set_expect_status(not_met);
  }
}

void ril_utf_test_t::validate_ims_unsol_auto_call_rejection_ind( ril_utf_node_base_t *exp_node,
                                                     struct ril_utf_msg_t *data)
{
  int validation_success = TRUE;
  ims_AutoCallRejectionInfo *exp = (ims_AutoCallRejectionInfo *) exp_node->get_payload();
  ims_AutoCallRejectionInfo *recv = (ims_AutoCallRejectionInfo *) data->payload;

  do
  {
    if (exp->callType != recv->callType) {
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: callType exp = %d and recv = %d\n",
          exp->callType, recv->callType);
      validation_success = FALSE;
      break;
    }
    if (exp->autoRejectionCause != recv->autoRejectionCause) {
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: autoRejectionCause exp = %d and recv = %d\n",
          exp->autoRejectionCause, recv->autoRejectionCause);
      validation_success = FALSE;
      break;
    }
    if (exp->sipErrorCode != recv->sipErrorCode) {
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: sipErrorCode exp = %d and recv = %d\n",
          exp->sipErrorCode, recv->sipErrorCode);
      validation_success = FALSE;
      break;
    }
    if (exp->hasNumber != recv->hasNumber) {
      RIL_UTF_DEBUG("\n QMI_RIL_UTF: hasNumber exp = %d and recv = %d\n",
          exp->hasNumber, recv->hasNumber);
      validation_success = FALSE;
      break;
    }
    if (exp->hasNumber && exp->number.arg) {
      if (!recv->number.arg) {
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: empty recv number");
        validation_success = FALSE;
        break;
      }
      if ((strcmp((char*)exp->number.arg, (char*)recv->number.arg) != 0)) {
        RIL_UTF_DEBUG("\n QMI_RIL_UTF: number exp = %s and recv = %s\n",
            (char *)exp->number.arg, (char *)recv->number.arg);
        validation_success = FALSE;
        break;
      }
    }
  } while (0);

  if (validation_success) {
      exp_node->set_expect_status(met);
  } else {
      RIL_UTF_DEBUG("\n QMI_RIL_UTL: Error, messages do not match");
      exp_node->set_expect_status(not_met);
  }
}
