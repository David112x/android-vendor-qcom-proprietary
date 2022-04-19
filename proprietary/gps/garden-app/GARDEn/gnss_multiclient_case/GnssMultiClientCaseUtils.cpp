/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/
#define LOG_NDEBUG 0
#define LOG_TAG "GARDEN_GMCC_Utils"

#include "LocationAPIClientBase.h"
#include "GnssMultiClientCaseUtils.h"

namespace garden {

GmccUtils* GmccUtils::sInstance = nullptr;
bool GmccUtils::sFileTestMode = false;

// Utilities
void GmccUtils::getUserInputEnterToContinue()
{
    // Don't wait for user input if in automation mode
    if (!sFileTestMode) {
        PRINT("\nPress Enter to continue..");
        std::string userInput;
        std::cin.clear();
        std::getline(std::cin, userInput);
    }
    PRINTLN("");
}

int GmccUtils::getUserInputInt(std::string prompt)
{
    std::string userInput;
    int intValue = -1;
    while(true) {
        PRINT("%s", prompt.c_str());
        std::cin.clear();
        std::getline(std::cin, userInput);
        std::stringstream inputStream(userInput);
        if ((inputStream >> intValue)) {
            break;
        }
        PRINTERROR("Please enter a valid number.");
    }
    PRINTLN("");
    return intValue;
}

double GmccUtils::getUserInputDouble(std::string prompt)
{
    std::string userInput;
    double doubleValue = -1.0;
    while(true) {
        PRINT("%s", prompt.c_str());
        std::cin.clear();
        std::getline(std::cin, userInput);
        std::stringstream inputStream(userInput);
        if ((inputStream >> doubleValue)) {
            break;
        }
        PRINTERROR("Please enter a valid double value.");
    }
    PRINTLN("");
    return doubleValue;
}

uint64_t GmccUtils::getUserInputMask64Bit(std::string prompt)
{
    std::string userInput;
    uint64_t mask = 0;
    while(true) {
        PRINT("%s", prompt.c_str());
        std::cin.clear();
        std::getline(std::cin, userInput);
        std::stringstream inputStream(userInput);
        if ((inputStream >> std::hex >> mask)) {
            break;
        }
        PRINTERROR("Please enter a valid 64 bit mask value.");
    }
    PRINTLN("");
    return mask;
}

std::string GmccUtils::getUserInputString(std::string prompt)
{
    PRINT("%s", prompt.c_str());
    std::string userInput;
    std::cin.clear();
    std::getline(std::cin, userInput);
    std::stringstream inputStream(userInput);
    if (!(inputStream >> userInput)) {
        PRINTERROR("Invalid input !");
    }
    PRINTLN("");
    return userInput;
}

bool GmccUtils::getUserInputYesNo(std::string prompt)
{
    std::string choice = getUserInputString(prompt);
    return (0 == choice.compare("y") ||
            0 == choice.compare("yes") ||
            0 == choice.compare("Y") ||
            0 == choice.compare("YES"));
}

SESSION_MODE
GmccUtils::getUserInputSessionMode(std::string prompt)
{
    PRINTLN("%s", prompt.c_str());
    SESSION_MODE sessionMode = SESSION_MODE_NONE;

    PRINTLN("0. SESSION_MODE_NONE\n"
            "1. SESSION_MODE_ON_FULL\n"
            "2. SESSION_MODE_ON_FIX\n"
            "3. SESSION_MODE_ON_TRIP_COMPLETED");
    int choice = getUserInputInt("Enter choice: ");
    switch(choice) {
    case 0:
        sessionMode = SESSION_MODE_NONE;
        break;
    case 1:
        sessionMode = SESSION_MODE_ON_FULL;
        break;
    case 2:
        sessionMode = SESSION_MODE_ON_FIX;
        break;
    case 3:
        sessionMode = SESSION_MODE_ON_TRIP_COMPLETED;
        break;
    default:
        sessionMode = SESSION_MODE_NONE;
        break;
    }

    return sessionMode;
}

TrackingOptions
GmccUtils::getUserInputTrackingOptions(std::string prompt)
{
    PRINTLN("%s\n", prompt.c_str());
    TrackingOptions options = {};

    PRINTLN("SUPL Mode: \n"
            "0: Standalone \n"
            "1: MSB \n"
            "2: MSA");
    int choice = getUserInputInt("Enter choice: ");
    switch(choice) {
    case 0:
        options.mode = GNSS_SUPL_MODE_STANDALONE;
        break;
    case 1:
        options.mode = GNSS_SUPL_MODE_MSB;
        break;
    case 2:
        options.mode = GNSS_SUPL_MODE_MSA;
        break;
    default:
        options.mode = GNSS_SUPL_MODE_STANDALONE;
        break;
    }

    options.minInterval = getUserInputInt("Min Interval (ms): ");
    options.minDistance = getUserInputInt("Min Distance (metres): ");

    options.size = sizeof(LocationOptions);

    if (getUserInputYesNo("Want to specify power options (y/n) ? ")) {

        PRINT("Power Mode: \n"
                "1: Improved Accuracy \n"
                "2: Normal \n"
                "3: Background defined power \n"
                "4: Background defined time \n"
                "5: Background keep warm \n");
        int choice = getUserInputInt("Enter choice: ");
        switch(choice) {
        case 1:
            options.powerMode = GNSS_POWER_MODE_M1;
            break;
        case 2:
            options.powerMode = GNSS_POWER_MODE_M2;
            break;
        case 3:
            options.powerMode = GNSS_POWER_MODE_M3;
            break;
        case 4:
            options.powerMode = GNSS_POWER_MODE_M4;
            break;
        case 5:
            options.powerMode = GNSS_POWER_MODE_M5;
            break;
        default:
            options.powerMode = GNSS_POWER_MODE_INVALID;
            break;
        }

        options.tbm = getUserInputInt("Time between measurements (ms): ");
    }

    return options;
}

int GmccUtils::getUserInputLocClientIndex(std::string prompt, uint32_t clientListSize)
{
    PRINTLN("%s", prompt.c_str());
    PRINTLN("Location API Client count: %d", clientListSize);

    // validate
    if (0 == clientListSize) {
        PRINTERROR("There are no Location API Clients created yet !");
        return -1;
    }

    int clientIndex = getUserInputInt("Index of Location API Client to use: ");

    if (clientIndex < 0 || clientIndex >= (int)clientListSize) {
        PRINTERROR("Invalid client index, should be in [0, %d]",
                clientListSize);
        return -1;
    }

    return clientIndex;
}

GnssConfig GmccUtils::getUserInputGnssConfig(std::string prompt)
{
    PRINT("%s", prompt.c_str());

    bool exit_loop = false;
    GnssConfig config = {};

    while(!exit_loop) {
        PRINTLN("Which Gnss Config would you like to update:\n\n"
                "1: GPS Lock \n"
                "2: SUPL Version \n"
                "3: Assistance Server \n"
                "4: LPP Profile \n"
                "5: LPPE Control Plane Mask \n"
                "6: LPPE User Plane Mask \n"
                "7: AGlonass Position Protocol Mask \n"
                "8: Emergency PDN \n"
                "9: SUPL Emergency Services \n"
                "10: SUPL Mode Mask \n"
                "11: Blacklisted SV IDs \n"
                "99: Display this menu again \n"
                "0: <-- back ( to submit the updated config ) \n");
        int choice = getUserInputInt("Enter choice: ");

        switch (choice) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
        case 10:
            PRINTERROR("Not yet supported !");
            break;
        case 11:
            getUserInputGnssConfigBlacklistSvId(config);
            printGnssConfigBlacklistSvId(config);
            getUserInputEnterToContinue();
            break;
        case 99:
            continue;
        case 0:
            exit_loop = true;
            break;
        default:
            PRINTERROR("Invalid command");
        }
    }

