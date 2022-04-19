/*
 *   Copyright (c) 2018 Qualcomm Technologies, Inc.
 *   All Rights Reserved.
 *   Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package vendor.qti.iwlan;

import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.telephony.data.QualifiedNetworksService;
import android.telephony.TelephonyManager;
import android.util.Log;
import android.util.SparseArray;
import android.content.Context;

import java.io.FileDescriptor;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.List;

import android.hardware.radio.V1_0.ApnTypes;

/**
 * Implementation of qualified networks service. Its a service
 * that provides qualified networks in order of preference. It
 * passes the requests to inner NetworkAvailabilityProviderImpl
 * which has a handler thread for each slot.
 */
public class QualifiedNetworksServiceImpl extends QualifiedNetworksService {
    public static final String TAG = QualifiedNetworksServiceImpl.class.getSimpleName();

    private static final int GET_QUALIFIED_NETWORKS_COMPLETE = 1;
    private static final int QUALIFIED_NETWORKS_CHANGED = 3;

    NetworkAvailabilityProviderImpl provider;

    private class NetworkAvailabilityProviderImpl extends NetworkAvailabilityProvider {

        private final Looper mLooper;

        private final Handler mHandler;

        private final IWlanProxy proxy;

        NetworkAvailabilityProviderImpl(int slotIndex) {
            super(slotIndex);

            proxy = IWlanProxy.getInstanceBySlotId(getApplicationContext(), getSlotIndex());

            HandlerThread thread = new HandlerThread(QualifiedNetworksServiceImpl.class.getSimpleName());
            thread.start();
            mLooper = thread.getLooper();
            mHandler = new Handler(mLooper) {
                @Override
                public void handleMessage(Message message) {

                    AsyncResult ar = (AsyncResult) message.obj;
                    switch (message.what) {
                        case GET_QUALIFIED_NETWORKS_COMPLETE:
                            proxy.iwlanLog.d(TAG, "get complete, Calling updateQualifiedNetworks");
                            updateQualifiedNetworks((List<vendor.qti.hardware.data.iwlan.V1_0.QualifiedNetworks>) ar.result);

                            break;
                        case QUALIFIED_NETWORKS_CHANGED:
                            proxy.iwlanLog.d(TAG, "info changed, Calling updateQualifiedNetworks");
                            updateQualifiedNetworks((List<vendor.qti.hardware.data.iwlan.V1_0.QualifiedNetworks>) ar.result);
                            break;
                        default:
                            return;
                    }
                }
            };

            //Query for initial networks once in constructor
            Message message = Message.obtain(mHandler, GET_QUALIFIED_NETWORKS_COMPLETE);
            proxy.getAllQualifiedNetworks(message);
            proxy.registerForQualifiedNetworksChanged(mHandler, QUALIFIED_NETWORKS_CHANGED);
        }

        private void updateQualifiedNetworks(List<vendor.qti.hardware.data.iwlan.V1_0.QualifiedNetworks> qnpList) {
            //We need to form bitmask of APN types that have the same preferred network
            //Hence create an intermediate result array for which key is preferred
            //network and value is mask of APN types
            //Framework expects an array of preferred networks but we only send the
            //most preferred one
            SparseArray<Integer> result = new SparseArray<>();
            int apnTypeMaskNullNw = 0;
            for (vendor.qti.hardware.data.iwlan.V1_0.QualifiedNetworks qNetwork : qnpList) {
                //Handle case when there are no available networks for an apn type
                if ((qNetwork.networks == null) || (qNetwork.networks.size() == 0)) {
                    proxy.iwlanLog.i(TAG, "No available networks for apnType  " + qNetwork.apnType);
                    apnTypeMaskNullNw = apnTypeMaskNullNw | qNetwork.apnType;
                }
                else {
                    int prefNw = qNetwork.networks.get(0);
                    proxy.iwlanLog.i(TAG, "Pref network for apnType  " + qNetwork.apnType
                          + " is " + prefNw);
                    int apnTypeMask = result.get(prefNw, qNetwork.apnType);
                    result.put(prefNw, apnTypeMask | qNetwork.apnType);
                }
            }

            //Notify changes to framework
            for (int i = 0; i < result.size(); i++) {
                List<Integer> resultNetwork = new ArrayList<>();
                resultNetwork.add(result.keyAt(i));
                int resultApnMask = result.valueAt(i);
                proxy.iwlanLog.i(TAG, "Calling updateQualifiedNetworkTypes for apnMask " + resultApnMask
                          + " pref network " + resultNetwork.get(0));
                updateQualifiedNetworkTypes(resultApnMask, resultNetwork);
            }
            //Notify if any apn types have no available network
            if (apnTypeMaskNullNw != 0) {
                proxy.iwlanLog.i(TAG, "Calling updateQualifiedNetworkTypes for apnMask " + apnTypeMaskNullNw
                          + " with null network " );
                updateQualifiedNetworkTypes(apnTypeMaskNullNw, null);
            }

        }

        @Override
        public void close() {
            Log.d(TAG, "QNP Service Closing");
            proxy.unregisterForQualifiedNetworksChanged(mHandler);
            IWlanProxy.disableIWlan(proxy);
        }

        public void dump(PrintWriter pw) {
            pw.println("NetworkAvailabilityProviderImpl:");
            proxy.iwlanLog.qnpLogBuffer.dump(pw);
        }
    }

    @Override
    public NetworkAvailabilityProvider onCreateNetworkAvailabilityProvider(int slotIndex) {
        Log.d(TAG, "Qualified Networks service created for slot " + slotIndex);
        TelephonyManager tm = (TelephonyManager) getApplicationContext().getSystemService(Context.TELEPHONY_SERVICE);
        if (slotIndex < 0 || slotIndex > tm.getPhoneCount()) {
            Log.e(TAG, "Tried to create QNP service with invalid slotIndex " + slotIndex);
            return null;
        }
        provider = new NetworkAvailabilityProviderImpl(slotIndex);
        return provider;
    }

    @Override
    protected void dump(FileDescriptor fd, PrintWriter writer, String[] args){
        writer.println("QualifiedNetworksServiceImpl:");
        provider.dump(writer);
    }
}
