/**
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#include "framework/SolicitedMessage.h"
#include "framework/add_message_id.h"
#include "MessageCommon.h"

namespace rildata {

class GetBearerAllocationRequestMessage : public SolicitedMessage<AllocatedBearerResult_t>,
                           public add_message_id<GetBearerAllocationRequestMessage> {
  private:
    int32_t callId;

  public:
    static constexpr const char *MESSAGE_NAME = "GetBearerAllocationRequestMessage";
    GetBearerAllocationRequestMessage(int32_t p):SolicitedMessage<AllocatedBearerResult_t>(get_class_message_id()), callId(p) {
      mName = MESSAGE_NAME;
    }
    ~GetBearerAllocationRequestMessage();

    int32_t getCallId() { return callId; }
    string dump();
};
}
