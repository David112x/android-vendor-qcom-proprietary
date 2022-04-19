/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

public final class VopsInfo {

    private boolean mVopsEnabled;

    public VopsInfo() {
        mVopsEnabled = false;
    }

    public boolean isVopsEnabled() {
        return mVopsEnabled;
    }

    public String toString() {
        return "VopsInfo vopsEnabled: " + mVopsEnabled;
    }
}
