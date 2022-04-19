/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file framework/hidl_gen/uim_remote_server/vendor/qti/hardware/radio/uim_remote_server/1.0/types.h
  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_UIM_REMOTE_SERVER_V1_0_TYPES_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_UIM_REMOTE_SERVER_V1_0_TYPES_H

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace uim_remote_server {
namespace V1_0 {

enum class UimRemoteServiceServerConnectRsp : uint32_t {
    UIM_REMOTE_SERVICE_SERVER_SUCCESS = 0u, // 0
    UIM_REMOTE_SERVICE_SERVER_CONNECT_FAILURE = 1u, // 1
    UIM_REMOTE_SERVICE_SERVER_MSG_SIZE_TOO_LARGE = 2u, // 2
    UIM_REMOTE_SERVICE_SERVER_MSG_SIZE_TOO_SMALL = 3u, // 3
    UIM_REMOTE_SERVICE_SERVER_CONNECT_OK_CALL_ONGOING = 4u, // 4
};

enum class UimRemoteServiceServerDisconnectType : uint32_t {
    UIM_REMOTE_SERVICE_SERVER_DISCONNECT_GRACEFUL = 0u, // 0
    UIM_REMOTE_SERVICE_SERVER_DISCONNECT_IMMEDIATE = 1u, // 1
};

enum class UimRemoteServiceServerApduType : uint32_t {
    UIM_REMOTE_SERVICE_SERVER_APDU = 0u, // 0
    UIM_REMOTE_SERVICE_SERVER_APDU7816 = 1u, // 1
};

enum class UimRemoteServiceServerResultCode : uint32_t {
    UIM_REMOTE_SERVICE_SERVER_SUCCESS = 0u, // 0
    UIM_REMOTE_SERVICE_SERVER_GENERIC_FAILURE = 1u, // 1
    UIM_REMOTE_SERVICE_SERVER_CARD_NOT_ACCESSSIBLE = 2u, // 2
    UIM_REMOTE_SERVICE_SERVER_CARD_ALREADY_POWERED_OFF = 3u, // 3
    UIM_REMOTE_SERVICE_SERVER_CARD_REMOVED = 4u, // 4
    UIM_REMOTE_SERVICE_SERVER_CARD_ALREADY_POWERED_ON = 5u, // 5
    UIM_REMOTE_SERVICE_SERVER_DATA_NOT_AVAILABLE = 6u, // 6
    UIM_REMOTE_SERVICE_SERVER_NOT_SUPPORTED = 7u, // 7
};

enum class UimRemoteServiceServerStatus : uint32_t {
    UIM_REMOTE_SERVICE_SERVER_UNKNOWN_ERROR = 0u, // 0
    UIM_REMOTE_SERVICE_SERVER_CARD_RESET = 1u, // 1
    UIM_REMOTE_SERVICE_SERVER_CARD_NOT_ACCESSIBLE = 2u, // 2
    UIM_REMOTE_SERVICE_SERVER_CARD_REMOVED = 3u, // 3
    UIM_REMOTE_SERVICE_SERVER_CARD_INSERTED = 4u, // 4
    UIM_REMOTE_SERVICE_SERVER_RECOVERED = 5u, // 5
};

enum class UimRemoteServiceServerTransferProtocol : uint32_t {
    UIM_REMOTE_SERVICE_SERVER_TP_T0 = 0u, // 0
    UIM_REMOTE_SERVICE_SERVER_TP_T1 = 1u, // 1
};

constexpr uint32_t operator|(const UimRemoteServiceServerConnectRsp lhs, const UimRemoteServiceServerConnectRsp rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const UimRemoteServiceServerConnectRsp rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const UimRemoteServiceServerConnectRsp lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const UimRemoteServiceServerConnectRsp lhs, const UimRemoteServiceServerConnectRsp rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const UimRemoteServiceServerConnectRsp rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const UimRemoteServiceServerConnectRsp lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const UimRemoteServiceServerConnectRsp e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const UimRemoteServiceServerConnectRsp e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
std::string toString(uint32_t o);
template<>
std::string toString<UimRemoteServiceServerConnectRsp>(uint32_t o);

std::string toString(UimRemoteServiceServerConnectRsp o);

constexpr uint32_t operator|(const UimRemoteServiceServerDisconnectType lhs, const UimRemoteServiceServerDisconnectType rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const UimRemoteServiceServerDisconnectType rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const UimRemoteServiceServerDisconnectType lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const UimRemoteServiceServerDisconnectType lhs, const UimRemoteServiceServerDisconnectType rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const UimRemoteServiceServerDisconnectType rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const UimRemoteServiceServerDisconnectType lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const UimRemoteServiceServerDisconnectType e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const UimRemoteServiceServerDisconnectType e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
std::string toString(uint32_t o);
template<>
std::string toString<UimRemoteServiceServerDisconnectType>(uint32_t o);

std::string toString(UimRemoteServiceServerDisconnectType o);

constexpr uint32_t operator|(const UimRemoteServiceServerApduType lhs, const UimRemoteServiceServerApduType rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const UimRemoteServiceServerApduType rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const UimRemoteServiceServerApduType lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const UimRemoteServiceServerApduType lhs, const UimRemoteServiceServerApduType rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const UimRemoteServiceServerApduType rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const UimRemoteServiceServerApduType lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const UimRemoteServiceServerApduType e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const UimRemoteServiceServerApduType e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
std::string toString(uint32_t o);
template<>
std::string toString<UimRemoteServiceServerApduType>(uint32_t o);

std::string toString(UimRemoteServiceServerApduType o);

constexpr uint32_t operator|(const UimRemoteServiceServerResultCode lhs, const UimRemoteServiceServerResultCode rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const UimRemoteServiceServerResultCode rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const UimRemoteServiceServerResultCode lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const UimRemoteServiceServerResultCode lhs, const UimRemoteServiceServerResultCode rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const UimRemoteServiceServerResultCode rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const UimRemoteServiceServerResultCode lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const UimRemoteServiceServerResultCode e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const UimRemoteServiceServerResultCode e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
std::string toString(uint32_t o);
template<>
std::string toString<UimRemoteServiceServerResultCode>(uint32_t o);

std::string toString(UimRemoteServiceServerResultCode o);

constexpr uint32_t operator|(const UimRemoteServiceServerStatus lhs, const UimRemoteServiceServerStatus rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const UimRemoteServiceServerStatus rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const UimRemoteServiceServerStatus lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const UimRemoteServiceServerStatus lhs, const UimRemoteServiceServerStatus rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const UimRemoteServiceServerStatus rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const UimRemoteServiceServerStatus lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const UimRemoteServiceServerStatus e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const UimRemoteServiceServerStatus e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
std::string toString(uint32_t o);
template<>
std::string toString<UimRemoteServiceServerStatus>(uint32_t o);

std::string toString(UimRemoteServiceServerStatus o);

constexpr uint32_t operator|(const UimRemoteServiceServerTransferProtocol lhs, const UimRemoteServiceServerTransferProtocol rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const uint32_t lhs, const UimRemoteServiceServerTransferProtocol rhs) {
    return static_cast<uint32_t>(lhs | static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator|(const UimRemoteServiceServerTransferProtocol lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) | rhs);
}

constexpr uint32_t operator&(const UimRemoteServiceServerTransferProtocol lhs, const UimRemoteServiceServerTransferProtocol rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const uint32_t lhs, const UimRemoteServiceServerTransferProtocol rhs) {
    return static_cast<uint32_t>(lhs & static_cast<uint32_t>(rhs));
}

constexpr uint32_t operator&(const UimRemoteServiceServerTransferProtocol lhs, const uint32_t rhs) {
    return static_cast<uint32_t>(static_cast<uint32_t>(lhs) & rhs);
}

constexpr uint32_t &operator|=(uint32_t& v, const UimRemoteServiceServerTransferProtocol e) {
    v |= static_cast<uint32_t>(e);
    return v;
}

constexpr uint32_t &operator&=(uint32_t& v, const UimRemoteServiceServerTransferProtocol e) {
    v &= static_cast<uint32_t>(e);
    return v;
}

template<typename>
std::string toString(uint32_t o);
template<>
std::string toString<UimRemoteServiceServerTransferProtocol>(uint32_t o);

std::string toString(UimRemoteServiceServerTransferProtocol o);


}  // namespace V1_0
}  // namespace uim_remote_server
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_UIM_REMOTE_SERVER_V1_0_TYPES_H
