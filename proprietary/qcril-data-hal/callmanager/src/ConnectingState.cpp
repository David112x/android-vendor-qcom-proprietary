/******************************************************************************
#  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <sstream>
#include "framework/Log.h"

#include "CallState.h"
#include "CallManager.h"
#include "UnSolMessages/DataCallTimerExpiredMessage.h"
#include "UnSolMessages/CleanDataCallStateMessage.h"

using namespace std;
using namespace SM;
using namespace rildata;

void ConnectingState::enter(void) {
  Log::getInstance().d("[ConnectingState] enter");
  using std::placeholders::_1;
  auto timeoutHandler = bind(&ConnectingState::timeoutHandler, this, _1);
  stopTimer();
  timer = TimeKeeper::getInstance().set_timer(timeoutHandler, nullptr, ConnectingState::TIMEOUT_MILLIS);
}

void ConnectingState::exit(void) {
  Log::getInstance().d("[ConnectingState] exit");
  stopTimer();
  auto callCleanupMsg = std::make_shared<CleanDataCallStateMessage>(callInfo.getCid());
  callCleanupMsg->broadcast();
}

int ConnectingState::handleEvent(const CallEventType &event){
  Log::getInstance().d("[ConnectingState] handleEvent event id = "
    + to_string((int)event.callEvent));
  logBuffer.addLogWithTimestamp("[SM]: " + getName() + " handling " + event.getEventName());

  int nextState = _eConnecting;
  CallInfo *pCallInstance = nullptr;
  DsiEventDataType *dsiEvent = nullptr;
  dsi_net_evt_t netEvent = DSI_EVT_INVALID;

  if (event.callEvent == _eDsiEvent) {
    dsiEvent = (DsiEventDataType *)event.callEventData;
    netEvent = dsiEvent->netEvent;
    pCallInstance = (CallInfo *)dsiEvent->eventData;
  }
  else {
    pCallInstance = (CallInfo *)event.callEventData;
  }

  switch (event.callEvent) {
    case _eConnectTimerExpiredEvent:
      stopTimer();
      nextState = _eError;
      break;
    case _eAbortEvent:
      stopTimer();
      dsiStopDataCall(pCallInstance->getDsiHandle());
      pCallInstance->setupDataCallFailResponse(DataCallFailCause_t::ERROR_UNSPECIFIED);
      nextState = _eDisconnecting;
      break;
    case _eDisconnectEvent:
      stopTimer();
      dsiStopDataCall(pCallInstance->getDsiHandle());
      nextState = _eDisconnecting;
      break;

    case  _eDsiEvent:
      Log::getInstance().d("[ConnectingState] handleEvent net event = "
        + to_string((int)dsiEvent->netEvent));

      switch (netEvent) {
        case DSI_EVT_NET_IS_CONN:
          pCallInstance->setLinkActiveState(_eActivePhysicalLinkUp);
          pCallInstance->configureParams(false);
          nextState = _eConnected;
          break;

        case DSI_EVT_NET_NO_NET:
          stopTimer();
          nextState = _eError;
          break;

        default:
          Log::getInstance().d("[ConnectingState] Unhandled DSI net event = "+to_string((int)netEvent));
          break;
      }
      break;

    default:
      Log::getInstance().d("[ConnectingState] Unhandled call event = "+to_string((int)event.callEvent));
      break;
  }

  return nextState;
}

void ConnectingState::timeoutHandler(void *data) {
  std::ignore = data;
  auto timeoutMsg = std::make_shared<DataCallTimerExpiredMessage>(callInfo.getCid(), DataCallTimerType::SetupDataCall);
  if (timeoutMsg != nullptr) {
    Log::getInstance().d("[ConnectingState]: Broadcasting setup data call timer expired");
    timeoutMsg->broadcast();
  }
}
