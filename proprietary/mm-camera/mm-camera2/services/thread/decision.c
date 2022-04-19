/*========================================================================
Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */


#include "dispatcher.h"
#include "decision.h"
#include "core.h"
#include "memory.h"
#include <string.h>
#include <stdlib.h>

#define GET_PRIUORITY_Q_IDX(color) ((color) >> 1)

/*========================================================================*/
/* INTERNAL FUNCTIONS */
/*========================================================================*/

/**
 * Function: get_new_handle
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
static unsigned int get_new_handle(
  unsigned int ptr,
  unsigned int idx,
  int session_key
)
{
  return ((ptr & 0xFFFFFF) | (idx << 24) | (session_key << 28));
}


/**
 * Function: htable_hash
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
static inline hslot_t htable_hash(hkey_t key)
{
  union key_words_t {
    hkey_t key;
    hslot_t slot;
  } key_words;

  key_words.key = key;
  return key_words.slot;
}


/**
 * Function: htable_add
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
static void htable_add(
  htable_t *hash_table,
  unsigned int key,
  void *val1,
  void *val2
)
{
  int i = 0;
  htable_entry_t *entry = NULL;
  htable_slot_t *slots = NULL;

  hslot_t slot = htable_hash(key);

  slots = &hash_table->slots[slot];

  for (i = 0; i < HTABLE_COLL_CHAIN; i++) {
    entry = &slots->entry[i];
    if (0 == entry->key) {
      entry->key = key;
      entry->val1 = val1;
      entry->val2 = val2;
      hash_table->size++;
      return;
    } else if (key == entry->key) {
      return;
    }
  }

  TH_LOG_ERR("htable_add failed");
  return;
}


/**
 * Function: htable_get
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
static void htable_get(
  htable_t *hash_table,
  unsigned int key,
  void **val1,
  void **val2
)
{
  int i = 0;
  htable_entry_t *entry = NULL;
  htable_slot_t *slots = NULL;

  hslot_t slot = htable_hash(key);

  slots = &hash_table->slots[slot];

  for (i = 0; i < HTABLE_COLL_CHAIN; i++) {
    entry = &slots->entry[i];
    if (key == entry->key) {
      *val1 = entry->val1;
      *val2 = entry->val2;
      return;
    }
  }

  *val1 = NULL;
  *val2 = NULL;
  TH_LOG_ERR("htable_get failed");

  return;
}


/**
 * Function: job_end_process
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
static void job_end_notify(
  th_decision_manager_t *decision_mgr,
  th_job_t *job
)
{
  th_job_t *job_temp = NULL;

  job->num_deps--;
  if (!job->num_deps) {//either this job was not data partitioned or all cousins finished
    //release semaphore
    TH_LOG_LOW("Blocking job (%s), releasing semaphore", job->job_func_name);
    sem_post(&job->msg_sem);
  } else {
    job_temp = job;
    while (job_temp->prev != -1) {
      job_temp = &decision_mgr->current_jobs[job_temp->prev];
      job_temp->num_deps--;
    }
    job_temp = job;
    while (job_temp->next != -1) {
      job_temp = &decision_mgr->current_jobs[job_temp->next];
      job_temp->num_deps--;
    }
    //adjust the links and remove the current job from the link of dependant jobs
    if (job->prev != -1) {
      job_temp = &decision_mgr->current_jobs[job->prev];
      job_temp->next = job->next;
    }
    if (job->next != -1) {
      job_temp = &decision_mgr->current_jobs[job->next];
      job_temp->prev = job->prev;
    }
  }

  return;
}


/**
 * Function: decide_job_priority
 *
 * Description: Translate the incoming priority to one
 * which the Thread Library understands (if needed)
 *
 * Arguments:
 *   @th_session: Current session context
 *   @job: Job which has come in
 *
 * Return values: None
 *
 * Notes: none
 **/
static void decide_job_priority(
  th_session_t *th_session,
  th_job_t *job
)
{
  th_err_t err = TH_SUCCESS;

  //simple implementation for now

  //if job comes with a priority, just sanitize it in the range 1, 2, 3
  if (job->priority) {
    if (job->priority < 1) {
      job->priority = JOB_PRIO_CRITICAL;
    } else if (job->priority > 3) {
      job->priority = JOB_PRIO_NORMAL;
    }
  } else if (job->relative_deadline) {
                                   //if job comes with a deadline, compare it with
                                   //fps and assign priority in the range 1, 2, 3
    if (job->relative_deadline < (th_session->deadline >> 1)) {
      job->priority = JOB_PRIO_CRITICAL;
    } else if (job->relative_deadline > th_session->deadline) {
      job->priority = JOB_PRIO_NORMAL;
    } else {
      job->priority = JOB_PRIO_HIGH;
    }
  } else {//if job comes with nothing, assign default priority of 3
    job->priority = JOB_PRIO_NORMAL;
  }

  TH_LOG_LOW(" decide_job_priority successful");
  return;
}


