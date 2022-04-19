/***************************************************************************
* Copyright (c) 2012-2015 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
****************************************************************************/

#include "dynamic_work_partition.h"

#ifndef __ANDROID__
#include "windows.h"
#include <process.h>
#define IDBG_ERROR printf
#define IDBG_HIGH printf
#endif

#ifdef __ANDROID__
static pthread_mutex_t   mutexCmd = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t   mutexMsg = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t    condMsg = PTHREAD_COND_INITIALIZER;
static pid_t oldpid = 0;
static pid_t currentpid = 0;
#else
static os_mutex_t   mutexCmd;
static os_mutex_t   mutexMsg;
static os_cond_t    condMsg;
static int32_t oldpid = 0;
static int32_t currentpid = 0;
#endif

static msg_payload_t     msgFlag;
static thread_param_struct_t threadControlParameter[NMAXTHREADS];
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

/*===========================================================================

Function            : dynamic_work_partition_init

Description         : Create max number of threads if not already created
                      Only once per process

Input parameter(s)  : size of payload

Output parameter(s) : 0/1

=========================================================================== */

int32_t dynamic_work_partition_init(uint32_t size)
{
  int32_t rc,i,j,readyFlag;
#ifdef __ANDROID__
  pthread_t threadid[NTHREADS];
#else
  os_thread_t threadid[NTHREADS];
#endif

  //Lock mutex
#ifdef __ANDROID__
  rc = pthread_mutex_lock(&mutexCmd);
#else
  currentpid = _getpid();
  if (oldpid != currentpid ) {
    os_mutex_init(&mutexCmd);
    os_mutex_init(&mutexMsg);
    os_cond_init(&condMsg);
  }
  rc = os_mutex_lock(&mutexCmd);
#endif
  if (rc != 0) {
    IDBG_ERROR("Mutex lock failed %d\n", rc);
    return 1;
  }

  for (i = 0; i < NTHREADS; ++i) {
    threadControlParameter[i].pthreadPayload = (void*) malloc (size);
    if (threadControlParameter[i].pthreadPayload == NULL) {
      IDBG_ERROR("Memory alloc failed\n");
      for (j=0; j < i; j++)
        free(threadControlParameter[j].pthreadPayload );
      oldpid = 0;
      return 1;
    }
#ifdef __ANDROID__
    currentpid = getpid();
#else
    currentpid = _getpid();
#endif
    if (oldpid != currentpid ) {
      threadControlParameter[i].threadCmdFlag = CMD_IDLE;
      threadControlParameter[i].threadMsgFlag = MSG_IDLE;
      threadControlParameter[i].pcondMsg = &condMsg;
      threadControlParameter[i].pmutexCmd = &mutexCmd;
      threadControlParameter[i].pmutexMsg = &mutexMsg;
#ifdef __ANDROID__
      pthread_cond_init(&threadControlParameter[i].threadCondCmd ,NULL);
      rc = pthread_create(&threadid[i],
        NULL,
        &dynamic_work_thread_func,
        &threadControlParameter[i]);
#else
      os_cond_init(&threadControlParameter[i].threadCondCmd);
      rc = os_thread_create(&threadid[i],
        (os_thread_func_t) &dynamic_work_thread_func,
        &threadControlParameter[i]);
#endif
      if (rc != 0) {
        IDBG_ERROR("Thread Create failed rc %d i %d\n", rc, i);
        for (j=0; j < i; j++)
          free(threadControlParameter[j].pthreadPayload );
        oldpid = 0;
        return 1;
      }
      pthread_setname_np(threadid[i], "CAM_imgLibWrkr");
    }
  }
  if (oldpid != currentpid) {
    //Wait for ready signal from the worker threads
    for (;;) {
#ifdef __ANDROID__
      rc = pthread_cond_wait(&condMsg, &mutexCmd);
#else
      rc = os_cond_wait(&condMsg, &mutexCmd);
#endif
      if (rc != 0) {
        IDBG_ERROR("Cond wait failed %d\n", rc);
        for (j=0; j < NTHREADS; j++)
          free(threadControlParameter[j].pthreadPayload );
        oldpid = 0;
        return 1;
      }
      readyFlag = TRUE;
      for (i = 0; i < NTHREADS; ++i) {
        if (threadControlParameter[i].threadMsgFlag != MSG_READY) {
          readyFlag = FALSE;
        }
      }
      //If all threads are ready, exit for
      if (readyFlag == TRUE) break;
    }
  }
  msgFlag = MSG_READY;
  oldpid = currentpid;
  //unLock mutex
#ifdef __ANDROID__
  rc = pthread_mutex_unlock(&mutexCmd);
#else
  rc = os_mutex_unlock(&mutexCmd);
#endif
  if (rc != 0) {
    IDBG_ERROR("Mutex unlock failed %d\n", rc);
    for (j=0; j < NTHREADS; j++)
      free(threadControlParameter[j].pthreadPayload );
    oldpid = 0;
    return 1;
  }
  return 0;
}

