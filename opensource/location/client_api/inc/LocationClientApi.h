/* Copyright (c) 2018-2020, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef LOCATIONCLIENTAPI_H
#define LOCATIONCLIENTAPI_H

#include <vector>
#include <string>
#include <functional>
#include <memory>

namespace location_client
{
class Geofence;

enum LocationCapabilitiesMask {
    /** LocationClientApi can support time-based tracking session
     *  via LocationClientApi::startPositionSession(uint32_t,
     *  LocReqEngineTypeMask, const EngineReportCbs&, ResponseCb)
     *  and LocationClientApi::startPositionSession(uint32_t, const
     *  GnssReportCbs&, ResponseCb) and
     *  LocationClientApi::startPositionSession(uint32_t, uint32_t,
     *  LocationCb, ResponseCb) with distanceInMeters set to 0.
     *  <br/> */
    LOCATION_CAPS_TIME_BASED_TRACKING_BIT           = (1<<0),
    /** LocationClientApi can support time-based batching session
     *  via LocationClientApi::startRoutineBatchingSession() with
     *  minInterval specified. <br/> */
    LOCATION_CAPS_TIME_BASED_BATCHING_BIT           = (1<<1),
    /** LocationClientApi can support distance based tracking
     *  session via
     *  LocationClientApi::startPositionSession(uint32_t, uint32_t,
     *  LocationCb, ResponseCb) with distanceInMeters specified.
     *  <br/> */
    LOCATION_CAPS_DISTANCE_BASED_TRACKING_BIT       = (1<<2),
    /** LocationClientApi can support distance-based batching via
     *  LocationClientApi::startRoutineBatchingSession() with
     *  minDistance set to none-zero value. <br/> */
    LOCATION_CAPS_DISTANCE_BASED_BATCHING_BIT       = (1<<3),
    /** LocationClientApi can support geo fence via
     *  LocationClientApi::addGeofences(). <br/>   */
    LOCATION_CAPS_GEOFENCE_BIT                      = (1<<4),
    /** LocationClientApi can support trip batching via
     *  LocationClientApi::startTripBatchingSession(). <br/>   */
    LOCATION_CAPS_OUTDOOR_TRIP_BATCHING_BIT         = (1<<5),
};

/**
 *  Specify the vaid fields and additional status for
 *  GnssSv. <br/>
 */
enum GnssSvOptionsMask {
    /** Ephemeris is available for this SV. <br/>   */
    GNSS_SV_OPTIONS_HAS_EPHEMER_BIT             = (1<<0),
    /** Almanac is available for this SV. <br/>   */
    GNSS_SV_OPTIONS_HAS_ALMANAC_BIT             = (1<<1),
    /** This SV is used in the position fix. <br/>   */
    GNSS_SV_OPTIONS_USED_IN_FIX_BIT             = (1<<2),
    /** This SV has valid GnssSv::carrierFrequencyHz. <br/> */
    GNSS_SV_OPTIONS_HAS_CARRIER_FREQUENCY_BIT   = (1<<3),
    /** This SV has valid GnssSv::gnssSignalTypeMask. <br/>   */
    GNSS_SV_OPTIONS_HAS_GNSS_SIGNAL_TYPE_BIT    = (1<<4)
};

/**
 *  Specify the valid fields in Location. <br/>
 *
 *  User should determine whether a field in
 *  Location is valid or not by checking
 *  whether the corrsponding bit in
 *  Location::flags is set or not. <br/>
 */
enum LocationFlagsMask {
    /** Location has valid Location::latitude and
     *  Location::longitude. <br/>   */
    LOCATION_HAS_LAT_LONG_BIT          = (1<<0),
    /** Location has valid Location::altitude. <br/>   */
    LOCATION_HAS_ALTITUDE_BIT          = (1<<1),
    /** Location has valid Location::speed. <br/>   */
    LOCATION_HAS_SPEED_BIT             = (1<<2),
    /** Location has valid Location::bearing. <br/>   */
    LOCATION_HAS_BEARING_BIT           = (1<<3),
    /** Location has valid Location::horizontalAccuracy. <br/>   */
    LOCATION_HAS_ACCURACY_BIT          = (1<<4),
    /** Location has valid Location::verticalAccuracy. <br/>   */
    LOCATION_HAS_VERTICAL_ACCURACY_BIT = (1<<5),
    /** Location has valid Location::speedAccuracy. <br/>   */
    LOCATION_HAS_SPEED_ACCURACY_BIT    = (1<<6),
    /** Location has valid Location::bearingAccuracy. <br/>   */
    LOCATION_HAS_BEARING_ACCURACY_BIT  = (1<<7),
    /** Location has valid Location::timestamp. <br/>   */
    LOCATION_HAS_TIMESTAMP_BIT         = (1<<8)
};

/**
 *  Specify the set of technologies that contribute to Location.
 *  <br/>
 */
enum LocationTechnologyMask {
    /** GNSS-based technology was used to calculate
     *  Location. <br/>   */
    LOCATION_TECHNOLOGY_GNSS_BIT     = (1<<0),
    /** Cell-based technology was used to calculate
     *  Location. <br/>   */
    LOCATION_TECHNOLOGY_CELL_BIT     = (1<<1),
    /** WiFi-based technology was used to calculate
     *  Location. <br/>   */
    LOCATION_TECHNOLOGY_WIFI_BIT     = (1<<2),
    /** Sensor-based technology was used to calculate
     *  Location. <br/>   */
    LOCATION_TECHNOLOGY_SENSORS_BIT  = (1<<3)
};

/** Specify the set of navigation solutions that contribute
 *  to GnssLocation. <br/> */
enum GnssLocationNavSolutionMask {
    /** SBAS ionospheric correction was used to calculate
     *  GnssLocation. <br/>   */
    LOCATION_SBAS_CORRECTION_IONO_BIT  = (1<<0),
    /** SBAS fast correction was used to calculate
     *  GnssLocation. <br/>   */
    LOCATION_SBAS_CORRECTION_FAST_BIT  = (1<<1),
    /** SBAS long-tem correction was used to calculate
     *  GnssLocation. <br/>   */
    LOCATION_SBAS_CORRECTION_LONG_BIT  = (1<<2),
    /** SBAS integrity information was used to calculate
     *  GnssLocation. <br/>   */
    LOCATION_SBAS_INTEGRITY_BIT        = (1<<3),
    /** DGNSS corretion was used to calculate
     *  GnssLocation. <br/>   */
    LOCATION_NAV_CORRECTION_DGNSS_BIT  = (1<<4),
    /** RTK corretion was used to calculate
     *  GnssLocation. <br/>   */
    LOCATION_NAV_CORRECTION_RTK_BIT    = (1<<5),
    /** PPP correction was used to calculate
     *  GnssLocation. <br/>   */
    LOCATION_NAV_CORRECTION_PPP_BIT    = (1<<6)
};

/**
 *  Specify the set of technologies that contribute to
 *  GnssLocation. <br/>
 */
enum GnssLocationPosTechMask {
    /** Technology used to generate GnssLocation
     *  is unknown. <br/>   */
    LOCATION_POS_TECH_DEFAULT_BIT                  = 0,
    /** Satellites-based technology was used to generate
     *  GnssLocation. <br/>   */
    LOCATION_POS_TECH_SATELLITE_BIT                = (1<<0),
    /** Cell towers were used to generate
     *  GnssLocation. <br/>   */
    LOCATION_POS_TECH_CELLID_BIT                   = (1<<1),
    /** Wi-Fi access points were used to generate
     *  GnssLocation. <br/>   */
    LOCATION_POS_TECH_WIFI_BIT                     = (1<<2),
    /** Sensors were used to generate
     *  GnssLocation. <br/>   */
    LOCATION_POS_TECH_SENSORS_BIT                  = (1<<3),
    /**  Reference location was used to generate GnssLocation.
     *   <br/> */
    LOCATION_POS_TECH_REFERENCE_LOCATION_BIT       = (1<<4),
    /** Coarse position injected into the location engine was used to
     *  generate GnssLocation.  <br/>   */
    LOCATION_POS_TECH_INJECTED_COARSE_POSITION_BIT = (1<<5),
    /** AFLT was used to generate GnssLocation. <br/>   */
    LOCATION_POS_TECH_AFLT_BIT                     = (1<<6),
    /** GNSS and network-provided measurements were used to generate
     *  GnssLocation. <br/>   */
    LOCATION_POS_TECH_HYBRID_BIT                   = (1<<7),
    /** Precise position engine was used to generate
     *  GnssLocation. <br/>   */
    LOCATION_POS_TECH_PPE_BIT                      = (1<<8)
};

/** Specify the valid fields in
 *  GnssLocationPositionDynamics. <br/>   */
enum GnssLocationPosDataMask {
    /** GnssLocationPositionDynamics has valid
     *  GnssLocationPositionDynamics::longAccel. <br/>   */
    LOCATION_NAV_DATA_HAS_LONG_ACCEL_BIT  = (1<<0),
    /** GnssLocationPositionDynamics has valid
     *  GnssLocationPositionDynamics::latAccel. <br/>   */
    LOCATION_NAV_DATA_HAS_LAT_ACCEL_BIT   = (1<<1),
    /** GnssLocationPositionDynamics has valid
     *  GnssLocationPositionDynamics::vertAccel. <br/>   */
    LOCATION_NAV_DATA_HAS_VERT_ACCEL_BIT  = (1<<2),
    /** GnssLocationPositionDynamics has valid
     *  GnssLocationPositionDynamics::yawRate. <br/>   */
    LOCATION_NAV_DATA_HAS_YAW_RATE_BIT    = (1<<3),
    /** GnssLocationPositionDynamics has valid
     *  GnssLocationPositionDynamics::pitch. <br/>   */
    LOCATION_NAV_DATA_HAS_PITCH_BIT       = (1<<4),
    /** GnssLocationPositionDynamics has valid
     *  GnssLocationPositionDynamics::longAccelUnc. <br/>   */
    LOCATION_NAV_DATA_HAS_LONG_ACCEL_UNC_BIT  = (1<<5),
    /** GnssLocationPositionDynamics has valid
     *  GnssLocationPositionDynamics::latAccelUnc. <br/>   */
    LOCATION_NAV_DATA_HAS_LAT_ACCEL_UNC_BIT   = (1<<6),
    /** GnssLocationPositionDynamics has valid
     *  GnssLocationPositionDynamics::vertAccelUnc. <br/>   */
    LOCATION_NAV_DATA_HAS_VERT_ACCEL_UNC_BIT  = (1<<7),
    /** GnssLocationPositionDynamics has valid
     *  GnssLocationPositionDynamics::yawRateUnc. <br/>   */
    LOCATION_NAV_DATA_HAS_YAW_RATE_UNC_BIT    = (1<<8),
    /** GnssLocationPositionDynamics has valid
     *  GnssLocationPositionDynamics::pitchUnc. <br/>   */
    LOCATION_NAV_DATA_HAS_PITCH_UNC_BIT       = (1<<9)
};

/** Specify the mask for available GNSS signal type and RF band
 *  used in GnssSv::gnssSignalTypeMask and
 *  GnssMeasUsageInfo::gnssSignalType. <br/>   */
enum GnssSignalTypeMask {
    /** GNSS signal is of GPS L1CA RF band. <br/>   */
    GNSS_SIGNAL_GPS_L1CA_BIT            = (1<<0),
    /** GNSS signal is of GPS L1C RF band. <br/>   */
    GNSS_SIGNAL_GPS_L1C_BIT             = (1<<1),
    /** GNSS signal is of GPS L2 RF band. <br/>   */
    GNSS_SIGNAL_GPS_L2_BIT              = (1<<2),
    /** GNSS signal is of GPS L5 RF band. <br/>   */
    GNSS_SIGNAL_GPS_L5_BIT              = (1<<3),
    /** GNSS signal is of GLONASS G1 (L1OF) RF band. <br/>   */
    GNSS_SIGNAL_GLONASS_G1_BIT          = (1<<4),
    /** GNSS signal is of GLONASS G2 (L2OF) RF band. <br/>   */
    GNSS_SIGNAL_GLONASS_G2_BIT          = (1<<5),
    /** GNSS signal is of GALILEO E1 RF band. <br/>   */
    GNSS_SIGNAL_GALILEO_E1_BIT          = (1<<6),
    /** GNSS signal is of GALILEO E5A RF band. <br/>   */
    GNSS_SIGNAL_GALILEO_E5A_BIT         = (1<<7),
    /** GNSS signal is of GALILEO E5B RF band. <br/>   */
    GNSS_SIGNAL_GALILEO_E5B_BIT         = (1<<8),
    /** GNSS signal is of BEIDOU B1 RF band. <br/>   */
    GNSS_SIGNAL_BEIDOU_B1_BIT           = (1<<9),
    /** GNSS signal is of BEIDOU B2 RF band. <br/>   */
    GNSS_SIGNAL_BEIDOU_B2_BIT           = (1<<10),
    /** GNSS signal is of QZSS L1CA RF band. <br/>   */
    GNSS_SIGNAL_QZSS_L1CA_BIT           = (1<<11),
    /** GNSS signal is of QZSS L1S RF band. <br/>   */
    GNSS_SIGNAL_QZSS_L1S_BIT            = (1<<12),
    /** GNSS signal is of QZSS L2 RF band. <br/>   */
    GNSS_SIGNAL_QZSS_L2_BIT             = (1<<13),
    /** GNSS signal is of QZSS L5 RF band. <br/>   */
    GNSS_SIGNAL_QZSS_L5_BIT             = (1<<14),
    /** GNSS signal is of SBAS L1 RF band. <br/>   */
    GNSS_SIGNAL_SBAS_L1_BIT             = (1<<15),
    /** GNSS signal is of BEIDOU B1I RF band. <br/>   */
    GNSS_SIGNAL_BEIDOU_B1I_BIT          = (1<<16),
    /** GNSS signal is of BEIDOU B1C RF band. <br/>   */
    GNSS_SIGNAL_BEIDOU_B1C_BIT          = (1<<17),
    /** GNSS signal is of BEIDOU B2I RF band. <br/>   */
    GNSS_SIGNAL_BEIDOU_B2I_BIT          = (1<<18),
    /** GNSS signal is of BEIDOU B2AI RF band. <br/>   */
    GNSS_SIGNAL_BEIDOU_B2AI_BIT         = (1<<19),
    /** GNSS signal is of NAVIC L5 RF band. <br/>   */
    GNSS_SIGNAL_NAVIC_L5_BIT            = (1<<20),
    /** GNSS signal is of BEIDOU B2A_Q RF band. <br/>   */
    GNSS_SIGNAL_BEIDOU_B2AQ_BIT         = (1<<21),
};

/** Specify LocationClientApi function call processing status.
 *  The status is returned via ResponseCb. <br/>   */
enum LocationResponse {
    /** LocationClientApi call is successful. <br/>   */
    LOCATION_RESPONSE_SUCCESS = 0,
    /** LocationClientApi call has failed. <br/>   */
    LOCATION_RESPONSE_UNKOWN_FAILURE,
    /** LocationClientApi call is not supported. <br/>   */
    LOCATION_RESPONSE_NOT_SUPPORTED
};

