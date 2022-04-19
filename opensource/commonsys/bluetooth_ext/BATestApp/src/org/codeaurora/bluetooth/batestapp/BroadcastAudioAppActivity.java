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

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Handler;
import android.os.Message;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.ToggleButton;
import android.widget.Toast;

public class BroadcastAudioAppActivity extends Activity implements
    CompoundButton.OnCheckedChangeListener, View.OnClickListener {

    public static final String ACTION_BAT_BA_ENABLE =
            "com.android.bluetooth.bat.profile.action.BA_ENABLE";
    public static final String ACTION_BAT_BA_DISABLE =
            "com.android.bluetooth.bat.profile.action.BA_DISABLE";
    public static final String ACTION_BAT_BRA_ENABLE =
            "com.android.bluetooth.bat.profile.action.BRA_ENABLE";
    public static final String ACTION_BAT_BRA_DISABLE =
            "com.android.bluetooth.bat.profile.action.BRA_DISABLE";
    public static final String ACTION_BAT_ENCRYPTION_KEY_REFRESH =
            "com.android.bluetooth.bat.profile.action.ENCRYPTION_KEY_REFRESH";
    public static final String ACTION_BAT_ASSOCIATE_BCA_RECEIVER =
            "com.android.bluetooth.bat.profile.action.ASSOCIATE_BCA_RECEIVER";
    public static final String ACTION_BAT_STATE_CHANGED =
            "com.android.bluetooth.bat.profile.action.BA_STATE_CHANGED";
    public static final String EXTRA_STATE =
            "com.android.bluetooth.bat.profile.extra.STATE";
    public static final String EXTRA_PREVIOUS_STATE =
            "com.android.bluetooth.bat.profile.extra.PREV_STATE";
    public static final String ACTION_BAT_ONFOUND_ONLOST_BCA_RECEIVER =
            "com.android.bluetooth.bat.profile.action.ONFOUND_ONLOST_BCA_RECEIVER";
    public static final String EXTRA_SCAN_RESULT =
            "com.android.bluetooth.bat.profile.extra.EXTRA_SCAN_RESULT";
    public static final String EXTRA_ONFOUND =
            "com.android.bluetooth.bat.profile.extra.EXTRA_ONFOUND";
    public static final String ACTION_BAT_BRA_STATE_CHANGED =
            "com.android.bluetooth.bat.profile.action.BRA_STATE_CHANGED";
    public static final String EXTRA_STATUS =
            "com.android.bluetooth.bat.profile.extra.STATUS";
    public static final String ACTION_BAT_ON_ASSOCIATED_BCA_RECEIVER =
            "com.android.bluetooth.bat.profile.action.ON_ASSOCIATED_BCA_RECEIVER";
    public static final String ACTION_BAT_ENCRYPTION_KEY_CHANGED =
            "com.android.bluetooth.bat.profile.action.BA_ENC_KEY_CHANGED";
    public static final String EXTRA_ECNRYPTION_KEY =
            "com.android.bluetooth.bat.profile.extra.ENC_KEY";

    public static final int STATE_DISABLED = 0;
    public static final int STATE_PAUSED = 1;
    public static final int STATE_PLAYING = 2;

    private static final String TAG = BAAudio.TAG + "BAAppActivity";
    BAAudio mBAAudiobj = null;
    private ToggleButton mBAEnable, mMegaPhoneEnable;
    private BluetoothAdapter mBtAdapter;
    private Button mBtnBRAEnable, mBtnChangeEnc;
    private boolean isStateChangePending = false;
    private int expectedState = STATE_DISABLED;
    private int BATState = STATE_DISABLED;

    // Implementation of 30 sec Timeout for BT
    private static final int BA_ENABLE_TIMEOUT = 0;
    private static final int BA_ENABLE_TIMEOUT_VALUE = 30000;

    private void onTimeout() {
        mTimeoutHandler.sendMessageDelayed(mTimeoutHandler.obtainMessage(BA_ENABLE_TIMEOUT),
                BA_ENABLE_TIMEOUT_VALUE);
    }

    private final Handler mTimeoutHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            switch (msg.what) {
                case BA_ENABLE_TIMEOUT:
                    Context context = getApplicationContext();
                    Toast toast = Toast.makeText(context, "BA Not enabled properly", Toast.LENGTH_SHORT);
                    toast.show();
                    Log.d(TAG, "BA not enabled properly");
                    isStateChangePending = false;
                    break;
                default:
                    break;
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        Log.d(TAG, "onCreate ");
        super.onCreate(savedInstanceState);

        setContentView(R.layout.layout_main);
        mBAEnable = (ToggleButton) findViewById(R.id.id_switch_bt_enable);
        mBAEnable.setOnCheckedChangeListener(this);
        mMegaPhoneEnable = (ToggleButton) findViewById(R.id.id_switch_mp);
        mMegaPhoneEnable.setOnCheckedChangeListener(this);
        mBtnChangeEnc = (Button) findViewById(R.id.id_btn_enc_change);
        mBtnChangeEnc.setOnClickListener(this);
        mBtnBRAEnable = (Button) findViewById(R.id.id_btn_enable_bra);
        mBtnBRAEnable.setOnClickListener(this);
        mBtnBRAEnable.setEnabled(false);
        mBtAdapter = BluetoothAdapter.getDefaultAdapter();

        if (mBtAdapter == null) {
            Log.d(TAG, " Device does not support Bluetooth");
            return;
        }
        boolean isBtEnabled = mBtAdapter.isEnabled();

        IntentFilter filter = new IntentFilter(BluetoothAdapter.ACTION_STATE_CHANGED);
        filter.addAction(ACTION_BAT_STATE_CHANGED);
        filter.addAction(BAAudio.BASERVICE_STATE_CHANGED);
        registerReceiver(mReceiver, filter);

        mBAAudiobj = new BAAudio(getApplicationContext());
    }

    @Override
    protected void onDestroy() {
        Log.d(TAG, " onDestroy ");
        if (mBAAudiobj != null) {
            mBAAudiobj.cleanup();
        }
        super.onDestroy();
        unregisterReceiver(mReceiver);
    }

    @Override
    public void onCheckedChanged(CompoundButton compoundButton, boolean isChecked) {

        Log.d(TAG, " onCheckChanged isChecked = " + isChecked + " button = " +
                compoundButton + " stateChangePending " + isStateChangePending);

        if (mBtAdapter == null || !mBtAdapter.isEnabled() || mBAAudiobj == null)
            return;

        if (compoundButton == mBAEnable) {
            if (isStateChangePending)
                return;
            if (isChecked) {
                if(BATState == STATE_DISABLED) {
                    isStateChangePending = true;
                    expectedState = STATE_PAUSED;
                    mBAEnable.setEnabled(false);
                    Intent intent_to_enable_BA = new Intent(ACTION_BAT_BA_ENABLE);
                    sendBroadcast(intent_to_enable_BA);
                    onTimeout();
                }
            } else {
               if(BATState != STATE_DISABLED) {
                   isStateChangePending = true;
                   expectedState = STATE_DISABLED;
                   mBAEnable.setEnabled(false);
                   Intent intent_to_disable_BA = new Intent(ACTION_BAT_BA_DISABLE);
                   sendBroadcast(intent_to_disable_BA);
               }
            }
        } else if (compoundButton == mMegaPhoneEnable) {
            if (isChecked) {
                boolean started = mBAAudiobj.startRecordAndPlay();
                setMPButtonText(started);
            } else {
               mBAAudiobj.stopRecordAndPlay();
               setMPButtonText(false);
            }
        }
    }

    @Override
    public void onClick(View view) {
        if (view == mBtnBRAEnable) {
            startActivity(new Intent(BroadcastAudioAppActivity.this,
                    BroadcastAudioDeviceListActivity.class));
        } else if (view == mBtnChangeEnc) {
            Intent intent_to_refresh_EncKey = new Intent(ACTION_BAT_ENCRYPTION_KEY_REFRESH);
            sendBroadcast(intent_to_refresh_EncKey);
        }
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            final String action = intent.getAction();
            Log.d(TAG, " Action " + action);

            if (action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {
                int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter
                        .ERROR);
                if (state == BluetoothAdapter.STATE_TURNING_OFF
                    || state == BluetoothAdapter.STATE_OFF) {
                    setBRAButtonState(STATE_DISABLED);
                }
            }

            if (action.equals(ACTION_BAT_STATE_CHANGED)) {
                final int state = intent.getIntExtra(EXTRA_STATE, -1);
                BATState = state;
                if(state != -1) {
                    setBRAButtonState(state);
                    mTimeoutHandler.removeCallbacksAndMessages(null);
                    Log.d(TAG, " stateChangePending " + isStateChangePending + " expectedState = "
                            + expectedState + " state " + state);
                    if (isStateChangePending) {
                        isStateChangePending = false;
                        mBAEnable.setEnabled(true);
                        if (expectedState != state) {
                            setBAButtonText(state);
                        }
                    } else {
                        // this must have come from state change in stack
                        setBAButtonText(state);
                    }
                }
            }
            if (action.equals(BAAudio.BASERVICE_STATE_CHANGED)) {
                final boolean serviceState = intent.getBooleanExtra(BAAudio.EXTRA_CONN_STATE,
                        false);
                final int baState = intent.getIntExtra(BAAudio.EXTRA_BA_STATE, -1);
                Log.d(TAG, " Service state changed serviceState = "
                        + serviceState + " baState = " + baState);
                setBAButtonText(baState);
            }
        }
    };

    private void setMPButtonText(boolean state) {
        Log.d(TAG," setMPButtonText state = " + state);
        if(state) {
            mMegaPhoneEnable.setChecked(true);
        } else {
            mMegaPhoneEnable.setChecked(false);
        }
    }

    private void setBAButtonText(int state) {
        boolean currentCheckStatus = mBAEnable.isChecked();
        Log.d(TAG," setBAButtonText state = " + state + " isChecked() = " + currentCheckStatus);
        if(state == STATE_DISABLED) {
            if (currentCheckStatus) {
                // button was checked and we are making it false, need to set it
                Log.d(TAG, " setting button false ");
                mBAEnable.setChecked(false);
            }
        } else {
            if (!currentCheckStatus) {
               Log.d(TAG, " setting button true ");
               mBAEnable.setChecked(true);
            }
        }
    }

    private void setBRAButtonState(int state) {
        Log.d(TAG, "setBRAButtonState state : "+state);
        if(state == STATE_DISABLED) {
            mBtnBRAEnable.setEnabled(false);
        } else {
            mBtnBRAEnable.setEnabled(true);
        }
    }
}