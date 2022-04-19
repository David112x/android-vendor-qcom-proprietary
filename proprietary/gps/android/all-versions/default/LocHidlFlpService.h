/*
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef LOC_HIDL_VERSION
#error "LOC_HIDL_VERSION must be set before including this file."
#endif

#include <vendor/qti/gnss/1.1/ILocHidlFlpService.h>
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

using ::vendor::qti::gnss::V1_1::ILocHidlFlpService;
using ::vendor::qti::gnss::V1_0::ILocHidlFlpServiceCallback;

struct LocHidlFlpService : public ILocHidlFlpService {
    LocHidlFlpService();
    virtual ~LocHidlFlpService();
    // Methods from ::vendor::qti::gnss::V1_1::ILocHidlFlpService follow.
    Return<bool> init(const sp<ILocHidlFlpServiceCallback>& callback) override;
    Return<uint32_t> getAllSupportedFeatures() override;
    Return<int32_t> getAllBatchedLocations(int32_t sessionId) override;
    Return<int32_t> stopFlpSession(int32_t sessionId) override;
    Return<int32_t> startFlpSession(int32_t id, uint32_t flags, int64_t minIntervalNanos,
            int32_t minDistanceMetres, uint32_t tripDistanceMeters) override;
    Return<int32_t> updateFlpSession(int32_t id, uint32_t flags, int64_t minIntervalNanos,
            int32_t minDistanceMetres, uint32_t tripDistanceMeters) override;
    Return<void> deleteAidingData(int64_t flags) override;
    Return<void> getMaxPowerAllocated() override;
    Return<void> updateXtraThrottle(bool enabled) override;

private:
    static void powerStatusCallback(bool isOn);

private:
    LocHidlFlpClient* mIzatFlpApiClient = nullptr;
    sp<ILocHidlFlpServiceCallback> mCallbackIface = nullptr;
    typedef void* (getLocationInterface)();
    GnssInterface* mGnssInterface = nullptr;
    bool mGetGnssInterfaceFailed = false;
    GnssInterface* getGnssInterface();
    static sp<LocHidlDeathRecipient> mDeathRecipient;
    static sp<ILocHidlFlpServiceCallback> sCallbackIface;
};

}  // namespace implementation
}  // namespace LOC_HIDL_VERSION
}  // namespace gnss
}  // namespace qti
}  // namespace vendor
