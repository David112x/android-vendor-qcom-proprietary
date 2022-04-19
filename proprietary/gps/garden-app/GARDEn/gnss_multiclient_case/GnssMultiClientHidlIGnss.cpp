/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#define LOG_NDEBUG 0
#define LOG_TAG "GARDEN_GMCC_HidlIGnss"

#include "GnssMultiClientHidlIGnss.h"
#include "GnssMultiClientCaseUtils.h"

#include <android/hardware/gnss/1.1/IGnssConfiguration.h>
#include <android/hardware/gnss/1.0/types.h>

using IGnss_V1_0 = android::hardware::gnss::V1_0::IGnss;
using IGnss_V1_1 = android::hardware::gnss::V1_1::IGnss;
using IGnss_V2_0 = android::hardware::gnss::V2_0::IGnss;
using android::hardware::gnss::V1_1::IGnssConfiguration;

namespace garden {

#define getUserInputEnterToContinue GmccUtils::get().getUserInputEnterToContinue
#define getUserInputInt GmccUtils::get().getUserInputInt
#define getUserInputDouble GmccUtils::get().getUserInputDouble
#define getUserInputMask64Bit GmccUtils::get().getUserInputMask64Bit
#define getUserInputString GmccUtils::get().getUserInputString
#define getUserInputYesNo GmccUtils::get().getUserInputYesNo
#define getUserInputSessionMode GmccUtils::get().getUserInputSessionMode
#define getUserInputTrackingOptions GmccUtils::get().getUserInputTrackingOptions
#define getUserInputLocClientIndex GmccUtils::get().getUserInputLocClientIndex
#define getUserInputGnssConfig GmccUtils::get().getUserInputGnssConfig
#define getUserInputGnssConfigFlagsMask GmccUtils::get().getUserInputGnssConfigFlagsMask
#define getUserInputGnssConfigBlacklistSvId GmccUtils::get().getUserInputGnssConfigBlacklistSvId
#define getUserInputGnssSvIdSource GmccUtils::get().getUserInputGnssSvIdSource
#define getUserInputGnssSvType GmccUtils::get().getUserInputGnssSvType
#define getUserInputGnssSvTypesMask GmccUtils::get().getUserInputGnssSvTypesMask
#define printGnssConfigBlacklistSvId GmccUtils::get().printGnssConfigBlacklistSvId
#define convertGnssSvIdMaskToList GmccUtils::get().convertGnssSvIdMaskToList
#define strUtilTokenize GmccUtils::get().strUtilTokenize


/* GnssMultiClientHidlIGnss static elements */
GnssMultiClientHidlIGnss* GnssMultiClientHidlIGnss::sInstance = nullptr;

/* GnssMultiClientHidlIGnss Public APIs */
GARDEN_RESULT GnssMultiClientHidlIGnss::menuTest()
{
    bool exit_loop = false;
    GARDEN_RESULT gardenResult = GARDEN_RESULT_INVALID;

    while(!exit_loop) {
        PRINTLN("\n\n"
                "1: Create HIDL client for IGnss 2.0 \n"
                "1001: IGnss 1.0 -> start \n"
                "1002: IGnss 1.0 -> stop \n"
                "1003: IGnssMeasurement 1.0 -> close \n"
                "1101: IGnss 1.1 -> setPositionMode_1_1 \n"
                "1102: IGnss 1.1 -> injectBestLocation \n"
                "1103: IGnssMeasurement 1.1 -> setCallback_1_1 \n"
                "1104: IGnssConfiguration 1.1 -> setBlacklist \n"
                "2001: IGnssConfiguration 2.0 -> setEsExtensionSec \n"
                "2002: IGnssVisibilityControl -> setCallback \n"
                "2003: IGnssVisibilityControl -> enable Nfw \n"
                "2004: IGnssVisibilityControl -> disable Nfw \n"
                "2005: IGnss 2.0 -> setCallback \n"
                "2006: IGnss 2.0 -> cleanup \n"
                "2007: IGnss 2.0 -> setCallback_2_0 \n"
                "2008: IGnss 2.0 -> injectBestLocation_2_0 \n"
                "2009: IGnssDebug 2.0 -> getDebugData_2_0 \n"
                "2010: IGnssBatching 2.0 -> init_2_0 \n"
                "2011: IGnssMeasurement 2.0 -> setCallback_2_0 \n"
                "2012: IAGnss2.0 -> setCallback_2_0 \n"
                "2013: IAGnss2.0 -> dataConnOpen_2_0 \n"
                "2014: IAGnss2.0 -> setServer_2_0 \n"
                "2015: IAGnss2.0 -> dataConnClosed_2_0 \n"
                "99: Display this menu again \n"
                "0: <-- back\n");
        int choice = getUserInputInt("Enter choice: ");

        switch (choice) {
        case 1:
            gardenResult = createHidlClient();
            break;
        case 1001:
            gardenResult = IGnss_1_0_start();
            break;
        case 1002:
            gardenResult = IGnss_1_0_stop();
            break;
        case 1003:
            gardenResult = IGnssMeasurement_1_0_close();
            break;
        case 1101:
            gardenResult = IGnss_1_1_setPositionMode_1_1();
            break;
        case 1102:
            gardenResult = IGnss_1_1_injectBestLocation();
            break;
        case 1103:
            gardenResult = IGnssMeasurement_1_1_setCallback_1_1();
            break;
        case 1104:
            gardenResult = IGnssConfiguration_1_1_setBlacklist();
            break;
        case 2001:
            gardenResult = IGnssConfiguration_2_0_setEsExtensionSec();
            break;
        case 2002:
            gardenResult = GnssVisibilityControl_setCallback();
            break;
        case 2003:
            gardenResult = GnssVisibilityControl_enableNfwLocationAccess(true);
            break;
        case 2004:
            gardenResult = GnssVisibilityControl_enableNfwLocationAccess(false);
            break;
        case 2005:
            gardenResult = IGnss_setCallback();
            break;
        case 2006:
            gardenResult = hidlClientCleanup();
            break;
        case 2007:
            gardenResult = IGnss_setCallback_2_0();
            break;
        case 2008:
            gardenResult = IGnss_injectBestLocation_2_0();
            break;
        case 2009:
            gardenResult = GnssDebug_getDebugData_2_0();
            break;
        case 2010:
            gardenResult = GnssBatching_init_2_0();
            break;
        case 2011:
            gardenResult = GnssMeasurement_2_0_setCallback_2_0();
            break;
        case 2012:
            gardenResult = AGnss_2_0_setCallback_2_0();
            break;
        case 2013:
            gardenResult = AGnss_2_0_dataConnOpen_2_0();
            break;
        case 2014:
            gardenResult = AGnss_2_0_setServer_2_0();
            break;
        case 2015:
            gardenResult = AGnss_2_0_dataConnClosed_2_0();
            break;
        case 99:
            continue;
        case 0:
            gardenResult = GARDEN_RESULT_ABORT;
            exit_loop = true;
            break;
        default:
            PRINTERROR("Invalid command");
        }

        if (0 != choice) {
            PRINTLN("\nExecution Result: %d", gardenResult);
            getUserInputEnterToContinue();
        }
    }

    return gardenResult;
}

/* Callbacks registered with HIDL framework */
void GnssDeathRecipient::serviceDied(uint64_t /*cookie*/, const wp<IBase>& /*who*/) {
    PRINTERROR("IGnss service died");
    GnssMultiClientHidlIGnss::get().mGnssIface = nullptr;
}

/* GnssMultiClientHidlIGnss TEST CASES */
GARDEN_RESULT GnssMultiClientHidlIGnss::createHidlClient()
{
    GARDEN_RESULT gardenResult = GARDEN_RESULT_FAILED;

    // Get IGNSS service
    mGnssIface = IGnss::getService();
    if (mGnssIface != nullptr) {
        PRINTLN("IGnss::getService() success.");

        // Link to IGNSS service Death
        mGnssDeathRecipient = new GnssDeathRecipient();
        android::hardware::Return<bool> linked = mGnssIface->linkToDeath(mGnssDeathRecipient, 0);
        if (linked.isOk() && linked) {
            PRINTLN("mGnssIface->linkToDeath() success.");

            // Set IGnssCallback
            auto result = mGnssIface->setCallback(mHidlIGnssCb);
            if (!result.isOk()) {
                PRINTERROR("mGnssIface->setCallback failed.");
            }

            // Get Extension : IGnssConfiguration 1.1
            auto gnssConfig = mGnssIface->getExtensionGnssConfiguration_1_1();
            if (gnssConfig.isOk()) {
                PRINTLN("mGnssIface->getExtensionGnssConfiguration_2_0() success.");
                mGnssConfigurationIface = gnssConfig;

                // Get Extension : IGnssMeasurement 1.1
                auto gnssMeasurement = mGnssIface->getExtensionGnssMeasurement_1_1();
                if (gnssMeasurement.isOk()) {
                    PRINTLN("mGnssIface->getExtensionGnssMeasurement_1_1() success.");
                    mGnssMeasurementIface_1_1 = gnssMeasurement;

                    // Get 2.0 Extensions
                    auto gnssConfig_2_0 = mGnssIface->getExtensionGnssConfiguration_2_0();
                    if (gnssConfig_2_0.isOk()) {
                        PRINTLN("mGnssIface->getExtensionGnssConfiguration_2_0() success.");
                        mGnssConfig_2_0 = gnssConfig_2_0;

                        auto visibilityControl = mGnssIface->getExtensionVisibilityControl();
                        if (visibilityControl.isOk()) {
                            PRINTLN("mGnssIface->getExtensionVisibilityControl() success.");
                            mGnssVisibilityControl = visibilityControl;

                            gardenResult = GARDEN_RESULT_PASSED;
                        } else {
                            PRINTERROR("mGnssIface->getExtensionVisibilityControl() failed.");
                        }
                    } else {
                        PRINTERROR("mGnssIface->getExtensionGnssConfiguration_2_0() failed.");
                    }
                } else {
                    PRINTERROR("mGnssIface->getExtensionGnssMeasurement_1_1() failed.");
                }
            } else {
                PRINTERROR("mGnssIface->getExtensionGnssConfiguration_1_1() failed.");
            }

            // Get 2.0 Extensions
            auto gnssMeasurement = mGnssIface->getExtensionGnssMeasurement_2_0();
            if (gnssMeasurement.isOk()) {
                PRINTLN("mGnssIface->getExtensionGnssMeasurement_2_0() success.");
                mGnssMeasurementIface_2_0 = gnssMeasurement;
            } else {
                PRINTERROR("mGnssIface->getExtensionGnssMeasurement_2_0() failed.");
            }
            auto gnssDebug_2_0 = mGnssIface->getExtensionGnssDebug_2_0();
            if (gnssDebug_2_0.isOk()) {
                PRINTLN("mGnssIface->getExtensionGnssDebug_2_0() success.");
                mGnssDebug_2_0 = gnssDebug_2_0;
            } else {
                PRINTERROR("mGnssIface->getExtensionGnssDebug_2_0() failed.");
            }

            auto gnssBatching_2_0 = mGnssIface->getExtensionGnssBatching_2_0();
            if (gnssBatching_2_0.isOk()) {
                PRINTLN("mGnssIface->getExtensionGnssBatching_2_0() success.");
                mGnssBatching_2_0 = gnssBatching_2_0;
            } else {
                PRINTERROR("mGnssIface->getExtensionGnssBatching_2_0() failed.");
            }
            auto agnssRil_2_0 = mGnssIface->getExtensionAGnssRil_2_0();
            if (agnssRil_2_0.isOk()) {
                PRINTLN("mGnssIface->getExtensionAGnssRil_2_0() success.");
                mAGnssRil_2_0 = agnssRil_2_0;
            } else {
                PRINTERROR("mGnssIface->getExtensionAGnssRil_2_0() failed.");
            }

            auto agnss_2_0 = mGnssIface->getExtensionAGnss_2_0();
            if (agnss_2_0.isOk()) {
                PRINTLN("mGnssIface->getExtensionAGnss_2_0() success.");
                mAGnss_2_0 = agnss_2_0;
            } else {
                PRINTERROR("mGnssIface->getExtensionAGnss_2_0() failed.");
            }

            // Setup measurement corrections callback.
            auto measurementCorrections = mGnssIface->getExtensionMeasurementCorrections();
            if (measurementCorrections.isOk()) {
                PRINTLN("mGnssIface->getExtensionMeasurementCorrections() success.");
                mMeasurementCorrections = measurementCorrections;
            } else {
                PRINTERROR("mGnssIface->getExtensionMeasurementCorrections() failed.");
            }


        } else {
            PRINTERROR("mGnssIface->linkToDeath() failed, error: %s", linked.description().c_str());
        }
    } else {
        PRINTERROR("IGnss::getService() call failed.");
    }

    return gardenResult;
}

GARDEN_RESULT GnssMultiClientHidlIGnss::IGnss_1_0_start()
{
    GARDEN_RESULT gardenResult = GARDEN_RESULT_FAILED;

    if (mGnssIface != nullptr) {

        auto result = mGnssIface->start();

        if (result.isOk() && result) {

            PRINTLN("mGnssIface->start success.");
            gardenResult = GARDEN_RESULT_PASSED;

        } else {
            PRINTERROR("mGnssIface->start failed.");
        }
    } else {
        PRINTERROR("HIDL client not created.");
    }

    return gardenResult;
}

GARDEN_RESULT GnssMultiClientHidlIGnss::IGnss_1_0_stop()
{
    GARDEN_RESULT gardenResult = GARDEN_RESULT_FAILED;

    if (mGnssIface != nullptr) {

        auto result = mGnssIface->stop();

        if (result.isOk() && result) {

            PRINTLN("mGnssIface->stop success.");
            gardenResult = GARDEN_RESULT_PASSED;

        } else {
            PRINTERROR("mGnssIface->stop failed.");
        }
    } else {
        PRINTERROR("HIDL client not created.");
    }

    return gardenResult;
}

GARDEN_RESULT GnssMultiClientHidlIGnss::IGnss_1_1_setPositionMode_1_1()
{
    GARDEN_RESULT gardenResult = GARDEN_RESULT_FAILED;

    if (mGnssIface != nullptr) {

        PRINTLN("Please specify position mode options:");
        uint32_t mode = getUserInputInt("Position Mode ( Standalone 0 / MSB 1 / MSA 2 ) : ");
        uint32_t recurrence = getUserInputInt("Recurrence ( Periodic 0 / Single 1 ) : ");
        uint32_t tbf = getUserInputInt("Min Interval (millis) : ");
        uint32_t accuracy = getUserInputInt("Preferred Accuracy (metres) : ");
        uint32_t time = getUserInputInt("Preferred Time (for first fix) (millis) : ");
        bool lowPowerMode = getUserInputYesNo("Use Low Power Mode (y/n) : ");

        auto result = mGnssIface->setPositionMode_1_1(
                static_cast<IGnss_V1_0::GnssPositionMode>(mode),
                static_cast<IGnss_V1_0::GnssPositionRecurrence>(recurrence),
                tbf, accuracy, time, lowPowerMode);

        if (result.isOk() && result) {

            PRINTLN("mGnssIface->setPositionMode_1_1 success.");
            gardenResult = GARDEN_RESULT_PASSED;

        } else {
            PRINTERROR("mGnssIface->setPositionMode_1_1 failed.");
        }
    } else {
        PRINTERROR("HIDL client not created.");
    }

    return gardenResult;
}

GARDEN_RESULT GnssMultiClientHidlIGnss::IGnss_1_1_injectBestLocation()
{
    GARDEN_RESULT gardenResult = GARDEN_RESULT_FAILED;

    android::hardware::gnss::V1_0::GnssLocation location;
    PRINTLN("Please specify values to inject:");
    location.gnssLocationFlags = static_cast<uint16_t>(0x11);
    location.latitudeDegrees = getUserInputDouble("Latitude (degrees): ");
    location.longitudeDegrees = getUserInputDouble("Longitude (degrees): ");
    location.horizontalAccuracyMeters = static_cast<float>(
            getUserInputInt("Horizontal Accuracy (metres): "));

    if (mGnssIface != nullptr) {

        auto result = mGnssIface->injectBestLocation(location);
        if (result.isOk() && result) {
            PRINTLN("mGnssIface->injectBestLocation success.");
            gardenResult = GARDEN_RESULT_PASSED;
        }

    } else {
        PRINTERROR("HIDL client not created.");
    }

    return gardenResult;
}

GARDEN_RESULT GnssMultiClientHidlIGnss::IGnssMeasurement_1_0_close()
{
    GARDEN_RESULT gardenResult = GARDEN_RESULT_FAILED;

    if (mGnssIface != nullptr) {
        if (mGnssMeasurementIface_2_0 != nullptr) {

            auto result = mGnssMeasurementIface_2_0->close();

            if (result.isOk()) {

                PRINTLN("mGnssMeasurementIface_2_0->close success.");
                gardenResult = GARDEN_RESULT_PASSED;

            } else {
                PRINTERROR("mGnssMeasurementIface_2_0->close failed.");
            }
        } else {
            PRINTERROR("mGnssMeasurementIface_2_0 null");
        }
    } else {
        PRINTERROR("HIDL client not created.");
    }

    return gardenResult;
}

GARDEN_RESULT GnssMultiClientHidlIGnss::IGnssMeasurement_1_1_setCallback_1_1()
{
    GARDEN_RESULT gardenResult = GARDEN_RESULT_FAILED;

    if (mGnssIface != nullptr) {
        if (mGnssMeasurementIface_1_1 != nullptr) {

            bool enableFullTracking = getUserInputYesNo("Enable Full Tracking (y/n): ");
            auto result = mGnssMeasurementIface_1_1->setCallback_1_1(mHidlIGnssCb,
                    enableFullTracking);

            if (result.isOk() &&
                    IGnssMeasurement_V1_0::GnssMeasurementStatus::SUCCESS == result) {

                PRINTLN("mGnssMeasurementIface_1_1->setCallback_1_1 success.");
                gardenResult = GARDEN_RESULT_PASSED;

            } else {
                PRINTERROR("mGnssMeasurementIface_1_1->setCallback_1_1 failed.");
            }
        } else {
            PRINTERROR("mGnssMeasurementIface_1_1 null");
        }
    } else {
        PRINTERROR("HIDL client not created.");
    }

    return gardenResult;
}

GARDEN_RESULT GnssMultiClientHidlIGnss::IGnssConfiguration_1_1_setBlacklist()
{
    GARDEN_RESULT gardenResult = GARDEN_RESULT_FAILED;

    if (mGnssIface != nullptr) {
        if (mGnssConfigurationIface != nullptr) {

            std::vector<IGnssConfiguration::BlacklistedSource> tempVector;

            // Populate the blacklist vector
            while(true) {
                if (!getUserInputYesNo("Add a blacklist source (y/n)? ")) {
                    break;
                }
                PRINTLN("GnssConstellationType: GPS 1 / GLONASS 3 / "
                        "QZSS 4 / BEIDOU 5 / GALILEO 6");
                PRINTLN("SV ID: GPS (1-32) / GLONASS (65-96) / QZSS (193-197) / "
                        "BEIDOU (201-237) / GALILEO (301-336)\n");
                IGnssConfiguration::BlacklistedSource source;
                source.constellation = (android::hardware::gnss::V1_0::GnssConstellationType)
                        getUserInputInt("Constellation Type: ");
                source.svid = (uint16_t)getUserInputInt("SV ID: ");
                tempVector.push_back(source);
            }

            hidl_vec<IGnssConfiguration::BlacklistedSource> blacklist(tempVector);
            PRINTLN("blacklist vector size %d", (int)blacklist.size());

            auto result = mGnssConfigurationIface->setBlacklist(blacklist);

            if (result.isOk() && result) {

                PRINTLN("mGnssConfigurationIface->setBlacklist success.");
                gardenResult = GARDEN_RESULT_PASSED;

            } else {
                PRINTERROR("mGnssConfigurationIface->setBlacklist failed.");
            }
        } else {
            PRINTERROR("mGnssConfigurationIface null");
        }
    } else {
        PRINTERROR("HIDL client not created.");
    }

    return gardenResult;
}

GARDEN_RESULT GnssMultiClientHidlIGnss::IGnssConfiguration_2_0_setEsExtensionSec()
{
    GARDEN_RESULT gardenResult = GARDEN_RESULT_FAILED;

    if (mGnssIface != nullptr) {
        if (mGnssVisibilityControl != nullptr) {

            uint32_t extDuration = getUserInputInt("Emergency session extension duration (seconds): ");

            auto result = mGnssConfig_2_0->setEsExtensionSec(extDuration);
            if (result.isOk() && result) {

                PRINTLN("mGnssConfig_2_0->setEsExtensionSec() success.");
                gardenResult = GARDEN_RESULT_PASSED;

            } else {
                PRINTERROR("mGnssConfig_2_0->setEsExtensionSec() failed.");
            }
        } else {
            PRINTERROR("mGnssVisibilityControl null");
        }
    } else {
        PRINTERROR("HIDL client not created.");
    }

    return gardenResult;
}

GARDEN_RESULT GnssMultiClientHidlIGnss::GnssVisibilityControl_setCallback()
{
    GARDEN_RESULT gardenResult = GARDEN_RESULT_FAILED;

    if (mGnssIface != nullptr) {
        if (mGnssConfig_2_0 != nullptr) {

            auto result = mGnssVisibilityControl->setCallback(mHidlIGnssCb);

            if (result.isOk() &&
                true == result) {

                PRINTLN("mGnssVisibilityControl->setCallback success.");
                gardenResult = GARDEN_RESULT_PASSED;

            } else {
                PRINTERROR("mGnssVisibilityControl->setCallback failed.");
            }
        } else {
            PRINTERROR("mGnssConfig_2_0 null");
        }
    } else {
        PRINTERROR("HIDL client not created.");
    }

    return gardenResult;
}

GARDEN_RESULT GnssMultiClientHidlIGnss::GnssVisibilityControl_enableNfwLocationAccess(bool enable)
{
    GARDEN_RESULT gardenResult = GARDEN_RESULT_FAILED;
    hidl_vec<::android::hardware::hidl_string> hVec;
    ::android::hardware::hidl_string hString;

    if (mGnssIface != nullptr) {
        if (mGnssConfig_2_0 != nullptr) {
            if (enable) {
                hString = "Test App";
                hVec.resize(1);
                hVec[0] = hString;
            } else {
                hVec.resize(0);
            }

            auto result = mGnssVisibilityControl->enableNfwLocationAccess(hVec);

            if (result.isOk() &&
                true == result) {

                PRINTLN("mGnssVisibilityControl->enableNfwLocationAccess success.");
                gardenResult = GARDEN_RESULT_PASSED;

            } else {
                PRINTERROR("mGnssVisibilityControl->enableNfwLocationAccess failed.");
            }
        } else {
            PRINTERROR("mGnssConfig_2_0 null");
        }
    } else {
        PRINTERROR("HIDL client not created.");
    }

    return gardenResult;
}

GARDEN_RESULT GnssMultiClientHidlIGnss::IGnss_setCallback()
{
    GARDEN_RESULT gardenResult = GARDEN_RESULT_FAILED;

    if (mGnssIface != nullptr) {
        auto result = mGnssIface->setCallback(mHidlIGnssCb);
        if (!result.isOk()) {
            PRINTERROR("mGnssIface->setCallback failed.");
        } else {
            PRINTLN("mGnssIface->setCallback success.");
            gardenResult = GARDEN_RESULT_PASSED;
        }
    }
    else {
        PRINTERROR("HIDL client not created.");
    }

    return gardenResult;
}

GARDEN_RESULT GnssMultiClientHidlIGnss::IGnss_setCallback_2_0()
{
    GARDEN_RESULT gardenResult = GARDEN_RESULT_FAILED;

    if (mGnssIface != nullptr) {
        auto result = mGnssIface->setCallback_2_0(mHidlIGnssCb);
        if (!result.isOk()) {
            PRINTERROR("mGnssIface->setCallback_2_0 failed.");
        } else {
            PRINTLN("mGnssIface->setCallback_2_0 success.");
            gardenResult = GARDEN_RESULT_PASSED;
        }
    }
    else {
        PRINTERROR("HIDL client not created.");
    }

    return gardenResult;
}

GARDEN_RESULT GnssMultiClientHidlIGnss::IGnss_injectBestLocation_2_0() {
    GARDEN_RESULT gardenResult = GARDEN_RESULT_FAILED;

    GnssLocation location;
    PRINTLN("Please specify values to inject:");
    location.v1_0.gnssLocationFlags = static_cast<uint16_t>(0x11);
    location.v1_0.latitudeDegrees = getUserInputDouble("Latitude (degrees): ");
    location.v1_0.longitudeDegrees = getUserInputDouble("Longitude (degrees): ");
    location.v1_0.horizontalAccuracyMeters = static_cast<float>(
            getUserInputInt("Horizontal Accuracy (metres): "));

    if (mGnssIface != nullptr) {

        auto result = mGnssIface->injectBestLocation_2_0(location);
        if (result.isOk() && result) {
            PRINTLN("mGnssIface->injectBestLocation_2_0 success.");
            gardenResult = GARDEN_RESULT_PASSED;
        }

    } else {
        PRINTERROR("HIDL client not created.");
    }

    return gardenResult;
}

GARDEN_RESULT GnssMultiClientHidlIGnss::GnssBatching_init_2_0()
{
    GARDEN_RESULT gardenResult = GARDEN_RESULT_FAILED;

    if (mGnssIface != nullptr) {
        if (mGnssBatching_2_0 != nullptr) {
            auto result = mGnssBatching_2_0->init_2_0(mHidlIGnssCb);
            if (!result.isOk()) {
                PRINTERROR("mGnssBatching_2_0->init_2_0() failed.");
            } else {
                PRINTLN("mGnssBatching_2_0->init_2_0() success.");
                gardenResult = GARDEN_RESULT_PASSED;
            }
        } else {
            PRINTERROR("mGnssBatching_2_0 null");
        }
    } else {
        PRINTERROR("HIDL client not created.");
    }

    return gardenResult;
}

GARDEN_RESULT GnssMultiClientHidlIGnss::GnssDebug_getDebugData_2_0()
{
    GARDEN_RESULT gardenResult = GARDEN_RESULT_FAILED;

    if (mGnssIface != nullptr) {
        if (mGnssDebug_2_0 != nullptr) {
            IGnssDebug_V2_0::DebugData data;
            auto result = mGnssDebug_2_0->getDebugData_2_0([&data](
                        const IGnssDebug_V2_0::DebugData& debugData) {
                            data = debugData;
                        });
            if (!result.isOk()) {
                PRINTERROR("mGnssDebug_2_0->getDebugData_2_0() failed.");
            } else {
                PRINTLN("mGnssDebug_2_0->getDebugData_2_0() success.");
                PRINTLN("DebugData: postionValid %d, LAT %f, LONG %f, timeEstimate %lld, SVs %u",
                        data.position.valid, data.position.latitudeDegrees,
                        data.position.longitudeDegrees, data.time.timeEstimate,
                        data.satelliteDataArray.size());
                gardenResult = GARDEN_RESULT_PASSED;
            }
        } else {
            PRINTERROR("mGnssDebug_2_0 null");
        }
    } else {
        PRINTERROR("HIDL client not created.");
    }

    return gardenResult;
}

GARDEN_RESULT GnssMultiClientHidlIGnss::GnssMeasurement_2_0_setCallback_2_0() {
    GARDEN_RESULT gardenResult = GARDEN_RESULT_FAILED;

    if (mGnssIface != nullptr) {
        if (mGnssMeasurementIface_2_0 != nullptr) {

            bool enableFullTracking = getUserInputYesNo("Enable Full Tracking (y/n): ");
            auto result = mGnssMeasurementIface_2_0->setCallback_2_0(mHidlIGnssCb,
                    enableFullTracking);

            if (result.isOk() &&
                    IGnssMeasurement_V1_0::GnssMeasurementStatus::SUCCESS == result) {

                PRINTLN("mGnssMeasurementIface_2_0->setCallback_2_0 success.");
                gardenResult = GARDEN_RESULT_PASSED;

            } else {
                PRINTERROR("mGnssMeasurementIface_2_0->setCallback_2_0 failed.");
            }
        } else {
            PRINTERROR("mGnssMeasurementIface_2_0 null");
        }
    } else {
        PRINTERROR("HIDL client not created.");
    }

    return gardenResult;
}
GARDEN_RESULT GnssMultiClientHidlIGnss::AGnss_2_0_setCallback_2_0() {
    GARDEN_RESULT gardenResult = GARDEN_RESULT_FAILED;

    if (mGnssIface != nullptr) {
        if (mAGnss_2_0 != nullptr) {
            auto result = mAGnss_2_0->setCallback(mHidlIGnssCb);
            if (!result.isOk()) {
                PRINTERROR("mAGnss_2_0->setCallback failed.");
            } else {
                PRINTLN("mAGnss_2_0->setCallback success.");
                gardenResult = GARDEN_RESULT_PASSED;
            }
        } else {
            PRINTERROR("mAGnss_2_0 null");
        }
    } else {
        PRINTERROR("HIDL client not created.");
    }

    return gardenResult;
}

GARDEN_RESULT GnssMultiClientHidlIGnss::AGnss_2_0_dataConnOpen_2_0() {
    GARDEN_RESULT gardenResult = GARDEN_RESULT_FAILED;
    if (mGnssIface != nullptr) {
        if (mAGnss_2_0 != nullptr) {
            uint64_t networkHandle = getUserInputMask64Bit(
                    "Input handle represents the network for use with the NDK API");
            std::string apn = getUserInputString("Input AGnss APN:");
            int apnType = getUserInputInt("Input AGnss APN IP Type:");
            auto result = mAGnss_2_0->dataConnOpen(networkHandle, apn,
                    (IAGnss_V2_0::ApnIpType)apnType);

            if (result.isOk() && true == result) {
                PRINTLN("mAGnss_2_0->dataConnOpen() success.");
                gardenResult = GARDEN_RESULT_PASSED;

            } else {
                PRINTERROR("mAGnss_2_0->dataConnOpen() failed.");
            }
        } else {
            PRINTERROR("mAGnss_2_0 null");
        }
    } else {
        PRINTERROR("HIDL client not created.");
    }

    return gardenResult;
}

GARDEN_RESULT GnssMultiClientHidlIGnss::AGnss_2_0_setServer_2_0() {
    GARDEN_RESULT gardenResult = GARDEN_RESULT_FAILED;
    if (mGnssIface != nullptr) {
        if (mAGnss_2_0 != nullptr) {
            int agnssType = getUserInputInt("Input AGnss type Specifies if SUPL or C2K");
            std::string host = getUserInputString("Input Hostname of the AGNSS server");
            int port = getUserInputInt("Input Port number associated with the server");
            auto result = mAGnss_2_0->setServer((IAGnssCallback_2_0::AGnssType)agnssType, host,
                    port);

            if (result.isOk() && true == result) {
                PRINTLN("mAGnss_2_0->setServer() success.");
                gardenResult = GARDEN_RESULT_PASSED;

            } else {
                PRINTERROR("mAGnss_2_0->setServer() failed.");
            }
        } else {
            PRINTERROR("mAGnss_2_0 null");
        }
    } else {
        PRINTERROR("HIDL client not created.");
    }

    return gardenResult;
}

GARDEN_RESULT GnssMultiClientHidlIGnss::AGnss_2_0_dataConnClosed_2_0() {
    GARDEN_RESULT gardenResult = GARDEN_RESULT_FAILED;
    if (mGnssIface != nullptr) {
        if (mAGnss_2_0 != nullptr) {
            auto result = mAGnss_2_0->dataConnClosed();
            if (result.isOk() && true == result) {
                PRINTLN("mAGnss_2_0->dataConnClosed() success.");
                gardenResult = GARDEN_RESULT_PASSED;

            } else {
                PRINTERROR("mAGnss_2_0->dataConnClosed() failed.");
            }
        } else {
            PRINTERROR("mAGnss_2_0 null");
        }
    } else {
        PRINTERROR("HIDL client not created.");
    }

    return gardenResult;
}

GARDEN_RESULT GnssMultiClientHidlIGnss::hidlClientCleanup()
{
    GARDEN_RESULT gardenResult = GARDEN_RESULT_FAILED;

    if (mGnssIface != nullptr) {
        mGnssIface->cleanup();
        PRINTLN("mGnssIface->cleanup success.");
        gardenResult = GARDEN_RESULT_PASSED;
    } else {
        PRINTERROR("HIDL client not created.");
    }

    return gardenResult;
}

} // namespace garden
