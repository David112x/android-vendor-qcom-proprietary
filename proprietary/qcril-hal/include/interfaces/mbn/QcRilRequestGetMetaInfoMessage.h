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
 * Request to query mbn config info
 * @Receiver: MbnModule
 *
 * Response:
 *   errorCode    : Valid error codes
 *                   SUCCESS
 *                   GENERIC_FAILURE
 *   responseData : std::shared_ptr<qcril::interfaces::MbnConfigResp>
 */
class QcRilRequestGetMetaInfoMessage : public QcRilRequestMessage,
                                        public add_message_id<QcRilRequestGetMetaInfoMessage>
{
  public:
    static constexpr const char *MESSAGE_NAME = "QCRIL_EVT_HOOK_GET_META_INFO";

    QcRilRequestGetMetaInfoMessage() = delete;
    ~QcRilRequestGetMetaInfoMessage() {};

    explicit QcRilRequestGetMetaInfoMessage(std::shared_ptr<MessageContext> context,
            qcril::interfaces::qcril_pdc_mbn_type mbnType, std::vector<uint8_t> configId):
            QcRilRequestMessage(get_class_message_id(), context),
            mMbnType(mbnType), mConfigId(configId) {
      mName = MESSAGE_NAME;
    }

    qcril::interfaces::qcril_pdc_mbn_type getMbnType() {
      return mMbnType;
    }

    std::vector<uint8_t> getConfigId(){
      return mConfigId;
    }

    virtual string dump() {
      return QcRilRequestMessage::dump() +
            "{" +  "mbn type = " +  std::to_string(mMbnType) +
            ", config id length = " + std::to_string(mConfigId.size()) + "}";
    }

  private:
    qcril::interfaces::qcril_pdc_mbn_type mMbnType;
    std::vector<uint8_t> mConfigId;
};
