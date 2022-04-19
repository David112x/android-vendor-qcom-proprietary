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
 * Request to deactivate mbn Configs
 * @Receiver: MbnModule
 *
 * Response:
 *   errorCode    : Valid error codes
 *                   SUCCESS
 *                   GENERIC_FAILURE
 *   responseData : nullptr
 */
class QcRilRequestDeactivateMbnConfigMessage : public QcRilRequestMessage,
                                        public add_message_id<QcRilRequestDeactivateMbnConfigMessage>
{
  public:
    static constexpr const char *MESSAGE_NAME = "QCRIL_EVT_HOOK_DEACTIVATE_META_CONFIG";

    QcRilRequestDeactivateMbnConfigMessage() = delete;
    ~QcRilRequestDeactivateMbnConfigMessage() {};

    explicit QcRilRequestDeactivateMbnConfigMessage
            (std::shared_ptr<MessageContext> context, qcril::interfaces::qcril_pdc_mbn_type mbnType):
            QcRilRequestMessage(get_class_message_id(), context), mMbnType(mbnType) {
      mName = MESSAGE_NAME;
    }

    qcril::interfaces::qcril_pdc_mbn_type getMbnType() {
      return mMbnType;
    }

  private:
    qcril::interfaces::qcril_pdc_mbn_type mMbnType;
};
