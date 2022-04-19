/*========================================================================
Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */


#include "common.h"
#include "session.h"
#include "memory.h"
#include "profile.h"
#include "decision.h"
#include "dispatcher.h"
#include "core.h"
#include <string.h>
#include <stdlib.h>

/*========================================================================*/
/* INTERNAL FUNCTIONS */
/*==  =====================================================================*/

/**
 * Function: create_session
 *
 * Description: TBD
 *
 * Arguments:
 *   @arg1: TBD
 *   @arg2: TBD
 *
 * Return values: TBD
 *
 * Notes: none
 **/
static th_err_t create_session(
  th_session_manager_t *th_sess_mgr,
  int idx
)
{
  th_err_t err = TH_SUCCESS;
  int i = 0, j = 0;
  th_session_t *th_session = NULL;

  if (IS_NULL(th_sess_mgr)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  th_sess_mgr->th_sessions[idx] = (th_session_t *)malloc(sizeof(th_session_t));
  if (IS_NULL(th_sess_mgr->th_sessions[idx])) {
    TH_LOG_ERR(" Mem allocation failure in thread services library");
    return TH_ERROR_FATAL;
  }
  memset(th_sess_mgr->th_sessions[idx], 0x0, sizeof(th_session_t));

  th_session = th_sess_mgr->th_sessions[idx];

  //allocate the component memories
  th_session->dispatcher = (th_dispatcher_t *)malloc(sizeof(th_dispatcher_t));
  if (IS_NULL(th_session->dispatcher)) {
    TH_LOG_ERR(" Mem allocation failure in thread services library");
    return TH_ERROR_FATAL;
  }
  memset(th_session->dispatcher, 0x0, sizeof(th_dispatcher_t));

  th_session->decision_mgr = (th_decision_manager_t *)
                                    malloc(sizeof(th_decision_manager_t));
  if (IS_NULL(th_session->decision_mgr)) {
    TH_LOG_ERR(" Mem allocation failure in thread services library");
    return TH_ERROR_FATAL;
  }
  memset(th_session->decision_mgr, 0x0, sizeof(th_decision_manager_t));

  //initialize the fields
  th_session->session_key = -1;
  th_session->deadline = 33; //33 ms, default for 30 fps
  th_session->mem_ptr = NULL;

  TH_LOG_LOW(" create_session successful");
  return err;
}


/**
 * Function: destroy_session
 *
 * Description: TBD
 *
 * Arguments:
 *   @arg1: TBD
 *   @arg2: TBD
 *
 * Return values: TBD
 *
 * Notes: none
 **/
static th_err_t destroy_session(
  th_session_manager_t *th_sess_mgr,
  int idx
)
{
  th_err_t err = TH_SUCCESS;
  th_session_t *th_session = NULL;

  if (IS_NULL(th_sess_mgr) || IS_NULL(th_sess_mgr->th_sessions[idx])) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  th_session = th_sess_mgr->th_sessions[idx];

  if (!IS_NULL(th_session->dispatcher)) {
    free(th_session->dispatcher);
  }

  if (!IS_NULL(th_session->decision_mgr)) {
    free(th_session->decision_mgr);
  }

  free(th_session);
  th_sess_mgr->th_sessions[idx] = NULL;

  TH_LOG_LOW(" destroy_session successful");
  return err;
}


/**
 * Function: find_thread_session
 *
 * Description: TBD
 *
 * Arguments:
 *   @arg1: TBD
 *   @arg2: TBD
 *
 * Return values: TBD
 *
 * Notes: none
 **/
static int find_thread_session(
  th_session_manager_t *th_sess_mgr,
  int session_key
)
{
  int i = 0;

  for (i = 0; i < MAX_TH_SESSIONS; i++) {
    if (session_key == th_sess_mgr->th_sessions[i]->session_key) {
      return i;
    }
  }

  return -1;
}


/**
 * Function: get_empty_session
 *
 * Description: TBD
 *
 * Arguments:
 *   @arg1: TBD
 *   @arg2: TBD
 *
 * Return values: TBD
 *
 * Notes: none
 **/
static int get_empty_session(th_session_manager_t *th_sess_mgr)
{
  int i = 0;

  for (i = 0; i < MAX_TH_SESSIONS; i++) {
    if (-1 == th_sess_mgr->th_sessions[i]->session_key) {
      return i;
    }
  }

  return i;
}


/*========================================================================*/
/* INTERNAL INTERFACE IMPLEMENTATION */
/*========================================================================*/

/**
 * Function: create_session_manager
 *
 * Description: Creates and initializes data structures for individual
 * sessions and profilers upto max concurrent sessions allowed.
 * This is done ONE TIME per lifetime of Camera Demon.
 *
 * Arguments:
 *   @th_serv: Pointer to global handle to library
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
th_err_t create_session_manager(th_services_t *th_serv)
{
  th_err_t err = TH_SUCCESS;
  int i = 0;
  th_session_manager_t *th_sess_mgr = NULL;

  //initialize session manager handle
  th_serv->th_sess_mgr =
             (th_session_manager_t *)malloc(sizeof(th_session_manager_t));
  if (IS_NULL(th_serv->th_sess_mgr)) {
    TH_LOG_ERR(" Mem allocation failure in thread services library");
    return TH_ERROR_FATAL;
  }
  memset(th_serv->th_sess_mgr, 0x0, sizeof(th_session_manager_t));

  th_sess_mgr = th_serv->th_sess_mgr;

  //create individual sessions
  for (i = 0; i < MAX_TH_SESSIONS; i++) {
    err = create_session(th_sess_mgr, i);
    if (err != TH_SUCCESS) {
      TH_LOG_ERR(" Failed at create_session, err: %x", err);
      return err;
    }
  }

  //create profiler for individual sessions
  err = create_profiler(th_sess_mgr);
  if (err != TH_SUCCESS) {
    TH_LOG_ERR(" Failed at create_profiler, err: %x", err);
    th_sess_mgr->is_profiler_created = 0;
    err = TH_SUCCESS;//this is OK, profiler may be compiled out
  } else {
    th_sess_mgr->is_profiler_created = 1;
  }

  th_sess_mgr->curr_session_key = -1;
  th_sess_mgr->last_session_key = -1;
  th_sess_mgr->num_active_sessions = 0;

  TH_LOG_LOW(" create_session_manager successful, ptr: %p",
                                            th_serv->th_sess_mgr);
  return err;
}


/**
 * Function: destroy_session_manager
 *
 * Description: Frees data structures for individual sessions
 * and profilers upto max concurrent sessions allowed.
 * This is done ONE TIME per lifetime of Camera Demon.
 *
 * Arguments:
 *   @th_serv: Pointer to global handle to library
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
th_err_t destroy_session_manager(th_services_t *th_serv)
{
  th_err_t err = TH_SUCCESS;
  int i = 0;
  th_session_manager_t *th_sess_mgr = NULL;

  if (IS_NULL(th_serv)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  th_sess_mgr = th_serv->th_sess_mgr;
  if (IS_NULL(th_sess_mgr)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  if (th_sess_mgr->is_profiler_created) {
    err = destroy_profiler(th_sess_mgr);//don't care about error code
    th_sess_mgr->is_profiler_created = 0;
  }

  for (i = 0; i < MAX_TH_SESSIONS; i++) {
    destroy_session(th_sess_mgr, i);//don't care about error code
  }

  free(th_sess_mgr);
  th_serv->th_sess_mgr = NULL;

  TH_LOG_LOW(" destroy_session_manager successful");
  return err;
}


/**
 * Function: init_session
 *
 * Description: Initializes a particular Thread Library session
 * Typically corresponds 1:1 with an actual Camera session
 *
 * Arguments:
 *   @th_serv: Pointer to global handle to library
 *   @session_key: Unique session id of actual Camera session
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
th_err_t init_thread_session(th_services_t *th_serv, int session_key)
{
  th_err_t err = TH_SUCCESS;
  int idx = 0;
  th_session_manager_t *th_sess_mgr = NULL;
  th_session_t *th_session = NULL;

  th_sess_mgr = th_serv->th_sess_mgr;
  if (IS_NULL(th_sess_mgr)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  idx = find_thread_session(th_sess_mgr, session_key);
  if (idx != -1) {
    TH_LOG_ERR(" Session already exists");
    return TH_ERROR_SESSION_EXISTS;
  }

  idx = get_empty_session(th_sess_mgr);
  if (idx == MAX_TH_SESSIONS) {
    TH_LOG_ERR(" No space in thread library to accomodate new session");
    return TH_ERROR_SESSION_INVALID;
  }

  th_session = th_sess_mgr->th_sessions[idx];
  if (IS_NULL(th_session)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  if (th_sess_mgr->is_profiler_created) {
    //assign the profiler for the session
    th_session->th_profiler = th_sess_mgr->th_profiler[idx];
    if (IS_NULL(th_session->th_profiler)) {
      TH_LOG_ERR(" Encountered NULL pointer");
      return TH_ERROR_NULLPTR;
    }
  }

  th_session->session_key = session_key;
  th_session->mem_ptr = get_memory_ptr(th_serv->th_mem_mgr);

  err = init_decision_manager(th_session);
  if (err != TH_SUCCESS) {
    TH_LOG_ERR(" Failed at init_decision_manager");
    return TH_ERROR_FATAL;
  }
  th_session->decision_manager_init = 1;

  err = init_queues_lists(th_session);
  if (err != TH_SUCCESS) {
    TH_LOG_ERR(" Failed at init_queues");
    return TH_ERROR_FATAL;
  }
  th_session->core_init = 1;

  err = init_profiler(th_session);
  if (err != TH_SUCCESS) {
    TH_LOG_ERR(" Failed at init_profiler");
    return TH_ERROR_FATAL;
  }
  th_session->profiler_init = 1;

  err = init_dispatcher(th_session);
  if (err != TH_SUCCESS) {
    TH_LOG_ERR(" Failed at init_dispatcher");
    return TH_ERROR_FATAL;
  }
  th_session->dispatcher_init = 1;

  th_sess_mgr->last_session_key = th_sess_mgr->curr_session_key;
  th_sess_mgr->curr_session_key = session_key;
  th_sess_mgr->num_active_sessions++;

  TH_LOG_LOW(" init_session successful, current session_key: %d", session_key);
  return err;
}


/**
 * Function: deinit_session
 *
 * Description: De-initializes a particular Thread Library session
 * Typically corresponds 1:1 with an actual Camera session
 *
 * Arguments:
 *   @th_serv: Pointer to global handle to library
 *   @session_key: Unique session id of actual Camera session
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
th_err_t deinit_thread_session(th_services_t *th_serv, int session_key)
{
  th_err_t err = TH_SUCCESS;
  int idx = 0;
  th_session_manager_t *th_sess_mgr = NULL;
  th_session_t *th_session = NULL;

  th_sess_mgr = th_serv->th_sess_mgr;
  if (IS_NULL(th_sess_mgr)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  idx = find_thread_session(th_sess_mgr, session_key);
  if (idx == -1) {
    TH_LOG_ERR(" Session does not exist");
    return TH_ERROR_SESSION_INVALID;
  }

  th_session = th_sess_mgr->th_sessions[idx];
  if (IS_NULL(th_session)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  if (th_session->dispatcher_init) {
    err = deinit_dispatcher(th_session);//don't care about error code
  }

  if (th_session->core_init) {
    err = deinit_queues_lists(th_session);//don't care about error code
  }

  if (th_session->decision_manager_init) {
    err = deinit_decision_manager(th_session);//don't care about error code
  }

  if (th_session->profiler_init) {
    err = deinit_profiler(th_session);//don't care about error code
  }

  th_session->session_key = -1;
  set_memory_ptr(th_serv->th_mem_mgr, th_session->mem_ptr);
  th_session->mem_ptr = NULL;

  th_sess_mgr->curr_session_key = th_sess_mgr->last_session_key;
  th_sess_mgr->num_active_sessions--;

  TH_LOG_MED(" deinit_session successful");
  return err;
}


/**
 * Function: submit_new_job
 *
 * Description: Submits a new job (previously registered) to
 * Thread Library. If the job is blocking, waits on a semaphore
 *
 * Arguments:
 *   @th_serv: Pointer to global handle to library
 *   @handle: Job Instance Handle received from job registration
 *   @job_params: Consolidated job parameters
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
th_err_t submit_new_job(
  th_services_t *th_serv,
  unsigned int handle,
  job_params_t *job_params
)
{
  int idx = 0;
  th_err_t err = TH_SUCCESS;
  th_job_t *job = NULL;
  th_session_manager_t *th_sess_mgr = NULL;
  th_session_t *th_session = NULL;
  th_profiler_t *th_profiler = NULL;

  if (IS_NULL(th_serv) || IS_NULL(th_serv->th_sess_mgr)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  th_sess_mgr = th_serv->th_sess_mgr;

  if (th_sess_mgr->curr_session_key == (unsigned int)-1) {
    TH_LOG_ERR(" No session is currently runing");
    return TH_ERROR_SESSION_INVALID;
  }

  idx = find_thread_session(th_sess_mgr, (handle & 0xF0000000) >> 28);
  if (idx == -1) {
    TH_LOG_ERR(" Session does not exist");
    return TH_ERROR_SESSION_INVALID;
  }

  th_session = th_sess_mgr->th_sessions[idx];
  if (IS_NULL(th_session)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  if (th_sess_mgr->is_profiler_created) {
    th_profiler = th_session->th_profiler;
    if (IS_NULL(th_profiler)) {
      TH_LOG_ERR(" Encountered NULL pointer");
      return TH_ERROR_NULLPTR;
    }
  }

  err = decision_manager_create_job_entry(th_session, handle,
                                            job_params, &job);
  if (err != TH_SUCCESS) {
    return err;
  }

  err = profiler_add_job(th_profiler, job);//don't care about error code for now

  if (job_params->is_blocking) {
    job->blocking = 1;
    //init semaphore
    sem_init(&job->msg_sem, 0, 0);
  } else {
    job->blocking = 0;
  }

  err = decision_manager_add_job(th_session, job);
  if (err != TH_SUCCESS) {
    TH_LOG_ERR(" Failed to add new job at decision_manager_add_job,"
                     "occupancy = %d", th_session->decision_mgr->job_occupancy);
    decision_manager_del_job(th_session, job);//don't care about the error code
    return err;
  }

  if (job_params->is_blocking) {
    //wait on semaphore
    TH_LOG_MED("Blocking job (%s), waiting on semaphore", job->job_func_name);
    sem_wait(&job->msg_sem);
    TH_LOG_MED("Blocking job (%s), coming out of wait", job->job_func_name);
    sem_destroy(&job->msg_sem);
  }

  TH_LOG_LOW("Job instance (%s) submitted to Thread Library",
                                                job->job_func_name);
  return err;
}


/**
 * Function: register_new_job
 *
 * Description: Registers a new job to the Thread Library.
 * Registration is typically 1:1 with HW instance (like ISP)
 * or SW instance (like MCT Start and Stop)
 *
 * Arguments:
 *   @th_serv: Pointer to global handle to library
 *   @arg1-argN: Rest correspond 1:1 with the Public API
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
th_err_t register_new_job(
  th_services_t *th_serv,
  unsigned int session_key,
  job_func_t job_func,
  char *job_func_name,
  job_callback_t flush_done_addr,
  int is_fifo,
  unsigned int *handle
)
{
  th_err_t err = TH_SUCCESS;
  int idx = 0;
  th_job_t *job = NULL;
  th_session_manager_t *th_sess_mgr = NULL;
  th_session_t *th_session = NULL;

  if (IS_NULL(th_serv) || IS_NULL(th_serv->th_sess_mgr)
      || IS_NULL(handle)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  th_sess_mgr = th_serv->th_sess_mgr;

  if (th_sess_mgr->curr_session_key == (unsigned int)-1) {
    TH_LOG_ERR(" No session is currently runing");
    return TH_ERROR_SESSION_INVALID;
  }

  if (session_key != th_sess_mgr->curr_session_key) {
    TH_LOG_HIGH(" Job registration is not of current session."
                " Session id coming with job registration may not exist");
  }

  idx = find_thread_session(th_sess_mgr, session_key);
  if (idx == -1) {
    TH_LOG_ERR(" Session does not exist");
    return TH_ERROR_SESSION_INVALID;
  }

  th_session = th_sess_mgr->th_sessions[idx];
  if (IS_NULL(th_session)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  err = decision_manager_register_job(th_session,
                                      job_func,
                                      job_func_name,
                                      flush_done_addr,
                                      is_fifo,
                                      session_key,
                                      handle);
  if (err != TH_SUCCESS) {
    TH_LOG_ERR(" Failed to register new job");
    return err;
  }

  TH_LOG_LOW("Job instance (%s) registered to Thread Library", job_func_name);
  return err;
}


/**
 * Function: stop_existing_job
 *
 * Description: Stops an existing registered job. After stop,
 * no new job post requests are accepted, and error is returned.
 * All jobs submitted before stop, and pending dispatch are returned
 * to the caller via the job_stopped callback. A stop is equivalent
 * to a flush, and flush_done callback is called after all jobs are
 * flushed out
 *
 * Arguments:
 *   @th_serv: Pointer to global handle to library
 *   @arg1-argN: Rest correspond 1:1 with the Public API
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
th_err_t stop_existing_job(
  th_services_t *th_serv,
  unsigned int handle,
  void *user_data,
  int is_blocking
)
{
  th_err_t err = TH_SUCCESS;
  unsigned int session_key = 0;
  int idx = 0;
  th_job_t *job = NULL;
  th_session_manager_t *th_sess_mgr = NULL;
  th_session_t *th_session = NULL;

  if (IS_NULL(th_serv) || IS_NULL(th_serv->th_sess_mgr)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  th_sess_mgr = th_serv->th_sess_mgr;

  if (th_sess_mgr->curr_session_key == (unsigned int)-1) {
    TH_LOG_ERR(" No session is currently runing");
    return TH_ERROR_SESSION_INVALID;
  }

  session_key = (handle >> 28) & 0xF;

  if (session_key != th_sess_mgr->curr_session_key) {
    TH_LOG_HIGH(" Job flush is not of current session."
                " Session id coming with job flush may not exist");
  }

  idx = find_thread_session(th_sess_mgr, session_key);
  if (idx == -1) {
    TH_LOG_ERR(" Session does not exist");
    return TH_ERROR_SESSION_INVALID;
  }

  th_session = th_sess_mgr->th_sessions[idx];
  if (IS_NULL(th_session)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  err = decision_manager_stop_job(th_session, handle, user_data, is_blocking);
  if (err != TH_SUCCESS) {
    TH_LOG_ERR(" Failed to register new job");
    return err;
  }

  TH_LOG_LOW("stop_existing_job successful");
  return err;
}

