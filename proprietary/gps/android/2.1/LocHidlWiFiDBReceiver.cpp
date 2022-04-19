/*
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#define LOG_TAG "LocSvc_HIDL_IzatWiFiDBReceiver"
#define LOG_NDEBUG 0

#include "LocHidlWiFiDBReceiver.h"

#define LOC_HIDL_VERSION V2_1
#include <log_util.h>
#include <gps_extended_c.h>
#include <LocationAPI.h>
#include <izat_remote_api.h>
#include <IzatRemoteApi.h>
#include <IzatTypes.h>
#include <string>
#include <izat_wifi_db_receiver.h>
#include <IzatWiFiDBReceiver.h>

using namespace izat_remote_api;
using namespace izat_manager;

namespace vendor {
namespace qti {
namespace gnss {
namespace LOC_HIDL_VERSION {
namespace implementation {

// ======== WiFiDBUpdater ========
class WiFiDBUpdaterWrapper : public WiFiDBUpdater {
    const wifiDBRecvApListUpdateCb mApListAvailCb;
    const wifiDBRecvStatusUpdateCb mStatusCb;
    const wifiDBRecvServiceRequestCb mServiceReqCb;
    const wifiDBRecvEnvNotifyCb mEnvNotifyCb;
    const void *mClientData;
public:
    inline WiFiDBUpdaterWrapper(wifiDBRecvApListUpdateCb apListAvailCb,
                                wifiDBRecvStatusUpdateCb statusCb,
                                wifiDBRecvServiceRequestCb serviceReqCb,
                                wifiDBRecvEnvNotifyCb envNotifyCb,
                                void *clientData) :
        WiFiDBUpdater(),
        mApListAvailCb(apListAvailCb),
        mStatusCb(statusCb),
        mServiceReqCb(serviceReqCb),
        mEnvNotifyCb(envNotifyCb),
        mClientData(clientData) {
        }
    inline virtual void statusUpdate(bool isSuccess, const char *errStr) override {
        mStatusCb(isSuccess, errStr, (void *)mClientData);
    }
    inline virtual void apListUpdate(std::vector<APInfo>* ap_list_ptr, const int apListStatus,
            const UlpLocation& ulpLoc, const bool bUlpLocValid) override {
        if (nullptr != ap_list_ptr) {
            bool completeList = true;
            std::vector<APInfo>& ap_list = *ap_list_ptr;
            APInfo_s **ap_list_arr = new APInfo_s *[ap_list.size()];
            if (ap_list_arr!=NULL) {
                for (size_t ii = 0; ii < ap_list.size(); ++ii) {
                    ap_list_arr[ii] = new APInfo_s;
                    if (ap_list_arr[ii]!=NULL) {
                        ap_list_arr[ii]->mac_R48b = ap_list[ii].mac_R48b;
                        ap_list_arr[ii]->cell_type = ap_list[ii].cellType;
                        ap_list_arr[ii]->cell_id1 = ap_list[ii].cellRegionID1;
                        ap_list_arr[ii]->cell_id2 = ap_list[ii].cellRegionID2;
                        ap_list_arr[ii]->cell_id3 = ap_list[ii].cellRegionID3;
                        ap_list_arr[ii]->cell_id4 = ap_list[ii].cellRegionID4;
                        memcpy(ap_list_arr[ii]->ssid, ap_list[ii].ssid,
                               sizeof(ap_list_arr[ii]->ssid));
                        ap_list_arr[ii]->ssid_valid_byte_count = ap_list[ii].ssid_valid_byte_count;
                        ap_list_arr[ii]->timestamp = ap_list[ii].utcTime;
                        ap_list_arr[ii]->fdal_status = ap_list[ii].fdal_status;
                    } else {
                        completeList=false;
                        break;
                    }
                }
                if (true == completeList) {
                    mApListAvailCb(ap_list_arr, ap_list.size(), apListStatus, ulpLoc, bUlpLocValid,
                            (void *)mClientData);
                }
                for (size_t ii = 0; ii < ap_list.size(); ++ii) {
                    if (ap_list_arr[ii]!=NULL) {
                        delete ap_list_arr[ii];
                    }
                }
                delete[] ap_list_arr;
            }
        }
    }
    inline virtual void serviceRequest() override {
        mServiceReqCb((void *)mClientData);
    }
    inline virtual void notifyCallbackEnv() override {
        if (nullptr != mEnvNotifyCb) {
            mEnvNotifyCb((void *)mClientData);
        }
    }
};

static sp<V1_0::ILocHidlWiFiDBReceiverCallback> sCallback = nullptr;
static uint64_t deathCount = 0;

LocHidlWiFiDBReceiver::LocHidlWiFiDBReceiver() :
        mIzatWiFiDBReceiverApiClient(nullptr), mCallbackIface(nullptr) {

    ENTRY_LOG();
}

LocHidlWiFiDBReceiver::~LocHidlWiFiDBReceiver() {

    ENTRY_LOG();

    if (mIzatWiFiDBReceiverApiClient!=nullptr) {
        delete (WiFiDBUpdaterWrapper*)mIzatWiFiDBReceiverApiClient;
        mIzatWiFiDBReceiverApiClient = nullptr;
    }
}

static void DBReceiverApListUpdateCb(APInfo_s *ap_list[], size_t ap_list_size,
        int apListStatus, const UlpLocation& gpsLoc, bool bUlpLocValid, void * /*clientData*/) {
    std::vector<ILocHidlWiFiDBReceiverCallback::LocHidlApInfoExt> apInfoList;
    int32_t apListSize = 0;

    LOC_LOGE("Native hidl ApList Update: AP list size %d", (int) ap_list_size);

    // Sanity check
    if (sCallback == nullptr) {
        LOC_LOGE("sCallback NULL");
        return;
    }

    for (size_t ii = 0; ii <(size_t) ap_list_size; ii++) {
         ILocHidlWiFiDBReceiverCallback::LocHidlApInfoExt info;
         info.base_apinfo_1_0.mac_R48b = ap_list[ii]->mac_R48b;
         info.base_apinfo_1_0.cell_type = ap_list[ii]->cell_type;
         info.base_apinfo_1_0.cell_id1 = ap_list[ii]->cell_id1;
         info.base_apinfo_1_0.cell_id2 = ap_list[ii]->cell_id2;
         info.base_apinfo_1_0.cell_id3 = ap_list[ii]->cell_id3;
         info.base_apinfo_1_0.cell_id4 = ap_list[ii]->cell_id4;
         info.base_apinfo_1_0.ssid = std::string(ap_list[ii]->ssid);
         info.base_apinfo_1_0.ssid_valid_byte_count = ap_list[ii]->ssid_valid_byte_count;
         info.utc_time = ap_list[ii]->timestamp;
         info.fdal_status = ap_list[ii]->fdal_status;
         apInfoList.push_back(info);
    }
    apListSize = (int32_t) ap_list_size;

    LocHidlUlpLocation ulpLoc;
    ulpLoc.size = gpsLoc.size;
    ulpLoc.position_source = gpsLoc.position_source;
    ulpLoc.tech_mask = gpsLoc.tech_mask;
    ulpLoc.gpsLocation.locationFlagsMask = gpsLoc.gpsLocation.flags;
    ulpLoc.gpsLocation.latitude = gpsLoc.gpsLocation.latitude;
    ulpLoc.gpsLocation.longitude = gpsLoc.gpsLocation.longitude;
    ulpLoc.gpsLocation.altitude = gpsLoc.gpsLocation.altitude;
    ulpLoc.gpsLocation.speed = gpsLoc.gpsLocation.speed;
    ulpLoc.gpsLocation.bearing = gpsLoc.gpsLocation.bearing;
    ulpLoc.gpsLocation.accuracy = gpsLoc.gpsLocation.accuracy;
    ulpLoc.gpsLocation.verticalAccuracy = gpsLoc.gpsLocation.vertUncertainity;
    ulpLoc.gpsLocation.timestamp = gpsLoc.gpsLocation.timestamp;
    ulpLoc.gpsLocation.locationTechnologyMask = ulpLoc.tech_mask;

    sp<::vendor::qti::gnss::V2_1::ILocHidlWiFiDBReceiverCallback> wifiDbRcvrCb =
        ::vendor::qti::gnss::V2_1::ILocHidlWiFiDBReceiverCallback::castFrom(sCallback);
    if (wifiDbRcvrCb != nullptr) {
        LOC_LOGD("wifiDbRcvrCb V2_1 obtained");
        wifiDbRcvrCb->apListUpdateCallback_2_1(apInfoList, apListSize,
                (LocHidlWifiDBListStatus)apListStatus, ulpLoc, bUlpLocValid);
    }
}

