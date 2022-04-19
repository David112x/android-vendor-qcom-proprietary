/*========================================================================
Copyright (c) 2015 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
*//*====================================================================== */


#include "common.h"
#include "decision.h"
#include "dispatcher.h"

#define Q_SANITIZE_IDX(idx) \
            ((idx) = (((idx) < 0) ? MAX_TH_QUEUE_ELEMENTS - 1 : (idx)))

#define Q_INC_IDX(idx) \
            ((idx) = (((idx) + 1) & (MAX_TH_QUEUE_ELEMENTS - 1)))

#define Q_GET_NEXT_IDX(idx, offset) \
            (((idx) + (offset)) & (MAX_TH_QUEUE_ELEMENTS - 1))


/*========================================================================*/
/* INTERNAL FUNCTIONS */
/*========================================================================*/

/**
 * Function: find_q
 *
 * Description: Finds a priority Q from the job priority
 *
 * Arguments:
 *   @priority_queues: List of Priority Qs
 *   @priority: Job priority
 *
 * Return values: The specific priority Q
 *
 * Notes: none
 **/
static th_queue_t *find_q(
  th_queue_t *priority_queues,
  int priority
)
{
  th_err_t err = TH_SUCCESS;

  //simple implementation for now, translate priority number to Q

  switch (priority) {
    case JOB_PRIO_CRITICAL:
      return &priority_queues[0];
      break;
    case JOB_PRIO_HIGH:
      return &priority_queues[1];
      break;
    case JOB_PRIO_NORMAL:
      return &priority_queues[2];
      break;
    default:
      return NULL;
  }

  TH_LOG_LOW(" find_q successful");
  return NULL;
}


/**
 * Function: push_head_down
 *
 * Description: If an element of a Q is deleted, push all
 * elements starting from the head down
 *
 * Arguments:
 *   @q: Soecific priority Q
 *   @idx: Index in Q, where the element is deleted
 *
 * Return values: None
 *
 * Notes: none
 **/
static inline void push_head_down(
  th_queue_t *q,
  int idx
)
{
  int idx2 = 0;

  idx2 = (idx - 1);
  Q_SANITIZE_IDX(idx2);
  while (idx != q->head) {
    q->elt[idx] = q->elt[idx2];
    idx = (idx - 1);
    Q_SANITIZE_IDX(idx);
    idx2 = (idx2 - 1);
    Q_SANITIZE_IDX(idx2);
  }

  return;
}


/**
 * Function: pull_tail_up
 *
 * Description: If an element of a Q is deleted, pull all
 * elements starting from the tail up
 *
 * Arguments:
 *   @q: Soecific priority Q
 *   @idx: Index in Q, where the element is deleted
 *
 * Return values: None
 *
 * Notes: none
 **/
static inline void pull_tail_up(
  th_queue_t *q,
  int idx
)
{
  int idx2 = 0;

  idx2 = Q_GET_NEXT_IDX(idx, 1);
  while (idx2 != q->tail) {
    q->elt[idx] = q->elt[idx2];
    Q_INC_IDX(idx);
    Q_INC_IDX(idx2);
  }

  return;
}


/**
 * Function: is_idx_greater
 *
 * Description: Compare distances of two Q elements from head
 *
 * Arguments:
 *   @idx1: Element at idx1
 *   @idx2: Element at idx2
 *   @head: Q head
 *   @tail: Q tail
 *
 * Return values: 1 or 0, based on which element was further
 *
 * Notes: none
 **/
static inline int is_idx_greater(
  int idx1,
  int idx2,
  int head,
  int tail
)
{
  int d1 = 0;
  int d2 = 0;

  if (tail < head) {
    d1 = (idx1 < head) ? (MAX_TH_QUEUE_ELEMENTS - 1 - head + idx1) :
                                                                (idx1 - head);
    d2 = (idx2 < head) ? (MAX_TH_QUEUE_ELEMENTS - 1 - head + idx2) :
                                                                (idx2 - head);
  } else {
    d1 = idx1 - head;
    d2 = idx2 - head;
  }

  return (d1 > d2) ? 1 : 0;
}

/*========================================================================*/
/* INTERNAL INTERFACE IMPLEMENTATION */
/*========================================================================*/

/**
 * Function: get_from_q
 *
 * Description: Pops an element from the Q
 *
 * Arguments:
 *   @q: Specific Q
 *
 * Return values: Element popped
 *
 * Notes: none
 **/
