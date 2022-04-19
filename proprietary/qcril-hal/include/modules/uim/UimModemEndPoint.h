/******************************************************************************
#  @file    UimModemEndPoint.h
#  @brief   Base class header file for QMI UIM module
#
#  ---------------------------------------------------------------------------
#
#  Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#  ---------------------------------------------------------------------------
#******************************************************************************/

#pragma once
#include "modules/qmi/ModemEndPoint.h"
#include "framework/Log.h"

/*=========================================================================

  CLASS:  UimModemEndPoint

===========================================================================*/
class UimModemEndPoint : public ModemEndPoint {
 public:
  UimModemEndPoint();
  ~UimModemEndPoint()
  {
    Log::getInstance().d("[UimModemEndPoint]: destructor");
    delete mModule;
    mModule = nullptr;
  }
  Message::Callback::Status requestSetupSync(std::shared_ptr<string>& sharedResponse);
  void cleanUp();
};
