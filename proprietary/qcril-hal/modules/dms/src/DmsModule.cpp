/******************************************************************************
#  Copyright (c) 2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#define TAG "[DmsModule]"
#include <cstring>
#include <framework/ModuleLooper.h>
#include <framework/QcrilInitMessage.h>
#include <framework/PolicyManager.h>
#include <framework/SingleDispatchRestriction.h>
#include <framework/SingleDispatchRestrictionHandleDuplicate.h>
#include <framework/ThreadPoolManager.h>

#include <modules/android/ril_message_factory.h>

#include "qcril_log.h"


#include "modules/nas/qcril_qmi_nas.h"
#include "modules/dms/DmsModule.h"
#include "telephony/ril.h"

static load_module<DmsModule> dms_module;

DmsModule &getDmsModule() {
    return (dms_module.get_module());
}

/*
 * 1. Indicate your preference for looper.
 * 2. Subscribe to the list of messages via mMessageHandler.
 * 3. Follow RAII practice.
 */
DmsModule::DmsModule() : AddPendingMessageList("DmsModule") {
  mName = "DmsModule";

  using std::placeholders::_1;
  mMessageHandler = {
    HANDLER(QcrilInitMessage, DmsModule::handleQcrilInit),

    HANDLER(RilRequestRadioPowerMessage, DmsModule::handleRadioPowerRequest),
    HANDLER(RilRequestGetModemActivityMessage, DmsModule::handleGetActivityInfoRequest),
    HANDLER(RilRequestGetBaseBandVersionMessage, DmsModule::handleBaseBandVersionRequest),
    HANDLER(RilRequestGetDeviceIdentityMessage, DmsModule::handleDeviceIdentiyRequest),

    // End Point Status Indication
    {REG_MSG("DMS_ENDPOINT_STATUS_IND"),
             std::bind(&DmsModule::handleDmsEndpointStatusIndMessage, this, _1)},
  };
}

/* Follow RAII.
*/
DmsModule::~DmsModule() {}

/*
 * Module specific initialization that does not belong to RAII .
 */
void DmsModule::init() {
    Module::init();

    PolicyManager::getInstance().setMessageRestriction(std::vector<std::string>{
        RilRequestGetModemActivityMessage::MESSAGE_NAME},
        std::make_shared<SingleDispatchRestrictionHandleDuplicate>());

    PolicyManager::getInstance().setMessageRestriction(std::vector<std::string>{
        RilRequestGetBaseBandVersionMessage::MESSAGE_NAME},
        std::make_shared<SingleDispatchRestrictionHandleDuplicate>());

    PolicyManager::getInstance().setMessageRestriction(std::vector<std::string>{
        RilRequestGetDeviceIdentityMessage::MESSAGE_NAME},
        std::make_shared<SingleDispatchRestrictionHandleDuplicate>());

    PolicyManager::getInstance().setMessageRestriction(std::vector<std::string>{
        RilRequestRadioPowerMessage::MESSAGE_NAME},
        std::make_shared<SingleDispatchRestriction>());
}

void DmsModule::handleQcrilInit(std::shared_ptr<Message> msg) {
    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());
}


void DmsModule::handleDmsEndpointStatusIndMessage(std::shared_ptr<Message> msg) {
    auto shared_indMsg(std::static_pointer_cast<EndpointStatusIndMessage>(msg));

    Log::getInstance().d("[" + mName + "]: Handling msg = " + msg->dump());

    if (shared_indMsg->getState() == ModemEndPoint::State::OPERATIONAL) {
        mReady = true;
        qcril_qmi_nas_notify_radio_power_if_dms_up();
        qcril_qmi_nas_trigger_bootup_optimization();
    } else {
        // state is not operational
        mReady = false;
        clearPendingMessageList();
    }
}

void qcril_qmi_nas_request_power_wrapper(void *cb_data)
{
    auto schedCbData = static_cast<ScheduleCallbackData*>(cb_data);
    auto msg(std::static_pointer_cast<RilRequestRadioPowerMessage>(schedCbData->getData()));
    delete schedCbData;
    qcril_qmi_nas_request_power(msg);
}


void DmsModule::handleRadioPowerRequest(std::shared_ptr<RilRequestRadioPowerMessage> shared_msg) {
    Log::getInstance().d("[" + mName + "]: Handling msg = " + shared_msg->dump());

    auto cbData = new ScheduleCallbackData(shared_msg);
    if (cbData) {
        ThreadPoolManager::getInstance().scheduleExecution(
            qcril_qmi_nas_request_power_wrapper, cbData);
    } else {
        shared_msg->sendResponse(shared_msg, Message::Callback::Status::SUCCESS,
            std::make_shared<QcRilRequestMessageCallbackPayload>(RIL_E_NO_MEMORY, nullptr));
    }
}


void DmsModule::handleGetActivityInfoRequest(std::shared_ptr<RilRequestGetModemActivityMessage> shared_msg) {
    Log::getInstance().d("[" + mName + "]: Handling msg = " + shared_msg->dump());
    if (mReady) {
        qcril_qmi_nas_dms_get_activity_info(shared_msg);
    } else {
        shared_msg->sendResponse(shared_msg, Message::Callback::Status::SUCCESS,
            std::make_shared<QcRilRequestMessageCallbackPayload>(RIL_E_RADIO_NOT_AVAILABLE , nullptr));
    }
}


void DmsModule::handleBaseBandVersionRequest(std::shared_ptr<RilRequestGetBaseBandVersionMessage> shared_msg) {
    Log::getInstance().d("[" + mName + "]: Handling msg = " + shared_msg->dump());
    if (mReady) {
        qcril_qmi_nas_dms_request_baseband_version(shared_msg);
    } else {
        shared_msg->sendResponse(shared_msg, Message::Callback::Status::SUCCESS,
            std::make_shared<QcRilRequestMessageCallbackPayload>(RIL_E_RADIO_NOT_AVAILABLE , nullptr));
    }
}


void DmsModule::handleDeviceIdentiyRequest(std::shared_ptr<RilRequestGetDeviceIdentityMessage> shared_msg) {
    Log::getInstance().d("[" + mName + "]: Handling msg = " + shared_msg->dump());
    if (mReady) {
        qcril_qmi_nas_dms_request_device_identity(shared_msg);
    } else {
        shared_msg->sendResponse(shared_msg, Message::Callback::Status::SUCCESS,
            std::make_shared<QcRilRequestMessageCallbackPayload>(RIL_E_RADIO_NOT_AVAILABLE , nullptr));
    }
}

