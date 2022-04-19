/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "framework/Module.h"
#include <framework/QcrilInitMessage.h>
#include "modules/uim/UimQmiUimIndicationMsg.h"

class SecureElementModule : public Module
{
  public:
    bool mReady;
    bool mServiceUp;
    bool card_accessible;
    bool mQmiUimUp;

    SecureElementModule();
    ~SecureElementModule();
    void init();

  private:
    void handleQcrilInitMessage(std::shared_ptr<QcrilInitMessage> msg);
    void handleQmiUimIndiaction(std::shared_ptr<UimQmiUimIndicationMsg> uim_qmi_ind_msg_ptr);
    void handleSecureElementOnStateChangeInd(std::shared_ptr<SecureElementOnStateChangeIndMsg> msg);
};
