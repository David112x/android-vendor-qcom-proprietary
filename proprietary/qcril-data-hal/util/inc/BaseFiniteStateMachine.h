/*===========================================================================

  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/
#pragma once

#include <string>
#include <unordered_map>
#include <memory>
#include <ostream>

#include "IState.h"
#include "SMLog.h"

using namespace std;

namespace SM {

template <class E>
class BaseFiniteStateMachine {
public:
  static constexpr int NULL_STATE = -1;
  BaseFiniteStateMachine();
  virtual ~BaseFiniteStateMachine() {
    SM_LOGD("[BaseFiniteStateMachine]: ~BaseFiniteStateMachine");
    for (auto it = stateMap.begin(); it != stateMap.end(); ++it ) {
      SM_LOGD("[BaseFiniteStateMachine]: deleting " + std::to_string(it->first));
      (it->second).reset();
    }
  }

  void addState(int stateId, shared_ptr<IState<E>> state);
  int getCurrentState();
  virtual void setCurrentState(int stateId);
  virtual int processEvent(const E& event);
  virtual void dump(string padding, ostream& os);

protected:
  shared_ptr<IState<E>> getStateById(int stateId);
  unordered_map<int, shared_ptr<IState<E>>> stateMap;
  int currentState;

private:
};

template<class E>
BaseFiniteStateMachine<E>::BaseFiniteStateMachine() : currentState(NULL_STATE) {}

template<class E>
void BaseFiniteStateMachine<E>::addState(int stateId, shared_ptr<IState<E>> state) {
  if (stateId != NULL_STATE) {
  stateMap.insert({stateId, state});
  }
}

template<class E>
void BaseFiniteStateMachine<E>::setCurrentState(int stateId) {
  auto currState = getStateById(currentState);
  auto nextState = getStateById(stateId);
  SM_LOGD("[BaseFiniteStateMachine]: setCurrentState = " +
          ((nextState == nullptr) ? "NULL" : nextState->getName()));
  if (currentState != stateId) {
    if (currState != nullptr) {
      currState->exit();
    }
    currentState = stateId;
    if (nextState != nullptr) {
      nextState->enter();
    }
  } else {
    SM_LOGD("[BaseFiniteStateMachine]: already in target state");
  }
}

template<class E>
int BaseFiniteStateMachine<E>::getCurrentState() {
  return currentState;
}

template<class E>
int BaseFiniteStateMachine<E>::processEvent(const E& event) {
  int ret = -1;
  auto search = stateMap.find(currentState);
  if (search!=stateMap.end()) {
    ret = search->second->handleEvent(event);
  }
  return ret;
}

template<class E>
shared_ptr<IState<E>> BaseFiniteStateMachine<E>::getStateById(int stateId) {
  shared_ptr<IState<E>> ret = nullptr;
  auto search = stateMap.find(stateId);
  if (search != stateMap.end()) {
    ret = search->second;
  }
  return ret;
}

template<class E>
void BaseFiniteStateMachine<E>::dump(string padding, ostream& os) {
  auto currState = getStateById(currentState);
  os << padding << "state=" <<
        (currState == nullptr ? "NULL" : currState->getName()) << endl;
}

} /* namespace SM */
