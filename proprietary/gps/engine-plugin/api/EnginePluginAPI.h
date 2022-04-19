/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

EnginePluginAPI.h

GENERAL DESCRIPTION
Interface file for Engine Plug-in API

=============================================================================*/


#ifndef ENGINE_PLUGIN_API_H
#define ENGINE_PLUGIN_API_H

#include <stdint.h>
#include <time.h>

/**
    Engine Plug-in Interface file version is defined using major and minor version.
    A Major version could be associated with non-backward compatible update.
    A change to only Minor version is generally backward compatible to header
    file having same Major version number.

    Note: Clients shall verify version number for compatibility.

    The backward compatibility of APIs is achieved by fixed size reserved array
    at end of each structure. Whenever new parameter is added, reserved array size
    is reduced by corresponding size of new parameters.
    This ensures size of structure remains the same.

    Note: In this header file, following terms are referenced interchangeably.
    GAL and GALILEO are referenced interchangeably.
    GLO and GLONASS are referenced interchangeably.
    BDS and BEIDOU  are referenced interchangeably.
*/

/** Major File Version */
#define EP_INTERFACE_FILE_MAJOR_VERSION  6
/** Minor File Version */
#define EP_INTERFACE_FILE_MINOR_VERSION  0

/** Type of Log Level */
typedef uint32_t epLogLevelType;
/** Log Level is Error */
#define EP_LOG_LEVEL_ERROR    ((epLogLevelType)0x00000001)
/** Log Level is Info */
#define EP_LOG_LEVEL_INFO     ((epLogLevelType)0x00000002)
/** Log Level is Warning */
#define EP_LOG_LEVEL_WARNING  ((epLogLevelType)0x00000003)
/** Log Level is Debug */
#define EP_LOG_LEVEL_DEBUG    ((epLogLevelType)0x00000004)
/** Log Level is Verbose */
#define EP_LOG_LEVEL_VERBOSE  ((epLogLevelType)0x00000005)

/** Type of Engine log */
typedef uint32_t epLogType;
/** Log of type debug */
#define EP_LOG_TYPE_DEBUG            ((epLogType)0x00000001)
/** Log of type Engine playback data */
#define EP_LOG_TYPE_PLAYBACK         ((epLogType)0x00000002)
/** Log of type NMEA sentence */
#define EP_LOG_TYPE_NMEA             ((epLogType)0x00000003)
/** Log of type RTCM service status */
#define EP_LOG_TYPE_RTCM_STATUS      ((epLogType)0x00000004)

/** Type of aiding data */
typedef uint32_t epAidingdataMask;

/** Delete all aiding data, including ephemeris, polynomial, iono,
    corrections and GLO additional parameters from both non-volatile
    and resident memory and internal registers such that any subsequent
    COMMAND_START will result in cold start. */
#define EP_DELETE_ALL_AIDING_DATA       ((epAidingdataMask)0x00000001)
/** Delete GPS Ephemeris data */
#define EP_DELETE_GPS_EPHEMERIS_DATA    ((epAidingdataMask)0x00000100)
/** Delete GLO Ephemeris data and additional parameters defined by
    epProvideGloAdditionalParam */
#define EP_DELETE_GLO_EPHEMERIS_DATA    ((epAidingdataMask)0x00000200)
/** Delete BDS Ephemeris data */
#define EP_DELETE_BDS_EPHEMERIS_DATA    ((epAidingdataMask)0x00000400)
/** Delete GAL Ephemeris data */
#define EP_DELETE_GAL_EPHEMERIS_DATA    ((epAidingdataMask)0x00000800)
/** Delete QZSS Ephemeris data */
#define EP_DELETE_QZSS_EPHEMERIS_DATA   ((epAidingdataMask)0x00001000)
/** Delete GPS Polynomial data */
#define EP_DELETE_GPS_POLYNOMIAL_DATA   ((epAidingdataMask)0x00100000)
/** Delete GLO Polynomial data */
#define EP_DELETE_GLO_POLYNOMIAL_DATA   ((epAidingdataMask)0x00200000)
/** Delete BDS Polynomial data */
#define EP_DELETE_BDS_POLYNOMIAL_DATA   ((epAidingdataMask)0x00400000)
/** Delete GAL Polynomial data */
#define EP_DELETE_GAL_POLYNOMIAL_DATA   ((epAidingdataMask)0x00800000)
/** Delete QZSS Polynomial data */
#define EP_DELETE_QZSS_POLYNOMIAL_DATA  ((epAidingdataMask)0x01000000)
/** Delete Klobuchar ionospheric data of GPS */
#define EP_DELETE_GPS_IONO_DATA         ((epAidingdataMask)0x02000000)
/** Delete Klobuchar ionospheric data of BDS */
#define EP_DELETE_BDS_IONO_DATA         ((epAidingdataMask)0x04000000)
/** Delete Klobuchar ionospheric data of QZSS */
#define EP_DELETE_QZSS_IONO_DATA        ((epAidingdataMask)0x08000000)


/** Connection Type */
typedef uint32_t epConnectionStatus;

/** No Data Connection */
#define EP_NO_CONNECTION      ((epConnectionStatus)0x00000000)
/** Data Connection using WiFi */
#define EP_WIFI_CONNECTION    ((epConnectionStatus)0x00000001)
/** Data Connection using Mobile */
#define EP_MOBILE_CONNECTION  ((epConnectionStatus)0x00000002)
/** Data Connection using other than WiFi and Mobile */
#define EP_OTHER_CONNECTION   ((epConnectionStatus)0x00000004)

/** Engine Plug-in Error */
typedef uint32_t epError;
/** Engine Plug-in interface header file version mismatch */
#define EP_ENGINE_PLUGIN_INTERFACE_FILE_VERSION_MISMATCH ((epError)0x00000001)
/** Engine Plug-in does not support 1 Hz Measurement report subscription requested */
#define EP_SUBSCRIPTION_MEASUREMENT_1HZ_REPORT_NOT_SUPPORTED ((epError)0x00000002)
/** Engine Plug-in does not support Polynomial report subscription requested */
#define EP_SUBSCRIPTION_POLYNOMIAL_REPORT_NOT_SUPPORTED  ((epError)0x00000004)
/** Engine Plug-in does not support Ephemeris report subscription requested */
#define EP_SUBSCRIPTION_EPHEMERIS_REPORT_NOT_SUPPORTED   ((epError)0x00000008)
/** Engine Plug-in does not support 1 Hz Position report subscription requested */
#define EP_SUBSCRIPTION_PVT_1HZ_REPORT_NOT_SUPPORTED ((epError)0x00000010)
/** Engine Plug-in does not support N Hz Position report subscription requested */
#define EP_SUBSCRIPTION_PVT_NHZ_REPORT_NOT_SUPPORTED ((epError)0x00000020)
/** Engine Plug-in does not support N Hz Measurement report subscription requested */
#define EP_SUBSCRIPTION_MEASUREMENT_NHZ_REPORT_NOT_SUPPORTED ((epError)0x00000040)
/** Engine Plug-in does not support 1 Hz Unpropagated Position report subscription requested */
#define EP_SUBSCRIPTION_UNPROPAGATED_PVT_1HZ_REPORT_NOT_SUPPORTED ((epError)0x00000080)
/** Engine Plug-in does not support Correction data subscription requested */
#define EP_SUBSCRIPTION_CORRECTION_DATA_NOT_SUPPORTED ((epError)0x00000100)
/** Engine Plug-in does not support iono model subscription requested */
#define EP_SUBSCRIPTION_IONO_MODEL_REPORT_NOT_SUPPORTED ((epError)0x00000200)


/**  Engine Plug-in Report subscription Mask
     Clients shall use the mask bits to subscribe to specific reports
     of interest
*/
typedef uint64_t epReportSubscriptionMask;
/** Subscribe to 1Hz PVT report. Only the PVT reports aligned to integer
   second will be reported */
#define EP_SUBSCRIPTION_PVT_1HZ_REPORT ((epReportSubscriptionMask)0x00000001)
/** Subscribe to N-Hz PVT report. All PVT reports will be reported */
#define EP_SUBSCRIPTION_PVT_NHZ_REPORT ((epReportSubscriptionMask)0x00000002)
/** Subscribe to 1Hz measurement report. */
#define EP_SUBSCRIPTION_MEASUREMENT_1HZ_REPORT ((epReportSubscriptionMask)0x00000004)
/** Subscribe to N-Hz measurement report. */
#define EP_SUBSCRIPTION_MEASUREMENT_NHZ_REPORT ((epReportSubscriptionMask)0x00000008)
/** Subscribe to polynomial report. */
#define EP_SUBSCRIPTION_POLYNOMIAL_REPORT  ((epReportSubscriptionMask)0x00000010)
/** Subscribe to ephemeris report. */
#define EP_SUBSCRIPTION_EPHEMERIS_REPORT   ((epReportSubscriptionMask)0x00000020)
/** Subscribe to Unpropagated 1Hz PVT report. */
#define EP_SUBSCRIPTION_UNPROPAGATED_PVT_1HZ_REPORT \
               ((epReportSubscriptionMask)0x00000040)
/** Subscribe to Correction data. */
#define EP_SUBSCRIPTION_CORRECTION_DATA \
               ((epReportSubscriptionMask)0x00000100)
/** Subscribe to Ionospheric model reports. */
#define EP_SUBSCRIPTION_IONO_MODEL_REPORT \
               ((epReportSubscriptionMask)0x00000200)

/** Subscribe to all reports. */
#define EP_SUBSCRIPTION_ALL_REPORT (EP_SUBSCRIPTION_PVT_1HZ_REPORT|\
      EP_SUBSCRIPTION_PVT_NHZ_REPORT| EP_SUBSCRIPTION_MEASUREMENT_1HZ_REPORT|\
      EP_SUBSCRIPTION_MEASUREMENT_NHZ_REPORT|EP_SUBSCRIPTION_POLYNOMIAL_REPORT|\
      EP_SUBSCRIPTION_EPHEMERIS_REPORT)

/** GNSS Signal Type and RF Band */
typedef uint32_t epGnssSignalTypeMask;
/*** Unknown Signal */
#define EP_GNSS_SIGNAL_UNKNOWN        ((epGnssSignalTypeMask)0x00000000)
/** GPS L1 CA-Code Signal */
#define EP_GNSS_SIGNAL_GPS_L1CA       ((epGnssSignalTypeMask)0x00000001)
/** GPS L1C RF Band */
#define EP_GNSS_SIGNAL_GPS_L1C        ((epGnssSignalTypeMask)0x00000002)
/** GPS L2 RF Band */
#define EP_GNSS_SIGNAL_GPS_L2C_L      ((epGnssSignalTypeMask)0x00000004)
/** GPS L5 Q-Code RF Band */
#define EP_GNSS_SIGNAL_GPS_L5_Q       ((epGnssSignalTypeMask)0x00000008)
/** GLONASS G1 (L1OF) RF Band */
#define EP_GNSS_SIGNAL_GLONASS_G1_CA  ((epGnssSignalTypeMask)0x00000010)
/** GLONASS G2 (L2OF) RF Band */
#define EP_GNSS_SIGNAL_GLONASS_G2_CA  ((epGnssSignalTypeMask)0x00000020)
/** GALILEO E1 MBOC Pilot RF Band */
#define EP_GNSS_SIGNAL_GALILEO_E1_C   ((epGnssSignalTypeMask)0x00000040)
/** GALILEO E5A BPSK Pilot RF Band */
#define EP_GNSS_SIGNAL_GALILEO_E5A_Q  ((epGnssSignalTypeMask)0x00000080)
/** GALILEO E5B RF Band */
#define EP_GNSS_SIGNAL_GALILIEO_E5B_Q ((epGnssSignalTypeMask)0x00000100)
/** BEIDOU B1_I RF Band */
#define EP_GNSS_SIGNAL_BEIDOU_B1_I    ((epGnssSignalTypeMask)0x00000200)
/** BEIDOU B1_C RF Band */
#define EP_GNSS_SIGNAL_BEIDOU_B1_C    ((epGnssSignalTypeMask)0x00000400)
/** BEIDOU B2_I RF Band */
#define EP_GNSS_SIGNAL_BEIDOU_B2_I    ((epGnssSignalTypeMask)0x00000800)
/** BEIDOU B2A_I RF Band */
#define EP_GNSS_SIGNAL_BEIDOU_B2A_I   ((epGnssSignalTypeMask)0x00001000)
/** QZSS L1CA RF Band */
#define EP_GNSS_SIGNAL_QZSS_L1CA      ((epGnssSignalTypeMask)0x00002000)
/** QZSS L1S RF Band */
#define EP_GNSS_SIGNAL_QZSS_L1S       ((epGnssSignalTypeMask)0x00004000)
/** QZSS L2 RF Band */
#define EP_GNSS_SIGNAL_QZSS_L2C_L     ((epGnssSignalTypeMask)0x00008000)
/** QZSS L5 RF Band */
#define EP_GNSS_SIGNAL_QZSS_L5_Q      ((epGnssSignalTypeMask)0x00010000)
/** SBAS L1 RF Band */
#define EP_GNSS_SIGNAL_SBAS_L1_CA     ((epGnssSignalTypeMask)0x00020000)

