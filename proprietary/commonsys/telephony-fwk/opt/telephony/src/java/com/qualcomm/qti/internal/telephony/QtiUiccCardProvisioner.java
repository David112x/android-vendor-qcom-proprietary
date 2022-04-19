/*
 * Copyright (c) 2016 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.internal.telephony;

import android.content.Context;
import android.content.Intent;
import android.os.AsyncResult;
import android.os.Handler;
import android.os.Message;
import android.os.Registrant;
import android.os.RegistrantList;
import android.telephony.Rlog;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.Log;

import com.android.internal.annotations.VisibleForTesting;
import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.PhoneConstants;
import com.android.internal.telephony.uicc.IccCardApplicationStatus.AppType;
import com.android.internal.telephony.uicc.IccCardStatus.CardState;
import com.android.internal.telephony.uicc.IccConstants;
import com.android.internal.telephony.uicc.IccFileHandler;
import com.android.internal.telephony.uicc.IccRefreshResponse;
import com.android.internal.telephony.uicc.IccUtils;
import com.android.internal.telephony.uicc.UiccCard;
import com.android.internal.telephony.uicc.UiccSlot;
import com.android.internal.telephony.uicc.UiccCardApplication;
import com.android.internal.telephony.uicc.UiccController;

import com.qualcomm.qcrilhook.QcRilHook;

import java.nio.ByteBuffer;
import java.nio.ByteOrder;
import java.util.concurrent.atomic.AtomicBoolean;

/**
 * This class has core logic to handle Uicc card Manual provision requests and stores the current
 * Uicc provision information.
 *
 * 1. It register with UiccController for Card state change event, once card state
 *    receives as present, queries current provision preference and ICCID values from
 *    RIL and broadcast an intent to clients with current card provision state.
 *
 * 2. Whenever it receives the UNSOL Manual provision change event from QMI RIL,
 *    processes it and if there is any change in state, it broadcasts intent with latest
 *    provision state.
 *
 * 3. This exposes public APIs to perform SIM Manual provision operations.
 */
public class QtiUiccCardProvisioner extends Handler {
    private static final String LOG_TAG = "QtiUiccCardProvisioner";
    private static final boolean VDBG = false;
    private static final boolean DBG = true;

    private static final int EVENT_ICC_CHANGED = 1;
    private static final int EVENT_UNSOL_MANUAL_PROVISION_STATUS_CHANGED = 2;
    private static final int EVENT_OEM_HOOK_SERVICE_READY = 3;
    private static final int EVENT_GET_ICCID_DONE = 4;
    private static final int EVENT_RADIO_AVAILABLE = 5;
    private static final int EVENT_SIM_REFRESH = 6;

    // Manual provision activation/deactivation
    // operation results.
    private static final int SUCCESS = 0;
    private static final int GENERIC_FAILURE = -1;
    private static final int INVALID_INPUT = -2;
    private static final int REQUEST_IN_PROGRESS = -3;

    private RegistrantList mManualProvisionChangedRegistrants = new RegistrantList();

    /**
     * This intent is used to broadcast Uicc card Manual provision state change
     *  information on any slot.
     */
    private static final String ACTION_UICC_MANUAL_PROVISION_STATUS_CHANGED =
            "org.codeaurora.intent.action.ACTION_UICC_MANUAL_PROVISION_STATUS_CHANGED";
    /**
     * This Intent extra field contains the latest Manual provision state.
     */
    private static final String EXTRA_NEW_PROVISION_STATE = "newProvisionState";

    private static QtiUiccCardProvisioner sInstance;
    private Context mContext;
    private QtiRilInterface mQtiRilInterface;
    private static UiccController mUiccController = null;
    private static int mNumPhones;
    private static Object sManualProvLock = new Object();
    private static AtomicBoolean mRequestInProgress = new AtomicBoolean(false);
    private boolean[] mIsIccIdQueryPending;
    private UiccProvisionStatus[] mOldProvisionStatus;

    private UiccProvisionStatus[] mProvisionStatus;
    private String[] mSimIccId;
    private String[] mSimFullIccId;
    private CardState[] mCardState;
    private final CommandsInterface[] mCis;

    @VisibleForTesting
    public static class UiccProvisionStatus {
        public static final int PROVISIONED = 1;
        public static final int NOT_PROVISIONED = 0;
        public static final int INVALID_STATE = -1;
        public static final int CARD_NOT_PRESENT = -2;

