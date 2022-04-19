/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.internal.telephony.dataconnection;

import android.app.AlarmManager;
import android.app.AlertDialog;
import android.app.PendingIntent;
import android.content.Context;
import android.os.AsyncResult;
import android.os.Message;
import android.telephony.DataFailCause;
import android.telephony.ServiceState;
import android.view.WindowManager;

import com.android.internal.telephony.dataconnection.ApnContext;
import com.android.internal.telephony.dataconnection.DataConnection;
import com.android.internal.telephony.dataconnection.DataServiceManager;
import com.android.internal.telephony.dataconnection.DcController;
import com.android.internal.telephony.dataconnection.DcTesterFailBringUpAll;
import com.android.internal.telephony.dataconnection.DcTracker;
import com.android.internal.telephony.Phone;

import java.io.FileDescriptor;
import java.io.PrintWriter;

public final class QtiDataConnection extends DataConnection {

    // Maximum data reject count
    protected static final int MAX_PDP_REJECT_COUNT = 3;

    // current data reject count
    protected static int mPdpRejectCount = 0;

    // current data reject cause
    protected static int mPdpRejectCause = DataFailCause.UNKNOWN;

    // data reject dialog, made static because only one dialog object can be
    // used between multiple dataconnection objects.
    protected static AlertDialog mDataRejectDialog = null;

    // Data reset event tracker to know reset events.
    protected QtiDataResetEventTracker mQtiDataResetEventTracker = null;

    protected QtiDcRetryAlarmController mDcRetryAlarmController;

    // Data reset event listener. Dc will get get onResetEvent
    // whenever any data reset event occurs.
    protected QtiDataResetEventTracker.ResetEventListener mResetEventListener =
           new QtiDataResetEventTracker.ResetEventListener() {

        @Override
        public void onResetEvent(boolean retry) {
            if (DBG) log("onResetEvent: reset mPdpRejectCount");
            mPdpRejectCount = 0;
            mPdpRejectCause = DataFailCause.UNKNOWN;
            mDcRetryAlarmController.cancel();
            //Dismiss dialog
            if (mDataRejectDialog != null && mDataRejectDialog.isShowing()) {
                if (DBG) log("onResetEvent: Dismiss dialog");
                mDataRejectDialog.dismiss();
            }
            if (retry) {
                if (DBG) log("onResetEvent: retry data call");
                sendMessage(obtainMessage(EVENT_RETRY_CONNECTION, mTag, 0));
            }
        }
    };

    // ***** Constructor (NOTE: uses dcc.getHandler() as its Handler)
    public QtiDataConnection(Phone phone, String name, int id,
            DcTracker dct, DataServiceManager dataServiceManager,
            DcTesterFailBringUpAll failBringUpAll, DcController dcc) {
        super(phone, name, id, dct, dataServiceManager, failBringUpAll, dcc);
        mDcRetryAlarmController = new QtiDcRetryAlarmController(mPhone, this);
    }

    public static DataConnection makeDataConnection(Phone phone, int id,
            DcTracker dct, DataServiceManager dataServiceManager,
            DcTesterFailBringUpAll failBringUpAll, DcController dcc) {
        DataConnection dc = new QtiDataConnection(phone,
                "QtiDC-" + mInstanceNumber.incrementAndGet(), id, dct,
                dataServiceManager, failBringUpAll, dcc);
        dc.start();
        return dc;
    }

    @Override
    protected void dispose() {
        super.dispose();
        if (mQtiDataResetEventTracker != null) {
            mQtiDataResetEventTracker.dispose();
            mQtiDataResetEventTracker = null;
        }
        if (mDcRetryAlarmController != null) {
            mDcRetryAlarmController.dispose();
            mDcRetryAlarmController = null;
        }
    }

    @Override
    protected void notifyDisconnectCompleted(DisconnectParams dp,
            boolean sendAll) {
        super.notifyDisconnectCompleted(dp, sendAll);
        // Data connection is disconnected,
        // reset pdp reject count to 0
        if (DBG) log("notifyDisconnectCompleted, reset mPdpRejectCount");
        mPdpRejectCount = 0;
        mPdpRejectCause = DataFailCause.UNKNOWN;
        // cancel the pending alarm and stop the data reset tracker
        mDcRetryAlarmController.cancel();
        if (mQtiDataResetEventTracker != null) {
            mQtiDataResetEventTracker.stopResetEventTracker();
        }
    }

