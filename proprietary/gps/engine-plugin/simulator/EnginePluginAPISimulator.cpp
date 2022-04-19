/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <sys/time.h>
#include "EnginePluginAPI.h"

#define EP_GNSS_ALL_CONSTELLATION (EP_GNSS_CONSTELLATION_GPS |\
                                   EP_GNSS_CONSTELLATION_GALILEO |\
                                   EP_GNSS_CONSTELLATION_SBAS |\
                                   EP_GNSS_CONSTELLATION_GLONASS |\
                                   EP_GNSS_CONSTELLATION_BEIDOU |\
                                   EP_GNSS_CONSTELLATION_QZSS)

#define EP_GNSS_ALL_SIGNAL_TYPE (EP_GNSS_SIGNAL_GPS_L1CA | EP_GNSS_SIGNAL_GPS_L1C |\
        EP_GNSS_SIGNAL_GPS_L2C_L | EP_GNSS_SIGNAL_GPS_L5_Q | EP_GNSS_SIGNAL_GLONASS_G1_CA |\
        EP_GNSS_SIGNAL_GLONASS_G2_CA | EP_GNSS_SIGNAL_GALILEO_E1_C |\
        EP_GNSS_SIGNAL_GALILEO_E5A_Q | EP_GNSS_SIGNAL_GALILIEO_E5B_Q |\
        EP_GNSS_SIGNAL_BEIDOU_B1_I | EP_GNSS_SIGNAL_BEIDOU_B1_C | EP_GNSS_SIGNAL_BEIDOU_B2_I |\
        EP_GNSS_SIGNAL_BEIDOU_B2A_I | EP_GNSS_SIGNAL_QZSS_L1CA | EP_GNSS_SIGNAL_QZSS_L1S |\
        EP_GNSS_SIGNAL_QZSS_L2C_L | EP_GNSS_SIGNAL_QZSS_L5_Q | EP_GNSS_SIGNAL_SBAS_L1_CA)

const struct EPCallbacks *pEpCbs = NULL;
static struct EPInterface epInterface = {};
static epSubscriptionInfo subscriptionMsg = {};

static void log_msg(const char * const format, ...)
{
    if (NULL != pEpCbs) {
        va_list args;
        va_start(args, format);
        int formatResult = -1;
        char buffer[512] = {0};
        formatResult = vsnprintf(buffer, sizeof(buffer), format, args);
        if ((0 <= formatResult) && (NULL != pEpCbs->epReportLogMessageCb)) {
            pEpCbs->epReportLogMessageCb(1,
                                            EP_LOG_LEVEL_ERROR,
                                            EP_LOG_TYPE_DEBUG,
                                            formatResult + 1,
                                            (const uint8_t*)buffer);
        }
        va_end(args);
    }
}

void ePSendEnginePluginError (epError error)
{
    log_msg("%s epError: %u\n", __func__, error);
    /* Copy input data to local structures and process it from separate thread context */
}

bool ePSessionContrlCommand(const epCommandRequest *cmdParameters)
{
    log_msg("%s \n", __func__);
    /* Copy input data to local structures and process it from separate thread context */
    return 1;
}

void ePProvideGnssEphemeris(const epGnssEphemerisReport *ephemerisResport)
{
    log_msg("%s \n", __func__);
    /* Copy input data to local structures and process it from separate thread context */
}

void ePProvideGnssSvMeasurement(const epGnssMeasurementReport *msrReport)
{
    log_msg("%s \n", __func__);
    /* Copy input data to local structures and process it from separate thread context */
}

void ePProvidePosition(const epPVTReport *positionReport)
{
    log_msg("%s \n", __func__);
    /* Copy input data to local structures and process it from separate thread context */
}

void  ePProvideCorrectionData(const epCorrectionStreamData *streamData)
{
    log_msg("%s \n", __func__);
    /* Copy input data to local structures and process it from separate thread context */
}

void  ePProvideKlobucharIonoModel (const epKlobucharIonoModel *ionoData)
{
    log_msg("%s \n", __func__);
    /* Copy input data to local structures and process it from separate thread context */
}

void  ePProvideGloAdditionalParam (const epGloAdditionalParameter *additionalData)
{
    log_msg("%s \n", __func__);
    /* Copy input data to local structures and process it from separate thread context */
}

