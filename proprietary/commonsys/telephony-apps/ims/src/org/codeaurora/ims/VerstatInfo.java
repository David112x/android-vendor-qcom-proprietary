/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *  */

package org.codeaurora.ims;

public class VerstatInfo {

    public final static int VERSTAT_VERIFICATION_NONE = 0;
    public final static int VERSTAT_VERIFICATION_PASS = 1;
    public final static int VERSTAT_VERIFICATION_FAIL = 2;

    private boolean canMarkUnwantedCall;
    private int verstatVerificationStatus;

    public VerstatInfo () {
        canMarkUnwantedCall = false;
        verstatVerificationStatus = VERSTAT_VERIFICATION_NONE;
    }

    public VerstatInfo(boolean canMarkUnwantedCall,
            int verstatVerificationStatus) {
        this.canMarkUnwantedCall = canMarkUnwantedCall;
        this.verstatVerificationStatus = verstatVerificationStatus;
    }

    public int getVerstatVerificationStatus() {
        return verstatVerificationStatus;
    }

    public boolean canMarkUnwantedCall() {
        return canMarkUnwantedCall;
    }

    public String toString() {
        return ("VerstatInfo : " + "canMarkUnwantedCall = " +
                canMarkUnwantedCall + " , verstatVerificationStatus = "
                + verstatVerificationStatus);
    }
}
