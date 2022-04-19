/*
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

import android.Manifest;
import android.content.Context;
import java.util.HashMap;
import java.util.HashSet;
import java.util.ArrayList;
import java.util.List;
import java.util.Collection;

import com.android.internal.annotations.VisibleForTesting;

import org.codeaurora.ims.ImsMultiIdentityManager;
import org.codeaurora.ims.internal.IImsMultiIdentityListener;

import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.RemoteException;
import com.qualcomm.ims.utils.Log;
import org.codeaurora.telephony.utils.AsyncResult;

public class ImsMultiIdentityImpl extends ImsMultiIdentityControllerBase{
    private ImsSenderRxr mCi;
    private int mPhoneId;
    private Context mContext;
    private Handler mHandler;
    private volatile IImsMultiIdentityListener mMultiIdentityListener;
    private HashMap<String, MultiIdentityLineInfo> mLinesInfoMap;
    private HashSet<String> mRegisteredSet;

    private final IBinder.DeathRecipient recipient = new IBinder.DeathRecipient() {
        @Override
        public void binderDied() {
            onClientDeath();
        }
    };

    private final String MODIFY_PHONE_STATE = Manifest.permission.MODIFY_PHONE_STATE;
    private final String READ_PHONE_STATE = Manifest.permission.READ_PHONE_STATE;

    private final int EVENT_MULTI_IDENTITY_UPDATE_REGISTRATION_STATE_RESPONSE = 1;
    private final int EVENT_MULTI_IDENTITY_REGISTRATION_STATUS_CHANGE = 2;
    private final int EVENT_MULTI_IDENTITY_INFO_PENDING_INDICATION = 3;
    private final int EVENT_MULTI_IDENTITY_INFO_PENDING_RESPONSE = 4;
    private final int EVENT_MULTI_IDENTITY_VIRTUAL_LINE_INFO_RESPONSE = 5;

    public ImsMultiIdentityImpl(int phoneId, ImsSenderRxr senderRxr, Context context,
            Looper looper) {
        mPhoneId = phoneId;
        mCi = senderRxr;
        mContext = context;
        mHandler = new ImsMultiIdentityHandler(looper);

        //handler for multi idenity lines registration status change UNSOL
        mCi.registerForMultiIdentityRegistrationStatusChange(mHandler,
                EVENT_MULTI_IDENTITY_REGISTRATION_STATUS_CHANGE, null);
        //handler for modem query about the lines' status UNSOL
        mCi.registerForMultiIdentityInfoPendingIndication(mHandler,
                EVENT_MULTI_IDENTITY_INFO_PENDING_INDICATION, null);
        mLinesInfoMap = new HashMap<>();
        mRegisteredSet = new HashSet<>();
    }

    //Handler for the events on response from ImsSenderRxr
    private class ImsMultiIdentityHandler extends Handler {
        ImsMultiIdentityHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Log.i (this, "Message received: what = " + msg.what);
            AsyncResult ar = (AsyncResult) msg.obj;

            switch (msg.what) {
                case EVENT_MULTI_IDENTITY_UPDATE_REGISTRATION_STATE_RESPONSE:
                    onRegisterMultiIdentityLinesResponse(ar);
                    break;
                case EVENT_MULTI_IDENTITY_REGISTRATION_STATUS_CHANGE:
                    onMultiIdentityRegistrationStatusChange(ar);
                    break;
                case EVENT_MULTI_IDENTITY_INFO_PENDING_INDICATION:
                    onMultiIdentityInfoPendingIndication();
                    break;
                case EVENT_MULTI_IDENTITY_INFO_PENDING_RESPONSE:
                    onMultiIdentityInfoPendingResponse(ar);
                    break;
                case EVENT_MULTI_IDENTITY_VIRTUAL_LINE_INFO_RESPONSE:
                    onMultiIdentityVirtualLineInfoResponse(ar);
                    break;
                default:
                    Log.i( this, "Unknown message = " + msg.what);
            }
        }
    }

    public void setMultiIdentityListener(IImsMultiIdentityListener listener) throws RemoteException
    {
        mContext.enforceCallingOrSelfPermission(MODIFY_PHONE_STATE, "setMultiIdentityListener");
        IImsMultiIdentityListener intf = mMultiIdentityListener;
        if (intf != null) {
            IBinder binder = intf.asBinder();
            binder.unlinkToDeath(recipient, 0);
        }
        mMultiIdentityListener = null;
        //Client can set a NULL listener.
        if (listener != null) {
            IBinder binder = listener.asBinder();
            binder.linkToDeath(recipient, 0);
        } else {
            Log.w(this, "Trying to set a NULL listener.");
        }
        mMultiIdentityListener = listener;
    }

    private IImsMultiIdentityListener getMultiIdentityListener() throws RemoteException {
        IImsMultiIdentityListener intf = mMultiIdentityListener;
        if (intf == null) {
            Log.e(this, "getMultiIdentityListener :: IImsMultiIdentityListener is null.");
            throw new RemoteException("getMultiIdentityListener :: " +
                   "IImsMultiIdentityListener is null.");
        }
        return intf;
    }

    //Multi Identity Client Process Death
    private void onClientDeath() {
        mMultiIdentityListener = null;
    }

    public void queryVirtualLineInfo(String msisdn) throws RemoteException{
        mContext.enforceCallingOrSelfPermission(READ_PHONE_STATE, "queryVirtualLineInfo");
        if (msisdn == null) {
            Log.e(this, "queryVirtualLineInfo :: msisdn is null.");
            throw new RemoteException("queryVirtualLineInfo :: msisdn is null.");
        }
        mCi.queryVirtualLineInfo(msisdn, mHandler.obtainMessage(
                EVENT_MULTI_IDENTITY_VIRTUAL_LINE_INFO_RESPONSE));
    }

    private void onMultiIdentityVirtualLineInfoResponse(AsyncResult ar) {
        VirtualLineInfo virtualInfo = (VirtualLineInfo)ar.result;
        if (virtualInfo == null) {
            Log.e(this, "multiIdentityVirtualLineInfoResponse :: virtualInfo is null.");
            return;
        }
        String msisdn = virtualInfo.mMsisdn;
        ArrayList<String> virtualLines = virtualInfo.mVirtualInfo;
        try {
            getMultiIdentityListener().onQueryVirtualLineInfoResponse(
                    mPhoneId, msisdn, virtualLines);
        } catch (Throwable t) {
            Log.e(this, "multiIdentityVirtualLineInfoResponse exception : " + t);
        }
    }

    private void onMultiIdentityInfoPendingResponse(AsyncResult ar) {
        Log.v(this, "multiIdentityInfoPendingResponse");
        if (ar == null || ar.exception != null) {
            Log.e(this, "multiIdentity Info Pending action was unsuccessfull");
        }
    }

    private void onMultiIdentityInfoPendingIndication() {
        //Send registration request for all the lines cached in mLinesInfoMap
        Collection<MultiIdentityLineInfo> lines;
        synchronized(mLinesInfoMap) {
            lines = mLinesInfoMap.values();
        }
        mCi.requestMultiIdentityLinesRegistration(lines,
            mHandler.obtainMessage(EVENT_MULTI_IDENTITY_INFO_PENDING_RESPONSE));
    }

    /**
     *  This function will send a list of the lines that need to be registered to modem
     *
     * */
    public void updateRegistrationStatus(List<MultiIdentityLineInfo> linesInfo) {
        mContext.enforceCallingOrSelfPermission(MODIFY_PHONE_STATE, "updateRegistrationStatus");
        Collection<MultiIdentityLineInfo> lines;
        synchronized(mLinesInfoMap) {
            for (MultiIdentityLineInfo line : linesInfo) {
                String key = line.getMsisdn();
                if (line.getLineStatus() == MultiIdentityLineInfo.LINE_STATUS_INACTIVE) {
                    //To deregister a line, remove it from the map
                    mLinesInfoMap.remove(key);
                    continue;
                }
                mLinesInfoMap.put(key, new MultiIdentityLineInfo(line));
            }
            lines = mLinesInfoMap.values();
        }
        //Registration request needs to be sent out for all the lines in the map
        //No need to include a listener in the Message because we will be using
        //mMultiIdentityListener for the response
        mCi.requestMultiIdentityLinesRegistration(lines,
            mHandler.obtainMessage(EVENT_MULTI_IDENTITY_UPDATE_REGISTRATION_STATE_RESPONSE));
    }

    private void onRegisterMultiIdentityLinesResponse(AsyncResult ar) {
        if (ar == null) {
            Log.e(this, "registerMultiIdentityLinesResponse :: NULL response received. Returning");
            return;
        }
        //in the future, may contain error codes.
        //If no exception is received, send the response as success
        int ret = (ar.exception == null) ? ImsMultiIdentityManager.REGISTRATION_RESPONSE_SUCCESS:
                ImsMultiIdentityManager.REGISTRATION_RESPONSE_FAILURE;
        try {
            getMultiIdentityListener().onUpdateRegistrationInfoResponse(mPhoneId, ret);
        } catch (Throwable t) {
            Log.e(this, "registerMultiIdentityLinesResponse exception : " + t);
        }
    }

    private void onMultiIdentityRegistrationStatusChange(AsyncResult ar) {
        if (ar == null || ar.result == null) {
            Log.e(this, "onMultiIdentityRegistrationStatusChange :: AsyncResult is null.");
            return;
        }
        if (ar.exception != null) {
            Log.e(this, "onMultiIdentityRegistrationStatusChange :: AsyncResult exception : "
                    + ar.exception);
            return;
        }

        ArrayList<MultiIdentityLineInfo> lines = (ArrayList<MultiIdentityLineInfo>) ar.result;
        synchronized (mRegisteredSet) {
            mRegisteredSet.clear();
            for (MultiIdentityLineInfo line : lines) {
                String key = line.getMsisdn();
                if (line.getLineStatus() == MultiIdentityLineInfo.LINE_STATUS_ACTIVE) {
                    //caching the msisdn instead of a MultiIdentityLineInfo object
                    mRegisteredSet.add(key);
                }
            }
        }
        try {
            //Send the information of the lines to the user
            getMultiIdentityListener().onRegistrationStatusChange(mPhoneId, lines);
        } catch (Throwable t) {
            Log.e(this, "onMultiIdentityRegistrationStatusChange exception : " + t);
        }
    }

    @VisibleForTesting
    public Handler getHandler() {
        return mHandler;
    }

}
