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
import android.media.Ringtone;
import android.media.RingtoneManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.util.Log;
import android.view.Gravity;
import android.view.Menu;
import android.view.MenuItem;
import android.widget.Toast;

public class DeviceActivity extends Activity {

    private static final String TAG = DeviceActivity.class.getSimpleName();

    public static final String EXTRAS_DEVICE = "BLUETOOTH_DEVICE";

    private static final int REQUEST_ENABLE_BT = 1;

    private BluetoothAdapter mBluetoothAdapter = null;

    protected PxpServiceProxy mPxpServiceProxy = null;

    protected BluetoothDevice mLeDevice = null;

    private DeviceInfoFragment mDeviceInfoFragment = null;

    private LinkLossFragment mLinkLossFragment = null;

    private PathLossFragment mPathLossFragment = null;

    private Uri mLinkNotification = null;

    private Uri mPathNotification = null;

    private Ringtone mLinkRingtone = null;

    private Ringtone mPathRingtone = null;

    private final ServiceConnection mConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {
            Log.d(TAG, "Connecting to service");
            mPxpServiceProxy = ((PxpServiceProxy.LocalBinder) service).getService();

            if (mPxpServiceProxy == null) {
                return;
            }

            PxpServiceProxy.registerClient(mHandler);

            Intent intent = getIntent();
            String action = intent.getAction();

            if (ConnectedDevicesActivity.DEVICE_CONNECTED_SELECTED.
                    equals(action)) {
                mLeDevice = (BluetoothDevice) intent.getParcelableExtra(EXTRAS_DEVICE);

            } else if (PairedDevicesActivity.DEVICE_PAIRED_SELECTED.
                    equals(action)) {
                mLeDevice = (BluetoothDevice) intent.getParcelableExtra(EXTRAS_DEVICE);

                if (mLeDevice != null && mPxpServiceProxy.connect(mLeDevice)) {
                    Log.d(TAG, "Pxp device connected");
                }

            } else if (ScanDevicesActivity.DEVICE_NOT_CONNECTED_SELECTED.
                    equals(action)) {
                mLeDevice = (BluetoothDevice) intent.getParcelableExtra(EXTRAS_DEVICE);

                if (mLeDevice != null && mPxpServiceProxy.connect(mLeDevice)) {
                    Log.d(TAG, "Pxp device connected");
                }
                return;

            } else if (MainActivity.DEVICE_BY_ADDRESS_SELECTED.
                    equals(action)) {
                mLeDevice = (BluetoothDevice) intent.getParcelableExtra(EXTRAS_DEVICE);

                if (mLeDevice != null && mPxpServiceProxy.connect(mLeDevice)) {
                    Log.d(TAG, "Pxp device connected");
                }
            }

            if (mLeDevice != null && mPxpServiceProxy.isPropertiesSet(mLeDevice)) {
                setInfo();
                invalidateOptionsMenu();
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName className) {
            mPxpServiceProxy = null;
        }
    };

    private final Handler mHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            CharSequence toastText = null;

            switch (msg.what) {

                case PxpServiceProxy.MSG_DEVICE_CONNECTED:
                    Log.d(TAG, "PxpServiceProxy.MSG_DEVICE_CONNECTED");

                    mLeDevice = (BluetoothDevice) msg.obj;

                    if (mLinkRingtone.isPlaying()) {
                        mLinkRingtone.stop();
                    }

                    this.removeMessages(PxpServiceProxy.MSG_LINK_LOSS_TIMEOUT);

                    toastText = "Connected";

                    if (mPxpServiceProxy.isPropertiesSet(mLeDevice)) {
                        setInfo();
                    }
                    break;

                case PxpServiceProxy.MSG_DEVICE_DISCONNECTED:
                    Log.d(TAG, "PxpServiceProxy.MSG_DEVICE_DISCONNECTED");

                    mLeDevice = (BluetoothDevice) msg.obj;

                    if (mLeDevice.equals(mLeDevice)) {
                        toastText = "Disconnected";
                        mLinkLossFragment.init();
                        mPathLossFragment.init();
                    }
                    break;

                case PxpServiceProxy.MSG_MISSING_LLS_SERVICE:
                    Log.d(TAG, "PxpServiceProxy.MSG_MISSING_LLS_SERVICE");
                    toastText = "Device not connected - Link Loss Service not found";
                    break;

                case PxpServiceProxy.MSG_LINKLOSS_ALERT:
                    Log.d(TAG, "PxpServiceProxy.MSG_LINKLOSS_ALERT");

                    int llsAlertValue = (Integer) msg.obj;
                    mLinkLossFragment.init();
                    mPathLossFragment.init();

                    // No alerting if alertLevel == 0
                    if (llsAlertValue == 0) {
                        return;
                    }

                    this.sendMessageDelayed(
                            this.obtainMessage(PxpServiceProxy.MSG_LINK_LOSS_TIMEOUT),
                            PxpServiceProxy.LINK_LOSS_ALERT_TIMEOUT);

                    mLinkRingtone.play();
                    break;

                case PxpServiceProxy.MSG_PATHLOSS_ALERT:
                    if (!mPathRingtone.isPlaying()) {
                        mPathRingtone.play();
                    }
                    break;

                case PxpServiceProxy.MSG_LINK_LOSS_TIMEOUT:
                    mLinkRingtone.stop();
                    break;

                case PxpServiceProxy.MSG_ALERT_FINISH:
                    if (mPathRingtone.isPlaying()) {
                        mPathRingtone.stop();
                    }
                    break;

                default:
                    super.handleMessage(msg);
            }

