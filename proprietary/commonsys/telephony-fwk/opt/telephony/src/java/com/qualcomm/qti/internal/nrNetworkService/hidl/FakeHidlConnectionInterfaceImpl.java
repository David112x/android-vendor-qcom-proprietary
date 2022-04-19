/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 */

package com.qualcomm.qti.internal.nrNetworkService.hidl;

import android.os.AsyncResult;
import android.os.RemoteException;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.os.Looper;
import android.util.Log;

import org.codeaurora.internal.BearerAllocationStatus;
import org.codeaurora.internal.DcParam;
import org.codeaurora.internal.NrConfigType;
import org.codeaurora.internal.SignalStrength;
import org.codeaurora.internal.Status;
import org.codeaurora.internal.Token;
import org.codeaurora.internal.UpperLayerIndInfo;
import org.codeaurora.internal.NrIconType;

public class FakeHidlConnectionInterfaceImpl implements IHidlConnectionInterface {
    private static final String TAG = "FakeHidlConnectionInterfaceImpl";

    private HandlerThread mWorkerThread = new HandlerThread(TAG + "BgThread");
    private Handler mWorkerHandler;
    private IHidlConnectionCallback mCallback;
    private int mSerial = -1;
    private final Token UNSOL = new Token(-1);

    private boolean m5gEnabledState = false;

    private final int EVENT_SIMULATE_5G_ENABLED = 0;
    private final int EVENT_SIMULATE_5G_DISABLED = 1;
    private final int EVENT_SIMULATE_QUERY_5G_STATUS = 2;
    private final int EVENT_SIMULATE_QUERY_NRDC_PARAM = 3;
    private final int EVENT_SIMULATE_QUERY_BEARER_ALLOCATION = 4;
    private final int EVENT_SIMULATE_QUERY_SIGNAL_STRENGTH = 5;
    private final int EVENT_SIMULATE_QUERY_UPPER_LAYER_IND_INFO = 6;
    private final int EVENT_SIMULATE_QUERY_CONFIG_TYPE = 7;
    private final int EVENT_SIMULATE_QUERY_ICON_TYPE = 8;
    private final int EVENT_SIMULATE_ENABLE_ENDC = 9;
    private final int EVENT_SIMULATE_QUERY_ENDC_STATUS = 10;


    public FakeHidlConnectionInterfaceImpl() {
        mWorkerThread.start();
        Looper workerLooper = mWorkerThread.getLooper();
        mWorkerHandler = new WorkerHandler(workerLooper);
        Log.d(TAG, "constructor... using its own bg thread Looper = " + workerLooper);
    }

    public FakeHidlConnectionInterfaceImpl(Looper workerLooper) {
        Log.d(TAG, "constructor... Looper = " + workerLooper);
        mWorkerHandler = new WorkerHandler(workerLooper);
    }

    private Token getNextToken() {
        return new Token(++mSerial);
    }

    private class WorkerHandler extends Handler {
        private static final String TAG = "FakeHidleConnection-WorkerHandler";
        public WorkerHandler() {
            super();
        }

