/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include <framework/MessageContext.h>
#include <framework/SolicitedMessage.h>
#include <interfaces/common.h>
#include <telephony/ril.h>

struct QcRilRequestMessageCallbackPayload : public generic_callback_payload {
  RIL_Errno errorCode;
  std::shared_ptr<qcril::interfaces::BasePayload> data;

public:
  QcRilRequestMessageCallbackPayload(
      RIL_Errno e, std::shared_ptr<qcril::interfaces::BasePayload> d) {
    errorCode = e;
    data = d;
  }
  ~QcRilRequestMessageCallbackPayload() { data = nullptr; }
};

enum class PendingMessageState {
  FREE,                /* Request is not waiting for callback or indications */
  AWAITING_CALLBACK,   /* Request is waiting for callback */
  AWAITING_INDICATION, /* Request is waiting for indication */
  COMPLETED_SUCCESS,   /* Request completed, success */
  COMPLETED_FAILURE    /* Request completed, failure */
};

class QcRilRequestMessage : public SolicitedMessage<QcRilRequestMessageCallbackPayload> {
 private:
  PendingMessageState mPendingState;
  bool mIsImsRequest = false;

 public:
  QcRilRequestMessage() = delete;
  ~QcRilRequestMessage() {}

  inline QcRilRequestMessage(message_id_t msg_id, std::shared_ptr<MessageContext> context)
      : SolicitedMessage<QcRilRequestMessageCallbackPayload>(context, msg_id),
        mPendingState(PendingMessageState::FREE) {}

  virtual string dump() {
    std::string dumpStr = mName;
    auto ctx = getContext();
    if (ctx != nullptr) {
      dumpStr = dumpStr + "[Context: " + ctx->toString() + "]";
    } else {
      dumpStr = dumpStr + "[Context: null ]";
    }
    return dumpStr;
  }

  PendingMessageState getPendingMessageState() { return mPendingState; }

  void setPendingMessageState(PendingMessageState state) { mPendingState = state; }

  // This is for identifying the request is received on IMS interface (eg: for Supplementary Service
  // requests).
  bool isImsRequest() { return mIsImsRequest; }
  void setIsImsRequest(bool val) {mIsImsRequest = val; }
};

class ScheduleCallbackData {
 public:
    explicit ScheduleCallbackData(std::shared_ptr<QcRilRequestMessage> data) {
        mData = data;
    }
    inline std::shared_ptr<QcRilRequestMessage> getData() { return mData; }
 private:
    std::shared_ptr<QcRilRequestMessage> mData;
};
