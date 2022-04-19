/*===========================================================================

   Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
   All Rights Reserved.
   Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_5_IIMSRADIORESPONSE_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_5_IIMSRADIORESPONSE_H

#include <vendor/qti/hardware/radio/ims/1.2/types.h>
#include <vendor/qti/hardware/radio/ims/1.4/IImsRadioResponse.h>
#include <vendor/qti/hardware/radio/ims/1.5/types.h>

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
namespace ims {
namespace V1_5 {

struct IImsRadioResponse : public ::vendor::qti::hardware::radio::ims::V1_4::IImsRadioResponse {
    /**
     * Type tag for use in template logic that indicates this is a 'pure' class.
     */
    typedef android::hardware::details::i_tag _hidl_tag;

    /**
     * Fully qualified interface name: "vendor.qti.hardware.radio.ims@1.5::IImsRadioResponse"
     */
    static const char* descriptor;

    /**
     * Returns whether this object's implementation is outside of the current process.
     */
    virtual bool isRemote() const override { return false; }

    /**
     * Response to ImsQmiIF.REQUEST_DIAL
     * 
     * @param token to match request/response. Response must include same token as request.
     * @param errorCode - errorCode of type ImsQmiIF.Error returned from RIL.
     * 
     */
    virtual ::android::hardware::Return<void> dialResponse(int32_t token, uint32_t errorCode) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_ANSWER
     * 
     * @param token Id to match request/response. Response must include same token.
     * 
     */
    virtual ::android::hardware::Return<void> answerResponse(int32_t token, uint32_t errorCode) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_HANGUP
     * 
     * @param token Id to match request/response. Response must include same token.
     * 
     */
    virtual ::android::hardware::Return<void> hangupResponse(int32_t token, uint32_t errorCode) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_IMS_REG_STATE_CHANGE
     * 
     * @param token Id to match request/response. Response must include same token.
     * 
     */
    virtual ::android::hardware::Return<void> requestRegistrationChangeResponse(int32_t token, uint32_t errorCode) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_QUERY_SERVICE_STATUS
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param srvStatusList List of service statuses.
     * 
     */
    virtual ::android::hardware::Return<void> queryServiceStatusResponse(int32_t token, uint32_t errorCode, const ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_0::ServiceStatusInfo>& srvStatusList) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_SET_SERVICE_STATUS
     * 
     * @param token Id to match request/response. Response must include same token.
     * 
     */
    virtual ::android::hardware::Return<void> setServiceStatusResponse(int32_t token, uint32_t errorCode) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_HOLD
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param sipError Sip error from the network.
     * 
     */
    virtual ::android::hardware::Return<void> holdResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo& sipError) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_RESUME
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param sipError Sip error from the network.
     * 
     */
    virtual ::android::hardware::Return<void> resumeResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo& sipError) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_SET_IMS_CONFIG
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param config Config that was set. Contains item and value.
     * 
     */
    virtual ::android::hardware::Return<void> setConfigResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::ConfigInfo& config) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_GET_IMS_CONFIG
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param config Config that was requested. Contains item and value.
     * 
     */
    virtual ::android::hardware::Return<void> getConfigResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::ConfigInfo& config) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_GET_IMS_REGISTRATION_STATE
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param registration IMS registration state.
     * 
     */
    virtual ::android::hardware::Return<void> getRegistrationResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::RegistrationInfo& registration) = 0;

    /**
     * Response to Supplementary service requests -
     *             ImsQmiIF.REQUEST_SET_CALL_FORWARD_STATUS
     *             ImsQmiIF.REQUEST_SET_CALL_WAITING
     *             ImsQmiIF.REQUEST_SUPP_SVC_STATUS
     *             ImsQmiIF.REQUEST_SET_COLR
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param errorCode - errorCode of type ImsQmiIF.Error returned from RIL.
     * @param suppServiceStatus supplementary service query status for call forwarding,
     * call waiting, etc. Has service class type enabled/disabled, facility type, error code, etc.
     * 
     */
    virtual ::android::hardware::Return<void> suppServiceStatusResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::SuppServiceStatus& suppServiceStatus) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_CONFERENCE
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param sipError Sip error from the network.
     * 
     */
    virtual ::android::hardware::Return<void> conferenceResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo& errorInfo) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_QUERY_CLIP
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param clipProvisionStatus CLIP info having status and error details
     * 
     */
    virtual ::android::hardware::Return<void> getClipResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::ClipProvisionStatus& clipProvisionStatus) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_GET_CLIR
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param clirInfo info
     * @param hasClirInfo - true or false if clirInfo has valid value or not
     * 
     */
    virtual ::android::hardware::Return<void> getClirResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::ClirInfo& clirInfo, bool hasClirInfo) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_SET_CLIR
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param errorCode - errorCode of type ImsQmiIF.Error returned from RIL.
     * 
     */
    virtual ::android::hardware::Return<void> setClirResponse(int32_t token, uint32_t errorCode) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_QUERY_COLR
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param errorCode - errorCode of type ImsQmiIF.Error returned from RIL.
     * @param colrInfo COLR Info - IP presentation and error details
     * 
     */
    virtual ::android::hardware::Return<void> getColrResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::ColrInfo& colrInfo) = 0;

    /**
     * ImsQmiIF.REQUEST_EXIT_EMERGENCY_CALLBACK_MODE
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param errorCode - errorCode of type ImsQmiIF.Error returned from RIL.
     * 
     */
    virtual ::android::hardware::Return<void> exitEmergencyCallbackModeResponse(int32_t token, uint32_t errorCode) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_DTMF
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param errorCode - errorCode of type ImsQmiIF.Error returned from RIL.
     * 
     */
    virtual ::android::hardware::Return<void> sendDtmfResponse(int32_t token, uint32_t errorCode) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_DTMF_START
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param errorCode - errorCode of type ImsQmiIF.Error returned from RIL.
     * 
     */
    virtual ::android::hardware::Return<void> startDtmfResponse(int32_t token, uint32_t errorCode) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_DTMF_STOP
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param errorCode - errorCode of type ImsQmiIF.Error returned from RIL.
     * 
     */
    virtual ::android::hardware::Return<void> stopDtmfResponse(int32_t token, uint32_t errorCode) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_SEND_UI_TTY_MODE
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param errorCode - errorCode of type ImsQmiIF.Error returned from RIL
     * 
     */
    virtual ::android::hardware::Return<void> setUiTTYModeResponse(int32_t token, uint32_t errorCode) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_MODIFY_CALL_INITIATE
     * 
     * @param token Id to match request/response. Response must include same token.
     * 
     */
    virtual ::android::hardware::Return<void> modifyCallInitiateResponse(int32_t token, uint32_t errorCode) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_MODIFY_CALL_CONFIRM
     * 
     * @param token Id to match request/response. Response must include same token.
     * 
     */
    virtual ::android::hardware::Return<void> modifyCallConfirmResponse(int32_t token, uint32_t errorCode) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_QUERY_CALL_FORWARD_STATUS
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param callInfoForwardInfoList list of call forward status information for different
     *         service classes.
     * @param errorDetails Sip error information if any.
     * 
     */
    virtual ::android::hardware::Return<void> queryCallForwardStatusResponse(int32_t token, uint32_t errorCode, const ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo>& callForwardInfoList, const ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo& errorDetails) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_QUERY_CALL_WAITING
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param errorDetails Sip error information if any.
     * 
     */
    virtual ::android::hardware::Return<void> getCallWaitingResponse(int32_t token, uint32_t errorCode, ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus serviceStatus, uint32_t serviceClass, const ::vendor::qti::hardware::radio::ims::V1_0::SipErrorInfo& errorDetails) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_EXPLICIT_CALL_TRANSFER
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param errorCode - errorCode of type ImsQmiIF.Error returned from RIL.
     * 
     */
    virtual ::android::hardware::Return<void> explicitCallTransferResponse(int32_t token, uint32_t errorCode) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_SET_SUPP_SVC_NOTIFICATION
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param status Service class status.
     * 
     */
    virtual ::android::hardware::Return<void> setSuppServiceNotificationResponse(int32_t token, uint32_t errorCode, ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus serviceStatus) = 0;

    /**
     * ImsQmiIF.REQUEST_GET_RTP_STATISTICS
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param errorCode - errorCode of type ImsQmiIF.Error returned from RIL.
     * @param packetCount total number of packets sent or received in sample duration.
     * 
     */
    virtual ::android::hardware::Return<void> getRtpStatisticsResponse(int32_t token, uint32_t errorCode, uint64_t packetCount) = 0;

    /**
     * ImsQmiIF.REQUEST_GET_RTP_ERROR_STATISTICS
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param errorCode - errorCode of type ImsQmiIF.Error returned from RIL.
     * @param packetErrorCount total number of packets errors encountered in sample duration.
     * 
     */
    virtual ::android::hardware::Return<void> getRtpErrorStatisticsResponse(int32_t token, uint32_t errorCode, uint64_t packetErrorCount) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_ADD_PARTICIPANT
     * 
     * @param token to match request/response. Response must include same token as request.
     * @param errorCode - errorCode of type ImsQmiIF.Error returned from RIL.
     * 
     */
    virtual ::android::hardware::Return<void> addParticipantResponse(int32_t token, uint32_t errorCode) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_DEFLECT_CALL
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param errorCode - errorCode of type ImsQmiIF.Error returned from RIL.
     * 
     */
    virtual ::android::hardware::Return<void> deflectCallResponse(int32_t token, uint32_t errorCode) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_SEND_GEOLOCATION_INFO
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param errorCode - errorCode of type ImsQmiIF.Error returned from RIL.
     * 
     */
    virtual ::android::hardware::Return<void> sendGeolocationInfoResponse(int32_t token, uint32_t errorCode) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_GET_IMS_SUB_CONFIG
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param errorCode - errorCode of type ImsQmiIF.Error returned from RIL.
     * @param ImsSubConfigInfo, provided information of Ims stack static support
     * 
     */
    virtual ::android::hardware::Return<void> getImsSubConfigResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_0::ImsSubConfigInfo& subConfigInfo) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_SEND_RTT_MSG
     * 
     * @param token to match request/response. Response must include same token as request.
     * @param errorCode - errorCode of type ImsQmiIF.Error returned from RIL.
     * 
     */
    virtual ::android::hardware::Return<void> sendRttMessageResponse(int32_t token, uint32_t errorCode) = 0;

    /**
     * Response to ImsQmiIF.REQUEST_CANCEL_MODIFY_CALL
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param errorCode - errorCode of type ImsQmiIF.Error returned from RIL.
     * 
     */
    virtual ::android::hardware::Return<void> cancelModifyCallResponse(int32_t token, uint32_t errorCode) = 0;

    /**
     * Response to MessageId.REQUEST_SEND_IMS_SMS.
     * Pass the result of the sent message.
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param msgeRef the message reference.
     * @param smsStatus status result of sending the sms. Valid values are defined
     * in types.hal.
     * @param reason reason in case status is failure. Valid values are defined in
     * types.hal.
     * 
     */
    virtual ::android::hardware::Return<void> sendImsSmsResponse(int32_t token, uint32_t msgeRef, ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult smsStatus, ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendFailureReason reason) = 0;

    /**
     * Response to Supplementary service requests -
     *             MessageId.REQUEST_SET_CALL_FORWARD_STATUS
     *             MessageId.REQUEST_SET_CALL_WAITING
     *             MessageId.REQUEST_SUPP_SVC_STATUS
     *             MessageId.REQUEST_SET_COLR
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param errorCode - errorCode of type ImsErrorCode.Error returned from RIL.
     * @param suppServiceStatus supplementary service query status for call forwarding,
     * call waiting, etc. Has service class type enabled/disabled, facility type, error code, etc.
     * 
     */
    virtual ::android::hardware::Return<void> suppServiceStatusResponse_1_3(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_3::SuppServiceStatus& suppServiceStatus) = 0;

    /**
     * Response to MessageId.REQUEST_QUERY_COLR
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param errorCode - errorCode of type ImsErrorCode.Error returned from RIL.
     * @param colrInfo COLR Info - IP presentation and error details
     * 
     */
    virtual ::android::hardware::Return<void> getColrResponse_1_3(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_3::ColrInfo& colrInfo) = 0;

    /**
     * Response to MessageId.REQUEST_REGISTER_MULTI_IDENTITY_LINES
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param errorCode - errorCode of type ImsErrorCode.Error returned from RIL.
     */
    virtual ::android::hardware::Return<void> registerMultiIdentityLinesResponse(int32_t token, uint32_t errorCode) = 0;

    /**
     * Response to MessageId.REQUEST_QUERY_VIRTUAL_LINE_INFO
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param msisdn - msisdn of the line for which info is being queried.
     * @param virtualLineInfo - the virtual line info of the msisdn
     */
    virtual ::android::hardware::Return<void> queryVirtualLineInfoResponse(int32_t token, const ::android::hardware::hidl_string& msisdn, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& virtualLineInfo) = 0;

    /**
     * Response to MessageId.REQUEST_SET_CALL_FORWARD_STATUS
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param errorCode - errorCode of type ImsErrorCode.Error returned from RIL.
     * @param CallForwardStatus - status and error details for all call forwarding requests
     *        (CFB, CFU, CFNRy, CFNRc, CFUT, CF_ALL and CF_ALL_CONDITIONAL)
     * 
     */
    virtual ::android::hardware::Return<void> setCallForwardStatusResponse(int32_t token, uint32_t errorCode, const ::vendor::qti::hardware::radio::ims::V1_5::CallForwardStatusInfo& callForwardStatus) = 0;

    virtual ::android::hardware::Return<void> sendImsSmsResponse_1_5(int32_t token, uint32_t msgRef, ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsSendStatusResult smsStatus, ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsSendFailureReason reason) = 0;

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
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_5::IImsRadioResponse>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_5::IImsRadioResponse>& parent, bool emitError = false);
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_5::IImsRadioResponse>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_4::IImsRadioResponse>& parent, bool emitError = false);
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_5::IImsRadioResponse>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_3::IImsRadioResponse>& parent, bool emitError = false);
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_5::IImsRadioResponse>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_2::IImsRadioResponse>& parent, bool emitError = false);
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_5::IImsRadioResponse>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadioResponse>& parent, bool emitError = false);
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_5::IImsRadioResponse>> castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError = false);

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
    static ::android::sp<IImsRadioResponse> tryGetService(const std::string &serviceName="default", bool getStub=false);
    /**
     * Deprecated. See tryGetService(std::string, bool)
     */
    static ::android::sp<IImsRadioResponse> tryGetService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return tryGetService(str, getStub); }
    /**
     * Deprecated. See tryGetService(std::string, bool)
     */
    static ::android::sp<IImsRadioResponse> tryGetService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return tryGetService(str, getStub); }
    /**
     * Calls tryGetService("default", bool). This is the recommended instance name for singleton services.
     */
    static ::android::sp<IImsRadioResponse> tryGetService(bool getStub) { return tryGetService("default", getStub); }
    /**
     * This gets the service of this type with the specified instance name. If the
     * service is not in the VINTF manifest on a Trebilized device, this will return
     * nullptr. If the service is not available, this will wait for the service to
     * become available. If the service is a lazy service, this will start the service
     * and return when it becomes available. If getStub is true, this will try to
     * return an unwrapped passthrough implementation in the same process. This is
     * useful when getting an implementation from the same partition/compilation group.
     */
    static ::android::sp<IImsRadioResponse> getService(const std::string &serviceName="default", bool getStub=false);
    /**
     * Deprecated. See getService(std::string, bool)
     */
    static ::android::sp<IImsRadioResponse> getService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return getService(str, getStub); }
    /**
     * Deprecated. See getService(std::string, bool)
     */
    static ::android::sp<IImsRadioResponse> getService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return getService(str, getStub); }
    /**
     * Calls getService("default", bool). This is the recommended instance name for singleton services.
     */
    static ::android::sp<IImsRadioResponse> getService(bool getStub) { return getService("default", getStub); }
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

static inline std::string toString(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_5::IImsRadioResponse>& o);

//
// type header definitions for package
//

static inline std::string toString(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_5::IImsRadioResponse>& o) {
    std::string os = "[class or subclass of ";
    os += ::vendor::qti::hardware::radio::ims::V1_5::IImsRadioResponse::descriptor;
    os += "]";
    os += o->isRemote() ? "@remote" : "@local";
    return os;
}


}  // namespace V1_5
}  // namespace ims
}  // namespace radio
}  // namespace hardware
}  // namespace qti
}  // namespace vendor

//
// global type declarations for package
//


#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_5_IIMSRADIORESPONSE_H
