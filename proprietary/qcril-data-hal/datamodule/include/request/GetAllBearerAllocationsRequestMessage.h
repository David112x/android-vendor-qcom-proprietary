/**
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#include "framework/SolicitedMessage.h"
#include "framework/add_message_id.h"
#include "MessageCommon.h"

namespace rildata {

class GetAllBearerAllocationsRequestMessage : public SolicitedMessage<AllocatedBearerResult_t>,
                                              public add_message_id<GetAllBearerAllocationsRequestMessage> {
  public:
    static constexpr const char *MESSAGE_NAME = "GetAllBearerAllocationsRequestMessage";
    GetAllBearerAllocationsRequestMessage():SolicitedMessage<AllocatedBearerResult_t>(get_class_message_id()) {
      mName = MESSAGE_NAME;
    }
    ~GetAllBearerAllocationsRequestMessage();

    string dump();
};
}
