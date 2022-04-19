#ifndef HIDL_GENERATED_ANDROID_HIDL_SAFE_UNION_V1_0_TYPES_H
#define HIDL_GENERATED_ANDROID_HIDL_SAFE_UNION_V1_0_TYPES_H

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace android {
namespace hidl {
namespace safe_union {
namespace V1_0 {

// Forward declaration for forward reference support:
struct Monostate;

/**
 * Unit type easily available to be used with safe_union.
 * 
 * Example usage:
 * 
 * safe_union MaybeFoo {
 *     Monostate no_init;
 *     Foo foo;
 * };
 */
struct Monostate final {
};

static_assert(sizeof(::android::hidl::safe_union::V1_0::Monostate) == 1, "wrong size");
static_assert(__alignof(::android::hidl::safe_union::V1_0::Monostate) == 1, "wrong alignment");

//
// type declarations for package
//

static inline std::string toString(const ::android::hidl::safe_union::V1_0::Monostate&);
static inline bool operator==(const ::android::hidl::safe_union::V1_0::Monostate& lhs, const ::android::hidl::safe_union::V1_0::Monostate& rhs);
static inline bool operator!=(const ::android::hidl::safe_union::V1_0::Monostate& lhs, const ::android::hidl::safe_union::V1_0::Monostate& rhs);

//
// type header definitions for package
//

static inline std::string toString(const ::android::hidl::safe_union::V1_0::Monostate&) {
    using ::android::hardware::toString;
    std::string os;
    os += "{";
    os += "}"; return os;
}

static inline bool operator==(const ::android::hidl::safe_union::V1_0::Monostate& /* lhs */, const ::android::hidl::safe_union::V1_0::Monostate& /* rhs */) {
    return true;
}

static inline bool operator!=(const ::android::hidl::safe_union::V1_0::Monostate& lhs, const ::android::hidl::safe_union::V1_0::Monostate& rhs){
    return !(lhs == rhs);
}


}  // namespace V1_0
}  // namespace safe_union
}  // namespace hidl
}  // namespace android

//
// global type declarations for package
//


#endif  // HIDL_GENERATED_ANDROID_HIDL_SAFE_UNION_V1_0_TYPES_H
