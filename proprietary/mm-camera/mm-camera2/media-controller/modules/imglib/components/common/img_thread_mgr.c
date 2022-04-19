/**********************************************************************
*  Copyright (c) 2014-2017 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/
#include "img_thread.h"

/*Global handle to the sigle instance of the thread pool*/
static img_thread_pool_t *g_thread_pool;

/*Gobal job count used to generate the job id*/
static uint32_t g_job_count;

/*Global mutex to syncronize the creation/destruction of thread pool*/
static pthread_mutex_t g_thread_pool_lock = PTHREAD_MUTEX_INITIALIZER;

/** img_thread_mgr_create_pool
 *
 *  Arguments:
 *     None
 *
 *  Return:
 *     0 on success and -1 on failure
 *
 *  Description:
 *      Creates a threadpool will the specified number
 *   of threads.
 *
 **/
int img_thread_mgr_create_pool()
{
  int i = 0;
  int num_of_threads = MAX_THREAD_COUNT;

  pthread_mutex_lock(&g_thread_pool_lock);
  if (g_thread_pool != NULL) {
    g_thread_pool->ref_count++;
    IDBG_HIGH("Thread pool already created ref_count %d, "
      "created %d threads, available=%d, current request %d",
      g_thread_pool->ref_count, g_thread_pool->num_of_threads,
      g_thread_pool->num_of_avail_threads, num_of_threads);

    pthread_mutex_unlock(&g_thread_pool_lock);
    return IMG_SUCCESS;
  }
  //Create Thread pool
  g_thread_pool = (img_thread_pool_t *)calloc(1, sizeof(img_thread_pool_t));
  if (NULL == g_thread_pool) {
    IDBG_ERROR("No memory to create new threadpool");
    pthread_mutex_unlock(&g_thread_pool_lock);
    return IMG_ERR_GENERAL;
  }
  g_thread_pool->ref_count = 0;
  g_thread_pool->num_of_clients = 0;
  g_thread_pool->total_client_count = 0;

  g_thread_pool->thread =
    (img_thread_t *)malloc(sizeof(img_thread_t) * (uint32_t)num_of_threads);
  if (NULL == g_thread_pool->thread) {
    IDBG_ERROR("No memory to create threads");
    goto THREAD_ALLOC_ERROR;
  }

  //Initialize mutexes
  if (pthread_mutex_init(&g_thread_pool->pool_mutex, NULL)) {
    IDBG_ERROR("Mutex initialization failed");
    goto MUTEX_INIT_ERROR;
  }

  if (pthread_cond_init(&g_thread_pool->pool_cond, NULL)) {
    IDBG_ERROR("Mutex initialization failed");
    goto COND_INIT_ERROR;
  }

  //Initialize the job list
  g_thread_pool->job_list = img_list_create();
  if (!g_thread_pool->job_list) {
    goto JOB_LIST_FAIL;
  }
  //Create the threads in the thread pool
  for (i = 0; i < num_of_threads; i++) {
    if (img_thread_create(&(g_thread_pool->thread[i]), NULL)) {
      IDBG_ERROR("Mutex initialization failed");
      goto THREAD_CREATE_ERROR;
     }
    g_thread_pool->num_of_threads++;
    g_thread_pool->num_of_avail_threads++;
  }

  g_thread_pool->ref_count++;
  IDBG_HIGH("ref count %d", g_thread_pool->ref_count);
  pthread_mutex_unlock(&g_thread_pool_lock);
  return IMG_SUCCESS;

THREAD_CREATE_ERROR:
  img_list_free(g_thread_pool->job_list);
  for (i = 0; i < g_thread_pool->num_of_threads; i++) {
    img_thread_destroy(&g_thread_pool->thread[i]);
  }
JOB_LIST_FAIL:
  pthread_cond_destroy(&g_thread_pool->pool_cond);
COND_INIT_ERROR:
  pthread_mutex_destroy(&g_thread_pool->pool_mutex);
MUTEX_INIT_ERROR:
  free(g_thread_pool->thread);
THREAD_ALLOC_ERROR:
  free(g_thread_pool);
  pthread_mutex_unlock(&g_thread_pool_lock);
  return IMG_ERR_GENERAL;

}

/** img_thread_destroy_pool
 *
 *  Arguments:
 *   None
 *
 *  Return: None
 *
 *  Description:
 *      Destroy thread pool
 *
 **/
