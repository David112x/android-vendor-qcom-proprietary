/*
 * Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *        * Redistributions of source code must retain the above copyright
 *            notice, this list of conditions and the following disclaimer.
 *        * Redistributions in binary form must reproduce the above copyright
 *            notice, this list of conditions and the following disclaimer in the
 *            documentation and/or other materials provided with the distribution.
 *        * Neither the name of The Linux Foundation nor
 *            the names of its contributors may be used to endorse or promote
 *            products derived from this software without specific prior written
 *            permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.    IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package org.codeaurora.bluetooth.bttestapp;

import android.bluetooth.BluetoothAdapter;
import android.app.Activity;
import android.bluetooth.BluetoothA2dp;
import android.bluetooth.BluetoothA2dpSink;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.SdpMasRecord;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.BluetoothUuid;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.content.pm.ActivityInfo;
import android.os.Bundle;
import android.os.IBinder;
import android.os.Parcelable;
import android.os.ParcelUuid;
import android.os.SystemProperties;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.ListView;
import android.widget.ToggleButton;
import android.widget.TextView;
import android.widget.Toast;

import org.codeaurora.bluetooth.bttestapp.R;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class MainActivity extends Activity {

    private final String TAG = "BtTestMainActivity";

    private static final String PREF_DEVICE = "device";
    private static final String PREF_SERVICES = "services";

    private static final String PICKER_ACTION = "android.bluetooth.devicepicker.action.LAUNCH";
    private static final String PICKER_SELECTED =
            "android.bluetooth.devicepicker.action.DEVICE_SELECTED";

    private BluetoothDevice mDevice;

    private boolean mIsBound = false;

    private boolean mDiscoveryInProgress = false;
    private BluetoothAdapter mBtAdapter;
    private Button mBtnDiscoverService, mBtnSelectDevice, mSinkButton, mSourceButton, mAvrcpButton;
    private static long current_time, switch_time;
    private ListView mLvServices;
    private ArrayList<String> mListUuid = new ArrayList<String>();

    private final String UUID_AVRCP_TARGET = "0000110C-0000-1000-8000-00805F9B34FB";
    private final String UUID_AUDIO_SOURCE = "0000110A-0000-1000-8000-00805F9B34FB";
    private final String UUID_AUDIO_SINK = "0000110B-0000-1000-8000-00805F9B34FB";
    private final String KEY_A2DP_SINK = "persist.vendor.service.bt.a2dp.sink";

    private final BroadcastReceiver mPickerReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            Log.v(TAG, "mPickerReceiver got " + action);

            if (PICKER_SELECTED.equals(action)) {
                BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                updateDevice(device);
                unregisterReceiver(this);

            }
        }
    };

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            Log.v(TAG, "mReceiver got " + action);

            if (BluetoothDevice.ACTION_UUID.equals(action)) {
                BluetoothDevice dev = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                if (!dev.equals(mDevice)) {
                    Log.i(TAG," some other device ");
                    return;
                }

                Parcelable uuids[] = intent.getParcelableArrayExtra(BluetoothDevice.EXTRA_UUID);

                if (uuids != null) {
                    for (Parcelable uuid : uuids) {
                         Log.v(TAG, "Received UUID: " + uuid.toString());
                       if (UUID_AVRCP_TARGET.equalsIgnoreCase(uuid.toString()) ||
                               UUID_AUDIO_SOURCE.equalsIgnoreCase(uuid.toString()) ||
                               UUID_AUDIO_SINK.equalsIgnoreCase(uuid.toString())) {
                            mAvrcpButton.setEnabled(true);
                        }
                    }
             }
           } else if (action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {
               final int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE,
                       BluetoothAdapter.ERROR);
               Log.d(TAG, " Action " + action + " state :" + state);
               setBTState();
               if (state == BluetoothAdapter.STATE_OFF) {
                   if (!mBtAdapter.isEnabled()) {
                       Log.d(TAG, " Enabling BT... ");
                       mBtAdapter.enable();
                   }
               } else if (state == BluetoothAdapter.STATE_ON) {
                   current_time = System.currentTimeMillis();
                   Log.d(TAG, "Time for BT OFF->ON : " + (current_time - switch_time) + " ms");
                   mSinkButton.setEnabled(true);
                   mSourceButton.setEnabled(true);
               }

           } else if (action.equals(BluetoothA2dpSink.ACTION_CONNECTION_STATE_CHANGED)) {
               int newState = intent.getIntExtra(BluetoothProfile.EXTRA_STATE, -1);
               Log.d(TAG, " Action " + action + ", new A2DP Sink State :" + newState);
               if (newState == BluetoothProfile.STATE_CONNECTED) {
                   Log.d(TAG, " BT-OFF to reconnection time = " +
                       (System.currentTimeMillis() - switch_time) +"ms");
               }
           } else if (action.equals(BluetoothA2dp.ACTION_CONNECTION_STATE_CHANGED)) {
               int newState = intent.getIntExtra(BluetoothProfile.EXTRA_STATE, -1);
               Log.d(TAG, " Action " + action + ", new A2DP Source State :" + newState);
               if (newState == BluetoothProfile.STATE_CONNECTED) {
                   Log.d(TAG, " BT-OFF to reconnection time = " +
                       (System.currentTimeMillis() - switch_time) +"ms");
               }
           }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Log.v(TAG, "onCreate");

        setRequestedOrientation(ActivityInfo.SCREEN_ORIENTATION_SENSOR_PORTRAIT);
        setContentView(R.layout.activity_main);
    }

    @Override
    protected void onStart() {
        super.onStart();

        Log.v(TAG, "onStart");

        String addr = getPreferences(MODE_PRIVATE).getString(PREF_DEVICE, null);

        try {
            BluetoothDevice dev = BluetoothAdapter.getDefaultAdapter().getRemoteDevice(addr);
            updateDevice(dev);
        } catch (IllegalArgumentException e) {
            // just leave device unset
        }
    }

    @Override
    protected void onResume() {
        super.onResume();

        Log.v(TAG, "onResume");

        IntentFilter filter = new IntentFilter();
        filter.addAction(BluetoothDevice.ACTION_UUID);
        filter.addAction(BluetoothDevice.ACTION_SDP_RECORD);
        filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
        filter.addAction(BluetoothA2dpSink.ACTION_CONNECTION_STATE_CHANGED);
        filter.addAction(BluetoothA2dp.ACTION_CONNECTION_STATE_CHANGED);
        registerReceiver(mReceiver, filter);

        if (BluetoothAdapter.getDefaultAdapter().isEnabled() == false) {
            Button b = (Button) findViewById(R.id.discover_services);
            b.setEnabled(false);

            b = (Button) findViewById(R.id.select_device);
            b.setEnabled(false);
        }
        mBtnDiscoverService = (Button) findViewById(R.id.discover_services);
        mBtnSelectDevice = (Button) findViewById(R.id.select_device);
        mSinkButton = (Button) findViewById(R.id.id_a2dp_sink);
        mSourceButton = (Button) findViewById(R.id.id_a2dp_source);
        mAvrcpButton = (Button) findViewById(R.id.id_btn_show_avrcp);
        mLvServices =(ListView)findViewById(R.id.id_lv_services);
        mBtAdapter = BluetoothAdapter.getDefaultAdapter();
        setBTState();
    }

    @Override
    protected void onPause() {
        super.onPause();
        Log.v(TAG, "onPause");
        unregisterReceiver(mReceiver);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.v(TAG, "onDestroy");
    }

    public void onButtonClick(View v) {
        if (v.getId() == R.id.select_device) {
            IntentFilter filter = new IntentFilter();
            filter.addAction(PICKER_SELECTED);
            registerReceiver(mPickerReceiver, filter);

            Intent intent = new Intent(PICKER_ACTION);
            intent.setFlags(Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
            startActivity(intent);

        } else if (v.getId() == R.id.discover_services) {
            if (mDevice == null || mDiscoveryInProgress) {
                return;
            }
            Log.v(TAG, "fetching UUIDs");
            mDiscoveryInProgress = mDevice.fetchUuidsWithSdp();
        }
    }

    public void showCoveArtActivity(View v) {
        Log.i(TAG," showCoveArtActivity");
        startActivity(new Intent(this,AvrcpCoverArtActivity.class));
    }

    public void showAvrcpActivity(View v) {
        Log.i(TAG," showAvrcpActivity");
        startActivity(new Intent(this,AvrcpTestActivity.class));
    }

    public void showHidHost(View v) {
        Log.i(TAG," showHidHost");
        startActivity(new Intent(this, HidTestApp.class));
    }


    private void updateDevice(BluetoothDevice device) {
        SharedPreferences.Editor prefs = getPreferences(MODE_PRIVATE).edit();

        TextView name = (TextView) findViewById(R.id.device_name);
        TextView addr = (TextView) findViewById(R.id.device_address);

        if (device == null) {
            name.setText(R.string.blank);
            addr.setText(R.string.blank);

            prefs.remove(PREF_DEVICE);
        } else {
            Intent intent = new Intent(BluetoothConnectionReceiver.ACTION_NEW_BLUETOOTH_DEVICE);
            intent.putExtra(BluetoothConnectionReceiver.EXTRA_DEVICE_ADDRESS, device.getAddress());
            sendBroadcast(intent);

            name.setText(device.getName());
            addr.setText(device.getAddress());

            prefs.putString(PREF_DEVICE, device.getAddress());
        }

        prefs.commit();

        mDevice = device;

    }

    public void onRadioButtonClicked(View v) {
        switch_time = System.currentTimeMillis();
        boolean isA2dpSinkEnabled = SystemProperties.getBoolean(KEY_A2DP_SINK, false);

        // Switch role to A2DP Source
        if (v.getId() == R.id.id_a2dp_source) {
            if (!isA2dpSinkEnabled) {
                Log.d(TAG, "Already in Source role, ignore user action ignored");
                return;
            }
            Log.d(TAG, "Switch role to A2DP Source");
            SystemProperties.set(KEY_A2DP_SINK, false + "");

        // Switch role to A2DP Sink
        } else if (v.getId() == R.id.id_a2dp_sink) {
            if (isA2dpSinkEnabled) {
                Log.d(TAG, "Already in A2DP Sink role, user action ignored");
                return;
            }
            Log.d(TAG, "Switch role to A2DP Sink");
            SystemProperties.set(KEY_A2DP_SINK, true + "");
        }
        mSinkButton.setEnabled(false);
        mSourceButton.setEnabled(false);

        // Turn BT OFF and ON in order to reflect property change
        if (mBtAdapter.isEnabled())
            mBtAdapter.disable();
        else
            mBtAdapter.enable();
    }

    private void setBTState() {
        boolean isBtEnabled = mBtAdapter.isEnabled();
        if (isBtEnabled) {
            mBtnDiscoverService.setEnabled(true);
            mBtnSelectDevice.setEnabled(true);
        } else {
            mBtnDiscoverService.setEnabled(false);
            mBtnSelectDevice.setEnabled(false);
        }
    }
}
