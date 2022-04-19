/********************************************************************
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *********************************************************************/

#pragma once

#include <vendor/qti/hardware/cryptfshw/1.0/ICryptfsHw.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace cryptfshw {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct CryptfsHw : public ICryptfsHw {
    CryptfsHw() {};
    ~CryptfsHw() {};
    // Methods from ::vendor::qti::hardware::cryptfshw::V1_0::ICryptfsHw follow.
    Return<int32_t> setKey(const hidl_string& password, const hidl_string& mode) override;
    Return<int32_t> setIceParam(uint32_t flag) override;
    Return<int32_t> updateKey(const hidl_string& oldPassword, const hidl_string& newPassword, const hidl_string& mode) override;
    Return<int32_t> clearKey() override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

}  // namespace implementation
}  // namespace V1_0
}  // namespace cryptfshw
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
