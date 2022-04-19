/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva.data;


import com.qualcomm.qti.sva.controller.ExtendedSmMgr;
import com.qualcomm.qti.sva.utils.LogUtils;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

public class SmModel implements ISmModel {
    public static final int NUM_OF_MAX_USERS = 3;
    public final String HEY_SNAPDRAGON = "Hey Snapdragon";
    public final String NI_HAO_XIAO_LONG = "ni hao xiao long";
    public final String HEY_SNAPDRAGON_3_0 = "HeySnapdragon_flt10p100";

    private final String TAG = SmModel.class.getSimpleName();

    //current support SVA_SOUND_MODEL_TYPE(1) and UNKNOWN_SOUND_MODEL_TYPE (0)
    private int mSmType;
    private MODEL_VERSION mSmVersion = MODEL_VERSION.VERSION_2_0;
    private String mSmFullFileName;
    private String mSmPrettyFileName;
    private String mSmSuffix;
    private String mSmPrettyKeyphrase;
    private List<IKeyphraseModel> mSmKeyphraseList = new ArrayList<>();
    private UUID mSmUUID;

    public SmModel(String smFullFileName) {
        mSmFullFileName = smFullFileName;
        mSmPrettyFileName = smFullFileName.substring(0, smFullFileName.indexOf("."));
        mSmSuffix = smFullFileName.substring(smFullFileName.lastIndexOf('.'));
        mSmUUID = UUID.randomUUID();
        LogUtils.d(TAG, "SmModel: mSmFullFileName = " + mSmFullFileName
                + " mSmSuffix = " + mSmSuffix + " mSmUUID = " + mSmUUID);
    }

    @Override
    public UUID getSoundModelUUID() {
        return mSmUUID;
    }

    @Override
    public MODEL_VERSION getSoundModelVersion() {
        return mSmVersion;
    }

    @Override
    public void setSoundModelVersion(MODEL_VERSION version) {
        mSmVersion = version;
    }

    public int getSoundModelType() {
        return mSmType;
    }

    @Override
    public void setSoundModelType(int smType) {
        mSmType = smType;
    }

    @Override
    public String getSoundModelFullFileName() {
        return mSmFullFileName;
    }

    @Override
    public void setSoundModelFullFileName(String fullFileName) {
        mSmFullFileName = fullFileName;
    }

    @Override
    public String getSoundModelPrettyFileName() {
        return mSmPrettyFileName;
    }

    @Override
    public void setSoundModelPrettyFileName(String prettyFileName) {
        mSmPrettyFileName = prettyFileName;
    }

    @Override
    public String getSoundModelSuffix() {
        return mSmSuffix;
    }

    @Override
    public void setSoundModelSuffix(String suffix) {
        mSmSuffix = suffix;
    }

    @Override
    public String getSoundModelPrettyKeyphrase() {
        return mSmPrettyKeyphrase;
    }

    @Override
    public void setSoundModelPrettyKeyphrase(String prettyKeyphrase) {
        mSmPrettyKeyphrase = prettyKeyphrase;
    }

    @Override
    public IKeyphraseModel getKeyphraseModel(String keyphrase) {
        LogUtils.d(TAG, "getKeyphraseModel: keyphrase = " + keyphrase);
        if (null == keyphrase) {
            LogUtils.d(TAG, "getKeyphraseModel: invalid input param");
            return null;
        }

        IKeyphraseModel keyphraseModel = null;
        for (IKeyphraseModel item : mSmKeyphraseList) {
            if (item.getKeyphraseFullName().equalsIgnoreCase(keyphrase)) {
                keyphraseModel = item;
                break;
            }
        }
        return keyphraseModel;
    }

    @Override
    public List<IKeyphraseModel> getSoundModelKeyphraseList() {
        return mSmKeyphraseList;
    }

    @Override
    public ArrayList<IUserModel> getAllUsers() {
        ArrayList<IUserModel> users = new ArrayList<>();
        for (IKeyphraseModel item : mSmKeyphraseList) {
            users.addAll(item.getUserList());
        }
        return users;
    }

    @Override
    public boolean isUserExistForKeyphrase(String keyphrase, String userName) {
        boolean bExist = false;
        IKeyphraseModel keyphraseModel = getKeyphraseModel(keyphrase);
        if (null != keyphraseModel) {
            IUserModel userModel = keyphraseModel.getUserModel(userName);
            if (null != userModel) {
                bExist = true;
            }
        }

        LogUtils.d(TAG, "isUserExistForKeyphrase: keyphrase = " + keyphrase
                + " userName = " + userName + " bExist = " + bExist);

        return bExist;
    }

    @Override
    public void addKeyphrase(String keyphrase, int keyphraseId) {
        LogUtils.d(TAG, "addKeyphrase: keyphrase = " + keyphrase
                + " keyphraseId = " + keyphraseId);
        if (null == keyphrase || keyphraseId < IKeyphraseModel.INVALID_ID) {
            LogUtils.d(TAG, "addKeyphrase: invalid input param");
            return;
        }

        IKeyphraseModel keyphraseModel = getKeyphraseModel(keyphrase);
        if (null == keyphraseModel) {
            keyphraseModel = new KeyphraseModel(keyphrase);
            keyphraseModel.setKeyphraseId(keyphraseId);
            mSmKeyphraseList.add(keyphraseModel);
        } else {
            keyphraseModel.setKeyphraseId(keyphraseId);
        }
    }

