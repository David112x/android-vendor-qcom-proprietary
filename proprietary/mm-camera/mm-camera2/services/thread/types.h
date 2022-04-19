#ifndef _TH_TYPES_H__
#define _TH_TYPES_H__

/*========================================================================

*//** @file common.h

FILE SERVICES:
     Datatype definitions for thread services library

     Detailed description

Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.

*//*====================================================================== */

/*========================================================================
                      Edit History

when       who    what, where, why
--------   ---    -------------------------------------------------------
11/11/14   rg     Initial Version.

========================================================================== */


/*========================================================================*/
/* INCLUDE FILES */
/*========================================================================*/
/* std headers */

/* local headers */
#include "config.h"
#include "common.h"

/*========================================================================*/
/* MACRO DEFINITIONS */
/*========================================================================*/
#define HTABLE_SLOT_BITS 8
#define HTABLE_SLOT_T uint8_t
#define HTABLE_SLOT_N ((1<<HTABLE_SLOT_BITS)-1)
#define HTABLE_COLL_CHAIN 3
#define MAX_FREQ_CEIL 50

#define DISP_FLASH_RED      0x1
#define DISP_FLASH_ORANGE   0x2
#define DISP_FLASH_GREEN    0x4

#if 1
#define TIMER_SYSTIME_TYPE struct timeval
#define __timer_gettime(t) (gettimeofday(&(t), NULL))
#define __timer_readtime(t) ( ((uint64_t)((t).tv_sec))*1000000 + \
                                        ((uint64_t)((t).tv_usec)) )
#define __timer_difftime(t,u) ( (uint64_t)((u).tv_sec-(t).tv_sec)*1000000 + \
                         (uint64_t)((u).tv_usec-(t).tv_usec) )

#else
#define TIMER_SYSTIME_TYPE struct timespec
#define __timer_gettime(t) (clock_gettime(CLOCK_REALTIME, &(t)))
#define __timer_readtime(t) \
 ( ((uint64_t)((t).tv_sec))*1000000000 + ((uint64_t)((t).tv_nsec)) ) / (1000)
#define __timer_difftime(t,u) \
    ( (uint64_t)((u).tv_sec-(t).tv_sec)*1000000000 + \
                  (uint64_t)((u).tv_nsec-(t).tv_nsec) ) / (1000)

#endif

#define GET_TRIGGER_TYPE(priority) (0x1 << ((priority) - 1))


/*========================================================================*/
/* TYPE DEFINITIONS */
/* catalog of type definitions for the thread service library */
/*========================================================================*/

typedef void *(*job_func_t)(void *arg);

typedef void (*job_callback_t)(void *user_data);

typedef enum {
  TH_JOB_CREATED = 0,
  TH_JOB_SUBMITTED,
  TH_JOB_READY,
  TH_JOB_WAITING,
  TH_JOB_COMPLETED,
  TH_JOB_INVALID_STATE
} th_job_state;

typedef enum {
  TH_MANAGER_FLUSH = 0,
  TH_MANAGER_INVALID
} manager_payload_type_t;

typedef unsigned int hkey_t;
typedef HTABLE_SLOT_T hslot_t;

typedef struct htable_entry_s {
  void *val1;
  void *val2;
  hkey_t key;
  hslot_t slot;
} htable_entry_t;

typedef struct htable_slot_s {
  htable_entry_t entry[HTABLE_COLL_CHAIN];
} htable_slot_t;

typedef struct htable_s {
  htable_slot_t *slots;
  int size;
} htable_t;

typedef struct {
  int slot;
  int next;
} job_free_list_t;

typedef struct {
  //handle to job function
  unsigned int handle;
  //address of job function
  job_func_t job_func;
  //user-friendly job function name
  char *job_func_name;
  //job callback, if application called flushed
  job_callback_t stopped_cb;
  //job data pointer(s)
  void *data_ptr[MAX_TH_DATAPART];
  //job priority, mutually exclusive to relative deadline
  int priority;
  //job relative deadline, mutually exclusive to priority
  int relative_deadline;
  //is job splitable
  int is_splitable;
  //is job blocking
  int blocking;
  //for internal book keeping
  int job_slot;
  //what is the current state of the job
  th_job_state state;
  //num of dependant jobs that this job is split into
  int num_deps;
  //job dependencies, if the incoming job is split into many
  int next;
  int prev;
  //job timestamp
  uint64_t job_ts;
  //semaphore to block job on
  sem_t  msg_sem;

  //handle back to the registered job instance
  void *reg_job_instance;
  //handle to the queue that the job will be part of
  void *th_q;
} th_job_t;

typedef struct {
  //is the job for the application instance in flight for this Q
  int in_flight[MAX_TH_QUEUES];
  //are the jobs for the application instance being flushed
  int is_flushed;
  //does the flush call block the caller
  int is_flush_block;
  //opaque to the thread library
  void *user_data;
  //number of jobs corresponding to this job handle in the TSL
  int job_count;
  //number of job requests that are pending because
  //this job is in flight for this particular Q
  int pend_count[MAX_TH_QUEUES];
  //unique job handle for the specific application instance
  unsigned int handle;
  //semaphore to block flush call on
  sem_t  msg_sem;

  //handle back to the main registered job
  void *reg_job;
} th_job_reg_instance_t;

typedef struct {
  //address of job function
  job_func_t job_addr;
  //user-friendly job function name
  char *job_func_name;
  //address of calback function (used after flush)
  job_callback_t stop_cb_addr;
  //can multiple job calls be made from the library at the same time
  int is_fifo;
  //number of times the job function is registered with the TSL
  //multiple instances of HW (e.g. ISP) will register same job function
  //for each instance
  int num_registered;
  //registered job instances
  th_job_reg_instance_t job_reg_instances[MAX_REGISTERED_INSTANCES];
} th_job_reg_t;

