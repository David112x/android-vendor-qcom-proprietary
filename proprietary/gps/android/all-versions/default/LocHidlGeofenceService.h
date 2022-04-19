/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef LOC_HIDL_VERSION
#error "LOC_HIDL_VERSION must be set before including this file."
#endif

#include <vendor/qti/gnss/1.1/ILocHidlGeofenceService.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <location_interface.h>

namespace vendor {
namespace qti {
namespace gnss {
namespace LOC_HIDL_VERSION {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

using ::vendor::qti::gnss::V1_1::ILocHidlGeofenceService;
using ::vendor::qti::gnss::V1_0::ILocHidlGeofenceServiceCallback;

struct LocHidlGeofenceService : public ILocHidlGeofenceService {
    LocHidlGeofenceService();
    virtual ~LocHidlGeofenceService();
    // Methods from ::vendor::qti::gnss::V1_1::ILocHidlGeofenceService follow.
    Return<bool> init(const sp<ILocHidlGeofenceServiceCallback>& callback) override;
    Return<void> addGeofence(int32_t id, double latitude, double longitude, double radius,
            uint32_t transitionTypes, int32_t responsiveness, int32_t confidence,
            int32_t dwellTime, uint32_t dwellTimeMask) override;
    Return<void> removeGeofence(int32_t id) override;
    Return<void> updateGeofence(int32_t id, uint32_t transitionTypes,
            int32_t responsiveness) override;
    Return<void> pauseGeofence(int32_t id) override;
    Return<void> resumeGeofence(int32_t id, uint32_t transitionTypes) override;

private:
    // This method is not part of the ILocHidl base class.
    // It is called by LocHidlGeofenceServiceDeathRecipient to remove all geofences added so far.
    Return<void> removeAllGeofences();

private:
    LocHidlGeofenceClient* mIzatGeofenceApiClient = nullptr;
    sp<ILocHidlGeofenceServiceCallback> mCallbackIface = nullptr;
    sp<LocHidlDeathRecipient> mDeathRecipient = nullptr;
};

}  // namespace implementation
}  // namespace LOC_HIDL_VERSION
}  // namespace gnss
}  // namespace qti
}  // namespace vendor
