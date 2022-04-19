/*!
 *
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#pragma once

#include <vendor/qti/spu/1.0/ISpcomSSREvent.h>

#include "SPUHidlClientTest.h"

using namespace std;

using android::sp;
using ::android::hardware::Void;
using ::android::hardware::Return;
using vendor::qti::spu::V1_0::ISpcomSSREvent;

class SPUHidlClientTest;

struct SpcomSSREvent : public ISpcomSSREvent {

public:

    explicit SpcomSSREvent(SPUHidlClientTest* testFw);
    virtual ~SpcomSSREvent();

    Return<void> callback() override;

private:

    static SPUHidlClientTest* sTestFw;
};
