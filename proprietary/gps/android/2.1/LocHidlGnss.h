/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef VENDOR_QTI_GNSS_V2_1_LOCHIDLGNSS_H
#define VENDOR_QTI_GNSS_V2_1_LOCHIDLGNSS_H

#define LOC_HIDL_VERSION V2_1

#include <vendor/qti/gnss/2.1/ILocHidlGnss.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace vendor {
namespace qti {
namespace gnss {
namespace LOC_HIDL_VERSION {
namespace implementation {

using ::android::hardware::gnss::V1_0::IAGnss;
using ::android::hardware::gnss::V1_0::IAGnssRil;
using ::android::hardware::gnss::V1_0::IGnss;
using ::android::hardware::gnss::V1_0::IGnssBatching;
using ::android::hardware::gnss::V1_0::IGnssCallback;
using ::android::hardware::gnss::V1_0::IGnssConfiguration;
using ::android::hardware::gnss::V1_0::IGnssDebug;
using ::android::hardware::gnss::V1_0::IGnssGeofencing;
using ::android::hardware::gnss::V1_0::IGnssMeasurement;
using ::android::hardware::gnss::V1_0::IGnssNavigationMessage;
using ::android::hardware::gnss::V1_0::IGnssNi;
using ::android::hardware::gnss::V1_0::IGnssXtra;

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

using ::vendor::qti::gnss::V2_1::ILocHidlGnss;
struct LocHidlGnss : public ILocHidlGnss {
    LocHidlGnss();
    virtual ~LocHidlGnss();
    // Methods from ::android::hardware::gnss::V1_0::IGnss follow.
    inline Return<bool> setCallback(const sp<IGnssCallback>& /*callback*/) override;
    inline Return<bool> start() override;
    inline Return<bool> stop() override;
    inline Return<void> cleanup() override;
    inline Return<bool> injectTime(int64_t /*timeMs*/, int64_t /*timeReferenceMs*/,
        int32_t /*uncertaintyMs*/) override;
    inline Return<bool> injectLocation(double /*latitudeDegrees*/, double /*longitudeDegrees*/,
        float /*accuracyMeters*/) override;
    inline Return<void> deleteAidingData(IGnss::GnssAidingData /*aidingDataFlags*/) override;
    inline Return<bool> setPositionMode(
        IGnss::GnssPositionMode /*mode*/, IGnss::GnssPositionRecurrence /*recurrence*/,
        uint32_t /*minIntervalMs*/, uint32_t /*preferredAccuracyMeters*/,
        uint32_t /*preferredTimeMs*/) override;
    inline Return<sp<IAGnssRil>> getExtensionAGnssRil() override;
    inline Return<sp<IGnssGeofencing>> getExtensionGnssGeofencing() override;
    inline Return<sp<IAGnss>> getExtensionAGnss() override;
    inline Return<sp<IGnssNi>> getExtensionGnssNi() override;
    inline Return<sp<IGnssMeasurement>> getExtensionGnssMeasurement() override;
    inline Return<sp<IGnssNavigationMessage>> getExtensionGnssNavigationMessage() override;
    inline Return<sp<IGnssXtra>> getExtensionXtra() override;
    inline Return<sp<IGnssConfiguration>> getExtensionGnssConfiguration() override;
    inline Return<sp<IGnssDebug>> getExtensionGnssDebug() override;
    inline Return<sp<IGnssBatching>> getExtensionGnssBatching() override;

    // Methods from ::vendor::qti::gnss::V1_0::ILocHidlGnss follow.
    Return<sp<V1_0::ILocHidlAGnss>> getExtensionLocHidlAGnss() override;
    Return<sp<V1_0::ILocHidlDebugReportService>> getExtensionLocHidlDebugReportService() override;
    Return<sp<V1_0::ILocHidlFlpService>> getExtensionLocHidlFlpService() override;
    Return<sp<V1_0::ILocHidlGeofenceService>> getExtensionLocHidlGeofenceService() override;
    Return<sp<V1_0::ILocHidlIzatProvider>> getExtensionLocHidlIzatNetworkProvider() override;
    Return<sp<V1_0::ILocHidlIzatProvider>> getExtensionLocHidlIzatFusedProvider() override;
    Return<sp<V1_0::ILocHidlGnssNi>> getExtensionLocHidlGnssNi() override;
    Return<sp<V1_0::ILocHidlIzatOsNpGlue>> getExtensionLocHidlIzatOsNpGlue() override;
    Return<sp<V1_0::ILocHidlIzatSubscription>> getExtensionLocHidlIzatSubscription() override;
    Return<sp<V1_0::ILocHidlRilInfoMonitor>> getExtensionLocHidlRilInfoMonitor() override;
    Return<sp<V1_0::ILocHidlXT>> getExtensionLocHidlXT() override;
    Return<sp<V1_0::ILocHidlWiFiDBReceiver>> getExtensionLocHidlWiFiDBReceiver() override;

