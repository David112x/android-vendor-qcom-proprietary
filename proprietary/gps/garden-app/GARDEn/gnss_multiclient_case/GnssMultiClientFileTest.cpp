/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#define LOG_NDEBUG 0
#define LOG_TAG "GARDEN_GMCC_HidlIGnss"

#include "GnssMultiClientCase.h"
#include "GnssMultiClientFileTest.h"
#include "GnssMultiClientCaseUtils.h"

#include <iostream>
#include <fstream>
#include <string>
#include <thread>

namespace garden {

#define printGnssConfigBlacklistSvId GmccUtils::get().printGnssConfigBlacklistSvId
#define convertGnssSvIdMaskToList GmccUtils::get().convertGnssSvIdMaskToList
#define strUtilTokenize GmccUtils::get().strUtilTokenize


GnssMultiClientFileTest* GnssMultiClientFileTest::sInstance = nullptr;

/* Entry Point Method */
GARDEN_RESULT GnssMultiClientFileTest::run()
{
    GARDEN_RESULT result = GARDEN_RESULT_INVALID;
    PRINTLN("GnssMultiClientFileTest::fileTest()");

    // Set utils class in automation mode
    GmccUtils::get().sFileTestMode = true;

    // Read the commands file
    std::string commandsFilePath = "/data/vendor/location/garden/commands.txt";
    std::ifstream commandsFile(commandsFilePath);

    if (commandsFile.is_open()) {

        // Let's run all commands one by one, break on failure
        std::string line;
        while(getline(commandsFile, line)) {
            result = processCommand(line);
            if (GARDEN_RESULT_PASSED != result) {
                PRINTERROR("Command processing failed, aborting.");
                break;
            }
        }

        // close the file
        commandsFile.close();

    } else {
        PRINTERROR("Commands file missing @ %s", commandsFilePath.c_str());
        result = GARDEN_RESULT_FAILED;
    }

    return result;
}

/* Process a command line */
GARDEN_RESULT GnssMultiClientFileTest::processCommand(std::string command) {

    GARDEN_RESULT result = GARDEN_RESULT_FAILED;
    PRINTLN("Executing command: %s", command.c_str());

    // Tokenize command
    std::vector<std::string> commandTokens = strUtilTokenize(command);
    PRINTLN("Command Token Count: %lu\n", commandTokens.size());

    if (commandTokens.size() > 0) {

        if (0 == commandTokens[0].compare("help")) {
            result = cmdHelp(commandTokens);
        } else if (0 == commandTokens[0].compare("setblacklist")) {
            result = cmdSetBlacklist(commandTokens);
        } else if (0 == commandTokens[0].compare("getblacklist")) {
            result = cmdGetBlacklist(commandTokens);
        } else if (0 == commandTokens[0].compare("setconstellation")) {
            result = cmdSetConstellation(commandTokens);
        } else if (0 == commandTokens[0].compare("getconstellation")) {
            result = cmdGetConstellation(commandTokens);
        } else if (0 == commandTokens[0].compare("resetconstellation")) {
            result = cmdResetConstellation(commandTokens);
        } else if (0 == commandTokens[0].compare("sleep")) {
            result = cmdSleep(commandTokens);
        } else if (0 == commandTokens[0].compare("repeat")) {
            result = cmdRepeat(commandTokens);
        } else {
            PRINTERROR("Unsupported command: [%s]", commandTokens[0].c_str());
            result = GARDEN_RESULT_FAILED;
        }

    } else {
        PRINTERROR("Invalid command: [%s]", command.c_str());
    }

    return result;
}

GARDEN_RESULT
GnssMultiClientFileTest::cmdHelp(const std::vector<std::string>& cmdTokens)
{
    if (1 == cmdTokens.size()) {

        PRINTLN("Available commands:");
        PRINTLN("- setblacklist");
        PRINTLN("- getblacklist");
        PRINTLN("- setconstellation");
        PRINTLN("- getconstellation");
        PRINTLN("- resetconstellation");
        PRINTLN("\nFor help on a specific cmd, \"help <command>\"");
    }

    if (cmdTokens.size() > 1) {

        if (0 == cmdTokens[1].compare("setblacklist")) {

            PRINTLN("Set Blacklist cmd usage:");
            PRINTLN("setblacklist <constellation> <blacklist_mask>");
            PRINTLN("constellation values: gps / glo / gal / bds / qzss");
            PRINTLN("blacklist_mask value: hex value");
            PRINTLN("e.g.");
            PRINTLN("setblacklist gps 0x1 gal 0xFF bds 0x23");

        } else if (0 == cmdTokens[1].compare("getblacklist")) {

            PRINTLN("Get Blacklist cmd usage:");
            PRINTLN("getblacklist");
            PRINTLN("Sends the Get Blacklist command to Modem and waits for response.");

        } else if (0 == cmdTokens[1].compare("setconstellation")) {

            PRINTLN("Set Constellation cmd usage:");
            PRINTLN("setconstellation enabled <constellation> ... disabled <constellation> ...");
            PRINTLN("constellation values: gps / glo / gal / bds / qzss");
            PRINTLN("e.g.");
            PRINTLN("setconstellation enabed gal bds disabled qzss");

        } else if (0 == cmdTokens[1].compare("getConstellation")) {

            PRINTLN("Get Constellation cmd usage:");
            PRINTLN("getconstellation");
            PRINTLN("Sends the Get Constellation command to Modem and waits for response.");

        } else if (0 == cmdTokens[1].compare("resetConstellation")) {

            PRINTLN("Reset Constellation cmd usage:");
            PRINTLN("resetconstellation");
            PRINTLN("Sends the Reset Constellation command to Modem.");
        }
    }

    return GARDEN_RESULT_PASSED;
}

GARDEN_RESULT
GnssMultiClientFileTest::cmdSetBlacklist(const std::vector<std::string>& cmdTokens)
{
    uint64_t gpsMask = 0;
    uint64_t galMask = 0;
    uint64_t gloMask = 0;
    uint64_t bdsMask = 0;
    uint64_t qzssMask = 0;
    uint64_t* mask = nullptr;

    bool isNextArgConstellation = true;
    for (int i=1; i<(int)cmdTokens.size(); i++) {
        if (isNextArgConstellation) {
            // Parse constellation name
            if (0 == cmdTokens[i].compare("gps")) {
                mask = &gpsMask;
            } else if (0 == cmdTokens[i].compare("gal")) {
                mask = &galMask;
            } else if (0 == cmdTokens[i].compare("glo")) {
                mask = &gloMask;
            } else if (0 == cmdTokens[i].compare("bds")) {
                mask = &bdsMask;
            } else if (0 == cmdTokens[i].compare("qzss")) {
                mask = &qzssMask;
            } else {
                PRINTERROR("Invalid constellation name: [%s]", cmdTokens[i].c_str());
                return GARDEN_RESULT_FAILED;
            }
            isNextArgConstellation = false;
        } else {
            // Parse mask value
            std::stringstream inputStream(cmdTokens[i]);
            if (!(inputStream >> std::hex >> (*mask))) {
                PRINTERROR("Failed to parse mask value [%s]", cmdTokens[i].c_str());
                return GARDEN_RESULT_FAILED;
            }
            isNextArgConstellation = true;
        }
    }

    // Now we have all mask values
    PRINTLN("Set Blacklist Masks specified:");
    PRINTLN("GPS: 0x%lx", gpsMask);
    PRINTLN("GAL: 0x%lx", galMask);
    PRINTLN("GLO: 0x%lx", gloMask);
    PRINTLN("BDS: 0x%lx", bdsMask);
    PRINTLN("QZSS: 0x%lx\n", qzssMask);

    // Create control client
    PRINTLN("Creating Control Client..");
    mGmcc.createLocationControlAPIClient();

    // Validate control client ptr
    if (mGmcc.mLocControlClientInfo.clientPtr == nullptr ||
            mGmcc.mLocControlClientInfo.clientPtr->mControlClient == nullptr) {
        PRINTERROR("Control client ptr NULL");
        return GARDEN_RESULT_FAILED;
    }

    // Create config
    PRINTLN("\nCreating GnssConfig for SV blacklist..");
    GnssConfig config = {};
    config.flags |= GNSS_CONFIG_FLAGS_BLACKLISTED_SV_IDS_BIT;

    // Convert mask values to sv ids
    convertGnssSvIdMaskToList(gpsMask, config.blacklistedSvIds, 1, GNSS_SV_TYPE_GPS);
    convertGnssSvIdMaskToList(gloMask, config.blacklistedSvIds,
            GNSS_SV_CONFIG_GLO_INITIAL_SV_ID, GNSS_SV_TYPE_GLONASS);
    convertGnssSvIdMaskToList(qzssMask, config.blacklistedSvIds,
            GNSS_SV_CONFIG_QZSS_INITIAL_SV_ID, GNSS_SV_TYPE_QZSS);
    convertGnssSvIdMaskToList(bdsMask, config.blacklistedSvIds,
            GNSS_SV_CONFIG_BDS_INITIAL_SV_ID, GNSS_SV_TYPE_BEIDOU);
    convertGnssSvIdMaskToList(galMask, config.blacklistedSvIds,
            GNSS_SV_CONFIG_GAL_INITIAL_SV_ID, GNSS_SV_TYPE_GALILEO);

    printGnssConfigBlacklistSvId(config);

    // Send command to control api
    PRINTLN("\nSubmitting the specified config to Control API..");
    mGmcc.mLocControlClientInfo.clientPtr->mControlClient->locAPIGnssUpdateConfig(config);

    // Clean up control api
    PRINTLN("\nDeleting Location control api instance..");
    mGmcc.destroyLocationControlAPIClient();

    return GARDEN_RESULT_PASSED;
}

GARDEN_RESULT
GnssMultiClientFileTest::cmdGetBlacklist(const std::vector<std::string>& /*cmdTokens*/)
{
    // Create control client
    PRINTLN("Creating Control Client..");
    mGmcc.createLocationControlAPIClient();

    // Validate control client ptr
    if (nullptr == mGmcc.mLocControlClientInfo.clientPtr ||
            nullptr == mGmcc.mLocControlClientInfo.clientPtr->mControlClient) {
        PRINTERROR("Control client ptr NULL");
        return GARDEN_RESULT_FAILED;
    }

    // Fetch config mask
    GnssConfigFlagsMask mask = GNSS_CONFIG_FLAGS_BLACKLISTED_SV_IDS_BIT;

    // Send command to control api
    PRINTLN("\nFetching the GNSS Config..");
    mGmcc.mLocControlClientInfo.clientPtr->mControlClient->locAPIGnssGetConfig(mask);

    // Clean up control api
    PRINTLN("\nDeleting Location control api instance..");
    mGmcc.destroyLocationControlAPIClient();

    return GARDEN_RESULT_PASSED;
}

GARDEN_RESULT
GnssMultiClientFileTest::cmdSetConstellation(const std::vector<std::string>& cmdTokens)
{
    PRINTLN("setconstellation command format: [setconstellation disabled <disabled_constellations>]");
    PRINTLN("e.g. [setconstellation disabled glo gal]");
    PRINTLN("Constellations strings [glo gal bds qzss]");

    if (0 != cmdTokens[1].compare("disabled")) {
        PRINTERROR("Invalid command format");
        PRINTLN("setconstellation command format: [setconstellation disabled <disabled_constellations>]");
        return GARDEN_RESULT_FAILED;
    }

    uint64_t disabledSvTypeMask = 0;
    uint64_t enabledSvTypeMask = 0;
    for (int i=2; i<(int)cmdTokens.size(); i++) {

        if (0 == cmdTokens[i].compare("glo")) {
            disabledSvTypeMask |= GNSS_SV_TYPES_MASK_GLO_BIT;
        } else if (0 == cmdTokens[i].compare("gal")) {
            disabledSvTypeMask |= GNSS_SV_TYPES_MASK_GAL_BIT;
        } else if (0 == cmdTokens[i].compare("bds")) {
            disabledSvTypeMask |= GNSS_SV_TYPES_MASK_BDS_BIT;
        } else if (0 == cmdTokens[i].compare("qzss")) {
            disabledSvTypeMask |= GNSS_SV_TYPES_MASK_QZSS_BIT;
        } else {
            PRINTERROR("Invalid constellation to disable: %s", cmdTokens[i].c_str());
        }
    }
    enabledSvTypeMask = ~disabledSvTypeMask;

    return mGmcc.gnssIfaceAPIUpdateSvTypeConfig(enabledSvTypeMask, disabledSvTypeMask);
}

GARDEN_RESULT
GnssMultiClientFileTest::cmdGetConstellation(const std::vector<std::string>& /*cmdTokens*/)
{
    return mGmcc.gnssIfaceAPIGetSvTypeConfig();
}

GARDEN_RESULT
GnssMultiClientFileTest::cmdResetConstellation(const std::vector<std::string>& /*cmdTokens*/)
{
    return mGmcc.gnssIfaceAPIResetSvTypeConfig();
}

GARDEN_RESULT
GnssMultiClientFileTest::cmdSleep(const std::vector<std::string>& cmdTokens)
{
    PRINTLN("sleep command format: [sleep <duration_seconds>]");

    // Parse sleep duration
    std::stringstream inputStream(cmdTokens[1]);
    uint32_t durationSec = 0;
    if (inputStream >> durationSec) {

        PRINTLN("Going to sleep for %d seconds", durationSec);
        std::this_thread::sleep_for(std::chrono::seconds(durationSec));

    } else {
        PRINTERROR("Failed to parse sleep duration [%s]", cmdTokens[1].c_str());
        return GARDEN_RESULT_FAILED;
    }

    return GARDEN_RESULT_PASSED;
}

GARDEN_RESULT
GnssMultiClientFileTest::cmdRepeat(const std::vector<std::string>& /*cmdTokens*/)
{
    PRINTLN("repeat command format: [repeat <number_of_cmds_to_repeat> <times_to_repeat>]");
    PRINTLN("TBD");

    return GARDEN_RESULT_PASSED;
}

} // namespace garden
