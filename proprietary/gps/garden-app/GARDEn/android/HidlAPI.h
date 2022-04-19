/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2017-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
#ifndef HIDLAPI_H
#define HIDLAPI_H

#include "IGnssAPI.h"
#include <LocationAPI.h>
#ifdef LOCHIDL
#include "DataItemConcreteTypes.h"
#include "IDataItemCore.h"
#include "SystemStatus.h"
#include "IzatWiFiDBReceiver.h"
#include "IzatWiFiDBProvider.h"
#include "wwan_db_provider.h"
#include "wwan_db_receiver.h"
#endif

namespace garden {

using getDebugData_2_0_cb = std::function<void(GnssDebugReport& debugData)>;
class HidlAPI : public IGnssAPI {
public:
    HidlAPI();
    virtual ~HidlAPI() = default;
    void setGnssCbs(GnssCbBase* callbacks) override;
    void setFlpCbs(GnssCbBase* callbacks) override;
    void setGeofenceCbs(GnssCbBase* callbacks) override;
#ifdef LOCHIDL
    void setIzatSubscriptionCbs(GnssCbBase* callbacks);
    void setIzatProviderCbs(GnssCbBase* callbacks);
    void setOsNpCbs(GnssCbBase* callbacks);
    void setGnssConfigCbs(GnssCbBase* callbacks);
    void setIzatConfigCbs(GnssCbBase* callbacks);
    void setAGnssExtCbs(GnssCbBase* callbacks);
    void setWiFiDBReceiverCbs(GnssCbBase* callbacks);
    void setWWanDBReceiverCbs(GnssCbBase* callbacks);
    void setWiFiDBProviderCbs(GnssCbBase* callbacks);
    void setWWanDBProviderCbs(GnssCbBase* callbacks);
#endif

    int gnssStart(void) override;
    int gnssStop(void) override;
    void gnssCleanup(void) override;
    int gnssInjectLocation(double latitude, double longitude, float accuracy) override;
    void gnssDeleteAidingData(LocGpsAidingData flags) override;
    int gnssSetPositionMode(LocGpsPositionMode mode,
            LocGpsPositionRecurrence recurrence, uint32_t min_interval,
            uint32_t preferred_accuracy, uint32_t preferred_time, bool lowPowerMode) override;
    void configurationUpdate(const char* config_data, int32_t length) override;
    void updateNetworkAvailability(int available, const char* apn) override;

    void gnssNiResponse(uint32_t id, GnssNiResponse response) override;
    int gnssInjectBestLocation(Location location) override;

    //flp
    virtual int flpStartSession(uint32_t mode, LocationOptions& options, uint32_t id=1) override;
    virtual int flpStopSession(uint32_t id=1) override;
    virtual int flpGetBatchSize() override;
    virtual int flpUpdateSession(uint32_t /*sessionMode*/, LocationOptions& /*options*/,
            uint32_t id=1) override;
    virtual int flpGetBatchedLocation(int /*lastNLocations*/, uint32_t id=1) override;
    virtual void flpCleanUp() override;

    // geofence
    virtual void addGeofence(uint32_t geofenceId, double latitudeDegrees, double longitudeDegrees,
            double radiusMeters, GeofenceBreachTypeMask monitorTransitions,
            uint32_t notificationResponsivenessMs,
            uint32_t dwellTimeSeconds = 0) override;
    virtual void removeGeofence(uint32_t geofenceId) override;
    virtual void modifyGeofence(uint32_t /*geofenceId*/,
            GeofenceBreachTypeMask /*monitorTransitions*/,
            uint32_t /*notificationResponsivenessMs*/,
            uint32_t dwellTimeSeconds = 0) override;
    virtual void pauseGeofence(uint32_t geofenceId) override;
    virtual void resumeGeofence(uint32_t geofenceId,
            GeofenceBreachTypeMask /*monitorTransitions*/) override;
    virtual void removeAllGeofences() override;

    void setAGnssCbs(GnssCbBase* callbacks);
    void setAGnssRilCbs(GnssCbBase* callbacks);
    void setGnssMeasurementCbs(GnssCbBase* callbacks);
    void setGnssVisibilityControlCbs(GnssCbBase* callbacks);

