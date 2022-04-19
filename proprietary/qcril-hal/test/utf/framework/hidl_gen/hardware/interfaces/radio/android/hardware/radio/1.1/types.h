#ifndef HIDL_GENERATED_ANDROID_HARDWARE_RADIO_V1_1_TYPES_H
#define HIDL_GENERATED_ANDROID_HARDWARE_RADIO_V1_1_TYPES_H

#include <android/hardware/radio/1.0/types.h>

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace android {
namespace hardware {
namespace radio {
namespace V1_1 {

// Forward declaration for forward reference support:
enum class CardPowerState : int32_t;
enum class RadioAccessNetworks : int32_t;
enum class GeranBands : int32_t;
enum class UtranBands : int32_t;
enum class EutranBands : int32_t;
enum class ScanType : int32_t;
enum class ScanStatus : int32_t;
enum class KeepaliveType : int32_t;
enum class KeepaliveStatusCode : int32_t;
struct RadioAccessSpecifier;
struct NetworkScanRequest;
struct NetworkScanResult;
struct KeepaliveRequest;
struct KeepaliveStatus;
struct ImsiEncryptionInfo;

enum class CardPowerState : int32_t {
    POWER_DOWN = 0,
    POWER_UP = 1, // (::android::hardware::radio::V1_1::CardPowerState.POWER_DOWN implicitly + 1)
    POWER_UP_PASS_THROUGH = 2, // (::android::hardware::radio::V1_1::CardPowerState.POWER_UP implicitly + 1)
};

enum class RadioAccessNetworks : int32_t {
    GERAN = 1,
    UTRAN = 2,
    EUTRAN = 3,
};

enum class GeranBands : int32_t {
    BAND_T380 = 1,
    BAND_T410 = 2,
    BAND_450 = 3,
    BAND_480 = 4,
    BAND_710 = 5,
    BAND_750 = 6,
    BAND_T810 = 7,
    BAND_850 = 8,
    BAND_P900 = 9,
    BAND_E900 = 10,
    BAND_R900 = 11,
    BAND_DCS1800 = 12,
    BAND_PCS1900 = 13,
    BAND_ER900 = 14,
};

enum class UtranBands : int32_t {
    BAND_1 = 1,
    BAND_2 = 2,
    BAND_3 = 3,
    BAND_4 = 4,
    BAND_5 = 5,
    BAND_6 = 6,
    BAND_7 = 7,
    BAND_8 = 8,
    BAND_9 = 9,
    BAND_10 = 10,
    BAND_11 = 11,
    BAND_12 = 12,
    BAND_13 = 13,
    BAND_14 = 14,
    BAND_19 = 19,
    BAND_20 = 20,
    BAND_21 = 21,
    BAND_22 = 22,
    BAND_25 = 25,
    BAND_26 = 26,
};

enum class EutranBands : int32_t {
    BAND_1 = 1,
    BAND_2 = 2,
    BAND_3 = 3,
    BAND_4 = 4,
    BAND_5 = 5,
    BAND_6 = 6,
    BAND_7 = 7,
    BAND_8 = 8,
    BAND_9 = 9,
    BAND_10 = 10,
    BAND_11 = 11,
    BAND_12 = 12,
    BAND_13 = 13,
    BAND_14 = 14,
    BAND_17 = 17,
    BAND_18 = 18,
    BAND_19 = 19,
    BAND_20 = 20,
    BAND_21 = 21,
    BAND_22 = 22,
    BAND_23 = 23,
    BAND_24 = 24,
    BAND_25 = 25,
    BAND_26 = 26,
    BAND_27 = 27,
    BAND_28 = 28,
    BAND_30 = 30,
    BAND_31 = 31,
    BAND_33 = 33,
    BAND_34 = 34,
    BAND_35 = 35,
    BAND_36 = 36,
    BAND_37 = 37,
    BAND_38 = 38,
    BAND_39 = 39,
    BAND_40 = 40,
    BAND_41 = 41,
    BAND_42 = 42,
    BAND_43 = 43,
    BAND_44 = 44,
    BAND_45 = 45,
    BAND_46 = 46,
    BAND_47 = 47,
    BAND_48 = 48,
    BAND_65 = 65,
    BAND_66 = 66,
    BAND_68 = 68,
    BAND_70 = 70,
};

enum class ScanType : int32_t {
    ONE_SHOT = 0,
    PERIODIC = 1,
};

enum class ScanStatus : int32_t {
    PARTIAL = 1,
    COMPLETE = 2,
};

enum class KeepaliveType : int32_t {
    NATT_IPV4 = 0,
    NATT_IPV6 = 1,
};

enum class KeepaliveStatusCode : int32_t {
    ACTIVE = 0,
    INACTIVE = 1, // (::android::hardware::radio::V1_1::KeepaliveStatusCode.ACTIVE implicitly + 1)
    PENDING = 2, // (::android::hardware::radio::V1_1::KeepaliveStatusCode.INACTIVE implicitly + 1)
};

struct RadioAccessSpecifier final {
    ::android::hardware::radio::V1_1::RadioAccessNetworks radioAccessNetwork __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_1::GeranBands> geranBands __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_1::UtranBands> utranBands __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_1::EutranBands> eutranBands __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<int32_t> channels __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_1::RadioAccessSpecifier, radioAccessNetwork) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_1::RadioAccessSpecifier, geranBands) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_1::RadioAccessSpecifier, utranBands) == 24, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_1::RadioAccessSpecifier, eutranBands) == 40, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_1::RadioAccessSpecifier, channels) == 56, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_1::RadioAccessSpecifier) == 72, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_1::RadioAccessSpecifier) == 8, "wrong alignment");

struct NetworkScanRequest final {
    ::android::hardware::radio::V1_1::ScanType type __attribute__ ((aligned(4)));
    int32_t interval __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_1::RadioAccessSpecifier> specifiers __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_1::NetworkScanRequest, type) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_1::NetworkScanRequest, interval) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_1::NetworkScanRequest, specifiers) == 8, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_1::NetworkScanRequest) == 24, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_1::NetworkScanRequest) == 8, "wrong alignment");

struct NetworkScanResult final {
    ::android::hardware::radio::V1_1::ScanStatus status __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_0::RadioError error __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::CellInfo> networkInfos __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_1::NetworkScanResult, status) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_1::NetworkScanResult, error) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_1::NetworkScanResult, networkInfos) == 8, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_1::NetworkScanResult) == 24, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_1::NetworkScanResult) == 8, "wrong alignment");

struct KeepaliveRequest final {
    ::android::hardware::radio::V1_1::KeepaliveType type __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<uint8_t> sourceAddress __attribute__ ((aligned(8)));
    int32_t sourcePort __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<uint8_t> destinationAddress __attribute__ ((aligned(8)));
    int32_t destinationPort __attribute__ ((aligned(4)));
    int32_t maxKeepaliveIntervalMillis __attribute__ ((aligned(4)));
    int32_t cid __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_1::KeepaliveRequest, type) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_1::KeepaliveRequest, sourceAddress) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_1::KeepaliveRequest, sourcePort) == 24, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_1::KeepaliveRequest, destinationAddress) == 32, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_1::KeepaliveRequest, destinationPort) == 48, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_1::KeepaliveRequest, maxKeepaliveIntervalMillis) == 52, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_1::KeepaliveRequest, cid) == 56, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_1::KeepaliveRequest) == 64, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_1::KeepaliveRequest) == 8, "wrong alignment");

