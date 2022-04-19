/******************************************************************************
  @file framework/hidl_gen/ims/vendor/qti/hardware/radio/ims/1.0/ImsRadioResponseAll.cpp
  DESCRIPTION
  ---------------------------------------------------------------------------
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/
#define LOG_TAG "vendor.qti.hardware.radio.ims@1.0::ImsRadioResponse"

#undef UNUSED

#include <vendor/qti/hardware/radio/ims/1.0/IImsRadioResponse.h>
#include <android/log.h>
#include <cutils/trace.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace ims {
namespace V1_0 {

std::string toString(const ::android::sp<IImsRadioResponse>& o) {
    std::string os = "[class or subclass of ";
    os += IImsRadioResponse::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* IImsRadioResponse::descriptor("vendor.qti.hardware.radio.ims@1.0::IImsRadioResponse");

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IImsRadioResponse::interfaceChain(interfaceChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IImsRadioResponse::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IImsRadioResponse::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IImsRadioResponse::getHashChain(getHashChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IImsRadioResponse::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IImsRadioResponse::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IImsRadioResponse::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IImsRadioResponse::getDebugInfo(getDebugInfo_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IImsRadioResponse::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IImsRadioResponse::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<IImsRadioResponse>> IImsRadioResponse::castFrom(const ::android::sp<IImsRadioResponse>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<IImsRadioResponse>> IImsRadioResponse::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
  return ::android::sp<IImsRadioResponse>(static_cast<IImsRadioResponse *>(parent.get()));
}

// static
::android::sp<IImsRadioResponse> IImsRadioResponse::tryGetService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

// static
::android::sp<IImsRadioResponse> IImsRadioResponse::getService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::status_t IImsRadioResponse::registerAsService(const std::string &serviceName) {
    return ::android::OK;
}

bool IImsRadioResponse::registerForNotifications(
        const std::string &serviceName,
        const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification) {
    return true;
}

}  // namespace V1_0
}  // namespace ims
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
