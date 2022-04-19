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

import android.app.Service;
import android.bluetooth.BluetoothDevice;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.RemoteException;
import android.util.Log;

import org.codeaurora.bluetooth.pxpservice.IPxpService;

import java.util.ArrayList;
import java.util.List;

public class PxpServiceProxy extends Service {

    private static final String TAG = PxpServiceProxy.class.getSimpleName();

    public static final String DEVICE_CONNECTED = "android.bluetooth.action.DEVICE_CONNECTED";

    public static final String DEVICE_DISCONNECTED = "android.bluetooth.action.DEVICE_DISCONNECTED";

    public static final String MISSING_LLS_SERVICE = "android.bluetooth.action.DEVICE_MISSING_LLS_SERVICE";

    public static final String LINKLOSS_ALERT = "android.bluetooth.action.LINKLOSS_ALERT";

    public static final String DEVICE_ALLERT = "action.DEVICE_ALLERT";

    public static final String PATHLOSS_ALLERT = "action.PATHLOSS_ALLERT";

    public static final String PATHLOSS_ALLERT_FINISH = "action.ALLERT_FINISH";

    public static final String EXTRAS_DEVICE = "BLUETOOTH_DEVICE";

    public static final String EXTRAS_TIME = "ALERT_TIME";

    public static final String EXTRAS_RSSI = "DEVICE_RSSI";

    public static final String EXTRAS_LLS_ALERT_VALUE = "LLS_ALERT_VALUE";

    public static final int MSG_REGISTER_CLIENT = 1;

    public static final int MSG_UNREGISTER_CLIENT = 2;

    public static final int MSG_DEVICE_CONNECTED = 1;

    public static final int MSG_MISSING_LLS_SERVICE = 2;

    public static final int MSG_LINKLOSS_ALERT = 3;

    public static final int MSG_DEVICE_DISCONNECTED = 4;

    public static final int MSG_LINK_LOSS_TIMEOUT = 5;

    public static final int MSG_PATHLOSS_ALERT = 6;

    public static final int MSG_ALERT_FINISH = 7;

    public static final int LINK_LOSS_ALERT_TIMEOUT = 15000;

    private IPxpService mPxpMonitorService = null;

    private static Handler mHandler = null;

    private final LocalBinder mBinder = new LocalBinder();

    protected ArrayList<AlertInformation> mAlertHistory = null;

    public class LocalBinder extends Binder {
        PxpServiceProxy getService() {
            return PxpServiceProxy.this;
        }
    }

    public static void registerClient(Handler handler) {
        mHandler = handler;
    }

    private final ServiceConnection mConnection = new ServiceConnection() {

        @Override
        public void onServiceConnected(ComponentName className, IBinder service) {
            mPxpMonitorService = IPxpService.Stub.asInterface((IBinder) service);

            Log.d(TAG, "Connected to Service");
        }

        @Override
        public void onServiceDisconnected(ComponentName className) {
            mPxpMonitorService = null;

            Log.d(TAG, "Disconnected from Service");
        }
    };

    @Override
    public void onCreate() {
        super.onCreate();

        Log.d(TAG, "On create: new intent BluetoothService");
        Intent gattServiceIntent = new Intent();
        gattServiceIntent.setAction("org.codeaurora.bluetooth.pxpservice.PxpMonitorService");
        bindService(gattServiceIntent, mConnection,0);
        startService(gattServiceIntent);

        mAlertHistory = new ArrayList<AlertInformation>();

        final IntentFilter intentFilter = new IntentFilter();
        intentFilter.addAction(DEVICE_ALLERT);
        intentFilter.addAction(DEVICE_CONNECTED);
        intentFilter.addAction(DEVICE_DISCONNECTED);
        intentFilter.addAction(MISSING_LLS_SERVICE);
        intentFilter.addAction(LINKLOSS_ALERT);
        intentFilter.addAction(PATHLOSS_ALLERT);
        intentFilter.addAction(PATHLOSS_ALLERT_FINISH);

        registerReceiver(mDataReceiver, intentFilter);
    }

