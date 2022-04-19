/**
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#pragma once
#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"
#include "data_system_determination_v01.h"

namespace rildata {

/********************** Class Definitions *************************/
class IntentToChangeApnPreferredSystemMessage: public UnSolicitedMessage,
                           public add_message_id<IntentToChangeApnPreferredSystemMessage> {

private:
  dsd_intent_to_change_apn_pref_sys_ind_msg_v01 mResult;

public:
  static constexpr const char *MESSAGE_NAME = "IntentToChangeApnPreferredSystemMessage";

  IntentToChangeApnPreferredSystemMessage(dsd_intent_to_change_apn_pref_sys_ind_msg_v01 const &p);
  ~IntentToChangeApnPreferredSystemMessage();

  std::shared_ptr<UnSolicitedMessage> clone();

  const dsd_intent_to_change_apn_pref_sys_ind_msg_v01 & getParams();

  string dump();
};

} //namespace
