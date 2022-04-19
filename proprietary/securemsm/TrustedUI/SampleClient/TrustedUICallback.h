/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef TRUSTEDUICALLBACK_H
#define TRUSTEDUICALLBACK_H

#include <vendor/qti/hardware/trustedui/1.0/ITrustedUICallback.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace trustedui {
namespace V1_0 {
namespace implementation {

using ::android::hardware::Return;
using ::android::hardware::Void;

struct TrustedUICallback : public ITrustedUICallback {
    // Methods from ::vendor::qti::hardware::trustedui::V1_0::ITrustedUICallback follow.
    Return<void> onComplete() override;
    Return<void> onError() override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};


}  // namespace implementation
}  // namespace V1_0
}  // namespace trustedui
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

#endif
