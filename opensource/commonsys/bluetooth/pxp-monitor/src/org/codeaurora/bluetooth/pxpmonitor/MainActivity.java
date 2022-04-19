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
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.Menu;
import android.view.View;
import android.widget.Toast;

import org.codeaurora.bluetooth.pxpmonitor.ConnectToDeviceDialogFragment.ConnectToDeviceDialogListener;

public class MainActivity extends Activity implements ConnectToDeviceDialogListener {

    private static final String TAG = MainActivity.class.getSimpleName();;

    private static final int REQUEST_ENABLE_BT = 1;

    public static final String DEVICE_BY_ADDRESS_SELECTED =
            "android.bluetooth.devicepicker.action.DEVICE_BY_ADDRESS_SELECTED";

    protected BluetoothAdapter mBluetoothAdapter = null;

    protected PxpServiceProxy mPxpServiceProxy = null;

    protected Intent intent = null;

    private final ServiceConnection mConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {
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
        setContentView(R.layout.activity_main);

        if (!getPackageManager().hasSystemFeature(PackageManager.FEATURE_BLUETOOTH_LE)) {
            Toast.makeText(this, "BLE not supported", Toast.LENGTH_SHORT).show();
            finish();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();

        final BluetoothManager bluetoothManager =
              (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        mBluetoothAdapter = bluetoothManager.getAdapter();

        if (!mBluetoothAdapter.isEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
        }
        else
        {
            Log.d(TAG, "On create: new intent BluetoothService");
            Intent gattServiceIntent = new Intent(this, PxpServiceProxy.class);

            if (bindService(gattServiceIntent, mConnection, 0) == false) {
                Log.e(TAG, "Unable to bind");
            }
            else
                startService(gattServiceIntent);
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {

        if (requestCode == REQUEST_ENABLE_BT && resultCode == Activity.RESULT_CANCELED) {
            finish();
            return;
        }
        else if(requestCode == REQUEST_ENABLE_BT && resultCode == Activity.RESULT_OK)
        {
            Intent gattServiceIntent = new Intent(this, PxpServiceProxy.class);
            Log.d(TAG, "On create: new intent BluetoothService");

            if (bindService(gattServiceIntent, mConnection,0) == false) {
                Log.e(TAG, "Unable to bind");
            }
            else
                startService(gattServiceIntent);
        }
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onDestroy() {

        Log.v(TAG, "onDestroy ");
        if(mConnection != null ) {
            unbindService(mConnection);
            mPxpServiceProxy = null;
            super.onDestroy();
       }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.main, menu);
        return true;
    }

    public void searchForDevices(View view) {
        final Intent intent = new Intent(this, ScanDevicesActivity.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
        startActivity(intent);
    }

    public void showConnectedDevices(View view) {
        final Intent intent = new Intent(this, ConnectedDevicesActivity.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
        startActivity(intent);
    }

    public void showPairedDevices(View view) {
        final Intent intent = new Intent(this, PairedDevicesActivity.class);
        startActivity(intent);
    }

    public void connectToDevice(View view) {
        ConnectToDeviceDialogFragment newFragment = new ConnectToDeviceDialogFragment();
        newFragment.show(getFragmentManager(), "address");
    }

    public void showAlertHistory(View view) {
        final Intent intent = new Intent(this, AlertsHistory.class);
        intent.setFlags(Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
        startActivity(intent);
    }

    @Override
    public void onConnectToDeviceDialogPositive(String address) {
        BluetoothDevice leDevice = null;

        if (BluetoothAdapter.checkBluetoothAddress(address)) {

            leDevice = mBluetoothAdapter.getRemoteDevice(address);

            final Intent intent = new Intent(this, DeviceActivity.class);
            intent.setAction(DEVICE_BY_ADDRESS_SELECTED);
            intent.setFlags(Intent.FLAG_ACTIVITY_EXCLUDE_FROM_RECENTS);
            intent.putExtra(DeviceActivity.EXTRAS_DEVICE, leDevice);
            startActivity(intent);

        } else {
            Log.w(TAG, "Wrong device address");
        }

    }
}
