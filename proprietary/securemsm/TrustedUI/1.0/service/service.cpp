/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#define LOG_TAG "trustedui@1.0-service-qti"

#include <android-base/logging.h>
#include <hidl/HidlTransportSupport.h>
#include "TrustedUIFactory.h"
#include "TrustedInput.h"

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::sp;

using vendor::qti::hardware::trustedui::V1_0::implementation::TrustedUIFactory;
using vendor::qti::hardware::trustedui::V1_0::implementation::TrustedInput;

int main() {
#ifdef ARCH_ARM_32
    android::hardware::ProcessState::initWithMmapSize((size_t)(2*PAGE_SIZE));
#endif
    configureRpcThreadpool(1, true /*willJoinThreadpool*/);
    auto trustedui = TrustedUIFactory::GetInstance();

    ALOGD("TrustedUI service starting.");
    auto status = trustedui->registerAsService();
    if (status != android::OK) {
        ALOGE("Could not register service for TrustedUI 1.0, status = 0x%x",
              status);
    }
    ALOGD("Registered TrustedUI HAL service success");

    /* Start ITrustedInput Service*/
    auto trustedInputDevice = new TrustedInput();

    ALOGD("TrustedInput service starting.");
    status = trustedInputDevice->registerAsService();
    if (status != android::OK) {
        ALOGE("Could not register service for TrustedInput 1.0, status = 0x%x",
                status);
    }
    ALOGD("Registered TrustedInput service success");

    joinRpcThreadpool();
    return -1;
}
