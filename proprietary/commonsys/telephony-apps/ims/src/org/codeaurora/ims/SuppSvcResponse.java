/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

import android.telephony.ims.ImsReasonInfo;

import java.util.List;
import java.util.ArrayList;

public class SuppSvcResponse {

    private int mStatus;

    private int mProvisionStatus;

    /* values are of type enum SuppSvcFacilityType */
    private int mFacilityType;
    private String mFailureCause;

    /* used by FACILITY_BS_MT for query operation */
    private List<BarredLines> mBarredLines;

    /* SIP error details on failure */
    private ImsReasonInfo mErrorDetails;

    // enum SuppSvcOperationType
    public static final int ACTIVATE = 1;
    public static final int DEACTIVATE = 2;
    public static final int QUERY = 3;
    public static final int REGISTER = 4;
    public static final int ERASURE = 5;

    // enum ServiceClassStatus
    public static final int INVALID = -1;
    public static final int DISABLED = 0;
    public static final int ENABLED = 1;

    // enum SuppSvcFacilityType
    /* Calling Line Identification Presentation */
    public static final int FACILITY_CLIP = 1;

    /* Connected Line Identification Presentation */
    public static final int FACILITY_COLP = 2;

    /* Bar All Outgoing Calls */
    public static final int FACILITY_BAOC = 3;

    /* Bar All Outgoing International Calls */
    public static final int FACILITY_BAOIC = 4;

    /* Bar all Outgoing International Calls except those
     * directed to home PLMN country
     */
    public static final int FACILITY_BAOICxH = 5;

    /* Bar All Incoming Calls */
    public static final int FACILITY_BAIC = 6;

    /* Bar All Incoming Calls when Roaming outside
     * the home PLMN country
     */
    public static final int FACILITY_BAICr = 7;

    /* Bar All incoming & outgoing Calls */
    public static final int FACILITY_BA_ALL = 8;

    /* Bar All Outgoing Calls */
    public static final int FACILITY_BA_MO = 9;

    /* Bar All Incming Calls */
    public static final int FACILITY_BA_MT = 10;

    /* Bar Incoming Calls from specific DN */
    public static final int FACILITY_BS_MT = 11;

    /* Bar All Incoming Calls from Anonymous numbers*/
    public static final int FACILITY_BAICa = 12;

    public static class LineStatus {
        private int mStatus;
        private String mNumber;

        public LineStatus(int status, String lineNumber) {
            mStatus = status;
            mNumber = lineNumber;
        }

        public String getNumber() {
            return mNumber;
        }

        public int getStatus() {
            return mStatus;
        }
    }

    public static class BarredLines {
        /*  TS 27.007 service class to query
         *  1 = voice
         *  2 = data
         *  4 = fax
         *  8 = sms
         *  16 = data sync
         *  32 = data async
         *  64 = packet
         *  128 = PAD
         */
        private int mServiceClass; /* only voice class i.e 1 is supported */

        /* every element in mLines corresponds to
         * each serviceClass
         */
        private List<LineStatus> mLines;

        public BarredLines(int serviceClass) {
            mServiceClass = serviceClass;
            mLines = new ArrayList<>();
        }

        public void addLine(LineStatus line) {
            mLines.add(line);
        }

        public List<LineStatus> getLines() {
            return mLines;
        }
    }

    public SuppSvcResponse(){
        mStatus = INVALID;
        mProvisionStatus = SuppService.STATUS_UNKNOWN;
        mFacilityType = 0;
        mFailureCause = "";
        mBarredLines = new ArrayList<>();
        mErrorDetails = null;
    }

    public void setStatus(int value) {
        mStatus = value;
    }

    public int getStatus() {
        return mStatus;
    }

    public void setProvisionStatus(int status) {
        mProvisionStatus = status;
    }

    public int getProvisionStatus() {
        return mProvisionStatus;
    }

    public void setFacilityType(int value) {
        mFacilityType = value;
    }

    public int getFacilityType() {
        return mFacilityType;
    }

    public void setFailureCause(String value) {
        mFailureCause = value;
    }

    public String getFailureCause() {
        return mFailureCause;
    }

    public void addBarredLines(BarredLines lines) {
        mBarredLines.add(lines);
    }

    public List<BarredLines> getBarredLines() {
        return mBarredLines;
    }

    public void setErrorDetails(ImsReasonInfo value) {
        mErrorDetails = value;
    }

    public ImsReasonInfo getErrorDetails() {
        return mErrorDetails;
    }

    public String toString() {
        return ("SuppSvcResponse status: " + mStatus +
                " facilityType: " + mFacilityType +
                " failureCause: " + mFailureCause +
                " provisionStatus: " + mProvisionStatus);
    }
}
