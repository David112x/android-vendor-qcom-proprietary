/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>
#include <interfaces/nas/nas_types.h>

/*
 * Unsol message to report network scan result
 *
 */
class RilUnsolEmergencyListIndMessage : public UnSolicitedMessage,
            public add_message_id<RilUnsolEmergencyListIndMessage> {
 private:
     qcril::interfaces::RIL_EccList mEccList;

 public:
  static constexpr const char *MESSAGE_NAME = "RIL_UNSOL_EMERGENCY_INFO_LIST";
  ~RilUnsolEmergencyListIndMessage() { }


  template<typename T>
  RilUnsolEmergencyListIndMessage(T info)
      : UnSolicitedMessage(get_class_message_id()),
      mEccList(std::forward<T>(info)) { }

  std::shared_ptr<UnSolicitedMessage> clone() {
    return std::make_shared<RilUnsolEmergencyListIndMessage>(mEccList);
  }

  const qcril::interfaces::RIL_EccList& getEmergencyList() { return mEccList; }

  string dump() {
    return RilUnsolEmergencyListIndMessage::MESSAGE_NAME;
  }
};
