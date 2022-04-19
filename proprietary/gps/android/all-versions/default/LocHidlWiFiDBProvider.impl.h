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
#include "izat_wifi_db_provider.h"
#include "IzatWiFiDBProvider.h"

using namespace izat_remote_api;
using namespace izat_manager;

namespace vendor {
namespace qti {
namespace gnss {
namespace LOC_HIDL_VERSION {
namespace implementation {

using ::vendor::qti::gnss::V2_1::LocHidlWifiDBListStatus;

// ======== WiFiDBProviderWrapper ========
class WiFiDBProviderWrapper : public WiFiDBProvider {
    const wifiDBProvObsLocDataUpdateCb mApObsListUpdateCb;
    const wifiDBProvServiceRequestCb mServiceReqCb;
    const wifiDBProvEnvNotifyCb mEnvNotifyCb;
    const void *mClientData;
public:
    inline WiFiDBProviderWrapper(wifiDBProvObsLocDataUpdateCb apObsListUpdateCb,
                                wifiDBProvServiceRequestCb serviceReqCb,
                                wifiDBProvEnvNotifyCb envNotifyCb,
                                void *clientData) :
        WiFiDBProvider(),
        mApObsListUpdateCb(apObsListUpdateCb),
        mServiceReqCb(serviceReqCb),
        mEnvNotifyCb(envNotifyCb),
        mClientData(clientData) {
    }

