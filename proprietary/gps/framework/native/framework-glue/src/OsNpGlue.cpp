/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright  (c) 2015 - 2016 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.
=============================================================================*/

#define LOG_TAG "OsNpGlue"
#define LOG_NDEBUG 0

#include <algorithm>
#include <list>
#include <comdef.h>
#include <IzatTypes.h>
#include <IzatRequest.h>
#include <IzatLocation.h>
#include <IIzatListener.h>
#include <OsNpGlue.h>
#include <loc_pla.h>
#include <log_util.h>

using namespace std;
using namespace izat_manager;

OsNpGlue * OsNpGlue::mInstance = NULL;

OsNpGlue * OsNpGlue::getInstance() {
#ifdef __ANDROID__
    if (!mInstance) {
        mInstance = new OsNpGlue();
    }
#endif
    return mInstance;
}

void OsNpGlue::destroyInstance() {
    OsNpGlue * tmp = mInstance;
    mInstance = nullptr;
    if (tmp) delete tmp;
}

void OsNpGlue::onLocationChanged(IzatLocation * location) {
    list<IIzatListener *>::iterator it = mListeners.begin();
    for ( ; it != mListeners.end(); ++it) {
       (*it)->onLocationChanged(location, IZAT_LOCATION_STATUS_FINAL);
    }
}

void OsNpGlue::onStatusChanged(IzatProviderStatus status) {
    list<IIzatListener *>::iterator it = mListeners.begin();
    for ( ; it != mListeners.end(); ++it) {
       (*it)->onStatusChanged(status);
    }
}

int32 OsNpGlue::subscribe(IIzatListener * izatListener) {
    if (izatListener) {
        mListeners.push_back(izatListener);
        mListeners.unique();
    }
    return 0;
}

int32 OsNpGlue::setRequest(const IzatRequest * request)
{
    int rc = -1;
#ifdef __ANDROID__
    if (request && mOsNp != NULL) {
        rc = mOsNp->onSetRequest(request);
    }
#endif
    return rc;
}

void OsNpGlue::stopRequest()
{
#ifdef __ANDROID__
    if (mOsNp != NULL) {
        mOsNp->onStopRequest();
    }
#endif
}

int32 OsNpGlue::unsubscribe(IIzatListener * izatListener) {
    if(izatListener) {
        list<IIzatListener *>::iterator it =
            find(mListeners.begin(), mListeners.end(), izatListener);
        if (it != mListeners.end()) {
            mListeners.erase(it);
        }
    }
    return 0;
}