    /**
     * returns true if radioTechnology is WCDMA rat, else false
     */
    private boolean isWCDMA(int radioTechnology) {
        return radioTechnology == ServiceState.RIL_RADIO_TECHNOLOGY_UMTS
            || radioTechnology == ServiceState.RIL_RADIO_TECHNOLOGY_HSDPA
            || radioTechnology == ServiceState.RIL_RADIO_TECHNOLOGY_HSUPA
            || radioTechnology == ServiceState.RIL_RADIO_TECHNOLOGY_HSPA
            || radioTechnology == ServiceState.RIL_RADIO_TECHNOLOGY_HSPAP;
    }

    /**
     * This function will display the pdp reject message
     */
    private void displayPopup(int pdpRejectCause) {
        if (DBG) log("displayPopup : " + pdpRejectCause);
        String title = mPhone.getContext().getResources().
                getString(com.android.internal.R.string.data_conn_status_title);
        String message = null;
        if (pdpRejectCause == DataFailCause.USER_AUTHENTICATION) {
            message = mPhone.getContext().getResources().
                    getString(com.android.internal.R.string.user_authentication_failed);
        } else if (pdpRejectCause == DataFailCause.SERVICE_OPTION_NOT_SUBSCRIBED) {
            message = mPhone.getContext().getResources().getString(
                    com.android.internal.R.string.service_not_subscribed);
        } else if (pdpRejectCause == DataFailCause.MULTI_CONN_TO_SAME_PDN_NOT_ALLOWED) {
            message = mPhone.getContext().getResources().getString(
                    com.android.internal.R.string.multi_conn_to_same_pdn_not_allowed);
        }
        if (mDataRejectDialog == null || !mDataRejectDialog.isShowing()) {
            AlertDialog.Builder builder = new AlertDialog.Builder(
                    mPhone.getContext());
            builder.setPositiveButton(android.R.string.ok, null);
            mDataRejectDialog = builder.create();
        }
        mDataRejectDialog.setMessage(message);
        mDataRejectDialog.setCanceledOnTouchOutside(false);
        mDataRejectDialog.setTitle(title);
        mDataRejectDialog.getWindow().setType(
                WindowManager.LayoutParams.TYPE_SYSTEM_ALERT);
        mDataRejectDialog.show();
    }

