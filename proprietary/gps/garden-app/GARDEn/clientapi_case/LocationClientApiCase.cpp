/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2018-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sstream>
#include <semaphore.h>
#include "GardenFramework.h"
#include "GardenUtil.h"
#include "LocationClientApiCase.h"

namespace garden {

#define BUFFERSIZE 50
#define CLIENTAPI_TEST_DELETEALL "deleteAll"
#define CLIENTAPI_TEST_GETENERGY "getEnergy"
#define CLIENTAPI_TEST_REG_SYSTEMINFO "regSysInfo"
#define CLIENTAPI_TEST_UNREG_SYSTEMINFO "unregSysInfo"
#define CLIENTAPI_TEST_DISABLE_OUTPUT   "disableOutput"
#define CLIENTAPI_TEST_ENABLE_OUTPUT    "enableOutput"

#define LAT 32.896535
#define LONG -117.201025
#define ACCU 50
#define NUM_LOOP_PINGTEST (1000)

enum ActionType {
    TEST_NO_ACTION = 0,
    TEST_GNSS_REPORT,
    TEST_LOCATION_REPORT,
    TEST_NETWORK_UP,
    TEST_NETWORK_DOWN,
    TEST_SYSTEMINFO_REG, // register for location system info
    TEST_SYSTEMINFO_UNREG, // unregister for location system info
    TEST_ROUTINE_BATCHING,
    TEST_TRIP_BATCHING,
    TEST_STOP,
    TEST_STOP_BATCHING,
    TEST_GEOFENCE,
    TEST_PING,
    TEST_ENGINE_REPORT,
};

static sem_t sem_locationcbreceived;
static sem_t sem_pingcbreceived;

// debug utility
static uint64_t getTimestamp() {
    struct timespec ts;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    uint64_t absolute_micro =
        ((uint64_t)(ts.tv_sec)) * 1000000ULL + ((uint64_t)(ts.tv_nsec)) / 1000ULL;
    return absolute_micro;
}

/******************************************************************************
Callback functions
******************************************************************************/
static void onLocationReceivedCb(void) {
    gardenPrint("<<< onLocationReceivedCb");
    sem_post(&sem_locationcbreceived);
}

static void onPingReceivedCb(void) {
    sem_post(&sem_pingcbreceived);
}

ClientReportsCbs clientReportsCbs;

/******************************************************************************
Constructors
******************************************************************************/
LocationClientApiCase::LocationClientApiCase() :
    mCb(nullptr),
    mPLocationClient_1(nullptr),
    mPLocationClient_2(nullptr) {
}

LocationClientApiCase::~LocationClientApiCase() {
    if (mPLocationClient_1) {
        delete mPLocationClient_1;
    }
    if (mPLocationClient_2) {
        delete mPLocationClient_2;
    }
}

GARDEN_RESULT LocationClientApiCase::preRun() {
    // this function will be called before every case.
    gardenPrint("preRun");
    if (!mCb) {
        mCb = new GnssCbBase();
    }
    return GARDEN_RESULT_PASSED;
}

GARDEN_RESULT LocationClientApiCase::postRun() {
    // this function will be called after every case.
    gardenPrint("postRun");
    if (mCb) {
        delete mCb;
        mCb = nullptr;
    }
    return GARDEN_RESULT_PASSED;
}

static void printHelp() {
    menuPrint("\n\n"
        "location_client_api_testapp\n"
        "Test command: client#,actionType(g/l),interval,distance(optional,with l option)\n"
        "Start client1 with Gnss Session type, interval 1000 ms, distance 0 meters: 1 g 1000 \n"
        "g: Gnss report session with all 5 types of callbacks\n"
        "g1: Gnss report session with LocationInfo callback\n"
        "g3: Gnss report session with LocationInfo and SV callbacks\n"
        "g5: Gnss report session with LocationInfo and NMEA callbacks\n"
        "g9: Gnss report session with LocationInfo and Data callbacks\n"
        "g17: Gnss report session with LocationInfo and Measurements callbacks\n"
        "g33: Gnss report session with LocationInfo and location system info callbacks\n"
        "b: Routine batching\n"
        "t: Outdoor trip batching\n"
        "l: Location session \n"
        "f: Geofence session\n"
        "s: Stop a session \n"
        "k: Stop a batching session \n"
        "Test command format: client#1 e,interval, engine mask\n"
        "engine mask, 1-fused, 2-SPE, 4-PPE, 3-fused+SPE, 5-fused+PPE, 7-fused+SPE+PPE\n"
        "e: Engine report session with all 5 types of callbacks\n"
        "e1: Engine report session with LocationInfo callback\n"
        "e3: Engine report session with LocationInfo and SV callbacks\n"
        "e5: Engine report session with LocationInfo and NMEA callbacks\n"
        "e9: Engine report session with LocationInfo and Data callbacks\n"
        "e17: Engine report session with LocationInfo and Measurements callbacks\n"
        "e33: Engine report session with LocationInfo and location system info callbacks\n"
        "e65: Engine report session with LocationInfo and SV poly callbacks\n"
        "u: Set network available \n"
        "d: Set network not available \n"
        "m: register for location system info \n"
        "n: unregister for location system info \n"
        "deleteAll: gnssDeleteAidingData \n"
        "getEnergy: get energy consumed by GNSS engine since device first boot \n"
        "regSysInfo: register to get system info update \n"
        "unregSysInfo: unregister to get system info update \n"
        "p: ping test \n"
        "q: Quit\n"
        "disableOutput: disable gnss+engine reports\n"
        "enableOutput: enable gnss+engine reports\n"
        "\nEnter Command:");
        fflush(stdout);
}

void LocationClientApiCase::testDeleteAll(::PositioningEngineMask posEngineMask) {
    mPLocationClient_1->setEngineMask(posEngineMask);
    mPLocationClient_1->gnssDeleteAidingData(LOC_GPS_DELETE_ALL);
}

void LocationClientApiCase::testGetEnergy() {
   gardenPrint("testGetEnergy enter\n");
   mPLocationClient_1->testGetEnergy();
}

void LocationClientApiCase::testRegSystemInfo() {
    gardenPrint("testRegSystemInfo\n");
    mPLocationClient_1->testRegSystemInfo();
}

void LocationClientApiCase::testUnregSystemInfo() {
    gardenPrint("testUnRegSystemInfo\n");
    mPLocationClient_1->testUnregSystemInfo();
}

void LocationClientApiCase::menuAddGeofence(int clientId) {
    int geofenceNum = 0;
    double latitude = LAT;
    double longitude = LONG;
    double radius = ACCU;
    ::GeofenceBreachTypeMask type = (::GeofenceBreachTypeMask)
            (::GEOFENCE_BREACH_ENTER_BIT |
            ::GEOFENCE_BREACH_EXIT_BIT);
    uint32_t responsiveness = 4000;
    uint32_t time = 0;
    char buf[16], *p;
    menuPrint ("\nEnter number of geofences:", geofenceNum);
    fflush (stdout);
    p = fgets (buf, 16, stdin);
    if (p == nullptr) {
        menuPrint("Error: fgets returned nullptr !!");
        return;
    }
    if (atoi(p) != 0) {
        geofenceNum = atoi(p);
    }
    for (int i=0; i<geofenceNum; ++i) {
        menuPrint ("\nEnter latitude (default %f):", latitude);
        fflush (stdout);
        p = fgets (buf, 16, stdin);
        if (p == nullptr) {
            menuPrint("Error: fgets returned nullptr !!");
            return;
        }
        if (atof(p) != 0) {
            latitude = atof(p);
        }
        menuPrint ("\nEnter longitude (%f):", longitude);
        fflush (stdout);
        p = fgets (buf, 16, stdin);
        if (p == nullptr) {
            menuPrint("Error: fgets returned nullptr !!");
            return;
        }
        if (atof(p) != 0) {
            longitude = atof(p);
        }
        menuPrint ("\nEnter radius (default %f):", radius);
        fflush (stdout);
        p = fgets (buf, 16, stdin);
        if (p == nullptr) {
            menuPrint("Error: fgets returned nullptr !!");
            return;
        }
        if (atof(p) != 0) {
            radius = atof(p);
        }
        menuPrint ("\nEnter breachType (default %u):", type);
        fflush (stdout);
        p = fgets (buf, 16, stdin);
        if (p == nullptr) {
            menuPrint("Error: fgets returned nullptr !!");
            return;
        }
        if (atoi(p) != 0) {
            type = static_cast<::GeofenceBreachTypeMask>(atoi(p));
        }
        menuPrint ("\nEnter responsiveness in seconds: (default %u):",
                responsiveness / 1000);
        fflush (stdout);
        p = fgets (buf, 16, stdin);
        if (p == nullptr) {
            menuPrint("Error: fgets returned nullptr !!");
            return;
        }
        if (atoi(p) != 0) {
            responsiveness = atoi(p) * 1000;
        }
        menuPrint ("\nEnter dwell time (default %u):", time);
        fflush (stdout);
        p = fgets (buf, 16, stdin);
        if (p == nullptr) {
            menuPrint("Error: fgets returned nullptr !!");
            return;
        }
        if (atof(p) != 0) {
            time = atof(p);
        }
        if (1 == clientId) {
            mPLocationClient_1->addGeofence(i, latitude, longitude,
                    radius, type, responsiveness, time);
        } else {
            mPLocationClient_2->addGeofence(i, latitude, longitude,
                    radius, type, responsiveness, time);
        }
    }
}

void LocationClientApiCase::menuModifyGeofence(int clientId) {
    int32_t geofenceNum = 0;
    ::GeofenceBreachTypeMask type = (::GeofenceBreachTypeMask)
            (::GEOFENCE_BREACH_ENTER_BIT |
            ::GEOFENCE_BREACH_EXIT_BIT);
    uint32_t responsiveness = 4000;
    uint32_t time = 0;
    int32_t geofenceId = 0;
    char buf[16], *p;
    menuPrint ("\nEnter number of geofences:", geofenceNum);
    fflush (stdout);
    p = fgets (buf, 16, stdin);
    if (p == nullptr) {
        menuPrint("Error: fgets returned nullptr !!");
        return;
    }
    if (atoi(p) != 0) {
        geofenceNum = atoi(p);
    }
    for (int i=0; i<geofenceNum; ++i) {
        menuPrint ("\nEnter id ");
        fflush (stdout);
        p = fgets (buf, 16, stdin);
        if (p == nullptr) {
            menuPrint("Error: fgets returned nullptr !!");
            return;
        }
        if (atoi(p) != 0) {
            geofenceId = atoi(p);
        }
        menuPrint ("\nEnter breachType (default %u):", type);
        fflush (stdout);
        p = fgets (buf, 16, stdin);
        if (p == nullptr) {
            menuPrint("Error: fgets returned nullptr !!");
            return;
        }
        if (atoi(p) != 0) {
            type = static_cast<GeofenceBreachTypeMask>(atoi(p));
        }
        menuPrint ("\nEnter responsiveness in seconds: (default %u):",
                responsiveness / 1000);
        fflush (stdout);
        p = fgets (buf, 16, stdin);
        if (p == nullptr) {
            menuPrint("Error: fgets returned nullptr !!");
            return;
        }
        if (atoi(p) != 0) {
            responsiveness = atoi(p) * 1000;
        }
        menuPrint ("\nEnter dwell time (default %u):", time);
        fflush (stdout);
        p = fgets (buf, 16, stdin);
        if (p == nullptr) {
            menuPrint("Error: fgets returned nullptr !!");
            return;
        }
        if (atof(p) != 0) {
            time = atof(p);
        }
        if (geofenceId == 0) {
            return;
        }
        if (1 == clientId) {
            mPLocationClient_1->modifyGeofence(geofenceId,
                    type, responsiveness, time);
        } else {
            mPLocationClient_2->modifyGeofence(geofenceId,
                type, responsiveness, time);
        }
    }
}

void LocationClientApiCase::menuPauseGeofence(int clientId) {
    int32_t geofenceNum = 0;
    int32_t geofenceId = 0;
    char buf[16], *p;
    menuPrint ("\nEnter number of geofences:", geofenceNum);
    fflush (stdout);
    p = fgets (buf, 16, stdin);
    if (p == nullptr) {
        menuPrint("Error: fgets returned nullptr !!");
        return;
    }
    if (atoi(p) != 0) {
        geofenceNum = atoi(p);
    }
    for (int i=0; i<geofenceNum; ++i) {
        menuPrint ("\nEnter id ");
        fflush (stdout);
        p = fgets (buf, 16, stdin);
        if (p == nullptr) {
            menuPrint("Error: fgets returned nullptr !!");
            return;
        }
        if (atoi(p) != 0) {
            geofenceId = atoi(p);
        }
        if (1 == clientId) {
            mPLocationClient_1->pauseGeofence(geofenceId);
        } else {
            mPLocationClient_2->pauseGeofence(geofenceId);
        }
    }
}

void LocationClientApiCase::menuResumeGeofence(int clientId) {
    int32_t geofenceNum = 0;
    int32_t geofenceId = 0;
    ::GeofenceBreachTypeMask type = (::GeofenceBreachTypeMask)
            (::GEOFENCE_BREACH_ENTER_BIT |
            ::GEOFENCE_BREACH_EXIT_BIT);
    char buf[16], *p;
    menuPrint ("\nEnter number of geofences:", geofenceNum);
    fflush (stdout);
    p = fgets (buf, 16, stdin);
    if (p == nullptr) {
        menuPrint("Error: fgets returned nullptr !!");
        return;
    }
    if (atoi(p) != 0) {
        geofenceNum = atoi(p);
    }
    for (int i=0; i<geofenceNum; ++i) {
        menuPrint ("\nEnter id ");
        fflush (stdout);
        p = fgets (buf, 16, stdin);
        if (p == nullptr) {
            menuPrint("Error: fgets returned nullptr !!");
            return;
        }
        if (atoi(p) != 0) {
            geofenceId = atoi(p);
        }
        if (1 == clientId) {
            mPLocationClient_1->resumeGeofence(geofenceId, type);
        } else {
            mPLocationClient_2->resumeGeofence(geofenceId, type);
        }
    }
}

void LocationClientApiCase::menuRemoveGeofence(int clientId) {
    int32_t geofenceNum = 0;
    int32_t geofenceId = 0;
    char buf[16], *p;
    menuPrint ("\nEnter number of geofences:", geofenceNum);
    fflush (stdout);
    p = fgets (buf, 16, stdin);
    if (p == nullptr) {
        menuPrint("Error: fgets returned nullptr !!");
        return;
    }
    if (atoi(p) != 0) {
        geofenceNum = atoi(p);
    }
    for (int i=0; i<geofenceNum; ++i) {
        menuPrint ("\nEnter id ");
        fflush (stdout);
        p = fgets (buf, 16, stdin);
        if (p == nullptr) {
            menuPrint("Error: fgets returned nullptr !!");
            return;
        }
        if (atoi(p) != 0) {
            geofenceId = atoi(p);
        }
        if (1 == clientId) {
            mPLocationClient_1->removeGeofence(geofenceId);
        } else {
            mPLocationClient_2->removeGeofence(geofenceId);
        }
    }
}

GARDEN_RESULT LocationClientApiCase::testGeofence(int clientId) {
    char buf[16], *p;
    bool exit_loop = false;

    while(!exit_loop)
    {
        menuPrint ("\n\n"
            "1: add_geofence\n"
            "2: pause_geofence\n"
            "3: resume geofence\n"
            "4: modify geofence\n"
            "5: remove geofence\n"
            "b: back\n"
            "q: quit\n\n"
            "Enter Command:");
        fflush (stdout);
        p = fgets (buf, 16, stdin);
        if (p == nullptr) {
            GARDEN_ERROR("Error: fgets returned nullptr !!");
        }

        switch (p[0]) {
        case '1':
            menuAddGeofence(clientId);
            break;
        case '2':
            menuPauseGeofence(clientId);
            break;
        case '3':
            menuResumeGeofence(clientId);
            break;
        case '4':
            menuModifyGeofence(clientId);
            break;
        case '5':
            menuRemoveGeofence(clientId);
            break;
        case 'b':
            exit_loop = true;
            break;
        case 'q':
            return GARDEN_RESULT_ABORT;
        default:
            menuPrint("\ninvalid command\n");
        }
    }
    return GARDEN_RESULT_PASSED;
}

void LocationClientApiCase::testPing() {
    gardenPrint("ping started ");
    uint64_t t0 = 0ULL;
    uint64_t t1 = 0ULL;
    uint64_t tall = 0ULL;
    sem_init(&sem_pingcbreceived, 0, 0);
    for (int i = 0; i<NUM_LOOP_PINGTEST; i++) {
        t0 = getTimestamp();
        mPLocationClient_1->pingTest();
        sem_wait(&sem_pingcbreceived);
        t1 = getTimestamp();
        tall += (t1 - t0);
        if (i % (NUM_LOOP_PINGTEST / 10) == 0) {
            gardenPrint(".");
        }
        usleep(5000);
    }
    gardenPrint("done average=%llu usec", tall / NUM_LOOP_PINGTEST);
}

GARDEN_RESULT LocationClientApiCase::menuCase() {
    clientReportsCbs.locationReceivedCallback = onLocationReceivedCb;
    clientReportsCbs.pingReceivedCallback = onPingReceivedCb;
    if (!mPLocationClient_1) {
        mPLocationClient_1 = new GnssLocationClientAPI();
        mPLocationClient_1->setAutoModeOptions(false, false);
        mPLocationClient_1->setClientReportsCbs(&clientReportsCbs);
    }

    bool exit_loop = false;
    while (!exit_loop) {

        printHelp();
        char str[BUFFERSIZE];
        char* pch = fgets(str, BUFFERSIZE, stdin);
        char* argPtr;
        if (nullptr == pch) {
            GARDEN_ERROR("Error: fgets returned nullptr !!");
        }

        /* remove newline, if present */
        int len = strlen(str) - 1;
        if( str[len] == '\n') {
            str[len] = '\0';
        }

        if (str[0] != '1' && str[0] != '2') {
            if (str[0] == 'Q' || str[0] == 'q') {
                exit_loop = true;
                break;
            } else if (!strcmp(str, CLIENTAPI_TEST_DELETEALL)) {
                ::PositioningEngineMask posEngineMask =
                    ::STANDARD_POSITIONING_ENGINE |
                    ::PRECISE_POSITIONING_ENGINE |
                    ::DEAD_RECKONING_ENGINE;
                testDeleteAll(posEngineMask);
                continue;
            } else if (!strcmp(str, CLIENTAPI_TEST_GETENERGY)) {
                testGetEnergy();
                continue;
            } else if (!strcmp(str, CLIENTAPI_TEST_REG_SYSTEMINFO)) {
                testRegSystemInfo();
                continue;
            } else if (!strcmp(str, CLIENTAPI_TEST_UNREG_SYSTEMINFO)) {
                testUnregSystemInfo();
                continue;
            } else if (!strcmp(str, CLIENTAPI_TEST_DISABLE_OUTPUT)) {
                mPLocationClient_1->setCbOutput(false);
                continue;
            } else if (!strcmp(str, CLIENTAPI_TEST_ENABLE_OUTPUT)) {
                mPLocationClient_1->setCbOutput(true);
                continue;
            } else {
                printHelp();
                continue;
            }
        }

        pch = strtok_r(str, " ", &argPtr);
        uint32_t clientId = 0;
        uint32_t intervalInMs = 0;
        uint32_t distanceInMeters = 0;
        ::LocReqEngineTypeMask locReqEngMask = ::LOC_REQ_ENGINE_FUSED_BIT;
        ActionType type = TEST_NO_ACTION;
        int gnssReportOptions = LOCATION_BIT | GNSS_SV_BIT | GNSS_NMEA_BIT |
                                GNSS_DATA_BIT | GNSS_MEAS_BIT | GNSS_POLY_BIT;
        GnssReportCbs reportcbs;
        EngineReportCbs engReportcbs;

        int i = 0;
        bool parseError = false;
        bool retVal = false;

        while (nullptr != pch) {
            switch (i) {
                case 0:
                    clientId = std::stoul(pch, nullptr, 10);
                    if ((2 == clientId) && (!mPLocationClient_2)) {
                        mPLocationClient_2 = new GnssLocationClientAPI();
                        mPLocationClient_1->setAutoModeOptions(false, false);
                        mPLocationClient_2->setClientReportsCbs(&clientReportsCbs);
                    }
                    break;
                case 1: {
                    if (*pch == 'G' || *pch == 'g' || *pch == 'E' || *pch == 'e') {
                        if (*pch == 'G' || *pch == 'g') {
                            type = TEST_GNSS_REPORT;
                        } else{
                            type = TEST_ENGINE_REPORT;
                        }
                        if (strlen(pch) > 1) {
                            pch++;
                            gnssReportOptions = std::stoul(pch,nullptr,10);
                        }

                        if (gnssReportOptions < LOCATION_BIT) {
                            gardenPrint("Invalid gnss report option, set to default\n");
                            gnssReportOptions = LOCATION_BIT | GNSS_SV_BIT |
                                                GNSS_NMEA_BIT | GNSS_DATA_BIT | GNSS_MEAS_BIT;
                        }
                    } else if (*pch == 'B' || *pch == 'b') {
                        type = TEST_ROUTINE_BATCHING;
                    } else if (*pch == 'T' || *pch == 't') {
                        type = TEST_TRIP_BATCHING;
                    } else if (*pch == 'F' || *pch == 'f') {
                        type = TEST_GEOFENCE;
                    } else if (*pch == 'L' || *pch == 'l') {
                        type = TEST_LOCATION_REPORT;
                    } else if (*pch == 'U' || *pch == 'u') {
                        type = TEST_NETWORK_UP;
                    } else if (*pch == 'D' || *pch == 'd') {
                        type = TEST_NETWORK_DOWN;
                    } else if (*pch == 'S' || *pch == 's') {
                        type = TEST_STOP;
                    } else if (*pch == 'M' || *pch == 'm') {
                        type = TEST_SYSTEMINFO_REG;
                    } else if (*pch == 'N' || *pch == 'n') {
                        type = TEST_SYSTEMINFO_UNREG;
                    } else if (*pch == 'K' || *pch == 'k') {
                        type = TEST_STOP_BATCHING;
                    } else if (*pch == 'P' || *pch == 'p') {
                        type = TEST_PING;
                    } else {
                        gardenPrint("Action type is not recognized %c \n",*pch);
                        parseError = true;
                        pch = nullptr;
                    }
                    break;
                }
                case 2:
                    intervalInMs = std::stoul(pch, nullptr, 10);
                    break;
                case 3:
                    if (type == TEST_LOCATION_REPORT) {
                        distanceInMeters = std::stoul(pch, nullptr, 10);
                    } else if (type == TEST_ENGINE_REPORT) {
                        locReqEngMask = (::LocReqEngineTypeMask)std::stoul(pch, nullptr, 10);
                    }
                    break;
                default:
                    pch = nullptr;
                    break;
            }
            pch = strtok_r(nullptr, " ", &argPtr);
            i++;
        } //while

        if (parseError) {
            printHelp();
            continue;
        }
        GnssLocationClientAPI* mPLocationClient = (1 == clientId? mPLocationClient_1: mPLocationClient_2);
        mPLocationClient->setGnssCbs(mCb);
        LocationOptions options;
        memset(&options, 0, sizeof(LocationOptions));
        options.size = sizeof(LocationOptions);
        options.minInterval = intervalInMs;
        options.minDistance = distanceInMeters;
        switch (type) {
            case TEST_GNSS_REPORT:
                mPLocationClient->gnssStartOptions(GNSS_REPORT,
                        intervalInMs, 0, ::LOC_REQ_ENGINE_FUSED_BIT, gnssReportOptions);
                mPLocationClient->gnssStart();
                if (1 == clientId) {
                    if (gnssReportOptions & LOC_SYS_INFO_BIT) {
                        testRegSystemInfo();
                    }
                }
                break;

            case TEST_ENGINE_REPORT:
                gardenPrint("gnssReportOptions %d ", gnssReportOptions);
                mPLocationClient->gnssStartOptions(ENGINE_REPORT,
                    intervalInMs, 0, locReqEngMask, gnssReportOptions);
                retVal = mPLocationClient->gnssStart();
                if (false == retVal) {
                    gardenPrint("startPositionSession with concurrent report return %d ", retVal);
                }

                if (1 == clientId) {
                    if (gnssReportOptions & LOC_SYS_INFO_BIT) {
                        testRegSystemInfo();
                    }
                }
                break;

            case TEST_LOCATION_REPORT:
                mPLocationClient->gnssStartOptions(LOCATION_REPORT,
                    intervalInMs, distanceInMeters, ::LOC_REQ_ENGINE_FUSED_BIT, 0);
                mPLocationClient->gnssStart();

                break;
            case TEST_NETWORK_UP:
                mPLocationClient->updateNetworkAvailability(true, NULL);
                break;
            case TEST_NETWORK_DOWN:
                mPLocationClient->updateNetworkAvailability(false, NULL);
                break;
            case TEST_ROUTINE_BATCHING:
                mPLocationClient->flpStartSession(SESSION_MODE_ON_FULL, options);
                break;
            case TEST_TRIP_BATCHING:
                mPLocationClient->flpStartSession(SESSION_MODE_ON_TRIP_COMPLETED, options);
                break;
            case TEST_GEOFENCE:
                mPLocationClient->setGeofenceCbs(NULL);
                testGeofence(clientId);
                break;
            case TEST_STOP:
                mPLocationClient->gnssStop();
                break;
            case TEST_SYSTEMINFO_REG:
                if (1 == clientId) {
                    testRegSystemInfo();
                }
                break;
            case TEST_SYSTEMINFO_UNREG:
                if (1 == clientId) {
                    testUnregSystemInfo();
                }
                break;
            case TEST_STOP_BATCHING:
                mPLocationClient->flpStopSession();
                break;
            case TEST_PING:
                if (1 == clientId) {
                    testPing();
                }
                break;
        } //switch(type)
    }//while(!exit_loop)

    return GARDEN_RESULT_PASSED;
}

GARDEN_RESULT LocationClientApiCase::autoStartCases(
        uint32_t interval, uint32_t loop, uint32_t option, int routeToNmeaPort) {

    int gnssReportOptions = LOCATION_BIT | GNSS_SV_BIT | GNSS_NMEA_BIT |
                            GNSS_DATA_BIT | GNSS_MEAS_BIT | GNSS_POLY_BIT;

    gardenPrint("autoStartCases interval=%u loop=%u routeToNmeaPort:%d\n",
            interval, loop, routeToNmeaPort);

    sem_init(&sem_locationcbreceived, 0, 0);

    clientReportsCbs.locationReceivedCallback = onLocationReceivedCb;
    clientReportsCbs.pingReceivedCallback = onPingReceivedCb;
    if (!mPLocationClient_1) {
        mPLocationClient_1 = new GnssLocationClientAPI();
        mPLocationClient_1->setAutoModeOptions(true, routeToNmeaPort);
        mPLocationClient_1->setClientReportsCbs(&clientReportsCbs);
    }

    // start
    mPLocationClient_1->gnssStartOptions(GNSS_REPORT,
            interval, 0, ::LOC_REQ_ENGINE_FUSED_BIT, option);
    mPLocationClient_1->gnssStart();

    // wait for location callbacks requested times
    for (int i=0; i<loop; i++) {
        sem_wait(&sem_locationcbreceived);
    }

    // stop session
    mPLocationClient_1->gnssStop();
    gardenPrint("autoStartCases done\n");

    return GARDEN_RESULT_PASSED;
}

} // namespace garden
