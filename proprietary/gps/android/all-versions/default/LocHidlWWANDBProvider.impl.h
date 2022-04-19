/*
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef LOC_HIDL_VERSION
#error "LOC_HIDL_VERSION must be set before including this file."
#endif

#include <log_util.h>
#include <gps_extended_c.h>
#include <LocationAPI.h>
#include <izat_remote_api.h>
#include <IzatRemoteApi.h>
#include <IzatTypes.h>
#include <string>
#include "wwan_db_provider.h"

using namespace izat_remote_api;
using namespace izat_manager;

namespace vendor {
namespace qti {
namespace gnss {
namespace LOC_HIDL_VERSION {
namespace implementation {

using ::vendor::qti::gnss::V2_1::ILocHidlWWANDBProviderCallback;
static sp<ILocHidlWWANDBProviderCallback> sCallback = nullptr;
static uint64_t deathCount = 0;

LocHidlWWANDBProvider::LocHidlWWANDBProvider() :
         mLBSAdapter(nullptr), mCallbackIface(nullptr) {
    ENTRY_LOG();
}

LocHidlWWANDBProvider::~LocHidlWWANDBProvider() {
    ENTRY_LOG();
}

// Methods from ILocHidlWWANDBProvider follow.
Return<bool> LocHidlWWANDBProvider::init(const sp<ILocHidlWWANDBProviderCallback>& callback) {
    FROM_HIDL_CLIENT();

    if (callback == nullptr) {
        LOC_LOGE("Null callback on Init !");
    }

    // Register death recipient
    if (mDeathRecipient == nullptr) {
        mDeathRecipient = new LocHidlDeathRecipient([this] {
                LOC_LOGE("%s]: LocHidlWWANDBProvider died.", __func__);
                sCallback = nullptr;
                });
    }

    if (mCallbackIface != nullptr && mDeathRecipient != nullptr) {
        mDeathRecipient->unregisterFromPeer(mCallbackIface);
    }
    mCallbackIface = callback;
    if (mCallbackIface != nullptr && mDeathRecipient != nullptr) {
        mDeathRecipient->registerToPeer(mCallbackIface, deathCount++);
    }

    if (nullptr == mLBSAdapter) {
        mLBSAdapter = LBSAdapter::get(LOC_API_ADAPTER_BIT_BS_OBS_DATA_SERVICE_REQ);
        if (nullptr == mLBSAdapter) {
            LOC_LOGE("mLBSAdapter is NULL after get");
        }
    }

    return true;
}

static void DBProvObsLocDataUpdateCb(BSObsData_s *bs_obs_data, size_t bs_obs_data_size,
        int bsListStatus, void */*clientData*/) {
    std::vector<ILocHidlWWANDBProviderCallback::LocHidlBSObsData> bsObsDataList;
    int32_t bsObsListSize = 0;

    LOC_LOGE("Native hidl ObsBsList Update: BS list size %d", (int) bs_obs_data_size);

    // Sanity check
    if (sCallback == nullptr) {
        LOC_LOGE("sCallback NULL");
        return;
    }

    for (size_t i = 0; i <(size_t) bs_obs_data_size; i++) {
         ILocHidlWWANDBProviderCallback::LocHidlBSObsData obsData;

         obsData.scanTimestamp_ms = bs_obs_data[i].scanTimestamp_ms;
         obsData.bUlpLocValid = bs_obs_data[i].bUlpLocValid;
         memset(&obsData.gpsLoc, 0, sizeof(obsData.gpsLoc));
         if (obsData.bUlpLocValid) {
            obsData.gpsLoc.size = bs_obs_data[i].gpsLoc.size;
            obsData.gpsLoc.position_source = bs_obs_data[i].gpsLoc.position_source;
            obsData.gpsLoc.tech_mask = bs_obs_data[i].gpsLoc.tech_mask;
            obsData.gpsLoc.gpsLocation.locationFlagsMask = bs_obs_data[i].gpsLoc.gpsLocation.flags;
            obsData.gpsLoc.gpsLocation.latitude = bs_obs_data[i].gpsLoc.gpsLocation.latitude;
            obsData.gpsLoc.gpsLocation.longitude = bs_obs_data[i].gpsLoc.gpsLocation.longitude;
            obsData.gpsLoc.gpsLocation.altitude = bs_obs_data[i].gpsLoc.gpsLocation.altitude;
            obsData.gpsLoc.gpsLocation.speed = bs_obs_data[i].gpsLoc.gpsLocation.speed;
            obsData.gpsLoc.gpsLocation.bearing = bs_obs_data[i].gpsLoc.gpsLocation.bearing;
            obsData.gpsLoc.gpsLocation.speedAccuracy = bs_obs_data[i].speed_unc;
            obsData.gpsLoc.gpsLocation.bearingAccuracy = bs_obs_data[i].bearing_unc;
            obsData.gpsLoc.gpsLocation.accuracy = bs_obs_data[i].gpsLoc.gpsLocation.accuracy;
            obsData.gpsLoc.gpsLocation.verticalAccuracy =
                    bs_obs_data[i].gpsLoc.gpsLocation.vertUncertainity;
            obsData.gpsLoc.gpsLocation.timestamp = bs_obs_data[i].gpsLoc.gpsLocation.timestamp;
         }
         obsData.cellInfo.cell_type = bs_obs_data[i].cellInfo.cell_type;
         obsData.cellInfo.cell_id1 = bs_obs_data[i].cellInfo.cell_id1;
         obsData.cellInfo.cell_id2 = bs_obs_data[i].cellInfo.cell_id2;
         obsData.cellInfo.cell_id3 = bs_obs_data[i].cellInfo.cell_id3;
         obsData.cellInfo.cell_id4 = bs_obs_data[i].cellInfo.cell_id4;

         bsObsDataList.push_back(obsData);
    }
    bsObsListSize = (int32_t) bs_obs_data_size;
    auto r = sCallback->bsObsLocDataUpdateCallback(bsObsDataList, bsObsListSize, bsListStatus);
    if (!r.isOk()) {
        LOC_LOGE("%s] Error from bsListUpdateCallback description=%s",
                 __func__, r.description().c_str());
    }
}

