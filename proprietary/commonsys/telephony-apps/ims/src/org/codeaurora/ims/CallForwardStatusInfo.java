/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

import java.util.Arrays;

import android.telephony.ims.ImsReasonInfo;

public class CallForwardStatusInfo {

    //Overall CF status
    private ImsReasonInfo sipErrorInfo = null;
    //Individual CF status used for CF_ALL and CF_ALL_CONDITIONAL
    private CallForwardStatus[] callForwardStatus = null;

    public CallForwardStatusInfo(ImsReasonInfo sipError, CallForwardStatus[] cfStatus) {
        sipErrorInfo = sipError;
        callForwardStatus = cfStatus;
    }

    public CallForwardStatusInfo() {
    }

    public String toString() {
        return " { CallForwardStatusInfo :: sipErrorInfo:" + sipErrorInfo + " Individual status: " +
                Arrays.toString(callForwardStatus) + "}";
    }

    public ImsReasonInfo getSipErrorInfo() {
        return sipErrorInfo;
    }

    public CallForwardStatus[] getCallForwardStatus() {
        return callForwardStatus;
    }
}
