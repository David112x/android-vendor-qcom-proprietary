/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

public final class BlockReasonDetailsInfo {

    //enum RegFailureReasonType
    public static final int REG_FAILURE_REASON_TYPE_INVALID = -1;
    public static final int REG_FAILURE_REASON_TYPE_UNSPECIFIED = 1;
    public static final int REG_FAILURE_REASON_TYPE_MOBILE_IP = 2;
    public static final int REG_FAILURE_REASON_TYPE_INTERNAL = 3;
    public static final int REG_FAILURE_REASON_TYPE_CALL_MANAGER_DEFINED = 4;
    public static final int REG_FAILURE_REASON_TYPE_3GPP_SPEC_DEFINED = 5;
    public static final int REG_FAILURE_REASON_TYPE_PPP = 6;
    public static final int REG_FAILURE_REASON_TYPE_EHRPD = 7;
    public static final int REG_FAILURE_REASON_TYPE_IPV6 = 8;
    public static final int REG_FAILURE_REASON_TYPE_IWLAN = 9;
    public static final int REG_FAILURE_REASON_TYPE_HANDOFF = 10;

    private int mRegFailureReasonType;
    private int mRegFailureReason;

    public BlockReasonDetailsInfo() {
        this(REG_FAILURE_REASON_TYPE_INVALID, Integer.MAX_VALUE);
    }

    public BlockReasonDetailsInfo(int reasonType, int reason) {
        mRegFailureReasonType = reasonType;
        mRegFailureReason = reason;
    }

    public void setRegFailureReasonType(int value) {
        mRegFailureReasonType = value;
    }

    public int getRegFailureReasonType() {
        return mRegFailureReasonType;
    }

    public void setRegFailureReason(int value) {
        mRegFailureReason = value;
    }

    public int getRegFailureReason() {
        return mRegFailureReason;
    }

    public String toString() {
        return ("BlockReasonDetailsInfo regFailureReasonType: " +
                 mRegFailureReasonType + " regFailureReason: " +
                 mRegFailureReason);
    }
}
