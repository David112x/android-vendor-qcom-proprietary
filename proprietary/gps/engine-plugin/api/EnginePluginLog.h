/******************************************************************************
    Copyright (c) 2007-2011, 2014, 2016, 2018-2020 Qualcomm Technologies, Inc.
    All Rights Reserved.
    Confidential and Proprietary - Qualcomm Technologies, Inc.
 *******************************************************************************/

#ifndef ENGINE_PLUGIN_LOG_H
#define ENGINE_PLUGIN_LOG_H

#include <stdint.h>
#include <time.h>

/** Major File Version */
#define EP_LOG_INTERFACE_FILE_MAJOR_VERSION  1
/** Minor File Version */
#define EP_LOG_INTERFACE_FILE_MINOR_VERSION  4

/* Engine Msg callback */
#define EP_1C54_DIAG_VERSION 3
/* Measurement msg Diag version*/
#define EP_1C5E_DIAG_VERSION 5
/* EP correction data version*/
#define EP_1C86_DIAG_VERSION 1
/* EPH Info Diag Version*/
#define EP_1C7B_DIAG_VERSION 4
/* PVT Diag MSG*/
#define EP_1C5B_DIAG_VERSION 9
/* IONO Params Info Diag Version*/
#define EP_1C8B_DIAG_VERSION 1
/* GLO ADD Params Info Diag Version*/
#define EP_1C8C_DIAG_VERSION 1
/* Control command Version*/
#define EP_1C56_DIAG_VERSION 4
/* Subscription info */
#define EP_1C57_DIAG_VERSION 4
/* Error info */
#define EP_1C58_DIAG_VERSION 2
/* Report status response Error info */
#define EP_1CBC_DIAG_VERSION 1

#ifdef __linux__
#define PACKED
#define PACKED_POST __attribute__((__packed__))
#endif
#define EP_DIAG_ID_LENGTH   128

#if defined(__linux__) || defined(USE_GLIB) || defined(__ANDROID__)
#define TYPEDEF_PACKED_STRUCT typedef PACKED struct PACKED_POST
#else
#define TYPEDEF_PACKED_STRUCT typedef struct
#endif

#ifdef _WIN32
#pragma pack(push, 1)
#endif
#ifdef _WIN32
#define int8    int8_t
#define uint8   uint8_t
#define uint32  uint32_t
#define int32   int32_t
#define uint16  uint16_t
#define int16   int16_t
#define uint64  uint64_t
#define int64   int64_t
#define boolean uint8_t
#define byte    uint8_t

typedef double    DOUBLE;
typedef float     FLOAT;
typedef long long INT64;
typedef unsigned long long UINT64;
typedef int INT32;
typedef unsigned int UINT32;
typedef short int INT16;
typedef unsigned short int UINT16;
typedef unsigned char UINT8;
#endif

#if !defined(USE_GLIB) && !defined(__ANDROID__)
#ifndef __LOG_HDR_TYPE__
#define __LOG_HDR_TYPE__
typedef struct PACKED_POST {
    uint16_t len;         /* Specifies the length, in bytes of the
                     entry, including this header. */

    uint16_t code;            /* Specifies the log code for the entry as
                     enumerated above. Note: This is
                     specified as word to guarantee size. */
                     /*upper 48 bits represent elapsed time since
                     6 Jan 1980 00:00:00 in 1.25 ms units. The
                     low order 16 bits represent elapsed time
                     since the last 1.25 ms tick in 1/32 chip
                     units (this 16 bit counter wraps at the
                     value 49152). */
    uint32_t ts_lo; /* Time stamp */
    uint32_t ts_hi;
  } log_hdr_type;
#endif
#endif


/** @brief EP API Diag logger header for all messages. */
TYPEDEF_PACKED_STRUCT {
    /** Used by Logging Module */
    log_hdr_type z_LogHeader;
    /** EP Message Version */
    uint8_t u_Version;
    /** Engine Plugin identifier */
    char epID[EP_DIAG_ID_LENGTH];
    /** Source Nname */
    char source[EP_DIAG_ID_LENGTH];
    /** Process identification */
    uint32_t u_Process_id;
    /** time at logging in milliseconds */
    uint64_t t_TimeTickMsec;

    uint8_t eventType;
    uint32_t reserved2;
} epDiagLogGenericHeader;

/** Type of Log Level */
typedef uint32_t epDiagLogLevelType;
/** Log Level as Error */
#define EP_DIAG_LOG_LEVEL_ERROR    ((epDiagLogLevelType)0x00000001)
/** Log Level as Info */
#define EP_DIAG_LOG_LEVEL_INFO     ((epDiagLogLevelType)0x00000002)
/** Log Level as Warning */
#define EP_DIAG_LOG_LEVEL_WARNING  ((epDiagLogLevelType)0x00000003)
/** Log Level as Debug */
#define EP_DIAG_LOG_LEVEL_DEBUG    ((epDiagLogLevelType)0x00000004)
/** Log Level as Verbose */
#define EP_DIAG_LOG_LEVEL_VERBOSE  ((epDiagLogLevelType)0x00000005)

/** Type of Engine log */
typedef uint32_t epDiagLogType;
/** Log of debugging */
#define EP_DIAG_LOG_TYPE_DEBUG            ((epDiagLogType)0x00000001)
/** Log of Engine playback data */
#define EP_DIAG_LOG_TYPE_PLAYBACK         ((epDiagLogType)0x00000002)
/** Log of nmea sentence */
#define EP_DIAG_LOG_TYPE_NMEA             ((epDiagLogType)0x00000003)
/** Log of rtcm service status */
#define EP_DIAG_LOG_TYPE_RTCM_STATUS      ((epDiagLogType)0x00000004)

TYPEDEF_PACKED_STRUCT {
    epDiagLogGenericHeader header;
    uint32_t moduleID;
    epDiagLogLevelType logLevel;
    epDiagLogType logType;
    uint32_t logLength;
    uint8_t logMessage[1];
} epDiagLogEngineMsg;

/** GNSS Signal Type and RF Band */
typedef uint32_t epDiagGnssSignalTypeMask;
/*** Unknow Signal */
#define EP_DIAG_GNSS_SIGNAL_UNKNOWN      ((epDiagGnssSignalTypeMask)0x00000000)
/** GPS L1CA Signal */
#define EP_DIAG_GNSS_SIGNAL_GPS_L1CA     ((epDiagGnssSignalTypeMask)0x00000001)
/** GPS L1C Signal */
#define EP_DIAG_GNSS_SIGNAL_GPS_L1C      ((epDiagGnssSignalTypeMask)0x00000002)
/** GPS L2 RF Band */
#define EP_DIAG_GNSS_SIGNAL_GPS_L2C_L       ((epDiagGnssSignalTypeMask)0x00000004)
/** GPS L5 RF Band */
#define EP_DIAG_GNSS_SIGNAL_GPS_L5_Q       ((epDiagGnssSignalTypeMask)0x00000008)
/** GLONASS G1 (L1OF) RF Band */
#define EP_DIAG_GNSS_SIGNAL_GLONASS_G1_CA   ((epDiagGnssSignalTypeMask)0x00000010)
/** GLONASS G2 (L2OF) RF Band */
#define EP_DIAG_GNSS_SIGNAL_GLONASS_G2_CA   ((epDiagGnssSignalTypeMask)0x00000020)
/** GALILEO E1 RF Band */
#define EP_DIAG_GNSS_SIGNAL_GALILEO_E1_C   ((epDiagGnssSignalTypeMask)0x00000040)
/** GALILEO E5A RF Band */
#define EP_DIAG_GNSS_SIGNAL_GALILEO_E5A_Q  ((epDiagGnssSignalTypeMask)0x00000080)
/** GALILEO E5B RF Band */
#define EP_DIAG_GNSS_SIGNAL_GALILIEO_E5B_Q ((epDiagGnssSignalTypeMask)0x00000100)
/** BEIDOU B1 RF Band */
#define EP_DIAG_GNSS_SIGNAL_BEIDOU_B1_I    ((epDiagGnssSignalTypeMask)0x00000200)
/** BEIDOU B1_C RF Band */
#define EP_DIAG_GNSS_SIGNAL_BEIDOU_B1_C    ((epDiagGnssSignalTypeMask)0x00000400)
/** BEIDOU B2 RF Band */
#define EP_DIAG_GNSS_SIGNAL_BEIDOU_B2_I    ((epDiagGnssSignalTypeMask)0x00000800)
/** BEIDOU B2A_I RF Band */
#define EP_DIAG_GNSS_SIGNAL_BEIDOU_B2A_I   ((epDiagGnssSignalTypeMask)0x00001000)
/** QZSS L1CA RF Band */
#define EP_DIAG_GNSS_SIGNAL_QZSS_L1CA      ((epDiagGnssSignalTypeMask)0x00002000)
/** QZSS L1S RF Band */
#define EP_DIAG_GNSS_SIGNAL_QZSS_L1S      ((epDiagGnssSignalTypeMask)0x00004000)
/** QZSS L2 RF Band */
#define EP_DIAG_GNSS_SIGNAL_QZSS_L2C_L      ((epDiagGnssSignalTypeMask)0x00008000)
/** QZSS L5 RF Band */
#define EP_DIAG_GNSS_SIGNAL_QZSS_L5_Q      ((epDiagGnssSignalTypeMask)0x00010000)
/** SBAS L1 RF Band */
#define EP_DIAG_GNSS_SIGNAL_SBAS_L1_CA      ((epDiagGnssSignalTypeMask)0x00020000)