/** Specify the SV constellation type in GnssSv
 *  and GnssMeasurementsData. <br/>   */
enum GnssSvType {
    /** SV is of unknown constellation. <br/>   */
    GNSS_SV_TYPE_UNKNOWN = 0,
    /** SV is of GPS constellation. <br/>  */
    GNSS_SV_TYPE_GPS     = 1,
    /**  SV is of SBAS constellation. <br/>  */
    GNSS_SV_TYPE_SBAS    = 2,
    /**  SV is of GLONASS constellation. <br/> */
    GNSS_SV_TYPE_GLONASS = 3,
    /**  SV is of QZSS constellation. <br/>   */
    GNSS_SV_TYPE_QZSS    = 4,
    /**  SV is of BEIDOU constellation. <br/>   */
    GNSS_SV_TYPE_BEIDOU  = 5,
    /**  SV is of GALILEO constellation. <br/>   */
    GNSS_SV_TYPE_GALILEO = 6,
    /**  SV is of NAVIC constellation. <br/>   */
    GNSS_SV_TYPE_NAVIC = 7,
};


/** Specify the valid fields in GnssLocation.
 *  <br/>
 *
 *  User should determine whether a field in
 *  GnssLocation is valid or not by checking
 *  whether the corrsponding bit in
 *  GnssLocation::gnssInfoFlags is set or
 *  not. <br/> */
enum GnssLocationInfoFlagMask {
    /** GnssLocation has valid
     *  GnssLocation::altitudeMeanSeaLevel. <br/>   */
    GNSS_LOCATION_INFO_ALTITUDE_MEAN_SEA_LEVEL_BIT      = (1<<0),
    /** GnssLocation has valid
     *  GnssLocation::pdop,
     *  GnssLocation::hdop and
     *  GnssLocation::vdop. <br/>   */
    GNSS_LOCATION_INFO_DOP_BIT                          = (1<<1),
    /** GnssLocation has valid
     *  GnssLocation::magneticDeviation. <br/>   */
    GNSS_LOCATION_INFO_MAGNETIC_DEVIATION_BIT           = (1<<2),
    /** GnssLocation has valid
     *  GnssLocation::horReliability. <br/>   */
    GNSS_LOCATION_INFO_HOR_RELIABILITY_BIT              = (1<<3),
    /** GnssLocation has valid
     *  GnssLocation::verReliability.  <br/>   */
    GNSS_LOCATION_INFO_VER_RELIABILITY_BIT              = (1<<4),
    /** GnssLocation has valid
     *  GnssLocation::horUncEllipseSemiMajor. <br/>   */
    GNSS_LOCATION_INFO_HOR_ACCURACY_ELIP_SEMI_MAJOR_BIT = (1<<5),
    /** GnssLocation has valid
     *  GnssLocation::horUncEllipseSemiMinor. <br/>   */
    GNSS_LOCATION_INFO_HOR_ACCURACY_ELIP_SEMI_MINOR_BIT = (1<<6),
    /** GnssLocation has valid
     *  GnssLocation::horUncEllipseOrientAzimuth. <br/>   */
    GNSS_LOCATION_INFO_HOR_ACCURACY_ELIP_AZIMUTH_BIT    = (1<<7),
    /** GnssLocation has valid
     *  GnssLocation::svUsedInPosition and
     *  GnssLocation::measUsageInfo. <br/>   */
    GNSS_LOCATION_INFO_GNSS_SV_USED_DATA_BIT            = (1<<8),
    /** GnssLocation has valid
     *  GnssLocation::navSolutionMask. <br/>   */
    GNSS_LOCATION_INFO_NAV_SOLUTION_MASK_BIT            = (1<<9),
    /** GnssLocation has valid
     *  GnssLocation::posTechMask. <br/>   */
    GNSS_LOCATION_INFO_POS_TECH_MASK_BIT                = (1<<10),
    /** Deplicated field as there is no corresponding field in
     *  GnssLocation. <br/>   */
    GNSS_LOCATION_INFO_SV_SOURCE_INFO_BIT               = (1<<11),
    /** GnssLocation has valid
     *  GnssLocation::altitudeMeanSeaLevel. <br/> */
    GNSS_LOCATION_INFO_POS_DYNAMICS_DATA_BIT            = (1<<12),
    /** GnssLocation has valid
     *  GnssLocation::gdop and
     *  GnssLocation::tdop. <br/>   */
    GNSS_LOCATION_INFO_EXT_DOP_BIT                      = (1<<13),
    /** GnssLocation has valid GnssLocation::northStdDeviation.
     *  <br/> */
    GNSS_LOCATION_INFO_NORTH_STD_DEV_BIT                = (1<<14),
    /** GnssLocation has valid
     *  GnssLocation::eastStdDeviation. <br/>   */
    GNSS_LOCATION_INFO_EAST_STD_DEV_BIT                 = (1<<15),
    /** GnssLocation has valid
     *  GnssLocation::northVelocity. <br/>   */
    GNSS_LOCATION_INFO_NORTH_VEL_BIT                    = (1<<16),
    /** GnssLocation has valid
     *  GnssLocation::eastVelocity. <br/>   */
    GNSS_LOCATION_INFO_EAST_VEL_BIT                     = (1<<17),
    /** GnssLocation has valid
     *  GnssLocation::upVelocity. <br/>   */
    GNSS_LOCATION_INFO_UP_VEL_BIT                       = (1<<18),
    /** GnssLocation has valid
     *  GnssLocation::northVelocityStdDeviation. <br/>   */
    GNSS_LOCATION_INFO_NORTH_VEL_UNC_BIT                = (1<<19),
    /** GnssLocation has valid
     *  GnssLocation::eastVelocityStdDeviation. <br/>   */
    GNSS_LOCATION_INFO_EAST_VEL_UNC_BIT                 = (1<<20),
    /** GnssLocation has valid
     *  GnssLocation::upVelocityStdDeviation. <br/>   */
    GNSS_LOCATION_INFO_UP_VEL_UNC_BIT                   = (1<<21),
    /** GnssLocation has valid
     *  GnssLocation::leapSeconds. <br/>   */
    GNSS_LOCATION_INFO_LEAP_SECONDS_BIT                 = (1<<22),
    /** GnssLocation has valid
     *  GnssLocation::timeUncMs. <br/>   */
    GNSS_LOCATION_INFO_TIME_UNC_BIT                     = (1<<23),
    /** GnssLocation has valid
     *  GnssLocation::numSvUsedInPosition. <br/>   */
    GNSS_LOCATION_INFO_NUM_SV_USED_IN_POSITION_BIT      = (1<<24),
    /** GnssLocation has valid
     *  GnssLocation::calibrationConfidencePercent. <br/>   */
    GNSS_LOCATION_INFO_CALIBRATION_CONFIDENCE_PERCENT_BIT = (1<<25),
    /** GnssLocation has valid
     *  GnssLocation::calibrationStatus.  <br/>   */
    GNSS_LOCATION_INFO_CALIBRATION_STATUS_BIT           = (1<<26),
    /** GnssLocation has valid
     *  GnssLocation::locOutputEngType.  <br/>   */
    GNSS_LOCATION_INFO_OUTPUT_ENG_TYPE_BIT              = (1<<27),
    /** GnssLocation has valid
     *  GnssLocation::locOutputEngMask. <br/>   */
    GNSS_LOCATION_INFO_OUTPUT_ENG_MASK_BIT              = (1<<28),
    /** GnssLocation has valid GnssLocation::conformityIndex.
     *  <br/> */
    GNSS_LOCATION_INFO_CONFORMITY_INDEX_BIT         = (1<<29),
};

/** Specify the reliability level of
 *  GnssLocation::horReliability and
 *  GnssLocation::verReliability. <br/>   */
enum LocationReliability {
    /** GnssLocation reliability is not set. <br/>   */
    LOCATION_RELIABILITY_NOT_SET = 0,
    /** GnssLocation reliability is very low. <br/>
     *  Use it at your own risk. <br/>   */
    LOCATION_RELIABILITY_VERY_LOW,
    /** GnssLocation reliability is low. <br/>
     *  Little or no cross-checking is possible <br/>   */
    LOCATION_RELIABILITY_LOW,
    /** GnssLocation reliability is medium. <br/>
     *  Limited cross-check has passed <br/>   */
    LOCATION_RELIABILITY_MEDIUM,
    /** GnssLocation reliability is high. <br/>
     *  Strong cross-check passed. <br/>   */
    LOCATION_RELIABILITY_HIGH
};

/** Specify the constellation that the SV belongs to in
 *   GnssMeasUsageInfo and GnssSystemTime. <br/> */
enum Gnss_LocSvSystemEnumType {
    /** SV is of GPS constellation. <br/>   */
    GNSS_LOC_SV_SYSTEM_GPS     = 1,
    /**  SV is of GALILEO constellation. <br/>   */
    GNSS_LOC_SV_SYSTEM_GALILEO = 2,
    /**  SV is of SBAS sconstellation. <br/>   */
    GNSS_LOC_SV_SYSTEM_SBAS    = 3,
    /**  SV is of GLONASS constellation. <br/>   */
    GNSS_LOC_SV_SYSTEM_GLONASS = 4,
    /**  SV is of BDS constellation. <br/>   */
    GNSS_LOC_SV_SYSTEM_BDS     = 5,
    /**  SV is of QZSS constellation. <br/>   */
    GNSS_LOC_SV_SYSTEM_QZSS    = 6,
    /**  SV is of NAVIC constellation. <br/>   */
    GNSS_LOC_SV_SYSTEM_NAVIC   = 7,
};

/** Specify the valid fields in GnssSystemTimeStructType.
 *  <br/>  */
enum GnssSystemTimeStructTypeFlags {
    /** GnssSystemTimeStructType has valid
     *  GnssSystemTimeStructType::systemWeek. <br/>  */
    GNSS_SYSTEM_TIME_WEEK_VALID             = (1 << 0),
    /** GnssSystemTimeStructType has valid
     *  GnssSystemTimeStructType::systemMsec. <br/>  */
    GNSS_SYSTEM_TIME_WEEK_MS_VALID          = (1 << 1),
    /** GnssSystemTimeStructType has valid
     *  GnssSystemTimeStructType::systemClkTimeBias. <br/>  */
    GNSS_SYSTEM_CLK_TIME_BIAS_VALID         = (1 << 2),
    /** GnssSystemTimeStructType has valid
     *  GnssSystemTimeStructType::systemClkTimeUncMs. <br/>  */
    GNSS_SYSTEM_CLK_TIME_BIAS_UNC_VALID     = (1 << 3),
    /** GnssSystemTimeStructType has valid
     *  GnssSystemTimeStructType::refFCount. <br/>  */
    GNSS_SYSTEM_REF_FCOUNT_VALID            = (1 << 4),
    /** GnssSystemTimeStructType has valid
     *  GnssSystemTimeStructType::numClockResets. <br/>  */
    GNSS_SYSTEM_NUM_CLOCK_RESETS_VALID      = (1 << 5)
};

/** Specify the valid fields in GnssGloTimeStructType. <br/>  */
enum GnssGloTimeStructTypeFlags {
    /** GnssGloTimeStructType has valid
     *  GnssGloTimeStructType::gloDays. <br/> */
    GNSS_CLO_DAYS_VALID                     = (1 << 0),
    /** GnssGloTimeStructType has valid
     *  GnssGloTimeStructType::gloMsec. <br/>  */
    GNSS_GLO_MSEC_VALID                     = (1 << 1),
    /** GnssGloTimeStructType has valid
     *  GnssGloTimeStructType::gloClkTimeBias. <br/>  */
    GNSS_GLO_CLK_TIME_BIAS_VALID            = (1 << 2),
    /** GnssGloTimeStructType has valid
     *  GnssGloTimeStructType::gloClkTimeUncMs. <br/>  */
    GNSS_GLO_CLK_TIME_BIAS_UNC_VALID        = (1 << 3),
    /** GnssGloTimeStructType has valid
     *  GnssGloTimeStructType::refFCount. <br/>  */
    GNSS_GLO_REF_FCOUNT_VALID               = (1 << 4),
    /** GnssGloTimeStructType has valid
     *  GnssGloTimeStructType::numClockResets. <br/>  */
    GNSS_GLO_NUM_CLOCK_RESETS_VALID         = (1 << 5),
    /** GnssGloTimeStructType has valid
     *  GnssGloTimeStructType::gloClkTimeBias. <br/>  */
    GNSS_GLO_FOUR_YEAR_VALID                = (1 << 6),
};

/** Specify the sensor calibrations status in
 *  GnssLocation. <br/>  */
enum DrCalibrationStatusMask {
    /** Indicate that roll calibration is needed. <br/>
     *  Need to take more turns on level ground. <br/>  */
    DR_ROLL_CALIBRATION_NEEDED  = (1<<0),
    /** Indicate that pitch calibration is needed. <br/>
     *  Need to take more turns on level ground. <br/>  */
    DR_PITCH_CALIBRATION_NEEDED = (1<<1),
    /** Indicate that yaw calibration is needed. <br/>
     *  Need to accelerate in a straight line. <br/>  */
    DR_YAW_CALIBRATION_NEEDED   = (1<<2),
    /** Indicate that odo calibration is needed. <br/>
     *  Need to accelerate in a straight line. <br/>  */
    DR_ODO_CALIBRATION_NEEDED   = (1<<3),
    /** Indicate that gyro calibration is needed. <br/>
     *  Need to take more turns on level ground. <br/>  */
    DR_GYRO_CALIBRATION_NEEDED  = (1<<4)
};

/** Specify the set of SVs that are used to calculate
 *  GnssLocation. <br/>  */
struct GnssLocationSvUsedInPosition {
    /** Specify the set of SVs from GPS constellation that are used
     *  to compute the position.<br/>
     *  Bit 0 to Bit 31 corresponds to GPS SV id 1 to 32. <br/> */
    uint64_t gpsSvUsedIdsMask;
    /** Specify the set of SVs from GLONASS constellation that are
     *  used to compute the position. <br/>
     *  Bit 0 to Bit 31 corresponds to GLO SV id 65 to 96. <br/> */
    uint64_t gloSvUsedIdsMask;
    /** Specify the set of SVs from GALILEO constellation that are
     *  used to compute the position. <br/>
     *  Bit 0 to Bit 35 corresponds to GAL SV id 301 to 336.
     *  <br/> */
    uint64_t galSvUsedIdsMask;
    /** ISpecify the set of SVs from BEIDOU constellation that are
     *  used to compute the position. <br/>
     *  Bit 0 to Bit 36 corresponds to BDS SV id 201 to 237.
     *  <br/> */
    uint64_t bdsSvUsedIdsMask;
    /** Specify the set of SVs from QZSS constellation that are used
     *  to compute the position. <br/>
     *  Bit 0 to Bit 4 corresponds to BDS SV id 193 to 197.
     *  <br/> */
    uint64_t qzssSvUsedIdsMask;
    /** Specify the set of SVs from NAVIC constellation that are
     *  used to compute the position. <br/>
     *  Bit 0 to Bit 13 corresponds to BDS SV id 401 to 414.
     *  <br/> */
    uint64_t navicSvUsedIdsMask;
};

