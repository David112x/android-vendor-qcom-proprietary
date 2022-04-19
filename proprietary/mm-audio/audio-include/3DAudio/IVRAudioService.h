/* Copyright (c) 2016-2018 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
*/


#ifndef ANDROID_IVRAUDIOSERVICE_H
#define ANDROID_IVRAUDIOSERVICE_H

#include <utils/RefBase.h>
#include <utils/String8.h>
#include <utils/String16.h>
#include <binder/IInterface.h>
#include <binder/Parcel.h>

namespace android {

class IVRAudioWorld;
class IVRAudioWorldClient;
class IVRAudioService: public IInterface
{
public:
    DECLARE_META_INTERFACE(VRAudioService)
    virtual sp<IVRAudioWorld>
        createVRAudioWorld(const sp<IVRAudioWorldClient> &client,
                           int32_t renderMode) = 0;
};

// ----------------------------------------------------------------------------

class BnVRAudioService: public BnInterface<IVRAudioService>
{
public:
    virtual status_t    onTransact( uint32_t code,
                                    const Parcel& data,
                                    Parcel* reply,
                                    uint32_t flags = 0);
};

} // namespace android

#endif // ANDROID_IVRSERVICE_H