void *get_from_q(
  th_queue_t *q
)
{
  th_err_t err = TH_SUCCESS;
  void *data = NULL;

  /* Q is empty */
  if (q->tail == q->head) {
    return NULL;
  }
  else {
    data = q->elt[q->head].data;
    Q_INC_IDX(q->head);
  }

  TH_LOG_LOW(" get_from_q(color:%d) successful, head:%d, tail:%d",
                                          q->color, q->head, q->tail);
  return data;
}


/**
 * Function: add_to_q
 *
 * Description: Push an element to the Q
 *
 * Arguments:
 *   @q: Specific Q
 *   @payload: Element to push
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
th_err_t add_to_q(
  th_queue_t *q,
  void *payload
)
{
  th_err_t err = TH_SUCCESS;

  /* Q is full */
  if (Q_GET_NEXT_IDX(q->tail, 1) == q->head) {
    err = TH_ERROR_JOBQ_FULL;
  }
  else {
    q->elt[q->tail].data = payload;
    q->elt[q->tail].dirty = 0;
    Q_INC_IDX(q->tail);
  }

  TH_LOG_LOW(" add_to_q(color:%d) successful, head:%d, tail:%d",
                                        q->color, q->head, q->tail);
  return err;
}


/**
 * Function: peek_from_q
 *
 * Description: Peeks the next element from the Q, based on
 * peek position. It doesn't remove the element from the Q
 *
 * Arguments:
 *   @q: Specific Q
 *
 * Return values: Element peeked
 *
 * Notes: none
 **/
q_element_t *peek_from_q(
  th_queue_t *q
)
{
  th_err_t err = TH_SUCCESS;
  int peeked_pos = 0;
  q_element_t *elt = NULL;

  if (q->head == q->tail || q->peek_pos == q->tail) {
    return NULL;
  }

  elt = &q->elt[q->peek_pos];
  peeked_pos = q->peek_pos;
  if (q->go_back) {
    q->peek_pos = q->go_back;
    q->go_back = 0;
  } else {
    Q_INC_IDX(q->peek_pos);
  }

  elt->dirty++;

  TH_LOG_LOW(" peek_from_q(color:%d) successful, from pos:%d, head:%d,"
                   " tail:%d ", q->color, peeked_pos, q->head, q->tail);

  return elt;
}


/**
 * Function: delete_from_q
 *
 * Description: Delete a job from the Q, and either push
 * the head down or pull the tail up
 *
 * Arguments:
 *   @q: Specific Q
 *   @data: Job to delete
 *   @q_pos: If non-zero, position to delete from (data param will be ignored)
 *   @pull_tail: How to compact the Q after delete
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
th_err_t delete_from_q(
  th_queue_t *q,
  void *data,
  int q_pos,
  int pull_tail
)
{
  th_err_t err = TH_SUCCESS;
  int idx = 0, idx2 = 0;
  void **array = NULL;

  idx = q->head;

  if (pull_tail) {//pull the tail up
    if (!q_pos) {
      while (idx != q->tail) {
        if (q->elt[idx].data == data) {
          pull_tail_up(q, idx);
          q->tail = q->tail - 1;
          Q_SANITIZE_IDX(q->tail);
          break;
        }
        Q_INC_IDX(idx);
      }
    } else {
      idx = Q_GET_NEXT_IDX(idx, q_pos);
      pull_tail_up(q, idx);
      q->tail = q->tail - 1;
      Q_SANITIZE_IDX(q->tail);
    }
  } else { //push the head down
    if (!q_pos) {
      while (idx != q->tail) {
        if (q->elt[idx].data == data) {
          push_head_down(q, idx);
          Q_INC_IDX(q->head);
          break;
        }
        Q_INC_IDX(idx);
      }
    } else {
      idx = Q_GET_NEXT_IDX(idx, q_pos);;
      push_head_down(q, idx);
      Q_INC_IDX(q->head);
    }
  }

  TH_LOG_LOW(" delete_from_q(color:%d) successful, head:%d, tail:%d",
                                           q->color, q->head, q->tail);
  return err;
}


/**
 * Function: compact_q
 *
 * Description: Compact the tainted Q. Multiple elements need to
 * be removed from the Q, as the jobs have either dispatched
 * or stopped
 *
 * Arguments:
 *   @q: Specific Q
 *
 * Return values: 1, if Q becomes empty after compaction
 *
 * Notes: It's an efficient O(1) operation
 **/