void img_thread_mgr_destroy_pool()
{
  int i = 0;
  if (!g_thread_pool) {
    IDBG_ERROR("%s %d: Thread pool does not exist", __func__, __LINE__);
    return;
  }
  pthread_mutex_lock(&g_thread_pool_lock);
  g_thread_pool->ref_count--;

  /*Destroy thread pool only if the ref count is 0*/
  if (g_thread_pool->ref_count == 0) {
    if (g_thread_pool->job_list) {
      img_list_free(g_thread_pool->job_list);
    }
    pthread_mutex_destroy(&g_thread_pool->pool_mutex);
    pthread_cond_destroy(&g_thread_pool->pool_cond);

    if (g_thread_pool->thread) {
      for (i = 0; i < g_thread_pool->num_of_threads; i++) {
        img_thread_destroy(&g_thread_pool->thread[i]);
      }
      free(g_thread_pool->thread);
      g_thread_pool->thread = NULL;
    }
    free(g_thread_pool);
    g_thread_pool = NULL;
  }
  pthread_mutex_unlock(&g_thread_pool_lock);
  return;
}

/** img_thread_mgr_find_job_by_client_id
 *
 *  Arguments:
 *   @p_job: Job Object
 *   @userdata: Client_id associated with the job
 *
 *  Return:
 *    TRUE: If the arguments are the same
 *    FALSE otherwise
 *
 *  Description:
 *      Helper function to find a job in the list by client id
 *
 **/
static int img_thread_mgr_find_job_by_client_id(void *p_nodedata,
  void *p_userdata)
{
  img_thread_job_t *p_job = NULL;
  uint32_t *client_id = NULL;

  if ((NULL == p_nodedata) || (NULL == p_userdata)) {
    IDBG_ERROR("%s %d:Invalid input",  __func__,  __LINE__);
    return FALSE;
  }

  p_job = (img_thread_job_t *)p_nodedata;
  client_id = (uint32_t *)p_userdata;

  if (p_job->job_params->client_id == *client_id) {
    IDBG_MED("%s %d: job_id is %d, client_id is %d, status %d",
      __func__, __LINE__, p_job->job_id, *client_id, p_job->status);
    return TRUE;
  }

  return FALSE;
}

/** img_thread_mgr_remove_job_by_client_id
 *
 *  Arguments:
 *   @p_job: Job Object
 *   @userdata: Client_id associated with the job
 *
 *  Return:
 *    TRUE: If the arguments are the same
 *    FALSE otherwise
 *
 *  Description:
 *      Helper function to find a job in the list by client id and
 *      free data
 *
 **/
static int img_thread_mgr_remove_job_by_client_id(void *p_nodedata,
  void *p_userdata)
{
  img_thread_job_t *p_job = NULL;
  int job_found = FALSE;

  if ((NULL == p_nodedata) || (NULL == p_userdata)) {
    IDBG_ERROR("%s %d:Invalid input",  __func__,  __LINE__);
    return FALSE;
  }

  job_found = img_thread_mgr_find_job_by_client_id(p_nodedata, p_userdata);
  if(job_found) {
    p_job = (img_thread_job_t *)p_nodedata;
    //If job is executing, wait for completion
    if (p_job->status == IMG_JOB_EXECUTING) {
      IDBG_ERROR("%s %d: Job %d is currently executing."
        "Cannot remove Job.", __func__, __LINE__, p_job->job_id);
       return FALSE;
    }
    free(p_job->job_params);
    p_job->job_params = NULL;
  }
  return job_found;
}

/** img_thread_mgr_get_next_job
 *
 *  Arguments:
 *   @p_job: Job obj
 *   @p_userdata: Job ID
 *
 *  Return:
 *    TRUE: If the arguments are the same
 *    FALSE otherwise
 *
 *  Description:
 *      Helper function to find a job in the list by job id
 *
 **/
static int img_thread_mgr_find_job_by_id(void *p_nodedata, void *p_userdata)
{
  img_thread_job_t *p_job = NULL;
  uint32_t *job_id = NULL;

  if ((NULL == p_nodedata) || (NULL == p_userdata)) {
    IDBG_ERROR("%s %d:Invalid input",  __func__,  __LINE__);
    return FALSE;
  }

  p_job = (img_thread_job_t *)p_nodedata;
  job_id = (uint32_t *)p_userdata;

  if (p_job->job_id == *job_id) {
    return TRUE;
  }

  return FALSE;
}