/*===========================================================================

Function            : dynamic_work_partition_deinit

Description         : Free the memory of payload

Input parameter(s)  : void

Output parameter(s) : 0/1

=========================================================================== */

int32_t dynamic_work_partition_deinit(void)
{
  int32_t rc,i;
  //Lock mutex
#ifdef __ANDROID__
  rc = pthread_mutex_lock(&mutexCmd);
#else
  rc = os_mutex_lock(&mutexCmd);
#endif
  if (rc != 0) {
    IDBG_ERROR("Mutex lock failed %d\n", rc);
    oldpid = 0;
    return 1;
  }

  for (i = 0; i < NTHREADS; ++i) {
    free(threadControlParameter[i].pthreadPayload);
  }
  //unLock mutex
#ifdef __ANDROID__
  rc = pthread_mutex_unlock(&mutexCmd);
#else
  rc = os_mutex_unlock(&mutexCmd);
#endif
  if (rc != 0) {
    IDBG_ERROR("Mutex unlock failed %d\n", rc);
    oldpid = 0;
    return 1;
  }
  return 0;
}

/*===========================================================================

Function            : dynamic_work_partition_execute

Description         : Execute and wait for thread completion

Input parameter(s)  : numberofthreads

Output parameter(s) : void

=========================================================================== */

int32_t dynamic_work_partition_execute(int32_t numberofthreads) {

  int32_t rc,doneFlag,i;
  //Lock msg mutex
#ifdef __ANDROID__
  rc = pthread_mutex_lock(&mutexMsg);
#else
  rc = os_mutex_lock(&mutexMsg);
#endif
  if (rc != 0) {
    IDBG_ERROR("Mutex lock failed %d\n", rc);
    return 1;
  }

  for (i=0; i<numberofthreads; i++) {
    threadControlParameter[i].threadCmdFlag = CMD_PROCESS;
#ifdef __ANDROID__
    rc = pthread_cond_signal(&(threadControlParameter[i].threadCondCmd));
#else
    rc = os_cond_signal(&(threadControlParameter[i].threadCondCmd));
#endif
    if (rc != 0) {
      IDBG_ERROR("Signal set failed %d\n", rc);
      oldpid = 0;
      return 1;
    }
  }
  //Wait for done signal from the worker threads
  for (;;) {
#ifdef __ANDROID__
    rc = pthread_cond_wait(&condMsg, &mutexMsg);
#else
    rc = os_cond_wait(&condMsg, &mutexMsg);
#endif
    if (rc != 0) {
      IDBG_ERROR("Cond wait failed %d\n", rc);
      oldpid = 0;
      return 1;
    }
    doneFlag = TRUE;
    for (i = 0; i < numberofthreads; ++i) {
      if (threadControlParameter[i].threadMsgFlag != MSG_DONE) {
        doneFlag = FALSE;
      }
    }
    //If all threads are ready, exit for
    if (doneFlag == TRUE) break;
  }
  for (i = 0; i < numberofthreads; ++i) {
    threadControlParameter[i].threadCmdFlag = CMD_IDLE;
    threadControlParameter[i].threadMsgFlag = MSG_IDLE;
  }
#ifdef __ANDROID__
  rc = pthread_mutex_unlock(&mutexMsg);
#else
  rc = os_mutex_unlock(&mutexMsg);
#endif
  if (rc != 0) {
    IDBG_ERROR("Mutex unlock failed %d\n", rc);
    oldpid = 0;
    return 1;
  }
  return 0;
}

/*===========================================================================

Function            : dynamic_work_partition_enqueue

Description         : Enqueue the payload

Input parameter(s)  : threadid,threadfunction,threadpayload, size of payload

Output parameter(s) : 0/1

=========================================================================== */

