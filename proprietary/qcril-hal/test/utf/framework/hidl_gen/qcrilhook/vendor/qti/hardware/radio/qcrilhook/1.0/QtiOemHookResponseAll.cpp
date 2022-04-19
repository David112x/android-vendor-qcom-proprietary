/******************************************************************************
  @file framework/hidl_gen/qcrilhook/vendor/qti/hardware/radio/qcrilhook/1.0/QtiOemHookResponseAll.cpp
  DESCRIPTION
  ---------------------------------------------------------------------------
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/
#define LOG_TAG "vendor.qti.hardware.radio.qcrilhook@1.0::QtiOemHookResponse"

#include <vendor/qti/hardware/radio/qcrilhook/1.0/IQtiOemHookResponse.h>
#include <android/log.h>
#include <cutils/trace.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qcrilhook {
namespace V1_0 {

std::string toString(const ::android::sp<IQtiOemHookResponse>& o) {
    std::string os = "[class or subclass of ";
    os += IQtiOemHookResponse::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* IQtiOemHookResponse::descriptor("vendor.qti.hardware.radio.qcrilhook@1.0::IQtiOemHookResponse");

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IQtiOemHookResponse::interfaceChain(interfaceChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQtiOemHookResponse::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQtiOemHookResponse::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQtiOemHookResponse::getHashChain(getHashChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IQtiOemHookResponse::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IQtiOemHookResponse::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IQtiOemHookResponse::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IQtiOemHookResponse::getDebugInfo(getDebugInfo_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQtiOemHookResponse::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IQtiOemHookResponse::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<IQtiOemHookResponse>> IQtiOemHookResponse::castFrom(const ::android::sp<IQtiOemHookResponse>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<IQtiOemHookResponse>> IQtiOemHookResponse::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return nullptr;
}

// static
::android::sp<IQtiOemHookResponse> IQtiOemHookResponse::tryGetService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

// static
::android::sp<IQtiOemHookResponse> IQtiOemHookResponse::getService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::status_t IQtiOemHookResponse::registerAsService(const std::string &serviceName) {
    return ::android::OK;
}

bool IQtiOemHookResponse::registerForNotifications(
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
