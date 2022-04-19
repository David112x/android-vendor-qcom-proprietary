/******************************************************************************
  @file framework/hidl_gen/secure_element/android/hardware/secure_element/1.0/ISecureElementHalCallback.h
  DESCRIPTION
  ---------------------------------------------------------------------------
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef HIDL_GENERATED_ANDROID_HARDWARE_SECURE_ELEMENT_V1_0_ISECUREELEMENTHALCALLBACK_H
#define HIDL_GENERATED_ANDROID_HARDWARE_SECURE_ELEMENT_V1_0_ISECUREELEMENTHALCALLBACK_H

#include <android/hidl/base/1.0/IBase.h>

#include <android/hidl/manager/1.0/IServiceNotification.h>

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace android {
namespace hardware {
namespace secure_element {
namespace V1_0 {

struct ISecureElementHalCallback : public ::android::hidl::base::V1_0::IBase {
    typedef android::hardware::details::i_tag _hidl_tag;

    // Forward declaration for forward reference support:

    virtual bool isRemote() const override { return false; }


    virtual ::android::hardware::Return<void> onStateChange(bool connected) = 0;

    using interfaceChain_cb = std::function<void(const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& descriptors)>;
    virtual ::android::hardware::Return<void> interfaceChain(interfaceChain_cb _hidl_cb) override;

    virtual ::android::hardware::Return<void> debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options) override;

    using interfaceDescriptor_cb = std::function<void(const ::android::hardware::hidl_string& descriptor)>;
    virtual ::android::hardware::Return<void> interfaceDescriptor(interfaceDescriptor_cb _hidl_cb) override;

    using getHashChain_cb = std::function<void(const ::android::hardware::hidl_vec<::android::hardware::hidl_array<uint8_t, 32>>& hashchain)>;
    virtual ::android::hardware::Return<void> getHashChain(getHashChain_cb _hidl_cb) override;

    virtual ::android::hardware::Return<void> setHALInstrumentation() override;

    virtual ::android::hardware::Return<bool> linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie) override;

    virtual ::android::hardware::Return<void> ping() override;

    using getDebugInfo_cb = std::function<void(const ::android::hidl::base::V1_0::DebugInfo& info)>;
    virtual ::android::hardware::Return<void> getDebugInfo(getDebugInfo_cb _hidl_cb) override;

    virtual ::android::hardware::Return<void> notifySyspropsChanged() override;

    virtual ::android::hardware::Return<bool> unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient) override;
    // cast static functions
    static ::android::hardware::Return<::android::sp<::android::hardware::secure_element::V1_0::ISecureElementHalCallback>> castFrom(const ::android::sp<::android::hardware::secure_element::V1_0::ISecureElementHalCallback>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<::android::hardware::secure_element::V1_0::ISecureElementHalCallback>> castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError = false);

    static const char* descriptor;

    static ::android::sp<ISecureElementHalCallback> tryGetService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<ISecureElementHalCallback> tryGetService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return tryGetService(str, getStub); }
    static ::android::sp<ISecureElementHalCallback> tryGetService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return tryGetService(str, getStub); }
    static ::android::sp<ISecureElementHalCallback> tryGetService(bool getStub) { return tryGetService("default", getStub); }
    static ::android::sp<ISecureElementHalCallback> getService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<ISecureElementHalCallback> getService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return getService(str, getStub); }
    static ::android::sp<ISecureElementHalCallback> getService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return getService(str, getStub); }
    static ::android::sp<ISecureElementHalCallback> getService(bool getStub) { return getService("default", getStub); }
    __attribute__ ((warn_unused_result))::android::status_t registerAsService(const std::string &serviceName="default");
    static bool registerForNotifications(
            const std::string &serviceName,
            const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification);
};

std::string toString(const ::android::sp<::android::hardware::secure_element::V1_0::ISecureElementHalCallback>&);

}  // namespace V1_0
}  // namespace secure_element
}  // namespace hardware
}  // namespace android

#endif  // HIDL_GENERATED_ANDROID_HARDWARE_SECURE_ELEMENT_V1_0_ISECUREELEMENTHALCALLBACK_H
