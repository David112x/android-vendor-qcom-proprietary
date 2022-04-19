/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file framework/hidl_gen/uim_remote_client/vendor/qti/hardware/radio/uim_remote_client/1.0/types.h
  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_UIM_REMOTE_CLIENT_V1_0_TYPES_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_UIM_REMOTE_CLIENT_V1_0_TYPES_H

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace uim_remote_client {
namespace V1_0 {

enum class UimRemoteClientEventStatusType : uint32_t {
    UIM_REMOTE_STATUS_CONN_UNAVAILABLE = 0u, // 0
    UIM_REMOTE_STATUS_CONN_AVAILABLE = 1u, // 1
    UIM_REMOTE_STATUS_CARD_INSERTED = 2u, // 2
    UIM_REMOTE_STATUS_CARD_REMOVED = 3u, // 3
    UIM_REMOTE_STATUS_CARD_ERROR = 4u, // 4
    UIM_REMOTE_STATUS_CARD_RESET = 5u, // 5
    UIM_REMOTE_STATUS_CARD_WAKEUP = 6u, // 6
};

enum class UimRemoteClientRsp : uint32_t {
    UIM_REMOTE_ERR_SUCCESS = 0u, // 0
    UIM_REMOTE_ERR_GENERIC_FAILURE = 1u, // 1
    UIM_REMOTE_ERR_NOT_SUPPORTED = 2u, // 2
    UIM_REMOTE_ERR_INVALID_PARAMETER = 3u, // 3
};

enum class UimRemoteClientErrorCauseType : uint32_t {
    UIM_REMOTE_CARD_ERROR_UNKNOWN = 0u, // 0
    UIM_REMOTE_CARD_ERROR_NO_LINK_EST = 1u, // 1
    UIM_REMOTE_CARD_ERROR_CMD_TIMEOUT = 2u, // 2
    UIM_REMOTE_CARD_ERROR_POWER_DOWN = 3u, // 3
    UIM_REMOTE_CARD_ERROR_POWER_DOWN_TELECOM = 4u, // 4
};

enum class UimRemoteClientTransportType : uint32_t {
    UIM_REMOTE_TRANSPORT_OTHER = 0u, // 0
    UIM_REMOTE_TRANSPORT_BLUETOOTH = 1u, // 1
    UIM_REMOTE_TRANSPORT_IP = 2u, // 2
};

enum class UimRemoteClientUsageType : uint32_t {
    UIM_REMOTE_USAGE_REDUCED = 0u, // 0
    UIM_REMOTE_USAGE_NORMAL = 1u, // 1
};

enum class UimRemoteClientEventRsp : uint32_t {
    UIM_REMOTE_SUCCESS = 0u, // 0
    UIM_REMOTE_FAILURE = 1u, // 1
};

enum class UimRemoteClientApduStatus : uint32_t {
    UIM_REMOTE_SUCCESS = 0u, // 0
    UIM_REMOTE_FAILURE = 1u, // 1
};

enum class UimRemoteClientApduRsp : uint32_t {
    UIM_REMOTE_SUCCESS = 0u, // 0
    UIM_REMOTE_FAILURE = 1u, // 1
};

enum class UimRemoteClientVoltageClass : uint32_t {
    UIM_REMOTE_VOLTAGE_CLASS_C_LOW = 0u, // 0
    UIM_REMOTE_VOLTAGE_CLASS_C = 1u, // 1
    UIM_REMOTE_VOLTAGE_CLASS_C_HIGH = 2u, // 2
    UIM_REMOTE_VOLTAGE_CLASS_B_LOW = 3u, // 3
    UIM_REMOTE_VOLTAGE_CLASS_B = 4u, // 4
    UIM_REMOTE_VOLTAGE_CLASS_B_HIGH = 5u, // 5
};

enum class UimRemoteClientPowerDownMode : uint32_t {
    UIM_REMOTE_PDOWN_TELECOM_INTERFACE = 0u, // 0
    UIM_REMOTE_PDOWN_CARD = 1u, // 1
};

struct UimRemoteEventReqType final {
    UimRemoteClientEventStatusType event __attribute__ ((aligned(4)));
    ::android::hardware::hidl_vec<uint8_t> atr __attribute__ ((aligned(8)));
    bool has_wakeupSupport __attribute__ ((aligned(1)));
    bool wakeupSupport __attribute__ ((aligned(1)));
    bool has_errorCode __attribute__ ((aligned(1)));
    UimRemoteClientErrorCauseType errorCode __attribute__ ((aligned(4)));
    bool has_transport __attribute__ ((aligned(1)));
    UimRemoteClientTransportType transport __attribute__ ((aligned(4)));
    bool has_usage __attribute__ ((aligned(1)));
    UimRemoteClientUsageType usage __attribute__ ((aligned(4)));
    bool has_apdu_timeout __attribute__ ((aligned(1)));
    uint32_t apduTimeout __attribute__ ((aligned(4)));
    bool has_disable_all_polling __attribute__ ((aligned(1)));
    uint32_t disableAllPolling __attribute__ ((aligned(4)));
    bool has_poll_timer __attribute__ ((aligned(1)));
    uint32_t pollTimer __attribute__ ((aligned(4)));
};

static_assert(offsetof(UimRemoteEventReqType, event) == 0, "wrong offset");
static_assert(offsetof(UimRemoteEventReqType, atr) == 8, "wrong offset");
static_assert(offsetof(UimRemoteEventReqType, has_wakeupSupport) == 24, "wrong offset");
static_assert(offsetof(UimRemoteEventReqType, wakeupSupport) == 25, "wrong offset");
static_assert(offsetof(UimRemoteEventReqType, has_errorCode) == 26, "wrong offset");
static_assert(offsetof(UimRemoteEventReqType, errorCode) == 28, "wrong offset");
static_assert(offsetof(UimRemoteEventReqType, has_transport) == 32, "wrong offset");
static_assert(offsetof(UimRemoteEventReqType, transport) == 36, "wrong offset");
static_assert(offsetof(UimRemoteEventReqType, has_usage) == 40, "wrong offset");
static_assert(offsetof(UimRemoteEventReqType, usage) == 44, "wrong offset");
static_assert(offsetof(UimRemoteEventReqType, has_apdu_timeout) == 48, "wrong offset");
static_assert(offsetof(UimRemoteEventReqType, apduTimeout) == 52, "wrong offset");
static_assert(offsetof(UimRemoteEventReqType, has_disable_all_polling) == 56, "wrong offset");
static_assert(offsetof(UimRemoteEventReqType, disableAllPolling) == 60, "wrong offset");
static_assert(offsetof(UimRemoteEventReqType, has_poll_timer) == 64, "wrong offset");
static_assert(offsetof(UimRemoteEventReqType, pollTimer) == 68, "wrong offset");
static_assert(sizeof(UimRemoteEventReqType) == 72, "wrong size");
static_assert(__alignof(UimRemoteEventReqType) == 8, "wrong alignment");

constexpr uint32_t operator|(const UimRemoteClientEventStatusType lhs, const UimRemoteClientEventStatusType rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const UimRemoteClientEventStatusType rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const UimRemoteClientEventStatusType lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const UimRemoteClientEventStatusType lhs, const UimRemoteClientEventStatusType rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const UimRemoteClientEventStatusType rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const UimRemoteClientEventStatusType lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const UimRemoteClientEventStatusType e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const UimRemoteClientEventStatusType e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
std::string toString(uint32_t o);
template<>
std::string toString<UimRemoteClientEventStatusType>(uint32_t o);

std::string toString(UimRemoteClientEventStatusType o);

constexpr uint32_t operator|(const UimRemoteClientRsp lhs, const UimRemoteClientRsp rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const UimRemoteClientRsp rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const UimRemoteClientRsp lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const UimRemoteClientRsp lhs, const UimRemoteClientRsp rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const UimRemoteClientRsp rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const UimRemoteClientRsp lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const UimRemoteClientRsp e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const UimRemoteClientRsp e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
std::string toString(uint32_t o);
template<>
std::string toString<UimRemoteClientRsp>(uint32_t o);

std::string toString(UimRemoteClientRsp o);

constexpr uint32_t operator|(const UimRemoteClientErrorCauseType lhs, const UimRemoteClientErrorCauseType rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const UimRemoteClientErrorCauseType rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const UimRemoteClientErrorCauseType lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const UimRemoteClientErrorCauseType lhs, const UimRemoteClientErrorCauseType rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const UimRemoteClientErrorCauseType rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const UimRemoteClientErrorCauseType lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const UimRemoteClientErrorCauseType e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const UimRemoteClientErrorCauseType e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
std::string toString(uint32_t o);
template<>
std::string toString<UimRemoteClientErrorCauseType>(uint32_t o);

std::string toString(UimRemoteClientErrorCauseType o);

constexpr uint32_t operator|(const UimRemoteClientTransportType lhs, const UimRemoteClientTransportType rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const UimRemoteClientTransportType rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const UimRemoteClientTransportType lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const UimRemoteClientTransportType lhs, const UimRemoteClientTransportType rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const UimRemoteClientTransportType rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const UimRemoteClientTransportType lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const UimRemoteClientTransportType e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const UimRemoteClientTransportType e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
std::string toString(uint32_t o);
template<>
std::string toString<UimRemoteClientTransportType>(uint32_t o);

std::string toString(UimRemoteClientTransportType o);

constexpr uint32_t operator|(const UimRemoteClientUsageType lhs, const UimRemoteClientUsageType rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const UimRemoteClientUsageType rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const UimRemoteClientUsageType lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const UimRemoteClientUsageType lhs, const UimRemoteClientUsageType rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const UimRemoteClientUsageType rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const UimRemoteClientUsageType lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const UimRemoteClientUsageType e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const UimRemoteClientUsageType e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
std::string toString(uint32_t o);
template<>
std::string toString<UimRemoteClientUsageType>(uint32_t o);

std::string toString(UimRemoteClientUsageType o);

constexpr uint32_t operator|(const UimRemoteClientEventRsp lhs, const UimRemoteClientEventRsp rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const UimRemoteClientEventRsp rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const UimRemoteClientEventRsp lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const UimRemoteClientEventRsp lhs, const UimRemoteClientEventRsp rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const UimRemoteClientEventRsp rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const UimRemoteClientEventRsp lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const UimRemoteClientEventRsp e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const UimRemoteClientEventRsp e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
std::string toString(uint32_t o);
template<>
std::string toString<UimRemoteClientEventRsp>(uint32_t o);

std::string toString(UimRemoteClientEventRsp o);

constexpr uint32_t operator|(const UimRemoteClientApduStatus lhs, const UimRemoteClientApduStatus rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const UimRemoteClientApduStatus rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const UimRemoteClientApduStatus lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const UimRemoteClientApduStatus lhs, const UimRemoteClientApduStatus rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const UimRemoteClientApduStatus rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const UimRemoteClientApduStatus lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const UimRemoteClientApduStatus e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const UimRemoteClientApduStatus e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
std::string toString(uint32_t o);
template<>
std::string toString<UimRemoteClientApduStatus>(uint32_t o);

std::string toString(UimRemoteClientApduStatus o);

constexpr uint32_t operator|(const UimRemoteClientApduRsp lhs, const UimRemoteClientApduRsp rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const UimRemoteClientApduRsp rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const UimRemoteClientApduRsp lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const UimRemoteClientApduRsp lhs, const UimRemoteClientApduRsp rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const UimRemoteClientApduRsp rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const UimRemoteClientApduRsp lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const UimRemoteClientApduRsp e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const UimRemoteClientApduRsp e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
std::string toString(uint32_t o);
template<>
std::string toString<UimRemoteClientApduRsp>(uint32_t o);

std::string toString(UimRemoteClientApduRsp o);

constexpr uint32_t operator|(const UimRemoteClientVoltageClass lhs, const UimRemoteClientVoltageClass rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const UimRemoteClientVoltageClass rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const UimRemoteClientVoltageClass lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const UimRemoteClientVoltageClass lhs, const UimRemoteClientVoltageClass rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const UimRemoteClientVoltageClass rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const UimRemoteClientVoltageClass lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const UimRemoteClientVoltageClass e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const UimRemoteClientVoltageClass e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
std::string toString(uint32_t o);
template<>
std::string toString<UimRemoteClientVoltageClass>(uint32_t o);

std::string toString(UimRemoteClientVoltageClass o);

constexpr uint32_t operator|(const UimRemoteClientPowerDownMode lhs, const UimRemoteClientPowerDownMode rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const UimRemoteClientPowerDownMode rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const UimRemoteClientPowerDownMode lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const UimRemoteClientPowerDownMode lhs, const UimRemoteClientPowerDownMode rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const UimRemoteClientPowerDownMode rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const UimRemoteClientPowerDownMode lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const UimRemoteClientPowerDownMode e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const UimRemoteClientPowerDownMode e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
std::string toString(uint32_t o);
template<>
std::string toString<UimRemoteClientPowerDownMode>(uint32_t o);

std::string toString(UimRemoteClientPowerDownMode o);

std::string toString(const UimRemoteEventReqType&);

bool operator==(const UimRemoteEventReqType&, const UimRemoteEventReqType&);

bool operator!=(const UimRemoteEventReqType&, const UimRemoteEventReqType&);


}  // namespace V1_0
}  // namespace uim_remote_client
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_UIM_REMOTE_CLIENT_V1_0_TYPES_H
