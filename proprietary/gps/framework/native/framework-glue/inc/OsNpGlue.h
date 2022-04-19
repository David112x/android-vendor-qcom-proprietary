/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

GENERAL DESCRIPTION
  Network Location Provider Glue

  Copyright (c) 2015-2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

#ifndef __OSNPGLUE_H__
#define __OSNPGLUE_H__

#include <list>
#include <comdef.h>
#include <IzatTypes.h>
#include <IOSLocationProvider.h>

#ifdef __ANDROID__
#include <utils/RefBase.h>
#endif
namespace izat_manager {
class IzatRequest;
class IzatLocation;
class IIzatListener;
}

using namespace std;
using namespace izat_manager;
#ifdef __ANDROID__
using android::sp;
struct IOsNpGlueCallback : public android::RefBase {
    IOsNpGlueCallback() = default;
    virtual ~IOsNpGlueCallback() = default;
    virtual int32 onSetRequest(const IzatRequest* request) = 0;
    virtual void onStopRequest() = 0;
};
#endif

class OsNpGlue : public IOSLocationProvider {
public:
    static OsNpGlue * getInstance();
    static void destroyInstance();
    inline static OsNpGlue * getHandle() { return mInstance; }

    void onLocationChanged(IzatLocation * location);
    void onStatusChanged(IzatProviderStatus status);

    // override IOSLocationProvider
    int32 subscribe(IIzatListener * izatListener);
    int32 unsubscribe(IIzatListener * izatListener);

    int32 setRequest(const IzatRequest * request);
    void stopRequest();
#ifdef __ANDROID__
    inline void setOsNp(const sp<IOsNpGlueCallback>& osnp) {
        mOsNp = osnp;
    }
#endif

private:
    inline OsNpGlue() {
#ifdef __ANDROID__
        mOsNp = nullptr;
#endif
    }
    inline ~OsNpGlue() {}

    static OsNpGlue * mInstance;
    list <IIzatListener *> mListeners;
#ifdef __ANDROID__
    sp<IOsNpGlueCallback> mOsNp;
#endif
};

#endif // #ifndef __FUSEDLOCATIONPROVIDERGLUE_H__

