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
 * Sends a burst Dual-Tone Multifrequency (DTMF)
 *
 * Response:
 *   errorCode    : Valid error codes
 *   responseData : nullptr
 */
class QcRilRequestCdmaBurstDtmfMessage : public QcRilRequestMessage,
                                public add_message_id<QcRilRequestCdmaBurstDtmfMessage> {
 private:
  std::optional<std::string> mDigitBuffer;
  std::optional<uint32_t> mDtmfOnLength;
  std::optional<uint32_t> mDtmfOffLength;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilRequestCdmaBurstDtmfMessage";

  QcRilRequestCdmaBurstDtmfMessage() = delete;

  ~QcRilRequestCdmaBurstDtmfMessage() {}

  inline QcRilRequestCdmaBurstDtmfMessage(std::shared_ptr<MessageContext> context)
      : QcRilRequestMessage(get_class_message_id(), context) {
    mName = MESSAGE_NAME;
  }

  bool hasDigitBuffer() { return mDigitBuffer ? true : false; }
  std::string getDigitBuffer() { return mDigitBuffer ? *mDigitBuffer : ""; }
  void setDigitBuffer(std::string val) { mDigitBuffer = val; }
  inline string dumpDigitBuffer() {
    return "mDigitBuffer = " + (mDigitBuffer ? *mDigitBuffer : "<invalid>");
  }

  bool hasDtmfOnLength() { return mDtmfOnLength ? true : false; }
  uint32_t getDtmfOnLength() { return *mDtmfOnLength; }
  void setDtmfOnLength(uint32_t val) { mDtmfOnLength = val; }
  inline string dumpDtmfOnLength() {
    return "mDtmfOnLength = " + (mDtmfOnLength ? std::to_string(*mDtmfOnLength) : "<invalid>");
  }

  bool hasDtmfOffLength() { return mDtmfOffLength ? true : false; }
  uint32_t getDtmfOffLength() { return *mDtmfOffLength; }
  void setDtmfOffLength(uint32_t val) { mDtmfOffLength = val; }
  inline string dumpDtmfOffLength() {
    return "mDtmfOffLength = " + (mDtmfOffLength ? std::to_string(*mDtmfOffLength) : "<invalid>");
  }

  virtual string dump() {
    return QcRilRequestMessage::dump() +
      "{" + dumpDigitBuffer() + dumpDtmfOnLength() + dumpDtmfOffLength() + "}";
  }
};
