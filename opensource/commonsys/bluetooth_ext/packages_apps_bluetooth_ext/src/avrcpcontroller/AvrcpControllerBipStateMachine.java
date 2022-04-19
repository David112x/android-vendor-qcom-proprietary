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
package com.android.bluetooth.avrcpcontroller;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothAvrcpController;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothProfile;
import android.content.Context;
import android.content.Intent;
import android.media.MediaMetadata;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.Process;
import android.os.SystemProperties;
import android.text.TextUtils;
import android.util.Log;
import com.android.bluetooth.R;
import com.android.internal.util.IState;
import com.android.internal.util.State;
import com.android.internal.util.StateMachine;
import com.android.bluetooth.btservice.ProfileService;
/**
 * Bluetooth AVRCP CONTROLLER BIP StateMachine * @hide
 */
public class AvrcpControllerBipStateMachine extends StateMachine {

    private static final boolean DBG = true;
    static final String TAG = "AvrcpControllerBipStateMachine";
    // messages from/to  AvrcpStateMachine
    public static final int MESSAGE_CONNECT_BIP                                  = 1;
    public static final int MESSAGE_DISCONNECT_BIP                               = 2;
    public static final int MESSAGE_FETCH_THUMBNAIL                              = 3;
    public static final int MESSAGE_FETCH_IMAGE                                  = 4;
    // Messages for handling error conditions.
    private static final int MSG_DISCONNECT_TIMEOUT                              = 6;

    // messages to obex handler
    public static final int MESSAGE_OBEX_CONNECT                                 = 101;
    public static final int MESSAGE_OBEX_DISCONNECT                              = 102;
    public static final int MESSAGE_OBEX_THUMBNAIL_FETCH                         = 103;
    public static final int MESSAGE_OBEX_IMAGE_FETCH                             = 104;
    // messages from obex handler
    public static final int MESSAGE_OBEX_CONNECTED                               = 105;
    public static final int MESSAGE_OBEX_DISCONNECTED                            = 106;
    public static final int MESSAGE_OBEX_THUMBNAIL_FETCHED                       = 107;
    public static final int MESSAGE_OBEX_IMAGE_FETCHED                           = 108;

    public static final String COVER_ART_HANDLE                                  = "CA_HANDLE";
    public static final String COVER_ART_IMAGE_LOCATION                          = "IMG_LOC";
    static final int DISCONNECT_TIMEOUT                                          = 3000;

    public static  String mSupportedCoverArtMimetype;
    protected static  String mImageType;
    public static  int mSupportedCovertArtHeight;
    public static  int mSupportedCoverArtWidth;
    public static  long mSupportedCoverArtMaxSize;
    private AvrcpControllerStateMachine mAvrcpCtrlStateMachine;
    private Disconnected mDisconnected;
    private Connecting mConnecting;
    private Connected mConnected;
    private Disconnecting mDisconnecting;
    private HandlerThread mHandlerThread = null;
    private AvrcpControllerBipObexHandler mAvrcpCtrlBipObexHandler = null;
    private final Context mContext;
    /* device associated with BIP client */
    private BluetoothDevice mDevice;

    protected static final String EXTRA_METADATA_IS_INVALID_HANDLE = "is_invalid_handle";

