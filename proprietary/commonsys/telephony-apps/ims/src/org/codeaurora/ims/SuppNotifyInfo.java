/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

public class SuppNotifyInfo {

    //enum NotificationType
    public static final int MO = 0;

    public static final int MT = 1;

    /*
     * 0 = MO intermediate result code
     * 1 = MT unsolicited result code
     */
    private int mNotificationType;
    private int mCode;

    /* CUG index - not used currently */
    private int mIndex;

    /* "type" (MT only).- not used currently */
    private int mType;

    /* "number"
     * (MT only, may be NULL). - not used currently
     */
    private String mNumber;

    /* connection id */
    private int mConnId;

    /* Received for MO scenarios.
     * Contains list of forwarded numbers when call forward
     * is set on MT side. The numbers in the list are
     * separated with "\n\r" characters.
     */
    private String mHistoryInfo;

    /* Local hold tone information.
     * Valid values:
     *   true  - UE to play hold tone
     *   false - Network to play hold tone
     */
    private boolean mHoldTone;
    private boolean hasHoldTone =  false;

    public SuppNotifyInfo() {
        mNotificationType = MO;
        mCode = 0;
        mIndex = 0;
        mType = 0;
        mNumber = "";
        mConnId = 0;
        mHistoryInfo = "";
        mHoldTone = false;
    }

    public void setNotificationType(int value) {
        mNotificationType = value;
    }

    public int getNotificationType() {
        return mNotificationType;
    }

    public void setCode(int value) {
        mCode = value;
    }

    public int getCode() {
        return mCode;
    }

    public void setIndex(int value) {
        mIndex = value;
    }

    public int getIndex() {
        return mIndex;
    }

    public void setType(int value) {
        mType = value;
    }

    public int getType() {
        return mType;
    }

    public void setNumber(String value) {
        mNumber = value;
    }

    public String getNumber() {
        return mNumber;
    }

    public void setConnId(int value) {
        mConnId = value;
    }

    public int getConnId() {
        return mConnId;
    }

    public void setHistoryInfo(String value) {
        mHistoryInfo = value;
    }

    public String getHistoryInfo() {
        return mHistoryInfo;
    }

    public void setHoldTone(boolean value) {
        mHoldTone = value;
        hasHoldTone = true;
    }

    public boolean getHoldTone() {
        return mHoldTone;
    }

    public boolean hasHoldTone() {
        return hasHoldTone;
    }

    public String toString() {
        return ("SuppNotifyInfo notificationType: " +
                 mNotificationType + " code: " + mCode +
                 " index: " + mIndex + " type: " +
                 mType + " number: " + mNumber +
                 " connId: " + mConnId + " historyInfo: " +
                 mHistoryInfo + " holdTone: " + mHoldTone);
    }
}
