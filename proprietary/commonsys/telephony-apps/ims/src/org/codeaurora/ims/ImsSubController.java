/*
 * Copyright (c) 2015-2016, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package org.codeaurora.ims;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Looper;
import android.os.Handler;
import android.os.Message;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import com.android.internal.annotations.VisibleForTesting;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.TelephonyIntents;
import com.qualcomm.ims.utils.Log;
import org.codeaurora.telephony.utils.AsyncResult;
import org.codeaurora.telephony.utils.Preconditions;
import java.lang.Boolean;
import java.util.Arrays;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.List;

/**
 * The responsibility of this class to is to control which subscription
 * can support IMS for 7+1, 7+5 and 7+7 Reduced scope architectures.
 * The inputs to this class are
 *    1) RAF - 7 + 1 mode
 *    2) DDS - 7 + 5 mode
 *    3) PolicyManager Decision - 7 + 7 mode Reduced scope
 * The outputs from this class are
 *    1) Which subsciption(s) IMS is enabled on
 */
public class ImsSubController implements ImsSenderRxr.ImsRadioServiceListener {
    private final Context mContext;
    private List<ImsStateListener> mListeners = new CopyOnWriteArrayList<>();
    private List<ImsStackConfigListener> mStackConfigListeners = new CopyOnWriteArrayList<>();
    private Handler mHandler;
    private static final int EVENT_SUB_CONFIG_CHANGED = 1;
    private static final int EVENT_GET_SUB_CONFIG = 2;
    private static final int EVENT_IMS_SERVICE_UP = 3;
    private static final int EVENT_IMS_SERVICE_DOWN = 4;
    private static final int EVENT_RADIO_AVAILABLE = 5;
    private static final int EVENT_RADIO_NOT_AVAILABLE = 6;

    private static final int INVALID_PHONE_ID = -1;
    private static final int DEFAULT_PHONE_ID = 0;
    private int mNumMultiModeStacks = 0;
    private boolean mIsReceiverRegistered = false;
    private ImsSenderRxr[] mSenderRxrs;
    private static final String ACTION_GET_RADIO_ACCESS_FAMILY_CHANGED
            = "org.codeaurora.intent.action.ACTION_RADIO_CAPABILITY_UPDATED";
    private static final String ACTION_DDS_SWITCH_DONE
            = "org.codeaurora.intent.action.ACTION_DDS_SWITCH_DONE";
    private static int mSimultStackCount = 0;
    private static List<Boolean> mStackStatus;
    // Modem provides stack status for 6 slots.
    // NOTE: Currently, we expect only 2 stacks to be active.
    private final int MAX_VALID_STACK_STATUS_COUNT = 6;
    private boolean mActiveStacks[] = new boolean[MAX_VALID_STACK_STATUS_COUNT];
    private SubscriptionManager mSubscriptionManager = null;
    /*
     * DSDV (Dual Sim Dual Volte) i.e. 7+7
     * TRUE -- Dsdv is supported
     * FALSE -- Dsdv is not supported and can be inferred when
     *          lower layers inform that sub config request is
     *          not supported via ImsErrorCode.REQUEST_NOT_SUPPORTED
     */
    private boolean mIsDsdv = true;
    private TelephonyManager mTm = null;

    public interface ImsStateListener {
        public void onActivateIms(int phoneId);
        public void onDeactivateIms(int phoneId);
    }

    public interface ImsStackConfigListener {
        /**
         * Method that reports the active/inactive status of each
         * IMS-capable stack.
         * @param activeStacks Array corresponding to IMS stacks (subscriptions).
         *        True and False values correspond to active and inactive respectively.
         * @param phoneId the serviceSub instance id that needs to act on this update
         */
        public void onStackConfigChanged(boolean[] activeStacks, int phoneId);
    }

    /**
     * Registers a stackConfigListener.
     * @param stackConfigListener Listener to be registered.
     * @param phoneId the serviceSub instance id that is registering for stack config updates
     * @see ImsSubController#ImsStackConfigListener
     * @throws IllegalArgumentException Will throw an error if stackConfigListener is null.
     */
    public void registerListener(ImsStackConfigListener stackConfigListener, int phoneId) {
        if (stackConfigListener == null) {
            throw new IllegalArgumentException("stackConfigListener is null!");
        }
        if (!mStackConfigListeners.contains(stackConfigListener)) {
            mStackConfigListeners.add(stackConfigListener);
        } else {
            Log.w(this, "registerListener :: duplicate stackConfigListener!");
        }
        synchronized(this.getClass()) {
            notifyStackConfigChanged(mActiveStacks, phoneId);
        }
    }

