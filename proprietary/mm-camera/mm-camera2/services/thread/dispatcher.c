/*========================================================================
Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */


#include "common.h"
#include "decision.h"
#include "core.h"
#include "profile.h"
#include "dispatcher.h"
#include <string.h>
#include <stdlib.h>

/*========================================================================*/
/* INTERNAL TYPES FOR DISPATCHER */
/*========================================================================*/
typedef struct {
  th_session_t *session;
  unsigned int idx;
} worker_thread_data;


/*========================================================================*/
/* INTERNAL FUNCTIONS */
/*========================================================================*/

static inline th_queue_t *GET_PRIORITY_Q(
  th_session_t *th_session,
  int idx,
  unsigned int mask
)
{
  unsigned int q = (0x1 << idx) & mask;
  return (q ? &th_session->priority_queues[idx] : NULL);
}


/**
 * Function: process_worker_queues
 *
 * Description: Processes the Q-s, on behalf of a worker thread,
 * in order or priority and based on Dispatcher mask
 *
 * Arguments:
 *   @th_session: Session Context
 *   @th_idx: Worker thread ID
 *   @this_run: No of jobs dispatched by this worker thread in this run
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
static th_err_t process_worker_queues(
  th_session_t *th_session,
  unsigned int th_idx,
  int *this_run
)
{
  th_err_t err = TH_SUCCESS, err_prof = TH_SUCCESS;
  int idx = 0, jobs_taken = 0;
  int rc = 0;
  th_dispatcher_t *dispatcher = NULL;
  th_queue_t *th_q = NULL;
  th_job_t *job = NULL;
  th_profiler_t *profiler = NULL;

  (void)th_idx;

  //NULL checks not necessary here, as these are already checked at source
  dispatcher = th_session->dispatcher;
  profiler = th_session->th_profiler;

  err_prof = profiler_start_thread_exec(profiler, th_idx);

  TH_LOG_LOW(" Worker Thread %d dispatched ", th_idx + 1);

  //read the job queues in order of priority
  for (idx = 0; idx < MAX_TH_QUEUES; idx++) {

    th_q = GET_PRIORITY_Q(th_session, idx, dispatcher->flash_mask);
    if (IS_NULL(th_q)) {
      continue;
    }

    //get the first job which is submitted or completed
    job = get_first_job(th_session, th_q, idx);
    while (job != NULL) {
      if (TH_JOB_READY == job->state) {
        //dispatch the job
        dispatch_job(th_session, job, th_session->th_profiler);
        jobs_taken++;
      } else if (TH_JOB_COMPLETED == job->state) {
        //send the stop done
        decision_manager_send_stop_done(th_session, job);
      }
      job = get_first_job(th_session, th_q, idx);
    }
  }

  err_prof = profiler_end_thread_exec(profiler, th_idx, jobs_taken);

  *this_run = jobs_taken;

  TH_LOG_LOW(" Worker Thread %d dispatched %d jobs", th_idx + 1, jobs_taken);

  return TH_SUCCESS;
}


/**
 * Function: process_manager_queue
 *
 * Description: Processes job flush requests
 *
 * Arguments:
 *   @th_session: Session Context
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
static th_err_t process_manager_queue(
  th_session_t *th_session
)
{
  th_err_t err = TH_SUCCESS, err_prof = TH_SUCCESS;
  int idx = 0, jobs_taken = 0;
  int rc = 0;
  th_dispatcher_t *dispatcher = NULL;
  th_queue_t *th_q = NULL;
  manager_payload_t *manager_data = NULL;

  dispatcher = th_session->dispatcher;
  if (IS_NULL(dispatcher)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  TH_LOG_LOW(" Manager Thread processing flush jobs");

  //attend all flush requests
  th_q = &th_session->manager_queue;
  do {
    pthread_mutex_lock(&th_q->q_lock);
    manager_data = (manager_payload_t *)get_from_q(th_q);
    pthread_mutex_unlock(&th_q->q_lock);

    if (manager_data != NULL) {
      if (TH_MANAGER_FLUSH == manager_data->type) {
        TH_LOG_LOW(" Calling flush done from manager thread for job (%s)",
                                           manager_data->data.job_func_name);
        manager_data->data.stop_cb_addr(manager_data->data.user_data);
        if (manager_data->data.msg_sem) {
          //release semaphore
          TH_LOG_LOW("Blocking flush (%s), releasing semaphore",
                                   manager_data->data.job_func_name);
          sem_post(manager_data->data.msg_sem);
        }

        //reset the flush mode flag, so that new jobs can be accepted
        pthread_mutex_lock(manager_data->data.flush_lock);
        manager_data->data.flush_flag = 0;
        pthread_mutex_unlock(manager_data->data.flush_lock);
      }
      free(manager_data);
    }
  } while (manager_data != NULL);

  TH_LOG_LOW(" process_manager_queue successful");
  return TH_SUCCESS;
}


/**
 * Function: common_dispatcher_function
 *
 * Description: Wrapper function which waits on signal to
 * process the next set of jobs from the Priotity Q-s
 *
 * Arguments:
 *   @data: Thread specific data
 *
 * Return values: Standard POSIX thread exit param
 *
 * Notes: Common function for all worker threads
 **/
