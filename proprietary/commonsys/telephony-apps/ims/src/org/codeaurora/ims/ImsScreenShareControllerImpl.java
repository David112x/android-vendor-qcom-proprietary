/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

import android.content.Context;
import android.Manifest;
import android.os.IBinder;
import android.os.RemoteException;

import com.qualcomm.ims.utils.Log;

import java.util.List;

import org.codeaurora.ims.ImsScreenShareManager;
import org.codeaurora.ims.internal.IImsScreenShareListener;

public class ImsScreenShareControllerImpl extends ImsScreenShareControllerBase {
    private ImsServiceSub mServiceSub;
    private Context mContext;
    private volatile IImsScreenShareListener mScreenShareListener;

    private final IBinder.DeathRecipient recipient = new IBinder.DeathRecipient() {
        @Override
        public void binderDied() {
            onClientDeath();
        }
    };

    private final String MODIFY_PHONE_STATE = Manifest.permission.MODIFY_PHONE_STATE;

    public ImsScreenShareControllerImpl(ImsServiceSub serviceSub, Context context) {
        mServiceSub = serviceSub;
        mContext = context;
    }

    //Screen Share Client Process Death
    private void onClientDeath() {
        mScreenShareListener = null;
    }

    public void onSetScreenShareListener(IImsScreenShareListener listener)
            throws RemoteException {
        mContext.enforceCallingOrSelfPermission(MODIFY_PHONE_STATE, "setScreenShareListener");
        List<ImsCallSessionImpl> sessionList =
                mServiceSub.getCallSessionByState(DriverCallIms.State.ACTIVE);

        if (sessionList.isEmpty()) {
            Log.d(this, "onSetScreenShareListener: no call is available");
            return;
        }

        IImsScreenShareListener intf = mScreenShareListener;
        if (intf != null) {
            IBinder binder = intf.asBinder();
            binder.unlinkToDeath(recipient, 0);
        }

        //Client can set a NULL listener.
        if (listener != null) {
            IBinder binder = listener.asBinder();
            binder.linkToDeath(recipient, 0);
        } else {
            Log.w(this, "Trying to set a NULL listener.");
        }
        mScreenShareListener = listener;

        sessionList.get(0).setScreenShareListener(listener);
    }

    public void onStartScreenShare(int width, int height) {
        mContext.enforceCallingOrSelfPermission(MODIFY_PHONE_STATE, "onStartScreenShare");
        List<ImsCallSessionImpl> sessionList =
                mServiceSub.getCallSessionByState(DriverCallIms.State.ACTIVE);

        if (sessionList.isEmpty()) {
            Log.d(this, "onStartScreenShare: no call is available");
            return;
        }

        sessionList.get(0).startScreenShare(width, height);
    }

    public void onStopScreenShare() {
        mContext.enforceCallingOrSelfPermission(MODIFY_PHONE_STATE, "onStopScreenShare");
        List<ImsCallSessionImpl> sessionList =
                mServiceSub.getCallSessionByState(DriverCallIms.State.ACTIVE);

        if (sessionList.isEmpty()) {
            Log.d(this, "onStopScreenShare: no call is available");
            return;
        }
        sessionList.get(0).stopScreenShare();
    }
}
