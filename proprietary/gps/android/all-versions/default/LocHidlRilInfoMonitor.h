/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef LOC_HIDL_VERSION
#error "LOC_HIDL_VERSION must be set before including this file."
#endif

#include <vendor/qti/gnss/1.1/ILocHidlRilInfoMonitor.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <LBSAdapter.h>

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

using ::vendor::qti::gnss::V1_1::ILocHidlRilInfoMonitor;

struct LocHidlRilInfoMonitor : public ILocHidlRilInfoMonitor {
    LocHidlRilInfoMonitor() = default;
    virtual ~LocHidlRilInfoMonitor() = default;
    // Methods from ::vendor::qti::gnss::V1_1::ILocHidlRilInfoMonitor follow.
    Return<void> init() override;
    Return<void> cinfoInject(int32_t cid, int32_t lac,
            int32_t mnc, int32_t mcc, bool roaming) override;
    Return<void> oosInform() override;
    Return<void> niSuplInit(const hidl_string& str) override;
    Return<void> chargerStatusInject(int32_t status) override;

private:
    LBSAdapter* mLBSAdapter = NULL;

};

}  // namespace implementation
}  // namespace LOC_HIDL_VERSION
}  // namespace gnss
}  // namespace qti
}  // namespace vendor
