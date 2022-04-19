/*=============================================================================
  Copyright (c) 2016-2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  =============================================================================*/

#include <izat_remote_api.h>
#include <IzatRemoteApi.h>
#include <IzatTypes.h>
#include <string>

#ifdef USE_GLIB
#include <LocationClientApi.h>
#endif

using namespace izat_remote_api;
using namespace izat_manager;


#ifdef USE_GLIB
static void parseGnssTime(
        const location_client::GnssSystemTimeStructType clientGnssTime,
        GnssSystemTimeStructType & halGnssTime) {

    if (location_client::GNSS_SYSTEM_TIME_WEEK_VALID & clientGnssTime.validityMask) {
        halGnssTime.validityMask |= GNSS_SYSTEM_TIME_WEEK_VALID;
        halGnssTime.systemWeek = clientGnssTime.systemWeek;
    }
    if (location_client::GNSS_SYSTEM_TIME_WEEK_MS_VALID & clientGnssTime.validityMask) {
        halGnssTime.validityMask |= GNSS_SYSTEM_TIME_WEEK_MS_VALID;
        halGnssTime.systemMsec = clientGnssTime.systemMsec;
    }
    if (location_client::GNSS_SYSTEM_CLK_TIME_BIAS_VALID & clientGnssTime.validityMask) {
        halGnssTime.validityMask |= GNSS_SYSTEM_CLK_TIME_BIAS_VALID;
        halGnssTime.systemClkTimeBias = clientGnssTime.systemClkTimeBias;
    }
    if (location_client::GNSS_SYSTEM_CLK_TIME_BIAS_UNC_VALID & clientGnssTime.validityMask) {
        halGnssTime.validityMask |= GNSS_SYSTEM_CLK_TIME_BIAS_UNC_VALID;
        halGnssTime.systemClkTimeUncMs = clientGnssTime.systemClkTimeUncMs;
    }
    if (location_client::GNSS_SYSTEM_REF_FCOUNT_VALID & clientGnssTime.validityMask) {
        halGnssTime.validityMask |= GNSS_SYSTEM_REF_FCOUNT_VALID;
        halGnssTime.refFCount = clientGnssTime.refFCount;
    }
    if (location_client::GNSS_SYSTEM_NUM_CLOCK_RESETS_VALID & clientGnssTime.validityMask) {
        halGnssTime.validityMask |= GNSS_SYSTEM_NUM_CLOCK_RESETS_VALID;
        halGnssTime.numClockResets = clientGnssTime.numClockResets;
    }

}

static void parseGloTime(
        const location_client::GnssGloTimeStructType clientGloTime,
        GnssGloTimeStructType & halGloTime) {

    if (location_client::GNSS_CLO_DAYS_VALID & clientGloTime.validityMask) {
        halGloTime.validityMask |= GNSS_CLO_DAYS_VALID;
        halGloTime.gloDays = clientGloTime.gloDays;
    }
    if (location_client::GNSS_GLO_MSEC_VALID  & clientGloTime.validityMask) {
        halGloTime.validityMask |= GNSS_GLO_MSEC_VALID ;
        halGloTime.gloMsec = clientGloTime.gloMsec;
    }
    if (location_client::GNSS_GLO_CLK_TIME_BIAS_VALID & clientGloTime.validityMask) {
        halGloTime.validityMask |= GNSS_GLO_CLK_TIME_BIAS_VALID;
        halGloTime.gloClkTimeBias = clientGloTime.gloClkTimeBias;
    }
    if (location_client::GNSS_GLO_CLK_TIME_BIAS_UNC_VALID & clientGloTime.validityMask) {
        halGloTime.validityMask |= GNSS_GLO_CLK_TIME_BIAS_UNC_VALID;
        halGloTime.gloClkTimeUncMs = clientGloTime.gloClkTimeUncMs;
    }
    if (location_client::GNSS_GLO_REF_FCOUNT_VALID & clientGloTime.validityMask) {
        halGloTime.validityMask |= GNSS_GLO_REF_FCOUNT_VALID;
        halGloTime.refFCount = clientGloTime.refFCount;
    }
    if (location_client::GNSS_GLO_NUM_CLOCK_RESETS_VALID & clientGloTime.validityMask) {
        halGloTime.validityMask |= GNSS_GLO_NUM_CLOCK_RESETS_VALID;
        halGloTime.numClockResets = clientGloTime.numClockResets;
    }
    if (location_client::GNSS_GLO_FOUR_YEAR_VALID & clientGloTime.validityMask) {
        halGloTime.validityMask |= GNSS_GLO_FOUR_YEAR_VALID;
        halGloTime.gloFourYear = clientGloTime.gloFourYear;
    }
}
#endif


