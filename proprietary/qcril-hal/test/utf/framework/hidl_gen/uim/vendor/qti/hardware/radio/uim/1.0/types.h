/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file framework/hidl_gen/uim/vendor/qti/hardware/radio/uim/1.0/types.h
  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_UIM_V1_0_TYPES_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_UIM_V1_0_TYPES_H

#include <android/hidl/base/1.0/IBase.h>

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace uim {
namespace V1_0 {

enum class UimRemoteSimlockOperationType : uint32_t {
    UIM_REMOTE_SIMLOCK_GENERATE_ENCRYPTED_KEY = 0u, // 0
    UIM_REMOTE_SIMLOCK_PROCESS_SIMLOCK_DATA = 1u, // 1
    UIM_REMOTE_SIMLOCK_GENERATE_HMAC = 2u, // 2
    UIM_REMOTE_SIMLOCK_GET_MAX_SUPPORTED_VERSION = 3u, // 3
    UIM_REMOTE_SIMLOCK_GET_STATUS = 4u, // 4
};

enum class UimRemoteSimlockResponseType : uint32_t {
    UIM_REMOTE_SIMLOCK_RESP_SUCCESS = 0u, // 0
    UIM_REMOTE_SIMLOCK_RESP_FAILURE = 1u, // 1
    UIM_REMOTE_SIMLOCK_RESP_GET_TIME_FAILED = 2u, // 2
};

enum class UimRemoteSimlockStatusType : uint32_t {
    UIM_REMOTE_SIMLOCK_STATE_LOCKED = 0u, // 0
    UIM_REMOTE_SIMLOCK_STATE_TEMPERORY_UNLOCK = 1u, // 1
    UIM_REMOTE_SIMLOCK_STATE_PERMANENT_UNLOCK = 2u, // 2
};

struct UimRemoteSimlockVersion final {
    uint8_t majorVersion __attribute__ ((aligned(1)));
    uint8_t minorVersion __attribute__ ((aligned(1)));
};

static_assert(offsetof(UimRemoteSimlockVersion, majorVersion) == 0, "wrong offset");
static_assert(offsetof(UimRemoteSimlockVersion, minorVersion) == 1, "wrong offset");
static_assert(sizeof(UimRemoteSimlockVersion) == 2, "wrong size");
static_assert(__alignof(UimRemoteSimlockVersion) == 1, "wrong alignment");

struct UimRemoteSimlockStatus final {
    UimRemoteSimlockStatusType status __attribute__ ((aligned(4)));
    uint32_t unlockTime __attribute__ ((aligned(4)));
};

static_assert(offsetof(UimRemoteSimlockStatus, status) == 0, "wrong offset");
static_assert(offsetof(UimRemoteSimlockStatus, unlockTime) == 4, "wrong offset");
static_assert(sizeof(UimRemoteSimlockStatus) == 8, "wrong size");
static_assert(__alignof(UimRemoteSimlockStatus) == 4, "wrong alignment");

constexpr uint32_t operator|(const UimRemoteSimlockOperationType lhs, const UimRemoteSimlockOperationType rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const UimRemoteSimlockOperationType rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const UimRemoteSimlockOperationType lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const UimRemoteSimlockOperationType lhs, const UimRemoteSimlockOperationType rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const UimRemoteSimlockOperationType rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const UimRemoteSimlockOperationType lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const UimRemoteSimlockOperationType e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const UimRemoteSimlockOperationType e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
std::string toString(uint32_t o);
template<>
std::string toString<UimRemoteSimlockOperationType>(uint32_t o);

std::string toString(UimRemoteSimlockOperationType o);

constexpr uint32_t operator|(const UimRemoteSimlockResponseType lhs, const UimRemoteSimlockResponseType rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const UimRemoteSimlockResponseType rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const UimRemoteSimlockResponseType lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const UimRemoteSimlockResponseType lhs, const UimRemoteSimlockResponseType rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const UimRemoteSimlockResponseType rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const UimRemoteSimlockResponseType lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const UimRemoteSimlockResponseType e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const UimRemoteSimlockResponseType e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
std::string toString(uint32_t o);
template<>
std::string toString<UimRemoteSimlockResponseType>(uint32_t o);

std::string toString(UimRemoteSimlockResponseType o);

constexpr uint32_t operator|(const UimRemoteSimlockStatusType lhs, const UimRemoteSimlockStatusType rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const UimRemoteSimlockStatusType rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const UimRemoteSimlockStatusType lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const UimRemoteSimlockStatusType lhs, const UimRemoteSimlockStatusType rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const UimRemoteSimlockStatusType rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const UimRemoteSimlockStatusType lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const UimRemoteSimlockStatusType e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const UimRemoteSimlockStatusType e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
std::string toString(uint32_t o);
template<>
std::string toString<UimRemoteSimlockStatusType>(uint32_t o);

std::string toString(UimRemoteSimlockStatusType o);

std::string toString(const UimRemoteSimlockVersion&);

bool operator==(const UimRemoteSimlockVersion&, const UimRemoteSimlockVersion&);

bool operator!=(const UimRemoteSimlockVersion&, const UimRemoteSimlockVersion&);

std::string toString(const UimRemoteSimlockStatus&);

bool operator==(const UimRemoteSimlockStatus&, const UimRemoteSimlockStatus&);

bool operator!=(const UimRemoteSimlockStatus&, const UimRemoteSimlockStatus&);


}  // namespace V1_0
}  // namespace uim
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_UIM_V1_0_TYPES_H