        private int userPreference;
        private int currentState;

        UiccProvisionStatus() {
            userPreference = INVALID_STATE;
            currentState = INVALID_STATE;
        }

        boolean equals(UiccProvisionStatus provisionStatus) {
            boolean result = true;

            if((provisionStatus.getUserPreference() != getUserPreference())
                    || (provisionStatus.getCurrentState() != getCurrentState())) {
                result = false;
            }
            return result;
        }

        int getUserPreference() {
            return userPreference;
        }

        void setUserPreference(int pref) {
            userPreference = pref;
        }

        int getCurrentState() {
            return currentState;
        }

        void setCurrentState(int state) {
            currentState = state;
        }

        @Override
        public String toString() {
            return "User pref " + userPreference + " Current pref " + currentState;
        }
    }

    public static QtiUiccCardProvisioner make(Context context, CommandsInterface[] cis) {
        if (sInstance == null) {
            sInstance = new QtiUiccCardProvisioner(context, cis);
        } else {
            Log.wtf(LOG_TAG, "QtiUiccCardProvisioner.make() should be called once");
        }
        return sInstance;
    }

    public static QtiUiccCardProvisioner make(Context context) {
        if (sInstance == null) {
            sInstance = new QtiUiccCardProvisioner(context, null);
        } else {
            Log.wtf(LOG_TAG, "QtiUiccCardProvisioner.make() should be called once");
        }
        return sInstance;
    }

    @VisibleForTesting
    public void dispose() {

        logd(" disposing... ");

        mUiccController.unregisterForIccChanged(this);
        mUiccController = null;

        mQtiRilInterface.unRegisterForServiceReadyEvent(this);
        mQtiRilInterface.unRegisterForUnsol(this);
        mQtiRilInterface = null;
    }

    public static QtiUiccCardProvisioner getInstance() {
        if (sInstance == null) {
            Log.e(LOG_TAG, "QtiUiccCardProvisioner.getInstance called before make");
        }
        return sInstance;
    }

    private QtiUiccCardProvisioner(Context context, CommandsInterface[] cis) {
        mContext = context;

        mNumPhones = ((TelephonyManager)mContext.getSystemService(
                Context.TELEPHONY_SERVICE)).getPhoneCount();
        logd(" Invoking constructor, no of phones = " + mNumPhones);

        mCis = cis;
        mProvisionStatus = new UiccProvisionStatus[mNumPhones];
        mOldProvisionStatus = new UiccProvisionStatus[mNumPhones];
        mSimIccId = new String[mNumPhones];
        mSimFullIccId = new String[mNumPhones];
        mCardState = new CardState[mNumPhones];
        mIsIccIdQueryPending = new boolean[mNumPhones];
        for (int index = 0; index < mNumPhones; index++) {
            mSimIccId[index] = null;
            mSimFullIccId[index] = null;
            mProvisionStatus[index] = new UiccProvisionStatus();
            mCardState[index] =  CardState.CARDSTATE_ABSENT;
            mIsIccIdQueryPending[index] = false;
            mOldProvisionStatus[index] = new UiccProvisionStatus();

            mCis[index].registerForAvailable(this, EVENT_RADIO_AVAILABLE, index);
            mCis[index].registerForIccRefresh(this, EVENT_SIM_REFRESH, index);
        }
        mUiccController = UiccController.getInstance();
        mUiccController.registerForIccChanged(this, EVENT_ICC_CHANGED, null);

        mQtiRilInterface = QtiRilInterface.getInstance(context);
        mQtiRilInterface.registerForServiceReadyEvent(this, EVENT_OEM_HOOK_SERVICE_READY, null);
        mQtiRilInterface.registerForUnsol(this,
                EVENT_UNSOL_MANUAL_PROVISION_STATUS_CHANGED, null);
    }

    public void registerForManualProvisionChanged
            (Handler handler, int what, Object obj) {
        Registrant r = new Registrant(handler, what, obj);
        synchronized (mManualProvisionChangedRegistrants) {
            mManualProvisionChangedRegistrants.add(r);
            r.notifyRegistrant();
        }
    }

    public void unregisterForManualProvisionChanged(Handler handler) {
        synchronized (mManualProvisionChangedRegistrants) {
            mManualProvisionChangedRegistrants.remove(handler);
        }
    }

