/*****************************************************************************
* Copyright (c) 2012, 2015 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*****************************************************************************/

#include "QIQueue.h"
#include "QICommon.h"

/*===========================================================================
 * Function: QIQueueObject
 *
 * Description: QIQueueObject constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIQueueObject::QIQueueObject()
{
  mId = 0;
  mInQue = false;
}

/*===========================================================================
 * Function: ~QIQueueObject
 *
 * Description: QIQueueObject destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIQueueObject::~QIQueueObject()
{
}

/*===========================================================================
 * Function: SetId
 *
 * Description: Set queue obj ID
 *
 * Input parameters:
 *   aId - Id to set
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QIQueueObject::SetId(uint32_t aId)
{
  mId = aId;
}

/*===========================================================================
 * Function: GetId
 *
 * Description: Get queue obj ID
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   Returns queue obj Id
 *
 * Notes: none
 *==========================================================================*/
uint32_t QIQueueObject::GetId()
{
  return mId;
}

/*===========================================================================
 * Function: SetInQue
 *
 * Description: Set enqueued in queue flag to true
 *
 * Input parameters:
 *   aFlag - Flag to set
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QIQueueObject::SetInQueFlag(bool aFlag)
{
  mInQue = aFlag;
}

/*===========================================================================
 * Function: IsInQue
 *
 * Description: Check if obj in queue
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   True if in queue, false if not
 *
 * Notes: none
 *==========================================================================*/
bool QIQueueObject::IsInQue()
{
  return mInQue;
}

/*===========================================================================
 * Function: QIQueue
 *
 * Description: QIQueue constructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIQueue::QIQueue()
{
  mFront = NULL;
  mRear = NULL;
  mCount = 0;
  mQueIDBase = 0;
}

/*===========================================================================
 * Function: ~QIQueue
 *
 * Description: QIQueue destructor
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
QIQueue::~QIQueue()
{
}

/*===========================================================================
 * Function: Enqueue
 *
 * Description: enqueues the data into the queue
 *
 * Input parameters:
 *   aObj - object to enqueue
 *
 * Return values:
 *   QI_SUCCESS or QI_ERR_
 *
 * Notes: none
 *==========================================================================*/
int QIQueue::Enqueue(QIQueueObject *aObj)
{
  QINode *lTemp = new QINode(aObj);

  if (NULL == lTemp) {
    QIDBG_ERROR("%s:%d] Enqueue failed", __func__, __LINE__);
    return QI_ERR_NO_MEMORY;
  }
  if ((NULL == mFront) || (NULL == mRear)) {
    mFront = mRear = lTemp;
  } else if (mFront == mRear) {
    mRear = lTemp;
    mFront->setNext(mRear);
  } else {
    mRear->setNext(lTemp);
    mRear = lTemp;
  }

  mCount++;

  if (aObj) {
    aObj->SetId(++mQueIDBase);
    aObj->SetInQueFlag(true);
    QIDBG_MED("%s:%d] enque obj %p into queue %p", __func__,
      __LINE__, aObj, this);
    QIDBG_MED("%s:%d] obj Id set is %d", __func__, __LINE__,
      aObj->GetId());
  }
  return QI_SUCCESS;
}

/*===========================================================================
 * Function: Dequeue
 *
 * Description: dequeues the data from the queue
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   base object pointer
 *   NULL in error case
 *
 * Notes: none
 *==========================================================================*/
QIQueueObject* QIQueue::Dequeue()
{
  QIQueueObject* lData = NULL;
  QINode* lNode = NULL;
  if ((NULL == mFront) || (NULL == mRear)) {
    QIDBG_ERROR("%s:%d] Dequeue failed", __func__, __LINE__);
    return NULL;
  }
  lData = static_cast<QIQueueObject*> (mFront->Data());
  lNode = mFront->Next();
  if (mFront == mRear) {
    mRear = NULL;
  }
  delete mFront;
  mFront = lNode;
  mCount--;

  if (lData) {
    lData->SetInQueFlag(false);
    lData->SetId(0);
    QIDBG_MED("%s:%d] dque obj %p from que %p", __func__,
      __LINE__, lData, this);
    QIDBG_MED("%s:%d] obj Id reset to %d", __func__, __LINE__,
      lData->GetId());
  }
  return lData;
}

/*===========================================================================
 * Function: DeleteAll
 *
 * Description: deletes all the data from the queue
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QIQueue::DeleteAll()
{
  QIQueueObject* lTemp = NULL;
  while (mCount > 0) {
    lTemp = Dequeue();
    if (NULL == lTemp) {
      QIDBG_ERROR("%s:%d] queue corrupted", __func__, __LINE__);
    }
    delete lTemp;
  }
}

/*===========================================================================
 * Function: Flush
 *
 * Description: flush all the data from the queue
 *
 * Input parameters:
 *   none
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QIQueue::Flush()
{
  QIQueueObject* lTemp = NULL;
  while (mCount > 0) {
    lTemp = Dequeue();
    if (NULL == lTemp) {
      QIDBG_ERROR("%s:%d] queue corrupted", __func__, __LINE__);
    }
  }
}

/*===========================================================================
 * Function: RemoveByID
 *
 * Description: Removes element in queue by ID.
 *
 * Input parameters:
 *   aQIBaseQueId - Queue element ID
 *
 * Return values:
 *   none
 *
 * Notes: none
 *==========================================================================*/
void QIQueue::RemoveByID(uint32_t aId)
{
  QIQueueObject* lData = NULL;
  QINode* lNodeCurr = NULL;
  QINode* lNodePrev = NULL;
  if ((NULL == mFront) || (NULL == mRear)) {
    QIDBG_MED("%s:%d] queue empty", __func__, __LINE__);
    return;
  }

  lNodeCurr = mFront;
  lData = static_cast<QIQueueObject*> (mFront->Data());

  if (lData && (lData->GetId() == aId)) {
    QIDBG_MED("%s:%d] ID match found", __func__, __LINE__);
    if (mFront == mRear) {
      mRear = NULL;
    }
    mFront = lNodeCurr->Next();
    goto cleanup;
  }

  lNodePrev = lNodeCurr;
  lNodeCurr = lNodeCurr->Next();

  while (lNodeCurr != NULL) {
    lData = static_cast<QIQueueObject*> (lNodeCurr->Data());
    if (lData && (lData->GetId() == aId)) {
      QIDBG_MED("%s:%d] ID match found", __func__, __LINE__);

      if (lNodeCurr == mRear) {
        mRear = lNodePrev;
      }

      lNodePrev->setNext(lNodeCurr->Next());
      goto cleanup;

    } else {
      QIDBG_MED("%s:%d] Try next node", __func__, __LINE__);
      lNodePrev = lNodeCurr;
      lNodeCurr = lNodeCurr->Next();
    }
  }

  QIDBG_MED("%s:%d] Queue Id not found", __func__, __LINE__);
  return;

cleanup:
  lData->SetInQueFlag(false);
  lData->SetId(0);
  delete lNodeCurr;
  lNodeCurr = NULL;
  mCount--;
  return;
}