struct KeepaliveStatus final {
    int32_t sessionHandle __attribute__ ((aligned(4)));
    ::android::hardware::radio::V1_1::KeepaliveStatusCode code __attribute__ ((aligned(4)));
};

static_assert(offsetof(::android::hardware::radio::V1_1::KeepaliveStatus, sessionHandle) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_1::KeepaliveStatus, code) == 4, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_1::KeepaliveStatus) == 8, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_1::KeepaliveStatus) == 4, "wrong alignment");

struct ImsiEncryptionInfo final {
    ::android::hardware::hidl_string mcc __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string mnc __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<uint8_t> carrierKey __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string keyIdentifier __attribute__ ((aligned(8)));
    int64_t expirationTime __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::V1_1::ImsiEncryptionInfo, mcc) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_1::ImsiEncryptionInfo, mnc) == 16, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_1::ImsiEncryptionInfo, carrierKey) == 32, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_1::ImsiEncryptionInfo, keyIdentifier) == 48, "wrong offset");
static_assert(offsetof(::android::hardware::radio::V1_1::ImsiEncryptionInfo, expirationTime) == 64, "wrong offset");
static_assert(sizeof(::android::hardware::radio::V1_1::ImsiEncryptionInfo) == 72, "wrong size");
static_assert(__alignof(::android::hardware::radio::V1_1::ImsiEncryptionInfo) == 8, "wrong alignment");

