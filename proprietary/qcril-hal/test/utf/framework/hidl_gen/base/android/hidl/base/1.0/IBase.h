/******************************************************************************
  @file framework/hidl_gen/base/android/hidl/base/1.0/IBase.h
  DESCRIPTION
  ---------------------------------------------------------------------------
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/

#ifndef HIDL_GENERATED_ANDROID_HIDL_BASE_V1_0_IBASE_H
#define HIDL_GENERATED_ANDROID_HIDL_BASE_V1_0_IBASE_H

#undef UNUSED

#include <android/hidl/base/1.0/types.h>

// skipped #include IServiceNotification.h

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace android {
namespace hidl {
namespace base {
namespace V1_0 {

struct IBase : virtual public ::android::RefBase {
    virtual bool isRemote() const { return false; }

    using interfaceChain_cb = std::function<void(const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& descriptors)>;
    virtual ::android::hardware::Return<void> interfaceChain(interfaceChain_cb _hidl_cb) { return hardware::Void(); }

    virtual ::android::hardware::Return<void> debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options) { return hardware::Void(); }

    using interfaceDescriptor_cb = std::function<void(const ::android::hardware::hidl_string& descriptor)>;
    virtual ::android::hardware::Return<void> interfaceDescriptor(interfaceDescriptor_cb _hidl_cb) { return hardware::Void(); }

    using getHashChain_cb = std::function<void(const ::android::hardware::hidl_vec<::android::hardware::hidl_array<uint8_t, 32>>& hashchain)>;
    virtual ::android::hardware::Return<void> getHashChain(getHashChain_cb _hidl_cb) { return hardware::Void(); }

    virtual ::android::hardware::Return<void> setHALInstrumentation() { return hardware::Void(); }

    virtual ::android::hardware::Return<bool> linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie) { return false; }

    virtual ::android::hardware::Return<void> ping() { return hardware::Void(); }

    using getDebugInfo_cb = std::function<void(const DebugInfo& info)>;
    virtual ::android::hardware::Return<void> getDebugInfo(getDebugInfo_cb _hidl_cb) { return hardware::Void(); }

    virtual ::android::hardware::Return<void> notifySyspropsChanged() { return hardware::Void(); }

    virtual ::android::hardware::Return<bool> unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient) { return false; }
    // cast static functions
    static ::android::hardware::Return<::android::sp<IBase>> castFrom(const ::android::sp<IBase>& parent, bool emitError = false) { return nullptr; }

    static const char* descriptor;

    // skipped getService, registerAsService, registerForNotifications

};

std::string toString(const ::android::sp<IBase>&);

}  // namespace V1_0
}  // namespace base
}  // namespace hidl
}  // namespace android

#endif  // HIDL_GENERATED_ANDROID_HIDL_BASE_V1_0_IBASE_H