static void DBReceiverStatusUpdateCb(bool status, const char* errStr, void* /*client data*/) {
    string reason;
    if ((status == false) && (errStr != nullptr)) {
        reason.assign(errStr);
    }

    // Sanity check
    if (sCallback == nullptr) {
        LOC_LOGE("sCallback NULL");
        return;
    }
    sCallback->statusUpdateCallback(status, reason);
}

static void DBReceiverServiceRequestCb(void* /*client data*/) {

    // Sanity check
    if (sCallback == nullptr) {
        LOC_LOGE("sCallback NULL");
        return;
    }
    sCallback->serviceRequestCallback();
}

static void DBReceiverEnvNotifyCb(void* /*client data*/) {

    // Sanity check
    if (sCallback == nullptr) {
        LOC_LOGE("sCallback NULL");
        return;
    }
    sCallback->attachVmOnCallback();
}

// Methods from ::vendor::qti::gnss::V1_0::ILocHidlWiFiDBReceiver follow.
Return<bool> LocHidlWiFiDBReceiver::init(
        const sp<::vendor::qti::gnss::V1_0::ILocHidlWiFiDBReceiverCallback>& callback) {

    FROM_HIDL_CLIENT();


    if (mIzatWiFiDBReceiverApiClient != nullptr) {
        LOC_LOGE("mIzatWiFiDBReceiverApiClient not null !");
        return false;
    }

    if (callback == nullptr) {
        LOC_LOGE("Null callback on Init !");
    }

    // Register death recipient
    if (mDeathRecipient == nullptr) {
        mDeathRecipient = new LocHidlDeathRecipient([this] {
                LOC_LOGE("%s]: ILocHidlWiFiDBReceiverCallback died.", __func__);
                sCallback = nullptr;
                if (mIzatWiFiDBReceiverApiClient != nullptr) {
                    delete (WiFiDBUpdaterWrapper*)mIzatWiFiDBReceiverApiClient;
                    mIzatWiFiDBReceiverApiClient = nullptr;
                }
                });
    }
    mDeathRecipient->registerToPeer(callback, deathCount++);
    mCallbackIface = callback;

    return true;
}


