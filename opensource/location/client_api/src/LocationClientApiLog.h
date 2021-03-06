
/* Copyright (c) 2019-2020 The Linux Foundation. All rights reserved.
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

#ifndef LOCATION_CLIENT_API_LOG_H
#define LOCATION_CLIENT_API_LOG_H

#include <stdint.h>
#include <time.h>
#include <comdef.h>
#include <log.h>
#include <log_codes.h>
#include "LocationClientApi.h"

#define CLIENT_DIAG_GNSS_SV_MAX            (176)
#define CLIENT_DIAG_GNSS_MEASUREMENTS_MAX  (128)
#define LOG_CLIENT_LOCATION_DIAG_MSG_VERSION        (1)
#define LOG_CLIENT_SV_REPORT_DIAG_MSG_VERSION       (2)
#define LOG_CLIENT_NMEA_REPORT_DIAG_MSG_VERSION     (1)
#define LOG_CLIENT_MEASUREMENTS_DIAG_MSG_VERSION    (1)
#define LOG_CLIENT_SV_POLY_DIAG_MSG_VERSION         (1)

#ifndef LOG_GNSS_CLIENT_API_NMEA_REPORT_C
#define LOG_GNSS_CLIENT_API_NMEA_REPORT_C (0x1CB2)
#endif

#ifndef LOG_GNSS_CLIENT_API_MEASUREMENTS_REPORT_C
#define LOG_GNSS_CLIENT_API_MEASUREMENTS_REPORT_C (0x1CB7)
#endif

#ifndef LOG_GNSS_CLIENT_API_SV_POLY_REPORT_C
#define LOG_GNSS_CLIENT_API_SV_POLY_REPORT_C (0x1CC7)
#endif

namespace location_client
{

/** Flags to indicate which values are valid in a Location */
typedef uint16_t clientDiagLocationFlagsMask;
typedef enum {
    /** location has valid latitude and longitude */
    CLIENT_DIAG_LOCATION_HAS_LAT_LONG_BIT          = (1<<0),
    /** location has valid altitude */
    CLIENT_DIAG_LOCATION_HAS_ALTITUDE_BIT          = (1<<1),
    /** location has valid speed */
    CLIENT_DIAG_LOCATION_HAS_SPEED_BIT             = (1<<2),
    /** location has valid bearing */
    CLIENT_DIAG_LOCATION_HAS_BEARING_BIT           = (1<<3),
    /** location has valid accuracy */
    CLIENT_DIAG_LOCATION_HAS_ACCURACY_BIT          = (1<<4),
    /** location has valid vertical accuracy */
    CLIENT_DIAG_LOCATION_HAS_VERTICAL_ACCURACY_BIT = (1<<5),
    /** location has valid speed accuracy */
    CLIENT_DIAG_LOCATION_HAS_SPEED_ACCURACY_BIT    = (1<<6),
    /** location has valid bearing accuracy */
    CLIENT_DIAG_LOCATION_HAS_BEARING_ACCURACY_BIT  = (1<<7),
    /** location has valid valid timestamp  */
    CLIENT_DIAG_LOCATION_HAS_TIMESTAMP_BIT         = (1<<8)
} clientDiagLocationFlagsBits;

typedef uint32_t clientDiagGnssLocationPosDataMask;
typedef enum {
    /** Navigation data has Forward Acceleration */
    CLIENT_DIAG_LOCATION_NAV_DATA_HAS_LONG_ACCEL_BIT  = (1<<0),
    /** Navigation data has Sideward Acceleration */
    CLIENT_DIAG_LOCATION_NAV_DATA_HAS_LAT_ACCEL_BIT   = (1<<1),
    /** Navigation data has Vertical Acceleration */
    CLIENT_DIAG_LOCATION_NAV_DATA_HAS_VERT_ACCEL_BIT  = (1<<2),
    /** Navigation data has Heading Rate */
    pLIENT_DIAG_LOCATION_NAV_DATA_HAS_YAW_RATE_BIT    = (1<<3),
    /** Navigation data has Body pitch */
    CLIENT_DIAG_LOCATION_NAV_DATA_HAS_PITCH_BIT       = (1<<4),
    /** Navigation data has Forward Acceleration uncertainty */
    CLIENT_DIAG_LOCATION_NAV_DATA_HAS_LONG_ACCEL_UNC_BIT = (1<<5),
    /** Navigation data has Sideward Acceleration uncertainty */
    CLIENT_DIAG_LOCATION_NAV_DATA_HAS_LAT_ACCEL_UNC_BIT  = (1<<6),
    /** Navigation data has Vertical Acceleration uncertainty */
    CLIENT_DIAG_LOCATION_NAV_DATA_HAS_VERT_ACCEL_UNC_BIT = (1<<7),
    /** Navigation data has Heading Rate uncertainty */
    CLIENT_DIAG_LOCATION_NAV_DATA_HAS_YAW_RATE_UNC_BIT   = (1<<8),
    /** Navigation data has Body pitch uncertainty */
    CLIENT_DIAG_LOCATION_NAV_DATA_HAS_PITCH_UNC_BIT      = (1<<9)
} clientDiagGnssLocationPosDataBits;

typedef enum
{
    /** GPS satellite. */
    CLIENT_DIAG_GNSS_LOC_SV_SYSTEM_GPS                    = 1,
    /** GALILEO satellite. */
    CLIENT_DIAG_GNSS_LOC_SV_SYSTEM_GALILEO                = 2,
    /** SBAS satellite. */
    CLIENT_DIAG_GNSS_LOC_SV_SYSTEM_SBAS                   = 3,
    //Leave 4 blank for backward compatibility.
    /** GLONASS satellite. */
    CLIENT_DIAG_GNSS_LOC_SV_SYSTEM_GLONASS                = 5,
    /** BDS satellite. */
    CLIENT_DIAG_GNSS_LOC_SV_SYSTEM_BDS                    = 6,
    /** QZSS satellite. */
    CLIENT_DIAG_GNSS_LOC_SV_SYSTEM_QZSS                   = 7
} clientDiagGnss_LocSvSystemEnumType;

typedef uint32_t clientDiagGnssSystemTimeStructTypeFlags;
typedef enum {
    CLIENT_DIAG_GNSS_SYSTEM_TIME_WEEK_VALID             = (1 << 0),
    CLIENT_DIAG_GNSS_SYSTEM_TIME_WEEK_MS_VALID          = (1 << 1),
    CLIENT_DIAG_GNSS_SYSTEM_CLK_TIME_BIAS_VALID         = (1 << 2),
    CLIENT_DIAG_GNSS_SYSTEM_CLK_TIME_BIAS_UNC_VALID     = (1 << 3),
    CLIENT_DIAG_GNSS_SYSTEM_REF_FCOUNT_VALID            = (1 << 4),
    CLIENT_DIAG_GNSS_SYSTEM_NUM_CLOCK_RESETS_VALID      = (1 << 5)
} clientDiagGnssSystemTimeTypeBits;

