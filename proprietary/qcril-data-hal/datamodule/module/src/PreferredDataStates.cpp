/*
 * Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "modules/qmi/ModemEndPointFactory.h"

#include "DSDModemEndPoint.h"
#include "PreferredDataStates.h"
#include "UnSolMessages/DDSSwitchIPCMessage.h"
#include "sync/TriggerDDSSwitchSyncMessage.h"
#include "request/SetPreferredDataModemRequestMessage.h"
#include  "UnSolMessages/NewDDSInfoMessage.h"
#include "UnSolMessages/VoiceIndMessage.h"
#include "legacy/qcril_data.h"
#include "DataCommon.h"

#include "SMLog.h"

using namespace SM;
using namespace rildata;
using std::placeholders::_1;

PreferredDataState::PreferredDataState(
    string name,
    function<void(int)> fn,
    shared_ptr<PreferredDataInfo_t> stateInfo
): IState(name, fn), mStateInfo(stateInfo)
{}

InitialState::InitialState(
    function<void(int)> fn,
    shared_ptr<PreferredDataInfo_t> info
) : PreferredDataState("Initial", fn, info)
{}

InitializedState::InitializedState(
    function<void(int)> fn,
    shared_ptr<PreferredDataInfo_t> info
) : PreferredDataState("Initialized", fn, info)
{}

SwitchingState::SwitchingState(
    function<void(int)> fn,
    shared_ptr<PreferredDataInfo_t> info
) : PreferredDataState("Switching", fn, info)
{}

DefaultDataState::DefaultDataState(
    function<void(int)> fn,
    shared_ptr<PreferredDataInfo_t> info
) : PreferredDataState("DefaultData", fn, info)
{}

NonDefaultDataState::NonDefaultDataState(
    function<void(int)> fn,
    shared_ptr<PreferredDataInfo_t> info
) : PreferredDataState("NonDefaultData", fn, info)
{}

ApTriggeredState::ApTriggeredState(
    function<void(int)> fn,
    shared_ptr<PreferredDataInfo_t> info
) : PreferredDataState("ApTriggered", fn, info)
{}

MpTriggeredState::MpTriggeredState(
    function<void(int)> fn,
    shared_ptr<PreferredDataInfo_t> info
) : PreferredDataState("MpTriggered", fn, info)
{}

string PreferredDataEventType::getEventName() const {
    switch (event) {
        case PreferredDataEventEnum::SetPreferredDataModem:
            return "SetPreferredDataModem";
        case PreferredDataEventEnum::InitializeSM:
            return "InitializeSM";
        case PreferredDataEventEnum::DDSSwitchInd:
            return "DDSSwitchInd";
        case PreferredDataEventEnum::CurrentDDSInd:
            return "CurrentDDSInd";
        case PreferredDataEventEnum::DDSSwitchTimerExpired:
            return "DDSSwitchTimerExpired";
        case PreferredDataEventEnum::DDSSwitchApStarted:
            return "DDSSwitchApStarted";
        case PreferredDataEventEnum::DDSSwitchCompleted:
            return "DDSSwitchCompleted";
        case PreferredDataEventEnum::DDSSwitchMpStarted:
            return "DDSSwitchMpStarted";
        case PreferredDataEventEnum::TempDDSSwitchTimerExpired:
            return "TempDDSSwitchTimerExpired";
        default:
            return "UNKNOWN";
    }
}

void PreferredDataState::ddsTransition() {
    int nextState = (mStateInfo->dds == mStateInfo->sub) ? DefaultData : NonDefaultData;
    transitionTo(nextState);
}

void PreferredDataState::sendResponse(std::shared_ptr<Message> msg, SetPreferredDataModemResult_t error) {
    SetPreferredDataModemResponse_t response;
    auto m = std::static_pointer_cast<SetPreferredDataModemRequestMessage>(msg);
    response.errCode = error;
    if (m != nullptr) {
        SM_LOGD("responding to SetPreferredDataModem with error=" +
                             std::to_string(static_cast<int>(error)));
        auto r = make_shared<SetPreferredDataModemResponse_t>(response);
        if (error == SetPreferredDataModemResult_t::NO_ERROR) {
            m->sendResponse(m, Message::Callback::Status::SUCCESS, r);
        } else {
            m->sendResponse(m, Message::Callback::Status::FAILURE, r);
        }
    } else {
        SM_LOGE("Could not send response. Message is null");
    }
}

int InitialState::handleEvent(const PreferredDataEventType& e) {
    SM_LOGD(getName() + ": handling " + e.getEventName());
    switch (e.event) {
        case InitializeSM:
        {
            DDSSubIdInfo ddsInfo;
            ddsInfo.dds_sub_id = QCRIL_INVALID_MODEM_STACK_ID;
            ddsInfo.switch_type = DSD_DDS_DURATION_ENUM_MIN_ENUM_VAL_V01;
            // get current dds, switch to DefaultData or NonDefaultData
            auto status = ModemEndPointFactory<DSDModemEndPoint>::getInstance().buildEndPoint()->getCurrentDDSSync(ddsInfo);
            if (status == Message::Callback::Status::SUCCESS) {
                 //Post DDS SUB Info to RIL Common
                auto msg = std::make_shared<rildata::NewDDSInfoMessage>(global_instance_id, ddsInfo);
                msg->broadcast();

                status = ModemEndPointFactory<DSDModemEndPoint>::getInstance().buildEndPoint()->registerForCurrentDDSInd();
                if (status == Message::Callback::Status::SUCCESS) {
                    mStateInfo->dds = ddsInfo.dds_sub_id;
                    ddsTransition();
                } else {
                    SM_LOGE(getName() + ": Failed to register for current DDS ind");
                }
            } else {
                SM_LOGE(getName() + ": Failed to get current DDS ");
            }
            return HANDLED;
        }
        case SetPreferredDataModem:
        {
            DDSSwitchInfo_t *switchInfo = static_cast<DDSSwitchInfo_t *>(e.data);
            auto m = std::static_pointer_cast<SetPreferredDataModemRequestMessage>(switchInfo->msg);
            SetPreferredDataModemResponse_t resp;
            resp.errCode = SetPreferredDataModemResult_t::INVALID_OPERATION;
            auto r = make_shared<SetPreferredDataModemResponse_t>(resp);
            SM_LOGE("responding to SetPreferredDataModem with error=" +
                                    std::to_string(static_cast<int>(r->errCode)));
            m->sendResponse(m, Message::Callback::Status::FAILURE, r);
            return HANDLED;
        }
        default:
            break;
    }
    return HANDLED;
}

int InitializedState::handleEvent(const PreferredDataEventType& e) {
    DDSSwitchInfo_t *switchInfo = static_cast<DDSSwitchInfo_t *>(e.data);
    SM_LOGD(getName() + ": handling " + e.getEventName());
    switch (e.event) {
        case SetPreferredDataModem:
        {
            // save request msg so we can respond after CURRENT_DDS_IND arrives
            auto m = std::static_pointer_cast<SetPreferredDataModemRequestMessage>(switchInfo->msg);
            if (switchInfo->subId == mStateInfo->dds) {
                // respond with success
                SM_LOGD("[InitializedState]: responding to SetPreferredDataModem with success");
                SetPreferredDataModemResponse_t resp;
                resp.errCode = SetPreferredDataModemResult_t::NO_ERROR;
                auto r = make_shared<SetPreferredDataModemResponse_t>(resp);
                m->sendResponse(m, Message::Callback::Status::SUCCESS, r);
            } else {
                mStateInfo->mPendingMessage =
                    std::static_pointer_cast<SetPreferredDataModemRequestMessage>(switchInfo->msg);
                transitionTo(ApTriggered);
            }
            return HANDLED;
        }
        case DDSSwitchApStarted:
        {
            transitionTo(ApTriggered);
            return HANDLED;
            break;
        }
        case CurrentDDSInd:
        case DDSSwitchMpStarted:
        {
            if (switchInfo->subId == mStateInfo->dds) {
                SM_LOGD(getName() + ": already on the triggered dds sub. Ignoring " + e.getEventName());
            } else {
                // cache current dds
                mStateInfo->dds = switchInfo->subId;
                transitionTo(MpTriggered);
                SM_LOGD(getName() + " switch dds to sub " + std::to_string(mStateInfo->dds));
            }
            return HANDLED;
        }
        case TempDDSSwitchTimerExpired:
        {
            SM_LOGD("Temp DDS timer Expired");
            int error;
            Message::Callback::Status status = Message::Callback::Status::FAILURE;
            mStateInfo->switch_type = DSD_DDS_SWITCH_PERMANENT_V01;
            SM_LOGD("Initiating Permanent DDS switch with sub" + std::to_string(mStateInfo->dds));
            status = ModemEndPointFactory<DSDModemEndPoint>::getInstance()
                         .buildEndPoint()->triggerDDSSwitchSync(mStateInfo->dds, error, mStateInfo->switch_type);
            return HANDLED;
        }
        default:
            break;
    }
    return UNHANDLED;
}

int DefaultDataState::handleEvent(const PreferredDataEventType& e) {
    std::ignore = e;
    return UNHANDLED;
}

int NonDefaultDataState::handleEvent(const PreferredDataEventType& e) {
    std::ignore = e;
    return UNHANDLED;
}

int SwitchingState::handleEvent(const PreferredDataEventType& e) {
    std::ignore = e;
    return UNHANDLED;
}

void ApTriggeredState::enter() {
    if (mStateInfo->isRilIpcNotifier) {
        SM_LOGD("[ApTriggeredState]: broadcasting IPC switch started");
        // Send ipc message
        auto ipcStarted = make_shared<DDSSwitchIPCMessage>(DDSSwitchIPCMessageState::ApStarted,
                                                        mStateInfo->dds);
        ipcStarted->broadcast();
        // send dds switch request to modem
        int error;
        Message::Callback::Status status = Message::Callback::Status::FAILURE;

        auto pendingMessage = std::static_pointer_cast<SetPreferredDataModemRequestMessage>(mStateInfo->mPendingMessage);
        if (pendingMessage != nullptr) {
            status = ModemEndPointFactory<DSDModemEndPoint>::getInstance()
                        .buildEndPoint()->triggerDDSSwitchSync(pendingMessage->getModemId(), error, mStateInfo->switch_type);
        } else {
            SM_LOGE("[ApTriggeredState]: pending SetPreferredDataModem message is null");
            error = static_cast<int>(SetPreferredDataModemResult_t::INVALID_ARG);
        }
        if (status != Message::Callback::Status::SUCCESS) {
            sendResponse(pendingMessage, static_cast<SetPreferredDataModemResult_t>(error));
            pendingMessage = nullptr;
        }
    }
}

void ApTriggeredState::exit() {
    if (mStateInfo->isRilIpcNotifier) {
        SM_LOGD("[ApTriggeredState]: broadcasting IPC switch Completed");
        // Send ipc message
        auto ipcCompleted = make_shared<DDSSwitchIPCMessage>(DDSSwitchIPCMessageState::Completed,
                                                            mStateInfo->dds);
        ipcCompleted->broadcast();
    }
}

int ApTriggeredState::handleEvent(const PreferredDataEventType& e) {
    SM_LOGD(getName() + ": handling " + e.getEventName());
    DDSSwitchInfo_t *switchInfo = static_cast<DDSSwitchInfo_t *>(e.data);
    switch (e.event) {
        case CurrentDDSInd:
        {
            if (mStateInfo->isRilIpcNotifier) {
                mStateInfo->dds = switchInfo->subId;
                ddsTransition();
                sendResponse(mStateInfo->mPendingMessage, SetPreferredDataModemResult_t::NO_ERROR);
                mStateInfo->mPendingMessage = nullptr;
            }
            return HANDLED;
        }
        case DDSSwitchInd:
        {
            if (switchInfo->switchAllowed == DDSSwitchRes::ALLOWED) {
                return HANDLED;
            }
            ddsTransition();

            if (switchInfo->switchAllowed == DDSSwitchRes::FAIL) {
                SM_LOGD("[ApTriggeredState]: DDSSwitch failed");
                sendResponse(mStateInfo->mPendingMessage, SetPreferredDataModemResult_t::DDS_SWITCH_NOT_ALLOWED);
                mStateInfo->mPendingMessage = nullptr;
            }

            return HANDLED;
        }
        case DDSSwitchTimerExpired:
        {
            ddsTransition();
            sendResponse(mStateInfo->mPendingMessage, SetPreferredDataModemResult_t::DDS_SWITCH_TIMEOUT);
            mStateInfo->mPendingMessage = nullptr;
            return HANDLED;
        }
        case SetPreferredDataModem:
        {
            /* If SetPreferredDataModem is triggered while request is still pending,
               respond with error */
            std::shared_ptr<SetPreferredDataModemRequestMessage> m =
                    std::static_pointer_cast<SetPreferredDataModemRequestMessage>(switchInfo->msg);
            if (m != nullptr) {
                SetPreferredDataModemResponse_t resp;
                resp.errCode = SetPreferredDataModemResult_t::REQUEST_ALREADY_PENDING;
                auto r = make_shared<SetPreferredDataModemResponse_t>(resp);
                SM_LOGE("responding to SetPreferredDataModem with error=" +
                                     std::to_string(static_cast<int>(r->errCode)));
                m->sendResponse(m, Message::Callback::Status::FAILURE, r);
            }
            return HANDLED;
        }
        case DDSSwitchCompleted:
        {
            mStateInfo->dds = switchInfo->subId;
            ddsTransition();
            return HANDLED;
        }
        default:
            break;
    }
    SM_LOGD(getName() + ": ignored " + e.getEventName());
    return UNHANDLED;
}

