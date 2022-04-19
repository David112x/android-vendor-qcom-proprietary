/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>
#include <interfaces/voice/voice.h>
#include <optional>

/*
 * Unsol message to notify dtmf
 */
class QcRilUnsolDtmfMessage : public UnSolicitedMessage,
                              public add_message_id<QcRilUnsolDtmfMessage> {
 private:
  uint8_t mCallId;
  qcril::interfaces::DtmfEvent mDtmfEvent;
  std::string mDigitBuffer;
  std::optional<qcril::interfaces::DtmfOnLength> mOnLength;
  std::optional<qcril::interfaces::DtmfOffLength> mOffLength;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolDtmfMessage";
  ~QcRilUnsolDtmfMessage() {}

  QcRilUnsolDtmfMessage(uint8_t callId, qcril::interfaces::DtmfEvent dtmfEvent,
                        std::string digitBuffer)
      : UnSolicitedMessage(get_class_message_id()),
        mCallId(callId),
        mDtmfEvent(dtmfEvent),
        mDigitBuffer(digitBuffer) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolDtmfMessage> msg =
        std::make_shared<QcRilUnsolDtmfMessage>(mCallId, mDtmfEvent, mDigitBuffer);
    if (msg) {
    }
    return msg;
  }

  uint8_t getCallId() { return mCallId; }
  void setCallId(uint8_t val) { mCallId = val; }
  inline std::string dumpCallId() { return " mCallId = " + std::to_string(mCallId); }

  qcril::interfaces::DtmfEvent getDtmfEvent() { return mDtmfEvent; }
  void setDtmfEvent(qcril::interfaces::DtmfEvent val) { mDtmfEvent = val; }
  inline std::string dumpDtmfEvent() { return " mDtmfEvent = " + toString(mDtmfEvent); }

  std::string getDigitBuffer() { return mDigitBuffer; }
  void setDigitBuffer(std::string val) { mDigitBuffer = val; }
  inline std::string dumpDigitBuffer() { return " mDigitBuffer = " + (mDigitBuffer); }

  bool hasOnLength() { return mOnLength ? true : false; }
  qcril::interfaces::DtmfOnLength getOnLength() { return *mOnLength; }
  void setOnLength(qcril::interfaces::DtmfOnLength val) { mOnLength = val; }
  inline string dumpOnLength() {
    return " mOnLength = " + (mOnLength ? toString(*mOnLength) : "<invalid>");
  }

  bool hasOffLength() { return mOffLength ? true : false; }
  qcril::interfaces::DtmfOffLength getOffLength() { return *mOffLength; }
  void setOffLength(qcril::interfaces::DtmfOffLength val) { mOffLength = val; }
  inline string dumpOffLength() {
    return " mOffLength = " + (mOffLength ? toString(*mOffLength) : "<invalid>");
  }

  std::string dump() {
    return mName + "{" + dumpCallId() + dumpDtmfEvent() + dumpDigitBuffer() + dumpOnLength() +
           dumpOffLength() + "}";
  }
};