/** Specify the SV measurements that are used to calculate
 *  GnssLocation. <br/> */
struct GnssMeasUsageInfo {
   /** Specify GNSS Constellation Type for the SV. */
    Gnss_LocSvSystemEnumType gnssConstellation;
    /** Specify satellite vehicle ID number. <br/>
     *  For SV id range of each supported constellations, refer to
     *  documentation section in GnssSv::svId. <br/>
     */
    uint16_t gnssSvId;
    /** Specify the signal type mask of the SV.  <br/> */
    GnssSignalTypeMask gnssSignalType;
};

/** Specify device body frame parameters. <br/>   */
struct GnssLocationPositionDynamics {
    /** Bitwise OR of GnssLocationPosDataMask to specify the
     *  valid fields in GnssLocationPositionDynamics. <br/>   */
    GnssLocationPosDataMask  bodyFrameDataMask;
    /** Forward acceleration in body frame, in unit of
     *  meters/second^2. <br/>   */
    float           longAccel;
    /** Sideward acceleration in body frame, in unit of
     *  meters/second^2. <br/>   */
    float           latAccel;
    /** Vertical acceleration in body frame, in unit of
     *  meters/second^2. <br/>   */
    float           vertAccel;
    /** Heading rate, in unit of radians/second. <br/>   */
    float           yawRate;
    /** Body pitch, in unit of radians. <br/>   */
    float           pitch;
    /** Uncertainty of forward acceleration in body frame, in unit
     *  of meters/second^2. <br/>   */
    float           longAccelUnc;
    /** Uncertainty of side-ward acceleration in body frame, in unit
     *  of meters/second^2. <br/>   */
    float           latAccelUnc;
    /** Uncertainty of vertical acceleration in body frame, in unit
     *  of meters/second^2. <br/>   */
    float           vertAccelUnc;
    /** Uncertainty of heading rate, in unit of radians/second.
     *  <br/> */
    float           yawRateUnc;
    /** Uncertainty of body pitch, in unit of radians. <br/>   */
    float           pitchUnc;
};

/** Specify none-Glonass GNSS system time info. */
struct GnssSystemTimeStructType {
    /** Bitwise OR of GnssSystemTimeStructTypeFlags to specify
     *  valid fields in GnssSystemTimeStructType. */
    GnssSystemTimeStructTypeFlags validityMask;
    /** Extended week number at reference tick, in unit of week.
     *  <br/>
     *  Set to 65535 if week number is unknown. <br/>
     *
     *  For GPS: calculated from midnight, Jan. 6, 1980. OTA decoded
     *  10 bit GPS week is extended to map between: [NV6264 to
     *  (NV6264 + 1023)] <br/>
     *
     *  For BDS: calculated from 00:00:00 on January 1, 2006 of
     *  Coordinated Universal Time (UTC).  <br/>
     *
     *  For GAL: calculated from 00:00 UT on Sunday August 22, 1999
     *  (midnight between August 21 and August 22). <br/>
     */
    uint16_t systemWeek;
    /** Time in to the current week at reference tick, in unit of
     *  milliseconds. <br/>
     *  Range is 0 to 604799999. <br/> */
    uint32_t systemMsec;
    /** System clock time bias, in unit of milliseconds. <br/>
     *  Note: System time (TOW Millisecond) = systemMsec - systemClkTimeBias. <br/> */
    float systemClkTimeBias;
    /** Single sided maximum time bias uncertainty, in unit of
     *  milliseconds. <br/> */
    float systemClkTimeUncMs;
    /** FCount (free running HW timer) value, in unit of
     *  milliseconds. <br/>
     *  Don't use for relative time purpose due to possible
     *  discontinuities. <br/> */
    uint32_t refFCount;
    /** Number of clock resets/discontinuities detected, which
     *  affects the local hardware counter value. <br/>   */
    uint32_t numClockResets;
};

/** Specify Glonass system time info. <br/>   */
struct GnssGloTimeStructType {
    /** Bitwise OR of GnssGloTimeStructTypeFlags to specify the
     *  valid fields in GnssGloTimeStructType. <br/>   */
    GnssGloTimeStructTypeFlags validityMask;
    /** GLONASS four year number from 1996. Refer to GLONASS ICD. <br/>
     *  Applicable only for GLONASS and shall be ignored for
     * other constellations.  <br/> */
    uint8_t gloFourYear;
    /** GLONASS day number in four years. Refer to GLONASS ICD.
     * <br/>
     * If unknown shall be set to 65535. <br/> */
    uint16_t gloDays;
    /** GLONASS time of day in unit of millisecond. Refer to GLONASS
     *  ICD. <br/>  */
    uint32_t gloMsec;
    /** GLONASS clock time bias, in unit of millisecond. <br/>
     *  Note: GLO time (TOD Millisecond) = gloMsec -
     *  gloClkTimeBias.<br/>
     *  Check for gloClkTimeUncMs before use. <br/> */
    float gloClkTimeBias;
    /** Single sided maximum time bias uncertainty, in unit of
     *  milliseconds. <br/>  */
    float gloClkTimeUncMs;
    /** FCount (free running HW timer) value, in unit of
     *  milliseconds. <br/>
     *  Don't use for relative time purpose due to possible
     *  discontinuities. <br/> */
    uint32_t  refFCount;
    /** Number of clock resets/discontinuities detected,
     *  affecting the local hardware counter value. <br/> */
    uint32_t numClockResets;
};

/** Union to hold GNSS system time from different
 *  constellations in GnssSystemTime <br/>   */
union SystemTimeStructUnion {
    /** System time info from GPS constellation. <br/>   */
    GnssSystemTimeStructType gpsSystemTime;
    /** System time info from GALILEO constellation. <br/>   */
    GnssSystemTimeStructType galSystemTime;
    /** System time info from BEIDOU constellation. <br/>   */
    GnssSystemTimeStructType bdsSystemTime;
    /** System time info from QZSS constellation. <br/>   */
    GnssSystemTimeStructType qzssSystemTime;
    /** System time info from GLONASS constellation. <br/>   */
    GnssGloTimeStructType gloSystemTime;
    /** System time info from NAVIC constellation. <br/>   */
    GnssSystemTimeStructType navicSystemTime;
};

/**  GNSS system time in GnssLocation. <br/>
*/
struct GnssSystemTime {
    /** Specify the source constellation for GNSS system time.
     *  <br/> */
    Gnss_LocSvSystemEnumType gnssSystemTimeSrc;
    /** Specify the GNSS system time corresponding to the source.
     *  <br/> */
    SystemTimeStructUnion u;
};

/** Specify the set of engines whose position reports are
    requested via startPositionSession(uint32_t,
    LocReqEngineTypeMask, const EngineReportCbs&,ResponseCb).
    <br/>
*/
typedef enum {
    /** Mask to indicate that client requests the fused/default
      position via registering location_client::EngineLocationsCb
      for the tracking session. <br/>
      The default position is the propagated/aggregated reports from
      all engines running on the system (e.g.: DR/SPE/PPE).  <br/>
    */
    LOC_REQ_ENGINE_FUSED_BIT = (1<<0),
    /** Mask to indicate that client requests the unmodified SPE
      position via registering location_client::EngineLocationsCb
      for the tracking session. <br/>
    */
    LOC_REQ_ENGINE_SPE_BIT   = (1<<1),
    /** Mask to indicate that the client requests unmodified PPE
      position via registering location_client::EngineLocationsCb
      for the tracking session. <br/>
    */
    LOC_REQ_ENGINE_PPE_BIT   = (1<<2),
} LocReqEngineTypeMask;

/** Specify the position engine type that produced GnssLocation. <br/> */
typedef enum {
    /** This is the propagated/aggregated report from the fixes of
     *  all engines running on the system (e.g.: DR/SPE/PPE).
     *  <br/> */
    LOC_OUTPUT_ENGINE_FUSED = 0,
    /** This fix is the unmodified fix from modem GNSS engine.
     *  <br/> */
    LOC_OUTPUT_ENGINE_SPE   = 1,
    /** This is the unmodified fix from PPE engine. <br/> */
    LOC_OUTPUT_ENGINE_PPE   = 2,
    /** This is the entry count of this enum. <br/>   */
    LOC_OUTPUT_ENGINE_COUNT,
} LocOutputEngineType;


/** Specify the set of position engines supported by
 *  LocationClientAPI. <br/>   */
typedef enum {
    /** Mask for standard GNSS position engine. <br/>   */
    STANDARD_POSITIONING_ENGINE = (1 << 0),
    /** Mask for dead reckoning position engine. <br/>   */
    DEAD_RECKONING_ENGINE       = (1 << 1),
    /** Mask for precise position engine. <br/>   */
    PRECISE_POSITIONING_ENGINE  = (1 << 2)
} PositioningEngineMask;

/** Specify the location info received by client via
 *  startPositionSession(uint32_t, uint32_t
 *  LocationCb, ResponseCb). <br/>   */
struct Location {
    /** Bitwise OR of LocationFlagsMask to specify the valid
     *  fields. <br/>   */
    LocationFlagsMask flags;
    /** UTC timestamp for location fix since January 1, 1970, in
     *  unit of milliseconds. <br/>   */
    uint64_t timestamp;
    /** Latitude, in unit of degrees, range [-90.0, 90.0]. <br/>  */
    double latitude;
    /** Longitude, in unit of degrees, range [-180.0, 180.0].
     *  <br/> */
    double longitude;
    /** Altitude above the WGS 84 reference ellipsoid, in unit of
     *  meters. <br/>   */
    double altitude;
    /** Horizontal speed, in meters/second. <br/>   */
    float speed;
    /** Bearing, in unit of degrees, range [0, 360) <br/>   */
    float bearing;
    /** Horizontal accuracy, in unit of meters. <br/>   */
    float horizontalAccuracy;
    /** Vertial accuracy, in uint of meters. <br/>   */
    float verticalAccuracy;
    /** Speed uncertainty, in unit meters/second. <br/>   */
    float speedAccuracy;
    /** Bearing uncertainty, in unit of degrees, range (0 to
     *  359.999). <br/>   */
    float bearingAccuracy;
    /** Sets of technology that contributed to the fix. <br/>   */
    LocationTechnologyMask techMask;
};

/** Specify the location info received by client via
 *  startPositionSession(uint32_t, const
 *  GnssReportCbs&, ResponseCb) and
 *  startPositionSession(uint32_t,
 *  LocReqEngineTypeMask, const EngineReportCbs&, ResponseCb).
 *  <br/>
 */
struct GnssLocation : public Location {
    /** Bitwise OR of GnssLocationInfoFlagMask for param
     *  validity. <br/>   */
    GnssLocationInfoFlagMask gnssInfoFlags;
    /** Altitude wrt mean sea level, in unit of meters. <br/>   */
    float altitudeMeanSeaLevel;
    /** Position dilusion of precision, range: 0 (highest accuracy)
     *  to 50 (lowest accuracy). <br/>   */
    float pdop;
    /** Horizontal dilusion of precision, range: 0 (highest
     *  accuracy) to 50 (lowest accuracy). <br/>   */
    float hdop;
    /** Vertical dilusion of precision, range: 0 (highest accuracy)
     *  to 50 (lowest accuracy). <br/>   */
    float vdop;
    /** Geometric  dilution of precision range: 0 (highest accuracy)
     *  to 50 (lowest accuracy). <br/>   */
    float gdop;
    /** Time dilution of precision range: 0 (highest accuracy) to 50
     *  (lowest accuracy). <br/>   */
    float tdop;
    /** Difference between the bearing to true
     * north and the bearing shown on a magnetic compass. <br/>
     * The deviation is positive when the magnetic north is east of
     * true north. <br/>   */
    float magneticDeviation;
    /** Horizontal reliability. <br/>   */
    LocationReliability horReliability;
    /** Vertical reliability. <br/>   */
    LocationReliability verReliability;
    /** Horizontal elliptical accuracy semi-major axis, in unit of
     *  meters. <br/>   */
    float horUncEllipseSemiMajor;
    /** Horizontal elliptical accuracy semi-minor axis, in unit of
     *  meters. <br/>  <br/>   */
    float horUncEllipseSemiMinor;
    /** Horizontal elliptical accuracy azimuth, in unit of degrees,
     *  range [0, 180]. <br/>   */
    float horUncEllipseOrientAzimuth;
    /** North standard deviation, in unit of meters. <br/>   */
    float northStdDeviation;
    /** East standard deviation, in unit of meters. <br/>   */
    float eastStdDeviation;
    /** North velocity, in unit of meters/sec. <br/>   */
    float northVelocity;
    /** East velocity, in unit of meters/sec. <br/>   */
    float eastVelocity;
    /** Up velocity, in unit of meters/sec. <br/>   */
    float upVelocity;
    /** North velocity uncertainty, in unit of meters/sec. <br/>  */
    float northVelocityStdDeviation;
    /** East velocity uncertainty, in unit of meters/sec <br/>   */
    float eastVelocityStdDeviation;
    /** Up velocity uncertainty, in unit of meters/sec. <br/>   */
    float upVelocityStdDeviation;
    /** Number of SV used in position report. <br/>   */
    uint16_t numSvUsedInPosition;
    /** GNSS sv used in position data. <br/>   */
    GnssLocationSvUsedInPosition svUsedInPosition;
    /** Navigation solutions that are used to calculate the position
     *  report. <br/>   */
    GnssLocationNavSolutionMask  navSolutionMask;
    /** Position technology used in computing this fix. */
    GnssLocationPosTechMask      posTechMask;
    /** Body frame dynamics info. <br/>   */
    GnssLocationPositionDynamics bodyFrameData;
    /** GNSS system time when this position is calculated. <br/>  */
    GnssSystemTime               gnssSystemTime;
    /** GNSS measurement usage info. <br/>   */
    std::vector<GnssMeasUsageInfo> measUsageInfo;
    /** Number of leap Seconds at time when this position is
     *  generated. */
    uint8_t leapSeconds;
    /** Time uncertainty, in unit of milliseconds. <br/>   */
    float timeUncMs;
    /** Sensor calibration confidence percent, range [0, 100].
     *  <br/> */
    uint8_t calibrationConfidencePercent;
    /** Sensor calibration status.  */
    DrCalibrationStatusMask calibrationStatus;
    /** Location engine type. <br/>
        When this field is set to LOC_ENGINE_SRC_FUSED, the fix is
        the propagated/aggregated reports from all engines running
        on the system (e.g.: DR/SPE/PPE).<br/>
        To check which location engine contributes to the fused
        output, check for locOutputEngMask. <br/> */
    LocOutputEngineType locOutputEngType;
    /** When loc output eng type is set to fused, this field
     *  indicates the set of engines contribute to the fix. <br/> */
    PositioningEngineMask locOutputEngMask;
    /** When robust location is enabled, this field
     * will indicate how well the various input data considered for
     * navigation solution conform to expectations.
     * Range: [0.0, 1.0], with 0.0 for least conforming and 1.0 for
     * most conforming. </br> */
    float conformityIndex;

