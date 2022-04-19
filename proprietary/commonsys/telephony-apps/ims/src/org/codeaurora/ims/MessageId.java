/*
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 */

/*
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.codeaurora.ims;

/**
 * Random numbers are added to all the constants in MessageId class.
 */
public class MessageId {

    private MessageId() {}

    // Request message IDs
    public static final int UNKNOWN_REQ = 0;

    /**
     * MsgType: REQUEST ,MsgId: REQUEST_IMS_REGISTRATION_STATE, Error:E_UNUSED ,
     *          Message: NULL
     *
     * MsgType: RESPONSE, MsgId: REQUEST_IMS_REGISTRATION_STATE, Error:Valid errors
     *          Message: message Registration
     *
     * Description:Request current IMS registration state
     *
     * Valid errors:
     *  E_SUCCESS
     *  E_RADIO_NOT_AVAILABLE
     *  E-GENERIC_FAILURE
     */
    public static final int REQUEST_IMS_REGISTRATION_STATE = 1;

    /**
     * MsgType: REQUEST ,MsgId: REQUEST_DIAL, Error:E_UNUSED ,
     *          Message: message Dial
     *
     * MsgType: RESPONSE, MsgId: REQUEST_DIAL, Error:Valid errors
     *          Message: is NULL
     *
     * Description: Initiate voice call
     *
     * This method is never used for supplementary service codes
     *
     * Valid errors:
     *  E_SUCCESS
     *  E_RADIO_NOT_AVAILABLE
     *  GENERIC_FAILURE
     */
    public static final int REQUEST_DIAL = 2;

    /**
     * REQUEST_ANSWER
     * MsgType: REQUEST
     *
     * Answer incoming call
     *
     * Will not be called for WAITING calls.
     * REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE will be used in this case
     * instead
     *
     * "data" is enum Calltype
     * "response" is NULL
     *
     * Valid errors:
     *  E_SUCCESS
     *  E_RADIO_NOT_AVAILABLE (radio resetting)
     *  E_GENERIC_FAILURE
     */
    public static final int REQUEST_ANSWER = 3;

    /**
     * REQUEST_HANGUP
     * MsgType: REQUEST
     *
     * Hang up a specific line (like AT+CHLD=1x)
     *
     * After this HANGUP request returns, connection will NOT
     * be active anymore in next REQUEST_GET_CURRENT_CALLS query.
     *
     * "data" is message Hangup
     *
     * "response" is NULL
     *
     * Valid errors:
     *  E_SUCCESS
     *  E_RADIO_NOT_AVAILABLE (radio resetting)
     *  E_GENERIC_FAILURE
     */
    public static final int REQUEST_HANGUP = 4;

    /**
     * REQUEST_LAST_CALL_FAIL_CAUSE
     * MsgType: REQUEST
     *
     * Requests the failure cause code for the most recently terminated call
     *
     * "data" is NULL
     * "response" is a "message CallFailCauseResponse"
     *
     * If the implementation does not have access to the exact cause codes,
     * then it should return one of the values listed in CallFailCause,
     * as the UI layer needs to distinguish these cases for tone generation or
     * error notification.
     *
     * Valid errors:
     *  E_SUCCESS
     *  E_RADIO_NOT_AVAILABLE
     *  E_GENERIC_FAILURE
     */
    public static final int REQUEST_LAST_CALL_FAIL_CAUSE = 5;

    /**
     * REQUEST_GET_CURRENT_CALLS
     * MsgType: REQUEST
     *
     * Requests current call list
     *
     * "data" is NULL
     *
     * "response" must be a "message CallList or null"
     *  null means no active calls in CallList
     *
     * Valid errors:
     *
     *  E_SUCCESS
     *  E_RADIO_NOT_AVAILABLE (radio resetting)
     *  E_GENERIC_FAILURE
     *      (request will be made again in a few hundred msec)
     */
    public static final int REQUEST_GET_CURRENT_CALLS = 6;

