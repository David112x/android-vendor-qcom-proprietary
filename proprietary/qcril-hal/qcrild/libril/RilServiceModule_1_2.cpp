/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#define TAG "RilServiceModule_1_2"
#include "RilServiceModule_1_2.h"

#if 1
// Move to common files */
using MessageHandlerTable = std::unordered_map<message_id_t,
        std::function<void(std::shared_ptr<Message>)>>;

void updateMessageHandler(MessageHandlerTable& consolidateList,
        MessageHandlerTable& updateList) {
    for (auto it = updateList.begin(); it != updateList.end(); it++)
        consolidateList[it->first] = it->second;
}
#endif

RilServiceModule_1_2::RilServiceModule_1_2(qcril_instance_id_e_type instance)
        : RilServiceModule(instance) {
    mName = "RilServiceModule_1_2";

    using std::placeholders::_1;
    MessageHandlerTable updateList = {
        HANDLER(QcrilInitMessage, RilServiceModule_1_2::handleQcrilInit),
        HANDLER(LinkCapIndMessage, RilServiceModule_1_2::handleLinkCapIndMessage),
    };
    updateMessageHandler(mMessageHandler, updateList);
}

void RilServiceModule_1_2::handleQcrilInit(std::shared_ptr<QcrilInitMessage> msg) {
    QCRIL_LOG_DEBUG("Handling QcrilInitMessage %s",msg->dump().c_str());
    std::shared_ptr<NasSetSignalStrengthCriteriaSupported> cfgMsg =
        std::make_shared<NasSetSignalStrengthCriteriaSupported>(true);
    if (cfgMsg) {
        cfgMsg->dispatch();
    }
}

void RilServiceModule_1_2::handleLinkCapIndMessage(std::shared_ptr<LinkCapIndMessage> msg) {
    QCRIL_LOG_DEBUG("Handling LinkCapIndMessage %s",msg->dump().c_str());
    auto ri = getRadioImpl();
    if (ri) {
        auto radioServiceRwlockPtr = radio::getRadioServiceRwlock(ri->mSlotId);
        radioServiceRwlockPtr->lock_shared();

        /* FIXME: find better ways to get 1_2 implementation */
        static_cast<RadioImpl_1_2*>(ri.get())->sendLinkCapInd(msg);

        radioServiceRwlockPtr->unlock_shared();
    }
}
