#ifndef HIDL_GENERATED_ANDROID_HARDWARE_RADIO_V1_1_IRADIO_H
#define HIDL_GENERATED_ANDROID_HARDWARE_RADIO_V1_1_IRADIO_H

#include <android/hardware/radio/1.0/IRadio.h>
#include <android/hardware/radio/1.1/types.h>

#include <android/hidl/manager/1.0/IServiceNotification.h>

#include <hidl/HidlSupport.h>
#include <hidl/MQDescriptor.h>
#include <hidl/Status.h>
#include <utils/NativeHandle.h>
#include <utils/misc.h>

namespace android {
namespace hardware {
namespace radio {
namespace V1_1 {

struct IRadio : public ::android::hardware::radio::V1_0::IRadio {
    typedef android::hardware::details::i_tag _hidl_tag;

    // Forward declaration for forward reference support:

    /**
     * This interface is used by telephony and telecom to talk to cellular radio.
     * All the functions have minimum one parameter:
     * serial: which corresponds to serial no. of request. Serial numbers must only be memorized for the
     * duration of a method call. If clients provide colliding serials (including passing the same
     * serial to different methods), multiple responses (one for each method call) must still be served.
     * setResponseFunctions must work with @1.1::IRadioResponse and @1.1::IRadioIndication.
     */
    virtual bool isRemote() const override { return false; }


    /**
     * Set response functions for radio requests & radio indications.
     * 
     * @param radioResponse Object containing response functions
     * @param radioIndication Object containing radio indications
     */
    virtual ::android::hardware::Return<void> setResponseFunctions(const ::android::sp<::android::hardware::radio::V1_0::IRadioResponse>& radioResponse, const ::android::sp<::android::hardware::radio::V1_0::IRadioIndication>& radioIndication) = 0;

    /**
     * Requests status of the ICC card
     * 
     * @param serial Serial number of request.
     * 
     * Response function is IRadioResponse.getIccCardStatusResponse()
     * 
     */
    virtual ::android::hardware::Return<void> getIccCardStatus(int32_t serial) = 0;

    /**
     * Supplies ICC PIN. Only called if CardStatus has AppState.PIN state
     * 
     * @param serial Serial number of request.
     * @param pin PIN value
     * @param aid AID value, See ETSI 102.221 8.1 and 101.220 4, empty string if no value.
     * 
     * Response function is IRadioResponse.supplyIccPinForAppResponse()
     * 
     */
    virtual ::android::hardware::Return<void> supplyIccPinForApp(int32_t serial, const ::android::hardware::hidl_string& pin, const ::android::hardware::hidl_string& aid) = 0;

    /**
     * Supplies ICC PUK and new PIN.
     * 
     * @param serial Serial number of request.
     * @param puk PUK value
     * @param pin New PIN value
     * @param aid AID value, See ETSI 102.221 8.1 and 101.220 4, empty string if no value.
     * 
     * Response function is IRadioResponse.supplyIccPukForAppResponse()
     * 
     */
    virtual ::android::hardware::Return<void> supplyIccPukForApp(int32_t serial, const ::android::hardware::hidl_string& puk, const ::android::hardware::hidl_string& pin, const ::android::hardware::hidl_string& aid) = 0;

    /**
     * Supplies ICC PIN2. Only called following operation where SIM_PIN2 was
     * returned as a a failure from a previous operation.
     * 
     * @param serial Serial number of request.
     * @param pin2 PIN2 value
     * @param aid AID value, See ETSI 102.221 8.1 and 101.220 4, empty string if no value.
     * 
     * Response function is IRadioResponse.supplyIccPin2ForAppResponse()
     * 
     */
    virtual ::android::hardware::Return<void> supplyIccPin2ForApp(int32_t serial, const ::android::hardware::hidl_string& pin2, const ::android::hardware::hidl_string& aid) = 0;

    /**
     * Supplies ICC PUK2 and new PIN2.
     * 
     * @param serial Serial number of request.
     * @param puk2 PUK2 value
     * @param pin2 New PIN2 value
     * @param aid AID value, See ETSI 102.221 8.1 and 101.220 4, empty string if no value.
     * 
     * Response function is IRadioResponse.supplyIccPuk2ForAppResponse()
     * 
     */
    virtual ::android::hardware::Return<void> supplyIccPuk2ForApp(int32_t serial, const ::android::hardware::hidl_string& puk2, const ::android::hardware::hidl_string& pin2, const ::android::hardware::hidl_string& aid) = 0;

    /**
     * Supplies old ICC PIN and new PIN.
     * 
     * @param serial Serial number of request.
     * @param oldPin Old pin value
     * @param newPin New pin value
     * @param aid AID value, See ETSI 102.221 8.1 and 101.220 4, empty string if no value.
     * 
     * Response function is IRadioResponse.changeIccPinForAppResponse()
     * 
     */
    virtual ::android::hardware::Return<void> changeIccPinForApp(int32_t serial, const ::android::hardware::hidl_string& oldPin, const ::android::hardware::hidl_string& newPin, const ::android::hardware::hidl_string& aid) = 0;

    /**
     * Supplies old ICC PIN2 and new PIN2.
     * 
     * @param serial Serial number of request.
     * @param oldPin2 Old pin2 value
     * @param newPin2 New pin2 value
     * @param aid AID value, See ETSI 102.221 8.1 and 101.220 4, empty string if no value.
     * 
     * Response function is IRadioResponse.changeIccPin2ForAppResponse()
     * 
     */
    virtual ::android::hardware::Return<void> changeIccPin2ForApp(int32_t serial, const ::android::hardware::hidl_string& oldPin2, const ::android::hardware::hidl_string& newPin2, const ::android::hardware::hidl_string& aid) = 0;

    /**
     * Requests that network personalization be deactivated
     * 
     * @param serial Serial number of request.
     * @param netPin Network depersonlization code
     * 
     * Response function is IRadioResponse.supplyNetworkDepersonalizationResponse()
     * 
     */
    virtual ::android::hardware::Return<void> supplyNetworkDepersonalization(int32_t serial, const ::android::hardware::hidl_string& netPin) = 0;

    /**
     * Requests current call list
     * 
     * @param serial Serial number of request.
     * 
     * Response function is IRadioResponse.getCurrentCallsResponse()
     * 
     */
    virtual ::android::hardware::Return<void> getCurrentCalls(int32_t serial) = 0;

    /**
     * Initiate voice call.
     * This method is never used for supplementary service codes
     * 
     * @param serial Serial number of request.
     * @param dialInfo Dial struct
     * 
     * Response function is IRadioResponse.dialResponse()
     * 
     */
    virtual ::android::hardware::Return<void> dial(int32_t serial, const ::android::hardware::radio::V1_0::Dial& dialInfo) = 0;

    /**
     * Get the SIM IMSI
     * Only valid when radio state is "RADIO_STATE_ON"
     * 
     * @param serial Serial number of request.
     * @param aid AID value, See ETSI 102.221 8.1 and 101.220 4, empty string if no value.
     * 
     * Response function is IRadioResponse.getImsiForAppResponse()
     * 
     */
    virtual ::android::hardware::Return<void> getImsiForApp(int32_t serial, const ::android::hardware::hidl_string& aid) = 0;

    /**
     * Hang up a specific line (like AT+CHLD=1x)
     * After this HANGUP request returns, Radio must show the connection is NOT
     * active anymore in next requestGetCurrentCalls query.
     * 
     * @param serial Serial number of request.
     * @param gsmIndex Connection index (value of 'x' in CHLD above)
     * 
     * Response function is IRadioResponse.hangupResponse()
     * 
     */
    virtual ::android::hardware::Return<void> hangup(int32_t serial, int32_t gsmIndex) = 0;

    /**
     * Hang up waiting or held (like AT+CHLD=0)
     * After this HANGUP request returns, Radio must show the connection is NOT
     * active anymore in next getCurrentCalls() query.
     * 
     * @param serial Serial number of request.
     * 
     * Response function is IRadioResponse.hangupWaitingOrBackgroundResponse()
     * 
     */
    virtual ::android::hardware::Return<void> hangupWaitingOrBackground(int32_t serial) = 0;

    /**
     * Hang up waiting or held (like AT+CHLD=1)
     * After this HANGUP request returns, Radio must show the connection is NOT
     * active anymore in next getCurrentCalls query.
     * 
     * @param serial Serial number of request.
     * 
     * Response function is IRadioResponse.hangupForegroundResumeBackgroundResponse()
     * 
     */
    virtual ::android::hardware::Return<void> hangupForegroundResumeBackground(int32_t serial) = 0;

