/*
Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.

Not a contribution.
*/
/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#define GNSS_VENDOR_SERVICE_INSTANCE "gnss_vendor"

#include "GardenUtil.h"
#include <loc_cfg.h>
#include "HidlAPI.h"
#include <mutex>
#include <algorithm>
#include <android/hardware/gnss/1.0/IGnss.h>
#include <android/hardware/gnss/1.1/IGnss.h>
#include <android/hardware/gnss/2.0/IGnss.h>

#include "loc_ril.h"

#define GEOFENCE_BREACH_ENTERED            (1<<0L)
#define GEOFENCE_BREACH_EXITED             (1<<1L)
#define GEOFENCE_BREACH_UNCERTAIN          (1<<2L)

#define GNSS_DEBUG_UNKNOWN_HORIZONTAL_ACCURACY_METERS (20000000)
#define GNSS_DEBUG_UNKNOWN_VERTICAL_ACCURACY_METERS   (20000)
#define GNSS_DEBUG_UNKNOWN_SPEED_ACCURACY_PER_SEC     (500)
#define GNSS_DEBUG_UNKNOWN_BEARING_ACCURACY_DEG       (180)

#define GNSS_DEBUG_UNKNOWN_UTC_TIME            (1483228800000LL) // 1/1/2017 00:00 GMT
#define GNSS_DEBUG_UNKNOWN_UTC_TIME_UNC_MIN    (999) // 999 ns
#define GNSS_DEBUG_UNKNOWN_UTC_TIME_UNC_MAX    (1.57783680E17) // 5 years in ns
#define GNSS_DEBUG_UNKNOWN_FREQ_UNC_NS_PER_SEC (2.0e5)  // ppm

#ifdef LOCHIDL
#include <vendor/qti/gnss/1.0/ILocHidlGnss.h>
#include <vendor/qti/gnss/1.1/ILocHidlGnss.h>
#include <vendor/qti/gnss/1.2/ILocHidlGnss.h>
#include <vendor/qti/gnss/2.1/ILocHidlGnss.h>
#include <vendor/qti/gnss/3.0/ILocHidlGnss.h>
#include <vendor/qti/gnss/2.1/ILocHidlWiFiDBReceiverCallback.h>
using vendor::qti::gnss::V3_0::ILocHidlGnss;
using vendor::qti::gnss::V3_0::ILocHidlIzatConfig;
using vendor::qti::gnss::V2_1::ILocHidlFlpService;
using vendor::qti::gnss::V1_0::ILocHidlGeofenceService;
using vendor::qti::gnss::V1_0::ILocHidlGnssNi;
using vendor::qti::gnss::V1_0::ILocHidlFlpServiceCallback;
using vendor::qti::gnss::V1_0::ILocHidlGeofenceServiceCallback;
using vendor::qti::gnss::V1_0::ILocHidlGnssNiCallback;
using vendor::qti::gnss::V1_0::LocHidlLocation;
using vendor::qti::gnss::V1_0::LocHidlIzatLocation;
using vendor::qti::gnss::V1_0::LocHidlBatchOptions;
using vendor::qti::gnss::V1_0::LocHidlBatchStatusInfo;
using vendor::qti::gnss::V1_1::ILocHidlIzatSubscription;
using vendor::qti::gnss::V1_0::ILocHidlIzatSubscriptionCallback;
using vendor::qti::gnss::V1_0::LocHidlSubscriptionDataItemId;
using vendor::qti::gnss::V1_1::ILocHidlRilInfoMonitor;
using vendor::qti::gnss::V1_1::ILocHidlIzatProvider;
using vendor::qti::gnss::V1_0::ILocHidlIzatProviderCallback;
using vendor::qti::gnss::V1_0::LocHidlNetworkPositionSourceType;
using vendor::qti::gnss::V1_0::LocHidlIzatProviderStatus;
using vendor::qti::gnss::V1_0::LocHidlIzatHorizontalAccuracy;
using vendor::qti::gnss::V1_0::LocHidlIzatRequest;
using vendor::qti::gnss::V1_0::LocHidlIzatStreamType;
using vendor::qti::gnss::V1_1::ILocHidlIzatOsNpGlue;
using vendor::qti::gnss::V1_0::ILocHidlIzatOsNpGlueCallback;
using vendor::qti::gnss::V1_1::ILocHidlXT;
using vendor::qti::gnss::V3_0::ILocHidlIzatConfig;
using vendor::qti::gnss::V3_0::ILocHidlIzatConfigCallback;
using vendor::qti::gnss::V2_1::ILocHidlGnssConfigService;
using vendor::qti::gnss::V2_1::ILocHidlGnssConfigServiceCallback;
using vendor::qti::gnss::V2_1::ILocHidlAGnss;
using vendor::qti::gnss::V2_1::ILocHidlAGnssCallback;
using vendor::qti::gnss::V1_1::ILocHidlDebugReportService;
using vendor::qti::gnss::V2_1::ILocHidlWiFiDBReceiver;
using vendor::qti::gnss::V2_1::ILocHidlWiFiDBReceiverCallback;
using vendor::qti::gnss::V2_1::ILocHidlWWANDBReceiver;
using vendor::qti::gnss::V2_1::ILocHidlWWANDBReceiverCallback;
using vendor::qti::gnss::V2_1::ILocHidlWWANDBProvider;
using vendor::qti::gnss::V2_1::ILocHidlWWANDBProviderCallback;
using vendor::qti::gnss::V2_1::ILocHidlWiFiDBProvider;
using vendor::qti::gnss::V2_1::ILocHidlWiFiDBProviderCallback;
using vendor::qti::gnss::V2_1::LocHidlUlpLocation;
using vendor::qti::gnss::V2_1::LocHidlWifiDBListStatus;
#endif
using android::OK;
using android::sp;
using android::wp;
using android::status_t;
using android::hardware::Return;
using android::hardware::Void;
using android::hardware::hidl_vec;
using android::hardware::hidl_string;
using android::hardware::hidl_death_recipient;
using android::hardware::hidl_bitfield;
using android::hidl::base::V1_0::IBase;
using android::hardware::gnss::V2_0::IAGnss;
using android::hardware::gnss::V2_0::IAGnssCallback;
using android::hardware::gnss::V2_0::IAGnssRil;
using android::hardware::gnss::V1_0::IAGnssRilCallback;
using android::hardware::gnss::V2_0::IGnss;
using android::hardware::gnss::V2_0::IGnssBatching;
using android::hardware::gnss::V2_0::IGnssBatchingCallback;
using android::hardware::gnss::V2_0::IGnssCallback;
using android::hardware::gnss::V2_0::IGnssConfiguration;
using android::hardware::gnss::V2_0::IGnssDebug;
using android::hardware::gnss::V1_0::IGnssGeofenceCallback;
using android::hardware::gnss::V1_0::IGnssGeofencing;
using android::hardware::gnss::V2_0::IGnssMeasurement;
using android::hardware::gnss::V2_0::IGnssMeasurementCallback;
using android::hardware::gnss::V1_0::IGnssNavigationMessage;
using android::hardware::gnss::V1_0::IGnssNavigationMessageCallback;
using android::hardware::gnss::V1_0::IGnssNi;
using android::hardware::gnss::V1_0::IGnssNiCallback;
using android::hardware::gnss::V1_0::IGnssXtra;
using android::hardware::gnss::V1_0::IGnssXtraCallback;
using android::hardware::gnss::V1_0::GnssConstellationType;
using android::hardware::gnss::visibility_control::V1_0::IGnssVisibilityControl;
using android::hardware::gnss::visibility_control::V1_0::IGnssVisibilityControlCallback;
using android::hardware::gnss::measurement_corrections::V1_0::IMeasurementCorrections;
using IAGnssRil_V1_0 = android::hardware::gnss::V2_0::IAGnssRil;
using IGnssConfiguration_V1_0 = android::hardware::gnss::V1_0::IGnssConfiguration;
using IGnssConfiguration_V1_1 = android::hardware::gnss::V1_1::IGnssConfiguration;

using IAGnss_v1_0 = android::hardware::gnss::V1_0::IAGnss;
using IAGnssCallback_v1_0 = android::hardware::gnss::V1_0::IAGnssCallback;
using GnssLocation = android::hardware::gnss::V1_0::GnssLocation;
using GnssLocation_v2_0 = android::hardware::gnss::V2_0::GnssLocation;
using android::hardware::gnss::V1_0::GnssMax;

using std::mutex;
using std::min;
using namespace loc_core;

