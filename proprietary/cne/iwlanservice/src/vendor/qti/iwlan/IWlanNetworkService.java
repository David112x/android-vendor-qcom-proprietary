/*
 * Copyright (c) 2018-2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 * Not a Contribution.
 *
 * Copyright (C) 2017 The Android Open Source Project
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
import android.hardware.radio.V1_0.RegState;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.telephony.AccessNetworkConstants;
import android.telephony.NetworkRegistrationInfo;
import android.telephony.NetworkService;
import android.telephony.NetworkServiceCallback;
import android.telephony.TelephonyManager;
import android.util.Log;

import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.util.concurrent.ConcurrentHashMap;
import java.util.ArrayList;
import java.util.List;

import vendor.qti.hardware.data.iwlan.V1_0.IWlanDataRegStateResult;

/**
 * Implementation of network services for iWlan. It's a service that handles network requests
 * for iWlan. It passes the requests to inner IWlanNetworkServiceProvider which has a
 * handler thread for each slot.
 */
public class IWlanNetworkService extends NetworkService {
    public static final String TAG = IWlanNetworkService.class.getSimpleName();

    private static final int GET_REGISTRATION_STATE_COMPLETE = 1;
    private static final int NETWORK_REGISTRATION_STATE_CHANGED = 3;

    IWlanNetworkServiceProvider provider;

    private class IWlanNetworkServiceProvider extends NetworkServiceProvider {

        private final ConcurrentHashMap<Message, NetworkServiceCallback> mCallbackMap = new ConcurrentHashMap<>();

        private final Looper mLooper;

        private final Handler mHandler;

        private final IWlanProxy proxy;

        IWlanNetworkServiceProvider(int slotIndex) {
            super(slotIndex);

            proxy = IWlanProxy.getInstanceBySlotId(getApplicationContext(), getSlotIndex());

            HandlerThread thread = new HandlerThread(IWlanNetworkService.class.getSimpleName());
            thread.start();
            mLooper = thread.getLooper();
            mHandler = new Handler(mLooper) {
                @Override
                public void handleMessage(Message message) {
                    NetworkServiceCallback callback = mCallbackMap.remove(message);

                    AsyncResult ar;
                    switch (message.what) {
                        case GET_REGISTRATION_STATE_COMPLETE:
                            if (callback == null) return;
                            ar = (AsyncResult) message.obj;
                            NetworkRegistrationInfo netState = getRegistrationStateFromResult((IWlanDataRegStateResult) ar.result);

                            int resultCode;
                            if (ar.exception != null || netState == null) {
                                resultCode = NetworkServiceCallback.RESULT_ERROR_FAILED;
                            } else {
                                resultCode = NetworkServiceCallback.RESULT_SUCCESS;
                            }

                            try {
                                proxy.iwlanLog.d(TAG, "Calling callback.onGetNetworkRegistrationInfoComplete."
                                        + "resultCode = " + resultCode + ", netState = " + netState);
                                callback.onRequestNetworkRegistrationInfoComplete(resultCode, netState);
                            } catch (Exception e) {
                                proxy.iwlanLog.d(TAG, "Exception: " + e);
                            }
                            break;
                        case NETWORK_REGISTRATION_STATE_CHANGED:
                            notifyNetworkRegistrationInfoChanged();
                            break;
                        default:
                            return;
                    }
                }
            };

            proxy.registerForDataRegistrationStateChanged(mHandler, NETWORK_REGISTRATION_STATE_CHANGED);
        }

        private int getRegStateFromHalRegState(int halRegState) {
            switch (halRegState) {
                case RegState.NOT_REG_MT_NOT_SEARCHING_OP:
                case RegState.NOT_REG_MT_NOT_SEARCHING_OP_EM:
                    return NetworkRegistrationInfo.REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING;
                case RegState.REG_HOME:
                    return NetworkRegistrationInfo.REGISTRATION_STATE_HOME;
                case RegState.NOT_REG_MT_SEARCHING_OP:
                case RegState.NOT_REG_MT_SEARCHING_OP_EM:
                    return NetworkRegistrationInfo.REGISTRATION_STATE_NOT_REGISTERED_SEARCHING;
                case RegState.REG_DENIED:
                case RegState.REG_DENIED_EM:
                    return NetworkRegistrationInfo.REGISTRATION_STATE_DENIED;
                case RegState.UNKNOWN:
                case RegState.UNKNOWN_EM:
                    return NetworkRegistrationInfo.REGISTRATION_STATE_UNKNOWN;
                case RegState.REG_ROAMING:
                    return NetworkRegistrationInfo.REGISTRATION_STATE_ROAMING;
                default:
                    return NetworkRegistrationInfo.REGISTRATION_STATE_NOT_REGISTERED_OR_SEARCHING;
            }
        }

