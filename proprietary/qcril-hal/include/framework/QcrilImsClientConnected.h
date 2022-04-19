/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once

#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>

/**
 * QcrilImsClientConnected: Sent by the ims interface module (e.g.
 * android_ims_radio) to notify the ims client is connected.
 */

class QcrilImsClientConnected : public UnSolicitedMessage,
                                public add_message_id<QcrilImsClientConnected> {
public:
  static constexpr const char *MESSAGE_NAME = "QcrilImsClientConnected";

public:
  QcrilImsClientConnected() : UnSolicitedMessage(get_class_message_id()) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    return (std::make_shared<QcrilImsClientConnected>());
  }

  inline ~QcrilImsClientConnected() {}

  string dump() { return mName + "{}"; }
};
