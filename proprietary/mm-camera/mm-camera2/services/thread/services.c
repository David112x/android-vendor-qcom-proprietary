/*========================================================================
Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */


#include "common.h"
#include "session.h"
#include "memory.h"
#include <string.h>
#include <stdlib.h>

/*========================================================================*/
/* GLOBAL DEFINITIONS */
/*========================================================================*/
static pthread_mutex_t global_thread_mutex = PTHREAD_MUTEX_INITIALIZER;
static int thread_services_initialized = 0;
static th_services_t *th_services = NULL;


/*========================================================================*/
/* INTERNAL FUNCTIONS */
/*==  =====================================================================*/

/**
 * Function: init_thread_services_internal
 *
 * Description: Shim layer function to interact with Session/Memory manager
 * Shim layer used to help in any future adaptation, extension of APIs
 *
 * Arguments:
 *   @arg1: TBD
 *   @arg2: TBD
 *
 * Return values: TBD
 *
 * Notes: none
 **/
static th_err_t init_thread_services_internal()
{
  th_err_t err = TH_SUCCESS;

  //The caller function doesn't have to do any clean-up
  //if this function fails. This is  a self-cleaning function

  if (th_services != NULL) {
    //deinit didn't happen properly, we cannot proceed
    return TH_ERROR_FATAL;
  }

  //initialize the global thread services handle
  th_services = (th_services_t *)malloc(sizeof(th_services_t));
  if (IS_NULL(th_services)) {
    TH_LOG_ERR(" Mem allocation failure in thread services library");
    return TH_ERROR_FATAL;
  }
  memset(th_services, 0x0, sizeof(th_services_t));

  //a. Create session manager
  //b. Create memory manager

  err = create_memory_manager(th_services);
  if (err != TH_SUCCESS) {
    TH_LOG_ERR(" Failed at create_memory_manager, err: %x", err);
    return err;
  }

  err = create_session_manager(th_services);
  if (err != TH_SUCCESS) {
    TH_LOG_ERR(" Failed at create_session_manager, err: %x", err);
    return err;
  }

  TH_LOG_LOW(" init_thread_services_internal successful, ptr: %p", th_services);
  return err;
}


/**
 * Function: deinit_thread_services_internal
 *
 * Description: Shim layer function to interact with Session/Memory manager
 * Shim layer used to help in any future adaptation, extension of APIs
 *
 * Arguments:
 *   @arg1: TBD
 *   @arg2: TBD
 *
 * Return values: TBD
 *
 * Notes: none
 **/
static th_err_t deinit_thread_services_internal()
{
  th_err_t err = TH_SUCCESS;

  //a. Destroy session manager
  //b. Destroy memory manager

  if (IS_NULL(th_services)) {
    TH_LOG_LOW(" Global pointer null, but thread_services_initialized = 1");
    return TH_SUCCESS;
  }

  err = destroy_session_manager(th_services);//don't care about error code
  if (err != TH_SUCCESS) {
    TH_LOG_ERR(" Failed at destroy_session_manager, err: %x", err);
  }

  err = destroy_memory_manager(th_services);//don't care about error code
  if (err != TH_SUCCESS) {
    TH_LOG_ERR(" Failed at destroy_memory_manager, err: %x", err);
  }

  if (!IS_NULL(th_services)) {
    free(th_services);
    th_services = NULL;
  }

  TH_LOG_LOW(" deinit_thread_services_internal successful");
  return TH_SUCCESS;//return success always
}


/**
 * Function: create_thread_session_internal
 *
 * Description: Shim layer function to interact with Session Manager
 * Shim layer used to help in any future adaptation, extension of APIs
 *
 * Arguments:
 *   @arg1: TBD
 *   @arg2: TBD
 *
 * Return values: TBD
 *
 * Notes: none
 **/
static th_err_t create_thread_session_internal(int session_key)
{
  th_err_t err = TH_SUCCESS;

  err = init_thread_session(th_services, session_key);
  if (err != TH_SUCCESS) {
    TH_LOG_ERR(" Failed at init_thread_session, err: %x", err);
    return err;
  }

  TH_LOG_LOW(" create_thread_session_internal successful");
  return err;
}


