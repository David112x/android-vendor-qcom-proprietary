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

enum AttachListCap {
    ACTION_SUPPORTED,
    ACTION_NOT_SUPPORTED
};

class GetAttachListCapabilitySyncMessage : public SolicitedSyncMessage<AttachListCap>,
                                    public add_message_id<GetAttachListCapabilitySyncMessage>
{
public:
  static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.data.GET_ATTACH_LIST_CAP_SYNC";
  inline GetAttachListCapabilitySyncMessage(GenericCallback<AttachListCap> *callback) :
    SolicitedSyncMessage<AttachListCap>(get_class_message_id())
  {
    std::ignore=callback;
    mName = MESSAGE_NAME;
  }
  ~GetAttachListCapabilitySyncMessage();

  string dump();
};