    /**
     * Switch waiting or holding call and active call (like AT+CHLD=2)
     * State transitions must be as follows:
     * 
     * Call transitions must happen as shown below.
     * 
     *   BEFORE                               AFTER
     * Call 1   Call 2                 Call 1       Call 2
     * ACTIVE   HOLDING                HOLDING     ACTIVE
     * ACTIVE   WAITING                HOLDING     ACTIVE
     * HOLDING  WAITING                HOLDING     ACTIVE
     * ACTIVE   IDLE                   HOLDING     IDLE
     * IDLE     IDLE                   IDLE        IDLE
     * 
     * @param serial Serial number of request.
     * 
     * Response function is IRadioResponse.switchWaitingOrHoldingAndActiveResponse()
     * 
     */
    virtual ::android::hardware::Return<void> switchWaitingOrHoldingAndActive(int32_t serial) = 0;

    /**
     * Conference holding and active (like AT+CHLD=3)
     * 
     * @param serial Serial number of request.
     * 
     * Response function is IRadioResponse.conferenceResponse()
     * 
     */
    virtual ::android::hardware::Return<void> conference(int32_t serial) = 0;

    /**
     * Send UDUB (user determined user busy) to ringing or
     * waiting call answer)
     * 
     * @param serial Serial number of request.
     * 
     * Response function is IRadioResponse.rejectCallResponse()
     * 
     */
    virtual ::android::hardware::Return<void> rejectCall(int32_t serial) = 0;

    /**
     * Requests the failure cause code for the most recently terminated call.
     * 
     * @param serial Serial number of request.
     * 
     * Response function is IRadioResponse.getLastCallFailCauseResponse()
     * 
     */
    virtual ::android::hardware::Return<void> getLastCallFailCause(int32_t serial) = 0;

    /**
     * Requests current signal strength and associated information.
     * Must succeed if radio is on.
     * 
     * @param serial Serial number of request.
     * 
     * Response function is IRadioResponse.getSignalStrengthResponse()
     */
    virtual ::android::hardware::Return<void> getSignalStrength(int32_t serial) = 0;

    /**
     * Request current voice registration state
     * 
     * @param serial Serial number of request.
     * 
     * Response function is IRadioResponse.getVoiceRegistrationStateResponse()
     */
    virtual ::android::hardware::Return<void> getVoiceRegistrationState(int32_t serial) = 0;

    /**
     * Request current data registration state
     * 
     * @param serial Serial number of request.
     * 
     * Response function is IRadioResponse.getDataRegistrationStateResponse()
     */
    virtual ::android::hardware::Return<void> getDataRegistrationState(int32_t serial) = 0;

    /**
     * Request current operator ONS or EONS
     * 
     * @param serial Serial number of request.
     * 
     * Response function is IRadioResponse.getOperatorResponse()
     */
    virtual ::android::hardware::Return<void> getOperator(int32_t serial) = 0;

    /**
     * Toggle radio on and off (for "airplane" mode)
     * If the radio is turned off/on the radio modem subsystem
     * is expected return to an initialized state. For instance,
     * any voice and data calls must be terminated and all associated
     * lists emptied.
     * 
     * @param serial Serial number of request.
     * @param on To turn on radio -> on = true, to turn off radio -> on = false.
     * 
     * Response function is IRadioResponse.setRadioPowerResponse()
     */
    virtual ::android::hardware::Return<void> setRadioPower(int32_t serial, bool on) = 0;

    /**
     * Send a DTMF tone
     * 
     * If the implementation is currently playing a tone requested via
     * startDtmf(), that tone must be cancelled and the new tone
     * must be played instead
     * 
     * @param serial Serial number of request.
     * @param s string with single char having one of 12 values: 0-9, *, #
     * 
     * Response function is IRadioResponse.sendDtmfResponse()
     */
    virtual ::android::hardware::Return<void> sendDtmf(int32_t serial, const ::android::hardware::hidl_string& s) = 0;

    /**
     * Send an SMS message
     * 
     * @param serial Serial number of request.
     * @param message GsmSmsMessage as defined in types.hal
     * 
     * Response function is IRadioResponse.sendSmsResponse()
     * 
     * Based on the return error, caller decides to resend if sending sms
     * fails. RadioError:SMS_SEND_FAIL_RETRY means retry (i.e. error cause is 332)
     * and RadioError:GENERIC_FAILURE means no retry (i.e. error cause is 500)
     */
    virtual ::android::hardware::Return<void> sendSms(int32_t serial, const ::android::hardware::radio::V1_0::GsmSmsMessage& message) = 0;

    /**
     * Send an SMS message. Identical to sendSms,
     * except that more messages are expected to be sent soon. If possible,
     * keep SMS relay protocol link open (eg TS 27.005 AT+CMMS command)
     * 
     * @param serial Serial number of request.
     * @param message GsmSmsMessage as defined in types.hal
     * 
     * Response function is IRadioResponse.sendSMSExpectMoreResponse()
     * 
     * Based on the return error, caller decides to resend if sending sms
     * fails. RadioError:SMS_SEND_FAIL_RETRY means retry (i.e. error cause is 332)
     * and RadioError:GENERIC_FAILURE means no retry (i.e. error cause is 500)
     */
    virtual ::android::hardware::Return<void> sendSMSExpectMore(int32_t serial, const ::android::hardware::radio::V1_0::GsmSmsMessage& message) = 0;

    /**
     * Setup a packet data connection. If DataCallResponse.status
     * return DataCallFailCause:NONE it is added to the list of data calls and a
     * unsolDataCallListChanged() is sent. The call remains in the
     * list until deactivateDataCall() is issued or the
     * radio is powered off/on. This list is returned by getDataCallList()
     * and dataCallListChanged().
     * 
     * The Radio is expected to:
     *  - Create one data call context.
     *  - Create and configure a dedicated interface for the context
     *  - The interface must be point to point.
     *  - The interface is configured with one or more addresses and
     *    is capable of sending and receiving packets. The prefix length
     *    of the addresses must be /32 for IPv4 and /128 for IPv6.
     *  - Must NOT change the linux routing table.
     *  - Support up to getDataRegistrationState response[5]
     *    number of simultaneous data call contexts.
     * 
     * @param serial Serial number of request.
     * @param radioTechnology Radio technology to use.
     * @param dataProfileInfo data profile info.
     * @param modemCognitive Indicating this profile was sent to the modem through setDataProfile
     *                       earlier.
     * @param roamingAllowed Indicating data roaming is allowed or not by the user.
     * @param isRoaming Indicating the device is roaming or not. The 'protocol' parameter in the old
     *                  RIL API must be filled accordingly based on the roaming condition.
     *                  Note this is for backward compatibility with the old radio modem. The modem
     *                  must not use this param for any other reason.
     * 
     * Response function is IRadioResponse.setupDataCallResponse()
     */
    virtual ::android::hardware::Return<void> setupDataCall(int32_t serial, ::android::hardware::radio::V1_0::RadioTechnology radioTechnology, const ::android::hardware::radio::V1_0::DataProfileInfo& dataProfileInfo, bool modemCognitive, bool roamingAllowed, bool isRoaming) = 0;

    /**
     * Request ICC I/O operation.
     * This is similar to the TS 27.007 "restricted SIM" operation
     * where it assumes all of the EF selection must be done by the
     * callee
     * 
     * Arguments and responses that are unused for certain
     * values of "command" must be ignored or set to empty string
     * 
     * @param serial Serial number of request.
     * @param iccIo IccIo
     * 
     * Please note that IccIo has a "PIN2" field which may be empty string,
     * or may specify a PIN2 for operations that require a PIN2 (eg
     * updating FDN records)
     * 
     * Response function is IRadioResponse.iccIOForAppResponse()
     */
    virtual ::android::hardware::Return<void> iccIOForApp(int32_t serial, const ::android::hardware::radio::V1_0::IccIo& iccIo) = 0;

    /**
     * Send a USSD message.
     * 
     * If a USSD session already exists, the message must be sent in the
     * context of that session. Otherwise, a new session must be created.
     * 
     * The network reply must be reported via unsolOnUssd
     * 
     * Only one USSD session must exist at a time, and the session is assumed
     * to exist until:
     * a) The android system invokes cancelUssd()
     * b) The implementation sends a unsolOnUssd() with a type code
     * of "0" (USSD-Notify/no further action) or "2" (session terminated)
     * 
     * @param serial Serial number of request.
     * @param ussd string containing the USSD request in UTF-8 format
     * 
     * Response function is IRadioResponse.sendUssdResponse()
     * 
     * See also requestCancelUssd, unsolOnUssd
     */
    virtual ::android::hardware::Return<void> sendUssd(int32_t serial, const ::android::hardware::hidl_string& ussd) = 0;