/** img_thread_mgr_get_job
 *
 *  Arguments:
 *   @p_nodedata: Job obj
 *   @p_userdata: Job obj specified by the user
 *
 *  Return:
 *    TRUE: If the arguments are the same
 *    FALSE otherwise
 *
 *  Description:
 *      Helper function to find a job in the job list that
 *      matches the specified job obj
 *
 **/
static int img_thread_mgr_get_job(void *p_nodedata, void *p_userdata)
{
  if ((NULL == p_nodedata) || (NULL == p_userdata)) {
    IDBG_ERROR("%s %d:Invalid input",  __func__,  __LINE__);
    return FALSE;
  }
  if ((img_thread_job_t*)p_nodedata == (img_thread_job_t*)p_userdata) {
    return TRUE;
  }
  return FALSE;
}

/** img_thread_mgr_schedule_ready_job
 *
 *  Arguments:
 *   @p_nodedata: Job obj
 *   @p_userdata: Userdata associated with the list. can be
 *              NULL.
 *
 *  Return: FALSE on invalid and failed operations
 *          TRUE otherwise
 *
 *  Description:
 *      Fetch a job from the job list that is ready to be
 *      scheduled and assign it to an appropriate thread from
 *      the thread pool
 *
 **/
static int img_thread_mgr_schedule_ready_job(void *p_nodedata, void *p_userdata)
{
  IMG_UNUSED(p_userdata);

  img_thread_job_t *p_job = NULL, *p_dep_job = NULL;
  int i = 0;
  int dep_jobs_done = 0;

  if (NULL == p_nodedata) {
    IDBG_ERROR("%s %d:Invalid input",  __func__,  __LINE__);
    return FALSE;
  }
  p_job = p_nodedata;

  //Only check jobs that are not schedules yet or not complete
  if (p_job->status == IMG_JOB_INIT) {
    //Check if the dependant jobs are complete
    for (i = 0; i < p_job->job_params->dep_job_count; i++ ) {
      p_dep_job = (img_thread_job_t *)img_list_find(g_thread_pool->job_list,
        img_thread_mgr_find_job_by_id, &(p_job->job_params->dep_job_ids[i]));
      //If any dependant job is present, no ned to search further
      if (p_dep_job) {
        if (p_dep_job->status == IMG_JOB_DONE)
          dep_jobs_done++;
      }
    }
    //If no dependant job exists, find appropriate thread to schedule
    if (dep_jobs_done == p_job->job_params->dep_job_count) {
      for (i = 0; i < g_thread_pool->num_of_threads; i++) {
        if ((g_thread_pool->thread[i].client_id == p_job->job_params->client_id)
          && (g_thread_pool->thread[i].core_affinity ==
          p_job->job_params->core_affinity) &&
          (g_thread_pool->thread[i].is_active == FALSE) &&
          (p_job->status != IMG_JOB_DONE)) {
          IDBG_HIGH("%s %d: Job %d scheduled on Thread %p for Client %d. current status %d",
            __func__, __LINE__, p_job->job_id,
            (void*)g_thread_pool->thread[i].threadid,
            g_thread_pool->thread[i].client_id, p_job->status);
          img_thread_send(&g_thread_pool->thread[i], p_job);

          if (p_job->status != IMG_JOB_DONE) {
            p_job->status = IMG_JOB_SCHEDULED;
            IDBG_LOW("%s %d: IMG_JOB_SCHEDULED for job %d",
              __func__, __LINE__, p_job->job_id);
          }
          break;
        }
      }
    } else {
      IDBG_MED("%s %d: Job %d not ready to be scheduled", __func__, __LINE__,
        p_job->job_id);
    }
  }
  return TRUE;
}

/** img_thread_mgr_fetch_job
 *
 *  Arguments:
 *   None
 *
 *  Return:
 *    None
 *
 *  Description:
 *      Traverse through the job list to find the next job
 *
 **/
void img_thread_mgr_fetch_job()
{
  //Traverse the list and schedule jobs that are ready
  pthread_mutex_lock(&g_thread_pool->pool_mutex);
  img_list_traverse(g_thread_pool->job_list, img_thread_mgr_schedule_ready_job,
    NULL);
  pthread_mutex_unlock(&g_thread_pool->pool_mutex);
  return;
}

/** img_thread_mgr_job_done
 *
 *  Arguments:
 *  @job_id: unique job id
 *
 *  Return:
 *    None
 *
 *  Description:
 *      Signal the waiting threads, get the job to be
 *      removed from the list and fetch the next job
 *
 **/
