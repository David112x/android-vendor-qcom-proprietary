/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qualcomm.location.izat.wifidbprovider;

import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.util.Log;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.HashMap;

import com.qti.wifidbprovider.*;
import com.qualcomm.location.izat.GTPClientHelper;

public class WiFiDBProvider {
    private static final String TAG = "WiFiDBProvider";
    private static final boolean VERBOSE_DBG = Log.isLoggable(TAG, Log.VERBOSE);

    private static final Object sCallBacksLock = new Object();
    private final Context mContext;

    private IWiFiDBProviderResponseListener mWiFiDBProviderResponseListener = null;

    private PendingIntent mListenerIntent = null;

    private static WiFiDBProvider sInstance = null;
    public static WiFiDBProvider getInstance(Context ctx) {
        if (sInstance == null) {
            sInstance = new WiFiDBProvider(ctx);
        }
        return sInstance;
    }

    private WiFiDBProvider(Context ctx) {
        if (VERBOSE_DBG) {
            Log.d(TAG, "WiFiDBProvider construction");
        }
        mContext = ctx;
        native_wifi_db_provider_init();
    }

    /* Remote binder */
    private final IWiFiDBProvider.Stub mBinder = new IWiFiDBProvider.Stub() {

        public boolean registerResponseListener(final IWiFiDBProviderResponseListener callback,
                                                PendingIntent notifyIntent) {
            if (VERBOSE_DBG) {
                Log.d(TAG, "WiFiDBProvider registerResponseListener");
            }

            if (callback == null) {
                Log.e(TAG, "callback is null");
                return false;
            }

            if (notifyIntent == null) {
                Log.w(TAG, "notifyIntent is null");
            }

            if (null != mWiFiDBProviderResponseListener) {
                Log.e(TAG, "Response listener already provided.");
                return false;
            }

            synchronized (sCallBacksLock) {
                mWiFiDBProviderResponseListener = callback;
                mListenerIntent = notifyIntent;
            }


            try {
                callback.asBinder().linkToDeath(new IBinder.DeathRecipient() {
                    @Override
                    public void binderDied() {
                        synchronized (sCallBacksLock) {
                            mWiFiDBProviderResponseListener = null;
                            mListenerIntent = null;
                        }
                    }
                }, 0);
            } catch (RemoteException e) {
                throw new RuntimeException("Failed unregister WiFi DB provider cb", e);
            }

            if (VERBOSE_DBG) {
                Log.d(TAG, "WiFiDBProvider GTPClientHelper.SetClientRegistrationStatus IN");
            }
            GTPClientHelper.SetClientRegistrationStatus(mContext,
                    GTPClientHelper.GTP_CLIENT_WIFI_PROVIDER, true);

            return true;
        }

        public void removeResponseListener(final IWiFiDBProviderResponseListener callback) {
            if (callback == null) {
                Log.e(TAG, "callback is null");
                return;
            }
            synchronized (sCallBacksLock) {
                mWiFiDBProviderResponseListener = null;
                mListenerIntent = null;
            }
            if (VERBOSE_DBG) {
                Log.d(TAG, "WiFiDBProvider GTPClientHelper.SetClientRegistrationStatus OUT");
            }
            GTPClientHelper.SetClientRegistrationStatus(mContext,
                    GTPClientHelper.GTP_CLIENT_WIFI_PROVIDER, false);
        }


        public int requestAPObsLocData() {
            if (VERBOSE_DBG) {
                Log.d(TAG, "in IWiFiDBProvider.Stub(): requestAPObsLocData()");
            }

            return native_wifi_db_provider_request_ap_obs_data();
        }

    };

    private void onApObsLocDataAvailable(APObsLocData[] obsDataArray, int status) {
        if (VERBOSE_DBG) {
            Log.d(TAG, "onApObsLocDataAvailable status: " + status);
        }

        List<APObsLocData> obsDataList = new ArrayList<APObsLocData>();
        try {
            obsDataList.addAll(Arrays.asList(obsDataArray));
        } catch (NullPointerException e) {
            Log.d(TAG, "onAPListAvailable: AP List is NULL");
        }
        synchronized (sCallBacksLock) {
            if (null != mWiFiDBProviderResponseListener) {
                try {
                    mWiFiDBProviderResponseListener.onApObsLocDataAvailable(
                            obsDataList, status);
                } catch (RemoteException e) {
                    Log.w(TAG, "onApObsLocDataAvailable remote exception, sending intent");
                    GTPClientHelper.SendPendingIntent(mContext, mListenerIntent, "WiFiDBProvider");
                }
            }
        }
    }

    private void onServiceRequest() {
        if (VERBOSE_DBG) {
            Log.d(TAG, "onServiceRequest");
        }
        synchronized (sCallBacksLock) {
            if (null != mWiFiDBProviderResponseListener) {
                try {
                    mWiFiDBProviderResponseListener.onServiceRequest();
                } catch (RemoteException e) {
                    Log.w(TAG, "onServiceRequest remote exception, sending intent");
                    GTPClientHelper.SendPendingIntent(mContext, mListenerIntent, "WiFiDBProvider");
                }
            }
        }
    }

    public IWiFiDBProvider getWiFiDBProviderBinder() {
        return mBinder;
    }

    protected void finalize() throws Throwable {
        native_wifi_db_provider_deinit();
    }

    static {
        native_wifi_db_provider_class_init();
    }

    private static native void native_wifi_db_provider_class_init();
    private native void native_wifi_db_provider_init();
    private native void native_wifi_db_provider_deinit();
    private native int native_wifi_db_provider_request_ap_obs_data();
}
