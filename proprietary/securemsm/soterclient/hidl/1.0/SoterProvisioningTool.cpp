/********************************************************************
---------------------------------------------------------------------
 Copyright (c) 2018 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
----------------------------------------------------------------------
SOTER Client app.
*********************************************************************/
/*===========================================================================

                      EDIT HISTORY FOR FILE

when       who     what, where, why
--------   ---     ----------------------------------------------------------
2/8/18     dy      Add hidl based client for decoupled soter
11/30/15   dy      Add soter client to support ATTK provision
===========================================================================*/

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/eventfd.h>
#include <errno.h>
#include <utils/Log.h>
#include <sys/mman.h>
#include <getopt.h>
#include <dlfcn.h>
#include <iostream>
#include <iomanip>
#include <vector>
#include <cstring>
#include <vendor/qti/hardware/soter/1.0/ISoter.h>
#include <vendor/qti/hardware/soter/1.0/types.h>
#include <limits.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif
#define LOG_TAG "vendor.qti.hardware.soter@1.0-provision"
//#define ALOGD(...) do {} while (0)  //Comment this line to enable ALOGD
//#define ALOGV(...) do {} while (0)  //Comment this line to enable ALOGV
using android::sp;
using vendor::qti::hardware::soter::V1_0::ISoter;
using vendor::qti::hardware::soter::V1_0::SoterErrorCode;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;

#define ATTK_COPY_NUM_DEFAULT          3
#define CMD_GENERATE_ATTK_KEY_PAIR     1
#define CMD_VERIFY_ATTK_KEY_PAIR       2
#define CMD_EXPORT_ATTK_PUBLIC_KEY     3
#define CMD_GET_DEVICE_ID              4

using namespace std;

static void qsc_usage(void)
{

    printf("*************************************************************\n");
    printf("************           SOTER CLIENT              ************\n");
    printf("*************************************************************\n");
    printf("\n"
        "Usage: soter_client [CMD]\n"
        "CMD can be:\n"
        " \t\t 1: Generate ATTK key pair\n"
        " \t\t 2: Verify ATTK key pair\n"
        " \t\t 3: Export ATTK public key \n"
        " \t\t 4: Get device ID \n"
        "---------------------------------------------------------\n\n\n");
}

SoterErrorCode GetDeviceId(void)
{
    hidl_vec<uint8_t> deviceId;
    size_t deviceIdLength;
    SoterErrorCode ret = SoterErrorCode::SOTER_ERROR_OK;

    ALOGI("Trying to connect to service ISoter V1.0");
    sp<ISoter> soterService = ISoter::getService();
    if (soterService.get() == nullptr) {
        ALOGE("Failed to get Soter service");
        return SoterErrorCode::SOTER_ERROR_OTHERS;
    }

    ALOGI("calling Soter service api : getDeviceId()");
    auto rc = soterService->getDeviceId(
                  [&](SoterErrorCode tmpReturn, const hidl_vec<uint8_t>& tmpDeviceId,
                      const auto& tmpDeviceIdLength) {;
                    ret = tmpReturn;
                    deviceIdLength = tmpDeviceIdLength;
                    deviceId = tmpDeviceId;
                  });
    if (!rc.isOk() || (ret != SoterErrorCode::SOTER_ERROR_OK)) {
        ALOGE("Failed to get device id, error %d", ret);
        printf("fail %d\n", ret);

    } else {
        for(auto p = deviceId.begin(); p!=deviceId.end(); ++p)
            printf("%02x",*p);
        printf("\n");
    }

    return ret;
}

SoterErrorCode VerifyAttkKeyPair(void)
{
    SoterErrorCode ret = SoterErrorCode::SOTER_ERROR_UNKNOWN_ERROR;

    ALOGI("Trying to connect to service ISoter V1.0");
    sp<ISoter> soterService = ISoter::getService();
    if (soterService.get() == nullptr) {
        ALOGE("Failed to get Soter service");
        return SoterErrorCode::SOTER_ERROR_OTHERS;
    }

    ALOGI("calling Soter service api : verifyAttkKeyPair()");
    ret = soterService->verifyAttkKeyPair();
    if (ret != SoterErrorCode::SOTER_ERROR_OK) {
        ALOGE("Failed to VerifyAttkKeyPair, error %d", ret);
        printf("fail %d\n", ret);
    } else
        printf("pass\n");

    return ret;
}

