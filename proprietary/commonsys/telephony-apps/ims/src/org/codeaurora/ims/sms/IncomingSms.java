/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims.sms;

public class IncomingSms {

    private final String mFormat;
    private final byte[] mPdu;
    private final int mVerstat;

    public IncomingSms(String format, byte[] pdu, int verstat){
        mFormat = format;
        mPdu = pdu;
        mVerstat = verstat;
    }

    public String getFormat(){
        return mFormat;
    }

    public byte[] getPdu(){
        return mPdu;
    }

    public int getVerstat(){
        return mVerstat;
    }
    @Override
    public String toString() {
        //Don't log pdu for privacy.
        return "{ mFormat = " + mFormat + " verstat = " + mVerstat + "}";
    }
}