    @Override
    public void addKeyphraseAndUser(String keyphrase, int keyphraseId,
                                    String userName, int userId) {
        LogUtils.d(TAG, "addKeyphraseAndUser: keyphrase = " + keyphrase
                + " keyphraseId = " + keyphraseId + " userName = " + userName
                + " userId = " + userId);
        if (null == keyphrase || keyphraseId < IKeyphraseModel.INVALID_ID
                || null == userName || userId < IUserModel.INVALID_ID) {
            LogUtils.d(TAG, "addKeyphraseAndUser: invalid input param");
            return;
        }

        IKeyphraseModel keyphraseModel = getKeyphraseModel(keyphrase);
        if (null == keyphraseModel) {
            keyphraseModel = new KeyphraseModel(keyphrase);
            keyphraseModel.setKeyphraseId(keyphraseId);
            keyphraseModel.addUser(userName, userId);
            mSmKeyphraseList.add(keyphraseModel);
        } else {
            keyphraseModel.setKeyphraseId(keyphraseId);
            keyphraseModel.addUser(userName, userId);
        }
    }

    @Override
    public void removeKeyphrase(String keyphrase) {
        LogUtils.d(TAG, "removeKeyphrase: keyphrase = " + keyphrase);
        if (null == keyphrase) {
            LogUtils.d(TAG, "removeKeyphrase: invalid input param");
            return;
        }

        IKeyphraseModel keyphraseModel = getKeyphraseModel(keyphrase);
        if (null != keyphraseModel) {
            mSmKeyphraseList.remove(keyphraseModel);
        }
    }

    @Override
    public void removeUser(String keyphrase, String userName) {
        LogUtils.d(TAG, "removeUser: keyphrase = " + keyphrase
                + " userName = " + userName);
        if (null == keyphrase || null == userName) {
            LogUtils.d(TAG, "removeUser: invalid input param");
            return;
        }

        IKeyphraseModel keyphraseModel = getKeyphraseModel(keyphrase);
        if (null == keyphraseModel) {
            LogUtils.d(TAG, "removeUser: invalid keyphrase = " + keyphrase);
            return;
        } else {
            keyphraseModel.removeUser(userName);
        }
    }

    @Override
    public boolean isUserTrainedSoundModel() {
        return mSmSuffix.endsWith(SUFFIX_TRAINED_SOUND_MODEL);
    }

    @Override
    public boolean canAddUser() {
        boolean canAddUser;
        if (!isUserTrainedSoundModel() || isUdkSm()) {
            // for pdk sm and udk sm, forbid directly add it by settings UI
            canAddUser = false;
        } else {
            // for pdk UV model
            if (mSmVersion == MODEL_VERSION.VERSION_3_0
                || mSmVersion == MODEL_VERSION.VERSION_4_0) {
                if (getAllUsers().size() > 0) {
                    canAddUser = false;
                } else {
                    canAddUser = true;
                }
            } else {
                if (getAllUsers().size() >= NUM_OF_MAX_USERS) {
                    canAddUser = false;
                } else {
                    canAddUser = true;
                }
            }
        }
        LogUtils.d(TAG, "canAddUser: canAddUser = " + canAddUser);
        return canAddUser;
    }

    @Override
    public String getKeyphraseNameById(int id) {
        String keyphraseName = null;

        for (IKeyphraseModel item : mSmKeyphraseList) {
            if (item.getKeyphraseId() == id) {
                keyphraseName = item.getKeyphraseFullName();
                break;
            }
        }

        LogUtils.d(TAG, "getKeyphraseNameById: id = " + id
                + " keyphraseName = " + keyphraseName);

        return keyphraseName;
    }

    @Override
    public boolean isUdkSm() {
        boolean isUdkSm = true;
        String keyphrase = getSoundModelPrettyKeyphrase();

        ArrayList<String> pdkphraseList = ExtendedSmMgr.getPDKkeyphraseList();
        if (pdkphraseList.isEmpty()) {
            if (HEY_SNAPDRAGON.equalsIgnoreCase(keyphrase)
                    || NI_HAO_XIAO_LONG.equalsIgnoreCase(keyphrase)
                    || HEY_SNAPDRAGON_3_0.equalsIgnoreCase(keyphrase)) {
                isUdkSm = false;
            } else {
                isUdkSm = true;
            }
        } else {
            for (String item : pdkphraseList) {
                if (null != keyphrase && keyphrase.equalsIgnoreCase(item)) {
                    isUdkSm = false;
                    break;
                }
            }
        }

        LogUtils.d(TAG, "isUdkSm: isUdkSm = " + isUdkSm);
        return isUdkSm;
    }
}
