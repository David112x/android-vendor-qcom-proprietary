/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_misc.c
  @brief   RIL UTF Misc. implementations

  DESCRIPTION
  Code for testing the test framework goes here. Also includes a SIGALRM based
  implementation for timed callbacks.
  ---------------------------------------------------------------------------
******************************************************************************/
// rild

struct timed_cb_list_t {
  RIL_TimedCallback callback;
  void *param;
  struct timeval *time_interval;
  struct timed_cb_list_t *next;
};

struct timed_cb_list_t *timed_cb_list = NULL;
struct timeval rild_timeval;
struct timeval last_set_timer;
struct itimerval rild_timer;
struct sigaction sigact;
int    is_RIL_requestTimedCallback_first_time = 1;

void update_timers( void );

/*
  If timeval microseconds out of range (i.e > 1000000)
  update the tv_sec / tv_usec fields accordingly
*/
#define MILLION_MICROSECONDS 1000000
int adjust_timeval_usec(struct timeval *x)
{
  if( x->tv_usec > MILLION_MICROSECONDS)
  {
    int nsec = (int) x->tv_usec /MILLION_MICROSECONDS;
    x->tv_sec += nsec;
    x->tv_usec -= (nsec * MILLION_MICROSECONDS);
  }
}

/*
 Return -1 if X < Y,  0 if x == Y, 1 if X > Y
*/
int diff_timeval(struct timeval *x, struct timeval *y )
{
  int ret_val;

  if ( x->tv_sec < y->tv_sec  )
  {
    ret_val = -1; // x < y
  }
  else if ( x->tv_sec == y->tv_sec )
  {
    if( x->tv_usec == y->tv_usec )
    {
      ret_val = 0; //x == y
    }
    else if( x->tv_usec < y->tv_usec )
    {
      ret_val = -1; // x< y
    }
    else
    {
      ret_val = 1; // x>y
    }
  }
  else
  {
    ret_val = 1; // x > y
  }

  return ret_val;
}

void qcril_timed_callback_dispatch_test
(
  void *param
)
{
  time_t t;

  time(&t);
  RIL_UTF_DEBUG("\n call back, timer id: %d @ %s", (int) param, ctime(&t));
}
/*
  Remove the first element in the cb list
  issue the callback
  set new timer
*/
void remove_from_cb_list()
{
  struct timed_cb_list_t *to_delete = timed_cb_list;
  struct itimerval timer_value;

  memset(&timer_value, 0x00, sizeof(struct itimerval));

  if ( timed_cb_list != NULL )
  {
    // Issue call back for expired timer
    // TBD: place in pending list here
    while( 1 )
    {
      to_delete = timed_cb_list;
      to_delete->callback( to_delete->param );

      // Remove expired timer from list
      timed_cb_list = timed_cb_list->next;
      free(to_delete->time_interval);
      free(to_delete);

      if( !timed_cb_list || diff_timeval(timed_cb_list->time_interval, &last_set_timer ) == 1)
      {
        break;
      }
    }

    // decrement all timers by amount
    update_timers();

    if( timed_cb_list )
    {
      // start next timer
      timer_value.it_value    = *timed_cb_list->time_interval;

      // store this value. It is used to reset all timers at expiry
      memcpy(&last_set_timer, &timer_value.it_value, sizeof(struct timeval));
      setitimer(ITIMER_REAL, &timer_value, NULL );
    }
  }
}

void sigHandler(int sigNum)
{
  switch( sigNum )
  {
    case SIGALRM:
      RIL_UTF_DEBUG("\n SIGALRM....");
      remove_from_cb_list();
      break;
    default:
      RIL_UTF_DEBUG("\n SIG#....%d", sigNum);
      break;
  }
}

void test_timer()
{
  rild_timeval.tv_sec = 1;
  rild_timeval.tv_usec = 200000;

  rild_timer.it_interval = rild_timeval;
  rild_timer.it_value    = rild_timeval;

  struct sigaction sigact;
  sigemptyset(&sigact.sa_mask);
  sigact.sa_flags   = 0;
  sigact.sa_handler = sigHandler;

  sigaction(SIGALRM, &sigact, NULL);

  setitimer(ITIMER_REAL, &rild_timer, NULL );
}