    private AvrcpControllerBipStateMachine(AvrcpControllerStateMachine mStateMachine,
                                           Handler handler, Context context) {
        super("AvrcpControllerBipStateMachine", handler);
        mContext = context;
        mAvrcpCtrlStateMachine = mStateMachine;
        mDisconnected = new Disconnected();
        mConnecting =  new Connecting();
        mConnected = new Connected();
        mDisconnecting = new Disconnecting();

        addState(mDisconnected);
        addState(mConnected);
        addState(mConnecting);
        addState(mDisconnecting);
        setInitialState(mDisconnected);

        mImageType = SystemProperties.get("persist.vendor.service.bt.avrcpct.imgtype");
        if (TextUtils.isEmpty(mImageType)) {
            mImageType = mContext.getResources().getString
                (R.string.avrcp_cover_art_default_image_type);
        }
        mSupportedCoverArtMimetype = SystemProperties.
                get("persist.vendor.service.bt.avrcpct.imgencode");
        if (TextUtils.isEmpty(mSupportedCoverArtMimetype)) {
            mSupportedCoverArtMimetype = mContext.getResources().getString(
                R.string.avrcp_cover_art_default_mimetype);
        }
        mSupportedCovertArtHeight = SystemProperties.
                getInt("persist.vendor.service.bt.avrcpct.imgheight",
                mContext.getResources().getInteger(R.integer.avrcp_cover_art_default_height));
        mSupportedCoverArtWidth = SystemProperties.
                getInt("persist.vendor.service.bt.avrcpct.imgwidth",
                mContext.getResources().getInteger(R.integer.avrcp_cover_art_default_width));

        mSupportedCoverArtMaxSize = SystemProperties.
                getInt("persist.vendor.service.bt.avrcpct.imgsize",
                mContext.getResources().getInteger(R.integer.avrcp_cover_art_default_maxsize));
        if (DBG) {
            Log.d(TAG, " Type :" + mImageType + ", Mime :" + mSupportedCoverArtMimetype + " Height:"
                + mSupportedCovertArtHeight + ", width :" + mSupportedCoverArtWidth + ", Max size:"
                    + mSupportedCoverArtMaxSize);
        }
        mHandlerThread = new HandlerThread("AvrcpControllerBipObexHandler",
                    Process.THREAD_PRIORITY_DISPLAY);
        mHandlerThread.start();
        mAvrcpCtrlBipObexHandler =
               new AvrcpControllerBipObexHandler(mHandlerThread.getLooper(), getHandler());
    }

    static AvrcpControllerBipStateMachine make(AvrcpControllerStateMachine mStateMachine,
                                            Handler handler, Context context) {
        Log.d("AvrcpControllerBipStateMachine", "make");
        AvrcpControllerBipStateMachine avrcpBipSm =
                new AvrcpControllerBipStateMachine(mStateMachine, handler, context);
        avrcpBipSm.start();
        return avrcpBipSm;
    }

    public void doQuit() {
        if (mAvrcpCtrlBipObexHandler != null) {
            mAvrcpCtrlBipObexHandler.removeCallbacksAndMessages(null);
            mAvrcpCtrlBipObexHandler.cleanup();
            mAvrcpCtrlBipObexHandler = null;
        }
        if (mHandlerThread != null) {
            mHandlerThread.quitSafely();
        }
        quitNow();
    }

    public int getBipState() {
        if(getCurrentState() == mDisconnected)
            return BluetoothProfile.STATE_DISCONNECTED;
        if (getCurrentState() == mConnecting)
            return BluetoothProfile.STATE_CONNECTING;
        if (getCurrentState() == mConnecting)
            return BluetoothProfile.STATE_CONNECTING;
        if (getCurrentState() == mConnecting)
            return BluetoothProfile.STATE_CONNECTING;
        return BluetoothProfile.STATE_DISCONNECTED;
    }

