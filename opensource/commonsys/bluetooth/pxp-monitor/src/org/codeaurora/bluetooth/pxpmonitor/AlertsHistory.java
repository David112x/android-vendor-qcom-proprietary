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

package org.codeaurora.bluetooth.pxpmonitor;

import android.app.Activity;
import android.app.ListActivity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.Iterator;

public class AlertsHistory extends ListActivity {

    private static final String TAG = AlertsHistory.class.getSimpleName();

    private static final int REQUEST_ENABLE_BT = 1;

    private static AlertHistoryAdapter mAlertHistoryAdapter = null;

    private BluetoothAdapter mBluetoothAdapter = null;

    private PxpServiceProxy mPxpServiceProxy = null;

    private final ServiceConnection mConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {
            mPxpServiceProxy = ((PxpServiceProxy.LocalBinder) service).getService();

            if (mPxpServiceProxy == null) {
                return;
            }
            listAlerts();
        }

        @Override
        public void onServiceDisconnected(ComponentName className) {
            mPxpServiceProxy = null;
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getActionBar().setTitle("Alert History");

        Log.d(TAG, "On create: new intent BluetoothService");
        Intent gattServiceIntent = new Intent(this, PxpServiceProxy.class);

        if (bindService(gattServiceIntent, mConnection, BIND_AUTO_CREATE) == false) {
            Log.e(TAG, "Unable to bind");
        }

        final BluetoothManager bluetoothManager =
                (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        mBluetoothAdapter = bluetoothManager.getAdapter();

        mAlertHistoryAdapter = new AlertHistoryAdapter(this);
        setListAdapter(mAlertHistoryAdapter);
    }

    @Override
    protected void onResume() {
        super.onResume();

        if (!mBluetoothAdapter.isEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
        }

        if (mPxpServiceProxy != null) {
            mAlertHistoryAdapter.clear();
            listAlerts();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        unbindService(mConnection);
        mPxpServiceProxy = null;
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (requestCode == REQUEST_ENABLE_BT && resultCode == Activity.RESULT_CANCELED) {
            finish();
            return;
        }

        if (mPxpServiceProxy != null) {
            listAlerts();
        }
    }

    private void listAlerts() {
        Iterator<AlertInformation> iterator =
                mPxpServiceProxy.mAlertHistory.iterator();

        while (iterator.hasNext()) {
            mAlertHistoryAdapter.addAlert(iterator.next());
            mAlertHistoryAdapter.notifyDataSetChanged();
        }

    }

    private class AlertHistoryAdapter extends BaseAdapter {

        private ArrayList<AlertInformation> mAlertInf;

        private LayoutInflater mInflater;

        private TextView mDeviceName;

        private TextView mDeviceAddress;

        private TextView mTime;

        private TextView mRssi;

        public AlertHistoryAdapter(Context context) {
            super();

            mAlertInf = new ArrayList<AlertInformation>();
            mInflater = LayoutInflater.from(context);
        }

        public void addAlert(AlertInformation alert) {
            if (!mAlertInf.contains(alert)) {
                mAlertInf.add(alert);
            }
        }

        public void clear() {
            mAlertInf.clear();
            this.notifyDataSetChanged();
        }

        @Override
        public int getCount() {
            return mAlertInf.size();
        }

        @Override
        public Object getItem(int i) {
            return mAlertInf.get(i);
        }

        @Override
        public long getItemId(int i) {
            return i;
        }

        @Override
        public View getView(int i, View view, ViewGroup viewGroup) {

            if (view == null) {
                view = mInflater.inflate(R.layout.activity_alerts_history, null);
            }

            mDeviceName = (TextView) view.findViewById(R.id.device_name);
            mDeviceAddress = (TextView) view.findViewById(R.id.device_address);
            mTime = (TextView) view.findViewById(R.id.alert_time);
            mRssi = (TextView) view.findViewById(R.id.device_rssi);

            Log.e(TAG, "i: " + i);

            AlertInformation alert = mAlertInf.get(i);

            final String deviceName = alert.device.getName();

            if (deviceName != null && deviceName.length() > 0) {
                mDeviceName.setText(deviceName);

            } else {
                mDeviceName.setText("Unknow device");
            }

            mDeviceAddress.setText(alert.device.getAddress());

            mTime.setText(alert.date);
            mRssi.setText("RSSI = " + String.valueOf(alert.rssi) + "dBm");

            return view;
        }
    }
}