/** GNSS Constellation type */
typedef uint16_t epGnssConstellationTypeMask;
/** Unknown Constellation */
#define EP_GNSS_CONSTELLATION_UNKNOWN ((epGnssConstellationTypeMask) 0x0000)
/** GPS Constellation */
#define EP_GNSS_CONSTELLATION_GPS ((epGnssConstellationTypeMask) 0x0001)
/** Galileo Constellation */
#define EP_GNSS_CONSTELLATION_GALILEO ((epGnssConstellationTypeMask) 0x0002)
/** SBAS Constellation */
#define EP_GNSS_CONSTELLATION_SBAS ((epGnssConstellationTypeMask) 0x0004)
/** GLONASS Constellation */
#define EP_GNSS_CONSTELLATION_GLONASS ((epGnssConstellationTypeMask) 0x0010)
/** BEIDOU Constellation */
#define EP_GNSS_CONSTELLATION_BEIDOU ((epGnssConstellationTypeMask) 0x0020)
/** QZSS Constellation */
#define EP_GNSS_CONSTELLATION_QZSS ((epGnssConstellationTypeMask) 0x0040)


/** Maximum number of GNSS SV measurement */
#define EP_GNSS_MAX_MEAS                  (128)

#define EP_TRIANGLE_COV_MATRIX_SIZE       (12)

/** Set applicable bits */
typedef uint32_t epPositionFlags;
/** Position Report uses GNSS signal */
#define EP_POSITION_FLAG_GNSS_USED      ((epPositionFlags)0x00000001)
/** Position Report uses WIFI */
#define EP_POSITION_FLAG_WIFI_USED      ((epPositionFlags)0x00000002)
/** Position Report uses Cell-ID */
#define EP_POSITION_FLAG_CELLID_USED    ((epPositionFlags)0x00000004)
/** Position Report is DGNSS corrected */
#define EP_POSITION_FLAG_DGNSS_CORR     ((epPositionFlags)0x00000008)
/** Position Report is RTK corrected */
#define EP_POSITION_FLAG_RTK_CORR       ((epPositionFlags)0x00000010)
/** Position Report is PPP corrected */
#define EP_POSITION_FLAG_PPP_CORR       ((epPositionFlags)0x00000020)
/** Position Report uses Sensors  */
#define EP_POSITION_FLAG_DR             ((epPositionFlags)0x00000040)
/** Position Report uses SBAS IONO correction  */
#define EP_POSITION_FLAG_SBAS_IONO_CORR ((epPositionFlags)0x00000080)
/** Position Report uses SBAS Fast correction */
#define EP_POSITION_FLAG_SBAS_FAST_CORR ((epPositionFlags)0x00000100)
/** Position Report uses SBAS Long correction */
#define EP_POSITION_FLAG_SBAS_LONG_CORR ((epPositionFlags)0x00000200)
/** Position report is unpropagated.
    Sent when client subscribes for UNPROPAGATED_PVT report */
#define EP_POSITION_FLAG_UNPROPAGATED_PVT ((epPositionFlags)0x00000400)
/** Position Report uses RTK fixed solution */
#define EP_POSITION_FLAG_RTK_FIXED        ((epPositionFlags)0x00000800)
/**
   Following table provides mapping of Solution type and
   corresponding Position Flags.

   |--------------------------------------------------|
   |            |      EP_POSITION_FLAG_              |
   |            |-------------------------------------|
   |            |GNSS_ |DGNSS_ | PPP_  | RTK_ | RTK_  |
   |            |USED  |CORR   | CORR  | CORR | FIXED |
   |--------------------------------------------------|
   | Standalone |   1  |   0   |   0   |  0   |   0   |
   |--------------------------------------------------|
   | DGNSS      |   1  |   1   |   0   |  0   |   0   |
   |--------------------------------------------------|
   | PPP        |   1  |   1   |   1   |  0   |   0   |
   |--------------------------------------------------|
   | RTK-Float  |   1  |   1   |   0   |  1   |   0   |
   |--------------------------------------------------|
   | RTK-Fix    |   1  |   1   |   0   |  1   |   1   |
   |--------------------------------------------------|

*/
/** Position Report Flag is Reserved */
#define EP_POSITION_FLAG_RESERVED_1   ((epPositionFlags)0x00001000)
/** Position Report Flag is Reserved */
#define EP_POSITION_FLAG_RESERVED_2   ((epPositionFlags)0x00002000)


/** Position mode DR only */
#define EP_POSITION_FLAG_DR_ONLY  (EP_POSITION_FLAG_DR)
/** Position mode GNSS+DR */
#define EP_POSITION_FLAG_GNSS_DR (EP_POSITION_FLAG_GNSS_USED | EP_POSITION_FLAG_DR)
/** Position mode GNSS only */
#define EP_POSITION_FLAG_GNSS_ONLY  (EP_POSITION_FLAG_GNSS_USED)
/** GNSS Unpropagated Position Report */
#define EP_POSITION_FLAG_GNSS_UNPROPAGATED_PVT\
           (EP_POSITION_FLAG_UNPROPAGATED_PVT | EP_POSITION_FLAG_GNSS_USED)
/** Position mode PPE only */
#define EP_POSITION_FLAG_PPE (EP_POSITION_FLAG_DGNSS_CORR |\
                                EP_POSITION_FLAG_RTK_CORR |\
                                EP_POSITION_FLAG_PPP_CORR)

/** Indicates client engine source for the PVT report */
typedef uint8_t epPVTSourceType;
/** Position Engine source is unknown */
#define EP_POSITION_SOURCE_UNKNOWN ((epPVTSourceType)0x00)
/** Position source is GNSS Modem Engine */
#define EP_POSITION_SOURCE_SPE     ((epPVTSourceType)0x01)
/** Position source is Precise Position Engine */
#define EP_POSITION_SOURCE_PPE     ((epPVTSourceType)0x02)
/** Position source is Dead-Reckoning Engine */
#define EP_POSITION_SOURCE_DRE     ((epPVTSourceType)0x03)
/** Position source is Reserved */
#define EP_POSITION_SOURCE_RESERVED ((epPVTSourceType)0x04)


/** GNSS Measurement Report Status Bit Mask */
typedef uint32_t epGnssMeasUsageStatusBitMask;
/** Measurement is used in the fix */
#define EP_GNSS_MEAS_USED_IN_PVT ((epGnssMeasUsageStatusBitMask)0x00000001)
/** Measurement is Bad */
#define EP_GNSS_MEAS_USAGE_STATUS_BAD_MEAS ((epGnssMeasUsageStatusBitMask)0x00000002)
/** Measurement has too low C/N */
#define EP_GNSS_MEAS_USAGE_STATUS_CNO_TOO_LOW ((epGnssMeasUsageStatusBitMask)0x00000004)
/** Measurement has too low elevation */
#define EP_GNSS_MEAS_USAGE_STATUS_ELEVATION_TOO_LOW ((epGnssMeasUsageStatusBitMask)0x00000008)
/** No ephemeris available for the measurement */
#define EP_GNSS_MEAS_USAGE_STATUS_NO_EPHEMERIS ((epGnssMeasUsageStatusBitMask)0x00000010)
/** No corrections available for the measurement */
#define EP_GNSS_MEAS_USAGE_STATUS_NO_CORRECTIONS ((epGnssMeasUsageStatusBitMask)0x00000020)
/** Corrections has timed out for the measurement */
#define EP_GNSS_MEAS_USAGE_STATUS_CORRECTION_TIMEOUT ((epGnssMeasUsageStatusBitMask)0x00000040)
/** Measurement is unhealthy */
#define EP_GNSS_MEAS_USAGE_STATUS_UNHEALTHY ((epGnssMeasUsageStatusBitMask)0x00000080)
/** Configuration is disabled for this measurement */
#define EP_GNSS_MEAS_USAGE_STATUS_CONFIG_DISABLED ((epGnssMeasUsageStatusBitMask)0x00000100)
/** Measurement not used for other reasons */
#define EP_GNSS_MEAS_USAGE_STATUS_OTHER ((epGnssMeasUsageStatusBitMask)0x00000200)

/** Maximum Size of SV Polynomial Velocity Coefficient  */
#define EP_SV_POLY_VELOCITY_COEF_MAX_SIZE         12
/** SV Polynomial Zeroth order Coefficient  */
#define EP_SV_POLY_XYZ_0_TH_ORDER_COEFF_MAX_SIZE  3
/** SV Polynomial nth order Coefficient  */
#define EP_SV_POLY_XYZ_N_TH_ORDER_COEFF_MAX_SIZE  9
/** SV Clock-bias Coefficient  */
#define EP_SV_POLY_SV_CLKBIAS_COEFF_MAX_SIZE      4

/** Polynomials Status Bit Mask */
typedef uint16_t epPolyStatusMask;
/** Polynomials based on XTRA */
#define EP_SV_POLY_SRC_ALM_CORR    ((epPolyStatusMask)0x01)
/** GLONASS string 4 has been received */
#define EP_SV_POLY_GLO_STR4        ((epPolyStatusMask)0x02)
/** GALILEO polynomial computed from I/Nav navigation message */
#define EP_SV_POLY_GAL_INAV        ((epPolyStatusMask)0x04)
/** GALILEO polynomial computed from F/Nav navigation message */
#define EP_SV_POLY_GAL_FNAV        ((epPolyStatusMask)0x08)


/** GNSS Measurement Status Mask */
typedef uint64_t epGNSSMeasurementStatusMask;
/** Satellite time in sub-millisecond (code phase) is known */
#define EP_MEAS_STATUS_SM_VALID              ((epGNSSMeasurementStatusMask)0x00000001)
/** Satellite sub-bit time is known */
#define EP_MEAS_STATUS_SB_VALID              ((epGNSSMeasurementStatusMask)0x00000002)
/** Satellite time in millisecond is known */
#define EP_MEAS_STATUS_MS_VALID              ((epGNSSMeasurementStatusMask)0x00000004)
/** Signal bit edge is confirmed  */
#define EP_MEAS_STATUS_BE_CONFIRM            ((epGNSSMeasurementStatusMask)0x00000008)
/** Satellite Doppler is measured */
#define EP_MEAS_STATUS_VELOCITY_VALID        ((epGNSSMeasurementStatusMask)0x00000010)
/** Status as Reserved */
#define EP_MEAS_STATUS_RESERVED              ((epGNSSMeasurementStatusMask)0x00000020)
/** TRUE/FALSE : Lock Point is valid/invalid.*/
#define EP_MEAS_STATUS_LP_VALID              ((epGNSSMeasurementStatusMask)0x00000040)
/** TRUE/FALSE : Lock Point is positive/negative*/
#define EP_MEAS_STATUS_LP_POS_VALID          ((epGNSSMeasurementStatusMask)0x00000080)
/** Range update from satellite differences is measured */
#define EP_MEAS_STATUS_FROM_RNG_DIFF         ((epGNSSMeasurementStatusMask)0x00000200)
/** Doppler update from satellite differences is measured */
#define EP_MEAS_STATUS_FROM_VE_DIFF          ((epGNSSMeasurementStatusMask)0x00000400)
/** Fresh GNSS measurement observed in last second  */
#define EP_MEAS_STATUS_GNSS_FRESH_MEAS_VALID ((epGNSSMeasurementStatusMask)0x08000000)

/** GNSS SV Status Mask */
typedef uint16_t epGnssSvStatusMask;
/** GNSS SV has Ephemeris */
#define EP_GNSS_SV_OPTIONS_HAS_EPHEMERIS_BIT ((epGnssSvStatusMask)0x01)
/** GNSS SV has Almanac */
#define EP_GNSS_SV_OPTIONS_HAS_ALMANAC_BIT ((epGnssSvStatusMask)0x02)
/** GNSS SV used in fix */
#define EP_GNSS_SV_OPTIONS_USED_IN_FIX_BIT ((epGnssSvStatusMask)0x04)

