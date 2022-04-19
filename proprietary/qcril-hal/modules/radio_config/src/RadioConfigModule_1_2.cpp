/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#ifndef QMI_RIL_UTF

#define TAG "RadioConfig_Module_1_2"
#include "RadioConfigModule_1_2.h"
#include <framework/ModuleLooper.h>
#include <framework/Log.h>

using MessageHandlerTable = std::unordered_map<message_id_t,
        std::function<void(std::shared_ptr<Message>)>>;

extern void updateMessageHandler(MessageHandlerTable& consolidateList,
                                          MessageHandlerTable& updateList);


/*===========================================================================

FUNCTION:  RadioConfigModule_1_2::RadioConfigModule_1_2()

===========================================================================*/
RadioConfigModule_1_2::RadioConfigModule_1_2() : RadioConfigModule_1_1()
{
  mName  = "RadioConfigModule_1_2";

  using std::placeholders::_1;
  MessageHandlerTable updateList = {
      HANDLER(QcrilInitMessage, RadioConfigModule_1_2::handleQcrilInitMessage),
      HANDLER(UimSlotStatusInd, RadioConfigModule_1_2::handleSlotStatusIndication_1_2),
  };
  updateMessageHandler(mMessageHandler, updateList);
} /* RadioConfigModule_1_1::RadioConfigModule_1_1 */


/*===========================================================================

FUNCTION:  RadioConfigModule_1_1::~RadioConfigModule_1_1()

===========================================================================*/
RadioConfigModule_1_2::~RadioConfigModule_1_2()
{
} /* RadioConfigModule_1_1::~RadioConfigModule_1_1 */


/*===========================================================================

FUNCTION:  RadioConfigModule_1_1::init()

===========================================================================*/
void RadioConfigModule_1_2::init()
{
  RadioConfigModule_1_1::init();
} /* RadioConfigModule_1_1::init */

/*===========================================================================

FUNCTION:  RadioConfigModule_1_1::handleQcrilInitMessage

===========================================================================*/
void RadioConfigModule_1_2::handleQcrilInitMessage
(
  std::shared_ptr<QcrilInitMessage> msg
)
{
  return RadioConfigModule_1_1::handleQcrilInitMessage(msg);
} /* RadioConfigModule_1_1::handleQcrilInitMessage */


/*===========================================================================

FUNCTION:  RadioConfigModule::handleSlotStatusIndiaction

===========================================================================*/
void RadioConfigModule_1_2::handleSlotStatusIndication_1_2
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
