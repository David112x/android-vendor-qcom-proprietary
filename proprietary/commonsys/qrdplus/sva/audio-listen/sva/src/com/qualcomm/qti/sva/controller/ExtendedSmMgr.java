/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva.controller;

import android.content.Context;
import android.content.Intent;

import com.qualcomm.listen.ListenSoundModel;
import com.qualcomm.listen.ListenTypes;
import com.qualcomm.qti.sva.data.ExtendedSmModel;
import com.qualcomm.qti.sva.data.IExtendedSmModel;
import com.qualcomm.qti.sva.data.IKeyphraseModel;
import com.qualcomm.qti.sva.data.ISettingsModel;
import com.qualcomm.qti.sva.data.ISmModel;
import com.qualcomm.qti.sva.data.SettingsModel;
import com.qualcomm.qti.sva.utils.FileUtils;
import com.qualcomm.qti.sva.utils.LogUtils;

import java.io.File;
import java.io.FilenameFilter;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.ArrayList;

public class ExtendedSmMgr {
    private static final int SVA_APP_KEYPHRASE_ID_BASE = 500;
    private final String TAG = ExtendedSmMgr.class.getSimpleName();
    private ArrayList<IExtendedSmModel> mSoundModelList = new ArrayList<>();
    private static ArrayList<String> mUIMKeyphraseList = new ArrayList<>();

    private static final int GMM_KEYPHRASE_THRESHOLD = 40;
    private static final int GMM_USER_THRESHOLD = 1;

    // adapt the STManager API limitation
    // different SM 's keyphrase, need assign different keyphrase ID
    // different client, need assign different keyphrase ID
    private int mKeyphraseIdCounter = 0;

    public ExtendedSmMgr() {
    }

    public void initSoundModels(Context context) {
        mSoundModelList.clear();
        mUIMKeyphraseList.clear();

        File smDir = new File(Global.PATH_ROOT);
        if (null != smDir && smDir.exists()) {
            File[] files = smDir.listFiles(new FilenameFilter() {
                @Override
                public boolean accept(File file, String fileName) {
                    if (fileName.endsWith(ISmModel.SUFFIX_DEFAULT_SOUND_MODEL)
                            || fileName.endsWith(ISmModel.SUFFIX_TRAINED_SOUND_MODEL)) {
                        return true;
                    }
                    return false;
                }
            });

            if (null == files || 0 == files.length) {
                LogUtils.e(TAG, "initSoundModels: no sound model file found");
                return;
            }

            for (File item : files) {
                String smFullFileName = item.getName();
                addSoundModel(context, smFullFileName);
            }
        }
    }

    public static ArrayList<String> getPDKkeyphraseList() {
        return mUIMKeyphraseList;
    }

    public ArrayList<IExtendedSmModel> getAllSoundModelList() {
        return mSoundModelList;
    }

    public ArrayList<IExtendedSmModel> getSingleKeyphraseSoundModelList() {
        ArrayList<IExtendedSmModel> singleKeyphraseModelList = new ArrayList<>();
        for (IExtendedSmModel item : mSoundModelList) {
            if (item.getSoundModelKeyphraseList().size() == 1) {
                singleKeyphraseModelList.add(item);
            }
        }
        return singleKeyphraseModelList;
    }

    public ArrayList<IExtendedSmModel> getTrainableSoundModelList() {
        ArrayList<IExtendedSmModel> trainableSmList = new ArrayList<>();
        for (IExtendedSmModel item : mSoundModelList) {
            if (item.getSoundModelKeyphraseList().size() == 1
                    && ISmModel.SUFFIX_DEFAULT_SOUND_MODEL.equalsIgnoreCase(
                    item.getSoundModelSuffix())) {
                trainableSmList.add(item);
            }
        }
        return trainableSmList;
    }

