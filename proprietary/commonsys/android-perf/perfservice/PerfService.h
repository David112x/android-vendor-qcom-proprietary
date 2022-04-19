/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

#ifndef ANDROID_PERFSERVICE_H
#define ANDROID_PERFSERVICE_H

#include "IPerfManager.h"
#include <map>
#include <set>
#include <binder/BinderService.h>

using namespace std;

namespace android {

class PerfService :
    public BinderService<PerfService>,
    public BnPerfManager
{
public:
    PerfService();
    virtual int perfLockAcquire(int duration, int len, int* boostsList);
    virtual int perfHint(int hint, String16& userDataStr, int userData1, int userData2, int tid);
    virtual int perfLockRelease();
    virtual int perfLockReleaseHandler(int _handle);
    virtual int perfUXEngine_events(int opcode, int pid, String16& pkg_name, int lat);
    virtual int setClientBinder(const sp<IBinder>& client);

    class DeathNotifier : public IBinder::DeathRecipient {
    public:
        DeathNotifier(PerfService *perf) : mPerf(perf) {}
        virtual ~DeathNotifier();
        virtual void binderDied(const wp<IBinder>& who);
    private:
        PerfService *mPerf;
    };
    struct BinderInfo {
        int pid;
        sp<IBinder> binder;
        sp<DeathNotifier> notifier;

        bool operator < (const BinderInfo &bi) const {
            return pid<bi.pid;
        }
    };
    struct HandleInfo {
        int handle;
        int pid;
        int tid;
        int hintId;
        int hintType;

        bool operator < (const HandleInfo &hi) const {
            return handle<hi.handle;
        }
    };
private:
    Mutex mLock;
    set<BinderInfo> mBinderSet;
    set<HandleInfo> mHandleSet;
    int mHandle = 0;
    int (*perf_lock_acq)(int handle, int duration, int list[], int numArgs);
    int (*perf_lock_rel)(int handle);
    int (*perf_hint)(int, const char *, int, int);
    int (*perf_ux_engine_events)(int, int, const char *, int);
};

} // namespace android

#endif // ANDROID_PERFSERVICE_H
