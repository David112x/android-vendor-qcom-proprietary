/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva.data;

import java.util.ArrayList;

public interface IKeyphraseModel {
    int INVALID_ID = -1;

    String getKeyphraseFullName();

    void setKeyphraseFullName(String fullName);

    String getKeyphrasePrettyName();

    void setKeyphrasePrettyName(String prettyName);

    int getKeyphraseId();

    void setKeyphraseId(int id);

    ArrayList<IUserModel> getUserList();

    IUserModel getUserModel(String userName);

    void addUser(String userName, int userId);

    void removeUser(String userName);

    String getUserNameById(int id);
}
