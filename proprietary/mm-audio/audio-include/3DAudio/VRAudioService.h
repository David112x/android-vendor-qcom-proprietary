/* Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
*/


#pragma once

#include <cutils/compiler.h>
#include <utils/Thread.h>
#include <utils/KeyedVector.h>

#include <binder/MemoryDealer.h>
#include <binder/BinderService.h>
#include "IVRAudioService.h"

class IVRAudioWorldClient;
class VRAudioServiceImpl;
class VRAudioWorld;

namespace android {

class VRAudioServiceNative :
    public BinderService<VRAudioServiceNative>,
    public BnVRAudioService
{
public:
    static const char* getServiceName() ANDROID_API { return "vendor.audio.vrservice"; }
    virtual sp<IVRAudioWorld>
            createVRAudioWorld(const sp<IVRAudioWorldClient> &client,
                               int32_t renderMode);
    status_t destroyVRAudioWorld(int16_t id);
    sp<IMemory> allocGlobalState(size_t allocSize);

    virtual     status_t    dump(int fd, const Vector<String16>& args);

private:
    VRAudioServiceNative();
    virtual ~VRAudioServiceNative();

    VRAudioServiceImpl *mVRAudioServiceImpl;
    friend class BinderService<VRAudioServiceNative>;
    KeyedVector<int16_t, sp<VRAudioWorld>> mWorlds;
    int16_t mNextWorldId;
    Mutex mLock;
    sp<MemoryDealer> mDealer;

    void onVRAudioWorldClientDied(uint32_t id);
    struct VRAudioWorldClientDeathListener;
    KeyedVector<uint32_t, sp<VRAudioWorldClientDeathListener>> mDeathListeners;
};

} //namespace android