    config.size = sizeof(GnssConfig);
    return config;
}

GnssConfigFlagsMask
GmccUtils::getUserInputGnssConfigFlagsMask(std::string prompt)
{
    PRINTLN("%s\n", prompt.c_str());

    PRINTLN("Here are the bit values for the config mask:\n");
    PRINTLN("GNSS_CONFIG_FLAGS_GPS_LOCK_VALID_BIT                   = (1<<0)   = 1     =    0x1");
    PRINTLN("GNSS_CONFIG_FLAGS_SUPL_VERSION_VALID_BIT               = (1<<1)   = 2     =    0x2");
    PRINTLN("GNSS_CONFIG_FLAGS_SET_ASSISTANCE_DATA_VALID_BIT        = (1<<2)   = 4     =    0x4");
    PRINTLN("GNSS_CONFIG_FLAGS_LPP_PROFILE_VALID_BIT                = (1<<3)   = 8     =    0x8");
    PRINTLN("GNSS_CONFIG_FLAGS_LPPE_CONTROL_PLANE_VALID_BIT         = (1<<4)   = 16    =   0x10");
    PRINTLN("GNSS_CONFIG_FLAGS_LPPE_USER_PLANE_VALID_BIT            = (1<<5)   = 32    =   0x20");
    PRINTLN("GNSS_CONFIG_FLAGS_AGLONASS_POSITION_PROTOCOL_VALID_BIT = (1<<6)   = 64    =   0x40");
    PRINTLN("GNSS_CONFIG_FLAGS_EM_PDN_FOR_EM_SUPL_VALID_BIT         = (1<<7)   = 128   =   0x80");
    PRINTLN("GNSS_CONFIG_FLAGS_SUPL_EM_SERVICES_BIT                 = (1<<8)   = 256   =  0x100");
    PRINTLN("GNSS_CONFIG_FLAGS_SUPL_MODE_BIT                        = (1<<9)   = 512   =  0x200");
    PRINTLN("GNSS_CONFIG_FLAGS_BLACKLISTED_SV_IDS_BIT               = (1<<10)  = 1024  =  0x400");

    GnssConfigFlagsMask mask =
            (GnssConfigFlagsMask)getUserInputMask64Bit("\nEnter the config mask value: ");

    // print the selected mask
    PRINTLN("\nGnss Config Mask Bits you've selected:\n");
    if (mask & GNSS_CONFIG_FLAGS_GPS_LOCK_VALID_BIT) {
        PRINTLN("GNSS_CONFIG_FLAGS_GPS_LOCK_VALID_BIT");
    }
    if (mask & GNSS_CONFIG_FLAGS_SUPL_VERSION_VALID_BIT) {
        PRINTLN("GNSS_CONFIG_FLAGS_SUPL_VERSION_VALID_BIT");
    }
    if (mask & GNSS_CONFIG_FLAGS_SET_ASSISTANCE_DATA_VALID_BIT) {
        PRINTLN("GNSS_CONFIG_FLAGS_SET_ASSISTANCE_DATA_VALID_BIT");
    }
    if (mask & GNSS_CONFIG_FLAGS_LPP_PROFILE_VALID_BIT) {
        PRINTLN("GNSS_CONFIG_FLAGS_LPP_PROFILE_VALID_BIT");
    }
    if (mask & GNSS_CONFIG_FLAGS_LPPE_CONTROL_PLANE_VALID_BIT) {
        PRINTLN("GNSS_CONFIG_FLAGS_LPPE_CONTROL_PLANE_VALID_BIT");
    }
    if (mask & GNSS_CONFIG_FLAGS_LPPE_USER_PLANE_VALID_BIT) {
        PRINTLN("GNSS_CONFIG_FLAGS_LPPE_USER_PLANE_VALID_BIT");
    }
    if (mask & GNSS_CONFIG_FLAGS_AGLONASS_POSITION_PROTOCOL_VALID_BIT) {
        PRINTLN("GNSS_CONFIG_FLAGS_AGLONASS_POSITION_PROTOCOL_VALID_BIT");
    }
    if (mask & GNSS_CONFIG_FLAGS_EM_PDN_FOR_EM_SUPL_VALID_BIT) {
        PRINTLN("GNSS_CONFIG_FLAGS_EM_PDN_FOR_EM_SUPL_VALID_BIT");
    }
    if (mask & GNSS_CONFIG_FLAGS_SUPL_EM_SERVICES_BIT) {
        PRINTLN("GNSS_CONFIG_FLAGS_SUPL_EM_SERVICES_BIT");
    }
    if (mask & GNSS_CONFIG_FLAGS_SUPL_MODE_BIT) {
        PRINTLN("GNSS_CONFIG_FLAGS_SUPL_MODE_BIT");
    }
    if (mask & GNSS_CONFIG_FLAGS_BLACKLISTED_SV_IDS_BIT) {
        PRINTLN("GNSS_CONFIG_FLAGS_BLACKLISTED_SV_IDS_BIT");
    }

    return mask;
}