/** GNSS Constellation type */
typedef uint16_t epDiagGnssConstellationTypeMask;
/** Unknow Constellation */
#define EP_DIAG_GNSS_CONSTELLATION_UNKNOWN ((epGnssConstellationTypeMask) 0x0000)
/** GPS Constellation */
#define EP_DIAG_GNSS_CONSTELLATION_GPS ((epDiagGnssConstellationTypeMask) 0x0001)
/** Galileo Constellation */
#define EP_DIAG_GNSS_CONSTELLATION_GALILEO ((epDiagGnssConstellationTypeMask) 0x0002)
/** SBAS Constellation */
#define EP_DIAG_GNSS_CONSTELLATION_SBAS ((epDiagGnssConstellationTypeMask) 0x0004)
/** COMPASS Constellation */
#define EP_DIAG_GNSS_CONSTELLATION_COMPASS ((epDiagGnssConstellationTypeMask) 0x0008)
/** GLONASS Constellation */
#define EP_DIAG_GNSS_CONSTELLATION_GLONASS ((epDiagGnssConstellationTypeMask) 0x0010)
/** BEIDOU Constellation */
#define EP_DIAG_GNSS_CONSTELLATION_BEIDOU ((epDiagGnssConstellationTypeMask) 0x0020)
/** QZSS Constellation */
#define EP_DIAG_GNSS_CONSTELLATION_QZSS ((epDiagGnssConstellationTypeMask) 0x0040)

/** Max number of GNSS SV measurement */
#define EP_DIAG_GNSS_MAX_MEAS                  128


/** GNSS Measurement Status Mask */
typedef uint64_t epDiagGNSSMeasurementStatusMask;
/** Satellite time in sub-millisecond (code phase) is known */
#define EP_DIAG_MEAS_STATUS_SM_VALID              ((epDiagGNSSMeasurementStatusMask)0x00000001)
/** Satellite sub-bit time is known */
#define EP_DIAG_MEAS_STATUS_SB_VALID              ((epDiagGNSSMeasurementStatusMask)0x00000002)
/** Satellite time in milliseconds is known */
#define EP_DIAG_MEAS_STATUS_MS_VALID              ((epDiagGNSSMeasurementStatusMask)0x00000004)
/** Signal bit edge is confirmed  */
#define EP_DIAG_MEAS_STATUS_BE_CONFIRM            ((epDiagGNSSMeasurementStatusMask)0x00000008)
/** Satellite Doppler is measured */
#define EP_DIAG_MEAS_STATUS_VELOCITY_VALID        ((epDiagGNSSMeasurementStatusMask)0x00000010)
/** Status as Reserved */
#define EP_DIAG_MEAS_STATUS_RESERVED              ((epDiagGNSSMeasurementStatusMask)0x00000020)
/** TRUE/FALSE : Lock Point is valid/invalid.*/
#define EP_DIAG_MEAS_STATUS_LP_VALID              ((epDiagGNSSMeasurementStatusMask)0x00000040)
/** TRUE/FALSE : Lock Point is positive/negative*/
#define EP_DIAG_MEAS_STATUS_LP_POS_VALID          ((epDiagGNSSMeasurementStatusMask)0x00000080)
/** Range update from satellite differences is measured */
#define EP_DIAG_MEAS_STATUS_FROM_RNG_DIFF         ((epDiagGNSSMeasurementStatusMask)0x00000200)
/** Doppler update from satellite differences is measured */
#define EP_DIAG_MEAS_STATUS_FROM_VE_DIFF          ((epDiagGNSSMeasurementStatusMask)0x00000400)
/** Fresh GNSS measurement observed in last second  */
#define EP_DIAG_MEAS_STATUS_GNSS_FRESH_MEAS_VALID ((epDiagGNSSMeasurementStatusMask)0x08000000)

/** GNSS SV search status */
typedef uint8_t epDiagGnssMeasSvSearchState;
/** SV is not being actively searched */
#define EP_DIAG_GNSS_SV_SRCH_STATE_IDLE (1)
/** The system is searching for this SV */
#define EP_DIAG_GNSS_SV_SRCH_STATE_SEARCH (2)
/** SV is being tracked */
#define EP_DIAG_GNSS_SV_SRCH_STATE_TRACK (3)

/** GNSS SV Info Mask */
typedef uint8_t epDiagGnssMeasSvInfoMask;
/** Ephemeris is available for this SV */
#define EP_DIAG_GNSS_SVINFO_MASK_HAS_EPHEMERIS ((epDiagGnssMeasSvInfoMask)0x01)
/** Almanac is available for this SV */
#define EP_DIAG_GNSS_SVINFO_MASK_HAS_ALMANAC ((epDiagGnssMeasSvInfoMask)0x02)

/** Flags to indicate valid fields in epDiagGnssSystemTimeStructType */
TYPEDEF_PACKED_STRUCT {
    uint16_t isSystemWeekValid : 1;
    uint16_t isSystemWeekMsecValid : 1;
    uint16_t isSystemClkTimeBiasValid : 1;
    uint16_t isSystemClkTimeUncMsValid : 1;
    uint16_t isRefFCountValid : 1;
    uint16_t isNumClockResetsValid : 1;
    uint16_t isIntraSystemTimeBias1AndUncValid : 1;
    uint16_t : 9;
} epDiagSystemTimeValidity;

/** Flags to indicate valid fields in epGnssLocGloTimeStructType */
typedef uint32_t epDiagGloTimeValidityM;
TYPEDEF_PACKED_STRUCT {
    uint16_t isGloClkTimeBiasValid : 1;
    uint16_t isGloClkTimeUncMsValid : 1;
    uint16_t isRefFCountValid : 1;
    uint16_t isNumClockResetsValid : 1;
    uint16_t isGloFourYearValid: 1;
    uint16_t isGloDaysValid: 1;
    uint16_t isGloMsecValid: 1;
    uint16_t : 9;
} epDiagGloTimeValidity;

/** Flags to indicate valid fields in epDiagGnssSVMeasurementStructType */
TYPEDEF_PACKED_STRUCT {
    uint32_t isHealthStatusValid : 1;
    uint32_t isSvInfoMaskValid : 1;
    uint32_t isLossOfLockValid : 1;
    uint32_t isFineSpeedValid : 1;
    uint32_t isFineSpeedUncValid : 1;
    uint32_t isCarrierPhaseValid : 1;
    uint32_t isCarrierPhaseUncValid : 1;
    uint32_t isCycleSlipCountValid : 1;
    uint32_t isPseudorangeValid : 1;
    uint32_t isPseudorangeUncValid : 1;
    uint32_t isDopplerValid : 1;
    uint32_t isDopplerUncValid : 1;
    uint32_t isCNoValid : 1;
    uint32_t isSVMSValid : 1;
    uint32_t isSVSubMSValid : 1;
    uint32_t isSVTimeUncValid : 1;
    uint32_t : 16;
} gnssDiagSVMeasurementValidity;

/** Flags to indicate SV LLI */
TYPEDEF_PACKED_STRUCT {
    uint8_t lossOfContinousLock : 1;
    uint8_t halfCycleAmbiguity : 1;
    uint8_t : 6;
} epDiagGnssMeasSvLLI;

/** Flags to indicate valid fields in epDiagGnssMeasurementReport */
TYPEDEF_PACKED_STRUCT {
    uint16_t isLeapSecValid : 1;
    uint16_t isClockDriftValid : 1;
    uint16_t isClockDriftStdDeviationValid : 1;
    uint16_t isGpsSystemTimeValid : 1;
    uint16_t isGalSystemTimeValid : 1;
    uint16_t isBdsSystemTimeValid : 1;
    uint16_t isQzssSystemTimeValid : 1;
    uint16_t isGloSystemTimeValid : 1;
    uint16_t : 8;
} epDiagGnssMeasurementReportValidity;

typedef struct {
    uint32_t tv_sec;
    uint32_t tv_nsec;
} timeSpec;

/** GNSS Measurement Report Type */
typedef enum {
/** GNSS Measurement 1 Hz Report */
    EP_DIAG_MEASUREMENT_1HZ_REPORT = 1,
/** GNSS Measurement 10 Hz Report */
    EP_DIAG_MEASUREMENT_NHZ_REPORT
} epDiagGnssMeasurementReportType;

/** Indicates AP time stamp  */
TYPEDEF_PACKED_STRUCT {
    /** Boot time received from pps-ktimer
        Mandatory Field */
    timeSpec apTimeStamp;
    /** Time-stamp uncertainty in Milliseconds
        Mandatory Field */
    float apTimeStampUncertaintyMs;
} epDiagGnssApTimeStampStructType;

/** GNSS System Time */
TYPEDEF_PACKED_STRUCT {
    /** Validity mask for below fields */
    epDiagSystemTimeValidity validityMask;
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
    /** Number of clock resets/discontinuities detected, affecting the local hardware counter value. */
    uint32_t numClockResets;
    /**  For GPS: GPS L1 to L5 time bias (L5-L1)
         For BDS: Reserved
         For GAL: GAL E1 to E5a time bias (E5a-E1)
         For QZSS: QZSS L1 to L5 time bias (L5-L1)
         Unit: millisecond  */
    float intraSystemTimeBias1;
    /**  Uncertainty of intraSystemTimeBias1
         Unit: millisecond  */
    float intraSystemTimeBias1Unc;
} epDiagGnssSystemTimeStructType;


