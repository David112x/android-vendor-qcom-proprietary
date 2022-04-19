/*===========================================================================
   Copyright (c) 2019 Qualcomm Technologies, Inc.
   All Rights Reserved.
   Confidential and Proprietary - Qualcomm Technologies, Inc.
===========================================================================*/

#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_6_TYPES_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_6_TYPES_H

#include <vendor/qti/hardware/radio/ims/1.0/types.h>
#include <vendor/qti/hardware/radio/ims/1.3/types.h>
#include <vendor/qti/hardware/radio/ims/1.4/types.h>
#include <vendor/qti/hardware/radio/ims/1.5/types.h>

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_6 {

// Forward declaration for forward reference support:
enum class RadioTechType : int32_t;
struct RegistrationInfo;
struct StatusForAccessTech;
struct ServiceStatusInfo;
struct CallDetails;
struct CallModifyInfo;
struct CallInfo;
struct HandoverInfo;

enum class RadioTechType : int32_t {
    RADIO_TECH_ANY = 0,
    RADIO_TECH_UNKNOWN = 1 /* (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_ANY implicitly + 1) */,
    RADIO_TECH_GPRS = 2 /* (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_UNKNOWN implicitly + 1) */,
    RADIO_TECH_EDGE = 3 /* (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_GPRS implicitly + 1) */,
    RADIO_TECH_UMTS = 4 /* (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_EDGE implicitly + 1) */,
    RADIO_TECH_IS95A = 5 /* (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_UMTS implicitly + 1) */,
    RADIO_TECH_IS95B = 6 /* (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_IS95A implicitly + 1) */,
    RADIO_TECH_1xRTT = 7 /* (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_IS95B implicitly + 1) */,
    RADIO_TECH_EVDO_0 = 8 /* (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_1xRTT implicitly + 1) */,
    RADIO_TECH_EVDO_A = 9 /* (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_EVDO_0 implicitly + 1) */,
    RADIO_TECH_HSDPA = 10 /* (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_EVDO_A implicitly + 1) */,
    RADIO_TECH_HSUPA = 11 /* (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_HSDPA implicitly + 1) */,
    RADIO_TECH_HSPA = 12 /* (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_HSUPA implicitly + 1) */,
    RADIO_TECH_EVDO_B = 13 /* (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_HSPA implicitly + 1) */,
    RADIO_TECH_EHRPD = 14 /* (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_EVDO_B implicitly + 1) */,
    RADIO_TECH_LTE = 15 /* (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_EHRPD implicitly + 1) */,
    RADIO_TECH_HSPAP = 16 /* (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_LTE implicitly + 1) */,
    RADIO_TECH_GSM = 17 /* (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_HSPAP implicitly + 1) */,
    RADIO_TECH_TD_SCDMA = 18 /* (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_GSM implicitly + 1) */,
    RADIO_TECH_WIFI = 19 /* (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_TD_SCDMA implicitly + 1) */,
    RADIO_TECH_IWLAN = 20 /* (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_WIFI implicitly + 1) */,
    RADIO_TECH_INVALID = 21 /* (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_IWLAN implicitly + 1) */,
    RADIO_TECH_NR5G = 22 /* (::vendor::qti::hardware::radio::ims::V1_0::RadioTechType.RADIO_TECH_INVALID implicitly + 1) */,
};

struct RegistrationInfo final {
    ::vendor::qti::hardware::radio::ims::V1_0::RegState state __attribute__ ((aligned(4)));
    uint32_t errorCode __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string errorMessage __attribute__ ((aligned(8)));
    ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType radioTech __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string pAssociatedUris __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::RegistrationInfo, state) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::RegistrationInfo, errorCode) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::RegistrationInfo, errorMessage) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::RegistrationInfo, radioTech) == 24, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::RegistrationInfo, pAssociatedUris) == 32, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_6::RegistrationInfo) == 48, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_6::RegistrationInfo) == 8, "wrong alignment");

struct StatusForAccessTech final {
    ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType networkMode __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_0::StatusType status __attribute__ ((aligned(4)));
    uint32_t restrictCause __attribute__ ((aligned(4)));
    bool hasRegistration __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_6::RegistrationInfo registration __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::StatusForAccessTech, networkMode) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::StatusForAccessTech, status) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::StatusForAccessTech, restrictCause) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::StatusForAccessTech, hasRegistration) == 12, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::StatusForAccessTech, registration) == 16, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_6::StatusForAccessTech) == 64, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_6::StatusForAccessTech) == 8, "wrong alignment");

struct ServiceStatusInfo final {
    bool hasIsValid __attribute__ ((aligned(1)));
    bool isValid __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_0::ServiceType type __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_0::CallType callType __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_0::StatusType status __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<uint8_t> userdata __attribute__ ((aligned(8)));
    uint32_t restrictCause __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_6::StatusForAccessTech> accTechStatus __attribute__ ((aligned(8)));
    ::vendor::qti::hardware::radio::ims::V1_0::RttMode rttMode __attribute__ ((aligned(4)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::ServiceStatusInfo, hasIsValid) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::ServiceStatusInfo, isValid) == 1, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::ServiceStatusInfo, type) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::ServiceStatusInfo, callType) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::ServiceStatusInfo, status) == 12, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::ServiceStatusInfo, userdata) == 16, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::ServiceStatusInfo, restrictCause) == 32, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::ServiceStatusInfo, accTechStatus) == 40, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::ServiceStatusInfo, rttMode) == 56, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_6::ServiceStatusInfo) == 64, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_6::ServiceStatusInfo) == 8, "wrong alignment");

