/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/GenericCallback.h"
#include "framework/SolicitedMessage.h"
#include "framework/Message.h"
#include "framework/add_message_id.h"
#include "framework/message_translator.h"
#include "framework/legacy.h"
#include "interfaces/QcRilRequestMessage.h"

class RilRequestGetPhoneCapabilityMessage : public SolicitedMessage<QcRilRequestMessageCallbackPayload>,
    public add_message_id<RilRequestGetPhoneCapabilityMessage> {

    public:
      static constexpr const char *MESSAGE_NAME = "com.qualcomm.qti.qcril.nas.GetPhoneCapability";
      ~RilRequestGetPhoneCapabilityMessage() {}

      RilRequestGetPhoneCapabilityMessage():
                          SolicitedMessage<QcRilRequestMessageCallbackPayload>(get_class_message_id())
      {
        mName = MESSAGE_NAME;
      }

      inline string dump() {
        return mName;
      }
};
