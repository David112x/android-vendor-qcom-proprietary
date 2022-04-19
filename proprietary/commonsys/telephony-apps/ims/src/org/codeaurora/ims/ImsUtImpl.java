/*
 * Copyright (c) 2015, 2016, 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 * Apache license notifications and license are retained
 * for attribution purposes only.
 *
 * Copyright (c) 2006 The Android Open Source Project
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

package org.codeaurora.ims;

import com.android.ims.ImsUtInterface;
import com.android.internal.annotations.VisibleForTesting;
import com.qualcomm.ims.utils.Log;
import org.codeaurora.telephony.utils.AsyncResult;

import android.Manifest;
import android.content.Context;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.telephony.ims.ImsCallForwardInfo;
import android.telephony.ims.ImsReasonInfo;
import android.telephony.ims.ImsSsData;
import android.telephony.ims.ImsSsInfo;
import android.telephony.ims.ImsUtListener;
import android.telephony.ims.stub.ImsUtImplBase;

import java.util.List;
import java.util.ArrayList;

import org.codeaurora.ims.ImsCallUtils;
import org.codeaurora.ims.ImsPhoneCommandsInterface;
import org.codeaurora.telephony.utils.CallForwardInfo;

public class ImsUtImpl extends ImsUtImplBase {

    private static final String LOG_TAG = "ImsUtImpl";
    private static final int MAX_REQUESTS_PENDING = 50; // TODO: Verify and set proper value!

    // Supplementary Service Events
    private static final int EVENT_QUERY_CF    = 1;
    private static final int EVENT_UPDATE_CF   = 2;
    private static final int EVENT_QUERY_CW    = 3;
    private static final int EVENT_UPDATE_CW   = 4;
    private static final int EVENT_QUERY_CLIR  = 5;
    private static final int EVENT_UPDATE_CLIR = 6;
    private static final int EVENT_QUERY_CLIP  = 7;
    private static final int EVENT_UPDATE_CLIP = 8;
    private static final int EVENT_QUERY_COLR  = 9;
    private static final int EVENT_UPDATE_COLR = 10;
    private static final int EVENT_QUERY_COLP  = 11;
    private static final int EVENT_UPDATE_COLP = 12;
    private static final int EVENT_QUERY_CB    = 13;
    private static final int EVENT_UPDATE_CB   = 14;
    private static final int EVENT_UNSOL_ON_SS = 15;

    // Used for various supp. services APIs.
    // See 27.007 +CCFC or +CLCK
    static final int SERVICE_CLASS_NONE       = 0; // no user input
    static final int SERVICE_CLASS_VOICE      = (1 << 0);
    static final int SERVICE_CLASS_DATA       = (1 << 1); //synonym for 16+32+64+128
    static final int SERVICE_CLASS_FAX        = (1 << 2);
    static final int SERVICE_CLASS_SMS        = (1 << 3);
    static final int SERVICE_CLASS_DATA_SYNC  = (1 << 4);
    static final int SERVICE_CLASS_DATA_ASYNC = (1 << 5);
    static final int SERVICE_CLASS_PACKET     = (1 << 6);
    static final int SERVICE_CLASS_PAD        = (1 << 7);
    static final int SERVICE_CLASS_MAX        = (1 << 7); // Max SERVICE_CLASS value

    // Call forwarding 'reason' values.
    static final int CF_REASON_UNCONDITIONAL    = 0;
    static final int CF_REASON_BUSY             = 1;
    static final int CF_REASON_NO_REPLY         = 2;
    static final int CF_REASON_NOT_REACHABLE    = 3;
    static final int CF_REASON_ALL              = 4;
    static final int CF_REASON_ALL_CONDITIONAL  = 5;

    // UT Error strings the lower layers can send as part of response exception.
    static final String UT_ERROR_GENERIC             = "E_GENERIC_FAILURE";
    static final String UT_ERROR_NETWORK_UNSUPPORTED = "E_NETWORK_NOT_SUPPORTED";
    static final String UT_ERROR_FDN_FAILURE = "E_FDN_CHECK_FAILURE";
    static final String UT_ERROR_SS_MODIFIED_TO_DIAL = "E_SS_MODIFIED_TO_DIAL";
    static final String UT_ERROR_SS_MODIFIED_TO_USSD = "E_SS_MODIFIED_TO_USSD";
    static final String UT_ERROR_SS_MODIFIED_TO_SS = "E_SS_MODIFIED_TO_SS";
    static final String UT_ERROR_SS_MODIFIED_TO_DIAL_VIDEO = "E_SS_MODIFIED_TO_DIAL_VIDEO";

    /*
     * Supplementary Service Query Bundle Keys
     * Used for supp. serv. query responses into the ssInfo Bundle.
     * It's duplicate code that it should be kept in sync with the
     * source {@link ImsPhoneMmiCode.UT_BUNDLE_KEY_CLIR}.
     */
    public static final String UT_BUNDLE_KEY_CLIR = "queryClir";

    /*
     * It's duplicate code that it should be kept in sync with the
     * source {@link ImsPhoneMmiCode.UT_BUNDLE_KEY_SSINFO}.
     */
    public static final String UT_BUNDLE_KEY_SSINFO = "imsSsInfo";

    // Permissions used by class.
    private final String MODIFY_PHONE_STATE = Manifest.permission.MODIFY_PHONE_STATE;
    private final String READ_PRIVILEGED_PHONE_STATE = Manifest.permission.READ_PRIVILEGED_PHONE_STATE;
    private final String READ_PHONE_STATE = Manifest.permission.READ_PHONE_STATE;

    private ImsServiceSub mServiceSub;
    private ImsSenderRxr mCi;
    private Context mContext;
    private ImsUtListenerImpl mListenerImpl = new ImsUtListenerImpl();
    private Handler mHandler = new ImsUtImplHandler();
    private static int requestId = -1;
    //Callforward status
    private static final int CF_DISABLED = 0;
    private static final int CF_ENABLED = 1;

    public ImsUtImpl(ImsServiceSub serviceSub, ImsSenderRxr senderRxr, Context context) {
        mServiceSub = serviceSub;
        mCi = senderRxr;
        mContext = context;
        mCi.registerForSuppServiceIndication(mHandler, EVENT_UNSOL_ON_SS, null);
    }

    private void enforceReadPhoneState(String fn) {
        if (mContext.checkCallingOrSelfPermission(READ_PRIVILEGED_PHONE_STATE)
                != PackageManager.PERMISSION_GRANTED
                && mContext.checkCallingOrSelfPermission(READ_PHONE_STATE)
                != PackageManager.PERMISSION_GRANTED) {
            mContext.enforceCallingOrSelfPermission(READ_PHONE_STATE, fn);
        }
    }

    @VisibleForTesting
    public Handler getHandler() {
        return mHandler;
    }

    /**
     * Closes the object. This object is not usable after being closed.
     */
    @Override
    public void close() {
        mContext.enforceCallingOrSelfPermission(MODIFY_PHONE_STATE, "close");
        mCi.unregisterForSuppServiceIndication(mHandler);
        mCi = null;
        mListenerImpl = null;
        mHandler = null;
    }

    /**
     * Retrieves the configuration of the call barring.
     */
    @Override
    public int queryCallBarring(int cbType) {
        return queryCallBarringForServiceClass(cbType, SERVICE_CLASS_NONE);
    }

    /**
     * Retrieves the configuration of the call barring for specified service class.
     */
    @Override
    public int queryCallBarringForServiceClass(int cbType, int serviceClass) {
        enforceReadPhoneState("queryCallBarring");
        int id = getIdForRequest();
        if (id < 0) {
            Log.e(this, "Invalid request id for queryCallBarring.");
            // ImsUt.java treats ids < 0 as an error.
            return -1;
        }

        int facility = getFacilityFromCbType(cbType);
        if (facility == -1) {
            Log.e(this, "Unsupported call barring facility code in queryCallBarring.");
            return -1;
        }

        if (facility == SuppSvcResponse.FACILITY_BS_MT) {
            mCi.suppSvcStatus(SuppSvcResponse.QUERY,
                    facility,
                    null,
                    SERVICE_CLASS_VOICE,
                    mHandler.obtainMessage(EVENT_QUERY_CB, id, 0, this));
        } else {
            mCi.suppSvcStatus(SuppSvcResponse.QUERY,
                    facility, null, serviceClass,
                    mHandler.obtainMessage(EVENT_QUERY_CB, id, 0, this));
        }

        return id;
    }

    private int getFacilityFromCbType(int cbType) {
        // NOTE: Refer to getCBTypeFromFacility in ImsPhone.java. All codes
        //       there are converted to appropriate ImsQmiIF codes.
        if (cbType == ImsUtInterface.CB_BAOC) {
            return SuppSvcResponse.FACILITY_BAOC;
        }
        else if (cbType == ImsUtInterface.CB_BOIC) {
            return SuppSvcResponse.FACILITY_BAOIC;
        }
        else if (cbType == ImsUtInterface.CB_BOIC_EXHC) {
            return SuppSvcResponse.FACILITY_BAOICxH;
        }
        else if (cbType == ImsUtInterface.CB_BAIC) {
            return SuppSvcResponse.FACILITY_BAIC;
        }
        else if (cbType == ImsUtInterface.CB_BIC_WR) {
            return SuppSvcResponse.FACILITY_BAICr;
        }
        else if (cbType == ImsUtInterface.CB_BA_ALL) {
            return SuppSvcResponse.FACILITY_BA_ALL;
        }
        else if (cbType == ImsUtInterface.CB_BA_MO) {
            return SuppSvcResponse.FACILITY_BA_MO;
        }
        else if (cbType == ImsUtInterface.CB_BA_MT) {
            return SuppSvcResponse.FACILITY_BA_MT;
        }
        else if (cbType == ImsUtInterface.CB_BS_MT) {
            return SuppSvcResponse.FACILITY_BS_MT;
        }
        else if (cbType == ImsUtInterface.CB_BIC_ACR) {
            return SuppSvcResponse.FACILITY_BAICa;
        }
        else { // Unsupported Call Barring Code
            return -1;
        }
    }

    /**
     * Retrieves the configuration of the call forward.
     */
    @Override
    public int queryCallForward(int condition, String number) {
        return queryCFForServiceClass(condition, number, SERVICE_CLASS_VOICE);
    }

    /**
     * Retrieves the configuration of the call forward for specified service class.
     */
    public int queryCFForServiceClass(int condition, String number, int serviceClass) {
        enforceReadPhoneState("queryCallForward");
        int reason = -1;

        int id = getIdForRequest();
        if (id < 0) {
            Log.e(this, "Invalid request id for queryCallForward.");
            // ImsUt.java treats ids < 0 as an error.
            return -1;
        }

        if (condition == ImsUtInterface.CDIV_CF_UNCONDITIONAL) {
            reason = CF_REASON_UNCONDITIONAL;
        }
        else if (condition == ImsUtInterface.CDIV_CF_BUSY) {
            reason = CF_REASON_BUSY;
        }
        else if (condition == ImsUtInterface.CDIV_CF_NO_REPLY) {
            reason = CF_REASON_NO_REPLY;
        }
        else if (condition == ImsUtInterface.CDIV_CF_NOT_REACHABLE) {
            reason = CF_REASON_NOT_REACHABLE;
        }
        else if (condition == ImsUtInterface.CDIV_CF_ALL) {
            reason = CF_REASON_ALL;
        }
        else if (condition == ImsUtInterface.CDIV_CF_ALL_CONDITIONAL) {
            reason = CF_REASON_ALL_CONDITIONAL;
        }
        else if (condition == ImsUtInterface.CDIV_CF_NOT_LOGGED_IN) {
            //TODO: NOT SUPPORTED CURRENTLY.
            // It's only supported in the IMS service (CS does not define it).
            // IR.92 recommends that an UE activates both the CFNRc and the CFNL
            // (CDIV using condition not-registered) to the same target.
            reason = -1;
        }
        else {
            Log.e(this, "Invalid condition for queryCallForward.");
            return -1;
        }

        mCi.queryCallForwardStatus(reason,
                                   serviceClass,
                                   number,
                                   mHandler.obtainMessage(EVENT_QUERY_CF, id, 0, this));
        return id;
    }

    /**
     * Retrieves the configuration of the call waiting.
     */
    @Override
    public int queryCallWaiting() {
        enforceReadPhoneState("queryCallWaiting");
        int id = getIdForRequest();
        if (id < 0) {
            Log.e(this, "Invalid request id for queryCallWaiting.");
            return -1;
        }
        mCi.queryCallWaiting(SERVICE_CLASS_NONE,
                             mHandler.obtainMessage(EVENT_QUERY_CW, id, 0, this));
        return id;
    }

    /**
     * Retrieves the default CLIR setting.
     */
    @Override
    public int queryClir() {
        enforceReadPhoneState("queryCLIR");
        int id = getIdForRequest();
        if (id < 0) {
            Log.e(this, "Invalid request id for queryCLIR.");
            return -1;
        }

        mCi.getCLIR(mHandler.obtainMessage(EVENT_QUERY_CLIR, id, 0, this));
        return id;
    }

    /**
     * Retrieves the CLIP call setting.
     */
    @Override
    public int queryClip() {
        enforceReadPhoneState("queryCLIP");
        int id = getIdForRequest();
        if (id < 0) {
            Log.e(this, "Invalid request id for queryCLIP.");
            return -1;
        }

        mCi.queryCLIP(mHandler.obtainMessage(EVENT_QUERY_CLIP, id, 0, this));
        return id;
    }

    /**
     * Retrieves the COLR call setting.
     */
    @Override
    public int queryColr() {
        enforceReadPhoneState("queryCOLR");
        int id = getIdForRequest();
        if (id < 0) {
            Log.e(this, "Invalid request id for queryCOLR.");
            return -1;
        }

        mCi.getCOLR(mHandler.obtainMessage(EVENT_QUERY_COLR, id, 0, this));
        return id;
    }

    /**
     * Retrieves the COLP call setting.
     */
    @Override
    public int queryColp() {
        enforceReadPhoneState("queryCOLP");
        int id = getIdForRequest();
        if (id < 0) {
            Log.e(this, "Invalid request id for queryCLIP.");
            return -1;
        }

        mCi.getSuppSvc("COLP", //TODO: String argument required. Use like this or define somewhere?
                       mHandler.obtainMessage(EVENT_QUERY_COLP, id, 0, this));
        return id;
    }

    /*
     * we have reused CF actions because CF and CB actions are used for
     * same purpose.However,We are updating CB actions here as per proto
     * file to be in sync with lower layer.
     */
    public int getIcbAction(int action) {
        if(action == ImsPhoneCommandsInterface.CF_ACTION_DISABLE){
            return SuppSvcResponse.DEACTIVATE;
        } else if(action == ImsPhoneCommandsInterface.CF_ACTION_ENABLE) {
            return  SuppSvcResponse.ACTIVATE;
        } else if (action == ImsPhoneCommandsInterface.CF_ACTION_ERASURE) {
            return SuppSvcResponse.ERASURE;
        } else if(action == ImsPhoneCommandsInterface.CF_ACTION_REGISTRATION) {
            return SuppSvcResponse.REGISTER;
        }
        return ImsUtInterface.INVALID;
    }

    /**
     * Updates the configuration of the call barring.
     */
    @Override
    public int updateCallBarring(int cbType, int action, String[] barrList) {
        return updateCallBarringForServiceClass(cbType, action, barrList, SERVICE_CLASS_NONE);
    }

    /**
     * Updates the configuration of the call barring for specified service class.
     */
    @Override
    public int updateCallBarringForServiceClass(int cbType, int action,
            String[] barrList, int serviceClass) {
        mContext.enforceCallingOrSelfPermission(MODIFY_PHONE_STATE, "updateCallBarring");
        int id = getIdForRequest();
        if (id < 0) {
            Log.e(this, "Invalid request id for updateCallBarring.");
            // ImsUt.java treats ids < 0 as an error.
            return -1;
        }

        int facility = getFacilityFromCbType(cbType);
        if (facility == -1) {
            Log.e(this, "Unsupported call barring facility code in updateCallBarring.");
            return -1;
        }

        int cbAction = getIcbAction(action);
        // Check for ICB case.
        if (facility == SuppSvcResponse.FACILITY_BS_MT) {
            mCi.suppSvcStatus(cbAction,
                    facility,
                    barrList,
                    SERVICE_CLASS_VOICE,
                    mHandler.obtainMessage(EVENT_UPDATE_CB, id, 0, this));
        } else {
            mCi.suppSvcStatus(cbAction,
                    facility, null, serviceClass,
                    mHandler.obtainMessage(EVENT_UPDATE_CB, id, 0, this));
        }

        return id;
    }

    /**
     * Updates the configuration of the call forward.
     */
    @Override
    public int updateCallForward(int action, int condition, String number, int serviceClass,
            int timeSeconds) {
        mContext.enforceCallingOrSelfPermission(MODIFY_PHONE_STATE, "updateCallForward");
        int id = getIdForRequest();
        if (id < 0) {
            Log.e(this, "Invalid request id for updateCallForward.");
            // ImsUt.java treats ids < 0 as an error.
            return -1;
        }
        mCi.setCallForward(action,
                           condition,
                           serviceClass,
                           number,
                           timeSeconds,
                           mHandler.obtainMessage(EVENT_UPDATE_CF, id, 0, this));
        return id;
    }

    /**
     * Updates the configuration of the call forward Unconditional Timer.
     */
    public int updateCallForwardUncondTimer(int startHour, int startMinute, int endHour,
            int endMinute, int action, int condition, String number) {
        int id = getIdForRequest();
        if (id < 0) {
            Log.e(this, "Invalid request id for updateCallForwardUncondTimer.");
            // ImsUt.java treats ids < 0 as an error.
            return -1;
        }
        mCi.setCallForwardUncondTimer(startHour, startMinute, endHour, endMinute,
                           action, condition,
                           ImsPhoneCommandsInterface.SERVICE_CLASS_VOICE,
                           number,
                           mHandler.obtainMessage(EVENT_UPDATE_CF, id, 0, this));
        return id;
    }

    /**
     * Updates the configuration of the call waiting.
     */
    @Override
    public int updateCallWaiting(boolean enable, int serviceClass) {
        mContext.enforceCallingOrSelfPermission(MODIFY_PHONE_STATE, "updateCallWaiting");
        int id = getIdForRequest();
        if (id < 0) {
            Log.e(this, "Invalid request id for updateCallForward.");
            return -1;
        }
        mCi.setCallWaiting(enable,
                           serviceClass,
                           mHandler.obtainMessage(EVENT_UPDATE_CW, id, 0, this));
        return id;
    }

    /**
     * Updates the configuration of the CLIR supplementary service.
     */
    @Override
    public int updateClir(int clirMode) {
        mContext.enforceCallingOrSelfPermission(MODIFY_PHONE_STATE, "updateCLIR");
        int id = getIdForRequest();
        if (id < 0) {
            Log.e(this, "Invalid request id for updateCLIR.");
            return -1;
        }
        mCi.setCLIR(clirMode,
                    mHandler.obtainMessage(EVENT_UPDATE_CLIR, id, 0, this));
        return id;
    }

    /**
     * Updates the configuration of the CLIP supplementary service.
     */
    @Override
    public int updateClip(boolean enable) {
        mContext.enforceCallingOrSelfPermission(MODIFY_PHONE_STATE, "updateCLIP");
        int id = getIdForRequest();
        if (id < 0) {
            Log.e(this, "Invalid request id for updateCLIP.");
            return -1;
        }
        mCi.setSuppSvc("CLIP",
                       enable,
                       mHandler.obtainMessage(EVENT_UPDATE_CLIP, id, 0, this));
        return id;
    }

    /**
     * Updates the configuration of the COLR supplementary service.
     */
    @Override
    public int updateColr(int presentation) {
        mContext.enforceCallingOrSelfPermission(MODIFY_PHONE_STATE, "updateCOLR");
        int id = getIdForRequest();
        if (id < 0) {
            Log.e(this, "Invalid request id for updateCOLR.");
            return -1;
        }
        mCi.setCOLR(presentation,
                    mHandler.obtainMessage(EVENT_UPDATE_COLR, id, 0, this));
        return id;
    }

    /**
     * Updates the configuration of the COLP supplementary service.
     */
    @Override
    public int updateColp(boolean enable) {
        mContext.enforceCallingOrSelfPermission(MODIFY_PHONE_STATE, "updateCOLP");
        int id = getIdForRequest();
        if (id < 0) {
            Log.e(this, "Invalid request id for updateCOLP.");
            return -1;
        }
        mCi.setSuppSvc("COLP",
                       enable,
                       mHandler.obtainMessage(EVENT_UPDATE_COLP, id, 0, this));
        return id;
    }

    /**
     * Sets the listener.
     */
    @Override
    public void setListener(ImsUtListener listener) {
        mContext.enforceCallingOrSelfPermission(MODIFY_PHONE_STATE, "setListener");
        mListenerImpl.mListener = listener;
    }

    /**
     * Method to get a request id for a request.
     * @return requestId
     */
    private int getIdForRequest() {
        // Note: This logic is in place to handle multiple UT requests at
        //       the same time. Currently, UI supports only one request.
        requestId++;
        if (requestId >= MAX_REQUESTS_PENDING) {
            requestId = 0;
        }
        return requestId;
    }

    @VisibleForTesting
    public void setIdForRequest(int id) {
        requestId = id;
    }

    /**
     * Method to parse the exception string within an AsyncResult exception
     * and return the correct IMS reason for failure.
     * @param ar The (AsyncResult) response message from lower layers.
     * @return The ImsReasonInfo with appropriate error code.
     */
    public ImsReasonInfo getImsReasonInfoFromResponseError(AsyncResult ar) {
        if (ar == null) {
            Log.i(this, "getImsReasonInfoFromResponseError :: Null AsyncResult!");
            return null;
        }
        if (ar.exception == null) {
            Log.i(this, "getImsReasonInfoFromResponseError :: "
                    + "No exception in AsyncResult!");
            return null;
        }
        // ImsSenderRxr packs a RuntimeException with the AyncResult.
        // This is the only type we are handling currently.
        if (!(ar.exception instanceof RuntimeException)) {
            Log.i(this, "getImsReasonInfoFromResponseError :: "
                    + "Improper exception type in AsyncResult!");
            return null;
        }

        int code = ImsReasonInfo.CODE_UNSPECIFIED;
        // Create a default ImsReasonInfo to be returned if no specific error
        // message has been propagated by the lower layers.
        ImsReasonInfo genericError = new ImsReasonInfo(code, ImsReasonInfo.CODE_UNSPECIFIED, null);

        RuntimeException ex = (RuntimeException)ar.exception;
        String error = ex.getMessage();
        if (error == null) {
            Log.i(this, "getImsReasonInfoFromResponseError :: "
                    + "Null message string in exception!");
            return genericError;
        }

        // Check the error string propagated with the exception.
        // Map it to the appropriate ImsReasonInfo code.
        if (error.compareTo(UT_ERROR_GENERIC) == 0) {
            return genericError;
        } else if (error.compareTo(UT_ERROR_NETWORK_UNSUPPORTED) == 0) {
            code = ImsReasonInfo.CODE_UT_SERVICE_UNAVAILABLE;
        } else if (error.compareTo(UT_ERROR_FDN_FAILURE) == 0) {
            code = ImsReasonInfo.CODE_FDN_BLOCKED;
        } else if (error.compareTo(UT_ERROR_SS_MODIFIED_TO_DIAL) == 0) {
            code = ImsReasonInfo.CODE_UT_SS_MODIFIED_TO_DIAL;
        } else if (error.compareTo(UT_ERROR_SS_MODIFIED_TO_USSD) == 0) {
            code = ImsReasonInfo.CODE_UT_SS_MODIFIED_TO_USSD;
        } else if (error.compareTo(UT_ERROR_SS_MODIFIED_TO_SS) == 0) {
            code = ImsReasonInfo.CODE_UT_SS_MODIFIED_TO_SS;
        } else if (error.compareTo(UT_ERROR_SS_MODIFIED_TO_DIAL_VIDEO) == 0) {
            code = ImsReasonInfo.CODE_UT_SS_MODIFIED_TO_DIAL_VIDEO;
        } else {
            Log.i(this, "getImsReasonInfoFromResponseError :: "
                    + "Unrecognized exception message string!");
            return genericError;
        }

        Log.i(this, "getImsReasonInfoFromResponseError :: "
                + "ImsReasonInfo code=" + code);
        return new ImsReasonInfo(code, ImsReasonInfo.CODE_UNSPECIFIED, null);
    }

    //Handler for tracking requests sent to ImsSenderRxr.
    private class ImsUtImplHandler extends Handler {
        ImsUtImplHandler() {
            super();
        }

        private void handleCFUTResponse(AsyncResult ar, Message msg) {

            ImsCallForwardTimerInfo cfInfoList[] = (ImsCallForwardTimerInfo[]) ar.result;

            if (cfInfoList.length < 1) {
                Log.e(this, "ImsCallForwardTimerInfo[] has no elements!");
                mListenerImpl.utConfigurationQueryFailed(msg.arg1,
                        new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR,
                        ImsReasonInfo.CODE_UNSPECIFIED, null));
                return;
            }

            boolean badCfResponse = false;
            ImsCallForwardTimerInfo cfInfo;
            ImsCallForwardInfo[] callForwardInfoList
                = new ImsCallForwardInfo[cfInfoList.length];

            int callForwardStatus = CF_DISABLED;
            int condition = ImsUtInterface.INVALID;
            for (int i = 0; i < cfInfoList.length; i++) {
                cfInfo = cfInfoList[i];

                if (cfInfo.status == CF_ENABLED) {
                    callForwardStatus = CF_ENABLED; //Enabled
                }
                else if (cfInfo.status == CF_DISABLED) {
                    callForwardStatus = CF_DISABLED; //Disabled
                }
                else {
                    badCfResponse = true;
                    Log.e(this, "Bad status in Query CFUT response.");
                }

                if (cfInfo.reason == CF_REASON_UNCONDITIONAL) {
                    condition = ImsUtInterface.CDIV_CF_UNCONDITIONAL;
                }
                else {
                    badCfResponse = true;
                    Log.e(this, "Bad reason in Query CFUT response.");
                }

                if (badCfResponse) {
                    mListenerImpl.utConfigurationQueryFailed(msg.arg1,
                            new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR,
                            ImsReasonInfo.CODE_UNSPECIFIED, null));
                    return;
                }

                callForwardInfoList[i] = new ImsCallForwardInfo(condition, callForwardStatus, 0,
                        cfInfo.serviceClass, new String(cfInfo.number), 0);
            }

            mListenerImpl.utConfigurationCallForwardQueried(msg.arg1,
                    callForwardInfoList);
        }

        @Override
        public void handleMessage(Message msg) {
            Log.i(this, "Message received: what = " + msg.what);
            AsyncResult ar;

            switch (msg.what) {

                case EVENT_QUERY_CB:
                    ar = (AsyncResult) msg.obj;
                    if (ar != null) {
                        if (msg.arg1 < 0) {
                            Log.e(this, "Invalid message id received in handleMessage.");
                            return;
                        }

                        if (ar.exception != null) {
                            Log.e(this, "Query CB error");

                            if (ar.userObj != null) {
                                mListenerImpl.utConfigurationQueryFailed(msg.arg1,
                                        getImsReasonInfoFromResponseError(ar));
                            }
                        }
                        else if (ar.result != null) {
                            SuppSvcResponse response = (SuppSvcResponse) ar.result;
                            final ImsReasonInfo sipError = response.getErrorDetails();
                            if (sipError != null) {
                                Log.e(this, "SuppSvcResponse has failure for CB query.");
                                mListenerImpl.utConfigurationQueryFailed(msg.arg1,
                                    new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR,
                                    sipError.getExtraCode(), sipError.getExtraMessage()));
                                return;
                            } else if (response.getFailureCause().length() > 0) {
                                ImsReasonInfo error = new ImsReasonInfo(ImsReasonInfo.
                                        CODE_UT_NETWORK_ERROR, ImsReasonInfo.CODE_UNSPECIFIED,
                                        response.getFailureCause());
                                Log.e(LOG_TAG, "CB query failed with error = " + error);
                                mListenerImpl.utConfigurationQueryFailed(msg.arg1, error);
                                return;
                            }
                            if (response.getStatus() == SuppSvcResponse.INVALID) {
                                Log.e(this, "No service status info in response for CB query.");
                                mListenerImpl.utConfigurationQueryFailed(msg.arg1,
                                        new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR,
                                        ImsReasonInfo.CODE_UNSPECIFIED, null));
                            }
                            else {
                                if(!response.getBarredLines().isEmpty()) {
                                    List<ImsSsInfo> ssInfoArray = new ArrayList<>();
                                    List<SuppSvcResponse.BarredLines> barredLines =
                                            response.getBarredLines();
                                    for (SuppSvcResponse.BarredLines lines : barredLines) {
                                        if (!barredLines.isEmpty()) {
                                            List<SuppSvcResponse.LineStatus> line =
                                                    lines.getLines();
                                            for (SuppSvcResponse.LineStatus lineStatus : line) {
                                                ImsSsInfo.Builder ssInfoBuilder =
                                                        new ImsSsInfo.Builder(
                                                        lineStatus.getStatus());
                                                ssInfoBuilder.setIncomingCommunicationBarringNumber(
                                                        lineStatus.getNumber());
                                                ssInfoArray.add(ssInfoBuilder.build());
                                            }
                                        }
                                    }
                                    mListenerImpl.utConfigurationCallBarringQueried(
                                            msg.arg1, ssInfoArray.toArray(
                                            new ImsSsInfo[ssInfoArray.size()]));
                                } else {
                                    ImsSsInfo[] ssInfoStatus = new ImsSsInfo[1];
                                    int status = ImsSsInfo.DISABLED;
                                    if (response.getStatus() == ImsSsInfo.ENABLED) {
                                        status = ImsSsInfo.ENABLED;
                                    }
                                    ImsSsInfo.Builder imsSsInfoBuilder =
                                            new ImsSsInfo.Builder(status);
                                    ImsSsInfo ssInfo = imsSsInfoBuilder.build();
                                    ssInfoStatus[0] = ssInfo;
                                    Log.i(this, "success callback Query Anonymous CB, status= "
                                            + ssInfo.getStatus());
                                    mListenerImpl.utConfigurationCallBarringQueried(msg.arg1,
                                            ssInfoStatus);
                                }
                            }
                        }
                        else {
                            Log.e(this, "Null response received for Query CB!");
                            mListenerImpl.utConfigurationQueryFailed(msg.arg1, new ImsReasonInfo(
                                    ImsReasonInfo.CODE_UT_NETWORK_ERROR,
                                    ImsReasonInfo.CODE_UNSPECIFIED, null));
                        }
                    }
                    break;

                case EVENT_UPDATE_CB:
                    ar = (AsyncResult) msg.obj;
                    if (ar != null) {
                        if (msg.arg1 < 0) {
                            Log.e(this, "Invalid message id received in handleMessage.");
                            return;
                        }

                        if (ar.exception != null) {
                            Log.e(this, "Update CB error");

                            if (ar.userObj != null) {
                                mListenerImpl.utConfigurationUpdateFailed(msg.arg1,
                                        getImsReasonInfoFromResponseError(ar));
                            }
                        }
                        else if (ar.result != null) {
                            SuppSvcResponse response = (SuppSvcResponse) ar.result;
                            final ImsReasonInfo sipError = response.getErrorDetails();
                            if (sipError != null) {
                                Log.e(this, "SuppSvcResponse has failure for CB update.");
                                mListenerImpl.utConfigurationUpdateFailed(msg.arg1,
                                         new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR,
                                         sipError.getExtraCode(), sipError.getExtraMessage()));
                                return;
                            } else if (response.getFailureCause().length() > 0) {
                                ImsReasonInfo error = new ImsReasonInfo(ImsReasonInfo.
                                        CODE_UT_NETWORK_ERROR, ImsReasonInfo.CODE_UNSPECIFIED,
                                        response.getFailureCause());
                                Log.e(LOG_TAG, "CB update failed with error = " + error);
                                mListenerImpl.utConfigurationUpdateFailed(msg.arg1,
                                        error);
                                return;
                            }
                            mListenerImpl.utConfigurationUpdated(msg.arg1);
                        }
                        else {
                            // Null response from RIL is a valid success scenario here.
                            mListenerImpl.utConfigurationUpdated(msg.arg1);
                        }
                    }
                    break;

                case EVENT_UPDATE_CW:
                    ar = (AsyncResult) msg.obj;
                    if (ar != null) {
                        if (msg.arg1 < 0) {
                            Log.e(this, "Invalid message id received in handleMessage.");
                            return;
                        }
                        if (ar.exception != null) {
                            Log.e(this, "Update CW error");

                            if (ar.result != null) {
                                // Update CW response has failure cause information.
                                // Check for it to determine request's success of failure.
                                SuppSvcResponse response
                                    = (SuppSvcResponse) ar.result;
                                final ImsReasonInfo sipError = response.getErrorDetails();
                                if (sipError != null) {
                                    Log.d(this, "SuppSvcResponse has failure for msg.what= "
                                            + msg.what);
                                    mListenerImpl.utConfigurationUpdateFailed(msg.arg1,
                                             new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR,
                                             sipError.getExtraCode(), sipError.getExtraMessage()));
                                } else if (response.getFailureCause().length() > 0) {
                                    ImsReasonInfo error = new ImsReasonInfo(
                                            ImsReasonInfo.CODE_UT_NETWORK_ERROR,
                                            ImsReasonInfo.CODE_UNSPECIFIED,
                                            response.getFailureCause());
                                    Log.e(LOG_TAG, "SuppSvc " + msg.what + " failed with error = "
                                            + error);
                                    mListenerImpl.utConfigurationUpdateFailed(msg.arg1,
                                            error);
                                } else {
                                    Log.e(LOG_TAG, "SuppSvcResponse failure with neither"
                                            + " errordetails nor failurecause");
                                    mListenerImpl.utConfigurationUpdateFailed(msg.arg1,
                                            getImsReasonInfoFromResponseError(ar));
                                }
                            }
                            else if (ar.userObj != null) {
                                mListenerImpl.utConfigurationUpdateFailed(msg.arg1,
                                        getImsReasonInfoFromResponseError(ar));
                            } else {
                                // Nothing to pass to frameworks for this request's response.
                                Log.e(LOG_TAG, "SuppSvcResponse failure with neither ar.result"
                                         + " nor userObj");
                                mListenerImpl.utConfigurationUpdateFailed(msg.arg1,
                                        getImsReasonInfoFromResponseError(ar));
                            }
                        }
                        else {
                            Log.i(this, "Success callback called for msg.what= "
                                    + msg.what);
                            mListenerImpl.utConfigurationUpdated(msg.arg1);
                        }
                    }
                    break;

                case EVENT_UPDATE_CF:
                    ar = (AsyncResult) msg.obj;
                    if (ar == null || msg.arg1 < 0) {
                        Log.e(this, "Invalid response: ar = " +  ar + " msgId = " + msg.arg1);
                        return;
                    }

                    if (ar.exception != null) {
                        Log.e(this, "Update CF error");

                        CallForwardStatusInfo cfStatusInfo = (CallForwardStatusInfo) ar.result;
                        final ImsReasonInfo sipError = cfStatusInfo != null ?
                                cfStatusInfo.getSipErrorInfo() : null;
                        String failCause = sipError != null ? sipError.getExtraMessage() : null;
                        if (failCause != null && !(failCause.isEmpty())) {
                            // Update CF response has failure cause information.
                            mListenerImpl.utConfigurationUpdateFailed(msg.arg1,
                                    new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR,
                                    sipError.getExtraCode(), failCause));
                        } else {
                            Log.e(LOG_TAG, "Update CF failure: ar = " + ar + " sipError: " +
                                    sipError);
                            mListenerImpl.utConfigurationUpdateFailed(msg.arg1,
                                    getImsReasonInfoFromResponseError(ar));
                        }
                    } else {
                        Log.i(this, "Success callback called for msg.what= " + msg.what);
                        mListenerImpl.utConfigurationUpdated(msg.arg1);
                    }
                    break;

                case EVENT_QUERY_CF:
                    ar = (AsyncResult) msg.obj;
                    if (ar != null) {
                        if (msg.arg1 < 0) {
                            Log.e(this, "Invalid message id received in handleMessage.");
                            return;
                        }
                        if (ar.exception != null) {
                            Log.e(this, "Query CF error");
                            if (ar.userObj != null) {
                                mListenerImpl.utConfigurationQueryFailed(msg.arg1,
                                        getImsReasonInfoFromResponseError(ar));
                            }
                        }
                        else if (ar.result != null) {
                            if (ar.result instanceof ImsCallForwardTimerInfo[]) {
                                Log.i(this, "Handle CFUT response");
                                handleCFUTResponse(ar, msg);
                                return;
                            }
                            CallForwardInfo cfInfoList[] = (CallForwardInfo[]) ar.result;

                            if (cfInfoList.length < 1) {
                                Log.e(this, "CallForwardInfo[] has no elements!");
                                mListenerImpl.utConfigurationQueryFailed(msg.arg1,
                                        new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR,
                                        ImsReasonInfo.CODE_UNSPECIFIED, null));
                                return;
                            }

                            boolean badCfResponse = false;
                            CallForwardInfo cfInfo;
                            ImsCallForwardInfo[] callForwardInfoList
                                = new ImsCallForwardInfo[cfInfoList.length];

                            int callForwardStatus = CF_DISABLED;
                            int condition = ImsUtInterface.INVALID;
                            int timeSeconds = 0;
                            for (int i = 0; i < cfInfoList.length; i++) {
                                cfInfo = cfInfoList[i];

                                if (cfInfo.status == CF_ENABLED) {
                                    callForwardStatus = CF_ENABLED; // Enabled
                                }
                                else if (cfInfo.status == CF_DISABLED) {
                                    callForwardStatus = CF_DISABLED; // Disabled
                                }
                                else {
                                    badCfResponse = true;
                                    Log.e(this, "Bad status in Query CF response.");
                                }

                                if (cfInfo.reason == CF_REASON_UNCONDITIONAL) {
                                    condition = ImsUtInterface.CDIV_CF_UNCONDITIONAL;
                                }
                                else if (cfInfo.reason == CF_REASON_BUSY) {
                                    condition = ImsUtInterface.CDIV_CF_BUSY;
                                }
                                else if (cfInfo.reason == CF_REASON_NO_REPLY) {
                                    condition = ImsUtInterface.CDIV_CF_NO_REPLY;
                                    // Time present only in this case.
                                    timeSeconds = cfInfo.timeSeconds;
                                }
                                else if (cfInfo.reason == CF_REASON_NOT_REACHABLE) {
                                    condition = ImsUtInterface.CDIV_CF_NOT_REACHABLE;
                                }
                                else if (cfInfo.reason == CF_REASON_ALL) {
                                    condition = ImsUtInterface.CDIV_CF_ALL;
                                }
                                else if (cfInfo.reason == CF_REASON_ALL_CONDITIONAL) {
                                    condition = ImsUtInterface.CDIV_CF_ALL_CONDITIONAL;
                                }
                                else {
                                    badCfResponse = true;
                                    Log.e(this, "Bad reason in Query CF response.");
                                }

                                if (badCfResponse) {
                                    mListenerImpl.utConfigurationQueryFailed(
                                            msg.arg1, new ImsReasonInfo(ImsReasonInfo.
                                            CODE_UT_NETWORK_ERROR, ImsReasonInfo.CODE_UNSPECIFIED,
                                            null));
                                    return;
                                }

                                callForwardInfoList[i] = new ImsCallForwardInfo(condition,
                                        callForwardStatus, cfInfo.toa, cfInfo.serviceClass,
                                        new String(cfInfo.number), timeSeconds);
                            }

                            mListenerImpl.utConfigurationCallForwardQueried(
                                                                             msg.arg1,
                                                                             callForwardInfoList);
                        }
                        else {
                            Log.e(this, "Null response received for Query CF!");
                            mListenerImpl.utConfigurationQueryFailed(
                                    msg.arg1, new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR,
                                    ImsReasonInfo.CODE_UNSPECIFIED, null));
                        }
                    }
                    break;

                case EVENT_QUERY_CW:
                    ar = (AsyncResult) msg.obj;
                    if (ar != null) {
                        if (msg.arg1 < 0) {
                            Log.e(this, "Invalid message id received in handleMessage.");
                            return;
                        }
                        if (ar.exception != null) {
                            Log.e(this, "Query CW error");
                            if (ar.userObj != null) {
                                mListenerImpl.utConfigurationQueryFailed(
                                        msg.arg1,
                                        getImsReasonInfoFromResponseError(ar));
                            }
                        }
                        else if (ar.result != null) {
                            int[] cwResponse = (int[]) ar.result;

                            ImsSsInfo[] callWaitingInfoList = new ImsSsInfo[1];

                            int status = ImsSsInfo.DISABLED;
                            if (cwResponse[0] == SuppSvcResponse.ENABLED) {
                                if ((cwResponse[1] & SERVICE_CLASS_VOICE) == SERVICE_CLASS_VOICE) {
                                    status = ImsSsInfo.ENABLED;
                                } else {
                                    status = ImsSsInfo.DISABLED;
                                }
                            }
                            else if (cwResponse[0] == SuppSvcResponse.DISABLED) {
                                status = ImsSsInfo.DISABLED;
                            }
                            else {
                                Log.e(this, "No service status received for CallWaitingInfo.");
                                mListenerImpl.utConfigurationQueryFailed(
                                        msg.arg1, new ImsReasonInfo(ImsReasonInfo.
                                        CODE_UT_NETWORK_ERROR, ImsReasonInfo.CODE_UNSPECIFIED,
                                        null));
                                return;
                            }

                            //NOTE: Service status is VOICE by default, hence not checked.
                            ImsSsInfo.Builder imsSsInfoBuilder = new ImsSsInfo.Builder(status);
                            callWaitingInfoList[0] = imsSsInfoBuilder.build();

                            mListenerImpl.utConfigurationCallWaitingQueried(
                                                                             msg.arg1,
                                                                             callWaitingInfoList);
                        }
                        else {
                            Log.e(this, "Null response received for Query CW!");
                            mListenerImpl.utConfigurationQueryFailed(
                                    msg.arg1, new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR,
                                    ImsReasonInfo.CODE_UNSPECIFIED, null));
                        }
                    }
                    break;

                case EVENT_QUERY_CLIR:
                    ar = (AsyncResult) msg.obj;
                    if (ar != null) {
                        if (msg.arg1 < 0) {
                            Log.e(this, "Invalid message id received in handleMessage.");
                            return;
                        }
                        if (ar.exception != null) {
                            if (msg.what == EVENT_QUERY_CLIR) {
                                Log.e(this, "Query CLIR error");
                            }

                            if (ar.userObj != null) {
                                mListenerImpl.utConfigurationQueryFailed(
                                        msg.arg1,
                                        getImsReasonInfoFromResponseError(ar));
                            }
                        }
                        else if (ar.result != null) {
                            int[] clirResp = (int[]) ar.result;
                            Bundle clirInfo = new Bundle();
                            clirInfo.putIntArray(UT_BUNDLE_KEY_CLIR, clirResp);
                            Log.i(this, "Calling success callback for Query CLIR.");
                            mListenerImpl.utConfigurationQueried(msg.arg1,
                                    clirInfo);
                        }
                    }
                    break;

                case EVENT_QUERY_CLIP:
                    ar = (AsyncResult) msg.obj;
                    SuppService clipStatus = (SuppService) ar.result;
                    if (ar != null) {
                        if (msg.arg1 < 0) {
                            Log.e(this, "Invalid message id received in handleMessage.");
                            return;
                        }
                        if (ar.exception != null) {
                            Log.e(this, "Error for Query Event= " + msg.what);
                            if (ar.userObj != null) {
                                ImsReasonInfo sipError = clipStatus.getErrorDetails();
                                if (sipError != null) {
                                    mListenerImpl.utConfigurationQueryFailed(
                                            msg.arg1, new ImsReasonInfo(
                                                          ImsReasonInfo.CODE_UT_NETWORK_ERROR,
                                                          sipError.getExtraCode(),
                                                          sipError.getExtraMessage()));
                                } else {
                                    mListenerImpl.utConfigurationQueryFailed(
                                            msg.arg1,
                                            getImsReasonInfoFromResponseError(ar));
                                }
                            }
                        } else if (clipStatus != null) {
                            ImsSsInfo.Builder imsSsInfoBuilder =
                                    new ImsSsInfo.Builder(clipStatus.getStatus());
                            ImsSsInfo ssInfo = imsSsInfoBuilder.build();
                            Bundle clInfo = new Bundle();
                            clInfo.putParcelable(UT_BUNDLE_KEY_SSINFO, ssInfo);
                            Log.d(this, "Success callback on Query event= " + msg.what);
                            mListenerImpl.utConfigurationQueried(
                                    msg.arg1, clInfo);
                        }
                    }
                    break;

                case EVENT_QUERY_COLR:
                    ar = (AsyncResult) msg.obj;
                    SuppService colr = (SuppService) ar.result;
                    if (ar != null) {
                        if (msg.arg1 < 0) {
                            Log.e(this, "Invalid message id received in handleMessage.");
                            return;
                        }
                        if (ar.exception != null) {
                            Log.e(this, "Error for Query Event= " + msg.what);

                            if (ar.userObj != null) {
                                ImsReasonInfo sipError = colr.getErrorDetails();
                                if (sipError != null) {
                                    mListenerImpl.utConfigurationQueryFailed(
                                             msg.arg1, new ImsReasonInfo(
                                                          ImsReasonInfo.CODE_UT_NETWORK_ERROR,
                                                          sipError.getExtraCode(),
                                                          sipError.getExtraMessage()));
                                } else {
                                    mListenerImpl.utConfigurationQueryFailed(
                                            msg.arg1,
                                            getImsReasonInfoFromResponseError(ar));
                                }
                            }
                        } else if (colr != null) {
                            ImsSsInfo.Builder ssInfoBuilder =
                                    new ImsSsInfo.Builder(colr.getStatus());
                            ImsSsInfo ssInfo = ssInfoBuilder.build();
                            Log.i(this, "Service= " + msg.what + " status= "
                                + ssInfo.getStatus());

                            ssInfoBuilder.setProvisionStatus(colr.getProvisionStatus());
                            Log.i(this, "Service= " + msg.what + " Provision Status= "
                                + ssInfo.getProvisionStatus());

                            Bundle clInfo = new Bundle();
                            clInfo.putParcelable(UT_BUNDLE_KEY_SSINFO, ssInfo);
                            Log.d(this, "Success callback on Query event= " + msg.what);
                            mListenerImpl.utConfigurationQueried(
                                    msg.arg1, clInfo);
                        }
                    }
                    break;

                case EVENT_QUERY_COLP:
                    ar = (AsyncResult) msg.obj;
                    if (ar != null) {
                        if (msg.arg1 < 0) {
                            Log.e(this, "Invalid message id received in handleMessage.");
                            return;
                        }
                        if (ar.exception != null) {
                            Log.e(this, "Query COLP error");

                            if (ar.userObj != null) {
                                mListenerImpl.utConfigurationQueryFailed(
                                        msg.arg1,
                                        getImsReasonInfoFromResponseError(ar));
                            }
                        }
                        else if (ar.result != null) {
                            // COLP response has failure cause information.
                            // Check for it to determiqne request's success of failure.
                            SuppSvcResponse response = (SuppSvcResponse) ar.result;
                            final ImsReasonInfo sipError = response.getErrorDetails();
                            if(sipError != null) {
                                Log.e(this, "SuppSvcResponse has failure for COLP query.");
                                mListenerImpl.utConfigurationQueryFailed(msg.arg1,
                                        new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR,
                                        sipError.getExtraCode(), sipError.getExtraMessage()));
                            } else if (response.getFailureCause().length() > 0) {
                                ImsReasonInfo error = new ImsReasonInfo(ImsReasonInfo.
                                        CODE_UT_NETWORK_ERROR, ImsReasonInfo.CODE_UNSPECIFIED,
                                        response.getFailureCause());
                                Log.e(LOG_TAG, "COLP query failed with error = " + error);
                                mListenerImpl.utConfigurationQueryFailed(
                                        msg.arg1,
                                        error);
                            }
                            else {
                                Bundle clInfo = new Bundle();
                                response = (SuppSvcResponse) ar.result;
                                ImsSsInfo.Builder ssInfoBuilder =
                                        new ImsSsInfo.Builder(response.getStatus());
                                ImsSsInfo ssInfo = ssInfoBuilder.build();
                                ssInfoBuilder.setProvisionStatus(response.getProvisionStatus());
                                Log.i(this, "Service= " + msg.what + " provision Status= "
                                        + ssInfo.getProvisionStatus() + " status = "
                                        + ssInfo.getStatus());

                                clInfo.putParcelable(UT_BUNDLE_KEY_SSINFO, ssInfo);
                                Log.i(this, "Success callback called for Query COLP.");
                                mListenerImpl.utConfigurationQueried(
                                        msg.arg1, clInfo);
                            }
                        }
                    }
                    break;

                case EVENT_UPDATE_CLIR:
                case EVENT_UPDATE_CLIP:
                case EVENT_UPDATE_COLR:
                case EVENT_UPDATE_COLP:

                    ar = (AsyncResult) msg.obj;
                    if (ar != null) {
                        if (msg.arg1 < 0) {
                            Log.e(this, "Invalid message id received in handleMessage.");
                            return;
                        }
                        if (ar.exception != null) {
                            if (msg.what == EVENT_UPDATE_CLIR) {
                                Log.e(this, "Update CLIR error");
                            }
                            else if (msg.what == EVENT_UPDATE_CLIP) {
                                Log.e(this, "Update CLIP error");
                            }
                            else if (msg.what == EVENT_UPDATE_COLR) {
                                Log.e(this, "Update COLR error");
                            }
                            else if (msg.what == EVENT_UPDATE_COLP) {
                                Log.e(this, "Update COLP error");
                            }
                            if (ar.result != null) {
                                // CLIP and COLP query responses have failure cause information.
                                // Check for it to determine request's success of failure.
                                SuppSvcResponse response =
                                        (SuppSvcResponse) ar.result;
                                final ImsReasonInfo sipError = response.getErrorDetails();
                                if (sipError != null) {
                                    Log.e(this,
                                            "SuppSvcResponse has failure for CLIP/COLP update");
                                    mListenerImpl.utConfigurationUpdateFailed(
                                            msg.arg1,
                                            new ImsReasonInfo(ImsReasonInfo.CODE_UT_NETWORK_ERROR,
                                            sipError.getExtraCode(), sipError.getExtraMessage()));
                                } else if (!response.getFailureCause().isEmpty()) {
                                    ImsReasonInfo error = new ImsReasonInfo(ImsReasonInfo.
                                            CODE_UT_NETWORK_ERROR, ImsReasonInfo.CODE_UNSPECIFIED,
                                            response.getFailureCause());
                                    Log.e(LOG_TAG, "SuppSvc " + msg.what + " failed, error: "
                                            + error);
                                    mListenerImpl.utConfigurationUpdateFailed(
                                             msg.arg1, error);
                                } else {
                                    Log.e(LOG_TAG, "SuppSvcResponse failure with neither"
                                            + " errordetails nor failurecause");
                                    mListenerImpl.utConfigurationUpdateFailed(
                                            msg.arg1,
                                            getImsReasonInfoFromResponseError(ar));
                                }
                            } else if (ar.userObj != null) {
                                    Log.e(LOG_TAG, "SuppSvcResponse failure with valid userObj");
                                    mListenerImpl.utConfigurationUpdateFailed(
                                            msg.arg1,
                                            getImsReasonInfoFromResponseError(ar));
                            } else {
                                // Nothing to pass to frameworks for this request's response.
                                Log.e(LOG_TAG, "SuppSvcResponse failure with neither ar.result"
                                         + " nor userObj");
                                mListenerImpl.utConfigurationUpdateFailed(
                                        msg.arg1,
                                        getImsReasonInfoFromResponseError(ar));
                            }
                        } else {
                            Log.i(this, "Success callback called for msg.what= "
                                    + msg.what);
                            mListenerImpl.utConfigurationUpdated(msg.arg1);
                        }
                    }
                    break;
                 case EVENT_UNSOL_ON_SS:
                    ar = (AsyncResult) msg.obj;
                    if (ar.exception == null) {
                        ImsSsData ssData = (ImsSsData) ar.result;
                        mListenerImpl.onSupplementaryServiceIndication(ssData);
                    } else {
                        Log.e(this, "exception in handling UNSOL_ON_SS");
                    }
                    break;
            }

        }

    }
}
