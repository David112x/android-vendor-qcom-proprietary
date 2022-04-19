/**
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#pragma once
#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"
#include "data_system_determination_v01.h"

namespace rildata {

class DsdSystemStatusPerApnMessage: public UnSolicitedMessage,
                           public add_message_id<DsdSystemStatusPerApnMessage> {

private:
  vector<dsd_apn_avail_sys_info_type_v01> avail_sys;

public:
  static constexpr const char *MESSAGE_NAME = "DsdSystemStatusPerApnMessage";

  DsdSystemStatusPerApnMessage() = delete;
  DsdSystemStatusPerApnMessage(const vector<dsd_apn_avail_sys_info_type_v01>& p)
  : UnSolicitedMessage(get_class_message_id()) {
    mName = MESSAGE_NAME;
    avail_sys = p;
  }
  ~DsdSystemStatusPerApnMessage(){};

  std::shared_ptr<UnSolicitedMessage> clone() {
    return std::make_shared<DsdSystemStatusPerApnMessage>(avail_sys);
  }

  vector<dsd_apn_avail_sys_info_type_v01> getAvailSys() {
    return avail_sys;
  }

  string dump() {
    return MESSAGE_NAME;
  }
};

} //namespace