void
GmccUtils::getUserInputGnssConfigBlacklistSvId(GnssConfig& config)
{
    GnssSvIdSource source = {};
    config.flags |= GNSS_CONFIG_FLAGS_BLACKLISTED_SV_IDS_BIT;

    // Check if mask input is needed
    bool maskInput = getUserInputYesNo(
            "Enter mask values (y/n) (n to enter SV ID individually): ");
    if (maskInput) {
        // Input mask values for all constellations
        uint64_t gpsMask = getUserInputMask64Bit("Enter GPS Blacklist Mask (SV Range 1-32): ");
        uint64_t gloMask = getUserInputMask64Bit("Enter GLO Blacklist Mask (SV Range 65-96): ");
        uint64_t qzssMask = getUserInputMask64Bit("Enter QZSS Blacklist Mask (SV Range 193-197): ");
        uint64_t bdsMask = getUserInputMask64Bit("Enter BDS Blacklist Mask (SV Range 201-237): ");
        uint64_t galMask = getUserInputMask64Bit("Enter GAL Blacklist Mask (SV Range 301-336): ");

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

    } else {
        while(true) {
            source = getUserInputGnssSvIdSource("SV ID to blacklist:");
            config.blacklistedSvIds.push_back(source);
            std::string another = getUserInputString("Another one (y/n) : ");
            if (0 != another.compare("y")) {
                break;
            }
        }
    }
}

