/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include "MessageCommon.h"

using namespace std;

namespace rildata {

class StopKeepAliveRequestMessage : public SolicitedMessage<ResponseError_t>,
                          public add_message_id<StopKeepAliveRequestMessage> {
  private:
    int32_t mSessionHandle;
    int32_t mSerial;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.data.StopKeepAliveRequestMessage";
    StopKeepAliveRequestMessage() = delete;
    StopKeepAliveRequestMessage(
      const int32_t serial,
      const int32_t sessionHandle):SolicitedMessage<ResponseError_t>(get_class_message_id()) {

      mName = MESSAGE_NAME;
      mSerial = serial;
      mSessionHandle = sessionHandle;

    }
    ~StopKeepAliveRequestMessage() = default;

    string dump(){return mName;}
    int32_t getSerial() {return mSerial;}
    int32_t* getHandle() {return &mSessionHandle;}
};

}
