/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/
#pragma once

#include "interfaces/common.h"

namespace qcril {
namespace interfaces {

typedef enum
{
  QCRIL_PDC_MBN_TYPE_SW = 0,
  QCRIL_PDC_MBN_TYPE_HW,

  QCRIL_PDC_MBN_TYPE_MAX = 0xFFFFFFFF
} qcril_pdc_mbn_type;

//Message Callback structure
class MbnConfigResp : public qcril::interfaces::BasePayload {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(int32_t, Length);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::vector<uint8_t>, Config);

 public:
  MbnConfigResp() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Length);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Config);
  }
  MbnConfigResp(const MbnConfigResp &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Length);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Config);
  }
  ~MbnConfigResp() {}
};

class FileVersionResp : public qcril::interfaces::BasePayload {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(int32_t, Length);
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(std::string, Version);
 public:
  FileVersionResp() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Length);
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Version);
  }
  FileVersionResp(const FileVersionResp &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Length);
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Version);
  }
  ~FileVersionResp() {}
};

class ConfigVersionResp : public qcril::interfaces::BasePayload {
  QCRIL_INTERFACE_OPTIONAL_MEMBER_VAR(int32_t, Version);
 public:
  ConfigVersionResp() {
    QCRIL_INTERFACE_RESET_OPTIONAL_MEMBER_VAR(Version);
  }
  ConfigVersionResp(const ConfigVersionResp &from) {
    QCRIL_INTERFACE_COPY_OPTIONAL_MEMBER_VAR(from, Version);
  }
  ~ConfigVersionResp() {}
};

} // namespace interfaces
} // namespace qcril