    /**
     * Unregisters a stackConfigListener.
     * @param stackConfigListener Listener to unregister
     * @see ImsSubController#ImsStackConfigListener
     * @throws IllegalArgumentException Will throw an error if listener is null.
     * @return true of listener is removed, false otherwise.
     */
    public boolean unregisterListener(ImsStackConfigListener stackConfigListener) {
        if (stackConfigListener == null) {
            throw new IllegalArgumentException("stackConfigListener is null");
        }
        return mStackConfigListeners.remove(stackConfigListener);
    }

    public ImsSubController(Context context, ImsSenderRxr[] senderRxrs) {
        this(context, senderRxrs, Looper.myLooper());
    }

    public ImsSubController(Context context, ImsSenderRxr[] senderRxrs, Looper looper) {
        mContext = context;
        mTm = (TelephonyManager) mContext.getSystemService(Context.TELEPHONY_SERVICE);
        mSenderRxrs = senderRxrs;
        mHandler = new ImsSubControllerHandler(looper);
        for (int i = 0; i < senderRxrs.length; i++) {
            senderRxrs[i].registerForAvailable(mHandler, EVENT_RADIO_AVAILABLE, i);
            senderRxrs[i].registerForNotAvailable(mHandler, EVENT_RADIO_NOT_AVAILABLE, i);
            senderRxrs[i].registerListener(this);
        }
        mSubscriptionManager = (SubscriptionManager) mContext.getSystemService(
                Context.TELEPHONY_SUBSCRIPTION_SERVICE);
    }

    @VisibleForTesting
    public void setIsDsdv(boolean isDsdv) {
        mIsDsdv = isDsdv;
    }

    @VisibleForTesting
    public Handler getHandler() {
        return mHandler;
    }

    @Override
    public void onServiceUp(int phoneId) {
        Log.i(this, "onServiceUp :: phoneId=" + phoneId);
        if (mTm.getPhoneCount() > 1) {
            Message msg = mHandler.obtainMessage(EVENT_IMS_SERVICE_UP);
            msg.arg1 = phoneId;
            msg.sendToTarget();
        } else {
            Log.i(this, "Single SIM mode");
            initSubscriptionStatus();
        }
    }

    @Override
    public void onServiceDown(int phoneId) {
        Log.i(this, "onServiceDown :: phoneId=" + phoneId);
        Message msg = mHandler.obtainMessage(EVENT_IMS_SERVICE_DOWN);
        msg.arg1 = phoneId;
        msg.sendToTarget();
    }

    /**
     * Registers listener.
     * @param listener Listener to be registered
     * @see ImsSubController#Listener
     * @throws IllegalArgumentException Will throw an error if listener is null.
     */
    public void registerListener(ImsStateListener listener) {
        if (listener == null) {
            throw new IllegalArgumentException("listener is null");
        }
        if (!mListeners.contains(listener)) {
            mListeners.add(listener);
        } else {
            Log.w(this, "Duplicate listener " + listener);
        }
    }

    /**
     * Unregisters listener.
     * @param listener Listener to unregister
     * @see ImsSubContrller#Listener
     * @throws IllegalArgumentException Will throw an error if listener is null.
     * @return true of listener is removed, false otherwise.
     */
    public boolean unregisterListener(ImsStateListener listener) {
        if (listener == null) {
            throw new IllegalArgumentException("listener is null");
        }
        return mListeners.remove(listener);
    }

    public static int getDefaultPhoneId() {
        return DEFAULT_PHONE_ID;
    }

    public boolean isDsdv() {
        return mIsDsdv;
    }

    private void notifyStackConfigChanged(boolean[] activeStacks, int phoneId) {
        Log.v(this, "notifyStackConfigChanged: activeStacks:" + Arrays.toString(activeStacks)
                + " phoneId: " + phoneId);
        for (ImsStackConfigListener listener : mStackConfigListeners) {
            listener.onStackConfigChanged(activeStacks, phoneId);
        }
    }