        private boolean isEmergencyOnly(int halRegState) {
            switch (halRegState) {
                case RegState.NOT_REG_MT_NOT_SEARCHING_OP_EM:
                case RegState.NOT_REG_MT_SEARCHING_OP_EM:
                case RegState.REG_DENIED_EM:
                case RegState.UNKNOWN_EM:
                    return true;
                case RegState.NOT_REG_MT_NOT_SEARCHING_OP:
                case RegState.REG_HOME:
                case RegState.NOT_REG_MT_SEARCHING_OP:
                case RegState.REG_DENIED:
                case RegState.UNKNOWN:
                case RegState.REG_ROAMING:
                default:
                    return false;
            }
        }

        private List<Integer> getAvailableServices(int regState, boolean emergencyOnly) {
            List<Integer> availableServices = new ArrayList<>();

            // In emergency only states, only SERVICE_TYPE_EMERGENCY is available.
            // Otherwise, certain services are available only if it's registered on home or roaming
            // network.
            if (emergencyOnly) {
                availableServices.add(NetworkRegistrationInfo.SERVICE_TYPE_EMERGENCY);
            } else if (regState == NetworkRegistrationInfo.REGISTRATION_STATE_ROAMING
                    || regState == NetworkRegistrationInfo.REGISTRATION_STATE_HOME) {
                availableServices.add(NetworkRegistrationInfo.SERVICE_TYPE_DATA);
            }

            return availableServices;
        }

        private NetworkRegistrationInfo getRegistrationStateFromResult(IWlanDataRegStateResult result) {
            if (result == null) {
                return null;
            }

            boolean emergencyOnly = isEmergencyOnly(result.regState);
            List<Integer> availableServices = getAvailableServices(result.regState, emergencyOnly);
            int regState = getRegStateFromHalRegState(result.regState);
            int accessNetworkTechnology = TelephonyManager.NETWORK_TYPE_IWLAN;

            NetworkRegistrationInfo netRegState = new NetworkRegistrationInfo.Builder()
                                .setDomain(NetworkRegistrationInfo.DOMAIN_PS)
                                .setTransportType(AccessNetworkConstants.TRANSPORT_TYPE_WLAN)
                                .setRegistrationState(regState)
                                .setAccessNetworkTechnology(accessNetworkTechnology)
                                .setRejectCause(result.reasonForDenial)
                                .setEmergencyOnly(emergencyOnly)
                                .setAvailableServices(availableServices)
                                .build();

            return netRegState;
        }

        @Override
        //Domain is not needed, will always using ps?
        public void requestNetworkRegistrationInfo(int domain, NetworkServiceCallback callback) {
            proxy.iwlanLog.d(TAG, "getNetworkRegistrationInfo for domain " + domain);
            Message message = null;

            if(domain == NetworkRegistrationInfo.DOMAIN_CS) {
                callback.onRequestNetworkRegistrationInfoComplete(NetworkServiceCallback.RESULT_ERROR_UNSUPPORTED, null);
            }
            else{
                // Only obtain the message when the caller wants a callback. If the caller doesn't care
                // the request completed or results, then no need to pass the message down.
                if (callback != null) {
                    message = Message.obtain(mHandler, GET_REGISTRATION_STATE_COMPLETE);
                    mCallbackMap.put(message, callback);
                }

                proxy.getDataRegistrationState(message);
            }
        }

        @Override
        public void close() {
            Log.d(TAG, "IWlanNetworkService Closing");
            proxy.unregisterForDataRegistrationStateChanged(mHandler);
            IWlanProxy.disableIWlan(proxy);
        }

        public void dump(PrintWriter pw) {
            pw.println("IWlanNetworkServiceProvider:");
            proxy.iwlanLog.networkLogBuffer.dump(pw);
        }
    }

    @Override
    public NetworkServiceProvider onCreateNetworkServiceProvider(int slotId) {
        Log.d(TAG, "IWlan network service created for slot " + slotId);
        TelephonyManager tm = (TelephonyManager) getApplicationContext().getSystemService(Context.TELEPHONY_SERVICE);
        if (slotId < 0 || slotId > tm.getPhoneCount()) {
            Log.e(TAG, "Tried to IWlan network service with invalid slotId " + slotId);
            return null;
        }
        provider = new IWlanNetworkServiceProvider(slotId);
        return provider;
    }

    @Override
    protected void dump(FileDescriptor fd, PrintWriter writer, String[] args){
        writer.println("IWlanNetworkService:");
        provider.dump(writer);
    }
}