// ======== LocationUpdater ========
class LocationUpdaterWrapper {
    const remoteClientInfo *mClientInfo;
    const void* mClientData;
#ifdef USE_GLIB
    location_client::LocationClientApi *mLocationClientApiObj;
#endif

public:
    inline LocationUpdaterWrapper(remoteClientInfo *pClientInfo, void* clientData) :
                                  mClientInfo(pClientInfo), mClientData(clientData) {

#ifdef USE_GLIB
        mLocationClientApiObj = new location_client::LocationClientApi(
                [] (location_client::LocationCapabilitiesMask capsMask) { });

        location_client::GnssReportCbs gnssRepCbs;

        if (pClientInfo->locCb) {
            location_client::GnssLocationCb locationCb =
                    [this] (const location_client::GnssLocation& gnssLocation) {
                UlpLocation ulpLocation;
                GpsLocationExtended locationExtended;

                memset(&ulpLocation, 0, sizeof(UlpLocation));
                memset(&locationExtended, 0, sizeof(GpsLocationExtended));
                ulpLocation.size = sizeof(UlpLocation);
                ulpLocation.gpsLocation.size = sizeof(LocGpsLocation);
                locationExtended.size = sizeof (GpsLocationExtended);

                if (location_client::LOCATION_HAS_LAT_LONG_BIT & gnssLocation.flags) {
                    ulpLocation.gpsLocation.flags |= LOC_GPS_LOCATION_HAS_LAT_LONG;
                    ulpLocation.gpsLocation.latitude = gnssLocation.latitude;
                    ulpLocation.gpsLocation.longitude = gnssLocation.longitude;
                }
                if (location_client::LOCATION_HAS_ALTITUDE_BIT & gnssLocation.flags) {
                    ulpLocation.gpsLocation.flags |= LOC_GPS_LOCATION_HAS_ALTITUDE;
                    ulpLocation.gpsLocation.altitude = gnssLocation.altitude;
                }
                if (location_client::LOCATION_HAS_SPEED_BIT & gnssLocation.flags) {
                    ulpLocation.gpsLocation.flags |= LOC_GPS_LOCATION_HAS_SPEED;
                    ulpLocation.gpsLocation.speed = gnssLocation.speed ;
                }
                if (location_client::LOCATION_HAS_BEARING_BIT & gnssLocation.flags) {
                    ulpLocation.gpsLocation.flags |= LOC_GPS_LOCATION_HAS_BEARING;
                     ulpLocation.gpsLocation.bearing = gnssLocation.bearing;
                }
                if (location_client::LOCATION_HAS_ACCURACY_BIT & gnssLocation.flags) {
                    ulpLocation.gpsLocation.flags |= LOC_GPS_LOCATION_HAS_ACCURACY;
                    ulpLocation.gpsLocation.accuracy = gnssLocation.horizontalAccuracy;
                }
                ulpLocation.gpsLocation.timestamp = gnssLocation.timestamp;

                if (location_client::LOCATION_TECHNOLOGY_GNSS_BIT & gnssLocation.techMask) {
                    ulpLocation.tech_mask |= LOC_POS_TECH_MASK_SATELLITE;
                }
                if (location_client::LOCATION_TECHNOLOGY_CELL_BIT & gnssLocation.techMask) {
                    ulpLocation.tech_mask |= LOC_POS_TECH_MASK_CELLID;
                }
                if (location_client::LOCATION_TECHNOLOGY_WIFI_BIT & gnssLocation.techMask) {
                    ulpLocation.tech_mask |= LOC_POS_TECH_MASK_WIFI;
                }
                if (location_client::LOCATION_TECHNOLOGY_SENSORS_BIT & gnssLocation.techMask) {
                    ulpLocation.tech_mask |= LOC_POS_TECH_MASK_SENSORS;
                }

                if (location_client::GNSS_LOCATION_INFO_ALTITUDE_MEAN_SEA_LEVEL_BIT &
                        gnssLocation.gnssInfoFlags) {
                    locationExtended.flags |= GPS_LOCATION_EXTENDED_HAS_ALTITUDE_MEAN_SEA_LEVEL;
                    locationExtended.altitudeMeanSeaLevel = gnssLocation.altitudeMeanSeaLevel;
                }

                if (location_client::GNSS_LOCATION_INFO_DOP_BIT & gnssLocation.gnssInfoFlags) {
                    locationExtended.flags |= GPS_LOCATION_EXTENDED_HAS_DOP;
                    locationExtended.pdop = gnssLocation.pdop;
                    locationExtended.hdop = gnssLocation.hdop;
                    locationExtended.vdop = gnssLocation.vdop;
                }

                if (location_client::GNSS_LOCATION_INFO_EXT_DOP_BIT & gnssLocation.gnssInfoFlags) {
                    locationExtended.flags |= GPS_LOCATION_EXTENDED_HAS_EXT_DOP;
                    locationExtended.extDOP.GDOP = gnssLocation.gdop;
                    locationExtended.extDOP.TDOP = gnssLocation.tdop;
                }

                if (location_client::GNSS_LOCATION_INFO_MAGNETIC_DEVIATION_BIT &
                        gnssLocation.gnssInfoFlags) {
                    locationExtended.flags |= GPS_LOCATION_EXTENDED_HAS_MAG_DEV;
                    locationExtended.magneticDeviation = gnssLocation.magneticDeviation;
                }

                if (location_client::LOCATION_HAS_VERTICAL_ACCURACY_BIT & gnssLocation.flags) {
                    locationExtended.flags |= GPS_LOCATION_EXTENDED_HAS_VERT_UNC;
                    locationExtended.vert_unc = gnssLocation.verticalAccuracy;
                }
                if (location_client::LOCATION_HAS_SPEED_ACCURACY_BIT & gnssLocation.flags) {
                    locationExtended.flags |= GPS_LOCATION_EXTENDED_HAS_SPEED_UNC;
                    locationExtended.speed_unc = gnssLocation.speedAccuracy;
                }
                if (location_client::LOCATION_HAS_BEARING_ACCURACY_BIT & gnssLocation.flags) {
                    locationExtended.flags |= GPS_LOCATION_EXTENDED_HAS_BEARING_UNC;
                    locationExtended.bearing_unc = gnssLocation.bearingAccuracy;
                }

                if (location_client::GNSS_LOCATION_INFO_HOR_RELIABILITY_BIT &
                        gnssLocation.gnssInfoFlags) {
                    locationExtended.flags |= GPS_LOCATION_EXTENDED_HAS_HOR_RELIABILITY;
                    switch (gnssLocation.horReliability) {
                        case location_client::LOCATION_RELIABILITY_VERY_LOW:
                        locationExtended.horizontal_reliability= LOC_RELIABILITY_VERY_LOW;
                        break;
                    case location_client::LOCATION_RELIABILITY_LOW:
                        locationExtended.horizontal_reliability = LOC_RELIABILITY_LOW;
                        break;
                    case location_client::LOCATION_RELIABILITY_MEDIUM:
                        locationExtended.horizontal_reliability = LOC_RELIABILITY_MEDIUM;
                        break;
                    case location_client::LOCATION_RELIABILITY_HIGH:
                        locationExtended.horizontal_reliability = LOC_RELIABILITY_HIGH;
                        break;
                    default:
                        locationExtended.horizontal_reliability = LOC_RELIABILITY_NOT_SET;
                        break;
                    }
                }

                if (location_client::GNSS_LOCATION_INFO_VER_RELIABILITY_BIT &
                        gnssLocation.gnssInfoFlags) {
                    locationExtended.flags |= GPS_LOCATION_EXTENDED_HAS_VERT_RELIABILITY;
                    switch (gnssLocation.verReliability) {
                        case location_client::LOCATION_RELIABILITY_VERY_LOW:
                        locationExtended.vertical_reliability= LOC_RELIABILITY_VERY_LOW;
                        break;
                    case location_client::LOCATION_RELIABILITY_LOW:
                        locationExtended.vertical_reliability = LOC_RELIABILITY_LOW;
                        break;
                    case location_client::LOCATION_RELIABILITY_MEDIUM:
                        locationExtended.vertical_reliability = LOC_RELIABILITY_MEDIUM;
                        break;
                    case location_client::LOCATION_RELIABILITY_HIGH:
                        locationExtended.vertical_reliability = LOC_RELIABILITY_HIGH;
                        break;
                    default:
                        locationExtended.vertical_reliability = LOC_RELIABILITY_NOT_SET;
                        break;
                    }
                }


                if (location_client::GNSS_LOCATION_INFO_HOR_ACCURACY_ELIP_SEMI_MAJOR_BIT &
                        gnssLocation.gnssInfoFlags) {
                    locationExtended.flags |= GPS_LOCATION_EXTENDED_HAS_HOR_ELIP_UNC_MAJOR;
                    locationExtended.horUncEllipseSemiMajor = gnssLocation.horUncEllipseSemiMajor;
                }

                if (location_client::GNSS_LOCATION_INFO_HOR_ACCURACY_ELIP_SEMI_MINOR_BIT &
                        gnssLocation.gnssInfoFlags) {
                    locationExtended.flags |= GPS_LOCATION_EXTENDED_HAS_HOR_ELIP_UNC_MINOR;
                    locationExtended.horUncEllipseSemiMinor = gnssLocation.horUncEllipseSemiMinor;
                }

                if (location_client::GNSS_LOCATION_INFO_HOR_ACCURACY_ELIP_AZIMUTH_BIT &
                        gnssLocation.gnssInfoFlags) {
                    locationExtended.flags |= GPS_LOCATION_EXTENDED_HAS_HOR_ELIP_UNC_AZIMUTH;
                    locationExtended.horUncEllipseOrientAzimuth =
                            gnssLocation.horUncEllipseOrientAzimuth;
                }

                if (location_client::GNSS_LOCATION_INFO_GNSS_SV_USED_DATA_BIT &
                      gnssLocation.gnssInfoFlags) {
                  locationExtended.flags |= GPS_LOCATION_EXTENDED_HAS_GNSS_SV_USED_DATA;
                  locationExtended.gnss_sv_used_ids.gps_sv_used_ids_mask =
                          gnssLocation.svUsedInPosition.gpsSvUsedIdsMask;
                  locationExtended.gnss_sv_used_ids.glo_sv_used_ids_mask =
                          gnssLocation.svUsedInPosition.gloSvUsedIdsMask;
                  locationExtended.gnss_sv_used_ids.gal_sv_used_ids_mask =
                          gnssLocation.svUsedInPosition.galSvUsedIdsMask;
                  locationExtended.gnss_sv_used_ids.bds_sv_used_ids_mask =
                          gnssLocation.svUsedInPosition.bdsSvUsedIdsMask;
                  locationExtended.gnss_sv_used_ids.qzss_sv_used_ids_mask =
                          gnssLocation.svUsedInPosition.qzssSvUsedIdsMask;

                  locationExtended.numOfMeasReceived =
                          (gnssLocation.measUsageInfo.size() > GNSS_SV_MAX ?
                          GNSS_SV_MAX: gnssLocation.measUsageInfo.size());
                  int svIdx = 0;
                  for (const auto & measUsage : gnssLocation.measUsageInfo) {
                      locationExtended.measUsageInfo[svIdx].gnssSignalType =
                              measUsage.gnssSignalType;
                      locationExtended.measUsageInfo[svIdx].gnssSvId =
                              measUsage.gnssSvId;
                      locationExtended.measUsageInfo[svIdx].gnssConstellation =
                              (Gnss_LocSvSystemEnumType) measUsage.gnssConstellation;
                      svIdx++;

                      if (svIdx >= GNSS_SV_MAX) {
                        break;
                      }
                  }
                }

                if (location_client::GNSS_LOCATION_INFO_NAV_SOLUTION_MASK_BIT &
                        gnssLocation.gnssInfoFlags) {
                    locationExtended.flags |= GPS_LOCATION_EXTENDED_HAS_NAV_SOLUTION_MASK;
                    locationExtended.navSolutionMask = gnssLocation.navSolutionMask;
                 }

                if (location_client::GNSS_LOCATION_INFO_POS_TECH_MASK_BIT &
                        gnssLocation.gnssInfoFlags) {
                    locationExtended.flags |= GPS_LOCATION_EXTENDED_HAS_POS_TECH_MASK;
                    locationExtended.tech_mask = gnssLocation.posTechMask;
                }

                if (location_client::GNSS_LOCATION_INFO_POS_DYNAMICS_DATA_BIT &
                        gnssLocation.gnssInfoFlags) {
                    locationExtended.flags |= GPS_LOCATION_EXTENDED_HAS_POS_DYNAMICS_DATA;
                    locationExtended.bodyFrameData.bodyFrameDataMask =
                            gnssLocation.bodyFrameData.bodyFrameDataMask;

                    locationExtended.bodyFrameData.longAccel = gnssLocation.bodyFrameData.longAccel;
                    locationExtended.bodyFrameData.latAccel = gnssLocation.bodyFrameData.latAccel;
                    locationExtended.bodyFrameData.vertAccel = gnssLocation.bodyFrameData.vertAccel;
                    locationExtended.bodyFrameData.yawRate = gnssLocation.bodyFrameData.yawRate;
                    locationExtended.bodyFrameData.pitch = gnssLocation.bodyFrameData.pitch;
                }

                switch (gnssLocation.gnssSystemTime.gnssSystemTimeSrc) {
                    case GNSS_LOC_SV_SYSTEM_GPS:
                        locationExtended.gnssSystemTime.gnssSystemTimeSrc = GNSS_LOC_SV_SYSTEM_GPS;
                        parseGnssTime(gnssLocation.gnssSystemTime.u.gpsSystemTime,
                                locationExtended.gnssSystemTime.u.gpsSystemTime);

                        if ((gnssLocation.gnssSystemTime.u.gpsSystemTime.validityMask &
                                GNSS_SYSTEM_TIME_WEEK_VALID) &&
                            (gnssLocation.gnssSystemTime.u.gpsSystemTime.validityMask &
                                GNSS_SYSTEM_TIME_WEEK_MS_VALID)) {
                            locationExtended.gpsTime.gpsWeek =
                                    gnssLocation.gnssSystemTime.u.gpsSystemTime.systemWeek;
                            locationExtended.gpsTime.gpsTimeOfWeekMs =
                                    gnssLocation.gnssSystemTime.u.gpsSystemTime.systemMsec;
                            locationExtended.flags |= GPS_LOCATION_EXTENDED_HAS_GPS_TIME;
                        }
                        break;
                    case GNSS_LOC_SV_SYSTEM_GALILEO:
                        locationExtended.gnssSystemTime.gnssSystemTimeSrc =
                                GNSS_LOC_SV_SYSTEM_GALILEO;
                        parseGnssTime(gnssLocation.gnssSystemTime.u.galSystemTime,
                                locationExtended.gnssSystemTime.u.galSystemTime);
                        break;
                    case GNSS_LOC_SV_SYSTEM_SBAS:
                        locationExtended.gnssSystemTime.gnssSystemTimeSrc = GNSS_LOC_SV_SYSTEM_SBAS;
                        break;
                    case GNSS_LOC_SV_SYSTEM_GLONASS:
                        locationExtended.gnssSystemTime.gnssSystemTimeSrc =
                                GNSS_LOC_SV_SYSTEM_GLONASS;
                        parseGloTime(gnssLocation.gnssSystemTime.u.gloSystemTime,
                                locationExtended.gnssSystemTime.u.gloSystemTime);
                        break;
                    case GNSS_LOC_SV_SYSTEM_BDS:
                        locationExtended.gnssSystemTime.gnssSystemTimeSrc = GNSS_LOC_SV_SYSTEM_BDS;
                        parseGnssTime(gnssLocation.gnssSystemTime.u.bdsSystemTime,
                                locationExtended.gnssSystemTime.u.bdsSystemTime);
                        break;
                }

                mClientInfo->locCb(&ulpLocation, &locationExtended, (void*)mClientData);
            };

            gnssRepCbs.gnssLocationCallback = locationCb;
        }

        if (pClientInfo->svReportCb) {
            location_client::GnssSvCb svCb =
                    [this] (const std::vector<location_client::GnssSv>& gnssSvs) {
                GnssSvNotification halSvNotification;
                memset(&halSvNotification, 0, sizeof(GnssSvNotification));
                halSvNotification.size = sizeof(GnssSvNotification);

                halSvNotification.count = (gnssSvs.size() > GNSS_SV_MAX ?
                        GNSS_SV_MAX :gnssSvs.size());

                int svIdx = 0;
                for (auto svInfo : gnssSvs) {
                    halSvNotification.gnssSvs[svIdx].size = sizeof(GnssSv);
                    halSvNotification.gnssSvs[svIdx].svId = svInfo.svId;

                    switch(svInfo.type) {
                        case location_client::GNSS_SV_TYPE_GPS:
                            halSvNotification.gnssSvs[svIdx].type = GNSS_SV_TYPE_GPS;
                        break;
                        case location_client::GNSS_SV_TYPE_SBAS:
                            halSvNotification.gnssSvs[svIdx].type = GNSS_SV_TYPE_SBAS;
                        break;
                        case location_client::GNSS_SV_TYPE_GLONASS:
                            halSvNotification.gnssSvs[svIdx].type = GNSS_SV_TYPE_GLONASS;
                        break;
                        case location_client::GNSS_SV_TYPE_QZSS:
                            halSvNotification.gnssSvs[svIdx].type = GNSS_SV_TYPE_QZSS;
                        break;
                        case location_client::GNSS_SV_TYPE_BEIDOU:
                            halSvNotification.gnssSvs[svIdx].type = GNSS_SV_TYPE_BEIDOU;
                        break;
                        case location_client::GNSS_SV_TYPE_GALILEO:
                            halSvNotification.gnssSvs[svIdx].type = GNSS_SV_TYPE_GALILEO;
                        break;
                        default:
                            halSvNotification.gnssSvs[svIdx].type = GNSS_SV_TYPE_UNKNOWN;
                        break;
                    }

                    halSvNotification.gnssSvs[svIdx].cN0Dbhz = svInfo.cN0Dbhz;
                    halSvNotification.gnssSvs[svIdx].elevation = svInfo.elevation;
                    halSvNotification.gnssSvs[svIdx].azimuth = svInfo.azimuth;

                    if (svInfo.gnssSvOptionsMask &
                            location_client::GNSS_SV_OPTIONS_HAS_EPHEMER_BIT) {
                        halSvNotification.gnssSvs[svIdx].gnssSvOptionsMask |=
                                GNSS_SV_OPTIONS_HAS_EPHEMER_BIT;
                    }
                    if (svInfo.gnssSvOptionsMask &
                            location_client::GNSS_SV_OPTIONS_HAS_ALMANAC_BIT) {
                        halSvNotification.gnssSvs[svIdx].gnssSvOptionsMask |=
                                GNSS_SV_OPTIONS_HAS_ALMANAC_BIT;
                    }
                    if (svInfo.gnssSvOptionsMask &
                            location_client::GNSS_SV_OPTIONS_USED_IN_FIX_BIT) {
                        halSvNotification.gnssSvs[svIdx].gnssSvOptionsMask |=
                                GNSS_SV_OPTIONS_USED_IN_FIX_BIT;
                    }

                    svIdx++;
                    if (svIdx >= GNSS_SV_MAX) {
                        break;
                    }
                }

                mClientInfo->svReportCb(&halSvNotification, (void *)mClientData);
            };

            gnssRepCbs.gnssSvCallback = svCb;
        }

        if (pClientInfo->nmeaCb) {
            location_client::GnssNmeaCb nmeaCb = [this]
                    (uint64_t timestamp, const std::string& nmea) {
                UlpNmea halNmea;
                memset(&halNmea, 0, sizeof(UlpNmea));
                halNmea.size = sizeof(UlpNmea);

                halNmea.len = (nmea.length() > ULP_MAX_NMEA_STRING_SIZE ?
                        ULP_MAX_NMEA_STRING_SIZE : nmea.length());
                memcpy(&halNmea.nmea_str, nmea.c_str(), halNmea.len);
                mClientInfo->nmeaCb(&halNmea, (void *)mClientData);
            };

            gnssRepCbs.gnssNmeaCallback = nmeaCb;
        }

        location_client::ResponseCb respCb = [this] (location_client::LocationResponse response) {
        };

        mLocationClientApiObj->startPositionSession(0, gnssRepCbs, respCb);
#endif

    }

