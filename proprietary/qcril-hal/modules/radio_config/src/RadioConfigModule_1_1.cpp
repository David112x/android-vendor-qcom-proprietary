/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#ifndef QMI_RIL_UTF

#define TAG "RadioConfig_Module_1_1"
#include <android/hardware/radio/1.0/types.h>
#include "RadioConfigModule_1_1.h"
#include <framework/ModuleLooper.h>
#include <framework/Log.h>

using MessageHandlerTable = std::unordered_map<message_id_t,
        std::function<void(std::shared_ptr<Message>)>>;

extern void updateMessageHandler(MessageHandlerTable& consolidateList,
        MessageHandlerTable& updateList);
/*===========================================================================

FUNCTION:  RadioConfigModule_1_1::RadioConfigModule_1_1()

===========================================================================*/
RadioConfigModule_1_1::RadioConfigModule_1_1() : RadioConfigModule()
{
    mName  = "RadioConfigModule_1_1";

    using std::placeholders::_1;
    MessageHandlerTable updateList = {
        HANDLER(QcrilInitMessage, RadioConfigModule_1_1::handleQcrilInitMessage),
    };
    updateMessageHandler(mMessageHandler, updateList);
} /* RadioConfigModule_1_1::RadioConfigModule_1_1 */


/*===========================================================================

FUNCTION:  RadioConfigModule_1_1::~RadioConfigModule_1_1()

===========================================================================*/
RadioConfigModule_1_1::~RadioConfigModule_1_1()
{
} /* RadioConfigModule_1_1::~RadioConfigModule_1_1 */


/*===========================================================================

FUNCTION:  RadioConfigModule_1_1::init()

===========================================================================*/
void RadioConfigModule_1_1::init()
{
    RadioConfigModule::init();
} /* RadioConfigModule_1_1::init */

/*===========================================================================

FUNCTION:  RadioConfigModule_1_1::handleQcrilInitMessage

===========================================================================*/
void RadioConfigModule_1_1::handleQcrilInitMessage
(
  std::shared_ptr<QcrilInitMessage> msg
)
{
    return RadioConfigModule::handleQcrilInitMessage(msg);
} /* RadioConfigModule_1_1::handleQcrilInitMessage */

#endif