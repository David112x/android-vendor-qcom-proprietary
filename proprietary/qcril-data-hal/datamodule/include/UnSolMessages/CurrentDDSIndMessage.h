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
#include "DataCommon.h"

namespace rildata {

/********************** Class Definitions *************************/
class CurrentDDSIndMessage: public UnSolicitedMessage,
                            public add_message_id<CurrentDDSIndMessage> {

private:
  int subId;
  dsd_dds_switch_type switchType;

public:
  static constexpr const char *MESSAGE_NAME = "CurrentDDSIndMessage";

  CurrentDDSIndMessage() = delete;
  ~CurrentDDSIndMessage();
  CurrentDDSIndMessage(int setSubId, dsd_dds_switch_type setSwitch);

  std::shared_ptr<UnSolicitedMessage> clone();
  int getSubId();
  dsd_dds_switch_type getSwitchType();
  string dump();
};

} //namespace