namespace garden {
struct GnssDeathRecipient;
sp<GnssDeathRecipient> gnssHalDeathRecipient = nullptr;
sp<GnssDeathRecipient> gnssLocHalDeathRecipient = nullptr;
sp<IGnss> gnssHal = nullptr;
sp<IAGnssRil> agnssRilIface = nullptr;
sp<IGnssGeofencing> gnssGeofencingIface = nullptr;
sp<IAGnss> agnssIface = nullptr;
sp<IGnssBatching> gnssBatchingIface = nullptr;
sp<IGnssDebug> gnssDebugIface = nullptr;
sp<IGnssConfiguration> gnssConfigIface = nullptr;
sp<IGnssNi> gnssNiIface = nullptr;
sp<IGnssMeasurement> gnssMeasurementIface = nullptr;
sp<IMeasurementCorrections> measurementCorrIface = nullptr;
sp<IGnssVisibilityControl> visibCtrlIface = nullptr;
#ifdef LOCHIDL
sp<ILocHidlGnss> gnssHidlHal =  nullptr;
sp<ILocHidlFlpService> locHidlFlpIface = nullptr;
sp<ILocHidlGeofenceService> locHidlGeofenceIface = nullptr;
sp<ILocHidlGnssNi> gnssHidlNiIface = nullptr;
sp<ILocHidlIzatSubscription> gnssSubscriptionIface = nullptr;
sp<ILocHidlRilInfoMonitor> gnssRilInfoIface = nullptr;
sp<ILocHidlIzatProvider> izatProviderIface = nullptr;
sp<ILocHidlIzatOsNpGlue> izatOsNpIface = nullptr;
sp<ILocHidlXT> izatXTIface = nullptr;
sp<ILocHidlGnssConfigService> lochidlgnssConfigIface = nullptr;
sp<ILocHidlIzatConfig> izatConfigIface = nullptr;
sp<ILocHidlAGnss> agnssExtIface = nullptr;
sp<ILocHidlDebugReportService> debugReportIface = nullptr;
sp<ILocHidlWiFiDBReceiver> wifiDBReceiverIface = nullptr;
sp<ILocHidlWiFiDBProvider> wifiDBProviderIface = nullptr;
sp<ILocHidlWWANDBReceiver> wwanDBReceiverIface = nullptr;
sp<ILocHidlWWANDBProvider> wwanDBProviderIface = nullptr;
#endif
static GnssCbBase* sGnssCbs = nullptr;
static GnssCbBase* sFlpCbs = nullptr;
static GnssCbBase* sGeofenceCbs = nullptr;
static GnssCbBase* sAGnssCbs = nullptr;
static GnssCbBase* sAGnssRilCbs = nullptr;
static GnssCbBase* sGnssMeasurementCbs = nullptr;
static GnssCbBase* sVisibCtrlCbs = nullptr;
static mutex sGnssLock;
static mutex sFlpLock;
static mutex sGeofenceLock;
static mutex sAGnssLock;
static mutex sAGnssRilLock;
static mutex sGnssMeasurementLock;
static mutex sVisibCtrlLock;

#ifdef LOCHIDL
static GnssCbBase* sDataItemCbs = nullptr;
static GnssCbBase* sIzatProviderCbs = nullptr;
static GnssCbBase* sOsNpCbs = nullptr;
static GnssCbBase* sGnssConfigCbs = nullptr;
static GnssCbBase* sIzatConfigCbs = nullptr;
static GnssCbBase* sAgnssExtCbs = nullptr;
static GnssCbBase* sWifiDBReceiverCbs = nullptr;
static GnssCbBase* sWifiDBProviderCbs = nullptr;
static GnssCbBase* sWwanDBReceiverCbs = nullptr;
static GnssCbBase* sWwanDBProviderCbs = nullptr;
static mutex sDataItemLock;
static mutex sIzatProviderLock;
static mutex sOsNpLock;
static mutex sGnssConfigLock;
static mutex sIzatConfigLock;
static mutex sAgnssExtLock;
static mutex sWiFiDBReceiverLock;
static mutex sWiFiDBProviderLock;
static mutex sWwanDBReceiverLock;
static mutex sWwanDBProviderLock;
#endif

struct GnssDeathRecipient : virtual public hidl_death_recipient {
    virtual void serviceDied(uint64_t cookie, const wp<IBase>& /*who*/) override {
        gardenPrint("%s] IGnss died with cookie: %" PRIu64, __FILE__, cookie);
        gnssHal = nullptr;
#ifdef LOCHIDL
        gnssHidlHal = nullptr;
#endif
    }
};
static void translateLocation(Location& location, const GnssLocation& gnssLocation) {
    location.size = sizeof(Location);
    location.flags = static_cast<uint16_t>(gnssLocation.gnssLocationFlags);
    location.timestamp = gnssLocation.timestamp;
    location.latitude = gnssLocation.latitudeDegrees;
    location.longitude = gnssLocation.longitudeDegrees;
    location.altitude = gnssLocation.altitudeMeters;
    location.speed = gnssLocation.speedMetersPerSec;
    location.bearing = gnssLocation.bearingDegrees;
    location.accuracy = gnssLocation.horizontalAccuracyMeters;
    location.verticalAccuracy = gnssLocation.horizontalAccuracyMeters;
    location.speedAccuracy = gnssLocation.speedAccuracyMetersPerSecond;
    location.bearingAccuracy = gnssLocation.bearingAccuracyDegrees;
}

static void translateToHidlLocation(GnssLocation& gnssLocation, const Location& location) {
    gnssLocation.gnssLocationFlags = location.flags ;
    gnssLocation.timestamp = location.timestamp;
    gnssLocation.latitudeDegrees = location.latitude;
    gnssLocation.longitudeDegrees = location.longitude;
    gnssLocation.altitudeMeters = location.altitude;
    gnssLocation.speedMetersPerSec = location.speed;
    gnssLocation.bearingDegrees = location.bearing;
    gnssLocation.horizontalAccuracyMeters = location.accuracy;
    gnssLocation.horizontalAccuracyMeters = location.verticalAccuracy;
    gnssLocation.speedAccuracyMetersPerSecond = location.speedAccuracy;
    gnssLocation.bearingAccuracyDegrees = location.bearingAccuracy;
}

#ifdef LOCHIDL
static void translateLocation(Location& location, const LocHidlLocation& hidlLocation) {
    location.size = sizeof(Location);
    location.flags = hidlLocation.locationFlagsMask;
    location.timestamp = hidlLocation.timestamp;
    location.latitude = hidlLocation.latitude;
    location.longitude = hidlLocation.longitude;
    location.altitude = hidlLocation.altitude;
    location.speed = hidlLocation.speed;
    location.bearing = hidlLocation.bearing;
    location.accuracy = hidlLocation.accuracy;
    location.verticalAccuracy = hidlLocation.verticalAccuracy;
    location.speedAccuracy = hidlLocation.speedAccuracy;
    location.bearingAccuracy = hidlLocation.bearingAccuracy;
    location.techMask = hidlLocation.locationTechnologyMask;
}

static void translateHidlUlpLocation(Location& location, const LocHidlUlpLocation& ulpLocation) {
    translateLocation(location, ulpLocation.gpsLocation);
}

static void translateHidlUlpLocationToUlpLocation(UlpLocation& location,
        const LocHidlUlpLocation& ulpLocation) {
    location.size = ulpLocation.size;
    location.position_source = ulpLocation.position_source;
    location.tech_mask = ulpLocation.tech_mask;
    location.gpsLocation.flags = ulpLocation.gpsLocation.locationFlagsMask;
    location.gpsLocation.latitude = ulpLocation.gpsLocation.latitude;
    location.gpsLocation.longitude = ulpLocation.gpsLocation.longitude;
    location.gpsLocation.altitude = ulpLocation.gpsLocation.altitude;
    location.gpsLocation.speed = ulpLocation.gpsLocation.speed;
    location.gpsLocation.bearing = ulpLocation.gpsLocation.bearing;
    location.gpsLocation.accuracy = ulpLocation.gpsLocation.accuracy;
    location.gpsLocation.vertUncertainity = ulpLocation.gpsLocation.verticalAccuracy;
    location.gpsLocation.timestamp = ulpLocation.gpsLocation.timestamp;
}

static void translateIzatLocation(Location& location, const LocHidlIzatLocation& izatLocation) {
    location.size = sizeof(Location);
    location.timestamp = izatLocation.utcTimestampInMsec;
    if (izatLocation.hasLatitude && izatLocation.hasLongitude) {
        location.latitude = izatLocation.latitude;
        location.longitude = izatLocation.longitude;
        location.flags |= LOCATION_HAS_LAT_LONG_BIT;
    }
    if (izatLocation.hasAltitudeWrtEllipsoid) {
        location.altitude = izatLocation.altitudeWrtEllipsoid;
        location.flags |= LOCATION_HAS_ALTITUDE_BIT;
    }
    if (izatLocation.hasSpeed) {
        location.speed = izatLocation.speed;
        location.flags |= LOCATION_HAS_SPEED_BIT;
    }
    if (izatLocation.hasBearing) {
        location.bearing = izatLocation.bearing;
        location.flags |= LOCATION_HAS_BEARING_BIT;
    }
    if (izatLocation.hasHorizontalAccuracy) {
        location.accuracy = izatLocation.horizontalAccuracy;
        location.flags |= LOCATION_HAS_ACCURACY_BIT;
    }
    if (izatLocation.hasVertUnc) {
        location.verticalAccuracy = izatLocation.vertUnc;
        location.flags |= LOCATION_HAS_VERTICAL_ACCURACY_BIT;
    }
    if (izatLocation.hasSpeedUnc) {
        location.speedAccuracy = izatLocation.speedUnc;
        location.flags |= LOCATION_HAS_SPEED_ACCURACY_BIT;
    }
    if (izatLocation.hasBearingUnc) {
        location.bearingAccuracy = izatLocation.bearingUnc;
        location.flags |= LOCATION_HAS_BEARING_ACCURACY_BIT;
    }
    if (izatLocation.hasNetworkPositionSource) {
        if (izatLocation.networkPositionSource == LocHidlNetworkPositionSourceType::WIFI) {
            location.techMask |= LOCATION_TECHNOLOGY_WIFI_BIT;
        } else if (izatLocation.networkPositionSource == LocHidlNetworkPositionSourceType::CELL) {
            location.techMask |= LOCATION_TECHNOLOGY_CELL_BIT;
        }
    }
}

static void translateToIzatLocation(LocHidlIzatLocation& izatLocation, const Location& location) {
    izatLocation.utcTimestampInMsec = location.timestamp;
    if (location.flags & LOCATION_HAS_LAT_LONG_BIT) {
        izatLocation.latitude = location.latitude;
        izatLocation.longitude = location.longitude;
        izatLocation.hasLatitude = true;
        izatLocation.hasLongitude = true;
    }
    if (location.flags & LOCATION_HAS_ALTITUDE_BIT) {
        izatLocation.altitudeWrtEllipsoid = location.altitude;
        izatLocation.hasAltitudeWrtEllipsoid = true;
    }
    if (location.flags & LOCATION_HAS_SPEED_BIT) {
        izatLocation.speed = location.speed;
        izatLocation.hasSpeed = true;
    }
    if (location.flags & LOCATION_HAS_BEARING_ACCURACY_BIT) {
        izatLocation.bearing = location.bearing;
        izatLocation.hasBearing = true;
    }
    if (location.flags & LOCATION_HAS_ACCURACY_BIT) {
        izatLocation.horizontalAccuracy = location.accuracy;
        izatLocation.hasHorizontalAccuracy = true;
    }
    if (location.flags & LOCATION_HAS_VERTICAL_ACCURACY_BIT) {
        izatLocation.hasVertUnc = true;
        izatLocation.vertUnc = location.verticalAccuracy;
    }
    if (location.flags & LOCATION_HAS_SPEED_ACCURACY_BIT) {
        izatLocation.speedUnc = location.speedAccuracy;
        izatLocation.hasSpeedUnc = true;
    }
    if (location.flags & LOCATION_HAS_BEARING_ACCURACY_BIT) {
        izatLocation.bearingUnc = location.bearingAccuracy;
        izatLocation.hasBearingUnc = true;
    }
    if (location.techMask & LOCATION_TECHNOLOGY_WIFI_BIT) {
        izatLocation.hasNetworkPositionSource = true;
        izatLocation.networkPositionSource = LocHidlNetworkPositionSourceType::WIFI;
    } else if (location.techMask & LOCATION_TECHNOLOGY_CELL_BIT) {
        izatLocation.hasNetworkPositionSource = true;
        izatLocation.networkPositionSource = LocHidlNetworkPositionSourceType::CELL;
    }
}

static GnssConstellationType intToGnssConstellation(uint8_t svTypeInt) {
    switch (svTypeInt) {
        case 1:
            return GnssConstellationType::GPS;
        case 2:
            return GnssConstellationType::SBAS;
        case 3:
            return GnssConstellationType::GLONASS;
        case 4:
            return GnssConstellationType::QZSS;
        case 5:
            return GnssConstellationType::BEIDOU;
        case 6:
            return GnssConstellationType::GALILEO;
        default:
            return GnssConstellationType::UNKNOWN;
    }
}

//Debug report service related translation
static void populateTimeAndClockReport(
        ILocHidlDebugReportService::LocHidlSystemStatusReports& hidlReports,
        SystemStatusReports& systemReports, int32_t maxReports) {

    int count = hidlReports.base_1_0.mTimeAndClockVec.size();
    count = count < maxReports ? count : maxReports;
    if (count <= 0) {
        LOC_LOGE("Invalid count: %d", count);
        return;
    }

    for (int i=0; i < count && i < maxReports; i++) {

        SystemStatusTimeAndClock status;

        status.mUtcTime.tv_sec = hidlReports.base_1_0.mTimeAndClockVec[i].mUtcTime.tv_sec;
        status.mUtcTime.tv_nsec = hidlReports.base_1_0.mTimeAndClockVec[i].mUtcTime.tv_nsec;

        status.mUtcReported.tv_sec = hidlReports.base_1_0.mTimeAndClockVec[i].mUtcReported.tv_sec;
        status.mUtcReported.tv_nsec = hidlReports.base_1_0.mTimeAndClockVec[i].mUtcReported.tv_nsec;

        status.mGpsWeek = hidlReports.base_1_0.mTimeAndClockVec[i].mGpsWeek;
        status.mGpsTowMs = hidlReports.base_1_0.mTimeAndClockVec[i].mGpsTowMs;
        status.mTimeValid = hidlReports.base_1_0.mTimeAndClockVec[i].mTimeValid;
        status.mTimeSource = hidlReports.base_1_0.mTimeAndClockVec[i].mTimeSource;
        status.mTimeUnc = hidlReports.base_1_0.mTimeAndClockVec[i].mTimeUnc;
        status.mClockFreqBias =  hidlReports.base_1_0.mTimeAndClockVec[i].mClockFreqBias;
        status.mClockFreqBiasUnc = hidlReports.base_1_0.mTimeAndClockVec[i].mClockFreqBiasUnc;
        systemReports.mTimeAndClock.push_back(status);
    }
}

static void populateXoStateReport(
        ILocHidlDebugReportService::LocHidlSystemStatusReports& hidlReports,
        SystemStatusReports& systemReports, int32_t maxReports) {

    int count = hidlReports.base_1_0.mXoStateVec.size();
    count = count < maxReports ? count : maxReports;
    if (count <= 0) {
        LOC_LOGE("Invalid count: %d", count);
        return;
    }

    for (int i=0; i < count && i < maxReports; i++) {

        SystemStatusXoState status;

        status.mUtcTime.tv_sec = hidlReports.base_1_0.mXoStateVec[i].mUtcTime.tv_sec;
        status.mUtcTime.tv_nsec = hidlReports.base_1_0.mXoStateVec[i].mUtcTime.tv_nsec;

        status.mUtcReported.tv_sec = hidlReports.base_1_0.mXoStateVec[i].mUtcReported.tv_sec;
        status.mUtcReported.tv_nsec = hidlReports.base_1_0.mXoStateVec[i].mUtcReported.tv_nsec;

        status.mXoState = hidlReports.base_1_0.mXoStateVec[i].mXoState;
        systemReports.mXoState.push_back(status);
    }
}

static void populateRfParamsReport(
        ILocHidlDebugReportService::LocHidlSystemStatusReports& hidlReports,
        SystemStatusReports& systemReports, int32_t maxReports) {

    int count = hidlReports.mRfAndParamsVec_1_1.size();
    count = count < maxReports ? count : maxReports;
    if (count <= 0) {
        LOC_LOGE("Invalid count: %d", count);
        return;
    }

    for (int i=0; i < count && i < maxReports; i++) {

        SystemStatusRfAndParams status;

        status.mUtcTime.tv_sec = hidlReports.mRfAndParamsVec_1_1[i].base_1_0.mUtcTime.tv_sec;
        status.mUtcTime.tv_nsec = hidlReports.mRfAndParamsVec_1_1[i].base_1_0.mUtcTime.tv_nsec;

        status.mUtcReported.tv_sec =
                hidlReports.mRfAndParamsVec_1_1[i].base_1_0.mUtcReported.tv_sec;
        status.mUtcReported.tv_nsec =
                hidlReports.mRfAndParamsVec_1_1[i].base_1_0.mUtcReported.tv_nsec;

        status.mPgaGain = hidlReports.mRfAndParamsVec_1_1[i].base_1_0.mPgaGain;
        status.mAdcI = hidlReports.mRfAndParamsVec_1_1[i].base_1_0.mAdcI;
        status.mAdcQ = hidlReports.mRfAndParamsVec_1_1[i].base_1_0.mAdcQ;
        status.mJammerGps =hidlReports.mRfAndParamsVec_1_1[i].base_1_0.mJammerGps;
        status.mJammerGlo = hidlReports.mRfAndParamsVec_1_1[i].base_1_0.mJammerGlo;
        status.mJammerBds = hidlReports.mRfAndParamsVec_1_1[i].base_1_0.mJammerBds;
        status.mJammerGal = hidlReports.mRfAndParamsVec_1_1[i].base_1_0.mJammerGal;
        status.mGpsBpAmpI = hidlReports.mRfAndParamsVec_1_1[i].base_1_0.mGpsBpAmpI;
        status.mGpsBpAmpQ = hidlReports.mRfAndParamsVec_1_1[i].base_1_0.mGpsBpAmpQ;
        status.mGloBpAmpI = hidlReports.mRfAndParamsVec_1_1[i].mGloBpAmpI;
        status.mGloBpAmpQ = hidlReports.mRfAndParamsVec_1_1[i].mGloBpAmpQ;
        status.mBdsBpAmpI = hidlReports.mRfAndParamsVec_1_1[i].mBdsBpAmpI;
        status.mBdsBpAmpQ = hidlReports.mRfAndParamsVec_1_1[i].mBdsBpAmpQ;
        status.mGalBpAmpI = hidlReports.mRfAndParamsVec_1_1[i].mGalBpAmpI;
        status.mGalBpAmpQ = hidlReports.mRfAndParamsVec_1_1[i].mGalBpAmpQ;
        systemReports.mRfAndParams.push_back(status);

    }
}

static void populateErrRecoveryReport(
        ILocHidlDebugReportService::LocHidlSystemStatusReports& hidlReports,
        SystemStatusReports& systemReports, int32_t maxReports) {

    int count = hidlReports.base_1_0.mErrRecoveryVec.size();
    count = count < maxReports ? count : maxReports;
    if (count <= 0) {
        LOC_LOGE("Invalid count: %d", count);
        return;
    }

    for (int i=0; i < count && i < maxReports; i++) {

        SystemStatusErrRecovery status;

        status.mUtcTime.tv_sec = hidlReports.base_1_0.mErrRecoveryVec[i].mUtcTime.tv_sec;
        status.mUtcTime.tv_nsec = hidlReports.base_1_0.mErrRecoveryVec[i].mUtcTime.tv_nsec;

        status.mUtcReported.tv_sec = hidlReports.base_1_0.mErrRecoveryVec[i].mUtcReported.tv_sec;
        status.mUtcReported.tv_nsec = hidlReports.base_1_0.mErrRecoveryVec[i].mUtcReported.tv_nsec;
        systemReports.mErrRecovery.push_back(status);
    }
}

static void populateInjectedPositionReport(
        ILocHidlDebugReportService::LocHidlSystemStatusReports& hidlReports,
        SystemStatusReports& systemReports, int32_t maxReports) {

    int count = hidlReports.base_1_0.mInjectedPositionVec.size();
    count = count < maxReports ? count : maxReports;
    if (count <= 0) {
        LOC_LOGE("Invalid count: %d", count);
        return;
    }

    for (int i=0; i < count && i < maxReports; i++) {

        SystemStatusInjectedPosition status;

        status.mUtcTime.tv_sec = hidlReports.base_1_0.mInjectedPositionVec[i].mUtcTime.tv_sec;
        status.mUtcTime.tv_nsec = hidlReports.base_1_0.mInjectedPositionVec[i].mUtcTime.tv_nsec;

        status.mUtcReported.tv_sec =
                hidlReports.base_1_0.mInjectedPositionVec[i].mUtcReported.tv_sec;
        status.mUtcReported.tv_nsec =
                hidlReports.base_1_0.mInjectedPositionVec[i].mUtcReported.tv_nsec;

        status.mEpiValidity = hidlReports.base_1_0.mInjectedPositionVec[i].mEpiValidity;
        status.mEpiLat = hidlReports.base_1_0.mInjectedPositionVec[i].mEpiLat;
        status.mEpiLon = hidlReports.base_1_0.mInjectedPositionVec[i].mEpiLon;
        status.mEpiHepe = hidlReports.base_1_0.mInjectedPositionVec[i].mEpiHepe;
        status.mEpiAlt = hidlReports.base_1_0.mInjectedPositionVec[i].mEpiAlt;
        status.mEpiAltUnc = hidlReports.base_1_0.mInjectedPositionVec[i].mEpiAltUnc;
        status.mEpiSrc = hidlReports.base_1_0.mInjectedPositionVec[i].mEpiSrc;
        systemReports.mInjectedPosition.push_back(status);
    }
}

static void populateBestPositionReport(
        ILocHidlDebugReportService::LocHidlSystemStatusReports& hidlReports,
        SystemStatusReports& systemReports, int32_t maxReports) {

    int count = hidlReports.base_1_0.mBestPositionVec.size();
    count = count < maxReports ? count : maxReports;
    if (count <= 0) {
        LOC_LOGE("Invalid count: %d", count);
        return;
    }

    for (int i=0; i < count && i < maxReports; i++) {

        SystemStatusBestPosition status;

        status.mUtcTime.tv_sec = hidlReports.base_1_0.mBestPositionVec[i].mUtcTime.tv_sec;
        status.mUtcTime.tv_nsec = hidlReports.base_1_0.mBestPositionVec[i].mUtcTime.tv_nsec;

        status.mUtcReported.tv_sec = hidlReports.base_1_0.mBestPositionVec[i].mUtcReported.tv_sec;
        status.mUtcReported.tv_nsec = hidlReports.base_1_0.mBestPositionVec[i].mUtcReported.tv_nsec;

        status.mBestLat = hidlReports.base_1_0.mBestPositionVec[i].mBestLat;
        status.mBestLon = hidlReports.base_1_0.mBestPositionVec[i].mBestLon;
        status.mBestHepe = hidlReports.base_1_0.mBestPositionVec[i].mBestHepe;
        status.mBestAlt = hidlReports.base_1_0.mBestPositionVec[i].mBestAlt;
        status.mBestAltUnc = hidlReports.base_1_0.mBestPositionVec[i].mBestAltUnc;
        systemReports.mBestPosition.push_back(status);
    }
}

static void populateXtraReport(
        ILocHidlDebugReportService::LocHidlSystemStatusReports& hidlReports,
        SystemStatusReports& systemReports, int32_t maxReports) {

    int count = hidlReports.base_1_0.mXtraVec.size();
    count = count < maxReports ? count : maxReports;
    if (count <= 0) {
        LOC_LOGE("Invalid count: %d", count);
        return;
    }

    for (int i=0; i < count && i < maxReports; i++) {

        SystemStatusXtra status;

        status.mUtcTime.tv_sec = hidlReports.base_1_0.mXtraVec[i].mUtcTime.tv_sec;
        status.mUtcTime.tv_nsec = hidlReports.base_1_0.mXtraVec[i].mUtcTime.tv_nsec;

        status.mUtcReported.tv_sec = hidlReports.base_1_0.mXtraVec[i].mUtcReported.tv_sec;
        status.mUtcReported.tv_nsec = hidlReports.base_1_0.mXtraVec[i].mUtcReported.tv_nsec;

        status.mXtraValidMask = hidlReports.base_1_0.mXtraVec[i].mXtraValidMask;
        status.mGpsXtraAge = hidlReports.base_1_0.mXtraVec[i].mGpsXtraAge;
        status.mGloXtraAge = hidlReports.base_1_0.mXtraVec[i].mGloXtraAge;
        status.mBdsXtraAge = hidlReports.base_1_0.mXtraVec[i].mBdsXtraAge;
        status.mGalXtraAge = hidlReports.base_1_0.mXtraVec[i].mGalXtraAge;
        status.mQzssXtraAge = hidlReports.base_1_0.mXtraVec[i].mQzssXtraAge;
        status.mGpsXtraValid = hidlReports.base_1_0.mXtraVec[i].mGpsXtraValid;
        status.mGloXtraValid = hidlReports.base_1_0.mXtraVec[i].mGloXtraValid;
        status.mBdsXtraValid = hidlReports.base_1_0.mXtraVec[i].mBdsXtraValid;
        status.mGalXtraValid = hidlReports.base_1_0.mXtraVec[i].mGalXtraValid;
        status.mQzssXtraValid = hidlReports.base_1_0.mXtraVec[i].mQzssXtraValid;
        systemReports.mXtra.push_back(status);
    }
}

static void populateEphemerisReport(
        ILocHidlDebugReportService::LocHidlSystemStatusReports& hidlReports,
        SystemStatusReports& systemReports, int32_t maxReports) {

    int count = hidlReports.base_1_0.mEphemerisVec.size();
    count = count < maxReports ? count : maxReports;
    if (count <= 0) {
        LOC_LOGE("Invalid count: %d", count);
        return;
    }

    for (int i=0; i < count && i < maxReports; i++) {

        SystemStatusEphemeris status;

        status.mUtcTime.tv_sec = hidlReports.base_1_0.mEphemerisVec[i].mUtcTime.tv_sec;
        status.mUtcTime.tv_nsec = hidlReports.base_1_0.mEphemerisVec[i].mUtcTime.tv_nsec;

        status.mUtcReported.tv_sec = hidlReports.base_1_0.mEphemerisVec[i].mUtcReported.tv_sec;
        status.mUtcReported.tv_nsec = hidlReports.base_1_0.mEphemerisVec[i].mUtcReported.tv_nsec;

        status.mGpsEpheValid = hidlReports.base_1_0.mEphemerisVec[i].mGpsEpheValid;
        status.mGloEpheValid = hidlReports.base_1_0.mEphemerisVec[i].mGloEpheValid;
        status.mBdsEpheValid = hidlReports.base_1_0.mEphemerisVec[i].mBdsEpheValid;
        status.mGalEpheValid = hidlReports.base_1_0.mEphemerisVec[i].mGalEpheValid;
        status.mQzssEpheValid = hidlReports.base_1_0.mEphemerisVec[i].mQzssEpheValid;
        systemReports.mEphemeris.push_back(status);
    }
}

static void populateSvHealthReport(
        ILocHidlDebugReportService::LocHidlSystemStatusReports& hidlReports,
        SystemStatusReports& systemReports, int32_t maxReports) {

    int count = hidlReports.base_1_0.mSvHealthVec.size();
    count = count < maxReports ? count : maxReports;
    if (count <= 0) {
        LOC_LOGE("Invalid count: %d", count);
        return;
    }

    for (int i=0; i < count && i < maxReports; i++) {

        SystemStatusSvHealth status;

        status.mUtcTime.tv_sec = hidlReports.base_1_0.mSvHealthVec[i].mUtcTime.tv_sec;
        status.mUtcTime.tv_nsec = hidlReports.base_1_0.mSvHealthVec[i].mUtcTime.tv_nsec;

        status.mUtcReported.tv_sec = hidlReports.base_1_0.mSvHealthVec[i].mUtcReported.tv_sec;
        status.mUtcReported.tv_nsec = hidlReports.base_1_0.mSvHealthVec[i].mUtcReported.tv_nsec;

        status.mGpsGoodMask = hidlReports.base_1_0.mSvHealthVec[i].mGpsGoodMask;
        status.mGloGoodMask = hidlReports.base_1_0.mSvHealthVec[i].mGloGoodMask;
        status.mBdsGoodMask = hidlReports.base_1_0.mSvHealthVec[i].mBdsGoodMask;
        status.mGalGoodMask = hidlReports.base_1_0.mSvHealthVec[i].mGalGoodMask;
        status.mQzssGoodMask = hidlReports.base_1_0.mSvHealthVec[i].mQzssGoodMask;

        status.mGpsBadMask = hidlReports.base_1_0.mSvHealthVec[i].mGpsBadMask;
        status.mGloBadMask = hidlReports.base_1_0.mSvHealthVec[i].mGloBadMask;
        status.mBdsBadMask = hidlReports.base_1_0.mSvHealthVec[i].mBdsBadMask;
        status.mGalBadMask = hidlReports.base_1_0.mSvHealthVec[i].mGalBadMask;
        status.mQzssBadMask = hidlReports.base_1_0.mSvHealthVec[i].mQzssBadMask;

        status.mGpsUnknownMask = hidlReports.base_1_0.mSvHealthVec[i].mGpsUnknownMask;
        status.mGloUnknownMask = hidlReports.base_1_0.mSvHealthVec[i].mGloUnknownMask;
        status.mBdsUnknownMask = hidlReports.base_1_0.mSvHealthVec[i].mBdsUnknownMask;
        status.mGalUnknownMask = hidlReports.base_1_0.mSvHealthVec[i].mGalUnknownMask;
        status.mQzssUnknownMask = hidlReports.base_1_0.mSvHealthVec[i].mQzssUnknownMask;
        systemReports.mSvHealth.push_back(status);
    }
}

static void populatePdrReport(
        ILocHidlDebugReportService::LocHidlSystemStatusReports& hidlReports,
        SystemStatusReports& systemReports, int32_t maxReports) {

    int count = hidlReports.base_1_0.mPdrVec.size();
    count = count < maxReports ? count : maxReports;
    if (count <= 0) {
        LOC_LOGE("Invalid count: %d", count);
        return;
    }

    for (int i=0; i < count && i < maxReports; i++) {

        SystemStatusPdr status;

        status.mUtcTime.tv_sec = hidlReports.base_1_0.mPdrVec[i].mUtcTime.tv_sec;
        status.mUtcTime.tv_nsec = hidlReports.base_1_0.mPdrVec[i].mUtcTime.tv_nsec;
        status.mUtcReported.tv_sec = hidlReports.base_1_0.mPdrVec[i].mUtcReported.tv_sec;
        status.mUtcReported.tv_nsec = hidlReports.base_1_0.mPdrVec[i].mUtcReported.tv_nsec;
        status.mFixInfoMask = hidlReports.base_1_0.mPdrVec[i].mFixInfoMask;
        systemReports.mPdr.push_back(status);
    }
}

static void populateNavDataReport(
        ILocHidlDebugReportService::LocHidlSystemStatusReports& /*hidlReports*/,
        SystemStatusReports& /*systemReports*/, int32_t /*maxReports*/) {

    /* Not yet supported */
}

static void populatePositionFailureReport(
        ILocHidlDebugReportService::LocHidlSystemStatusReports& hidlReports,
        loc_core::SystemStatusReports& systemReports, int32_t maxReports) {

    int count = hidlReports.base_1_0.mPositionFailureVec.size();
    count = count < maxReports ? count : maxReports;
    if (count <= 0) {
        LOC_LOGE("Invalid count: %d", count);
        return;
    }

    for (int i=0; i < count && i < maxReports; i++) {

        SystemStatusPositionFailure status;

        status.mUtcTime.tv_sec = hidlReports.base_1_0.mPositionFailureVec[i].mUtcTime.tv_sec;
        status.mUtcTime.tv_nsec = hidlReports.base_1_0.mPositionFailureVec[i].mUtcTime.tv_nsec;
        status.mUtcReported.tv_sec =
                hidlReports.base_1_0.mPositionFailureVec[i].mUtcReported.tv_sec;
        status.mUtcReported.tv_nsec =
                hidlReports.base_1_0.mPositionFailureVec[i].mUtcReported.tv_nsec;
        status.mFixInfoMask = hidlReports.base_1_0.mPositionFailureVec[i].mFixInfoMask;
        status.mHepeLimit = hidlReports.base_1_0.mPositionFailureVec[i].mHepeLimit;
        systemReports.mPositionFailure.push_back(status);
    }
}

#endif
static void translateStatus(LocationError& error, IGnssGeofenceCallback::GeofenceStatus& status) {
    switch (status) {
        case IGnssGeofenceCallback::GeofenceStatus::OPERATION_SUCCESS:
            error = LOCATION_ERROR_SUCCESS;
            break;
        case IGnssGeofenceCallback::GeofenceStatus::ERROR_TOO_MANY_GEOFENCES:
            error = LOCATION_ERROR_GEOFENCES_AT_MAX;
            break;
        case IGnssGeofenceCallback::GeofenceStatus::ERROR_ID_EXISTS:
            error = LOCATION_ERROR_ID_EXISTS;
            break;
        case IGnssGeofenceCallback::GeofenceStatus::ERROR_ID_UNKNOWN:
            error = LOCATION_ERROR_ID_UNKNOWN;
            break;
        case IGnssGeofenceCallback::GeofenceStatus::ERROR_INVALID_TRANSITION:
            error = LOCATION_ERROR_INVALID_PARAMETER;
            break;
        default:
            error = LOCATION_ERROR_GENERAL_FAILURE;
    }
}

static void translateGnssConstellationType(GnssSvType& out,
        const ::android::hardware::gnss::V1_0::GnssConstellationType& in) {
    switch(in) {
        case ::android::hardware::gnss::V1_0::GnssConstellationType::GPS:
            out = GNSS_SV_TYPE_GPS;
            break;
        case ::android::hardware::gnss::V1_0::GnssConstellationType::SBAS:
            out = GNSS_SV_TYPE_SBAS;
            break;
        case ::android::hardware::gnss::V1_0::GnssConstellationType::GLONASS:
            out = GNSS_SV_TYPE_GLONASS;
            break;
        case ::android::hardware::gnss::V1_0::GnssConstellationType::QZSS:
            out = GNSS_SV_TYPE_QZSS;
            break;
        case ::android::hardware::gnss::V1_0::GnssConstellationType::BEIDOU:
            out = GNSS_SV_TYPE_BEIDOU;
            break;
        case ::android::hardware::gnss::V1_0::GnssConstellationType::GALILEO:
            out = GNSS_SV_TYPE_GALILEO;
            break;
        default:
            out = GNSS_SV_TYPE_UNKNOWN;
            break;
    }
}

static void translateGnssConstellationType(GnssSvType& out,
        const ::android::hardware::gnss::V2_0::GnssConstellationType& in) {
    switch(in) {
        case ::android::hardware::gnss::V2_0::GnssConstellationType::GPS:
            out = GNSS_SV_TYPE_GPS;
                break;
        case ::android::hardware::gnss::V2_0::GnssConstellationType::SBAS:
            out = GNSS_SV_TYPE_SBAS;
                break;
        case ::android::hardware::gnss::V2_0::GnssConstellationType::GLONASS:
            out = GNSS_SV_TYPE_GLONASS;
                break;
        case ::android::hardware::gnss::V2_0::GnssConstellationType::QZSS:
            out = GNSS_SV_TYPE_QZSS;
                break;
        case ::android::hardware::gnss::V2_0::GnssConstellationType::BEIDOU:
            out = GNSS_SV_TYPE_BEIDOU;
                break;
        case ::android::hardware::gnss::V2_0::GnssConstellationType::GALILEO:
            out = GNSS_SV_TYPE_GALILEO;
                break;
        case ::android::hardware::gnss::V2_0::GnssConstellationType::IRNSS:
            out = GNSS_SV_TYPE_NAVIC;
                break;
        default:
            out = GNSS_SV_TYPE_UNKNOWN;
                break;
    }
}

static void translateGnssMeasurementsCodeType(GnssMeasurementsCodeType& out,
        const ::android::hardware::hidl_string& in) {
    if (in.size() != 1) { //GNSS_MEASUREMENTS_CODE_TYPE_UNKNOWN
        out = GNSS_MEASUREMENTS_CODE_TYPE_OTHER;
        return;
    }
    std::string str(in.c_str());
    switch(str[0]) {
        case 'A':
            out = GNSS_MEASUREMENTS_CODE_TYPE_A;
            break;
        case 'B':
            out = GNSS_MEASUREMENTS_CODE_TYPE_B;
            break;
        case 'C':
            out = GNSS_MEASUREMENTS_CODE_TYPE_C;
            break;
        case 'I':
            out = GNSS_MEASUREMENTS_CODE_TYPE_I;
            break;
        case 'L':
            out = GNSS_MEASUREMENTS_CODE_TYPE_L;
            break;
        case 'M':
            out = GNSS_MEASUREMENTS_CODE_TYPE_M;
            break;
        case 'P':
            out = GNSS_MEASUREMENTS_CODE_TYPE_P;
            break;
        case 'Q':
            out = GNSS_MEASUREMENTS_CODE_TYPE_Q;
            break;
        case 'S':
            out = GNSS_MEASUREMENTS_CODE_TYPE_S;
            break;
        case 'W':
            out = GNSS_MEASUREMENTS_CODE_TYPE_W;
            break;
        case 'X':
            out = GNSS_MEASUREMENTS_CODE_TYPE_X;
            break;
        case 'Y':
            out = GNSS_MEASUREMENTS_CODE_TYPE_Y;
            break;
        case 'Z':
            out = GNSS_MEASUREMENTS_CODE_TYPE_Z;
            break;
        case 'N':
            out = GNSS_MEASUREMENTS_CODE_TYPE_N;
            break;
        default:
            out = GNSS_MEASUREMENTS_CODE_TYPE_OTHER;
    }
}

static void translateGnssClock(GnssMeasurementsClock& out,
        const IGnssMeasurementCallback::GnssClock& in) {
    memset(&out, 0, sizeof(out));
    if (in.gnssClockFlags & IGnssMeasurementCallback::GnssClockFlags::HAS_LEAP_SECOND) {
        out.flags |= GNSS_MEASUREMENTS_CLOCK_FLAGS_LEAP_SECOND_BIT;
    }
    if (in.gnssClockFlags & IGnssMeasurementCallback::GnssClockFlags::HAS_TIME_UNCERTAINTY) {
        out.flags |= GNSS_MEASUREMENTS_CLOCK_FLAGS_TIME_UNCERTAINTY_BIT;
    }
    if (in.gnssClockFlags & IGnssMeasurementCallback::GnssClockFlags::HAS_FULL_BIAS) {
        out.flags |= GNSS_MEASUREMENTS_CLOCK_FLAGS_FULL_BIAS_BIT;
    }
    if (in.gnssClockFlags & IGnssMeasurementCallback::GnssClockFlags::HAS_BIAS) {
        out.flags |= GNSS_MEASUREMENTS_CLOCK_FLAGS_BIAS_BIT;
    }
    if (in.gnssClockFlags & IGnssMeasurementCallback::GnssClockFlags::HAS_BIAS_UNCERTAINTY) {
        out.flags |= GNSS_MEASUREMENTS_CLOCK_FLAGS_BIAS_UNCERTAINTY_BIT;
    }
    in.gnssClockFlags & IGnssMeasurementCallback::GnssClockFlags::HAS_DRIFT; {
        out.flags & GNSS_MEASUREMENTS_CLOCK_FLAGS_DRIFT_BIT;
    }
    in.gnssClockFlags & IGnssMeasurementCallback::GnssClockFlags::HAS_DRIFT_UNCERTAINTY; {
        out.flags & GNSS_MEASUREMENTS_CLOCK_FLAGS_DRIFT_UNCERTAINTY_BIT;
    }
    out.leapSecond = in.leapSecond;
    out.timeNs = in.timeNs;
    out.timeUncertaintyNs = in.timeUncertaintyNs;
    out.fullBiasNs = in.fullBiasNs;
    out.biasNs = in.biasNs;
    out.biasUncertaintyNs = in.biasUncertaintyNs;
    out.driftNsps = in.driftNsps;
    out.driftUncertaintyNsps = in.driftUncertaintyNsps;
    out.hwClockDiscontinuityCount = in.hwClockDiscontinuityCount;
}

static void translateGnssMeasurement(GnssMeasurementsData& out,
        const android::hardware::gnss::V1_0::IGnssMeasurementCallback::GnssMeasurement& in) {
    memset(&out, 0, sizeof(out));
    out.size = sizeof(out);
    if (in.flags & IGnssMeasurementCallback::GnssMeasurementFlags::HAS_SNR) {
        out.flags |= GNSS_MEASUREMENTS_DATA_SIGNAL_TO_NOISE_RATIO_BIT;
    }
    if (in.flags & IGnssMeasurementCallback::GnssMeasurementFlags::HAS_CARRIER_FREQUENCY) {
        out.flags |= GNSS_MEASUREMENTS_DATA_CARRIER_FREQUENCY_BIT;
    }
    if (in.flags & IGnssMeasurementCallback::GnssMeasurementFlags::HAS_CARRIER_CYCLES) {
        out.flags |= GNSS_MEASUREMENTS_DATA_CARRIER_CYCLES_BIT;
    }
    if (in.flags & IGnssMeasurementCallback::GnssMeasurementFlags::HAS_CARRIER_PHASE) {
        out.flags |= GNSS_MEASUREMENTS_DATA_CARRIER_PHASE_BIT;
    }
    if (in.flags & IGnssMeasurementCallback::GnssMeasurementFlags::HAS_CARRIER_PHASE_UNCERTAINTY) {
        out.flags |= GNSS_MEASUREMENTS_DATA_CARRIER_PHASE_UNCERTAINTY_BIT;
    }
    if (in.flags & IGnssMeasurementCallback::GnssMeasurementFlags::HAS_AUTOMATIC_GAIN_CONTROL) {
        out.flags |= GNSS_MEASUREMENTS_DATA_AUTOMATIC_GAIN_CONTROL_BIT;
    }
    out.svId = in.svid;
    translateGnssConstellationType(out.svType, in.constellation);
    out.timeOffsetNs = in.timeOffsetNs;
    if (in.state & IGnssMeasurementCallback::GnssMeasurementState::STATE_CODE_LOCK) {
        out.stateMask |= GNSS_MEASUREMENTS_STATE_CODE_LOCK_BIT;
    }
    if (in.state & IGnssMeasurementCallback::GnssMeasurementState::STATE_BIT_SYNC) {
        out.stateMask |= GNSS_MEASUREMENTS_STATE_BIT_SYNC_BIT;
    }
    if (in.state & IGnssMeasurementCallback::GnssMeasurementState::STATE_SUBFRAME_SYNC) {
        out.stateMask |= GNSS_MEASUREMENTS_STATE_SUBFRAME_SYNC_BIT;
    }
    if (in.state & IGnssMeasurementCallback::GnssMeasurementState::STATE_TOW_DECODED) {
        out.stateMask |= GNSS_MEASUREMENTS_STATE_TOW_DECODED_BIT;
    }
    if (in.state & IGnssMeasurementCallback::GnssMeasurementState::STATE_MSEC_AMBIGUOUS) {
        out.stateMask |= GNSS_MEASUREMENTS_STATE_MSEC_AMBIGUOUS_BIT;
    }
    if (in.state & IGnssMeasurementCallback::GnssMeasurementState::STATE_SYMBOL_SYNC) {
        out.stateMask |= GNSS_MEASUREMENTS_STATE_SYMBOL_SYNC_BIT;
    }
    if (in.state & IGnssMeasurementCallback::GnssMeasurementState::STATE_GLO_STRING_SYNC) {
        out.stateMask |= GNSS_MEASUREMENTS_STATE_GLO_STRING_SYNC_BIT;
    }
    if (in.state & IGnssMeasurementCallback::GnssMeasurementState::STATE_GLO_TOD_DECODED) {
        out.stateMask |= GNSS_MEASUREMENTS_STATE_GLO_TOD_DECODED_BIT;
    }
    if (in.state & IGnssMeasurementCallback::GnssMeasurementState::STATE_BDS_D2_BIT_SYNC) {
        out.stateMask |= GNSS_MEASUREMENTS_STATE_BDS_D2_BIT_SYNC_BIT;
    }
    if (in.state & IGnssMeasurementCallback::GnssMeasurementState::STATE_BDS_D2_SUBFRAME_SYNC) {
        out.stateMask |= GNSS_MEASUREMENTS_STATE_BDS_D2_SUBFRAME_SYNC_BIT;
    }
    if (in.state & IGnssMeasurementCallback::GnssMeasurementState::STATE_GAL_E1BC_CODE_LOCK) {
        out.stateMask |= GNSS_MEASUREMENTS_STATE_GAL_E1BC_CODE_LOCK_BIT;
    }
    if (in.state & IGnssMeasurementCallback::GnssMeasurementState::STATE_GAL_E1C_2ND_CODE_LOCK) {
        out.stateMask |= GNSS_MEASUREMENTS_STATE_GAL_E1C_2ND_CODE_LOCK_BIT;
    }
    if (in.state & IGnssMeasurementCallback::GnssMeasurementState::STATE_GAL_E1B_PAGE_SYNC) {
        out.stateMask |= GNSS_MEASUREMENTS_STATE_GAL_E1B_PAGE_SYNC_BIT;
    }
    if (in.state & IGnssMeasurementCallback::GnssMeasurementState::STATE_SBAS_SYNC) {
        out.stateMask |=  GNSS_MEASUREMENTS_STATE_SBAS_SYNC_BIT;
    }
    out.receivedSvTimeNs = in.receivedSvTimeInNs;
    out.receivedSvTimeUncertaintyNs = in.receivedSvTimeUncertaintyInNs;
    out.carrierToNoiseDbHz = in.cN0DbHz;
    out.pseudorangeRateMps = in.pseudorangeRateMps;
    out.pseudorangeRateUncertaintyMps = in.pseudorangeRateUncertaintyMps;
    if (in.accumulatedDeltaRangeState &
            IGnssMeasurementCallback::GnssAccumulatedDeltaRangeState::ADR_STATE_VALID) {
        out.adrStateMask |= GNSS_MEASUREMENTS_ACCUMULATED_DELTA_RANGE_STATE_VALID_BIT;
    }
    if (in.accumulatedDeltaRangeState &
            IGnssMeasurementCallback::GnssAccumulatedDeltaRangeState::ADR_STATE_RESET) {
        out.adrStateMask |= GNSS_MEASUREMENTS_ACCUMULATED_DELTA_RANGE_STATE_RESET_BIT;
    }
    if (in.accumulatedDeltaRangeState &
            IGnssMeasurementCallback::GnssAccumulatedDeltaRangeState::ADR_STATE_CYCLE_SLIP) {
        out.adrStateMask |= GNSS_MEASUREMENTS_ACCUMULATED_DELTA_RANGE_STATE_CYCLE_SLIP_BIT;
    }
    out.adrMeters = in.accumulatedDeltaRangeM;
    out.adrUncertaintyMeters = in.accumulatedDeltaRangeUncertaintyM;
    out.carrierFrequencyHz = in.carrierFrequencyHz;
    out.carrierCycles = in.carrierCycles;
    out.carrierPhase = in.carrierPhase;
    out.carrierPhaseUncertainty = in.carrierPhaseUncertainty;
    out.multipathIndicator = static_cast<GnssMeasurementsMultipathIndicator>(in.multipathIndicator);
    out.signalToNoiseRatioDb = in.snrDb;
    out.agcLevelDb = in.agcLevelDb;
}

static void translateGnssData(GnssMeasurementsNotification& out,
        const IGnssMeasurementCallback::GnssData& in) {
    out.size = sizeof(out);
    out.count = in.measurements.size();
    translateGnssClock(out.clock, in.clock);
    for (size_t i = 0; i < out.count; i++) {
        translateGnssMeasurement(out.measurements[i], in.measurements[i].v1_1.v1_0);
        translateGnssConstellationType(out.measurements[i].svType,
                in.measurements[i].constellation);
        translateGnssMeasurementsCodeType(out.measurements[i].codeType,
                in.measurements[i].codeType);
        if (in.measurements[i].v1_1.accumulatedDeltaRangeState &
                IGnssMeasurementCallback::GnssAccumulatedDeltaRangeState::ADR_STATE_VALID) {
            out.measurements[i].adrStateMask |=
                    GNSS_MEASUREMENTS_ACCUMULATED_DELTA_RANGE_STATE_VALID_BIT;
        }
        if (in.measurements[i].v1_1.accumulatedDeltaRangeState &
                IGnssMeasurementCallback::GnssAccumulatedDeltaRangeState::ADR_STATE_RESET) {
            out.measurements[i].adrStateMask |=
                    GNSS_MEASUREMENTS_ACCUMULATED_DELTA_RANGE_STATE_RESET_BIT;
        }
        if (in.measurements[i].v1_1.accumulatedDeltaRangeState &
                IGnssMeasurementCallback::GnssAccumulatedDeltaRangeState::ADR_STATE_CYCLE_SLIP) {
            out.measurements[i].adrStateMask |=
                    GNSS_MEASUREMENTS_ACCUMULATED_DELTA_RANGE_STATE_CYCLE_SLIP_BIT;
        }
        if (in.measurements[i].v1_1.accumulatedDeltaRangeState &
                IGnssMeasurementCallback::GnssAccumulatedDeltaRangeState::ADR_STATE_HALF_CYCLE_RESOLVED) {
            out.measurements[i].adrStateMask |=
                    GNSS_MEASUREMENTS_ACCUMULATED_DELTA_RANGE_STATE_HALF_CYCLE_RESOLVED_BIT;
        }
        if (in.measurements[i].state &
                IGnssMeasurementCallback::GnssMeasurementState::STATE_CODE_LOCK) {
            out.measurements[i].stateMask |= GNSS_MEASUREMENTS_STATE_CODE_LOCK_BIT;
        }
        if (in.measurements[i].state &
                IGnssMeasurementCallback::GnssMeasurementState::STATE_BIT_SYNC) {
            out.measurements[i].stateMask |= GNSS_MEASUREMENTS_STATE_BIT_SYNC_BIT;
        }
        if (in.measurements[i].state &
                IGnssMeasurementCallback::GnssMeasurementState::STATE_SUBFRAME_SYNC) {
            out.measurements[i].stateMask |= GNSS_MEASUREMENTS_STATE_SUBFRAME_SYNC_BIT;
        }
        if (in.measurements[i].state &
                IGnssMeasurementCallback::GnssMeasurementState::STATE_TOW_DECODED) {
            out.measurements[i].stateMask |= GNSS_MEASUREMENTS_STATE_TOW_DECODED_BIT;
        }
        if (in.measurements[i].state &
                IGnssMeasurementCallback::GnssMeasurementState::STATE_MSEC_AMBIGUOUS) {
            out.measurements[i].stateMask |= GNSS_MEASUREMENTS_STATE_MSEC_AMBIGUOUS_BIT;
        }
        if (in.measurements[i].state &
                IGnssMeasurementCallback::GnssMeasurementState::STATE_SYMBOL_SYNC) {
            out.measurements[i].stateMask |= GNSS_MEASUREMENTS_STATE_SYMBOL_SYNC_BIT;
        }
        if (in.measurements[i].state &
                IGnssMeasurementCallback::GnssMeasurementState::STATE_GLO_STRING_SYNC) {
            out.measurements[i].stateMask |= GNSS_MEASUREMENTS_STATE_GLO_STRING_SYNC_BIT;
        }
        if (in.measurements[i].state &
                IGnssMeasurementCallback::GnssMeasurementState::STATE_GLO_TOD_DECODED) {
            out.measurements[i].stateMask |= GNSS_MEASUREMENTS_STATE_GLO_TOD_DECODED_BIT;
        }
        if (in.measurements[i].state &
                IGnssMeasurementCallback::GnssMeasurementState::STATE_BDS_D2_BIT_SYNC) {
            out.measurements[i].stateMask |= GNSS_MEASUREMENTS_STATE_BDS_D2_BIT_SYNC_BIT;
        }
        if (in.measurements[i].state &
                IGnssMeasurementCallback::GnssMeasurementState::STATE_BDS_D2_SUBFRAME_SYNC) {
            out.measurements[i].stateMask |= GNSS_MEASUREMENTS_STATE_BDS_D2_SUBFRAME_SYNC_BIT;
        }
        if (in.measurements[i].state &
                IGnssMeasurementCallback::GnssMeasurementState::STATE_GAL_E1BC_CODE_LOCK) {
            out.measurements[i].stateMask |= GNSS_MEASUREMENTS_STATE_GAL_E1BC_CODE_LOCK_BIT;
        }
        if (in.measurements[i].state &
                IGnssMeasurementCallback::GnssMeasurementState::STATE_GAL_E1C_2ND_CODE_LOCK) {
            out.measurements[i].stateMask |= GNSS_MEASUREMENTS_STATE_GAL_E1C_2ND_CODE_LOCK_BIT;
        }
        if (in.measurements[i].state &
                IGnssMeasurementCallback::GnssMeasurementState::STATE_GAL_E1B_PAGE_SYNC) {
            out.measurements[i].stateMask |= GNSS_MEASUREMENTS_STATE_GAL_E1B_PAGE_SYNC_BIT;
        }
        if (in.measurements[i].state &
                IGnssMeasurementCallback::GnssMeasurementState::STATE_SBAS_SYNC) {
            out.measurements[i].stateMask |=  GNSS_MEASUREMENTS_STATE_SBAS_SYNC_BIT;
        }
        if (in.measurements[i].state &
                IGnssMeasurementCallback::GnssMeasurementState::STATE_TOW_KNOWN) {
            out.measurements[i].stateMask |=  GNSS_MEASUREMENTS_STATE_TOW_KNOWN_BIT;
        }
        if (in.measurements[i].state &
                IGnssMeasurementCallback::GnssMeasurementState::STATE_GLO_TOD_KNOWN) {
            out.measurements[i].stateMask |=  GNSS_MEASUREMENTS_STATE_GLO_TOD_KNOWN_BIT;
        }
        if (in.measurements[i].state &
                IGnssMeasurementCallback::GnssMeasurementState::STATE_2ND_CODE_LOCK) {
            out.measurements[i].stateMask |=  GNSS_MEASUREMENTS_STATE_2ND_CODE_LOCK_BIT;
        }
    }
}

static void translateGnssEphemerisType(GnssEphemerisType& out,
        const IGnssDebug::SatelliteEphemerisType& in) {
    switch(in) {
        case IGnssDebug::SatelliteEphemerisType::EPHEMERIS:
            out = GNSS_EPH_TYPE_EPHEMERIS;
            break;
        case IGnssDebug::SatelliteEphemerisType::ALMANAC_ONLY:
            out = GNSS_EPH_TYPE_ALMANAC;
            break;
        default:
            out = GNSS_EPH_TYPE_UNKNOWN;
            break;
    }
}

static void translateGnssEphemerisSource(GnssEphemerisSource& out,
        const IGnssDebug::SatelliteEphemerisSource& in) {
    switch(in) {
        case IGnssDebug::SatelliteEphemerisSource::DEMODULATED:
            out = GNSS_EPH_SOURCE_DEMODULATED;
            break;
        case IGnssDebug::SatelliteEphemerisSource::SUPL_PROVIDED:
            out = GNSS_EPH_SOURCE_SUPL_PROVIDED;
            break;
        case IGnssDebug::SatelliteEphemerisSource::OTHER_SERVER_PROVIDED:
            out = GNSS_EPH_SOURCE_OTHER_SERVER_PROVIDED;
            break;
        case IGnssDebug::SatelliteEphemerisSource::OTHER:
        default:
            out = GNSS_EPH_SOURCE_UNKNOWN;
            break;
    }
}

static void translateGnssEphemerisHealth(GnssEphemerisHealth& out,
        const IGnssDebug::SatelliteEphemerisHealth& in) {
    switch(in) {
        case IGnssDebug::SatelliteEphemerisHealth::GOOD:
            out = GNSS_EPH_HEALTH_GOOD;
            break;
        case IGnssDebug::SatelliteEphemerisHealth::BAD:
            out = GNSS_EPH_HEALTH_BAD;
            break;
        default:
            out = GNSS_EPH_HEALTH_UNKNOWN;
            break;
    }
}

static void translateGnssDebugReport(GnssDebugReport& reports,
        const ::android::hardware::gnss::V2_0::IGnssDebug::DebugData& data) {
    if (data.position.valid) {
        reports.mLocation.mValid = true;
        reports.mLocation.mLocation.latitude = data.position.latitudeDegrees;
        reports.mLocation.mLocation.longitude = data.position.longitudeDegrees;
        reports.mLocation.mLocation.altitude = data.position.altitudeMeters;

        reports.mLocation.mLocation.speed = data.position.speedMetersPerSec;
        reports.mLocation.mLocation.bearing = data.position.bearingDegrees;
        reports.mLocation.mLocation.accuracy = data.position.horizontalAccuracyMeters;
        reports.mLocation.verticalAccuracyMeters = data.position.verticalAccuracyMeters;
        reports.mLocation.speedAccuracyMetersPerSecond = data.position.speedAccuracyMetersPerSecond;
        reports.mLocation.bearingAccuracyDegrees = data.position.bearingAccuracyDegrees;

        reports.mTime.size = sizeof(reports.mTime);
        reports.mTime.mValid = true;
        reports.mTime.timeEstimate = data.time.timeEstimate;
        reports.mTime.timeUncertaintyNs = data.time.timeUncertaintyNs;
        reports.mTime.frequencyUncertaintyNsPerSec = data.time.frequencyUncertaintyNsPerSec;

    }
    else {
        reports.mLocation.mValid = false;
        reports.mTime.mValid = false;
    }

    if (reports.mLocation.verticalAccuracyMeters <= 0 ||
        reports.mLocation.verticalAccuracyMeters > GNSS_DEBUG_UNKNOWN_VERTICAL_ACCURACY_METERS) {
        reports.mLocation.verticalAccuracyMeters = GNSS_DEBUG_UNKNOWN_VERTICAL_ACCURACY_METERS;
    }
    if (reports.mLocation.speedAccuracyMetersPerSecond <= 0 ||
        reports.mLocation.speedAccuracyMetersPerSecond > GNSS_DEBUG_UNKNOWN_SPEED_ACCURACY_PER_SEC){
        reports.mLocation.speedAccuracyMetersPerSecond = GNSS_DEBUG_UNKNOWN_SPEED_ACCURACY_PER_SEC;
    }
    if (reports.mLocation.bearingAccuracyDegrees <= 0 ||
        reports.mLocation.bearingAccuracyDegrees > GNSS_DEBUG_UNKNOWN_BEARING_ACCURACY_DEG) {
        reports.mLocation.bearingAccuracyDegrees = GNSS_DEBUG_UNKNOWN_BEARING_ACCURACY_DEG;
    }


    if (reports.mTime.timeEstimate < GNSS_DEBUG_UNKNOWN_UTC_TIME) {
        reports.mTime.timeEstimate = GNSS_DEBUG_UNKNOWN_UTC_TIME;
    }
    if (reports.mTime.timeUncertaintyNs <= 0) {
        reports.mTime.timeUncertaintyNs = (float)GNSS_DEBUG_UNKNOWN_UTC_TIME_UNC_MIN;
    }
    else if (reports.mTime.timeUncertaintyNs > GNSS_DEBUG_UNKNOWN_UTC_TIME_UNC_MAX) {
        reports.mTime.timeUncertaintyNs = (float)GNSS_DEBUG_UNKNOWN_UTC_TIME_UNC_MAX;
    }
    if (reports.mTime.frequencyUncertaintyNsPerSec <= 0 ||
        reports.mTime.frequencyUncertaintyNsPerSec > (float)GNSS_DEBUG_UNKNOWN_FREQ_UNC_NS_PER_SEC){
        reports.mTime.frequencyUncertaintyNsPerSec = (float)GNSS_DEBUG_UNKNOWN_FREQ_UNC_NS_PER_SEC;
    }

    // satellite data block
    GnssDebugSatelliteInfo s = { };
    std::vector<::android::hardware::gnss::V2_0::IGnssDebug::SatelliteData> s_array;

    for (uint32_t i=0; i<data.satelliteDataArray.size(); i++) {
        s.svid = data.satelliteDataArray[i].v1_0.svid;
        translateGnssConstellationType(s.constellation,
            data.satelliteDataArray[i].v1_0.constellation);
        translateGnssEphemerisType(
            s.mEphemerisType, data.satelliteDataArray[i].v1_0.ephemerisType);
        translateGnssEphemerisSource(
            s.mEphemerisSource, data.satelliteDataArray[i].v1_0.ephemerisSource);
        translateGnssEphemerisHealth(
            s.mEphemerisHealth, data.satelliteDataArray[i].v1_0.ephemerisHealth);

        s.ephemerisAgeSeconds =
            data.satelliteDataArray[i].v1_0.ephemerisAgeSeconds;
        s.serverPredictionIsAvailable =
            data.satelliteDataArray[i].v1_0.serverPredictionIsAvailable;
        s.serverPredictionAgeSeconds =
            data.satelliteDataArray[i].v1_0.serverPredictionAgeSeconds;

        reports.mSatelliteInfo.push_back(s);
    }
}

static void translateGnssNfwNotification(GnssNfwNotification& out,
    const IGnssVisibilityControlCallback::NfwNotification& in) {
    memset(&out, 0, sizeof(IGnssVisibilityControlCallback::NfwNotification));
    strlcpy(out.proxyAppPackageName, in.proxyAppPackageName.c_str(),
            in.proxyAppPackageName.size()<GNSS_MAX_NFW_APP_STRING_LEN?
            in.proxyAppPackageName.size():GNSS_MAX_NFW_APP_STRING_LEN);
    out.protocolStack = (GnssNfwProtocolStack)in.protocolStack;
    strlcpy(out.otherProtocolStackName, in.otherProtocolStackName.c_str(),
            in.otherProtocolStackName.size()<GNSS_MAX_NFW_STRING_LEN?
            in.otherProtocolStackName.size():GNSS_MAX_NFW_STRING_LEN);

    out.requestor = (GnssNfwRequestor)in.requestor;
    strlcpy(out.requestorId, in.requestorId.c_str(), in.requestorId.size()<GNSS_MAX_NFW_STRING_LEN?
            in.requestorId.size():GNSS_MAX_NFW_STRING_LEN);
    out.responseType = (GnssNfwResponseType)in.responseType;
    out.inEmergencyMode = in.inEmergencyMode;
    out.isCachedLocation = in.isCachedLocation;
}
/*
 * GnssCallback class implements the callback methods for IGnss interface.
 */
struct GnssCallback : public IGnssCallback {
    Return<void> gnssLocationCb(const GnssLocation& location) override;
    Return<void> gnssStatusCb(const IGnssCallback::GnssStatusValue status) override;
    Return<void> gnssSvStatusCb(const IGnssCallback::GnssSvStatus& svStatus) override;
    Return<void> gnssNmeaCb(android::hardware::gnss::V1_0::GnssUtcTime timestamp,
            const ::android::hardware::hidl_string& nmea) override;
    Return<void> gnssSetCapabilitesCb(uint32_t capabilities) override;
    Return<void> gnssAcquireWakelockCb() override;
    Return<void> gnssReleaseWakelockCb() override;
    Return<void> gnssRequestTimeCb() override;
    Return<void> gnssSetSystemInfoCb(const IGnssCallback::GnssSystemInfo& info) override;
    Return<void> gnssRequestLocationCb(const bool independentFromGnss) override;;
    // New in 1.1
    Return<void> gnssNameCb(const android::hardware::hidl_string& name) override;

