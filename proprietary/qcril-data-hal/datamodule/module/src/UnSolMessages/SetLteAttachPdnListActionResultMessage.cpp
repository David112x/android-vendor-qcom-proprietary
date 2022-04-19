/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include "UnSolMessages/SetLteAttachPdnListActionResultMessage.h"

using namespace rildata;

SetLteAttachPdnListActionResultMessage::SetLteAttachPdnListActionResultMessage
(
  wds_set_lte_attach_pdn_list_action_result_ind_msg_v01 const &p
) : UnSolicitedMessage(get_class_message_id())
{
  mName = MESSAGE_NAME;
  mResult = p;
}

SetLteAttachPdnListActionResultMessage::~SetLteAttachPdnListActionResultMessage()
{
}

std::shared_ptr<UnSolicitedMessage> SetLteAttachPdnListActionResultMessage::clone()
{
  return std::make_shared<SetLteAttachPdnListActionResultMessage>(mResult);
}

const wds_set_lte_attach_pdn_list_action_result_ind_msg_v01& SetLteAttachPdnListActionResultMessage::getParams()
{
  return mResult;
}

string SetLteAttachPdnListActionResultMessage::dump()
{
  return SetLteAttachPdnListActionResultMessage::MESSAGE_NAME;
}