    @Override
    public void handleMessage(Message msg) {
        AsyncResult ar;
        int phoneId = -1;

        switch(msg.what) {
            case EVENT_RADIO_AVAILABLE:
                ar = (AsyncResult)msg.obj;
                if (ar != null) {
                    phoneId = (Integer) ar.userObj;
                    logd("RADIO_AVAILABLE for phone: " + phoneId);

                    if (isAllCardProvisionInfoReceived()) {
                        int[] subIds = QtiSubscriptionController.getInstance().getSubId(phoneId);
                        if (((subIds != null) && (subIds.length != 0)
                                && (QtiSubscriptionController.getInstance()
                                .isActiveSubId(subIds[0])))
                                || (mSimIccId[phoneId] == null
                                && (getCurrentProvisioningStatus(phoneId) ==
                                UiccProvisionStatus.CARD_NOT_PRESENT))) {
                            QtiSubscriptionController.getInstance().updateUserPreferences();
                        }

                        mCis[phoneId].unregisterForAvailable(this);
                    }
                }
                break;
            case EVENT_ICC_CHANGED:
                ar = (AsyncResult)msg.obj;
                if ((ar != null) && ar.result != null) {
                    updateIccAvailability((Integer) ar.result);
                } else {
                    loge("Error: Invalid card index EVENT_ICC_CHANGED ");
                }
                break;
           case EVENT_OEM_HOOK_SERVICE_READY:
                ar = (AsyncResult)msg.obj;
                if ((ar != null) && ar.result != null) {
                    boolean isServiceReady = (boolean)ar.result;
                    if (isServiceReady) {
                        queryAllUiccProvisionInfo();
                    }
                } else {
                    loge("Error: empty result, EVENT_OEM_HOOK_SERVICE_READY");
                }
                break;

           case EVENT_UNSOL_MANUAL_PROVISION_STATUS_CHANGED:
                org.codeaurora.telephony.utils.AsyncResult asyncresult =
                        (org.codeaurora.telephony.utils.AsyncResult)msg.obj;
                if ((asyncresult != null) && asyncresult.result != null) {
                    handleUnsolManualProvisionEvent((Message)asyncresult.result);
                } else {
                    loge("Error: empty result, UNSOL_MANUAL_PROVISION_STATUS_CHANGED");
                }
                break;
           case EVENT_GET_ICCID_DONE:
                ar = (AsyncResult) msg.obj;
                String iccId = null;
                String fullIccId = null;
                if (ar != null) {
                    phoneId = (Integer) ar.userObj;
                    if (ar.result != null) {
                        byte[] data = (byte[]) ar.result;
                        fullIccId = IccUtils.bchToString(data, 0, data.length);
                        iccId = IccUtils.stripTrailingFs(fullIccId);
                    } else {
                        logd("Exception in GET iccId[" + phoneId + "] " +  ar.exception);
                    }
                }
                if ((phoneId >= 0) && (phoneId < mNumPhones)) {
                    mIsIccIdQueryPending[phoneId] = false;
                    // Once IccId avaiable add subInfo record entry
                    if (!TextUtils.isEmpty(iccId)) {
                        logi("SIM_IO add subInfo record, iccId[" + phoneId + "] = " + iccId);
                        mSimFullIccId[phoneId] = fullIccId;
                        QtiSubscriptionInfoUpdater.getInstance().addSubInfoRecord(phoneId, iccId);
                        mSimIccId[phoneId] = iccId;
                        if ((mSimIccId[phoneId] != null) && isAllCardProvisionInfoReceived()) {
                            SubscriptionManager subscriptionManager = (SubscriptionManager) mContext
                                    .getSystemService(Context.TELEPHONY_SUBSCRIPTION_SERVICE);
                            int[] subIds =
                                    QtiSubscriptionController.getInstance().getSubId(phoneId);

                            if ((subIds != null) && (subIds.length != 0) &&
                                    (subscriptionManager.isActiveSubscriptionId(subIds[0]))) {
                                QtiSubscriptionInfoUpdater.getInstance().updateUserPreferences();
                            }
                        }
                        if ((mOldProvisionStatus != null) &&
                                (!mOldProvisionStatus[phoneId].equals(mProvisionStatus[phoneId]))) {
                            logd(" broadcasting ProvisionInfo, phoneId = " +  phoneId);
                            broadcastManualProvisionStatusChanged(phoneId,
                                        getCurrentProvisioningStatus(phoneId));
                            mOldProvisionStatus[phoneId] = mProvisionStatus[phoneId];
                        }
                    } else {
                        logi(" EVENT_GET_ICCID_DONE, ICCID is empty, phoneId = " +  phoneId);
                    }
                }
                break;

            case EVENT_SIM_REFRESH:
                ar = (AsyncResult)msg.obj;

                if (ar != null) {
                    phoneId = (Integer)ar.userObj;
                    logd(" SIM refresh: " + phoneId);
                    if (ar.result != null) {
                        IccRefreshResponse refreshRsp = (IccRefreshResponse)ar.result;

                        // Clear the ICCID when SIM Refresh reset received so that next time when
                        // SIM change event received new ICCID and provision info would be updated
                        if (refreshRsp.refreshResult == IccRefreshResponse.REFRESH_RESULT_RESET) {
                           mSimIccId[phoneId] = null;
                           mSimFullIccId[phoneId] = null;
                        }
                    } else {
                        logd("SIM refresh Exception: " + ar.exception + " phoneId " + phoneId);
                    }
                }
                break;


           default:
               loge("Error: hit default case " + msg.what);
               break;
        }
    }

