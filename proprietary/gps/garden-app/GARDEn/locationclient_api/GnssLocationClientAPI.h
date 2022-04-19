/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
#ifndef GNSSLOCATIONCLIENTAPI_H
#define GNSSLOCATIONCLIENTAPI_H

#include <vector>
#include <gps_extended_c.h>
#include <loc_gps.h>
#include "IGnssAPI.h"
#include "GnssCb.h"
#include "LocationClientApi.h"
#include "LocationClientApiImpl.h"

using namespace location_client;

namespace garden {

using GeofenceBreachTypeMask = ::GeofenceBreachTypeMask;
using Location = ::Location;

enum GnssCallbackOptionsMask {
    LOCATION_BIT =      (1 << 0),
    GNSS_SV_BIT =       (1 << 1),
    GNSS_NMEA_BIT =     (1 << 2),
    GNSS_DATA_BIT =     (1 << 3),
    GNSS_MEAS_BIT =     (1 << 4),
    LOC_SYS_INFO_BIT =  (1 << 5),
    GNSS_POLY_BIT =     (1 << 6),
};

enum GnssStartType {
    GNSS_REPORT     = 0,
    LOCATION_REPORT = 1,
    ENGINE_REPORT   = 2
};

typedef std::function<void(
    void
)> LocationReceivedCb;

typedef std::function<void(
    void
)> PingReceivedCb;

struct ClientReportsCbs {
    LocationReceivedCb      locationReceivedCallback;
    PingReceivedCb          pingReceivedCallback;
};

class GnssLocationClientAPI : public IGnssAPI {
private:
    LocationClientApi*      mPClient;
    LocationClientApiImpl*  mPClientImpl;
    GnssReportCbs           mReportCbs;
    EngineReportCbs         mEngineReportCbs;
    GnssStartType           mStartType;
    uint32_t                mIntervalInMs;
    uint32_t                mDistanceInMeters;
    ::LocReqEngineTypeMask  mEngMask;
    vector<Geofence>        mGeofences;
    ::PositioningEngineMask   mPosEngineMask;
    bool                    mEnableOutput;
    bool                    mRouteToNmeaPort;
    bool                    mAutoMode;

public:
    GnssLocationClientAPI();
    virtual ~GnssLocationClientAPI();
    void setGnssCbs(GnssCbBase* callbacks) override;
    void setFlpCbs(GnssCbBase* callbacks) override;
    void setGeofenceCbs(GnssCbBase* callbacks) override;
    int gnssStart(void) override;
    int gnssStop(void) override;
    void gnssCleanup(void) override;
    int gnssInjectLocation(double latitude, double longitude,
            float accuracy) override;
    void gnssDeleteAidingData(LocGpsAidingData flags) override;
    int gnssSetPositionMode(LocGpsPositionMode mode,
            LocGpsPositionRecurrence recurrence, uint32_t min_interval,
            uint32_t preferred_accuracy, uint32_t preferred_time,
            bool lowPowerMode=false) override;
    void configurationUpdate(const char* config_data, int32_t length) override;
    void updateNetworkAvailability(int available, const char* apn) override;
    void gnssNiResponse(uint32_t id, GnssNiResponse response) override;
    int gnssInjectBestLocation(Location location) override;

    //flp
    int flpStartSession(uint32_t mode, LocationOptions& options, uint32_t id=1) override;
    int flpStopSession(uint32_t id=1) override;
    int flpGetBatchSize() override;
    int flpUpdateSession(uint32_t sessionMode, LocationOptions& options, uint32_t id=1) override;
    int flpGetBatchedLocation(int lastNLocations, uint32_t id=1) override;
    void flpCleanUp() override;

    // geofence
    virtual void addGeofence(uint32_t geofenceId, double latitudeDegrees,
            double longitudeDegrees, double radiusMeters,
            GeofenceBreachTypeMask monitorTransitions,
            uint32_t notificationResponsivenessMs,
            uint32_t dwellTimeSeconds) override;
    virtual void removeGeofence(uint32_t geofenceId) override;
    virtual void modifyGeofence(uint32_t geofenceId,
            GeofenceBreachTypeMask monitorTransitions,
            uint32_t notificationResponsivenessMs,
            uint32_t dwellTimeSeconds) override;
    virtual void pauseGeofence(uint32_t geofenceId) override;
    virtual void resumeGeofence(uint32_t geofenceId,
            GeofenceBreachTypeMask monitorTransitions) override;
    virtual void removeAllGeofences() override;

    // location client api specific
    void setClientReportsCbs(ClientReportsCbs* callbacks);
    void gnssStartOptions(GnssStartType startType, uint32_t intervalInMs,
            uint32_t distanceInMeters, ::LocReqEngineTypeMask engMask,
            int gnssReportOptions);
    uint32_t gnssDeleteAidingData(GnssAidingData& data);
    void testGetEnergy();
    void testRegSystemInfo();
    void testUnregSystemInfo();
    void pingTest();
    inline void setCbOutput(bool enableOutput) { mEnableOutput = enableOutput; }
    inline bool getCbOutput() { return mEnableOutput; }
    inline void setAutoModeOptions(bool autoMode, bool routeToNmeaPort) {
        mAutoMode = autoMode;
        mRouteToNmeaPort = routeToNmeaPort;
    }
    inline bool getAutoMode() { return mAutoMode; }
    inline bool getRouteToNmeaPort() { return mRouteToNmeaPort; }
    void setEngineMask(::PositioningEngineMask posEngineMask) {
        mPosEngineMask = posEngineMask;
    }
};

} // namespace garden

#endif //GNSSLOCATIONCLIENTAPI_H
