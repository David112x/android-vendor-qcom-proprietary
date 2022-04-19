/******************************************************************************
  @file framework/hidl_gen/qcrilhook/vendor/qti/hardware/radio/qcrilhook/1.0/QtiOemHookIndicationAll.cpp
  DESCRIPTION
  ---------------------------------------------------------------------------
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/
#define LOG_TAG "vendor.qti.hardware.radio.qcrilhook@1.0::QtiOemHookIndication"

#include <vendor/qti/hardware/radio/qcrilhook/1.0/IQtiOemHookIndication.h>
#include <android/log.h>
#include <cutils/trace.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qcrilhook {
namespace V1_0 {

std::string toString(const ::android::sp<IQtiOemHookIndication>& o) {
    std::string os = "[class or subclass of ";
    os += IQtiOemHookIndication::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* IQtiOemHookIndication::descriptor("vendor.qti.hardware.radio.qcrilhook@1.0::IQtiOemHookIndication");

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IQtiOemHookIndication::interfaceChain(interfaceChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQtiOemHookIndication::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQtiOemHookIndication::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQtiOemHookIndication::getHashChain(getHashChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IQtiOemHookIndication::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IQtiOemHookIndication::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IQtiOemHookIndication::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IQtiOemHookIndication::getDebugInfo(getDebugInfo_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQtiOemHookIndication::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IQtiOemHookIndication::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<IQtiOemHookIndication>> IQtiOemHookIndication::castFrom(const ::android::sp<IQtiOemHookIndication>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<IQtiOemHookIndication>> IQtiOemHookIndication::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return nullptr;
}

// static
::android::sp<IQtiOemHookIndication> IQtiOemHookIndication::tryGetService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

// static
::android::sp<IQtiOemHookIndication> IQtiOemHookIndication::getService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::status_t IQtiOemHookIndication::registerAsService(const std::string &serviceName) {
    return ::android::OK;
}

bool IQtiOemHookIndication::registerForNotifications(
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