    private void handleSubConfigException(Throwable exception) {
        Preconditions.checkArgument(exception != null);
        final int errorCode = ((ImsRilException)exception).getErrorCode();
        Log.i(this, "handleSubConfigException error : " + errorCode);
        if (errorCode == ImsErrorCode.REQUEST_NOT_SUPPORTED) {
            mIsDsdv = false;
        // This error means an older modem that does not support the request is
        // being paired with HLOS that has L+L feature. In this case fallback
        // to legacy behavior.
            initSubscriptionStatus();
        } else {
            Log.w (this, "Unhandled error code : " + errorCode);
        }
    }

    private void handleSubConfigChanged(AsyncResult ar) {
        if (ar.exception != null) {
            handleSubConfigException(ar.exception);
        } else if(ar.result != null) {
            ImsSubConfigDetails config = (ImsSubConfigDetails) ar.result;
            mSimultStackCount = config.getSimultStackCount();
            mStackStatus = config.getImsStackEnabledList();
            boolean[] activeStacks = new boolean[MAX_VALID_STACK_STATUS_COUNT];

            for(int i = 0; i < mSimultStackCount; ++i) {
                activeStacks[i] = mStackStatus.get(i);
            }

            if (ar.userObj == null) {
                Log.e(this, "handleSubConfigChanged ar.userObj is null");
                return;
            }

            notifyStackConfigChanged(activeStacks, (int)ar.userObj);
        } else {
            Log.e(this, "ar.result and ar.exception are null");
        }
    }

    private class ImsSubControllerHandler extends Handler {
        public ImsSubControllerHandler() {
            super();
        }

