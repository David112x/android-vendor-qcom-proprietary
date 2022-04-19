/*
 * Copyright (c) 2015, 2017-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (C) 2006 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

package org.codeaurora.ims;

import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hidl.manager.V1_0.IServiceManager;
import android.hidl.manager.V1_0.IServiceNotification;
import android.location.Address;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.HwBinder;
import android.os.Looper;
import android.os.Message;
import android.os.PowerManager;
import android.os.RemoteException;
import android.os.PowerManager.WakeLock;
import android.os.SystemProperties;
import android.telephony.ims.ImsCallForwardInfo;
import android.telephony.ims.ImsCallProfile;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsSsData;
import android.telephony.ims.ImsSsInfo;
import android.telephony.ims.stub.ImsSmsImplBase;
import android.telephony.PhoneNumberUtils;
import android.telephony.SubscriptionManager;

import com.android.ims.ImsException;
import com.android.internal.annotations.VisibleForTesting;
import com.android.internal.telephony.TelephonyIntents;
import com.android.internal.telephony.TelephonyProperties;
import com.qualcomm.ims.utils.Log;
import org.codeaurora.telephony.utils.AsyncResult;
import org.codeaurora.telephony.utils.Registrant;
import org.codeaurora.telephony.utils.RegistrantList;
import org.codeaurora.telephony.utils.SomeArgs;
import org.codeaurora.ims.sms.SmsResponse;
import org.codeaurora.ims.sms.StatusReport;
import org.codeaurora.ims.sms.IncomingSms;
import org.codeaurora.ims.MultiIdentityLineInfo;
import vendor.qti.hardware.radio.ims.V1_0.*;
import vendor.qti.hardware.radio.ims.V1_3.AutoCallRejectionInfo;
import vendor.qti.hardware.radio.ims.V1_4.MultiIdentityLineInfoHal;
import vendor.qti.hardware.radio.ims.V1_2.ImsSmsMessage;
import vendor.qti.hardware.radio.ims.V1_2.ImsSmsSendStatusReport;
import vendor.qti.hardware.radio.ims.V1_2.IncomingImsSms;

import java.lang.ClassCastException;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.List;

import org.codeaurora.ims.QtiCallConstants;
import org.codeaurora.ims.utils.QtiImsExtUtils;

/**
 * {@hide}
 */
class IFRequest {

    // ***** Class Variables
    static int sNextSerial = 0;
    static Object sSerialMonitor = new Object();
    private static Object sPoolSync = new Object();
    private static IFRequest sPool = null;
    private static int sPoolSize = 0;
    private static final int MAX_POOL_SIZE = 4;

    // ***** Instance Variables
    int mSerial;
    int mRequest;
    Message mResult;
    // FIXME delete parcel
    // Parcel mp;
    IFRequest mNext;
    byte[] mData;

    /**
     * Retrieves a new IFRequest instance from the pool.
     *
     * @param request MessageId.REQUEST_*
     * @param result sent when operation completes
     * @return a IFRequest instance from the pool.
     */
    static IFRequest obtain(int request, Message result) {
        IFRequest rr = null;

        synchronized (sPoolSync) {
            if (sPool != null) {
                rr = sPool;
                sPool = rr.mNext;
                rr.mNext = null;
                sPoolSize--;
            }
        }

        if (rr == null) {
            rr = new IFRequest();
        }

        synchronized (sSerialMonitor) {
            rr.mSerial = sNextSerial++;
        }
        rr.mRequest = request;
        rr.mResult = result;

        if (result != null && result.getTarget() == null) {
            throw new NullPointerException("Message target must not be null");
        }

        return rr;
    }

    void setResult(Message newMsg) {
        mResult = newMsg;
    }

    /**
     * Returns a IFRequest instance to the pool. Note: This should only be
     * called once per use.
     */
    void release() {
        synchronized (sPoolSync) {
            if (sPoolSize < MAX_POOL_SIZE) {
                this.mNext = sPool;
                sPool = this;
                sPoolSize++;
                mResult = null;
            }
        }
    }

    private IFRequest() {
    }

    static void resetSerial() {
        synchronized (sSerialMonitor) {
            sNextSerial = 0;
        }
    }

    String serialString() {
        // Cheesy way to do %04d
        StringBuilder sb = new StringBuilder(8);
        String sn;

        sn = Integer.toString(mSerial);

        // sb.append("J[");
        sb.append('[');
        for (int i = 0, s = sn.length(); i < 4 - s; i++) {
            sb.append('0');
        }

        sb.append(sn);
        sb.append(']');
        return sb.toString();
    }

    void onError(int error, Object ret) {
        RuntimeException ex;
        String errorMsg;

        if (error == ImsErrorCode.SUCCESS) {
            ex = null;
        } else {
            errorMsg = ImsSenderRxr.errorIdToString(error);
            ex = new ImsRilException(error, errorMsg);
        }

        Log.i(this, serialString() + "< "
                + ImsSenderRxr.msgIdToString(mRequest)
                + " error: " + error);

        if (mResult != null) {
            AsyncResult.forMessage(mResult, ret, ex);
            mResult.sendToTarget();
        }
    }
}

/**
 * IMS implementation of the CommandsInterface. {@hide}
 */
public class ImsSenderRxr extends ImsPhoneBaseCommands {
    static final String LOG_TAG = "ImsSenderRxr";

    /**
     * Wake lock timeout should be longer than the longest timeout in the vendor
     */
    private static final int DEFAULT_WAKE_LOCK_TIMEOUT = 60000;
    private static final int PDU_LENGTH_OFFSET = 4;
    private static final int MSG_TAG_LENGTH = 1;
    private static final int STATUS_INTERROGATE = 2;

    // ***** Events
    static final int EVENT_WAKE_LOCK_TIMEOUT = 1;

    // ***** Instance Variables
    WakeLock mWakeLock;
    int mWakeLockTimeout;

    final ImsRadioHandler mImsRadioHandler;

    // The number of requests sent out but waiting for response. It increases
    // while
    // sending request and decreases while handling response. It should match
    // mRequestList.size() unless there are requests no replied while
    // WAKE_LOCK_TIMEOUT occurs.
    int mRequestMessagesWaiting;

    /* Variable caching the Phone ID */
    private Integer mPhoneId;

    /* Variable caching the presence of UNSOL call list indication */
    private boolean mIsUnsolCallListPresent = false;

    private boolean mWfcRoamingConfigurationSupport = false;

    // I'd rather this be LinkedList or something
    ArrayList<IFRequest> mRequestsList = new ArrayList<IFRequest>();

    // When we are testing emergency calls
    AtomicBoolean mTestingEmergencyCall = new AtomicBoolean(false);

    // The death recepient object which gets notified when IImsRadio service dies.
    final ImsRadioDeathRecipient mDeathRecipient;

    // Notification object used to listen to the start of the IImsRadio
    private final ImsRadioServiceNotification mServiceNotification =
        new ImsRadioServiceNotification();

    // ***** Constants
    static final int ZERO_SECONDS = 0;

    // Names of the Ims Radio services appended with slot id (for multiple sims)
    static final String[] IIMS_RADIO_SERVICE_NAME = {"imsradio0", "imsradio1", "imsradio2"};

    private Registrant mIncomingSmsRegistrant;
    private Registrant mSendSmsStatusReportRegistrant;
    private RegistrantList mHandoverStatusRegistrants;
    private RegistrantList mRefreshConfInfoRegistrations;
    private RegistrantList mSrvStatusRegistrations;
    private RegistrantList mTtyStatusRegistrants;
    private RegistrantList mRadioStateRegistrations;
    private RegistrantList mGeolocationRegistrants;
    private RegistrantList mVoWiFiCallQualityRegistrants;
    private RegistrantList mSsaccRegistrants;
    private RegistrantList mVopsRegistrants;
    private RegistrantList mParticipantStatusRegistrants;
    private RegistrantList mImsSubConfigChangeRegistrants;
    private RegistrantList mRegistrationBlockStatusRegistrants;
    protected RegistrantList mModifyCallRegistrants;
    protected RegistrantList mMwiRegistrants;
    private RegistrantList mRttMessageRegistrants;
    private RegistrantList mRttModifyRegistrants;
    protected Registrant mSsnRegistrant;
    protected Registrant mSsIndicationRegistrant;
    private RegistrantList mAutoRejectRegistrants;
    private RegistrantList mVoiceInfoStatusRegistrants;
    private RegistrantList mMultiIdentityStatusChangeRegistrants;
    private RegistrantList mMultiIdentityInfoPendingRegistrants;
    private RegistrantList mWfcRoamingModeConfigRegistrants;
    private RegistrantList mUssdInfoRegistrants;

    private List<ImsRadioServiceListener> mListeners = new CopyOnWriteArrayList<>();

    // Synchronization object of HAL interfaces.
    private final Object mHalSync = new Object();

    /* Object of the type ImsRadioResponse which is registered with the IImsRadio
     * service for all callbacks to be routed back */
    private IImsRadioResponse mImsRadioResponse;

    /* Object of the type ImsRadioIndication which is registered with the IImsRadio
     * service for all unsolicited messages to be sent*/
    private IImsRadioIndication mImsRadioIndication;

    /*  Instance of the IImsRadio interface */
    private volatile vendor.qti.hardware.radio.ims.V1_0.IImsRadio mImsRadio;

    /*Used by Sms over Ims functions*/
    private volatile vendor.qti.hardware.radio.ims.V1_2.IImsRadio mImsRadioV12;

    private volatile vendor.qti.hardware.radio.ims.V1_3.IImsRadio mImsRadioV13;
    private volatile vendor.qti.hardware.radio.ims.V1_4.IImsRadio mImsRadioV14;
    private volatile vendor.qti.hardware.radio.ims.V1_5.IImsRadio mImsRadioV15;
    private volatile vendor.qti.hardware.radio.ims.V1_6.IImsRadio mImsRadioV16;

    public interface ImsRadioServiceListener {
        void onServiceUp(int phoneId);
        void onServiceDown(int phoneId);
    }

    /**
     * Registers listener.
     * @param listener Listener to be registered
     * @throws IllegalArgumentException Will throw an error if listener is null.
     * @see ImsSubController#Listener
     */
    public void registerListener(ImsRadioServiceListener listener) {
        if (listener == null) {
            throw new IllegalArgumentException("listener is null");
        }
        synchronized(mListeners) {
            if (!mListeners.contains(listener)) {
                mListeners.add(listener);

                if (imsRadioV10() != null) {
                    listener.onServiceUp(mPhoneId);
                } else {
                    listener.onServiceDown(mPhoneId);
                }
            } else {
                Log.e(this, "Duplicate listener " + listener);
            }
        }
    }

    /**
     * Unregisters listener.
     * @param listener Listener to unregister
     * @see ImsSubContrller#Listener
     * @throws IllegalArgumentException Will throw an error if listener is null.
     */
    public void unregisterListener(ImsRadioServiceListener listener) {
        if (listener == null) {
            throw new IllegalArgumentException("listener is null");
        }
        synchronized(mListeners) {
            if (mListeners.contains(listener)) {
                mListeners.remove(listener);
            } else {
                Log.e(this, "Listener not found " + listener);
            }
        }
    }

    private void notifyServiceUp(String instanceName) {
        Log.i(this, "onServiceUp : " + instanceName);
        synchronized (mListeners) {
            for (ImsRadioServiceListener l : mListeners) {
                l.onServiceUp(mPhoneId);
            }
        }
    }

    private void notifyServiceDown(String instanceName) {
        Log.i(this, "onServiceDown : " + instanceName);
        synchronized(mListeners) {
            for (ImsRadioServiceListener l : mListeners) {
                l.onServiceDown(mPhoneId);
            }
        }
    }

    /**
     * Class that implements the binder death recipeint to be notified when
     * IImsRadio service dies.
     */
    final class ImsRadioDeathRecipient implements HwBinder.DeathRecipient {
        /**
         * Callback that gets called when the service has died
         */
        @Override
        public void serviceDied(long cookie) {
            Log.e(this, " IImsRadio Died");
            resetServiceAndRequestList();
        }
    }

    private synchronized void resetServiceAndRequestList() {
        notifyServiceDown(IIMS_RADIO_SERVICE_NAME[mPhoneId]);
        clearRequestsList(ImsErrorCode.RADIO_NOT_AVAILABLE, false);
        resetHalInterfaces();
    }

    private void resetHalInterfaces() {
        Log.d(this, "resetHalInterfaces: Resetting HAL interfaces.");
        synchronized (mHalSync) {
            mImsRadio = null;
            mImsRadioV12 = null;
            mImsRadioV13 = null;
            mImsRadioV14 = null;
            mImsRadioV15 = null;
            mImsRadioV16 = null;
            mImsRadioResponse = null;
            mImsRadioIndication = null;
        }
    }


    /**
     * Class that implements the service notification which gets called once the
     * service with fully qualified name fqName has started
     */
    final class ImsRadioServiceNotification extends IServiceNotification.Stub {
        /**
         * Callback that gets called when the service has registered
         * @param fqName - Fully qualified name of the service
         * @param name - Name of the service
         * @param preexisting - if the registration is preexisting
         */
        @Override
        public void onRegistration(String fqName, String name, boolean preexisting) {
            Log.v(this, "Ims Radio interface service started " + fqName + " " + name +
                " preexisting =" + preexisting);
                initImsRadio();
        }
    }

    /**
     * Register for notification when the
     * vendor.qti.hardware.radio.ims@1.0::IImsRadio is registered
     */
    private void registerForImsRadioServiceNotification() {
        try {
            final boolean ret = IServiceManager.getService()
                    .registerForNotifications("vendor.qti.hardware.radio.ims@1.0::IImsRadio",
                    IIMS_RADIO_SERVICE_NAME[mPhoneId], mServiceNotification);
            if (!ret) {
                Log.e(this, "Failed to register for service start notifications");
            }
        } catch (RemoteException ex) {
            Log.e(this, "Failed to register for service start notifications. Exception " + ex);
        }
    }

    private vendor.qti.hardware.radio.ims.V1_0.IImsRadio imsRadioV10() {
        vendor.qti.hardware.radio.ims.V1_0.IImsRadio imsRadio = null;
        synchronized (mHalSync) {
            imsRadio = mImsRadio;
        }
        return imsRadio;
    }

    private vendor.qti.hardware.radio.ims.V1_2.IImsRadio imsRadioV12() {
        vendor.qti.hardware.radio.ims.V1_2.IImsRadio imsRadio = null;
        synchronized (mHalSync) {
            imsRadio = mImsRadioV12;
        }
        return imsRadio;
    }

    private vendor.qti.hardware.radio.ims.V1_3.IImsRadio imsRadioV13() {
        vendor.qti.hardware.radio.ims.V1_3.IImsRadio imsRadio = null;
        synchronized (mHalSync) {
            imsRadio = mImsRadioV13;
        }
        return imsRadio;
    }

    private vendor.qti.hardware.radio.ims.V1_4.IImsRadio imsRadioV14() {
        vendor.qti.hardware.radio.ims.V1_4.IImsRadio imsRadio = null;
        synchronized (mHalSync) {
            imsRadio = mImsRadioV14;
        }
        return imsRadio;
    }

    private vendor.qti.hardware.radio.ims.V1_5.IImsRadio imsRadioV15() {
        vendor.qti.hardware.radio.ims.V1_5.IImsRadio imsRadio = null;
        synchronized (mHalSync) {
            imsRadio = mImsRadioV15;
        }
        return imsRadio;
    }

    private vendor.qti.hardware.radio.ims.V1_6.IImsRadio imsRadioV16() {
        vendor.qti.hardware.radio.ims.V1_6.IImsRadio imsRadio = null;
        synchronized (mHalSync) {
            imsRadio = mImsRadioV16;
        }
        return imsRadio;
    }

    private static String toAvailability(boolean v) {
        return v ? "available" : "unavailable";
    }

    public boolean isSmsSupported() {
        return mImsRadioV12 != null;
    }

    /**
     * Initialize the instance of IImsRadio. Get the service and register the callback object
     * to be called for the responses to the solicited and unsolicited requests.
     */
    private void initImsRadio() {
        try {
            vendor.qti.hardware.radio.ims.V1_0.IImsRadio imsRadio =
                    IImsRadio.getService(IIMS_RADIO_SERVICE_NAME[mPhoneId]);
            if (imsRadio == null) {
                resetHalInterfaces();
                Log.e(this, "initImsRadio: imsRadio is null.");
                return;
            }
            Log.i(this, "initImsRadio: imsRadioV10 availability: " + toAvailability(imsRadio !=
                    null));

            vendor.qti.hardware.radio.ims.V1_1.IImsRadio imsRadioV11 =
                    vendor.qti.hardware.radio.ims.V1_1.IImsRadio.castFrom(imsRadio);
            Log.i(this, "initImsRadio: imsRadioV11 availability: " + toAvailability(imsRadioV11
                    != null));

            vendor.qti.hardware.radio.ims.V1_2.IImsRadio imsRadioV12 =
                    vendor.qti.hardware.radio.ims.V1_2.IImsRadio.castFrom(imsRadio);
            Log.i(this, "initImsRadio: imsRadioV12 availability: " + toAvailability(imsRadioV12
                    != null));

            vendor.qti.hardware.radio.ims.V1_3.IImsRadio imsRadioV13 =
                    vendor.qti.hardware.radio.ims.V1_3.IImsRadio.castFrom(imsRadio);
            Log.i(this, "initImsRadio: imsRadioV13 availability: " + toAvailability(imsRadioV13
                    != null));

            vendor.qti.hardware.radio.ims.V1_4.IImsRadio imsRadioV14 =
                    vendor.qti.hardware.radio.ims.V1_4.IImsRadio.castFrom(imsRadio);
            Log.i(this, "initImsRadio: imsRadioV14 availability: " + toAvailability(imsRadioV14
                    != null));

            vendor.qti.hardware.radio.ims.V1_5.IImsRadio imsRadioV15 =
                    vendor.qti.hardware.radio.ims.V1_5.IImsRadio.castFrom(imsRadio);
            Log.i(this, "initImsRadio: imsRadioV15 availability: " + toAvailability(imsRadioV15
                    != null));

            vendor.qti.hardware.radio.ims.V1_6.IImsRadio imsRadioV16 =
                    vendor.qti.hardware.radio.ims.V1_6.IImsRadio.castFrom(imsRadio);
            Log.i(this, "initImsRadio: imsRadioV16 availability: " + toAvailability(imsRadioV16
                    != null));

            ImsRadioResponse imsRadioResponse = new ImsRadioResponse();
            ImsRadioIndication imsRadioIndication = new ImsRadioIndication();

            synchronized (mHalSync) {
                mImsRadioResponse = imsRadioResponse;
                mImsRadioIndication = imsRadioIndication;
                imsRadio.setCallback(mImsRadioResponse, mImsRadioIndication);
                imsRadio.linkToDeath(mDeathRecipient, 0 /* Not Used */);

                mImsRadio = imsRadio;
                mImsRadioV12 = imsRadioV12;
                mImsRadioV13 = imsRadioV13;
                mImsRadioV14 = imsRadioV14;
                mImsRadioV15 = imsRadioV15;
                mImsRadioV16 = imsRadioV16;
            }

            notifyServiceUp(IIMS_RADIO_SERVICE_NAME[mPhoneId]);

        } catch (Exception ex) {
            Log.e(this, "initImsRadio: Exception: " + ex);
            resetServiceAndRequestList();
        }
    }