typedef uint32_t clientDiagGnssGloTimeStructTypeFlags;
typedef enum {
    CLIENT_DIAG_GNSS_CLO_DAYS_VALID                     = (1 << 0),
    CLIENT_DIAG_GNSS_GLOS_MSEC_VALID                    = (1 << 1),
    CLIENT_DIAG_GNSS_GLO_CLK_TIME_BIAS_VALID            = (1 << 2),
    CLIENT_DIAG_GNSS_GLO_CLK_TIME_BIAS_UNC_VALID        = (1 << 3),
    CLIENT_DIAG_GNSS_GLO_REF_FCOUNT_VALID               = (1 << 4),
    CLIENT_DIAG_GNSS_GLO_NUM_CLOCK_RESETS_VALID         = (1 << 5),
    CLIENT_DIAG_GNSS_GLO_FOUR_YEAR_VALID                = (1 << 6)
} clientDiagGnssGloTimeTypeBits;

typedef PACKED struct PACKED_POST {
    /** Validity mask for below fields */
    clientDiagGnssSystemTimeStructTypeFlags validityMask;
    /** Extended week number at reference tick.
    Unit: Week.
    Set to 65535 if week number is unknown.
    For GPS:
      Calculated from midnight, Jan. 6, 1980.
      OTA decoded 10 bit GPS week is extended to map between:
      [NV6264 to (NV6264 + 1023)].
      NV6264: Minimum GPS week number configuration.
      Default value of NV6264: 1738
    For BDS:
      Calculated from 00:00:00 on January 1, 2006 of Coordinated Universal Time (UTC).
    For GAL:
      Calculated from 00:00 UT on Sunday August 22, 1999 (midnight between August 21 and August 22).
   */
    uint16_t systemWeek;
    /** Time in to the current week at reference tick.
       Unit: Millisecond. Range: 0 to 604799999.
       Check for systemClkTimeUncMs before use */
    uint32_t systemMsec;
    /** System clock time bias (sub-millisecond)
        Units: Millisecond
        Note: System time (TOW Millisecond) = systemMsec - systemClkTimeBias.
        Check for systemClkTimeUncMs before use. */
    float systemClkTimeBias;
    /** Single sided maximum time bias uncertainty
        Units: Millisecond */
    float systemClkTimeUncMs;
    /** FCount (free running HW timer) value. Don't use for relative time purpose
         due to possible discontinuities.
         Unit: Millisecond */
    uint32_t refFCount;
    /** Number of clock resets/discontinuities detected,
        affecting the local hardware counter value. */
    uint32_t numClockResets;
} clientDiagGnssSystemTimeStructType;

typedef PACKED struct PACKED_POST {
    /** GLONASS day number in four years. Refer to GLONASS ICD.
        Applicable only for GLONASS and shall be ignored for other constellations.
        If unknown shall be set to 65535 */
    uint16_t gloDays;
    /** Validity mask for below fields */
    clientDiagGnssGloTimeStructTypeFlags validityMask;
    /** GLONASS time of day in Millisecond. Refer to GLONASS ICD.
        Units: Millisecond
        Check for gloClkTimeUncMs before use */
    uint32_t gloMsec;
    /** GLONASS clock time bias (sub-millisecond)
        Units: Millisecond
        Note: GLO time (TOD Millisecond) = gloMsec - gloClkTimeBias.
        Check for gloClkTimeUncMs before use. */
    float gloClkTimeBias;
    /** Single sided maximum time bias uncertainty
        Units: Millisecond */
    float gloClkTimeUncMs;
    /** FCount (free running HW timer) value. Don't use for relative time purpose
        due to possible discontinuities.
        Unit: Millisecond */
    uint32_t  refFCount;
    /** Number of clock resets/discontinuities detected,
        affecting the local hardware counter value. */
    uint32_t numClockResets;
    /** GLONASS four year number from 1996. Refer to GLONASS ICD.
        Applicable only for GLONASS and shall be ignored for other constellations.
        If unknown shall be set to 255 */
    uint8_t gloFourYear;
} clientDiagGnssGloTimeStructType;

typedef PACKED union PACKED_POST {
    clientDiagGnssSystemTimeStructType gpsSystemTime;
    clientDiagGnssSystemTimeStructType galSystemTime;
    clientDiagGnssSystemTimeStructType bdsSystemTime;
    clientDiagGnssSystemTimeStructType qzssSystemTime;
    clientDiagGnssGloTimeStructType gloSystemTime;
} clientDiagSystemTimeStructUnion;

typedef PACKED struct PACKED_POST {
    /** Contains Body frame LocPosDataMask bits. */
    clientDiagGnssLocationPosDataMask  bodyFrameDataMask;
    /** Forward Acceleration in body frame (m/s2)*/
    float           longAccel;
    /** Sideward Acceleration in body frame (m/s2)*/
    float           latAccel;
    /** Vertical Acceleration in body frame (m/s2)*/
    float           vertAccel;
    /** Heading Rate (Radians/second) */
    float           yawRate;
    /** Body pitch (Radians) */
    float           pitch;
    /** Uncertainty of Forward Acceleration in body frame */
    float           longAccelUnc;
    /** Uncertainty of Side-ward Acceleration in body frame */
    float           latAccelUnc;
    /** Uncertainty of Vertical Acceleration in body frame */
    float           vertAccelUnc;
    /** Uncertainty of Heading Rate */
    float           yawRateUnc;
    /** Uncertainty of Body pitch */
    float           pitchUnc;
} clientDiagGnssLocationPositionDynamics;

/** @struct
    Time applicability of PVT report
*/
typedef PACKED struct PACKED_POST {
    /** Specifies GNSS system time reported. Mandatory field */
    clientDiagGnss_LocSvSystemEnumType gnssSystemTimeSrc;
    /** Reporting of GPS system time is recommended.
      If GPS time is unknown & other satellite system time is known,
      it should be reported.
      Mandatory field
     */
    clientDiagSystemTimeStructUnion u;
} clientDiagGnssSystemTime;