    /**
     * returns true if Data call is rejected with following cause codes:
     * 1. USER_AUTHENTICATION
     * 2. SERVICE_OPTION_NOT_SUBSCRIBED
     * 3. MULTI_CONN_TO_SAME_PDN_NOT_ALLOWED
     */
    private boolean handleDataReject(SetupResult result,
            ConnectionParams cp) {
        ApnContext apnContext = cp.mApnContext;
        boolean handled = false;
        int delay = mPhone.getContext().getResources().getInteger(
                com.android.internal.R.integer.data_retry_delay);
        log("handleDataReject: delay from config: " + delay);

        /*
         * If previously rejected code is not same as current data reject cause,
         * then reset the count and reset the reject cause.
         */
        if (mPdpRejectCause != result.mFailCause) {
            if (DBG) log("handleDataReject: mPdpRejectCause: " + mPdpRejectCause
                    + ", result.mFailCause: " + result.mFailCause + ", reset mPdpRejectCount");
            mPdpRejectCount = 0;
            mPdpRejectCause = DataFailCause.UNKNOWN;
            if (mQtiDataResetEventTracker != null) {
                mQtiDataResetEventTracker.stopResetEventTracker();
            }
        }
        /*
         * If failure reason is USER_AUTHENTICATION or
         * SERVICE_OPTION_NOT_SUBSCRIBED or MULTI_CONN_TO_SAME_PDN_NOT_ALLOWED,
         * increment counter and store cause
         */
        if (isPdpRejectCause(result.mFailCause)) {
            if (mQtiDataResetEventTracker == null) {
                mQtiDataResetEventTracker = new QtiDataResetEventTracker(mTransportType,
                        mPhone, QtiDataConnection.this, mResetEventListener);
            }
            if (mPdpRejectCount == 0) {
                mQtiDataResetEventTracker.startResetEventTracker();
            }
            mPdpRejectCount++;
            if (DBG) log ("handleDataReject: new mPdpRejectCount = " + mPdpRejectCount);
            mPdpRejectCause = result.mFailCause;
            /* If MAX Reject count reached, display pop-up to user */
            if (MAX_PDP_REJECT_COUNT <= mPdpRejectCount) {
                if (DBG) log("handleDataReject: reached max retry count");
                displayPopup(mPdpRejectCause);
                delay = -1;
            }

            if (delay > 0) {
                if (DBG) log("handleDataReject: DcActivatingState: **ERR_RilError retry**");
                if (apnContext != null) {
                    PendingIntent intent = apnContext.getReconnectIntent();
                    if (intent != null) {
                        AlarmManager am = (AlarmManager) mPhone.getContext()
                                .getSystemService(Context.ALARM_SERVICE);
                        am.cancel(intent);
                        apnContext.setReconnectIntent(null);
                    } else {
                        if (DBG) log("handleDataReject: intent = null");
                    }
                } else {
                    if (DBG) log("handleDataReject: apnContext = null");
                }

                log("handleDataReject: startRetryAlarm with delay " + delay + ", mTag: " + mTag);
                mDcRetryAlarmController.startRetryAlarm(EVENT_RETRY_CONNECTION,
                        mTag, delay);
            }
            mInactiveState.setEnterNotificationParams(cp, result.mFailCause);
            transitionTo(mInactiveState);
            handled = true;
        }
        return handled;
    }

    @Override
    protected void handlePdpRejectCauseSuccess() {
        if (mPdpRejectCount > 0 && mPdpRejectCause != DataFailCause.UNKNOWN) {
            log("handlePdpRejectCauseSuccess: reset mPdpRejectCount");
            mPdpRejectCount = 0;
            mPdpRejectCause = DataFailCause.UNKNOWN;
            if (mQtiDataResetEventTracker != null) {
                mQtiDataResetEventTracker.stopResetEventTracker();
            }
        }
    }

    @Override
    protected boolean isPdpRejectCauseFailureHandled(SetupResult result,
            ConnectionParams cp) {
        // only check for reject cause USER_AUTHENTICATION or
        // SERVICE_OPTION_NOT_SUBSCRIBED or MULTI_CONN_TO_SAME_PDN_NOT_ALLOWED if the rat
        // isWCDMA and feature flag is enabled
        boolean ishandled = false;
        if (isWCDMA(mPhone.getServiceState().getRilDataRadioTechnology())
                && isPdpRejectConfigEnabled()) {
            if (DBG) log("isPdpRejectCauseFailureHandled: config is enabled"
                    + "mPdpRejectCount: " + mPdpRejectCount);
            ishandled = handleDataReject(result, cp);
        } else {
            if (DBG) log("isPdpRejectCauseFailureHandled: DataConnection not on wcdma");
            mPdpRejectCount = 0;
            mPdpRejectCause = DataFailCause.UNKNOWN;
            if (mQtiDataResetEventTracker != null) {
                mQtiDataResetEventTracker.stopResetEventTracker();
            }
        }
        return ishandled;
    }

    @Override
    protected boolean isDataCallConnectAllowed() {
       boolean connectAllowed = true;
       if (mPdpRejectCount > 0) {
           if (DBG) log("isDataCallConnectAllowed: do not allow, mPdpRejectCount="
                   + mPdpRejectCount);
           connectAllowed = false;
       }
       return connectAllowed;
    }

    @Override
    public void dump(FileDescriptor fd, PrintWriter pw, String[] args) {
        pw.print("QtiDataConnection ");
        pw.println(" mDcRetryAlarmController=" + mDcRetryAlarmController);
        super.dump(fd, pw, args);
    }
}
