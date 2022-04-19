/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
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

public class ImsConfigItem {

    //enum ConfigItem
    public static final int NONE = 0;

    /* Value for this item is: (String)
     * String consisting of 0-7 seperated
     * by comma "," e.g., "1,5,7"
     * Reference for 0-7 values:
     * Values:
     *   - 0  -- 4.75 kbps
     *   - 1  -- 5.15 kbps
     *   - 2  -- 5.9 kbps
     *   - 3  -- 6.17 kbps
     *   - 4 -- 7.4 kbps
     *   - 5 -- 7.95 kbps
     *   - 6 -- 10.2 kbps
     *   - 7 -- 12.2 kbps
     * Spec reference: RFC 4867
     */
    public static final int VOCODER_AMRMODESET = 1;

    /* Value for this Item is: (String)
     * String consisting of 0-8 seperated
     * by comma "," e.g., "2,6,8"
     * Reference for 0-8 values:
     * Values:
     *   - 0  -- 6.60 kbps
     *   - 1  -- 8.85 kbps
     *   - 2  -- 12.65 kbps
     *   - 3  -- 14.25 kbps
     *   - 4 -- 15.85 kbps
     *   - 5 -- 18.25 kbps
     *   - 6 -- 19.85 kbps
     *   - 7 -- 23.05 kbps
     *   - 8 -- 23.85 kbps
     * Spec reference: RFC 4867
     */
    public static final int VOCODER_AMRWBMODESET = 2;

    /* Value for this item is: (Int)
     * SIP Session duration, in seconds
     */
    public static final int SIP_SESSION_TIMER = 3;

    /* Value for this item is: (Int)
     * Minimum allowed value for session timer, in seconds
     */
    public static final int MIN_SESSION_EXPIRY = 4;

    /* Value for this item is: (Int)
     * SIP timer operator mode A, in seconds
     * Valid Range: 0 - 30
     * By default 6 seconds is used.
     */
    public static final int CANCELLATION_TIMER = 5;

    /* Value for this item is: (Int)
     * Ims Registration wait time when iRAT transitions from eHRPD to LTE,
     * in seconds.
     */
    public static final int T_DELAY = 6;

    /* Value for this item is: (Int)
     * Flag that allows a device to silently redial over 1xRTT.
     * If this is not included in the request, a value of TRUE
     * (i.e., enabled) is used.
     * Value:
     *   - 0 -- Disable
     *   - 1 -- Enable
     */
    public static final int SILENT_REDIAL_ENABLE = 7;

    /* Value for this item is: (Int)
     * RTT estimate, in milliseconds.
     */
    public static final int SIP_T1_TIMER = 8;

    /* Value for this item is: (Int)
     * Maximum retransmit interval, in milliseconds,
     * for non-invite requests
     * and invite responses.
     */
    public static final int SIP_T2_TIMER = 9;

    /* Value for this item is: (Int)
     * Non-invite transaction timeout timer, in milliseconds.
     */
    public static final int SIP_TF_TIMER = 10;

    /* Value for this item is: (Int)
     * Enable VoLTE Support through Client Provisioning
     * Values:
     *   - 0 -- Disable
     *   - 1 -- Enable
     */
    public static final int VLT_SETTING_ENABLED = 11;

    /* Value for this item is: (Int)
     * Enable VT Support through Client Provisioning
     * Values:
     *   - 0 -- Disable
     *   - 1 -- Enable
     */
    public static final int LVC_SETTING_ENABLED = 12;

    /* Value for this item is: (String)
     * Ims Domain Name
     */
    public static final int DOMAIN_NAME = 13;

    /* Value for this item is: (Int)
     * Sms Format.
     * Values:
     *   - 0 -- 3GPP2
     *   - 1 -- 3GPP
     */
    public static final int SMS_FORMAT = 14;

