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
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.BaseAdapter;
import android.widget.ListView;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

public class ScanDevicesActivity extends ListActivity {
    private static final String TAG = ScanDevicesActivity.class.getSimpleName();

    public static final String DEVICE_NOT_CONNECTED_SELECTED =
            "android.bluetooth.devicepicker.action.NOT_CONNECTED_DEVICE_SELECTED";

    private static final int REQUEST_ENABLE_BT = 1;

    private BluetoothAdapter mBluetoothAdapter = null;

    private ScanDevicesListAdapter mDeviceListAdapter = null;

    private PxpServiceProxy mPxpServiceProxy = null;

    private Handler mHandler;

    private boolean mScanning;

    private static final long SCAN_PERIOD = 30000; // 30sec for now

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
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getActionBar().setTitle("Scan for Devices");

        Log.d(TAG, "On create: new intent BluetoothService");
        Intent gattServiceIntent = new Intent(this, PxpServiceProxy.class);

        if (bindService(gattServiceIntent, mConnection, BIND_AUTO_CREATE) == false) {
            Log.e(TAG, "Unable to bind");
        }

        mHandler = new Handler();

        final BluetoothManager bluetoothManager =
                (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        mBluetoothAdapter = bluetoothManager.getAdapter();

        mDeviceListAdapter = new ScanDevicesListAdapter(this);
        setListAdapter(mDeviceListAdapter);
        scanDevice(true);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.scan, menu);

        if (mScanning) {
            menu.findItem(R.id.menu_stop).setVisible(true);
            menu.findItem(R.id.action_search).setVisible(false);

        } else {
            menu.findItem(R.id.menu_stop).setVisible(false);
            menu.findItem(R.id.action_search).setVisible(true);
        }
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.action_search:
                mDeviceListAdapter.clear();
                scanDevice(true);
                break;

            case R.id.menu_stop:
                scanDevice(false);
                break;
        }
        return true;
    }

    @Override
    protected void onResume() {
        super.onResume();
        scanDevice(true);
    }

    @Override
    protected void onPause() {
        super.onPause();

        if (!mBluetoothAdapter.isEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
        }

        scanDevice(false);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {

        if (requestCode == REQUEST_ENABLE_BT && resultCode == Activity.RESULT_CANCELED) {
            finish();
            return;
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();

        unbindService(mConnection);
        mPxpServiceProxy = null;
    }

    @Override
    protected void onListItemClick(ListView l, View v, int position, long id) {
        final BluetoothDevice leDevice = mDeviceListAdapter.getDevice(position);

        if (mScanning) {
            scanDevice(false);
        }

        final Intent intent = new Intent(this, DeviceActivity.class);
        intent.setAction(DEVICE_NOT_CONNECTED_SELECTED);
        intent.putExtra(DeviceActivity.EXTRAS_DEVICE, leDevice);
        startActivity(intent);
    }

    private void scanDevice(final boolean enable) {

        Runnable scan = new Runnable() {

            @Override
            public void run() {
                mScanning = false;
                mBluetoothAdapter.stopLeScan(mLeScanCallback);
                invalidateOptionsMenu();
            }
        };

        if (enable) {
            mHandler.postDelayed(scan, SCAN_PERIOD);
            mScanning = true;
            mBluetoothAdapter.startLeScan(mLeScanCallback);

        } else {
            mScanning = false;
            mBluetoothAdapter.stopLeScan(mLeScanCallback);
            mHandler.removeCallbacks(scan);
        }
        invalidateOptionsMenu();
    }

    private class ScanDevicesListAdapter extends BaseAdapter {
        private ArrayList<BluetoothDevice> mLeDevices;
        private LayoutInflater mInflater;
        private Map<BluetoothDevice, DeviceInfo> mHashMapDevice;
        private TextView mDeviceName;
        private TextView mDeviceAddress;
        private TextView mDeviceRssi;
        private TextView mDeviceAdvData;

        public ScanDevicesListAdapter(Context context) {
            super();
            mLeDevices = new ArrayList<BluetoothDevice>();
            mInflater = LayoutInflater.from(context);
            mHashMapDevice = new HashMap<BluetoothDevice, DeviceInfo>();
        }

        public void addDevice(BluetoothDevice device, int rssi, String advData) {

            if (!mLeDevices.contains(device)) {
                mLeDevices.add(device);
                DeviceInfo deviceInfo = new DeviceInfo(rssi, advData);
                mHashMapDevice.put(device, deviceInfo);
            }
            this.notifyDataSetChanged();
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
            mDeviceRssi = (TextView) view.findViewById(R.id.device_rssi);
            mDeviceAdvData = (TextView) view.findViewById(R.id.device_adv_data);

            BluetoothDevice device = mLeDevices.get(i);

            DeviceInfo deviceInfo = mHashMapDevice.get(device);

            final String deviceName = device.getName();

            if (deviceName != null && deviceName.length() > 0) {
                mDeviceName.setText(deviceName);

            } else {
                mDeviceName.setText("Unknow device");
            }

            mDeviceAddress.setText(device.getAddress());
            mDeviceRssi.setText("RSSI = " + String.valueOf(deviceInfo.mRssi) + "dBm");
            mDeviceAdvData.setText("Adv data: " + deviceInfo.mAdvData);

            return view;
        }

        private class DeviceInfo {
            private int mRssi;
            private String mAdvData;

            DeviceInfo(int rssi, String advData) {
                mRssi = rssi;
                mAdvData = advData;
            }
        }
    }

    private BluetoothAdapter.LeScanCallback mLeScanCallback =
            new BluetoothAdapter.LeScanCallback() {

                @Override
                public void onLeScan(final BluetoothDevice device, final int rssi,
                        final byte[] scanRecord) {
                    runOnUiThread(new Runnable() {
                        @Override
                        public void run() {
                            mDeviceListAdapter.addDevice(device, rssi, Arrays.toString(scanRecord));
                        }
                    });
                }
            };
}
