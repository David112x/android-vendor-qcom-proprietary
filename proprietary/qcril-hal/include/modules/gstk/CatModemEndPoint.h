/******************************************************************************
#  Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#  ---------------------------------------------------------------------------
#******************************************************************************/

#pragma once
#include "modules/qmi/ModemEndPoint.h"
#include "framework/Log.h"

/*=========================================================================

  CLASS:  CatModemEndPoint

===========================================================================*/
class CatModemEndPoint : public ModemEndPoint {
 public:
  CatModemEndPoint();
  ~CatModemEndPoint()
  {
    Log::getInstance().d("[CatModemEndPoint]: destructor");
    delete mModule;
    mModule = nullptr;
  }
  Message::Callback::Status requestSetupSync(std::shared_ptr<string>& sharedResponse);
  void cleanUp();
};