/**
 * Function: decision_manager_get_empty_slot
 *
 * Description: Fetches the next free slot from the master
 * job list and update the free list.
 * Operates under master job list lock
 *
 * Arguments:
 *   @decision_mgr: Decision Manager for current session
 *   @reg_job_instance: Registered job instance of the job type
 *   @need_lock: Whether master joblist lock needs to taken
 *
 * Return values: Next available index
 *
 * Notes: none
 **/
static int decision_manager_get_one_empty_slot(
  th_decision_manager_t *decision_mgr,
  th_job_reg_instance_t *reg_job_instance,
  int need_lock
)
{
  int idx = -1;

  if (need_lock) {
    pthread_mutex_lock(&decision_mgr->master_joblist_lock);
  }

  if (decision_mgr->free_list_head != decision_mgr->free_list_tail) {
    //get the free slot
    idx = decision_mgr->free_list_head;
    //update the head of the free list to the next free slot
    decision_mgr->free_list_head = decision_mgr->free_list[idx].next;

    decision_mgr->job_occupancy++;
    //one more job of this type (instance), keep track of it
    reg_job_instance->job_count++;
  }

  if (need_lock) {
    pthread_mutex_unlock(&decision_mgr->master_joblist_lock);
  }

  TH_LOG_LOW(" Job entry created at slot #%d, job occupancy: %d",
                                  idx, decision_mgr->job_occupancy);

  return idx;
}


/**
 * Function: decide_job_partition
 *
 * Description: Breaks up a data partitioned job into multiple
 * sub-jobs and assigns them slots in the master job list
 *
 * Arguments:
 *   @th_session: Current session context
 *   @job: Job which has come in
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
static th_err_t decide_job_partition(
  th_session_t *th_session,
  th_job_t *job
)
{
  th_err_t err = TH_SUCCESS;
  int idx = 0, slot = 0, prev_slot = 0;
  th_job_t *job_next = NULL;
  th_job_t *job_first = job;
  th_decision_manager_t *decision_mgr = NULL;
  job_func_t job_addr = NULL;
  char *job_func_name = NULL;

  if (1 == job->num_deps) {
    job->next = job->prev =  -1;
    return TH_SUCCESS;
  }

  decision_mgr = th_session->decision_mgr;

  //create and link up all the sub-jobs
  //they are all of the same priority and scheduled to the same job Q
  prev_slot = -1;
  while (idx < (job->num_deps - 1)) {
    slot = decision_manager_get_one_empty_slot(decision_mgr,
                           (th_job_reg_instance_t *)job->reg_job_instance, 1);
    if (slot == -1) {
      job->next = -1;
      TH_LOG_ERR(" Failed at decision_manager_get_slot");
      return TH_ERROR_FATAL;
    }

    job_next = &decision_mgr->current_jobs[slot];
    job_next->job_slot = slot;
    job_next->data_ptr[0] = job_first->data_ptr[idx + 1];
    job_next->data_ptr[1] = NULL;
    job_next->priority = job_first->priority;
    job_next->job_ts = job_first->job_ts;
    job_next->handle = job_first->handle;
    job_next->job_func = job_first->job_func;
    job_next->job_func_name = job_first->job_func_name;
    job_next->num_deps = job_first->num_deps;

    job->prev = prev_slot;
    job->next = slot;
    job = job_next;
    job->next = -1;
    prev_slot = slot;
    idx++;
  }

  TH_LOG_LOW(" decide_job_partition successful");
  return err;
}

/*========================================================================*/
/* INTERNAL INTERFACE IMPLEMENTATION */
/*========================================================================*/


