/*
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_1_TYPES_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_1_TYPES_H

#include <vendor/qti/hardware/radio/qtiradio/2.0/types.h>

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace V2_1 {

// Forward declaration for forward reference support:
enum class BearerStatus : int32_t;
enum class ConfigType : int32_t;
enum class PlmnInfoListStatus : int32_t;
enum class UpperLayerIndStatus : int32_t;
struct UpperLayerIndInfo;

enum class BearerStatus : int32_t {
    INVALID = -1 /* (-1) */,
    NOT_ALLOCATED = 0,
    ALLOCATED = 1,
    MMW_ALLOCATED = 2,
};

enum class ConfigType : int32_t {
    INVALID = -1 /* (-1) */,
    NSA_CONFIGURATION = 0,
    SA_CONFIGURATION = 1,
};

enum class PlmnInfoListStatus : int32_t {
    INVALID = -1 /* (-1) */,
    UNAVAILABLE = 0,
    AVAILABLE = 1,
};

enum class UpperLayerIndStatus : int32_t {
    INVALID = -1 /* (-1) */,
    UNAVAILABLE = 0,
    AVAILABLE = 1,
};

struct UpperLayerIndInfo final {
    ::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus plmnInfoList __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus upplerLayerInd __attribute__ ((aligned(4)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndInfo, plmnInfoList) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndInfo, upplerLayerInd) == 4, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndInfo) == 8, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndInfo) == 4, "wrong alignment");

//
// type declarations for package
//

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus o);

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType o);

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus o);

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus o);

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus e) {
    v &= static_cast<int32_t>(e);
    return v;
}

static inline std::string toString(const ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndInfo& o);
static inline bool operator==(const ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndInfo& lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndInfo& rhs);
static inline bool operator!=(const ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndInfo& lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndInfo& rhs);

//
// type header definitions for package
//

template<>
inline std::string toString<::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus::INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus::INVALID)) {
        os += (first ? "" : " | ");
        os += "INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus::INVALID;
    }
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus::NOT_ALLOCATED) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus::NOT_ALLOCATED)) {
        os += (first ? "" : " | ");
        os += "NOT_ALLOCATED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus::NOT_ALLOCATED;
    }
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus::ALLOCATED) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus::ALLOCATED)) {
        os += (first ? "" : " | ");
        os += "ALLOCATED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus::ALLOCATED;
    }
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus::MMW_ALLOCATED) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus::MMW_ALLOCATED)) {
        os += (first ? "" : " | ");
        os += "MMW_ALLOCATED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus::MMW_ALLOCATED;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus::INVALID) {
        return "INVALID";
    }
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus::NOT_ALLOCATED) {
        return "NOT_ALLOCATED";
    }
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus::ALLOCATED) {
        return "ALLOCATED";
    }
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus::MMW_ALLOCATED) {
        return "MMW_ALLOCATED";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

template<>
inline std::string toString<::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType::INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType::INVALID)) {
        os += (first ? "" : " | ");
        os += "INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType::INVALID;
    }
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType::NSA_CONFIGURATION) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType::NSA_CONFIGURATION)) {
        os += (first ? "" : " | ");
        os += "NSA_CONFIGURATION";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType::NSA_CONFIGURATION;
    }
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType::SA_CONFIGURATION) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType::SA_CONFIGURATION)) {
        os += (first ? "" : " | ");
        os += "SA_CONFIGURATION";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType::SA_CONFIGURATION;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType::INVALID) {
        return "INVALID";
    }
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType::NSA_CONFIGURATION) {
        return "NSA_CONFIGURATION";
    }
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType::SA_CONFIGURATION) {
        return "SA_CONFIGURATION";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

template<>
inline std::string toString<::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus::INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus::INVALID)) {
        os += (first ? "" : " | ");
        os += "INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus::INVALID;
    }
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus::UNAVAILABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus::UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "UNAVAILABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus::UNAVAILABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus::AVAILABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus::AVAILABLE)) {
        os += (first ? "" : " | ");
        os += "AVAILABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus::AVAILABLE;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus::INVALID) {
        return "INVALID";
    }
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus::UNAVAILABLE) {
        return "UNAVAILABLE";
    }
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus::AVAILABLE) {
        return "AVAILABLE";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

template<>
inline std::string toString<::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus::INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus::INVALID)) {
        os += (first ? "" : " | ");
        os += "INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus::INVALID;
    }
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus::UNAVAILABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus::UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "UNAVAILABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus::UNAVAILABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus::AVAILABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus::AVAILABLE)) {
        os += (first ? "" : " | ");
        os += "AVAILABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus::AVAILABLE;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus::INVALID) {
        return "INVALID";
    }
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus::UNAVAILABLE) {
        return "UNAVAILABLE";
    }
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus::AVAILABLE) {
        return "AVAILABLE";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

static inline std::string toString(const ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".plmnInfoList = ";
    os += ::vendor::qti::hardware::radio::qtiradio::V2_1::toString(o.plmnInfoList);
    os += ", .upplerLayerInd = ";
    os += ::vendor::qti::hardware::radio::qtiradio::V2_1::toString(o.upplerLayerInd);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndInfo& lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndInfo& rhs) {
    if (lhs.plmnInfoList != rhs.plmnInfoList) {
        return false;
    }
    if (lhs.upplerLayerInd != rhs.upplerLayerInd) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndInfo& lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndInfo& rhs){
    return !(lhs == rhs);
}


}  // namespace V2_1
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
template<> constexpr std::array<::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus, 4> hidl_enum_values<::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus> = {
    ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus::INVALID,
    ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus::NOT_ALLOCATED,
    ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus::ALLOCATED,
    ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus::MMW_ALLOCATED,
};
}  // namespace details
}  // namespace hardware
}  // namespace android

namespace android {
namespace hardware {
namespace details {
template<> constexpr std::array<::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType, 3> hidl_enum_values<::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType> = {
    ::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType::INVALID,
    ::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType::NSA_CONFIGURATION,
    ::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType::SA_CONFIGURATION,
};
}  // namespace details
}  // namespace hardware
}  // namespace android

namespace android {
namespace hardware {
namespace details {
template<> constexpr std::array<::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus, 3> hidl_enum_values<::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus> = {
    ::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus::INVALID,
    ::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus::UNAVAILABLE,
    ::vendor::qti::hardware::radio::qtiradio::V2_1::PlmnInfoListStatus::AVAILABLE,
};
}  // namespace details
}  // namespace hardware
}  // namespace android

namespace android {
namespace hardware {
namespace details {
template<> constexpr std::array<::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus, 3> hidl_enum_values<::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus> = {
    ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus::INVALID,
    ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus::UNAVAILABLE,
    ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndStatus::AVAILABLE,
};
}  // namespace details
}  // namespace hardware
}  // namespace android


#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_1_TYPES_H