    public static String dumpMessageString(int message) {
        String str = "UNKNOWN";
        switch (message) {
            case MESSAGE_CONNECT_BIP:
                str = "IN_AVRCP_CONNECT_BIP";
                break;
            case MESSAGE_DISCONNECT_BIP:
                str = "IN_AVRCP_DISCONNECT_BIP";
                break;
            case MESSAGE_FETCH_THUMBNAIL:
                str = "IN_AVRCP_FETCH_THUMBNAIL";
                break;
            case MESSAGE_FETCH_IMAGE:
                str = "IN_AVRCP_FETCH_IMAGE";
                break;
            case MSG_DISCONNECT_TIMEOUT:
                str = "MSG_DISCONNECT_TIMEOUT";
                break;
            case MESSAGE_OBEX_CONNECT:
                str = "REQ_OBEX_CONNECT";
                break;
            case MESSAGE_OBEX_DISCONNECT:
                str = "REQ_OBEX_DISCONNECT";
                break;
            case MESSAGE_OBEX_THUMBNAIL_FETCH:
                str = "REQ_OBEX_THUMBNAIL_FETCH";
                break;
            case MESSAGE_OBEX_IMAGE_FETCH:
                str = "REQ_OBEX_IAMGE_FETCH";
                break;
            case MESSAGE_OBEX_CONNECTED:
                str = "CB_OBEX_CONNECTED";
                break;
            case MESSAGE_OBEX_DISCONNECTED:
                str = "CB_OBEX_DISCONNECTED";
                break;
            case MESSAGE_OBEX_THUMBNAIL_FETCHED:
                str = "CB_OBEX_THUMBNAIL_FETCHED";
                break;
            case MESSAGE_OBEX_IMAGE_FETCHED:
                str = "CB_OBEX_IMAGE_FETCHED";
                break;
            default:
                str = Integer.toString(message);
                break;
        }
        return str;
    }

    private class Disconnected extends State {
        private static final String STATE_TAG = AvrcpControllerBipStateMachine.TAG +
                ".Disconnected";
        @Override
        public void enter() {
            if (DBG) {
                Log.d(STATE_TAG, "Enter: " + dumpMessageString(getCurrentMessage().what));
            }
        }
        @Override
        public boolean processMessage(Message message) {
            if (DBG) {
                Log.d(STATE_TAG, "Recvd: " + dumpMessageString(getCurrentMessage().what));
            }
            switch (message.what) {
                case MESSAGE_CONNECT_BIP:
                    // arg1 is l2capPsm, obj is device
                    if (mAvrcpCtrlBipObexHandler == null) {
                        //Should not happen
                        Log.w(STATE_TAG,"ObexHandler not available: CONNECT cannot be supported");
                        break;
                    }
                    if (message.obj instanceof BluetoothDevice) {
                        mDevice = (BluetoothDevice)message.obj;
                    } else {
                        Log.w(STATE_TAG,"Invalid  arg list: break");
                        break;
                    }
                    //Handle last user request. Ignore previous ones that are still pending.
                    mAvrcpCtrlBipObexHandler.removeMessages(MESSAGE_OBEX_CONNECT);
                    if(DBG) Log.v(STATE_TAG,"MESSAGE_OBEX_CONNECT: from device: " + mDevice);
                    mAvrcpCtrlBipObexHandler
                        .obtainMessage(MESSAGE_OBEX_CONNECT, message.arg1, 0, mDevice)
                        .sendToTarget();
                    transitionTo(mConnecting);
                    break;

                default:
                    return NOT_HANDLED;
            }
            return HANDLED;
        }
    }

