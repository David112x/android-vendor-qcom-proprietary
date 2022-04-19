/******************************************************************************
  @file framework/hidl_gen/radio_config/android/hardware/radio/config/1.0/RadioConfigAll.cpp
  DESCRIPTION
  ---------------------------------------------------------------------------
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/
#define LOG_TAG "android.hardware.radio.config@1.0::RadioConfig"

#undef UNUSED

#include <android/hardware/radio/config/1.0/IRadioConfig.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"

namespace android {
namespace hardware {
namespace radio {
namespace config {
namespace V1_0 {

std::string toString(const ::android::sp<::android::hardware::radio::config::V1_0::IRadioConfig>& o) {
    std::string os = "[class or subclass of ";
    os += ::android::hardware::radio::config::V1_0::IRadioConfig::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* IRadioConfig::descriptor("android.hardware.radio.config@1.0::IRadioConfig");

// Methods from ::android::hardware::radio::config::V1_0::IRadioConfig follow.
// no default implementation for: ::android::hardware::Return<void> IRadioConfig::setResponseFunctions(const ::android::sp<::android::hardware::radio::config::V1_0::IRadioConfigResponse>& radioConfigResponse, const ::android::sp<::android::hardware::radio::config::V1_0::IRadioConfigIndication>& radioConfigIndication)
// no default implementation for: ::android::hardware::Return<void> IRadioConfig::getSimSlotsStatus(int32_t serial)
// no default implementation for: ::android::hardware::Return<void> IRadioConfig::setSimSlotsMapping(int32_t serial, const ::android::hardware::hidl_vec<uint32_t>& slotMap)

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IRadioConfig::interfaceChain(interfaceChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IRadioConfig::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IRadioConfig::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IRadioConfig::getHashChain(getHashChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IRadioConfig::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IRadioConfig::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IRadioConfig::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IRadioConfig::getDebugInfo(getDebugInfo_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IRadioConfig::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IRadioConfig::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<::android::hardware::radio::config::V1_0::IRadioConfig>> IRadioConfig::castFrom(const ::android::sp<::android::hardware::radio::config::V1_0::IRadioConfig>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<::android::hardware::radio::config::V1_0::IRadioConfig>> IRadioConfig::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return nullptr;
}

static std::unordered_map<std::string, ::android::sp<IRadioConfig>> mHidlServices;
// static
::android::sp<IRadioConfig> IRadioConfig::tryGetService(const std::string &serviceName, const bool getStub) {
    return mHidlServices[serviceName];
}

// static
::android::sp<IRadioConfig> IRadioConfig::getService(const std::string &serviceName, const bool getStub) {
    return mHidlServices[serviceName];
}

::android::status_t IRadioConfig::registerAsService(const std::string &serviceName) {
    mHidlServices[serviceName] = this;
    return ::android::OK;
}

bool IRadioConfig::registerForNotifications(
        const std::string &serviceName,
        const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification) {
    return true;
}

}  // namespace V1_0
}  // namespace config
}  // namespace radio
}  // namespace hardware
}  // namespace android
