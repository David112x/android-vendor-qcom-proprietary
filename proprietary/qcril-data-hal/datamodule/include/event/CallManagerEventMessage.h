/**
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#pragma once
#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include <framework/add_message_id.h>

namespace rildata {

enum CallManagerEventType_t {
  _eCallManagerEventNone = 0,
  _eCallManagerEventPcscfAddressChanged,
  _eCallManagerEventLinkDormant,
  _eCallManagerEventLinkActive,
};

struct CallManagerEvent_t {
  CallManagerEventType_t type;
  int                    cid;
};

class CallManagerEventMessage : public UnSolicitedMessage,
                             public add_message_id<CallManagerEventMessage> {
private:
  CallManagerEvent_t mEvent;

public:
  static constexpr const char *MESSAGE_NAME = "CallManagerEventMessage";
  ~CallManagerEventMessage() = default;

  inline CallManagerEventMessage(CallManagerEvent_t evt): UnSolicitedMessage(get_class_message_id())
  {
    mEvent = evt;
    mName = MESSAGE_NAME;
  }

  inline std::shared_ptr<UnSolicitedMessage> clone()
  {
    return std::static_pointer_cast<UnSolicitedMessage>(std::make_shared<CallManagerEventMessage>(mEvent));
  }

  inline string dump()
  {
    return mName;
  }

  inline CallManagerEvent_t getCallManagerEvent() {
    return mEvent;
  }
};

} //namespace
