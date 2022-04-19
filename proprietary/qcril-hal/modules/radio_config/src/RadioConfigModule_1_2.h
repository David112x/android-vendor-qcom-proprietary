/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "RadioConfigModule_1_1.h"
#include "RadioConfigImpl_1_2.h"

class RadioConfigModule_1_2 : public RadioConfigModule_1_1
{
  public:
    RadioConfigModule_1_2();
    ~RadioConfigModule_1_2();
    void init();
  protected:
    void handleQcrilInitMessage(std::shared_ptr<QcrilInitMessage> msg);
  private:
    void handleSlotStatusIndication_1_2(std::shared_ptr<UimSlotStatusInd> msg);
};