    ~LocationUpdaterWrapper() {
#ifdef USE_GLIB
        mLocationClientApiObj->stopPositionSession();

        delete mLocationClientApiObj;
        mLocationClientApiObj = NULL;
#endif
    }
};

void* registerLocationUpdater(remoteClientInfo *pClientInfo, void* clientData) {
    return (pClientInfo && clientData) ? new LocationUpdaterWrapper(pClientInfo, clientData) : NULL;
}

void unregisterLocationUpdater(void* locationUpdaterHandle) {
    if (locationUpdaterHandle) {
        delete (LocationUpdaterWrapper*)locationUpdaterHandle;
    }
}

// ======== SstpUpdater ========
class SstpUpdaterWrapper : public SstpUpdater {
    const sstpSiteUpdateCb mSiteCb;
    const sstpMccUpdateCb mMccCb;
    const errReportCb mErrCb;
    const void* mClientData;
public:
    inline SstpUpdaterWrapper(sstpSiteUpdateCb siteCb, sstpMccUpdateCb mccCb,
                              errReportCb errCb, void* clientData) :
        SstpUpdater(), mSiteCb(siteCb), mMccCb(mccCb), mErrCb(errCb),
        mClientData(clientData) {
    }
    inline virtual void errReport(const char* errStr) override {
        if (mErrCb != nullptr) mErrCb(errStr, (void*)mClientData);
    }
    inline virtual void siteUpdate(const char* name, double lat, double lon,
                                   float unc, int32_t uncConfidence) override {
        mSiteCb(name, lat, lon, unc, uncConfidence, (void*)mClientData);
    }
    inline virtual void mccUpdate(uint32_t mcc, const char* confidence) override {
        mMccCb(mcc, confidence, (void*)mClientData);
    }
};

void* registerSstpUpdater(sstpSiteUpdateCb siteCb, sstpMccUpdateCb mccCb,
                          errReportCb errCb, void* clientData) {
    return (siteCb && mccCb) ?
        new SstpUpdaterWrapper(siteCb, mccCb, errCb, clientData) :
        NULL;
}

void unregisterSstpUpdater(void* sstpUpdaterHandle) {
    if (sstpUpdaterHandle) {
        delete (SstpUpdaterWrapper*)sstpUpdaterHandle;
    }
}

void stopSstpUpdate(void* sstpUpdaterHandle) {
    if (sstpUpdaterHandle) {
        ((SstpUpdaterWrapper*)sstpUpdaterHandle)->stop();
    }
}
