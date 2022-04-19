/*===========================================================================

  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#pragma once

#include "MessageCommon.h"

using namespace std;

namespace rildata {

class DeactivateDataCallRadioResponseIndMessage : public UnSolicitedMessage,
                          public add_message_id<DeactivateDataCallRadioResponseIndMessage> {
private:
  ResponseError_t response;
  int32_t serial;
  Message::Callback::Status status;

public:
  static constexpr const char *MESSAGE_NAME = "DeactivateDataCallRadioResponseIndMessage";

  DeactivateDataCallRadioResponseIndMessage() = delete;
  ~DeactivateDataCallRadioResponseIndMessage() = default;
  DeactivateDataCallRadioResponseIndMessage(ResponseError_t setResponse, int32_t setSerial, Message::Callback::Status setStatus):
    UnSolicitedMessage(get_class_message_id()), response(setResponse), serial(setSerial), status(setStatus) {}

  string dump(){return mName;}

  std::shared_ptr<UnSolicitedMessage> clone() {
    return std::make_shared<DeactivateDataCallRadioResponseIndMessage>(response, serial, status);
  };

  ResponseError_t getResponse() { return response; }
  int32_t getSerial() { return serial; }
  Message::Callback::Status getStatus() { return status; }
};

class DeactivateDataCallIWlanResponseIndMessage : public UnSolicitedMessage,
                          public add_message_id<DeactivateDataCallIWlanResponseIndMessage> {
private:
  ResponseError_t response;
  int32_t serial;
  Message::Callback::Status status;

public:
  static constexpr const char *MESSAGE_NAME = "DeactivateDataCallIWlanResponseIndMessage";

  DeactivateDataCallIWlanResponseIndMessage() = delete;
  ~DeactivateDataCallIWlanResponseIndMessage() = default;
  DeactivateDataCallIWlanResponseIndMessage(ResponseError_t setResponse, int32_t setSerial, Message::Callback::Status setStatus):
    UnSolicitedMessage(get_class_message_id()), response(setResponse), serial(setSerial), status(setStatus) {}

  string dump(){return mName;}

  std::shared_ptr<UnSolicitedMessage> clone() {
    return std::make_shared<DeactivateDataCallIWlanResponseIndMessage>(response, serial, status);
  };

  ResponseError_t getResponse() { return response; }
  int32_t getSerial() { return serial; }
  Message::Callback::Status getStatus() { return status; }
};

class DeactivateDataCallRequestMessage : public SolicitedMessage<ResponseError_t>,
                          public add_message_id<DeactivateDataCallRequestMessage> {
  private:
    int32_t mSerial;
    int32_t mCid;
    DataRequestReason_t mReason;
    shared_ptr<function<void(int32_t)>> mAcknowlegeRequestCb;

  public:
    static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.data.DeactivateDataCallRequestMessage";
    DeactivateDataCallRequestMessage() = delete;
    DeactivateDataCallRequestMessage(
      const int32_t serial,
      const int32_t cid,
      const DataRequestReason_t reason,
      const shared_ptr<function<void(int32_t)>> ackCb
      ):SolicitedMessage<ResponseError_t>(get_class_message_id()) {

      mName = MESSAGE_NAME;
      mSerial = serial;
      mCid = cid;
      mReason = reason;
      mAcknowlegeRequestCb = ackCb;
    }
    ~DeactivateDataCallRequestMessage() = default;

    string dump(){return mName;}
    int32_t getSerial() {return mSerial;}
    int32_t getCid() {return mCid;}
    DataRequestReason_t getDataRequestReason() {return mReason;}
    shared_ptr<function<void(int32_t)>> getAcknowlegeRequestCb() {return mAcknowlegeRequestCb;}
};

}
