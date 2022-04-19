/**********************************************************************
*  Copyright (c) 2015-2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include "img_thread.h"

/** img_thread_start_job
 *
 *  Arguments:
 *    @arg: thread obj
 *
 *  Return:
 *    NULL
 *
 *  Description:
 *     Executes jobs in the threads job queue
 *
 **/
void *img_thread_start_job(void *arg)
{
  struct timespec *start_time = NULL, *end_time = NULL;
  int lrc = 0;

  if (NULL == arg) {
    IDBG_ERROR("%s %d: Invalid input. cannot start thread", __func__, __LINE__);
    return NULL;
  }
  img_thread_t *p_th = (img_thread_t *)arg;
  pthread_setname_np(p_th->threadid,"CAM_imgTh");

  IDBG_LOW("%s %d:] pthread %p going to signal",  __func__,  __LINE__,
    (void *)p_th->threadid);
  pthread_mutex_lock(&p_th->mutex);
  p_th->is_ready = TRUE;
  pthread_cond_signal(&p_th->cond);
  pthread_mutex_unlock(&p_th->mutex);
  IDBG_LOW("%s %d:] pthread %p DONE signal",  __func__,  __LINE__,
    (void *)p_th->threadid);

  for (;;) {
    pthread_mutex_lock(&p_th->mutex);
    IDBG_LOW("%s %d:] p_th->int_q.count %d , p_th->exit_flag %d",  __func__,
      __LINE__,p_th->int_q.count, p_th->exit_flag);
    while ((p_th->int_q.count == 0) && (!p_th->exit_flag)) {
      p_th->is_active = FALSE;
      IDBG_LOW("%s %d:] pthread %p in wait state",  __func__,  __LINE__,
        (void *)p_th->threadid);
      pthread_cond_wait(&p_th->cond, &p_th->mutex);
    }


    if (p_th->exit_flag) {
      break;
    }
    //Get the job from the queue
    img_thread_job_t *p_job = img_q_dequeue(&p_th->int_q);
    if (NULL == p_job) {
      IDBG_ERROR("%s %d:] img_q_dequeue returned NULL ",  __func__,  __LINE__ );
      return NULL;
    }

    //reset job id
    if (FALSE == p_job->job_params->delete_on_completion) {
      p_job->job_id = 0;
    }

    p_job->status = IMG_JOB_EXECUTING;
    IDBG_MED("%s %d: IMG_JOB_EXECUTING for job %d, p_job %p."
      "set thread %p active", __func__, __LINE__, p_job->job_id, p_job,
      (void *)p_th->threadid);
    p_th->is_active = TRUE;
    pthread_mutex_unlock(&p_th->mutex);

    //Get start time
    lrc = clock_gettime(CLOCK_REALTIME, &p_job->start_time);
    start_time = &p_job->start_time;
    if (lrc < 0)
      IDBG_ERROR("%s:%d] Error getting start time %d", __func__, __LINE__, lrc);
    IDBG_MED("%s %d: jobid %d start time is %llu", __func__, __LINE__,
      p_job->job_id, GET_TIME_IN_MS(start_time));

    //Execute the task
    p_job->job_params->execute(p_job->job_params->args);

    //Get end time
    end_time = &p_job->end_time;
    lrc = clock_gettime(CLOCK_REALTIME, &p_job->end_time);
    if (lrc < 0)
      IDBG_ERROR("%s:%d] Error getting start time %d", __func__, __LINE__, lrc);
    IDBG_MED("%s %d: jobid %d end time is %llu", __func__, __LINE__,
      p_job->job_id, GET_TIME_IN_MS(end_time));

    pthread_mutex_lock(&p_th->mutex);
    IDBG_MED("%s %d: IMG_JOB_DONE for job %d, p_job %p",
      __func__, __LINE__, p_job->job_id, p_job);
    p_job->status = IMG_JOB_DONE;
    p_th->is_active = FALSE;
    pthread_mutex_unlock(&p_th->mutex);
    //Job done - fetch next job
    img_thread_mgr_job_done(p_job->job_id);

  }
  return NULL;
}

/** img_thread_create
 *
 *  Arguments:
 *     @p_th: thread structure
 *     @p_sh_q: Shared queue
 *
 *  Return:
 *     0 on success and -1 on failure
 *
 *  Description:
 *      Create a new thread
 *
 **/
