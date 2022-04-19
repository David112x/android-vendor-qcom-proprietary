/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva.controller;

import android.content.Context;
import android.media.AudioAttributes;
import android.media.SoundPool;

import com.qualcomm.qti.sva.R;
import com.qualcomm.qti.sva.utils.LogUtils;

public class TonePlayer implements SoundPool.OnLoadCompleteListener {
    private final String TAG = TonePlayer.class.getSimpleName();

    private SoundPool mSoundPool;
    private static final int MAX_STREAMS = 8;
    private static final int[] RES_IDS = {
            R.raw.succeed,
            R.raw.fail_m
    };
    private final int[] mSoundIdArray;
    private final boolean[] mSoundIdReadyArray;

    public TonePlayer(Context context) {
        AudioAttributes attributes = new AudioAttributes.Builder()
                .setUsage(AudioAttributes.USAGE_NOTIFICATION)
                .build();
        mSoundPool = new SoundPool.Builder()
                .setMaxStreams(MAX_STREAMS)
                .setAudioAttributes(attributes)
                .build();
        mSoundPool.setOnLoadCompleteListener(this);
        mSoundIdArray = new int[RES_IDS.length];
        mSoundIdReadyArray = new boolean[RES_IDS.length];
        for (int i = 0; i < RES_IDS.length; i++) {
            mSoundIdArray[i] = mSoundPool.load(context, RES_IDS[i], 1);
            LogUtils.d(TAG, "TonePlayer: i = " + i + " mSoundIdArray[i] = "
                    + mSoundIdArray[i]);
            mSoundIdReadyArray[i] = false;
        }
    }

    @Override
    public void onLoadComplete(SoundPool soundPool, int soundId, int status) {
        if (0 != status) {
            LogUtils.e(TAG, "onLoadComplete: load sound track fail for id = " + soundId);
            return;
        }

        for (int i = 0; i < mSoundIdArray.length; i++) {
            if (soundId == mSoundIdArray[i]) {
                mSoundIdReadyArray[i] = true;
                break;
            }
        }
    }

    public void play(int resId) {
        LogUtils.d(TAG, "play: resId = " + resId);
        if (resId < 0 || resId >= RES_IDS.length) {
            LogUtils.e(TAG, "play: invalid resId");
            return;
        }

        if (!mSoundIdReadyArray[resId]) {
            LogUtils.e(TAG, "play: sound " + mSoundIdArray[resId] + " is not loaded yet");
            return;
        }

        mSoundPool.play(mSoundIdArray[resId],
                0.5f, 0.5f, 0, 0, 1f);
    }

    public void release() {
        LogUtils.d(TAG, "release: enter");
        if (null != mSoundPool) {
            mSoundPool.release();
            mSoundPool = null;
        }
    }

    @Override
    protected void finalize() throws Throwable {
        release();
        super.finalize();
    }
}