    /* Value for this item is: (Int)
     * SMS over IP network indication flag
     * Values:
     *    - 0 -- Turn off MO SMS
     *    - 1 -- Turn on MO SMS
     */
    public static final int SMS_OVER_IP = 15;

    /* Value for this item is: (Int)
     * Publish timer, in seconds, when publish is sent on
     * an IMS network using 4G Radio access technology
     */
    public static final int PUBLISH_TIMER = 16;

    /* Value for this item is: (Int)
     * Publish extended timer, in seconds,
     * when publish is sent on an ims
     * network in non-4G RAT or
     * when in Airplane powerdown mode in a 4G RAT
     */
    public static final int PUBLISH_TIMER_EXTENDED = 17;

    /* Value for this item is: (Int)
     * Duration of time, in seconds,
     * for which the retrieved capability
     * is considered valid
     */
    public static final int CAPABILITIES_CACHE_EXPIRATION = 18;

    /* Value for this item is: (Int)
     * Duration of time, in seconds,
     * for which the retrieved availability
     * is considered valid
     */
    public static final int AVAILABILITY_CACHE_EXPIRATION = 19;

    /* Value for this item is: (Int)
     * Duration of time, in seconds,
     * between successive capability polling
     */
    public static final int CAPABILITIES_POLL_INTERVAL = 20;

    /* Value for this item is: (Int)
     * Duration, in seconds,
     * between successive publish requests
     */
    public static final int SOURCE_THROTTLE_PUBLISH = 21;

    /* Value for this item is: (Int)
     * Maximum number of entries that
     * can be kept in the list subscription
     */
    public static final int MAX_NUM_ENTRIES_IN_RCL = 22;

    /* Value for this item is: (Int)
     * Expiry timer value, in seconds,
     * for the list subscription req
     */
    public static final int CAPAB_POLL_LIST_SUB_EXP = 23;

    /* Value for this item is: (Int)
     * Gzip compression enable flag
     * Values:
     *   - 0 -- Disable
     *   - 1 -- Enable
     */
    public static final int GZIP_FLAG = 24;

    /* Value for this item is: (Int)
     * Enable presence support through client provisioning
     * Values:
     *   - 0 -- Disable
     *   - 1 -- Enable
     */
    public static final int EAB_SETTING_ENABLED = 25;

    /* Value for this item is: (Int)
     * Mobile Data Status
     * Values:
     *   - 0 -- Disable
     *   - 1 -- Enable
     */
    public static final int MOBILE_DATA_ENABLED = 26;

    /* Value for this item is: (int)
     * Wi-Fi Calling Status
     * Values:
     *   - 0 -- NOT_SUPPORTED
     *   - 1 -- WIFI_CALLING_ON
     *   - 2 -- WIFI_CALLING_OFF
     */
    public static final int VOICE_OVER_WIFI_ENABLED = 27;

    /* Value for this item is: (Int)
     * Wi-Fi Call Roaming status.
     * Values:
     *   - 1 -- ROAMING_ENABLED
     *   - 0 -- ROAMING_DISABLED
     *
     * Note: Generic error will be returned
     * is the value is ROAMING_NOT_SUPPORTED
     */
    public static final int VOICE_OVER_WIFI_ROAMING = 28;

    /* Value for this item is: (Int)
     * Wi-Fi Call Mode Preference
     * Values:
     *   - 0 -- WIFI_ONLY/WLAN_ONLY
     *   - 1 -- CELLULAR_PREFERRED
     *   - 2 -- WIFI_PREFERRED/WLAN_PREFERRED
     *   - 10 -- IMS_PREFERRED
     *
     * Note: Generic error will be returned
     * if the value is not any of the above.
     */
    public static final int VOICE_OVER_WIFI_MODE = 29;

