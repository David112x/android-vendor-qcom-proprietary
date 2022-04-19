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
 * Request to select mbn config ID
 * @Receiver: MbnModule
 *
 * Response:
 *   errorCode    : Valid error codes
 *                   SUCCESS
 *                   GENERIC_FAILURE
 *   responseData : nullptr
 */
class QcRilRequestSelectMbnConfigMessage : public QcRilRequestMessage,
                                        public add_message_id<QcRilRequestSelectMbnConfigMessage>
{
  public:
    static constexpr const char *MESSAGE_NAME = "QCRIL_EVT_HOOK_SELECT_MBN_CONFIG";

    QcRilRequestSelectMbnConfigMessage() = delete;
    ~QcRilRequestSelectMbnConfigMessage() {};

    explicit QcRilRequestSelectMbnConfigMessage(std::shared_ptr<MessageContext> context,
            uint32_t subId, qcril::interfaces::qcril_pdc_mbn_type mbnType, std::vector<uint8_t> configId):
            QcRilRequestMessage(get_class_message_id(), context),
            mSubId(subId), mMbnType(mbnType), mConfigId(configId) {
      mName = MESSAGE_NAME;
    }

    uint32_t getSubId() {
      return mSubId;
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
            ", config id length = " + std::to_string(mConfigId.size())  + "}";
    }

  private:
    uint32_t mSubId;
    qcril::interfaces::qcril_pdc_mbn_type mMbnType;
    std::vector<uint8_t> mConfigId;
};
