/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package com.android.bluetooth.ba;

import android.bluetooth.BluetoothA2dp;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.BluetoothAdapter;

import com.android.bluetooth.ba.BluetoothBAStreamServiceRecord;
import com.android.bluetooth.ba.BluetoothBAEncryptionKey;
import com.android.bluetooth.btservice.ProfileService;
import com.android.bluetooth.a2dp.A2dpService;
import com.android.bluetooth.hfp.HeadsetService;
import com.android.bluetooth.Utils;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Binder;
import android.os.SystemProperties;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.provider.Settings;
import android.media.AudioManager;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Objects;
import java.nio.ByteBuffer;

/**
 * Provides Bluetooth Broadcast Audio profile, as a service in
 * the Bluetooth application.
 * @hide
 */
public class BATService extends ProfileService {

    private static final boolean DBG = true;
    private static final String TAG = "BATService";

    private static final int NUM_SERIVCE_RECORD = 1;
    private static final long STREAM_ID_48 = 1;
    // currently we are using 512(fr_samples),186(frame_size),frequency.
    // check GattService specification for more details
    //private static final long  CODEC_CONFIG_CELT = (long)0x020000BA0100;
    // Messages
    private final int MESSAGE_BAT_STATE_CHANGE_REQ = 1;
    private final int MESSAGE_BAT_REFRESH_ENC_KEY_REQ = 2;
    private final int MESSAGE_BAT_VOL_CHANGE_REQ = 3;

    private final int MESSAGE_BAT_STATE_CHANGE_EVT = 101;
    private final int MESSAGE_BAT_ENC_CHANGE_EVT = 102;
    private final int MESSAGE_BAT_DIV_CHANGE_EVT = 103;
    private final int MESSAGE_BAT_STREAMING_ID_EVT = 104;

    private final String BLUETOOTH_PERM = android.Manifest.permission.BLUETOOTH;
    static final String BLUETOOTH_PERM_ADMIN = android.Manifest.permission.BLUETOOTH_ADMIN;
    public static final String mBAAddress = "FA:CE:FA:CE:FA:CE";
    private BluetoothDevice mBADevice;
    // we will listen for vol change intent from audio manager.
    // this intent is called in all following 3 cases
    // 1 - Local vol changing when there is no abs vol
    // 2 - Local vol changes if abs vol not supported
    // 3 - Vol changed from remote if abs vol is supported

    /**
     * Intent used to update state change of Broadcast Audio Transmitter.
     *
     * This intent will have 2 extras:
     * #EXTRA_STATE - The current state of the profile.
     * #EXTRA_PREVIOUS_STATE - The previous state of the profile.
     *
     * value of states can be any of
     * STATE_DISABLED: Broadcast Audio is disabled.
     * STATE_PAUSED:   Broadcast Audio is enabled but streaming is paused.
     * STATE_PLAYING:  Broadcast Audio is enabled but streaming is ongoing.
     *
     */
    protected static final String ACTION_BAT_STATE_CHANGED =
            "com.android.bluetooth.bat.profile.action.BA_STATE_CHANGED";
    protected static final String EXTRA_STATE =
            "com.android.bluetooth.bat.profile.extra.STATE";
    protected static final String EXTRA_PREVIOUS_STATE =
            "com.android.bluetooth.bat.profile.extra.PREV_STATE";

     /**
     * Intent is used to update state change of Broadcast Receiver Association.
     *
     * This intent will have 1 extra:
     * #EXTRA_STATUS – status code
     *
     * value of states can be any of
     *
     * BRA_ENABLED_SUCESSS (0)
     * BRA_ENABLED_FAILED (1)
     * BRA_DISABLED_SUCESSS (2)
     * BRA_DISABLED_FAILED(3)
     */
    protected static final String ACTION_BAT_BRA_STATE_CHANGED =
            "com.android.bluetooth.bat.profile.action.BRA_STATE_CHANGED";
    protected static final String EXTRA_STATUS =
            "com.android.bluetooth.bat.profile.extra.STATUS";

    /**
     * Intent is used to for updating  Broadcast Audio Receiver info, after Broadcast audio receiver is Lost or Found.
     *
     * This intent will have 2 extras:
     * #EXTRA_SCAN_RESULT – Scan Result for particular device.
     * #EXTRA_ONFOUND –  true if device found, false if device lost.
     *
     */
    protected static final String ACTION_BAT_ONFOUND_ONLOST_BCA_RECEIVER =
            "com.android.bluetooth.bat.profile.action.ONFOUND_ONLOST_BCA_RECEIVER";
    protected static final String EXTRA_SCAN_RESULT =
            "com.android.bluetooth.bat.profile.extra.EXTRA_SCAN_RESULT";
    protected static final String EXTRA_ONFOUND =
            "com.android.bluetooth.bat.profile.extra.EXTRA_ONFOUND";

    /**
     * Intent is used to for updating Broadcast Audio Aeceiver info, after a Broadcast audio receiver is associated.
     *
     * This intent will have 2 extras:
     * #EXTRA_DEVICE–  BluetoothDevice info
     * #EXTRA_STATUS – Status code
     *
     *  value of status code can be any of
     *
     *  ASSOCIATE_BCA_RECEIVER_SUCCESS(4)
     *  ASSOCIATE_BCA_RECEIVER_FAILED(5)
     */
    protected static final String ACTION_BAT_ON_ASSOCIATED_BCA_RECEIVER =
            "com.android.bluetooth.bat.profile.action.ON_ASSOCIATED_BCA_RECEIVER";