    public void registerForImsSubConfigChanged(Handler h, int what, Object obj) {
        mImsSubConfigChangeRegistrants.addUnique(h, what, obj);
    }

    public void deregisterForImsSubConfigChanged(Handler h) {
        mImsSubConfigChangeRegistrants.remove(h);
    }

    public void registerForSsacStatusChanged(Handler h, int what, Object obj) {
        Registrant r = new WakeLockRegistrant(h, what, obj, mNotifyWakeLock);
        mSsaccRegistrants.add(r);
    }

    public void unregisterForSsacStatusChanged(Handler h) {
        mSsaccRegistrants.remove(h);
    }

    public void registerForVopsStatusChanged(Handler h, int what, Object obj) {
        Registrant r = new WakeLockRegistrant(h, what, obj, mNotifyWakeLock);
        mVopsRegistrants.add(r);
    }

    public void unregisterForVopsStatusChanged(Handler h) {
        mVopsRegistrants.remove(h);
    }

    public void registerForWfcRoamingModeFeatureSupport(Handler h, int what, Object obj) {
        Registrant r = new WakeLockRegistrant(h, what, obj, mNotifyWakeLock);
        mWfcRoamingModeConfigRegistrants.add(r);
        mWfcRoamingModeConfigRegistrants.notifyRegistrants(new AsyncResult(null,
                mWfcRoamingConfigurationSupport, null));
    }

    public void unRegisterForWfcRoamingModeFeatureSupport(Handler h) {
        mWfcRoamingModeConfigRegistrants.remove(h);
    }

    public void registerForHandoverStatusChanged(Handler h, int what, Object obj) {
        Registrant r = new WakeLockRegistrant(h, what, obj, mNotifyWakeLock);
        mHandoverStatusRegistrants.add(r);
    }

    public void unregisterForHandoverStatusChanged(Handler h) {
        mHandoverStatusRegistrants.remove(h);
    }

    public void registerForRefreshConfInfo(Handler h, int what, Object obj) {
        Registrant r = new WakeLockRegistrant(h, what, obj, mNotifyWakeLock);
        mRefreshConfInfoRegistrations.add(r);
    }

    public void unregisterForRefreshConfInfo(Handler h) {
        mRefreshConfInfoRegistrations.remove(h);
    }

    public void registerForSrvStatusUpdate(Handler h, int what, Object obj) {
        Registrant r = new WakeLockRegistrant(h, what, obj, mNotifyWakeLock);
        mSrvStatusRegistrations.add(r);
    }

    public void unregisterForSrvStatusUpdate(Handler h) {
        mSrvStatusRegistrations.remove(h);
    }

    public void registerForTtyStatusChanged(Handler h, int what, Object obj) {
        Registrant r = new WakeLockRegistrant(h, what, obj, mNotifyWakeLock);
        mTtyStatusRegistrants.add(r);
    }

    public void unregisterForTtyStatusChanged(Handler h) {
        mTtyStatusRegistrants.remove(h);
    }

    public void registerForGeolocationRequest(Handler h, int what, Object obj) {
        Registrant r = new WakeLockRegistrant(h, what, obj, mNotifyWakeLock);
        mGeolocationRegistrants.add(r);
    }

    public void unregisterForGeolocationRequest(Handler h) {
        mGeolocationRegistrants.remove(h);
    }

    public void registerForVoWiFiCallQualityUpdate(Handler h, int what, Object obj) {
        Registrant r = new WakeLockRegistrant(h, what, obj, mNotifyWakeLock);
        mVoWiFiCallQualityRegistrants.add(r);
    }

    public void unregisterForVoWiFiCallQualityUpdate(Handler h) {
        mVoWiFiCallQualityRegistrants.remove(h);
    }

    public void registerForParticipantStatusInfo(Handler h, int what, Object obj) {
        Registrant r = new WakeLockRegistrant(h, what, obj, mNotifyWakeLock);
        mParticipantStatusRegistrants.add(r);
    }

    public void unregisterForParticipantStatusInfo(Handler h) {
        mParticipantStatusRegistrants.remove(h);
    }

    public void registerForRegistrationBlockStatus(Handler h, int what, Object obj) {
        Registrant r = new WakeLockRegistrant(h, what, obj, mNotifyWakeLock);
        mRegistrationBlockStatusRegistrants.add(r);
    }

    public void unregisterForRegistrationBlockStatus(Handler h) {
        mRegistrationBlockStatusRegistrants.remove(h);
    }

    public void setOnSuppServiceNotification(Handler h, int what, Object obj) {
        mSsnRegistrant = new WakeLockRegistrant(h, what, obj, mNotifyWakeLock);
    }

    public void unSetOnSuppServiceNotification(Handler h) {
        mSsnRegistrant.clear();
    }

    public void registerForSuppServiceIndication(Handler h, int what, Object obj) {
        mSsIndicationRegistrant = new Registrant (h, what, obj);
    }

    public void unregisterForSuppServiceIndication(Handler h) {
        mSsIndicationRegistrant.clear();
    }

    public void registerForUssdInfo(Handler h, int what, Object obj) {
        Registrant r = new WakeLockRegistrant(h, what, obj, mNotifyWakeLock);
        mUssdInfoRegistrants.add(r);
    }

    public void unregisterForUssdInfo(Handler h) {
        mUssdInfoRegistrants.remove(h);
    }

    public void setOnIncomingImsSms(Handler h, int what, Object obj){
        mIncomingSmsRegistrant = new Registrant (h, what, obj);
    }

    public void unSetIncomingImsSms(Handler h){
        if (mIncomingSmsRegistrant != null) {
            mIncomingSmsRegistrant.clear();
            mIncomingSmsRegistrant = null;
        }
    }

    public void setOnImsSmsStatusReport(Handler h, int what, Object obj){
        mSendSmsStatusReportRegistrant = new Registrant(h, what, obj);
    }

    public void unSetImsSmsStatusReport(Handler h){
        if (mSendSmsStatusReportRegistrant != null) {
            mSendSmsStatusReportRegistrant.clear();
            mSendSmsStatusReportRegistrant = null;
        }
    }

    public void registerForCallAutoRejection(Handler h, int what, Object obj) {
        Registrant r = new WakeLockRegistrant(h, what, obj, mNotifyWakeLock);
        mAutoRejectRegistrants.add(r);
    }

    public void unregisterForCallAutoRejection(Handler h) {
        mAutoRejectRegistrants.remove(h);
    }

    public void registerForMultiIdentityRegistrationStatusChange(
            Handler h, int what, Object obj) {
        Registrant r = new WakeLockRegistrant(h, what, obj, mNotifyWakeLock);
        mMultiIdentityStatusChangeRegistrants.add(r);
    }

    public void registerForMultiIdentityInfoPendingIndication(
            Handler h, int what, Object obj) {
        Registrant r = new WakeLockRegistrant(h, what, obj, mNotifyWakeLock);
        mMultiIdentityInfoPendingRegistrants.add(r);
    }

    public void unregisterForMultiIdentityRegistrationStatusChange(Handler h) {
        mMultiIdentityStatusChangeRegistrants.remove(h);
    }

    public void unregisterForMultiIdentityPendingInfoRequest(Handler h) {
        mMultiIdentityInfoPendingRegistrants.remove(h);
    }

    @VisibleForTesting
    public Registrant getIncomingSmsRegistrant(){
        return mIncomingSmsRegistrant;
    }

    @VisibleForTesting
    public Registrant getSendSmsStatusReportRegistrant(){
        return mSendSmsStatusReportRegistrant;
    }

    class ImsRadioHandler extends Handler {
        // ***** Handler implementation
        @Override
        public void handleMessage(Message msg) {
            IFRequest rr = (IFRequest) (msg.obj);

            switch (msg.what) {
                case EVENT_WAKE_LOCK_TIMEOUT:
                    /**
                     * Haven't heard back from the last request. Assume we're not getting a
                     * response and release the wake lock.
                     * The timer of WAKE_LOCK_TIMEOUT is reset with each new send request. So when
                     * WAKE_LOCK_TIMEOUT occurs all requests in mRequestList already waited at
                     * least DEFAULT_WAKE_LOCK_TIMEOUT but no response.
                     * Reset mRequestMessagesWaiting to enable releaseWakeLockIfDone().
                     *
                     * Note: Keep mRequestList so that delayed response can still be handled when
                     * response finally comes.
                     */
                    synchronized (mWakeLock) {
                        if (mWakeLock.isHeld()) {

                            if (mRequestMessagesWaiting != 0) {
                                Log.i(this, "Number of messages still waiting for response "
                                        + mRequestMessagesWaiting + " at TIMEOUT. Reset to 0");

                                mRequestMessagesWaiting = 0;

                                synchronized (mRequestsList) {
                                    int count = mRequestsList.size();
                                    Log.i(this, "WAKE_LOCK_TIMEOUT " +
                                            " mRequestList=" + count);

                                    for (int i = 0; i < count; i++) {
                                        rr = mRequestsList.get(i);
                                        Log.i(this, i + ": [" + rr.mSerial + "] "
                                                + msgIdToString(rr.mRequest));
                                    }
                                }
                            }
                            mWakeLock.release();
                        }
                    }
                    break;
            }
        }
    }

    /**
     * Class implementing all callbacks defined in IImsRadioResponse for getting responses from
     * the RIL for the requests sent.
     *
     */
    class ImsRadioResponse extends vendor.qti.hardware.radio.ims.V1_6.IImsRadioResponse.Stub {

        /**
         * Callback for getting the response to the DIAL request sent to the RIL via the
         * IImsRadio interface
         * @param token to match request/response. Response must include same token as request
         * @param errorCode of type ImsErrorCode.Error send back from RIL for the dial request
         *
         */
        @Override
        public void dialResponse(int token, int errorCode) {
            // TODO: Map proto error codes to IImsRadio error codes to be used by the interface.
            // Change usage of errors of type ImsErrorCode.Error to some proprietary error code
            // and return that to clients.
            removeFromQueueAndSendResponse(token, errorCode);
        }

        /**
         * Callback for getting the response to the sendsms request sent to the RIL via the
         * IImsRadio interface
         * @param token to match request/response. Response must include same token as request
         * @param messageRef reference of the sent message.
         * @param smsStatusResult to check status of the sent message..
         */
        @Override
        public void sendImsSmsResponse_1_5(int token, int messageRef, int smsStatusResult,
                int reason) {
            Log.i(this, "Ims sms response received");
            IFRequest rr = findAndRemoveRequestFromList(token);
            if (rr == null) {
                return;
            }

            SmsResponse response = ImsRadioUtils.imsSmsResponsefromHidl(messageRef,
                    smsStatusResult, reason);
            sendResponse(rr,ImsErrorCode.SUCCESS,response);
        }

        /**
         * Callback for getting the response to the sendsms request sent to the RIL via the
         * IImsRadio interface
         * @param token to match request/response. Response must include same token as request
         * @param messageRef reference of the sent message.
         * @param smsStatusResult to check status of the sent message..
         */
        @Override
        public void sendImsSmsResponse(int token, int messageRef, int smsStatusResult, int reason) {
            sendImsSmsResponse_1_5( token, messageRef, smsStatusResult, reason);
        }

        @Override
        public void setConfigResponse(int token, int errorCode, ConfigInfo config) {
                setConfigResponse_1_6(token, errorCode,
                        ImsRadioUtilsV16.migrateConfigInfoFrom(config));
        }

        @Override
        public void setConfigResponse_1_6(int token, int errorCode,
                vendor.qti.hardware.radio.ims.V1_6.ConfigInfo config) {
            Log.i(this, "Set config response received.");
            IFRequest rr = findAndRemoveRequestFromList(token);
            if (rr == null) {
                return;
            }

            Object ret = processConfigResponse(config);
            sendResponse(rr, errorCode, ret);
        }

        @Override
        public void getConfigResponse(int token, int errorCode, ConfigInfo config) {
            Log.i(this, "Get config response received.");
            IFRequest rr = findAndRemoveRequestFromList(token);
            if(rr == null) {
                return;
            }

            Object ret = processConfigResponse(ImsRadioUtilsV16.migrateConfigInfoFrom(config));
            sendResponse(rr, errorCode, ret);
        }

        @Override
        public void queryServiceStatusResponse(int token, int errorCode,
            ArrayList<ServiceStatusInfo> serviceStatusInfoList) {
            Log.i(this, "queryServiceStatusResponse()");
            queryServiceStatusResponse_1_6(token, errorCode,
                    ImsRadioUtilsV16.migrateServiceStatusInfo(serviceStatusInfoList));
        }

        @Override
        public void queryServiceStatusResponse_1_6(int token, int errorCode,
                ArrayList<vendor.qti.hardware.radio.ims.V1_6.ServiceStatusInfo> infoList) {
            Log.i(this, "queryServiceStatusResponse_1_6()");
            IFRequest rr = findAndRemoveRequestFromList(token);
            if(rr == null) {
                return;
            }

            Object ret = null;
            if(infoList != null) {
                ret = ImsRadioUtils.handleSrvStatus(infoList);
            }
            sendResponse(rr, errorCode, ret);
        }

        @Override
        public void setServiceStatusResponse(int token, int errorCode) {
            Log.i(this, "SetServiceStatus response received.");
            IFRequest rr = findAndRemoveRequestFromList(token);
            if(rr == null) {
                Log.e(this, "SetServiceStatusResponsse rr is NULL");
                return;
            }

            try {
                SomeArgs setCapArgs = (SomeArgs)rr.mResult.obj;
                Message orgMsg = (Message)setCapArgs.arg1;
                /* On getting response from RIL, retreive the data stored in IFRequest#mResult,
                 * and reply back to sender with retreived data and with original message that
                 * sender is expecting
                 */
                rr.setResult(orgMsg);
                sendResponse(rr, errorCode, (ArrayList<CapabilityStatus>) setCapArgs.arg2);
            } catch (ClassCastException ex) {
                Log.e(this, "setServiceStatusResponse exception = " + ex);
            }
        }

        private void sendSipErrorInfo(int token, int errorCode, SipErrorInfo errorInfo) {
            IFRequest rr = findAndRemoveRequestFromList(token);
            if(rr == null) {
                return;
            }

            ImsReasonInfo sipErrorInfo = ImsRadioUtils.sipErrorFromHal(errorInfo);
            Log.i(this, msgIdToString(rr.mRequest) + " response - sip error code: " +
                errorInfo.errorCode + " sip error string: " + errorInfo.errorString);
            sendResponse(rr, errorCode, sipErrorInfo);
        }

        @Override
        public void resumeResponse(int token, int errorCode, SipErrorInfo errorInfo) {
            sendSipErrorInfo(token, errorCode, errorInfo);
        }

        @Override
        public void holdResponse(int token, int errorCode, SipErrorInfo errorInfo) {
            sendSipErrorInfo(token, errorCode, errorInfo);
        }

        @Override
        public void hangupResponse(int token, int errorCode) {
            Log.i(this, "Got hangup response from ImsRadio.");
            removeFromQueueAndSendResponse(token, errorCode);
        }

        @Override
        public void answerResponse(int token, int errorCode) {
            Log.i(this, "Got answer response from ImsRadio.");
            removeFromQueueAndSendResponse(token, errorCode);
        }

        @Override
        public void requestRegistrationChangeResponse(int token, int errorCode) {
            Log.i(this, "Got registration change response from ImsRadio.");
            removeFromQueueAndSendResponse(token, errorCode);
        }

        @Override
        public void getRegistrationResponse(int token, int errorCode,
                RegistrationInfo registration) {
            Log.i(this, "getRegistrationResponse()");
            getRegistrationResponse_1_6(token, errorCode,
                    ImsRadioUtilsV16.migrateRegistrationInfo(registration));
        }

        @Override
        public void getRegistrationResponse_1_6(int token, int errorCode,
                vendor.qti.hardware.radio.ims.V1_6.RegistrationInfo registration) {
            Log.i(this, "getRegistrationResponse_1_6()");
            IFRequest rr = findAndRemoveRequestFromList(token);
            if(rr == null) {
                return;
            }

            ImsRegistrationInfo regMessage = null;
            if (registration != null) {
                regMessage = ImsRadioUtils.registrationFromHal(registration);
            }
            sendResponse(rr, errorCode, regMessage);
        }