    /**
     * REQUEST_HANGUP_WAITING_OR_BACKGROUND
     *
     * Hang up waiting or held (like AT+CHLD=0)
     *
     * After this HANGUP request returns, the connection is NOT
     * active anymore in next REQUEST_GET_CURRENT_CALLS query.
     *
     * "data" is NULL
     * "response" is NULL
     *
     * Valid errors:
     *  E_SUCCESS
     *  E_RADIO_NOT_AVAILABLE (radio resetting)
     *  E_GENERIC_FAILURE
     */
    public static final int REQUEST_HANGUP_WAITING_OR_BACKGROUND = 7;
    public static final int REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND = 8;

    /**
     * REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE
     * MsgType: REQUEST
     *
     * For switching calls. Also used for answering waiting call
     *
     * "data" is enum Calltype
     * "response" is NULL
     *
     * Valid errors:
     *  E_SUCCESS
     *  E_RADIO_NOT_AVAILABLE (radio resetting)
     *  E_GENERIC_FAILURE
     */
    public static final int REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE = 9;

    /**
     * REQUEST_CONFERENCE
     * MsgType: REQUEST
     *
     * Request a conference operation for the calls that are alive
     *
     * "data" is NULL
     * "response" is NULL
     *
     * Valid errors:
     *  E_SUCCESS
     *  E_RADIO_NOT_AVAILABLE (radio resetting)
     *  E_GENERIC_FAILURE
     */
    public static final int REQUEST_CONFERENCE = 10;

    public static final int REQUEST_EXIT_EMERGENCY_CALLBACK_MODE = 11;

    /**
     * REQUEST_EXPLICIT_CALL_TRANSFER
     * MsgType: REQUEST
     *
     * For transfering the call to a third party.
     * The call can be transferred to third party (Transfer target) by
     * passing the transfer target address (blind or assured transfer)
     * or by passing the call id of the already established call with
     * transfer target (consultative transfer).
     * In success case, the call/calls will be disconnected.
     *
     * "data" is message ExplicitCallTransfer
     * "response" is NULL
     *
     * Valid errors:
     *  E_SUCCESS
     *  E_RADIO_NOT_AVAILABLE (radio resetting)
     *  E_GENERIC_FAILURE
     */
    public static final int REQUEST_EXPLICIT_CALL_TRANSFER = 12;

    public static final int REQUEST_DTMF = 13;
    public static final int REQUEST_DTMF_START = 14;
    public static final int REQUEST_DTMF_STOP = 15;
    public static final int REQUEST_UDUB = 16;

    public static final int REQUEST_MODIFY_CALL_INITIATE = 17;
    public static final int REQUEST_MODIFY_CALL_CONFIRM = 18;

    /**
     * REQUEST_QUERY_CALL_CLIP
     * MsgType: REQUEST
     *
     * Queries the status of the CLIP supplementary service
     *
     * (for MMI code "*#30#")
     * "data" is NULL
     * "response" is "message ClipProvisionStatus"
     *
     * Valid errors:
     *  SUCCESS
     *  RADIO_NOT_AVAILABLE (radio resetting)
     *  GENERIC_FAILURE
     *  FDN_CHECK_FAILURE
     */
    public static final int REQUEST_QUERY_CLIP = 19;

    /**
     * REQUEST_GET_CLIR
     * MsgType: REQUEST
     *
     * Gets current CLIR status
     *
     * "data" is NULL
     * "response" must be a "message Clir"
     *
     * Valid errors:
     *  SUCCESS
     *  RADIO_NOT_AVAILABLE
     *  GENERIC_FAILURE
     *  FDN_CHECK_FAILURE
     */
    public static final int REQUEST_GET_CLIR = 20;

    /**
     * REQUEST_SET_CLIR
     * MsgType: REQUEST
     *
     * Sets the Clir parameter
     *
     * "data" is "message Clir"
     * "response" is NULL
     *
     * Valid errors:
     *  SUCCESS
     *  RADIO_NOT_AVAILABLE
     *  GENERIC_FAILURE
     *  FDN_CHECK_FAILURE
     */
    public static final int REQUEST_SET_CLIR = 21;