    public ArrayList<IExtendedSmModel> getCanMergeSoundModelList() {
        ArrayList<IExtendedSmModel> canMergeSmList = new ArrayList<>();
        for (IExtendedSmModel item : mSoundModelList) {
            if (ISmModel.MODEL_VERSION.VERSION_2_0 == item.getSoundModelVersion()
                    && !item.isUdkSm()) {
                canMergeSmList.add(item);
            }
        }
        return canMergeSmList;
    }

    public ArrayList<IExtendedSmModel> getCanDeleteSoundModelList() {
        ArrayList<IExtendedSmModel> canDeleteSmList = new ArrayList<>();
        for (IExtendedSmModel item : mSoundModelList) {
            if (item.isUserTrainedSoundModel()) {
                canDeleteSmList.add(item);
            }
        }
        return canDeleteSmList;
    }

    public ArrayList<IExtendedSmModel> getAllLoadedSoundModelList() {
        ArrayList<IExtendedSmModel> loadedSoundModelList = new ArrayList<>();
        for (IExtendedSmModel item : mSoundModelList) {
            if (IExtendedSmModel.SessionStatus.STARTED == item.getSessionStatus()
                    || IExtendedSmModel.SessionStatus.LOADED == item.getSessionStatus()) {
                loadedSoundModelList.add(item);
            }
        }
        return loadedSoundModelList;
    }

    public ArrayList<String> getUIMKeyphraseList() {
        return mUIMKeyphraseList;
    }

    public IExtendedSmModel getSoundModel(String smFullName) {
        LogUtils.d(TAG, "getSoundModel: smFullName = " + smFullName);
        if (null == smFullName) {
            LogUtils.d(TAG, "getSoundModel: invalid input param");
            return null;
        }

        IExtendedSmModel extendedSmModel = null;
        for (IExtendedSmModel item : mSoundModelList) {
            if (item.getSoundModelFullFileName().equalsIgnoreCase(smFullName)) {
                extendedSmModel = item;
                break;
            }
        }
        return extendedSmModel;
    }

    public void addSoundModel(Context context, String smFullName) {
        LogUtils.d(TAG, "addSoundModel: smFullName = " + smFullName);
        if (null == smFullName) {
            LogUtils.d(TAG, "addSoundModel: invalid input param");
            return;
        }

        IExtendedSmModel soundModel = getSoundModel(smFullName);
        if (null == soundModel) {
            LogUtils.d(TAG, "addSoundModel: new one sound model");
            soundModel = new ExtendedSmModel(smFullName);
            mSoundModelList.add(soundModel);
        } else {
            LogUtils.d(TAG, "addSoundModel: sound model already exists");
        }

        updateSoundModel(context, soundModel);

        // set the opaque data transfer enabled by default for 3.0 model
        ISettingsModel settingsModel = new SettingsModel(context, smFullName);
        ISmModel.MODEL_VERSION version = soundModel.getSoundModelVersion();
        if (ISmModel.MODEL_VERSION.VERSION_3_0 == version
            || ISmModel.MODEL_VERSION.VERSION_4_0 == version) {
            LogUtils.d(TAG, "addSoundModel: 3.0 and 4.0 model set opaque data transfer enabled");
            settingsModel.setOpaqueDataTransferEnabled(true);
        }
    }

    public void updateSoundModel(Context context, IExtendedSmModel soundModel) {
        LogUtils.d(TAG, "updateSoundModel: soundModel = " + soundModel);
        if (null == soundModel) {
            LogUtils.e(TAG, "updateSoundModel: invalid input param");
            return;
        }

        String smFullName = soundModel.getSoundModelFullFileName();
        ListenTypes.SVASoundModelInfo modelInfo = query(smFullName);
        if (null != modelInfo) {
            soundModel.setSoundModelType(modelInfo.type);
            soundModel.setSoundModelVersion(convertVersion(modelInfo.version));
            if (null != modelInfo.keywordInfo && modelInfo.keywordInfo.length > 0) {
                soundModel.setSoundModelPrettyKeyphrase(modelInfo.keywordInfo[0].keywordPhrase);
                addKeyphraseAndUsers(context, modelInfo, soundModel);
            }
        }

        // set actionName and actionIntent for sound model
        ISettingsModel settingsModel = new SettingsModel(context, smFullName);
        soundModel.setSoundModelActionName(settingsModel.getActionName());
        soundModel.setSoundModelActionIntent(settingsModel.getActionIntent());
    }

