/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef LOC_HIDL_VERSION
#error "LOC_HIDL_VERSION must be set before including this file."
#endif

#include <vendor/qti/gnss/1.1/ILocHidlAGnss.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <gps_extended_c.h>
#include <location_interface.h>

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

using ::vendor::qti::gnss::V1_1::ILocHidlAGnss;

struct LocHidlAGnss : public ILocHidlAGnss {
    LocHidlAGnss() = default;
    virtual ~LocHidlAGnss() = default;
    // Methods from ::android::hardware::gnss::V1_0::IAGnss follow.
    Return<void> setCallback(
            const sp<::android::hardware::gnss::V1_0::IAGnssCallback>& callback) override;
    inline Return<bool> dataConnClosed() override { return bool {}; }
    inline Return<bool> dataConnFailed() override { return bool {}; }
    inline Return<bool> setServer(
            ::android::hardware::gnss::V1_0::IAGnssCallback::AGnssType /*type*/,
            const hidl_string& /*hostname*/, int32_t /*port*/) override { return bool {}; }
    inline Return<bool> dataConnOpen(const hidl_string& /*apn*/,
            ::android::hardware::gnss::V1_0::IAGnss::ApnIpType /*apnIpType*/) override {
        return bool {};
    }

    // Methods from ::vendor::qti::gnss::V1_1::ILocHidlAGnss follow.
    Return<bool> dataConnOpenExt(const hidl_string& apn,
            ::android::hardware::gnss::V1_0::IAGnss::ApnIpType apnIpType,
            ::android::hardware::gnss::V1_0::IAGnssCallback::AGnssType agnssType) override;
    Return<bool> dataConnClosedExt(
            ::android::hardware::gnss::V1_0::IAGnssCallback::AGnssType agnssType) override;
    Return<bool> dataConnFailedExt(
            ::android::hardware::gnss::V1_0::IAGnssCallback::AGnssType agnssType) override;
    // Method to fetch GNSS Interface
    const GnssInterface* getGnssInterface();

    /* Data call setup callback passed down to GNSS HAL implementation */
    static void agnssStatusIpV4Cb(AGnssExtStatusIpV4 status);

    // Methods from ::android::hidl::base::V1_0::IBase follow.

private:
    const GnssInterface* mGnssInterface = nullptr;
    static sp<LocHidlDeathRecipient> mDeathRecipient;
    static sp<::android::hardware::gnss::V1_0::IAGnssCallback> sAGnssCbIface;
};

}  // namespace implementation
}  // namespace LOC_HIDL_VERSION
}  // namespace gnss
}  // namespace qti
}  // namespace vendor
