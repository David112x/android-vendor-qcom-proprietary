/******************************************************************************
#  Copyright (c) 2017-2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include "framework/TimeKeeper.h"
#include <framework/message_id.h>

class Module;
using std::string;

class Message: public std::enable_shared_from_this<Message> {
 friend class Dispatcher;
 private:
   message_id_t m_h;
 public:
  class Callback {
   public:
    enum class Status { FAILURE, SUCCESS, CANCELLED, NO_HANDLER_FOUND, TIMEOUT, BROADCAST_NOT_ALLOWED };
    using cb_t = std::function<void(std::shared_ptr<Message>, Status, std::shared_ptr<void>)>;
   private:
    cb_t theCallback;
   public:
    string mClientToken;

    inline Callback(string token) : mClientToken{token} {}
    inline Callback(cb_t f) : theCallback(f), mClientToken{""} {}
    inline Callback &operator=(cb_t f) { theCallback = f; return *this; }
    virtual ~Callback();
    inline virtual Callback *clone() {
        if (theCallback != nullptr) {
            return new Callback(theCallback);
        } else {
            return nullptr;
        }
    }
    virtual void handleAsyncResponse(std::shared_ptr<Message> solicitedMsg,
                                     Status status,
                                     std::shared_ptr<void> responseDataPtr);
  };

  enum class ExecutionStatus {
    NONE,
    CANCELLATION_INPROGRESS,
    CANCELLED,
    HANDLER_INPROGRESS,
    HANDLER_EXECUTED,
    CALLBACK_INPROGRESS,
    CALLBACK_EXECUTED
  };

  // Message(string str, size_t dataSize, void* dataPtr, Message::Callback*
  // callback) {};
  virtual ~Message() = 0;

  void informDispatchFailure(std::shared_ptr<Message> solicitedMsg,
                    Message::Callback::Status status);

  virtual string dump() = 0;

  string to_string();
  string get_message_name() {
      string msg_name{"<Unknown>"};

      if (m_h) {
          msg_name = m_h->get_name();
      }
      return msg_name;
  }

  /* Note: No copy allowed to avoid slicing.*/
  Message(const Message &) = delete;
  Message &operator=(const Message &) = delete;

  /* Cancelled before module could execute the callback*/
  void cancelling();
  void cancelled();
  /* Module started handler execution*/
  void handlerExecuting();
  void handlerExecuted();
  /* Module finished callback execution finished*/
  void callbackExecuting();
  void callbackExecuted();


  bool isExpired();
  bool isHandlerExecuting();
  bool isHandlerExecuted();
  bool isCallbackExecuting();
  bool isCallbackExecuted();

  Message::ExecutionStatus getExecutionStatus();

  void setTimerId(TimeKeeper::timer_id tid);

  TimeKeeper::timer_id getTimerId();

  void setMessageExpiryTimer(TimeKeeper::millisec timeOut);
  TimeKeeper::millisec getMessageExpiryTimer();

  Message() = delete;
  enum class MessageType {
    None,
    SolicitedMessage,
    SolicitedSyncMessage,
    UnSolicitedMessage,
  };

  virtual MessageType getMessageType() = 0;
  virtual bool isSameMessage(std::shared_ptr<Message> msg);

  inline bool isCallbackPresent() {
      if(mCallback) return true;
      else return false;
  }

  message_id_t get_message_id()
  {
      return m_h;
  }

  void set_next_hop(Module* module)
  {
    m_next_hop = module;
  }

  Module* get_next_hop()
  {
    return m_next_hop;
  }

 protected:
   Message(message_id_t id,
           TimeKeeper::millisec timeout = TimeKeeper::INVALID_MESSAGE_EXPIRY_TIMEOUT);
   Message(const char *name, message_id_t id,
           TimeKeeper::millisec timeout = TimeKeeper::INVALID_MESSAGE_EXPIRY_TIMEOUT);

   inline void invokeCallback(std::shared_ptr<Message> solicitedMsg, Message::Callback::Status status,
     std::shared_ptr<void> responseData) {
     if (mCallback != nullptr) {
       mCallback->handleAsyncResponse(solicitedMsg, status, responseData);
     }
   }

  string mName;
  Message::Callback *mCallback;
  ExecutionStatus mExecutionStatus{ExecutionStatus::NONE};
  TimeKeeper::timer_id mTid{TimeKeeper::no_timer};
  qtimutex::QtiRecursiveMutex mMutex;
  Module* m_next_hop;
  TimeKeeper::millisec mExpiryTimer{0};
};

inline Message::Message(const char *name, message_id_t h, TimeKeeper::millisec timeout):
        m_h(h), mName(name), mCallback(nullptr), m_next_hop(nullptr), mExpiryTimer(timeout)
  {}

inline Message::Message(message_id_t h, TimeKeeper::millisec timeout):
        Message("<Unknown>", h, timeout)
  {}

inline string Message::to_string() { return get_message_name(); }
