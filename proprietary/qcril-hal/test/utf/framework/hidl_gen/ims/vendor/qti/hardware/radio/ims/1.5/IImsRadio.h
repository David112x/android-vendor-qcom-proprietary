/*===========================================================================

   Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
   All Rights Reserved.
   Confidential and Proprietary - Qualcomm Technologies, Inc.

===========================================================================*/

#ifndef HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_5_IIMSRADIO_H
#define HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_5_IIMSRADIO_H

#include <vendor/qti/hardware/radio/ims/1.4/IImsRadio.h>
#include <vendor/qti/hardware/radio/ims/1.4/types.h>
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

struct IImsRadio : public ::vendor::qti::hardware::radio::ims::V1_4::IImsRadio {
    /**
     * Type tag for use in template logic that indicates this is a 'pure' class.
     */
    typedef android::hardware::details::i_tag _hidl_tag;

    /**
     * Fully qualified interface name: "vendor.qti.hardware.radio.ims@1.5::IImsRadio"
     */
    static const char* descriptor;

    /**
     * Returns whether this object's implementation is outside of the current process.
     */
    virtual bool isRemote() const override { return false; }

    /**
     * Set callback that has response functions for the requests send from ImsService
     * 
     * @param imsRadioResponse Object containing the call backs for requests like DIAL, etc.
     * @param imsRadioIndication Object containing the call backs for UNSOLs
     */
    virtual ::android::hardware::Return<void> setCallback(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadioResponse>& imsRadioResponse, const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadioIndication>& imsRadioIndication) = 0;

    /**
     * ImsQmiIF.REQUEST_DIAL
     * 
     * @param token to match request/response. Responses must include the same token as requests.
     * @param DailRequest - the struct containing the dial request params like address, clir
     * call details, etc.
     * 
     */
    virtual ::android::hardware::Return<void> dial(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::DialRequest& dialRequest) = 0;

    /**
     * ImsQmiIF.REQUEST_ADD_PARTICIPANT
     * 
     * @param token to match request/response. Responses must include the same token as requests.
     * @param DailRequest - the struct containing the dial request params like address, clir
     * call details, etc.
     * 
     */
    virtual ::android::hardware::Return<void> addParticipant(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::DialRequest& dialRequest) = 0;

    /**
     * ImsQmiIF.REQUEST_IMS_REGISTRATION_STATE
     * 
     * @param token Id to match request/response. Response must include same token.
     * 
     */
    virtual ::android::hardware::Return<void> getImsRegistrationState(int32_t token) = 0;

    /**
     * ImsQmiIF.REQUEST_ANSWER
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param callType
     * @param IpPresentation
     * @param mode Rtt Mode
     * 
     */
    virtual ::android::hardware::Return<void> answer(int32_t token, ::vendor::qti::hardware::radio::ims::V1_0::CallType callType, ::vendor::qti::hardware::radio::ims::V1_0::IpPresentation presentation, ::vendor::qti::hardware::radio::ims::V1_0::RttMode mode) = 0;

    /**
     * ImsQmiIF.REQUEST_HANGUP
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param hangup Payload for the hangup request. It consists of connection index,
     *               is_multiparty, connetion URI, conference Id, fail cause.
     * 
     */
    virtual ::android::hardware::Return<void> hangup(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::HangupRequestInfo& hangup) = 0;

    /**
     * ImsQmiIF.REQUEST_IMS_REG_STATE_CHANGE
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param state New registration state requested.
     * 
     */
    virtual ::android::hardware::Return<void> requestRegistrationChange(int32_t token, ::vendor::qti::hardware::radio::ims::V1_0::RegState state) = 0;

    /**
     * ImsQmiIF.REQUEST_IMS_QUERY_SERVICE_STATUS
     * 
     * @param token Id to match request/response. Response must include same token.
     * 
     */
    virtual ::android::hardware::Return<void> queryServiceStatus(int32_t token) = 0;

    /**
     * ImsQmiIF.REQUEST_IMS_SET_SERVICE_STATUS
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param srvStatusInfo New service state requested.
     * 
     */
    virtual ::android::hardware::Return<void> setServiceStatus(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::ServiceStatusInfo& srvStatusInfo) = 0;

