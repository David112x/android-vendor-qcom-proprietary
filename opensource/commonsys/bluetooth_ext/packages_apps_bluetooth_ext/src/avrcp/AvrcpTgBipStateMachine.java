/*
 * Copyright (c) 2018, The Linux Foundation. All rights reserved.
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

import java.io.IOException;

import javax.obex.ServerSession;
import android.annotation.NonNull;
import com.android.bluetooth.BluetoothObexTransport;
import com.android.internal.util.StateMachine;
import com.android.internal.util.State;

import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothProfile;
import android.bluetooth.BluetoothSocket;
import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.Looper;
import android.os.Message;
import android.util.Log;
/**
 * Bluetooth Avrcp TG BIP StateMachine
 *              (New connection socket)
 *          CONNECTED   ----->  FINISHED
 *                (OBEX Server done)
 */
public class AvrcpTgBipStateMachine extends StateMachine {
    private String TAG = "AvrcpBipSM";
    private final boolean DEBUG = true;
    static final int DISCONNECT = 1;
    private Context mCtx;
    private final Finished mFinished = new Finished();
    private final Connected mConnected = new Connected();
    private BipStateBase mPrevState;
    private BluetoothDevice mRemoteDevice;
    private Handler mServiceHandler;
    private BluetoothSocket mConnSocket;
    private ServerSession mServerSession = null;
    private AvrcpBipRspObexServer mAvrcpBipRspServer;
    private String mTag;

    private AvrcpTgBipStateMachine(Context context, Looper looper,
            @NonNull BluetoothDevice device, @NonNull BluetoothSocket connSocket,
            Handler handler) {
        super("AvrcpTGBipStateMachine", looper);
        mRemoteDevice = device;
        mCtx = context;
        mServiceHandler = handler;
        mConnSocket = connSocket;
        setTag();
        addState(mFinished);
        addState(mConnected);
        setInitialState(mConnected);
    }

    static AvrcpTgBipStateMachine make(Context context, Looper looper,
            BluetoothDevice device, BluetoothSocket connSocket,
             Handler handler) {
        AvrcpTgBipStateMachine stateMachine = new AvrcpTgBipStateMachine(context,
                looper, device, connSocket, handler);
        stateMachine.start();
        return stateMachine;
    }

    private void setTag(){
        try {
            mTag = "_" + mRemoteDevice.getAddress().replaceAll(":","").substring(6,12);
            TAG += mTag;
        } catch (Exception e) {
            Log.w(TAG, "setTag failed ", e);
        }
    }

    BluetoothDevice getRemoteDevice() {
        return mRemoteDevice;
    }

    String getSmTag() {
        return mTag;
    }

    AvrcpBipRspObexServer getBipRsp() {
        return mAvrcpBipRspServer;
    }

    private abstract class BipStateBase extends State {
        /**
         * Get a state value from {@link BluetoothProfile} that represents the connection state of
         * this headset state
         *
         * @return a value in {@link BluetoothProfile#STATE_DISCONNECTED},
         * {@link BluetoothProfile#STATE_CONNECTING}, {@link BluetoothProfile#STATE_CONNECTED}, or
         * {@link BluetoothProfile#STATE_DISCONNECTING}
        */
        abstract int getConnectionStateInt();

        @Override
        public void enter() {
            // Crash if mPrevState is null and state is not Disconnected
            enforceValidConnectionStateTransition();
        }

        @Override
        public void exit() {
            mPrevState = this;
        }

        /**
         * Broadcast connection state change for this state machine
        */
        protected void broadcastStateTransitions() {
            int prevStateInt = BluetoothProfile.STATE_DISCONNECTED;
            if (mPrevState != null) {
                prevStateInt = mPrevState.getConnectionStateInt();
            }
            if (getConnectionStateInt() != prevStateInt) {
                stateLogD("connection state changed: " + mPrevState + " -> "
                        + this);
            } else {
                Log.v(TAG," INVALID TRA ");
            }
        }

        /**
         * Verify if the current state transition is legal by design. This is called from enter()
         * method and crash if the state transition is not expected by the state machine design.
         *
         * Note:
         * This method uses state objects to verify transition because these objects should be final
         * and any other instances are invalid
        */
        private void enforceValidConnectionStateTransition() {
            boolean isValidTransition = false;
            if (this == mFinished) {
                isValidTransition = mPrevState == mConnected;
            } else if (this == mConnected) {
                isValidTransition = mPrevState == mFinished;
            }
            if (!isValidTransition) {
                throw new IllegalStateException("Invalid state transition from "
                        + mPrevState + " to " + this + " for device " + mRemoteDevice);
            }
        }

        protected void stateLogD(String msg) {
            log(getName() + ": currentDevice=" + mRemoteDevice + ", msg=" + msg);
        }
    }

    class Finished extends BipStateBase {

        @Override
        int getConnectionStateInt() {
            return BluetoothProfile.STATE_DISCONNECTED;
        }

        @Override
        public void enter() {
            super.enter();
            if (DEBUG) Log.d(TAG, "Finished Enter");
            mAvrcpBipRspServer = null;
            stopObexServerSession();
            closeConnectionSocket();
            mServiceHandler.obtainMessage(AvrcpBipRsp.MSG_STATE_MACHINE_DONE,
                    AvrcpTgBipStateMachine.this).sendToTarget();
            broadcastStateTransitions();
            if (DEBUG) Log.d(TAG,"Finished DONE");
        }

        private void stopObexServerSession() {
            if (DEBUG) Log.d(TAG, "stopObexServerSession");
            if (mServerSession != null) {
                if (DEBUG) Log.d(TAG, "mServerSession exists - shutting it down...");
                mServerSession.close();
                mServerSession = null;
            }
        }

            private void closeConnectionSocket() {
                if (DEBUG) Log.d(TAG, "closeConnectionSocket");
                if (mConnSocket != null) {
                    try {
                        mConnSocket.close();
                    } catch (IOException e) {
                        Log.e(TAG, "Close Connection Socket error: ", e);
                    } finally {
                        mConnSocket = null;
                    }
                }
            }
        }

    class Connected extends BipStateBase {

        @Override
        int getConnectionStateInt() {
            return BluetoothProfile.STATE_CONNECTED;
        }

        @Override
        public void enter() {
            try {
                if (DEBUG) Log.d(TAG, "Connected Enter");
                startObexServerSession(mConnSocket);
            } catch (IOException ex) {
                Log.e(TAG, "Caught exception starting OBEX server session" + ex.toString());
            }
        }

        @Override
        public boolean processMessage(Message message) {
            switch (message.what) {
                case DISCONNECT:
                    if (DEBUG) {
                        Log.d(TAG, "DISCONNECT");
                    }
                    transitionTo(mFinished);
                    break;
            }
            return HANDLED;
        }

    }

    private synchronized void startObexServerSession(BluetoothSocket socket)
            throws IOException {
        if (DEBUG) Log.d(TAG, "startObexServerSession");
        mAvrcpBipRspServer = new AvrcpBipRspObexServer(mCtx, this);
        BluetoothObexTransport transport = new BluetoothObexTransport(socket);
        mServerSession = new ServerSession(transport, mAvrcpBipRspServer, null);
        if (DEBUG) Log.d(TAG, "startObexServerSession success");
    }
}
