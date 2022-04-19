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

import android.app.ListActivity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
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
import android.widget.ListView;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.Set;

public class PairedDevicesActivity extends ListActivity {

    private static final String TAG = PairedDevicesActivity.class.getSimpleName();

    private static final int REQUEST_ENABLE_BT = 1;

    public static final String DEVICE_PAIRED_SELECTED =
            "android.bluetooth.devicepicker.action.DEVICE_PAIRED_SELECTED";

    private PairedDevicesListAdapter mPairedDeviceListAdapter;

    private BluetoothAdapter mAdapter = null;

    private PxpServiceProxy mPxpServiceProxy = null;

    private final ServiceConnection mConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {
            Log.d(TAG, "Connecting to service");
            mPxpServiceProxy = ((PxpServiceProxy.LocalBinder) service).getService();
        }

        @Override
        public void onServiceDisconnected(ComponentName className) {
            mPxpServiceProxy = null;
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getActionBar().setTitle("Paired Devices");

        Log.d(TAG, "On create: new intent BluetoothService");
        Intent gattServiceIntent = new Intent(this, PxpServiceProxy.class);

        if (bindService(gattServiceIntent, mConnection, BIND_AUTO_CREATE) == false) {
            Log.e(TAG, "Unable to bind");
        }

        final BluetoothManager bluetoothManager =
                (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        mAdapter = bluetoothManager.getAdapter();

        mPairedDeviceListAdapter = new PairedDevicesListAdapter(this);
        setListAdapter(mPairedDeviceListAdapter);
        getPairedDevices();
    }

    @Override
    protected void onResume() {
        super.onResume();

        if (!mAdapter.isEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
        }

        mPairedDeviceListAdapter.clear();
        getPairedDevices();
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (requestCode == REQUEST_ENABLE_BT) {
            getPairedDevices();
        }
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {

        final BluetoothDevice leDevice = mPairedDeviceListAdapter.getDevice(position);

        if (leDevice == null) {
            return;
        }

        Log.d(TAG, "Trying to connect to paired device :" + leDevice.getAddress());

        final Intent intent = new Intent(this, DeviceActivity.class);
        intent.setAction(DEVICE_PAIRED_SELECTED);
        intent.putExtra(DeviceActivity.EXTRAS_DEVICE, leDevice);
        startActivity(intent);
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        unbindService(mConnection);
        mPxpServiceProxy = null;
    }

    private void getPairedDevices() {
        Set<BluetoothDevice> devices = mAdapter.getBondedDevices();

        for (BluetoothDevice device : devices) {
            mPairedDeviceListAdapter.addDevice(device);
            mPairedDeviceListAdapter.notifyDataSetChanged();
        }
    }

    private class PairedDevicesListAdapter extends BaseAdapter {

        private ArrayList<BluetoothDevice> mLeDevices;

        private LayoutInflater mInflater;

        private TextView mDeviceName;

        private TextView mDeviceAddress;

        public PairedDevicesListAdapter(Context context) {
            super();
            mLeDevices = new ArrayList<BluetoothDevice>();
            mInflater = LayoutInflater.from(context);
        }

        public void addDevice(BluetoothDevice device) {
            if (!mLeDevices.contains(device)) {
                mLeDevices.add(device);
            }
        }

        public BluetoothDevice getDevice(int position) {
            return mLeDevices.get(position);
        }

        public void clear() {
            mLeDevices.clear();
            this.notifyDataSetChanged();
        }

        @Override
        public int getCount() {
            return mLeDevices.size();
        }

        @Override
        public Object getItem(int i) {
            return mLeDevices.get(i);
        }

        @Override
        public long getItemId(int i) {
            return i;
        }

        @Override
        public View getView(int i, View view, ViewGroup viewGroup) {

            if (view == null) {
                view = mInflater.inflate(R.layout.activity_list_devices, null);
            }

            mDeviceName = (TextView) view.findViewById(R.id.device_name);
            mDeviceAddress = (TextView) view.findViewById(R.id.device_address);

            BluetoothDevice device = mLeDevices.get(i);

            final String deviceName = device.getName();

            if (deviceName != null && deviceName.length() > 0) {
                mDeviceName.setText(deviceName);

            } else {
                mDeviceName.setText("Unknown device");
            }

            mDeviceAddress.setText(device.getAddress());

            return view;
        }
    }
}
