#define LOG_TAG "android.hardware.radio.deprecated@1.0::OemHookIndication"
#undef UNUSED

#include <android/hardware/radio/deprecated/1.0/IOemHookIndication.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"

namespace android {
namespace hardware {
namespace radio {
namespace deprecated {
namespace V1_0 {

std::string toString(const ::android::sp<IOemHookIndication>& o) {
    std::string os = "[class or subclass of ";
    os += IOemHookIndication::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* IOemHookIndication::descriptor("android.hardware.radio.deprecated@1.0::IOemHookIndication");

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IOemHookIndication::interfaceChain(interfaceChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IOemHookIndication::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IOemHookIndication::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IOemHookIndication::getHashChain(getHashChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IOemHookIndication::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IOemHookIndication::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IOemHookIndication::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IOemHookIndication::getDebugInfo(getDebugInfo_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IOemHookIndication::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IOemHookIndication::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<::android::hardware::radio::deprecated::V1_0::IOemHookIndication>> IOemHookIndication::castFrom(const ::android::sp<::android::hardware::radio::deprecated::V1_0::IOemHookIndication>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<::android::hardware::radio::deprecated::V1_0::IOemHookIndication>> IOemHookIndication::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return ::android::sp<::android::hardware::radio::deprecated::V1_0::IOemHookIndication>(static_cast<::android::hardware::radio::deprecated::V1_0::IOemHookIndication*>(parent.get()));
}

// static
::android::sp<IOemHookIndication> IOemHookIndication::tryGetService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

// static
::android::sp<IOemHookIndication> IOemHookIndication::getService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::status_t IOemHookIndication::registerAsService(const std::string &serviceName) {
    return ::android::OK;
}

bool IOemHookIndication::registerForNotifications(
        const std::string &serviceName,
        const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification) {
    return true;
}

}  // namespace V1_0
}  // namespace deprecated
}  // namespace radio
}  // namespace hardware
}  // namespace android