void MpTriggeredState::enter() {
    if (mStateInfo->isRilIpcNotifier) {
        SM_LOGD("[ApTriggeredState]: broadcasting MP IPC switch started");
        // Send ipc message
        auto ipcStarted = make_shared<DDSSwitchIPCMessage>(DDSSwitchIPCMessageState::MpStarted,
                                                             mStateInfo->dds);
        ipcStarted->broadcast();
    }
}

int MpTriggeredState::handleEvent(const PreferredDataEventType& e) {
    SM_LOGD(getName() + ": handling " + e.getEventName());
    DDSSwitchInfo_t *switchInfo = static_cast<DDSSwitchInfo_t *>(e.data);
    switch (e.event) {
        case SetPreferredDataModem:
        {
            auto m = std::static_pointer_cast<SetPreferredDataModemRequestMessage>(switchInfo->msg);
            if (switchInfo->subId == mStateInfo->dds) {
                // Send ipc message
                auto ipcStarted = make_shared<DDSSwitchIPCMessage>(DDSSwitchIPCMessageState::ApStarted,
                                                                   switchInfo->subId);
                ipcStarted->broadcast();
                // respond with success
                SM_LOGD(getName() + " responding to SetPreferredDataModem with success");
                SetPreferredDataModemResponse_t resp;
                resp.errCode = SetPreferredDataModemResult_t::NO_ERROR;
                auto r = make_shared<SetPreferredDataModemResponse_t>(resp);
                m->sendResponse(m, Message::Callback::Status::SUCCESS, r);
                ddsTransition();
            } else {
                // save request msg so we can respond after CURRENT_DDS_IND arrives
                mStateInfo->mPendingMessage = m;
                mStateInfo->dds = switchInfo->subId;
                transitionTo(ApTriggered);
            }
            return HANDLED;
        }
        case DDSSwitchApStarted:
            if (switchInfo->subId == mStateInfo->dds) {
                ddsTransition();
            } else {
                mStateInfo->dds = switchInfo->subId;
                transitionTo(ApTriggered);
            }
            return HANDLED;
        default:
            break;
    }
    SM_LOGD(getName() + ": ignored " + e.getEventName());
    return UNHANDLED;
}
