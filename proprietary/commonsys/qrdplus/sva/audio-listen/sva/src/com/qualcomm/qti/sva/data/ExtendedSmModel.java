/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva.data;

import android.content.Intent;
import android.hardware.soundtrigger.SoundTrigger;

import com.qualcomm.qti.sva.utils.LogUtils;

public class ExtendedSmModel extends SmModel implements IExtendedSmModel {
    private final String TAG = ExtendedSmModel.class.getSimpleName();

    private final String DEFAULT_ACTION_NAME = "None";
    private String mSmActionName = DEFAULT_ACTION_NAME;
    private Intent mSmActionIntent;
    private SessionStatus mSessionStatus = SessionStatus.UNLOADED;
    private SoundTrigger.KeyphraseRecognitionExtra[] mKpRecognitionExtraArray;
    private SoundTrigger.RecognitionConfig mRecognitionConfig;
    private int mSoundModelHandle = -1;
    private int mSessionId = 0;

    public ExtendedSmModel(String smFullFileName) {
        super(smFullFileName);
    }

    @Override
    public SessionStatus getSessionStatus() {
        return mSessionStatus;
    }

    @Override
    public void setSessionStatus(SessionStatus status) {
        mSessionStatus = status;
    }

    @Override
    public String getSoundModelActionName() {
        return mSmActionName;
    }

    @Override
    public void setSoundModelActionName(String actionName) {
        if (null == actionName) {
            mSmActionName = DEFAULT_ACTION_NAME;
        } else {
            mSmActionName = actionName;
        }
    }

    @Override
    public Intent getSoundModelActionIntent() {
        return mSmActionIntent;
    }

    @Override
    public void setSoundModelActionIntent(Intent actionIntent) {
        mSmActionIntent = actionIntent;
    }

    @Override
    public SoundTrigger.KeyphraseRecognitionExtra[] getKeyphraseRecognitionExtra() {
        return mKpRecognitionExtraArray;
    }

    @Override
    public void setKeyphraseRecognitionExtra(SoundTrigger.KeyphraseRecognitionExtra[] extraArray) {
        mKpRecognitionExtraArray = extraArray.clone();
    }

    @Override
    public SoundTrigger.RecognitionConfig getRecognitionConfig() {
        return mRecognitionConfig;
    }

    @Override
    public void setRecognitionConfig(SoundTrigger.RecognitionConfig config) {
        mRecognitionConfig = config;
    }

    @Override
    public int getSoundModelHandle() {
        return mSoundModelHandle;
    }

    @Override
    public void setSoundModelHandle(int handle) {
        mSoundModelHandle = handle;
    }

    @Override
    public int getSessionId() {
        return mSessionId;
    }

    @Override
    public void setSessionId(int id) {
        LogUtils.d(TAG, "setSessionId: id = " + id);
        if (id <= 0) {
            LogUtils.d(TAG, "setSessionId: invalid input param");
        }

        mSessionId = id;
    }
}