    /**
     * Intent is used, when Broadcast Audio Encryption Key is refreshed.
     *
     */
    private final String ACTION_BAT_ENCRYPTION_KEY_REFRESHED =
            "com.android.bluetooth.bat.profile.action.ENCRYPTION_KEY_REFRESHED";
    private final String ACTION_BAT_ENCRYPTION_KEY_REFRESH =
            "com.android.bluetooth.bat.profile.action.ENCRYPTION_KEY_REFRESH";

    /**
     * Intents are used to enable/disable Broadcast audio
     */
    private final String ACTION_BAT_BA_ENABLE =
            "com.android.bluetooth.bat.profile.action.BA_ENABLE";
    private final String ACTION_BAT_BA_DISABLE =
            "com.android.bluetooth.bat.profile.action.BA_DISABLE";

    /**
     * Intent used to update encryption key .
     *
     * This intent will have 1 extra:
     * #EXTRA_ENCRYPTION_KEY - The current value of encryption key.
     *
     * value of EncyptionKey would be 128-bit value. This value would change
     * on every new BA session. We will send BluetoothBAEncryptionKey object.
     *
     */
    protected static final String ACTION_BAT_ENCRYPTION_KEY_CHANGED =
            "com.android.bluetooth.bat.profile.action.BA_ENC_KEY_CHANGED";
    protected static final String EXTRA_ECNRYPTION_KEY =
            "com.android.bluetooth.bat.profile.extra.ENC_KEY";

    /**
     * Intent used to update DIV value .
     *
     * This intent will have 1 extras:
     * #EXTRA_DIV_VALUE - The current value of DIV.
     *
     * value of DIV would be 2 byte value. This value would change
     * on every new BA session. We will send integer value.
     *
     */
    protected static final String ACTION_BAT_DIV_CHANGED =
            "com.android.bluetooth.bat.profile.action.BA_DIV_CHANGED";
    protected static final String EXTRA_DIV_VALUE =
            "com.android.bluetooth.bat.profile.extra.DIV";

    /**
     * Intent used to update  active stream id for Broadcast Audio Transmitter.
     *
     * This intent will have 1 extra:
     * #EXTRA_STREAM_ID - The active streaming id.
     *
     * value of states can be any of
     * 0: Broadcast Audio is not in STATE_PLAYING.
     * valid streaming id:   Valid streaming id if BA is in STATE_PLAYING.
     */
    protected static final String ACTION_BAT_STREAMING_ID_CHANGED =
            "com.android.bluetooth.bat.profile.action.BA_STR_ID_CHANGED";
    protected static final String EXTRA_STREAM_ID =
            "com.android.bluetooth.bat.profile.extra.STR_ID";

    /**
     * Intent used to update  Vendor Specific AVRCP Command.
     *
     * This intent will be sent whenever there is Vendor Specific AVRCP command
     * received from primary headset.
     *
     * This intent will have 2 extra:
     * #EXTRA_AVRCP_VS_ENABLE_BA - value describing enable/disable of BA.
     * #EXTRA_AVRCP_VS_ENABLE_RA - value describing enable/disable of receiver
     *                              association mode.
     *
     * value of states can be any of
     * 0: Disable  BA/RA.
     * 1: ENABLE  BA/RA.
     */
    private final String ACTION_BAT_AVRCP_VS_CMD =
            "com.android.bluetooth.bat.profile.action.BA_AVRCP_VS_CMD";
    private final String EXTRA_AVRCP_VS_ENABLE_BA =
            "com.android.bluetooth.bat.profile.extra.ENABLE_BA";
    private final String EXTRA_AVRCP_VS_ENABLE_RA =
            "com.android.bluetooth.bat.profile.extra.ENABLE_RA";
    protected static final String ACTION_BAT_BRA_ENABLE =
            "com.android.bluetooth.bat.profile.action.BRA_ENABLE";
    public static final String ACTION_BAT_BRA_DISABLE =
            "com.android.bluetooth.bat.profile.action.BRA_DISABLE";
    protected static final String ACTION_BAT_ASSOCIATE_BCA_RECEIVER =
            "com.android.bluetooth.bat.profile.action.ASSOCIATE_BCA_RECEIVER";