/** GNSS Signal Type and RF Band */
typedef uint32_t clientDiagGnssSignalTypeMask;
typedef enum {
    /** GPS L1CA Signal */
    CLIENT_DIAG_GNSS_SIGNAL_GPS_L1CA            = (1<<0),
    /** GPS L1C Signal */
    CLIENT_DIAG_GNSS_SIGNAL_GPS_L1C             = (1<<1),
    /** GPS L2 RF Band */
    CLIENT_DIAG_GNSS_SIGNAL_GPS_L2              = (1<<2),
    /** GPS L5 RF Band */
    CLIENT_DIAG_GNSS_SIGNAL_GPS_L5              = (1<<3),
    /** GLONASS G1 (L1OF) RF Band */
    CLIENT_DIAG_GNSS_SIGNAL_GLONASS_G1          = (1<<4),
    /** GLONASS G2 (L2OF) RF Band */
    CLIENT_DIAG_GNSS_SIGNAL_GLONASS_G2          = (1<<5),
    /** GALILEO E1 RF Band */
    CLIENT_DIAG_GNSS_SIGNAL_GALILEO_E1          = (1<<6),
    /** GALILEO E5A RF Band */
    CLIENT_DIAG_GNSS_SIGNAL_GALILEO_E5A         = (1<<7),
    /** GALILEO E5B RF Band */
    CLIENT_DIAG_GNSS_SIGNAL_GALILEO_E5B         = (1<<8),
    /** BEIDOU B1 RF Band */
    CLIENT_DIAG_GNSS_SIGNAL_BEIDOU_B1           = (1<<9),
    /** BEIDOU B2 RF Band */
    CLIENT_DIAG_GNSS_SIGNAL_BEIDOU_B2           = (1<<10),
    /** QZSS L1CA RF Band */
    CLIENT_DIAG_GNSS_SIGNAL_QZSS_L1CA           = (1<<11),
    /** QZSS L1S RF Band */
    CLIENT_DIAG_GNSS_SIGNAL_QZSS_L1S            = (1<<12),
    /** QZSS L2 RF Band */
    CLIENT_DIAG_GNSS_SIGNAL_QZSS_L2             = (1<<13),
    /** QZSS L5 RF Band */
    CLIENT_DIAG_GNSS_SIGNAL_QZSS_L5             = (1<<14),
    /** SBAS L1 RF Band */
    CLIENT_DIAG_GNSS_SIGNAL_SBAS_L1             = (1<<15),
    /** BEIDOU B1I RF Band */
    CLIENT_DIAG_GNSS_SIGNAL_BEIDOU_B1I          = (1<<16),
    /** BEIDOU B1C RF Band */
    CLIENT_DIAG_GNSS_SIGNAL_BEIDOU_B1C          = (1<<17),
    /** BEIDOU B2I RF Band */
    CLIENT_DIAG_GNSS_SIGNAL_BEIDOU_B2I          = (1<<18),
    /** BEIDOU B2AI RF Band */
    CLIENT_DIAG_GNSS_SIGNAL_BEIDOU_B2AI         = (1<<19),
    /** NAVIC L5 RF Band */
    CLIENT_DIAG_GNSS_SIGNAL_NAVIC_L5            = (1<<20),
    /** BEIDOU B2AQ RF Band */
    CLIENT_DIAG_GNSS_SIGNAL_BEIDOU_B2AQ         = (1<<21)
} clientDiagGnssSignalTypeBits;

typedef PACKED struct PACKED_POST {
    /** GnssSignalType mask */
    clientDiagGnssSignalTypeMask gnssSignalType;
   /** Specifies GNSS Constellation Type */
    clientDiagGnss_LocSvSystemEnumType gnssConstellation;
    /**  GNSS SV ID.
     For GPS:      1 to 32
     For GLONASS:  65 to 96. When slot-number to SV ID mapping is unknown, set as 255.
     For SBAS:     120 to 151
     For QZSS-L1CA:193 to 197
     For BDS:      201 to 237
     For GAL:      301 to 336 */
    uint16_t gnssSvId;
} clientDiagGnssMeasUsageInfo;

typedef enum {
    CLIENT_DIAG_GNSS_SV_TYPE_UNKNOWN = 0,
    CLIENT_DIAG_GNSS_SV_TYPE_GPS,
    CLIENT_DIAG_GNSS_SV_TYPE_SBAS,
    CLIENT_DIAG_GNSS_SV_TYPE_GLONASS,
    CLIENT_DIAG_GNSS_SV_TYPE_QZSS,
    CLIENT_DIAG_GNSS_SV_TYPE_BEIDOU,
    CLIENT_DIAG_GNSS_SV_TYPE_GALILEO,
} clientDiagGnssSvType;

typedef uint16_t clientDiagGnssSvOptionsMask;
typedef enum {
    CLIENT_DIAG_GNSS_SV_OPTIONS_HAS_EPHEMER_BIT = (1<<0),
    CLIENT_DIAG_GNSS_SV_OPTIONS_HAS_ALMANAC_BIT = (1<<1),
    CLIENT_DIAG_GNSS_SV_OPTIONS_USED_IN_FIX_BIT = (1<<2),
    CLIENT_DIAG_GNSS_SV_OPTIONS_HAS_CARRIER_FREQUENCY_BIT = (1<<3)
} clientDiagGnssSvOptionsBits;

typedef PACKED struct PACKED_POST {
    /** Unique Identifier */
    uint16_t svId;
    /** type of SV (GPS, SBAS, GLONASS, QZSS, BEIDOU, GALILEO) */
    clientDiagGnssSvType type;
    /** signal strength */
    float cN0Dbhz;
    /** elevation of SV (in degrees) */
    float elevation;
    /** azimuth of SV (in degrees) */
    float azimuth;
    /** Bitwise OR of GnssSvOptionsBits */
    clientDiagGnssSvOptionsMask gnssSvOptionsMask;
    /** carrier frequency of the signal tracked */
    float carrierFrequencyHz;
    /** Specifies GNSS signal type */
    clientDiagGnssSignalTypeMask gnssSignalTypeMask;
} clientDiagGnssSv;

typedef uint16_t clientDiagLocationTechnologyMask;
typedef enum {
    /** location was calculated using GNSS */
    CLIENT_DIAG_LOCATION_TECHNOLOGY_GNSS_BIT     = (1<<0),
    /** location was calculated using Cell */
    CLIENT_DIAG_LOCATION_TECHNOLOGY_CELL_BIT     = (1<<1),
    /** location was calculated using WiFi */
    CLIENT_DIAG_LOCATION_TECHNOLOGY_WIFI_BIT     = (1<<2),
    /** location was calculated using Sensors */
    CLIENT_DIAG_LOCATION_TECHNOLOGY_SENSORS_BIT  = (1<<3),
} clientDiagLocationTechnologyBits;