/** GALILEO Ephemeris Validity Mask */
typedef uint16_t epGalileoEphemerisValidityMask;
/** bgdE1E5a is valid */
#define EP_GALILEO_EPHEMERIS_BGDE1E5A ((epGalileoEphemerisValidityMask)0x01)
/** bgdE1E5b is valid */
#define EP_GALILEO_EPHEMERIS_BGDE1E5B ((epGalileoEphemerisValidityMask)0x02)


/** GNSS SV Carrier phase ambiguity Type */
typedef uint8_t epCarrierPhaseAmbiguityType;
/** Carrier phase ambiguity not present */
#define EP_CARRIER_PHASE_AMBIGUITY_RESOLUTION_NONE ((epCarrierPhaseAmbiguityType)0)
/** Carrier phase ambiguity is present and float */
#define EP_CARRIER_PHASE_AMBIGUITY_RESOLUTION_FLOAT ((epCarrierPhaseAmbiguityType)1)
/** Carrier phase ambiguity is present and fix */
#define EP_CARRIER_PHASE_AMBIGUITY_RESOLUTION_FIXED ((epCarrierPhaseAmbiguityType)2)

/** Position fix status */
typedef uint8_t epSolutionStatus;
/** No Position fix */
#define EP_NO_FIX ((epSolutionStatus) 0)
/** Position fix is valid */
#define EP_VALID_FIX ((epSolutionStatus) 1)


/** GNSS SV search status */
typedef uint8_t epGnssMeasSvSearchState;
/** SV is not being actively searched */
#define EP_GNSS_SV_SRCH_STATE_IDLE (1)
/** The system is searching for this SV */
#define EP_GNSS_SV_SRCH_STATE_SEARCH (2)
/** SV is being tracked */
#define EP_GNSS_SV_SRCH_STATE_TRACK (3)

/** GNSS SV Info Mask */
typedef uint8_t epGnssMeasSvInfoMask;
/** Ephemeris is available for this SV */
#define EP_GNSS_SVINFO_MASK_HAS_EPHEMERIS ((epGnssMeasSvInfoMask)0x01)
/** Almanac is available for this SV */
#define EP_GNSS_SVINFO_MASK_HAS_ALMANAC ((epGnssMeasSvInfoMask)0x02)

/** Path for configuration files */
#define EP_PPE_FILES_PATH "/data/vendor/location-partner/ppe/"
#define EP_DRE_FILES_PATH "/data/vendor/location-partner/dre/"

/** Base index for different satellite system IDs */
#define EP_SVID_OFFSET_GLONASS     (64)
#define EP_SVID_OFFSET_BDS         (200)
#define EP_SVID_OFFSET_GALILEO     (300)
#define EP_SVID_OFFSET_QZSS        (192)
#define EP_SVID_INVALID_VALUE_FOR_GLONASS (255)


/** Feature status */
typedef enum {
    /** No license found */
    EP_FEATURE_STATUS_NONE = 1,
    /** Valid license found */
    EP_FEATURE_STATUS_OK,
    /** Expired license found */
    EP_FEATURE_STATUS_EXPIRED
} epFeatureStatus;


typedef enum {
    /** No license found */
    EP_INSTALL_LICENSE_STATUS_NONE = 1,
    /** Valid license found */
    EP_INSTALL_LICENSE_STATUS_OK,
    /** Invalid license Parameter
        Report this error to license server */
    EP_ERROR_LICENSE_INVAILD_PARAMS,
    /** Invalid license Status
        Retry after reboot */
    EP_ERROR_LICENSE_INVAILD_STATUS,
    /** Invalid license No Memory
        Retry after reboot */
    EP_ERROR_LICENSE_NO_MEMORY,
    /** Expired license found
        Report this error to license server */
    EP_ERROR_LICENSE_CERTIFICATE_EXPIRED,
    /** Incorrect OEM ID for License certificate
        Report this error to license server */
    EP_ERROR_LICENSE_CERTIFICATE_OEM,
    /** Incorrect HW version for License certificate
        Report this error to license server */
    EP_ERROR_LICENSE_CERTIFICATE_HWVERSION,
    /** Incorrect Device ID for License certificate
        Report this error to license server */
    EP_ERROR_LICENSE_CERTIFICATE_DEVICEID,
    /** Incorrect PFM for License certificate
        Report this error to license server */
    EP_ERROR_INVALID_PFM_EXTENSION,
    /** Incorrect PFMFILER for License certificate
        Retry after reboot */
    EP_ERROR_PFMFILER_FAILD
} epInstallLicenseStatus;

/** GNSS Session control commands */
typedef enum {
    /** Start positioning session.
        Engine can use any previously stored non-volatile data. */
    EP_COMMAND_START = 1,
    /** Terminate ongoing positioning session.
        Engine can save any non-volatile data
        for use during any subsequent COMMAND_START request.

        After COMMAND_STOP, Engine Plugin may issue COMMAND_DELETE_AIDING_DATA to delete
        SV Polynomial, SV ephemerides, and Ionospheric model for all supported constellations.
        In response, the engine shall clear/invalidate stored SV Polynomial, SV ephemerides,
        Glo additional parameters, and Ionospheric model for all supported constellations.

        After COMMAND_STOP, Engine Plugin may also issue COMMAND_DELETE_AIDING_DATA with bit 0
        set as 1 to delete all aiding data. In response to this delete command, the engine shall
        clear/invalidate any previously stored aiding/non-volatile data and internal registers
        such that any subsequent COMMAND_START will result in cold start.
    */
    EP_COMMAND_STOP,
    /** Session position mode set request */
    EP_COMMAND_SET_POSITION_MODE,
    EP_COMMAND_RESERVE,
    /** Delete aiding data stored in the engine */
    EP_COMMAND_DELETE_AIDING_DATA,
    /** Max enum, add new enum above this line */
    EP_COMMAND_MAX = UINT8_MAX
} epCommandType;

typedef enum {
   /** License type is unknown */
   EP_LICENSE_UNDEFINED = 1,
   /** License type is Evaluation license */
   EP_LICENSE_EVALUATION,
   /** License type is Commercial Production license */
   EP_LICENSE_PRODUCTION
} epLicenseType;

typedef enum {
    /** Attestation token is created successfully */
    EP_ATTESTATION_STATUS_OK = 1,
    /** The security has been compromised
        Report this error to license server */
    EP_ATTESTATION_INVAILD_STATUS,
    /** Error no memory
        Retry after reboot */
    EP_ATTESTATION_NO_MEMORY
} epAttestationStatus;

typedef enum {
   /** Update Polynomial for this satellite */
   EP_UPDATE_POLY = 1,
   /** Invalidate any previously received polynomial */
   EP_INVALIDATE_POLY
} epPolyUpdateAction;

/** GALILEO Signal Type */
typedef enum {
/** GALILEO E1 MBOC Pilot RF Band */
   EP_E1B = 1,
/** GALILEO E5A BPSK Pilot RF Band */
   EP_E5A,
/** GALILEO E5B RF Band */
   EP_E5B
} epGalSVDataSource;

/** GNSS Measurement Report Type */
typedef enum {
/** GNSS Measurement 1 Hz Report */
    EP_MEASUREMENT_1HZ_REPORT = 1,
/** GNSS Measurement 10 Hz Report */
    EP_MEASUREMENT_NHZ_REPORT
} epGnssMeasurementReportType;


/** GNSS Klobuchar Iono Data Source */
typedef enum {
/** Data source is GPS */
    EP_IONO_DATA_SOURCE_GPS = 1,
/** Data source is BDS */
    EP_IONO_DATA_SOURCE_BDS,
/** Data source is QZSS */
    EP_IONO_DATA_SOURCE_QZSS
} epGnssKlobucharIonoDataSource;


/** Validity bits for the fields in epPVTReport */
typedef struct
{
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
    uint64_t isFullCovMatrixValid: 1;
    uint64_t : 19;
}  epPVTReportValidity;

/** Validity bits for the fields in epGnssSystemTimeStructType */
typedef struct {
    uint16_t isSystemWeekValid : 1;
    uint16_t isSystemWeekMsecValid : 1;
    uint16_t isSystemClkTimeBiasValid : 1;
    uint16_t isSystemClkTimeUncMsValid : 1;
    uint16_t isRefFCountValid : 1;
    uint16_t isNumClockResetsValid : 1;
    uint16_t isIntraSystemTimeBias1AndUncValid : 1;
    uint16_t : 9;
}epSystemTimeValidity;

/** Validity bits for the fields in epGnssGloTimeStructType */
typedef struct {
    uint16_t isGloClkTimeBiasValid : 1;
    uint16_t isGloClkTimeUncMsValid : 1;
    uint16_t isRefFCountValid : 1;
    uint16_t isNumClockResetsValid : 1;
    uint16_t isGloFourYearValid: 1;
    uint16_t isGloDaysValid: 1;
    uint16_t isGloMsecValid: 1;
    uint16_t : 9;
}epGloTimeValidity;

/** Validity bits for the fields in epGnssSvPolynomial */
typedef struct
{
    uint32_t isIodeValid : 1;
    uint32_t isT0Valid : 1;
    uint32_t isPolyCoeffXYZ0Valid : 1;
    uint32_t isPolyCoefXYZNValid :  1;
    uint32_t isPolyCoefOtherValid : 1;
    uint32_t isSvPosUncValid: 1;
    uint32_t isIonoDelayValid : 1;
    uint32_t isIonoDotValid : 1;
    uint32_t isSbasIonoDelayValid : 1;
    uint32_t isSbasIonoDotValid : 1;
    uint32_t isTropoDelayValid : 1;
    uint32_t isElevationValid : 1;
    uint32_t isElevationDotValid : 1;
    uint32_t isElevationUncValid : 1;
    uint32_t isVelCoefValid : 1;
    uint32_t isEnhancedIODValid : 1;
    uint32_t : 16;
}  epPolyReportValidity;


/** Validity bits for the fields in epGnssSVMeasurementStructType */
typedef struct {
    uint32_t isHealthStatusValid : 1;
    uint32_t isSvInfoMaskValid : 1;
    uint32_t isLossOfLockValid : 1;
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
    uint32_t : 18;
} epGnssSVMeasurementValidity;

/** Flags to indicate status of Loss of Lock Indicator */
typedef struct {
    uint8_t lossOfContinousLock : 1;
    uint8_t halfCycleAmbiguity : 1;
    uint8_t : 6;
} epGnssMeasSvLLI;


/** Validity bits for the fields in epGnssMeasurementReport */
typedef struct {
    uint16_t isLeapSecValid : 1;
    uint16_t isClockDriftValid : 1;
    uint16_t isClockDriftStdDeviationValid : 1;
    uint16_t isGpsSystemTimeValid : 1;
    uint16_t isGalSystemTimeValid : 1;
    uint16_t isBdsSystemTimeValid : 1;
    uint16_t isQzssSystemTimeValid : 1;
    uint16_t isGloSystemTimeValid : 1;
    uint16_t : 8;
} epGnssMeasurementReportValidity;

/** Validity bits for the fields in epGnssSv */
typedef struct {
    uint8_t isGloFrequencyValid : 1;
    uint8_t isElevationValid : 1;
    uint8_t isAzimuthValid : 1;
    uint8_t : 5;
} epGnssSvValidity;

/** Validity bits for the fields in epGloAdditionalParameter */
typedef struct {
    uint8_t isTauCSecValid : 1;
    uint8_t isLeapSecValid : 1;
    uint8_t : 6;
} epGloAdditionalParameterType;

/** validity bits for GPS Time Group Delay in epPolyGpsTgd */
typedef struct {
   /** validity bit of the ISC for GPS L1C/A Band.*/
   uint32_t isIscL1caValid : 1;
   /** validity bit of the ISC for GPS L2C Band.*/
   uint32_t isIscL2cValid : 1;
   /** validity bit of the ISC for GPS L5I5 Band.*/
   uint32_t isIscL5I5Valid : 1;
   /** validity bit of the ISC for GPS L5Q5 Band.*/
   uint32_t isIscL5Q5Valid : 1;
   /** validity bit of the TGD for GPS Band.*/
   uint32_t isTgdValid : 1;
   uint32_t : 27;
} epGpsTgdValidity;

/** validity bits for GLONASS Time Group Delay in epPolyGloTgd */
typedef struct {
   /** validity bit of the Time Group Delay for GLONASS G1-G2 Band.*/
   uint32_t isTgdG1G2Valid : 1;
   uint32_t : 31;
} epGloTgdValidity;

/** validity bits for BDS Time Group Delay in epPolyBdsTgd */
typedef struct {
   /** validity bit of the Time Group Delay for BDS B1 Band. */
   uint32_t isTgdB1Valid : 1;
   /** validity bit of the Time Group Delay for BDS B2 Band. */
   uint32_t isTgdB2Valid : 1;
   /** validity bit of the Time Group Delay for BDS B2A Band. */
   uint32_t isTgdB2AValid : 1;
   /** validity bit of the Inter Signal Correction for BDS B2A Band. */
   uint32_t isIscB2AValid : 1;
   uint32_t : 28;
} epBdsTgdValidity;

