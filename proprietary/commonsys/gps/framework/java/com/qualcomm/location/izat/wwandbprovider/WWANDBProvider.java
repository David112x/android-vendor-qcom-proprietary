/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qualcomm.location.izat.wwandbprovider;

import android.app.PendingIntent;
import android.content.Context;
import android.location.Location;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.util.Log;

import com.qti.wwandbreceiver.BSInfo;
import com.qualcomm.location.izat.GTPClientHelper;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;
import com.qti.wwandbprovider.*;

public class WWANDBProvider {
    private static final String TAG = "WWANDBProvider";
    private static final boolean VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);

    private static final Object sCallBacksLock = new Object();
    private final Context mContext;

    private IWWANDBProviderResponseListener mWWANDBProviderResponseListener = null;

    private PendingIntent mListenerIntent = null;

    private static WWANDBProvider sInstance = null;
    public static WWANDBProvider getInstance(Context ctx) {
        if (sInstance == null) {
            sInstance = new WWANDBProvider(ctx);
        }
        return sInstance;
    }

    private WWANDBProvider(Context ctx) {
        if (VERBOSE) {
            Log.d(TAG, "WWANDBProvider construction");
        }
        mContext = ctx;
        native_wwan_db_provider_init();
    }

    /* Remote binder */
    private final IWWANDBProvider.Stub mBinder = new IWWANDBProvider.Stub() {

        public boolean registerResponseListener(final IWWANDBProviderResponseListener callback,
                                                PendingIntent notifyIntent) {
            if (VERBOSE) {
                Log.d(TAG, "WWANDBProvider registerResponseListener");
            }

            if (callback == null) {
                Log.e(TAG, "callback is null");
                return false;
            }

            if (notifyIntent == null) {
                Log.w(TAG, "notifyIntent is null");
            }

            if (null != mWWANDBProviderResponseListener) {
                Log.e(TAG, "Response listener already provided.");
                return false;
            }

            synchronized (sCallBacksLock) {
                mWWANDBProviderResponseListener = callback;
                mListenerIntent = notifyIntent;
            }

            try {
                callback.asBinder().linkToDeath(new DeathRecipient() {
                    @Override
                    public void binderDied() {
                        synchronized (sCallBacksLock) {
                            mWWANDBProviderResponseListener = null;
                            mListenerIntent = null;
                        }
                    }
                }, 0);
            } catch (RemoteException e) {
                throw new RuntimeException("Failed to unregister WWWAN DB provider cb", e);
            }

            if (VERBOSE) {
                Log.d(TAG, "WWANDBProvider GTPClientHelper.SetClientRegistrationStatus IN");
            }
            GTPClientHelper.SetClientRegistrationStatus(mContext,
                    GTPClientHelper.GTP_CLIENT_WWAN_PROVIDER, true);

            return true;
        }

        public void removeResponseListener(final IWWANDBProviderResponseListener callback) {
            if (callback == null) {
                Log.e(TAG, "callback is null");
                return;
            }
            synchronized (sCallBacksLock) {
                mWWANDBProviderResponseListener = null;
                mListenerIntent = null;
            }
            if (VERBOSE) {
                Log.d(TAG, "WWANDBProvider GTPClientHelper.SetClientRegistrationStatus OUT");
            }
            GTPClientHelper.SetClientRegistrationStatus(mContext,
                    GTPClientHelper.GTP_CLIENT_WWAN_PROVIDER, false);
        }


        public int requestBSObsLocData() {
            if (VERBOSE) {
                Log.d(TAG, "in IWWANDBProvider.Stub(): requestBSObsLocData()");
            }

            return native_wwan_db_provider_request_bs_obs_data();
        }

    };

    private void onBsObsLocDataAvailable(BSObsLocationData[] obsDataArray, int status) {
        if (VERBOSE) {
            Log.d(TAG, "onBsObsLocDataAvailable status: " + status);
        }

        List<BSObsLocationData> obsDataList = new ArrayList<BSObsLocationData>();
        try {
            obsDataList.addAll(Arrays.asList(obsDataArray));
        } catch (NullPointerException e) {
            Log.d(TAG, "onAPListAvailable: AP List is NULL");
        }
        synchronized (sCallBacksLock) {
            if (null != mWWANDBProviderResponseListener) {
                try {
                    mWWANDBProviderResponseListener.onBsObsLocDataAvailable(
                            obsDataList, status);
                } catch (RemoteException e) {
                    Log.w(TAG, "onBsObsLocDataAvailable remote exception, sending intent");
                    GTPClientHelper.SendPendingIntent(mContext, mListenerIntent, "WWANDBProvider");
                }
            }
        }
    }

    private void onServiceRequest() {
        if (VERBOSE) {
            Log.d(TAG, "onServiceRequest");
        }
        synchronized (sCallBacksLock) {
            if (null != mWWANDBProviderResponseListener) {
                try {
                    mWWANDBProviderResponseListener.onServiceRequest();
                } catch (RemoteException e) {
                    Log.w(TAG, "onServiceRequest remote exception, sending intent");
                    GTPClientHelper.SendPendingIntent(mContext, mListenerIntent, "WWANDBProvider");
                }
            }
        }
    }

    public IWWANDBProvider getWWANDBProviderBinder() {
        return mBinder;
    }

    protected void finalize() throws Throwable {
        native_wwan_db_provider_deinit();
    }

    static {
        native_wwan_db_provider_class_init();
    }

    private static native void native_wwan_db_provider_class_init();
    private native void native_wwan_db_provider_init();
    private native void native_wwan_db_provider_deinit();
    private native int native_wwan_db_provider_request_bs_obs_data();
}