    private void handleUnsolManualProvisionEvent(Message msg) {
        if (msg == null ||  msg.obj == null) {
            loge("Null data received in handleUnsolManualProvisionEvent");
            return;
        }
        ByteBuffer payload = ByteBuffer.wrap((byte[])msg.obj);
        payload.order(ByteOrder.nativeOrder());

        int rspId = payload.getInt();
        int slotId = msg.arg1;
        if (VDBG) logd(" Unsol: rspId " + rspId + " slotId " + msg.arg1);
        if (isValidSlotId(slotId) && (rspId ==
                QcRilHook.QCRILHOOK_UNSOL_UICC_PROVISION_STATUS_CHANGED)) {
            logi(" Unsol: rspId " + rspId + " slotId " + msg.arg1);
            queryUiccProvisionInfo(slotId, false);
            int dataSubId = SubscriptionManager.getDefaultDataSubscriptionId();
            int dataSlotId = SubscriptionManager.getSlotIndex(dataSubId);
            if (slotId == dataSlotId &&
                     getCurrentProvisioningStatus(dataSlotId) == UiccProvisionStatus.PROVISIONED) {
                logd("Set dds after SSR");
                QtiRadioCapabilityController.getInstance().setDdsIfRequired(false);
            }
        }
    }

    private void queryAllUiccProvisionInfo() {
        for(int index = 0; index < mNumPhones; index++) {
            logd(" query  provision info, card state[" + index + "] = " + mCardState[index]);
            if (mCardState[index] == CardState.CARDSTATE_PRESENT &&
                        (mIsIccIdQueryPending[index] != true)) {
                queryUiccProvisionInfo(index, true);
            }
        }
    }

    public String getUiccIccId(int slotId) {
        return mSimIccId[slotId];
    }

    public String getUiccFullIccId(int slotId) {
        return mSimFullIccId[slotId];
    }