/** validity bits for GALILEO Time Group Delay in epPolyGalTgd */
typedef struct {
   /** validity bit of the Time Group Delay for GAL E1-E5a Band. */
   uint32_t isTgdE1E5aValid : 1;
   /** validity bit of the Time Group Delay for GAL E1-E5b Band. */
   uint32_t isTgdE1E5bValid : 1;
   uint32_t : 30;
} epGalTgdValidity;

/** validity bits for Navic Time Group Delay in epPolyNavicTgd */
typedef struct {
   /** validity bit of the Time Group Delay for Navic L5 Band. */
   uint32_t isTgdL5Valid : 1;
   uint32_t : 31;
} epNavicTgdValidity;

/** UTC Time stamp. Unit: millisecond since Jan. 1, 1970 */
typedef uint64_t epUtcTimestampMs;

/** Subscription request command parameters */
typedef struct {
    /** Subscribe Constellation Type
        Mandatory Field */
    epGnssConstellationTypeMask constellationType;

    /** Subscribe GNSS signal type.
        Note: For API backward compatibility reasons, if gnssSignal is set 0
        (i.e. EP_GNSS_SIGNAL_UNKNOWN), it will be mapped as
        request for L1 signal subscription.
        Mandatory Field */
    epGnssSignalTypeMask gnssSignal;

    uint8_t reserve[12];
} epSubscriptionModifier;

/** Subscribe required reports from Engine Plug-in */
typedef struct {
    /** Report subscription mask
        Mandatory Field */
    epReportSubscriptionMask flags;
    /** Modifier applicable to subscription of measurement,
        ephemeris, polynomial, ionospheric */

    /** Subscription modifier for Measurement. Valid only
        when EP_SUBSCRIPTION_MEASUREMENT_1HZ_REPORT subscription
        is requested */
    epSubscriptionModifier subscriptionModMeasurement;
    /** Subscription modifier for Measurement. Valid only
        when EP_SUBSCRIPTION_MEASUREMENT_NHZ_REPORT subscription
        is requested */
    epSubscriptionModifier subscriptionModMeasurementNHz;
    /** Subscription modifier for Ephemeris. Valid only
        when EP_SUBSCRIPTION_EPHEMERIS_REPORT subscription
        is requested.
        Engine Client shall subscribe to constellations and gnssSignal type of interest.
        GPS ephemeris will be sent when GPS is selected in constellationType.
        BDS ephemeris will be sent when BDS is selected in constellationType.
        GAL ephemeris will be sent, from decoded E1 signal, when GAL is selected
        in constellationType and EP_GNSS_SIGNAL_GALILEO_E1_C is selected in gnssSignal.
        GAL ephemeris will be sent, from decoded E5a signal, when GAL is selected
        in constellationType and EP_GNSS_SIGNAL_GALILEO_E5A_Q is selected in gnssSignal.
        GLO ephemeris will be sent when GLO is selected in constellationType.
        QZSS ephemeris will be sent when QZSS is selected in constellationType.
    */
    epSubscriptionModifier subscriptionModEphemeris;
    /** Subscription modifier for Polynomial. Valid only
        when EP_SUBSCRIPTION_POLYNOMIAL_REPORT subscription
        is requested */
    epSubscriptionModifier subscriptionModPolynomial;
    /** Subscription modifier for Ionospheric Model. Valid only
        when EP_SUBSCRIPTION_IONO_MODEL_REPORT subscription
        is requested.
        Engine Client shall subscribe to constellations of interest and ignore gnssSignal field.
        Note: Only the Ionospheric models from GPS, BDS and QZSS satellite
              system are supported. Ionospheric model from other satellite
              systems are not supported. */
    epSubscriptionModifier subscriptionModIONO;

    uint8_t reserve [16];
} epSubscriptionInfo;

/** Position mode command parameters  */
typedef struct {
    /** GNSS session interval in millisecond
        Value-0 indicates Single shot session
        Non zero value indicates tracking session with interval specified
        Valid only when command is
        COMMAND_SET_POSITION_MODE command
        Mandatory Field */
    unsigned int interval;
    uint8_t reserve[32];
} epSetPositionModeStructType;


/** Delete Aiding command parameters  */
typedef struct {
    /** Flag indicating aiding data to delete
        Valid in case of COMMAND_DELETE_AIDING_DATA command
        Mandatory Field */
    epAidingdataMask flags;
    uint8_t reserve[32];
} epDeleteAidingDataStructType;


/** Engine Plug-in Command parameters */
typedef struct {
    /** Session control command
        Mandatory Field */
    epCommandType command;
    /** Command parameters for set position mode
        and delete Aiding data commands */
    union {
      /** Parameters for Set position command */
      epSetPositionModeStructType  positionModeInfo;
      /** Parameters for Delete Aiding Data */
      epDeleteAidingDataStructType deleteAidingData;
    } cmdReqParameter;
    uint8_t reserve[32];
} epCommandRequest;

/** Indicates AP time stamp  */
typedef struct {
    /** Boot time received from pps-ktimer
        Mandatory Field */
    timespec apTimeStamp;
    /** Time-stamp uncertainty in Millisecond
        Mandatory Field */
    float apTimeStampUncertaintyMs;
    uint8_t reserve[32];
} epGnssApTimeStampStructType;

/** GNSS System Time */
typedef struct {
    /** Validity mask for below fields */
    epSystemTimeValidity validityMask;
    /** Extended week number at reference tick.
    Unit: week.
    Set to 65535 if week number is unknown.
    For GPS:
      Calculated from 00:00:00 UTC on Sunday January 6, 1980 (midnight between Jan 5 and Jan 6).
    For BDS:
      Calculated from 00:00:00 UTC on Sunday January 1, 2006 (midnight between Dec 31 and Jan 1).
    For GAL:
      Calculated from 00:00:00 UTC on Sunday August 22, 1999 (midnight between August 21 and August 22).
   */
    uint16_t systemWeek;
    /** Time in to the current week at reference tick.
       Unit: millisecond. Range: 0 to 604799999.
       Check for systemClkTimeUncMs before use */
    uint32_t systemMsec;
    /** System clock time bias (sub-millisecond)
        Unit: millisecond
        Note: System time (TOW Millisecond) = systemMsec - systemClkTimeBias.
        Check for systemClkTimeUncMs before use. */
    float systemClkTimeBias;
    /** Single sided maximum time bias uncertainty
        Unit: millisecond */
    float systemClkTimeUncMs;
    /** FCount (free running HW timer) value. Don't use for relative time purpose
         due to possible discontinuities.
         Unit: millisecond */
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
    uint8_t reserve[8];
} epGnssSystemTimeStructType;


/** GNSS GLONASS Time */
typedef struct {
    /** GLONASS day number in four years. Refer to GLONASS ICD.
        Applicable only for GLONASS and shall be ignored for other constellations.
        If unknown shall be set to 65535
        Mandatory Field */
    uint16_t gloDays;
    /** Validity mask for below fields */
    epGloTimeValidity validityMask;
    /** GLONASS time of day in Millisecond. Refer to GLONASS ICD.
        Unit: millisecond */
    uint32_t gloMsec;
    /** GLONASS clock time bias (sub-millisecond)
        Unit: millisecond
        Note: GLO time (TOD Millisecond) = gloMsec - gloClkTimeBias.
        Check for gloClkTimeUncMs before use. */
    float gloClkTimeBias;
    /** Single sided maximum time bias uncertainty
        Unit: millisecond */
    float gloClkTimeUncMs;
    /** FCount (free running HW timer) value. Don't use for relative time purpose
        due to possible discontinuities.
        Unit: millisecond */
    uint32_t  refFCount;
    /** Number of clock resets/discontinuities detected, affecting the local hardware counter value. */
    uint32_t numClockResets;
    /** GLONASS four year number from 1996. Refer to GLONASS ICD.
        Applicable only for GLONASS and shall be ignored for other constellations.
        If unknown shall be set to 255 */
    uint8_t gloFourYear;
    uint8_t reserve[16];
} epGnssGloTimeStructType;


/** GNSS Measurement Usage Info */
typedef struct {
    /** Specifies a GNSS signal type
        Mandatory Field*/
    epGnssSignalTypeMask gnssSignalType;
    /** Specifies a GNSS Constellation Type
        Mandatory Field */
    epGnssConstellationTypeMask gnssConstellation;
    /**  GNSS SV ID.
         For GPS:      1 to 32
         For GLONASS:  65 to 96. When slot-number to SV ID mapping is unknown, set as 255.
         For SBAS:     120 to 151
         For QZSS-L1CA:193 to 197
         For BDS:      201 to 237
         For GAL:      301 to 336
         Mandatory Field */
    uint16_t gnssSvId;
    /** GLONASS frequency number + 8.
        Valid only for a GLONASS system and
        is to be ignored for all other systems.
        Range: 1 to 14 */
    uint8_t gloFrequency;
    /** Carrier phase ambiguity type.
        Mandatory Field */
    epCarrierPhaseAmbiguityType carrierPhaseAmbiguityType;
    /** Specifies measurement usage status
        Mandatory Field */
    epGnssMeasUsageStatusBitMask measUsageStatusMask;
    uint8_t reserve[32];
} epMeasUsageInfo;


