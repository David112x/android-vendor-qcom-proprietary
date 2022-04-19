/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 **/

package org.codeaurora.ims;

import java.util.ArrayList;

public class VirtualLineInfo {
    public String mMsisdn;
    public ArrayList<String> mVirtualInfo;

    public VirtualLineInfo(String msisdn, ArrayList<String> info) {
        mMsisdn = msisdn;
        mVirtualInfo = info;
    }

    public String toString() {
        return ("{VirtualLineInfo : " + "msisdn = " + mMsisdn + " , " +
                "p-associated URIs = " + mVirtualInfo + "}");
    }
}
