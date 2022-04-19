/*****************************************************************************
* Copyright (c) 2012, 2015, 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*****************************************************************************/

#ifndef __QITHREAD_H__
#define __QITHREAD_H__

#include "QIBase.h"
#include "QIQueue.h"
#include <pthread.h>

/*===========================================================================
 * Class: QThreadObject
 *
 * Description: This class represents the thread object. If this object is
 *              passed to QIthread object, then Execute() function will be
 *              called in the new thread context
 *
 *
 * Notes: none
 *==========================================================================*/
class QThreadObject : public QIQueueObject
{

public:

  /** Execute
   *
   *  virtual function which will be called during thread
   *  execution
   **/
  virtual void Execute() = 0;

  /** ~QThreadObject
   *
   *  virtual destructor
   **/
  virtual ~QThreadObject()
  {
  }
};

/*===========================================================================
 * Class: QIThread
 *
 * Description: This class represents the thread utility
 *
 *
 * Notes: none
 *==========================================================================*/
class QIThread : public QIBase {

public:

  /** QIThread:
   *
   *  contructor
   **/
  QIThread();

  /** ~QIThread:
   *
   *  virtual destructor
   **/
  virtual ~QIThread();

  /** run:
   *  @aData: thread object
   *
   *  function called in new thread context
   **/
  virtual void run(QThreadObject *aData);

  /** StartThread:
   *  @aData: thread object
   *
   *  function to start the thread, if thread already started
   *  queue job
   **/
  int StartThread(QThreadObject *aData);

  /** StopThread:
   *  @aData: thread object
   *
   *  function to stop the thread
   **/
  void StopThread(QThreadObject *aData);

  /** JoinThread:
   *
   *  function to join the thread
   **/
  void JoinThread();

  /** IsSelf:
   *
   *  checks if the function call is made in the QIthread's
   *  context
   **/
  inline bool IsSelf()
  {
    return pthread_equal(pthread_self(), mThreadID);
  }

  /** WaitForCompletion:
   *  @apCond: condition object
   *  @apMutex: mutex object
   *  @ms: time in milliseconds
   *
   *  Static function to wait until the timer has expired.
   **/
  static int WaitForCompletion(pthread_cond_t *apCond,
    pthread_mutex_t *apMutex, uint32_t ms);

  /** setThreadName:
   *  @aThreadName: desired name for the thread
   *
   *  function to name the thread
   **/
  inline void setThreadName(const char *aThreadName)
  {
    pthread_setname_np(mThreadID, aThreadName);
  };

private:

  /** sRun:
   *  @aData: thread class object
   *
   *  Static function to executed in new thread context
   **/
  static void* sRun(void *aData);

protected:

  /** mThreadID:
   *
   *  thread id
   **/
  pthread_t mThreadID;

  /** mData:
   *
   *  thread object
   **/
  QThreadObject *mData;

  /** mStarted:
   *
   *  thread start indication flag
   **/
  bool mStarted;

  /** mMutex:
   *
   *  mutex object
   **/
  pthread_mutex_t mMutex;

  /** mCond:
   *
   *  condition object
   **/
  pthread_cond_t mCond;

  /** mReady:
   *
   *  thread ready indication flag
   **/

  bool mReady;

  /** mQueue
   *
   *  job queue
   **/
  QIQueue mQueue;

  /** mStopThread
   *
   *  thread stop indication flag
   **/
  bool mStopThread;
};

#endif //__QITHREAD_H__
