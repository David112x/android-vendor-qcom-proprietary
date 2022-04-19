/******************************************************************************
#  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "RadioConfigImpl.h"
#include <framework/Module.h>
#include <framework/QcrilInitMessage.h>
#include "modules/uim/UimSlotStatusInd.h"

class RadioConfigModule : public Module
{
  public:
    RadioConfigModule();
    ~RadioConfigModule();
    void init();
    sp<RadioConfigImpl> getRadioConfigImpl();
  protected:
    void handleQcrilInitMessage(std::shared_ptr<QcrilInitMessage> msg);
  private:
    void handleSlotStatusIndiaction(std::shared_ptr<UimSlotStatusInd> msg);
};
