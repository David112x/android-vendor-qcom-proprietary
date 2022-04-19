/******************************************************************************
#  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#ifndef QMI_RIL_UTF

#define TAG "RadioConfig_Module"
#include "RadioConfigModule.h"
#include <framework/ModuleLooper.h>
#include <framework/Log.h>


/*===========================================================================

FUNCTION:  RadioConfigModule::RadioConfigModule()

===========================================================================*/
RadioConfigModule::RadioConfigModule()
{
  mName  = "RadioConfigModule";
  QCRIL_LOG_INFO("RadioConfigModule::RadioConfigModule");
  mLooper = std::unique_ptr<ModuleLooper>(new ModuleLooper);

  using std::placeholders::_1;
  mMessageHandler = {
      HANDLER(QcrilInitMessage, RadioConfigModule::handleQcrilInitMessage),
      HANDLER(UimSlotStatusInd, RadioConfigModule::handleSlotStatusIndiaction),
  };
} /* RadioConfigModule::RadioConfigModule */


/*===========================================================================

FUNCTION:  RadioConfigModule::RadioConfigModule()

===========================================================================*/
RadioConfigModule::~RadioConfigModule()
{
  mLooper = nullptr;
} /* RadioConfigModule::~RadioConfigModule() */


/*===========================================================================

FUNCTION:  RadioConfigModule::init()

===========================================================================*/
void RadioConfigModule::init()
{
  Module::init();
} /* RadioConfigModule::init */


/*===========================================================================

FUNCTION:  RadioConfigModule::getRadioConfigImpl()

===========================================================================*/
sp<RadioConfigImpl> RadioConfigModule::getRadioConfigImpl()
{
  return getRadioConfig();
} /* RadioConfigModule::getRadioConfigImpl */


/*===========================================================================

FUNCTION:  RadioConfigModule::handleQcrilInitMessage

===========================================================================*/
void RadioConfigModule::handleQcrilInitMessage
(
  std::shared_ptr<QcrilInitMessage> msg
)
{
  if (msg != NULL)
  {
    QCRIL_LOG_INFO("Handling %s", msg->dump().c_str());
  }
} /* RadioConfigModule::handleQcrilInitMessage */


/*===========================================================================

FUNCTION:  RadioConfigModule::handleSlotStatusIndiaction

===========================================================================*/
void RadioConfigModule::handleSlotStatusIndiaction
(
  const std::shared_ptr<UimSlotStatusInd> msg_ptr
)
{
  if (msg_ptr == NULL)
  {
    return;
  }

  QCRIL_LOG_INFO("Handling %s", msg_ptr->dump().c_str());

  auto rci = getRadioConfigImpl();
  if (rci) {
    rci->sendSlotStatusIndication(msg_ptr);
  }
} /* RadioConfigModule::handleSlotStatusIndiaction */

#endif

