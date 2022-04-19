/******************************************************************************
#  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include "framework/Message.h"
#include "framework/Dispatcher.h"
#include "framework/SyncApiSession.h"

#define TAG "RILQMessage"
#include <framework/Log.h>
using std::lock_guard;

Message::~Message() {
  // Log::getInstance().d("Message base destructor");
}

Message::Callback::~Callback() {
  // Log::getInstance().d("Base Callback desctructor");
}

void Message::handlerExecuting() {
  lock_guard<qtimutex::QtiRecursiveMutex> lock(mMutex);
  mExecutionStatus = ExecutionStatus::HANDLER_INPROGRESS;
}

void Message::handlerExecuted() {
  lock_guard<qtimutex::QtiRecursiveMutex> lock(mMutex);
  mExecutionStatus = ExecutionStatus::HANDLER_EXECUTED;
}

void Message::callbackExecuting() {
  lock_guard<qtimutex::QtiRecursiveMutex> lock(mMutex);
  mExecutionStatus = ExecutionStatus::CALLBACK_INPROGRESS;
}

void Message::callbackExecuted() {
  lock_guard<qtimutex::QtiRecursiveMutex> lock(mMutex);
  mExecutionStatus = ExecutionStatus::CALLBACK_EXECUTED;
}

Message::ExecutionStatus Message::getExecutionStatus() {
  lock_guard<qtimutex::QtiRecursiveMutex> lock(mMutex);
  return mExecutionStatus;
}

void Message::cancelling() {
  lock_guard<qtimutex::QtiRecursiveMutex> lock(mMutex);
  mExecutionStatus = ExecutionStatus::CANCELLATION_INPROGRESS;
}

void Message::cancelled() {
  lock_guard<qtimutex::QtiRecursiveMutex> lock(mMutex);
  mExecutionStatus = ExecutionStatus::CANCELLED;
}

bool Message::isHandlerExecuted() {
  lock_guard<qtimutex::QtiRecursiveMutex> lock(mMutex);
  if (mExecutionStatus == ExecutionStatus::HANDLER_EXECUTED) {
    return true;
  } else {
    return false;
  }
}

bool Message::isHandlerExecuting() {
  lock_guard<qtimutex::QtiRecursiveMutex> lock(mMutex);
  if (mExecutionStatus == ExecutionStatus::HANDLER_INPROGRESS) {
    return true;
  }
  else {
    return false;
  }
}

bool Message::isCallbackExecuted() {
  lock_guard<qtimutex::QtiRecursiveMutex> lock(mMutex);
  if (mExecutionStatus == ExecutionStatus::CALLBACK_EXECUTED) {
    return true;
  }
  else {
    return false;
  }
}

bool Message::isCallbackExecuting() {
  lock_guard<qtimutex::QtiRecursiveMutex> lock(mMutex);
  if (mExecutionStatus == ExecutionStatus::CALLBACK_INPROGRESS) {
    return true;
  }
  else {
    return false;
  }
}

bool Message::isExpired() {
  lock_guard<qtimutex::QtiRecursiveMutex> lock(mMutex);
  if (mExecutionStatus == ExecutionStatus::CANCELLATION_INPROGRESS ||
      mExecutionStatus == ExecutionStatus::CANCELLED) {
    return true;
  } else {
    return false;
  }
}

void Message::setTimerId(TimeKeeper::timer_id tid) {
  lock_guard<qtimutex::QtiRecursiveMutex> lock(mMutex);
  mTid = tid;
}

/* by default, only when they point to the same area will it return true */
bool Message::isSameMessage(std::shared_ptr<Message> msg) {
  return (this == msg.get());
}

TimeKeeper::timer_id Message::getTimerId() {
  lock_guard<qtimutex::QtiRecursiveMutex> lock(mMutex);
  return mTid;
}

void Message::informDispatchFailure(std::shared_ptr<Message> solicitedMsg,
                           Message::Callback::Status status) {
  lock_guard<qtimutex::QtiRecursiveMutex> lock(mMutex);
  if (mCallback != nullptr) {
    mCallback->handleAsyncResponse(solicitedMsg, status, nullptr);
  }
  // Dispatcher::getInstance().informMessageCallbackFinished(solicitedMsg);
}

void Message::setMessageExpiryTimer(TimeKeeper::millisec timeOut) {
  lock_guard<qtimutex::QtiRecursiveMutex> lock(mMutex);
  mExpiryTimer = timeOut;
}

TimeKeeper::millisec Message::getMessageExpiryTimer() {
  lock_guard<qtimutex::QtiRecursiveMutex> lock(mMutex);
  return mExpiryTimer;
}

void Message::Callback::handleAsyncResponse(std::shared_ptr<Message> solicitedMsg,
                                 Status status,
                                 std::shared_ptr<void> responseDataPtr)
{
    // If callback function is not provided in constructor, this function must be overriden
    QCRIL_LOG_ASSERT(theCallback != nullptr);
    if (theCallback != nullptr) {
        theCallback(solicitedMsg, status, responseDataPtr);
    }
}