static void DBProvServiceRequestCb(void */*clientData*/) {
    // Sanity check
    if (sCallback == nullptr) {
        LOC_LOGE("sCallback NULL");
        return;
    }
    auto r = sCallback->serviceRequestCallback();
    if (!r.isOk()) {
        LOC_LOGE("%s] Error from serviceRequestCallback description=%s",
                 __func__, r.description().c_str());
    }
}

static void DBProvEnvNotifyCb(void */*clientData*/) {
    // Sanity check
    if (sCallback == nullptr) {
        LOC_LOGE("sCallback NULL");
        return;
    }
    auto r = sCallback->attachVmOnCallback();
    if (!r.isOk()) {
        LOC_LOGE("%s] Error from attachVmOnCallback description=%s",
                 __func__, r.description().c_str());
    }
}

Return<void> LocHidlWWANDBProvider::registerWWANDBProvider(const sp<ILocHidlWWANDBProviderCallback>& callback) {
    FROM_HIDL_CLIENT();

    sCallback = callback;

    if (callback == nullptr) {
        LOC_LOGE("Null callback on registering null !");
    }

    if (sCallback == nullptr) {
        LOC_LOGE("Null sCallback on registering null !");
    }

    void *clientData = NULL;
    if (nullptr != mLBSAdapter) {
        CellCSCallbacks cellCsCallBacks {DBProvObsLocDataUpdateCb,
                                         DBProvServiceRequestCb,
                                         DBProvEnvNotifyCb};
        mLBSAdapter->registerCellCSCommand(cellCsCallBacks, clientData);
    } else {
        LOC_LOGE("mLBSAdapter is NULL");
    }

    if (mCallbackIface != nullptr && mDeathRecipient != nullptr) {
        mDeathRecipient->unregisterFromPeer(mCallbackIface);
    }
    mCallbackIface = callback;
    if (mCallbackIface != nullptr && mDeathRecipient != nullptr) {
        mDeathRecipient->registerToPeer(mCallbackIface, deathCount++);
    }
    return Void();
}

Return<void> LocHidlWWANDBProvider::unregisterWWANDBProvider() {
    if (nullptr != mLBSAdapter) {
        LOC_LOGE("mLBSAdapter is not NULL");
        mLBSAdapter->unregisterCellCSCommand();
    } else {
        LOC_LOGE("mLBSAdapter is NULL");
    }
    sCallback = nullptr;
    return Void();
}

Return<void> LocHidlWWANDBProvider::sendBSObsLocDataRequest() {
    FROM_HIDL_CLIENT();

    if (nullptr != mLBSAdapter) {
        LOC_LOGE("mLBSAdapter is not NULL");
        mLBSAdapter->requestCellCSDataCommand();
    } else {
        LOC_LOGE("mLBSAdapter is NULL");
    }

    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

ILocHidlWWANDBProvider* HIDL_FETCH_ILocHidlWWANDBProvider(const char* /* name */) {
    return new LocHidlWWANDBProvider();
}

}  // namespace implementation
}  // namespace LOC_HIDL_VERSION
}  // namespace gnss
}  // namespace qti
}  // namespace vendor
