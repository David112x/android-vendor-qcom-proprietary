/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

import android.telephony.ims.ImsReasonInfo;

public class SuppService {

    // enum ClipStatus or ServiceClassProvisionStatus
    public static final int NOT_PROVISIONED = 0;
    public static final int PROVISIONED = 1;
    public static final int STATUS_UNKNOWN = 2;

    // enum IpPresentation
    public static final int IP_PRESENTATION_NUM_ALLOWED = 0;
    public static final int IP_PRESENTATION_NUM_RESTRICTED = 1;
    public static final int IP_PRESENTATION_NUM_DEFAULT = 2;

    private int mServiceClassStatus;
    private int mProvisionStatus;
    private int mStatus;
    private int mPresentation;
    private ImsReasonInfo mErrorDetails;

    public SuppService() {
        this(SuppSvcResponse.INVALID, STATUS_UNKNOWN, 0, IP_PRESENTATION_NUM_DEFAULT, null);
    }

    public SuppService(int serviceClassStatus, int provisionStatus,
        int status, int presentation, ImsReasonInfo errorInfo) {
        mServiceClassStatus = serviceClassStatus;
        mProvisionStatus = provisionStatus;
        mStatus = status;
        mPresentation = presentation;
        mErrorDetails = errorInfo;
    }

    public void setServiceClassStatus(int status) {
        mServiceClassStatus = status;
    }

    public int getServiceClassStatus() {
        return mServiceClassStatus;
    }

    public void setProvisionStatus(int status) {
        mProvisionStatus = status;
    }

    public int getProvisionStatus() {
        return mProvisionStatus;
    }

    public void setStatus(int status) {
        mStatus = status;
    }

    public int getStatus() {
        return mStatus;
    }

    public void setPresentation(int presentation) {
        mPresentation = presentation;
    }

    public int getPresentation() {
        return mPresentation;
    }

    public void setErrorDetails(ImsReasonInfo errorInfo) {
        mErrorDetails = errorInfo;
    }

    public ImsReasonInfo getErrorDetails() {
        return mErrorDetails;
    }

    public String toString() {
        return ("SuppService status: " + mStatus + ((mErrorDetails != null) ?
                " errorCode: " + mErrorDetails.getExtraCode() +
                " errorMessage: " + mErrorDetails.getExtraMessage() : ""));
    }
}