            if (toastText != null) {
                Toast toast = Toast.makeText(DeviceActivity.this, toastText, Toast.LENGTH_SHORT);
                toast.setGravity(Gravity.CENTER, 0, 0);
                toast.show();
            }

            invalidateOptionsMenu();
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_device);

        mDeviceInfoFragment = (DeviceInfoFragment) getFragmentManager().findFragmentById(
                R.id.device_info_fragment);
        mLinkLossFragment = (LinkLossFragment) getFragmentManager().findFragmentById(
                R.id.link_loss_fragment);
        mPathLossFragment = (PathLossFragment) getFragmentManager().findFragmentById(
                R.id.path_loss_fragment);

        Log.d(TAG, "On create: new intent BluetoothService");
        Intent gattServiceIntent = new Intent(this, PxpServiceProxy.class);

        if (bindService(gattServiceIntent, mConnection, BIND_AUTO_CREATE) == false) {
            Log.e(TAG, "Unable to bind");
        }

        final BluetoothManager bluetoothManager =
                (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
        mBluetoothAdapter = bluetoothManager.getAdapter();

        mLinkNotification = RingtoneManager.getDefaultUri(RingtoneManager.TYPE_ALARM);
        mLinkRingtone = RingtoneManager.getRingtone(getApplicationContext(), mLinkNotification);

        mPathNotification = RingtoneManager.getDefaultUri(RingtoneManager.TYPE_RINGTONE);
        mPathRingtone = RingtoneManager.getRingtone(getApplicationContext(), mPathNotification);

    }

    @Override
    protected void onResume() {
        super.onResume();

        if (!mBluetoothAdapter.isEnabled()) {
            Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
        }

        if (mLeDevice != null && mPxpServiceProxy.isPropertiesSet(mLeDevice)) {
            setInfo();
            invalidateOptionsMenu();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();

        if (mLinkRingtone.isPlaying()) {
            mLinkRingtone.stop();
        }

        if (mPathRingtone.isPlaying()) {
            mPathRingtone.stop();
        }

        unbindService(mConnection);
    }

    @Override
    protected void onPause() {
        super.onPause();
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {

        if (requestCode == REQUEST_ENABLE_BT && resultCode == Activity.RESULT_CANCELED) {
            finish();
            return;
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        getMenuInflater().inflate(R.menu.device, menu);

        if (null == mPxpServiceProxy || !mPxpServiceProxy.isPropertiesSet(mLeDevice)) {
            return false;
        }

        if (mPxpServiceProxy.getConnectionState(mLeDevice)) {
            menu.findItem(R.id.menu_disconnect).setVisible(true);
            menu.findItem(R.id.menu_connect).setVisible(false);

        } else {
            menu.findItem(R.id.menu_disconnect).setVisible(false);
            menu.findItem(R.id.menu_connect).setVisible(true);
        }
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.menu_disconnect:
                Log.d(TAG, "Disconnecting");
                mPxpServiceProxy.disconnect(mLeDevice);
                break;

            case R.id.menu_connect:
                Log.d(TAG, "Connecting");
                if (mPxpServiceProxy.connect(mLeDevice) == true) {
                    setInfo();
                }
                invalidateOptionsMenu();
                break;
        }
        return true;
    }

    void setInfo() {

        final int LinkLossService = 0;
        final int ImmediateAlertService = 1;
        final int TxPowerService = 2;

        mDeviceInfoFragment.setDeviceInfo(mLeDevice.getName(), mLeDevice.getAddress());

        mLinkLossFragment.onServiceFound(mPxpServiceProxy.checkServiceStatus(mLeDevice,
                LinkLossService));

        mPathLossFragment.onServiceFound(
                mPxpServiceProxy.checkServiceStatus(mLeDevice, ImmediateAlertService),
                mPxpServiceProxy.checkServiceStatus(mLeDevice, TxPowerService));
    }
}