Return<void> LocHidlWiFiDBReceiver::registerWiFiDBUpdater(
        const sp<::vendor::qti::gnss::V1_0::ILocHidlWiFiDBReceiverCallback>& callback) {

    FROM_HIDL_CLIENT();

    if (mIzatWiFiDBReceiverApiClient != nullptr) {
        LOC_LOGE("mIzatWiFiDBReceiverApiClient not null !");
    }

    sCallback = callback;

    if (callback == nullptr) {
        LOC_LOGE("Null callback on registering null !");
    }

    if (sCallback == nullptr) {
        LOC_LOGE("Null sCallback on registering null !");
    }

    void *clientData = NULL;
    // Clean up previous wrapper
    if (mIzatWiFiDBReceiverApiClient != nullptr) {
        delete (WiFiDBUpdaterWrapper*)mIzatWiFiDBReceiverApiClient;
    }
    mIzatWiFiDBReceiverApiClient = new WiFiDBUpdaterWrapper(DBReceiverApListUpdateCb,
                                                            DBReceiverStatusUpdateCb,
                                                            DBReceiverServiceRequestCb,
                                                            DBReceiverEnvNotifyCb, clientData);

    if (mCallbackIface != nullptr && mDeathRecipient != nullptr) {
        mDeathRecipient->unregisterFromPeer(mCallbackIface);
    }
    mCallbackIface = callback;
    if (mCallbackIface != nullptr && mDeathRecipient != nullptr) {
        mDeathRecipient->registerToPeer(mCallbackIface, deathCount++);
    }
    return Void();
}