struct CallDetails final {
    ::vendor::qti::hardware::radio::ims::V1_0::CallType callType __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_0::CallDomain callDomain __attribute__ ((aligned(4)));
    uint32_t extrasLength __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<::android::hardware::hidl_string> extras __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_6::ServiceStatusInfo> localAbility __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_6::ServiceStatusInfo> peerAbility __attribute__ ((aligned(8)));
    uint32_t callSubstate __attribute__ ((aligned(4)));
    uint32_t mediaId __attribute__ ((aligned(4)));
    uint32_t causeCode __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_0::RttMode rttMode __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string sipAlternateUri __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallDetails, callType) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallDetails, callDomain) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallDetails, extrasLength) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallDetails, extras) == 16, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallDetails, localAbility) == 32, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallDetails, peerAbility) == 48, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallDetails, callSubstate) == 64, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallDetails, mediaId) == 68, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallDetails, causeCode) == 72, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallDetails, rttMode) == 76, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallDetails, sipAlternateUri) == 80, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_6::CallDetails) == 96, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_6::CallDetails) == 8, "wrong alignment");

struct CallModifyInfo final {
    uint32_t callIndex __attribute__ ((aligned(4)));
    bool hasCallDetails __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_6::CallDetails callDetails __attribute__ ((aligned(8)));
    ::vendor::qti::hardware::radio::ims::V1_0::CallModifyFailCause failCause __attribute__ ((aligned(4)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallModifyInfo, callIndex) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallModifyInfo, hasCallDetails) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallModifyInfo, callDetails) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallModifyInfo, failCause) == 104, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_6::CallModifyInfo) == 112, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_6::CallModifyInfo) == 8, "wrong alignment");

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
    ::vendor::qti::hardware::radio::ims::V1_6::CallDetails callDetails __attribute__ ((aligned(8)));
    bool hasFailCause __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_5::CallFailCauseResponse failCause __attribute__ ((aligned(8)));
    bool hasIsEncrypted __attribute__ ((aligned(1)));
    bool isEncrypted __attribute__ ((aligned(1)));
    bool hasIsCalledPartyRinging __attribute__ ((aligned(1)));
    bool isCalledPartyRinging __attribute__ ((aligned(1)));
    ::android::hardware::hidl_string historyInfo __attribute__ ((aligned(8)));
    bool hasIsVideoConfSupported __attribute__ ((aligned(1)));
    bool isVideoConfSupported __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_3::VerstatInfo verstatInfo __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_5::TirMode tirMode __attribute__ ((aligned(4)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, state) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, index) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, toa) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, hasIsMpty) == 12, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, isMpty) == 13, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, hasIsMT) == 14, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, isMT) == 15, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, mtMultiLineInfo) == 16, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, als) == 40, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, hasIsVoice) == 44, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, isVoice) == 45, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, hasIsVoicePrivacy) == 46, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, isVoicePrivacy) == 47, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, number) == 48, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, numberPresentation) == 64, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, name) == 72, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, namePresentation) == 88, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, hasCallDetails) == 92, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, callDetails) == 96, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, hasFailCause) == 192, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, failCause) == 200, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, hasIsEncrypted) == 272, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, isEncrypted) == 273, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, hasIsCalledPartyRinging) == 274, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, isCalledPartyRinging) == 275, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, historyInfo) == 280, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, hasIsVideoConfSupported) == 296, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, isVideoConfSupported) == 297, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, verstatInfo) == 300, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo, tirMode) == 308, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo) == 312, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_6::CallInfo) == 8, "wrong alignment");

struct HandoverInfo final {
    ::vendor::qti::hardware::radio::ims::V1_0::HandoverType type __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType srcTech __attribute__ ((aligned(4)));
    ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType targetTech __attribute__ ((aligned(4)));
    bool hasHoExtra __attribute__ ((aligned(1)));
    ::vendor::qti::hardware::radio::ims::V1_0::Extra hoExtra __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string errorCode __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string errorMessage __attribute__ ((aligned(8)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::HandoverInfo, type) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::HandoverInfo, srcTech) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::HandoverInfo, targetTech) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::HandoverInfo, hasHoExtra) == 12, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::HandoverInfo, hoExtra) == 16, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::HandoverInfo, errorCode) == 40, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::HandoverInfo, errorMessage) == 56, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_6::HandoverInfo) == 72, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_6::HandoverInfo) == 8, "wrong alignment");

//
// type declarations for package
//
enum class ConfigItem : int32_t;
struct ConfigInfo;

template<typename>
static inline std::string toString(int32_t o);
static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_6::RadioTechType o);

constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType lhs, const ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}
constexpr int32_t operator|(const ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType lhs, const ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const int32_t lhs, const ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}
constexpr int32_t operator&(const ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}
constexpr int32_t &operator|=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType e) {
    v |= static_cast<int32_t>(e);
    return v;
}
constexpr int32_t &operator&=(int32_t& v, const ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_6::RegistrationInfo& o);
static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_6::RegistrationInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_6::RegistrationInfo& rhs);
static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_6::RegistrationInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_6::RegistrationInfo& rhs);

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_6::StatusForAccessTech& o);
static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_6::StatusForAccessTech& lhs, const ::vendor::qti::hardware::radio::ims::V1_6::StatusForAccessTech& rhs);
static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_6::StatusForAccessTech& lhs, const ::vendor::qti::hardware::radio::ims::V1_6::StatusForAccessTech& rhs);

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_6::ServiceStatusInfo& o);
static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_6::ServiceStatusInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_6::ServiceStatusInfo& rhs);
static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_6::ServiceStatusInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_6::ServiceStatusInfo& rhs);

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_6::CallDetails& o);
static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_6::CallDetails& lhs, const ::vendor::qti::hardware::radio::ims::V1_6::CallDetails& rhs);
static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_6::CallDetails& lhs, const ::vendor::qti::hardware::radio::ims::V1_6::CallDetails& rhs);

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_6::CallModifyInfo& o);
static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_6::CallModifyInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_6::CallModifyInfo& rhs);
static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_6::CallModifyInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_6::CallModifyInfo& rhs);

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_6::CallInfo& o);
static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_6::CallInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_6::CallInfo& rhs);
static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_6::CallInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_6::CallInfo& rhs);

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_6::HandoverInfo& o);
static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_6::HandoverInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_6::HandoverInfo& rhs);
static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_6::HandoverInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_6::HandoverInfo& rhs);

