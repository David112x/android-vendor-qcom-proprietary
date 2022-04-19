/*
Copyright (c) 2019 Qualcomm Technologies, Inc.
All Rights Reserved.
Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_4_IQTIRADIO_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_4_IQTIRADIO_H

#include <vendor/qti/hardware/radio/qtiradio/2.3/IQtiRadio.h>
#include <vendor/qti/hardware/radio/qtiradio/2.4/types.h>

#include <android/hidl/manager/1.0/IServiceNotification.h>

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace vendor {
namespace qti {
namespace hardware {
namespace radio {
namespace qtiradio {
namespace V2_4 {

struct IQtiRadio : public ::vendor::qti::hardware::radio::qtiradio::V2_3::IQtiRadio {
    /**
     * Type tag for use in template logic that indicates this is a 'pure' class.
     */
    typedef android::hardware::details::i_tag _hidl_tag;

    /**
     * Fully qualified interface name: "vendor.qti.hardware.radio.qtiradio@2.4::IQtiRadio"
     */
    static const char* descriptor;

    /**
     * Returns whether this object's implementation is outside of the current process.
     */
    virtual bool isRemote() const override { return false; }

    /**
     * Set callback for qtiradio requests and qtiradio indications
     * 
     * @param responseCallback Object containing response callback functions
     * @param indicationCallback Object containing oem hook indication callback functions
     */
    virtual ::android::hardware::Return<void> setCallback(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V1_0::IQtiRadioResponse>& responseCallback, const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V1_0::IQtiRadioIndication>& indicationCallback) = 0;

    /**
     * @param serial Serial number of request.
     * 
     * Response callback is IRadioResponse.getAtrResponse()
     */
    virtual ::android::hardware::Return<void> getAtr(int32_t serial) = 0;

    /**
     * QTIRADIO_REQUEST_ENABLE_5G
     * 
     * @param serial to match request/response. Responses must include the same token as requests.
     * 
     * Response callback is IQtiRadioResponse.enable5gResponse()
     */
    virtual ::android::hardware::Return<void> enable5g(int32_t serial) = 0;

    /**
     * QTIRADIO_REQUEST_DISABLE_5G
     * 
     * @param serial to match request/response. Responses must include the same token as requests.
     * 
     * Response callback is IQtiRadioResponse.disable5gResponse()
     */
    virtual ::android::hardware::Return<void> disable5g(int32_t serial) = 0;

    /**
     * QTIRADIO_REQUEST_ENABLE_5G_ONLY
     * 
     * @param serial to match request/response. Responses must include the same token as requests.
     * 
     * Response callback is IQtiRadioResponse.enable5gOnlyResponse()
     */
    virtual ::android::hardware::Return<void> enable5gOnly(int32_t serial) = 0;

    /**
     * QTIRADIO_REQUEST_QUERY_5G_STATUS
     * 
     * @param serial to match request/response. Responses must include the same token as requests.
     * 
     * Response callback is IQtiRadioResponse.on5gStatusResponse()
     */
    virtual ::android::hardware::Return<void> query5gStatus(int32_t serial) = 0;

    /**
     * QTIRADIO_REQUEST_QUERY_ENDC_DCNR_PARAM
     * 
     * @param serial to match request/response. Responses must include the same token as requests.
     * 
     * Response callback is IQtiRadioResponse.onNrDcParamResponse()
     */
    virtual ::android::hardware::Return<void> queryNrDcParam(int32_t serial) = 0;

    /**
     * QTIRADIO_REQUEST_QUERY_NR_BEARER_ALLOCATION
     * 
     * @param serial to match request/response. Responses must include the same token as requests.
     * 
     * Response callback is IQtiRadioResponse.onNrBearerAllocationResponse()
     */
    virtual ::android::hardware::Return<void> queryNrBearerAllocation(int32_t serial) = 0;

    /**
     * QTIRADIO_REQUEST_QUERY_NR_SIGNAL_STRENGTH
     * 
     * @param serial to match request/response. Responses must include the same token as requests.
     * 
     * Response callback is IQtiRadioResponse.onNrSignalStrengthResponse()
     */
    virtual ::android::hardware::Return<void> queryNrSignalStrength(int32_t serial) = 0;

    /**
     * Send a CDMA SMS message
     * 
     * @param serial Serial number of request.
     * @param sms Cdma Sms to be sent described by CdmaSmsMessage in types.hal
     * @param expectMore more messages are expected to be sent or not
     * 
     * Response callback is IRadioResponse.sendCdmaSmsResponse()
     */
    virtual ::android::hardware::Return<void> sendCdmaSms(int32_t serial, const ::android::hardware::radio::V1_0::CdmaSmsMessage& sms, bool expectMore) = 0;

    virtual ::android::hardware::Return<void> queryUpperLayerIndInfo(int32_t serial) = 0;

    virtual ::android::hardware::Return<void> query5gConfigInfo(int32_t serial) = 0;

    virtual ::android::hardware::Return<void> queryNrIconType(int32_t serial) = 0;

    /**
     * QTIRADIO_REQUEST_ENABLE_ENDC
     * 
     * @param serial to match request/response. Responses must include the same token as requests.
     * @param enable set to true/false
     * 
     * Response callback is IQtiRadioResponse.onEnableEndcResponse()
     */
    virtual ::android::hardware::Return<void> enableEndc(int32_t serial, bool enable) = 0;

    /**
     * QTIRADIO_REQUEST_QUERY_ENDC_STATUS
     * 
     * @param serial to match request/response. Responses must include the same token as requests.
     * 
     * Response callback is IQtiRadioResponse.onEndcStatusResponse()
     */
    virtual ::android::hardware::Return<void> queryEndcStatus(int32_t serial) = 0;

    /**
     * @param property name to get value
     * 
     * @return int value of property
     * 
     */
    virtual ::android::hardware::Return<int32_t> getPropertyValueInt(const ::android::hardware::hidl_string& prop, int32_t def) = 0;

    /**
     * @param property name to get value
     * 
     * @return boolean value of property
     * 
     */
    virtual ::android::hardware::Return<bool> getPropertyValueBool(const ::android::hardware::hidl_string& prop, bool def) = 0;

    /**
     * Return callback for getPropertyValueString
     */
    using getPropertyValueString_cb = std::function<void(const ::android::hardware::hidl_string& val)>;
    /**
     * @param property name to get value
     * 
     * @return string value of property
     * 
     */
