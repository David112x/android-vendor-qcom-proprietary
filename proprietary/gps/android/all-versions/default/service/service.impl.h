/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef LOC_HIDL_VERSION
#error "LOC_HIDL_VERSION must be set before including this file."
#endif

#include <hidl/LegacySupport.h>
#include <android/hardware/gnss/2.0/IGnss.h>

#include "loc_cfg.h"

using android::hardware::registerPassthroughServiceImplementation;
using android::status_t;
using android::OK;

using vendor::qti::gnss::LOC_HIDL_VERSION::ILocHidlGnss;

int main(int /* argc */, char* /* argv */ []) {

    ALOGI("%s", __FUNCTION__);

    status_t status = registerPassthroughServiceImplementation<ILocHidlGnss>("gnss_vendor");
    if (status != OK) {
        ALOGE("Error while registering gnss vendor hal service: %d", status);
        return -1;
    }

    return 0;
}