int compact_q(
  th_queue_t *q
)
{
  int ret = 0;
  int head = 0, tail = 0;
  int idx = 0, idx2 = 0;

  head = q->head;
  tail = q->tail;

  if (tail == head) {
    return TH_SUCCESS;
  }

  idx2 = head;
  idx = Q_GET_NEXT_IDX(head, 1);
  while (idx != tail) {
    if (idx != tail) {
      if (q->elt[idx2].dirty) {
        if (!q->elt[idx].dirty) {
          q->elt[idx2] = q->elt[idx];
          TH_LOG_LOW(" copying job(color:%d) from %d to %d",
                                        q->color, idx, idx2);
          q->elt[idx].dirty = 1;
        }
      }
      if (!q->elt[idx2].dirty) {
        Q_INC_IDX(idx2);
      }
      Q_INC_IDX(idx);
    }
  }

  if (!q->elt[idx2].dirty) {
    tail = Q_GET_NEXT_IDX(idx2, 1);
  } else {
    tail = idx2;
  }

  q->tail = tail;
  if (q->tail == q->head) {
    ret = 1;
  }

  TH_LOG_MED(" compact_q(color:%d) successful, head:%d, tail:%d",
                                        q->color, q->head, q->tail);
  return ret;
}


/**
 * Function: set_all_jobs
 *
 * Description: Set all job states with specific handle
 * in the specific Q, to to_state
 *
 * Arguments:
 *   @q: Specific Q
 *   @handle: Registered job handle
 *   @to_state: State to move the first job, with this handle, to
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
th_err_t set_all_jobs(
  void *q,
  unsigned int handle,
  th_job_state to_state
)
{
  int q_pos = 0, num_set = 0;
  th_queue_t *th_q = NULL;
  th_job_t *job = NULL;
  q_element_t *elt = NULL;

  th_q = (th_queue_t *)q;

  pthread_mutex_lock(&th_q->q_lock);

  q_pos = th_q->head;

  while (q_pos != th_q->tail) {
    elt = &th_q->elt[q_pos];
    job = (th_job_t *)elt->data;
    //found a match
    if (handle == job->handle) {
      job->state = to_state;
      //update the peek_pos, if needed
      if (!num_set &&
           is_idx_greater(th_q->peek_pos, q_pos, th_q->head, th_q->tail)) {
        th_q->peek_pos = q_pos;
      }
      num_set++;
    }
    Q_INC_IDX(q_pos);
  }

  pthread_mutex_unlock(&th_q->q_lock);

  TH_LOG_LOW(" set_all_jobs successful(color:%d), num_set:%d, to_state:%d",
                                            th_q->color, num_set, to_state);

  return TH_SUCCESS;
}


/**
 * Function: set_first_job
 *
 * Description: Set the state of the first job with specific
 * handle in the specific Q, from from_state to to_state
 *
 * Arguments:
 *   @th_session: Session Context
 *   @ref_job: Job which is making its next waiting one eligible
 *   @from_state: State to find the first matching job
 *   @to_state: State to move the first matching job to
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
th_err_t set_first_job(
  th_session_t *th_session,
  th_job_t *ref_job,
  th_job_state from_state,
  th_job_state to_state
)
{
  int q_pos = 0;
  th_queue_t *th_q = NULL;
  th_job_t *job = NULL;
  q_element_t *elt = NULL;

  th_q = ref_job->th_q;

  pthread_mutex_lock(&th_q->q_lock);

  //first update job registry to make a new (prev waiting) job eligible
  decision_manager_update_job_registry(th_session, ref_job);

  q_pos = th_q->head;
  while (q_pos != th_q->tail) {
    elt = &th_q->elt[q_pos];
    job = (th_job_t *)elt->data;
    //found a match
    if ((ref_job->handle == job->handle) && (from_state == job->state)) {
      job->state = to_state;
      if (is_idx_greater(th_q->peek_pos, q_pos, th_q->head, th_q->tail)) {
        //just peek this position, as a previously waiting job is now ready
        //after that, go back to the old peek position
        th_q->go_back = th_q->peek_pos;
        th_q->peek_pos = q_pos;
      }
      break;
    }
    Q_INC_IDX(q_pos);
  }

  pthread_mutex_unlock(&th_q->q_lock);

  if (job) {
    TH_LOG_LOW(" set_first_job(%s) successful(color:%d), pos:%d, to_state:%d",
                             job->job_func_name, th_q->color, q_pos, to_state);
  } else {
    TH_LOG_ERR(" set_first_job(color:%d) failed", th_q->color);
  }

  return TH_SUCCESS;
}


/**
 * Function: get_first_job
 *
 * Description: Get the first job in the specific Q, with
 *  state =TH_JOB_SUBMITTED or TH_JOB_COMPLETED
 *
 * Arguments:
 *   @th_session: Session context
 *   @th_q: Specific Q to get the job from
 *   @idx: We may need to update the dispatcher mask, indicates which bit
 *
 * Return values: Job got
 *
 * Notes: none
 **/