    /**
     * ImsQmiIF.REQUEST_HOLD
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param callId Id of the call to be held.
     * 
     */
    virtual ::android::hardware::Return<void> hold(int32_t token, uint32_t callId) = 0;

    /**
     * ImsQmiIF.REQUEST_RESUME
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param callId Id of the call to be resumed.
     * 
     */
    virtual ::android::hardware::Return<void> resume(int32_t token, uint32_t callId) = 0;

    /**
     * ImsQmiIF.REQUEST_SET_IMS_CONFIG
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param config Config to be set. Contains item and value.
     * 
     */
    virtual ::android::hardware::Return<void> setConfig(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::ConfigInfo& config) = 0;

    /**
     * ImsQmiIF.REQUEST_GET_IMS_CONFIG
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param config Config requested. Contains item and value.
     * 
     */
    virtual ::android::hardware::Return<void> getConfig(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::ConfigInfo& config) = 0;

    /**
     * ImsQmiIF.REQUEST_CONFERENCE
     * 
     * @param token Id to match request/response. Response must include same token.
     * 
     */
    virtual ::android::hardware::Return<void> conference(int32_t token) = 0;

    /**
     * ImsQmiIF.REQUEST_QUERY_CLIP
     * 
     * @param token Id to match request/response. Response must include same token.
     * 
     */
    virtual ::android::hardware::Return<void> getClip(int32_t token) = 0;

    /**
     * ImsQmiIF.REQUEST_QUERY_CLIR
     * 
     * @param token Id to match request/response. Response must include same token.
     * 
     */
    virtual ::android::hardware::Return<void> getClir(int32_t token) = 0;

    /**
     * ImsQmiIF.REQUEST_SET_CLIR
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param clirInfo CLIR info to set
     * 
     */
    virtual ::android::hardware::Return<void> setClir(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::ClirInfo& clirInfo) = 0;

    /**
     * ImsQmiIF.REQUEST_GET_COLR
     * 
     * @param token Id to match request/response. Response must include same token.
     * 
     */
    virtual ::android::hardware::Return<void> getColr(int32_t token) = 0;

    /**
     * ImsQmiIF.REQUEST_SET_COLR
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param colrInfo COLR Info - IP presentation and error details to be set
     * 
     */
    virtual ::android::hardware::Return<void> setColr(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::ColrInfo& colrInfo) = 0;

    /**
     * ImsQmiIF.REQUEST_EXIT_EMERGENCY_CALLBACK_MODE
     * 
     * @param token Id to match request/response. Response must include same token.
     * 
     */
    virtual ::android::hardware::Return<void> exitEmergencyCallbackMode(int32_t token) = 0;

    /**
     * ImsQmiIF.REQUEST_DTMF
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param dtmfInfo DTMF tones to send
     * 
     */
    virtual ::android::hardware::Return<void> sendDtmf(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::DtmfInfo& dtmfInfo) = 0;

    /**
     * ImsQmiIF.REQUEST_DTMF_START
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param dtmfInfo DTMF tones to send
     * 
     */
    virtual ::android::hardware::Return<void> startDtmf(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::DtmfInfo& dtmfInfo) = 0;

    /**
     * ImsQmiIF.REQUEST_DTMF_STOP
     * 
     * @param token Id to match request/response. Response must include same token.
     * 
     */
    virtual ::android::hardware::Return<void> stopDtmf(int32_t token) = 0;

    /**
     * ImsQmiIF.REQUEST_SEND_UI_TTY_MODE
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param ttyInfo TTY info to set having TTY mode and additional info as bytes
     * 
     */
    virtual ::android::hardware::Return<void> setUiTtyMode(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::TtyInfo& ttyInfo) = 0;

    /**
     * ImsQmiIF.REQUEST_MODIFY_CALL_INITIATE
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param CallModifyInfo new call modify type requested
     */
    virtual ::android::hardware::Return<void> modifyCallInitiate(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyInfo& callModifyInfo) = 0;