    private void queryUiccProvisionInfo(int phoneId, boolean useSimIORequest) {
        if (!mQtiRilInterface.isServiceReady() || !isValidSlotId(phoneId)) {
            logi("Oem hook service is not ready yet " + phoneId);
            return;
        }
        UiccProvisionStatus oldStatus = mProvisionStatus[phoneId];

        UiccProvisionStatus subStatus = mQtiRilInterface.getUiccProvisionPreference(phoneId);
        if ((subStatus.getCurrentState() != UiccProvisionStatus.INVALID_STATE) &&
                (subStatus.getUserPreference() != UiccProvisionStatus.INVALID_STATE)) {
            synchronized(sManualProvLock) {
                mProvisionStatus[phoneId] = subStatus;
            }
        }

        // FIXME Consider moving iccid querying to speprate method and
        // change the name of this method as needed.
        String fullIccId = getFullIccIdFromUicc(phoneId);
        String iccId = IccUtils.stripTrailingFs(fullIccId);

        if (mSimIccId[phoneId] == null || !mSimIccId[phoneId].equals(iccId)) {
            if (!TextUtils.isEmpty(iccId)) {
                mSimFullIccId[phoneId] = fullIccId;
                 QtiSubscriptionInfoUpdater.getInstance().addSubInfoRecord(phoneId, iccId);
                 mSimIccId[phoneId] = iccId;
            } else {
                logd(" queryUiccProvisionInfo: useSimIORequest=  " +useSimIORequest);
                if (useSimIORequest) {
                    loadIccId(phoneId);
                } else {
                    fullIccId = mQtiRilInterface.getUiccIccId(phoneId);
                    iccId = IccUtils.stripTrailingFs(fullIccId);
                    // Once IccId avaiable add subInfo record entry
                    if (!TextUtils.isEmpty(iccId)) {
                        logi("OEM add subInfo record, iccId[" + phoneId + "] = " + iccId);
                        mSimFullIccId[phoneId] = fullIccId;
                        QtiSubscriptionInfoUpdater.getInstance().
                                addSubInfoRecord(phoneId, iccId);
                        mSimIccId[phoneId] = iccId;
                    } else {
                        logi(" queryUiccProvisionInfo, ICCID[" + phoneId +"] is null");
                    }
                }
            }
        }
        logd(" queryUiccProvisionInfo, iccId[" + phoneId +"] = " +
                mSimIccId[phoneId] + " " + mProvisionStatus[phoneId]);

        if (!oldStatus.equals(mProvisionStatus[phoneId])) {
            if ((mSimIccId[phoneId] != null) && isAllCardProvisionInfoReceived()) {
                int[] subIds = QtiSubscriptionController.getInstance().getSubId(phoneId);
                SubscriptionManager subscriptionManager = (SubscriptionManager) mContext
                        .getSystemService(Context.TELEPHONY_SUBSCRIPTION_SERVICE);

                // Update user preference only if the slot has a valid sub id, so that
                // subscription activation and deactivation would result in updating user
                // preference and new SIMs identified do not trigger user preference update
                // without updating sub id.
                if ((subIds != null) && (subIds.length != 0) &&
                        (subscriptionManager.isActiveSubscriptionId(subIds[0]))) {
                    QtiSubscriptionInfoUpdater.getInstance().updateUserPreferences();
                }
            }
            if (!(useSimIORequest && mSimIccId[phoneId] == null)) {
                logd(" broadcasting ProvisionInfo, phoneId = " +  phoneId);
                broadcastManualProvisionStatusChanged(phoneId,
                            getCurrentProvisioningStatus(phoneId));
                mOldProvisionStatus[phoneId] = mProvisionStatus[phoneId];
            }
        }
    }

    private String getFullIccIdFromUicc(int phoneId) {
        String fullIccId = null;
        UiccCard uiccCard = mUiccController.getUiccCard(phoneId);
        UiccSlot uiccSlot = mUiccController.getUiccSlotForPhone(phoneId);
        if (uiccCard != null) {
            fullIccId = uiccCard.getIccId();
        } else if (uiccSlot != null){
            logi(" queryUiccProvisionInfo, uiccCard[" + phoneId +"] object is null");
            fullIccId = uiccSlot.getIccId();
        } else {
            logi(" queryUiccProvisionInfo, uiccSlot[" + phoneId +"] object is null");
        }
        logi(" queryUiccProvisionInfo, full iccId for " + phoneId +"is " + fullIccId);
        return fullIccId;
    }

    private void loadIccId(int phoneId) {
        UiccCard uiccCard = mUiccController.getUiccCard(phoneId);
        if (uiccCard != null) {
            UiccCardApplication validApp = null;
            int numApps = uiccCard.getNumApplications();
            for (int i = 0; i < numApps; i++) {
                UiccCardApplication app = uiccCard.getApplicationIndex(i);
                if (app != null && app.getType() != AppType.APPTYPE_UNKNOWN) {
                    validApp = app;
                    break;
                }
            }
            if (validApp != null) {
                IccFileHandler fileHandler = validApp.getIccFileHandler();
                if (fileHandler != null) {
                    mIsIccIdQueryPending[phoneId] = true;
                    fileHandler.loadEFTransparent(IccConstants.EF_ICCID,
                                obtainMessage(EVENT_GET_ICCID_DONE, phoneId));
                }
            }
        }
    }

