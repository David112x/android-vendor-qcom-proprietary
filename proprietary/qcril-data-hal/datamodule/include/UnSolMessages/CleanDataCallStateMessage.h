/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"

class CleanDataCallStateMessage: public UnSolicitedMessage,
                                 public add_message_id<CleanDataCallStateMessage> {
private:
  int cid;

public:
  static constexpr const char *MESSAGE_NAME = "CleanDataCallStateMessage";
  CleanDataCallStateMessage() = delete;
  ~CleanDataCallStateMessage();
  CleanDataCallStateMessage(int setcid);

  std::shared_ptr<UnSolicitedMessage> clone();
  int getcid();
  void setcid(int setcid);
  string dump();
};
