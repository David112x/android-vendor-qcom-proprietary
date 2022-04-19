/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <interfaces/QcRilRequestMessage.h>
#include "framework/add_message_id.h"

/*
 * Request to cleanup all loaded mbn configs
 * @Receiver: MbnModule
 *
 * Response:
 *   errorCode    : Valid error codes
 *                   SUCCESS
 *                   GENERIC_FAILURE
 *   responseData : nullptr
 */
class QcRilRequestCleanupMbnConfigMessage : public QcRilRequestMessage,
                                        public add_message_id<QcRilRequestCleanupMbnConfigMessage>
{
  public:
    static constexpr const char *MESSAGE_NAME = "QCRIL_EVT_HOOK_CLEANUP_MBN_CONFIG";

    ~QcRilRequestCleanupMbnConfigMessage() {};

    inline QcRilRequestCleanupMbnConfigMessage(std::shared_ptr<MessageContext> context):
            QcRilRequestMessage(get_class_message_id(), context) {
      mName = MESSAGE_NAME;
    }
};
