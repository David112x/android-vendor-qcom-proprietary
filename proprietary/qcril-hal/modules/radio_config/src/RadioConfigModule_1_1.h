/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "RadioConfigModule.h"
#include "RadioConfigImpl_1_1.h"

class RadioConfigModule_1_1 : public RadioConfigModule
{
  public:
    RadioConfigModule_1_1();
    ~RadioConfigModule_1_1();
    void init();
  protected:
    void handleQcrilInitMessage(std::shared_ptr<QcrilInitMessage> msg);
};
