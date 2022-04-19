/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <interfaces/QcRilRequestMessage.h>
#include "framework/add_message_id.h"

/*
 * Request to update ADN records message on SIM card
 * @Receiver: PbmModule
 *
 * Response:
 *   errorCode    : Valid error codes
 *                   SUCCESS
 *                   GENERIC_FAILURE
 *   responseData : std::shared_ptr<qcril::interfaces::AdnRecordUpdatedResp>
 */
class QcRilRequestUpdateAdnRecordMessage : public QcRilRequestMessage,
                                        public add_message_id<QcRilRequestUpdateAdnRecordMessage>
{
  public:
    static constexpr const char *MESSAGE_NAME = "QCRIL_EVT_HOOK_UPDATE_ADN_RECORD";

    QcRilRequestUpdateAdnRecordMessage() = delete;
    ~QcRilRequestUpdateAdnRecordMessage() {};

    explicit QcRilRequestUpdateAdnRecordMessage(
            std::shared_ptr<MessageContext> context, const qcril::interfaces::AdnRecordInfo & recordData):
            QcRilRequestMessage(get_class_message_id(), context) {
      mName = MESSAGE_NAME;
      mRecordData = recordData;
    }

    const qcril::interfaces::AdnRecordInfo& getRecordData() {
        return mRecordData;
    };

    virtual string dump() {
        return QcRilRequestMessage::dump() +
              "{" +  "record id = " +  std::to_string(mRecordData.record_id) +
              ", name = " + mRecordData.name + ", number = " + mRecordData.number + "}";
    }

  private:
    qcril::interfaces::AdnRecordInfo mRecordData;
};
