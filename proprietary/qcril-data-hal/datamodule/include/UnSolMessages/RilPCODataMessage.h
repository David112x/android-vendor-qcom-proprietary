/**
* Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#pragma once
#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include <framework/add_message_id.h>
#include "MessageCommon.h"

#define UPDATE_PCO_DATA

namespace rildata {

struct PcoDataInfo_t {
  int32_t cid;
  std::string bearerProto;
  int32_t pcoId;
  std::vector<uint8_t> contents;
};

class RilPCODataMessage : public UnSolicitedMessage,public add_message_id<RilPCODataMessage>  {
private:
  PcoDataInfo_t mPcoData;

public:
  static constexpr const char *MESSAGE_NAME = "RilPCODataMessage";

  RilPCODataMessage() = delete;
  RilPCODataMessage(PcoDataInfo_t const &p);
  ~RilPCODataMessage();

  PcoDataInfo_t getPcoDataInfo() {
    return mPcoData;
  }

  std::shared_ptr<UnSolicitedMessage> clone();

  string dump() {
    return RilPCODataMessage::MESSAGE_NAME;
  }
};
} //namespace
