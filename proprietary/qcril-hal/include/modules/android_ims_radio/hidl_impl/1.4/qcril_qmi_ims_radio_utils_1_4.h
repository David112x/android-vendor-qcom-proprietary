/*===========================================================================
   Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
   All Rights Reserved.
   Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#pragma once

#include <hidl/HidlSupport.h>
#include <modules/android_ims_radio/hidl_impl/base/qcril_qmi_ims_radio_utils.h>
#include <vendor/qti/hardware/radio/ims/1.4/types.h>

using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_4 {
namespace utils {

bool convertCallInfo(V1_4::CallInfo &out, const qcril::interfaces::CallInfo &in);
bool convertCallInfoList(hidl_vec<V1_4::CallInfo> &out, const std::vector<qcril::interfaces::CallInfo> &in);

// =========================
void convertToHidlMultiIdentityRegistrationStatus(
    const std::vector<qcril::interfaces::MultiIdentityInfo> &in_lines,
    hidl_vec<V1_4::MultiIdentityLineInfoHal> &out_lines);
void convertToRilMultiIdentityInfo(const V1_4::MultiIdentityLineInfoHal &in_line,
                                   qcril::interfaces::MultiIdentityInfo &out_line);
int convertToRilMultiIdentityRegistrationStatus(V1_4::MultiIdentityRegistrationStatus status);
int convertToRilMultiIdentityLineType(V1_4::MultiIdentityLineType type);
void convertToHidlVirtualLines(hidl_vec<hidl_string> &out, const std::vector<std::string> &in);

// enum conversions.
V1_4::MultiIdentityRegistrationStatus convertToHIdlMultiIdentityRegistrationStatus(int status);

}  // namespace utils
}  // namespace V1_4
}  // namespace ims
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
