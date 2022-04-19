/*
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package org.codeaurora.ims;

import android.content.Context;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.PersistableBundle;
import android.os.RemoteException;
import android.telephony.SubscriptionManager;
import android.provider.Telephony.Sms;
import android.telephony.CarrierConfigManager;
import android.telephony.ims.stub.ImsSmsImplBase;
import android.telephony.SmsMessage;

import org.codeaurora.ims.sms.IncomingSms;
import org.codeaurora.ims.sms.SmsResponse;
import org.codeaurora.ims.sms.StatusReport;
import com.qualcomm.ims.utils.Log;
import org.codeaurora.telephony.utils.AsyncResult;

import java.util.HashMap;
import java.util.Random;

public class ImsSmsImpl extends ImsSmsImplBase {
    static final String LOG_TAG = "ImsSmsImpl";
    private Context mContext;
    private final Object mLock = new Object();
    private ImsSenderRxr mSmsCi; /* Commands Interface */
    private int mPhoneId = 0;
    /* Stores message ref comes from modem in #sendResponseToFramework
       with a token and used to retrive this token to send to
       framework later  when the same messge ref is received
       in status report */
    private HashMap<Integer, Integer> mSmsMap = new HashMap<Integer, Integer>();
    private HandlerThread mHandlerThread;
    private Handler mHandler;
    private boolean mSmsFwkReady = false;
    private int mIncomingToken = 0;

    private final int EVENT_SEND_SMS_RESPONSE = 1;
    private final int EVENT_UNSOL_INCOMING_SMS = 2;
    private final int EVENT_UNSOL_STATUS_REPORT = 3;

    public static final int MT_IMS_STATUS_VALIDATION_NONE = 0;
    public static final int MT_IMS_STATUS_VALIDATION_PASS = 1;
    public static final int MT_IMS_STATUS_VALIDATION_FAIL = 2;

    public ImsSmsImpl(Context context, int phoneId, ImsSenderRxr ci) {
        mContext = context;
        mSmsCi = ci;
        mPhoneId = phoneId;
        mHandlerThread = new HandlerThread(ImsSmsImpl.class.getSimpleName());
        mHandlerThread.start();
        mHandler = new ImsSmsHandler(mHandlerThread.getLooper());
        mSmsCi.setOnIncomingImsSms(mHandler, EVENT_UNSOL_INCOMING_SMS, null);
        mSmsCi.setOnImsSmsStatusReport(mHandler, EVENT_UNSOL_STATUS_REPORT, null);
    }

    private void sendStatusReportErrorToRIL(int msgRef){
        mSmsCi.acknowledgeSmsReport(msgRef, STATUS_REPORT_STATUS_ERROR, null);
    }

    private void sendDeliveryErrorToRIL(){
        mSmsCi.acknowledgeSms(0, DELIVER_STATUS_ERROR_GENERIC, null);
    }

    private int getSubId(){
        SubscriptionManager subscriptionManager = (SubscriptionManager) mContext.getSystemService(
                Context.TELEPHONY_SUBSCRIPTION_SERVICE);
        if (subscriptionManager == null) {
            return SubscriptionManager.INVALID_SUBSCRIPTION_ID;
        }

        int[] subIds = subscriptionManager.getSubscriptionIds(mPhoneId);
        if (subIds == null || subIds.length == 0) {
            return SubscriptionManager.INVALID_SUBSCRIPTION_ID;
        }
        return subIds[0];
    }

    private boolean isRetryAlwaysOverIMS() {
        CarrierConfigManager configManager = (CarrierConfigManager)
            mContext.getSystemService(Context.CARRIER_CONFIG_SERVICE);
        PersistableBundle b = null;
        boolean retryAlwaysOverIMS = false;
        if (configManager != null) {
            b = configManager.getConfigForSubId(getSubId());
        }
        if (b != null) {
            retryAlwaysOverIMS = b.getBoolean("config_retry_sms_over_ims", false);
        }
        return retryAlwaysOverIMS;
    }

    private void sendResponseToFramework(AsyncResult ar) {
        SmsResponse smsResponse = (SmsResponse) ar.result;
        int token = (int) ar.userObj;
        if (smsResponse == null || !mSmsFwkReady) {
            Log.w(this, "smsResponse =" + smsResponse +
                    " isFrameworkRead=" + mSmsFwkReady);
            return;
        }
        final int ref = smsResponse.getMsgRef();
        int result = smsResponse.getResult();
        final int reason = smsResponse.getReason();
        if ((result == ImsSmsImplBase.SEND_STATUS_ERROR_FALLBACK) && isRetryAlwaysOverIMS()) {
            result = ImsSmsImplBase.SEND_STATUS_ERROR_RETRY;
        }
        Log.i(this,"onSendSmsResult:: token:"+token+
                " smsResponse:"+ smsResponse);

        synchronized (mLock) { mSmsMap.put(ref, token); }
        try {
            onSendSmsResult(token, ref, result, reason);
        } catch (RuntimeException ex) {
            Log.e(this, "onSendSmsResult: Ex:" + ex.getMessage());
        }
    }

    private void sendSmsToFramework(AsyncResult ar) {
        IncomingSms incomingSms = (IncomingSms) ar.result;
        if (incomingSms == null || !mSmsFwkReady) {
            Log.w(this, "incomingSms =" + incomingSms +
                    " isFrameworkRead=" + mSmsFwkReady);
            sendDeliveryErrorToRIL();
            return;
        }
        if(mIncomingToken == Integer.MAX_VALUE){ mIncomingToken = 0; }

        /**verstat info is applicable only to 3gpp MT Sms.
         * valid values are:
         * MT_IMS_STATUS_VALIDATION_NONE.
         * MT_IMS_STATUS_VALIDATION_PASS.
         * MT_IMS_STATUS_VALIDATION_FAIL.
         *
         * For other formats, ims service always gets MT_IMS_STATUS_VALIDATION_NONE.
         */
        Log.i(this,"onSmsReceived:: token:"+mIncomingToken+
                " incomingSms:" + incomingSms);

        try {
            onSmsReceived(mIncomingToken++, incomingSms.getFormat(), incomingSms.getPdu());
        } catch (RuntimeException ex) {
            Log.e(this, "onSmsReceived: Ex:" + ex.getMessage());
            sendDeliveryErrorToRIL();
        }
    }

    private boolean isSmsDeliverCompleted(StatusReport report) {
        if (report == null) {
            return false;
        }

        // For CDMA status report always return true
        if (SmsMessage.FORMAT_3GPP2.equals(report.getFormat())) {
            return true;
        }

        SmsMessage msg = SmsMessage.createFromPdu(report.getPdu(), report.getFormat());
        if (msg != null) {
            int status = msg.getStatus();
            Log.d(this, "isSmsDeliverCompleted : status = " + status);
            // return true if status is failed or not pending
            return (status >= Sms.STATUS_FAILED || status < Sms.STATUS_PENDING );
        }
        return false;
    }

    private void sendStatusReportToFramework(AsyncResult ar) {
        StatusReport statusReport = (StatusReport) ar.result;
        int token = 0;
        if (statusReport == null || !mSmsFwkReady) {
            Log.w(this, "statusReport =" + statusReport +
                    " isFrameworkRead=" + mSmsFwkReady);
            sendStatusReportErrorToRIL(0);
            return;
        }
        final int ref = statusReport.getMsgRef();
        final String format =  statusReport.getFormat();
        synchronized (mLock) {
            Integer tokenObj = mSmsMap.get(ref);
            token = (tokenObj == null) ? -1 : tokenObj.intValue();
        }

        Log.i(this,"onSmsStatusReportReceived:: token: "+token+
                " statusReport:" + statusReport);

        try {
            onSmsStatusReportReceived(token, ref, format, statusReport.getPdu());
        } catch (RuntimeException ex) {
            Log.e(this, "onSmsStatusReportReceived: Ex:" + ex.getMessage());
            sendStatusReportErrorToRIL(ref);
        }

        // Remove token from map if delivery status is not pending
        if (isSmsDeliverCompleted(statusReport)) {
            synchronized (mLock) {
                mSmsMap.remove(ref);
            }
        }
    }

    private class ImsSmsHandler extends Handler {
        public ImsSmsHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            Log.i(this, "Message received: what = " + msg.what);
            AsyncResult ar = (AsyncResult) msg.obj;
            if(ar == null){
                Log.e(this, "msg.obg is null");
                return;
            }
            switch (msg.what) {
                case EVENT_SEND_SMS_RESPONSE:
                    sendResponseToFramework(ar);
                    break;
                case EVENT_UNSOL_INCOMING_SMS:
                    sendSmsToFramework(ar);
                    break;
                case EVENT_UNSOL_STATUS_REPORT:
                    sendStatusReportToFramework(ar);
                    break;
                default:
                    Log.i(LOG_TAG, "Invalid Response");
            }
        }
    }

    @Override
    public void sendSms(int token, int messageRef, String format,
                        String smsc, boolean isRetry, byte[] pdu) {
        Log.i(LOG_TAG,"sendSms:: token:"+token+" msgRef:"+messageRef+
                " format:"+format+" isRetry:"+isRetry);

        mSmsCi.sendSms(messageRef, format, smsc, isRetry, pdu,
                mHandler.obtainMessage(EVENT_SEND_SMS_RESPONSE, token));
    }

    @Override
    public void acknowledgeSms(int token, int messageRef, int result) {
        Log.i(LOG_TAG,"acknowledgeSms:: token:"+token+" msgRef:"+messageRef+
                " result:"+result);

        mSmsCi.acknowledgeSms(messageRef, result, null);
    }

    @Override
    public void acknowledgeSmsReport(int token, int messageRef, int result) {
        Log.i(LOG_TAG,"acknowledgeSmsReport:: token:"+token+" msgRef:"+messageRef+
                " result:"+result);

        mSmsCi.acknowledgeSmsReport(messageRef, result, null);
    }

    @Override
    public String getSmsFormat() {
        return mSmsCi.getSmsFormat();
    }

    @Override
    public void onReady() {
        mSmsFwkReady = true;
    }
}
