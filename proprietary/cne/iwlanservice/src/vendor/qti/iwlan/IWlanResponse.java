/*
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 *
 * Copyright (C) 2016 The Android Open Source Project
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

package vendor.qti.iwlan;

import android.hardware.radio.V1_0.RadioError;
import android.hardware.radio.V1_4.SetupDataCallResult;
import android.os.Message;
import android.text.TextUtils;
import android.util.Log;

import java.util.ArrayList;

import vendor.qti.hardware.data.iwlan.V1_0.IWlanResponseInfo;
import vendor.qti.hardware.data.iwlan.V1_0.IIWlanResponse;
import vendor.qti.hardware.data.iwlan.V1_0.IWlanDataRegStateResult;
import vendor.qti.hardware.data.iwlan.V1_0.QualifiedNetworks;

public class IWlanResponse extends IIWlanResponse.Stub {
    private static final String TAG = IWlanIndication.class.getSimpleName();

    IWlanProxy mProxy;

    IWlanResponse(IWlanProxy proxy) {
        mProxy = proxy;
    }

    /**
     * Helper function to send response msg
     * @param msg Response message to be sent
     * @param ret Return object to be included in the response message
     */
    static void sendMessageResponse(Message msg, Object ret) {
        if (msg != null) {
            AsyncResult.forMessage(msg, ret, null);
            msg.sendToTarget();
        }
    }

    static String responseInfoToString(IWlanResponseInfo responseInfo) {
        StringBuilder b = new StringBuilder();
        b.append("Serial = " + responseInfo.serial);
        b.append(", RadioError = " + responseInfo.error);
        return b.toString();
    }

    static String setDataCallResponseToString(SetupDataCallResult setupDataCallResult) {
        StringBuilder b = new StringBuilder();
        b.append("Fail Cause = " + setupDataCallResult.cause);
        b.append(", Suggested Retry Time = " + setupDataCallResult.suggestedRetryTime);
        b.append(", Active  = " + setupDataCallResult.active);
        b.append(", Type  = " + setupDataCallResult.type);
        b.append(", Addresses  = [" + TextUtils.join(",", setupDataCallResult.addresses) + "]");
        b.append(", DNSes  = [" + TextUtils.join(",", setupDataCallResult.dnses) + "]");
        b.append(", Gateways  = [" + TextUtils.join(",", setupDataCallResult.gateways) + "]");
        b.append(", P-CSCFs  = [" + TextUtils.join(",", setupDataCallResult.pcscf) + "]");
        b.append(", MTU  = " + setupDataCallResult.mtu);
        return b.toString();
    }

    static String dataCallResultListToString(ArrayList<SetupDataCallResult> dataCallResultList) {
        StringBuilder b = new StringBuilder();
        for(SetupDataCallResult dataCall :  dataCallResultList) {
            b.append("--- Data Call ---");
            b.append(setDataCallResponseToString(dataCall));
        }
        return b.toString();
    }

    static String iwlanDataRegStateResultToString(IWlanDataRegStateResult state) {
        StringBuilder b = new StringBuilder();
        b.append("RegState = " + state.regState);
        b.append(", Reason for denial = " + state.reasonForDenial);
        return b.toString();
    }

    static String qualifiedNetworksListToString(ArrayList<QualifiedNetworks> qualifiedNetworksList) {
        StringBuilder b = new StringBuilder();
        for (QualifiedNetworks qNetwork : qualifiedNetworksList) {
            //Handle case when there are no available networks for an apn type
            if ((qNetwork.networks == null) || (qNetwork.networks.size() == 0)) {
                b.append("No available networks for apnType  " + qNetwork.apnType);
            }
            else {
                int prefNw = qNetwork.networks.get(0);
                b.append("Pref network for apnType  " + qNetwork.apnType + " is " + prefNw);
            }
        }
        return b.toString();
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param setupDataCallResult Response to data call setup as defined by setupDataCallResult in
     *                            types.hal
     */
    public void setupDataCallResponse(IWlanResponseInfo responseInfo,
                                      SetupDataCallResult setupDataCallResult) {
        mProxy.iwlanLog.d(IWlanDataService.TAG, "setupDataCallResponse: " + responseInfoToString(responseInfo));
        mProxy.iwlanLog.d(IWlanDataService.TAG, "setupDataCallResponse: " + setDataCallResponseToString(setupDataCallResult));
        responseSetupDataCall(responseInfo, setupDataCallResult);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     */
    public void deactivateDataCallResponse(IWlanResponseInfo responseInfo) {
        mProxy.iwlanLog.d(IWlanDataService.TAG, "deactivateDataCallResponse");
        responseVoid(responseInfo, IWlanDataService.TAG);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param dataCallResultList Response to get data call list as defined by setupDataCallResult in
     *                           types.hal
     */
    public void getDataCallListResponse(IWlanResponseInfo responseInfo,
                                        ArrayList<SetupDataCallResult> dataCallResultList) {
        mProxy.iwlanLog.d(IWlanDataService.TAG, "getDataCallListResponse: " + dataCallResultListToString(dataCallResultList));
        responseDataCallList(responseInfo, dataCallResultList);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param state network registration state parameter
     */
    public void getDataRegistrationStateResponse(IWlanResponseInfo responseInfo, IWlanDataRegStateResult state) {
        mProxy.iwlanLog.d(IWlanNetworkService.TAG, "getDataRegistrationStateResponse: " + iwlanDataRegStateResultToString(state));
        responseDataRegistrationState(responseInfo, state);
    }

    /**
     * @param responseInfo Response info struct containing response type, serial no. and error
     * @param qualifiedNetworksList networks list
     */
    public void getAllQualifiedNetworksResponse(IWlanResponseInfo responseInfo, ArrayList<QualifiedNetworks> qualifiedNetworksList) {
        mProxy.iwlanLog.d(QualifiedNetworksServiceImpl.TAG, "getAllQualifiedNetworksResponse: " + qualifiedNetworksListToString(qualifiedNetworksList));
        responseQualifiedNetworksChanged(responseInfo, qualifiedNetworksList);
    }

    /**
     * Acknowledge the receipt of radio request sent to the vendor. This must be sent only for
     * radio request which take long time to respond.
     * For more details, refer https://source.android.com/devices/tech/connect/ril.html
     *
     * @param serial Serial no. of the request whose acknowledgement is sent.
     */
    public void acknowledgeRequest(int serial) {
        mProxy.processRequestAck(serial);
    }

    private void responseSetupDataCall(IWlanResponseInfo responseInfo,
                                       SetupDataCallResult setupDataCallResult) {
        IWlanRequest rr = mProxy.processResponse(responseInfo, true, IWlanDataService.TAG);

        if (rr != null) {
            sendMessageResponse(rr.mResult, setupDataCallResult);
        }
    }

    private void responseVoid(IWlanResponseInfo responseInfo, String tag) {
        IWlanRequest rr = mProxy.processResponse(responseInfo, true, tag);

        if (rr != null) {
            Object ret = null;
            sendMessageResponse(rr.mResult, ret);
        }
    }

    private void responseDataCallList(IWlanResponseInfo responseInfo,
                                      ArrayList<SetupDataCallResult> dataCallResultList) {
        IWlanRequest rr = mProxy.processResponse(responseInfo, false, IWlanDataService.TAG);

        if (rr != null) {
            sendMessageResponse(rr.mResult, dataCallResultList);
        }
        //need to decrement the wakelock on sync requests after sending response
        mProxy.decrementWakeLock(rr);
    }

    private void responseDataRegistrationState(IWlanResponseInfo responseInfo, IWlanDataRegStateResult state) {
        IWlanRequest rr = mProxy.processResponse(responseInfo, false, IWlanNetworkService.TAG);

        if (rr != null) {
            sendMessageResponse(rr.mResult, state);
        }
        //need to decrement the wakelock on sync requests after sending response
        mProxy.decrementWakeLock(rr);
    }

    private void responseQualifiedNetworksChanged(IWlanResponseInfo responseInfo,
                                                  ArrayList<QualifiedNetworks> qualifiedNetworksList) {
        IWlanRequest rr = mProxy.processResponse(responseInfo, false, QualifiedNetworksServiceImpl.TAG);

        if (rr != null) {
            sendMessageResponse(rr.mResult, qualifiedNetworksList);
        }
        //need to decrement the wakelock on sync requests after sending response
        mProxy.decrementWakeLock(rr);
    }
}
