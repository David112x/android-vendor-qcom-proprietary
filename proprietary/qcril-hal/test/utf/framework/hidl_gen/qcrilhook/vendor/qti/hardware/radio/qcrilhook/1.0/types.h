/******************************************************************************
  @file framework/hidl_gen/qcrilhook/vendor/qti/hardware/radio/qcrilhook/1.0/types.h
  DESCRIPTION
  ---------------------------------------------------------------------------
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QCRILHOOK_V1_0_TYPES_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QCRILHOOK_V1_0_TYPES_H

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qcrilhook {
namespace V1_0 {

enum class RadioError : int32_t {
    NONE = 0,
    RADIO_NOT_AVAILABLE = 1,
    GENERIC_FAILURE = 2,
    PASSWORD_INCORRECT = 3,
    SIM_PIN2 = 4,
    SIM_PUK2 = 5,
    REQUEST_NOT_SUPPORTED = 6,
    CANCELLED = 7,
    OP_NOT_ALLOWED_DURING_VOICE_CALL = 8,
    OP_NOT_ALLOWED_BEFORE_REG_TO_NW = 9,
    SMS_SEND_FAIL_RETRY = 10,
    SIM_ABSENT = 11,
    SUBSCRIPTION_NOT_AVAILABLE = 12,
    MODE_NOT_SUPPORTED = 13,
    FDN_CHECK_FAILURE = 14,
    ILLEGAL_SIM_OR_ME = 15,
    MISSING_RESOURCE = 16,
    NO_SUCH_ELEMENT = 17,
    DIAL_MODIFIED_TO_USSD = 18,
    DIAL_MODIFIED_TO_SS = 19,
    DIAL_MODIFIED_TO_DIAL = 20,
    USSD_MODIFIED_TO_DIAL = 21,
    USSD_MODIFIED_TO_SS = 22,
    USSD_MODIFIED_TO_USSD = 23,
    SS_MODIFIED_TO_DIAL = 24,
    SS_MODIFIED_TO_USSD = 25,
    SUBSCRIPTION_NOT_SUPPORTED = 26,
    SS_MODIFIED_TO_SS = 27,
    LCE_NOT_SUPPORTED = 36,
    NO_MEMORY = 37,
    INTERNAL_ERR = 38,
    SYSTEM_ERR = 39,
    MODEM_ERR = 40,
    INVALID_STATE = 41,
    NO_RESOURCES = 42,
    SIM_ERR = 43,
    INVALID_ARGUMENTS = 44,
    INVALID_SIM_STATE = 45,
    INVALID_MODEM_STATE = 46,
    INVALID_CALL_ID = 47,
    NO_SMS_TO_ACK = 48,
    NETWORK_ERR = 49,
    REQUEST_RATE_LIMITED = 50,
    SIM_BUSY = 51,
    SIM_FULL = 52,
    NETWORK_REJECT = 53,
    OPERATION_NOT_ALLOWED = 54,
    EMPTY_RECORD = 55,
    INVALID_SMS_FORMAT = 56,
    ENCODING_ERR = 57,
    INVALID_SMSC_ADDRESS = 58,
    NO_SUCH_ENTRY = 59,
    NETWORK_NOT_READY = 60,
    NOT_PROVISIONED = 61,
    NO_SUBSCRIPTION = 62,
    NO_NETWORK_FOUND = 63,
    DEVICE_IN_USE = 64,
    ABORTED = 65,
    INVALID_RESPONSE = 66,
    OEM_ERROR_1 = 501,
    OEM_ERROR_2 = 502,
    OEM_ERROR_3 = 503,
    OEM_ERROR_4 = 504,
    OEM_ERROR_5 = 505,
    OEM_ERROR_6 = 506,
    OEM_ERROR_7 = 507,
    OEM_ERROR_8 = 508,
    OEM_ERROR_9 = 509,
    OEM_ERROR_10 = 510,
    OEM_ERROR_11 = 511,
    OEM_ERROR_12 = 512,
    OEM_ERROR_13 = 513,
    OEM_ERROR_14 = 514,
    OEM_ERROR_15 = 515,
    OEM_ERROR_16 = 516,
    OEM_ERROR_17 = 517,
    OEM_ERROR_18 = 518,
    OEM_ERROR_19 = 519,
    OEM_ERROR_20 = 520,
    OEM_ERROR_21 = 521,
    OEM_ERROR_22 = 522,
    OEM_ERROR_23 = 523,
    OEM_ERROR_24 = 524,
    OEM_ERROR_25 = 525,
};

constexpr int32_t operator|(const RadioError lhs, const RadioError rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const RadioError rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const RadioError lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const RadioError lhs, const RadioError rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const RadioError rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const RadioError lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const RadioError e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const RadioError e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
std::string toString(int32_t o);
template<>
std::string toString<RadioError>(int32_t o);

std::string toString(RadioError o);


}  // namespace V1_0
}  // namespace qcrilhook
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QCRILHOOK_V1_0_TYPES_H
