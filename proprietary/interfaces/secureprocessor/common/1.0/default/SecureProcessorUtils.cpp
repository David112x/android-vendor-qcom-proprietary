/*
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <vendor/qti/hardware/secureprocessor/device/1.0/ISecureProcessor.h>
#include "SecureProcessorUtils.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace secureprocessor {
namespace common {
namespace V1_0 {

void convertToHidl(SecureProcessorConfig* inCfg, SecureProcessorCfgBuf *outCfg) {
    if (inCfg == nullptr || outCfg == nullptr) {
        return;
    }
    uint32_t size = inCfg->getSize();
    outCfg->setToExternal((uint8_t *) inCfg->releaseAndGetBuffer(), size);
}

void convertFromHidl(const SecureProcessorCfgBuf *inCfg,
                     SecureProcessorConfig** outCfg) {

    if (inCfg == nullptr || outCfg == nullptr) {
        return;
    }
    *outCfg = new SecureProcessorConfig((void *) inCfg->data());
}

}  // namespace V1_0
}  // namespace common
}  // namespace secureprocessor
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
