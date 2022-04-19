/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <interfaces/QcRilRequestMessage.h>
#include "framework/add_message_id.h"

/*
 * Request to validate mbn config
 * @Receiver: MbnModule
 *
 * Response:
 *   errorCode    : Valid error codes
 *                   SUCCESS
 *                   GENERIC_FAILURE
 *   responseData : nullptr
 */
class QcRilRequestValidateMbnConfigMessage : public QcRilRequestMessage,
                                        public add_message_id<QcRilRequestValidateMbnConfigMessage>
{
  public:
    static constexpr const char *MESSAGE_NAME = "QCRIL_EVT_HOOK_VALIDATE_MBN_CONFIG";

    QcRilRequestValidateMbnConfigMessage() = delete;
    ~QcRilRequestValidateMbnConfigMessage() {};

    explicit QcRilRequestValidateMbnConfigMessage(
        std::shared_ptr<MessageContext> context, uint32_t subId, std::vector<uint8_t> configId):
        QcRilRequestMessage(get_class_message_id(), context), mSubId(subId), mConfigId(configId) {
      mName = MESSAGE_NAME;
    }

    uint32_t getSubId() {
      return mSubId;
    }

    std::vector<uint8_t> getConfigId(){
      return mConfigId;
    }

    virtual string dump() {
      return QcRilRequestMessage::dump() +
            "{" + "config id length = " + std::to_string(mConfigId.size()) + "}";
    }

  private:
    uint32_t mSubId;
    std::vector<uint8_t> mConfigId;
};