        @Override
        public void suppServiceStatusResponse_1_3(int token, int errorCode,
            vendor.qti.hardware.radio.ims.V1_3.SuppServiceStatus suppServiceStatus) {
            IFRequest rr = findAndRemoveRequestFromList(token);
            if (rr == null) {
                return;
            }

            Log.i(this, "Response to " + msgIdToString(rr.mRequest) +
                " status:" + suppServiceStatus.status +
                " provisionStatus:" + suppServiceStatus.provisionStatus +
                " facilityType:" + suppServiceStatus.facilityType +
                " failureCause:" + suppServiceStatus.failureCause +
                " errorCode:" + suppServiceStatus.errorDetails.errorCode);

            SuppSvcResponse suppSvcResponse =
                ImsRadioUtils.suppSvcStatusResponseFromHal(suppServiceStatus);
            if (rr.mRequest == MessageId.REQUEST_SET_CALL_FORWARD_STATUS) {
                CallForwardStatusInfo cfStatusInfo = new CallForwardStatusInfo(
                        suppSvcResponse.getErrorDetails(), null);
                sendResponse(rr, errorCode, cfStatusInfo);
            } else {
                sendResponse(rr, errorCode, suppSvcResponse);
            }
        }

        @Override
        public void suppServiceStatusResponse(int token, int errorCode,
            SuppServiceStatus suppServiceStatus) {
            vendor.qti.hardware.radio.ims.V1_3.SuppServiceStatus status =
                    ImsRadioUtilsV13.migrateSuppServiceStatusFromV10(suppServiceStatus);
            suppServiceStatusResponse_1_3(token, errorCode, status);
        }

        @Override
        public void conferenceResponse(int token, int errorCode, SipErrorInfo errorInfo) {
            sendSipErrorInfo(token, errorCode, errorInfo);
        }

        @Override
        public void getClipResponse(int token, int errorCode,
            ClipProvisionStatus clipProvisionStatus) {
            IFRequest rr = findAndRemoveRequestFromList(token);
            if (rr == null) {
                return;
            }

            SuppService clipProvStatus = new SuppService();

            final int clipStatus = clipProvisionStatus.clipStatus;
            if (clipStatus != ClipStatus.INVALID) {
                clipProvStatus.setStatus(ImsRadioUtils.clipStatusFromHal(
                    clipProvisionStatus.clipStatus));
                Log.i(this, "getClipResponse from ImsRadio. Clipstatus " + clipProvisionStatus);
            }

            if (clipProvisionStatus.hasErrorDetails) {
                clipProvStatus.setErrorDetails(ImsRadioUtils.sipErrorFromHal(
                    clipProvisionStatus.errorDetails));
                Log.i(this, "getClipResponse from ImsRadio. Errorcode " +
                    clipProvisionStatus.errorDetails.errorCode + "String " +
                    clipProvisionStatus.errorDetails.errorString);
            }

            sendResponse(rr, errorCode, clipProvStatus);
        }

        @Override
        public void getClirResponse(int token, int errorCode, ClirInfo clirInfo,
            boolean hasClirInfo) {
            IFRequest rr = findAndRemoveRequestFromList(token);
            if (rr == null) {
                return;
            }

            int[] response = null;

            if (hasClirInfo) {
                response = new int[2];
                if (clirInfo.paramN != Integer.MAX_VALUE) {
                    response[0] = clirInfo.paramN;
                }
                if (clirInfo.paramM != Integer.MAX_VALUE) {
                    response[1] = clirInfo.paramM;
                }
                Log.i(this, "getClirResponse from ImsRadio. param_m - " + clirInfo.paramM +
                    "param_n - " + clirInfo.paramN);
            }

            sendResponse(rr, errorCode, response);
        }

        @Override
        public void queryCallForwardStatusResponse(int token, int errorCode,
                ArrayList<vendor.qti.hardware.radio.ims.V1_0.CallForwardInfo> callForwardInfoList,
                SipErrorInfo errorDetails) {
            IFRequest rr = findAndRemoveRequestFromList(token);
            if(rr == null) {
                return;
            }

            Object ret = ImsRadioUtils.buildCFStatusResponseFromHal(callForwardInfoList);
            sendResponse(rr, errorCode, ret);
        }

        @Override
        public void getCallWaitingResponse(int token, int errorCode, int inServiceStatus,
                int serviceClass, SipErrorInfo errorDetails) {
            IFRequest rr = findAndRemoveRequestFromList(token);
            if(rr == null) {
                return;
            }
            int[] response = null;

            if (inServiceStatus != ServiceClassStatus.INVALID) {
                int outServiceStatus = ImsRadioUtils.serviceClassStatusFromHal(inServiceStatus);
                if (outServiceStatus == SuppSvcResponse.DISABLED) {
                    response = new int[1];
                    response[0] = SuppSvcResponse.DISABLED;
                } else {
                    response = new int[2];
                    response[0] = SuppSvcResponse.ENABLED;
                    response[1] = serviceClass;
                }
            }

            sendResponse(rr, errorCode, response);
        }

        @Override
        public void setClirResponse(int token, int errorCode) {
            Log.i(this, "Got setClirResponse from ImsRadio. error " + errorCode);
            removeFromQueueAndSendResponse(token, errorCode);
        }

        @Override
        public void getColrResponse_1_3(int token, int errorCode,
                vendor.qti.hardware.radio.ims.V1_3.ColrInfo colrInfo) {
            IFRequest rr = findAndRemoveRequestFromList(token);
            if(rr == null) {
                return;
            }

            SuppService colrValue = new SuppService();

            colrValue.setServiceClassStatus(
                    ImsRadioUtils.serviceClassStatusFromHal(colrInfo.status));
            colrValue.setProvisionStatus(ImsRadioUtils.serviceClassProvisionStatusFromHal
                    (colrInfo.provisionStatus));
            colrValue.setStatus(
                    ImsRadioUtils.serviceClassStatusFromHal(colrInfo.status));

            final int presentation = ImsRadioUtils.ipPresentationFromHal(colrInfo.presentation);
            if (presentation != IpPresentation.IP_PRESENTATION_INVALID) {
                colrValue.setPresentation(presentation);
                Log.i(this, "getColrResponse from ImsRadio. presentation " + presentation);
            }

            if (colrInfo.hasErrorDetails) {
                colrValue.setErrorDetails(ImsRadioUtils.sipErrorFromHal(colrInfo.errorDetails));
                 Log.i(this, "getColrResponse from ImsRadio. errorcode " +
                     colrInfo.errorDetails.errorCode + "string " +
                     colrInfo.errorDetails.errorString);
            }

            sendResponse(rr, errorCode, colrValue);
        }

        @Override
        public void getColrResponse(int token, int errorCode, ColrInfo colrInfo) {
            vendor.qti.hardware.radio.ims.V1_3.ColrInfo ci =
                    ImsRadioUtilsV13.migrateColrInfoFromV10(colrInfo);
            getColrResponse_1_3(token, errorCode, ci);
        }

        @Override
        public void exitEmergencyCallbackModeResponse(int token, int errorCode) {
            Log.i(this, "Got exitEmergencyCallbackModeResponse from ImsRadio error " + errorCode);
            removeFromQueueAndSendResponse(token, errorCode);
        }

        @Override
        public void sendDtmfResponse(int token, int errorCode) {
            Log.i(this, "Got sendDtmfResponse from ImsRadio error " + errorCode);
            removeFromQueueAndSendResponse(token, errorCode);
        }

        @Override
        public void startDtmfResponse(int token, int errorCode) {
            Log.i(this, "Got startDtmfResponse from ImsRadio error " + errorCode);
            removeFromQueueAndSendResponse(token, errorCode);
        }

        @Override
        public void stopDtmfResponse(int token, int errorCode) {
            Log.i(this, "Got stopDtmfResponse from ImsRadio error " + errorCode);
            removeFromQueueAndSendResponse(token, errorCode);
        }

        @Override
        public void setUiTTYModeResponse(int token, int errorCode) {
            Log.i(this, "Got setUiTTYModeResponse from ImsRadio error " + errorCode);
            removeFromQueueAndSendResponse(token, errorCode);
        }

        @Override
        public void modifyCallInitiateResponse(int token, int errorCode) {
            Log.i(this, "Got modify call initiate response from ImsRadio.");
            removeFromQueueAndSendResponse(token, errorCode);
        }

        @Override
        public void cancelModifyCallResponse(int token, int errorCode) {
            Log.i(this, "Got cancel modify call response from ImsRadio.");
            removeFromQueueAndSendResponse(token, errorCode);
        }

        @Override
        public void modifyCallConfirmResponse(int token, int errorCode) {
            Log.i(this, "Got modify call confirm response from ImsRadio.");
            removeFromQueueAndSendResponse(token, errorCode);
        }

        @Override
        public void explicitCallTransferResponse(int token, int errorCode) {
            Log.i(this, "Got explicit call transfer response from ImsRadio.");
            removeFromQueueAndSendResponse(token, errorCode);
        }

        @Override
        public void setSuppServiceNotificationResponse(int token, int errorCode,
                int serviceStatusClass) {
            Log.i(this, "Got set supp service notification response from ImsRadio.");
            removeFromQueueAndSendResponse(token, errorCode);
        }

        @Override
        public void getRtpStatisticsResponse(int token, int errorCode,
                long packetCount) {
            Log.i(this, "Got getRtpStatisticsResponse from ImsRadio packetCount = " +
                     packetCount + " error " + errorCode);
            IFRequest rr = findAndRemoveRequestFromList(token);
            if(rr == null) {
                return;
            }
            sendResponse(rr, errorCode, packetCount);
        }

        @Override
        public void getRtpErrorStatisticsResponse(int token, int errorCode,
                long packetErrorCount) {
            Log.i(this, "Got getRtpErrorStatisticsResponse from ImsRadio packetErrorCount = " +
                    packetErrorCount + " error " + errorCode);
            IFRequest rr = findAndRemoveRequestFromList(token);
            if(rr == null) {
                return;
            }
            sendResponse(rr, errorCode, packetErrorCount);
        }

        @Override
        public void addParticipantResponse(int token, int errorCode) {
            // TODO: Map proto error codes to IImsRadio error codes to be used by the interface.
            // Change usage of errors of type ImsErrorCode.Error to some proprietary error code
            // and return that to clients.
            Log.i(this, "Add Participant response received. errorCode: " + errorCode);
            removeFromQueueAndSendResponse(token, errorCode);
        }

        @Override
        public void deflectCallResponse(int token, int errorCode) {
            Log.i(this, "Got deflect call response from ImsRadio.");
            removeFromQueueAndSendResponse(token, errorCode);
        }

        @Override
        public void sendGeolocationInfoResponse(int token, int errorCode) {
            Log.i(this, "Received GeoLocationInfo response from ImsRadio.");
            removeFromQueueAndSendResponse(token, errorCode);
        }

        @Override
        public void getImsSubConfigResponse(int token, int errorCode,
                ImsSubConfigInfo subConfigInfo) {
            Log.i(this, "Received Subconfig response from ImsRadio.");
            IFRequest rr = findAndRemoveRequestFromList(token);
            if (rr == null) {
                return;
            }
            Object ret = ImsRadioUtils.imsSubconfigFromHal(subConfigInfo);
            sendResponse(rr, errorCode, ret);
        }

        public void sendRttMessageResponse(int token, int errorCode) {
            Log.i(this, "Send Rtt Message response received. errorCode: " + errorCode);
            removeFromQueueAndSendResponse(token, errorCode);
        }

        public void registerMultiIdentityLinesResponse(int token, int errorCode) {
            Log.i(this, "registerMultiIdentityLines Response received. errorCode : " + errorCode);
            removeFromQueueAndSendResponse(token, errorCode);
        }

        public void queryVirtualLineInfoResponse(int token, String msisdn,
                ArrayList<String> virtualLineInfo) {
            IFRequest rr = findAndRemoveRequestFromList(token);
            if (rr == null) {
                return;
            }
            VirtualLineInfo virtualInfo = new VirtualLineInfo(msisdn, virtualLineInfo);
            Log.i(this, "queryVirtualLineInfoResponse :: " + virtualInfo);
            sendResponse(rr, ImsErrorCode.SUCCESS, virtualInfo);
        }

        public void setCallForwardStatusResponse(int token, int errorCode,
                vendor.qti.hardware.radio.ims.V1_5.CallForwardStatusInfo callForwardStatusInfo) {
            IFRequest rr = findAndRemoveRequestFromList(token);
            if (rr == null) {
                return;
            }

            CallForwardStatusInfo cfStatusInfo = ImsRadioUtilsV15.callForwardStatusInfoFromHal(
                    callForwardStatusInfo);
            Log.i(this, "setCallForwardStatusResponse :: " + cfStatusInfo);
            sendResponse(rr, errorCode, cfStatusInfo);
        }
    }

     /**
      * Class implementing all callbacks defined in IImsRadioIndication for getting indications
      * from RIL.
      *
      */
    class ImsRadioIndication extends vendor.qti.hardware.radio.ims.V1_6.IImsRadioIndication.Stub {

        @Override
        public void onCallStateChanged_1_6(
                ArrayList<vendor.qti.hardware.radio.ims.V1_6.CallInfo> callList) {
            Log.i(this, "onCallStateChanged_1_6()");
            if(callList == null) {
                Log.e(this, "Call list is null.");
                return;
            }

            ArrayList<DriverCallIms> response = new ArrayList<DriverCallIms>();
            int numOfCalls = callList.size();
            mIsUnsolCallListPresent = true;

            for(int i = 0; i < numOfCalls; ++i) {
                DriverCallIms dc = ImsRadioUtils.buildDriverCallImsFromHal(callList.get(i));
                // Check for an error message from the network.
                // If network sends a "Forbidden - Not authorized for service" string,
                // throw an intent. This intent is expected to be processed by OMA-DM
                // applications.
                if (dc.callFailCause.getExtraMessage() != null &&
                        dc.callFailCause.getExtraMessage().equalsIgnoreCase(
                        ImsReasonInfo.EXTRA_MSG_SERVICE_NOT_AUTHORIZED)) {
                    int subId = QtiImsExtUtils.getSubscriptionIdFromPhoneId(mContext, mPhoneId);
                    log("Throwing ACTION_FORBIDDEN_NO_SERVICE_AUTHORIZATION intent " +
                            "for subId " + subId);
                    Intent intent = new Intent(
                            TelephonyIntents.ACTION_FORBIDDEN_NO_SERVICE_AUTHORIZATION);
                    intent.putExtra(SubscriptionManager.EXTRA_SUBSCRIPTION_INDEX, subId);
                    intent.addFlags(Intent.FLAG_RECEIVER_REPLACE_PENDING);
                    mContext.sendBroadcast(intent);
                }
                if (dc.isVoicePrivacy) {
                    mVoicePrivacyOnRegistrants.notifyRegistrants();
                    log("InCall VoicePrivacy is enabled");
                } else {
                    mVoicePrivacyOffRegistrants.notifyRegistrants();
                    log("InCall VoicePrivacy is disabled");
                }
                response.add(dc);
            }

            Collections.sort(response);
            unsljLogRet(MessageId.UNSOL_RESPONSE_CALL_STATE_CHANGED, response);
            mCallStateRegistrants.notifyRegistrants(new AsyncResult(null, response, null));
        }

        @Override
        public void onCallStateChanged_1_5(
                ArrayList<vendor.qti.hardware.radio.ims.V1_5.CallInfo> callList) {
            Log.i(this, "onCallStateChanged_1_5()");
            onCallStateChanged_1_6(ImsRadioUtilsV16.migrateCallListFrom(callList));
        }

        @Override
        public void onCallStateChanged_1_4(ArrayList<
                vendor.qti.hardware.radio.ims.V1_4.CallInfo> callList) {
            Log.i(this, "onCallStateChanged_1_4()");
            onCallStateChanged_1_5(ImsRadioUtilsV15.migrateCallListFrom(callList));
        }

        @Override
        public void onCallStateChanged_1_3(ArrayList<
                vendor.qti.hardware.radio.ims.V1_3.CallInfo> callList) {
            Log.i(this, "onCallStateChanged_1_3()");
            onCallStateChanged_1_4(ImsRadioUtilsV14.migrateCallListFromV13(callList));
        }

        @Override
        public void onCallStateChanged_1_2(ArrayList<
                vendor.qti.hardware.radio.ims.V1_2.CallInfo> callList) {
            Log.i(this, "onCallStateChanged_1_2()");
            onCallStateChanged_1_3(ImsRadioUtilsV13.migrateCallListFromV12(callList));
        }

        @Override
        public void onCallStateChanged_1_1(ArrayList<
                vendor.qti.hardware.radio.ims.V1_1.CallInfo> callList) {
            Log.i(this, "onCallStateChanged_1_1()");
            onCallStateChanged_1_2(ImsRadioUtilsV12.migrateCallListFromV11(callList));
        }

        @Override
        public void onCallStateChanged(ArrayList<CallInfo> callList) {
            Log.i(this, "onCallStateChanged()");
            onCallStateChanged_1_1(ImsRadioUtils.migrateCallListToV11(callList));
        }

        @Override
        public void onImsSmsStatusReport(ImsSmsSendStatusReport smsStatusReport) {
            unsljLog(MessageId.UNSOL_IMS_SMS_STATUS_REPORT);

            if (smsStatusReport.pdu != null) {
                StatusReport smsReport = ImsRadioUtils.statusReportfromHidl(smsStatusReport);

                if(mSendSmsStatusReportRegistrant!=null) {
                    mSendSmsStatusReportRegistrant.notifyRegistrant(new AsyncResult(null,
                            smsReport, null));
                }
            }
        }

