/**
* Copyright (c) 2018 Qualcomm Technologies, Inc.
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
class DataAllBearerTypeChangedMessage: public UnSolicitedMessage,
                           public add_message_id<DataAllBearerTypeChangedMessage> {

private:
  AllocatedBearer_t mBearerInfo;

public:
  static constexpr const char *MESSAGE_NAME = "DataAllBearerTypeChangedMessage";

  DataAllBearerTypeChangedMessage() = delete;
  ~DataAllBearerTypeChangedMessage();
  DataAllBearerTypeChangedMessage(AllocatedBearer_t const &bearerInfo);

  std::shared_ptr<UnSolicitedMessage> clone();
  const AllocatedBearer_t & getBearerInfo();
  string dump();
};

} //namespace
