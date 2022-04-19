/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef LOC_HIDL_VERSION
#error "LOC_HIDL_VERSION must be set before including this file."
#endif

#include <OsNpGlue.h>
#include <vendor/qti/gnss/1.1/ILocHidlIzatOsNpGlue.h>
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

using ::vendor::qti::gnss::V1_1::ILocHidlIzatOsNpGlue;

struct LocHidlIzatOsNpGlue : public ILocHidlIzatOsNpGlue {
    LocHidlIzatOsNpGlue() = default;
    virtual ~LocHidlIzatOsNpGlue() = default;
    // Methods from ::vendor::qti::gnss::V1_0::ILocHidlIzatOsNpGlue follow.
    Return<bool> setCallback(
            const sp<::vendor::qti::gnss::V1_0::ILocHidlIzatOsNpGlueCallback>& callback) override;
    Return<void> locationChanged(
            const ::vendor::qti::gnss::V1_0::LocHidlIzatLocation& location) override;

private:
    sp<LocHidlDeathRecipient> mDeathRecipient = nullptr;
    sp<IOsNpGlueCallback> mCallback = nullptr;
};

}  // namespace implementation
}  // namespace LOC_HIDL_VERSION
}  // namespace gnss
}  // namespace qti
}  // namespace vendor
