/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include "framework/Message.h"
#include <QtiMutex.h>

class PendingMessageList {
  struct PendingMessage {
    std::shared_ptr<Message> mMessage;
  };

private:
  std::string mName;

  /* HashMap of token from Message and PendingMessage */
  std::unordered_map<uint16_t, PendingMessage> mList;
  uint16_t mToken;
  qtimutex::QtiSharedMutex mMutex;

public:
  PendingMessageList(std::string name);
  bool empty();
  bool size();
  std::shared_ptr<Message> pop();
  std::pair<uint16_t, bool> insert(std::shared_ptr<Message> message);
  std::shared_ptr<Message> find(uint16_t token);
  std::shared_ptr<Message> find(message_id_t messageId);
  std::shared_ptr<Message> extract(uint16_t token);
  std::shared_ptr<Message> extract(message_id_t messageId);
  bool erase(uint16_t token);
  bool erase(std::shared_ptr<Message> message);
  /** Erase all the elements from the list */
  void clear();
  void print();
};