/*
 result = X - Y
 assumption: X > Y (always)
*/
void timeval_subtract_internal(struct timeval *result, struct timeval *x, struct timeval *y)
{
  result->tv_sec = x->tv_sec - y->tv_sec;

  if( x->tv_usec < y->tv_usec )
  {
    result->tv_sec -= 1; //adjust for carryover
    result->tv_usec = MILLION_MICROSECONDS + x->tv_usec - y->tv_usec;
  }
  else
  {
    result->tv_usec = x->tv_usec - y->tv_usec;
  }
}
/* Subtract the `struct timeval' values X and Y,
   result =  |X-Y| (i.e absolute value)
   Return -1 if X < Y,  0 if x == Y, 1 if X > Y  */

int timeval_subtract ( struct timeval *result, struct timeval *x, struct timeval *y )
{
  int ret_val;

  adjust_timeval_usec(x);
  adjust_timeval_usec(y);

  ret_val = diff_timeval(x, y);

  if( ret_val == 1 || ret_val == 0) // X >= Y
  {
    timeval_subtract_internal(result, x, y);
  }
  else if( ret_val == -1 ) // X < Y
  {
    timeval_subtract_internal(result, y, x);
  }

  return ret_val;
}

/*
  Decrease all timers in the timed cb list by
  the difference of the current timer's it_interval
  and it_value.
*/
void update_timers( void )
{
  struct itimerval curr_timerval;
  struct timeval   time_elapsed;
  struct timed_cb_list_t *temp_lcl = timed_cb_list;

  //get current timer value
  getitimer(ITIMER_REAL, &curr_timerval);

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: timer val %d", curr_timerval.it_value.tv_sec);

  //Find out time elapsed
  //last_set_timer has the original timer value
  //it_value has the time remaining until reset
  //last_set_timer - it_value = time elapsed
  timeval_subtract(&time_elapsed, &last_set_timer, &curr_timerval.it_value);

  //update all timervals (i.e subtract time elapsed since last timer reset)
  while( temp_lcl )
  {
    // TBD: remove expired timers and place into pending list
    timeval_subtract(temp_lcl->time_interval, temp_lcl->time_interval, &time_elapsed);
    temp_lcl = temp_lcl->next;
  }
}

struct timed_cb_list_t* create_timed_cb_list_entry(RIL_TimedCallback callback, void *param, const struct timeval *relativeTime)
{
  struct timed_cb_list_t *node = NULL;
  node = (struct timed_cb_list_t *) malloc(sizeof(struct timed_cb_list_t));
  node->callback      = callback;
  node->param         = param;
  node->time_interval = (struct timeval *) malloc(sizeof(struct timeval));
  memcpy(node->time_interval, relativeTime , sizeof(struct timeval));
  node->next          = NULL;
}

/*
Adding:
   - If list empty, add cb
    - start timer, return
   - if not empty
    - go through the list and add at the right place
    - store in increasing order of time remaining
    - if adding to head of list, get and set new timer resolution

*/
void add_to_callback_list(RIL_TimedCallback callback, void *param, const struct timeval *relativeTime)
{

  struct timed_cb_list_t *node = create_timed_cb_list_entry(callback, param, relativeTime);
  struct itimerval timer_value;

  // This timer_value will be used only if the new cb timer is the lowest of all
  // outstanding timers( or if there are no outstanding timers at all)
  memset(&timer_value, 0x00, sizeof(struct itimerval));
  timer_value.it_value    = *node->time_interval;

  if ( timed_cb_list == NULL )
  {
    // add first callback to cb timer list
    timed_cb_list =  node;

    // store this value. It is used to reset all timers at expiry
    memcpy(&last_set_timer, &timer_value.it_value, sizeof(struct timeval));

    // start timer
    setitimer(ITIMER_REAL, &timer_value, NULL );
  }
  else
  {
    struct timed_cb_list_t *temp = timed_cb_list;
    struct timed_cb_list_t *prev = NULL;
    int relative_time_still_greatest;

    // timed cb list maintained in ascending order
    while ( temp != NULL && diff_timeval(temp->time_interval, (struct timeval *)relativeTime) != 1)
    {
       prev = temp;
       temp = temp->next;
    }

    if ( temp == timed_cb_list )
    {
     // decrease all timers by the amount of time
     // expired since the last timer reset. We do this
     // since a new timer, with lower value than the
     // current timer has been found.
     update_timers();

     // insert new cb at the head of the list
     timed_cb_list = node;
     timed_cb_list->next = temp;

     // store this value. It is used to reset all timers at expiry
     memcpy(&last_set_timer, &timer_value.it_value, sizeof(struct timeval));

     // set timer again to new value
     setitimer(ITIMER_REAL, &timer_value, NULL );
    }
    else
    {
     // insert in between or at end of list. No need to
     // set timer
     prev->next = node;
     node->next = temp;
    }
  }
}

