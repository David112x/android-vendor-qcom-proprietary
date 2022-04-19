/*
 * Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include "ResponseCode.h"
#include "SensingDaemon.h"

#include <android-base/logging.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <pthread.h>
#include <string>
#include <sstream>
#include <iostream>
#include <stdio.h>

#define MAX_VALUE_LENGTH    128
#define RESULT_STR_LENGTH   256
#define TIME_BETWEEN_READ_DATA 4000 /* 4ms */
#define DEFAULT_FILE_DIRECTORY "/data/vendor/sensing/"

sp<SensingDaemon> SensingDaemon::sInstance;

sp<SensingDaemon> SensingDaemon::getInstance()
{
    if (sInstance == NULL) {
        sInstance = new SensingDaemon();
    }

    return sInstance;
}

int SensingDaemon::sensingWriteBurstSizeToFile(const char* burstSizeOutputFile)
{
    LOG(INFO) << "Got burst size: " << mBurstSize;
    return uint32ToFile(burstSizeOutputFile, mBurstSize);
}

void* SensingDaemon::dumpingThreadFunc(void* pThis)
{
    SensingDaemon* pDaemon = static_cast<SensingDaemon*>(pThis);
    pDaemon->sensingDumpingThread();

    return NULL;
}

void SensingDaemon::sensingDumpingThread()
{
    LOG(INFO) << "Starting dumping thread";

    if (mBurstSize > CYCLIC_BUF_MAX_SIZE) {
        LOG(ERROR) << "too big burst size: " << mBurstSize;
        return;
    }

    void *dataBuf = malloc(CYCLIC_BUF_MAX_SIZE);
    if (dataBuf == NULL) {
        LOG(ERROR) << "Data buf allocation failed: " << errno;
        return;
    }

    uint32_t dataBufRecvLen = 0;
    /* size of dri tsf array in bytes = max number of bursts * size of dri tsf */
    uint32_t driTsfArrSize = (CYCLIC_BUF_MAX_SIZE / mBurstSize) * sizeof(uint64_t);
    uint64_t *driTsfArr = (uint64_t*)malloc(driTsfArrSize);
    if (driTsfArr == NULL) {
        LOG(ERROR) << "Dri tsf buf allocation failed: " << errno;
        free(dataBuf);
        return;
    }

    uint32_t numOfBursts = 0;

    while (mDumpingFlag) {
        int rc = wigig_sensing_lib_read_data
        (
            userHandle,
            dataBuf,
            CYCLIC_BUF_MAX_SIZE,
            &dataBufRecvLen,
            driTsfArr,
            driTsfArrSize,
            &numOfBursts
        );
        if (!rc) {
            if (dataBufRecvLen == 0 || numOfBursts == 0) {
                LOG(DEBUG) << "No data was available, skip writing";
            }
            else {
                /* write data to file */
                LOG(INFO) << "Read " << numOfBursts << " bursts, writing to file";

                splitDataAndWrite(mBurstFile, dataBuf, dataBufRecvLen);

                /* length of tsf_dris to write should be number of bursts(which is number of tsf) * size of each tsf */
                splitDataAndWrite(mTsfFile, driTsfArr, numOfBursts * sizeof(uint64_t));
            }
        }
        else{
            LOG(ERROR) << "Error reading data: " << rc;
            mDumpingFlag = false;
        }
    }

    free(dataBuf);
    free(driTsfArr);
}

int SensingDaemon::sensingGetMode(char *resultStr, size_t resultLen)
{
    wigig_sensing_lib_mode_type mode;
    int rc = wigig_sensing_lib_get_mode(userHandle, &mode);

    if (rc) {
        LOG(ERROR) << "Get mode failed: " << rc;
        strlcpy(resultStr, "Get mode failed", resultLen);
        return ResponseCode::OperationFailed;
    }

    LOG(INFO) << "Got mode =  " << mode;
    strlcpy(resultStr, std::to_string(mode).c_str(), resultLen);
    return ResponseCode::CommandOkay;
}

