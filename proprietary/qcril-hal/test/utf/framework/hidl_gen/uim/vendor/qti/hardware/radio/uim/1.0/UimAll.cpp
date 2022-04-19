/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.

  ---------------------------------------------------------------------------

  @file framework/hidl_gen/uim/vendor/qti/hardware/radio/uim/1.0/UimAll.cpp
  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#define LOG_TAG "vendor.qti.hardware.radio.uim@1.0::Uim"

#undef UNUSED

#include <vendor/qti/hardware/radio/uim/1.0/IUim.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace uim {
namespace V1_0 {

std::string toString(const ::android::sp<IUim>& o) {
    std::string os = "[class or subclass of ";
    os += IUim::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* IUim::descriptor("vendor.qti.hardware.radio.uim@1.0::IUim");

// Methods from IUim follow.
// no default implementation for: ::android::hardware::Return<void> IUim::setCallback(const ::android::sp<IUimResponse>& responseCallback, const ::android::sp<IUimIndication>& indicationCallback)
// no default implementation for: ::android::hardware::Return<void> IUim::UimRemoteSimlockRequest(int32_t token, UimRemoteSimlockOperationType simlockOp, const ::android::hardware::hidl_vec<uint8_t>& simlockData)

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IUim::interfaceChain(interfaceChain_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUim::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUim::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUim::getHashChain(getHashChain_cb _hidl_cb){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IUim::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IUim::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IUim::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IUim::getDebugInfo(getDebugInfo_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUim::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IUim::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<IUim>> IUim::castFrom(const ::android::sp<IUim>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<IUim>> IUim::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return nullptr;
}

// static
::android::sp<IUim> IUim::tryGetService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

// static
::android::sp<IUim> IUim::getService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::status_t IUim::registerAsService(const std::string &serviceName) {
    return ::android::OK;
}

bool IUim::registerForNotifications(
        const std::string &serviceName,
        const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification) {
    return true;
}

}  // namespace V1_0
}  // namespace uim
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