Return<void> LocHidlWiFiDBReceiver::unregisterWiFiDBUpdater() {
    FROM_HIDL_CLIENT();

    if (mIzatWiFiDBReceiverApiClient != nullptr) {
        delete (WiFiDBUpdaterWrapper*)mIzatWiFiDBReceiverApiClient;
        mIzatWiFiDBReceiverApiClient = nullptr;
    }
    sCallback = nullptr;

    return Void();
}

Return<void> LocHidlWiFiDBReceiver::sendAPListRequest(int32_t expireInDays) {
    FROM_HIDL_CLIENT();

    if (mIzatWiFiDBReceiverApiClient == nullptr) {
        LOC_LOGE("WiFiDBReceiver API Client null !");
        return Void();
    }

    ((WiFiDBUpdaterWrapper *)mIzatWiFiDBReceiverApiClient)->sendAPListReq(expireInDays);

    return Void();
}

Return<void> LocHidlWiFiDBReceiver::pushAPWiFiDB(
        const hidl_vec<LocHidlApLocationData>& apLocationDataList,
        int16_t apLocationDataListSize,
        const hidl_vec<LocHidlApSpecialInfo>& apSpecialInfoList,
        int16_t apSpecialInfoListSize, int32_t daysValid) {

    return pushAPWiFiDB_2_1(apLocationDataList, apLocationDataListSize, apSpecialInfoList,
                            apSpecialInfoListSize, daysValid, false);
}

Return<void> LocHidlWiFiDBReceiver::pushAPWiFiDB_2_1(
        const hidl_vec<LocHidlApLocationData>& apLocationDataList,
        int16_t apLocationDataListSize,
        const hidl_vec<LocHidlApSpecialInfo>& apSpecialInfoList,
        int16_t apSpecialInfoListSize, int32_t daysValid, bool is_lookup) {

    FROM_HIDL_CLIENT();

    if (mIzatWiFiDBReceiverApiClient == nullptr) {
        LOC_LOGE("WiFiDBReceiver API Client null !");
        return Void();
    }

    LOC_LOGE("Native hidl ApList size %d ApSpecial size %d",
              (int) apLocationDataListSize, (int) apSpecialInfoListSize);

    std::vector<WiFiDBUpdater::APLocationData> loc_list;
    std::vector<WiFiDBUpdater::APSpecialInfo> ap_list;

    for (size_t ii = 0; ii <(size_t) apLocationDataListSize; ii++) {
        WiFiDBUpdater::APLocationData l_ad;
        l_ad.mac_R48b = apLocationDataList[ii].mac_R48b;
        l_ad.latitude = apLocationDataList[ii].latitude;
        l_ad.longitude = apLocationDataList[ii].longitude;
        l_ad.max_antena_range = apLocationDataList[ii].max_antenna_range;
        l_ad.horizontal_error = apLocationDataList[ii].horizontal_error;
        l_ad.reliability = apLocationDataList[ii].reliability;
        l_ad.valid_mask = apLocationDataList[ii].valid_bits;
        loc_list.push_back(l_ad);
    }

    for (size_t ii = 0; ii < (size_t) apSpecialInfoListSize; ii++) {
        WiFiDBUpdater::APSpecialInfo s_ad;
        s_ad.mac_R48b = apSpecialInfoList[ii].mac_R48b;
        s_ad.info = apSpecialInfoList[ii].info;
        ap_list.push_back(s_ad);
    }

    ((WiFiDBUpdaterWrapper*)mIzatWiFiDBReceiverApiClient)->pushWiFiDB(
            &loc_list, &ap_list, daysValid, is_lookup);
    return Void();
}

// Methods from ::vendor::qti::gnss::V2_1::ILocHidlWiFiDBReceiver follow.
Return<void> LocHidlWiFiDBReceiver::sendScanListRequest() {
    FROM_HIDL_CLIENT();

    if (mIzatWiFiDBReceiverApiClient == nullptr) {
        LOC_LOGE("WiFiDBReceiver API Client null !");
        return Void();
    }
    ((WiFiDBUpdaterWrapper *)mIzatWiFiDBReceiverApiClient)->sendScanListReq();

    return Void();
}

}  // namespace implementation
}  // namespace LOC_HIDL_VERSION
}  // namespace gnss
}  // namespace qti
}  // namespace vendor

#undef LOC_HIDL_VERSION
