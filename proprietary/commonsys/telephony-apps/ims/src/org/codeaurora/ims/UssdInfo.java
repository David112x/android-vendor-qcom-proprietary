/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *  Not a Contribution.
 *
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package org.codeaurora.ims;

public final class UssdInfo {

    // enum UssdModeType
    public static final int INVALID = -1;
    public static final int NOTIFY = 0;
    public static final int REQUEST = 1;
    public static final int NW_RELEASE = 2;
    public static final int LOCAL_CLIENT = 3;
    public static final int NOT_SUPPORTED = 4;
    public static final int NW_TIMEOUT = 5;

    private int mType;
    private int mErrorCode;
    private String mErrorMessage;

    public UssdInfo() {
        mType = INVALID;
        mErrorCode = INVALID;
        mErrorMessage = "";
    }

    public void setType(int type) {
        mType = type;
    }

    public void setErrorCode(int code) {
        mErrorCode = code;
    }

    public void setErrorMessage(String message) {
        mErrorMessage = message;
    }

    public int getType() {
        return mType;
    }

    public int getErrorCode() {
        return mErrorCode;
    }

    public String getErrorMessage() {
        return mErrorMessage;
    }

    public String toString() {
        return "UssdInfo type: " + mType +
               " errorCode: " + mErrorCode +
               " errorMessage: " + mErrorMessage;
    }
}
