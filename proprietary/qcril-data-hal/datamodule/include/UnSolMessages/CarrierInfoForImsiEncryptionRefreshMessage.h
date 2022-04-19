/**
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#pragma once
#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"

namespace rildata {

/********************** Class Definitions *************************/
class CarrierInfoForImsiEncryptionRefreshMessage: public UnSolicitedMessage,
                           public add_message_id<CarrierInfoForImsiEncryptionRefreshMessage> {

public:
  static constexpr const char *MESSAGE_NAME = "CarrierInfoForImsiEncryptionRefreshMessage";
  CarrierInfoForImsiEncryptionRefreshMessage():UnSolicitedMessage(get_class_message_id()){};
  ~CarrierInfoForImsiEncryptionRefreshMessage(){};

  std::shared_ptr<UnSolicitedMessage> clone(){
    return std::make_shared<CarrierInfoForImsiEncryptionRefreshMessage>();
  };
  string dump(){return MESSAGE_NAME;};
};

} //namespace