int img_thread_create(img_thread_t *p_th, img_queue_t *p_sh_q)
{
  int lrc = 0;
  if (NULL == p_th) {
    IDBG_ERROR("%s %d:] Invalid input params",  __func__,  __LINE__);
    return -1;
  }

  //Initialize internal queue
  img_q_init(&p_th->int_q, "Internal_q");

  //Initialize thread mutex
  if (pthread_mutex_init(&p_th->mutex, NULL)) {
    IDBG_ERROR("%s %d: Mutex initialization failed", __func__, __LINE__);
    goto MUTEX_ERROR;
  }
  //Initialize the thread cond
  if (pthread_cond_init(&p_th->cond,  NULL)) {
    IDBG_ERROR("%s %d: Mutex initialization failed", __func__, __LINE__);
    goto COND_ERROR;
  }

  pthread_mutex_lock(&p_th->mutex);

  p_th->p_sh_q = p_sh_q;
  p_th->core_affinity = IMG_CORE_ARM;
  p_th->client_id = 0;
  p_th->exit_flag = 0;
  p_th->is_ready = 0;
  lrc = pthread_create(&p_th->threadid, NULL, img_thread_start_job, p_th);
  if (lrc < 0) {
    IDBG_ERROR("%s %d: pthread creation failed %d", __func__, __LINE__, errno);
    pthread_mutex_unlock(&p_th->mutex);
    goto THREAD_ERROR;
  }
  if (FALSE == p_th->is_ready) {
    IDBG_LOW("%s %d: before wait", __func__, __LINE__);
    pthread_cond_wait(&p_th->cond, &p_th->mutex);
  }
  IDBG_LOW("%s %d: after wait ,  thread ready now :)", __func__, __LINE__);
  pthread_mutex_unlock(&p_th->mutex);
  return lrc;

THREAD_ERROR:
  pthread_cond_destroy(&p_th->cond);
COND_ERROR:
  pthread_mutex_destroy(&p_th->mutex);
MUTEX_ERROR:
  img_q_deinit(&p_th->int_q);
  return -1;
}

/** img_thread_join
 *
 *  Arguments:
 *     @p_th: thread structure
 *
 *  Return:
 *     None
 *
 *  Description:
 *      Join the thread
 *
 **/
static void img_thread_join(img_thread_t *p_th)
{
  if (NULL == p_th) {
    IDBG_ERROR("%s %d:] Invalid input params",  __func__,  __LINE__);
    return;
  }
  if (!(IsSelf(p_th->threadid))) {
    pthread_join(p_th->threadid, NULL);
  } else {
    IDBG_WARN("%s %d: Cannot call join on self %p", __func__, __LINE__,
      (void *)p_th->threadid);
  }
  return;
}

/** img_thread_destroy
 *
 *  Arguments:
 *     @p_th: thread structure
 *
 *  Return:
 *     None
 *
 *  Description:
 *      Destroy a thread
 *
 **/
void img_thread_destroy(img_thread_t *p_th)
{
  if (NULL == p_th) {
    IDBG_ERROR("%s %d:] Invalid input params",  __func__,  __LINE__);
    return;
  }
  pthread_mutex_lock(&p_th->mutex);
  //Set the exit flag
  p_th->exit_flag = 1;
  //Wake up the thread
  pthread_cond_signal(&p_th->cond);
  pthread_mutex_unlock(&p_th->mutex);
  //Join the thread
  img_thread_join(p_th);
  p_th->threadid = -1;
  pthread_mutex_destroy(&p_th->mutex);
  pthread_cond_destroy(&p_th->cond);
  img_q_deinit(&p_th->int_q);
  return;
}

/** img_thread_send
 *
 *  Arguments:
 *     @p_th: thread structure
 *     @p_data: Data to be executed by the thread
 *
 *  Return:
 *     None
 *
 *  Description:
 *      Enqueue the data to the thread queue/shared queue
 *
 **/
void img_thread_send(img_thread_t *p_th, img_thread_job_t *p_data)
{
  if ((NULL == p_th) || (NULL == p_data)) {
    IDBG_ERROR("%s %d: Invalid input", __func__, __LINE__);
    return;
  }
  img_q_enqueue(&p_th->int_q, p_data);
  pthread_cond_signal(&p_th->cond);

  return;
}

/** img_thread_reset
 *
 *  Arguments:
 *     @p_th: thread structure
 *
 *  Return:
 *     None
 *
 *  Description:
 *      Reset the thread to orig state and make it available for
 *     the pool
 *
 **/
void img_thread_reset(img_thread_t *p_th)
{
  if (NULL == p_th) {
    IDBG_ERROR("%s %d: Invalid input", __func__, __LINE__);
    return;
  }
  img_q_flush(&p_th->int_q);
  p_th->client_id = 0;
  p_th->core_affinity = IMG_CORE_ARM;
  p_th->is_active = FALSE;

  return;
}
