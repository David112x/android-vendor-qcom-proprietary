/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_timed_cb_sim.c
  @brief   RIL UTF timed callback simulation

  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#include "ril_utf_timed_cb_sim.h"
#include "ril_utf_core.h"

/*========================================================
 * Global Variables
========================================================*/
extern pthread_mutex_t main_thread_q_access;
extern pthread_mutex_t main_thread_ril_q_access;
extern struct timed_cb_t *timed_cb_list_head;
extern struct timed_cb_t *timed_cb_list_tail;
extern int q_empty;
extern pthread_cond_t main_thread_q_ready;
extern pthread_t callback_thread;
extern pthread_cond_t callback_thread_request;

/*========================================================
 * Local Variables
========================================================*/
pthread_mutex_t timer_list_access = PTHREAD_MUTEX_INITIALIZER;
struct timed_cb_t *cb_timer_head;
struct timed_cb_t *cb_timer_tail;


/*========================================================
  FUNCTION: timer_compare
    - returns 1 if a is greater than b
    - otherwise returns 0

  Critical section access:
  - None.
========================================================*/
int timer_compare( struct timespec a, struct timespec b)
{
  int ret_val = 1;
  if (b.tv_sec > a.tv_sec)
  {
    ret_val = 0;
  }

  if ( (b.tv_sec == a.tv_sec) &&
       (b.tv_nsec > a.tv_nsec) )
  {
    ret_val = 0;
  }

  return ret_val;
}

/*========================================================
  FUNCTION: create_timed_cb_list_node
  - allocates memory and initializes params for a node (to
    be added to a timed callback list).
  - returns a pointer to the node.

  Critical section access:
  - None.
========================================================*/
struct timed_cb_t* create_timed_cb_list_node ( RIL_TimedCallback callback, void *param, struct timespec timeout)
{
  struct timed_cb_t *temp = NULL;

  temp = (struct timed_cb_t *) malloc(sizeof(struct timed_cb_t));
  temp->callback = callback;
  temp->param    = param;
  temp->timeout  = timeout;
  temp->next     = NULL;
  temp->prev     = NULL;

  return temp;
}

/*========================================================
  FUNCTION: add_to_timed_cb_list
  - Adds a node to the end of the main thread dispatch
    queue.
  - upon completion, signal "main_thread_q_ready" (
    the main thread will be blocked on this signal).
  - This function can be called from a temporary timer
    thread, or from the main thread itself.

  Critical section access:
  - main_thread_q_access must be locked before using
    this function
  - Needed for access to the "timed_cb_list".
========================================================*/
void add_to_timed_cb_list( struct timed_cb_t *node )
{
  // reset node pointers
  node->next = NULL;
  node->prev = NULL;

  if( timed_cb_list_head )
  {
    timed_cb_list_tail->next = node;
  }
  else
  {
    timed_cb_list_head = node;
  }

  timed_cb_list_tail = node;

  q_empty = FALSE;

  // signal "main thread" that q is ready
  pthread_cond_signal(&main_thread_q_ready);

}


/*========================================================
  FUNCTION: start_callback_timer
  - Start a timer with the specified timeout.
  - A new (and temporary) timer thread will be created.

  Critical section access:
  - None
========================================================*/
void start_callback_timer(RIL_TimedCallback callback, void *param, const struct timeval *relativeTime)
{
  struct timespec   timeout;
  struct timeval tmp;

  if (relativeTime == NULL)
  {
    tmp.tv_sec = 0;
    tmp.tv_usec = 0;
  }
  else
  {
    tmp.tv_sec = relativeTime->tv_sec;
    tmp.tv_usec = relativeTime->tv_usec;
  }

  calc_timeout(&timeout, &tmp);

  pthread_mutex_lock(&timer_list_access);

  struct timed_cb_t *node = create_timed_cb_list_node (callback, param, timeout);
  enqueue_timed_cb(node);

  pthread_mutex_unlock(&timer_list_access);

  // signal timed_cb_thread
  pthread_cond_signal(&callback_thread_request);

  return;
}