    /* Default constructor to initalize GnssLocation structure */
    inline GnssLocation() :
            Location({}), gnssInfoFlags((GnssLocationInfoFlagMask)0),
            altitudeMeanSeaLevel(0.0f), pdop(0.0f), hdop(0.0f),
            vdop(0.0f), gdop(0.0f), tdop(0.0f), magneticDeviation(0.0f),
            horReliability(LOCATION_RELIABILITY_NOT_SET),
            verReliability(LOCATION_RELIABILITY_NOT_SET),
            horUncEllipseSemiMajor(0.0f), horUncEllipseSemiMinor(0.0f),
            horUncEllipseOrientAzimuth(0.0f), northStdDeviation(0.0f),
            eastStdDeviation(0.0f), northVelocity(0.0f), eastVelocity(0.0f),
            upVelocity(0.0f), northVelocityStdDeviation(0.0f),
            eastVelocityStdDeviation(0.0f), upVelocityStdDeviation(0.0f),
            numSvUsedInPosition(0), svUsedInPosition({}),
            navSolutionMask((GnssLocationNavSolutionMask)0),
            posTechMask((GnssLocationPosTechMask)0),
            bodyFrameData({}),
            gnssSystemTime({}), measUsageInfo(), leapSeconds(0),
            timeUncMs(0.0f), calibrationConfidencePercent(0),
            calibrationStatus((DrCalibrationStatusMask)0),
            locOutputEngType ((LocOutputEngineType)0),
            locOutputEngMask((PositioningEngineMask)0),
            conformityIndex(0.0f) {
    }
};

/** GNSS SV report that comes when clients registers for
 *  location_client::GnssSvCb. */
struct GnssSv {
    /** Unique SV Identifier. This field is always valid. <br/>
     *   SV Range for supported constellation is specified as below:
     *   <br/>
     *    - For GPS:     1 to 32 <br/>
     *    - For GLONASS: 65 to 96 <br/>
     *    - For SBAS:    120 to 158 and 183 to 191 <br/>
     *    - For QZSS:    193 to 197 <br/>
     *    - For BDS:     201 to 237 <br/>
     *    - For GAL:     301 to 336 <br/>
     *    - For NAVIC:   401 to 414 <br/>   */
    uint16_t svId;
    /** Constellation type of the SV (GPS, SBAS, GLONASS, QZSS,
     *  BEIDOU, GALILEO). <br/>
     *  This field is always valid.  <br/> */
    GnssSvType type;
     /** Signal-to-noise ratio at antenna of the SV, in unit of
      * dB-Hz. <br/>
      * This field is always valid.  <br/> */
    float cN0Dbhz;
    /** Elevation of the SV, in unit of degrees. <br/> This field is
     *  always valid.  <br/> */
    float elevation;
    /** Azimuth of the SV, in unit of degrees. <br/> This field is
     *  always valid.  <br/> */
    float azimuth;
    /** Bitwise OR of GnssSvOptionsMask to specify additional
     *  info and valid fields in GnssSv. <br/>
     *  This field is always valid.  <br/>  */
    GnssSvOptionsMask gnssSvOptionsMask;
    /** Carrier frequency of the signal tracked. <br/> This field is
     *  valid if gnssSvOptionsMask has
     *  GNSS_SV_OPTIONS_HAS_CARRIER_FREQUENCY_BIT set.  <br/> */
    float carrierFrequencyHz;
    /** GNSS signal type mask of the SV. <br/>
     *  This field is valid if gnssSvOptionsMask has
     *  GNSS_SV_OPTIONS_HAS_GNSS_SIGNAL_TYPE_BIT. <br/> */
    GnssSignalTypeMask gnssSignalTypeMask;
};

/** Specify the GNSS signal type and RF band for jammer info and
 *  automatic gain control metric in GnssData. <br/>
 *  To find out the jammer info and automatic gain control
 *  metric for a particular GNSS signal type, refer to the array
 *  element with index set to the signal type. <br/>
 */
enum GnssSignalTypes {
    /**  GNSS signal is of GPS L1CA RF band.  <br/>   */
    GNSS_SIGNAL_TYPE_GPS_L1CA = 0,
    /**  GNSS signal is of GPS L1C RF band.  <br/>   */
    GNSS_SIGNAL_TYPE_GPS_L1C = 1,
     /**  GNSS signal is of GPS L2C_L RF band.  <br/>   */
    GNSS_SIGNAL_TYPE_GPS_L2C_L = 2,
    /**  GNSS signal is of GPS L5_Q RF band.  <br/>   */
    GNSS_SIGNAL_TYPE_GPS_L5_Q = 3,
    /**  GNSS signal is of GLONASS G1 (L1OF) RF band.  <br/>   */
    GNSS_SIGNAL_TYPE_GLONASS_G1 = 4,
    /**  GNSS signal is of GLONASS G2 (L2OF) RF band.  <br/>   */
    GNSS_SIGNAL_TYPE_GLONASS_G2 = 5,
    /**  GNSS signal is of GALILEO E1_C RF band.  <br/>   */
    GNSS_SIGNAL_TYPE_GALILEO_E1_C = 6,
    /**  GNSS signal is of GALILEO E5A_Q RF band.  <br/>   */
    GNSS_SIGNAL_TYPE_GALILEO_E5A_Q = 7,
    /**  GNSS signal is of GALILEO E5B_Q RF band.  <br/>   */
    GNSS_SIGNAL_TYPE_GALILEO_E5B_Q = 8,
    /**  GNSS signal is of BEIDOU B1_I RF band.  <br/>   */
    GNSS_SIGNAL_TYPE_BEIDOU_B1_I = 9,
    /**  GNSS signal is of BEIDOU B1C RF band.  <br/>   */
    GNSS_SIGNAL_TYPE_BEIDOU_B1C = 10,
    /**  GNSS signal is of BEIDOU B2_I RF band.  <br/>   */
    GNSS_SIGNAL_TYPE_BEIDOU_B2_I = 11,
    /**  GNSS signal is of BEIDOU B2A_I RF band.  <br/>   */
    GNSS_SIGNAL_TYPE_BEIDOU_B2A_I = 12,
    /**  GNSS signal is of QZSS L1CA RF band.  <br/>   */
    GNSS_SIGNAL_TYPE_QZSS_L1CA = 13,
    /**  GNSS signal is of QZSS L1S RF band.  <br/>   */
    GNSS_SIGNAL_TYPE_QZSS_L1S = 14,
    /**  GNSS signal is of QZSS L2C_L RF band.  <br/>   */
    GNSS_SIGNAL_TYPE_QZSS_L2C_L = 15,
    /**  GNSS signal is of QZSS L5_Q RF band.  <br/>   */
    GNSS_SIGNAL_TYPE_QZSS_L5_Q = 16,
    /**  GNSS signal is of SBAS L1_CA RF band.  <br/>   */
    GNSS_SIGNAL_TYPE_SBAS_L1_CA = 17,
    /**  GNSS signal is of NAVIC L5 RF band. <br/>   */
    GNSS_SIGNAL_TYPE_NAVIC_L5 = 18,
    /**  GNSS signal is of BEIDOU B2A_Q RF band.  <br/>   */
    GNSS_SIGNAL_TYPE_BEIDOU_B2A_Q = 19,
    /** Maximum number of signal types. <br/>   */
    GNSS_MAX_NUMBER_OF_SIGNAL_TYPES
};

/** Specify valid mask of data fields in
 *  GnssData. <br/>   */
enum GnssDataMask {
    /** Jammer Indicator is available. <br/>   */
    GNSS_DATA_JAMMER_IND_BIT = (1ULL << 0),
    /** AGC is available. <br/>   */
    GNSS_DATA_AGC_BIT = (1ULL << 1)
};

/** Specify the additional GNSS data that can be provided
 *  during a tracking session, currently jammer and automatic
 *  gain control data are available. <br/>
 *
 *  To find out the jammer info and automatic gain control
 *  metric for a particular GNSS signal type, refer to the array
 *  element with index set to the interested RF band.  <br/>
 *
 *  For a particular RF band, user can determine
 *  GnssData::jammerInd is valid or not by checking the element
 *  at index of the specified RF band in
 *  GnssData::gnssDataMask has
 *  GNSS_DATA_JAMMER_IND_BIT set. <br/>
 *
 *  For a particular RF band, user can determine GnssData::agc
 *  is valid or not by checking the element at index of the
 *  specified RF band in GnssData::gnssDataMask has
 *  GNSS_DATA_AGC_BIT set. <br/>
 */
struct GnssData {
    /** Bitwise OR of GnssDataMask to indicate the valid data
     *  fields. <br/> */
    GnssDataMask  gnssDataMask[GNSS_MAX_NUMBER_OF_SIGNAL_TYPES];
    /** Jammer Indication for each GNSS signal.  <br/>   */
    double        jammerInd[GNSS_MAX_NUMBER_OF_SIGNAL_TYPES];
    /** Automatic gain control metric, in unit of dB.  <br/>   */
    double        agc[GNSS_MAX_NUMBER_OF_SIGNAL_TYPES];
};

/** Specify valid fields in
 *  GnssMeasurementsData.  <br/>   */
enum GnssMeasurementsDataFlagsMask{
    /** GnssMeasurementsData has valid
     *  GnssMeasurementsData::svId.  <br/>   */
    GNSS_MEASUREMENTS_DATA_SV_ID_BIT                        = (1<<0),
    /** GnssMeasurementsData has valid
     *  GnssMeasurementsData::svType.  <br/>   */
    GNSS_MEASUREMENTS_DATA_SV_TYPE_BIT                      = (1<<1),
    /** GnssMeasurementsData has valid
     *  GnssMeasurementsData::stateMask.  <br/>   */
    GNSS_MEASUREMENTS_DATA_STATE_BIT                        = (1<<2),
    /** GnssMeasurementsData has valid
     *  GnssMeasurementsData::receivedSvTimeNs.  <br/>   */
    GNSS_MEASUREMENTS_DATA_RECEIVED_SV_TIME_BIT             = (1<<3),
    /** GnssMeasurementsData has valid
     *  GnssMeasurementsData::receivedSvTimeUncertaintyNs.  <br/> */
    GNSS_MEASUREMENTS_DATA_RECEIVED_SV_TIME_UNCERTAINTY_BIT = (1<<4),
    /** GnssMeasurementsData has valid
     *  GnssMeasurementsData::carrierToNoiseDbHz.  <br/>   */
    GNSS_MEASUREMENTS_DATA_CARRIER_TO_NOISE_BIT             = (1<<5),
    /** GnssMeasurementsData has valid
     *  GnssMeasurementsData::pseudorangeRateMps.  <br/>   */
    GNSS_MEASUREMENTS_DATA_PSEUDORANGE_RATE_BIT             = (1<<6),
    /** GnssMeasurementsData has valid
     *  GnssMeasurementsData::pseudorangeRateUncertaintyMps.
     *  <br/> */
    GNSS_MEASUREMENTS_DATA_PSEUDORANGE_RATE_UNCERTAINTY_BIT = (1<<7),
    /** GnssMeasurementsData has valid
     *  GnssMeasurementsData::adrStateMask.  <br/>   */
    GNSS_MEASUREMENTS_DATA_ADR_STATE_BIT                    = (1<<8),
    /** GnssMeasurementsData has valid
     *  GnssMeasurementsData::adrMeters.  <br/>   */
    GNSS_MEASUREMENTS_DATA_ADR_BIT                          = (1<<9),
    /** GnssMeasurementsData has valid
     *  GnssMeasurementsData::adrUncertaintyMeters.  <br/>   */
    GNSS_MEASUREMENTS_DATA_ADR_UNCERTAINTY_BIT              = (1<<10),
    /** GnssMeasurementsData has valid
     *  GnssMeasurementsData::carrierFrequencyHz.  <br/>   */
    GNSS_MEASUREMENTS_DATA_CARRIER_FREQUENCY_BIT            = (1<<11),
    /** GnssMeasurementsData has valid
     *  GnssMeasurementsData::carrierCycles.  <br/>   */
    GNSS_MEASUREMENTS_DATA_CARRIER_CYCLES_BIT               = (1<<12),
    /** GnssMeasurementsData has valid
     *  GnssMeasurementsData::carrierPhase.  <br/>   */
    GNSS_MEASUREMENTS_DATA_CARRIER_PHASE_BIT                = (1<<13),
    /** GnssMeasurementsData has valid
     *  GnssMeasurementsData::carrierPhaseUncertainty.  <br/>   */
    GNSS_MEASUREMENTS_DATA_CARRIER_PHASE_UNCERTAINTY_BIT    = (1<<14),
    /** GnssMeasurementsData has valid
     *  GnssMeasurementsData::multipathIndicator.  <br/>   */
    GNSS_MEASUREMENTS_DATA_MULTIPATH_INDICATOR_BIT          = (1<<15),
    /** GnssMeasurementsData has valid
     *  GnssMeasurementsData::signalToNoiseRatioDb.  <br/>   */
    GNSS_MEASUREMENTS_DATA_SIGNAL_TO_NOISE_RATIO_BIT        = (1<<16),
    /** GnssMeasurementsData has valid
     *  GnssMeasurementsData::agcLevelDb.  <br/>   */
    GNSS_MEASUREMENTS_DATA_AUTOMATIC_GAIN_CONTROL_BIT       = (1<<17),
};

/** Specify GNSS measurement state in
 *  GnssMeasurementsData::stateMask.  <br/>   */