typedef uint16_t clientDiagDrCalibrationStatusMask;
typedef enum {
    /** Indicate that roll calibration is needed. Need to take more
     *  turns on level ground */
    CLIENT_DIAG_DR_ROLL_CALIBRATION_NEEDED  = (1<<0),
    /** Indicate that pitch calibration is needed. Need to take more
     *  turns on level ground */
    CLIENT_DIAG_DR_PITCH_CALIBRATION_NEEDED = (1<<1),
    /** Indicate that yaw calibration is needed. Need to accelerate
     *  in a straight line  */
    CLIENT_DIAG_DR_YAW_CALIBRATION_NEEDED   = (1<<2),
    /** Indicate that odo calibration is needed. Need to accelerate
     *  in a straight line  */
    CLIENT_DIAG_DR_ODO_CALIBRATION_NEEDED   = (1<<3),
    /** Indicate that gyro calibration is needed. Need to take more
     *  turns on level ground */
    CLIENT_DIAG_DR_GYRO_CALIBRATION_NEEDED  = (1<<4)
} clientDiagDrCalibrationStatusBits;

typedef uint32_t clientDiagGnssLocationInfoFlagMask;
typedef enum {
    /** valid altitude mean sea level */
    CLIENT_DIAG_GNSS_LOCATION_INFO_ALTITUDE_MEAN_SEA_LEVEL_BIT      = (1<<0),
    /** valid pdop, hdop, and vdop */
    CLIENT_DIAG_GNSS_LOCATION_INFO_DOP_BIT                          = (1<<1),
    /** valid magnetic deviation */
    CLIENT_DIAG_GNSS_LOCATION_INFO_MAGNETIC_DEVIATION_BIT           = (1<<2),
    /** valid horizontal reliability */
    CLIENT_DIAG_GNSS_LOCATION_INFO_HOR_RELIABILITY_BIT              = (1<<3),
    /** valid vertical reliability */
    CLIENT_DIAG_GNSS_LOCATION_INFO_VER_RELIABILITY_BIT              = (1<<4),
    /** valid elipsode semi major */
    CLIENT_DIAG_GNSS_LOCATION_INFO_HOR_ACCURACY_ELIP_SEMI_MAJOR_BIT = (1<<5),
    /** valid elipsode semi minor */
    CLIENT_DIAG_GNSS_LOCATION_INFO_HOR_ACCURACY_ELIP_SEMI_MINOR_BIT = (1<<6),
    /** valid accuracy elipsode azimuth */
    CLIENT_DIAG_GNSS_LOCATION_INFO_HOR_ACCURACY_ELIP_AZIMUTH_BIT    = (1<<7),
    /** valid gnss sv used in pos data */
    CLIENT_DIAG_GNSS_LOCATION_INFO_GNSS_SV_USED_DATA_BIT            = (1<<8),
    /** valid navSolutionMask */
    CLIENT_DIAG_GNSS_LOCATION_INFO_NAV_SOLUTION_MASK_BIT            = (1<<9),
    /** valid LocPosTechMask */
    CLIENT_DIAG_GNSS_LOCATION_INFO_POS_TECH_MASK_BIT                = (1<<10),
    /** valid LocSvInfoSource */
    CLIENT_DIAG_GNSS_LOCATION_INFO_SV_SOURCE_INFO_BIT               = (1<<11),
    /** valid position dynamics data */
    CLIENT_DIAG_GNSS_LOCATION_INFO_POS_DYNAMICS_DATA_BIT            = (1<<12),
    /** valid gdop, tdop */
    CLIENT_DIAG_GNSS_LOCATION_INFO_EXT_DOP_BIT                      = (1<<13),
    /** valid North standard deviation */
    CLIENT_DIAG_GNSS_LOCATION_INFO_NORTH_STD_DEV_BIT                = (1<<14),
    /** valid East standard deviation */
    CLIENT_DIAG_GNSS_LOCATION_INFO_EAST_STD_DEV_BIT                 = (1<<15),
    /** valid North Velocity */
    CLIENT_DIAG_GNSS_LOCATION_INFO_NORTH_VEL_BIT                    = (1<<16),
    /** valid East Velocity */
    CLIENT_DIAG_GNSS_LOCATION_INFO_EAST_VEL_BIT                     = (1<<17),
    /** valid Up Velocity */
    CLIENT_DIAG_GNSS_LOCATION_INFO_UP_VEL_BIT                       = (1<<18),
    /** valid North Velocity Uncertainty */
    CLIENT_DIAG_GNSS_LOCATION_INFO_NORTH_VEL_UNC_BIT                = (1<<19),
    /** valid East Velocity Uncertainty */
    CLIENT_DIAG_GNSS_LOCATION_INFO_EAST_VEL_UNC_BIT                 = (1<<20),
    /** valid Up Velocity Uncertainty */
    CLIENT_DIAG_GNSS_LOCATION_INFO_UP_VEL_UNC_BIT                   = (1<<21),
    /** valid leap seconds */
    CLIENT_DIAG_GNSS_LOCATION_INFO_LEAP_SECONDS_BIT                 = (1<<22),
    /** valid time uncertainty */
    CLIENT_DIAG_GNSS_LOCATION_INFO_TIME_UNC_BIT                     = (1<<23),
    /** valid numSvUsedInPosition */
    CLIENT_DIAG_GNSS_LOCATION_INFO_NUM_SV_USED_IN_POSITION_BIT      = (1<<24),
    /** valid calibrationConfidencePercent */
    CLIENT_DIAG_GNSS_LOCATION_INFO_CALIBRATION_CONFIDENCE_PERCENT_BIT = (1<<25),
    /** valid calibrationStatus */
    CLIENT_DIAG_GNSS_LOCATION_INFO_CALIBRATION_STATUS_BIT           = (1<<26),
    /** valid output engine type */
    CLIENT_DIAG_GNSS_LOCATION_INFO_OUTPUT_ENG_TYPE_BIT              = (1<<27),
    /** valid output engine mask */
    CLIENT_DIAG_GNSS_LOCATION_INFO_OUTPUT_ENG_MASK_BIT              = (1<<28),
    /** valid output conformityIndex */
    CLIENT_DIAG_GNSS_LOCATION_INFO_CONFORMITY_INDEX_BIT             = (1<<29),
} clientDiagGnssLocationInfoFlagBits;

typedef enum {
    CLIENT_DIAG_LOCATION_RELIABILITY_NOT_SET = 0,
    CLIENT_DIAG_LOCATION_RELIABILITY_VERY_LOW,
    CLIENT_DIAG_LOCATION_RELIABILITY_LOW,
    CLIENT_DIAG_LOCATION_RELIABILITY_MEDIUM,
    CLIENT_DIAG_LOCATION_RELIABILITY_HIGH,
} clientDiagLocationReliability;

typedef PACKED struct PACKED_POST {
    uint64_t gpsSvUsedIdsMask;
    uint64_t gloSvUsedIdsMask;
    uint64_t galSvUsedIdsMask;
    uint64_t bdsSvUsedIdsMask;
    uint64_t qzssSvUsedIdsMask;
} clientDiagGnssLocationSvUsedInPosition;

