/*========================================================================
Copyright (c) 2015-2018 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */


#include "common.h"
#include "profile.h"

#define DUMP_JOB_FREQ_HIST 1

#if defined(ENABLE_THREAD_PROFILING)
/*========================================================================*/
/* INTERNAL FUNCTIONS */
/*========================================================================*/

/**
 * Function: create_profiler
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
static int profiler_find_job(
  th_profiler_t *th_profiler,
  th_job_t *job,
  int *idx
)
{
  int i = 0, vacant_slot = -1;

  for (i = 0; i < MAX_TH_UNIQUE_JOBS; i++) {
    if (th_profiler->th_job_profile[i].addr == (void *)(job->job_func)) {
      *idx = i;
      return 0;//existing job
    }
    if (-1 == vacant_slot && IS_NULL(th_profiler->th_job_profile[i].addr)) {
      vacant_slot = i;
    }
  }

  if (vacant_slot != -1) {
    *idx = vacant_slot;
    return 1;//new job
  }

  *idx = -1;
  return 0;//doesn't matter what we return, idx will always be checked
}


/**
 * Function: profile_publish_results
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
static void profile_publish_results(th_profiler_t *th_profiler)
{
  unsigned int idx = 0, idx2 = 0;
  unsigned int num_threads = MAX_TH_WORK_THREADS;
  int base_freq = 0, ceil_freq = 0;
  th_job_profile_t *job_profile = NULL;
  th_thread_profile_t *th_profile = NULL;
  FILE *job_freq_handle = NULL;

  if (IS_NULL(th_profiler)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return;
  }

#if (DUMP_JOB_FREQ_HIST == 1)
  job_freq_handle = fopen(QCAMERA_DUMP_FRM_LOCATION"job_hist.txt", "w");
#endif

  TH_LOG_ERR(" *** Thread Services Library profiling information ***");
  TH_LOG_ERR(" Number of worker threads used: %d ", num_threads);
  TH_LOG_ERR(" Unique Job Requests: %d ", th_profiler->num_job_req);

  TH_LOG_ERR(" *** Job Statistics ***");
  TH_LOG_ERR(" Total Job Requests(Calls): %d ", th_profiler->tot_job_req);
  for (idx = 0; idx < MAX_TH_UNIQUE_JOBS; idx++) {

    job_profile = &th_profiler->th_job_profile[idx];

    if (0 == job_profile->num_calls) {
      job_profile->num_calls = 1;
    }
    if (job_profile->max_latency == 999999ll) {
      job_profile->min_latency = 0;
    }

    if (!IS_NULL(job_profile->addr)) {
      TH_LOG_ERR(" Job Name         : %s ", job_profile->name);
      TH_LOG_ERR(" Job Num of calls : %ld ", job_profile->num_calls);
      TH_LOG_ERR(" Job Min Wait Time: %"PRIu64" us", job_profile->min_latency);
      TH_LOG_ERR(" Job Max Wait Time: %"PRIu64" us", job_profile->max_latency);
      TH_LOG_ERR(" Job Avg Wait Time: %"PRIu64" us",
                   job_profile->tot_latency / job_profile->num_calls);

#if (DUMP_JOB_FREQ_HIST == 1)
      if (job_freq_handle) {
        fprintf(job_freq_handle, "\n Job Name: %s\n", job_profile->name);
        for (idx2 = 0; idx2 < MAX_FREQ_CEIL; idx2++) {
          base_freq = idx2 * 100;
          ceil_freq = base_freq + 100;
          fprintf(job_freq_handle, "    %4d - %4d : %5d\n", base_freq, ceil_freq,
                                            job_profile->hist_freq[idx2]);
        }
      }
#endif
    }
  }

  TH_LOG_ERR(" *** Worker Thread Statistics ***");
  for (idx = 0; idx < MAX_TH_WORK_THREADS; idx++) {

    th_profile = &th_profiler->th_thread_profile[idx];
    if (0 == th_profile->num_scheduled) {
      th_profile->num_scheduled = 1;
    }

    TH_LOG_ERR(" Worker thread #%d ", idx + 1);
    TH_LOG_ERR(" Total jobs picked                      : %ld ",
                                   th_profile->total_jobs_picked);
    TH_LOG_ERR(" Max consecutive jobs picked at a time  : %ld",
                              th_profile->max_consec_jobs_picked);
    TH_LOG_ERR(" Number of times consecutive jobs picked: %ld",
                              th_profile->times_consec_jobs_picked);
    TH_LOG_ERR(" Number of times scheduled              : %ld",
                                         th_profile->num_scheduled);
    TH_LOG_ERR(" Avg Block Time                         : %"PRIu64" us",
                            th_profile->tot_wait / th_profile->num_scheduled);
    TH_LOG_ERR(" Avg Execute Time                       : %"PRIu64" us",
                            th_profile->tot_exec / th_profile->num_scheduled);
  }

#if (DUMP_JOB_FREQ_HIST == 1)
  fclose(job_freq_handle);
#endif

  return;
}


/*========================================================================*/
/* INTERNAL INTERFACE IMPLEMENTATION */
/*========================================================================*/

