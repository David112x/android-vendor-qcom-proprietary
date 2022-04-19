/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
#ifndef GNSSCBBASE_H
#define GNSSCBBASE_H
#include <LocationAPIClientBase.h>
#include <gps_extended_c.h>
#include <mutex>
#ifdef LOCHIDL
#include "IzatWiFiDBReceiver.h"
#include "IzatWiFiDBProvider.h"
#include "wwan_db_provider.h"
#include "wwan_db_receiver.h"
#endif
using std::mutex;

namespace garden {
using lock_guard = std::lock_guard<mutex>;

class GnssCbBase {
public:
    GnssCbBase() = default;
    virtual ~GnssCbBase() = default;

    virtual void gnssLocationCb(Location& location);
    virtual void gnssStatusCb(uint32_t status);
    virtual void gnssSvStatusCb(GnssSvNotification& svNotify);
    virtual void gnssNmeaCb(uint64_t timestamp, const char* nmea, int length);
    virtual void gnssMeasurementsCb(GnssMeasurementsNotification& gnssMeasurementsNotification);
    virtual void gnssDataCb(GnssDataNotification& dataNotify);
    virtual void gnssSetCapabilitiesCb(uint32_t capabilities);
    virtual void gnssRequestUtcTimeCb();
    virtual void gnssNiNotifyCb(uint32_t id, GnssNiNotification& notification);
    virtual void gnssLocInfoNotifCb(GnssLocationInfoNotification gnssLocInfoNotif);
    virtual void gnssRequestLocationCb(bool independentFromGnss, bool isUserEmergency) {}

    //flp
    virtual void batchingCb(size_t /*count*/, Location* /*location*/) {}
    virtual void startBatchingCb(LocationError /*error*/) {}
    virtual void stopBatchingCb(LocationError /*error*/) {}
    virtual void updateBatchingCb(LocationError /*error*/) {}
    virtual void getBatchedLocationCb(LocationError /*error*/) {}
    virtual void startTrackingCb(LocationError /*error*/) {}
    virtual void stopTrackingCb(LocationError /*error*/) {}
    virtual void updateTrackingCb(LocationError /*error*/) {}

    // geofence
    virtual void geofenceBreachCb(uint32_t /*id*/, Location /*location*/,
            GeofenceBreachType /*transition*/, uint64_t /*timeStamp*/) {}
    virtual void geofenceStatusCb(GeofenceStatusAvailable /*status*/) {}
    virtual void addGeofencesCb(uint32_t /*id*/, LocationError /*error*/) {}
    virtual void removeGeofencesCb(uint32_t /*id*/, LocationError /*error*/) {}
    virtual void pauseGeofencesCb(uint32_t /*id*/, LocationError /*error*/) {}
    virtual void resumeGeofencesCb(uint32_t /*id*/, LocationError /*error*/) {}
    virtual void modifyGeofencesCb(uint32_t /*id*/, LocationError /*error*/) {}

    //AGnss
    virtual void agnssStatusCb(uint8_t agnssType, uint8_t status) {}
    //AGnssRil
    virtual void requestSetIdCb(uint32_t setIdflag) {}
    virtual void requestRefLocCb() {}
    //GnssMeasurement
    virtual void gnssMeasurementCb(GnssMeasurementsNotification data) {}
    //GnssXtra
    virtual void downloadReqCb() {}
    //GnssVisibilityControl
    virtual void nfwNotifyCb(GnssNfwNotification notification) {}
    virtual bool isInEmergencySession() { return true; }

    // data items
    virtual void requestDataCb(std::vector<int> dataItemArray) {}
    virtual void updateSubscribeCb(std::vector<int> dataItemArray, bool subscribe) {}
    virtual void unsbscribeAllCb() {}
    virtual void turnOnCb(int32_t dit, int32_t timeout) {}
    virtual void turnOffCb(int32_t dit) {}

    //IzatProvider
    virtual void izatProviderLocationCb(Location& location) {}
    virtual void onStatusChanged(uint8_t status) {}

    //OsNpGlue
    virtual void onSetRequest(int providerType,
        int32_t numFixes, int32_t tbf, float displacement, int accuracy) {}
    virtual void onStopRequest() {}

    //Izat/Gnss config
    virtual void getGnssSvTypeConfigCb(std::vector<uint8_t> disabledSvTypeList) {}
    virtual void izatConfigcallback(std::string izatConfigContent) {}

    //AGNSS EXT
    virtual void agnssStatusIpV4Cb(int agpsType, uint32_t apnTypeMask, int status) {}

#ifdef LOCHIDL
    //Wifi/Wwan DB receiver
    virtual void apListUpdateCallback(
            std::vector<izat_remote_api::WiFiDBUpdater::APInfo> apList,
            uint8_t status, Location location, bool locationValid) {}
    virtual void wifiDBReceiverStatusUpdateCallback(bool status, std::string reason) {}
    virtual void wifiDBReceiverServiceRequestCallback() {}
    virtual void wifiDBReceiverAttachVmOnCallback() {}

    virtual void bsListUpdateCallback(
            std::vector<BSInfo_s> bsInfoList, uint8_t status, Location location) {}
    virtual void wwanDBReceiverStatusUpdateCallback(bool status, std::string reason) {}
    virtual void wwanDBReceiverServiceRequestCallback() {}
    virtual void wwanDBReceiverAttachVmOnCallback() {}

    virtual void apObsLocDataUpdateCallback(
            std::vector<izat_remote_api::WiFiDBProvider::APObsData> apObsLocDataList,
            uint8_t apListStatus) {}
    virtual void wifiDBProviderServiceRequestCallback() {}
    virtual void wifiDBProviderAttachVmOnCallback() {}

    virtual void bsObsLocDataUpdateCallback(
            std::vector<BSObsData_s> bsObsLocDataList, int8_t bsListStatus) {}
    virtual void wwanDBProviderServiceRequestCallback() {}
    virtual void wwanDBProviderAttachVmOnCallback() {}
#endif
};

}

#endif // GNSSCBBASE_H