/** GNSS GLONASS Time */
TYPEDEF_PACKED_STRUCT {
    /** GLONASS day number in four years. Refer to GLONASS ICD.
        Applicable only for GLONASS and shall be ignored for other constellations.
        If unknown shall be set to 65535
        Mandatory Field */
    uint16_t gloDays;
    /** GLONASS time of day in Millisecond. Refer to GLONASS ICD.
        Units: Millisecond
        Check for gloClkTimeUncMs before use */
    /** Validity mask for below fields */
    epDiagGloTimeValidity validityMask;
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
    /** Number of clock resets/discontinuities detected, affecting the local hardware counter value. */
    uint32_t numClockResets;
    /** GLONASS four year number from 1996. Refer to GLONASS ICD.
        Applicable only for GLONASS and shall be ignored for other constellations.
        If unknown shall be set to 255
        Mandatory Field */
    uint8_t gloFourYear;
} epDiagGnssGloTimeStructType;

/** GNSS SV Measurement Info */
TYPEDEF_PACKED_STRUCT {
    /** Specifies GNSS signal type. Mandatory field */
    epDiagGnssSignalTypeMask gnssSignal;
    /** Specifies GNSS system. Mandatory field */
    epDiagGnssConstellationTypeMask gnssSystem;
    /** Specifies GNSS SV ID.
             For GPS:             1 to 32
             For GLONASS:     65 to 96. When slot-number to SV ID mapping is unknown, set as 255.
             For SBAS:           120 to 151
             For QZSS-L1CA:  193 to 197
             For BDS:             201 to 237
             For GAL:              301 to 336
             Mandatory Field */
    uint16_t gnssSvId;
    /** GLONASS frequency number + 7.
         Valid only for a GLONASS system and
         is to be ignored for all other systems.
             - Range: 1 to 14 */
    uint8_t gloFrequency;
    /** Specifies satellite search state. Mandatory field */
    epDiagGnssMeasSvSearchState svSearchState;
    /**  Carrier to Noise ratio
         - Units: 0.1 dBHz
         Mandatory field */
    float cN0dbHz;
    /**  Bit-mask indicating SV measurement status.
         Valid bit-masks:
         If any MSB bit in 0xFFC0000000000000 DONT_USE is set, the measurement
         must not be used by the client.
         Mandatory field */
    epDiagGNSSMeasurementStatusMask  measurementStatus;
    /**  Satellite time Millisecond.
         When measurementStatus bit-0 (sub-ms valid),
         1 (sub-bit known) & 2 (SV time known) are set, for:
         GPS, BDS, GAL, QZSS: Range of 0 through (604800000-1).
         GLO: Range of 0 through (86400000-1).
         Unit:Millisecond
         Note: All SV times in the current measurement block are already
               propagated to common reference time epoch.
         Mandatory field */
    uint32_t svMs;
    /**  Satellite time sub-millisecond.
         When measurementStatus bit-0 "Sub-millisecond is valid" is set,
         Range of 0 through 0.99999 [Milliseconds]
         Total SV Time = svMs + svSubMs
         - Units: Millisecond
        Mandatory field */
    float svSubMs;
    /**  Satellite Time uncertainty
         - Units: Millisecond
         Mandatory field */
    float svTimeUncMs;
    /** Satellite Doppler
        - Units: meter per sec
        Mandatory field */
    float dopplerShift;
    /** Satellite Doppler uncertainty
        - Units: meter per sec */
    float dopplerShiftUnc;
    /** Validity mask for below fields */
    gnssDiagSVMeasurementValidity validityMask;
    /** Health status.
        Range: 0 to 2;
        0 = unhealthy, 1 = healthy, 2 = unknown */
    uint8_t healthStatus;
    /** Indicates whether almanac and ephemeris information is available. */
    epDiagGnssMeasSvInfoMask svInfoMask;
    /** Loss of signal lock indicator */
    epDiagGnssMeasSvLLI lossOfLock;
    /** Increments when a CSlip is detected */
    uint8_t cycleSlipCount;
    /** Carrier phase derived speed
        Units: m/s */
    /** Estimated pseudorange uncertainty
        Unit: Meters */
    float pseudorange_uncertinty;
    /** Carrier phase measurement [Carrier cycles]  */
    double carrierPhase;
    /** Carrier phase measurement Unc [Carrier cycles]  */
    float carrierPhaseUnc;
    /** Estimated pseudorange
        Unit: Meters */
    double pseudorange;
} epDiagGnssSVMeasurementStructType;

/** GNSS Measurement Report */
TYPEDEF_PACKED_STRUCT {
    /** Receiver clock Drift
        Units: meter per sec */
    float clockDrift;
    /** Receiver clock drift std deviation
        Units: Meter per sec */
    float clockDriftStdDeviation;
    /** GPS system time information. Check validity before using */
    epDiagGnssSystemTimeStructType gpsSystemTime;
    /** GAL system time information. Check validity before using */
    epDiagGnssSystemTimeStructType galSystemTime;
    /** BDS system time information. Check validity before using */
    epDiagGnssSystemTimeStructType bdsSystemTime;
    /** QZSS system time information. Check validity before using */
    epDiagGnssSystemTimeStructType qzssSystemTime;
    /** GLO system time information. Check validity before using */
    epDiagGnssGloTimeStructType gloSystemTime;
    /** Validity mask */
    epDiagGnssMeasurementReportValidity validityMask;
    /** GPS time leap second delta to UTC time. Check validity before using
        Units: sec */
    uint8_t leapSec;
    /** Number of Measurements in this report block.
        Mandatory field */
    uint8_t numMeas;
    /** Satellite measurement Information
        Mandatory field */
    epDiagGnssSVMeasurementStructType svMeasurement[EP_DIAG_GNSS_MAX_MEAS];
    /** Local time of last PPS pulse, associated with GNSS Measurement report
        Mandatory field */
    epDiagGnssApTimeStampStructType lastPpsLocaltimeStamp;
    /** GNSS Measurement Report Type */
    epDiagGnssMeasurementReportType gnssMeasReportType;
} epDiagGnssMeasurementReportStruct;

TYPEDEF_PACKED_STRUCT {
    epDiagLogGenericHeader header;
    uint8_t segmentNo;
    uint8_t maxSegmentNo;
    epDiagGnssMeasurementReportStruct msrInfo;
} epDiagGnssMeasurementReport;

TYPEDEF_PACKED_STRUCT {
    epDiagLogGenericHeader header;
    /** Source of correction data */
    uint8_t correctionSource;
    /** Length of correction data in correctionData */
    uint32_t correctionLength;
    /** Correction Data, size of correctionLength*/
    uint8_t correctionData[1];
} epDiagCorrectionStreamData;

/** GALILEO Ephemeris Validity Mask */
typedef uint16_t epDiagGalileoEphemerisValidityMask;
/** bgdE1E5a is present */
#define EP_DIAG_GALILEO_EPHEMERIS_BGDE1E5A ((epDiagGalileoEphemerisValidityMask)0x01)
/** bgdE1E5b is present */
#define EP_DIAG_GALILEO_EPHEMERIS_BGDE1E5B ((epDiagGalileoEphemerisValidityMask)0x02)

/** GALILEO Signal Type */
typedef enum {
/** GALILEO E1 MBOC Pilot RF Band */
   DIAG_E1B = 1,
/** GALILEO E5A BPSK Pilot RF Band */
   DIAG_E5A,
/** GALILEO E5B RF Band */
   DIAG_E5B
} epDiagGalSVDataSource;


/** Represents GPS Navigation model info */
TYPEDEF_PACKED_STRUCT {
    /**  GNSS SV ID.
         For GPS:      1 to 32
         Mandatory Field */
    uint16_t gnssSvId;
    /** The six-bit health indication as defined in GPS ICD, for L1/L2 only
        Mandatory Field */
    uint8_t svHealthMask;
    /** Issue of Data (ephemeris)
        Mandatory Field */
    uint16_t iode;
    /** Square root of the semi-major axis
        Mandatory Field */
    double aSqrt;
    /** Mean motion difference
        Unit: radians/s
        Mandatory Field */
    double deltaNRadPerSec;
    /** Mean anomaly at reference time
        Unit: radians
        Mandatory Field */
    double m0Rad;
    /** Eccentricity
        Mandatory Field */
    double eccentricity;
    /** Longitude of ascending node of orbit plane at weekly epoch
        Unit: radians
        Mandatory Field */
    double omegaORad;
    /** Inclination angle at reference time
        Unit: radians
        Mandatory Field */
    double inclinationAngleRad;
    /** Argument of perigee
        Unit: radians
        Mandatory Field */
    double omegaRad;
    /** Rate of right ascension
        Unit: radians/s
        Mandatory Field */
    double omegaDotRadPerSec;
    /** Rate of inclination angle
        Unit: radians/s
        Mandatory Field */
    double iDotRadPerSec;
    /** Amplitude of the cosine harmonic correction term
        to the argument of latitude
        Unit: radians
        Mandatory Field */
    double cUcRad;
    /** Amplitude of the sine harmonic correction term
        to the argument of latitude
        Unit: radians
        Mandatory Field */
    double cUsRad;
    /** Amplitude of the cosine harmonic correction term
        to the orbit radius
        Unit: meter
        Mandatory Field */
    double cRcMeter;
    /** Amplitude of the sine harmonic correction term
        to the orbit radius
        Unit: meter
        Mandatory Field */
    double cRsMeter;
    /** Amplitude of the cosine harmonic correction term
        to the angle of inclination
        Unit: radians/s
        Mandatory Field */
    double cIcRad;
    /** Amplitude of the sine harmonic correction term
        to the angle of inclination (radians)
        Unit: radians/s
        Mandatory Field */
    double cIsRad;
    /** Reference time ephemeris
        Unit: second
        Mandatory Field */
    uint32_t fullToeSec;
    /** Clock data reference time
        Unit: s
        Mandatory Field */
    uint32_t fullTocSec;
    /** Clock bias correction coefficient
        Unit: s
        Mandatory Field */
    double af0Sec;
    /** Clock drift correction coefficient
        Unit: seconds/s
        Mandatory Field */
    double af1;
    /** Clock drift rate correction coefficient
        Unit: seconds/s^2
        Mandatory Field */
    double af2;
    /** User Range Accuracy Index(4bit)
        Mandatory Field */
    uint8_t urai;
    /** codes on L2 channel
        Mandatory Field */
    uint8_t codeL2;
    /** L2 P data flag
        Mandatory Field */
    uint8_t L2PData;
    /** Indicates the curve-fit interval used by the CS
        Unit: hours
        Range: 0 = 0-4 hours
               1 = greater than 4 hours
        Mandatory Field */
    uint8_t fitInterval;
    /** Time of Group delay
        Unit:seconds
        Mandatory Field */
    double  tgd;
    uint16_t iodc;
} epDiagGpsSatelliteEphemerisData;

