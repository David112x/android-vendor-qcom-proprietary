/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef LOC_HIDL_VERSION
#error "LOC_HIDL_VERSION must be set before including this file."
#endif

#include <vendor/qti/gnss/1.1/ILocHidlIzatProvider.h>
#include <vendor/qti/gnss/1.0/ILocHidlIzatProviderCallback.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <IzatTypes.h>
#include <IIzatManager.h>
#include <IOSListener.h>
#include <IzatRequest.h>

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

using ::vendor::qti::gnss::V1_1::ILocHidlIzatProvider;

using namespace izat_manager;

class Listener;
struct LocHidlIzatProvider : public ILocHidlIzatProvider {
    LocHidlIzatProvider(::vendor::qti::gnss::V1_0::LocHidlIzatStreamType privderType);
    virtual inline ~LocHidlIzatProvider() { mDeathRecipient = nullptr; }
    // Methods from ::vendor::qti::gnss::V1_1::ILocHidlIzatProvider follow.
    Return<bool> init(
            const sp<::vendor::qti::gnss::V1_0::ILocHidlIzatProviderCallback>& callback) override;
    Return<void> deinit() override;
    Return<bool> onEnable() override;
    Return<bool> onDisable() override;
    Return<bool> onAddRequest(
            const ::vendor::qti::gnss::V1_0::LocHidlIzatRequest& request) override;
    Return<bool> onRemoveRequest(
            const ::vendor::qti::gnss::V1_0::LocHidlIzatRequest& request) override;

    void createIzatRequest(
            IzatRequest& izatRequest,
            const ::vendor::qti::gnss::V1_0::LocHidlIzatRequest& gnssIzatRequest);

    sp<LocHidlDeathRecipient> mDeathRecipient = nullptr;
    Listener* mListener;
    IIzatManager* mIzatManager;
    const IzatStreamType mProviderType;
};

}  // namespace implementation
}  // namespace LOC_HIDL_VERSION
}  // namespace gnss
}  // namespace qti
}  // namespace vendor
