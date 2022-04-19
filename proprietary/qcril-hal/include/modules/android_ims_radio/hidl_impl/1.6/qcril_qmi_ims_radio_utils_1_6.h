/*===========================================================================
   Copyright (c) 2019 Qualcomm Technologies, Inc.
   All Rights Reserved.
   Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#pragma once

#include <hidl/HidlSupport.h>
#include <modules/android_ims_radio/hidl_impl/base/qcril_qmi_ims_radio_utils.h>
#include <modules/android_ims_radio/hidl_impl/1.5/qcril_qmi_ims_radio_utils_1_5.h>
#include <vendor/qti/hardware/radio/ims/1.6/types.h>

using ::android::hardware::hidl_vec;

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_6 {
namespace utils {

void convertConfigInfo(V1_6::ConfigInfo &outConfig, const qcril::interfaces::ConfigInfo &inConfig);
void readConfigInfo(const V1_6::ConfigInfo &inConfig, qcril::interfaces::ConfigInfo &outConfig);
qcril::interfaces::ConfigItem convertConfigItem(const V1_6::ConfigItem in);
V1_6::ConfigItem convertConfigItem(qcril::interfaces::ConfigItem in);
bool convertCallInfoList(hidl_vec<V1_6::CallInfo> &out,
                         const std::vector<qcril::interfaces::CallInfo> &in);
bool convertCallInfo(V1_6::CallInfo &out, const qcril::interfaces::CallInfo &in);
bool convertServiceStatusInfo(V1_6::ServiceStatusInfo &out,
                              const qcril::interfaces::ServiceStatusInfo &in);
bool convertServiceStatusInfoList(hidl_vec<V1_6::ServiceStatusInfo> &out,
                                  const qcril::interfaces::ServiceStatusInfoList &in);
bool convertServiceStatusInfoList(hidl_vec<V1_6::ServiceStatusInfo> &out,
                                  const std::vector<qcril::interfaces::ServiceStatusInfo> &in);
bool convertRegistrationInfo(V1_6::RegistrationInfo &out, const qcril::interfaces::Registration &in);
V1_6::RadioTechType convertRadioTechType(const RIL_RadioTechnology &in);
RIL_RadioTechnology convertRadioTechType(const V1_6::RadioTechType &in);
bool convertAccessTechnologyStatus(V1_6::StatusForAccessTech &out,
                                   const qcril::interfaces::AccessTechnologyStatus &in);
std::shared_ptr<qcril::interfaces::AccessTechnologyStatus> readAccessTechStatus(
        const V1_6::StatusForAccessTech &in);
std::shared_ptr<qcril::interfaces::Registration> readRegistration(const V1_6::RegistrationInfo &in);
void readServiceStatusInfo(const V1_6::ServiceStatusInfo &in, qcril::interfaces::ServiceStatusInfo &out);
}  // namespace utils
}  // namespace V1_6
}  // namespace ims
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