    @Override
    public void onDestroy() {
        Log.d(TAG, "onDestroy");
        super.onDestroy();
        mHandler = null;
        unregisterReceiver(mDataReceiver);
    }

    @Override
    public IBinder onBind(Intent arg0) {
        Log.v(TAG, "onBind()");
        return mBinder;
    }

    @Override
    public boolean onUnbind(Intent intent) {
        Log.v(TAG, "onUnbind()");
        mHandler = null;
        return super.onUnbind(intent);
    }

    public synchronized boolean connect(BluetoothDevice leDevice) {
        try {
            return mPxpMonitorService.connect(leDevice);
        } catch (RemoteException e) {
            Log.e(TAG, e.toString());
        }
        return false;
    }

    public synchronized void disconnect(BluetoothDevice leDevice) {
        try {
            mPxpMonitorService.disconnect(leDevice);
        } catch (RemoteException e) {
            Log.e(TAG, e.toString());
        }
    }

    public synchronized int getLinkLossAlertLevel(BluetoothDevice leDevice) {
        try {
            return mPxpMonitorService.getLinkLossAlertLevel(leDevice);
        } catch (RemoteException e) {
            Log.e(TAG, e.toString());
        }
        return 0;
    }

    public synchronized boolean setLinkLossAlertLevel(BluetoothDevice leDevice, int level) {
        try {
            return mPxpMonitorService.setLinkLossAlertLevel(leDevice, level);
        } catch (RemoteException e) {
            Log.e(TAG, e.toString());
        }
        return false;
    }

    public synchronized int getPathLossAlertLevel(BluetoothDevice leDevice) {
        try {
            return mPxpMonitorService.getPathLossAlertLevel(leDevice);
        } catch (RemoteException e) {
            Log.e(TAG, e.toString());
        }
        return 0;
    }

    public synchronized boolean setPathLossAlertLevel(BluetoothDevice leDevice, int level) {
        try {
            return mPxpMonitorService.setPathLossAlertLevel(leDevice, level);
        } catch (RemoteException e) {
            Log.e(TAG, e.toString());
        }

        return false;
    }

    public synchronized int getMaxPathLossThreshold(BluetoothDevice leDevice) {
        try {
            return mPxpMonitorService.getMaxPathLossThreshold(leDevice);
        } catch (RemoteException e) {
            Log.e(TAG, e.toString());
        }
        return 0;
    }

    public synchronized void setMaxPathLossThreshold(BluetoothDevice leDevice, int threshold) {
        try {
            mPxpMonitorService.setMaxPathLossThreshold(leDevice, threshold);
        } catch (RemoteException e) {
            Log.e(TAG, e.toString());
        }
    }

    public synchronized int getMinPathLossThreshold(BluetoothDevice leDevice) {
        try {
            return mPxpMonitorService.getMinPathLossThreshold(leDevice);
        } catch (RemoteException e) {
            Log.e(TAG, e.toString());
        }
        return 0;
    }

    public synchronized void setMinPathLossThreshold(BluetoothDevice leDevice, int threshold) {
        try {
            mPxpMonitorService.setMinPathLossThreshold(leDevice, threshold);
        } catch (RemoteException e) {
            Log.e(TAG, e.toString());
        }
    }

    public synchronized boolean isPropertiesSet(BluetoothDevice leDevice) {
        try {
            return mPxpMonitorService.isPropertiesSet(leDevice);
        } catch (RemoteException e) {
            Log.e(TAG, e.toString());
        }
        return false;
    }

    public synchronized boolean getConnectionState(BluetoothDevice leDevice) {
        try {

            return mPxpMonitorService.getConnectionState(leDevice);

        } catch (RemoteException e) {
            Log.e(TAG, e.toString());
        }
        return false;
    }

    public synchronized boolean checkServiceStatus(BluetoothDevice leDevice, int service) {
        try {
            return mPxpMonitorService.checkServiceStatus(leDevice, service);
        } catch (RemoteException e) {
            Log.e(TAG, e.toString());
        }
        return false;
    }