/*========================================================
  FUNCTION: timed_cb_thread
    - used to keep track of outstanding timers

  Critical section access:
  - None
========================================================*/
void* timed_cb_thread(void *param)
{
  (void) param;
  struct timespec   timeout;
  struct timespec   temp;
  struct timeval e_timeout;
  int err_val;
  struct timeval    ctime;
  struct timed_cb_t *node;

  pthread_mutex_lock(&timer_list_access);

  // loop and handle callbacks as they timeout and additional timeout requests
  while (1)
  {
    e_timeout.tv_sec = 0;
    e_timeout.tv_usec = 1; // default timeout is 10 micro seconds

    calc_timeout(&timeout, &e_timeout);

    err_val = pthread_cond_timedwait(&callback_thread_request, &timer_list_access, &timeout);

    // Check for reboot request
    if (ril_utf_check_reboot_state() == 1)
    {
      ril_utf_timed_cb_thread_shutdown();
    }

    if (cb_timer_head != NULL)
    {
      // We are either waking up due to timeout or to a new node
      // In either case we need to check top node to see if it has expired
      gettimeofday(&ctime, NULL);
      temp.tv_sec = ctime.tv_sec;
      temp.tv_nsec = ctime.tv_usec * 1000;

      if (timer_compare(temp, cb_timer_head->timeout) != 0)
      {
        node = pop_top_node();
        MAIN_THRD_DISPATCH_Q_LOCK();
        add_to_timed_cb_list( node );
        MAIN_THRD_DISPATCH_Q_UNLOCK();
      }
    }

    if (cb_timer_head == NULL)
    {
      // no outstanding timeouts so just wait default timeout for new nodes / shutdown sig
      calc_timeout(&timeout, &e_timeout);
    }
    else
    {
      timeout = cb_timer_head->timeout;
    }


  } // end while

  pthread_mutex_unlock(&timer_list_access);
}


/*========================================================
  FUNCTION: enqueue_timed_cb
    - Rationale: This adds nodes to an ordered stack
      where the top of the stack is the cb with the
      shortest time left before its timer expires

  Critical section access:
  - timer_list_access must be locked
========================================================*/
void enqueue_timed_cb(struct timed_cb_t *node)
{
  struct timespec   timeout = node->timeout;
  struct timed_cb_t *ptr;

  // Step through list looking for location to insert
  for (ptr = cb_timer_head; ptr != NULL; ptr = ptr->next)
  {
    if (timer_compare(ptr->timeout, timeout))
    {
      node->prev = ptr->prev;
      node->next = ptr;
      if (cb_timer_head == ptr)
      {
        cb_timer_head = node;
      }
      else
      {
        ptr->prev->next = node;
      }
      ptr->prev = node;
      break;
    }
  }

  // Edge case where our insert is new last node
  if (ptr == NULL)
  {
    if (cb_timer_head == NULL)
    {
      cb_timer_head = node;
      cb_timer_tail = node;
    }
    else
    {
      cb_timer_tail->next = node;
      node->prev = cb_timer_tail;
      cb_timer_tail = node;
    }
  }

}

/*========================================================
  FUNCTION: pop_top_node

  Critical section access:
  - Main thread q must be locked
========================================================*/
struct timed_cb_t* pop_top_node()
{
  struct timed_cb_t *ptr = cb_timer_head;
  cb_timer_head = ptr->next;

  if (cb_timer_head == NULL)
  {
    cb_timer_tail = NULL;
  }
  else
  {
    cb_timer_head->prev = NULL;
  }

  return ptr;
}

/*========================================================
  FUNCTION: empty_timed_cb_queue

  Critical section access:
  - Main thread q must be locked
========================================================*/
void empty_timed_cb_queue()
{
  struct timed_cb_t *ptr;
  struct timed_cb_t *next;

  ptr = cb_timer_head;
  cb_timer_head = NULL;
  while (ptr != NULL)
  {
    next = ptr->next;
    free(ptr);
    ptr = next;
  }
}


/*========================================================
  FUNCTION: init_timed_cb_emulation
    - start timed callback thread and init globals

  Critical section access:
  - None
========================================================*/
void init_timed_cb_emulation()
{
  pthread_attr_t cb_thrd_attr;
  pthread_attr_init(&cb_thrd_attr);

  cb_timer_head = NULL;
  cb_timer_tail = NULL;

  pthread_create(&callback_thread, &cb_thrd_attr, timed_cb_thread, NULL);
}

/*========================================================
  FUNCTION: ril_utf_timed_cb_thread_shutdown
    - timed cb thread shutdown

  Critical section access:
  - None
========================================================*/
void ril_utf_timed_cb_thread_shutdown()
{
  empty_timed_cb_queue();
  pthread_mutex_unlock(&timer_list_access);

  pthread_exit(NULL);
}

/*========================================================
  END. UTL timer support implementation.
========================================================*/