    /**
     * ImsQmiIF.REQUEST_MODIFY_CALL_CONFIRM
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param CallModifyInfo new call modify type confirmed
     */
    virtual ::android::hardware::Return<void> modifyCallConfirm(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::CallModifyInfo& callModifyInfo) = 0;

    /**
     * ImsQmiIF.REQUEST_QUERY_CALL_FORWARD_STATUS
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param callForwardInfo Call forward query details.
     * 
     */
    virtual ::android::hardware::Return<void> queryCallForwardStatus(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo& callForwardInfo) = 0;

    /**
     * ImsQmiIF.REQUEST_SET_CALL_FORWARD_STATUS
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param callForwardInfo Set call forward request details.
     * 
     */
    virtual ::android::hardware::Return<void> setCallForwardStatus(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::CallForwardInfo& callForwardInfo) = 0;

    /**
     * ImsQmiIF.REQUEST_QUERY_CALL_WAITING
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param serviceClass serviceClass on which the call waiting is set.
     * 
     */
    virtual ::android::hardware::Return<void> getCallWaiting(int32_t token, uint32_t serviceClass) = 0;

    /**
     * ImsQmiIF.REQUEST_SET_CALL_WAITING
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param serviceClass serviceClass on which the call waiting is set.
     * 
     */
    virtual ::android::hardware::Return<void> setCallWaiting(int32_t token, ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus serviceStatus, uint32_t serviceClass) = 0;

    /**
     * ImsQmiIF.REQUEST_SET_SUPP_SVC_NOTIFICATION
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param status Service class status.
     * 
     */
    virtual ::android::hardware::Return<void> setSuppServiceNotification(int32_t token, ::vendor::qti::hardware::radio::ims::V1_0::ServiceClassStatus status) = 0;

    /**
     * ImsQmiIF.REQUEST_EXPLICIT_CALL_TRANSFER
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param ectInfo Payload for the call transfer request.
     * 
     */
    virtual ::android::hardware::Return<void> explicitCallTransfer(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::ExplicitCallTransferInfo& ectInfo) = 0;

    /**
     * ImsQmiIF.REQUEST_SUPP_SVC_STATUS
     * 
     * @param token Id to match request/response. Response must include same token.
     * 
     */
    virtual ::android::hardware::Return<void> suppServiceStatus(int32_t token, int32_t operationType, ::vendor::qti::hardware::radio::ims::V1_0::FacilityType facilityType, const ::vendor::qti::hardware::radio::ims::V1_0::CbNumListInfo& cbNumListInfo) = 0;

    /**
     * ImsQmiIF.REQUEST_GET_RTP_STATISTICS
     * 
     * @param token Id to match request/response. Response must include same token.
     * 
     */
    virtual ::android::hardware::Return<void> getRtpStatistics(int32_t token) = 0;

    /**
     * ImsQmiIF.REQUEST_GET_RTP_ERROR_STATISTICS
     * 
     * @param token Id to match request/response. Response must include same token.
     * 
     */
    virtual ::android::hardware::Return<void> getRtpErrorStatistics(int32_t token) = 0;

    /**
     * ImsQmiIF.REQUEST_DEFLECT_CALL
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param deflectRequestInfo Payload for the call deflect request.
     * 
     */
    virtual ::android::hardware::Return<void> deflectCall(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_0::DeflectRequestInfo& deflectRequestInfo) = 0;

    /**
     * ImsQmiIF.REQUEST_SEND_GEOLOCATION_INFO
     * 
     * @param token to match request/response. Responses must include the same token as requests.
     * @param lat - Latitude location coordinate
     * @param lon - Longitude location coordinate
     * @param addesssInfo - the struct containing the reverse-geocoded address information.
     * 
     */
    virtual ::android::hardware::Return<void> sendGeolocationInfo(int32_t token, double lat, double lon, const ::vendor::qti::hardware::radio::ims::V1_0::AddressInfo& addressInfo) = 0;

    /**
     * ImsQmiIF.REQUEST_GET_IMS_SUB_CONFIG
     * 
     * @param token Id to match request/response. Response must include same token.
     * 
     */
    virtual ::android::hardware::Return<void> getImsSubConfig(int32_t token) = 0;

