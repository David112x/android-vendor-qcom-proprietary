/******************************************************************************
#  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <deque>
#include <memory>
#include <deque>
#include <unordered_map>

#include "framework/Restriction.h"
class SingleDispatchAndFamilyPairTimerRestriction : public Restriction {
 public:
  SingleDispatchAndFamilyPairTimerRestriction();
  ~SingleDispatchAndFamilyPairTimerRestriction();

  bool isDispatchAllowed(std::shared_ptr<Message> msg);
  void onMessageCallbackCompletion(std::shared_ptr<Message> msg);
  void abandonPendingMessages();
  bool setMessagePair(std::string msgStr1, std::string msgStr2);
  bool setMessageTimer(std::string msg, TimeKeeper::millisec timer);

  string to_string();

 private:
  qtimutex::QtiSharedMutex mMutex;
  std::shared_ptr<Message> mHandlingMsg;
  std::shared_ptr<Message> mRedispatchedMsg;
  /* Map of restricted messages for which callback are not yet executed.
  Key = Message string, Value = Deque of client message shared pointer*/
  std::deque<std::shared_ptr<Message> > mRestrictedMessagesQueue;
  std::unordered_map<std::string, std::string> mPairMessages;
  std::unordered_map<std::string, TimeKeeper::millisec> mMessageTimers;
  TimeKeeper::timer_id mTid{TimeKeeper::no_timer};
};