    // New in 2.0
    Return<void> gnssRequestLocationCb_2_0(
            const bool independentFromGnss, const bool isUserEmergency) override;
    Return<void> gnssSetCapabilitiesCb_2_0(uint32_t capabilities) override;
    Return<void> gnssLocationCb_2_0(const GnssLocation_v2_0& location) override;
    Return<void> gnssSvStatusCb_2_0(const hidl_vec<IGnssCallback::GnssSvInfo>& svInfoList) override;
};

/*
 * GnssBatchingCallback interface implements the callback methods
 * required by the IGnssBatching interface.
 */
struct GnssBatchingCallback : public IGnssBatchingCallback {
    /*
    * Methods from ::android::hardware::gps::V2_0::IGnssBatchingCallback
    * follow.
    */
    Return<void> gnssLocationBatchCb(
            const hidl_vec<GnssLocation_v2_0>& locations) override;
};

struct GnssGeofenceCallback : public IGnssGeofenceCallback {
    // Methods from ::android::hardware::gps::V1_0::IGnssGeofenceCallback follow.
    Return<void> gnssGeofenceTransitionCb(int32_t geofenceId, const GnssLocation& location,
            GeofenceTransition transition,
            android::hardware::gnss::V1_0::GnssUtcTime timestamp) override;
    Return<void> gnssGeofenceStatusCb(GeofenceAvailability status,
            const GnssLocation& location) override;
    Return<void> gnssGeofenceAddCb(int32_t geofenceId, GeofenceStatus status) override;
    Return<void> gnssGeofenceRemoveCb(int32_t geofenceId, GeofenceStatus status) override;
    Return<void> gnssGeofencePauseCb(int32_t geofenceId, GeofenceStatus status) override;
    Return<void> gnssGeofenceResumeCb(int32_t geofenceId, GeofenceStatus status) override;
};

Return<void> GnssCallback::gnssLocationCb(const GnssLocation& gnssLocation) {
    Location location;
    memset(&location, 0, sizeof(Location));
    translateLocation(location, gnssLocation);
    LOC_LOGD("%s]: (flags: %02x)", __FUNCTION__, location.flags);

    lock_guard guard(sGnssLock);
    if (sGnssCbs != nullptr) {
        sGnssCbs->gnssLocationCb(location);
    }
    return Void();
}

Return<void> GnssCallback::gnssStatusCb(const IGnssCallback::GnssStatusValue status) {
    lock_guard guard(sGnssLock);
    if (sGnssCbs != nullptr) {
        sGnssCbs->gnssStatusCb(static_cast<uint32_t>(status));
    }
    return Void();
}

Return<void> GnssCallback::gnssSvStatusCb(const IGnssCallback::GnssSvStatus& svStatus) {
    GnssSvNotification gnssSvNotification;
    memset(&gnssSvNotification, 0, sizeof(GnssSvNotification));
    gnssSvNotification.size = sizeof(GnssSvNotification);
    uint32_t numSvs = std::min((uint32_t)svStatus.numSvs,
            std::min(static_cast<uint32_t>(GnssMax::SVS_COUNT), (uint32_t)(GNSS_SV_MAX)));
    if (numSvs > 0) {
        gnssSvNotification.count = numSvs;
        for (int i=0; i< numSvs; ++i) {
            gnssSvNotification.gnssSvs[i].size = sizeof(GnssSv);
            gnssSvNotification.gnssSvs[i].svId = static_cast<uint16_t>(svStatus.gnssSvList[i].svid);
            gnssSvNotification.gnssSvs[i].type =
                static_cast<GnssSvType>(svStatus.gnssSvList[i].constellation);
            gnssSvNotification.gnssSvs[i].cN0Dbhz = svStatus.gnssSvList[i].cN0Dbhz;
            gnssSvNotification.gnssSvs[i].elevation = svStatus.gnssSvList[i].elevationDegrees;
            gnssSvNotification.gnssSvs[i].azimuth = svStatus.gnssSvList[i].azimuthDegrees;
            gnssSvNotification.gnssSvs[i].gnssSvOptionsMask =
                static_cast<uint16_t>(svStatus.gnssSvList[i].svFlag);
        }
        LOC_LOGd("(count: %zu)", gnssSvNotification.count);
        lock_guard guard(sGnssLock);
        if (sGnssCbs != nullptr) {
            sGnssCbs->gnssSvStatusCb(gnssSvNotification);
        }
    } else {
        LOC_LOGe("Invalid Sv count");
    }
    return Void();
}

Return<void> GnssCallback::gnssNmeaCb(android::hardware::gnss::V1_0::GnssUtcTime timestamp,
        const ::android::hardware::hidl_string& nmea) {
    lock_guard guard(sGnssLock);
    if (sGnssCbs != nullptr) {
        sGnssCbs->gnssNmeaCb(timestamp, nmea.c_str(), nmea.size());
    }
    return Void();
}

Return<void> GnssCallback::gnssSetCapabilitesCb(uint32_t capabilities) {
    LOC_LOGD("%s]: (%02x)", __FUNCTION__, capabilities);
    lock_guard guard(sGnssLock);
    if (sGnssCbs != nullptr) {
        sGnssCbs->gnssSetCapabilitiesCb(capabilities);
    }
    return Void();
}

Return<void> GnssCallback::gnssAcquireWakelockCb() {
    return Void();
}

Return<void> GnssCallback::gnssReleaseWakelockCb() {
    return Void();
}

Return<void> GnssCallback::gnssRequestTimeCb() {
    lock_guard guard(sGnssLock);
    if (sGnssCbs != nullptr) {
        sGnssCbs->gnssRequestUtcTimeCb();
    }
    return Void();
}
Return<void> GnssCallback::gnssRequestLocationCb(const bool independentFromGnss) {
    lock_guard guard(sGnssLock);
    if (sGnssCbs != nullptr) {
        sGnssCbs->gnssRequestLocationCb(independentFromGnss, false);
    }
    return Void();
}

Return<void> GnssCallback::gnssSetSystemInfoCb(const IGnssCallback::GnssSystemInfo& info) {
    return Void();
}

Return<void> GnssCallback::gnssNameCb(const android::hardware::hidl_string& name) {
    return Void();
}

Return<void> GnssCallback::gnssRequestLocationCb_2_0(
        const bool independentFromGnss, const bool isUserEmergency) {
    lock_guard guard(sGnssLock);
    if (sGnssCbs != nullptr) {
        sGnssCbs->gnssRequestLocationCb(independentFromGnss,isUserEmergency);
    }
    return Void();
}

Return<void> GnssCallback::gnssSetCapabilitiesCb_2_0(uint32_t capabilities) {
    LOC_LOGD("%s]: (%02x)", __FUNCTION__, capabilities);
    lock_guard guard(sGnssLock);
    if (sGnssCbs != nullptr) {
        sGnssCbs->gnssSetCapabilitiesCb(capabilities);
    }
    return Void();
}

Return<void> GnssCallback::gnssLocationCb_2_0(const GnssLocation_v2_0& gnssLocation) {
    Location location;
    memset(&location, 0, sizeof(Location));
    translateLocation(location, gnssLocation.v1_0);
    LOC_LOGD("%s]: (flags: %02x)", __FUNCTION__, location.flags);

    lock_guard guard(sGnssLock);
    if (sGnssCbs != nullptr) {
        sGnssCbs->gnssLocationCb(location);
    }
    return Void();
}

Return<void> GnssCallback::gnssSvStatusCb_2_0(
        const hidl_vec<IGnssCallback::GnssSvInfo>& svInfoList) {
    GnssSvNotification gnssSvNotification;
    memset(&gnssSvNotification, 0, sizeof(GnssSvNotification));
    gnssSvNotification.size = sizeof(GnssSvNotification);
    uint32_t numSvs = std::min((uint32_t)svInfoList.size(),
            std::min(static_cast<uint32_t>(GnssMax::SVS_COUNT), (uint32_t)(GNSS_SV_MAX)));
    if (numSvs > 0) {
        gnssSvNotification.count = numSvs;
        for (int i=0; i< numSvs; ++i) {
            gnssSvNotification.gnssSvs[i].size = sizeof(GnssSv);
            gnssSvNotification.gnssSvs[i].svId = static_cast<uint16_t>(svInfoList[i].v1_0.svid);
            gnssSvNotification.gnssSvs[i].type =
                    static_cast<GnssSvType>(svInfoList[i].constellation);
            gnssSvNotification.gnssSvs[i].cN0Dbhz = svInfoList[i].v1_0.cN0Dbhz;
            gnssSvNotification.gnssSvs[i].elevation = svInfoList[i].v1_0.elevationDegrees;
            gnssSvNotification.gnssSvs[i].azimuth = svInfoList[i].v1_0.azimuthDegrees;
            gnssSvNotification.gnssSvs[i].carrierFrequencyHz =
                    svInfoList[i].v1_0.carrierFrequencyHz;
            gnssSvNotification.gnssSvs[i].gnssSvOptionsMask =
                static_cast<uint16_t>(svInfoList[i].v1_0.svFlag);
        }
        LOC_LOGd("(count: %zu)", gnssSvNotification.count);
        lock_guard guard(sGnssLock);
        if (sGnssCbs != nullptr) {
            sGnssCbs->gnssSvStatusCb(gnssSvNotification);
        }
    } else {
        LOC_LOGe("Invalid Sv count");
    }
    return Void();

}

Return<void> GnssBatchingCallback::gnssLocationBatchCb(
        const hidl_vec<GnssLocation_v2_0>& locations) {
    size_t length = locations.size();
    LOC_LOGd("(count: %zu)", length);
    lock_guard guard(sFlpLock);
    Location *location = new Location[length];
    if (nullptr == location) {
        LOC_LOGe("fail to alloc memory");
        return Void();
    }
    if(sFlpCbs != nullptr) {
        for (int i=0; i<length; ++i) {
            memset(&location[i], 0, sizeof(Location));
            translateLocation(location[i], locations[i].v1_0);
        }
        sFlpCbs->batchingCb(length, location);
    }
    delete[] location;
    return Void();
}

Return<void> GnssGeofenceCallback::gnssGeofenceTransitionCb( int32_t geofenceId,
        const GnssLocation& gnssLocation, GeofenceTransition transition,
        android::hardware::gnss::V1_0::GnssUtcTime timestamp) {
    LOC_LOGD("%s]: (%d)", __func__, geofenceId);
    Location location;
    memset(&location, 0, sizeof(Location));
    translateLocation(location, gnssLocation);
    GeofenceBreachType geoBreach = GEOFENCE_BREACH_UNKNOWN;
    if (IGnssGeofenceCallback::GeofenceTransition::ENTERED == transition) {
        geoBreach = GEOFENCE_BREACH_ENTER;
    } else if (IGnssGeofenceCallback::GeofenceTransition::EXITED == transition) {
        geoBreach = GEOFENCE_BREACH_EXIT;
    }
    lock_guard guard(sGeofenceLock);
    if (sGeofenceCbs) {
        sGeofenceCbs->geofenceBreachCb(geofenceId, location, geoBreach,
                static_cast<uint64_t>(timestamp));
    }
    return Void();
}

Return<void> GnssGeofenceCallback::gnssGeofenceStatusCb(GeofenceAvailability status,
        const GnssLocation& location) {
    LOC_LOGD("%s]: (%d)", __func__, status);
    GeofenceStatusAvailable available = GEOFENCE_STATUS_AVAILABILE_NO;
    if (IGnssGeofenceCallback::GeofenceAvailability::AVAILABLE == status) {
        available = GEOFENCE_STATUS_AVAILABILE_YES;
    }
    lock_guard guard(sGeofenceLock);
    if (sGeofenceCbs) {
        sGeofenceCbs->geofenceStatusCb(available);
    }
    return Void();
}

Return<void> GnssGeofenceCallback::gnssGeofenceAddCb(int32_t geofenceId, GeofenceStatus status) {
    LOC_LOGD("%s]: (%d)", __func__, geofenceId);
    LocationError error;
    translateStatus(error, status);
    lock_guard guard(sGeofenceLock);
    if (sGeofenceCbs) {
        sGeofenceCbs->addGeofencesCb(geofenceId, error);
    }
    return Void();
}

Return<void> GnssGeofenceCallback::gnssGeofenceRemoveCb(int32_t geofenceId,
        GeofenceStatus status) {
    LOC_LOGD("%s]: (%d)", __func__, geofenceId);
    LocationError error;
    translateStatus(error, status);
    lock_guard guard(sGeofenceLock);
    if (sGeofenceCbs) {
        sGeofenceCbs->removeGeofencesCb(geofenceId, error);
    }
    return Void();
}

Return<void> GnssGeofenceCallback::gnssGeofencePauseCb(int32_t geofenceId,
        GeofenceStatus status) {
    LOC_LOGD("%s]: (%d)", __func__, geofenceId);
    LocationError error;
    translateStatus(error, status);
    lock_guard guard(sGeofenceLock);
    if (sGeofenceCbs) {
        sGeofenceCbs->pauseGeofencesCb(geofenceId, error);
    }
    return Void();
}

Return<void> GnssGeofenceCallback::gnssGeofenceResumeCb(int32_t geofenceId,
        GeofenceStatus status) {
    LOC_LOGD("%s]: (%d)", __func__, geofenceId);
    LocationError error;
    translateStatus(error, status);
    lock_guard guard(sGeofenceLock);
    if (sGeofenceCbs) {
        sGeofenceCbs->resumeGeofencesCb(geofenceId, error);
    }
    return Void();
}
#ifdef LOCHIDL
/* ===========================================================
 *   HIDL Callbacks : ILocHidlFlpServiceCallback.hal
 * ===========================================================*/
struct GnssFlpServiceCallback : public ILocHidlFlpServiceCallback {

