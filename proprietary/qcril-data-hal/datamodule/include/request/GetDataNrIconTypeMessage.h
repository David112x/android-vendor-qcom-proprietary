/**
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#include "framework/SolicitedMessage.h"
#include "framework/add_message_id.h"
#include "MessageCommon.h"

namespace rildata {

class GetDataNrIconTypeMessage : public SolicitedMessage<NrIconType_t>,
                           public add_message_id<GetDataNrIconTypeMessage> {
  public:
    static constexpr const char *MESSAGE_NAME = "GetDataNrIconTypeMessage";
    GetDataNrIconTypeMessage():SolicitedMessage<NrIconType_t>(get_class_message_id()) {
      mName = MESSAGE_NAME;
    }
    ~GetDataNrIconTypeMessage();

    string dump();
};
}