    /**
     * REQUEST_QUERY_CALL_FORWARD_STATUS
     * MsgType: REQUEST
     *
     * Requests the call forward settings
     *
     * "data" must be a "message CallForwardInfoList"
     *
     * "response" must be a "message CallForwardInfoList or null"
     * "response" points to an array of CallForwardInfo, one for
     * each distinct registered phone number.
     *
     * For example, if data is forwarded to +18005551212 and voice is forwarded
     * to +18005559999, then two separate CallForwardInfo should be returned
     *
     * If, however, both data and voice are forwarded to +18005551212, then
     * a single CallForwardInfo can be returned with the service class
     * set to "data + voice = 3")
     *
     * Valid errors:
     *  SUCCESS
     *  RADIO_NOT_AVAILABLE
     *  GENERIC_FAILURE
     *  NETWORK_NOT_SUPPORTED
     *  FDN_CHECK_FAILURE
     */
    public static final int REQUEST_QUERY_CALL_FORWARD_STATUS = 22;

    /**
     * REQUEST_SET_CALL_FORWARD_STATUS
     * MsgType: REQUEST
     *
     * Configure call forward rule
     *
     * "data" must be a "message CallForwardInfoList"
     *
     * "response" is of type "SuppSvcResponse"
     *
     * Valid errors:
     *  SUCCESS
     *  RADIO_NOT_AVAILABLE
     *  GENERIC_FAILURE
     *  FDN_CHECK_FAILURE
     */
    public static final int REQUEST_SET_CALL_FORWARD_STATUS = 23;

    /**
     * REQUEST_QUERY_CALL_WAITING
     * MsgType: REQUEST
     *
     * Query current call waiting state
     *
     * "data" must be a "message ServiceClass"
     *
     * "response" is a "message CallWaitingInfo"
     *
     * Valid errors:
     *  SUCCESS
     *  RADIO_NOT_AVAILABLE
     *  GENERIC_FAILURE
     *  NETWORK_NOT_SUPPORTED
     *  FDN_CHECK_FAILURE
     */
    public static final int REQUEST_QUERY_CALL_WAITING = 24;

    /**
     * REQUEST_SET_CALL_WAITING
     * MsgType: REQUEST
     *
     * Configure current call waiting state
     *
     * "data" is "message CallWaitingInfo"
     * "response" is of type SuppSvcResponse
     *
     * Valid errors:
     *  SUCCESS
     *  RADIO_NOT_AVAILABLE
     *  GENERIC_FAILURE
     *  FDN_CHECK_FAILURE
     */
    public static final int REQUEST_SET_CALL_WAITING = 25;

    /**
     * MsgType: REQUEST ,MsgId: REQUEST_IMS_REG_STATE_CHANGE, Error:E_UNUSED ,
     *          Message: message Registration
     *                   enum REGISTERED - To register IMS
     *                   enum NOT_REGISTERED - To deregister IMS
     *
     * MsgType: RESPONSE, MsgId: REQUEST_IMS_REG_STATE_CHANGE, Error:Valid errors
     *          Message: NULL
     *
     * Description:Request current IMS Registration/Deregistration
     *
     * Valid errors:
     *  E_SUCCESS
     *  E_RADIO_NOT_AVAILABLE
     *  E-GENERIC_FAILURE
     */
    public static final int REQUEST_IMS_REG_STATE_CHANGE = 26;

    /**
     * REQUEST_SET_SUPP_SVC_NOTIFICATION
     * Enables/disables supplementary service related notifications
     * from the network.
     *
     * Notifications are reported via UNSOL_SUPP_SVC_NOTIFICATION.
     *
     * "data" is int *
     * ((int *)data)[0] is == 1 for notifications enabled
     * ((int *)data)[0] is == 0 for notifications disabled
     *
     * "response" is NULL
     *
     * Valid errors:
     *  E_SUCCESS
     *  E_RADIO_NOT_AVAILABLE
     *  E_GENERIC_FAILURE
     *  E_FDN_CHECK_FAILURE
     *
     * See also: UNSOL_SUPP_SVC_NOTIFICATION.
     */
    public static final int REQUEST_SET_SUPP_SVC_NOTIFICATION = 27;