th_job_t * get_first_job(
  th_session_t *th_session,
  th_queue_t *th_q,
  int idx
)
{
  int ret = 0;
  th_err_t err = TH_SUCCESS;
  q_element_t *elt = NULL;
  th_job_t *job = NULL;

  pthread_mutex_lock(&th_q->q_lock);

  if (!(th_session->dispatcher->flash_mask & (0x1 << idx))) {
    pthread_mutex_unlock(&th_q->q_lock);
    return NULL;
  }

  while ((elt = peek_from_q(th_q)) != NULL) {
    if (elt->dirty > 1) {//the job has already been processed once
      continue;
    }
    job = (th_job_t *)elt->data;
    TH_LOG_LOW(" Peeked job(color:%d), name:%s, state:%d",
                 th_q->color, job->job_func_name, job->state);

    if (TH_JOB_COMPLETED == job->state) {//job was stopped
      break;
    } else if (TH_JOB_SUBMITTED == job->state) {//job can become ready or waiting
      err = decision_manager_check_job_eligibility(th_session, job);
      if (TH_ERROR_JOB_HOLD == err) {
        job->state = TH_JOB_WAITING;
        TH_LOG_MED(" job(%s) going to WAITING state(color:%d)",
                                  job->job_func_name, th_q->color);
      } else if (TH_SUCCESS == err) {//job will become ready, return it to dispatcher
        job->state = TH_JOB_READY;
        break;
      } else if (TH_ERROR_JOB_STOPPED == err) {//no need to dispatch, return to dispatcher
        job->state = TH_JOB_COMPLETED;
        break;
      }
    }
    elt->dirty = 0;
  }

  if (elt) {
    if (!th_q->need_to_compact) {
      //an element in the Q has become dirty, we definitely need
      //to compact the Q later on, when last element is peeked
      th_q->need_to_compact = 1;
    }
    pthread_mutex_unlock(&th_q->q_lock);
    TH_LOG_MED(" get_first_job(%s) successful(color:%d), head:%d, tail:%d",
                   job->job_func_name, th_q->color, th_q->head, th_q->tail);
  } else {
    if (th_q->need_to_compact) {
      //last element is peeked, now compact the Q
      ret = compact_q(th_q);
      th_q->need_to_compact = 0;
    }
    th_q->peek_pos = th_q->head;
    th_q->go_back = 0;

    //if the Q of this COLOR is empty, turn the specific dispatcher mask OFF
    if (ret) {
      pthread_mutex_lock(&th_session->dispatcher->thread_lock);
      th_session->dispatcher->flash_mask &= ~(0x1 << idx);
      pthread_mutex_unlock(&th_session->dispatcher->thread_lock);
    }

    pthread_mutex_unlock(&th_q->q_lock);
    job = NULL;
    TH_LOG_LOW("No jobs to get(color:%d)", th_q->color);
  }

  return job;
}


