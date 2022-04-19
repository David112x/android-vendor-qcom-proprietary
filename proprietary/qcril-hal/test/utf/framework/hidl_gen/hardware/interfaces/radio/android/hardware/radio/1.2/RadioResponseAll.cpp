#define LOG_TAG "android.hardware.radio@1.2::RadioResponse"
#undef UNUSED

#include <android/hardware/radio/1.2/IRadioResponse.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"

namespace android {
namespace hardware {
namespace radio {
namespace V1_2 {

std::string toString(const ::android::sp<IRadioResponse>& o) {
    std::string os = "[class or subclass of ";
    os += IRadioResponse::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* IRadioResponse::descriptor("android.hardware.radio@1.2::IRadioResponse");

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IRadioResponse::interfaceChain(interfaceChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IRadioResponse::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IRadioResponse::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IRadioResponse::getHashChain(getHashChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IRadioResponse::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IRadioResponse::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IRadioResponse::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IRadioResponse::getDebugInfo(getDebugInfo_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IRadioResponse::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IRadioResponse::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<::android::hardware::radio::V1_2::IRadioResponse>> IRadioResponse::castFrom(const ::android::sp<::android::hardware::radio::V1_2::IRadioResponse>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<::android::hardware::radio::V1_2::IRadioResponse>> IRadioResponse::castFrom(const ::android::sp<::android::hardware::radio::V1_1::IRadioResponse>& parent, bool emitError) {
    return ::android::sp<::android::hardware::radio::V1_2::IRadioResponse>(static_cast<::android::hardware::radio::V1_2::IRadioResponse*>(parent.get()));
}

// static 
::android::hardware::Return<::android::sp<::android::hardware::radio::V1_2::IRadioResponse>> IRadioResponse::castFrom(const ::android::sp<::android::hardware::radio::V1_0::IRadioResponse>& parent, bool emitError) {
    return ::android::sp<::android::hardware::radio::V1_2::IRadioResponse>(static_cast<::android::hardware::radio::V1_2::IRadioResponse*>(parent.get()));
}

// static 
::android::hardware::Return<::android::sp<::android::hardware::radio::V1_2::IRadioResponse>> IRadioResponse::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return ::android::sp<::android::hardware::radio::V1_2::IRadioResponse>(static_cast<::android::hardware::radio::V1_2::IRadioResponse*>(parent.get()));
}

// static
::android::sp<IRadioResponse> IRadioResponse::tryGetService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

// static
::android::sp<IRadioResponse> IRadioResponse::getService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::status_t IRadioResponse::registerAsService(const std::string &serviceName) {
    return ::android::OK;
}

bool IRadioResponse::registerForNotifications(
        const std::string &serviceName,
        const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification) {
    return true;
}

}  // namespace V1_2
}  // namespace radio
}  // namespace hardware
}  // namespace android
