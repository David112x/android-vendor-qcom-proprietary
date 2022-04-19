#define LOG_TAG "android.hardware.radio@1.1::Radio"
#undef UNUSED

#include <android/hardware/radio/1.1/IRadio.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"

namespace android {
namespace hardware {
namespace radio {
namespace V1_1 {

const char* IRadio::descriptor("android.hardware.radio@1.1::IRadio");

std::string toString(const ::android::sp<IRadio>& o) {
    std::string os = "[class or subclass of ";
    os += IRadio::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IRadio::interfaceChain(interfaceChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IRadio::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IRadio::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IRadio::getHashChain(getHashChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IRadio::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IRadio::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IRadio::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IRadio::getDebugInfo(getDebugInfo_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IRadio::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IRadio::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<::android::hardware::radio::V1_1::IRadio>> IRadio::castFrom(const ::android::sp<::android::hardware::radio::V1_1::IRadio>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<::android::hardware::radio::V1_1::IRadio>> IRadio::castFrom(const ::android::sp<::android::hardware::radio::V1_0::IRadio>& parent, bool emitError) {
    return ::android::sp<::android::hardware::radio::V1_1::IRadio>(static_cast<::android::hardware::radio::V1_1::IRadio*>(parent.get()));
}

// static 
::android::hardware::Return<::android::sp<::android::hardware::radio::V1_1::IRadio>> IRadio::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return ::android::sp<::android::hardware::radio::V1_1::IRadio>(static_cast<::android::hardware::radio::V1_1::IRadio*>(parent.get()));
}

static std::unordered_map<std::string, ::android::sp<::android::hardware::radio::V1_1::IRadio>> mHidlServices;
// static
::android::sp<IRadio> IRadio::tryGetService(const std::string &serviceName, const bool getStub) {
    return mHidlServices[serviceName];
}

// static
::android::sp<IRadio> IRadio::getService(const std::string &serviceName, const bool getStub) {
    return mHidlServices[serviceName];
}

::android::status_t IRadio::registerAsService(const std::string &serviceName) {
    mHidlServices[serviceName] = this;
    ::android::hardware::radio::V1_0::IRadio::registerAsService(serviceName);
    return ::android::OK;
}

bool IRadio::registerForNotifications(
        const std::string &serviceName,
        const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification) {
    return true;
}

}  // namespace V1_1
}  // namespace radio
}  // namespace hardware
}  // namespace android