/** Represents Glonass Navigation model info */
TYPEDEF_PACKED_STRUCT {
   /**  GNSS SV ID.
         For GLONASS:  65 to 96. When slot-number to SV ID mapping is unknown,
         set as 255.
         Mandatory Field */
    uint16_t gnssSvId;
    /** Satellite health. 0 means broadcasting satellite is good and 1 means not
        Mandatory Field */
    uint8_t bnHealth;
    /** Ln SV health flages for GLONASS-M. 0 means broadcasting satellite is good
        and 1 means not
        Mandatory Field */
    uint8_t lnHealth;
    /** User Range Accuracy index.  P32 ICD Glonass for value of Ft.
        Mandatory Field */
    uint8_t FT;
    /** Glonass vehicle type. M=1 means type M
        Mandatory Field */
    uint8_t M;
    /** Age of current information in days
        Unit: days
        Mandatory Field */
    uint8_t En;
    /** Carrier frequency number + 8
        Range: 1-14
        Mandatory Field */
    uint8_t freqNo;
    /** Time interval between adjacent values of tb
        Unit: minutes
        Mandatory Field */
    uint8_t P1;
    /** if tb is odd and 0 if tb is even
        Mandatory Field */
    uint8_t P2;
    /** Time difference between navigation RF signal transmission in G2 and G1
        Unit: second
        Mandatory Field */
    float deltaTau;
    /** X component of SV position
        Unit: meter
        Mandatory Field */
    double x;
    /** Y component of SV position
        Unit: meter
        Mandatory Field */
    double y;
    /** Z component of SV position
        Unit: meter
        Mandatory Field */
    double z;
    /** X component of SV velocity
        Unit: meter/sec
        Mandatory Field */
    double vx;
    /** Y component of SV velocity
        Unit: meter/sec
        Mandatory Field */
    double vy;
    /** Z component of SV velocity
        Unit: meter/sec
        Mandatory Field */
    double vz;
    /** X component of SV sola-luni acceleration
        Unit: meter/sec^2
        Mandatory Field */
    double ax;
    /** Y component of SV sola-luni acceleration
        Unit: meter/sec^2
        Mandatory Field */
    double lsy;
    /** Z component of SV sola-luni acceleration
        Unit: meter/sec^2
        Mandatory Field */
    double lsz;
    /** SV clock bias, offset between GLONASS system time and the satellite clock
        Unit: second
        Mandatory Field */
    float tauN;
    /** SV clock frequency error ratio
        Mandatory Field */
    float gamma;
    /** Current date, calendar number of day within four-year interval.
        Starting from the 1st of January in a leap year
        Unit: days
        Mandatory Field */
    uint32_t fullToeSec;
} epDiagGlonassSatelliteEphemerisData;

/** BDS Navigation model Info */
TYPEDEF_PACKED_STRUCT {
    /**  GNSS SV ID.
         For BDS:      201 to 237
         Mandatory Field */
    uint16_t gnssSvId;
    /** Autonomous satellite Health flag applied for both B1 and B2 (SatH1)
        Mandatory Field */
    uint8_t svHealthMask;
    /** Age of Data, Ephemeris
        Mandatory Field */
    uint16_t aode;
    /** Age of Data, Clock
        Mandatory Field */
    uint8_t aodc;
    /** Square root of the semi-major axis
        Mandatory Field */
    double aSqrt;
    /** Mean motion difference
        Unit: radians/s
        Mandatory Field */
    double deltaNRadPerSec;
   /** Mean anomaly at reference time
        Unit: radians
        Mandatory Field */
    double m0Rad;
    /** Eccentricity
        Mandatory Field */
    double eccentricity;
    /** Longitude of ascending node of orbit plane at weekly epoch
        Unit: radians
        Mandatory Field */
    double omegaORad;
   /** Inclination angle at reference time
        Unit: radians
        Mandatory Field */
    double inclinationAngleRad;
    /** Argument of perigee
        Unit: radians
        Mandatory Field */
    double omegaRad;
    /** Rate of right ascension
        Unit: radians/s
        Mandatory Field */
    double omegaDotRadPerSec;
    /** Rate of inclination angle
        Unit: radians/s
        Mandatory Field */
    double iDotRadPerSec;
    /** Amplitude of the cosine harmonic correction term
        to the argument of latitude
        Unit: radians
        Mandatory Field */
    double cUcRad;
    /** Amplitude of the sine harmonic correction term
        to the argument of latitude
        Unit: radians
        Mandatory Field */
    double cUsRad;
    /** Amplitude of the cosine harmonic correction term
        to the orbit radius
        Unit: meter
        Mandatory Field */
    double cRcMeter;
    /** Amplitude of the sine harmonic correction term
        to the orbit radius
        Unit: meter
        Mandatory Field */
    double cRsMeter;
    /** Amplitude of the cosine harmonic correction term
        to the angle of inclination
        Unit: radians/s
        Mandatory Field */
    double cIcRad;
    /** Amplitude of the sine harmonic correction term
        to the angle of inclination (radians)
        Unit: radians/s
        Mandatory Field */
    double cIsRad;
  /** Reference time ephemeris
        Unit: second
        Mandatory Field */
    uint32_t fullToeSec;
   /** Clock data reference time
        Unit: second
        Mandatory Field */
    uint32_t fullTocSec;
    /** Clock bias correction coefficient
        Unit: second
        Mandatory Field */
    double af0Sec;
    /** Clock drift correction coefficient
        Unit: seconds/s
        Mandatory Field */
    double af1;
    /** Clock drift rate correction coefficient
        Unit: seconds/s^2
        Mandatory Field */
    double af2;
    /** Equipment group delay differential for the B1 signal
        Unit: nenoseconds
        Mandatory Field */
    double tgd1Ns;
    /** Equipment group delay differential for the B2 signal
        Unit: nenoseconds
        Mandatory Field */
    uint8_t urai;
} epDiagBDSSatelliteEphemerisData;

/** GALILEO Navigation model Info */
TYPEDEF_PACKED_STRUCT {
    /**  GNSS SV ID.
         For GAL:      301 to 336
         Mandatory Field */
    uint16_t gnssSvId;
    /** SV Health Status of signal identified by dataSource.
        Mandatory Field */
    uint8_t svHealth;
    /** GALILEO Signal Type
        Mandatory Field */
    epDiagGalSVDataSource dataSource;
    /** Ephemeris and clock correction issue of data
        Mandatory Field */
    uint16_t iodNav;
    /** Mean motion difference
        Unit: radians/s
        Mandatory Field */
    double deltaNRadPerSec;
    /** Mean anomaly at reference time
        Unit: radians
        Mandatory Field */
    double m0Rad;
    /** Eccentricity
        Mandatory Field */
    double eccentricity;
    /** Square root of the semi-major axis
        Mandatory Field */
    double aSqrt;
    /** Longitude of ascending node of orbit plane at weekly epoch
        Unit: radians
        Mandatory Field */
    double omegaORad;
    /** Inclination angle at reference time
        Unit: radians
        Mandatory Field */
    double inclinationAngleRad;
    /** Argument of perigee
        Unit: radians
        Mandatory Field */
    double omegaRad;
    /** Rate of right ascension
        Unit: radians/s
        Mandatory Field */
    double omegaDotRadPerSec;
    /** Rate of inclination angle
        Unit: radians/s
        Mandatory Field */
    double iDotRadPerSec;
    /** Amplitude of the cosine harmonic correction term
        to the argument of latitude
        Unit: radians
        Mandatory Field */
    double cUcRad;
    /** Amplitude of the sine harmonic correction term
        to the argument of latitude
        Unit: radians
        Mandatory Field */
    double cUsRad;
    /** Amplitude of the cosine harmonic correction term
        to the orbit radius
        Unit: meter
        Mandatory Field */
    double cRcMeter;
    /** Amplitude of the sine harmonic correction term
        to the orbit radius
        Unit: meter
        Mandatory Field */
    double cRsMeter;
    /** Amplitude of the cosine harmonic correction term
        to the angle of inclination
        Unit: radians/s
        Mandatory Field */
    double cIcRad;
    /** Amplitude of the sine harmonic correction term
        to the angle of inclination (radians)
        Unit: radians/s
        Mandatory Field */
    double cIsRad;
    /** Reference time ephemeris
        Unit: second
        Mandatory Field */
    uint32_t fullToeSec;
    /** Clock data reference time
        Unit: s
        Mandatory Field */
    uint32_t fullTocSec;
    /** SV Clock bias correction coefficient
        Unit: seconds
        Mandatory Field */
    double af0Sec;
    /** SV Clock drift correction coefficient
        Unit: seconds/s
        Mandatory Field */
    double af1;
    /** SV Clock drift rate correction coefficient
        Unit: seconds/s^2
        Mandatory Field */
    double af2;
    /** E1-E5ab group delay from F/Nav (E5a).
        Unit: seconds
        Mandatory Field */
    double bgdE1E5a;
    /** E1-E5b group delay from I/Nav (E1B or E5b). For E1B or E5b signal,
        both bgdE1E5a and bgdE1E5b are vailid. For E5a signal,
        only bgdE1E5a is valid. Signal source identified by dataSource.
        Unit: seconds
        Mandatory Field */
    double bgdE1E5b;
    /** Signal-in-space accuracy index for dual frequency
        E1-E5b/E5a depending on dataSource
        Mandatory Field */
    uint8_t sisaIndex;
    /* Validity Mask for Optional Parameters */
    epDiagGalileoEphemerisValidityMask validityMask;
} epDiagGalileoSatelliteEphemerisData;

