/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva.data;

public interface IUserModel {
    int INVALID_ID = -1;

    String getUserName();

    void setUserName(String userName);

    int getUserId();

    void setUserId(int userId);
}