    Return<void> gnssLocationTrackingCb(
      const LocHidlLocation& gnssLocation) override;

    Return<void> gnssLocationBatchingCb(
      const LocHidlBatchOptions &batchOptions,
      const hidl_vec<LocHidlLocation>& gnssLocations) override;

    Return<void> gnssBatchingStatusCb(
            const LocHidlBatchStatusInfo &batchStatus,
            const hidl_vec<uint32_t>& listOfCompletedTrips) override;

    Return<void> gnssMaxPowerAllocatedCb(int32_t powerInMW) override;
};

Return<void> GnssFlpServiceCallback::gnssLocationTrackingCb(
        const LocHidlLocation& gnssLocation) {
    LOC_LOGD("%s]: (flags: %02x)", __FUNCTION__, gnssLocation.locationFlagsMask);
    Location location;
    memset(&location, 0, sizeof(Location));
    translateLocation(location, gnssLocation);
    lock_guard guard(sFlpLock);
    if (sFlpCbs != nullptr) {
        sFlpCbs->gnssLocationCb(location);
    }
    return Void();
}

Return<void> GnssFlpServiceCallback::gnssLocationBatchingCb(
        const LocHidlBatchOptions &batchOptions,
        const hidl_vec<LocHidlLocation>& gnssLocations) {
    size_t length = gnssLocations.size();
    LOC_LOGd("(count: %zu)", length);
    lock_guard guard(sFlpLock);
    Location *location = new Location[length];
    if (nullptr == location) {
        LOC_LOGe("fail to alloc memory");
        return Void();
    }
    if(sFlpCbs != nullptr) {
        for (int i=0; i<length; ++i) {
            memset(&location[i], 0, sizeof(Location));
            translateLocation(location[i], gnssLocations[i]);
        }
        sFlpCbs->batchingCb(length, location);
    }
    delete[] location;
    return Void();
}

Return<void> GnssFlpServiceCallback::gnssBatchingStatusCb(
        const LocHidlBatchStatusInfo &batchStatusInfo,
        const hidl_vec<uint32_t>& listOfCompletedTrips) {
    return Void();
}

Return<void> GnssFlpServiceCallback::gnssMaxPowerAllocatedCb(
        int32_t powerInMW) {
    return Void();
}
#endif

/*
 * GnssNiCallback implements callback methods required by the IGnssNi interface.
 */
struct GnssNiCallback : public IGnssNiCallback {
    Return<void> niNotifyCb(const IGnssNiCallback::GnssNiNotification& notification)
            override;
};

Return<void> GnssNiCallback::niNotifyCb(
        const IGnssNiCallback::GnssNiNotification& notification) {
    LOC_LOGD("%s]: (id: %d)", __FUNCTION__, notification.notificationId);
    ::GnssNiNotification gnssNiNotification;
    memset(&gnssNiNotification, 0, sizeof(::GnssNiNotification));
    gnssNiNotification.size = sizeof(::GnssNiNotification);
    if (static_cast<uint8_t>(notification.niType) != 0) {
        gnssNiNotification.type = static_cast<::GnssNiType>((uint8_t)notification.niType-1);
    }
    gnssNiNotification.options = static_cast<uint16_t>(notification.notifyFlags);
    gnssNiNotification.timeout = notification.timeoutSec;
    gnssNiNotification.timeoutResponse = static_cast<GnssNiResponse>(notification.defaultResponse);
    memcpy(&gnssNiNotification.requestor, notification.requestorId.c_str(),
            notification.requestorId.size()<GNSS_NI_REQUESTOR_MAX?
            notification.requestorId.size():GNSS_NI_REQUESTOR_MAX);
    gnssNiNotification.requestorEncoding =
            static_cast<::GnssNiEncodingType>(notification.requestorIdEncoding);
    memcpy(&gnssNiNotification.message, notification.notificationMessage.c_str(),
            notification.notificationMessage.size()<GNSS_NI_MESSAGE_ID_MAX?
            notification.notificationMessage.size():GNSS_NI_MESSAGE_ID_MAX);
    gnssNiNotification.messageEncoding =
            static_cast<::GnssNiEncodingType>(notification.notificationIdEncoding);
    lock_guard guard(sGnssLock);
    if (sGnssCbs == nullptr) {
        gardenPrint("%s]: mGnssNiCbIface is nullptr", __FUNCTION__);
        return Void();
    }
    sGnssCbs->gnssNiNotifyCb(notification.notificationId, gnssNiNotification);
    return Void();
}

#ifdef LOCHIDL
/* =================================================
 *   HIDL Callbacks : ILocHidlGnssNiCallback.hal
 * =================================================*/
struct GnssNiVendorCallback : public ILocHidlGnssNiCallback {

    Return<void> niNotifyCb(
            const IGnssNiCallback::GnssNiNotification& notification) override;
    Return<void> niNotifyCbExt(
            const ILocHidlGnssNiCallback::GnssNiNotificationExt& notification) override;

