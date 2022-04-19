/*
 * Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <framework/TimeKeeper.h>

#include "IState.h"
#include "MessageCommon.h"
#include "DataCommon.h"
#include "voice_service_v02.h"
#include  "voice_service_common_v02.h"
#include "data_system_determination_v01.h"
#include "UnSolMessages/VoiceIndMessage.h"

namespace rildata {

using SM::IState;

enum PreferredDataStateEnum : int {
    Initial = 0,
    Initialized,
    DefaultData,
    NonDefaultData,
    Switching,
    ApTriggered,
    MpTriggered
};

enum PreferredDataEventEnum : int {
    SetPreferredDataModem = 0,
    InitializeSM,
    DDSSwitchInd,
    CurrentDDSInd,
    DDSSwitchTimerExpired,
    DDSSwitchApStarted,
    DDSSwitchCompleted,
    DDSSwitchMpStarted,
    TempDDSSwitchTimerExpired,
};

enum DDSSwitchTypeEnum : int {
     Permenant = 0,
     Temp,
};

struct PreferredDataEventType {
    PreferredDataEventEnum event;
    string getEventName() const;
    void *data;
};

struct PreferredDataInfo_t {
    int dds;
    int sub;
    int nextDds;
    bool isRilIpcNotifier;
    shared_ptr<Message> mPendingMessage;
    VoiceIndResp mVoiceCallInfo;
    dsd_dds_switch_type_enum_v01  switch_type;
};

enum class DDSSwitchRes : int {
   ERROR = -1,
   FAIL = 0,
   ALLOWED = 1,
};

struct DDSSwitchInfo_t {
    int subId;
    DDSSwitchRes switchAllowed;
    shared_ptr<Message> msg;
    dsd_dds_switch_type_enum_v01 switch_type;
};

class PreferredDataState : public IState<PreferredDataEventType> {
public:
    PreferredDataState(string name, function<void(int)> fn, shared_ptr<PreferredDataInfo_t> info);
    virtual void enter() {};
    virtual void exit() {};
    virtual int handleEvent(const PreferredDataEventType&) = 0;
    void ddsTransition();
    void sendResponse(std::shared_ptr<Message> m, SetPreferredDataModemResult_t error);
    TimeKeeper::timer_id tempDDSSwitchRequestTimer;
    bool TempddsSwitchRequestPending = false;

protected:
    shared_ptr<PreferredDataInfo_t> mStateInfo;
};

class InitialState : public PreferredDataState {
public:
    InitialState(function<void(int)> fn, shared_ptr<PreferredDataInfo_t> info);
    int handleEvent(const PreferredDataEventType& event) override;
};

class InitializedState : public PreferredDataState {
public:
    InitializedState(function<void(int)> fn, shared_ptr<PreferredDataInfo_t> info);
    int handleEvent(const PreferredDataEventType& event) override;
};

class SwitchingState : public PreferredDataState {
public:
    SwitchingState(function<void(int)> fn, shared_ptr<PreferredDataInfo_t> info);
    int handleEvent(const PreferredDataEventType& event) override;
};

class DefaultDataState : public PreferredDataState {
public:
    DefaultDataState(function<void(int)> fn, shared_ptr<PreferredDataInfo_t> info);
    int handleEvent(const PreferredDataEventType& event) override;
};

class NonDefaultDataState : public PreferredDataState {
public:
    NonDefaultDataState(function<void(int)> fn, shared_ptr<PreferredDataInfo_t> info);
    int handleEvent(const PreferredDataEventType& event) override;
};

class ApTriggeredState : public PreferredDataState {
public:
    ApTriggeredState(function<void(int)> fn, shared_ptr<PreferredDataInfo_t> info);
    void enter() override;
    void exit() override;
    int handleEvent(const PreferredDataEventType& event) override;
};

class MpTriggeredState : public PreferredDataState {
public:
    MpTriggeredState(function<void(int)> fn, shared_ptr<PreferredDataInfo_t> info);
    void enter() override;
    int handleEvent(const PreferredDataEventType& event) override;
};

}
