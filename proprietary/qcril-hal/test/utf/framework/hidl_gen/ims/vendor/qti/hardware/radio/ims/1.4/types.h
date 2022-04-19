/*
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_4_TYPES_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_4_TYPES_H

#include <vendor/qti/hardware/radio/ims/1.0/types.h>
#include <vendor/qti/hardware/radio/ims/1.3/types.h>

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_4 {

// Forward declaration for forward reference support:
enum class MultiIdentityRegistrationStatus : int32_t;
enum class MultiIdentityLineType : int32_t;
struct MultiIdentityLineInfoHal;
struct DialRequest;
struct CallInfo;

enum class MultiIdentityRegistrationStatus : int32_t {
    STATUS_UNKNOWN = 0,
    STATUS_DISABLE = 1, // (::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus.STATUS_UNKNOWN implicitly + 1)
    STATUS_ENABLE = 2, // (::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus.STATUS_DISABLE implicitly + 1)
};

enum class MultiIdentityLineType : int32_t {
    LINE_TYPE_UNKNOWN = 0,
    LINE_TYPE_PRIMARY = 1, // (::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType.LINE_TYPE_UNKNOWN implicitly + 1)
    LINE_TYPE_SECONDARY = 2, // (::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType.LINE_TYPE_PRIMARY implicitly + 1)
};

struct MultiIdentityLineInfoHal final {
    ::android::hardware::hidl_string msisdn __attribute__ ((aligned(8)));
    ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus registrationStatus __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType lineType __attribute__ ((aligned(4)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineInfoHal, msisdn) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineInfoHal, registrationStatus) == 16, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineInfoHal, lineType) == 20, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineInfoHal) == 24, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineInfoHal) == 8, "wrong alignment");

struct DialRequest final {
    ::android::hardware::hidl_string address __attribute__ ((aligned(8)));
    uint32_t clirMode __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation presentation __attribute__ ((aligned(4)));
    bool hasCallDetails __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_0::CallDetails callDetails __attribute__ ((aligned(8)));
    bool hasIsConferenceUri __attribute__ ((aligned(1)));
    bool isConferenceUri __attribute__ ((aligned(1)));
    bool hasIsCallPull __attribute__ ((aligned(1)));
    bool isCallPull __attribute__ ((aligned(1)));
    bool hasIsEncrypted __attribute__ ((aligned(1)));
    bool isEncrypted __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineInfoHal multiLineInfo __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::DialRequest, address) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::DialRequest, clirMode) == 16, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::DialRequest, presentation) == 20, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::DialRequest, hasCallDetails) == 24, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::DialRequest, callDetails) == 32, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::DialRequest, hasIsConferenceUri) == 128, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::DialRequest, isConferenceUri) == 129, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::DialRequest, hasIsCallPull) == 130, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::DialRequest, isCallPull) == 131, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::DialRequest, hasIsEncrypted) == 132, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::DialRequest, isEncrypted) == 133, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::DialRequest, multiLineInfo) == 136, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_4::DialRequest) == 160, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_4::DialRequest) == 8, "wrong alignment");

struct CallInfo final {
    ::vendor::qti::hardware::radio::ims::V1_0::CallState state __attribute__ ((aligned(4)));
    uint32_t index __attribute__ ((aligned(4)));
    uint32_t toa __attribute__ ((aligned(4)));
    bool hasIsMpty __attribute__ ((aligned(1)));
    bool isMpty __attribute__ ((aligned(1)));
    bool hasIsMT __attribute__ ((aligned(1)));
    bool isMT __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineInfoHal mtMultiLineInfo __attribute__ ((aligned(8)));
    uint32_t als __attribute__ ((aligned(4)));
    bool hasIsVoice __attribute__ ((aligned(1)));
    bool isVoice __attribute__ ((aligned(1)));
    bool hasIsVoicePrivacy __attribute__ ((aligned(1)));
    bool isVoicePrivacy __attribute__ ((aligned(1)));
    ::android::hardware::hidl_string number __attribute__ ((aligned(8)));
    uint32_t numberPresentation __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string name __attribute__ ((aligned(8)));
    uint32_t namePresentation __attribute__ ((aligned(4)));
    bool hasCallDetails __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_0::CallDetails callDetails __attribute__ ((aligned(8)));
    bool hasFailCause __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_3::CallFailCauseResponse failCause __attribute__ ((aligned(8)));
    bool hasIsEncrypted __attribute__ ((aligned(1)));
    bool isEncrypted __attribute__ ((aligned(1)));
    bool hasIsCalledPartyRinging __attribute__ ((aligned(1)));
    bool isCalledPartyRinging __attribute__ ((aligned(1)));
    ::android::hardware::hidl_string historyInfo __attribute__ ((aligned(8)));
    bool hasIsVideoConfSupported __attribute__ ((aligned(1)));
    bool isVideoConfSupported __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_3::VerstatInfo verstatInfo __attribute__ ((aligned(4)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo, state) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo, index) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo, toa) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo, hasIsMpty) == 12, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo, isMpty) == 13, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo, hasIsMT) == 14, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo, isMT) == 15, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo, mtMultiLineInfo) == 16, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo, als) == 40, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo, hasIsVoice) == 44, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo, isVoice) == 45, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo, hasIsVoicePrivacy) == 46, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo, isVoicePrivacy) == 47, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo, number) == 48, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo, numberPresentation) == 64, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo, name) == 72, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo, namePresentation) == 88, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo, hasCallDetails) == 92, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo, callDetails) == 96, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo, hasFailCause) == 192, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo, failCause) == 200, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo, hasIsEncrypted) == 272, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo, isEncrypted) == 273, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo, hasIsCalledPartyRinging) == 274, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo, isCalledPartyRinging) == 275, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo, historyInfo) == 280, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo, hasIsVideoConfSupported) == 296, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo, isVideoConfSupported) == 297, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo, verstatInfo) == 300, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo) == 312, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_4::CallInfo) == 8, "wrong alignment");

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus lhs, const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus lhs, const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus::STATUS_UNKNOWN) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus::STATUS_UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "STATUS_UNKNOWN";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus::STATUS_UNKNOWN;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus::STATUS_DISABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus::STATUS_DISABLE)) {
        os += (first ? "" : " | ");
        os += "STATUS_DISABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus::STATUS_DISABLE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus::STATUS_ENABLE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus::STATUS_ENABLE)) {
        os += (first ? "" : " | ");
        os += "STATUS_ENABLE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus::STATUS_ENABLE;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus::STATUS_UNKNOWN) {
        return "STATUS_UNKNOWN";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus::STATUS_DISABLE) {
        return "STATUS_DISABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus::STATUS_ENABLE) {
        return "STATUS_ENABLE";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType lhs, const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType lhs, const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType::LINE_TYPE_UNKNOWN) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType::LINE_TYPE_UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "LINE_TYPE_UNKNOWN";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType::LINE_TYPE_UNKNOWN;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType::LINE_TYPE_PRIMARY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType::LINE_TYPE_PRIMARY)) {
        os += (first ? "" : " | ");
        os += "LINE_TYPE_PRIMARY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType::LINE_TYPE_PRIMARY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType::LINE_TYPE_SECONDARY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType::LINE_TYPE_SECONDARY)) {
        os += (first ? "" : " | ");
        os += "LINE_TYPE_SECONDARY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType::LINE_TYPE_SECONDARY;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType::LINE_TYPE_UNKNOWN) {
        return "LINE_TYPE_UNKNOWN";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType::LINE_TYPE_PRIMARY) {
        return "LINE_TYPE_PRIMARY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType::LINE_TYPE_SECONDARY) {
        return "LINE_TYPE_SECONDARY";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineInfoHal& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".msisdn = ";
    os += ::android::hardware::toString(o.msisdn);
    os += ", .registrationStatus = ";
    os += ::vendor::qti::hardware::radio::ims::V1_4::toString(o.registrationStatus);
    os += ", .lineType = ";
    os += ::vendor::qti::hardware::radio::ims::V1_4::toString(o.lineType);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineInfoHal& lhs, const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineInfoHal& rhs) {
    if (lhs.msisdn != rhs.msisdn) {
        return false;
    }
    if (lhs.registrationStatus != rhs.registrationStatus) {
        return false;
    }
    if (lhs.lineType != rhs.lineType) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineInfoHal& lhs,const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineInfoHal& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_4::DialRequest& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".address = ";
    os += ::android::hardware::toString(o.address);
    os += ", .clirMode = ";
    os += ::android::hardware::toString(o.clirMode);
    os += ", .presentation = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.presentation);
    os += ", .hasCallDetails = ";
    os += ::android::hardware::toString(o.hasCallDetails);
    os += ", .callDetails = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.callDetails);
    os += ", .hasIsConferenceUri = ";
    os += ::android::hardware::toString(o.hasIsConferenceUri);
    os += ", .isConferenceUri = ";
    os += ::android::hardware::toString(o.isConferenceUri);
    os += ", .hasIsCallPull = ";
    os += ::android::hardware::toString(o.hasIsCallPull);
    os += ", .isCallPull = ";
    os += ::android::hardware::toString(o.isCallPull);
    os += ", .hasIsEncrypted = ";
    os += ::android::hardware::toString(o.hasIsEncrypted);
    os += ", .isEncrypted = ";
    os += ::android::hardware::toString(o.isEncrypted);
    os += ", .multiLineInfo = ";
    os += ::vendor::qti::hardware::radio::ims::V1_4::toString(o.multiLineInfo);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_4::DialRequest& lhs, const ::vendor::qti::hardware::radio::ims::V1_4::DialRequest& rhs) {
    if (lhs.address != rhs.address) {
        return false;
    }
    if (lhs.clirMode != rhs.clirMode) {
        return false;
    }
    if (lhs.presentation != rhs.presentation) {
        return false;
    }
    if (lhs.hasCallDetails != rhs.hasCallDetails) {
        return false;
    }
    if (lhs.callDetails != rhs.callDetails) {
        return false;
    }
    if (lhs.hasIsConferenceUri != rhs.hasIsConferenceUri) {
        return false;
    }
    if (lhs.isConferenceUri != rhs.isConferenceUri) {
        return false;
    }
    if (lhs.hasIsCallPull != rhs.hasIsCallPull) {
        return false;
    }
    if (lhs.isCallPull != rhs.isCallPull) {
        return false;
    }
    if (lhs.hasIsEncrypted != rhs.hasIsEncrypted) {
        return false;
    }
    if (lhs.isEncrypted != rhs.isEncrypted) {
        return false;
    }
    if (lhs.multiLineInfo != rhs.multiLineInfo) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_4::DialRequest& lhs,const ::vendor::qti::hardware::radio::ims::V1_4::DialRequest& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_4::CallInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".state = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.state);
    os += ", .index = ";
    os += ::android::hardware::toString(o.index);
    os += ", .toa = ";
    os += ::android::hardware::toString(o.toa);
    os += ", .hasIsMpty = ";
    os += ::android::hardware::toString(o.hasIsMpty);
    os += ", .isMpty = ";
    os += ::android::hardware::toString(o.isMpty);
    os += ", .hasIsMT = ";
    os += ::android::hardware::toString(o.hasIsMT);
    os += ", .isMT = ";
    os += ::android::hardware::toString(o.isMT);
    os += ", .mtMultiLineInfo = ";
    os += ::vendor::qti::hardware::radio::ims::V1_4::toString(o.mtMultiLineInfo);
    os += ", .als = ";
    os += ::android::hardware::toString(o.als);
    os += ", .hasIsVoice = ";
    os += ::android::hardware::toString(o.hasIsVoice);
    os += ", .isVoice = ";
    os += ::android::hardware::toString(o.isVoice);
    os += ", .hasIsVoicePrivacy = ";
    os += ::android::hardware::toString(o.hasIsVoicePrivacy);
    os += ", .isVoicePrivacy = ";
    os += ::android::hardware::toString(o.isVoicePrivacy);
    os += ", .number = ";
    os += ::android::hardware::toString(o.number);
    os += ", .numberPresentation = ";
    os += ::android::hardware::toString(o.numberPresentation);
    os += ", .name = ";
    os += ::android::hardware::toString(o.name);
    os += ", .namePresentation = ";
    os += ::android::hardware::toString(o.namePresentation);
    os += ", .hasCallDetails = ";
    os += ::android::hardware::toString(o.hasCallDetails);
    os += ", .callDetails = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.callDetails);
    os += ", .hasFailCause = ";
    os += ::android::hardware::toString(o.hasFailCause);
    os += ", .failCause = ";
    os += ::vendor::qti::hardware::radio::ims::V1_3::toString(o.failCause);
    os += ", .hasIsEncrypted = ";
    os += ::android::hardware::toString(o.hasIsEncrypted);
    os += ", .isEncrypted = ";
    os += ::android::hardware::toString(o.isEncrypted);
    os += ", .hasIsCalledPartyRinging = ";
    os += ::android::hardware::toString(o.hasIsCalledPartyRinging);
    os += ", .isCalledPartyRinging = ";
    os += ::android::hardware::toString(o.isCalledPartyRinging);
    os += ", .historyInfo = ";
    os += ::android::hardware::toString(o.historyInfo);
    os += ", .hasIsVideoConfSupported = ";
    os += ::android::hardware::toString(o.hasIsVideoConfSupported);
    os += ", .isVideoConfSupported = ";
    os += ::android::hardware::toString(o.isVideoConfSupported);
    os += ", .verstatInfo = ";
    os += ::vendor::qti::hardware::radio::ims::V1_3::toString(o.verstatInfo);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_4::CallInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_4::CallInfo& rhs) {
    if (lhs.state != rhs.state) {
        return false;
    }
    if (lhs.index != rhs.index) {
        return false;
    }
    if (lhs.toa != rhs.toa) {
        return false;
    }
    if (lhs.hasIsMpty != rhs.hasIsMpty) {
        return false;
    }
    if (lhs.isMpty != rhs.isMpty) {
        return false;
    }
    if (lhs.hasIsMT != rhs.hasIsMT) {
        return false;
    }
    if (lhs.isMT != rhs.isMT) {
        return false;
    }
    if (lhs.mtMultiLineInfo != rhs.mtMultiLineInfo) {
        return false;
    }
    if (lhs.als != rhs.als) {
        return false;
    }
    if (lhs.hasIsVoice != rhs.hasIsVoice) {
        return false;
    }
    if (lhs.isVoice != rhs.isVoice) {
        return false;
    }
    if (lhs.hasIsVoicePrivacy != rhs.hasIsVoicePrivacy) {
        return false;
    }
    if (lhs.isVoicePrivacy != rhs.isVoicePrivacy) {
        return false;
    }
    if (lhs.number != rhs.number) {
        return false;
    }
    if (lhs.numberPresentation != rhs.numberPresentation) {
        return false;
    }
    if (lhs.name != rhs.name) {
        return false;
    }
    if (lhs.namePresentation != rhs.namePresentation) {
        return false;
    }
    if (lhs.hasCallDetails != rhs.hasCallDetails) {
        return false;
    }
    if (lhs.callDetails != rhs.callDetails) {
        return false;
    }
    if (lhs.hasFailCause != rhs.hasFailCause) {
        return false;
    }
    if (lhs.failCause != rhs.failCause) {
        return false;
    }
    if (lhs.hasIsEncrypted != rhs.hasIsEncrypted) {
        return false;
    }
    if (lhs.isEncrypted != rhs.isEncrypted) {
        return false;
    }
    if (lhs.hasIsCalledPartyRinging != rhs.hasIsCalledPartyRinging) {
        return false;
    }
    if (lhs.isCalledPartyRinging != rhs.isCalledPartyRinging) {
        return false;
    }
    if (lhs.historyInfo != rhs.historyInfo) {
        return false;
    }
    if (lhs.hasIsVideoConfSupported != rhs.hasIsVideoConfSupported) {
        return false;
    }
    if (lhs.isVideoConfSupported != rhs.isVideoConfSupported) {
        return false;
    }
    if (lhs.verstatInfo != rhs.verstatInfo) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_4::CallInfo& lhs,const ::vendor::qti::hardware::radio::ims::V1_4::CallInfo& rhs){
    return !(lhs == rhs);
}


}  // namespace V1_4
}  // namespace ims
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus>
{
    const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus* end() { return begin() + 3; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus kVals[3] {
            ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus::STATUS_UNKNOWN,
            ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus::STATUS_DISABLE,
            ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityRegistrationStatus::STATUS_ENABLE,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType>
{
    const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType* begin() { return static_begin(); }
    const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType* end() { return begin() + 3; }
    private:
    static const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType* static_begin() {
        static const ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType kVals[3] {
            ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType::LINE_TYPE_UNKNOWN,
            ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType::LINE_TYPE_PRIMARY,
            ::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineType::LINE_TYPE_SECONDARY,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android

#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_4_TYPES_H
