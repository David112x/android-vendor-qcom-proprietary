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
#include "MessageCommon.h"

namespace rildata {

class SetProfileParamsSyncMessage : public SolicitedSyncMessage<int>,
                                    public add_message_id<SetProfileParamsSyncMessage>
{
private:
  vector<DataProfileInfo_t> mProfiles;

public:
  static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.data.SET_PROFILE_PARAMS_SYNC_MESSAGE";
  inline SetProfileParamsSyncMessage(GenericCallback<int> *callback) :
    SolicitedSyncMessage<int>(get_class_message_id())
  {
    std::ignore = callback;
    mName = MESSAGE_NAME;
  }
  ~SetProfileParamsSyncMessage() {}

  void setParams(const vector<DataProfileInfo_t>& pInfo)
  {
    mProfiles = pInfo;
  }

  vector<DataProfileInfo_t> getParams()
  {
    return mProfiles;
  }

  string dump() { return mName; }
};

}
