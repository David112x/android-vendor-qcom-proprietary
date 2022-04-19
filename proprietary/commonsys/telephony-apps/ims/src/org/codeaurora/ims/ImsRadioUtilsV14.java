/* Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

import org.codeaurora.ims.MultiIdentityLineInfo;
import vendor.qti.hardware.radio.ims.V1_0.ServiceStatusInfo;
import vendor.qti.hardware.radio.ims.V1_4.CallInfo;
import vendor.qti.hardware.radio.ims.V1_4.DialRequest;
import vendor.qti.hardware.radio.ims.V1_4.MultiIdentityLineInfoHal;
import java.util.ArrayList;

import com.qualcomm.ims.utils.Log;

public class ImsRadioUtilsV14 {
    private ImsRadioUtilsV14() {
    }

    public static int getCallFailCauseForImsReason(int imsReason) {
        return ImsRadioUtilsV15.getCallFailCauseForImsReason(imsReason);
    }

    public static MultiIdentityLineInfo fromMultiIdentityLineInfoHal(
            MultiIdentityLineInfoHal from) {
        MultiIdentityLineInfo to = new MultiIdentityLineInfo(
                from.msisdn, from.lineType, from.registrationStatus);
        return to;
    }

    public static MultiIdentityLineInfoHal toMultiIdentityLineInfoHal(
            MultiIdentityLineInfo from) {
        MultiIdentityLineInfoHal to = new MultiIdentityLineInfoHal();
        toMultiIdentityLineInfoHal(from, to);
        return to;
    }

    /**
    * Utility function to populate MulltiIdentity Information in HIDL
    *
    * If from is null, the deafult value of MultiIdentityLineInfo class will be used
    */
    public static void toMultiIdentityLineInfoHal(MultiIdentityLineInfo from,
            MultiIdentityLineInfoHal to)
    {
        MultiIdentityLineInfo src = from;
        if (src == null) {
            src = MultiIdentityLineInfo.getDefaultLine();
        }
        to.msisdn = src.getMsisdn();
        to.lineType = src.getLineType();
        to.registrationStatus = src.getLineStatus();
    }

    /**
    * Convert the Call Info from V1_3 to V1_4
    */
    private static CallInfo migrateCallInfoFromV13(
            vendor.qti.hardware.radio.ims.V1_3.CallInfo from) {

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
        to.failCause.failCause = from.failCause.failCause;

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
        toMultiIdentityLineInfoHal(MultiIdentityLineInfo.getDefaultLine(),
                to.mtMultiLineInfo);

        return to;
    }

    public static ArrayList<CallInfo> migrateCallListFromV13(ArrayList<
                    vendor.qti.hardware.radio.ims.V1_3.CallInfo> callList) {

        if (callList == null) {
            return null;
        }
        ArrayList<CallInfo> list = new ArrayList<CallInfo>();
        for (vendor.qti.hardware.radio.ims.V1_3.CallInfo from : callList) {
            CallInfo to = migrateCallInfoFromV13(from);
            list.add(to);
        }
        return list;
    }
}