/**
 * Function: init_decision_manager
 *
 * Description: Initializes the following Decision Manager structures
 * 1) Free list in the master job list
 * 2) Job registry
 *
 * Arguments:
 *   @th_session: Current session context
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
th_err_t init_decision_manager(th_session_t *th_session)
{
  th_err_t err = TH_SUCCESS;
  int idx = 0, idx2 = 0, idx3 = 0;
  th_decision_manager_t *decision_mgr = NULL;
  th_job_reg_t *reg_job = NULL;

  decision_mgr = th_session->decision_mgr;
  if (IS_NULL(decision_mgr)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  pthread_mutex_init(&decision_mgr->master_joblist_lock, NULL);

  //initialize the entire list as free
  for (idx = 0; idx < MAX_TH_JOB_REQUESTS; idx++) {
    decision_mgr->free_list[idx].slot = idx;
    decision_mgr->free_list[idx].next = idx + 1;
  }
  decision_mgr->free_list_head = 0;
  decision_mgr->free_list_tail = MAX_TH_JOB_REQUESTS - 1;

  decision_mgr->job_occupancy = 0;

  //initialize the master job registry
  for (idx = 0; idx < MAX_TH_UNIQUE_JOBS; idx++) {
    reg_job = &decision_mgr->reg_jobs[idx];
    reg_job->job_addr = NULL;
    reg_job->stop_cb_addr = NULL;
    reg_job->num_registered = 0;
    reg_job->is_fifo = 0;
    //there may be HW or SW instances of a particular job
    //this is akin to separate threads e.g. for different ISP HWs
    for (idx2 = 0; idx2 < MAX_REGISTERED_INSTANCES; idx2++) {
      reg_job->job_reg_instances[idx2].handle = 0;
      reg_job->job_reg_instances[idx2].is_flushed = 0;
      reg_job->job_reg_instances[idx2].job_count = 0;
      for (idx3 = 0; idx3 < MAX_TH_QUEUES; idx3++) {
        reg_job->job_reg_instances[idx2].in_flight[idx3] = 0;
        reg_job->job_reg_instances[idx2].pend_count[idx3] = 0;
      }
      reg_job->job_reg_instances[idx2].user_data = NULL;
    }
  }

  decision_mgr->hash_table.size = 0;//not used for now
  decision_mgr->hash_table.slots =
           (htable_slot_t *)malloc(sizeof(htable_slot_t) * HTABLE_SLOT_N);
  if (IS_NULL(decision_mgr->hash_table.slots)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    //local cleanup
    pthread_mutex_destroy(&decision_mgr->master_joblist_lock);
    return TH_ERROR_NULLPTR;
  }

  TH_LOG_MED(" init_decision_manager successful");
  return err;
}


/**
 * Function: deinit_decision_manager
 *
 * Description: De-initializes Decision Manager structures
 *
 * Arguments:
 *   @th_session: Current session context
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
th_err_t deinit_decision_manager(th_session_t *th_session)
{
  th_err_t err = TH_SUCCESS;
  int idx = 0;
  th_decision_manager_t *decision_mgr = NULL;

  decision_mgr = th_session->decision_mgr;
  if (IS_NULL(decision_mgr)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  TH_LOG_LOW(" Job Occupancy at deinit: %d", decision_mgr->job_occupancy);

  pthread_mutex_destroy(&decision_mgr->master_joblist_lock);

  if (!IS_NULL(decision_mgr->hash_table.slots)) {
    free(decision_mgr->hash_table.slots);
  }

  TH_LOG_MED(" deinit_decision_manager successful");
  return err;
}


/**
 * Function: decision_manager_send_stop_done
 *
 * Description: Send job done callback after job has either
 * finished execution or was flushed. Unblocks caller if it
 * was a blocking job
 *
 * Arguments:
 *   @th_session: Current session context
 *   @job: Job which has finished
 *
 * Return values: None
 *
 * Notes: none
 **/
void decision_manager_send_stop_done(
  th_session_t *th_session,
  th_job_t *job
)
{
  th_err_t err = TH_SUCCESS;
  th_decision_manager_t *decision_mgr = NULL;

  decision_mgr = th_session->decision_mgr;
  if (IS_NULL(decision_mgr)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return;
  }

  //unblock caller
  if (job->blocking) {
    job_end_notify(decision_mgr, job);
  }

  if (job->stopped_cb != NULL) {
    job->stopped_cb(job->data_ptr[0]);
  }

  //job instance is stopped, free the job slot in the master job list
  //don't care about the error code for now
  err = decision_manager_set_one_empty_slot(th_session, job, 0);

  return;
}


