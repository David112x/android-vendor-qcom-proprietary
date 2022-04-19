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
class DataBearerTypeChangedMessage: public UnSolicitedMessage,
                           public add_message_id<DataBearerTypeChangedMessage> {

private:
  int32_t mCid;
  BearerInfo_t mBearerInfo;
public:
  static constexpr const char *MESSAGE_NAME = "DataBearerTypeChangedMessage";

  DataBearerTypeChangedMessage() = delete;
  ~DataBearerTypeChangedMessage();
  DataBearerTypeChangedMessage(int32_t cid, BearerInfo_t const &bearerInfo);

  std::shared_ptr<UnSolicitedMessage> clone();
  int32_t getCid();
  const BearerInfo_t & getBearerInfo();
  string dump();
};

} //namespace
