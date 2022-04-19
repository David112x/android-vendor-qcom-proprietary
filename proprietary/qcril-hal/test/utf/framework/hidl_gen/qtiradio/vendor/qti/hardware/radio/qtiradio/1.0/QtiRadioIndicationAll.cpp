/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#define LOG_TAG "vendor.qti.hardware.radio.qtiradio@1.0::QtiRadioIndication"

#undef UNUSED

#include <vendor/qti/hardware/radio/qtiradio/1.0/IQtiRadioIndication.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace V1_0 {

std::string toString(const ::android::sp<IQtiRadioIndication>& o) {
    std::string os = "[class or subclass of ";
    os += IQtiRadioIndication::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* IQtiRadioIndication::descriptor("vendor.qti.hardware.radio.qtiradio@1.0::IQtiRadioIndication");

// Methods from IQtiRadioIndication follow.
// no default implementation for: ::android::hardware::Return<void> IQtiRadioIndication::qtiRadioIndication(QtiRadioIndicationType type)

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IQtiRadioIndication::interfaceChain(interfaceChain_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQtiRadioIndication::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQtiRadioIndication::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQtiRadioIndication::getHashChain(getHashChain_cb _hidl_cb){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IQtiRadioIndication::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IQtiRadioIndication::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IQtiRadioIndication::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IQtiRadioIndication::getDebugInfo(getDebugInfo_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IQtiRadioIndication::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IQtiRadioIndication::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<IQtiRadioIndication>> IQtiRadioIndication::castFrom(const ::android::sp<IQtiRadioIndication>& parent, bool /* emitError */) {
    return parent;
}

// static
::android::sp<IQtiRadioIndication> IQtiRadioIndication::tryGetService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

// static
::android::sp<IQtiRadioIndication> IQtiRadioIndication::getService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::status_t IQtiRadioIndication::registerAsService(const std::string &serviceName) {
    return ::android::OK;
}

bool IQtiRadioIndication::registerForNotifications(
        const std::string &serviceName,
        const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification) {
    return true;
}

static_assert(sizeof(::android::hardware::MQDescriptor<char, ::android::hardware::kSynchronizedReadWrite>) == 32, "wrong size");
static_assert(sizeof(::android::hardware::hidl_handle) == 16, "wrong size");
static_assert(sizeof(::android::hardware::hidl_memory) == 40, "wrong size");
static_assert(sizeof(::android::hardware::hidl_string) == 16, "wrong size");
static_assert(sizeof(::android::hardware::hidl_vec<char>) == 16, "wrong size");

}  // namespace V1_0
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