enum GnssMeasurementsStateMask {
    /** GNSS measurement state is unknown.  <br/>   */
    GNSS_MEASUREMENTS_STATE_UNKNOWN_BIT                 = 0,
    /** GNSS measurement state is "code lock". <br/>   */
    GNSS_MEASUREMENTS_STATE_CODE_LOCK_BIT               = (1<<0),
    /** GNSS measurement state is "bit sync". <br/>   */
    GNSS_MEASUREMENTS_STATE_BIT_SYNC_BIT                = (1<<1),
    /** GNSS measurement state is "subframe sync". <br/>   */
    GNSS_MEASUREMENTS_STATE_SUBFRAME_SYNC_BIT           = (1<<2),
    /** GNSS measurement state is "tow decoded". <br/>   */
    GNSS_MEASUREMENTS_STATE_TOW_DECODED_BIT             = (1<<3),
    /** GNSS measurement state is "msec ambiguous". <br/>   */
    GNSS_MEASUREMENTS_STATE_MSEC_AMBIGUOUS_BIT          = (1<<4),
    /** GNSS measurement state is "symbol sync". <br/>   */
    GNSS_MEASUREMENTS_STATE_SYMBOL_SYNC_BIT             = (1<<5),
    /** GNSS measurement state is "GLONASS string sync".  <br/>   */
    GNSS_MEASUREMENTS_STATE_GLO_STRING_SYNC_BIT         = (1<<6),
    /** GNSS measurement state is "GLONASS TOD decoded". <br/>   */
    GNSS_MEASUREMENTS_STATE_GLO_TOD_DECODED_BIT         = (1<<7),
    /** GNSS measurement state is "BDS D2 bit sync".  <br/>   */
    GNSS_MEASUREMENTS_STATE_BDS_D2_BIT_SYNC_BIT         = (1<<8),
    /** GNSS measurement state is "BDS D2 subframe sync". <br/>   */
    GNSS_MEASUREMENTS_STATE_BDS_D2_SUBFRAME_SYNC_BIT    = (1<<9),
    /** GNSS measurement state is "Galileo E1BC code lock". <br/> */
    GNSS_MEASUREMENTS_STATE_GAL_E1BC_CODE_LOCK_BIT      = (1<<10),
    /** GNSS measurement state is "Galileo E1C second code lock". */
    GNSS_MEASUREMENTS_STATE_GAL_E1C_2ND_CODE_LOCK_BIT   = (1<<11),
    /** GNSS measurement state is "Galileo E1B page sync".  <br/> */
    GNSS_MEASUREMENTS_STATE_GAL_E1B_PAGE_SYNC_BIT       = (1<<12),
    /** GNSS measurement state is "SBAS sync".  <br/>   */
    GNSS_MEASUREMENTS_STATE_SBAS_SYNC_BIT               = (1<<13),
};

/** Specify accumulated delta range state in
 *  GnssMeasurementsData::adrStateMask.  <br/>   */
enum GnssMeasurementsAdrStateMask {
    /** Accumulated delta range state is unknown. <br/>   */
    GNSS_MEASUREMENTS_ACCUMULATED_DELTA_RANGE_STATE_UNKNOWN         = 0,
    /** Accumulated delta range state is valid.  <br/>   */
    GNSS_MEASUREMENTS_ACCUMULATED_DELTA_RANGE_STATE_VALID_BIT       = (1<<0),
    /** Accumulated delta range state is "reset".  <br/>   */
    GNSS_MEASUREMENTS_ACCUMULATED_DELTA_RANGE_STATE_RESET_BIT       = (1<<1),
    /** Accumulated delta range state is "cycle slip". <br/>   */
    GNSS_MEASUREMENTS_ACCUMULATED_DELTA_RANGE_STATE_CYCLE_SLIP_BIT  = (1<<2),
};

/** Specify the GNSS multipath indicator state in
 *  GnssMeasurementsData::multipathIndicator. <br/>   */
enum GnssMeasurementsMultipathIndicator {
    /** Multipath indicator is unknown. <br/>   */
    GNSS_MEASUREMENTS_MULTIPATH_INDICATOR_UNKNOWN = 0,
    /** Multipath indicator is present. <br/>   */
    GNSS_MEASUREMENTS_MULTIPATH_INDICATOR_PRESENT,
    /** Multipath indicator is not present. <br/>   */
    GNSS_MEASUREMENTS_MULTIPATH_INDICATOR_NOT_PRESENT,
};

/** Specify the valid fields in
 *  GnssMeasurementsClock. <br/>
 */
enum GnssMeasurementsClockFlagsMask {
    /** GnssMeasurementsClock has valid
     *  GnssMeasurementsClock::leapSecond. <br/>   */
    GNSS_MEASUREMENTS_CLOCK_FLAGS_LEAP_SECOND_BIT                   = (1<<0),
    /** GnssMeasurementsClock has valid
     *  GnssMeasurementsClock::timeNs. <br/>   */
    GNSS_MEASUREMENTS_CLOCK_FLAGS_TIME_BIT                          = (1<<1),
    /** GnssMeasurementsClock has valid
     *  GnssMeasurementsClock::timeUncertaintyNs. <br/>   */
    GNSS_MEASUREMENTS_CLOCK_FLAGS_TIME_UNCERTAINTY_BIT              = (1<<2),
    /** GnssMeasurementsClock has valid
     *  GnssMeasurementsClock::fullBiasNs. <br/>   */
    GNSS_MEASUREMENTS_CLOCK_FLAGS_FULL_BIAS_BIT                     = (1<<3),
    /** GnssMeasurementsClock has valid
     *  GnssMeasurementsClock::biasNs. <br/>   */
    GNSS_MEASUREMENTS_CLOCK_FLAGS_BIAS_BIT                          = (1<<4),
    /** GnssMeasurementsClock has valid
     *  GnssMeasurementsClock::biasUncertaintyNs. <br/>   */
    GNSS_MEASUREMENTS_CLOCK_FLAGS_BIAS_UNCERTAINTY_BIT              = (1<<5),
    /** GnssMeasurementsClock has valid
     *  GnssMeasurementsClock::driftNsps. <br/>   */
    GNSS_MEASUREMENTS_CLOCK_FLAGS_DRIFT_BIT                         = (1<<6),
    /** GnssMeasurementsClock has valid
     *  GnssMeasurementsClock::driftUncertaintyNsps. <br/>   */
    GNSS_MEASUREMENTS_CLOCK_FLAGS_DRIFT_UNCERTAINTY_BIT             = (1<<7),
    /** GnssMeasurementsClock has valid
     *  GnssMeasurementsClock::hwClockDiscontinuityCount. <br/>   */
    GNSS_MEASUREMENTS_CLOCK_FLAGS_HW_CLOCK_DISCONTINUITY_COUNT_BIT  = (1<<8),
};

/** Specify the SV pseudo range and carrier phase measurement
 *  from GNSS positioning engine. <br/>
 *
 *  User should determine whether a field in
 *  GnssMeasurementsClock is valid or not by
 *  checking whether the corrsponding bit in
 *  GnssMeasurementsClock::flags is set or
 *  not. <br/>   */
struct GnssMeasurementsData {
    /** Bitwise OR of GnssMeasurementsDataFlagsMask to specify the
     *  valid fields in GnssMeasurementsData. */
    GnssMeasurementsDataFlagsMask flags;
    /** Specify satellite vehicle ID number. <br/>
     *  For SV id range of each supported constellations, refer to
     *  documentation in GnssSv::svId. <br/>
     */
    int16_t svId;
    /** SV constellation type. <br/>   */
    GnssSvType svType;
    /** Time offset when the measurement was taken,
     *  in unit of nanoseconds. <br/>   */
    double timeOffsetNs;
    /** Bitwise OR of GnssMeasurementsStateMask to specify the
     *  GNSS measurement state. <br/>   */
    GnssMeasurementsStateMask stateMask;
    /** Received GNSS time of the week in nanoseconds when the
     *  measurement was taken. <br/>   */
    int64_t receivedSvTimeNs;
    /** Satellite time. <br/>
     *  All SV times in the current measurement block are already
     *  propagated to a common reference time epoch, in unit of
     *  nano seconds.  <br/> */
    int64_t receivedSvTimeUncertaintyNs;
    /** Signal strength, carrier to noise ratio, in unit of dB-Hz
     *  <br/> */
    double carrierToNoiseDbHz;
    /** Uncorrected pseudorange rate, in unit of metres/second
     *  <br/> */
    double pseudorangeRateMps;
    /** Uncorrected pseudorange rate uncertainty, in unit of
     *  meters/second  <br/> */
    double pseudorangeRateUncertaintyMps;
    /** Bitwise OR of GnssMeasurementsAdrStateMask. <br/>   */
    GnssMeasurementsAdrStateMask adrStateMask;
    /** Accumulated delta range, in unit of meters  <br/> */
    double adrMeters;
    /** Accumulated delta range uncertainty, in unit of meters
     *  <br/> */
    double adrUncertaintyMeters;
    /** Carrier frequency of the tracked signal, in unit of Hertz
     *  <br/> */
    float carrierFrequencyHz;
    /** The number of full carrier cycles between the receiver and
     *  the satellite. <br/>   */
    int64_t carrierCycles;
    /** The RF carrier phase that the receiver has detected.
     *  <br/> */
    double carrierPhase;
    /** The RF carrier phase uncertainty. <br/>   */
    double carrierPhaseUncertainty;
    /** Multipath indicator, could be unknown, present or not
     *  present. <br/>   */
    GnssMeasurementsMultipathIndicator multipathIndicator;
    /** Signal to noise ratio, in unit of dB <br/> */
    double signalToNoiseRatioDb;
    /** Automatic gain control level, in unit of dB <br/> */
    double agcLevelDb;
};

/** Specify GNSS measurements clock. <br/>
 *  The main equation describing the relationship between
 *  various components is: <br/>
 *  utcTimeNs = timeNs - (fullBiasNs + biasNs) - leapSecond *
 *  1,000,000,000 <br/> */
struct GnssMeasurementsClock {
    /** Bitwise OR of GnssMeasurementsClockFlagsMask. <br/>   */
    GnssMeasurementsClockFlagsMask flags;
    /** Leap second, in unit of seconds. <br/>   */
    int16_t leapSecond;
    /** Time, monotonically increasing as long as the power is on,
     *  in unit of nanoseconds. <br/>   */
    int64_t timeNs;
    /** Time uncertainty (one sigma), in unit of nanoseconds
     *  <br/> */
    double timeUncertaintyNs;
    /** Full bias, in uint of nanoseconds. <br/>   */
    int64_t fullBiasNs;
    /** Sub-nanoseconds bias, in unit of nonoseconds <br/> */
    double biasNs;
    /** Bias uncertainty (one sigma), in unit of nanoseconds
     *  <br/> */
    double biasUncertaintyNs;
    /** Clock drift, in unit of nanoseconds/second <br/> */
    double driftNsps;
    /** Clock drift uncertainty (one sigma), in unit of
     *  nanoseconds/second <br/> */
    double driftUncertaintyNsps;
    /** HW clock discontinuity count - incremented
     *  for each discontinuity in HW clock. <br/>   */
    uint32_t hwClockDiscontinuityCount;
};

/** Specify GNSS measurements clock and data. <br/>   */
struct GnssMeasurements {
    /** GNSS measurements clock info. <br/>   */
    GnssMeasurementsClock clock;
    /** GNSS measurements data. <br/>   */
    std::vector<GnssMeasurementsData> measurements;
};

/** Specify the valid fields in LeapSecondSystemInfo. <br/> */
enum LeapSecondSysInfoMask{
    /** LeapSecondSystemInfo has valid
     *  LeapSecondSystemInfo::leapSecondCurrent. <br/>   */
    LEAP_SECOND_SYS_INFO_CURRENT_LEAP_SECONDS_BIT = (1ULL << 0),
    /** LeapSecondSystemInfo has valid
     *  LeapSecondSystemInfo::leapSecondChangeInfo. <br/>   */
    LEAP_SECOND_SYS_INFO_LEAP_SECOND_CHANGE_BIT = (1ULL << 1)
};

/** Specify leap second change event info as part of
 *  LeapSecondSystemInfo.  <br/>   */
struct LeapSecondChangeInfo {
    /** GPS timestamp that corrresponds to the last known leap
        second change event. <br/>
        The info can be available on two scenario: <br/>
        1: this leap second change event has been scheduled and yet
           to happen <br/>
        2: this leap second change event has already happened and
           next leap second change event has not yet been
           scheduled. <br/>   */
    GnssSystemTimeStructType gpsTimestampLsChange;
    /** Number of leap seconds prior to the leap second change event
     *  that corresponds to the timestamp at gpsTimestampLsChange.
     *  <br/>
     *  In unit of seconds. <br/> */
    uint8_t leapSecondsBeforeChange;
    /** Number of leap seconds after the leap second change event
     *  that corresponds to the timestamp at gpsTimestampLsChange.
     *  <br/>
     *  In unit of seconds. <br/> */
    uint8_t leapSecondsAfterChange;
};

/** Specify leap second system info, including current leap
 *  second and leap second change event info if available. <br/>
 *
 *  User should determine whether a field in
 *  LeapSecondSystemInfo is valid or not by
 *  checking whether the corrsponding bit in
 *  LeapSecondSystemInfo::leapSecondInfoMask is
 *  set or not. <br/>  */
struct LeapSecondSystemInfo {
    /** Bitwise OR of LeapSecondSystemInfo::LeapSecondSysInfoMask to
     *  specify valid fields in LeapSecondSystemInfo. */
    LeapSecondSysInfoMask leapSecondInfoMask;
    /** Current leap seconds, in unit of seconds. <br/>
     *  This info will only be available if the leap second change
     *  info is not available. <br/>   */
    uint8_t               leapSecondCurrent;
    /** Leap second change event info. The info can be available on
        two scenario: <br/>
        1: this leap second change event has been scheduled and yet
           to happen <br/>
        2: this leap second change event has already happened and
           next leap second change event has not yet been scheduled.
           <br/>

        If leap second change info is avaiable, to figure out the
        current leap second info, compare current gps time with
        LeapSecondChangeInfo::gpsTimestampLsChange to know whether
        to choose leapSecondBefore or leapSecondAfter as current
        leap second. <br/> */
    LeapSecondChangeInfo  leapSecondChangeInfo;
};

/** Specify the set of valid fields in
 *  LocationSystemInfo. <br/>   */
typedef enum {
    /** LocationSystemInfo has valid
     *  LocationSystemInfo::leapSecondSysInfo. <br/>   */
    LOC_SYS_INFO_LEAP_SECOND = (1ULL << 0),
} LocationSystemInfoMask;

/** Specify the location system info that can be received via
 *  LocationSystemInfoCb. <br/>
 *
 *  User should determine whether a field in
 *  LocationSystemInfo is valid or not by
 *  checking whether the corrsponding bit in
 *  LocationSystemInfo::flags is set or not.
 *  <br/> */
struct LocationSystemInfo {
    /** Bitwise OR of LocationSystemInfoDataMask to indicate the
     *  valid fields in LocationSystemInfo. <br/>   */
    LocationSystemInfoMask systemInfoMask;
    /** Current leap second and leap second info. <br/>   */
    LeapSecondSystemInfo   leapSecondSysInfo;
};

