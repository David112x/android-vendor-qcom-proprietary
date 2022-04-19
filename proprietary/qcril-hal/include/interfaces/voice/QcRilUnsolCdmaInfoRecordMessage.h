/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <framework/UnSolicitedMessage.h>
#include <framework/add_message_id.h>
#include <interfaces/voice/voice.h>
#include <optional>

/**
 * Unsol message to notify CDMA info records
 */
class QcRilUnsolCdmaInfoRecordMessage : public UnSolicitedMessage,
                                        public add_message_id<QcRilUnsolCdmaInfoRecordMessage> {
 private:
  std::optional<std::shared_ptr<RIL_CDMA_InformationRecords>> mCdmaInfoRecords;

 public:
  static constexpr const char *MESSAGE_NAME = "QcRilUnsolCdmaInfoRecordMessage";
  ~QcRilUnsolCdmaInfoRecordMessage() {}

  QcRilUnsolCdmaInfoRecordMessage() : UnSolicitedMessage(get_class_message_id()) {
    mName = MESSAGE_NAME;
  }

  std::shared_ptr<UnSolicitedMessage> clone() {
    std::shared_ptr<QcRilUnsolCdmaInfoRecordMessage> msg =
        std::make_shared<QcRilUnsolCdmaInfoRecordMessage>();
    return msg;
  }

  bool hasCdmaInfoRecords() { return mCdmaInfoRecords ? true : false; }
  std::shared_ptr<RIL_CDMA_InformationRecords> getCdmaInfoRecords() { return *mCdmaInfoRecords; }
  void setCdmaInfoRecords(std::shared_ptr<RIL_CDMA_InformationRecords> val) {
    mCdmaInfoRecords = val;
  }

  std::string dump() { return mName + "{}"; }
};
