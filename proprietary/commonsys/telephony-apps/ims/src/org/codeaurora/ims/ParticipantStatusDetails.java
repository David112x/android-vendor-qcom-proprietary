/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

public final class ParticipantStatusDetails {

    public static final int INVALID = -1;
    public static final int REMOVE = 0;
    public static final int ADD = 1;

    /* Call id */
    private int mCallId;

    /* Operation of conference participant */
    private int mOperation;

    /* sip error code as defined in RFC3261 */
    private int mSipStatus;

    /* Participant URI */
    private String mParticipantUri;

    /* is explicit call transfer */
    private boolean mIsExplicitTransfer;

    public ParticipantStatusDetails() {
        mCallId = Integer.MAX_VALUE;
        mOperation = INVALID;
        mSipStatus = Integer.MAX_VALUE;
        mParticipantUri = "";
        mIsExplicitTransfer = false;
    }

    public void setCallId(int id) {
        mCallId = id;
    }

    public int getCallId() {
        return mCallId;
    }

    public void setOperation(int operation) {
        mOperation = operation;
    }

    public int getOperation() {
        return mOperation;
    }

    public void setSipStatus(int status) {
        mSipStatus = status;
    }

    public int getSipStatus() {
        return mSipStatus;
    }

    public void setParticipantUri(String uri) {
        mParticipantUri = uri;
    }

    public String getParticipantUri() {
        return mParticipantUri;
    }

    public void setIsEct(boolean isTransfer) {
        mIsExplicitTransfer = isTransfer;
    }

    public boolean getIsEct() {
        return mIsExplicitTransfer;
    }

     public String toString() {
        return "ParticipantStatusDetails callId: "
               + mCallId + " operation: " + mOperation
               + " sipStatus: " + mSipStatus
               + " participantUri: " + mParticipantUri
               + " isExplicitTransfer: " + mIsExplicitTransfer;
    }
}
