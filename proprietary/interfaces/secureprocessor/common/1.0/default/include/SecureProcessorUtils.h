/*
 * Copyright (c) 2020 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#ifndef SECURE_PROCESSOR_1_0_UTILS_H
#define SECURE_PROCESSOR_1_0_UTILS_H

#include "SecureProcessorConfig.h"

using vendor::qti::hardware::secureprocessor::common::V1_0::SecureProcessorCfgBuf;

namespace vendor {
namespace qti {
namespace hardware {
namespace secureprocessor{
namespace common {
namespace V1_0 {

/**
  * convertToHidl
  *   Retrive the configuration backing buffer from config class
  *   object and convert it to HIDL cfgBuf.
  *
  * @param inCfg Input configuration class object
  * @param outCfg HIDL cfgBuf
  *
  * @return none.
  */
void convertToHidl(SecureProcessorConfig* inCfg, SecureProcessorCfgBuf *outCfg);

/**
  * convertFromHidl
  *   Create a configuration class object using HIDL cfgBuf as
  *   it's backing buffer for de-marshaling the configuration
  *   entries.
  *
  * @param inCfg HIDL cfgBuf
  * @param outCfg Output configuration class object
  *
  * @return none.
  */
void convertFromHidl(const SecureProcessorCfgBuf *inCfg,
                     SecureProcessorConfig** outCfg);

} // namespace V1_0
} // namespace common
} // namespace secureprocessor
} // namespace hardware
} // namespace qti
} // namespace vendor

#endif
