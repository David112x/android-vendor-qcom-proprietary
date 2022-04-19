/*===========================================================================

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#pragma once

#include "MessageCommon.h"

using namespace std;

namespace rildata {

struct IWlanDataRegistrationStateResult_t {
  ResponseError_t respErr;
  DataRegState_t regState;
  int32_t reasonForDenial;
};

class GetIWlanDataRegistrationStateRequestMessage : public SolicitedMessage<IWlanDataRegistrationStateResult_t>,
                          public add_message_id<GetIWlanDataRegistrationStateRequestMessage> {
  public:

    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.data.GetIWlanDataRegistrationStateRequestMessage";

    inline GetIWlanDataRegistrationStateRequestMessage():
      SolicitedMessage<IWlanDataRegistrationStateResult_t>(get_class_message_id())
    {
      mName = MESSAGE_NAME;
    }
    ~GetIWlanDataRegistrationStateRequestMessage() = default;

    string dump(){return mName;}
};

}