void img_thread_mgr_job_done(uint32_t job_id)
{
  img_thread_job_t *p_job;

  pthread_mutex_lock(&g_thread_pool->pool_mutex);

  pthread_cond_signal(&g_thread_pool->pool_cond);

  if (job_id <= 0) {
    IDBG_MED("%s:%d] Invalid job id or delete_on_completion not set",
      __func__, __LINE__);
    goto error;
  }

  // check if the job exists or not
  p_job = (img_thread_job_t *)img_list_find(g_thread_pool->job_list,
    img_thread_mgr_find_job_by_id, &job_id);
  if (!p_job) {
    IDBG_MED("%s:%d] Job not available, must have already removed",
      __func__, __LINE__);
    goto error;
  }

  if (!p_job->job_params) {
    IDBG_ERROR("%s:%d] Error, Invalid job params!!!",__func__, __LINE__);
    goto error;
  }

  if (TRUE == p_job->job_params->delete_on_completion) {
    IDBG_MED("%s:%d] mgr Deleting Job Memory %d , dont use after this",
      __func__, __LINE__, p_job->job_params->delete_on_completion);
    img_thread_mgr_remove_job(p_job);
  } else {
    IDBG_WARN("%s:%d] Warning!!! Job not removed from list",
      __func__, __LINE__);
  }

error:
  pthread_mutex_unlock(&g_thread_pool->pool_mutex);
  img_thread_mgr_fetch_job();

  return;
}

/** img_thread_mgr_remove_job
 *
 *  Arguments:
 *   @p_job: Job to be removed from list
 *
 *  Return:
 *    0 on success, -1 on failure
 *
 *  Description:
 *      Remove non-executing job from the job list
 *
 **/
