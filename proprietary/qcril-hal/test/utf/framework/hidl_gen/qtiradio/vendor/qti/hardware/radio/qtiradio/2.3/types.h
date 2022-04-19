/*
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_3_TYPES_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_3_TYPES_H

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace V2_3 {

// Forward declaration for forward reference support:
enum class EndcStatus : int32_t;

enum class EndcStatus : int32_t {
    INVALID = -1 /* (-1) */,
    DISABLED = 0,
    ENABLED = 1,
};

//
// type declarations for package
//

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus o);

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus e) {
    v &= static_cast<int32_t>(e);
    return v;
}

//
// type header definitions for package
//

template<>
inline std::string toString<::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus::INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus::INVALID)) {
        os += (first ? "" : " | ");
        os += "INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus::INVALID;
    }
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus::DISABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus::DISABLED)) {
        os += (first ? "" : " | ");
        os += "DISABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus::DISABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus::ENABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus::ENABLED)) {
        os += (first ? "" : " | ");
        os += "ENABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus::ENABLED;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus::INVALID) {
        return "INVALID";
    }
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus::DISABLED) {
        return "DISABLED";
    }
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus::ENABLED) {
        return "ENABLED";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}


}  // namespace V2_3
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

//
// global type declarations for package
//

namespace android {
namespace hardware {
namespace details {
template<> constexpr std::array<::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus, 3> hidl_enum_values<::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus> = {
    ::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus::INVALID,
    ::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus::DISABLED,
    ::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus::ENABLED,
};
}  // namespace details
}  // namespace hardware
}  // namespace android


#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_3_TYPES_H
