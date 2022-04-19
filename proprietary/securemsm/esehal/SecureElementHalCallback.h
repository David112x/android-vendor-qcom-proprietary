/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef ANDROID_HARDWARE_SECURE_ELEMENT_V1_0_SECUREELEMENTHALCALLBACK_H
#define ANDROID_HARDWARE_SECURE_ELEMENT_V1_0_SECUREELEMENTHALCALLBACK_H

#include <android/hardware/secure_element/1.0/ISecureElementHalCallback.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace android {
namespace hardware {
namespace secure_element {
namespace V1_0 {
namespace implementation {

using ::android::hardware::secure_element::V1_0::ISecureElementHalCallback;
using ::android::hidl::base::V1_0::IBase;
using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct SecureElementHalCallback : public ISecureElementHalCallback {
    // Methods from ::android::hardware::secure_element::V1_0::ISecureElementHalCallback follow.
    Return<void> onStateChange(bool cardPresent) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

extern "C" ISecureElementHalCallback* HIDL_FETCH_ISecureElementHalCallback(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace secure_element
}  // namespace hardware
}  // namespace android

#endif  // ANDROID_HARDWARE_SECURE_ELEMENT_V1_0_SECUREELEMENTHALCALLBACK_H
