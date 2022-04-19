/*
 * Copyright (c) 2015-2016, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *           * Redistributions of source code must retain the above copyright
 *             notice, this list of conditions and the following disclaimer.
 *           * Redistributions in binary form must reproduce the above
 *           * copyright notice, this list of conditions and the following
 *             disclaimer in the documentation and/or other materials provided
 *             with the distribution.
 *           * Neither the name of The Linux Foundation nor the names of its
 *             contributors may be used to endorse or promote products derived
 *             from this software without specific prior written permission.
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

package com.android.bluetooth.avrcp;

import com.android.bluetooth.IObexConnectionHandler;
import com.android.bluetooth.ObexServerSockets;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Message;
import android.os.Handler;
import android.os.HandlerThread;
import android.util.Log;

import java.util.HashMap;

public class AvrcpBipRsp implements IObexConnectionHandler {
    private final String TAG = "AvrcpBipRsp";

    private static final String LOG_TAG = "AvrcpBip";

    private static final boolean D = true;

    static boolean V = Log.isLoggable(LOG_TAG, Log.VERBOSE);

    private boolean mIsRegistered = false;

    // The handle to the socket registration with SDP
    private ObexServerSockets mServerSocket = null;

    private BluetoothAdapter mAdapter;

    private Context mContext;

    private final int START_LISTENER = 1;

    private final int STOP_LISTENER = 2;

    private final int BIP_L2CAP_PSM = 0x1021;

    private final HashMap<BluetoothDevice, AvrcpTgBipStateMachine> mBipStateMachineMap
            = new HashMap<>();

    private int mMaxBipDevices;

    protected static final int MSG_STATE_MACHINE_DONE = 5;

    private HandlerThread mHandlerThread;

    private final int BD_ADDR_LEN = 6; // bytes

    public AvrcpBipRsp (Context context, int maxBipConnections) {
        mContext = context;
        mMaxBipDevices = maxBipConnections;
        mAdapter = BluetoothAdapter.getDefaultAdapter();
        mHandlerThread = new HandlerThread("BipHandlerThread");
        mHandlerThread.start();
        mBipStateMachineMap.clear();
        if (D) Log.d(TAG, "mMaxBipDevices :" + mMaxBipDevices);
    }

    private final BroadcastReceiver mAvrcpBipRspReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (D) Log.d(TAG, "action: " + action);
            if (action == null)
                return;
            if (action.equals(BluetoothAdapter.ACTION_STATE_CHANGED)) {
                int state = intent.getIntExtra(BluetoothAdapter.EXTRA_STATE,
                                               BluetoothAdapter.ERROR);
                if (D) Log.d(TAG, "state:" + state);
                if (state == BluetoothAdapter.STATE_TURNING_OFF) {
                    stop();
                } else if (state == BluetoothAdapter.STATE_ON) {
                    // start ServerSocket listener threads
                    mSessionStatusHandler.sendMessage(mSessionStatusHandler
                            .obtainMessage(START_LISTENER));
                }
            }
        }
    };

    private final Handler mSessionStatusHandler = new Handler() {
        @Override
        public void handleMessage(Message msg) {
            if (D) Log.d(TAG, "Handler(): got msg=" + msg.what);
            switch (msg.what) {
                case START_LISTENER:
                    if (mAdapter != null && mAdapter.isEnabled()) {
                        startL2capListener();
                    } else {
                        Log.w(TAG, "Adapter is disabled, ignoring..");
                    }
                    break;
                case STOP_LISTENER:
                    closeServerSocket();
                    break;
                case MSG_STATE_MACHINE_DONE:
                    AvrcpTgBipStateMachine sm = (AvrcpTgBipStateMachine) msg.obj;
                    BluetoothDevice remoteDevice = sm.getRemoteDevice();
                     if (D) Log.d(TAG ,"sm: " + sm + " remoteDevice: " + remoteDevice);
                    sm.quitNow();
                    synchronized (mBipStateMachineMap) {
                        mBipStateMachineMap.remove(remoteDevice);
                    }
                    if (D) Log.d(TAG ,"MSG_STATE_MACHINE_DONE");
                    break;
            }
        }
    };

    private synchronized void startL2capListener() {
        if (D) Log.d(TAG, "startL2capListener");
        closeServerSocket();
        if(mServerSocket == null) {
            /* AVRCP 1.6 does not support obex over rfcomm */
            mServerSocket = ObexServerSockets.createWithFixedChannels(this, -1,
                              BIP_L2CAP_PSM);
            if(mServerSocket == null) {
                Log.e(TAG, "Failed to start the listener");
                return;
            }
        }
    }

    private final synchronized void closeServerSocket() {
        if(D) Log.d(TAG, "closeServerSocket");
        // Step 1, 2: clean up active server session and connection socket
        synchronized (mBipStateMachineMap) {
            for (AvrcpTgBipStateMachine stateMachine : mBipStateMachineMap.values()) {
                stateMachine.sendMessage(AvrcpTgBipStateMachine.DISCONNECT);
            }
        }
        // Step 3: clean up existing server sockets
        if (mServerSocket != null) {
            mServerSocket.shutdown(false);
            mServerSocket = null;
        }
    }

   void start() {
        IntentFilter filter = new IntentFilter();
        filter.addAction(BluetoothAdapter.ACTION_STATE_CHANGED);
        if (!mIsRegistered) {
            try {
                mContext.registerReceiver(mAvrcpBipRspReceiver, filter);
                mIsRegistered = true;
            } catch (Exception e) {
                Log.e(TAG,"Unable to register avrcpbip receiver",e);
            }
        } else {
            Log.w(TAG, "receiver already registered!!");
        }
    }

    synchronized boolean stop() {
        if (D) Log.d(TAG, "stop()");
        if (mIsRegistered) {
            unRegisterReceiver();
        } else {
            if (D) Log.d(TAG, "already stopped, returning");
            return true;
        }
        mSessionStatusHandler.sendMessage(mSessionStatusHandler
                .obtainMessage(STOP_LISTENER));
        if (D) Log.d(TAG, "returning from stop()");
        return true;
    }

    @Override
    public synchronized boolean onConnect(BluetoothDevice device, BluetoothSocket socket) {
        /* Signal to the service that we have received an incoming connection. */
        if (device == null || socket == null) {
            Log.e(TAG, "onConnect unexpected error, device: " + device + " socket: " + socket);
            return false;
        }
        int smCount = 0;
        synchronized (mBipStateMachineMap) {
            smCount = mBipStateMachineMap.size();
        }
        if (D) Log.d(TAG, "onConnect device :" + device + " smCount:" + smCount);
        if (smCount >= mMaxBipDevices) {
            Log.w(TAG, "Cannot connect to " + device + " reached to max size :" + smCount);
            return false;
        }
        AvrcpTgBipStateMachine sm = AvrcpTgBipStateMachine.make(mContext,
                mHandlerThread.getLooper(), device, socket,  mSessionStatusHandler);
        synchronized (mBipStateMachineMap) {
            mBipStateMachineMap.put(device, sm);
        }
        return true;
    }

    /**
     * Called when an unrecoverable error occurred in an accept thread.
     * Close down the server socket, and restart.
     * TODO: Change to message, to call start in correct context.
     */
    @Override
    public synchronized void onAcceptFailed() {
        Log.w(TAG, "onAcceptFailed, restarting the server socket");
        if (mSessionStatusHandler != null) {
            mSessionStatusHandler.removeCallbacksAndMessages(null);
        }
        mSessionStatusHandler.sendMessage(mSessionStatusHandler.obtainMessage
                (START_LISTENER));
    }

    BluetoothDevice getBluetoothDevice(byte[] address) {
        BluetoothDevice device = null;
        try {
            if (address != null && address.length == BD_ADDR_LEN) {
                device = mAdapter.getRemoteDevice(address);
            } else {
                Log.w(TAG, " getBluetoothDevice invalid address ");
            }
        } catch (RuntimeException e) {
            Log.w(TAG, " getBluetoothDevice " + e.toString());
        }
        return device;
    }

    synchronized String getImgHandleFromTitle(byte[] address, String title) {
        if(D) Log.d(TAG," getImgHandleFromTitle address :" + address + " title :" + title);
        if (address == null || address.length != BD_ADDR_LEN) {
            Log.w(TAG," retrun invalid getImgHandleFromTitle");
            return "";
        }
        BluetoothDevice device = getBluetoothDevice(address);
        AvrcpTgBipStateMachine bipSm = mBipStateMachineMap.get(device);
        if(bipSm != null) {
            AvrcpBipRspObexServer bipServer = bipSm.getBipRsp();
            if(bipServer != null) {
                return bipServer.getImgHandleFromTitle(title);
            } else {
                Log.w(TAG,"getImgHandleFromTitle bipServer null");
            }
        } else {
            Log.w(TAG,"getImgHandleFromTitle bipSm null");
        }
        return "";
    }

    synchronized String getImgHandle(BluetoothDevice device, String albumName) {
        if (D) Log.v(TAG," getImgHandle device :" + device + " albumName :" + albumName);
        if (device == null || albumName == null) {
            Log.w(TAG," retrun invalid getImgHandle ");
            return "";
        }
        AvrcpTgBipStateMachine bipSm = mBipStateMachineMap.get(device);
        if(bipSm != null) {
            AvrcpBipRspObexServer bipServer = bipSm.getBipRsp();
            if(bipServer != null) {
                return bipServer.getImgHandle(albumName);
            } else {
                Log.v(TAG,"getImgHandle bipServer null");
            }
        } else {
            Log.v(TAG,"getImgHandle bipSm null");
        }
        return "";
    }

    void disconnect(BluetoothDevice device) {
        AvrcpTgBipStateMachine bipSm = mBipStateMachineMap.get(device);
        if (D) Log.d(TAG, "disconnect device :" + device + " bipSm :" + bipSm);
        if( bipSm != null) {
            bipSm.sendMessage(AvrcpTgBipStateMachine.DISCONNECT);
        }
    }

    private void unRegisterReceiver() {
        try {
            mIsRegistered = false;
            mContext.unregisterReceiver(mAvrcpBipRspReceiver);
        } catch (Exception e) {
            Log.w(TAG," unRegisterReceiver ", e);
        }
    }

    private int getMaxDevices() {
        int max_supported_bip_connections = 5;
        int maxBipConnection = 1;
        try {
            boolean isAllowMultiBip = android.os.SystemProperties.
                    getBoolean("persist.vendor.bt.a2dp.multi_bip", false);
            maxBipConnection = (isAllowMultiBip) ? max_supported_bip_connections : maxBipConnection;
        } catch (SecurityException e) {
            Log.e(TAG, " getMaxDevices " + e.toString());
        }
        return maxBipConnection;
    }

    // Todo : Added to avaoid compilation issue
    public AvrcpBipRsp (Context context) {
        mContext = context;
        mMaxBipDevices = getMaxDevices();
        mAdapter = BluetoothAdapter.getDefaultAdapter();
        mHandlerThread = new HandlerThread("BipHandlerThread");
        mHandlerThread.start();
        mBipStateMachineMap.clear();
        if (D) Log.d(TAG, "mMaxBipDevices, ignore :" + mMaxBipDevices);
    }

    public String getImgHandle(String imgHandle){
        if (D) Log.d(TAG, "getImgHandle ignore ");
        return "";
    }

}
