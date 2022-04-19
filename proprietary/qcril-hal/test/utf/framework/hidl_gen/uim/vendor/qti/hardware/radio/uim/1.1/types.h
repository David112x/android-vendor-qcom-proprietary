/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_UIM_V1_1_TYPES_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_UIM_V1_1_TYPES_H

#include <vendor/qti/hardware/radio/uim/1.0/types.h>

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace uim {
namespace V1_1 {

// Forward declaration for forward reference support:
enum class UimRemoteSimlockOperationType : uint32_t;

enum class UimRemoteSimlockOperationType : uint32_t {
    UIM_REMOTE_SIMLOCK_GENERATE_ENCRYPTED_KEY = 0u,
    UIM_REMOTE_SIMLOCK_PROCESS_SIMLOCK_DATA = 1u,
    UIM_REMOTE_SIMLOCK_GENERATE_HMAC = 2u,
    UIM_REMOTE_SIMLOCK_GET_MAX_SUPPORTED_VERSION = 3u,
    UIM_REMOTE_SIMLOCK_GET_STATUS = 4u,
    UIM_REMOTE_SIMLOCK_GENERATE_BLOB_REQUEST = 5u,
    UIM_REMOTE_SIMLOCK_UNLOCK_TIMER_START = 6u,
    UIM_REMOTE_SIMLOCK_UNLOCK_TIMER_STOP = 7u,
};

constexpr uint32_t operator|(const ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType lhs, const ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType lhs, const ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(uint32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType>(uint32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_GENERATE_ENCRYPTED_KEY) == static_cast<uint32_t>(::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_GENERATE_ENCRYPTED_KEY)) {
        os += (first ? "" : " | ");
        os += "UIM_REMOTE_SIMLOCK_GENERATE_ENCRYPTED_KEY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_GENERATE_ENCRYPTED_KEY;
    }
    if ((o & ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_PROCESS_SIMLOCK_DATA) == static_cast<uint32_t>(::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_PROCESS_SIMLOCK_DATA)) {
        os += (first ? "" : " | ");
        os += "UIM_REMOTE_SIMLOCK_PROCESS_SIMLOCK_DATA";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_PROCESS_SIMLOCK_DATA;
    }
    if ((o & ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_GENERATE_HMAC) == static_cast<uint32_t>(::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_GENERATE_HMAC)) {
        os += (first ? "" : " | ");
        os += "UIM_REMOTE_SIMLOCK_GENERATE_HMAC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_GENERATE_HMAC;
    }
    if ((o & ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_GET_MAX_SUPPORTED_VERSION) == static_cast<uint32_t>(::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_GET_MAX_SUPPORTED_VERSION)) {
        os += (first ? "" : " | ");
        os += "UIM_REMOTE_SIMLOCK_GET_MAX_SUPPORTED_VERSION";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_GET_MAX_SUPPORTED_VERSION;
    }
    if ((o & ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_GET_STATUS) == static_cast<uint32_t>(::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_GET_STATUS)) {
        os += (first ? "" : " | ");
        os += "UIM_REMOTE_SIMLOCK_GET_STATUS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_GET_STATUS;
    }
    if ((o & ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_GENERATE_BLOB_REQUEST) == static_cast<uint32_t>(::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_GENERATE_BLOB_REQUEST)) {
        os += (first ? "" : " | ");
        os += "UIM_REMOTE_SIMLOCK_GENERATE_BLOB_REQUEST";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_GENERATE_BLOB_REQUEST;
    }
    if ((o & ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_UNLOCK_TIMER_START) == static_cast<uint32_t>(::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_UNLOCK_TIMER_START)) {
        os += (first ? "" : " | ");
        os += "UIM_REMOTE_SIMLOCK_UNLOCK_TIMER_START";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_UNLOCK_TIMER_START;
    }
    if ((o & ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_UNLOCK_TIMER_STOP) == static_cast<uint32_t>(::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_UNLOCK_TIMER_STOP)) {
        os += (first ? "" : " | ");
        os += "UIM_REMOTE_SIMLOCK_UNLOCK_TIMER_STOP";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_UNLOCK_TIMER_STOP;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_GENERATE_ENCRYPTED_KEY) {
        return "UIM_REMOTE_SIMLOCK_GENERATE_ENCRYPTED_KEY";
    }
    if (o == ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_PROCESS_SIMLOCK_DATA) {
        return "UIM_REMOTE_SIMLOCK_PROCESS_SIMLOCK_DATA";
    }
    if (o == ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_GENERATE_HMAC) {
        return "UIM_REMOTE_SIMLOCK_GENERATE_HMAC";
    }
    if (o == ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_GET_MAX_SUPPORTED_VERSION) {
        return "UIM_REMOTE_SIMLOCK_GET_MAX_SUPPORTED_VERSION";
    }
    if (o == ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_GET_STATUS) {
        return "UIM_REMOTE_SIMLOCK_GET_STATUS";
    }
    if (o == ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_GENERATE_BLOB_REQUEST) {
        return "UIM_REMOTE_SIMLOCK_GENERATE_BLOB_REQUEST";
    }
    if (o == ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_UNLOCK_TIMER_START) {
        return "UIM_REMOTE_SIMLOCK_UNLOCK_TIMER_START";
    }
    if (o == ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_UNLOCK_TIMER_STOP) {
        return "UIM_REMOTE_SIMLOCK_UNLOCK_TIMER_STOP";
    }
    std::string os;
    os += toHexString(static_cast<uint32_t>(o));
    return os;
}


}  // namespace V1_1
}  // namespace uim
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType>
{
    const ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType* end() { return begin() + 8; }
    private:
    static const ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType* static_begin() {
        static const ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType kVals[8] {
            ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_GENERATE_ENCRYPTED_KEY,
            ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_PROCESS_SIMLOCK_DATA,
            ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_GENERATE_HMAC,
            ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_GET_MAX_SUPPORTED_VERSION,
            ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_GET_STATUS,
            ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_GENERATE_BLOB_REQUEST,
            ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_UNLOCK_TIMER_START,
            ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType::UIM_REMOTE_SIMLOCK_UNLOCK_TIMER_STOP,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android

#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_UIM_V1_1_TYPES_H
