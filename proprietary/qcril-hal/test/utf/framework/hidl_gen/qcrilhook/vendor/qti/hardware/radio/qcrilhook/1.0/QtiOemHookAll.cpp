/******************************************************************************
  @file framework/hidl_gen/qcrilhook/vendor/qti/hardware/radio/qcrilhook/1.0/QtiOemHookAll.cpp
  DESCRIPTION
  ---------------------------------------------------------------------------
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/
#define LOG_TAG "vendor.qti.hardware.radio.qcrilhook@1.0::QtiOemHook"

#include <vendor/qti/hardware/radio/qcrilhook/1.0/IQtiOemHook.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qcrilhook {
namespace V1_0 {

std::string toString(const ::android::sp<IQtiOemHook>& o) {
    std::string os = "[class or subclass of ";
    os += IQtiOemHook::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* IQtiOemHook::descriptor("vendor.qti.hardware.radio.qcrilhook@1.0::IQtiOemHook");

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IQtiOemHook::interfaceChain(interfaceChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQtiOemHook::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQtiOemHook::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQtiOemHook::getHashChain(getHashChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IQtiOemHook::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IQtiOemHook::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IQtiOemHook::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IQtiOemHook::getDebugInfo(getDebugInfo_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQtiOemHook::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IQtiOemHook::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<IQtiOemHook>> IQtiOemHook::castFrom(const ::android::sp<IQtiOemHook>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<IQtiOemHook>> IQtiOemHook::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return nullptr;
}

static std::unordered_map<std::string, ::android::sp<IQtiOemHook>> mHidlServices;
// static
::android::sp<IQtiOemHook> IQtiOemHook::tryGetService(const std::string &serviceName, const bool getStub) {
    return mHidlServices[serviceName];
}

// static
::android::sp<IQtiOemHook> IQtiOemHook::getService(const std::string &serviceName, const bool getStub) {
    return mHidlServices[serviceName];
}

::android::status_t IQtiOemHook::registerAsService(const std::string &serviceName) {
    mHidlServices[serviceName] = this;
    return ::android::OK;
}

bool IQtiOemHook::registerForNotifications(
        const std::string &serviceName,
        const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification) {
    return false;
}

}  // namespace V1_0
}  // namespace qcrilhook
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