void ePProvideSvPolynomial(const epGnssSvPolynomial *svPolynomial)
{
    log_msg("%s \n", __func__);
    /* Copy input data to local structures and process it from separate thread context */
}

bool ePRequestVersionInfo(uint8_t *engineVersion, size_t maxBuffSizeVer,
        uint8_t *engineId, size_t maxBuffSizeId)
{
    log_msg("%s maxBuffSizeVer: %d maxBuffSizeId: %d\n",
            __func__, maxBuffSizeVer, maxBuffSizeId);
    bool returnValue = true;
    if ((NULL != engineVersion) && (NULL != engineId)) {
        *engineVersion = 1;
        *engineId = 1;
    } else {
        returnValue = false;
    }
    return returnValue;
}

static void ep_init()
{
    subscriptionMsg.flags = (EP_SUBSCRIPTION_PVT_1HZ_REPORT | \
            EP_SUBSCRIPTION_UNPROPAGATED_PVT_1HZ_REPORT | \
            EP_SUBSCRIPTION_MEASUREMENT_1HZ_REPORT |\
            EP_SUBSCRIPTION_MEASUREMENT_NHZ_REPORT | \
            EP_SUBSCRIPTION_POLYNOMIAL_REPORT | \
            EP_SUBSCRIPTION_EPHEMERIS_REPORT | \
            EP_SUBSCRIPTION_CORRECTION_DATA | \
            EP_SUBSCRIPTION_IONO_MODEL_REPORT);

    subscriptionMsg.subscriptionModMeasurement.constellationType = EP_GNSS_ALL_CONSTELLATION;
    subscriptionMsg.subscriptionModMeasurement.gnssSignal = EP_GNSS_ALL_SIGNAL_TYPE;
    subscriptionMsg.subscriptionModMeasurementNHz.constellationType = EP_GNSS_ALL_CONSTELLATION;
    subscriptionMsg.subscriptionModMeasurementNHz.gnssSignal = EP_GNSS_ALL_SIGNAL_TYPE;
    subscriptionMsg.subscriptionModEphemeris.constellationType = EP_GNSS_ALL_CONSTELLATION;
    subscriptionMsg.subscriptionModEphemeris.gnssSignal = EP_GNSS_ALL_SIGNAL_TYPE;
    subscriptionMsg.subscriptionModPolynomial.constellationType = EP_GNSS_ALL_CONSTELLATION;
    subscriptionMsg.subscriptionModPolynomial.gnssSignal = EP_GNSS_ALL_SIGNAL_TYPE;
    subscriptionMsg.subscriptionModIONO.constellationType = EP_GNSS_ALL_CONSTELLATION;
    subscriptionMsg.subscriptionModIONO.gnssSignal = EP_GNSS_ALL_SIGNAL_TYPE;

    memset(&epInterface, 0, sizeof(epInterface));
    epInterface.epInterfaceFileMajorVersion = EP_INTERFACE_FILE_MAJOR_VERSION;
    epInterface.epInterfaceFileMinorVersion = EP_INTERFACE_FILE_MINOR_VERSION;
    epInterface.epRequestVersionInfo = ePRequestVersionInfo;
    epInterface.epSessionContrlCommand = ePSessionContrlCommand;
    epInterface.epProvideGnssEphemeris = ePProvideGnssEphemeris;
    epInterface.epProvideGnssSvMeasurement = ePProvideGnssSvMeasurement;
    epInterface.epProvidePosition = ePProvidePosition;
    epInterface.epProvideCorrectionData = ePProvideCorrectionData;
    epInterface.epProvideKlobucharIonoModel = ePProvideKlobucharIonoModel;
    epInterface.epProvideGloAdditionalParam = ePProvideGloAdditionalParam;
    epInterface.epSendEnginePluginError = ePSendEnginePluginError;
    epInterface.epProvideSvPolynomial = ePProvideSvPolynomial;

}

const struct EPInterface* get_ep_interface(const struct EPCallbacks* callbacks)
{
    ep_init();
    pEpCbs = callbacks;

    /* Call epRequestReportSubscription from different thread context once get_ep_interface
      gets returns */
    if ((NULL != callbacks) && (NULL != callbacks->epRequestReportSubscription)) {
        callbacks->epRequestReportSubscription(&subscriptionMsg);
    }
    return &epInterface;
}
