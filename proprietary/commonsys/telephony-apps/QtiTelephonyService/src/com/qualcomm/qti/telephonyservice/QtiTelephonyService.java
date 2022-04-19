/*
 * Copyright (c) 2014, 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.telephonyservice;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.concurrent.Executor;
import java.util.List;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.media.AudioManager;
import android.os.IBinder;
import android.telephony.TelephonyManager;
import android.util.Log;

import com.qualcomm.qcrilhook.QcRilHook;
import com.qualcomm.qcrilhook.IQcRilHook;

public class QtiTelephonyService extends Service {

    private static final String TAG = "QtiTelephonyService";

    private IQcRilHook mQcRilHook;
    private QtiTelephonyServiceImpl mService;
    private List<QcRilAudio> mQcRilAudioHals = new ArrayList<>();
    private AudioServerStateCallback mServerStateCallback = new AudioServerStateCallback();
    private AudioManager mAudioManager = null;
    private Context mContext = null;

    private class AudioServerStateCallback extends AudioManager.AudioServerStateCallback {
        @Override
        public void onAudioServerDown() {
            for (QcRilAudio audio: mQcRilAudioHals) {
                Log.d(TAG, "notifying onAudioServerDown for: " + audio.getInstanceName());
                audio.onAudioServerDown();
            }
        }

        @Override
        public void onAudioServerUp() {
            for (QcRilAudio audio: mQcRilAudioHals) {
                Log.d(TAG, "notifying onAudioServerUp for: " + audio.getInstanceName());
                audio.onAudioServerUp();
            }
        }

    }

    @Override
    public void onCreate() {
        Log.i(TAG, "onCreate:");
        mQcRilHook = new QcRilHook(getApplicationContext());
        mService = new QtiTelephonyServiceImpl(super.getApplicationContext(), mQcRilHook);
        mContext = getApplicationContext();

        // RIL Audio HAL initialization.
        TelephonyManager tm  = (TelephonyManager) mContext.getSystemService(
                Context.TELEPHONY_SERVICE);
        final int slotCount = tm.getPhoneCount() + 1;
        for (int i = 1; i < slotCount; ++i) {
            mQcRilAudioHals.add(new QcRilAudio(i, mContext));
        }

        // Creating callback for audio server status
        Log.d(TAG, "registering audio server state callback");
        mAudioManager = (AudioManager) mContext.getSystemService(Context.AUDIO_SERVICE);
        Executor exec = mContext.getMainExecutor();
        mAudioManager.setAudioServerStateCallback(exec, mServerStateCallback);
    }

    // setQcRilHook allows instrumentation to set a test double for IQcRilHook.
    // The method is intentionally private - instrumentation uses reflection to access this method.
    private void setQcRilHook(IQcRilHook qcRilHook) {
        mQcRilHook = qcRilHook;
        mService.setQcRilHook(qcRilHook);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.d(TAG, "Service started");
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public IBinder onBind(Intent intent) {
        Log.d(TAG, "onBind");
        return mService;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        Log.d(TAG, "onUnbind");
        return super.onUnbind(intent);
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        mService = null;

        // RIL Audio HAL de-initialization
        for (QcRilAudio h : mQcRilAudioHals) {
            h.dispose();
        }

        // unregister audio server state callback
        if (mAudioManager != null) {
            Log.d(TAG, "unregistering audio server state callback");
            mAudioManager.clearAudioServerStateCallback();
            mAudioManager = null;
        }

        super.onDestroy();
    }
}
