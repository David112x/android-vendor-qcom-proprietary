/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file framework/hidl_gen/lpa/vendor/qti/hardware/radio/lpa/1.0/types.h
  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_LPA_V1_0_TYPES_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_LPA_V1_0_TYPES_H

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace lpa {
namespace V1_0 {

enum class UimLpaUserEventId : uint32_t {
    UIM_LPA_UNKNOWN_EVENT_ID = 0u, // 0
    UIM_LPA_ADD_PROFILE = 1u, // 1
    UIM_LPA_ENABLE_PROFILE = 2u, // 2
    UIM_LPA_DISABLE_PROFILE = 3u, // 3
    UIM_LPA_DELETE_PROFILE = 4u, // 4
    UIM_LPA_EUICC_MEMORY_RESET = 5u, // 5
    UIM_LPA_GET_PROFILE = 6u, // 6
    UIM_LPA_UPDATE_NICKNAME = 7u, // 7
    UIM_LPA_GET_EID = 8u, // 8
    UIM_LPA_USER_CONSENT = 9u, // 9
    UIM_LPA_SRV_ADDR_OPERATION = 10u, // 10
};

enum class UimLpaResult : uint32_t {
    UIM_LPA_RESULT_SUCCESS = 0u, // 0
    UIM_LPA_RESULT_FAILURE = 1u, // 1
    UIM_LPA_RESULT_CONFRM_CODE_MISSING = 2u, // 2
};

enum class UimLpaProfileClassType : uint32_t {
    UIM_LPA_PROFILE_CLASS_TEST = 0u, // 0
    UIM_LPA_PROFILE_CLASS_PROVISIONING = 1u, // 1
    UIM_LPA_PROFILE_CLASS_OPERATIONAL = 2u, // 2
};

enum class UimLpaProfilePolicyMask : uint32_t {
    UIM_LPA_PROFILE_TYPE_DISABLE_NOT_ALLOWED = 1u, // 1
    UIM_LPA_PROFILE_TYPE_DELETE_NOT_ALLOWED = 2u, // 2
    UIM_LPA_PROFILE_TYPE_DELETE_ON_DISABLE = 4u, // 4
};

enum class UimLpaEuiccResetMask : uint32_t {
    UIM_LPA_EUICC_RESET_TEST_PROFILES = 1u, // 1
    UIM_LPA_EUICC_RESET_OPERATIONAL_PROFILES = 2u, // 2
    UIM_LPA_EUICC_RESET_TO_DEFAULT_SMDP_ADDRESS = 4u, // 4
};

enum class UimLpaSrvAddrOp : uint32_t {
    UIM_LPA_SERVER_ADDRESS_GET_OPERATION = 1u, // 1
    UIM_LPA_SERVER_ADDRESS_SET_OPERATION = 2u, // 2
};

enum class UimLpaProfileState : uint32_t {
    UIM_LPA_PROFILE_STATE_UNKNOWN = 0u, // 0
    UIM_LPA_PROFILE_STATE_ACTIVE = 1u, // 1
    UIM_LPA_PROFILE_STATE_INACTIVE = 2u, // 2
};

enum class UimLpaIconType : uint32_t {
    UIM_LPA_ICON_TYPE_UNKNOWN = 0u, // 0
    UIM_LPA_ICON_TYPE_JPEG = 1u, // 1
    UIM_LPA_ICON_TYPE_PNG = 2u, // 2
};

enum class UimLpaAddProfileStatus : uint32_t {
    UIM_LPA_ADD_PROFILE_STATUS_NONE = 0u, // 0
    UIM_LPA_ADD_PROFILE_STATUS_ERROR = 1u, // 1
    UIM_LPA_ADD_PROFILE_STATUS_DOWNLOAD_PROGRESS = 2u, // 2
    UIM_LPA_ADD_PROFILE_STATUS_INSTALLATION_PROGRESS = 3u, // 3
    UIM_LPA_ADD_PROFILE_STATUS_INSTALLATION_COMPLETE = 4u, // 4
    UIM_LPA_ADD_PROFILE_STATUS_GET_USER_CONSENT = 5u, // 5
};

enum class UimLpaAddProfileFailureCause : uint32_t {
    UIM_LPA_ADD_PROFILE_FAILURE_CAUSE_NONE = 0u, // 0
    UIM_LPA_ADD_PROFILE_FAILURE_CAUSE_GENERIC = 1u, // 1
    UIM_LPA_ADD_PROFILE_FAILURE_CAUSE_SIM = 2u, // 2
    UIM_LPA_ADD_PROFILE_FAILURE_CAUSE_NETWORK = 3u, // 3
    UIM_LPA_ADD_PROFILE_FAILURE_CAUSE_MEMORY = 4u, // 4
};

struct UimLpaSrvAddrOpReq final {
    UimLpaSrvAddrOp opCode __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string smdpAddress __attribute__ ((aligned(8)));
};

static_assert(offsetof(UimLpaSrvAddrOpReq, opCode) == 0, "wrong offset");
static_assert(offsetof(UimLpaSrvAddrOpReq, smdpAddress) == 8, "wrong offset");
static_assert(sizeof(UimLpaSrvAddrOpReq) == 24, "wrong size");
static_assert(__alignof(UimLpaSrvAddrOpReq) == 8, "wrong alignment");

struct UimLpaUserReq final {
    UimLpaUserEventId event __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string activationCode __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string confirmationCode __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string nickname __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<uint8_t> iccid __attribute__ ((aligned(8)));
    int32_t resetMask __attribute__ ((aligned(4)));
    bool userOk __attribute__ ((aligned(1)));
    UimLpaSrvAddrOpReq srvOpReq __attribute__ ((aligned(8)));
};

static_assert(offsetof(UimLpaUserReq, event) == 0, "wrong offset");
static_assert(offsetof(UimLpaUserReq, activationCode) == 8, "wrong offset");
static_assert(offsetof(UimLpaUserReq, confirmationCode) == 24, "wrong offset");
static_assert(offsetof(UimLpaUserReq, nickname) == 40, "wrong offset");
static_assert(offsetof(UimLpaUserReq, iccid) == 56, "wrong offset");
static_assert(offsetof(UimLpaUserReq, resetMask) == 72, "wrong offset");
static_assert(offsetof(UimLpaUserReq, userOk) == 76, "wrong offset");
static_assert(offsetof(UimLpaUserReq, srvOpReq) == 80, "wrong offset");
static_assert(sizeof(UimLpaUserReq) == 104, "wrong size");
static_assert(__alignof(UimLpaUserReq) == 8, "wrong alignment");

struct UimLpaProfileInfo final {
    UimLpaProfileState state __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<uint8_t> iccid __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string profileName __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string nickName __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string spName __attribute__ ((aligned(8)));
    UimLpaIconType iconType __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<uint8_t> icon __attribute__ ((aligned(8)));
    UimLpaProfileClassType profileClass __attribute__ ((aligned(4)));
    UimLpaProfilePolicyMask profilePolicy __attribute__ ((aligned(4)));
};

static_assert(offsetof(UimLpaProfileInfo, state) == 0, "wrong offset");
static_assert(offsetof(UimLpaProfileInfo, iccid) == 8, "wrong offset");
static_assert(offsetof(UimLpaProfileInfo, profileName) == 24, "wrong offset");
static_assert(offsetof(UimLpaProfileInfo, nickName) == 40, "wrong offset");
static_assert(offsetof(UimLpaProfileInfo, spName) == 56, "wrong offset");
static_assert(offsetof(UimLpaProfileInfo, iconType) == 72, "wrong offset");
static_assert(offsetof(UimLpaProfileInfo, icon) == 80, "wrong offset");
static_assert(offsetof(UimLpaProfileInfo, profileClass) == 96, "wrong offset");
static_assert(offsetof(UimLpaProfileInfo, profilePolicy) == 100, "wrong offset");
static_assert(sizeof(UimLpaProfileInfo) == 104, "wrong size");
static_assert(__alignof(UimLpaProfileInfo) == 8, "wrong alignment");

struct UimLpaSrvAddrOpResp final {
    ::android::hardware::hidl_string smdpAddress __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string smdsAddress __attribute__ ((aligned(8)));
};

static_assert(offsetof(UimLpaSrvAddrOpResp, smdpAddress) == 0, "wrong offset");
static_assert(offsetof(UimLpaSrvAddrOpResp, smdsAddress) == 16, "wrong offset");
static_assert(sizeof(UimLpaSrvAddrOpResp) == 32, "wrong size");
static_assert(__alignof(UimLpaSrvAddrOpResp) == 8, "wrong alignment");

struct UimLpaUserResp final {
    UimLpaUserEventId event __attribute__ ((aligned(4)));
    UimLpaResult result __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<uint8_t> eid __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<UimLpaProfileInfo> profiles __attribute__ ((aligned(8)));
    UimLpaSrvAddrOpResp srvAddr __attribute__ ((aligned(8)));
};

static_assert(offsetof(UimLpaUserResp, event) == 0, "wrong offset");
static_assert(offsetof(UimLpaUserResp, result) == 4, "wrong offset");
static_assert(offsetof(UimLpaUserResp, eid) == 8, "wrong offset");
static_assert(offsetof(UimLpaUserResp, profiles) == 24, "wrong offset");
static_assert(offsetof(UimLpaUserResp, srvAddr) == 40, "wrong offset");
static_assert(sizeof(UimLpaUserResp) == 72, "wrong size");
static_assert(__alignof(UimLpaUserResp) == 8, "wrong alignment");

struct UimLpaAddProfileProgressInd final {
    UimLpaAddProfileStatus status __attribute__ ((aligned(4)));
    UimLpaAddProfileFailureCause cause __attribute__ ((aligned(4)));
    int32_t progress __attribute__ ((aligned(4)));
    UimLpaProfilePolicyMask policyMask __attribute__ ((aligned(4)));
    bool userConsentRequired __attribute__ ((aligned(1)));
};

static_assert(offsetof(UimLpaAddProfileProgressInd, status) == 0, "wrong offset");
static_assert(offsetof(UimLpaAddProfileProgressInd, cause) == 4, "wrong offset");
static_assert(offsetof(UimLpaAddProfileProgressInd, progress) == 8, "wrong offset");
static_assert(offsetof(UimLpaAddProfileProgressInd, policyMask) == 12, "wrong offset");
static_assert(offsetof(UimLpaAddProfileProgressInd, userConsentRequired) == 16, "wrong offset");
static_assert(sizeof(UimLpaAddProfileProgressInd) == 20, "wrong size");
static_assert(__alignof(UimLpaAddProfileProgressInd) == 4, "wrong alignment");

struct UimLpaHttpCustomHeader final {
    ::android::hardware::hidl_string headerName __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string headerValue __attribute__ ((aligned(8)));
};

static_assert(offsetof(UimLpaHttpCustomHeader, headerName) == 0, "wrong offset");
static_assert(offsetof(UimLpaHttpCustomHeader, headerValue) == 16, "wrong offset");
static_assert(sizeof(UimLpaHttpCustomHeader) == 32, "wrong size");
static_assert(__alignof(UimLpaHttpCustomHeader) == 8, "wrong alignment");

struct UimLpaHttpTransactionInd final {
    int32_t tokenId __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<uint8_t> payload __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string contentType __attribute__ ((aligned(8)));
    ::android::hardware::hidl_vec<UimLpaHttpCustomHeader> customHeaders __attribute__ ((aligned(8)));
    ::android::hardware::hidl_string url __attribute__ ((aligned(8)));
};

static_assert(offsetof(UimLpaHttpTransactionInd, tokenId) == 0, "wrong offset");
static_assert(offsetof(UimLpaHttpTransactionInd, payload) == 8, "wrong offset");
static_assert(offsetof(UimLpaHttpTransactionInd, contentType) == 24, "wrong offset");
static_assert(offsetof(UimLpaHttpTransactionInd, customHeaders) == 40, "wrong offset");
static_assert(offsetof(UimLpaHttpTransactionInd, url) == 56, "wrong offset");
static_assert(sizeof(UimLpaHttpTransactionInd) == 72, "wrong size");
static_assert(__alignof(UimLpaHttpTransactionInd) == 8, "wrong alignment");

constexpr uint32_t operator|(const UimLpaUserEventId lhs, const UimLpaUserEventId rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const UimLpaUserEventId rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const UimLpaUserEventId lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const UimLpaUserEventId lhs, const UimLpaUserEventId rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const UimLpaUserEventId rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const UimLpaUserEventId lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const UimLpaUserEventId e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const UimLpaUserEventId e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
std::string toString(uint32_t o);
template<>
std::string toString<UimLpaUserEventId>(uint32_t o);

std::string toString(UimLpaUserEventId o);

constexpr uint32_t operator|(const UimLpaResult lhs, const UimLpaResult rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const UimLpaResult rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const UimLpaResult lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const UimLpaResult lhs, const UimLpaResult rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const UimLpaResult rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const UimLpaResult lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const UimLpaResult e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const UimLpaResult e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
std::string toString(uint32_t o);
template<>
std::string toString<UimLpaResult>(uint32_t o);

std::string toString(UimLpaResult o);

constexpr uint32_t operator|(const UimLpaProfileClassType lhs, const UimLpaProfileClassType rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const UimLpaProfileClassType rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const UimLpaProfileClassType lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const UimLpaProfileClassType lhs, const UimLpaProfileClassType rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const UimLpaProfileClassType rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const UimLpaProfileClassType lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const UimLpaProfileClassType e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const UimLpaProfileClassType e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
std::string toString(uint32_t o);
template<>
std::string toString<UimLpaProfileClassType>(uint32_t o);

std::string toString(UimLpaProfileClassType o);

constexpr uint32_t operator|(const UimLpaProfilePolicyMask lhs, const UimLpaProfilePolicyMask rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const UimLpaProfilePolicyMask rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const UimLpaProfilePolicyMask lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const UimLpaProfilePolicyMask lhs, const UimLpaProfilePolicyMask rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const UimLpaProfilePolicyMask rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const UimLpaProfilePolicyMask lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const UimLpaProfilePolicyMask e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const UimLpaProfilePolicyMask e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
std::string toString(uint32_t o);
template<>
std::string toString<UimLpaProfilePolicyMask>(uint32_t o);

std::string toString(UimLpaProfilePolicyMask o);

constexpr uint32_t operator|(const UimLpaEuiccResetMask lhs, const UimLpaEuiccResetMask rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const UimLpaEuiccResetMask rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const UimLpaEuiccResetMask lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const UimLpaEuiccResetMask lhs, const UimLpaEuiccResetMask rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const UimLpaEuiccResetMask rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const UimLpaEuiccResetMask lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const UimLpaEuiccResetMask e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const UimLpaEuiccResetMask e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
std::string toString(uint32_t o);
template<>
std::string toString<UimLpaEuiccResetMask>(uint32_t o);

std::string toString(UimLpaEuiccResetMask o);

constexpr uint32_t operator|(const UimLpaSrvAddrOp lhs, const UimLpaSrvAddrOp rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const UimLpaSrvAddrOp rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const UimLpaSrvAddrOp lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const UimLpaSrvAddrOp lhs, const UimLpaSrvAddrOp rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const UimLpaSrvAddrOp rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const UimLpaSrvAddrOp lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const UimLpaSrvAddrOp e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const UimLpaSrvAddrOp e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
std::string toString(uint32_t o);
template<>
std::string toString<UimLpaSrvAddrOp>(uint32_t o);

std::string toString(UimLpaSrvAddrOp o);

constexpr uint32_t operator|(const UimLpaProfileState lhs, const UimLpaProfileState rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const UimLpaProfileState rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const UimLpaProfileState lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const UimLpaProfileState lhs, const UimLpaProfileState rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const UimLpaProfileState rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const UimLpaProfileState lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const UimLpaProfileState e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const UimLpaProfileState e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
std::string toString(uint32_t o);
template<>
std::string toString<UimLpaProfileState>(uint32_t o);

std::string toString(UimLpaProfileState o);

constexpr uint32_t operator|(const UimLpaIconType lhs, const UimLpaIconType rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const UimLpaIconType rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const UimLpaIconType lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const UimLpaIconType lhs, const UimLpaIconType rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const UimLpaIconType rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const UimLpaIconType lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const UimLpaIconType e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const UimLpaIconType e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
std::string toString(uint32_t o);
template<>
std::string toString<UimLpaIconType>(uint32_t o);

std::string toString(UimLpaIconType o);

constexpr uint32_t operator|(const UimLpaAddProfileStatus lhs, const UimLpaAddProfileStatus rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const UimLpaAddProfileStatus rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const UimLpaAddProfileStatus lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const UimLpaAddProfileStatus lhs, const UimLpaAddProfileStatus rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const UimLpaAddProfileStatus rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const UimLpaAddProfileStatus lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const UimLpaAddProfileStatus e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const UimLpaAddProfileStatus e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
std::string toString(uint32_t o);
template<>
std::string toString<UimLpaAddProfileStatus>(uint32_t o);

std::string toString(UimLpaAddProfileStatus o);

constexpr uint32_t operator|(const UimLpaAddProfileFailureCause lhs, const UimLpaAddProfileFailureCause rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const UimLpaAddProfileFailureCause rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const UimLpaAddProfileFailureCause lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const UimLpaAddProfileFailureCause lhs, const UimLpaAddProfileFailureCause rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const UimLpaAddProfileFailureCause rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const UimLpaAddProfileFailureCause lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const UimLpaAddProfileFailureCause e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const UimLpaAddProfileFailureCause e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
std::string toString(uint32_t o);
template<>
std::string toString<UimLpaAddProfileFailureCause>(uint32_t o);

std::string toString(UimLpaAddProfileFailureCause o);

std::string toString(const UimLpaSrvAddrOpReq&);

bool operator==(const UimLpaSrvAddrOpReq&, const UimLpaSrvAddrOpReq&);

bool operator!=(const UimLpaSrvAddrOpReq&, const UimLpaSrvAddrOpReq&);

std::string toString(const UimLpaUserReq&);

bool operator==(const UimLpaUserReq&, const UimLpaUserReq&);

bool operator!=(const UimLpaUserReq&, const UimLpaUserReq&);

std::string toString(const UimLpaProfileInfo&);

bool operator==(const UimLpaProfileInfo&, const UimLpaProfileInfo&);

bool operator!=(const UimLpaProfileInfo&, const UimLpaProfileInfo&);

std::string toString(const UimLpaSrvAddrOpResp&);

bool operator==(const UimLpaSrvAddrOpResp&, const UimLpaSrvAddrOpResp&);

bool operator!=(const UimLpaSrvAddrOpResp&, const UimLpaSrvAddrOpResp&);

std::string toString(const UimLpaUserResp&);

bool operator==(const UimLpaUserResp&, const UimLpaUserResp&);

bool operator!=(const UimLpaUserResp&, const UimLpaUserResp&);

std::string toString(const UimLpaAddProfileProgressInd&);

bool operator==(const UimLpaAddProfileProgressInd&, const UimLpaAddProfileProgressInd&);

bool operator!=(const UimLpaAddProfileProgressInd&, const UimLpaAddProfileProgressInd&);

std::string toString(const UimLpaHttpCustomHeader&);

bool operator==(const UimLpaHttpCustomHeader&, const UimLpaHttpCustomHeader&);

bool operator!=(const UimLpaHttpCustomHeader&, const UimLpaHttpCustomHeader&);

std::string toString(const UimLpaHttpTransactionInd&);

bool operator==(const UimLpaHttpTransactionInd&, const UimLpaHttpTransactionInd&);

bool operator!=(const UimLpaHttpTransactionInd&, const UimLpaHttpTransactionInd&);


}  // namespace V1_0
}  // namespace lpa
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_LPA_V1_0_TYPES_H