//
// type header definitions for package
//
enum class ConfigItem : int32_t {
    CONFIG_ITEM_NONE = 0,
    CONFIG_ITEM_VOCODER_AMRMODESET = 1 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_NONE implicitly + 1) */,
    CONFIG_ITEM_VOCODER_AMRWBMODESET = 2 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VOCODER_AMRMODESET implicitly + 1) */,
    CONFIG_ITEM_SIP_SESSION_TIMER = 3 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VOCODER_AMRWBMODESET implicitly + 1) */,
    CONFIG_ITEM_MIN_SESSION_EXPIRY = 4 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_SESSION_TIMER implicitly + 1) */,
    CONFIG_ITEM_CANCELLATION_TIMER = 5 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_MIN_SESSION_EXPIRY implicitly + 1) */,
    CONFIG_ITEM_T_DELAY = 6 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_CANCELLATION_TIMER implicitly + 1) */,
    CONFIG_ITEM_SILENT_REDIAL_ENABLE = 7 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_T_DELAY implicitly + 1) */,
    CONFIG_ITEM_SIP_T1_TIMER = 8 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SILENT_REDIAL_ENABLE implicitly + 1) */,
    CONFIG_ITEM_SIP_T2_TIMER = 9 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_T1_TIMER implicitly + 1) */,
    CONFIG_ITEM_SIP_TF_TIMER = 10 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_T2_TIMER implicitly + 1) */,
    CONFIG_ITEM_VLT_SETTING_ENABLED = 11 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_TF_TIMER implicitly + 1) */,
    CONFIG_ITEM_LVC_SETTING_ENABLED = 12 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VLT_SETTING_ENABLED implicitly + 1) */,
    CONFIG_ITEM_DOMAIN_NAME = 13 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_LVC_SETTING_ENABLED implicitly + 1) */,
    CONFIG_ITEM_SMS_FORMAT = 14 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_DOMAIN_NAME implicitly + 1) */,
    CONFIG_ITEM_SMS_OVER_IP = 15 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SMS_FORMAT implicitly + 1) */,
    CONFIG_ITEM_PUBLISH_TIMER = 16 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SMS_OVER_IP implicitly + 1) */,
    CONFIG_ITEM_PUBLISH_TIMER_EXTENDED = 17 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_PUBLISH_TIMER implicitly + 1) */,
    CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION = 18 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_PUBLISH_TIMER_EXTENDED implicitly + 1) */,
    CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION = 19 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION implicitly + 1) */,
    CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL = 20 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION implicitly + 1) */,
    CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH = 21 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL implicitly + 1) */,
    CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL = 22 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH implicitly + 1) */,
    CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP = 23 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL implicitly + 1) */,
    CONFIG_ITEM_GZIP_FLAG = 24 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP implicitly + 1) */,
    CONFIG_ITEM_EAB_SETTING_ENABLED = 25 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_GZIP_FLAG implicitly + 1) */,
    CONFIG_ITEM_MOBILE_DATA_ENABLED = 26 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_EAB_SETTING_ENABLED implicitly + 1) */,
    CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED = 27 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_MOBILE_DATA_ENABLED implicitly + 1) */,
    CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING = 28 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED implicitly + 1) */,
    CONFIG_ITEM_VOICE_OVER_WIFI_MODE = 29 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING implicitly + 1) */,
    CONFIG_ITEM_CAPABILITY_DISCOVERY_ENABLED = 30 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VOICE_OVER_WIFI_MODE implicitly + 1) */,
    CONFIG_ITEM_EMERGENCY_CALL_TIMER = 31 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_CAPABILITY_DISCOVERY_ENABLED implicitly + 1) */,
    CONFIG_ITEM_SSAC_HYSTERESIS_TIMER = 32 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_EMERGENCY_CALL_TIMER implicitly + 1) */,
    CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS = 33 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SSAC_HYSTERESIS_TIMER implicitly + 1) */,
    CONFIG_ITEM_LBO_PCSCF_ADDRESS = 34 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS implicitly + 1) */,
    CONFIG_ITEM_KEEP_ALIVE_ENABLED = 35 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_LBO_PCSCF_ADDRESS implicitly + 1) */,
    CONFIG_ITEM_REGISTRATION_RETRY_BASE_TIME_SEC = 36 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_KEEP_ALIVE_ENABLED implicitly + 1) */,
    CONFIG_ITEM_REGISTRATION_RETRY_MAX_TIME_SEC = 37 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_REGISTRATION_RETRY_BASE_TIME_SEC implicitly + 1) */,
    CONFIG_ITEM_SPEECH_START_PORT = 38 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_REGISTRATION_RETRY_MAX_TIME_SEC implicitly + 1) */,
    CONFIG_ITEM_SPEECH_END_PORT = 39 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SPEECH_START_PORT implicitly + 1) */,
    CONFIG_ITEM_SIP_INVITE_REQ_RETX_INTERVAL_MSEC = 40 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SPEECH_END_PORT implicitly + 1) */,
    CONFIG_ITEM_SIP_INVITE_RSP_WAIT_TIME_MSEC = 41 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_INVITE_REQ_RETX_INTERVAL_MSEC implicitly + 1) */,
    CONFIG_ITEM_SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC = 42 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_INVITE_RSP_WAIT_TIME_MSEC implicitly + 1) */,
    CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC = 43 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC implicitly + 1) */,
    CONFIG_ITEM_SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC = 44 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC implicitly + 1) */,
    CONFIG_ITEM_SIP_INVITE_RSP_RETX_INTERVAL_MSEC = 45 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC implicitly + 1) */,
    CONFIG_ITEM_SIP_ACK_RECEIPT_WAIT_TIME_MSEC = 46 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_INVITE_RSP_RETX_INTERVAL_MSEC implicitly + 1) */,
    CONFIG_ITEM_SIP_ACK_RETX_WAIT_TIME_MSEC = 47 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_ACK_RECEIPT_WAIT_TIME_MSEC implicitly + 1) */,
    CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC = 48 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_ACK_RETX_WAIT_TIME_MSEC implicitly + 1) */,
    CONFIG_ITEM_SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC = 49 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC implicitly + 1) */,
    CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT = 50 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC implicitly + 1) */,
    CONFIG_ITEM_AMR_WB_BANDWIDTH_EFFICIENT_PT = 51 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT implicitly + 1) */,
    CONFIG_ITEM_AMR_OCTET_ALIGNED_PT = 52 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_AMR_WB_BANDWIDTH_EFFICIENT_PT implicitly + 1) */,
    CONFIG_ITEM_AMR_BANDWIDTH_EFFICIENT_PT = 53 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_AMR_OCTET_ALIGNED_PT implicitly + 1) */,
    CONFIG_ITEM_DTMF_WB_PT = 54 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_AMR_BANDWIDTH_EFFICIENT_PT implicitly + 1) */,
    CONFIG_ITEM_DTMF_NB_PT = 55 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_DTMF_WB_PT implicitly + 1) */,
    CONFIG_ITEM_AMR_DEFAULT_MODE = 56 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_DTMF_NB_PT implicitly + 1) */,
    CONFIG_ITEM_SMS_PSI = 57 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_AMR_DEFAULT_MODE implicitly + 1) */,
    CONFIG_ITEM_VIDEO_QUALITY = 58 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SMS_PSI implicitly + 1) */,
    CONFIG_ITEM_THRESHOLD_LTE1 = 59 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VIDEO_QUALITY implicitly + 1) */,
    CONFIG_ITEM_THRESHOLD_LTE2 = 60 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_THRESHOLD_LTE1 implicitly + 1) */,
    CONFIG_ITEM_THRESHOLD_LTE3 = 61 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_THRESHOLD_LTE2 implicitly + 1) */,
    CONFIG_ITEM_THRESHOLD_1x = 62 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_THRESHOLD_LTE3 implicitly + 1) */,
    CONFIG_ITEM_THRESHOLD_WIFI_A = 63 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_THRESHOLD_1x implicitly + 1) */,
    CONFIG_ITEM_THRESHOLD_WIFI_B = 64 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_THRESHOLD_WIFI_A implicitly + 1) */,
    CONFIG_ITEM_T_EPDG_LTE = 65 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_THRESHOLD_WIFI_B implicitly + 1) */,
    CONFIG_ITEM_T_EPDG_WIFI = 66 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_T_EPDG_LTE implicitly + 1) */,
    CONFIG_ITEM_T_EPDG_1x = 67 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_T_EPDG_WIFI implicitly + 1) */,
    CONFIG_ITEM_VWF_SETTING_ENABLED = 68 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_T_EPDG_1x implicitly + 1) */,
    CONFIG_ITEM_VCE_SETTING_ENABLED = 69 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VWF_SETTING_ENABLED implicitly + 1) */,
    CONFIG_ITEM_RTT_SETTING_ENABLED = 70 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VCE_SETTING_ENABLED implicitly + 1) */,
    CONFIG_ITEM_SMS_APP = 71 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_RTT_SETTING_ENABLED implicitly + 1) */,
    CONFIG_ITEM_VVM_APP = 72 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_SMS_APP implicitly + 1) */,
    CONFIG_ITEM_INVALID = 73 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VVM_APP implicitly + 1) */,
    CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE = 74 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_INVALID implicitly + 1) */,
    CONFIG_ITEM_SET_AUTO_REJECT_CALL_MODE_CONFIG = 75 /* (::vendor::qti::hardware::radio::ims::V1_0::ConfigItem.CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE implicitly + 1) */,
};

struct ConfigInfo final {
    ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem item __attribute__ ((aligned(4)));
    bool hasBoolValue __attribute__ ((aligned(1)));
    bool boolValue __attribute__ ((aligned(1)));
    uint32_t intValue __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string stringValue __attribute__ ((aligned(8)));
    ::vendor::qti::hardware::radio::ims::V1_0::ConfigFailureCause errorCause __attribute__ ((aligned(4)));
};