/** Represents QZSS Navigation model info */
TYPEDEF_PACKED_STRUCT {
    /**  QZSS SV ID.
         For QZSS:      193 to 202
         Mandatory Field */
    uint16_t gnssSvId;
    /** SV Health
        Mandatory Field */
    uint8_t svHealthMask;
    /** Issue of Data (ephemeris)
        Mandatory Field */
    uint16_t iode;
    /** Square root of the semi-major axis
        Mandatory Field */
    double aSqrt;
    /** Mean motion difference
        Unit: radians/s
        Mandatory Field */
    double deltaNRadPerSec;
    /** Mean anomaly at reference time
        Unit: radians
        Mandatory Field */
    double m0Rad;
    /** Eccentricity
        Mandatory Field */
    double eccentricity;
    /** Longitude of ascending node of orbit plane at weekly epoch
        Unit: radians
        Mandatory Field */
    double omegaORad;
    /** Inclination angle at reference time
        Unit: radians
        Mandatory Field */
    double inclinationAngleRad;
    /** Argument of perigee
        Unit: radians
        Mandatory Field */
    double omegaRad;
    /** Rate of right ascension
        Unit: radians/s
        Mandatory Field */
    double omegaDotRadPerSec;
    /** Rate of inclination angle
        Unit: radians/s
        Mandatory Field */
    double iDotRadPerSec;
    /** Amplitude of the cosine harmonic correction term
        to the argument of latitude
        Unit: radians
        Mandatory Field */
    double cUcRad;
    /** Amplitude of the sine harmonic correction term
        to the argument of latitude
        Unit: radians
        Mandatory Field */
    double cUsRad;
    /** Amplitude of the cosine harmonic correction term
        to the orbit radius
        Unit: meter
        Mandatory Field */
    double cRcMeter;
    /** Amplitude of the sine harmonic correction term
        to the orbit radius
        Unit: meter
        Mandatory Field */
    double cRsMeter;
    /** Amplitude of the cosine harmonic correction term
        to the angle of inclination
        Unit: radians
        Mandatory Field */
    double cIcRad;
    /** Amplitude of the sine harmonic correction term
        to the angle of inclination (radians)
        Unit: radians
        Mandatory Field */
    double cIsRad;
    /** Reference time ephemeris
        Unit: s
        Mandatory Field */
    uint32_t fullToeSec;
    /** Clock data reference time
        Unit: s
        Mandatory Field */
    uint32_t fullTocSec;
    /** Clock bias correction coefficient
        Unit: seconds
        Mandatory Field */
    double af0Sec;
    /** Clock drift correction coefficient
        Unit: seconds/s
        Mandatory Field */
    double af1;
    /** Clock drift rate correction coefficient
        Unit: seconds/s^2
        Mandatory Field */
    double af2;
    /** User Range Accuracy Index(4bit)
        Mandatory Field */
    uint8_t urai;
    /** codes on L2 channel
        Mandatory Field */
    uint8_t codeL2;
    /** L2 P data flag
        Mandatory Field */
    uint8_t L2PData;
    /** Indicates the curve-fit interval used by the CS
        Unit: hours
        Range: 0 = 0-4 hours
               1 = greater than 4 hours
        Mandatory Field */
    uint8_t fitInterval;
    /** Time of Group delay
        Unit:seconds
        Mandatory Field */
    double  tgd;
    uint16_t iodc;
} epDiagQzssSatelliteEphemerisData;


/** GPS Navigation model for each SV */
TYPEDEF_PACKED_STRUCT {
    uint16_t numOfEphemeris;
    epDiagGpsSatelliteEphemerisData gpsEphemerisData[32];
} epDiagGpsEphemerisResponse;

/** GLONASS Navigation model for each SV */
TYPEDEF_PACKED_STRUCT {
    uint16_t numOfEphemeris;
    epDiagGlonassSatelliteEphemerisData gloEphemerisData[24];
} epDiagGlonassEphemerisResponse;

/** BDS Navigation model for each SV */
TYPEDEF_PACKED_STRUCT {
    uint16_t numOfEphemeris;
    epDiagBDSSatelliteEphemerisData bdsEphemerisData[37];
} epDiagBdsEphemerisResponse;

/** GALILEO Navigation model for each SV */
TYPEDEF_PACKED_STRUCT {
    uint16_t numOfEphemeris;
    epDiagGalileoSatelliteEphemerisData galEphemerisData[36];
} epDiagGalileoEphemerisResponse;

/** QZSS Navigation model for each SV */
TYPEDEF_PACKED_STRUCT {
    uint16_t numOfEphemeris;
    epDiagQzssSatelliteEphemerisData qzssEphemerisData[5];
} epDiagQzssEphemerisResponse;

/** Navigation model */
TYPEDEF_PACKED_STRUCT {
    epDiagLogGenericHeader header;
    /** Indicates GNSS Constellation Type
        Mandatory field */
    epDiagGnssConstellationTypeMask gnssConstellation;
    union {
       /** GPS Ephemeris */
       epDiagGpsEphemerisResponse gpsEphemeris;
       /** GLONASS Ephemeris */
       epDiagGlonassEphemerisResponse glonassEphemeris;
       /** BDS Ephemeris */
       epDiagBdsEphemerisResponse bdsEphemeris;
       /** GALILEO Ephemeris */
       epDiagGalileoEphemerisResponse galileoEphemeris;
       /** QZSS Ephemeris */
       epDiagQzssEphemerisResponse qzssEphemeris;
    } ephInfo;
    epDiagGnssSystemTimeStructType gpsSystemTime;
} epDiagGnssEphemerisReport;

/** Set applicable bits */
typedef uint32_t epDiagPositionFlags;
/** Position Report uses GNSS signal */
#define EP_DIAG_POSITION_FLAG_GNSS_USED      ((epDiagPositionFlags)0x00000001)
/** Position Report uses WIFI */
#define EP_DIAG_POSITION_FLAG_WIFI_USED      ((epDiagPositionFlags)0x00000002)
/** Position Report uses Cell-ID */
#define EP_DIAG_POSITION_FLAG_CELLID_USED    ((epDiagPositionFlags)0x00000004)
/** Position Report is DGNSS corrected */
#define EP_DIAG_POSITION_FLAG_DGNSS_CORR     ((epDiagPositionFlags)0x00000008)
/** Position Report is RTK corrected */
#define EP_DIAG_POSITION_FLAG_RTK_CORR       ((epDiagPositionFlags)0x00000010)
/** Position Report is PPP corrected */
#define EP_DIAG_POSITION_FLAG_PPP_CORR       ((epDiagPositionFlags)0x00000020)
/** Position Report uses Sensors  */
#define EP_DIAG_POSITION_FLAG_DR             ((epDiagPositionFlags)0x00000040)
/** Position Report uses SBAS Iono correction  */
#define EP_DIAG_POSITION_FLAG_SBAS_IONO_CORR ((epDiagPositionFlags)0x00000080)
/** Position Report uses SBAS Fast correction */
#define EP_DIAG_POSITION_FLAG_SBAS_FAST_CORR ((epDiagPositionFlags)0x00000100)
/** Position Report uses SBAS Long correction */
#define EP_DIAG_POSITION_FLAG_SBAS_LONG_CORR ((epDiagPositionFlags)0x00000200)
/** Position report is unpropagated.
    Sent when client subscribes for UNPROPAGATED_PVT report */
#define EP_DIAG_POSITION_FLAG_UNPROPAGATED_PVT ((epDiagPositionFlags)0x00000400)
/** Position Report uses RTK fixed solution */
#define EP_DIAG_POSITION_FLAG_RTK_FIXED        ((epDiagPositionFlags)0x00000800)

/** Position fix status */
typedef uint8_t epDiagSolutionStatus;
/** No Position fix */
#define EP_DIAG_NO_FIX ((epDiagSolutionStatus) 0)
/** Position fix is valid */
#define EP_DIAG_VALID_FIX ((epDiagSolutionStatus) 1)

