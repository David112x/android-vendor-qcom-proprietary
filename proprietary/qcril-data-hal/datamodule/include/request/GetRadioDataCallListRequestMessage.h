/*===========================================================================

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#pragma once

#include "MessageCommon.h"

using namespace std;

namespace rildata {

class GetRadioDataCallListRequestMessage : public SolicitedMessage<DataCallListResult_t>,
                          public add_message_id<GetRadioDataCallListRequestMessage> {
  public:

    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.data.GetRadioDataCallListRequestMessage";

    inline GetRadioDataCallListRequestMessage():
      SolicitedMessage<DataCallListResult_t>(get_class_message_id())
    {
      mName = MESSAGE_NAME;
    }
    ~GetRadioDataCallListRequestMessage() = default;

    string dump(){return mName;}
};

}