/**
 * Function: create_profiler
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
th_err_t create_profiler(th_session_manager_t *th_sess_mgr)
{
  th_err_t err = TH_SUCCESS;
  int idx = 0, idx2 = 0;
  th_profiler_t *th_profiler = NULL;

  for (idx = 0; idx < MAX_TH_SESSIONS; idx++) {
    th_sess_mgr->th_profiler[idx] = (th_profiler_t *)
                                malloc(sizeof(th_profiler_t));
    if (IS_NULL(th_sess_mgr->th_profiler[idx])) {
      TH_LOG_ERR(" Mem allocation failure in thread services library");
      return TH_ERROR_FATAL;
    }
    memset(th_sess_mgr->th_profiler[idx], 0x0, sizeof(th_profiler_t));

    th_profiler = th_sess_mgr->th_profiler[idx];
    th_profiler->num_job_req = 0;
    th_profiler->tot_job_req = 0;

    //pthread_mutex_init(&th_profiler->prof_lock, NULL);

    for (idx2 = 0; idx2 < MAX_TH_UNIQUE_JOBS; idx2++) {
      th_profiler->th_job_profile[idx2].addr = NULL;
      th_profiler->th_job_profile[idx2].name = NULL;
    }
  }

  TH_LOG_LOW(" create_profiler successful");
  return err;
}


/**
 * Function: destroy_profiler
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
th_err_t destroy_profiler(th_session_manager_t *th_sess_mgr)
{
  th_err_t err = TH_SUCCESS;
  int idx = 0;

  for (idx = 0; idx < MAX_TH_SESSIONS; idx++) {
    if (!IS_NULL(th_sess_mgr->th_profiler[idx])) {
      //pthread_mutex_destroy(&th_sess_mgr->th_profiler[idx]->prof_lock);
      free(th_sess_mgr->th_profiler[idx]);
      th_sess_mgr->th_profiler[idx] = NULL;
    }
  }

  TH_LOG_LOW(" destroy_profiler successful");
  return err;
}

/**
 * Function: profiler_add_job
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
th_err_t profiler_add_job(
  th_profiler_t *th_profiler,
  th_job_t *job
)
{
  th_err_t err = TH_SUCCESS;
  int idx = 0;
  int is_new_job = 0;
  TIMER_SYSTIME_TYPE begin_time;
  th_job_profile_t *job_profile = NULL;

  if (IS_NULL(th_profiler)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  is_new_job = profiler_find_job(th_profiler, job, &idx);
  if (idx == -1) {
    TH_LOG_ERR(" Failed in profiler_find_job");
    return TH_ERROR_FATAL;
  }

  job_profile = &th_profiler->th_job_profile[idx];

  if (is_new_job) {
    TH_LOG_MED(" New job registerd in profiler: %s", job->job_func_name);
    th_profiler->num_job_req++;
    job_profile->name = job->job_func_name;
    job_profile->addr = (void *)job->job_func;
  }

  //have a begin and end time array
  __timer_gettime(begin_time);

  for (idx = 0; idx < MAX_PENDING_JOBS; idx++) {
    if (job_profile->job_prof_inst[idx].handle == 0) {
      break;
    }
  }

  if (idx < MAX_PENDING_JOBS) {
    job_profile->job_prof_inst[idx].begin_time = begin_time;
    job_profile->job_prof_inst[idx].handle = (unsigned int)job;
    job_profile->num_calls++;
    th_profiler->tot_job_req++;
  }

  TH_LOG_LOW(" profiler_add_job successful");
  return err;
}


/**
 * Function: profiler_finish_job
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
th_err_t profiler_finish_job(
  th_profiler_t *th_profiler,
  th_job_t *job
)
{
  th_err_t err = TH_SUCCESS;
  int idx = 0, bucket = 0;
  int is_new_job = 0;
  uint64_t wait = 0;
  TIMER_SYSTIME_TYPE end_time;
  th_job_profile_t *job_profile = NULL;

  if (IS_NULL(th_profiler)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  is_new_job = profiler_find_job(th_profiler, job, &idx);
  if (idx == -1 || is_new_job) {
    TH_LOG_ERR(" Failed in profiler_find_job");
    return TH_ERROR_FATAL;
  }

  job_profile = &th_profiler->th_job_profile[idx];

  __timer_gettime(end_time);

  for (idx = 0; idx < MAX_PENDING_JOBS; idx++) {
    if (job_profile->job_prof_inst[idx].handle == (unsigned int)job) {
      break;
    }
  }

  if (idx < MAX_PENDING_JOBS) {
    job_profile->job_prof_inst[idx].end_time = end_time;
    wait = __timer_difftime(job_profile->job_prof_inst[idx].begin_time,
                                job_profile->job_prof_inst[idx].end_time);
    job_profile->job_prof_inst[idx].handle = 0;

    if (wait < 99999) {//guard against garbage values in timer functions
      if (wait > job_profile->max_latency) {
        job_profile->max_latency = wait;
      } else if (wait < job_profile->min_latency) {
        job_profile->min_latency = wait;
      }

      job_profile->tot_latency += wait;

      bucket = wait / 100;
      if (bucket >= MAX_FREQ_CEIL) {
        bucket = MAX_FREQ_CEIL - 1;
      }
      job_profile->hist_freq[bucket] = job_profile->hist_freq[bucket] + 1;
    }
  }

  TH_LOG_LOW(" profiler_finish_job successful");
  return err;
}


/**
 * Function: profiler_start_thread_exec
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
th_err_t profiler_start_thread_exec(
  th_profiler_t *th_profiler,
  unsigned int idx
)
{
  th_err_t err = TH_SUCCESS;
  th_thread_profile_t *th_profile = NULL;

  if (IS_NULL(th_profiler) || idx >= MAX_TH_WORK_THREADS) {
    TH_LOG_ERR(" Fatal error occured in thread service library");
    return TH_ERROR_FATAL;
  }

  th_profile = &th_profiler->th_thread_profile[idx];
  __timer_gettime(th_profile->start_exec);

  TH_LOG_LOW(" profiler_start_thread_exec successful");
  return err;
}


/**
 * Function: profiler_end_thread_exec
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
th_err_t profiler_end_thread_exec(
  th_profiler_t *th_profiler,
  unsigned int idx,
  int jobs_taken
)
{
  th_err_t err = TH_SUCCESS;
  th_thread_profile_t *th_profile = NULL;
  uint64_t wait = 0, exec = 0;

  if (IS_NULL(th_profiler) || idx >= MAX_TH_WORK_THREADS) {
    TH_LOG_ERR(" Fatal error occured in thread service library");
    return TH_ERROR_FATAL;
  }

  th_profile = &th_profiler->th_thread_profile[idx];

  if (jobs_taken) {
    th_profile->total_jobs_picked += jobs_taken;
    if (jobs_taken > 1) {
      th_profile->times_consec_jobs_picked++;
    }
    if (th_profile->max_consec_jobs_picked < jobs_taken) {
      th_profile->max_consec_jobs_picked = jobs_taken;
    }

    th_profile->num_scheduled++;

    __timer_gettime(th_profile->end_exec);
    th_profile->tot_exec +=
            __timer_difftime(th_profile->start_exec, th_profile->end_exec);
  }

  TH_LOG_LOW(" profiler_end_thread_exec successful");
  return err;
}


/**
 * Function: profiler_start_thread_wait
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
th_err_t profiler_start_thread_wait(
  th_profiler_t *th_profiler,
  unsigned int idx
)
{
  th_err_t err = TH_SUCCESS;
  th_thread_profile_t *th_profile = NULL;

  if (IS_NULL(th_profiler) || idx >= MAX_TH_WORK_THREADS) {
    TH_LOG_ERR(" Fatal error occured in thread service library");
    return TH_ERROR_FATAL;
  }

  th_profile = &th_profiler->th_thread_profile[idx];
  __timer_gettime(th_profile->start_wait);

  TH_LOG_LOW(" profiler_start_thread_wait successful");
  return err;
}


/**
 * Function: profiler_end_thread_wait
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
th_err_t profiler_end_thread_wait(
  th_profiler_t *th_profiler,
  unsigned int idx
)
{
  th_err_t err = TH_SUCCESS;
  th_thread_profile_t *th_profile = NULL;

  if (IS_NULL(th_profiler) || idx >= MAX_TH_WORK_THREADS) {
    TH_LOG_ERR(" Fatal error occured in thread service library");
    return TH_ERROR_FATAL;
  }

  th_profile = &th_profiler->th_thread_profile[idx];
  __timer_gettime(th_profile->end_wait);

  th_profile->tot_wait +=
          __timer_difftime(th_profile->start_wait, th_profile->end_wait);

  TH_LOG_LOW(" profiler_end_thread_wait successful");
  return err;
}


/**
 * Function: init_profiler
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
th_err_t init_profiler(th_session_t *th_session)
{
  th_err_t err = TH_SUCCESS;
  int idx = 0;
  th_profiler_t *th_profiler = NULL;

  th_profiler = th_session->th_profiler;
  if (IS_NULL(th_profiler)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  pthread_mutex_lock(&th_profiler->prof_lock);

  th_profiler->num_job_req = 0;
  th_profiler->tot_job_req = 0;

  for (idx = 0; idx < MAX_TH_UNIQUE_JOBS; idx++) {
    memset(&th_profiler->th_job_profile[idx], 0x0, sizeof(th_job_profile_t));
    th_profiler->th_job_profile[idx].min_latency = 999999ll;
  }

  for (idx = 0; idx < MAX_TH_WORK_THREADS; idx++) {
    memset(&th_profiler->th_thread_profile[idx],
                  0x0, sizeof(th_thread_profile_t));
  }

  pthread_mutex_unlock(&th_profiler->prof_lock);

  TH_LOG_MED(" init_profiler successful");
  return err;
}


/**
 * Function: deinit_profiler
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
th_err_t deinit_profiler(th_session_t *th_session)
{
  th_err_t err = TH_SUCCESS;
  int idx = 0;
  th_profiler_t *th_profiler = NULL;

  th_profiler = th_session->th_profiler;
  if (IS_NULL(th_profiler)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  pthread_mutex_lock(&th_profiler->prof_lock);

  if (th_profiler->tot_job_req) {
    profile_publish_results(th_profiler);
  }

  for (idx = 0; idx < MAX_TH_UNIQUE_JOBS; idx++) {
    th_profiler->th_job_profile[idx].addr = NULL;
    th_profiler->th_job_profile[idx].name = NULL;
  }

  pthread_mutex_unlock(&th_profiler->prof_lock);

  TH_LOG_MED(" deinit_profiler successful");
  return err;
}
#endif //defined(ENABLE_THREAD_PROFILING)

