/*!
 *
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#include <chrono>
#include <thread>
#include <utils/StrongPointer.h>

#include "SPUHidlClientTest.h"
#include "log.h"

using namespace std;
using android::sp;

#define MAJOR 0
#define MINOR 2

#define TEST_APP_PATH "/vendor/firmware_mnt/image/spu_hal_test.signed"
#define TEST_APP_CHANNEL "iuicc"

SPUHidlClientTest* testFw = nullptr;

int32_t coreTestFlow() {

    int32_t ret = SPUHidlClientTest::SUCCESS;
    int32_t err = 0;

    // Check test framework pointer
    if(testFw == nullptr) {
        ALOGE("Invalid test Fw ptr");
        return SPUHidlClientTest::GENERAL_ERROR;
    }

    // Load test spu app if not loaded
    err = testFw->loadApp();
    ALOGI("loadApp() returned [%d]", err);
    if(err != SPUHidlClientTest::SUCCESS &&
            err != SPUHidlClientTest::ALREADY_LOADED) {
        ALOGE("Failed to load app [%s] with channel [%s], err [%d]",
        TEST_APP_PATH, TEST_APP_CHANNEL, err);
        return SPUHidlClientTest::GENERAL_ERROR;
    }

    // Test send message sync to spu test app
    err = testFw->sendPing();
    if(err != SPUHidlClientTest::SUCCESS) {
        ALOGE("PING-PONG test failed, err [%d]", err);
        ret = SPUHidlClientTest::GENERAL_ERROR;
    }

    // Test shared buffer read and write
    err = testFw->sendSharedBuffFlow();
    if(err != SPUHidlClientTest::SUCCESS) {
        ALOGE("Shared buffer flow test failed, err [%d]", err);
        ret = SPUHidlClientTest::GENERAL_ERROR;
    }

    // Trigger SSR and wait for recovery
    err = testFw->triggerSSRFlow();
    if(err != SPUHidlClientTest::SUCCESS) {
        ALOGE("Failed to trigger SSR, err [%d]", err);
        ret = SPUHidlClientTest::GENERAL_ERROR;
    }

    // On each cycle check if SSR flag was raised. In case SSR occurred
    // call SSR handling flow
    err = testFw->handleSSR();
    if(err != SPUHidlClientTest::SUCCESS) {
        ALOGE("Failed to handle SSR, err [%d]", err);
        ret = SPUHidlClientTest::GENERAL_ERROR;
    }

    return ret;
}

int main(int argc, char** argv) {

    // Print app version
    ALOGI("vendor.spu.qti@1.0-test-client version [%d.%d] Date [%s] Time [%s]",
            MAJOR, MINOR, __DATE__, __TIME__);

    // Check input
    if(argc > 2) {
    ALOGE("Can get only 1 command line parameter");
    return SPUHidlClientTest::INPUT_ERROR;
    }

    // Default cycles number is 1
    int testCycles = 1;

    // Number of cycles input could only be valid positive integer
    if(argc == 2) {
        int testCyclesInput = atoi(argv[1]);
        if(testCyclesInput > 0) {
            testCycles = testCyclesInput;
        } else {
            ALOGE("Input should be a number of cycles");
            return SPUHidlClientTest::INPUT_ERROR;
        }
    }

    // Print number of cycles
    ALOGD("Number of test cycles: [%d]", testCycles);

    // Init test framework
    testFw = new SPUHidlClientTest(TEST_APP_PATH, TEST_APP_CHANNEL);
    if(testFw == nullptr) {
        ALOGE("Failed to create framework object");
        return SPUHidlClientTest::GENERAL_ERROR;
    }

    ALOGD("Test framework created successfully");

    int32_t ret = SPUHidlClientTest::SUCCESS;

    // Test read sysparam and health API
    int32_t err = testFw->readSpuInfo();
    if(err != SPUHidlClientTest::SUCCESS) {
        ALOGE("Failed to read SPU info, err [%d]", err);
    }

    // Run test core in loop
    for(size_t cycle = 0; cycle < testCycles; ++cycle) {

        ret = SPUHidlClientTest::SUCCESS;
        ALOGI("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
        ALOGI("@@@         TEST CYCLE #%.2zu         @@@", cycle + 1);
        ALOGI("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n\n");

        if (coreTestFlow() != SPUHidlClientTest::SUCCESS) {
        ALOGE("Error in core test flow");
        ret = SPUHidlClientTest::GENERAL_ERROR;
        }

        ALOGI("\n\n@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@");
        ALOGI("@@@ TEST CYCLE #%.2zu done - ret: [%d] @@@", cycle + 1, ret);
        ALOGI("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n\n");
    }

    // Release test framework
    if(testFw != nullptr) {
        delete testFw;
        testFw = nullptr;
    }

    return 0;
}
