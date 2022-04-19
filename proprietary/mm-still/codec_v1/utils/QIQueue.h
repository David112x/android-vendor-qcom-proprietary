/*****************************************************************************
* Copyright (c) 2012, 2015 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*****************************************************************************/

#ifndef __QIQUEUE_H__
#define __QIQUEUE_H__

#include "QINode.h"

/*===========================================================================
 * Class: QIQueueObject
 *
 * Description: This class represents the Queue object. This object can be
 * queued in QIQueue object.
 *
 *
 * Notes: none
 *==========================================================================*/
class QIQueueObject : public QIBase {

public:

  /** QIQueueObject
   *
   *  constructor
   **/
  QIQueueObject();

  /** ~QIQueueObject
   *
   *  virtual destructor
   **/
  virtual ~QIQueueObject();

  /** SetId
   *
   *  Set queue obj id
   **/
  void SetId(uint32_t aId);

  /** GetId
   *
   *  Get queue obj id
   **/
  uint32_t GetId();

  /** SetInQueFlag
   *
   *  Set in Queue flag
   **/
  void SetInQueFlag(bool aFlag);

  /** IsInQue
   *
   *  Check if obj in queue
   **/
  bool IsInQue();

private:

  /** mId:
   *
   * ID for object enqueued
   **/
  uint32_t mId;

  /** mInQue:
   *
   * Flag indicating object in queue
   **/
  bool mInQue;
};

/*===========================================================================
 * Class: QIQueue
 *
 * Description: This class represents the queue utility
 *
 *
 * Notes: none
 *==========================================================================*/
class QIQueue : public QIBase {

public:

  /** QIQueue
   *
   *  constructor
   **/
  QIQueue();

  /** ~QIQueue
   *
   *  virtual destructor
   **/
  virtual ~QIQueue();

  /** Enqueue
   *  @aObj: base object to be enqueued
   *
   *  enqueues an object to the queue
   **/
  int Enqueue(QIQueueObject *aObj);

  /** Dequeue
   *
   *  dequeues an object from the queue
   **/
  QIQueueObject* Dequeue();

  /** Count
   *
   *  returns the queue count
   **/
  inline int Count()
  {
    return mCount;
  }

  /** DeleteAll
   *
   *  deletes all elements in the queue
   **/
  void DeleteAll();

  /** Flush
   *
   *  flushes all elements in the queue without calling the
   *  destructor of the base objects
   **/
  void Flush();

  /** RemoveByID
   *  @aQIBaseQueId: queue element id.
   *
   *  Removes element by ID from queue
   *
   **/
  void RemoveByID(uint32_t aId);

private:

  /** mFront
   *
   *  front of the queue
   **/
  QINode *mFront;

  /** mRear
   *
   *  rear of the queue
   **/
  QINode *mRear;

  /** mCount
   *
   *  number of elements in the queue
   **/
  uint32_t mCount;

   /** mQueIDBase
   *
   *  Queue element ID base
   **/
  uint32_t mQueIDBase;
};

#endif //__QIQUEUE_H__