        public WorkerHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Log.d(TAG, "handleMessage msg.what = " + msg.what);
            switch (msg.what) {
                case EVENT_SIMULATE_5G_ENABLED: {
                    int slotId = msg.arg1;
                    Token token = (Token) msg.obj;
                    if (mCallback != null) {
                        Log.d(TAG, "EVENT_SIMULATE_5G_ENABLED: token = " + token);
                        mCallback.on5gStatus(slotId, token, new Status(Status.SUCCESS), true);

                        mCallback.onNrDcParam(slotId, UNSOL, new Status(Status.SUCCESS),
                                new DcParam(DcParam.ENDC_AVAILABLE, DcParam.DCNR_UNRESTRICTED));
                        mCallback.onUpperLayerIndInfo(slotId, UNSOL, new Status(Status.SUCCESS),
                                new UpperLayerIndInfo(UpperLayerIndInfo.PLMN_INFO_LIST_AVAILABLE,
                                UpperLayerIndInfo.UPPER_LAYER_IND_INFO_AVAILABLE));
                        mCallback.onAnyNrBearerAllocation(slotId, UNSOL, new Status(Status.SUCCESS),
                                new BearerAllocationStatus(BearerAllocationStatus.MMW_ALLOCATED));
                        mCallback.onSignalStrength(slotId, UNSOL, new Status(Status.SUCCESS),
                                new SignalStrength());
                        mCallback.on5gConfigInfo(slotId, UNSOL, new Status(Status.SUCCESS),
                                new NrConfigType(NrConfigType.NSA_CONFIGURATION));
                        mCallback.onNrIconType(slotId, UNSOL, new Status(Status.SUCCESS),
                                new NrIconType(NrIconType.TYPE_5G_BASIC));

                        mCallback.on5gStatus(slotId, UNSOL, new Status(Status.SUCCESS), true);

                        m5gEnabledState = true;
                    }
                    break;
                }

                case EVENT_SIMULATE_5G_DISABLED: {
                    int slotId = msg.arg1;
                    Token token = (Token) msg.obj;
                    if (mCallback != null) {
                        Log.d(TAG, "EVENT_SIMULATE_5G_DISABLED: token = " + token);
                        mCallback.on5gStatus(slotId, token, new Status(Status.SUCCESS), false);

                        mCallback.onNrDcParam(slotId, UNSOL, new Status(Status.SUCCESS),
                                new DcParam(DcParam.ENDC_UNAVAILABLE, DcParam.DCNR_RESTRICTED));
                        mCallback.onUpperLayerIndInfo(slotId, UNSOL, new Status(Status.SUCCESS),
                                new UpperLayerIndInfo(UpperLayerIndInfo.PLMN_INFO_LIST_UNAVAILABLE,
                                UpperLayerIndInfo.UPPER_LAYER_IND_INFO_UNAVAILABLE));
                        mCallback.onAnyNrBearerAllocation(slotId, UNSOL, new Status(Status.SUCCESS),
                                new BearerAllocationStatus(BearerAllocationStatus.NOT_ALLOCATED));
                        mCallback.onSignalStrength(slotId, UNSOL, new Status(Status.SUCCESS), null);
                        mCallback.onNrIconType(slotId, UNSOL, new Status(Status.SUCCESS),
                                new NrIconType(NrIconType.TYPE_NONE));

                        mCallback.on5gStatus(slotId, UNSOL, new Status(Status.SUCCESS), false);

                        m5gEnabledState = false;

                    }
                    break;
                }

                case EVENT_SIMULATE_QUERY_5G_STATUS: {
                    int slotId = msg.arg1;
                    Token token = (Token) msg.obj;
                    if (mCallback != null) {
                        Log.d(TAG, "EVENT_SIMULATE_QUERY_5G_STATUS: token = " + token);
                        mCallback.on5gStatus(slotId, token, new Status(Status.SUCCESS),
                                m5gEnabledState);
                    }
                }
                break;

                case EVENT_SIMULATE_QUERY_NRDC_PARAM: {
                    int slotId = msg.arg1;
                    Token token = (Token) msg.obj;
                    if (mCallback != null) {
                        Log.d(TAG, "EVENT_SIMULATE_QUERY_NRDC_PARAM: token = " + token);
                        DcParam dcParam = (m5gEnabledState)?
                                new DcParam(DcParam.ENDC_AVAILABLE, DcParam.DCNR_UNRESTRICTED) :
                                new DcParam(DcParam.ENDC_UNAVAILABLE, DcParam.DCNR_RESTRICTED);
                        mCallback.onNrDcParam(slotId, token, new Status(Status.SUCCESS),
                                    dcParam);
                    }
                }
                break;

                case EVENT_SIMULATE_QUERY_BEARER_ALLOCATION: {
                    int slotId = msg.arg1;
                    Token token = (Token) msg.obj;
                    if (mCallback != null) {
                        Log.d(TAG, "EVENT_SIMULATE_QUERY_BEARER_ALLOCATION: token = " + token);
                        BearerAllocationStatus bearerAllocationStatus = (m5gEnabledState)?
                                new BearerAllocationStatus(
                                BearerAllocationStatus.MMW_ALLOCATED) :
                                new BearerAllocationStatus(
                                BearerAllocationStatus.NOT_ALLOCATED);
                        mCallback.onAnyNrBearerAllocation(slotId, token, new Status(Status
                                .SUCCESS), bearerAllocationStatus);
                    }
                }
                break;

                case EVENT_SIMULATE_QUERY_SIGNAL_STRENGTH: {
                    int slotId = msg.arg1;
                    Token token = (Token) msg.obj;
                    if (mCallback != null) {
                        Log.d(TAG, "EVENT_SIMULATE_QUERY_SIGNAL_STRENGTH: token = " + token);
                        SignalStrength ss = (m5gEnabledState)? new SignalStrength() : null;
                        mCallback.onSignalStrength(slotId, token, new Status(Status
                                    .SUCCESS), ss);
                    }
                }
                break;

                case EVENT_SIMULATE_QUERY_UPPER_LAYER_IND_INFO: {
                    int slotId = msg.arg1;
                    Token token = (Token) msg.obj;
                    if (mCallback != null) {
                        Log.d(TAG, "EVENT_SIMULATE_QUERY_UPPER_LAYER_IND_INFO: token = " +
                                token);
                        UpperLayerIndInfo upperLayerIndInfo = (m5gEnabledState)?
                                new UpperLayerIndInfo(UpperLayerIndInfo.
                                PLMN_INFO_LIST_AVAILABLE, UpperLayerIndInfo.
                                UPPER_LAYER_IND_INFO_AVAILABLE) : new UpperLayerIndInfo(
                                UpperLayerIndInfo.PLMN_INFO_LIST_UNAVAILABLE,
                                UpperLayerIndInfo.UPPER_LAYER_IND_INFO_UNAVAILABLE);
                        mCallback.onUpperLayerIndInfo(slotId, token, new Status(Status
                                .SUCCESS), upperLayerIndInfo);
                    }
                }
                break;

                case EVENT_SIMULATE_QUERY_CONFIG_TYPE: {
                    int slotId = msg.arg1;
                    Token token = (Token) msg.obj;
                    if (mCallback != null) {
                        Log.d(TAG, "EVENT_SIMULATE_QUERY_CONFIG_TYPE: token = " +
                                token);
                        NrConfigType nrConfigType = new NrConfigType(
                                NrConfigType.NSA_CONFIGURATION);
                        mCallback.on5gConfigInfo(slotId, token, new Status(Status
                                .SUCCESS), nrConfigType);
                    }
                }
                break;

                case EVENT_SIMULATE_QUERY_ICON_TYPE: {
                    int slotId = msg.arg1;
                    Token token = (Token) msg.obj;
                    if (mCallback != null) {
                        Log.d(TAG, "EVENT_SIMULATE_QUERY_ICON_TYPE: token = " +
                                token);
                        NrIconType nrIconType = new NrIconType(
                                NrIconType.TYPE_5G_BASIC);
                        mCallback.onNrIconType(slotId, token, new Status(Status
                                .SUCCESS), nrIconType);
                    }
                }
                break;

                case EVENT_SIMULATE_ENABLE_ENDC: {
                    int slotId = msg.arg1;
                    Token token = (Token) msg.obj;
                    if (mCallback != null) {
                        Log.d(TAG, "EVENT_SIMULATE_ENABLE_ENDC: token = " +
                                token);
                        mCallback.onEnableEndc(slotId, token, new Status(Status
                                .SUCCESS));
                    }
                }
                break;

                case EVENT_SIMULATE_QUERY_ENDC_STATUS: {
                    int slotId = msg.arg1;
                    Token token = (Token) msg.obj;
                    if (mCallback != null) {
                        Log.d(TAG, "EVENT_SIMULATE_QUERY_ENDC_STATUS: token = " +
                                token);
                        mCallback.onEndcStatus(slotId, token, new Status(Status
                                .SUCCESS), true);
                    }
                }
                break;
            }
        }
    }

    @Override
    public Token enable5g(int slotId) throws RemoteException {
        Log.d(TAG, "enable5g: ");
        Token token = getNextToken();
        if (mCallback != null) {
            mWorkerHandler.sendMessageDelayed(mWorkerHandler
                    .obtainMessage(EVENT_SIMULATE_5G_ENABLED, slotId, -1, token), 2 * 1000);
        }
        Log.d(TAG, "enable5g: token = " + token);
        return token;
    }

    @Override
    public Token disable5g(int slotId) throws RemoteException {
        Log.d(TAG, "disable5g: ");
        Token token = getNextToken();
        mWorkerHandler.sendMessageDelayed(mWorkerHandler
                .obtainMessage(EVENT_SIMULATE_5G_DISABLED, slotId, -1, token), 2 * 1000);
        Log.d(TAG, "disable5g: token = " + token);
        return token;
    }

    @Override
    public Token enable5gOnly(int slotId) throws RemoteException {
        return null;
    }

    @Override
    public Token query5gStatus(int slotId) throws RemoteException {
        Log.d(TAG, "query5gStatus: ");
        Token token = getNextToken();

        mWorkerHandler.sendMessage(mWorkerHandler
                .obtainMessage(EVENT_SIMULATE_QUERY_5G_STATUS, slotId, -1, token));
        Log.d(TAG, "query5gStatus: token = " + token);
        return token;
    }

    @Override
    public Token query5gConfigInfo(int slotId) throws RemoteException {
        Log.d(TAG, "query5gConfigInfo: ");
        Token token = getNextToken();

        mWorkerHandler.sendMessage(mWorkerHandler
                .obtainMessage(EVENT_SIMULATE_QUERY_CONFIG_TYPE, slotId, -1, token));
        Log.d(TAG, "query5gConfigInfo: token = " + token);
        return token;
    }

    @Override
    public Token queryNrDcParam(int slotId) throws RemoteException {
        Log.d(TAG, "queryNrDcParam: ");
        Token token = getNextToken();
        mWorkerHandler.sendMessage(mWorkerHandler
                .obtainMessage(EVENT_SIMULATE_QUERY_NRDC_PARAM, slotId, -1, token));
        Log.d(TAG, "queryNrDcParam: token = " + token);
        return token;
    }

    @Override
    public Token queryNrBearerAllocation(int slotId) throws RemoteException {
        Log.d(TAG, "queryNrBearerAllocation: ");
        Token token = getNextToken();
        mWorkerHandler.sendMessage(mWorkerHandler
                .obtainMessage(EVENT_SIMULATE_QUERY_BEARER_ALLOCATION, slotId, -1, token));
        Log.d(TAG, "queryNrBearerAllocation: token = " + token);
        return token;
    }

    @Override
    public Token queryNrSignalStrength(int slotId) throws RemoteException {
        Log.d(TAG, "queryNrSignalStrength: ");
        Token token = getNextToken();
        mWorkerHandler.sendMessage(mWorkerHandler
                .obtainMessage(EVENT_SIMULATE_QUERY_SIGNAL_STRENGTH, slotId, -1, token));
        Log.d(TAG, "queryNrSignalStrength: token = " + token);
        return token;
    }

    @Override
    public Token queryUpperLayerIndInfo(int slotId) throws RemoteException {
        Log.d(TAG, "queryUpperLayerIndInfo: ");
        Token token = getNextToken();
        mWorkerHandler.sendMessage(mWorkerHandler
                .obtainMessage(EVENT_SIMULATE_QUERY_UPPER_LAYER_IND_INFO, slotId, -1, token));
        Log.d(TAG, "queryUpperLayerIndInfo: token = " + token);
        return token;
    }

    @Override
    public Token queryNrIconType(int slotId) throws RemoteException {
        Log.d(TAG, "queryNrIconType: ");
        Token token = getNextToken();
        mWorkerHandler.sendMessage(mWorkerHandler
                .obtainMessage(EVENT_SIMULATE_QUERY_ICON_TYPE, slotId, -1, token));
        Log.d(TAG, "queryNrIconType: token = " + token);
        return token;
    }

    @Override
    public Token enableEndc(int slotId, boolean enable) throws RemoteException {
        Log.d(TAG, "enableEndc: ");
        Token token = getNextToken();
        mWorkerHandler.sendMessage(mWorkerHandler
                .obtainMessage(EVENT_SIMULATE_ENABLE_ENDC, slotId, -1, token));
        Log.d(TAG, "enableEndc: token = " + token);
        return token;
    }

    @Override
    public Token queryEndcStatus(int slotId) throws RemoteException {
        Log.d(TAG, "queryEndcStatus: ");
        Token token = getNextToken();
        mWorkerHandler.sendMessage(mWorkerHandler
                .obtainMessage(EVENT_SIMULATE_QUERY_ENDC_STATUS, slotId, -1, token));
        Log.d(TAG, "queryEndcStatus: token = " + token);
        return token;
    }

    @Override
    public void registerCallback(IHidlConnectionCallback callback) {
        Log.d(TAG, "registerCallback: callback = " + callback);
        mCallback = callback;
    }

    @Override
    public void unRegisterCallback(IHidlConnectionCallback callback) {
        Log.d(TAG, "unRegisterCallback: callback = " + callback);
        if (mCallback == callback) {
            mCallback = null;
        }
    }
}
