/**
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#pragma once
#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"

namespace rildata {

enum class DataCallTimerType : int {
  Handover = 0,
  SetupDataCall = 1,
  PartialRetry = 2,
  PartialRetryResponse = 3,
  DeactivateDataCall = 4,
};

/********************** Class Definitions *************************/
class DataCallTimerExpiredMessage: public UnSolicitedMessage,
                           public add_message_id<DataCallTimerExpiredMessage> {
private:
  int mCid;
  DataCallTimerType mType;

public:
  static constexpr const char *MESSAGE_NAME = "DataCallTimerExpiredMessage";

  DataCallTimerExpiredMessage(int cid, DataCallTimerType type);
  ~DataCallTimerExpiredMessage();

  int getCid();
  DataCallTimerType getType();
  static string getStringType(DataCallTimerType);

  std::shared_ptr<UnSolicitedMessage> clone();

  string dump();
};

} //namespace