    private void updateIccAvailability(int phoneId) {
        if (!isValidSlotId(phoneId)) {
            loge("Invalid phone Index!!! " + phoneId);
            return;
        }
        CardState newState = CardState.CARDSTATE_ABSENT;
        UiccSlot newSlot = mUiccController.getUiccSlotForPhone(phoneId);
        if (newSlot != null) {
            newState = newSlot.getCardState();
        } else {
            logd("updateIccAvailability, uicc card null, ignore " + phoneId);
            return;
        }
        logd("updateIccAvailability, card state[" + phoneId + "] = " + newState);

        mCardState[phoneId] = newState;
        int currentState = getCurrentProvisioningStatus(phoneId);
        if (mCardState[phoneId] == CardState.CARDSTATE_PRESENT &&
                (mSimIccId[phoneId] == null ||
                currentState == UiccProvisionStatus.INVALID_STATE ||
                currentState == UiccProvisionStatus.CARD_NOT_PRESENT) &&
                (mIsIccIdQueryPending[phoneId] != true)) {
            queryUiccProvisionInfo(phoneId, true);
        } else if ((mCardState[phoneId] == CardState.CARDSTATE_ABSENT &&
                !newSlot.isStateUnknown()) ||
                (mCardState[phoneId] == CardState.CARDSTATE_ERROR)) {
            synchronized(sManualProvLock) {
                mProvisionStatus[phoneId].setUserPreference(UiccProvisionStatus.CARD_NOT_PRESENT);
                mProvisionStatus[phoneId].setCurrentState(UiccProvisionStatus.CARD_NOT_PRESENT);
                mOldProvisionStatus[phoneId].setUserPreference(
                        UiccProvisionStatus.CARD_NOT_PRESENT);
                mOldProvisionStatus[phoneId].setCurrentState(UiccProvisionStatus.CARD_NOT_PRESENT);
                mSimIccId[phoneId] = null;
                mSimFullIccId[phoneId] = null;
                mManualProvisionChangedRegistrants.notifyRegistrants(
                        new AsyncResult(null, phoneId, null));
            }
            if (isAllCardProvisionInfoReceived()) {
                QtiSubscriptionInfoUpdater.getInstance().updateUserPreferences();
            }
        }
    }

    private void broadcastManualProvisionStatusChanged(int phoneId, int newProvisionState) {
        Intent intent = new Intent(ACTION_UICC_MANUAL_PROVISION_STATUS_CHANGED);
        intent.putExtra(PhoneConstants.PHONE_KEY, phoneId);
        intent.putExtra(EXTRA_NEW_PROVISION_STATE, newProvisionState);
        mContext.sendBroadcast(intent);
        mManualProvisionChangedRegistrants.notifyRegistrants(new AsyncResult(null, phoneId, null));
    }

    /// local utility method to get current provision status.
    private int getCurrentProvisioningStatus(int slotId) {
        synchronized(sManualProvLock) {
            return mProvisionStatus[slotId].getCurrentState();
        }
    }

    public int getCurrentUiccCardProvisioningStatus(int slotId) {
        int currentStatus = UiccProvisionStatus.INVALID_STATE;

        // Single SIM devices are auto provisioned.
        if ((mNumPhones == 1) && isValidSlotId(slotId)) {
            currentStatus = UiccProvisionStatus.PROVISIONED;
        } else if (canProcessRequest(slotId)) {
            currentStatus = getCurrentProvisioningStatus(slotId);
        }
        if (VDBG) logd(" getCurrentUiccCardProvisioningStatus, state[" +
                slotId + "] = " + currentStatus);
        return currentStatus;
    }

    public int getUiccCardProvisioningUserPreference(int slotId) {
        int userPref = UiccProvisionStatus.INVALID_STATE;

        // Single SIM devices are auto provisioned.
        if ((mNumPhones == 1) && isValidSlotId(slotId)) {
            userPref = UiccProvisionStatus.PROVISIONED;
        } else if (canProcessRequest(slotId)) {
            synchronized(sManualProvLock) {
                userPref = mProvisionStatus[slotId].getUserPreference();
            }
        }
        if (VDBG) logd(" getUiccCardProvisioningUserPreference pref[" + slotId + "] = " + userPref);
        return userPref;
    }

