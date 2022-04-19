/* Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

import android.telephony.ims.ImsReasonInfo;
import vendor.qti.hardware.radio.ims.V1_2.CallFailCause;
import vendor.qti.hardware.radio.ims.V1_2.CallInfo;
import vendor.qti.hardware.radio.ims.V1_0.ServiceStatusInfo;
import java.util.ArrayList;
import java.util.Arrays;

public class ImsRadioUtilsV12{
    private static int migrateCallFailCauseFromV11(int callFailCause) {
        //We need this check to make sure that an invalid call fail cause in
        //V11 gets mapped to an invalid call fail cause in V12. Otherwise it
        //might get mapped to a valid fail cause.
        return (callFailCause == vendor.qti.hardware.radio.ims.V1_1.
                    CallFailCause.CALL_FAIL_INVALID) ?
            CallFailCause.CALL_FAIL_INVALID : callFailCause;
    }

   /**
    * IMS Call Fail Causes that were introduced in IMS Radio HAL V1_2
    */
    public static int getImsReasonForCallFailCause(int failCause) {
        switch (failCause) {
            case CallFailCause.CALL_FAIL_RADIO_OFF:
                return ImsReasonInfo.CODE_RADIO_OFF;
            case CallFailCause.CALL_FAIL_OUT_OF_SERVICE:
                return ImsReasonInfo.CODE_LOCAL_NETWORK_NO_SERVICE;
            case CallFailCause.CALL_FAIL_NO_VALID_SIM:
                return ImsReasonInfo.CODE_NO_VALID_SIM;
            case CallFailCause.CALL_FAIL_RADIO_INTERNAL_ERROR:
                return ImsReasonInfo.CODE_RADIO_INTERNAL_ERROR;
            case CallFailCause.CALL_FAIL_NETWORK_RESP_TIMEOUT:
                return ImsReasonInfo.CODE_NETWORK_RESP_TIMEOUT;
            case CallFailCause.CALL_FAIL_NETWORK_REJECT:
                return ImsReasonInfo.CODE_NETWORK_REJECT;
            case CallFailCause.CALL_FAIL_RADIO_ACCESS_FAILURE:
                return ImsReasonInfo.CODE_RADIO_ACCESS_FAILURE;
            case CallFailCause.CALL_FAIL_RADIO_LINK_FAILURE:
                return ImsReasonInfo.CODE_RADIO_LINK_FAILURE;
            case CallFailCause.CALL_FAIL_RADIO_LINK_LOST:
                return ImsReasonInfo.CODE_RADIO_LINK_LOST;
            case CallFailCause.CALL_FAIL_RADIO_UPLINK_FAILURE:
                return ImsReasonInfo.CODE_RADIO_UPLINK_FAILURE;
            case CallFailCause.CALL_FAIL_RADIO_SETUP_FAILURE:
                return ImsReasonInfo.CODE_RADIO_SETUP_FAILURE;
            case CallFailCause.CALL_FAIL_RADIO_RELEASE_NORMAL:
                return ImsReasonInfo.CODE_RADIO_RELEASE_NORMAL;
            case CallFailCause.CALL_FAIL_RADIO_RELEASE_ABNORMAL:
                return ImsReasonInfo.CODE_RADIO_RELEASE_ABNORMAL;
            case CallFailCause.CALL_FAIL_NETWORK_DETACH:
                return ImsReasonInfo.CODE_NETWORK_DETACH;
            //The OEM Call Fail Causes (CALL_FAIL_OEM_CAUSE_1 - 15) are for debugging
            //purpose only. They shall not be used as a part of the business requirements.
            case CallFailCause.CALL_FAIL_OEM_CAUSE_1:
                return ImsReasonInfo.CODE_OEM_CAUSE_1;
            case CallFailCause.CALL_FAIL_OEM_CAUSE_2:
                return ImsReasonInfo.CODE_OEM_CAUSE_2;
            case CallFailCause.CALL_FAIL_OEM_CAUSE_3:
                return ImsReasonInfo.CODE_OEM_CAUSE_3;
            case CallFailCause.CALL_FAIL_OEM_CAUSE_4:
                return ImsReasonInfo.CODE_OEM_CAUSE_4;
            case CallFailCause.CALL_FAIL_OEM_CAUSE_5:
                return ImsReasonInfo.CODE_OEM_CAUSE_5;
            case CallFailCause.CALL_FAIL_OEM_CAUSE_6:
                return ImsReasonInfo.CODE_OEM_CAUSE_6;
            case CallFailCause.CALL_FAIL_OEM_CAUSE_7:
                return ImsReasonInfo.CODE_OEM_CAUSE_7;
            case CallFailCause.CALL_FAIL_OEM_CAUSE_8:
                return ImsReasonInfo.CODE_OEM_CAUSE_8;
            case CallFailCause.CALL_FAIL_OEM_CAUSE_9:
                return ImsReasonInfo.CODE_OEM_CAUSE_9;
            case CallFailCause.CALL_FAIL_OEM_CAUSE_10:
                return ImsReasonInfo.CODE_OEM_CAUSE_10;
            case CallFailCause.CALL_FAIL_OEM_CAUSE_11:
                return ImsReasonInfo.CODE_OEM_CAUSE_11;
            case CallFailCause.CALL_FAIL_OEM_CAUSE_12:
                return ImsReasonInfo.CODE_OEM_CAUSE_12;
            case CallFailCause.CALL_FAIL_OEM_CAUSE_13:
                return ImsReasonInfo.CODE_OEM_CAUSE_13;
            case CallFailCause.CALL_FAIL_OEM_CAUSE_14:
                return ImsReasonInfo.CODE_OEM_CAUSE_14;
            case CallFailCause.CALL_FAIL_OEM_CAUSE_15:
                return ImsReasonInfo.CODE_OEM_CAUSE_15;
            case CallFailCause.CALL_FAIL_NO_CSFB_IN_CS_ROAM:
                return ImsReasonInfo.CODE_NO_CSFB_IN_CS_ROAM;
            default:
                return ImsRadioUtilsV13.getImsReasonForCallFailCause(failCause);
        }
    }

   /**
    * Convert the Call Info from V1_1 to V1_2
    */
    private static CallInfo migrateCallInfoFromV11(
            vendor.qti.hardware.radio.ims.V1_1.CallInfo from) {

        CallInfo to = new CallInfo();
        to.state = from.state;
        to.index = from.index;
        to.toa = from.toa;
        to.hasIsMpty = from.hasIsMpty;
        to.isMpty = from.isMpty;
        to.hasIsMT = from.hasIsMT;
        to.isMT = from.isMT;
        to.als = from.als;
        to.hasIsVoice = from.hasIsVoice;
        to.isVoice = from.isVoice;
        to.hasIsVoicePrivacy = from.hasIsVoicePrivacy;
        to.isVoicePrivacy = from.isVoicePrivacy;
        to.number = from.number;
        to.numberPresentation = from.numberPresentation;
        to.name = from.name;
        to.namePresentation = from.namePresentation;

        to.hasCallDetails = from.hasCallDetails;
        to.callDetails.callType = from.callDetails.callType;
        to.callDetails.callDomain = from.callDetails.callDomain;
        to.callDetails.extrasLength = from.callDetails.extrasLength;

        for(String extra : from.callDetails.extras) {
            to.callDetails.extras.add(extra);
        }

        for(ServiceStatusInfo localAbility : from.callDetails.localAbility) {
            to.callDetails.localAbility.add(localAbility);
        }

        for(ServiceStatusInfo peerAbility : from.callDetails.peerAbility) {
            to.callDetails.peerAbility.add(peerAbility);
        }

        to.callDetails.callSubstate = from.callDetails.callSubstate;
        to.callDetails.mediaId = from.callDetails.mediaId;
        to.callDetails.causeCode = from.callDetails.causeCode;
        to.callDetails.rttMode = from.callDetails.rttMode;
        to.callDetails.sipAlternateUri = from.callDetails.sipAlternateUri;

        to.hasFailCause = from.hasFailCause;
        to.failCause.failCause =
                migrateCallFailCauseFromV11(from.failCause.failCause);

        for(Byte errorinfo : from.failCause.errorinfo) {
            to.failCause.errorinfo.add(errorinfo);
        }

        to.failCause.networkErrorString = from.failCause.networkErrorString;
        to.failCause.hasErrorDetails = from.failCause.hasErrorDetails;
        to.failCause.errorDetails.errorCode = from.failCause.errorDetails.errorCode;
        to.failCause.errorDetails.errorString =
                from.failCause.errorDetails.errorString;

        to.hasIsEncrypted = from.hasIsEncrypted;
        to.isEncrypted = from.isEncrypted;
        to.hasIsCalledPartyRinging = from.hasIsCalledPartyRinging;
        to.isCalledPartyRinging = from.isCalledPartyRinging;
        to.historyInfo = from.historyInfo;
        to.hasIsVideoConfSupported = from.hasIsVideoConfSupported;
        to.isVideoConfSupported = from.isVideoConfSupported;
        return to;
    }

    public static ArrayList<CallInfo> migrateCallListFromV11(ArrayList<
                    vendor.qti.hardware.radio.ims.V1_1.CallInfo> callList) {

        if (callList == null) {
            return null;
        }
        ArrayList<CallInfo> list = new ArrayList<CallInfo>();
        for (vendor.qti.hardware.radio.ims.V1_1.CallInfo from : callList) {
            CallInfo to = migrateCallInfoFromV11(from);
            list.add(to);
        }
        return list;
    }
}
