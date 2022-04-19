#define LOG_TAG "android.hardware.radio.deprecated@1.0::OemHookResponse"
#undef UNUSED

#include <android/hardware/radio/deprecated/1.0/IOemHookResponse.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"

namespace android {
namespace hardware {
namespace radio {
namespace deprecated {
namespace V1_0 {

std::string toString(const ::android::sp<IOemHookResponse>& o) {
    std::string os = "[class or subclass of ";
    os += IOemHookResponse::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* IOemHookResponse::descriptor("android.hardware.radio.deprecated@1.0::IOemHookResponse");

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IOemHookResponse::interfaceChain(interfaceChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IOemHookResponse::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IOemHookResponse::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IOemHookResponse::getHashChain(getHashChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IOemHookResponse::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IOemHookResponse::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IOemHookResponse::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IOemHookResponse::getDebugInfo(getDebugInfo_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IOemHookResponse::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IOemHookResponse::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<::android::hardware::radio::deprecated::V1_0::IOemHookResponse>> IOemHookResponse::castFrom(const ::android::sp<::android::hardware::radio::deprecated::V1_0::IOemHookResponse>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<::android::hardware::radio::deprecated::V1_0::IOemHookResponse>> IOemHookResponse::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return ::android::sp<::android::hardware::radio::deprecated::V1_0::IOemHookResponse>(static_cast<::android::hardware::radio::deprecated::V1_0::IOemHookResponse*>(parent.get()));
}

// static
::android::sp<IOemHookResponse> IOemHookResponse::tryGetService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

// static
::android::sp<IOemHookResponse> IOemHookResponse::getService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::status_t IOemHookResponse::registerAsService(const std::string &serviceName) {
    return ::android::OK;
}

bool IOemHookResponse::registerForNotifications(
        const std::string &serviceName,
        const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification) {
    return true;
}

}  // namespace V1_0
}  // namespace deprecated
}  // namespace radio
}  // namespace hardware
}  // namespace android
