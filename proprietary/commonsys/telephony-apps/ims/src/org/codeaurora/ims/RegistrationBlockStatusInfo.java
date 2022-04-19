/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

public final class RegistrationBlockStatusInfo {

    private BlockStatusInfo mIsOnWwan;
    private BlockStatusInfo mIsOnWlan;

    public RegistrationBlockStatusInfo () {
         this(null, null);
    }

     public RegistrationBlockStatusInfo (BlockStatusInfo onWwan,
             BlockStatusInfo onWlan) {
         mIsOnWwan = onWwan;
         mIsOnWlan = onWlan;
    }

    public void setStatusOnWwan(BlockStatusInfo value) {
        mIsOnWwan = value;
    }

    public BlockStatusInfo getStatusOnWwan() {
        return mIsOnWwan;
    }

    public void setStatusOnWlan(BlockStatusInfo value) {
        mIsOnWlan = value;
    }

    public BlockStatusInfo getStatusOnWlan() {
        return mIsOnWlan;
    }

}