    Return<void> gnssCapabilitiesCb(uint32_t capabilitiesMask) override;
};

Return<void> GnssNiVendorCallback::niNotifyCb(
        const IGnssNiCallback::GnssNiNotification&) {
    ALOGE("%s] Not supported !", __FUNCTION__);
    return Void();
}

Return<void> GnssNiVendorCallback::niNotifyCbExt(
        const ILocHidlGnssNiCallback::GnssNiNotificationExt& gnssNiNotification) {
    LOC_LOGD("%s]: (%d)", __func__, gnssNiNotification.notificationId);
    ::GnssNiNotification gnssNiNotif;
    memset(&gnssNiNotif, 0, sizeof(gnssNiNotif));
    gnssNiNotif.size = sizeof(gnssNiNotif);
    if (static_cast<uint8_t>(gnssNiNotification.niType) != 0) {
        gnssNiNotif.type = static_cast<::GnssNiType>((uint8_t)gnssNiNotification.niType-1);
    }
    gnssNiNotif.options = static_cast<uint16_t>(gnssNiNotification.notifyFlags);
    gnssNiNotif.timeout = gnssNiNotification.timeoutSec;
    gnssNiNotif.timeoutResponse = static_cast<GnssNiResponse>(gnssNiNotification.defaultResponse);
    memcpy(&gnssNiNotif.requestor, gnssNiNotification.requestorId.c_str(),
            gnssNiNotification.requestorId.size()<GNSS_NI_REQUESTOR_MAX?
            gnssNiNotification.requestorId.size():GNSS_NI_REQUESTOR_MAX);
    gnssNiNotif.requestorEncoding =
            static_cast<::GnssNiEncodingType>(gnssNiNotification.requestorIdEncoding);
    memcpy(&gnssNiNotif.message, gnssNiNotification.notificationMessage.c_str(),
            gnssNiNotification.notificationMessage.size()<GNSS_NI_MESSAGE_ID_MAX?
            gnssNiNotification.notificationMessage.size():GNSS_NI_MESSAGE_ID_MAX);
    gnssNiNotif.messageEncoding =
            static_cast<::GnssNiEncodingType>(gnssNiNotification.notificationIdEncoding);

    lock_guard guard(sGnssLock);
    if (sGnssCbs == nullptr) {
        gardenPrint("%s]: mGnssNiCbIface is nullptr", __FUNCTION__);
        return Void();
    }
    sGnssCbs->gnssNiNotifyCb(gnssNiNotification.notificationId, gnssNiNotif);
    return Void();
}

Return<void> GnssNiVendorCallback::gnssCapabilitiesCb(
        uint32_t capabilitiesMask) {
    ALOGE("%s] Not supported !", __FUNCTION__);
    return Void();
}

/* ===========================================================
 *   HIDL Callbacks : ILocHidlGeofenceServiceCallback.hal
 * ===========================================================*/
struct GnssGeofenceServiceProviderCallback :
        public ILocHidlGeofenceServiceCallback {

    Return<void> gnssGeofenceBreachCb(
            int32_t count, const hidl_vec<int32_t>& idList,
            const LocHidlLocation& hidlLocation, int32_t breachType,
            uint64_t timestamp) override;
    Return<void> gnssGeofenceStatusCb(
            int32_t statusAvailable, int32_t locTechType) override;
    Return<void> gnssAddGeofencesCb(
            int32_t count, const hidl_vec<int32_t>& locationErrorList,
            const hidl_vec<int32_t>& idList) override;
    Return<void> gnssRemoveGeofencesCb(
            int32_t count, const hidl_vec<int32_t>& locationErrorList,
            const hidl_vec<int32_t>& idList) override;
    Return<void> gnssPauseGeofencesCb(
            int32_t count, const hidl_vec<int32_t>& locationErrorList,
            const hidl_vec<int32_t>& idList) override;
    Return<void> gnssResumeGeofencesCb(
            int32_t count, const hidl_vec<int32_t>& locationErrorList,
            const hidl_vec<int32_t>& idList) override;
};

Return<void> GnssGeofenceServiceProviderCallback::gnssGeofenceBreachCb(
        int32_t count, const hidl_vec<int32_t>& idList,
        const LocHidlLocation& hidlLocation, int32_t breachType,
        uint64_t timestamp) {
    LOC_LOGD("%s]: (%zu)", __func__, count);
    Location location;
    translateLocation(location, hidlLocation);
    lock_guard guard(sGeofenceLock);
    if (sGeofenceCbs) {
        for (size_t i = 0; i < count; i++) {
            sGeofenceCbs->geofenceBreachCb(idList[i], location,
                    static_cast<GeofenceBreachType>(breachType), timestamp);
        }
    }
    return Void();
}

Return<void> GnssGeofenceServiceProviderCallback::gnssGeofenceStatusCb(
        int32_t statusAvailable, int32_t /*locTechType*/) {
    LOC_LOGD("%s]: (%d)", __func__, statusAvailable);
    lock_guard guard(sGeofenceLock);
    if (sGeofenceCbs) {
        sGeofenceCbs->geofenceStatusCb(static_cast<GeofenceStatusAvailable>(statusAvailable));
    }
    return Void();
}

Return<void> GnssGeofenceServiceProviderCallback::gnssAddGeofencesCb(
        int32_t count, const hidl_vec<int32_t>& locationErrorList,
        const hidl_vec<int32_t>& idList) {
    LOC_LOGD("%s]: (%d)", __func__, count);
    lock_guard guard(sGeofenceLock);
    for (size_t i = 0; i < count; i++) {
        if (sGeofenceCbs) {
            sGeofenceCbs->addGeofencesCb(idList[i],
                    static_cast<LocationError>(locationErrorList[i]));
        }
    }
    return Void();
}

Return<void> GnssGeofenceServiceProviderCallback::gnssRemoveGeofencesCb(
        int32_t count, const hidl_vec<int32_t>& locationErrorList,
        const hidl_vec<int32_t>& idList) {
    LOC_LOGD("%s]: (%d)", __func__, count);
    lock_guard guard(sGeofenceLock);
    for (size_t i = 0; i < count; i++) {
        if (sGeofenceCbs) {
            sGeofenceCbs->removeGeofencesCb(idList[i],
                    static_cast<LocationError>(locationErrorList[i]));
        }
    }
    return Void();
}

Return<void> GnssGeofenceServiceProviderCallback::gnssPauseGeofencesCb(
        int32_t count, const hidl_vec<int32_t>& locationErrorList,
        const hidl_vec<int32_t>& idList) {
    LOC_LOGD("%s]: (%d)", __func__, count);
    lock_guard guard(sGeofenceLock);
    for (size_t i = 0; i < count; i++) {
        if (sGeofenceCbs) {
            sGeofenceCbs->pauseGeofencesCb(idList[i],
                    static_cast<LocationError>(locationErrorList[i]));
        }
    }
    return Void();
}

Return<void> GnssGeofenceServiceProviderCallback::gnssResumeGeofencesCb(
        int32_t count, const hidl_vec<int32_t>& locationErrorList,
        const hidl_vec<int32_t>& idList) {
    LOC_LOGD("%s]: (%d)", __func__, count);
    lock_guard guard(sGeofenceLock);
    for (size_t i = 0; i < count; i++) {
        if (sGeofenceCbs) {
            sGeofenceCbs->resumeGeofencesCb(idList[i],
                    static_cast<LocationError>(locationErrorList[i]));
        }
    }
    return Void();
}

/* ===========================================================
 *   HIDL Callbacks : ILocHidlIzatSubscriptionCallback.hal
 * ===========================================================*/
struct IzatSubscriptionCallback : public ILocHidlIzatSubscriptionCallback {
    Return<void> requestData(const hidl_vec<LocHidlSubscriptionDataItemId>& list) override;
    Return<void> updateSubscribe(const hidl_vec<LocHidlSubscriptionDataItemId>& list,
                                 bool subscribe) override;
    Return<void> unsubscribeAll() override;
    Return<void> turnOnModule(LocHidlSubscriptionDataItemId di, int32_t timeout) override;
    Return<void> turnOffModule(LocHidlSubscriptionDataItemId di) override;
};

Return<void> IzatSubscriptionCallback::requestData(
        const hidl_vec<LocHidlSubscriptionDataItemId>& list) {
    std::vector<int> dataItemArray;

    for (int i = 0; i < list.size(); i++){
        dataItemArray.push_back((int)list[i]);
    }

    lock_guard guard(sDataItemLock);
    if (sDataItemCbs) {
        sDataItemCbs->requestDataCb(dataItemArray);
    }
    return Void();
}

Return<void> IzatSubscriptionCallback::updateSubscribe(
        const hidl_vec<LocHidlSubscriptionDataItemId>& list, bool subscribe) {
    std::vector<int> dataItemArray;

    for (int i = 0; i < list.size(); i++){
        dataItemArray.push_back((int)list[i]);
    }

    lock_guard guard(sDataItemLock);
    if (sDataItemCbs) {
        sDataItemCbs->updateSubscribeCb(dataItemArray, subscribe);
    }
    return Void();
}

Return<void> IzatSubscriptionCallback::unsubscribeAll() {
    lock_guard guard(sDataItemLock);
    if (sDataItemCbs) {
        sDataItemCbs->unsbscribeAllCb();
    }
    return Void();
}

Return<void> IzatSubscriptionCallback::turnOnModule(
        LocHidlSubscriptionDataItemId di, int32_t timeout) {
    lock_guard guard(sDataItemLock);
    if (sDataItemCbs) {
        sDataItemCbs->turnOnCb((int32_t)di, timeout);
    }
    return Void();
}

Return<void> IzatSubscriptionCallback::turnOffModule(LocHidlSubscriptionDataItemId di) {
    lock_guard guard(sDataItemLock);
    if (sDataItemCbs) {
        sDataItemCbs->turnOffCb((int32_t)di);
    }
    return Void();
}

/* ===========================================================
 *   HIDL Callbacks : ILocHidlIzatProviderCallback.hal
 * ===========================================================*/
struct IzatProviderCallback : public ILocHidlIzatProviderCallback {
    Return<void> onLocationChanged(const LocHidlIzatLocation& izatlocation) override;
    Return<void> onStatusChanged(LocHidlIzatProviderStatus status) override;
};

Return<void> IzatProviderCallback::onLocationChanged(const LocHidlIzatLocation& izatlocation) {
    Location location;
    memset(&location, 0, sizeof(Location));
    translateIzatLocation(location, izatlocation);
    lock_guard guard(sIzatProviderLock);
    if (sIzatProviderCbs) {
        sIzatProviderCbs->izatProviderLocationCb(location);
    }
    return Void();
}

Return<void> IzatProviderCallback::onStatusChanged(LocHidlIzatProviderStatus status) {
    lock_guard guard(sIzatProviderLock);
    if (sIzatProviderCbs) {
        sIzatProviderCbs->onStatusChanged((uint8_t)status);
    }
    return Void();
}

/* ===========================================================
 *   HIDL Callbacks : ILocHidlOsNpGlueCallback.hal
 * ===========================================================*/
struct OsNpGlueCallback : public ILocHidlIzatOsNpGlueCallback {
    Return<void> onSetRequest(const LocHidlIzatRequest& request) override;
    Return<void> onStopRequest() override;
};

Return<void> OsNpGlueCallback::onSetRequest(const LocHidlIzatRequest& request) {
    int providerType = (int)request.provider;
    int32_t numFixes = request.numUpdates;
    int32_t tbf = request.timeIntervalBetweenFixes;
    float displacement = request.smallestDistanceBetweenFixes;
    int accuracy = (int)request.suggestedHorizontalAccuracy;

    lock_guard guard(sOsNpLock);
    if (sOsNpCbs) {
        sOsNpCbs->onSetRequest(providerType, numFixes, tbf, displacement, accuracy);
    }
    return Void();
}

Return<void> OsNpGlueCallback::onStopRequest() {
    lock_guard guard(sOsNpLock);
    if (sOsNpCbs) {
        sOsNpCbs->onStopRequest();
    }
    return Void();
}

/* ===========================================================
 *   HIDL Callbacks : ILocHidlOsNpGlueCallback.hal
 * ===========================================================*/
struct IzatConfigCallback : public ILocHidlIzatConfigCallback {
    Return<void> izatConfigCallback(const hidl_string& izatConfigContent) override;
};

Return<void> IzatConfigCallback::izatConfigCallback(const hidl_string& izatConfigContent) {
    lock_guard guard(sIzatConfigLock);
    if (sIzatConfigCbs) {
        sIzatConfigCbs->izatConfigcallback(izatConfigContent.c_str());
    }
    return Void();
}

/* ===========================================================
 *   HIDL Callbacks : ILocHidlGnssConfigServiceCallback.hal
 * ===========================================================*/
struct GnssConfigServiceCallback : public ILocHidlGnssConfigServiceCallback {
    Return<void> getGnssSvTypeConfigCb(
            const hidl_vec<GnssConstellationType>& disabledSvTypeList) override;
};

Return<void> GnssConfigServiceCallback::getGnssSvTypeConfigCb(
        const hidl_vec<GnssConstellationType>& disabledSvTypeList) {
    lock_guard guard(sGnssConfigLock);
    if (sGnssConfigCbs) {
        int cnt = disabledSvTypeList.size();
        std::vector<uint8_t> svList;
        for (int i = 0; i < cnt; i++) {
            svList.push_back((uint8_t)disabledSvTypeList[i]);
        }
        sGnssConfigCbs->getGnssSvTypeConfigCb(svList);
    }
    return Void();
}

/* =================================================
 *    HIDL Callbacks : ILocHidlAGnssCallback.hal
 * =================================================*/
struct AgnssExtCallback : public ILocHidlAGnssCallback {
    Return<void> agnssStatusIpV6Cb(
            const ILocHidlAGnssCallback::AGnssStatusIpV6& agps_status) override;
    Return<void> agnssStatusIpV4Cb(
            const ILocHidlAGnssCallback::AGnssStatusIpV4& agps_status) override;
    Return<void> locHidlAgnssStatusIpV6Cb(
            const ILocHidlAGnssCallback::LocHidlAGnssStatusIpV6& agps_status) override;
    Return<void> locHidlAgnssStatusIpV4Cb(
            const ILocHidlAGnssCallback::LocHidlAGnssStatusIpV4& agps_status) override;
};

Return<void> AgnssExtCallback::agnssStatusIpV6Cb(
        const ILocHidlAGnssCallback::AGnssStatusIpV6& agps_status) {
    LOC_LOGD("%s]: Not supported", __func__);
    return Void();
}

Return<void> AgnssExtCallback::agnssStatusIpV4Cb(
        const ILocHidlAGnssCallback::AGnssStatusIpV4& agps_status) {
    lock_guard guard(sAgnssExtLock);
    if (sAgnssExtCbs) {
        sAgnssExtCbs->agnssStatusIpV4Cb((int)agps_status.type,
                (uint32_t)ILocHidlAGnssCallback::LocHidlApnTypeMask::APN_TYPE_MASK_SUPL,
                (int)agps_status.status);
    }
    return Void();
}

Return<void> AgnssExtCallback::locHidlAgnssStatusIpV6Cb(
        const ILocHidlAGnssCallback::LocHidlAGnssStatusIpV6& agps_status) {
    LOC_LOGD("%s]: Not supported", __func__);
    return Void();
}

Return<void> AgnssExtCallback::locHidlAgnssStatusIpV4Cb(
        const ILocHidlAGnssCallback::LocHidlAGnssStatusIpV4& agps_status) {
    lock_guard guard(sAgnssExtLock);
    if (sAgnssExtCbs) {
        sAgnssExtCbs->agnssStatusIpV4Cb((int)agps_status.type,
                                        (uint32_t)agps_status.apnTypeMask,
                                        (int)agps_status.status);
    }
    return Void();
}

/* ===========================================================
 *   HIDL Callbacks : ILocHidlWiFiDBReceiverCallback.hal
 * ===========================================================*/

struct WiFiDBReceiverCallback : public ILocHidlWiFiDBReceiverCallback {
    Return<void> apListUpdateCallback(
        const hidl_vec<::vendor::qti::gnss::V1_0::ILocHidlWiFiDBReceiverCallback::LocHidlApInfo>&
        apInfoList, int32_t apListSize) override;
    Return<void> apListUpdateCallback_2_1(
        const hidl_vec<ILocHidlWiFiDBReceiverCallback::LocHidlApInfoExt>& apInfoList,
        int32_t apListSize, vendor::qti::gnss::V2_1::LocHidlWifiDBListStatus apListStatus,
        const LocHidlUlpLocation& ulpLocation, bool ulpLocValid) override;
    Return<void> statusUpdateCallback(
        bool status, const hidl_string& reason) override;
    Return<void> serviceRequestCallback() override;
    Return<void> attachVmOnCallback() override;
};

Return<void> WiFiDBReceiverCallback::apListUpdateCallback(
        const hidl_vec<::vendor::qti::gnss::V1_0::ILocHidlWiFiDBReceiverCallback::LocHidlApInfo>&
        apInfoList, int32_t apListSize) {
    return Void();
}

Return<void> WiFiDBReceiverCallback::apListUpdateCallback_2_1(
    const hidl_vec<ILocHidlWiFiDBReceiverCallback::LocHidlApInfoExt>& apInfoList,
    int32_t apListSize, vendor::qti::gnss::V2_1::LocHidlWifiDBListStatus apListStatus,
    const LocHidlUlpLocation& ulpLocation, bool ulpLocValid) {
    lock_guard guard(sWiFiDBReceiverLock);
    if (sWifiDBReceiverCbs) {
        std::vector<izat_remote_api::WiFiDBUpdater::APInfo> apList;
        uint8_t status = (uint8_t) apListStatus;
        Location location;

        for (int i = 0; i < apListSize; i++) {
            izat_remote_api::WiFiDBUpdater::APInfo apInfo;
            apInfo.mac_R48b = apInfoList[i].base_apinfo_1_0.mac_R48b;
            apInfo.cellType = apInfoList[i].base_apinfo_1_0.cell_type;
            apInfo.cellRegionID1 = apInfoList[i].base_apinfo_1_0.cell_id1;
            apInfo.cellRegionID2 = apInfoList[i].base_apinfo_1_0.cell_id2;
            apInfo.cellRegionID3 = apInfoList[i].base_apinfo_1_0.cell_id3;
            apInfo.cellRegionID4 = apInfoList[i].base_apinfo_1_0.cell_id4;
            memcpy(apInfo.ssid, apInfoList[i].base_apinfo_1_0.ssid.c_str(),
                    sizeof(apInfo.ssid));
            apInfo.ssid_valid_byte_count = apInfoList[i].base_apinfo_1_0.ssid_valid_byte_count;
            apInfo.utcTime = apInfoList[i].utc_time;
            apInfo.fdal_status = apInfoList[i].fdal_status;
            apList.push_back(apInfo);
        }
        if (ulpLocValid) {
            translateHidlUlpLocation(location, ulpLocation);
        }
        sWifiDBReceiverCbs->apListUpdateCallback(apList, apList.size(), location, ulpLocValid);
    }
    return Void();
}

Return<void> WiFiDBReceiverCallback::statusUpdateCallback(
        bool status, const hidl_string& reason) {
    lock_guard guard(sWiFiDBReceiverLock);
    if (sWifiDBReceiverCbs) {
        sWifiDBReceiverCbs->wifiDBReceiverStatusUpdateCallback(status, reason.c_str());
    }
    return Void();
}

Return<void> WiFiDBReceiverCallback::serviceRequestCallback() {
    lock_guard guard(sWiFiDBReceiverLock);
    if (sWifiDBReceiverCbs) {
        sWifiDBReceiverCbs->wifiDBReceiverServiceRequestCallback();
    }
    return Void();
}

Return<void> WiFiDBReceiverCallback::attachVmOnCallback() {
    lock_guard guard(sWiFiDBReceiverLock);
    if (sWifiDBReceiverCbs) {
        sWifiDBReceiverCbs->wifiDBReceiverAttachVmOnCallback();
    }
    return Void();
}

/* ===========================================================
 *   HIDL Callbacks : ILocHidlWWANDBReceiverCallback.hal
 * ===========================================================*/
struct WWANDBReceiverCallback : public ILocHidlWWANDBReceiverCallback {

