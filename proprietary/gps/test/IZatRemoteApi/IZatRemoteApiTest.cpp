/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2016 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>   /* Standard input/output definitions */
#include <string.h>  /* String function definitions */
#include <fcntl.h>   /* File control definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */
#include <limits.h>

#include <izat_remote_api.h>
#include <gps_extended_c.h>
#include <loc_cfg.h>
#include <iostream>
#include <loc_pla.h>
#include <loc_misc_utils.h>

using namespace std;
int id1 = 1;
int tty_fd;
#define SERIAL_PORT "/dev/ttyHSL0"
#define NMEA_PORT "/dev/ttyGS0"

/* Final Data subscription mask*/
#define FINAL_POS_DATA_SUB_MASK           (0x001)
#define FINAL_SVINFO_DATA_SUB_MASK        (0x002)
#define FINAL_NMEA_DATA_SUB_MASK          (0x004)
#define FINAL_DATA_SUB_MASK_ALL           (FINAL_POS_DATA_SUB_MASK | \
                                           FINAL_SVINFO_DATA_SUB_MASK | \
                                           FINAL_NEMA_DATA_SUB_MASK)


/** Structure that holds the command line options given to main **/
typedef struct command_line_options {
    int t;                        // timeout in seconds
    int w;                        // wait after time out before exit in seconds
    int finalSubMask;             // Final data subscription mask.
    int printLogs;                // Print nmea string
} CommandLineOptionsType;

// Default values
static CommandLineOptionsType sOptions = {
    100,                         // timeout in seconds.
    2,                           // wait after time out before exit in seconds.
    0,                           // Final data subscription mask
    1                            // Print logs to sonsole
};

void setRequiredPerm()
{
    if (0 == getuid()) {
        // set tty groups to ablet o access nmea port
        char grps[LOC_MAX_PARAM_NAME] = "tty";
        gid_t grpIds[LOC_PROCESS_MAX_NUM_GROUPS] = {};
        char *splitGrpString[LOC_PROCESS_MAX_NUM_GROUPS];
        int i = 0, numGrpIds = 0;

        int numGrps = loc_util_split_string(grps, splitGrpString,
                LOC_PROCESS_MAX_NUM_GROUPS, ' ');
        for(i=0; i<numGrps; i++) {
            struct group* grp = getgrnam(splitGrpString[i]);
            if (grp) {
                grpIds[numGrpIds] = grp->gr_gid;
                printf("Group %s = %d\n", splitGrpString[i], grpIds[numGrpIds]);
                numGrpIds++;
            }
        }
        if (0 != numGrpIds) {
            i = setgroups(numGrpIds, grpIds);
            if(i == -1) {
                printf("Could not set groups. errno:%d, %s\n", errno, strerror(errno));
            } else {
                printf("Total of %d groups set for app\n", numGrpIds);
            }
        }

        // run as locclient for communication with hal daemon
        if(-1 == setgid(GID_LOCCLIENT)) {
            printf("%s:%d]: Error: setgid GID_LOCCLIENT failed. %s\n",
                     __func__, __LINE__, strerror(errno));
        }
        if(-1 == setuid(UID_LOCCLIENT)) {
            printf("%s:%d]: Error: setuid UID_LOCCLIENT failed. %s\n",
                     __func__, __LINE__, strerror(errno));
        }
    } else {
        printf("Test app started as user: %d\n", getuid());
    }
}

/*
* 'open_port()' - Open NMEA port.
*/
int open_port(void)
{
    int n; /* File descriptor for the port */

    printf("open_port \n");
    tty_fd = open(NMEA_PORT, O_RDWR | O_NOCTTY | O_NDELAY);
    if (tty_fd == -1)
    {
        /* Could not open the port. */
        printf("open_port: Unable to open %s \n", NMEA_PORT);
    }
    printf ("open_port success \n");
}

static void print_and_sleep(int seconds) {
    printf("sleep for %d seconds... \n", seconds);
    sleep(seconds);
}

// SSTP Test
static void sstpErrorReportCallback(const char* errorString, void* clientData) {
    printf("%s clientData - %p errorString - %s \n", __func__, clientData, errorString);
}

