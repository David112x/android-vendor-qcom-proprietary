/******************************************************************************
#  Copyright (c) 2017 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file framework/hidl_gen/uim_remote_server/vendor/qti/hardware/radio/uim_remote_server/1.0/UimRemoteServiceServerIndicationAll.cpp
  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#define LOG_TAG "vendor.qti.hardware.radio.uim_remote_server@1.0::UimRemoteServiceServerIndication"

#undef UNUSED
#include <vendor/qti/hardware/radio/uim_remote_server/1.0/IUimRemoteServiceServerIndication.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace uim_remote_server {
namespace V1_0 {

std::string toString(const ::android::sp<IUimRemoteServiceServerIndication>& o) {
    std::string os = "[class or subclass of ";
    os += IUimRemoteServiceServerIndication::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* IUimRemoteServiceServerIndication::descriptor("vendor.qti.hardware.radio.uim_remote_server@1.0::IUimRemoteServiceServerIndication");

// Methods from IUimRemoteServiceServerIndication follow.
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceServerIndication::uimRemoteServiceServerDisconnectIndication(UimRemoteServiceServerDisconnectType disconnectType)
// no default implementation for: ::android::hardware::Return<void> IUimRemoteServiceServerIndication::uimRemoteServiceServerStatusIndication(UimRemoteServiceServerStatus status)

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IUimRemoteServiceServerIndication::interfaceChain(interfaceChain_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimRemoteServiceServerIndication::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimRemoteServiceServerIndication::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimRemoteServiceServerIndication::getHashChain(getHashChain_cb _hidl_cb){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IUimRemoteServiceServerIndication::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IUimRemoteServiceServerIndication::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IUimRemoteServiceServerIndication::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IUimRemoteServiceServerIndication::getDebugInfo(getDebugInfo_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimRemoteServiceServerIndication::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IUimRemoteServiceServerIndication::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<IUimRemoteServiceServerIndication>> IUimRemoteServiceServerIndication::castFrom(const ::android::sp<IUimRemoteServiceServerIndication>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<IUimRemoteServiceServerIndication>> IUimRemoteServiceServerIndication::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return nullptr;
}

// static
::android::sp<IUimRemoteServiceServerIndication> IUimRemoteServiceServerIndication::tryGetService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

// static
::android::sp<IUimRemoteServiceServerIndication> IUimRemoteServiceServerIndication::getService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::status_t IUimRemoteServiceServerIndication::registerAsService(const std::string &serviceName) {
    return ::android::OK;
}

bool IUimRemoteServiceServerIndication::registerForNotifications(
        const std::string &serviceName,
        const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification) {
    return true;
}

}  // namespace V1_0
}  // namespace uim_remote_server
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
