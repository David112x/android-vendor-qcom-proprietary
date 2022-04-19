/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <UnSolMessages/DataRegistrationStateMessage.h>

using namespace rildata;

DataRegistrationStateMessage::~DataRegistrationStateMessage()
{
}

DataRegistrationStateMessage::DataRegistrationStateMessage
(
   int instance_id,
   const Rat setRatValue
) : UnSolicitedMessage(get_class_message_id())
{
  mName = MESSAGE_NAME;
  mInstanceId = instance_id;
  mRatValue = setRatValue;
}

std::shared_ptr<UnSolicitedMessage> DataRegistrationStateMessage::clone()
{
  return std::make_shared<DataRegistrationStateMessage>
                          (mInstanceId, mRatValue);
}

Rat DataRegistrationStateMessage::getRatValue()
{
  return mRatValue;
}

string DataRegistrationStateMessage::dump()
{
  string result = DataRegistrationStateMessage::MESSAGE_NAME;
  result += "Rat = ";
  switch (mRatValue) {
  case UNKNOWN:
    result += "UNKNOWN";
    break;
  case NR_NSA:
    result += "NR_NSA";
    break;
  case NR_NSA_MMWV:
    result += "NR_NSA_MMWV";
    break;
  case NR_SA:
    result += "NR_SA";
    break;
  case NR_SA_MMWV:
    result += "NR_SA_MMWV";
    break;
  default:
    result += "Invalid value";
    break;
  }
  return result;
}

int DataRegistrationStateMessage::getInstanceId()
{
  return mInstanceId;
}