    Return<void> bsListUpdateCallback(
            const hidl_vec<
            ::vendor::qti::gnss::V1_2::ILocHidlWWANDBReceiverCallback::LocHidlBsInfo>& bsInfoList,
            int32_t bsListSize) override;
    Return<void> bsListUpdateCallback_2_1(
            const hidl_vec<ILocHidlWWANDBReceiverCallback::LocHidlBsInfoExt>& bsInfoList,
            int32_t bsListSize, uint8_t status, const LocHidlUlpLocation& ulpLocation) override;
    Return<void> statusUpdateCallback(
            bool status, const hidl_string& reason) override;
    Return<void> serviceRequestCallback() override;
    Return<void> attachVmOnCallback() override;
};

Return<void> WWANDBReceiverCallback::bsListUpdateCallback(
        const hidl_vec<::vendor::qti::gnss::V1_2::ILocHidlWWANDBReceiverCallback::LocHidlBsInfo>&
        bsInfoList, int32_t bsListSize) {
    return Void();
}

Return<void> WWANDBReceiverCallback::bsListUpdateCallback_2_1(
        const hidl_vec<ILocHidlWWANDBReceiverCallback::LocHidlBsInfoExt>& bsInfoList,
        int32_t bsListSize, uint8_t status, const LocHidlUlpLocation& ulpLocation) {
    lock_guard guard(sWwanDBReceiverLock);
    if (sWwanDBReceiverCbs) {
        std::vector<BSInfo_s> bsList;
        Location location;

        for (int i = 0; i < bsListSize; i++) {
            BSInfo_s bsInfo;
            bsInfo.cell_type = bsInfoList[i].base_bsinfo_1_2.cell_type;
            bsInfo.cell_id1 = bsInfoList[i].base_bsinfo_1_2.cell_id1;
            bsInfo.cell_id2 = bsInfoList[i].base_bsinfo_1_2.cell_id2;
            bsInfo.cell_id3 = bsInfoList[i].base_bsinfo_1_2.cell_id3;
            bsInfo.cell_id4 = bsInfoList[i].base_bsinfo_1_2.cell_id4;
            bsInfo.cell_timestamp = bsInfoList[i].timestamp;
            bsList.push_back(bsInfo);
        }
        translateHidlUlpLocation(location, ulpLocation);

        sWwanDBReceiverCbs->bsListUpdateCallback(bsList, status, location);
    }
    return Void();
}

Return<void> WWANDBReceiverCallback::statusUpdateCallback(
        bool status, const hidl_string& reason) {
    lock_guard guard(sWwanDBReceiverLock);
    if (sWwanDBReceiverCbs) {
        sWwanDBReceiverCbs->wwanDBReceiverStatusUpdateCallback(status, reason.c_str());
    }
    return Void();
}

Return<void> WWANDBReceiverCallback::serviceRequestCallback() {
    lock_guard guard(sWwanDBReceiverLock);
    if (sWwanDBReceiverCbs) {
        sWwanDBReceiverCbs->wwanDBReceiverServiceRequestCallback();
    }
    return Void();
}

Return<void> WWANDBReceiverCallback::attachVmOnCallback() {
    lock_guard guard(sWwanDBReceiverLock);
    if (sWwanDBReceiverCbs) {
        sWwanDBReceiverCbs->wwanDBReceiverAttachVmOnCallback();
    }
    return Void();
}


/* ===========================================================
 *   HIDL Callbacks : ILocHidlWiFiDBProviderCallback.hal
 * ===========================================================*/

struct WiFiDBProviderCallback : public ILocHidlWiFiDBProviderCallback {
    Return<void> apObsLocDataUpdateCallback(
            const hidl_vec<LocHidlApObsData>& apObsLocDataList,
            int32_t apObsLocDataListSize, LocHidlWifiDBListStatus apListStatus) override;
    Return<void> serviceRequestCallback() override;
    Return<void> attachVmOnCallback() override;
};

Return<void> WiFiDBProviderCallback::apObsLocDataUpdateCallback(
        const hidl_vec<LocHidlApObsData>& apObsLocDataList,
        int32_t apObsLocDataListSize, LocHidlWifiDBListStatus apListStatus) {
    lock_guard guard(sWiFiDBProviderLock);
    if (sWifiDBProviderCbs) {
        std::vector<izat_remote_api::WiFiDBProvider::APObsData> apObsList;
        uint8_t status = (uint8_t) apListStatus;
        for (int i = 0; i < apObsLocDataListSize; i++) {
            izat_remote_api::WiFiDBProvider::APObsData apObsData;
            apObsData.scanTimestamp_ms = apObsLocDataList[i].scanTimestamp_ms;
            apObsData.bUlpLocValid = apObsLocDataList[i].bUlpLocValid;
            apObsData.cellInfo.cellType = apObsLocDataList[i].cellInfo.cell_type;
            apObsData.cellInfo.cellRegionID1 = apObsLocDataList[i].cellInfo.cell_id1;
            apObsData.cellInfo.cellRegionID2 = apObsLocDataList[i].cellInfo.cell_id2;
            apObsData.cellInfo.cellRegionID3 = apObsLocDataList[i].cellInfo.cell_id3;
            apObsData.cellInfo.cellRegionID4 = apObsLocDataList[i].cellInfo.cell_id4;
            translateHidlUlpLocationToUlpLocation(apObsData.gpsLoc,
                    apObsLocDataList[i].gpsLoc);
            for (int j = 0; j < apObsLocDataList[i].ap_scan_info_size; j++) {
                izat_remote_api::WiFiDBProvider::APScanInfo apScanInfo;
                apScanInfo.mac_R48b = apObsLocDataList[i].ap_scan_info[j].mac_R48b;
                apScanInfo.rssi = apObsLocDataList[i].ap_scan_info[j].rssi;
                apScanInfo.age_usec = apObsLocDataList[i].ap_scan_info[j].age_usec;
                apScanInfo.channel_id = apObsLocDataList[i].ap_scan_info[j].channel_id;
                memcpy(apScanInfo.ssid, apObsLocDataList[i].ap_scan_info[j].ssid.c_str(),
                        sizeof(apScanInfo.ssid));
                apScanInfo.ssid_valid_byte_count =
                        apObsLocDataList[i].ap_scan_info[j].ssid_valid_byte_count;
                apObsData.ap_scan_info.push_back(apScanInfo);
            }
            apObsList.push_back(apObsData);
        }
        sWifiDBProviderCbs->apObsLocDataUpdateCallback(apObsList, status);
    }
    return Void();
}

Return<void> WiFiDBProviderCallback::serviceRequestCallback() {
    lock_guard guard(sWiFiDBProviderLock);
    if (sWifiDBProviderCbs) {
        sWifiDBProviderCbs->wifiDBProviderServiceRequestCallback();
    }
    return Void();
}

Return<void> WiFiDBProviderCallback::attachVmOnCallback() {
    lock_guard guard(sWiFiDBProviderLock);
    if (sWifiDBProviderCbs) {
        sWifiDBProviderCbs->wifiDBProviderAttachVmOnCallback();
    }
    return Void();
}

/* ===========================================================
 *   HIDL Callbacks : ILocHidlWWANDBProviderCallback.hal
 * ===========================================================*/

struct WWANDBProviderCallback : public ILocHidlWWANDBProviderCallback {
    Return<void> bsObsLocDataUpdateCallback(
            const hidl_vec<LocHidlBSObsData>& bsObsLocDataList,
            int32_t bsObsLocDataListSize, int8_t bsListStatus) override;
    Return<void> serviceRequestCallback() override;
    Return<void> attachVmOnCallback() override;
};

Return<void> WWANDBProviderCallback::bsObsLocDataUpdateCallback(
        const hidl_vec<LocHidlBSObsData>& bsObsLocDataList,
        int32_t bsObsLocDataListSize, int8_t bsListStatus) {
    lock_guard guard(sWwanDBProviderLock);
    if (sWwanDBProviderCbs) {
        std::vector<BSObsData_s> bsObsList;

        for (int i = 0; i < bsObsLocDataListSize; i++) {
            BSObsData_s bsObsData;
            bsObsData.scanTimestamp_ms = bsObsLocDataList[i].scanTimestamp_ms;
            bsObsData.bUlpLocValid = bsObsLocDataList[i].bUlpLocValid;
            if (bsObsData.bUlpLocValid) {
                translateHidlUlpLocationToUlpLocation(bsObsData.gpsLoc, bsObsLocDataList[i].gpsLoc);
            }
            bsObsData.cellInfo.cell_type = bsObsLocDataList[i].cellInfo.cell_type;
            bsObsData.cellInfo.cell_id1 = bsObsLocDataList[i].cellInfo.cell_id1;
            bsObsData.cellInfo.cell_id2 = bsObsLocDataList[i].cellInfo.cell_id2;
            bsObsData.cellInfo.cell_id3 = bsObsLocDataList[i].cellInfo.cell_id3;
            bsObsData.cellInfo.cell_id4 = bsObsLocDataList[i].cellInfo.cell_id4;
            bsObsList.push_back(bsObsData);
        }
        sWwanDBProviderCbs->bsObsLocDataUpdateCallback(bsObsList, bsListStatus);
    }
    return Void();
}

Return<void> WWANDBProviderCallback::serviceRequestCallback() {
    lock_guard guard(sWwanDBProviderLock);
    if (sWwanDBProviderCbs) {
        sWwanDBProviderCbs->wwanDBProviderServiceRequestCallback();
    }
    return Void();
}

Return<void> WWANDBProviderCallback::attachVmOnCallback() {
    lock_guard guard(sWwanDBProviderLock);
    if (sWwanDBProviderCbs) {
        sWwanDBProviderCbs->wwanDBProviderAttachVmOnCallback();
    }
    return Void();
}

#endif
/* ===================================================================
 *   HIDL Interface
 * ===================================================================*/

sp<IGnssBatching>& getBatchingIface() {
    if (gnssBatchingIface == nullptr) {
        if (gnssHal != nullptr) {
            gnssBatchingIface = gnssHal->getExtensionGnssBatching_2_0();
        }
        if (gnssBatchingIface == nullptr) {
            ALOGE("Unable to get a handle to IGnssBatching extension!");
        }
    }
    return gnssBatchingIface;
}

sp<IGnssGeofencing>& getGeofenceIface() {
    if (gnssGeofencingIface == nullptr) {
        if (gnssHal != nullptr) {
            gnssGeofencingIface = gnssHal->getExtensionGnssGeofencing();
        }
        if (gnssGeofencingIface == nullptr) {
            ALOGE("Unable to get a handle to IGnssGeofencing extension!");
        }
    }
    return gnssGeofencingIface;
}

sp<IAGnss> Gnss_2_0_getExtensionAGnss_2_0() {
    if (agnssIface == nullptr) {
        if (gnssHal != nullptr) {
            auto agnssHal = gnssHal->getExtensionAGnss_2_0();
            if (!agnssHal.isOk()) {
                ALOGE("Unable to get a handle to IAGnss extension!");
            } else {
                agnssIface = agnssHal;
            }
        }
    }
    return agnssIface;
}

sp<IAGnssRil> Gnss_2_0_getExtensionAGnssRil_2_0() {
    if (agnssRilIface == nullptr) {
        if (gnssHal != nullptr) {
            auto agnssRilHal = gnssHal->getExtensionAGnssRil_2_0();
            if (!agnssRilHal.isOk()) {
                ALOGE("Unable to get a handle to IAGnssRil extension!");
            } else {
                agnssRilIface = agnssRilHal;
            }
        }
    }
    return agnssRilIface;
}

sp<IGnssConfiguration> Gnss_2_0_getExtensionGnssConfiguration_2_0() {
    if (gnssConfigIface == nullptr) {
        if (gnssHal != nullptr) {
            auto gnssConfigHal = gnssHal->getExtensionGnssConfiguration_2_0();
            if (!gnssConfigHal.isOk()) {
                ALOGE("Unable to get a handle to IGnssConfiguration extension!");
            } else {
                gnssConfigIface = gnssConfigHal;
            }
        }
    }
    return gnssConfigIface;
}

sp<IGnssDebug> Gnss_2_0_getExtensionGnssDebug_2_0() {
    if (gnssDebugIface == nullptr) {
        if (gnssHal != nullptr) {
            auto gnssDebugHal = gnssHal->getExtensionGnssDebug_2_0();
            if (!gnssDebugHal.isOk()) {
                ALOGE("Unable to get a handle to IGnssDebug extension!");
            } else {
                gnssDebugIface = gnssDebugHal;
            }
        }
    }
    return gnssDebugIface;
}

sp<IGnssMeasurement> Gnss_2_0_getExtensionGnssMeasurement_2_0() {
    if (gnssMeasurementIface == nullptr) {
        if (gnssHal != nullptr) {
            auto gnssMeasurementHal = gnssHal->getExtensionGnssMeasurement_2_0();
            if (!gnssMeasurementHal.isOk()) {
                ALOGE("Unable to get a handle to IGnssMeasurement extension!");
            } else {
                gnssMeasurementIface = gnssMeasurementHal;
            }
        }
    }
    return gnssMeasurementIface;
}

sp<IMeasurementCorrections> Gnss_2_0_getExtensionMeasurementCorrections() {
    if (measurementCorrIface == nullptr) {
        if (gnssHal != nullptr) {
            auto measurementCorrHal = gnssHal->getExtensionMeasurementCorrections();
            if (!measurementCorrHal.isOk()) {
                ALOGE("Unable to get a handle to IAGnss extension!");
            } else {
                measurementCorrIface = measurementCorrHal;
            }
        }
    }
    return measurementCorrIface;
}

sp<IGnssVisibilityControl> Gnss_2_0_getExtensionVisibilityControl() {
    if (visibCtrlIface == nullptr) {
        if (gnssHal != nullptr) {
            auto visibCtrlHal = gnssHal->getExtensionVisibilityControl();
            if (!visibCtrlHal.isOk()) {
                ALOGE("Unable to get a handle to IGnssVisibilityControl extension!");
            } else {
                visibCtrlIface = visibCtrlHal;
            }
        }
    }
    return visibCtrlIface;
}

#ifdef LOCHIDL
sp<ILocHidlFlpService>& getLocHidlFlpIface() {
    if (locHidlFlpIface == nullptr) {
        if (gnssHidlHal != nullptr) {
            locHidlFlpIface = gnssHidlHal->getExtensionLocHidlFlpService_2_1();
        }
        if (locHidlFlpIface == nullptr) {
            ALOGE("Unable to get a handle to ILocHidlFlpService extension!");
        }
    }
    return locHidlFlpIface;
}

sp<ILocHidlGeofenceService>& getLocHidlGeofenceIface() {
    if (locHidlGeofenceIface == nullptr) {
        if (gnssHidlHal != nullptr) {
            locHidlGeofenceIface = gnssHidlHal->getExtensionLocHidlGeofenceService();
        }
        if (locHidlGeofenceIface == nullptr) {
            ALOGE("Unable to get a handle to ILocHidlGeofenceService extension!");
        }
    }
    return locHidlGeofenceIface;
}

sp<ILocHidlIzatSubscription>& getLocHidlIzatSubscriptionIface() {
    if (gnssSubscriptionIface == nullptr) {
        if (gnssHidlHal != nullptr) {
            gnssSubscriptionIface = gnssHidlHal->getExtensionLocHidlIzatSubscription_1_1();
        }
        if (gnssSubscriptionIface == nullptr) {
            ALOGE("Unable to get a handle to ILocHidlIzatSubscription extension!");
        }
    }
    return gnssSubscriptionIface;

}

sp<ILocHidlRilInfoMonitor>& getLocHidlRilInfoMonitorIface() {
    if (gnssRilInfoIface == nullptr) {
        if (gnssHidlHal != nullptr) {
            gnssRilInfoIface = gnssHidlHal->getExtensionLocHidlRilInfoMonitor_1_1();
        }
        if (gnssRilInfoIface == nullptr) {
            ALOGE("Unable to get a handle to ILocHidlIzatSubscription extension!");
        }
    }
    return gnssRilInfoIface;
}

sp<ILocHidlIzatProvider>& getLocHidlIzatProviderIface() {
    if (izatProviderIface == nullptr) {
        if (gnssHidlHal != nullptr) {
            izatProviderIface = gnssHidlHal->getExtensionLocHidlIzatNetworkProvider_1_1();
        }
        if (izatProviderIface == nullptr) {
            ALOGE("Unable to get a handle to ILocHidlIzatProvider extension!");
        }
    }
    return izatProviderIface;
}

sp<ILocHidlIzatOsNpGlue>& getLocHidlOsNpGlueIface() {
    if (izatOsNpIface == nullptr) {
        if (gnssHidlHal != nullptr) {
            izatOsNpIface = gnssHidlHal->getExtensionLocHidlIzatOsNpGlue_1_1();
        }
        if (izatOsNpIface == nullptr) {
            ALOGE("Unable to get a handle to ILocHidlIzatOsNpGlue extension!");
        }
    }
    return izatOsNpIface;
}

sp<ILocHidlXT>& getLocHidlXTIface() {
    if (izatXTIface == nullptr) {
        if (gnssHidlHal != nullptr) {
            izatXTIface = gnssHidlHal->getExtensionLocHidlXT_1_1();
        }
        if (izatXTIface == nullptr) {
            ALOGE("Unable to get a handle to ILocHidlIzatXT extension!");
        }
    }
    return izatXTIface;
}

sp<ILocHidlIzatConfig>& getLocHidlIzatConfigIface() {
    if (izatConfigIface == nullptr) {
        if (gnssHidlHal != nullptr) {
            izatConfigIface = gnssHidlHal->getExtensionLocHidlIzatConfig();
        }
        if (izatConfigIface == nullptr) {
            ALOGE("Unable to get a handle to ILocHidlIzatConfig extension!");
        }
    }
    return izatConfigIface;
}

sp<ILocHidlGnssConfigService>& getLocHidlGnssConfigIface() {
    if (lochidlgnssConfigIface == nullptr) {
        if (gnssHidlHal != nullptr) {
            lochidlgnssConfigIface = gnssHidlHal->getExtensionLocHidlGnssConfigService_2_1();
        }
        if (lochidlgnssConfigIface == nullptr) {
            ALOGE("Unable to get a handle to ILocHidlGnssConfig extension!");
        }
    }
    return lochidlgnssConfigIface;
}

sp<ILocHidlAGnss>& getLocHidlAgnssIface() {
    if (agnssExtIface == nullptr) {
        if (gnssHidlHal != nullptr) {
            agnssExtIface = gnssHidlHal->getExtensionLocHidlAGnss_2_1();
        }
        if (agnssExtIface == nullptr) {
            ALOGE("Unable to get a handle to ILocHidlAGnss extension!");
        }
    }
    return agnssExtIface;
}

sp<ILocHidlDebugReportService>& getLocHidlDebugReportServiceIface() {
    if (debugReportIface == nullptr) {
        if (gnssHidlHal != nullptr) {
            debugReportIface = gnssHidlHal->getExtensionLocHidlDebugReportService_1_1();
        }
        if (debugReportIface == nullptr) {
            ALOGE("Unable to get a handle to ILocHidlDebugReportService extension!");
        }
    }
    return debugReportIface;
}

sp<ILocHidlWiFiDBReceiver>& getLocHidlWiFiDBReceiverIface() {
    if (wifiDBReceiverIface == nullptr) {
        if (gnssHidlHal != nullptr) {
            wifiDBReceiverIface = gnssHidlHal->getExtensionLocHidlWiFiDBReceiver_2_1();
        }
        if (wifiDBReceiverIface == nullptr) {
            ALOGE("Unable to get a handle to ILocHidlWiFiDBReceiver extension!");
        }
    }
    return wifiDBReceiverIface;
}

sp<ILocHidlWiFiDBProvider>& getLocHidlWiFiDBProviderIface() {
    if (wifiDBProviderIface == nullptr) {
        if (gnssHidlHal != nullptr) {
            wifiDBProviderIface = gnssHidlHal->getExtensionLocHidlWiFiDBProvider();
        }
        if (wifiDBProviderIface == nullptr) {
            ALOGE("Unable to get a handle to ILocHidlWiFiDBProvider extension!");
        }
    }
    return wifiDBProviderIface;
}

sp<ILocHidlWWANDBReceiver>& getLocHidlWwanDBReceiverIface() {
    if (wwanDBReceiverIface == nullptr) {
        if (gnssHidlHal != nullptr) {
            wwanDBReceiverIface = gnssHidlHal->getExtensionLocHidlWWANDBReceiver_2_1();
        }
        if (wwanDBReceiverIface == nullptr) {
            ALOGE("Unable to get a handle to ILocHidlWWANDBReceiver extension!");
        }
    }
    return wwanDBReceiverIface;
}

sp<ILocHidlWWANDBProvider>& getLocHidlWwanDBProviderIface() {
    if (wwanDBProviderIface == nullptr) {
        if (gnssHidlHal != nullptr) {
            wwanDBProviderIface = gnssHidlHal->getExtensionLocHidlWWANDBProvider();
        }
        if (wwanDBProviderIface == nullptr) {
            ALOGE("Unable to get a handle to ILocHidlWWANDBProvider extension!");
        }
    }
    return wwanDBProviderIface;
}
#endif

void HidlAPI::setGnssCbs(GnssCbBase* callbacks) {
    {
        lock_guard guard(sGnssLock);
        sGnssCbs = callbacks;
    }
    sp<IGnssCallback> gnssCbIface = new GnssCallback();
    /*
     * Fail if the main interface fails to initialize
     */
    if (gnssHal == nullptr) {
        ALOGE("Unable to Initialize GNSS HAL\n");
    }

    auto result = gnssHal->setCallback_2_0(gnssCbIface);
    if (!result.isOk() || !result) {
        ALOGE("SetCallback for Gnss Interface fails\n");
    }

    auto gnssNi = gnssHal->getExtensionGnssNi();
    if (!gnssNi.isOk()) {
        ALOGD("Unable to get a handle to GnssNi");
    } else {
        gnssNiIface = gnssNi;
    }
    sp<IGnssNiCallback> gnssNiCbIface = new GnssNiCallback();
    if (gnssNiIface != nullptr) {
        gnssNiIface->setCallback(gnssNiCbIface);
    } else {
        ALOGE("Unable to initialize GNSS NI interface");
    }
}

void HidlAPI::setFlpCbs(GnssCbBase* callbacks) {
    {
        lock_guard guard(sFlpLock);
        sFlpCbs = callbacks;
    }
    gardenPrint("entry of setFlpCbs.");
#ifdef LOCHIDL
    auto flpHidlIface = getLocHidlFlpIface();
    if (flpHidlIface == nullptr) {
        ALOGE("ILocHidlFlpService Iface NULL!");
        return;
    }
    sp<GnssFlpServiceCallback> gnssFlpServiceCb =
        new GnssFlpServiceCallback();
    sp<ILocHidlFlpService> flpVendorIface = flpHidlIface;
    auto init = flpVendorIface->init(gnssFlpServiceCb);
#else
    sp<IGnssBatchingCallback> gnssBatchingCbIface = new GnssBatchingCallback();
    auto iface = getBatchingIface();
    if (iface == nullptr) {
        ALOGE("IGnssBatching Iface NULL!");
        return;
    }
    iface->init_2_0(gnssBatchingCbIface);
#endif
}

void HidlAPI::setGeofenceCbs(GnssCbBase* callbacks) {
    {
        lock_guard guard(sGeofenceLock);
        sGeofenceCbs = callbacks;
    }
    gardenPrint("entry of setGeofenceCbs.");
#ifdef LOCHIDL
    auto geofenceVendor = getLocHidlGeofenceIface();
    sp<GnssGeofenceServiceProviderCallback> gnssGeofenceServiceProviderCb =
        new GnssGeofenceServiceProviderCallback();
    sp<ILocHidlGeofenceService> geofenceVendorIface = geofenceVendor;
    auto init = geofenceVendorIface->init(gnssGeofenceServiceProviderCb);
#else
    sp<IGnssGeofenceCallback> gnssGeofencingCbIface = new GnssGeofenceCallback();
    auto iface = getGeofenceIface();
    if (iface == nullptr) {
        ALOGE("IGnssGeofencing Iface NULL!");
        return;
    }
    iface->setCallback(gnssGeofencingCbIface);
#endif
}

#ifdef LOCHIDL
void HidlAPI::setIzatSubscriptionCbs(GnssCbBase* callbacks) {
    {
        lock_guard guard(sGeofenceLock);
        sDataItemCbs = callbacks;
    }
    gardenPrint("entry of setIzatSubscriptionCbs.");
    auto iface = getLocHidlIzatSubscriptionIface();
    if (iface == nullptr) {
        ALOGE("gnssSubscriptionIface Iface NULL!");
        return;
    }
    sp<IzatSubscriptionCallback> subscriptionCb = new IzatSubscriptionCallback();
    iface->init(subscriptionCb);
}

void HidlAPI::setIzatProviderCbs(GnssCbBase* callbacks) {
    {
        lock_guard guard(sIzatProviderLock);
        sIzatProviderCbs = callbacks;
    }
    gardenPrint("entry of setIzatProviderCbs.");
    auto iface = getLocHidlIzatProviderIface();
    if (iface == nullptr) {
        ALOGE("gnssIzatProviderIface Iface NULL!");
        return;
    }
    sp<IzatProviderCallback> izatProviderCb = new IzatProviderCallback();
    iface->init(izatProviderCb);
}

void HidlAPI::setOsNpCbs(GnssCbBase* callbacks) {
    {
        lock_guard guard(sOsNpLock);
        sOsNpCbs = callbacks;
    }
    gardenPrint("entry of setOsNpGlueCbs.");
    auto iface = getLocHidlOsNpGlueIface();
    if (iface == nullptr) {
        ALOGE("gnssOsNpIface Iface NULL!");
        return;
    }
    sp<OsNpGlueCallback> osNpCb = new OsNpGlueCallback();
    iface->setCallback(osNpCb);
}

void HidlAPI::setIzatConfigCbs(GnssCbBase* callbacks) {
    {
        lock_guard guard(sIzatConfigLock);
        sIzatConfigCbs = callbacks;
    }
    gardenPrint("entry of setIzatConfigCbs.");
    auto iface = getLocHidlIzatConfigIface();
    if (iface == nullptr) {
        ALOGE("gnssIzatConfig Iface NULL!");
        return;
    }
    sp<IzatConfigCallback> izatConfigCb = new IzatConfigCallback();
    iface->init(izatConfigCb);
}

void HidlAPI::setGnssConfigCbs(GnssCbBase* callbacks) {
    {
        lock_guard guard(sGnssConfigLock);
        sGnssConfigCbs = callbacks;
    }
    gardenPrint("entry of setGnssConfigCbs.");
    auto iface = getLocHidlGnssConfigIface();
    if (iface == nullptr) {
        ALOGE("gnssGnssConfig Iface NULL!");
        return;
    }
    sp<GnssConfigServiceCallback> gnssConfigCb = new GnssConfigServiceCallback();
    iface->init(gnssConfigCb);
}

void HidlAPI::setAGnssExtCbs(GnssCbBase* callbacks) {
    {
        lock_guard guard(sAgnssExtLock);
        sAgnssExtCbs = callbacks;
    }
    gardenPrint("entry of setAgnssExtCbs.");
    auto iface = getLocHidlAgnssIface();
    if (iface == nullptr) {
        ALOGE("gnssAgnss Iface NULL!");
        return;
    }
    sp<AgnssExtCallback> agnssExtCb = new AgnssExtCallback();
    iface->setCallbackExt(agnssExtCb);
}

void HidlAPI::setWiFiDBReceiverCbs(GnssCbBase* callbacks) {
    {
        lock_guard guard(sWiFiDBReceiverLock);
        sWifiDBReceiverCbs = callbacks;
    }
    gardenPrint("entry of setWiFiDBReceiverCbs.");
    auto iface = getLocHidlWiFiDBReceiverIface();
    if (iface == nullptr) {
        ALOGE("wifidb receiver Iface NULL!");
        return;
    }
    sp<WiFiDBReceiverCallback> wifiDBReceiverCb = new WiFiDBReceiverCallback();
    iface->init(wifiDBReceiverCb);
    iface->registerWiFiDBUpdater(wifiDBReceiverCb);
}

void HidlAPI::setWWanDBReceiverCbs(GnssCbBase* callbacks) {
    {
        lock_guard guard(sWwanDBReceiverLock);
        sWwanDBReceiverCbs = callbacks;
    }
    gardenPrint("entry of setWwanDBReceiverCbs.");
    auto iface = getLocHidlWwanDBReceiverIface();
    if (iface == nullptr) {
        ALOGE("wwandb receiver Iface NULL!");
        return;
    }
    sp<WWANDBReceiverCallback> wwanDBReceiverCb = new WWANDBReceiverCallback();
    iface->init(wwanDBReceiverCb);
    iface->registerWWANDBUpdater_2_1(wwanDBReceiverCb);
}

void HidlAPI::setWiFiDBProviderCbs(GnssCbBase* callbacks) {
    {
        lock_guard guard(sWiFiDBProviderLock);
        sWifiDBProviderCbs = callbacks;
    }
    gardenPrint("entry of setWiFiDBProviderCbs.");
    auto iface = getLocHidlWiFiDBProviderIface();
    if (iface == nullptr) {
        ALOGE("wifidb provider Iface NULL!");
        return;
    }
    sp<WiFiDBProviderCallback> wifiDBProviderCb = new WiFiDBProviderCallback();
    iface->init(wifiDBProviderCb);
    iface->registerWiFiDBProvider(wifiDBProviderCb);
}

void HidlAPI::setWWanDBProviderCbs(GnssCbBase* callbacks) {
    {
        lock_guard guard(sWwanDBProviderLock);
        sWwanDBProviderCbs = callbacks;
    }
    gardenPrint("entry of setWWanDBProviderCbs.");
    auto iface = getLocHidlWwanDBProviderIface();
    if (iface == nullptr) {
        ALOGE("wwandb provider Iface NULL!");
        return;
    }
    sp<WWANDBProviderCallback> wwanDBProviderCb = new WWANDBProviderCallback();
    iface->init(wwanDBProviderCb);
    iface->registerWWANDBProvider(wwanDBProviderCb);
}

#endif

HidlAPI::HidlAPI() {
    gnssHal = IGnss::getService();
#ifdef LOCHIDL
    gnssHidlHal = ILocHidlGnss::getService(GNSS_VENDOR_SERVICE_INSTANCE);
    if (gnssHidlHal != nullptr) {
        gardenPrint("get Lochidl api success.");
        gnssLocHalDeathRecipient = new GnssDeathRecipient();
        android::hardware::Return<bool> loclinked =
                gnssHidlHal->linkToDeath(gnssLocHalDeathRecipient, 0);
        if (!loclinked.isOk()) {
            ALOGE("Transaction error in linking to GnssHAL death: %s",
                    loclinked.description().c_str());
        } else if (!loclinked) {
            ALOGW("Unable to link to GnssHidlHal death notifications");
        } else {
            ALOGD("Link to death notification successful");
        }
    } else {
        gardenPrint("get Lochidl api failed.");
    }
#else
    if (gnssHal != nullptr) {
        gardenPrint("get hidl api success.");
        gnssHalDeathRecipient = new GnssDeathRecipient();
        android::hardware::Return<bool> linked = gnssHal->linkToDeath(gnssHalDeathRecipient, 0);
        if (!linked.isOk()) {
            ALOGE("Transaction error in linking to GnssHAL death: %s",
                    linked.description().c_str());
        } else if (!linked) {
            ALOGW("Unable to link to GnssHal death notifications");
        } else {
            ALOGD("Link to death notification successful");
        }
    } else {
        gardenPrint("get hidl api failed.");
    }
#endif
}

int HidlAPI::gnssStart() {
    if (gnssHal != nullptr) {
        gnssHal->start();
    }
    return 0;
}

int HidlAPI::gnssStop() {
    if (gnssHal != nullptr) {
        gnssHal->stop();
    }
    return 0;
}

void HidlAPI::gnssCleanup() {
    if (gnssHal != nullptr) {
        gnssHal->cleanup();
    }
}

int HidlAPI::gnssInjectLocation(double latitude, double longitude, float accuracy) {
    if (gnssHal != nullptr) {
        gnssHal->injectLocation(latitude, longitude, accuracy);
    }
    return 0;
}

void HidlAPI::gnssDeleteAidingData(LocGpsAidingData flags) {
    if (gnssHal != nullptr) {
        gnssHal->cleanup();
    }
    if (gnssHal != nullptr) {
        gnssHal->deleteAidingData(static_cast<IGnss::GnssAidingData>(flags));
    }
}

bool HidlAPI::Gnss_1_0_injectTime(int64_t timeMs, int64_t timeReferenceMs, int32_t uncertaintyMs) {
    if (gnssHal != nullptr) {
        auto result = gnssHal->injectTime(timeMs, timeReferenceMs, uncertaintyMs);
        if (!result.isOk() || false == result) {
            return false;
        }
    }
    return true;
}

int HidlAPI::gnssSetPositionMode(LocGpsPositionMode mode,
        LocGpsPositionRecurrence recurrence, uint32_t min_interval,
        uint32_t preferred_accuracy, uint32_t preferred_time, bool lowPowerMode) {
    if (gnssHal != nullptr) {
        gnssHal->setPositionMode_1_1(static_cast<IGnss::GnssPositionMode>(mode),
                static_cast<IGnss::GnssPositionRecurrence>(recurrence),
                min_interval, preferred_accuracy, preferred_time, lowPowerMode);
    }
    return 0;
}

void HidlAPI::configurationUpdate(const char* config_data, int32_t length) {
    LOC_LOGD("%s]: (%s %d)", __func__, config_data, length);
    const int n = 11;
    uint8_t flags[n];
    memset(&flags, 0, sizeof(uint8_t) * n);
    GnssConfig data;
    memset(&data, 0, sizeof(GnssConfig));
    data.size = sizeof(GnssConfig);

    const loc_param_s_type gnssConfTable[] =
    {
        {"GPS_LOCK",                            &data.gpsLock,                      flags+0,  'n'},
        {"SUPL_VER",                            &data.suplVersion,                  flags+1,  'n'},
        {"LPP_PROFILE",                         &data.lppProfile,                   flags+3,  'n'},
        {"LPPE_CP_TECHNOLOGY",                  &data.lppeControlPlaneMask,         flags+4,  'n'},
        {"LPPE_UP_TECHNOLOGY",                  &data.lppeUserPlaneMask,            flags+5,  'n'},
        {"A_GLONASS_POS_PROTOCOL_SELECT",       &data.aGlonassPositionProtocolMask, flags+6,  'n'},
        {"USE_EMERGENCY_PDN_FOR_EMERGENCY_SUPL",&data.emergencyPdnForEmergencySupl, flags+7,  'n'},
        {"SUPL_ES",                             &data.suplEmergencyServices,        flags+8,  'n'},
        {"SUPL_MODE",                           &data.suplModeMask,                 flags+9,  'n'},
        {"EMERGENCY_EXTENSION_SECONDS",         &data.emergencyExtensionSeconds,    flags+10, 'n'},
    };
    UTIL_UPDATE_CONF(config_data, length, gnssConfTable);

    for (int i = 0; i < n; i++) {
        if (flags[i] != 0)
            data.flags |= (0x1 << i);
    }

    if (gnssHal != nullptr) {
        auto gnssConfig = gnssHal->getExtensionGnssConfiguration_2_0();
        if (!gnssConfig.isOk()) {
            ALOGE("Unable to get a handle to IGnssConfiguration extension!");
        } else {
            sp<IGnssConfiguration> gnssConf = gnssConfig;
            gnssConf->setSuplEs(data.suplEmergencyServices);
            gnssConf->setSuplVersion(data.suplVersion);
            gnssConf->setSuplMode(data.suplModeMask);
            gnssConf->setGpsLock(data.gpsLock);
            gnssConf->setLppProfile(data.lppProfile);
            gnssConf->setGlonassPositioningProtocol(data.aGlonassPositionProtocolMask);
            gnssConf->setEmergencySuplPdn(data.emergencyPdnForEmergencySupl);
            gnssConf->setEsExtensionSec(data.emergencyExtensionSeconds);
        }
    }
}

void HidlAPI::updateNetworkAvailability(int /*available*/, const char* /*apn*/) {
}

void HidlAPI::gnssNiResponse(uint32_t id, GnssNiResponse response) {
    if (gnssHal != nullptr) {
        auto gnssNi = gnssHal->getExtensionGnssNi();
        if (!gnssNi.isOk()) {
            ALOGE("Unable to get a handle to IGnssNi extension!");
        } else {
            sp<IGnssNi> gnssNiIface = gnssNi;
            gnssNiIface->respond(id, static_cast<IGnssNiCallback::GnssUserResponseType>(response));
        }
    }
}

int HidlAPI::gnssInjectBestLocation(Location location) {
    if (gnssHal != nullptr) {
        GnssLocation_v2_0 gnssLocation;
        memset(&gnssLocation, 0, sizeof(gnssLocation));
        translateToHidlLocation(gnssLocation.v1_0,location);
        gnssHal->injectBestLocation_2_0(gnssLocation);
    }
    return 0;
}

int HidlAPI::flpStartSession(uint32_t mode, LocationOptions& options, uint32_t id) {
#ifdef LOCHIDL
    auto iface = getLocHidlFlpIface();
    if (iface  == nullptr) {
        ALOGE("ILocHidlFlpService Iface NULL!");
        return 1;
    }
    uint32_t flags = 0;
    if (SESSION_MODE_ON_FULL == mode) {
        flags = static_cast<uint32_t>(IGnssBatching::Flag::WAKEUP_ON_FIFO_FULL);
    }
    iface->startFlpSession_2_1(id, flags, options.minInterval*1000000, options.minDistance, 0,
            GNSS_POWER_MODE_M2, 0);
#else
    IGnssBatching::Options batchingOptions;
    memset(&batchingOptions, 0, sizeof(batchingOptions));
    batchingOptions.periodNanos = options.minInterval*1000000;
    if (SESSION_MODE_ON_FULL == mode)
        batchingOptions.flags = static_cast<uint8_t>(IGnssBatching::Flag::WAKEUP_ON_FIFO_FULL);
    auto iface = getBatchingIface();
    if (iface == nullptr) {
        ALOGE("IGnssBatching Iface NULL!");
        return 1;
    }
    iface->start(batchingOptions);
#endif
    return 0;
}

int HidlAPI::flpStopSession(uint32_t id) {
#ifdef LOCHIDL
    auto iface = getLocHidlFlpIface();
    if (iface  == nullptr) {
        ALOGE("ILocHidlFlpService Iface NULL!");
        return 1;
    }
    iface->stopFlpSession(id);
#else
    auto iface = getBatchingIface();
    if (iface == nullptr) {
        ALOGE("IGnssBatching Iface NULL!");
        return 1;
    }
    iface->stop();
#endif
    return 0;
}

int HidlAPI::flpGetBatchSize() {
#ifdef LOCHIDL
    auto iface = getLocHidlFlpIface();
    ALOGE("%s] Not supported !", __FUNCTION__);
    return 0;
#else
    auto iface = getBatchingIface();
    if (iface == nullptr) {
        ALOGE("IGnssBatching Iface NULL!");
        return 1;
    }
    return iface->getBatchSize();
#endif
}

int HidlAPI::flpUpdateSession(uint32_t sessionMode, LocationOptions& options,
        uint32_t id) {
#ifdef LOCHIDL
    auto iface = getLocHidlFlpIface();
    if (iface  == nullptr) {
        ALOGE("ILocHidlFlpService Iface NULL!");
        return 1;
    }
    uint32_t flags = 0;
    if (SESSION_MODE_ON_FULL == sessionMode)
        flags = static_cast<uint32_t>(IGnssBatching::Flag::WAKEUP_ON_FIFO_FULL);
    iface->updateFlpSession_2_1(id, flags, options.minInterval*1000000, options.minDistance, 0,
            GNSS_POWER_MODE_M2, 0);
#endif
    return 0;
}

int HidlAPI::flpGetBatchedLocation(int /*lastNLocations*/, uint32_t id) {
#ifdef LOCHIDL
    auto iface = getLocHidlFlpIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlFlpService Iface NULL!");
        return 1;
    }
    iface->getAllBatchedLocations(id);
#endif
    return 0;
}