int SensingDaemon::sensingGetParams(char *resultStr, size_t resultLen, int argc, const char *const *argv)
{
    int resultCode = ResponseCode::CommandOkay;
    strlcpy(resultStr, "Get params finished", resultLen);

    if (argc != 3) {
        strlcpy(resultStr, "Incorrect number of parameters", resultLen);
        return ResponseCode::CommandParameterError;
    }

    FILE *paramsFile;
    const char* paramsOutputFile = argv[2];
    void* buf = malloc(WIGIG_SENSING_PARAMETERS_MAX_SIZE);
    uint32_t paramsRecvLen = 0;
    int rc = wigig_sensing_lib_get_parameters(userHandle, buf, WIGIG_SENSING_PARAMETERS_MAX_SIZE, &paramsRecvLen);

    if (rc || !paramsRecvLen) {
        LOG(ERROR) << __FUNCTION__ << "Error getting params: " << rc;
        resultCode = ResponseCode::OperationFailed;
        strlcpy(resultStr, "Error getting params", resultLen);
        goto out;
    }

    paramsFile = openFile(paramsOutputFile);
    if (paramsFile == NULL) {
        LOG(ERROR) << "GetParams output file error: " << -errno;
        resultCode = ResponseCode::OperationFailed;
        strlcpy(resultStr, "GetParams output file error", resultLen);
        goto out;
    }

    /* write data to file */
    splitDataAndWrite(paramsFile, buf, paramsRecvLen);
    fclose(paramsFile);

out:
    free(buf);
    return resultCode;
}

bool SensingDaemon::isModeValid(wigig_sensing_lib_mode_type mode)
{
    if (mode == WIGIG_SENSING_LIB_SEARCH_MODE || mode == WIGIG_SENSING_LIB_FACIAL_MODE ||
        mode == WIGIG_SENSING_LIB_GESTURE_MODE || mode == WIGIG_SENSING_LIB_STOP_MODE ||
        mode == WIGIG_SENSING_LIB_CUSTOME_MODE)
        return true;

    return false;
}

int SensingDaemon::sensingHandleChangeMode(char *resultStr, size_t resultLen, int argc, const char *const *argv)
{
    /* default message for change mode unless overwritten by more specific message */
    strlcpy(resultStr, "Changed mode finished", resultLen);

    if (argc < 3) {
        strlcpy(resultStr, "Incorrect number of parameters", resultLen);
        return ResponseCode::CommandParameterError;
    }

    uint32_t modeTemp;
    std::stringstream modeStr(argv[2]);
    modeStr >> modeTemp;
    wigig_sensing_lib_mode_type mode = (wigig_sensing_lib_mode_type)modeTemp;
    LOG(INFO) << __FUNCTION__ << " mode " << mode;

    if (!isModeValid(mode)) {
        strlcpy(resultStr, "Invalid mode", resultLen);
        return ResponseCode::CommandParameterValueError;
    }

    if (mode == WIGIG_SENSING_LIB_STOP_MODE) {
        return sensingStopMode(resultStr, resultLen);
    }
    /* nonstop mode requires the following: mode, channel, burstsize file, data file, tsf file */
    if (argc == 7) {
        return sensingChangeMode(resultStr, resultLen, mode, (const char*)argv[3], (const char*)argv[4], (const char*)argv[5], (const char*)argv[6]);
    }

    strlcpy(resultStr, "Incorrect number of parameters", resultLen);
    return ResponseCode::CommandParameterError;
}

int SensingDaemon::sensingStopMode(char *resultStr, size_t resultLen)
{
    LOG(INFO) << "Sending stop mode";
    int rc = wigig_sensing_lib_change_mode
    (
        userHandle,
        WIGIG_SENSING_LIB_STOP_MODE,
        0,
        &mBurstSize
    );

    /* Continue on to stop the thread regardless of change_mode result */
    if (!mDumpingFlag) {
        LOG(INFO) << __FUNCTION__ << "Dumping thread was not started";
    }
    else {
        LOG(INFO) << __FUNCTION__ << "Terminating dumping thread...";

        mDumpingFlag = false;
        wigig_sensing_lib_cancel_read(userHandle);
        pthread_join(mDumpThread, NULL);

        fclose(mBurstFile);
        fclose(mTsfFile);
    }

    if (rc) {
        LOG(ERROR) << __FUNCTION__ << "Error changing mode: " << rc;
        strlcpy(resultStr, "Changed mode to stop failed", resultLen);
        return ResponseCode::OperationFailed;
    }

    LOG(INFO) << __FUNCTION__ << " finished";

    strlcpy(resultStr, "Stop mode success", resultLen);
    return ResponseCode::CommandOkay;
}

