/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>
#include <interfaces/ims/ims.h>

/*
 * Unsol message to notify change in vops
 *
 */
class QcRilUnsolImsVopsIndication : public UnSolicitedMessage,
                                     public add_message_id<QcRilUnsolImsVopsIndication> {
 private:
  bool mVopsStatus;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolImsVopsIndication";
  ~QcRilUnsolImsVopsIndication() { }

  QcRilUnsolImsVopsIndication(bool status)
      : UnSolicitedMessage(get_class_message_id()), mVopsStatus(status) {}

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolImsVopsIndication> msg =
        std::make_shared<QcRilUnsolImsVopsIndication>(mVopsStatus);
    return msg;
  }

  bool getVopsStatus() { return mVopsStatus; };

  string dump() {
    return QcRilUnsolImsVopsIndication::MESSAGE_NAME;
  }
};