int img_thread_mgr_remove_job(img_thread_job_t *p_job)
{
  int rc = IMG_SUCCESS;
  if (!p_job) {
    IDBG_ERROR("%s %d; Invalid Input", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }
  if (p_job->status != IMG_JOB_EXECUTING) {
    IDBG_MED("%s %d: Remove job %d from the list", __func__, __LINE__,
      p_job->job_id);
    free(p_job->job_params);
    p_job->job_params = NULL;
    rc = img_list_remove(g_thread_pool->job_list, img_thread_mgr_get_job, p_job);
  } else {
    IDBG_WARN("%s %d: Job %d is currently executing."
      "Cannot remove job", __func__, __LINE__, p_job->job_id);
    rc = IMG_ERR_BUSY;
  }
  return rc;
}


/** img_thread_mgr_reset_job_list
 *
 *  Arguments:
 *   @client_d: Client ID who's jobs need to be removed from the
 *            list
 *
 *  Return:
 *    Standard Error Values
 *
 *  Description:
 *      Remove all jobs associated with the given client_id from
 *      the list
 *
 **/
static int img_thread_mgr_reset_job_list(uint32_t client_id)
{
  int rc = IMG_SUCCESS;
  pthread_mutex_lock(&g_thread_pool->pool_mutex);
  IDBG_MED("%s %d: Remove jobs for client %d from the list", __func__, __LINE__,
    client_id);
  rc = img_list_remove(g_thread_pool->job_list,
    img_thread_mgr_remove_job_by_client_id, &client_id);
  pthread_mutex_unlock(&g_thread_pool->pool_mutex);
  return rc;

}

/** img_thread_mgr_schedule_job
 *
 *  Arguments:
 *   @job_params: Job parameters
 *
 *  Return: unique jobid
 *          -0 on failure
 *
 *  Description:
 *    Schedules a new job with the thread pool
 *
 **/
uint32_t img_thread_mgr_schedule_job(img_thread_job_params_t *job_params)
{
  img_thread_job_t *p_job = NULL;
  uint32_t job_id;
  if (NULL == job_params) {
    IDBG_ERROR("%s %d: Invalid params", __func__, __LINE__);
    return 0;
  }
  if ((NULL == job_params->execute) || (job_params->client_id == 0) ||
    (g_thread_pool->total_client_count > MAX_NUM_CLIENTS)) {
    IDBG_ERROR("Invalid job params %p, clientid %d, total_client_count %d",
      job_params->execute, job_params->client_id,
      g_thread_pool->total_client_count);
    return 0;
  }

  if (job_params->dep_job_count > MAX_DEPENDANT_JOBS) {
    IDBG_WARN("%s %d: Dependant job count %d exceeds MAX %d", __func__,
      __LINE__, job_params->dep_job_count, MAX_DEPENDANT_JOBS);
  }
  pthread_mutex_lock(&g_thread_pool->pool_mutex);

  p_job = (img_thread_job_t *) malloc(sizeof(img_thread_job_t));
  if (!p_job) {
    IDBG_ERROR("%s %d: Cannot allocate memory", __func__, __LINE__);
    pthread_mutex_unlock(&g_thread_pool->pool_mutex);
    return 0;
  }

  //Make a copy of the job params
  p_job->job_params = (img_thread_job_params_t *)
    malloc(sizeof(img_thread_job_params_t));
  if(!p_job->job_params) {
    IDBG_ERROR("%s %d: Cannot allocate memory for job params", __func__, __LINE__);
    free(p_job);
    pthread_mutex_unlock(&g_thread_pool->pool_mutex);
    return 0;
  }
  memcpy(p_job->job_params, job_params, sizeof(img_thread_job_params_t));

  g_job_count++;
  job_id = CREATE_JOB_ID(job_params->client_id, g_job_count);
  p_job->job_id = job_id;
  p_job->status = IMG_JOB_INIT;

  //Add job to job list
  img_list_append(g_thread_pool->job_list, p_job);
  pthread_mutex_unlock(&g_thread_pool->pool_mutex);
  img_thread_mgr_fetch_job();

  return job_id;
}

/** img_thread_mgr_reserve_threads
 *
 *  Arguments:
 *   @num_of_threads: Num of threads required
 *
 *  Return: Unique clientid
 *          -1 on failure
 *
 *  Description:
 *    Reserve the number threads from the threadpool that needs
 *    to be used to schedule all the jobs by the client
 *
 **/
uint32_t img_thread_mgr_reserve_threads(int num_of_threads,
  img_core_type_t thread_affinity[])
{
  uint32_t client_id;
  int i = 0, affinity_index = 0;

  if ((num_of_threads <= 0) || (!thread_affinity)) {
    IDBG_ERROR("%s %d: Invalid input num_of_threads = %d",
      __func__, __LINE__, num_of_threads);
    return 0;
  }
  if (num_of_threads > g_thread_pool->num_of_avail_threads) {
    IDBG_ERROR("%s %d: Num pf threads requested exceeds available num of"
      "threads", __func__,__LINE__);
    return 0;
  }
  //ToDo: check for cyclic dependancies

  g_thread_pool->total_client_count++;

  pthread_mutex_lock(&g_thread_pool_lock);
  client_id = ++g_thread_pool->num_of_clients;
  for (i = 0; i < g_thread_pool->num_of_threads; i++) {
    if (g_thread_pool->thread[i].client_id <= 0) {
      g_thread_pool->thread[i].client_id = client_id;
      g_thread_pool->thread[i].core_affinity = thread_affinity[affinity_index];
      affinity_index++;
      g_thread_pool->num_of_avail_threads--;
    }
    if (affinity_index == num_of_threads)
      break;
  }
  pthread_mutex_unlock(&g_thread_pool_lock);
  return client_id;
}

/** img_thread_mgr_unreserve_threads
 *
 *  Arguments:
 *   @client_id: Client ID
 *
 *  Return:
 *    Standard Error values
 *
 *  Description:
 *    Unreserve the threads from the threadpool that were
 *    reserved by the client
 *
 **/
int img_thread_mgr_unreserve_threads(uint32_t client_id)
{
  int i = 0;
  int rc = IMG_SUCCESS;
  if ((client_id == 0) ||
    (g_thread_pool->total_client_count > MAX_NUM_CLIENTS)) {
    IDBG_ERROR("%s %d: Invalid client id %d", __func__, __LINE__, client_id);
    return IMG_ERR_INVALID_INPUT;
  }

  g_thread_pool->total_client_count--;

  IDBG_MED("%s %d: Unreserving client %d from the thread pool",
    __func__, __LINE__, client_id);
  pthread_mutex_lock(&g_thread_pool_lock);
  for (i = 0; i < g_thread_pool->num_of_threads; i++) {
    if (g_thread_pool->thread[i].client_id == client_id) {
      rc = img_thread_mgr_reset_job_list(client_id);
      if (rc == IMG_SUCCESS) {
          img_thread_reset(&g_thread_pool->thread[i]);
          g_thread_pool->num_of_avail_threads++;
      } else {
        IDBG_ERROR("%s %d: Cannot remove jobs, Unreserve failed for client %d",
          __func__, __LINE__, client_id);
      }
    }
  }
  pthread_mutex_unlock(&g_thread_pool_lock);
  return rc;
}

/** img_thread_mgr_get_time_of_job
 *
 *  Arguments:
 *   @job_id: Job ID
 *
 *  Return:
 *    Time in milliseconds; 0 on failure
 *
 *  Description:
 *    Get the time taken by a job specified by the jobid
 *
 **/
uint64_t img_thread_mgr_get_time_of_job(uint32_t job_id)
{
  uint64_t time = 0;
  img_thread_job_t *p_job = NULL;
  struct timespec *start_time = NULL, *end_time = NULL;

  //Get job
  p_job = (img_thread_job_t *)img_list_find(g_thread_pool->job_list,
    img_thread_mgr_find_job_by_id, &job_id);
  if (!p_job) {
    IDBG_ERROR("%s %d: Cannot find job %d", __func__, __LINE__, job_id);
    return 0;
  }

  //Get Start and end time
  start_time = &p_job->start_time;
  end_time = &p_job->end_time;
  if(start_time > end_time) {
    IDBG_ERROR("%s %d: Cannot get time of job %d", __func__, __LINE__, job_id);
    return 0;
  }
  time = (uint64_t)(GET_TIME_IN_MS(end_time) - GET_TIME_IN_MS(start_time));

  IDBG_MED("%s %d : jobid %d start time %llu, endtime %llu, time taken %llu",
    __func__, __LINE__, p_job->job_id, GET_TIME_IN_MS(start_time),
    GET_TIME_IN_MS(end_time), time);

  return time;
}

/** img_thread_mgr_get_time_of_joblist
 *
 *  Arguments:
 *   @num_of_jobs: Number of jobs
 *   @job_id_list = array of job id list
 *
 *  Return:
 *    Time in milliseconds
 *
 *  Description:
 *    Get the time taken by all the jobs by the speicifed job id
 *    list
 *
 **/
uint64_t img_thread_mgr_get_time_of_joblist(int num_of_jobs,
  uint32_t *job_id_list)
{
  uint64_t start_time = 0, end_time = 0, time_taken = 0;
  struct timespec *job_start_time = NULL, *job_end_time = NULL;
  int i = 0;
  img_thread_job_t *p_job_list[num_of_jobs];

  if ((num_of_jobs <= 0) || (NULL == job_id_list) ) {
    IDBG_ERROR("%s %d: Invalid input", __func__, __LINE__);
    return time_taken;
  }
  //Get the list of jobs
  for (i = 0; i < num_of_jobs; i++) {
    p_job_list[i] = (img_thread_job_t *)img_list_find(g_thread_pool->job_list,
      img_thread_mgr_find_job_by_id, &job_id_list[i]);
    if (!p_job_list[i]) {
      IDBG_ERROR("%s %d: Cannot find job %d", __func__, __LINE__,
        job_id_list[i]);
      return 0;
    }
  }

  job_start_time = &p_job_list[0]->start_time;
  job_end_time = &p_job_list[0]->end_time;

  start_time = (uint64_t)GET_TIME_IN_MS(job_start_time);
  end_time = (uint64_t)GET_TIME_IN_MS(job_start_time);

  for (i = 0; i < num_of_jobs; i++) {
    job_start_time = &p_job_list[i]->start_time;
    job_end_time = &p_job_list[i]->end_time;
    if (start_time > (uint64_t) GET_TIME_IN_MS(job_start_time)) {
      start_time = (uint64_t)GET_TIME_IN_MS(job_start_time);
    }
    if (end_time < (uint64_t) GET_TIME_IN_MS(job_end_time)) {
      end_time = (uint64_t)GET_TIME_IN_MS(job_end_time);
    }
  }
  time_taken = end_time = start_time;

  IDBG_MED("%s %d: Time taken between the jobs is %llums", __func__,
    __LINE__, time_taken);

  return time_taken;
}

/** img_thread_mgr_wait_for_completion
 *
 *  Arguments:
 *   @p_cond: pointer to the pthread condition
 *   @p_mutex: pointer to the pthread mutex
 *   @max_time: time in milli seconds
 *
 *  Return:
 *    IMG_SUCCESS on success
 *    IMG_ERR_TIMEOUT if times out
 *    IMG_ERR_GENERAL on other errors
 *
 *  Description:
 *    Wait for the specified duration of time, i.e max_time. If
 *    max_time is 0 will wait till signal for condition is
 *    received.
 *
 **/
static int img_thread_mgr_wait_for_completion(pthread_cond_t *p_cond,
  pthread_mutex_t *p_mutex, uint32_t max_time)
{
  int rc = IMG_SUCCESS;
  struct timespec lTs;
  rc = clock_gettime(CLOCK_REALTIME, &lTs);

  if (rc < 0) {
     return IMG_ERR_GENERAL;
  }

  if (max_time == 0) {
    rc = pthread_cond_wait(p_cond, p_mutex);
    return rc;
  }

  if (max_time >= 1000) {
    lTs.tv_sec += (__kernel_time_t)(max_time / 1000);
    lTs.tv_nsec += (__kernel_time_t)((max_time % 1000) * 1000000);
  } else {
    lTs.tv_nsec += (__kernel_time_t)(max_time * 1000000);
  }

  rc = pthread_cond_timedwait(p_cond, p_mutex, &lTs);
  if (rc == ETIMEDOUT || rc == ESHUTDOWN) {
    rc = IMG_ERR_TIMEOUT;
  }
  return rc;
}

/** img_thread_mgr_wait_for_completion_by_jobid
 *
 *  Arguments:
 *   @job_id: Job ID
 *   @max_time: max time (in ms) to wait
 *
 *  Return:
 *    Standard Imaging error values
 *
 *  Description:
 *    Wait till the job with job_id completes execution.
 *    Can specify max duration of time to wait, using max_time.
 *    If max_time is 0 will wait indefinitely till job completes
 *    execution.
 *
 **/
int img_thread_mgr_wait_for_completion_by_jobid(uint32_t job_id,
  uint32_t max_time)
{
  img_thread_job_t *p_job = NULL;

  int rc = IMG_SUCCESS;

  if(job_id <= 0) {
    IDBG_ERROR("%s %d: Invalid jobid", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  pthread_mutex_lock(&g_thread_pool->pool_mutex);
  p_job = (img_thread_job_t *)img_list_find(g_thread_pool->job_list,
    img_thread_mgr_find_job_by_id, &job_id);
  if (!p_job) {
    pthread_mutex_unlock(&g_thread_pool->pool_mutex);
    IDBG_ERROR("%s %d: No job with jobid %d present",
      __func__, __LINE__, job_id);
    return IMG_ERR_INVALID_INPUT;
  }
  for (;;) {
    if (p_job->status != IMG_JOB_DONE) {
      rc = img_thread_mgr_wait_for_completion(&g_thread_pool->pool_cond,
        &g_thread_pool->pool_mutex, max_time);
    }
    if ((p_job->status == IMG_JOB_DONE) || (rc == IMG_ERR_TIMEOUT)) {
        IDBG_MED("%s %d: jobid %d done or timeout %d", __func__, __LINE__,
          job_id, rc);
      break;
    }
  }
  pthread_mutex_unlock(&g_thread_pool->pool_mutex);
  return rc;
}

/** img_thread_mgr_wait_for_completion_by_joblist
 *
 *  Arguments:
 *   @num_of_jobs: Number of jobs
 *   @p_job_list: Array of job ids
 *   @max_time: max time (in ms) to wait
 *
 *  Return:
 *    Standard imaging error values
 *
 *  Description:
 *    Wait till the list of jobs complete execution.
 *    Can specify max duration of time to wait, using max_time.
 *    If max_time is 0 will wait indefinitely till jobs complete
 *    execution.
 *
 **/
int img_thread_mgr_wait_for_completion_by_joblist(int num_of_jobs,
  uint32_t *p_job_id_list, uint32_t max_time)
{
  int i = 0;
  int rc = IMG_SUCCESS;
  img_thread_job_t *p_job = NULL;
  img_thread_job_t *p_job_list[num_of_jobs];
  uint8_t jobs_done_count = 0, jobs_found = 0;

  if ((NULL == p_job_id_list) || (num_of_jobs <= 0)) {
    IDBG_ERROR("%s %d: Invalid Input", __func__, __LINE__);
    return IMG_ERR_INVALID_INPUT;
  }

  pthread_mutex_lock(&g_thread_pool->pool_mutex);
  for (i = 0; i < num_of_jobs; i++) {
    p_job = (img_thread_job_t *)img_list_find(g_thread_pool->job_list,
      img_thread_mgr_find_job_by_id, &(p_job_id_list[i]));
    if (p_job) {
      p_job_list[jobs_found] = p_job;
      jobs_found++;
    } else {
      IDBG_ERROR("%s %d: No job with job id %d",__func__, __LINE__,
        p_job_id_list[i]);
    }
  }

  for(;;) {
    jobs_done_count = 0;
    for (i = 0; i < jobs_found; i++) {
      if (p_job_list[i]->status == IMG_JOB_DONE) {
        jobs_done_count++;
      }
    }
    if(jobs_done_count != jobs_found) {
      rc = img_thread_mgr_wait_for_completion(&g_thread_pool->pool_cond,
        &g_thread_pool->pool_mutex, max_time);
    }

   if((jobs_done_count == jobs_found) || (rc == IMG_ERR_TIMEOUT)) {
     pthread_mutex_unlock(&g_thread_pool->pool_mutex);
     break;
   }
  }
  return rc;
}

/** img_thread_mgr_wait_for_completion_by_clientid
 *
 *  Arguments:
 *   @client_id: Client ID
 *   @max_time: max time (in ms) to wait
 *
 *  Return:
 *     Standard imaging error values
 *
 *  Description:
 *    Wait till all the jobs with client_id completes execution
 *    or for max time specified.
 *    Assumes that the client has already scheduled all jobs
 *    before making this call.
 *    If max_time is 0 will wait indefinitely till all the jobs
 *    with client_id completes execution.
 *
 **/
int img_thread_mgr_wait_for_completion_by_clientid(uint32_t client_id,
  uint32_t max_time)
{
  int rc = IMG_SUCCESS;
  int i = 0;
  int job_count = 0, job_done_count = 0;
  int total_jobs = img_list_count(g_thread_pool->job_list);
  img_thread_job_t *p_job[total_jobs];

  if(client_id <= 0) {
    IDBG_ERROR("%s %d: Invalid Client id %d", __func__, __LINE__, client_id);
    return IMG_ERR_INVALID_INPUT;
  }

  //Get all jobs with client ID.
  pthread_mutex_lock(&g_thread_pool->pool_mutex);
  job_count = img_list_find_all(g_thread_pool->job_list,
    img_thread_mgr_find_job_by_client_id, &client_id, (void *)p_job);

  for(;;) {
    // reset the error
    rc = IMG_SUCCESS;
    job_done_count = 0;
    for (i = 0 ; i < job_count; i++) {
      if (p_job[i]->status == IMG_JOB_DONE) {
        job_done_count++;
      }
    }
    if(job_count != job_done_count) {
      rc = img_thread_mgr_wait_for_completion(&g_thread_pool->pool_cond,
        &g_thread_pool->pool_mutex, max_time);
    }

    if ((job_count == job_done_count) || (rc == IMG_ERR_TIMEOUT)) {
      pthread_mutex_unlock(&g_thread_pool->pool_mutex);
      break;
    }
  }
  return rc;
}

/** img_thread_mgr_get_number_of_jobs_by_clientid_taskexec
 *
 *  Arguments:
 *   @client_id: Client ID
 *   @execute: task exec.
 *       If exec is NULL, returns the number of jobs by clientID
 *       If exec is non-NULL, returns the number of jobs by clientID+exec
 *
 *  Return:
 *     numbers of jobs pending for this Client ID
 *     -1 on error
 *
 *  Description:
 *    Gets the number of jobs that are peinding/being executed
 *    for the given client_id and for the give task_exec type
 *
 **/
int img_thread_mgr_get_number_of_jobs_by_clientid_taskexec(
  uint32_t client_id, task_exec execute)
{
  int i = 0;
  int job_count = 0, pending_jobs = 0;
  int total_jobs = img_list_count(g_thread_pool->job_list);
  img_thread_job_t *p_job[total_jobs];

  if(client_id <= 0) {
    IDBG_ERROR("Invalid Client id %d", client_id);
    return -1;
  }

  // Get all jobs with client ID.
  pthread_mutex_lock(&g_thread_pool->pool_mutex);
  job_count = img_list_find_all(g_thread_pool->job_list,
    img_thread_mgr_find_job_by_client_id, &client_id, (void *)p_job);

  for (i = 0 ; i < job_count; i++) {
    if ((p_job[i]->status != IMG_JOB_DONE) &&
      ((execute == NULL) || (p_job[i]->job_params->execute == execute))) {
      pending_jobs++;
    }
  }
  pthread_mutex_unlock(&g_thread_pool->pool_mutex);
  return pending_jobs;
}