    /**
     * REQUEST_ADD_PARTICIPANT
     * MsgType: Request
     * Request to add a participant to the existing conversation
     *
     * "data" is Message Dial
     *
     * "response" is message SipErrorInfo
     *
     * Valid errors:
     *  E_SUCCESS
     *
     */
    public static final int REQUEST_ADD_PARTICIPANT = 28;

    /**
     * REQUEST_QUERY_SERVICE_STATUS
     * MsgType: Request
     * Request to get the status of IMS Services
     *
     * "data" is null
     *
     * "response" is message SrvStatusList - all services must be specified in the response
     *
     * Valid errors:
     *  E_SUCCESS
     *
     */
    public static final int REQUEST_QUERY_SERVICE_STATUS = 29;

    /**
     * REQUEST_SET_SERVICE_STATUS
     * MsgType: Request
     * Request to set the status of an IMS Service
     *
     * "data" is message Info - this is a delta notification, services not listed in the
     * request must be left untouched
     *
     * "response" is null
     *
     * Valid errors:
     *  E_SUCCESS
     *
     */
    public static final int REQUEST_SET_SERVICE_STATUS = 30;

    /**
     * REQUEST_SUPP_SVC_STATUS
     * MsgType: Request
     * Request to activate/deactivate/query the supplementary service of an IMS Service
     * This is currently being used for Call Barring, CLIP, COLP
     *
     * "data" is message SuppSvcRequest
     * "response" is of type "SuppSvcResponse"
     *
     * Valid errors:
     *  SUCCESS
     *  RADIO_NOT_AVAILABLE (radio resetting)
     *  GENERIC_FAILURE
     *  FDN_CHECK_FAILURE
     *
     */
    public static final int REQUEST_SUPP_SVC_STATUS = 31;

    /**
     *
     * REQUEST_DEFLECT_CALL
     * MsgType: Request
     * Request to the deflect the call to specific number mentioned by user
     * "data" is message DeflectCall
     * "response" is - null
     *
     * Valid errors:
     * All enum Error type
     *
     */
    public static final int REQUEST_DEFLECT_CALL = 32;

    /**
     * REQUEST_GET_COLR
     * MsgType: REQUEST
     *
     * Gets current COLR status
     *
     * "data" is NULL
     * "response" is "message Colr"
     *
     * Valid errors:
     *  SUCCESS
     *  RADIO_NOT_AVAILABLE
     *  GENERIC_FAILURE
     *  FDN_CHECK_FAILURE
     */
    public static final int REQUEST_GET_COLR = 33;

    /**
     * REQUEST_SET_COLR
     * MsgType: REQUEST
     *
     * Sets the Colr parameter
     *
     * "data" is "message Colr"
     * "response" is "message SuppSvcResponse"
     *
     * Valid errors:
     *  SUCCESS
     *  RADIO_NOT_AVAILABLE
     *  GENERIC_FAILURE
     *  FDN_CHECK_FAILURE
     */
    public static final int REQUEST_SET_COLR = 34;

    /**
     * REQUEST_QUERY_VT_CALL_QUALITY
     * @deprecated
     * MsgType: REQUEST
     *
     * Query current video call quality setting.
     * This is DEPRECATED. Use REQUEST_GET_IMS_CONFIG with
     * item == CONFIG_ITEM_VIDEO_QUALITY instead.
     *
     * "data" is null
     *
     * "response" is a "message VideoCallQuality"
     *
     * Valid errors:
     *  SUCCESS
     *  GENERIC_FAILURE
     */
    public static final int REQUEST_QUERY_VT_CALL_QUALITY = 35;

    /**
     * REQUEST_SET_VT_CALL_QUALITY
     * @deprecated
     * MsgType: REQUEST
     *
     * Set setting(s). This is DEPRECATED.
     * Use REQUEST_SET_IMS_CONFIG with item ==
     * CONFIG_ITEM_VIDEO_QUALITY instead.
     *
     * "data" must be a "message VideoCallQuality"
     *
     * "response" is null
     *
     * Valid errors:
     *  SUCCESS
     *  GENERIC_FAILURE
     */
    public static final int REQUEST_SET_VT_CALL_QUALITY = 36;