         @Override
         public void onIncomingImsSms(IncomingImsSms imsSms) {
             unsljLog(MessageId.UNSOL_INCOMING_IMS_SMS);

             if(imsSms.pdu!=null && imsSms.format!=null){
                 IncomingSms newSms = ImsRadioUtils.incomingSmsfromHidl(imsSms);
                 if(mIncomingSmsRegistrant!=null) {
                     mIncomingSmsRegistrant.notifyRegistrant(new AsyncResult(null, newSms, null));
                 }
             }
         }

        @Override
        public void onRing() {
            unsljLogRet(MessageId.UNSOL_CALL_RING, null);

            if (mRingRegistrant != null) {
                mRingRegistrant.notifyRegistrant(
                        new AsyncResult(null, null, null));
            }
        }

        @Override
        public void onRingbackTone(int tone) {
            int[] response = new int[1];
            log(" responseCallRingBack ");

            response[0] = ImsRadioUtils.ringbackToneFromHal(tone);
            log("responseCallRingBack " + response[0]);

            boolean playtone = false;
            unsljLogRet(MessageId.UNSOL_RINGBACK_TONE, response);
            if (response != null) playtone = (response[0] == 1);
            if (mRingbackToneRegistrants != null) {
                mRingbackToneRegistrants.notifyRegistrants(
                        new AsyncResult(null, playtone, null));
            }
        }


        @Override
        public void onRegistrationChanged(RegistrationInfo registration) {
            Log.i(this, "onRegistrationChanged()");
            onRegistrationChanged_1_6(ImsRadioUtilsV16.migrateRegistrationInfo(registration));
        }

        @Override
        public void onRegistrationChanged_1_6(
                vendor.qti.hardware.radio.ims.V1_6.RegistrationInfo registration) {
            Log.i(this, "onRegistrationChanged_1_6()");
            unsljLog(MessageId.UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED);
            ImsRegistrationInfo regMessage = ImsRadioUtils.registrationFromHal(registration);

            mImsNetworkStateChangedRegistrants.notifyRegistrants(
                    new AsyncResult(null, regMessage, null));
        }

        @Override
        public void onHandover(HandoverInfo inHandover) {
            Log.i(this, "onHandover()");
            onHandover_1_6(ImsRadioUtilsV16.migrateHandoverInfo(inHandover));
        }

        @Override
        public void onHandover_1_6(
                vendor.qti.hardware.radio.ims.V1_6.HandoverInfo inHandover) {
            Log.i(this, "onHandover_1_6()");
            HoInfo outHandover = ImsRadioUtils.handoverFromHal(inHandover);

            unsljLogRet(MessageId.UNSOL_RESPONSE_HANDOVER, outHandover);
            if (outHandover != null) {
                mHandoverStatusRegistrants.notifyRegistrants(
                        new AsyncResult(null, outHandover, null));
            }
        }

        @Override
        public void onServiceStatusChanged(ArrayList<ServiceStatusInfo> srvStatusList) {
            Log.i(this, "onServiceStatusChanged()");
            onServiceStatusChanged_1_6(ImsRadioUtilsV16.migrateServiceStatusInfo(srvStatusList));
        }

        @Override
        public void onServiceStatusChanged_1_6(
                ArrayList<vendor.qti.hardware.radio.ims.V1_6.ServiceStatusInfo> srvStatusList) {
            Log.i(this, "onServiceStatusChanged_1_6()");
            Object ret = ImsRadioUtils.handleSrvStatus(srvStatusList);
            unsljLogRet(MessageId.UNSOL_SRV_STATUS_UPDATE, ret);
            if (ret != null) {
                mSrvStatusRegistrations.notifyRegistrants(new AsyncResult(null, ret, null));
            }
        }

        @Override
        public void onRadioStateChanged(int radioState) {
            unsljLogRet(MessageId.UNSOL_RADIO_STATE_CHANGED, radioState);
            switch (radioState) {
                case vendor.qti.hardware.radio.ims.V1_0.RadioState.RADIO_STATE_OFF:
                    setRadioState(RadioState.RADIO_OFF);
                    break;
                case vendor.qti.hardware.radio.ims.V1_0.RadioState.RADIO_STATE_UNAVAILABLE:
                    setRadioState(RadioState.RADIO_UNAVAILABLE);
                    break;
                case vendor.qti.hardware.radio.ims.V1_0.RadioState.RADIO_STATE_ON:
                    setRadioState(RadioState.RADIO_ON);
                    break;
                default:
                    Log.e(this, "onRadioStateChanged: Invalid Radio State Change");
                    break;
            }
        }

        @Override
        public void onEnterEmergencyCallBackMode() {
            unsljLog(MessageId.UNSOL_ENTER_EMERGENCY_CALLBACK_MODE);
            if (mEmergencyCallbackModeRegistrant != null) {
                mEmergencyCallbackModeRegistrant.notifyRegistrant();
            }
        }

        @Override
        public void onExitEmergencyCallBackMode() {
            unsljLog(MessageId.UNSOL_EXIT_EMERGENCY_CALLBACK_MODE);
            if (mExitEmergencyCallbackModeRegistrants != null) {
                mExitEmergencyCallbackModeRegistrants.notifyRegistrants(
                    new AsyncResult(null, null, null));
            }
        }

        @Override
        public void onTtyNotification(TtyInfo ttyInfo) {
            int[] mode = null;
            if (ttyInfo != null) {
                mode = new int[1];
                mode[0] = ImsRadioUtils.ttyModeFromHal(ttyInfo.mode);
            }

            // Userdata is not being used currently
            byte[] ret;
            if (ttyInfo.userData != null && !ttyInfo.userData.isEmpty()) {
                ret = ImsRadioUtils.getByteArray(ttyInfo.userData);
            }
            unsljLogRet(MessageId.UNSOL_TTY_NOTIFICATION, mode);
            if (mode != null && mTtyStatusRegistrants != null) {
                mTtyStatusRegistrants.notifyRegistrants(new AsyncResult(null, mode, null));
            }
        }

        @Override
        public void onRefreshConferenceInfo(ConferenceInfo conferenceInfo) {
            if (conferenceInfo != null) {
                ConfInfo info = new ConfInfo();

            if (conferenceInfo.confInfoUri != null && !conferenceInfo.confInfoUri.isEmpty()) {
                info.setConfInfoUri(conferenceInfo.confInfoUri);
                Log.v(this, "onRefreshConferenceInfo: confUri = " +
                    conferenceInfo.confInfoUri);
            }

            if (conferenceInfo.conferenceCallState != ConferenceCallState.INVALID) {
                info.setConfCallState(ImsRadioUtils.conferenceCallStateFromHal(
                conferenceInfo.conferenceCallState));
            }
                Log.v(this, "onRefreshConferenceInfo: conferenceCallState = " +
                    conferenceInfo.conferenceCallState);
                unsljLogRet(MessageId.UNSOL_REFRESH_CONF_INFO, info);
                if (info != null) {
                    mRefreshConfInfoRegistrations.notifyRegistrants(
                        new AsyncResult(null, info, null));
                }
            }
        }

        @Override
        public void onRefreshViceInfo(ViceInfo viceInfo) {
            if (viceInfo != null && viceInfo.viceInfoUri != null) {
                ViceUriInfo info = new ViceUriInfo(viceInfo.viceInfoUri);
                // Notify the registrants in both the following cases :
                // 1) Valid Vice XML present
                // 2) Vice XML with 0 length. For ex: Modem triggers indication with 0 length
                // when APM toggle happens. Notify so that UI gets cleared
                Log.v(this, "onRefreshViceInfo: viceUri = " + viceInfo.viceInfoUri);
                unsljLogRet(MessageId.UNSOL_REFRESH_VICE_INFO, info);
                if (info != null) {
                    mRefreshViceInfoRegistrants
                        .notifyRegistrants(new AsyncResult(null, info, null));
                }
            }
        }

        /**
         * MessageId.UNSOL_MODIFY_CALL
         *
         * @param CallModifyInfo call modify info.
         */
        @Override
        public void onModifyCall(CallModifyInfo callModifyInfo) {
            if (callModifyInfo == null) {
                unsljLogRet(MessageId.UNSOL_MODIFY_CALL, null);
                Log.e(this, "onModifyCall callModifyInfo is null");
                return;
            }
            CallModify ret = ImsRadioUtils.callModifyFromHal(callModifyInfo);
            unsljLogRet(MessageId.UNSOL_MODIFY_CALL, ret);
            mModifyCallRegistrants
                    .notifyRegistrants(new AsyncResult(null, ret, null));
        }

        @Override
        public void onSuppServiceNotification(SuppServiceNotification suppServiceNotification) {
            Object ret = ImsRadioUtils.suppServiceNotificationFromHal(suppServiceNotification);
            unsljLogRet(MessageId.UNSOL_SUPP_SVC_NOTIFICATION, ret);
            if (mSsnRegistrant != null) {
                mSsnRegistrant.notifyRegistrant(
                        new AsyncResult (null, ret, null));
            }
        }

        @Override
        public void onMessageWaiting(MessageWaitingIndication messageWaitingIndication) {
            Mwi ret = ImsRadioUtils.messageWaitingIndicationFromHal(
                    messageWaitingIndication);
            unsljLogRet(MessageId.UNSOL_MWI, ret);
            if (ret != null) {
                mMwiRegistrants.notifyRegistrants(new AsyncResult (null, ret, null));
            } else {
                Log.e(this, "onMessageWaiting: Data is null");
            }
        }

        @Override
        public void onGeolocationInfoRequested(double lat, double lon) {
            GeoLocationInfo ret =
                    ImsRadioUtils.geolocationIndicationFromHal(lat, lon);
            unsljLogRet(MessageId.UNSOL_REQUEST_GEOLOCATION, ret);
            if (ret != null) {
                mGeolocationRegistrants.notifyRegistrants(new AsyncResult (null, ret, null));
            } else {
                Log.e(this, "onGeolocationInfoRequested: Null location data!");
            }
        }

        @Override
        public void onIncomingCallAutoRejected_1_5(
                vendor.qti.hardware.radio.ims.V1_5.AutoCallRejectionInfo rejectInfo) {
            unsljLogRet(MessageId.UNSOL_AUTO_CALL_REJECTION_IND, rejectInfo);
            if (rejectInfo == null) {
                Log.e(this, "onIncomingCallAutoRejected: rejectInfo is null. Returning");
                return;
            }

            DriverCallIms dc = new DriverCallIms();
            int imsReasonInfoCode = ImsRadioUtilsV13.getImsReasonForCallFailCause(
                    rejectInfo.autoRejectionCause);
            imsReasonInfoCode = imsReasonInfoCode == ImsReasonInfo.CODE_UNSPECIFIED ?
                    ImsReasonInfo.CODE_REJECT_UNKNOWN : imsReasonInfoCode;
            dc.callFailCause = new ImsReasonInfo(imsReasonInfoCode, rejectInfo.sipErrorCode, null);
            dc.callDetails = new CallDetails();
            dc.callDetails.call_type = ImsRadioUtils.callTypeFromHal(rejectInfo.callType);
            dc.number = rejectInfo.number;
            Log.v(this, "onIncomingCallAutoRejected :: Call auto rejected from : "
                    + rejectInfo.number);
            mAutoRejectRegistrants.notifyRegistrants(new AsyncResult(null, dc, null));
        }

        @Override
        public void onIncomingCallAutoRejected(AutoCallRejectionInfo rejectInfo) {
            onIncomingCallAutoRejected_1_5(ImsRadioUtilsV15.migrateAutoCallRejectionInfoFrom(
                    rejectInfo));
        }

        public void onImsSubConfigChanged(ImsSubConfigInfo config) {
            ImsSubConfigDetails ret = ImsRadioUtils.imsSubconfigFromHal(config);
            unsljLogRet(MessageId.UNSOL_IMS_SUB_CONFIG_CHANGED, ret);
            if (ret != null) {
                mImsSubConfigChangeRegistrants
                    .notifyRegistrants(new AsyncResult(null, ret, null));
            }
        }

        @Override
        public void onParticipantStatusInfo(ParticipantStatusInfo participantStatusInfo) {
            ParticipantStatusDetails ret =
                    ImsRadioUtils.participantStatusFromHal(participantStatusInfo);
            unsljLogRet(MessageId.UNSOL_PARTICIPANT_STATUS_INFO, ret);
            if (ret != null) {
                mParticipantStatusRegistrants
                    .notifyRegistrants(new AsyncResult(null, ret, null));
            }
        }

        @Override
        public void onRegistrationBlockStatus(boolean hasBlockStatusOnWwan,
                BlockStatus blockStatusOnWwan, boolean hasBlockStatusOnWlan,
                BlockStatus blockStatusOnWlan) {
            RegistrationBlockStatusInfo ret =
                    ImsRadioUtils.registrationBlockStatusFromHal(hasBlockStatusOnWwan,
                            blockStatusOnWwan, hasBlockStatusOnWlan, blockStatusOnWlan);
            unsljLogRet(MessageId.UNSOL_RESPONSE_REGISTRATION_BLOCK_STATUS, ret);
            if (ret != null) {
                mRegistrationBlockStatusRegistrants.notifyRegistrants(
                        new AsyncResult (null, ret, null));
            } else {
                Log.e(this, "onRegistrationBlockStatus: Data is null.");
            }
        }

        public void onRttMessageReceived(String msg) {
            if (msg != null) {
                Log.v(this, "onRttMessageReceived: msg = " + msg);
                unsljLogRet(MessageId.UNSOL_RESPONSE_RTT_MSG_RECEIVED, msg);
                mRttMessageRegistrants
                    .notifyRegistrants(new AsyncResult(null, msg, null));
            }
        }

        @Override
        public void onVoiceInfoChanged(int voiceInfo) {
            Log.v(this, "onVoiceInfoChanged: VoiceInfo = " + voiceInfo);
            unsljLogRet(MessageId.UNSOL_VOICE_INFO, voiceInfo);
            mVoiceInfoStatusRegistrants
                .notifyRegistrants(new AsyncResult(null, voiceInfo, null));
        }

        @Override
        public void onVoWiFiCallQuality(int voWiFiCallQuality) {
            unsljLogRet(MessageId.UNSOL_VOWIFI_CALL_QUALITY, voWiFiCallQuality);
            Object ret = ImsRadioUtils.voWiFiCallQualityFromHal(voWiFiCallQuality);
            if (mVoWiFiCallQualityRegistrants != null) {
                mVoWiFiCallQualityRegistrants.notifyRegistrants(
                        new AsyncResult (null, ret, null));
            }
        }

        @Override
        public void onSupplementaryServiceIndication(StkCcUnsolSsResult ss) {
            ImsSsData.Builder ssDataBuilder  = new ImsSsData.Builder(
                    ImsRadioUtils.serviceTypeFromRILServiceType(ss.serviceType),
                    ImsRadioUtils.requestTypeFromRILRequestType(ss.requestType),
                    ImsRadioUtils.teleserviceTypeFromRILTeleserviceType(ss.teleserviceType),
                    ss.serviceClass, ss.result);
            ImsSsData ssData = ssDataBuilder.build();

            int num;
            if (ssData.isTypeCf() && ssData.isTypeInterrogation()) {
                List <ImsCallForwardInfo> cfInfo = new ArrayList<>();
                if (ss.cfData == null) {
                    Log.d(this, "cfData is null, which is unexpected for: " +
                            ssData.getServiceType());
                } else {
                    CfData cfData = ss.cfData.get(0);
                    Object ret = ImsRadioUtils.buildCFStatusResponseFromHal(cfData.cfInfo);
                    if (ret instanceof ImsCallForwardTimerInfo[]) {
                        Log.i(this, "onSupplementaryServiceIndication : Not handling CFUT SS");
                        unsljLogRet(MessageId.UNSOL_ON_SS, null);
                        return;
                    } else {
                        num = cfData.cfInfo.size();
                        for (int i = 0; i < num; i++) {
                            vendor.qti.hardware.radio.ims.V1_0.CallForwardInfo rCfInfo = cfData.
                                    cfInfo.get(i);
                            cfInfo.add(new ImsCallForwardInfo(ImsRadioUtils.
                                    getUTInterfaceCFReasonFromCommandsInterfaceCFReason(
                                    rCfInfo.reason),
                                    rCfInfo.status,
                                    rCfInfo.toa,
                                    rCfInfo.serviceClass,
                                    rCfInfo.number,
                                    rCfInfo.timeSeconds));

                            Log.i(this, "[SS Data] CF Info " + i + " : " +  cfInfo.get(i));
                        }
                    }
                }
                ssDataBuilder.setCallForwardingInfo(cfInfo);
            } else if (ssData.isTypeIcb() && ssData.isTypeInterrogation()) {
                List <ImsSsInfo> imsSsInfo = new ArrayList<>();
                if (ss.cbNumInfo == null) {
                    Log.d(this, "cbNumInfo is null, which is unexpected for: " +
                                ssData.getServiceType());
                } else {
                    num = ss.cbNumInfo.size();
                    for (int i = 0; i < num; i++) {
                        ImsSsInfo.Builder imsSsInfoBuilder =
                                new ImsSsInfo.Builder(ss.cbNumInfo.get(i).status);

                        imsSsInfoBuilder.setIncomingCommunicationBarringNumber(
                                ss.cbNumInfo.get(i).number);
                        imsSsInfo.add(imsSsInfoBuilder.build());
                        Log.i(this, "[SS Data] ICB Info " + i + " : " +  imsSsInfo.get(i));
                    }
                }
                ssDataBuilder.setSuppServiceInfo(imsSsInfo);
            } else {
                /** Handling for SS_CLIP/SS_CLIR/SS_COLP/SS_WAIT
                    Currently, max size of the array sent is 2.
                    Expected format for all except SS_CLIR is:
                    status - ssInfo[0]
                    provision status - (Optional) ssInfo[1]
                */
                List <ImsSsInfo> imsSsInfo = new ArrayList<>();
                if (ss.ssInfoData == null) {
                    Log.d(this, "imsSsInfo is null, which is unexpected for: " +
                                ssData.getServiceType());
                } else {
                    SsInfoData ssInfoData = ss.ssInfoData.get(0);
                    num = ssInfoData.ssInfo.size();
                    if (num > 0) {
                        ImsSsInfo.Builder imsSsInfoBuilder =
                                new ImsSsInfo.Builder(ssInfoData.ssInfo.get(0));
                        if (ssData.isTypeClir() && ssData.isTypeInterrogation()) {
                            // creating ImsSsInfoBuilder with first int as status not used
                            imsSsInfoBuilder.setClirOutgoingState(ssInfoData.ssInfo.get(0));
                            if (num > 1) {
                                imsSsInfoBuilder.setClirInterrogationStatus(
                                        ssInfoData.ssInfo.get(1));
                            }
                        } else if (num > 1) {
                            imsSsInfoBuilder.setProvisionStatus(ssInfoData.ssInfo.get(1));
                        }
                        imsSsInfo.add(imsSsInfoBuilder.build());
                    }
                }
                ssDataBuilder.setSuppServiceInfo(imsSsInfo);
            }

            unsljLogRet(MessageId.UNSOL_ON_SS, ssData);

            if (mSsIndicationRegistrant != null) {
                mSsIndicationRegistrant.notifyRegistrant(new AsyncResult(null, ssData, null));
            }
        }