static void sstpSiteUpdateCallback(const char* siteName, double lat, double lon,
                                   float unc, int32_t uncConf, void* clientData) {
    printf("%s clientData - %p siteName - %s latitude - %f longitude - %f"
           " uncertainty - %f uncertainty confidence - %d\n",
           __func__, clientData, siteName, lat, lon, unc, uncConf);
}

static void sstpMccUpdateCallback(uint32_t mcc, const char* confidence,
                                  void* clientData) {
    printf("%s clientData - %p mcc - %d confidence - %s \n", __func__,
           clientData, mcc, confidence);
}

void sstp_help(char* cmd) {
    printf("For SSTP Test : %s -s <seconds to wait after listener registered> "
           " <seconds to wait after calling stopSstpUpdate> "
           " <seconds to wait after listener unregistered> \n", cmd);

}

int sstp_main(int argc, char** argv) {
    if (argc < 5) {
        printf("Usage: \n");
        sstp_help(argv[0]);
    } else {
        printf("Testing izat_remote_api sstpUpdate \n");

        printf("registerSstpUpdater \n");
        void* handle = registerSstpUpdater(sstpSiteUpdateCallback,
                                           sstpMccUpdateCallback,
                                           sstpErrorReportCallback,
                                           (void*)0xDEADBEEF);

        print_and_sleep(atoi(argv[2]));

        printf("stopSstpUdate...\n");
        stopSstpUpdate(handle);

        print_and_sleep(atoi(argv[3]));

        printf("unregisterLocationUpdater...\n");
        unregisterSstpUpdater(handle);

        print_and_sleep(atoi(argv[4]));

        printf("exiting...\n");
    }

    return 0;
}

// nmea updater Test
static void nmeaUpdateCallback(UlpNmea *nmea, void* clientData) {

    int n;
    bool charFound = false;
    if (sOptions.printLogs)
        printf("NMEA: %s \n", nmea->nmea_str);

    /* NMEA strings contains "\r\n" at end of the string, remove '\r' and write to port */
    size_t nmeaLen = strlen(nmea->nmea_str);
    char* p = strrchr(nmea->nmea_str, '\r');
    if ((NULL != p) && (*(p + 1) == '\n')) {
        *p = '\n';
        *(p + 1) = '\0';
        charFound = true;
    }
    n = write(tty_fd, nmea->nmea_str, nmeaLen - 1);
    if (charFound) {
        *p = '\r';
        *(p + 1) = '\n';
    }

    if (n < 0)
        printf("write() of %d bytes failed!\n", n);
}
static void locationUpdateCallback(UlpLocation *location,
                             GpsLocationExtended *locExtended,
                             void* clientData) {
    if (sOptions.printLogs) {
        printf("%s clientData - %p\n"
        "| position_source - 0x%x \n"
        "| LocGpsLocation.flags - 0x%x \n"
        "|\t lat - %f \n"
        "|\t lon - %f \n"
        "|\t accuracy - %f \n"
        "|\t altitude - %f \n"
        "|\t speed - %f \n"
        "|\t timestamp- %lld \n"
        ,__func__ , clientData,
        location->position_source,
        location->gpsLocation.flags,
        location->gpsLocation.latitude,
        location->gpsLocation.longitude,
        location->gpsLocation.accuracy,
        location->gpsLocation.altitude,
        location->gpsLocation.speed,
        location->gpsLocation.timestamp
        );
    }
}

void svInfoUpdateCallback(GnssSvNotification *svNotify,
                                 void* clientData)
{
   const char* constellationString[] = { "Unknown", "GPS", "SBAS", "GLONASS",
        "QZSS", "BEIDOU", "GALILEO" };
    if (sOptions.printLogs) {
        printf("%s clientData - %p\n" ,__func__ , clientData);
        for (int i = 0; i < svNotify->count && i < LOC_GNSS_MAX_SVS; i++) {
            if (svNotify->gnssSvs[i].type >
                sizeof(constellationString) / sizeof(constellationString[0]) - 1) {
                svNotify->gnssSvs[i].type = GNSS_SV_TYPE_UNKNOWN;
            }
            printf("   %03d: %*s  %02d    %f    %f    %f   0x%02X \n",
                i,
                13,
                constellationString[svNotify->gnssSvs[i].type],
                svNotify->gnssSvs[i].svId,
                svNotify->gnssSvs[i].cN0Dbhz,
                svNotify->gnssSvs[i].elevation,
                svNotify->gnssSvs[i].azimuth,
                svNotify->gnssSvs[i].gnssSvOptionsMask);
        }
    }
}

