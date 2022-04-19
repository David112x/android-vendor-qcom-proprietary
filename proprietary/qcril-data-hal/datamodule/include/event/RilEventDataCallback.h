/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#pragma once
#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include <framework/add_message_id.h>
#include "dsi_netctrl.h"

namespace rildata {

typedef struct
{
  dsi_net_evt_t      evt;
  void              *data;
  int                data_len;
  void              *self;
  dsi_evt_payload_t  payload;
} EventDataType;

class RilEventDataCallback : public UnSolicitedMessage,
                             public add_message_id<RilEventDataCallback> {
private:
  EventDataType eventData;

public:
  static constexpr const char *MESSAGE_NAME = "QCRIL_EVT_DATA_EVENT_CALLBACK";
  ~RilEventDataCallback() = default;

  inline RilEventDataCallback(EventDataType p): UnSolicitedMessage(get_class_message_id())
  {
    eventData = p;
    mName = MESSAGE_NAME;
  }

  inline std::shared_ptr<UnSolicitedMessage> clone()
  {
    return std::static_pointer_cast<UnSolicitedMessage>(std::make_shared<RilEventDataCallback>(eventData));
  }

  inline string dump()
  {
    return mName;
  }

  inline EventDataType *getEventData() {
    return &eventData;
  }
};

} //namespace