int32_t dynamic_work_partition_enqueue(int32_t threadid, int32_t pfunc(void*),
  void *ppayload,uint32_t size)
{
  int32_t rc;
  //Lock mutex
#ifdef __ANDROID__
  rc = pthread_mutex_lock(&mutexCmd);
#else
  rc = os_mutex_lock(&mutexCmd);
#endif
  if (rc != 0) {
    IDBG_ERROR("Mutex lock failed %d\n", rc);
    return 1;
  }
  threadControlParameter[threadid].pthreadProcessFunc = pfunc;
  memcpy(threadControlParameter[threadid].pthreadPayload,ppayload,size);
#ifdef __ANDROID__
  //Lock mutex
  rc = pthread_mutex_unlock(&mutexCmd);
#else
  rc = os_mutex_unlock(&mutexCmd);
#endif
  if (rc != 0) {
    IDBG_ERROR("Mutex unlock failed %d\n", rc);
    oldpid = 0;
    return 1;
  }
  return 0;
}


/*===========================================================================

Function            : dynamic_work_thread_func

Description         : Dynamic work partitioning thread function

Input parameter(s)  : void *

Output parameter(s) : void *

=========================================================================== */

static void* dynamic_work_thread_func(void *arg)
{
  int           rc;
  thread_param_struct_t *plocalThreadParameter = (thread_param_struct_t *)arg;
#ifdef __ANDROID__
  rc = pthread_mutex_lock(plocalThreadParameter->pmutexCmd);
#else
  rc = os_mutex_lock(plocalThreadParameter->pmutexCmd);
#endif
  if (rc != 0) {
    IDBG_ERROR("Mutex lock failed %d\n", rc);
    oldpid = 0;
    return NULL;
  }

  //Signal ready to main thread
  plocalThreadParameter->threadMsgFlag = MSG_READY;
#ifdef __ANDROID__
  rc = pthread_cond_signal(plocalThreadParameter->pcondMsg);
#else
  rc = os_cond_signal(plocalThreadParameter->pcondMsg);
#endif
  for ( ; ; ) {
    //Wait for command from main thread
#ifdef __ANDROID__
    rc = pthread_cond_wait(&(plocalThreadParameter->threadCondCmd),
      plocalThreadParameter->pmutexCmd);
#else
    rc = os_cond_wait(&(plocalThreadParameter->threadCondCmd),
      plocalThreadParameter->pmutexCmd);
#endif
    if (rc != 0) {
      IDBG_ERROR("Mutex cond wait failed %d\n", rc);
#ifdef __ANDROID__
      pthread_mutex_unlock(plocalThreadParameter->pmutexCmd);
#else
      os_mutex_unlock(plocalThreadParameter->pmutexCmd);
#endif
      oldpid = 0;
      return NULL;
    }
#ifdef __ANDROID__
    rc = pthread_mutex_unlock(plocalThreadParameter->pmutexCmd);
#else
    rc = os_mutex_unlock(plocalThreadParameter->pmutexCmd);
#endif
    if (rc != 0) {
      IDBG_ERROR("Mutex unlock failed %d\n", rc);
      oldpid = 0;
      return NULL;
    }
    if (plocalThreadParameter->threadCmdFlag == CMD_PROCESS) {
      //Call processing segment in Q6 or ARM
      if (0 == plocalThreadParameter->pthreadProcessFunc
          (plocalThreadParameter->pthreadPayload)) {
        //Signal Done message to main thread
#ifdef __ANDROID__
        rc = pthread_mutex_lock(plocalThreadParameter->pmutexMsg);
#else
        rc = os_mutex_lock(plocalThreadParameter->pmutexMsg);
#endif

        if (rc != 0) {
          IDBG_ERROR("Mutex lock failed %d\n", rc);
          oldpid = 0;
          return NULL;
        }
        plocalThreadParameter->threadCmdFlag = CMD_IDLE;
        plocalThreadParameter->threadMsgFlag = MSG_DONE;
#ifdef __ANDROID__
        rc = pthread_cond_signal(plocalThreadParameter->pcondMsg);
#else
        rc = os_cond_signal(plocalThreadParameter->pcondMsg);
#endif
        if (rc != 0) {
          IDBG_ERROR("Signal set failed %d\n", rc);
#ifdef __ANDROID__
          pthread_mutex_unlock(plocalThreadParameter->pmutexMsg);
#else
          os_mutex_unlock(plocalThreadParameter->pmutexMsg);
#endif
          oldpid = 0;
          return NULL;
        }
        //Lock the command mutex, to avoid new commands from
        //main thread till this thread goes to wait state
#ifdef __ANDROID__
        rc = pthread_mutex_lock(plocalThreadParameter->pmutexCmd);
#else
        rc = os_mutex_lock(plocalThreadParameter->pmutexCmd);
#endif
        if (rc != 0) {
          IDBG_ERROR("Mutex lock failed %d\n", rc);
#ifdef __ANDROID__
          pthread_mutex_unlock(plocalThreadParameter->pmutexMsg);
#else
          os_mutex_unlock(plocalThreadParameter->pmutexMsg);
#endif
          oldpid = 0;
          return NULL;
        }
        //Unlock the message mutex, so main thread can process done
        //message
#ifdef __ANDROID__
        rc = pthread_mutex_unlock(plocalThreadParameter->pmutexMsg);
#else
        os_mutex_unlock(plocalThreadParameter->pmutexMsg);
#endif
        if (rc != 0) {
          IDBG_ERROR("Mutex unlock failed %d\n", rc);
#ifdef __ANDROID__
          pthread_mutex_unlock(plocalThreadParameter->pmutexCmd);
#else
          os_mutex_unlock(plocalThreadParameter->pmutexCmd);
#endif
          oldpid = 0;
          return NULL;
        }
      } else {
        //Signal Abort message to main thread
#ifdef __ANDROID__
        rc = pthread_mutex_lock(plocalThreadParameter->pmutexMsg);
#else
        os_mutex_lock(plocalThreadParameter->pmutexMsg);
#endif
        //Signal exit to main thread
        plocalThreadParameter->threadCmdFlag = CMD_IDLE;
        plocalThreadParameter->threadMsgFlag = MSG_ABORT;
#ifdef __ANDROID__
        rc = pthread_cond_signal(plocalThreadParameter->pcondMsg);
#else
        rc = os_cond_signal(plocalThreadParameter->pcondMsg);
#endif
        if (rc != 0) {
          IDBG_ERROR("Signal set failed %d\n", rc);
#ifdef __ANDROID__
          pthread_mutex_unlock(plocalThreadParameter->pmutexMsg);
#else
          os_mutex_unlock(plocalThreadParameter->pmutexMsg);
#endif
          oldpid = 0;
          return NULL;
        }
        //Lock the command mutex, to avoid new commands from
        //main thread till this thread goes to wait state
#ifdef __ANDROID__
        rc = pthread_mutex_lock(plocalThreadParameter->pmutexCmd);
#else
        rc = os_mutex_lock(plocalThreadParameter->pmutexCmd);
#endif
        if (rc != 0) {
          IDBG_ERROR("Mutex lock failed %d\n", rc);
#ifdef __ANDROID__
          pthread_mutex_unlock(plocalThreadParameter->pmutexMsg);
#else
          os_mutex_unlock(plocalThreadParameter->pmutexMsg);
#endif
          oldpid = 0;
          return NULL;
        }
        //Unlock the message mutex, so main thread can process done
        //message
#ifdef __ANDROID__
        rc = pthread_mutex_unlock(plocalThreadParameter->pmutexMsg);
#else
        rc = os_mutex_unlock(plocalThreadParameter->pmutexMsg);
#endif
        if (rc != 0) {
          IDBG_ERROR("Mutex unlock failed %d\n", rc);
#ifdef __ANDROID__
          pthread_mutex_unlock(plocalThreadParameter->pmutexCmd);
#else
          os_mutex_unlock(plocalThreadParameter->pmutexCmd);
#endif
          oldpid = 0;
          return NULL;
        }
      }
    } else {
      //Signal exit to main thread
#ifdef __ANDROID__
      rc = pthread_mutex_lock(plocalThreadParameter->pmutexCmd);
#else
      rc = os_mutex_lock(plocalThreadParameter->pmutexCmd);
#endif

      if (rc != 0) {
        IDBG_ERROR("Mutex lock failed %d\n", rc);
        oldpid = 0;
        return NULL;
      }

      plocalThreadParameter->threadMsgFlag = MSG_EXIT;
#ifdef __ANDROID__
      rc = pthread_cond_signal(plocalThreadParameter->pcondMsg);
#else
      rc = os_cond_signal(plocalThreadParameter->pcondMsg);
#endif
    }
  }
}