    public void deleteSoundModel(String smFullName) {
        LogUtils.d(TAG, "deleteSoundModel: smFullName = " + smFullName);
        if (null == smFullName) {
            LogUtils.d(TAG, "deleteSoundModel: invalid input param");
            return;
        }
        IExtendedSmModel soundModel = getSoundModel(smFullName);
        if (null != soundModel) {
            mSoundModelList.remove(soundModel);
        }
    }

    private void addKeyphraseAndUsers(Context context, ListenTypes.SVASoundModelInfo modelInfo,
                                      IExtendedSmModel soundModel) {
        if (null == modelInfo || null == soundModel) {
            LogUtils.e(TAG, "addKeyphraseAndUsers: invalid input param");
            return;
        }

        ArrayList<String> indicesArr = generateKeyphraseUserIndicesArray(modelInfo);
        if (!indicesArr.isEmpty()) {
            for (ListenTypes.KeywordInfo keywordItem : modelInfo.keywordInfo) {
                int keyphraseId = generateKeyphraseId(context, indicesArr,
                        keywordItem.keywordPhrase);
                soundModel.addKeyphrase(keywordItem.keywordPhrase, keyphraseId);

                LogUtils.e(TAG, "addKeyphraseAndUsers: keyphrase " + keywordItem
                        + " 's keyphraseId = " + keyphraseId);

                // add the UIM model keyphrase to list
                if (!soundModel.isUserTrainedSoundModel()) {
                    if (!mUIMKeyphraseList.contains(keywordItem.keywordPhrase)) {
                        LogUtils.e(TAG, "addKeyphraseAndUsers: add "
                                + keywordItem.keywordPhrase + " to mUIMKeyphraseList");
                        mUIMKeyphraseList.add(keywordItem.keywordPhrase);
                    }
                }

                int userId = 0;
                if (IExtendedSmModel.VERSION_0300 == modelInfo.version) {
                    /* current keywordItem.activeUsers is null for 3.0 model,
                      which is not returned from SML, only one keyphrase support
                      for 3.0 model, so use userNames[] array to obtain the users
                    */
                    for (String userItem : modelInfo.userNames) {
                        userId = generateUserId(indicesArr,
                                keywordItem.keywordPhrase, userItem);
                        soundModel.addKeyphraseAndUser(keywordItem.keywordPhrase,
                                keyphraseId, userItem, userId);
                        LogUtils.e(TAG, "addKeyphraseAndUsers: 3.0 model user " + userItem
                                + " 's userId = " + userId);
                    }
                } else {
                    for (String userItem : keywordItem.activeUsers) {
                        userId = generateUserId(indicesArr,
                                keywordItem.keywordPhrase, userItem);
                        soundModel.addKeyphraseAndUser(keywordItem.keywordPhrase,
                                keyphraseId, userItem, userId);
                        LogUtils.e(TAG, "addKeyphraseAndUsers: user " + userItem
                                + " 's userId = " + userId);
                    }
                }
            }
        }
    }

    private int generateUserId(ArrayList<String> indicesArray, String keyphrase, String userName) {
        if (null == indicesArray || indicesArray.isEmpty()
                || null == keyphrase || null == userName) {
            LogUtils.e(TAG, "generateUserId: invalid input param");
            return IKeyphraseModel.INVALID_ID;
        }

        return indicesArray.indexOf(userName + "|" + keyphrase);
    }

    private int generateKeyphraseId(Context context, ArrayList<String> indicesArray,
                                    String keyphrase) {
        if (null == indicesArray || indicesArray.isEmpty() || null == keyphrase) {
            LogUtils.e(TAG, "generateKeyphraseId: invalid input param");
            return IKeyphraseModel.INVALID_ID;
        }

        return indicesArray.indexOf(keyphrase);
    }