static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::ConfigInfo, item) == 0, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::ConfigInfo, hasBoolValue) == 4, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::ConfigInfo, boolValue) == 5, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::ConfigInfo, intValue) == 8, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::ConfigInfo, stringValue) == 16, "wrong offset");
static_assert(offsetof(::vendor::qti::hardware::radio::ims::V1_6::ConfigInfo, errorCause) == 32, "wrong offset");
static_assert(sizeof(::vendor::qti::hardware::radio::ims::V1_6::ConfigInfo) == 40, "wrong size");
static_assert(__alignof(::vendor::qti::hardware::radio::ims::V1_6::ConfigInfo) == 8, "wrong alignment");

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_6::ConfigInfo& o);
static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_6::ConfigItem o);

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_6::ConfigInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".item = ";
    os += ::vendor::qti::hardware::radio::ims::V1_6::toString(o.item);
    os += ", .hasBoolValue = ";
    os += ::android::hardware::toString(o.hasBoolValue);
    os += ", .boolValue = ";
    os += ::android::hardware::toString(o.boolValue);
    os += ", .intValue = ";
    os += ::android::hardware::toString(o.intValue);
    os += ", .stringValue = ";
    os += ::android::hardware::toString(o.stringValue);
    os += ", .errorCause = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.errorCause);
    os += "}"; return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_6::ConfigItem o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_NONE) {
        return "CONFIG_ITEM_NONE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_VOCODER_AMRMODESET) {
        return "CONFIG_ITEM_VOCODER_AMRMODESET";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_VOCODER_AMRWBMODESET) {
        return "CONFIG_ITEM_VOCODER_AMRWBMODESET";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_SIP_SESSION_TIMER) {
        return "CONFIG_ITEM_SIP_SESSION_TIMER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_MIN_SESSION_EXPIRY) {
        return "CONFIG_ITEM_MIN_SESSION_EXPIRY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_CANCELLATION_TIMER) {
        return "CONFIG_ITEM_CANCELLATION_TIMER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_T_DELAY) {
        return "CONFIG_ITEM_T_DELAY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_SILENT_REDIAL_ENABLE) {
        return "CONFIG_ITEM_SILENT_REDIAL_ENABLE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_SIP_T1_TIMER) {
        return "CONFIG_ITEM_SIP_T1_TIMER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_SIP_T2_TIMER) {
        return "CONFIG_ITEM_SIP_T2_TIMER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_SIP_TF_TIMER) {
        return "CONFIG_ITEM_SIP_TF_TIMER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_VLT_SETTING_ENABLED) {
        return "CONFIG_ITEM_VLT_SETTING_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_LVC_SETTING_ENABLED) {
        return "CONFIG_ITEM_LVC_SETTING_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_DOMAIN_NAME) {
        return "CONFIG_ITEM_DOMAIN_NAME";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_SMS_FORMAT) {
        return "CONFIG_ITEM_SMS_FORMAT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_SMS_OVER_IP) {
        return "CONFIG_ITEM_SMS_OVER_IP";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_PUBLISH_TIMER) {
        return "CONFIG_ITEM_PUBLISH_TIMER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_PUBLISH_TIMER_EXTENDED) {
        return "CONFIG_ITEM_PUBLISH_TIMER_EXTENDED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION) {
        return "CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION) {
        return "CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL) {
        return "CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH) {
        return "CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL) {
        return "CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP) {
        return "CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_GZIP_FLAG) {
        return "CONFIG_ITEM_GZIP_FLAG";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_EAB_SETTING_ENABLED) {
        return "CONFIG_ITEM_EAB_SETTING_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_MOBILE_DATA_ENABLED) {
        return "CONFIG_ITEM_MOBILE_DATA_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED) {
        return "CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING) {
        return "CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_MODE) {
        return "CONFIG_ITEM_VOICE_OVER_WIFI_MODE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_CAPABILITY_DISCOVERY_ENABLED) {
        return "CONFIG_ITEM_CAPABILITY_DISCOVERY_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_EMERGENCY_CALL_TIMER) {
        return "CONFIG_ITEM_EMERGENCY_CALL_TIMER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_SSAC_HYSTERESIS_TIMER) {
        return "CONFIG_ITEM_SSAC_HYSTERESIS_TIMER";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS) {
        return "CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_LBO_PCSCF_ADDRESS) {
        return "CONFIG_ITEM_LBO_PCSCF_ADDRESS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_KEEP_ALIVE_ENABLED) {
        return "CONFIG_ITEM_KEEP_ALIVE_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_REGISTRATION_RETRY_BASE_TIME_SEC) {
        return "CONFIG_ITEM_REGISTRATION_RETRY_BASE_TIME_SEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_REGISTRATION_RETRY_MAX_TIME_SEC) {
        return "CONFIG_ITEM_REGISTRATION_RETRY_MAX_TIME_SEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_SPEECH_START_PORT) {
        return "CONFIG_ITEM_SPEECH_START_PORT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_SPEECH_END_PORT) {
        return "CONFIG_ITEM_SPEECH_END_PORT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_SIP_INVITE_REQ_RETX_INTERVAL_MSEC) {
        return "CONFIG_ITEM_SIP_INVITE_REQ_RETX_INTERVAL_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_WAIT_TIME_MSEC) {
        return "CONFIG_ITEM_SIP_INVITE_RSP_WAIT_TIME_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC) {
        return "CONFIG_ITEM_SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC) {
        return "CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC) {
        return "CONFIG_ITEM_SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_SIP_INVITE_RSP_RETX_INTERVAL_MSEC) {
        return "CONFIG_ITEM_SIP_INVITE_RSP_RETX_INTERVAL_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_SIP_ACK_RECEIPT_WAIT_TIME_MSEC) {
        return "CONFIG_ITEM_SIP_ACK_RECEIPT_WAIT_TIME_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_SIP_ACK_RETX_WAIT_TIME_MSEC) {
        return "CONFIG_ITEM_SIP_ACK_RETX_WAIT_TIME_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC) {
        return "CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC) {
        return "CONFIG_ITEM_SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT) {
        return "CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_AMR_WB_BANDWIDTH_EFFICIENT_PT) {
        return "CONFIG_ITEM_AMR_WB_BANDWIDTH_EFFICIENT_PT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_AMR_OCTET_ALIGNED_PT) {
        return "CONFIG_ITEM_AMR_OCTET_ALIGNED_PT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_AMR_BANDWIDTH_EFFICIENT_PT) {
        return "CONFIG_ITEM_AMR_BANDWIDTH_EFFICIENT_PT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_DTMF_WB_PT) {
        return "CONFIG_ITEM_DTMF_WB_PT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_DTMF_NB_PT) {
        return "CONFIG_ITEM_DTMF_NB_PT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_AMR_DEFAULT_MODE) {
        return "CONFIG_ITEM_AMR_DEFAULT_MODE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_SMS_PSI) {
        return "CONFIG_ITEM_SMS_PSI";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_VIDEO_QUALITY) {
        return "CONFIG_ITEM_VIDEO_QUALITY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE1) {
        return "CONFIG_ITEM_THRESHOLD_LTE1";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE2) {
        return "CONFIG_ITEM_THRESHOLD_LTE2";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_THRESHOLD_LTE3) {
        return "CONFIG_ITEM_THRESHOLD_LTE3";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_THRESHOLD_1x) {
        return "CONFIG_ITEM_THRESHOLD_1x";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_THRESHOLD_WIFI_A) {
        return "CONFIG_ITEM_THRESHOLD_WIFI_A";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_THRESHOLD_WIFI_B) {
        return "CONFIG_ITEM_THRESHOLD_WIFI_B";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_T_EPDG_LTE) {
        return "CONFIG_ITEM_T_EPDG_LTE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_T_EPDG_WIFI) {
        return "CONFIG_ITEM_T_EPDG_WIFI";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_T_EPDG_1x) {
        return "CONFIG_ITEM_T_EPDG_1x";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_VWF_SETTING_ENABLED) {
        return "CONFIG_ITEM_VWF_SETTING_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_VCE_SETTING_ENABLED) {
        return "CONFIG_ITEM_VCE_SETTING_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_RTT_SETTING_ENABLED) {
        return "CONFIG_ITEM_RTT_SETTING_ENABLED";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_SMS_APP) {
        return "CONFIG_ITEM_SMS_APP";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_VVM_APP) {
        return "CONFIG_ITEM_VVM_APP";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_INVALID) {
        return "CONFIG_ITEM_INVALID";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE) {
        return "CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::ConfigItem::CONFIG_ITEM_SET_AUTO_REJECT_CALL_MODE_CONFIG) {
        return "CONFIG_ITEM_SET_AUTO_REJECT_CALL_MODE_CONFIG";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

template<>
inline std::string toString<::vendor::qti::hardware::radio::ims::V1_6::RadioTechType>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_6::RadioTechType> flipped = 0;
    bool first = true;
    if ((o & ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_ANY) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_ANY)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_ANY";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_ANY;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_UNKNOWN) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_UNKNOWN)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_UNKNOWN";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_UNKNOWN;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_GPRS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_GPRS)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_GPRS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_GPRS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_EDGE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_EDGE)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_EDGE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_EDGE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_UMTS) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_UMTS)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_UMTS";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_UMTS;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_IS95A) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_IS95A)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_IS95A";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_IS95A;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_IS95B) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_IS95B)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_IS95B";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_IS95B;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_1xRTT) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_1xRTT)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_1xRTT";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_1xRTT;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_EVDO_0) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_EVDO_0)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_EVDO_0";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_EVDO_0;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_EVDO_A) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_EVDO_A)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_EVDO_A";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_EVDO_A;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_HSDPA) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_HSDPA)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_HSDPA";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_HSDPA;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_HSUPA) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_HSUPA)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_HSUPA";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_HSUPA;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_HSPA) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_HSPA)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_HSPA";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_HSPA;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_EVDO_B) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_EVDO_B)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_EVDO_B";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_EVDO_B;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_EHRPD) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_EHRPD)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_EHRPD";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_EHRPD;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_LTE) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_LTE)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_LTE";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_LTE;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_HSPAP) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_HSPAP)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_HSPAP";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_HSPAP;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_GSM) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_GSM)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_GSM";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_GSM;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_TD_SCDMA) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_TD_SCDMA)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_TD_SCDMA";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_TD_SCDMA;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_WIFI) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_WIFI)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_WIFI";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_WIFI;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_IWLAN) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_IWLAN)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_IWLAN";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_IWLAN;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_INVALID) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_INVALID)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_INVALID";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_INVALID;
    }
    if ((o & ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_NR5G) == static_cast<int32_t>(::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_NR5G)) {
        os += (first ? "" : " | ");
        os += "RADIO_TECH_NR5G";
        first = false;
        flipped |= ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_NR5G;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::vendor::qti::hardware::radio::ims::V1_6::RadioTechType o) {
    using ::android::hardware::details::toHexString;
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_ANY) {
        return "RADIO_TECH_ANY";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_UNKNOWN) {
        return "RADIO_TECH_UNKNOWN";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_GPRS) {
        return "RADIO_TECH_GPRS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_EDGE) {
        return "RADIO_TECH_EDGE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_UMTS) {
        return "RADIO_TECH_UMTS";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_IS95A) {
        return "RADIO_TECH_IS95A";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_IS95B) {
        return "RADIO_TECH_IS95B";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_1xRTT) {
        return "RADIO_TECH_1xRTT";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_EVDO_0) {
        return "RADIO_TECH_EVDO_0";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_EVDO_A) {
        return "RADIO_TECH_EVDO_A";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_HSDPA) {
        return "RADIO_TECH_HSDPA";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_HSUPA) {
        return "RADIO_TECH_HSUPA";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_HSPA) {
        return "RADIO_TECH_HSPA";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_EVDO_B) {
        return "RADIO_TECH_EVDO_B";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_EHRPD) {
        return "RADIO_TECH_EHRPD";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_LTE) {
        return "RADIO_TECH_LTE";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_HSPAP) {
        return "RADIO_TECH_HSPAP";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_GSM) {
        return "RADIO_TECH_GSM";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_TD_SCDMA) {
        return "RADIO_TECH_TD_SCDMA";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_WIFI) {
        return "RADIO_TECH_WIFI";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_IWLAN) {
        return "RADIO_TECH_IWLAN";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_INVALID) {
        return "RADIO_TECH_INVALID";
    }
    if (o == ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_NR5G) {
        return "RADIO_TECH_NR5G";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_6::RegistrationInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".state = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.state);
    os += ", .errorCode = ";
    os += ::android::hardware::toString(o.errorCode);
    os += ", .errorMessage = ";
    os += ::android::hardware::toString(o.errorMessage);
    os += ", .radioTech = ";
    os += ::vendor::qti::hardware::radio::ims::V1_6::toString(o.radioTech);
    os += ", .pAssociatedUris = ";
    os += ::android::hardware::toString(o.pAssociatedUris);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_6::RegistrationInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_6::RegistrationInfo& rhs) {
    if (lhs.state != rhs.state) {
        return false;
    }
    if (lhs.errorCode != rhs.errorCode) {
        return false;
    }
    if (lhs.errorMessage != rhs.errorMessage) {
        return false;
    }
    if (lhs.radioTech != rhs.radioTech) {
        return false;
    }
    if (lhs.pAssociatedUris != rhs.pAssociatedUris) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_6::RegistrationInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_6::RegistrationInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_6::StatusForAccessTech& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".networkMode = ";
    os += ::vendor::qti::hardware::radio::ims::V1_6::toString(o.networkMode);
    os += ", .status = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.status);
    os += ", .restrictCause = ";
    os += ::android::hardware::toString(o.restrictCause);
    os += ", .hasRegistration = ";
    os += ::android::hardware::toString(o.hasRegistration);
    os += ", .registration = ";
    os += ::vendor::qti::hardware::radio::ims::V1_6::toString(o.registration);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_6::StatusForAccessTech& lhs, const ::vendor::qti::hardware::radio::ims::V1_6::StatusForAccessTech& rhs) {
    if (lhs.networkMode != rhs.networkMode) {
        return false;
    }
    if (lhs.status != rhs.status) {
        return false;
    }
    if (lhs.restrictCause != rhs.restrictCause) {
        return false;
    }
    if (lhs.hasRegistration != rhs.hasRegistration) {
        return false;
    }
    if (lhs.registration != rhs.registration) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_6::StatusForAccessTech& lhs, const ::vendor::qti::hardware::radio::ims::V1_6::StatusForAccessTech& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_6::ServiceStatusInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".hasIsValid = ";
    os += ::android::hardware::toString(o.hasIsValid);
    os += ", .isValid = ";
    os += ::android::hardware::toString(o.isValid);
    os += ", .type = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.type);
    os += ", .callType = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.callType);
    os += ", .status = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.status);
    os += ", .userdata = ";
    os += ::android::hardware::toString(o.userdata);
    os += ", .restrictCause = ";
    os += ::android::hardware::toString(o.restrictCause);
    os += ", .accTechStatus = ";
    os += ::android::hardware::toString(o.accTechStatus);
    os += ", .rttMode = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.rttMode);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_6::ServiceStatusInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_6::ServiceStatusInfo& rhs) {
    if (lhs.hasIsValid != rhs.hasIsValid) {
        return false;
    }
    if (lhs.isValid != rhs.isValid) {
        return false;
    }
    if (lhs.type != rhs.type) {
        return false;
    }
    if (lhs.callType != rhs.callType) {
        return false;
    }
    if (lhs.status != rhs.status) {
        return false;
    }
    if (lhs.userdata != rhs.userdata) {
        return false;
    }
    if (lhs.restrictCause != rhs.restrictCause) {
        return false;
    }
    if (lhs.accTechStatus != rhs.accTechStatus) {
        return false;
    }
    if (lhs.rttMode != rhs.rttMode) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_6::ServiceStatusInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_6::ServiceStatusInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_6::CallDetails& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".callType = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.callType);
    os += ", .callDomain = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.callDomain);
    os += ", .extrasLength = ";
    os += ::android::hardware::toString(o.extrasLength);
    os += ", .extras = ";
    os += ::android::hardware::toString(o.extras);
    os += ", .localAbility = ";
    os += ::android::hardware::toString(o.localAbility);
    os += ", .peerAbility = ";
    os += ::android::hardware::toString(o.peerAbility);
    os += ", .callSubstate = ";
    os += ::android::hardware::toString(o.callSubstate);
    os += ", .mediaId = ";
    os += ::android::hardware::toString(o.mediaId);
    os += ", .causeCode = ";
    os += ::android::hardware::toString(o.causeCode);
    os += ", .rttMode = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.rttMode);
    os += ", .sipAlternateUri = ";
    os += ::android::hardware::toString(o.sipAlternateUri);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_6::CallDetails& lhs, const ::vendor::qti::hardware::radio::ims::V1_6::CallDetails& rhs) {
    if (lhs.callType != rhs.callType) {
        return false;
    }
    if (lhs.callDomain != rhs.callDomain) {
        return false;
    }
    if (lhs.extrasLength != rhs.extrasLength) {
        return false;
    }
    if (lhs.extras != rhs.extras) {
        return false;
    }
    if (lhs.localAbility != rhs.localAbility) {
        return false;
    }
    if (lhs.peerAbility != rhs.peerAbility) {
        return false;
    }
    if (lhs.callSubstate != rhs.callSubstate) {
        return false;
    }
    if (lhs.mediaId != rhs.mediaId) {
        return false;
    }
    if (lhs.causeCode != rhs.causeCode) {
        return false;
    }
    if (lhs.rttMode != rhs.rttMode) {
        return false;
    }
    if (lhs.sipAlternateUri != rhs.sipAlternateUri) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_6::CallDetails& lhs, const ::vendor::qti::hardware::radio::ims::V1_6::CallDetails& rhs){
    return !(lhs == rhs);
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_6::CallModifyInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_6::CallModifyInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_6::CallInfo& o) {
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
    os += ::vendor::qti::hardware::radio::ims::V1_6::toString(o.callDetails);
    os += ", .hasFailCause = ";
    os += ::android::hardware::toString(o.hasFailCause);
    os += ", .failCause = ";
    os += ::vendor::qti::hardware::radio::ims::V1_5::toString(o.failCause);
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
    os += ", .tirMode = ";
    os += ::vendor::qti::hardware::radio::ims::V1_5::toString(o.tirMode);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_6::CallInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_6::CallInfo& rhs) {
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
    if (lhs.tirMode != rhs.tirMode) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_6::CallInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_6::CallInfo& rhs){
    return !(lhs == rhs);
}

