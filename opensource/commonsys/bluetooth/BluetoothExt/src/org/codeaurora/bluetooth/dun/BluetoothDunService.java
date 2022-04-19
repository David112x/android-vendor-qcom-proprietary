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

package org.codeaurora.bluetooth.dun;

import android.app.Service;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.NotificationChannel;
import android.app.PendingIntent;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothDun;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothServerSocket;
import android.bluetooth.BluetoothSocket;
import android.bluetooth.IBluetooth;
import android.bluetooth.IBluetoothDun;
import android.content.ComponentName;
import android.content.Context;
import android.content.BroadcastReceiver;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.content.SharedPreferences;
import android.net.ConnectivityManager;
import android.net.LocalSocket;
import android.net.LocalSocketAddress;
import android.os.IBinder;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.HwBinder;
import android.os.Looper;
import android.os.Message;
import android.os.ParcelUuid;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemProperties;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.HashSet;
import java.util.List;

import java.io.OutputStream;
import java.io.InputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Arrays;
import java.util.concurrent.atomic.AtomicLong;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;
import java.util.NoSuchElementException;
import java.util.UUID;

import com.android.internal.telephony.ITelephony;
import org.codeaurora.bluetooth.R;

import vendor.qti.hardware.bluetooth_dun.V1_0.CtrlMsg;
import vendor.qti.hardware.bluetooth_dun.V1_0.Status;
import vendor.qti.hardware.bluetooth_dun.V1_0.IBluetoothDunServer;
import vendor.qti.hardware.bluetooth_dun.V1_0.IBluetoothDunServerResponse;


/**
 * Provides Bluetooth Dun profile, as a service in the BluetoothExt APK.
 * @hide
 */

public class BluetoothDunService extends Service {

    private static final String TAG                             = "BluetoothDunService";
    public static final String LOG_TAG                          = "BluetoothDun";
    public static boolean VERBOSE;

    private final static String DUN_SERVER                      = "qcom.dun.server";

    private static final int MESSAGE_START_LISTENER             = 0x01;
    private static final int MESSAGE_DUN_USER_TIMEOUT           = 0x02;
    private static final int MESSAGE_HIDL_DAEMON_DEAD           = 0x03;

    private static final int USER_CONFIRM_TIMEOUT_VALUE         = 30000; // 30 seconds

    private static final int MON_THREAD_SLEEP_INTERVAL          = 200; // 200 mseconds

    /**
     * IPC message types which can be exchanged between
     * Dun server process and BluetoothDunService
     */
    private static final byte DUN_IPC_MSG_DUN_REQUEST           = 0x00;
    private static final byte DUN_IPC_MSG_DUN_RESPONSE          = 0x01;
    private static final byte DUN_IPC_MSG_CTRL_REQUEST          = 0x02;
    private static final byte DUN_IPC_MSG_CTRL_RESPONSE         = 0x03;
    private static final byte DUN_IPC_MSG_MDM_STATUS            = 0x04;

    /**
     * Control message types between Dun server and
     * BluetoothDunService
     */
    private static final byte DUN_CRTL_MSG_DISCONNECT_REQ       = 0x00;
    private static final byte DUN_CRTL_MSG_CONNECTED_RESP       = 0x01;
    private static final byte DUN_CRTL_MSG_DISCONNECTED_RESP    = 0x02;

    /**
     * Offsets of message data with in the IPC message
     */
    private static final byte DUN_IPC_MSG_OFF_MSG_TYPE          = 0x00;
    private static final byte DUN_IPC_MSG_OFF_MSG_LEN           = 0x01;
    private static final byte DUN_IPC_MSG_OFF_MSG               = 0x03;

    /**
     *  maximum message size (Dun HIDL server <---> DUN service)
     */
    private static final int DUN_HIDL_MAX_MSG_LEN                = 16*1024;

    /**
     * Server supported DUN Profile's maximum message size
     */
    private static final int DUN_MAX_MSG_LEN                    = 32764;

    /**
     * IPC message's header size
     */
    private static final int DUN_IPC_HEADER_SIZE                = 0x03;

    /**
     * IPC message's maximum message size (Dun server <---> DUN service)
     */
    private static final int DUN_MAX_IPC_MSG_LEN                = DUN_MAX_MSG_LEN +
                                                                  DUN_IPC_HEADER_SIZE;
    /**
     * IPC message's control message size
     */
    private static final byte DUN_IPC_CTRL_MSG_SIZE             = 0x01;

    /**
     * IPC message's modem status message size
     */
    private static final byte DUN_IPC_MDM_STATUS_MSG_SIZE       = 0x01;

    /**
     * BT socket options
     */
    private static final int BTSOCK_OPT_GET_MODEM_BITS          = 0x01;
    private static final int BTSOCK_OPT_SET_MODEM_BITS          = 0x02;
    private static final int BTSOCK_OPT_CLR_MODEM_BITS          = 0x03;

    private static IBluetooth mAdapterService                   = null;

    private static BluetoothDevice mRemoteDevice                = null;

    private static volatile SocketAcceptThread mAcceptThread    = null;

    /**
     * Thread for reading the requests from DUN client using the socket
     * mRfcommSocket and upload the same requests to DUNd using the
     * socket mDundSocket
     */
    private static volatile UplinkThread mUplinkThread          = null;

    /**
     * Thtread for reading(download) the responses from DUNd using
     * the socket mDundSocket and route the same responses to
     * DUN client using the socket mRfcommSocket
     */
    private static volatile DownlinkThread mDownlinkThread      = null;

    /**
     * Thtread for monitoring the modem status and base on the status
     * chnage it will update the DUN server with the updated status
     */
    private static volatile MonitorThread mMonitorThread      = null;

    /**
     * Thread for reading the requests from DUN client using the socket
     * mRfcommSocket and upload the same requests to DUN hidl server using the
     * hal interface
     */
    private static volatile UplinkHIDLThread mUplinkHIDLThread          = null;

    /**
     * Thread for reading(download) the responses from DUN hidl server using
     * the hal interface and route the same responses to
     * DUN client using the socket mRfcommSocket
     */
    private static volatile DownlinkHIDLThread mDownlinkHIDLThread          = null;

    /**
     * Listening Rfcomm Socket
     */
    private static volatile BluetoothServerSocket mListenSocket = null;

    /**
     * Connected Rfcomm socket
     */
    private static volatile BluetoothSocket mRfcommSocket       = null;

    /**
     * Socket represents the connection between DUNd and DUN service
     */
    private static volatile LocalSocket mDundSocket             = null;

    private boolean mIsWaitingAuthorization                     = false;

    private volatile boolean mInterrupted                       = false;

    private boolean mDunEnable                                  = false;

    private byte mRmtMdmStatus                                  = 0x00;

    private CountDownLatch mDunConnectSignal;

    private boolean mIsDunHIDLConnected = false;

    private volatile IBluetoothDunServer mBluetoothDunServerProxy = null;

    private boolean mHidlSupported = false;

    private BluetoothDunServerResponse mDunServerResponse;

    private final AtomicLong mBluetoothDunServerProxyCookie = new AtomicLong(0);

    private BluetoothDunServerDeathRecipient mBluetoothDunServerDeathRecipient;

    /**
     * DUN profile's UUID
     */
    private static final String DUN_UUID = "00001103-0000-1000-8000-00805F9B34FB";

    /**
     * Intent indicating incoming connection request which is sent to
     * BluetoothDunPermissionActivity
     */
    public static final String DUN_ACCESS_REQUEST_ACTION =
                                     "org.codeaurora.bluetooth.dun.accessrequest";

    /**
     * Intent indicating incoming connection request accepted by user which is
     * sent from BluetoothDunPermissionActivity
     */
    public static final String DUN_ACCESS_ALLOWED_ACTION =
                                     "org.codeaurora.bluetooth.dun.accessallowed";

    /**
     * Intent indicating incoming connection request denied by user which is
     * sent from BluetoothDunPermissionActivity
     */
    public static final String DUN_ACCESS_DISALLOWED_ACTION =
                                  "org.codeaurora.bluetooth.dun.accessdisallowed";

    /**
     * Intent indicating timeout for user confirmation, which is sent to
     * BluetoothDunPermissionActivity
     */
    public static final String USER_CONFIRM_TIMEOUT_ACTION =
                                "org.codeaurora.bluetooth.dun.userconfirmtimeout";

    public static final String THIS_PACKAGE_NAME = "org.codeaurora.bluetooth";

    /**
     * Intent Extra name indicating always allowed which is sent from
     * BluetoothDunPermissionActivity
     */
    public static final String DUN_EXTRA_ALWAYS_ALLOWED =
                                     "org.codeaurora.bluetooth.dun.alwaysallowed";

    // Ensure not conflict with Opp notification ID
    private static final int DUN_NOTIFICATION_ID_ACCESS = -1000006;

    /**
     * Intent Extra name indicating BluetoothDevice which is sent to
     * BluetoothDunPermissionActivity
     */
    public static final String EXTRA_BLUETOOTH_DEVICE =
                                   "org.codeaurora.bluetooth.dun.bluetoothdevice";

    /**
     * String signifies the DUN profile status
     */
    public static final String BLUETOOTH_DUN_PROFILE_STATUS = "vendor.bluetooth.dun.status";

    public static final ParcelUuid DUN = ParcelUuid.fromString(DUN_UUID);

    private static final Object mAcceptLock = new Object();

    private static final Object mUplinkLock = new Object();

    private static final Object mDownlinkLock = new Object();

    private static final Object mMonitorLock = new Object();

    private static final Object mAuthLock = new Object();

    private HashMap<BluetoothDevice, BluetoothDunDevice> mDunDevices;

    private IBinder mDunBinder;

    private BluetoothAdapter mAdapter;

    private DunServiceMessageHandler mDunHandler;

    private ITelephony mTelephonyService;