    public synchronized boolean checkFailedReadTxPowerLevel(BluetoothDevice leDevice) {
        try {
            return mPxpMonitorService.checkFailedReadTxPowerLevel(leDevice);
        } catch (RemoteException e) {
            Log.e(TAG, e.toString());
        }
        return false;
    }

    public synchronized void disconnectDevices() {
        try {
            mPxpMonitorService.disconnectDevices();
        } catch (RemoteException e) {
            Log.e(TAG, e.toString());
        }
    }

    public synchronized List<BluetoothDevice> getConnectedDevices() {
        try {
            return mPxpMonitorService.getConnectedDevices();
        } catch (RemoteException e) {
            Log.e(TAG, e.toString());
        }
        return null;
    }

    private final BroadcastReceiver mDataReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            BluetoothDevice mLeDevice;
            String action = intent.getAction();

            Message msg = null;

            if (DEVICE_CONNECTED.equals(action)) {
                Log.d(TAG, "Acton = DEVICE_CONNECTED");
                mLeDevice = (BluetoothDevice) intent.getParcelableExtra(EXTRAS_DEVICE);
                Log.d(TAG, "After reading mLeDevice");
                msg = Message.obtain(null,
                        MSG_DEVICE_CONNECTED, mLeDevice);

            } else if (MISSING_LLS_SERVICE.equals(action)) {
                Log.d(TAG, "Action = MISSING_LLS_SERVICE");
                mLeDevice = (BluetoothDevice) intent.getParcelableExtra(EXTRAS_DEVICE);
                msg = Message.obtain(null,
                        MSG_MISSING_LLS_SERVICE, mLeDevice);

            } else if (LINKLOSS_ALERT.equals(action)) {
                Log.d(TAG, "Action = LINKLOSS_ALERT");
                mLeDevice = (BluetoothDevice) intent.getParcelableExtra(EXTRAS_DEVICE);
                int llsAlertValue = intent.getIntExtra(EXTRAS_LLS_ALERT_VALUE, 0);

                msg = Message.obtain(null,
                        MSG_LINKLOSS_ALERT, llsAlertValue);

            } else if (DEVICE_DISCONNECTED.equals(action)) {
                Log.d(TAG, "Action = DEVICE_DISCONNECTED");
                mLeDevice = (BluetoothDevice) intent
                        .getParcelableExtra(EXTRAS_DEVICE);
                msg = Message.obtain(null,
                        MSG_DEVICE_DISCONNECTED, mLeDevice);

            } else if (DEVICE_ALLERT.equals(action)) {
                Log.d(TAG, "DEVICE_ALLERT");

                AlertInformation alertInfo = new AlertInformation();
                alertInfo.device = (BluetoothDevice) intent.getParcelableExtra(EXTRAS_DEVICE);
                alertInfo.date = intent.getStringExtra(EXTRAS_TIME);
                alertInfo.rssi = intent.getIntExtra(EXTRAS_RSSI, 0);
                mAlertHistory.add(alertInfo);

            } else if (PATHLOSS_ALLERT.equals(action)) {
                Log.d(TAG, "PATHLOSS_ALLERT");

                mLeDevice = (BluetoothDevice) intent
                        .getParcelableExtra(EXTRAS_DEVICE);
                msg = Message.obtain(null,
                        MSG_PATHLOSS_ALERT, mLeDevice);
            } else if (PATHLOSS_ALLERT_FINISH.equals(action)) {
                Log.d(TAG, "DEVICE_ALLERT_FINISH");

                mLeDevice = (BluetoothDevice) intent
                        .getParcelableExtra(EXTRAS_DEVICE);
                msg = Message.obtain(null,
                        MSG_ALERT_FINISH, mLeDevice);
            }

            if (msg == null) {
                return;
            }

            if (mHandler != null) {
                mHandler.sendMessage(msg);
            }
        }
    };
}