void RIL_requestTimedCallback (RIL_TimedCallback callback, void *param, const struct timeval *relativeTime)
{
  // Setup SIGALRM hndlr only for the very first call
  if ( is_RIL_requestTimedCallback_first_time )
  {
    is_RIL_requestTimedCallback_first_time = 0; // reset first time counter

    sigemptyset(&sigact.sa_mask);
    sigact.sa_flags   = 0;
    sigact.sa_handler = sigHandler;
    sigaction(SIGALRM, &sigact, NULL);
  }

  if( relativeTime == NULL )
  {
    // TBD: Issue callback immediately...modve into add_to_cb_list
  }
  else
  {
    add_to_callback_list( callback, param, relativeTime);
  }
}

void test_timed_call_backs()
{
  int timer_id;
  struct timeval time_val;
  time_t t;

  time (&t);
  RIL_UTF_DEBUG("\n Starting @ %s\n", ctime(&t));
  // first timer
  timer_id = 1;
  time_val.tv_sec = 2;
  time_val.tv_usec = 0;
  RIL_requestTimedCallback(qcril_timed_callback_dispatch_test, (void *) timer_id, &time_val);

  timer_id = 11;
  time_val.tv_sec = 2;
  time_val.tv_usec = 0;
  RIL_requestTimedCallback(qcril_timed_callback_dispatch_test, (void *) timer_id, &time_val);

  timer_id = 2;
  time_val.tv_sec = 5;
  time_val.tv_usec = 0;
  RIL_requestTimedCallback(qcril_timed_callback_dispatch_test, (void *) timer_id, &time_val);

  timer_id = 3;
  time_val.tv_sec = 3;
  time_val.tv_usec = 0;
  RIL_requestTimedCallback(qcril_timed_callback_dispatch_test, (void *) timer_id, &time_val);

  timer_id = 4;
  time_val.tv_sec = 1;
  time_val.tv_usec = 0;
  RIL_requestTimedCallback(qcril_timed_callback_dispatch_test, (void *) timer_id, &time_val);
}

void qcril_timed_callback_dispatch_test
(
  void *param
)
{
  time_t t;

  time(&t);
  RIL_UTF_DEBUG("\n call back, timer id: %d @ %s", (int) param, ctime(&t));
}


void* test_callback_timer_1( void *param )
{
  int timer_id;
  struct timeval time_val;
  time_t t;

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Inside test thread...");
  time (&t);
  RIL_UTF_DEBUG("\n Starting  1 @ %s\n", ctime(&t));
  // first timer
  timer_id = 11;
  time_val.tv_sec = 3;
  time_val.tv_usec = 0;
  RIL_requestTimedCallback(qcril_timed_callback_dispatch_test, (void *) timer_id, &time_val);
}

void test_start_callback_timer_1()
{
  pthread_attr_t timer_thrd_attr;
  pthread_t timer_thread_id;

  pthread_attr_init(&timer_thrd_attr);
  pthread_attr_setdetachstate(&timer_thrd_attr, PTHREAD_CREATE_DETACHED);

  pthread_create(&timer_thread_id, &timer_thrd_attr, test_callback_timer_1, NULL);
}

void* test_callback_timer_2( void *param )
{
  int timer_id;
  struct timeval time_val;
  time_t t;

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Inside test thread...");
  time (&t);
  RIL_UTF_DEBUG("\n Starting  2 @ %s\n", ctime(&t));
  // first timer
  timer_id = 12;
  time_val.tv_sec = 3;
  time_val.tv_usec = 0;
  RIL_requestTimedCallback(qcril_timed_callback_dispatch_test, (void *) timer_id, &time_val);
}

