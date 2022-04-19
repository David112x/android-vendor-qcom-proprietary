/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

public final class BlockStatusInfo {

    //enum BlockReasonType
    public static final int REASON_INVALID = -1;
    public static final int REASON_PDP_FAILURE = 1;
    public static final int REASON_REGISTRATION_FAILURE = 2;
    public static final int REASON_HANDOVER_FAILURE = 3;
    public static final int REASON_OTHER_FAILURE = 4;

    private int mReason;
    private BlockReasonDetailsInfo mReasonDetails;

    public BlockStatusInfo() {
        this(REASON_INVALID, null);
    }

    public BlockStatusInfo(int reason, BlockReasonDetailsInfo info) {
        mReason = reason;
        mReasonDetails = info;
    }

    public void setReason(int value) {
        mReason = value;
    }

    public int getReason() {
        return mReason;
    }

     public void setReasonDetails(BlockReasonDetailsInfo value) {
        mReasonDetails = value;
    }

    public BlockReasonDetailsInfo getReasonDetails() {
        return mReasonDetails;
    }

    public String toString() {
        return ("BlockStatusInfo blockReason: " + mReason);
    }
}