void HidlAPI::flpCleanUp() {
#ifdef LOCHIDL
    auto iface = getLocHidlFlpIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlFlpService Iface NULL!");
        return;
    }
    iface->init(nullptr);
#else
    auto iface = getBatchingIface();
    if (iface == nullptr) {
        ALOGE("IGnssBatching Iface NULL!");
        return;
    }
    iface->cleanup();
#endif
}

void HidlAPI::addGeofence(uint32_t geofenceId, double latitudeDegrees, double longitudeDegrees,
        double radiusMeters, GeofenceBreachTypeMask monitorTransitions,
        uint32_t notificationResponsivenessMs,
        uint32_t dwellTimeSeconds) {
#ifdef LOCHIDL
    auto iface = getLocHidlGeofenceIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlGeofenceService Iface NULL!");
        return;
    }
    iface->addGeofence(geofenceId, latitudeDegrees, longitudeDegrees, radiusMeters,
           GEOFENCE_BREACH_ENTERED|GEOFENCE_BREACH_EXITED|GEOFENCE_BREACH_UNCERTAIN,
           notificationResponsivenessMs, 0/*confidence not used*/, 1,
           0/*dwell time mask not used*/);
#else
    auto iface = getGeofenceIface();
    if (iface == nullptr) {
        ALOGE("IGnssGeofencing Iface NULL!");
        return;
    }
    iface->addGeofence(geofenceId, latitudeDegrees, longitudeDegrees, radiusMeters,
           static_cast<IGnssGeofenceCallback::GeofenceTransition>(0x100),
           GEOFENCE_BREACH_ENTERED|GEOFENCE_BREACH_EXITED|GEOFENCE_BREACH_UNCERTAIN,
           notificationResponsivenessMs, 100);
#endif
}
void HidlAPI::removeGeofence(uint32_t geofenceId) {
#ifdef LOCHIDL
    auto iface = getLocHidlGeofenceIface();
#else
    auto iface = getGeofenceIface();
#endif
    if (iface == nullptr) {
        ALOGE("IGnssGeofencing Iface NULL!");
        return;
    }
    iface->removeGeofence(geofenceId);
}
void HidlAPI::modifyGeofence(uint32_t geofenceId, GeofenceBreachTypeMask monitorTransitions,
        uint32_t notificationResponsivenessMs, uint32_t dwellTimeSeconds) {
#ifdef LOCHIDL
    auto iface = getLocHidlGeofenceIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlGeofenceService Iface NULL!");
        return;
    }
    iface->updateGeofence(geofenceId, monitorTransitions, notificationResponsivenessMs);
#endif
}
void HidlAPI::pauseGeofence(uint32_t geofenceId) {
#ifdef LOCHIDL
    auto iface = getLocHidlGeofenceIface();
#else
    auto iface = getGeofenceIface();
#endif
    if (iface == nullptr) {
        ALOGE("IGnssGeofencing Iface NULL!");
        return;
    }
    iface->pauseGeofence(geofenceId);
}
void HidlAPI::resumeGeofence(uint32_t geofenceId, GeofenceBreachTypeMask monitorTransitions) {
#ifdef LOCHIDL
    auto iface = getLocHidlGeofenceIface();
#else
    auto iface = getGeofenceIface();
#endif
    if (iface == nullptr) {
        ALOGE("IGnssGeofencing Iface NULL!");
        return;
    }
    iface->resumeGeofence(geofenceId, monitorTransitions);
}
void HidlAPI::removeAllGeofences() {
}

/* ===========================================================
 *   HIDL Callbacks : IAGnssCallback.hal
 * ===========================================================*/
struct AGnssCallback: public IAGnssCallback {
    Return<void> agnssStatusCb(IAGnssCallback::AGnssType type,
            IAGnssCallback::AGnssStatusValue status);
};
Return<void> AGnssCallback::agnssStatusCb(IAGnssCallback::AGnssType type,
        IAGnssCallback::AGnssStatusValue status) {
    lock_guard guard(sAGnssLock);
    if (sAGnssCbs) {
        sAGnssCbs->agnssStatusCb(static_cast<uint8_t>(type), static_cast<uint8_t>(status));
    }
    return Void();

}

/* ===========================================================
 *   HIDL Callbacks : IAGnssRilCallback.hal
 * ===========================================================*/
struct AGnssRilCallback: public IAGnssRilCallback {
    Return<void> requestSetIdCb(hidl_bitfield<IAGnssRilCallback::ID> setIdflag);
    Return<void> requestRefLocCb();

};

Return<void> AGnssRilCallback::requestSetIdCb(hidl_bitfield<IAGnssRilCallback::ID> setIdflag) {
    lock_guard guard(sAGnssRilLock);
    if (sAGnssRilCbs) {
        sAGnssRilCbs->requestSetIdCb(setIdflag);
    }
    return Void();
}

Return<void> AGnssRilCallback::requestRefLocCb() {
    lock_guard guard(sAGnssRilLock);
    if (sAGnssRilCbs) {
        sAGnssRilCbs->requestRefLocCb();
    }
    return Void();
}

/* ===========================================================
 *   HIDL Callbacks : IGnssMeasurementCallback.hal
 * ===========================================================*/
struct GnssMeasurementCallback: public IGnssMeasurementCallback {
    Return<void> gnssMeasurementCb_2_0(const IGnssMeasurementCallback::GnssData& data);
    Return<void> GnssMeasurementCb(
            const ::android::hardware::gnss::V1_0::IGnssMeasurementCallback::GnssData& data) {
        return Void();}
    Return<void> gnssMeasurementCb(
            const ::android::hardware::gnss::V1_1::IGnssMeasurementCallback::GnssData& data) {
        return Void();}
};
Return<void> GnssMeasurementCallback::gnssMeasurementCb_2_0(
        const IGnssMeasurementCallback::GnssData& data) {
    lock_guard guard(sGnssMeasurementLock);
    GnssMeasurementsNotification measNotif;
    translateGnssData(measNotif, data);
    if (sGnssMeasurementCbs) {
        sGnssMeasurementCbs->gnssMeasurementCb(measNotif);
    }
    return Void();
}

/* ===========================================================
 *   HIDL Callbacks : IGnssVisibilityControlCallback.hal
 * ===========================================================*/
struct GnssVisibilityControlCallback: public IGnssVisibilityControlCallback {
    Return<void> nfwNotifyCb(const IGnssVisibilityControlCallback::NfwNotification& notification);
    Return<bool> isInEmergencySession();
};
Return<void> GnssVisibilityControlCallback::nfwNotifyCb(
        const IGnssVisibilityControlCallback::NfwNotification& notification) {
    lock_guard guard(sVisibCtrlLock);
    GnssNfwNotification gnssNfwNotif;
    translateGnssNfwNotification(gnssNfwNotif, notification);
    if (sVisibCtrlCbs) {
        sVisibCtrlCbs->nfwNotifyCb(gnssNfwNotif);
    }
    return Void();

}
Return<bool> GnssVisibilityControlCallback::isInEmergencySession() {
    lock_guard guard(sVisibCtrlLock);
    if (sVisibCtrlCbs) {
        sVisibCtrlCbs->isInEmergencySession();
    }
    return true;
}

//=============================Set Gnss HAL Callback Utils=========================================
//=================================================================================================
void HidlAPI::setAGnssCbs(GnssCbBase* callbacks) {
    {
        lock_guard guard(sAGnssLock);
        sAGnssCbs = callbacks;
    }
    auto agnssIface = Gnss_2_0_getExtensionAGnss_2_0();
    sp<IAGnssCallback> agnssCbIface = new AGnssCallback();
    if (agnssIface != nullptr) {
        agnssIface->setCallback(agnssCbIface);
    } else {
        ALOGE("Unable to initialize AGNSS interface");
    }
}

void HidlAPI::setAGnssRilCbs(GnssCbBase* callbacks) {
    {
        lock_guard guard(sAGnssRilLock);
        sAGnssRilCbs = callbacks;
    }
    auto agnssRilIface = Gnss_2_0_getExtensionAGnssRil_2_0();
    sp<IAGnssRilCallback> agnssRilCbIface = new AGnssRilCallback();
    if (agnssRilIface != nullptr) {
        agnssRilIface->setCallback(agnssRilCbIface);
    } else {
        ALOGE("Unable to initialize AGnssRil interface");
    }
}

void HidlAPI::setGnssMeasurementCbs(GnssCbBase* callbacks) {
    {
        lock_guard guard(sGnssMeasurementLock);
        sGnssMeasurementCbs = callbacks;
    }
    auto gnssMeasurementIface = Gnss_2_0_getExtensionGnssMeasurement_2_0();
    sp<IGnssMeasurementCallback> gnssMeasurementCbIface = new GnssMeasurementCallback();
    if (gnssMeasurementIface != nullptr) {
        gnssMeasurementIface->setCallback_2_0(gnssMeasurementCbIface, true);
    } else {
        ALOGE("Unable to initialize GnssMeasurement interface");
    }
}

void HidlAPI::setGnssVisibilityControlCbs(GnssCbBase* callbacks) {
    {
        lock_guard guard(sVisibCtrlLock);
        sVisibCtrlCbs = callbacks;
    }
    auto visibCtrl = Gnss_2_0_getExtensionVisibilityControl();
    sp<IGnssVisibilityControlCallback> visibCtrlCbIface = new GnssVisibilityControlCallback();
    if (visibCtrl != nullptr) {
        visibCtrl->setCallback(visibCtrlCbIface);
    } else {
        ALOGE("Unable to initialize GnssXtra interface");
    }
}

//=============================================Gnss HAL APIs=======================================
//=================================================================================================
    //IAGnss
bool HidlAPI::AGnss_2_0_dataConnClosed_2_0() {
    if (agnssIface != nullptr) {
        auto result = agnssIface->dataConnClosed();
        if (result.isOk() && true == result) {
            ALOGV("agnssIface->dataConnClosed() success.");
            return true;
        } else {
            ALOGE("agnssIface->dataConnClosed() failed.");
        }
    } else {
        ALOGE("agnssIface null");
    }
    return false;
}
bool HidlAPI::AGnss_2_0_dataConnFailed_2_0() {
    if (agnssIface != nullptr) {
        auto result = agnssIface->dataConnFailed();
        if (!result.isOk() || !result) {
            ALOGE("%s: Failed to notify unavailability of AGnss data connection", __func__);
            return false;

        }
        return true;
    } else {
        ALOGE("agnssIface null");
    }
    return false;
}

bool HidlAPI::AGnss_2_0_setServer_2_0(int32_t type, const std::string& hostname, int32_t port) {
    if (agnssIface != nullptr) {
        auto result = agnssIface->setServer((IAGnssCallback::AGnssType)type, hostname, port);

        if (result.isOk() && true == result) {
            ALOGV("agnssIface->setServer() success.");
            return true;

        } else {
            ALOGE("agnssIface->setServer() failed.");
        }
    } else {
        ALOGE("agnssIface null");
    }
    return false;
}
bool HidlAPI::AGnss_2_0_dataConnOpen_2_0(uint64_t networkHandle, const std::string& apn,
        int32_t apnIpType) {
    if (agnssIface != nullptr) {
        auto result = agnssIface->dataConnOpen(networkHandle, apn,
                (IAGnss::ApnIpType)apnIpType);

        if (result.isOk() && true == result) {
            ALOGV("agnssIface->dataConnOpen() success.");
            return true;
        } else {
            ALOGE("agnssIface->dataConnOpen() failed.");
        }
    } else {
        ALOGE("agnssIface null");
    }
    return false;
}

    //IAGnssRil
void HidlAPI::AGnssRil_1_0_setRefLocation(int type, int mcc, int mnc, int lac, int cid) {
    if (agnssRilIface != nullptr) {
        ::android::hardware::gnss::V1_0::IAGnssRil::AGnssRefLocation agnssReflocation;
        switch (static_cast<IAGnssRil_V1_0::AGnssRefLocationType>(type)) {
            case IAGnssRil_V1_0::AGnssRefLocationType::GSM_CELLID:
            case IAGnssRil_V1_0::AGnssRefLocationType::UMTS_CELLID:
                agnssReflocation.type = static_cast<IAGnssRil_V1_0::AGnssRefLocationType>(type);
                agnssReflocation.cellID.mcc = mcc;
                agnssReflocation.cellID.mnc = mnc;
                agnssReflocation.cellID.lac = lac;
                agnssReflocation.cellID.cid = cid;
                break;
            default:
                ALOGE("Neither a GSM nor a UMTS cellid (%s:%d).", __FUNCTION__, __LINE__);
                return;
        }
        auto result = agnssRilIface->setRefLocation(agnssReflocation);
        if (!result.isOk()) {
            ALOGE("agnssRilIface->setRefLocation failed.");
        } else {
            ALOGV("agnssRilIface->setRefLocation success.");
        }
    } else {
        ALOGE("agnssRilIface is null");
    }
}

bool HidlAPI::AGnssRil_1_0_setSetId(int type, const std::string& setid) {
    if (agnssRilIface != nullptr) {
        auto result = agnssRilIface->setSetId((IAGnssRil::SetIDType)type, setid);
        if (!result.isOk() || false == result) {
            ALOGE("agnssRilIface->setSetId failed.");
        } else {
            ALOGV("agnssRilIface->setSetId success.");
            return true;
        }
    } else {
        ALOGE("agnssRilIface is null");
    }
    return false;
}
bool AGnssRil_1_0_updateNetworkAvailability(bool available, const std::string& apn) {
    if (agnssRilIface != nullptr) {
        auto result = agnssRilIface->updateNetworkAvailability(available, apn);
        if (!result.isOk() || false == result) {
            ALOGE("agnssRilIface->updateNetworkAvailability failed.");
        } else {
            ALOGV("agnssRilIface->updateNetworkAvailability success.");
            return true;
        }
    } else {
        ALOGE("agnssRilIface is null");
    }
    return false;
}
bool HidlAPI::AGnssRil_2_0_updateNetworkState_2_0(uint64_t networkHandle, bool connected,
        uint16_t attributes, const std::string& apn) {
    if (agnssRilIface != nullptr) {
        IAGnssRil::NetworkAttributes attr;
        attr.capabilities = attributes;
        attr.networkHandle = networkHandle;
        attr.isConnected = connected;
        attr.apn = apn;
        auto result = agnssRilIface->updateNetworkState_2_0(attr);
        if (!result.isOk() || false == result) {
            ALOGE("agnssRilIface->updateNetworkState_2_0 failed.");
        } else {
            ALOGV("agnssRilIface->updateNetworkState_2_0 success.");
            return true;
        }
    } else {
        ALOGE("agnssRilIface is null");
    }
    return false;
}

    //IGnssConfiguration
bool HidlAPI::GnssConfiguration_1_1_setBlacklist(
        const std::vector<int>& constellations, const std::vector<int>& svIDs) {
    if (gnssConfigIface != nullptr) {
        int length = constellations.size();
        hidl_vec<IGnssConfiguration_V1_1::BlacklistedSource> blacklist;
        blacklist.resize(length);

        for (int i = 0; i < length; i++) {
            blacklist[i].constellation = static_cast<GnssConstellationType>(constellations[i]);
            blacklist[i].svid = svIDs[i];
        }
        auto result = gnssConfigIface->setBlacklist(blacklist);
        if (!result.isOk() || false == result) {
            ALOGE("gnssConfigIface->setBlacklist failed.");
        } else {
            ALOGV("gnssConfigIface->setBlacklist success.");
            return true;
        }
    } else {
        ALOGE("gnssConfigIface is null");
    }
    return false;
}

    //IGnssDebug
void  HidlAPI::GnssDebug_2_0_getDebugData_2_0(getDebugData_2_0_cb _hidl_cb) {
    IGnssDebug::DebugData data;

    auto gnssDebugIface = Gnss_2_0_getExtensionGnssDebug_2_0();
    if (gnssDebugIface != nullptr) {
        auto result = gnssDebugIface->getDebugData_2_0([&data](
                const IGnssDebug::DebugData& debugData) {
                data = debugData;
                });
        if (!result.isOk()) {
            ALOGE("gnssDebugIface->getDebugData_2_0 failed.");
        } else {
            ALOGV("gnssDebugIface->getDebugData_2_0 success.");
        }
    }
    GnssDebugReport report;

    translateGnssDebugReport(report, data);

    _hidl_cb(report);
}

    //IGnssMeasurement
void GnssMeasurement_1_0_close() {
    if (gnssMeasurementIface != nullptr) {
        auto result = gnssMeasurementIface->close();
        if (result.isOk()) {
            ALOGV("gnssMeasurementIface->close() success.");
        } else {
            ALOGE("gnssMeasurementIface->close() failed.");
        }
    } else {
        ALOGE("gnssMeasurementIface null");
    }
}

    //IGnssVisibilityControl
bool HidlAPI::GnssVisibilityControl_1_0_enableNfwLocationAccess(
        const std::vector<std::string>& proxyApps) {
    if (visibCtrlIface != nullptr) {
        int length = proxyApps.size();
        hidl_vec<android::hardware::hidl_string> hidlProxyApps(length);
        for (int i=0; i<length; ++i) {
            hidlProxyApps[i] = proxyApps[i];
        }
        auto result = visibCtrlIface->enableNfwLocationAccess(hidlProxyApps);
        if (result.isOk() && true == result) {
            ALOGV("visibCtrlIface->enableNfwLocationAccess() success.");
            return true;
        } else {
            ALOGE("visibCtrlIface->enableNfwLocationAccess() failed.");
        }
    } else {
        ALOGE("visibCtrlIface null");
    }
    return false;
}

