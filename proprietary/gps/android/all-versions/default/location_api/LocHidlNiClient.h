/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef LOC_HIDL_VERSION
#error "LOC_HIDL_VERSION must be set before including this file."
#endif

#include <vendor/qti/gnss/1.1/ILocHidlGnssNi.h>
#include <vendor/qti/gnss/1.0/ILocHidlGnssNiCallback.h>
#include <LocationAPI.h>
#include <LocationAPIClientBase.h>

namespace vendor {
namespace qti {
namespace gnss {
namespace LOC_HIDL_VERSION {
namespace implementation {

using ::android::sp;
using ::android::hardware::gnss::V1_0::IGnssNiCallback;

class LocHidlNiClient : public LocationAPIClientBase
{
public:
    LocHidlNiClient(const sp<::vendor::qti::gnss::V1_0::ILocHidlGnssNiCallback>& callback);
    virtual ~LocHidlNiClient() = default;

    void onCapabilitiesCb(LocationCapabilitiesMask capabilitiesMask) final;
    void onGnssNiCb(uint32_t id, GnssNiNotification gnssNiNotification) final;

    void gnssNiRespond(int32_t notifId,
            IGnssNiCallback::GnssUserResponseType userResponse);

private:
    sp<::vendor::qti::gnss::V1_0::ILocHidlGnssNiCallback> mGnssCbIface;
};

}  // namespace implementation
}  // namespace LOC_HIDL_VERSION
}  // namespace gnss
}  // namespace qti
}  // namespace vendor
