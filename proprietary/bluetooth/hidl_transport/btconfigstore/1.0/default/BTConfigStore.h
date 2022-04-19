/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef VENDOR_QTI_HARDWARE_BTCONFIGSTORE_V1_0_IMPL_H
#define VENDOR_QTI_HARDWARE_BTCONFIGSTORE_V1_0_IMPL_H

#pragma once

#include <vendor/qti/hardware/btconfigstore/1.0/IBTConfigStore.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include "data_handler.h"
#include "btcommon_interface_defs.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace btconfigstore {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;
using ::vendor::qti::hardware::btconfigstore::V1_0::VendorProperty;
using ::vendor::qti::hardware::btconfigstore::V1_0::AddOnFeaturesList;
using ::vendor::qti::hardware::btconfigstore::V1_0::Result;
using ::android::hardware::bluetooth::V1_0::implementation::DataHandler;

struct BTConfigStore : public IBTConfigStore {
    // Methods from ::vendor::qti::hardware::btconfigstore::V1_0::IBTConfigStore follow.
    Return<void> getVendorProperties(const uint32_t vPropType, getVendorProperties_cb _hidl_cb) override;
    Return<Result> setVendorProperty(const VendorProperty& vendorProp) override;
    Return<void> getAddOnFeatures(getAddOnFeatures_cb _hidl_cb) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.

};

extern "C" IBTConfigStore* HIDL_FETCH_IBTConfigStore(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace btconfigstore
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
#endif
