/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef LOC_HIDL_VERSION
#error "LOC_HIDL_VERSION must be set before including this file."
#endif

#include <log_util.h>

namespace vendor {
namespace qti {
namespace gnss {
namespace LOC_HIDL_VERSION {
namespace implementation {

LocHidlGnssNi::LocHidlGnssNi() : mNiClient(nullptr), mCallbackIface(nullptr) {

    ENTRY_LOG();
}

LocHidlGnssNi::~LocHidlGnssNi() {

    ENTRY_LOG();
    if (mNiClient != nullptr) {
        delete mNiClient;
        mNiClient = nullptr;
    }
}

// Methods from ::vendor::qti::gnss::V1_0::ILocHidlGnssNi follow.
Return<void> LocHidlGnssNi::setVendorCallback(
        const sp<ILocHidlGnssNiCallback>& callback) {
    static uint64_t deathCount = 0;
    FROM_HIDL_CLIENT();

    if (mNiClient != nullptr) {
        LOC_LOGE("mNiClient not null !");
        return Void();
    }
    mNiClient = new LocHidlNiClient(callback);

    // Register death recipient
    if (mDeathRecipient == nullptr) {
        mDeathRecipient = new LocHidlDeathRecipient([this] {
                LOC_LOGE("%s]: ILocHidlGnssNiCallback died.", __func__);
                if (mNiClient != nullptr) {
                    delete mNiClient;
                    mNiClient = nullptr;
                }
                });
    }
    mDeathRecipient->registerToPeer(callback, deathCount++);
    mCallbackIface = callback;

    return Void();
}

Return<void> LocHidlGnssNi::respond(int32_t notifId,
        IGnssNiCallback::GnssUserResponseType userResponse) {

    FROM_HIDL_CLIENT();

    if (mNiClient == nullptr) {
        LOC_LOGE("IZAT API Client null !");
    return Void();
    }


    mNiClient->gnssNiRespond(notifId, userResponse);
    return Void();
}


// Methods from ::android::hidl::base::V1_0::IBase follow.

}  // namespace implementation
}  // namespace LOC_HIDL_VERSION
}  // namespace gnss
}  // namespace qti
}  // namespace vendor