    /**
     * REQUEST_HOLD
     * MsgType: REQUEST
     *
     * Set setting(s)
     *
     * "data" must be a "message Hold"
     *
     * "response" is message SipErrorInfo
     *
     * Valid errors:
     *  SUCCESS
     *  GENERIC_FAILURE
     */
    public static final int REQUEST_HOLD = 37;

    /**
     * REQUEST_RESUME
     * MsgType: REQUEST
     *
     * Set setting(s)
     *
     * "data" must be a "message Resume"
     *
     * "response" is message SipErrorInfo
     *
     * Valid errors:
     *  SUCCESS
     *  GENERIC_FAILURE
     */
    public static final int REQUEST_RESUME = 38;

    /**
     * REQUEST_SEND_UI_TTY_MODE
     * MsgType: REQUEST
     *
     * Request to set the TTY mode
     *
     * "data" is message TtyNotify
     *
     * "response" is null
     *
     * Valid errors:
     *  SUCCESS
     *  GENERIC_FAILURE
     */
    public static final int REQUEST_SEND_UI_TTY_MODE = 39;

    /**
     * REQUEST_GET_RTP_STATISTICS
     * MsgType: REQUEST
     *
     * Requests RTP statistics i.e. Packet Count
     *
     * "data" is null
     *
     * "response" is message RtpStatisticsData
     *
     * Valid errors:
     *  SUCCESS
     *  GENERIC_FAILURE
     */
    public static final int REQUEST_GET_RTP_STATISTICS = 40;

    /**
     * REQUEST_GET_RTP_ERROR_STATISTICS
     * MsgType: REQUEST
     *
     * Requests RTP error statistics i.e. Packet Error Count
     *
     * "data" is null
     *
     * "response" is message RtpStatisticsData
     *
     * Valid errors:
     *  SUCCESS
     *  GENERIC_FAILURE
     */
    public static final int REQUEST_GET_RTP_ERROR_STATISTICS = 41;

    /**
     * REQUEST_GET_WIFI_CALLING_STATUS
     * MsgType: REQUEST
     *
     * Request to get the wifi calling settings and preference.
     *
     * "data" is null
     *
     * "response" is message WifiCallingInfo
     *
     * Valid errors:
     *  SUCCESS
     *  GENERIC_FAILURE
     */
    public static final int REQUEST_GET_WIFI_CALLING_STATUS = 42;

    /**
     * REQUEST_SET_WIFI_CALLING_STATUS
     * MsgType: REQUEST
     *
     * Request to set the wifi calling settings and preference.
     *
     * "data" is message WifiCallingInfo
     *
     * "response" is null
     *
     * Valid errors:
     *  SUCCESS
     *  GENERIC_FAILURE
     */
    public static final int REQUEST_SET_WIFI_CALLING_STATUS = 43;

    /**
     * REQUEST_SET_IMS_CONFIG
     * MsgType: REQUEST
     *
     * Sets value for Ims Config item
     *
     * "data" is message ConfigMsg
     *
     * "response" is message ConfigMsg
     *
     * Valid errors:
     *  SUCCESS
     *  GENERIC_FAILURE
     */
    public static final int REQUEST_SET_IMS_CONFIG = 44;

    /**
     * REQUEST_GET_IMS_CONFIG
     * MsgType: REQUEST
     *
     * Gets Ims Config item value
     *
     * "data" is message ConfigMsg
     *
     * "response" is message ConfigMsg
     *
     * Valid errors:
     *  SUCCESS
     *  GENERIC_FAILURE
     */
    public static final int REQUEST_GET_IMS_CONFIG = 45;

    /**
     * REQUEST_SEND_GEOLOCATION_INFO
     * MsgType: REQUEST
     *
     * Send GeoLocation Address info.
     *
     * "data" is message GeoLocationInfo
     *
     * "response" is null
     *
     * Valid errors:
     *  SUCCESS
     *  GENERIC_FAILURE
     */
    public static final int REQUEST_SEND_GEOLOCATION_INFO = 46;

