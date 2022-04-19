/*===========================================================================
 *
 *    Copyright (c) 2019 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *===========================================================================*/

#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_3_IQTIRADIORESPONSE_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_3_IQTIRADIORESPONSE_H

#include <vendor/qti/hardware/radio/qtiradio/2.0/types.h>
#include <vendor/qti/hardware/radio/qtiradio/2.2/IQtiRadioResponse.h>
#include <vendor/qti/hardware/radio/qtiradio/2.3/types.h>

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
namespace V2_3 {

struct IQtiRadioResponse : public ::vendor::qti::hardware::radio::qtiradio::V2_2::IQtiRadioResponse {
    /**
     * Type tag for use in template logic that indicates this is a 'pure' class.
     */
    typedef android::hardware::details::i_tag _hidl_tag;

    /**
     * Fully qualified interface name: "vendor.qti.hardware.radio.qtiradio@2.3::IQtiRadioResponse"
     */
    static const char* descriptor;

    /**
     * Returns whether this object's implementation is outside of the current process.
     */
    virtual bool isRemote() const override { return false; }

    /**
     * @param info Response info struct containing response type, serial no. and error
     * @param atr String containing the SIM ATR (Answer To Reset; as per ISO/IEC 7816-4)
     * 
     * Valid errors returned:
     *   RadioError:NONE
     *   RadioError:RADIO_NOT_AVAILABLE
     *   RadioError:RIL_E_REQUEST_NOT_SUPPORTED
     */
    virtual ::android::hardware::Return<void> getAtrResponse(const ::vendor::qti::hardware::radio::qtiradio::V1_0::QtiRadioResponseInfo& info, const ::android::hardware::hidl_string& atr) = 0;

    /**
     * Response to IQtiRadio.enable5g
     * 
     * @param serial to match request/response. Response must inclue same serial as request.
     * @param errorCode - errorCode as per types.hal returned from RIL.
     * @param status SUCCESS/FAILURE of the request.
     */
    virtual ::android::hardware::Return<void> onEnable5gResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_0::Status status) = 0;