    //IAGnss
    bool AGnss_2_0_dataConnClosed_2_0();
    bool AGnss_2_0_dataConnFailed_2_0();
    bool AGnss_2_0_setServer_2_0(int32_t type, const std::string& hostname, int32_t port);
    bool AGnss_2_0_dataConnOpen_2_0(uint64_t networkHandle, const std::string& apn,
            int32_t apnIpType);

    //IAGnssRil
    void AGnssRil_1_0_setRefLocation(int type, int mcc, int mnc, int lac, int cid);
    bool AGnssRil_1_0_setSetId(int type, const std::string& setid);
    bool AGnssRil_1_0_updateNetworkAvailability(bool available, const std::string& apn);
    bool AGnssRil_2_0_updateNetworkState_2_0(uint64_t networkHandle, bool connected,
            uint16_t attributes, const std::string& apn);

    //IGnssConfiguration
    bool GnssConfiguration_1_1_setBlacklist(
            const std::vector<int>& constellations, const std::vector<int>& svIDs);

    //IGnssDebug
    void GnssDebug_2_0_getDebugData_2_0(getDebugData_2_0_cb _hidl_cb);

    //IGnssMeasurement
    void GnssMeasurement_1_0_close();

    //IGnssVisibilityControl
    bool GnssVisibilityControl_1_0_enableNfwLocationAccess(
            const std::vector<std::string>& proxyApps);

    //IGnss
    bool Gnss_1_0_injectTime(int64_t timeMs, int64_t timeReferenceMs, int32_t uncertaintyMs);

#ifdef LOCHIDL
    //LOC HIDL APIs
    //Izat subscription
    void dataItemsUpdate (std::list<IDataItemCore *> & dlist);
    void dataItemUpdate (IDataItemCore* dataItem);
    void izatSubscriptionCleanUp();

    //Ril info monitor
    void rilInfoMonitorInit();
    void cinfoInject(int32_t cid, int32_t lac, int32_t mnc, int32_t mcc, bool roaming);
    void oosInform();
    void niSuplInit(std::string& str);
    void chargerStatusInject(int32_t status);

    //Izatprovider
    bool onEnable();
    bool onDisable();
    bool onAddRequest(int providerType, int32_t numFixes, int32_t tbf,
            float displacement, int accuracy);
    bool onRemoveRequest(int providerType, int32_t numFixes, int32_t tbf,
            float displacement, int accuracy);

    //OsNpGlue
    void OsNpLocationChanged(Location& location);

    //XT
    void xtInit();
    void setUserPref(bool userPref);

    //Gnss config
    void getGnssSvTypeConfig();
    void setGnssSvTypeConfig(std::vector<uint8_t> svTypeVec);
    void resetGnssSvTypeConfig();
    //Izat config
    bool readIzatConfig();
    //AGNSS
    void dataConnOpenExt(uint8_t agpsType, std::string apn, int bearerType);
    void dataConnClosedExt(uint8_t agpsType);
    void dataConnFailedExt(uint8_t agpsType);

    //Debug report
    void getDebugReport(int maxReports, loc_core::SystemStatusReports& systemReports);

    //Wifi/Wwan DB receiver
    void unregisterWiFiDBUpdater();
    void sendAPListRequest(int32_t expireInDays);
    void pushWiFiDB(std::vector<izat_remote_api::WiFiDBUpdater::APLocationData> apLocationData,
            std::vector<izat_remote_api::WiFiDBUpdater::APSpecialInfo> apSpecialInfo,
            int32_t daysValid, bool isLookup);

    void unregisterWWANDBUpdater();
    void sendBSListRequest(int32_t expireInDays);
    void pushBSWWANDB(
            std::vector<BSLocationData_s> bsLocationDataList,
            std::vector<BSSpecialInfo_s> bsSpecialInfoList,
            int32_t daysValid);

    void unregisterWiFiDBProvider();
    void sendAPObsLocDataRequest();

    void unregisterWWANDBProvider();
    void sendBSObsLocDataRequest();
#endif
private:
    GnssCbBase* mGnssCallbacks;
    GnssCbBase* mFlpCbs;
    GnssCbBase* mGeofenceCbs;

};

} // namespace garden

#endif //GNSSLOCATION_H