    /**
     * ImsQmiIF.REQUEST_SEND_RTT_MSG
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param  message Rtt text message
     * 
     */
    virtual ::android::hardware::Return<void> sendRttMessage(int32_t token, const ::android::hardware::hidl_string& message) = 0;

    /**
     * ImsQmiIF.REQUEST_CANCEL_MODIFY_CALL
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param callId Id of the call to be cancelled modify.
     * 
     */
    virtual ::android::hardware::Return<void> cancelModifyCall(int32_t token, uint32_t callId) = 0;

    /**
     * ImsQmiIF.REQUEST_HANGUP
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param hangup Payload for the hangup request. It consists of connection index,
     *               is_multiparty, connetion URI, conference Id, fail cause.
     * 
     */
    virtual ::android::hardware::Return<void> hangup_1_1(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_1::HangupRequestInfo& hangup) = 0;

    /**
     * MessageId.REQUEST_HANGUP
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param hangup Payload for the hangup request. It consists of connection index,
     *              is_multiparty, connetion URI, conference Id, fail cause.
     * 
     */
    virtual ::android::hardware::Return<void> hangup_1_2(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_2::HangupRequestInfo& hangup) = 0;

    /**
     * MessageId.REQUEST_SEND_IMS_SMS
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param imsSms ImsSmsMessage as defined in types.hal
     * 
     * Response function is IImsRadioResponse.sendImsSmsResponse()
     * 
     */
    virtual ::android::hardware::Return<void> sendImsSms(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsMessage& imsSms) = 0;

    /**
     * MessageId.REQUEST_ACKNOWLEDGE_IMS_SMS
     * 
     * @param token Id to match request/response. Response must include same token.
     * @messageRef the message reference or -1 of unavailable.
     * @smsResult result of delivering the message. Valid values are defined in types.hal.
     * 
     */
    virtual ::android::hardware::Return<void> acknowledgeSms(int32_t token, uint32_t messageRef, ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsDeliverStatusResult smsResult) = 0;

    /**
     * MessageId.REQUEST_ACKNOWLEDGE_IMS_SMS_REPORT
     * 
     * @param token Id to match request/response. Response must include same token.
     * @messageRef the message reference or -1 of unavailable.
     * @smsResult result of delivering the message. Valid values are defined in types.hal.
     * 
     */
    virtual ::android::hardware::Return<void> acknowledgeSmsReport(int32_t token, uint32_t messageRef, ::vendor::qti::hardware::radio::ims::V1_2::ImsSmsStatusReportResult smsReport) = 0;

    /**
     * Return callback for getSmsFormat
     */
    using getSmsFormat_cb = std::function<void(const ::android::hardware::hidl_string& format)>;
    /**
     * MessageId.REQUEST_IMS_SMS_FORMAT
     * 
     * @return the format of the message. Valid values are {SmsMessage#FORMAT_3GPP} and
     * {SmsMessage#FORMAT_3GPP2}.
     * 
     */
    virtual ::android::hardware::Return<void> getSmsFormat(getSmsFormat_cb _hidl_cb) = 0;

    /**
     * MessageId.REQUEST_SEND_GEOLOCATION_INFO
     * 
     * @param token to match request/response. Responses must include the same token as requests.
     * @param lat - Latitude location coordinate
     * @param lon - Longitude location coordinate
     * @param addesssInfo - the struct containing the reverse-geocoded address information.
     * 
     */
    virtual ::android::hardware::Return<void> sendGeolocationInfo_1_2(int32_t token, double lat, double lon, const ::vendor::qti::hardware::radio::ims::V1_2::AddressInfo& addressInfo) = 0;

    /**
     * MessageId.REQUEST_HANGUP
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param hangup Payload for the hangup request. It consists of connection index,
     *               is_multiparty, connetion URI, conference Id, fail cause.
     * 
     */
    virtual ::android::hardware::Return<void> hangup_1_3(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_3::HangupRequestInfo& hangup) = 0;

    /**
     * MessageId.REQUEST_SET_COLR
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param colrInfo COLR Info - IP presentation and error details to be set
     * 
     */
    virtual ::android::hardware::Return<void> setColr_1_3(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_3::ColrInfo& colrInfo) = 0;

