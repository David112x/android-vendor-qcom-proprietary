/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "NestedStateMachine.h"
#include "PreferredDataStates.h"

namespace rildata {
using SM::NestedStateMachine;

class PreferredDataStateMachine : public NestedStateMachine<PreferredDataEventType> {
public:
    PreferredDataStateMachine();
    ~PreferredDataStateMachine();
    void initialize(shared_ptr<PreferredDataInfo_t> pInfo);
};

}
