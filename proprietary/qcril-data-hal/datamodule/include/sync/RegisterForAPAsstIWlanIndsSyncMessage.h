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

class RegisterForAPAsstIWlanIndsSyncMessage : public SolicitedSyncMessage<int>,
                                    public add_message_id<RegisterForAPAsstIWlanIndsSyncMessage>
{
private:
  bool mRegister;

public:
  static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.data.RegisterForAPAsstIWlanIndsSyncMessage";
  inline RegisterForAPAsstIWlanIndsSyncMessage(GenericCallback<int> *callback) :
    SolicitedSyncMessage<int>(get_class_message_id())
  {
    ignore(callback);
    mName = MESSAGE_NAME;
  }

  void setParams(const bool _register)
  {
    mRegister = _register;
  }

  bool getParams()
  {
    return mRegister;
  }
  ~RegisterForAPAsstIWlanIndsSyncMessage() {}

  string dump() {
    return mName;
  }
};