    /**
     * Cancel the current USSD session if one exists.
     * 
     * @param serial Serial number of request.
     * 
     * Response function is IRadioResponse.cancelPendingUssdResponse()
     */
    virtual ::android::hardware::Return<void> cancelPendingUssd(int32_t serial) = 0;

    /**
     * Gets current CLIR status
     * 
     * @param serial Serial number of request.
     * 
     * Response function is IRadioResponse.getClirResponse()
     */
    virtual ::android::hardware::Return<void> getClir(int32_t serial) = 0;

    /**
     * Set current CLIR status
     * 
     * @param serial Serial number of request.
     * @param status "n" parameter from TS 27.007 7.7
     * 
     * Response function is IRadioResponse.setClirResponse()
     */
    virtual ::android::hardware::Return<void> setClir(int32_t serial, int32_t status) = 0;

    /**
     * Request call forward status.
     * 
     * @param serial Serial number of request.
     * @param callInfo CallForwardInfo
     * 
     * Response function is IRadioResponse.getCallForwardStatusResponse()
     */
    virtual ::android::hardware::Return<void> getCallForwardStatus(int32_t serial, const ::android::hardware::radio::V1_0::CallForwardInfo& callInfo) = 0;

    /**
     * Configure call forward rule
     * 
     * @param serial Serial number of request.
     * @param callInfo CallForwardInfo
     * 
     * Response function is IRadioResponse.setCallForwardResponse()
     */
    virtual ::android::hardware::Return<void> setCallForward(int32_t serial, const ::android::hardware::radio::V1_0::CallForwardInfo& callInfo) = 0;

    /**
     * Query current call waiting state
     * 
     * @param serial Serial number of request.
     * @param serviceClass Service class is the TS 27.007 service class to query
     * 
     * Response function is IRadioResponse.getCallWaitingResponse()
     */
    virtual ::android::hardware::Return<void> getCallWaiting(int32_t serial, int32_t serviceClass) = 0;

    /**
     * Configure current call waiting state
     * 
     * @param serial Serial number of request.
     * @param enable is false for "disabled" and true for "enabled"
     * @param serviceClass is the TS 27.007 service class bit vector of services to modify
     * 
     * Response function is IRadioResponse.setCallWaitingResponse()
     */
    virtual ::android::hardware::Return<void> setCallWaiting(int32_t serial, bool enable, int32_t serviceClass) = 0;

    /**
     * Acknowledge successful or failed receipt of SMS previously indicated
     * via unsolResponseNewSms
     * 
     * @param serial Serial number of request.
     * @param success is true on successful receipt
     *        (basically, AT+CNMA=1 from TS 27.005 is 0 on failed receipt
     *        (basically, AT+CNMA=2 from TS 27.005)
     * @param cause: if success is false, this contains the failure cause as defined
     *        in TS 23.040, 9.2.3.22.
     * 
     * Response function is IRadioResponse.acknowledgeLastIncomingGsmSmsResponse()
     */
    virtual ::android::hardware::Return<void> acknowledgeLastIncomingGsmSms(int32_t serial, bool success, ::android::hardware::radio::V1_0::SmsAcknowledgeFailCause cause) = 0;

    /**
     * Answer incoming call
     * Must not be called for WAITING calls.
     * switchWaitingOrHoldingAndActive() must be used in this case
     * instead
     * 
     * @param serial Serial number of request.
     * 
     * Response function is IRadioResponse.acceptCallResponse()
     */
    virtual ::android::hardware::Return<void> acceptCall(int32_t serial) = 0;

    /**
     * Deactivate packet data connection and remove from the
     * data call list if RadioError:NONE is returned. Any other return
     * values must also try to remove the call from the list. An
     * unsolDataCallListChanged must be
     * issued because of an deactivateDataCall.
     * 
     * @param serial Serial number of request.
     * @param cid Indicates CID
     * @param reasonRadioShutDown Indicates Disconnect Reason
     *        false => No specific reason specified
     *        true => Radio shutdown requested
     * 
     * Response function is IRadioResponse.deactivateDataCallResponse()
     */
    virtual ::android::hardware::Return<void> deactivateDataCall(int32_t serial, int32_t cid, bool reasonRadioShutDown) = 0;

    /**
     * Query the status of a facility lock state
     * 
     * @param serial Serial number of request.
     * @param facility is the facility string code from TS 27.007 7.4
     *        (eg "AO" for BAOC, "SC" for SIM lock)
     * @param password is the password, or "" if not required
     * @param serviceClass is the TS 27.007 service class bit vector of services to query
     * @param appId is AID value, See ETSI 102.221 8.1 and 101.220 4, empty string if no value.
     *        This is only applicable in the case of Fixed Dialing Numbers (FDN) requests.
     * 
     * Response function is IRadioResponse.getFacilityLockForAppResponse()
     */
    virtual ::android::hardware::Return<void> getFacilityLockForApp(int32_t serial, const ::android::hardware::hidl_string& facility, const ::android::hardware::hidl_string& password, int32_t serviceClass, const ::android::hardware::hidl_string& appId) = 0;

    /**
     * Enable/disable one facility lock
     * 
     * @param serial Serial number of request.
     * @param facility is the facility string code from TS 27.007 7.4 (eg "AO" for BAOC)
     * @param lockState false for "unlock" and true for "lock"
     * @param password is the password
     * @param serviceClass is string representation of decimal TS 27.007
     *        service class bit vector. Eg, the string
     *        "1" means "set this facility for voice services"
     * @param appId is AID value, See ETSI 102.221 8.1 and 101.220 4, empty string if no value.
     *        This is only applicable in the case of Fixed Dialing Numbers (FDN) requests.
     * 
     * Response function is IRadioResponse.setFacilityLockForAppResponse()
     */
    virtual ::android::hardware::Return<void> setFacilityLockForApp(int32_t serial, const ::android::hardware::hidl_string& facility, bool lockState, const ::android::hardware::hidl_string& password, int32_t serviceClass, const ::android::hardware::hidl_string& appId) = 0;

    /**
     * Change call barring facility password
     * 
     * @param serial Serial number of request.
     * @param facility facility string code from TS 27.007 7.4 (eg "AO" for BAOC)
     * @param oldPassword old password
     * @param newPassword new password
     * 
     * Response function is IRadioResponse.setBarringPasswordResponse()
     */
    virtual ::android::hardware::Return<void> setBarringPassword(int32_t serial, const ::android::hardware::hidl_string& facility, const ::android::hardware::hidl_string& oldPassword, const ::android::hardware::hidl_string& newPassword) = 0;

    /**
     * Query current network selection mode
     * 
     * @param serial Serial number of request.
     * 
     * Response function is IRadioResponse.getNetworkSelectionModeResponse()
     */
    virtual ::android::hardware::Return<void> getNetworkSelectionMode(int32_t serial) = 0;

    /**
     * Specify that the network must be selected automatically.
     * This request must not respond until the new operator is selected and registered.
     * 
     * @param serial Serial number of request.
     * 
     * Response function is IRadioResponse.setNetworkSelectionModeAutomaticResponse()
     */
    virtual ::android::hardware::Return<void> setNetworkSelectionModeAutomatic(int32_t serial) = 0;

    /**
     * Manually select a specified network.
     * This request must not respond until the new operator is selected and registered.
     * 
     * @param serial Serial number of request.
     * @param operatorNumeric string specifying MCCMNC of network to select (eg "310170")
     * 
     * Response function is IRadioResponse.setNetworkSelectionModeManualResponse()
     */
    virtual ::android::hardware::Return<void> setNetworkSelectionModeManual(int32_t serial, const ::android::hardware::hidl_string& operatorNumeric) = 0;

    /**
     * Scans for available networks
     * 
     * @param serial Serial number of request.
     * 
     * Response function is IRadioResponse.getAvailableNetworksResponse()
     */
    virtual ::android::hardware::Return<void> getAvailableNetworks(int32_t serial) = 0;

    /**
     * Start playing a DTMF tone. Continue playing DTMF tone until
     * stopDtmf is received.
     * If a startDtmf() is received while a tone is currently playing,
     * it must cancel the previous tone and play the new one.
     * 
     * @param serial Serial number of request.
     * @param s string having a single character with one of 12 values: 0-9,*,#
     * 
     * Response function is IRadioResponse.startDtmfResponse()
     */
    virtual ::android::hardware::Return<void> startDtmf(int32_t serial, const ::android::hardware::hidl_string& s) = 0;

