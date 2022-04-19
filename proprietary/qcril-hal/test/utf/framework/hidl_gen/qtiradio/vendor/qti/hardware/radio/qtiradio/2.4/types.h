/*
  Copyright (c) 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_4_TYPES_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_4_TYPES_H

#include <android/hardware/radio/1.1/types.h>

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace V2_4 {

// Forward declaration for forward reference support:
enum class PublicKeyType : int32_t;
struct ImsiEncryptionInfo;

/**
 * Public key type from carrier certificate.
 * If not specified, default type is WLAN.
 */
enum class PublicKeyType : int32_t {
    UNKNOWN = 0,
    EPDG = 1,
    WLAN = 2,
};

/**
 * Carrier specific Information sent by the carrier,
 * which will be used to encrypt the IMSI and IMPI.
 */
struct ImsiEncryptionInfo final {
    ::android::hardware::radio::V1_1::ImsiEncryptionInfo base __attribute__ ((aligned(8)));
    ::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType keyType __attribute__ ((aligned(4)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::qtiradio::V2_4::ImsiEncryptionInfo, base) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::qtiradio::V2_4::ImsiEncryptionInfo, keyType) == 72, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::qtiradio::V2_4::ImsiEncryptionInfo) == 80, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::qtiradio::V2_4::ImsiEncryptionInfo) == 8, "wrong alignment");

//
// type declarations for package
//

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType o);

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

static inline std::string toString(const ::vendor::qti::hardware::radio::qtiradio::V2_4::ImsiEncryptionInfo& o);
static inline bool operator==(const ::vendor::qti::hardware::radio::qtiradio::V2_4::ImsiEncryptionInfo& lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_4::ImsiEncryptionInfo& rhs);
static inline bool operator!=(const ::vendor::qti::hardware::radio::qtiradio::V2_4::ImsiEncryptionInfo& lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_4::ImsiEncryptionInfo& rhs);

//
// type header definitions for package
//

template<>
inline std::string toString<::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType::UNKNOWN) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType::UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "UNKNOWN";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType::UNKNOWN;
    }
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType::EPDG) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType::EPDG)) {
        os += (first ? "" : " | ");
        os += "EPDG";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType::EPDG;
    }
    if ((o & ::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType::WLAN) == static_cast<int32_t>(::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType::WLAN)) {
        os += (first ? "" : " | ");
        os += "WLAN";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType::WLAN;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType::UNKNOWN) {
        return "UNKNOWN";
    }
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType::EPDG) {
        return "EPDG";
    }
    if (o == ::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType::WLAN) {
        return "WLAN";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

static inline std::string toString(const ::vendor::qti::hardware::radio::qtiradio::V2_4::ImsiEncryptionInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".base = ";
    os += ::android::hardware::radio::V1_1::toString(o.base);
    os += ", .keyType = ";
    os += ::vendor::qti::hardware::radio::qtiradio::V2_4::toString(o.keyType);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::qtiradio::V2_4::ImsiEncryptionInfo& lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_4::ImsiEncryptionInfo& rhs) {
    if (lhs.base != rhs.base) {
        return false;
    }
    if (lhs.keyType != rhs.keyType) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::qtiradio::V2_4::ImsiEncryptionInfo& lhs, const ::vendor::qti::hardware::radio::qtiradio::V2_4::ImsiEncryptionInfo& rhs){
    return !(lhs == rhs);
}


}  // namespace V2_4
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
template<> constexpr std::array<::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType, 3> hidl_enum_values<::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType> = {
    ::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType::UNKNOWN,
    ::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType::EPDG,
    ::vendor::qti::hardware::radio::qtiradio::V2_4::PublicKeyType::WLAN,
};
}  // namespace details
}  // namespace hardware
}  // namespace android


#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_4_TYPES_H
