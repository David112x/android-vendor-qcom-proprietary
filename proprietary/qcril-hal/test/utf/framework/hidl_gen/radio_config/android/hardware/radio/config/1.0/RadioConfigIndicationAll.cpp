/******************************************************************************
  @file framework/hidl_gen/radio_config/android/hardware/radio/config/1.0/RadioConfigIndicationAll.cpp
  DESCRIPTION
  ---------------------------------------------------------------------------
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------
******************************************************************************/
#define LOG_TAG "android.hardware.radio.config@1.0::RadioConfigIndication"

#undef UNUSED

#include <android/hardware/radio/config/1.0/IRadioConfigIndication.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"

namespace android {
namespace hardware {
namespace radio {
namespace config {
namespace V1_0 {

std::string toString(const ::android::sp<::android::hardware::radio::config::V1_0::IRadioConfigIndication>& o) {
    std::string os = "[class or subclass of ";
    os += ::android::hardware::radio::config::V1_0::IRadioConfigIndication::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* IRadioConfigIndication::descriptor("android.hardware.radio.config@1.0::IRadioConfigIndication");

// Methods from ::android::hardware::radio::config::V1_0::IRadioConfigIndication follow.
// no default implementation for: ::android::hardware::Return<void> IRadioConfigIndication::simSlotsStatusChanged(::android::hardware::radio::V1_0::RadioIndicationType type, const ::android::hardware::hidl_vec<::android::hardware::radio::config::V1_0::SimSlotStatus>& slotStatus)

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IRadioConfigIndication::interfaceChain(interfaceChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IRadioConfigIndication::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IRadioConfigIndication::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IRadioConfigIndication::getHashChain(getHashChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IRadioConfigIndication::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IRadioConfigIndication::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IRadioConfigIndication::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IRadioConfigIndication::getDebugInfo(getDebugInfo_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IRadioConfigIndication::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IRadioConfigIndication::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<::android::hardware::radio::config::V1_0::IRadioConfigIndication>> IRadioConfigIndication::castFrom(const ::android::sp<::android::hardware::radio::config::V1_0::IRadioConfigIndication>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<::android::hardware::radio::config::V1_0::IRadioConfigIndication>> IRadioConfigIndication::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return nullptr;
}

// static
::android::sp<IRadioConfigIndication> IRadioConfigIndication::tryGetService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

// static
::android::sp<IRadioConfigIndication> IRadioConfigIndication::getService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::status_t IRadioConfigIndication::registerAsService(const std::string &serviceName) {
    return ::android::OK;
}

bool IRadioConfigIndication::registerForNotifications(
        const std::string &serviceName,
        const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification) {
    return true;
}

}  // namespace V1_0
}  // namespace config
}  // namespace radio
}  // namespace hardware
}  // namespace android