/**
 * Function: init_queues_lists
 *
 * Description: Inits the Priority Q-s structures
 *
 * Arguments:
 *   @th_session: Session context
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
th_err_t init_queues_lists(th_session_t *th_session)
{
  th_err_t err = TH_SUCCESS;
  int i = 0, j = 0;

  //init worker Qs
  for (i = 0; i < MAX_TH_QUEUES; i++) {
    th_session->priority_queues[i].head = 0;
    th_session->priority_queues[i].tail = 0;
    th_session->priority_queues[i].peek_pos = 0;
    th_session->priority_queues[i].go_back = 0;
    th_session->priority_queues[i].need_to_compact = 0;

    for (j = 0; j < MAX_TH_QUEUE_ELEMENTS; j++) {
      th_session->priority_queues[i].elt[j].data = NULL;
    }

    //this is a hard coding of the Q colors, but its OK since
    //the priorities are not foreseen to be changed in future
    if (i == 0) {
      th_session->priority_queues[i].color = DISP_FLASH_RED;
    } else if (i == 1) {
      th_session->priority_queues[i].color = DISP_FLASH_ORANGE;
    } else if (i == 2) {
      th_session->priority_queues[i].color = DISP_FLASH_GREEN;
    }

    pthread_mutex_init(&th_session->priority_queues[i].q_lock, NULL);
  }

  //init manager Q
  th_session->manager_queue.head = 0;
  th_session->manager_queue.tail = 0;
  pthread_mutex_init(&th_session->manager_queue.q_lock, NULL);

  TH_LOG_MED(" init_queues_lists successful");
  return err;
}


/**
 * Function: deinit_queues_lists
 *
 * Description: De-inits the Priority Q-s structures
 *
 * Arguments:
 *   @th_session: Session context
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
th_err_t deinit_queues_lists(th_session_t *th_session)
{
  th_err_t err = TH_SUCCESS;
  int i = 0;

  for (i = 0; i < MAX_TH_QUEUES; i++) {
    pthread_mutex_destroy(&th_session->priority_queues[i].q_lock);
  }

  pthread_mutex_destroy(&th_session->manager_queue.q_lock);

  TH_LOG_MED(" deinit_queues_lists successful");
  return err;
}


/**
 * Function: add_job_to_queue
 *
 * Description: Add a new job and its dependant jobs to a Q
 *
 * Arguments:
 *   @th_session: Session context
 *   @job: Incoming job, processed by Decision Manager
 *
 * Return values: Thread Library standard error
 *
 * Notes: none
 **/
th_err_t add_job_to_queue(
  th_session_t *th_session,
  th_job_t *job
)
{
  th_err_t err = TH_SUCCESS;
  int priority = 0;
  th_queue_t *th_q = NULL;
  th_job_t *job_next = NULL;
  th_decision_manager_t *decision_mgr = NULL;
  th_dispatcher_t *dispatcher = NULL;

  decision_mgr = th_session->decision_mgr;
  dispatcher = th_session->dispatcher;
  if (IS_NULL(decision_mgr) || IS_NULL(dispatcher)) {
    TH_LOG_ERR(" Encountered NULL pointer");
    return TH_ERROR_NULLPTR;
  }

  priority = job->priority;
  job_next = job;

  do {
    job = job_next;
    if (IS_NULL(th_q)) {//all sub jobs of this job go to the same Q
      th_q = &th_session->priority_queues[job->priority - 1];
    }

    pthread_mutex_lock(&th_q->q_lock);
    err = add_to_q(th_q, job);
    if (err != TH_SUCCESS) {
      pthread_mutex_unlock(&th_q->q_lock);
      TH_LOG_ERR(" Failed to add new job at add_to_q");
      return err;
    }
    job->state = TH_JOB_SUBMITTED;
    job->th_q = (void *)th_q;//store the Q pointer for future use

    //trigger the dispatcher to schedule this job to threads
    err = trigger_dispatcher(th_session, GET_TRIGGER_TYPE(priority));
    pthread_mutex_unlock(&th_q->q_lock);

    if (job->next != -1) {
      job_next = &decision_mgr->current_jobs[job->next];
    }
  } while (job->next != -1);

  TH_LOG_MED(" add_job_to_queue (%s) successful (color:%d), head:%d, tail:%d",
                      job->job_func_name, th_q->color, th_q->head, th_q->tail);
  return err;
}


/**
 * Function: remove_job_from_queue
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
th_err_t remove_job_from_queue(
  th_session_t *th_session,
  th_job_t *job
)
{
  th_err_t err = TH_SUCCESS;
  th_queue_t *th_q = NULL;
  th_job_t *job_next = NULL;
  th_decision_manager_t *decision_mgr = NULL;

  decision_mgr = th_session->decision_mgr;
  job_next = job;

  do {
    job = job_next;
    if (IS_NULL(th_q)) {//all sub jobs of this job go to the same Q
      th_q = &th_session->priority_queues[job->priority - 1];
    }

    //delete from Q under the Q lock
    pthread_mutex_lock(&th_q->q_lock);
    err = delete_from_q(th_q, job, -1, 0);
    pthread_mutex_unlock(&th_q->q_lock);
    if (err != TH_SUCCESS) {
      TH_LOG_ERR(" Failed to add new job at add_to_q");
      return err;
    }

    if (job->next != -1) {
      job_next = &decision_mgr->current_jobs[job->next];
    }
  } while (job->next != -1);

  TH_LOG_LOW(" remove_job_from_queue successful");
  return err;
}

