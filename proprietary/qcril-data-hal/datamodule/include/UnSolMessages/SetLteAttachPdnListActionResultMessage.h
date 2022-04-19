/**
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#pragma once
#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"
#include "wireless_data_service_v01.h"

namespace rildata {

/********************** Class Definitions *************************/
class SetLteAttachPdnListActionResultMessage: public UnSolicitedMessage,
                           public add_message_id<SetLteAttachPdnListActionResultMessage> {

private:
  wds_set_lte_attach_pdn_list_action_result_ind_msg_v01 mResult;

public:
  static constexpr const char *MESSAGE_NAME = "SetLteAttachPdnListActionResult";

  SetLteAttachPdnListActionResultMessage(wds_set_lte_attach_pdn_list_action_result_ind_msg_v01 const &p);
  ~SetLteAttachPdnListActionResultMessage();

  std::shared_ptr<UnSolicitedMessage> clone();

  const wds_set_lte_attach_pdn_list_action_result_ind_msg_v01 & getParams();

  string dump();
};

} //namespace
