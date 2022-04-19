/*
 * Copyright (c) 2017, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *    * Neither the name of The Linux Foundation nor the names of its
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
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

package org.codeaurora.bluetooth.batestapp;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothProfile;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.media.AudioAttributes;
import android.media.AudioFocusRequest;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioManager.OnAudioFocusChangeListener;
import android.media.AudioRecord;
import android.media.AudioTrack;
import android.media.MediaRecorder;
import android.media.audiofx.AcousticEchoCanceler;
import android.media.audiofx.AutomaticGainControl;
import android.media.audiofx.NoiseSuppressor;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.util.Log;

// Class to handle all interactions with Audio part of Broadcast Audio Transmitter.
public class BAAudio {

    static final String TAG = "BAAPP   ";
    public static final String BASERVICE_STATE_CHANGED = "android.bluetooth.bat.service";
    public static final String EXTRA_BA_STATE = "android.bluetooth.extra.ba.state";
    public static final String EXTRA_CONN_STATE = "android.bluetooth.extra.conn.state";
    private static final int SAMPLE_RATE = 16000;
    /* CHANNEL_CONFIGURATION_MONO @deprecated Use {@link #CHANNEL_OUT_MONO} or {@link #CHANNEL_IN_MONO} instead.  */
    private static final int CHANNEL_CONFIG_RECORD = AudioFormat.CHANNEL_IN_MONO;
    private static final int CHANNEL_CONFIG_PLAYBACK = AudioFormat.CHANNEL_OUT_MONO;
    private static final int AUDIO_FORMAT = AudioFormat.ENCODING_PCM_16BIT;
    private static final int RECORD_BUF_SIZE = AudioRecord.getMinBufferSize(SAMPLE_RATE,
            CHANNEL_CONFIG_RECORD, AUDIO_FORMAT);
    private static final int PLAYBACK_BUF_SIZE = AudioTrack.getMinBufferSize(SAMPLE_RATE,
            CHANNEL_CONFIG_PLAYBACK, AUDIO_FORMAT);
    private final static int MSG_STOP_RECORD_PLAY = 1;
    private final static int MSG_START_RECORD_PLAY = 2;
    private final static int MSG_AUDIO_FOCUS_CHANGE = 3;
    private static boolean sIsBAReady = false;
    private static boolean sIsPlaying = false;
    private BluetoothAdapter mAdapter;
    private BAAudioReceiver mReceiver;
    private Context mContext;
    private int mCurrBATState;
    private AudioRecord mAudioRecord = null;
    private AudioTrack mAudioTrack = null;
    private int mCurrAudioFocusState = AudioManager.AUDIOFOCUS_LOSS;
    private AudioManager mAudioManager;
    private BAMsgHandler mHandler;
    private StreamingThread mStrThread;
    // This is returned when requesting focus from AudioManager
    private AudioFocusRequest mfocusRequest;
    private int BAT_State = BroadcastAudioAppActivity.STATE_DISABLED;
    static int BA_TRANSMITTER = 23;

    private OnAudioFocusChangeListener mAudioFocusListener = new OnAudioFocusChangeListener() {
        public void onAudioFocusChange(int focusVal) {
            Log.d(TAG, "focusChangs val = " + focusVal);
            if (mHandler != null)
                mHandler.obtainMessage(MSG_AUDIO_FOCUS_CHANGE, focusVal, 0).sendToTarget();
        }
    };

    public BAAudio(Context context) {

        Log.d(TAG, " BAAudio constructor");
        mContext = context;
        mReceiver = new BAAudioReceiver();
        mAdapter = BluetoothAdapter.getDefaultAdapter();

        IntentFilter filter = new IntentFilter(
                BroadcastAudioAppActivity.ACTION_BAT_STATE_CHANGED);
        filter.addAction(BroadcastAudioAppActivity.ACTION_BAT_ENCRYPTION_KEY_CHANGED);
        filter.addAction(BroadcastAudioAppActivity.ACTION_BAT_STATE_CHANGED);
        mContext.registerReceiver(mReceiver, filter);

        mCurrBATState = BroadcastAudioAppActivity.STATE_DISABLED;
        mAudioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);

        HandlerThread thread = new HandlerThread("BAaudioHandler");
        thread.start();
        Looper looper = thread.getLooper();
        mHandler = new BAMsgHandler(looper);

        AudioAttributes streamAttributes =
                new AudioAttributes.Builder().setUsage(AudioAttributes.USAGE_MEDIA)
                        .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                                .build();
        mfocusRequest = new AudioFocusRequest.Builder(AudioManager.AUDIOFOCUS_GAIN)
                .setAudioAttributes(streamAttributes)
                        .setOnAudioFocusChangeListener(mAudioFocusListener)
                                .build();
    }

    public void broadcastServiceConnection(boolean isConnected, int state) {
        Log.d(TAG, " Sending Intent service conn = " + isConnected + " ba_state = " + state);
        Intent intent = new Intent(BASERVICE_STATE_CHANGED);
        intent.putExtra(EXTRA_CONN_STATE, isConnected);
        intent.putExtra(EXTRA_BA_STATE, state);
        mContext.sendBroadcast(intent);
    }

    public void cleanup() {
        Log.v(TAG," cleanup ");
        if ((mCurrAudioFocusState == AudioManager.AUDIOFOCUS_GAIN) ||
                (mCurrAudioFocusState == AudioManager.AUDIOFOCUS_LOSS_TRANSIENT)) {
            mAudioManager.abandonAudioFocusRequest(mfocusRequest);
        }
        if (mContext != null) {
            mContext.unregisterReceiver(mReceiver);
        }
    }
    private synchronized void initAudioRecordSink() {
        mAudioRecord = new AudioRecord(MediaRecorder.AudioSource.VOICE_COMMUNICATION,
                SAMPLE_RATE, CHANNEL_CONFIG_RECORD, AUDIO_FORMAT, RECORD_BUF_SIZE);
        Log.d(TAG," mAudioRecord initialized = " + mAudioRecord.getState());
        if (AutomaticGainControl.isAvailable()) {
            AutomaticGainControl agc = AutomaticGainControl.create(
                    mAudioRecord.getAudioSessionId());
            if (agc != null) {
                Log.d(TAG, "AGC is " + (agc.getEnabled() ? "enabled" : "disabled"));
                agc.setEnabled(true);
                Log.d(TAG, "AGC is " + (agc.getEnabled() ? "enabled" : "disabled"
                                        + " after trying to" + " enable"));
            }
        } else {
            Log.d(TAG, "AGC is unavailable");
        }

        if (NoiseSuppressor.isAvailable()) {
            NoiseSuppressor ns = NoiseSuppressor.create(mAudioRecord.getAudioSessionId());
            if (ns != null) {
                Log.d(TAG, "NS is " + (ns.getEnabled() ? "enabled" : "disabled"));
                ns.setEnabled(true);
                Log.d(TAG, "NS is " + (ns.getEnabled() ? "enabled" : "disabled"
                                       + " after trying to " + "disable"));
            }
        } else {
            Log.d(TAG, "NS is unavailable");
        }
        if (AcousticEchoCanceler.isAvailable()) {

            AcousticEchoCanceler aec = AcousticEchoCanceler.create(
                mAudioRecord.getAudioSessionId());
            if (aec != null) {
                Log.d(TAG, "AEC is " + (aec.getEnabled() ? "enabled" : "disabled"));
                aec.setEnabled(true);
                Log.d(TAG, "AEC is " + (aec.getEnabled() ? "enabled" : "disabled"
                        + " after trying to" + " disable"));
            }
        } else {
            Log.d(TAG, "aec is unavailable");
        }
        //AudioTrack(int, int, int, int, int, int)' is deprecated
        //use AudioTrack.Builder or
        //AudioTrack(AudioAttributes attributes, AudioFormat format, int bufferSizeInBytes, int mode, int sessionId)
        AudioManager am = (AudioManager)mContext.getSystemService(Context.AUDIO_SERVICE);
        AudioFormat audioFormat = (new AudioFormat.Builder())
                             .setChannelMask(CHANNEL_CONFIG_PLAYBACK)
                             .setEncoding(AudioFormat.ENCODING_PCM_16BIT)
                             .setSampleRate(SAMPLE_RATE)
                             .build();
        AudioAttributes audioAttributes = (new AudioAttributes.Builder())
                                .setUsage(AudioAttributes.USAGE_MEDIA)
                                .setContentType(AudioAttributes.CONTENT_TYPE_MUSIC)
                                .build();
        mAudioTrack = new AudioTrack(audioAttributes, audioFormat, PLAYBACK_BUF_SIZE,
                                        AudioTrack.MODE_STREAM, am.generateAudioSessionId());
    }

    public boolean startRecordAndPlay() {
        if (mHandler != null) {
            if (mCurrAudioFocusState == AudioManager.AUDIOFOCUS_LOSS_TRANSIENT) {
                Log.d(TAG, " Transient Loss , don't start now ");
                return false;
            }
            mHandler.obtainMessage(MSG_START_RECORD_PLAY).sendToTarget();
            return true;
        }
        return false;
    }

    public void stopRecordAndPlay() {
        if (mHandler != null) {
            if (mStrThread == null) {
                Log.d(TAG, " Streaming already stopped, bail out ");
                return;
            }
            mHandler.obtainMessage(MSG_STOP_RECORD_PLAY).sendToTarget();
        }
    }

    private class BAAudioReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {

            String action = intent.getAction();
            Log.d(TAG, action);
            int extraVal = 0;

            if(action == null)
                return;
            if (action.equals(BroadcastAudioAppActivity.ACTION_BAT_STATE_CHANGED)) {
                extraVal = intent.getIntExtra(BroadcastAudioAppActivity.EXTRA_STATE,
                        -1);
                if (extraVal != -1)
                    mCurrBATState = extraVal;
                Log.d(TAG, " ACTION_BAT_STATE_CHANGED mCurrBATState = " + mCurrBATState + " " +
                        "extraVal = "
                        + extraVal);
            }
            // Need to get BAT State for above class.
            if (action.equals(BroadcastAudioAppActivity.ACTION_BAT_STATE_CHANGED)) {
                final int state = intent.getIntExtra(BroadcastAudioAppActivity.EXTRA_STATE, -1);
                BAT_State = state;
            }
        }
    }

    private class StreamingThread extends Thread {
        public void run() {
            if ((mAudioRecord == null) || (mAudioTrack == null)) {
                initAudioRecordSink();
                Log.d(TAG, "mAudioRecord and mAudioTrack object created");
            }
            Log.d(TAG, " StreamThread Starts + ");
            mAudioTrack.setPlaybackRate(SAMPLE_RATE);
            byte[] buffer = new byte[RECORD_BUF_SIZE];
            mAudioRecord.startRecording();
            Log.d(TAG, "Audio Recording started");
            mAudioTrack.play();
            Log.d(TAG, "Audio Playing started");
            while (!Thread.interrupted()) {
                try {
                    int sizee = mAudioRecord.read(buffer, 0, RECORD_BUF_SIZE);
                    if (sizee != 0)
                        Log.d(TAG, "Reading first");
                    else
                        Log.d(TAG, "Something wrong");

                    mAudioTrack.write(buffer, 0, buffer.length);
                } catch (Throwable t) {
                    Log.d(TAG, "Read write failed- ERROR");
                    t.printStackTrace();
                    break;
                }
            }
            Log.d(TAG, " StreamThread Stops - ");
        }
    }

    private final class BAMsgHandler extends Handler {
        private BAMsgHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Log.v(TAG, " handleMessage what :"+msg.what);
            switch (msg.what) {

                case MSG_START_RECORD_PLAY:
                    Log.d(TAG, " Current Audio Focus = " + mCurrAudioFocusState);
                    if (mCurrAudioFocusState == AudioManager.AUDIOFOCUS_LOSS_TRANSIENT)
                        Log.d(TAG, " Transient Loss occurred, call must be in progress, don't "
                            + "start now ");
                    // requestAudioFocus (AudioManager.OnAudioFocusChangeListener l, int, int)
                    // method was deprecated in API level 26. use requestAudioFocus(AudioFocusRequest)
                    int focusGranted = mAudioManager.requestAudioFocus(mfocusRequest);
                    Log.d(TAG, " Focus Granted = " + focusGranted);
                    if (focusGranted == AudioManager.AUDIOFOCUS_REQUEST_GRANTED) {
                        if (mStrThread == null) {
                            mStrThread = new StreamingThread();
                            mStrThread.start();
                        }
                    }
                    break;

                case MSG_STOP_RECORD_PLAY:
                    if (mStrThread == null) {
                        Log.d(TAG, " Streaming thread null ");
                        break;
                    }
                    if (!mStrThread.isAlive()) {
                        Log.d(TAG, " Streaming thread not active ");
                        break;
                    }
                    mStrThread.interrupt();
                    try {
                        mStrThread.join();
                    } catch (InterruptedException e) {
                        Log.d(TAG, " Thread Interrupted while joining ");
                        e.printStackTrace();
                    }
                    if (mAudioRecord != null) {
                        mAudioRecord.stop();
                        mAudioRecord.release();
                        mAudioRecord = null;
                    }
                    if (mAudioTrack != null) {
                        mAudioTrack.stop();
                        mAudioTrack.release();
                        mAudioTrack = null;
                    }
                    mStrThread = null;
                    if (mCurrAudioFocusState == AudioManager.AUDIOFOCUS_GAIN) {
                        //abandonAudioFocus (AudioManager.OnAudioFocusChangeListener l) method was
                        //deprecated in API level 26. use abandonAudioFocusRequest(AudioFocusRequest)
                        mAudioManager.abandonAudioFocusRequest(mfocusRequest);
                    }
                    break;
                case MSG_AUDIO_FOCUS_CHANGE:
                    mCurrAudioFocusState = msg.arg1;
                    switch (mCurrAudioFocusState) {
                        case AudioManager.AUDIOFOCUS_GAIN:
                            if (mStrThread == null) {
                                mStrThread = new StreamingThread();
                                mStrThread.start();
                            }
                            break;
                        case AudioManager.AUDIOFOCUS_GAIN_TRANSIENT:
                            break;
                        case AudioManager.AUDIOFOCUS_LOSS:
                            // abandon audio focus in this case
                            mAudioManager.abandonAudioFocusRequest(mfocusRequest);
                            mHandler.obtainMessage(MSG_STOP_RECORD_PLAY).sendToTarget();
                            break;
                        case AudioManager.AUDIOFOCUS_LOSS_TRANSIENT:
                            mHandler.obtainMessage(MSG_STOP_RECORD_PLAY).sendToTarget();
                            break;
                    }
                    break;
            }
        }
    }
}