/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <interfaces/QcRilRequestMessage.h>
#include "framework/add_message_id.h"

/*
 * Request to query ADN records message on SIM card
 * @Receiver: PbmModule
 *
 * Response:
 *   errorCode    : Valid error codes
 *                   SUCCESS
 *                   GENERIC_FAILURE
 *   responseData : std::shared_ptr<qcril::interfaces::AdnCountInfoResp>
 */
class QcRilRequestGetAdnRecordMessage : public QcRilRequestMessage,
                                        public add_message_id<QcRilRequestGetAdnRecordMessage>
{
  public:
    static constexpr const char *MESSAGE_NAME = "QCRIL_EVT_HOOK_GET_ADN_RECORD";

    ~QcRilRequestGetAdnRecordMessage() {}

    inline QcRilRequestGetAdnRecordMessage(std::shared_ptr<MessageContext> context):
            QcRilRequestMessage(get_class_message_id(), context) {
      mName = MESSAGE_NAME;
    }
};
