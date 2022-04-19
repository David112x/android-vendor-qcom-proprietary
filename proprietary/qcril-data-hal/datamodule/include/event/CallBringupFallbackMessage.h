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

class CallBringupFallbackMessage : public UnSolicitedMessage,
                             public add_message_id<CallBringupFallbackMessage> {
private:
  int cid;

public:
  static constexpr const char *MESSAGE_NAME = "QCRIL_DATA_CALL_BRINGUP_FALLBACK";
  ~CallBringupFallbackMessage() = default;

  inline CallBringupFallbackMessage(int c): UnSolicitedMessage(get_class_message_id())
  {
    cid = c;
    mName = MESSAGE_NAME;
  }

  inline std::shared_ptr<UnSolicitedMessage> clone()
  {
    return std::static_pointer_cast<UnSolicitedMessage>(std::make_shared<CallBringupFallbackMessage>(cid));
  }

  inline string dump()
  {
    return mName;
  }

  inline int getCid() {
    return cid;
  }
};

} //namespace
