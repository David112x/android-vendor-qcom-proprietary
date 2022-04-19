/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include "UnSolMessages/IntentToChangeApnPreferredSystemMessage.h"

using namespace rildata;

IntentToChangeApnPreferredSystemMessage::IntentToChangeApnPreferredSystemMessage
(
  dsd_intent_to_change_apn_pref_sys_ind_msg_v01 const &p
) : UnSolicitedMessage(get_class_message_id())
{
  mName = MESSAGE_NAME;
  mResult = p;
}

IntentToChangeApnPreferredSystemMessage::~IntentToChangeApnPreferredSystemMessage()
{
}

std::shared_ptr<UnSolicitedMessage> IntentToChangeApnPreferredSystemMessage::clone()
{
  return std::make_shared<IntentToChangeApnPreferredSystemMessage>(mResult);
}

const dsd_intent_to_change_apn_pref_sys_ind_msg_v01& IntentToChangeApnPreferredSystemMessage::getParams()
{
  return mResult;
}

string IntentToChangeApnPreferredSystemMessage::dump()
{
  return IntentToChangeApnPreferredSystemMessage::MESSAGE_NAME;
}
