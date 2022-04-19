/*
 * Copyright (c) 2015, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (C) 2014 The Android Open Source Project
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

package com.qualcomm.qti.internal.telephony.dataconnection;

import android.telephony.AccessNetworkConstants;
import android.telephony.CarrierConfigManager;
import android.telephony.DataFailCause;
import android.telephony.DataFailCause.FailCause;
import android.telephony.data.ApnSetting;
import android.os.SystemProperties;
import android.telephony.Rlog;
import android.telephony.ServiceState;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;

import com.android.internal.telephony.dataconnection.ApnContext;
import com.android.internal.telephony.dataconnection.DataConnection;
import com.android.internal.telephony.dataconnection.DcTracker;
import com.android.internal.telephony.GsmCdmaPhone;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.uicc.IccRecords;
import com.android.internal.util.ArrayUtils;
import com.android.internal.util.AsyncChannel;

import android.database.Cursor;
import android.content.Context;
import android.os.PersistableBundle;
import android.os.SystemProperties;
import android.provider.Telephony;

import java.util.HashSet;
import java.util.Iterator;

public final class QtiDcTracker extends DcTracker {
    private String LOG_TAG = "QtiDCT";
    private HashSet<String> mIccidSet = new HashSet<String>();
    private int mTransportType = AccessNetworkConstants.TRANSPORT_TYPE_WWAN;

    // Constructor
    public QtiDcTracker(Phone phone, int transportType) {
        super(phone, transportType);
        mTransportType = transportType;
        LOG_TAG = LOG_TAG + "-" +
                ((transportType == AccessNetworkConstants.TRANSPORT_TYPE_WWAN) ? "C" : "I");
        if (DBG) log(LOG_TAG + ".constructor");
        fillIccIdSet();
    }

    @Override
    protected void onRecordsLoadedOrSubIdChanged() {
        if (DBG) log("onRecordsLoaded: createAllApnList");
        // Just support auto attach for WWAN only
        if (mTransportType == AccessNetworkConstants.TRANSPORT_TYPE_WWAN) {
            mAutoAttachOnCreationConfig = mPhone.getContext().getResources()
                    .getBoolean(com.android.internal.R.bool.config_auto_attach_data_on_creation);
        }

        createAllApnList();
        setDataProfilesAsNeeded();
        if (mIccRecords.get() != null && mIccRecords.get().getEssentialRecordsLoaded()) {
            setInitialAttachApn();
        }
        mPhone.notifyDataConnection();
        setupDataOnAllConnectableApns(Phone.REASON_SIM_LOADED);
    }

    @Override
    protected boolean allowInitialAttachForOperator() {
        IccRecords r = mIccRecords.get();
        String iccId = (r != null) ? r.getIccId() : "";
        if (iccId != null) {
            Iterator<String> itr = mIccidSet.iterator();
            while (itr.hasNext()) {
                if (iccId.contains(itr.next())) {
                    return false;
                }
            }
        }
        return true;
    }

    private void fillIccIdSet() {
        mIccidSet.add("8991840");
        mIccidSet.add("8991854");
        mIccidSet.add("8991855");
        mIccidSet.add("8991856");
        mIccidSet.add("8991857");
        mIccidSet.add("8991858");
        mIccidSet.add("8991859");
        mIccidSet.add("899186");
        mIccidSet.add("8991870");
        mIccidSet.add("8991871");
        mIccidSet.add("8991872");
        mIccidSet.add("8991873");
        mIccidSet.add("8991874");
    }

    @Override
    protected void onVoiceCallEnded() {
        if (DBG) log("onVoiceCallEnded");
        mInVoiceCall = false;
        if (isConnected()) {
            if (!mPhone.getServiceStateTracker().isConcurrentVoiceAndDataAllowed()) {
                startNetStatPoll();
                startDataStallAlarm(DATA_STALL_NOT_SUSPECTED);
                mPhone.notifyDataConnection();
            } else {
                // clean slate after call end.
                resetPollStats();
            }
        }
        //Allow data call retry only on DDS sub
        if (mPhone.getSubId() == SubscriptionManager.getDefaultDataSubscriptionId()) {
            // reset reconnect timer
            setupDataOnAllConnectableApns(Phone.REASON_VOICE_CALL_ENDED);
        }
    }

    @Override
    protected void log(String s) {
        Rlog.d(LOG_TAG, "[" + mPhone.getPhoneId() + "]" + s);
    }

    @Override
    /** Return new data connection */
    protected DataConnection createDataConnection() {
        if (DBG) log("createDataConnection Ex");

        int id = mUniqueIdGenerator.getAndIncrement();
        DataConnection conn = QtiDataConnection.makeDataConnection (mPhone, id,
                this, mDataServiceManager, mDcTesterFailBringUpAll, mDcc);
        mDataConnections.put(id, conn);
        if (DBG) log("createDataConnection() X id=" + id + " dc=" + conn);
        return conn;
    }

    @Override
    protected boolean isPermanentFailure(@FailCause int dcFailCause) {
        if (mPhone.getContext().getResources().getBoolean(
                com.android.internal.R.bool.config_pdp_retry_for_29_33_55_enabled) &&
                dcFailCause == DataFailCause.MULTI_CONN_TO_SAME_PDN_NOT_ALLOWED) {
            if (DBG) log("isPermanentFailure: MULTI_CONN_TO_SAME_PDN_NOT_ALLOWED");
            return true;
        }

        return super.isPermanentFailure(dcFailCause);
    }
}
