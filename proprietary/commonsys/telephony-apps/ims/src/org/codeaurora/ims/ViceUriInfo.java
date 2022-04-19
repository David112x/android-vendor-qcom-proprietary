/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

import java.util.ArrayList;

public final class ViceUriInfo {

    private final byte[] mViceInfoUri;

    public ViceUriInfo() {
        mViceInfoUri = null;
    }

    public ViceUriInfo(ArrayList<Byte> uri) {
        mViceInfoUri = ImsRadioUtils.getByteArray(uri);
    }

    public byte[] getViceInfoUri() {
        return mViceInfoUri;
    }
}
