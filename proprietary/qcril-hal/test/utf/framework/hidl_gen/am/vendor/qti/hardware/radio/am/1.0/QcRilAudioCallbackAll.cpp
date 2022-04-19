/******************************************************************************
  @file framework/hidl_gen/am/vendor/qti/hardware/radio/am/1.0/QcRilAudioCallbackAll.cpp
  DESCRIPTION
  ---------------------------------------------------------------------------
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/
#define LOG_TAG "vendor.qti.hardware.radio.am@1.0::QcRilAudioCallback"

#undef UNUSED

#include <vendor/qti/hardware/radio/am/1.0/IQcRilAudioCallback.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"


namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace am {
namespace V1_0 {

std::string toString(const ::android::sp<IQcRilAudioCallback>& o) {
    std::string os = "[class or subclass of ";
    os += IQcRilAudioCallback::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* IQcRilAudioCallback::descriptor("vendor.qti.hardware.radio.am@1.0::IQcRilAudioCallback");


// Methods from IQcRilAudioCallback follow.
// no default implementation for: ::android::hardware::Return<void> IQcRilAudioCallback::getParameters(const ::android::hardware::hidl_string& params, getParameters_cb _hidl_cb)
// no default implementation for: ::android::hardware::Return<int32_t> IQcRilAudioCallback::setParameters(const ::android::hardware::hidl_string& params)

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IQcRilAudioCallback::interfaceChain(interfaceChain_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQcRilAudioCallback::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQcRilAudioCallback::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQcRilAudioCallback::getHashChain(getHashChain_cb _hidl_cb){
   return ::android::hardware::Void();
}

::android::hardware::Return<void> IQcRilAudioCallback::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IQcRilAudioCallback::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IQcRilAudioCallback::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IQcRilAudioCallback::getDebugInfo(getDebugInfo_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQcRilAudioCallback::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IQcRilAudioCallback::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<IQcRilAudioCallback>> IQcRilAudioCallback::castFrom(const ::android::sp<IQcRilAudioCallback>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<IQcRilAudioCallback>> IQcRilAudioCallback::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return nullptr;
}

// static
::android::sp<IQcRilAudioCallback> IQcRilAudioCallback::tryGetService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

// static
::android::sp<IQcRilAudioCallback> IQcRilAudioCallback::getService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::status_t IQcRilAudioCallback::registerAsService(const std::string &serviceName) {
    return ::android::OK;
}

bool IQcRilAudioCallback::registerForNotifications(
        const std::string &serviceName,
        const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification) {
    return true;
}

}  // namespace V1_0
}  // namespace am
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
