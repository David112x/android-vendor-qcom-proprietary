/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <interfaces/QcRilRequestMessage.h>
#include "framework/add_message_id.h"
#include "interfaces/mbn/mbn.h"

/*
 * Request to enable modem update
 * @Receiver: MbnModule
 *
 * Response:
 *   errorCode    : Valid error codes
 *                   SUCCESS
 *                   GENERIC_FAILURE
 *   responseData : nullptr
 */
class QcRilRequestEnableModemUpdateMessage : public QcRilRequestMessage,
                                        public add_message_id<QcRilRequestEnableModemUpdateMessage>
{
  public:
    static constexpr const char *MESSAGE_NAME = "QCRIL_EVT_HOOK_ENABLE_MODEM_UPDATE";

    QcRilRequestEnableModemUpdateMessage() = delete;
    ~QcRilRequestEnableModemUpdateMessage() {};

    explicit QcRilRequestEnableModemUpdateMessage(std::shared_ptr<MessageContext> context,
            uint32_t subId):
            QcRilRequestMessage(get_class_message_id(), context), mSubId(subId) {
      mName = MESSAGE_NAME;
    }

    uint32_t getSubId() {
      return mSubId;
    }

    virtual string dump() {
      return QcRilRequestMessage::dump();
    }

  private:
    uint32_t mSubId;
};
