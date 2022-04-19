/*
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_2_TYPES_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_2_TYPES_H

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace V2_2 {

// Forward declaration for forward reference support:
enum class NrIconType : int32_t;

enum class NrIconType : int32_t {
    INVALID = -1, // (-1)
    TYPE_NONE = 0,
    TYPE_5G_BASIC = 1,
    TYPE_5G_UWB = 2,
};

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType::INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType::INVALID)) {
        os += (first ? "" : " | ");
        os += "INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType::INVALID;
    }
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType::TYPE_NONE) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType::TYPE_NONE)) {
        os += (first ? "" : " | ");
        os += "TYPE_NONE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType::TYPE_NONE;
    }
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType::TYPE_5G_BASIC) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType::TYPE_5G_BASIC)) {
        os += (first ? "" : " | ");
        os += "TYPE_5G_BASIC";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType::TYPE_5G_BASIC;
    }
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType::TYPE_5G_UWB) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType::TYPE_5G_UWB)) {
        os += (first ? "" : " | ");
        os += "TYPE_5G_UWB";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType::TYPE_5G_UWB;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType::INVALID) {
        return "INVALID";
    }
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType::TYPE_NONE) {
        return "TYPE_NONE";
    }
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType::TYPE_5G_BASIC) {
        return "TYPE_5G_BASIC";
    }
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType::TYPE_5G_UWB) {
        return "TYPE_5G_UWB";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}


}  // namespace V2_2
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType>
{
    const ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType* end() { return begin() + 4; }
    private:
    static const ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType* static_begin() {
        static const ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType kVals[4] {
            ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType::INVALID,
            ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType::TYPE_NONE,
            ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType::TYPE_5G_BASIC,
            ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType::TYPE_5G_UWB,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android

#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_2_TYPES_H
