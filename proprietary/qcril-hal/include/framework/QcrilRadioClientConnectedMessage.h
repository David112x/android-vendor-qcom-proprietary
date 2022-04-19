/******************************************************************************
#  Copyright (c) 2020 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once

#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>

class QcrilRadioClientConnectedMessage : public UnSolicitedMessage,
                                public add_message_id<QcrilRadioClientConnectedMessage> {
public:
  static constexpr const char *MESSAGE_NAME = "QcrilRadioClientConnectedMessage";

public:
  QcrilRadioClientConnectedMessage() : UnSolicitedMessage(get_class_message_id()) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    return (std::make_shared<QcrilRadioClientConnectedMessage>());
  }

  inline ~QcrilRadioClientConnectedMessage() {}

  string dump() { return mName + "{}"; }
};
