/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

import android.telephony.ims.ImsReasonInfo;

public final class ImsRegistrationInfo {

    //enum RegState
    public static final int INVALID = -1;
    public static final int REGISTERED = 1;
    public static final int NOT_REGISTERED = 2;
    public static final int REGISTERING = 3;

    private int mState;

    /* Returned when the state is NOT_REGISTERED
     * Values:
     * - 3xx -- Redirection
     * - 4xx -- Client failure
     * - 5xx -- Server failure
     * - 6xx -- Global failure
     */
    private int mErrorCode;

    /* String associated with the error code */
    private String mErrorMessage;
    private int mRadioTech;

    /* List of semi-colon-separated URI(s)
     * for self-identity.
     */
    private String mPAssociatedUris;

    /* Reuse the registration message for IMS
     * Registration/Deregistration
     */
    public ImsRegistrationInfo() {
        mState = INVALID;
        mErrorCode = ImsReasonInfo.CODE_UNSPECIFIED;
        mErrorMessage = null;
        mRadioTech = RadioTech.RADIO_TECH_INVALID;
        mPAssociatedUris = null;
    }

    public void setState(int value) {
        mState = value;
    }

    public int getState() {
        if (mState == INVALID) {
            mState = NOT_REGISTERED;
        }
        return mState;
    }

    public void setErrorCode(int value) {
        mErrorCode = value;
    }

    public int getErrorCode() {
        return mErrorCode;
    }

    public void setErrorMessage(String value) {
        mErrorMessage = value;
    }

    public String getErrorMessage() {
        return mErrorMessage;
    }

    public void setRadioTech(int value) {
        mRadioTech = value;
    }

    public int getRadioTech() {
        return mRadioTech;
    }

    public void setPAssociatedUris(String value) {
        mPAssociatedUris = value;
    }

    public String getPAssociatedUris() {
        return mPAssociatedUris;
    }

    public String toString() {
        return ("ImsRegistrationInfo state: " + mState
                 + " errorCode: " + mErrorCode
                 + " errorMessage: " + mErrorMessage
                 + " radioTech: " + mRadioTech
                 + " pAssociatedUris" + mPAssociatedUris);
    }
}
