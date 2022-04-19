#ifndef HIDL_GENERATED_ANDROID_HARDWARE_RADIO_DEPRECATED_V1_0_IOEMHOOK_H
#define HIDL_GENERATED_ANDROID_HARDWARE_RADIO_DEPRECATED_V1_0_IOEMHOOK_H

#include <android/hardware/radio/deprecated/1.0/IOemHookIndication.h>
#include <android/hardware/radio/deprecated/1.0/IOemHookResponse.h>
#include <android/hidl/base/1.0/IBase.h>

#include <android/hidl/manager/1.0/IServiceNotification.h>

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace android {
namespace hardware {
namespace radio {
namespace deprecated {
namespace V1_0 {

struct IOemHook : public ::android::hidl::base::V1_0::IBase {
    typedef android::hardware::details::i_tag _hidl_tag;

    // Forward declaration for forward reference support:

    /**
     * This interface has APIs for OEM-specific use-cases.
     * USE OF THIS INTERFACE IS DISCOURATED. IT IS PRESENT ONLY FOR BACKWARD COMPATIBILITY AND WILL BE
     * REMOVED IN O (ATTEMPTING TO REMOVE IT IN O, BUT IF NOT IN O WILL BE REMOVED IN P).
     * ALSO NOTE THAT FRAMEWORK EXPECTS THE SERVICE IMPLEMENTING THIS INTERFACE TO RESIDE
     * IN THE SAME PROCESS AS IRADIO SERVICE.
     */
    virtual bool isRemote() const override { return false; }


    /**
     * Set response functions for oem hook requests & oem hook indications.
     * 
     * @param oemHookResponse Object containing response functions
     * @param oemHookIndication Object containing oem hook indications
     */
    virtual ::android::hardware::Return<void> setResponseFunctions(const ::android::sp<::android::hardware::radio::deprecated::V1_0::IOemHookResponse>& oemHookResponse, const ::android::sp<::android::hardware::radio::deprecated::V1_0::IOemHookIndication>& oemHookIndication) = 0;

    /**
     * This request passes raw byte arrays between framework and vendor code.
     * 
     * @param serial Serial number of request.
     * @param data data passed as raw bytes
     * 
     * Response function is IOemHookResponse.sendRequestRawResponse()
     */
    virtual ::android::hardware::Return<void> sendRequestRaw(int32_t serial, const ::android::hardware::hidl_vec<uint8_t>& data) = 0;

    /**
     * This request passes strings between framework and vendor code.
     * 
     * @param serial Serial number of request.
     * @param data data passed as strings
     * 
     * Response function is IOemHookResponse.sendRequestStringsResponse()
     */
    virtual ::android::hardware::Return<void> sendRequestStrings(int32_t serial, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& data) = 0;

    using interfaceChain_cb = std::function<void(const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& descriptors)>;
    virtual ::android::hardware::Return<void> interfaceChain(interfaceChain_cb _hidl_cb) override;

    virtual ::android::hardware::Return<void> debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options) override;

    using interfaceDescriptor_cb = std::function<void(const ::android::hardware::hidl_string& descriptor)>;
    virtual ::android::hardware::Return<void> interfaceDescriptor(interfaceDescriptor_cb _hidl_cb) override;

    using getHashChain_cb = std::function<void(const ::android::hardware::hidl_vec<::android::hardware::hidl_array<uint8_t, 32>>& hashchain)>;
    virtual ::android::hardware::Return<void> getHashChain(getHashChain_cb _hidl_cb) override;

    virtual ::android::hardware::Return<void> setHALInstrumentation() override;

    virtual ::android::hardware::Return<bool> linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie) override;

    virtual ::android::hardware::Return<void> ping() override;

    using getDebugInfo_cb = std::function<void(const ::android::hidl::base::V1_0::DebugInfo& info)>;
    virtual ::android::hardware::Return<void> getDebugInfo(getDebugInfo_cb _hidl_cb) override;

    virtual ::android::hardware::Return<void> notifySyspropsChanged() override;

    virtual ::android::hardware::Return<bool> unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient) override;
    // cast static functions
    static ::android::hardware::Return<::android::sp<::android::hardware::radio::deprecated::V1_0::IOemHook>> castFrom(const ::android::sp<::android::hardware::radio::deprecated::V1_0::IOemHook>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<::android::hardware::radio::deprecated::V1_0::IOemHook>> castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError = false);

    static const char* descriptor;

    static ::android::sp<IOemHook> tryGetService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<IOemHook> tryGetService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return tryGetService(str, getStub); }
    static ::android::sp<IOemHook> tryGetService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return tryGetService(str, getStub); }
    static ::android::sp<IOemHook> tryGetService(bool getStub) { return tryGetService("default", getStub); }
    static ::android::sp<IOemHook> getService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<IOemHook> getService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return getService(str, getStub); }
    static ::android::sp<IOemHook> getService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return getService(str, getStub); }
    static ::android::sp<IOemHook> getService(bool getStub) { return getService("default", getStub); }
    __attribute__ ((warn_unused_result))::android::status_t registerAsService(const std::string &serviceName="default");
    static bool registerForNotifications(
            const std::string &serviceName,
            const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification);
};

std::string toString(const ::android::sp<::android::hardware::radio::deprecated::V1_0::IOemHook>& o);


}  // namespace V1_0
}  // namespace deprecated
}  // namespace radio
}  // namespace hardware
}  // namespace android

#endif  // HIDL_GENERATED_ANDROID_HARDWARE_RADIO_DEPRECATED_V1_0_IOEMHOOK_H
