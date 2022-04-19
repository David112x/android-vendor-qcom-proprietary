/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef LOC_HIDL_VERSION
#error "LOC_HIDL_VERSION must be set before including this file."
#endif

#include <vendor/qti/gnss/1.1/ILocHidlXT.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace vendor {
namespace qti {
namespace gnss {
namespace LOC_HIDL_VERSION {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

using ::vendor::qti::gnss::V1_1::ILocHidlXT;

struct LocHidlXT : public ILocHidlXT {
    LocHidlXT() = default;
    virtual ~LocHidlXT() = default;
    // Methods from ::vendor::qti::gnss::V1_1::ILocHidlXT follow.
    Return<void> init() override;
    Return<void> setUserPref(bool userPref) override;

private:
    typedef void (createXtAdapterInstance)();
    typedef void (setXtAdapterUserPref)(bool userPref);

};

}  // namespace implementation
}  // namespace LOC_HIDL_VERSION
}  // namespace gnss
}  // namespace qti
}  // namespace vendor
