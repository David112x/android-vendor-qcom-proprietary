/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "PreferredDataStateMachine.h"

using namespace rildata;

PreferredDataStateMachine::PreferredDataStateMachine() {}
PreferredDataStateMachine::~PreferredDataStateMachine() {}

/**
 * @brief
 * Constructs the state machine for handling SetPreferredDataModem.
 * Hierarchy:
 * Initial
 *  -> Switching
 *      -> MpTriggered
 *      -> ApTriggered
 *  -> Initialized
 *      -> NonDefaultData
 *      -> DefaultData
*/
void PreferredDataStateMachine::initialize(shared_ptr<PreferredDataInfo_t> pInfo) {
    auto transitionFn = bind(&PreferredDataStateMachine::setCurrentState, this, placeholders::_1);
    addState(Initial, make_shared<InitialState>(transitionFn, pInfo));
    addState(Initialized, make_shared<InitializedState>(transitionFn, pInfo));
    addState(DefaultData, make_shared<DefaultDataState>(transitionFn, pInfo));
    addState(NonDefaultData, make_shared<NonDefaultDataState>(transitionFn, pInfo));
    addState(Switching, make_shared<SwitchingState>(transitionFn, pInfo));
    addState(ApTriggered, make_shared<ApTriggeredState>(transitionFn, pInfo));
    addState(MpTriggered, make_shared<MpTriggeredState>(transitionFn, pInfo));
    setParentState(Initialized, Initial);
    setParentState(Switching, Initial);
    setParentState(DefaultData, Initialized);
    setParentState(NonDefaultData, Initialized);
    setParentState(ApTriggered, Switching);
    setParentState(MpTriggered, Switching);
}
