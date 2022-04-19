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

class GetApnTypesForName : public SolicitedSyncMessage<std::list<int32_t>>,
                                    public add_message_id<GetApnTypesForName>
{
 public:
  static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.data.GetApnTypesForName";
  inline GetApnTypesForName() :
    SolicitedSyncMessage<std::list<int32_t>>(get_class_message_id())
  {
    mName = MESSAGE_NAME;
  }
  ~GetApnTypesForName() { }

  void setParams(const string apnName)
  {
    mApnName = apnName;
  }

  string getParams()
  {
    return mApnName;
  }

  string dump()
  { return mName; }

private:
  string mApnName;
};