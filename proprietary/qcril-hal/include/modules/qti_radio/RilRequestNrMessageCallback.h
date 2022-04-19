/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/GenericCallback.h"
#include "framework/message_translator.h"
#include "framework/Message.h"
#include "MessageCommon.h"
#include <telephony/ril.h>

class QtiRadioGetDataNrIconTypeMessageCb : public GenericCallback<rildata:: NrIconType_t> {
  private:
    RIL_Token nrIconToken;

  public:
    inline QtiRadioGetDataNrIconTypeMessageCb(RIL_Token clientToken) :
          nrIconToken(clientToken){};

    inline ~QtiRadioGetDataNrIconTypeMessageCb() {};

    void onResponse(std::shared_ptr<Message> solicitedMsg, Status status,
        std::shared_ptr<rildata:: NrIconType_t> responseDataPtr) override;

    Message::Callback *clone() override;

    RIL_Token getNrIconToken() {
        return nrIconToken;
    }
};