/** Position velocity Time Info */
typedef struct {
    /** Specifies position flags.
        Mandatory field */
    epPositionFlags posFlags;
    /** Status of PVT report.
        Mandatory field */
    epSolutionStatus statusOfFix;
    /** Time applicability of PVT report */
    struct {
      /** Specifies GNSS system time reported.
          Mandatory field */
      epGnssConstellationTypeMask gnssSystemTimeSrc;
      /** Reporting of GPS system time is recommended.
         If GPS time is unknown & other satellite system time is known,
         it should be reported.
         Mandatory field
      */
      union {
          epGnssSystemTimeStructType gpsSystemTime;
          epGnssSystemTimeStructType galSystemTime;
          epGnssSystemTimeStructType bdsSystemTime;
          epGnssSystemTimeStructType qzssSystemTime;
          epGnssGloTimeStructType gloSystemTime;
      }u;
    } epGnssSystemTime;
    /** UTC time stamp. Mandatory, if Leap Second is known.
        Unit: millisecond since Jan. 1, 1970 */
    epUtcTimestampMs utcTimestampMs;
    /** Please ignore this field; included for information purposes only.
        Local time of last PPS pulse, associated with this PVT report
        SPE provides PPS TimeStamp */
    epGnssApTimeStampStructType lastPpsLocaltimeStamp;
    /** Validity bits for fields in epPVTReport */
    epPVTReportValidity validityMask;
    /** Latitude (specified in WGS84 datum).
        Unit: degree
        Range: -90.0 to 90.0
        Positive values indicate northern latitude
        Negative values indicate southern latitude */
    double latitudeDeg;
    /** Longitude (specified in WGS84 datum).
        Unit: degree
        Range: -180.0 to 180.0
        Positive values indicate eastern longitude
        Negative values indicate western longitude */
    double longitudeDeg;
    /** Altitude with respect to the WGS84 ellipsoid.
        Unit: meter. Range: -500 to 15883 */
    float altitudeWrtEllipsoid;
    /** Altitude with respect to mean sea level.
        Unit: meter */
    float altitudeWrtMeanSeaLevel;
    /** North standard deviation.
        Unit: meter */
    float northStdDeviation;
    /** East standard deviation.
        Unit: meter */
    float eastStdDeviation;
    /** Altitude standard deviation.
        Unit: meter */
    float altitudeStdDeviation;
    /** Horizontal Speed.
        Unit: meter/second */
    float horizontalSpeed;
    /** Horizontal Speed uncertainty
        Uncertainty is defined with 68% confidence level
        Unit: meter/second */
    float horizontalSpeedUnc;
    /** North Velocity.
        Unit: meter/second */
    float northVelocity;
    /** East Velocity.
        Unit: meter/second */
    float eastVelocity;
    /** Up Velocity.
        Unit: meter/second */
    float upVelocity;
    /** North Velocity standard deviation.
        Unit: meter/second */
    float northVelocityStdDeviation;
    /** East Velocity standard deviation.
        Unit: meter/second */
    float eastVelocityStdDeviation;
    /** Up Velocity standard deviation
        Unit: meter/second */
    float upVelocityStdDeviation;
    /** Heading angle, referenced to North (0 deg) and positive clock-wise.
        Range: 0 to 359.999. Unit: degree.
        Check for headingUnc before use */
    float headingDeg;
    /** Heading uncertainty.
        Uncertainty is defined with 68% confidence level.
        Range: 0 to 359.999.
        Unit: degree */
    float headingUncDeg;
    /** Difference between the bearing to true north and the bearing shown
        on a magnetic compass. The deviation is positive when the magnetic
        north is east of true north.
        Unit: degree */
    float magneticDeviationDeg;
    /** Semi-major axis of horizontal elliptical uncertainty.
        Uncertainty is defined with 39% confidence level.
        Unit: meter */
    float horUncEllipseSemiMajor;
    /** Semi-minor axis of horizontal elliptical uncertainty.
        Uncertainty is defined with 39% confidence level.
        Unit: meter */
    float horUncEllipseSemiMinor;
    /** Elliptical horizontal uncertainty azimuth of orientation.
        Range: 0 to 180
        Units: Decimal degrees */
    float horUncEllipseOrientAzDeg;
    /** Estimated clock bias. Unit: meter */
    float clockbiasMeter;
    /** Estimated clock bias std deviation. Unit: meter */
    float clockBiasStdDeviationMeter;
    /** Estimated clock drift. Unit: meter/second */
    float clockDrift;
    /** Estimated clock drift std deviation. Unit: meter/second */
    float clockDriftStdDeviation;
    /** Position dilution of precision.
        Range: 0 (highest accuracy) to 50 (lowest accuracy) */
    float pDop;
    /** Horizontal dilution of precision.
        Range: 0 (highest accuracy) to 50 (lowest accuracy) */
    float hDop;
    /** Vertical dilution of precision.
        Range: 0 (highest accuracy) to 50 (lowest accuracy) */
    float vDop;
    /** Geometric  dilution of precision.
        Range: 0 (highest accuracy) to 50 (lowest accuracy) */
    float gDop;
    /** Time dilution of precision.
        Range: 0 (highest accuracy) to 50 (lowest accuracy) */
    float tDop;
    /** Forward Acceleration in body frame
        Unit: meter/second^2 */
    float longAccel;
    /** Side-ward Acceleration in body frame
        Unit: meter/second^2 */
    float latAccel;
    /** Vertical Acceleration in body frame
        Unit: meter/second^2 */
    float vertAccel;
    /**  Yaw rate is in body frame
        Unit: radian/second */
    float yawRate;
    /** Body pitch
        Unit: radian */
    float pitchRad;
    /** Number of valid reference stations. Range:[0-4] */
    uint8_t numValidRefStations;
    /** Reference station(s) number */
    uint16_t referenceStation[4];
    /** Number of measurements received for use in fix.
        Shall be used as maximum index in-to svUsageInfo[].
        Set to 0, if svUsageInfo reporting is not supported.
        Range: 0-EP_GNSS_MAX_MEAS */
    uint8_t numOfMeasReceived;
    /** Measurement Usage Information */
    epMeasUsageInfo measUsageInfo[EP_GNSS_MAX_MEAS];
    /** Engine information generating this PVT.
        Shall be filled EnginePlugin component.
        Mandatory field */
    epPVTSourceType pvtSource;
    /** Uncertainty of Forward Acceleration in body frame
        Uncertainty is defined with 68% confidence level */
    float longAccelUnc;
    /** Uncertainty of Side-ward Acceleration in body frame
        Uncertainty is defined with 68% confidence level */
    float latAccelUnc;
    /** Uncertainty of Vertical Acceleration in body frame
        Uncertainty is defined with 68% confidence level */
    float vertAccelUnc;
    /** Uncertainty of Heading Rate
        Uncertainty is defined with 68% confidence level */
    float yawRateUnc;
    /** Uncertainty of Body pitch
        Uncertainty is defined with 68% confidence level */
    float pitchRadUnc;
    /** Heading Rate is in NED frame.
        Range: 0 to 359.999.
        Unit: degree/second */
    float headingRateDeg;
    /*** Upper triangle elements of full matrix of position
         and velocity estimate in ECEF

         The full covariance matrix of PPE position
         (x, y, z in ECEF, in the unit of meters) estimate is a 3x3 matrix
            | px,x  px,y  px,z |
            | py,x  py,y  py,z |
            | pz,x  pz,y  pz,z |

         The full covariance matrix of PPE velocity
         (vx,vy, vz in ECEF, in the unit of m/s) estimate is a 3x3 matrix
            | pvx,vx  pvx,vy  pvx,vz |
            | pvy,vx  pvy,vy  pvy,vz |
            | pvz,vx  pvz,vy  pvz,vz |

        upperTriangleFullCovMatrix =
          { px,x, px,y, px,z, py,y, py,z, pz,z, pvx,vx, pvx,vy, pvx,vz, pvy,vy, pvy,vz, pvz,vz}
        Uint: px,x, px,y, px,z, py,y, py,z, pz,z is in meter
              pvx,vx, pvx,vy, pvx,vz, pvy,vy, pvy,vz, pvz,vz is in meters/seconds
    */
    float upperTriangleFullCovMatrix[EP_TRIANGLE_COV_MATRIX_SIZE];
    uint8_t reserve[32];
} epPVTReport;


/** Represents GPS Navigation model info */
typedef struct {
    /**  GNSS SV ID.
         For GPS:      1 to 32
         Mandatory Field */
    uint16_t gnssSvId;
    /** The six-bit health indication as defined in GPS ICD
        (https://www.gps.gov/technical/icwg/IS-GPS-200H.pdf Table 20-VIII.in
        section 20.3.3.5.1.3 SV Health)
        Mandatory Field */
    uint8_t svHealthMask;
    /** Issue of Data (ephemeris)
        Mandatory Field */
    uint8_t iode;
    /** Square root of the semi-major axis
        Unit: sqrt(meter)
        Mandatory Field */
    double aSqrt;
    /** Mean motion difference
        Unit: radian/second
        Mandatory Field */
    double deltaNRadPerSec;
    /** Mean anomaly at reference time
        Unit: radian
        Mandatory Field */
    double m0Rad;
    /** Eccentricity
        Mandatory Field */
    double eccentricity;
    /** Longitude of ascending node of orbit plane at weekly epoch
        Unit: radian
        Mandatory Field */
    double omegaORad;
    /** Inclination angle at reference time
        Unit: radian
        Mandatory Field */
    double inclinationAngleRad;
    /** Argument of perigee
        Unit: radian
        Mandatory Field */
    double omegaRad;
    /** Rate of right ascension
        Unit: radian/second
        Mandatory Field */
    double omegaDotRadPerSec;
    /** Rate of inclination angle
        Unit: radian/second
        Mandatory Field */
    double iDotRadPerSec;
    /** Amplitude of the cosine harmonic correction term
        to the argument of latitude
        Unit: radian
        Mandatory Field */
    double cUcRad;
    /** Amplitude of the sine harmonic correction term
        to the argument of latitude
        Unit: radian
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
        Unit: radian
        Mandatory Field */
    double cIcRad;
    /** Amplitude of the sine harmonic correction term
        to the angle of inclination (radian)
        Unit: radian
        Mandatory Field */
    double cIsRad;
    /** Reference time of ephemeris.
        Full seconds of TOE which is calculated from:
        (extendedGPSWeekNumber*604800+TOE).Extended GPS week
        number is calculated from 00:00:00 UTC on Sunday January 6, 1980
        (midnight between Jan 5 and Jan 6)
        Unit: second
        Mandatory Field */
    uint32_t fullToeSec;
    /** Clock data reference time.
        Full seconds of TOC which is calculated from:
        (extendedGPSWeekNumber*604800+TOC).Extended GPS week
        number is calculated from 00:00:00 UTC on Sunday January 6, 1980
        (midnight between Jan 5 and Jan 6)
        Unit: second
        Mandatory Field */
    uint32_t fullTocSec;
    /** Clock bias correction coefficient
        Unit: second
        Mandatory Field */
    double af0Sec;
    /** Clock drift correction coefficient
        Unit: second/second
        Mandatory Field */
    double af1;
    /** Clock drift rate correction coefficient
        Unit: second/second^2
        Mandatory Field */
    double af2;
    /** User Range Accuracy Index(4bit)
        as defined in GPS ICD
        (https://www.gps.gov/technical/icwg/IS-GPS-200H.pdf
         Section 20.3.3.3.1.3 SV Accuracy)
        Mandatory Field */
    uint8_t urai;
    /** codes on L2 channel
        as defined in GPS ICD
        (https://www.gps.gov/technical/icwg/IS-GPS-200H.pdf
        Section 20.3.3.3.1.2 Code(s) on L2 Channel)
        Mandatory Field */
    uint8_t codeL2;
    /** L2 P data flag
        as defined in GPS ICD
        (https://www.gps.gov/technical/icwg/IS-GPS-200H.pdf
        Section 20.3.3.3.1.6 Data Flag for L2 P-Code)
        Mandatory Field */
    uint8_t L2PData;
    /** Indicates the curve-fit interval used by the CS
        Unit: hour
        Range: 0 = 0-4 hours
               1 = greater than 4 hours
        Mandatory Field */
    uint8_t fitInterval;
    /** Time of Group delay
        Unit: second
        Mandatory Field */
    double  tgd;
    /** Issue of Data, Clock
        Mandatory Field */
    uint16_t iodc;
    uint8_t reserve[78];
} epGpsSatelliteEphemerisData;

/** Represents Glonass Navigation model info */
typedef struct {
   /**  GNSS SV ID.
         For GLONASS:  65 to 96. When slot-number to SV ID mapping is unknown,
         set as 255.
         Mandatory Field */
    uint16_t gnssSvId;
    /** Satellite health. 0 means broadcasting satellite is good and 1 means not
        Mandatory Field */
    uint8_t bnHealth;
    /** Ln SV health flags for GLONASS-M. 0 means broadcasting satellite is good
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
        Unit: day
        Mandatory Field */
    uint8_t En;
    /** Carrier frequency number + 8
        Range: 1-14
        Mandatory Field */
    uint8_t freqNo;
    /** Time interval between adjacent values of tb
        Unit: minute
        Mandatory Field */
    uint8_t P1;
    /** 1 if tb is odd and 0 if tb is even
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
        Unit: meter/second
        Mandatory Field */
    double vx;
    /** Y component of SV velocity
        Unit: meter/second
        Mandatory Field */
    double vy;
    /** Z component of SV velocity
        Unit: meter/second
        Mandatory Field */
    double vz;
    /** X component of SV sola-luni acceleration
        Unit: meter/second^2
        Mandatory Field */
    double lsx;
    /** Y component of SV sola-luni acceleration
        Unit: meter/second^2
        Mandatory Field */
    double lsy;
    /** Z component of SV sola-luni acceleration
        Unit: meter/second^2
        Mandatory Field */
    double lsz;
    /** SV clock bias, offset between GLONASS system time and the satellite clock
        Unit: second
        Mandatory Field */
    float tauN;
    /** SV clock frequency error ratio
        Unit: unitless
        Mandatory Field */
    float gamma;
    /** Reference time of ephemeris. Full seconds of TOE
        which is calculated from: [(N4-1)*1461 + (NT-1)]*86400 +tb*900
        Unit: second
        Mandatory Field */
    uint32_t fullToeSec;
    uint8_t reserve[70];
} epGlonassSatelliteEphemerisData;


