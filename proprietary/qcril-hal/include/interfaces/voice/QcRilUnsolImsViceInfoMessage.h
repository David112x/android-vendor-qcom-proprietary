/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>
#include <interfaces/voice/voice.h>
#include <optional>

/*
 * Unsol message to notify clients about a VoLTE over Internet Connected Endpoint (VICE)
 * dialog event
 */
class QcRilUnsolImsViceInfoMessage : public UnSolicitedMessage,
                                     public add_message_id<QcRilUnsolImsViceInfoMessage> {
 private:
  std::optional<std::vector<uint8_t>> mViceInfoUri;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolImsViceInfoMessage";
  ~QcRilUnsolImsViceInfoMessage() {}

  QcRilUnsolImsViceInfoMessage() : UnSolicitedMessage(get_class_message_id()) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolImsViceInfoMessage> msg =
        std::make_shared<QcRilUnsolImsViceInfoMessage>();
    if (msg) {
      if (mViceInfoUri) {
        msg->setViceInfoUri(*mViceInfoUri);
      }
    }
    return msg;
  }

  bool hasViceInfoUri() { return mViceInfoUri ? true : false; }
  std::vector<uint8_t> getViceInfoUri() { return *mViceInfoUri; }
  void setViceInfoUri(std::vector<uint8_t> val) { mViceInfoUri = val; }
#if 0
  inline std::string dumpViceInfoUri() {
    return " mViceInfoUri = " + (mViceInfoUri ? *mViceInfoUri : "<invalid>");
  }
#endif

  string dump() { return mName + "{}"; }
};
