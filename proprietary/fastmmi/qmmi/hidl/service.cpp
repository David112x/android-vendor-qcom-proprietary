/*
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#include <vendor/qti/hardware/factory/1.1/IFactory.h>
#include <hidl/LegacySupport.h>
#include <binder/ProcessState.h>

#include "Factory.h"

using vendor::qti::hardware::factory::V1_1::IFactory;
using vendor::qti::hardware::factory::V1_0::implementation::Factory;

using android::sp;
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::status_t;
using android::OK;

int main() {
    android::sp<IFactory> service = new Factory();
    ALOGI("start to register Factory HAL service");
    configureRpcThreadpool(1, true /*callerWillJoin*/);
    if (service->registerAsService() != OK) {
        ALOGE("Can not register Factory HAL service");
        return -1;
    }

    ALOGI("registerd Factory HAL service");
    joinRpcThreadpool();

    return 1; // should never get here
}