typedef struct {
  void *data;
  int dirty;
} q_element_t;

typedef struct {
  int head;
  int tail;
  int peek_pos;
  int go_back;
  q_element_t elt[MAX_TH_QUEUE_ELEMENTS];

  //color of Q = priority of job
  int color;
  //this Q needs to compact elements, as some may be dirty
  int need_to_compact;

  //lock to secure this Q operations
  pthread_mutex_t q_lock;
} th_queue_t;

typedef struct {
  unsigned int tid;
  pthread_t worker_thread;
  job_func_t worker_thrd_func;
} th_thread_config_t;

typedef struct {
  //current number of jobs submitted to the decision manager
  int job_occupancy;
  //opaque handle to hash table
  htable_t hash_table;
  //lock to secure master job list add/delete-s
  pthread_mutex_t master_joblist_lock;
  int free_list_head;
  int free_list_tail;
  job_free_list_t free_list[MAX_TH_JOB_REQUESTS];
  th_job_t current_jobs[MAX_TH_JOB_REQUESTS];
  th_job_reg_t reg_jobs[MAX_TH_UNIQUE_JOBS];
} th_decision_manager_t;

typedef struct {
  int threads_running;
  int manager_running;
  int manager_pend;

  unsigned int flash_mask;

  //lock and signal to synchronize the threads
  pthread_cond_t read_ok;
  pthread_mutex_t thread_lock;
  th_thread_config_t work_threads[MAX_TH_WORK_THREADS];
  pthread_cond_t manager_ok;
  pthread_mutex_t manager_lock;
  th_thread_config_t manager_thread;
} th_dispatcher_t;

typedef struct {
  unsigned int handle;
  //point in time when an instance of a job was submitted to the TSL
  TIMER_SYSTIME_TYPE begin_time;
  //point in time when an instance of a job schedules in the TSL
  TIMER_SYSTIME_TYPE end_time;
} th_job_profile_inst_t;

typedef struct {
  //min time that a unique job function took from submission to scheduling
  uint64_t min_latency;
  //max time that a unique job function took from submission to scheduling
  uint64_t max_latency;
  //total time that a unique job function spent in the Q-s
  uint64_t tot_latency;
  //number of times an instance of a unique job was submitted to the TSL
  long num_calls;
  //address of job function
  void *addr;
  //name of job function
  char *name;
  //frequency histogram of the wait times for this job
  int hist_freq[MAX_FREQ_CEIL];
  th_job_profile_inst_t job_prof_inst[MAX_PENDING_JOBS];
} th_job_profile_t;

typedef struct {
  //total number of jobs assigned to this worker thread
  long total_jobs_picked;
  //number of times this worker thread picked >1
  //consecutive jobs when dispatcher was triggered
  long times_consec_jobs_picked;
  //max number of consecutive jobs picked
  //by this worker thread at one go
  long max_consec_jobs_picked;

  //number of times scheduled
  long num_scheduled;
  //start to wait for a wait
  TIMER_SYSTIME_TYPE start_wait;
  //end of wait for a job
  TIMER_SYSTIME_TYPE end_wait;
  //start to execute a series of jobs
  TIMER_SYSTIME_TYPE start_exec;
  //end of executing a series of jobs
  TIMER_SYSTIME_TYPE end_exec;
  //average time spent blocked
  uint64_t tot_wait;
  //average time spent in job execution
  uint64_t tot_exec;
} th_thread_profile_t;

typedef struct {
  //number of unique job requests
  int num_job_req;
  //total number of job requests
  int tot_job_req;
  //profiler lock
  pthread_mutex_t prof_lock;
  th_job_profile_t th_job_profile[MAX_TH_UNIQUE_JOBS];
  th_thread_profile_t th_thread_profile[MAX_TH_WORK_THREADS];
} th_profiler_t;

typedef struct {
  //session id
  int session_key;
  //common deadline for all jobs in this session
  int deadline;
  //start of memory for this session
  void *mem_ptr;

  //boolean conditions
  int decision_manager_init;
  int core_init;
  int profiler_init;
  int dispatcher_init;

  th_dispatcher_t *dispatcher;
  th_decision_manager_t *decision_mgr;
  th_profiler_t *th_profiler;
  //main priority Q-s for dispatcher to look into
  th_queue_t priority_queues[MAX_TH_QUEUES];
  th_queue_t manager_queue;
} th_session_t;

typedef struct {
  //what is the current active session to service the incoming thread service request
  unsigned int curr_session_key;
  unsigned int last_session_key;
  //number of active camera sessions getting thread service
  int num_active_sessions;
  int is_profiler_created;
  th_profiler_t *th_profiler[MAX_TH_SESSIONS];
  th_session_t *th_sessions[MAX_TH_SESSIONS];
} th_session_manager_t;

typedef struct {
} th_mem_manager_t;

typedef struct {
  th_session_manager_t *th_sess_mgr;
  th_mem_manager_t *th_mem_mgr;
} th_services_t;

typedef struct {
  job_callback_t stop_cb_addr;
  char *job_func_name;
  void *user_data;
  int *flush_flag;
  pthread_mutex_t *flush_lock;
  sem_t *msg_sem;
} flush_data_t;

typedef struct {
  manager_payload_type_t type;
  flush_data_t data;
} manager_payload_t;

typedef struct {
  job_callback_t stopped_cb;
  void *data_ptr;
  int priority;
  int relative_deadline;
  int is_splitable;
  int is_blocking;
  uint64_t job_ts;
} job_params_t;

#endif /* end of macro _TH_TYPES_H__ */
