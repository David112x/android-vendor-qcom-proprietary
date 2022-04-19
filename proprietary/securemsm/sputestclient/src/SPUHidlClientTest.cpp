/*!
 *
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#include <hidl/HidlSupport.h>
#include <hidl/HidlTransportSupport.h>
#include <android/hidl/allocator/1.0/IAllocator.h>
#include <android/hidl/memory/1.0/IMemory.h>
#include <hidlmemory/mapping.h>
#include <fstream>
#include <errno.h>
#include <chrono>
#include <thread>
#include <mutex>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <qti-utils.h>

#include "SPUHidlClientTest.h"
#include "log.h"

using namespace std;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_vec;

// command struct to send commands to spu_hal_test spu application
struct Command {
    uint32_t cmdId;
    uint64_t sharedBuffAddress;
    uint32_t sharedBuffSize;
};

SPUHidlClientTest::SPUHidlClientTest(const char *path, const char *appName) :
       mSSROccured(false), mMutex(), mAppPath(path), mAppName(appName),
       mIsMessagingValid(false), mIsFwValid(false), mClock(0) {

    if (initTestFw() != 0 || path == nullptr || appName == nullptr) {
        ALOGE("Failed to init test framework");
        return;
    }

    mIsFwValid = true;
}

SPUHidlClientTest::~SPUHidlClientTest() {
    freeTestFw();
}

// Test sysparam read API to read SPU application's version
int32_t SPUHidlClientTest::readAppVersion(const char *appName,
        uint32_t appId) {

    if (appName == nullptr) {
        ALOGE("Read app version - invalid param");
        return INVALID_PARAM;
    }

    mSpuManager->sysparamRead(ISPUManager::SysparamID::APP_BUILD_VERSION,
    appId, 0, [&](int32_t err, uint32_t buildVer) {
        if (err != 0) {
            ALOGE("Failed to read app build version, err[%d]", err);
            return GENERAL_ERROR;
        }

        ALOGI("%s, v%d.%d", appName, ((uint32_t) - 1 == buildVer) ?
        0 : (buildVer >> 16), ((uint32_t) - 1 == buildVer) ?
        0 : (buildVer & 0xFFFF));

        return SUCCESS;
    });

    return SUCCESS;
}

// Test SPU health status API and prints SPU applications version
int32_t SPUHidlClientTest::readSpuInfo() {

    ALOGD("Read health status");

    unique_lock<mutex> lock(mMutex);

    if (mIsFwValid == false) {
        ALOGE("Test framework error, cannot load app");
        return GENERAL_ERROR;
    }

    // Print health status
    mSpuManager->checkHealth(
            [&](int32_t err, const ISPUManager::HealthStatusData &status) {

            if (err != 0) {
                    ALOGE("Failed to read health status, err[%d]", err);
                    return GENERAL_ERROR;
                }

                ALOGI("ARI mode [%d]", status.ariMode);
                ALOGI("ARI fuse gauge [%d]", status.ariFuseGauge);
                ALOGI("Fips enables [%d]", status.fipsEnabled);
                ALOGI("Fips self test [%d]", status.fipsSelfTestPassed);
                ALOGI("Sensors calibrated [%d]", status.sensorsCalibrated);
                ALOGI("Ari on fuse [%d]", status.ariOnFuse);
                ALOGI("POR Indicator [%d]", status.porIndicator);
                ALOGI("POR status [%d]", status.porCounter);
                ALOGI("AUX data[0] [%d]", status.auxData0);
                ALOGI("AUX data[1] [%d]", status.auxData1);
                ALOGI("AUX DATA[2] [%d]", status.auxData2);
                ALOGI("AUX DATA[3] [%d]", status.auxData3);
                ALOGI("IAR CE status [%d]", status.iarCeStatus);
                return SUCCESS;
            });

    // Read build hash
    mSpuManager->sysparamRead(ISPUManager::SysparamID::BUILD_VERSION_HASH, 0,
    0, [&](int32_t err, uint32_t buildHash) {
                if (err != 0) {
                    ALOGE("Failed to read app build version");
                    return GENERAL_ERROR;
                }

                ALOGI("SPSS build hash: %08X", buildHash);

                return SUCCESS;
            });

    // Print applications version
    uint32_t ret = readAppVersion("asym_cryptoapp", 0x3D86A887);
    ret |= readAppVersion("cryptoapp", 0xEF80345E);
    ret |= readAppVersion("sp_keymaster", 0x24500E2F);
    ret |= readAppVersion("macchiato", 0x09068E40);

    if(ret != SUCCESS) {
        ALOGE("Failed to read application version, err [%d]", ret);
        return GENERAL_ERROR;
    }

    return ret;
}

int32_t SPUHidlClientTest::initTestFw() {

    unique_lock<mutex> lock(mMutex);

    ALOGD("Init test framework");

    // Init SPU hidl service and wait for SPU link up
    if (initSpuManagerService() != 0) {
        ALOGE("Failed to get service");
        return GENERAL_ERROR;
    }

    // Allocate shared memory in size of iUICC app
    mAshmemAllocator = IAllocator::tryGetService("ashmem");
    if (mAshmemAllocator == nullptr) {
        ALOGE("Failed to get shared memory allocator service");
        return GENERAL_ERROR;
    }

    return SUCCESS;
}

// To release objects obtained from SPU HAL server as strong pointer call
// sp::clear API
//
// Strong pointer clear function decrement the pointer copies count and set it
// to nullptr. When the counter is zero the destructor is called and the
// resources of the object are released. Since the service doesn't hold copies
// of the spcom client and spcom shared buffer objects, when the client clear
// all the copies of strong pointer objects then the destructor will be called
// and the resources will be released
void SPUHidlClientTest::freeTestFw() {

    mIsFwValid = false;
    mIsMessagingValid = false;

    if (mAshmemAllocator != nullptr) {
        ALOGD("Clear ashmem allocator");
        mAshmemAllocator.clear();
    }

    if (mHidlSharedMemory != nullptr) {
        ALOGD("Clear hidl shared memory");
        mHidlSharedMemory.clear();
    }

    if (mSharedBuffer != nullptr) {
        ALOGD("Clear SPU shared buffer");
        mSharedBuffer.clear();
    }

    if (mSSREvent != nullptr) {
        ALOGD("Release SSR event");
        mSSREvent.clear();
    }

    if (mClient != nullptr) {
        ALOGD("Clear SPU client");
        mClient.clear();
    }

    if (mSpuManager != nullptr) {
        ALOGD("Clear SPU hidl server");
        mSpuManager.clear();
    }
}

int32_t SPUHidlClientTest::initSpuManagerService() {

    // Get SPU hidl service handle
    mSpuManager = ISPUManager::tryGetService();
    if (mSpuManager == nullptr) {
        ALOGE("Failed to get service ISPUManager");
        return GENERAL_ERROR;
    }

    // Wait for SPU link up
    bool linkStatus = mSpuManager->waitForLinkUp(sDefaultTimeotMs);
    if (linkStatus != true) {
        ALOGE("SPU not linked up");
        return RESOURCE_BUSY;
    }

    return SUCCESS;
}

// Load SPU application:
//   Allocate HIDL shared memory
//   Copy The application content to the shared memory
//   Call spcomlib load app from buffer API
//   Check app is loaded
int32_t SPUHidlClientTest::loadApp() {

    unique_lock<mutex> lock(mMutex);

    if (mIsFwValid == false) {
        ALOGE("Test framework error, cannot load app");
        return GENERAL_ERROR;
    }

    if (mAppPath == nullptr || mAppName == nullptr) {
        ALOGE("Load app invalid params");
        return INVALID_PARAM;
    }

    if (mSpuManager->isAppLoaded(mAppName) == true) {
        ALOGD("App is already loaded [%s]", mAppName);
        return ALREADY_LOADED;
    }

    ALOGD("Going to load app [%s], channel [%s]", mAppPath, mAppName);

    // Open app image for read
    ifstream appBinFile(mAppPath, ifstream::in);
    if (!appBinFile) {
        ALOGE("Couldn't open image file");
        return GENERAL_ERROR;
    }

    // Read app size
    appBinFile.seekg(0, appBinFile.end);
    int fileLen = appBinFile.tellg();
    appBinFile.seekg(0, appBinFile.beg);

    ALOGD("App file size [%d]", fileLen);

    if(fileLen <= 0) {
        ALOGE("Failed to read file [%s], size[%d]", mAppPath, fileLen);
        return GENERAL_ERROR;
    }

    mAshmemAllocator->allocate(fileLen,
            [&](bool success, const hidl_memory &mem) {

            if (!success) {
                    ALOGE("Failed to allocate shared memory");
                    return GENERAL_ERROR;
                }

                sp<IMemory> memory;

                // Map HIDL shared memory to client
                memory = mapMemory(mem);
                if (memory == nullptr) {
                    ALOGE("Failed to fetch IMemory obj to handle the hidl_mem");
                    return GENERAL_ERROR;
                }

                // Copy SPU application content to the mapped shared memory
                memory->update();
                void *data = memory->getPointer();

                for (long int i = 0; i < fileLen; ++i) {
                    *(((uint8_t*) data) + i) = appBinFile.get();
                }
                memory->commit();

                // Load application share with HIDL server over HIDL shared
                // memory
                int res = mSpuManager->loadApp(mAppName, mAppName, mem, fileLen,
                        sMaxSwapSz);

                if (res != 0) {
                    ALOGE("Failed to load app, res [%d]", res);
                    return LOAD_APP_ERROR;
                }

                return SUCCESS;
            });

    // Check if app was loaded successfully
    if (mSpuManager->isAppLoaded(mAppName) != true) {
        ALOGE("Failed to load app");
        return LOAD_APP_ERROR;
    }

    ALOGD("App [%s] was loaded successfully, channel [%s]", mAppPath, mAppName);

    return SUCCESS;
}

// Open spcom channel
int32_t SPUHidlClientTest::registerSpcomClient() {

    ALOGD("Register client [%s] was called", mAppName);

    if (mSpuManager == nullptr) {
        ALOGE("Can't get client before spu object was obtained");
        return INVALID_PARAM;
    }

    if (mIsFwValid == false) {
        ALOGE("Test framework not initialized");
        return GENERAL_ERROR;
    }

    mClient = mSpuManager->getSpcomClient(mAppName);
    if (mClient == nullptr) {
        ALOGE("Failed to obtain client object");
        return GENERAL_ERROR;
    }

    ALOGD("Get client done successfully");

    // Create SSR event handler
    if(!mSSREvent) {
        mSSREvent = new SpcomSSREvent(this);
        if(!mSSREvent) {
            ALOGE("Failed to create SSR handling object");
        }
    }

    if(mClient->addSSRCallback(mSSREvent) != 0) {
        ALOGE("Failed to set SSR callback");
        return GENERAL_ERROR;
    }

#if (SPSS_TARGET == 8250)
    bool isShareable = true;
#endif

    int32_t ret = mClient->registerClient(isShareable);
    if (ret != 0) {
        ALOGE("Failed to register client, err[%d]", ret);
        return GENERAL_ERROR;
    }
    ALOGD("Registered client successfully, channel [%s] - shareable [%d]",
            mAppName, isShareable);

    // Will fail if no connection within <sConnectionRetries> seconds
    int connectRetries = sConnectionRetries;
    while (mClient->isConnected() != true) {

        ALOGD("Wait for channel connectivity [%s] - Retries left [%d]",
                mAppName, connectRetries);

        this_thread::sleep_for(chrono::milliseconds(sOneSecondInMs));

        if (--connectRetries == 0) {
            ALOGE("No connection on channel [%s]", mAppName);
            mIsMessagingValid = false;
            return RESOURCE_BUSY;
        }
    }

    mIsMessagingValid = true;

    ALOGD("Client is now connected to server [%s]", mAppName);

    return SUCCESS;
}

// close spcom channel
int32_t SPUHidlClientTest::unregisterSpcomClient() {

    ALOGD("Unregister client [%s] was called", mAppName);

    if (mClient == nullptr) {
        ALOGE("Can't unregister null client");
        return GENERAL_ERROR;
    }

    uint32_t ret = mClient->unregisterClient();
    if (ret != 0) {
        ALOGE("Failed to unregister client, err[%u]", ret);
        return GENERAL_ERROR;
    }

    mClient = nullptr;

    return SUCCESS;
}

// Impl function to commands to spu_hal_test SPU application
// Sends command according to command ID and verify the response buffer
// expected from SPU side against the expected response buffer
int32_t SPUHidlClientTest::sendCmdImpl(hidlCmdId cmdId,
        const char *expectedResp) {

    ALOGD("Send cmd: [%d]", cmdId);

    if (cmdId >= hidlCmdId::LAST) {
        ALOGE("Invalid cmd ID [%d]", cmdId);
        return INVALID_PARAM;
    }

    // Get SPU Client, register and wait for connection
    if (mClient == nullptr || mIsMessagingValid == false) {
        ALOGD("Register client");
        int32_t ret = registerSpcomClient();
        if (ret != 0) {
            ALOGE("Failed to register client, err[%d]", ret);
            return GENERAL_ERROR;
        }
    }

    // Prepare cmd message to be sent to spu server
    Command cmd;
    memset(&cmd, 0x00, sizeof(Command));
    cmd.cmdId = cmdId;

    // Copy message to hidl vector
    hidl_vec<uint8_t> msg(sizeof(Command));
    for (size_t i = 0; i < sizeof(Command); ++i) {
        msg[i] = ((uint8_t*) &cmd)[i];
    }

   // Send cmd message to SPU server and wait for response
   // if response is valid print the response to log
    int32_t ret = SUCCESS;
    mClient->sendMessage(msg, sDefaultTimeotMs,
            [&](int32_t rxBytes, const hidl_vec<uint8_t> &response) {

                if (response.data() == nullptr) {
                    ALOGE("Failed to get response, err: %d", rxBytes);
                    ret = MESSAGING_ERROR;
                    return ret;
                }

                ALOGD("Response: %.10s, response size: %zu, rxBytes: %d",
                        response.data(), response.size(), rxBytes);

                if (strncmp((const char*) response.data(), expectedResp,
                        rxBytes) != 0) {
                    ALOGE("Unexpected response: %.10s, expected: %.10s",
                            response.data(), expectedResp);
                    ret = MESSAGING_ERROR;
                }

                return ret;
            });

    return ret;
}

// Test send message API to SPU app and wait for response
int32_t SPUHidlClientTest::sendPing() {

    unique_lock<mutex> lock(mMutex);

    if (sendCmdImpl(hidlCmdId::PING, "PONG") != SUCCESS) {
        ALOGE("PING-PONG test failed");
        return GENERAL_ERROR;
    }

    ALOGD("PING-PONG test done successfully");

    return SUCCESS;
}

// Allocate share buffer and send it to SPU application.
// The SPU app will get the physical address of the buffer in the DDR
// and map it for its usage
int32_t SPUHidlClientTest::sendInitSharedBuffCmd() {

    Command cmd;
    cmd.cmdId = hidlCmdId::SHARED_BUFFER_INIT;
    cmd.sharedBuffAddress = 0;
    cmd.sharedBuffSize = sPgSz;

    ALOGD("Send cmd: [%d]", cmd.cmdId);

    // Get SPU Client, register and wait for connection
    if (mClient == nullptr || mIsMessagingValid == false) {
        if (registerSpcomClient() != 0) {
            ALOGE("Failed to register client");
            return GENERAL_ERROR;
        }
    }

    hidl_vec<uint8_t> msg(sizeof(Command));

    for (size_t i = 0; i < sizeof(Command); ++i) {
        msg[i] = ((uint8_t*) &cmd)[i];
    }

    // Allocate HIDL shared memory for SPU shared buffer
    // HIDL shared memory is shared between HIDL client and server
    // SPU shared buffer also holds an internal ION buffer which is shared with
    // SPU server application
    int32_t ret = SUCCESS;
    mAshmemAllocator->allocate(sPgSz,
            [&](bool success, const hidl_memory &mem) {

                // Check allocation status
                if (!success) {
                    ALOGE("Failed to allocate shared memory");
                    ret = GENERAL_ERROR;
                    return ret;
                }

               // Create SPUSharedBuffer - double buffer
               // HIDL client <=> HIDL server: hidl_memory
               // HIDL server <=> SPU app: ION buffer
                mSharedBuffer = mClient->getSpcomSharedBuffer(mem);
                if (mSharedBuffer == nullptr) {
                    ALOGE("Failed to get shared buffer");
                    ret = GENERAL_ERROR;
                    return ret;
                }

                // Map HIDL memory buffer
                mHidlSharedMemory = mapMemory(mem);
                if (mHidlSharedMemory == nullptr) {
                    ALOGE("Failed to map HIDL memory");
                    ret = GENERAL_ERROR;
                }

                return ret;
            });

    ALOGD("Got sharedBuffer successfully, going to send modify command to %s",
    mAppName);

    // Calculate the offset of the buffer address to be shared with SPU side
    size_t offset = (size_t) &cmd.sharedBuffAddress - (size_t) &cmd.cmdId;

    // Send the message and the shared buffer address and size to SPU app
    mClient->sendSharedBufferMessage(msg, mSharedBuffer, offset,
            sDefaultTimeotMs,
            [&](int32_t rxBytes, const hidl_vec<uint8_t> &response) {
                if (response.data() == nullptr) {
                    ALOGE("Failed to get response, err: %d", rxBytes);
                    ret = MESSAGING_ERROR;
                    return ret;
                }

                ALOGD("Response: %.10s, response size: %zu, rxBytes: %d",
                        response.data(), response.size(), rxBytes);

                const char *expectedResp = "SHARED BUFFER INIT DONE";
                if (strncmp((const char*) response.data(), expectedResp,
                        rxBytes) != 0) {
                    ALOGE("Unexpected response: %.10s, expected: %.10s",
                            response.data(), expectedResp);
                    ret = MESSAGING_ERROR;
                }

                return ret;
            });

    return ret;
}

// Ask SPU app to read from shared buffer and share the content
// Verify the response and also the shared buffer content
int32_t SPUHidlClientTest::sendReadSharedBuffCmd(char expectedByte) {

    // Created expected response buffer
    char expectedResp[sMaxResponseSz] = { 0 };
    memset(expectedResp, expectedByte, sMaxResponseSz - 1);
    expectedResp[sMaxResponseSz - 1] = '\0';

    // Send shared buffer read command to SPU side
    if (sendCmdImpl(hidlCmdId::SHARED_BUFFER_READ, expectedResp) != SUCCESS) {
        ALOGE("SPU read [%c]s from shared buffer test failed", expectedByte);
        return GENERAL_ERROR;
    }

    mHidlSharedMemory->read();

    // Read 4 first byres from HIDL shared memory
    void *data = mHidlSharedMemory->getPointer();
    ALOGD("Read first 4 bytes directly from shared memory: %c%c%c%c",
            ((uint8_t* )data)[0], ((uint8_t* )data)[1], ((uint8_t* )data)[2],
            ((uint8_t* )data)[3]);

    mHidlSharedMemory->commit();

    return SUCCESS;
}

// Ask SPU app to write to the shared buffer and verify the response
int32_t SPUHidlClientTest::sendWriteSharedBuffCmd(char expectedByte) {

    // Created expected response buffer
    char expectedResp[sMaxResponseSz] = { 0 };
    memset(expectedResp, expectedByte, sMaxResponseSz - 1);
    expectedResp[sMaxResponseSz - 1] = '\0';

    // Send shared buffer write command to SPU side
    if (sendCmdImpl(hidlCmdId::SHARED_BUFFER_WRITE, expectedResp) != SUCCESS) {
        ALOGE("SPU write [%c]s to shared buffer test failed", expectedByte);
        return GENERAL_ERROR;
    }

    // Sync shared buffer - will copy from ION buffer to hidl shared buffer
    // After sync the data in the ION buffer will be accessiable to HIDL client
    mSharedBuffer->copyFromSpu();

    mHidlSharedMemory->read();

    // Read 4 first byres from hidl shared memory - for debug
    void *data = mHidlSharedMemory->getPointer();
    ALOGD("Read first 4 bytes directly from shared memory: %c%c%c%c",
            ((uint8_t* )data)[0], ((uint8_t* )data)[1], ((uint8_t* )data)[2],
            ((uint8_t* )data)[3]);

    mHidlSharedMemory->commit();

    return SUCCESS;
}

int32_t SPUHidlClientTest::writeToSharedBuff(const char *str, uint64_t nBytes) {

    // Get pointer of hidl shared memory
    void *data = mHidlSharedMemory->getPointer();
    if (data == nullptr) {
        ALOGE("Failed to obtain hidl shared memory data");
        return GENERAL_ERROR;
    }

    // Check in hidl shared buffer for edit
    mHidlSharedMemory->update();

    // Write directly to hidl shared buffer
    uint64_t ret = memscpy(data, sPgSz, str, nBytes + 1);
    if (ret != nBytes + 1) { // + 1 is for null terminator
        ALOGE("Failed to write all message to hidl shared memory, ret [%zu]",
                ret);
    }

    // Commit changes to hidl shared buffer
    // This is still not obtained in SPU app side
    mHidlSharedMemory->commit();

    // Flush will make the HIDL server copy the changes from hidl shared memory to
    // ION buffer and will be available is SPU app side
    mSharedBuffer->copyToSpu();

    ALOGD("Wrote %.10s to shared buffer", str);

    return SUCCESS;
}

int32_t SPUHidlClientTest::readFromSharedBuff(uint64_t nBytes, char expectedByte) {

    // Sync hidl shared memory with ION buffer content (sync the double buffer)
    mSharedBuffer->copyFromSpu();

    // Get pointer of hidl shared memory
    void *data = mHidlSharedMemory->getPointer();
    if (data == nullptr) {
        ALOGE("Failed to obtain hidl shared memory data");
        return GENERAL_ERROR;
    }

    // Copy nBytes from hidl shared buffer to read buffer and print to log
    char readBuff[sMaxResponseSz];
    memset(readBuff, 0x00, sMaxResponseSz);
    memscpy(readBuff, sMaxResponseSz - 1, data, nBytes);
    ALOGD("Read directly from shared buffer [%.10s]", readBuff);

    // Compare response to expected
    char expectedResp[sMaxResponseSz];
    memset(expectedResp, expectedByte, sMaxResponseSz - 1);
    expectedResp[sMaxResponseSz - 1] = '\0';

    if (strncmp(readBuff, expectedResp, nBytes) != 0) {
        ALOGE("Unexpected response: %.10s, expected: %.10s", readBuff,
                expectedResp);
        return GENERAL_ERROR;
    }

    return SUCCESS;
}

int32_t SPUHidlClientTest::sendReleaseSharedBuffCmd() {

    // Release buffer from SPU side
    if (sendCmdImpl(hidlCmdId::SHARED_BUFFER_RELEASE, "SHARED BUFFER FREE DONE") != 0) {
        ALOGE("SPU app failed to release buffer");
        return GENERAL_ERROR;
    }

    return SUCCESS;
}

int32_t SPUHidlClientTest::sendSharedBuffFlow() {

    unique_lock<mutex> lock(mMutex);

    // Check test framework is in valid state
    if (mIsFwValid == false || mIsMessagingValid == false) {
        ALOGE("Test framework error, cannot run shared buffer flow");
        return GENERAL_ERROR;
    }

    // Init shared buffer and send it to SPU app
    // SPU app will open extmem window
    ALOGD("***********INIT-SHARED-BUFFER-TEST************");

    int32_t ret = sendInitSharedBuffCmd();
    if (ret != 0) {
        ALOGE("Failed to init shared buffer, err[%d]", ret);
        return ret;
    }

    // Write 'A's to shared buffer
    ALOGD("*********WRITE-A-TO-SHARED-BUFFER*************");

    char msg[sMaxResponseSz];
    memset(msg, 'A', sMaxResponseSz);
    ret = writeToSharedBuff(msg, sMaxResponseSz);
    if (ret != 0) {
        ALOGE("Failed to write to shared buffer, err[%d]", ret);
        return ret;
    }

    // Send SPU app a command to read from shared buffer and expect all 'A's
    ALOGD("****SPU-READ-FROM-SHARED-BUFFER-EXPECT-A*****");

    ret = sendReadSharedBuffCmd('A');
    if (ret != 0) {
        ALOGE("Failed to send read buffer cmd, err[%d]", ret);
        return ret;
    }

    // Send SPU app a command to write 'B's to shared buffer
    ALOGD("*******SPU-WRITE-B-TO-SHARED-BUFFER**********");

    ret = sendWriteSharedBuffCmd('B');
    if (ret != 0) {
        ALOGE("Failed to send write buffer cmd");
        return ret;
    }

    // Send SPU app a command to read from shared buffer and expect all 'B's
    ALOGD("****SPU-READ-FROM-SHARED-BUFFER-EXPECT-B*****");

    ret = sendReadSharedBuffCmd('B');
    if (ret != 0) {
        ALOGE("Failed to send read buffer cmd");
        return ret;
    }

    // Read from shared buffer and expect all 'B's
    ALOGD("***READ-FROM-SHARED-BUFFER-EXPECT-B*********");

    size_t bytesToRead = sMaxResponseSz;
    ret = readFromSharedBuff(bytesToRead, 'B');
    if (ret != 0) {
        ALOGE("Failed to read from shared buffer");
        return ret;
    }

    ret = sendReleaseSharedBuffCmd();
    if(ret != 0) {
        ALOGE("Failed to release shared buff from spu side");
        return ret;
    }

    return SUCCESS;
}

int32_t SPUHidlClientTest::triggerSSRFlow() {

    unique_lock<mutex> lock(mMutex);

    ALOGD("Reset SPU");

    // Check test framework is in valid state
    if (mIsFwValid == false) {
        ALOGE("Test framework error, cannot run shared buffer flow");
        return GENERAL_ERROR;
    }

    // Get SSR trigger time
    mClock = clock() * 1000 / CLOCKS_PER_SEC;

    // Reset SPU
    int32_t ret = mSpuManager->resetSpu();
    if(ret != 0) {
        ALOGE("Failed to reset SPU, err[%d]", ret);
        return SSR_ERROR;
    }

   return SUCCESS;
}

// SSR handling in service releases all shared buffers data
// and change the internal state of all buffers to invalid
// User should release the shared buffers by calling
// mSharedBuffer.clear() and to allocate new buffers
// User should also call mClient->unregister() and
// then mClient->register() to reset spcom client channel
int32_t SPUHidlClientTest::handleSSR() {

    // Wait for SSR handling lock
    unique_lock<mutex> lock(mMutex);

    // Wait for SSR detection
    for(int retries = 30; retries && mSSROccured == false; --retries) {
        this_thread::sleep_for(chrono::milliseconds(sOneSecondInMs));
    }

    if(mSSROccured == false) {
        ALOGE("Failed to detect SSR");
        mClient->addSSRCallback(nullptr);
    } else {
        ALOGI("Time diff between SSR trigger and SSR detection: [%ld ms]",
        clock() * 1000 / CLOCKS_PER_SEC - mClock);
    }

    // After SSR need to reset spcom client
    mIsMessagingValid = false;
    mSSROccured = false;

    int32_t ret = SUCCESS;

    if(mSharedBuffer != nullptr) {

    	// Verify copy operations on shared buffer fails
		int32_t err = mSharedBuffer->copyFromSpu();
		if(err == 0) {
			ALOGE("Copy from spu should fail after SSR");
			ret = MESSAGING_ERROR;
		}

		err = mSharedBuffer->copyToSpu();
		if(err == 0) {
			ALOGE("Copy to spu should fail after SSR");
			ret = MESSAGING_ERROR;
		}

		// Release shared buffer
		ALOGD("Clear SPU shared buffer");
		mSharedBuffer.clear();
		if(mSharedBuffer.get() != nullptr) {
			ALOGE("Shared buffer wasn't released");
			ret = GENERAL_ERROR;
		}
    }

    // Release client
    if (mClient != nullptr) {
        ALOGD("Clear SPU client");
        mClient->unregisterClient();
        mClient.clear();
        if(mClient.get() != nullptr) {
            ALOGE("Spcom client wasn't released");
            ret = GENERAL_ERROR;
        }
    }

    if(ret != SUCCESS) {
        return ret;
    }

    // Wait for SPU link up
    bool linkStatus = mSpuManager->waitForLinkUp(sDefaultTimeotMs);
    if (linkStatus != true) {
        ALOGE("SPU not linked up");
        return RESOURCE_BUSY;
    }

    return SUCCESS;
}
