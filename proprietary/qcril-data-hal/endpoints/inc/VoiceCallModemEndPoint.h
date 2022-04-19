/******************************************************************************
#  Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once
#include "modules/qmi/ModemEndPoint.h"
#include "VoiceCallModemEndPointModule.h"
#include "framework/Log.h"
#include "voice_service_v02.h"

#define QCRIL_DATA_QMI_TIMEOUT 10000
#define VOICEIND_WAITING_TIMEOUT 20

class VoiceCallModemEndPoint : public ModemEndPoint
{
public:
  VoiceCallModemEndPoint(std::string moduleName, rildata::SubId sub) : ModemEndPoint(moduleName)
  {
    mModule = new VoiceCallModemEndPointModule("VoiceCallModemEndPointModule", *this, sub);
    mModule->init();
    mSub = sub;
    Log::getInstance().d("[VoiceCallModemEndPoint]: xtor");
  }
  ~VoiceCallModemEndPoint()
  {
    Log::getInstance().d("[VoiceCallModemEndPoint]: destructor");
    delete mModule;
    mModule = nullptr;
  }

  void requestSetup(string clientToken, GenericCallback<string>* cb);
  bool isVoiceCall();

  TimeKeeper::timer_id mVoiceCallOrigTimer;
  bool mIsVoiceCallOrigTimer = false;

private:
  rildata::SubId mSub;
};