/** BDS Navigation model Info */
typedef struct {
    /**  GNSS SV ID.
         For BDS:      201 to 237
         Mandatory Field */
    uint16_t gnssSvId;
    /** Autonomous satellite Health flag as defined in BDS ICD Version 2.1, Section 5.2.4.6
        (http://en.beidou.gov.cn/SYSTEMS/ICD/201806/P020180608523308843290.pdf),
        applied for both B1 and B2 (SatH1)
        Mandatory Field */
    uint8_t svHealthMask;
    /** Age of Data, Ephemeris
        Mandatory Field */
    uint8_t aode;
    /** Age of Data, Clock
        Mandatory Field */
    uint8_t aodc;
    /** Square root of the semi-major axis
        Unit: sqrt(meter)
        Mandatory Field */
    double aSqrt;
    /** Mean motion difference
        Unit: radian/second
        Mandatory Field */
    double deltaNRadPerSec;
   /** Mean anomaly at reference time
        Unit: radian
        Mandatory Field */
    double m0Rad;
    /** Eccentricity
        Mandatory Field */
    double eccentricity;
    /** Longitude of ascending node of orbit plane at weekly epoch
        Unit: radian
        Mandatory Field */
    double omegaORad;
   /** Inclination angle at reference time
        Unit: radian
        Mandatory Field */
    double inclinationAngleRad;
    /** Argument of perigee
        Unit: radian
        Mandatory Field */
    double omegaRad;
    /** Rate of right ascension
        Unit: radian/second
        Mandatory Field */
    double omegaDotRadPerSec;
    /** Rate of inclination angle
        Unit: radian/second
        Mandatory Field */
    double iDotRadPerSec;
    /** Amplitude of the cosine harmonic correction term
        to the argument of latitude
        Unit: radian
        Mandatory Field */
    double cUcRad;
    /** Amplitude of the sine harmonic correction term
        to the argument of latitude
        Unit: radian
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
        Unit: radian
        Mandatory Field */
    double cIcRad;
    /** Amplitude of the sine harmonic correction term
        to the angle of inclination (radian)
        Unit: radian
        Mandatory Field */
    double cIsRad;
    /** Reference time ephemeris. Full seconds of TOE
        which is calculated from: (extendedBDSWeekNumber*604800+TOE)
        Extended BDS week number is calculated from 00:00:00 on January 2006
        (midnight between Dec 31 and Jan 1)
        Unit: second
        Mandatory Field */
    uint32_t fullToeSec;
    /** Clock data reference time. Full seconds of TOC.
        which is calculated from: (extendedBDSWeekNumber*604800+TOC)
        Extended BDS week number is calculated from 00:00:00 on January 2006
        (midnight between Dec 31 and Jan 1)
        Unit: second
        Mandatory Field */
    uint32_t fullTocSec;
    /** Clock bias correction coefficient
        Unit: second
        Mandatory Field */
    double af0Sec;
    /** Clock drift correction coefficient
        Unit: second/second
        Mandatory Field */
    double af1;
    /** Clock drift rate correction coefficient
        Unit: second/second^2
        Mandatory Field */
    double af2;
    /** Equipment group delay differential for the B1 signal
        Unit: nanosecond
        Mandatory Field */
    double tgd1Ns;
    /** User Range Accuracy Index(4bit)
        Mandatory Field */
    uint8_t urai;
    uint8_t reserve[88];
} epBDSSatelliteEphemerisData;

/** GALILEO Navigation model Info */
typedef struct {
    /**  GNSS SV ID.
         For GAL:      301 to 336
         Mandatory Field */
    uint16_t gnssSvId;
    /** SV Health Status defined in section 5.1.9.3 of European GNSS (Galileo)
        Open Service Signal-In-Space Interface Control Document, Issue 1.3.
        * Only 3 bits in 'svHealth' for E5a (F/NAV)
        * F/NAV Bit 0 is the DVS bit (Data Validity Status)
        * F/NAV Bit 1 and 2 are HS bits (Signal Health Status)
        * 6 bits in 'svHealth' for E1B, E5b (I/NAV). Bit 0, 2, 3 are for E1B,
          bit 1, 4, 5 are for E5B
        * I/NAV Bit 0 is the DVS bit for E1B
        * I/NAV Bit 2, and 3 are the HS bits for E1B
        * I/NAV Bit 1 is the DVS bit for E5B
        * I/NAV Bit 4 and 5 are the HS bits for E5B
        * A signal (E1B, E5a, or E5b) is only fully useful when all three bits
          for a component are 0
        Mandatory Field */
    uint8_t svHealth;
    /** GALILEO Signal Type
        Mandatory Field */
    epGalSVDataSource dataSource;
    /** Ephemeris and clock correction issue of data
        Mandatory Field */
    uint16_t iodNav;
    /** Mean motion difference
        Unit: radian/second
        Mandatory Field */
    double deltaNRadPerSec;
    /** Mean anomaly at reference time
        Unit: radian
        Mandatory Field */
    double m0Rad;
    /** Eccentricity
        Mandatory Field */
    double eccentricity;
    /** Square root of the semi-major axis
        Unit: sqrt(meter)
        Mandatory Field */
    double aSqrt;
    /** Longitude of ascending node of orbit plane at weekly epoch
        Unit: radian
        Mandatory Field */
    double omegaORad;
    /** Inclination angle at reference time
        Unit: radian
        Mandatory Field */
    double inclinationAngleRad;
    /** Argument of perigee
        Unit: radian
        Mandatory Field */
    double omegaRad;
    /** Rate of right ascension
        Unit: radian/second
        Mandatory Field */
    double omegaDotRadPerSec;
    /** Rate of inclination angle
        Unit: radian/second
        Mandatory Field */
    double iDotRadPerSec;
    /** Amplitude of the cosine harmonic correction term
        to the argument of latitude
        Unit: radian
        Mandatory Field */
    double cUcRad;
    /** Amplitude of the sine harmonic correction term
        to the argument of latitude
        Unit: radian
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
        Unit: radian
        Mandatory Field */
    double cIcRad;
    /** Amplitude of the sine harmonic correction term
        to the angle of inclination (radian)
        Unit: radian
        Mandatory Field */
    double cIsRad;
    /** Reference time of ephemeris. Full seconds of TOE
        which is calculated from: (extendedGALWeekNumber*604800+TOE).
        Extended GAL week number is calculated from 00:00:00 UTC
        on Sunday August 22, 1999 (midnight between August 21 and August 22).
        Unit: second
        Mandatory Field */
    uint32_t fullToeSec;
    /** Clock data reference time. Full seconds of TOC.
        which is calculated from: (extendedGALWeekNumber*604800+TOC).
        Extended GAL week number is calculated from 00:00:00 UTC
        on Sunday August 22, 1999 (midnight between August 21 and August 22).
        Unit: second
        Mandatory Field */
    uint32_t fullTocSec;
    /** SV Clock bias correction coefficient
        Unit: second
        Mandatory Field */
    double af0Sec;
    /** SV Clock drift correction coefficient
        Unit: second/second
        Mandatory Field */
    double af1;
    /** SV Clock drift rate correction coefficient
        Unit: second/second^2
        Mandatory Field */
    double af2;
    /** E1-E5ab group delay.
        Unit: second */
    double bgdE1E5a;
    /** E1-E5b group delay from I/Nav (E1B or E5b). For E1B or E5b signal,
        both bgdE1E5a and bgdE1E5b are valid. For E5a signal,
        only bgdE1E5a is valid. Signal source identified by dataSource.
        Unit: second */
    double bgdE1E5b;
    /** Signal-in-space accuracy index for dual frequency
        E1-E5b/E5a depending on dataSource
        Mandatory Field */
    uint8_t sisaIndex;
    /* Validity Mask for Optional Parameters */
    epGalileoEphemerisValidityMask validityMask;
    uint8_t reserve[78];
} epGalileoSatelliteEphemerisData;

/** Represents QZSS Navigation model info */
typedef struct {
    /**  QZSS SV ID.
         For QZSS:      193 to 202
         Mandatory Field */
    uint16_t gnssSvId;
    /** The six-bit health indication as defined in QZSS ICD
        (IS-QZSS-PNT-002, Table 4.1.2-5 and 4.1.2.-6 in section 4.1.2.3)
        * Bit 0: reserved Health (unused) from 5-bit health (LSB)
        * Bit 1: L1C Health from 5-bit health
        * Bit 2: L5 Health from 5-bit health
        * Bit 3: L2C Health from 5-bit health
        * Bit 4: L1 C/A Health from 5-bit health (MSB)
        * Bit 5: L1 C/A Health from 1-bit health
        Mandatory Field */
    uint8_t svHealthMask;
    /** Issue of Data (ephemeris)
        Mandatory Field */
    uint8_t iode;
    /** Square root of the semi-major axis
        Unit: sqrt(meter)
        Mandatory Field */
    double aSqrt;
    /** Mean motion difference
        Unit: radian/second
        Mandatory Field */
    double deltaNRadPerSec;
    /** Mean anomaly at reference time
        Unit: radian
        Mandatory Field */
    double m0Rad;
    /** Eccentricity
        Mandatory Field */
    double eccentricity;
    /** Longitude of ascending node of orbit plane at weekly epoch
        Unit: radian
        Mandatory Field */
    double omegaORad;
    /** Inclination angle at reference time
        Unit: radian
        Mandatory Field */
    double inclinationAngleRad;
    /** Argument of perigee
        Unit: radian
        Mandatory Field */
    double omegaRad;
    /** Rate of right ascension
        Unit: radian/second
        Mandatory Field */
    double omegaDotRadPerSec;
    /** Rate of inclination angle
        Unit: radian/second
        Mandatory Field */
    double iDotRadPerSec;
    /** Amplitude of the cosine harmonic correction term
        to the argument of latitude
        Unit: radian
        Mandatory Field */
    double cUcRad;
    /** Amplitude of the sine harmonic correction term
        to the argument of latitude
        Unit: radian
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
        Unit: radian
        Mandatory Field */
    double cIcRad;
    /** Amplitude of the sine harmonic correction term
        to the angle of inclination (radian)
        Unit: radian
        Mandatory Field */
    double cIsRad;
    /** Reference time of ephemeris. Full seconds of TOE
        which is calculated from: (extendedQZSSWeekNumber*604800+TOE).
        Extended QZSS week number is calculated from 00:00:00 UTC on
        Sunday January 6, 1980 (midnight between Jan 5 and Jan 6).
        Unit: second
        Mandatory Field */
    uint32_t fullToeSec;
    /** Clock data reference time. Full seconds of TOC.
        which is calculated from: (extendedQZSSWeekNumber*604800+TOC).
        Extended QZSS week number is calculated from 00:00:00 UTC on
        Sunday January 6, 1980 (midnight between Jan 5 and Jan 6).
        Unit: second
        Mandatory Field */
    uint32_t fullTocSec;
    /** Clock bias correction coefficient
        Unit: second
        Mandatory Field */
    double af0Sec;
    /** Clock drift correction coefficient
        Unit: second/second
        Mandatory Field */
    double af1;
    /** Clock drift rate correction coefficient
        Unit: second/second^2
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
        Unit: hour
        Range: 0 = 0-4 hours
               1 = greater than 4 hours
        Mandatory Field */
    uint8_t fitInterval;
    /** Time of Group delay
        Unit: second
        Mandatory Field */
    double  tgd;
    /** Issue of Data, Clock
        Mandatory Field */
    uint16_t iodc;
    uint8_t reserve[78];
} epQzssSatelliteEphemerisData;


/** GPS Navigation model for each SV */
typedef struct {
    /** Number of ephemeris contained in this response */
    uint16_t numOfEphemeris;
    /** GPS Ephemeris Data array */
    epGpsSatelliteEphemerisData gpsEphemerisData[32];
    uint8_t reserve[64];
} epGpsEphemerisResponse;

/** GLONASS Navigation model for each SV */
typedef struct {
    /** Number of ephemeris contained in this response */
    uint16_t numOfEphemeris;
    /** GLONASS Ephemeris Data array */
    epGlonassSatelliteEphemerisData gloEphemerisData[24];
    uint8_t reserve[64];
} epGlonassEphemerisResponse;

/** BDS Navigation model for each SV */
typedef struct {
    /** Number of ephemeris contained in this response */
    uint16_t numOfEphemeris;
    /** BDS Ephemeris Data array */
    epBDSSatelliteEphemerisData bdsEphemerisData[37];
    uint8_t reserve[64];
} epBdsEphemerisResponse;

/** GALILEO Navigation model for each SV */
typedef struct {
    /** Number of ephemeris contained in this response */
    uint16_t numOfEphemeris;
    /** GAL Ephemeris Data array */
    epGalileoSatelliteEphemerisData galEphemerisData[36];
    uint8_t reserve[64];
} epGalileoEphemerisResponse;

/** QZSS Navigation model for each SV */
typedef struct {
    /** Number of ephemeris contained in this response */
    uint16_t numOfEphemeris;
    /** QZSS Ephemeris Data array */
    epQzssSatelliteEphemerisData qzssEphemerisData[5];
    uint8_t reserve[64];
} epQzssEphemerisResponse;


/** Navigation model */
typedef struct {
    /** Indicates GNSS Constellation Type
        Mandatory field */
    epGnssConstellationTypeMask gnssConstellation;
    union {
       /** GPS Ephemeris Response */
       epGpsEphemerisResponse gpsEphemeris;
       /** GLONASS Ephemeris Response */
       epGlonassEphemerisResponse glonassEphemeris;
       /** BDS Ephemeris Response */
       epBdsEphemerisResponse bdsEphemeris;
       /** GALILEO Ephemeris Response */
       epGalileoEphemerisResponse galileoEphemeris;
       /** QZSS Ephemeris Response */
       epQzssEphemerisResponse qzssEphemeris;
    } ephInfo;
    /** GPS time when this ephemeris report is sent */
    epGnssSystemTimeStructType gpsSystemTime;
    uint8_t reserve[24];
} epGnssEphemerisReport;

