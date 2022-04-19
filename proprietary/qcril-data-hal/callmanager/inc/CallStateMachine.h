/*===========================================================================

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#pragma once

#include <string>

#include "CallState.h"
#include "BaseFiniteStateMachine.h"
#include "LocalLogBuffer.h"

using namespace std;
using namespace SM;

namespace rildata {

struct CallEventType;
class CallInfo;
/**
 * @brief CallStateMachine class
 * @details CallStateMachine class
 */
class CallStateMachine : public BaseFiniteStateMachine<CallEventType> {
private:
  LocalLogBuffer& logBuffer;
public:
  CallStateMachine(CallInfo& pCallInfo, LocalLogBuffer& logBuffer);
  ~CallStateMachine();
  void setCurrentState(int stateId) override;
};

} /* namespace rildata */
