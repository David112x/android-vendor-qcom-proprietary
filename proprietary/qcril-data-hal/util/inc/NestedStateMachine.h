/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#pragma once

#include "BaseFiniteStateMachine.h"
#include <stack>
#include <unordered_set>

namespace SM {

template <class E>
class NestedStateMachine : public BaseFiniteStateMachine<E> {
public:
    NestedStateMachine();
    void setCurrentState(int stateId) override;
    int processEvent(const E& event) override;
    void setParentState(int stateId, int parentStateId);
    virtual void dump(string padding, ostream& os) override;

protected:
    int getParentState(int stateId);
    bool getActive(int stateId);

private:
    unordered_map<int, int> parentStateMap;
    unordered_set<int> activeSet;
};

template <class E>
NestedStateMachine<E>::NestedStateMachine():
    parentStateMap{}, activeSet{{BaseFiniteStateMachine<E>::NULL_STATE}} {}

template <class E>
void NestedStateMachine<E>::setCurrentState(int stateId) {
    std::stack<int> entryStack;

    int lcaState = stateId;
    if (stateId == BaseFiniteStateMachine<E>::getCurrentState()) {
        return;
    }
    // find the next state
    while (!getActive(lcaState)) {
        entryStack.push(lcaState);
        lcaState = getParentState(lcaState);
    }

    // at this point, we have the LCA of current and target states
    // perform exits up to the LCA
    int stateIterator = BaseFiniteStateMachine<E>::getCurrentState();
    auto stateIteratorPtr = BaseFiniteStateMachine<E>::getStateById(stateIterator);
    while (stateIterator != lcaState && stateIteratorPtr != nullptr) {
        stateIteratorPtr->exit();
        activeSet.erase(stateIterator);
        stateIterator = getParentState(stateIterator);
        stateIteratorPtr = BaseFiniteStateMachine<E>::getStateById(stateIterator);
    }
    // perform entries down to the target
    stateIteratorPtr = BaseFiniteStateMachine<E>::getStateById(stateIterator);
    while (!entryStack.empty()) {
        stateIterator = entryStack.top();
        stateIteratorPtr = BaseFiniteStateMachine<E>::getStateById(stateIterator);
        if (stateIteratorPtr != nullptr) {
            stateIteratorPtr->enter();
            activeSet.insert(stateIterator);
        }
        entryStack.pop();
    }
    BaseFiniteStateMachine<E>::currentState = stateId;
}

template <class E>
int NestedStateMachine<E>::processEvent(const E& event) {
    int status = IState<E>::UNHANDLED;
    int handlingState = BaseFiniteStateMachine<E>::getCurrentState();
    auto handlingStatePtr = BaseFiniteStateMachine<E>::getStateById(handlingState);
    while (handlingStatePtr != nullptr) {
        status = handlingStatePtr->handleEvent(event);
        SM_LOGD("[NestedStateMachine]: processEvent handled by " + handlingStatePtr->getName());
        if (status != IState<E>::UNHANDLED) {
            return status;
        }
        handlingState = getParentState(handlingState);
        handlingStatePtr = BaseFiniteStateMachine<E>::getStateById(handlingState);
    }
    SM_LOGE("[NestedStateMachine]: processEvent handled by null state");
    return IState<E>::HANDLED;
}

template <class E>
void NestedStateMachine<E>::setParentState(int stateId, int parentStateId) {
    SM_LOGD("[NestedStateMachine]: setParentState " + std::to_string(stateId) + " " + std::to_string(parentStateId));
    if (parentStateMap.find(stateId) != parentStateMap.end()) {
        parentStateMap.at(stateId) = parentStateId;
    } else {
    parentStateMap.insert({stateId, parentStateId});
    }
}

template <class E>
void NestedStateMachine<E>::dump(string padding, ostream& os) {
    unordered_multimap<int, int> tree;
    for (auto it = parentStateMap.begin(); it != parentStateMap.end(); it++) {
        if (it->first != it->second) {
            tree.insert({it->second, it->first});
        }
    }

    int root = -1;
    for (auto it = BaseFiniteStateMachine<E>::stateMap.begin(); it != BaseFiniteStateMachine<E>::stateMap.end(); it++) {
        if (parentStateMap.find(it->first) == parentStateMap.end()) {
            root = it->first;
        }
    }

    // Print HSM hierarchy by performing a dfs
    // Stack contains pair<state, depth>
    // Depth tells how many leading tabs are needed (layers of nesting)
    stack<pair<int, int>> dfsStack;
    dfsStack.push({root, 0});
    os << padding << "HSM:" << endl;

    while (!dfsStack.empty()) {
        auto next = dfsStack.top();
        dfsStack.pop();

        // Generate the log message for the current State
        string fmt_state = "";
        // Add tabspaces to show nested layers
        for (int i = 1; i < next.second; i++) {
            fmt_state += "    ";
        }

        // Do not print relationship symbol for null state
        if ( next.second > 0 ) {
            fmt_state += " -> ";
        }
        fmt_state += BaseFiniteStateMachine<E>::getStateById(next.first)->getName();

        // Append active and current state symbols
        if (getActive(next.first)) {
            fmt_state += " [*]";
        }
        if (next.first == BaseFiniteStateMachine<E>::getCurrentState()) {
            fmt_state += " [current state]";
        }
        os << padding << fmt_state.c_str() << endl;

        // Add children to tree, increment depth by 1
        auto range = tree.equal_range(next.first);
        for (auto it = range.first; it != range.second; it++) {
            dfsStack.push({it->second, next.second + 1});
        }
    }
}

template <class E>
int NestedStateMachine<E>::getParentState(int stateId) {
    auto search = parentStateMap.find(stateId);
    if (search != parentStateMap.end()) {
        return search->second;
    }
    SM_LOGE("[NestedStateMachine]: no parent state");
    return BaseFiniteStateMachine<E>::NULL_STATE;
}

template <class E>
bool NestedStateMachine<E>::getActive(int stateId) {
    return (activeSet.find(stateId) != activeSet.end());
}

}