//    virtual ::android::hardware::Return<void> getPropertyValueString(const ::android::hardware::hidl_string& prop, const ::android::hardware::hidl_string& def, getPropertyValueString_cb _hidl_cb) = 0;

    /**
     * Provide Carrier specific information to the modem that must be used to
     * encrypt the IMSI and IMPI. Sent by the framework during boot, carrier
     * switch and everytime the framework receives a new certificate.
     * 
     * @param serial Serial number of request.
     * @param message ImsiEncryptionInfo as defined in types.hal.
     * 
     * 
     * Response callback is
     * IQtiRadioResponse.setCarrierInfoForImsiEncryptionResponse()
     */
    virtual ::android::hardware::Return<void> setCarrierInfoForImsiEncryption(int32_t serial, const ::vendor::qti::hardware::radio::qtiradio::V2_4::ImsiEncryptionInfo& imsiEncryptionInfo) = 0;

    /**
     * Return callback for interfaceChain
     */
    using interfaceChain_cb = std::function<void(const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& descriptors)>;
    virtual ::android::hardware::Return<void> interfaceChain(interfaceChain_cb _hidl_cb) override;

    virtual ::android::hardware::Return<void> debug(const ::android::hardware::hidl_handle& fd, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& options) override;

    /**
     * Return callback for interfaceDescriptor
     */
    using interfaceDescriptor_cb = std::function<void(const ::android::hardware::hidl_string& descriptor)>;
    virtual ::android::hardware::Return<void> interfaceDescriptor(interfaceDescriptor_cb _hidl_cb) override;

    /**
     * Return callback for getHashChain
     */
    using getHashChain_cb = std::function<void(const ::android::hardware::hidl_vec<::android::hardware::hidl_array<uint8_t, 32>>& hashchain)>;
    virtual ::android::hardware::Return<void> getHashChain(getHashChain_cb _hidl_cb) override;

    virtual ::android::hardware::Return<void> setHALInstrumentation() override;

    virtual ::android::hardware::Return<bool> linkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient, uint64_t cookie) override;

    virtual ::android::hardware::Return<void> ping() override;

    /**
     * Return callback for getDebugInfo
     */
    using getDebugInfo_cb = std::function<void(const ::android::hidl::base::V1_0::DebugInfo& info)>;
    virtual ::android::hardware::Return<void> getDebugInfo(getDebugInfo_cb _hidl_cb) override;

    virtual ::android::hardware::Return<void> notifySyspropsChanged() override;

    virtual ::android::hardware::Return<bool> unlinkToDeath(const ::android::sp<::android::hardware::hidl_death_recipient>& recipient) override;

    // cast static functions
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_4::IQtiRadio>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_4::IQtiRadio>& parent, bool emitError = false);
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_4::IQtiRadio>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_3::IQtiRadio>& parent, bool emitError = false);
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_4::IQtiRadio>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_2::IQtiRadio>& parent, bool emitError = false);
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_4::IQtiRadio>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadio>& parent, bool emitError = false);
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_4::IQtiRadio>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_0::IQtiRadio>& parent, bool emitError = false);
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_4::IQtiRadio>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V1_0::IQtiRadio>& parent, bool emitError = false);
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_4::IQtiRadio>> castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError = false);

    // helper methods for interactions with the hwservicemanager
    /**
     * This gets the service of this type with the specified instance name. If the
     * service is currently not available or not in the VINTF manifest on a Trebilized
     * device, this will return nullptr. This is useful when you don't want to block
     * during device boot. If getStub is true, this will try to return an unwrapped
     * passthrough implementation in the same process. This is useful when getting an
     * implementation from the same partition/compilation group.
     * 
     * In general, prefer getService(std::string,bool)
     */
    static ::android::sp<IQtiRadio> tryGetService(const std::string &serviceName="default", bool getStub=false);
    /**
     * Deprecated. See tryGetService(std::string, bool)
     */
    static ::android::sp<IQtiRadio> tryGetService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return tryGetService(str, getStub); }
    /**
     * Deprecated. See tryGetService(std::string, bool)
     */
    static ::android::sp<IQtiRadio> tryGetService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return tryGetService(str, getStub); }
    /**
     * Calls tryGetService("default", bool). This is the recommended instance name for singleton services.
     */
    static ::android::sp<IQtiRadio> tryGetService(bool getStub) { return tryGetService("default", getStub); }
    /**
     * This gets the service of this type with the specified instance name. If the
     * service is not in the VINTF manifest on a Trebilized device, this will return
     * nullptr. If the service is not available, this will wait for the service to
     * become available. If the service is a lazy service, this will start the service
     * and return when it becomes available. If getStub is true, this will try to
     * return an unwrapped passthrough implementation in the same process. This is
     * useful when getting an implementation from the same partition/compilation group.
     */
    static ::android::sp<IQtiRadio> getService(const std::string &serviceName="default", bool getStub=false);
    /**
     * Deprecated. See getService(std::string, bool)
     */
    static ::android::sp<IQtiRadio> getService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return getService(str, getStub); }
    /**
     * Deprecated. See getService(std::string, bool)
     */
    static ::android::sp<IQtiRadio> getService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return getService(str, getStub); }
    /**
     * Calls getService("default", bool). This is the recommended instance name for singleton services.
     */
    static ::android::sp<IQtiRadio> getService(bool getStub) { return getService("default", getStub); }
    /**
     * Registers a service with the service manager. For Trebilized devices, the service
     * must also be in the VINTF manifest.
     */
    __attribute__ ((warn_unused_result))::android::status_t registerAsService(const std::string &serviceName="default");
    /**
     * Registers for notifications for when a service is registered.
     */
    static bool registerForNotifications(
            const std::string &serviceName,
            const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification);
};

//
// type declarations for package
//

static inline std::string toString(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_4::IQtiRadio>& o);

//
// type header definitions for package
//

static inline std::string toString(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_4::IQtiRadio>& o) {
    std::string os = "[class or subclass of ";
    os += ::vendor::qti::hardware::radio::qtiradio::V2_4::IQtiRadio::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}


}  // namespace V2_4
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

//
// global type declarations for package
//


#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_4_IQTIRADIO_H
