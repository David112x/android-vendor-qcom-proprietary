/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#define LOG_TAG "LocSvc_HIDL_HidlGnss"
#define LOG_NDEBUG 0

#include "LocHidlGnss.h"
#include "LocHidlAGnss.h"
#include "LocHidlDebugReportService.h"
#include "LocHidlFlpService.h"
#include "LocHidlGeofenceService.h"
#include "LocHidlIzatProvider.h"
#include "LocHidlGnssNi.h"
#include "LocHidlIzatOsNpGlue.h"
#include "LocHidlIzatSubscription.h"
#include "LocHidlRilInfoMonitor.h"
#include "LocHidlXT.h"
#include "LocHidlWiFiDBReceiver.h"
#include "LocHidlWWANDBReceiver.h"
#include "LocHidlUtils.h"
#include "LocHidlGnssConfigService.h"
#include "LocHidlWiFiDBProvider.h"
#include "LocHidlWWANDBProvider.h"

#define LOC_HIDL_VERSION V2_1
#include <log_util.h>
#include <dlfcn.h>
#define GNSS_LIB_V10 "android.hardware.gnss@1.0-impl-qti.so"
#define GNSS_LIB_V11 "android.hardware.gnss@1.1-impl-qti.so"
#include <OSFramework.h>

namespace vendor {
namespace qti {
namespace gnss {
namespace LOC_HIDL_VERSION {
namespace implementation {

using ::vendor::qti::gnss::V1_0::LocHidlIzatStreamType;
using ::vendor::qti::gnss::LOC_HIDL_VERSION::implementation::LocHidlAGnss;
using ::vendor::qti::gnss::LOC_HIDL_VERSION::implementation::LocHidlDebugReportService;
using ::vendor::qti::gnss::LOC_HIDL_VERSION::implementation::LocHidlFlpService;
using ::vendor::qti::gnss::LOC_HIDL_VERSION::implementation::LocHidlGeofenceService;
using ::vendor::qti::gnss::LOC_HIDL_VERSION::implementation::LocHidlGnssNi;
using ::vendor::qti::gnss::LOC_HIDL_VERSION::implementation::LocHidlIzatOsNpGlue;
using ::vendor::qti::gnss::LOC_HIDL_VERSION::implementation::LocHidlIzatProvider;
using ::vendor::qti::gnss::LOC_HIDL_VERSION::implementation::LocHidlIzatSubscription;
using ::vendor::qti::gnss::LOC_HIDL_VERSION::implementation::LocHidlRilInfoMonitor;
using ::vendor::qti::gnss::LOC_HIDL_VERSION::implementation::LocHidlWiFiDBReceiver;
using ::vendor::qti::gnss::LOC_HIDL_VERSION::implementation::LocHidlWWANDBReceiver;
using ::vendor::qti::gnss::LOC_HIDL_VERSION::implementation::LocHidlXT;
using ::vendor::qti::gnss::LOC_HIDL_VERSION::implementation::LocHidlGnssConfigService;
typedef IGnss* (HIDL_FETCH_IGnss_t)(const char* );

static IGnss* getIGnssFromLib(const char* libname) {
    if (nullptr == libname)
        return nullptr;
    LOC_LOGI("try getting IGnss from %s", libname);
    const char* error = nullptr;
    dlerror();
    void *handle = dlopen(libname, RTLD_NOW);
    if (nullptr != (error = dlerror()) || nullptr == handle)  {
        LOC_LOGE("dlopen for %s failed, error = %s", libname, error == nullptr ? "UNKNOWN" : error);
        return nullptr;
    }

    HIDL_FETCH_IGnss_t* getter = nullptr;
    getter = (HIDL_FETCH_IGnss_t*)dlsym(handle, "HIDL_FETCH_IGnss");
    if (nullptr != (error = dlerror()) || nullptr == getter)  {
        LOC_LOGE("dlsym for %s failed, error = %s", libname, error == nullptr ? "UNKNOWN" : error);
        return nullptr;
    }

    IGnss* gnss = (IGnss*)(*getter)(nullptr);
    if (nullptr == gnss) {
        LOC_LOGE("failed to get IGnss from %s", libname);
    }
    return gnss;
}

LocHidlGnss::LocHidlGnss() {
    ENTRY_LOG();
    IGnss* gnss = getIGnssFromLib(GNSS_LIB_V10);
    if (nullptr == gnss) {
        gnss = getIGnssFromLib(GNSS_LIB_V11);
    }
    mGnss = gnss;
}

LocHidlGnss::~LocHidlGnss() {
    ENTRY_LOG();
}

// Methods from ::android::hardware::gnss::V1_0::IGnss follow.
Return<bool> LocHidlGnss::setCallback(const sp<IGnssCallback>& callback) {
    FROM_HIDL_CLIENT();
    if (mGnss != nullptr) {
        return mGnss->setCallback(callback);
    }
    return bool {};
}
Return<bool> LocHidlGnss::start() {
    FROM_HIDL_CLIENT();
    if (mGnss != nullptr) {
        return mGnss->start();
    }
    return bool {};
}
Return<bool> LocHidlGnss::stop() {
    FROM_HIDL_CLIENT();
    if (mGnss != nullptr) {
        return mGnss->stop();
    }
    return bool {};
}
Return<void> LocHidlGnss::cleanup() {
    FROM_HIDL_CLIENT();
    if (mGnss != nullptr) {
        return mGnss->cleanup();
    }
    return Void();
}
Return<bool> LocHidlGnss::injectTime(int64_t timeMs, int64_t timeReferenceMs,
        int32_t uncertaintyMs) {
    FROM_HIDL_CLIENT();
    if (mGnss != nullptr) {
        return mGnss->injectTime(timeMs, timeReferenceMs, uncertaintyMs);
    }
    return bool {};
}
Return<bool> LocHidlGnss::injectLocation(double latitudeDegrees, double longitudeDegrees,
        float accuracyMeters) {
    FROM_HIDL_CLIENT();
    if (mGnss != nullptr) {
        return mGnss->injectLocation(latitudeDegrees, longitudeDegrees, accuracyMeters);
    }
    return bool {};
}
Return<void> LocHidlGnss::deleteAidingData(IGnss::GnssAidingData aidingDataFlags) {
    FROM_HIDL_CLIENT();
    if (mGnss != nullptr) {
        return mGnss->deleteAidingData(aidingDataFlags);
    }
    return Void();
}
Return<bool> LocHidlGnss::setPositionMode(
        IGnss::GnssPositionMode mode, IGnss::GnssPositionRecurrence recurrence,
        uint32_t minIntervalMs, uint32_t preferredAccuracyMeters,
        uint32_t preferredTimeMs) {
    FROM_HIDL_CLIENT();
    if (mGnss != nullptr) {
        return mGnss->setPositionMode(mode, recurrence, minIntervalMs,
                preferredAccuracyMeters, preferredTimeMs);
    }
    return bool {};
}
Return<sp<IAGnssRil>> LocHidlGnss::getExtensionAGnssRil() {
    FROM_HIDL_CLIENT();
    if (mGnss != nullptr) {
        return mGnss->getExtensionAGnssRil();
    }
    return sp<IAGnssRil> {};
}
Return<sp<IGnssGeofencing>> LocHidlGnss::getExtensionGnssGeofencing() {
    FROM_HIDL_CLIENT();
    if (mGnss != nullptr) {
        return mGnss->getExtensionGnssGeofencing();
    }
    return sp<IGnssGeofencing> {};
}
Return<sp<IAGnss>> LocHidlGnss::getExtensionAGnss() {
    FROM_HIDL_CLIENT();
    if (mGnss != nullptr) {
        return mGnss->getExtensionAGnss();
    }
    return sp<IAGnss> {};
}
Return<sp<IGnssNi>> LocHidlGnss::getExtensionGnssNi() {
    FROM_HIDL_CLIENT();
    if (mGnss != nullptr) {
        return mGnss->getExtensionGnssNi();
    }
    return sp<IGnssNi> {};
}
Return<sp<IGnssMeasurement>> LocHidlGnss::getExtensionGnssMeasurement() {
    FROM_HIDL_CLIENT();
    if (mGnss != nullptr) {
        return mGnss->getExtensionGnssMeasurement();
    }
    return sp<IGnssMeasurement> {};
}
Return<sp<IGnssNavigationMessage>> LocHidlGnss::getExtensionGnssNavigationMessage() {
    FROM_HIDL_CLIENT();
    if (mGnss != nullptr) {
        return mGnss->getExtensionGnssNavigationMessage();
    }
    return sp<IGnssNavigationMessage> {};
}
Return<sp<IGnssXtra>> LocHidlGnss::getExtensionXtra() {
    FROM_HIDL_CLIENT();
    if (mGnss != nullptr) {
        return mGnss->getExtensionXtra();
    }
    return sp<IGnssXtra> {};
}
Return<sp<IGnssConfiguration>> LocHidlGnss::getExtensionGnssConfiguration() {
    FROM_HIDL_CLIENT();
    if (mGnss != nullptr) {
        return mGnss->getExtensionGnssConfiguration();
    }
    return sp<IGnssConfiguration> {};
}
Return<sp<IGnssDebug>> LocHidlGnss::getExtensionGnssDebug() {
    FROM_HIDL_CLIENT();
    if (mGnss != nullptr) {
        return mGnss->getExtensionGnssDebug();
    }
    return sp<IGnssDebug> {};
}
Return<sp<IGnssBatching>> LocHidlGnss::getExtensionGnssBatching() {
    FROM_HIDL_CLIENT();
    if (mGnss != nullptr) {
        return mGnss->getExtensionGnssBatching();
    }
    return sp<IGnssBatching> {};
}

// Methods from ::vendor::qti::gnss::V1_0::ILocHidlGnss follow.
Return<sp<V1_0::ILocHidlAGnss>> LocHidlGnss::getExtensionLocHidlAGnss() {
    FROM_HIDL_CLIENT();
    if (mAGnssVendor == nullptr) {
        mAGnssVendor = new LocHidlAGnss();
    }
    return mAGnssVendor;
}

Return<sp<V1_0::ILocHidlDebugReportService>> LocHidlGnss::getExtensionLocHidlDebugReportService() {
    FROM_HIDL_CLIENT();
    if (mDebugReportService == nullptr) {
        mDebugReportService = new LocHidlDebugReportService();
    }
    return mDebugReportService;
}

Return<sp<V1_0::ILocHidlFlpService>> LocHidlGnss::getExtensionLocHidlFlpService() {
    FROM_HIDL_CLIENT();
    if (mGnssFlpServiceProvider == nullptr) {
        mGnssFlpServiceProvider = new LocHidlFlpService();
    }
    return mGnssFlpServiceProvider;
}

Return<sp<V1_0::ILocHidlGeofenceService>> LocHidlGnss::getExtensionLocHidlGeofenceService() {
    FROM_HIDL_CLIENT();
    if (mGnssGeofenceServiceProvider == nullptr) {
        mGnssGeofenceServiceProvider = new LocHidlGeofenceService();
    }
    return mGnssGeofenceServiceProvider;
}

Return<sp<V1_0::ILocHidlIzatProvider>> LocHidlGnss::getExtensionLocHidlIzatNetworkProvider() {
    FROM_HIDL_CLIENT();
    if (mIzatNetworkProvider == nullptr) {
        mIzatNetworkProvider = new LocHidlIzatProvider(LocHidlIzatStreamType::NETWORK);
    }
    return mIzatNetworkProvider;
}

Return<sp<V1_0::ILocHidlIzatProvider>> LocHidlGnss::getExtensionLocHidlIzatFusedProvider() {
    FROM_HIDL_CLIENT();
    if (mIzatFusedProvider == nullptr) {
        mIzatFusedProvider = new LocHidlIzatProvider(LocHidlIzatStreamType::FUSED);
    }
    return mIzatFusedProvider;
}

Return<sp<V1_0::ILocHidlGnssNi>> LocHidlGnss::getExtensionLocHidlGnssNi() {
    FROM_HIDL_CLIENT();
    if (mGnssNiVendor == nullptr) {
        mGnssNiVendor = new LocHidlGnssNi();
    }
    return mGnssNiVendor;
}

Return<sp<V1_0::ILocHidlIzatOsNpGlue>> LocHidlGnss::getExtensionLocHidlIzatOsNpGlue() {
    FROM_HIDL_CLIENT();
    if (mIzatOsNpGlue == nullptr) {
        mIzatOsNpGlue = new LocHidlIzatOsNpGlue();
    }
    return mIzatOsNpGlue;
}

Return<sp<V1_0::ILocHidlIzatSubscription>> LocHidlGnss::getExtensionLocHidlIzatSubscription() {
    FROM_HIDL_CLIENT();
    if (mIzatSubscription == nullptr) {
        mIzatSubscription = new LocHidlIzatSubscription();
    }
    return mIzatSubscription;
}

Return<sp<V1_0::ILocHidlRilInfoMonitor>> LocHidlGnss::getExtensionLocHidlRilInfoMonitor() {
    FROM_HIDL_CLIENT();
    if (mGnssRilInfoMonitor == nullptr) {
        mGnssRilInfoMonitor = new LocHidlRilInfoMonitor();
    }
    return mGnssRilInfoMonitor;
}

Return<sp<V1_0::ILocHidlXT>> LocHidlGnss::getExtensionLocHidlXT() {
    FROM_HIDL_CLIENT();
    if (mGnssXT == nullptr) {
        mGnssXT = new LocHidlXT();
    }
    return mGnssXT;
}

Return<sp<V1_0::ILocHidlWiFiDBReceiver>> LocHidlGnss::getExtensionLocHidlWiFiDBReceiver() {
    FROM_HIDL_CLIENT();
    if (mIzatWiFiDBReceiver == nullptr) {
        mIzatWiFiDBReceiver = new LocHidlWiFiDBReceiver();
    }
    return mIzatWiFiDBReceiver;
}

// Methods from ::vendor::qti::gnss::V1_1::ILocHidlGnss follow.
Return<sp<V1_1::ILocHidlAGnss>> LocHidlGnss::getExtensionLocHidlAGnss_1_1() {
    FROM_HIDL_CLIENT();
    if (mAGnssVendor == nullptr) {
        mAGnssVendor = new LocHidlAGnss();
    }
    return mAGnssVendor;
}

Return<sp<V1_1::ILocHidlDebugReportService>> LocHidlGnss::getExtensionLocHidlDebugReportService_1_1() {
    FROM_HIDL_CLIENT();
    if (mDebugReportService == nullptr) {
        mDebugReportService = new LocHidlDebugReportService();
    }
    return mDebugReportService;
}

Return<sp<V1_1::ILocHidlFlpService>> LocHidlGnss::getExtensionLocHidlFlpService_1_1() {
    FROM_HIDL_CLIENT();
    if (mGnssFlpServiceProvider == nullptr) {
        mGnssFlpServiceProvider = new LocHidlFlpService();
    }
    return mGnssFlpServiceProvider;
}

Return<sp<V1_1::ILocHidlGeofenceService>> LocHidlGnss::getExtensionLocHidlGeofenceService_1_1() {
    FROM_HIDL_CLIENT();
    if (mGnssGeofenceServiceProvider == nullptr) {
        mGnssGeofenceServiceProvider = new LocHidlGeofenceService();
    }
    return mGnssGeofenceServiceProvider;
}

Return<sp<V1_1::ILocHidlIzatProvider>> LocHidlGnss::getExtensionLocHidlIzatNetworkProvider_1_1() {
    FROM_HIDL_CLIENT();
    if (mIzatNetworkProvider == nullptr) {
        mIzatNetworkProvider = new LocHidlIzatProvider(LocHidlIzatStreamType::NETWORK);
    }
    return mIzatNetworkProvider;
}

Return<sp<V1_1::ILocHidlIzatProvider>> LocHidlGnss::getExtensionLocHidlIzatFusedProvider_1_1() {
    FROM_HIDL_CLIENT();
    if (mIzatFusedProvider == nullptr) {
        mIzatFusedProvider = new LocHidlIzatProvider(LocHidlIzatStreamType::FUSED);
    }
    return mIzatFusedProvider;
}

Return<sp<V1_1::ILocHidlGnssNi>> LocHidlGnss::getExtensionLocHidlGnssNi_1_1() {
    FROM_HIDL_CLIENT();
    if (mGnssNiVendor == nullptr) {
        mGnssNiVendor = new LocHidlGnssNi();
    }
    return mGnssNiVendor;
}

Return<sp<V1_1::ILocHidlIzatOsNpGlue>> LocHidlGnss::getExtensionLocHidlIzatOsNpGlue_1_1() {
    FROM_HIDL_CLIENT();
    if (mIzatOsNpGlue == nullptr) {
        mIzatOsNpGlue = new LocHidlIzatOsNpGlue();
    }
    return mIzatOsNpGlue;
}

Return<sp<V1_1::ILocHidlIzatSubscription>> LocHidlGnss::getExtensionLocHidlIzatSubscription_1_1() {
    FROM_HIDL_CLIENT();
    if (mIzatSubscription == nullptr) {
        mIzatSubscription = new LocHidlIzatSubscription();
    }
    return mIzatSubscription;
}

Return<sp<V1_1::ILocHidlRilInfoMonitor>> LocHidlGnss::getExtensionLocHidlRilInfoMonitor_1_1() {
    FROM_HIDL_CLIENT();
    if (mGnssRilInfoMonitor == nullptr) {
        mGnssRilInfoMonitor = new LocHidlRilInfoMonitor();
    }
    return mGnssRilInfoMonitor;
}

Return<sp<V1_1::ILocHidlXT>> LocHidlGnss::getExtensionLocHidlXT_1_1() {
    FROM_HIDL_CLIENT();
    if (mGnssXT == nullptr) {
        mGnssXT = new LocHidlXT();
    }
    return mGnssXT;
}

Return<sp<V1_1::ILocHidlWiFiDBReceiver>> LocHidlGnss::getExtensionLocHidlWiFiDBReceiver_1_1() {
    FROM_HIDL_CLIENT();
    if (mIzatWiFiDBReceiver == nullptr) {
        mIzatWiFiDBReceiver = new LocHidlWiFiDBReceiver();
    }
    return mIzatWiFiDBReceiver;
}

// Methods from ::vendor::qti::gnss::V1_2::ILocHidlGnss follow.

Return<sp<V1_2::ILocHidlWWANDBReceiver>> LocHidlGnss::getExtensionLocHidlWWANDBReceiver() {
    FROM_HIDL_CLIENT();
    if (mIzatWWANDBReceiver == nullptr) {
        mIzatWWANDBReceiver = new LocHidlWWANDBReceiver();
    }
    return mIzatWWANDBReceiver;
}

// Methods from ::vendor::qti::gnss::V2_1::ILocHidlGnss follow.
Return<sp<V2_1::ILocHidlAGnss>> LocHidlGnss::getExtensionLocHidlAGnss_2_1() {
    FROM_HIDL_CLIENT();
    if (mAGnssVendorV2_1 == nullptr) {
        mAGnssVendorV2_1 = new LocHidlAGnss();
    }
    return mAGnssVendorV2_1;
}

Return<sp<V2_1::ILocHidlFlpService>> LocHidlGnss::getExtensionLocHidlFlpService_2_1() {
    FROM_HIDL_CLIENT();
    if (mGnssFlpServiceProvider == nullptr) {
        mGnssFlpServiceProvider = new LocHidlFlpService();
    }
    return mGnssFlpServiceProvider;
}

Return<sp<V2_1::ILocHidlGnssConfigService>> LocHidlGnss::getExtensionLocHidlGnssConfigService_2_1() {
    FROM_HIDL_CLIENT();
    if (mGnssConfigService == nullptr) {
        mGnssConfigService = new LocHidlGnssConfigService();
    }
    return mGnssConfigService;
}

Return<sp<V2_1::ILocHidlWiFiDBReceiver>> LocHidlGnss::getExtensionLocHidlWiFiDBReceiver_2_1() {
    FROM_HIDL_CLIENT();
    if (mIzatWiFiDBReceiverV2_1 == nullptr) {
        mIzatWiFiDBReceiverV2_1 = new LocHidlWiFiDBReceiver();
    }
    return mIzatWiFiDBReceiverV2_1;
}

Return<sp<V2_1::ILocHidlWiFiDBProvider>> LocHidlGnss::getExtensionLocHidlWiFiDBProvider() {
    FROM_HIDL_CLIENT();
    if (mIzatWiFiDBProvider == nullptr) {
        mIzatWiFiDBProvider = new LocHidlWiFiDBProvider();
    }
    return mIzatWiFiDBProvider;
}

Return<sp<V2_1::ILocHidlWWANDBReceiver>> LocHidlGnss::getExtensionLocHidlWWANDBReceiver_2_1() {
    FROM_HIDL_CLIENT();
    if (mIzatWWANDBReceiverV2_1 == nullptr) {
        mIzatWWANDBReceiverV2_1 = new LocHidlWWANDBReceiver();
    }
    return mIzatWWANDBReceiverV2_1;
}

Return<sp<V2_1::ILocHidlWWANDBProvider>> LocHidlGnss::getExtensionLocHidlWWANDBProvider() {
    FROM_HIDL_CLIENT();
    if (mIzatWWANDBProvider == nullptr) {
        mIzatWWANDBProvider = new LocHidlWWANDBProvider();
    }
    return mIzatWWANDBProvider;
}

// Methods from ::android::hidl::base::V1_0::IBase follow.

ILocHidlGnss* HIDL_FETCH_ILocHidlGnss(const char* /* name */) {
    ENTRY_LOG();
    auto hal = new LocHidlGnss();
    OSFramework::getOSFramework();
    return hal;
}

}  // namespace implementation
}  // namespace LOC_HIDL_VERSION
}  // namespace gnss
}  // namespace qti
}  // namespace vendor
#undef LOC_HIDL_VERSION