    /**
     * Response to IQtiRadio.disable5g
     * 
     * @param serial to match request/response. Response must inclue same serial as request.
     * @param errorCode - errorCode as per types.hal returned from RIL.
     * @param status SUCCESS/FAILURE of the request.
     */
    virtual ::android::hardware::Return<void> onDisable5gResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_0::Status status) = 0;

    /**
     * Response to IQtiRadio.enable5gOnly
     * 
     * @param serial to match request/response. Response must inclue same serial as request.
     * @param errorCode - errorCode as per types.hal returned from RIL.
     * @param status SUCCESS/FAILURE of the request.
     */
    virtual ::android::hardware::Return<void> onEnable5gOnlyResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_0::Status status) = 0;

    /**
     * Response to IQtiRadio.query5gStatus
     * 
     * @param serial to match request/response. Response must inclue same serial as request.
     * @param errorCode - errorCode as per types.hal returned from RIL.
     * @param enabled values as per types.hal to indicate status of 5g in NSA or SA mode.
     */
    virtual ::android::hardware::Return<void> on5gStatusResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_0::EnableStatus enabled) = 0;

    /**
     * Response to IQtiRadio.queryNrDcParam
     * 
     * @param serial to match request/response. Response must inclue same serial as request.
     * @param errorCode - errorCode as per types.hal returned from RIL.
     * @param dcParam info about EN-DC and restrict-DCNR..
     */
    virtual ::android::hardware::Return<void> onNrDcParamResponse(int32_t serial, uint32_t errorCode, const ::vendor::qti::hardware::radio::qtiradio::V2_0::DcParam& dcParam) = 0;

    /**
     * Response to IQtiRadio.queryNrBearerAllocation
     * 
     * @param serial to match request/response. Response must inclue same serial as request.
     * @param errorCode - errorCode as per types.hal returned from RIL.
     * @param bearerStatus values as per types.hal to indicate status of 5g bearer allocation..
     */
    virtual ::android::hardware::Return<void> onNrBearerAllocationResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_0::BearerStatus bearerStatus) = 0;

    /**
     * Response to IQtiRadio.queryNrSignalStrength
     * 
     * @param serial to match request/response. Response must inclue same serial as request.
     * @param errorCode - errorCode as per types.hal returned from RIL.
     * @param signalStrength values as per types.hal to indicate 5g signal strength parameters.
     */
    virtual ::android::hardware::Return<void> onSignalStrengthResponse(int32_t serial, uint32_t errorCode, const ::vendor::qti::hardware::radio::qtiradio::V2_0::SignalStrength& signalStrength) = 0;

    /**
     * @param info Response info struct containing response type, serial no. and error
     * @param sms Sms result struct as defined by SendSmsResult in types.hal
     * 
     * Valid errors returned:
     *   RadioError:NONE
     *   RadioError:RADIO_NOT_AVAILABLE
     *   RadioError:INVALID_ARGUMENTS
     *   RadioError:SMS_SEND_FAIL_RETRY
     *   RadioError:NETWORK_REJECT
     *   RadioError:INVALID_STATE
     *   RadioError:NO_MEMORY
     *   RadioError:REQUEST_RATE_LIMITED
     *   RadioError:INVALID_SMS_FORMAT
     *   RadioError:SYSTEM_ERR
     *   RadioError:FDN_CHECK_FAILURE
     *   RadioError:MODEM_ERR
     *   RadioError:NETWORK_ERR
     *   RadioError:ENCODING_ERR
     *   RadioError:INVALID_SMSC_ADDRESS
     *   RadioError:INTERNAL_ERR
     *   RadioError:SYSTEM_ERR
     *   RadioError:REQUEST_NOT_SUPPORTED
     *   RadioError:OPERATION_NOT_ALLOWED
     *   RadioError:ENCODING_ERR
     *   RadioError:NO_RESOURCES
     *   RadioError:CANCELLED
     *   RadioError:SIM_ABSENT
     */
    virtual ::android::hardware::Return<void> sendCdmaSmsResponse(const ::vendor::qti::hardware::radio::qtiradio::V1_0::QtiRadioResponseInfo& info, const ::android::hardware::radio::V1_0::SendSmsResult& sms) = 0;

    virtual ::android::hardware::Return<void> onUpperLayerIndInfoResponse(int32_t serial, uint32_t errorCode, const ::vendor::qti::hardware::radio::qtiradio::V2_1::UpperLayerIndInfo& uliInfo) = 0;

    /**
     * Response to IQtiRadio.queryNrBearerAllocation
     * 
     * @param serial to match request/response. Response must inclue same serial as request.
     * @param errorCode - errorCode as per types.hal returned from RIL.
     * @param bearerStatus values as per types.hal to indicate status of 5g bearer allocation..
     */
    virtual ::android::hardware::Return<void> onNrBearerAllocationResponse_2_1(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_1::BearerStatus bearerStatus) = 0;

    /**
     * Response to IQtiRadio.queryNrBearerAllocation
     * 
     * @param serial to match request/response. Response must inclue same serial as request.
     * @param errorCode - errorCode as per types.hal returned from RIL.
     * @param config values as per types.hal to indicate status of 5g configuration type NSA/SA.
     */
    virtual ::android::hardware::Return<void> on5gConfigInfoResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_1::ConfigType config) = 0;

    /**
     * Response to IQtiRadio.queryNrIconType
     * 
     * @param serial to match request/response. Response must include same serial as request.
     * @param errorCode - errorCode as per types.hal returned from RIL.
     * @param NrIconType as per types.hal to indicate 5G icon - NONE(Non-5G) or
     *        5G BASIC or 5G UWB shown on the UI.
     */
    virtual ::android::hardware::Return<void> onNrIconTypeResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_2::NrIconType iconType) = 0;

    /**
     * Response to IQtiRadio.enableEndc
     * 
     * @param serial to match request/response. Response must inclue same serial as request.
     * @param errorCode - errorCode as per types.hal returned from RIL.
     * @param status SUCCESS/FAILURE of the request.
     */
    virtual ::android::hardware::Return<void> onEnableEndcResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_0::Status status) = 0;

    /**
     * Response to IQtiRadio.queryEndcStatus
     * 
     * @param serial to match request/response. Response must inclue same serial as request.
     * @param errorCode - errorCode as per types.hal returned from RIL.
     * @param endcStatus values as per types.hal to indicate ENDC is enabled/disabled.
     */
    virtual ::android::hardware::Return<void> onEndcStatusResponse(int32_t serial, uint32_t errorCode, ::vendor::qti::hardware::radio::qtiradio::V2_3::EndcStatus endcStatus) = 0;

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
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_3::IQtiRadioResponse>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_3::IQtiRadioResponse>& parent, bool emitError = false);
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_3::IQtiRadioResponse>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_2::IQtiRadioResponse>& parent, bool emitError = false);
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_3::IQtiRadioResponse>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_1::IQtiRadioResponse>& parent, bool emitError = false);
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_3::IQtiRadioResponse>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_0::IQtiRadioResponse>& parent, bool emitError = false);
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_3::IQtiRadioResponse>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V1_0::IQtiRadioResponse>& parent, bool emitError = false);
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_3::IQtiRadioResponse>> castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError = false);

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
    static ::android::sp<IQtiRadioResponse> tryGetService(const std::string &serviceName="default", bool getStub=false);
    /**
     * Deprecated. See tryGetService(std::string, bool)
     */
    static ::android::sp<IQtiRadioResponse> tryGetService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return tryGetService(str, getStub); }
    /**
     * Deprecated. See tryGetService(std::string, bool)
     */
    static ::android::sp<IQtiRadioResponse> tryGetService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return tryGetService(str, getStub); }
    /**
     * Calls tryGetService("default", bool). This is the recommended instance name for singleton services.
     */
    static ::android::sp<IQtiRadioResponse> tryGetService(bool getStub) { return tryGetService("default", getStub); }
    /**
     * This gets the service of this type with the specified instance name. If the
     * service is not in the VINTF manifest on a Trebilized device, this will return
     * nullptr. If the service is not available, this will wait for the service to
     * become available. If the service is a lazy service, this will start the service
     * and return when it becomes available. If getStub is true, this will try to
     * return an unwrapped passthrough implementation in the same process. This is
     * useful when getting an implementation from the same partition/compilation group.
     */
    static ::android::sp<IQtiRadioResponse> getService(const std::string &serviceName="default", bool getStub=false);
    /**
     * Deprecated. See getService(std::string, bool)
     */
    static ::android::sp<IQtiRadioResponse> getService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return getService(str, getStub); }
    /**
     * Deprecated. See getService(std::string, bool)
     */
    static ::android::sp<IQtiRadioResponse> getService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return getService(str, getStub); }
    /**
     * Calls getService("default", bool). This is the recommended instance name for singleton services.
     */
    static ::android::sp<IQtiRadioResponse> getService(bool getStub) { return getService("default", getStub); }
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

static inline std::string toString(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_3::IQtiRadioResponse>& o);

//
// type header definitions for package
//

static inline std::string toString(const ::android::sp<::vendor::qti::hardware::radio::qtiradio::V2_3::IQtiRadioResponse>& o) {
    std::string os = "[class or subclass of ";
    os += ::vendor::qti::hardware::radio::qtiradio::V2_3::IQtiRadioResponse::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}


}  // namespace V2_3
}  // namespace qtiradio
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

//
// global type declarations for package
//


#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_QTIRADIO_V2_3_IQTIRADIORESPONSE_H
