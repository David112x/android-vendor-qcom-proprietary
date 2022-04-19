/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>
#include <interfaces/ims/ims.h>

/*
 * Unsol message to notify ecbm
 *
 */
class QcRilUnsolImsExitEcbmIndication : public UnSolicitedMessage,
                                     public add_message_id<QcRilUnsolImsExitEcbmIndication> {

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolImsExitEcbmIndication";
  ~QcRilUnsolImsExitEcbmIndication() { }

  QcRilUnsolImsExitEcbmIndication()
      : UnSolicitedMessage(get_class_message_id()) {}

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolImsExitEcbmIndication> msg =
        std::make_shared<QcRilUnsolImsExitEcbmIndication>();
    return msg;
  }

  string dump() {
    return QcRilUnsolImsExitEcbmIndication::MESSAGE_NAME;
  }
};
