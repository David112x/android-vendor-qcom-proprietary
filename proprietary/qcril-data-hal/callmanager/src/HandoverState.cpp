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

using namespace std;
using namespace SM;
using namespace rildata;

void HandoverState::enter(void) {
  Log::getInstance().d("[HandoverState] enter");
  subState = _eStarted;
  using std::placeholders::_1;
  auto timeoutHandler = bind(&HandoverState::timeoutHandler, this, _1);
  stopTimer();
  timer = TimeKeeper::getInstance().set_timer(timeoutHandler, nullptr, HandoverState::TIMEOUT_MILLIS);
  //There is no need to ack a pending intent if HO has already started
  //Hence clear any pending intent
  callInfo.setIntentToChangeInProgress(false);
}

void HandoverState::exit(void) {
  Log::getInstance().d("[HandoverState] exit");
  stopTimer();
}

int HandoverState::handleEvent(const CallEventType &event) {
  Log::getInstance().d("[HandoverState] handleEvent event id = "
    + to_string((int)event.callEvent));
  logBuffer.addLogWithTimestamp("[SM]: " + getName() + " handling " + event.getEventName());

  int nextState = _eHandover;
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

  switch(event.callEvent) {
    case _eDsiEvent:
      Log::getInstance().d("[HandoverState] DSI net event id ="+ to_string((int)netEvent));
      switch (netEvent) {
        case DSI_EVT_NET_NO_NET:
          nextState = _eError;
          break;

        case DSI_EVT_NET_HANDOFF:
          break;

        default:
          break;
      }
      break;

    case _eHandoverDeactivateDataEvent:
      pCallInstance->deactivateDataCallResponse();
      break;

    case _eHandoverApAsstPrefSysResultEvent:
      subState = _eApAsstPrefSysResultReceived;
      break;

    case _eHandoverInformationInitEvent:
      subState = _eInfoInitReceived;
      break;

    case _eHandoverApAsstPrefSysResultFailureEvent:
    case _eHandoverInformationFailureEvent:
    case _eHandoverTimerExpiredEvent:
    case _eHandoverPreferenceUnchangedEvent:
      pCallInstance->setPreferredNetworkType(pCallInstance->getCurrentNetworkType());
      subState = _eFinished;
      nextState = _eConnected;
      break;

    case _eHandoverInformationSuccessEvent:
    {
      bool dual_ip = (pCallInstance->IsV4Connected() && pCallInstance->IsV6Connected());
      bool dual_handover_done = dual_ip && pCallInstance->IsV4HandedOver() && pCallInstance->IsV6HandedOver();
      if(!dual_ip || dual_handover_done) {
        // Perform bitwise| to set network type to both current and preferred
        HandoffNetworkType_t target = static_cast<HandoffNetworkType_t>(
            pCallInstance->getPreferredNetworkType() | pCallInstance->getCurrentNetworkType());
        pCallInstance->setCurrentNetworkType(target);
        subState = _eFinished;
        nextState = _eConnected;
      }
      break;
    }

    case _eDisconnectEvent:
      Log::getInstance().d("[HandoverState] disconnecting");
      dsiStopDataCall(pCallInstance->getDsiHandle());
      nextState = _eDisconnecting;
      break;

    default:
      Log::getInstance().d("[HandoverState] Unhandled call event = " + std::to_string(event.callEvent));
      break;
  }

  return nextState;
}

void HandoverState::timeoutHandler(void *data) {
  std::ignore = data;
  auto timeoutMsg = std::make_shared<DataCallTimerExpiredMessage>(callInfo.getCid(), DataCallTimerType::Handover);
  if (timeoutMsg != nullptr) {
    Log::getInstance().d("[HandoverState]: Broadcasting handover timer expired");
    timeoutMsg->broadcast();
  }
}
