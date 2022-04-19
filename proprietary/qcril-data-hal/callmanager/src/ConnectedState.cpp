/******************************************************************************
#  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#include <sstream>
#include "framework/Log.h"
#include "modules/qmi/ModemEndPointFactory.h"
#include "UnSolMessages/DataCallTimerExpiredMessage.h"
#include "UnSolMessages/CleanDataCallStateMessage.h"

#include "CallInfo.h"
#include "CallState.h"
#include "CallManager.h"

#include "DSDModemEndPoint.h"

using namespace std;
using namespace SM;
using namespace rildata;

void ConnectedState::enter(void) {
  Log::getInstance().d("[ConnectedState] enter");
  retryCount = 0;
}

void ConnectedState::exit(void) {
  Log::getInstance().d("[ConnectedState] exit");
  //Stop any timers if still running
  stopTimer();
  stopDsiResponseTimer();
  auto callCleanupMsg = std::make_shared<CleanDataCallStateMessage>(callInfo.getCid());
  callCleanupMsg->broadcast();
}

TimeKeeper::millisec ConnectedState::getTimeoutMillis(unsigned long maxTimeoutMillis) {
  // Timeout value doubles after each retry until MAX is reached
  TimeKeeper::millisec defaultTimeoutMillis = (TimeKeeper::millisec)DEFAULT_PARTIAL_RETRY_TIMEOUT * 1000;
  TimeKeeper::millisec timeoutMillis = defaultTimeoutMillis << retryCount;
  // handle integer overflow
  if (timeoutMillis <= 0 || timeoutMillis > maxTimeoutMillis) {
    timeoutMillis = maxTimeoutMillis;
  }
  return timeoutMillis;
}

void ConnectedState::stopDsiResponseTimer() {
  if (dsiResponseTimer != TimeKeeper::no_timer) {
    TimeKeeper::getInstance().clear_timer(dsiResponseTimer);
    dsiResponseTimer = TimeKeeper::no_timer;
  }
}

int ConnectedState::handleEvent(const CallEventType &event) {
  Log::getInstance().d("[ConnectedState] handleEvent event id = "
    + to_string((int)event.callEvent));
  logBuffer.addLogWithTimestamp("[SM]: " + getName() + " handling " + event.getEventName());
  int nextState = _eConnected;
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
    case _eDisconnectEvent:
      Log::getInstance().d("[ConnectedState] disconnecting");
      dsiStopDataCall(pCallInstance->getDsiHandle());
      nextState = _eDisconnecting;
      break;
    case _eHandoverSetupDataEvent:
      Log::getInstance().d("[ConnectedState] doing handover");
      do {
        string apnName("");
        if(pCallInstance->getProfileInfo()->getIsEmergencyCall()) {
          apnName = pCallInstance->getProfileInfo()->getEmergencyCallApn();
        }
        else {
          apnName = pCallInstance->getProfileInfo()->getApn();
        }
        HandoffNetworkType_t prefNetwork = pCallInstance->getPreferredNetworkType();
        CallManager::sendSetApnPreferredSystemRequest(apnName, prefNetwork);
        pCallInstance->setIntentToChangeInProgress(false);
      }while(0);
      nextState = _eHandover;
      break;

    case _eHandoverDeactivateDataEvent:
    {
      pCallInstance->deactivateDataCallResponse();
      // Perform bitwise& to set network type to preferred only
      HandoffNetworkType_t target = static_cast<HandoffNetworkType_t>(
          pCallInstance->getPreferredNetworkType() & pCallInstance->getCurrentNetworkType());
      pCallInstance->setCurrentNetworkType(target);
      nextState = _eConnected;
      break;
    }

    case _ePartialRetryTimerExpiredEvent:
      // this event will only come after temporary failure
      if (DSI_SUCCESS != dsi_start_data_call(pCallInstance->getDsiHandle())) {
        Log::getInstance().d("[ConnectedState] dsi_start_data_call failed");
      }
      startDsiResponseTimer();
      retryCount++;
      nextState = _eConnected;
      break;

    case _ePartialRetryResponseTimerExpiredEvent:
      // in case of no partial retry setup_data_call response
      startPartialRetryTimer(pCallInstance);
      nextState = _eConnected;
      break;

     case _eHandoverInformationInitEvent:
      //Handover triggered by modem while still in Connected state. This could
      //be due to some mismatch between AP and modem, or because we acked the
      //intent when attach is in progress
      Log::getInstance().d("[ConnectedState] HO init received in ConnectedState, move to HO state");
      nextState = _eHandover;
      break;

    case _eDsiEvent:
      switch(netEvent) {
        case DSI_EVT_NET_PARTIAL_CONN:
          // partial retry failure case
          startPartialRetryTimer(pCallInstance);
          nextState = _eConnected;
          break;

        case DSI_EVT_NET_IS_CONN:
          stopDsiResponseTimer();
          pCallInstance->setLinkActiveState(_eActivePhysicalLinkUp);
          pCallInstance->configureParams(true);
          // if dual ip call but not all IPs have connected
          if (pCallInstance->IsV4V6() &&
              (!pCallInstance->IsV4Connected() || !pCallInstance->IsV6Connected())) {
            startPartialRetryTimer(pCallInstance);
          }
          nextState = _eConnected;
          break;

        case DSI_EVT_NET_NEWADDR:
        case DSI_EVT_NET_RECONFIGURED:
          // once both IPs have connected, stop partial retry attempts
          pCallInstance->setLinkActiveState(_eActivePhysicalLinkUp);
          pCallInstance->configureParams(true);
          if (pCallInstance->IsV4V6() &&
              (pCallInstance->IsV4Connected() && pCallInstance->IsV6Connected())) {
            Log::getInstance().d("[ConnectedState] Dual ip connection established");
            stopTimer();
            stopDsiResponseTimer();
          }
          nextState = _eConnected;
          break;

        case DSI_EVT_NET_NO_NET:
          Log::getInstance().d("[ConnectedState] DSI_EVT_NET_NO_NET is delivered");
          nextState = _eError;
          break;

        case DSI_EVT_NET_DELADDR:
          pCallInstance->configureParams(true);
          startPartialRetryTimer(pCallInstance);
          nextState = _eConnected;
          break;

        default:
          break;
      }
      break;

    default:
      Log::getInstance().d("[ConnectedState] unhandled event = " + std::to_string(event.callEvent));
      break;
  }

  return nextState;
}

void ConnectedState::startPartialRetryTimer(CallInfo *pCallInstance) {
  using std::placeholders::_1;
  if (!pCallInstance->IsPartialRetryEnabled()) {
    Log::getInstance().d("[ConnectedState]: partial retry is not enabled");
    return;
  }
  auto handler = bind(&ConnectedState::timeoutHandler, this, _1);
  stopTimer();
  auto timeoutMillis = getTimeoutMillis(pCallInstance->getMaxPartialRetryTimeout() * 1000);
  Log::getInstance().d("[ConnectedState]: partial retry timer started for " +
                       std::to_string((unsigned long)timeoutMillis) + "ms");
  timer = TimeKeeper::getInstance().set_timer(handler, nullptr, timeoutMillis);
}

void ConnectedState::startDsiResponseTimer() {
  using std::placeholders::_1;
  auto handler = bind(&ConnectedState::dsiResponseTimeoutHandler, this, _1);
  stopDsiResponseTimer();
  dsiResponseTimer = TimeKeeper::getInstance().set_timer(handler, nullptr, DSI_RESPONSE_TIMEOUT_MILLIS);
}

void ConnectedState::timeoutHandler(void *data) {
  std::ignore = data;
  auto timeoutMsg = std::make_shared<DataCallTimerExpiredMessage>(callInfo.getCid(), DataCallTimerType::PartialRetry);
  if (timeoutMsg != nullptr) {
    Log::getInstance().d("[ConnectedState]: Broadcasting partial retry timer expired");
    timeoutMsg->broadcast();
  }
}

void ConnectedState::dsiResponseTimeoutHandler(void *data) {
  std::ignore = data;
  auto timeoutMsg = std::make_shared<DataCallTimerExpiredMessage>(callInfo.getCid(), DataCallTimerType::PartialRetryResponse);
  if (timeoutMsg != nullptr) {
    Log::getInstance().d("[ConnectedState]: Broadcasting partial retry timer expired");
    timeoutMsg->broadcast();
  }
}