#ifdef LOCHIDL
void HidlAPI::dataItemUpdate(IDataItemCore* dataItem) {
    if (dataItem == NULL) {
        return;
    }

    auto iface = getLocHidlIzatSubscriptionIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlIzatSubscription interface NULL!");
        return;
    }
    DataItemId id = dataItem->getId();
    switch (id) {
        case MODEL_DATA_ITEM_ID:
        {
            ILocHidlIzatSubscription::StringDataItem di;
            di.id = id;
            di.str = (static_cast<izat_manager::ModelDataItem*>(dataItem))->mModel;
            iface->stringDataItemUpdate(di);
            break;
        }
        case MANUFACTURER_DATA_ITEM_ID:
        {
            ILocHidlIzatSubscription::StringDataItem di;
            di.id = id;
            di.str = (static_cast<izat_manager::ManufacturerDataItem*>(dataItem))->mManufacturer;
            iface->stringDataItemUpdate(di);
            break;
        }
        case TAC_DATA_ITEM_ID:
        {
            ILocHidlIzatSubscription::StringDataItem di;
            di.id = id;
            di.str = (static_cast<izat_manager::TacDataItem*>(dataItem))->mValue;
            iface->stringDataItemUpdate(di);
            break;
        }
        case MCCMNC_DATA_ITEM_ID:
        {
            ILocHidlIzatSubscription::StringDataItem di;
            di.id = id;
            di.str = (static_cast<izat_manager::MccmncDataItem*>(dataItem))->mValue;
            iface->stringDataItemUpdate(di);
            break;
        }
        case NETWORKINFO_DATA_ITEM_ID:
        {
            ILocHidlIzatSubscription::NetworkInfoDataItem di;
            di.type = (static_cast<izat_manager::NetworkInfoDataItem*>(dataItem))->mType;
            di.available = (static_cast<izat_manager::NetworkInfoDataItem*>(dataItem))->mAvailable;
            di.connected = (static_cast<izat_manager::NetworkInfoDataItem*>(dataItem))->mConnected;
            di.roaming = (static_cast<izat_manager::NetworkInfoDataItem*>(dataItem))->mRoaming;
            di.typeName = (static_cast<izat_manager::NetworkInfoDataItem*>(dataItem))->mTypeName;
            di.subTypeName =
                    (static_cast<izat_manager::NetworkInfoDataItem*>(dataItem))->mSubTypeName;
            iface->networkinfoUpdate(di);
            break;
        }
        case RILSERVICEINFO_DATA_ITEM_ID:
        {
            ILocHidlIzatSubscription::RilServiceInfoDataItem di;
            di.type = (static_cast<izat_manager::RilServiceInfoDataItem*>(dataItem))
                    ->mServiceInfo->carrierAirIf_type;
            di.mcc = (static_cast<izat_manager::RilServiceInfoDataItem*>(dataItem))
                    ->mServiceInfo->carrierMcc;
            di.mnc = (static_cast<izat_manager::RilServiceInfoDataItem*>(dataItem))
                    ->mServiceInfo->carrierMnc;
            di.name = (static_cast<izat_manager::RilServiceInfoDataItem*>(dataItem))
                    ->mServiceInfo->carrierName;
            iface->serviceinfoUpdate(di);
            break;
        }
        case RILCELLINFO_DATA_ITEM_ID:
        {
            LOC_RilCellInfo *cellInfo =
                    (static_cast<izat_manager::RilCellInfoDataItem*>(dataItem))->mCellInfo;
            if (cellInfo->rtType == LOC_RIL_TECH_LTE) {
                ILocHidlIzatSubscription::CellLteDataItem di;
                di.status = cellInfo->nwStatus;
                di.type = cellInfo->rtType;
                di.mcc = cellInfo->u.lteCinfo.mcc;
                di.mnc = cellInfo->u.lteCinfo.mnc;
                di.pci = cellInfo->u.lteCinfo.pci;
                di.tac = cellInfo->u.lteCinfo.tac;
                di.cid = cellInfo->u.lteCinfo.cid;
                di.mask = cellInfo->valid_mask;
                iface->cellLteUpdate(di);
            } else if (cellInfo->rtType == LOC_RIL_TECH_WCDMA ||
                    cellInfo->rtType == LOC_RIL_TECH_GSM) {
                ILocHidlIzatSubscription::CellGwDataItem di;
                di.status = cellInfo->nwStatus;
                di.type = cellInfo->rtType;
                if (cellInfo->rtType == LOC_RILAIRIF_GSM) {
                    di.mcc = cellInfo->u.gsmCinfo.mcc;
                    di.mnc = cellInfo->u.gsmCinfo.mnc;
                    di.lac = cellInfo->u.gsmCinfo.lac;
                    di.cid = cellInfo->u.gsmCinfo.cid;
                } else if (cellInfo->rtType == LOC_RILAIRIF_WCDMA) {
                    di.mcc = cellInfo->u.wcdmaCinfo.mcc;
                    di.mnc = cellInfo->u.wcdmaCinfo.mnc;
                    di.lac = cellInfo->u.wcdmaCinfo.lac;
                    di.cid = cellInfo->u.wcdmaCinfo.cid;
                }
                di.mask = cellInfo->valid_mask;
                iface->cellGwUpdate(di);
            } else if (cellInfo->rtType == LOC_RIL_TECH_CDMA){
                ILocHidlIzatSubscription::CellCdmaDataItem di;
                di.status = cellInfo->nwStatus;
                di.type = cellInfo->rtType;
                di.sid = cellInfo->u.cdmaCinfo.sid;
                di.nid = cellInfo->u.cdmaCinfo.nid;
                di.bsid = cellInfo->u.cdmaCinfo.bsid;
                di.bslat = cellInfo->u.cdmaCinfo.bslat;
                di.bslong = cellInfo->u.cdmaCinfo.bslon;
                di.timeOffset = cellInfo->u.cdmaCinfo.local_timezone_offset_from_utc;
                di.inDST = cellInfo->u.cdmaCinfo.local_timezone_on_daylight_savings;
                di.mask = cellInfo->valid_mask;
                iface->cellCdmaUpdate(di);
            } else if (cellInfo->nwStatus == LOC_NW_OOO){
                ILocHidlIzatSubscription::CellOooDataItem di;
                di.status = cellInfo->nwStatus;
                di.mask = cellInfo->valid_mask;
                iface->cellOooUpdate(di);
            }
            break;
        }
        case SERVICESTATUS_DATA_ITEM_ID:
        {
            ILocHidlIzatSubscription::ServiceStateDataItem di;
            di.status =
                    (static_cast<izat_manager::ServiceStatusDataItem*>(dataItem))->mServiceState;
            iface->serviceStateUpdate(di);
            break;
        }
        case SCREEN_STATE_DATA_ITEM_ID:
        {
            ILocHidlIzatSubscription::ScreenStatusDataItem di;
            di.status = (static_cast<izat_manager::ScreenStateDataItem*>(dataItem))->mState;
            iface->screenStatusUpdate(di);
            break;
        }
        case POWER_CONNECTED_STATE_DATA_ITEM_ID:
        {
            ILocHidlIzatSubscription::PowerConnectStatusDataItem di;
            di.status = (static_cast<izat_manager::PowerConnectStateDataItem*>(dataItem))->mState;
            iface->powerConnectStatusUpdate(di);
            break;
        }
        case TIMEZONE_CHANGE_DATA_ITEM_ID:
        {
            ILocHidlIzatSubscription::TimeZoneChangeDataItem di;
            di.curTimeMillis =
                    (static_cast<izat_manager::TimeZoneChangeDataItem*>(dataItem))->mCurrTimeMillis;
            di.rawOffset =
                    (static_cast<izat_manager::TimeZoneChangeDataItem*>(dataItem))->mRawOffsetTZ;
            di.dstOffset =
                    (static_cast<izat_manager::TimeZoneChangeDataItem*>(dataItem))->mDstOffsetTZ;
            iface->timezoneChangeUpdate(di);
            break;
        }
        case TIME_CHANGE_DATA_ITEM_ID:
        {
            ILocHidlIzatSubscription::TimeChangeDataItem di;
            di.curTimeMillis =
                    (static_cast<izat_manager::TimeChangeDataItem*>(dataItem))->mCurrTimeMillis;
            di.rawOffset =
                    (static_cast<izat_manager::TimeChangeDataItem*>(dataItem))->mRawOffsetTZ;
            di.dstOffset =
                    (static_cast<izat_manager::TimeChangeDataItem*>(dataItem))->mDstOffsetTZ;
            iface->timeChangeUpdate(di);
            break;
        }
        case SHUTDOWN_STATE_DATA_ITEM_ID:
        {
            iface->shutdownUpdate();
            break;
        }
        case WIFI_SUPPLICANT_STATUS_DATA_ITEM_ID:
        {
            ILocHidlIzatSubscription::WifiSupplicantStatusDataItem di;
            izat_manager::WifiSupplicantStatusDataItem* wifiDi =
                static_cast<izat_manager::WifiSupplicantStatusDataItem*>(dataItem);
            di.state = wifiDi->mState;
            di.apMacAddressValid = wifiDi->mApMacAddressValid;
            di.apSsidValid = wifiDi->mWifiApSsidValid;
            di.apSsid = wifiDi->mWifiApSsid;
            std::vector<uint8_t> vec;
            for (int i = 0; i < MAC_ADDRESS_LENGTH; i++) {
                vec.push_back(wifiDi->mApMacAddress[i]);
            }
            di.apMacAddress = vec;
            iface->wifiSupplicantStatusUpdate(di);
            break;
        }
        case BTLE_SCAN_DATA_ITEM_ID:
        {
            ILocHidlIzatSubscription::BtLeDeviceScanDetailsDataItem di;
            izat_manager::BtLeDeviceScanDetailsDataItem* btDi =
                static_cast<izat_manager::BtLeDeviceScanDetailsDataItem*>(dataItem);
            di.validSrnData = btDi->mValidSrnData;
            di.apSrnRssi = btDi->mApSrnRssi;
            di.apSrnTimestamp = btDi->mApSrnTimestamp;
            di.requestTimestamp = btDi->mRequestTimestamp;
            di.receiveTimestamp = btDi->mReceiveTimestamp;
            di.errorCause = btDi->mErrorCause;
            std::vector<uint8_t> vec;
            for (int i = 0; i < SRN_MAC_ADDRESS_LENGTH; i++) {
                vec.push_back(btDi->mApSrnMacAddress[i]);
            }
            di.apSrnMacAddress = vec;
            iface->btleScanDataInject(di);
            break;
        }
        case BT_SCAN_DATA_ITEM_ID:
        {
            ILocHidlIzatSubscription::BtDeviceScanDetailsDataItem di;
            izat_manager::BtDeviceScanDetailsDataItem* btDi =
                static_cast<izat_manager::BtDeviceScanDetailsDataItem*>(dataItem);
            di.validSrnData = btDi->mValidSrnData;
            di.apSrnRssi = btDi->mApSrnRssi;
            di.apSrnTimestamp = btDi->mApSrnTimestamp;
            di.requestTimestamp = btDi->mRequestTimestamp;
            di.receiveTimestamp = btDi->mReceiveTimestamp;
            di.errorCause = btDi->mErrorCause;
            std::vector<uint8_t> vec;
            for (int i = 0; i < SRN_MAC_ADDRESS_LENGTH; i++) {
                vec.push_back(btDi->mApSrnMacAddress[i]);
            }
            di.apSrnMacAddress = vec;
            iface->btClassicScanDataInject(di);
            break;
        }
        case BATTERY_LEVEL_DATA_ITEM_ID:
        {
            ILocHidlIzatSubscription::BatteryLevelDataItem di;
            di.batteryPct =
                    (static_cast<izat_manager::BatteryLevelDataItem*>(dataItem))->mBatteryPct;
            iface->batteryLevelUpdate(di);
        }
        default: break;
    }
}

void HidlAPI::dataItemsUpdate(std::list<IDataItemCore *> & dlist) {
    std::vector<ILocHidlIzatSubscription::BoolDataItem> dataItemArray;
    auto iface = getLocHidlIzatSubscriptionIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlIzatSubscription interface NULL!");
        return;
    }
    for (auto it = dlist.begin(); it != dlist.end(); it++) {
        ILocHidlIzatSubscription::BoolDataItem di;
        IDataItemCore* dataItem = *it;
        DataItemId id = dataItem->getId();
        di.id = id;
        di.enabled = false;
        switch (id) {
            case ENH_DATA_ITEM_ID:
            {
                di.enabled = (static_cast<izat_manager::ENHDataItem*>(dataItem))->mEnabled;
                break;
            }
            case AIRPLANEMODE_DATA_ITEM_ID:
            {
                di.enabled = (static_cast<izat_manager::AirplaneModeDataItem*>(dataItem))->mMode;
                break;
            }
            case GPSSTATE_DATA_ITEM_ID:
            {
                di.enabled = (static_cast<izat_manager::GPSStateDataItem*>(dataItem))->mEnabled;
                break;
            }
            case NLPSTATUS_DATA_ITEM_ID:
            {
                di.enabled = (static_cast<izat_manager::NLPStatusDataItem*>(dataItem))->mEnabled;
                break;
            }
            case WIFIHARDWARESTATE_DATA_ITEM_ID:
            {
                di.enabled =
                        (static_cast<izat_manager::WifiHardwareStateDataItem*>(dataItem))->mEnabled;
                break;
            }
            case ASSISTED_GPS_DATA_ITEM_ID:
            {
                di.enabled = (static_cast<izat_manager::AssistedGpsDataItem*>(dataItem))->mEnabled;
                break;
            }
            default: break;
        }
        dataItemArray.push_back(di);
    }
    if (dataItemArray.size() != 0) {
        iface->boolDataItemUpdate(dataItemArray);
    }
}

void HidlAPI::izatSubscriptionCleanUp() {
    auto iface = getLocHidlIzatSubscriptionIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlIzatSubscription interface NULL!");
        return;
    }
    iface->deinit();
}

void HidlAPI::rilInfoMonitorInit() {
    auto iface = getLocHidlRilInfoMonitorIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlRilInfoMonitor interface NULL!");
        return;
    }
    iface->init();
}

void HidlAPI::cinfoInject(int32_t cid, int32_t lac, int32_t mnc, int32_t mcc, bool roaming) {
    auto iface = getLocHidlRilInfoMonitorIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlRilInfoMonitor interface NULL!");
        return;
    }
    iface->cinfoInject(cid, lac, mnc, mcc, roaming);
}

void HidlAPI::oosInform() {
    auto iface = getLocHidlRilInfoMonitorIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlRilInfoMonitor interface NULL!");
        return;
    }
    iface->oosInform();
}

void HidlAPI::niSuplInit(std::string& str) {
    auto iface = getLocHidlRilInfoMonitorIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlRilInfoMonitor interface NULL!");
        return;
    }
    iface->niSuplInit(str.c_str());
}

void HidlAPI::chargerStatusInject(int32_t status) {
    auto iface = getLocHidlRilInfoMonitorIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlRilInfoMonitor interface NULL!");
        return;
    }
    iface->chargerStatusInject(status);
}

bool HidlAPI::onEnable() {
    auto iface = getLocHidlIzatProviderIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlIzatProvider interface NULL!");
        return false;
    }
    iface->onEnable();
    return true;
}

bool HidlAPI::onDisable() {
    auto iface = getLocHidlIzatProviderIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlIzatProvider interface NULL!");
        return false;
    }
    iface->onDisable();
    return true;
}

bool HidlAPI::onAddRequest(int providerType,
        int32_t numFixes, int32_t tbf, float displacement, int accuracy) {
    auto iface = getLocHidlIzatProviderIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlIzatProvider interface NULL!");
        return false;
    }
    LocHidlIzatRequest request;
    request.provider = LocHidlIzatStreamType::NETWORK;
    request.numUpdates = numFixes;
    request.timeIntervalBetweenFixes = tbf;
    request.smallestDistanceBetweenFixes = displacement;
    request.suggestedHorizontalAccuracy =
            static_cast<LocHidlIzatHorizontalAccuracy>(accuracy);
    iface->onAddRequest(request);
    return true;
}
bool HidlAPI::onRemoveRequest(int providerType,
        int32_t numFixes, int32_t tbf, float displacement, int accuracy) {
    auto iface = getLocHidlIzatProviderIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlIzatProvider interface NULL!");
        return false;
    }
    LocHidlIzatRequest request;
    request.provider = LocHidlIzatStreamType::NETWORK;
    request.numUpdates = numFixes;
    request.timeIntervalBetweenFixes = tbf;
    request.smallestDistanceBetweenFixes = displacement;
    request.suggestedHorizontalAccuracy =
            static_cast<LocHidlIzatHorizontalAccuracy>(accuracy);
    iface->onRemoveRequest(request);

    return true;
}

void HidlAPI::OsNpLocationChanged(Location& location) {
    LocHidlIzatLocation izatLocation;
    memset(&izatLocation, 0, sizeof(LocHidlIzatLocation));
    translateToIzatLocation(izatLocation, location);

    auto iface = getLocHidlOsNpGlueIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlOsNpGlue interface NULL!");
        return;
    }
    iface->locationChanged(izatLocation);
}

void HidlAPI::xtInit() {
    auto iface = getLocHidlXTIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlXT interface NULL!");
        return;
    }
    iface->init();
}

void HidlAPI::setUserPref(bool userPref) {
    auto iface = getLocHidlXTIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlXT interface NULL!");
        return;
    }
    iface->setUserPref(userPref);
}

void HidlAPI::getGnssSvTypeConfig() {
    auto iface = getLocHidlGnssConfigIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlGnssConfig interface NULL!");
        return;
    }
    iface->getGnssSvTypeConfig();
}

void HidlAPI::setGnssSvTypeConfig(std::vector<uint8_t> svTypeVec) {
    auto iface = getLocHidlGnssConfigIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlGnssConfig interface NULL!");
        return;
    }
    std::vector<GnssConstellationType> disabledSvTypeVec;
    for (int i = 0; i < svTypeVec.size(); i++) {
        disabledSvTypeVec.push_back(intToGnssConstellation(svTypeVec[i]));
    }
    iface->setGnssSvTypeConfig(disabledSvTypeVec);
}

void HidlAPI::resetGnssSvTypeConfig() {
    auto iface = getLocHidlGnssConfigIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlGnssConfig interface NULL!");
        return;
    }
    iface->resetGnssSvTypeConfig();
}

bool HidlAPI::readIzatConfig() {
    auto iface = getLocHidlIzatConfigIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlIzatConfig interface NULL!");
        return false;
    }
    iface->readConfig();
    return true;
}

void HidlAPI::dataConnOpenExt(uint8_t agpsType, std::string apn, int bearerType) {
    auto iface = getLocHidlAgnssIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlAGnss interface NULL!");
        return;
    }
    IAGnss_v1_0::ApnIpType type;
    if (bearerType == 0) {
        type = IAGnss_v1_0::ApnIpType::IPV4;
    } else if (bearerType == 1) {
        type = IAGnss_v1_0::ApnIpType::IPV4V6;
    } else {
        type = IAGnss_v1_0::ApnIpType::INVALID;
    }
    iface->dataConnOpenExt(apn, type, static_cast<IAGnssCallback_v1_0::AGnssType>(agpsType));
}

void HidlAPI::dataConnClosedExt(uint8_t agpsType) {
    auto iface = getLocHidlAgnssIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlAGnss interface NULL!");
        return;
    }
    iface->dataConnClosedExt(static_cast<IAGnssCallback_v1_0::AGnssType>(agpsType));
}

void HidlAPI::dataConnFailedExt(uint8_t agpsType) {
    auto iface = getLocHidlAgnssIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlAGnss interface NULL!");
        return;
    }
    iface->dataConnFailedExt(static_cast<IAGnssCallback_v1_0::AGnssType>(agpsType));
}

void HidlAPI::getDebugReport(int maxReports, loc_core::SystemStatusReports& systemReports) {
    auto iface = getLocHidlDebugReportServiceIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlDebugReportService interface NULL!");
        return;
    }

    ILocHidlDebugReportService::LocHidlSystemStatusReports hidlReports = {};
    iface->getReport_1_1(maxReports,
            [&hidlReports]
            (const ILocHidlDebugReportService::ILocHidlDebugReportService::\
                    LocHidlSystemStatusReports& reports) {
                hidlReports = reports;
            });

    populateTimeAndClockReport(hidlReports, systemReports, maxReports);
    populateXoStateReport(hidlReports, systemReports, maxReports);
    populateRfParamsReport(hidlReports, systemReports, maxReports);
    populateErrRecoveryReport(hidlReports, systemReports, maxReports);
    populateInjectedPositionReport(hidlReports, systemReports, maxReports);
    populateBestPositionReport(hidlReports, systemReports, maxReports);
    populateXtraReport(hidlReports, systemReports, maxReports);
    populateEphemerisReport(hidlReports, systemReports, maxReports);
    populateSvHealthReport(hidlReports, systemReports, maxReports);
    populatePdrReport(hidlReports, systemReports, maxReports);
    populateNavDataReport(hidlReports, systemReports, maxReports);
    populatePositionFailureReport(hidlReports, systemReports, maxReports);
}

void HidlAPI::unregisterWiFiDBUpdater() {
    auto iface = getLocHidlWiFiDBReceiverIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlWiFiDBReceiver interface NULL!");
        return;
    }
    iface->unregisterWiFiDBUpdater();
}

void HidlAPI::sendAPListRequest(int32_t expireInDays) {
    auto iface = getLocHidlWiFiDBReceiverIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlWiFiDBReceiver interface NULL!");
        return;
    }
    iface->sendAPListRequest(expireInDays);
}

void HidlAPI::pushWiFiDB(std::vector<izat_remote_api::WiFiDBUpdater::APLocationData> apLocationData,
        std::vector<izat_remote_api::WiFiDBUpdater::APSpecialInfo> apSpecialInfo,
        int32_t daysValid, bool isLookup) {
    auto iface = getLocHidlWiFiDBReceiverIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlWiFiDBReceiver interface NULL!");
        return;
    }
    std::vector<ILocHidlWiFiDBReceiver::LocHidlApLocationData> apLocList;
    std::vector<ILocHidlWiFiDBReceiver::LocHidlApSpecialInfo> splList;
    int16_t apLocationDataListSize = apLocationData.size();
    int16_t apSpecialInfoListSize = apSpecialInfo.size();

    for (int i = 0; i < apLocationDataListSize; i++) {
        ILocHidlWiFiDBReceiver::LocHidlApLocationData apLocData;
        apLocData.mac_R48b = apLocationData[i].mac_R48b;
        apLocData.latitude = apLocationData[i].latitude;
        apLocData.longitude = apLocationData[i].longitude;
        apLocData.max_antenna_range = apLocationData[i].max_antena_range;
        apLocData.horizontal_error = apLocationData[i].horizontal_error;
        apLocData.reliability = apLocationData[i].reliability;
        apLocData.valid_bits = (int8_t)apLocationData[i].valid_mask;
        apLocList.push_back(apLocData);
    }

    for (int i = 0; i < apSpecialInfoListSize; i++) {
        ILocHidlWiFiDBReceiver::LocHidlApSpecialInfo apInfo;
        apInfo.mac_R48b = apSpecialInfo[i].mac_R48b;
        apInfo.info = apSpecialInfo[i].info;
        splList.push_back(apInfo);
    }

    iface->pushAPWiFiDB_2_1(apLocList, apLocationDataListSize, splList, apSpecialInfoListSize,
            daysValid, isLookup);
}

void HidlAPI::unregisterWWANDBUpdater() {
    auto iface = getLocHidlWwanDBReceiverIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlWwanDBReceiver interface NULL!");
        return;
    }
    iface->unregisterWWANDBUpdater();
}

void HidlAPI::sendBSListRequest(int32_t expireInDays) {
    auto iface = getLocHidlWwanDBReceiverIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlWwanDBReceiver interface NULL!");
        return;
    }
    iface->sendBSListRequest(expireInDays);
}

void HidlAPI::pushBSWWANDB(
        std::vector<BSLocationData_s> bsLocationDataList,
        std::vector<BSSpecialInfo_s> bsSpecialInfoList,
        int32_t daysValid) {
    auto iface = getLocHidlWwanDBReceiverIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlWwanDBReceiver interface NULL!");
        return;
    }
    std::vector<ILocHidlWWANDBReceiver::LocHidlBsLocationData> bsLocList;
    std::vector<ILocHidlWWANDBReceiver::LocHidlBsSpecialInfo> splList;
    int16_t bsLocationDataListSize = bsLocationDataList.size();
    int16_t bsSpecialInfoListSize = bsSpecialInfoList.size();

    for (int i = 0; i < bsLocationDataListSize; i++) {
        ILocHidlWWANDBReceiver::LocHidlBsLocationData bsLocData;
        bsLocData.cellType = bsLocationDataList[i].cellType;
        bsLocData.cellRegionID1 = bsLocationDataList[i].cellRegionID1;
        bsLocData.cellRegionID2 = bsLocationDataList[i].cellRegionID2;
        bsLocData.cellRegionID3 = bsLocationDataList[i].cellRegionID3;
        bsLocData.cellRegionID4 = bsLocationDataList[i].cellRegionID4;
        bsLocData.latitude = bsLocationDataList[i].latitude;
        bsLocData.longitude = bsLocationDataList[i].longitude;
        bsLocData.valid_bits = bsLocationDataList[i].valid_bits;
        bsLocData.horizontal_coverage_radius = bsLocationDataList[i].horizontal_coverage_radius;
        bsLocData.horizontal_confidence = bsLocationDataList[i].horizontal_confidence;
        bsLocData.horizontal_reliability = bsLocationDataList[i].horizontal_reliability;
        bsLocData.altitude = bsLocationDataList[i].altitude;
        bsLocData.altitude_uncertainty = bsLocationDataList[i].altitude_uncertainty;
        bsLocData.altitude_confidence = bsLocationDataList[i].altitude_confidence;
        bsLocData.altitude_reliability = bsLocationDataList[i].altitude_reliability;
        bsLocList.push_back(bsLocData);
    }

    for (int i = 0; i < bsSpecialInfoListSize; i++) {
        ILocHidlWWANDBReceiver::LocHidlBsSpecialInfo bsInfo;
        bsInfo.cellType = bsSpecialInfoList[i].cellType;
        bsInfo.cellRegionID1 = bsSpecialInfoList[i].cellRegionID1;
        bsInfo.cellRegionID2 = bsSpecialInfoList[i].cellRegionID2;
        bsInfo.cellRegionID3 = bsSpecialInfoList[i].cellRegionID3;
        bsInfo.cellRegionID4 = bsSpecialInfoList[i].cellRegionID4;
        bsInfo.info = bsSpecialInfoList[i].info;
        splList.push_back(bsInfo);
    }

    iface->pushBSWWANDB(bsLocList, bsLocationDataListSize, splList, bsSpecialInfoListSize,
            daysValid);
}

void HidlAPI::unregisterWiFiDBProvider() {
    auto iface = getLocHidlWiFiDBProviderIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlWiFiDBProvider interface NULL!");
        return;
    }
    iface->unregisterWiFiDBProvider();
}

void HidlAPI::sendAPObsLocDataRequest() {
    auto iface = getLocHidlWiFiDBProviderIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlWiFiDBProvider interface NULL!");
        return;
    }
    iface->sendAPObsLocDataRequest();
}

void HidlAPI::unregisterWWANDBProvider() {
    auto iface = getLocHidlWwanDBProviderIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlWwanDBProvider interface NULL!");
        return;
    }
    iface->unregisterWWANDBProvider();
}

void HidlAPI::sendBSObsLocDataRequest() {
    auto iface = getLocHidlWwanDBProviderIface();
    if (iface == nullptr) {
        ALOGE("ILocHidlWwanDBProvider interface NULL!");
        return;
    }
    iface->sendBSObsLocDataRequest();
}

#endif
} // namespace garden
