/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.qcolor;

import android.hardware.display.ColorDisplayManager;

class QColorMode {
    private static final int SELF_DEFINED_COLOR_MODE_AUTOMATIC = 1;
    private String mModeName;
    private int mColorMode;

    public QColorMode(String name, int intent) {
        mModeName = name;
        if (intent == SELF_DEFINED_COLOR_MODE_AUTOMATIC) {
            mColorMode = ColorDisplayManager.COLOR_MODE_AUTOMATIC;
        } else {
            mColorMode = intent;
        }
    }

    public String getModeName() {
        return mModeName;
    }

    public int getColorMode() {
        return mColorMode;
    }

    public boolean isColorModeAvailable(int[] availableColorModes) {
        if (availableColorModes != null) {
            for (int mode : availableColorModes) {
                if (mode == mColorMode) {
                    return true;
                }
            }
        }
        return false;
    }
}
