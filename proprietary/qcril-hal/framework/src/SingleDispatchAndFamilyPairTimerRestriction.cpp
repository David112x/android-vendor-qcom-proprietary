/******************************************************************************
#  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include "framework/SingleDispatchAndFamilyPairTimerRestriction.h"
#include "framework/Dispatcher.h"
#include "framework/Log.h"

using std::lock_guard;

SingleDispatchAndFamilyPairTimerRestriction::SingleDispatchAndFamilyPairTimerRestriction() {
  mName = "SingleDispatchAndFamilyPairTimerRestriction";
  mHandlingMsg = nullptr;
  mRedispatchedMsg = nullptr;
}

SingleDispatchAndFamilyPairTimerRestriction::~SingleDispatchAndFamilyPairTimerRestriction() {}

/*
 * Below logic ensures that msg which are pending in a separate queue due to restriction
 * are processed exactly in the order they arrived. This algo addresses a particular
 * race-condition where pending msg is re-enqueued to main dispatcher queue just after
 * a new msg from telephony is enqueued and hence ends up with incorrect order.
 * Here is the flow:
 * - Forward the incoming msg to module if msg dispatch is not restricted. Set mHandlingMsg
 *   to this msg.
 * - Append the incoming msg to mRestrictedMessagesQueue if restriction is applicable (There
 *   is current handling msg or re-dispatched pending msg or queue is not empty)
 * - Upon msg's callback execution by respective module, pop the head of mRestrictedMessagesQueue
 *   and redispatch. While doing that save the refrence to this msg in mRedispatchedMsg.
 * - If the incoming msg is mRedispatchedMsg then set mHandlingMsg to mRedispatchedMsg and
 *   mRedispatchedMsg to nullptr. Now forward this msg to respective module.
 */
bool SingleDispatchAndFamilyPairTimerRestriction::isDispatchAllowed(
    std::shared_ptr<Message> msg) {
  lock_guard<qtimutex::QtiSharedMutex> lock(mMutex);

  string msgStr = msg->to_string();
  bool okToDispatch = false;

  if (msg == mRedispatchedMsg) {
    /* case 1: if the msg is the expected re-dispatched msg, handle it directly */
    assert(mHandlingMsg == nullptr);
    mHandlingMsg = msg;
    mRedispatchedMsg = nullptr;
    okToDispatch = true;
  } else if (!mHandlingMsg && !mRedispatchedMsg && !mRestrictedMessagesQueue.size()) {
    /* case 2: if nothing is under handling and nothing is in the waiting queue, and
     * no re-dispatched message is waiting execution, handle it immediately */
    mHandlingMsg = msg;
    okToDispatch = true;
  } else {
    /* Add this msg to the waiting list */
    mRestrictedMessagesQueue.push_back(msg);
    okToDispatch = false;
  }

  if (okToDispatch) {
    /* Message is restricted but no message is outstanding. Clear to go * ahead.*/
    Log::getInstance().d(
        "[" + mName + "]: Restrictions enabled for " +
        msg->to_string() + " but allowed to dispatch.");
    return true;
  } else {
    /* One message from the groupd is still inprogress and callback is not yet
     * executed. Wait.*/
    Log::getInstance().d("[" + mName + "]: Can not forward msg = " +
                         msg->to_string() + " until restriction is cleared");
    return false;
  }
}

bool SingleDispatchAndFamilyPairTimerRestriction::setMessagePair(std::string msgStr1,
    std::string msgStr2) {

    /* both message name records do not exist */
    if ((mPairMessages.find(msgStr1) == mPairMessages.end())
          && (mPairMessages.find(msgStr2) == mPairMessages.end())) {
        mPairMessages[msgStr1] = msgStr2;
        mPairMessages[msgStr2] = msgStr1;
        return true;
    }

    Log::getInstance().d("[" + mName + "]: One of the messages already in pair list: "
                + msgStr1 + " " + msgStr2);
    return false;
}

bool SingleDispatchAndFamilyPairTimerRestriction::setMessageTimer(std::string msg,
                                                                  TimeKeeper::millisec timer) {
  mMessageTimers[msg] = timer;
  Log::getInstance().d("[" + mName + "]: Set timer : " + std::to_string(timer) +
                       " for msg: " + msg);
  return true;
}

void SingleDispatchAndFamilyPairTimerRestriction::onMessageCallbackCompletion(
    std::shared_ptr<Message> msg) {
  TimeKeeper::millisec timer = 0;
  {
    lock_guard<qtimutex::QtiSharedMutex> lock(mMutex);

    string msgStr = msg->to_string();
    Log::getInstance().d("[" + mName + "]: onMessageCallbackCompletion msg = " + msg->to_string());

    if (msg != mHandlingMsg) {
      Log::getInstance().d("[" + mName + "]: msg mismatch.. should not be here");
      return;
    }
    if (mMessageTimers.find(msgStr) != mMessageTimers.end()) {
      timer = mMessageTimers[msgStr];
    }
  }

  mTid = TimeKeeper::getInstance().set_timer(
      [this, msg](void *user_data) {
        QCRIL_NOTUSED(user_data);
        mTid = TimeKeeper::no_timer;
        string msgStr = msg->to_string();
        {
          lock_guard<qtimutex::QtiSharedMutex> lock(mMutex);

          mHandlingMsg = nullptr;
          mRedispatchedMsg = nullptr;
          /* take the paired message as higher priority for next dispatch */
          if (mPairMessages.find(msgStr) != mPairMessages.end()) {
            string pairStr = mPairMessages[msgStr];
            std::shared_ptr<Message> pairMsg;
            for (auto it = mRestrictedMessagesQueue.begin(); it != mRestrictedMessagesQueue.end();
                 ++it) {
              if ((*it != nullptr) && (!(*it)->to_string().compare(pairStr))) {
                pairMsg = *it;
                mRestrictedMessagesQueue.erase(it);
                break;
              }
            }
            if (pairMsg) {
              mRedispatchedMsg = pairMsg;
              Log::getInstance().d("[" + mName + "]: Requeue paired msg = " + pairMsg->to_string());
            }
          }

          /* if no paired msg is dispatched, fetch the front msg in the queue */
          if (!mRedispatchedMsg && !mRestrictedMessagesQueue.empty()) {
            mRedispatchedMsg = mRestrictedMessagesQueue.front();
            mRestrictedMessagesQueue.pop_front();
            Log::getInstance().d("[" + mName + "]: Requeue msg = " + mRedispatchedMsg->to_string());
          }
        }

        if (nullptr != mRedispatchedMsg) {
          Dispatcher::getInstance().dispatch(mRedispatchedMsg);
        }
      },
      nullptr, timer);
}

void SingleDispatchAndFamilyPairTimerRestriction::abandonPendingMessages() {
  lock_guard<qtimutex::QtiSharedMutex> lock(mMutex);
  Log::getInstance().d("[" + mName + "]: abandonPendingMessages");

  if (!mRestrictedMessagesQueue.empty()) {
    for (const auto &pendingMsg : mRestrictedMessagesQueue) {
      if (pendingMsg) {
        Log::getInstance().d("[" + mName + "]: informing CANCELLED");
        pendingMsg->informDispatchFailure(pendingMsg, Message::Callback::Status::CANCELLED);
      }
    }
    mRestrictedMessagesQueue.clear();
  }
  if (mTid != TimeKeeper::no_timer) {
    TimeKeeper::getInstance ().clear_timer(mTid);
  }
}

string SingleDispatchAndFamilyPairTimerRestriction::to_string() { return mName; }