typedef uint32_t clientDiagGnssLocationNavSolutionMask;
typedef enum {
    /** SBAS ionospheric correction is used*/
    CLIENT_DIAG_LOCATION_SBAS_CORRECTION_IONO_BIT  = (1<<0),
    /** SBAS fast correction is used*/
    CLIENT_DIAG_LOCATION_SBAS_CORRECTION_FAST_BIT  = (1<<1),
    /** SBAS long-tem correction is used*/
    CLIENT_DIAG_LOCATION_SBAS_CORRECTION_LONG_BIT  = (1<<2),
    /** SBAS integrity information is used*/
    CLIENT_DIAG_LOCATION_SBAS_INTEGRITY_BIT        = (1<<3),
    /** Position Report is DGNSS corrected*/
    CLIENT_DIAG_LOCATION_NAV_CORRECTION_DGNSS_BIT  = (1<<4),
    /** Position Report is RTK corrected*/
    CLIENT_DIAG_LOCATION_NAV_CORRECTION_RTK_BIT    = (1<<5),
    /** Position Report is PPP corrected*/
    CLIENT_DIAG_LOCATION_NAV_CORRECTION_PPP_BIT    = (1<<6)
} clientDiagGnssLocationNavSolutionBits;

typedef uint32_t clientDiagGnssLocationPosTechMask;
typedef enum {
    CLIENT_DIAG_LOCATION_POS_TECH_DEFAULT_BIT                  = 0,
    CLIENT_DIAG_LOCATION_POS_TECH_SATELLITE_BIT                = (1<<0),
    CLIENT_DIAG_LOCATION_POS_TECH_CELLID_BIT                   = (1<<1),
    CLIENT_DIAG_LOCATION_POS_TECH_WIFI_BIT                     = (1<<2),
    CLIENT_DIAG_LOCATION_POS_TECH_SENSORS_BIT                  = (1<<3),
    CLIENT_DIAG_LOCATION_POS_TECH_REFERENCE_LOCATION_BIT       = (1<<4),
    CLIENT_DIAG_LOCATION_POS_TECH_INJECTED_COARSE_POSITION_BIT = (1<<5),
    CLIENT_DIAG_LOCATION_POS_TECH_AFLT_BIT                     = (1<<6),
    CLIENT_DIAG_LOCATION_POS_TECH_HYBRID_BIT                   = (1<<7),
    CLIENT_DIAG_LOCATION_POS_TECH_PPE_BIT                      = (1<<8)
} clientDiagGnssLocationPosTechBits;

typedef enum {
    /** This is the propagated/aggregated reports from all engines
        running on the system (e.g.: DR/SPE/PPE). */
    CLIENT_DIAG_LOC_OUTPUT_ENGINE_FUSED = 0,
    /** This fix is the unmodified fix from modem GNSS engine */
    CLIENT_DIAG_LOC_OUTPUT_ENGINE_SPE   = 1,
    /** This is the unmodified fix from PPP/RTK correction engine */
    CLIENT_DIAG_LOC_OUTPUT_ENGINE_PPE   = 2,
    CLIENT_DIAG_LOC_OUTPUT_ENGINE_COUNT,
} clientDiagLocOutputEngineType;

typedef uint32_t clientDiagPositioningEngineMask;
typedef enum {
    CLIENT_DIAG_STANDARD_POSITIONING_ENGINE = (1 << 0),
    CLIENT_DIAG_DEAD_RECKONING_ENGINE       = (1 << 1),
    CLIENT_DIAG_PRECISE_POSITIONING_ENGINE  = (1 << 2)
} clientDiagPositioningEngineBits;

typedef PACKED struct PACKED_POST {
    /** Used by Logging Module
      *  Mandatory field */
    log_hdr_type logHeader;
    /** clientDiag Message Version
     *  Mandatory field */
    uint8 version;
    /** bitwise OR of LocationFlagsBits to mark which params are valid */
    clientDiagLocationFlagsMask flags;
    /** UTC timestamp for location fix, milliseconds since January 1, 1970 */
    uint64_t timestamp;
    /** in degrees; range [-90.0, 90.0] */
    double latitude;
    /** in degrees; range [-180.0, 180.0] */
    double longitude;
    /** in meters above the WGS 84 reference ellipsoid */
    double altitude;
    /** in meters per second */
    float speed;
    /** in degrees; range [0, 360) */
    float bearing;
    /** in meters */
    float horizontalAccuracy;
    /** in meters */
    float verticalAccuracy;
    /** in meters/second */
    float speedAccuracy;
    /** in degrees (0 to 359.999) */
    float bearingAccuracy;
    clientDiagLocationTechnologyMask techMask;
    /** bitwise OR of GnssLocationInfoBits for param validity */
    clientDiagGnssLocationInfoFlagMask gnssInfoFlags;
    /** altitude wrt mean sea level */
    float altitudeMeanSeaLevel;
    /** position dilusion of precision */
    float pdop;
    /** horizontal dilusion of precision */
    float hdop;
    /** vertical dilusion of precision */
    float vdop;
    /** geometric  dilution of precision */
    float gdop;
    /** time dilution of precision */
    float tdop;
    /** magnetic deviation */
    float magneticDeviation;
    /** horizontal reliability */
    clientDiagLocationReliability horReliability;
    /** vertical reliability */
    clientDiagLocationReliability verReliability;
    /** horizontal elliptical accuracy semi-major axis */
    float horUncEllipseSemiMajor;
    /** horizontal elliptical accuracy semi-minor axis */
    float horUncEllipseSemiMinor;
    /** horizontal elliptical accuracy azimuth */
    float horUncEllipseOrientAzimuth;
    /** North standard deviation Unit: Meters */
    float northStdDeviation;
    /** East standard deviation. Unit: Meters */
    float eastStdDeviation;
    /** North Velocity.Unit: Meters/sec */
    float northVelocity;
    /** East Velocity  Unit: Meters/sec */
    float eastVelocity;
    /** Up Velocity  Unit: Meters/sec */
    float upVelocity;
    float northVelocityStdDeviation;
    float eastVelocityStdDeviation;
    float upVelocityStdDeviation;
    /** Gnss sv used in position data */
    clientDiagGnssLocationSvUsedInPosition svUsedInPosition;
    /** Nav solution mask to indicate sbas corrections */
    clientDiagGnssLocationNavSolutionMask  navSolutionMask;
    /** Position technology used in computing this fix */
    clientDiagGnssLocationPosTechMask      posTechMask;
    /** Body Frame Dynamics: 4wayAcceleration and pitch set with validity */
    clientDiagGnssLocationPositionDynamics bodyFrameData;
    /**  GNSS System Time */
    clientDiagGnssSystemTime               gnssSystemTime;
    /** Number of measurements received for use in fix */
    uint8_t                                numOfMeasReceived;
    /** Gnss measurement usage info  */
    clientDiagGnssMeasUsageInfo measUsageInfo[CLIENT_DIAG_GNSS_SV_MAX];
    /** Leap Seconds */
    uint8_t leapSeconds;
    /** Time uncertainty in milliseconds   */
    float timeUncMs;
    /** number of SV used in position report */
    uint16_t numSvUsedInPosition;
    /** Sensor calibration confidence percent, valid range [0,
     *  100] */
    uint8_t calibrationConfidencePercent;
    /** Sensor calibration status */
    clientDiagDrCalibrationStatusMask calibrationStatus;
    /** boot timestamp in nano-second when this diag log packet is
     *  created and filled with the info at location client api
     *  layer */
    uint64_t bootTimestampNs;
    /** location engine type. When the fix. when the type is set to
        LOC_ENGINE_SRC_FUSED, the fix is the propagated/aggregated
        reports from all engines running on the system (e.g.:
        DR/SPE/PPE). To check which location engine contributes to
        the fused output, check for locOutputEngMask. */
    clientDiagLocOutputEngineType locOutputEngType;
    /** when loc output eng type is set to fused, this field
        indicates the set of engines contribute to the fix. */
    clientDiagPositioningEngineMask locOutputEngMask;
    /** When robust location is enabled, this field
     * will indicate how well the various input data considered for
     * navigation solution conform to expectations.
     * Range: [0.0, 1.0], with 0.0 for least conforming and 1.0 for
     * most conforming. */
    float conformityIndex;
} clientDiagGnssLocationStructType;