    inline virtual void onAPObsLocDataAvailable(std::vector<APObsData>* ap_obsdata_ptr,
            const int apListStatus) override {
        if (nullptr != ap_obsdata_ptr) {
            bool completeList = true;
            std::vector<APObsData>& ap_obsdata_info = *ap_obsdata_ptr;

            APObsData_s **ap_obsdatalist_arr = new APObsData_s *[ap_obsdata_info.size()];
            if (ap_obsdatalist_arr!=NULL) {
                for (size_t ii = 0; ii < ap_obsdata_info.size(); ++ii) {
                    ap_obsdatalist_arr[ii] = new APObsData_s;
                    if (ap_obsdatalist_arr[ii]!=NULL) {
                        ap_obsdatalist_arr[ii]->scanTimestamp_ms =
                                ap_obsdata_info[ii].scanTimestamp_ms;
                        ap_obsdatalist_arr[ii]->bUlpLocValid =
                                ap_obsdata_info[ii].bUlpLocValid;
                        if (ap_obsdata_info[ii].bUlpLocValid) {
                            // if bUlpLocValid is true only, then fill up ulplocation
                            ap_obsdatalist_arr[ii]->gpsLoc.size =
                                    ap_obsdata_info[ii].gpsLoc.size;
                            ap_obsdatalist_arr[ii]->gpsLoc.position_source =
                                    ap_obsdata_info[ii].gpsLoc.position_source;
                            ap_obsdatalist_arr[ii]->gpsLoc.tech_mask =
                                    ap_obsdata_info[ii].gpsLoc.tech_mask;
                            ap_obsdatalist_arr[ii]->gpsLoc.gpsLocation.flags =
                                    ap_obsdata_info[ii].gpsLoc.gpsLocation.flags;
                            ap_obsdatalist_arr[ii]->gpsLoc.gpsLocation.latitude =
                                    ap_obsdata_info[ii].gpsLoc.gpsLocation.latitude;
                            ap_obsdatalist_arr[ii]->gpsLoc.gpsLocation.longitude =
                                    ap_obsdata_info[ii].gpsLoc.gpsLocation.longitude;
                            ap_obsdatalist_arr[ii]->gpsLoc.gpsLocation.altitude =
                                    ap_obsdata_info[ii].gpsLoc.gpsLocation.altitude;
                            ap_obsdatalist_arr[ii]->gpsLoc.gpsLocation.speed =
                                    ap_obsdata_info[ii].gpsLoc.gpsLocation.speed;
                            ap_obsdatalist_arr[ii]->gpsLoc.gpsLocation.bearing =
                                    ap_obsdata_info[ii].gpsLoc.gpsLocation.bearing;
                            ap_obsdatalist_arr[ii]->gpsLoc.gpsLocation.accuracy =
                                    ap_obsdata_info[ii].gpsLoc.gpsLocation.accuracy;
                            ap_obsdatalist_arr[ii]->gpsLoc.gpsLocation.vertUncertainity =
                                    ap_obsdata_info[ii].gpsLoc.gpsLocation.vertUncertainity;
                            ap_obsdatalist_arr[ii]->gpsLoc.gpsLocation.timestamp =
                                    ap_obsdata_info[ii].gpsLoc.gpsLocation.timestamp;
                        }
                        ap_obsdatalist_arr[ii]->cellInfo.cell_type =
                                ap_obsdata_info[ii].cellInfo.cellType;
                        ap_obsdatalist_arr[ii]->cellInfo.cell_id1 =
                                ap_obsdata_info[ii].cellInfo.cellRegionID1;
                        ap_obsdatalist_arr[ii]->cellInfo.cell_id2 =
                                ap_obsdata_info[ii].cellInfo.cellRegionID2;
                        ap_obsdatalist_arr[ii]->cellInfo.cell_id3 =
                                ap_obsdata_info[ii].cellInfo.cellRegionID3;
                        ap_obsdatalist_arr[ii]->cellInfo.cell_id4 =
                                ap_obsdata_info[ii].cellInfo.cellRegionID4;

                        ap_obsdatalist_arr[ii]->ap_scan_info_size =
                                ap_obsdata_info[ii].ap_scan_info.size();
                        size_t scan_list_len = (size_t) ap_obsdatalist_arr[ii]->ap_scan_info_size;

                        for (size_t jj = 0; jj < scan_list_len; jj++) {
                            APScanInfo_s ap_scan_info;
                            ap_scan_info.mac_R48b =
                                    ap_obsdata_info[ii].ap_scan_info[jj].mac_R48b;
                            ap_scan_info.rssi =
                                    ap_obsdata_info[ii].ap_scan_info[jj].rssi;
                            ap_scan_info.age_usec =
                                    ap_obsdata_info[ii].ap_scan_info[jj].age_usec;
                            ap_scan_info.channel_id =
                                    ap_obsdata_info[ii].ap_scan_info[jj].channel_id;
                            memcpy(ap_scan_info.ssid,
                                        ap_obsdata_info[ii].ap_scan_info[jj].ssid,
                                            sizeof(ap_scan_info.ssid));
                            ap_scan_info.ssid_valid_byte_count =
                                    ap_obsdata_info[ii].ap_scan_info[jj].ssid_valid_byte_count;

                            ap_obsdatalist_arr[ii]->ap_scan_info.push_back(ap_scan_info);
                        }
                    } else {
                        completeList=false;
                        break;
                    }
                }
                if (true == completeList) {
                    mApObsListUpdateCb(ap_obsdatalist_arr, ap_obsdata_info.size(), apListStatus,
                            (void *)mClientData);
                }
                for (size_t ii = 0; ii < ap_obsdata_info.size(); ++ii) {
                    if (ap_obsdatalist_arr[ii]!=NULL) {
                        delete ap_obsdatalist_arr[ii];
                    }
                }
                delete[] ap_obsdatalist_arr;
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

LocHidlWiFiDBProvider::LocHidlWiFiDBProvider() :
        mIzatWiFiDBProviderApiClient(nullptr), mCallbackIface(nullptr) {

    ENTRY_LOG();
}

LocHidlWiFiDBProvider::~LocHidlWiFiDBProvider() {

    ENTRY_LOG();

    if (mIzatWiFiDBProviderApiClient!=nullptr) {
        delete (WiFiDBProviderWrapper*)mIzatWiFiDBProviderApiClient;
        mIzatWiFiDBProviderApiClient = nullptr;
    }
}

// Methods from ILocHidlWiFiDBProvider follow.
static sp<ILocHidlWiFiDBProviderCallback> sCallback = nullptr;
static uint64_t deathCount = 0;
Return<bool> LocHidlWiFiDBProvider::init(const sp<ILocHidlWiFiDBProviderCallback>& callback) {

    FROM_HIDL_CLIENT();

    if (callback == nullptr) {
        LOC_LOGE("Null callback on Init !");
    }

    // Register death recipient
    if (mDeathRecipient == nullptr) {
        mDeathRecipient = new LocHidlDeathRecipient([this] {
                LOC_LOGE("%s]: ILocHidlWiFiDBProviderCallback died.", __func__);
                sCallback = nullptr;
                });
    }
    mDeathRecipient->registerToPeer(callback, deathCount++);
    mCallbackIface = callback;

    return true;
}

static void DBProviderApListUpdateCb(APObsData_s *ap_obs_data[], size_t ap_obs_data_size,
        int apListStatus, void * /*clientData*/ ) {
    std::vector<ILocHidlWiFiDBProviderCallback::LocHidlApObsData> apObsDataList;
    int32_t apListSize = 0;

    LOC_LOGE("Native hidl ApList Update: AP list size: %d apListStatus: %u", (int) ap_obs_data_size, apListStatus);

    // Sanity check
    if (sCallback == nullptr) {
        LOC_LOGE("sCallback NULL");
        return;
    }

    for (size_t ii = 0; ii <(size_t) ap_obs_data_size; ii++) {
         ILocHidlWiFiDBProviderCallback::LocHidlApObsData info;
         info.scanTimestamp_ms = ap_obs_data[ii]->scanTimestamp_ms;
         info.bUlpLocValid = ap_obs_data[ii]->bUlpLocValid;
         if (info.bUlpLocValid) {
            // if bUlpLocValid is true only, then fill up ulplocation
            info.gpsLoc.size = ap_obs_data[ii]->gpsLoc.size;
            info.gpsLoc.position_source = ap_obs_data[ii]->gpsLoc.position_source;
            info.gpsLoc.tech_mask = ap_obs_data[ii]->gpsLoc.tech_mask;
            info.gpsLoc.gpsLocation.locationFlagsMask = ap_obs_data[ii]->gpsLoc.gpsLocation.flags;
            info.gpsLoc.gpsLocation.latitude = ap_obs_data[ii]->gpsLoc.gpsLocation.latitude;
            info.gpsLoc.gpsLocation.longitude = ap_obs_data[ii]->gpsLoc.gpsLocation.longitude;
            info.gpsLoc.gpsLocation.altitude = ap_obs_data[ii]->gpsLoc.gpsLocation.altitude;
            info.gpsLoc.gpsLocation.speed = ap_obs_data[ii]->gpsLoc.gpsLocation.speed;
            info.gpsLoc.gpsLocation.bearing = ap_obs_data[ii]->gpsLoc.gpsLocation.bearing;
            info.gpsLoc.gpsLocation.accuracy = ap_obs_data[ii]->gpsLoc.gpsLocation.accuracy;
            info.gpsLoc.gpsLocation.verticalAccuracy =
                    ap_obs_data[ii]->gpsLoc.gpsLocation.vertUncertainity;
            info.gpsLoc.gpsLocation.timestamp = ap_obs_data[ii]->gpsLoc.gpsLocation.timestamp;
            info.gpsLoc.gpsLocation.locationTechnologyMask = ap_obs_data[ii]->gpsLoc.tech_mask;
         }
         info.cellInfo.cell_type = ap_obs_data[ii]->cellInfo.cell_type;
         info.cellInfo.cell_id1 = ap_obs_data[ii]->cellInfo.cell_id1;
         info.cellInfo.cell_id2 = ap_obs_data[ii]->cellInfo.cell_id2;
         info.cellInfo.cell_id3 = ap_obs_data[ii]->cellInfo.cell_id3;
         info.cellInfo.cell_id4 = ap_obs_data[ii]->cellInfo.cell_id4;

         std::vector<ILocHidlWiFiDBProviderCallback::LocHidlApScanData> apScanDataList;
         info.ap_scan_info_size = (uint32_t) ap_obs_data[ii]->ap_scan_info_size;
         for (size_t jj = 0; jj <(size_t) info.ap_scan_info_size; jj++) {
            ILocHidlWiFiDBProviderCallback::LocHidlApScanData scanData;
            scanData.mac_R48b = ap_obs_data[ii]->ap_scan_info[jj].mac_R48b;
            scanData.rssi = ap_obs_data[ii]->ap_scan_info[jj].rssi;
            scanData.age_usec = ap_obs_data[ii]->ap_scan_info[jj].age_usec;

            scanData.channel_id = ap_obs_data[ii]->ap_scan_info[jj].channel_id;
            scanData.ssid = std::string(ap_obs_data[ii]->ap_scan_info[jj].ssid);
            scanData.ssid_valid_byte_count =
                    ap_obs_data[ii]->ap_scan_info[jj].ssid_valid_byte_count;
            apScanDataList.push_back(scanData);
         }
         info.ap_scan_info = apScanDataList;

         apObsDataList.push_back(info);
    }
    apListSize = (int32_t) ap_obs_data_size;

    sCallback->apObsLocDataUpdateCallback(apObsDataList, apListSize,
            (LocHidlWifiDBListStatus)apListStatus);
}

static void DBProviderServiceRequestCb(void* /*client data*/) {

    // Sanity check
    if (sCallback == nullptr) {
        LOC_LOGE("sCallback NULL");
        return;
    }
    sCallback->serviceRequestCallback();
}

static void DBProviderEnvNotifyCb(void* /*client data*/) {

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

Return<void> LocHidlWiFiDBProvider::registerWiFiDBProvider(
        const sp<ILocHidlWiFiDBProviderCallback>& callback) {
    FROM_HIDL_CLIENT();

    sCallback = callback;

    if (callback == nullptr) {
        LOC_LOGE("Null callback on registering null !");
    }

    if (sCallback == nullptr) {
        LOC_LOGE("Null sCallback on registering null !");
    }

    void *clientData = NULL;
    // Clean up previous wrapper
    if (mIzatWiFiDBProviderApiClient != nullptr) {
        delete (WiFiDBProviderWrapper*)mIzatWiFiDBProviderApiClient;
    }

    mIzatWiFiDBProviderApiClient = new WiFiDBProviderWrapper(DBProviderApListUpdateCb,
                                                            DBProviderServiceRequestCb,
                                                            DBProviderEnvNotifyCb, clientData);
    if (mCallbackIface != nullptr && mDeathRecipient != nullptr) {
        mDeathRecipient->unregisterFromPeer(mCallbackIface);
    }
    mCallbackIface = callback;
    if (mCallbackIface != nullptr && mDeathRecipient != nullptr) {
        mDeathRecipient->registerToPeer(mCallbackIface, deathCount++);
    }
    return Void();
}

Return<void> LocHidlWiFiDBProvider::unregisterWiFiDBProvider() {
    if (mIzatWiFiDBProviderApiClient != nullptr) {
        delete (WiFiDBProviderWrapper*)mIzatWiFiDBProviderApiClient;
        mIzatWiFiDBProviderApiClient = nullptr;
    }
    sCallback = nullptr;
    return Void();
}

Return<void> LocHidlWiFiDBProvider::sendAPObsLocDataRequest() {
    FROM_HIDL_CLIENT();

    if (mIzatWiFiDBProviderApiClient == nullptr) {
        LOC_LOGE("WiFiDBProvider API Client null !");
        return Void();
    }
   ((WiFiDBProviderWrapper *)mIzatWiFiDBProviderApiClient)->sendAPOBSLocDataReq();

    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

ILocHidlWiFiDBProvider* HIDL_FETCH_ILocHidlWiFiDBProvider(const char* /* name */) {
    return new LocHidlWiFiDBProvider();
}

}  // namespace implementation
}  // namespace LOC_HIDL_VERSION
}  // namespace gnss
}  // namespace qti
}  // namespace vendor
