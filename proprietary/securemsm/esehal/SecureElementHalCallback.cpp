/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "SecureElementHalCallback.h"

namespace android {
namespace hardware {
namespace secure_element {
namespace V1_0 {
namespace implementation {

// Methods from ::android::hardware::secure_element::V1_0::ISecureElementHalCallback follow.
Return<void> SecureElementHalCallback::onStateChange(bool cardPresent) {
    // TODO implement
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

ISecureElementHalCallback* HIDL_FETCH_ISecureElementHalCallback(const char* /* name */) {
    return new SecureElementHalCallback();
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace secure_element
}  // namespace hardware
}  // namespace android
