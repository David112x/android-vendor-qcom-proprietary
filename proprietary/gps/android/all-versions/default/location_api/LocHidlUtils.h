/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef LOC_HIDL_VERSION
#error "LOC_HIDL_VERSION must be set before including this file."
#endif

#include <vendor/qti/gnss/1.0/types.h>
#include <vendor/qti/gnss/1.1/types.h>
#include <LocationAPI.h>
#include <IzatLocation.h>
#include <IzatRequest.h>
#include <DataItemId.h>

namespace vendor {
namespace qti {
namespace gnss {
namespace LOC_HIDL_VERSION {
namespace implementation {

#define TO_HIDL_CLIENT()   ALOGV("[%s][%d] [HS] =>>>> [HC]", __func__, __LINE__)
#define FROM_HIDL_CLIENT() ALOGV("[%s][%d] [HS] <<<<= [HC]", __func__, __LINE__)

using namespace izat_manager;
using ::android::hardware::hidl_death_recipient;
using ::android::hidl::base::V1_0::IBase;
using ::android::sp;
using ::android::wp;

struct LocHidlDeathRecipient : hidl_death_recipient {
    LocHidlDeathRecipient(std::function<void(void)> fun) : mFun(fun), mPeerDied(false) {
    }
    LocHidlDeathRecipient() = delete;
    ~LocHidlDeathRecipient() = default;
    void registerToPeer(const sp<::android::hidl::base::V1_0::IBase>& peer, uint64_t cookie);
    void unregisterFromPeer(const sp<::android::hidl::base::V1_0::IBase>& peer);
    inline bool peerDied() { return mPeerDied; }
    virtual void serviceDied(uint64_t cookie,
            const ::android::wp<::android::hidl::base::V1_0::IBase>& who) override;
    std::function<void(void)> mFun;
    bool mPeerDied;
};

class LocHidlUtils {

public:
    static void locationToLocHidlLocation(
            const Location& location, V1_0::LocHidlLocation& gnssLocation);

    static void izatLocationToGnssIzatLocation(
            const IzatLocation& izatLocation,
            V1_0::LocHidlIzatLocation& gnssIzatLocation);

    inline static V1_0::LocHidlIzatStreamType izatStreamToGnssIzatStream(
            IzatStreamType ist) {
        return static_cast<V1_0::LocHidlIzatStreamType>(ist);
    }

    inline static IzatStreamType gnssIzatStreamToIzatStreamType(
            V1_0::LocHidlIzatStreamType gist) {
        return static_cast<IzatStreamType>(gist);
    }

    inline static V1_0::LocHidlIzatProviderStatus izatStatusToGnssIzatStatus(
            izat_manager::IzatProviderStatus ips) {
        return static_cast<V1_0::LocHidlIzatProviderStatus>(ips);
    }

    static DataItemId translateToDataItemId(V1_0::LocHidlSubscriptionDataItemId id);

    static V1_0::LocHidlSubscriptionDataItemId
    translateToSubscriptionDataItemId(DataItemId id);

    static void izatRequestToGnssIzatRequest(
            const IzatRequest& ir, V1_0::LocHidlIzatRequest& gir);

    static V1_1::LocHidlSubscriptionDataItemId_1_1
    translateToSubscriptionDataItemId_1_1(DataItemId id);

};

}  // namespace implementation
}  // namespace LOC_HIDL_VERSION
}  // namespace gnss
}  // namespace qti
}  // namespace vendor
