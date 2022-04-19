/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>
#include <interfaces/ims/ims.h>

/*
 * Unsol message to notify modem support for Wfc roaming config
 *
 */
class QcRilUnsolImsWfcRoamingConfigIndication : public UnSolicitedMessage,
                                     public add_message_id<QcRilUnsolImsWfcRoamingConfigIndication> {
 private:
  bool mWfcRoamingConfigSupport;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolImsWfcRoamingConfigIndication";
  ~QcRilUnsolImsWfcRoamingConfigIndication() { }

  QcRilUnsolImsWfcRoamingConfigIndication(bool status)
      : UnSolicitedMessage(get_class_message_id()), mWfcRoamingConfigSupport(status) {}

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolImsWfcRoamingConfigIndication> msg =
        std::make_shared<QcRilUnsolImsWfcRoamingConfigIndication>(mWfcRoamingConfigSupport);
    return msg;
  }

  bool getWfcRoamingConfigSupport() { return mWfcRoamingConfigSupport; };

  string dump() {
    return QcRilUnsolImsWfcRoamingConfigIndication::MESSAGE_NAME;
  }
};
