/******************************************************************************
  @file    ril_utf_hidl_services.cpp
  @brief   RIL UTF IMS HIDL services

  DESCRIPTION

  ---------------------------------------------------------------------------
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/

#include "ril_utf_hidl_services.h"

using ::android::hidl::base::V1_0::IBase;

namespace ril_utf {
namespace hidl_services {

#if 0
std::unordered_map<std::string, IBase *> hidlServices;

void add(const std::string &serviceName, IBase *service) {
  hidlServices[serviceName] = service;
}

IBase *get(const std::string &serviceName) {
  return hidlServices[serviceName];
}
#endif

} // namespace hidl_services
} // namespace ril_utf
