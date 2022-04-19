/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package org.codeaurora.ims.sms;

public class SmsResponse {

    private final int mMessageRef;
    private final int mSendSmsResult;
    private final int mSendSmsReason;

    public SmsResponse(int msgRef, int result, int reason){
        mMessageRef = msgRef;
        mSendSmsResult = result;
        mSendSmsReason = reason;
    }

    public int getMsgRef(){
        return mMessageRef;
    }

    public int getResult(){
        return mSendSmsResult;
    }

    public int getReason(){
        return mSendSmsReason;
    }

    @Override
    public String toString() {
        return "{ mMessageRef = " + mMessageRef
                + ", mSendSmsResult = " + mSendSmsResult
                + ", mSendSmsReason = " + mSendSmsReason
                + "}";
    }
}