constexpr int32_t operator|(const ::android::hardware::radio::V1_1::CardPowerState lhs, const ::android::hardware::radio::V1_1::CardPowerState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_1::CardPowerState rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_1::CardPowerState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_1::CardPowerState lhs, const ::android::hardware::radio::V1_1::CardPowerState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_1::CardPowerState rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_1::CardPowerState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_1::CardPowerState e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_1::CardPowerState e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_1::CardPowerState>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_1::CardPowerState> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_1::CardPowerState::POWER_DOWN) == static_cast<int32_t>(::android::hardware::radio::V1_1::CardPowerState::POWER_DOWN)) {
        os += (first ? "" : " | ");
        os += "POWER_DOWN";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::CardPowerState::POWER_DOWN;
    }
    if ((o & ::android::hardware::radio::V1_1::CardPowerState::POWER_UP) == static_cast<int32_t>(::android::hardware::radio::V1_1::CardPowerState::POWER_UP)) {
        os += (first ? "" : " | ");
        os += "POWER_UP";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::CardPowerState::POWER_UP;
    }
    if ((o & ::android::hardware::radio::V1_1::CardPowerState::POWER_UP_PASS_THROUGH) == static_cast<int32_t>(::android::hardware::radio::V1_1::CardPowerState::POWER_UP_PASS_THROUGH)) {
        os += (first ? "" : " | ");
        os += "POWER_UP_PASS_THROUGH";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::CardPowerState::POWER_UP_PASS_THROUGH;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_1::CardPowerState o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_1::CardPowerState::POWER_DOWN) {
        return "POWER_DOWN";
    }
    if (o == ::android::hardware::radio::V1_1::CardPowerState::POWER_UP) {
        return "POWER_UP";
    }
    if (o == ::android::hardware::radio::V1_1::CardPowerState::POWER_UP_PASS_THROUGH) {
        return "POWER_UP_PASS_THROUGH";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_1::RadioAccessNetworks lhs, const ::android::hardware::radio::V1_1::RadioAccessNetworks rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_1::RadioAccessNetworks rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_1::RadioAccessNetworks lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_1::RadioAccessNetworks lhs, const ::android::hardware::radio::V1_1::RadioAccessNetworks rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_1::RadioAccessNetworks rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_1::RadioAccessNetworks lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_1::RadioAccessNetworks e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_1::RadioAccessNetworks e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_1::RadioAccessNetworks>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_1::RadioAccessNetworks> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_1::RadioAccessNetworks::GERAN) == static_cast<int32_t>(::android::hardware::radio::V1_1::RadioAccessNetworks::GERAN)) {
        os += (first ? "" : " | ");
        os += "GERAN";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::RadioAccessNetworks::GERAN;
    }
    if ((o & ::android::hardware::radio::V1_1::RadioAccessNetworks::UTRAN) == static_cast<int32_t>(::android::hardware::radio::V1_1::RadioAccessNetworks::UTRAN)) {
        os += (first ? "" : " | ");
        os += "UTRAN";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::RadioAccessNetworks::UTRAN;
    }
    if ((o & ::android::hardware::radio::V1_1::RadioAccessNetworks::EUTRAN) == static_cast<int32_t>(::android::hardware::radio::V1_1::RadioAccessNetworks::EUTRAN)) {
        os += (first ? "" : " | ");
        os += "EUTRAN";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::RadioAccessNetworks::EUTRAN;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_1::RadioAccessNetworks o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_1::RadioAccessNetworks::GERAN) {
        return "GERAN";
    }
    if (o == ::android::hardware::radio::V1_1::RadioAccessNetworks::UTRAN) {
        return "UTRAN";
    }
    if (o == ::android::hardware::radio::V1_1::RadioAccessNetworks::EUTRAN) {
        return "EUTRAN";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_1::GeranBands lhs, const ::android::hardware::radio::V1_1::GeranBands rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_1::GeranBands rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_1::GeranBands lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_1::GeranBands lhs, const ::android::hardware::radio::V1_1::GeranBands rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_1::GeranBands rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_1::GeranBands lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_1::GeranBands e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_1::GeranBands e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_1::GeranBands>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_1::GeranBands> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_1::GeranBands::BAND_T380) == static_cast<int32_t>(::android::hardware::radio::V1_1::GeranBands::BAND_T380)) {
        os += (first ? "" : " | ");
        os += "BAND_T380";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::GeranBands::BAND_T380;
    }
    if ((o & ::android::hardware::radio::V1_1::GeranBands::BAND_T410) == static_cast<int32_t>(::android::hardware::radio::V1_1::GeranBands::BAND_T410)) {
        os += (first ? "" : " | ");
        os += "BAND_T410";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::GeranBands::BAND_T410;
    }
    if ((o & ::android::hardware::radio::V1_1::GeranBands::BAND_450) == static_cast<int32_t>(::android::hardware::radio::V1_1::GeranBands::BAND_450)) {
        os += (first ? "" : " | ");
        os += "BAND_450";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::GeranBands::BAND_450;
    }
    if ((o & ::android::hardware::radio::V1_1::GeranBands::BAND_480) == static_cast<int32_t>(::android::hardware::radio::V1_1::GeranBands::BAND_480)) {
        os += (first ? "" : " | ");
        os += "BAND_480";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::GeranBands::BAND_480;
    }
    if ((o & ::android::hardware::radio::V1_1::GeranBands::BAND_710) == static_cast<int32_t>(::android::hardware::radio::V1_1::GeranBands::BAND_710)) {
        os += (first ? "" : " | ");
        os += "BAND_710";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::GeranBands::BAND_710;
    }
    if ((o & ::android::hardware::radio::V1_1::GeranBands::BAND_750) == static_cast<int32_t>(::android::hardware::radio::V1_1::GeranBands::BAND_750)) {
        os += (first ? "" : " | ");
        os += "BAND_750";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::GeranBands::BAND_750;
    }
    if ((o & ::android::hardware::radio::V1_1::GeranBands::BAND_T810) == static_cast<int32_t>(::android::hardware::radio::V1_1::GeranBands::BAND_T810)) {
        os += (first ? "" : " | ");
        os += "BAND_T810";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::GeranBands::BAND_T810;
    }
    if ((o & ::android::hardware::radio::V1_1::GeranBands::BAND_850) == static_cast<int32_t>(::android::hardware::radio::V1_1::GeranBands::BAND_850)) {
        os += (first ? "" : " | ");
        os += "BAND_850";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::GeranBands::BAND_850;
    }
    if ((o & ::android::hardware::radio::V1_1::GeranBands::BAND_P900) == static_cast<int32_t>(::android::hardware::radio::V1_1::GeranBands::BAND_P900)) {
        os += (first ? "" : " | ");
        os += "BAND_P900";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::GeranBands::BAND_P900;
    }
    if ((o & ::android::hardware::radio::V1_1::GeranBands::BAND_E900) == static_cast<int32_t>(::android::hardware::radio::V1_1::GeranBands::BAND_E900)) {
        os += (first ? "" : " | ");
        os += "BAND_E900";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::GeranBands::BAND_E900;
    }
    if ((o & ::android::hardware::radio::V1_1::GeranBands::BAND_R900) == static_cast<int32_t>(::android::hardware::radio::V1_1::GeranBands::BAND_R900)) {
        os += (first ? "" : " | ");
        os += "BAND_R900";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::GeranBands::BAND_R900;
    }
    if ((o & ::android::hardware::radio::V1_1::GeranBands::BAND_DCS1800) == static_cast<int32_t>(::android::hardware::radio::V1_1::GeranBands::BAND_DCS1800)) {
        os += (first ? "" : " | ");
        os += "BAND_DCS1800";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::GeranBands::BAND_DCS1800;
    }
    if ((o & ::android::hardware::radio::V1_1::GeranBands::BAND_PCS1900) == static_cast<int32_t>(::android::hardware::radio::V1_1::GeranBands::BAND_PCS1900)) {
        os += (first ? "" : " | ");
        os += "BAND_PCS1900";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::GeranBands::BAND_PCS1900;
    }
    if ((o & ::android::hardware::radio::V1_1::GeranBands::BAND_ER900) == static_cast<int32_t>(::android::hardware::radio::V1_1::GeranBands::BAND_ER900)) {
        os += (first ? "" : " | ");
        os += "BAND_ER900";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::GeranBands::BAND_ER900;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_1::GeranBands o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_1::GeranBands::BAND_T380) {
        return "BAND_T380";
    }
    if (o == ::android::hardware::radio::V1_1::GeranBands::BAND_T410) {
        return "BAND_T410";
    }
    if (o == ::android::hardware::radio::V1_1::GeranBands::BAND_450) {
        return "BAND_450";
    }
    if (o == ::android::hardware::radio::V1_1::GeranBands::BAND_480) {
        return "BAND_480";
    }
    if (o == ::android::hardware::radio::V1_1::GeranBands::BAND_710) {
        return "BAND_710";
    }
    if (o == ::android::hardware::radio::V1_1::GeranBands::BAND_750) {
        return "BAND_750";
    }
    if (o == ::android::hardware::radio::V1_1::GeranBands::BAND_T810) {
        return "BAND_T810";
    }
    if (o == ::android::hardware::radio::V1_1::GeranBands::BAND_850) {
        return "BAND_850";
    }
    if (o == ::android::hardware::radio::V1_1::GeranBands::BAND_P900) {
        return "BAND_P900";
    }
    if (o == ::android::hardware::radio::V1_1::GeranBands::BAND_E900) {
        return "BAND_E900";
    }
    if (o == ::android::hardware::radio::V1_1::GeranBands::BAND_R900) {
        return "BAND_R900";
    }
    if (o == ::android::hardware::radio::V1_1::GeranBands::BAND_DCS1800) {
        return "BAND_DCS1800";
    }
    if (o == ::android::hardware::radio::V1_1::GeranBands::BAND_PCS1900) {
        return "BAND_PCS1900";
    }
    if (o == ::android::hardware::radio::V1_1::GeranBands::BAND_ER900) {
        return "BAND_ER900";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_1::UtranBands lhs, const ::android::hardware::radio::V1_1::UtranBands rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_1::UtranBands rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_1::UtranBands lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_1::UtranBands lhs, const ::android::hardware::radio::V1_1::UtranBands rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_1::UtranBands rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_1::UtranBands lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_1::UtranBands e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_1::UtranBands e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_1::UtranBands>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_1::UtranBands> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_1::UtranBands::BAND_1) == static_cast<int32_t>(::android::hardware::radio::V1_1::UtranBands::BAND_1)) {
        os += (first ? "" : " | ");
        os += "BAND_1";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::UtranBands::BAND_1;
    }
    if ((o & ::android::hardware::radio::V1_1::UtranBands::BAND_2) == static_cast<int32_t>(::android::hardware::radio::V1_1::UtranBands::BAND_2)) {
        os += (first ? "" : " | ");
        os += "BAND_2";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::UtranBands::BAND_2;
    }
    if ((o & ::android::hardware::radio::V1_1::UtranBands::BAND_3) == static_cast<int32_t>(::android::hardware::radio::V1_1::UtranBands::BAND_3)) {
        os += (first ? "" : " | ");
        os += "BAND_3";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::UtranBands::BAND_3;
    }
    if ((o & ::android::hardware::radio::V1_1::UtranBands::BAND_4) == static_cast<int32_t>(::android::hardware::radio::V1_1::UtranBands::BAND_4)) {
        os += (first ? "" : " | ");
        os += "BAND_4";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::UtranBands::BAND_4;
    }
    if ((o & ::android::hardware::radio::V1_1::UtranBands::BAND_5) == static_cast<int32_t>(::android::hardware::radio::V1_1::UtranBands::BAND_5)) {
        os += (first ? "" : " | ");
        os += "BAND_5";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::UtranBands::BAND_5;
    }
    if ((o & ::android::hardware::radio::V1_1::UtranBands::BAND_6) == static_cast<int32_t>(::android::hardware::radio::V1_1::UtranBands::BAND_6)) {
        os += (first ? "" : " | ");
        os += "BAND_6";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::UtranBands::BAND_6;
    }
    if ((o & ::android::hardware::radio::V1_1::UtranBands::BAND_7) == static_cast<int32_t>(::android::hardware::radio::V1_1::UtranBands::BAND_7)) {
        os += (first ? "" : " | ");
        os += "BAND_7";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::UtranBands::BAND_7;
    }
    if ((o & ::android::hardware::radio::V1_1::UtranBands::BAND_8) == static_cast<int32_t>(::android::hardware::radio::V1_1::UtranBands::BAND_8)) {
        os += (first ? "" : " | ");
        os += "BAND_8";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::UtranBands::BAND_8;
    }
    if ((o & ::android::hardware::radio::V1_1::UtranBands::BAND_9) == static_cast<int32_t>(::android::hardware::radio::V1_1::UtranBands::BAND_9)) {
        os += (first ? "" : " | ");
        os += "BAND_9";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::UtranBands::BAND_9;
    }
    if ((o & ::android::hardware::radio::V1_1::UtranBands::BAND_10) == static_cast<int32_t>(::android::hardware::radio::V1_1::UtranBands::BAND_10)) {
        os += (first ? "" : " | ");
        os += "BAND_10";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::UtranBands::BAND_10;
    }
    if ((o & ::android::hardware::radio::V1_1::UtranBands::BAND_11) == static_cast<int32_t>(::android::hardware::radio::V1_1::UtranBands::BAND_11)) {
        os += (first ? "" : " | ");
        os += "BAND_11";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::UtranBands::BAND_11;
    }
    if ((o & ::android::hardware::radio::V1_1::UtranBands::BAND_12) == static_cast<int32_t>(::android::hardware::radio::V1_1::UtranBands::BAND_12)) {
        os += (first ? "" : " | ");
        os += "BAND_12";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::UtranBands::BAND_12;
    }
    if ((o & ::android::hardware::radio::V1_1::UtranBands::BAND_13) == static_cast<int32_t>(::android::hardware::radio::V1_1::UtranBands::BAND_13)) {
        os += (first ? "" : " | ");
        os += "BAND_13";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::UtranBands::BAND_13;
    }
    if ((o & ::android::hardware::radio::V1_1::UtranBands::BAND_14) == static_cast<int32_t>(::android::hardware::radio::V1_1::UtranBands::BAND_14)) {
        os += (first ? "" : " | ");
        os += "BAND_14";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::UtranBands::BAND_14;
    }
    if ((o & ::android::hardware::radio::V1_1::UtranBands::BAND_19) == static_cast<int32_t>(::android::hardware::radio::V1_1::UtranBands::BAND_19)) {
        os += (first ? "" : " | ");
        os += "BAND_19";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::UtranBands::BAND_19;
    }
    if ((o & ::android::hardware::radio::V1_1::UtranBands::BAND_20) == static_cast<int32_t>(::android::hardware::radio::V1_1::UtranBands::BAND_20)) {
        os += (first ? "" : " | ");
        os += "BAND_20";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::UtranBands::BAND_20;
    }
    if ((o & ::android::hardware::radio::V1_1::UtranBands::BAND_21) == static_cast<int32_t>(::android::hardware::radio::V1_1::UtranBands::BAND_21)) {
        os += (first ? "" : " | ");
        os += "BAND_21";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::UtranBands::BAND_21;
    }
    if ((o & ::android::hardware::radio::V1_1::UtranBands::BAND_22) == static_cast<int32_t>(::android::hardware::radio::V1_1::UtranBands::BAND_22)) {
        os += (first ? "" : " | ");
        os += "BAND_22";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::UtranBands::BAND_22;
    }
    if ((o & ::android::hardware::radio::V1_1::UtranBands::BAND_25) == static_cast<int32_t>(::android::hardware::radio::V1_1::UtranBands::BAND_25)) {
        os += (first ? "" : " | ");
        os += "BAND_25";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::UtranBands::BAND_25;
    }
    if ((o & ::android::hardware::radio::V1_1::UtranBands::BAND_26) == static_cast<int32_t>(::android::hardware::radio::V1_1::UtranBands::BAND_26)) {
        os += (first ? "" : " | ");
        os += "BAND_26";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::UtranBands::BAND_26;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_1::UtranBands o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_1::UtranBands::BAND_1) {
        return "BAND_1";
    }
    if (o == ::android::hardware::radio::V1_1::UtranBands::BAND_2) {
        return "BAND_2";
    }
    if (o == ::android::hardware::radio::V1_1::UtranBands::BAND_3) {
        return "BAND_3";
    }
    if (o == ::android::hardware::radio::V1_1::UtranBands::BAND_4) {
        return "BAND_4";
    }
    if (o == ::android::hardware::radio::V1_1::UtranBands::BAND_5) {
        return "BAND_5";
    }
    if (o == ::android::hardware::radio::V1_1::UtranBands::BAND_6) {
        return "BAND_6";
    }
    if (o == ::android::hardware::radio::V1_1::UtranBands::BAND_7) {
        return "BAND_7";
    }
    if (o == ::android::hardware::radio::V1_1::UtranBands::BAND_8) {
        return "BAND_8";
    }
    if (o == ::android::hardware::radio::V1_1::UtranBands::BAND_9) {
        return "BAND_9";
    }
    if (o == ::android::hardware::radio::V1_1::UtranBands::BAND_10) {
        return "BAND_10";
    }
    if (o == ::android::hardware::radio::V1_1::UtranBands::BAND_11) {
        return "BAND_11";
    }
    if (o == ::android::hardware::radio::V1_1::UtranBands::BAND_12) {
        return "BAND_12";
    }
    if (o == ::android::hardware::radio::V1_1::UtranBands::BAND_13) {
        return "BAND_13";
    }
    if (o == ::android::hardware::radio::V1_1::UtranBands::BAND_14) {
        return "BAND_14";
    }
    if (o == ::android::hardware::radio::V1_1::UtranBands::BAND_19) {
        return "BAND_19";
    }
    if (o == ::android::hardware::radio::V1_1::UtranBands::BAND_20) {
        return "BAND_20";
    }
    if (o == ::android::hardware::radio::V1_1::UtranBands::BAND_21) {
        return "BAND_21";
    }
    if (o == ::android::hardware::radio::V1_1::UtranBands::BAND_22) {
        return "BAND_22";
    }
    if (o == ::android::hardware::radio::V1_1::UtranBands::BAND_25) {
        return "BAND_25";
    }
    if (o == ::android::hardware::radio::V1_1::UtranBands::BAND_26) {
        return "BAND_26";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_1::EutranBands lhs, const ::android::hardware::radio::V1_1::EutranBands rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_1::EutranBands rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_1::EutranBands lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_1::EutranBands lhs, const ::android::hardware::radio::V1_1::EutranBands rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_1::EutranBands rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_1::EutranBands lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_1::EutranBands e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_1::EutranBands e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_1::EutranBands>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_1::EutranBands> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_1) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_1)) {
        os += (first ? "" : " | ");
        os += "BAND_1";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_1;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_2) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_2)) {
        os += (first ? "" : " | ");
        os += "BAND_2";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_2;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_3) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_3)) {
        os += (first ? "" : " | ");
        os += "BAND_3";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_3;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_4) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_4)) {
        os += (first ? "" : " | ");
        os += "BAND_4";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_4;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_5) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_5)) {
        os += (first ? "" : " | ");
        os += "BAND_5";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_5;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_6) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_6)) {
        os += (first ? "" : " | ");
        os += "BAND_6";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_6;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_7) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_7)) {
        os += (first ? "" : " | ");
        os += "BAND_7";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_7;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_8) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_8)) {
        os += (first ? "" : " | ");
        os += "BAND_8";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_8;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_9) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_9)) {
        os += (first ? "" : " | ");
        os += "BAND_9";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_9;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_10) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_10)) {
        os += (first ? "" : " | ");
        os += "BAND_10";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_10;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_11) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_11)) {
        os += (first ? "" : " | ");
        os += "BAND_11";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_11;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_12) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_12)) {
        os += (first ? "" : " | ");
        os += "BAND_12";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_12;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_13) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_13)) {
        os += (first ? "" : " | ");
        os += "BAND_13";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_13;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_14) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_14)) {
        os += (first ? "" : " | ");
        os += "BAND_14";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_14;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_17) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_17)) {
        os += (first ? "" : " | ");
        os += "BAND_17";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_17;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_18) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_18)) {
        os += (first ? "" : " | ");
        os += "BAND_18";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_18;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_19) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_19)) {
        os += (first ? "" : " | ");
        os += "BAND_19";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_19;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_20) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_20)) {
        os += (first ? "" : " | ");
        os += "BAND_20";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_20;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_21) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_21)) {
        os += (first ? "" : " | ");
        os += "BAND_21";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_21;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_22) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_22)) {
        os += (first ? "" : " | ");
        os += "BAND_22";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_22;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_23) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_23)) {
        os += (first ? "" : " | ");
        os += "BAND_23";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_23;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_24) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_24)) {
        os += (first ? "" : " | ");
        os += "BAND_24";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_24;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_25) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_25)) {
        os += (first ? "" : " | ");
        os += "BAND_25";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_25;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_26) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_26)) {
        os += (first ? "" : " | ");
        os += "BAND_26";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_26;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_27) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_27)) {
        os += (first ? "" : " | ");
        os += "BAND_27";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_27;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_28) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_28)) {
        os += (first ? "" : " | ");
        os += "BAND_28";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_28;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_30) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_30)) {
        os += (first ? "" : " | ");
        os += "BAND_30";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_30;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_31) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_31)) {
        os += (first ? "" : " | ");
        os += "BAND_31";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_31;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_33) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_33)) {
        os += (first ? "" : " | ");
        os += "BAND_33";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_33;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_34) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_34)) {
        os += (first ? "" : " | ");
        os += "BAND_34";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_34;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_35) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_35)) {
        os += (first ? "" : " | ");
        os += "BAND_35";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_35;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_36) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_36)) {
        os += (first ? "" : " | ");
        os += "BAND_36";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_36;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_37) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_37)) {
        os += (first ? "" : " | ");
        os += "BAND_37";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_37;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_38) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_38)) {
        os += (first ? "" : " | ");
        os += "BAND_38";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_38;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_39) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_39)) {
        os += (first ? "" : " | ");
        os += "BAND_39";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_39;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_40) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_40)) {
        os += (first ? "" : " | ");
        os += "BAND_40";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_40;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_41) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_41)) {
        os += (first ? "" : " | ");
        os += "BAND_41";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_41;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_42) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_42)) {
        os += (first ? "" : " | ");
        os += "BAND_42";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_42;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_43) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_43)) {
        os += (first ? "" : " | ");
        os += "BAND_43";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_43;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_44) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_44)) {
        os += (first ? "" : " | ");
        os += "BAND_44";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_44;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_45) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_45)) {
        os += (first ? "" : " | ");
        os += "BAND_45";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_45;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_46) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_46)) {
        os += (first ? "" : " | ");
        os += "BAND_46";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_46;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_47) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_47)) {
        os += (first ? "" : " | ");
        os += "BAND_47";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_47;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_48) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_48)) {
        os += (first ? "" : " | ");
        os += "BAND_48";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_48;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_65) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_65)) {
        os += (first ? "" : " | ");
        os += "BAND_65";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_65;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_66) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_66)) {
        os += (first ? "" : " | ");
        os += "BAND_66";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_66;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_68) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_68)) {
        os += (first ? "" : " | ");
        os += "BAND_68";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_68;
    }
    if ((o & ::android::hardware::radio::V1_1::EutranBands::BAND_70) == static_cast<int32_t>(::android::hardware::radio::V1_1::EutranBands::BAND_70)) {
        os += (first ? "" : " | ");
        os += "BAND_70";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::EutranBands::BAND_70;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_1::EutranBands o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_1) {
        return "BAND_1";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_2) {
        return "BAND_2";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_3) {
        return "BAND_3";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_4) {
        return "BAND_4";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_5) {
        return "BAND_5";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_6) {
        return "BAND_6";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_7) {
        return "BAND_7";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_8) {
        return "BAND_8";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_9) {
        return "BAND_9";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_10) {
        return "BAND_10";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_11) {
        return "BAND_11";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_12) {
        return "BAND_12";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_13) {
        return "BAND_13";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_14) {
        return "BAND_14";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_17) {
        return "BAND_17";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_18) {
        return "BAND_18";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_19) {
        return "BAND_19";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_20) {
        return "BAND_20";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_21) {
        return "BAND_21";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_22) {
        return "BAND_22";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_23) {
        return "BAND_23";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_24) {
        return "BAND_24";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_25) {
        return "BAND_25";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_26) {
        return "BAND_26";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_27) {
        return "BAND_27";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_28) {
        return "BAND_28";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_30) {
        return "BAND_30";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_31) {
        return "BAND_31";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_33) {
        return "BAND_33";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_34) {
        return "BAND_34";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_35) {
        return "BAND_35";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_36) {
        return "BAND_36";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_37) {
        return "BAND_37";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_38) {
        return "BAND_38";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_39) {
        return "BAND_39";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_40) {
        return "BAND_40";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_41) {
        return "BAND_41";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_42) {
        return "BAND_42";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_43) {
        return "BAND_43";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_44) {
        return "BAND_44";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_45) {
        return "BAND_45";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_46) {
        return "BAND_46";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_47) {
        return "BAND_47";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_48) {
        return "BAND_48";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_65) {
        return "BAND_65";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_66) {
        return "BAND_66";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_68) {
        return "BAND_68";
    }
    if (o == ::android::hardware::radio::V1_1::EutranBands::BAND_70) {
        return "BAND_70";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_1::ScanType lhs, const ::android::hardware::radio::V1_1::ScanType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_1::ScanType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_1::ScanType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_1::ScanType lhs, const ::android::hardware::radio::V1_1::ScanType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_1::ScanType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_1::ScanType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_1::ScanType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_1::ScanType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_1::ScanType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_1::ScanType> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_1::ScanType::ONE_SHOT) == static_cast<int32_t>(::android::hardware::radio::V1_1::ScanType::ONE_SHOT)) {
        os += (first ? "" : " | ");
        os += "ONE_SHOT";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::ScanType::ONE_SHOT;
    }
    if ((o & ::android::hardware::radio::V1_1::ScanType::PERIODIC) == static_cast<int32_t>(::android::hardware::radio::V1_1::ScanType::PERIODIC)) {
        os += (first ? "" : " | ");
        os += "PERIODIC";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::ScanType::PERIODIC;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_1::ScanType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_1::ScanType::ONE_SHOT) {
        return "ONE_SHOT";
    }
    if (o == ::android::hardware::radio::V1_1::ScanType::PERIODIC) {
        return "PERIODIC";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_1::ScanStatus lhs, const ::android::hardware::radio::V1_1::ScanStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_1::ScanStatus rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_1::ScanStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_1::ScanStatus lhs, const ::android::hardware::radio::V1_1::ScanStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_1::ScanStatus rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_1::ScanStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_1::ScanStatus e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_1::ScanStatus e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_1::ScanStatus>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_1::ScanStatus> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_1::ScanStatus::PARTIAL) == static_cast<int32_t>(::android::hardware::radio::V1_1::ScanStatus::PARTIAL)) {
        os += (first ? "" : " | ");
        os += "PARTIAL";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::ScanStatus::PARTIAL;
    }
    if ((o & ::android::hardware::radio::V1_1::ScanStatus::COMPLETE) == static_cast<int32_t>(::android::hardware::radio::V1_1::ScanStatus::COMPLETE)) {
        os += (first ? "" : " | ");
        os += "COMPLETE";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::ScanStatus::COMPLETE;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_1::ScanStatus o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_1::ScanStatus::PARTIAL) {
        return "PARTIAL";
    }
    if (o == ::android::hardware::radio::V1_1::ScanStatus::COMPLETE) {
        return "COMPLETE";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_1::KeepaliveType lhs, const ::android::hardware::radio::V1_1::KeepaliveType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_1::KeepaliveType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_1::KeepaliveType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_1::KeepaliveType lhs, const ::android::hardware::radio::V1_1::KeepaliveType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_1::KeepaliveType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_1::KeepaliveType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_1::KeepaliveType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_1::KeepaliveType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_1::KeepaliveType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_1::KeepaliveType> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_1::KeepaliveType::NATT_IPV4) == static_cast<int32_t>(::android::hardware::radio::V1_1::KeepaliveType::NATT_IPV4)) {
        os += (first ? "" : " | ");
        os += "NATT_IPV4";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::KeepaliveType::NATT_IPV4;
    }
    if ((o & ::android::hardware::radio::V1_1::KeepaliveType::NATT_IPV6) == static_cast<int32_t>(::android::hardware::radio::V1_1::KeepaliveType::NATT_IPV6)) {
        os += (first ? "" : " | ");
        os += "NATT_IPV6";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::KeepaliveType::NATT_IPV6;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_1::KeepaliveType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_1::KeepaliveType::NATT_IPV4) {
        return "NATT_IPV4";
    }
    if (o == ::android::hardware::radio::V1_1::KeepaliveType::NATT_IPV6) {
        return "NATT_IPV6";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_1::KeepaliveStatusCode lhs, const ::android::hardware::radio::V1_1::KeepaliveStatusCode rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::V1_1::KeepaliveStatusCode rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::V1_1::KeepaliveStatusCode lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_1::KeepaliveStatusCode lhs, const ::android::hardware::radio::V1_1::KeepaliveStatusCode rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::V1_1::KeepaliveStatusCode rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::V1_1::KeepaliveStatusCode lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::V1_1::KeepaliveStatusCode e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::V1_1::KeepaliveStatusCode e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::V1_1::KeepaliveStatusCode>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_1::KeepaliveStatusCode> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::V1_1::KeepaliveStatusCode::ACTIVE) == static_cast<int32_t>(::android::hardware::radio::V1_1::KeepaliveStatusCode::ACTIVE)) {
        os += (first ? "" : " | ");
        os += "ACTIVE";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::KeepaliveStatusCode::ACTIVE;
    }
    if ((o & ::android::hardware::radio::V1_1::KeepaliveStatusCode::INACTIVE) == static_cast<int32_t>(::android::hardware::radio::V1_1::KeepaliveStatusCode::INACTIVE)) {
        os += (first ? "" : " | ");
        os += "INACTIVE";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::KeepaliveStatusCode::INACTIVE;
    }
    if ((o & ::android::hardware::radio::V1_1::KeepaliveStatusCode::PENDING) == static_cast<int32_t>(::android::hardware::radio::V1_1::KeepaliveStatusCode::PENDING)) {
        os += (first ? "" : " | ");
        os += "PENDING";
        first = false;
        flipped |= ::android::hardware::radio::V1_1::KeepaliveStatusCode::PENDING;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::V1_1::KeepaliveStatusCode o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::V1_1::KeepaliveStatusCode::ACTIVE) {
        return "ACTIVE";
    }
    if (o == ::android::hardware::radio::V1_1::KeepaliveStatusCode::INACTIVE) {
        return "INACTIVE";
    }
    if (o == ::android::hardware::radio::V1_1::KeepaliveStatusCode::PENDING) {
        return "PENDING";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

static inline std::string toString(const ::android::hardware::radio::V1_1::RadioAccessSpecifier& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".radioAccessNetwork = ";
    os += ::android::hardware::radio::V1_1::toString(o.radioAccessNetwork);
    os += ", .geranBands = ";
    os += ::android::hardware::toString(o.geranBands);
    os += ", .utranBands = ";
    os += ::android::hardware::toString(o.utranBands);
    os += ", .eutranBands = ";
    os += ::android::hardware::toString(o.eutranBands);
    os += ", .channels = ";
    os += ::android::hardware::toString(o.channels);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_1::RadioAccessSpecifier& lhs, const ::android::hardware::radio::V1_1::RadioAccessSpecifier& rhs) {
    if (lhs.radioAccessNetwork != rhs.radioAccessNetwork) {
        return false;
    }
    if (lhs.geranBands != rhs.geranBands) {
        return false;
    }
    if (lhs.utranBands != rhs.utranBands) {
        return false;
    }
    if (lhs.eutranBands != rhs.eutranBands) {
        return false;
    }
    if (lhs.channels != rhs.channels) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_1::RadioAccessSpecifier& lhs,const ::android::hardware::radio::V1_1::RadioAccessSpecifier& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_1::NetworkScanRequest& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".type = ";
    os += ::android::hardware::radio::V1_1::toString(o.type);
    os += ", .interval = ";
    os += ::android::hardware::toString(o.interval);
    os += ", .specifiers = ";
    os += ::android::hardware::toString(o.specifiers);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_1::NetworkScanRequest& lhs, const ::android::hardware::radio::V1_1::NetworkScanRequest& rhs) {
    if (lhs.type != rhs.type) {
        return false;
    }
    if (lhs.interval != rhs.interval) {
        return false;
    }
    if (lhs.specifiers != rhs.specifiers) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_1::NetworkScanRequest& lhs,const ::android::hardware::radio::V1_1::NetworkScanRequest& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_1::NetworkScanResult& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".status = ";
    os += ::android::hardware::radio::V1_1::toString(o.status);
    os += ", .error = ";
    os += ::android::hardware::radio::V1_0::toString(o.error);
    os += ", .networkInfos = ";
    os += ::android::hardware::toString(o.networkInfos);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_1::NetworkScanResult& lhs, const ::android::hardware::radio::V1_1::NetworkScanResult& rhs) {
    if (lhs.status != rhs.status) {
        return false;
    }
    if (lhs.error != rhs.error) {
        return false;
    }
    if (lhs.networkInfos != rhs.networkInfos) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_1::NetworkScanResult& lhs,const ::android::hardware::radio::V1_1::NetworkScanResult& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_1::KeepaliveRequest& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".type = ";
    os += ::android::hardware::radio::V1_1::toString(o.type);
    os += ", .sourceAddress = ";
    os += ::android::hardware::toString(o.sourceAddress);
    os += ", .sourcePort = ";
    os += ::android::hardware::toString(o.sourcePort);
    os += ", .destinationAddress = ";
    os += ::android::hardware::toString(o.destinationAddress);
    os += ", .destinationPort = ";
    os += ::android::hardware::toString(o.destinationPort);
    os += ", .maxKeepaliveIntervalMillis = ";
    os += ::android::hardware::toString(o.maxKeepaliveIntervalMillis);
    os += ", .cid = ";
    os += ::android::hardware::toString(o.cid);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_1::KeepaliveRequest& lhs, const ::android::hardware::radio::V1_1::KeepaliveRequest& rhs) {
    if (lhs.type != rhs.type) {
        return false;
    }
    if (lhs.sourceAddress != rhs.sourceAddress) {
        return false;
    }
    if (lhs.sourcePort != rhs.sourcePort) {
        return false;
    }
    if (lhs.destinationAddress != rhs.destinationAddress) {
        return false;
    }
    if (lhs.destinationPort != rhs.destinationPort) {
        return false;
    }
    if (lhs.maxKeepaliveIntervalMillis != rhs.maxKeepaliveIntervalMillis) {
        return false;
    }
    if (lhs.cid != rhs.cid) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_1::KeepaliveRequest& lhs,const ::android::hardware::radio::V1_1::KeepaliveRequest& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_1::KeepaliveStatus& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".sessionHandle = ";
    os += ::android::hardware::toString(o.sessionHandle);
    os += ", .code = ";
    os += ::android::hardware::radio::V1_1::toString(o.code);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_1::KeepaliveStatus& lhs, const ::android::hardware::radio::V1_1::KeepaliveStatus& rhs) {
    if (lhs.sessionHandle != rhs.sessionHandle) {
        return false;
    }
    if (lhs.code != rhs.code) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_1::KeepaliveStatus& lhs,const ::android::hardware::radio::V1_1::KeepaliveStatus& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::android::hardware::radio::V1_1::ImsiEncryptionInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".mcc = ";
    os += ::android::hardware::toString(o.mcc);
    os += ", .mnc = ";
    os += ::android::hardware::toString(o.mnc);
    os += ", .carrierKey = ";
    os += ::android::hardware::toString(o.carrierKey);
    os += ", .keyIdentifier = ";
    os += ::android::hardware::toString(o.keyIdentifier);
    os += ", .expirationTime = ";
    os += ::android::hardware::toString(o.expirationTime);
    os += "}"; return os;
}

static inline bool operator==(const ::android::hardware::radio::V1_1::ImsiEncryptionInfo& lhs, const ::android::hardware::radio::V1_1::ImsiEncryptionInfo& rhs) {
    if (lhs.mcc != rhs.mcc) {
        return false;
    }
    if (lhs.mnc != rhs.mnc) {
        return false;
    }
    if (lhs.carrierKey != rhs.carrierKey) {
        return false;
    }
    if (lhs.keyIdentifier != rhs.keyIdentifier) {
        return false;
    }
    if (lhs.expirationTime != rhs.expirationTime) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::android::hardware::radio::V1_1::ImsiEncryptionInfo& lhs,const ::android::hardware::radio::V1_1::ImsiEncryptionInfo& rhs){
    return !(lhs == rhs);
}


}  // namespace V1_1
}  // namespace radio
}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_1::CardPowerState>
{
    const ::android::hardware::radio::V1_1::CardPowerState* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_1::CardPowerState* end() { return begin() + 3; }
    private:
    static const ::android::hardware::radio::V1_1::CardPowerState* static_begin() {
        static const ::android::hardware::radio::V1_1::CardPowerState kVals[3] {
            ::android::hardware::radio::V1_1::CardPowerState::POWER_DOWN,
            ::android::hardware::radio::V1_1::CardPowerState::POWER_UP,
            ::android::hardware::radio::V1_1::CardPowerState::POWER_UP_PASS_THROUGH,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_1::RadioAccessNetworks>
{
    const ::android::hardware::radio::V1_1::RadioAccessNetworks* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_1::RadioAccessNetworks* end() { return begin() + 3; }
    private:
    static const ::android::hardware::radio::V1_1::RadioAccessNetworks* static_begin() {
        static const ::android::hardware::radio::V1_1::RadioAccessNetworks kVals[3] {
            ::android::hardware::radio::V1_1::RadioAccessNetworks::GERAN,
            ::android::hardware::radio::V1_1::RadioAccessNetworks::UTRAN,
            ::android::hardware::radio::V1_1::RadioAccessNetworks::EUTRAN,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_1::GeranBands>
{
    const ::android::hardware::radio::V1_1::GeranBands* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_1::GeranBands* end() { return begin() + 14; }
    private:
    static const ::android::hardware::radio::V1_1::GeranBands* static_begin() {
        static const ::android::hardware::radio::V1_1::GeranBands kVals[14] {
            ::android::hardware::radio::V1_1::GeranBands::BAND_T380,
            ::android::hardware::radio::V1_1::GeranBands::BAND_T410,
            ::android::hardware::radio::V1_1::GeranBands::BAND_450,
            ::android::hardware::radio::V1_1::GeranBands::BAND_480,
            ::android::hardware::radio::V1_1::GeranBands::BAND_710,
            ::android::hardware::radio::V1_1::GeranBands::BAND_750,
            ::android::hardware::radio::V1_1::GeranBands::BAND_T810,
            ::android::hardware::radio::V1_1::GeranBands::BAND_850,
            ::android::hardware::radio::V1_1::GeranBands::BAND_P900,
            ::android::hardware::radio::V1_1::GeranBands::BAND_E900,
            ::android::hardware::radio::V1_1::GeranBands::BAND_R900,
            ::android::hardware::radio::V1_1::GeranBands::BAND_DCS1800,
            ::android::hardware::radio::V1_1::GeranBands::BAND_PCS1900,
            ::android::hardware::radio::V1_1::GeranBands::BAND_ER900,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_1::UtranBands>
{
    const ::android::hardware::radio::V1_1::UtranBands* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_1::UtranBands* end() { return begin() + 20; }
    private:
    static const ::android::hardware::radio::V1_1::UtranBands* static_begin() {
        static const ::android::hardware::radio::V1_1::UtranBands kVals[20] {
            ::android::hardware::radio::V1_1::UtranBands::BAND_1,
            ::android::hardware::radio::V1_1::UtranBands::BAND_2,
            ::android::hardware::radio::V1_1::UtranBands::BAND_3,
            ::android::hardware::radio::V1_1::UtranBands::BAND_4,
            ::android::hardware::radio::V1_1::UtranBands::BAND_5,
            ::android::hardware::radio::V1_1::UtranBands::BAND_6,
            ::android::hardware::radio::V1_1::UtranBands::BAND_7,
            ::android::hardware::radio::V1_1::UtranBands::BAND_8,
            ::android::hardware::radio::V1_1::UtranBands::BAND_9,
            ::android::hardware::radio::V1_1::UtranBands::BAND_10,
            ::android::hardware::radio::V1_1::UtranBands::BAND_11,
            ::android::hardware::radio::V1_1::UtranBands::BAND_12,
            ::android::hardware::radio::V1_1::UtranBands::BAND_13,
            ::android::hardware::radio::V1_1::UtranBands::BAND_14,
            ::android::hardware::radio::V1_1::UtranBands::BAND_19,
            ::android::hardware::radio::V1_1::UtranBands::BAND_20,
            ::android::hardware::radio::V1_1::UtranBands::BAND_21,
            ::android::hardware::radio::V1_1::UtranBands::BAND_22,
            ::android::hardware::radio::V1_1::UtranBands::BAND_25,
            ::android::hardware::radio::V1_1::UtranBands::BAND_26,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_1::EutranBands>
{
    const ::android::hardware::radio::V1_1::EutranBands* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_1::EutranBands* end() { return begin() + 48; }
    private:
    static const ::android::hardware::radio::V1_1::EutranBands* static_begin() {
        static const ::android::hardware::radio::V1_1::EutranBands kVals[48] {
            ::android::hardware::radio::V1_1::EutranBands::BAND_1,
            ::android::hardware::radio::V1_1::EutranBands::BAND_2,
            ::android::hardware::radio::V1_1::EutranBands::BAND_3,
            ::android::hardware::radio::V1_1::EutranBands::BAND_4,
            ::android::hardware::radio::V1_1::EutranBands::BAND_5,
            ::android::hardware::radio::V1_1::EutranBands::BAND_6,
            ::android::hardware::radio::V1_1::EutranBands::BAND_7,
            ::android::hardware::radio::V1_1::EutranBands::BAND_8,
            ::android::hardware::radio::V1_1::EutranBands::BAND_9,
            ::android::hardware::radio::V1_1::EutranBands::BAND_10,
            ::android::hardware::radio::V1_1::EutranBands::BAND_11,
            ::android::hardware::radio::V1_1::EutranBands::BAND_12,
            ::android::hardware::radio::V1_1::EutranBands::BAND_13,
            ::android::hardware::radio::V1_1::EutranBands::BAND_14,
            ::android::hardware::radio::V1_1::EutranBands::BAND_17,
            ::android::hardware::radio::V1_1::EutranBands::BAND_18,
            ::android::hardware::radio::V1_1::EutranBands::BAND_19,
            ::android::hardware::radio::V1_1::EutranBands::BAND_20,
            ::android::hardware::radio::V1_1::EutranBands::BAND_21,
            ::android::hardware::radio::V1_1::EutranBands::BAND_22,
            ::android::hardware::radio::V1_1::EutranBands::BAND_23,
            ::android::hardware::radio::V1_1::EutranBands::BAND_24,
            ::android::hardware::radio::V1_1::EutranBands::BAND_25,
            ::android::hardware::radio::V1_1::EutranBands::BAND_26,
            ::android::hardware::radio::V1_1::EutranBands::BAND_27,
            ::android::hardware::radio::V1_1::EutranBands::BAND_28,
            ::android::hardware::radio::V1_1::EutranBands::BAND_30,
            ::android::hardware::radio::V1_1::EutranBands::BAND_31,
            ::android::hardware::radio::V1_1::EutranBands::BAND_33,
            ::android::hardware::radio::V1_1::EutranBands::BAND_34,
            ::android::hardware::radio::V1_1::EutranBands::BAND_35,
            ::android::hardware::radio::V1_1::EutranBands::BAND_36,
            ::android::hardware::radio::V1_1::EutranBands::BAND_37,
            ::android::hardware::radio::V1_1::EutranBands::BAND_38,
            ::android::hardware::radio::V1_1::EutranBands::BAND_39,
            ::android::hardware::radio::V1_1::EutranBands::BAND_40,
            ::android::hardware::radio::V1_1::EutranBands::BAND_41,
            ::android::hardware::radio::V1_1::EutranBands::BAND_42,
            ::android::hardware::radio::V1_1::EutranBands::BAND_43,
            ::android::hardware::radio::V1_1::EutranBands::BAND_44,
            ::android::hardware::radio::V1_1::EutranBands::BAND_45,
            ::android::hardware::radio::V1_1::EutranBands::BAND_46,
            ::android::hardware::radio::V1_1::EutranBands::BAND_47,
            ::android::hardware::radio::V1_1::EutranBands::BAND_48,
            ::android::hardware::radio::V1_1::EutranBands::BAND_65,
            ::android::hardware::radio::V1_1::EutranBands::BAND_66,
            ::android::hardware::radio::V1_1::EutranBands::BAND_68,
            ::android::hardware::radio::V1_1::EutranBands::BAND_70,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_1::ScanType>
{
    const ::android::hardware::radio::V1_1::ScanType* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_1::ScanType* end() { return begin() + 2; }
    private:
    static const ::android::hardware::radio::V1_1::ScanType* static_begin() {
        static const ::android::hardware::radio::V1_1::ScanType kVals[2] {
            ::android::hardware::radio::V1_1::ScanType::ONE_SHOT,
            ::android::hardware::radio::V1_1::ScanType::PERIODIC,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_1::ScanStatus>
{
    const ::android::hardware::radio::V1_1::ScanStatus* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_1::ScanStatus* end() { return begin() + 2; }
    private:
    static const ::android::hardware::radio::V1_1::ScanStatus* static_begin() {
        static const ::android::hardware::radio::V1_1::ScanStatus kVals[2] {
            ::android::hardware::radio::V1_1::ScanStatus::PARTIAL,
            ::android::hardware::radio::V1_1::ScanStatus::COMPLETE,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_1::KeepaliveType>
{
    const ::android::hardware::radio::V1_1::KeepaliveType* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_1::KeepaliveType* end() { return begin() + 2; }
    private:
    static const ::android::hardware::radio::V1_1::KeepaliveType* static_begin() {
        static const ::android::hardware::radio::V1_1::KeepaliveType kVals[2] {
            ::android::hardware::radio::V1_1::KeepaliveType::NATT_IPV4,
            ::android::hardware::radio::V1_1::KeepaliveType::NATT_IPV6,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::V1_1::KeepaliveStatusCode>
{
    const ::android::hardware::radio::V1_1::KeepaliveStatusCode* begin() { return static_begin(); }
    const ::android::hardware::radio::V1_1::KeepaliveStatusCode* end() { return begin() + 3; }
    private:
    static const ::android::hardware::radio::V1_1::KeepaliveStatusCode* static_begin() {
        static const ::android::hardware::radio::V1_1::KeepaliveStatusCode kVals[3] {
            ::android::hardware::radio::V1_1::KeepaliveStatusCode::ACTIVE,
            ::android::hardware::radio::V1_1::KeepaliveStatusCode::INACTIVE,
            ::android::hardware::radio::V1_1::KeepaliveStatusCode::PENDING,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android

#endif  // HIDL_GENERATED_ANDROID_HARDWARE_RADIO_V1_1_TYPES_H
