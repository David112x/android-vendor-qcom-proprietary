/*===========================================================================

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#pragma once

#include "MessageCommon.h"

using namespace std;

namespace rildata {

class GetIWlanDataCallListRequestMessage : public SolicitedMessage<DataCallListResult_t>,
                          public add_message_id<GetIWlanDataCallListRequestMessage> {
  public:

    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.data.GetIWlanDataCallListRequestMessage";

    inline GetIWlanDataCallListRequestMessage():
      SolicitedMessage<DataCallListResult_t>(get_class_message_id())
    {
      mName = MESSAGE_NAME;
    }
    ~GetIWlanDataCallListRequestMessage() = default;

    string dump(){return mName;}
};

}
