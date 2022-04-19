/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <interfaces/QcRilRequestMessage.h>
#include "framework/add_message_id.h"

/*
 * Request to query mbn available config IDs
 * @Receiver: MbnModule
 *
 * Response:
 *   errorCode    : Valid error codes
 *                   SUCCESS
 *                   GENERIC_FAILURE
 *   responseData : nullptr
 */
class QcRilRequestGetAvlMbnConfigMessage : public QcRilRequestMessage,
                                        public add_message_id<QcRilRequestGetAvlMbnConfigMessage>
{
  public:
    static constexpr const char *MESSAGE_NAME = "QCRIL_EVT_HOOK_GET_AVAILABLE_MBN_CONFIG";

    ~QcRilRequestGetAvlMbnConfigMessage() {};

    inline QcRilRequestGetAvlMbnConfigMessage(std::shared_ptr<MessageContext> context):
            QcRilRequestMessage(get_class_message_id(), context) {
      mName = MESSAGE_NAME;
    }
};
