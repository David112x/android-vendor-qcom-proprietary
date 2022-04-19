/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qualcomm.location.izat.wwandbreceiver;

import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.location.Location;
import android.os.IBinder;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.util.Log;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashMap;
import java.util.List;

import com.qti.wwandbreceiver.*;
import com.qualcomm.location.izat.GTPClientHelper;

public class WWANDBReceiver {
    private static final String TAG = "WWANDBReceiver";
    private static final boolean VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);

    private static final Object sCallBacksLock = new Object();
    private final Context mContext;
    // In case and old SDK client < 7.0 is used
    private boolean mIsLegacySDKClient = false;

    private IWWANDBReceiverResponseListener mWWANDBReceiverResponseListener = null;
    private PendingIntent mListenerIntent = null;

    private static WWANDBReceiver sInstance = null;
    public static WWANDBReceiver getInstance(Context ctx) {
        if (sInstance == null) {
            sInstance = new WWANDBReceiver(ctx);
        }
        return sInstance;
    }

    private WWANDBReceiver(Context ctx) {
        if (VERBOSE) {
            Log.d(TAG, "WWANDBReceiver construction");
        }
        mContext = ctx;
        native_wwan_db_receiver_init();
    }

    /* Remote binder */
    private final IWWANDBReceiver.Stub mBinder = new IWWANDBReceiver.Stub() {

        @Deprecated
        // For backwards compatibility with possible static-linked SDK
        public boolean registerResponseListener(final IWWANDBReceiverResponseListener callback) {
            mIsLegacySDKClient = true;
            return registerResponseListenerExt(callback, null);
        }

        public boolean registerResponseListenerExt(final IWWANDBReceiverResponseListener callback,
                                                   PendingIntent notifyIntent) {
            if (callback == null) {
                Log.e(TAG, "callback is null");
                return false;
            }

            if (notifyIntent == null) {
                Log.w(TAG, "notifyIntent is null");
            }

            if (null != mWWANDBReceiverResponseListener) {
                Log.e(TAG, "Response listener already provided.");
                return false;
            }

            synchronized (sCallBacksLock) {
                mWWANDBReceiverResponseListener = callback;
                mListenerIntent = notifyIntent;
            }

            try {
                callback.asBinder().linkToDeath(new IBinder.DeathRecipient() {
                    @Override
                    public void binderDied() {
                        synchronized (sCallBacksLock) {
                            mWWANDBReceiverResponseListener = null;
                            mListenerIntent = null;
                        }
                    }
                }, 0);
            } catch (RemoteException e) {
                throw new RuntimeException("Failed unregister WWAN DB provider cb", e);
            }

            if (VERBOSE) {
                Log.d(TAG, "WWANDBReceiver GTPClientHelper.SetClientRegistrationStatus IN");
            }
            GTPClientHelper.SetClientRegistrationStatus(mContext,
                    GTPClientHelper.GTP_CLIENT_WWAN_RECEIVER, true);

            return true;
        }

        public void removeResponseListener(final IWWANDBReceiverResponseListener callback) {
            if (null == callback) {
                Log.e(TAG, "callback is null");
                return;
            }
            synchronized (sCallBacksLock) {
                mWWANDBReceiverResponseListener = null;
                mListenerIntent = null;
            }
            if (VERBOSE) {
                Log.d(TAG, "WWANDBReceiver GTPClientHelper.SetClientRegistrationStatus OUT");
            }
            GTPClientHelper.SetClientRegistrationStatus(mContext,
                    GTPClientHelper.GTP_CLIENT_WWAN_RECEIVER, false);
        }


        public int requestBSList(int expireInDays) {
            if (VERBOSE) {
                Log.d(TAG, "in IWWANDBReceiver.Stub(): requestBSList()");
            }
            return native_wwan_db_receiver_request_bs_list(expireInDays);
        }

        public int pushWWANDB(List<BSLocationData> locData,
                              List<BSSpecialInfo> splData,
                              int daysValid) {
            if (VERBOSE) {
                Log.d(TAG, "in IWWANDBReceiver.Stub(): pushWWANDB() ");
            }
            BSLocationData[] locs = locData.toArray(new BSLocationData[0]);

            BSSpecialInfo[] spls = splData.toArray(new BSSpecialInfo[0]);
            return native_wwan_db_receiver_push_ad(locs, spls, daysValid);
        }
    };

    private void onBSListAvailable(BSInfo[] bsInfoList, int status, Location location) {
        if (VERBOSE) {
            Log.d(TAG, "onBSListAvailable");
        }
        List bsList = new ArrayList<BSInfo>();
        try {
            bsList.addAll(Arrays.asList(bsInfoList));
        } catch (NullPointerException e) {
            Log.d(TAG, "onBSListAvailable: BS List is NULL");
        }
        synchronized (sCallBacksLock) {
            if (null != mWWANDBReceiverResponseListener) {
                try {
                    if (mIsLegacySDKClient) {
                        mWWANDBReceiverResponseListener.onBSListAvailable(bsList);
                    } else {
                        mWWANDBReceiverResponseListener.onBSListAvailableExt(
                                bsList, status, location);
                    }
                } catch (RemoteException e) {
                    Log.w(TAG, "onBSListAvailable remote exception, sending intent");
                    GTPClientHelper.SendPendingIntent(mContext, mListenerIntent, "WWANDBReceiver");
                }
            }
        }
    }

    private void onStatusUpdate(boolean isSuccess, String error) {
        if (VERBOSE) {
            Log.d(TAG, "onStatusUpdate");
        }
        synchronized (sCallBacksLock) {
            if (null != mWWANDBReceiverResponseListener) {
                try {
                    mWWANDBReceiverResponseListener.onStatusUpdate(isSuccess, error);
                    if (VERBOSE) {
                        Log.d(TAG, "onStatusUpdate: send update to listener");
                    }
                } catch (RemoteException e) {
                    Log.w(TAG, "onStatusUpdate remote exception, sending intent");
                    GTPClientHelper.SendPendingIntent(mContext, mListenerIntent, "WWANDBReceiver");
                }
            }
        }
    }

    private void onServiceRequest() {
        if (VERBOSE) {
            Log.d(TAG, "onServiceRequest");
        }
        synchronized (sCallBacksLock) {
            if (null != mWWANDBReceiverResponseListener) {
                try {
                    mWWANDBReceiverResponseListener.onServiceRequest();
                } catch (RemoteException e) {
                    Log.w(TAG, "onServiceRequest remote exception, sending intent");
                    GTPClientHelper.SendPendingIntent(mContext, mListenerIntent, "WWANDBReceiver");
                }
            }
        }
    }

    public IWWANDBReceiver getWWANDBReceiverBinder() {
        return mBinder;
    }

    protected void finalize() throws Throwable {
        native_wwan_db_receiver_deinit();
    }

    static {
        native_wwan_db_receiver_class_init();
    }

    private static native void native_wwan_db_receiver_class_init();
    private native void native_wwan_db_receiver_init();
    private native void native_wwan_db_receiver_deinit();
    private native int native_wwan_db_receiver_request_bs_list(int expireInDays);
    private native int native_wwan_db_receiver_push_ad(BSLocationData[] locData,
                                                       BSSpecialInfo[] splData,
                                                       int daysValid);
}
