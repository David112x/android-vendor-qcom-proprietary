/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qualcomm.location.izat.wifidbreceiver;

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

import com.qti.wifidbreceiver.*;
import com.qualcomm.location.izat.GTPClientHelper;

public class WiFiDBReceiver {
    private static final String TAG = "WiFiDBReceiver";
    private static final boolean VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);

    // As defined in LocHidlWifiDBListStatus & GTP
    private static final int LOOKUP_STATUS = 3;

    private static final Object sCallBacksLock = new Object();
    private final Context mContext;
    // In case and old SDK client < 7.0 is used
    private boolean mIsLegacySDKClient = false;

    private IWiFiDBReceiverResponseListener mWiFiDBReceiverResponseListener = null;
    private PendingIntent mListenerIntent = null;

    private static WiFiDBReceiver sInstance = null;
    public static WiFiDBReceiver getInstance(Context ctx) {
        if (sInstance == null) {
            sInstance = new WiFiDBReceiver(ctx);
        }
        return sInstance;
    }

    private WiFiDBReceiver(Context ctx) {
        if (VERBOSE) {
            Log.d(TAG, "WiFiDBReceiver construction");
        }
        mContext = ctx;
        native_wifi_db_receiver_init();
    }

    /* Remote binder */
    private final IWiFiDBReceiver.Stub mBinder = new IWiFiDBReceiver.Stub() {

        @Deprecated
        // For backwards compatibility with possible static-linked SDK
        public boolean registerResponseListener(final IWiFiDBReceiverResponseListener callback) {
            mIsLegacySDKClient = true;
            return registerResponseListenerExt(callback, null);
        }

        public boolean registerResponseListenerExt(final IWiFiDBReceiverResponseListener callback,
                                                PendingIntent notifyIntent) {
            if (callback == null) {
                Log.e(TAG, "callback is null");
                return false;
            }

            if (notifyIntent == null) {
                Log.w(TAG, "notifyIntent is null");
            }

            if (null != mWiFiDBReceiverResponseListener) {
                Log.e(TAG, "Response listener already provided.");
                return false;
            }

            synchronized (sCallBacksLock) {
                mWiFiDBReceiverResponseListener = callback;
                mListenerIntent = notifyIntent;
            }

            try {
                callback.asBinder().linkToDeath(new IBinder.DeathRecipient() {
                    @Override
                    public void binderDied() {
                        synchronized (sCallBacksLock) {
                            mWiFiDBReceiverResponseListener = null;
                            mListenerIntent = null;
                        }
                    }
                }, 0);
            } catch (RemoteException e) {
                throw new RuntimeException("Failed unregister WiFi DB provider cb", e);
            }

            if (VERBOSE) {
                Log.d(TAG, "WiFiDBReceiver GTPClientHelper.SetClientRegistrationStatus IN");
            }
            GTPClientHelper.SetClientRegistrationStatus(mContext,
                    GTPClientHelper.GTP_CLIENT_WIFI_RECEIVER, true);

            return true;
        }

        public void removeResponseListener(final IWiFiDBReceiverResponseListener callback) {
            if (callback == null) {
                Log.e(TAG, "callback is null");
                return;
            }
            synchronized (sCallBacksLock) {
                mWiFiDBReceiverResponseListener = null;
                mListenerIntent = null;
            }
            if (VERBOSE) {
                Log.d(TAG, "WiFiDBReceiver GTPClientHelper.SetClientRegistrationStatus OUT");
            }
            GTPClientHelper.SetClientRegistrationStatus(mContext,
                    GTPClientHelper.GTP_CLIENT_WIFI_RECEIVER, false);
        }


        public int requestAPList(int expireInDays) {
            if (VERBOSE) {
                Log.d(TAG, "in IWiFiDBReceiver.Stub(): requestAPList()");
            }

            return native_wifi_db_receiver_request_ap_list(expireInDays);
        }

        public int requestScanList() {
            if (VERBOSE) {
                Log.d(TAG, "in IWiFiDBReceiver.Stub(): requestScanList()");
            }

            return native_wifi_db_receiver_request_scan_list();
        }

        public int pushWiFiDB(List<APLocationData> locData,
                              List<APSpecialInfo> splData,
                              int daysValid) {
            // Legacy SDK used reverse MAC as 445566112233 like original WiFi module
            if (mIsLegacySDKClient) {
                for (APLocationData apLocationData: locData) {
                    apLocationData.mMacAddress = reverseLo24Hi24(apLocationData.mMacAddress);
                }
                for (APSpecialInfo apSpecialInfo: splData) {
                    apSpecialInfo.mMacAddress = reverseLo24Hi24(apSpecialInfo.mMacAddress);
                }
            }

            APLocationData[] locs = locData.toArray(new APLocationData[0]);
            APSpecialInfo[] spls = splData.toArray(new APSpecialInfo[0]);
            return native_wifi_db_receiver_push_ad(locs, spls, daysValid, false);
        }

        public int pushLookupResult(List<APLocationData> locData,
                              List<APSpecialInfo> splData) {
            APLocationData[] locs = locData.toArray(new APLocationData[0]);
            APSpecialInfo[] spls = splData.toArray(new APSpecialInfo[0]);
            return native_wifi_db_receiver_push_ad(locs, spls, 0, true);
        }
    };

    private void onAPListAvailable(APInfoExt[] apInfoExtArray, int status, Location location) {
        if (VERBOSE) {
            Log.d(TAG, "onAPListAvailable status: " + status);
        }
        List<APInfoExt> apInfoExtList = new ArrayList<APInfoExt>();
        try {
            apInfoExtList.addAll(Arrays.asList(apInfoExtArray));
        } catch (NullPointerException e) {
            Log.d(TAG, "onAPListAvailable: AP List is NULL");
        }
        synchronized (sCallBacksLock) {
            if (null != mWiFiDBReceiverResponseListener) {
                try {
                    if (mIsLegacySDKClient) {
                        List<APInfo> apInfoList = new ArrayList<APInfo>();
                        for (APInfoExt apInfoExt: apInfoExtList) {
                            APInfo apInfo = new APInfo();
                            apInfo.mMacAddress = apInfoExt.mMacAddress;
                            // Legacy SDK used reverse MAC as 445566112233 like original WiFi module
                            apInfo.mMacAddress = reverseLo24Hi24(apInfo.mMacAddress);
                            apInfo.mSSID = apInfoExt.mSSID;
                            apInfo.mCellType = apInfoExt.mCellType;
                            apInfo.mCellRegionID1 = apInfoExt.mCellRegionID1;
                            apInfo.mCellRegionID2 = apInfoExt.mCellRegionID2;
                            apInfo.mCellRegionID3 = apInfoExt.mCellRegionID3;
                            apInfo.mCellRegionID4 = apInfoExt.mCellRegionID4;

                            apInfoList.add(apInfo);
                        }

                        mWiFiDBReceiverResponseListener.onAPListAvailable(
                                apInfoList);
                    } else {
                        // Check special status to indicate it is the new onLookupRequest
                        if (LOOKUP_STATUS == status) {
                            mWiFiDBReceiverResponseListener.onLookupRequest(
                                    apInfoExtList, location);
                        } else {
                            mWiFiDBReceiverResponseListener.onAPListAvailableExt(
                                    apInfoExtList, status, location);
                        }
                    }
                } catch (RemoteException e) {
                    Log.w(TAG, "onAPListAvailable remote exception, sending intent");
                    GTPClientHelper.SendPendingIntent(mContext, mListenerIntent, "WiFiDBReceiver");
               }
            }
        }
    }

    private void onStatusUpdate(boolean isSuccess, String error) {
        if (VERBOSE) {
            Log.d(TAG, "onStatusUpdate");
        }
        synchronized (sCallBacksLock) {
            if (null != mWiFiDBReceiverResponseListener) {
                try {
                    mWiFiDBReceiverResponseListener.onStatusUpdate(isSuccess,
                                                                                        error);
                    if (VERBOSE) {
                        Log.d(TAG, "onStatusUpdate: send update to listener");
                    }
                } catch (RemoteException e) {
                    Log.w(TAG, "onStatusUpdate remote exception, sending intent");
                    GTPClientHelper.SendPendingIntent(mContext, mListenerIntent, "WiFiDBReceiver");
                }
            }
        }
    }

    private void onServiceRequest() {
        if (VERBOSE) {
            Log.d(TAG, "onServiceRequest");
        }
        synchronized (sCallBacksLock) {
            if (null != mWiFiDBReceiverResponseListener) {
                try {
                    mWiFiDBReceiverResponseListener.onServiceRequest();
                } catch (RemoteException e) {
                    Log.w(TAG, "onServiceRequest remote exception, sending intent");
                    GTPClientHelper.SendPendingIntent(mContext, mListenerIntent,
                            "WiFiDBReceiver");
                }
            }
        }
    }

    public IWiFiDBReceiver getWiFiDBReceiverBinder() {
        return mBinder;
    }

    protected void finalize() throws Throwable {
        native_wifi_db_receiver_deinit();
    }

    private static String reverseLo24Hi24(String mac) {
        String res = mac;
        if (null != mac && mac.length() == 12) {
            res = mac.substring(6, 12) + mac.substring(0, 6);
        }
        return res;
    }

    static {
        native_wifi_db_receiver_class_init();
    }

    private static native void native_wifi_db_receiver_class_init();
    private native void native_wifi_db_receiver_init();
    private native void native_wifi_db_receiver_deinit();
    private native int native_wifi_db_receiver_request_ap_list(int expireInDays);
    private native int native_wifi_db_receiver_request_scan_list();
    // Lookup & normal push share same signature and pipe, is_lookup is interpreted in GTP
    private native int native_wifi_db_receiver_push_ad(APLocationData[] locData,
                                                       APSpecialInfo[] splData,
                                                       int daysValid, boolean is_lookup);
}