/** UTC Time stamp. Units: Milliseconds since Jan. 1, 1970 */
typedef uint64_t epDiagUtcTimestampMs;

/** Validity bits for fields in epPVTReport */
TYPEDEF_PACKED_STRUCT {
    uint64_t isUtcTimestampMsValid : 1;
    uint64_t isLatitudeValid : 1;
    uint64_t isLongitudeValid : 1;
    uint64_t isAltitudeWrtEllipsoidValid : 1;
    uint64_t isAltitudeWrtMeanSeaLevelValid : 1;
    uint64_t isNorthStdDeviationValid : 1;
    uint64_t isEastStdDeviationValid : 1;
    uint64_t isAltitudeStdDeviationValid : 1;
    uint64_t isHorizontalSpeedValid : 1;
    uint64_t isHorizontalSpeedUncValid : 1;
    uint64_t isNorthVelocityValid : 1;
    uint64_t isEastVelocityValid : 1;
    uint64_t isUpVelocityValid : 1;
    uint64_t isNorthVelocityStdDeviationValid : 1;
    uint64_t isEastVelocityStdDeviationValid : 1;
    uint64_t isUpVelocityStdDeviationValid : 1;
    uint64_t isHeadingDegValid : 1;
    uint64_t isHeadingUncDegValid : 1;
    uint64_t isMagneticDeviationDegValid : 1;
    uint64_t isHorizontalEllipticalUncSemiMajorAxisValid : 1;
    uint64_t isHorizontalEllipticalUncSemiMinorAxisValid : 1;
    uint64_t isHorizontalEllipticalUncAZValid : 1;
    uint64_t isClockbiasValid : 1;
    uint64_t isClockBiasStdDeviationValid : 1;
    uint64_t isClockDriftValid : 1;
    uint64_t isClockDriftStdDeviationValid : 1;
    uint64_t isPdopValid : 1;
    uint64_t isHdopValid : 1;
    uint64_t isVdopValid : 1;
    uint64_t isGdopValid : 1;
    uint64_t isTdopValid : 1;
    uint64_t isLongAccelValid : 1;
    uint64_t isLatAccelValid : 1;
    uint64_t isVertAccelValid : 1;
    uint64_t isYawRateValid : 1;
    uint64_t isPitchRadValid : 1;
    uint64_t isReferenceStationValid : 1;
    uint64_t isLongAccelUncValid : 1;
    uint64_t isLatAccelUncValid : 1;
    uint64_t isVertAccelUncValid : 1;
    uint64_t isYawRateUncValid : 1;
    uint64_t isPitchRadUncValid : 1;
    uint64_t isPpsLocaltimeStampValid: 1;
    uint64_t isHeadingRateDegValid: 1;
    uint64_t : 20;
}  epDiagPVTReportValidity;

/** GNSS SV Carrier phase ambiguity Type */
typedef uint8_t epDiagCarrierPhaseAmbiguityType;
/** Carrier phase ambiguity not present */
#define EP_DIAG_CARRIER_PHASE_AMBIGUITY_RESOLUTION_NONE ((epDiagCarrierPhaseAmbiguityType)0)
/** Carrier phase ambiguity is present and float */
#define EP_DIAG_CARRIER_PHASE_AMBIGUITY_RESOLUTION_FLOAT ((epDiagCarrierPhaseAmbiguityType)1)
/** Carrier phase ambiguity is present and fix */
#define EP_DIAG_CARRIER_PHASE_AMBIGUITY_RESOLUTION_FIXED ((epDiagCarrierPhaseAmbiguityType)2)

/** GNSS Measurement Report Status Bit Mask */
typedef uint16_t epDiagGnssMeasUsageStatusBitMask;
/** Used in fix */
#define EP_DIAG_GNSS_MEAS_USED_IN_PVT ((epDiagGnssMeasUsageStatusBitMask)0x00000001)
/** Measurement is Bad */
#define EP_DIAG_GNSS_MEAS_USAGE_STATUS_BAD_MEAS ((epDiagGnssMeasUsageStatusBitMask)0x00000002)
/** Measurement has too low C/N */
#define EP_DIAG_GNSS_MEAS_USAGE_STATUS_CNO_TOO_LOW ((epDiagGnssMeasUsageStatusBitMask)0x00000004)
/** Measurement has too low elevation */
#define EP_DIAG_GNSS_MEAS_USAGE_STATUS_ELEVATION_TOO_LOW ((epDiagGnssMeasUsageStatusBitMask)0x00000008)
/** No ephemeris available for this measurement */
#define EP_DIAG_GNSS_MEAS_USAGE_STATUS_NO_EPHEMERIS ((epDiagGnssMeasUsageStatusBitMask)0x00000010)
/** No corrections available for the measurement */
#define EP_DIAG_GNSS_MEAS_USAGE_STATUS_NO_CORRECTIONS ((epDiagGnssMeasUsageStatusBitMask)0x00000020)
/** Corrections has timed out for the measurement */
#define EP_DIAG_GNSS_MEAS_USAGE_STATUS_CORRECTION_TIMEOUT ((epDiagGnssMeasUsageStatusBitMask)0x00000040)
/** Measurement is unhealthy */
#define EP_DIAG_GNSS_MEAS_USAGE_STATUS_UNHEALTHY ((epDiagGnssMeasUsageStatusBitMask)0x00000080)
/** Configuration is disabled for this measurement */
#define EP_DIAG_GNSS_MEAS_USAGE_STATUS_CONFIG_DISABLED ((epDiagGnssMeasUsageStatusBitMask)0x00000100)
/** Measurement not used for other reasons */
#define EP_DIAG_GNSS_MEAS_USAGE_STATUS_OTHER ((epDiagGnssMeasUsageStatusBitMask)0x00000200)

/** GNSS Measurement Usage Info */
TYPEDEF_PACKED_STRUCT {
    /** Specifies one GNSS signal type
        Mandatory Field*/
    epDiagGnssSignalTypeMask gnssSignalType;
    /** Specifies one GNSS Constellation Type
        Mandatory Field */
    epDiagGnssConstellationTypeMask gnssConstellation;
    /**  GNSS SV ID.
         For GPS:      1 to 32
         For GLONASS:  65 to 96. When slot-number to SV ID mapping is unknown, set as 255.
         For SBAS:     120 to 151
         For QZSS-L1CA:193 to 197
         For BDS:      201 to 237
         For GAL:      301 to 336
         Mandatory Field */
    uint16_t gnssSvId;
    /** GLONASS frequency number + 7.
        Valid only for a GLONASS system and
        is to be ignored for all other systems.
        Range: 1 to 14 */
    uint8_t gloFrequency;
    /** Carrier phase ambiguity type.
        Mandatory Field */
    epDiagCarrierPhaseAmbiguityType carrierPhaseAmbiguityType;
    /** Specifies measurement usage status
        Mandatory Field */
    epDiagGnssMeasUsageStatusBitMask measUsageStatusMask;

} epDiagMeasUsageInfo;

/** Indicates PVT report calculated by which Position engine */
typedef uint8_t epDiagPVTSourceType;
/** Position Engine source is unknown */
#define EP_DIAG_POSITION_SOURCE_UNKNOWN ((epDiagPVTSourceType)0x00)
/** Position source is GNSS Modem Engine */
#define EP_DIAG_POSITION_SOURCE_SPE     ((epDiagPVTSourceType)0x01)
/** Position source is precise position engine */
#define EP_DIAG_POSITION_SOURCE_PPE     ((epDiagPVTSourceType)0x02)
/** Position source is Dead-Reckoning Engine */
#define EP_DIAG_POSITION_SOURCE_DRE     ((epDiagPVTSourceType)0x03)