int SensingDaemon::sensingChangeMode(char *resultStr, size_t resultLen, wigig_sensing_lib_mode_type mode, const char* channelArr,
                   const char* burstSizeOutputFile, const char* burstOutputFile, const char* tsfOutputFile)
{
    uint32_t channel;
    int rc;
    int pRes;
    std::stringstream channelStr(channelArr);
    channelStr >> channel;
    LOG(INFO) << __FUNCTION__ << ": channel " << channel;

    mBurstFile = openFile(burstOutputFile);
    if (mBurstFile == NULL) {
        LOG(ERROR) << "burstOutputFile fopen error: " << -errno;
        strlcpy(resultStr, "Error with bursts output file", resultLen);
        return ResponseCode::OperationFailed;
    }

    mTsfFile = openFile(tsfOutputFile);
    if (mTsfFile == NULL) {
        LOG(ERROR) << "tsfOutputFile fopen error: " << -errno;
        strlcpy(resultStr, "Error with tsf output file", resultLen);
        fclose(mBurstFile);
        return ResponseCode::OperationFailed;
    }

    /* Send change mode request only if it's ok to write burst/tsf files */
    mBurstSize = 0;
    rc = wigig_sensing_lib_change_mode
    (
        userHandle,
        mode,
        channel,
        &mBurstSize
    );

    LOG(INFO) << "Change mode status = " << rc << " burst size received = " << mBurstSize;
    if (rc) {
        strlcpy(resultStr, "Change mode failed, error", resultLen);
        goto out_err_close_all;
    }

    if (!mBurstSize) {
        strlcpy(resultStr, "Invalid burst size", resultLen);
        goto out_err_close_all;
    }

    /* write burst size to file once and close file during each change mode operation */
    if (sensingWriteBurstSizeToFile(burstSizeOutputFile)) {
        LOG(ERROR) << "burstSizeOutputFile fopen error: " << -errno;
        strlcpy(resultStr, "Error with burst size output file", resultLen);
        goto out_err_close_all;
    }

    if (mDumpingFlag) {
        mDumpingFlag = false;
        wigig_sensing_lib_cancel_read(userHandle);
        pthread_join(mDumpThread, NULL);
        LOG(INFO) << __FUNCTION__ << " Dumping thread terminated...";
    }

    mDumpingFlag = true;
    pRes = pthread_create(&mDumpThread, NULL, dumpingThreadFunc, this);
    if (pRes) {
        LOG(ERROR) << "Could not create dumping thread: " << pRes;
        strlcpy(resultStr, "Could not create dumping thread", resultLen);
        goto out_err_close_all;
    }

    strlcpy(resultStr, "Change mode success", resultLen);
    return ResponseCode::CommandOkay;

out_err_close_all:
    fclose(mTsfFile);
    fclose(mBurstFile);
    return ResponseCode::OperationFailed;
}

int SensingDaemon::sensingStartService(char *resultStr, size_t resultLen)
{
    int rc = wigig_sensing_lib_register(&userHandle, false);
    LOG(INFO) << "register finished, status: " << rc;
    if (rc) {
        strlcpy(resultStr, "Sensing register failed", resultLen);
        return ResponseCode::OperationFailed;
    }

    strlcpy(resultStr, "Sensingdaemon started successfully", resultLen);
    return ResponseCode::CommandOkay;
}

int SensingDaemon::sensingStopService(char *resultStr, size_t resultLen)
{
    /* continue onto stop daemon even if stop mode fails */
    sensingStopMode(resultStr, resultLen);

    int rc = wigig_sensing_lib_unregister(userHandle);
    LOG(INFO) << "unregister status: " << rc;
    if (rc) {
        strlcpy(resultStr, "Sensing unregister failed", resultLen);
        return ResponseCode::OperationFailed;
    }

    strlcpy(resultStr, "Sensingdaemon stopped successfully", resultLen);
    return ResponseCode::CommandOkay;
}

