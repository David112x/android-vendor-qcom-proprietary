/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

import java.util.List;
import java.util.ArrayList;
import java.util.Arrays;
import java.lang.Boolean;

public class ImsSubConfigDetails {

    /* Simultaneous IMS stack static
     * support, is 1 for 7+1, 7+5 and
     * 7+7 reduced scope; is 2 for
     * 7+7 full scope
     */
    private int mSimultStackCount;

    /* index of the array corresponds to
     * slot number and value true corresponds
     * to IMS enabled.
     * ex: [true, false] - Ims enabled on
     * slot 1; disabled on slot 2
     */
    private final List<Boolean> mImsStackEnabled;

    public ImsSubConfigDetails() {
        mSimultStackCount = 0;
        mImsStackEnabled = new ArrayList<Boolean>();
    }

    public void setSimultStackCount(int count) {
        mSimultStackCount = count;
    }

    public int getSimultStackCount() {
        return mSimultStackCount;
    }

    public void addImsStackEnabled(boolean value) {
        mImsStackEnabled.add(value);
    }

    public List<Boolean> getImsStackEnabledList() {
        return mImsStackEnabled;
    }

    public String toString() {
        return "ImsSubConfigDetails simultStackCount: " +
               mSimultStackCount + " mImsStackEnabled: " +
               Arrays.toString(mImsStackEnabled.toArray());
    }
}
