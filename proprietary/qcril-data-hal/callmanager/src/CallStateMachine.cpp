/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include "framework/Log.h"

#include "CallState.h"
#include "CallStateMachine.h"

using namespace rildata;

CallStateMachine::CallStateMachine(CallInfo& setCallInfo, LocalLogBuffer& setLogBuffer): logBuffer(setLogBuffer) {
  Log::getInstance().d("[CallStateMachine]: CallStateMachine");

  shared_ptr<IdleState> IdleStateInstance(new IdleState(setCallInfo, logBuffer));
  shared_ptr<ConnectingState> ConnectingStateInstance(new ConnectingState(setCallInfo, logBuffer));
  shared_ptr<ConnectedState> ConnectedStateInstance(new ConnectedState(setCallInfo, logBuffer));
  shared_ptr<HandoverState> HandoverStateInstance(new HandoverState(setCallInfo, logBuffer));
  shared_ptr<DisconnectingState> DisconnectingStateInstance(new DisconnectingState(setCallInfo, logBuffer));
  shared_ptr<ErrorState> ErrorStateInstance(new ErrorState(setCallInfo, logBuffer));

  addState(_eIdle, IdleStateInstance);
  addState(_eConnecting, ConnectingStateInstance);
  addState(_eConnected, ConnectedStateInstance);
  addState(_eHandover, HandoverStateInstance);
  addState(_eDisconnecting, DisconnectingStateInstance);
  addState(_eError, ErrorStateInstance);
}

CallStateMachine::~CallStateMachine() {
  Log::getInstance().d("[CallStateMachine]: ~CallStateMachine");
}

/**
 * CallStateMachine::setCurrentState()
 *
 * @brief
 * Wrapper around BaseFiniteStateMachine::setCurrentState to add log message
 * for every state transition
 */
void CallStateMachine::setCurrentState(int stateId) {
  int currentId = BaseFiniteStateMachine<CallEventType>::getCurrentState();
  if (currentId != NULL_STATE && currentId != stateId) {
    auto nextState = getStateById(stateId);
    string stateName = (nextState == nullptr ? "NULL" : nextState->getName());
    logBuffer.addLogWithTimestamp("[SM]: transitioning to " + stateName);
  }
  BaseFiniteStateMachine<CallEventType>::setCurrentState(stateId);
}