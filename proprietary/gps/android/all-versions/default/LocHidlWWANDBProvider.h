/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef LOC_HIDL_VERSION
#error "LOC_HIDL_VERSION must be set before including this file."
#endif

#include <vendor/qti/gnss/2.1/ILocHidlWWANDBProvider.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <location_interface.h>
#include "LBSAdapter.h"

namespace vendor {
namespace qti {
namespace gnss {
namespace LOC_HIDL_VERSION {
namespace implementation {

using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_death_recipient;
using ::android::sp;
using ::android::wp;

using ::vendor::qti::gnss::V2_1::ILocHidlWWANDBProvider;
using ::vendor::qti::gnss::V2_1::ILocHidlWWANDBProviderCallback;

struct LocHidlWWANDBProvider : public ILocHidlWWANDBProvider {
    LocHidlWWANDBProvider();
    ~LocHidlWWANDBProvider();
    // Methods from ILocHidlWWANDBProvider follow.
    Return<bool> init(const sp<::vendor::qti::gnss::V2_1::ILocHidlWWANDBProviderCallback>& callback) override;
    Return<void> registerWWANDBProvider(const sp<ILocHidlWWANDBProviderCallback>& callback) override;
    Return<void> unregisterWWANDBProvider() override;
    Return<void> sendBSObsLocDataRequest() override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.
private:
    LBSAdapter* mLBSAdapter = nullptr;
    sp<ILocHidlWWANDBProviderCallback> mCallbackIface = nullptr;
    sp<LocHidlDeathRecipient> mDeathRecipient = nullptr;
};

extern "C" ILocHidlWWANDBProvider* HIDL_FETCH_ILocHidlWWANDBProvider(const char* name);

}  // namespace implementation
}  // namespace V1_2
}  // namespace gnss
}  // namespace qti
}  // namespace vendor
