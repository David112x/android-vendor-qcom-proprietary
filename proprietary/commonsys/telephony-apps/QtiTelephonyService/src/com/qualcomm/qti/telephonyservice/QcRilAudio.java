/*
 * Copyright (c) 2017 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.telephonyservice;

import android.content.Context;
import android.media.AudioManager;
import android.os.IBinder;
import android.os.HwBinder;
import android.os.Message;
import android.os.RemoteException;
import android.util.Log;
import java.util.concurrent.atomic.AtomicLong;
import vendor.qti.hardware.radio.am.V1_0.AudioError;
import vendor.qti.hardware.radio.am.V1_0.IQcRilAudio;
import vendor.qti.hardware.radio.am.V1_0.IQcRilAudioCallback;

import android.hidl.manager.V1_0.IServiceManager;
import android.hidl.manager.V1_0.IServiceNotification;

public class QcRilAudio {
    final static String TAG = "QcRilAudio";

    private final String mInstanceName;
    private final AtomicLong mRilAudioCookie = new AtomicLong(0);

    private final AudioProxyDeathRecipient mRilAudioDeathRecipient = new AudioProxyDeathRecipient();
    private final ServiceNotification mServiceNotification = new ServiceNotification();
    private final IQcRilAudioCallback mQcRilAudioCallback = new QcRilAudioCallback();
    private IQcRilAudio mRilAudio;
    private boolean mIsDisposed = false;
    private AudioManager mAudioManager = null;

    private class QcRilAudioCallback extends IQcRilAudioCallback.Stub {
        @Override
        public int setParameters(String keyValuePairs) {
            Log.d(TAG, "setParameters with: " + keyValuePairs);
            if (mAudioManager == null) {
                Log.e(TAG, "AudioManager is null, which should not occur.");
                return AudioError.AUDIO_GENERIC_FAILURE;
            }

            mAudioManager.setParameters(keyValuePairs);
            return getAudioServerRunningErrorCode();
        }

        @Override
        public String getParameters(String key) {
            Log.d(TAG, "getParameters for key: " + key);
            if (mAudioManager == null) {
                Log.e(TAG, "AudioManager is null, which should not occur");
                return "";
            }

            return mAudioManager.getParameters(key);
        }
    }

    private class ServiceNotification extends IServiceNotification.Stub {
        @Override
        public void onRegistration(String fqName, String name, boolean preexisting) {
            Log.d(TAG, "onRegistration: fqName=" + fqName + " name=" + name);

            if (!mInstanceName.equals(name) || isDisposed()) {
                Log.d(TAG, "onRegistration: Ignoring.");
                dump("onRegistration");
                return;
            }

            initHal();
            setError(getAudioServerRunningErrorCode());
        }
    }

    /**
     * Checks if the Audio Server is running.
     * @return AudioError type error code
     **/
    private int getAudioServerRunningErrorCode() {
        if (mAudioManager.isAudioServerRunning()) {
            return AudioError.AUDIO_STATUS_OK;
        }
        return AudioError.AUDIO_GENERIC_FAILURE;
    }

    private class AudioProxyDeathRecipient implements HwBinder.DeathRecipient {
        @Override
        public void serviceDied(long cookie) {
            if (isDisposed()) {
                Log.d(TAG, "serviceDied: Ignoring.");
                dump("serviceDied");
                return;
            }

            final long current = mRilAudioCookie.get();
            if (cookie != current) {
                Log.v(TAG, "serviceDied: Ignoring. provided=" + cookie + " expected=" + current);
                return;
            }

            Log.e(TAG, "IQcRilAudio service died");
            resetService();
        }

    }

    public QcRilAudio(int slotId, Context context) {
        mInstanceName = "slot" + slotId;

        try {
            boolean ret = IServiceManager.getService()
                    .registerForNotifications("vendor.qti.hardware.radio.am@1.0::IQcRilAudio",
                    mInstanceName, mServiceNotification);
            mAudioManager = (AudioManager) context.getSystemService(
                    Context.AUDIO_SERVICE);
            if (!ret) {
                Log.e(TAG, "Unable to register service start notification: ret = " + ret);
            }
        } catch (RemoteException ex) {
            Log.e(TAG, "Unable to register service start notification");
        }
    }

    public synchronized void dispose() {
        if (isDisposed()) {
            return;
        }
        mIsDisposed = true;

        try {
            if (mRilAudio != null) {
                mRilAudio.unlinkToDeath(mRilAudioDeathRecipient);
                mRilAudio = null;
            }
        } catch(RemoteException e) {
            Log.d(TAG, "dispose: Exception=" + e );
        }
    }

    public boolean isDisposed() {
        return mIsDisposed;
    }

    private synchronized void initHal() {
        Log.d(TAG, "initHal");

        try {
            mRilAudio = IQcRilAudio.getService(mInstanceName);
            if (mRilAudio == null) {
                Log.e(TAG, "initHal: mRilAudio == null");
                return;
            }

            mRilAudio.linkToDeath(mRilAudioDeathRecipient,
                    mRilAudioCookie.incrementAndGet());
            mRilAudio.setCallback(mQcRilAudioCallback);

        } catch (Exception e) {
                Log.e(TAG, "initHal: Exception: " + e);
        }
    }

    public synchronized void onAudioServerUp() {
        setError(AudioError.AUDIO_STATUS_OK);
    }

    public synchronized void onAudioServerDown() {
        setError(AudioError.AUDIO_STATUS_SERVER_DIED);
    }

    private synchronized void setError(int errorCode) {
        if (mRilAudio == null) {
            Log.w(TAG, "setError - mRilAudio is null, returning.");
            return;
        }

        try {
            mRilAudio.setError(errorCode);
        } catch (Exception e) {
            Log.e(TAG, "setError request to IQcRilAudio. Exception: " + e);
        }
    }

    private synchronized void resetService() {
        mRilAudio = null;
    }

    private void dump(String fn) {
        Log.d(TAG, fn + ": InstanceName=" + mInstanceName);
        Log.d(TAG, fn + ": isDisposed=" + isDisposed());
    }

    public String getInstanceName() {
        return mInstanceName;
    }

}