    /**
     * MessageId.REQUEST_REGISTER_MULTI_IDENTITY_LINES
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param info List of MultiIdentityLineInfo.
     */
    virtual ::android::hardware::Return<void> registerMultiIdentityLines(int32_t token, const ::android::hardware::hidl_vec<::vendor::qti::hardware::radio::ims::V1_4::MultiIdentityLineInfoHal>& info) = 0;

    /**
     * MessageId.REQUEST_QUERY_VIRTUAL_LINE_INFO
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param msisdn - msisdn of the line for which the virtual line info is being queried.
     */
    virtual ::android::hardware::Return<void> queryVirtualLineInfo(int32_t token, const ::android::hardware::hidl_string& msisdn) = 0;

    /**
     * MessageId.REQUEST_DIAL
     * 
     * @param token to match request/response. Responses must include the same token as requests.
     * @param DailRequest - the struct containing the dial request params like address, clir
     * call details, etc.
     * 
     */
    virtual ::android::hardware::Return<void> dial_1_4(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_4::DialRequest& dialRequest) = 0;

    /**
     * MessageId.REQUEST_ADD_PARTICIPANT
     * 
     * @param token to match request/response. Responses must include the same token as requests.
     * @param DailRequest - the struct containing the dial request params like address, clir
     * call details, etc.
     * 
     */
    virtual ::android::hardware::Return<void> addParticipant_1_4(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_4::DialRequest& dialRequest) = 0;

    /**
     * MessageId.REQUEST_SET_IMS_CONFIG
     * 
     * @param token Id to match request/response. Response must include same token.
     * @param config Config to be set. Contains item and value.
     * 
     */
    virtual ::android::hardware::Return<void> setConfig_1_5(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_5::ConfigInfo& config) = 0;

    /**
     * MessageId.REQUEST_EMERGENCY_DIAL
     * 
     * Initiate emergency voice call, with zero or more emergency service category(s), zero or
     * more emergency Uniform Resource Names (URN), and routing information for handling the call.
     * IMS uses this request to make its emergency call instead of using @1.4::IImsRadio.dial_1_4
     * if the 'address' in the 'dialRequest' field is identified as an emergency number by Android.
     * 
     * Some countries or carriers require some emergency numbers that must be handled with normal
     * call routing or emergency routing. If the 'routing' field is specified as
     * @1.5::EmergencyCallRoute#NORMAL, the implementation must use normal call routing to
     * handle the call; if it is specified as @1.5::EmergencyNumberRoute#EMERGENCY, the
     * implementation must use emergency routing to handle the call; if it is
     * @1.5::EmergencyNumberRoute#UNKNOWN, Android does not know how to handle the call.
     * 
     * If the dialed emergency number does not have a specified emergency service category, the
     * 'categories' field is set to @1.5::EmergencyServiceCategory#UNSPECIFIED; if the dialed
     * emergency number does not have specified emergency Uniform Resource Names, the 'urns' field
     * is set to an empty list. If the underlying technology used to request emergency services
     * does not support the emergency service category or emergency uniform resource names, the
     * field 'categories' or 'urns' may be ignored.
     * 
     * If 'isTesting' is true, this request is for testing purpose, and must not be sent to a real
     * emergency service; otherwise it's for a real emergency call request.
     * 
     * Reference: 3gpp 22.101, Section 10 - Emergency Calls;
     *            3gpp 23.167, Section 6 - Functional description;
     *            3gpp 24.503, Section 5.1.6.8.1 - General;
     *            RFC 5031
     * 
     * @param token to match request/response. Responses must include the same token as requests.
     * @param dialRequest - the struct containing the dial request params like address, clir
     *     call details, etc.
     * @param categories - the Emergency Service Category(s) of the call.
     * @param urns - the emergency Uniform Resource Names (URN)
     * @param route - the emergency call routing information.
     * @param hasKnownUserIntentEmergency - indicate if user's intent for the emergency call
     *     is known.
     * @param isTesting - to represent real or test emergency call.
     * 
     * Response function is IImsRadioResponse.dialResponse()
     * 
     */
    virtual ::android::hardware::Return<void> emergencyDial(int32_t token, const ::vendor::qti::hardware::radio::ims::V1_4::DialRequest& dialRequest, ::android::hardware::hidl_bitfield<::vendor::qti::hardware::radio::ims::V1_5::EmergencyServiceCategory> categories, const ::android::hardware::hidl_vec<::android::hardware::hidl_string>& urns, ::vendor::qti::hardware::radio::ims::V1_5::EmergencyCallRoute route, bool hasKnownUserIntentEmergency, bool isTesting) = 0;

