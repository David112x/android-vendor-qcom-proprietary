/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#ifndef ANDROID_IPERFMANAGER_H
#define ANDROID_IPERFMANAGER_H

#include <stdint.h>
#include <sys/types.h>

#include <utils/RefBase.h>
#include <utils/Errors.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

namespace android {

class IPerfManager : public IInterface
{
public:
    DECLARE_META_INTERFACE(PerfManager);
    virtual int perfLockAcquire(int duration, int len, int* boostsList) = 0;
    virtual int perfHint(int hint, String16& userDataStr, int userData1, int userData2, int tid) = 0;
    virtual int perfLockRelease() = 0;
    virtual int perfLockReleaseHandler(int _handle) = 0;
    virtual int perfUXEngine_events(int opcode, int pid, String16& pkg_name, int lat) = 0;
    virtual int setClientBinder(const sp<IBinder>& client) = 0;

    enum {
        PERF_LOCK_RELEASE = IBinder::FIRST_CALL_TRANSACTION,
        PERF_LOCK_RELEASE_HANDLER = IBinder::FIRST_CALL_TRANSACTION + 1,
        PERF_HINT = IBinder::FIRST_CALL_TRANSACTION + 2,
        PERF_LOCK_ACQUIRE = IBinder::FIRST_CALL_TRANSACTION + 3,
        PERF_UX_ENGINE_EVENTS = IBinder::FIRST_CALL_TRANSACTION + 4,
        SET_CLIENT_BINDER = IBinder::FIRST_CALL_TRANSACTION + 5,
    };
};

class BnPerfManager : public BnInterface<IPerfManager>
{
public:
    virtual status_t onTransact(uint32_t code,
                            const Parcel& data,
                            Parcel* reply,
                            uint32_t flags = 0);
};
}; //namespace android
#endif //ANDROID_IPERFMANAGER_H
