/******************************************************************************
  @file framework/hidl_gen/radio_config/android/hardware/radio/config/1.0/RadioConfigResponseAll.cpp
  DESCRIPTION
  ---------------------------------------------------------------------------
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/
#define LOG_TAG "android.hardware.radio.config@1.0::RadioConfigResponse"

#undef UNUSED

#include <android/hardware/radio/config/1.0/IRadioConfigResponse.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"

namespace android {
namespace hardware {
namespace radio {
namespace config {
namespace V1_0 {

std::string toString(const ::android::sp<::android::hardware::radio::config::V1_0::IRadioConfigResponse>& o) {
    std::string os = "[class or subclass of ";
    os += ::android::hardware::radio::config::V1_0::IRadioConfigResponse::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* IRadioConfigResponse::descriptor("android.hardware.radio.config@1.0::IRadioConfigResponse");

// Methods from ::android::hardware::radio::config::V1_0::IRadioConfigResponse follow.
// no default implementation for: ::android::hardware::Return<void> IRadioConfigResponse::getSimSlotsStatusResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info, const ::android::hardware::hidl_vec<::android::hardware::radio::config::V1_0::SimSlotStatus>& slotStatus)
// no default implementation for: ::android::hardware::Return<void> IRadioConfigResponse::setSimSlotsMappingResponse(const ::android::hardware::radio::V1_0::RadioResponseInfo& info)

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IRadioConfigResponse::interfaceChain(interfaceChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IRadioConfigResponse::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IRadioConfigResponse::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IRadioConfigResponse::getHashChain(getHashChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IRadioConfigResponse::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IRadioConfigResponse::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IRadioConfigResponse::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IRadioConfigResponse::getDebugInfo(getDebugInfo_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IRadioConfigResponse::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IRadioConfigResponse::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<::android::hardware::radio::config::V1_0::IRadioConfigResponse>> IRadioConfigResponse::castFrom(const ::android::sp<::android::hardware::radio::config::V1_0::IRadioConfigResponse>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<::android::hardware::radio::config::V1_0::IRadioConfigResponse>> IRadioConfigResponse::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return nullptr;
}

// static
::android::sp<IRadioConfigResponse> IRadioConfigResponse::tryGetService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

// static
::android::sp<IRadioConfigResponse> IRadioConfigResponse::getService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::status_t IRadioConfigResponse::registerAsService(const std::string &serviceName) {
    return ::android::OK;
}

bool IRadioConfigResponse::registerForNotifications(
        const std::string &serviceName,
        const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification) {
    return true;
}

}  // namespace V1_0
}  // namespace config
}  // namespace radio
}  // namespace hardware
}  // namespace android
