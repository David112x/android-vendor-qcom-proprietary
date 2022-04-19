/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

import android.telephony.emergency.EmergencyNumber;
import android.telephony.emergency.EmergencyNumber.EmergencyCallRouting;
import android.telephony.emergency.EmergencyNumber.EmergencyServiceCategories;

import java.util.ArrayList;
import java.util.List;

/**
 * This class stores information related to emergency call like service
 * category(s), call routing, etc.
 */
public final class EmergencyCallInfo {

    private @EmergencyServiceCategories int mEmergencyServiceCategories;
    private List<String> mEmergencyUrns;
    private @EmergencyCallRouting int mEmergencyCallRouting;
    private boolean mEmergencyCallTesting;
    private boolean mHasKnownUserIntentEmergency;

    public EmergencyCallInfo() {
        this(EmergencyNumber.EMERGENCY_SERVICE_CATEGORY_UNSPECIFIED,
             new ArrayList<>(),
             EmergencyNumber.EMERGENCY_CALL_ROUTING_UNKNOWN,
             false, false);
    }

    public EmergencyCallInfo(@EmergencyServiceCategories int categories,
                List<String> urns,
                @EmergencyCallRouting int routing,
                boolean isTesting,
                boolean hasIntentEmergency) {
        mEmergencyServiceCategories = categories;
        mEmergencyUrns = urns;
        mEmergencyCallRouting = routing;
        mEmergencyCallTesting = isTesting;
        mHasKnownUserIntentEmergency = hasIntentEmergency;
    }

    public void setEmergencyServiceCategories(
            @EmergencyServiceCategories int categories) {
        mEmergencyServiceCategories = categories;
    }

    public void setEmergencyUrns(List<String> urns) {
        mEmergencyUrns = urns;
    }

    public void setEmergencyCallRouting(
            @EmergencyCallRouting int routing) {
        mEmergencyCallRouting = routing;
    }

    public void setEmergencyCallTesting(boolean isTesting) {
        mEmergencyCallTesting = isTesting;
    }

    public void setHasKnownUserIntentEmergency(boolean hasIntentEmergency) {
        mHasKnownUserIntentEmergency = hasIntentEmergency;
    }

    public @EmergencyServiceCategories int getEmergencyServiceCategories() {
        return mEmergencyServiceCategories;
    }

    public List<String> getEmergencyUrns() {
        return mEmergencyUrns;
    }

    public @EmergencyCallRouting int getEmergencyCallRouting() {
        return mEmergencyCallRouting;
    }

    public boolean isEmergencyCallTesting() {
        return mEmergencyCallTesting;
    }

    public boolean hasKnownUserIntentEmergency() {
        return mHasKnownUserIntentEmergency;
    }

    public String toString() {
        return ("EmergencyCallInfo mEmergencyServiceCategories "
                + mEmergencyServiceCategories
                + " mEmergencyUrns " + mEmergencyUrns
                + " mEmergencyCallRouting " + mEmergencyCallRouting
                + " mEmergencyCallTesting " + mEmergencyCallTesting
                + " mHasKnownUserIntentEmergency " + mHasKnownUserIntentEmergency);
    }
}
