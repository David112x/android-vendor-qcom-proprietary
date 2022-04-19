/* Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

import android.telephony.ims.ImsReasonInfo;
import vendor.qti.hardware.radio.ims.V1_3.CallFailCause;
import vendor.qti.hardware.radio.ims.V1_3.CallInfo;
import vendor.qti.hardware.radio.ims.V1_3.ColrInfo;
import vendor.qti.hardware.radio.ims.V1_3.ServiceClassProvisionStatus;
import vendor.qti.hardware.radio.ims.V1_3.SuppServiceStatus;
import vendor.qti.hardware.radio.ims.V1_0.CbNumInfo;
import vendor.qti.hardware.radio.ims.V1_0.CbNumListInfo;
import vendor.qti.hardware.radio.ims.V1_0.ServiceClassStatus;
import vendor.qti.hardware.radio.ims.V1_0.ServiceStatusInfo;
import java.util.ArrayList;
import java.util.Arrays;

import com.qualcomm.ims.utils.Log;

public class ImsRadioUtilsV13 {
    private ImsRadioUtilsV13() {
    }
    /*Create a VerstatInfo instance form the information received from modem*/
    public static VerstatInfo createVerstatInfo(
            vendor.qti.hardware.radio.ims.V1_3.VerstatInfo verstatInfo) {
        return new VerstatInfo(verstatInfo.canMarkUnwantedCall,
            verstatInfo.verificationStatus);
    }

    public static int getCallFailCauseForImsReason(int imsReason) {
        switch(imsReason) {
            case ImsReasonInfo.CODE_SIP_USER_MARKED_UNWANTED:
                return CallFailCause.CALL_FAIL_SIP_USER_MARKED_UNWANTED;
            default:
                return ImsRadioUtilsV14.getCallFailCauseForImsReason(imsReason);
        }
    }

    //Call Fail Causes that were introduced in Ims Radio HAL V1_3
    public static Boolean isFailCauseIntroducedInV13(int failCause) {
       switch(failCause) {
           case CallFailCause.CALL_FAIL_SRV_NOT_REGISTERED:
           case CallFailCause.CALL_FAIL_CALL_TYPE_NOT_ALLOWED:
           case CallFailCause.CALL_FAIL_EMRG_CALL_ONGOING:
           case CallFailCause.CALL_FAIL_CALL_SETUP_ONGOING:
           case CallFailCause.CALL_FAIL_MAX_CALL_LIMIT_REACHED:
           case CallFailCause.CALL_FAIL_UNSUPPORTED_SIP_HDRS:
           case CallFailCause.CALL_FAIL_CALL_TRANSFER_ONGOING:
           case CallFailCause.CALL_FAIL_PRACK_TIMEOUT:
           case CallFailCause.CALL_FAIL_QOS_FAILURE:
           case CallFailCause.CALL_FAIL_ONGOING_HANDOVER:
           case CallFailCause.CALL_FAIL_VT_WITH_TTY_NOT_ALLOWED:
           case CallFailCause.CALL_FAIL_CALL_UPGRADE_ONGOING:
           case CallFailCause.CALL_FAIL_CONFERENCE_WITH_TTY_NOT_ALLOWED:
           case CallFailCause.CALL_FAIL_CALL_CONFERENCE_ONGOING:
           case CallFailCause.CALL_FAIL_VT_WITH_AVPF_NOT_ALLOWED:
           case CallFailCause.CALL_FAIL_ENCRYPTION_CALL_ONGOING:
           case CallFailCause.CALL_FAIL_CALL_ONGOING_CW_DISABLED:
           case CallFailCause.CALL_FAIL_CALL_ON_OTHER_SUB:
           case CallFailCause.CALL_FAIL_1X_COLLISION:
           case CallFailCause.CALL_FAIL_UI_NOT_READY:
           case CallFailCause.CALL_FAIL_CS_CALL_ONGOING:
           case CallFailCause.CALL_FAIL_SIP_USER_MARKED_UNWANTED:
               return true;
           default:
               return false;
       }
    }

    private static int migrateCallFailCauseFromV12(int callFailCause) {
        //We need this check to make sure that an invalid call fail cause in
        //V12 gets mapped to an invalid call fail cause in V13. Otherwise it
        //might get mapped to a valid fail cause.
        return (callFailCause == vendor.qti.hardware.radio.ims.V1_2.
                    CallFailCause.CALL_FAIL_INVALID) ?
            CallFailCause.CALL_FAIL_INVALID : callFailCause;
    }

   /**
    * IMS Call Fail Causes that were introduced in IMS Radio HAL V1_3
    */
    public static int getImsReasonForCallFailCause(int failCause) {
        switch (failCause) {
            case CallFailCause.CALL_FAIL_SRV_NOT_REGISTERED:
                return ImsReasonInfo.CODE_REJECT_SERVICE_NOT_REGISTERED;
            case CallFailCause.CALL_FAIL_CALL_TYPE_NOT_ALLOWED:
                return ImsReasonInfo.CODE_REJECT_CALL_TYPE_NOT_ALLOWED;
            case CallFailCause.CALL_FAIL_EMRG_CALL_ONGOING:
                return ImsReasonInfo.CODE_REJECT_ONGOING_E911_CALL;
            case CallFailCause.CALL_FAIL_CALL_SETUP_ONGOING:
                return ImsReasonInfo.CODE_REJECT_ONGOING_CALL_SETUP;
            case CallFailCause.CALL_FAIL_MAX_CALL_LIMIT_REACHED:
                return ImsReasonInfo.CODE_REJECT_MAX_CALL_LIMIT_REACHED;
            case CallFailCause.CALL_FAIL_UNSUPPORTED_SIP_HDRS:
                return ImsReasonInfo.CODE_REJECT_UNSUPPORTED_SIP_HEADERS;
            case CallFailCause.CALL_FAIL_CALL_TRANSFER_ONGOING:
                return ImsReasonInfo.CODE_REJECT_ONGOING_CALL_TRANSFER;
            case CallFailCause.CALL_FAIL_PRACK_TIMEOUT:
                return ImsReasonInfo.CODE_REJECT_INTERNAL_ERROR;
            case CallFailCause.CALL_FAIL_QOS_FAILURE:
                return ImsReasonInfo.CODE_REJECT_QOS_FAILURE;
            case CallFailCause.CALL_FAIL_ONGOING_HANDOVER:
                return ImsReasonInfo.CODE_REJECT_ONGOING_HANDOVER;
            case CallFailCause.CALL_FAIL_VT_WITH_TTY_NOT_ALLOWED:
                return ImsReasonInfo.CODE_REJECT_VT_TTY_NOT_ALLOWED;
            case CallFailCause.CALL_FAIL_CALL_UPGRADE_ONGOING:
                return ImsReasonInfo.CODE_REJECT_ONGOING_CALL_UPGRADE;
            case CallFailCause.CALL_FAIL_CONFERENCE_WITH_TTY_NOT_ALLOWED:
                return ImsReasonInfo.CODE_REJECT_CONFERENCE_TTY_NOT_ALLOWED;
            case CallFailCause.CALL_FAIL_CALL_CONFERENCE_ONGOING:
                return ImsReasonInfo.CODE_REJECT_ONGOING_CONFERENCE_CALL;
            case CallFailCause.CALL_FAIL_VT_WITH_AVPF_NOT_ALLOWED:
                return ImsReasonInfo.CODE_REJECT_VT_AVPF_NOT_ALLOWED;
            case CallFailCause.CALL_FAIL_ENCRYPTION_CALL_ONGOING:
                return ImsReasonInfo.CODE_REJECT_ONGOING_ENCRYPTED_CALL;
            case CallFailCause.CALL_FAIL_CALL_ONGOING_CW_DISABLED:
                return ImsReasonInfo.CODE_REJECT_ONGOING_CALL_WAITING_DISABLED;
            case CallFailCause.CALL_FAIL_CALL_ON_OTHER_SUB:
                return ImsReasonInfo.CODE_REJECT_CALL_ON_OTHER_SUB;
            case CallFailCause.CALL_FAIL_1X_COLLISION:
                return ImsReasonInfo.CODE_REJECT_1X_COLLISION;
            case CallFailCause.CALL_FAIL_UI_NOT_READY:
                return ImsReasonInfo.CODE_REJECT_INTERNAL_ERROR;
            case CallFailCause.CALL_FAIL_CS_CALL_ONGOING:
                return ImsReasonInfo.CODE_REJECT_ONGOING_CS_CALL;
           default:
                return ImsRadioUtilsV15.getImsReasonForCallFailCause(failCause);
        }
    }

   /**
    * Convert the Call Info from V1_2 to V1_3
    */
    private static CallInfo migrateCallInfoFromV12(
            vendor.qti.hardware.radio.ims.V1_2.CallInfo from) {

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
                migrateCallFailCauseFromV12(from.failCause.failCause);

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

    public static ArrayList<CallInfo> migrateCallListFromV12(ArrayList<
                    vendor.qti.hardware.radio.ims.V1_2.CallInfo> callList) {

        if (callList == null) {
            return null;
        }
        ArrayList<CallInfo> list = new ArrayList<CallInfo>();
        for (vendor.qti.hardware.radio.ims.V1_2.CallInfo from : callList) {
            CallInfo to = migrateCallInfoFromV12(from);
            list.add(to);
        }
        return list;
    }

    public static ColrInfo migrateColrInfoFromV10(
            vendor.qti.hardware.radio.ims.V1_0.ColrInfo from) {
        ColrInfo to = new ColrInfo();
        to.status = ServiceClassStatus.INVALID;
        to.provisionStatus = ServiceClassProvisionStatus.INVALID;
        to.presentation = from.presentation;
        to.hasErrorDetails = from.hasErrorDetails;
        to.errorDetails.errorCode = from.errorDetails.errorCode;
        to.errorDetails.errorString = from.errorDetails.errorString;
        return to;
    }

    public static SuppServiceStatus migrateSuppServiceStatusFromV10(
            vendor.qti.hardware.radio.ims.V1_0.SuppServiceStatus from) {
        SuppServiceStatus to = new SuppServiceStatus();
        to.status = from.status;
        to.provisionStatus = ServiceClassProvisionStatus.INVALID;
        to.facilityType = from.facilityType;
        to.failureCause = from.failureCause;

        for (CbNumListInfo numList : from.cbNumListInfo) {
            CbNumListInfo info = new CbNumListInfo();
            info.serviceClass = numList.serviceClass;

            for (CbNumInfo numInfo : numList.cbNumInfo) {
                info.cbNumInfo.add(numInfo);
            }
            to.cbNumListInfo.add(info);
        }

        to.hasErrorDetails = from.hasErrorDetails;
        to.errorDetails.errorCode = from.errorDetails.errorCode;
        to.errorDetails.errorString = from.errorDetails.errorString;
        return to;
    }
}
