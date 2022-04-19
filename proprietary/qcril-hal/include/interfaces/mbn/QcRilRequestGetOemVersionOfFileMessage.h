/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <interfaces/QcRilRequestMessage.h>
#include "framework/add_message_id.h"

/*
 * Request to get OEM version of file
 * @Receiver: MbnModule
 *
 * Response:
 *   errorCode    : Valid error codes
 *                   SUCCESS
 *                   GENERIC_FAILURE
 *   responseData : std::shared_ptr<qcril::interfaces::FileVersionResp>
 */
class QcRilRequestGetOemVersionOfFileMessage : public QcRilRequestMessage,
                                        public add_message_id<QcRilRequestGetOemVersionOfFileMessage>
{
  public:
    static constexpr const char *MESSAGE_NAME = "QCRIL_EVT_HOOK_GET_OEM_VERSION_OF_FILE";

    QcRilRequestGetOemVersionOfFileMessage() = delete;
    ~QcRilRequestGetOemVersionOfFileMessage() {};

    explicit QcRilRequestGetOemVersionOfFileMessage(
            std::shared_ptr<MessageContext> context, const char* mbnFilePath):
            QcRilRequestMessage(get_class_message_id(), context), mFilePath(mbnFilePath) {
      mName = MESSAGE_NAME;
    }

    std::string getFilePath(){
      return mFilePath;
    }

    virtual string dump() {
      return QcRilRequestMessage::dump() +
            "{" + "file path = " + mFilePath + "}";
    }

  private:
    std::string mFilePath;
};
