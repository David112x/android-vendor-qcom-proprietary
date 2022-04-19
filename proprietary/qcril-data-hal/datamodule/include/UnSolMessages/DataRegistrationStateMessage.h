/**
* Copyright (c) 2018 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#pragma once
#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"
namespace rildata {

enum Rat {
  UNKNOWN = 0,
  NR_NSA = 20,
  NR_NSA_MMWV = 21,
  NR_SA = 22,
  NR_SA_MMWV = 23
};

/********************** Class Definitions *************************/
class DataRegistrationStateMessage: public UnSolicitedMessage,
                                    public add_message_id<DataRegistrationStateMessage> {
private:
  Rat mRatValue;
  int mInstanceId;

public:
  static constexpr const char *MESSAGE_NAME = "QCRIL_DATA_REGISTRATION_STATE";

  DataRegistrationStateMessage() = delete;
  ~DataRegistrationStateMessage();

  DataRegistrationStateMessage(int instance_id, const Rat setRatValue);

  std::shared_ptr<UnSolicitedMessage> clone();

  Rat getRatValue();

  string dump();

  int getInstanceId();
};

} //namespace
