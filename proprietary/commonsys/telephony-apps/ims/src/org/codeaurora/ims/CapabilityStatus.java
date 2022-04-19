/* Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package org.codeaurora.ims;

import com.android.ims.ImsConfig.FeatureValueConstants;
import android.telephony.ims.stub.ImsRegistrationImplBase;

/*
 * This class holds value, capability, rat over which capability is to be enabled or
 * disabled When user performs some operation for e.g. turns ON/OFF enhanced 4g mode
 * etc. then we create List of CapabilityStatus that is sent to lower layers as payload
 * with one single request.
 */

public class CapabilityStatus {
    /* Holds capability that is to be either enabled or disabled.
     * Refer {@link android.telephony.ims.feature.MmTelFeature.MmTelCapabilities}
     */
    private int mCapability;
    /* Holds RAT on which capability is to enabled or disabled.
     * Refer {@link android.telephony.ims.stub.ImsRegistrationImplBase}
     * for possible RAT values
     */
    private int mRadioTech;
    /* Holds whether a capability is getting enabled or disabled.
     * Refer {@link com.android.ims.ImsConfig.FeatureValueConstants}
     */
    private int mStatus;
    private static final int INVALID_CAPABILITY = -1;

    public CapabilityStatus(int capability, int radioTech, int status) {
        mCapability = capability;
        mRadioTech = radioTech;
        mStatus = status;
    }

    // returns rat over which the capability is to be enabled or disabled
    public int getRadioTech() {
        return mRadioTech;
    }

    // returns capability that is to be either enabled or disabled
    public int getCapability() {
        return mCapability;
    }

    // returns whether capability is enabled or disabled
    public int getStatus() {
        return mStatus;
    }

    /**
     * @return string representation.
     */
    @Override
    public String toString() {
        return (" " + mCapability
                + " " + mRadioTech
                + " " + mStatus);
    }
}
