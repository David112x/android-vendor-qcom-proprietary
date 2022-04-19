/**
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#pragma once
#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"
#include "MessageCommon.h"

namespace rildata {

/********************** Class Definitions *************************/
class UiInfoFragmentMessage: public UnSolicitedMessage,
                           public add_message_id<UiInfoFragmentMessage> {
  NrIconEnum_t mIcon;
public:
  static constexpr const char *MESSAGE_NAME = "UiInfoFragmentMessage";

  UiInfoFragmentMessage() = delete;
  ~UiInfoFragmentMessage() {}
  UiInfoFragmentMessage(NrIconEnum_t setIcon): UnSolicitedMessage(get_class_message_id()),
    mIcon(setIcon) { mName = MESSAGE_NAME; }

  NrIconEnum_t getIcon() { return mIcon; }
  std::shared_ptr<UnSolicitedMessage> clone() {
    return std::make_shared<UiInfoFragmentMessage>(mIcon);
  }
  string dump() {
    return std::string(MESSAGE_NAME);
  }
};

} //namespace
