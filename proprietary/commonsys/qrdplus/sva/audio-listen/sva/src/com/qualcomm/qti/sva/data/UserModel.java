/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva.data;

import com.qualcomm.qti.sva.utils.LogUtils;

public class UserModel implements IUserModel {
    private final String TAG = UserModel.class.getSimpleName();

    private String mUserName;
    private int mUserId = INVALID_ID;

    public UserModel(String userName) {
        mUserName = userName;
    }

    @Override
    public String getUserName() {
        return mUserName;
    }

    @Override
    public void setUserName(String userName) {
        mUserName = userName;
    }

    @Override
    public int getUserId() {
        return mUserId;
    }

    @Override
    public void setUserId(int userId) {
        LogUtils.d(TAG, "setUserId: userId = " + userId);
        if (userId < INVALID_ID) {
            LogUtils.d(TAG, "setUserId: invalid userId");
            return;
        }
        mUserId = userId;
    }
}