GnssSvIdSource
GmccUtils::getUserInputGnssSvIdSource(std::string prompt)
{
    PRINT("%s", prompt.c_str());
    GnssSvIdSource source = {};

    // Ask for constellation
    PRINTLN("Select a constellation:\n"
            "1: GPS (1-32) (Can't be blacklisted) \n"
            "2: SBAS (Can't be blacklisted) \n"
            "3: GLONASS (65-96) \n"
            "4: QZSS (193-197) \n"
            "5: BEIDOU (201-237) \n"
            "6: GALILEO (301-336) \n");
    int choice = getUserInputInt("Enter choice: ");
    source.constellation = (GnssSvType)choice;

    std::string svType = "NA";
    switch(choice) {
    case 1:
        svType = "GPS (1-32) (Can't be blacklisted)";
        break;
    case 2:
        svType = "SBAS (Can't be blacklisted)";
        break;
    case 3:
        svType = "GLONASS (65-96)";
        break;
    case 4:
        svType = "QZSS (193-197)";
        break;
    case 5:
        svType = "BEIDOU (201-237)";
        break;
    case 6:
        svType = "GALILEO (301-336)";
        break;
    }

    PRINTLN("Selected %s", svType.c_str());
    choice = getUserInputInt("Enter SV ID to blacklist: ");
    source.svId = choice;

    source.size = sizeof(GnssSvIdSource);
    return source;
}

GnssSvType
GmccUtils::getUserInputGnssSvType(std::string prompt)
{
    PRINT("%s", prompt.c_str());
    GnssSvType svType;

    // Ask for constellation
    PRINTLN("\nSelect a constellation:\n"
            "1: GPS (1-32) (Can't be blacklisted) \n"
            "2: SBAS (Can't be blacklisted) \n"
            "3: GLONASS (65-96) \n"
            "4: QZSS (193-197) \n"
            "5: BEIDOU (201-237) \n"
            "6: GALILEO (301-336) \n");
    int choice = getUserInputInt("Enter choice: ");
    svType = (GnssSvType)choice;

    return svType;
}

GnssSvTypesMask
GmccUtils::getUserInputGnssSvTypesMask(std::string prompt)
{
    PRINTLN("%s", prompt.c_str());
    GnssSvTypesMask mask = 0;

    bool valueAsMask = getUserInputYesNo(
            "Would you like to enter value as mask (y/n): ");
    if (valueAsMask) {

        PRINTLN("Below are values to be used in mask.");
        PRINTLN("GNSS_SV_TYPES_MASK_GLO_BIT  = (1<<0) = 1");
        PRINTLN("GNSS_SV_TYPES_MASK_BDS_BIT  = (1<<1) = 2");
        PRINTLN("GNSS_SV_TYPES_MASK_QZSS_BIT = (1<<2) = 4");
        PRINTLN("GNSS_SV_TYPES_MASK_GAL_BIT  = (1<<3) = 8");

        mask = getUserInputMask64Bit("Enter hex mask value: ");

    } else {

        PRINTLN("Constellation values: gps / glo / gal / bds / qzss");
        PRINTLN("Enter list of required constellations like: \"glo gal bds\"");

        std::string values = getUserInputString("Enter constellation list: ");

        // tokenize values
        std::vector<std::string> constellations = strUtilTokenize(values);
        std::string constellation;
        for (auto itor = constellations.begin(); itor < constellations.end(); ++itor) {
            constellation = *itor;
            if (0 == constellation.compare("glo")) {
                mask |= GNSS_SV_TYPES_MASK_GLO_BIT;
            } else if (0 == constellation.compare("gal")) {
                mask |= GNSS_SV_TYPES_MASK_GAL_BIT;
            } else if (0 == constellation.compare("bds")) {
                mask |= GNSS_SV_TYPES_MASK_BDS_BIT;
            } else if (0 == constellation.compare("qzss")) {
                mask |= GNSS_SV_TYPES_MASK_QZSS_BIT;
            }
        }
    }

    return mask;
}

