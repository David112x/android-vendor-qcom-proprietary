/*
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 *
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

package vendor.qti.iwlan;

import android.content.Context;
import android.hardware.radio.V1_4.SetupDataCallResult;
import android.net.LinkAddress;
import android.net.LinkProperties;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.telephony.CellIdentity;
import android.telephony.CellIdentityCdma;
import android.telephony.CellIdentityGsm;
import android.telephony.CellIdentityLte;
import android.telephony.CellIdentityTdscdma;
import android.telephony.CellIdentityWcdma;
import android.telephony.data.DataCallResponse;
import android.telephony.data.DataProfile;
import android.telephony.data.DataService;
import android.telephony.data.DataServiceCallback;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.util.Log;

import java.net.Inet4Address;
import java.net.InetAddress;
import java.net.UnknownHostException;
import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.concurrent.ConcurrentHashMap;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * This class represents iwlan service which handles wlan data requests and response
 * from the cellular modem.
 */
public class IWlanDataService extends DataService {
    public static final String TAG = IWlanDataService.class.getSimpleName();

    private static final int SETUP_DATA_CALL_COMPLETE               = 1;
    private static final int DEACTIVATE_DATA_CALL_COMPLETE          = 2;
    private static final int REQUEST_DATA_CALL_LIST_COMPLETE        = 3;
    private static final int DATA_CALL_LIST_CHANGED                 = 4;

    IWlanDataServiceProvider provider;

    private class IWlanDataServiceProvider extends DataService.DataServiceProvider {

        private final ConcurrentHashMap<Message, DataServiceCallback> mCallbackMap = new ConcurrentHashMap<>();

        private final Looper mLooper;

        private final Handler mHandler;

        private final IWlanProxy proxy;

        private IWlanDataServiceProvider(int slotIndex) {
            super(slotIndex);

            proxy = IWlanProxy.getInstanceBySlotId(getApplicationContext(), slotIndex);

            HandlerThread thread = new HandlerThread(IWlanDataService.class.getSimpleName());
            thread.start();
            mLooper = thread.getLooper();
            mHandler = new Handler(mLooper) {
                @Override
                public void handleMessage(Message message) {
                    DataServiceCallback callback = mCallbackMap.remove(message);

                    AsyncResult ar = (AsyncResult) message.obj;
                    switch (message.what) {
                        case SETUP_DATA_CALL_COMPLETE:
                            SetupDataCallResult result = (SetupDataCallResult) ar.result;
                            callback.onSetupDataCallComplete(ar.exception != null
                                    ? DataServiceCallback.RESULT_ERROR_ILLEGAL_STATE
                                    : DataServiceCallback.RESULT_SUCCESS,
                                    convertDataCallResult(result));
                            break;
                        case DEACTIVATE_DATA_CALL_COMPLETE:
                            callback.onDeactivateDataCallComplete(ar.exception != null
                                    ? DataServiceCallback.RESULT_ERROR_ILLEGAL_STATE
                                    : DataServiceCallback.RESULT_SUCCESS);
                            break;
                        case REQUEST_DATA_CALL_LIST_COMPLETE:
                            callback.onRequestDataCallListComplete(
                                    ar.exception != null
                                            ? DataServiceCallback.RESULT_ERROR_ILLEGAL_STATE
                                            : DataServiceCallback.RESULT_SUCCESS,
                                    ar.exception != null
                                            ? null
                                            : getDataCallList((List<SetupDataCallResult>) ar.result)
                                    );
                            break;
                        case DATA_CALL_LIST_CHANGED:
                            notifyDataCallListChanged(getDataCallList((List<SetupDataCallResult>) ar.result));
                            break;
                        default:
                            proxy.iwlanLog.e(TAG, "Unexpected event: " + message.what);
                            return;
                    }
                }
            };

            proxy.iwlanLog.d(TAG, "Register for data call list changed.");
            proxy.registerForDataCallListChanged(mHandler, DATA_CALL_LIST_CHANGED);
        }

        private List<DataCallResponse> getDataCallList(List<SetupDataCallResult> dcList) {
            List<DataCallResponse> dcResponseList = new ArrayList<>();
            for (SetupDataCallResult dcResult : dcList) {
                dcResponseList.add(convertDataCallResult(dcResult));
            }
            return dcResponseList;
        }

        @Override
        public void setupDataCall(int accessNetwork, DataProfile dataProfile, boolean isRoaming,
                                  boolean allowRoaming, int reason, LinkProperties linkProperties,
                                  DataServiceCallback callback) {
            proxy.iwlanLog.d(TAG, "setupDataCall " + getSlotIndex());

            Message message = null;
            // Only obtain the message when the caller wants a callback. If the caller doesn't care
            // the request completed or results, then no need to pass the message down.
            if (callback != null) {
                message = Message.obtain(mHandler, SETUP_DATA_CALL_COMPLETE);
                mCallbackMap.put(message, callback);
            }

            proxy.setupDataCall(dataProfile, isRoaming, allowRoaming, reason, linkProperties, message);
        }

