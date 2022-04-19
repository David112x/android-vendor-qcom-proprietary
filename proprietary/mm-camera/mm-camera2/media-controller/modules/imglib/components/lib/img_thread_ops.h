/**********************************************************************
*  Copyright (c) 2014-2016 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#ifndef __IMG_THREAD_OPS_H__
#define __IMG_THREAD_OPS_H__

#define MAX_DEPENDANT_JOBS 30

/** img_core_type_t:
 *
 *  IMG_CORE_DSP: DSP
 *  IMG_CORE_GPU: GPU
 *  IMG_CORE_ARM: ARM
 *  IMG_CORE_ANY: ANY available core
 *
 * Description:
 *    Core affinity
 */
typedef enum {
  IMG_CORE_DSP,
  IMG_CORE_GPU,
  IMG_CORE_ARM,
  IMG_CORE_ANY
} img_core_type_t;

/**Function ptr to the task to be executed
*/
typedef int (*task_exec)(void *param);

/** img_thread_job_t:
 *
 *  Arguments:
 *    @dep_job_id: Array of dependant job ids
 *    @dep_job_count: Num of dependant jobs
 *    @core_affinity: Core affinity of the job
 *    @execute: function ptr to the function to be executed
 *    @args: Argument for the execute function
 *    @client_id: ID of the client scheduling the job. Can be
 *              obtained after reserving threads on the
 *              threadpool.
 *    @delete_on_completion: job memory to be freed after
 *       execution is finised
 *  Description:
 *      Job structure
 */
typedef struct {
  uint32_t *dep_job_ids;
  int dep_job_count;
  img_core_type_t core_affinity;
  task_exec execute;
  void *args;
  uint32_t client_id;
  int32_t delete_on_completion;
} img_thread_job_params_t;

/** img_thread_mgr_create_pool
 *
 *  Arguments:
 *   None
 *
 *  Return: 0 on success
 *          -1 on failure
 *
 *  Description:
 *    Creates a new threadpool. There can be only one
 *    thread pool for all the imglib modules
 *
 **/
int img_thread_mgr_create_pool();


/** img_thread_mgr_destroy_pool
 *
 *  Arguments:
 *   None
 *
 *  Return:
 *   None
 *
 *  Description:
 *    Destroy the threadpool
 *
 **/
void img_thread_mgr_destroy_pool();

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
  img_core_type_t thread_affinity[]);

/** img_thread_mgr_unreserve_threads
 *
 *  Arguments:
 *   @client_id: Client ID
 *
 *  Return:
 *    Standard Error Values
 *
 *  Description:
 *    Unreserve the threads from the threadpool that were
 *    reserved by the client
 *
 **/
int img_thread_mgr_unreserve_threads(uint32_t client_id);

/** img_thread_ops_t
 *    @client_id: Client ID
 *    @schedule_job: function pointer for scheduling a job
 *    @get_time_of_job: Get the time taken by a job specified by
 *                    the jobid
 *    @get_time_of_joblist: Wait till all the jobs with the specified job ids
         completes execution
 *    @wait_for_completion_by_jobid: Wait till the list of jobs
 *      complete execution
 *    @wait_for_completion_by_joblist: Wait till the list of
 *       jobs complete execution
 *    @wait_for_completion_by_clientid: Wait till allthe
 *       jobswith client_id completes execution
 *
 *    Thread ops table
 **/
typedef struct {

  /**client id
  *
  * Client id : ID recieved from thread pool on reserving threads
  **/
  uint32_t client_id;

  /** schedule_job
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
  uint32_t (*schedule_job)(img_thread_job_params_t *job_params);

  /** get_time_of_job
  *
  *  Arguments:
  *  @job_id: Job ID
  *
  *   Return: Time in milliseconds; 0 on failure
  *
  *  Description:
  *    Get the time taken by a job specified by the jobid
  *
  **/
  uint64_t (*get_time_of_job)(uint32_t job_id);


  /** get_time_of_joblist
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
  uint64_t (*get_time_of_joblist)(int num_of_jobs, uint32_t *job_id_list);

  /** wait_for_completion_by_jobid
  *
  *  Arguments:
  *   @job_id: Job ID
  *   @max_time: max time (in ms) to wait
  *
  *  Return:
  *    Standard Imaging error values
  *
  *  Description:
  *    Wait till all the jobs with the specified job ids completes
  *    execution. Can specify max duration of time to wait, using
  *    max_time. If max_time is 0 will wait indefinitely till job
  *    completes execution.
  *
  **/
 int (*wait_for_completion_by_jobid)(uint32_t job_id, uint32_t max_time);

  /** wait_for_completion_by_joblist
  *
  *  Arguments:
  *   @num_of_jobs: Number of jobs
  *   @p_job_list: Array of job ids
  *   @max_time: max time (in ms) to wait
  *
  *  Return:
  *    Standard Imaging error values
  *
  *  Description:
  *    Wait till the list of jobs complete execution. Can specify
  *    max duration of time to wait, using max_time. If max_time
  *    is 0 will wait indefinitely till jobs complete execution.
  **/
  int (*wait_for_completion_by_joblist)(int num_of_jobs, uint32_t *p_job_list,
    uint32_t max_time);

  /** wait_for_completion_by_clientid
  *
  *  Arguments:
  *   @client_id: Client ID
  *   @max_time: max time (in ms) to wait
  *
  *  Return:
  *    Standard Imaging error values
  *
  *  Description:
  *    Wait till all the jobs with client_id completes execution
  *    or for max time specified. Assumes that the client has
  *    already scheduled all jobs before making this call. If
  *    max_time is 0 will wait indefinitely till all the jobs with
  *    client_id completes execution.
  *
  **/
  int (*wait_for_completion_by_clientid)(uint32_t client_id, uint32_t max_time);


} img_thread_ops_t;

#endif //__IMG_THREAD_OPS_H__
