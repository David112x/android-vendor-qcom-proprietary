/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <interfaces/QcRilRequestMessage.h>
#include "framework/add_message_id.h"

/*
 * Request to get QC version of config
 * @Receiver: MbnModule
 *
 * Response:
 *   errorCode    : Valid error codes
 *                   SUCCESS
 *                   GENERIC_FAILURE
 *   responseData : std::shared_ptr<qcril::interfaces::ConfigVersionResp>
 */
class QcRilRequestGetQcVersionOfConfigMessage : public QcRilRequestMessage,
                                        public add_message_id<QcRilRequestGetQcVersionOfConfigMessage>
{
  public:
    static constexpr const char *MESSAGE_NAME = "QCRIL_EVT_HOOK_GET_QC_VERSION_OF_CONFIG";

    QcRilRequestGetQcVersionOfConfigMessage() = delete;
    ~QcRilRequestGetQcVersionOfConfigMessage() {};

    explicit QcRilRequestGetQcVersionOfConfigMessage(
            std::shared_ptr<MessageContext> context, std::vector<uint8_t> configId):
            QcRilRequestMessage(get_class_message_id(), context), mConfigId(configId) {
      mName = MESSAGE_NAME;
    }

    std::vector<uint8_t> getConfigId(){
      return mConfigId;
    }

    virtual string dump() {
      return QcRilRequestMessage::dump() +
            "{" + "config id length = " + std::to_string(mConfigId.size()) + "}";
    }

  private:
    std::vector<uint8_t> mConfigId;
};