    /**
     * REQUEST_GET_VOWIFI_CALL_QUALITY
     * MsgType: REQUEST
     *
     * Gets VoWiFI quality for current active call
     *
     * "data" is message null
     *
     * "response" is message VoWiFiCallQuality
     *
     * Valid errors:
     *  SUCCESS
     *  GENERIC_FAILURE
     */
    public static final int REQUEST_GET_VOWIFI_CALL_QUALITY = 47;

    /**
     * REQUEST_GET_VOPS_INFO
     * MsgType: REQUEST
     *
     * Query VOPS value
     *
     * "data" is null
     *
     * "response" is VopsInfo
     * True = Voice supported on LTE
     * False = Voice not supported on LTE
     *
     * Valid errors:
     *  SUCCESS
     *  GENERIC_FAILURE
     */
    public static final int REQUEST_GET_VOPS_INFO = 48;

    /**
     * REQUEST_GET_SSAC_INFO
     * MsgType: REQUEST
     *
     * Query SSAC value
     *
     * "data" is null
     *
     * "response" is of type SsacInfo
     *
     * Valid errors:
     *  SUCCESS
     *  GENERIC_FAILURE
     */
    public static final int REQUEST_GET_SSAC_INFO = 49;

    /**
     * REQUEST_SET_VOLTE_PREF
     * MsgType: REQUEST
     *
     * Update VoLTE preference to lower layers
     *
     * "data" is of type VolteUserPref
     *
     * "response" is null
     *
     * Valid errors:
     *  SUCCESS
     *  GENERIC_FAILURE
     */
    public static final int REQUEST_SET_VOLTE_PREF = 50;

    /**
     * REQUEST_GET_VOLTE_PREF
     * MsgType: REQUEST
     *
     * Query VoLTE preference from lower layers
     *
     * "data" is null
     *
     * "response" is of type VolteUserPref
     *
     * Valid errors:
     *  SUCCESS
     *  GENERIC_FAILURE
     */
    public static final int REQUEST_GET_VOLTE_PREF = 51;

    /**
     * REQUEST_GET_HANDOVER_CONFIG
     * MsgType: REQUEST
     *
     * Get IMS Handover status between WWAN and WLAN
     *
     * "data" is null
     *
     * "response" is HandoverConfigMsg
     *
     * Valid errors:
     *  SUCCESS
     *  GENERIC_FAILURE
     */
    public static final int REQUEST_GET_HANDOVER_CONFIG = 52;

    /**
     * REQUEST_SET_HANDOVER_CONFIG
     * MsgType: REQUEST
     *
     * Set IMS Handover status between WWAN and WLAN
     *
     * "data" is HandoverConfigMsg
     *
     * "response" is null
     *
     * Valid errors:
     *  SUCCESS
     *  GENERIC_FAILURE
     */
    public static final int REQUEST_SET_HANDOVER_CONFIG = 53;

    /**
     * REQUEST_GET_IMS_SUB_CONFIG
     * MsgType: REQUEST
     *
     * Get IMS Subscription specific configuration
     *
     * "data" is message null
     *
     * "response" is message ImsSubConfig
     *
     * Valid errors:
     *  SUCCESS
     *  GENERIC_FAILURE
     */
    public static final int REQUEST_GET_IMS_SUB_CONFIG = 54;

    /**
     * REQUEST_SEND_RTT_MSG
     * MsgType: REQUEST
     *
     * Request to send the RTT message
     *
     * "data" is message RttMessage
     *
     * "response" is null
     *
     * Valid errors:
     *  SUCCESS
     *  GENERIC_FAILURE
     */
    public static final int REQUEST_SEND_RTT_MSG = 55;

    /**
     * REQUEST_CANCEL_MODIFY_CALL
     * MsgType: REQUEST
     *
     * Cancel call modify request
     *
     * "data" is of type CancelModifyCall
     *
     * "response" is null
     *
     * Valid errors:
     *  SUCCESS
     *  GENERIC_FAILURE
     */
    public static final int REQUEST_CANCEL_MODIFY_CALL = 56;