/**
 * Function: destroy_thread_session_internal
 *
 * Description: Shim layer function to interact with Session manager
 * Shim layer used to help in any future adaptation, extension of APIs
 *
 * Arguments:
 *   @arg1: TBD
 *   @arg2: TBD
 *
 * Return values: TBD
 *
 * Notes: none
 **/
static th_err_t destroy_thread_session_internal(int session_key)
{
  th_err_t err = TH_SUCCESS;

  err = deinit_thread_session(th_services, session_key);
  if (err != TH_SUCCESS) {
    TH_LOG_ERR(" Failed at deinit_thread_session, err: %x", err);
    return err;
  }

  TH_LOG_LOW(" destroy_thread_session_internal successful");
  return err;
}


/**
 * Function: post_new_job_internal
 *
 * Description: Shim layer function to interact with Session manager
 * Shim layer used to help in any future adaptation, extension of APIs
 *
 * Arguments:
 *   @arg1: TBD
 *   @arg2: TBD
 *
 * Return values: TBD
 *
 * Notes: none
 **/
static th_err_t post_new_job_internal(
  unsigned int handle,
  job_callback_t stopped_cb,
  void *data_ptr,
  int priority,
  int relative_deadline,
  int is_splitable,
  int is_blocking,
  uint64_t job_ts
)
{
  th_err_t err = TH_SUCCESS;
  job_params_t job_params;

  if (0 == handle) {
    TH_LOG_ERR(" Encountered NULL job handle");
    return TH_ERROR_FATAL;
  }

  if (IS_NULL(data_ptr)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  job_params.stopped_cb = stopped_cb;
  job_params.data_ptr = data_ptr;
  job_params.is_blocking = is_blocking;
  job_params.is_splitable = is_splitable;
  job_params.priority = priority;
  job_params.relative_deadline = relative_deadline;
  job_params.job_ts = job_ts;

  err = submit_new_job(th_services, handle, &job_params);
  if (err != TH_SUCCESS) {
    TH_LOG_ERR(" Failed at submit_new_job, err: %x", err);
    return err;
  }

  TH_LOG_LOW(" post_new_job_internal successful");
  return err;
}


/**
 * Function: register_job_request_internal
 *
 * Description: Shim layer function to interact with Session manager
 * Shim layer used to help in any future adaptation, extension of APIs
 *
 * Arguments:
 *   @arg1: TBD
 *   @arg2: TBD
 *
 * Return values: TBD
 *
 * Notes: none
 **/
static th_err_t register_job_request_internal(
  unsigned int session_key,
  job_func_t job_func,
  char *job_func_name,
  job_callback_t flush_done_addr,
  int is_fifo,
  unsigned int *handle
)
{
  th_err_t err = TH_SUCCESS;

  if (job_func == NULL) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  err = register_new_job(th_services,
                         session_key,
                         job_func,
                         job_func_name,
                         flush_done_addr,
                         is_fifo,
                         handle);
  if (err != TH_SUCCESS) {
    TH_LOG_ERR(" Failed at submit_new_job, err: %x", err);
    return err;
  }

  TH_LOG_LOW(" register_job_request_internal successful");
  return err;
}


/**
 * Function: stop_job_request_internal
 *
 * Description: Shim layer function to interact with Session manager
 * Shim layer used to help in any future adaptation, extension of APIs
 *
 * Arguments:
 *   @arg1: TBD
 *   @arg2: TBD
 *
 * Return values: TBD
 *
 * Notes: none
 **/
static th_err_t stop_job_request_internal(
  unsigned int handle,
  void *user_data,
  int is_blocking
)
{
  th_err_t err = TH_SUCCESS;

  if (0 == handle) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  err = stop_existing_job(th_services, handle, user_data, is_blocking);
  if (err != TH_SUCCESS) {
    TH_LOG_ERR(" Failed at stop_existing_job, err: %x", err);
    return err;
  }

  TH_LOG_LOW(" stop_job_request_internal successful");
  return err;
}


/*========================================================================*/
/* PUBLIC INTERFACE IMPLEMENTATION */
/*========================================================================*/

/**
 * Function: init_thread_services
 *
 * Description: Please refer public API documentaion in header
 *
 * Arguments:
 *   @arg1:
 *   @arg2:
 *
 * Return values:
 *
 * Notes: none
 **/
th_err_t init_thread_services()
{
  th_err_t err = TH_SUCCESS;

  //lock global mutex
  //access to thread services library is always locked
  pthread_mutex_lock(&global_thread_mutex);

  //if already initialized, deinit first
  if (thread_services_initialized) {
    TH_LOG_HIGH(" Thread services already initialized, doing deinit first");
    deinit_thread_services_internal();//don't care about error code
  }

  //init thread services library
  err = init_thread_services_internal();
  if (err != TH_SUCCESS) {
    //attempt to clean up before exit
    deinit_thread_services_internal();//don't care about error code
    //unlock global mutex
    pthread_mutex_unlock(&global_thread_mutex);
    TH_LOG_ERR(" Fatal error occured in thread service library");
    return TH_ERROR_FATAL;
  }

  //thread services now initialized
  thread_services_initialized = 1;

  //unlock global mutex
  pthread_mutex_unlock(&global_thread_mutex);

  TH_LOG_MED(" Thread services init successful");
  return err;
}


/**
 * Function: deinit_thread_services
 *
 * Description: Please refer public API documentaion in header
 *
 * Arguments:
 *   @arg1:
 *   @arg2:
 *
 * Return values:
 *
 * Notes: none
 **/
th_err_t deinit_thread_services()
{
  th_err_t err = TH_SUCCESS;

  //lock global mutex
  //access to thread services library is always locked
  pthread_mutex_lock(&global_thread_mutex);

  if (!thread_services_initialized) {
    //unlock global mutex
    pthread_mutex_unlock(&global_thread_mutex);
    TH_LOG_HIGH(" Thread services currently not initialized, returning");
    return err;
  }

  //deinit thread services library
  err = deinit_thread_services_internal();
  if (err != TH_SUCCESS) {
    //unlock global mutex
    pthread_mutex_unlock(&global_thread_mutex);
    TH_LOG_ERR(" Fatal error occured in thread service library");
    return TH_ERROR_FATAL;
  }

  //thread services now deinitialized
  thread_services_initialized = 0;

  //unlock global mutex
  pthread_mutex_unlock(&global_thread_mutex);

  TH_LOG_MED(" Thread services deinit successful");
  return err;
}


/**
 * Function: create_thread_session
 *
 * Description: Please refer public API documentaion in header
 *
 * Arguments:
 *   @arg1:
 *   @arg2:
 *
 * Return values:
 *
 * Notes: none
 **/
th_err_t create_session(unsigned int session_key)
{
  th_err_t err = TH_SUCCESS;

  //lock global mutex
  //access to thread services library is always locked
  pthread_mutex_lock(&global_thread_mutex);

  //if thread services not initialized, try to init, and if not success return fatal error
  if (!thread_services_initialized) {
    //unlock global mutex, it will be locked in init_thread_services
    pthread_mutex_unlock(&global_thread_mutex);
    TH_LOG_HIGH(" Thread services not initialized, doing init first");
    err = init_thread_services();
    if (err != TH_SUCCESS) {
      TH_LOG_ERR(" Thread services not initialized");
      return TH_ERROR_FATAL;
    } else {
      //lock global mutex
      pthread_mutex_lock(&global_thread_mutex);
    }
  }

  //create the thread library session
  err = create_thread_session_internal(session_key);
  if (err != TH_SUCCESS) {
    //attempt to clean up before exit
    destroy_thread_session_internal(session_key);//don't care about error code
    //unlock global mutex
    pthread_mutex_unlock(&global_thread_mutex);
    TH_LOG_ERR(" Fatal error occured in thread service library");
    return TH_ERROR_FATAL;
  }

  //unlock global mutex
  pthread_mutex_unlock(&global_thread_mutex);

  TH_LOG_MED(" Thread session create successful for key: %d", session_key);
  return err;
}


/**
 * Function: deinit_thread_session
 *
 * Description: Please refer public API documentaion in header
 *
 * Arguments:
 *   @arg1:
 *   @arg2:
 *
 * Return values:
 *
 * Notes: none
 **/
th_err_t destroy_session(unsigned int session_key)
{
  th_err_t err = TH_SUCCESS;

  //lock global mutex
  //access to thread services library is always locked
  pthread_mutex_lock(&global_thread_mutex);

  //if thread services not initialized, return fatal error
  if (!thread_services_initialized) {
    //unlock global mutex
    pthread_mutex_unlock(&global_thread_mutex);
    TH_LOG_ERR(" Thread services not initialized");
    return TH_ERROR_FATAL;
  }

  //destroy the thread library session
  err = destroy_thread_session_internal(session_key);
  if (err != TH_SUCCESS) {
    //unlock global mutex
    pthread_mutex_unlock(&global_thread_mutex);
    TH_LOG_ERR(" Fatal error occured in thread service library");
    return TH_ERROR_FATAL;
  }

  //unlock global mutex
  pthread_mutex_unlock(&global_thread_mutex);

  TH_LOG_MED(" Thread session destroy successful for key: %d", session_key);
  return err;
}


/**
 * Function: post_job_request
 *
 * Description: Please refer public API documentaion in header
 *
 * Arguments:
 *   @arg1:
 *   @arg2:
 *
 * Return values:
 *
 * Notes: none
 **/
th_err_t post_job_request(
  unsigned int handle,
  void (*stopped_cb)(void *user_data),
  void *data_ptr,
  int priority,
  int relative_deadline,
  int is_splitable,
  int is_blocking
)
{
  th_err_t err = TH_SUCCESS;
  uint64_t job_ts = 0;
  TIMER_SYSTIME_TYPE timestamp;

  //if thread services not initialized, return fatal error
  if (!thread_services_initialized) {
    TH_LOG_ERR(" Thread services not initialized");
    return TH_ERROR_FATAL;
  }

  //attach the timestamp of arrival of this job atomically
  pthread_mutex_lock(&global_thread_mutex);
  __timer_gettime(timestamp);
  job_ts = __timer_readtime(timestamp);
  pthread_mutex_unlock(&global_thread_mutex);

  //post job to the internal queues
  err = post_new_job_internal(handle,
                              stopped_cb,
                              data_ptr,
                              priority,
                              relative_deadline,
                              is_splitable,
                              is_blocking,
                              job_ts);
  if (err != TH_SUCCESS) {
    TH_LOG_ERR(" Fatal error occured in thread service library");
    return TH_ERROR_FATAL;
  }

  TH_LOG_LOW(" Post new job request successful");
  return err;
}

/**
 * Function: register_job_request
 *
 * Description: Please refer public API documentaion in header
 *
 * Arguments:
 *   @arg1:
 *   @arg2:
 *
 * Return values:
 *
 * Notes: none
 **/
unsigned int register_job_request(
  unsigned int session_key,
  void *(*job_func_addr) (void *arg),
  char *job_func_name,
  void (*flush_done_addr) (void *arg),
  int is_fifo
)
{
  th_err_t err = TH_SUCCESS;
  unsigned int handle = 0;

  //if thread services not initialized, return fatal error
  if (!thread_services_initialized) {
    TH_LOG_ERR(" Thread services not initialized");
    return TH_ERROR_FATAL;
  }

  //post job to the internal queues
  err = register_job_request_internal(session_key,
                                      job_func_addr,
                                      job_func_name,
                                      flush_done_addr,
                                      is_fifo,
                                      &handle);
  if (err != TH_SUCCESS) {
    TH_LOG_ERR(" Fatal error occured in thread service library");
    return 0;
  }

  TH_LOG_HIGH(" register_job_request successful, job name/handle: %s/%x",
                                                job_func_name, handle);
  return handle;
}

/**
 * Function: stop_job_request
 *
 * Description: Please refer public API documentaion in header
 *
 * Arguments:
 *   @arg1:
 *   @arg2:
 *
 * Return values:
 *
 * Notes: none
 **/
th_err_t stop_job_request(
  unsigned int handle,
  void *user_data,
  int is_blocking
)
{
  th_err_t err = TH_SUCCESS;

  //if thread services not initialized, return fatal error
  if (!thread_services_initialized) {
    TH_LOG_ERR(" Thread services not initialized");
    return TH_ERROR_FATAL;
  }

  //post job to the internal queues
  err = stop_job_request_internal(handle, user_data, is_blocking);
  if (err != TH_SUCCESS) {
    TH_LOG_ERR(" Fatal error occured in thread service library");
    return TH_ERROR_FATAL;
  }

  TH_LOG_LOW(" stop_job_request successful");
  return err;
}

