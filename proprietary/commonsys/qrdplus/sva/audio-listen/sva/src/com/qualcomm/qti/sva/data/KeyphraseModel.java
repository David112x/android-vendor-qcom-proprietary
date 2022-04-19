/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva.data;

import com.qualcomm.qti.sva.utils.LogUtils;

import java.util.ArrayList;

public class KeyphraseModel implements IKeyphraseModel {
    private final String TAG = KeyphraseModel.class.getSimpleName();

    private String mKeyphraseFullName;
    private String mKeyphrasePrettyName;
    private int mKeyphraseId = INVALID_ID;
    private ArrayList<IUserModel> mUserList = new ArrayList<>();

    public KeyphraseModel(String keyphraseName) {
        mKeyphraseFullName = keyphraseName;
    }

    @Override
    public String getKeyphraseFullName() {
        return mKeyphraseFullName;
    }

    @Override
    public void setKeyphraseFullName(String fullName) {
        mKeyphraseFullName = fullName;
    }

    @Override
    public String getKeyphrasePrettyName() {
        return mKeyphrasePrettyName;
    }

    @Override
    public void setKeyphrasePrettyName(String prettyName) {
        mKeyphrasePrettyName = prettyName;
    }

    @Override
    public int getKeyphraseId() {
        return mKeyphraseId;
    }

    @Override
    public void setKeyphraseId(int id) {
        LogUtils.d(TAG, "setKeyphraseId: id = " + id);
        if (id < INVALID_ID) {
            LogUtils.d(TAG, "setKeyphraseId: invalid keyphrase id");
            return;
        }
        mKeyphraseId = id;
    }

    @Override
    public ArrayList<IUserModel> getUserList() {
        return mUserList;
    }

    @Override
    public IUserModel getUserModel(String userName) {
        LogUtils.d(TAG, "getUserModel: userName = " + userName);
        if (null == userName) {
            LogUtils.d(TAG, "getUserModel: invalid input param");
            return null;
        }

        IUserModel userModel = null;
        for (IUserModel item : mUserList) {
            if (item.getUserName().equalsIgnoreCase(userName)) {
                userModel = item;
                break;
            }
        }
        return userModel;
    }

    @Override
    public void addUser(String userName, int userId) {
        LogUtils.d(TAG, "addUser: userName = " + userName
                + " userId = " + userId);
        if (null == userName || userId < IUserModel.INVALID_ID) {
            LogUtils.d(TAG, "addUser: invalid input param");
            return;
        }

        IUserModel userModel = getUserModel(userName);
        if (null == userModel) {
            userModel = new UserModel(userName);
            userModel.setUserId(userId);
            mUserList.add(userModel);
        } else {
            userModel.setUserId(userId);
        }
    }

    @Override
    public void removeUser(String userName) {
        LogUtils.d(TAG, "removeUser: userName = " + userName);
        if (null == userName) {
            LogUtils.d(TAG, "removeUser: invalid input param");
            return;
        }

        // fix the ConcurrentModificationException crash
        ArrayList<IUserModel> deleteList = new ArrayList<>();
        for (IUserModel item : mUserList) {
            if (userName.equalsIgnoreCase(item.getUserName())) {
                deleteList.add(item);
            }
        }

        if (!deleteList.isEmpty()) {
            mUserList.removeAll(deleteList);
        }
    }

    @Override
    public String getUserNameById(int id) {
        String userName = null;
        for (IUserModel item : mUserList) {
            if (item.getUserId() == id) {
                userName = item.getUserName();
                break;
            }
        }
        LogUtils.d(TAG, "getUserNameById: id = " + id
                + " userName = " + userName);
        return userName;
    }
}
