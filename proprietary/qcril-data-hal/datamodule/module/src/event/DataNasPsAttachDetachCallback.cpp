/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include "framework/Log.h"
#include "framework/legacy.h"
#include "event/DataNasPsAttachDetachCallback.h"
#include "ProfileHandler.h"


void DataNasPsAttachDetachCallback::onResponse(
    std::shared_ptr<Message> solicitedMsg, Status status,
    std::shared_ptr<RIL_Errno> responseDataPtr)
{
  mStatus = status;
  if (mStatus == Message::Callback::Status::SUCCESS) {
    Log::getInstance().d("[DataNasPsAttachDetachCallback]: Callback[msg = " +
        solicitedMsg->dump() + "] executed. client data = " +
        mClientToken + " status = Message::Callback::Status::SUCCESS");
    mResponseData = responseDataPtr;
    std::shared_ptr<RIL_Errno> resp = std::static_pointer_cast<RIL_Errno>(responseDataPtr);
    if(resp)
    {
      RIL_Errno ret;
      std::memset(&ret, 0, sizeof(RIL_Errno));
      memcpy(&ret, resp.get(), sizeof(RIL_Errno));

      ProfileHandler::processNasPsAttachDetachResp(&ret);

    } else {
      Log::getInstance().d("[DataNasPsAttachDetachCallback]: resp is NULL..not processing ");
    }
  } else {
    Log::getInstance().d("[DataNasPsAttachDetachCallback]: Callback[msg = " +
        solicitedMsg->dump() + "] executed. client data = " +
        mClientToken + " FAILURE !!");
  }
}

Message::Callback::Status DataNasPsAttachDetachCallback::getStatus()
{
  if (mStatus == Message::Callback::Status::SUCCESS) {
    Log::getInstance().d("[DataNasPsAttachDetachCallback]: Message::Callback::Status::SUCCESS");
  } else {
    Log::getInstance().d("[DataNasPsAttachDetachCallback]: not SUCCESS");
  }
  return mStatus;
}

Message::Callback *DataNasPsAttachDetachCallback::clone()
{
  DataNasPsAttachDetachCallback *clone = new DataNasPsAttachDetachCallback(mClientToken);
  return clone;
}