static void* common_dispatcher_function(void *data)
{
  th_err_t err = TH_SUCCESS, err_prof = TH_SUCCESS;
  unsigned int idx = 0;
  int rc = 0, i = 0;
  int num_sched = 0, num_jobs_taken = 0, this_run_jobs = 0;
  worker_thread_data *th_data = NULL;
  th_session_t *th_session = NULL;
  th_dispatcher_t *dispatcher = NULL;
  th_profiler_t *profiler = NULL;

  if (IS_NULL(data)) {
    TH_LOG_ERR(" Fatal error in common_dispatcher_function");
    return NULL;
  }

  th_data = (worker_thread_data *)data;
  th_session = th_data->session;
  idx = th_data->idx;

  if (IS_NULL(th_session) || idx >= MAX_TH_WORK_THREADS) {
    TH_LOG_ERR(" Fatal error in common_dispatcher_function");
    free(data);
    return NULL;
  }

  dispatcher = th_session->dispatcher;
  if (IS_NULL(dispatcher)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return NULL;
  }

  TH_LOG_HIGH(" Worker thread %d starts", idx + 1);

  profiler = th_session->th_profiler;

  pthread_mutex_lock(&dispatcher->thread_lock);
  //threads have started running
  dispatcher->threads_running = 1;

  while (dispatcher->threads_running) {

    err_prof = profiler_start_thread_wait(profiler, idx);
    //go to sleep if nothing is going on
    while (dispatcher->threads_running && !dispatcher->flash_mask) {
      rc = pthread_cond_wait(&dispatcher->read_ok, &dispatcher->thread_lock);
    }

    err_prof = profiler_end_thread_wait(profiler, idx);
    //received stop request, break out
    if (!dispatcher->threads_running) {
      TH_LOG_HIGH(" Worker thread (%d) received stop request", idx + 1);
      break;
    }

    pthread_mutex_unlock(&dispatcher->thread_lock);
    //go and process the jobs that have come in
    err = process_worker_queues(th_session, idx, &this_run_jobs);

    pthread_mutex_lock(&dispatcher->thread_lock);

    num_sched++;
    if (this_run_jobs) {
      num_jobs_taken++;
    }
    this_run_jobs = 0;
  }
  //let the other threads know as well
  pthread_cond_broadcast(&dispatcher->read_ok);
  pthread_mutex_unlock(&dispatcher->thread_lock);

  TH_LOG_HIGH(" Worker thread (%d) ran %d times, got jobs %d times",
                                           idx + 1, num_sched, num_jobs_taken);
  TH_LOG_HIGH(" Worker thread (%d) exit", idx + 1);

  free(data);

  return NULL;
}


/**
 * Function: manager_thread_function
 *
 * Description: Manager thread currently process all
 * job flush requests
 *
 * Arguments:
 *   @data: Thread specific data
 *
 * Return values: Standard POSIX thread exit param
 *
 * Notes: none
 **/