/**
 * Function: desicion_manager_send_stop_done
 *
 * Description: Send flush done to caller, after all inflight
 * jobs have either dispatched or stopped
 *
 * Arguments:
 *   @th_session: Current session context
 *   @handle: Registered job instance handle
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
th_err_t desicion_manager_send_flush_done(
  th_session_t *th_session,
  unsigned int handle
)
{
  th_err_t err = TH_SUCCESS;
  int idx = 0, idx2 = 0, send_local_flush_done = 0;
  th_job_reg_t *reg_job = NULL;
  th_job_reg_instance_t *reg_job_instance = NULL;
  th_decision_manager_t *decision_mgr = NULL;
  th_queue_t *th_q = NULL;
  manager_payload_t *flush_data = NULL;
  void *user_data = NULL;

  decision_mgr = th_session->decision_mgr;
  if (IS_NULL(decision_mgr)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  pthread_mutex_lock(&decision_mgr->master_joblist_lock);

  htable_get(&decision_mgr->hash_table, handle,
              (void**)&reg_job, (void**)&reg_job_instance);
  if (IS_NULL(reg_job) || IS_NULL(reg_job_instance)) {
    pthread_mutex_unlock(&decision_mgr->master_joblist_lock);
    TH_LOG_ERR(" Current job not registered");
    return TH_ERROR_FATAL;
  }

  user_data = reg_job_instance->user_data;
  reg_job_instance->user_data = NULL;

  pthread_mutex_unlock(&decision_mgr->master_joblist_lock);

  //post the stop/flush done callback to the manager thread
  if (reg_job->stop_cb_addr) {
    flush_data = (manager_payload_t *)malloc(sizeof(manager_payload_t));
    if (IS_NULL(flush_data)) {
      TH_LOG_ERR(" Encountered NULL pointer");
      return TH_ERROR_FATAL;
    }
    flush_data->type = TH_MANAGER_FLUSH;
    flush_data->data.stop_cb_addr = reg_job->stop_cb_addr;
    flush_data->data.user_data = user_data;
    flush_data->data.job_func_name = reg_job->job_func_name;
    flush_data->data.msg_sem = NULL;
    flush_data->data.flush_flag = &reg_job_instance->is_flushed;
    flush_data->data.flush_lock =
                 &th_session->decision_mgr->master_joblist_lock;
    if (reg_job_instance->is_flush_block) {
      flush_data->data.msg_sem = &reg_job_instance->msg_sem;
    }

    th_q = &th_session->manager_queue;
    pthread_mutex_lock(&th_q->q_lock);
    err = add_to_q(th_q, flush_data);
    pthread_mutex_unlock(&th_q->q_lock);

    pthread_mutex_lock(&th_session->dispatcher->manager_lock);
    th_session->dispatcher->manager_pend = 1;
    //trigger manager thread to send the flush done
    if (th_session->dispatcher->manager_running) {
      pthread_cond_signal(&th_session->dispatcher->manager_ok);
    } else {
      send_local_flush_done = 1;
    }
    pthread_mutex_unlock(&th_session->dispatcher->manager_lock);
    //unblock the caller in case manager thread had stopped
    if (send_local_flush_done) {
      reg_job->stop_cb_addr(user_data);
      if (reg_job_instance->is_flush_block) {
        //release semaphore
        TH_LOG_LOW("Blocking flush (%s), releasing semaphore",
                                          reg_job->job_func_name);
        sem_post(&reg_job_instance->msg_sem);
      }
      //reset the flush mode flag, so that new jobs can be accepted
      reg_job_instance->is_flushed = 0;
    }
  } else {//unblock caller if no flush done callback was provided
    if (reg_job_instance->is_flush_block) {
      //release semaphore
      TH_LOG_LOW("Blocking flush (%s), releasing semaphore",
                                        reg_job->job_func_name);
      sem_post(&reg_job_instance->msg_sem);
    }
    //reset the flush mode flag, so that new jobs can be accepted
    reg_job_instance->is_flushed = 0;
  }

  return err;
}


/**
 * Function: decision_manager_add_job
 *
 * Description: Calcuate job priority, split up data partitioned
 * jobs to multiple dependant sub-jobs, and add jobs to the
 * specific colored Qs based on the job priority
 *
 * Arguments:
 *   @th_session: Current session context
 *   @job: Job which has come in
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
th_err_t decision_manager_add_job(
  th_session_t *th_session,
  th_job_t *job
)
{
  th_err_t err = TH_SUCCESS;

  //decide the final job priority
  decide_job_priority(th_session, job);

  //based on job characteristics, divide into multiple jobs and setup the dependencies
  err = decide_job_partition(th_session, job);
  if (err != TH_SUCCESS) {
    TH_LOG_ERR(" Failed at decide_job_partition");
    return err;
  }

  //now add all the (sub)jobs to the appropriate Q
  err = add_job_to_queue(th_session, job);
  if (err != TH_SUCCESS) {
    TH_LOG_ERR(" Failed at add_job_to_queue");
    return err;
  }

  TH_LOG_LOW(" decision_manager_add_job successful");
  return err;
}


/**
 * Function: decision_manager_del_job
 *
 * Description: Delete a job and its sub jobs from the
 * assigned Q, and reclaim the slot(s) in the master job list
 *
 * Arguments:
 *   @th_session: Current session context
 *   @job: Job which has finished
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
th_err_t decision_manager_del_job(
  th_session_t *th_session,
  th_job_t *job
)
{
  th_err_t err = TH_SUCCESS;

  //remove this job and all it's sub jobs from the Q-s
  err = remove_job_from_queue(th_session, job);//don't care about error code

  //return the job slots taken by this job and all it's sub-jobs
  decision_manager_set_empty_slot(th_session, job);

  TH_LOG_LOW(" decision_manager_add_job successful");
  return err;
}


/**
 * Function: decision_manager_check_job_eligibility
 *
 * Description: For a job, submitted to a Q, decides what can
 * be done - a) Dispatch it, b) Put it to hold or c) Mark it as stopped
 *
 * Arguments:
 *   @th_session: Current session context
 *   @job: Job which needs to be evaluated
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
th_err_t decision_manager_check_job_eligibility(
  th_session_t *th_session,
  th_job_t *job
)
{
  th_err_t err = TH_FAILURE;
  int idx  = 0;
  th_queue_t *th_q = NULL;
  th_job_reg_t *reg_job = NULL;
  th_job_reg_instance_t *reg_job_instance = NULL;
  th_decision_manager_t *decision_mgr = NULL;

  if (IS_NULL(th_session) || IS_NULL(job)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  //NULL checks not necessary here, as these are already checked upstream
  decision_mgr = th_session->decision_mgr;
  reg_job_instance = (th_job_reg_instance_t *)job->reg_job_instance;
  reg_job = (th_job_reg_t *)reg_job_instance->reg_job;

  pthread_mutex_lock(&decision_mgr->master_joblist_lock);

  th_q = (th_queue_t *)job->th_q;
  idx = GET_PRIUORITY_Q_IDX(th_q->color);

  if (reg_job_instance->is_flushed) {//first check if the job is in flush mode
    err = TH_ERROR_JOB_STOPPED;
  } else if (!reg_job->is_fifo) {//then check if the job is non-fifo
    err = TH_SUCCESS;
  } else if (reg_job_instance->in_flight[idx]) {//finally check if the job is in flight
    if (job->state != TH_JOB_WAITING) {
      reg_job_instance->pend_count[idx]++;//jobs of this instance piling up
    }
    err = TH_ERROR_JOB_HOLD;
  } else {//if none of the above, the job is ready to fly
    reg_job_instance->in_flight[idx] = 1;
    err = TH_SUCCESS;
  }

  pthread_mutex_unlock(&decision_mgr->master_joblist_lock);

  return err;
}


/**
 * Function: decision_manager_register_job
 *
 * Description: Register a particular job instance to the Thread Library
 *
 * Arguments:
 *   @th_session: Current session context
 *   @arg1-argN: Rest correspond 1:1 with the Public API
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
th_err_t decision_manager_register_job(
  th_session_t *th_session,
  job_func_t job_func,
  char *job_func_name,
  job_callback_t flush_done_addr,
  int is_fifo,
  int session_key,
  unsigned int *handle
)
{
  th_err_t err = TH_SUCCESS;
  int idx = 0, idx2 = 0, vacant = -1;
  int new_job_added = 0;
  th_job_reg_t *reg_job = NULL;
  th_job_reg_instance_t *reg_job_instance = NULL;
  th_decision_manager_t *decision_mgr = NULL;

  decision_mgr = th_session->decision_mgr;
  if (IS_NULL(decision_mgr)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  pthread_mutex_lock(&decision_mgr->master_joblist_lock);

  for (idx = 0; idx < MAX_TH_UNIQUE_JOBS; idx++) {
    if (decision_mgr->reg_jobs[idx].job_addr == job_func) {
      reg_job = &decision_mgr->reg_jobs[idx];
      for (idx2 = 0; idx2 < MAX_REGISTERED_INSTANCES; idx2++) {
        if (!reg_job->job_reg_instances[idx2].handle) {
          reg_job_instance = &reg_job->job_reg_instances[idx2];
          break;
        }
      }
      //another instance of the job type wants to be registered
      //this will happen for job types which have multiple HW
      //and/or SW instances
      if (idx2 < MAX_REGISTERED_INSTANCES) {
        reg_job_instance->reg_job = (void *)reg_job;
        reg_job_instance->handle =
             get_new_handle((unsigned int)reg_job_instance, idx2, session_key);
        *handle = reg_job_instance->handle;
        reg_job->num_registered++;
        vacant = -1;
        new_job_added = 1;
        htable_add(&decision_mgr->hash_table, *handle,
                    (void*)reg_job, (void*)reg_job_instance);
      }
      break;
    } else if (-1 == vacant &&
               NULL == decision_mgr->reg_jobs[idx].job_addr) {
      vacant = idx;
    }
  }

  //new job type came in, register the first instance of this job type
  //more instances may follow, for jobs which have multiple HW
  //and/or SW instances
  if (vacant != -1) {
    reg_job = &decision_mgr->reg_jobs[vacant];

    reg_job->is_fifo = is_fifo;
    reg_job->job_addr = job_func;
    reg_job->job_func_name = job_func_name;
    reg_job->stop_cb_addr = flush_done_addr;
    reg_job->num_registered = 1;
    reg_job->job_reg_instances[0].handle =
          get_new_handle((unsigned int)&reg_job->job_reg_instances[0],
                          0, session_key);
    reg_job->job_reg_instances[0].reg_job = (void *)reg_job;
    *handle = reg_job->job_reg_instances[0].handle;
    new_job_added = 1;
    htable_add(&decision_mgr->hash_table, *handle,
                (void*)reg_job, (void*)&reg_job->job_reg_instances[0]);
  }

  pthread_mutex_unlock(&decision_mgr->master_joblist_lock);

  if (!new_job_added) {
    TH_LOG_ERR(" Could not register new job in Thread Library");
    *handle = 0;
    return TH_ERROR_FATAL;
  }

  TH_LOG_LOW(" decision_manager_register_job successful");
  return err;
}


/**
 * Function: decision_manager_stop_job
 *
 * Description: TBD
 *
 * Arguments:
 *   @th_session: Current session context
 *   @arg1-argN: Rest correspond 1:1 with the Public API
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
th_err_t decision_manager_stop_job(
  th_session_t *th_session,
  unsigned int handle,
  void *user_data,
  int is_blocking
)
{
  th_err_t err = TH_SUCCESS;
  int idx = 0;
  int send_flush_done = 0, vacant = -1;
  th_job_reg_t *reg_job = NULL;
  th_job_reg_instance_t *reg_job_instance = NULL;
  th_decision_manager_t *decision_mgr = NULL;

  decision_mgr = th_session->decision_mgr;
  if (IS_NULL(decision_mgr)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  pthread_mutex_lock(&decision_mgr->master_joblist_lock);

  htable_get(&decision_mgr->hash_table, handle,
              (void**)&reg_job, (void**)&reg_job_instance);
  if (IS_NULL(reg_job) || IS_NULL(reg_job_instance)) {
    pthread_mutex_unlock(&decision_mgr->master_joblist_lock);
    TH_LOG_ERR(" Job not found in Thread Library");
    return TH_ERROR_FATAL;
  }

  //update the job registry
  reg_job_instance->is_flushed = 1;
  reg_job_instance->is_flush_block = is_blocking;
  reg_job_instance->user_data = user_data;
  for (idx = 0; idx < MAX_TH_QUEUES; idx++) {
    //jobs of this type and instance no more need to wait
    reg_job_instance->pend_count[idx] = 0;
  }

  TH_LOG_MED("Job (%s) at stop request, job_count:%d",
                         reg_job->job_func_name, reg_job_instance->job_count);
  //if there are no more pending jobs, prepare to send flush_done
  if (reg_job_instance->job_count == 0) {
    send_flush_done = 1;
  }
  if (is_blocking) {
    //init semaphore
    sem_init(&reg_job_instance->msg_sem, 0, 0);
  }

  pthread_mutex_unlock(&decision_mgr->master_joblist_lock);

  if (send_flush_done) {
    //dont care about error code for now
    err = desicion_manager_send_flush_done(th_session, handle);
  } else {//set all jobs of this type & instance to completed
    for (idx = 0; idx < MAX_TH_QUEUES; idx++) {
      err = set_all_jobs(&th_session->priority_queues[idx],
                          reg_job_instance->handle, TH_JOB_COMPLETED);
    }
    //and trigger the dispatcher to process accordingly
    err = trigger_dispatcher(th_session, 0x7);
  }

  if (is_blocking) {
    if (TH_SUCCESS == err) {
      //wait on semaphore
      TH_LOG_MED("Blocking flush (%s), waiting on semaphore",
                                           reg_job->job_func_name);
      sem_wait(&reg_job_instance->msg_sem);
      TH_LOG_MED("Blocking flush (%s), coming out of wait",
                                           reg_job->job_func_name);
    }
    sem_destroy(&reg_job_instance->msg_sem);
  }

  TH_LOG_HIGH(" decision_manager_stop_job (%s) successful",
                                          reg_job->job_func_name);
  return err;
}


/**
 * Function: decision_manager_create_job_entry
 *
 * Description: Creates an entry in the master job list for the
 * job that is posted to the Thread Library
 *
 * Arguments:
 *   @th_session: Current session context
 *   @handle: Registered job handle for this job type/instance
 *   @job_params: Incoming job paramters of this job
 *   @this_job: OUT parameter, contains the job created
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
th_err_t decision_manager_create_job_entry(
  th_session_t *th_session,
  unsigned int handle,
  job_params_t *job_params,
  th_job_t **this_job
)
{
  int idx = 0, num_part = 0;
  th_job_reg_t *reg_job = NULL;
  th_job_reg_instance_t *reg_job_instance = NULL;
  th_job_t *job = NULL;
  th_decision_manager_t *decision_mgr = NULL;

  *this_job = NULL;

  decision_mgr = th_session->decision_mgr;
  if (IS_NULL(decision_mgr)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return -1;
  }

  pthread_mutex_lock(&decision_mgr->master_joblist_lock);

  htable_get(&decision_mgr->hash_table, handle,
              (void**)&reg_job, (void**)&reg_job_instance);
  if (IS_NULL(reg_job) || IS_NULL(reg_job_instance)) {
    pthread_mutex_unlock(&decision_mgr->master_joblist_lock);
    TH_LOG_ERR(" Current job not registered");
    return TH_ERROR_JOB_UNREGISTERED;
  }

  if (reg_job_instance->is_flushed) {
    pthread_mutex_unlock(&decision_mgr->master_joblist_lock);
    TH_LOG_HIGH(" Job (%s) is in flush mode, cannot accept job now",
                                               reg_job->job_func_name);
    return TH_ERROR_JOB_STOPPED;
  }

  idx = decision_manager_get_one_empty_slot(decision_mgr, reg_job_instance, 0);

  pthread_mutex_unlock(&decision_mgr->master_joblist_lock);

  if (idx != -1) {
    job = &th_session->decision_mgr->current_jobs[idx];
    job->handle = handle;
    job->reg_job_instance = (void *)reg_job_instance;
    job->job_func = reg_job->job_addr;;
    job->job_func_name = reg_job->job_func_name;
    job->job_slot = idx;
    //do the basic job initialization, some of these params can be changed later on
    job->stopped_cb = job_params->stopped_cb;
    job->priority = job_params->priority;
    job->relative_deadline = job_params->relative_deadline;
    job->is_splitable = job_params->is_splitable;
    job->job_ts = job_params->job_ts;
    job->state = TH_JOB_CREATED;

    if (!job_params->is_splitable) {
      //if the job is not splitable, it is expected to have one data payload
      job->data_ptr[0] = job_params->data_ptr;
      job->data_ptr[1] = NULL;
      num_part = 1;
    } else {
      //if the job is splitable, it will have multiple data payloads
      num_part = 0;
      void *temp = NULL;
      do {
        temp = ((void **)job_params->data_ptr)[num_part];
        job->data_ptr[num_part] = temp;
        num_part++;
      } while (temp != NULL && num_part < MAX_TH_DATAPART);
    }
    job->num_deps = num_part;
    *this_job = job;

    return TH_SUCCESS;
  }

  TH_LOG_ERR(" Couldn't find a slot to fit in current job request");
  return TH_ERROR_JOBLIST_FULL;
}


/**
 * Function: decision_manager_set_empty_slot
 *
 * Description: Job has either finished execute or stopped.
 * Reclaim the slot in the master list, and trigger dispatcher
 * if other jobs were pending on this one
 *
 * Arguments:
 *   @th_session: Current session context
 *   @job: Job which has finished
 *   @unblock_caller: Whether caller needs to be unblocked
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
th_err_t decision_manager_set_one_empty_slot(
  th_session_t *th_session,
  th_job_t *job,
  int unblock_caller
)
{
  th_err_t err = TH_SUCCESS;
  int idx = 0;
  int send_flush_done = 0, send_dispatch = 0;
  unsigned int trigger_type = 0;
  th_queue_t *th_q = NULL;
  th_job_reg_t *reg_job = NULL;
  th_job_reg_instance_t *reg_job_instance = NULL;
  th_decision_manager_t *decision_mgr = NULL;

  //NULL checks not necessary here, as these are already checked upstream
  decision_mgr = th_session->decision_mgr;
  reg_job_instance = (th_job_reg_instance_t *)job->reg_job_instance;
  reg_job = (th_job_reg_t *)reg_job_instance->reg_job;

  if (unblock_caller && job->blocking) {
    //try to see if caller can be unblocked on job completion
    job_end_notify(decision_mgr, job);
  }

  pthread_mutex_lock(&decision_mgr->master_joblist_lock);

  th_q = (th_queue_t *)job->th_q;
  idx = GET_PRIUORITY_Q_IDX(th_q->color);

  reg_job_instance->job_count--;

  if (reg_job_instance->is_flushed) {
    TH_LOG_MED(" Job %s flush pending, job count: %d", job->job_func_name,
                                                 reg_job_instance->job_count);
  }

  if (reg_job_instance->is_flushed && reg_job_instance->job_count == 0) {
    //check if it was the last job standing, post flush
    send_flush_done = 1;
  } else if (!reg_job_instance->is_flushed &&
              reg_job->is_fifo && reg_job_instance->pend_count[idx] > 0) {
    //check if a pending job needs to be dispatched
    send_dispatch = 1;
  }

  //if no job was pending, update the in-flight status in job registry
  if (!send_dispatch) {
    reg_job_instance->in_flight[idx] = 0;
  }

  //this slot gets appended to the free list
  decision_mgr->free_list[decision_mgr->free_list_tail].next = job->job_slot;
  decision_mgr->free_list_tail = job->job_slot;
  decision_mgr->job_occupancy--;

  pthread_mutex_unlock(&decision_mgr->master_joblist_lock);

  TH_LOG_LOW(" Job %s slotted at #%d is freed, current job occupancy: %d",
               job->job_func_name, job->job_slot, decision_mgr->job_occupancy);

  if (send_flush_done) {
    err = desicion_manager_send_flush_done(th_session, job->handle);
  }

  if (send_dispatch) {
    //set the first job that was waiting on this, back to contention
    err = set_first_job(th_session, job, TH_JOB_WAITING, TH_JOB_SUBMITTED);
    if (TH_SUCCESS == err) {//and trigger the dispatcher
      err = trigger_dispatcher(th_session, GET_TRIGGER_TYPE(job->priority));
    }
  }

  return err;
}


/**
 * Function: decision_manager_update_job_registry
 *
 * Description: Update job registry when the first pending
 * job becomes eligible for dispatch.
 *
 * Arguments:
 *   @th_session: Current session context
 *   @job: Job which is ready to fly
 *
 * Return values: None
 *
 * Notes: none
 **/
