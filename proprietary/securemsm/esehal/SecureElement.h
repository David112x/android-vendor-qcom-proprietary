/**
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef ANDROID_HARDWARE_SECURE_ELEMENT_V1_0_SECUREELEMENT_H
#define ANDROID_HARDWARE_SECURE_ELEMENT_V1_0_SECUREELEMENT_H

#include <android/hardware/secure_element/1.0/ISecureElement.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include "gpqese/gpqese-apdu.h"

namespace android {
namespace hardware {
namespace secure_element {
namespace V1_0 {
namespace implementation {

using ::android::hardware::secure_element::V1_0::ISecureElement;
using ::android::hardware::secure_element::V1_0::ISecureElementHalCallback;
using ::android::hardware::secure_element::V1_0::LogicalChannelResponse;
using ::android::hardware::secure_element::V1_0::SecureElementStatus;
using ::android::hidl::base::V1_0::IBase;
using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct SecureElement : public ISecureElement {
  // Methods from ::android::hardware::secure_element::V1_0::ISecureElement follow.
  Return<void> init(const sp<ISecureElementHalCallback>
                                   &clientCallback) override;
  Return<void> getAtr(getAtr_cb _hidl_cb) override;
  Return<bool> isCardPresent() override;
  Return<void> transmit(const hidl_vec<uint8_t> &data,
                        transmit_cb _hidl_cb) override;
  Return<void> openLogicalChannel(const hidl_vec<uint8_t> &aid, uint8_t p2,
                                  openLogicalChannel_cb _hidl_cb) override;
  Return<void> openBasicChannel(const hidl_vec<uint8_t> &aid, uint8_t p2,
                                openBasicChannel_cb _hidl_cb) override;
  Return<SecureElementStatus> closeChannel(uint8_t channelNumber) override;

  // Methods from ::android::hidl::base::V1_0::IBase follow.

};

}  // namespace implementation
}  // namespace V1_0
}  // namespace secure_element
}  // namespace hardware
}  // namespace android
#endif  // ANDROID_HARDWARE_SECURE_ELEMENT_V1_0_SECUREELEMENT_H
