/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/add_message_id.h>
#include <interfaces/QcRilRequestMessage.h>
#include <interfaces/voice/voice.h>
#include <optional>

/*
 * Sends a simple Flash
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : nullptr
 */
class QcRilRequestCdmaFlashMessage : public QcRilRequestMessage,
                                     public add_message_id<QcRilRequestCdmaFlashMessage> {
 private:
  std::optional<std::string> mFeatureCode;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestCdmaFlashMessage";

  QcRilRequestCdmaFlashMessage() = delete;

  ~QcRilRequestCdmaFlashMessage() {}

  inline QcRilRequestCdmaFlashMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  bool hasFeatureCode() { return mFeatureCode ? true : false; }
  std::string getFeatureCode() { return mFeatureCode ? *mFeatureCode : ""; }
  void setFeatureCode(std::string val) { mFeatureCode = val; }
  inline string dumpFeatureCode() {
    return "mFeatureCode = " + (mFeatureCode ? (*mFeatureCode) : "<invalid>");
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() +
      "{" + dumpFeatureCode() + "}";
  }
};