    /**
     * Stop playing a currently playing DTMF tone.
     * 
     * @param serial Serial number of request.
     * 
     * Response function is IRadioResponse.stopDtmfResponse()
     */
    virtual ::android::hardware::Return<void> stopDtmf(int32_t serial) = 0;

    /**
     * Return string value indicating baseband version, eg response from AT+CGMR
     * 
     * @param serial Serial number of request.
     * 
     * Response function is IRadioResponse.getBasebandVersionResponse()
     */
    virtual ::android::hardware::Return<void> getBasebandVersion(int32_t serial) = 0;

    /**
     * Separate a party from a multiparty call placing the multiparty call
     * (less the specified party) on hold and leaving the specified party
     * as the only other member of the current (active) call
     * 
     * Like AT+CHLD=2x
     * 
     * See TS 22.084 1.3.8.2 (iii)
     * TS 22.030 6.5.5 "Entering "2X followed by send"
     * TS 27.007 "AT+CHLD=2x"
     * 
     * @param serial Serial number of request.
     * @param gsmIndex contains Connection index (value of 'x' in CHLD above)
     * 
     * Response function is IRadioResponse.separateConnectionResponse()
     */
    virtual ::android::hardware::Return<void> separateConnection(int32_t serial, int32_t gsmIndex) = 0;

    /**
     * Turn on or off uplink (microphone) mute.
     * Must only be sent while voice call is active.
     * Must always be reset to "disable mute" when a new voice call is initiated
     * 
     * @param serial Serial number of request.
     * @param enable true for "enable mute" and false for "disable mute"
     * 
     * Response function is IRadioResponse.setMuteResponse()
     */
    virtual ::android::hardware::Return<void> setMute(int32_t serial, bool enable) = 0;

    /**
     * Queries the current state of the uplink mute setting
     * 
     * @param serial Serial number of request.
     * 
     * Response function is IRadioResponse.getMuteResponse()
     */
    virtual ::android::hardware::Return<void> getMute(int32_t serial) = 0;

    /**
     * Queries the status of the CLIP supplementary service
     * (for MMI code "*#30#")
     * 
     * @param serial Serial number of request.
     * 
     * Response function is IRadioResponse.getClipResponse()
     */
    virtual ::android::hardware::Return<void> getClip(int32_t serial) = 0;

    /**
     * Returns the data call list. An entry is added when a
     * setupDataCall() is issued and removed on a
     * deactivateDataCall(). The list is emptied when
     * setRadioPower() off/on is issued.
     * 
     * @param serial Serial number of request.
     * 
     * Response function is IRadioResponse.getDataCallListResponse()
     */
    virtual ::android::hardware::Return<void> getDataCallList(int32_t serial) = 0;

    /**
     * Enables/disables supplementary service related notifications from the network.
     * Notifications are reported via unsolSuppSvcNotification().
     * 
     * @param serial Serial number of request.
     * @param enable true = notifications enabled, false = notifications disabled.
     * 
     * Response function is IRadioResponse.setSuppServiceNotificationsResponse()
     */
    virtual ::android::hardware::Return<void> setSuppServiceNotifications(int32_t serial, bool enable) = 0;

    /**
     * Stores a SMS message to SIM memory.
     * 
     * @param serial Serial number of request.
     * @param smsWriteArgs SmsWriteArgs defined in types.hal
     * 
     * Response function is IRadioResponse.writeSmsToSimResponse()
     */
    virtual ::android::hardware::Return<void> writeSmsToSim(int32_t serial, const ::android::hardware::radio::V1_0::SmsWriteArgs& smsWriteArgs) = 0;

    /**
     * Deletes a SMS message from SIM memory.
     * 
     * @param serial Serial number of request.
     * @param index Record index of the message to delete.
     * 
     * Response function is IRadioResponse.deleteSmsOnSimResponse()
     */
    virtual ::android::hardware::Return<void> deleteSmsOnSim(int32_t serial, int32_t index) = 0;

    /**
     * Assign a specified band for RF configuration.
     * 
     * @param serial Serial number of request.
     * @param mode RadioBandMode defined in types.hal
     * 
     * Response function is IRadioResponse.setBandModeResponse()
     */
    virtual ::android::hardware::Return<void> setBandMode(int32_t serial, ::android::hardware::radio::V1_0::RadioBandMode mode) = 0;

    /**
     * Get the list of band modes supported by RF.
     * 
     * @param serial Serial number of request.
     * 
     * Response function is IRadioResponse.getAvailableBandModesResponse()
     */
    virtual ::android::hardware::Return<void> getAvailableBandModes(int32_t serial) = 0;

    /**
     * Requests to send a SAT/USAT envelope command to SIM.
     * The SAT/USAT envelope command refers to 3GPP TS 11.14 and 3GPP TS 31.111
     * 
     * @param serial Serial number of request.
     * @param command SAT/USAT command in hexadecimal format string starting with command tag
     * 
     * Response function is IRadioResponse.sendEnvelopeResponse()
     */
    virtual ::android::hardware::Return<void> sendEnvelope(int32_t serial, const ::android::hardware::hidl_string& command) = 0;

    /**
     * Requests to send a terminal response to SIM for a received proactive command
     * 
     * @param serial Serial number of request.
     * @param commandResponse SAT/USAT response in hexadecimal format string starting with
     *        first byte of response data
     * 
     * Response function is IRadioResponse.sendTerminalResponseResponseToSim()
     */
    virtual ::android::hardware::Return<void> sendTerminalResponseToSim(int32_t serial, const ::android::hardware::hidl_string& commandResponse) = 0;

    /**
     * When STK application gets stkCallSetup(), the call actually has
     * been initialized by mobile device already. (We could see the call has been in the 'call
     * list') So, STK application needs to accept/reject the call according to user
     * operations.
     * 
     * @param serial Serial number of request.
     * @param accept true = accept the call setup, false = reject the call setup
     * 
     * Response callback is IRadioResponse.handleStkCallSetupRequestFromSimResponse()
     */
    virtual ::android::hardware::Return<void> handleStkCallSetupRequestFromSim(int32_t serial, bool accept) = 0;

    /**
     * Connects the two calls and disconnects the subscriber from both calls.
     * 
     * @param serial Serial number of request.
     * 
     * Response callback is IRadioResponse.explicitCallTransferResponse()
     */
    virtual ::android::hardware::Return<void> explicitCallTransfer(int32_t serial) = 0;

    /**
     * Requests to set the preferred network type for searching and registering
     * (CS/PS domain, RAT, and operation mode)
     * 
     * @param serial Serial number of request.
     * @param nwType PreferredNetworkType defined in types.hal
     * 
     * Response callback is IRadioResponse.setPreferredNetworkTypeResponse()
     */
    virtual ::android::hardware::Return<void> setPreferredNetworkType(int32_t serial, ::android::hardware::radio::V1_0::PreferredNetworkType nwType) = 0;

    /**
     * Query the preferred network type (CS/PS domain, RAT, and operation mode)
     * for searching and registering
     * 
     * @param serial Serial number of request.
     * 
     * Response callback is IRadioResponse.getPreferredNetworkTypeResponse()
     */
    virtual ::android::hardware::Return<void> getPreferredNetworkType(int32_t serial) = 0;

    /**
     * Request neighboring cell id in GSM network
     * 
     * @param serial Serial number of request.
     * 
     * Response callback is IRadioResponse.getNeighboringCidsResponse()
     */
    virtual ::android::hardware::Return<void> getNeighboringCids(int32_t serial) = 0;

    /**
     * Enables/disables network state change notifications due to changes in
     * LAC and/or CID (for GSM) or BID/SID/NID/latitude/longitude (for CDMA).
     * Basically +CREG=2 vs. +CREG=1 (TS 27.007).
     * Note:  The Radio implementation must default to "updates enabled"
     * when the screen is on and "updates disabled" when the screen is off.
     * 
     * @param serial Serial number of request.
     * @param enable true = updates enabled (+CREG=2), false = updates disabled (+CREG=1)
     * 
     * Response callback is IRadioResponse.setLocationUpdatesResponse()
     */
    virtual ::android::hardware::Return<void> setLocationUpdates(int32_t serial, bool enable) = 0;

    /**
     * Request to set the location where the CDMA subscription shall
     * be retrieved
     * 
     * @param serial Serial number of request.
     * @param cdmaSub CdmaSubscriptionSource
     * 
     * Response callback is IRadioResponse.setCdmaSubscriptionSourceResponse()
     */
    virtual ::android::hardware::Return<void> setCdmaSubscriptionSource(int32_t serial, ::android::hardware::radio::V1_0::CdmaSubscriptionSource cdmaSub) = 0;

