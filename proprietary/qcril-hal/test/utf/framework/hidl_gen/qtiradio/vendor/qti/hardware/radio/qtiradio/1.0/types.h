/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V1_0_TYPES_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V1_0_TYPES_H

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace V1_0 {

enum class QtiRadioError : int32_t {
    NONE = 0,
    RADIO_NOT_AVAILABLE = 1,
    GENERIC_FAILURE = 2,
};

enum class QtiRadioResponseType : int32_t {
    SOLICITED = 0,
    SOLICITED_ACK = 1,
    SOLICITED_ACK_EXP = 2,
};

struct QtiRadioResponseInfo final {
    QtiRadioResponseType type __attribute__ ((aligned(4)));
    int32_t serial __attribute__ ((aligned(4)));
    QtiRadioError error __attribute__ ((aligned(4)));
};

static_assert(offsetof(QtiRadioResponseInfo, type) == 0, "wrong offset");
static_assert(offsetof(QtiRadioResponseInfo, serial) == 4, "wrong offset");
static_assert(offsetof(QtiRadioResponseInfo, error) == 8, "wrong offset");
static_assert(sizeof(QtiRadioResponseInfo) == 12, "wrong size");
static_assert(__alignof(QtiRadioResponseInfo) == 4, "wrong alignment");

enum class QtiRadioIndicationType : int32_t {
    UNSOLICITED = 0,
    UNSOLICITED_ACK_EXP = 1,
};

constexpr int32_t operator|(const QtiRadioError lhs, const QtiRadioError rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const QtiRadioError rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const QtiRadioError lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const QtiRadioError lhs, const QtiRadioError rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const QtiRadioError rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const QtiRadioError lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const QtiRadioError e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const QtiRadioError e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
std::string toString(int32_t o);
template<>
std::string toString<QtiRadioError>(int32_t o);

std::string toString(QtiRadioError o);

constexpr int32_t operator|(const QtiRadioResponseType lhs, const QtiRadioResponseType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const QtiRadioResponseType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const QtiRadioResponseType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const QtiRadioResponseType lhs, const QtiRadioResponseType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const QtiRadioResponseType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const QtiRadioResponseType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const QtiRadioResponseType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const QtiRadioResponseType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
std::string toString(int32_t o);
template<>
std::string toString<QtiRadioResponseType>(int32_t o);

std::string toString(QtiRadioResponseType o);

std::string toString(const QtiRadioResponseInfo&);

bool operator==(const QtiRadioResponseInfo&, const QtiRadioResponseInfo&);

bool operator!=(const QtiRadioResponseInfo&, const QtiRadioResponseInfo&);

constexpr int32_t operator|(const QtiRadioIndicationType lhs, const QtiRadioIndicationType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const QtiRadioIndicationType rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const QtiRadioIndicationType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const QtiRadioIndicationType lhs, const QtiRadioIndicationType rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const QtiRadioIndicationType rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const QtiRadioIndicationType lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const QtiRadioIndicationType e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const QtiRadioIndicationType e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
std::string toString(int32_t o);
template<>
std::string toString<QtiRadioIndicationType>(int32_t o);

std::string toString(QtiRadioIndicationType o);


}  // namespace V1_0
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V1_0_TYPES_H
