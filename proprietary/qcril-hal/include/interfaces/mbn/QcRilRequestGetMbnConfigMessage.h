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
 * Request to query mbn config ID
 * @Receiver: MbnModule
 *
 * Response:
 *   errorCode    : Valid error codes
 *                   SUCCESS
 *                   GENERIC_FAILURE
 *   responseData : std::shared_ptr<qcril::interfaces::MbnConfigResp>
 */
class QcRilRequestGetMbnConfigMessage : public QcRilRequestMessage,
                                        public add_message_id<QcRilRequestGetMbnConfigMessage>
{
  public:
    static constexpr const char *MESSAGE_NAME = "QCRIL_EVT_HOOK_GET_MBN_CONFIG";

    QcRilRequestGetMbnConfigMessage() = delete;
    ~QcRilRequestGetMbnConfigMessage() {};

    explicit QcRilRequestGetMbnConfigMessage(std::shared_ptr<MessageContext> context,
                    uint32_t subId, qcril::interfaces::qcril_pdc_mbn_type mbnType):
            QcRilRequestMessage(get_class_message_id(), context), mSubId(subId), mMbnType(mbnType) {
      mName = MESSAGE_NAME;
    }

    uint32_t getSubId() {
      return mSubId;
    }

    qcril::interfaces::qcril_pdc_mbn_type getMbnType() {
      return mMbnType;
    }

    virtual string dump() {
      return QcRilRequestMessage::dump() +
            "{" +  "mbn type = " +  std::to_string(mMbnType) + "}";
    }

  private:
    uint32_t mSubId;
    qcril::interfaces::qcril_pdc_mbn_type mMbnType;
};
