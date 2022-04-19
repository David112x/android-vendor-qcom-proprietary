/*
 * Copyright (c) 2018, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

package com.qualcomm.qti.internal.nrNetworkService;

import android.app.AppOpsManager;
import android.content.Context;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import android.os.Binder;
import android.util.Log;

import com.android.internal.util.IndentingPrintWriter;
import com.android.internal.telephony.CommandsInterface;

import com.qualcomm.qti.internal.nrNetworkService.hidl.QtiRadioHidlClient;
import com.qualcomm.qti.internal.nrNetworkService.hidl.FakeHidlConnectionInterfaceImpl;
import com.qualcomm.qti.internal.nrNetworkService.hidl.IHidlConnectionCallback;
import com.qualcomm.qti.internal.nrNetworkService.hidl.IHidlConnectionInterface;
import org.codeaurora.internal.BearerAllocationStatus;
import org.codeaurora.internal.Client;
import org.codeaurora.internal.DcParam;
import org.codeaurora.internal.INetworkCallback;
import org.codeaurora.internal.NrConfigType;
import org.codeaurora.internal.SignalStrength;
import org.codeaurora.internal.Status;
import org.codeaurora.internal.Token;
import org.codeaurora.internal.UpperLayerIndInfo;
import org.codeaurora.internal.NrIconType;

import java.util.ArrayList;
import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.util.concurrent.ConcurrentHashMap;
import com.android.internal.annotations.VisibleForTesting;

public class MainServiceImpl {
    private static final String TAG = "MainServiceImpl";

    private static final int EVENT_ON_BEARER_ALLOCATION_CHANGE_IND = 0;
    private static final int EVENT_ON_5G_ENABLE_STATUS_CHANGE_IND = 1;
    private static final int EVENT_ON_5G_SIGNAL_STRENGTH_CHANGE_IND = 2;
    private static final int EVENT_ON_NR_DUAL_CONNECTIVITY_CHANGE_IND = 3;
    private static final int EVENT_ON_RESTRICT_DCNR_CHANGE = 4;
    private static final int EVENT_ON_UPPER_LAYER_INDICATION_INFO = 5;
    private static final int EVENT_ON_5G_CONFIG_INFO = 6;
    private static final int EVENT_ON_NR_ICON_TYPE = 7;
    private static final int EVENT_ON_ENABLE_ENDC = 8;
    private static final int EVENT_ON_ENDC_STATUS = 9;

    private boolean SUCCESS = true;
    private boolean FAILED = false;

    private static Context mContext;
    private AppOpsManager mAppOpsManager;
    private int mClientIndex = -1;
    private HandlerThread mWorkerThread = new HandlerThread(TAG + "BgThread");
    private Handler mWorkerThreadHandler;
    private IHidlConnectionInterface mHidlConnectionInterface;
    private final ArrayList<INetworkCallback> mCallbackList = new ArrayList<INetworkCallback>();
    private ConcurrentHashMap<Integer, Transaction> mInflightRequests = new
            ConcurrentHashMap<Integer, Transaction>();
    private static MainServiceImpl sInstance = null;

    private class WorkerHandler extends Handler {
        private static final String TAG = MainServiceImpl.TAG + "Handler: ";

        public WorkerHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG, "handleMessage msg.what = " + msg.what);
            switch (msg.what) {
                case EVENT_ON_BEARER_ALLOCATION_CHANGE_IND: {
                    Log.d(TAG, "EVENT_ON_BEARER_ALLOCATION_CHANGE_IND");
                    int slotId = msg.arg1;
                    Result result = (Result) msg.obj;
                    onAnyNrBearerAllocation(slotId, result.mToken, result.mStatus,
                            (BearerAllocationStatus) result.mData);
                    break;
                }

                case EVENT_ON_5G_ENABLE_STATUS_CHANGE_IND: {
                    Log.d(TAG, "EVENT_ON_5G_ENABLE_STATUS");
                    int slotId = msg.arg1;
                    Result result = (Result) msg.obj;
                    on5gStatus(slotId, result.mToken, result.mStatus, (boolean) result.mData);
                    break;
                }

                case EVENT_ON_5G_SIGNAL_STRENGTH_CHANGE_IND: {
                    Log.d(TAG, "EVENT_ON_5G_SIGNAL_STRENGTH_CHANGE_IND");
                    int slotId = msg.arg1;
                    Result result = (Result) msg.obj;
                    onSignalStrength(slotId, result.mToken, result.mStatus,
                            (SignalStrength) result.mData);
                    break;
                }

                case EVENT_ON_NR_DUAL_CONNECTIVITY_CHANGE_IND: {
                    Log.d(TAG, "EVENT_ON_NR_DUAL_CONNECTIVITY_CHANGE_IND");
                    int slotId = msg.arg1;
                    Result result = (Result) msg.obj;
                    onNrDcParam(slotId, result.mToken, result.mStatus, (DcParam) result.mData);
                    break;
                }
                case EVENT_ON_UPPER_LAYER_INDICATION_INFO: {
                    Log.d(TAG, "EVENT_ON_UPPER_LAYER_INDICATION_INFO");
                    int slotId = msg.arg1;
                    Result result = (Result) msg.obj;
                    onUpperLayerIndInfo(slotId, result.mToken, result.mStatus,
                            (UpperLayerIndInfo) result.mData);
                    break;
                }

                case EVENT_ON_5G_CONFIG_INFO: {
                    Log.d(TAG, "EVENT_ON_5G_CONFIG_INFO");
                    int slotId = msg.arg1;
                    Result result = (Result) msg.obj;
                    on5gConfigInfo(slotId, result.mToken, result.mStatus,
                            (NrConfigType) result.mData);
                    break;
                }

                case EVENT_ON_NR_ICON_TYPE: {
                    Log.d(TAG, "EVENT_ON_NR_ICON_TYPE");
                    int slotId = msg.arg1;
                    Result result = (Result) msg.obj;
                    onNrIconType(slotId, result.mToken, result.mStatus,
                            (NrIconType) result.mData);
                    break;
                }

                case EVENT_ON_ENABLE_ENDC: {
                    Log.d(TAG, "EVENT_ON_ENABLE_ENDC");
                    int slotId = msg.arg1;
                    Result result = (Result) msg.obj;
                    onEnableEndc(slotId, result.mToken, result.mStatus);
                    break;
                }

                case EVENT_ON_ENDC_STATUS: {
                    Log.d(TAG, "EVENT_ON_ENDC_STATUS");
                    int slotId = msg.arg1;
                    Result result = (Result) msg.obj;
                    onEndcStatus(slotId, result.mToken, result.mStatus, (boolean) result.mData);
                    break;
                }
            }
        }
    }

    class Result {
        Token mToken;
        Status mStatus;
        Object mData;

        public Result(Token mToken, Status mStatus, Object mData) {
            this.mToken = mToken;
            this.mStatus = mStatus;
            this.mData = mData;
        }

        @Override
        public String toString() {
            return "Result{" + "mToken=" + mToken + ", mStatus=" + mStatus + ", mData=" + mData +
                    '}';
        }
    }

    class Transaction {
        Token mToken;
        String mName;
        Client mClient;

        public Transaction(Token token, String name, Client client) {
            mToken = token;
            mName = name;
            mClient = client;
        }

        @Override
        public String toString() {
            return "Transaction{" + "mToken=" + mToken + ", mName='" + mName + '\'' + ", mClient="
                    + mClient + '}';
        }
    }

    IHidlConnectionCallback mHidlConnectionCallback = new IHidlConnectionCallback() {
        @Override
        public void on5gStatus(int slotId, Token token, Status status, boolean enableStatus) {
            Log.d(TAG, "on5gStatus slotId = " + slotId + " token = " + token + " status = " +
                    status + " enableStatus = " + enableStatus);

            mWorkerThreadHandler.sendMessage(mWorkerThreadHandler.obtainMessage
                    (EVENT_ON_5G_ENABLE_STATUS_CHANGE_IND, slotId, -1, new Result(token, status,
                            enableStatus)));
        }

        @Override
        public void onNrDcParam(int slotId, Token token, Status status, DcParam dcParam) {
            Log.d(TAG, "onNrDcParam slotId = " + slotId + " token = " + token + " status = " +
                    "" + status + " DcParam = " + dcParam);
            mWorkerThreadHandler.sendMessage(mWorkerThreadHandler.obtainMessage
                    (EVENT_ON_NR_DUAL_CONNECTIVITY_CHANGE_IND, slotId, -1, new Result(token, status,
                            dcParam)));
        }

        @Override
        public void onAnyNrBearerAllocation(int slotId, Token token, Status status,
                BearerAllocationStatus bearerStatus) {
            Log.d(TAG, "onAnyNrBearerAllocation slotId = " + slotId +
                    " bearerStatus = " + bearerStatus);
            mWorkerThreadHandler.sendMessage(mWorkerThreadHandler.obtainMessage
                    (EVENT_ON_BEARER_ALLOCATION_CHANGE_IND, slotId, -1, new Result(token, status,
                            bearerStatus)));
        }

        @Override
        public void onUpperLayerIndInfo(int slotId, Token token, Status status,
                UpperLayerIndInfo ulInfo) {
            Log.d(TAG, "onUpperLayerIndInfo slotId = " + slotId + " UpperLayerIndInfo = " + ulInfo);
            mWorkerThreadHandler.sendMessage(mWorkerThreadHandler.obtainMessage
                    (EVENT_ON_UPPER_LAYER_INDICATION_INFO, slotId, -1, new Result(token, status,
                            ulInfo)));
        }

        @Override
        public void on5gConfigInfo(int slotId, Token token, Status status,
                                        NrConfigType nrConfigType) {
            Log.d(TAG, "on5gConfigInfo slotId = " + slotId + " NrConfigType = " + nrConfigType);
            mWorkerThreadHandler.sendMessage(mWorkerThreadHandler.obtainMessage
                    (EVENT_ON_5G_CONFIG_INFO, slotId, -1, new Result(token, status,
                            nrConfigType)));
        }

        @Override
        public void onSignalStrength(int slotId, Token token, Status status,
                SignalStrength signalStrength) {
            Log.d(TAG, "onSignalStrength slotId = " + slotId + " signalStrength = " +
                    signalStrength);
            mWorkerThreadHandler.sendMessage(mWorkerThreadHandler.obtainMessage
                    (EVENT_ON_5G_SIGNAL_STRENGTH_CHANGE_IND, slotId, -1, new Result(token, status,
                            signalStrength)));
        }

        @Override
        public void onNrIconType(int slotId, Token token, Status status,
                                   NrIconType nrIconType) {
            Log.d(TAG, "onNrIconType slotId = " + slotId + " NrIconType = " + nrIconType);
            mWorkerThreadHandler.sendMessage(mWorkerThreadHandler.obtainMessage
                    (EVENT_ON_NR_ICON_TYPE, slotId, -1, new Result(token, status,
                            nrIconType)));
        }

        @Override
        public void onEnableEndc(int slotId, Token token, Status status) {
            Log.d(TAG, "onEnableEndc slotId = " + slotId + " token = " + token + " status = " +
                    status);

            mWorkerThreadHandler.sendMessage(mWorkerThreadHandler.obtainMessage
                    (EVENT_ON_ENABLE_ENDC, slotId, -1, new Result(token, status, null)));
        }

        @Override
        public void onEndcStatus(int slotId, Token token, Status status, boolean enableStatus) {
            Log.d(TAG, "onEndcStatus slotId = " + slotId + " token = " + token + " status = " +
                    status + " enable = " + enableStatus);

            mWorkerThreadHandler.sendMessage(mWorkerThreadHandler.obtainMessage
                    (EVENT_ON_ENDC_STATUS, slotId, -1, new Result(token, status,
                            enableStatus)));
        }
    };

    public static MainServiceImpl init(Context context) {
        synchronized (MainServiceImpl.class) {
            mContext = context;
            if (sInstance == null) {
                sInstance = new MainServiceImpl(context);
            } else {
                Log.wtf(TAG, "init() called multiple times!  sInstance = " + sInstance);
            }
            return sInstance;
        }
    }

    public static MainServiceImpl getInstance() {
        if (sInstance == null) {
            Log.wtf(TAG, "getInstance null");
        }
        return sInstance;
    }

    public MainServiceImpl(Context context) {
        Log.d(TAG, "MainServiceImpl");
        mContext = context;
        mAppOpsManager = (AppOpsManager) mContext.getSystemService(Context.APP_OPS_SERVICE);
        setHidlClient(new QtiRadioHidlClient());
        mWorkerThread.start();
        setLooper(mWorkerThread.getLooper());
    }

    @VisibleForTesting 
    public MainServiceImpl(Context context, IHidlConnectionInterface hidlClient,
            Looper workerLooper){
        Log.d(TAG, "MainServiceImpl");
        mContext = context;
        mAppOpsManager = (AppOpsManager) mContext.getSystemService(Context.APP_OPS_SERVICE);
        setHidlClient(hidlClient);
        setLooper(workerLooper);
    }

    public void setLooper(Looper workerLooper) {
        mWorkerThreadHandler = new MainServiceImpl.WorkerHandler(workerLooper);
    }

    public void setHidlClient(IHidlConnectionInterface hidlClient) {
        mHidlConnectionInterface = hidlClient;
        mHidlConnectionInterface.registerCallback(mHidlConnectionCallback);
    }

    /* AIDL implementation */
    public Token enable5g(int slotId, Client client) throws RemoteException {
        int uid = Binder.getCallingUid();
        String packageName = mContext.getPackageManager().getNameForUid(uid);
        Log.d(TAG, "enable5g: slotId = " + slotId + " uid = " + uid + " package=" + packageName);

        if (!isClientValid(client)) return null;
        Token token = mHidlConnectionInterface.enable5g(slotId);
        mInflightRequests.put(token.get(), new Transaction(token, "Enable5g", client));
        return token;
    }

    public Token disable5g(int slotId, Client client) throws RemoteException {
        Log.d(TAG, "disable5g: slotId = " + slotId);
        if (!isClientValid(client)) return null;
        Token token = mHidlConnectionInterface.disable5g(slotId);
        mInflightRequests.put(token.get(), new Transaction(token, "disable5g", client));
        return token;
    }

    public Token enable5gOnly(int slotId, Client client) throws RemoteException {
        Log.d(TAG, "enable5gOnly: slotId = " + slotId);
        if (!isClientValid(client)) return null;
        throw new RemoteException("not implemented");
    }

    public Token query5gStatus(int slotId, Client client) throws RemoteException {
        Log.d(TAG, "query5gStatus: slotId = " + slotId);
        if (!isClientValid(client)) return null;
        Token token = mHidlConnectionInterface.query5gStatus(slotId);
        mInflightRequests.put(token.get(), new Transaction(token, "query5gStatus", client));
        return token;
    }

    public Token queryNrDcParam(int slotId, Client client) throws RemoteException {
        Log.d(TAG, "queryNrDcParam: slotId = " + slotId);
        if (!isClientValid(client)) return null;
        Token token = mHidlConnectionInterface.queryNrDcParam(slotId);
        mInflightRequests.put(token.get(), new Transaction(token, "queryNrDcParam", client));
        return token;
    }

    public Token queryNrBearerAllocation(int slotId, Client client) throws RemoteException {
        Log.d(TAG, "queryNrBearerAllocation: slotId = " + slotId);
        if (!isClientValid(client)) return null;
        Token token = mHidlConnectionInterface.queryNrBearerAllocation(slotId);
        mInflightRequests.put(token.get(), new Transaction(token, "queryNrBearerAllocation",
                client));
        return token;
    }

    public Token queryNrSignalStrength(int slotId, Client client) throws RemoteException {
        Log.d(TAG, "queryNrSignalStrength: slotId = " + slotId);
        if (!isClientValid(client)) return null;
        Token token = mHidlConnectionInterface.queryNrSignalStrength(slotId);
        mInflightRequests.put(token.get(), new Transaction(token, "queryNrSignalStrength",
                client));
        return token;
    }

    public Token queryUpperLayerIndInfo(int slotId, Client client) throws RemoteException {
        Log.d(TAG, "queryUpperLayerIndInfo: slotId = " + slotId);
        if (!isClientValid(client)) return null;
        Token token = mHidlConnectionInterface.queryUpperLayerIndInfo(slotId);
        mInflightRequests.put(token.get(), new Transaction(token, "queryUpperLayerIndInfo",
                client));
        return token;
    }

    public Token query5gConfigInfo(int slotId, Client client) throws RemoteException {
        Log.d(TAG, "query5gConfigInfo: slotId = " + slotId);
        if (!isClientValid(client)) return null;
        Token token = mHidlConnectionInterface.query5gConfigInfo(slotId);
        mInflightRequests.put(token.get(), new Transaction(token, "query5gConfigInfo",
                client));
        return token;
    }

    public Token queryNrIconType(int slotId, Client client) throws RemoteException {
        Log.d(TAG, "queryNrIconType: slotId = " + slotId);
        if (!isClientValid(client)) return null;
        Token token = mHidlConnectionInterface.queryNrIconType(slotId);
        mInflightRequests.put(token.get(), new Transaction(token, "queryNrIconType",
                client));
        return token;
    }

    public Token enableEndc(int slotId, boolean enabled, Client client) throws RemoteException {
        Log.d(TAG, "enableEndc: slotId = " + slotId);
        if (!isClientValid(client)) return null;
        Token token = mHidlConnectionInterface.enableEndc(slotId, enabled);
        mInflightRequests.put(token.get(), new Transaction(token, "enableEndc",
                client));
        return token;
    }

    public Token queryEndcStatus(int slotId, Client client) throws RemoteException {
        Log.d(TAG, "queryEndcStatus: slotId = " + slotId);
        if (!isClientValid(client)) return null;
        Token token = mHidlConnectionInterface.queryEndcStatus(slotId);
        mInflightRequests.put(token.get(), new Transaction(token, "queryEndcStatus",
                client));
        return token;
    }

    private boolean addCallback(INetworkCallback callback) {
        IBinder binder = callback.asBinder();
        synchronized (mCallbackList) {
            for (INetworkCallback it : mCallbackList) {
                if (it.asBinder().equals(binder)) {
                    // Found an existing callback with same binder.
                    return FAILED;
                }
            }
            Log.d(TAG, "add callback= " + callback);
            mCallbackList.add(callback);
        }
        return SUCCESS;
    }

    private void removeCallback(INetworkCallback callback) {
        IBinder binder = callback.asBinder();
        Log.d(TAG, "removeCallback: callback= " + callback + ", Binder = " + binder);
        synchronized (mCallbackList) {
            for (INetworkCallback it : mCallbackList) {
                if (it.asBinder().equals(binder)) {
                    Log.d(TAG, "remove callback= " + it);
                    mCallbackList.remove(it);
                    return;
                }
            }
        }
    }

    private void removeClientFromInflightRequests(INetworkCallback callback) {
        for(int key : mInflightRequests.keySet()) {
            Transaction txn = mInflightRequests.get(key);
            if (txn.mClient.getCallback().asBinder() == callback.asBinder()) {
                Log.d(TAG, "removeClientFromInflightRequests: Token = " + key + " => " +
                        mInflightRequests.get(key));
                mInflightRequests.remove(key);
            }
        }
    }

    private boolean isClientValid(Client client) {
        if (client == null || client.getCallback() == null) {
            Log.e(TAG, "Invalid client");
            return false;
        }
        synchronized (mCallbackList) {
            for (INetworkCallback it : mCallbackList) {
                if (it.asBinder().equals(client.getCallback().asBinder())) {
                    return true;
                }
            }
        }
        Log.d(TAG, "This client is unregistered: " + client);
        return false;
    }

    public Client registerCallback(String packageName, INetworkCallback callback) throws
            RemoteException {
        Client client = null;
        IBinder binder = callback.asBinder();

        binder.linkToDeath(new ClientBinderDeathRecipient(callback), 0);

        int uid = Binder.getCallingUid();
        String callerPackageName = mContext.getPackageManager().getNameForUid(uid);
        Log.d(TAG, "registerCallback: uid = " + uid + " callerPackage=" + callerPackageName +
                        "callback = " + callback + "binder = " + binder);
        mAppOpsManager.checkPackage(Binder.getCallingUid(), packageName);

        if (addCallback(callback) == SUCCESS) {
            client = new Client(++mClientIndex, uid, packageName, callback);
            Log.d(TAG, "registerCallback: client = " + client);

        } else {
            Log.d(TAG, "registerCallback: callback could not be added.");
        }
        return client;
    }

    public void unRegisterCallback(INetworkCallback callback) throws RemoteException {
        removeCallback(callback);
        removeClientFromInflightRequests(callback);
    }

    class ClientBinderDeathRecipient implements IBinder.DeathRecipient {
        INetworkCallback mCallback;

        public ClientBinderDeathRecipient(INetworkCallback callback) {
            Log.d(TAG, "registering for client cb = " + callback + " binder = "
                    + callback.asBinder() + " death " + "notification");
            mCallback = callback;
        }

        @Override
        public void binderDied() {
            Log.d(TAG, "Client callback = " + mCallback +" binder = " + mCallback.asBinder() +
                    "died");

            IBinder binder = mCallback.asBinder();
            binder.unlinkToDeath(this, 0);

            try {
                unRegisterCallback(mCallback);
            } catch (RemoteException e) {
                Log.d(TAG, "Exception while unregistering callback = " + mCallback + " binder = "
                        + mCallback.asBinder());
            }
        }
    }

    ArrayList<INetworkCallback> retrieveCallbacks(int tokenKey) {
        ArrayList<INetworkCallback> list = new ArrayList<INetworkCallback>();
        if (tokenKey != Token.UNSOL) {
            if (mInflightRequests.containsKey(tokenKey)) {
                Transaction txn = mInflightRequests.get(tokenKey);
                Client client = txn.mClient;
                if (isClientValid(client)) {
                    list.add(client.getCallback());
                } else {
                    Log.e(TAG, "This client is invalid now: " + client);
                }
            }
        } else {
            list = mCallbackList;
        }

        return list;
    }

    /* Private delegates */
    private void on5gStatus(int slotId, Token token, Status status, boolean enableStatus) {
        try {
            int tokenKey = token.get();
            synchronized (mCallbackList) {
                for (INetworkCallback callback : retrieveCallbacks(tokenKey)) {
                    Log.d(TAG, "on5gStatus: Responding back for transaction = " +
                            mInflightRequests.get(tokenKey));
                    callback.on5gStatus(slotId, token, status, enableStatus);
                    mInflightRequests.remove(tokenKey);
                }
            }
        } catch (RemoteException e) {
            Log.d(TAG, "on5gStatus: Exception = " + e);
        }
    }

    private void onNrDcParam(int slotId, Token token, Status status, DcParam dcParam) {
        try {
            int tokenKey = token.get();
            synchronized (mCallbackList) {
                for (INetworkCallback callback : retrieveCallbacks(tokenKey)) {
                    Log.d(TAG, "onNrDcParam: Responding back for transaction = " +
                            mInflightRequests.get(tokenKey));
                    callback.onNrDcParam(slotId, token, status, dcParam);
                    mInflightRequests.remove(tokenKey);
                }
            }
        } catch (RemoteException e) {
            Log.d(TAG, "on5gStatus: Exception = " + e);
        }
    }

    private void onUpperLayerIndInfo(int slotId, Token token, Status status,
                                          UpperLayerIndInfo ulInfo) {
        try {
            int tokenKey = token.get();
            synchronized (mCallbackList) {
                for (INetworkCallback callback : retrieveCallbacks(tokenKey)) {
                    Log.d(TAG, "onUpperLayerIndInfo: Responding back for transaction = " +
                            mInflightRequests.get(tokenKey));
                    callback.onUpperLayerIndInfo(slotId, token, status, ulInfo);
                    mInflightRequests.remove(tokenKey);
                }
            }
        } catch (RemoteException e) {
            Log.d(TAG, "onUpperLayerIndInfo: Exception = " + e);
        }
    }

    private void on5gConfigInfo(int slotId, Token token, Status status,
                                     NrConfigType nrConfigType) {
        try {
            int tokenKey = token.get();
            synchronized (mCallbackList) {
                for (INetworkCallback callback : retrieveCallbacks(tokenKey)) {
                    Log.d(TAG, "on5gConfigInfo: Responding back for transaction = " +
                            mInflightRequests.get(tokenKey));
                    callback.on5gConfigInfo(slotId, token, status, nrConfigType);
                    mInflightRequests.remove(tokenKey);
                }
            }
        } catch (RemoteException e) {
            Log.d(TAG, "on5gConfigInfo: Exception = " + e);
        }
    }

    private void onAnyNrBearerAllocation(int slotId, Token token, Status status,
            BearerAllocationStatus bearerStatus) {
        try {
            int tokenKey = token.get();
            synchronized (mCallbackList) {
                for (INetworkCallback callback : retrieveCallbacks(tokenKey)) {
                    Log.d(TAG, "onAnyNrBearerAllocation: Responding back for transaction = " +
                            mInflightRequests.get(tokenKey));
                    callback.onAnyNrBearerAllocation(slotId, token, status, bearerStatus);
                    mInflightRequests.remove(tokenKey);
                }
            }
        } catch (RemoteException e) {
            Log.d(TAG, "on5gStatus: Exception = " + e);
        }
    }

    private void onSignalStrength(int slotId, Token token, Status status, SignalStrength
            signalStrength) {
        try {
            int tokenKey = token.get();
            synchronized (mCallbackList) {
                for (INetworkCallback callback : retrieveCallbacks(tokenKey)) {
                    Log.d(TAG, "onSignalStrength: Responding back for transaction = " +
                            mInflightRequests.get(tokenKey));
                    callback.onSignalStrength(slotId, token, status, signalStrength);
                    mInflightRequests.remove(tokenKey);
                }
            }
        } catch (RemoteException e) {
            Log.d(TAG, "on5gStatus: Exception = " + e);
        }
    }

    private void onNrIconType(int slotId, Token token, Status status,
                                NrIconType nrIconType) {
        try {
            int tokenKey = token.get();
            synchronized (mCallbackList) {
                for (INetworkCallback callback : retrieveCallbacks(tokenKey)) {
                    Log.d(TAG, "onNrIconType: Responding back for transaction = " +
                            mInflightRequests.get(tokenKey));
                    callback.onNrIconType(slotId, token, status, nrIconType);
                    mInflightRequests.remove(tokenKey);
                }
            }
        } catch (RemoteException e) {
            Log.d(TAG, "onNrIconType: Exception = " + e);
        }
    }

    private void onEnableEndc(int slotId, Token token, Status status) {
        try {
            int tokenKey = token.get();
            synchronized (mCallbackList) {
                for (INetworkCallback callback : retrieveCallbacks(tokenKey)) {
                    Log.d(TAG, "onEnableEndc: Responding back for transaction = " +
                            mInflightRequests.get(tokenKey));
                    callback.onEnableEndc(slotId, token, status);
                    mInflightRequests.remove(tokenKey);
                }
            }
        } catch (RemoteException e) {
            Log.d(TAG, "onEnableEndc: Exception = " + e);
        }
    }

    private void onEndcStatus(int slotId, Token token, Status status, boolean enableStatus) {
        try {
            int tokenKey = token.get();
            synchronized (mCallbackList) {
                for (INetworkCallback callback : retrieveCallbacks(tokenKey)) {
                    Log.d(TAG, "onEndcStatus: Responding back for transaction = " +
                            mInflightRequests.get(tokenKey));
                    callback.onEndcStatus(slotId, token, status, enableStatus);
                    mInflightRequests.remove(tokenKey);
                }
            }
        } catch (RemoteException e) {
            Log.d(TAG, "onEndcStatus: Exception = " + e);
        }
    }

    public int getAidlClientsCount() {
        synchronized (mCallbackList) {
            return mCallbackList.size();
        }
    }

    public int getInflightRequestsCount() {
        return mInflightRequests.size();
    }

    private void dumpAidlClients(PrintWriter pw) {
        synchronized (mCallbackList) {
            for (INetworkCallback callback : mCallbackList) {
                IBinder binder = callback.asBinder();
                pw.println("Callback = " + callback + "-> Binder = " + binder);
            }
        }
    }

    private void dumpInflightRequests(PrintWriter pw){
        for(Integer key : mInflightRequests.keySet()) {
            pw.println("Token = " + key + " => " + mInflightRequests.get(key));
        }
    }

    // Dump service.
    public void dump(FileDescriptor fd, PrintWriter printwriter, String[] args) {
        PrintWriter pw = printwriter;
        pw.println("5G-Middleware:");
        pw.println("mHidlConnectionInterface = " + mHidlConnectionInterface);
        pw.println("AIDL clients : ");
        dumpAidlClients(pw);
        pw.flush();

        pw.println("Inflight requests : ");
        dumpInflightRequests(pw);
        pw.flush();
    }
}
