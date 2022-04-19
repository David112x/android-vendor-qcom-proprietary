/**********************************************************************
* Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                 *
**********************************************************************/

#ifndef __IMG_THREAD_H__
#define __IMG_THREAD_H__

#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include "img_thread_ops.h"
#include "img_queue.h"
#include "img_list.h"
#include "img_dbg.h"
#include "img_common.h"

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define MAX_THREAD_COUNT 20
#define MAX_NUM_CLIENTS 10

#define CREATE_JOB_ID(client_id, job_count) ((client_id << 28)+ job_count)
#define GET_CLIENT_ID(job_id) ((job_id & 0xF0000000) >> 28)
#define GET_TIME_IN_MS(ts) \
  ((ts->tv_sec * 1000LL) + (ts->tv_nsec / 1000000LL))

/*checks if the function call is made in the thread context*/
#define IsSelf(thread_id) (pthread_equal(pthread_self(), thread_id))


/** img_thread_job_status_t:
 *
 *  Arguments:
 *    @IMG_JOB_INIT: Job has been initialized
 *    @IMG_JOB_SCHEDULED: Job has been scheduled
 *    @IMG_JOB_EXECUTING: Job is executing
 *    IMG_JOB_DONE: Job has completed execution
 *
 *  Description:
 *      Job status enum
 */
typedef enum {
  IMG_JOB_INIT,
  IMG_JOB_SCHEDULED,
  IMG_JOB_EXECUTING,
  IMG_JOB_DONE,
} img_thread_job_status_t;

/** img_thread_job_t:
 *
 *  Arguments:
 *    @job_id: unique job id
 *    @job_params: Job params
 *    @status: status of the job
 *    @start_time: Time job was started
 *    @end_time: Time job ended
 *
 *  Description:
 *      Job structure
 */
typedef struct {
  uint32_t job_id;
  img_thread_job_params_t *job_params;
  img_thread_job_status_t status;
  struct timespec start_time;
  struct timespec end_time;
} img_thread_job_t;


/** img_thread_t:
 *
 *  Arguments:
 *     @p_sh_q: Shared queue
 *     @int_q: internal queue
 *     @mutex: mutex for thread
 *     @name: thread name
 *     @cond: conditional variable for thread
 *     @threadid: id of the thread
 *     @is_active: flag to indicate if the thread is active
 *     @client_id: Client ID of the client that reserves the
 *               thread
 *     @exit_flag: Flag indicating the thread to exit
 *     @is_ready: thread ready to schedule jobs on
 *
 *  Description:
 *      Thread structure
 *
 **/
typedef struct {
  img_queue_t *p_sh_q;
  img_queue_t int_q;
  pthread_mutex_t mutex;
  char *name;
  pthread_cond_t cond;
  pthread_t threadid;
  uint8_t is_active;
  img_core_type_t core_affinity;
  uint32_t client_id;
  uint8_t exit_flag;
  int32_t is_ready;
} img_thread_t;


/** img_thread_pool_t:
 *
 *  Arguments:
 *     @thread: Array of threads in the threadpool
 *     @pool_mutex: mutex for thread pool
 *     @pool_cond: cond for thread pool
 *     @num_of_threads: number of threads in thread pool
 *     @ref_count: Count of the number of clients requesting the
 *      thread pool. Thread pool will not be destroyed till the
 *      ref_count is 0
 *     @num_of_avail_threads: Number of threads available in the
 *                          thread pool
 *     @num_of_clients: Number of clients
 *
 *  Description:
 *      Thread pool structure
 *
 **/
typedef struct {
  img_thread_t *thread;
  pthread_mutex_t pool_mutex;
  pthread_cond_t pool_cond;
  int num_of_threads;
  img_list_t *job_list;
  int ref_count;
  int num_of_avail_threads;
  uint32_t num_of_clients;
  uint32_t total_client_count;
} img_thread_pool_t;

/* Functions */

/** img_thread_mgr_schedule_job
 *
 *  Arguments:
 *   @job_params: Job parameters
 *
 *  Return: unique jobid
 *          -1 on failure
 *
 *  Description:
 *    Schedules a new job with the thread pool
 *
 **/
uint32_t img_thread_mgr_schedule_job(img_thread_job_params_t *job_params);

/** img_thread_mgr_remove_job
 *
 *  Arguments:
 *   @p_job: Job to be removed from list
 *
 *  Return:
 *    0 on success, -1 on failure
 *
 *  Description:
 *      Remove a job from the job list and fetch the next job
 *
 **/
int img_thread_mgr_remove_job(img_thread_job_t *p_job);
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
void *img_thread_start_job(void *arg);

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
void img_thread_mgr_fetch_job();

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
void img_thread_mgr_job_done(uint32_t job_id);

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
void img_thread_send(img_thread_t *p_th, img_thread_job_t *p_data);

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
void img_thread_reset(img_thread_t *p_th);

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
void img_thread_destroy(img_thread_t *p_th);

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
int img_thread_create(img_thread_t *p_th, img_queue_t *p_sh_q);

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
uint64_t img_thread_mgr_get_time_of_job(uint32_t job_id);

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
  uint32_t *job_id_list);

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
 *    Wait till the job with job_id completes execution
 *
 **/
int img_thread_mgr_wait_for_completion_by_jobid(uint32_t job_id,
  uint32_t max_time);

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
 *    Wait till the job with job_id completes execution
 *
 **/
int img_thread_mgr_wait_for_completion_by_joblist(int num_of_jobs,
  uint32_t *p_job_id_list, uint32_t max_time);

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
 *
 **/
int img_thread_mgr_wait_for_completion_by_clientid(uint32_t client_id,
  uint32_t max_time);

/** img_thread_mgr_get_number_of_jobs_by_clientid_taskexec
 *
 *  Arguments:
 *   @client_id: Client ID
 *   @execute: task exec.
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
  uint32_t client_id, task_exec execute);


#endif //__IMG_THREAD_H__