    private ArrayList<String> generateKeyphraseUserIndicesArray(
            ListenTypes.SVASoundModelInfo modelInfo) {
        ArrayList<String> indicesArray = new ArrayList<>();
        if (null == modelInfo) {
            LogUtils.e(TAG, "generateKeyphraseUserIndicesArray: invalid input param");
            return indicesArray;
        }

        for (ListenTypes.KeywordInfo keywordItem : modelInfo.keywordInfo) {
            indicesArray.add(keywordItem.keywordPhrase);
            LogUtils.d(TAG, "generateKeyphraseUserIndicesArray: "
                    + keywordItem.keywordPhrase + " index = " + (indicesArray.size() - 1));
        }

        for (String userNameItem : modelInfo.userNames) {
            for (ListenTypes.KeywordInfo keywordItem : modelInfo.keywordInfo) {
                /* current keywordItem.activeUsers is null for 3.0 model,
                   which is not returned from SML, only one keyphrase support
                   for 3.0 model, so use userNames[] array to obtain the users
                */
                if (IExtendedSmModel.VERSION_0300 == modelInfo.version) {
                    for (String userItem : modelInfo.userNames) {
                        if (userNameItem.equals(userItem)) {
                            indicesArray.add(userNameItem + "|" + keywordItem.keywordPhrase);
                            LogUtils.d(TAG, "generateKeyphraseUserIndicesArray: 3.0 model "
                                    + keywordItem.keywordPhrase + " & " + userNameItem
                                    + " index = " + (indicesArray.size() - 1));
                        }
                    }
                } else {
                    for (String userItem : keywordItem.activeUsers) {
                        if (userNameItem.equals(userItem)) {
                            indicesArray.add(userNameItem + "|" + keywordItem.keywordPhrase);
                            LogUtils.d(TAG, "generateKeyphraseUserIndicesArray: 2.0 model "
                                    + keywordItem.keywordPhrase + " & " + userNameItem
                                    + " index = " + (indicesArray.size() - 1));
                        }
                    }
                }
            }
        }

        return indicesArray;
    }

    // this is only available for STManager API
    public String getSmNameByKeyphraseId(int keyphraseId) {
        String smName = null;
        if (keyphraseId < 0) {
            LogUtils.e(TAG, "getSmNameByKeyphraseId: invalid input param");
            return null;
        }

        for (IExtendedSmModel item : mSoundModelList) {
            for (IKeyphraseModel keyphraseItem : item.getSoundModelKeyphraseList()) {
                if (keyphraseId == keyphraseItem.getKeyphraseId()) {
                    smName = item.getSoundModelFullFileName();
                    break;
                }
            }
        }

        LogUtils.e(TAG, "getSmNameByKeyphraseId: keyphraseId = " + keyphraseId
                + " smName = " + smName);

        return smName;
    }

    private IExtendedSmModel.MODEL_VERSION convertVersion(int version) {
        IExtendedSmModel.MODEL_VERSION convertedVersion;
        switch (version) {
            case IExtendedSmModel.VERSION_0100:
                convertedVersion = IExtendedSmModel.MODEL_VERSION.VERSION_1_0;
                break;
            case IExtendedSmModel.VERSION_0200:
                convertedVersion = IExtendedSmModel.MODEL_VERSION.VERSION_2_0;
                break;
            case IExtendedSmModel.VERSION_0300:
                convertedVersion = IExtendedSmModel.MODEL_VERSION.VERSION_3_0;
                break;
            case IExtendedSmModel.VERSION_0400:
                convertedVersion = IExtendedSmModel.MODEL_VERSION.VERSION_4_0;
                break;
            default:
                convertedVersion = IExtendedSmModel.MODEL_VERSION.VERSION_UNKNOWN;
                break;
        }

        LogUtils.d(TAG, "convertVersion: version = " + version
                + " convertedVersion = " + convertedVersion);

        return convertedVersion;
    }

