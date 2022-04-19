/*****************************************************************************
* Copyright (c) 2012, 2015 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*****************************************************************************/

#include "QIThread.h"
#include "QICommon.h"
#include "errno.h"
#include "QImageSWEncoder.h"

/*===========================================================================
 * Function: QIThread
 *
 * Description: QIThread constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIThread::QIThread()
{
  mData = NULL;
  mThreadID = 0;
  mStarted = false;
  mReady = false;
  mStopThread = false;
  pthread_mutex_init(&mMutex, NULL);
  pthread_cond_init(&mCond, NULL);
}

/*===========================================================================
 * Function: QIThread
 *
 * Description: QIThread destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIThread::~QIThread()
{
  // Set started flag to false to force thread stop
  mStarted = false;
  JoinThread();
  pthread_mutex_destroy(&mMutex);
  pthread_cond_destroy(&mCond);
}

/*===========================================================================
 * Function: JoinThread
 *
 * Description: Stop and join the thread.
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: This has to be called only after StopThread is called.
 *==========================================================================*/
void QIThread::JoinThread()
{
  QIDBG_MED("%s:%d] E", __func__, __LINE__);

  QI_LOCK(&mMutex);
  if (IsSelf() || !mThreadID) {
    QIDBG_MED("%s:%d] Skip joinThread", __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return;
  }

  if (mStarted && mQueue.Count()) {
    QIDBG_MED("%s:%d] Thread in use, dont stop",
      __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return;
  }

  mStopThread = true;

  if (mQueue.Count()) {
    mQueue.Flush();
  }
  QI_SIGNAL(&mCond);
  QI_UNLOCK(&mMutex);

  QIDBG_MED("%s:%d] Wait on thread join",
    __func__, __LINE__);
  pthread_join(mThreadID, NULL);
  QIDBG_MED("%s:%d] Done waiting on thread join",
    __func__, __LINE__);

  QI_LOCK(&mMutex);
  mReady = false;
  mStopThread = false;
  mStarted = false;
  mThreadID = 0;
  QI_BROADCAST(&mCond);
  QI_UNLOCK(&mMutex);

  QIDBG_MED("%s:%d] X", __func__, __LINE__);
}

/*===========================================================================
 * Function: run
 *
 * Description: The thread loop function. If the thread object is passed
 *              Execute() method of the thread object will be called.
 *              Otherwise user has to implement this function
 *
 * Input parameters:
 *   aData - pointer to the thread object
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QIThread::run(QThreadObject *aData)
{
  if (aData)
    aData->Execute();
}

/*===========================================================================
 * Function: sRun
 *
 * Description: static function called by the pthread API. member function
 *              of the class will be called from this function.
 *
 * Input parameters:
 *   aData - pointer to the QIThread object
 *
 * Return values:
 *   NULL
 *
 * Notes: none
 *==========================================================================*/
void* QIThread::sRun(void *aData)
{
  QIThread *lThread = (QIThread *)aData;
  QThreadObject *l_tobj;

  QIDBG_MED("%s:%d] New thread created", __func__, __LINE__);

  while (true) {
    QI_LOCK(&lThread->mMutex);

    if (lThread->mReady == false) {
      lThread->mReady = true;
      QIDBG_MED("%s:%d] Thread ready", __func__, __LINE__);
      QI_BROADCAST(&lThread->mCond);
    }

    while ((lThread->mQueue.Count() == 0) &&
      (!lThread->mStopThread)) {
      QIDBG_MED("%s:%d] Wait till threadobj queued or Thread stopped",
        __func__, __LINE__);
      QI_WAIT(&lThread->mCond, &lThread->mMutex);
    }

    QIDBG_MED("%s:%d] after wait", __func__, __LINE__);
    if (lThread->mStopThread) {
      QIDBG_MED("%s:%d] thread stop", __func__, __LINE__);
      QI_UNLOCK(&lThread->mMutex);
      break;
    }

    l_tobj = (QThreadObject *) (lThread->mQueue.Dequeue());
    QI_UNLOCK(&lThread->mMutex);

    QIDBG_MED("%s:%d] Run Threadobj %p", __func__, __LINE__, l_tobj);
    lThread->run(l_tobj);
  }

  return NULL;
}

/*===========================================================================
 * Function: StartThread
 *
 * Description: Starts the execution of the thread
 *
 * Input parameters:
 *   aData - pointer to the QThreadObject object
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
int QIThread::StartThread(QThreadObject *aData)
{
  int lStatus = QI_SUCCESS;

  QIDBG_MED("%s:%d] E", __func__, __LINE__);

  QI_LOCK(&mMutex);

  while (mStopThread) {
    QIDBG_MED("%s:%d] Wait for stop to complete",
      __func__, __LINE__);
    QI_WAIT(&mCond, &mMutex);
  }

  if (!mStarted) {
    QIDBG_MED("%s:%d] Create pthread", __func__, __LINE__);
    lStatus = pthread_create(&mThreadID, NULL,
      &QIThread::sRun, (void *)this);
    if (lStatus < 0) {
      QIDBG_ERROR("%s:%d] pthread creation failed %d",
        __func__, __LINE__, errno);
      QI_UNLOCK(&mMutex);
      return QI_ERR_GENERAL;
    }
    mStarted = true;
  }

  while (false == mReady) {
    QIDBG_MED("%s:%d] before wait", __func__, __LINE__);
    QI_WAIT(&mCond, &mMutex);
  }
  QIDBG_MED("%s:%d] after wait", __func__, __LINE__);

  if (QI_ERROR(mQueue.Enqueue(aData))) {
    QIDBG_ERROR("%s:%d] cannot enqueue threadobj",
      __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return QI_ERR_NO_MEMORY;
  }

  QI_SIGNAL(&mCond);
  QI_UNLOCK(&mMutex);

  QIDBG_MED("%s:%d] X", __func__, __LINE__);
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: StopThread
 *
 * Description: Prepares for stop by deleting the thread obj from queue.
 *
 * Input parameters:
 *   aData - pointer to the QThreadObject object
 *
 * Return values:
 *   none
 *
 * Notes: JoinThread needs to be called to stop thread.
 *==========================================================================*/
void QIThread::StopThread(QThreadObject *aData)
{
  QIDBG_MED("%s:%d] E", __func__, __LINE__);

  QI_LOCK(&mMutex);

  if (!mStarted) {
    QIDBG_MED("%s:%d] StopThread called before startThread",
      __func__, __LINE__);
    QI_UNLOCK(&mMutex);
    return;
  }

  if (!aData) {
    if (mQueue.Count()) {
      mQueue.Flush();
    }
    QI_UNLOCK(&mMutex);
    return;
  }

  if (aData->IsInQue()) {
    // remove thread obj from queue
    QIDBG_MED("%s:%d] Delete BaseQueID %d",
      __func__, __LINE__, aData->GetId());
    mQueue.RemoveByID(aData->GetId());
  }

  QI_UNLOCK(&mMutex);

  QIDBG_MED("%s:%d] X", __func__, __LINE__);
}

/*===========================================================================
 * Function: WaitForCompletion
 *
 * Description: Static function to wait until the timer has expired.
 *
 * Input parameters:
 *   apCond - pointer to the pthread condition
 *   apMutex - pointer to the pthread mutex
 *   aMs - time in milli seconds
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
int QIThread::WaitForCompletion(pthread_cond_t *apCond,
  pthread_mutex_t *apMutex, uint32_t aMs)
{
  struct timespec lTs;
  int lrc = clock_gettime(CLOCK_REALTIME, &lTs);
  if (lrc < 0)
    return QI_ERR_GENERAL;

  if (aMs >= 1000) {
    lTs.tv_sec += (aMs / 1000);
    lTs.tv_nsec += ((aMs % 1000) * 1000000);
  } else {
    lTs.tv_nsec += (aMs * 1000000);
  }

  lrc = pthread_cond_timedwait(apCond, apMutex, &lTs);
  if (lrc == ETIMEDOUT) {
    lrc = QI_ERR_TIMEOUT;
  }
  return lrc;
}
