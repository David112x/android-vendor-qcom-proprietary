/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef TRUSTEDUI_FACTORY_H
#define TRUSTEDUI_FACTORY_H


#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <vendor/qti/hardware/trustedui/1.0/ITrustedUI.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace trustedui {
namespace V1_0 {
namespace implementation {

struct TrustedUIFactory {
    static std::shared_ptr<ITrustedUI> GetInstance();
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace trustedui
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

#endif