int SensingDaemon::sensingClearData(char *resultStr, size_t resultLen, int argc, const char *const *argv)
{
    /* clear data requires the following: data file, tsf file
       if data/tsf file names are new, new files will be created
       if data/tsf file names are the same as old ones,
       the old files will be discarded and replaced */
    if (argc < 4) {
        strlcpy(resultStr, "Incorrect number of parameters", resultLen);
        return ResponseCode::CommandParameterError;
    }

    if (mDumpingFlag) {
        mDumpingFlag = false;
        pthread_join(mDumpThread, NULL);
        fclose(mBurstFile);
        fclose(mTsfFile);
        LOG(INFO) << __FUNCTION__ << " Dumping thread terminated...";
    }

    int rc = wigig_sensing_lib_clear_data(userHandle);
    if (rc) {
        LOG(ERROR) << "Could not clear data: " << rc;
        strlcpy(resultStr, "Could not clear data", resultLen);
        return ResponseCode::OperationFailed;
    }

    mBurstFile = openFile(argv[2]);
    if (mBurstFile == NULL) {
        LOG(ERROR) << "burstOutputFile fopen error: " << -errno;
        strlcpy(resultStr, "Error with bursts output file", resultLen);
        return ResponseCode::OperationFailed;
    }

    mTsfFile = openFile(argv[3]);
    if (mTsfFile == NULL) {
        LOG(ERROR) << "tsfOutputFile fopen error: " << -errno;
        strlcpy(resultStr, "Error with tsf output file", resultLen);
        fclose(mBurstFile);
        return ResponseCode::OperationFailed;
    }

    mDumpingFlag = true;
    rc = pthread_create(&mDumpThread, NULL, dumpingThreadFunc, this);
    if (rc) {
        LOG(ERROR) << "Could not create dumping thread: " << rc;
        strlcpy(resultStr, "Could not create dumping thread", resultLen);
        fclose(mTsfFile);
        fclose(mBurstFile);
        return ResponseCode::OperationFailed;
    }

    LOG(INFO) << "Clear data success";
    strlcpy(resultStr, "Clear data success", resultLen);
    return ResponseCode::CommandOkay;
}

int SensingDaemon::runCommand(SocketClient *cli, int argc, const char *const *argv)
{
    char resultStr[RESULT_STR_LENGTH];
    int resultCode = runCommand(resultStr, argc, argv, sizeof(resultStr));

    cli->sendMsg(resultCode, resultStr, false);
    return 0;
}

int SensingDaemon::runCommand(char *resultStr, int argc, const char *const *argv,
                    size_t resultLen)
{
    LOG(INFO) << __FUNCTION__ << " got ndc command: " << argv[1];

    if (argc < 2) {
        strlcpy(resultStr, "Missing argument", resultLen);
        return ResponseCode::CommandSyntaxError;
    }

    if (!strcmp(argv[1], "start")) {
        return sensingStartService(resultStr, resultLen);
    }
    if (!strcmp(argv[1], "stop")) {
        return sensingStopService(resultStr, resultLen);
    }
    if (!strcmp(argv[1], "get_mode")) {
        return sensingGetMode(resultStr, resultLen);
    }
    if (!strcmp(argv[1], "get_params")) {
        return sensingGetParams(resultStr, resultLen, argc, argv);
    }
    if (!strcmp(argv[1], "change_mode")) {
        return sensingHandleChangeMode(resultStr, resultLen, argc, argv);
    }
    if (!strcmp(argv[1], "clear")) {
        return sensingClearData(resultStr, resultLen, argc, argv);
    }

    strlcpy(resultStr, "Unknown sensing daemon command", resultLen);
    return ResponseCode::CommandSyntaxError;
}

SensingDaemon::SensingDaemon()
{
    LOG(DEBUG) << "constructing SensingDaemon";
    mBurstSize = 0;
    mDumpingFlag = false;
    mBurstFile = NULL;
    mTsfFile = NULL;
    LOG(DEBUG) << "SensingDaemon is constructed";
}

int SensingDaemon::uint32ToFile(const char *fname, uint32_t value)
{
    String8 svalue = String8::format("%08x", value);
    FILE *f = openFile(fname);
    if (f == NULL)
        return -errno;
    size_t res = fwrite(svalue.string(), 1, svalue.length(), f);
    fclose(f);
    return (res == svalue.length() ? 0 : -EINVAL);
}

void SensingDaemon::splitDataAndWrite(FILE *f, void* data, uint32_t dataLen)
{
    uint32_t numOfDwords = dataLen / sizeof(uint32_t);
    size_t res;

    if (f == NULL) {
        LOG(ERROR) << "File handle is null, cannot write";
        return;
    }

    uint32_t* tempData = (uint32_t*)data;
    for (int i = 0; i < numOfDwords; i++) {
        String8 svalue = String8::format("%08x\n", *(tempData + i));
        res = fwrite(svalue.string(), 1, svalue.length(), f);
        if (res != svalue.length()) {
            LOG(ERROR) << "fwrite error: " << errno;
            return;
        }
    }
}

FILE* SensingDaemon::openFile(const char *fileName)
{
    /* default folder to store daemon files */
    std::string filePath = DEFAULT_FILE_DIRECTORY;
    filePath += fileName;
    return fopen(filePath.c_str(), "w");
}