/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva.controller;

public class MsgResponse {
    private Result mResult;
    private String mSmFullName;
    private int mErrorCode;

    public MsgResponse(Result result, String smFullName, int errorCode) {
        mResult = result;
        mSmFullName = smFullName;
        mErrorCode = errorCode;
    }

    public Result getResult() {
        return mResult;
    }

    public String getSmFullName() {
        return mSmFullName;
    }

    public int getErrorCode() {
        return mErrorCode;
    }

    @Override
    public String toString() {
        return "MsgResponse{" +
                "mResult=" + mResult +
                ", mSmFullName='" + mSmFullName + '\'' +
                '}';
    }

    public enum Result {
        SUCCESS,
        FAILURE
    }
}
