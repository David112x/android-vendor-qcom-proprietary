/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

public class HoInfo {

    // enum HandoverState
    /* Handover not valid */
    public static final int INVALID = -1;

    /* Handover triggered */
    public static final int START = 0;

    /* Handover completed successfully */
    public static final int COMPLETE_SUCCESS = 1;

    /* Handover failed */
    public static final int COMPLETE_FAIL = 2;

    /* Handover canceled */
    public static final int CANCEL = 3;

    /* Handover not triggered */
    public static final int NOT_TRIGGERED = 4;

    /* Handover not triggered since mobile
     * data is turned off */
    public static final int NOT_TRIGGERED_MOBILE_DATA_OFF = 5;

    private int mType;
    private int mSrcTech;
    private int mTargetTech;
    private int mExtraType;
    private byte[] mExtraInfo;
    private String mErrorCode;
    private String mErrorMessage;

    public HoInfo() {
        mType = INVALID;
        mSrcTech = RadioTech.RADIO_TECH_INVALID;
        mTargetTech = RadioTech.RADIO_TECH_INVALID;
        mExtraType = ExtraInfo.INVALID;
        mExtraInfo = null;
        mErrorCode = "";
        mErrorMessage = "";
    }

    public void setType(int value){
        mType = value;
    }

    public int getType(){
        return mType;
    }

    public void setSrcTech(int value){
        mSrcTech = value;
    }

    public int getSrcTech() {
        return mSrcTech;
    }

    public void setTargetTech(int value){
        mTargetTech = value;
    }

    public int getTargetTech() {
        return mTargetTech;
    }

    public void setExtraType(int value){
        mExtraType = value;
    }

    public int getExtraType() {
        return mExtraType;
    }

    public void setExtraInfo(byte[] value){
        mExtraInfo = value;
    }

    public byte[] getExtraInfo() {
        return mExtraInfo;
    }

    public void setExtra(int type, byte[] info) {
        setExtraType(type);
        setExtraInfo(info);
    }

    public void setErrorCode(String value){
        mErrorCode = value;
    }

    public String getErrorCode() {
        return mErrorCode;
    }

    public void setErrorMessage(String value){
        mErrorMessage = value;
    }

    public String getErrorMessage() {
        return mErrorMessage;
    }
}
