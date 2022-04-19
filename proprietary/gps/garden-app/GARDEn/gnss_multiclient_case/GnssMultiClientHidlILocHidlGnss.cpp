/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#define LOG_NDEBUG 0
#define LOG_TAG "GARDEN_GMCC_ILocHidlGnss"

#include "GnssMultiClientHidlILocHidlGnss.h"
#include "GnssMultiClientCaseUtils.h"

#include <android/hardware/gnss/1.0/types.h>

using ::vendor::qti::gnss::V3_0::ILocHidlGnss;

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


/* GnssMultiClientHidlILocHidlGnss static elements */
GnssMultiClientHidlILocHidlGnss* GnssMultiClientHidlILocHidlGnss::sInstance = nullptr;

/* GnssMultiClientHidlILocHidlGnss Public APIs */
GARDEN_RESULT GnssMultiClientHidlILocHidlGnss::menuTest()
{
    bool exit_loop = false;
    GARDEN_RESULT gardenResult = GARDEN_RESULT_INVALID;

    while(!exit_loop) {
        PRINTLN("\n\n"
                "1: Create HIDL client for ILocHidlGnss 3.0 \n"
                "1001: ILocHidlIzatConfig 1.0 -> init \n"
                "1002: ILocHidlIzatConfig 1.0 -> readConfig \n"
                "99: Display this menu again \n"
                "0: <-- back\n");
        int choice = getUserInputInt("Enter choice: ");

        switch (choice) {
        case 1:
            gardenResult = createHidlClient();
            break;
        case 1001:
            gardenResult = ILocHidlIzatConfig_1_0_init();
            break;
        case 1002:
            gardenResult = ILocHidlIzatConfig_1_0_readConfig();
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
void LocHidlGnssDeathRecipient::serviceDied(uint64_t /*cookie*/, const wp<IBase>& /*who*/) {
    PRINTERROR("ILocHidlGnss service died");
    GnssMultiClientHidlILocHidlGnss::get().mLocHidlGnssIface = nullptr;
}

/* GnssMultiClientHidlILocHidlGnss TEST CASES */
GARDEN_RESULT GnssMultiClientHidlILocHidlGnss::createHidlClient()
{
    GARDEN_RESULT gardenResult = GARDEN_RESULT_FAILED;

    // Get IGNSS service
    mLocHidlGnssIface = ILocHidlGnss::getService(GNSS_VENDOR_SERVICE_INSTANCE);
    if (mLocHidlGnssIface != nullptr) {
        PRINTLN("ILocHidlGnss::getService() success.");

        // Link to IGNSS service Death
        mLocHidlGnssDeathRecipient = new LocHidlGnssDeathRecipient();
        android::hardware::Return<bool> linked = 
            mLocHidlGnssIface->linkToDeath(mLocHidlGnssDeathRecipient, 0);
        if (linked.isOk() && linked) {
            PRINTLN("mLocHidlGnssIface->linkToDeath() success.");

            // Get Extension : IGnssConfiguration 1.1
            auto izatConfig = mLocHidlGnssIface->getExtensionLocHidlIzatConfig();
            if (izatConfig.isOk()) {

                PRINTLN("mLocHidlGnssIface->getExtensionLocHidlIzatConfig() success.");
                mLocHidlGnssExtensionIzatConfig = izatConfig;

                gardenResult = GARDEN_RESULT_PASSED;

            } else {
                PRINTERROR("mLocHidlGnssIface->getExtensionLocHidlIzatConfig() failed.");
            }
        } else {
            PRINTERROR("mLocHidlGnssIface->linkToDeath() failed, error: %s", 
                         linked.description().c_str());
        }
    } else {
        PRINTERROR("ILocHidlGnss::getService() call failed.");
    }

    return gardenResult;
}

GARDEN_RESULT GnssMultiClientHidlILocHidlGnss::ILocHidlIzatConfig_1_0_init()
{
    GARDEN_RESULT gardenResult = GARDEN_RESULT_FAILED;

    if (mLocHidlGnssIface != nullptr) {
        if (mLocHidlGnssExtensionIzatConfig != nullptr) {

            auto result = mLocHidlGnssExtensionIzatConfig->init(mLocHidlGnssCb);

            if (result.isOk() && result) {

                PRINTLN("mLocHidlGnssExtensionIzatConfig->init success.");
                gardenResult = GARDEN_RESULT_PASSED;

            } else {
                PRINTERROR("mLocHidlGnssExtensionIzatConfig->init failed.");
            }
        } else {
            PRINTERROR("mLocHidlGnssExtensionIzatConfig null");
        }
    } else {
        PRINTERROR("LOC HIDL client not created.");
    }

    return gardenResult;
}

GARDEN_RESULT GnssMultiClientHidlILocHidlGnss::ILocHidlIzatConfig_1_0_readConfig()
{
    GARDEN_RESULT gardenResult = GARDEN_RESULT_FAILED;

    if (mLocHidlGnssIface != nullptr) {
        if (mLocHidlGnssExtensionIzatConfig != nullptr) {

            auto result = mLocHidlGnssExtensionIzatConfig->readConfig();

            if (result.isOk() && result) {

                PRINTLN("mLocHidlGnssExtensionIzatConfig->readConfig success.");
                gardenResult = GARDEN_RESULT_PASSED;

            } else {
                PRINTERROR("mLocHidlGnssExtensionIzatConfig->readConfig failed.");
            }
        } else {
            PRINTERROR("mLocHidlGnssExtensionIzatConfig null");
        }
    } else {
        PRINTERROR("LOC HIDL client not created.");
    }

    return gardenResult;
}

} // namespace garden
