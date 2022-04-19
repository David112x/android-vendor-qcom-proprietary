/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/SolicitedSyncMessage.h"
#include "framework/Util.h"
#include "framework/add_message_id.h"
#include "framework/Dispatcher.h"
#include <list>

typedef uint64_t BringUpCapability;
#define BRING_UP_LEGACY (1 << 0)
#define BRING_UP_APN_NAME (1 << 1)
#define BRING_UP_APN_TYPE (1 << 2)
class GetCallBringUpCapabilitySyncMessage : public SolicitedSyncMessage<BringUpCapability>,
                                    public add_message_id<GetCallBringUpCapabilitySyncMessage>
{
public:
  static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.data.GET_CALL_BRING_UP_CAPABILITY_SYNC";
  inline GetCallBringUpCapabilitySyncMessage(GenericCallback<BringUpCapability> *callback) :
    SolicitedSyncMessage<BringUpCapability>(get_class_message_id())
  {
    std::ignore=callback;
    mName = MESSAGE_NAME;
  }
  ~GetCallBringUpCapabilitySyncMessage();

  string dump();
};