/** Specify the valid fields in GnssSvPoly. <br/> */
enum GnssSvPolyValidityMask {
    /** GnssSvPoly has valid GnssSvPoly::svId. <br/>
     *  This flag should always be set. <br/> */
    GNSS_SV_POLY_HAS_SV_ID              = 0x0000000001,
    /** GnssSvPoly has valid GnssSvPoly::svConstellation. <br/>
     *  This flag should always be set. <br/> */
    GNSS_SV_POLY_HAS_SV_CONSTELLATION   = 0x0000000002,
    /** GnssSvPoly has valid GnssSvPoly::gloFrequency. <br/>
     *  This flag should be set only if this is Glonass SV. <br/> */
    GNSS_SV_POLY_HAS_GLO_FREQUENCY      = 0x0000000004,
    /** GnssSvPoly has valid GnssSvPoly::actionType. <br/>
     *  This flag should always be set. <br/> */
    GNSS_SV_POLY_HAS_ACTION_TYPE        = 0x0000000008,
    /** GnssSvPoly has valid GnssSvPoly::statusMask. <br/> */
    GNSS_SV_POLY_HAS_STATUS_MASK        = 0x0000000010,
    /** GnssSvPoly has valid GnssSvPoly::T0. <br/> */
    GNSS_SV_POLY_HAS_T0                 = 0x0000000020,
    /** GnssSvPoly has valid GnssSvPoly::polyCofXYZ0. <br/> */
    GNSS_SV_POLY_HAS_POLY_COF_XYZ0      = 0x0000000040,
    /** GnssSvPoly has valid GnssSvPoly::polyCofXYZN. <br/> */
    GNSS_SV_POLY_HAS_POLY_COF_XYZN      = 0x0000000080,
    /** GnssSvPoly has valid GnssSvPoly::polyCofClockBias. <br/> */
    GNSS_SV_POLY_HAS_POLY_COF_CLK_BIAS  = 0x0000000100,
    /** GnssSvPoly has valid GnssSvPoly::iode. <br/> */
    GNSS_SV_POLY_HAS_IODE               = 0x0000000200,
    /** GnssSvPoly has valid GnssSvPoly::enhancedIOD. <br/> */
    GNSS_SV_POLY_HAS_ENHANCED_IOD       = 0x0000000400,
    /** GnssSvPoly has valid GnssSvPoly::svPosUnc. <br/> */
    GNSS_SV_POLY_HAS_SV_POS_UNC         = 0x0000000800,
    /** GnssSvPoly has valid GnssSvPoly::ionoDelay. <br/> */
    GNSS_SV_POLY_HAS_IONO_DELAY         = 0x0000001000,
    /** GnssSvPoly has valid GnssSvPoly::ionoDot.  <br/> */
    GNSS_SV_POLY_HAS_IONO_DOT           = 0x0000002000,
    /** GnssSvPoly has valid GnssSvPoly::sbasIonoDelay. <br/> */
    GNSS_SV_POLY_HAS_SBAS_IONO_DELAY    = 0x0000004000,
    /** GnssSvPoly has valid GnssSvPoly::sbasIonoDot. <br/> */
    GNSS_SV_POLY_HAS_SBAS_IONO_DOT      = 0x0000008000,
    /** GnssSvPoly has valid GnssSvPoly::tropoDelay. <br/> */
    GNSS_SV_POLY_HAS_TROPO_DELAY        = 0x0000010000,
    /** GnssSvPoly has valid GnssSvPoly::elevation. <br/> */
    GNSS_SV_POLY_HAS_ELEVATION          = 0x0000020000,
    /** GnssSvPoly has valid GnssSvPoly::elevationDot. <br/> */
    GNSS_SV_POLY_HAS_ELEVATION_DOT      = 0x0000040000,
    /** GnssSvPoly has valid GnssSvPoly::elevationUnc. <br/> */
    GNSS_SV_POLY_HAS_ELEVATION_UNC      = 0x0000080000,
    /** GnssSvPoly has valid GnssSvPoly::velCof. <br/> */
    GNSS_SV_POLY_HAS_VEL_COF            = 0x0000100000,
    /** GnssSvPoly has valid GnssSvPoly::gpsIscL1ca. <br/> */
    GNSS_SV_POLY_HAS_GPS_ISC_L1CA       = 0x0000200000,
    /** GnssSvPoly has valid GnssSvPoly::gpsIscL2c. <br/> */
    GNSS_SV_POLY_HAS_GPS_ISC_L2C        = 0x0000400000,
    /** GnssSvPoly has valid GnssSvPoly::gpsIscL5I5. <br/> */
    GNSS_SV_POLY_HAS_GPS_ISC_L5I5       = 0x0000800000,
    /** GnssSvPoly has valid GnssSvPoly::gpsIscL5Q5. <br/> */
    GNSS_SV_POLY_HAS_GPS_ISC_L5Q5       = 0x0001000000,
    /** GnssSvPoly has valid GnssSvPoly::gpsTgd.  <br/> */
    GNSS_SV_POLY_HAS_GPS_TGD            = 0x0002000000,
    /** GnssSvPoly has valid GnssSvPoly::gloTgdG1G2. <br/> */
    GNSS_SV_POLY_HAS_GLO_TGD_G1G2       = 0x0004000000,
    /** GnssSvPoly has valid GnssSvPoly::bdsTgdB1. <br/> */
    GNSS_SV_POLY_HAS_BDS_TGD_B1         = 0x00080000000,
    /** GnssSvPoly has valid GnssSvPoly::bdsTgdB2. <br/> */
    GNSS_SV_POLY_HAS_BDS_TGD_B2         = 0x00100000000,
    /** GnssSvPoly has valid GnssSvPoly::bdsTgdB2a. <br/> */
    GNSS_SV_POLY_HAS_BDS_TGD_B2A        = 0x00200000000,
    /** GnssSvPoly has valid GnssSvPoly::bdsIscB2a. <br/> */
    GNSS_SV_POLY_HAS_BDS_ISC_B2A        = 0x00400000000,
    /** GnssSvPoly has valid GnssSvPoly::galBgdE1E5a. <br/> */
    GNSS_SV_POLY_HAS_GAL_BGD_E1E5A      = 0x00800000000,
    /** GnssSvPoly has valid GnssSvPoly::galBgdE1E5b. <br/> */
    GNSS_SV_POLY_HAS_GAL_BGD_E1E5B      = 0x01000000000,
    /** GnssSvPoly has valid GnssSvPoly::navicTgdL5. <br/> */
    GNSS_SV_POLY_HAS_NAVIC_TDG_L5       = 0x02000000000,
};

/** Specify the SV polynomial report status. <br/> */
enum GnssSvPolyStatusMask {
    /** Polynomials based on XTRA. <br/> */
    GNSS_SV_POLY_STATUS_SRC_ALM_CORR = 0x01,
    /** GLONASS string 4 has been received. <br/> */
    GNSS_SV_POLY_STATUS_GLO_STR4 = 0x2,
    /** GALILEO polynomial computed from I/Nav navigation
     *  message. <br/> */
    GNSS_SV_POLY_STATUS_GAL_FNAV = 0x04,
    /** GALILEO polynomial computed from F/Nav navigation message.
     *  <br/> */
    GNSS_SV_POLY_STATUS_GAL_INAV = 0x08,
};

/** Action to be performed for the received polynomial for this
 *  satellite. <br/> */
enum GnssSvPolyActionType {
    /** Update Polynomial for this satellite. <br/> */
    GNSS_SV_POLY_ACTION_UPDATE = 1,
    /** Invalidate any previously received polynomial. <br/> */
    GNSS_SV_POLY_ACTION_INVALIDATE = 2,
};

/** Const define for the array size of
 *  GnssSvPoly::polyCofXYZ0. <br/>  */
#define GNSS_SV_POLY_XYZ_0_TH_ORDER_COF_SIZE  3
/** Const define for the array size of
 *  GnssSvPoly::polyCofXYZN. <br/>  */
#define GNSS_SV_POLY_XYZ_N_TH_ORDER_COF_SIZE  9
/** Const define for the array size of
 *  GnssSvPoly::polyCofClockBias. <br/>  */
#define GNSS_SV_POLY_SV_CLKBIAS_COF_SIZE      4
/** Const define for the array size of GnssSvPoly::velCoef.
 *  <br/>  */
#define GNSS_SV_POLY_VELOCITY_COF_SIZE       12

struct GnssSvPoly {
    /** Bitwise OR of GnssSvPolyValidityMask to specify the
     *  valid fields in GnssSvPoly. <br/> */
    GnssSvPolyValidityMask validityMask;

    /** Specify satellite vehicle ID number. <br/>
     *  For SV id range of each supported constellations, refer to
     *  documentation in GnssSv::svId. <br/>
     */
    uint16_t svId;

    /** Specify GNSS Constellation Type for the SV. */
    Gnss_LocSvSystemEnumType svConstellation;

    /**  GLONASS frequency number + 8. This field will only be
     *   marked as valid via validityMask for GLONASS SVs. <br/>
     *   Range: 1 to 14 <br/>   */
    uint8_t gloFrequency;

    /** SV polynomial action type, update or invalidate. <br/> */
    GnssSvPolyActionType actionType;

    /**  SV polynomial report status. <br/> */
    GnssSvPolyStatusMask statusMask;

    /**  Reference time for polynomial calculations. <br/>
     * - GPS, QZSS: Seconds in the week <br/>
     * - GLONASS: Full seconds since Jan. 1, 1996 <br/>
     * - BDS: Full seconds since Jan. 1, 2006 <br/>
     * - Galileo: Calculated from 00:00 UT on Sunday, August 22,
     *     1999 (midnight between August 21 and August 22) <br/>
     */
    double T0;

    /** Polynomial Coefficient's 0th Term for X, Y, and Z
     *  Coordinates (C0X, C0Y, C0Z). <br/>
     *  If this field is indicated as valid via validityMask, then
     *  every element in the array is valid. <br/> */
    double polyCofXYZ0[GNSS_SV_POLY_XYZ_0_TH_ORDER_COF_SIZE];

    /** Polynomial coefficient's 1st, 2nd, and 3rd terms for X, Y
     *  and Z coordinates (C1X, C2X,... C2Z, C3Z). <br/>
     *  If this field is indicated as valid via validityMask, then
     *  every element in the array is valid. <br/>
     *  Units are specified as below: <br/>
     *       - 1st term -- Meters/second <br/>
     *       - 2nd term -- Meters/second^2 <br/>
     *       - 3rd term -- Meters/seconds^3 <br/> */
    double polyCofXYZN[GNSS_SV_POLY_XYZ_N_TH_ORDER_COF_SIZE];

    /** Polynomial coefficients for satellite clock bias
     *  correction (C0T, C1T, C2T, C3T). <br/>
     *  If this field is indicated as valid via validityMask, then
     *  every element in the array is valid. <br/>
     *  Units are specified below <br/>
     *        - 0th term -- Milliseconds/second <br/>
     *        - 1st term -- Milliseconds/second^2 <br/>
     *        - 2nd term -- Milliseconds/second^3 <br/>
     *        - 3rd term -- Milliseconds/second^4 <br/> */
    float polyCofClockBias[GNSS_SV_POLY_SV_CLKBIAS_COF_SIZE];

    /**  Ephemeris reference time. <br/>
     *       - GPS -- Issue of data ephemeris used (unitless) <br/>
     *       - GLONASS -- Tb 7-bit <br/>
     *       - Galileo -- 10-bit <br/>  */
    uint16_t iode;

    /** Enhanced Reference Time, for BDS ephemeris, this is TOE.
     *  <br/> */
    uint32_t enhancedIOD;

    /** SV Position Uncertainty, in unit of meters. <br/> */
    float svPosUnc;

    /** Ionospheric delay at T0, in unit of meters. <br/> */
    float ionoDelay;

    /** Ionospheric delay rate, in unit of Meters/second. <br/>
     */
    float ionoDot;

    /** SBAS ionospheric delay at T0, in unit of  Meters. <br/>
    */
    float sbasIonoDelay;

    /** SBAS ionospheric delay rate, in unit of Meters/second. <br/>
    */
    float sbasIonoDot;

    /** Tropospheric delay, in unit of Meters. <br/>
    */
    float tropoDelay;

    /** Satellite elevation at T0, in unit of Radians. <br/>
    */
    float elevation;

    /** Satellite elevation rate, in unit of Radians/second. <br/>
    */
    float elevationDot;

    /** SV elevation uncertainty, in unit of Radians. <br/>
    */
    float elevationUnc;

    /** Polynomial coefficients for SV velocity (C0X, C1X, C2X,
     *  C3X,... C2Z, C3Z). <br/>
     *  If this field is indicated as valid via validityMask, then
     *    every element in the array is valid. <br/>
     *    Units: <br/>
     *    - 0th term -- Meters/second <br/>
     *    - 1st term -- Meters/second^2 <br/>
     *    - 2nd term -- Meters/second^3 <br/>
     *   - 3rd term -- Meters/second^4 <br/> */
    double velCof[GNSS_SV_POLY_VELOCITY_COF_SIZE];

    /** Inter-signal correction - GPS/QZSS L1C/A, in unit of
     *  milliseconds. <br/>
     */
    float gpsIscL1ca;

    /** Inter-signal correction - GPS/QZSS L2C, in unit of
     *  milliseconds. <br/>
     */
    float gpsIscL2c;

    /** Inter-signal correction - GPS/QZSS L5I5, in unit of
     *  milliseconds. <br/>
     */
    float gpsIscL5I5;

    /** Inter-signal correction - GPS/QZSS L5Q5, in unit of
     *  milliseconds. <br/>
     */
    float gpsIscL5Q5;

    /** Time of group delay - GPS/QZSS, 13 bits from CNAV, 8 bits
     *  from LNAV, in unit of  milliseconds. <br/>
     */
    float gpsTgd;

    /** Time of group delay - GLONASS G1-G2, in unit of
     *  milliseconds. <br/>
     */
    float gloTgdG1G2;

    /** Time of group delay - BDS B1, in unit of
     *  milliseconds. <br/>
     */
    float bdsTgdB1;

    /** Time of group delay - BDS B2, in unit of
     *  milliseconds. <br/>
     */
    float bdsTgdB2;

    /** Time of group delay - BDS B2A, in unit of milliseconds.
     *  <br/>
     */
    float bdsTgdB2a;

    /** Inter-signal correction - BDS B2A, in unit of milliseconds.
     *  <br/>
     */
    float bdsIscB2a;

    /** Broadcast group delay - Galileo E1-E5a, in unit of
     *  milliseconds. <br/>
     */
    float galBgdE1E5a;

    /** Broadcast group delay - Galileo E1-E5b, in unit of
     *  milliseconds. <br/>
     */
    float galBgdE1E5b;

    /** Time of group delay - NavIC L5, in unit of
     *  milliseconds. <br/>
     */
    float navicTgdL5;
};


enum BatchingStatus {
    BATCHING_STATUS_INACTIVE    = 0,
    BATCHING_STATUS_ACTIVE      = 1,
    BATCHING_STATUS_DONE        = 2
};

enum GeofenceBreachTypeMask {
    GEOFENCE_BREACH_ENTER_BIT     = (1<<0),
    GEOFENCE_BREACH_EXIT_BIT      = (1<<1),
    GEOFENCE_BREACH_DWELL_IN_BIT  = (1<<2),
    GEOFENCE_BREACH_DWELL_OUT_BIT = (1<<3),
};

/** @brief Provides the capabilities of the system. <br/>

    @param capsMask: Bitwise OR of LocationCapabilitiesMask
                   <br/>
*/
typedef std::function<void(
    LocationCapabilitiesMask capsMask
)> CapabilitiesCb;

/** @brief Callback to be invoked to inform the processing
           status of the the LocationClientApi function calls.
           <br/>

    @param response: if not LOCATION_RESPONSE_SUCCESS, then the
                   last API call failed. <br/>
*/
typedef std::function<void(
    LocationResponse response
)> ResponseCb;