SoterErrorCode ExportAttkKeyPair(void)
{
    uint32_t PubKeyLen;
    SoterErrorCode ret = SoterErrorCode::SOTER_ERROR_UNKNOWN_ERROR;
    hidl_vec<uint8_t> pubKeyData;

    ALOGI("Trying to connect to service ISoter V1.0");
    sp<ISoter> soterService = ISoter::getService();
    if (soterService.get() == nullptr) {
        ALOGE("Failed to get Soter service");
        return SoterErrorCode::SOTER_ERROR_OTHERS;
    }

    ALOGI("calling Soter service api : exportAttkPublicKey()");
    auto rc = soterService->exportAttkPublicKey(
                  [&](SoterErrorCode tmpReturn, const hidl_vec<uint8_t>& tmpPubKeyData,
                      const auto& tmpPubKeyDataLength) {
                    ret = tmpReturn;
                    PubKeyLen = tmpPubKeyDataLength;
                    pubKeyData = tmpPubKeyData;
                  });

    if (!rc.isOk() || (ret != SoterErrorCode::SOTER_ERROR_OK)) {
        ALOGE("Failed to export attk, error %d", ret);
        printf("fail %d\n", ret);
    } else {
        printf("PubKeyLen %d\n", PubKeyLen);
        std::string str(pubKeyData.begin(), pubKeyData.end());
        std::cout << str;
    }

    return ret;
}

SoterErrorCode GenerateAttkKeyPair(void)
{
    SoterErrorCode ret = SoterErrorCode::SOTER_ERROR_UNKNOWN_ERROR;
    uint8_t copy_num = ATTK_COPY_NUM_DEFAULT;

    ALOGI("Trying to connect to service ISoter V1.0");
    sp<ISoter> soterService = ISoter::getService();
    if (soterService.get() == nullptr) {
        ALOGE("Failed to get Soter service");
        return SoterErrorCode::SOTER_ERROR_OTHERS;
    }

    ALOGI("calling Soter service api : generateAttkKeyPair()");
    ret = soterService->generateAttkKeyPair(copy_num);
    if (ret != SoterErrorCode::SOTER_ERROR_OK) {
        ALOGE("Failed to generate attk, error %d", ret);
        printf("fail %d\n", ret);
    } else
        printf("pass\n");

    return ret;
}

SoterErrorCode RunSoterCmd(char *argv[])
{
    SoterErrorCode ret = SoterErrorCode::SOTER_ERROR_UNKNOWN_ERROR;

    if(argv == NULL) {
        ALOGE("No arguments to process, exiting!");
        return SoterErrorCode::SOTER_ERROR_OTHERS;
    }
    /* CMD_ID */
    switch(atoi(argv[1])) {
    case CMD_GENERATE_ATTK_KEY_PAIR:
        ret = GenerateAttkKeyPair();
        break;
    case CMD_VERIFY_ATTK_KEY_PAIR:
        ret = VerifyAttkKeyPair();
        break;
    case CMD_EXPORT_ATTK_PUBLIC_KEY:
        ret = ExportAttkKeyPair();
        break;
    case CMD_GET_DEVICE_ID:
        ret = GetDeviceId();
        break;
    default:
        ALOGD("command not supported.");
        qsc_usage();
        break;
    }

    return ret;
}

int main(int argc, char **argv)
{
    SoterErrorCode ret = SoterErrorCode::SOTER_ERROR_UNKNOWN_ERROR;

    if( !argv || (argc != 2) || (!strcmp(argv[1],"h")))
        qsc_usage();
    else {
        ret = RunSoterCmd(argv);
        if (ret != SoterErrorCode::SOTER_ERROR_OK)
            ALOGE("qsc_run_soter_cmd failed,ret=%d", ret);
    }
    return static_cast<int>(ret);
}
