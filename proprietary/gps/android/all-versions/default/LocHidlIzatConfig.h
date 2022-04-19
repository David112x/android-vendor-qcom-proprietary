/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef LOC_HIDL_VERSION
#error "LOC_HIDL_VERSION must be set before including this file."
#endif

#include <vendor/qti/gnss/3.0/ILocHidlIzatConfig.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

#include <string>

namespace vendor {
namespace qti {
namespace gnss {
namespace LOC_HIDL_VERSION {
namespace implementation {

using ::android::hidl::base::V1_0::DebugInfo;
using ::android::hidl::base::V1_0::IBase;
using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::hardware::hidl_death_recipient;
using ::android::sp;
using ::android::wp;

using ::vendor::qti::gnss::V3_0::ILocHidlIzatConfig;
using ::vendor::qti::gnss::V3_0::ILocHidlIzatConfigCallback;

struct LocHidlIzatConfig : public ILocHidlIzatConfig {

    LocHidlIzatConfig();
    ~LocHidlIzatConfig();

    // Methods from ILocHidlIzatConfig follow.
    Return<bool> init(const sp<ILocHidlIzatConfigCallback>& callback) override;
    Return<bool> readConfig() override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

private:
    sp<ILocHidlIzatConfigCallback> mCallbackIface = nullptr;
    sp<LocHidlDeathRecipient> mDeathRecipient = nullptr;

    bool readIzatConf(std::string& izatConfContent);
};

extern "C" ILocHidlIzatConfig* HIDL_FETCH_ILocHidlIzatConfig(const char* name);

}  // namespace implementation
}  // namespace LOC_HIDL_VERSION
}  // namespace gnss
}  // namespace qti
}  // namespace vendor
