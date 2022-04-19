/*
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

/*============================================================================
 FILE:         LocalLogBuffer.cpp

 OVERVIEW:     Log rildata information

 DEPENDENCIES: Logging, C++ STL
 ============================================================================*/
#define LOG_TAG "LocalLogBuffer/dump"

/* External Includes */
#include <cutils/log.h>
#include <time.h>
#include <unistd.h>

#ifdef QMI_RIL_UTF
#include <sys/time.h>
#endif

/* Internal Includes */
#include "LocalLogBuffer.h"

using namespace rildata;

LocalLogBuffer::LocalLogBuffer(string name, int maxLogs) : mName(name),
    mMaxLogs(maxLogs) {
} /* LocalLogBuffer */

void LocalLogBuffer::addLog(string log) {
    logMutex.lock();
    while (mLogs.size() > mMaxLogs)
        mLogs.pop_front();
    mLogs.push_back(log);
    logMutex.unlock();
} /* addLog */

void LocalLogBuffer::addLogWithTimestamp(string log) {
    struct timeval tv;
    char timeBuffer[50];
    char timeBufferWithMS[100];
    memset(timeBuffer,0,sizeof(timeBuffer));
    memset(timeBufferWithMS,0,sizeof(timeBufferWithMS));
    std::string finalLog("    ");
    gettimeofday(&tv, NULL);
    strftime(timeBuffer, sizeof(timeBuffer), "%F %X", (std::localtime (&tv.tv_sec)));
    snprintf(timeBufferWithMS, sizeof(timeBufferWithMS), "%s.%03d",timeBuffer, (int)tv.tv_usec/1000);
    finalLog += (timeBufferWithMS);
    finalLog += " [" + getName() + "]";
    finalLog += log;
    logMutex.lock();
    while (mLogs.size() > mMaxLogs)
        mLogs.pop_front();
    mLogs.push_back(finalLog);
    logMutex.unlock();
} /* addLog */

void LocalLogBuffer::toLogcat() {
    logMutex.lock();
    for (size_t i = 0; i < mLogs.size(); i++)
        ALOGD("%s: %s", mName.c_str(), mLogs[i].c_str());
    logMutex.unlock();
} /* toLogcat */

void LocalLogBuffer::toFd(string msg, int fd) {
    write(fd, msg.c_str(), msg.size());
} /* toFd */

void LocalLogBuffer::setName(string name) {
    logMutex.lock();
    mName = name;
    logMutex.unlock();
}

string LocalLogBuffer::getName() {
    return mName;
}

vector<string> LocalLogBuffer::getLogs()
{
    logMutex.lock();
    vector<string> retLogs(mLogs.begin(), mLogs.end());
    logMutex.unlock();
    return retLogs;
}
