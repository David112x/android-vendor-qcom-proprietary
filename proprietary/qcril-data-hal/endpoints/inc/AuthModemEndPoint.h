/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include <vector>

#include "framework/Log.h"
#include "modules/qmi/ModemEndPoint.h"

#include "AuthManager.h"
#include "AuthModemEndPointModule.h"

class AuthModemEndPoint : public ModemEndPoint {
 public:
  static constexpr const char *NAME = "AUTH";
  AuthModemEndPoint() : ModemEndPoint(NAME) {
    mModule = new AuthModemEndPointModule("AuthModemEndPointModule", *this);
    mModule->init();
    Log::getInstance().d("[AuthModemEndPoint]: xtor");
  }
  ~AuthModemEndPoint() {
      Log::getInstance().d("[AuthModemEndPoint]: destructor");
    delete mModule;
    mModule = nullptr;
  }

  void requestSetup(string clientToken, GenericCallback<string>* cb);
};
