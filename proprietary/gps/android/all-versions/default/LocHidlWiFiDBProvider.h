/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef LOC_HIDL_VERSION
#error "LOC_HIDL_VERSION must be set before including this file."
#endif

#include <vendor/qti/gnss/2.1/ILocHidlWiFiDBProvider.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

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

using ::vendor::qti::gnss::V2_1::ILocHidlWiFiDBProvider;
using ::vendor::qti::gnss::V2_1::ILocHidlWiFiDBProviderCallback;

struct LocHidlWiFiDBProvider : public ILocHidlWiFiDBProvider {
    LocHidlWiFiDBProvider();
    ~LocHidlWiFiDBProvider();
    // Methods from ILocHidlWiFiDBProvider follow.
    Return<bool> init(const sp<ILocHidlWiFiDBProviderCallback>& callback) override;
    Return<void> registerWiFiDBProvider(
            const sp<ILocHidlWiFiDBProviderCallback>& callback) override;
    Return<void> unregisterWiFiDBProvider() override;
    Return<void> sendAPObsLocDataRequest() override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

private:
    void* mIzatWiFiDBProviderApiClient = nullptr;
    sp<ILocHidlWiFiDBProviderCallback> mCallbackIface = nullptr;
    sp<LocHidlDeathRecipient> mDeathRecipient = nullptr;
};

extern "C" ILocHidlWiFiDBProvider* HIDL_FETCH_ILocHidlWiFiDBProvider(const char* name);

}  // namespace implementation
}  // namespace LOC_HIDL_VERSION
}  // namespace gnss
}  // namespace qti
}  // namespace vendor
