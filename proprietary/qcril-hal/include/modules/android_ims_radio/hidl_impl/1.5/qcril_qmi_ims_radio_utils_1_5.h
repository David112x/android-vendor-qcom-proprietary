/*===========================================================================
   Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
   All Rights Reserved.
   Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#pragma once

#include <hidl/HidlSupport.h>
#include <modules/android_ims_radio/hidl_impl/1.4/qcril_qmi_ims_radio_utils_1_4.h>
#include <vendor/qti/hardware/radio/ims/1.5/types.h>

using ::android::hardware::hidl_bitfield;
using ::android::hardware::hidl_string;
using ::android::hardware::hidl_vec;

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_5 {
namespace utils {

V1_5::CallFailCause convertCallFailCause(const qcril::interfaces::CallFailCause &in);
qcril::interfaces::CallFailCause convertCallFailCause(const V1_5::CallFailCause &in);
bool convertCallInfo(V1_5::CallInfo &out, const qcril::interfaces::CallInfo &in);
bool convertCallInfoList(hidl_vec<V1_5::CallInfo> &out,
                         const std::vector<qcril::interfaces::CallInfo> &in);
bool convertCallFailCauseResponse(
    V1_5::CallFailCauseResponse &out,
    const std::shared_ptr<qcril::interfaces::CallFailCauseResponse> &in);
bool convertCallForwardStatusInfo(
    V1_5::CallForwardStatusInfo &out,
    const std::shared_ptr<qcril::interfaces::SetCallForwardRespData> &in);
uint64_t convertToRilEmergencyServiceCategories(
    const hidl_bitfield<V1_5::EmergencyServiceCategory> categories);
qcril::interfaces::EmergencyCallRouting convertToRilEmergencyCallRoute(
    const V1_5::EmergencyCallRoute route);
void readConfigInfo(const V1_5::ConfigInfo &inConfig, qcril::interfaces::ConfigInfo &outConfig);
qcril::interfaces::ConfigItem convertConfigItem(const V1_5::ConfigItem in);
V1_5::TirMode convertTirMode(const qcril::interfaces::TirMode &in);
V1_5::UssdModeType convertUssdModeType(const qcril::interfaces::UssdModeType &in);
qcril::interfaces::DeliverStatus convertHidlToRilSmsAckResult(
    const V1_5::ImsSmsDeliverStatusResult result);

void convertRilImsResponseToHidlResponse(RIL_Errno rilErr, RIL_RadioTechnologyFamily tech,
    std::optional<bool> bLteOnlyReg, V1_2::ImsSmsSendStatusResult& statusReport,
    V1_5::ImsSmsSendFailureReason& failureReason);

V1_2::ImsSmsSendFailureReason convertHidlSendSmsFailureReasonToV12(
    V1_5::ImsSmsSendFailureReason reason);

bool convertSipErrorInfo(V1_0::SipErrorInfo &out,
                         const std::shared_ptr<qcril::interfaces::SipErrorInfo> &in);

}  // namespace utils
}  // namespace V1_5
}  // namespace ims
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