        @Override
        public void onVopsChanged(boolean isVopsEnabled) {
                unsljLogRet(MessageId.UNSOL_VOPS_CHANGED, isVopsEnabled);
                if (mVopsRegistrants != null) {
                    mVopsRegistrants
                        .notifyRegistrants(new AsyncResult(null, isVopsEnabled, null));
                }
        }

        @Override
        public void onMultiIdentityRegistrationStatusChange(
                ArrayList<MultiIdentityLineInfoHal> lines) {
            ArrayList<MultiIdentityLineInfo> linesInfo = new ArrayList<>();
            for(MultiIdentityLineInfoHal line : lines) {
                MultiIdentityLineInfo lineInfo =  ImsRadioUtilsV14.
                        fromMultiIdentityLineInfoHal(line);
                linesInfo.add(lineInfo);
            }
            unsljLogRet(MessageId.UNSOL_MULTI_IDENTITY_REGISTRATION_STATUS_CHANGE, linesInfo);
            mMultiIdentityStatusChangeRegistrants.notifyRegistrants(
                    new AsyncResult(null, linesInfo, null));
        }

        @Override
        public void onMultiIdentityInfoPending() {
            unsljLogRet(MessageId.UNSOL_MULTI_IDENTITY_INFO_PENDING, null);
            mMultiIdentityInfoPendingRegistrants.notifyRegistrants(
                    new AsyncResult(null, null, null));
        }

        @Override
        public void onModemSupportsWfcRoamingModeConfiguration(
                boolean wfcRoamingConfigurationSupport) {
            Log.i(this, "wfcRoamingConfigurationSupport = " + wfcRoamingConfigurationSupport);
            unsljLogRet(MessageId.UNSOL_MODEM_SUPPORTS_WFC_ROAMING_MODE,
                    wfcRoamingConfigurationSupport);
            mWfcRoamingConfigurationSupport = wfcRoamingConfigurationSupport;
            if (mWfcRoamingModeConfigRegistrants != null) {
                mWfcRoamingModeConfigRegistrants
                        .notifyRegistrants(new AsyncResult(null, wfcRoamingConfigurationSupport,
                         null));
            }
        }

        @Override
        public void onUssdMessageFailed(int type, SipErrorInfo errorDetails) {
            UssdInfo ussdInfo = ImsRadioUtils.ussdInfoFromHal(type, errorDetails);
            unsljLogRet(MessageId.UNSOL_USSD_FAILED, ussdInfo);
            if (mUssdInfoRegistrants != null) {
                mUssdInfoRegistrants.notifyRegistrants(new AsyncResult(null, ussdInfo, null));
            }
        }
    }

    /* Overloading function with default argument. */
    private void removeFromQueueAndSendResponse(int token) {
        removeFromQueueAndSendResponse(token, ImsErrorCode.GENERIC_FAILURE);
    }

    private void removeFromQueueAndSendResponse(int token, Object ret) {
        removeFromQueueAndSendResponse(token, ImsErrorCode.GENERIC_FAILURE, ret);
    }

    /**
     * Removes the request matching the token id from the request list and sends the response
     * to the client
     *
     * @param token to match request/response. Response must include same token as request
     * @param errorCode of type ImsErrorCode.Error send back from RIL for the dial request
     * @param ret Result object of the request to be passed back to the client
     *
     */
    private void removeFromQueueAndSendResponse(int token, int errorCode) {
        removeFromQueueAndSendResponse(token, errorCode, null);
    }

    private void removeFromQueueAndSendResponse(int token, int errorCode, Object ret) {
        IFRequest rr = findAndRemoveRequestFromList(token);

        if (rr == null) {
            Log.w(this, "Unexpected solicited response or Invalid token id! token: "
                        + token + " error: " + errorCode);
            return;
        }
        sendResponse(rr, errorCode, ret);
    }

    /**
     * Sends the response to the client corresponding to the IFRequest passed in as a param and
     * returns any error send to the client along with the result object
     *
     * @param rr IFRequest containing the token id, request type, request id, etc.
     * @param errorCode of type ImsErrorCode.Error send back from RIL for the dial request
     * @param ret Result object of the request to be passed back to the client
     *
     */
    private void sendResponse(IFRequest rr, int error, Object ret) {
        if (error != ImsErrorCode.SUCCESS) {
            rr.onError(error, ret);
            rr.release();
            releaseWakeLockIfDone();
            return;
        }

        log(rr.serialString() + "< " + msgIdToString(rr.mRequest)
                + " " + retToString(rr.mRequest, ret));

        if (rr.mResult != null) {
            AsyncResult.forMessage(rr.mResult, ret, null);
            rr.mResult.sendToTarget();
        }

        rr.release();
        releaseWakeLockIfDone();
    }

    public ImsSenderRxr(Context context, int phoneId)
    {
        super(context);

        mPhoneId = phoneId;
        initNotifyRegistrants();

        PowerManager pm = (PowerManager) context.getSystemService(Context.POWER_SERVICE);
        mWakeLock = pm.newWakeLock(PowerManager.PARTIAL_WAKE_LOCK, LOG_TAG);
        mWakeLock.setReferenceCounted(false);
        mWakeLockTimeout = SystemProperties.getInt(TelephonyProperties.PROPERTY_WAKE_LOCK_TIMEOUT,
                DEFAULT_WAKE_LOCK_TIMEOUT);
        mImsRadioHandler = new ImsRadioHandler();
        mRequestMessagesWaiting = 0;

        mDeathRecipient = new ImsRadioDeathRecipient();
        // register for ImsRadio service notification
        registerForImsRadioServiceNotification();
    }

    private void initNotifyRegistrants() {
        mHandoverStatusRegistrants = new WakeLockRegistrantList(mNotifyWakeLock);
        mRefreshConfInfoRegistrations = new WakeLockRegistrantList(mNotifyWakeLock);
        mSrvStatusRegistrations = new WakeLockRegistrantList(mNotifyWakeLock);
        mTtyStatusRegistrants = new WakeLockRegistrantList(mNotifyWakeLock);
        mRadioStateRegistrations = new WakeLockRegistrantList(mNotifyWakeLock);
        mGeolocationRegistrants = new WakeLockRegistrantList(mNotifyWakeLock);
        mVoWiFiCallQualityRegistrants = new WakeLockRegistrantList(mNotifyWakeLock);
        mSsaccRegistrants = new WakeLockRegistrantList(mNotifyWakeLock);
        mVopsRegistrants = new WakeLockRegistrantList(mNotifyWakeLock);
        mParticipantStatusRegistrants = new WakeLockRegistrantList(mNotifyWakeLock);
        mImsSubConfigChangeRegistrants = new WakeLockRegistrantList(mNotifyWakeLock);
        mRegistrationBlockStatusRegistrants = new WakeLockRegistrantList(mNotifyWakeLock);
        mModifyCallRegistrants = new WakeLockRegistrantList(mNotifyWakeLock);
        mMwiRegistrants = new WakeLockRegistrantList(mNotifyWakeLock);
        mRttMessageRegistrants = new WakeLockRegistrantList(mNotifyWakeLock);
        mRttModifyRegistrants = new WakeLockRegistrantList(mNotifyWakeLock);
        mAutoRejectRegistrants = new WakeLockRegistrantList(mNotifyWakeLock);
        mVoiceInfoStatusRegistrants = new WakeLockRegistrantList(mNotifyWakeLock);
        mMultiIdentityStatusChangeRegistrants = new WakeLockRegistrantList(mNotifyWakeLock);
        mMultiIdentityInfoPendingRegistrants = new WakeLockRegistrantList(mNotifyWakeLock);
        mWfcRoamingModeConfigRegistrants = new WakeLockRegistrantList(mNotifyWakeLock);
        mUssdInfoRegistrants = new WakeLockRegistrantList(mNotifyWakeLock);
    }

    /**
     * Holds a PARTIAL_WAKE_LOCK whenever a) There is outstanding request sent
     * to the interface and no replied b) There is a request pending to be sent
     * out. There is a WAKE_LOCK_TIMEOUT to release the lock, though it
     * shouldn't happen often.
     */

    private void acquireWakeLock() {
        synchronized (mWakeLock) {
            mWakeLock.acquire();

            mImsRadioHandler.removeMessages(EVENT_WAKE_LOCK_TIMEOUT);
            Message msg = mImsRadioHandler.obtainMessage(EVENT_WAKE_LOCK_TIMEOUT);
            mImsRadioHandler.sendMessageDelayed(msg, mWakeLockTimeout);
        }
    }

    private void releaseWakeLockIfDone() {
        synchronized (mWakeLock) {
            if (mWakeLock.isHeld() &&
                    (mRequestMessagesWaiting == 0)) {
                mWakeLock.release();
            }
        }
    }

    /**
     * Acquires the wake lock, queues the IFRequest in the request list so that when the response
     * comes we can match it with the request.
     *
     * @param rr IFRequest containing the token id, request type, request id, etc.
     */
    private void queueRequest(IFRequest rr) {
        acquireWakeLock();

       synchronized (mRequestsList) {
           mRequestsList.add(rr);
           mRequestMessagesWaiting++;
        }
    }

    /**
     * Release each request in mRequestsList then clear the list
     *
     * @param error is the ImsErrorCode.Error sent back
     * @param loggable true means to print all requests in mRequestslist
     */
    private void clearRequestsList(int error, boolean loggable) {
        IFRequest rr;
        synchronized (mRequestsList) {
            int count = mRequestsList.size();
            if (loggable) {
                Log.i(this,"clearRequestsList: mRequestList=" + count);
            }

            for (int i = 0; i < count; i++) {
                rr = mRequestsList.get(i);
                if (loggable) {
                    Log.i(this, i + ": [" + rr.mSerial + "] " +
                            msgIdToString(rr.mRequest));
                }
                rr.onError(error, null);
                rr.release();
            }
            mRequestsList.clear();
            mRequestMessagesWaiting = 0;
            releaseWakeLockIfDone();
        }

        /* Clear the existing calls also */
        if (mIsUnsolCallListPresent) {
            mIsUnsolCallListPresent = false;
            mCallStateRegistrants
                    .notifyRegistrants(new AsyncResult(null, null,
                    new RuntimeException(ImsSenderRxr.errorIdToString(error))));
        }
    }

    private IFRequest findAndRemoveRequestFromList(int serial) {
        synchronized (mRequestsList) {
            for (int i = 0, s = mRequestsList.size(); i < s; i++) {
                IFRequest rr = mRequestsList.get(i);

                if (rr.mSerial == serial) {
                    mRequestsList.remove(i);
                    if (mRequestMessagesWaiting > 0)
                        mRequestMessagesWaiting--;
                    return rr;
                }
            }
        }

        return null;
    }

    private String retToString(int req, Object ret) {

        if (ret == null)
            return "";

        StringBuilder sb;
        String s;
        int length;
        if (ret instanceof int[]) {
            int[] intArray = (int[]) ret;
            length = intArray.length;
            sb = new StringBuilder("{");
            if (length > 0) {
                int i = 0;
                sb.append(intArray[i++]);
                while (i < length) {
                    sb.append(", ").append(intArray[i++]);
                }
            }
            sb.append("}");
            s = sb.toString();
        } else if (ret instanceof String[]) {
            String[] strings = (String[]) ret;
            length = strings.length;
            sb = new StringBuilder("{");
            if (length > 0) {
                int i = 0;
                sb.append(strings[i++]);
                while (i < length) {
                    sb.append(", ").append(strings[i++]);
                }
            }
            sb.append("}");
            s = sb.toString();
        } else if (req == MessageId.UNSOL_RESPONSE_CALL_STATE_CHANGED
                || req == MessageId.REQUEST_GET_CURRENT_CALLS ) {
            ArrayList<DriverCallIms> calls = (ArrayList<DriverCallIms>) ret;
            sb = new StringBuilder(" ");
            for (DriverCallIms dc : calls) {
                sb.append("[").append(dc).append("] ");
            }
            s = sb.toString();
        } else {
            s = ret.toString();
        }
        return s;
    }

    public void registerForModifyCall(Handler h, int what, Object obj) {
        Registrant r = new WakeLockRegistrant(h, what, obj, mNotifyWakeLock);
        mModifyCallRegistrants.add(r);
    }

    public void unregisterForModifyCall(Handler h) {
        mModifyCallRegistrants.remove(h);
    }

    public void registerForMwi(Handler h, int what, Object obj) {
        Registrant r = new WakeLockRegistrant(h, what, obj, mNotifyWakeLock);
        mMwiRegistrants.add(r);
    }

    public void unregisterForMwi(Handler h) {
        mMwiRegistrants.remove(h);
    }

    static String errorIdToString(int request) {
        String errorMsg;
        switch (request) {
            case ImsErrorCode.SUCCESS:
                return "SUCCESS";
            case ImsErrorCode.RADIO_NOT_AVAILABLE:
                return "E_RADIO_NOT_AVAILABLE";
            case ImsErrorCode.GENERIC_FAILURE:
                return "E_GENERIC_FAILURE";
            case ImsErrorCode.REQUEST_NOT_SUPPORTED:
                return "E_REQUEST_NOT_SUPPORTED";
            case ImsErrorCode.CANCELLED:
                return "E_CANCELLED";
            case ImsErrorCode.UNUSED:
                return "E_UNUSED";
            case ImsErrorCode.INVALID_PARAMETER:
                return "E_INVALID_PARAMETER";
            case ImsErrorCode.REJECTED_BY_REMOTE:
                return "E_REJECTED_BY_REMOTE";
            case ImsErrorCode.NETWORK_NOT_SUPPORTED:
                return "E_NETWORK_NOT_SUPPORTED";
            case ImsErrorCode.FDN_CHECK_FAILURE:
                return "E_FDN_CHECK_FAILURE";
            case ImsErrorCode.SS_MODIFIED_TO_DIAL:
                return "E_SS_MODIFIED_TO_DIAL";
            case ImsErrorCode.SS_MODIFIED_TO_USSD:
                return "E_SS_MODIFIED_TO_USSD";
            case ImsErrorCode.SS_MODIFIED_TO_SS:
                return "E_SS_MODIFIED_TO_SS";
            case ImsErrorCode.SS_MODIFIED_TO_DIAL_VIDEO:
                return "E_SS_MODIFIED_TO_DIAL_VIDEO";
            default:
                return "E_UNKNOWN";
        }
    }