void
GmccUtils::printGnssConfigBlacklistSvId(const GnssConfig& config)
{
    PRINTLN("\n\nGNSS CONFIG BLACKLISTED SV IDS");
    PRINTLN("config.flags = 0x%x", config.flags);
    PRINTLN("config.blacklistedSvIds.size() = %lu", config.blacklistedSvIds.size());
    std::string gpsBlacklistedSvIds = "GPS: ";
    std::string sbasBlacklistedSvIds = "SBAS: ";
    std::string gloBlacklistedSvIds = "GLO: ";
    std::string qzssBlacklistedSvIds = "QZSS: ";
    std::string bdsBlacklistedSvIds = "BDS: ";
    std::string galBlacklistedSvIds = "GAL: ";
    for (GnssSvIdSource source: config.blacklistedSvIds) {
        switch(source.constellation) {
        case GNSS_SV_TYPE_GPS:
            gpsBlacklistedSvIds.append(std::to_string(source.svId));
            gpsBlacklistedSvIds.append(" ");
            break;
        case GNSS_SV_TYPE_SBAS:
            sbasBlacklistedSvIds.append(std::to_string(source.svId));
            sbasBlacklistedSvIds.append(" ");
            break;
        case GNSS_SV_TYPE_GLONASS:
            gloBlacklistedSvIds.append(std::to_string(source.svId));
            gloBlacklistedSvIds.append(" ");
            break;
        case GNSS_SV_TYPE_QZSS:
            qzssBlacklistedSvIds.append(std::to_string(source.svId));
            qzssBlacklistedSvIds.append(" ");
            break;
        case GNSS_SV_TYPE_BEIDOU:
            bdsBlacklistedSvIds.append(std::to_string(source.svId));
            bdsBlacklistedSvIds.append(" ");
            break;
        case GNSS_SV_TYPE_GALILEO:
            galBlacklistedSvIds.append(std::to_string(source.svId));
            galBlacklistedSvIds.append(" ");
            break;
        default:
            PRINTLN("Invalid constellation %d", source .constellation);
            break;
        }
    }
    PRINTLN("%s", gpsBlacklistedSvIds.c_str());
    PRINTLN("%s", sbasBlacklistedSvIds.c_str());
    PRINTLN("%s", gloBlacklistedSvIds.c_str());
    PRINTLN("%s", qzssBlacklistedSvIds.c_str());
    PRINTLN("%s", bdsBlacklistedSvIds.c_str());
    PRINTLN("%s", galBlacklistedSvIds.c_str());
}

void GmccUtils::convertGnssSvIdMaskToList(
        uint64_t svIdMask, std::vector<GnssSvIdSource>& svIdList,
        GnssSvId initialSvId, GnssSvType svType)
{
    GnssSvIdSource source = {};
    source.size = sizeof(GnssSvIdSource);
    source.constellation = svType;

    // SV ID 0 => All SV IDs in mask
    if (GNSS_SV_CONFIG_ALL_BITS_ENABLED_MASK == svIdMask) {
        source.svId = 0;
        svIdList.push_back(source);
        return;
    }

    // Convert each bit in svIdMask to vector entry
    uint32_t bitNumber = 0;
    while (svIdMask > 0) {
        if (svIdMask & 0x1) {
            source.svId = bitNumber + initialSvId;
            svIdList.push_back(source);
        }
        bitNumber++;
        svIdMask >>= 1;
    }
}

std::vector<std::string>
GmccUtils::strUtilTokenize(const std::string& str)
{
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream stream(str);
    // Tokenize by whitespace
    for (std::string s; stream >> s; ) {
        tokens.push_back(s);
    }
    return tokens;
}

} // namespace garden