    /**
     * Request to set the roaming preferences in CDMA
     * 
     * @param serial Serial number of request.
     * @param type CdmaRoamingType defined in types.hal
     * 
     * Response callback is IRadioResponse.setCdmaRoamingPreferenceResponse()
     */
    virtual ::android::hardware::Return<void> setCdmaRoamingPreference(int32_t serial, ::android::hardware::radio::V1_0::CdmaRoamingType type) = 0;

    /**
     * Request the actual setting of the roaming preferences in CDMA in the modem
     * 
     * @param serial Serial number of request.
     * 
     * Response callback is IRadioResponse.getCdmaRoamingPreferenceResponse()
     */
    virtual ::android::hardware::Return<void> getCdmaRoamingPreference(int32_t serial) = 0;

    /**
     * Request to set the TTY mode
     * 
     * @param serial Serial number of request.
     * @param mode TtyMode
     * 
     * Response callback is IRadioResponse.setTTYModeResponse()
     */
    virtual ::android::hardware::Return<void> setTTYMode(int32_t serial, ::android::hardware::radio::V1_0::TtyMode mode) = 0;

    /**
     * Request the setting of TTY mode
     * 
     * @param serial Serial number of request.
     * 
     * Response callback is IRadioResponse.getTTYModeResponse()
     */
    virtual ::android::hardware::Return<void> getTTYMode(int32_t serial) = 0;

    /**
     * Request to set the preferred voice privacy mode used in voice scrambling.
     * 
     * @param serial Serial number of request.
     * @param enable false for Standard Privacy Mode (Public Long Code Mask)
     *        true for Enhanced Privacy Mode (Private Long Code Mask)
     * 
     * Response callback is IRadioResponse.setPreferredVoicePrivacyResponse()
     */
    virtual ::android::hardware::Return<void> setPreferredVoicePrivacy(int32_t serial, bool enable) = 0;

    /**
     * Request the setting of preferred voice privacy mode.
     * 
     * @param serial Serial number of request.
     * 
     * Response callback is IRadioResponse.getPreferredVoicePrivacyResponse()
     */
    virtual ::android::hardware::Return<void> getPreferredVoicePrivacy(int32_t serial) = 0;

    /**
     * Send FLASH command
     * 
     * @param serial Serial number of request.
     * @param featureCode String associated with Flash command
     * 
     * Response callback is IRadioResponse.sendCDMAFeatureCodeResponse()
     */
    virtual ::android::hardware::Return<void> sendCDMAFeatureCode(int32_t serial, const ::android::hardware::hidl_string& featureCode) = 0;

    /**
     * Send DTMF string
     * 
     * @param serial Serial number of request.
     * @param dtmf DTMF string
     * @param on DTMF ON length in milliseconds, or 0 to use default
     * @param off is the DTMF OFF length in milliseconds, or 0 to use default
     * 
     * Response callback is IRadioResponse.sendBurstDtmfResponse()
     */
    virtual ::android::hardware::Return<void> sendBurstDtmf(int32_t serial, const ::android::hardware::hidl_string& dtmf, int32_t on, int32_t off) = 0;

    /**
     * Send a CDMA SMS message
     * 
     * @param serial Serial number of request.
     * @param sms Cdma Sms to be sent described by CdmaSmsMessage in types.hal
     * 
     * Response callback is IRadioResponse.sendCdmaSmsResponse()
     */
    virtual ::android::hardware::Return<void> sendCdmaSms(int32_t serial, const ::android::hardware::radio::V1_0::CdmaSmsMessage& sms) = 0;

    /**
     * Acknowledge the success or failure in the receipt of SMS
     * previously indicated via responseCdmaNewSms()
     * 
     * @param serial Serial number of request.
     * @param smsAck Cdma Sms ack to be sent described by CdmaSmsAck in types.hal
     * 
     * Response callback is IRadioResponse.acknowledgeLastIncomingCdmaSmsResponse()
     */
    virtual ::android::hardware::Return<void> acknowledgeLastIncomingCdmaSms(int32_t serial, const ::android::hardware::radio::V1_0::CdmaSmsAck& smsAck) = 0;

    /**
     * Request the setting of GSM/WCDMA Cell Broadcast SMS config.
     * 
     * @param serial Serial number of request.
     * 
     * Response callback is IRadioResponse.getGsmBroadcastConfigResponse()
     */
    virtual ::android::hardware::Return<void> getGsmBroadcastConfig(int32_t serial) = 0;

    /**
     * Set GSM/WCDMA Cell Broadcast SMS config
     * 
     * @param serial Serial number of request.
     * @param configInfo Setting of GSM/WCDMA Cell broadcast config
     * 
     * Response callback is IRadioResponse.setGsmBroadcastConfigResponse()
     */
    virtual ::android::hardware::Return<void> setGsmBroadcastConfig(int32_t serial, const ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::GsmBroadcastSmsConfigInfo>& configInfo) = 0;

    /**
     * Enable or disable the reception of GSM/WCDMA Cell Broadcast SMS
     * 
     * @param serial Serial number of request.
     * @param activate indicates to activate or turn off the reception of GSM/WCDMA
     *        Cell Broadcast SMS. true = activate, false = turn off
     * 
     * Response callback is IRadioResponse.setGsmBroadcastActivationResponse()
     */
    virtual ::android::hardware::Return<void> setGsmBroadcastActivation(int32_t serial, bool activate) = 0;

    /**
     * Request the setting of CDMA Broadcast SMS config
     * 
     * @param serial Serial number of request.
     * 
     * Response callback is IRadioResponse.getCdmaBroadcastConfigResponse()
     */
    virtual ::android::hardware::Return<void> getCdmaBroadcastConfig(int32_t serial) = 0;

    /**
     * Set CDMA Broadcast SMS config
     * 
     * @param serial Serial number of request.
     * @param configInfo CDMA Broadcast SMS config to be set.
     * 
     * Response callback is IRadioResponse.setCdmaBroadcastConfigResponse()
     */
    virtual ::android::hardware::Return<void> setCdmaBroadcastConfig(int32_t serial, const ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::CdmaBroadcastSmsConfigInfo>& configInfo) = 0;

    /**
     * Enable or disable the reception of CDMA Cell Broadcast SMS
     * 
     * @param serial Serial number of request.
     * @param activate indicates to activate or turn off the reception of CDMA
     *        Cell Broadcast SMS. true = activate, false = turn off
     * 
     * Response callback is IRadioResponse.setCdmaBroadcastActivationResponse()
     */
    virtual ::android::hardware::Return<void> setCdmaBroadcastActivation(int32_t serial, bool activate) = 0;

    /**
     * Request the device MDN / H_SID / H_NID.
     * The request is only allowed when CDMA subscription is available. When CDMA
     * subscription is changed, application layer must re-issue the request to
     * update the subscription information.
     * 
     * @param serial Serial number of request.
     * 
     * Response callback is IRadioResponse.getCDMASubscriptionResponse()
     */
    virtual ::android::hardware::Return<void> getCDMASubscription(int32_t serial) = 0;

    /**
     * Stores a CDMA SMS message to RUIM memory.
     * 
     * @param serial Serial number of request.
     * @param cdmaSms CDMA message as defined by CdmaSmsWriteArgs in types.hal
     * 
     * Response callback is IRadioResponse.writeSmsToRuimResponse()
     */
    virtual ::android::hardware::Return<void> writeSmsToRuim(int32_t serial, const ::android::hardware::radio::V1_0::CdmaSmsWriteArgs& cdmaSms) = 0;

    /**
     * Deletes a CDMA SMS message from RUIM memory.
     * 
     * @param serial Serial number of request.
     * @param index record index of the message to delete
     * 
     * Response callback is IRadioResponse.deleteSmsOnRuimResponse()
     */
    virtual ::android::hardware::Return<void> deleteSmsOnRuim(int32_t serial, int32_t index) = 0;

    /**
     * Request the device ESN / MEID / IMEI / IMEISV.
     * The request is always allowed and contains GSM and CDMA device identity.
     * When CDMA subscription is changed the ESN/MEID changes. The application
     * layer must re-issue the request to update the device identity in this case.
     * 
     * @param serial Serial number of request.
     * 
     * Response callback is IRadioResponse.getDeviceIdentityResponse()
     */
    virtual ::android::hardware::Return<void> getDeviceIdentity(int32_t serial) = 0;