/** Position velocity Time Info */
TYPEDEF_PACKED_STRUCT {
    epDiagLogGenericHeader header;
    /** Specifies position flags.
        Mandatory field */
    epDiagPositionFlags posFlags;
    /** Status of PVT report.
        Mandatory field */
    epDiagSolutionStatus statusOfFix;
    /** Time applicability of PVT report */
    struct PACKED_POST {
      /** Specifies GNSS system time reported.
          Mandatory field */
      epDiagGnssConstellationTypeMask gnssSystemTimeSrc;
      /** Reporting of GPS system time is recommended.
         If GPS time is unknown & other satellite system time is known,
         it should be reported.
         Mandatory field
      */
      union {
          epDiagGnssSystemTimeStructType gpsSystemTime;
          epDiagGnssSystemTimeStructType galSystemTime;
          epDiagGnssSystemTimeStructType bdsSystemTime;
          epDiagGnssSystemTimeStructType qzssSystemTime;
          epDiagGnssGloTimeStructType gloSystemTime;
      }u;
    } gnssSystemTime;
    /** UTC time stamp. Mandatory, if Leap Second is known.
        Unit: Milliseconds since Jan. 1, 1970 */
    epDiagUtcTimestampMs utcTimestampMs;
    /** Local time of last PPS pulse, associated with this PVT report
        Mandatory field */
    epDiagGnssApTimeStampStructType lastPpsLocaltimeStamp;
    /** Validity bits for fields in epPVTReport */
    epDiagPVTReportValidity validityMask;
    /** Latitude (specified in WGS84 datum).
        Units: Degrees
        Range: -90.0 to 90.0
        Positive values indicate northern latitude
        Negative values indicate southern latitude */
    double latitudeDeg;
    /** Longitude (specified in WGS84 datum).
        Units: Degrees
        Range: -180.0 to 180.0
        Positive values indicate eastern longitude
        Negative values indicate western longitude */
    double longitudeDeg;
    /** Altitude with respect to the WGS84 ellipsoid.
        Unit:Meters. Range: -500 to 15883 */
    float altitudeWrtEllipsoid;
    /** Altitude with respect to mean sea level.
        Unit: Meters */
    float altitudeWrtMeanSeaLevel;
    /** North standard deviation.
        Unit: Meters */
    float northStdDeviation;
    /** East standard deviation.
        Unit: Meters */
    float eastStdDeviation;
    /** Altitude standard deviation.
        Unit: Meters */
    float altitudeStdDeviation;
    /** Horizontal Speed.
        Unit: Meters/sec */
    float horizontalSpeed;
    /** Horizontal Speed uncertainty
        Unit: Meters/sec */
    float horizontalSpeedUnc;
    /** North Velocity.
        Unit: Meters/sec */
    float northVelocity;
    /** East Velocity.
        Unit: Meters/sec */
    float eastVelocity;
    /** Up Velocity.
        Unit: Meters/sec */
    float upVelocity;
    /** North Velocity standard deviation.
        Unit: Meters/sec */
    float northVelocityStdDeviation;
    /** East Velocity standard deviation.
        Unit: Meters/sec */
    float eastVelocityStdDeviation;
    /** Up Velocity standard deviation
        Unit: Meters/sec */
    float upVelocityStdDeviation;
    /** Heading angle, referenced to North (0 deg) and positive clock-wise.
        Range: 0 to 359.999. Unit: Degrees.
        Check for headingUnc before use */
    float headingDeg;
    /** Heading uncertainty. Range: 0 to 359.999. Unit: Degrees */
    float headingUncDeg;
    /** Difference between the bearing to true north and the bearing shown
        on a magnetic compass. The deviation is positive when the magnetic
        north is east of true north.
        Unit: Degrees */
    float magneticDeviationDeg;
    /** Semi-major axis of horizontal elliptical uncertainty.
        Unit: Meters */
    float horUncEllipseSemiMajor;
    /** Semi-minor axis of horizontal elliptical uncertainty.
        Unit: Meters */
    float horUncEllipseSemiMinor;
    /** Elliptical horizontal uncertainty azimuth of orientation.
        Range: 0 to 180
        Units: Decimal degrees */
    float horUncEllipseOrientAzDeg;
    /** Estimated clock bias. Unit: Meter */
    float clockbiasMeter;
    /** Estimated clock bias std deviation. Unit: Meter */
    float clockBiasStdDeviationMeter;
    /** Estimated clock drift. Unit: Meters/sec */
    float clockDrift;
    /** Estimated clock drift std deviation. Unit: Meters/sec */
    float clockDriftStdDeviation;
    /** Position dilution of precision.
        Range: 1 (highest accuracy) to 50 (lowest accuracy) */
    float pDop;
    /** Horizontal dilution of precision.
        Range: 1 (highest accuracy) to 50 (lowest accuracy) */
    float hDop;
    /** Vertical dilution of precision.
        Range: 1 (highest accuracy) to 50 (lowest accuracy) */
    float vDop;
    /** Geometric  dilution of precision.
        Range: 1 (highest accuracy) to 50 (lowest accuracy) */
    float gDop;
    /** Time dilution of precision.
        Range: 1 (highest accuracy) to 50 (lowest accuracy) */
    float tDop;
    /** Forward Acceleration in body frame
        Unit: Meter/sec^2 */
    float longAccel;
    /** Side-ward Acceleration in body frame
        Unit: Meter/sec^2 */
    float latAccel;
    /** Vertical Acceleration in body frame
        Unit: Meter/sec^2 */
    float vertAccel;
    /** Heading Rate
        Unit: Radian/sec */
    float yawRate;
    /** Body pitch
        Unit: Radian */
    float pitchRad;
    /** Number of valid reference stations. Range:[0-4] */
    uint8_t numValidRefStations;
    /** Reference station(s) number */
    uint16_t referenceStation[4];
    /** Number of measurements received for use in fix.
        Shall be used as maximum index in-to svUsageInfo[].
        Set to 0, if svUsageInfo reporting is not supported.
        Range: 0-EP_DIAG_GNSS_MAX_MEAS */
    uint8_t numOfMeasReceived;
    /** Measurement Usage Information */
    epDiagMeasUsageInfo measUsageInfo[EP_DIAG_GNSS_MAX_MEAS];
    epDiagPVTSourceType pvtSource;
    /** Uncertainty of Forward Acceleration in body frame */
    float longAccelUnc;
    /** Uncertainty of Side-ward Acceleration in body frame */
    float latAccelUnc;
    /** Uncertainty of Vertical Acceleration in body frame */
    float vertAccelUnc;
    /** Uncertainty of Heading Rate */
    float yawRateUnc;
    /** Uncertainty of Body pitch */
    float pitchRadUnc;
    /** Heading Rate is in NED frame.
        Range: 0 to 359.999.
        Unit: Degrees per Seconds */
    float headingRateDeg;
} epDiagPVTReport;

/** Type of aiding data */
typedef uint32_t epDiagAidingdataMask;
/** All Aiding Data including Vendor */
#define EP_DIAG_DELETE_ALL_AIDING_DATA       ((epDiagAidingdataMask)0x00000001)
/** Delete GPS Ephemeris data */
#define EP_DIAG_DELETE_GPS_EPHEMERIS_DATA    ((epDiagAidingdataMask)0x00000100)
/** Delete GLO Ephemeris data */
#define EP_DIAG_DELETE_GLO_EPHEMERIS_DATA    ((epDiagAidingdataMask)0x00000200)
/** Delete BDS Ephemeris data */
#define EP_DIAG_DELETE_BDS_EPHEMERIS_DATA    ((epDiagAidingdataMask)0x00000400)
/** Delete GAL Ephemeris data */
#define EP_DIAG_DELETE_GAL_EPHEMERIS_DATA    ((epDiagAidingdataMask)0x00000800)
/** Delete QZSS Ephemeris data */
#define EP_DIAG_DELETE_QZSS_EPHEMERIS_DATA   ((epDiagAidingdataMask)0x00001000)
/** Delete GPS Polynomial data */
#define EP_DIAG_DELETE_GPS_POLYNOMIAL_DATA   ((epDiagAidingdataMask)0x00100000)
/** Delete GLO Polynomial data */
#define EP_DIAG_DELETE_GLO_POLYNOMIAL_DATA   ((epDiagAidingdataMask)0x00200000)
/** Delete BDS Polynomial data */
#define EP_DIAG_DELETE_BDS_POLYNOMIAL_DATA   ((epDiagAidingdataMask)0x00400000)
/** Delete GAL Polynomial data */
#define EP_DIAG_DELETE_GAL_POLYNOMIAL_DATA   ((epDiagAidingdataMask)0x00800000)
/** Delete QZSS Polynomial data */
#define EP_DIAG_DELETE_QZSS_POLYNOMIAL_DATA  ((epDiagAidingdataMask)0x01000000)
/** Delete Klobuchar ionospheric data of GPS */
#define EP_DIAG_DELETE_GPS_IONO_DATA         ((epDiagAidingdataMask)0x02000000)
/** Delete Klobuchar ionospheric data of BDS */
#define EP_DIAG_DELETE_BDS_IONO_DATA         ((epDiagAidingdataMask)0x04000000)
/** Delete Klobuchar ionospheric data of QZSS */
#define EP_DIAG_DELETE_QZSS_IONO_DATA        ((epDiagAidingdataMask)0x08000000)

/** GNSS Session control commands */
typedef enum {
    /** Session start request */
    EP_DIAG_COMMAND_START = 1,
    /** Session stop request */
    EP_DIAG_COMMAND_STOP,
    /** Session position mode set request */
    EP_DIAG_COMMAND_SET_POSITION_MODE,
    /** Engine reset request
        Recommended to be used when engine library is not responding */
    EP_DIAG_COMMAND_RESET,
    /** Delete aiding data stored in the engine */
    EP_DIAG_COMMAND_DELETE_AIDING_DATA
} epDiagCommandType;

/** Position mode command parameters  */
TYPEDEF_PACKED_STRUCT {
    /** GNSS session interval in millisecond
        Value-0 indicates Single shot session
        Non zero value indicates tracking session with interval specified
        Valid only when command is
        COMMAND_SET_POSITION_MODE command
        Mandatory Field */
    unsigned int interval;
} epDiagSetPositionModeStructType;

/** Delete Aiding command parameters  */
TYPEDEF_PACKED_STRUCT {
    /** Flag indicating aiding data to delete
        Valid in case of COMMAND_DELETE_AIDING_DATA command
        Mandatory Field */
    epDiagAidingdataMask flags;
} epDeleteDiagAidingDataStructType;

/** Engine Plug-in Command parameters */
TYPEDEF_PACKED_STRUCT {
    epDiagLogGenericHeader header;
    /** Session control command which includes start, stop, deleteAidData
        Mandatory Field */
    epDiagCommandType command;
    /** Command parameters for set position mode
        and delete Aiding data commands */
    union {
      /** Parameters for Set position command */
      epDiagSetPositionModeStructType  positionModeInfo;
      /** Parameters for Delete Aiding Data */
      epDeleteDiagAidingDataStructType deleteAidingData;
    } cmdReqParameter;
} epDiagCommandRequest;

