/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once
#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"
#include "MessageCommon.h"

namespace rildata {

enum class TriggerDDSSwitchError : int {
  MODEM_NOT_UP = -1,
  NO_ERROR = 0
};

/********************** Class Definitions *************************/
class DDSSwitchResultIndMessage: public UnSolicitedMessage,
                                 public add_message_id<DDSSwitchResultIndMessage> {
private:
  bool allowed;
  bool failed;
  TriggerDDSSwitchError error;

public:
  static constexpr const char *MESSAGE_NAME = "DDSSwitchResultIndMessage";

  DDSSwitchResultIndMessage();
  ~DDSSwitchResultIndMessage();

  bool getAllowed();
  void setAllowed(bool setAllowed);
  bool getFailed();
  void setFailed(bool setFailed);
  TriggerDDSSwitchError getError();
  void setError(TriggerDDSSwitchError setError);
  std::shared_ptr<UnSolicitedMessage> clone();
  string dump();
};

} //namespace
