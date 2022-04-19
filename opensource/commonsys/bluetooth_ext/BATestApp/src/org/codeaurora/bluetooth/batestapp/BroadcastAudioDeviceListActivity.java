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

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.le.ScanRecord;
import android.bluetooth.le.ScanResult;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.Button;
import android.widget.ListView;

public class BroadcastAudioDeviceListActivity extends Activity
    implements View.OnClickListener, OnItemClickListener {

    private static final String TAG = BAAudio.TAG + " BAListActivity";
    GattBroadcastServiceClientHandler mGattBroadcastServiceClientHandler;
    boolean mIsAssociationInProgress = false;
    boolean mIsBRAEnabled = false;
    BroadcastAudioDevice mDevice;

    private ListView mLvDevices;
    private AdapterDevice mAdapterDevice;
    private List<BroadcastAudioDevice> mList;
    private Button mBtnBack;
    private boolean mIsCleanupCompleted = false;

    public static final int BRA_ENABLED_SUCCESS = 0;
    public static final int BRA_ENABLED_FAILED = 1;
    public static final int BRA_DISABLED_SUCCESS = 2;
    public static final int BRA_DISABLED_FAILED = 3;
    public static final int ASSOCIATE_BCA_RECEIVER_SUCCESS = 4;
    public static final int ASSOCIATE_BCA_RECEIVER_FAILED = 5;
    private static final int CONFIGURE_BROADCAST_RECEIVER_ASSOCIATION = 2;

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {

            final String action = intent.getAction();
            Log.d(TAG, " action " + action);

            if(action == null)
                return;
            if (action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {
                int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE, BluetoothAdapter
                        .ERROR);
                if (state == BluetoothAdapter.STATE_TURNING_OFF) {
                    cleanup();
                    finish();
                }
            } else if (action.equals(
                    BroadcastAudioAppActivity.ACTION_BAT_ONFOUND_ONLOST_BCA_RECEIVER)) {
                ScanResult result = (ScanResult) intent.getParcelableExtra(
                        BroadcastAudioAppActivity.EXTRA_SCAN_RESULT);
                boolean isFound = intent.getBooleanExtra(BroadcastAudioAppActivity.EXTRA_ONFOUND,
                        false);
                onFoundOnLostBCAReceiver(result, isFound);
            } else if (action.equals(
                    BroadcastAudioAppActivity.ACTION_BAT_ON_ASSOCIATED_BCA_RECEIVER)) {
                int status = intent.getIntExtra(BroadcastAudioAppActivity.EXTRA_STATUS, -1);
                BluetoothDevice dev = (BluetoothDevice) intent.getParcelableExtra(
                        BluetoothDevice.EXTRA_DEVICE);
                onAssociatedBCAReceiver(dev, status);
            } else if (action.equals(BroadcastAudioAppActivity.ACTION_BAT_BRA_STATE_CHANGED)) {
                int status = intent.getIntExtra(BroadcastAudioAppActivity.EXTRA_STATUS, -1);
                onConfiguredBroadcastReceiverAssociation(status);
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);
        setContentView(R.layout.layout_devices);

        mLvDevices = (ListView) findViewById(R.id.id_lv_deviceslist);
        mList = Collections.synchronizedList(new ArrayList<BroadcastAudioDevice>());
        mAdapterDevice = new AdapterDevice(mList, getLayoutInflater());
        mLvDevices.setAdapter(mAdapterDevice);
        mLvDevices.setOnItemClickListener(this);
        mBtnBack = (Button) findViewById(R.id.id_btn_back);
        mBtnBack.setOnClickListener(this);
        mBtnBack.setEnabled(true);

        IntentFilter filter = new IntentFilter(BluetoothAdapter.ACTION_STATE_CHANGED);
        filter.addAction(BroadcastAudioAppActivity.ACTION_BAT_BRA_STATE_CHANGED);
        filter.addAction(BroadcastAudioAppActivity.ACTION_BAT_ONFOUND_ONLOST_BCA_RECEIVER);
        filter.addAction(BroadcastAudioAppActivity.ACTION_BAT_ON_ASSOCIATED_BCA_RECEIVER);
        registerReceiver(mReceiver, filter);

        HandlerThread thread = new HandlerThread("GattBroadcastServiceClientHandler");
        thread.start();
        Looper looper = thread.getLooper();

        mGattBroadcastServiceClientHandler = new GattBroadcastServiceClientHandler(looper);
        mGattBroadcastServiceClientHandler.sendMessage(
                mGattBroadcastServiceClientHandler.obtainMessage(
                        CONFIGURE_BROADCAST_RECEIVER_ASSOCIATION, true));
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        Log.v(TAG, "onDestroy: mReceiver" + mReceiver + " mIsCleanupCompleted: "
                + mIsCleanupCompleted);
        if (!mIsCleanupCompleted){
            cleanup();
        }
        if (mReceiver != null) {
            unregisterReceiver(mReceiver);
        }
    }

    private void cleanup() {
        Log.v(TAG, " cleanup: mIsBRAEnabled: " + mIsBRAEnabled
                + " mGattBroadcastServiceClientHandler: " + mGattBroadcastServiceClientHandler);
        if (mGattBroadcastServiceClientHandler != null) {
            if (mIsBRAEnabled) {
                mBtnBack.setEnabled(false);
                mGattBroadcastServiceClientHandler.sendMessage(
                        mGattBroadcastServiceClientHandler.obtainMessage(
                                CONFIGURE_BROADCAST_RECEIVER_ASSOCIATION,
                                false));
            }
            mGattBroadcastServiceClientHandler.close();
            mGattBroadcastServiceClientHandler.removeCallbacksAndMessages(null);
            Looper looper = mGattBroadcastServiceClientHandler.getLooper();
            if (looper != null) {
                looper.quit();
                Log.i(TAG, "Quit looper");
            }
            mGattBroadcastServiceClientHandler = null;
            Log.i(TAG, "Remove Handler");
            mIsCleanupCompleted = true;
        }
    }

    @Override
    public void onClick(View view) {
        Log.v(TAG, " onClick ");
        cleanup();
        finish();
    }

    @Override
    public void onItemClick(AdapterView<?> arg0, View arg1, int arg2, long arg3) {
        BroadcastAudioDevice device = mList.get(arg2);
        String msg = getString(R.string.msg_association_initiated, device.getName());
        if (mIsAssociationInProgress) {
            Log.d(TAG, "Association is in progress ignore click");
            return;
        }
        if (mGattBroadcastServiceClientHandler != null) {
            Log.d(TAG, " sendMessage ASSOCIATE_BCA_RECEIVER" );
            mGattBroadcastServiceClientHandler.sendMessage(
                    mGattBroadcastServiceClientHandler.obtainMessage(
                            GattBroadcastServiceClientHandler.ASSOCIATE_BCA_RECEIVER,
                            device.getBluetoothDevice()));
            mIsAssociationInProgress = true;
            mDevice = device;
        } else {
            Log.e(TAG, "mGattBroadcastServiceClientHandler is null");
        }
    }

    private synchronized void addOrRemoveBCAReceiverDevice(BroadcastAudioDevice device,
            boolean isAdd) {
        boolean isFound = false;
        BluetoothDevice bDevice = device.getBluetoothDevice();
        Log.d(TAG, " addOrRemoveBCAReceiverDevice: isAdd: " + isAdd);
        for (BroadcastAudioDevice BADev : mList) {
            if (bDevice.equals(BADev.getBluetoothDevice())) {
                isFound = true;
                if (!isAdd) {
                    Log.d(TAG, "addOrRemoveBCAReceiverDevice Device removed :"
                            + BADev.getName());
                    mList.remove(BADev);
                }
                break;
            }
        }
        if (isAdd && !isFound) {
            Log.d(TAG, " New Device Added :" + device.getName());
            mList.add(device);
        }
    }

    private synchronized void refreshDevices() {
        Log.d(TAG, " refreshDevices size:" + mList.size());
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                mAdapterDevice.refreshDevices(mList);
            }
        });
    }

    public void onFoundOnLostBCAReceiver(ScanResult result, boolean isFound) {
        ScanRecord record = result.getScanRecord();
        String mac = result.getDevice().getAddress();
        Log.d(TAG, "onFoundOnLostBCAReceiver: " + isFound + " mac :" + mac + "record :" + record);
        BroadcastAudioDevice device = new BroadcastAudioDevice(result.getDevice(),
                result.getScanRecord());
        addOrRemoveBCAReceiverDevice(device, isFound);
        refreshDevices();
    }

    public void onConfiguredBroadcastReceiverAssociation(int status) {
        Log.d(TAG, "onConfiguredBroadcastReceiverAssociation status: " + status);
        if (status == BRA_ENABLED_SUCCESS) {
            mIsBRAEnabled = true;
        } else if (status == BRA_DISABLED_SUCCESS) {
            mIsBRAEnabled = false;
        } else if (status == BRA_ENABLED_FAILED) {
            finish();
        } else if (status == BRA_DISABLED_FAILED) {
            mIsBRAEnabled = false;
        }
    }

    public void onAssociatedBCAReceiver(BluetoothDevice device, int status) {
        Log.d(TAG, "onAssociatedBCAReceiver: status " + status);
        if (mDevice == null){
            Log.d(TAG, "onAssociatedBCAReceiver: Nothing to show on UI, mDevice is null");
            return;
        }

        if (status == ASSOCIATE_BCA_RECEIVER_SUCCESS) {
            String msg = getString(R.string.msg_association_success, mDevice.getName());
            List<BroadcastAudioDevice> tempList =
                    new ArrayList<BroadcastAudioDevice>(mList);
            for (BroadcastAudioDevice BADev : tempList) {
                if (device.equals(BADev.getBluetoothDevice())) {
                    addOrRemoveBCAReceiverDevice(BADev, false);
                }
            }
            refreshDevices();
        } else if (status == ASSOCIATE_BCA_RECEIVER_FAILED) {
            String msg = getString(R.string.msg_association_failed, mDevice.getName());
        }
        mIsAssociationInProgress = false;
    }

    private class GattBroadcastServiceClientHandler extends Handler {
        public static final int ASSOCIATE_BCA_RECEIVER = 3;

        private GattBroadcastServiceClientHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG, "Handler(): got msg=" + msg.what);

            switch (msg.what) {
                case CONFIGURE_BROADCAST_RECEIVER_ASSOCIATION:
                    Boolean enable = (Boolean) msg.obj;
                    if(enable) {
                        Intent intent_to_configure_BRA = new Intent(
                                BroadcastAudioAppActivity.ACTION_BAT_BRA_ENABLE);
                        sendBroadcast(intent_to_configure_BRA);
                    } else {
                        Intent intent_to_configure_BRA = new Intent(
                                BroadcastAudioAppActivity.ACTION_BAT_BRA_DISABLE);
                        sendBroadcast(intent_to_configure_BRA);
                    }
                    break;
                case ASSOCIATE_BCA_RECEIVER:
                    BluetoothDevice device = (BluetoothDevice) msg.obj;
                    Intent intent_to_associate_BCA_Receiver = new Intent(
                            BroadcastAudioAppActivity.ACTION_BAT_ASSOCIATE_BCA_RECEIVER);
                    intent_to_associate_BCA_Receiver.putExtra(
                            BluetoothDevice.EXTRA_DEVICE, device);
                    sendBroadcast(intent_to_associate_BCA_Receiver);
                    break;
                default:
                    break;
            }
        }

        void close() {
            Log.v(TAG, " close ");
        }
    }
}
