/*
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_0_TYPES_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_0_TYPES_H

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace V2_0 {

// Forward declaration for forward reference support:
enum class Status : int32_t;
enum class EnableStatus : int32_t;
enum class BearerStatus : int32_t;
enum class EndcAvailableType : int32_t;
enum class RestrictDcnrType : int32_t;
struct DcParam;
struct SignalStrength;

enum class Status : int32_t {
    INVALID = -1, // (-1)
    FAILURE = 0,
    SUCCESS = 1,
};

enum class EnableStatus : int32_t {
    INVALID = -1, // (-1)
    DISABLED = 0,
    ENABLED = 1,
};

enum class BearerStatus : int32_t {
    INVALID = -1, // (-1)
    NOT_ALLOCATED = 0,
    ALLOCATED = 1,
};

enum class EndcAvailableType : int32_t {
    INVALID = -1, // (-1)
    ENDC_UNAVAILABLE = 0,
    ENDC_AVAILABLE = 1,
};

enum class RestrictDcnrType : int32_t {
    INVALID = -1, // (-1)
    DCNR_RESTRICTED = 0,
    DCNR_UNRESTRICTED = 1,
};

struct DcParam final {
    ::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType endc __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType dcnr __attribute__ ((aligned(4)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::qtiradio::V2_0::DcParam, endc) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::qtiradio::V2_0::DcParam, dcnr) == 4, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::qtiradio::V2_0::DcParam) == 8, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::qtiradio::V2_0::DcParam) == 4, "wrong alignment");

struct SignalStrength final {
    int32_t rsrp __attribute__ ((aligned(4)));
    int32_t snr __attribute__ ((aligned(4)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::qtiradio::V2_0::SignalStrength, rsrp) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::qtiradio::V2_0::SignalStrength, snr) == 4, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::qtiradio::V2_0::SignalStrength) == 8, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::qtiradio::V2_0::SignalStrength) == 4, "wrong alignment");

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::qtiradio::V2_0::Status lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_0::Status rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_0::Status rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::qtiradio::V2_0::Status lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::qtiradio::V2_0::Status lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_0::Status rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_0::Status rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::qtiradio::V2_0::Status lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::qtiradio::V2_0::Status e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::qtiradio::V2_0::Status e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::qtiradio::V2_0::Status>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::qtiradio::V2_0::Status> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_0::Status::INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_0::Status::INVALID)) {
        os += (first ? "" : " | ");
        os += "INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_0::Status::INVALID;
    }
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_0::Status::FAILURE) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_0::Status::FAILURE)) {
        os += (first ? "" : " | ");
        os += "FAILURE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_0::Status::FAILURE;
    }
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_0::Status::SUCCESS) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_0::Status::SUCCESS)) {
        os += (first ? "" : " | ");
        os += "SUCCESS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_0::Status::SUCCESS;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::qtiradio::V2_0::Status o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_0::Status::INVALID) {
        return "INVALID";
    }
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_0::Status::FAILURE) {
        return "FAILURE";
    }
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_0::Status::SUCCESS) {
        return "SUCCESS";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus::INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus::INVALID)) {
        os += (first ? "" : " | ");
        os += "INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus::INVALID;
    }
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus::DISABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus::DISABLED)) {
        os += (first ? "" : " | ");
        os += "DISABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus::DISABLED;
    }
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus::ENABLED) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus::ENABLED)) {
        os += (first ? "" : " | ");
        os += "ENABLED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus::ENABLED;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus::INVALID) {
        return "INVALID";
    }
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus::DISABLED) {
        return "DISABLED";
    }
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus::ENABLED) {
        return "ENABLED";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus::INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus::INVALID)) {
        os += (first ? "" : " | ");
        os += "INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus::INVALID;
    }
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus::NOT_ALLOCATED) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus::NOT_ALLOCATED)) {
        os += (first ? "" : " | ");
        os += "NOT_ALLOCATED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus::NOT_ALLOCATED;
    }
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus::ALLOCATED) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus::ALLOCATED)) {
        os += (first ? "" : " | ");
        os += "ALLOCATED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus::ALLOCATED;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus::INVALID) {
        return "INVALID";
    }
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus::NOT_ALLOCATED) {
        return "NOT_ALLOCATED";
    }
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus::ALLOCATED) {
        return "ALLOCATED";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType::INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType::INVALID)) {
        os += (first ? "" : " | ");
        os += "INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType::INVALID;
    }
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType::ENDC_UNAVAILABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType::ENDC_UNAVAILABLE)) {
        os += (first ? "" : " | ");
        os += "ENDC_UNAVAILABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType::ENDC_UNAVAILABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType::ENDC_AVAILABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType::ENDC_AVAILABLE)) {
        os += (first ? "" : " | ");
        os += "ENDC_AVAILABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType::ENDC_AVAILABLE;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType::INVALID) {
        return "INVALID";
    }
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType::ENDC_UNAVAILABLE) {
        return "ENDC_UNAVAILABLE";
    }
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType::ENDC_AVAILABLE) {
        return "ENDC_AVAILABLE";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType::INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType::INVALID)) {
        os += (first ? "" : " | ");
        os += "INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType::INVALID;
    }
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType::DCNR_RESTRICTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType::DCNR_RESTRICTED)) {
        os += (first ? "" : " | ");
        os += "DCNR_RESTRICTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType::DCNR_RESTRICTED;
    }
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType::DCNR_UNRESTRICTED) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType::DCNR_UNRESTRICTED)) {
        os += (first ? "" : " | ");
        os += "DCNR_UNRESTRICTED";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType::DCNR_UNRESTRICTED;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType::INVALID) {
        return "INVALID";
    }
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType::DCNR_RESTRICTED) {
        return "DCNR_RESTRICTED";
    }
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType::DCNR_UNRESTRICTED) {
        return "DCNR_UNRESTRICTED";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

static inline std::string toString(const ::vendor::qti::hardware::radio::qtiradio::V2_0::DcParam& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".endc = ";
    os += ::vendor::qti::hardware::radio::qtiradio::V2_0::toString(o.endc);
    os += ", .dcnr = ";
    os += ::vendor::qti::hardware::radio::qtiradio::V2_0::toString(o.dcnr);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::qtiradio::V2_0::DcParam& lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_0::DcParam& rhs) {
    if (lhs.endc != rhs.endc) {
        return false;
    }
    if (lhs.dcnr != rhs.dcnr) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::qtiradio::V2_0::DcParam& lhs,const ::vendor::qti::hardware::radio::qtiradio::V2_0::DcParam& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::qtiradio::V2_0::SignalStrength& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".rsrp = ";
    os += ::android::hardware::toString(o.rsrp);
    os += ", .snr = ";
    os += ::android::hardware::toString(o.snr);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::qtiradio::V2_0::SignalStrength& lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_0::SignalStrength& rhs) {
    if (lhs.rsrp != rhs.rsrp) {
        return false;
    }
    if (lhs.snr != rhs.snr) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::qtiradio::V2_0::SignalStrength& lhs,const ::vendor::qti::hardware::radio::qtiradio::V2_0::SignalStrength& rhs){
    return !(lhs == rhs);
}


}  // namespace V2_0
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::qtiradio::V2_0::Status>
{
    const ::vendor::qti::hardware::radio::qtiradio::V2_0::Status* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::qtiradio::V2_0::Status* end() { return begin() + 3; }
    private:
    static const ::vendor::qti::hardware::radio::qtiradio::V2_0::Status* static_begin() {
        static const ::vendor::qti::hardware::radio::qtiradio::V2_0::Status kVals[3] {
            ::vendor::qti::hardware::radio::qtiradio::V2_0::Status::INVALID,
            ::vendor::qti::hardware::radio::qtiradio::V2_0::Status::FAILURE,
            ::vendor::qti::hardware::radio::qtiradio::V2_0::Status::SUCCESS,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus>
{
    const ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus* end() { return begin() + 3; }
    private:
    static const ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus* static_begin() {
        static const ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus kVals[3] {
            ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus::INVALID,
            ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus::DISABLED,
            ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus::ENABLED,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus>
{
    const ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus* end() { return begin() + 3; }
    private:
    static const ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus* static_begin() {
        static const ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus kVals[3] {
            ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus::INVALID,
            ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus::NOT_ALLOCATED,
            ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus::ALLOCATED,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType>
{
    const ::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType* end() { return begin() + 3; }
    private:
    static const ::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType* static_begin() {
        static const ::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType kVals[3] {
            ::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType::INVALID,
            ::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType::ENDC_UNAVAILABLE,
            ::vendor::qti::hardware::radio::qtiradio::V2_0::EndcAvailableType::ENDC_AVAILABLE,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType>
{
    const ::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType* end() { return begin() + 3; }
    private:
    static const ::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType* static_begin() {
        static const ::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType kVals[3] {
            ::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType::INVALID,
            ::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType::DCNR_RESTRICTED,
            ::vendor::qti::hardware::radio::qtiradio::V2_0::RestrictDcnrType::DCNR_UNRESTRICTED,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android

#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_0_TYPES_H