    private ListenTypes.SVASoundModelInfo query(String smFullName) {
        LogUtils.d(TAG, "query: smFullName = " + smFullName);
        if (null == smFullName) {
            LogUtils.e(TAG, "query: invalid input param");
            return null;
        }

        String filePath = Global.PATH_ROOT + "/" + smFullName;
        if (FileUtils.isExist(filePath)) {
            try {
                ByteBuffer smBuffer;
                smBuffer = FileUtils.readFileToByteBuffer(filePath);
                return (ListenTypes.SVASoundModelInfo) ListenSoundModel.query(smBuffer);
            } catch (IOException e) {
                LogUtils.e(TAG, "query: file IO exception");
                e.printStackTrace();
                return null;
            }
        } else {
            LogUtils.e(TAG, "query: error file not exists");
            return null;
        }
    }

    public void setSoundModelAction(String smFullName, String actionName, Intent actionIntent) {
        LogUtils.d(TAG, "setSoundModelAction: smFullName = " + smFullName);
        if (null == smFullName) {
            LogUtils.e(TAG, "setSoundModelAction: invalid input param");
            return;
        }

        IExtendedSmModel soundModel = getSoundModel(smFullName);
        if (null != soundModel) {
            soundModel.setSoundModelActionName(actionName);
            soundModel.setSoundModelActionIntent(actionIntent);
        }
    }

    public String getSoundModelFullPath(String smFullName) {
        LogUtils.d(TAG, "getSoundModelFullPath: smFullName = " + smFullName);
        if (null == smFullName) {
            LogUtils.e(TAG, "getSoundModelFullPath: invalid input param");
            return null;
        }
        return Global.PATH_ROOT + "/" + smFullName;
    }


    public boolean hasActiveSessions() {
        boolean hasActiveSession = false;
        for (IExtendedSmModel item : mSoundModelList) {
            if (IExtendedSmModel.SessionStatus.STARTED == item.getSessionStatus()) {
                hasActiveSession = true;
                break;
            }
        }

        return hasActiveSession;
    }

    public boolean hasLoadedOrActiveSessions() {
        boolean hasLoadedOrActiveSession = false;
        for (IExtendedSmModel item : mSoundModelList) {
            if (IExtendedSmModel.SessionStatus.UNLOADED != item.getSessionStatus()) {
                hasLoadedOrActiveSession = true;
                break;
            }
        }

        return hasLoadedOrActiveSession;
    }

    public void resetAllSessionStatus() {
        LogUtils.d(TAG, "resetAllSessionStatus: enter");
        for (IExtendedSmModel item : mSoundModelList) {
            item.setSessionStatus(IExtendedSmModel.SessionStatus.UNLOADED);
        }
    }

    public String getSmNameByHandle(int smHandle) {
        String smName = null;
        for (IExtendedSmModel item : mSoundModelList) {
            if (item.getSoundModelHandle() == smHandle) {
                smName = item.getSoundModelFullFileName();
                break;
            }
        }
        LogUtils.d(TAG, "getSmNameByHandle: smHandle = " + smHandle
                + " smName = " + smName);
        return smName;
    }

    public ByteBuffer getLanguageModel(ISmModel.MODEL_VERSION version) {
        LogUtils.d(TAG, "getLanguageModel: enter");
        String filePath = null;
        if (version == ISmModel.MODEL_VERSION.VERSION_4_0) {
            filePath = Global.PATH_ROOT + "/" + Global.NAME_MERGED_LANGUAGE_MODEL;
            LogUtils.d(TAG, "getLanguageModel: merged model");
        } else {
            filePath = Global.PATH_ROOT + "/" + Global.NAME_LANGUAGE_MODEL;
            LogUtils.d(TAG, "getLanguageModel: default model");
        }
        if (FileUtils.isExist(filePath)) {
            try {
                ByteBuffer languageModel = FileUtils.readFileToByteBuffer(filePath);
                return languageModel;
            } catch (IOException e) {
                e.printStackTrace();
                return null;
            }
        } else {
            LogUtils.d(TAG, "getLanguageModel: language model file not exist");
            return null;
        }
    }

}
