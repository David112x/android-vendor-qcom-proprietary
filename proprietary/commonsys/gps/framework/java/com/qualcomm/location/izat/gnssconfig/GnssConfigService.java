/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qualcomm.location.izat.gnssconfig;

import android.content.Context;
import android.os.IBinder;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.os.Bundle;

import android.util.Log;
import java.lang.IndexOutOfBoundsException;
import java.util.List;
import java.util.ArrayList;
import java.util.Map;

import com.qti.gnssconfig.*;
import com.qualcomm.location.izat.CallbackData;
import com.qualcomm.location.izat.DataPerPackageAndUser;

public class GnssConfigService {
    private static final String TAG = "GnssConfigService";
    private static final boolean VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);

    private static final Object sCallBacksLock = new Object();
    private RemoteCallbackList<IGnssConfigCallback> mGnssConfigCallbacks
        = new RemoteCallbackList<IGnssConfigCallback>();
    private final Context mContext;

    private class ClientGnssConfigData extends CallbackData {
        private IGnssConfigCallback mCallback;

        public ClientGnssConfigData(IGnssConfigCallback callback) {
            mCallback = callback;
            super.mCallback = callback;
        }
    }

    private DataPerPackageAndUser<ClientGnssConfigData> mDataPerPackageAndUser;

    private static GnssConfigService sInstance = null;
    public static GnssConfigService getInstance(Context ctx) {
        if (sInstance == null) {
            sInstance = new GnssConfigService(ctx);
        }
        return sInstance;
    }

    private GnssConfigService(Context ctx) {
        if (VERBOSE) {
            Log.d(TAG, "GnssConfigService construction");
        }

        mContext = ctx;
        mDataPerPackageAndUser = new DataPerPackageAndUser<ClientGnssConfigData>(mContext,
                new UserChanged());

        native_gnssconfig_init();
    }

    /* Remote binder */
    private final IGnssConfigService.Stub mBinder = new IGnssConfigService.Stub() {

        public void registerCallback(final IGnssConfigCallback callback) {
            if (callback == null) {
                Log.e(TAG, "callback is null");
                return;
            }

            synchronized (sCallBacksLock) {
                if (VERBOSE) {
                    Log.d(TAG, "getGnssSvTypeConfig: " +
                            mDataPerPackageAndUser.getPackageName(null));
                }


                ClientGnssConfigData clData = mDataPerPackageAndUser.getData();
                if (null == clData) {
                    clData = new ClientGnssConfigData(callback);
                    mDataPerPackageAndUser.setData(clData);
                } else {
                    if (null != clData.mCallback) {
                        mGnssConfigCallbacks.unregister(clData.mCallback);
                    }
                    clData.mCallback = callback;
                }

                mGnssConfigCallbacks.register(callback);
            }

            try {
                callback.asBinder().linkToDeath(new IBinder.DeathRecipient() {
                    @Override
                    public void binderDied() {
                        synchronized (sCallBacksLock) {
                            ClientGnssConfigData clData =
                                    mDataPerPackageAndUser.getDataByCallback(callback);

                            if (null != clData) {
                                if (VERBOSE) {
                                    Log.d(TAG, "Package died: " +
                                            clData.mPackageName);
                                }
                            }

                            mGnssConfigCallbacks.unregister(callback);
                        }
                    }
                }, 0);
            } catch (RemoteException e) {
                throw new RuntimeException("Failed unregister gnss config cb", e);
            }
        }

        public void getGnssSvTypeConfig() {
            if (VERBOSE) {
                Log.d(TAG, "getGnssSvTypeConfig: " + mDataPerPackageAndUser.getPackageName(null));
            }

            synchronized(sCallBacksLock) {

                native_gnssconfig_getGnssSvTypeConfig();
            }
        }

        public void setGnssSvTypeConfig(int[] disabledSvTypeArray) {

            synchronized (sCallBacksLock) {

                Log.d(TAG, "setGnssSvTypeConfig: arrLen: " + disabledSvTypeArray.length +
                           ", package: " + mDataPerPackageAndUser.getPackageName(null));

                native_gnssconfig_setGnssSvTypeConfig(disabledSvTypeArray);
            }
        }

        public void resetGnssSvTypeConfig() {

            synchronized(sCallBacksLock) {
                if (VERBOSE) {
                    Log.d(TAG, "resetGnssSvTypeConfig: " +
                                mDataPerPackageAndUser.getPackageName(null));
                }
                native_gnssconfig_resetGnssSvTypeConfig();
            }
        }
    };

    private void onGnssSvTypeConfigCb(int[] disabledSvTypeArray) {

        synchronized (sCallBacksLock) {

            Log.d(TAG, "onGnssSvTypeConfigCb: " + mDataPerPackageAndUser.getPackageName(null));

            for (ClientGnssConfigData clData : mDataPerPackageAndUser.getAllData()) {
                if (VERBOSE) {
                    Log.d(TAG, "Invoking cb for client: " + clData.mPackageName);
                }
                try {
                    clData.mCallback.getGnssSvTypeConfigCb(disabledSvTypeArray);
                } catch (RemoteException e) {
                    Log.e(TAG, "onGnssSvTypeConfigCb: Failed to invoke cb");
                }
            }
        }
    }

    class UserChanged implements DataPerPackageAndUser.UserChangeListener<ClientGnssConfigData> {
        @Override
        public void onUserChange(Map<String, ClientGnssConfigData> prevUserData,
                                 Map<String, ClientGnssConfigData> currentUserData) {
            if (VERBOSE) {
                Log.d(TAG, "Active user has changed, updating gnssConfig callbacks...");
            }

            synchronized (sCallBacksLock) {
                // Remove prevUser callbacks
                for (ClientGnssConfigData gnssConfigData: prevUserData.values()) {
                    mGnssConfigCallbacks.unregister(gnssConfigData.mCallback);
                }

                // Add back current user callbacks
                for (ClientGnssConfigData gnssConfigData: currentUserData.values()) {
                    mGnssConfigCallbacks.register(gnssConfigData.mCallback);
                }
            }
        }
    }

    public IGnssConfigService getGnssConfigBinder() {
        return mBinder;
    }

    static {
        native_gnssconfig_class_init();
    }

    private static native void native_gnssconfig_class_init();
    private native void native_gnssconfig_init();
    private native void native_gnssconfig_getGnssSvTypeConfig();
    private native void native_gnssconfig_setGnssSvTypeConfig(int[] disabledSvTypeArray);
    private native void native_gnssconfig_resetGnssSvTypeConfig();
}