static void* manager_thread_function(void *data)
{
  th_err_t err = TH_SUCCESS;
  int rc = 0;
  int jobs_taken = 0;
  th_session_t *th_session = NULL;
  th_dispatcher_t *dispatcher = NULL;

  if (IS_NULL(data)) {
    TH_LOG_ERR(" Fatal error in common_dispatcher_function");
    return NULL;
  }

  th_session = (th_session_t *)data;
  dispatcher = th_session->dispatcher;
  if (IS_NULL(dispatcher)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return NULL;
  }

  TH_LOG_HIGH(" Manager thread starts");

  pthread_mutex_lock(&dispatcher->manager_lock);
  dispatcher->manager_running = 1;

  while (dispatcher->manager_running) {
    //wait either for flush request or to stop
    while (dispatcher->manager_running && !dispatcher->manager_pend) {
      rc = pthread_cond_wait(&dispatcher->manager_ok,
                                 &dispatcher->manager_lock);
    }

    if (!dispatcher->manager_running) {
      TH_LOG_HIGH(" Manager thread received stop request");
      break;
    }

    pthread_mutex_unlock(&dispatcher->manager_lock);
    err = process_manager_queue(th_session);
    pthread_mutex_lock(&dispatcher->manager_lock);
    dispatcher->manager_pend = 0;
  }

  pthread_mutex_unlock(&dispatcher->manager_lock);

  TH_LOG_HIGH(" Manager thread exit");
  return NULL;
}


/*========================================================================*/
/* INTERNAL INTERFACE IMPLEMENTATION */
/*========================================================================*/

/**
 * Function: dispatch_job
 *
 * Description: Executes a job and frees up the slot after that
 *
 * Arguments:
 *   @th_session: Session context
 *   @job: Job to dispatch
 *   @profiler: Profiler context
 *
 * Return values: None
 *
 * Notes: none
 **/
void dispatch_job
(
  th_session_t *th_session,
  th_job_t *job,
  th_profiler_t *profiler
)
{
  th_err_t err = TH_SUCCESS;
  int need_trigger = 0;

  TH_LOG_LOW("Job (%s) starts to execute in Thread Library, job slot: %d",
                                          job->job_func_name, job->job_slot);

  //don't care about the error code for now
  err = profiler_finish_job(profiler, job);

  job->job_func(job->data_ptr[0]);
  job->state = TH_JOB_COMPLETED;

  //free the job slot in the master job list
  //don't care about the error code for now
  err = decision_manager_set_one_empty_slot(th_session, job, 1);

  (void)profiler;//prevent compiler werror, when profiler is not enabled
  TH_LOG_LOW(" dispatch_job successful");
  return;
}