    /**
     * package and class name to which we send intent to check DUN profile
     * access permission
     */
    private static final String ACCESS_AUTHORITY_PACKAGE = "com.android.settings";
    private static final String ACCESS_AUTHORITY_CLASS =
        "com.android.settings.bluetooth.BluetoothPermissionRequest";

    private static final String DUN_ACCESS_PERMISSION_PREFERENCE_FILE =
            "dun_access_permission";

    private static final String BLUETOOTH_ADMIN_PERM =
                    android.Manifest.permission.BLUETOOTH_ADMIN;

    private static final String BLUETOOTH_PERM =
                    android.Manifest.permission.BLUETOOTH;

    private static final String BLUETOOTH_PRIVILEGED =
                    android.Manifest.permission.BLUETOOTH_PRIVILEGED;

    private NotificationChannel mNotificationChannel = null;

    private final String DUN_NOTIFICATION_CHANNEL = "dun_notification_channel";

    private class DunBroadcastReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            parseIntent(intent);
        }
    }

    private DunBroadcastReceiver mDunReceiver = null;

    @Override
    public void onCreate() {

        super.onCreate();
        VERBOSE = Log.isLoggable(LOG_TAG, Log.VERBOSE);
        String baseband = SystemProperties.get("ro.baseband");
        String platform = SystemProperties.get("ro.board.platform");
        Log.i(TAG, "onCreate baseband: " + baseband + ", platform: " + platform);

        if (baseband.equals("mdm") &&
           (platform.equals("msmnile") || platform.equals("kona"))) {
            mHidlSupported = true;
            mBluetoothDunServerDeathRecipient = new BluetoothDunServerDeathRecipient();
        }
        mDunDevices = new HashMap<BluetoothDevice, BluetoothDunDevice>();

        mAdapter = BluetoothAdapter.getDefaultAdapter();
        mDunBinder = initBinder();

        HandlerThread thread = new HandlerThread("BluetoothDunHandler");
        thread.start();
        Looper looper = thread.getLooper();
        mDunHandler = new DunServiceMessageHandler(looper);

        mTelephonyService = ITelephony.Stub.asInterface(ServiceManager.getService("phone"));
        if (mTelephonyService == null) {
            Log.e(TAG, "Failed to get Telephony Service interface");
        } else {
            Log.i(TAG, "Telephony Service interface got Successfully");
        }

        if (mDunReceiver == null) {
            mDunReceiver = new DunBroadcastReceiver();
            IntentFilter filter = new IntentFilter(BluetoothAdapter.ACTION_STATE_CHANGED);
            filter.addAction(BluetoothDevice.ACTION_ACL_DISCONNECTED);
            filter.addAction(BluetoothDevice.ACTION_BOND_STATE_CHANGED);
            registerReceiver(mDunReceiver, filter);
        }
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {

        mInterrupted = false;

        if (mAdapter == null) {
            Log.w(TAG, "Device does not have BT or device is not ready");
            // Release all resources
            closeDunService();
        } else {
            if (intent != null) {
                parseIntent(intent);
            }
        }
        return START_NOT_STICKY;
    }

    @Override
    public void onDestroy() {
        if (VERBOSE) Log.v(TAG, "onDestroy");

        super.onDestroy();

        if(mDunDevices != null)
            mDunDevices.clear();

        closeDunService();
        if(mDunHandler != null) {
            mDunHandler.removeCallbacksAndMessages(null);
            Looper looper = mDunHandler.getLooper();
            if (looper != null) {
                looper.quit();
                Log.i(TAG, "Quit looper");
            }

            mDunHandler = null;
            Log.i(TAG, "Remove Handler");
        }
    }

    @Override
    public IBinder onBind(Intent intent) {
        if (VERBOSE) Log.v(TAG, "onBind");
        return mDunBinder;
    }

    private IBinder initBinder() {
        return new BluetoothDunBinder(this);
    }

    private final class DunServiceMessageHandler extends Handler {
        private DunServiceMessageHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Log.i(TAG, "MessageHandler: " +  msg.what);
            switch (msg.what) {
                case MESSAGE_START_LISTENER:
                    if (mAdapter.isEnabled()) {
                        startRfcommListenerThread();
                    }
                    break;
                case MESSAGE_DUN_USER_TIMEOUT:
                    synchronized(mAuthLock) {
                        if (!mIsWaitingAuthorization) {
                            // already handled, ignore it
                            break;
                        }

                        mIsWaitingAuthorization = false;
                    }
                    Intent intent = new Intent(USER_CONFIRM_TIMEOUT_ACTION);
                    intent.setPackage(THIS_PACKAGE_NAME);
                    intent.putExtra(BluetoothDevice.EXTRA_DEVICE, mRemoteDevice);
                    intent.addFlags(Intent.FLAG_RECEIVER_FOREGROUND);
                    sendBroadcast(intent, BLUETOOTH_ADMIN_PERM);
                    removeDunNotification(DUN_NOTIFICATION_ID_ACCESS);
                    /* close the rfcomm socket and restart the listener thread */
                    closeRfcommSocket();
                    startRfcommListenerThread();

                    if (VERBOSE) Log.v(TAG, "Authorization Timeout");
                    break;
                case MESSAGE_HIDL_DAEMON_DEAD:
                    Log.i(TAG, " MESSAGE_HIDL_DAEMON_DEAD cookie = " + msg.obj
                            + " mRadioConfigProxyCookie = "
                            + mBluetoothDunServerProxyCookie.get());
                    if ((long) msg.obj == mBluetoothDunServerProxyCookie.get()) {
                        mBluetoothDunServerProxy = null;
                    }
                    break;

                default:
                    if (VERBOSE)
                        Log.v(TAG, " MessageHandler: " +  msg.what + " not handled");
                    break;
            }
        }
    };

    // process the intent from DUN receiver
    private void parseIntent(final Intent intent) {

        String action = intent.getAction();
        Log.i(TAG, "action :" + action);
        if (action == null) return;  /* Nothing to do */

        boolean removeTimeoutMsg = true;
        if (action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)
                || action.equals(Intent.ACTION_BOOT_COMPLETED)) {
            int state = getState();
            if (VERBOSE) Log.v(TAG, "parseIntent: state: " + state);
            if ((state == BluetoothAdapter.STATE_ON) && (!mDunEnable)) {
                synchronized(mConnection) {
                    try {
                        if (mAdapterService == null) {
                            Intent bindIntent = new Intent(IBluetooth.class.getName());
                            ComponentName comp = bindIntent.resolveSystemService(getPackageManager(), 0);
                            bindIntent.setComponent(comp);
                            if (comp == null || !bindServiceAsUser(bindIntent, mConnection, 0,
                                    android.os.Process.myUserHandle())) {
                                Log.e(TAG, "Could not bind with " + bindIntent);
                                return;
                            }
                        }
                    } catch (Exception e) {
                        Log.e(TAG, "bindService Exception", e);
                        return;
                    }
                }

                if (mHidlSupported) {
                    Log.v(TAG, "get proxy object of hidl daemon");
                    try {
                        if(getBluetoothDunServerProxy()){
                           mDunEnable = true;
                        }
                    } catch (RuntimeException e) {
                        Log.v(TAG, "Could not dun server hidl proxy object: " + e);
                    }
                } else {
                    Log.v(TAG, "Starting server process");
                    try {
                        SystemProperties.set(BLUETOOTH_DUN_PROFILE_STATUS, "running");
                        mDunEnable = true;
                    } catch (RuntimeException e) {
                        Log.v(TAG, "Could not start server process: " + e);
                    }
                }

                if (mDunEnable) {
                    mDunHandler.sendMessage(mDunHandler
                            .obtainMessage(MESSAGE_START_LISTENER));
                } else {
                    //DUN server process is not started successfully.
                    //So clean up service connection to avoid service connection leak
                    if (mAdapterService != null) {
                        try {
                            mAdapterService = null;
                            unbindService(mConnection);
                        } catch (IllegalArgumentException e) {
                            Log.e(TAG, "could not unbind the adapter Service", e);
                        }
                    }
                    return;
                }
            } else if (state == BluetoothAdapter.STATE_TURNING_OFF) {
                // Send any pending timeout now, as this service will be destroyed.
                if (mDunHandler.hasMessages(MESSAGE_DUN_USER_TIMEOUT)) {
                    Intent timeoutIntent = new Intent(BluetoothDevice.ACTION_CONNECTION_ACCESS_CANCEL);
                    timeoutIntent.setClassName(ACCESS_AUTHORITY_PACKAGE, ACCESS_AUTHORITY_CLASS);
                    timeoutIntent.addFlags(Intent.FLAG_RECEIVER_FOREGROUND);
                    sendBroadcast(timeoutIntent, BLUETOOTH_ADMIN_PERM);
                }

                if (mDunEnable) {
                    if (mHidlSupported) {
                        Log.i(TAG, "closing proxy with dun server hidl interface");
                        try {
                            if (mBluetoothDunServerProxy != null) {
                                mBluetoothDunServerProxy.close_server();
                                mBluetoothDunServerProxy = null;
                            }
                        } catch (RemoteException ex) {
                            Log.e(TAG, "close_server exception: " + ex.toString());
                        }
                    } else {
                        Log.i(TAG, "Stopping dun server process");
                        try {
                            SystemProperties.set(BLUETOOTH_DUN_PROFILE_STATUS, "stopped");
                        } catch (RuntimeException e) {
                            Log.e(TAG, "Could not stop process: " + e);
                        }
                    }

                    synchronized(mConnection) {
                        try {
                            mAdapterService = null;
                            unbindService(mConnection);
                        } catch (Exception e) {
                            Log.e(TAG, "", e);
                        }
                    }

                    // Release all resources
                    closeDunService();
                    mDunEnable = false;
                }
                synchronized(mAuthLock) {
                    if (mIsWaitingAuthorization) {
                        removeDunNotification(DUN_NOTIFICATION_ID_ACCESS);
                        mIsWaitingAuthorization = false;
                    }
                }

            } else {
                removeTimeoutMsg = false;
            }
        } else if (action.equals(BluetoothDevice.ACTION_ACL_DISCONNECTED) &&
                                 mIsWaitingAuthorization) {
            if (mRemoteDevice == null) {
               Log.e(TAG, "Unexpected error!");
               return;
            }
            BluetoothDevice device =
                intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
            if (device != null && device.equals(mRemoteDevice)) {
                if (mDunHandler != null) {
                   /* Let the user timeout handle this case as well */
                   mDunHandler.sendMessage(mDunHandler.obtainMessage(MESSAGE_DUN_USER_TIMEOUT));
                   removeTimeoutMsg = false;
                }
            }
        } else if (action.equals(DUN_ACCESS_ALLOWED_ACTION)) {

            if (mRemoteDevice == null) {
               Log.e(TAG, "Unexpected error!");
               return;
            }

            synchronized(mAuthLock) {
                if (!mIsWaitingAuthorization) {
                    // this reply is not for us
                    return;
                }

                mIsWaitingAuthorization = false;
            }

            if (intent.getBooleanExtra(BluetoothDunService.DUN_EXTRA_ALWAYS_ALLOWED, false) == true) {
                  if(mRemoteDevice != null) {
                     setDunAccessPermission(mRemoteDevice,
                        BluetoothDevice.ACCESS_ALLOWED);
                     Log.v(TAG, "setDunAccessPermission() ACCESS_ALLOWED " +
                        mRemoteDevice.getName());
                  }
            }

            /* start the uplink thread */
            startUplinkThread();

        } else if (action.equals(DUN_ACCESS_DISALLOWED_ACTION)) {
            /* close the rfcomm socket and restart the listener thread */
            Log.d(TAG,"DUN_ACCESS_DISALLOWED_ACTION:" + mIsWaitingAuthorization);

            mIsWaitingAuthorization = false;
            closeRfcommSocket();

            if (mDunHandler != null) {
                mDunHandler.sendMessage(mDunHandler.obtainMessage(MESSAGE_START_LISTENER));
            }
        } else if (BluetoothDevice.ACTION_BOND_STATE_CHANGED.equals(action)) {

            if (intent.hasExtra(BluetoothDevice.EXTRA_DEVICE)) {
                BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                if(device != null)
                    Log.d(TAG,"device: "+ device.getName());

               if ((device != null) &&
                (intent.getIntExtra(BluetoothDevice.EXTRA_BOND_STATE,
                               BluetoothDevice.BOND_NONE) == BluetoothDevice.BOND_NONE)) {
                   Log.d(TAG,"BOND_STATE_CHANGED REFRESH trustDevices "+ device.getName());
                   setDunAccessPermission(device, BluetoothDevice.ACCESS_UNKNOWN);
               }
            }

        } else {
            removeTimeoutMsg = false;
        }

        if (removeTimeoutMsg && mDunHandler != null) {
            mDunHandler.removeMessages(MESSAGE_DUN_USER_TIMEOUT);
        }
    }

    private int getState() {
        return (mAdapter != null) ? mAdapter.getState() : BluetoothAdapter.ERROR;
    }

    private void startRfcommListenerThread() {
        Log.i(TAG, "startRfcommListenerThread " + mAcceptThread);

        synchronized(mAcceptLock) {
            // if it is already running,close it
            if (mAcceptThread != null) {
                try {
                    mAcceptThread.shutdown();
                    mAcceptThread.join();
                    mAcceptThread = null;
                } catch (InterruptedException ex) {
                    Log.w(TAG, "mAcceptThread close error" + ex);
                }
            }
            if (mAcceptThread == null) {
                mAcceptThread = new SocketAcceptThread();
                mAcceptThread.setName("BluetoothDunAcceptThread");
                mAcceptThread.start();
            }
        }
    }

    private void stopRfcommListenerThread() {
        Log.i(TAG, "stopRfcommListenerThread " + mAcceptThread);

        synchronized(mAcceptLock) {
            if (mAcceptThread != null) {
                try {
                    mAcceptThread.shutdown();
                    mAcceptThread.join();
                    mAcceptThread = null;
                } catch (InterruptedException ex) {
                    Log.w(TAG, "mAcceptThread close error" + ex);
                }
            }
        }
    }

    private void startUplinkThread() {
        Log.i(TAG, "startUplinkThread: mHidlSupported: " + mHidlSupported);

        synchronized(mUplinkLock) {
            if (mHidlSupported) {
                if (mUplinkHIDLThread != null) {
                    try {
                        mUplinkHIDLThread.shutdown();
                        mUplinkHIDLThread.join();
                        mUplinkHIDLThread = null;
                    } catch (InterruptedException ex) {
                        Log.w(TAG, "mUplinkHIDLThread close error" + ex);
                        mUplinkHIDLThread = null;
                    }
                }

                if (mUplinkHIDLThread == null) {
                    mUplinkHIDLThread = new UplinkHIDLThread();
                    mUplinkHIDLThread.setName("BluetoothDunUplinkHIDLThread");
                    mUplinkHIDLThread.start();
                }
            } else {
                if (mUplinkThread != null) {
                    try {
                        mUplinkThread.shutdown();
                        mUplinkThread.join();
                        mUplinkThread = null;
                    } catch (InterruptedException ex) {
                        Log.w(TAG, "mUplinkThread close error" + ex);
                        mUplinkThread = null;
                    }
                }

                if (mUplinkThread == null) {
                    mUplinkThread = new UplinkThread();
                    mUplinkThread.setName("BluetoothDunUplinkThread");
                    mUplinkThread.start();
                }
            }
        }
    }

    private void stopUplinkThread() {
        Log.i(TAG, "stopUplinkThread: mHidlSupported: " + mHidlSupported);

        synchronized(mUplinkLock) {
            if (mHidlSupported) {
                if (mUplinkHIDLThread != null) {
                    try {
                        mUplinkHIDLThread.shutdown();
                        mUplinkHIDLThread.join();
                        mUplinkHIDLThread = null;
                    } catch (InterruptedException ex) {
                        Log.w(TAG, "mUplinkHIDLThread close error" + ex);
                        mUplinkHIDLThread = null;
                    }
                }
            } else {
                if (mUplinkThread != null) {
                    try {
                        mUplinkThread.shutdown();
                        mUplinkThread.join();
                        mUplinkThread = null;
                    } catch (InterruptedException ex) {
                        Log.w(TAG, "mUplinkThread close error" + ex);
                        mUplinkThread = null;
                    }
                }
            }
        }
    }

    private void startDownlinkThread() {
        Log.i(TAG, "startDownlinkThread: mHidlSupported: " + mHidlSupported);

        synchronized(mDownlinkLock) {
            if (mHidlSupported) {
                if (mDownlinkHIDLThread == null) {
                    HandlerThread thread = new HandlerThread("BluetoothDunHIDLThread");
                    thread.start();
                    Looper looper = thread.getLooper();
                    mDownlinkHIDLThread = new DownlinkHIDLThread(this, looper);
                }
            } else {
                if (mDownlinkThread == null) {
                    mDownlinkThread = new DownlinkThread();
                    mDownlinkThread.setName("BluetoothDunDownlinkThread");
                    mDownlinkThread.start();
                }
            }
        }
    }


    private void stopDownlinkThread() {
        Log.i(TAG, "stopDownlinkThread: mHidlSupported: " + mHidlSupported);
        synchronized (mDownlinkLock) {
            if (mHidlSupported) {
                if (mDownlinkHIDLThread != null) {
                    // Perform cleanup in Handler running on worker Thread
                    mDownlinkHIDLThread.removeCallbacksAndMessages(null);
                    Looper looper = mDownlinkHIDLThread.getLooper();
                    if (looper != null) {
                        looper.quit();
                        if (VERBOSE) {
                            Log.i(TAG, "Quit looper");
                        }
                    }
                    mDownlinkHIDLThread = null;
                }
            } else {
                if (mDownlinkThread != null) {
                    try {
                        mDownlinkThread.shutdown();
                        mDownlinkThread.join();
                        mDownlinkThread = null;
                    } catch (InterruptedException ex) {
                        Log.w(TAG, "mDownlinkThread close error" + ex);
                        mDownlinkThread = null;
                    }
                }
            }
        }
    }

    private void startMonitorThread() {
        Log.i(TAG, "startMonitorThread");

        synchronized(mMonitorLock) {
            if (mMonitorThread  == null) {
                mMonitorThread  = new MonitorThread();
                mMonitorThread .setName("BluetoothDunMonitorThread");
                mMonitorThread .start();
            }
        }
    }

    private void stopMonitorThread() {
        Log.i(TAG, "stopMonitorThread");

        synchronized(mMonitorLock) {
            if (mMonitorThread != null) {
                try {
                    mMonitorThread.shutdown();
                    mMonitorThread.join();
                    mMonitorThread = null;
                } catch (InterruptedException ex) {
                    Log.w(TAG, "mMonitorThread close error" + ex);
                    mMonitorThread = null;
                }
            }
        }
    }

    private final boolean initRfcommSocket() {
        if (VERBOSE) Log.v(TAG, "initRfcommSocket");

        boolean initRfcommSocketOK = false;
        final int CREATE_RETRY_TIME = 10;

        // It's possible that create will fail in some cases. retry for 10 times
        for (int i = 0; i < CREATE_RETRY_TIME && !mInterrupted; i++) {
            initRfcommSocketOK = true;
            try {
                // start listening for incoming rfcomm channel connection
                mListenSocket = mAdapter.listenUsingRfcommWithServiceRecord("Dial up Networking", DUN.getUuid());

            } catch (IOException e) {
                Log.e(TAG, "Error create RfcommListenSocket " + e.toString());
                initRfcommSocketOK = false;
            } catch (SecurityException e) {
                Log.e(TAG, "initRfcommServerSocket failed " + e.toString());
                initRfcommSocketOK = false;
                break;
            }
            if (!initRfcommSocketOK) {
                // Need to break out of this loop if BT is being turned off.
                if (mAdapter == null) break;
                int state = mAdapter.getState();
                if ((state != BluetoothAdapter.STATE_TURNING_ON) && (state != BluetoothAdapter.STATE_ON)) {
                    Log.w(TAG, "initRfcommSocket failed as BT is (being) turned off");
                    break;
                }
                try {
                    if (VERBOSE) Log.v(TAG, "wait 300 ms");
                    Thread.sleep(300);
                } catch (InterruptedException e) {
                    Log.e(TAG, "socketAcceptThread thread was interrupted");
                    break;
                }
            } else {
                break;
            }
        }

        if (mInterrupted) {
            initRfcommSocketOK = false;
            //close the listen socket to avoid resource leakage
            closeListenSocket();
        }

        if (initRfcommSocketOK && mListenSocket != null) {
            Log.d(TAG, "Succeed to create listening socket ");

        } else {
            Log.e(TAG, "Error to create listening socket after " + CREATE_RETRY_TIME + " try");
        }
        return initRfcommSocketOK;
    }

    private final boolean initDundClientSocket() {
        if (VERBOSE) Log.v(TAG, "initDundClientSocket");

        boolean initDundSocketOK = false;

        try {
            LocalSocketAddress locSockAddr = new LocalSocketAddress(DUN_SERVER);
            mDundSocket = new LocalSocket();
            mDundSocket.connect(locSockAddr);
            initDundSocketOK = true;
        } catch (java.io.IOException e) {
            Log.e(TAG, "cant connect: " + e);
        }

        if (initDundSocketOK) {
            if (VERBOSE) Log.v(TAG, "Succeed to create Dund socket ");

        } else {
            Log.e(TAG, "Error to create Dund socket");
        }
        return initDundSocketOK;
    }

    private final synchronized void closeListenSocket() {
        // exit SocketAcceptThread early
        Log.d(TAG, "Close listen Socket : ");
        if (mListenSocket != null) {
            try {
                // this will cause mListenSocket.accept() return early with
                // IOException
                mListenSocket.close();
                mListenSocket = null;
            } catch (IOException ex) {
                Log.e(TAG, "Close listen Socket error: " + ex);
            }
        }
    }

    private final synchronized void closeRfcommSocket() {
        Log.d(TAG, "Close rfcomm Socket : ");
        if (mRfcommSocket != null) {
            try {
                mRfcommSocket.close();
                mRfcommSocket = null;
            } catch (IOException e) {
                Log.e(TAG, "Close Rfcomm Socket error: " + e.toString());
            }
        }
    }

    private final synchronized void closeDundSocket() {
        Log.d(TAG, "Close dund  Socket : ");
        if (mDundSocket != null) {
            try {
                mDundSocket.close();
                mDundSocket = null;
            } catch (IOException e) {
                Log.e(TAG, "Close Dund Socket error: " + e.toString());
            }
        }
    }

    private final void closeDunService() {
        Log.d(TAG, "closeDunService");

        // exit initRfcommSocket early
        mInterrupted = true;
        try {
            if (mDunReceiver != null) {
                unregisterReceiver(mDunReceiver);
                mDunReceiver = null;
            }
        } catch (Exception e) {
            Log.w(TAG, "Unable to unregister dun receiver", e);
        }

        closeListenSocket();

        closeRfcommSocket();

        if (mHidlSupported == false) {
          closeDundSocket();
        }

        stopMonitorThread();

        stopDownlinkThread();

        stopUplinkThread();
        /* remove the MESSAGE_START_LISTENER message which might be posted
         * by the uplink thread */
        mDunHandler.removeMessages(MESSAGE_START_LISTENER);

        stopRfcommListenerThread();

        stopSelf();

        Log.d(TAG, "closeDunService out");
    }

    private void createDunNotification(BluetoothDevice device) {
        if (VERBOSE) Log.v(TAG, "Creating DUN access notification");

        NotificationManager nm = (NotificationManager)
                getSystemService(Context.NOTIFICATION_SERVICE);
        if (mNotificationChannel == null) {
            mNotificationChannel = new NotificationChannel(DUN_NOTIFICATION_CHANNEL,
                    getString(R.string.dun_notification_group),
                    NotificationManager.IMPORTANCE_HIGH);
            nm.createNotificationChannel(mNotificationChannel);
        }


        // Create an intent triggered by clicking on the status icon.
        Intent clickIntent = new Intent();
        clickIntent.setClass(this, BluetoothDunPermissionActivity.class);
        clickIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
        clickIntent.setAction(DUN_ACCESS_REQUEST_ACTION);
        clickIntent.putExtra(EXTRA_BLUETOOTH_DEVICE, device);

        // Create an intent triggered by clicking on the
        // "Clear All Notifications" button
        Intent deleteIntent = new Intent();
        deleteIntent.setClass(this, BluetoothDunReceiver.class);

        Notification notification = null;
        String name = device.getName();
        if (TextUtils.isEmpty(name)) {
            name = getString(R.string.defaultname);
        }

        deleteIntent.setAction(DUN_ACCESS_DISALLOWED_ACTION);

        notification = new Notification.Builder(this, DUN_NOTIFICATION_CHANNEL)
                .setContentTitle(getString(R.string.dun_notif_ticker))
                .setTicker(getString(R.string.dun_notif_ticker))
                .setContentText(getString(R.string.dun_notif_message, name))
                .setSmallIcon(android.R.drawable.stat_sys_data_bluetooth)
                .setAutoCancel(true)
                .setOnlyAlertOnce(true)
                .setContentIntent(PendingIntent.getActivity(this, 0, clickIntent, 0))
                .setDeleteIntent(PendingIntent.getBroadcast(this, 0, deleteIntent, 0))
                .setColor(this.getColor(
                        com.android.internal.R.color.system_notification_accent_color))
                .setOnlyAlertOnce(true)
                .build();
        notification.flags |= Notification.FLAG_NO_CLEAR; // Cannot be set with the builder.
        nm.notify(DUN_NOTIFICATION_ID_ACCESS, notification);

        if (VERBOSE) Log.v(TAG, "Awaiting Authorization : DUN Connection : " + device.getName());

    }

    private void removeDunNotification(int id) {
        Context context = getApplicationContext();
        NotificationManager nm = (NotificationManager) context
                .getSystemService(Context.NOTIFICATION_SERVICE);
        nm.cancel(id);
    }

    /**
     * A thread that runs in the background waiting for remote rfcomm
     * connect.Once a remote socket connected, this thread shall be
     * shutdown.When the remote disconnect,this thread shall run again
     * waiting for next request.
     */
    private class SocketAcceptThread extends Thread {

        private boolean stopped = false;

        @Override
        public void run() {
            BluetoothServerSocket listenSocket;
            if (mListenSocket == null) {
                if (!initRfcommSocket()) {
                    return;
                }
            }

            while (!stopped) {
                try {
                    listenSocket = mListenSocket;
                    if (listenSocket == null) {
                        Log.w(TAG, "mListenSocket is null");
                        break;
                    }
                    Log.d(TAG, "Listening for rfcomm socket connection...");
                    mRfcommSocket = mListenSocket.accept();

                    synchronized (BluetoothDunService.this) {
                        if (mRfcommSocket == null) {
                            Log.w(TAG, " mRfcommSocket is null");
                            break;
                        }
                        mRemoteDevice = mRfcommSocket.getRemoteDevice();
                    }
                    if (mRemoteDevice == null) {
                        /* close the rfcomm socket */
                        closeRfcommSocket();
                        Log.i(TAG, "getRemoteDevice() = null");
                        break;
                    }
                    int trust = BluetoothDevice.ACCESS_UNKNOWN;

                    if (mRemoteDevice != null)
                        trust = getDunAccessPermission(mRemoteDevice);
                    if (VERBOSE) Log.v(TAG, "getDunAccessPermission() = " + trust);

                    if (trust == BluetoothDevice.ACCESS_ALLOWED) {
                        /* start the uplink thread */
                        startUplinkThread();
                    } else if (trust == BluetoothDevice.ACCESS_UNKNOWN) {
                        createDunNotification(mRemoteDevice);

                        if (VERBOSE) Log.v(TAG, "incoming connection accepted from: "+ mRemoteDevice);
                        mIsWaitingAuthorization = true;

                        mDunHandler.sendMessageDelayed(mDunHandler.obtainMessage(MESSAGE_DUN_USER_TIMEOUT),
                                 USER_CONFIRM_TIMEOUT_VALUE);
                    }
                    stopped = true; // job done ,close this thread;
                    Log.i(TAG, "SocketAcceptThread stopped ");
                } catch (IOException | NullPointerException ex) {
                    stopped=true;
                    Log.w(TAG, "Handled Accept thread exception: " + ex.toString());
                }
            }
        }

        void shutdown() {
            stopped = true;
            interrupt();
        }
    }

    /**
     * A thread that runs in the background waiting for remote DUNd socket
     * connection.Once the connection is established, this thread starts the
     * downlink thread and starts forwarding the DUN profile requests to
     * the DUNd on receiving the requests from rfcomm channel.
     */
    private class UplinkThread extends Thread {

        private boolean stopped = false;
        private boolean IntExit = false;
        private boolean mIsATDReceived = false;
        private boolean mDunArbitrationStarted = false;
        private OutputStream mDundOutputStream = null;
        private InputStream mRfcommInputStream = null;
        ByteBuffer IpcMsgBuffer = ByteBuffer.allocate(DUN_MAX_IPC_MSG_LEN);
        private int NumRead = 0;
        private byte TempByte = 0;

        @Override
        public void run() {
            if (mDundSocket == null) {
                if (!initDundClientSocket()) {
                    /* close the rfcomm socket to avoid resource leakage */
                    closeRfcommSocket();
                    /*restart the listener thread */
                    mDunHandler.sendMessage(mDunHandler.obtainMessage(MESSAGE_START_LISTENER));
                    return;
                }
            }

            if (mDundSocket != null && mDundSocket.isConnected()) {
                try {
                    mDundOutputStream = mDundSocket.getOutputStream();
                } catch (IOException ex) {
                    Log.w(TAG, "Handled mDundOutputStream exception: " + ex.toString());
                }
            } else {
                Log.w(TAG, "UplinkThread: Dund Socket is not connected: " +  mDundSocket);
                return;
            }

            if (mRfcommSocket != null && mRfcommSocket.isConnected()) {
                try {
                    mRfcommInputStream = mRfcommSocket.getInputStream();
                } catch (IOException ex) {
                    Log.w(TAG, "Handled mRfcommInputStream exception: " + ex.toString());
                }
            } else {
                Log.w(TAG, "UplinkThread: rfcomm Socket is not connected: " +  mRfcommSocket);
                return;
            }

            // create the downlink thread for reading the responses
            //from DUN server process
            startDownlinkThread();
            // create the modem status monitor thread
            startMonitorThread();

            while (!stopped) {
                try {
                    if (VERBOSE)
                        Log.v(TAG, "Reading the DUN request from Rfcomm channel");
                    /* Read the DUN request from Rfcomm channel */
                    NumRead = mRfcommInputStream.read(IpcMsgBuffer.array(), DUN_IPC_MSG_OFF_MSG,
                                                                                DUN_MAX_MSG_LEN);
                    if (NumRead < 0) {
                        IntExit = true;
                        break;
                    } else if (NumRead != 0) {

                        if (mIsATDReceived == false && isATDCommand(IpcMsgBuffer, NumRead)) {
                            mIsATDReceived = true;
                            ConnectivityManager cm = (ConnectivityManager)
                                    getSystemService(Context.CONNECTIVITY_SERVICE);
                            // Disable Mobile data call
                            if (cm != null) {
                                if (cm.getMobileDataEnabled()) {
                                    mDunArbitrationStarted = enableDataConnectivity(false);
                                } else {
                                    mDunArbitrationStarted = false;
                                    Log.i(TAG, "Embedded data call was already disabled");
                                }
                            } else {
                                Log.e(TAG, "ConnectivityManager service interface is null");
                            }
                        }

                        /* Write the same DUN request to the DUN server socket with
                           some additional parameters */
                        IpcMsgBuffer.put(DUN_IPC_MSG_OFF_MSG_TYPE, DUN_IPC_MSG_DUN_REQUEST);
                        IpcMsgBuffer.putShort(DUN_IPC_MSG_OFF_MSG_LEN, (short)NumRead);
                        /* swap bytes of message len as buffer is Big Endian */
                        TempByte = IpcMsgBuffer.get(DUN_IPC_MSG_OFF_MSG_LEN);

                        IpcMsgBuffer.put( DUN_IPC_MSG_OFF_MSG_LEN, IpcMsgBuffer.get(DUN_IPC_MSG_OFF_MSG_LEN + 1));

                        IpcMsgBuffer.put(DUN_IPC_MSG_OFF_MSG_LEN + 1, TempByte);
                        try {
                            mDundOutputStream.write(IpcMsgBuffer.array(), 0, NumRead + DUN_IPC_HEADER_SIZE);
                        } catch (IOException ex) {
                            break;
                        }
                    }
                } catch (IOException ex) {
                    IntExit = true;
                    Log.w(TAG, "Handled Rfcomm channel Read exception: " + ex.toString());
                    break;
                }
            }
            // send the disconneciton request immediate to Dun server
            if (IntExit) {
                disconnect(mRemoteDevice);
            }

            /* close the dund socket */
            closeDundSocket();
            /* close the rfcomm socket */
            closeRfcommSocket();
            // Intimate the Settings APP about the disconnection
            handleDunDeviceStateChange(mRemoteDevice, BluetoothProfile.STATE_DISCONNECTED);

            /* wait for dun downlink thread to close */
                if (VERBOSE) Log.v(TAG, "wait for dun downlink thread to close");
                stopDownlinkThread();

            /* wait for dun monitor thread to close */
                if (VERBOSE) Log.v(TAG, "wait for dun monitor thread to close ");
                stopMonitorThread();

            if (mDunArbitrationStarted) {
                mDunArbitrationStarted = enableDataConnectivity(true);
            }

            if (IntExit && !stopped) {
                if (VERBOSE) Log.v(TAG, "starting the listener thread ");
                /* start the listener thread */
                mDunHandler.sendMessage(mDunHandler.obtainMessage(MESSAGE_START_LISTENER));
            }
            /* reset the modem status */
            mRmtMdmStatus = 0x00;
            Log.d(TAG, "uplink thread exited");
        }

        void shutdown() {
            stopped = true;
            interrupt();
        }
    }

    /**
      * A thread that runs in the background waiting for remote Rfcomm socket
      * connection.Once the connection is established, this thread starts the
      * downlink hidl thread and starts forwarding the DUN profile requests to
      * the DUN HIDL server on receiving the requests from rfcomm channel.
      */
     private class UplinkHIDLThread extends Thread {
         private boolean stopped = false;
         private boolean IntExit = false;
         private boolean mIsATDReceived = false;
         private boolean mDunArbitrationStarted = false;
         private InputStream mRfcommInputStream = null;
         ByteBuffer IpcMsgBuffer = ByteBuffer.allocate(DUN_HIDL_MAX_MSG_LEN);
         private int NumRead = 0;

         @Override
         public void run() {
            /* create the downlink hidl thread for reading the responses
                      from DUN server hidl daemon */
            startDownlinkThread();

             try {
               if (mBluetoothDunServerProxy != null) {
                    mBluetoothDunServerProxy.sendCtrlMsg(CtrlMsg.DUN_CONNECT_REQ);
                    mDunConnectSignal = new CountDownLatch(1);
                    Log.i(TAG, "Waiting for dun server hidl connect response");
                    try {
                        mDunConnectSignal.await(2, TimeUnit.SECONDS);
                    } catch (InterruptedException e) {
                        mIsDunHIDLConnected = false;
                        Log.e(TAG, "Interrupt received while waiting to connect", e);
                    }
                }
             } catch (RemoteException ex) {
                 Log.e(TAG, "sendCtrlMsg exception: " + ex.toString());
                 mIsDunHIDLConnected = false;
             }

             if(mIsDunHIDLConnected == false) {
                 /* close the rfcomm socket to avoid resource leakage */
                 closeRfcommSocket();
                 /*restart the listener thread */
                 mDunHandler.sendMessage(mDunHandler.obtainMessage(MESSAGE_START_LISTENER));
                 stopDownlinkThread();
                 return;
             }

             if (mRfcommSocket != null && mRfcommSocket.isConnected()) {
                 try {
                     mRfcommInputStream = mRfcommSocket.getInputStream();
                 } catch (IOException ex) {
                     Log.w(TAG, "Handled mRfcommInputStream exception: " + ex.toString());
                 }
             } else {
                 Log.w(TAG, "UplinkThread: rfcomm Socket is not connected: " +  mRfcommSocket);
                 return;
             }

             // create the modem status monitor thread
             startMonitorThread();

             while (!stopped) {
                 try {
                     if (VERBOSE)
                         Log.v(TAG, "Reading the DUN request from Rfcomm channel");
                     /* Read the DUN request from Rfcomm channel */
                     NumRead = mRfcommInputStream.read(IpcMsgBuffer.array(), 0,
                            DUN_HIDL_MAX_MSG_LEN);

                     if (NumRead < 0) {
                         IntExit = true;
                         break;
                     } else if (NumRead != 0) {
                         if (mIsATDReceived == false && isATDCommand(IpcMsgBuffer, NumRead)) {
                             mIsATDReceived = true;
                             ConnectivityManager cm = (ConnectivityManager)
                                     getSystemService(Context.CONNECTIVITY_SERVICE);
                             // Disable Mobile data call
                             if (cm != null) {
                                 if (cm.getMobileDataEnabled()) {
                                     mDunArbitrationStarted = enableDataConnectivity(false);
                                 } else {
                                     mDunArbitrationStarted = false;
                                     Log.i(TAG, "Embedded data call was already disabled");
                                 }
                             } else {
                                 Log.e(TAG, "ConnectivityManager service interface is null");
                             }
                         }
                     }
                     try {
                         if (mBluetoothDunServerProxy != null && mIsDunHIDLConnected) {
                             mBluetoothDunServerProxy.sendUplinkData(getByteArrayListFromByteArray(
                                    IpcMsgBuffer.array(), NumRead));
                         } else {
                            Log.i(TAG, "UplinkHIDLThread:: mIsDunHIDLConnected: "
                                  + mIsDunHIDLConnected);
                            IntExit = true;
                            break;
                         }
                     } catch (RemoteException ex) {
                         Log.e(TAG, "sendUplinkData exception: " + ex.toString());
                         IntExit = true;
                         break;
                     }
                 } catch (IOException ex) {
                     IntExit = true;
                     Log.w(TAG, "Handled Rfcomm channel Read exception: " + ex.toString());
                     break;
                 }
             }
             // send the disconneciton request immediate to Dun server
             if (IntExit && mIsDunHIDLConnected) {
                 disconnect(mRemoteDevice);
             }

             mIsDunHIDLConnected = false;

             /* close the rfcomm socket */
             closeRfcommSocket();
             // Intimate the Settings APP about the disconnection
             handleDunDeviceStateChange(mRemoteDevice, BluetoothProfile.STATE_DISCONNECTED);

             /* wait for dun downlink thread to close */
                 if (VERBOSE) Log.v(TAG, "wait for dun downlink thread to close");
                 stopDownlinkThread();

             /* wait for dun monitor thread to close */
                 if (VERBOSE) Log.v(TAG, "wait for dun monitor thread to close ");
                 stopMonitorThread();

             if (mDunArbitrationStarted) {
                 mDunArbitrationStarted = enableDataConnectivity(true);
             }

             if (mIsDunHIDLConnected == false) {
                 if (VERBOSE) Log.v(TAG, "starting the listener thread ");
                 /* start the listener thread */
                 mDunHandler.sendMessage(mDunHandler.obtainMessage(MESSAGE_START_LISTENER));
             }
             /* reset the modem status */
             mRmtMdmStatus = 0x00;
             Log.d(TAG, "uplink thread exited");
         }

         void shutdown() {
             stopped = true;
             interrupt();
         }
     }

    /**
     * A thread that runs in the background and monitors the modem status of rfcomm
     * channel and when there is a change in the modem status it will update the
     * DUN server process with the new status.
     */
    private class MonitorThread extends Thread {

        private boolean stopped = false;
        private int len = 0;
        private byte modemStatus = 0;
        ByteBuffer IpcMsgBuffer = ByteBuffer.allocate(DUN_IPC_MDM_STATUS_MSG_SIZE);
        private byte TempByte = 0;

        byte mdmBits = 0;

        @Override
        public void run() {

            while (!stopped) {
                try {
                    //wait for 200 ms
                    Thread.sleep(MON_THREAD_SLEEP_INTERVAL);
                } catch (InterruptedException e) {
                    Log.e(TAG, "MonitorThread thread was interrupted");
                    break;
                }
                try {
                    if ((mRfcommSocket == null) || (!mRfcommSocket.isConnected())) {
                        Log.w(TAG, "MonitorThread: rfcomm Socket is not connected: "
                                + mRfcommSocket);
                        break;
                    }
                    len = mRfcommSocket.getSocketOpt(BTSOCK_OPT_GET_MODEM_BITS, IpcMsgBuffer.array());
                    if(len != DUN_IPC_MDM_STATUS_MSG_SIZE)  {
                        Log.w(TAG, "getSocketOpt return mismatch len: " + len);
                        continue;
                    }
                } catch (IOException ex) {
                    Log.w(TAG, "Handled getSocketOpt Exception: " + ex.toString());
                }
                if( mdmBits != IpcMsgBuffer.get(0)) {
                    mdmBits = IpcMsgBuffer.get(0);
                    notifyModemStatus(mdmBits);
                }
            }
            Log.d(TAG, "MonitorThread thread exited");
        }

        void shutdown() {
            stopped = true;
            interrupt();
        }
    }


    /**
     * A thread that runs in the background and starts forwarding the
     * DUN profile responses received from Dund to the rfcomm channel.
     */
    private class DownlinkThread extends Thread {

        private boolean stopped = false;
        private OutputStream mRfcommOutputStream = null;
        private InputStream mDundInputStream = null;
        ByteBuffer IpcMsgBuffer = ByteBuffer.allocate(2*DUN_MAX_IPC_MSG_LEN);
        private int NumRead = 0;
        private int ReadIndex = 0;
        private short MsgLen = 0;
        private byte TempByte = 0;

        @Override
        public void run() {

            if (mDundSocket != null && mDundSocket.isConnected()) {
                try {
                    mDundInputStream = mDundSocket.getInputStream();
                } catch (IOException ex) {
                    Log.w(TAG, "Handled mDundInputStream exception: " + ex.toString());
                }
            } else {
                Log.w(TAG, "DownlinkThread: Dund Socket is not connected: " +  mDundSocket);
                return;
            }

            if (mRfcommSocket != null && mRfcommSocket.isConnected()) {
                try {
                    mRfcommOutputStream = mRfcommSocket.getOutputStream();
                } catch (IOException ex) {
                    Log.w(TAG, "Handled mRfcommOutputStream exception: " + ex.toString());
                }
            } else {
                Log.w(TAG, "DownlinkThread: rfcomm Socket is not connected: " +  mRfcommSocket);
                return;
            }

            while (!stopped) {
                try {
                    /* Read the DUN responses from DUN server */
                    NumRead = mDundInputStream.read(IpcMsgBuffer.array(),0, DUN_MAX_IPC_MSG_LEN);
                    Log.d(TAG, "Read the DUN response from Dund, NumRead: " + NumRead);
                    if (NumRead < 0) {
                        break;
                    } else if(NumRead != 0) {
                        /* some times read buffer contains multiple responses */
                        do {
                            if (VERBOSE) Log.v(TAG, "ReadIndex: " + ReadIndex);
                            /* swap bytes of message len as buffer is Big Endian */
                            TempByte = IpcMsgBuffer.get(ReadIndex + DUN_IPC_MSG_OFF_MSG_LEN);

                            IpcMsgBuffer.put(ReadIndex + DUN_IPC_MSG_OFF_MSG_LEN, IpcMsgBuffer.get(ReadIndex +
                                         DUN_IPC_MSG_OFF_MSG_LEN + 1));

                            IpcMsgBuffer.put(ReadIndex + DUN_IPC_MSG_OFF_MSG_LEN + 1, TempByte);

                            MsgLen =  IpcMsgBuffer.getShort(ReadIndex + DUN_IPC_MSG_OFF_MSG_LEN);

                            /* Max DUN msg/response length including IPC header is 32767,
                               which can only be read from the DunD socket in single read.
                               As msg/response length is varied & single msg/response can
                               be smaller than max length, So multiple msgs/responses are
                               read in single read. Boundary condition hits, when max length
                               is already read, which include multiple responses, but some of
                               the bytes of last response still need to be read. */

                            if((ReadIndex + MsgLen + DUN_IPC_HEADER_SIZE) > NumRead) {
                                int tempNumRead = 0;
                                tempNumRead = mDundInputStream.read(IpcMsgBuffer.array(), NumRead,
                                        ReadIndex + MsgLen + DUN_IPC_HEADER_SIZE - NumRead);
                                Log.w(TAG, "Boundary condition hit, ReadIndex: " + ReadIndex
                                        + " MsgLen: " + MsgLen + " NumRead: " + NumRead
                                        + " tempNumRead: " + tempNumRead);
                                if (tempNumRead >= 0)
                                    NumRead += tempNumRead;
                                else
                                    break;
                            }

                            if (IpcMsgBuffer.get(ReadIndex + DUN_IPC_MSG_OFF_MSG_TYPE) == DUN_IPC_MSG_DUN_RESPONSE) {
                                try {
                                    mRfcommOutputStream.write(IpcMsgBuffer.array(), ReadIndex + DUN_IPC_MSG_OFF_MSG,
                                           IpcMsgBuffer.getShort(ReadIndex + DUN_IPC_MSG_OFF_MSG_LEN));
                                } catch (IOException ex) {
                                    stopped = true;
                                    break;
                                }
                                if (VERBOSE)
                                    Log.v(TAG, "DownlinkThread Msg written to Rfcomm");
                            }
                            else if (IpcMsgBuffer.get(ReadIndex + DUN_IPC_MSG_OFF_MSG_TYPE) == DUN_IPC_MSG_CTRL_RESPONSE) {

                                if (IpcMsgBuffer.get(ReadIndex + DUN_IPC_MSG_OFF_MSG) == DUN_CRTL_MSG_CONNECTED_RESP) {
                                    handleDunDeviceStateChange(mRemoteDevice, BluetoothProfile.STATE_CONNECTED);
                                }
                                else if (IpcMsgBuffer.get(ReadIndex + DUN_IPC_MSG_OFF_MSG) == DUN_CRTL_MSG_DISCONNECTED_RESP) {
                                    handleDunDeviceStateChange(mRemoteDevice, BluetoothProfile.STATE_DISCONNECTED);
                                }
                                if (VERBOSE)
                                    Log.v(TAG, "DownlinkThread control message received");
                            }
                            else if (IpcMsgBuffer.get(ReadIndex + DUN_IPC_MSG_OFF_MSG_TYPE) == DUN_IPC_MSG_MDM_STATUS) {
                                /* handle the modem status messages */
                                handleModemStatusChange(IpcMsgBuffer.get(ReadIndex + DUN_IPC_MSG_OFF_MSG));
                                if (VERBOSE)
                                    Log.v(TAG, "DownlinkThread modem status message received");
                            }
                            ReadIndex = ReadIndex + MsgLen + DUN_IPC_HEADER_SIZE;
                        } while(ReadIndex > 0 && ReadIndex < NumRead);
                    }

                } catch (IOException ex) {
                    stopped = true;
                    Log.w(TAG, "Handled Dund Read exception: " + ex.toString());
                }
                /* reset the ReadIndex */
                ReadIndex = 0;
            }
            /* close the dund socket */
            closeDundSocket();
            /* close the rfcomm socket */
            closeRfcommSocket();

            Log.d(TAG, "downlink thread exited ");
        }

        void shutdown() {
            stopped = true;
            interrupt();
        }
    }


    /**
     * A thread that runs in the background and starts forwarding the
     * DUN profile responses received from Dun Server HIDL to the rfcomm channel.
     */
      private final class DownlinkHIDLThread extends Handler {
          Context mContxt;
          private OutputStream mRfcommOutputStream = null;

          private DownlinkHIDLThread(Context contxt, Looper looper) {
              super(looper);
              mContxt = contxt;
              if (mRfcommSocket != null && mRfcommSocket.isConnected()) {
                try {
                    mRfcommOutputStream = mRfcommSocket.getOutputStream();
                } catch (IOException ex) {
                    Log.w(TAG, "Handled mRfcommOutputStream exception: " + ex.toString());
                }
            } else {
                Log.w(TAG, "DownlinkThread: rfcomm Socket is not connected: " +  mRfcommSocket);
                return;
            }
          }

           @Override
          public void handleMessage(Message msg) {
              if (VERBOSE) Log.v(TAG, "Handler(): got msg=" + msg.what);
              switch (msg.what) {
                  case BluetoothDunServerResponse.DUN_CONNECT_RESP:
                      boolean status = (boolean)msg.obj;
                      Log.i(TAG, "DUN_CONNECT_RESP ");

                      if (status) {
                          mIsDunHIDLConnected = true;
                          handleDunDeviceStateChange(mRemoteDevice,
                                  BluetoothProfile.STATE_CONNECTED);
                      } else {
                          mIsDunHIDLConnected = false;
                          handleDunDeviceStateChange(mRemoteDevice,
                                  BluetoothProfile.STATE_DISCONNECTED);
                      }

                      if (mDunConnectSignal != null) {
                          mDunConnectSignal.countDown();
                          mDunConnectSignal = null;
                      }
                      break;
                  case BluetoothDunServerResponse.DUN_DISCONNECT_RESP:
                      Log.i(TAG, "DUN_DISCONNECT_RESP ");
                      mIsDunHIDLConnected = false;
                      if (mUplinkHIDLThread != null) {
                          mUplinkHIDLThread.shutdown();
                      }
                      break;
                  case BluetoothDunServerResponse.MODEM_STATUS_CHANGE_EVENT:
                      Log.i(TAG, "MODEM_STATUS_CHANGE_EVENT ");
                      handleModemStatusChange((byte)msg.obj);
                      break;
                  case BluetoothDunServerResponse.DOWNLINK_DATA_EVENT:
                      try {
                          byte [] data = (byte[])msg.obj;
                          mRfcommOutputStream.write(data, 0, data.length);
                          if (VERBOSE) Log.v(TAG, "DownlinkThread Msg written to Rfcomm len"
                                  + data.length);
                      } catch (IOException ex) {
                          Log.w(TAG, "Handled mRfcommOutputStream exception: "
                                  + ex.toString());
                          break;
                      }
                      break;
                  default:
                    break;
             }
           }
      };

    private class BluetoothDunServerResponse extends IBluetoothDunServerResponse.Stub {

        private static final String TAG = "BluetoothDunServerResponse";
        public static final int DUN_CONNECT_RESP              = 0x01;
        public static final int DUN_DISCONNECT_RESP           = 0x02;
        public static final int MODEM_STATUS_CHANGE_EVENT     = 0x03;
        public static final int DOWNLINK_DATA_EVENT           = 0x04;
        public BluetoothDunServerResponse(){
        }

        /**
         * Invoked when control message is received from the
         * DUN HIDL daemon.
         */
        public void ctrlMsgEvent(int msgType, byte status) {
          if (mDownlinkHIDLThread != null) {
             Log.v(TAG, "ctrlMsgEvent: msgType: " + msgType + " status: " + status);
              if(msgType == CtrlMsg.DUN_CONNECT_RESP) {
                  Message msg = mDownlinkHIDLThread.obtainMessage(DUN_CONNECT_RESP,
                      status == Status.SUCCESS ? true:false);
                  msg.sendToTarget();
              } else {
                  Message msg = mDownlinkHIDLThread.obtainMessage(DUN_DISCONNECT_RESP);
                  msg.sendToTarget();
              }
           }
        }

        /**
         * This function is invoked when downlink data is received from the
         * DUN HIDL daemon.
         */
        public void downlinkDataEvent(java.util.ArrayList<Byte> data){
          if (mDownlinkHIDLThread != null) {
              byte[] byte_data = getbyteArrayFromArrayList(data);
              Message msg = mDownlinkHIDLThread.obtainMessage(DOWNLINK_DATA_EVENT, byte_data);
              msg.sendToTarget();
          }
       }

        /**
         * This function is invoked when modem status change is received from the
         * DUN HIDL daemon.
         */
        public void modemStatusChangeEvent(byte status){
          if (mDownlinkHIDLThread != null) {
              Log.i(TAG, "modemStatusChangeEvent: status: " + status);
              Message msg = mDownlinkHIDLThread.obtainMessage(MODEM_STATUS_CHANGE_EVENT, status);
              msg.sendToTarget();
           }
        }
    }

    class BluetoothDunServerDeathRecipient implements HwBinder.DeathRecipient {
        @Override
        public void serviceDied(long cookie) {
            // Deal with service going away
            Log.w(TAG, " serviceDied");
            mDunHandler.sendMessage(mDunHandler.obtainMessage(MESSAGE_HIDL_DAEMON_DEAD, cookie));
        }
    }

    private boolean isATDCommand(ByteBuffer rfcommData, int dataLen) {
        String[] atdCommands = {"ATDT*98", "ATDT*99", "ATDT#777", "ATD*98", "ATD*99",
                "ATD#777"};
        String temp;
        if (mHidlSupported) {
            temp =   new String(rfcommData.array(),0,dataLen);
        } else {
             temp =   new String(rfcommData.array(),
                rfcommData.arrayOffset() + DUN_IPC_MSG_OFF_MSG,
                dataLen);
        }

        Log.i(TAG, "rfcomm data: " + temp);
        if (temp != null) {
            for (int i = 0; i < atdCommands.length; i++) {
                if (temp.contains(atdCommands[i])) {
                    Log.w(TAG, "ATD Received");
                    return true;
                }
            }
        }
        return false;
    }

    private boolean enableDataConnectivity(boolean value) {
        boolean result = false;
        int RETRY_COUNT = 3;

        if (mTelephonyService == null) {
            Log.e(TAG, "Telephony Service interface is null");
            return result;
        }

        if (value) {
            try {
                int retry_count = RETRY_COUNT;
                while (retry_count > 0) {
                    result = mTelephonyService.enableDataConnectivity();
                    if (result == true) {
                        Log.i(TAG, "Success: enableDataConnectivity");
                        break;
                    } else {
                        Log.i(TAG, "Failure: enableDataConnectivity");
                        retry_count--;
                    }
                }
            } catch (RemoteException e) {
                Log.e(TAG, "Exception - Data Call Not Enabled");
            }
        } else if (value == false) {
            try {
                int retry_count = RETRY_COUNT;
                while (retry_count > 0) {
                    result = mTelephonyService.disableDataConnectivity();
                    if (result == true) {
                        Log.i(TAG, "Success: disableDataConnectivity");
                        break;
                    } else {
                        Log.i(TAG, "Failure: disableDataConnectivity");
                        retry_count--;
                    }
                }
            } catch (RemoteException e) {
                Log.e(TAG, "Exception - Data Call Not Disabled");
            }
       }
       return result;
    }

    boolean disconnect(BluetoothDevice device) {
        Log.i(TAG, "disconnect: mHidlSupported: " + mHidlSupported);
        enforceCallingOrSelfPermission(BLUETOOTH_PERM, "Need BLUETOOTH permission");

        if (mHidlSupported) {
            try {
              Log.i(TAG, "disconnect: mIsDunHIDLConnected: " + mIsDunHIDLConnected);
              if ((mBluetoothDunServerProxy != null) && mIsDunHIDLConnected)
                mBluetoothDunServerProxy.sendCtrlMsg(CtrlMsg.DUN_DISCONNECT_REQ);
            } catch (RemoteException ex) {
                Log.e(TAG, "sendCtrlMsg exception: " + ex.toString());
            }
        } else {
            OutputStream mDundOutputStream = null;
            int WriteLen = DUN_IPC_HEADER_SIZE + DUN_IPC_CTRL_MSG_SIZE;
            ByteBuffer IpcMsgBuffer = ByteBuffer.allocate(WriteLen);

            if (mDundSocket != null && mDundSocket.isConnected()) {
                try {
                    mDundOutputStream = mDundSocket.getOutputStream();
                } catch (IOException ex) {
                    Log.w(TAG, "disconnect: Handled mDundOutputStream exception: "
                            + ex.toString());
                }
            } else {
                Log.w(TAG, "disconnect: Dund Socket is not connected: "
                        +  mDundSocket);
                return false;
            }

            IpcMsgBuffer.put(DUN_IPC_MSG_OFF_MSG_TYPE, DUN_IPC_MSG_CTRL_REQUEST);
            IpcMsgBuffer.putShort(DUN_IPC_MSG_OFF_MSG_LEN,DUN_IPC_CTRL_MSG_SIZE);
            IpcMsgBuffer.put(DUN_IPC_MSG_OFF_MSG, DUN_CRTL_MSG_DISCONNECT_REQ);
            try {
                if (mDundOutputStream != null)
                    mDundOutputStream.write(IpcMsgBuffer.array(), 0, WriteLen);
            } catch (IOException ex) {
                Log.w(TAG, "disconnect: Handled mDundOutputStream write exception: "
                        + ex.toString());
            }
        }
        return true;
    }

    boolean notifyModemStatus(byte status) {
        Log.i(TAG, "notifyModemStatus: mHidlSupported: " + mHidlSupported
                + ", status: " + status);
        enforceCallingOrSelfPermission(BLUETOOTH_PERM, "Need BLUETOOTH permission");

        if (mHidlSupported) {
            try {
                if (mBluetoothDunServerProxy != null && mIsDunHIDLConnected) {
                    mBluetoothDunServerProxy.sendModemStatus(status);
                } else {
                    Log.i(TAG, "notifyModemStatus: mIsDunHIDLConnected: "
                            + mIsDunHIDLConnected);
                }
            } catch (RemoteException ex) {
                Log.e(TAG, "notifyModemStatus exception: " + ex.toString());
            }
        } else {
            OutputStream mDundOutputStream = null;
            int WriteLen = DUN_IPC_HEADER_SIZE + DUN_IPC_MDM_STATUS_MSG_SIZE;
            ByteBuffer IpcMsgBuffer = ByteBuffer.allocate(WriteLen);
            Log.d(TAG, "notifyModemStatus: status: " + status);

            if (mDundSocket != null && mDundSocket.isConnected()) {
                try {
                    mDundOutputStream = mDundSocket.getOutputStream();
                } catch (IOException ex) {
                    Log.w(TAG, "notifyModemStatus: mDundOutputStream exception: "
                            + ex.toString());
                }
            } else {
                Log.w(TAG, "notifyModemStatus: Dund Socket is not connected: "
                        +  mDundSocket);
                return false;
            }

            IpcMsgBuffer.put(DUN_IPC_MSG_OFF_MSG_TYPE, DUN_IPC_MSG_MDM_STATUS);
            IpcMsgBuffer.putShort(DUN_IPC_MSG_OFF_MSG_LEN,DUN_IPC_MDM_STATUS_MSG_SIZE);
            IpcMsgBuffer.put(DUN_IPC_MSG_OFF_MSG, status);
            try {
                if (mDundOutputStream != null)
                    mDundOutputStream.write(IpcMsgBuffer.array(), 0, WriteLen);
            } catch (IOException ex) {
                Log.e(TAG, "Handled mDundOutputStream write exception: " + ex.toString());
            }
        }
        return true;
    }


    private boolean getBluetoothDunServerProxy() {
        try {
            try {
                mBluetoothDunServerProxy= IBluetoothDunServer.getService(true);
                Log.e(TAG, "getBluetoothDunServerProxy " + mBluetoothDunServerProxy);
            } catch (NoSuchElementException e) {
                Log.e(TAG, "getBluetoothDunServerProxy: " + e);
                return false;
            }

            mDunServerResponse = new BluetoothDunServerResponse();
            mBluetoothDunServerProxy.initialize(mDunServerResponse);
            // Link to death recipient and set response. If fails, set proxy to null and return.
            mBluetoothDunServerProxy.linkToDeath(mBluetoothDunServerDeathRecipient,
                    mBluetoothDunServerProxyCookie.incrementAndGet());
        } catch (RemoteException | RuntimeException e) {
            mBluetoothDunServerProxy = null;
            Log.e(TAG, "getBluetoothDunServerProxy: " + e);
            return false;
        }
        return true;
    }

    /**
     * Convert from an array list of Byte to an array of primitive bytes.
     */
    public byte[] getbyteArrayFromArrayList(java.util.ArrayList<Byte> bytes) {
        byte[] byteArray = new byte[bytes.size()];
        int i = 0;
        for (Byte b : bytes) {
            byteArray[i++] = b;
        }
        return byteArray;
    }

    /**
     * Convert from an array of primitive bytes to an array list of Byte.
     */
    public ArrayList<Byte> getByteArrayListFromByteArray(byte[] bytes, int NumRead) {
        ArrayList<Byte> byteList = new ArrayList<>();
        for (Byte b : bytes) {
            if (NumRead == 0) {
              return byteList;
            }
            byteList.add(b);
            NumRead-- ;
        }
        return byteList;
    }

    int getDunAccessPermission(BluetoothDevice device) {
        enforceCallingOrSelfPermission(BLUETOOTH_PERM, "Need BLUETOOTH permission");
        SharedPreferences pref = getSharedPreferences(DUN_ACCESS_PERMISSION_PREFERENCE_FILE,
                Context.MODE_PRIVATE);
        if (!pref.contains(device.getAddress())) {
            return BluetoothDevice.ACCESS_UNKNOWN;
        }
        return pref.getBoolean(device.getAddress(), false)
                ? BluetoothDevice.ACCESS_ALLOWED : BluetoothDevice.ACCESS_REJECTED;
    }

    boolean setDunAccessPermission(BluetoothDevice device, int value) {
        enforceCallingOrSelfPermission(BLUETOOTH_PRIVILEGED,
                                       "Need BLUETOOTH PRIVILEGED permission");
        SharedPreferences pref = getSharedPreferences(DUN_ACCESS_PERMISSION_PREFERENCE_FILE,
                Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = pref.edit();
        if (value == BluetoothDevice.ACCESS_UNKNOWN) {
            editor.remove(device.getAddress());
        } else {
            editor.putBoolean(device.getAddress(), value == BluetoothDevice.ACCESS_ALLOWED);
        }
        return editor.commit();
    }

    int getConnectionState(BluetoothDevice device) {
        BluetoothDunDevice dunDevice = mDunDevices.get(device);
        if (dunDevice == null) {
            return BluetoothDun.STATE_DISCONNECTED;
        }
        return dunDevice.mState;
    }

    public void notifyProfileConnectionStateChanged(BluetoothDevice device,
            int profileId, int newState, int prevState) {
        if (mAdapterService != null) {
            try {
                mAdapterService.sendConnectionStateChange(device, profileId, newState, prevState);
            }catch (RemoteException re) {
                Log.e(TAG, "",re);
            }
        }
    }

    private final void handleDunDeviceStateChange(BluetoothDevice device, int state) {
        int prevState;
        BluetoothDunDevice dunDevice = mDunDevices.get(device);
        if (dunDevice == null) {
            prevState = BluetoothProfile.STATE_DISCONNECTED;
        } else {
            prevState = dunDevice.mState;
        }
        Log.d(TAG, "handleDunDeviceStateChange device: " + device + " preState: " + prevState
                + " state: " + state);
        if (prevState == state) return;

        if (dunDevice == null) {
            dunDevice = new BluetoothDunDevice(state);
            mDunDevices.put(device, dunDevice);
        } else {
            dunDevice.mState = state;
        }

        /* Notifying the connection state change of the profile before sending
           the intent for connection state change, as it was causing a race
           condition, with the UI not being updated with the correct connection
           state. */
        notifyProfileConnectionStateChanged(device, BluetoothProfile.DUN, state, prevState);
        Intent intent = new Intent(BluetoothDun.ACTION_CONNECTION_STATE_CHANGED);
        intent.putExtra(BluetoothDevice.EXTRA_DEVICE, device);
        intent.putExtra(BluetoothDun.EXTRA_PREVIOUS_STATE, prevState);
        intent.putExtra(BluetoothDun.EXTRA_STATE, state);
        intent.addFlags(Intent.FLAG_RECEIVER_FOREGROUND);
        sendBroadcast(intent, BLUETOOTH_PERM);

    }

    private final void handleModemStatusChange(byte status) {
        byte mdmBits = 0;
        int WriteLen = DUN_IPC_MDM_STATUS_MSG_SIZE;
        ByteBuffer IpcMsgBuffer = ByteBuffer.allocate(WriteLen);

        if (VERBOSE) Log.v(TAG, "handleModemStatusChange  status: " + status);

        if ((mRfcommSocket == null) || (!mRfcommSocket.isConnected())) {
            Log.w(TAG, "rfcomm socket is not connected" + mRfcommSocket);
            return;
        }
        if(mRmtMdmStatus != status) {
            mdmBits = (byte)((~(int)mRmtMdmStatus) & ((int)mRmtMdmStatus ^ (int)status));
            if (VERBOSE) Log.v(TAG, "handleModemStatusChange  mdmBits " + mdmBits);
            if(mdmBits > 0) {
                IpcMsgBuffer.put(0, mdmBits);
                try {
                    mRfcommSocket.setSocketOpt(BTSOCK_OPT_SET_MODEM_BITS, IpcMsgBuffer.array(), WriteLen);
                } catch (IOException ex) {
                    Log.w(TAG, "Handled getSocketOpt Exception: " + ex.toString());
                }
            }
            mdmBits = (byte)(((int)mRmtMdmStatus) & ((int)mRmtMdmStatus ^ (int)status));
            if (VERBOSE) Log.v(TAG, "handleModemStatusChange  mdmBits " + mdmBits);
            if(mdmBits > 0) {
                IpcMsgBuffer.put(0, mdmBits);
                try {
                    mRfcommSocket.setSocketOpt(BTSOCK_OPT_CLR_MODEM_BITS, IpcMsgBuffer.array(), WriteLen);
                } catch (IOException ex) {
                     Log.w(TAG, "Handled getSocketOpt Exception: " + ex.toString());
                }
            }
            mRmtMdmStatus = status;
        }
    }

    List<BluetoothDevice> getConnectedDevices() {
        enforceCallingOrSelfPermission(BLUETOOTH_PERM, "Need BLUETOOTH permission");
        List<BluetoothDevice> devices = getDevicesMatchingConnectionStates(
                new int[] {BluetoothProfile.STATE_CONNECTED});
        return devices;
    }

    List<BluetoothDevice> getDevicesMatchingConnectionStates(int[] states) {
         enforceCallingOrSelfPermission(BLUETOOTH_PERM, "Need BLUETOOTH permission");
        List<BluetoothDevice> dunDevices = new ArrayList<BluetoothDevice>();

        for (BluetoothDevice device: mDunDevices.keySet()) {
            int dunDeviceState = getConnectionState(device);
            for (int state : states) {
                if (state == dunDeviceState) {
                    dunDevices.add(device);
                    break;
                }
            }
        }
        return dunDevices;
    }

    private ServiceConnection mConnection = new ServiceConnection() {
        public void onServiceConnected(ComponentName className, IBinder service) {
            mAdapterService = IBluetooth.Stub.asInterface(service);
        }

        public void onServiceDisconnected(ComponentName className) {
            mAdapterService = null;
        }
    };

    /**
     * Handlers for incoming service calls
     */
    private static class BluetoothDunBinder extends IBluetoothDun.Stub {

        private BluetoothDunService mService;

        public BluetoothDunBinder(BluetoothDunService svc) {
            mService = svc;
        }

        private BluetoothDunService getService() {
            if (mService != null)
                return mService;
            return null;
        }

        public boolean disconnect(BluetoothDevice device) {
            BluetoothDunService service = getService();
            if (service == null) return false;
            return service.disconnect(device);
        }

        public int getConnectionState(BluetoothDevice device) {
            BluetoothDunService service = getService();
            if (service == null) return BluetoothDun.STATE_DISCONNECTED;
            return service.getConnectionState(device);
        }
        public List<BluetoothDevice> getConnectedDevices() {
            BluetoothDunService service = getService();
            if (service == null) return new ArrayList<BluetoothDevice>(0);
            return service.getConnectedDevices();
        }

        public List<BluetoothDevice> getDevicesMatchingConnectionStates(int[] states) {
            BluetoothDunService service = getService();
            if (service == null) return new ArrayList<BluetoothDevice>(0);
            return service.getDevicesMatchingConnectionStates(states);
        }
    };

    private class BluetoothDunDevice {
        private int mState;

        BluetoothDunDevice(int state) {
            mState = state;
        }
    }
}
