/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

public class ExtraInfo {

    /* Handover is invalid */
    public static final int INVALID = -1;

    /* Handover from LTE to IWLAN failed */
    public static final int LTE_TO_IWLAN_HO_FAIL = 1;

    private final int mExtraType;
    private final byte[] mExtraInfo;

    public ExtraInfo(int type, byte[] info) {
        mExtraType = type;
        mExtraInfo = info;
    }

     public String toString() {
        return "ExtraInfo extraType: " + mExtraType;
    }
}
