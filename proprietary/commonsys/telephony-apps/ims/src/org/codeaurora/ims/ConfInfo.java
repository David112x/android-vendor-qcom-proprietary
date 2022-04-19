/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

import java.util.ArrayList;

public final class ConfInfo {

    // enum ConfCallState
    public static final int INVALID = -1;
    public static final int RINGING = 0;
    public static final int FOREGROUND = 1;
    public static final int BACKGROUND = 2;

    private byte[] mConfInfoUri;
    private int mConfCallState;

    public ConfInfo() {
        mConfInfoUri = null;
        mConfCallState = INVALID;
    }

    public byte[] getConfInfoUri() {
        return mConfInfoUri;
    }

    public int getConfCallState() {
        return mConfCallState;
    }

    public void setConfInfoUri(ArrayList<Byte> uri) {
       mConfInfoUri = ImsRadioUtils.getByteArray(uri);
    }

    public void setConfCallState(int state) {
        mConfCallState = state;
    }

    public String toString() {
        return ("Conf call state: " + mConfCallState + " Conf info uri: " + mConfInfoUri);
    }
}
