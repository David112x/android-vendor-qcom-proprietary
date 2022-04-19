/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims.sms;

public class StatusReport {

    private final int mMessageRef;
    private final String mFormat;
    private final byte[] mPdu;

    public StatusReport(int messageRef, String format, byte[] pdu){
        mMessageRef = messageRef;
        mFormat = format;
        mPdu = pdu;
    }

    public int getMsgRef(){
        return mMessageRef;
    }

    public String getFormat(){
        return mFormat;
    }

    public byte[] getPdu(){
        return mPdu;
    }

    @Override
    public String toString() {
        //Don't log pdu for privacy.
        return  "{ mMessageRef = " + mMessageRef
                + ", mFormat = " + mFormat
                + "}";
    }
}
