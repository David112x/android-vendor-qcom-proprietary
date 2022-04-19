/*
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.qmmi.testcase.Audio;

import android.content.Intent;
import android.media.AudioManager;

import com.qualcomm.qti.qmmi.R;
import com.qualcomm.qti.qmmi.bean.TestCase;
import com.qualcomm.qti.qmmi.framework.BaseService;
import com.qualcomm.qti.qmmi.utils.LogUtils;

import java.util.HashMap;

import android.os.AsyncTask;
import android.os.SystemProperties;
import android.provider.Settings.System;

public class AudioService extends BaseService {

    private final static String INIT_SERVICE = "vendor.mm_audio_ftm";

    private AudioManager mAudioManager = null;
    private TestCase mTestCase = null;

    //silent control
    private int mPreRingMode = -1;
    private boolean isPreTouchSoundsEnabled = false;

    private HashMap<String,String> METHODS_MAP = new HashMap<String,String>(){
        {
            put("mm_audio_ftm","");
        }
    };

    public int onStartCommand(Intent intent, int flags, int startId) {
        LogUtils.logi("onStartCommand");
        initService();
        return super.onStartCommand(intent, flags, startId);
    }

    private boolean initService() {
        mAudioManager = (AudioManager) getSystemService(AUDIO_SERVICE);
        if (mAudioManager == null) {
            LogUtils.loge("get audio service fail");
            return false;
        }
        return true;
    }

    @Override
    public void register() {}


    @Override
    public int run(TestCase testCase) {

        mTestCase = testCase;
        if (mTestCase == null) {
            LogUtils.loge("Null param, please check the framework error");
            return -1;
        }

        String method = mTestCase.getMethod();
        LogUtils.logi("method:" + method);

        if (method != null && isMethodSupported(method)) {
            if (method.equalsIgnoreCase("mm_audio_ftm")) {
                enterSilent();
                TestByInitService(true);
            }
        } else {
            LogUtils.loge("no method , or not support method");
        }

        LogUtils.logi("Audio service run");
        return 0;
    }

    void enterSilent() {
        controlTouchSound(true);

        if (mAudioManager != null) {
            mPreRingMode = mAudioManager.getRingerMode();
            LogUtils.logi("Pre Ring Mode:" + mPreRingMode);
            mAudioManager.setRingerMode(AudioManager.RINGER_MODE_SILENT);
        } else {
            //every time return to unavailable
            mPreRingMode = -1;
        }
    }

    private void controlTouchSound(boolean isCaseBegin){
        LogUtils.logi("Control Touch Sound: is Case Begin:" + isCaseBegin);
        //0: Disable; 1: Enable
        if (isCaseBegin){
            isPreTouchSoundsEnabled = System.getInt(this.getContentResolver(),
                    System.SOUND_EFFECTS_ENABLED, 1) != 0;
        }
        LogUtils.logi("Pre Touch Sounds state:" + isPreTouchSoundsEnabled);

        if (isPreTouchSoundsEnabled) {
            if(isCaseBegin){
                AsyncTask.execute(new Runnable() {
                    @Override
                    public void run() {
                        mAudioManager.loadSoundEffects();
                    }
                });
                System.putInt(this.getContentResolver(), System.SOUND_EFFECTS_ENABLED, 0);
                updateView(mTestCase.getName(), this.getResources().getString(R.string.audio_disable_touch_sounds));

                try {
                    Thread.sleep(5000);
                } catch (InterruptedException e) {
                    e.printStackTrace();
                }
            } else {
                AsyncTask.execute(new Runnable() {
                    @Override
                    public void run() {
                        mAudioManager.unloadSoundEffects();
                    }
                });
                System.putInt(this.getContentResolver(), System.SOUND_EFFECTS_ENABLED, 1);
                updateView(mTestCase.getName(), this.getResources().getString(R.string.audio_enable_touch_sounds));
            }
        }
    }

    boolean isMethodSupported(String method) {
        return METHODS_MAP.containsKey(method);
    }

    private void TestByInitService(boolean isStart){

        String initService = mTestCase.getParameter().get("init_service");
        LogUtils.logi(" initService: " + initService + " isStart:" + isStart);

        if (initService != null && initService.length() > 0) {
            //Start test Audio TC
            if (isStart) {
                updateView(mTestCase.getName(), this.getResources().getString(R.string.audio_start_test));
                SystemProperties.set("ctl.start", initService);
            } else {
                updateView(mTestCase.getName(), this.getResources().getString(R.string.audio_stop_test));
                SystemProperties.set("ctl.stop", initService);
            }
        } else {
            updateView(mTestCase.getName(), this.getResources().getString(R.string.audio_miss_params));
            LogUtils.loge("no init_service value");
        }
    }

    @Override
    public int stop(TestCase testCase) {
        if (mTestCase == null) {
            LogUtils.loge("Null param, please check the framework error");
            return -1;
        }

        //exist silent,restore pre mode back
        String method = mTestCase.getMethod();
        if (method != null && isMethodSupported(method)) {
            if (method.equalsIgnoreCase("mm_audio_ftm")) {
                exitSilent();
                TestByInitService(false);
            }
        }

        return 0;
    }

    void exitSilent() {
        if (mAudioManager != null && mPreRingMode >= 0) {
            mAudioManager.setRingerMode(mPreRingMode);
        }

        controlTouchSound(false);
    }
}