void test_start_callback_timer_2()
{
  pthread_attr_t timer_thrd_attr;
  pthread_t timer_thread_id;

  pthread_attr_init(&timer_thrd_attr);
  pthread_attr_setdetachstate(&timer_thrd_attr, PTHREAD_CREATE_DETACHED);

  pthread_create(&timer_thread_id, &timer_thrd_attr, test_callback_timer_2, NULL);
}
void* test_callback_timer_3( void *param )
{
  int timer_id;
  struct timeval time_val;
  time_t t;

  RIL_UTF_DEBUG("\n QMI_RIL_UTL: Inside test thread...");
  time (&t);
  RIL_UTF_DEBUG("\n Starting  3 @ %s\n", ctime(&t));
  // first timer
  timer_id = 13;
  time_val.tv_sec = 3;
  time_val.tv_usec = 0;
  RIL_requestTimedCallback(qcril_timed_callback_dispatch_test, (void *) timer_id, &time_val);
}

void test_start_callback_timer_3()
{
  pthread_attr_t timer_thrd_attr;
  pthread_t timer_thread_id;

  pthread_attr_init(&timer_thrd_attr);
  pthread_attr_setdetachstate(&timer_thrd_attr, PTHREAD_CREATE_DETACHED);

  pthread_create(&timer_thread_id, &timer_thrd_attr, test_callback_timer_3, NULL);
}

void test_timed_call_backs()
{
  int timer_id;
  struct timeval time_val;
  time_t t;

  time (&t);
  RIL_UTF_DEBUG("\n Starting @ %s\n", ctime(&t));
  // first timer
  timer_id = 1;
  time_val.tv_sec = 2;
  time_val.tv_usec = 0;
  RIL_requestTimedCallback(qcril_timed_callback_dispatch_test, (void *) timer_id, &time_val);

  timer_id = 5;
  time_val.tv_sec = 2;
  time_val.tv_usec = 0;
  RIL_requestTimedCallback(qcril_timed_callback_dispatch_test, (void *) timer_id, &time_val);

  timer_id = 2;
  time_val.tv_sec = 5;
  time_val.tv_usec = 0;
  RIL_requestTimedCallback(qcril_timed_callback_dispatch_test, (void *) timer_id, &time_val);

  timer_id = 3;
  time_val.tv_sec = 3;
  time_val.tv_usec = 0;
  RIL_requestTimedCallback(qcril_timed_callback_dispatch_test, (void *) timer_id, &time_val);

  timer_id = 4;
  time_val.tv_sec = 1;
  time_val.tv_usec = 0;
  RIL_requestTimedCallback(qcril_timed_callback_dispatch_test, (void *) timer_id, &time_val);

  test_start_callback_timer_1();
  test_start_callback_timer_2();
  test_start_callback_timer_3();

  // immediate timer
  timer_id = 6;
  RIL_requestTimedCallback(qcril_timed_callback_dispatch_test, (void *) timer_id, NULL);

  // immediate timer
  timer_id = 7;
  RIL_requestTimedCallback(qcril_timed_callback_dispatch_test, (void *) timer_id, NULL);
  // immediate timer
  timer_id = 8;
  RIL_requestTimedCallback(qcril_timed_callback_dispatch_test, (void *) timer_id, NULL);
}

void prop_testing()
{
  test_property_get();
  property_set("persist.radio.adb_log_on", "0");
  test_property_get();
}

int test_property_get(void)
{
  char args[96];
  int len;
  char *end_ptr;
  unsigned long ret_val;

  property_get( "persist.radio.adb_log_on", args, "" );
  len = strlen( args );
  RIL_UTF_DEBUG("\n QMI_RIL_UTL: len %d \n", len);
  if ( len > 0 )
  {
    ret_val = strtoul( args, &end_ptr, 0 );
    if ( ( errno == ERANGE ) && ( ( ret_val == ULONG_MAX ) || ( ret_val == 0 ) ) )
    {
        QCRIL_LOG_ERROR( "Fail to convert adb_log_on setting %s", args );
    }
    else if ( ret_val > 1 )
    {
      QCRIL_LOG_ERROR( "Invalid saved adb_log_on setting %ld, use default", ret_val );
    }
    else
    {
      QCRIL_LOG_MSG(MSG_LEGACY_HIGH,"return_value in MSG_LOG: %d\n", ret_val );
    }
  }
}