    /* Value for this item is: (Int)
     * Flag indicating whether Discovery
     * Capability is enabled
     * Values:
     *   - 0 -- Disable
     *          Presence doesnot publish/subscribe
     *          and ignores any notification received
     *
     *   - 1 -- Enable
     *          Presence publishes/subscribes and
     *          processes any notification received
     *
     * NOTE: Not supported currently
     */
    public static final int CAPABILITY_DISCOVERY_ENABLED = 30;

    /* Value for this item is: (Int)
     * Emergenciy call timer
     *
     * NOTE: Not supported currently
     */
    public static final int EMERGENCY_CALL_TIMER = 31;

    /* Value for this item is: (Int)
     * When UE moves from non-Voice area to Voice
     * area, the device shall start a hysteresis timer
     * and domain selection will not notify IMS about
     * voice preference until the expiry of hysteresis
     * timer
     *
     * NOTE: Not supported currently
     */
    public static final int SSAC_HYSTERESIS_TIMER = 32;

    /* VoLTE user opted in status.
     * Value is in Integer format.
     * Opted-in (1) Opted-out (0).
     */
    public static final int VOLTE_USER_OPT_IN_STATUS = 33;

    /* Proxy for Call Session Control Function(P-CSCF)
     * address for Local-BreakOut(LBO).
     * Value is in String format.
     */
    public static final int LBO_PCSCF_ADDRESS = 34;

    /* Keep Alive Enabled for SIP.
     * Value is in Integer format. On(1), OFF(0).
     */
    public static final int KEEP_ALIVE_ENABLED = 35;

    /* Registration retry Base Time value
     * in seconds.
     * Value is in Integer format.
     */
    public static final int REGISTRATION_RETRY_BASE_TIME_SEC = 36;

    /* Registration retry Max Time value
     * in seconds.
     * Value is in Integer format.
     */
    public static final int REGISTRATION_RETRY_MAX_TIME_SEC = 37;

    /* Smallest RTP port for speech codec.
     * Value is in integer format.
     */
    public static final int SPEECH_START_PORT = 38;

    /* Largest RTP port for speech code.
     * Value is in Integer format.
     */
    public static final int SPEECH_END_PORT = 39;

    /* SIP Timer A's value in msec.
     * Timer A is the INVITE request
     * retransmit interval, UDP only.
     * Value is in Integer format.
     */
    public static final int SIP_INVITE_REQ_RETX_INTERVAL_MSEC = 40;

    /* SIP Timer B's value in msec.
     * Timer B is the wait time for
     * INVITE message to be acknowledged.
     * Value is in Integer format.
     */
    public static final int SIP_INVITE_RSP_WAIT_TIME_MSEC = 41;

    /* SIP Timer D's value in msec.
     * Timer D is the wait time for
     * response retransmits of the
     * invite client transactions.
     * Value is in Integer format.
     */
    public static final int SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC = 42;

    /* SIP Timer E's value in msec.
     * Timer E is the value
     * Non-INVITE request
     * retransmit interval, for
     * UDP only.
     * Value in Integer format.
     */
    public static final int SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC = 43;

    /* SIP Timer F's value in msec
     * Timer F is the value of
     * Non-INVITE transaction
     * timeout timer.
     * Value is in Integer format.
     */
    public static final int SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC = 44;

    /* SIP Timer G's value in msec.
     * Timer G is the value of INVITE
     * response retransmit interval.
     * Value is in Integer format.
     */
    public static final int SIP_INVITE_RSP_RETX_INTERVAL_MSEC = 45;

    /* SIP Timer H's value in msec.
     * Timer H is value of wait time
     * for ACK receipt.
     * Value is in Integer format.
     */
    public static final int SIP_ACK_RECEIPT_WAIT_TIME_MSEC = 46;

    /* SIP Timer I's value in msec.
     * Timer I is value of wait time for
     * ACK retransmits.
     * Value is in Integer format.
     */
    public static final int SIP_ACK_RETX_WAIT_TIME_MSEC = 47;

