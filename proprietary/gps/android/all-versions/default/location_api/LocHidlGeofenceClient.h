/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef LOC_HIDL_VERSION
#error "LOC_HIDL_VERSION must be set before including this file."
#endif

#include <vendor/qti/gnss/1.1/ILocHidlGeofenceService.h>
#include <vendor/qti/gnss/1.0/ILocHidlGeofenceServiceCallback.h>
#include <LocationAPI.h>
#include <LocationAPIClientBase.h>

namespace vendor {
namespace qti {
namespace gnss {
namespace LOC_HIDL_VERSION {
namespace implementation {

using ::android::sp;

class LocHidlGeofenceClient : public LocationAPIClientBase
{
public:
    LocHidlGeofenceClient(
            const sp<::vendor::qti::gnss::V1_0::ILocHidlGeofenceServiceCallback>& callback);
    virtual ~LocHidlGeofenceClient() = default;

    void onGeofenceBreachCb(GeofenceBreachNotification geofenceBreachNotification) final;
    void onGeofenceStatusCb(GeofenceStatusNotification geofenceStatusNotification) final;
    void onAddGeofencesCb(size_t count, LocationError* errors, uint32_t* ids) final;
    void onRemoveGeofencesCb(size_t count, LocationError* errors, uint32_t* ids) final;
    void onPauseGeofencesCb(size_t count, LocationError* errors, uint32_t* ids) final;
    void onResumeGeofencesCb(size_t count, LocationError* errors, uint32_t* ids) final;

private:
    sp<::vendor::qti::gnss::V1_0::ILocHidlGeofenceServiceCallback> mGnssCbIface;
};

}  // namespace implementation
}  // namespace LOC_HIDL_VERSION
}  // namespace gnss
}  // namespace qti
}  // namespace vendor
