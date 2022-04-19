/* Copyright (c) 2015-2016, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.ims.vt;

import android.content.Context;
import android.util.Log;

import org.codeaurora.ims.DriverCallIms;
import org.codeaurora.ims.ImsCallSessionImpl;
import org.codeaurora.ims.ImsServiceSub;

import java.util.List;

// TODO Maybe rename to ImsGlobals..
public class ImsVideoGlobals {
    private static String TAG = "VideoCall_ImsVideoGlobals";

    private static ImsVideoGlobals sInstance = null;
    private int mActivePhoneId = 0;
    private ImsServiceSub[] mServiceSubs;
    private CameraController mCameraController;
    private MediaController mMediaController;
    private LowBatteryHandler mLowBatteryHandler;

    private Context mContext;


    public static synchronized void init(ImsServiceSub[] serviceSubArr, Context context) {
        if (serviceSubArr == null || serviceSubArr.length < 1 || serviceSubArr[0] == null) {
            throw new IllegalArgumentException("Default subscription is null.");
        }

        if (sInstance == null) {
            sInstance = new ImsVideoGlobals(serviceSubArr, context);
        } else {
            throw new RuntimeException("ImsVideoGlobals: Multiple initializaiton.");
        }
    }

    public synchronized static ImsVideoGlobals getInstance() {
        if (sInstance == null) {
            throw new RuntimeException("ImsVideoGlobals: getInstance called before init.");
        }
        return sInstance;
    }

    public void dispose() {
        log("dispose()");
        synchronized(ImsVideoGlobals.class) {
            for (ImsServiceSub sub: mServiceSubs) {
                sub.removeListener(mMediaController);
            }
            getActiveServiceSub().removeListener(mLowBatteryHandler);
            mCameraController.dispose();
            mMediaController.dispose();
            mLowBatteryHandler.dispose();
            sInstance = null;
        }
    }

    // TODO Introduce onSubscriptonChanged event and subscribe for that event.
    public void setPhoneIdWithActiveCall(int phoneId) {
        log("setPhoneIdWithActiveCall, phoneId # " + phoneId);
        if (phoneId < 0 || phoneId >= mServiceSubs.length) {
            throw new IllegalArgumentException("Invalid phoneId.");
        }

        if (getActiveServiceSub() != null) {
            getActiveServiceSub().removeListener(mLowBatteryHandler);
        }

        setActivePhoneId(phoneId);
        mLowBatteryHandler.onActiveSubChanged(getActiveServiceSub());
        getActiveServiceSub().addListener(mLowBatteryHandler);
    }

    private ImsVideoGlobals(ImsServiceSub[] serviceSubs, Context context) {
        mServiceSubs = serviceSubs;
        // Set the default active phone to 0
        setActivePhoneId(0);

        mContext = context;
        CameraController.init(context, ImsMedia.getInstance());
        MediaController.init(context, ImsMedia.getInstance());
        LowBatteryHandler.init(getActiveServiceSub(), context);

        mCameraController = CameraController.getInstance();
        mMediaController = MediaController.getInstance();
        mLowBatteryHandler = LowBatteryHandler.getInstance();
        for (ImsServiceSub sub: mServiceSubs) {
            sub.addListener(mMediaController);
        }
    }

    private ImsServiceSub getActiveServiceSub() {
        return mServiceSubs[mActivePhoneId];
    }

    private void setActivePhoneId(int phoneId) {
        mActivePhoneId = phoneId;
    }

    /* package */
    ImsCallSessionImpl getActiveCallSession() {
        List<ImsCallSessionImpl> sessionList =
                getActiveServiceSub().getCallSessionByState(DriverCallIms.State.ACTIVE);
        if (sessionList.size() > 1) log("Multiple Active Calls: " + sessionList);
        return sessionList.isEmpty() ? null : sessionList.get(0);
    }

    /* package */
    ImsCallSessionImpl getOutgoingCallSession() {
        List<ImsCallSessionImpl> sessionList =
                getActiveServiceSub().getCallSessionByState(DriverCallIms.State.ALERTING);
        if (sessionList.isEmpty()) {
            sessionList = getActiveServiceSub().getCallSessionByState(DriverCallIms.State.DIALING);
        }

        if (sessionList.size() > 1) log("Multiple Outgoing Calls: " + sessionList);
        return sessionList.isEmpty() ? null : sessionList.get(0);
    }

    /* package */
    ImsCallSessionImpl getIncomingCallSession() {
        List<ImsCallSessionImpl> sessionList =
                getActiveServiceSub().getCallSessionByState(DriverCallIms.State.INCOMING);
        if (sessionList.size() > 1) log("Multiple Incoming Calls: " + sessionList);
        return sessionList.isEmpty() ? null: sessionList.get(0);
    }

    /* package */
    ImsCallSessionImpl getBackgroundCallSession() {
        List<ImsCallSessionImpl> sessionList =
                getActiveServiceSub().getCallSessionByState(DriverCallIms.State.HOLDING);
        if (sessionList.size() > 1) log("Multiple Background Calls: " + sessionList);
        return sessionList.isEmpty() ? null: sessionList.get(0);
    }

    /* package */
    ImsCallSessionImpl getActiveOrOutgoingCallSession() {
        ImsCallSessionImpl session = getActiveCallSession();
        if (session==null) {
            session = getOutgoingCallSession();
        }
        return session;
    }

    /* package */
    ImsCallSessionImpl getActiveOrBackgroundCallSession() {
        ImsCallSessionImpl session = getActiveCallSession();
        if (session == null) {
            session = getBackgroundCallSession();
        }
        return session;
    }

    private ImsVideoCallProviderImpl getImsVideoCallProviderImpl(ImsCallSessionImpl session) {
        return session == null ? null : session.getImsVideoCallProviderImpl();
    }

    /* package */
    ImsVideoCallProviderImpl getActiveCallVideoProvider() {
        return getImsVideoCallProviderImpl(getActiveCallSession());
    }

    /* package */
    ImsVideoCallProviderImpl getOutgoingCallVideoProvider() {
        return getImsVideoCallProviderImpl(getOutgoingCallSession());
    }

    /* package */
    ImsVideoCallProviderImpl getIncomingCallVideoProvider() {
        return getImsVideoCallProviderImpl(getIncomingCallSession());
    }

    /* package */
    ImsVideoCallProviderImpl getBackgroundCallVideoProvider() {
        return getImsVideoCallProviderImpl(getBackgroundCallSession());
    }

    /* package */
    ImsVideoCallProviderImpl getActiveOrOutgoingCallVideoProvider() {
        return getImsVideoCallProviderImpl(getActiveOrOutgoingCallSession());
    }

    /* package */
    ImsVideoCallProviderImpl getActiveOrBackgroundCallVideoProvider() {
        return getImsVideoCallProviderImpl(getActiveOrBackgroundCallSession());
    }

    /* package */
    ImsCallSessionImpl findSessionbyMediaId(int mediaId) {
        return getActiveServiceSub().findSessionByMediaId(mediaId);
    }

    /* package */
    ImsVideoCallProviderImpl findVideoCallProviderbyMediaId(int mediaId) {
        return getImsVideoCallProviderImpl(getActiveServiceSub().findSessionByMediaId(mediaId));
    }

    /* package */
    CameraController getCameraController() { return mCameraController; }

    /* package */
    MediaController getMediaController() { return mMediaController; }

    private static void log(String msg) {
        Log.d(TAG, msg);
    }
    private static void loge(String msg) {
        Log.e(TAG, msg);
    }
}
