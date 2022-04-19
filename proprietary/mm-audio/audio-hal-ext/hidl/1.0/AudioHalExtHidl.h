/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef VENDOR_QTI_AUDIOHALEXT_V1_0_AUDIOHALEXT_H
#define VENDOR_QTI_AUDIOHALEXT_V1_0_AUDIOHALEXT_H

#include <vendor/qti/hardware/audiohalext/1.0/IAudioHalExt.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <log/log.h>
#include <stdlib.h>
#include <string>

#include "ConfigParser.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace audiohalext {
namespace V1_0 {
namespace implementation {

using ::android::hardware::hidl_array;
using ::android::hardware::hidl_memory;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;
using ::android::hardware::Return;
using ::android::hardware::Void;
using ::android::sp;

struct AudioHalExtHidl: public IAudioHalExt {
    AudioHalExtHidl();
    static AudioHalExtHidl *getInstance();

    // Methods from ::vendor::qti::hardware::audiohalext::V1_0::IAudioHalExt follow.
    Return<void> getApmConfigs(getApmConfigs_cb _hidl_cb) override;
    Return<void> getAVConfigs(getAVConfigs_cb _hidl_cb) override;

    // Methods from ::android::hidl::base::V1_0::IBase follow.
private:
    ConfigParser *inst;
};

extern "C" IAudioHalExt* HIDL_FETCH_IAudioHalExt(const char* name);

}  // namespace implementation
}  // namespace V1_0
}  // namespace audiohalext
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

#endif  // VENDOR_QTI_AUDIOHALEXT_V1_0_AUDIOHALEXT_H