void decision_manager_update_job_registry(
  th_session_t *th_session,
  th_job_t *job
)
{
  int idx = 0;
  th_queue_t *th_q = NULL;
  th_job_reg_t *reg_job = NULL;
  th_job_reg_instance_t *reg_job_instance = NULL;
  th_decision_manager_t *decision_mgr = NULL;

  //NULL checks not necessary here, as these are already checked upstream
  decision_mgr = th_session->decision_mgr;
  reg_job_instance = (th_job_reg_instance_t *)job->reg_job_instance;
  reg_job = (th_job_reg_t *)reg_job_instance->reg_job;

  pthread_mutex_lock(&decision_mgr->master_joblist_lock);

  th_q = (th_queue_t *)job->th_q;
  idx = GET_PRIUORITY_Q_IDX(th_q->color);
  reg_job_instance->in_flight[idx] = 0;
  reg_job_instance->pend_count[idx]--;

  pthread_mutex_unlock(&decision_mgr->master_joblist_lock);

  return;
}


/**
 * Function: decision_manager_set_empty_slot
 *
 * Description: TBD
 *
 * Arguments:
 *   @th_session: Current session context
 *   @job: Job which has finished
 *
 * Return values: None
 *
 * Notes: none
 **/
void decision_manager_set_empty_slot(
  th_session_t *th_session,
  th_job_t *job
)
{
  int idx  = 0;
  th_job_t *job_next = NULL;
  th_decision_manager_t *decision_mgr = NULL;

  decision_mgr = th_session->decision_mgr;
  job_next = job;

  do {
    job = job_next;
    decision_manager_set_one_empty_slot(th_session, job, 0);
    if (job->next != -1) {
      job_next = &decision_mgr->current_jobs[job->next];
    }
  } while (job->next != -1);

  return;
}

