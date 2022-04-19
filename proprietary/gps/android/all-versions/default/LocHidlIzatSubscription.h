/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef LOC_HIDL_VERSION
#error "LOC_HIDL_VERSION must be set before including this file."
#endif

#include <vendor/qti/gnss/1.1/ILocHidlIzatSubscription.h>
#include <vendor/qti/gnss/1.1/types.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <Subscription.h>

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

using ::vendor::qti::gnss::V1_0::ILocHidlIzatSubscriptionCallback;
using ::vendor::qti::gnss::V1_1::ILocHidlIzatSubscription;
using ::vendor::qti::gnss::V1_1::LocHidlSubscriptionDataItemId_1_1;

struct LocHidlIzatSubscription : public ILocHidlIzatSubscription {
    LocHidlIzatSubscription();
    virtual ~LocHidlIzatSubscription();
    // Methods from ::vendor::qti::gnss::V1_1::ILocHidlIzatSubscription follow.
    Return<bool> init(const sp<ILocHidlIzatSubscriptionCallback>& callback) override;
    Return<void> deinit() override;
    Return<void> boolDataItemUpdate(const hidl_vec<BoolDataItem>& dataItemArray) override;
    Return<void> stringDataItemUpdate(const StringDataItem& dataItem) override;
    Return<void> networkinfoUpdate(const NetworkInfoDataItem& dataItem) override;
    Return<void> serviceinfoUpdate(const RilServiceInfoDataItem& dataItem) override;
    Return<void> cellLteUpdate(const CellLteDataItem& dataItem) override;
    Return<void> cellGwUpdate(const CellGwDataItem& dataItem) override;
    Return<void> cellCdmaUpdate(const CellCdmaDataItem& dataItem) override;
    Return<void> cellOooUpdate(const CellOooDataItem& dataItem) override;
    Return<void> serviceStateUpdate(const ServiceStateDataItem& dataItem) override;
    Return<void> screenStatusUpdate(const ScreenStatusDataItem& dataItem) override;
    Return<void> powerConnectStatusUpdate(const PowerConnectStatusDataItem& dataItem) override;
    Return<void> timezoneChangeUpdate(const TimeZoneChangeDataItem& dataItem) override;
    Return<void> timeChangeUpdate(const TimeChangeDataItem& dataItem) override;
    Return<void> shutdownUpdate() override;
    Return<void> wifiSupplicantStatusUpdate(const WifiSupplicantStatusDataItem& dataItem) override;
    Return<void> btleScanDataInject(const BtLeDeviceScanDetailsDataItem& dataItem) override;
    Return<void> btClassicScanDataInject(const BtDeviceScanDetailsDataItem& dataItem) override;
    Return<void> batteryLevelUpdate(const BatteryLevelDataItem& dataItem) override;

private:
    struct IzatSubscriptionCallback: SubscriptionCallbackIface {

        IzatSubscriptionCallback(
                sp<ILocHidlIzatSubscriptionCallback> hidlCbIface) :
                    mHidlCbIface(hidlCbIface) {};
        ~IzatSubscriptionCallback() = default;
        sp<ILocHidlIzatSubscriptionCallback> mHidlCbIface;

        virtual void updateSubscribe(
                const std::list<DataItemId> & l, bool subscribe) override;
        virtual void requestData(const std::list<DataItemId> & l) override;
        virtual void unsubscribeAll() override;
        virtual void turnOnModule(DataItemId dit,int timeOut) override;
        virtual void turnOffModule(DataItemId dit) override;
};

private:
    sp<ILocHidlIzatSubscriptionCallback> mCallbackIface = nullptr;
    sp<LocHidlDeathRecipient> mDeathRecipient = nullptr;
    IzatSubscriptionCallback* mSubscriptionCb;
};

}  // namespace implementation
}  // namespace LOC_HIDL_VERSION
}  // namespace gnss
}  // namespace qti
}  // namespace vendor
