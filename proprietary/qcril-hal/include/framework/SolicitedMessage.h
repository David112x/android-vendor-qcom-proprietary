/******************************************************************************
#  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <memory>
#include "framework/Dispatcher.h"
#include "framework/Message.h"
#include "framework/MessageContext.h"
#include "framework/GenericCallback.h"
#include "framework/Log.h"

template <typename R>
class SolicitedMessage : public Message {
  std::shared_ptr<MessageContext> ctx;
public:
  SolicitedMessage() = delete;
  SolicitedMessage(message_id_t id,
    TimeKeeper::millisec timeout = TimeKeeper::INVALID_MESSAGE_EXPIRY_TIMEOUT);
  SolicitedMessage(std::weak_ptr<MessageContext> ctx, message_id_t id,
    TimeKeeper::millisec timeout = TimeKeeper::INVALID_MESSAGE_EXPIRY_TIMEOUT);
  SolicitedMessage(const char *name, std::weak_ptr<MessageContext> ctx, message_id_t id,
    TimeKeeper::millisec timeout = TimeKeeper::INVALID_MESSAGE_EXPIRY_TIMEOUT);
  virtual ~SolicitedMessage();
  virtual std::shared_ptr<R> duplicateResponseData(std::shared_ptr<R> responseData);
  void dispatch();
  void sendResponse(std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status, std::shared_ptr<R> responseData);
  void setCallback(GenericCallback<R> *callback);
  std::shared_ptr<R> getResponseData();
  Message::MessageType getMessageType() override;
  std::shared_ptr<MessageContext> getContext();
  void setContext(std::shared_ptr<MessageContext> ctx) {
      this->ctx = ctx;
  }
private:
  std::shared_ptr<R> mRespData = nullptr;
};

template <typename R>
SolicitedMessage<R>::SolicitedMessage(message_id_t id, TimeKeeper::millisec timeout) :
  Message(id, timeout), ctx() {
}

template <typename R>
SolicitedMessage<R>::SolicitedMessage(std::weak_ptr<MessageContext> ctx,
        message_id_t id,
        TimeKeeper::millisec timeout) :
  Message(id, timeout), ctx(ctx) {
}

template <typename R>
SolicitedMessage<R>::SolicitedMessage(const char *name, std::weak_ptr<MessageContext> ctx,
        message_id_t id,
        TimeKeeper::millisec timeout) :
  Message(name, id, timeout), ctx(ctx) {
}

template <typename R>
SolicitedMessage<R>::~SolicitedMessage() {
  if (mCallback) {
    delete mCallback;
    mCallback = nullptr;
  }
}

template <typename R>
void SolicitedMessage<R>::dispatch() {
  std::shared_ptr<Message> shared_msgPtr = shared_from_this();
  Dispatcher::getInstance().dispatch(shared_msgPtr);
}

template <typename R>
std::shared_ptr<R> SolicitedMessage<R>::duplicateResponseData(std::shared_ptr<R> responseData) {
  return responseData;
}

template <typename R>
std::shared_ptr<R> SolicitedMessage<R>::getResponseData() {
  return mRespData;
}

template <typename R>
void SolicitedMessage<R>::sendResponse(std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
  std::shared_ptr<R> responseData) {

  if (!isExpired()) {
    mRespData = responseData;
    invokeCallback(solicitedMsg, status, responseData);
    Dispatcher::getInstance().informMessageCallbackFinished(solicitedMsg);
  }
  else {
    Log::getInstance().d(
      "[SolicitedAsyncMessage]: Callback is not invoked, msg is "
      "already cancelled = " +
      solicitedMsg->dump());
  }
}

template <typename R>
void SolicitedMessage<R>::setCallback(GenericCallback<R> *callback) {
  if (callback != nullptr) {
    mCallback = callback->clone();
  }
  else {
    mCallback = nullptr;
  }
}

template <typename R>
Message::MessageType SolicitedMessage<R>::getMessageType() {
  return (Message::MessageType::SolicitedMessage);
}

template <typename R>
std::shared_ptr<MessageContext> SolicitedMessage<R>::getContext() { return ctx; }