/** @brief
    LocationCb is for receiving Location when
    LocationClientApi is in a positioning session. <br/>

    When there are multiple engines running on the system, the
    received Location is fused report from all engines. <br/>

    When there is only standard SPE engine running on the
    system, the received Location is from modem GNSS engine.
    <br/>

    @param location: received basic location info. <br/>
*/
typedef std::function<void(
    const Location& location
)> LocationCb;

/** @brief
    GnssLocationCb is for receiving GnssLocation which contains
    richer info than Location when LocationClientApi is
    in a positioning session. <br/>

    When there are multiple engines running on the system, the
    received GnssLocation is fused report from all engines.
    <br/>

    When there is only standard SPE engine running on the
    system, the received GnssLocation is from modem GNSS engine.
    <br/>

    @param gnssLocation: rich GnssLocation. <br/>
*/
typedef std::function<void(
    const GnssLocation& gnssLocation
)> GnssLocationCb;

/** @brief
    EngineLocationsCb is for receiving GnssLocation from
    selected position engines when LocationClientApi is in a
    positioning session. <br/>

    @param engLocations: rich GnssLocation from requested
                       position engines <br/>
*/
typedef std::function<void(
    const std::vector<GnssLocation> & engLocations
)> EngineLocationsCb;


/** @brief
    GnssSvCb is for receiving GnssSv when LocationClientApi is
    in a positioning session.  <br/>

    @param gnssSvs: GNSS SV report. <br/>
*/
typedef std::function<void(
    const std::vector<GnssSv>& gnssSvs
)> GnssSvCb;

/** @brief
    GnssNmeaCb is for receiving NMEA sentences when
    LocationClientApi is in a positioning session. <br/>

    @param timestamp: timestamp that NMEA sentence is
                    generated. <br/>

    @param nmea: nmea strings generated from position and SV
           report. <br/>
*/
typedef std::function<void(
    uint64_t timestamp, const std::string& nmea
)> GnssNmeaCb;

/** @brief
    GnssDataCb is for receiving GnssData, e.g.:
    jammer information when LocationClientApi is in a
    positioning session. <br/>

    @param gnssData: GNSS jammer and agc info. <br/>
*/
typedef std::function<void(
    const GnssData& gnssData
)> GnssDataCb;

/** @brief
    GnssMeasurementsCb is for receiving GnssMeasurements
    information when LocationClientApi is in a positioning
    session. <br/>

    @param gnssMeasurements: GNSS SV measurements. <br/>
*/
typedef std::function<void(
    const GnssMeasurements& gnssMeasurements
)> GnssMeasurementsCb;

/** @brief
    GnssSvPolyCb is for receiving SV polynomial information
    when LocationClientApi is in a positioning session. <br/>

    @param gnssSvPoly: SV polynomial info in a session
*/
typedef std::function<void(
    const GnssSvPoly& gnssSvPoly
)> GnssSvPolyCb;

/** @brief
    LocationSystemInfoCb is for receiving rare occuring location
    system information update as defined in
    LocationSystemInfo. <br/>
    @param locationSystemInfo: rare location system event, e.g.:
           leap second change. <br/>
*/
typedef std::function<void(
    const LocationSystemInfo & locationSystemInfo
)> LocationSystemInfoCb;

/** @brief
    BatchingCb is for delivering locations in a batching
    session.
    @param locations: the locations batched in a session
    @param batchStatus: BatchingStatus of the batching session, which is
        BATCHING_STATUS_INACTIVE when unable to compute positions for batching;
        BATCHING_STATUS_DONE when trip distance has been traversed for tripBatching;
        BATCHING_STATUS_ACTIVE when able to compute positions for batching.
*/
typedef std::function<void(
    const std::vector<Location>& locations,
    BatchingStatus batchStatus
)> BatchingCb;

/** @brief Used by geofence APIs

    @param responses: include Geofence objects and correponding responses.
*/
class Geofence;
typedef std::function<void(
    std::vector<std::pair<Geofence, LocationResponse>>& responses
)> CollectiveResponseCb;

/** @brief
    GeofenceBreachCb is for receiving geofences that have a
    state change.
    @param geofences: array of geofence objects that have breached
    @param location: location associated with breach
    @param type: type of breach
    @param timestamp: timestamp of breach
*/
typedef std::function<void(
    const std::vector<Geofence>& geofences,
    Location location,
    GeofenceBreachTypeMask type,
    uint64_t timestamp
)> GeofenceBreachCb;


/** Specify the set of callbacks to receive the reports when
 *  invoking startPositionSession(uint32_t,
 *  LocReqEngineTypeMask, const GnssReportCbs&, ResponseCb) with
 *  intervalInMs specified. <br/>   */
struct GnssReportCbs {
    /** Callback to receive GnssLocation. <br/>
        When there are multiple engines running on the system, the
        received Location is fused report from all engines. <br/>
        When there is only standard SPE engine running on the
        system, the received Location is from modem GNSS engine. <br/>
      */
    GnssLocationCb gnssLocationCallback;
    /** Callback to receive GnssSv from modem GNSS engine. <br/> */
    GnssSvCb gnssSvCallback;
    /** Callback to receive NMEA sentences. <br/> */
    GnssNmeaCb gnssNmeaCallback;
    /** Callback to receive GnssData from modem GNSS engine.
     *  <br/> */
    GnssDataCb gnssDataCallback;
    /** Callback to receive GnssMeasurements modem GNSS engine. <br/>  */
    GnssMeasurementsCb gnssMeasurementsCallback;
};

/** Specify the set of callbacks to receive the reports when
 *  invoking startPositionSession(uint32_t,
 *  LocReqEngineTypeMask, const EngineReportCbs&, ResponseCb)
 *  with intervalInMs specified. */
struct EngineReportCbs {
    /** Callback to receive GnssLocation from selected engines.
     *  <br/> */
    EngineLocationsCb engLocationsCallback;
    /** Callback to receive GnssSv from modem GNSS engine. <br/> */
    GnssSvCb gnssSvCallback;
    /** Callback to receive NMEA sentences. <br/>
     *  NMEA will be generated from GnssSv and position report.
     *  <br/>
     *  When there are multiple engines running on the system,
     *  position related NMEA sentences will be generated from the
     *  fused position report. <br/>
     *  When there is only SPE engine running on the system,
     *  position related NMEA sentences will be generated from the
     *  position report from modem GNSS engine report. <br/> */
    GnssNmeaCb gnssNmeaCallback;
    /** Callback to receive GnssData from modem GNSS engine.
     *  <br/> */
    GnssDataCb gnssDataCallback;
    /** Callback to receive GnssMeasurements from modem GNSS engine.
     *  <br/> */
    GnssMeasurementsCb gnssMeasurementsCallback;
    /** Callback to receive GnssSvPoly from modem GNSS engine.
     *  <br/> */
    GnssSvPolyCb gnssSvPolyCallback;
};

/**
 *  Specify the valid fields in GnssEnergyConsumedInfo. */
enum GnssEnergyConsumedInfoMask {
    /** GnssEnergyConsumedInfo has valid
     *  GnssEnergyConsumedInfo::totalEnergyConsumedSinceFirstBoot.
     *  <br/>
    */
    ENERGY_CONSUMED_SINCE_FIRST_BOOT_BIT = (1<<0),
};

/** Specify the info regarding energy consumed by GNSS
 *  engine. <br/>   */
struct GnssEnergyConsumedInfo {
    /** Bitwise OR of GnssEnergyConsumedInfoMask to
     *  specify the valid fields in GnssEnergyConsumedInfo. <br/> */
    GnssEnergyConsumedInfoMask flags;

    /** Energy consumed by the modem GNSS engine since device first
     *  ever bootup, in unit of 0.1 milli watt seconds. <br/>
     */
    uint64_t totalEnergyConsumedSinceFirstBoot;
};

/** @fn
    @brief Used to retrieve energy consumed by modem GNSS
           engine as defined in GnssEnergyConsumedInfo. <br/>

    @param gnssEneryConsumed: info regarding energy consumed by
                            modem GNSS engine. <br/>
*/
typedef std::function<void(
    const GnssEnergyConsumedInfo& gnssEneryConsumed
)> GnssEnergyConsumedCb;

class LocationClientApiImpl;
class LocationClientApi
{
public:
    /** @brief
        Creates an instance of LocationClientApi object. <br/>
        If capsCallback is not nullptr, capsCallback will be invoked
        to report the capabilities of the underlying system. <br/>

        @param
        capsCallback: If this callback is not null,
                      location_client::LocationCapabilitiesMask will
                      be reported via this callback after the
                      construction of the object.  <br/>
                      This callback is allowed to be be null. <br/>
    */
    LocationClientApi(CapabilitiesCb capsCallback);

    /** @brief Default destructor */
    virtual ~LocationClientApi();

    /* ================================== POSITIONING ================================== */

    /** @brief Start or update a session with specified
        parameters to receive basic location report in format of
        Location.
        <br/>

        If locationCallback is nullptr, this call is no op. <br/>
        Otherwise, if this API is called for first time or after
        stopPositionSession(), a position session will be started
        with the specified parameters and callbacks. <br/>

        If called during a session (no matter from which
        location_client::startPositionSession()), parameters and
        callbacks will be updated, and the session continues but
        with the new set of parameters and callbacks. <br/>

        @param intervalInMs <br/>
        Time between fixes, or TBF, in milliseconds. <br/>
        The actual interval of reports recieved will be determined
        by the underlying system.  <br/>
        For example, if intervalInMs is specified as 10
        milliseconds, the report interval will be 100 milli-seconds
        if the highest report rate supported by the positioning
        engines is 10Hz.  <br/>
        Also, if there is another application in the system having a
        session with shorter interval, this client may benefit and
        receive reports at that interval. <br/>
        0 to indicate that client is interested in passive
        listening of the registered reports for session initiated by
        other clients. <br/>

        @param distanceInMeters <br/>
        Distance between fixes, in meters. <br/>
        0 to indicate don't care.<br/>
        When distanceInMeters is set to none zero, intervalInMs
        indicates the max latency that position report should be
        reported after the min distance criteria has been met. For
        example device has been static, at UTC time of x
        millisecond, the device starts to move, at UTC time of y
        milliseconds, the device has moved by distanceInMeters. Then
        the location API client shall expect to get a fix no later
        at UTC time of y+intervalInMs milliseconds. <br/>
        The underlying system may have a minimum distance threshold
        (e.g. 1 meter). Effective distance will not be smaller than
        this lower bound. <br/>
        The effective distance may have a granularity level higher
        than 1 m, e.g. 5 m. So distanceInMeters being 59 may be
        honored at 60 m, depending on the system. <br/>
        Where there is anotehr application in they system having a
        session with shorter distance, this client may benefit and
        receive reports at that distance.<br/>

        @param locationCallback
        Callback to receive positions. <br/>

        @param responseCallback
        Callback to receive system responses. <br/>
        This callback is allowed to be null. <br/>

        @return True, if a session is successfully started. If
                responseCallback is not null, it will then will then
                be invoked to deliver the processing status. If the
                processing status is LOCATION_RESPONSE_SUCCESS,
                LocationCb will be invoked to deliver Location.
                <br/>

         @return False, if no session is started, i.e. when
                locationCallback is nullptr. In this case,
                ResponseCb will not be invoked. <br/>
    */
    bool startPositionSession(uint32_t intervalInMs, uint32_t distanceInMeters,
                              LocationCb locationCallback, ResponseCb responseCallback);

    /** @brief Start or update a session to receive rich location
               info in format of GnssLocation and other reports,
               e.g.: SV report and NMEA report.
        If gnssReportCallbacks is nullptr, this call is no op. <br/>
        Otherwise, if this API is called for first time or after
        stopPositionSession(), a position session will be started
        with the specified parameters and callbacks. <br/>
        If this API is called during a session (no matter from which
        startPositionSession() API), parameters and callbacks will
        be updated, and the session continues but with the new set
        of parameters and callbacks. <br/>

        @param intervalInMs <br/>
        Time between fixes, or TBF, in milliseconds. <br/>
        The actual interval of reports recieved will be determined
        by the underlying system. <br/>
        For example, if intervalInMs is specified as 10
        milliseconds, the report interval will be 100 milli-seconds
        if the highest report rate supported by the positioning
        engines is 10Hz.  <br/>
        Also, if there is another application in the system having a
        session with shorter interval, this client may benefit and
        receive reports at that interval. <br/>
        0 to indicate that client is interested in passive
        listening of the registered reports for session initiated by
        other clients. <br/>

        @param gnssReportCallbacks
        Table of callbacks to receive GNSS locations / sv info /
        nmea report / sv measurement report.  <br/>

        @param responseCallback
        Callback to receive system responses. <br/>
        This callback is allowed to be null. <br/>

        @return True, if a session is successfully started. If
                responseCallback is not null, it will then will then
                be invoked to deliver the processing status.
                If the processing status is
                LOCATION_RESPONSE_SUCCESS, GnssReportCb will be
                invoked to deliver registered reports. <br/>

        @return False, if no session is started, i.e. when
                locationCallback is nullptr. In this case,
                ResponseCb will not be invoked. <br/>
    */
    bool startPositionSession(uint32_t intervalInMs,
                              const GnssReportCbs& gnssReportCallbacks,
                              ResponseCb responseCallback);

    /** @brief
        Start or update a session which may provide richer default
        combined poisiton report and position reports from other
        engines in format of GnssLocation together with NMEA, SV
        report, SV measurement reports. <br/>

        If EngineReportCbs is populated with nullptr only, this call
        is no op. Otherwise...<br/>
        If this API is called for first time or after
        stopPositionSession(), a position session will be started
        with the specified parameters and callbacks. <br/>
        If this API is called during a session (no matter from which
        startPositionSession() API), parameters / callback will be
        updated, and the session continues but with the new set of
        parameters / callbacks. <br/>

        @param intervalInMs
        Time between fixes, or TBF, in milliseconds. <br/>
        The actual interval of reports recieved will be determined
        by the underlying system. <br/>
        For example, if intervalInMs is specified as 10
        milliseconds, the report interval will be 100 milli-seconds
        if the highest report rate supported by the positioning
        engines is 10Hz.  <br/>
        Also, if there is another application in the system having a
        session with shorter interval, this client may benefit and
        receive reports at that interval. <br/>
        0 to indicate that client is interested in passive listening
        of the registered reports for session initiated by other
        clients. <br/>

        @param locReqEngineSrcMask <br/>
        The bit maks of the location engines that client is
        interested to receive position report. <br/>
        If LOC_REQ_ENGINE_FUSED_BIT is set, but SPE is the only positioning
        engine running on the system, then the fused position will be the
        unpropagated SPE report. <br/>
        If LOC_REQ_ENGINE_FUSED_BIT is set, and there is at least one
        additional engine running on the system other than SPE engine,
        then the fused position will be the propagated report to
        current time. <br/>
        If LOC_REQ_ENGINE_PPE_BIT is set, but PPE is not enabled on
        the system, then PPE position will not be returned. <br/>
        The SPE and PPE report will be the original reports from the
        position engine without any modification and they will be
        send out to client as soon as avaiable. <br/>

        @param engReportCallbacks
        Table of callbacks to receive engine locations / sv report / nmea
        report/ sv measurement report. <br/>

        @param responseCallback
        Callback to receive system responses. <br/>
        This callback is allowed to be null. <br/>

        @return True, if a session is successfully started. If
                responseCallback is not null, it will then be
                invoked to deliver the processing status. If the
                processing status is LOCATION_RESPONSE_SUCCESS,
                EngineReportCbs will be invoked to deliver
                registered reports. <br/>

        @return False, if no session is started, i.e. when
                locationCallback is nullptr. In this case,
                ResponseCb will not be invoked. <br/>
    */
    bool startPositionSession(uint32_t intervalInMs,
                              LocReqEngineTypeMask locReqEngMask,
                              const EngineReportCbs& engReportCallbacks,
                              ResponseCb responseCallback);