    static String msgIdToString(int request) {
        // TODO - check all supported messages are covered
        switch (request) {
            case MessageId.REQUEST_GET_CURRENT_CALLS:
                return "GET_CURRENT_CALLS";
            case MessageId.REQUEST_DIAL:
                return "DIAL";
            case MessageId.REQUEST_ANSWER:
                return "REQUEST_ANSWER";
            case MessageId.REQUEST_DEFLECT_CALL:
                return "REQUEST_DEFLECT_CALL";
            case MessageId.REQUEST_ADD_PARTICIPANT:
                return "REQUEST_ADD_PARTICIPANT";
            case MessageId.REQUEST_HANGUP:
                return "HANGUP";
            case MessageId.REQUEST_HANGUP_WAITING_OR_BACKGROUND:
                return "HANGUP_WAITING_OR_BACKGROUND";
            case MessageId.REQUEST_HANGUP_FOREGROUND_RESUME_BACKGROUND:
                return "HANGUP_FOREGROUND_RESUME_BACKGROUND";
            case MessageId.REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE:
                return "MessageId.REQUEST_SWITCH_WAITING_OR_HOLDING_AND_ACTIVE";
            case MessageId.REQUEST_CONFERENCE:
                return "CONFERENCE";
            case MessageId.REQUEST_UDUB:
                return "UDUB";
            case MessageId.REQUEST_SEND_UI_TTY_MODE:
                return "REQUEST_SEND_UI_TTY_MODE";
            case MessageId.REQUEST_MODIFY_CALL_INITIATE:
                return "MODIFY_CALL_INITIATE";
            case MessageId.REQUEST_MODIFY_CALL_CONFIRM:
                return "MODIFY_CALL_CONFIRM";
            case MessageId.UNSOL_MODIFY_CALL:
                return "UNSOL_MODIFY_CALL";
            case MessageId.REQUEST_LAST_CALL_FAIL_CAUSE:
                return "LAST_CALL_FAIL_CAUSE";
            case MessageId.REQUEST_DTMF:
                return "DTMF";
            case MessageId.REQUEST_DTMF_START:
                return "DTMF_START";
            case MessageId.REQUEST_DTMF_STOP:
                return "DTMF_STOP";
            case MessageId.REQUEST_EXPLICIT_CALL_TRANSFER:
                return "MessageId.REQUEST_EXPLICIT_CALL_TRANSFER";
            case MessageId.REQUEST_EXIT_EMERGENCY_CALLBACK_MODE:
                return "MessageId.REQUEST_EXIT_EMERGENCY_CALLBACK_MODE";
            case MessageId.REQUEST_IMS_REGISTRATION_STATE:
                return "REQUEST_IMS_REGISTRATION_STATE";
            case MessageId.REQUEST_QUERY_CLIP:
                return "REQUEST_QUERY_CLIP";
            case MessageId.REQUEST_QUERY_SERVICE_STATUS:
                return "REQUEST_QUERY_SERVICE_STATUS";
            case MessageId.REQUEST_SET_SERVICE_STATUS:
                return "REQUEST_SET_SERVICE_STATUS";
            case MessageId.REQUEST_GET_CLIR:
                return "REQUEST_GET_CLIR";
            case MessageId.REQUEST_SET_CLIR:
                return "REQUEST_SET_CLIR";
            case MessageId.REQUEST_QUERY_CALL_FORWARD_STATUS:
                return "REQUEST_QUERY_CALL_FORWARD_STATUS";
            case MessageId.REQUEST_SET_CALL_FORWARD_STATUS:
                return "REQUEST_SET_CALL_FORWARD_STATUS";
            case MessageId.REQUEST_QUERY_CALL_WAITING:
                return "REQUEST_QUERY_CALL_WAITING";
            case MessageId.REQUEST_SET_CALL_WAITING:
                return "REQUEST_SET_CALL_WAITING";
            case MessageId.REQUEST_SET_SUPP_SVC_NOTIFICATION:
                return "REQUEST_SET_SUPP_SVC_NOTIFICATION";
            case MessageId.REQUEST_SUPP_SVC_STATUS:
                return "REQUEST_SUPP_SVC_STATUS";
            case MessageId.REQUEST_GET_RTP_STATISTICS:
                return "REQUEST_GET_RTP_STATISTICS";
            case MessageId.REQUEST_GET_RTP_ERROR_STATISTICS:
                return "REQUEST_GET_RTP_ERROR_STATISTICS";
            case MessageId.REQUEST_GET_WIFI_CALLING_STATUS:
                return "REQUEST_GET_WIFI_CALLING_STATUS";
            case MessageId.REQUEST_SET_WIFI_CALLING_STATUS:
                return "REQUEST_SET_WIFI_CALLING_STATUS";
            case MessageId.REQUEST_GET_COLR:
                return "REQUEST_GET_COLR";
            case MessageId.REQUEST_SET_COLR:
                return "REQUEST_SET_COLR";
            case MessageId.REQUEST_HOLD:
                return "REQUEST_HOLD";
            case MessageId.REQUEST_RESUME:
                return "REQUEST_RESUME";
            case MessageId.REQUEST_SET_IMS_CONFIG:
                return "REQUEST_SET_IMS_CONFIG";
            case MessageId.REQUEST_GET_IMS_CONFIG:
                return "REQUEST_GET_IMS_CONFIG";
            case MessageId.REQUEST_SEND_GEOLOCATION_INFO:
                return "REQUEST_SEND_GEOLOCATION_INFO";
            case MessageId.REQUEST_GET_VOPS_INFO:
                return "REQUEST_GET_VOPS_INFO";
            case MessageId.REQUEST_GET_SSAC_INFO:
                return "REQUEST_GET_SSAC_INFO";
            case MessageId.REQUEST_SET_VOLTE_PREF:
                return "REQUEST_SET_VOLTE_PREF";
            case MessageId.REQUEST_GET_VOLTE_PREF:
                return "REQUEST_GET_VOLTE_PREF";
            case MessageId.REQUEST_GET_HANDOVER_CONFIG:
                return "REQUEST_GET_HANDOVER_CONFIG";
            case MessageId.REQUEST_SET_HANDOVER_CONFIG:
                return "REQUEST_SET_HANDOVER_CONFIG";
            case MessageId.REQUEST_GET_IMS_SUB_CONFIG:
                return "REQUEST_GET_IMS_SUB_CONFIG";
            case MessageId.REQUEST_SEND_RTT_MSG:
                return "REQUEST_SEND_RTT_MSG";
            case MessageId.REQUEST_CANCEL_MODIFY_CALL:
                return "REQUEST_CANCEL_MODIFY_CALL";
            case MessageId.REQUEST_SEND_IMS_SMS:
                return "REQUEST_SEND_IMS_SMS";
            case MessageId.REQUEST_ACK_IMS_SMS:
                return "REQUEST_ACK_IMS_SMS";
            case MessageId.REQUEST_ACK_IMS_SMS_STATUS_REPORT:
                return "REQUEST_ACK_IMS_SMS_STATUS_REPORT";
            case MessageId.REQUEST_REGISTER_MULTI_IDENTITY_LINES:
                return "REQUEST_REGISTER_MULTI_IDENTITY_LINES";
            case MessageId.REQUEST_QUERY_VIRTUAL_LINE_INFO:
                return "REQUEST_QUERY_VIRTUAL_LINE_INFO";
            case MessageId.REQUEST_EMERGENCY_DIAL:
                return "REQUEST_EMERGENCY_DIAL";
            case MessageId.UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED:
                return "UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED";
            case MessageId.UNSOL_RESPONSE_CALL_STATE_CHANGED:
                return "UNSOL_RESPONSE_CALL_STATE_CHANGED";
            case MessageId.UNSOL_CALL_RING:
                return "UNSOL_CALL_RING";
            case MessageId.UNSOL_ENTER_EMERGENCY_CALLBACK_MODE:
                return "UNSOL_ENTER_EMERGENCY_CALLBACK_MODE";
            case MessageId.UNSOL_RINGBACK_TONE:
                return "UNSOL_RINGBACK_TONE";
            case MessageId.UNSOL_EXIT_EMERGENCY_CALLBACK_MODE:
                return "UNSOL_EXIT_EMERGENCY_CALLBACK_MODE";
            case MessageId.REQUEST_IMS_REG_STATE_CHANGE:
                return "REQUEST_IMS_REG_STATE_CHANGE";
            case MessageId.UNSOL_RESPONSE_HANDOVER:
                return "UNSOL_RESPONSE_HANDOVER";
            case MessageId.UNSOL_REFRESH_CONF_INFO:
                return "UNSOL_REFRESH_CONF_INFO";
            case MessageId.UNSOL_SRV_STATUS_UPDATE:
                return "UNSOL_SRV_STATUS_UPDATE";
            case MessageId.UNSOL_SUPP_SVC_NOTIFICATION:
                return "UNSOL_SUPP_SVC_NOTIFICATION";
            case MessageId.UNSOL_TTY_NOTIFICATION:
                return "UNSOL_TTY_NOTIFICATION";
            case MessageId.UNSOL_RADIO_STATE_CHANGED:
                return "UNSOL_RADIO_STATE_CHANGED";
            case MessageId.UNSOL_MWI:
                return "UNSOL_MWI";
            case MessageId.UNSOL_REQUEST_GEOLOCATION:
                return "UNSOL_REQUEST_GEOLOCATION";
            case MessageId.UNSOL_REFRESH_VICE_INFO:
                return "UNSOL_REFRESH_VICE_INFO";
            case MessageId.UNSOL_VOWIFI_CALL_QUALITY:
                return "UNSOL_VOWIFI_CALL_QUALITY";
            case MessageId.UNSOL_VOPS_CHANGED:
                return "UNSOL_VOPS_CHANGED";
            case MessageId.UNSOL_SSAC_CHANGED:
                return "UNSOL_SSAC_CHANGED";
            case MessageId.UNSOL_PARTICIPANT_STATUS_INFO:
                return "UNSOL_PARTICIPANT_STATUS_INFO";
            case MessageId.UNSOL_IMS_SUB_CONFIG_CHANGED:
                return "UNSOL_IMS_SUB_CONFIG_CHANGED";
            case MessageId.UNSOL_RESPONSE_REGISTRATION_BLOCK_STATUS:
                return "UNSOL_RESPONSE_REGISTRATION_BLOCK_STATUS";
            case MessageId.UNSOL_RESPONSE_RTT_MSG_RECEIVED:
                return "UNSOL_RESPONSE_RTT_MSG_RECEIVED";
            case MessageId.UNSOL_ON_SS:
                return "UNSOL_ON_SS";
            case MessageId.UNSOL_IMS_SMS_STATUS_REPORT:
                return "UNSOL_IMS_SMS_STATUS_REPORT";
            case MessageId.UNSOL_INCOMING_IMS_SMS:
                return "UNSOL_INCOMING_IMS_SMS";
            case MessageId.UNSOL_AUTO_CALL_REJECTION_IND:
                return "UNSOL_AUTO_CALL_REJECTION_IND";
            case MessageId.UNSOL_VOICE_INFO:
                return "UNSOL_VOICE_INFO";
            case MessageId.UNSOL_MULTI_IDENTITY_REGISTRATION_STATUS_CHANGE:
                return "UNSOL_MULTI_IDENTITY_REGISTRATION_STATUS_CHANGE";
            case MessageId.UNSOL_MULTI_IDENTITY_INFO_PENDING:
                return "UNSOL_MULTI_IDENTITY_INFO_PENDING";
            case MessageId.UNSOL_MODEM_SUPPORTS_WFC_ROAMING_MODE:
                return "UNSOL_MODEM_SUPPORTS_WFC_ROAMING_MODE";
            case MessageId.UNSOL_USSD_FAILED:
                return "UNSOL_USSD_FAILED";
            default:
                return "<unknown message>";
        }
    }

    public void log(String msg) {
        Log.i(this, msg + "[SUB" + mPhoneId + "]");
    }

    public void logv(String msg) {
        Log.v(this, msg + "[SUB" + mPhoneId + "]");
    }

    /**
     * Use this only for unimplemented methods. Prints stack trace if the
     * unimplemented method is ever called
     */
    public void logUnimplemented() {
        try {
            Exception e = new Exception();
            throw e;
        } catch (Exception e) {
            Log.i(this, "Unimplemented method. Stack trace: ");
            e.printStackTrace();
        }
    }

    public void unsljLog(int response) {
        log("[UNSL]< " + msgIdToString(response));
    }

    public void unsljLogMore(int response, String more) {
        log("[UNSL]< " + msgIdToString(response) + " " + more);
    }

    public void unsljLogRet(int response, Object ret) {
        log("[UNSL]< " + msgIdToString(response) + " " + retToString(response, ret));
    }

    public void unsljLogvRet(int response, Object ret) {
        logv("[UNSL]< " + msgIdToString(response) + " " + retToString(response, ret));
    }

    public void logSolicitedRequest(IFRequest rr) {
        log(rr.serialString() + "> " + msgIdToString(rr.mRequest) + " ");
    }

    @Override
    public void setPhoneType(int phoneType) { // Called by Phone constructor
        log("setPhoneType=" + phoneType + " old value=" + mPhoneType);
        mPhoneType = phoneType;
    }

