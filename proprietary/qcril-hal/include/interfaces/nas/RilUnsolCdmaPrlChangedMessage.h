/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>

/*
 * Unsol message to notify CDMA prl change
 *
 */
class RilUnsolCdmaPrlChangedMessage : public UnSolicitedMessage,
            public add_message_id<RilUnsolCdmaPrlChangedMessage> {
 public:
  static constexpr const char *MESSAGE_NAME = "RIL_UNSOL_CDMA_PRL_CHANGED";
  ~RilUnsolCdmaPrlChangedMessage() { }

  RilUnsolCdmaPrlChangedMessage(int32_t prl)
      : UnSolicitedMessage(get_class_message_id()), mPrl(prl) {}

  std::shared_ptr<UnSolicitedMessage> clone() {
    return std::make_shared<RilUnsolCdmaPrlChangedMessage>(mPrl);
  }

  int32_t getPrl() { return mPrl; }

  string dump() {
    return RilUnsolCdmaPrlChangedMessage::MESSAGE_NAME;
  }
 private:
  int32_t mPrl;
};