    /**
     * Request the radio's system selection module to exit emergency
     * callback mode. Radio must not respond with SUCCESS until the modem has
     * completely exited from Emergency Callback Mode.
     * 
     * @param serial Serial number of request.
     * 
     * Response callback is IRadioResponse.exitEmergencyCallbackModeResponse()
     */
    virtual ::android::hardware::Return<void> exitEmergencyCallbackMode(int32_t serial) = 0;

    /**
     * Get the default Short Message Service Center address on the device.
     * 
     * @param serial Serial number of request.
     * 
     * Response callback is IRadioResponse.getSmscAddressResponse()
     */
    virtual ::android::hardware::Return<void> getSmscAddress(int32_t serial) = 0;

    /**
     * Set the default Short Message Service Center address on the device.
     * 
     * @param serial Serial number of request.
     * @param smsc Short Message Service Center address to set
     * 
     * Response callback is IRadioResponse.setSmscAddressResponse()
     */
    virtual ::android::hardware::Return<void> setSmscAddress(int32_t serial, const ::android::hardware::hidl_string& smsc) = 0;

    /**
     * Indicates whether there is storage available for new SMS messages.
     * 
     * @param serial Serial number of request.
     * @param available true if memory is available for storing new messages,
     *        false if memory capacity is exceeded
     * 
     * Response callback is IRadioResponse.reportSmsMemoryStatusResponse()
     */
    virtual ::android::hardware::Return<void> reportSmsMemoryStatus(int32_t serial, bool available) = 0;

    /**
     * Indicates that the StkService is running and is
     * ready to receive unsolicited stkXXXXX commands.
     * 
     * @param serial Serial number of request.
     * 
     * Response callback is IRadioResponse.reportStkServiceIsRunningResponse()
     */
    virtual ::android::hardware::Return<void> reportStkServiceIsRunning(int32_t serial) = 0;

    /**
     * Request to query the location where the CDMA subscription shall be retrieved.
     * 
     * @param serial Serial number of request.
     * 
     * Response callback is IRadioResponse.getCdmaSubscriptionSourceResponse()
     */
    virtual ::android::hardware::Return<void> getCdmaSubscriptionSource(int32_t serial) = 0;

    /**
     * Request the ISIM application on the UICC to perform AKA
     * challenge/response algorithm for IMS authentication
     * 
     * @param serial Serial number of request.
     * @param challenge challenge string in Base64 format
     * 
     * Response callback is IRadioResponse.requestIsimAuthenticationResponse()
     */
    virtual ::android::hardware::Return<void> requestIsimAuthentication(int32_t serial, const ::android::hardware::hidl_string& challenge) = 0;

    /**
     * Acknowledge successful or failed receipt of SMS previously indicated
     * via unsol responseNewSms(), including acknowledgement TPDU to send
     * as the RP-User-Data element of the RP-ACK or RP-ERROR PDU.
     * 
     * @param serial Serial number of request.
     * @param success true on successful receipt (send RP-ACK)
     *        false on failed receipt (send RP-ERROR)
     * @param ackPdu acknowledgement TPDU in hexadecimal format
     * 
     * Response callback is IRadioResponse.acknowledgeIncomingGsmSmsWithPduResponse()
     */
    virtual ::android::hardware::Return<void> acknowledgeIncomingGsmSmsWithPdu(int32_t serial, bool success, const ::android::hardware::hidl_string& ackPdu) = 0;

    /**
     * Requests to send a SAT/USAT envelope command to SIM.
     * The SAT/USAT envelope command refers to 3GPP TS 11.14 and 3GPP TS 31.111.
     * 
     * This request has one difference from sendEnvelope():
     * the SW1 and SW2 status bytes from the UICC response are returned along with
     * the response data, using the same structure as iccIOForApp().
     * 
     * The implementation must perform normal processing of a '91XX'
     * response in SW1/SW2 to retrieve the pending proactive command and send it
     * as an unsolicited response, as sendEnvelope() does.
     * 
     * @param serial Serial number of request.
     * @param contents SAT/USAT command in hexadecimal format starting with command tag
     * 
     * Response callback is IRadioResponse.sendEnvelopeWithStatusResponse()
     */
    virtual ::android::hardware::Return<void> sendEnvelopeWithStatus(int32_t serial, const ::android::hardware::hidl_string& contents) = 0;

    /**
     * Query the radio technology type (3GPP/3GPP2) used for voice. Query is valid only
     * when radio state is not RADIO_STATE_UNAVAILABLE
     * 
     * @param serial Serial number of request.
     * 
     * Response callback is IRadioResponse.getVoiceRadioTechnologyResponse()
     */
    virtual ::android::hardware::Return<void> getVoiceRadioTechnology(int32_t serial) = 0;

    /**
     * Request all of the current cell information known to the radio. The radio
     * must return list of all current cells, including the neighboring cells. If for a particular
     * cell information isn't known then the appropriate unknown value will be returned.
     * This does not cause or change the rate of unsolicited cellInfoList().
     * 
     * @param serial Serial number of request.
     * 
     * Response callback is IRadioResponse.getCellInfoListResponse()
     */
    virtual ::android::hardware::Return<void> getCellInfoList(int32_t serial) = 0;

    /**
     * Sets the minimum time between when unsolicited cellInfoList() must be invoked.
     * A value of 0, means invoke cellInfoList() when any of the reported
     * information changes. Setting the value to INT_MAX(0x7fffffff) means never issue
     * a unsolicited cellInfoList().
     * 
     * @param serial Serial number of request.
     * @param rate minimum time in milliseconds to indicate time between unsolicited cellInfoList()
     * 
     * Response callback is IRadioResponse.setCellInfoListRateResponse()
     */
    virtual ::android::hardware::Return<void> setCellInfoListRate(int32_t serial, int32_t rate) = 0;

    /**
     * Set an apn to initial attach network
     * 
     * @param serial Serial number of request.
     * @param dataProfileInfo data profile containing APN settings
     * @param modemCognitive indicating the data profile was sent to the modem through
     *                       setDataProfile earlier.
     * @param isRoaming Indicating the device is roaming or not. The 'protocol' parameter in the old
     *                  RIL_InitialAttachApn must be filled accordingly based on the roaming
     *                  condition. Note this is for backward compatibility with the old radio modem.
     *                  The modem must not use this param for any other reason.
     * 
     * Response callback is IRadioResponse.setInitialAttachApnResponse()
     */
    virtual ::android::hardware::Return<void> setInitialAttachApn(int32_t serial, const ::android::hardware::radio::V1_0::DataProfileInfo& dataProfileInfo, bool modemCognitive, bool isRoaming) = 0;

    /**
     * Request current IMS registration state
     * 
     * @param serial Serial number of request.
     * 
     * Response callback is IRadioResponse.getImsRegistrationStateResponse()
     */
    virtual ::android::hardware::Return<void> getImsRegistrationState(int32_t serial) = 0;

    /**
     * Send a SMS message over IMS.
     * Based on the return error, caller decides to resend if sending sms
     * fails. SMS_SEND_FAIL_RETRY means retry, and other errors means no retry.
     * In case of retry, data is encoded based on Voice Technology available.
     * 
     * @param serial Serial number of request.
     * @param message ImsSmsMessage as defined in types.hal to be sent
     * 
     * Response callback is IRadioResponse.sendImsSmsResponse()
     */
    virtual ::android::hardware::Return<void> sendImsSms(int32_t serial, const ::android::hardware::radio::V1_0::ImsSmsMessage& message) = 0;

    /**
     * Request APDU exchange on the basic channel. This command reflects TS 27.007
     * "generic SIM access" operation (+CSIM). The modem must ensure proper function
     * of GSM/CDMA, and filter commands appropriately. It must filter
     * channel management and SELECT by DF name commands.
     * "sessionid" field must be ignored.
     * 
     * @param serial Serial number of request.
     * @param message SimApdu as defined in types.hal to be sent
     * 
     * Response callback is IRadioResponse.iccTransmitApduBasicChannelResponse()
     */
    virtual ::android::hardware::Return<void> iccTransmitApduBasicChannel(int32_t serial, const ::android::hardware::radio::V1_0::SimApdu& message) = 0;

    /**
     * Open a new logical channel and select the given application. This command
     * reflects TS 27.007 "open logical channel" operation (+CCHO).
     * 
     * @param serial Serial number of request.
     * @param aid AID value, See ETSI 102.221 and 101.220.
     * @param p2 P2 value, described in ISO 7816-4. Ignore if equal to P2Constant:NO_P2
     * 
     * Response callback is IRadioResponse.iccOpenLogicalChannelResponse()
     */
    virtual ::android::hardware::Return<void> iccOpenLogicalChannel(int32_t serial, const ::android::hardware::hidl_string& aid, int32_t p2) = 0;

