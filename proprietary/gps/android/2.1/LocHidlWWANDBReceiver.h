/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef VENDOR_QTI_GNSS_V2_1_LOCHIDLWWANDBRECEIVER_H
#define VENDOR_QTI_GNSS_V2_1_LOCHIDLWWANDBRECEIVER_H

#include "LocHidlUtils.h"

#define LOC_HIDL_VERSION V2_1
#include <vendor/qti/gnss/2.1/ILocHidlWWANDBReceiver.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <location_interface.h>
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

using ::vendor::qti::gnss::V2_1::ILocHidlWWANDBReceiver;
struct LocHidlWWANDBReceiver : public ILocHidlWWANDBReceiver {

    LocHidlWWANDBReceiver();
    virtual ~LocHidlWWANDBReceiver();

    // Methods from ::vendor::qti::gnss::V1_2::ILocHidlWWANDBReceiver follow.
    Return<bool> init(
            const sp<::vendor::qti::gnss::V1_2::ILocHidlWWANDBReceiverCallback>& callback) override;
    Return<void> registerWWANDBUpdater(
            const sp<::vendor::qti::gnss::V1_2::ILocHidlWWANDBReceiverCallback>& callback) override;
    Return<void> unregisterWWANDBUpdater() override;
    Return<void> sendBSListRequest(int32_t expireInDays) override;
    Return<void> pushBSWWANDB(
            const hidl_vec<LocHidlBsLocationData>& bsLocationDataList,
            int16_t bsLocationDataListSize,
            const hidl_vec<LocHidlBsSpecialInfo>& bsSpecialInfoList,
            int16_t bsSpecialInfoListSize, int32_t daysValid) override;

    // Methods from ::vendor::qti::gnss::V2_1::ILocHidlWWANDBReceiver follow.
    Return<bool> init_2_1(
            const sp<::vendor::qti::gnss::V2_1::ILocHidlWWANDBReceiverCallback>& callback) override;
    Return<void> registerWWANDBUpdater_2_1  (
            const sp<::vendor::qti::gnss::V2_1::ILocHidlWWANDBReceiverCallback>& callback) override;
private:
    LBSAdapter* mLBSAdapter = nullptr;
    sp<::vendor::qti::gnss::V1_2::ILocHidlWWANDBReceiverCallback> mCallbackIface = nullptr;
    sp<LocHidlDeathRecipient> mDeathRecipient = nullptr;
};

}  // namespace implementation
}  // namespace LOC_HIDL_VERSION
}  // namespace gnss
}  // namespace qti
}  // namespace vendor

#undef LOC_HIDL_VERSION

#endif  // VENDOR_QTI_GNSS_V2_1_LOCHIDLWWANDBRECEIVER_H
