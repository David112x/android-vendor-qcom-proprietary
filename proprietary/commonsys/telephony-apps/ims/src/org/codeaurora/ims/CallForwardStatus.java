/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

import android.telephony.ims.ImsReasonInfo;

public class CallForwardStatus {

    private int reason; /* from TS 27.007 7.11 "reason" */
    private int status; /* 0 - SUCCESS, 1 - FAIL */
    private ImsReasonInfo sipErrorInfo = null;
    private final int INVALID = -1;
    private final int FAILED = 1;

    public CallForwardStatus(int reason, int status, ImsReasonInfo sipError) {
        this.reason = reason;
        this.status = status;
        this.sipErrorInfo = sipError;
    }

    public CallForwardStatus() {
        reason = INVALID;
        status = FAILED;
        sipErrorInfo = null;
    }

    public String toString() {
        return "{ CallForwardStatus : reason: " + reason + " status: " + status +
                " ImsReasonInfo : " + sipErrorInfo + " }";
    }

    public int getReason() {
        return reason;
    }

    public int getStatus() {
        return status;
    }

    public ImsReasonInfo getSipErrorInfo() {
        return sipErrorInfo;
    }
}
