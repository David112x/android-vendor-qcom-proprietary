/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/add_message_id.h>
#include <interfaces/QcRilRequestMessage.h>
#include <optional>
#include "interfaces/ims/ims.h"

/*
 * Request to set the ims service status to lower layers
 *
 * Response:
 *   errorCode    : Valid error codes
 *                   SUCCESS
 *                   GENERIC_FAILURE
 *   responseData : none
 */

class QcRilRequestImsSetServiceStatusMessage
    : public QcRilRequestMessage,
      public add_message_id<QcRilRequestImsSetServiceStatusMessage> {
 private:
  std::optional<bool> mVolteEnabled;
  std::optional<bool> mVideoTelephonyEnabled;
  std::optional<bool> mWifiCallingEnabled;
  std::optional<bool> mUTEnabled;

 public:
  static constexpr const char* MESSAGE_NAME = "QcRilRequestImsSetServiceStatusMessage";

  QcRilRequestImsSetServiceStatusMessage() = delete;

  ~QcRilRequestImsSetServiceStatusMessage(){};

  inline QcRilRequestImsSetServiceStatusMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  bool hasVolteEnabled() {
    return mVolteEnabled ? true : false;
  }
  bool getVolteEnabled() {
    return *mVolteEnabled;
  }
  void setVolteEnabled(bool val) {
    mVolteEnabled = val;
  }
  void resetVolteEnabled() {
    mVolteEnabled.reset();
  }
  string dumpVolteEnabled() {
    return std::string(" mVolteEnabled = ") +
           (mVolteEnabled ? (*mVolteEnabled ? "true" : "false") : "<invalid>");
  }

  bool hasVideoTelephonyEnabled() {
    return mVideoTelephonyEnabled ? true : false;
  }
  bool getVideoTelephonyEnabled() {
    return *mVideoTelephonyEnabled;
  }
  void setVideoTelephonyEnabled(bool val) {
    mVideoTelephonyEnabled = val;
  }
  void resetVideoTelephonyEnabled() {
    mVideoTelephonyEnabled.reset();
  }
  string dumpVideoTelephonyEnabled() {
    return std::string(" mVideoTelephonyEnabled = ") +
           (mVideoTelephonyEnabled ? (*mVideoTelephonyEnabled ? "true" : "false") : "<invalid>");
  }

  bool hasWifiCallingEnabled() {
    return mWifiCallingEnabled ? true : false;
  }
  bool getWifiCallingEnabled() {
    return *mWifiCallingEnabled;
  }
  void setWifiCallingEnabled(bool val) {
    mWifiCallingEnabled = val;
  }
  void resetWifiCallingEnabled() {
    mWifiCallingEnabled.reset();
  }
  string dumpWifiCallingEnabled() {
    return std::string(" mWifiCallingEnabled = ") +
           (mWifiCallingEnabled ? (*mWifiCallingEnabled ? "true" : "false") : "<invalid>");
  }

  bool hasUTEnabled() {
    return mUTEnabled ? true : false;
  }
  bool getUTEnabled() {
    return *mUTEnabled;
  }
  void setUTEnabled(bool val) {
    mUTEnabled = val;
  }
  void resetUTEnabled() {
    mUTEnabled.reset();
  }
  string dumpUTEnabled() {
    return std::string(" mUTEnabled = ") +
           (mUTEnabled ? (*mUTEnabled ? "true" : "false") : "<invalid>");
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() + "{" + dumpVolteEnabled() + dumpVideoTelephonyEnabled() +
           dumpWifiCallingEnabled() + dumpUTEnabled() + "}";
  }
};
