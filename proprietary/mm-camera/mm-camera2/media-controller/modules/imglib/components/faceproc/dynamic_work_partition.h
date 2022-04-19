/***************************************************************************
* Copyright (c) 2012-2014,2016 Qualcomm Technologies, Inc. All Rights Reserved. *
* Qualcomm Technologies Proprietary and Confidential.                      *
****************************************************************************/


#ifndef __ANDROID__
#include <stdint.h>
#include "os_thread.h"
#else
#include <stdlib.h>
#include <string.h>
#include "img_dbg.h"
#include "pthread.h"
#endif
/*===========================================================================
Macro definitions
=========================================================================== */
#define DYNAMIC_WORK_QUEUE_MAX_SIZE 30

//dsp
#define NTHREADS   1
#define NMAXTHREADS 10


/*===========================================================================
typedef
=========================================================================== */
/** cmd_payload_t
 *   @CMD_IDLE: Idel command
 *   @CMD_PROCESS: Process command
 *   @CMD_EXIT: Exit command
 *
 *   Enum for command
 **/
typedef enum {
  CMD_IDLE,
  CMD_PROCESS,
  CMD_EXIT
}
cmd_payload_t;

/** msg_payload_t
 *   @MSG_IDLE: Idel message
 *   @MSG_READY: Ready message
 *   @MSG_DONE: Done message
 *   @MSG_ABORT: Abort message
 *   @CMD_EXIT: Exit message
 *
 *   Enum for message
 **/
typedef enum {
  MSG_IDLE,
  MSG_READY,
  MSG_DONE,
  MSG_ABORT,
  MSG_EXIT
}
msg_payload_t;

/** core_type_t
 *   @CORE_DSP: DSP core
 *   @CORE_GPU: GPU core
 *   @CORE_ARM: ARM core
 *
 *   Enum for core type
 **/
typedef enum {
  CORE_DSP,
  CORE_GPU,
  CORE_ARM
}
core_type_t;

#ifdef __ANDROID__
/** thread_param_struct_t
 *   @threadCoreType: Thread core type
 *   @threadCmdFlag: Thread command flag
 *   @threadMsgFlag: Thread message flag
 *   @threadCondCmd: Thread condition command
 *   @pthreadProcessFunc: Pointer to thread process function
 *   @pthreadDefaultProcessFunc: Pointer to default thread process function
 *   @pmutexCmd: Thread mutex cmd
 *   @pmutexMsg: Thread mutex msg
 *   @pcondMsg: Thread condition msg
 *   @pthreadPayload: Thread payload
 *
 *   Struct for thread parameter
 **/
typedef struct {
  core_type_t       threadCoreType;
  cmd_payload_t     threadCmdFlag;
  msg_payload_t     threadMsgFlag;
  pthread_cond_t    threadCondCmd;
  int32_t           (*pthreadProcessFunc)(void *);
  int32_t          (*pthreadDefaultProcessFunc) (void *);
  pthread_mutex_t * pmutexCmd;
  pthread_mutex_t * pmutexMsg;
  pthread_cond_t  * pcondMsg;
  void            * pthreadPayload;
}
#else
typedef struct {
  core_type_t       threadCoreType;
  cmd_payload_t     threadCmdFlag;
  msg_payload_t     threadMsgFlag;
  os_cond_t    threadCondCmd;
  int32_t           (*pthreadProcessFunc)(void *);
  int32_t          (*pthreadDefaultProcessFunc) (void *);
  os_mutex_t * pmutexCmd;
  os_mutex_t * pmutexMsg;
  os_cond_t  * pcondMsg;
  void            * pthreadPayload;
}
#endif

thread_param_struct_t;


/*===========================================================================

Function            : dynamic_work_partition_init

Description         : Create max number of threads

Input parameter(s)  : size

Output parameter(s) : 0/1

=========================================================================== */

int32_t dynamic_work_partition_init (uint32_t size);


/*===========================================================================

Function            : dynamic_work_thread_func

Description         : Dynamic work partitioning thread function

Input parameter(s)  : void *

Output parameter(s) : void *

=========================================================================== */

static void * dynamic_work_thread_func(void *arg);


/*===========================================================================

Function            : dynamic_work_partition_enqueue

Description         : Enqueue the payload

Input parameter(s)  : void

Output parameter(s) : 0/1

=========================================================================== */

int32_t dynamic_work_partition_enqueue(int32_t threadid, int32_t pfunc(void*),
  void *ppayload,uint32_t size);

/*===========================================================================

Function            : dynamic_work_partition_execute

Description         : Execute

Input parameter(s)  : void

Output parameter(s) : void

=========================================================================== */

int32_t dynamic_work_partition_execute(int32_t numberofthreads);

/*===========================================================================

Function            : dynamic_work_partition_deinit

Description         : Free the memory

Input parameter(s)  : void

Output parameter(s) : 0/1

=========================================================================== */

int32_t dynamic_work_partition_deinit(void);