        @Override
        public void deactivateDataCall(int cid, int reason, DataServiceCallback callback) {
            proxy.iwlanLog.d(TAG, "deactivateDataCall " + getSlotIndex());

            Message message = null;
            // Only obtain the message when the caller wants a callback. If the caller doesn't care
            // the request completed or results, then no need to pass the message down.
            if (callback != null) {
                message = Message.obtain(mHandler, DEACTIVATE_DATA_CALL_COMPLETE);
                mCallbackMap.put(message, callback);
            }

            proxy.deactivateDataCall(cid, reason, message);
        }

        @Override
        public void requestDataCallList(DataServiceCallback callback) {
            proxy.iwlanLog.d(TAG, "requestDataCallList " + getSlotIndex());

            Message message = null;
            // Only obtain the message when the caller wants a callback. If the caller doesn't care
            // the request completed or results, then no need to pass the message down.
            if (callback != null) {
                message = Message.obtain(mHandler, REQUEST_DATA_CALL_LIST_COMPLETE);
                mCallbackMap.put(message, callback);
            }
            proxy.requestDataCallList(message);
        }

        @Override
        public void close() {
            Log.d(TAG, "IWlanDataService Closing");
            proxy.unregisterForDataCallListChanged(mHandler);
            IWlanProxy.disableIWlan(proxy);
        }

        public void dump(PrintWriter pw) {
            pw.println("IWlanDataServiceProvider:");
            proxy.iwlanLog.dataLogBuffer.dump(pw);
        }
    }

    @Override
    public DataServiceProvider onCreateDataServiceProvider(int slotId) {
        Log.d(TAG, "IWlan data service created for slot " + slotId);
        TelephonyManager tm = (TelephonyManager) getApplicationContext().getSystemService(Context.TELEPHONY_SERVICE);
        if (slotId < 0 || slotId > tm.getPhoneCount()) {
            Log.e(TAG, "Tried to create iwlan data service with invalid slotId " + slotId);
            return null;
        }
        provider = new IWlanDataServiceProvider(slotId);
        return provider;
    }

    @Override
    protected void dump(FileDescriptor fd, PrintWriter writer, String[] args){
        writer.println("IWlanDataService:");
        if(provider != null)
            provider.dump(writer);
    }

    /**
     * Convert SetupDataCallResult defined in types.hal into DataCallResponse
     * @param dcResult setup data call result
     * @return converted DataCallResponse object
     */
    public DataCallResponse convertDataCallResult(SetupDataCallResult dcResult) {
        if (dcResult == null) return null;

        // Process address
        String[] addresses = dcResult.addresses.stream().toArray(String[]::new);
        List<LinkAddress> laList = new ArrayList<>();
        if (addresses != null) {
            for (String address : addresses) {
                address = address.trim();
                if (address.isEmpty()) continue;

                try {
                    LinkAddress la = new LinkAddress(address);

                    laList.add(la);
                } catch (IllegalArgumentException e) {
                    Log.e(TAG, "Unknown address: " + address + ", exception = " + e);
                }
            }
        }

        // Process dns
        String[] dnses = dcResult.dnses.stream().toArray(String[]::new);
        List<InetAddress> dnsList = new ArrayList<>();
        if (dnses != null) {
            for (String dns : dnses) {
                dns = dns.trim();
                InetAddress ia;
                try {
                    ia = InetAddress.getByName(dns);
                    dnsList.add(ia);
                } catch (UnknownHostException e) {
                    Log.e(TAG, "Unknown dns: " + dns + ", exception = " + e);
                }
            }
        }

        // Process gateway
        String[] gateways = dcResult.gateways.stream().toArray(String[]::new);
        List<InetAddress> gatewayList = new ArrayList<>();
        if (gateways != null) {
            for (String gateway : gateways) {
                gateway = gateway.trim();
                InetAddress ia;
                try {
                    ia = InetAddress.getByName(gateway);
                    gatewayList.add(ia);
                } catch (UnknownHostException e) {
                    Log.e(TAG, "Unknown gateway: " + gateway + ", exception = " + e);
                }
            }
        }

        // Process pcscf
        String[] pcscfs = dcResult.pcscf.stream().toArray(String[]::new);
        List<InetAddress> pcscfList = new ArrayList<>();
        if (pcscfs != null) {
            for (String pcscf : pcscfs) {
                pcscf = pcscf.trim();
                InetAddress ia;
                try {
                    ia = InetAddress.getByName(pcscf);
                    pcscfList.add(ia);
                } catch (UnknownHostException e) {
                    Log.e(TAG, "Unknown pcscf: " + pcscf + ", exception = " + e);
                }
            }
        }


        return new DataCallResponse.Builder()
                .setCause(dcResult.cause)
                .setSuggestedRetryTime(dcResult.suggestedRetryTime)
                .setId(dcResult.cid)
                .setLinkStatus(dcResult.active)
                .setProtocolType(dcResult.type)
                .setInterfaceName(dcResult.ifname)
                .setAddresses(laList)
                .setDnsAddresses(dnsList)
                .setGatewayAddresses(gatewayList)
                .setPcscfAddresses(pcscfList)
                .setMtu(dcResult.mtu)
                .build();
    }
}
