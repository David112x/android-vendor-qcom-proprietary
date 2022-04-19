/**
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#pragma once
#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"
#include "MessageCommon.h"

namespace rildata {

/********************** Class Definitions *************************/
class DataNrIconTypeIndMessage: public UnSolicitedMessage,
                           public add_message_id<DataNrIconTypeIndMessage> {

private:
  NrIconType_t mIcon;
public:
  static constexpr const char *MESSAGE_NAME = "DataNrIconTypeIndMessage";

  DataNrIconTypeIndMessage() = delete;
  ~DataNrIconTypeIndMessage();
  DataNrIconTypeIndMessage(NrIconType_t setIcon);

  std::shared_ptr<UnSolicitedMessage> clone();
  bool isUwb();
  bool isBasic();
  bool isNone();
  string dump();
};

} //namespace
