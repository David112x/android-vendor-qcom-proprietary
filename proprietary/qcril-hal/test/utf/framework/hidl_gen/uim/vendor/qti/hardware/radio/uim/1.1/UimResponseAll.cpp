/******************************************************************************
#  Copyright (c) 2018 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
#******************************************************************************/

#define LOG_TAG "vendor.qti.hardware.radio.uim@1.1::UimResponse"

#undef UNUSED

#include <vendor/qti/hardware/radio/uim/1.1/IUimResponse.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"


namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace uim {
namespace V1_1 {

const char* IUimResponse::descriptor("vendor.qti.hardware.radio.uim@1.1::IUimResponse");

// Methods from ::vendor::qti::hardware::radio::uim::V1_0::IUimResponse follow.
// no default implementation for: ::android::hardware::Return<void> IUimResponse::UimRemoteSimlockResponse(int32_t token, ::vendor::qti::hardware::radio::uim::V1_0::UimRemoteSimlockResponseType response, ::vendor::qti::hardware::radio::uim::V1_0::UimRemoteSimlockOperationType simlockOp, const ::android::hardware::hidl_vec<uint8_t>& simlockRspData, const ::vendor::qti::hardware::radio::uim::V1_0::UimRemoteSimlockVersion& version, const ::vendor::qti::hardware::radio::uim::V1_0::UimRemoteSimlockStatus& status)

// Methods from ::vendor::qti::hardware::radio::uim::V1_1::IUimResponse follow.
// no default implementation for: ::android::hardware::Return<void> IUimResponse::uimRemoteSimlockResponse_1_1(int32_t token, ::vendor::qti::hardware::radio::uim::V1_0::UimRemoteSimlockResponseType response, ::vendor::qti::hardware::radio::uim::V1_1::UimRemoteSimlockOperationType simlockOp, const ::android::hardware::hidl_vec<uint8_t>& simlockRspData, const ::vendor::qti::hardware::radio::uim::V1_0::UimRemoteSimlockVersion& version, const ::vendor::qti::hardware::radio::uim::V1_0::UimRemoteSimlockStatus& status)

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IUimResponse::interfaceChain(interfaceChain_cb _hidl_cb){
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimResponse::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimResponse::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    _hidl_cb(::vendor::qti::hardware::radio::uim::V1_1::IUimResponse::descriptor);
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimResponse::getHashChain(getHashChain_cb _hidl_cb){
    _hidl_cb({
        (uint8_t[32]){0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0} /* 0000000000000000000000000000000000000000000000000000000000000000 */,
        (uint8_t[32]){32,18,159,29,70,197,233,65,49,170,88,90,176,233,56,245,143,171,217,223,223,136,99,14,125,99,113,218,205,65,84,93} /* 20129f1d46c5e94131aa585ab0e938f58fabd9dfdf88630e7d6371dacd41545d */,
        (uint8_t[32]){189,218,182,24,77,122,52,109,166,160,125,192,130,140,241,154,105,111,76,170,54,17,197,31,46,20,86,90,20,180,15,217} /* bddab6184d7a346da6a07dc0828cf19a696f4caa3611c51f2e14565a14b40fd9 */});
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IUimResponse::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IUimResponse::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IUimResponse::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IUimResponse::getDebugInfo(getDebugInfo_cb _hidl_cb){
    _hidl_cb({ -1 /* pid */, 0 /* ptr */, 
    #if defined(__LP64__)
    ::android::hidl::base::V1_0::DebugInfo::Architecture::IS_64BIT
    #else
    ::android::hidl::base::V1_0::DebugInfo::Architecture::IS_32BIT
    #endif
    });
    return ::android::hardware::Void();}

::android::hardware::Return<void> IUimResponse::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IUimResponse::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::uim::V1_1::IUimResponse>> IUimResponse::castFrom(const ::android::sp<::vendor::qti::hardware::radio::uim::V1_1::IUimResponse>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::uim::V1_1::IUimResponse>> IUimResponse::castFrom(const ::android::sp<::vendor::qti::hardware::radio::uim::V1_0::IUimResponse>& parent, bool emitError) {
    return nullptr;
}

// static 
::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::uim::V1_1::IUimResponse>> IUimResponse::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return nullptr;
}

// static
::android::sp<IUimResponse> IUimResponse::tryGetService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

// static
::android::sp<IUimResponse> IUimResponse::getService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::status_t IUimResponse::registerAsService(const std::string &serviceName) {
    return ::android::OK;
}

bool IUimResponse::registerForNotifications(
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
