/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.net.connextension;

import android.content.ComponentName;
import android.util.Log;
import android.app.ActivityThread;
import android.content.Context;
import android.os.Binder;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.SystemClock;
import android.os.Message;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.telephony.SubscriptionManager;
import android.os.Handler;
import android.content.ServiceConnection;
import android.os.RemoteException;
import android.net.NetworkSpecifier;
import android.net.StringNetworkSpecifier;
import android.provider.Settings;
import android.util.Log;
import com.android.internal.util.WakeupMessage;
import java.util.ArrayList;
import java.util.Map;
import java.util.HashMap;
import java.util.Set;
import java.util.HashSet;
import org.codeaurora.internal.INetworkCallback;
import org.codeaurora.internal.NetworkCallbackBase;
import org.codeaurora.internal.IExtTelephony;
import org.codeaurora.internal.ServiceUtil;
import org.codeaurora.internal.SignalStrength;
import org.codeaurora.internal.Client;
import org.codeaurora.internal.Status;
import org.codeaurora.internal.Token;
import org.codeaurora.internal.DcParam;
import org.codeaurora.internal.BearerAllocationStatus;
import org.codeaurora.internal.UpperLayerIndInfo;
import org.codeaurora.internal.NrConfigType;

public class TCPBufferManager implements Handler.Callback {
    private static boolean DBG = true;
    private static boolean VDBG = DBG && false;
    private static final String LOGTAG = "TCPBufferManager";
    private Context mCtx;
    private HandlerThread mInternalThread;
    private Handler mInternalHandler;
    private Handler mConnHandler;
    private boolean mIsServiceBound;
    private IExtTelephony mExtTelephonyService;
    private String mPackageName;
    private Client mClient;
    private SubscriptionManager mSubscriptionManager;

    private static final String RAT_5G = "5gnr";
    private static final int INVALID_SUBSCRIPTION_ID = -1;
    //Try at most 8 times for connecting to ExtTelephonyService
    //With exponential backoff in seconds: 2,4,8,16,32,64,128,256
    private static int connectionRetry = 8;
    private static int backOffTimeMilliSeconds = 2000;
    private boolean is5GConnected = false;
    private int subscriptionIdFor5G = -1;
    private ConnectivityManager mConnectivityManager;

    /*
     * Events for internal handler
     * EVENT_CMD_CONNECT_EXT_TELEPHONY_SERV event to connect to ExtTelephonyService
     * EVENT_ON_5G_ENABLE_STATUS            event to handle 5G bearer allocation indication
     */
    private static final int EVENT_CMD_CONNECT_EXT_TELEPHONY_SERV = 1;
    private static final int EVENT_ON_5G_ENABLE_STATUS = 0;

    /*
     * Event for Connectivity Service
     * EVENT_UPDATE_TCP_BUFFER_FOR_5G   event to update the tcp buffer for 5G.
     *                                  Value should match ConnectivityService
     *                                  defined constant
     */
    private static final int EVENT_UPDATE_TCP_BUFFER_FOR_5G = 160;

    /*
     *  Test Code
     */
    private WakeupMessage mKickAlarm;
    private WakeupMessage mRetryAlarm;
    private static final int CMD_KICK = 2;

    /*
     * Schedules a retry trigger to connect to ExtTelephonyService service
     */
    private void scheduleRetry() {
        long now = SystemClock.elapsedRealtime();
        long alarmTime = now + backOffTimeMilliSeconds;
        mRetryAlarm.schedule(alarmTime);
        if (VDBG)
            Log.d(LOGTAG, "Retry scheduled in " + backOffTimeMilliSeconds
                  + "ms");
        backOffTimeMilliSeconds = 2 * backOffTimeMilliSeconds;
    }

    @Override
    public boolean handleMessage(Message msg) {
        boolean ret_val = true;
        switch (msg.what) {
            case EVENT_ON_5G_ENABLE_STATUS : {
                int slotId = msg.arg1;
                boolean enabled = msg.arg2 == 1;
                if (VDBG)
                    Log.d(LOGTAG, "EVENT_SET_5G_ENABLE_STATUS: slotId = " + slotId + " status "
                            + enabled);
                handle5GStatus(slotId, enabled);
                break;
            }
            case EVENT_CMD_CONNECT_EXT_TELEPHONY_SERV : {
                if (VDBG)
                    Log.d(LOGTAG, "Event received EVENT_CMD_CONNECT_EXT_TELEPHONY_SERV");
                connecteToExtTelephonyService();
                break;
            }
            default :
                Log.w(LOGTAG, "Unhandled Message: " + msg);
                ret_val = false;
                break;
        }
        return ret_val;
    } /* handleMessage */

