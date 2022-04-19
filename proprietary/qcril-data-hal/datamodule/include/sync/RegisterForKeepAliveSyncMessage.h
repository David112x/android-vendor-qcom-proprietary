/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/SolicitedSyncMessage.h"
#include "framework/Util.h"
#include "framework/add_message_id.h"
#include "DSDModemEndPoint.h"

class RegisterForKeepAliveSyncMessage : public SolicitedSyncMessage<int>,
                                    public add_message_id<RegisterForKeepAliveSyncMessage>
{
private:
  bool mRegister;

public:
  static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.data.RegisterForKeepAliveSyncMessage";
  inline RegisterForKeepAliveSyncMessage(GenericCallback<int> *callback) :
    SolicitedSyncMessage<int>(get_class_message_id())
  {
    (void)(callback);
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

  ~RegisterForKeepAliveSyncMessage()
  {
  }

  string dump()
  {
    return mName;
  }
};