    /**
     * MsgType: REQUEST
     *
     * Description:Request submit sms over ims.
     */
    public static final int REQUEST_SEND_IMS_SMS = 57;

    /**
     * MsgType: REQUEST
     *
     * Description:Request delivery report of
     * incoming sms over ims.
     */
    public static final int REQUEST_ACK_IMS_SMS = 58;

    /**
     * MsgType: REQUEST
     *
     * Description:Request delivery status report of sms over ims.
     */
    public static final int REQUEST_ACK_IMS_SMS_STATUS_REPORT = 59;

    /**
     * MsgType: REQUEST
     *
     * Description:Request MultiIdentity Lines Registration
     */
    public static final int REQUEST_REGISTER_MULTI_IDENTITY_LINES = 60;

    /**
     * MsgType: REQUEST
     *
     * Description:Request Virtual Line Info
     */
    public static final int REQUEST_QUERY_VIRTUAL_LINE_INFO = 61;

    /**
     * MsgType: REQUEST
     *
     * Description:Request to dial emergency call
     */
    public static final int REQUEST_EMERGENCY_DIAL = 62;

    public static final int UNSOL_RSP_BASE = 200;

    public static final int UNSOL_RESPONSE_CALL_STATE_CHANGED = 201;
    public static final int UNSOL_CALL_RING = 202;
    public static final int UNSOL_RINGBACK_TONE = 203;

    /**
     * UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED
     *
     * Notification for change in ims registration state
     * Registration failures reported through this API are temporary.
     *
     * "response" is Registration
     */
    public static final int UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED = 204;

    public static final int UNSOL_ENTER_EMERGENCY_CALLBACK_MODE = 205;
    public static final int UNSOL_EXIT_EMERGENCY_CALLBACK_MODE = 206;

    public static final int UNSOL_MODIFY_CALL = 207;

    /**
     * UNSOL_RESPONSE_HANDOVER
     *
     * Notification for handover status.
     * Registration handover failures reported through this API are temporary.
     *
     * "response" is message Handover
     */
    public static final int UNSOL_RESPONSE_HANDOVER = 208;


    /**
     * UNSOL_REFRESH_CONF_INFO
     *
     * Notification for updated conference information
     *
     * "response" is message ConfInfo
     */
    public static final int UNSOL_REFRESH_CONF_INFO = 209;

    /**
     * UNSOL_SRV_STATUS_UPDATE
     *
     * Status of services supported on Ims network. Currently supported services are sms, volte,
     * VT but interface is extensible if more services are added in future
     *
     * "response" is message SrvStatusList
     */
    public static final int UNSOL_SRV_STATUS_UPDATE = 210;

    /**
     * UNSOL_SUPP_SVC_NOTIFICATION
     *
     * Reports supplementary service related notification from the network.
     *
     * "data" is a const SuppSvcNotification *
     *
     */
    public static final int UNSOL_SUPP_SVC_NOTIFICATION = 211;


    /**
     * UNSOL_TTY_NOTIFICATION
     *
     * Reports Text Telephone related notification from the network.
     *
     * "response" is message TtyNotify
     *
     */
    public static final int UNSOL_TTY_NOTIFICATION = 212;

    /**
     * UNSOL_RADIO_STATE_CHANGED
     *
     * Notification for radio state change
     *
     * "response" is message RadioStateChanged
     */
    public static final int UNSOL_RADIO_STATE_CHANGED = 213;

    /**
     * UNSOL_MWI
     *
     * Notification for message waiting indication
     *
     * "response" is message Mwi
     */
    public static final int UNSOL_MWI = 214;

    /**
     * UNSOL_REFRESH_VICE_INFO
     *
     * Notification for updated vice information
     *
     * "response" is message ViceInfo
     */
    public static final int UNSOL_REFRESH_VICE_INFO = 215;

    /**
     * UNSOL_REQUEST_GEOLOCATION
     *
     * Request from lower layers for GeoLocation address
     * information.
     *
     * 'request' is GeoLocationInfo
     */
    public static final int UNSOL_REQUEST_GEOLOCATION = 216;

