/******************************************************************************
  @file framework/hidl_gen/am/vendor/qti/hardware/radio/am/1.0/QcRilAudioAll.cpp
  DESCRIPTION
  ---------------------------------------------------------------------------
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/
#define LOG_TAG "vendor.qti.hardware.radio.am@1.0::QcRilAudio"

#undef UNUSED

#include <vendor/qti/hardware/radio/am/1.0/IQcRilAudio.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace am {
namespace V1_0 {

std::string toString(const ::android::sp<IQcRilAudio>& o) {
    std::string os = "[class or subclass of ";
    os += IQcRilAudio::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* IQcRilAudio::descriptor("vendor.qti.hardware.radio.am@1.0::IQcRilAudio");


// Methods from IQcRilAudio follow.
// no default implementation for: ::android::hardware::Return<void> IQcRilAudio::setCallback(const ::android::sp<IQcRilAudioCallback>& callback)
// no default implementation for: ::android::hardware::Return<void> IQcRilAudio::setError(AudioError errorCode)

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IQcRilAudio::interfaceChain(interfaceChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQcRilAudio::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQcRilAudio::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQcRilAudio::getHashChain(getHashChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IQcRilAudio::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IQcRilAudio::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IQcRilAudio::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IQcRilAudio::getDebugInfo(getDebugInfo_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQcRilAudio::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IQcRilAudio::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<IQcRilAudio>> IQcRilAudio::castFrom(const ::android::sp<IQcRilAudio>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<IQcRilAudio>> IQcRilAudio::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return nullptr;
}

static std::unordered_map<std::string, ::android::sp<IQcRilAudio>> mHidlServices;
// static
::android::sp<IQcRilAudio> IQcRilAudio::tryGetService(const std::string &serviceName, const bool getStub) {
    return mHidlServices[serviceName];
}

// static
::android::sp<IQcRilAudio> IQcRilAudio::getService(const std::string &serviceName, const bool getStub) {
    return mHidlServices[serviceName];
}

::android::status_t IQcRilAudio::registerAsService(const std::string &serviceName) {
    mHidlServices[serviceName] = this;
    return ::android::OK;
}

bool IQcRilAudio::registerForNotifications(
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
