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
import java.util.Iterator;
import java.util.List;

public class ConnectedDevicesActivity extends ListActivity {

    private static final String TAG = ConnectedDevicesActivity.class.getSimpleName();

    public static final String DEVICE_CONNECTED_SELECTED = "android.bluetooth.devicepicker.action.DEVICE_SELECTED";

    private static final int REQUEST_ENABLE_BT = 1;

    private ConnectedDeviceAdapter mConnectedDeviceAdapter;

    private BluetoothAdapter mBluetoothAdapter = null;

    private PxpServiceProxy mPxpServiceProxy = null;

    private final ServiceConnection mConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {
            mPxpServiceProxy = ((PxpServiceProxy.LocalBinder) service).getService();

            if (mPxpServiceProxy == null) {
                return;
            }

            listConnectedDevices();
        }

        @Override
        public void onServiceDisconnected(ComponentName className) {
            mPxpServiceProxy = null;
        }
    };

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        Log.d(TAG, "On create: new intent BluetoothService");
        Intent gattServiceIntent = new Intent(this, PxpServiceProxy.class);

        if (bindService(gattServiceIntent, mConnection, BIND_AUTO_CREATE) == false) {
            Log.e(TAG, "Unable to bind");
        }

        final BluetoothManager bluetoothManager =
                (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        mBluetoothAdapter = bluetoothManager.getAdapter();

        mConnectedDeviceAdapter = new ConnectedDeviceAdapter(this);
        setListAdapter(mConnectedDeviceAdapter);
    }

    @Override
    protected void onResume() {
        super.onResume();

        if (!mBluetoothAdapter.isEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
        }

        if (mPxpServiceProxy != null) {
            mConnectedDeviceAdapter.clear();
            listConnectedDevices();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        unbindService(mConnection);
        mPxpServiceProxy = null;
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if (requestCode == REQUEST_ENABLE_BT && resultCode == Activity.RESULT_CANCELED) {
            finish();
            return;
        }

        if (mPxpServiceProxy != null) {
            listConnectedDevices();
        }
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {

        final BluetoothDevice leDevice = mConnectedDeviceAdapter.getDevice(position);

        if (leDevice == null) {
            return;
        }

        final Intent intent = new Intent(this, DeviceActivity.class);

        intent.setAction(DEVICE_CONNECTED_SELECTED);
        intent.setFlags(Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
        intent.putExtra(DeviceActivity.EXTRAS_DEVICE, leDevice);

        startActivity(intent);
    }

    private synchronized void listConnectedDevices() {

        List<BluetoothDevice> connectedDevices = mPxpServiceProxy.getConnectedDevices();

        Iterator<BluetoothDevice> iterator = connectedDevices.iterator();

        while (iterator.hasNext()) {
            mConnectedDeviceAdapter.addDevice(iterator.next());
            mConnectedDeviceAdapter.notifyDataSetChanged();
        }
    }

    private class ConnectedDeviceAdapter extends BaseAdapter {

        private ArrayList<BluetoothDevice> mLeDevices;

        private LayoutInflater mInflater;

        private TextView mDeviceName;

        private TextView mDeviceAddress;

        public ConnectedDeviceAdapter(Context context) {
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
                mDeviceName.setText("Unknow device");
            }

            mDeviceAddress.setText(device.getAddress());

            return view;
        }
    }
}
