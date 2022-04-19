/*
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#define LOG_TAG "LocSvc_HIDL_vendor.qti.gnss@1.2-service"

#include <android/hardware/gnss/1.1/IGnss.h>
#include <vendor/qti/gnss/1.2/ILocHidlGnss.h>
#include <hidl/LegacySupport.h>
#include "loc_cfg.h"

using android::hardware::registerPassthroughServiceImplementation;
using android::status_t;
using android::OK;

using vendor::qti::gnss::V1_2::ILocHidlGnss;

int main(int /* argc */, char* /* argv */ []) {

    ALOGI("%s", __FUNCTION__);

    status_t status = registerPassthroughServiceImplementation<ILocHidlGnss>("gnss_vendor");
    if (status != OK) {
        ALOGE("Error while registering gnss vendor hal service: %d", status);
        return -1;
    }

    return 0;
}
