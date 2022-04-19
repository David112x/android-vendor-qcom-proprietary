/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include "UnSolMessages/SetApnPreferredSystemResultMessage.h"

using namespace rildata;

SetApnPreferredSystemResultMessage::SetApnPreferredSystemResultMessage
(
  dsd_ap_asst_apn_pref_sys_result_ind_msg_v01 const &p
) : UnSolicitedMessage(get_class_message_id())
{
  mName = MESSAGE_NAME;
  mResult = p;
}

SetApnPreferredSystemResultMessage::~SetApnPreferredSystemResultMessage()
{
}

std::shared_ptr<UnSolicitedMessage> SetApnPreferredSystemResultMessage::clone()
{
  return std::make_shared<SetApnPreferredSystemResultMessage>(mResult);
}

const dsd_ap_asst_apn_pref_sys_result_ind_msg_v01& SetApnPreferredSystemResultMessage::getParams()
{
  return mResult;
}

string SetApnPreferredSystemResultMessage::dump()
{
  return SetApnPreferredSystemResultMessage::MESSAGE_NAME;
}