    public void addParticipant(String address, int clirMode, CallDetails callDetails,
            Message result) {
        log("addParticipant address = " + Log.pii(address) + " clirMode = " + clirMode
                + " callDetails = " + callDetails);
        final int msgId = MessageId.REQUEST_ADD_PARTICIPANT;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, result);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        vendor.qti.hardware.radio.ims.V1_4.DialRequest dialRequest =
                ImsRadioUtils.buildDialRequest(address, clirMode, callDetails,
                        false /*isEncrypted*/);
        try {
            logSolicitedRequest(rr);
            vendor.qti.hardware.radio.ims.V1_4.IImsRadio imsRadio = imsRadioV14();
            if (imsRadio != null) {
                imsRadio.addParticipant_1_4(rr.mSerial, dialRequest);
            } else {
                Log.w(this, "mImsRadio V1.4 is null. invoking mImsRadio.dial()");
                imsRadioV10().addParticipant(rr.mSerial, ImsRadioUtils.migrateFromDialRequestV14(
                        dialRequest));
            }
            Log.v(this, msgIdString + " request to IImsRadio - token = " + rr.mSerial +
                    " address = " + dialRequest.address + " callType = " +
                    dialRequest.callDetails.callType + " callDomain = " +
                    dialRequest.callDetails.callDomain + " isConferenceUri = " +
                    dialRequest.isConferenceUri + "isCallPull =" + dialRequest.isCallPull +
                    " isEncrypted = " + dialRequest.isEncrypted);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdString + " to IImsRadio: Exception: " + ex);
        }
    }

    private boolean sendErrorOnImsRadioDown(IFRequest rr, String msgIdString) {
        return sendErrorOnImsRadioDown(rr, msgIdString, null);
    }

    /**
     * If the Ims Radio service is down, send the error to clients and release the IFRequest
     *
     * @param rr IFRequest containing the token id, request type, request id, etc.
     * @return boolean - true if error was sent on Ims Radio service down, false if Service is up.
     * @param ret Result object of the request to be passed back to the client
     */
    private boolean sendErrorOnImsRadioDown(IFRequest rr, String msgIdString, Object ret) {
        if (imsRadioV10() == null) {
            Log.e(this, "ImsRadio HAL is not available. Can't send " + msgIdString + " to QCRIL");
            rr.onError(ImsErrorCode.RADIO_NOT_AVAILABLE, ret);
            rr.release();
            return true;
        }
        return false;
    }

    public void
    dial(String address, EmergencyCallInfo eInfo, int clirMode, CallDetails callDetails,
            boolean isEncrypted, Message result) {
        log("Dial Request - address= " + Log.pii(address) + "clirMode= " + clirMode
                + " callDetails= " + callDetails + " isEncrypted= " + isEncrypted);
        final int msgId = (eInfo == null) ? MessageId.REQUEST_DIAL
                                          : MessageId.REQUEST_EMERGENCY_DIAL;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, result);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        vendor.qti.hardware.radio.ims.V1_4.DialRequest dialRequest =
                ImsRadioUtils.buildDialRequest(address, clirMode, callDetails, isEncrypted);
        try {
            logSolicitedRequest(rr);
            if (eInfo != null && imsRadioV15() != null) {
                int categories = ImsRadioUtilsV15.mapEmergencyServiceCategoryToHal(
                        eInfo.getEmergencyServiceCategories());
                int route = ImsRadioUtilsV15.mapEmergencyCallRoutingToHal(
                        eInfo.getEmergencyCallRouting());
                ArrayList<String> urns = new ArrayList<>(eInfo.getEmergencyUrns());
                log("Dialing emergency call");
                imsRadioV15().emergencyDial(rr.mSerial, dialRequest, categories,
                    urns, route, eInfo.hasKnownUserIntentEmergency(),
                    eInfo.isEmergencyCallTesting());
            } else {
                vendor.qti.hardware.radio.ims.V1_4.IImsRadio imsRadio = imsRadioV14();
                if (imsRadio != null) {
                    imsRadio.dial_1_4(rr.mSerial, dialRequest);
                } else {
                    Log.w(this, "mImsRadio V1.4 is null. invoking mImsRadio.dial()");
                    imsRadioV10().dial(rr.mSerial, ImsRadioUtils.migrateFromDialRequestV14(
                            dialRequest));
                }
            }
        Log.v(this, msgIdString + " request to IImsRadio - token = " + rr.mSerial +
                "address = " + dialRequest.address + "callType =" +
                dialRequest.callDetails.callType + "callDomain =" +
                dialRequest.callDetails.callDomain + "isConferenceUri =" +
                dialRequest.isConferenceUri + "isCallPull =" + dialRequest.isCallPull +
                "isEncrypted =" + dialRequest.isEncrypted +
                "rttMode =" + dialRequest.callDetails.rttMode);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdString + "to IImsRadio: Exception: " + ex);
        }
    }

    public void
    acceptCall(Message result, int callType) {
        acceptCall(result, callType, RttMode.RTT_MODE_DISABLED);
    }

    public void
    acceptCall(Message result, int callType, int rttMode) {
        acceptCall(result, callType, IpPresentation.IP_PRESENTATION_NUM_DEFAULT, rttMode);
    }

    public void
    acceptCall(Message result, int callType, int ipPresentation, int rttMode) {
        final int msgId = MessageId.REQUEST_ANSWER;
        final String msgIdString = msgIdToString(msgId);

        IFRequest rr = IFRequest.obtain(msgId, result);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        try {
            logSolicitedRequest(rr);
            imsRadioV10().answer(rr.mSerial, ImsRadioUtils.callTypeToHal(callType),
            ipPresentation, rttMode);
            Log.i(this, "rtt mode : " + rttMode + " and ipPresentation: " +
                    ipPresentation + " to HAL");
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdString + "request to IImsRadio: Exception: " + ex);
        }
    }

    public void deflectCall(int index, String number, Message result) {
        logv("deflect call to: " + number + "connid:" + index);
        final int msgId = MessageId.REQUEST_DEFLECT_CALL;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, result);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        DeflectRequestInfo deflectRequestInfo = new DeflectRequestInfo();
        deflectRequestInfo.connIndex = index;
        deflectRequestInfo.number = number;
        try {
            logSolicitedRequest(rr);
            imsRadioV10().deflectCall(rr.mSerial, deflectRequestInfo);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdString + "request to IImsRadio: Exception: " + ex);
        }
    }

    public void sendSms(int messageRef, String format, String smsc,
                        boolean isRetry, byte[] pdu, Message result){
        Log.i(this,"sendSms over IImsRadio with format:" + format);

        final int msgId = MessageId.REQUEST_SEND_IMS_SMS;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, result);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);

        ImsSmsMessage imsSms = ImsRadioUtils.buildImsSms(messageRef, format,
                smsc, isRetry, pdu);
        try {
            logSolicitedRequest(rr);
            imsRadioV12().sendImsSms(rr.mSerial, imsSms);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdToString(msgId) + "to ImsRadioV12: Exception: " + ex.getMessage());
        }
    }

    public void acknowledgeSms(int messageRef, int result, Message target){
        Log.i(this,"acknowledgeSms: messageRef: " + messageRef + " result: " + result);

        final int msgId = MessageId.REQUEST_ACK_IMS_SMS;
        final String msgIdString = msgIdToString(msgId);
        // Don't queue this IFRequest in the request list as the response is not
        //sent by the hidl service.
        IFRequest rr = IFRequest.obtain(msgId, target);
        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        int deliverStatus = ImsRadioUtils.imsSmsDeliverStatusToHidl(result);
        try {
            logSolicitedRequest(rr);
            vendor.qti.hardware.radio.ims.V1_5.IImsRadio imsRadioV15 = imsRadioV15();
            if(imsRadioV15 != null) {
                imsRadioV15().acknowledgeSms_1_5(rr.mSerial, messageRef, deliverStatus);
            } else {
                Log.w(this, "ImsRadioV15 is null. Invoking ImsRadioV12.acknowledgeSms()");
                imsRadioV12().acknowledgeSms(rr.mSerial, messageRef, deliverStatus);
            }
        } catch (Exception ex) {
            Log.e(this, msgIdToString(msgId) + "request to ImsRadio: Exception: " + ex);
        }
        rr.release();
    }

    public void acknowledgeSmsReport(int messageRef, int result, Message target){
        Log.i(this,"acknowledgeSmsReport: messageRef: " + messageRef + " result: " + result);

        final int msgId = MessageId.REQUEST_ACK_IMS_SMS_STATUS_REPORT ;
        final String msgIdString = msgIdToString(msgId);
        //Don't queue this IFRequest in the request list as the response is not
        //sent by the hidl service.
        IFRequest rr = IFRequest.obtain(msgId, target);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        int statusReportStatus = ImsRadioUtils.imsSmsStatusReportStatusToHidl(result);
        try {
            logSolicitedRequest(rr);
            imsRadioV12().acknowledgeSmsReport(rr.mSerial, messageRef, statusReportStatus);
        } catch (Exception ex) {
            Log.e(this, msgIdToString(msgId) + "to ImsRadioV12: Exception: " + ex);
        }
        rr.release();
    }

    public String getSmsFormat(){
        try {
            return imsRadioV12().getSmsFormat();
        } catch (Exception ex) {
            Log.e(this, "Failed to getSmsFormat. Exception " + ex);
        }
        return null;
    }

    public synchronized void sendGeolocationInfo(double lat, double lon,
            Address address, Message result) {
        Log.i(this,"sendGeolocationInfo: lat: " + lat + " lon: " + lon + " address: " + address);
        final int msgId = MessageId.REQUEST_SEND_GEOLOCATION_INFO;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, result);
        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }
        queueRequest(rr);

        vendor.qti.hardware.radio.ims.V1_2.AddressInfo addressInfo =
                ImsRadioUtils.getHidlAddressInfo(lat, lon, address);
        try {
            logSolicitedRequest(rr);
            vendor.qti.hardware.radio.ims.V1_2.IImsRadio imsRadioV12 = imsRadioV12();
            if (imsRadioV12 != null) {
                imsRadioV12.sendGeolocationInfo_1_2(rr.mSerial, lat, lon, addressInfo);
            } else {
                imsRadioV10().sendGeolocationInfo(rr.mSerial, lat, lon,
                        ImsRadioUtils.migrateAddressToV10(addressInfo));
            }
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdString + "to IImsRadio: Exception: " + ex);
        }
    }

    public void
    hangupConnection(int index, Message result) {
            hangupWithReason(index, null, null, false, Integer.MAX_VALUE, null, result);
    }

    public void
    hangupWithReason(int connectionId, String userUri, String confUri,
            boolean mpty, int failCause, String errorInfo, Message result) {
        final int msgId = MessageId.REQUEST_HANGUP;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, result);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        vendor.qti.hardware.radio.ims.V1_3.HangupRequestInfo hangup =
                ImsRadioUtils.buildHangupRequest(connectionId, userUri,
                    confUri, mpty, failCause, errorInfo);
        try {
            logSolicitedRequest(rr);
            vendor.qti.hardware.radio.ims.V1_3.IImsRadio imsRadioV13 = imsRadioV13();
            if (imsRadioV13 != null) {
                imsRadioV13.hangup_1_3(rr.mSerial, hangup);
            } else {
                Log.w(this, "ImsRadioV13 is null. Invoking ImsRadioV10.hangup()");
                imsRadioV10().hangup(rr.mSerial,
                    ImsRadioUtils.migrateHangupRequestInfoFromV13(hangup));
            }
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdString + "request to IImsRadio: Exception: " + ex);
        }
    }

    public void queryServiceStatus(Message result) {
        final int msgId = MessageId.REQUEST_QUERY_SERVICE_STATUS;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, result);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        try {
            logSolicitedRequest(rr);
            imsRadioV10().queryServiceStatus(rr.mSerial);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, "Serivce status query request to IImsRadio: Exception: " + ex);
        }
    }

    public void setServiceStatus(Message result, ArrayList<CapabilityStatus> capabilityStatusList,
            int restrictCause) {
        final int msgId = MessageId.REQUEST_SET_SERVICE_STATUS;
        final String msgIdString = msgIdToString(msgId);

        vendor.qti.hardware.radio.ims.V1_6.IImsRadio imsRadio = imsRadioV16();
        if (imsRadio != null) {
            IFRequest rr = IFRequest.obtain(msgId, result);
            if (sendErrorOnImsRadioDown(rr, msgIdString, capabilityStatusList)) {
                return;
            }
            /* Pack required information into SomeArgs.
             * This data will be retrieved in corresponding response callback.
             * Note that we are also packing Message object passed to
             * this function which will be used to reply back to the sender.
             */
            SomeArgs setCapArgs = SomeArgs.obtain();
            // pack original message
            setCapArgs.arg1 = Message.obtain(result);
            // pack the capability list
            setCapArgs.arg2 = capabilityStatusList;
            // create a new message such that it holds setCapArgs object
            Message newMsg = Message.obtain();
            newMsg.obj = setCapArgs;

            /* We are ready to send the request to RIL. Set new msg as the result
             * in IFRequest and in the corresponding response callback, data from
             * new message will be retrived and will be used to reply back to sender.
             */
            rr.setResult(newMsg);
            queueRequest(rr);
            ArrayList<vendor.qti.hardware.radio.ims.V1_6.ServiceStatusInfo> serviceStatusInfoList =
                    ImsRadioUtils.buildServiceStatusInfoList(capabilityStatusList, restrictCause);
            logSolicitedRequest(rr);
            Log.i(this, msgIdString + " to ImsRadio: token -" + rr.mSerial +
                    " RestrictCause:" + restrictCause);
            try {
                imsRadio.setServiceStatus_1_6(rr.mSerial, serviceStatusInfoList);
            } catch (Exception ex) {
                // replace with original message that sender is expecting
                rr.setResult(result);
                removeFromQueueAndSendResponse(rr.mSerial, capabilityStatusList);
                Log.e(this, "SetServiceStatus request to IImsRadio: Exception: " + ex);
            }
        } else {
            for (CapabilityStatus capabilityStatus : capabilityStatusList) {
                IFRequest rr = IFRequest.obtain(msgId, result);
                if (sendErrorOnImsRadioDown(rr, msgIdString, capabilityStatusList)) {
                    return;
                }

                /* Pack required information into SomeArgs.
                 * This data will be retrieved in corresponding response callback.
                 * Note that we are also packing Message object passed to
                 * this function which will be used to reply back to the sender.
                 */
                SomeArgs setCapArgs = SomeArgs.obtain();
                // pack original message
                setCapArgs.arg1 = Message.obtain(result);

                // create a list containing only the capability that we are sending to lower layers
                ArrayList<CapabilityStatus> newCapabilityStatusList = new ArrayList<>();
                newCapabilityStatusList.add(capabilityStatus);
                // pack the new capability list
                setCapArgs.arg2 = newCapabilityStatusList;

                // create a new message such that it holds setCapArgs object
                Message newMsg = Message.obtain();
                newMsg.obj = setCapArgs;

                /* We are ready to send the request to RIL. Set new msg as the result
                 * in IFRequest and in the corresponding response callback, data from
                 * new message will be retrived and will be used to reply back to sender.
                 */
                rr.setResult(newMsg);
                queueRequest(rr);
                Log.v(this, "SetServiceStatus capabilityStatus: " + capabilityStatus);
                vendor.qti.hardware.radio.ims.V1_6.ServiceStatusInfo serviceStatusInfo =
                        ImsRadioUtils.buildServiceStatusInfo(
                        ImsRadioUtils.mapCapabilityToSrvType(capabilityStatus.getCapability()),
                        ImsRadioUtils.mapRadioTechToHidlRadioTech(capabilityStatus.getRadioTech()),
                        ImsRadioUtils.mapValueToServiceStatus(capabilityStatus.getStatus()),
                        restrictCause);

                logSolicitedRequest(rr);
                Log.i(this, msgIdString + " to ImsRadio: token -" + rr.mSerial);

                try {
                    Log.w(this, "mImsRadio V1.6 is null. invoking mImsRadio.setServiceStatus()");
                    imsRadioV10().setServiceStatus(rr.mSerial, ImsRadioUtils.
                            migrateServiceStatusInfoFromV16(serviceStatusInfo));
                } catch (Exception ex) {
                    // replace with original message that sender is expecting
                    rr.setResult(result);
                    removeFromQueueAndSendResponse(rr.mSerial, capabilityStatusList);
                    Log.e(this, "SetServiceStatus request to IImsRadio: Exception: " + ex);
                }
            }
        }
    }

    public void getImsRegistrationState(Message result) {
        final int msgId = MessageId.REQUEST_IMS_REGISTRATION_STATE;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, result);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        try {
            logSolicitedRequest(rr);
            imsRadioV10().getImsRegistrationState(rr.mSerial);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdString + "request to IImsRadio: Exception: " + ex);
        }
    }

    public void sendImsRegistrationState(int imsRegState, Message result) {
        final int msgId = MessageId.REQUEST_IMS_REG_STATE_CHANGE;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, result);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        try {
            logSolicitedRequest(rr);
            Log.i(this, msgIdString + " request to ImsRadio - token:" + rr.mSerial + " RegState" +
                    imsRegState);
            imsRadioV10().requestRegistrationChange(rr.mSerial,
                    ImsRadioUtils.regStateToHal(imsRegState));
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdString + "request to IImsRadio: Exception: " + ex);
        }
   }

    public void modifyCallInitiate(Message result, CallModify callModify) {
        logv("modifyCallInitiate callModify= " + callModify);
        final int msgId = MessageId.REQUEST_MODIFY_CALL_INITIATE;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, result);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        try {
            logSolicitedRequest(rr);
            imsRadioV10().modifyCallInitiate(rr.mSerial,
                    ImsRadioUtils.buildCallModifyInfo(callModify));
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdString + "request to IImsRadio: Exception: " + ex);
        }
    }

    public void cancelModifyCall(Message result, int callId) {
        logv("cancelModifyCall");
        final int msgId = MessageId.REQUEST_CANCEL_MODIFY_CALL;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, result);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        try {
            logSolicitedRequest(rr);
            Log.i(this, msgIdString + " request to ImsRadio - " + rr.mSerial + " callId:" +
                    callId);
            imsRadioV10().cancelModifyCall(rr.mSerial, callId);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdString + "request to IImsRadio: Exception: " + ex);
        }
    }

    public void modifyCallConfirm(Message result, CallModify callModify) {
        logv("modifyCallConfirm callModify= " + callModify);
        final int msgId = MessageId.REQUEST_MODIFY_CALL_CONFIRM;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, result);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        try {
            logSolicitedRequest(rr);
            imsRadioV10().modifyCallConfirm(rr.mSerial,
                    ImsRadioUtils.buildCallModifyInfo(callModify));
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdString + "request to IImsRadio: Exception: " + ex);
        }
    }

    public void hold(Message result, int callId) {
        final int msgId = MessageId.REQUEST_HOLD;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, result);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        try {
            logSolicitedRequest(rr);
            Log.i(this, msgIdString + " request to ImsRadio - " + rr.mSerial + " callId:" +
                    callId);
            imsRadioV10().hold(rr.mSerial, callId);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdString + "request to IImsRadio: Exception: " + ex);
        }
    }

    public void resume(Message result, int callId) {
        final int msgId = MessageId.REQUEST_RESUME;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, result);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        try {
            logSolicitedRequest(rr);
            Log.i(this, msgIdString + " request to ImsRadio - " + rr.mSerial + " callId:" +
                    callId);
            imsRadioV10().resume(rr.mSerial, callId);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdString + "request to IImsRadio: Exception: " + ex);
        }
    }

    public void conference(Message result) {
        final int msgId = MessageId.REQUEST_CONFERENCE;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, result);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        try {
            logSolicitedRequest(rr);
            imsRadioV10().conference(rr.mSerial);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdString + "request to IImsRadio: Exception: " + ex);
        }
    }

    public void explicitCallTransfer(int srcCallId, int type, String number,
            int destCallId, Message result) {
        logv("explicitCallTransfer srcCallId= " + srcCallId + " type= "+ type + " number= "+
                number + "destCallId = " + destCallId);
        final int msgId = MessageId.REQUEST_EXPLICIT_CALL_TRANSFER;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, result);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        ExplicitCallTransferInfo ectInfo = ImsRadioUtils.buildExplicitCallTransferInfo(srcCallId,
                type, number, destCallId);
        try {
            logSolicitedRequest(rr);
            imsRadioV10().explicitCallTransfer(rr.mSerial, ectInfo);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdString + "request to IImsRadio: Exception: " + ex);
        }
    }

    public void sendConfigRequest(int requestType, int item, boolean boolValue,
        int intValue, String strValue, int errorCause, Message result) {
        final String msgIdString = msgIdToString(requestType);
        IFRequest rr = IFRequest.obtain(requestType, result);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        try {
            logSolicitedRequest(rr);
            if (requestType == MessageId.REQUEST_GET_IMS_CONFIG) {
                ConfigInfo configInfo = ImsRadioUtils.buildConfigInfo(item, boolValue,
                        intValue, strValue, errorCause);
                Log.i(this, msgIdString + " request to ImsRadio: token " + rr.mSerial +
                        " item:" + configInfo.item + " boolValue:" + boolValue + " intValue:" +
                        intValue + " strValue:" + strValue + " errorCause:" + errorCause);
                imsRadioV10().getConfig(rr.mSerial, configInfo);
            } else {
                vendor.qti.hardware.radio.ims.V1_6.IImsRadio imsRadioV16 = imsRadioV16();
                vendor.qti.hardware.radio.ims.V1_6.ConfigInfo configInfo = ImsRadioUtilsV16.
                        buildConfigInfo(item, boolValue, intValue, strValue, errorCause);
                    Log.i(this, msgIdString + " request to ImsRadio: token " + rr.mSerial +
                            " item:" + configInfo.item + " boolValue:" + boolValue + " intValue:" +
                            intValue + " strValue:" + strValue + " errorCause:" + errorCause);
                if (imsRadioV16 != null) {
                    imsRadioV16.setConfig_1_6(rr.mSerial, configInfo);
                } else {
                    vendor.qti.hardware.radio.ims.V1_5.ConfigInfo configInfoV15 =
                            ImsRadioUtilsV15.migrateConfigInfoFromV16(configInfo);
                    if (imsRadioV15() != null) {
                        imsRadioV15().setConfig_1_5(rr.mSerial, configInfoV15);
                    } else {
                        imsRadioV10().setConfig(rr.mSerial, ImsRadioUtils.
                            migrateConfigInfoFromV15(configInfoV15));
                    }
                }
            }
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdString + " request to IImsRadio: Exception: " + ex);
        }
    }

    public void sendDtmf(char c, Message result) {
        final int msgId = MessageId.REQUEST_DTMF;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, result);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        DtmfInfo dtmfValue = new DtmfInfo();
        dtmfValue.dtmf = Character.toString(c);
        try {
            logSolicitedRequest(rr);
            imsRadioV10().sendDtmf(rr.mSerial, dtmfValue);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdString + "to IImsRadio: Exception: " + ex);
        }
    }

    public void startDtmf(char c, Message result) {
        final int msgId = MessageId.REQUEST_DTMF_START;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, result);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        DtmfInfo dtmfValue = new DtmfInfo();
        dtmfValue.dtmf = Character.toString(c);
        try {
            logSolicitedRequest(rr);
            imsRadioV10().startDtmf(rr.mSerial, dtmfValue);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdToString(msgId) + "to IImsRadio: Exception: " + ex);
        }
    }

    public void stopDtmf(Message result) {
        final int msgId = MessageId.REQUEST_DTMF_STOP;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, result);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        try {
            logSolicitedRequest(rr);
            imsRadioV10().stopDtmf(rr.mSerial);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdToString(msgId) + "to IImsRadio: Exception: " + ex);
        }
    }

    private Object processConfigResponse(vendor.qti.hardware.radio.ims.V1_6.ConfigInfo configInfo) {
         if (configInfo.hasBoolValue) {
             return configInfo.boolValue;
         } else if (configInfo.intValue != Integer.MAX_VALUE) {
             return configInfo.intValue;
         } else if (configInfo.stringValue != null) {
             return configInfo.stringValue;
         } else if (configInfo.errorCause != ConfigFailureCause.CONFIG_FAILURE_INVALID) {
             return ImsRadioUtils.configFailureCauseFromHal(configInfo.errorCause);
         }
         return ImsRadioUtilsV16.configInfoFromHal(configInfo);
    }

    private void disableSrvStatus() {
        Log.v(this, "disableSrvStatus");
        if (mSrvStatusRegistrations != null) {
            ImsRilException ex = new ImsRilException(ImsErrorCode.RADIO_NOT_AVAILABLE, null);
            mSrvStatusRegistrations
                    .notifyRegistrants(new AsyncResult(null, null, ex));
        }
    }

    public void setSuppServiceNotifications(boolean enable, Message result) {
        logv("setSuppServiceNotifications enable = " + enable);
        final int msgId = MessageId.REQUEST_SET_SUPP_SVC_NOTIFICATION;
        final String msgIdString = msgIdToString(msgId);

        IFRequest rr = IFRequest.obtain(msgId, result);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        try {
            int status = enable ? ServiceClassStatus.ENABLED : ServiceClassStatus.DISABLED;
            logSolicitedRequest(rr);
            imsRadioV10().setSuppServiceNotification(rr.mSerial, status);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdString + "request to IImsRadio: Exception: " + ex);
        }
    }

    public void getCLIR(Message result) {
        logv("getCLIR");
        final int msgId = MessageId.REQUEST_GET_CLIR;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, result);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        try {
            logSolicitedRequest(rr);
            imsRadioV10().getClir(rr.mSerial);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, "GetClir request to IImsRadio: Exception: " + ex);
        }
    }

    public void setCLIR(int clirMode, Message result) {
        logv("setCLIR clirmode = " + clirMode);
        final int msgId = MessageId.REQUEST_SET_CLIR;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, result);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        ClirInfo clirValue = new ClirInfo();
        clirValue.paramN = clirMode;
        try {
            logSolicitedRequest(rr);
            imsRadioV10().setClir(rr.mSerial, clirValue);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdToString(msgId) + "to IImsRadio: Exception: " + ex);
        }
    }

    public void queryCallWaiting(int serviceClass, Message response) {
        logv("queryCallWaiting serviceClass = " + serviceClass);
        final int msgId = MessageId.REQUEST_QUERY_CALL_WAITING;
        final String msgIdString = msgIdToString(msgId);

        IFRequest rr = IFRequest.obtain(msgId, response);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        try {
            logSolicitedRequest(rr);
            imsRadioV10().getCallWaiting(rr.mSerial, serviceClass);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdString + "request to IImsRadio: Exception: " + ex);
        }
    }

    public void setCallWaiting(boolean enable, int serviceClass,
            Message response) {
        logv("setCallWaiting enable = " + enable + "serviceClass = "
                + serviceClass);
        final int msgId = MessageId.REQUEST_SET_CALL_WAITING;
        final String msgIdString = msgIdToString(msgId);

        IFRequest rr = IFRequest.obtain(msgId, response);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        try {
            int serviceStatus = enable ? ServiceClassStatus.ENABLED : ServiceClassStatus.DISABLED;
            logSolicitedRequest(rr);
            imsRadioV10().setCallWaiting(rr.mSerial, serviceStatus, serviceClass);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdString + "request to IImsRadio: Exception: " + ex);
        }
    }

    public void queryIncomingCallBarring(String facility, int serviceClass, Message response) {
        suppSvcStatus(SuppSvcResponse.QUERY, facilityStringToInt(facility),
                             null, serviceClass, response);
    }

    public void setIncomingCallBarring(int operation, String facility, String[] icbNum,
            int serviceClass, Message response) {
        suppSvcStatus(operation, facilityStringToInt(facility), icbNum, serviceClass, response);
    }

    public void setCallForward(int action, int cfReason, int serviceClass,
            String number, int timeSeconds, Message response) {
        logv("setCallForward cfReason= " + cfReason + " serviceClass = "
                + serviceClass + "number = " + number + "timeSeconds = "
                + timeSeconds);
        setCallForwardInternal(Integer.MAX_VALUE, Integer.MAX_VALUE, Integer.MAX_VALUE,
                Integer.MAX_VALUE, action, cfReason, serviceClass, number, timeSeconds, response);
    }

    public void setCallForwardUncondTimer(int startHour, int startMinute, int endHour,
            int endMinute, int action, int cfReason, int serviceClass, String number,
            Message response) {
        setCallForwardInternal(startHour, startMinute, endHour, endMinute, action, cfReason,
                serviceClass, number, ZERO_SECONDS, response);
    }

    private void setCallForwardInternal(int startHour, int startMinute, int endHour,
            int endMinute, int action, int cfReason, int serviceClass, String number,
            int timeSeconds, Message response) {
        logv("setCallForwardInternal cfReason= " + cfReason + " serviceClass = " +
                serviceClass + "number = " + number + "startHour = " + startHour +
                "startMinute = " + startMinute + "endHour = " + endHour + "endMin = " +
                endMinute);
        final int msgId = MessageId.REQUEST_SET_CALL_FORWARD_STATUS;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, response);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        vendor.qti.hardware.radio.ims.V1_0.CallForwardInfo cfInfo =
                ImsRadioUtils.buildCallForwardInfo(cfReason, serviceClass, number,
                action, timeSeconds);
        if (startHour != Integer.MAX_VALUE && startMinute != Integer.MAX_VALUE) {
            cfInfo.hasCallFwdTimerStart = true;
            ImsRadioUtils.buildCallFwdTimerInfo(cfInfo.callFwdTimerStart, startHour, startMinute);
        }

        if (endHour != Integer.MAX_VALUE && endMinute != Integer.MAX_VALUE) {
            cfInfo.hasCallFwdTimerEnd = true;
            ImsRadioUtils.buildCallFwdTimerInfo(cfInfo.callFwdTimerEnd, endHour, endMinute);
        }
        try {
            logSolicitedRequest(rr);
            imsRadioV10().setCallForwardStatus(rr.mSerial, cfInfo);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdString + "to IImsRadio: Exception: " + ex);
        }
    }

    public void queryCallForwardStatus(int cfReason, int serviceClass,
            String number, Message response) {
        logv("queryCallForwardStatus cfReason= " + cfReason
                + " serviceClass = " + serviceClass + "number = " + number);
        final int msgId = MessageId.REQUEST_QUERY_CALL_FORWARD_STATUS;;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, response);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        vendor.qti.hardware.radio.ims.V1_0.CallForwardInfo cfInfo =
                ImsRadioUtils.buildCallForwardInfo(cfReason, serviceClass, number,
                STATUS_INTERROGATE, 0);
        try {
            logSolicitedRequest(rr);
            imsRadioV10().queryCallForwardStatus(rr.mSerial, cfInfo);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdString + "to IImsRadio: Exception: " + ex);
        }
    }

    public void queryCLIP(Message response) {
        logv("queryClip");
        final int msgId = MessageId.REQUEST_QUERY_CLIP;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, response);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        try {
            logSolicitedRequest(rr);
            imsRadioV10().getClip(rr.mSerial);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, "GetClip request to IImsRadio: Exception: " + ex);
        }
    }

    public void setUiTTYMode(int uiTtyMode, Message response) {
        logv("setUiTTYMode uittyMode=" + uiTtyMode);
        final int msgId = MessageId.REQUEST_SEND_UI_TTY_MODE;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, response);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        TtyInfo info = new TtyInfo();
        info.mode = ImsRadioUtils.ttyModeToHal(uiTtyMode);
        try {
            logSolicitedRequest(rr);
            imsRadioV10().setUiTtyMode(rr.mSerial, info);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdToString(msgId) + "to IImsRadio: Exception: " + ex);
        }
    }

    public void exitEmergencyCallbackMode(Message response) {
        logv("exitEmergencyCallbackMode");
        final int msgId = MessageId.REQUEST_EXIT_EMERGENCY_CALLBACK_MODE;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, response);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        try {
            logSolicitedRequest(rr);
            imsRadioV10().exitEmergencyCallbackMode(rr.mSerial);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdString + "request to IImsRadio: Exception: " + ex);
        }
    }

    @Override
    public void queryFacilityLock(String facility, String password,
            int serviceClass, Message response) {
        suppSvcStatus(SuppSvcResponse.QUERY, facilityStringToInt(facility), response);
    }

    @Override
    public void setFacilityLock(String facility, boolean lockState,
            String password, int serviceClass, Message response) {
        int operation = lockState ? SuppSvcResponse.ACTIVATE : SuppSvcResponse.DEACTIVATE;
        suppSvcStatus(operation, facilityStringToInt(facility), response);
    }

    public void getSuppSvc(String facility, Message response) {
        suppSvcStatus(SuppSvcResponse.QUERY, facilityStringToInt(facility), response);
    }

    public void setSuppSvc(String facility, boolean lockState, Message response) {
        int operation = lockState ? SuppSvcResponse.ACTIVATE : SuppSvcResponse.DEACTIVATE;
        suppSvcStatus(operation, facilityStringToInt(facility), response);
    }

    public void suppSvcStatus(int operationType, int facility, String[] icbNum,
            int serviceClassValue, Message response) {
        logv("suppSvcStatus operationType = " + operationType + " facility = "
                + facility + "serviceClassValue = " + serviceClassValue);
        suppSvcStatusInternal(operationType, facility, icbNum, null, serviceClassValue, response);
    }

    public void suppSvcStatus(int operationType, int facility, Message response) {
        logv("suppSvcStatus operationType = " + operationType + " facility = "
                + facility);
        suppSvcStatusInternal(operationType, facility, null, null, Integer.MAX_VALUE, response);
    }

    public void suppSvcStatus(int operationType, int facility, String[] icbNum, String password,
            int serviceClassValue, Message response) {
        logv("suppSvcStatus operationType = " + operationType + " facility = " + facility
                + "serviceClassValue = " + serviceClassValue);
        suppSvcStatusInternal(operationType, facility, icbNum, password, serviceClassValue,
                response);
    }

    private void suppSvcStatusInternal(int operationType, int facility, String[] inCbNumList,
            String password, int serviceClass, Message response) {
        final int msgId = MessageId.REQUEST_SUPP_SVC_STATUS;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, response);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        CbNumListInfo cbNumListInfo = ImsRadioUtils.buildCbNumListInfo(inCbNumList, serviceClass);
        try {
            logSolicitedRequest(rr);
            vendor.qti.hardware.radio.ims.V1_6.IImsRadio imsRadio = imsRadioV16();
            if (imsRadio != null) {
                imsRadio.suppServiceStatus_1_6(rr.mSerial, operationType,
                        ImsRadioUtils.facilityTypeToHal(facility), cbNumListInfo,
                        password != null ? password : "");
            } else {
                imsRadioV10().suppServiceStatus(rr.mSerial, operationType,
                        ImsRadioUtils.facilityTypeToHal(facility), cbNumListInfo);
            }
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdString + "to IImsRadio: Exception: " + ex);
        }
    }

    public void getCOLR(Message result) {
        logv("getCOLR");
        final int msgId = MessageId.REQUEST_GET_COLR;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, result);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        try {
            logSolicitedRequest(rr);
            imsRadioV10().getColr(rr.mSerial);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, "GetColr request to IImsRadio: Exception: " + ex);
        }
    }

    public void setCOLR(int presentationValue, Message result) {
        logv("setCOLR presentationValue = " + presentationValue);
        final int msgId = MessageId.REQUEST_SET_COLR;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, result);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        ColrInfo colrValue = new ColrInfo();
        colrValue.presentation = ImsRadioUtils.ipPresentationToHal(presentationValue);
        try {
            logSolicitedRequest(rr);
            imsRadioV10().setColr(rr.mSerial, colrValue);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdToString(msgId) + "to IImsRadio: Exception: " + ex);
        }
    }

    static int facilityStringToInt(String sc) {
        if (sc == null) {
            throw new RuntimeException ("invalid supplementary service");
        }

        if (sc.equals("CLIP")) {
            return SuppSvcResponse.FACILITY_CLIP;
        }
        else if (sc.equals("COLP")) {
            return SuppSvcResponse.FACILITY_COLP;
        }
        return 0;
    }

    public void getPacketCount(Message response) {
        logv("getPacketCount");
        final int msgId = MessageId.REQUEST_GET_RTP_STATISTICS;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, response);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        try {
            logSolicitedRequest(rr);
            imsRadioV10().getRtpStatistics(rr.mSerial);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdString + "request to IImsRadio: Exception: " + ex);
        }
    }

    public void getPacketErrorCount(Message response) {
        logv("getPacketErrorCount");
        final int msgId = MessageId.REQUEST_GET_RTP_ERROR_STATISTICS;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, response);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        try {
            logSolicitedRequest(rr);
            imsRadioV10().getRtpErrorStatistics(rr.mSerial);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdString + "request to IImsRadio: Exception: " + ex);
        }
    }

    public void getImsSubConfig(Message response) {
        logv("getImsSubConfig");
        final int msgId = MessageId.REQUEST_GET_IMS_SUB_CONFIG;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, response);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        try {
            logSolicitedRequest(rr);
            imsRadioV10().getImsSubConfig(rr.mSerial);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdString + "request to IImsRadio: Exception: " + ex);
        }
    }

    public void getWifiCallingPreference(Message response) {
        Log.i(this, "getWifiCallingPreference : Not supported");
    }

    public void setWifiCallingPreference(int wifiCallingStatus, int wifiCallingPreference,
            Message response) {
        Log.i(this, "setWifiCallingPreference : Not supported");
    }

    public void getHandoverConfig(Message response) {
        Log.i(LOG_TAG, "getHandoverConfig : Not supported");
    }

    public void setHandoverConfig(int hoConfig, Message response) {
        Log.i(LOG_TAG, "setHandoverConfig : Not supported");
    }

    public void queryVopsStatus(Message response) {
        Log.i(this, "queryVopsStatus : Not supported");
    }

    public void querySsacStatus(Message response) {
        Log.i(this, "querySsacStatus : Not supported");
    }

    public void updateVoltePref(int preference, Message response) {
        Log.i(this, "updateVoltePref : Not supported");
    }

    public void queryVoltePref(Message response) {
        Log.i(this, "queryVoltePref : Not supported");
    }

    /* ImsPhoneCommandsInterface API's */
    public void rejectCall(Message result) {
        Log.i(this, "rejectCall : Not supported");
    }

    public void
    getLastCallFailCause(Message result) {
        Log.i(this, "getLastCallFailCause : Not supported");
    }

    public void hangupWaitingOrBackground(Message result) {
        Log.i(this, "hangupWaitingOrBackground : Not supported");
    }

    public void getCurrentCalls(Message result) {
        Log.i(this, "getCurrentCalls : Not supported");
    }

    public void switchWaitingOrHoldingAndActive(Message result) {
        Log.i(this, "switchWaitingOrHoldingAndActive : Not supported");
    }

    public void explicitCallTransfer(Message result) {
        Log.i(this, "explicitCallTransfer : Not supported");
    }

    public void hangupForegroundResumeBackground(Message result) {
        Log.i(this, "hangupForegroundResumeBackground : Not supported");
    }


    public void registerForRttMessage(Handler h, int what, Object obj) {
        mRttMessageRegistrants.addUnique(h, what, obj);
    }

    public void deregisterForRttMessage(Handler h) {
        mRttMessageRegistrants.remove(h);
    }

    public void registerForVoiceInfo(Handler h, int what, Object obj) {
        mVoiceInfoStatusRegistrants.addUnique(h, what, obj);
    }

    public void deregisterForVoiceInfo(Handler h) {
        mVoiceInfoStatusRegistrants.remove(h);
    }

    public void sendRttMessage(String message, Message response) {
        Log.i(this, "RTT: sendRttMessage msg = " + message);
        final int msgId = MessageId.REQUEST_SEND_RTT_MSG;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, response);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        try {
            logSolicitedRequest(rr);
            imsRadioV10().sendRttMessage(rr.mSerial, message);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdToString(msgId) + "to IImsRadio: Exception: " + ex);
        }
    }

    public void queryVirtualLineInfo(String msisdn, Message response) {
        Log.i(this, "queryVirtualLineInfo = " + msisdn);
        final int msgId = MessageId.REQUEST_QUERY_VIRTUAL_LINE_INFO;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, response);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }
        queueRequest(rr);
        try {
            logSolicitedRequest(rr);
            imsRadioV14().queryVirtualLineInfo(rr.mSerial, msisdn);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdToString(msgId) + "to IImsRadio V1.4: Exception: " + ex);
        }
    }

    public void requestMultiIdentityLinesRegistration(Collection<MultiIdentityLineInfo> linesInfo,
            Message response) {
        Log.i(this, "registerMultiIdentityLines = " + linesInfo);
        final int msgId = MessageId.REQUEST_REGISTER_MULTI_IDENTITY_LINES;
        final String msgIdString = msgIdToString(msgId);
        IFRequest rr = IFRequest.obtain(msgId, response);

        if (sendErrorOnImsRadioDown(rr, msgIdString)) {
            return;
        }

        queueRequest(rr);
        ArrayList<MultiIdentityLineInfoHal> halLinesInfo = new ArrayList<>();
        for (MultiIdentityLineInfo line : linesInfo) {
            MultiIdentityLineInfoHal lineInfo =  ImsRadioUtilsV14.
                    toMultiIdentityLineInfoHal(line);
            halLinesInfo.add(lineInfo);
        }
        try {
            logSolicitedRequest(rr);
            imsRadioV14().registerMultiIdentityLines(rr.mSerial, halLinesInfo);
        } catch (Exception ex) {
            removeFromQueueAndSendResponse(rr.mSerial);
            Log.e(this, msgIdToString(msgId) + "to IImsRadio: Exception: " + ex);
        }
    }
}