    public int activateUiccCard(int slotId) {
        logd(" activateUiccCard: phoneId = " + slotId);

        if (!mQtiRilInterface.isServiceReady()) {
            logi("Oem hook service is not ready yet " + slotId);
            return GENERIC_FAILURE;
        }

        enforceModifyPhoneState("activateUiccCard");

        int activateStatus = SUCCESS;

        if (!canProcessRequest(slotId)) {
            activateStatus = INVALID_INPUT;
        } else if (getCurrentProvisioningStatus(slotId) == UiccProvisionStatus.PROVISIONED) {
            logd(" Uicc card in slot[" + slotId + "] already activated ");
        } else if (!isFlexMapInProgress() && mRequestInProgress.compareAndSet(false, true)) {
            boolean retVal = false;

            retVal = mQtiRilInterface.setUiccProvisionPreference(
                    UiccProvisionStatus.PROVISIONED, slotId);
            if (!retVal) {
                activateStatus = GENERIC_FAILURE;
            } else {
                synchronized(sManualProvLock) {
                    mProvisionStatus[slotId].setCurrentState(UiccProvisionStatus.PROVISIONED);
                }
            }
            logi(" activation result[" + slotId + "] = " + retVal);
            mRequestInProgress.set(false);
        } else {
            activateStatus = REQUEST_IN_PROGRESS;
        }

        return activateStatus;
    }

    public int deactivateUiccCard(int slotId) {
        logd(" deactivateUiccCard: phoneId = " + slotId);

        if (!mQtiRilInterface.isServiceReady()) {
            logi("Oem hook service is not ready yet " + slotId);
            return GENERIC_FAILURE;
        }

        enforceModifyPhoneState("deactivateUiccCard");

        int deactivateState = SUCCESS;
        if (!canProcessRequest(slotId)) {
            deactivateState = INVALID_INPUT;
        } else if (getCurrentProvisioningStatus(slotId) == UiccProvisionStatus.NOT_PROVISIONED) {
            logd(" Uicc card in slot[" + slotId + "] already in deactive state ");
        } else if (!isFlexMapInProgress() && mRequestInProgress.compareAndSet(false, true)) {
            boolean retVal = false;

            retVal = mQtiRilInterface.setUiccProvisionPreference(
                    UiccProvisionStatus.NOT_PROVISIONED, slotId);
            if (!retVal) {
                deactivateState = GENERIC_FAILURE;
            } else {
                synchronized(sManualProvLock) {
                    mProvisionStatus[slotId].setCurrentState(UiccProvisionStatus.NOT_PROVISIONED);
                }
            }
            logi(" deactivation result[" + slotId + "] = " + retVal);
            mRequestInProgress.set(false);
        } else {
            deactivateState = REQUEST_IN_PROGRESS;
        }

        return deactivateState;
    }

    private void enforceModifyPhoneState(String message) {
        mContext.enforceCallingOrSelfPermission(
                android.Manifest.permission.MODIFY_PHONE_STATE, message);
    }

    // Manual provision requests shall be processed for MSIM targets only
    private boolean canProcessRequest(int slotId) {
        boolean retVal = false;

        if (mNumPhones > 1 && isValidSlotId(slotId)) {
            retVal = true;
        } else {
            loge("Request can't be processed, slotId " + slotId + " numPhones " + mNumPhones);
        }
        return retVal;
    }

    private boolean isValidSlotId(int slotId) {
        boolean retVal = false;

        if (slotId >= 0 && slotId < mNumPhones) {
            retVal = true;
        }
        return retVal;
    }

    public boolean isFlexMapInProgress() {
        boolean retVal = false;
        QtiRadioCapabilityController rcController = QtiRadioCapabilityController.getInstance();

        if (rcController != null) {
            retVal = rcController.isSetNWModeInProgress();
            logd("isFlexMapInProgress: = " + retVal);
        }

        return retVal;
    }

    public boolean isAnyProvisionRequestInProgress() {
        return  mRequestInProgress.get();
    }

    // Utility method returns true if it receives valid provision
    // state from below layers for all SIM cards.
    public boolean isAllCardProvisionInfoReceived() {
        boolean received = true;
        for(int index = 0; index < mNumPhones; index++) {
            int provPref = getCurrentProvisioningStatus(index);
            if ((provPref == UiccProvisionStatus.INVALID_STATE) ||
                    (mSimIccId[index] != null &&
                    (provPref == UiccProvisionStatus.CARD_NOT_PRESENT))) {
                logd("isAllCardProvisionInfoReceived, prov pref[" + index + "] = " + provPref);
                received = false;
                break;
            }
        }
        return received;
    }

    private void logd(String string) {
        if (DBG) Rlog.d(LOG_TAG, string);
    }

    private void logi(String string) {
        Rlog.i(LOG_TAG, string);
    }

    private void loge(String string) {
        Rlog.e(LOG_TAG, string);
    }
}
