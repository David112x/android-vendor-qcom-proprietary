/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2019-2020 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <loc_pla.h>
#include "log_util.h"
#include "loc_cfg.h"
#include <sys/types.h>
#include <grp.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <sys/capability.h>
#include <loc_misc_utils.h>
#include <fcntl.h>
#include "qapi_location.h"
#include <string.h>
#include <semaphore.h>

#define BUFFERSIZE 50
static sem_t sem_responsecbreceived;
static sem_t sem_locationcbreceived;

#define qapiPrint(fmt, ...) \
    fprintf(stdout, fmt "\n", ##__VA_ARGS__); \
    LOC_LOGV(fmt, ##__VA_ARGS__)

using namespace std;

static uint32_t clientIdArr;

static qapi_Location_Callbacks_t sLocationCallbacks;

static void location_capabilities_callback(
    qapi_Location_Capabilities_Mask_t capabilitiesMask)
{
    qapiPrint("LOC API CALLBACK : CAPABILITIES (mask 0x%x)", capabilitiesMask);
}

static void location_response_callback(
    qapi_Location_Error_t err, uint32_t id)
{
    qapiPrint("LOC API CALLBACK : RESPONSE (err=%u id=%u)", err, id);
    sem_post(&sem_responsecbreceived);
}

static void location_collective_response_callback(size_t count,
    qapi_Location_Error_t* err,
    uint32_t* ids)
{
    uint32_t i = 0;
    qapiPrint("LOC API CALLBACK : COLLECTIVE RESPONSE (count %d)", count);

    for (i = 0; i < count; ++i) {
        qapiPrint("ID[%d] = %d, err[%d] = %d", i, ids[i], i, err[i]);
    }
}

static void location_tracking_callback(qapi_Location_t location)
{
    uint64_t ts_sec = location.timestamp / 1000;

    qapiPrint("LOC API CALLBACK : TRACKING");

    if (location.flags & QAPI_LOCATION_IS_BEST_AVAIL_POS_BIT) {
        qapiPrint("LOC API CALLBACK : BEST AVAILABLE POSITION");
    }
    qapiPrint("Time(HH:MM:SS): %02d:%02d:%02d",
        (int)((ts_sec / 3600) % 24), (int)((ts_sec / 60) % 60), (int)(ts_sec % 60));

    if (location.flags & QAPI_LOCATION_HAS_LAT_LONG_BIT) {
        qapiPrint("LAT: %d.%d LON: %d.%d",
            (int)location.latitude, (abs((int)(location.latitude * 100000))) % 100000,
            (int)location.longitude, (abs((int)(location.longitude * 100000))) % 100000);
    } else {
        qapiPrint("LAT/LONG Not Available");
    }
    if (location.flags & QAPI_LOCATION_HAS_ACCURACY_BIT) {
        qapiPrint("ACC: %d.%d",
            (int)location.accuracy, (abs((int)(location.accuracy * 100))) % 100);
    } else {
        qapiPrint("Accuracy Not Available");
    }
    if (location.flags & QAPI_LOCATION_HAS_ALTITUDE_BIT) {
        qapiPrint("ALT: %d.%d",
            (int)location.altitude, (abs((int)(location.altitude * 100))) % 100);
    } else {
        qapiPrint("Altitude Not Available");
    }
    if (location.flags & QAPI_LOCATION_HAS_VERTICAL_ACCURACY_BIT) {
        qapiPrint("VERT ACC: %d.%d",
            (int)location.verticalAccuracy, (abs((int)(location.verticalAccuracy * 100))) % 100);
    } else {
        qapiPrint("Vertical Accuracy Not Available");
    }
    if (location.flags & QAPI_LOCATION_HAS_ALTITUDE_MSL_BIT) {
        qapiPrint("ALT_MSL: %d.%d",
            (int)location.altitudeMeanSeaLevel, (abs((int)(location.altitudeMeanSeaLevel * 100))) % 100);
    } else {
        qapiPrint("Altitude MSL Not Available");
    }
    if (location.flags & QAPI_LOCATION_HAS_SPEED_BIT) {
        qapiPrint("SPEED: %d.%d",
            (int)location.speed, (abs((int)(location.speed * 100))) % 100);
    } else {
        qapiPrint("Speed Not Available");
    }
    if (location.flags & QAPI_LOCATION_HAS_SPEED_ACCURACY_BIT) {
        qapiPrint("SPEED ACC: %d.%d",
            (int)location.speedAccuracy, (abs((int)(location.speedAccuracy * 100))) % 100);
    } else {
        qapiPrint("Speed Accuracy Not Available");
    }
    if (location.flags & QAPI_LOCATION_HAS_BEARING_BIT) {
        qapiPrint("BEARING: %d.%d",
            (int)location.bearing, (abs((int)(location.bearing * 100))) % 100);
    } else {
        qapiPrint("Bearing Not Available");
    }
    if (location.flags & QAPI_LOCATION_HAS_BEARING_ACCURACY_BIT) {
        qapiPrint("BEARING ACC: %d.%d",
            (int)location.bearingAccuracy, (abs((int)(location.bearingAccuracy * 100))) % 100);
    } else {
        qapiPrint("Bearing Accuracy Not Available");
    }
    sem_post(&sem_locationcbreceived);
}

static void location_single_shot_callback(qapi_Location_t location,
                                          qapi_Location_Error_t err)
{
    uint64_t ts_sec = location.timestamp / 1000;

    qapiPrint("LOC API CALLBACK : SINGLE SHOT");

    if (QAPI_LOCATION_ERROR_TIMEOUT == err) {
        qapiPrint("Timeout occurred, location is invalid");
    } else {
        location_tracking_callback(location);
    }
}

static int loc_init()
{
    qapi_Location_Error_t ret;

    memset(&sLocationCallbacks, 0, sizeof(qapi_Location_Callbacks_t));
    sLocationCallbacks.size = sizeof(qapi_Location_Callbacks_t);

    sem_init(&sem_responsecbreceived, 0, 0);
    sem_init(&sem_locationcbreceived, 0, 0);

    sLocationCallbacks.capabilitiesCb = location_capabilities_callback;
    sLocationCallbacks.responseCb = location_response_callback;
    sLocationCallbacks.collectiveResponseCb = location_collective_response_callback;
    sLocationCallbacks.trackingCb = location_tracking_callback;
    sLocationCallbacks.singleShotCb = location_single_shot_callback;

    qapiPrint("Invoking qapi_Loc_Init()");

    ret = (qapi_Location_Error_t)qapi_Loc_Init(
                &clientIdArr,
                &sLocationCallbacks);

    if (QAPI_LOCATION_ERROR_SUCCESS != ret) {
        qapiPrint("LOC INIT returned failure ! (ret %d)", ret);
        return -1;
    }
    return 0;
}

static int loc_deinit()
{
    qapi_Location_Error_t ret;

    qapiPrint("Invoking qapi_Loc_Deinit()");
    ret = (qapi_Location_Error_t)qapi_Loc_Deinit(clientIdArr);
    if (QAPI_LOCATION_ERROR_SUCCESS != ret) {
        qapiPrint("Returned failure ! (ret %d)", ret);
        return -1;
    }
    qapiPrint("Returned success");
    sem_destroy(&sem_responsecbreceived);
    sem_destroy(&sem_locationcbreceived);
    return 0;
}

static int start_tracking(uint32_t minInterval, uint32_t minDistance)
{
    uint32_t id = (uint32_t)-1;
    qapi_Location_Options_t locationOptions = { 0 };
    qapi_Location_Error_t ret;

    qapiPrint("LOCATION START TRACKING");
    qapiPrint("Interval %u, Distance %u", minInterval, minDistance);

    locationOptions.size = sizeof(qapi_Location_Options_t);
    locationOptions.minInterval = minInterval;
    locationOptions.minDistance = minDistance;

    qapiPrint("Invoking qapi_Loc_Start_Tracking()");
    ret = (qapi_Location_Error_t)qapi_Loc_Start_Tracking(
            clientIdArr, &locationOptions, &id);

    if (QAPI_LOCATION_ERROR_SUCCESS != ret) {
        qapiPrint("Returned failure ! (ret %d)", ret);
        return -1;
    }
    qapiPrint("Returned success, id %d", id);

    qapiPrint("Waiting on Callback...");
    sem_wait(&sem_responsecbreceived);

    qapiPrint("REQUEST SUCCESS");
    return id;
}

static int stop_tracking()
{
    qapi_Location_Error_t ret;

    qapiPrint("LOCATION STOP TRACKING");

    qapiPrint("Invoking qapi_Loc_Stop_Tracking()");
    ret = (qapi_Location_Error_t)qapi_Loc_Stop_Tracking(clientIdArr, 0);

    if (QAPI_LOCATION_ERROR_SUCCESS != ret) {
        qapiPrint("Returned failure ! (ret %d)", ret);
        return -1;
    }
    qapiPrint("Returned success");

    qapiPrint("Waiting on Callback...");
    sem_wait(&sem_responsecbreceived);

    qapiPrint("REQUEST SUCCESS");
    return 0;
}

static int get_singleshot()
{
    uint32_t id = (uint32_t)-1;
    qapi_Location_Options_t locationOptions = { 0 };
    qapi_Location_Error_t ret;

    qapiPrint("LOCATION GET SINGLE SHOT");

    qapiPrint("Invoking qapi_Loc_Get_Single_Shot()");
    ret = (qapi_Location_Error_t)qapi_Loc_Get_Single_Shot(
        clientIdArr, QAPI_LOCATION_POWER_HIGH, &id);

    if (QAPI_LOCATION_ERROR_SUCCESS != ret) {
        qapiPrint("Returned failure ! (ret %d)", ret);
        return -1;
    }
    qapiPrint("Returned success, id %d", id);

    qapiPrint("Waiting on Callback...");
    sem_wait(&sem_responsecbreceived);

    qapiPrint("REQUEST SUCCESS");
    return id;
}

static int cancel_singleshot()
{
    qapi_Location_Error_t ret;

    qapiPrint("LOCATION CANCEL SINGLE SHOT");

    qapiPrint("Invoking qapi_Loc_Cancel_Single_Shot()");
    ret = (qapi_Location_Error_t)qapi_Loc_Cancel_Single_Shot(clientIdArr, 0);

    if (QAPI_LOCATION_ERROR_SUCCESS != ret) {
        qapiPrint("Returned failure ! (ret %d)", ret);
        return -1;
    }
    qapiPrint("Returned success");

    qapiPrint("Waiting on Callback...");
    sem_wait(&sem_responsecbreceived);

    qapiPrint("REQUEST SUCCESS");
    return 0;
}

static int get_best_available_position()
{
    uint32_t id = (uint32_t)-1;
    qapi_Location_Error_t ret;

    qapiPrint("LOCATION GET BEST AVAILABLE POSITION");

    qapiPrint("Invoking qapi_Loc_Get_Best_Available_Position()");
    ret = (qapi_Location_Error_t)qapi_Loc_Get_Best_Available_Position(clientIdArr, &id);

    if (QAPI_LOCATION_ERROR_SUCCESS != ret) {
        qapiPrint("Returned failure ! (ret %d)", ret);
        return -1;
    }
    qapiPrint("Returned success");

//    qapiPrint("Waiting on Callback...");
//    sem_wait(&sem_responsecbreceived);

    qapiPrint("REQUEST SUCCESS");
    return 0;
}

bool checkSepolicyEnforceMode()
{
    char buf[5];
    bool sepolicy_enforced = false;
    int fd = open("/sys/fs/selinux/enforce", O_RDONLY);
    if (fd >= 0) {
        // able to open node. read the value
        int rc = read(fd, buf, sizeof(buf));
        if (rc >= 0) {
            if ('1' == buf[0]) {
                sepolicy_enforced = true;
            }
        }
        close(fd);
    }

    qapiPrint("Sepolicy enforce mode: %d", sepolicy_enforced);
    if (sepolicy_enforced) {
        //sepolicy enforced mode, but remoteapi mode is false.
        qapiPrint("Sepolicy is in enforced mode !!");
        qapiPrint("So qapi demo app can successfully be run only in two ways");
        qapiPrint("    1. Auto test using -S option");
        qapiPrint("    2. Menu test using option 8");
    }
    return sepolicy_enforced;
}

bool setRequiredPermToRunAsLocClient()
{
    bool setuser_status = false;
    if (0 == getuid()) {
        if (-1 == setgid(GID_LOCCLIENT)) {
            qapiPrint("%s:%d]: Error: setgid GID_LOCCLIENT failed. %s",
                __func__, __LINE__, strerror(errno));
        }
        if (-1 == setuid(UID_LOCCLIENT)) {
            qapiPrint("%s:%d]: Error: setuid UID_LOCCLIENT failed. %s",
                __func__, __LINE__, strerror(errno));
        }
        else {
            setuser_status = true;
        }
    }
    else {
        qapiPrint("Garden app started as user: %d", getuid());
        if (UID_LOCCLIENT != getuid()) {
            qapiPrint("%s:%d]: Error: qapi_demo_app not started as loc_client user",
                __func__, __LINE__);
        }
        else {
            setuser_status = true;
        }
    }
    // returns true if qapi demo app is running as loc_client user.
    return setuser_status;
}

int main(int argc, char *argv[]) {
    char buf[16], *p;
    bool exit_loop = false;
    char str[BUFFERSIZE];
    char* pch;
    uint32_t intervalInMs = 0;
    uint32_t distanceInMeters = 0;

    checkSepolicyEnforceMode();

    if (!setRequiredPermToRunAsLocClient()) {
        // Quit if unable to start as gps user
        return -1;
    }

    while (!exit_loop) {
        qapiPrint("\n\n"
            "1: loc init\n"
            "2: loc deinit\n"
            "3: start tracking\n"
            "4: stop tracking\n"
            "5: get singleshot\n"
            "6: cancel singleshot\n"
            "7: get best available position\n"
            "q: quit\n"
            "\nEnter Command:");
        fflush(stdout);
        p = fgets(buf, 16, stdin);
        if (nullptr == p) {
            qapiPrint("Error: fgets returned nullptr !!");
        }

        switch (p[0]) {
        case '1':
            if (0 == loc_init()) {
                qapiPrint("Location Init Success");
            } else {
                qapiPrint("Location Init Failure!");
            }
            break;
        case '2':
            if (0 == loc_deinit()) {
                qapiPrint("Location Deinit Success");
            } else {
                qapiPrint("Location Deinit Failure!");
            }
            break;
        case '3':
            qapiPrint("Enter rate in milliseconds (>=1000):");
            pch = fgets(str, BUFFERSIZE, stdin);
            intervalInMs = std::stoul(pch, nullptr, 10);

            qapiPrint("Enter distance in meters:");
            pch = fgets(str, BUFFERSIZE, stdin);

            distanceInMeters = std::stoul(pch, nullptr, 10);

            if (1 == start_tracking(intervalInMs, distanceInMeters)) {
                qapiPrint("Start Tracking Success");
            } else {
                qapiPrint("Start Tracking Failure!");
            }
            break;
        case '4':
            if (0 == stop_tracking()) {
                qapiPrint("Stop Tracking Success");
            } else {
                qapiPrint("Stop Tracking Failure!");
            }
            break;
        case '5':
            if (1 == get_singleshot()) {
                qapiPrint("Get Singleshot Success");
            } else {
                qapiPrint("Get Singleshot Failure!");
            }
            break;
        case '6':
            if (0 == cancel_singleshot()) {
                qapiPrint("Cancel Singleshot Success");
            } else {
                qapiPrint("Cancel Singleshot Failure!");
            }
            break;
        case '7':
            get_best_available_position();
            break;
        case 'q':
            exit_loop = true;
            break;
        default:
            qapiPrint("\ninvalid command");
        }
    }

    return 0;
}
