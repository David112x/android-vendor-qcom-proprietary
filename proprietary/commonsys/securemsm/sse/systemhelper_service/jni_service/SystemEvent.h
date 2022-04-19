/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef SYSTEMEVENT_H
#define SYSTEMEVENT_H

#include <vendor/qti/hardware/systemhelper/1.0/ISystemEvent.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace systemhelper {
namespace V1_0 {
namespace implementation {

using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using ::vendor::qti::hardware::systemhelper::V1_0::ISystemEventCallback;

struct SystemEvent : public ISystemEvent {
    // Methods from ::vendor::qti::hardware::systemhelper::V1_0::ISystemEvent follow.
    Return<int32_t> registerEvent(uint64_t eventIds, const sp<ISystemEventCallback>& cb) override;
    Return<int32_t> deRegisterEvent(uint64_t eventIds, const sp<ISystemEventCallback>& cb) override;
};

}  // namespace implementation
}  // namespace V1_0
}  // namespace systemhelper
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
#endif
