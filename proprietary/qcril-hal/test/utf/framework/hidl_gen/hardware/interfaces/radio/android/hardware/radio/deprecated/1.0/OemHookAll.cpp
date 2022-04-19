#define LOG_TAG "android.hardware.radio.deprecated@1.0::OemHook"
#undef UNUSED

#include <android/hardware/radio/deprecated/1.0/IOemHook.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"

namespace android {
namespace hardware {
namespace radio {
namespace deprecated {
namespace V1_0 {

std::string toString(const ::android::sp<IOemHook>& o) {
    std::string os = "[class or subclass of ";
    os += IOemHook::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* IOemHook::descriptor("android.hardware.radio.deprecated@1.0::IOemHook");
// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IOemHook::interfaceChain(interfaceChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IOemHook::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IOemHook::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IOemHook::getHashChain(getHashChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IOemHook::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IOemHook::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IOemHook::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IOemHook::getDebugInfo(getDebugInfo_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IOemHook::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IOemHook::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<::android::hardware::radio::deprecated::V1_0::IOemHook>> IOemHook::castFrom(const ::android::sp<::android::hardware::radio::deprecated::V1_0::IOemHook>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<::android::hardware::radio::deprecated::V1_0::IOemHook>> IOemHook::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return ::android::sp<::android::hardware::radio::deprecated::V1_0::IOemHook>(static_cast<::android::hardware::radio::deprecated::V1_0::IOemHook*>(parent.get()));
}

static std::unordered_map<std::string, ::android::sp<IOemHook>> mHidlServices;
// static
::android::sp<IOemHook> IOemHook::tryGetService(const std::string &serviceName, const bool getStub) {
    return mHidlServices[serviceName];
}

// static
::android::sp<IOemHook> IOemHook::getService(const std::string &serviceName, const bool getStub) {
    return mHidlServices[serviceName];
}

::android::status_t IOemHook::registerAsService(const std::string &serviceName) {
    mHidlServices[serviceName] = this;
    return ::android::OK;
}

bool IOemHook::registerForNotifications(
        const std::string &serviceName,
        const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification) {
    return true;
}

}  // namespace V1_0
}  // namespace deprecated
}  // namespace radio
}  // namespace hardware
}  // namespace android
