/*===========================================================================

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#pragma once

#include "MessageCommon.h"

using namespace std;

namespace rildata {

struct DataCallListResult_t {
  ResponseError_t respErr;
  vector<DataCallResult_t> call;
};

class GetDataCallListRequestMessage : public SolicitedMessage<DataCallListResult_t>,
                          public add_message_id<GetDataCallListRequestMessage> {
  public:
    int32_t mSerial;

    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.data.GetDataCallListRequestMessage";
    GetDataCallListRequestMessage() = delete;

    inline GetDataCallListRequestMessage(int32_t serial):
      SolicitedMessage<DataCallListResult_t>(get_class_message_id())
    {
      mName = MESSAGE_NAME;
      mSerial = serial;
    }
    ~GetDataCallListRequestMessage() = default;

    string dump(){return mName;}
};

}
