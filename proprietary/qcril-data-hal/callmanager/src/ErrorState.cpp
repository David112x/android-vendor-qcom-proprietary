/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <sstream>
#include "framework/Log.h"

#include "CallState.h"
#include "CallManager.h"

using namespace std;
using namespace SM;
using namespace rildata;

void ErrorState::enter(void) {
  Log::getInstance().d("[ErrorState] enter");
}

void ErrorState::exit(void) {
  Log::getInstance().d("[ErrorState] exit");
}

int ErrorState::handleEvent(const CallEventType &event) {
  Log::getInstance().d("[ErrorState] handleEvent event id = "
    + to_string((int)event.callEvent));
  logBuffer.addLogWithTimestamp("[SM]: " + getName() + " handling " + event.getEventName());

  int nextState = _eError;
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
      Log::getInstance().d("[ErrorState] DSI net event = "+
                        to_string((int)netEvent));
      break;

    default:
      Log::getInstance().d("[ErrorState] Unhandled call event = "+
                        to_string((int)event.callEvent));
      break;
  }

  return nextState;
}