    /**
     * MessageId.REQUEST_ACKNOWLEDGE_IMS_SMS
     * 
     * @param token Id to match request/response. Response must include same token.
     * @messageRef the message reference or -1 of unavailable.
     * @smsResult result of delivering the message. Valid values are defined in types.hal.
     * 
     */
    virtual ::android::hardware::Return<void> acknowledgeSms_1_5(int32_t token, uint32_t messageRef, ::vendor::qti::hardware::radio::ims::V1_5::ImsSmsDeliverStatusResult smsResult) = 0;

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
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_5::IImsRadio>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_5::IImsRadio>& parent, bool emitError = false);
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_5::IImsRadio>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_4::IImsRadio>& parent, bool emitError = false);
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_5::IImsRadio>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_3::IImsRadio>& parent, bool emitError = false);
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_5::IImsRadio>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_2::IImsRadio>& parent, bool emitError = false);
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_5::IImsRadio>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_1::IImsRadio>& parent, bool emitError = false);
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_5::IImsRadio>> castFrom(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_0::IImsRadio>& parent, bool emitError = false);
    /**
     * This performs a checked cast based on what the underlying implementation actually is.
     */
    static ::android::hardware::Return<::android::sp<::vendor::qti::hardware::radio::ims::V1_5::IImsRadio>> castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError = false);

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
    static ::android::sp<IImsRadio> tryGetService(const std::string &serviceName="default", bool getStub=false);
    /**
     * Deprecated. See tryGetService(std::string, bool)
     */
    static ::android::sp<IImsRadio> tryGetService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return tryGetService(str, getStub); }
    /**
     * Deprecated. See tryGetService(std::string, bool)
     */
    static ::android::sp<IImsRadio> tryGetService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return tryGetService(str, getStub); }
    /**
     * Calls tryGetService("default", bool). This is the recommended instance name for singleton services.
     */
    static ::android::sp<IImsRadio> tryGetService(bool getStub) { return tryGetService("default", getStub); }
    /**
     * This gets the service of this type with the specified instance name. If the
     * service is not in the VINTF manifest on a Trebilized device, this will return
     * nullptr. If the service is not available, this will wait for the service to
     * become available. If the service is a lazy service, this will start the service
     * and return when it becomes available. If getStub is true, this will try to
     * return an unwrapped passthrough implementation in the same process. This is
     * useful when getting an implementation from the same partition/compilation group.
     */
    static ::android::sp<IImsRadio> getService(const std::string &serviceName="default", bool getStub=false);
    /**
     * Deprecated. See getService(std::string, bool)
     */
    static ::android::sp<IImsRadio> getService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return getService(str, getStub); }
    /**
     * Deprecated. See getService(std::string, bool)
     */
    static ::android::sp<IImsRadio> getService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return getService(str, getStub); }
    /**
     * Calls getService("default", bool). This is the recommended instance name for singleton services.
     */
    static ::android::sp<IImsRadio> getService(bool getStub) { return getService("default", getStub); }
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

static inline std::string toString(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_5::IImsRadio>& o);

//
// type header definitions for package
//

static inline std::string toString(const ::android::sp<::vendor::qti::hardware::radio::ims::V1_5::IImsRadio>& o) {
    std::string os = "[class or subclass of ";
    os += ::vendor::qti::hardware::radio::ims::V1_5::IImsRadio::descriptor;
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


#endif  // HIDL_GENERATED_VENDOR_QTI_HARDWARE_RADIO_IMS_V1_5_IIMSRADIO_H
