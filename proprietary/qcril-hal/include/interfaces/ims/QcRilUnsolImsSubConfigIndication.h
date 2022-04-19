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
 * Unsol message to notify change in sub config
 *
 */
class QcRilUnsolImsSubConfigIndication : public UnSolicitedMessage,
                                     public add_message_id<QcRilUnsolImsSubConfigIndication> {
 private:
  std::shared_ptr<qcril::interfaces::ImsSubConfigInfo> mSubConfig;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolImsSubConfigIndication";
  ~QcRilUnsolImsSubConfigIndication() { mSubConfig = nullptr; }

  QcRilUnsolImsSubConfigIndication(std::shared_ptr<qcril::interfaces::ImsSubConfigInfo> config)
      : UnSolicitedMessage(get_class_message_id()), mSubConfig(config) {}

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolImsSubConfigIndication> msg =
        std::make_shared<QcRilUnsolImsSubConfigIndication>(mSubConfig);
    return msg;
  }

  std::shared_ptr<qcril::interfaces::ImsSubConfigInfo> getImsSubConfig() { return mSubConfig; };

  void setImsSubConfig(std::shared_ptr<qcril::interfaces::ImsSubConfigInfo> config) {
    mSubConfig = config;
  }

  string dump() {
    return QcRilUnsolImsSubConfigIndication::MESSAGE_NAME;
  }
};