typedef uint32_t clientDiagGnssMeasurementsDataFlagsMask;
typedef enum {
    CLIENT_DIAG_GNSS_MEASUREMENTS_DATA_SV_ID_BIT = (1 << 0),
    CLIENT_DIAG_GNSS_MEASUREMENTS_DATA_SV_TYPE_BIT = (1 << 1),
    CLIENT_DIAG_GNSS_MEASUREMENTS_DATA_STATE_BIT = (1 << 2),
    CLIENT_DIAG_GNSS_MEASUREMENTS_DATA_RECEIVED_SV_TIME_BIT = (1 << 3),
    CLIENT_DIAG_GNSS_MEASUREMENTS_DATA_RECEIVED_SV_TIME_UNCERTAINTY_BIT = (1 << 4),
    CLIENT_DIAG_GNSS_MEASUREMENTS_DATA_CARRIER_TO_NOISE_BIT = (1 << 5),
    CLIENT_DIAG_GNSS_MEASUREMENTS_DATA_PSEUDORANGE_RATE_BIT = (1 << 6),
    CLIENT_DIAG_GNSS_MEASUREMENTS_DATA_PSEUDORANGE_RATE_UNCERTAINTY_BIT = (1 << 7),
    CLIENT_DIAG_GNSS_MEASUREMENTS_DATA_ADR_STATE_BIT = (1 << 8),
    CLIENT_DIAG_GNSS_MEASUREMENTS_DATA_ADR_BIT = (1 << 9),
    CLIENT_DIAG_GNSS_MEASUREMENTS_DATA_ADR_UNCERTAINTY_BIT = (1 << 10),
    CLIENT_DIAG_GNSS_MEASUREMENTS_DATA_CARRIER_FREQUENCY_BIT = (1 << 11),
    CLIENT_DIAG_GNSS_MEASUREMENTS_DATA_CARRIER_CYCLES_BIT = (1 << 12),
    CLIENT_DIAG_GNSS_MEASUREMENTS_DATA_CARRIER_PHASE_BIT = (1 << 13),
    CLIENT_DIAG_GNSS_MEASUREMENTS_DATA_CARRIER_PHASE_UNCERTAINTY_BIT = (1 << 14),
    CLIENT_DIAG_GNSS_MEASUREMENTS_DATA_MULTIPATH_INDICATOR_BIT = (1 << 15),
    CLIENT_DIAG_GNSS_MEASUREMENTS_DATA_SIGNAL_TO_NOISE_RATIO_BIT = (1 << 16),
    CLIENT_DIAG_GNSS_MEASUREMENTS_DATA_AUTOMATIC_GAIN_CONTROL_BIT = (1 << 17),
} clientDiagGnssMeasurementsDataFlagsBits;

typedef uint32_t clientDiagGnssMeasurementsStateMask;
typedef enum {
    CLIENT_DIAG_GNSS_MEASUREMENTS_STATE_UNKNOWN_BIT = 0,
    CLIENT_DIAG_GNSS_MEASUREMENTS_STATE_CODE_LOCK_BIT = (1 << 0),
    CLIENT_DIAG_GNSS_MEASUREMENTS_STATE_BIT_SYNC_BIT = (1 << 1),
    CLIENT_DIAG_GNSS_MEASUREMENTS_STATE_SUBFRAME_SYNC_BIT = (1 << 2),
    CLIENT_DIAG_GNSS_MEASUREMENTS_STATE_TOW_DECODED_BIT = (1 << 3),
    CLIENT_DIAG_GNSS_MEASUREMENTS_STATE_MSEC_AMBIGUOUS_BIT = (1 << 4),
    CLIENT_DIAG_GNSS_MEASUREMENTS_STATE_SYMBOL_SYNC_BIT = (1 << 5),
    CLIENT_DIAG_GNSS_MEASUREMENTS_STATE_GLO_STRING_SYNC_BIT = (1 << 6),
    CLIENT_DIAG_GNSS_MEASUREMENTS_STATE_GLO_TOD_DECODED_BIT = (1 << 7),
    CLIENT_DIAG_GNSS_MEASUREMENTS_STATE_BDS_D2_BIT_SYNC_BIT = (1 << 8),
    CLIENT_DIAG_GNSS_MEASUREMENTS_STATE_BDS_D2_SUBFRAME_SYNC_BIT = (1 << 9),
    CLIENT_DIAG_GNSS_MEASUREMENTS_STATE_GAL_E1BC_CODE_LOCK_BIT = (1 << 10),
    CLIENT_DIAG_GNSS_MEASUREMENTS_STATE_GAL_E1C_2ND_CODE_LOCK_BIT = (1 << 11),
    CLIENT_DIAG_GNSS_MEASUREMENTS_STATE_GAL_E1B_PAGE_SYNC_BIT = (1 << 12),
    CLIENT_DIAG_GNSS_MEASUREMENTS_STATE_SBAS_SYNC_BIT = (1 << 13),
} clientDiagGnssMeasurementsStateBits;

