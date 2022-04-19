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
 * Unsol message to notify change in vowifi call quality
 *
 */
class QcRilUnsolImsVowifiCallQuality : public UnSolicitedMessage,
                                     public add_message_id<QcRilUnsolImsVowifiCallQuality> {
 private:
  qcril::interfaces::VowifiQuality mQuality;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolImsVowifiCallQuality";
  ~QcRilUnsolImsVowifiCallQuality() { }

  QcRilUnsolImsVowifiCallQuality(qcril::interfaces::VowifiQuality quality)
      : UnSolicitedMessage(get_class_message_id()), mQuality(quality) {}

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolImsVowifiCallQuality> msg =
        std::make_shared<QcRilUnsolImsVowifiCallQuality>(mQuality);
    return msg;
  }

  void setCallQuality(qcril::interfaces::VowifiQuality quality) {
    mQuality = quality;
  }

  qcril::interfaces::VowifiQuality getCallQuality() { return mQuality; };

  string dump() {
    return QcRilUnsolImsVowifiCallQuality::MESSAGE_NAME;
  }
};
