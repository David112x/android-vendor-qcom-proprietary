/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef LOC_HIDL_VERSION
#error "LOC_HIDL_VERSION must be set before including this file."
#endif

#include <log_util.h>
#include "loc_cfg.h"
#include <gps_extended_c.h>
#include <LocationAPI.h>
#include <izat_remote_api.h>
#include <IzatRemoteApi.h>
#include <IzatTypes.h>
#include "izat_wifi_db_provider.h"
#include "IzatWiFiDBProvider.h"

#include <stdio.h>

using namespace izat_remote_api;
using namespace izat_manager;

namespace vendor {
namespace qti {
namespace gnss {
namespace LOC_HIDL_VERSION {
namespace implementation {


LocHidlIzatConfig::LocHidlIzatConfig() : mCallbackIface(nullptr), mDeathRecipient(nullptr) {

    ENTRY_LOG();
}

LocHidlIzatConfig::~LocHidlIzatConfig() {

    ENTRY_LOG();
}

// Methods from ILocHidlIzatConfig follow.
static sp<ILocHidlIzatConfigCallback> sCallback = nullptr;
static uint64_t deathCount = 0;
Return<bool> LocHidlIzatConfig::init(const sp<ILocHidlIzatConfigCallback>& callback) {

    FROM_HIDL_CLIENT();

    if (callback == nullptr) {
        LOC_LOGE("Null callback on Init !");
    }

    // Register death recipient
    if (mDeathRecipient == nullptr) {
        mDeathRecipient = new LocHidlDeathRecipient([this] {
                LOC_LOGE("%s]: ILocHidlIzatConfigCallback died.", __func__);
                sCallback = nullptr;
                });
    }
    mDeathRecipient->registerToPeer(callback, deathCount++);
    mCallbackIface = callback;

    return true;
}

Return<bool> LocHidlIzatConfig::readConfig() {

    FROM_HIDL_CLIENT();

    // Sanity check
    if (mCallbackIface == nullptr) {
        LOC_LOGE("mCallbackIface NULL");
        return false;
    }

    // Read izat.conf file
    string izatConfContent;
    bool success = readIzatConf(izatConfContent);
    if (!success) {
        LOC_LOGE("Failed to read izat.conf");
        return false;
    }

    // Invoke callback
    TO_HIDL_CLIENT();
    auto r = mCallbackIface->izatConfigCallback(izatConfContent);
    if (!r.isOk()) {
        LOC_LOGE("%s] Error from izatConfigCallback description=%s",
                 __func__, r.description().c_str());
        return false;
    }

    return true;
}

bool LocHidlIzatConfig::readIzatConf(std::string& izatConfContent) {

    std::FILE *fp = fopen(LOC_PATH_IZAT_CONF, "r");

    if(NULL != fp) {
        std::fseek(fp, 0, SEEK_END);
        izatConfContent.resize(std::ftell(fp));
        std::rewind(fp);
        std::fread(&izatConfContent[0], 1, izatConfContent.size(), fp);
        std::fclose(fp);
        return true;
    } else {
        LOC_LOGE("Failed to open izat.conf");
    }

    return false;
}

// Methods from ::android::hidl::base::V1_0::IBase follow.

ILocHidlIzatConfig* HIDL_FETCH_ILocHidlIzatConfig(const char* /* name */) {
    return new LocHidlIzatConfig();
}

}  // namespace implementation
}  // namespace LOC_HIDL_VERSION
}  // namespace gnss
}  // namespace qti
}  // namespace vendor
