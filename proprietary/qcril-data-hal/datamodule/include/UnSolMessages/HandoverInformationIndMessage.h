/**
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
**/

#pragma once
#include "framework/Message.h"
#include "framework/UnSolicitedMessage.h"
#include "framework/add_message_id.h"
#include "MessageCommon.h"
#include "wireless_data_service_v01.h"

namespace rildata {

/********************** Class Definitions *************************/
class HandoverInformationIndMessage: public UnSolicitedMessage,
                           public add_message_id<HandoverInformationIndMessage> {
private:
  HandoffState_t state;
  IpFamilyType_t ipType;
  int cid;
  bool isFailReasonSet;
  wds_handoff_failure_reason_type_v01 failReason;

public:
  static constexpr const char *MESSAGE_NAME = "HandoverInformationIndMessage";

  HandoverInformationIndMessage(HandoffState_t setState, IpFamilyType_t setIpType, int cid);
  ~HandoverInformationIndMessage();


  HandoffState_t getState();
  IpFamilyType_t getIpType();
  int getCid();
  bool hasFailReason();
  void setFailReason(wds_handoff_failure_reason_type_v01 failReason);
  wds_handoff_failure_reason_type_v01 getFailReason();

  std::shared_ptr<UnSolicitedMessage> clone();

  string dump();
};

} //namespace