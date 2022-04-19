/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#define LOG_TAG "vendor.qti.secureelement@1.0-service"

#include <android/hardware/secure_element/1.0/ISecureElement.h>
#include <hidl/LegacySupport.h>
#include "eSEClient.h"
#include "SecureElement.h"

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;

using android::hardware::secure_element::V1_0::ISecureElement;
using android::hardware::secure_element::V1_0::implementation::SecureElement;
using android::sp;
using android::OK;

ese_update_state_t ese_update = ESE_UPDATE_COMPLETED;

int main() {
    ALOGE("vendor::qti::secure_element service starts");
    android::status_t status;

    configureRpcThreadpool(10, true /*callerWillJoin*/);

#if(ESE_CLIENT_LIB_PRESENT == true)
    checkEseClientUpdate();
#endif

    sp<ISecureElement> se_service = new SecureElement();
    status = se_service->registerAsService("eSE1");

    LOG_ALWAYS_FATAL_IF(status != OK, "Error while registering secure_element service: %d", status);

#if(ESE_CLIENT_LIB_PRESENT == true)
    perform_eSEClientUpdate();
#endif

    joinRpcThreadpool();
    return status;
}
