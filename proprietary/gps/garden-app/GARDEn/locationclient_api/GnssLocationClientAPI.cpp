/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
#include "GnssLocationClientAPI.h"
#include <dlfcn.h>
#include <loc_pla.h>
#include <log_util.h>
#include <fcntl.h>
#include <sstream>
#include "GardenUtil.h"
#include <mutex>
#include <algorithm>

using std::mutex;
using lock_guard = std::lock_guard<std::mutex>;
using namespace location_client;
#define NMEA_PORT "/dev/at_usb1"
static int ttyFd = -1;
// debug events counter
static uint32_t numLocationCb = 0;
static uint32_t numGnssLocationCb = 0;
static uint32_t numEngLocationCb = 0;
static uint32_t numGnssSvCb = 0;
static uint32_t numGnssNmeaCb = 0;
static uint32_t numDataCb = 0;
static uint32_t numMeasurementsCb = 0;
static uint32_t numPolyCb = 0;
static uint32_t numBatchingCb = 0;
static uint32_t numGeofenceCb = 0;
static uint32_t numGfBreachCb = 0;

namespace garden {

static GnssCbBase* sGnssCbs = nullptr;
static std::mutex sGnssLock;
static class GnssLocationClientAPI* mPlocationClient;
static ClientReportsCbs* sClientReportsCbs = nullptr;

static bool openPort(void)
{
    bool retVal = true;

    gardenPrint("opening NMEA port %s ", NMEA_PORT);
    ttyFd = open(NMEA_PORT, O_RDWR | O_NOCTTY | O_NDELAY);
    if (ttyFd == -1) {
        /* Could not open the port. */
        gardenPrint("Unable to open %s \n", NMEA_PORT);
        retVal = false;
    } else {
        gardenPrint("openPort success ttyFd: %d\n", ttyFd);
    }
    return retVal;
}

static bool sendNMEAToTty(const std::string& nmea)
{
    int n;
    char buffer[201] = { 0 };
    bool retVal = true;
    strlcpy(buffer, nmea.c_str(), sizeof(buffer));
    /* NMEA strings contains "\r\n" at end of the string, remove '\r' and send it */
    char* p = strchr(buffer, '\r');
    if ((NULL != p) && (*(p + 1) == '\n')) {
        *p = '\n';
        *(p + 1) = '\0';
    }
    if (1 < nmea.length() && sizeof(buffer) > nmea.length()) {
        n = write(ttyFd, buffer, nmea.length() - 1);
        if (n < 0) {
            gardenPrint("write() of %d bytes failed!\n", n);
            retVal = false;
        } else if (0 == n) {
            gardenPrint("write() of %d bytes returned 0, errno:%d [%s]\n",
                nmea.length(), errno, strerror(errno));
            /* Sleep of 0.1 msec and reattempt to write*/
            usleep(100);
            n = write(ttyFd, buffer, nmea.length() - 1);
            if (n < 0) {
                gardenPrint("reattempt write() failed! errno:%d [%s] \n", errno, strerror(errno));
                retVal = false;
            } else if (0 == n) {
                gardenPrint("reattempt write() of %d bytes returned 0, errno:%d [%s]\n",
                    nmea.length(), errno, strerror(errno));
            }
        }
    } else {
        gardenPrint("Failed to write Len: %d %s \n", nmea.length(), nmea.c_str());
    }
    return true;
}

static void onCapabilitiesCb(location_client::LocationCapabilitiesMask mask) {
    gardenPrint("<<< onCapabilitiesCb mask=0x%x", mask);
}

static void onResponseCb(location_client::LocationResponse response) {
    gardenPrint("<<< onResponseCb err=%u", response);
}

static void locationCb(const location_client::Location& location) {
    numLocationCb++;
    if (mPlocationClient->getCbOutput()) {
        gardenPrint("<<< locationCb cnt=%u time=%" PRIu64" mask=0x%x lat=%f lon=%f alt=%f",
            numLocationCb,
            location.timestamp,
            location.flags,
            location.latitude,
            location.longitude,
            location.altitude);
    }
}

static void gnssLocationCb(const location_client::GnssLocation& gnssLocation) {
    numGnssLocationCb++;
    if (mPlocationClient->getCbOutput()) {
        gardenPrint("<<< gnssLocationCb cnt=%u time=%" PRIu64" mask=0x%x lat=%f lon=%f alt=%f "
            "info mask=0x%x eng type %d, eng mask 0x%x, conformityIndex %f",
            numGnssLocationCb,
            gnssLocation.timestamp,
            gnssLocation.flags,
            gnssLocation.latitude,
            gnssLocation.longitude,
            gnssLocation.altitude,
            gnssLocation.gnssInfoFlags,
            gnssLocation.locOutputEngType,
            gnssLocation.locOutputEngMask,
            gnssLocation.conformityIndex);
    }
    if (mPlocationClient->getAutoMode()) {
        if (sClientReportsCbs) {
            sClientReportsCbs->locationReceivedCallback();
        }
    }
}

static void gnssSvCb(const std::vector<location_client::GnssSv>& gnssSvs) {
    numGnssSvCb++;
    if (mPlocationClient->getCbOutput()) {
        stringstream ss;
        ss << "<<< gnssSvCb c=" << numGnssSvCb << " s=" << gnssSvs.size();
        for (auto sv : gnssSvs) {
            ss << " " << sv.type << ":" << sv.svId << "/" << (uint32_t)sv.cN0Dbhz;
        }
        string s = ss.str();
        gardenPrint("%s", s.c_str());
    }
}

static void gnssNmeaCb(uint64_t timestamp, const std::string& nmea) {
    numGnssNmeaCb++;
    if (mPlocationClient->getCbOutput()) {
        gardenPrint("<<< gnssNmeaCb cnt=%u time=%" PRIu64" nmea=%s",
                numGnssNmeaCb, timestamp, nmea.c_str());
    }

    if (mPlocationClient->getRouteToNmeaPort()) {
        if (-1 == ttyFd) {
            if (openPort()) {
                sendNMEAToTty(nmea);
            }
        } else {
            sendNMEAToTty(nmea);
        }
    }
}

static void gnssEnergyConsumedInfoCb(const GnssEnergyConsumedInfo& gnssEneryConsumed) {
    gardenPrint("<<< onGnssEnergyConsumedInfoCb energy: (valid=%d, value=%" PRIu64"",
            (gnssEneryConsumed.flags & ENERGY_CONSUMED_SINCE_FIRST_BOOT_BIT) != 0,
            gnssEneryConsumed.totalEnergyConsumedSinceFirstBoot);
}

static void gnssEnergyConsumedResponseCb(location_client::LocationResponse response) {
    gardenPrint("<<< onGnssEnergyConsumedResponseCb err=%u\n", response);
}

static void locationSystemInfoCb(const location_client::LocationSystemInfo& systemInfo) {
    gardenPrint("<<< onLocationSystemInfoCb: "
        "(system info mask=0x%x, leap second info mask=0x%x"
        " gps timestamp of leap second change (%d week, %d msec), "
        " leap seconds before change %d, leap seconds after change %d,"
        " leap second current %d",
        systemInfo.systemInfoMask,
        systemInfo.leapSecondSysInfo.leapSecondInfoMask,
        systemInfo.leapSecondSysInfo.leapSecondChangeInfo.gpsTimestampLsChange.systemWeek,
        systemInfo.leapSecondSysInfo.leapSecondChangeInfo.gpsTimestampLsChange.systemMsec,
        systemInfo.leapSecondSysInfo.leapSecondChangeInfo.leapSecondsBeforeChange,
        systemInfo.leapSecondSysInfo.leapSecondChangeInfo.leapSecondsAfterChange,
        systemInfo.leapSecondSysInfo.leapSecondCurrent);
}

static void locationSystemInfoResponseCb(location_client::LocationResponse response) {
    gardenPrint("<<< onLocationSystemInfoCb err=%u\n", response);
}

static void gnssDataCb(const location_client::GnssData& gnssData) {
    numDataCb++;
    if (mPlocationClient->getCbOutput()) {
        gardenPrint("<<< gnssDataCb cnt=%u mPlocationClient=%p", numDataCb, mPlocationClient);
    }
}

static void gnssMeasurementsCb(const location_client::GnssMeasurements& gnssMeasurements) {
    numMeasurementsCb++;
    if (mPlocationClient->getCbOutput()) {
        gardenPrint("<<< gnssMeasurementsCb cnt=%u mPlocationClient=%p",
                    numMeasurementsCb,
                    mPlocationClient);
        gardenPrint(" Clocks Info");
        gardenPrint(" time_ns: %" PRId64 " full_bias_ns: %" PRId64 ""
            " bias_ns: %f, bias_uncertainty_ns: %f,"
            " drift_nsps: %f, drift_uncertainty_nsps: %f,"
            " hw_clock_discontinuity_count: %d, flags: 0x%04x",
            gnssMeasurements.clock.timeNs,
            gnssMeasurements.clock.fullBiasNs,
            gnssMeasurements.clock.biasNs,
            gnssMeasurements.clock.biasUncertaintyNs,
            gnssMeasurements.clock.driftNsps,
            gnssMeasurements.clock.driftUncertaintyNsps,
            gnssMeasurements.clock.hwClockDiscontinuityCount,
            gnssMeasurements.clock.flags);
        for (int i = 0;
             i < gnssMeasurements.measurements.size() && i < GNSS_MEASUREMENTS_MAX; i++) {
            gardenPrint("%02d : Const: %d,  svid: %d,"
                " time_offset_ns: %f, state %d,"
                " c_n0_dbhz: %f, c_freq_Hz: %f,"
                " pseudorange_rate_mps: %f,"
                " pseudorange_rate_uncertainty_mps: %f,"
                " agcLevelDb: %f, flags: 0x%08x",
                i + 1,
                gnssMeasurements.measurements[i].svType,
                gnssMeasurements.measurements[i].svId,
                gnssMeasurements.measurements[i].timeOffsetNs,
                gnssMeasurements.measurements[i].stateMask,
                gnssMeasurements.measurements[i].carrierToNoiseDbHz,
                gnssMeasurements.measurements[i].carrierFrequencyHz,
                gnssMeasurements.measurements[i].pseudorangeRateMps,
                gnssMeasurements.measurements[i].pseudorangeRateUncertaintyMps,
                gnssMeasurements.measurements[i].agcLevelDb,
                gnssMeasurements.measurements[i].flags);
        }
    }
}


static void gnssSvPolyCb(const location_client::GnssSvPoly & gnssSvPoly) {
    numPolyCb++;
    if (mPlocationClient->getCbOutput()) {
        gardenPrint("<<< gnssSvPolyCb cnt=%u, sv id=%d, sv constellation=%d, freq=%d,"
                    "mask=0x%" PRIx64 ", action type=%d, status=0x%x\n"
                    "T0=%f, iode=%u, enhancedIOD=%u, svPosUnc=%f, "
                    "ionoDelay=%f, ionoDot=%f, sbasIonoDelay=%f, "
                    "sbasIonoDot=%f, tropoDelay=%f, elevation=%f,"
                    "elevationDot=%f, elevationUnc=%f, gpsIscL1ca=%f"
                    "gpsIscL2c=%f, gpsIscL5I5=%f, gpsIscL5Q5=%f, gpsTgd= %f, gloTgdG1G2 = %f"
                    "bdsTgdB1=%f, bdsTgdB2=%f, bdsTgdB2a=%f, bdsIscB2a=%f"
                    "galBgdE1E5a=%f, galBgdE1E5b=%f, navicTgdL5=%f",
                    numPolyCb, gnssSvPoly.svId, gnssSvPoly.svConstellation,
                    gnssSvPoly.gloFrequency, gnssSvPoly.validityMask,
                    gnssSvPoly.actionType, gnssSvPoly.statusMask,
                    gnssSvPoly.T0, gnssSvPoly.iode, gnssSvPoly.enhancedIOD,
                    gnssSvPoly.svPosUnc, gnssSvPoly.ionoDelay, gnssSvPoly.ionoDot,
                    gnssSvPoly.sbasIonoDelay, gnssSvPoly.sbasIonoDot, gnssSvPoly.tropoDelay,
                    gnssSvPoly.elevation, gnssSvPoly.elevationDot, gnssSvPoly.elevationUnc,
                    gnssSvPoly.gpsIscL1ca, gnssSvPoly.gpsIscL2c, gnssSvPoly.gpsIscL5I5,
                    gnssSvPoly.gpsIscL5Q5, gnssSvPoly.gpsTgd, gnssSvPoly.gloTgdG1G2,
                    gnssSvPoly.bdsTgdB1, gnssSvPoly.bdsTgdB2, gnssSvPoly.bdsTgdB2a,
                    gnssSvPoly.bdsIscB2a, gnssSvPoly.galBgdE1E5a, gnssSvPoly.galBgdE1E5b,
                    gnssSvPoly.navicTgdL5);
        gardenPrint("<<< gnssSvPolyCb polyCofXYZ0 %f %f %f",
                    gnssSvPoly.polyCofXYZ0[0], gnssSvPoly.polyCofXYZ0[1],
                    gnssSvPoly.polyCofXYZ0[2]);
        gardenPrint("<<< gnssSvPolyCb polyCofXYZN %f %f %f  %f %f %f  %f %f %f",
                    gnssSvPoly.polyCofXYZN[0], gnssSvPoly.polyCofXYZN[1],
                    gnssSvPoly.polyCofXYZN[2], gnssSvPoly.polyCofXYZN[3],
                    gnssSvPoly.polyCofXYZN[4], gnssSvPoly.polyCofXYZN[5],
                    gnssSvPoly.polyCofXYZN[6], gnssSvPoly.polyCofXYZN[7],
                    gnssSvPoly.polyCofXYZN[8]);
        gardenPrint("<<< gnssSvPolyCb polyCofClockBias %f %f %f  %f",
                    gnssSvPoly.polyCofClockBias[0], gnssSvPoly.polyCofClockBias[1],
                    gnssSvPoly.polyCofClockBias[2],
                    gnssSvPoly.polyCofClockBias[3]);
        gardenPrint("<<< gnssSvPolyCb velCof %f %f %f  %f %f %f  %f %f %f  %f %f %f",
                    gnssSvPoly.velCof[0], gnssSvPoly.velCof[1], gnssSvPoly.velCof[2],
                    gnssSvPoly.velCof[3], gnssSvPoly.velCof[4], gnssSvPoly.velCof[5],
                    gnssSvPoly.velCof[6], gnssSvPoly.velCof[7], gnssSvPoly.velCof[8],
                    gnssSvPoly.velCof[9], gnssSvPoly.velCof[10], gnssSvPoly.velCof[11]);
    }
}

static void engLocationsCb(const std::vector<location_client::GnssLocation>& locations) {
    numEngLocationCb++;
    if (mPlocationClient->getCbOutput()) {
        gardenPrint("<<< engLocationsCb numOfReports =%d",
            locations.size());
        for (auto location : locations) {
            gardenPrint("<<< engLocationsCb cnt=%u time=%" PRIu64" mask=0x%x lat=%f lon=%f alt=%f"
                "info mask=0x%x eng type %d, eng mask 0x%x",
                numEngLocationCb,
                location.timestamp,
                location.flags,
                location.latitude,
                location.longitude,
                location.altitude,
                location.gnssInfoFlags,
                location.locOutputEngType,
                location.locOutputEngMask);
        }
    }

    if (mPlocationClient->getAutoMode()) {
        if (sClientReportsCbs) {
            sClientReportsCbs->locationReceivedCallback();
        }
    }
}

static void pingTestCb(uint32_t response) {
    if (sClientReportsCbs) {
        sClientReportsCbs->pingReceivedCallback();
    }
}

static void batchingCb(const std::vector<location_client::Location>& locations,
    location_client::BatchingStatus status) {
    numBatchingCb++;
    gardenPrint("<<< onBatchingCb batchingStatus=%d", status);
    if (location_client::BATCHING_STATUS_ACTIVE == status) {
        for (auto each : locations) {
            gardenPrint("<<< batchingCb cnt=%d time=%" PRIu64 " mask=0x%x lat=%f lon=%f alt=%f",
                numBatchingCb,
                each.timestamp,
                each.flags,
                each.latitude,
                each.longitude,
                each.altitude);
        }
    }
}

static void onGeofenceBreachCb(const std::vector<Geofence>& geofences,
    location_client::Location location, location_client::GeofenceBreachTypeMask type,
    uint64_t timestamp) {
    numGfBreachCb++;
    gardenPrint("<<< onGeofenceBreachCallback, cnt=%u, breachType=%d", numGfBreachCb, type);
    gardenPrint("<<< time=%" PRIu64" lat=%f lon=%f alt=%f",
        timestamp,
        location.latitude,
        location.longitude,
        location.altitude);
}

static void onCollectiveResponseCb(std::vector<pair<Geofence, LocationResponse>>& responses) {
    numGeofenceCb++;
    for (int i = 0; i<responses.size(); ++i) {
        gardenPrint("<<< onCollectiveResponseCb cnt=%u lat=%f lon=%f radius=%f, response=%u",
            numGeofenceCb,
            responses[i].first.getLatitude(),
            responses[i].first.getLongitude(),
            responses[i].first.getRadius(),
            responses[i].second);
    }
}

void GnssLocationClientAPI::setGnssCbs(GnssCbBase* callbacks) {
    std::lock_guard<std::mutex> guard(sGnssLock);
    sGnssCbs = callbacks;
}

void GnssLocationClientAPI::setFlpCbs(GnssCbBase* callbacks) {
}

void GnssLocationClientAPI::setGeofenceCbs(GnssCbBase* callbacks) {
    numGeofenceCb = 0;
    numGfBreachCb = 0;
}

GnssLocationClientAPI::GnssLocationClientAPI() :
    mStartType(GNSS_REPORT),
    mIntervalInMs(1000),
    mDistanceInMeters(0),
    mEngMask(::LOC_REQ_ENGINE_FUSED_BIT),
    mEnableOutput(true),
    mAutoMode(false),
    mRouteToNmeaPort(false) {
    mPClient = new LocationClientApi(onCapabilitiesCb);
    mPClientImpl = new LocationClientApiImpl(onCapabilitiesCb);
    mPlocationClient = this;
    memset(&mReportCbs, 0, sizeof(mReportCbs));
    mGeofences.clear();
}

GnssLocationClientAPI::~GnssLocationClientAPI() {
    if (mPClient) {
        delete mPClient;
        mPClient = nullptr;
    }
    if (mPClientImpl) {
        mPClientImpl->destroy();
    }
}

int GnssLocationClientAPI::gnssStart() {
    LOC_LOGD("%s]: ()", __func__);
    int retVal = 0;

    numGnssSvCb = 0;
    numGnssNmeaCb = 0;
    // start tracking session
    switch (mStartType) {
    case LOCATION_REPORT:
        numLocationCb = 0;
        numGnssLocationCb = 0;
        retVal = mPClient->startPositionSession(mIntervalInMs, mDistanceInMeters,
                locationCb, onResponseCb);
        break;
    case ENGINE_REPORT:
        numEngLocationCb = 0;
        retVal = mPClient->startPositionSession(mIntervalInMs,
                (location_client::LocReqEngineTypeMask)mEngMask,
                mEngineReportCbs, onResponseCb);
        break;
    case GNSS_REPORT:
    default:
        numLocationCb = 0;
        numGnssLocationCb = 0;
        retVal = mPClient->startPositionSession(mIntervalInMs, mReportCbs, onResponseCb);
        break;
    }
    return retVal;
}

int GnssLocationClientAPI::gnssStop() {
    LOC_LOGD("%s]: ()", __func__);
    int retVal = 0;
    mPClient->stopPositionSession();
    return retVal;
}

void GnssLocationClientAPI::gnssCleanup() {
}

int GnssLocationClientAPI::gnssInjectLocation(double latitude, double longitude,
        float accuracy) {
    ENTRY_LOG_CALLFLOW();
    return 0;
}

void GnssLocationClientAPI::gnssDeleteAidingData(LocGpsAidingData flags) {
    LOC_LOGD("%s]: (%02x)", __func__, flags);

    GnssAidingData aidingData;
    aidingData.deleteAll = true;
    aidingData.posEngineMask = mPosEngineMask;

    mPClientImpl->gnssDeleteAidingData(aidingData);
}

int GnssLocationClientAPI::gnssSetPositionMode(LocGpsPositionMode mode,
        LocGpsPositionRecurrence recurrence, uint32_t min_interval,
        uint32_t preferred_accuracy, uint32_t preferred_time, bool lowPowerMode) {
    LOC_LOGD("%s]: (%d %d %d %d %d)", __func__,
            mode, recurrence, min_interval, preferred_accuracy, preferred_time);

    return 0;
}

void GnssLocationClientAPI::configurationUpdate(const char* config_data, int32_t length) {
    LOC_LOGD("%s]: (%s %d)", __func__, config_data, length);
}

void GnssLocationClientAPI::updateNetworkAvailability(int available, const char* /*apn*/) {
    mPClient->updateNetworkAvailability(available);
}

void GnssLocationClientAPI::gnssNiResponse(uint32_t id, GnssNiResponse response) {
}

int GnssLocationClientAPI::gnssInjectBestLocation(Location location) {
    ENTRY_LOG_CALLFLOW();
    return 0;
}

//flp
int GnssLocationClientAPI::flpStartSession(uint32_t mode, LocationOptions& options, uint32_t id) {
    gardenPrint("%s ()]:, id: %u, mode: %d, size: %d, minInterval: %d, minDistance: %d,"
           " suplMode: %d", __func__, id, mode, options.size, options.minInterval,
            options.minDistance, options.mode);

    numBatchingCb = 0;
    return mPClient->startRoutineBatchingSession(
        options.minInterval, options.minDistance, batchingCb, onResponseCb);
}

int GnssLocationClientAPI::flpStopSession(uint32_t id) {
    mPClient->stopBatchingSession();
    return 0;
}

int GnssLocationClientAPI::flpGetBatchSize() {
    int batchSize = 0;
    gardenPrint("calling %s...", __func__);
    return 0; // TODO
}

int GnssLocationClientAPI::flpUpdateSession(uint32_t sessionMode, LocationOptions& options, uint32_t id) {
    LOC_LOGD("%s]: (%u %ld %d)", __FUNCTION__,
            id, options.minInterval*1000000L, sessionMode);
    return 0; // TODO
}

int GnssLocationClientAPI::flpGetBatchedLocation(int lastNLocations, uint32_t id) {
    LOC_LOGD("%s]: (%u %d)", __FUNCTION__, id, lastNLocations);
    return 0; // TODO
}

void GnssLocationClientAPI::flpCleanUp() {
    LOC_LOGD("%s]:", __FUNCTION__);
}

//geofence
void GnssLocationClientAPI::addGeofence(uint32_t geofenceId, double latitudeDegrees,
            double longitudeDegrees, double radiusMeters,
            GeofenceBreachTypeMask monitorTransitions,
            uint32_t notificationResponsivenessMs,
            uint32_t dwellTimeSeconds) {
    LOC_LOGD("%s]: (%d %f %f %f %d %d %d)", __FUNCTION__, geofenceId,
            latitudeDegrees, longitudeDegrees, radiusMeters, monitorTransitions,
            notificationResponsivenessMs, dwellTimeSeconds);

    Geofence gf(latitudeDegrees, longitudeDegrees,
                radiusMeters,
                (location_client::GeofenceBreachTypeMask)monitorTransitions,
                notificationResponsivenessMs, dwellTimeSeconds);
    mGeofences.push_back(gf);
    mPClient->addGeofences(mGeofences, onGeofenceBreachCb, onCollectiveResponseCb);
}

void GnssLocationClientAPI::removeGeofence(uint32_t geofenceId) {
    LOC_LOGD("%s]: (%d)", __FUNCTION__, geofenceId);
    vector<Geofence> removeGf;

    if (geofenceId >0 && geofenceId <= mGeofences.size()) {
        removeGf.push_back(mGeofences[geofenceId - 1]);
    }
    mPClient->removeGeofences(removeGf);
}

void GnssLocationClientAPI::modifyGeofence(uint32_t geofenceId,
        GeofenceBreachTypeMask monitorTransitions,
        uint32_t notificationResponsivenessMs,
        uint32_t dwellTimeSeconds) {
    LOC_LOGD("%s]: (%d  %d %d %d )", __FUNCTION__, geofenceId,
            monitorTransitions, notificationResponsivenessMs,
            dwellTimeSeconds);
    vector<Geofence> modifyGf;

    mGeofences[geofenceId].setBreachType(
            (location_client::GeofenceBreachTypeMask)monitorTransitions);
    mGeofences[geofenceId].setResponsiveness(notificationResponsivenessMs);
    mGeofences[geofenceId].setDwellTime(dwellTimeSeconds);
    modifyGf.push_back(mGeofences[geofenceId - 1]);

    mPClient->modifyGeofences(modifyGf);
}

void GnssLocationClientAPI::pauseGeofence(uint32_t geofenceId) {
    LOC_LOGD("%s]: (%d)", __func__, geofenceId);
    vector<Geofence> pauseGf;

    if (geofenceId > 0 && geofenceId <= mGeofences.size()) {
        pauseGf.push_back(mGeofences[geofenceId - 1]);
    }

    mPClient->pauseGeofences(pauseGf);
}

void GnssLocationClientAPI::resumeGeofence(uint32_t geofenceId,
        GeofenceBreachTypeMask monitorTransitions) {
    LOC_LOGD("%s]: (%d %d)", __func__, geofenceId, monitorTransitions);
    vector<Geofence> resumeGf;

    if (geofenceId > 0 && geofenceId <= mGeofences.size()) {
        resumeGf.push_back(mGeofences[geofenceId - 1]);
    }

    mPClient->resumeGeofences(resumeGf);
}

void GnssLocationClientAPI::removeAllGeofences() {
    LOC_LOGD("%s]", __FUNCTION__);
}

void GnssLocationClientAPI::gnssStartOptions(GnssStartType startType,
        uint32_t intervalInMs, uint32_t distanceInMeters,
        ::LocReqEngineTypeMask engMask, int gnssReportOptions) {

    mStartType = startType;
    mIntervalInMs = intervalInMs;
    mDistanceInMeters = distanceInMeters;
    mEngMask = engMask;

    if (GNSS_REPORT == mStartType) {
        memset(&mReportCbs, 0, sizeof(mReportCbs));
        if (gnssReportOptions & LOCATION_BIT) {
            mReportCbs.gnssLocationCallback = GnssLocationCb(gnssLocationCb);
        }
        if (gnssReportOptions & GNSS_SV_BIT) {
            mReportCbs.gnssSvCallback = GnssSvCb(gnssSvCb);
        }
        if (gnssReportOptions & GNSS_NMEA_BIT) {
            mReportCbs.gnssNmeaCallback = GnssNmeaCb(gnssNmeaCb);
        }
        if (gnssReportOptions & GNSS_DATA_BIT) {
            numDataCb = 0;
            mReportCbs.gnssDataCallback = GnssDataCb(gnssDataCb);
        }
        if (gnssReportOptions & GNSS_MEAS_BIT) {
            numMeasurementsCb = 0;
            mReportCbs.gnssMeasurementsCallback = GnssMeasurementsCb(gnssMeasurementsCb);
        }
    } else if (ENGINE_REPORT == mStartType) {
        memset(&mEngineReportCbs, 0, sizeof(mEngineReportCbs));
        if (gnssReportOptions & LOCATION_BIT) {
            mEngineReportCbs.engLocationsCallback = EngineLocationsCb(engLocationsCb);
        }
        if (gnssReportOptions & GNSS_SV_BIT) {
            mEngineReportCbs.gnssSvCallback = GnssSvCb(gnssSvCb);
        }
        if (gnssReportOptions & GNSS_NMEA_BIT) {
            mEngineReportCbs.gnssNmeaCallback = GnssNmeaCb(gnssNmeaCb);
        }
        if (gnssReportOptions & GNSS_DATA_BIT) {
            numDataCb = 0;
            mEngineReportCbs.gnssDataCallback = GnssDataCb(gnssDataCb);
        }
        if (gnssReportOptions & GNSS_MEAS_BIT) {
            numMeasurementsCb = 0;
            mEngineReportCbs.gnssMeasurementsCallback = GnssMeasurementsCb(gnssMeasurementsCb);
        }
        if (gnssReportOptions & GNSS_POLY_BIT) {
            gardenPrint("Set SV poly cb");
            numPolyCb = 0;
            mEngineReportCbs.gnssSvPolyCallback = GnssSvPolyCb(gnssSvPolyCb);
        }
    }
}

uint32_t GnssLocationClientAPI::gnssDeleteAidingData(GnssAidingData& data) {
    mPClientImpl->gnssDeleteAidingData(data);
    return 0;
}

void GnssLocationClientAPI::testGetEnergy() {
    mPClient->getGnssEnergyConsumed(gnssEnergyConsumedInfoCb,
            gnssEnergyConsumedResponseCb);
}

void GnssLocationClientAPI::testRegSystemInfo() {
    mPClient->updateLocationSystemInfoListener(locationSystemInfoCb,
            locationSystemInfoResponseCb);
}

void GnssLocationClientAPI::testUnregSystemInfo() {
    mPClient->updateLocationSystemInfoListener(nullptr,
            nullptr);
}

void GnssLocationClientAPI::pingTest() {
    mPClientImpl->pingTest(pingTestCb);
}

void GnssLocationClientAPI::setClientReportsCbs(ClientReportsCbs* callbacks) {
    sClientReportsCbs = callbacks;
}

} // namespace garden
