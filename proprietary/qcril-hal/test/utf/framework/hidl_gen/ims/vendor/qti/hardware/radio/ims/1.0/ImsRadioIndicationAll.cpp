/******************************************************************************
  @file framework/hidl_gen/ims/vendor/qti/hardware/radio/ims/1.0/ImsRadioIndicationAll.cpp
  DESCRIPTION
  ---------------------------------------------------------------------------
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/
#define LOG_TAG "vendor.qti.hardware.radio.ims@1.0::ImsRadioIndication"

#undef UNUSED

#include <vendor/qti/hardware/radio/ims/1.0/IImsRadioIndication.h>
#include <android/log.h>
#include <cutils/trace.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_0 {

std::string toString(const ::android::sp<IImsRadioIndication>& o) {
    std::string os = "[class or subclass of ";
    os += IImsRadioIndication::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* IImsRadioIndication::descriptor("vendor.qti.hardware.radio.ims@1.0::IImsRadioIndication");

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IImsRadioIndication::interfaceChain(interfaceChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IImsRadioIndication::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IImsRadioIndication::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IImsRadioIndication::getHashChain(getHashChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IImsRadioIndication::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IImsRadioIndication::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IImsRadioIndication::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IImsRadioIndication::getDebugInfo(getDebugInfo_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IImsRadioIndication::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IImsRadioIndication::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<IImsRadioIndication>> IImsRadioIndication::castFrom(const ::android::sp<IImsRadioIndication>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<IImsRadioIndication>> IImsRadioIndication::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
  return ::android::sp<IImsRadioIndication>(static_cast<IImsRadioIndication *>(parent.get()));
}

// static
::android::sp<IImsRadioIndication> IImsRadioIndication::tryGetService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

// static
::android::sp<IImsRadioIndication> IImsRadioIndication::getService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::status_t IImsRadioIndication::registerAsService(const std::string &serviceName) {
    return ::android::OK;
}

bool IImsRadioIndication::registerForNotifications(
        const std::string &serviceName,
        const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification) {
    return false;
}

}  // namespace V1_0
}  // namespace ims
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