    private class Connecting extends State {
        private static final String STATE_TAG = AvrcpControllerBipStateMachine.TAG + ".Connecting";
        @Override
        public void enter() {
            if (DBG) {
                Log.d(STATE_TAG, "Enter: " + dumpMessageString(getCurrentMessage().what));
            }
        }
        @Override
        public boolean processMessage(Message message) {
            if (DBG) {
                Log.d(STATE_TAG, "Recvd: " +dumpMessageString(getCurrentMessage().what));
            }
            switch (message.what) {
                case MESSAGE_OBEX_CONNECTED:
                    mAvrcpCtrlStateMachine.sendMessage(CoverArtUtils.MESSAGE_BIP_CONNECTED);
                    transitionTo(mConnected);
                    break;

                case MESSAGE_OBEX_DISCONNECTED:
                    mAvrcpCtrlStateMachine.sendMessage(CoverArtUtils.MESSAGE_BIP_DISCONNECTED);
                    transitionTo(mDisconnected);
                    break;

                case MESSAGE_FETCH_IMAGE:
                case MESSAGE_FETCH_THUMBNAIL:
                    Log.w(STATE_TAG," MESSAGE_FETCH_IMAGE");
                    removeDeferredMessages(message.what);
                    deferMessage(message);
                    break;

                case MESSAGE_DISCONNECT_BIP:
                    if (mAvrcpCtrlBipObexHandler == null) {
                        //Should not happen
                        Log.w(STATE_TAG,"ObexHandler not available: DISC cannot be supported");
                        break;
                    }
                    if (message.obj instanceof BluetoothDevice ) {
                        if (!((BluetoothDevice) message.obj).equals(mDevice)) {
                            Log.w(STATE_TAG,"DISC requested for different device: Not supported");
                            break;
                        }
                    } else {
                        Log.w(STATE_TAG,"Invalid  arg list: break");
                        break;
                    }
                    if (DBG) Log.v(STATE_TAG,"MESSAGE_OBEX_DISCONNECT: device: " + mDevice);
                    mAvrcpCtrlBipObexHandler.removeCallbacksAndMessages(null);
                    Message msg = mAvrcpCtrlBipObexHandler
                            .obtainMessage(MESSAGE_OBEX_DISCONNECT, mDevice);
                    msg.sendToTarget();
                    sendMessageDelayed(MSG_DISCONNECT_TIMEOUT, DISCONNECT_TIMEOUT);
                    transitionTo(mDisconnecting);
                    break;

                default:
                    if (DBG) Log.w(STATE_TAG," Unhandled Message ");
                    return NOT_HANDLED;
            }
            return HANDLED;
        }
    }