typedef uint16_t clientDiagGnssMeasurementsAdrStateMask;
typedef enum {
    CLIENT_DIAG_GNSS_MEASUREMENTS_ACCUMULATED_DELTA_RANGE_STATE_UNKNOWN = 0,
    CLIENT_DIAG_GNSS_MEASUREMENTS_ACCUMULATED_DELTA_RANGE_STATE_VALID_BIT = (1 << 0),
    CLIENT_DIAG_GNSS_MEASUREMENTS_ACCUMULATED_DELTA_RANGE_STATE_RESET_BIT = (1 << 1),
    CLIENT_DIAG_GNSS_MEASUREMENTS_ACCUMULATED_DELTA_RANGE_STATE_CYCLE_SLIP_BIT = (1 << 2),
} clientDiagGnssMeasurementsAdrStateBits;

typedef enum {
    CLIENT_DIAG_GNSS_MEASUREMENTS_MULTIPATH_INDICATOR_UNKNOWN = 0,
    CLIENT_DIAG_GNSS_MEASUREMENTS_MULTIPATH_INDICATOR_PRESENT,
    CLIENT_DIAG_GNSS_MEASUREMENTS_MULTIPATH_INDICATOR_NOT_PRESENT,
} clientDiagGnssMeasurementsMultipathIndicator;

typedef uint32_t clientDiagGnssMeasurementsClockFlagsMask;
typedef enum {
    CLIENT_DIAG_GNSS_MEASUREMENTS_CLOCK_FLAGS_LEAP_SECOND_BIT = (1 << 0),
    CLIENT_DIAG_GNSS_MEASUREMENTS_CLOCK_FLAGS_TIME_BIT = (1 << 1),
    CLIENT_DIAG_GNSS_MEASUREMENTS_CLOCK_FLAGS_TIME_UNCERTAINTY_BIT = (1 << 2),
    CLIENT_DIAG_GNSS_MEASUREMENTS_CLOCK_FLAGS_FULL_BIAS_BIT = (1 << 3),
    CLIENT_DIAG_GNSS_MEASUREMENTS_CLOCK_FLAGS_BIAS_BIT = (1 << 4),
    CLIENT_DIAG_GNSS_MEASUREMENTS_CLOCK_FLAGS_BIAS_UNCERTAINTY_BIT = (1 << 5),
    CLIENT_DIAG_GNSS_MEASUREMENTS_CLOCK_FLAGS_DRIFT_BIT = (1 << 6),
    CLIENT_DIAG_GNSS_MEASUREMENTS_CLOCK_FLAGS_DRIFT_UNCERTAINTY_BIT = (1 << 7),
    CLIENT_DIAG_GNSS_MEASUREMENTS_CLOCK_FLAGS_HW_CLOCK_DISCONTINUITY_COUNT_BIT = (1 << 8),
} clientDiagGnssMeasurementsClockFlagsBits;

typedef PACKED struct PACKED_POST {
    clientDiagGnssMeasurementsDataFlagsMask flags;  // bitwise OR of GnssMeasurementsDataFlagsBits
    int16_t svId;
    clientDiagGnssSvType svType;
    double timeOffsetNs;
    clientDiagGnssMeasurementsStateMask stateMask;  // bitwise OR of GnssMeasurementsStateBits
    int64_t receivedSvTimeNs;
    int64_t receivedSvTimeUncertaintyNs;
    double carrierToNoiseDbHz;
    double pseudorangeRateMps;
    double pseudorangeRateUncertaintyMps;
    clientDiagGnssMeasurementsAdrStateMask adrStateMask;
    double adrMeters;
    double adrUncertaintyMeters;
    float carrierFrequencyHz;
    int64_t carrierCycles;
    double carrierPhase;
    double carrierPhaseUncertainty;
    clientDiagGnssMeasurementsMultipathIndicator multipathIndicator;
    double signalToNoiseRatioDb;
    double agcLevelDb;
} clientDiagGnssMeasurementsData;

typedef PACKED struct PACKED_POST {
    clientDiagGnssMeasurementsClockFlagsMask flags;// bitwise OR of GnssMeasurementsClockFlagsBits
    int16_t leapSecond;
    int64_t timeNs;
    double timeUncertaintyNs;
    int64_t fullBiasNs;
    double biasNs;
    double biasUncertaintyNs;
    double driftNsps;
    double driftUncertaintyNsps;
    uint32_t hwClockDiscontinuityCount;
} clientDiagGnssMeasurementsClock;

typedef PACKED struct PACKED_POST {
    /** Used by Logging Module
    *  Mandatory field */
    log_hdr_type logHeader;
    /** clientDiag Message Version
    *  Mandatory field */
    uint8 version;
    uint32_t count;        // number of items in GnssMeasurementsData array
    clientDiagGnssMeasurementsClock clock; // clock
    clientDiagGnssMeasurementsData measurements[CLIENT_DIAG_GNSS_MEASUREMENTS_MAX];
} clientDiagGnssMeasurementsStructType;

typedef PACKED struct PACKED_POST {
    /** Used by Logging Module
      *  Mandatory field */
    log_hdr_type logHeader;
    /** clientDiag Message Version
     *  Mandatory field */
    uint8 version;
    /** number of SVs in the GnssSvs array */
    uint32_t count;
    clientDiagGnssSv gnssSvs[CLIENT_DIAG_GNSS_SV_MAX];
} clientDiagGnssSvStructType;


typedef PACKED struct PACKED_POST {
    /** Used by Logging Module
      *  Mandatory field */
    log_hdr_type logHeader;
    /** clientDiag Message Version
     *  Mandatory field */
    uint8 version;
/** Timestamp of NMEA generation */
    uint64_t timestamp;
/** Length of nmea data */
    uint32_t nmeaLength;
/** One or more nmea strings separated by \n charachter */
    uint8_t nmea[1];
} clientDiagGnssNmeaStructType;