/** GNSS Klobuchar Iono Data Source */
typedef enum {
/** Data source is GPS */
    EP_DIAG_IONO_DATA_SOURCE_GPS = 1,
/** Data source is BDS */
    EP_DIAG_IONO_DATA_SOURCE_BDS,
/** Data source is QZSS */
    EP_DIAG_IONO_DATA_SOURCE_QZSS
} epDiagGnssKlobucharIonoDataSource;

TYPEDEF_PACKED_STRUCT {
    epDiagLogGenericHeader header;
    /** GPS time when these ionospheric model parameters are sent. */
    epDiagGnssSystemTimeStructType gpsSystemTime;
    /** Iono Data Source */
    epDiagGnssKlobucharIonoDataSource ionoDataSource;
    /** Klobuchar model parameter - alpha0
        Unit: second
        Mandatory Field */
    float ionoAlpha0;
    /** Klobuchar model parameter - alpha1
        Unit: second/semi-circle
        Mandatory Field */
    float ionoAlpha1;
    /** Klobuchar model parameter - alpha2
        Unit: second/semi-circle^2
        Mandatory Field */
    float ionoAlpha2;
    /** Klobuchar model parameter - alpha3
        Unit: second/semi-circle^3
        Mandatory Field */
    float ionoAlpha3;
    /** Klobuchar model parameter - Beta0
        Unit: second
        Mandatory Field */
    float ionoBeta0;
    /** Klobuchar model parameter - Beta1
        Unit: second/semi-circle
        Mandatory Field */
    float ionoBeta1;
    /** Klobuchar model parameter - Beta2
        Unit: second/semi-circle^2
        Mandatory Field */
    float ionoBeta2;
    /** Klobuchar model parameter - Beta3
        Unit: second/semi-circle^3
        Mandatory Field */
    float ionoBeta3;
} epDiagKlobucharIonoModel;

/** Validity bits for the fields in epGloAdditionalParameter */
TYPEDEF_PACKED_STRUCT {
    uint8_t isTauCSecValid : 1;
    uint8_t isLeapSecValid : 1;
    uint8_t : 6;
} epDiagGloAdditionalParameterType;

/** Glonass additional parameters */
TYPEDEF_PACKED_STRUCT {
    epDiagLogGenericHeader header;
    /** GPS time when these Glonass additional parameters are sent.
        Mandatory Field */
    epDiagGnssSystemTimeStructType gpsSystemTime;
    /** Validity bits for following fields */
    epDiagGloAdditionalParameterType validityMask;
    /** Glonass time scale correction to UTC(SU) time as in Glonass ICD Edition 5.1 page 36
        Unit: second */
    double tauCSec;
    /** Leap seconds (GPS-UTC) from GPS
        Unit: second */
    int8_t leapSec;
} epDiagGloAdditionalParameter;

/**  Engine Plug-in Report subscription Mask
     Clients shall use the mask bits to subscribe to specific reports
     of interest
*/
typedef uint64_t epDiagReportSubscriptionMask;
/** Subscribe to 1Hz PVT report. Only the PVT reports aligned to integer
   second will be reported */
#define EP_DIAG_SUBSCRIPTION_PVT_1HZ_REPORT ((epDiagReportSubscriptionMask)0x00000001)
/** Subscribe to N-Hz PVT report. All PVT reports will be reported */
#define EP_DIAG_SUBSCRIPTION_PVT_NHZ_REPORT ((epDiagReportSubscriptionMask)0x00000002)
/** Subscribe to 1Hz measurement report. */
#define EP_DIAG_SUBSCRIPTION_MEASUREMENT_1HZ_REPORT ((epDiagReportSubscriptionMask)0x00000004)
/** Subscribe to N-Hz measurement report. */
#define EP_DIAG_SUBSCRIPTION_MEASUREMENT_NHZ_REPORT ((epDiagReportSubscriptionMask)0x00000008)
/** Subscribe to polynomial report. */
#define EP_DIAG_SUBSCRIPTION_POLYNOMIAL_REPORT  ((epDiagReportSubscriptionMask)0x00000010)
/** Subscribe to ephemeris report. */
#define EP_DIAG_SUBSCRIPTION_EPHEMERIS_REPORT   ((epDiagReportSubscriptionMask)0x00000020)
/** Subscribe Unpropagated 1Hz PVT report. */
#define EP_DIAG_SUBSCRIPTION_UNPROPAGATED_PVT_1HZ_REPORT \
               ((epDiagReportSubscriptionMask)0x00000040)
/** Subscribe to Correction data. */
#define EP_DIAG_SUBSCRIPTION_CORRECTION_DATA \
               ((epDiagReportSubscriptionMask)0x00000100)
/** Subscribe to Ionospheric model reports. */
#define EP_DIAG_SUBSCRIPTION_IONO_MODEL_REPORT \
               ((epDiagReportSubscriptionMask)0x00000200)
/** Subscription request command parameters */
TYPEDEF_PACKED_STRUCT {
    /** Subscribe Constellation Type
        Mandatory Field */
    epDiagGnssConstellationTypeMask constellationType;
    epDiagGnssSignalTypeMask gnssSignalMask;
} epDiagSubscriptionModifier;

/** Subscribe required reports from Engine Plug-in */
TYPEDEF_PACKED_STRUCT {
    epDiagLogGenericHeader header;
    /** Report subscription mask
        Mandatory Field */
    epDiagReportSubscriptionMask flags;
    /** Modifier applicable to subscription of measurement,
        ephemeris, polynomial */

    /** Subscription modifier for Measurement. It is valid only
        when EP_DIAG_SUBSCRIPTION_MEASUREMENT_1HZ_REPORT set */
    epDiagSubscriptionModifier subscriptionModMeasurement;
    /** Subscription modifier for Measurement It is valid only
        when EP_SUBSCRIPTION_MEASUREMENT_NHZ_REPORT set */
    epDiagSubscriptionModifier subscriptionModMeasurementNHz;
    /** Subscription modifier for Ephemeris It is valid only
        when EP_SUBSCRIPTION_EPHEMERIS_REPORT set */
    epDiagSubscriptionModifier subscriptionModEphemeris;
    /** Subscription modifier for Polynomial It is valid only
        when EP_SUBSCRIPTION_POLYNOMIAL_REPORT set */
    epDiagSubscriptionModifier subscriptionModPolynomial;
    /** Subscription modifier for Ionospheric Model. Valid only
        when EP_SUBSCRIPTION_IONO_MODEL_REPORT subscription
        is requested.
        Engine Client shall subscribe to constellations of interest and ignore gnssSignal field.
        Note: Only the Ionospheric models from GPS, BDS and QZSS satellite
              system are supported. Ionospheric model from other satellite
              systems are not supported. */
    epDiagSubscriptionModifier subscriptionModIONO;
} epDiagSubscriptionInfo;

/** Engine Plugin Error */
typedef uint32_t epDiagError;
/** Indicates Engine Plug-in interface file version mismatch. */
#define EP_DIAG_ENGINE_PLUGIN_INTERFACE_FILE_VERSION_MISMATCH ((epDiagError)0x00000001)
/** Engine Plug-in is not supporting Measurement. */
#define EP_DIAG_SUBSCRIPTION_MEASUREMENT_REPORT_NOT_SUPPORTED ((epDiagError)0x00000002)
/** Engine Plug-in is not Polynomial. */
#define EP_DIAG_SUBSCRIPTION_POLYNOMIAL_REPORT_NOT_SUPPORTED  ((epDiagError)0x00000004)
/** Engine Plug-in is not Ephemeris. */
#define EP_DIAG_SUBSCRIPTION_EPHEMERIS_REPORT_NOT_SUPPORTED   ((epDiagError)0x00000008)
/** Engine Plug-in does not support 1 Hz Position report subscription requested */
#define EP_DIAG_SUBSCRIPTION_PVT_1HZ_REPORT_NOT_SUPPORTED ((epDiagError)0x00000010)
/** Engine Plug-in does not support N Hz Position report subscription requested */
#define EP_DIAG_SUBSCRIPTION_PVT_NHZ_REPORT_NOT_SUPPORTED ((epDiagError)0x00000020)
/** Engine Plug-in does not support N Hz Measurement report subscription requested */
#define EP_DIAG_SUBSCRIPTION_MEASUREMENT_NHZ_REPORT_NOT_SUPPORTED ((epDiagError)0x00000040)
/** Engine Plug-in does not support 1 Hz Unpropagated Position report subscription requested */
#define EP_DIAG_SUBSCRIPTION_UNPROPAGATED_PVT_1HZ_REPORT_NOT_SUPPORTED ((epDiagError)0x00000080)
/** Engine Plug-in does not support Correction data subscription requested */
#define EP_DIAG_SUBSCRIPTION_CORRECTION_DATA_NOT_SUPPORTED ((epDiagError)0x00000100)
/** Engine Plug-in does not support iono model subscription requested */
#define EP_DIAG_SUBSCRIPTION_IONO_MODEL_REPORT_NOT_SUPPORTED ((epDiagError)0x00000200)

TYPEDEF_PACKED_STRUCT {
    epDiagLogGenericHeader header;
    epDiagError error;
}epDiagLogError;

TYPEDEF_PACKED_STRUCT {
    epDiagLogGenericHeader header;
    uint32_t status;
} epDiagReportStatusRespInfo;


#ifdef _WIN32
#pragma pack(pop)
#endif
#endif /*ENGINE_PLUGIN_LOG_H */
