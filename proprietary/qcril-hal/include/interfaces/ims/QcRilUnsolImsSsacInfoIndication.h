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
class QcRilUnsolImsSsacInfoIndication : public UnSolicitedMessage,
                                     public add_message_id<QcRilUnsolImsSsacInfoIndication> {
 private:
  std::shared_ptr<qcril::interfaces::SsacInfo> mSsacInfo;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolImsSsacInfoIndication";
  ~QcRilUnsolImsSsacInfoIndication() { mSsacInfo = nullptr; }

  QcRilUnsolImsSsacInfoIndication(std::shared_ptr<qcril::interfaces::SsacInfo> info)
      : UnSolicitedMessage(get_class_message_id()), mSsacInfo(info) {}

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolImsSsacInfoIndication> msg =
        std::make_shared<QcRilUnsolImsSsacInfoIndication>(mSsacInfo);
    return msg;
  }

  std::shared_ptr<qcril::interfaces::SsacInfo> getServiceStatus() { return mSsacInfo; };

  void setServiceStatus(std::shared_ptr<qcril::interfaces::SsacInfo> info) {
    mSsacInfo = info;
  }

  string dump() {
    return QcRilUnsolImsSsacInfoIndication::MESSAGE_NAME;
  }
};
