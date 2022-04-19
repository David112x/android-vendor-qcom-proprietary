/******************************************************************************

  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

#******************************************************************************/
#ifdef FEATURE_DATA_LQE
#pragma once
#include "modules/qmi/ModemEndPoint.h"
#include "OTTModemEndPointModule.h"
#include "framework/Log.h"

class OTTModemEndPoint : public ModemEndPoint
{
public:
  static constexpr const char *NAME = "OTTModemEndPoint";

  OTTModemEndPoint() : ModemEndPoint(NAME) {
    mModule = new OTTModemEndPointModule("OTTModemEndPointModule", *this);
    mModule->init();
    Log::getInstance().d("[OTTModemEndPoint]: xtor");
  }

  ~OTTModemEndPoint() {
    Log::getInstance().d("[OTTModemEndPoint]: destructor");
    delete mModule;
    mModule = nullptr;
  }

  void requestSetup(string clientToken, GenericCallback<string>* cb);

};
#endif /* FEATURE_DATA_LQE */
