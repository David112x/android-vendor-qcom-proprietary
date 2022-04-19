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
class QcRilUnsolImsPendingMultiLineStatus : public UnSolicitedMessage,
                                     public add_message_id<QcRilUnsolImsPendingMultiLineStatus> {

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolImsPendingMultiLineStatus";
  ~QcRilUnsolImsPendingMultiLineStatus() { }

  QcRilUnsolImsPendingMultiLineStatus()
      : UnSolicitedMessage(get_class_message_id()) {}

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolImsPendingMultiLineStatus> msg =
        std::make_shared<QcRilUnsolImsPendingMultiLineStatus>();
    return msg;
  }

  string dump() {
    return QcRilUnsolImsPendingMultiLineStatus::MESSAGE_NAME;
  }
};