    // Methods from ::vendor::qti::gnss::V1_1::ILocHidlGnss follow.
    Return<sp<V1_1::ILocHidlAGnss>> getExtensionLocHidlAGnss_1_1() override;
    Return<sp<V1_1::ILocHidlDebugReportService>>
                    getExtensionLocHidlDebugReportService_1_1() override;
    Return<sp<V1_1::ILocHidlFlpService>> getExtensionLocHidlFlpService_1_1() override;
    Return<sp<V1_1::ILocHidlGeofenceService>> getExtensionLocHidlGeofenceService_1_1() override;
    Return<sp<V1_1::ILocHidlIzatProvider>> getExtensionLocHidlIzatNetworkProvider_1_1() override;
    Return<sp<V1_1::ILocHidlIzatProvider>> getExtensionLocHidlIzatFusedProvider_1_1() override;
    Return<sp<V1_1::ILocHidlGnssNi>> getExtensionLocHidlGnssNi_1_1() override;
    Return<sp<V1_1::ILocHidlIzatOsNpGlue>> getExtensionLocHidlIzatOsNpGlue_1_1() override;
    Return<sp<V1_1::ILocHidlIzatSubscription>> getExtensionLocHidlIzatSubscription_1_1() override;
    Return<sp<V1_1::ILocHidlRilInfoMonitor>> getExtensionLocHidlRilInfoMonitor_1_1() override;
    Return<sp<V1_1::ILocHidlXT>> getExtensionLocHidlXT_1_1() override;
    Return<sp<V1_1::ILocHidlWiFiDBReceiver>> getExtensionLocHidlWiFiDBReceiver_1_1() override;

    // Methods from ::vendor::qti::gnss::V1_2::ILocHidlGnss follow.
    Return<sp<V1_2::ILocHidlWWANDBReceiver>> getExtensionLocHidlWWANDBReceiver();

    // Methods from ::vendor::qti::gnss::V2_1::ILocHidlGnss follow.
    Return<sp<::vendor::qti::gnss::V2_1::ILocHidlAGnss>> getExtensionLocHidlAGnss_2_1() override;
    Return<sp<V2_1::ILocHidlFlpService>> getExtensionLocHidlFlpService_2_1() override;
    Return<sp<V2_1::ILocHidlGnssConfigService>> getExtensionLocHidlGnssConfigService_2_1();
    Return<sp<V2_1::ILocHidlWiFiDBReceiver>> getExtensionLocHidlWiFiDBReceiver_2_1() override;
    Return<sp<V2_1::ILocHidlWiFiDBProvider>> getExtensionLocHidlWiFiDBProvider() override;
    Return<sp<V2_1::ILocHidlWWANDBReceiver>> getExtensionLocHidlWWANDBReceiver_2_1() override;
    Return<sp<V2_1::ILocHidlWWANDBProvider>> getExtensionLocHidlWWANDBProvider() override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

private:
    sp<IGnss> mGnss = nullptr;
    sp<V1_1::ILocHidlAGnss> mAGnssVendor = nullptr;
    sp<V1_1::ILocHidlDebugReportService> mDebugReportService = nullptr;
    sp<V2_1::ILocHidlFlpService> mGnssFlpServiceProvider = nullptr;
    sp<V1_1::ILocHidlGeofenceService> mGnssGeofenceServiceProvider = nullptr;
    sp<V1_1::ILocHidlIzatProvider> mIzatNetworkProvider = nullptr;
    sp<V1_1::ILocHidlIzatProvider> mIzatFusedProvider = nullptr;
    sp<V1_1::ILocHidlGnssNi> mGnssNiVendor = nullptr;
    sp<V1_1::ILocHidlIzatOsNpGlue> mIzatOsNpGlue = nullptr;
    sp<V1_1::ILocHidlIzatSubscription> mIzatSubscription = nullptr;
    sp<V1_1::ILocHidlRilInfoMonitor> mGnssRilInfoMonitor = nullptr;
    sp<V1_1::ILocHidlXT> mGnssXT = nullptr;
    sp<V1_1::ILocHidlWiFiDBReceiver> mIzatWiFiDBReceiver = nullptr;
    sp<V1_2::ILocHidlWWANDBReceiver> mIzatWWANDBReceiver = nullptr;
    sp<V2_1::ILocHidlAGnss> mAGnssVendorV2_1 = nullptr;
    sp<V2_1::ILocHidlGnssConfigService> mGnssConfigService = nullptr;
    sp<V2_1::ILocHidlWiFiDBReceiver> mIzatWiFiDBReceiverV2_1 = nullptr;
    sp<V2_1::ILocHidlWWANDBReceiver> mIzatWWANDBReceiverV2_1 = nullptr;
    sp<V2_1::ILocHidlWiFiDBProvider> mIzatWiFiDBProvider = nullptr;
    sp<V2_1::ILocHidlWWANDBProvider> mIzatWWANDBProvider = nullptr;
};

extern "C" ILocHidlGnss* HIDL_FETCH_ILocHidlGnss(const char* name);

}  // namespace implementation
}  // namespace LOC_HIDL_VERSION
}  // namespace gnss
}  // namespace qti
}  // namespace vendor
#undef LOC_HIDL_VERSION
#endif  // VENDOR_QTI_GNSS_V2_1_LOCHIDLGNSS_H
