/* Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

import vendor.qti.hardware.radio.ims.V1_0.ConfigFailureCause;
import vendor.qti.hardware.radio.ims.V1_6.ConfigInfo;
import vendor.qti.hardware.radio.ims.V1_6.ConfigItem;

import com.qualcomm.ims.utils.Log;
import org.codeaurora.ims.MultiIdentityLineInfo;
import vendor.qti.hardware.radio.ims.V1_3.VerstatInfo;
import vendor.qti.hardware.radio.ims.V1_4.DialRequest;
import vendor.qti.hardware.radio.ims.V1_4.MultiIdentityLineInfoHal;
import vendor.qti.hardware.radio.ims.V1_5.CallFailCause;
import vendor.qti.hardware.radio.ims.V1_6.CallInfo;
import vendor.qti.hardware.radio.ims.V1_6.CallDetails;
import vendor.qti.hardware.radio.ims.V1_6.HandoverInfo;
import vendor.qti.hardware.radio.ims.V1_6.ServiceStatusInfo;
import vendor.qti.hardware.radio.ims.V1_6.StatusForAccessTech;
import vendor.qti.hardware.radio.ims.V1_6.RegistrationInfo;
import vendor.qti.hardware.radio.ims.V1_6.RadioTechType;
import java.util.ArrayList;

public class ImsRadioUtilsV16 {
    private ImsRadioUtilsV16() {
    }

    public static ImsConfigItem configInfoFromHal(ConfigInfo configInfo) {
        if (configInfo == null) {
            return null;
        }

        ImsConfigItem config = new ImsConfigItem();
        config.setItem(configInfoItemFromHal(configInfo.item));
        if (configInfo.hasBoolValue) {
            config.setBoolValue(configInfo.boolValue);
        }

        if (configInfo.intValue != Integer.MAX_VALUE) {
            config.setIntValue(configInfo.intValue);
        }

        config.setStringValue(configInfo.stringValue);

        if (configInfo.errorCause != ConfigFailureCause.CONFIG_FAILURE_INVALID) {
            config.setErrorCause(ImsRadioUtils.configFailureCauseFromHal(configInfo.errorCause));
        }

        return config;
    }

    public static int configInfoItemFromHal(int item) {
        switch (item) {
            case ConfigItem.CONFIG_ITEM_SET_AUTO_REJECT_CALL_MODE_CONFIG:
                return ImsConfigItem.AUTO_REJECT_CALL_MODE;
            default:
                return ImsRadioUtilsV15.configInfoItemFromHal(item);
        }
    }

    public static int configInfoItemToHal(int item) {
        switch (item) {
            case ImsConfigItem.AUTO_REJECT_CALL_MODE:
                return ConfigItem.CONFIG_ITEM_SET_AUTO_REJECT_CALL_MODE_CONFIG;
            default:
                return ImsRadioUtilsV15.configInfoItemToHal(item);
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

    public static boolean isConfigItemIntroducedInV16(int item) {
        switch(item) {
            case ConfigItem.CONFIG_ITEM_SET_AUTO_REJECT_CALL_MODE_CONFIG:
                return true;
            default:
                return false;
        }
    }

    /**
     * Convert the ConfigInfo from V1_0 to V1_6
     */
     public static ConfigInfo migrateConfigInfoFrom(
            vendor.qti.hardware.radio.ims.V1_0.ConfigInfo from) {
        ConfigInfo configInfo = new ConfigInfo();
        configInfo.item = from.item;
        configInfo.hasBoolValue = from.hasBoolValue;
        configInfo.boolValue = from.boolValue;
        configInfo.intValue = from.intValue;
        if (from.stringValue != null) {
            configInfo.stringValue = from.stringValue;
        }
        configInfo.errorCause = from.errorCause;

        return configInfo;
     }

    /**
     * Convert the Call Info from V1_5 to V1_6
     */
    private static CallInfo migrateCallInfoFrom(
            vendor.qti.hardware.radio.ims.V1_5.CallInfo from) {

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
        to.callDetails = migrateCallDetails(from.callDetails);

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
        to.tirMode = from.tirMode;

        return to;
    }

    public static CallDetails migrateCallDetails(
            vendor.qti.hardware.radio.ims.V1_0.CallDetails from) {
        if (from == null) {
            return null;
        }
        CallDetails to = new CallDetails();
        to.callType = from.callType;
        to.callDomain = from.callDomain;
        to.extrasLength = from.extrasLength;

        for(String extra : from.extras) {
            to.extras.add(extra);
        }

        migrateServiceStatusInfo(from.localAbility, to.localAbility);
        migrateServiceStatusInfo(from.peerAbility, to.peerAbility);

        to.callSubstate = from.callSubstate;
        to.mediaId = from.mediaId;
        to.causeCode = from.causeCode;
        to.rttMode = from.rttMode;
        to.sipAlternateUri = from.sipAlternateUri;
        return to;
    }

    public static ArrayList<ServiceStatusInfo> migrateServiceStatusInfo(
            ArrayList<vendor.qti.hardware.radio.ims.V1_0.ServiceStatusInfo> from) {
        if (from == null || from.isEmpty()) {
            return null;
        }
        ArrayList<ServiceStatusInfo> to = new ArrayList<>(from.size());
        migrateServiceStatusInfo(from, to);
        return to;
    }

    public static void migrateServiceStatusInfo(
            ArrayList<vendor.qti.hardware.radio.ims.V1_0.ServiceStatusInfo> from,
            ArrayList<ServiceStatusInfo> to) {
        if (from == null || from.isEmpty()) return;
        for (vendor.qti.hardware.radio.ims.V1_0.ServiceStatusInfo info : from)
            to.add(migrateServiceStatusInfo(info));
    }

    private static ServiceStatusInfo migrateServiceStatusInfo(
            vendor.qti.hardware.radio.ims.V1_0.ServiceStatusInfo from) {
        ServiceStatusInfo to = new ServiceStatusInfo();
        to.hasIsValid = from.hasIsValid;
        to.isValid = from.isValid;
        to.type = from.type;
        to.callType = from.callType;
        to.status = from.status;
        for(Byte data : from.userdata)
            to.userdata.add(data);
        migrateAccTechStatus(from.accTechStatus, to.accTechStatus);
        to.restrictCause = from.restrictCause;
        to.rttMode = from.rttMode;
        return to;
    }

    private static void migrateAccTechStatus(
            ArrayList<vendor.qti.hardware.radio.ims.V1_0.StatusForAccessTech> from,
            ArrayList<StatusForAccessTech> to) {
        for (vendor.qti.hardware.radio.ims.V1_0.StatusForAccessTech status : from)
            to.add(migrateAccTechStatus(status));
    }

    private static StatusForAccessTech migrateAccTechStatus(
            vendor.qti.hardware.radio.ims.V1_0.StatusForAccessTech from) {
        StatusForAccessTech to = new StatusForAccessTech();
        to.networkMode = from.networkMode;
        to.status = from.status;
        to.restrictCause = from.restrictCause;
        to.hasRegistration = from.hasRegistration;
        to.registration = migrateRegistrationInfo(from.registration);
        return to;
    }

    public static RegistrationInfo migrateRegistrationInfo(
            vendor.qti.hardware.radio.ims.V1_0.RegistrationInfo from) {
        if (from == null) {
            return null;
        }
        RegistrationInfo to = new RegistrationInfo();
        to.state = from.state;
        to.errorCode = from.errorCode;
        to.errorMessage = from.errorMessage;
        to.radioTech = from.radioTech;
        to.pAssociatedUris = from.pAssociatedUris;
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

    public static ArrayList<CallInfo> migrateCallListFrom(ArrayList<
            vendor.qti.hardware.radio.ims.V1_5.CallInfo> callList) {

        if (callList == null) {
            return null;
        }
        ArrayList<CallInfo> list = new ArrayList<CallInfo>();
        for (vendor.qti.hardware.radio.ims.V1_5.CallInfo from : callList) {
            CallInfo to = migrateCallInfoFrom(from);
            list.add(to);
        }
        return list;
    }

    public static HandoverInfo migrateHandoverInfo(
            vendor.qti.hardware.radio.ims.V1_0.HandoverInfo from) {
        if (from == null) {
            return null;
        }
        HandoverInfo to = new HandoverInfo();
        to.type = from.type;
        to.srcTech = from.srcTech;
        to.targetTech = from.targetTech;
        to.hasHoExtra = from.hasHoExtra;
        to.hoExtra = from.hoExtra;
        to.errorCode = from.errorCode;
        to.errorMessage = from.errorMessage;
        return to;
    }
}
