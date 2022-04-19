/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#define LOG_TAG "vendor.qti.hardware.radio.uim@1.1::Uim"

#undef UNUSED

#include <vendor/qti/hardware/radio/uim/1.1/IUim.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"


namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace uim {
namespace V1_1 {

const char* IUim::descriptor("vendor.qti.hardware.radio.uim@1.1::IUim");

// Methods from ::vendor::qti::hardware::radio::uim::V1_0::IUim follow.
// no default implementation for: ::android::hardware::Return<void> IUim::setCallback(const ::android::sp<::vendor::qti::hardware::radio::uim::V1_0::IUimResponse>& responseCallback, const ::android::sp<::vendor::qti::hardware::radio::uim::V1_0::IUimIndication>& indicationCallback)
// no default implementation for: ::android::hardware::Return<void> IUim::UimRemoteSimlockRequest(int32_t token, ::vendor::qti::hardware::radio::uim::V1_0::UimRemoteSimlockOperationType simlockOp, const ::android::hardware::hidl_vec<uint8_t>& simlockData)

// Methods from ::vendor::qti::hardware::radio::uim::V1_1::IUim follow.
// no default implementation for: ::android::hardware::Return<void> IUim::uimRemoteSimlockRequest_1_1(int32_t token, ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType simlockOp, const ::android::hardware::hidl_vec<uint8_t>& simlockData)

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IUim::interfaceChain(interfaceChain_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUim::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUim::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    _hidl_cb(::vendor::qti::hardware::radio::uim::V1_1::IUim::descriptor);
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUim::getHashChain(getHashChain_cb _hidl_cb){
    _hidl_cb({
        (uint8_t[32]){0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} /* 0000000000000000000000000000000000000000000000000000000000000000 */,
        (uint8_t[32]){35,137,201,97,25,111,236,229,113,197,222,212,52,38,196,247,27,220,90,91,130,94,203,14,156,109,34,187,177,126,247,136} /* 2389c961196fece571c5ded43426c4f71bdc5a5b825ecb0e9c6d22bbb17ef788 */,
        (uint8_t[32]){189,218,182,24,77,122,52,109,166,160,125,192,130,140,241,154,105,111,76,170,54,17,197,31,46,20,86,90,20,180,15,217} /* bddab6184d7a346da6a07dc0828cf19a696f4caa3611c51f2e14565a14b40fd9 */});
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
    _hidl_cb({ -1 /* pid */, 0 /* ptr */, 
    #if defined(__LP64__)
    ::android::hidl::base::V1_0::DebugInfo::Architecture::IS_64BIT
    #else
    ::android::hidl::base::V1_0::DebugInfo::Architecture::IS_32BIT
    #endif
    });
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUim::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IUim::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::uim::V1_1::IUim>> IUim::castFrom(const ::android::sp<::vendor::qti::hardware::radio::uim::V1_1::IUim>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::uim::V1_1::IUim>> IUim::castFrom(const ::android::sp<::vendor::qti::hardware::radio::uim::V1_0::IUim>& parent, bool emitError) {
    return nullptr;
}

// static 
::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::uim::V1_1::IUim>> IUim::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
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

static_assert(sizeof(::android::hardware::MQDescriptor<char, ::android::hardware::kSynchronizedReadWrite>) == 32, "wrong size");
static_assert(sizeof(::android::hardware::hidl_handle) == 16, "wrong size");
static_assert(sizeof(::android::hardware::hidl_memory) == 40, "wrong size");
static_assert(sizeof(::android::hardware::hidl_string) == 16, "wrong size");
static_assert(sizeof(::android::hardware::hidl_vec<char>) == 16, "wrong size");

}  // namespace V1_1
}  // namespace uim
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor
