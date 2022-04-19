/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 */
/*
 * Copyright (C) 2018 The Android Open Source Project
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
 */

package com.qualcomm.qti.networksetting;

import android.annotation.IntDef;
import android.telephony.AccessNetworkConstants.AccessNetworkType;
import android.telephony.CellInfo;
import android.telephony.NetworkScan;
import android.telephony.NetworkScanRequest;
import android.telephony.RadioAccessSpecifier;
import android.telephony.RadioAccessFamily;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.telephony.TelephonyScanManager;
import android.util.Log;

import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;
import com.android.internal.telephony.CellNetworkScanResult;

import java.lang.annotation.Retention;
import java.lang.annotation.RetentionPolicy;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.Executor;
import java.util.stream.Collectors;

/**
 * A helper class that builds the common interface and performs the network scan for two different
 * network scan APIs.
 */
public class NetworkScanHelper {
    public static final String TAG = "NetworkScanHelper";
    private static final boolean DBG = true;
    // Based on current implementation, this flag can't be reset when requesting to stop scan
    // because the flow of stopping scan depends on modem's status.
    private boolean mIsScanInProgress = false;

    private int RAF_GSM = RadioAccessFamily.RAF_GSM |
        RadioAccessFamily.RAF_GPRS |
        RadioAccessFamily.RAF_EDGE;

   private static final int RAF_WCDMA = RadioAccessFamily.RAF_HSUPA |
        RadioAccessFamily.RAF_HSDPA |
        RadioAccessFamily.RAF_HSPA |
        RadioAccessFamily.RAF_HSPAP |
        RadioAccessFamily.RAF_UMTS;

   private static final int RAF_LTE = RadioAccessFamily.RAF_LTE |
        RadioAccessFamily.RAF_LTE_CA;

   private static final int RAF_TD_SCDMA = RadioAccessFamily.RAF_TD_SCDMA;

   private static final int RAF_NR = RadioAccessFamily.RAF_NR;

   private static final int INVALID_NETWORK_MODE = -1;

    /**
     * Callbacks interface to inform the network scan results.
     */
    public interface NetworkScanCallback {
        /**
         * Called when the results is returned from {@link TelephonyManager}. This method will be
         * called at least one time if there is no error occurred during the network scan.
         *
         * <p> This method can be called multiple times in one network scan, until
         * {@link #onComplete()} or {@link #onError(int)} is called.
         *
         * @param results
         */
        void onResults(List<CellInfo> results);

        /**
         * Called when the current network scan process is finished. No more
         * {@link #onResults(List)} will be called for the current network scan after this method is
         * called.
         */
        void onComplete();

        /**
         * Called when an error occurred during the network scan process.
         *
         * <p> There is no more result returned from {@link TelephonyManager} if an error occurred.
         *
         * <p> {@link #onComplete()} will not be called if an error occurred.
         *
         * @see {@link android.telephony.NetworkScan.ScanErrorCode}
         */
        void onError(int errorCode);
    }

    @Retention(RetentionPolicy.SOURCE)
    @IntDef({NETWORK_SCAN_TYPE_WAIT_FOR_ALL_RESULTS, NETWORK_SCAN_TYPE_INCREMENTAL_RESULTS})
    public @interface NetworkQueryType {}

    /**
     * Performs the network scan using {@link TelephonyManager#getAvailableNetworks()}. The network
     * scan results won't be returned to the caller until the network scan is completed.
     *
     * <p> This is typically used when the modem doesn't support the new network scan api
     * {@link TelephonyManager#requestNetworkScan(
     * NetworkScanRequest, Executor, TelephonyScanManager.NetworkScanCallback)}.
     */
    public static final int NETWORK_SCAN_TYPE_WAIT_FOR_ALL_RESULTS = 1;

    /**
     * Performs the network scan using {@link TelephonyManager#requestNetworkScan(
     * NetworkScanRequest, Executor, TelephonyScanManager.NetworkScanCallback)} The network scan
     * results will be returned to the caller periodically in a small time window until the network
     * scan is completed. The complete results should be returned in the last called of
     * {@link NetworkScanCallback#onResults(List)}.
     *
     * <p> This is recommended to be used if modem supports the new network scan api
     * {@link TelephonyManager#requestNetworkScan(
     * NetworkScanRequest, Executor, TelephonyScanManager.NetworkScanCallback)}
     */
    public static final int NETWORK_SCAN_TYPE_INCREMENTAL_RESULTS = 2;

    /** The constants below are used in the async network scan. */
    private static final boolean INCREMENTAL_RESULTS = true;
    private static final int SEARCH_PERIODICITY_SEC = 5;
    private static final int MAX_SEARCH_TIME_SEC = 254;
    private static final int INCREMENTAL_RESULTS_PERIODICITY_SEC = 3;

    private static NetworkScanRequest mNetworkScanRequest;

    private final NetworkScanCallback mNetworkScanCallback;
    private final TelephonyManager mTelephonyManager;
    private final TelephonyScanManager.NetworkScanCallback mInternalNetworkScanCallback;
    private final Executor mExecutor;

    private NetworkScan mNetworkScanRequester;

    public NetworkScanHelper(TelephonyManager tm, NetworkScanCallback callback, Executor executor) {
        mTelephonyManager = tm;
        mNetworkScanCallback = callback;
        mInternalNetworkScanCallback = new NetworkScanCallbackImpl();
        mExecutor = executor;
    }

