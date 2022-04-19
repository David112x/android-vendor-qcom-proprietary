/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.networksetting;

import android.app.AlertDialog;
import android.content.Context;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.telephony.PhoneStateListener;
import android.telephony.ServiceState;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.SubscriptionManager.OnSubscriptionsChangedListener;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.util.SparseArray;
import android.view.WindowManager;

import java.util.ArrayList;
import java.util.List;
import java.util.stream.Collectors;

public class RoamingAlerter extends Handler{
    private static String TAG = RoamingAlerter.class.getSimpleName();
    private static final int EVENT_ROAMING_ALERTING_ON_DDS_SUB = 1;

    private SubscriptionManager mSubscriptionManager;
    private HandlerThread mHandlerThread;
    private Context mContext;

    private OnSubscriptionsChangedListener mOnSubscriptionsChangedListener =
            new OnSubscriptionsChangedListener() {
        private ArrayList<RoamingStatusListener> cacheSubListener = new ArrayList<>();

        @Override
        public void onSubscriptionsChanged() {
            if (mSubscriptionManager != null) {
                List<SubscriptionInfo> subList =
                        mSubscriptionManager.getActiveSubscriptionInfoList();
                if (subList != null) {
                    // unavaiable
                    List<RoamingStatusListener> unavaiables = null;
                    for (SubscriptionInfo subInfo : subList) {
                        unavaiables = cacheSubListener.stream()
                                .filter(RoamingStatusListener ->
                                        subInfo.getSubscriptionId()
                                        != RoamingStatusListener.getSubId())
                                .collect(Collectors.toList());
                    }
                    if (unavaiables != null) {
                        for (RoamingStatusListener listener : unavaiables) {
                            listener.unregister();
                        }
                        cacheSubListener.removeAll(unavaiables);
                    }
                    // avaiable
                    SparseArray<RoamingStatusListener> avaiableListeners = new SparseArray<>();
                    for (RoamingStatusListener listener : cacheSubListener) {
                        Log.d(TAG, "previously, avaiable list " + listener);
                        avaiableListeners.put(listener.getSubId(), listener);
                    }
                    for (SubscriptionInfo subInfo : subList) {
                        if (avaiableListeners.indexOfKey(subInfo.getSubscriptionId()) < 0) {
                            RoamingStatusListener listener = new RoamingStatusListener(
                                    RoamingAlerter.this.getBackgroundLooper(),
                                    mContext,
                                    subInfo.getSubscriptionId(),
                                    RoamingAlerter.this
                                            .obtainMessage(EVENT_ROAMING_ALERTING_ON_DDS_SUB));
                            listener.register();
                            cacheSubListener.add(listener);
                        }
                    }
                } else {
                    for (RoamingStatusListener listener : cacheSubListener) {
                        listener.unregister();
                    }
                    cacheSubListener.clear();
                }
            }
        }
    };

    public RoamingAlerter(Context context) {
        mContext = context;
        mSubscriptionManager = context.getSystemService(SubscriptionManager.class);
        mHandlerThread = new HandlerThread(TAG + "_background");
    }

    public void monitor() {
        if (mHandlerThread != null) mHandlerThread.start();
        if (mSubscriptionManager != null) mSubscriptionManager
                .addOnSubscriptionsChangedListener(mOnSubscriptionsChangedListener);
    }

    public void unmonitor() {
        if (mSubscriptionManager != null) mSubscriptionManager
                .removeOnSubscriptionsChangedListener(mOnSubscriptionsChangedListener);
        if (mHandlerThread != null) mHandlerThread.quitSafely();
    }

    private Looper getBackgroundLooper () {
        return mHandlerThread != null ? mHandlerThread.getLooper() : Looper.myLooper();
    }

    @Override
    public void handleMessage(Message msg) {
        switch (msg.what) {
            case EVENT_ROAMING_ALERTING_ON_DDS_SUB:
                Log.d(TAG, "Roaming indication on default data SUB");
                showRoamingAlertingDialog();
                break;
            default:
                Log.d(TAG, "Unexpected event");
                break;
        }
    }

    private void showRoamingAlertingDialog() {
        Log.d(TAG, "showRoamingAlertingDialog...");
        AlertDialog.Builder builder = new AlertDialog.Builder(mContext);
        AlertDialog dialog = builder.setMessage(R.string.roaming_alerting_message)
                .setNegativeButton(android.R.string.ok, null)
                .setCancelable(false)
                .create();
        dialog.getWindow().setType(WindowManager.LayoutParams.TYPE_SYSTEM_DIALOG);
        dialog.show();
    }

    private static final class RoamingStatusListener extends PhoneStateListener {
        private boolean mIsRoaming = false;
        private final Message mRoamingInd;
        private TelephonyManager mTm;
        private boolean mIsInformed = false;

        public RoamingStatusListener(Looper looper, Context context, int subId, Message msg) {
            super(looper);
            mSubId = subId;
            mRoamingInd = msg;
            mTm = TelephonyManager.from(context).createForSubscriptionId(subId);
        }

        public int getSubId() {
            return mSubId;
        }

        public void register() {
            if (mTm != null) {
                mTm.listen(this, PhoneStateListener.LISTEN_SERVICE_STATE
                        | PhoneStateListener.LISTEN_ACTIVE_DATA_SUBSCRIPTION_ID_CHANGE);
            }
        }

        public void unregister() {
            if (mTm != null) mTm.listen(this, PhoneStateListener.LISTEN_NONE);
        }

        @Override
        public void onServiceStateChanged(ServiceState serviceState) {
            if (mIsRoaming != serviceState.getRoaming()) {
                mIsRoaming = serviceState.getRoaming();
                mIsInformed = false; // reset, so that informs again after becoming roaming.
                if (mSubId == SubscriptionManager.getDefaultDataSubscriptionId()) {
                    informRoamingAlertingIfRequired();
                }
            }
        }

        @Override
        public void onActiveDataSubscriptionIdChanged(int subId) {
            if (mSubId == subId) {
                informRoamingAlertingIfRequired();
            } else {
                mIsInformed = false; // next time, will inform again.
            }
        }

        private void informRoamingAlertingIfRequired() {
            if (mRoamingInd != null
                    && !mIsInformed
                    && mIsRoaming
                    && mTm != null
                    && mTm.isDataEnabled()
                    && mTm.isDataRoamingEnabled()) {
                mIsInformed = true;
                mRoamingInd.sendToTarget();
            }
        }

        @Override
        public String toString() {
            return "RoamingStatusListener sub: " + mSubId + " roaming:" + mIsRoaming;
        }
    }
}
