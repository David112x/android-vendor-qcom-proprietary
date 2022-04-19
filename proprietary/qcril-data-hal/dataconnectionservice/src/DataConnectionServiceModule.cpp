/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "framework/Log.h"
#include "framework/ModuleLooper.h"
#include "framework/QcrilInitMessage.h"

#include "BearerAllocationUpdateMessage.h"
#include "DataConnectionServiceModule.h"

using namespace rildata;

static load_module<rildata::DataConnectionServiceModule> DataConnectionServiceModule;

/**
 * DataConnectionServiceModule::DataConnectionServiceModule()
 *
 * @brief
 * Initializes DataConnectionServiceModule
 */
DataConnectionServiceModule::DataConnectionServiceModule() {
    Log::getInstance().d("DataConnectionServiceModule()");
    mLooper = std::unique_ptr<ModuleLooper>(new ModuleLooper);
    mName = "DataConnectionServiceModule";
    mMessageHandler = {
        HANDLER(QcrilInitMessage, DataConnectionServiceModule::handleQcrilInit),
        HANDLER(BearerAllocationUpdateMessage, DataConnectionServiceModule::handleBearerAllocationUpdate)
    };
}

void DataConnectionServiceModule::init() {
    Log::getInstance().d("DataConnectionServiceModule::init");
    Module::init();
}

/**
 * DataConnectionServiceModule::handleQcrilInit()
 *
 * @brief
 * Handler for QcrilInit message. Registers DataConnection HAL service
 */
void DataConnectionServiceModule::handleQcrilInit(std::shared_ptr<Message> msg)
{
    std::shared_ptr<QcrilInitMessage> initMsg = std::static_pointer_cast<QcrilInitMessage>(msg);
    Log::getInstance().d("[" + mName + "]: Handling msg = " + initMsg->dump() +
                         ", instance = " + std::to_string((int)initMsg->get_instance_id()));
    mConnection = new DataConnectionServiceImpl(initMsg->get_instance_id());
    mConnection->init();
}

/**
 * DataConnectionServiceModule::handleQcrilInit()
 *
 * @brief
 * Handler for BearerAllocationUpdate message. Sends updated bearer list to
 * registered HAL clients
 */
void DataConnectionServiceModule::handleBearerAllocationUpdate(std::shared_ptr<Message> msg)
{
    std::shared_ptr<BearerAllocationUpdateMessage> updateMsg = std::static_pointer_cast<BearerAllocationUpdateMessage>(msg);
    Log::getInstance().d("[" + mName + "]: Handling msg = " + updateMsg->dump());
    if (mConnection != nullptr) {
        mConnection->notifyBearerAllocationUpdate(updateMsg->getBearers());
    }
}