/** GPS - Time Group Delay and Inter Signal Correction Info */
typedef struct {
   /** Validity bits for following fields */
   epGpsTgdValidity valid;
   /** Inter Signal Correction (ISC) for GPS L1C/A band.
       Unit: milliseconds */
   float iscL1ca;
   /** Inter Signal Correction (ISC) for GPS L2C band.
       Unit: milliseconds */
   float iscL2;
   /** Inter Signal Correction (ISC) for GPS L5I5 band.
       Unit: milliseconds */
   float iscL5I5;
   /** Inter Signal Correction (ISC) for GPS L5Q5 band.
       Unit: milliseconds */
   float iscL5Q5;
   /** Time of Group Delay(TGD) for GPS.
       Unit: milliseconds */
   float tgdGps;
} epPolyGpsTgd;

/** GLO - Time Group Delay Info */
typedef struct {
   /** Validity bits for following fields */
   epGloTgdValidity valid;
   /** Time of Group Delay - GLONASS G1-G2
       Unit: milliseconds */
   float tgdG1G2;
} epPolyGloTgd;

/** BDS - Time Group Delay and Inter Signal Correction Info */
typedef struct {
   /** Validity bits for following fields */
   epBdsTgdValidity valid;
   /** Time of Group Delay - BDS B1
       Unit: milliseconds */
   float tgdB1;
   /** Time of Group Delay - BDS B2
       Unit: milliseconds */
   float tgdB2;
   /** Time of Group Delay - BDS B2A
       Unit: milliseconds */
   float tgdB2a;
   /** Inter Signal Correction - BDS B2A
       Unit: milliseconds */
   float iscB2a;
} epPolyBdsTgd;

/** GAL - Broadcast Group Delay Info */
typedef struct {
   /** Validity bits for following fields */
   epGalTgdValidity valid;
   /** Broadcast Group Delay - E1-E5a
       Unit: milliseconds */
   float bgdE1E5a;
   /** Broadcast Group Delay - E1-E5b
       Unit: milliseconds */
   float bgdE1E5b;
} epPolyGalTgd;

/** Navic - Time Group Delay Info */
typedef struct {
   /** Validity bits for following fields */
   epNavicTgdValidity valid;
   /** Time of Group Delay - NavIC L5
       Unit: milliseconds */
   float tgdL5;
} epPolyNavicTgd;

/** Polynomial Time Group Delay and Inter Signal Correction Info */
typedef union {
   /** GPS - Time Group Delay and Inter Signal Correction Info */
   epPolyGpsTgd   gps;
   /** GLO - Time Group Delay Info */
   epPolyGloTgd   glo;
   /** BDS - Time Group Delay and Inter Signal Correction Info */
   epPolyBdsTgd   bds;
   /** GAL - Broadcast Group Delay Info */
   epPolyGalTgd   gal;
   /** Navic - Time Group Delay Info */
   epPolyNavicTgd navic;
} epPolyTgd;

/** GNSS SV Polynomial Info */
typedef struct {
    /** GNSS SV ID.
        For GPS:      1 to 32
        For GLONASS:  65 to 96. When slot-number to SV ID mapping is unknown, set as 255.
        For SBAS:     120 to 151
        For QZSS-L1CA:193 to 197
        For BDS:      201 to 237
        For GAL:      301 to 336
        0: Invalid,
        All others are reserved
        Mandatory field */
    uint16_t gnssSvId;
    /** Frequency index, only valid if u_SysInd is GLO */
    int8_t freqNum;
    /** Polynomial status mask
        Mandatory field */
    epPolyStatusMask polyStatus;
    /** Validity bits for following fields */
    epPolyReportValidity validityMask;
    /** Ephemeris reference time
        GPS:Issue of Data Ephemeris used [unit less].
        GLO: Tb 7-bit, refer to ICD02 */
    uint16_t iode;
    /** Reference time for polynomial calculations
        GPS: Secs in week.
        GLO: Full secs since Jan/01/96
    */
    double T0;
    /** C0X, C0Y, C0Z */
    double polyCoeffXYZ0[EP_SV_POLY_XYZ_0_TH_ORDER_COEFF_MAX_SIZE];
    /** C1X, C2X ... C2Z, C3Z */
    double polyCoefXYZN[EP_SV_POLY_XYZ_N_TH_ORDER_COEFF_MAX_SIZE];
    /** C0T, C1T, C2T, C3T */
    float polyCoefOther[EP_SV_POLY_SV_CLKBIAS_COEFF_MAX_SIZE];
    /** SV position uncertainty [m].
        Uncertainty is defined with 68% confidence level.
    */
    float svPosUnc;
    /** Ionospheric delay at d_T0 [m]. */
    float ionoDelay;
    /** Ionospheric delay rate [m/s].  */
    float ionoDot;
    /** SBAS Ionospheric delay at d_T0 [m]. */
    float sbasIonoDelay;
    /** SBAS Ionospheric delay rate [m/s].  */
    float sbasIonoDot;
    /** Tropospheric delay [m]. */
    float tropoDelay;
    /** Elevation [rad] at d_T0 */
    float elevation;
    /** Elevation rate [rad/s] */
    float elevationDot;
    /** SV elevation [rad] uncertainty
        Uncertainty is defined with 99% confidence level.
    */
    float elevationUnc;
    /** Coefficients of velocity polynomial */
    double velCoef[EP_SV_POLY_VELOCITY_COEF_MAX_SIZE];
    /** Enhanced Reference Time */
    uint32_t enhancedIOD;
    /** The polyAction indicates if polynomial for the SV is to be updated or to be invalidated.
        Mandatory Field */
    epPolyUpdateAction polyAction;
    /** Polynomial Time Group Delay and Inter Signal Correction Info
        Mandatory Field */
    epPolyTgd tgd;
    uint8_t reserve[4];
} epGnssSvPolynomial;


/** GNSS SV Info */
typedef struct {
    /** Specifies GNSS signal type
        Mandatory Field */
    epGnssSignalTypeMask gnssSignalType;
    /** Specifies GNSS Constellation Type
        Mandatory Field */
    epGnssConstellationTypeMask gnssConstellation;
    /**  GNSS SV ID.
         For GPS:      1 to 32
         For GLONASS:  65 to 96. When slot-number to SV ID mapping is unknown, set as 255.
         For SBAS:     120 to 151
         For QZSS-L1CA:193 to 197
         For BDS:      201 to 237
         For GAL:      301 to 336
         Mandatory Field */
    uint16_t gnssSvId;
    /** GLONASS frequency number + 8.
        Valid only for a GLONASS system and
        is to be ignored for all other systems.
        Range: 1 to 14 */
    uint8_t gloFrequency;
    /** Validity bits for following fields */
    epGnssSvValidity validityMask;
    /** signal strength
         - Unit: 1 dBHz
        Mandatory field */
    float cN0dbHz;
    /** SV elevation Unit: degree */
    float elevation;
    /** SV azimuth Unit: degree */
    float azimuth;
    /** SV status. Mandatory Field */
    epGnssSvStatusMask gnssSvStatusMask;
    uint8_t reserve[64];
} epGnssSv;

/** GNSS SV Notification */
typedef struct {
    /** Number of SVs in the gnssSvs array
        Mandatory field */
    uint8_t svCount;
    /** SV Information, Mandatory field */
    epGnssSv gnssSvs[EP_GNSS_MAX_MEAS];
    uint8_t reserve[64];
} epGnssSvNotification;


/** GNSS SV Measurement Info */
typedef struct {
    /** Specifies GNSS signal type. Mandatory field */
    epGnssSignalTypeMask gnssSignal;
    /** Specifies GNSS system. Mandatory field */
    epGnssConstellationTypeMask gnssSystem;
    /** Specifies GNSS SV ID.
             For GPS:             1 to 32
             For GLONASS:     65 to 96. When slot-number to SV ID mapping is unknown, set as 255.
             For SBAS:           120 to 151 and 183 to 192
             For QZSS-L1CA:   193 to 197
             For BDS:             201 to 237
             For GAL:              301 to 336
             Mandatory Field */
    uint16_t gnssSvId;
    /** GLONASS frequency number + 8.
         Valid only for a GLONASS system and
         is to be ignored for all other systems.
             - Range: 1 to 14 */
    uint8_t gloFrequency;
    /** Please ignore this field; included for information purposes only
        Specifies satellite search state. */
    epGnssMeasSvSearchState svSearchState;
    /**  Carrier to Noise ratio
         - Unit: 1 dBHz */
    float cN0dbHz;
    /**  Please ignore this field; included for information purposes only
         Bit-mask indicating SV measurement status.
         Valid bit-masks:
         If any of the upper 10 bits are set (i.e 0xFFC0000000000000), the
         measurement should not be used by the client. */
    epGNSSMeasurementStatusMask  measurementStatus;
    /**  Satellite time Millisecond.
         When measurementStatus bit-0 (sub-ms valid),
         1 (sub-bit known) & 2 (SV time known) are set, for:
         GPS, BDS, GAL, QZSS: Range of 0 through (604800000-1)
         GLO: Range of 0 through (86400000-1).
         Unit: millisecond
         Note: All SV times in the current measurement block are already
               propagated to common reference time epoch.
               The common reference time means that the measurements
               are propagated from each SV's individual integrate and
               dump period to a common time for the measurements.
         */
    uint32_t svMs;
    /**  Satellite time sub-millisecond.
         When measurementStatus bit-0 "Sub-millisecond is valid" is set,
         Range of 0 through 0.99999 [Millisecond]
         Total SV Time = svMs + svSubMs
         - Unit: millisecond */
    float svSubMs;
    /**  Satellite Time uncertainty
         Uncertainty is defined with 68% confidence level.
         - Unit: millisecond */
    float svTimeUncMs;
    /** dopplerShift is the observed Doppler.
           - Unit: meter/second
        Satellite Doppler (Hz)
           = dopplerShift (m/s) * -CarrierFreqInHz/(Speed of Light meter/sec). */
    float dopplerShift;
    /** Observed Doppler uncertainty
        Uncertainty is defined with 68% confidence level.
        - Unit: meter/second */
    float dopplerShiftUnc;
    /** Validity mask for below fields */
    epGnssSVMeasurementValidity validityMask;
    /** Health status.
        Range: 0 to 2;
        0 = unhealthy, 1 = healthy, 2 = unknown */
    /** Please ignore this field; included for information purposes only
        EnginePlugin will send measurement only when SV is healthy */
    uint8_t healthStatus;
    /** Please ignore this field; included for information purposes only
        Indicates whether almanac and ephemeris information is available. */
    epGnssMeasSvInfoMask svInfoMask;
    /** Loss of signal lock indicator */
    epGnssMeasSvLLI lossOfLock;
    /** Increments when a cycle slip is detected */
    uint8_t cycleSlipCount;
    /** Estimated pseudorange uncertainty
        Uncertainty is defined with 68% confidence level.
        Unit: meter */
    float pseudorange_uncertainty;
    /** Carrier phase measurement [Carrier cycles]
        Examples:
        Following condition indicates Carrier Phase is valid but Lock Point Unknown:
            validityMask.isCarrierPhaseValid == 1 &&
            validityMask.isLossOfLockValid == 1 &&
            lossOfLock.halfCycleAmbiguity == 1

        Following condition indicates Carrier Phase is valid and Lock Point Known:
            validityMask.isCarrierPhaseValid == 1 &&
            validityMask.isLossOfLockValid == 1 &&
            lossOfLock.halfCycleAmbiguity == 0

       Note: lossOfLock.lossOfContinousLock == 1 indicates cycle slips occurred since the prior epoch.
    */
    double carrierPhase;
    /** Carrier phase measurement Unc [Carrier cycles]  */
    float carrierPhaseUnc;
    /** Estimated pseudorange
        pseudorange is valid when isPseudorangeValid is set
        Unit: meter */
    double pseudorange;
    uint8_t reserve[36];
} epGnssSVMeasurementStructType;

