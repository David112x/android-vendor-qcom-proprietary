/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef LOC_HIDL_VERSION
#error "LOC_HIDL_VERSION must be set before including this file."
#endif

#include <vendor/qti/gnss/1.1/ILocHidlGnssNi.h>
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

using ::android::hardware::gnss::V1_0::IGnssNiCallback;
using ::vendor::qti::gnss::V1_1::ILocHidlGnssNi;
using ::vendor::qti::gnss::V1_0::ILocHidlGnssNiCallback;

struct LocHidlGnssNi : public ILocHidlGnssNi {
    LocHidlGnssNi();
    virtual ~LocHidlGnssNi();
    // Methods from ::android::hardware::gnss::V1_0::IGnssNi follow.
    inline Return<void> setCallback(const sp<IGnssNiCallback>& /*callback*/) override {
        return Void();
    }
    Return<void> respond(int32_t notifId,
            IGnssNiCallback::GnssUserResponseType userResponse) override;

    // Methods from ::vendor::qti::gnss::V1_1::ILocHidlGnssNi follow.
    Return<void> setVendorCallback(const sp<ILocHidlGnssNiCallback>& callback) override;

private:
    LocHidlNiClient* mNiClient = nullptr;
    sp<ILocHidlGnssNiCallback> mCallbackIface = nullptr;
    sp<LocHidlDeathRecipient> mDeathRecipient = nullptr;
};

}  // namespace implementation
}  // namespace LOC_HIDL_VERSION
}  // namespace gnss
}  // namespace qti
}  // namespace vendor
