/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file framework/hidl_gen/manager/android/hidl/manager/1.0/IServiceNotification.h
  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#ifndef HIDL_GENERATED_ANDROID_HIDL_MANAGER_V1_0_ISERVICENOTIFICATION_H
#define HIDL_GENERATED_ANDROID_HIDL_MANAGER_V1_0_ISERVICENOTIFICATION_H

#undef UNUSED

#include <android/hidl/base/1.0/IBase.h>

#include <android/hidl/manager/1.0/IServiceNotification.h>

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace android {
namespace hidl {
namespace manager {
namespace V1_0 {

struct IServiceNotification : public ::android::hidl::base::V1_0::IBase {
    virtual bool isRemote() const override { return false; }


#if 0
    virtual ::android::hardware::Return<void> onRegistration(const ::android::hardware::hidl_string& fqName, const ::android::hardware::hidl_string& name, bool preexisting) { return hardware::Void(); }

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
    static ::android::hardware::Return<::android::sp<IServiceNotification>> castFrom(const ::android::sp<IServiceNotification>& parent, bool emitError = false) { return nullptr; }
    static ::android::hardware::Return<::android::sp<IServiceNotification>> castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError = false) { return nullptr; }

    static const char* descriptor;

    static ::android::sp<IServiceNotification> tryGetService(const std::string &serviceName="default", bool getStub=false) { return nullptr; }
    static ::android::sp<IServiceNotification> tryGetService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return tryGetService(str, getStub); }
    static ::android::sp<IServiceNotification> tryGetService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return tryGetService(str, getStub); }
    static ::android::sp<IServiceNotification> tryGetService(bool getStub) { return tryGetService("default", getStub); }
    static ::android::sp<IServiceNotification> getService(const std::string &serviceName="default", bool getStub=false) { return nullptr; }
    static ::android::sp<IServiceNotification> getService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return getService(str, getStub); }
    static ::android::sp<IServiceNotification> getService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return getService(str, getStub); }
    static ::android::sp<IServiceNotification> getService(bool getStub) { return getService("default", getStub); }
    ::android::status_t registerAsService(const std::string &serviceName="default") { return android::OK; }
    static bool registerForNotifications(
            const std::string &serviceName,
            const ::android::sp<IServiceNotification> &notification) { return false; }
#endif
};

std::string toString(const ::android::sp<IServiceNotification>&);

}  // namespace V1_0
}  // namespace manager
}  // namespace hidl
}  // namespace android

#endif  // HIDL_GENERATED_ANDROID_HIDL_MANAGER_V1_0_ISERVICENOTIFICATION_H