        public ImsSubControllerHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Log.i(this, "Message received: what = " + msg.what);
            AsyncResult ar;
            int phoneId = INVALID_PHONE_ID;
            try {
                switch (msg.what) {
                    case EVENT_SUB_CONFIG_CHANGED:
                    case EVENT_GET_SUB_CONFIG:
                        ar = (AsyncResult)msg.obj;
                        phoneId = (int)ar.userObj;
                        Log.i(this, "Received SubConfig Event phoneId = " + phoneId);
                        handleSubConfigChanged(ar);
                        break;
                    case EVENT_IMS_SERVICE_UP:
                        phoneId = (int) msg.arg1;
                        Log.i(this, "Received EVENT_IMS_SERVICE_UP phoneId = " + phoneId);
                        mSenderRxrs[phoneId].registerForImsSubConfigChanged(
                                mHandler, EVENT_SUB_CONFIG_CHANGED, phoneId);
                        if (mSenderRxrs[phoneId].getRadioState().isAvailable()) {
                            mSenderRxrs[phoneId].getImsSubConfig(
                                    mHandler.obtainMessage(EVENT_GET_SUB_CONFIG, phoneId));
                        } else {
                            Log.w(this, "radio not available");
                        }
                        break;
                    case EVENT_IMS_SERVICE_DOWN:
                        phoneId = (int) msg.arg1;
                        Log.i(this, "Received EVENT_IMS_SERVICE_DOWN phoneId = " + phoneId);
                        mSenderRxrs[phoneId].deregisterForImsSubConfigChanged(mHandler);
                        updateStackConfig(phoneId, false);
                        break;

                    case EVENT_RADIO_NOT_AVAILABLE:
                        ar = (AsyncResult)msg.obj;
                        phoneId = (int)ar.userObj;
                        Log.i(this, "Received EVENT_RADIO_NOT_AVAILABLE phoneId = " + phoneId);
                        updateStackConfig(phoneId, false);
                        break;

                    case EVENT_RADIO_AVAILABLE:
                        ar = (AsyncResult)msg.obj;
                        phoneId = (int)ar.userObj;
                        Log.i(this, "Received EVENT_RADIO_AVAILABLE phoneId = " + phoneId);
                        mSenderRxrs[phoneId].getImsSubConfig(
                                mHandler.obtainMessage(EVENT_GET_SUB_CONFIG, phoneId));
                        break;
                    default:
                        Log.w(this, "Unknown message = " + msg.what);
                        break;
                }
            } catch (ArrayIndexOutOfBoundsException exc) {
                Log.e(this, "handleMessage :: Inavlid phoneId " + phoneId);
            }
        }
    }

    private void updateStackConfig(int phoneId, boolean isEnabled) {
        boolean[] activeStacks;
        Log.v(this, "updateStackConfig phoneId: " + phoneId + " isEnabled: " + isEnabled
                 + " mIsDsdv : " + mIsDsdv);

        if (mIsDsdv) {
            if (mStackStatus == null) {
                Log.w(this, "updateStackConfig Stacks are not yet initialized");
                return;
            }

            if (mStackStatus.get(phoneId) == isEnabled) {
                Log.w(this, "updateStackConfig nothing to update");
                return;
            }

            activeStacks = new boolean[MAX_VALID_STACK_STATUS_COUNT];
            mStackStatus.set(phoneId, isEnabled);
            for(int i = 0; i < mSimultStackCount; ++i) {
                activeStacks[i] = mStackStatus.get(i);
            }
        } else {

            /* Unregister the earlier receiver, if any */
            if (mIsReceiverRegistered) {
                Log.v(this, "updateStackConfig: unregistering BroadcastReceiver");
                mContext.unregisterReceiver(mBroadcastReceiver);
                mIsReceiverRegistered = false;
            }

            if (mActiveStacks[phoneId] == isEnabled) {
                Log.w(this, "updateStackConfig nothing to update");
                return;
            }

            if (!isEnabled) {
                /*
                 * When we notify that a particular stack is not enabled (for eg. due to modem
                 * ssr, rild crash etc.), feature state for that stack will be set to UNAVAILABLE.
                 * When lower layers again inform that the stack is enabled, feature state for
                 * stack needs to be set to AVAILABLE. In 7+1/7+5 cases, this is not happening.
                 * Reset <mNumMultiModeStacks> to ensure that same sequence of events that happen
                 * on normal boot-up happens in modem ssr/rild crash etc. use-cases.
                 */
                mNumMultiModeStacks = 0;
            }

            mActiveStacks[phoneId] = isEnabled;
            activeStacks = mActiveStacks;
        }
        notifyStackConfigChanged(activeStacks, phoneId);
    }

    /* Method to initialize the Phone Id */
    private void initSubscriptionStatus() {
        mSimultStackCount = 0;
        mStackStatus = null;
        // TODO: Check how Single-SIM/backward compatible multi-SIM scenarios are handled.
        if (mTm.getPhoneCount() > 1) {
            Log.i(this, "initSubscriptionStatus: [Multi-sim] Using RAF and DDS to decide IMS Sub");
            handleRafInfoChange();

            /* Check if Radio Access Family event change is to be registered */
            if (mNumMultiModeStacks == 0) {
                Log.i(this, "initSubscriptionStatus: registered for RAF info");

                /* Start listening to the RAF change event. */
                mContext.registerReceiver(mBroadcastReceiver,
                        new IntentFilter(ACTION_GET_RADIO_ACCESS_FAMILY_CHANGED));
                mIsReceiverRegistered = true;
            }
        } else {
            /* If not multi-sim, a change in socket communication is not required */
            Log.i(this, "initSubscriptionStatus: Not multi-sim.");
            updateActiveImsStackForPhoneId(DEFAULT_PHONE_ID);
        }
    }

    /* Method to re-initialize the Ims Phone instances.
       This API will only be used for single IMS stack. */
    public void updateActiveImsStackForPhoneId(int phoneId) {
        if (phoneId == SubscriptionManager.INVALID_SIM_SLOT_INDEX) {
            Log.e(this, "switchImsPhone: Invalid phoneId: " + phoneId);
            return;
        }

        synchronized(this.getClass()) {
            for (int i = 0; i < mActiveStacks.length; ++i) {
                if (i == phoneId) {
                    mActiveStacks[i] = true;
                } else {
                    mActiveStacks[i] = false;
                }
            }

            notifyStackConfigChanged(mActiveStacks, phoneId);
        }
    }

    /* Method to validate a subId and then check for IMSPhonechange.
       This is only used for single IMS stack. */
    private void updateActiveImsStackForSubId(int ddsSubId) {
        int phoneId = 0; /* By default, always assume dds as default phone ID */

        /*
         * If the subscription ID received via intent is valid,
         * then retrieve the corresponding phone ID.
         */
        if (SubscriptionManager.isValidSubscriptionId(ddsSubId)) {
            phoneId = SubscriptionManager.getSlotIndex(ddsSubId);
        } else {
            /* Recheck the subId via API */
            ddsSubId = SubscriptionManager.getDefaultDataSubscriptionId();
            /* If the subscription ID is Valid, then retrive corresponding phone ID */
            if (SubscriptionManager.isValidSubscriptionId(ddsSubId)) {
                phoneId = SubscriptionManager.getSlotIndex(ddsSubId);
            }
        }

        Log.i(this, "updateActiveImsStackForSubId: new DDS = " + ddsSubId);
        updateActiveImsStackForPhoneId(phoneId);
    }

    /**
     * Method to identify the RAT mask for Multimode support.
     * Checks for LTE, to support IMS.
     */
    private boolean isMultiModeSupported(long nRatMask) {
        return (nRatMask & TelephonyManager.NETWORK_TYPE_BITMASK_LTE) != 0;
    }

    /* Method to handle Radio Acess Family change event */
    private void handleRafInfoChange() {
        /* If registered for DDS change event, ignore checking for RAF change event */
        if (mNumMultiModeStacks > 0) {
            Log.i(this, "handleRafInfoChange: " + mNumMultiModeStacks +
                    "Multimode stacks. Handled by DDS switch. EXIT!!!");
            return;
        }

        /* Check for the number of multi-mode stacks */
        final int numPhones = mTm.getPhoneCount();
        int tempPhoneId = 0; /* Always assume default phone ID as 0 */
        for (int i = 0; i < numPhones; i++) {
            final SubscriptionInfo subInfo =
                    mSubscriptionManager.getActiveSubscriptionInfoForSimSlotIndex(i);
            if (subInfo != null) {
                final int subId = subInfo.getSubscriptionId();
                long rafInfo = mTm.createForSubscriptionId(subId).getSupportedRadioAccessFamily();
                Log.i(this, "handleRafInfoChange: Phone: " + i + " Info:" + rafInfo);
                if (isMultiModeSupported(rafInfo)) {
                    mNumMultiModeStacks++;
                    tempPhoneId = i;
                }
            } else {
                Log.e(this, "handleRafInfoChange: subIds not valid");
            }
        }
        Log.i(this, "handleRafInfoChange: NumPhones: " + numPhones +
                "Multiple Multimode stacks: " + mNumMultiModeStacks);

        /*
         * If more than 1 Multimode stack is present, then listen to DDS change event
         * else, continue to listen for Radio Access Family change event.
         */
        if (mNumMultiModeStacks > 1) {

            /* Unregister the earlier receiver, if any */
            if (mIsReceiverRegistered) {
                mContext.unregisterReceiver(mBroadcastReceiver);
                mIsReceiverRegistered = false;
            }

            /*
             * To avoid the scenarios where DDS change event is not broadcasted yet,
             * by default check the data subscription information once.
             */
            updateActiveImsStackForSubId(SubscriptionManager.INVALID_SUBSCRIPTION_ID);

            /* Start listening to the DDS change event. */
            mContext.registerReceiver(mBroadcastReceiver,
                    new IntentFilter(ACTION_DDS_SWITCH_DONE));
            mIsReceiverRegistered = true;
            Log.i(this, "handleRafInfoChange: registered for DDS switch...");

        } else {
            mNumMultiModeStacks = 0;
            updateActiveImsStackForPhoneId(tempPhoneId);
        }
    }

    /* Receiver to handle RAF and DDS change event */
    private BroadcastReceiver mBroadcastReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            Log.i(this, "mBroadcastReceiver - " + action);
            if (action.equals(ACTION_DDS_SWITCH_DONE)) {
                int ddsSubId = intent.getIntExtra(PhoneConstants.SUBSCRIPTION_KEY,
                        SubscriptionManager.INVALID_SUBSCRIPTION_ID);
                Log.i(this, "got ACTION_DDS_SWITCH_DONE, new DDS = "
                        + ddsSubId);
                updateActiveImsStackForSubId(ddsSubId);
            } else if (action.equals(ACTION_GET_RADIO_ACCESS_FAMILY_CHANGED)) {
                handleRafInfoChange();
            }
        }
    };
}