    private class Connected extends State {
        private static final String STATE_TAG = AvrcpControllerBipStateMachine.TAG + ".Connected";
        @Override
        public void enter() {
            if (DBG) {
                Log.d(STATE_TAG, "Enter: " + dumpMessageString(getCurrentMessage().what));
            }
        }
        @Override
        public boolean processMessage(Message message) {
            if (DBG) {
                Log.d(STATE_TAG, "Recvd: " + dumpMessageString(getCurrentMessage().what));
            }
            Bundle imageData;
            Message msg = null;
            switch (message.what) {
                case MESSAGE_OBEX_IMAGE_FETCHED:
                    imageData = new Bundle();
                    imageData.putString(COVER_ART_HANDLE,
                            message.getData().getString(COVER_ART_HANDLE));
                    imageData.putString(COVER_ART_IMAGE_LOCATION,
                            message.getData().getString(COVER_ART_IMAGE_LOCATION));
                    msg = mAvrcpCtrlStateMachine.obtainMessage(
                            CoverArtUtils.MESSAGE_BIP_IMAGE_FETCHED);
                    msg.setData(imageData);
                    msg.sendToTarget();
                    break;

                case MESSAGE_OBEX_THUMBNAIL_FETCHED:
                    imageData = new Bundle();
                    imageData.putString(COVER_ART_HANDLE,
                            message.getData().getString(COVER_ART_HANDLE));
                    imageData.putString(COVER_ART_IMAGE_LOCATION,
                            message.getData().getString(COVER_ART_IMAGE_LOCATION));
                    msg = mAvrcpCtrlStateMachine.obtainMessage(
                            CoverArtUtils.MESSAGE_BIP_THUMB_NAIL_FETCHED);
                    msg.setData(imageData);
                    msg.sendToTarget();
                    break;

                case MESSAGE_FETCH_IMAGE:
                    if(mAvrcpCtrlBipObexHandler == null) {
                        //Should not happen
                        Log.w(STATE_TAG,"ObexHandler is null: IMAGE FETCH cannot be supported");
                        throw new IllegalStateException("ObexHandler died");
                    }
                    // Honor most recent request as other request are obsolete.
                    mAvrcpCtrlBipObexHandler.removeMessages(MESSAGE_FETCH_IMAGE);
                    imageData = new Bundle();
                    imageData.putString(COVER_ART_HANDLE, (String)message.obj);
                    msg = mAvrcpCtrlBipObexHandler.obtainMessage(MESSAGE_OBEX_IMAGE_FETCH);
                    msg.setData(imageData);
                    msg.sendToTarget();
                    break;

                case MESSAGE_FETCH_THUMBNAIL:
                    if (mAvrcpCtrlBipObexHandler == null) {
                        //Should not happen
                        Log.w(STATE_TAG,"ObexHandler not available");
                        break;
                    }
                    // Honor most recent request as other request are obsolete.
                    mAvrcpCtrlBipObexHandler.removeMessages(MESSAGE_OBEX_THUMBNAIL_FETCH);
                    msg = mAvrcpCtrlBipObexHandler.obtainMessage(MESSAGE_OBEX_THUMBNAIL_FETCH,
                            message.obj);
                    msg.sendToTarget();
                    break;

                case MESSAGE_DISCONNECT_BIP:
                    if (mAvrcpCtrlBipObexHandler == null) {
                        //Should not happen
                        Log.w(STATE_TAG,"ObexHandler not available: CONNECT cannot be supported");
                        break;
                    }
                    if (message.obj instanceof BluetoothDevice ) {
                        if(!((BluetoothDevice) message.obj).equals(mDevice)) {
                            Log.w(STATE_TAG,"DISC requested for different device: Not supported");
                            break;
                        }
                    } else {
                        Log.w(STATE_TAG,"Invalid  arg list: break");
                        break;
                    }
                    if(DBG) Log.v(STATE_TAG,"MESSAGE_OBEX_DISCONNECT: device: " + mDevice);
                    mAvrcpCtrlBipObexHandler.removeCallbacksAndMessages(null);
                    msg = mAvrcpCtrlBipObexHandler.obtainMessage(MESSAGE_OBEX_DISCONNECT, mDevice);
                    msg.sendToTarget();
                    sendMessageDelayed(MSG_DISCONNECT_TIMEOUT, DISCONNECT_TIMEOUT);
                    transitionTo(mDisconnecting);
                    break;

                case MESSAGE_OBEX_DISCONNECTED:
                    mAvrcpCtrlStateMachine.sendMessage(CoverArtUtils.MESSAGE_BIP_DISCONNECTED);
                    transitionTo(mDisconnected);
                    break;

                default:
                    return NOT_HANDLED;
            }
            return HANDLED;
        }
    }

    private class Disconnecting extends State {
        private static final String STATE_TAG = AvrcpControllerBipStateMachine.TAG +
                ".Disconnecting";
        @Override
        public void enter() {
            if (DBG) {
                Log.d(STATE_TAG, "Enter: " + dumpMessageString(getCurrentMessage().what));
            }
        }
        @Override
        public boolean processMessage(Message message) {
            if (DBG) {
                Log.d(STATE_TAG, "Recvd: " + dumpMessageString(getCurrentMessage().what));
            }
            switch (message.what) {
                case MESSAGE_OBEX_DISCONNECTED:
                    removeMessages(MSG_DISCONNECT_TIMEOUT);
                    mAvrcpCtrlStateMachine.sendMessage(
                            CoverArtUtils.MESSAGE_BIP_DISCONNECTED);
                    transitionTo(mDisconnected);
                    break;

                case MSG_DISCONNECT_TIMEOUT:
                    Log.w(STATE_TAG,"Disconnect Timeout, Forcing");
                    mAvrcpCtrlBipObexHandler.abort();
                    mAvrcpCtrlStateMachine.sendMessage(
                            CoverArtUtils.MESSAGE_BIP_DISCONNECTED);
                    transitionTo(mDisconnected);
                    break;

                case MESSAGE_CONNECT_BIP:
                    removeDeferredMessages(message.what);
                    deferMessage(message);
                    break;

                default:
                    return NOT_HANDLED;
            }
            return HANDLED;
        }
    }

}