/**
 * Function: init_dispatcher
 *
 * Description: Initializes the dispatcher and sets the Worker
 * threads and Manager thread running
 *
 * Arguments:
 *   @th_session: Session context
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
th_err_t init_dispatcher(th_session_t *th_session)
{
  th_err_t err = TH_SUCCESS;
  unsigned int j = 0;
  int rc = 0;
  th_dispatcher_t *dispatcher = NULL;
  worker_thread_data *th_data[MAX_TH_WORK_THREADS];
  pthread_attr_t attr;

  dispatcher = th_session->dispatcher;
  if (IS_NULL(dispatcher)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  dispatcher->manager_running = 0;
  dispatcher->threads_running = 0;
  dispatcher->manager_pend = 0;
  dispatcher->flash_mask = 0;

  pthread_cond_init(&dispatcher->manager_ok, NULL);
  pthread_mutex_init(&dispatcher->manager_lock, NULL);
  pthread_cond_init(&dispatcher->read_ok, NULL);
  pthread_mutex_init(&dispatcher->thread_lock, NULL);

  //create worker threads first
  for (j = 0; j < MAX_TH_WORK_THREADS; j++) {
    dispatcher->work_threads[j].tid = j;
    dispatcher->work_threads[j].worker_thrd_func = common_dispatcher_function;

    //to be freed in the individual worker thread routines
    th_data[j] = (worker_thread_data *)malloc(sizeof(worker_thread_data));
    if (IS_NULL(th_data[j])) {
      TH_LOG_ERR(" Fatal error in init_dispatcher");
      //local clean-up
      pthread_cond_destroy(&dispatcher->read_ok);
      pthread_mutex_destroy(&dispatcher->thread_lock);
      return TH_ERROR_FATAL;
    }
    th_data[j]->idx = j;
    th_data[j]->session = th_session;

    rc = pthread_create(&dispatcher->work_threads[j].worker_thread,
                        NULL,
                        dispatcher->work_threads[j].worker_thrd_func,
                        th_data[j]);
    if (rc != 0) {
      TH_LOG_ERR(" Fatal error in pthread_create");
      //local clean-up
      pthread_cond_destroy(&dispatcher->read_ok);
      pthread_mutex_destroy(&dispatcher->thread_lock);
      return TH_ERROR_FATAL;
    }
  }

  //then create manager thread
  dispatcher->manager_thread.worker_thrd_func = manager_thread_function;
  rc = pthread_create(&dispatcher->manager_thread.worker_thread,
                      NULL,
                      dispatcher->manager_thread.worker_thrd_func,
                      th_session);
  if (rc != 0) {
    TH_LOG_ERR(" Fatal error in pthread_create");
    //local clean-up
    pthread_cond_destroy(&dispatcher->read_ok);
    pthread_mutex_destroy(&dispatcher->thread_lock);
    pthread_cond_destroy(&dispatcher->manager_ok);
    pthread_mutex_destroy(&dispatcher->manager_lock);
    return TH_ERROR_FATAL;
  }

  TH_LOG_MED(" init_dispatcher successful");
  return err;
}


/**
 * Function: deinit_dispatcher
 *
 * Description: De-inits the Dispatcher and joins all processing threads
 *
 * Arguments:
 *   @th_session: Session context
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
th_err_t deinit_dispatcher(th_session_t *th_session)
{
  th_err_t err = TH_SUCCESS;
  int i = 0;
  th_dispatcher_t *dispatcher = th_session->dispatcher;

  pthread_mutex_lock(&dispatcher->thread_lock);
  dispatcher->threads_running = 0;//worker threads, don't look for any more jobs
  pthread_cond_broadcast(&dispatcher->read_ok);//broadcast the message
  pthread_mutex_unlock(&dispatcher->thread_lock);

  //wait for the worker threads to join
  TH_LOG_HIGH(" Waiting for worker threads to join");
  for (i = 0; i < MAX_TH_WORK_THREADS; i++) {
    pthread_join(dispatcher->work_threads[i].worker_thread, NULL);
  }
  TH_LOG_HIGH(" All worker threads joined");

  pthread_cond_destroy(&dispatcher->read_ok);
  pthread_mutex_destroy(&dispatcher->thread_lock);

  pthread_mutex_lock(&dispatcher->manager_lock);
  dispatcher->manager_running = 0;
  pthread_cond_signal(&dispatcher->manager_ok);
  pthread_mutex_unlock(&dispatcher->manager_lock);

  //wait for manager thread to join
  TH_LOG_HIGH(" Waiting for manager thread to join");
  pthread_join(dispatcher->manager_thread.worker_thread, NULL);
  TH_LOG_HIGH(" Manager thread joined");

  pthread_cond_destroy(&dispatcher->manager_ok);
  pthread_mutex_destroy(&dispatcher->manager_lock);

  TH_LOG_MED(" deinit_dispatcher successful");
  return err;
}


/**
 * Function: trigger_dispatcher
 *
 * Description: Triggers the worker threads, if processing is needed
 * for a job in a specific priority Q
 *
 * Arguments:
 *   @th_session: Session context
 *   @trigger_mask:
 *     Bit 0 = DISP_FLASH_RED
 *     Bit 1 = DISP_FLASH_ORANGE
 *     Bit 2 = DISP_FLASH_GREEN
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
th_err_t trigger_dispatcher(
  th_session_t *th_session,
  unsigned int trigger_mask
)
{
  th_err_t err = TH_SUCCESS;
  int i = 0;
  th_dispatcher_t *dispatcher = NULL;

  dispatcher = th_session->dispatcher;

  pthread_mutex_lock(&dispatcher->thread_lock);

  //If the trigger mask was already ON for a
  //particular COLOR, no need to trigger again
  trigger_mask &= 0x7;
  if (dispatcher->flash_mask & DISP_FLASH_RED) {
    trigger_mask &= 0x6;//0b110
  }
  if (dispatcher->flash_mask & DISP_FLASH_ORANGE) {
    trigger_mask &= 0x5;//0b101
  }
  if (dispatcher->flash_mask & DISP_FLASH_GREEN) {
    trigger_mask &= 0x3;//0b011
  }

  if (trigger_mask) {
    dispatcher->flash_mask |= trigger_mask;
    pthread_cond_broadcast(&dispatcher->read_ok);
  }

  pthread_mutex_unlock(&dispatcher->thread_lock);

  TH_LOG_LOW(" trigger_dispatcher successful, flash mask:%x",
                                        dispatcher->flash_mask);
  return err;
}

