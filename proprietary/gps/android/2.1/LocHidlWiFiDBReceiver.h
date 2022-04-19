/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef VENDOR_QTI_GNSS_V2_1_LOCHIDLWIFIDBRECEIVER_H
#define VENDOR_QTI_GNSS_V2_1_LOCHIDLWIFIDBRECEIVER_H

#include "LocHidlUtils.h"

#define LOC_HIDL_VERSION V2_1

#include <vendor/qti/gnss/2.1/ILocHidlWiFiDBReceiver.h>
#include <vendor/qti/gnss/2.1/ILocHidlWiFiDBReceiverCallback.h>
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

using ::vendor::qti::gnss::V2_1::ILocHidlWiFiDBReceiver;
using ::vendor::qti::gnss::V2_1::ILocHidlWiFiDBReceiverCallback;
struct LocHidlWiFiDBReceiver : public ILocHidlWiFiDBReceiver {
    LocHidlWiFiDBReceiver();
    virtual ~LocHidlWiFiDBReceiver();
    // Methods from ::vendor::qti::gnss::V1_1::ILocHidlWiFiDBReceiver follow.
    Return<bool> init(
            const sp<::vendor::qti::gnss::V1_0::ILocHidlWiFiDBReceiverCallback>& callback) override;
    Return<void> registerWiFiDBUpdater(
            const sp<::vendor::qti::gnss::V1_0::ILocHidlWiFiDBReceiverCallback>& callback) override;
    Return<void> unregisterWiFiDBUpdater() override;
    Return<void> sendAPListRequest(int32_t expireInDays) override;
    Return<void> pushAPWiFiDB(
            const hidl_vec<LocHidlApLocationData>& apLocationDataList,
            int16_t apLocationDataListSize,
            const hidl_vec<LocHidlApSpecialInfo>& apSpecialInfoList,
            int16_t apSpecialInfoListSize, int32_t daysValid) override;
    // Methods from ::vendor::qti::gnss::V2_1::ILocHidlWiFiDBReceiver follow.
    Return<void> sendScanListRequest() override;
    Return<void> pushAPWiFiDB_2_1(
            const hidl_vec<ILocHidlWiFiDBReceiver::LocHidlApLocationData>& apLocationDataList,
            int16_t apLocationDataListSize,
            const hidl_vec<ILocHidlWiFiDBReceiver::LocHidlApSpecialInfo>& apSpecialInfoList,
            int16_t apSpecialInfoListSize, int32_t daysValid, bool is_lookup) override;

private:
    void* mIzatWiFiDBReceiverApiClient = nullptr;
    sp<V1_0::ILocHidlWiFiDBReceiverCallback> mCallbackIface = nullptr;
    sp<LocHidlDeathRecipient> mDeathRecipient = nullptr;
};

}  // namespace implementation
}  // namespace LOC_HIDL_VERSION
}  // namespace gnss
}  // namespace qti
}  // namespace vendor

#undef LOC_HIDL_VERSION

#endif  // VENDOR_QTI_GNSS_V2_1_LOCHIDLWIFIDBRECEIVER_H