    /* SIP Timer J's value in
     * msec. Timer J is the
     * value of wait time for
     * non-invite request
     * retransmission.
     * Value is in Integer
     * format.
     */
    public static final int SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC = 48;

    /* SIP Timer K's value in
     * msec. Timer K is the
     * value of wait time for
     * non-invite response
     * retransmits.
     * Value is in Integer
     * format.
     */
    public static final int SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC = 49;

    /* AMR WB octet aligned dynamic payload
     * type.
     * Value is in Integer format.
     */
    public static final int AMR_WB_OCTET_ALIGNED_PT = 50;

    /* AMR WB bandwidth efficient
     * payload type.
     * Value is in Integer format.
     */
    public static final int AMR_WB_BANDWIDTH_EFFICIENT_PT = 51;

    /* AMR octet aligned dynamic payload type.
     * Value is in Integer format.
     */
    public static final int AMR_OCTET_ALIGNED_PT = 52;

    /* AMR bandwidth efficient payload
     * type.
     * Value is in Integer format.
     */
    public static final int AMR_BANDWIDTH_EFFICIENT_PT = 53;

    /* DTMF WB payload type.
     * Value is in Integer format.
     */
    public static final int DTMF_WB_PT = 54;

    /* DTMF NB payload type.
     * Value is in Integer format.
     */
    public static final int DTMF_NB_PT = 55;

    /* AMR Default encoding mode.
     * Value is in Integer format.
     */
    public static final int AMR_DEFAULT_MODE = 56;

    /* SMS Public Service Identity.
     * Value is in String format.
     */
    public static final int SMS_PSI = 57;

    /* Value for this item is: (Int)
     * Video Quality values are
     *  - 0 -- Low Quality
     *  - 1 -- Medium Quality
     *  - 2 -- High Quality
     */
    public static final int VIDEO_QUALITY = 58;

    /* Value for this item is: (Int)
     * Threshold for LTE to WiFi Handover
     */
    public static final int THRESHOLD_LTE1 = 59;

    /* Value for this item is: (Int)
     * Threshold for WiFi to LTE Handover
     */
    public static final int THRESHOLD_LTE2 = 60;

    /* Value for this item is: (Int)
     * Threshold for LTE to WiFi Handover
     * and WiFi to LTE Handover
     */
    public static final int THRESHOLD_LTE3 = 61;

    /* Value for this item is: (Int)
     * 1x Threshold value for comparing
     * averaged signal strength.
     */
    public static final int THRESHOLD_1x = 62;

    /* Value for this item is: (Int)
     * Threshold for LTE to WiFi Handover
     */
    public static final int THRESHOLD_WIFI_A = 63;

    /* Value for this item is: (Int)
     * Threshold for WiFi to LTE Handover
     */
    public static final int THRESHOLD_WIFI_B = 64;

    /* Value for this item is: (Int)
     * Ims handover hysteresis timer for
     * WLAN to WWAN in seconds.
     */
    public static final int T_EPDG_LTE = 65;

    /* Value for this item is: (Int)
     * Ims handover hysteresis timer for
     * WWAN to WLAN in seconds.
     */
    public static final int T_EPDG_WIFI = 66;

    /* Value for this item is: (Int)
     * Ims handover hysteresis timer for
     * WLAN to 1x in seconds.
     */
    public static final int T_EPDG_1x = 67;

    /* Value for this item is: (Int)
     * Enable VoWiFi through client
     * provisioning.
     * Values
     *   - 0 -- DISABLED
     *   - 1 -- ENABLED
     */
    public static final int VWF_SETTING_ENABLED = 68;

    /* Value for this item is: (Int)
     * Enable VICE through client
     * provisioning.
     * Values
     *   - 0 -- DISABLED
     *   - 1 -- ENABLED
     */
    public static final int VCE_SETTING_ENABLED = 69;