/** Specify the valid fields in clientDiagGnssSvPoly. <br/> */
enum clientDiagGnssSvPolyValidityMask {
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::svId. <br/>
     *  This flag should always be set. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_SV_ID              = 0x0000000001,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::svConstellation. <br/>
     *  This flag should always be set. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_SV_CONSTELLATION   = 0x0000000002,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::gloFrequency. <br/>
     *  This flag should be set only if this is Glonass SV. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_GLO_FREQUENCY      = 0x0000000004,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::actionType. <br/>
     *  This flag should always be set. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_ACTION_TYPE        = 0x0000000008,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::statusMask. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_STATUS_MASK        = 0x0000000010,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::T0. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_T0                 = 0x0000000020,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::polyCofXYZ0. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_POLY_COF_XYZ0      = 0x0000000040,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::polyCofXYZN. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_POLY_COF_XYZN      = 0x0000000080,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::polyCofClockBias. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_POLY_COF_CLK_BIAS  = 0x0000000100,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::iode. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_IODE               = 0x0000000200,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::enhancedIOD. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_ENHANCED_IOD       = 0x0000000400,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::svPosUnc. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_SV_POS_UNC         = 0x0000000800,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::ionoDelay. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_IONO_DELAY         = 0x0000001000,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::ionoDot.  <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_IONO_DOT           = 0x0000002000,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::sbasIonoDelay. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_SBAS_IONO_DELAY    = 0x0000004000,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::sbasIonoDot. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_SBAS_IONO_DOT      = 0x0000008000,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::tropoDelay. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_TROPO_DELAY        = 0x0000010000,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::elevation. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_ELEVATION          = 0x0000020000,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::elevationDot. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_ELEVATION_DOT      = 0x0000040000,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::elevationUnc. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_ELEVATION_UNC      = 0x0000080000,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::velCof. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_VEL_COF            = 0x0000100000,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::gpsIscL1ca. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_GPS_ISC_L1CA       = 0x0000200000,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::gpsIscL2c. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_GPS_ISC_L2C        = 0x0000400000,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::gpsIscL5I5. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_GPS_ISC_L5I5       = 0x0000800000,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::gpsIscL5Q5. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_GPS_ISC_L5Q5       = 0x0001000000,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::gpsTgd.  <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_GPS_TGD            = 0x0002000000,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::gloTgdG1G2. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_GLO_TGD_G1G2       = 0x0004000000,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::bdsTgdB1. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_BDS_TGD_B1         = 0x00080000000,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::bdsTgdB2. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_BDS_TGD_B2         = 0x00100000000,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::bdsTgdB2a. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_BDS_TGD_B2A        = 0x00200000000,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::bdsIscB2a. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_BDS_ISC_B2A        = 0x00400000000,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::galBgdE1E5a. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_GAL_BGD_E1E5A      = 0x00800000000,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::galBgdE1E5b. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_GAL_BGD_E1E5B      = 0x01000000000,
    /** clientDiagGnssSvPoly has valid clientDiagGnssSvPoly::navicTgdL5. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_HAS_NAVIC_TDG_L5       = 0x02000000000,
};

/**  SV Polynomial Report Status. <br/> */
enum clientDiagGnssSvPolyStatusMask {
    /** Polynomials based on XTRA. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_STATUS_SRC_ALM_CORR = 0x01,
    /** GLONASS string 4 has been received. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_STATUS_GLO_STR4 = 0x2,
    /** GALILEO polynomial computed from I/Nav navigation
     *  message. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_STATUS_GAL_FNAV = 0x04,
    /** GALILEO polynomial computed from F/Nav navigation message.
     *  <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_STATUS_GAL_INAV = 0x08,
};

/** Action to be performed for the received polynomial for this
 *  satellite. <br/> */
enum clientDiagGnssSvPolyActionType {
    /** Update Polynomial for this satellite. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_ACTION_UPDATE = 1,
    /** Invalidate any previously received polynomial. <br/> */
    CLIENT_DIAG_GNSS_SV_POLY_ACTION_INVALIDATE = 2,
};

/** Const define for the array size of clientDiagGnssSvPoly::velCoef.
 *  <br/>  */
#define CLIENT_DIAG_GNSS_SV_POLY_VELOCITY_COF_SIZE       12
/** Const define for the array size of
 *  clientDiagGnssSvPoly::polyCofXYZ0. <br/>  */
#define CLIENT_DIAG_GNSS_SV_POLY_XYZ_0_TH_ORDER_COF_SIZE  3
/** Const define for the array size of
 *  clientDiagGnssSvPoly::polyCofXYZN. <br/>  */
#define CLIENT_DIAG_GNSS_SV_POLY_XYZ_N_TH_ORDER_COF_SIZE  9
/** Const define for the array size of
 *  clientDiagGnssSvPoly::polyCofClockBias. <br/>  */
#define CLIENT_DIAG_GNSS_SV_POLY_SV_CLKBIAS_COF_SIZE      4

typedef PACKED struct PACKED_POST {
    /** Used by Logging Module
      *  Mandatory field */
    log_hdr_type logHeader;
    /** clientDiag Message Version
     *  Mandatory field */
    uint8 version;
    /** Bitwise OR of clientDiagGnssSvPolyValidityMask to specify the
     *  valid fields in clientDiagGnssSvPoly. <br/> */
    clientDiagGnssSvPolyValidityMask validityMask;

    /** Specify satellite vehicle ID number. <br/> For SV id range
     *  of each supported constellations, refer to documentation in
     *  GnssSv::svId. <br/>
     */
    uint16_t svId;

    /** Specify GNSS Constellation Type for the SV. */
    clientDiagGnss_LocSvSystemEnumType svConstellation;

    /**  GLONASS frequency number + 8. Valid only for GLONASS
     *   systems and must be ignored for all other systems. <br/>
     *   Range: 1 to 14 <br/>   */
    uint8_t gloFrequency;

    /** SV polynomial action type, update or invalidate. <br/> */
    clientDiagGnssSvPolyActionType actionType;

    /**  SV Polynomial Report Status. <br/> */
    clientDiagGnssSvPolyStatusMask statusMask;

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
     *  every element is valid. <br/> */
    double polyCofXYZ0[CLIENT_DIAG_GNSS_SV_POLY_XYZ_0_TH_ORDER_COF_SIZE];

    /** Polynomial coefficient's 1st, 2nd, and 3rd terms for X, Y
     *  and Z coordinates (C1X, C2X,... C2Z, C3Z). <br/>
     *  If this field is indicated as valid via validityMask, then
     *  every element is valid. <br/>
     *  Units are specified as below: <br/>
     *       - 1st term -- Meters/second <br/>
     *       - 2nd term -- Meters/second^2 <br/>
     *       - 3rd term -- Meters/seconds^3 <br/> */
    double polyCofXYZN[CLIENT_DIAG_GNSS_SV_POLY_XYZ_N_TH_ORDER_COF_SIZE];

    /** Polynomial coefficients for satellite clock bias
     *  correction (C0T, C1T, C2T, C3T). <br/> If this field is
     *  indicated as valid via validityMask, then every element is
     *  valid. <br/>
     *  Units are specified below <br/>
     *        - 0th term -- Milliseconds/second <br/>
     *        - 1st term -- Milliseconds/second^2 <br/>
     *        - 2nd term -- Milliseconds/second^3 <br/>
     *        - 3rd term -- Milliseconds/second^4 <br/> */
    float polyCofClockBias[CLIENT_DIAG_GNSS_SV_POLY_SV_CLKBIAS_COF_SIZE];

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
     *    every element is valid. <br/>
     *    Units: <br/>
     *    - 0th term -- Meters/second <br/>
     *    - 1st term -- Meters/second^2 <br/>
     *    - 2nd term -- Meters/second^3 <br/>
     *   - 3rd term -- Meters/second^4 <br/> */
    double velCof[CLIENT_DIAG_GNSS_SV_POLY_VELOCITY_COF_SIZE];

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

} clientDiagGnssSvPoly;

}
#endif /*LOCATION_CLIENT_API_LOG_H */
