/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.internal.telephony;

import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.os.Registrant;
import android.os.RegistrantList;
import android.telephony.Rlog;

import android.telephony.PreciseCallState;
import android.telephony.TelephonyManager;
import com.android.internal.telephony.GsmCdmaCall;
import com.android.internal.telephony.imsphone.ImsPhone;
import com.android.internal.telephony.imsphone.ImsPhoneCall;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneConstants;

/*
 * This class will register for precise call state changes and notifies call
 * start/call end to its registered clients
 */
public class QtiCallStateNotifier {
    private final String LOG_TAG = "QtiCallStateNotifier";
    private final int mNumPhones;
    private Phone[] mPhones;
    private GsmCdmaCall[] mFgCsCalls;
    private GsmCdmaCall[] mBgCsCalls;
    private GsmCdmaCall[] mRiCsCalls;
    private ImsPhone[] mImsPhones;
    private ImsPhoneCall[] mFgImsCalls;
    private ImsPhoneCall[] mBgImsCalls;
    private ImsPhoneCall[] mRiImsCalls;

    private boolean mIsCallInActiveState = false;
    private boolean mIsCallInProgress = false;

    private RegistrantList mCallEndRegistrants = new RegistrantList();

    /* Integer constants for event handling */
    private final int EVENT_PRECISE_CS_CALL_STATE_CHANGED = 101;
    private final int EVENT_PRECISE_IMS_CALL_STATE_CHANGED = 102;

    private CallStateHandler mCallStateHandler;

    /** Singleton instance. */
    protected static QtiCallStateNotifier mInstance = null;

    public static void init(Phone[] phones) {
        synchronized (QtiCallStateNotifier.class) {
            if (mInstance == null) {
                mInstance = new QtiCallStateNotifier(phones);
            }
        }
    }

    public static QtiCallStateNotifier getInstance() {
        synchronized (QtiCallStateNotifier.class) {
            if (mInstance == null) {
                throw new RuntimeException("QtiCallStateNotifier is not initialized!");
            }
            return mInstance;
        }
    }

    private QtiCallStateNotifier(Phone[] phones) {
        mNumPhones = TelephonyManager.getDefault().getPhoneCount();
        mPhones = phones;

        mImsPhones = new ImsPhone[mNumPhones];
        mFgCsCalls = new GsmCdmaCall[mNumPhones];
        mBgCsCalls = new GsmCdmaCall[mNumPhones];
        mRiCsCalls = new GsmCdmaCall[mNumPhones];
        mFgImsCalls = new ImsPhoneCall[mNumPhones];
        mBgImsCalls = new ImsPhoneCall[mNumPhones];
        mRiImsCalls = new ImsPhoneCall[mNumPhones];

        mCallStateHandler = new CallStateHandler();

        // Assign phones, and register for call state change
        for (int i=0; i < mNumPhones; i++) {
            if (mPhones[i] != null) {
                mPhones[i].registerForPreciseCallStateChanged(mCallStateHandler,
                            EVENT_PRECISE_CS_CALL_STATE_CHANGED, i);
                mFgCsCalls[i] = (GsmCdmaCall) mPhones[i].getForegroundCall();
                mBgCsCalls[i] = (GsmCdmaCall) mPhones[i].getBackgroundCall();
                mRiCsCalls[i] = (GsmCdmaCall) mPhones[i].getRingingCall();
            }

            mImsPhones[i] = (ImsPhone)mPhones[i].getImsPhone();
            if (mImsPhones[i] != null) {
                mImsPhones[i].registerForPreciseCallStateChanged(mCallStateHandler,
                            EVENT_PRECISE_IMS_CALL_STATE_CHANGED, i);
                mFgImsCalls[i] = mImsPhones[i].getForegroundCall();
                mBgImsCalls[i] = mImsPhones[i].getBackgroundCall();
                mRiImsCalls[i] = mImsPhones[i].getRingingCall();
            }
        }
    }

    /*
     * Returns true when call is in active state
     */
    public boolean isCallActive() {
        return mIsCallInActiveState;
    }

    /*
     * Returns true when call is started
     */
    public boolean isCallInProgress() {
         return mIsCallInProgress;
    }

    /*
     * Registrant to notify call end
     */
    public void registerForCallEnd(Handler h, int what, Object o) {
        mCallEndRegistrants.addUnique(h, what, o);
    }

    public void unregisterForCallEnd(Handler h) {
        mCallEndRegistrants.remove(h);
    }

    /*
     * Method to check if any of the calls are active
     */
    private boolean isCallActive(int phoneId) {
        return ((mFgCsCalls[phoneId] != null &&
                mFgCsCalls[phoneId].getState() == GsmCdmaCall.State.ACTIVE) ||
                (mBgCsCalls[phoneId] != null &&
                mBgCsCalls[phoneId].getState() == GsmCdmaCall.State.ACTIVE) ||
                (mRiCsCalls[phoneId] != null &&
                mRiCsCalls[phoneId].getState() == GsmCdmaCall.State.ACTIVE) ||
                (mFgImsCalls[phoneId] != null &&
                mFgImsCalls[phoneId].getState() == ImsPhoneCall.State.ACTIVE) ||
                (mBgImsCalls[phoneId] != null &&
                mBgImsCalls[phoneId].getState() == ImsPhoneCall.State.ACTIVE) ||
                (mRiImsCalls[phoneId] != null &&
                mRiImsCalls[phoneId].getState() == ImsPhoneCall.State.ACTIVE));
    }

    /*
     * Method to check if all the calls are ended
     */
    private boolean isCallIdle(int phoneId) {
        return ((mFgCsCalls[phoneId] == null || mFgCsCalls[phoneId].isIdle()) &&
                (mBgCsCalls[phoneId] == null || mBgCsCalls[phoneId].isIdle()) &&
                (mRiCsCalls[phoneId] == null || mRiCsCalls[phoneId].isIdle()) &&
                (mFgImsCalls[phoneId] == null || mFgImsCalls[phoneId].isIdle()) &&
                (mBgImsCalls[phoneId] == null || mBgImsCalls[phoneId].isIdle()) &&
                (mRiImsCalls[phoneId] == null || mRiImsCalls[phoneId].isIdle()));
    }

    /*
     * Handler Class
     */
    private class CallStateHandler extends Handler {
        @Override
        public void handleMessage(Message msg) {
            AsyncResult ar = (AsyncResult)msg.obj;
            int phoneId = (int)ar.userObj;
            switch (msg.what) {
                case EVENT_PRECISE_CS_CALL_STATE_CHANGED:
                case EVENT_PRECISE_IMS_CALL_STATE_CHANGED: {
                    if (!mIsCallInActiveState && isCallActive(phoneId)) {
                        //Notify if call is in active state
                        log("processCallStateChanged: call active on phone" + phoneId);
                        mIsCallInActiveState = true;
                    } else if (isCallIdle(phoneId)) {
                        //Notify when call is ended
                        log("processCallStateChanged: call disconnected on phone" + phoneId);
                        mIsCallInActiveState = false;
                        mIsCallInProgress = false;
                        mCallEndRegistrants.notifyRegistrants();
                    } else {
                        if (!mIsCallInProgress) {
                            log("processCallStateChanged: call started on phone" + phoneId);
                            mIsCallInProgress = true;
                        }
                    }

                    break;
                }
            }
        }
    }

    protected void log(String l) {
        Rlog.d(LOG_TAG, l);
    }

}
