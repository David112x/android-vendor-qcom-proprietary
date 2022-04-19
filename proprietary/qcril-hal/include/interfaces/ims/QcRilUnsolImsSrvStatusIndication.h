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
class QcRilUnsolImsSrvStatusIndication : public UnSolicitedMessage,
                                     public add_message_id<QcRilUnsolImsSrvStatusIndication> {
 private:
  std::shared_ptr<qcril::interfaces::ServiceStatusInfoList> mServiceStatus;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolImsSrvStatusIndication";
  ~QcRilUnsolImsSrvStatusIndication() { mServiceStatus = nullptr; }

  QcRilUnsolImsSrvStatusIndication(std::shared_ptr<qcril::interfaces::ServiceStatusInfoList> info)
      : UnSolicitedMessage(get_class_message_id()), mServiceStatus(info) {}

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolImsSrvStatusIndication> msg =
        std::make_shared<QcRilUnsolImsSrvStatusIndication>(mServiceStatus);
    return msg;
  }

  std::shared_ptr<qcril::interfaces::ServiceStatusInfoList> getServiceStatus() { return mServiceStatus; };

  void setServiceStatus(std::shared_ptr<qcril::interfaces::ServiceStatusInfoList> info) {
    mServiceStatus = info;
  }

  string dump() {
    return QcRilUnsolImsSrvStatusIndication::MESSAGE_NAME;
  }
};
