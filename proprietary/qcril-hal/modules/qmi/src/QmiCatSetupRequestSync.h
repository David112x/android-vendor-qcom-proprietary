/******************************************************************************
#  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include <string>
#include "framework/SolicitedSyncMessage.h"
#include "framework/add_message_id.h"

using std::string;

class QmiCatSetupRequestSync : public SolicitedSyncMessage<string>,
                               public add_message_id<QmiCatSetupRequestSync>
{
 public:
  static constexpr const char *MESSAGE_NAME = "QMI_CAT_CLIENT_SETUP_SYNC_REQ";

  inline QmiCatSetupRequestSync(string clientToken):
    SolicitedSyncMessage< string >(get_class_message_id())
  {
    mToken = clientToken;
    mName = MESSAGE_NAME;
  }

  ~QmiCatSetupRequestSync()
  {
  }

  string dump()
  {
    return mName + "{ client token = " + mToken + "}";
  }

 private:
  string mToken;
};