    /* Value for this item is: (Int)
     * Enable RTT through client
     * provisioning.
     * Values
     *   - 0 -- DISABLED
     *   - 1 -- ENABLED
     */
    public static final int RTT_SETTING_ENABLED = 70;

    /* Value for this item is: (Int)
     * Report sms app selection through
     * client provisioning.
     * Values
     *   - 0 -- SMS app selection not allowed.
     *   - 1 -- RCS app is default sms app.
     *   - 2 -- RCS app is not default sms app.
     */
    public static final int SMS_APP = 71;

    /* Value for this item is: (Int)
     * Report vvm app selection through
     * client provisioning.
     * Values
     *   - 0 -- RCS app is default vvm app.
     *   - 1 -- RCS app is not default vvm app.
     */
    public static final int VVM_APP = 72;

    /* Value for this item is: (Int)
     * Wi-Fi Call Mode Roaming Preference
     * Values:
     *   - 0 -- WIFI_ONLY/WLAN_ONLY
     *   - 1 -- CELLULAR_PREFERRED
     *   - 2 -- WIFI_PREFERRED/WLAN_PREFERRED
     *   - 10 -- IMS_PREFERRED
     *
     * Note: Generic error will be returned
     * if the value is not any of the above.
     */
    public static final int VOICE_OVER_WIFI_ROAMING_MODE = 73;

    /* Value for this item is: (Int)
     * Auto Reject Call Preference
     * Values:
     *   - 0 -- DISABLED
     *   - 1 -- ENABLED
     *
     * Note: Generic error will be returned
     * if the value is not any of the above.
     */
    public static final int AUTO_REJECT_CALL_MODE = 74;

    //enum ConfigFailureCause
    public static final int NO_ERR = 0;
    public static final int IMS_NOT_READY = 1;
    public static final int FILE_NOT_AVAILABLE = 2;
    public static final int READ_FAILED = 3;
    public static final int WRITE_FAILED = 4;
    public static final int OTHER_INTERNAL_ERR = 5;

    /* This message is a common structure used for
     * Setting or Getting Config items
     * Message id's used with this message are
     *
     * 1. REQUEST_SET_IMS_CONFIG: To set config item
     *
     *     Request: Should contain only the following
     *               1. "item"
     *               2. "intValue"/"StringValue" - Depending on the item
     *     Response: May contain two fields or one
     *               1. "item"
     *               2. "errorCause" - If there is any error in setting the value
     *
     * 2. REQUEST_GET_IMS_CONFIG: To get value of the Config item
     *
     *     Request: Should only contain "item" (to get the value of)
     *
     *     Response: May contain the following
     *               1. "item"
     *               2. "intValue"/"stringValue" - Depending on the item
     *               3. "errorCause" - If there was any error getting the value
     *
     * Note: Out of three values int/bool/string Value. Only one of them needs to be
     *       used, rest should have has_xxx value set to FALSE.
     */

    /* Determines the Config */
    private int mItem;

    /* Value of the Boolean ConfigItem */
    private boolean mBoolValue;

    /* Value of the Integer ConfigItem */
    private int mIntValue;

    /* Value of the String ConfigItem */
    private String mStringValue;

    private int mErrorCause;

    public ImsConfigItem() {
        mItem = NONE;
        mBoolValue = false;
        mIntValue = 0;
        mStringValue = "";
        mErrorCause = NO_ERR;
    }

    public void setItem(int value) {
        mItem = value;
    }

    public void setBoolValue(boolean value) {
        mBoolValue = value;
    }

    public void setIntValue(int value) {
        mIntValue = value;
    }

    public void setStringValue(String value) {
        mStringValue = value;
    }

    public void setErrorCause(int value) {
        mErrorCause = value;
    }

    public String toString() {
        return ("ImsConfigItem item: " + mItem
                 + " boolValue: " + mBoolValue
                 + " intValue: " + mIntValue
                 + " stringValue: " + mStringValue
                 + " errorCause" + mErrorCause);
    }
}
