/*
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#define LOG_TAG "LocSvc_HIDL_IzatWWANDBReceiver"
#define LOG_NDEBUG 0

#include "LocHidlWWANDBReceiver.h"

#define LOC_HIDL_VERSION V2_1
#include <log_util.h>
#include <gps_extended_c.h>
#include <LocationAPI.h>
#include <izat_remote_api.h>
#include <IzatRemoteApi.h>
#include <IzatTypes.h>
#include <string>
#include <wwan_db_receiver.h>

using namespace izat_remote_api;
using namespace izat_manager;

namespace vendor {
namespace qti {
namespace gnss {
namespace LOC_HIDL_VERSION {
namespace implementation {

using ::vendor::qti::gnss::V2_1::ILocHidlWWANDBReceiverCallback;
static sp<V1_2::ILocHidlWWANDBReceiverCallback> sCallback = nullptr;
static uint64_t deathCount = 0;

LocHidlWWANDBReceiver::LocHidlWWANDBReceiver() :
        mLBSAdapter(nullptr), mCallbackIface(nullptr) {

    ENTRY_LOG();
}

LocHidlWWANDBReceiver::~LocHidlWWANDBReceiver() {

    ENTRY_LOG();
}

static void DBReceiverBsListUpdateCb(BSInfo_s *bs_list, size_t bs_list_size, uint8_t status,
        const UlpLocation &location, void * /*clientData*/) {
    std::vector<ILocHidlWWANDBReceiverCallback::LocHidlBsInfoExt> bsInfoList;
    int32_t bsListSize = 0;

    LOC_LOGE("Native hidl BsList Update: BS list size %d", (int) bs_list_size);

    // Sanity check
    if (sCallback == nullptr) {
        LOC_LOGE("sCallback NULL");
        return;
    }

    for (size_t i = 0; i <(size_t) bs_list_size; i++) {
         ILocHidlWWANDBReceiverCallback::LocHidlBsInfoExt info;
         info.base_bsinfo_1_2.cell_type = bs_list[i].cell_type;
         info.base_bsinfo_1_2.cell_id1 = bs_list[i].cell_id1;
         info.base_bsinfo_1_2.cell_id2 = bs_list[i].cell_id2;
         info.base_bsinfo_1_2.cell_id3 = bs_list[i].cell_id3;
         info.base_bsinfo_1_2.cell_id4 = bs_list[i].cell_id4;
         info.timestamp = bs_list[i].cell_timestamp;
         bsInfoList.push_back(info);
    }
    bsListSize = (int32_t) bs_list_size;

    LocHidlUlpLocation ulpLoc;
    ulpLoc.size = location.size;
    ulpLoc.position_source = location.position_source;
    ulpLoc.tech_mask = location.tech_mask;
    ulpLoc.gpsLocation.locationFlagsMask = location.gpsLocation.flags;
    ulpLoc.gpsLocation.latitude = location.gpsLocation.latitude;
    ulpLoc.gpsLocation.longitude = location.gpsLocation.longitude;
    ulpLoc.gpsLocation.altitude = location.gpsLocation.altitude;
    ulpLoc.gpsLocation.speed = location.gpsLocation.speed;
    ulpLoc.gpsLocation.bearing = location.gpsLocation.bearing;
    ulpLoc.gpsLocation.accuracy = location.gpsLocation.accuracy;
    ulpLoc.gpsLocation.verticalAccuracy = location.gpsLocation.vertUncertainity;
    ulpLoc.gpsLocation.timestamp = location.gpsLocation.timestamp;

    sp<::vendor::qti::gnss::V2_1::ILocHidlWWANDBReceiverCallback> wwanDbRcvrCb =
        ::vendor::qti::gnss::V2_1::ILocHidlWWANDBReceiverCallback::castFrom(sCallback);
    if (wwanDbRcvrCb != nullptr) {
        LOC_LOGD("wwanDbRcvrCb V2_1 obtained");
        auto r = wwanDbRcvrCb->bsListUpdateCallback_2_1(bsInfoList, bsListSize, status, ulpLoc);
        if (!r.isOk()) {
            LOC_LOGE("%s] Error from bsListUpdateCallback_2_1 description=%s",
                     __func__, r.description().c_str());
        }
    }
}

