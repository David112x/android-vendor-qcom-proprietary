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
 * Unsol message to notify change in voiceinfo
 *
 */
class QcRilUnsolImsVoiceInfo : public UnSolicitedMessage,
                                     public add_message_id<QcRilUnsolImsVoiceInfo> {
 private:
  qcril::interfaces::VoiceInfo mVoiceInfo;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolImsVoiceInfo";
  ~QcRilUnsolImsVoiceInfo() { }

  QcRilUnsolImsVoiceInfo(qcril::interfaces::VoiceInfo status)
      : UnSolicitedMessage(get_class_message_id()), mVoiceInfo(status) {}

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolImsVoiceInfo> msg =
        std::make_shared<QcRilUnsolImsVoiceInfo>(mVoiceInfo);
    return msg;
  }

  qcril::interfaces::VoiceInfo getVoiceInfo() { return mVoiceInfo; };

  string dump() {
    return QcRilUnsolImsVoiceInfo::MESSAGE_NAME;
  }
};
