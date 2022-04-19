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
class SetApnPreferredSystemResultMessage: public UnSolicitedMessage,
                           public add_message_id<SetApnPreferredSystemResultMessage> {

private:
  dsd_ap_asst_apn_pref_sys_result_ind_msg_v01 mResult;

public:
  static constexpr const char *MESSAGE_NAME = "SetApnPreferredSystemResultMessage";

  SetApnPreferredSystemResultMessage(dsd_ap_asst_apn_pref_sys_result_ind_msg_v01 const &p);
  ~SetApnPreferredSystemResultMessage();

  std::shared_ptr<UnSolicitedMessage> clone();

  const dsd_ap_asst_apn_pref_sys_result_ind_msg_v01 & getParams();

  string dump();
};

} //namespace
