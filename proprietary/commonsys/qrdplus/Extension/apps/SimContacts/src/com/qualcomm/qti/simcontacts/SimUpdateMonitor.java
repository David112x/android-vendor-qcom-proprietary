/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
package com.qualcomm.qti.simcontacts;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.telephony.SubscriptionManager;
import android.util.Log;

import com.android.internal.telephony.IccCardConstants;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

public class SimUpdateMonitor {

    public static final int SIM_STATE_ERROR = 0;
    public static final int SIM_STATE_NOT_READY = 1;
    public static final int SIM_STATE__READY = 2;
    private static final String TAG = "SimUpdateMonitor";
    private static final String ACTION_SIM_STATE_CHANGED = "android.intent.action.SIM_STATE_CHANGED";
    private static final String ACTION_SIM_REFRESH_UPDATE = "org.codeaurora.intent.action.ACTION_SIM_REFRESH_UPDATE";
    private static final String ACTION_ADN_INIT_DONE = "qualcomm.intent.action.ACTION_ADN_INIT_DONE";
    private static SimUpdateMonitor sInstance;
    private Context mContext;
    private List<SimStateListener> mListeners = new ArrayList<>();
    private Map<Integer, SimStateData> mSimStateDatas = new HashMap<>();

    public interface SimStateListener{
        void onSimStateChanged(int subscriptionId, int state);
    }

    private BroadcastReceiver mSimStateReceiver = new BroadcastReceiver() {

        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (Log.isLoggable(TAG, Log.DEBUG)) {
                Log.d(TAG, "mSimStateReceiver action = " + action);
            }
            if (ACTION_SIM_STATE_CHANGED.equals(action)) {
                handleSimStateChange(intent);
            }
        }
    };

    public static SimUpdateMonitor getInstance(Context context) {
        if (sInstance == null) {
            sInstance = new SimUpdateMonitor(context);
        }
        return sInstance;
    }

    private SimUpdateMonitor(Context context) {
        mContext = context;

        IntentFilter intentFilter = new IntentFilter(ACTION_SIM_STATE_CHANGED);
        intentFilter.addAction(ACTION_SIM_REFRESH_UPDATE);
        intentFilter.addAction(ACTION_ADN_INIT_DONE);
        context.registerReceiver(mSimStateReceiver, intentFilter);
    }

    public void registerSimStateListener(SimStateListener listener) {
        if (!mListeners.contains(listener)) {
            mListeners.add(listener);
        }
    }

    public void unRegisterSimStateListener(SimStateListener listener) {
        if (mListeners.contains(listener)) {
            mListeners.remove(listener);
        }
    }

    private void handleSimStateChange(Intent intent) {
        SimStateData data = SimStateData.parseFromIntent(intent);
        if (!SubscriptionManager.isValidSubscriptionId(data.mSubscriptionId)) {
            Log.w(TAG, "invalid subscriptionId");
            return;
        }
        SimStateData cache = mSimStateDatas.get(data.mSubscriptionId);
        final boolean changed;
        if (cache == null) {
            mSimStateDatas.put(data.mSubscriptionId, data);
            changed = true;
        } else {
            changed = (cache.mState != data.mState
                    || cache.mSubscriptionId != data.mSubscriptionId
                    || cache.mSlotId != data.mSlotId);
            cache.mState = data.mState;
            cache.mSubscriptionId = data.mSubscriptionId;
            cache.mState = data.mSlotId;
        }
        if (changed) {
            for (SimStateListener listener : mListeners) {
                listener.onSimStateChanged(data.mSubscriptionId, data.mState);
            }
        }
        if (Log.isLoggable(TAG, Log.DEBUG)) {
            Log.d(TAG, "handleSimStateChange SimStateData = " + data + "  changed  = " + changed);
        }

    }

    private static class SimStateData {
        public int mState;
        public int mSubscriptionId;
        public int mSlotId;

        public SimStateData(int state, int subscriptionId, int slotId) {
            mState = state;
            mSubscriptionId = subscriptionId;
            mSlotId = slotId;
        }

        static SimStateData parseFromIntent(Intent intent) {
            int state = -1;
            String stateExtra = intent.getStringExtra(IccCardConstants.INTENT_KEY_ICC_STATE);
            int slotId = intent.getIntExtra(PhoneConstants.SLOT_KEY, 0);
            int subscriptionId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                    SubscriptionManager.INVALID_SUBSCRIPTION_ID);
            if (IccCardConstants.INTENT_VALUE_ICC_LOADED.equals(stateExtra)
                    || IccCardConstants.INTENT_VALUE_ICC_IMSI.equals(stateExtra)
                    || IccCardConstants.INTENT_VALUE_ICC_READY.equals(stateExtra)) {
                state = SIM_STATE__READY;
            } else if (IccCardConstants.INTENT_VALUE_ICC_ABSENT.equals(stateExtra)
                    || IccCardConstants.INTENT_VALUE_ICC_UNKNOWN.equals(stateExtra)
                    || IccCardConstants.INTENT_VALUE_ICC_CARD_IO_ERROR.equals(stateExtra)) {
                state = SIM_STATE_ERROR;
            } else {
                state = SIM_STATE_NOT_READY;
            }
            return new SimStateData(state, subscriptionId, slotId);
        }

        @Override
        public String toString() {
            return "SimStateData{state=" + mState
                    + ",subscriptionId=" + mSubscriptionId
                    + ",slotId=" + mSlotId + "}";
        }
    }

}
