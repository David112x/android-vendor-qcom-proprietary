/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#include <framework/AddPendingMessageList.h>
#include <interfaces/QcRilRequestMessage.h>

AddPendingMessageList::AddPendingMessageList(std::string name) : mMessageList(name) {}

PendingMessageList &AddPendingMessageList::getPendingMessageList() { return mMessageList; }

void AddPendingMessageList::clearPendingMessageList() {
  while (!mMessageList.empty()) {
    auto pendingMsg = mMessageList.pop();
    auto msg(std::static_pointer_cast<QcRilRequestMessage>(pendingMsg));
    Log::getInstance().d("clearPendingMessageList: msg = " + msg->dump());
    auto respPayload =
        std::make_shared<QcRilRequestMessageCallbackPayload>(RIL_E_RADIO_NOT_AVAILABLE, nullptr);
    msg->sendResponse(msg, Message::Callback::Status::SUCCESS, respPayload);
  }
}
