/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>

/*
 * Unsol message to notify voice rat change
 *
 */
class RilUnsolUiccSubsStatusChangedMessage : public UnSolicitedMessage,
            public add_message_id<RilUnsolUiccSubsStatusChangedMessage> {
 public:
  static constexpr const char *MESSAGE_NAME = "RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED";
  ~RilUnsolUiccSubsStatusChangedMessage() { }

  RilUnsolUiccSubsStatusChangedMessage(bool status)
      : UnSolicitedMessage(get_class_message_id()), mStatus(status) {}

  std::shared_ptr<UnSolicitedMessage> clone() {
    return std::make_shared<RilUnsolUiccSubsStatusChangedMessage>(mStatus);
  }

  bool getStatus() { return mStatus; }

  string dump() {
    return RilUnsolUiccSubsStatusChangedMessage::MESSAGE_NAME;
  }
 private:
  bool mStatus;
};
