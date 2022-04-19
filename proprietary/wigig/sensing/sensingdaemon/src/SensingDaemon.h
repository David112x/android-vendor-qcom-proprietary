/*
 * Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef _SENSING_DAEMON_H
#define _SENSING_DAEMON_H

#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <utils/RefBase.h>
#include <utils/SortedVector.h>
#include <utils/String8.h>
#include <utils/StrongPointer.h>
#include <sysutils/SocketClient.h>

#include "qmi_client.h"
#include "wigig_sensing_lib.h"

using namespace android;

class SensingDaemon;

class SensingDaemon : public RefBase {
public:
    static sp<SensingDaemon> getInstance();

    int runCommand(SocketClient *cli, int argc, const char *const *argv);
    int runCommand(char *resultStr, int argc, const char *const *argv,
               size_t resultLen);

    int sensingStartService(char *resultStr, size_t resultLen);
    int sensingStopService(char *resultStr, size_t resultLen);
    int sensingGetMode(char *resultStr, size_t resultLen);
    int sensingGetParams(char *resultStr, size_t resultLen, int argc, const char *const *argv);
    int sensingHandleChangeMode(char *resultStr, size_t resultLen, int argc, const char *const *argv);
    int sensingChangeMode(char *resultStr, size_t resultLen, wigig_sensing_lib_mode_type mode, const char* channelArr,
                          const char* burstSizeOutputFile, const char* burstOutputFile,
                          const char* tsfOutputFile);
    int sensingStopMode(char *resultStr, size_t resultLen);
    int sensingClearData(char *resultStr, size_t resultLen, int argc, const char *const *argv);

    void sensingDumpingThread();
    static void* dumpingThreadFunc(void*);

    wigig_sensing_lib_client_type userHandle;
public:
    // general utilities
    // all file manipulations are limited to /data/vendor/sensing/
    int uint32ToFile(const char *fname, uint32_t value);
    void splitDataAndWrite(FILE *f, void* data, uint32_t dataLen);
    FILE* openFile(const char *fileName);

    // sensing related utilities
    int sensingWriteBurstSizeToFile(const char* burstSizeOutputFile);
    bool isModeValid(wigig_sensing_lib_mode_type mode);
private:
    SensingDaemon();
private:
    static sp<SensingDaemon> sInstance;
    uint32_t mBurstSize;
    bool mDumpingFlag;
    pthread_t mDumpThread;
    FILE *mBurstFile;
    FILE *mTsfFile;
};

#endif // _SENSING_DAEMON_H