    /**
     * UNSOL_VOWIFI_CALL_QUALITY
     *
     * Notification for VoWiFi call quality information
     *
     * "response" is message VoWiFiCallQuality
     */
    public static final int UNSOL_VOWIFI_CALL_QUALITY = 217;

    /**
     * UNSOL_VOPS_CHANGED
     *
     * Request from lower layers for Vops state change
     *
     * 'response' is VopsInfo
     */
    public static final int UNSOL_VOPS_CHANGED = 218;

    /**
     * UNSOL_SSAC_CHANGED
     *
     * Notification for SSAC state change
     *
     * 'response' is message SsacInfo
     */
    public static final int UNSOL_SSAC_CHANGED = 219;

    /**
     * UNSOL_PARTICIPANT_STATUS_INFO
     *
     * Notification for participant status information
     *
     * 'response' is message ParticipantStatusInfo
     */
    public static final int UNSOL_PARTICIPANT_STATUS_INFO = 220;

    /**
     * UNSOL_IMS_SUB_CONFIG_CHANGED
     *
     * Notification for IMS Subscription configuration changes
     *
     * 'response' is message ImsSubConfig
     */
    public static final int UNSOL_IMS_SUB_CONFIG_CHANGED = 221;

    /**
     * UNSOL_RESPONSE_REGISTRATION_BLOCK_STATUS
     *
     * Notification for permanent ims registration failure.
     * Permanent registration failure happens when modem has exhausted all its
     * retries. At this point modem will stop attempting to register until user
     * toggles the registration state or network reports favorable conditions for
     * registration.
     *
     * 'response' is message RegistrationBlockStatus
     */
    public static final int UNSOL_RESPONSE_REGISTRATION_BLOCK_STATUS = 222;

    /**
     * UNSOL_RESPONSE_RTT_MSG_RECEIVED
     *
     * Notification for received RTT message
     *
     * 'response' is message RttMessage
     */
    public static final int UNSOL_RESPONSE_RTT_MSG_RECEIVED = 223;

    /**
     * UNSOL_ON_SS
     *
     * Called when SS response is received when DIAL/USSD/SS is changed to SS by
     * call control.
     *
     * "data" is const StkCcUnsolSsResult @types.hal
     */
    public static final int UNSOL_ON_SS = 224;

     /**
     * MsgType: INDICATION
     *
     * Description:Indication of status report for sent sms over ims.
     */
    public static final int UNSOL_IMS_SMS_STATUS_REPORT = 225;

    /**
     * MsgType: INDICATION
     *
     * Description:Indication of delivering sms over ims.
     */
    public static final int UNSOL_INCOMING_IMS_SMS = 226;


    /**
     * MsgType: INDICATION
     *
     * Description:Indication of auto call rejections .
     */
    public static final int UNSOL_AUTO_CALL_REJECTION_IND = 227;

    /**
     *  UNSOL_VOICE_INFO
     *
     * MsgType: INDICATION
     *
     * Description: Notification for RTT voice info indicating speech or silence from remote user
     *
     * "data" is VoiceInfo @types.hal
     */
    public static final int UNSOL_VOICE_INFO = 228;

    /**
     * MsgType: INDICATION
     *
     * Description: Indication of Registration state changes of Multi Identity Lines.
     */
    public static final int UNSOL_MULTI_IDENTITY_REGISTRATION_STATUS_CHANGE = 229;

    /**
     * MsgType: INDICATION
     *
     * Description: Indication that Modem needs information about the Multi Identity Lines.
     */
    public static final int UNSOL_MULTI_IDENTITY_INFO_PENDING = 230;

    /**
     * UNSOL_MODEM_SUPPORTS_WFC_ROAMING_MODE
     *
     * MsgType: INDICATION
     *
     * Description: Indication if modem supports separate WFC roaming mode configuration.
     *
     */
    public static final int UNSOL_MODEM_SUPPORTS_WFC_ROAMING_MODE  = 231;

    /**
     * UNSOL_USSD_FAILED
     *
     * MsgType: INDICATION
     *
     * Description: Indication if modem sends IMS USSD failure.
     *
     */
    public static final int UNSOL_USSD_FAILED = 232;
}
