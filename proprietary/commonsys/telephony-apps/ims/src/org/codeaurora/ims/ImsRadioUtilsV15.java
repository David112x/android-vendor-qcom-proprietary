/* Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

import android.telephony.emergency.EmergencyNumber;
import android.telephony.emergency.EmergencyNumber.EmergencyCallRouting;
import android.telephony.emergency.EmergencyNumber.EmergencyServiceCategories;
import android.telephony.ims.ImsException;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.stub.ImsSmsImplBase;
import android.telephony.SmsManager;

import org.codeaurora.ims.MultiIdentityLineInfo;
import org.codeaurora.ims.QtiCallConstants;
import org.codeaurora.ims.utils.QtiImsExtUtils;
import vendor.qti.hardware.radio.ims.V1_0.ServiceStatusInfo;
import vendor.qti.hardware.radio.ims.V1_0.IpPresentation;
import vendor.qti.hardware.radio.ims.V1_3.VerstatInfo;
import vendor.qti.hardware.radio.ims.V1_4.DialRequest;
import vendor.qti.hardware.radio.ims.V1_4.MultiIdentityLineInfoHal;
import vendor.qti.hardware.radio.ims.V1_5.CallInfo;
import vendor.qti.hardware.radio.ims.V1_5.AutoCallRejectionInfo;
import vendor.qti.hardware.radio.ims.V1_5.CallFailCause;
import vendor.qti.hardware.radio.ims.V1_5.ConfigInfo;
import vendor.qti.hardware.radio.ims.V1_5.ConfigItem;
import vendor.qti.hardware.radio.ims.V1_5.EmergencyCallRoute;
import vendor.qti.hardware.radio.ims.V1_5.EmergencyServiceCategory;
import vendor.qti.hardware.radio.ims.V1_5.ImsSmsDeliverStatusResult;
import vendor.qti.hardware.radio.ims.V1_5.ImsSmsSendFailureReason;
import java.util.ArrayList;

import com.qualcomm.ims.utils.Log;

public class ImsRadioUtilsV15 {
    private ImsRadioUtilsV15() {
    }

    public static int getCallFailCauseForImsReason(int imsReason) {
        switch(imsReason) {
            case ImsReasonInfo.CODE_SIP_METHOD_NOT_ALLOWED:
                return CallFailCause.CALL_FAIL_SIP_METHOD_NOT_ALLOWED;
            case ImsReasonInfo.CODE_SIP_PROXY_AUTHENTICATION_REQUIRED:
                return CallFailCause.CALL_FAIL_SIP_PROXY_AUTHENTICATION_REQUIRED;
            case ImsReasonInfo.CODE_SIP_REQUEST_ENTITY_TOO_LARGE:
                return CallFailCause.CALL_FAIL_SIP_REQUEST_ENTITY_TOO_LARGE;
            case ImsReasonInfo.CODE_SIP_EXTENSION_REQUIRED:
                return CallFailCause.CALL_FAIL_SIP_EXTENSION_REQUIRED;
            case ImsReasonInfo.CODE_SIP_REQUEST_URI_TOO_LARGE:
                return CallFailCause.CALL_FAIL_SIP_REQUEST_URI_TOO_LARGE;
            case ImsReasonInfo.CODE_SIP_INTERVAL_TOO_BRIEF:
                return CallFailCause.CALL_FAIL_SIP_INTERVAL_TOO_BRIEF;
            case ImsReasonInfo.CODE_SIP_CALL_OR_TRANS_DOES_NOT_EXIST:
                return CallFailCause.CALL_FAIL_SIP_CALL_OR_TRANS_DOES_NOT_EXIST;
            case ImsReasonInfo.CODE_REJECTED_ELSEWHERE:
                return CallFailCause.CALL_FAIL_REJECTED_ELSEWHERE;
            case ImsReasonInfo.CODE_USER_REJECTED_SESSION_MODIFICATION:
                return CallFailCause.CALL_FAIL_USER_REJECTED_SESSION_MODIFICATION;
            case ImsReasonInfo.CODE_USER_CANCELLED_SESSION_MODIFICATION:
                return CallFailCause.CALL_FAIL_USER_CANCELLED_SESSION_MODIFICATION;
            case ImsReasonInfo.CODE_SESSION_MODIFICATION_FAILED:
                return CallFailCause.CALL_FAIL_SESSION_MODIFICATION_FAILED;
            case ImsReasonInfo.CODE_SIP_LOOP_DETECTED:
                return CallFailCause.CALL_FAIL_SIP_LOOP_DETECTED;
            case ImsReasonInfo.CODE_SIP_TOO_MANY_HOPS:
                return CallFailCause.CALL_FAIL_SIP_TOO_MANY_HOPS;
            case ImsReasonInfo.CODE_SIP_AMBIGUOUS:
                return CallFailCause.CALL_FAIL_SIP_AMBIGUOUS;
            case ImsReasonInfo.CODE_SIP_REQUEST_PENDING:
                return CallFailCause.CALL_FAIL_SIP_REQUEST_PENDING;
            case ImsReasonInfo.CODE_SIP_UNDECIPHERABLE:
                return CallFailCause.CALL_FAIL_SIP_UNDECIPHERABLE;
            case QtiCallConstants.CODE_RETRY_ON_IMS_WITHOUT_RTT:
                return CallFailCause.CALL_FAIL_RETRY_ON_IMS_WITHOUT_RTT;
            default:
               Log.i(ImsRadioUtilsV15.class,"Unsupported imsReason for " +
                        "ending call. Passing end cause as 'misc'.");
               return CallFailCause.CALL_FAIL_ERROR_UNSPECIFIED;
        }
    }

    /**
     * IMS Call Fail Causes that were introduced in IMS Radio HAL V1_5
     */
    public static int getImsReasonForCallFailCause(int failCause) {
        switch (failCause) {
            case CallFailCause.CALL_FAIL_SIP_METHOD_NOT_ALLOWED:
                return ImsReasonInfo.CODE_SIP_METHOD_NOT_ALLOWED;
            case CallFailCause.CALL_FAIL_SIP_PROXY_AUTHENTICATION_REQUIRED:
                return ImsReasonInfo.CODE_SIP_PROXY_AUTHENTICATION_REQUIRED;
            case CallFailCause.CALL_FAIL_SIP_REQUEST_ENTITY_TOO_LARGE:
                return ImsReasonInfo.CODE_SIP_REQUEST_ENTITY_TOO_LARGE;
            case CallFailCause.CALL_FAIL_SIP_EXTENSION_REQUIRED:
                return ImsReasonInfo.CODE_SIP_EXTENSION_REQUIRED;
            case CallFailCause.CALL_FAIL_SIP_REQUEST_URI_TOO_LARGE:
                return ImsReasonInfo.CODE_SIP_REQUEST_URI_TOO_LARGE;
            case CallFailCause.CALL_FAIL_SIP_INTERVAL_TOO_BRIEF:
                 return ImsReasonInfo.CODE_SIP_INTERVAL_TOO_BRIEF;
            case CallFailCause.CALL_FAIL_SIP_CALL_OR_TRANS_DOES_NOT_EXIST:
                return ImsReasonInfo.CODE_SIP_CALL_OR_TRANS_DOES_NOT_EXIST;
            case CallFailCause.CALL_FAIL_REJECTED_ELSEWHERE:
                return ImsReasonInfo.CODE_REJECTED_ELSEWHERE;
            case CallFailCause.CALL_FAIL_USER_REJECTED_SESSION_MODIFICATION:
                return ImsReasonInfo.CODE_USER_REJECTED_SESSION_MODIFICATION;
            case CallFailCause.CALL_FAIL_USER_CANCELLED_SESSION_MODIFICATION:
                return ImsReasonInfo.CODE_USER_CANCELLED_SESSION_MODIFICATION;
            case CallFailCause.CALL_FAIL_SESSION_MODIFICATION_FAILED:
                return ImsReasonInfo.CODE_SESSION_MODIFICATION_FAILED;
            case CallFailCause.CALL_FAIL_SIP_LOOP_DETECTED:
                return ImsReasonInfo.CODE_SIP_LOOP_DETECTED;
            case CallFailCause.CALL_FAIL_SIP_TOO_MANY_HOPS:
                return ImsReasonInfo.CODE_SIP_TOO_MANY_HOPS;
            case CallFailCause.CALL_FAIL_SIP_AMBIGUOUS:
                return ImsReasonInfo.CODE_SIP_AMBIGUOUS;
            case CallFailCause.CALL_FAIL_SIP_REQUEST_PENDING:
                return ImsReasonInfo.CODE_SIP_REQUEST_PENDING;
            case CallFailCause.CALL_FAIL_SIP_UNDECIPHERABLE:
                return ImsReasonInfo.CODE_SIP_UNDECIPHERABLE;
            /* use a QtiCallConstant mapping to avoid dependency on AOSP */
            case CallFailCause.CALL_FAIL_RETRY_ON_IMS_WITHOUT_RTT:
                return QtiCallConstants.CODE_RETRY_ON_IMS_WITHOUT_RTT;
            default:
                return ImsReasonInfo.CODE_UNSPECIFIED;
        }
    }

    /**
    * Convert the Call Info from V1_4 to V1_5
    */
    private static CallInfo migrateCallInfoFrom(
            vendor.qti.hardware.radio.ims.V1_4.CallInfo from) {

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

        migarateVerstatInfo(from.verstatInfo, to.verstatInfo);
        migrateMultiIdentityLineInfo(from.mtMultiLineInfo,
                to.mtMultiLineInfo);

        return to;
    }

    private static void migarateVerstatInfo(VerstatInfo from, VerstatInfo to) {
        to.canMarkUnwantedCall = from.canMarkUnwantedCall;
        to.verificationStatus = from.verificationStatus;
    }

    private static void migrateMultiIdentityLineInfo(MultiIdentityLineInfoHal from,
            MultiIdentityLineInfoHal to) {
        to.msisdn = from.msisdn;
        to.registrationStatus = from.registrationStatus;
        to.lineType = from.lineType;
    }

    public static AutoCallRejectionInfo migrateAutoCallRejectionInfoFrom(
            vendor.qti.hardware.radio.ims.V1_3.AutoCallRejectionInfo from) {
        AutoCallRejectionInfo to = new AutoCallRejectionInfo();
        to.callType = from.callType;
        to.autoRejectionCause = from.autoRejectionCause;
        to.sipErrorCode = from.sipErrorCode;
        to.number = from.number;
        return to;
    }

    public static ArrayList<CallInfo> migrateCallListFrom(ArrayList<
                    vendor.qti.hardware.radio.ims.V1_4.CallInfo> callList) {

        if (callList == null) {
            return null;
        }
        ArrayList<CallInfo> list = new ArrayList<CallInfo>();
        for (vendor.qti.hardware.radio.ims.V1_4.CallInfo from : callList) {
            CallInfo to = migrateCallInfoFrom(from);
            list.add(to);
        }
        return list;
    }

    public static int configInfoItemToHal(int item) {
           switch (item) {
              case ImsConfigItem.VOICE_OVER_WIFI_ROAMING_MODE:
                return ConfigItem.CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE;
              default:
                return ImsRadioUtils.configInfoItemToHal(item);
           }
    }

    public static int configInfoItemFromHal(int item) {
        switch(item) {
            case ConfigItem.CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE:
                return ImsConfigItem.VOICE_OVER_WIFI_ROAMING_MODE;
            default:
                return ImsRadioUtils.configInfoItemFromHal(item);
        }
    }

    public static ConfigInfo buildConfigInfo(int item, boolean boolValue, int intValue,
            String stringValue, int errorCause) {
        ConfigInfo configInfo = new ConfigInfo();

        configInfo.item = configInfoItemToHal(item);
        configInfo.hasBoolValue = true;
        configInfo.boolValue = boolValue;
        configInfo.intValue = intValue;
        if (stringValue != null) {
            configInfo.stringValue = stringValue;
        }
        configInfo.errorCause = ImsRadioUtils.configFailureCauseToHal(errorCause);

        return configInfo;
    }

    //Utility to check if Config item is introduced in Ims Radio HAL V1_5
    public static Boolean isConfigItemIntroducedInV15(int item) {
       switch(item) {
           case ConfigItem.CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING_MODE:
               return true;
           default:
               return false;
       }
    }

    /*
      This functions throws an ImsException if the ConfigItem was introduced in V16
      when converting V16 ConfigInfo to V15 ConfigInfo
    */
    public static ConfigInfo migrateConfigInfoFromV16(
            vendor.qti.hardware.radio.ims.V1_6.ConfigInfo from) throws ImsException {
        ConfigInfo to = new ConfigInfo();

        if (ImsRadioUtilsV16.isConfigItemIntroducedInV16(from.item)) {
            throw new ImsException("Config item not supported in current HAL");
        }
        to.item = from.item;
        to.hasBoolValue = from.hasBoolValue;
        to.boolValue = from.boolValue;
        to.intValue = from.intValue;
        if (from.stringValue != null) {
            to.stringValue = from.stringValue;
        }
        to.errorCause = ImsRadioUtils.configFailureCauseToHal(from.errorCause);

        return to;
    }

    public static int mapEmergencyServiceCategoryToHal(
            @EmergencyServiceCategories int categories) {
        int toHalCategories = EmergencyServiceCategory.UNSPECIFIED;
        if ((categories & EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_POLICE)
                == EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_POLICE) {
            toHalCategories |= EmergencyServiceCategory.POLICE;
        }
        if ((categories & EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_AMBULANCE)
                == EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_AMBULANCE) {
            toHalCategories |= EmergencyServiceCategory.AMBULANCE;
        }
        if ((categories & EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_FIRE_BRIGADE)
                == EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_FIRE_BRIGADE) {
            toHalCategories |= EmergencyServiceCategory.FIRE_BRIGADE;
        }
        if ((categories & EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_MARINE_GUARD)
                == EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_MARINE_GUARD) {
            toHalCategories |= EmergencyServiceCategory.MARINE_GUARD;
        }
        if ((categories &
                EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_MOUNTAIN_RESCUE)
                == EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_MOUNTAIN_RESCUE) {
            toHalCategories |= EmergencyServiceCategory.MOUNTAIN_RESCUE;
        }
        if ((categories & EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_MIEC)
                == EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_MIEC) {
            toHalCategories |= EmergencyServiceCategory.MIEC;
        }
        if ((categories & EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_AIEC)
                == EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_AIEC) {
            toHalCategories |= EmergencyServiceCategory.AIEC;
        }
        return toHalCategories;
    }

    public static int mapEmergencyCallRoutingToHal (
             @EmergencyCallRouting int routing) {
        switch (routing) {
            case EmergencyNumber.EMERGENCY_CALL_ROUTING_NORMAL:
                return EmergencyCallRoute.NORMAL;
            case EmergencyNumber.EMERGENCY_CALL_ROUTING_EMERGENCY:
                return EmergencyCallRoute.EMERGENCY;
            default:
                return EmergencyCallRoute.UNKNOWN;
        }
    }

    /**
     * Utility function to convert TIR presentation
     * to IpPresentation
     * @param presentation : IpPresentation
     */
    public static int mapTirPresentationToIpPresentation(int presentation) {
        switch(presentation) {
            case QtiImsExtUtils.QTI_IMS_TIR_PRESENTATION_UNRESTRICTED:
                return IpPresentation.IP_PRESENTATION_NUM_ALLOWED;
            case QtiImsExtUtils.QTI_IMS_TIR_PRESENTATION_RESTRICTED:
                return IpPresentation.IP_PRESENTATION_NUM_RESTRICTED;
            default:
                return IpPresentation.IP_PRESENTATION_NUM_DEFAULT;
        }
    }

    public static CallForwardStatusInfo callForwardStatusInfoFromHal(
            vendor.qti.hardware.radio.ims.V1_5.CallForwardStatusInfo imsRadioCFStatusInfo) {
        return new CallForwardStatusInfo(ImsRadioUtils.sipErrorFromHal(
                imsRadioCFStatusInfo.errorDetails),
                toCallForwardStatus(imsRadioCFStatusInfo.status));
    }

    private static CallForwardStatus[] toCallForwardStatus(
            ArrayList<vendor.qti.hardware.radio.ims.V1_5.CallForwardStatus> fromList) {
        if (fromList == null) {
            return null;
        }

        int size = fromList.size();
        CallForwardStatus[] toList = new CallForwardStatus[size];

        for (int i = 0; i < size; i++) {
            vendor.qti.hardware.radio.ims.V1_5.CallForwardStatus fromCfStatus = fromList.get(i);
            toList[i] = new CallForwardStatus(fromCfStatus.reason, fromCfStatus.status,
                    ImsRadioUtils.sipErrorFromHal(fromCfStatus.errorDetails));
        }
        return toList;
    }

    public static int mapHidlToFrameworkResponseReason(int hidlReason) {
        switch(hidlReason) {
            case ImsSmsSendFailureReason.RESULT_ERROR_FDN_CHECK_FAILURE:
                return SmsManager.RESULT_ERROR_FDN_CHECK_FAILURE;
            case ImsSmsSendFailureReason.RESULT_RADIO_NOT_AVAILABLE:
                return SmsManager.RESULT_RADIO_NOT_AVAILABLE;
            case ImsSmsSendFailureReason.RESULT_NETWORK_REJECT:
                return SmsManager.RESULT_NETWORK_REJECT;
            case ImsSmsSendFailureReason.RESULT_INVALID_ARGUMENTS:
                return SmsManager.RESULT_INVALID_ARGUMENTS;
            case ImsSmsSendFailureReason.RESULT_INVALID_STATE:
                return SmsManager.RESULT_INVALID_STATE;
            case ImsSmsSendFailureReason.RESULT_NO_MEMORY:
                return SmsManager.RESULT_NO_MEMORY;
            case ImsSmsSendFailureReason.RESULT_INVALID_SMS_FORMAT:
                return SmsManager.RESULT_INVALID_SMS_FORMAT;
            case ImsSmsSendFailureReason.RESULT_SYSTEM_ERROR:
                return SmsManager.RESULT_SYSTEM_ERROR;
            case ImsSmsSendFailureReason.RESULT_MODEM_ERROR:
                return SmsManager.RESULT_MODEM_ERROR;
            case ImsSmsSendFailureReason.RESULT_NETWORK_ERROR:
                return SmsManager.RESULT_NETWORK_ERROR;
            case ImsSmsSendFailureReason.RESULT_ENCODING_ERROR:
                return SmsManager.RESULT_ENCODING_ERROR;
            case ImsSmsSendFailureReason.RESULT_INVALID_SMSC_ADDRESS:
                return SmsManager.RESULT_INVALID_SMSC_ADDRESS;
            case ImsSmsSendFailureReason.RESULT_OPERATION_NOT_ALLOWED:
                return SmsManager.RESULT_OPERATION_NOT_ALLOWED;
            case ImsSmsSendFailureReason.RESULT_INTERNAL_ERROR:
                return SmsManager.RESULT_INTERNAL_ERROR;
            case ImsSmsSendFailureReason.RESULT_NO_RESOURCES:
                return SmsManager.RESULT_NO_RESOURCES;
            case ImsSmsSendFailureReason.RESULT_CANCELLED:
                return SmsManager.RESULT_CANCELLED;
            case ImsSmsSendFailureReason.RESULT_REQUEST_NOT_SUPPORTED:
                return SmsManager.RESULT_REQUEST_NOT_SUPPORTED;
            default:
                Log.e(ImsRadioUtilsV15.class, "Failure reason is unknown");
                return SmsManager.RESULT_ERROR_GENERIC_FAILURE;
        }
    }

    public static int imsSmsDeliverStatusToHidl(int status) {
        switch(status) {
            case ImsSmsImplBase.DELIVER_STATUS_ERROR_NO_MEMORY:
                return ImsSmsDeliverStatusResult.DELIVER_STATUS_ERROR_NO_MEMORY;
            case ImsSmsImplBase.DELIVER_STATUS_ERROR_REQUEST_NOT_SUPPORTED:
                return ImsSmsDeliverStatusResult.DELIVER_STATUS_ERROR_REQUEST_NOT_SUPPORTED;
            default:
                Log.e(ImsRadioUtilsV15.class, "unknown deliver status");
                return ImsSmsDeliverStatusResult.DELIVER_STATUS_ERROR;
        }
    }
}
