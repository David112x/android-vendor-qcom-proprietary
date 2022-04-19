/******************************************************************************
  @file framework/hidl_gen/secure_element/android/hardware/secure_element/1.0/SecureElementAll.cpp
  DESCRIPTION
  ---------------------------------------------------------------------------
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/
#define LOG_TAG "android.hardware.secure_element@1.0::SecureElement"

#undef UNUSED

#include <android/hardware/secure_element/1.0/ISecureElement.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"

namespace android {
namespace hardware {
namespace secure_element {
namespace V1_0 {

std::string toString(const ::android::sp<::android::hardware::secure_element::V1_0::ISecureElement>& o) {
    std::string os = "[class or subclass of ";
    os += ::android::hardware::secure_element::V1_0::ISecureElement::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* ISecureElement::descriptor("android.hardware.secure_element@1.0::ISecureElement");

// Methods from ::android::hardware::secure_element::V1_0::ISecureElement follow.
// no default implementation for: ::android::hardware::Return<void> ISecureElement::init(const ::android::sp<::android::hardware::secure_element::V1_0::ISecureElementHalCallback>& clientCallback)
// no default implementation for: ::android::hardware::Return<void> ISecureElement::getAtr(getAtr_cb _hidl_cb)
// no default implementation for: ::android::hardware::Return<bool> ISecureElement::isCardPresent()
// no default implementation for: ::android::hardware::Return<void> ISecureElement::transmit(const ::android::hardware::hidl_vec<uint8_t>& data, transmit_cb _hidl_cb)
// no default implementation for: ::android::hardware::Return<void> ISecureElement::openLogicalChannel(const ::android::hardware::hidl_vec<uint8_t>& aid, uint8_t p2, openLogicalChannel_cb _hidl_cb)
// no default implementation for: ::android::hardware::Return<void> ISecureElement::openBasicChannel(const ::android::hardware::hidl_vec<uint8_t>& aid, uint8_t p2, openBasicChannel_cb _hidl_cb)
// no default implementation for: ::android::hardware::Return<::android::hardware::secure_element::V1_0::SecureElementStatus> ISecureElement::closeChannel(uint8_t channelNumber)

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> ISecureElement::interfaceChain(interfaceChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> ISecureElement::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> ISecureElement::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> ISecureElement::getHashChain(getHashChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();
}

::android::hardware::Return<void> ISecureElement::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> ISecureElement::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> ISecureElement::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> ISecureElement::getDebugInfo(getDebugInfo_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> ISecureElement::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> ISecureElement::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<::android::hardware::secure_element::V1_0::ISecureElement>> ISecureElement::castFrom(const ::android::sp<::android::hardware::secure_element::V1_0::ISecureElement>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<::android::hardware::secure_element::V1_0::ISecureElement>> ISecureElement::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return nullptr;
}

static std::unordered_map<std::string, ::android::sp<ISecureElement>> mHidlServices;
// static
::android::sp<ISecureElement> ISecureElement::tryGetService(const std::string &serviceName, const bool getStub) {
    return mHidlServices[serviceName];
}

// static
::android::sp<ISecureElement> ISecureElement::getService(const std::string &serviceName, const bool getStub) {
    return mHidlServices[serviceName];
}

::android::status_t ISecureElement::registerAsService(const std::string &serviceName) {
    mHidlServices[serviceName] = this;
    return ::android::OK;
}

bool ISecureElement::registerForNotifications(
        const std::string &serviceName,
        const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification) {
    return true;
}

}  // namespace V1_0
}  // namespace secure_element
}  // namespace hardware
}  // namespace android
