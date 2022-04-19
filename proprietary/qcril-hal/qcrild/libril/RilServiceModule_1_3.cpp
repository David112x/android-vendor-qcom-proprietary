/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#define TAG "RilServiceModule_1_3"
#include "RilServiceModule_1_3.h"

// Move to common files */
using MessageHandlerTable = std::unordered_map<message_id_t,
        std::function<void(std::shared_ptr<Message>)>>;

extern void updateMessageHandler(MessageHandlerTable& consolidateList,
        MessageHandlerTable& updateList);

RilServiceModule_1_3::RilServiceModule_1_3(qcril_instance_id_e_type instance)
        : RilServiceModule_1_2(instance) {
    mName = "RilServiceModule_1_3";

    using std::placeholders::_1;
    MessageHandlerTable updateList = {
        HANDLER(QcrilInitMessage, RilServiceModule_1_3::handleQcrilInit),
    };
    updateMessageHandler(mMessageHandler, updateList);
}

void RilServiceModule_1_3::handleQcrilInit(std::shared_ptr<QcrilInitMessage> msg) {
    QCRIL_LOG_DEBUG("Handling QcrilInitMessage %s",msg->dump().c_str());
    std::shared_ptr<NasSetSignalStrengthCriteriaSupported> cfgMsg =
        std::make_shared<NasSetSignalStrengthCriteriaSupported>(true);
    if (cfgMsg) {
        cfgMsg->dispatch();
    }
}

