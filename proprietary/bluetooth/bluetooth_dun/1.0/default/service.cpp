/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <sys/types.h>
#include <sys/stat.h>
#include <hidl/LegacySupport.h>
#include <hwbinder/ProcessState.h>
#include <vendor/qti/hardware/bluetooth_dun/1.0/IBluetoothDunServer.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "vendor.qti.hardware.bluetooth_dun@1.0-service"


using android::hardware::defaultPassthroughServiceImplementation;
using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::hardware::registerPassthroughServiceImplementation;
using android::OK;
using vendor::qti::hardware::bluetooth_dun::V1_0::IBluetoothDunServer;
using vendor::qti::hardware::bluetooth_dun::V1_0::IBluetoothDunServerResponse;

#ifdef QTI_VND_FWK_DETECT_SUPPORTED
#ifdef __cplusplus
extern "C" {
#include "vndfwk-detect.h"
}
#endif
#endif

int main() {

    //Initialize Mmap size the moment the process starts
    android::hardware::ProcessState::initWithMmapSize((size_t)(256144));

    configureRpcThreadpool(1, true /*callerWillJoin*/);

    bool isVendorEnhancedFramework;

#ifdef QTI_VND_FWK_DETECT_SUPPORTED
    /* 0: Pure AOSP for both system and odm
       1: Pure AOSP for system and QC Value-adds for odm
       2: QC value-adds for system and Pure AOSP for odm
       3: QC value-adds for both system and odm
    */
    int vendorEnhancedInfo = getVendorEnhancedInfo();
    ALOGI("vendorEnhancedInfo: %d", vendorEnhancedInfo);
    isVendorEnhancedFramework = (vendorEnhancedInfo & 1) ? true : false;
#else
    isVendorEnhancedFramework = false;
#endif

    ALOGI("isVendorEnhancedFramework: %d", isVendorEnhancedFramework);
    ALOGI("Registering BT DUN Service");

    android::status_t status = OK;

    if (isVendorEnhancedFramework) {
        status = registerPassthroughServiceImplementation<IBluetoothDunServer>();

        if (status != OK)
            ALOGI("Error while registering BT service: %d", status);
    }

    joinRpcThreadpool();

    return status;
}
