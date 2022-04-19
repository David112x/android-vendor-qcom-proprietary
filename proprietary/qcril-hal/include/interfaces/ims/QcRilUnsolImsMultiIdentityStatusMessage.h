/******************************************************************************
#  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>
#include <interfaces/ims/ims.h>
/*
 * Unsol message to notify multiline status
 *
 */
class QcRilUnsolImsMultiIdentityStatusMessage : public UnSolicitedMessage,
                                 public add_message_id<QcRilUnsolImsMultiIdentityStatusMessage> {
 private:
  std::vector<qcril::interfaces::MultiIdentityInfo> mLineInfo;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolImsMultiIdentityStatusMessage";
  ~QcRilUnsolImsMultiIdentityStatusMessage() { }

  QcRilUnsolImsMultiIdentityStatusMessage(
    std::vector<qcril::interfaces::MultiIdentityInfo> info)
      : UnSolicitedMessage(get_class_message_id()) ,mLineInfo(info) {}

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolImsMultiIdentityStatusMessage> msg =
        std::make_shared<QcRilUnsolImsMultiIdentityStatusMessage>(mLineInfo);
    return msg;
  }

  std::vector<qcril::interfaces::MultiIdentityInfo> &getLineInfo() {
    return mLineInfo;
  }

  string dump() {
    return QcRilUnsolImsMultiIdentityStatusMessage::MESSAGE_NAME;
  }
};
