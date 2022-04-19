/*===========================================================================

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#pragma once

#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"
#include "MessageCommon.h"

using namespace std;

namespace rildata {

class QualifiedNetworksChangeIndMessage : public UnSolicitedMessage,
                          public add_message_id<QualifiedNetworksChangeIndMessage> {
private:
    std::vector<QualifiedNetwork_t> mQualifiedNetworks;
  public:

    static constexpr const char *MESSAGE_NAME = "QualifiedNetworksChangeIndMessage";

    inline QualifiedNetworksChangeIndMessage(std::vector<QualifiedNetwork_t> const &qualifiedNetworks):
      UnSolicitedMessage(get_class_message_id())
    {
      mName = MESSAGE_NAME;
      mQualifiedNetworks = qualifiedNetworks;
    }
    ~QualifiedNetworksChangeIndMessage();

    std::shared_ptr<UnSolicitedMessage> clone();

    string dump();

    std::vector<QualifiedNetwork_t> getQualifiedNetworks();
};

}