    /**
     * Close a previously opened logical channel. This command reflects TS 27.007
     * "close logical channel" operation (+CCHC).
     * 
     * @param serial Serial number of request.
     * @param channelId session id of the logical channel (+CCHC).
     * 
     * Response callback is IRadioResponse.iccCloseLogicalChannelResponse()
     */
    virtual ::android::hardware::Return<void> iccCloseLogicalChannel(int32_t serial, int32_t channelId) = 0;

    /**
     * Exchange APDUs with a UICC over a previously opened logical channel. This
     * command reflects TS 27.007 "generic logical channel access" operation
     * (+CGLA). The modem must filter channel management and SELECT by DF name
     * commands.
     * 
     * @param serial Serial number of request.
     * @param message SimApdu as defined in types.hal to be sent
     * 
     * Response callback is IRadioResponse.iccTransmitApduLogicalChannelResponse()
     */
    virtual ::android::hardware::Return<void> iccTransmitApduLogicalChannel(int32_t serial, const ::android::hardware::radio::V1_0::SimApdu& message) = 0;

    /**
     * Read one of the radio NV items.
     * This is used for device configuration by some CDMA operators.
     * 
     * @param serial Serial number of request.
     * @param itemId NvItem is radio NV item as defined in types.hal
     * 
     * Response callback is IRadioResponse.nvReadItemResponse()
     */
    virtual ::android::hardware::Return<void> nvReadItem(int32_t serial, ::android::hardware::radio::V1_0::NvItem itemId) = 0;

    /**
     * Write one of the radio NV items.
     * This is used for device configuration by some CDMA operators.
     * 
     * @param serial Serial number of request.
     * @param item NvWriteItem as defined in types.hal
     * 
     * Response callback is IRadioResponse.nvWriteItemResponse()
     */
    virtual ::android::hardware::Return<void> nvWriteItem(int32_t serial, const ::android::hardware::radio::V1_0::NvWriteItem& item) = 0;

    /**
     * Update the CDMA Preferred Roaming List (PRL) in the radio NV storage.
     * This is used for device configuration by some CDMA operators.
     * 
     * @param serial Serial number of request.
     * @param prl PRL as a byte array
     * 
     * Response callback is IRadioResponse.nvWriteCdmaPrlResponse()
     */
    virtual ::android::hardware::Return<void> nvWriteCdmaPrl(int32_t serial, const ::android::hardware::hidl_vec<uint8_t>& prl) = 0;

    /**
     * Reset the radio NV configuration to the factory state.
     * This is used for device configuration by some CDMA operators.
     * 
     * @param serial Serial number of request.
     * @param resetType ResetNvType as defined in types.hal
     * 
     * Response callback is IRadioResponse.nvResetConfigResponse()
     */
    virtual ::android::hardware::Return<void> nvResetConfig(int32_t serial, ::android::hardware::radio::V1_0::ResetNvType resetType) = 0;

    /**
     * Selection/de-selection of a subscription from a SIM card
     * 
     * @param serial Serial number of request.
     * @param uiccSub SelectUiccSub as defined in types.hal
     * 
     * Response callback is IRadioResponse.setUiccSubscriptionResponse()
     */
    virtual ::android::hardware::Return<void> setUiccSubscription(int32_t serial, const ::android::hardware::radio::V1_0::SelectUiccSub& uiccSub) = 0;

    /**
     * Tells the modem whether data calls are allowed or not
     * 
     * @param serial Serial number of request.
     * @param allow true to allow data calls, false to disallow data calls
     * 
     * Response callback is IRadioResponse.setDataAllowedResponse()
     */
    virtual ::android::hardware::Return<void> setDataAllowed(int32_t serial, bool allow) = 0;

    /**
     * Request all of the current hardware (modem and sim) associated with Radio.
     * 
     * @param serial Serial number of request.
     * 
     * Response callback is IRadioResponse.getHardwareConfigResponse()
     */
    virtual ::android::hardware::Return<void> getHardwareConfig(int32_t serial) = 0;

    /**
     * Returns the response of SIM Authentication through Radio challenge request.
     * 
     * @param serial Serial number of request.
     * @param authContext P2 value of authentication command, see P2 parameter in
     *        3GPP TS 31.102 7.1.2
     * @param authData the challenge string in Base64 format, see 3GPP TS 31.102 7.1.2
     * @param aid AID value, See ETSI 102.221 8.1 and 101.220 4, empty string if no value
     * 
     * Response callback is IRadioResponse.requestIccSimAuthenticationResponse()
     */
    virtual ::android::hardware::Return<void> requestIccSimAuthentication(int32_t serial, int32_t authContext, const ::android::hardware::hidl_string& authData, const ::android::hardware::hidl_string& aid) = 0;

    /**
     * Set data profile in modem.
     * Modem must erase existed profiles from framework, and apply new profiles
     * 
     * @param serial Serial number of request.
     * @param profiles Array of DataProfiles to set.
     * @param isRoaming Indicating the device is roaming or not. The 'protocol' parameter in the old
     *                  RIL API RIL_DataProfileInfo must be filled accordingly based on the
     *                  roaming condition. Note this is for backward compatibility with the old
     *                  radio modem. The modem must not use this param for any other reason.
     * 
     * Response callback is IRadioResponse.setDataProfileResponse()
     */
    virtual ::android::hardware::Return<void> setDataProfile(int32_t serial, const ::android::hardware::hidl_vec<::android::hardware::radio::V1_0::DataProfileInfo>& profiles, bool isRoaming) = 0;

    /**
     * Device is shutting down. All further commands are ignored
     * and RADIO_NOT_AVAILABLE must be returned.
     * 
     * @param serial Serial number of request.
     * 
     * Response callback is IRadioResponse.requestShutdownResponse()
     */
    virtual ::android::hardware::Return<void> requestShutdown(int32_t serial) = 0;

    /**
     * Used to get phone radio capability.
     * 
     * @param serial Serial number of request.
     * 
     * Response callback is IRadioResponse.getRadioCapabilityResponse()
     */
    virtual ::android::hardware::Return<void> getRadioCapability(int32_t serial) = 0;

    /**
     * Used to set the phones radio capability. Be VERY careful
     * using this request as it may cause some vendor modems to reset. Because
     * of the possible modem reset any radio commands after this one may not be
     * processed.
     * 
     * @param serial Serial number of request.
     * @param rc RadioCapability structure to be set
     * 
     * Response callback is IRadioResponse.setRadioCapabilityResponse()
     */
    virtual ::android::hardware::Return<void> setRadioCapability(int32_t serial, const ::android::hardware::radio::V1_0::RadioCapability& rc) = 0;

    /**
     * Start Link Capacity Estimate (LCE) service if supported by the radio.
     * 
     * @param serial Serial number of request.
     * @param reportInterval desired reporting interval (ms).
     * @param pullMode LCE service mode. true: PULL; false: PUSH.
     * 
     * Response callback is IRadioResponse.startLceServiceResponse()
     */
    virtual ::android::hardware::Return<void> startLceService(int32_t serial, int32_t reportInterval, bool pullMode) = 0;

    /**
     * Stop Link Capacity Estimate (LCE) service, the STOP operation must be
     * idempotent for the radio modem.
     * 
     * @param serial Serial number of request.
     * 
     * Response callback is IRadioResponse.stopLceServiceResponse()
     */
    virtual ::android::hardware::Return<void> stopLceService(int32_t serial) = 0;

    /**
     * Pull LCE service for capacity information.
     * 
     * @param serial Serial number of request.
     * 
     * Response callback is IRadioResponse.pullLceDataResponse()
     */
    virtual ::android::hardware::Return<void> pullLceData(int32_t serial) = 0;

    /**
     * Get modem activity information for power consumption estimation.
     * Request clear-on-read statistics information that is used for
     * estimating the per-millisecond power consumption of the cellular
     * modem.
     * 
     * @param serial Serial number of request.
     * 
     * Response callback is IRadioResponse.getModemActivityInfoResponse()
     */
    virtual ::android::hardware::Return<void> getModemActivityInfo(int32_t serial) = 0;

