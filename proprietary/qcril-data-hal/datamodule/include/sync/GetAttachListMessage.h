/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/SolicitedSyncMessage.h"
#include "framework/Util.h"
#include "framework/add_message_id.h"
#include "framework/Dispatcher.h"
#include <list>

class GetAttachListMessage : public SolicitedSyncMessage<std::list<uint16_t>>,
                                    public add_message_id<GetAttachListMessage>
{
 public:
  static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.data.GET_ATTACH_LIST";
  inline GetAttachListMessage(GenericCallback<std::list<uint16_t>> *callback) :
    SolicitedSyncMessage<std::list<uint16_t>>(get_class_message_id())
  {
    std::ignore = callback;
    mName = MESSAGE_NAME;
  }
  ~GetAttachListMessage();

  string dump();
};
