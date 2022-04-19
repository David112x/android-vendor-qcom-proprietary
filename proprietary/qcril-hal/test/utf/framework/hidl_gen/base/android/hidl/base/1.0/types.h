/******************************************************************************
  @file framework/hidl_gen/base/android/hidl/base/1.0/types.h
  DESCRIPTION
  ---------------------------------------------------------------------------
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef HIDL_GENERATED_ANDROID_HIDL_BASE_V1_0_TYPES_H
#define HIDL_GENERATED_ANDROID_HIDL_BASE_V1_0_TYPES_H

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace android {
namespace hidl {
namespace base {
namespace V1_0 {

struct DebugInfo final {
    enum class Architecture : int32_t {
        UNKNOWN = 0,
        IS_64BIT = 1,
        IS_32BIT = 2,
    };

    int32_t pid __attribute__ ((aligned(4)));
    uint64_t ptr __attribute__ ((aligned(8)));
    DebugInfo::Architecture arch __attribute__ ((aligned(4)));
};

static_assert(offsetof(DebugInfo, pid) == 0, "wrong offset");
static_assert(offsetof(DebugInfo, ptr) == 8, "wrong offset");
static_assert(offsetof(DebugInfo, arch) == 16, "wrong offset");
static_assert(sizeof(DebugInfo) == 24, "wrong size");
static_assert(__alignof(DebugInfo) == 8, "wrong alignment");

constexpr int32_t operator|(const DebugInfo::Architecture lhs, const DebugInfo::Architecture rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const int32_t lhs, const DebugInfo::Architecture rhs) {
    return static_cast<int32_t>(lhs | static_cast<int32_t>(rhs));
}

constexpr int32_t operator|(const DebugInfo::Architecture lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) | rhs);
}

constexpr int32_t operator&(const DebugInfo::Architecture lhs, const DebugInfo::Architecture rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const int32_t lhs, const DebugInfo::Architecture rhs) {
    return static_cast<int32_t>(lhs & static_cast<int32_t>(rhs));
}

constexpr int32_t operator&(const DebugInfo::Architecture lhs, const int32_t rhs) {
    return static_cast<int32_t>(static_cast<int32_t>(lhs) & rhs);
}

constexpr int32_t &operator|=(int32_t& v, const DebugInfo::Architecture e) {
    v |= static_cast<int32_t>(e);
    return v;
}

constexpr int32_t &operator&=(int32_t& v, const DebugInfo::Architecture e) {
    v &= static_cast<int32_t>(e);
    return v;
}

template<typename>
std::string toString(int32_t o);
template<>
std::string toString<DebugInfo::Architecture>(int32_t o);

std::string toString(DebugInfo::Architecture o);

std::string toString(const DebugInfo&);

bool operator==(const DebugInfo&, const DebugInfo&);

bool operator!=(const DebugInfo&, const DebugInfo&);


}  // namespace V1_0
}  // namespace base
}  // namespace hidl
}  // namespace android

#endif  // HIDL_GENERATED_ANDROID_HIDL_BASE_V1_0_TYPES_H