    /**
     * Set carrier restrictions. Expected modem behavior:
     *  If never receives this command
     *  - Must allow all carriers
     *  Receives this command with allAllowed true
     *  - Must allow all carriers. If a previously allowed SIM is present, modem must not reload
     *    the SIM. If a previously disallowed SIM is present, reload the SIM and notify Android.
     *  Receives this command with a list of carriers & allAllowed = false
     *  - Only allow specified carriers, persist across power cycles and FDR. If a present SIM
     *    is in the allowed list, modem must not reload the SIM. If a present SIM is *not* in
     *    the allowed list, modem must detach from the registered network and only keep emergency
     *    service, and notify Android SIM refresh reset with new SIM state being
     *    CardState:RESTRICTED. Emergency service must be enabled.
     * 
     * @param serial Serial number of request.
     * @param allAllowed true only when all carriers are allowed. Ignore "carriers" struct.
     *        If false, consider "carriers" struct
     * @param carriers CarrierRestrictions consisting allowed and excluded carriers as defined
     *        in types.hal
     * 
     * Response callback is IRadioResponse.setAllowedCarriersResponse()
     */
    virtual ::android::hardware::Return<void> setAllowedCarriers(int32_t serial, bool allAllowed, const ::android::hardware::radio::V1_0::CarrierRestrictions& carriers) = 0;

    /**
     * Get carrier restrictions.
     * 
     * @param serial Serial number of request.
     * 
     * Response callback is IRadioResponse.getAllowedCarriersResponse()
     */
    virtual ::android::hardware::Return<void> getAllowedCarriers(int32_t serial) = 0;

    /**
     * Send the updated device state.
     * This is providing the device state information for the modem to perform power saving
     * strategies.
     * 
     * @param serial Serial number of request.
     * @param deviceStateType The updated device state type.
     * @param state The updated state. See the definition of state at DeviceStateType.
     * 
     * Response callback is IRadioResponse.sendDeviceStateResponse()
     */
    virtual ::android::hardware::Return<void> sendDeviceState(int32_t serial, ::android::hardware::radio::V1_0::DeviceStateType deviceStateType, bool state) = 0;

    /**
     * Set the indication filter.
     * This is used to prevent unnecessary application processor wake up for power saving purposes
     * by suppressing the indications in certain scenarios.
     * 
     * @param serial Serial number of request.
     * @param indicationFilter 32-bit bitmap of IndicationFilter. Bits set to 1 indicate the
     *        indications are enabled. See IndicationFilter for the definition of each bit.
     * 
     * Response callback is IRadioResponse.setIndicationFilterResponse()
     */
    virtual ::android::hardware::Return<void> setIndicationFilter(int32_t serial, ::android::hardware::hidl_bitfield<::android::hardware::radio::V1_0::IndicationFilter> indicationFilter) = 0;

    /**
     * Set SIM card power state.
     * Request is equivalent to inserting or removing the card.
     * 
     * The radio modem must generate IRadioIndication.simStatusChanged() as if the SIM had been
     * inserted or removed.
     * 
     * @param serial Serial number of request
     * @param powerUp True if powering up the sim card
     * 
     * Response callback is IRadioResponse.setSimCardPowerResponse()
     */
    virtual ::android::hardware::Return<void> setSimCardPower(int32_t serial, bool powerUp) = 0;

    /**
     * When response type received from a radio indication or radio response is
     * RadioIndicationType:UNSOLICITED_ACK_EXP or RadioResponseType:SOLICITED_ACK_EXP respectively,
     * acknowledge the receipt of those messages by sending responseAcknowledgement().
     * 
     */
    virtual ::android::hardware::Return<void> responseAcknowledgement() = 0;

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
     * IRadioResponse.setCarrierInfoForImsiEncryptionResponse()
     */
    virtual ::android::hardware::Return<void> setCarrierInfoForImsiEncryption(int32_t serial, const ::android::hardware::radio::V1_1::ImsiEncryptionInfo& imsiEncryptionInfo) = 0;

    /**
     * Set SIM card power state.
     * Request is equivalent to inserting or removing the card.
     * 
     * The radio modem must generate IRadioIndication.simStatusChanged() as if the SIM had been
     * inserted or removed.
     * 
     * @param serial Serial number of request
     * @param powerUp POWER_DOWN if powering down the SIM card,
     *                POWER_UP if powering up the SIM card,
     *                POWER_UP_PASS_THROUGH if powering up the SIM card in pass through mode.
     * When SIM card is in POWER_UP_PASS_THROUGH, the modem does not send any command to it
     * (for example SELECT of MF, or TERMINAL CAPABILITY), and the SIM card is controlled
     * completely by Telephony sending APDUs directly. The SIM card state must be
     * RIL_CARDSTATE_PRESENT and the number of card apps will be 0.
     * No new error code is generated. Emergency calls are supported in the same way as if the
     * SIM card is absent.
     * POWER_UP_PASS_THROUGH mode is valid only for the specific card session where it is activated,
     * and normal behavior occurs at the next SIM initialization, unless POWER_UP_PASS_THROUGH mode
     * is requested again.
     * The device is required to power down the SIM card before it can switch the mode between
     * POWER_UP and POWER_UP_PASS_THROUGH.
     * At device power up, the SIM interface is powered up automatically. Each subsequent request
     * to this method is processed only after the completion of the previous one.
     * 
     * Response callback is IRadioResponse.setSimCardPowerResponse_1_1()
     */
    virtual ::android::hardware::Return<void> setSimCardPower_1_1(int32_t serial, ::android::hardware::radio::V1_1::CardPowerState powerUp) = 0;

    /**
     * Starts a network scan
     * 
     * @param serial Serial number of request.
     * @param request Defines the radio networks/bands/channels which need to be scanned.
     * 
     * Response function is IRadioResponse.startNetworkScanResponse()
     */
    virtual ::android::hardware::Return<void> startNetworkScan(int32_t serial, const ::android::hardware::radio::V1_1::NetworkScanRequest& request) = 0;

    /**
     * Stops ongoing network scan
     * 
     * @param serial Serial number of request.
     * 
     * Response function is IRadioResponse.stopNetworkScanResponse()
     */
    virtual ::android::hardware::Return<void> stopNetworkScan(int32_t serial) = 0;

    /**
     * Start a Keepalive session (for IPsec)
     * 
     * @param serial Serial number of request.
     * @param keepalive A request structure containing all necessary info to describe a keepalive
     * 
     * Response function is IRadioResponse.startKeepaliveResponse()
     */
    virtual ::android::hardware::Return<void> startKeepalive(int32_t serial, const ::android::hardware::radio::V1_1::KeepaliveRequest& keepalive) = 0;

    /**
     * Stop an ongoing Keepalive session (for IPsec)
     * 
     * @param serial Serial number of request.
     * @param sessionHandle The handle that was provided by IRadioResponse.startKeepaliveResponse
     * 
     * Response function is IRadioResponse.stopKeepaliveResponse()
     */
    virtual ::android::hardware::Return<void> stopKeepalive(int32_t serial, int32_t sessionHandle) = 0;

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
    static ::android::hardware::Return<::android::sp<::android::hardware::radio::V1_1::IRadio>> castFrom(const ::android::sp<::android::hardware::radio::V1_1::IRadio>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<::android::hardware::radio::V1_1::IRadio>> castFrom(const ::android::sp<::android::hardware::radio::V1_0::IRadio>& parent, bool emitError = false);
    static ::android::hardware::Return<::android::sp<::android::hardware::radio::V1_1::IRadio>> castFrom(const ::android::sp<::android::hidl::base::V1_0::IBase>& parent, bool emitError = false);

    static const char* descriptor;

    static ::android::sp<IRadio> tryGetService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<IRadio> tryGetService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return tryGetService(str, getStub); }
    static ::android::sp<IRadio> tryGetService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return tryGetService(str, getStub); }
    static ::android::sp<IRadio> tryGetService(bool getStub) { return tryGetService("default", getStub); }
    static ::android::sp<IRadio> getService(const std::string &serviceName="default", bool getStub=false);
    static ::android::sp<IRadio> getService(const char serviceName[], bool getStub=false)  { std::string str(serviceName ? serviceName : "");      return getService(str, getStub); }
    static ::android::sp<IRadio> getService(const ::android::hardware::hidl_string& serviceName, bool getStub=false)  { std::string str(serviceName.c_str());      return getService(str, getStub); }
    static ::android::sp<IRadio> getService(bool getStub) { return getService("default", getStub); }
    __attribute__ ((warn_unused_result))::android::status_t registerAsService(const std::string &serviceName="default");
    static bool registerForNotifications(
            const std::string &serviceName,
            const ::android::sp<::android::hidl::manager::V1_0::IServiceNotification> &notification);
};

std::string toString(const ::android::sp<::android::hardware::radio::V1_1::IRadio>& o);

}  // namespace V1_1
}  // namespace radio
}  // namespace hardware
}  // namespace android

#endif  // HIDL_GENERATED_ANDROID_HARDWARE_RADIO_V1_1_IRADIO_H
