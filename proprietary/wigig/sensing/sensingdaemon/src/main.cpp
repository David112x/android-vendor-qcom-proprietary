/*********************************************************************
 Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

#include <android-base/logging.h>
#include <hidl/HidlTransportSupport.h>
#include <utils/StrongPointer.h>
#include "CommandListener.h"
#include "SensingDaemon.h"

#define LOG_TAG "sensingdaemon"

using android::hardware::configureRpcThreadpool;
using android::hardware::joinRpcThreadpool;
using android::sp;

int main(int /*argc*/, char** argv) {
    android::base::InitLogging(argv,
                               android::base::LogdLogger(android::base::SYSTEM));

    LOG(INFO) << "SensingDaemon is starting up...";

    configureRpcThreadpool(1, true /* callerWillJoin */);

    sp<SensingDaemon> sd = SensingDaemon::getInstance();

    CommandListener commandListener(sd);

    if (commandListener.startListener()) {
        LOG(ERROR) << "Unable to start CommandListener: " << strerror(errno);
        return -1;
    }

    joinRpcThreadpool();

    return 0;
}
