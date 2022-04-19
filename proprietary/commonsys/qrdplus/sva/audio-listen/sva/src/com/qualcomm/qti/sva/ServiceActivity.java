/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva;

import android.app.Activity;
import android.content.ComponentName;
import android.content.ServiceConnection;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;

import com.qualcomm.qti.sva.controller.Msg;
import com.qualcomm.qti.sva.utils.LogUtils;

public abstract class ServiceActivity extends Activity {
    private final String TAG = ServiceActivity.class.getSimpleName();

    private Messenger mServerMessenger;
    private boolean mServiceConnected;

    private ServiceConnection mServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName componentName, IBinder iBinder) {
            LogUtils.d(TAG, "enter onServiceConnected");
            mServiceConnected = true;
            mServerMessenger = new Messenger(iBinder);
            onServiceConnectedCallback();
        }

        @Override
        public void onServiceDisconnected(ComponentName componentName) {
            LogUtils.d(TAG, "enter onServiceDisconnected");
            mServiceConnected = false;
            onServiceDisconnectedCallback();
            mServerMessenger = null;
        }
    };

    protected ServiceConnection getServiceConnection() {
        return mServiceConnection;
    }

    protected boolean isServiceConnected() {
        return mServiceConnected;
    }

    protected void onServiceConnectedCallback() {
    }

    protected void onServiceDisconnectedCallback() {
    }

    protected Messenger getServerMessenger() {
        return mServerMessenger;
    }

    protected abstract Messenger getClientMessenger();

    protected void sendRequest(int what, Object obj) {
        if (null == getServerMessenger()) {
            LogUtils.e(TAG, "sendRequest: getServerMessenger is null");
            return;
        }

        Message msg = Message.obtain(null, what, obj);
        msg.replyTo = getClientMessenger();
        try {
            getServerMessenger().send(msg);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    protected void sendRequest(int what, int msgArg1) {
        if (null == getServerMessenger()) {
            LogUtils.e(TAG, "sendRequest: getServerMessenger is null");
            return;
        }

        Message msg = Message.obtain(null, what);
        msg.replyTo = getClientMessenger();
        msg.arg1 = msgArg1;
        try {
            getServerMessenger().send(msg);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    protected void sendRequest(int what, int msgArg1, int msgArg2) {
        if (null == getServerMessenger()) {
            LogUtils.e(TAG, "sendRequest: getServerMessenger is null");
            return;
        }

        Message msg = Message.obtain(null, what);
        msg.replyTo = getClientMessenger();
        msg.arg1 = msgArg1;
        msg.arg2 = msgArg2;
        try {
            getServerMessenger().send(msg);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    protected void sendRequest(int what, Object obj, int msgArg1, int msgArg2) {
        if (null == getServerMessenger()) {
            LogUtils.e(TAG, "sendRequest: getServerMessenger is null");
            return;
        }

        Message msg = Message.obtain(null, what, obj);
        msg.replyTo = getClientMessenger();
        msg.arg1 = msgArg1;
        msg.arg2 = msgArg2;
        try {
            getServerMessenger().send(msg);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    protected void sendRequest(Message msg) {
        if (null == getServerMessenger()) {
            LogUtils.e(TAG, "sendRequest: getServerMessenger is null");
            return;
        }

        try {
            msg.replyTo = getClientMessenger();
            getServerMessenger().send(msg);
        } catch (RemoteException e) {
            e.printStackTrace();
        }
    }

    public void registerClient(Enum clientId) {
        LogUtils.v(TAG, "registerClient: clientId = " + clientId);
        sendRequest(Msg.MSG_REGISTER_CLIENT_REQ, clientId);
    }

    public void deRegisterClient(Enum clientId) {
        LogUtils.v(TAG, "deRegisterClient: clientId = " + clientId);
        sendRequest(Msg.MSG_DE_REGISTER_CLIENT_REQ, clientId);
    }
}