    protected static final int STATE_DISABLED = 0;
    protected static final int STATE_PAUSED = 1;
    protected static final int STATE_PLAYING = 2;
    private final int ENABLE_BA_TRANSMITTER = 0;
    private final int DISABLE_BA_TRANSMITTER = 1;
    private final int INVALID_DIV = 0xFFFF;
    public static int BA_TRANSMITTER = 23;      // Added to access in Config.java

    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {

            String action = intent.getAction();
            Log.d(TAG," action: " + action);

            if(action == null)
                return;
            if (action.equals(AudioManager.VOLUME_CHANGED_ACTION)) {
                Log.d(TAG," onReceive  AudioManager Vol Changed");
                int streamType = intent.getIntExtra(AudioManager.EXTRA_VOLUME_STREAM_TYPE, -1);
                Log.d(TAG," streamType = " + streamType);
                if (streamType == AudioManager.STREAM_MUSIC) {
                    int streamValue = intent.getIntExtra(
                            AudioManager.EXTRA_VOLUME_STREAM_VALUE, -1);
                    int streamPrevValue = intent.getIntExtra(
                            AudioManager.EXTRA_PREV_VOLUME_STREAM_VALUE, -1);
                    Log.d(TAG," prevVol = " + streamPrevValue + " streamVol = " + streamValue);
                    if (mMsgHandler.hasMessages(MESSAGE_BAT_VOL_CHANGE_REQ)) {
                        mMsgHandler.removeMessages(MESSAGE_BAT_VOL_CHANGE_REQ);
                    }
                    mMsgHandler.obtainMessage(MESSAGE_BAT_VOL_CHANGE_REQ,
                            streamValue,streamValue).sendToTarget();
                }
            } else if (action.equals(ACTION_BAT_BA_ENABLE)) {
                setBATState(ENABLE_BA_TRANSMITTER);
            } else if (action.equals(ACTION_BAT_BA_DISABLE)) {
                setBATState(DISABLE_BA_TRANSMITTER);
            } else if (action.equals(ACTION_BAT_ENCRYPTION_KEY_REFRESH)) {
               refreshEncryptionKey();
            } else if (action.equals(BluetoothAdapter.ACTION_BLE_STATE_CHANGED)) {
                int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE,
                        BluetoothAdapter.ERROR);
                int prevState = intent.getIntExtra(BluetoothAdapter.EXTRA_PREVIOUS_STATE,
                        BluetoothAdapter.ERROR);
                if (state == BluetoothAdapter.STATE_BLE_ON || state == BluetoothAdapter.STATE_ON) {
                    Log.d(TAG,"ACTION_BLE_STATE_CHANGED state: " + state);
                    if (mGattBroadcastService == null) {
                        mGattBroadcastService = new GattBroadcastService();
                        mGattBroadcastService.start(getApplicationContext());
                    }
                }
            }
        }
    };

    private AudioManager mAudioManager;
    private int mCurrStackBATState;
    private int mPrevStackBATState;
    private int mCurrDIV;
    private int mCurrVolLevel;
    private boolean isCodecReconfigRequired;
    private BluetoothBAEncryptionKey mCurrEncryptionKey;
    private BluetoothBAStreamServiceRecord mServiceRecord;
    private BATMessageHandler mMsgHandler;
    private static BATService sBATService;
    private BluetoothAdapter mAdapter;
    private GattBroadcastService mGattBroadcastService;

    // we need pending state only during transition from enable/disable.
    // no need for statemachine, at this point. Can be manager with a variable.

    static {
        classInitNative();
    }

    protected IProfileServiceBinder initBinder() {
        return new BluetoothBATBinder(this);
    }

    private static class BluetoothBATBinder extends Binder implements IProfileServiceBinder {

        BluetoothBATBinder(BATService service) {}

        @Override
        public void cleanup() {
        }
    }

    @Override
    protected boolean start() {
        Log.d(TAG, "BATService :: start + ");
        mAudioManager = (AudioManager) getSystemService(Context.AUDIO_SERVICE);
        mCurrVolLevel = mAudioManager.getStreamVolume(AudioManager.STREAM_MUSIC);
        mAdapter = BluetoothAdapter.getDefaultAdapter();
        mCurrStackBATState = BA_STACK_STATE_IDLE;
        mPrevStackBATState = mCurrStackBATState;
        isCodecReconfigRequired = false;
        initNative();
        setBATService(this);
        HandlerThread thread =  new HandlerThread("BATMsgHandler");
        thread.start();
        Looper looper = thread.getLooper();
        mMsgHandler = new BATMessageHandler(looper);
        Log.d(TAG," mMsgHandler = " + mMsgHandler);
        // initialize with default value.
        mCurrDIV = INVALID_DIV;
        byte[] mEncryptionKey = new byte[BluetoothBAEncryptionKey.ENCRYPTION_KEY_LENGTH];
        mCurrEncryptionKey = new BluetoothBAEncryptionKey(mEncryptionKey,
                BluetoothBAEncryptionKey.SECURITY_KEY_TYPE_PRIVATE);
        // create defualt service record.

        mServiceRecord =  new BluetoothBAStreamServiceRecord(1);
        mServiceRecord.addServiceRecordValue(STREAM_ID_48,
                BluetoothBAStreamServiceRecord.BSSR_TYPE_STREAM_ID, (long)1);
        mServiceRecord.addServiceRecordValue(STREAM_ID_48,
                BluetoothBAStreamServiceRecord.BSSR_TYPE_SECURITY_ID,
                (BluetoothBAStreamServiceRecord.BSSR_SECURITY_KEY_TYPE_PRIVATE|
                BluetoothBAStreamServiceRecord.BSSR_SECURITY_ENCRYPT_TYPE_AESCCM));
        mServiceRecord.addServiceRecordValue(STREAM_ID_48,
                BluetoothBAStreamServiceRecord.BSSR_TYPE_CODEC_TYPE_ID,
                BluetoothBAStreamServiceRecord.BSSR_CODEC_TYPE_CELT);
        mServiceRecord.addServiceRecordValue(STREAM_ID_48,
                BluetoothBAStreamServiceRecord.BSSR_TYPE_ERASURE_CODE_ID,
                BluetoothBAStreamServiceRecord.BSSR_ERASURE_CODE_2_5);
        mServiceRecord.addServiceRecordValue(STREAM_ID_48,
                BluetoothBAStreamServiceRecord.BSSR_TYPE_CHANNELS_ID,
                BluetoothBAStreamServiceRecord.BSSR_CHANNELS_STEREO);
        mServiceRecord.addServiceRecordValue(STREAM_ID_48,
                BluetoothBAStreamServiceRecord.BSSR_TYPE_SAMPLE_SIZE_ID,
                BluetoothBAStreamServiceRecord.BSSR_SAMPLE_SIZE_16_BIT);
        mServiceRecord.addServiceRecordValue(STREAM_ID_48,
                BluetoothBAStreamServiceRecord.BSSR_TYPE_AFH_UPDATE_METHOD_ID,
                BluetoothBAStreamServiceRecord
                .BSSR_AFH_CHANNEL_MAP_UPDATE_METHOD_TRIGGERED_SYNC_TRAIN);
        mServiceRecord.addServiceRecordValue(STREAM_ID_48,
                BluetoothBAStreamServiceRecord.BSSR_TYPE_CODEC_CONFIG_CELT_FREQ_ID,
                BluetoothBAStreamServiceRecord.BSSR_CODEC_FREQ_48KHZ);
        mServiceRecord.addServiceRecordValue(STREAM_ID_48,
                BluetoothBAStreamServiceRecord.BSSR_TYPE_CODEC_CONFIG_CELT_FRAME_SIZE_ID,(long)186);
        mServiceRecord.addServiceRecordValue(STREAM_ID_48,
                BluetoothBAStreamServiceRecord
                .BSSR_TYPE_CODEC_CONFIG_CELT_FRAME_SAMPLES_ID, (long)512);

        IntentFilter filter = new IntentFilter(AudioManager.VOLUME_CHANGED_ACTION);
        filter.addAction(ACTION_BAT_BA_ENABLE);
        filter.addAction(ACTION_BAT_BA_DISABLE);
        filter.addAction(ACTION_BAT_ENCRYPTION_KEY_REFRESH);
        filter.addAction(BluetoothAdapter.ACTION_BLE_STATE_CHANGED);
        registerReceiver(mBroadcastReceiver, filter);
        mMsgHandler.obtainMessage(MESSAGE_BAT_VOL_CHANGE_REQ,
                mCurrVolLevel,mCurrVolLevel).sendToTarget();
        mBADevice = mAdapter.getRemoteDevice(mBAAddress);
        return true;
    }

    @Override
    protected boolean stop() {
        Log.d(TAG, "BATService :: stop + ");
        if(mMsgHandler != null) {
            mMsgHandler.removeCallbacksAndMessages(null);
            Looper looper = mMsgHandler.getLooper();
            if(looper != null)
                looper.quit();
        }

        if(mGattBroadcastService != null) {
            mGattBroadcastService.stop();
            mGattBroadcastService = null;
        }
        Log.d(TAG, "BATService :: stop - ");
        return true;
    }

    @Override
    protected void cleanup() {
        if(mMsgHandler != null) {
            mMsgHandler.removeCallbacksAndMessages(null);
            Looper looper = mMsgHandler.getLooper();
            if(looper != null)
                looper.quit();
        }
        try {
            unregisterReceiver(mBroadcastReceiver);
        }
        catch (IllegalArgumentException e) {
            Log.e(TAG, "Receiver not registered");
        }
        cleanupNative();
        clearBATService();
    }

    //API Methods

    public static synchronized BATService getBATService(){
        if (sBATService != null && sBATService.isAvailable()) {
            if (DBG) Log.d(TAG, "getBATService(): returning " + sBATService);
            return sBATService;
        }
        if (DBG)  {
            if (sBATService == null) {
                Log.d(TAG, "getBATService(): service is NULL");
            } else if (!(sBATService.isAvailable())) {
                Log.d(TAG,"getBATService(): service is cleaning up");
            }
        }
        return null;
    }

    private static synchronized void setBATService(BATService instance) {
        if (instance != null) {
            if (DBG) Log.d(TAG, "setBATService(): set to: " + instance);
            sBATService = instance;
        } else {
            if (DBG)  {
                if (instance == null) {
                    Log.d(TAG, "sBATService(): service not available");
                }
            }
        }
    }

    private static synchronized void clearBATService() {
        sBATService = null;
    }

    private String dumpMessageString(int message) {
        String str = "UNKNOWN";
        switch (message) {
            case MESSAGE_BAT_STATE_CHANGE_REQ: str = "REQ_STATE_CHANGE"; break;
            case MESSAGE_BAT_REFRESH_ENC_KEY_REQ: str = "REQ_REFRESH_KEY"; break;
            case MESSAGE_BAT_DIV_CHANGE_EVT: str = "CB_DIV_UPDATED"; break;
            case MESSAGE_BAT_ENC_CHANGE_EVT: str = "CB_ENC_KEY_UPDATED"; break;
            case MESSAGE_BAT_STATE_CHANGE_EVT: str = "CB_STATE_CHANDED"; break;
            case MESSAGE_BAT_STREAMING_ID_EVT: str = "CB_STREAM_ID_UPDATED"; break;
            default:
                str = Integer.toString(message);
        }
        return str;
    }

    private String dumpStateString ( int state) {
        String str = "UNKNOWN";
        switch (state) {
            case BA_STACK_STATE_IDLE: str = "BT_STACK_STATE_IDLE"; break;
            case BA_STACK_STATE_PENDING: str = "BA_STACK_STATE_PENDING"; break;
            case BA_STACK_STATE_PAUSED: str = "BA_STACK_STATE_PAUSED"; break;
            case BA_STACK_STATE_STREAMING: str = "BA_STACK_STATE_STREAMING"; break;
            case BA_STACK_STATE_AUDIO_PENDING: str = "BA_STACK_STATE_AUDIO_PENDING"; break;
            default:str = "UNKNOWN"; break;
        }
        return str;
    }
    // From P onwards, we do send all States, independent of what A2DP has sent.
    private void fakeA2dpConnectionBroadcastUpdate() {
        Log.d(TAG," fakeA2dpConnectionBroadcastUpdate state changed "
            + dumpStateString(mPrevStackBATState) + " -> " + dumpStateString(mCurrStackBATState));
        boolean updateConnectionState = false;
        Intent intent = new Intent(BluetoothA2dp.ACTION_CONNECTION_STATE_CHANGED);
        // when state changes from PENDING to IDLE, send A2DP Connected -> Disconnected
        if((mPrevStackBATState == BA_STACK_STATE_PENDING) &&
                (mCurrStackBATState == BA_STACK_STATE_IDLE)) {
            intent.putExtra(BluetoothProfile.EXTRA_PREVIOUS_STATE,
                    BluetoothProfile.STATE_CONNECTED);
            intent.putExtra(BluetoothProfile.EXTRA_STATE, BluetoothProfile.STATE_DISCONNECTED);
            updateConnectionState = true;
        }
        // when state changes from PENDING to PAUSED, send A2DP Disconnected -> Connected
        if((mPrevStackBATState == BA_STACK_STATE_PENDING) &&
                (mCurrStackBATState == BA_STACK_STATE_PAUSED)) {
            intent.putExtra(BluetoothProfile.EXTRA_PREVIOUS_STATE,
                                                       BluetoothProfile.STATE_DISCONNECTED);
            intent.putExtra(BluetoothProfile.EXTRA_STATE, BluetoothProfile.STATE_CONNECTED);
            updateConnectionState = true;
        }
        // when state changed from IDLE To PENDING, send disconnected to connecting.
        if((mPrevStackBATState == BA_STACK_STATE_IDLE) &&
                (mCurrStackBATState == BA_STACK_STATE_PENDING)) {
            intent.putExtra(BluetoothProfile.EXTRA_PREVIOUS_STATE,
                    BluetoothProfile.STATE_DISCONNECTED);
            intent.putExtra(BluetoothProfile.EXTRA_STATE, BluetoothProfile.STATE_CONNECTING);
            updateConnectionState = true;
        }
        // when state changed from PAUSE/STREAMING To PENDING, send connected to disconnecting.
        if(((mPrevStackBATState == BA_STACK_STATE_PAUSED) ||
                (mPrevStackBATState == BA_STACK_STATE_STREAMING)) &&
                (mCurrStackBATState == BA_STACK_STATE_PENDING)) {
            intent.putExtra(BluetoothProfile.EXTRA_PREVIOUS_STATE,
                    BluetoothProfile.STATE_DISCONNECTED);
            intent.putExtra(BluetoothProfile.EXTRA_STATE, BluetoothProfile.STATE_DISCONNECTING);
            updateConnectionState = true;
        }

        if(!updateConnectionState) {
            Log.d(TAG," state transition from " + dumpStateString(mPrevStackBATState)
                  + " -> " + dumpStateString(mCurrStackBATState)
             + " don't send intent ");
            return;
        }
        intent.putExtra(BluetoothDevice.EXTRA_DEVICE, mBADevice);
        intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);
        sendBroadcast(intent, BLUETOOTH_PERM);
    }

    private void fakeA2dpStreamingBroadcastUpdate() {
        Log.d(TAG," fakeA2dpStreamingBroadcastUpdate state changed "
              + dumpStateString(mPrevStackBATState)
              + " -> " + dumpStateString(mCurrStackBATState));

        boolean updatePlayState = false;
        Intent intent = new Intent(BluetoothA2dp.ACTION_PLAYING_STATE_CHANGED);
        intent.putExtra(BluetoothDevice.EXTRA_DEVICE, mBADevice);
        intent.addFlags(Intent.FLAG_RECEIVER_REGISTERED_ONLY_BEFORE_BOOT);

        if(mCurrStackBATState == BA_STACK_STATE_PAUSED) {
            intent.putExtra(BluetoothProfile.EXTRA_PREVIOUS_STATE, BluetoothA2dp.STATE_PLAYING);
            intent.putExtra(BluetoothProfile.EXTRA_STATE, BluetoothA2dp.STATE_NOT_PLAYING);
            updatePlayState = true;
        }

        if(mCurrStackBATState == BA_STACK_STATE_STREAMING) {
            intent.putExtra(BluetoothProfile.EXTRA_PREVIOUS_STATE, BluetoothA2dp.STATE_NOT_PLAYING);
            intent.putExtra(BluetoothProfile.EXTRA_STATE, BluetoothA2dp.STATE_PLAYING);
            updatePlayState = true;
        }

        if(!updatePlayState) {
            Log.d(TAG," state transition from " + mPrevStackBATState + " -> " + mCurrStackBATState
                    + " don't send intent ");
            return;
        }

        sendBroadcast(intent, BLUETOOTH_PERM);
    }

    private void broadcastBAStateChanged() {
        Log.d(TAG," broadcastBAStateChanged  currStackState = "
              + dumpStateString(mCurrStackBATState));
        int broadcastSate;
        switch (mCurrStackBATState) {
            case BA_STACK_STATE_IDLE:
                broadcastSate = STATE_DISABLED;
                break;
            case BA_STACK_STATE_PAUSED:
                broadcastSate = STATE_PAUSED;
                break;
            case BA_STACK_STATE_STREAMING:
                broadcastSate = STATE_PLAYING;
                break;
            default:
                // we don't want to send intent for other states.
                return;
        }
        Log.d(TAG," broadcasting state = " + broadcastSate);
        Intent intent = new Intent(ACTION_BAT_STATE_CHANGED);
        intent.putExtra(EXTRA_STATE, broadcastSate);
        intent.putExtra(BluetoothProfile.EXTRA_PREVIOUS_STATE, mPrevStackBATState);
        sendBroadcast(intent, BLUETOOTH_PERM_ADMIN);
    }

    private void broadcastEncKeyUpdate(BluetoothBAEncryptionKey encKey) {
        Log.d(TAG," broadcastEncKeyUpdate ");
        Intent intent = new Intent(ACTION_BAT_ENCRYPTION_KEY_CHANGED);
        intent.putExtra(EXTRA_ECNRYPTION_KEY, encKey);
        sendBroadcast(intent, BLUETOOTH_PERM_ADMIN);
    }

    private void broadcastDIVUpdate(int div) {
        Log.d(TAG," broadcastDIVUpdate div =  "+ div);
        Intent intent = new Intent(ACTION_BAT_DIV_CHANGED);
        intent.putExtra(EXTRA_DIV_VALUE, div);
        sendBroadcast(intent, BLUETOOTH_PERM_ADMIN);
    }

    private void broadcastStramIdpdate(int streamID) {
        Log.d(TAG," broadcastStramIdpdate streamID =  " + streamID);
        Intent intent = new Intent(ACTION_BAT_STREAMING_ID_CHANGED);
        intent.putExtra(EXTRA_STREAM_ID, streamID);
        sendBroadcast(intent, BLUETOOTH_PERM_ADMIN);
    }

    // this api checks if abs vol is supported by AVRCP with any of the connected devices
    // if yes, AVRCP would have updated audiomanager about absolute vol support,
    // otherwise we will do it
    // if this api returns false, we will call audiomanager api to indicate absvol support.
    public boolean checkAbsVolSupport() {
        boolean ret = false;
        A2dpService a2dpService = A2dpService.getA2dpService();
        if (a2dpService != null) {;
            ret = a2dpService.isAvrcpAbsoluteVolumeSupported();
        }
        Log.d(TAG," checkAbsVolSupport returning " + ret);
        return ret;
    }
    /* BA is simulated as an A2DP Connection to MM-Audio.
     * Whenever Active device changes in A2DP, we inform AM about disc of previous and
     * connection of new.
     */
    private void informAudioConnection() {
        Log.d(TAG," informAudioConnection state changed "
              + dumpStateString(mPrevStackBATState)
              + " -> " + dumpStateString(mCurrStackBATState));
        A2dpService a2dpService = A2dpService.getA2dpService();
        BluetoothDevice a2dpActiveDevice = null;
        if (a2dpService != null) {
            a2dpActiveDevice = a2dpService.getActiveDevice();
        }
        Log.d(TAG," A2DPActiveDevice = " + a2dpActiveDevice);

        // check for transition from PENDING to PAUSED
        if((mPrevStackBATState == BA_STACK_STATE_PENDING) &&
                (mCurrStackBATState == BA_STACK_STATE_PAUSED)) {
            Log.d(TAG," updating AudioManager: Connected for BA ");
            mAudioManager.handleBluetoothA2dpActiveDeviceChange(
                mBADevice, BluetoothProfile.STATE_CONNECTED,BluetoothProfile.A2DP, true, -1);
            //BA audio works on the principal of absVol
            //Currently mm-audio tracks value of last updated absVol support,
            //and does not use address.Even if avrcp has updated support as true,
            //we doing it again, should not lead to any issue.
            Log.d(TAG," inform AudioManager for absvol support ");
            mAudioManager.avrcpSupportsAbsoluteVolume(mBAAddress, true);
        }
        // check for transition from PENDING to IDLE
        if((mPrevStackBATState == BA_STACK_STATE_PENDING) &&
                (mCurrStackBATState == BA_STACK_STATE_IDLE)) {

            if (a2dpActiveDevice != null) {
                // inform BA device as disconnected, don't send noisy intent
                // as a2dp has to be updated as well. Switching should happen to
                // A2DP in this case.
                Log.d(TAG," updating AudioManager: Connected for A2DP ");
                mAudioManager.handleBluetoothA2dpActiveDeviceChange(
                    a2dpActiveDevice, BluetoothProfile.STATE_CONNECTED,BluetoothProfile.A2DP,
                    true, -1);
            } else {// a2dp active device is null.
                // inform BA device as disconnected. we have to send noisy intent
                // because BA seems to be last device.
                mAudioManager.handleBluetoothA2dpActiveDeviceChange(
                        mBADevice, BluetoothProfile.STATE_DISCONNECTED,BluetoothProfile.A2DP,
                        false, -1);
            }
            if (!checkAbsVolSupport()) {
                //Abs vol not supported by AVRCP, we should update false from BA.
                Log.d(TAG," AbsVol not supported by AVRCP, update false from BA ");
                mAudioManager.avrcpSupportsAbsoluteVolume(mBAAddress, false);
            }
        }
        // if transition is from AUDIO_PENDING->STREAMING, we need to update volume
        if((mPrevStackBATState == BA_STACK_STATE_AUDIO_PENDING) &&
                (mCurrStackBATState == BA_STACK_STATE_STREAMING)) {
            mCurrVolLevel = mAudioManager.getStreamVolume(AudioManager.STREAM_MUSIC);
            mMsgHandler.obtainMessage(MESSAGE_BAT_VOL_CHANGE_REQ,
                mCurrVolLevel,mCurrVolLevel).sendToTarget();
        }
    }

    // service level apis to be called from native callbacks
    private void processBAStateUpdate(int newState) {
        Log.d(TAG," processBAStateUpdate newState = " + dumpStateString(newState));
        mMsgHandler.obtainMessage(MESSAGE_BAT_STATE_CHANGE_EVT, newState,0).sendToTarget();
    }

    private void processEncryptionKeyUpdate(int size, byte[] enc_key) {
        Log.d(TAG," processEncryptionKeyUpdate size = " + size);
        Bundle data =  new Bundle();
        data.putByteArray("encKey", enc_key);
        Message msg = mMsgHandler.obtainMessage(MESSAGE_BAT_ENC_CHANGE_EVT, size, 0);
        msg.setData(data);
        mMsgHandler.sendMessage(msg);
    }

    private void processDivUpdate(int size, byte[] div_key) {
        Log.d(TAG," processDivUpdate size = " + size);
        ByteBuffer bb = ByteBuffer.wrap(div_key);
        Message msg = mMsgHandler.obtainMessage(MESSAGE_BAT_DIV_CHANGE_EVT, (int)bb.getShort(), 0);
        mMsgHandler.sendMessage(msg);
    }

    private void processStreamIdUpdate(int streamId) {
       Log.d(TAG," processStreamIdUpdate streamID = " + streamId);
       Message msg = mMsgHandler.obtainMessage(MESSAGE_BAT_STREAMING_ID_EVT, (int)streamId, 0);
       mMsgHandler.sendMessage(msg);
    }

    public boolean isBATActive() {
        Log.d(TAG," isBATActive  mCurrStackState = " + dumpStateString(mCurrStackBATState));
        return ((mCurrStackBATState == BA_STACK_STATE_PAUSED) ||
                (mCurrStackBATState == BA_STACK_STATE_AUDIO_PENDING) ||
                (mCurrStackBATState == BA_STACK_STATE_STREAMING));
    }

    public boolean isA2dpSuspendFromBA() {
        Log.d(TAG," isA2dpSuspendFromBA  mCurrStackState = "+
                        dumpStateString(mCurrStackBATState)
              + "codecReconfig = " + isCodecReconfigRequired);
        return isCodecReconfigRequired;
    }

    private boolean isA2dpPlaying() {
        A2dpService a2dpService = A2dpService.getA2dpService();
        if (a2dpService == null) {
            Log.d(TAG," isA2dpPlaying = false a2dpService null");
            return false;
        }
        List<BluetoothDevice> devList = a2dpService.getConnectedDevices();
        if (devList.isEmpty()) {
            Log.d(TAG," isA2dpPlaying = false no connected devices ");
            return false;
        }
        for (BluetoothDevice dev: devList) {
            if (a2dpService.isA2dpPlaying(dev)) {
                Log.d(TAG," isA2dpPlaying = true playing dev =  " + dev);
                return true;
            }
        }
        Log.d(TAG," isA2dpPlaying = false  dev connected but not playing ");
        return false;
    }

    private boolean isCallActive() {
        HeadsetService headsetService = HeadsetService.getHeadsetService();
        if (headsetService == null) {
            Log.d(TAG," isCallActive = false HeadsetService null");
            return false;
        }
        boolean callActive = headsetService.isInCall()||
                             headsetService.isRinging()
                             ||headsetService.isAudioOn();
        Log.d(TAG," isCallActive: " + callActive);
        return callActive;
    }

    private void performCodecSwitch() {
        A2dpService a2dpService = A2dpService.getA2dpService();
        BluetoothDevice a2dpActiveDevice = null;
        if (a2dpService != null) {
            a2dpActiveDevice = a2dpService.getActiveDevice();
        }
        Log.d(TAG," A2DPActiveDevice = " + a2dpActiveDevice);

        Log.d(TAG," performCodecSwitch state changed "
              + dumpStateString(mPrevStackBATState) + " -> "
              + dumpStateString(mCurrStackBATState)
              + " isCodecReconfigReq "+ isCodecReconfigRequired);
        // if there is a transition from PENDING to IDLE and A2DP active device is there
        // perform codecswitch.
        if ((mPrevStackBATState == BA_STACK_STATE_PENDING) &&
              (mCurrStackBATState == BA_STACK_STATE_IDLE)) {
            if(a2dpActiveDevice != null) {
                mAudioManager.setParameters("reconfigA2dp=true");
            }
        }
        // if there is a transition from PENDING to PASUED, and A2dp was streaming earlier.
        if ((mPrevStackBATState == BA_STACK_STATE_PENDING) &&
                (mCurrStackBATState == BA_STACK_STATE_PAUSED)) {
            if(isCodecReconfigRequired) {
                if (!isCallActive()) {
                    mAudioManager.setParameters("reconfigA2dp=true");
                }
                isCodecReconfigRequired = false;
            }
        }
    }

    // a message handler to serialize all message operations
    private final class BATMessageHandler extends Handler {
        private BATMessageHandler(Looper looper) {
            super(looper);
            Log.d(TAG," MessageHandler constructor ");
        }
        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG," HandleMessage = " + dumpMessageString(msg.what));
            switch(msg.what) {
                case MESSAGE_BAT_STATE_CHANGE_REQ:
                    int newState = msg.arg1;
                    if (newState == ENABLE_BA_TRANSMITTER) {
                        if (isCallActive()) {
                            Log.d(TAG," Call active, can't initilze BA ");
                            return;
                        }
                        if (isA2dpPlaying()) {
                            // A2DP Streaming was ongoing before BA Start request.
                            isCodecReconfigRequired = true;
                            //mAudioManager.setParameters("A2dpSuspended=true");
                        }
                        setBAStateNative(1);
                    } else if (newState == DISABLE_BA_TRANSMITTER) {
                        isCodecReconfigRequired = false;
                        setBAStateNative(0);
                    }
                    break;

                case MESSAGE_BAT_REFRESH_ENC_KEY_REQ:
                    refreshEncKeyNative();
                    break;

                case MESSAGE_BAT_STATE_CHANGE_EVT:
                    mPrevStackBATState = mCurrStackBATState;
                    mCurrStackBATState = msg.arg1;
                    broadcastBAStateChanged();
                    fakeA2dpConnectionBroadcastUpdate();
                    fakeA2dpStreamingBroadcastUpdate();
                    informAudioConnection();
                    performCodecSwitch();
                    break;

                case MESSAGE_BAT_ENC_CHANGE_EVT:
                    mCurrEncryptionKey = new BluetoothBAEncryptionKey(
                            msg.getData().getByteArray("encKey"),
                            BluetoothBAEncryptionKey.SECURITY_KEY_TYPE_PRIVATE);
                    broadcastEncKeyUpdate(mCurrEncryptionKey);
                    break;

                case MESSAGE_BAT_DIV_CHANGE_EVT:
                    mCurrDIV = msg.arg1;
                    broadcastDIVUpdate(mCurrDIV);
                    break;

                case MESSAGE_BAT_STREAMING_ID_EVT:
                    broadcastStramIdpdate(msg.arg1);
                    break;

                case MESSAGE_BAT_VOL_CHANGE_REQ:
                    mCurrVolLevel = msg.arg1;
                    int maxVolLevel = mAudioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
                    Log.d(TAG," currLevel = " + msg.arg1 + " Max Vol = " + maxVolLevel);
                    setVolNative(mCurrVolLevel, maxVolLevel);
                    break;
            }
        }
    }
    public boolean setBATState(int newState) {
        Log.d(TAG," setBATState ( " + newState + ") currState = "
              + dumpStateString(mCurrStackBATState));
        if((newState == ENABLE_BA_TRANSMITTER) &&
             (mCurrStackBATState != BA_STACK_STATE_IDLE)) {
            // we are already enabled, or in process of getting enabled
            return false;
        }
        if((newState == DISABLE_BA_TRANSMITTER) &&
           (mCurrStackBATState == BA_STACK_STATE_IDLE)) {
            // we are already disabled
            return false;
        }
        // if we are in middle of transition, or have already queued a message,send false
        if (mCurrStackBATState == BA_STACK_STATE_PENDING)
            return false;
        if (mMsgHandler.hasMessages(MESSAGE_BAT_STATE_CHANGE_REQ))
            return false;
        if (isCallActive() && (newState == ENABLE_BA_TRANSMITTER)) {
            Log.d(TAG," setBATState Call active, can't initilze BA ");
            return false;
        }
        mMsgHandler.obtainMessage(MESSAGE_BAT_STATE_CHANGE_REQ,newState,0).sendToTarget();
        return true;
    }

    protected int getBATState() {
        int state = STATE_DISABLED;
        Log.d(TAG," getBATState = " + dumpStateString(mCurrStackBATState));
        switch (mCurrStackBATState) {
            case BA_STACK_STATE_IDLE:
            case BA_STACK_STATE_PENDING:
                state = STATE_DISABLED;
                break;
            case BA_STACK_STATE_PAUSED:
            case BA_STACK_STATE_AUDIO_PENDING:
                state = STATE_PAUSED;
                break;
            case BA_STACK_STATE_STREAMING:
                state = STATE_PLAYING;
                break;
        }
        return state;
    }

    protected int getDIV() {
        Log.d(TAG," getDIV = " + mCurrDIV);
        if ((mCurrStackBATState == BA_STACK_STATE_IDLE) ||
            (mCurrStackBATState == BA_STACK_STATE_PENDING))
            return 0;
        return mCurrDIV;
    }

    protected long getStreamId() {
        Log.d(TAG," getStreamId state = " + dumpStateString(mCurrStackBATState));
        // send same ID every time.
        return STREAM_ID_48;
    }

    protected BluetoothBAEncryptionKey getEncryptionKey() {
        Log.d(TAG," getEncryptionKey state = " + dumpStateString(mCurrStackBATState));
        if ((mCurrStackBATState == BA_STACK_STATE_IDLE) ||
                (mCurrStackBATState == BA_STACK_STATE_PENDING) )
            return null;
        return mCurrEncryptionKey;
    }

    private boolean refreshEncryptionKey() {
        Log.d(TAG," refreshEncryptionKey state = " + dumpStateString(mCurrStackBATState));
        if (mCurrStackBATState == BA_STACK_STATE_IDLE)
            return false;
        mMsgHandler.obtainMessage(MESSAGE_BAT_REFRESH_ENC_KEY_REQ).sendToTarget();
        return true;
    }

    protected BluetoothBAStreamServiceRecord getBAServiceRecord() {
        Log.d(TAG," getBAServiceRecord state = " + dumpStateString(mCurrStackBATState));
        return mServiceRecord;
    }

    private void onBATStateChanged(int newState) {
        Log.d(TAG," onBATStateChanged ( " + newState + " )");
        if (sBATService != null) {
            sBATService.processBAStateUpdate(newState);
        }
    }

    private void onEncKeyUpdate(byte size, byte[] enc_key) {
        Log.d(TAG," onEncKeyUpdate  size = " + size);
        if (sBATService != null) {
            sBATService.processEncryptionKeyUpdate(size, enc_key);
        }
    }

    private void onDivUpdate(byte size, byte[] div_key) {
        Log.d(TAG," onDivUpdate size = " + size);
        if (sBATService != null) {
            sBATService.processDivUpdate(size, div_key);
        }
    }

    private void onStreamIdUpdate(byte streamId) {
        Log.d(TAG," onStreamIdUpdate  streamID = " + streamId);
        if(sBATService != null) {
            sBATService.processStreamIdUpdate(streamId);
        }
    }

    // make these states in sync with values in hal
    private final int BA_STACK_STATE_IDLE = 0;
    private final int BA_STACK_STATE_PENDING = 1;// transitioining between IDLE and non-idle state.
    private final int BA_STACK_STATE_PAUSED = 2;
    private final int BA_STACK_STATE_STREAMING = 3;
    private final int BA_STACK_STATE_AUDIO_PENDING = 4;// transitioning between Pu and Str

    private native static void classInitNative();
    private native static void initNative();
    private native static void refreshEncKeyNative();
    private native static void cleanupNative();
    private native static void setBAStateNative(int enable);
    private native static void setVolNative(int volLevel, int maxVolLevel);
}
