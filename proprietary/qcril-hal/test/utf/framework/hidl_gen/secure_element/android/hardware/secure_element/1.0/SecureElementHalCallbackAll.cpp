/******************************************************************************
  @file framework/hidl_gen/secure_element/android/hardware/secure_element/1.0/SecureElementHalCallbackAll.cpp
  DESCRIPTION
  ---------------------------------------------------------------------------
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/
#define LOG_TAG "android.hardware.secure_element@1.0::SecureElementHalCallback"

#undef UNUSED

#include <android/hardware/secure_element/1.0/ISecureElementHalCallback.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"

namespace android {
namespace hardware {
namespace secure_element {
namespace V1_0 {

std::string toString(const ::android::sp<::android::hardware::secure_element::V1_0::ISecureElementHalCallback>& o) {
    std::string os = "[class or subclass of ";
    os += ::android::hardware::secure_element::V1_0::ISecureElementHalCallback::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* ISecureElementHalCallback::descriptor("android.hardware.secure_element@1.0::ISecureElementHalCallback");

// Methods from ::android::hardware::secure_element::V1_0::ISecureElementHalCallback follow.
// no default implementation for: ::android::hardware::Return<void> ISecureElementHalCallback::onStateChange(bool connected)

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> ISecureElementHalCallback::interfaceChain(interfaceChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> ISecureElementHalCallback::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> ISecureElementHalCallback::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> ISecureElementHalCallback::getHashChain(getHashChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();
}

::android::hardware::Return<void> ISecureElementHalCallback::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> ISecureElementHalCallback::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> ISecureElementHalCallback::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> ISecureElementHalCallback::getDebugInfo(getDebugInfo_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> ISecureElementHalCallback::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> ISecureElementHalCallback::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<::android::hardware::secure_element::V1_0::ISecureElementHalCallback>> ISecureElementHalCallback::castFrom(const ::android::sp<::android::hardware::secure_element::V1_0::ISecureElementHalCallback>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<::android::hardware::secure_element::V1_0::ISecureElementHalCallback>> ISecureElementHalCallback::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return nullptr;
}

// static
::android::sp<ISecureElementHalCallback> ISecureElementHalCallback::tryGetService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

// static
::android::sp<ISecureElementHalCallback> ISecureElementHalCallback::getService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::status_t ISecureElementHalCallback::registerAsService(const std::string &serviceName) {
    return ::android::OK;
}

bool ISecureElementHalCallback::registerForNotifications(
        const std::string &serviceName,
        const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification) {
    return true;
}

}  // namespace V1_0
}  // namespace secure_element
}  // namespace hardware
}  // namespace android
