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
class QcRilUnsolImsEnterEcbmIndication : public UnSolicitedMessage,
                                     public add_message_id<QcRilUnsolImsEnterEcbmIndication> {

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolImsEnterEcbmIndication";
  ~QcRilUnsolImsEnterEcbmIndication() { }

  QcRilUnsolImsEnterEcbmIndication()
      : UnSolicitedMessage(get_class_message_id()) {}

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolImsEnterEcbmIndication> msg =
        std::make_shared<QcRilUnsolImsEnterEcbmIndication>();
    return msg;
  }

  string dump() {
    return QcRilUnsolImsEnterEcbmIndication::MESSAGE_NAME;
  }
};
