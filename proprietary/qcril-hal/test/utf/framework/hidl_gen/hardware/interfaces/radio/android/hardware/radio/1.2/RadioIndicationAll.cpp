#define LOG_TAG "android.hardware.radio@1.2::RadioIndication"
#undef UNUSED

#include <android/hardware/radio/1.2/IRadioIndication.h>
#include <android/log.h>
#include <cutils/trace.h>
#include "ril_utf_hidl_services.h"

namespace android {
namespace hardware {
namespace radio {
namespace V1_2 {

std::string toString(const ::android::sp<IRadioIndication>& o) {
    std::string os = "[class or subclass of ";
    os += IRadioIndication::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}

const char* IRadioIndication::descriptor("android.hardware.radio@1.2::IRadioIndication");

// Methods from ::android::hidl::base::V1_0::IBase follow.
::android::hardware::Return<void> IRadioIndication::interfaceChain(interfaceChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IRadioIndication::debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options){
    (void)fd;
    (void)options;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IRadioIndication::interfaceDescriptor(interfaceDescriptor_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IRadioIndication::getHashChain(getHashChain_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IRadioIndication::setHALInstrumentation(){
    return ::android::hardware::Void();
}

::android::hardware::Return<bool> IRadioIndication::linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie){
    (void)cookie;
    return (recipient != nullptr);
}

::android::hardware::Return<void> IRadioIndication::ping(){
    return ::android::hardware::Void();
}

::android::hardware::Return<void> IRadioIndication::getDebugInfo(getDebugInfo_cb _hidl_cb){
    (void)_hidl_cb;
    return ::android::hardware::Void();}

::android::hardware::Return<void> IRadioIndication::notifySyspropsChanged(){
    return ::android::hardware::Void();}

::android::hardware::Return<bool> IRadioIndication::unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient){
    return (recipient != nullptr);
}


// static 
::android::hardware::Return<::android::sp<::android::hardware::radio::V1_2::IRadioIndication>> IRadioIndication::castFrom(const ::android::sp<::android::hardware::radio::V1_2::IRadioIndication>& parent, bool /* emitError */) {
    return parent;
}

// static 
::android::hardware::Return<::android::sp<::android::hardware::radio::V1_2::IRadioIndication>> IRadioIndication::castFrom(const ::android::sp<::android::hardware::radio::V1_1::IRadioIndication>& parent, bool emitError) {
    return ::android::sp<::android::hardware::radio::V1_2::IRadioIndication>(static_cast<::android::hardware::radio::V1_2::IRadioIndication*>(parent.get()));
}

// static 
::android::hardware::Return<::android::sp<::android::hardware::radio::V1_2::IRadioIndication>> IRadioIndication::castFrom(const ::android::sp<::android::hardware::radio::V1_0::IRadioIndication>& parent, bool emitError) {
    return ::android::sp<::android::hardware::radio::V1_2::IRadioIndication>(static_cast<::android::hardware::radio::V1_2::IRadioIndication*>(parent.get()));
}

// static 
::android::hardware::Return<::android::sp<::android::hardware::radio::V1_2::IRadioIndication>> IRadioIndication::castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError) {
    return ::android::sp<::android::hardware::radio::V1_2::IRadioIndication>(static_cast<::android::hardware::radio::V1_2::IRadioIndication*>(parent.get()));
}

// static
::android::sp<IRadioIndication> IRadioIndication::tryGetService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

// static
::android::sp<IRadioIndication> IRadioIndication::getService(const std::string &serviceName, const bool getStub) {
    return nullptr;
}

::android::status_t IRadioIndication::registerAsService(const std::string &serviceName) {
    return ::android::OK;
}

bool IRadioIndication::registerForNotifications(
        const std::string &serviceName,
        const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification) {
    return nullptr;
}

}  // namespace V1_2
}  // namespace radio
}  // namespace hardware
}  // namespace android