/** GNSS Measurement Report */
typedef struct {
    /** Receiver clock Drift
        Unit: meter/second */
    float clockDrift;
    /** Receiver clock drift std deviation
        Unit: meter/second */
    float clockDriftStdDeviation;
    /** GPS system time information. Check validity before using */
    epGnssSystemTimeStructType gpsSystemTime;
    /** GAL system time information. Check validity before using */
    epGnssSystemTimeStructType galSystemTime;
    /** BDS system time information. Check validity before using */
    epGnssSystemTimeStructType bdsSystemTime;
    /** QZSS system time information. Check validity before using */
    epGnssSystemTimeStructType qzssSystemTime;
    /** GLO system time information. Check validity before using */
    epGnssGloTimeStructType gloSystemTime;
    /** Validity mask */
    epGnssMeasurementReportValidity validityMask;
    /** GPS time leap second delta to UTC time. Check validity before using
        Unit: second */
    uint8_t leapSec;
    /** Number of valid Measurements in this report block.
        Mandatory field */
    uint8_t numMeas;
    /** Satellite measurement Information
        Mandatory field */
    epGnssSVMeasurementStructType svMeasurement[EP_GNSS_MAX_MEAS];
    /** Please ignore this field; included for information purposes only.
        Local time of last PPS pulse, associated with GNSS Measurement report */
    epGnssApTimeStampStructType lastPpsLocaltimeStamp;
    /** GNSS Measurement Report Type */
    epGnssMeasurementReportType gnssMeasReportType;
    uint8_t reserve[32];
} epGnssMeasurementReport;


/** Klobuchar Ionospheric Model
    For more information on these parameters, see
    GPS ICD:
       (https://www.gps.gov/technical/icwg/IS-GPS-200H.pdf);
    BDS ICD Version 2.1:
       (http://en.beidou.gov.cn/SYSTEMS/ICD/201806/P020180608523308843290.pdf);
    QZSS ICD:
       (http://qzss.go.jp/en/technical/download/pdf/ps-is-qzss/is-qzss-pnt-003.pdf?t=1546562302356)
*/

typedef struct {
    /** GPS time when these ionospheric model parameters are sent. */
    epGnssSystemTimeStructType gpsSystemTime;
    /** Iono Data Source */
    epGnssKlobucharIonoDataSource ionoDataSource;
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
    uint8_t reserve[64];
} epKlobucharIonoModel;


/** Glonass additional parameters */
typedef struct {
    /** GPS time when these Glonass additional parameters are sent.
        Mandatory Field */
    epGnssSystemTimeStructType gpsSystemTime;
    /** Validity bits for following fields */
    epGloAdditionalParameterType validityMask;
    /** Glonass time scale correction to UTC(SU) time as in Glonass ICD Edition 5.1 page 36
        Unit: second */
    double tauCSec;
    /** Leap seconds (GPS-UTC) from GPS
        Unit: second */
    int8_t leapSec;
    uint8_t reserve[64];
} epGloAdditionalParameter;


typedef struct {
    /** Source of correction data */
    uint8_t correctionSource;
    /** Length of correction data in correctionData */
    uint32_t correctionLength;
    /** Correction Data */
    uint8_t *correctionData;
    uint8_t reserve[64];
} epCorrectionStreamData;



/** Below APIs shall be called by the plugged-in engine clients
    (Engine plug-in <-- Client Engine)
    APIs will run in the context of Engine Plug-in

    Note: Engine plug-in shall handle the incoming data in its own context/thread and
    not block the call from Client Engine.
*/

struct EPCallbacks {
    /**
      * @brief Report the computed Position
      *
      * @param[in]  epPVTReport: Position Report.
      *
      * @return none
      */
    void (* epReportPositionCb)(const epPVTReport *pvtReport);
    /**
      * @brief  Report engine status response.
      *
      * @param[in] status: Engine Status.
      *
      * @return none
      *
      */
    void (* epReportStatusRspCb)(uint32_t status);

    /**
      * @brief Log a message
      *
      * @param[in]  moduleID: Valid Module ID
      * @param[in]  logLevel: Log level
      * @param[in]  logType:  Log Type
      * @param[in]  logLength: Log length
      * @param[in]  logMessage: Log message pointer
      *
      * @return none
      *
      */
    void (* epReportLogMessageCb)(uint32_t moduleID, epLogLevelType logLevel,
                     epLogType logType, uint32_t logLength, const uint8_t* logMessage);

    /**
      * @brief Report SV Status
      *
      * @param[in] epGnssSvNotification: SV Status
      *
      * @return none.
      *
      */
    void (* epReportSvStatusCb)(const epGnssSvNotification *svNotify);

    /**
      * @brief Subscribe to reports of interest.
      *        Engine plug-in shall send error, if the requested subscription is not supportable
      * @param[in] epSubscriptionInfo:  Subscription Mask Info
      *
      * @return none.
      *
      */
    void (* epRequestReportSubscription)(const epSubscriptionInfo *requestedSubscription);

    /**
      * @brief  API of query feature Status by Engine Client.
      *
      * @return none.
      *
      */
    void  (* epRequestFeatureStatusCb)(void);

    /**
      * @brief Request to create attestation statement with user data and nonce.
      *        Put user data and the nonce into the attestation statement and
      *        return to the client in epProvideAttestationStatement().
      *
      * @param[in] nonce: Transaction Info provided by the server.
      * @param[in] nonceLen: Length of Transaction Info provided by the server.
      * @param[in] data:  Application-defined data.
      * @param[in] lengthOfData: Length of the data buffer.
      *
      * @return none.
      *
      */
    void (* epRequestAttestationStatementCb) (uint8_t *nonce, size_t lengthOfNonce,
                   uint8_t *data, uint32_t lengthOfData);
    /**
      * @brief Request to Install License.
      *
      * @param[in] licenseBuffer: License Buffer.
      * @param[in] lengthOfLicenseBuffer: Length of the license buffer.
      *
      * @return none.
      *
      */
    void (* epRequestInstallLicenseCb) (uint8_t *licenseBuffer,
                   uint32_t lengthOfLicenseBuffer);

    uint8_t reserve[52];
};


/** Below APIs shall be called by Engine plug-in
    (Engine plug-in --> Client Engine)
    APIs will run in the context of called engine.

    Note: Client Engine shall handle the incoming data in its own context/thread
    and not block the calls from Engine plug-in.
*/
struct EPInterface {
    /** Provide Engine Plug-in interface header file
        Major version (EP_INTERFACE_FILE_MAJOR_VERSION) to engine Plug-in.
        Note: Using this information, Engine Plug-in detects of any major version
        mismatch. If mismatch is found, Engine Plug-in responds with an error as
        EP_ENGINE_PLUGIN_INTERFACE_FILE_VERSION_MISMATCH  */
    uint16_t  epInterfaceFileMajorVersion;

    /** Provide Engine Plug-in Interface header file minor version
        (EP_INTERFACE_FILE_MINOR_VERSION) to engine Plug-in. */
    uint16_t  epInterfaceFileMinorVersion;

    /**
      * @brief Send Engine Plug-in Error.
      *
      * @param[in] epError: Data connection status
      *
      * @return none.
      */
    void  (* epSendEnginePluginError) (epError error);

    /**
     * @brief Request Version information of Engine.
     *        This is a blocking Call.
     * @param[out] engineVersion:  Version info Buffer
     * @param[in]  maxBuffSizeVer: Version info Max Size (128 bytes)
     * @param[out] engineId: Identification info Buffer
     * @param[in]  maxBuffSizeId: Identification info Max Size (128 bytes)
     *
     * @return bool: True: In case of success, False: Otherwise.
     *
     */
    bool  (* epRequestVersionInfo)(uint8_t *engineVersion, size_t maxBuffSizeVer,
                           uint8_t *engineId, size_t maxBuffSizeId);

    /**
     * @brief Session Control Command.
     *
     * @param[in] epCommandRequest: Session Control Command request
     *
     * @return bool: True: In case of success, False: Otherwise.
     */
    bool  (* epSessionContrlCommand)(const epCommandRequest *cmdParameters);

    /**
      * @brief Provide GNSS Ephemeris.
      *        When ephemeris reports are subscribed, EnginePlugin shall send the ephemeris
      *        available at start, and then continues to output when the ephemeris is changed.
      *
      * @param[in] epGnssEphemerisReport: GNSS Ephemeris Info.
      *
      * @return none.
      */
    void  (* epProvideGnssEphemeris)(const epGnssEphemerisReport *ephemerisResport);

    /**
      * @brief Provide GNSS Measurement
      *
      * @param[in] epGnssMeasurementReport: Provide GNSS Measurement
      *
      * @return none.
      */
    void  (* epProvideGnssSvMeasurement)(const epGnssMeasurementReport *msrReport);

    /**
      * @brief Provide Position, Velocity, Time (PVT)
      *
      * @param[in] epPVTReport: Provide Position Velocity Time.
      *
      * @return none.
      *
      */
    void  (* epProvidePosition)(const epPVTReport *positionReport);

    /**
      * @brief Provide SV Polynomial.
      *
      * @param[in] epGnssSvPolynomial: Provide GNSS SV Polynomial Data.
      *
      * @return void.
      *
      */
    void (* epProvideSvPolynomial)(const epGnssSvPolynomial *svPolynomial);

    /**
      * @brief Provide Device Data Connection Status.
      *
      * @param[in] epConnectionStatus: Data connection status
      *
      * @return none.
      */
    void  (* epProvideConnectionStatus) (epConnectionStatus connStatus);

    /**
      * @brief Provide SV Info
      *
      * @param[in] epGnssSvNotification: SV Status Info
      *
      * @return none.
      *
      */
    void (* epProvideSvInfo)(const epGnssSvNotification *svNotify);
    /**
      * @brief Clean-up API
      *
      * @param[in] none
      *
      * @return none.
      *
      */
    void  (* cleanup)(void);

    /**
      * @brief API for Provide attestation statement
      *
      * @param[in] attestationStatement: Buffer of attestation statement
      * @param[in] sizeOfAttestationStatement: size of attestation statement
      * @param[in] status: Provide Status of Attestation Statement
      *
      * @return none.
      *
      */
    void  (* epProvideAttestationStatement)(uint8_t *attestationStatement,
               uint32_t sizeOfAttestationStatement, epAttestationStatus status);
    /**
      * @brief API for Provide status of Install License
      *
      * @param[in] status: License installation status.
      *
      * @return none.
      *
      */
     void (* epProvideInstallLicenseStatus)(epInstallLicenseStatus status);
    /**
      * @brief  API provides feature Status from Engine Client.
      *
      * @param[In] status: License status
      * @param[In] licenseType: License Type
      *
      * @return none.
      *
      */
      void  (* epProvideFeatureStatus)(epFeatureStatus status,
                                       epLicenseType licenseType);

    /**
      * @brief  API provides Correction Data from Engine Client.
      *
      * @param[In] streamData: Info related to correction data
      *
      * @return none.
      *
      */
      void  (* epProvideCorrectionData)(const epCorrectionStreamData *streamData);

    /**
      * @brief  API provides the parameters of the Klobuchar ionospheric model.
      *         For more information on these parameters, see
      *            GPS ICD (https://www.gps.gov/technical/icwg/IS-GPS-200H.pdf);
      *            BDS ICD Version 2.1
      *              (http://en.beidou.gov.cn/SYSTEMS/ICD/201806/P020180608523308843290.pdf);
      *            QZSS ICD
      *              (http://qzss.go.jp/en/technical/download/pdf/ps-is-qzss/
      *                 is-qzss-pnt-003.pdf?t=1546562302356).
      *
      *         EnginePlugin shall send valid Klobuchar ionospheric model when GPS, BDS,
      *         and/or QZSS ionospheric model reports are subscribed.
      *         EnginePlugin shall send the Klobuchar ionospheric model available at start, and
      *         then continues to output when the Klobuchar ionospheric model is changed.
      *         EnginePlugin shall send valid Klobuchar ionospheric model decoded from GPS only
      *         if GPS ionospheric model is subscribed.
      *         EnginePlugin shall send valid Klobuchar ionospheric model decoded from BDS only
      *         if BDS ionospheric model is subscribed.
      *         EnginePlugin shall send valid Klobuchar ionospheric model decoded from QZSS only
      *         if QZSSS ionospheric model is subscribed.
      *
      * @param[In] epKlobucharIonoModel: Info related to Iono correction Data
      *
      * @return none.
      *
      */
      void  (* epProvideKlobucharIonoModel) (const epKlobucharIonoModel *ionoData);

    /**
      * @brief  API provides the parameters of Glonass tauC and GPS leap second.
      *         EnginePlugin shall send these parameters only when Glonass ephemeris is subscribed.
      *         EnginePlugin shall send these additional parameters available at start, and
      *         subsequently when these parameters are changed.
      *
      * @param[In] epGloAdditionalParameter: Info related to correction Data
      *
      * @return none.
      *
      */
      void  (* epProvideGloAdditionalParam) (const epGloAdditionalParameter *additionalData);

      uint8_t reserve[44];
};


/**
 * @brief Get Engine API Interface.
 *        This is blocking Call.
 *
 * @param[in] callbacks: Engine Callbacks.
 *
 * @return EPInterface: API Interface of Engine.
*/
#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

const struct EPInterface* get_ep_interface(const struct EPCallbacks* callbacks);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /** ENGINE_PLUGIN_API_H */