    /**
     * Performs a network scan for the given type {@code type}.
     * {@link #NETWORK_SCAN_TYPE_INCREMENTAL_RESULTS} is recommended if modem supports
     * {@link TelephonyManager#requestNetworkScan(
     * NetworkScanRequest, Executor, TelephonyScanManager.NetworkScanCallback)}.
     *
     * @param type used to tell which network scan API should be used.
     */
    public void startNetworkScan(@NetworkQueryType int type, int subId) {
        if (type == NETWORK_SCAN_TYPE_WAIT_FOR_ALL_RESULTS) {
            Log.e(TAG, "unexpected scan type!");
        } else if (type == NETWORK_SCAN_TYPE_INCREMENTAL_RESULTS) {
            if (DBG) Log.d(TAG, "start network scan async");
            if (mIsScanInProgress) {
                if (DBG) Log.d(TAG, "Scan is already in progress, skipping new request");
                return;
            }
            mIsScanInProgress = true;
            createNetworkScanRequest(subId);
            mNetworkScanRequester = mTelephonyManager.requestNetworkScan(
                    mNetworkScanRequest,
                    mExecutor,
                    mInternalNetworkScanCallback);
        }
    }

    //Get preferred network mode based on subId
    private int getPreferredNetworkModeForSubId(int subId) {

        int mode;
        int phoneId = SubscriptionManager.getPhoneId(subId);
        if (phoneId == SubscriptionManager.INVALID_PHONE_INDEX) {
            Log.e(TAG, "Invalid Phone Id, SubId = " + subId);
            return INVALID_NETWORK_MODE;
        }

        Phone phone = PhoneFactory.getPhone(phoneId);

        mode = android.provider.Settings.Global.getInt(
                phone.getContext().getContentResolver(),
                android.provider.Settings.Global.PREFERRED_NETWORK_MODE + subId,
                Phone.PREFERRED_NT_MODE);
        if (DBG) Log.d(TAG, "getPreferredNetworkModeForSubId: phoneNwMode = " + mode +
                " subId = "+ subId);
        return mode;
    }

    private void createNetworkScanRequest(int subId) {

        int nwMode = getPreferredNetworkModeForSubId(subId);
        if (nwMode == INVALID_NETWORK_MODE) {
            Log.e(TAG, "Invalid network mode");
            return;
        }

        int raf = RadioAccessFamily.getRafFromNetworkType(nwMode);

        ArrayList<Integer> accessNwType = new ArrayList<Integer>();

        if ((raf & RAF_GSM) == RAF_GSM) {
            accessNwType.add(AccessNetworkType.GERAN);
        }

        if ((raf & RAF_WCDMA) == RAF_WCDMA) {
            accessNwType.add(AccessNetworkType.UTRAN);
        }

        if ((raf & RAF_TD_SCDMA) == RAF_TD_SCDMA) {
            accessNwType.add(AccessNetworkType.UTRAN);
        }

        if ((raf & RAF_LTE) == RAF_LTE || (raf & RAF_NR) == RAF_NR) {
            accessNwType.add(AccessNetworkType.EUTRAN);
        }

        Log.d(TAG, "Network = " + nwMode);

        RadioAccessSpecifier[] mRadioAccessSpecifiers =
                new RadioAccessSpecifier[accessNwType.size()];

            for (int i = 0; i <  accessNwType.size(); i++) {
                mRadioAccessSpecifiers [i] = new RadioAccessSpecifier(
                        accessNwType.get(i),
                        null /* bands */,
                        null /* channels */);
                Log.d(TAG, "Acess network type: " + accessNwType.get(i));
            }

            mNetworkScanRequest = new NetworkScanRequest(
                    NetworkScanRequest.SCAN_TYPE_ONE_SHOT,
                    mRadioAccessSpecifiers,
                    SEARCH_PERIODICITY_SEC,
                    MAX_SEARCH_TIME_SEC,
                    INCREMENTAL_RESULTS,
                    INCREMENTAL_RESULTS_PERIODICITY_SEC,
                    null /*List of PLMN ids (MCC-MNC)*/);

     }

    /**
     * Stops the network scan of type {@link #NETWORK_SCAN_TYPE_INCREMENTAL_RESULTS}.
     */
    public void stopNetworkQuery() {
        if (DBG) Log.d(TAG, "stop network scan async");
        if (mNetworkScanRequester != null) {
            mNetworkScanRequester.stopScan();
        }
    }

    private void onResults(List<CellInfo> cellInfos) {
        mNetworkScanCallback.onResults(cellInfos);
    }

    private void onComplete() {
        mNetworkScanCallback.onComplete();
    }

    private void onError(int errCode) {
        mNetworkScanCallback.onError(errCode);
    }

    private final class NetworkScanCallbackImpl extends TelephonyScanManager.NetworkScanCallback {
        public void onResults(List<CellInfo> results) {
            if (DBG) Log.d(TAG, "async scan onResults() results = " + results);
            NetworkScanHelper.this.onResults(results);
        }

        public void onComplete() {
            if (DBG) Log.d(TAG, "async scan onComplete()");
            mIsScanInProgress = false;
            NetworkScanHelper.this.onComplete();
        }

        public void onError(@NetworkScan.ScanErrorCode int errCode) {
            if (DBG) Log.d(TAG, "async scan onError() errorCode = " + errCode);
            mIsScanInProgress = false;
            NetworkScanHelper.this.onError(errCode);
        }
    }
}
