/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#pragma once
#include "framework/UnSolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"

class QcRilUnsolAdnRecordsOnSimMessage : public UnSolicitedMessage,
                public add_message_id<QcRilUnsolAdnRecordsOnSimMessage> {
 private:
  qcril::interfaces::AdnRecords mAdnRecords;

 public:
  static constexpr const char *MESSAGE_NAME =
        "com.qualcomm.qti.qcril.pbm.adn_records_on_sim";
  QcRilUnsolAdnRecordsOnSimMessage() = delete;

  ~QcRilUnsolAdnRecordsOnSimMessage() {}

  explicit inline QcRilUnsolAdnRecordsOnSimMessage(const qcril::interfaces::AdnRecords& adnRecords)
      : UnSolicitedMessage(get_class_message_id()) {
    mName = MESSAGE_NAME;
    mAdnRecords = adnRecords;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    return std::make_shared<QcRilUnsolAdnRecordsOnSimMessage>(mAdnRecords);
  }

  virtual string dump() {
    return mName;
  }

  qcril::interfaces::AdnRecords* getAdnRecords() {
    return &mAdnRecords;
  }
};
