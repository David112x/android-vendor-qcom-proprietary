/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef VENDOR_QTI_GNSS_V2_1_LOCHIDLGNSSCONFIGSERVICE_H
#define VENDOR_QTI_GNSS_V2_1_LOCHIDLGNSSCONFIGSERVICE_H

#include "LocHidlUtils.h"

#define LOC_HIDL_VERSION V2_1

#include <vendor/qti/gnss/2.1/ILocHidlGnssConfigService.h>
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

using ::vendor::qti::gnss::V2_1::ILocHidlGnssConfigService;
using ::vendor::qti::gnss::V2_1::ILocHidlGnssConfigServiceCallback;
using ::android::hardware::gnss::V1_0::GnssConstellationType;

struct LocHidlGnssConfigService : public ILocHidlGnssConfigService {
    LocHidlGnssConfigService();
    virtual ~LocHidlGnssConfigService();
    // Methods from ::vendor::qti::gnss::V2_1::ILocHidlGnssConfigService follow.
    Return<bool> init(const sp<ILocHidlGnssConfigServiceCallback>& callback) override;
    Return<void> getGnssSvTypeConfig() override;
    Return<void> setGnssSvTypeConfig(const hidl_vec<GnssConstellationType>& disabledSvTypeVec) override;
    Return<void> resetGnssSvTypeConfig() override;

private:
    const GnssInterface* getGnssInterface();
    static void getGnssSvTypeConfigCallback(const GnssSvTypeConfig& config);

    static void svTypeVecToConfig(const std::vector<GnssConstellationType>& disabledSvTypeVec, GnssSvTypeConfig& config);
    static void svTypeConfigToVec(const GnssSvTypeConfig& config, std::vector<GnssConstellationType>& disabledSvTypeVec);

    static GnssConstellationType intToGnssConstellation(int32_t svTypeInt);
    static int32_t gnssConstellationToInt(GnssConstellationType svType);

private:
    const GnssInterface* mGnssInterface = nullptr;
    bool mGetGnssInterfaceFailed = false;
    static sp<LocHidlDeathRecipient> mDeathRecipient;
    static sp<ILocHidlGnssConfigServiceCallback> sCallbackIface;
};

}  // namespace implementation
}  // namespace LOC_HIDL_VERSION
}  // namespace gnss
}  // namespace qti
}  // namespace vendor

#undef LOC_HIDL_VERSION

#endif  // VENDOR_QTI_GNSS_V2_1_LOCHIDLGNSSCONFIGSERVICE_H
