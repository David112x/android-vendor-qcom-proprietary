/*!
 *
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#pragma once

#include <android/hidl/allocator/1.0/IAllocator.h>
#include <android/hidl/memory/1.0/IMemory.h>
#include <vendor/qti/spu/1.0/ISPUManager.h>
#include <vendor/qti/spu/1.0/ISpcomSSREvent.h>
#include <utils/StrongPointer.h>
#include <stdint.h>
#include <ctime>

#include "SpcomSSREvent.h"

#include "log.h"

using namespace std;

using android::sp;
using vendor::qti::spu::V1_0::ISPUManager;
using vendor::qti::spu::V1_0::ISpcomClient;
using vendor::qti::spu::V1_0::ISpcomSharedBuffer;
using vendor::qti::spu::V1_0::ISpcomSSREvent;
using ::android::hidl::memory::V1_0::IMemory;
using ::android::hidl::allocator::V1_0::IAllocator;

struct SpcomSSREvent;

class SPUHidlClientTest {

public:

    // Definitions

    /*
     * Return values enum
     */
    enum testError {
        SUCCESS =  0,
        GENERAL_ERROR = -1,
        INVALID_PARAM = -2,
        RESOURCE_BUSY = -3,
        LOAD_APP_ERROR = -4,
        ALREADY_LOADED = -5,
        MESSAGING_ERROR = -6,
        SSR_ERROR = -7,
        SHARED_BUFFER_ERROR = -8,
        INPUT_ERROR = -9
    };

    explicit SPUHidlClientTest(const char *path, const char *channelName);
    ~SPUHidlClientTest();

    // Test flow methods

    /*
     * Read health status and external params of SPU
     *
     * @Return nonzero on failure, zero on success
     */
    int32_t readSpuInfo();

    /*
     * Load SPU app from path and channel name which were delivered from
     * command line
     *
     * @Return nonzero on failure, zero on success
     */
    int32_t loadApp();

    /*
     * Send echo command to spu application, and prints the first 4 bytes in
     * the response message
     *
     * @Return nonzero on failure, zero on success
     */
    int32_t sendPing();

    /*
     * Invokes ION test flow: create shared buffer, send share it with SPU,
     * lock buffer, send read and write commands to SPU side and unlock the
     * buffer
     *
     * @Return nonzero on failure, zero on success
     */
    int32_t sendSharedBuffFlow();

    /*
     * Invokes SSR - sends reset command to SPU
     *
     * @Return nonzero on failure, zero on success
     */
    int32_t triggerSSRFlow();

    /*
     * SSR handling flow
     *
     * @Return nonzero on failure, zero on success
     */
    int32_t handleSSR();

    bool mSSROccured;

private:

    friend struct SpcomSSREvent;

    int32_t registerSpcomClient();
    int32_t unregisterSpcomClient();

    // Definitions
    enum hidlCmdId {
        PING = 0,
        SHARED_BUFFER_INIT = 1,
        SHARED_BUFFER_READ = 2,
        SHARED_BUFFER_WRITE = 3,
        SHARED_BUFFER_RELEASE = 4,
        LAST = 5
    };

    // Internal
    int32_t initSpuManagerService();
    int32_t initTestFw();
    void freeTestFw();

    int32_t sendCmdImpl(hidlCmdId cmd_id, const char *expectedResp);
    int32_t sendInitSharedBuffCmd();
    int32_t sendReadSharedBuffCmd(char expectedByte);
    int32_t sendWriteSharedBuffCmd(char expectedByte);
    int32_t sendReleaseSharedBuffCmd();

    int32_t writeToSharedBuff(const char *str, uint64_t nBytes);
    int32_t readFromSharedBuff(uint64_t nBytes, char expectedByte);

    int32_t readAppVersion(const char* appName, uint32_t appId);

    // Const globals
    static const uint32_t sPgSz = 4096;
    static const uint32_t sDefaultTimeotMs = 5000;
    static const uint32_t sMaxSwapSz = 256 * 1024;
    static const uint32_t sOneSecondInMs = 1000;
    static const uint32_t sMaxResponseSz = 268;
    static const uint32_t sConnectionRetries = 15;

    // Class members
    mutex mMutex;
    sp<SpcomSSREvent> mSSREvent = nullptr;
    sp<ISPUManager> mSpuManager = nullptr;
    sp<ISpcomClient> mClient = nullptr;
    sp<ISpcomSharedBuffer> mSharedBuffer = nullptr;
    sp<IMemory> mHidlSharedMemory = nullptr;
    sp<IAllocator> mAshmemAllocator = nullptr;
    const char *mAppPath;
    const char *mAppName;
    bool mIsMessagingValid;
    bool mIsFwValid;
    clock_t mClock;

    // No copy
    SPUHidlClientTest(const SPUHidlClientTest&) = delete;
    SPUHidlClientTest& operator=(const SPUHidlClientTest&) = delete;
    SPUHidlClientTest() = delete;
};