    /*
     * Wrapper provided to invoke the bounded ExtTelephonyService Service and connect with it.
     * Provides various methods to track the status of the service
     */
    private ServiceConnection mMainServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            if (DBG)
                Log.d(LOGTAG, "onServiceConnected: " + name);

            try {
                mExtTelephonyService = IExtTelephony.Stub.asInterface(service);
                mClient = mExtTelephonyService.registerCallback(mPackageName, mCallback);
                if (VDBG)
                    Log.d(LOGTAG, "Client = " + mClient);
                mIsServiceBound = true;

            } catch (RemoteException e) {
                Log.e(LOGTAG, "onServiceConnected: Exception = " + e);
            }
        }
        @Override
        public void onServiceDisconnected(ComponentName name) {
            if (DBG)
                Log.d(LOGTAG, "onServiceDisconnected:");
            cleanup();
        }
        @Override
        public void onBindingDied(ComponentName name) {
            if (DBG)
                Log.d(LOGTAG, "onBindingDied:");
            cleanup();
        }
        private void cleanup() {
            if (VDBG)
                Log.d(LOGTAG, "cleanup");
            mIsServiceBound = false;
            mExtTelephonyService = null;
            mClient = null;
        }
    };

    /*
     * Callback to listen to various 5G indications.
     *
     */
    private INetworkCallback mCallback = new NetworkCallbackBase() {
        @Override
        public void on5gStatus(int slotId, Token token, Status status, boolean enableStatus)
                throws RemoteException {
            Log.e(LOGTAG, "on5gStatus not supported");
        }
        @Override
        public void onNrDcParam(int slotId, Token token, Status status, DcParam dcParam)
                throws RemoteException {
            Log.e(LOGTAG, "onNrDcParam not supported");
        }

        /*
         * These API is invoked to indicate the current status of 5G RAT. Callback
         * is invoked either as api response or to indicate unsol change(token = -1).
         */
        @Override
        public void onAnyNrBearerAllocation(int slotId, Token token, Status status,
                                            BearerAllocationStatus bearerStatus)
                throws RemoteException {
            if (VDBG)
                Log.d(LOGTAG, "onAnyNrBearerAllocation: slotId = " + slotId + " token = " + token
                        + " status" + status + " bearerStatus = " + bearerStatus.get());
            if (status.get() == Status.SUCCESS) {
                boolean allocated = false;
                if(bearerStatus.get() == BearerAllocationStatus.ALLOCATED ||
                   bearerStatus.get() == BearerAllocationStatus.MMW_ALLOCATED) {
                    allocated = true;
                } else {
                    allocated = false;
                }
                mInternalHandler.sendMessage(mInternalHandler
                        .obtainMessage(EVENT_ON_5G_ENABLE_STATUS, slotId, (allocated) ? 1 : 0));
            }
        }

        @Override
        public void onUpperLayerIndInfo(int slotId, Token token, Status status,
                                        UpperLayerIndInfo uilInfo) throws RemoteException {
            Log.i(LOGTAG, "onUpperLayerIndInfo not supported");
        }

        @Override
        public void onSignalStrength(int slotId, Token token, Status status,
                SignalStrength signalStrength) throws RemoteException {
            Log.e(LOGTAG, "onSignalStrength not supported");
        }

        @Override
        public void on5gConfigInfo(int slotId, Token token, Status status,
                NrConfigType nrConfigType) throws RemoteException {
            Log.e(LOGTAG, "on5gConfigInfo not supported");
        }
    };

    /*
     * Returns the TCP buffer size, if slotId of the default network is equal to the Rat on which
     * 5G gets enabled
     */
    public String get5GTcpBuffers(String currentBuffers, NetworkSpecifier specifier) {
        if (is5GConnected && getSubIdFromNetworkSpecifier(specifier) == subscriptionIdFor5G) {
            return getBuffersFromConfig(RAT_5G);
        } else {
            if (DBG)
                Log.d(LOGTAG, "returning current buffer " + currentBuffers);
            return currentBuffers;
        }

    }

    /*
     * Allows a handler to be registered with buffer manager.
     * initializes the internal handler and connects to the ExtTelephonyService Service.
     */
    public void registerHandler(Handler handler) {
        mConnHandler = handler;
        mCtx = ActivityThread.currentApplication().getApplicationContext();
        mConnectivityManager = (ConnectivityManager) mCtx
                .getSystemService(Context.CONNECTIVITY_SERVICE);
        mInternalThread = new HandlerThread("EventHandler");
        mInternalThread.start();
        mInternalHandler = new Handler(mInternalThread.getLooper(), this);
        mPackageName = mCtx.getPackageName();
        if (VDBG)
            Log.d(LOGTAG, "getting application context packagename " + mPackageName + " config "
                    + getBuffersFromConfig(RAT_5G));

        mKickAlarm = new WakeupMessage(mCtx, mInternalHandler, "KICK", CMD_KICK);
        mRetryAlarm = new WakeupMessage(mCtx, mInternalHandler, "RETRY", EVENT_CMD_CONNECT_EXT_TELEPHONY_SERV);
        mInternalHandler.sendMessage(mInternalHandler.obtainMessage(EVENT_CMD_CONNECT_EXT_TELEPHONY_SERV));
    }
    /*
     * Use the ServiceUtil.bindService to connect to correct service.
     */
    private void connecteToExtTelephonyService() {
        Log.d(LOGTAG, "Trying to connect to service");
        boolean result = ServiceUtil.bindService(mCtx, mMainServiceConnection);
        if (!result) {
            Log.d(LOGTAG, "Not connected to service " + result + ". retry = " + connectionRetry);
            if(connectionRetry > 0) {
                scheduleRetry();
                connectionRetry--;
            } else {
                Log.e(LOGTAG, "Cannot connect to ExtTelephonyService");
            }
        } else {
            Log.d(LOGTAG, "Service connection returned " + result);
        }
    }

    /*
     * Returns subscription id from the slotid.
     */
    private int getSubscriptionIdFromSlotId(int slotId) {

        int sub[] = SubscriptionManager.getSubId(slotId);
        if (sub != null && sub.length > 0) {
            if (DBG)
                Log.d(LOGTAG, "getSubscriptionIdFromSlotId subid " + sub[0]);
            return sub[0];
        }
        return INVALID_SUBSCRIPTION_ID;
    }

    /*
     * handles 5G allocation indication. Sends a event to ConnectivityService to
     * update the TCP buffers
     */
    private void handle5GStatus(int slotId, boolean status) {
        is5GConnected = status;
        subscriptionIdFor5G = getSubscriptionIdFromSlotId(slotId);
        if (DBG) {
            Log.d(LOGTAG, "handle5GStatus subid " + subscriptionIdFor5G + " is5GConnected "
                    + is5GConnected);
        }
        mConnHandler.sendMessage(mConnHandler.obtainMessage(EVENT_UPDATE_TCP_BUFFER_FOR_5G));
    }

    /*
     * Returns slotId from NetworkSpecifier
     */
    private int getSubIdFromNetworkSpecifier(NetworkSpecifier specifier) {

        int subId = INVALID_SUBSCRIPTION_ID;
        if (specifier != null) {
            if (specifier instanceof StringNetworkSpecifier) {
                try {
                    subId = Integer.parseInt(((StringNetworkSpecifier) specifier).specifier);
                } catch (NumberFormatException e) {
                    Log.e(LOGTAG, "NumberFormatException on "
                            + ((StringNetworkSpecifier) specifier).specifier);
                    subId = INVALID_SUBSCRIPTION_ID;
                }
            }
        }
        return subId;
    }

    /*
     * Parses config and returns buffer size for a particular
     * rat type
     */
    private String getBuffersFromConfig(String ratType) {
        // in the form:
        // "ratname:rmem_min,rmem_def,rmem_max,wmem_min,wmem_def,wmem_max"
        String sizes = null;
        String[] configOverride = mCtx.getResources()
                .getStringArray(com.android.internal.R.array.config_mobile_tcp_buffers);
        for (int i = 0; i < configOverride.length; i++) {
            String[] split = configOverride[i].split(":");
            if (VDBG)
                Log.d(LOGTAG, "configOverride " + split[0] + "  " + split[1]);
            if (ratType.equals(split[0]) && split.length == 2) {
                sizes = split[1];
                break;
            }
        }
        if (DBG)
            Log.d(LOGTAG, "get buffer size = " + sizes + " for ratType " + ratType);
        return sizes;
    }
}
