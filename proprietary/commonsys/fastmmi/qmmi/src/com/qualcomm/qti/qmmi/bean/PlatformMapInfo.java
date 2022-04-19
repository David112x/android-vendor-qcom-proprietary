/*
 * Copyright (c) 2018, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.qmmi.bean;

public class PlatformMapInfo {
    private String mCaseListFile;
    private String mTestCaseFile;

    public String getCaseListFile() {
        return mCaseListFile;
    }

    public void setCaseListFile(String caseListFile) {
        mCaseListFile = caseListFile;
    }

    public String getTestCaseFile() {
        return mTestCaseFile;
    }

    public void setTestCaseFile(String testCaseFile) {
        mTestCaseFile =testCaseFile;
    }

    @Override
    public String toString() {
        return "PlatformMapInfo{" +
                "mCaseListFile='" + mCaseListFile + '\'' +
                ", mTestCaseFile='" + mTestCaseFile +
                '}';
    }
}