    /** @brief Stop the ongoing positioning session and
     *  de-register the callbacks of previous
     *  startPositionSession. <br/>
     *  If ResponseCb is registerd with previous
     *  startPositionSession(), it will be invoked to deliver the
     *  processing status. <br/> */
    void stopPositionSession();

    /** @example example1:testTrackingApi
    * <pre>
    * <code>
    *    // Sample Code
    * static void onCapabilitiesCb(location_client::LocationCapabilitiesMask mask) {
    *     //...
    * }
    * static void onResponseCb(location_client::LocationResponse response) {
    *     //...
    * }
    * static void onGnssLocationCb(const GnssLocation& location) {
    *     //...
    * }
    *
    * static void onGnssSvCb(const std::vector<location_client::GnssSv>& gnssSvs) {
    *     //...
    * }
    *
    * static void onGnssNmeaCb(uint64_t timestamp, const std::string& nmea) {
    *     //...
    * }
    * void testTrackingApi() {
    *     LocationClientApi *pClient = new LocationClientApi(onCapabilitiesCb);
    *     if (nullptr == pClient) {
    *         LOC_LOGe("failed to create LocationClientApi instance");
    *         return;
    *     }
    *
    *     uint32_t option = 0x111;
    *     uint32_t interval = 1000;
    *     // set callbacks
    *     GnssReportCbs reportcbs;
    *     if (option & 1<<0) {
    *         reportcbs.gnssLocationCallback = GnssLocationCb(onGnssLocationCb);
    *     }
    *     if (option & 1<<1) {
    *         reportcbs.gnssSvCallback = GnssSvCb(onGnssSvCb);
    *     }
    *     if (option & 1<<2) {
    *         reportcbs.gnssNmeaCallback = GnssNmeaCb(onGnssNmeaCb);
    *     }
    *
    *     // start tracking session
    *     pClient->startPositionSession(interval, reportcbs, onResponseCb);
    *     //...
    *     // stop session
    *     pClient->stopPositionSession();
    * </code>
    * </pre>
    */

    /* ================================== BATCHING ================================== */

    /** @brief starts an outdoor trip mode batching session with specified parameters.
        Trip mode batching completes on its own when trip distance is covered.
        The behavior of the call is non contextual. The current state or the history of
        actions does not influence the end result of this call. For example, calling
        this function when idle, or calling this function after another startTripBatchingSession()
        or startRoutineBatchingSession(), or calling this function after stopBatchingSession()
        achieve the same result, which is one of the below:
        If batchingCallback is nullptr, this call is no op. Otherwise...
        If both minInterval and tripDistance are don't care, this call is no op.
           Otherwise...
        If called during a session (no matter from which startTripBatchingSession()/
        startRoutineBatchingSession() API), parameters / callback will be updated,
        and the session continues but with the new set of parameters / callback.
        locations are reported on the batchingCallback in batches when batch is full.
        @param minInterval
        Time between fixes, or TBF, in milliseconds. The actual
        interval of reports recieved will be no larger than
        milliseconds being rounded up the next interval granularity
        supported by the underlying system.
        0 to indicate don't care.
        1)  The underlying system may have a minimum interval threshold
        (e.g. 100 ms or 1000 ms). Effective intervals will not be smaller
        than this lower bound.
        2) The effective intervals may have a granularity level higher
        than 1 ms, e.g. 100 ms or 1000 ms. So milliseconds being 1559
        may be honored at 1600 or 2000 ms, depending on the system.
        3) Where there is anotehr application in they system having a
        session with shorter interval, this client may benefit and
        receive reports at that interval.
        @param tripDistance
        the trip distance from the start of outdoor trip batching; 0 means don't care
        @param batchingCallback
        callback to receive batching positions and status
        @param responseCallback
        callback to receive system responses; optional.
        @return True, if a batching session is successfully started.
                False, if no session is started, i.e. when batchingCallback is nullptr.
    */
    bool startTripBatchingSession(uint32_t minInterval, uint32_t tripDistance,
                                  BatchingCb batchingCallback, ResponseCb responseCallback);

    /** @brief starts a routine mode batching session with specified parameters.
        The behavior of the call is non contextual. The current state or the history of
        actions does not influence the end result of this call. For example, calling
        this function when idle, or calling this function after another startTripBatchingSession()
        or startRoutineBatchingSession(), or calling this function after stopBatchingSession()
        achieve the same result, which is one of the below:
        If batchingCallback is nullptr, this call is no op. Otherwise...
        If both minInterval and minDistance are don't care, this call is no op.
           Otherwise...
        If called during a session (no matter from which startTripBatchingSession()/
        startRoutineBatchingSession() API), parameters / callback will be updated,
        and the session continues but with the new set of parameters / callback.
        locations are reported on the batchingCallback in batches when batch is full.
        @param minInterval
        Time between fixes, or TBF, in milliseconds. The actual
        interval of reports recieved will be no larger than
        milliseconds being rounded up the next interval granularity
        supported by the underlying system.
        0 to indicate don't care.
        1)  The underlying system may have a minimum interval threshold
        (e.g. 100 ms or 1000 ms). Effective intervals will not be smaller
        than this lower bound.
        2) The effective intervals may have a granularity level higher
        than 1 ms, e.g. 100 ms or 1000 ms. So milliseconds being 1559
        may be honored at 1600 or 2000 ms, depending on the system.
        3) Where there is anotehr application in they system having a
        session with shorter interval, this client may benefit and
        receive reports at that interval.
        @param minDistance
        specifies the minimum distance in unit of meters that should
        be traversed before a position should be batched.
        If 0, the positions are batched after the minInterval period expires.
        If both minInterval and minDistance are specified, the position are batched only after
        minInterval has expired AND minDistance has been traversed.
        @param batchingCallback
        callback to receive batching positions and status
        @param responseCallback
        callback to receive system responses; optional.
        @return True, if a batching session is successfully started.
                False, if no session is started, i.e. when batchingCallback is nullptr.
    */
    bool startRoutineBatchingSession(uint32_t minInterval, uint32_t minDistance,
                                     BatchingCb batchingCallback, ResponseCb responseCallback);
    /** @brief Stops the batching session.
    */
    void stopBatchingSession();

    /** @example example2:testBatchingApi
    * <pre>
    * <code>
    *    // Sample Code
    * static void onBatchingCb(const std::vector<location_client::Location>& locations,
    *         BatchingStatus status) {
    *     //...
    * }
    * void testBatchingApi() {
    *     // batching session
    *     uint32_t intervalInMs = 0;
    *     uint32_t distanceInMeters = 0;
    *     pClient->startRoutineBatchingSession(
    *             intervalInMs, distanceInMeters, onBatchingCb, onResponseCb);
    *     // ...
    *     pClient->stopBatchingSession();
    * }
    * </code>
    * </pre>
    */

    /* ================================== Geofence ================================== */
    /** @brief Adds any number of geofences. The geofenceBreachCallback will
        deliver the status of each geofence according to the Geofence parameter for each.
        @param geofences
        Geofence objects, Once addGeofences returns, each Geofence object in the vector would
        be the identifier throughout the remaining communication of that geofence.
        Such a Geofence object can be copied or cloned, but they would all reference
        the same geofence.
        @param gfBreachCb
        callback to receive geofences state change. addGeofences is no op if gfBreachCb is null
        @param responseCallback
        callback to receive geofence ids and system responses; optional.
    */
    void addGeofences(std::vector<Geofence>& geofences, GeofenceBreachCb gfBreachCb,
                      CollectiveResponseCb responseCallback);

    /** @brief Removes any number of geofences.
        @param geofences
        Geofence objects, must be originally added to the system. Otherwise it would be no op.
    */
    void removeGeofences(std::vector<Geofence>& geofences);

    /** @brief Modifies any number of geofences.
        @param geofences
        Geofence objects, must be originally added to the system. Otherwise it would be no op.
        Modifiable fields include breachTypeMask, responsiveness and dwelltime.
        A geofence that has been added to the system may have these fields modified.
        But it is not going to take any effect until modifyGeofences is called with
        the changed geofence passed in.
    */
    void modifyGeofences(std::vector<Geofence>& geofences);

    /** @brief Pauses any number of geofences, which is similar to removeGeofences,
        only that they can be resumed at any time.
        @param geofences
        Geofence objects, must be originally added to the system. Otherwise it would be no op.
    */
    void pauseGeofences(std::vector<Geofence>& geofences);

    /** @brief Resumes any number of geofences that are currently paused.
        @param geofences
        Geofence objects, must be originally added to the system. Otherwise it would be no op.
    */
    void resumeGeofences(std::vector<Geofence>& geofences);

    /** @example example3:testGeofenceApi
    * <pre>
    * <code>
    *    // Sample Code
    *
    * vector<Geofence> sGeofences;
    * static void onGeofenceBreachCb( const std::vector<Geofence>& geofences,
    *         Location location, GeofenceBreachTypeMask type,
    *         uint64_t timestamp) {
    *     //...
    * }
    * static void onCollectiveResponseCb(std::vector<pair<Geofence, LocationResponse>>& responses) {
    *     //...
    * }
    *
    * void testGeofenceApi() {
    *     double latitude = 32.896535;
    *     double longitude = -117.201025;
    *     double radius = 50;
    *     GeofenceBreachTypeMask type = (GeofenceBreachTypeMask)3;
    *     uint32_t responsiveness = 4000;
    *     uint32_t time = 0;
    *     Geofence gf(latitude, longitude, radius, type, responsiveness, time);
    *     sGeofences.push_back(gf);
    *
    *     pClient->addGeofences(sGeofences, onGeofenceBreachCb, onCollectiveResponseCb);
    *     vector<Geofence> pauseGeofences;
    *     pauseGeofences.push_back(sGeofences[0]);
    *     pClient->pauseGeofences(pauseGeofences);
    *     vector<Geofence> resumeGeofences;
    *     resumeGeofences.push_back(sGeofences[0]);
    *     pClient->resumeGeofences(resumeGeofences);
    *     vector<Geofence> removeGeofences;
    *     for (int i=0; i<sGeofences.size(); ++i) {
    *         removeGeofences.push_back(sGeofences[i]);
    *     }
    *     pClient->removeGeofences(removeGeofences);
    * }
    * </code>
    * </pre>
    */

    /* ================================== Other APIs ================================== */
    /** @brief Inform LocationClientAPI of the device network
               availability status.
        @param available
        True if available. <br/>
        False otherwise. <br/>  */
    void updateNetworkAvailability(bool available);

    /** @brief Get energy consumed info of modem GNSS engine. <br/>
        If called while the previous call is still being processed,
        then the callback will be updated, and engery consumed info
        will be delivered via the new callbacks. <br/>

        @param gnssEnergyConsumedCallback
        Callback to receive energy consumed info by modem GNSS
        engine. <br/>
        @param responseCallback
        Callback to receive the processing status of this API call.
        <br/>
        This callback is allowed to be null. <br/>

        If the procesing status is LOCATION_RESPONSE_SUCCESS,
        GnssEnergyConsumedCb will be invoked shortly to deliver the
        energy consumed info. <br/> */
    void getGnssEnergyConsumed(GnssEnergyConsumedCb gnssEnergyConsumedCallback,
                               ResponseCb responseCallback);

    /** @brief
        Register/update listener to receive location system info
        that are not tied with positioning session, e.g.: next leap
        second event. <br/>
        One set of callbacks can be registered per instance of
        LocationClientApi.  <br/>
        The callback may be invoked multiple times to update same or
        different pieces of system info.  <br/>

        @param locSystemInfoCallback
        Callback to receive system info update. <br/>
        Pass a null callback to stop receiving the update. <br/>

        @param responseCallback
        Callback to receive the processing status of this API
        call. <br/>
        This callback is allowed to be null. <br>
        */
    void updateLocationSystemInfoListener(LocationSystemInfoCb locSystemInfoCallback,
                                          ResponseCb responseCallback);

private:
    /** Internal implementation for LocationClientApi */
    LocationClientApiImpl* mApiImpl;
};

class GeofenceImpl;
class Geofence {
    friend class GeofenceImpl;
    friend class LocationClientApi;
    std::shared_ptr<GeofenceImpl> mGeofenceImpl;
    double mLatitude;
    double mLongitude;
    double mRadius;
    GeofenceBreachTypeMask mBreachType;
    uint32_t mResponsiveness;
    uint32_t mDwellTime;
public:
    virtual ~Geofence() {}
    inline Geofence(double lat, double lon, double r, GeofenceBreachTypeMask type,
            uint32_t responsiveness, uint32_t time) :
            mGeofenceImpl(nullptr), mLatitude(lat), mLongitude(lon), mRadius(r),
            mBreachType(type), mResponsiveness(responsiveness), mDwellTime(time) {}
    inline Geofence(const Geofence& geofence): mGeofenceImpl(geofence.mGeofenceImpl),
            mLatitude(geofence.mLatitude), mLongitude(geofence.mLongitude),
            mRadius(geofence.mRadius), mBreachType(geofence.mBreachType),
            mResponsiveness(geofence.mResponsiveness), mDwellTime(geofence.mDwellTime) {}
    // clients who extends Geofence class should implement this clone method, so that its own
    // data stashed in the object gets copied to the new object, if desired so.
    virtual Geofence* clone() { return new Geofence(*this); }
    bool operator==(Geofence& other);
    inline double getLatitude() const { return mLatitude; }
    inline double getLongitude() const { return mLongitude; }
    inline double getRadius() const { return mRadius; }
    inline GeofenceBreachTypeMask getBreachType() const { return mBreachType; }
    inline uint32_t getResponsiveness() const { return mResponsiveness; }
    inline uint32_t getDwellTime() const { return mDwellTime; }
    inline void setBreachType(GeofenceBreachTypeMask type) { mBreachType = type; }
    inline void setResponsiveness(uint32_t responsiveness) { mResponsiveness = responsiveness; }
    inline void setDwellTime(uint32_t time) { mDwellTime = time; }
};

} // namespace location_client

#endif /* LOCATIONCLIENTAPI_H */
