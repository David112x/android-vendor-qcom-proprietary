/******************************************************************************
  @file framework/hidl_gen/radio_config/android/hardware/radio/config/1.0/types.h
  DESCRIPTION
  ---------------------------------------------------------------------------
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef HIDL_GENERATED_ANDROID_HARDWARE_RADIO_CONFIG_V1_0_TYPES_H
#define HIDL_GENERATED_ANDROID_HARDWARE_RADIO_CONFIG_V1_0_TYPES_H

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace android {
namespace hardware {
namespace radio {
namespace config {
namespace V1_0 {

// Forward declaration for forward reference support:
enum class SlotState : int32_t;
struct SimSlotStatus;

enum class SlotState : int32_t {
    INACTIVE = 0,
    ACTIVE = 1,
};

struct SimSlotStatus final {
    ::android::hardware::radio::V1_0::CardState cardState __attribute__ ((aligned(4)));
    ::android::hardware::radio::config::V1_0::SlotState slotState __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string atr __attribute__ ((aligned(8)));
    uint32_t logicalSlotId __attribute__ ((aligned(4)));
    ::android::hardware::hidl_string iccid __attribute__ ((aligned(8)));
};

static_assert(offsetof(::android::hardware::radio::config::V1_0::SimSlotStatus, cardState) == 0, "wrong offset");
static_assert(offsetof(::android::hardware::radio::config::V1_0::SimSlotStatus, slotState) == 4, "wrong offset");
static_assert(offsetof(::android::hardware::radio::config::V1_0::SimSlotStatus, atr) == 8, "wrong offset");
static_assert(offsetof(::android::hardware::radio::config::V1_0::SimSlotStatus, logicalSlotId) == 24, "wrong offset");
static_assert(offsetof(::android::hardware::radio::config::V1_0::SimSlotStatus, iccid) == 32, "wrong offset");
static_assert(sizeof(::android::hardware::radio::config::V1_0::SimSlotStatus) == 48, "wrong size");
static_assert(__alignof(::android::hardware::radio::config::V1_0::SimSlotStatus) == 8, "wrong alignment");

constexpr int32_t operator|(const ::android::hardware::radio::config::V1_0::SlotState lhs, const ::android::hardware::radio::config::V1_0::SlotState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const ::android::hardware::radio::config::V1_0::SlotState rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const ::android::hardware::radio::config::V1_0::SlotState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const ::android::hardware::radio::config::V1_0::SlotState lhs, const ::android::hardware::radio::config::V1_0::SlotState rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const ::android::hardware::radio::config::V1_0::SlotState rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const ::android::hardware::radio::config::V1_0::SlotState lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const ::android::hardware::radio::config::V1_0::SlotState e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const ::android::hardware::radio::config::V1_0::SlotState e) {
    v &= static_cast<int32_t>(e);
    return v;
}

#ifdef REALLY_IS_HIDL_INTERNAL_LIB_ANDROID_HARDWARE_RADIO_CONFIG_V1_0
template<typename>
std::string toString(int32_t o);
template<>
std::string toString<::android::hardware::radio::config::V1_0::SlotState>(int32_t o);

std::string toString(::android::hardware::radio::config::V1_0::SlotState o);

#else
template<typename>
static inline std::string toString(int32_t o);
template<>
inline std::string toString<::android::hardware::radio::config::V1_0::SlotState>(int32_t o) {
    using ::android::hardware::details::toHexString;
    std::string os;
    ::android::hardware::hidl_bitfield<::android::hardware::radio::config::V1_0::SlotState> flipped = 0;
    bool first = true;
    if ((o & ::android::hardware::radio::config::V1_0::SlotState::INACTIVE) == static_cast<int32_t>(::android::hardware::radio::config::V1_0::SlotState::INACTIVE)) {
        os += (first ? "" : " | ");
        os += "INACTIVE";
        first = false;
        flipped |= ::android::hardware::radio::config::V1_0::SlotState::INACTIVE;
    }
    if ((o & ::android::hardware::radio::config::V1_0::SlotState::ACTIVE) == static_cast<int32_t>(::android::hardware::radio::config::V1_0::SlotState::ACTIVE)) {
        os += (first ? "" : " | ");
        os += "ACTIVE";
        first = false;
        flipped |= ::android::hardware::radio::config::V1_0::SlotState::ACTIVE;
    }
    if (o != flipped) {
        os += (first ? "" : " | ");
        os += toHexString(o & (~flipped));
    }os += " (";
    os += toHexString(o);
    os += ")";
    return os;
}

static inline std::string toString(::android::hardware::radio::config::V1_0::SlotState o) {
    using ::android::hardware::details::toHexString;
    if (o == ::android::hardware::radio::config::V1_0::SlotState::INACTIVE) {
        return "INACTIVE";
    }
    if (o == ::android::hardware::radio::config::V1_0::SlotState::ACTIVE) {
        return "ACTIVE";
    }
    std::string os;
    os += toHexString(static_cast<int32_t>(o));
    return os;
}

#endif  // REALLY_IS_HIDL_INTERNAL_LIB

std::string toString(const ::android::hardware::radio::config::V1_0::SimSlotStatus&);

bool operator==(const ::android::hardware::radio::config::V1_0::SimSlotStatus&, const ::android::hardware::radio::config::V1_0::SimSlotStatus&);

bool operator!=(const ::android::hardware::radio::config::V1_0::SimSlotStatus&, const ::android::hardware::radio::config::V1_0::SimSlotStatus&);

}  // namespace V1_0
}  // namespace config
}  // namespace radio
}  // namespace hardware
}  // namespace android
namespace android {
namespace hardware {
template<> struct hidl_enum_iterator<::android::hardware::radio::config::V1_0::SlotState>
{
    const ::android::hardware::radio::config::V1_0::SlotState* begin() { return static_begin(); }
    const ::android::hardware::radio::config::V1_0::SlotState* end() { return begin() + 2; }
    private:
    static const ::android::hardware::radio::config::V1_0::SlotState* static_begin() {
        static const ::android::hardware::radio::config::V1_0::SlotState kVals[2] {
            ::android::hardware::radio::config::V1_0::SlotState::INACTIVE,
            ::android::hardware::radio::config::V1_0::SlotState::ACTIVE,
        };
        return &kVals[0];
    }};

}  // namespace hardware
}  // namespace android

#endif  // HIDL_GENERATED_ANDROID_HARDWARE_RADIO_CONFIG_V1_0_TYPES_H