void location_help(char *cmd)
{
    printf("For Location Test: %s -l -t <time out in sec after reg> -w <time out in sec after unreg> -D <DR data sub mask> -p <0/1>\n", cmd);
    printf("    -t:  seconds to wait after listener registered: Default: %d\n", sOptions.t);
    printf("    -w:  seconds to wait after listener unregistered: Default: %d\n", sOptions.w);
    printf("    -D:  DR data type subscription mask\n"
           "         DR_POS_DATA        (0x001) Enable DR Position data indication\n"
           "         DR_SVINFO_DATA     (0x002) Enable DR SV Info data indication\n"
           "         DR_NMEA_DATA       (0x004) Enable DR NMEA data indication\n"
           "             Note:     Should subscribe for Position and SV report indication\n");


    printf("    -p:  Print Log strings, Defaults: Default: %d\n", sOptions.printLogs);
    printf("    -h:  print this help\n");
}

int location_main(int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: \n");
        location_help(argv[0]);
    } else {
        int opt;
        extern char *optarg;
        char *endptr;
        void *handle1 = NULL, *handle2 = NULL;

        while ((opt = getopt (argc, argv, "t:w:D:p:h")) != -1) {
            switch (opt) {
                case 't':
                    sOptions.t = atoi(optarg);
                    printf("Timeout after registration: %d \n", sOptions.t);
                    break;
                case 'w':
                    sOptions.w = atoi(optarg);
                    printf("Timeout after unregistration: %d \n", sOptions.w);
                    break;
                case 'D':
                    sOptions.finalSubMask = strtol(optarg, NULL, 16);
                    printf("QDR Data Sub Types : 0x%x \n", sOptions.finalSubMask);
                    break;
                case 'p':
                    sOptions.printLogs = atoi(optarg);
                    printf("Print NMEA strings : %d \n", sOptions.printLogs);
                    break;
                case 'h':
                default:
                    location_help(argv[0]);
                    return 0;
            }
        }

        setRequiredPerm();

        open_port();

        printf("Testing izat_remote_api locationUpdate \n");
        remoteClientInfo cbData;
        memset(&cbData, 0 , sizeof(remoteClientInfo));

        if (sOptions.finalSubMask & FINAL_POS_DATA_SUB_MASK){
            printf("registering for locationUpdateCallback \n");
            cbData.locCb = locationUpdateCallback;
        }
        if (sOptions.finalSubMask & FINAL_SVINFO_DATA_SUB_MASK){
            printf("registering for svInfoUpdateCallback\n");
            cbData.svReportCb = svInfoUpdateCallback;
        }
        if (sOptions.finalSubMask & FINAL_NMEA_DATA_SUB_MASK){
            printf("registering for nmeaUpdateCallback\n");
            cbData.nmeaCb = nmeaUpdateCallback;
        }

        if (sOptions.finalSubMask){
            printf("calling registerLocationUpdater \n");
            handle1 = registerLocationUpdater(&cbData, (void*)0xDEADBEEF);
        }

        print_and_sleep(sOptions.t);
        printf("unregisterLocationUpdater \n");
        if(handle1)
            unregisterLocationUpdater(handle1);

        print_and_sleep(sOptions.w);

        printf("exiting...\n");
    }

    return 0;
}

void printHelp(char* cmd) {
    printf("Usage: \n");
    location_help(cmd);
    sstp_help(cmd);
}

// main
int main(int argc, char** argv) {
    UTIL_READ_CONF_DEFAULT(LOC_PATH_GPS_CONF);

    if (argc < 2) {
        printHelp(argv[0]);
    } else {
        int opt;
        while ((opt = getopt (argc, argv, "ls")) != -1) {
            switch (opt) {
            case 'l':
                location_main(argc, argv);
                break;
            case 's':
                sstp_main(argc, argv);
                break;
            default:
                break;
            }
        }
    }
    exit(0);
}
