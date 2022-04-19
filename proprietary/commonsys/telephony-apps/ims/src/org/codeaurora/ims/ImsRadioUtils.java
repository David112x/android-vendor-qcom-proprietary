/*
 * Copyright (c) 2017, 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

import android.location.Address;
import android.telephony.PhoneNumberUtils;
import android.telephony.SmsManager;
import android.telephony.ims.ImsException;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsSsData;
import android.telephony.ims.feature.MmTelFeature.MmTelCapabilities;
import android.telephony.ims.stub.ImsRegistrationImplBase;

import com.android.ims.ImsConfig.FeatureValueConstants;
import com.android.ims.ImsUtInterface;
import android.telephony.ims.stub.ImsSmsImplBase;

import com.qualcomm.ims.utils.Log;
import org.codeaurora.ims.sms.SmsResponse;
import org.codeaurora.ims.sms.StatusReport;
import org.codeaurora.ims.sms.IncomingSms;
import org.codeaurora.ims.MultiIdentityLineInfo;
import vendor.qti.hardware.radio.ims.V1_0.AddressInfo;
import vendor.qti.hardware.radio.ims.V1_0.BlockReasonDetails;
import vendor.qti.hardware.radio.ims.V1_0.BlockReasonType;
import vendor.qti.hardware.radio.ims.V1_0.BlockStatus;
import vendor.qti.hardware.radio.ims.V1_0.CallDomain;
import vendor.qti.hardware.radio.ims.V1_0.CallFailCause;
import vendor.qti.hardware.radio.ims.V1_0.CallForwardInfo;
import vendor.qti.hardware.radio.ims.V1_0.CallFwdTimerInfo;
import vendor.qti.hardware.radio.ims.V1_0.CallInfo;
import vendor.qti.hardware.radio.ims.V1_0.CallModifyInfo;
import vendor.qti.hardware.radio.ims.V1_0.CallModifyFailCause;
import vendor.qti.hardware.radio.ims.V1_0.CallState;
import vendor.qti.hardware.radio.ims.V1_0.CallType;
import vendor.qti.hardware.radio.ims.V1_0.CbNumInfo;
import vendor.qti.hardware.radio.ims.V1_0.CbNumListInfo;
import vendor.qti.hardware.radio.ims.V1_0.ClipStatus;
import vendor.qti.hardware.radio.ims.V1_0.ConferenceCallState;
import vendor.qti.hardware.radio.ims.V1_0.ConfigFailureCause;
import vendor.qti.hardware.radio.ims.V1_0.ConfigInfo;
import vendor.qti.hardware.radio.ims.V1_0.ConfigItem;
import vendor.qti.hardware.radio.ims.V1_0.ConfParticipantOperation;
import vendor.qti.hardware.radio.ims.V1_0.DialRequest;
import vendor.qti.hardware.radio.ims.V1_0.EctType;
import vendor.qti.hardware.radio.ims.V1_0.ExplicitCallTransferInfo;
import vendor.qti.hardware.radio.ims.V1_0.Extra;
import vendor.qti.hardware.radio.ims.V1_0.ExtraType;
import vendor.qti.hardware.radio.ims.V1_0.FacilityType;
import vendor.qti.hardware.radio.ims.V1_0.HangupRequestInfo;
import vendor.qti.hardware.radio.ims.V1_0.HandoverType;
import vendor.qti.hardware.radio.ims.V1_0.ImsSubConfigInfo;
import vendor.qti.hardware.radio.ims.V1_0.IpPresentation;
import vendor.qti.hardware.radio.ims.V1_0.MessageDetails;
import vendor.qti.hardware.radio.ims.V1_0.MessagePriority;
import vendor.qti.hardware.radio.ims.V1_0.MessageSummary;
import vendor.qti.hardware.radio.ims.V1_0.MessageType;
import vendor.qti.hardware.radio.ims.V1_0.MessageWaitingIndication;
import vendor.qti.hardware.radio.ims.V1_0.NotificationType;
import vendor.qti.hardware.radio.ims.V1_0.ParticipantStatusInfo;
import vendor.qti.hardware.radio.ims.V1_0.RegFailureReasonType;
import vendor.qti.hardware.radio.ims.V1_0.RegState;
import vendor.qti.hardware.radio.ims.V1_0.RttMode;
import vendor.qti.hardware.radio.ims.V1_0.ServiceClassStatus;
import vendor.qti.hardware.radio.ims.V1_0.SipErrorInfo;
import vendor.qti.hardware.radio.ims.V1_0.SsRequestType;
import vendor.qti.hardware.radio.ims.V1_0.SsServiceType;
import vendor.qti.hardware.radio.ims.V1_0.SsTeleserviceType;
import vendor.qti.hardware.radio.ims.V1_0.StatusType;
import vendor.qti.hardware.radio.ims.V1_0.SuppServiceNotification;
import vendor.qti.hardware.radio.ims.V1_0.SuppServiceStatus;
import vendor.qti.hardware.radio.ims.V1_0.ToneOperation;
import vendor.qti.hardware.radio.ims.V1_0.TtyMode;
import vendor.qti.hardware.radio.ims.V1_0.VoWiFiCallQualityInfo;
import vendor.qti.hardware.radio.ims.V1_2.ImsSmsDeliverStatusResult;
import vendor.qti.hardware.radio.ims.V1_2.ImsSmsMessage;
import vendor.qti.hardware.radio.ims.V1_2.ImsSmsSendStatusReport;
import vendor.qti.hardware.radio.ims.V1_2.ImsSmsSendFailureReason;
import vendor.qti.hardware.radio.ims.V1_2.ImsSmsSendStatusResult;
import vendor.qti.hardware.radio.ims.V1_2.ImsSmsStatusReportResult;
import vendor.qti.hardware.radio.ims.V1_2.IncomingImsSms;
import vendor.qti.hardware.radio.ims.V1_2.VerificationStatus;
import vendor.qti.hardware.radio.ims.V1_3.ServiceClassProvisionStatus;
import vendor.qti.hardware.radio.ims.V1_5.TirMode;
import vendor.qti.hardware.radio.ims.V1_5.UssdModeType;
import vendor.qti.hardware.radio.ims.V1_6.HandoverInfo;
import vendor.qti.hardware.radio.ims.V1_6.RadioTechType;
import vendor.qti.hardware.radio.ims.V1_6.RegistrationInfo;
import vendor.qti.hardware.radio.ims.V1_6.ServiceStatusInfo;
import vendor.qti.hardware.radio.ims.V1_6.StatusForAccessTech;
import com.android.internal.annotations.VisibleForTesting;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class ImsRadioUtils {
    private static String TAG = "ImsRadioUtils";

    private static final int INVALID_CONNID = 0;

    /**
     * Converts CallDetails object to the ImsRadio CallDetails.
     *
     * @param imsRadioCallDetails ImsRadio call details
     * @param callDetails IMS call details
     *
     */
    public static void callDetailsToHal(
        vendor.qti.hardware.radio.ims.V1_0.CallDetails imsRadioCallDetails, CallDetails callDetails) {
        imsRadioCallDetails.callType = callTypeToHal(callDetails.call_type);
        imsRadioCallDetails.callDomain = callDomainToHal(callDetails.call_domain);

        String callFailImsReason = callDetails.getValueForKeyFromExtras(callDetails.extras,
                QtiCallConstants.EXTRA_RETRY_CALL_FAIL_REASON);
        if (callFailImsReason != null) {
            int callFailCause = getCallFailCauseForImsReason(Integer.parseInt(callFailImsReason));
            callDetails.setValueForKeyInExtras(callDetails.extras,
                    QtiCallConstants.EXTRA_RETRY_CALL_FAIL_REASON,
                    Integer.toString(callFailCause));
        }

        String radioTech = callDetails.getValueForKeyFromExtras(callDetails.extras,
               QtiCallConstants.EXTRA_RETRY_CALL_FAIL_RADIOTECH);
        if(radioTech != null) {
            int halRadioTech = mapRadioTechToHal(Integer.parseInt(radioTech));
            callDetails.setValueForKeyInExtras(callDetails.extras,
                    QtiCallConstants.EXTRA_RETRY_CALL_FAIL_RADIOTECH,
                    Integer.toString(halRadioTech));
        }

        if (RttMode.RTT_MODE_INVALID != callDetails.getRttMode()) {
            imsRadioCallDetails.rttMode = callDetails.getRttMode();
        } else {
            imsRadioCallDetails.rttMode = RttMode.RTT_MODE_DISABLED;
        }
        imsRadioCallDetails.extrasLength = (callDetails.extras != null) ?
            callDetails.extras.length : 0;
        for (int i = 0; i < imsRadioCallDetails.extrasLength; i++) {
           imsRadioCallDetails.extras.add(i, callDetails.extras[i]);
        }
    }

    /**
     * Converts clir Mode to IP presentation
     *
     * @param clirMode caller identification mode
     * @return presentation hal IP presentation
     *
     */
    public static int getIpPresentation(int clirMode) {
        switch (clirMode) {
            case ImsPhoneCommandsInterface.CLIR_SUPPRESSION:
                return IpPresentation.IP_PRESENTATION_NUM_RESTRICTED;
            case ImsPhoneCommandsInterface.CLIR_INVOCATION:
            case ImsPhoneCommandsInterface.CLIR_DEFAULT:
                return IpPresentation.IP_PRESENTATION_NUM_ALLOWED;
            default:
                return IpPresentation.IP_PRESENTATION_INVALID;
        }
    }

    /**
     * Get the conference uri from call details
     *
     * @param callDetails IMS call details
     * @return isConferenceUri true/false if it is a conference call or not
     *
     */
    public static boolean getIsConferenceUri(CallDetails callDetails) {
        boolean isConferenceUri = false;
        if (callDetails != null && callDetails.extras != null) {
            String value = callDetails.getValueForKeyFromExtras(callDetails.extras,
                CallDetails.EXTRAS_IS_CONFERENCE_URI);
            if (value != null && Boolean.valueOf(value)) {
                isConferenceUri = true;
            }
        }
        return isConferenceUri;
    }

    public static DriverCallIms buildDriverCallImsFromHal(
            vendor.qti.hardware.radio.ims.V1_6.CallInfo call) {
        DriverCallIms dc = new DriverCallIms(ImsRadioUtilsV13.
                createVerstatInfo(call.verstatInfo));
        if (call.state != CallState.CALL_STATE_INVALID) {
            dc.state = callStateFromHal(call.state);
        }

        if (call.index != Integer.MAX_VALUE) {
            dc.index = call.index;
        }

        if (call.toa != Integer.MAX_VALUE) {
            dc.TOA = call.toa;
        }

        if (call.hasIsMpty) {
            dc.isMpty = call.isMpty;
        }

        if (call.hasIsMT) {
            dc.isMT = call.isMT;
        }

        if (call.als != Integer.MAX_VALUE) {
            dc.als = call.als;
        }

        if (call.hasIsVoice) {
            dc.isVoice = call.isVoice;
        }

        if (call.hasIsVoicePrivacy) {
            dc.isVoicePrivacy = call.isVoicePrivacy;
        }

        dc.numberPresentation = DriverCallIms.presentationFromCLIP(
                call.numberPresentation);
        dc.name = call.name;
        dc.namePresentation = DriverCallIms.presentationFromCLIP(
                call.namePresentation);

        if (call.hasIsEncrypted) {
            dc.isEncrypted = call.isEncrypted;
        }

        dc.isTirOverwriteAllowed = isTirOverwriteAllowed(call.tirMode);

        dc.historyInfo = call.historyInfo;

        if (call.hasIsVideoConfSupported) {
            dc.mConfSupported = DriverCallIms.CONF_SUPPORT_INDICATED |
                    (call.isVideoConfSupported ?
                    DriverCallIms.CONF_VIDEO_SUPPORTED :
                    DriverCallIms.CONF_SUPPORT_NONE);
        }

        if (call.hasCallDetails) {
            dc.callDetails = getCallDetails(call.callDetails);
        }

        dc.number = call.number;
        dc.number = PhoneNumberUtils.stringFromStringAndTOA(dc.number, dc.TOA);

        int imsReasonCode = ImsReasonInfo.CODE_UNSPECIFIED;
        int imsReasonExtraCode = ImsReasonInfo.CODE_UNSPECIFIED;
        String imsReasonExtraMessage = null;
        if (call.hasFailCause) {
            String networkError = null;
            // Check for an error message from the network.
            if (call.failCause.hasErrorDetails) {
                networkError = call.failCause.errorDetails.errorString;
            }

            final int failCause = call.failCause.failCause;
            // Check if the CallFailCauseResponse has an error code.
            if (failCause != CallFailCause.CALL_FAIL_INVALID) {
                imsReasonCode = getImsReasonForCallFailCause(failCause);
                if (call.failCause.hasErrorDetails &&
                        (call.failCause.errorDetails.errorCode != Integer.MAX_VALUE)) {
                    imsReasonExtraCode = call.failCause.errorDetails.errorCode;
                } else {
                    log("CallFailCauseResponse has no int error code!");
                }

                dc.mCallFailReason = failCause;
                // If there is a network error, propagate it through
                // the ImsReasonInfo object.
                if (networkError != null) {
                    imsReasonExtraMessage = networkError;
                }
            } else {
                log("CallFailCauseResponse failCause is Invalid");
            }
        }

        dc.callFailCause = new ImsReasonInfo(imsReasonCode, imsReasonExtraCode,
                imsReasonExtraMessage);

        //Multi Line Information
        dc.mMtMultiLineInfo = ImsRadioUtilsV14.fromMultiIdentityLineInfoHal(call.mtMultiLineInfo);
        return dc;
    }

    public static ServiceStatusInfo buildServiceStatusInfo(int srvType, int rat, int enabled,
             int restrictCause) {
        Log.i(TAG, "buildServiceStatusInfo srvType = " + srvType + " rat = " + rat +
                " enabled = " + enabled);
        StatusForAccessTech statusForAccessTech = new StatusForAccessTech();
        statusForAccessTech.networkMode = ImsRadioUtils.mapRadioTechToHal(rat);
        statusForAccessTech.status = ImsRadioUtils.statusTypeToHal(enabled);
        statusForAccessTech.restrictCause = restrictCause;
        statusForAccessTech.hasRegistration = false;

        ServiceStatusInfo serviceStatusInfo = new ServiceStatusInfo();
        serviceStatusInfo.hasIsValid = true;
        serviceStatusInfo.isValid = true;
        serviceStatusInfo.callType = ImsRadioUtils.callTypeToHal(srvType);
        serviceStatusInfo.accTechStatus.add(statusForAccessTech);

        return serviceStatusInfo;
    }

    public static ArrayList<ServiceStatusInfo> buildServiceStatusInfoList(
            ArrayList<CapabilityStatus> capabilityStatusList, int restrictCause) {
        ArrayList<ServiceStatusInfo> serviceStatusInfoList = new ArrayList<ServiceStatusInfo>();
        for (CapabilityStatus capabilityStatus : capabilityStatusList) {
            Log.v(TAG, "buildServiceStatusInfoList capabilityStatus = " + capabilityStatus);
            ServiceStatusInfo serviceStatusInfo = buildServiceStatusInfo(
                    ImsRadioUtils.mapCapabilityToSrvType(capabilityStatus.getCapability()),
                    ImsRadioUtils.mapRadioTechToHidlRadioTech(capabilityStatus.getRadioTech()),
                    ImsRadioUtils.mapValueToServiceStatus(capabilityStatus.getStatus()),
                    restrictCause);
            serviceStatusInfoList.add(serviceStatusInfo);
        }
        return serviceStatusInfoList;
    }

    private static int getImsReasonForCallFailCause(int failCause) {
        log("Call fail cause= " + failCause);

        switch (failCause) {
            // SIP Codes
            case CallFailCause.CALL_FAIL_SIP_REDIRECTED:
                return ImsReasonInfo.CODE_SIP_REDIRECTED;
            case CallFailCause.CALL_FAIL_SIP_BAD_REQUEST:
                return ImsReasonInfo.CODE_SIP_BAD_REQUEST;
            case CallFailCause.CALL_FAIL_SIP_FORBIDDEN:
                return ImsReasonInfo.CODE_SIP_FORBIDDEN;
            case CallFailCause.CALL_FAIL_SIP_NOT_FOUND:
                return ImsReasonInfo.CODE_SIP_NOT_FOUND;
            case CallFailCause.CALL_FAIL_SIP_NOT_SUPPORTED:
                return ImsReasonInfo.CODE_SIP_NOT_SUPPORTED;
            case CallFailCause.CALL_FAIL_SIP_REQUEST_TIMEOUT:
                return ImsReasonInfo.CODE_SIP_REQUEST_TIMEOUT;
            case CallFailCause.CALL_FAIL_SIP_TEMPORARILY_UNAVAILABLE:
                return ImsReasonInfo.CODE_SIP_TEMPRARILY_UNAVAILABLE;
            case CallFailCause.CALL_FAIL_SIP_BAD_ADDRESS:
                return ImsReasonInfo.CODE_SIP_BAD_ADDRESS;
            case CallFailCause.CALL_FAIL_SIP_BUSY:
                return ImsReasonInfo.CODE_SIP_BUSY;
            case CallFailCause.CALL_FAIL_SIP_REQUEST_CANCELLED:
                return ImsReasonInfo.CODE_SIP_REQUEST_CANCELLED;
            case CallFailCause.CALL_FAIL_SIP_NOT_ACCEPTABLE:
                return ImsReasonInfo.CODE_SIP_NOT_ACCEPTABLE;
            case CallFailCause.CALL_FAIL_SIP_NOT_REACHABLE:
                return ImsReasonInfo.CODE_SIP_NOT_REACHABLE;
            case CallFailCause.CALL_FAIL_SIP_SERVER_INTERNAL_ERROR:
                return ImsReasonInfo.CODE_SIP_SERVER_INTERNAL_ERROR;
            case CallFailCause.CALL_FAIL_SIP_SERVER_NOT_IMPLEMENTED:
            case CallFailCause.CALL_FAIL_SIP_SERVER_BAD_GATEWAY:
            case CallFailCause.CALL_FAIL_SIP_SERVER_VERSION_UNSUPPORTED:
            case CallFailCause.CALL_FAIL_SIP_SERVER_MESSAGE_TOOLARGE:
            case CallFailCause.CALL_FAIL_SIP_SERVER_PRECONDITION_FAILURE:
                return ImsReasonInfo.CODE_SIP_SERVER_ERROR;
            case CallFailCause.CALL_FAIL_SIP_SERVICE_UNAVAILABLE:
                return ImsReasonInfo.CODE_SIP_SERVICE_UNAVAILABLE;
            case CallFailCause.CALL_FAIL_SIP_SERVER_TIMEOUT:
                return ImsReasonInfo.CODE_SIP_SERVER_TIMEOUT;
            case CallFailCause.CALL_FAIL_SIP_USER_REJECTED:
                return ImsReasonInfo.CODE_SIP_USER_REJECTED;
            case CallFailCause.CALL_FAIL_SIP_GLOBAL_ERROR:
                return ImsReasonInfo.CODE_SIP_GLOBAL_ERROR;
            case vendor.qti.hardware.radio.ims.V1_1.CallFailCause.
                        CALL_FAIL_SIP_ALTERNATE_EMERGENCY_CALL:
                return ImsReasonInfo.CODE_SIP_ALTERNATE_EMERGENCY_CALL;

            // Media Codes
            case CallFailCause.CALL_FAIL_MEDIA_INIT_FAILED:
                return ImsReasonInfo.CODE_MEDIA_INIT_FAILED;
            case CallFailCause.CALL_FAIL_MEDIA_NO_DATA:
                return ImsReasonInfo.CODE_MEDIA_NO_DATA;
            case CallFailCause.CALL_FAIL_MEDIA_NOT_ACCEPTABLE:
                return ImsReasonInfo.CODE_MEDIA_NOT_ACCEPTABLE;
            case CallFailCause.CALL_FAIL_MEDIA_UNSPECIFIED_ERROR:
                return ImsReasonInfo.CODE_MEDIA_UNSPECIFIED;
            case CallFailCause.CALL_FAIL_NORMAL:
                return ImsReasonInfo.CODE_USER_TERMINATED;
            case CallFailCause.CALL_FAIL_BUSY:
                return ImsReasonInfo.CODE_SIP_BUSY;
            case CallFailCause.CALL_FAIL_NETWORK_UNAVAILABLE:
                return ImsReasonInfo.CODE_SIP_TEMPRARILY_UNAVAILABLE;
            case CallFailCause.CALL_FAIL_ANSWERED_ELSEWHERE:
                return ImsReasonInfo.CODE_ANSWERED_ELSEWHERE;
            case CallFailCause.CALL_FAIL_FDN_BLOCKED:
                return ImsReasonInfo.CODE_FDN_BLOCKED;
            case CallFailCause.CALL_FAIL_IMEI_NOT_ACCEPTED:
                return ImsReasonInfo.CODE_IMEI_NOT_ACCEPTED;
            case CallFailCause.CALL_FAIL_CS_RETRY_REQUIRED:
                return ImsReasonInfo.CODE_LOCAL_CALL_CS_RETRY_REQUIRED;
            case CallFailCause.CALL_FAIL_HO_NOT_FEASIBLE:
                return ImsReasonInfo.CODE_LOCAL_HO_NOT_FEASIBLE;
            case CallFailCause.CALL_FAIL_LOW_BATTERY:
                return ImsReasonInfo.CODE_LOW_BATTERY;
            case CallFailCause.CALL_FAIL_EMERGENCY_TEMP_FAILURE:
                return ImsReasonInfo.CODE_EMERGENCY_TEMP_FAILURE;
            case CallFailCause.CALL_FAIL_EMERGENCY_PERM_FAILURE:
                return ImsReasonInfo.CODE_EMERGENCY_PERM_FAILURE;
            case CallFailCause.CALL_FAIL_PULL_OUT_OF_SYNC:
                return ImsReasonInfo.CODE_CALL_PULL_OUT_OF_SYNC;
            case CallFailCause.CALL_FAIL_CAUSE_CALL_PULLED:
                return ImsReasonInfo.CODE_CALL_END_CAUSE_CALL_PULL;
            case CallFailCause.CALL_FAIL_ACCESS_CLASS_BLOCKED:
                return ImsReasonInfo.CODE_ACCESS_CLASS_BLOCKED;
            case CallFailCause.CALL_FAIL_DIAL_MODIFIED_TO_SS:
                return ImsReasonInfo.CODE_DIAL_MODIFIED_TO_SS;
            case CallFailCause.CALL_FAIL_DIAL_MODIFIED_TO_USSD:
                return ImsReasonInfo.CODE_DIAL_MODIFIED_TO_USSD;
            case CallFailCause.CALL_FAIL_DIAL_MODIFIED_TO_DIAL:
                return ImsReasonInfo.CODE_DIAL_MODIFIED_TO_DIAL;
            case CallFailCause.CALL_FAIL_DIAL_MODIFIED_TO_DIAL_VIDEO:
                return ImsReasonInfo.CODE_DIAL_MODIFIED_TO_DIAL_VIDEO;
            case CallFailCause.CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_SS:
                return ImsReasonInfo.CODE_DIAL_VIDEO_MODIFIED_TO_SS;
            case CallFailCause.CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_USSD:
                return ImsReasonInfo.CODE_DIAL_VIDEO_MODIFIED_TO_USSD;
            case CallFailCause.CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL:
                return ImsReasonInfo.CODE_DIAL_VIDEO_MODIFIED_TO_DIAL;
            case CallFailCause.CALL_FAIL_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO:
                return ImsReasonInfo.CODE_DIAL_VIDEO_MODIFIED_TO_DIAL_VIDEO;
            case CallFailCause.CALL_FAIL_SIP_ERROR:
            case CallFailCause.CALL_FAIL_UNOBTAINABLE_NUMBER:
            case CallFailCause.CALL_FAIL_CONGESTION:
            case CallFailCause.CALL_FAIL_INCOMPATIBILITY_DESTINATION:
            case CallFailCause.CALL_FAIL_CALL_BARRED:
            case CallFailCause.CALL_FAIL_FEATURE_UNAVAILABLE:
            case CallFailCause.CALL_FAIL_ERROR_UNSPECIFIED:
            default:
                return ImsRadioUtilsV12.getImsReasonForCallFailCause(failCause);
        }
    }

    public static CallDetails getCallDetails(
            vendor.qti.hardware.radio.ims.V1_6.CallDetails inCallDetails) {
        CallDetails outCallDetails = new CallDetails();

        if (inCallDetails.callType != CallType.CALL_TYPE_INVALID) {
            outCallDetails.call_type = callTypeFromHal(inCallDetails.callType);
        }

        if (inCallDetails.callDomain != CallDomain.CALL_DOMAIN_INVALID) {
            outCallDetails.call_domain = callDomainFromHal(inCallDetails.callDomain);
        }

        if (inCallDetails.callSubstate != Integer.MAX_VALUE) {
            outCallDetails.callsubstate = toCallSubstateConstants(
                    inCallDetails.callSubstate);
        }

        if (inCallDetails.mediaId != Integer.MAX_VALUE) {
            outCallDetails.callMediaId = inCallDetails.mediaId;
        }

        outCallDetails.extras = new String[inCallDetails.extras.size()];
        outCallDetails.extras = inCallDetails.extras.toArray(outCallDetails.extras);

        outCallDetails.localAbility = copySrvStatusList(inCallDetails.localAbility);
        outCallDetails.peerAbility = copySrvStatusList(inCallDetails.peerAbility);

        if (inCallDetails.causeCode != Integer.MAX_VALUE) {
            outCallDetails.causeCode = inCallDetails.causeCode;
        }

        if (inCallDetails.rttMode != RttMode.RTT_MODE_INVALID) {
            outCallDetails.rttMode = inCallDetails.rttMode;
        }

        if (!inCallDetails.sipAlternateUri.isEmpty()) {
            outCallDetails.sipAlternateUri = inCallDetails.sipAlternateUri;
        }

        log("Call Details = " + outCallDetails);

        return outCallDetails;
    }

    private static byte[] getSmsPdu(ArrayList<Byte> hidlPdu) {
        byte[] pdu = new byte[hidlPdu.size()];

        for(int i = 0; i < pdu.length; i++) {
            pdu[i] = hidlPdu.get(i);
        }
        return pdu;
    }

    public static StatusReport statusReportfromHidl(ImsSmsSendStatusReport report) {
        return new StatusReport(report.messageRef, report.format, getSmsPdu(report.pdu));
    }

    private static int mapHidlToFrameworkResponseResult(int hidlResult) {
        switch (hidlResult) {
            case ImsSmsSendStatusResult.SEND_STATUS_OK:
                return ImsSmsImplBase.SEND_STATUS_OK;
            case ImsSmsSendStatusResult.SEND_STATUS_ERROR:
                return ImsSmsImplBase.SEND_STATUS_ERROR;
            case ImsSmsSendStatusResult.SEND_STATUS_ERROR_RETRY:
                return ImsSmsImplBase.SEND_STATUS_ERROR_RETRY;
            case ImsSmsSendStatusResult.SEND_STATUS_ERROR_FALLBACK:
                return ImsSmsImplBase.SEND_STATUS_ERROR_FALLBACK;
            default:
                return ImsSmsImplBase.SEND_STATUS_ERROR;
        }
    }

    private static int mapHidlToFrameworkResponseReason(int hidlReason) {
        switch (hidlReason) {
            case ImsSmsSendFailureReason.RESULT_ERROR_NONE:
                return SmsManager.RESULT_ERROR_NONE;
            case ImsSmsSendFailureReason.RESULT_ERROR_GENERIC_FAILURE:
                return SmsManager.RESULT_ERROR_GENERIC_FAILURE;
            case ImsSmsSendFailureReason.RESULT_ERROR_RADIO_OFF:
                return SmsManager.RESULT_ERROR_RADIO_OFF;
            case ImsSmsSendFailureReason.RESULT_ERROR_NULL_PDU:
                return SmsManager.RESULT_ERROR_NULL_PDU;
            case ImsSmsSendFailureReason.RESULT_ERROR_NO_SERVICE:
                return SmsManager.RESULT_ERROR_NO_SERVICE;
            case ImsSmsSendFailureReason.RESULT_ERROR_LIMIT_EXCEEDED:
                return SmsManager.RESULT_ERROR_LIMIT_EXCEEDED;
            case ImsSmsSendFailureReason.RESULT_ERROR_SHORT_CODE_NOT_ALLOWED:
                return SmsManager.RESULT_ERROR_SHORT_CODE_NOT_ALLOWED;
            case ImsSmsSendFailureReason.RESULT_ERROR_SHORT_CODE_NEVER_ALLOWED:
                return SmsManager.RESULT_ERROR_SHORT_CODE_NEVER_ALLOWED;
            default:
                return ImsRadioUtilsV15.mapHidlToFrameworkResponseReason(hidlReason);
        }
    }

    public static SmsResponse imsSmsResponsefromHidl(int messageRef,
                                 int smsStatusResult,int hidlReason) {
        int statusResult = mapHidlToFrameworkResponseResult(smsStatusResult);
        int reason = mapHidlToFrameworkResponseReason(hidlReason);

        return new SmsResponse(messageRef, statusResult, reason);
    }

    private static int mapHidlToFrameworkVerstat(int verstat){
        switch(verstat){
            case VerificationStatus.STATUS_VALIDATION_NONE:
                return ImsSmsImpl.MT_IMS_STATUS_VALIDATION_NONE;
            case VerificationStatus.STATUS_VALIDATION_PASS:
                return ImsSmsImpl.MT_IMS_STATUS_VALIDATION_PASS;
            case VerificationStatus.STATUS_VALIDATION_FAIL:
                return ImsSmsImpl.MT_IMS_STATUS_VALIDATION_FAIL;
            default:
                return ImsSmsImpl.MT_IMS_STATUS_VALIDATION_NONE;
        }
    }
    public static IncomingSms incomingSmsfromHidl(IncomingImsSms imsSms) {
        int verstat = mapHidlToFrameworkVerstat(imsSms.verstat);
        return new IncomingSms(imsSms.format, getSmsPdu(imsSms.pdu), verstat);
    }

    private static ServiceStatus[] copySrvStatusList(ArrayList<ServiceStatusInfo> fromList) {
        ServiceStatus[] toList = null;
        if (fromList != null) {
            int listLen = fromList.size();
            log("Num of SrvUpdates = " + listLen);
            toList = new ServiceStatus[listLen];
            for (int i = 0; i < listLen; i++) {
                ServiceStatusInfo fromInfo = fromList.get(i);
                if (fromInfo != null) {
                    toList[i] = new ServiceStatus();
                    if (fromInfo.hasIsValid) {
                        toList[i].isValid = fromInfo.isValid;
                    }
                    toList[i].type = callTypeFromHal(fromInfo.callType);
                    if (fromInfo.accTechStatus.size() > 0) {
                        toList[i].accessTechStatus = unpackAccTechStatus(fromInfo);
                    } else {
                        toList[i].accessTechStatus = new ServiceStatus.StatusForAccessTech[1];
                        toList[i].accessTechStatus[0] = new ServiceStatus.StatusForAccessTech();
                        ServiceStatus.StatusForAccessTech act = toList[i].accessTechStatus[0];
                        act.networkMode = RadioTech.RADIO_TECH_LTE;

                        if (fromInfo.status != StatusType.STATUS_INVALID) {
                            act.status = statusTypeFromHal(fromInfo.status);
                        }
                        if (fromInfo.restrictCause != Integer.MAX_VALUE) {
                            act.restrictCause = fromInfo.restrictCause;
                        }
                    }
                    if (fromInfo.status == StatusType.STATUS_ENABLED &&
                            fromInfo.restrictCause != CallDetails.CALL_RESTRICT_CAUSE_NONE) {
                        log("Partially Enabled Status due to Restrict Cause");
                        toList[i].status = ServiceStatus.STATUS_PARTIALLY_ENABLED;
                    } else {
                        if (fromInfo.status != StatusType.STATUS_INVALID) {
                            toList[i].status = statusTypeFromHal(fromInfo.status);
                        }
                    }
                    if (RttMode.RTT_MODE_INVALID != fromInfo.rttMode) {
                        toList[i].rttMode = fromInfo.rttMode;
                    } else {
                        toList[i].rttMode = RttMode.RTT_MODE_DISABLED;
                    }
                    log("RTT: copySrvStatusList rtt mode = " + fromInfo.rttMode);
                    int userdataLen = fromInfo.userdata.size();
                    if (userdataLen > 0) {
                        toList[i].userdata = new byte[userdataLen];
                        for(int j = 0; j < userdataLen; ++j) {
                            toList[i].userdata[j] = fromInfo.userdata.get(j).byteValue();
                        }
                    }
                } else {
                    log("Null service status in list");
                }
            }
        }
        return toList;
    }
    /*
     *   Unpacks the status for access tech from Hal into proto object. The proto object is used to
     *   fill the array of ServiceStatus.StatusForAccessTech.
     *
     *   @param info Service status info from hal.
     *
     *   @return Array of ServiceStatus.StatusForAccessTech.
     *
     */
    private static ServiceStatus.StatusForAccessTech[] unpackAccTechStatus(ServiceStatusInfo info) {
        int statusListLen = info.accTechStatus.size();

        ServiceStatus.StatusForAccessTech[] statusForAccessTech =
                new ServiceStatus.StatusForAccessTech[statusListLen];

        for (int j = 0; j < statusListLen; j++) {
            statusForAccessTech[j] = new ServiceStatus.StatusForAccessTech();
            statusForAccessTech[j] = statusForAccessTechFromHal(info.accTechStatus.get(j));
            log(" networkMode = " + statusForAccessTech[j].networkMode +
                    " status = " + statusForAccessTech[j].status +
                    " restrictCause = " + statusForAccessTech[j].restrictCause +
                    " registered = " + statusForAccessTech[j].registered);
       }

       return statusForAccessTech;
    }

    public static ServiceStatus.StatusForAccessTech statusForAccessTechFromHal(
            StatusForAccessTech inStatus) {
        ServiceStatus.StatusForAccessTech outStatus = null;

        if (inStatus != null) {
            outStatus = new ServiceStatus.StatusForAccessTech();
            if (inStatus.networkMode != RadioTechType.RADIO_TECH_INVALID) {
                outStatus.networkMode = radioTechFromHal(inStatus.networkMode);
            }

            if (inStatus.status != StatusType.STATUS_INVALID) {
                outStatus.status = statusTypeFromHal(inStatus.status);
            }

            if (inStatus.restrictCause != Integer.MAX_VALUE) {
                outStatus.restrictCause = inStatus.restrictCause;
            }

            if (inStatus.hasRegistration) {
                if (inStatus.registration != null) { // Registered is
                                                     // optional field
                    outStatus.registered = regStateFromHal(inStatus.registration.state);
                } else {
                    outStatus.registered = ImsRegistrationInfo.NOT_REGISTERED;;
                    Log.e(ImsRadioUtils.class, "Registered not sent");
                }
            }
        }

        return outStatus;
    }

    public static ImsRegistrationInfo registrationFromHal(RegistrationInfo inRegistration) {
        ImsRegistrationInfo outRegistration = null;

        if (inRegistration != null) {
            outRegistration = new ImsRegistrationInfo();
            if (inRegistration.state != RegState.INVALID) {
                outRegistration.setState(regStateFromHal(inRegistration.state));
            }

            if (inRegistration.errorCode != Integer.MAX_VALUE) {
                outRegistration.setErrorCode(inRegistration.errorCode);
            }

            outRegistration.setErrorMessage(inRegistration.errorMessage);

            if (inRegistration.radioTech != RadioTechType.RADIO_TECH_INVALID) {
                outRegistration.setRadioTech(radioTechFromHal(inRegistration.radioTech));
            }

            outRegistration.setPAssociatedUris(inRegistration.pAssociatedUris);
        }

        return outRegistration;
    }

    private static int toCallSubstateConstants(int callSubstate) {
        switch (callSubstate) {
            case CallDetails.CALL_SUBSTATE_AUDIO_CONNECTED_SUSPENDED:
                return QtiCallConstants.CALL_SUBSTATE_AUDIO_CONNECTED_SUSPENDED;
            case CallDetails.CALL_SUBSTATE_VIDEO_CONNECTED_SUSPENDED:
                return QtiCallConstants.CALL_SUBSTATE_VIDEO_CONNECTED_SUSPENDED;
            case CallDetails.CALL_SUBSTATE_AVP_RETRY:
                return QtiCallConstants.CALL_SUBSTATE_AVP_RETRY;
            case CallDetails.CALL_SUBSTATE_MEDIA_PAUSED:
                return QtiCallConstants.CALL_SUBSTATE_MEDIA_PAUSED;
            case CallDetails.CALL_SUBSTATE_NONE:
            default:
                return QtiCallConstants.CALL_SUBSTATE_NONE;
        }
    }

    public static int regStateFromHal(int state) {
        switch (state) {
            case RegState.REGISTERED:
                return ImsRegistrationInfo.REGISTERED;
            case RegState.REGISTERING:
                return ImsRegistrationInfo.REGISTERING;
            case RegState.NOT_REGISTERED:
                return ImsRegistrationInfo.NOT_REGISTERED;
            default:
                return ImsRegistrationInfo.INVALID;
        }
    }

    public static int regStateToHal(int state) {
        switch (state) {
            case ImsRegistrationInfo.REGISTERED:
                return RegState.REGISTERED;
            case ImsRegistrationInfo.NOT_REGISTERED:
                return RegState.NOT_REGISTERED;
            case ImsRegistrationInfo.REGISTERING:
                return RegState.REGISTERING;
            default:
                return RegState.INVALID;
        }
    }

    public static int radioTechFromHal(int radioTech) {
        switch (radioTech) {
            case RadioTechType.RADIO_TECH_ANY:
                return RadioTech.RADIO_TECH_ANY;
            case RadioTechType.RADIO_TECH_UNKNOWN:
                return RadioTech.RADIO_TECH_UNKNOWN;
            case RadioTechType.RADIO_TECH_GPRS:
                return RadioTech.RADIO_TECH_GPRS;
            case RadioTechType.RADIO_TECH_EDGE:
                return RadioTech.RADIO_TECH_EDGE;
            case RadioTechType.RADIO_TECH_UMTS:
                return RadioTech.RADIO_TECH_UMTS;
            case RadioTechType.RADIO_TECH_IS95A:
                return RadioTech.RADIO_TECH_IS95A;
            case RadioTechType.RADIO_TECH_IS95B:
                return RadioTech.RADIO_TECH_IS95B;
            case RadioTechType.RADIO_TECH_1xRTT:
                return RadioTech.RADIO_TECH_1xRTT;
            case RadioTechType.RADIO_TECH_EVDO_0:
                return RadioTech.RADIO_TECH_EVDO_0;
            case RadioTechType.RADIO_TECH_EVDO_A:
                return RadioTech.RADIO_TECH_EVDO_A;
            case RadioTechType.RADIO_TECH_HSDPA:
                return RadioTech.RADIO_TECH_HSDPA;
            case RadioTechType.RADIO_TECH_HSUPA:
                return RadioTech.RADIO_TECH_HSUPA;
            case RadioTechType.RADIO_TECH_HSPA:
                return RadioTech.RADIO_TECH_HSPA;
            case RadioTechType.RADIO_TECH_EVDO_B:
                return RadioTech.RADIO_TECH_EVDO_B;
            case RadioTechType.RADIO_TECH_EHRPD:
                return RadioTech.RADIO_TECH_EHRPD;
            case RadioTechType.RADIO_TECH_LTE:
                return RadioTech.RADIO_TECH_LTE;
            case RadioTechType.RADIO_TECH_HSPAP:
                return RadioTech.RADIO_TECH_HSPAP;
            case RadioTechType.RADIO_TECH_GSM:
                return RadioTech.RADIO_TECH_GSM;
            case RadioTechType.RADIO_TECH_TD_SCDMA:
                return RadioTech.RADIO_TECH_TD_SCDMA;
            case RadioTechType.RADIO_TECH_WIFI:
                return RadioTech.RADIO_TECH_WIFI;
            case RadioTechType.RADIO_TECH_IWLAN:
                return RadioTech.RADIO_TECH_IWLAN;
            case RadioTechType.RADIO_TECH_NR5G:
                return RadioTech.RADIO_TECH_NR5G;
            default:
                return RadioTech.RADIO_TECH_INVALID;
        }
    }

    public static int callTypeToHal(int callType) {
        switch (callType) {
            case CallDetails.CALL_TYPE_VOICE:
                return CallType.CALL_TYPE_VOICE;
            case CallDetails.CALL_TYPE_VT_TX:
                return CallType.CALL_TYPE_VT_TX;
            case CallDetails.CALL_TYPE_VT_RX:
                return CallType.CALL_TYPE_VT_RX;
            case CallDetails.CALL_TYPE_VT:
                return CallType.CALL_TYPE_VT;
            case CallDetails.CALL_TYPE_VT_NODIR:
                return CallType.CALL_TYPE_VT_NODIR;
            case CallDetails.CALL_TYPE_CS_VS_TX:
                return CallType.CALL_TYPE_CS_VS_TX;
            case CallDetails.CALL_TYPE_CS_VS_RX:
                return CallType.CALL_TYPE_CS_VS_RX;
            case CallDetails.CALL_TYPE_PS_VS_TX:
                return CallType.CALL_TYPE_PS_VS_TX;
            case CallDetails.CALL_TYPE_PS_VS_RX:
                return CallType.CALL_TYPE_PS_VS_RX;
            case CallDetails.CALL_TYPE_SMS:
                return CallType.CALL_TYPE_SMS;
            case CallDetails.CALL_TYPE_UT:
                return CallType.CALL_TYPE_UT;
            case CallDetails.CALL_TYPE_UNKNOWN:
            default:
                return CallType.CALL_TYPE_UNKNOWN;
        }
    }

    public static int callTypeFromHal(int callType) {
        switch (callType) {
            case CallType.CALL_TYPE_VOICE:
                return CallDetails.CALL_TYPE_VOICE;
            case CallType.CALL_TYPE_VT_TX:
                return CallDetails.CALL_TYPE_VT_TX;
            case CallType.CALL_TYPE_VT_RX:
                return CallDetails.CALL_TYPE_VT_RX;
            case CallType.CALL_TYPE_VT:
                return CallDetails.CALL_TYPE_VT;
            case CallType.CALL_TYPE_VT_NODIR:
                return CallDetails.CALL_TYPE_VT_NODIR;
            case CallType.CALL_TYPE_CS_VS_TX:
                return CallDetails.CALL_TYPE_CS_VS_TX;
            case CallType.CALL_TYPE_CS_VS_RX:
                return CallDetails.CALL_TYPE_CS_VS_RX;
            case CallType.CALL_TYPE_PS_VS_TX:
                return CallDetails.CALL_TYPE_PS_VS_TX;
            case CallType.CALL_TYPE_PS_VS_RX:
                return CallDetails.CALL_TYPE_PS_VS_RX;
            case CallType.CALL_TYPE_SMS:
                return CallDetails.CALL_TYPE_SMS;
            case CallType.CALL_TYPE_UT:
                return CallDetails.CALL_TYPE_UT;
            case CallType.CALL_TYPE_UNKNOWN:
            case CallType.CALL_TYPE_INVALID:
            default:
                return CallDetails.CALL_TYPE_UNKNOWN;
        }
    }

    public static int ipPresentationToHal(int presentation) {
        switch (presentation) {
            case SuppService.IP_PRESENTATION_NUM_ALLOWED:
                return IpPresentation.IP_PRESENTATION_NUM_ALLOWED;
            case SuppService.IP_PRESENTATION_NUM_RESTRICTED:
                return IpPresentation.IP_PRESENTATION_NUM_RESTRICTED;
            case SuppService.IP_PRESENTATION_NUM_DEFAULT:
                 return IpPresentation.IP_PRESENTATION_NUM_DEFAULT;
            default:
                return IpPresentation.IP_PRESENTATION_INVALID;
        }
    }

    public static ImsReasonInfo sipErrorFromHal(SipErrorInfo errorInfo) {
        int ismReasonCode = ImsReasonInfo.CODE_UNSPECIFIED;
        int imsReasonExtraCode = ImsReasonInfo.CODE_UNSPECIFIED;
        String imsReasonExtraMessage = errorInfo.errorString;
        if (errorInfo.errorCode != Integer.MAX_VALUE) {
            imsReasonExtraCode = errorInfo.errorCode;
        }
        return new ImsReasonInfo(ismReasonCode, imsReasonExtraCode, imsReasonExtraMessage);
    }

    public static vendor.qti.hardware.radio.ims.V1_4.DialRequest
            buildDialRequest(String address, int clirMode,
                    CallDetails callDetails, boolean isEncrypted) {
        /**
          * DialRequest is the dial request struct containing params that are passed through
          * the HIDL interface dial. Populate the dial request struct from
          * the params passed into the dial API. {@see DialRequest}
          */
        vendor.qti.hardware.radio.ims.V1_4.DialRequest dialRequest = new
                vendor.qti.hardware.radio.ims.V1_4.DialRequest();
        if (address != null) {
            dialRequest.address = address;
        }
        dialRequest.clirMode = clirMode;

        dialRequest.presentation = getIpPresentation(clirMode);

        if (callDetails != null) {
            dialRequest.hasCallDetails = true;
            callDetailsToHal(dialRequest.callDetails, callDetails);
        }

        final boolean isConferenceUri = ImsRadioUtils.getIsConferenceUri(callDetails);
        if (isConferenceUri) {
            dialRequest.isConferenceUri = isConferenceUri;
            dialRequest.hasIsConferenceUri = isConferenceUri;
        }
        final boolean isCallPull = (callDetails != null) ? callDetails.getCallPull() : false;
        if (isCallPull) {
            dialRequest.isCallPull = isCallPull;
            dialRequest.hasIsCallPull = isCallPull;
        }

        dialRequest.hasIsEncrypted = true;
        dialRequest.isEncrypted = isEncrypted;

        final MultiIdentityLineInfo multiIdentityLineInfo = (callDetails != null) ?
                callDetails.getMultiIdentityLineInfo() : null;
        ImsRadioUtilsV14.toMultiIdentityLineInfoHal(
                multiIdentityLineInfo,
                dialRequest.multiLineInfo);

        return dialRequest;
    }

    public static DialRequest migrateFromDialRequestV14(
            vendor.qti.hardware.radio.ims.V1_4.DialRequest from) {
        DialRequest to = new DialRequest();

        to.address = from.address;
        to.clirMode = from.clirMode;
        to.presentation = from.presentation;
        to.hasCallDetails = from.hasCallDetails;
        //Call Details
        to.callDetails.callType = from.callDetails.callType;
        to.callDetails.callDomain = from.callDetails.callDomain;
        to.callDetails.extrasLength = from.callDetails.extrasLength;

        for(String extra : from.callDetails.extras) {
            to.callDetails.extras.add(extra);
        }

        for(vendor.qti.hardware.radio.ims.V1_0.ServiceStatusInfo localAbility :
                from.callDetails.localAbility) {
            to.callDetails.localAbility.add(localAbility);
        }

        for(vendor.qti.hardware.radio.ims.V1_0.ServiceStatusInfo peerAbility :
                from.callDetails.peerAbility) {
            to.callDetails.peerAbility.add(peerAbility);
        }

        to.callDetails.callSubstate = from.callDetails.callSubstate;
        to.callDetails.mediaId = from.callDetails.mediaId;
        to.callDetails.causeCode = from.callDetails.causeCode;
        to.callDetails.rttMode = from.callDetails.rttMode;
        to.callDetails.sipAlternateUri = from.callDetails.sipAlternateUri;

        to.hasIsConferenceUri = from.hasIsConferenceUri;
        to.isConferenceUri = from.isConferenceUri;
        to.hasIsCallPull = from.hasIsCallPull;
        to.isCallPull = from.isCallPull;
        to.hasIsEncrypted = from.hasIsEncrypted;
        to.isEncrypted = from.isEncrypted;

        return to;
    }

    public static vendor.qti.hardware.radio.ims.V1_3.HangupRequestInfo
        buildHangupRequest(int connectionId, String userUri,
            String confUri, boolean mpty, int failCause, String errorInfo) {
        vendor.qti.hardware.radio.ims.V1_3.HangupRequestInfo hangup = new
                vendor.qti.hardware.radio.ims.V1_3.HangupRequestInfo();
        /* If Calltracker has a matching local connection the connection id will be used.
         * if there is no matching connection object and if it is a remotely added participant
         * then connection id will not be present hence 0
         */
        if (connectionId != INVALID_CONNID) {
            hangup.connIndex = connectionId;
        } else {
            hangup.connIndex = Integer.MAX_VALUE;
        }

        if (userUri != null) {
            hangup.connUri = userUri;
        }
        hangup.hasMultiParty = true;
        hangup.multiParty = mpty;
        hangup.conf_id = Integer.MAX_VALUE;
        if (failCause != Integer.MAX_VALUE) {
            hangup.hasFailCauseResponse = true;
            if (errorInfo != null && !errorInfo.isEmpty()) {
                Log.v(TAG, "hangupWithReason errorInfo = " + errorInfo);
                for(Byte b: errorInfo.getBytes()) {
                    hangup.failCauseResponse.errorinfo.add(0, b);
                }
            }
            int callFailCause = getCallFailCauseForImsReason(failCause);
            hangup.failCauseResponse.failCause = callFailCause;
            Log.v(TAG, "hangupWithReason callFailCause=" + callFailCause);
            // Check for unsupported call end reason. If so, set
            // the errorInfo string to the reason code, similar to KK.
            if (callFailCause == CallFailCause.CALL_FAIL_MISC) {
                for(Byte b: Integer.toString(failCause).getBytes()) {
                    hangup.failCauseResponse.errorinfo.add(0, b);
                }
            }
            Log.v(TAG, "hangupWithReason MISC callFailCause, errorInfo=" + failCause);
        }
        return hangup;
    }

    public static int getCallFailCauseForImsReason(int imsReason) {
        Log.i(TAG, "imsReason= " + imsReason);
        int failCause;

        switch (imsReason) {
            case ImsReasonInfo.CODE_USER_DECLINE:
                failCause = CallFailCause.CALL_FAIL_USER_REJECT;
                break;
            case ImsReasonInfo.CODE_USER_TERMINATED:
                failCause = CallFailCause.CALL_FAIL_USER_BUSY;
                break;
            case ImsReasonInfo.CODE_LOW_BATTERY:
                failCause = CallFailCause.CALL_FAIL_LOW_BATTERY;
                break;
            case ImsReasonInfo.CODE_BLACKLISTED_CALL_ID:
                failCause = CallFailCause.CALL_FAIL_BLACKLISTED_CALL_ID;
                break;
            default:
                failCause = ImsRadioUtilsV13.getCallFailCauseForImsReason(imsReason);
        }
        return failCause;
    }

    public static int statusTypeToHal(int status) {
        switch (status) {
            case ServiceStatus.STATUS_DISABLED:
                return StatusType.STATUS_DISABLED;
            case ServiceStatus.STATUS_PARTIALLY_ENABLED:
                return StatusType.STATUS_PARTIALLY_ENABLED;
            case ServiceStatus.STATUS_ENABLED:
                return StatusType.STATUS_ENABLED;
            case ServiceStatus.STATUS_NOT_SUPPORTED:
                return StatusType.STATUS_NOT_SUPPORTED;
            default:
                return StatusType.STATUS_INVALID;
        }
    }

    public static int statusTypeFromHal(int status) {
        switch (status) {
            case StatusType.STATUS_DISABLED:
                return ServiceStatus.STATUS_DISABLED;
            case StatusType.STATUS_PARTIALLY_ENABLED:
                return ServiceStatus.STATUS_PARTIALLY_ENABLED;
            case StatusType.STATUS_ENABLED:
                return ServiceStatus.STATUS_ENABLED;
            case StatusType.STATUS_NOT_SUPPORTED:
            default:
                return ServiceStatus.STATUS_NOT_SUPPORTED;
        }
    }

    public static int mapCapabilityToSrvType (int capability) {
        switch (capability) {
            case MmTelCapabilities.CAPABILITY_TYPE_VOICE:
                return CallDetails.CALL_TYPE_VOICE;
            case MmTelCapabilities.CAPABILITY_TYPE_VIDEO:
                return CallDetails.CALL_TYPE_VT;
            case MmTelCapabilities.CAPABILITY_TYPE_UT:
                return CallDetails.CALL_TYPE_UT;
            case MmTelCapabilities.CAPABILITY_TYPE_SMS:
                return CallDetails.CALL_TYPE_SMS;
            default:
                return CallDetails.CALL_TYPE_UNKNOWN;
        }
    }

    public static int mapRadioTechToHidlRadioTech(int rat) {
        switch (rat) {
            case ImsRegistrationImplBase.REGISTRATION_TECH_LTE:
                return RadioTech.RADIO_TECH_LTE;
            case ImsRegistrationImplBase.REGISTRATION_TECH_IWLAN:
                return RadioTech.RADIO_TECH_IWLAN;
            default:
                return RadioTech.RADIO_TECH_UNKNOWN;
        }
    }

    public static int mapValueToServiceStatus(int value) {
        switch (value) {
            case FeatureValueConstants.ON:
                return ServiceStatus.STATUS_ENABLED;
            case FeatureValueConstants.OFF:
                return ServiceStatus.STATUS_DISABLED;
            default:
                return ServiceStatus.STATUS_DISABLED;
        }
    }

    public static int mapRadioTechToHal(int radioTech) {
        switch (radioTech) {
            case RadioTech.RADIO_TECH_ANY:
                return RadioTechType.RADIO_TECH_ANY;
            case RadioTech.RADIO_TECH_UNKNOWN:
                return RadioTechType.RADIO_TECH_UNKNOWN;
            case RadioTech.RADIO_TECH_GPRS:
                return RadioTechType.RADIO_TECH_GPRS;
            case RadioTech.RADIO_TECH_EDGE:
                return RadioTechType.RADIO_TECH_EDGE;
            case RadioTech.RADIO_TECH_UMTS:
                return RadioTechType.RADIO_TECH_UMTS;
            case RadioTech.RADIO_TECH_IS95A:
                return RadioTechType.RADIO_TECH_IS95A;
            case RadioTech.RADIO_TECH_IS95B:
                return RadioTechType.RADIO_TECH_IS95B;
            case RadioTech.RADIO_TECH_1xRTT:
                return RadioTechType.RADIO_TECH_1xRTT;
            case RadioTech.RADIO_TECH_EVDO_0:
                return RadioTechType.RADIO_TECH_EVDO_0;
            case RadioTech.RADIO_TECH_EVDO_A:
                return RadioTechType.RADIO_TECH_EVDO_A;
            case RadioTech.RADIO_TECH_HSDPA:
                return RadioTechType.RADIO_TECH_HSDPA;
            case RadioTech.RADIO_TECH_HSUPA:
                return RadioTechType.RADIO_TECH_HSUPA;
            case RadioTech.RADIO_TECH_HSPA:
                return RadioTechType.RADIO_TECH_HSPA;
            case RadioTech.RADIO_TECH_EVDO_B:
                return RadioTechType.RADIO_TECH_EVDO_B;
            case RadioTech.RADIO_TECH_EHRPD:
                return RadioTechType.RADIO_TECH_EHRPD;
            case RadioTech.RADIO_TECH_LTE:
                return RadioTechType.RADIO_TECH_LTE;
            case RadioTech.RADIO_TECH_HSPAP:
                return RadioTechType.RADIO_TECH_HSPAP;
            case RadioTech.RADIO_TECH_GSM:
                return RadioTechType.RADIO_TECH_GSM;
            case RadioTech.RADIO_TECH_TD_SCDMA:
                return RadioTechType.RADIO_TECH_TD_SCDMA;
            case RadioTech.RADIO_TECH_WIFI:
                return RadioTechType.RADIO_TECH_WIFI;
            case RadioTech.RADIO_TECH_IWLAN:
                return RadioTechType.RADIO_TECH_IWLAN;
            case RadioTech.RADIO_TECH_NR5G:
                return RadioTechType.RADIO_TECH_NR5G;
            default:
                return RadioTechType.RADIO_TECH_INVALID;
        }
    }

    public static int callDomainFromHal(int callDomain) {
        switch (callDomain) {
            case CallDomain.CALL_DOMAIN_UNKNOWN:
                return CallDetails.CALL_DOMAIN_UNKNOWN;
            case CallDomain.CALL_DOMAIN_CS:
                return CallDetails.CALL_DOMAIN_CS;
            case CallDomain.CALL_DOMAIN_PS:
                return CallDetails.CALL_DOMAIN_PS;
            case CallDomain.CALL_DOMAIN_AUTOMATIC:
                return CallDetails.CALL_DOMAIN_AUTOMATIC;
            case CallDomain.CALL_DOMAIN_NOT_SET:
            case CallDomain.CALL_DOMAIN_INVALID:
            default:
                return CallDetails.CALL_DOMAIN_NOT_SET;
        }
    }

    public static int callDomainToHal(int callDomain) {
        switch (callDomain) {
            case CallDetails.CALL_DOMAIN_UNKNOWN:
                return CallDomain.CALL_DOMAIN_UNKNOWN;
            case CallDetails.CALL_DOMAIN_CS:
                return CallDomain.CALL_DOMAIN_CS;
            case CallDetails.CALL_DOMAIN_PS:
                return CallDomain.CALL_DOMAIN_PS;
            case CallDetails.CALL_DOMAIN_AUTOMATIC:
                return CallDomain.CALL_DOMAIN_AUTOMATIC;
            case CallDetails.CALL_DOMAIN_NOT_SET:
            default:
                return CallDomain.CALL_DOMAIN_NOT_SET;
        }
    }

    public static int ringbackToneFromHal(int tone) {
        switch (tone) {
            case ToneOperation.START:
                return CallDetails.RINGBACK_TONE_START;
            case ToneOperation.STOP:
            case ToneOperation.INVALID:
            default:
                return CallDetails.RINGBACK_TONE_STOP;
        }
    }

    public static void log(String msg) {
        Log.i(ImsRadioUtils.class, msg);
    }

    protected static Object handleSrvStatus(ArrayList<ServiceStatusInfo> inList) {

        if (inList == null) {
            log("inList is null.");
            return null;
        }

        ServiceStatus[] outList =  copySrvStatusList(inList);
        ArrayList<ServiceStatus> response = new ArrayList<ServiceStatus>(Arrays.asList(outList));

        return response;
    }

    public static HoInfo handoverFromHal(HandoverInfo inHandover) {

        if (inHandover == null) {
            return null;
        }

        HoInfo outHandover = new HoInfo();

        if (inHandover.type != HandoverType.INVALID) {
            outHandover.setType(handoverTypeFromHal(inHandover.type));
        }

        if (inHandover.srcTech != RadioTechType.RADIO_TECH_INVALID) {
            outHandover.setSrcTech(radioTechFromHal(inHandover.srcTech));
        }

        if (inHandover.targetTech != RadioTechType.RADIO_TECH_INVALID) {
            outHandover.setTargetTech(radioTechFromHal(inHandover.targetTech));
        }

        if (inHandover.hasHoExtra) {
            outHandover.setExtra(extraTypeFromHal(inHandover.hoExtra.type),
                                 getByteArray(inHandover.hoExtra.extraInfo));
        }

        outHandover.setErrorCode(inHandover.errorCode);
        outHandover.setErrorMessage(inHandover.errorMessage);

        return outHandover;
    }

    public static int handoverTypeFromHal(int inType) {
        switch (inType) {
            case HandoverType.START:
                return HoInfo.START;
            case HandoverType.COMPLETE_SUCCESS:
                return HoInfo.COMPLETE_SUCCESS;
            case HandoverType.COMPLETE_FAIL:
                return HoInfo.COMPLETE_FAIL;
            case HandoverType.CANCEL:
                return HoInfo.CANCEL;
            case HandoverType.NOT_TRIGGERED_MOBILE_DATA_OFF:
                return HoInfo.NOT_TRIGGERED_MOBILE_DATA_OFF;
            case HandoverType.NOT_TRIGGERED:
                return HoInfo.NOT_TRIGGERED;
            default:
                return HoInfo.INVALID;
        }
    }

    private static DriverCallIms.State callStateFromHal(int inCallState) {
        switch (inCallState) {
            case CallState.CALL_ACTIVE:
                return DriverCallIms.State.ACTIVE;
            case CallState.CALL_HOLDING:
                return DriverCallIms.State.HOLDING;
            case CallState.CALL_DIALING:
                return DriverCallIms.State.DIALING;
            case CallState.CALL_ALERTING:
                return DriverCallIms.State.ALERTING;
            case CallState.CALL_INCOMING:
                return DriverCallIms.State.INCOMING;
            case CallState.CALL_WAITING:
                return DriverCallIms.State.WAITING;
            case CallState.CALL_END:
            default:
                return DriverCallIms.State.END;
        }
    }

    private static int extraTypeFromHal(int extraHo) {
        switch (extraHo) {
            case ExtraType.LTE_TO_IWLAN_HO_FAIL:
                return ExtraInfo.LTE_TO_IWLAN_HO_FAIL;
            default:
                return ExtraInfo.INVALID;
        }
    }

    public static CallForwardInfo buildCallForwardInfo(int cfReason, int serviceClass,
            String number, int action, int timeSeconds) {
        CallForwardInfo cfInfo = new CallForwardInfo();

        cfInfo.status = action;
        cfInfo.reason = cfReason;
        cfInfo.serviceClass = serviceClass;
        cfInfo.toa = PhoneNumberUtils.toaFromString(number);
        if (number != null) {
            cfInfo.number = number;
        }
        cfInfo.hasCallFwdTimerStart = false;
        cfInfo.hasCallFwdTimerEnd = false;
        cfInfo.timeSeconds = timeSeconds;

        return cfInfo;
    }

    public static ImsSmsMessage buildImsSms(int messageRef, String format,
             String smsc, boolean isRetry, byte[] pdu) {
        ImsSmsMessage imsSms = new ImsSmsMessage();

        imsSms.messageRef = messageRef;
        imsSms.format = format;
        imsSms.shallRetry = isRetry;
        imsSms.smsc = smsc == null ? "" : smsc;
        for (byte a : pdu) {
            imsSms.pdu.add(a);
        }
        return imsSms;
    }

    public static int imsSmsDeliverStatusToHidl(int status) {
        switch(status){
            case ImsSmsImplBase.DELIVER_STATUS_OK:
                return ImsSmsDeliverStatusResult.DELIVER_STATUS_OK;
            case ImsSmsImplBase.DELIVER_STATUS_ERROR_GENERIC:
                return ImsSmsDeliverStatusResult.DELIVER_STATUS_ERROR;
            default:
                return ImsRadioUtilsV15.imsSmsDeliverStatusToHidl(status);
        }
    }

    public static int imsSmsStatusReportStatusToHidl(int report) {
        switch (report){
            case ImsSmsImplBase.STATUS_REPORT_STATUS_OK:
                return ImsSmsStatusReportResult.STATUS_REPORT_STATUS_OK;
            case ImsSmsImplBase.STATUS_REPORT_STATUS_ERROR:
                return ImsSmsStatusReportResult.STATUS_REPORT_STATUS_ERROR;
            default:
                return ImsSmsStatusReportResult.STATUS_REPORT_STATUS_ERROR;
        }
    }

    public static void buildCallFwdTimerInfo(CallFwdTimerInfo callFwdTimerInfo, int hour,
            int minute) {
        callFwdTimerInfo.year = Integer.MAX_VALUE;
        callFwdTimerInfo.month = Integer.MAX_VALUE;
        callFwdTimerInfo.day = Integer.MAX_VALUE;
        callFwdTimerInfo.hour = hour;
        callFwdTimerInfo.minute = minute;
        callFwdTimerInfo.second = Integer.MAX_VALUE;
        callFwdTimerInfo.timezone = Integer.MAX_VALUE;
    }

    public static Object buildCFStatusResponseFromHal( ArrayList<CallForwardInfo> inCfInfoList) {
        org.codeaurora.telephony.utils.CallForwardInfo outCfInfoList[] = null;

        if (inCfInfoList != null) {
            int infoListSize = inCfInfoList.size();
            outCfInfoList = new org.codeaurora.telephony.utils.CallForwardInfo[infoListSize];
            ImsCallForwardTimerInfo timerInfo[] = new ImsCallForwardTimerInfo[infoListSize];

            for (int i = 0; i < infoListSize; i++) {
                outCfInfoList[i] = new org.codeaurora.telephony.utils.CallForwardInfo();
                CallForwardInfo inCfInfo = inCfInfoList.get(i);

                if (inCfInfo.status != Integer.MAX_VALUE) {
                    outCfInfoList[i].status = inCfInfo.status;
                }

                if (inCfInfo.reason != Integer.MAX_VALUE) {
                    outCfInfoList[i].reason = inCfInfo.reason;
                }

                if (inCfInfo.serviceClass != Integer.MAX_VALUE) {
                    outCfInfoList[i].serviceClass = inCfInfo.serviceClass;
                }

                if (inCfInfo.toa != Integer.MAX_VALUE) {
                    outCfInfoList[i].toa = inCfInfo.toa;
                }

                outCfInfoList[i].number = inCfInfo.number;

                if (inCfInfo.timeSeconds != Integer.MAX_VALUE) {
                    outCfInfoList[i].timeSeconds = inCfInfo.timeSeconds;
                }

                if (inCfInfo.hasCallFwdTimerStart && inCfInfo.hasCallFwdTimerEnd) {
                    log("responseQueryCallForward for Timer");
                    CallFwdTimerInfo startCallTimerInfo = inCfInfo.callFwdTimerStart;
                    timerInfo[i] = new ImsCallForwardTimerInfo();
                    if (inCfInfo.status != Integer.MAX_VALUE) {
                        timerInfo[i].status = inCfInfo.status;
                    }

                    if (inCfInfo.reason != Integer.MAX_VALUE) {
                        timerInfo[i].reason = inCfInfo.reason;
                    }

                    if (inCfInfo.serviceClass != Integer.MAX_VALUE) {
                        timerInfo[i].serviceClass = inCfInfo.serviceClass;
                    }

                    timerInfo[i].number = inCfInfo.number;

                    if (startCallTimerInfo.hour != Integer.MAX_VALUE) {
                        timerInfo[i].startHour = startCallTimerInfo.hour;
                    }

                    if (startCallTimerInfo.minute != Integer.MAX_VALUE) {
                        timerInfo[i].startMinute = startCallTimerInfo.minute;
                    }
                    CallFwdTimerInfo endCallTimerInfo = inCfInfo.callFwdTimerEnd;

                    if (endCallTimerInfo.hour != Integer.MAX_VALUE) {
                        timerInfo[i].endHour = endCallTimerInfo.hour;
                    }

                    if (endCallTimerInfo.minute != Integer.MAX_VALUE) {
                        timerInfo[i].endMinute = endCallTimerInfo.minute;
                    }
                    return timerInfo;
                }
            }
        } else {
            outCfInfoList = new org.codeaurora.telephony.utils.CallForwardInfo[0];
            log("inCfInfoList is null.");
        }

        return outCfInfoList;
    }

    public static int serviceClassStatusFromHal(int inServiceStatus) {
        switch(inServiceStatus) {
            case ServiceClassStatus.ENABLED:
                return SuppSvcResponse.ENABLED;
            case ServiceClassStatus.DISABLED:
                return SuppSvcResponse.DISABLED;
            default:
                return SuppSvcResponse.INVALID;
        }
    }

    public static int serviceClassProvisionStatusFromHal(int inServiceProvisionStatus) {
        switch(inServiceProvisionStatus) {
            case ServiceClassProvisionStatus.NOT_PROVISIONED:
                return SuppService.NOT_PROVISIONED;
            case ServiceClassProvisionStatus.PROVISIONED:
                return SuppService.PROVISIONED;
            default:
                return SuppService.STATUS_UNKNOWN;
        }
    }

    public static SuppNotifyInfo suppServiceNotificationFromHal(
            SuppServiceNotification inNotification) {
        if (inNotification == null) {
            return null;
        }

        SuppNotifyInfo outNotification = new SuppNotifyInfo();

        if (inNotification.notificationType != NotificationType.INVALID) {
            outNotification.setNotificationType(notificationTypeFromHal(
                    inNotification.notificationType));
        }

        if (inNotification.code != Integer.MAX_VALUE) {
            outNotification.setCode(inNotification.code);
        }

        if (inNotification.index != Integer.MAX_VALUE) {
            outNotification.setIndex(inNotification.index);
        }

        if (inNotification.type != Integer.MAX_VALUE) {
            outNotification.setType(inNotification.type);
        }

        if (inNotification.connId != Integer.MAX_VALUE) {
            outNotification.setConnId(inNotification.connId);
        }

        outNotification.setNumber(inNotification.number);
        outNotification.setHistoryInfo(inNotification.historyInfo);

        if (inNotification.hasHoldTone) {
            outNotification.setHoldTone(inNotification.holdTone);
        }

        return outNotification;
    }

    private static int notificationTypeFromHal(int inNotificationType) {
        switch (inNotificationType) {
            case NotificationType.MO:
                return SuppNotifyInfo.MO;
            case NotificationType.MT:
                return SuppNotifyInfo.MT;
            default:
                return Integer.MAX_VALUE;
        }
    }

    public static byte[] getByteArray(ArrayList<Byte> inList) {

        if (inList == null) {
            return null;
        }

        byte[] outArray = new byte[inList.size()];

        for(int i = 0; i < outArray.length; ++i) {
            outArray[i] = inList.get(i).byteValue();
        }

        return outArray;
    }

    public static int FacilityTypeFromHal(int facilityType) {
         switch (facilityType) {
            case FacilityType.FACILITY_CLIP:
                return SuppSvcResponse.FACILITY_CLIP;
            case FacilityType.FACILITY_COLP:
                return SuppSvcResponse.FACILITY_COLP;
            case FacilityType.FACILITY_BAOC:
                return SuppSvcResponse.FACILITY_BAOC;
            case FacilityType.FACILITY_BAOICxH:
                return SuppSvcResponse.FACILITY_BAOICxH;
            case FacilityType.FACILITY_BAIC:
                return SuppSvcResponse.FACILITY_BAIC;
            case FacilityType.FACILITY_BAICr:
                return SuppSvcResponse.FACILITY_BAICr;
            case FacilityType.FACILITY_BA_ALL:
                return SuppSvcResponse.FACILITY_BA_ALL;
            case FacilityType.FACILITY_BA_MO:
                return SuppSvcResponse.FACILITY_BA_MO;
            case FacilityType.FACILITY_BA_MT:
                return SuppSvcResponse.FACILITY_BA_MT;
            case FacilityType.FACILITY_BS_MT:
                return SuppSvcResponse.FACILITY_BS_MT;
            case FacilityType.FACILITY_BAICa:
                return SuppSvcResponse.FACILITY_BAICa;
            case FacilityType.FACILITY_BAOIC:
                return SuppSvcResponse.FACILITY_BAOIC;
            case FacilityType.FACILITY_INVALID:
            default:
                return SuppSvcResponse.FACILITY_BA_ALL;
        }
    }

    public static int facilityTypeToHal(int facilityType) {
         switch (facilityType) {
            case SuppSvcResponse.FACILITY_CLIP:
                return FacilityType.FACILITY_CLIP;
            case SuppSvcResponse.FACILITY_COLP:
                return FacilityType.FACILITY_COLP;
            case SuppSvcResponse.FACILITY_BAOC:
                return FacilityType.FACILITY_BAOC;
            case SuppSvcResponse.FACILITY_BAOIC:
                return FacilityType.FACILITY_BAOIC;
            case SuppSvcResponse.FACILITY_BAOICxH:
                return FacilityType.FACILITY_BAOICxH;
            case SuppSvcResponse.FACILITY_BAIC:
                return FacilityType.FACILITY_BAIC;
            case SuppSvcResponse.FACILITY_BAICr:
                return FacilityType.FACILITY_BAICr;
            case SuppSvcResponse.FACILITY_BA_ALL:
                return FacilityType.FACILITY_BA_ALL;
            case SuppSvcResponse.FACILITY_BA_MO:
                return FacilityType.FACILITY_BA_MO;
            case SuppSvcResponse.FACILITY_BA_MT:
                return FacilityType.FACILITY_BA_MT;
            case SuppSvcResponse.FACILITY_BS_MT:
                return FacilityType.FACILITY_BS_MT;
            case SuppSvcResponse.FACILITY_BAICa:
                return FacilityType.FACILITY_BAICa;
            default:
                return FacilityType.FACILITY_INVALID;
        }
    }

    public static SuppSvcResponse suppSvcStatusResponseFromHal(
        vendor.qti.hardware.radio.ims.V1_3.SuppServiceStatus suppServiceStatus) {
        SuppSvcResponse suppSvcResponse = new SuppSvcResponse();

        suppSvcResponse.setStatus(serviceClassStatusFromHal(suppServiceStatus.status));

        suppSvcResponse.setProvisionStatus(
                serviceClassProvisionStatusFromHal(suppServiceStatus.provisionStatus));

        if (suppServiceStatus.facilityType != FacilityType.FACILITY_INVALID) {
            suppSvcResponse.setFacilityType(FacilityTypeFromHal(suppServiceStatus.facilityType));
        }

        if (suppServiceStatus.failureCause != null &&
                    !suppServiceStatus.failureCause.isEmpty()) {
            suppSvcResponse.setFailureCause(suppServiceStatus.failureCause);
        }

        for (CbNumListInfo numList : suppServiceStatus.cbNumListInfo) {
            if (numList.serviceClass == Integer.MAX_VALUE) {
                continue;
            }
            SuppSvcResponse.BarredLines lines =
                    new SuppSvcResponse.BarredLines(numList.serviceClass);

            if (numList.cbNumInfo != null) {
               for (CbNumInfo numInfo : numList.cbNumInfo) {
                    SuppSvcResponse.LineStatus lineStatus =
                        new SuppSvcResponse.LineStatus(numInfo.status,
                        numInfo.number);
                    lines.addLine(lineStatus);
                }
            }
            suppSvcResponse.addBarredLines(lines);
        }

        if (suppServiceStatus.hasErrorDetails) {
            suppSvcResponse.setErrorDetails(sipErrorFromHal(suppServiceStatus.errorDetails));
        }
        return suppSvcResponse;
    }

    public static int ipPresentationFromHal(int presentation) {
        switch(presentation) {
            case IpPresentation.IP_PRESENTATION_NUM_ALLOWED:
                return SuppService.IP_PRESENTATION_NUM_ALLOWED;
            case IpPresentation.IP_PRESENTATION_NUM_RESTRICTED:
                return SuppService.IP_PRESENTATION_NUM_RESTRICTED;
            case IpPresentation.IP_PRESENTATION_NUM_DEFAULT:
            default:
                 return SuppService.IP_PRESENTATION_NUM_DEFAULT;
        }
    }

    public static int ttyModeFromHal(int ttyMode) {
        switch(ttyMode) {
            case TtyMode.TTY_MODE_OFF:
                return CallDetails.TTY_MODE_OFF;
            case TtyMode.TTY_MODE_FULL:
                return CallDetails.TTY_MODE_FULL;
            case TtyMode.TTY_MODE_HCO:
                return CallDetails.TTY_MODE_HCO;
            case TtyMode.TTY_MODE_VCO:
                return CallDetails.TTY_MODE_VCO;
            default:
                 return CallDetails.TTY_MODE_OFF;
        }
    }

    public static int ttyModeToHal(int ttyMode) {
        switch(ttyMode) {
            case CallDetails.TTY_MODE_FULL:
                return TtyMode.TTY_MODE_FULL;
            case CallDetails.TTY_MODE_HCO:
                return TtyMode.TTY_MODE_HCO;
            case CallDetails.TTY_MODE_VCO:
                return TtyMode.TTY_MODE_VCO;
            case CallDetails.TTY_MODE_OFF:
                return TtyMode.TTY_MODE_OFF;
            default:
                 return TtyMode.TTY_MODE_INVALID;
        }
    }

    public static int clipStatusFromHal(int clipStatus) {
        switch(clipStatus) {
            case ClipStatus.NOT_PROVISIONED:
                return SuppService.NOT_PROVISIONED;
            case ClipStatus.PROVISIONED:
                return SuppService.PROVISIONED;
            case ClipStatus.STATUS_UNKNOWN:
            default:
                return SuppService.STATUS_UNKNOWN;
        }
    }

    public static int conferenceCallStateFromHal(int conferenceCallState) {
         switch(conferenceCallState) {
            case ConferenceCallState.RINGING:
                return ConfInfo.RINGING;
            case ConferenceCallState.FOREGROUND:
                return ConfInfo.FOREGROUND;
            case ConferenceCallState.BACKGROUND:
            default:
                return ConfInfo.BACKGROUND;
        }
    }

    public static CallModifyInfo buildCallModifyInfo(CallModify callModify) {
        log("buildCallModifyInfo callModify= " + callModify);
        CallModifyInfo callModifyInfo = new CallModifyInfo();
        callModifyInfo.callIndex = callModify.call_index;

        if (callModify.call_details != null) {
            callModifyInfo.hasCallDetails = true;
            callDetailsToHal(callModifyInfo.callDetails, callModify.call_details);
        }

        //CallModifyFailCause is not used for outgoing requests.
        callModifyInfo.failCause = CallModifyFailCause.E_INVALID;

        return callModifyInfo;
    }

    public static CallModify callModifyFromHal(CallModifyInfo callModifyInfo) {
        CallModify callModify = new CallModify();

        if (callModifyInfo.hasCallDetails) {
            callModify.call_details = getCallDetails(ImsRadioUtilsV16.
                    migrateCallDetails(callModifyInfo.callDetails));
        }

        if (callModifyInfo.callIndex != Integer.MAX_VALUE) {
            callModify.call_index = callModifyInfo.callIndex;
        }

        callModify.error = ImsErrorCode.SUCCESS;
        if (callModifyInfo.failCause != CallModifyFailCause.E_INVALID) {
            callModify.error = callModifyFailCauseFromHal(callModifyInfo.failCause);
        }

        return callModify;
    }

    private static int callModifyFailCauseFromHal(int failCause) {
        switch (failCause) {
            case CallModifyFailCause.E_SUCCESS:
                return ImsErrorCode.SUCCESS;
            case CallModifyFailCause.E_RADIO_NOT_AVAILABLE:
                return ImsErrorCode.RADIO_NOT_AVAILABLE;
            case CallModifyFailCause.E_GENERIC_FAILURE:
                return ImsErrorCode.GENERIC_FAILURE;
            case CallModifyFailCause.E_REQUEST_NOT_SUPPORTED:
                return ImsErrorCode.REQUEST_NOT_SUPPORTED;
            case CallModifyFailCause.E_CANCELLED:
                return ImsErrorCode.CANCELLED;
            case CallModifyFailCause.E_UNUSED:
                return ImsErrorCode.UNUSED;
            case CallModifyFailCause.E_INVALID_PARAMETER:
                return ImsErrorCode.INVALID_PARAMETER;
            case CallModifyFailCause.E_REJECTED_BY_REMOTE:
                return ImsErrorCode.REJECTED_BY_REMOTE;
            case CallModifyFailCause.E_IMS_DEREGISTERED:
                return ImsErrorCode.IMS_DEREGISTERED;
            case CallModifyFailCause.E_NETWORK_NOT_SUPPORTED:
                return ImsErrorCode.NETWORK_NOT_SUPPORTED;
            case CallModifyFailCause.E_HOLD_RESUME_FAILED:
                return ImsErrorCode.HOLD_RESUME_FAILED;
            case CallModifyFailCause.E_HOLD_RESUME_CANCELED:
                return ImsErrorCode.HOLD_RESUME_CANCELED;
            case CallModifyFailCause.E_REINVITE_COLLISION:
                return ImsErrorCode.REINVITE_COLLISION;
            default:
                return ImsErrorCode.SUCCESS;
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
        configInfo.errorCause = configFailureCauseToHal(errorCause);

        return configInfo;
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
            config.setErrorCause(configFailureCauseFromHal(configInfo.errorCause));
        }

        return config;
    }

    public static int configInfoItemToHal(int item) {
        switch (item) {
            case ImsConfigItem.NONE:
                return ConfigItem.CONFIG_ITEM_NONE;
            case ImsConfigItem.VOCODER_AMRMODESET:
                return ConfigItem.CONFIG_ITEM_VOCODER_AMRMODESET;
            case ImsConfigItem.VOCODER_AMRWBMODESET:
                return ConfigItem.CONFIG_ITEM_VOCODER_AMRWBMODESET;
            case ImsConfigItem.SIP_SESSION_TIMER:
                return ConfigItem.CONFIG_ITEM_SIP_SESSION_TIMER;
            case ImsConfigItem.MIN_SESSION_EXPIRY:
                return ConfigItem.CONFIG_ITEM_MIN_SESSION_EXPIRY;
            case ImsConfigItem.CANCELLATION_TIMER:
                return ConfigItem.CONFIG_ITEM_CANCELLATION_TIMER;
            case ImsConfigItem.T_DELAY:
                return ConfigItem.CONFIG_ITEM_T_DELAY;
            case ImsConfigItem.SILENT_REDIAL_ENABLE:
                return ConfigItem.CONFIG_ITEM_SILENT_REDIAL_ENABLE;
            case ImsConfigItem.SIP_T1_TIMER:
                return ConfigItem.CONFIG_ITEM_SIP_T1_TIMER;
            case ImsConfigItem.SIP_T2_TIMER:
                return ConfigItem.CONFIG_ITEM_SIP_T2_TIMER;
            case ImsConfigItem.SIP_TF_TIMER:
                return ConfigItem.CONFIG_ITEM_SIP_TF_TIMER;
            case ImsConfigItem.VLT_SETTING_ENABLED:
                return ConfigItem.CONFIG_ITEM_VLT_SETTING_ENABLED;
            case ImsConfigItem.LVC_SETTING_ENABLED:
                return ConfigItem.CONFIG_ITEM_LVC_SETTING_ENABLED;
            case ImsConfigItem.DOMAIN_NAME:
                return ConfigItem.CONFIG_ITEM_DOMAIN_NAME;
            case ImsConfigItem.SMS_FORMAT:
                return ConfigItem.CONFIG_ITEM_SMS_FORMAT;
            case ImsConfigItem.SMS_OVER_IP:
                return ConfigItem.CONFIG_ITEM_SMS_OVER_IP;
            case ImsConfigItem.PUBLISH_TIMER:
                return ConfigItem.CONFIG_ITEM_PUBLISH_TIMER;
            case ImsConfigItem.PUBLISH_TIMER_EXTENDED:
                return ConfigItem.CONFIG_ITEM_PUBLISH_TIMER_EXTENDED;
            case ImsConfigItem.CAPABILITIES_CACHE_EXPIRATION:
                return ConfigItem.CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION;
            case ImsConfigItem.AVAILABILITY_CACHE_EXPIRATION:
                return ConfigItem.CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION;
            case ImsConfigItem.CAPABILITIES_POLL_INTERVAL:
                return ConfigItem.CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL;
            case ImsConfigItem.SOURCE_THROTTLE_PUBLISH:
                return ConfigItem.CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH;
            case ImsConfigItem.MAX_NUM_ENTRIES_IN_RCL:
                return ConfigItem.CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL;
            case ImsConfigItem.CAPAB_POLL_LIST_SUB_EXP:
                return ConfigItem.CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP;
            case ImsConfigItem.GZIP_FLAG:
                return ConfigItem.CONFIG_ITEM_GZIP_FLAG;
            case ImsConfigItem.EAB_SETTING_ENABLED:
                return ConfigItem.CONFIG_ITEM_EAB_SETTING_ENABLED;
            case ImsConfigItem.MOBILE_DATA_ENABLED:
                return ConfigItem.CONFIG_ITEM_MOBILE_DATA_ENABLED;
            case ImsConfigItem.VOICE_OVER_WIFI_ENABLED:
                return ConfigItem.CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED;
            case ImsConfigItem.VOICE_OVER_WIFI_ROAMING:
                return ConfigItem.CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING;
            case ImsConfigItem.VOICE_OVER_WIFI_MODE:
                return ConfigItem.CONFIG_ITEM_VOICE_OVER_WIFI_MODE;
            case ImsConfigItem.CAPABILITY_DISCOVERY_ENABLED:
                return ConfigItem.CONFIG_ITEM_CAPABILITY_DISCOVERY_ENABLED;
            case ImsConfigItem.EMERGENCY_CALL_TIMER:
                return ConfigItem.CONFIG_ITEM_EMERGENCY_CALL_TIMER;
            case ImsConfigItem.SSAC_HYSTERESIS_TIMER:
                return ConfigItem.CONFIG_ITEM_SSAC_HYSTERESIS_TIMER;
            case ImsConfigItem.VOLTE_USER_OPT_IN_STATUS:
                return ConfigItem.CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS;
            case ImsConfigItem.LBO_PCSCF_ADDRESS:
                return ConfigItem.CONFIG_ITEM_LBO_PCSCF_ADDRESS;
            case ImsConfigItem.KEEP_ALIVE_ENABLED:
                return ConfigItem.CONFIG_ITEM_KEEP_ALIVE_ENABLED;
            case ImsConfigItem.REGISTRATION_RETRY_BASE_TIME_SEC:
                return ConfigItem.CONFIG_ITEM_REGISTRATION_RETRY_BASE_TIME_SEC;
            case ImsConfigItem.REGISTRATION_RETRY_MAX_TIME_SEC:
                return ConfigItem.CONFIG_ITEM_REGISTRATION_RETRY_MAX_TIME_SEC;
            case ImsConfigItem.SPEECH_START_PORT:
                return ConfigItem.CONFIG_ITEM_SPEECH_START_PORT;
            case ImsConfigItem.SPEECH_END_PORT:
                return ConfigItem.CONFIG_ITEM_SPEECH_END_PORT;
            case ImsConfigItem.SIP_INVITE_REQ_RETX_INTERVAL_MSEC:
                return ConfigItem.CONFIG_ITEM_SIP_INVITE_REQ_RETX_INTERVAL_MSEC;
            case ImsConfigItem.SIP_INVITE_RSP_WAIT_TIME_MSEC:
                return ConfigItem.CONFIG_ITEM_SIP_INVITE_RSP_WAIT_TIME_MSEC;
            case ImsConfigItem.SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC:
                return ConfigItem.CONFIG_ITEM_SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC;
            case ImsConfigItem.SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC:
                return ConfigItem.CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC;
            case ImsConfigItem.SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC:
                return ConfigItem.CONFIG_ITEM_SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC;
            case ImsConfigItem.SIP_INVITE_RSP_RETX_INTERVAL_MSEC:
                return ConfigItem.CONFIG_ITEM_SIP_INVITE_RSP_RETX_INTERVAL_MSEC;
            case ImsConfigItem.SIP_ACK_RECEIPT_WAIT_TIME_MSEC:
                return ConfigItem.CONFIG_ITEM_SIP_ACK_RECEIPT_WAIT_TIME_MSEC;
            case ImsConfigItem.SIP_ACK_RETX_WAIT_TIME_MSEC:
                return ConfigItem.CONFIG_ITEM_SIP_ACK_RETX_WAIT_TIME_MSEC;
            case ImsConfigItem.SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC:
                return ConfigItem.CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC;
            case ImsConfigItem.SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC:
                return ConfigItem.CONFIG_ITEM_SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC;
            case ImsConfigItem.AMR_WB_OCTET_ALIGNED_PT:
                return ConfigItem.CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT;
            case ImsConfigItem.AMR_WB_BANDWIDTH_EFFICIENT_PT:
                return ConfigItem.CONFIG_ITEM_AMR_WB_BANDWIDTH_EFFICIENT_PT;
            case ImsConfigItem.AMR_OCTET_ALIGNED_PT:
                return ConfigItem.CONFIG_ITEM_AMR_OCTET_ALIGNED_PT;
            case ImsConfigItem.AMR_BANDWIDTH_EFFICIENT_PT:
                return ConfigItem.CONFIG_ITEM_AMR_BANDWIDTH_EFFICIENT_PT;
            case ImsConfigItem.DTMF_WB_PT:
                return ConfigItem.CONFIG_ITEM_DTMF_WB_PT;
            case ImsConfigItem.DTMF_NB_PT:
                return ConfigItem.CONFIG_ITEM_DTMF_NB_PT;
            case ImsConfigItem.AMR_DEFAULT_MODE:
                return ConfigItem.CONFIG_ITEM_AMR_DEFAULT_MODE;
            case ImsConfigItem.SMS_PSI:
                return ConfigItem.CONFIG_ITEM_SMS_PSI;
            case ImsConfigItem.VIDEO_QUALITY:
                return ConfigItem.CONFIG_ITEM_VIDEO_QUALITY;
            case ImsConfigItem.THRESHOLD_LTE1:
                return ConfigItem.CONFIG_ITEM_THRESHOLD_LTE1;
            case ImsConfigItem.THRESHOLD_LTE2:
                return ConfigItem.CONFIG_ITEM_THRESHOLD_LTE2;
            case ImsConfigItem.THRESHOLD_LTE3:
                return ConfigItem.CONFIG_ITEM_THRESHOLD_LTE3;
            case ImsConfigItem.THRESHOLD_1x:
                return ConfigItem.CONFIG_ITEM_THRESHOLD_1x;
            case ImsConfigItem.THRESHOLD_WIFI_A:
                return ConfigItem.CONFIG_ITEM_THRESHOLD_WIFI_A;
            case ImsConfigItem.THRESHOLD_WIFI_B:
                return ConfigItem.CONFIG_ITEM_THRESHOLD_WIFI_B;
            case ImsConfigItem.T_EPDG_LTE:
                return ConfigItem.CONFIG_ITEM_T_EPDG_LTE;
            case ImsConfigItem.T_EPDG_WIFI:
                return ConfigItem.CONFIG_ITEM_T_EPDG_WIFI;
            case ImsConfigItem.T_EPDG_1x:
                return ConfigItem.CONFIG_ITEM_T_EPDG_1x;
            case ImsConfigItem.VWF_SETTING_ENABLED:
                return ConfigItem.CONFIG_ITEM_VWF_SETTING_ENABLED;
            case ImsConfigItem.VCE_SETTING_ENABLED:
                return ConfigItem.CONFIG_ITEM_VCE_SETTING_ENABLED;
            case ImsConfigItem.SMS_APP:
                return ConfigItem.CONFIG_ITEM_SMS_APP;
            case ImsConfigItem.VVM_APP:
                return ConfigItem.CONFIG_ITEM_VVM_APP;
            case ImsConfigItem.RTT_SETTING_ENABLED:
                return ConfigItem.CONFIG_ITEM_RTT_SETTING_ENABLED;
            default:
                return ConfigItem.CONFIG_ITEM_INVALID;
        }
    }

    public static int configInfoItemFromHal(int item) {
        switch (item) {
            case ConfigItem.CONFIG_ITEM_NONE:
                return ImsConfigItem.NONE;
            case ConfigItem.CONFIG_ITEM_VOCODER_AMRMODESET:
                return ImsConfigItem.VOCODER_AMRMODESET;
            case ConfigItem.CONFIG_ITEM_VOCODER_AMRWBMODESET:
                return ImsConfigItem.VOCODER_AMRWBMODESET;
            case ConfigItem.CONFIG_ITEM_SIP_SESSION_TIMER:
                return ImsConfigItem.SIP_SESSION_TIMER;
            case ConfigItem.CONFIG_ITEM_MIN_SESSION_EXPIRY:
                return ImsConfigItem.MIN_SESSION_EXPIRY;
            case ConfigItem.CONFIG_ITEM_CANCELLATION_TIMER:
                return ImsConfigItem.CANCELLATION_TIMER;
            case ConfigItem.CONFIG_ITEM_T_DELAY:
                return ImsConfigItem.T_DELAY;
            case ConfigItem.CONFIG_ITEM_SILENT_REDIAL_ENABLE:
                return ImsConfigItem.SILENT_REDIAL_ENABLE;
            case ConfigItem.CONFIG_ITEM_SIP_T1_TIMER:
                return ImsConfigItem.SIP_T1_TIMER;
            case ConfigItem.CONFIG_ITEM_SIP_T2_TIMER:
                return ImsConfigItem.SIP_T2_TIMER;
            case ConfigItem.CONFIG_ITEM_SIP_TF_TIMER:
                return ImsConfigItem.SIP_TF_TIMER;
            case ConfigItem.CONFIG_ITEM_VLT_SETTING_ENABLED:
                return ImsConfigItem.VLT_SETTING_ENABLED;
            case ConfigItem.CONFIG_ITEM_LVC_SETTING_ENABLED:
                return ImsConfigItem.LVC_SETTING_ENABLED;
            case ConfigItem.CONFIG_ITEM_DOMAIN_NAME:
                return ImsConfigItem.DOMAIN_NAME;
            case ConfigItem.CONFIG_ITEM_SMS_FORMAT:
                return ImsConfigItem.SMS_FORMAT;
            case ConfigItem.CONFIG_ITEM_SMS_OVER_IP:
                return ImsConfigItem.SMS_OVER_IP;
            case ConfigItem.CONFIG_ITEM_PUBLISH_TIMER:
                return ImsConfigItem.PUBLISH_TIMER;
            case ConfigItem.CONFIG_ITEM_PUBLISH_TIMER_EXTENDED:
                return ImsConfigItem.PUBLISH_TIMER_EXTENDED;
            case ConfigItem.CONFIG_ITEM_CAPABILITIES_CACHE_EXPIRATION:
                return ImsConfigItem.CAPABILITIES_CACHE_EXPIRATION;
            case ConfigItem.CONFIG_ITEM_AVAILABILITY_CACHE_EXPIRATION:
                return ImsConfigItem.AVAILABILITY_CACHE_EXPIRATION;
            case ConfigItem.CONFIG_ITEM_CAPABILITIES_POLL_INTERVAL:
                return ImsConfigItem.CAPABILITIES_POLL_INTERVAL;
            case ConfigItem.CONFIG_ITEM_SOURCE_THROTTLE_PUBLISH:
                return ImsConfigItem.SOURCE_THROTTLE_PUBLISH;
            case ConfigItem.CONFIG_ITEM_MAX_NUM_ENTRIES_IN_RCL:
                return ImsConfigItem.MAX_NUM_ENTRIES_IN_RCL;
            case ConfigItem.CONFIG_ITEM_CAPAB_POLL_LIST_SUB_EXP:
                return ImsConfigItem.CAPAB_POLL_LIST_SUB_EXP;
            case ConfigItem.CONFIG_ITEM_GZIP_FLAG:
                return ImsConfigItem.GZIP_FLAG;
            case ConfigItem.CONFIG_ITEM_EAB_SETTING_ENABLED:
                return ImsConfigItem.EAB_SETTING_ENABLED;
            case ConfigItem.CONFIG_ITEM_MOBILE_DATA_ENABLED:
                return ImsConfigItem.MOBILE_DATA_ENABLED;
            case ConfigItem.CONFIG_ITEM_VOICE_OVER_WIFI_ENABLED:
                return ImsConfigItem.VOICE_OVER_WIFI_ENABLED;
            case ConfigItem.CONFIG_ITEM_VOICE_OVER_WIFI_ROAMING:
                return ImsConfigItem.VOICE_OVER_WIFI_ROAMING;
            case ConfigItem.CONFIG_ITEM_VOICE_OVER_WIFI_MODE:
                return ImsConfigItem.VOICE_OVER_WIFI_MODE;
            case ConfigItem.CONFIG_ITEM_CAPABILITY_DISCOVERY_ENABLED:
                return ImsConfigItem.CAPABILITY_DISCOVERY_ENABLED;
            case ConfigItem.CONFIG_ITEM_EMERGENCY_CALL_TIMER:
                return ImsConfigItem.EMERGENCY_CALL_TIMER;
            case ConfigItem.CONFIG_ITEM_SSAC_HYSTERESIS_TIMER:
                return ImsConfigItem.SSAC_HYSTERESIS_TIMER;
            case ConfigItem.CONFIG_ITEM_VOLTE_USER_OPT_IN_STATUS:
                return ImsConfigItem.VOLTE_USER_OPT_IN_STATUS;
            case ConfigItem.CONFIG_ITEM_LBO_PCSCF_ADDRESS:
                return ImsConfigItem.LBO_PCSCF_ADDRESS;
            case ConfigItem.CONFIG_ITEM_KEEP_ALIVE_ENABLED:
                return ImsConfigItem.KEEP_ALIVE_ENABLED;
            case ConfigItem.CONFIG_ITEM_REGISTRATION_RETRY_BASE_TIME_SEC:
                return ImsConfigItem.REGISTRATION_RETRY_BASE_TIME_SEC;
            case ConfigItem.CONFIG_ITEM_REGISTRATION_RETRY_MAX_TIME_SEC:
                return ImsConfigItem.REGISTRATION_RETRY_MAX_TIME_SEC;
            case ConfigItem.CONFIG_ITEM_SPEECH_START_PORT:
                return ImsConfigItem.SPEECH_START_PORT;
            case ConfigItem.CONFIG_ITEM_SPEECH_END_PORT:
                return ImsConfigItem.SPEECH_END_PORT;
            case ConfigItem.CONFIG_ITEM_SIP_INVITE_REQ_RETX_INTERVAL_MSEC:
                return ImsConfigItem.SIP_INVITE_REQ_RETX_INTERVAL_MSEC;
            case ConfigItem.CONFIG_ITEM_SIP_INVITE_RSP_WAIT_TIME_MSEC:
                return ImsConfigItem.SIP_INVITE_RSP_WAIT_TIME_MSEC;
            case ConfigItem.CONFIG_ITEM_SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC:
                return ImsConfigItem.SIP_INVITE_RSP_RETX_WAIT_TIME_MSEC;
            case ConfigItem.CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC:
                return ImsConfigItem.SIP_NON_INVITE_REQ_RETX_INTERVAL_MSEC;
            case ConfigItem.CONFIG_ITEM_SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC:
                return ImsConfigItem.SIP_NON_INVITE_TXN_TIMEOUT_TIMER_MSEC;
            case ConfigItem.CONFIG_ITEM_SIP_INVITE_RSP_RETX_INTERVAL_MSEC:
                return ImsConfigItem.SIP_INVITE_RSP_RETX_INTERVAL_MSEC;
            case ConfigItem.CONFIG_ITEM_SIP_ACK_RECEIPT_WAIT_TIME_MSEC:
                return ImsConfigItem.SIP_ACK_RECEIPT_WAIT_TIME_MSEC;
            case ConfigItem.CONFIG_ITEM_SIP_ACK_RETX_WAIT_TIME_MSEC:
                return ImsConfigItem.SIP_ACK_RETX_WAIT_TIME_MSEC;
            case ConfigItem.CONFIG_ITEM_SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC:
                return ImsConfigItem.SIP_NON_INVITE_REQ_RETX_WAIT_TIME_MSEC;
            case ConfigItem.CONFIG_ITEM_SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC:
                return ImsConfigItem.SIP_NON_INVITE_RSP_RETX_WAIT_TIME_MSEC;
            case ConfigItem.CONFIG_ITEM_AMR_WB_OCTET_ALIGNED_PT:
                return ImsConfigItem.AMR_WB_OCTET_ALIGNED_PT;
            case ConfigItem.CONFIG_ITEM_AMR_WB_BANDWIDTH_EFFICIENT_PT:
                return ImsConfigItem.AMR_WB_BANDWIDTH_EFFICIENT_PT;
            case ConfigItem.CONFIG_ITEM_AMR_OCTET_ALIGNED_PT:
                return ImsConfigItem.AMR_OCTET_ALIGNED_PT;
            case ConfigItem.CONFIG_ITEM_AMR_BANDWIDTH_EFFICIENT_PT:
                return ImsConfigItem.AMR_BANDWIDTH_EFFICIENT_PT;
            case ConfigItem.CONFIG_ITEM_DTMF_WB_PT:
                return ImsConfigItem.DTMF_WB_PT;
            case ConfigItem.CONFIG_ITEM_DTMF_NB_PT:
                return ImsConfigItem.DTMF_NB_PT;
            case ConfigItem.CONFIG_ITEM_AMR_DEFAULT_MODE:
                return ImsConfigItem.AMR_DEFAULT_MODE;
            case ConfigItem.CONFIG_ITEM_SMS_PSI:
                return ImsConfigItem.SMS_PSI;
            case ConfigItem.CONFIG_ITEM_VIDEO_QUALITY:
                return ImsConfigItem.VIDEO_QUALITY;
            case ConfigItem.CONFIG_ITEM_THRESHOLD_LTE1:
                return ImsConfigItem.THRESHOLD_LTE1;
            case ConfigItem.CONFIG_ITEM_THRESHOLD_LTE2:
                return ImsConfigItem.THRESHOLD_LTE2;
            case ConfigItem.CONFIG_ITEM_THRESHOLD_LTE3:
                return ImsConfigItem.THRESHOLD_LTE3;
            case ConfigItem.CONFIG_ITEM_THRESHOLD_1x:
                return ImsConfigItem.THRESHOLD_1x;
            case ConfigItem.CONFIG_ITEM_THRESHOLD_WIFI_A:
                return ImsConfigItem.THRESHOLD_WIFI_A;
            case ConfigItem.CONFIG_ITEM_THRESHOLD_WIFI_B:
                return ImsConfigItem.THRESHOLD_WIFI_B;
            case ConfigItem.CONFIG_ITEM_T_EPDG_LTE:
                return ImsConfigItem.T_EPDG_LTE;
            case ConfigItem.CONFIG_ITEM_T_EPDG_WIFI:
                return ImsConfigItem.T_EPDG_WIFI;
            case ConfigItem.CONFIG_ITEM_T_EPDG_1x:
                return ImsConfigItem.T_EPDG_1x;
            case ConfigItem.CONFIG_ITEM_VWF_SETTING_ENABLED:
                return ImsConfigItem.VWF_SETTING_ENABLED;
            case ConfigItem.CONFIG_ITEM_VCE_SETTING_ENABLED:
                return ImsConfigItem.VCE_SETTING_ENABLED;
            case ConfigItem.CONFIG_ITEM_SMS_APP:
                return ImsConfigItem.SMS_APP;
            case ConfigItem.CONFIG_ITEM_VVM_APP:
                return ImsConfigItem.VVM_APP;
            default:
                return ImsConfigItem.NONE;
        }
    }

    public static int configFailureCauseToHal(int errorCause) {
        switch (errorCause) {
            case ImsConfigItem.NO_ERR:
                return ConfigFailureCause.CONFIG_NO_ERR;
            case ImsConfigItem.IMS_NOT_READY:
                return ConfigFailureCause.CONFIG_IMS_NOT_READY;
            case ImsConfigItem.FILE_NOT_AVAILABLE:
                return ConfigFailureCause.CONFIG_FILE_NOT_AVAILABLE;
            case ImsConfigItem.READ_FAILED:
                return ConfigFailureCause.CONFIG_READ_FAILED;
            case ImsConfigItem.WRITE_FAILED:
                return ConfigFailureCause.CONFIG_WRITE_FAILED;
            case ImsConfigItem.OTHER_INTERNAL_ERR:
                return ConfigFailureCause.CONFIG_OTHER_INTERNAL_ERR;
            default:
                return ConfigFailureCause.CONFIG_FAILURE_INVALID;
        }
    }

    public static int configFailureCauseFromHal(int errorCause) {
        switch (errorCause) {
            case ConfigFailureCause.CONFIG_NO_ERR:
                return ImsConfigItem.NO_ERR;
            case ConfigFailureCause.CONFIG_IMS_NOT_READY:
                return ImsConfigItem.IMS_NOT_READY;
            case ConfigFailureCause.CONFIG_FILE_NOT_AVAILABLE:
                return ImsConfigItem.FILE_NOT_AVAILABLE;
            case ConfigFailureCause.CONFIG_READ_FAILED:
                return ImsConfigItem.READ_FAILED;
            case ConfigFailureCause.CONFIG_WRITE_FAILED:
                return ImsConfigItem.WRITE_FAILED;
            case ConfigFailureCause.CONFIG_OTHER_INTERNAL_ERR:
            default:
                return ImsConfigItem.OTHER_INTERNAL_ERR;
        }
    }

    public static CbNumListInfo buildCbNumListInfo(String[] inCbNumList, int serviceClass) {
        CbNumListInfo outCbNumListInfo = new CbNumListInfo();

        if (serviceClass != Integer.MAX_VALUE) {
            outCbNumListInfo.serviceClass = serviceClass;
        }

        if (inCbNumList != null) {
            for (int i = 0; i < inCbNumList.length; ++i) {
                CbNumInfo cbNumInfo = new CbNumInfo();
                cbNumInfo.status = ServiceClassStatus.INVALID;
                if (inCbNumList[i] != null) {
                    cbNumInfo.number = inCbNumList[i];
                }
                outCbNumListInfo.cbNumInfo.add(cbNumInfo);
            }
        }

        return outCbNumListInfo;
    }

    public static int messageTypeFromHal(int type) {
        switch (type) {
            case MessageType.MWI_MSG_VOICE:
                return Mwi.MWI_MSG_VOICE;
            case MessageType.MWI_MSG_VIDEO:
                return Mwi.MWI_MSG_VIDEO;
            case MessageType.MWI_MSG_FAX:
                return Mwi.MWI_MSG_FAX;
            case MessageType.MWI_MSG_PAGER:
                return Mwi.MWI_MSG_PAGER;
            case MessageType.MWI_MSG_MULTIMEDIA:
                return Mwi.MWI_MSG_MULTIMEDIA;
            case MessageType.MWI_MSG_TEXT:
                return Mwi.MWI_MSG_TEXT;
            case MessageType.MWI_MSG_NONE:
            default:
                return Mwi.MWI_MSG_NONE;
        }
    }

    public static int messagePriorityFromHal(int type) {
        switch (type) {
            case MessagePriority.MWI_MSG_PRIORITY_LOW:
                return Mwi.MWI_MSG_PRIORITY_LOW;
            case MessagePriority.MWI_MSG_PRIORITY_NORMAL:
                return Mwi.MWI_MSG_PRIORITY_NORMAL;
            case MessagePriority.MWI_MSG_PRIORITY_URGENT:
                return Mwi.MWI_MSG_PRIORITY_URGENT;
            case MessagePriority.MWI_MSG_PRIORITY_UNKNOWN:
            default:
                return Mwi.MWI_MSG_PRIORITY_UNKNOWN;
        }
    }

    public static Mwi messageWaitingIndicationFromHal(MessageWaitingIndication
            messageWaitingIndication) {
        if (messageWaitingIndication == null) {
            return null;
        }

        Mwi mwi = new Mwi();
        Log.i(TAG, "messageWaitingIndicationFromHal summaryCount = " +
                  messageWaitingIndication.messageSummary.size());
        for (MessageSummary summary : messageWaitingIndication.messageSummary) {
            if (summary != null) {
                mwi.mwiMsgSummary.add(messageSummaryFromHal(summary));
            }
        }
        if (!messageWaitingIndication.ueAddress.isEmpty()) {
            mwi.mUeAddress = messageWaitingIndication.ueAddress;
        }
        Log.i(TAG, "messageWaitingIndicationFromHal detailsCount = " +
                  messageWaitingIndication.messageDetails.size());
        for (MessageDetails details : messageWaitingIndication.messageDetails) {
            if (details != null) {
                mwi.mwiMsgDetails.add(messageDetailsFromHal(details));
            }
        }
        return mwi;
    }

    private static Mwi.MwiMessageSummary messageSummaryFromHal(MessageSummary summary) {
        if (summary == null) {
            return null;
        }

        Mwi.MwiMessageSummary mwiMessageSummary = new Mwi.MwiMessageSummary();

        if (summary.type != MessageType.MWI_MSG_INVALID) {
            mwiMessageSummary.mMessageType = messageTypeFromHal(summary.type);
        }

        if (summary.newMessageCount != Integer.MAX_VALUE) {
            mwiMessageSummary.mNewMessage = summary.newMessageCount;
        }

        if (summary.oldMessageCount != Integer.MAX_VALUE) {
            mwiMessageSummary.mOldMessage = summary.oldMessageCount;
        }

        if (summary.newUrgentMessageCount != Integer.MAX_VALUE) {
            mwiMessageSummary.mNewUrgent = summary.newUrgentMessageCount;
        }

        if (summary.oldUrgentMessageCount != Integer.MAX_VALUE) {
            mwiMessageSummary.mOldUrgent = summary.oldUrgentMessageCount;
        }
        return mwiMessageSummary;
    }

    private static Mwi.MwiMessageDetails messageDetailsFromHal(MessageDetails details) {
        if (details == null) {
            return null;
        }

        Mwi.MwiMessageDetails mwiMessageDetails = new Mwi.MwiMessageDetails();

        if (!details.toAddress.isEmpty()) {
            mwiMessageDetails.mToAddress = details.toAddress;
        }

        if (!details.fromAddress.isEmpty()) {
            mwiMessageDetails.mFromAddress = details.fromAddress;
        }

        if (!details.subject.isEmpty()) {
            mwiMessageDetails.mSubject = details.subject;
        }

        if (!details.date.isEmpty()) {
            mwiMessageDetails.mDate = details.date;
        }

        if (details.priority != MessagePriority.MWI_MSG_PRIORITY_INVALID) {
            mwiMessageDetails.mPriority = messagePriorityFromHal(details.priority);
        }

        if (!details.id.isEmpty()) {
            mwiMessageDetails.mMessageId = details.id;
        }

        if (details.type != MessageType.MWI_MSG_INVALID) {
            mwiMessageDetails.mMessageType = messageTypeFromHal(details.type);
        }
        return mwiMessageDetails;
    }

    public static ExplicitCallTransferInfo buildExplicitCallTransferInfo(int srcCallId, int type,
        String number, int destCallId) {
        ExplicitCallTransferInfo ectInfo = new ExplicitCallTransferInfo();
        ectInfo.callId = srcCallId;
        ectInfo.ectType = ectTypeToHal(type);
        if (number != null) {
            ectInfo.targetAddress = number;
        }
        if (destCallId > 0) {
            ectInfo.targetCallId = destCallId;
        } else {
            ectInfo.targetCallId = Integer.MAX_VALUE;
        }
        return ectInfo;
    }

    private static int ectTypeToHal(int type) {
        switch (type) {
            case EctTypeInfo.BLIND_TRANSFER:
                return EctType.BLIND_TRANSFER;
            case EctTypeInfo.ASSURED_TRANSFER:
                return EctType.ASSURED_TRANSFER;
            case EctTypeInfo.CONSULTATIVE_TRANSFER:
                return EctType.CONSULTATIVE_TRANSFER;
            default:
                return EctType.INVALID;
        }
    }

    public static GeoLocationInfo
            geolocationIndicationFromHal(double lat, double lon) {
        // Note: The UNSOL from lower layers is actually a 'query' that only
        // has the location coordinates. Address is not required.
        return new GeoLocationInfo(lat, lon);
    }

    public static ImsSubConfigDetails imsSubconfigFromHal(ImsSubConfigInfo subConfigInfo) {

        if (subConfigInfo == null) {
            return null;
        }

        ImsSubConfigDetails subConfig = new ImsSubConfigDetails();
        if (subConfigInfo.simultStackCount != Integer.MAX_VALUE) {
            subConfig.setSimultStackCount(subConfigInfo.simultStackCount);
        }

        for (int i = 0; i < subConfigInfo.imsStackEnabled.size(); i++) {
            subConfig.addImsStackEnabled(subConfigInfo.imsStackEnabled.get(i));
        }

        return subConfig;
    }

    public static int operationFromHal(int type) {
        switch (type) {
            case ConfParticipantOperation.ADD:
                return ParticipantStatusDetails.ADD;
            case ConfParticipantOperation.REMOVE:
                return ParticipantStatusDetails.REMOVE;
            default:
                return ParticipantStatusDetails.INVALID;
        }
    }

    public static ParticipantStatusDetails participantStatusFromHal(
            ParticipantStatusInfo participantStatusInfo) {

        if (participantStatusInfo == null) {
            return null;
        }

        ParticipantStatusDetails participantStatus = new ParticipantStatusDetails();

        if (participantStatusInfo.callId != Integer.MAX_VALUE) {
            participantStatus.setCallId(participantStatusInfo.callId);
        }

        if (participantStatusInfo.operation != ConfParticipantOperation.INVALID) {
            participantStatus.setOperation(operationFromHal(participantStatusInfo.operation));
        }

        participantStatus.setParticipantUri(participantStatusInfo.participantUri);

        if (participantStatusInfo.sipStatus != Integer.MAX_VALUE) {
            participantStatus.setSipStatus(participantStatusInfo.sipStatus);
        }

        if (participantStatusInfo.hasIsEct) {
            participantStatus.setIsEct(participantStatusInfo.isEct);
        }

        return participantStatus;
    }

    public static RegistrationBlockStatusInfo registrationBlockStatusFromHal(
            boolean hasBlockStatusOnWwan, BlockStatus blockStatusOnWwan,
            boolean hasBlockStatusOnWlan, BlockStatus blockStatusOnWlan) {
        RegistrationBlockStatusInfo regBlockStatus = new RegistrationBlockStatusInfo();

        if (hasBlockStatusOnWwan) {
            regBlockStatus.setStatusOnWwan(blockStatusFromHal(blockStatusOnWwan));
        }

        if (hasBlockStatusOnWlan) {
            regBlockStatus.setStatusOnWlan(blockStatusFromHal(blockStatusOnWlan));
        }

        return regBlockStatus;
    }

    public static BlockStatusInfo blockStatusFromHal(BlockStatus inBlockStatus) {
        if (inBlockStatus == null) {
            return null;
        }

        BlockStatusInfo outBlockStatus = new BlockStatusInfo();

        if (inBlockStatus.blockReason != BlockReasonType.INVALID) {
            outBlockStatus.setReason(blockReasonTypeFromHal(inBlockStatus.blockReason));
        }

        if (inBlockStatus.hasBlockReasonDetails) {
            outBlockStatus.setReasonDetails(blockReasonDetailsFromHal(
                    inBlockStatus.blockReasonDetails));
        }

        return outBlockStatus;
    }

    public static int blockReasonTypeFromHal(int inBlockReason) {
        switch (inBlockReason) {
            case BlockReasonType.PDP_FAILURE:
                return BlockStatusInfo.REASON_PDP_FAILURE;
            case BlockReasonType.REGISTRATION_FAILURE:
                return BlockStatusInfo.REASON_REGISTRATION_FAILURE;
            case BlockReasonType.HANDOVER_FAILURE:
                return BlockStatusInfo.REASON_HANDOVER_FAILURE;
            case BlockReasonType.OTHER_FAILURE:
                return BlockStatusInfo.REASON_OTHER_FAILURE;
            default:
                return BlockStatusInfo.REASON_INVALID;
        }
    }

    public static BlockReasonDetailsInfo blockReasonDetailsFromHal(
            BlockReasonDetails inBlockReasonDetails) {
        BlockReasonDetailsInfo outBlockReasonDetails = new BlockReasonDetailsInfo();

        if (inBlockReasonDetails.regFailureReasonType != RegFailureReasonType.INVALID) {
            outBlockReasonDetails.setRegFailureReasonType(regFailureReasonTypeFromHal(
                    inBlockReasonDetails.regFailureReasonType));
        }

        if (inBlockReasonDetails.regFailureReason != Integer.MAX_VALUE) {
            outBlockReasonDetails.setRegFailureReason(inBlockReasonDetails.regFailureReason);
        }

        return outBlockReasonDetails;
    }

    public static int regFailureReasonTypeFromHal(int inRegFailureReasonType) {
        switch (inRegFailureReasonType) {
            case RegFailureReasonType.MOBILE_IP:
                return BlockReasonDetailsInfo.REG_FAILURE_REASON_TYPE_MOBILE_IP;
            case RegFailureReasonType.INTERNAL:
                return BlockReasonDetailsInfo.REG_FAILURE_REASON_TYPE_INTERNAL;
            case RegFailureReasonType.CALL_MANAGER_DEFINED:
                return BlockReasonDetailsInfo.REG_FAILURE_REASON_TYPE_CALL_MANAGER_DEFINED;
            case RegFailureReasonType.TYPE_3GPP_SPEC_DEFINED:
                return BlockReasonDetailsInfo.REG_FAILURE_REASON_TYPE_3GPP_SPEC_DEFINED;
            case RegFailureReasonType.PPP:
                return BlockReasonDetailsInfo.REG_FAILURE_REASON_TYPE_PPP;
            case RegFailureReasonType.EHRPD:
                return BlockReasonDetailsInfo.REG_FAILURE_REASON_TYPE_EHRPD;
            case RegFailureReasonType.IPV6:
                return BlockReasonDetailsInfo.REG_FAILURE_REASON_TYPE_IPV6;
            case RegFailureReasonType.IWLAN:
                return BlockReasonDetailsInfo.REG_FAILURE_REASON_TYPE_IWLAN;
            case RegFailureReasonType.HANDOFF:
                return BlockReasonDetailsInfo.REG_FAILURE_REASON_TYPE_HANDOFF;
            case RegFailureReasonType.UNSPECIFIED:
                return BlockReasonDetailsInfo.REG_FAILURE_REASON_TYPE_UNSPECIFIED;
            default:
                return BlockReasonDetailsInfo.REG_FAILURE_REASON_TYPE_INVALID;
        }
    }

    public static Object voWiFiCallQualityFromHal(int voWiFiCallQuality) {
        int[] ret = new int[1];
        switch (voWiFiCallQuality) {
            case VoWiFiCallQualityInfo.VOWIFI_CALL_QUALITY_EXCELLENT:
                ret[0] = VoWiFiQuality.QUALITY_EXCELLENT;
                break;
            case VoWiFiCallQualityInfo.VOWIFI_CALL_QUALITY_FAIR:
                ret[0] = VoWiFiQuality.QUALITY_FAIR;
                break;
            case VoWiFiCallQualityInfo.VOWIFI_CALL_QUALITY_BAD:
                ret[0] = VoWiFiQuality.QUALITY_BAD;
                break;
            default:
                ret[0] = VoWiFiQuality.QUALITY_NONE;
                break;
        }
        return ret;
    }

    public static UssdInfo ussdInfoFromHal(int type, SipErrorInfo errorDetails) {
        UssdInfo info = new UssdInfo();

        switch (type) {
            case UssdModeType.NOTIFY:
                info.setType(UssdInfo.NOTIFY);
                break;
            case UssdModeType.REQUEST:
                info.setType(UssdInfo.REQUEST);
                break;
            case UssdModeType.NW_RELEASE:
                info.setType(UssdInfo.NW_RELEASE);
                break;
            case UssdModeType.LOCAL_CLIENT:
                info.setType(UssdInfo.LOCAL_CLIENT);
                break;
            case UssdModeType.NOT_SUPPORTED:
                info.setType(UssdInfo.NOT_SUPPORTED);
                break;
            case UssdModeType.NW_TIMEOUT:
                info.setType(UssdInfo.NW_TIMEOUT);
                break;
        }

        if (errorDetails.errorCode != Integer.MAX_VALUE) {
            info.setErrorCode(errorDetails.errorCode);
        }
        info.setErrorMessage(errorDetails.errorString);
        return info;
    }

    public static int serviceTypeFromRILServiceType(int serviceType) {
        switch (serviceType) {
            case SsServiceType.SS_CFU:
                return ImsSsData.SS_CFU;
            case SsServiceType.SS_CF_BUSY:
                return ImsSsData.SS_CF_BUSY;
            case SsServiceType.SS_CF_NO_REPLY:
                return ImsSsData.SS_CF_NO_REPLY;
            case SsServiceType.SS_CF_NOT_REACHABLE:
                return ImsSsData.SS_CF_NOT_REACHABLE;
            case SsServiceType.SS_CF_ALL:
                return ImsSsData.SS_CF_ALL;
            case SsServiceType.SS_CF_ALL_CONDITIONAL:
                return ImsSsData.SS_CF_ALL_CONDITIONAL;
            case SsServiceType.SS_CFUT:
                return ImsSsData.SS_CFUT;
            case SsServiceType.SS_CLIP:
                return ImsSsData.SS_CLIP;
            case SsServiceType.SS_CLIR:
                return ImsSsData.SS_CLIR;
            case SsServiceType.SS_COLP:
                return ImsSsData.SS_COLP;
            case SsServiceType.SS_COLR:
                return ImsSsData.SS_COLR;
            case SsServiceType.SS_CNAP:
                return ImsSsData.SS_CNAP;
            case SsServiceType.SS_WAIT:
                return ImsSsData.SS_WAIT;
            case SsServiceType.SS_BAOC:
                return ImsSsData.SS_BAOC;
            case SsServiceType.SS_BAOIC:
                return ImsSsData.SS_BAOIC;
            case SsServiceType.SS_BAOIC_EXC_HOME:
                return ImsSsData.SS_BAOIC_EXC_HOME;
            case SsServiceType.SS_BAIC:
                return ImsSsData.SS_BAIC;
            case SsServiceType.SS_BAIC_ROAMING:
                return ImsSsData.SS_BAIC_ROAMING;
            case SsServiceType.SS_ALL_BARRING:
                return ImsSsData.SS_ALL_BARRING;
            case SsServiceType.SS_OUTGOING_BARRING:
                return ImsSsData.SS_OUTGOING_BARRING;
            case SsServiceType.SS_INCOMING_BARRING:
                return ImsSsData.SS_INCOMING_BARRING;
            case SsServiceType.SS_INCOMING_BARRING_DN:
                return ImsSsData.SS_INCOMING_BARRING_DN;
            case SsServiceType.SS_INCOMING_BARRING_ANONYMOUS:
                return ImsSsData.SS_INCOMING_BARRING_ANONYMOUS;
            default:
        }
        return ImsUtInterface.INVALID;
    }

    public static int requestTypeFromRILRequestType(int requestType) {
        switch (requestType) {
            case SsRequestType.SS_ACTIVATION:
                return ImsSsData.SS_ACTIVATION;
            case SsRequestType.SS_DEACTIVATION:
                return ImsSsData.SS_DEACTIVATION;
            case SsRequestType.SS_INTERROGATION:
                return ImsSsData.SS_INTERROGATION;
            case SsRequestType.SS_REGISTRATION:
                return ImsSsData.SS_REGISTRATION;
            case SsRequestType.SS_ERASURE:
                return ImsSsData.SS_ERASURE;
            default:
        }
        return ImsUtInterface.INVALID;
    }

    public static int teleserviceTypeFromRILTeleserviceType(int teleservice) {
        switch (teleservice) {
            case SsTeleserviceType.SS_ALL_TELE_AND_BEARER_SERVICES:
                return ImsSsData.SS_ALL_TELE_AND_BEARER_SERVICES;
            case SsTeleserviceType.SS_ALL_TELESEVICES:
                return ImsSsData.SS_ALL_TELESEVICES;
            case SsTeleserviceType.SS_TELEPHONY:
                return ImsSsData.SS_TELEPHONY;
            case SsTeleserviceType.SS_ALL_DATA_TELESERVICES:
                return ImsSsData.SS_ALL_DATA_TELESERVICES;
            case SsTeleserviceType.SS_SMS_SERVICES:
                return ImsSsData.SS_SMS_SERVICES;
            case SsTeleserviceType.SS_ALL_TELESERVICES_EXCEPT_SMS:
                return ImsSsData.SS_ALL_TELESERVICES_EXCEPT_SMS;
            default:
        }
        return ImsUtInterface.INVALID;
    }

    public static int getUTInterfaceCFReasonFromCommandsInterfaceCFReason(
            int commandsInterfaceCFReason) {
        switch(commandsInterfaceCFReason) {
            case ImsPhoneCommandsInterface.CF_REASON_UNCONDITIONAL:
                return ImsUtInterface.CDIV_CF_UNCONDITIONAL;
            case ImsPhoneCommandsInterface.CF_REASON_BUSY:
                return ImsUtInterface.CDIV_CF_BUSY;
            case ImsPhoneCommandsInterface.CF_REASON_NO_REPLY:
                return ImsUtInterface.CDIV_CF_NO_REPLY;
            case ImsPhoneCommandsInterface.CF_REASON_NOT_REACHABLE:
                return ImsUtInterface.CDIV_CF_NOT_REACHABLE;
            case ImsPhoneCommandsInterface.CF_REASON_ALL:
                return ImsUtInterface.CDIV_CF_ALL;
            case ImsPhoneCommandsInterface.CF_REASON_ALL_CONDITIONAL:
                return ImsUtInterface.CDIV_CF_ALL_CONDITIONAL;
            default:
                break;
        }
        return ImsUtInterface.INVALID;
    }

    private static int migrateCallFailCauseToV11(int callFailCause) {
        //We need this check to make sure that an invalid call fail cause in
        //V10 gets mapped to an invalid call fail cause in V11. Otherwise it
        //might get mapped to a valid fail cause.
        return (callFailCause == CallFailCause.CALL_FAIL_INVALID) ?
            vendor.qti.hardware.radio.ims.V1_1.CallFailCause.CALL_FAIL_INVALID :
                callFailCause;
    }

    private static vendor.qti.hardware.radio.ims.V1_1.CallInfo migrateCallInfoToV11(
            CallInfo callInfo) {

        vendor.qti.hardware.radio.ims.V1_1.CallInfo callInfoV11 =
                new vendor.qti.hardware.radio.ims.V1_1.CallInfo();

        callInfoV11.state = callInfo.state;
        callInfoV11.index = callInfo.index;
        callInfoV11.toa = callInfo.toa;
        callInfoV11.hasIsMpty = callInfo.hasIsMpty;
        callInfoV11.isMpty = callInfo.isMpty;
        callInfoV11.hasIsMT = callInfo.hasIsMT;
        callInfoV11.isMT = callInfo.isMT;
        callInfoV11.als = callInfo.als;
        callInfoV11.hasIsVoice = callInfo.hasIsVoice;
        callInfoV11.isVoice = callInfo.isVoice;
        callInfoV11.hasIsVoicePrivacy = callInfo.hasIsVoicePrivacy;
        callInfoV11.isVoicePrivacy = callInfo.isVoicePrivacy;
        callInfoV11.number = callInfo.number;
        callInfoV11.numberPresentation = callInfo.numberPresentation;
        callInfoV11.name = callInfo.name;
        callInfoV11.namePresentation = callInfo.namePresentation;

        callInfoV11.hasCallDetails = callInfo.hasCallDetails;
        callInfoV11.callDetails.callType = callInfo.callDetails.callType;
        callInfoV11.callDetails.callDomain = callInfo.callDetails.callDomain;
        callInfoV11.callDetails.extrasLength = callInfo.callDetails.extrasLength;

        for(int i=0; i < callInfo.callDetails.extras.size(); i++) {
            callInfoV11.callDetails.extras.add(callInfo.callDetails.extras.get(i));
        }

        for(int i=0; i < callInfo.callDetails.localAbility.size(); i++) {
            callInfoV11.callDetails.localAbility.add(callInfo.callDetails.localAbility.get(i));
        }

        for(int i=0; i < callInfo.callDetails.peerAbility.size(); i++) {
            callInfoV11.callDetails.peerAbility.add(callInfo.callDetails.peerAbility.get(i));
        }

        callInfoV11.callDetails.callSubstate = callInfo.callDetails.callSubstate;
        callInfoV11.callDetails.mediaId = callInfo.callDetails.mediaId;
        callInfoV11.callDetails.causeCode = callInfo.callDetails.causeCode;
        callInfoV11.callDetails.rttMode = callInfo.callDetails.rttMode;
        callInfoV11.callDetails.sipAlternateUri = callInfo.callDetails.sipAlternateUri;

        callInfoV11.hasFailCause = callInfo.hasFailCause;
        callInfoV11.failCause.failCause =
                migrateCallFailCauseToV11(callInfo.failCause.failCause);

        for(int i=0; i < callInfo.failCause.errorinfo.size(); i++) {
            callInfoV11.failCause.errorinfo.add(callInfo.failCause.errorinfo.get(i));
        }

        callInfoV11.failCause.networkErrorString = callInfo.failCause.networkErrorString;
        callInfoV11.failCause.hasErrorDetails = callInfo.failCause.hasErrorDetails;
        callInfoV11.failCause.errorDetails.errorCode = callInfo.failCause.errorDetails.errorCode;
        callInfoV11.failCause.errorDetails.errorString =
                callInfo.failCause.errorDetails.errorString;

        callInfoV11.hasIsEncrypted = callInfo.hasIsEncrypted;
        callInfoV11.isEncrypted = callInfo.isEncrypted;
        callInfoV11.hasIsCalledPartyRinging = callInfo.hasIsCalledPartyRinging;
        callInfoV11.isCalledPartyRinging = callInfo.isCalledPartyRinging;
        callInfoV11.historyInfo = callInfo.historyInfo;
        callInfoV11.hasIsVideoConfSupported = callInfo.hasIsVideoConfSupported;
        callInfoV11.isVideoConfSupported = callInfo.isVideoConfSupported;

        return callInfoV11;
    }


    public static ArrayList<vendor.qti.hardware.radio.ims.V1_1.CallInfo>
            migrateCallListToV11(ArrayList<CallInfo> callList) {

        if (callList == null) {
            return null;
        }
        ArrayList<vendor.qti.hardware.radio.ims.V1_1.CallInfo> list =
                new ArrayList<vendor.qti.hardware.radio.ims.V1_1.CallInfo>();

        for (CallInfo callInfo : callList) {
            vendor.qti.hardware.radio.ims.V1_1.CallInfo info = migrateCallInfoToV11(callInfo);
            list.add(info);
        }
        return list;
    }

    public static vendor.qti.hardware.radio.ims.V1_2.AddressInfo
            getHidlAddressInfo(double lat, double lon, Address address) {
        String info;
        vendor.qti.hardware.radio.ims.V1_2.AddressInfo addressInfo =
                new vendor.qti.hardware.radio.ims.V1_2.AddressInfo();
        if (address == null) {
            Log.v(TAG, "Null Address!");
            return addressInfo;
        }

        info = address.getLocality();
        if (info != null) {
            addressInfo.city = info;
        }
        info = address.getAdminArea();
        if (info != null) {
            addressInfo.state = info;
        }
        info = address.getCountryName();
        if (info != null) {
            addressInfo.country = info;
        }
        info = address.getPostalCode();
        if (info != null) {
            addressInfo.postalCode = info;
        }
        info = address.getCountryCode();
        if (info != null) {
            addressInfo.countryCode = info;
        }
        info = address.getThoroughfare();
        if (info != null) {
            addressInfo.street = info;
        }
        info = address.getSubThoroughfare();
        if (info != null) {
            addressInfo.houseNumber = info;
        }
        Log.v(TAG, "address=" + address + ",houseNumber=" + address.getSubThoroughfare());
        return addressInfo;
    }

    public static AddressInfo
            migrateAddressToV10(vendor.qti.hardware.radio.ims.V1_2.AddressInfo addressInfo) {
        AddressInfo addressInfoV10 = new AddressInfo();
        addressInfoV10.city = addressInfo.city;
        addressInfoV10.state = addressInfo.state;
        addressInfoV10.country = addressInfo.country;
        addressInfoV10.postalCode = addressInfo.postalCode;
        addressInfoV10.countryCode = addressInfo.countryCode;
        return addressInfoV10;
    }

    public static HangupRequestInfo migrateHangupRequestInfoFromV13(
                vendor.qti.hardware.radio.ims.V1_3.HangupRequestInfo from) {
        HangupRequestInfo to = new HangupRequestInfo();
        to.connIndex = from.connIndex;
        to.hasMultiParty = from.hasMultiParty;
        to.multiParty = from.multiParty;
        to.connUri = from.connUri;
        to.conf_id = from.conf_id;
        to.hasFailCauseResponse = from.hasFailCauseResponse;
        to.failCauseResponse.failCause = migrateCallFailCausefromV13(
                from.failCauseResponse.failCause);

        for(Byte errorInfo : from.failCauseResponse.errorinfo) {
            to.failCauseResponse.errorinfo.add(errorInfo);
        }

        to.failCauseResponse.networkErrorString = from.failCauseResponse.networkErrorString;
        to.failCauseResponse.hasErrorDetails = from.failCauseResponse.hasErrorDetails;
        to.failCauseResponse.errorDetails.errorCode =
                from.failCauseResponse.errorDetails.errorCode;
        to.failCauseResponse.errorDetails.errorString =
                from.failCauseResponse.errorDetails.errorString;
        return to;
    }

    //Should be used only if the user is certain that a fail cause
    //that was not introduced in V13, should be present in V10
    private static int migrateCallFailCausefromV13(int failCause) {
        if (ImsRadioUtilsV13.isFailCauseIntroducedInV13(failCause)) {
            return CallFailCause.CALL_FAIL_ERROR_UNSPECIFIED;
        }
        //Assuming the fail cause is present in V10
        return failCause;
    }

    /*
      This functions throws an ImsException if the ConfigItem was introduced in V15
      when converting V15 to V10 ConfigInfo
    */
    public static ConfigInfo migrateConfigInfoFromV15(
                vendor.qti.hardware.radio.ims.V1_5.ConfigInfo from) throws ImsException {
        ConfigInfo to = new ConfigInfo();

        if (ImsRadioUtilsV15.isConfigItemIntroducedInV15(from.item)) {
            throw new ImsException("Config item not supported in current HAL");
        }
        to.item = from.item;
        to.hasBoolValue = from.hasBoolValue;
        to.boolValue = from.boolValue;
        to.intValue = from.intValue;
        if (from.stringValue != null) {
            to.stringValue = from.stringValue;
        }
        to.errorCause = configFailureCauseToHal(from.errorCause);

        return to;
    }

    // Map TirMode to a boolean value, where if mode is temporary return true
    // all other values return false
    private static boolean isTirOverwriteAllowed(int tirMode) {
        return tirMode == TirMode.TEMPORARY;
    }

    public static vendor.qti.hardware.radio.ims.V1_0.ServiceStatusInfo
            migrateServiceStatusInfoFromV16(ServiceStatusInfo from) {
        vendor.qti.hardware.radio.ims.V1_0.ServiceStatusInfo to =
                new vendor.qti.hardware.radio.ims.V1_0.ServiceStatusInfo();
        to.hasIsValid = from.hasIsValid;
        to.isValid = from.isValid;
        to.type = from.type;
        to.callType = from.callType;
        to.status = from.status;
        to.userdata = from.userdata;
        to.restrictCause = from.restrictCause;
        for (StatusForAccessTech status : from.accTechStatus) {
            to.accTechStatus.add(migrateAccTechStatusFromV16(status));
        }
        to.rttMode = from.rttMode;
        return to;
    }

    @VisibleForTesting
    public static vendor.qti.hardware.radio.ims.V1_0.StatusForAccessTech
            migrateAccTechStatusFromV16(StatusForAccessTech from) {
        vendor.qti.hardware.radio.ims.V1_0.StatusForAccessTech to =
                new vendor.qti.hardware.radio.ims.V1_0.StatusForAccessTech();
        to.networkMode = migrateRadioTechTypeFromV16(from.networkMode);
        to.status = from.status;
        to.restrictCause = from.restrictCause;
        to.hasRegistration = from.hasRegistration;
        to.registration = migrateRegistrationInfo(from.registration);
        return to;
    }

    @VisibleForTesting
    public static vendor.qti.hardware.radio.ims.V1_0.RegistrationInfo
            migrateRegistrationInfo(RegistrationInfo from) {
         vendor.qti.hardware.radio.ims.V1_0.RegistrationInfo to =
                 new  vendor.qti.hardware.radio.ims.V1_0.RegistrationInfo();
         to.state = from.state;
         to.errorCode = from.errorCode;
         to.errorMessage = from.errorMessage;
         to.radioTech = migrateRadioTechTypeFromV16(from.radioTech);
         to.pAssociatedUris = from.pAssociatedUris;
         return to;
    }

    private static int migrateRadioTechTypeFromV16(int from) {
        return (from == RadioTechType.RADIO_TECH_NR5G) ? RadioTechType.RADIO_TECH_LTE : from;
    }
}