static inline std::string toString(const ::vendor::qti::hardware::radio::ims::V1_6::HandoverInfo& o) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += ".type = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.type);
    os += ", .srcTech = ";
    os += ::vendor::qti::hardware::radio::ims::V1_6::toString(o.srcTech);
    os += ", .targetTech = ";
    os += ::vendor::qti::hardware::radio::ims::V1_6::toString(o.targetTech);
    os += ", .hasHoExtra = ";
    os += ::android::hardware::toString(o.hasHoExtra);
    os += ", .hoExtra = ";
    os += ::vendor::qti::hardware::radio::ims::V1_0::toString(o.hoExtra);
    os += ", .errorCode = ";
    os += ::android::hardware::toString(o.errorCode);
    os += ", .errorMessage = ";
    os += ::android::hardware::toString(o.errorMessage);
    os += "}"; return os;
}

static inline bool operator==(const ::vendor::qti::hardware::radio::ims::V1_6::HandoverInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_6::HandoverInfo& rhs) {
    if (lhs.type != rhs.type) {
        return false;
    }
    if (lhs.srcTech != rhs.srcTech) {
        return false;
    }
    if (lhs.targetTech != rhs.targetTech) {
        return false;
    }
    if (lhs.hasHoExtra != rhs.hasHoExtra) {
        return false;
    }
    if (lhs.hoExtra != rhs.hoExtra) {
        return false;
    }
    if (lhs.errorCode != rhs.errorCode) {
        return false;
    }
    if (lhs.errorMessage != rhs.errorMessage) {
        return false;
    }
    return true;
}

static inline bool operator!=(const ::vendor::qti::hardware::radio::ims::V1_6::HandoverInfo& lhs, const ::vendor::qti::hardware::radio::ims::V1_6::HandoverInfo& rhs){
    return !(lhs == rhs);
}

}  // namespace V1_6
}  // namespace ims
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
template<> constexpr std::array<::vendor::qti::hardware::radio::ims::V1_6::RadioTechType, 23> hidl_enum_values<::vendor::qti::hardware::radio::ims::V1_6::RadioTechType> = {
    ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_ANY,
    ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_UNKNOWN,
    ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_GPRS,
    ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_EDGE,
    ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_UMTS,
    ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_IS95A,
    ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_IS95B,
    ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_1xRTT,
    ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_EVDO_0,
    ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_EVDO_A,
    ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_HSDPA,
    ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_HSUPA,
    ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_HSPA,
    ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_EVDO_B,
    ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_EHRPD,
    ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_LTE,
    ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_HSPAP,
    ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_GSM,
    ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_TD_SCDMA,
    ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_WIFI,
    ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_IWLAN,
    ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_INVALID,
    ::vendor::qti::hardware::radio::ims::V1_6::RadioTechType::RADIO_TECH_NR5G,
};
}  // namespace details
}  // namespace hardware
}  // namespace android


#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_6_TYPES_H
