/*===========================================================================

  Copyright (c) 2018-19 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#pragma once

#include <string>

#include "CallInfo.h"
#include "IState.h"
#include "LocalLogBuffer.h"
#include "framework/TimeKeeper.h"

#define SETUP_DATA_CALL_TIMEOUT_SECONDS 155
#define DEACTIVATE_DATA_CALL_TIMEOUT_SECONDS 90
#define HANDOVER_TIMEOUT_SECONDS 60
#define DEFAULT_PARTIAL_RETRY_TIMEOUT 5
#define DEFAULT_MAX_PARTIAL_RETRY_TIMEOUT 100

using namespace std;
using namespace SM;

namespace rildata {

class CallInfo;

enum CallStateEnum {
  _eIdle = 0,
  _eConnecting,
  _eConnected,
  _eHandover,
  _eDisconnecting,
  _eError
};

enum CallEventTypeEnum {
  _eConnectEvent = 0,
  _eConnectFailEvent,
  _eConnectTimerExpiredEvent,
  _eDisconnectEvent,
  _eDisconnectFailEvent,
  _eDisconnectTimerExpiredEvent,
  _eHandoverSetupDataEvent,
  _eHandoverApAsstPrefSysResultEvent,
  _eHandoverApAsstPrefSysResultFailureEvent,
  _eHandoverInformationInitEvent,
  _eHandoverInformationSuccessEvent,
  _eHandoverInformationFailureEvent,
  _eHandoverDeactivateDataEvent,
  _eHandoverTimerExpiredEvent,
  _eHandoverPreferenceUnchangedEvent,
  _ePartialRetryTimerExpiredEvent,
  _ePartialRetryResponseTimerExpiredEvent,
  _eAbortEvent,
  _eDsiEvent,
  _eCallEvtEvent
};

/**
 * @brief call event
 * @details call event to be processed by call state machine
 */
struct CallEventType {
  CallEventTypeEnum callEvent;
  void * callEventData;
  std::string getEventName() const;
};

/**
 * @brief CallState class
 * @details CallState class
 *
 * @param setName Call state name
 */
class CallState : public IState<CallEventType> {
protected:
  CallInfo& callInfo;
  LocalLogBuffer& logBuffer;
  TimeKeeper::timer_id timer;

  virtual void timeoutHandler(void *) {};
  void stopTimer() {
    if (timer != TimeKeeper::no_timer) {
      TimeKeeper::getInstance().clear_timer(timer);
      timer = TimeKeeper::no_timer;
    }
  }
public:
  CallState(string setName, CallInfo& setCallInfo, LocalLogBuffer& setLogBuffer):
    IState(setName), callInfo(setCallInfo),
    logBuffer(setLogBuffer), timer(TimeKeeper::no_timer) {};
  virtual ~CallState() = default;
};

class IdleState : public CallState
{
public:
  IdleState(CallInfo& setCallInfo, LocalLogBuffer& setLogBuffer):
    CallState("IdleState", setCallInfo, setLogBuffer){}
  virtual void enter();
  virtual void exit();
  virtual int handleEvent(const CallEventType &event);
};

class ConnectingState : public CallState
{
private:
  virtual void timeoutHandler(void *) override;
public:
  static constexpr TimeKeeper::millisec TIMEOUT_MILLIS = SETUP_DATA_CALL_TIMEOUT_SECONDS * 1000;
  ConnectingState(CallInfo& setCallInfo, LocalLogBuffer& setLogBuffer):
    CallState("ConnectingState", setCallInfo, setLogBuffer){}
  virtual void enter();
  virtual void exit();
  virtual int handleEvent(const CallEventType &event);
};

class ConnectedState : public CallState {
private:
  TimeKeeper::timer_id dsiResponseTimer;
  int retryCount;

  virtual void timeoutHandler(void *) override;
  void stopDsiResponseTimer();
  void dsiResponseTimeoutHandler(void *);
  TimeKeeper::millisec getTimeoutMillis(unsigned long maxTimeoutMillis);
  void startPartialRetryTimer(CallInfo *pCallInstance);
  void startDsiResponseTimer();
  bool isPartialRetryAllowed();
public:
  static constexpr TimeKeeper::millisec DSI_RESPONSE_TIMEOUT_MILLIS = SETUP_DATA_CALL_TIMEOUT_SECONDS * 1000;
  ConnectedState(CallInfo& setCallInfo, LocalLogBuffer& setLogBuffer):
    CallState("ConnectedState", setCallInfo, setLogBuffer),
    dsiResponseTimer(TimeKeeper::no_timer), retryCount(0) {}
  virtual void enter();
  virtual void exit();
  virtual int handleEvent(const CallEventType &event);
};

class HandoverState : public CallState {

enum HandoverSubStateEnum {
  _eStarted = 0,
  _eApAsstPrefSysResultReceived,
  _eInfoInitReceived,
  _eInfoCompleteReceived,
  _eFinished
};

private:
  HandoverSubStateEnum subState;
  virtual void timeoutHandler(void *) override;
public:
  static constexpr TimeKeeper::millisec TIMEOUT_MILLIS = HANDOVER_TIMEOUT_SECONDS * 1000;
  HandoverState(CallInfo& setCallInfo, LocalLogBuffer& setLogBuffer):
    CallState("HandoverState", setCallInfo, setLogBuffer){}
  virtual void enter();
  virtual void exit();
  virtual int handleEvent(const CallEventType &event);
};

class DisconnectingState : public CallState {
private:
  virtual void timeoutHandler(void *) override;
public:
  static constexpr TimeKeeper::millisec TIMEOUT_MILLIS = DEACTIVATE_DATA_CALL_TIMEOUT_SECONDS *1000;
  DisconnectingState(CallInfo& setCallInfo, LocalLogBuffer& setLogBuffer):
    CallState("DisconnectingState", setCallInfo, setLogBuffer){}
  virtual void enter();
  virtual void exit();
  virtual int handleEvent(const CallEventType &event);
};

class ErrorState : public CallState {
public:
  ErrorState(CallInfo& setCallInfo, LocalLogBuffer& setLogBuffer):
    CallState("ErrorState", setCallInfo, setLogBuffer){}
  virtual void enter();
  virtual void exit();
  virtual int handleEvent(const CallEventType &event);
};

} /* namespace rildata */
