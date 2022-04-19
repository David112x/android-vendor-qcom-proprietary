/*********************************************************************
 Copyright (c) 2017 Qualcomm Technologies, Inc.
 All Rights Reserved.
 Confidential and Proprietary - Qualcomm Technologies, Inc.
**********************************************************************/

package com.qualcomm.qti.translator;


import com.qualcomm.qti.uceservice.V2_0.PresenceCapabilityInfo;
import com.qualcomm.qti.uceservice.V2_0.PresServiceInfo;
import com.qualcomm.qti.uceservice.V2_0.OptionsCapabilityInfo;
import com.qualcomm.qti.uceservice.V2_0.CapabilityInfo;

import com.android.ims.internal.uce.common.CapInfo;
import com.android.ims.internal.uce.presence.PresCapInfo;
import com.android.ims.internal.uce.options.OptionsCapInfo;

import android.util.Log;

import java.util.Arrays;


public class AidlToHidlTranslator
{
  private final static String logTAG = "AIDL to HIDL Translator";

/* Translator methods pertaining to PresService class */

public PresenceCapabilityInfo getHidlPresCapInfo(PresCapInfo  aidlPresCapInfo) {
    PresenceCapabilityInfo hidlPresCapInfo = new PresenceCapabilityInfo();

    hidlPresCapInfo.capInfo.imSupported = aidlPresCapInfo.getCapInfo().isImSupported();
    hidlPresCapInfo.capInfo.ftSupported = aidlPresCapInfo.getCapInfo().isFtSupported();
    hidlPresCapInfo.capInfo.ftThumbSupported = aidlPresCapInfo.getCapInfo().isFtThumbSupported();
    hidlPresCapInfo.capInfo.ftSnFSupported = aidlPresCapInfo.getCapInfo().isFtSnFSupported();
    hidlPresCapInfo.capInfo.ftHttpSupported = aidlPresCapInfo.getCapInfo().isFtHttpSupported();
    hidlPresCapInfo.capInfo.imageShareSupported = aidlPresCapInfo.getCapInfo().isIsSupported();
    hidlPresCapInfo.capInfo.videoShareDuringCSSupported = aidlPresCapInfo.getCapInfo().isVsDuringCSSupported();
    hidlPresCapInfo.capInfo.videoShareSupported = aidlPresCapInfo.getCapInfo().isVsSupported();
    hidlPresCapInfo.capInfo.socialPresenceSupported = aidlPresCapInfo.getCapInfo().isSpSupported();
    hidlPresCapInfo.capInfo.capDiscViaPresenceSupported = aidlPresCapInfo.getCapInfo().isCdViaPresenceSupported();
    hidlPresCapInfo.capInfo.ipVoiceSupported = aidlPresCapInfo.getCapInfo().isIpVoiceSupported();
    hidlPresCapInfo.capInfo.ipVideoSupported = aidlPresCapInfo.getCapInfo().isIpVideoSupported();
    hidlPresCapInfo.capInfo.geoPullFtSupported = aidlPresCapInfo.getCapInfo().isGeoPullFtSupported();
    hidlPresCapInfo.capInfo.geoPullSupported = aidlPresCapInfo.getCapInfo().isGeoPullSupported();
    hidlPresCapInfo.capInfo.geoPushSupported = aidlPresCapInfo.getCapInfo().isGeoPushSupported();
    hidlPresCapInfo.capInfo.smSupported = aidlPresCapInfo.getCapInfo().isSmSupported();
    hidlPresCapInfo.capInfo.fullSnFGroupChatSupported = aidlPresCapInfo.getCapInfo().isFullSnFGroupChatSupported();
    hidlPresCapInfo.capInfo.rcsIpVoiceCallSupported = aidlPresCapInfo.getCapInfo().isRcsIpVoiceCallSupported();
    hidlPresCapInfo.capInfo.rcsIpVideoCallSupported = aidlPresCapInfo.getCapInfo().isRcsIpVideoCallSupported();
    hidlPresCapInfo.capInfo.rcsIpVideoOnlyCallSupported = aidlPresCapInfo.getCapInfo().isRcsIpVideoOnlyCallSupported();
    //hidlPresCapInfo.capInfo.mExts.addAll(Arrays.asList(aidlPresCapInfo.getCapInfo().getExts()));
    hidlPresCapInfo.capInfo.capTimestamp = aidlPresCapInfo.getCapInfo().getCapTimestamp();
    hidlPresCapInfo.contactUri = "";
    if(aidlPresCapInfo.getContactUri() !=  null){
       hidlPresCapInfo.contactUri = aidlPresCapInfo.getContactUri();
    }

    return hidlPresCapInfo;
}

public PresServiceInfo getHidlPresSvcInfo(com.android.ims.internal.uce.presence.PresServiceInfo aidlPresServiceInfo) {
    PresServiceInfo hidlPresSvcInfo = new PresServiceInfo();

    hidlPresSvcInfo.serviceId = aidlPresServiceInfo.getServiceId();
    hidlPresSvcInfo.serviceDesc = aidlPresServiceInfo.getServiceDesc();
    hidlPresSvcInfo.serviceVer = aidlPresServiceInfo.getServiceVer();
    hidlPresSvcInfo.mediaCap = aidlPresServiceInfo.getMediaType();

    return hidlPresSvcInfo;
}

/* Translator methods pertaining to OptionsService class */

public OptionsCapabilityInfo getHidlOptCapInfo(OptionsCapInfo aidlOptCapInfo) {
    OptionsCapabilityInfo hidlOptCapInfo = new OptionsCapabilityInfo();

    getHidlCapInfo(hidlOptCapInfo.capInfo, aidlOptCapInfo.getCapInfo());
    hidlOptCapInfo.sdp = new String();
    hidlOptCapInfo.sdp = aidlOptCapInfo.getSdp();

    return hidlOptCapInfo;
}

public void getHidlCapInfo(CapabilityInfo hidlCapInfo, CapInfo aidlCapInfo) {

    hidlCapInfo.imSupported = aidlCapInfo.isImSupported();
    hidlCapInfo.ftSupported = aidlCapInfo.isFtSupported();
    hidlCapInfo.ftThumbSupported = aidlCapInfo.isFtThumbSupported();
    hidlCapInfo.ftSnFSupported = aidlCapInfo.isFtSnFSupported();
    hidlCapInfo.ftHttpSupported = aidlCapInfo.isFtHttpSupported();
    hidlCapInfo.imageShareSupported = aidlCapInfo.isIsSupported();
    hidlCapInfo.videoShareDuringCSSupported = aidlCapInfo.isVsDuringCSSupported();
    hidlCapInfo.videoShareSupported = aidlCapInfo.isVsSupported();
    hidlCapInfo.socialPresenceSupported = aidlCapInfo.isSpSupported();
    hidlCapInfo.capDiscViaPresenceSupported = aidlCapInfo.isCdViaPresenceSupported();
    hidlCapInfo.ipVoiceSupported = aidlCapInfo.isIpVoiceSupported();
    hidlCapInfo.ipVideoSupported = aidlCapInfo.isIpVideoSupported();
    hidlCapInfo.geoPullFtSupported = aidlCapInfo.isGeoPullFtSupported();
    hidlCapInfo.geoPullSupported = aidlCapInfo.isGeoPullSupported();
    hidlCapInfo.geoPushSupported = aidlCapInfo.isGeoPushSupported();
    hidlCapInfo.smSupported = aidlCapInfo.isSmSupported();
    hidlCapInfo.fullSnFGroupChatSupported = aidlCapInfo.isFullSnFGroupChatSupported();
    hidlCapInfo.rcsIpVoiceCallSupported = aidlCapInfo.isRcsIpVoiceCallSupported();
    hidlCapInfo.rcsIpVideoCallSupported = aidlCapInfo.isRcsIpVideoCallSupported();
    hidlCapInfo.rcsIpVideoOnlyCallSupported = aidlCapInfo.isRcsIpVideoOnlyCallSupported();
    //hidlCapInfo.mExts.addAll(Arrays.asList(aidlCapInfo.getExts()));
    hidlCapInfo.capTimestamp = aidlCapInfo.getCapTimestamp();
}

}
