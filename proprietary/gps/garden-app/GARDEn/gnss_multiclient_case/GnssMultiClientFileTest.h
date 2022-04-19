/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/
#ifndef GNSS_MULTI_CLIENT_FILE_TEST_H
#define GNSS_MULTI_CLIENT_FILE_TEST_H

#include "IGnssAPI.h"
#include "IGardenCase.h"

namespace garden {

/* GARDEn file based command validation */
class GnssMultiClientFileTest {

private:
    GnssMultiClientCase mGmcc;

private:
    static GnssMultiClientFileTest* sInstance;
    GnssMultiClientFileTest(): mGmcc() {};

public:
    static GnssMultiClientFileTest& get() {
        if (nullptr == sInstance) {
            sInstance = new GnssMultiClientFileTest();
        }
        return *sInstance;
    }

public:
    GARDEN_RESULT run();

private:
    GARDEN_RESULT processCommand(std::string command);

    GARDEN_RESULT cmdHelp(const std::vector<std::string>& cmdTokens);
    GARDEN_RESULT cmdSetBlacklist(const std::vector<std::string>& cmdTokens);
    GARDEN_RESULT cmdGetBlacklist(const std::vector<std::string>& cmdTokens);
    GARDEN_RESULT cmdSetConstellation(const std::vector<std::string>& cmdTokens);
    GARDEN_RESULT cmdGetConstellation(const std::vector<std::string>& cmdTokens);
    GARDEN_RESULT cmdResetConstellation(const std::vector<std::string>& cmdTokens);

    GARDEN_RESULT cmdSleep(const std::vector<std::string>& cmdTokens);
    GARDEN_RESULT cmdRepeat(const std::vector<std::string>& cmdTokens);
};

} //namespace garden

#endif //GNSS_MULTI_CLIENT_FILE_TEST_H