static void DBReceiverStatusUpdateCb(bool status, const char* errStr, void* /*client data*/) {
    string reason;
    if ((false == status) && (nullptr != errStr)) {
        reason.assign(errStr);
    }

    // Sanity check
    if (sCallback == nullptr) {
        LOC_LOGE("sCallback NULL");
        return;
    }
    auto r = sCallback->statusUpdateCallback(status, reason);
    if (!r.isOk()) {
        LOC_LOGE("%s] Error from statusUpdateCallback description=%s",
                 __func__, r.description().c_str());
    }
}

static void DBReceiverServiceRequestCb(void* /*client data*/) {

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

static void DBReceiverEnvNotifyCb(void* /*client data*/) {

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

// Methods from ::vendor::qti::gnss::V1_2::IWWANDBReceiver follow.
Return<bool> LocHidlWWANDBReceiver::init(
        const sp<::vendor::qti::gnss::V1_2::ILocHidlWWANDBReceiverCallback>& callback) {

    FROM_HIDL_CLIENT();

    if (callback == nullptr) {
        LOC_LOGE("Null callback on Init !");
    }

    // Register death recipient
    if (mDeathRecipient == nullptr) {
        mDeathRecipient = new LocHidlDeathRecipient([this] {
                LOC_LOGE("%s]: LocHidlWWANDBReceiver died.", __func__);
                sCallback = nullptr;
                mCallbackIface = nullptr;
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
        mLBSAdapter = LBSAdapter::get(LOC_API_ADAPTER_BIT_FDCL_SERVICE_REQ);
        if (nullptr == mLBSAdapter) {
            LOC_LOGE("mLBSAdapter is NULL after get");
        }
    }

    return true;
}

Return<void> LocHidlWWANDBReceiver::registerWWANDBUpdater(
        const sp<::vendor::qti::gnss::V1_2::ILocHidlWWANDBReceiverCallback>& callback) {

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
        FdclCallbacks fdclCallbacks {DBReceiverBsListUpdateCb,
                                     DBReceiverStatusUpdateCb,
                                     DBReceiverServiceRequestCb,
                                     DBReceiverEnvNotifyCb};
        mLBSAdapter->registerFdclCommand(fdclCallbacks, clientData);
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

Return<void> LocHidlWWANDBReceiver::unregisterWWANDBUpdater() {

    FROM_HIDL_CLIENT();

    if (nullptr != mLBSAdapter) {
        mLBSAdapter->unregisterFdclCommand();
    } else {
        LOC_LOGE("mLBSAdapter is NULL");
    }
    sCallback = nullptr;

    return Void();
}

Return<void> LocHidlWWANDBReceiver::sendBSListRequest(int32_t expireInDays) {

    FROM_HIDL_CLIENT();

    if (nullptr != mLBSAdapter) {
        mLBSAdapter->requestFdclDataCommand(expireInDays);
    } else {
        LOC_LOGE("mLBSAdapter is NULL");
    }

    return Void();
}

Return<void> LocHidlWWANDBReceiver::pushBSWWANDB(
        const hidl_vec<LocHidlBsLocationData>& bsLocationDataList,
        int16_t bsLocationDataListSize,
        const hidl_vec<LocHidlBsSpecialInfo>& bsSpecialInfoList,
        int16_t bsSpecialInfoListSize, int32_t daysValid) {

    FROM_HIDL_CLIENT();

    LOC_LOGE("Native hidl BsList size %d BsSpecial size %d",
              (int) bsLocationDataListSize, (int) bsSpecialInfoListSize);

    FdclData fdclData = {};
    fdclData.daysValid = daysValid;
    for (size_t i = 0; i <(size_t) bsLocationDataListSize; i++) {
        BSLocationData_s l_ad;
        l_ad.cellType = bsLocationDataList[i].cellType;
        l_ad.cellRegionID1 = bsLocationDataList[i].cellRegionID1;
        l_ad.cellRegionID2 = bsLocationDataList[i].cellRegionID2;
        l_ad.cellRegionID3 = bsLocationDataList[i].cellRegionID3;
        l_ad.cellRegionID4 = bsLocationDataList[i].cellRegionID4;
        l_ad.latitude = bsLocationDataList[i].latitude;
        l_ad.longitude = bsLocationDataList[i].longitude;
        l_ad.valid_bits = bsLocationDataList[i].valid_bits;
        l_ad.horizontal_coverage_radius = bsLocationDataList[i].horizontal_coverage_radius;
        l_ad.horizontal_confidence = bsLocationDataList[i].horizontal_confidence;
        l_ad.horizontal_reliability = bsLocationDataList[i].horizontal_reliability;
        l_ad.altitude = bsLocationDataList[i].altitude;
        l_ad.altitude_uncertainty = bsLocationDataList[i].altitude_uncertainty;
        l_ad.altitude_confidence = bsLocationDataList[i].altitude_confidence;
        l_ad.altitude_reliability = bsLocationDataList[i].altitude_reliability;

        fdclData.bsLocationDataList.push_back(l_ad);
    }

    for (size_t i = 0; i < (size_t) bsSpecialInfoListSize; i++) {
        BSSpecialInfo_s s_ad;
        s_ad.cellType = bsSpecialInfoList[i].cellType;
        s_ad.cellRegionID1 = bsSpecialInfoList[i].cellRegionID1;
        s_ad.cellRegionID2 = bsSpecialInfoList[i].cellRegionID2;
        s_ad.cellRegionID3 = bsSpecialInfoList[i].cellRegionID3;
        s_ad.cellRegionID4 = bsSpecialInfoList[i].cellRegionID4;
        s_ad.info = bsSpecialInfoList[i].info;
        fdclData.bsSpecialInfoList.push_back(s_ad);
    }

    if (nullptr != mLBSAdapter) {
        mLBSAdapter->injectFdclDataCommand(fdclData);
    } else {
        LOC_LOGE("mLBSAdapter is NULL");
    }

    return Void();
}

// Methods from ::vendor::qti::gnss::V2_1::IWWANDBReceiver follow.
Return<bool> LocHidlWWANDBReceiver::init_2_1(
        const sp<::vendor::qti::gnss::V2_1::ILocHidlWWANDBReceiverCallback>& callback) {

    FROM_HIDL_CLIENT();

    if (callback == nullptr) {
        LOC_LOGE("Null callback on Init !");
    }

    // Register death recipient
    if (mDeathRecipient == nullptr) {
        mDeathRecipient = new LocHidlDeathRecipient([this] {
                LOC_LOGE("%s]: LocHidlWWANDBReceiver died.", __func__);
                sCallback = nullptr;
                mCallbackIface = nullptr;
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
        mLBSAdapter = LBSAdapter::get(LOC_API_ADAPTER_BIT_FDCL_SERVICE_REQ);
        if (nullptr == mLBSAdapter) {
            LOC_LOGE("mLBSAdapter is NULL after get");
        }
    }
    return true;
}
Return<void> LocHidlWWANDBReceiver::registerWWANDBUpdater_2_1  (
        const sp<::vendor::qti::gnss::V2_1::ILocHidlWWANDBReceiverCallback>& callback) {

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
        FdclCallbacks fdclCallbacks {DBReceiverBsListUpdateCb,
                                     DBReceiverStatusUpdateCb,
                                     DBReceiverServiceRequestCb,
                                     DBReceiverEnvNotifyCb};
        mLBSAdapter->registerFdclCommand(fdclCallbacks, clientData);
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

// Methods from ::android::hidl::base::V1_2::IBase follow.

}  // namespace implementation
}  // namespace LOC_HIDL_VERSION
}  // namespace gnss
}  // namespace qti
}  // namespace vendor

#undef LOC_HIDL_VERSION
