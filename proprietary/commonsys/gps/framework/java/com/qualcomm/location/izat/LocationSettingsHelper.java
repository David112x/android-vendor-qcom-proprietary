/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qualcomm.location.izat;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

import android.location.LocationManager;
import android.app.ActivityManager;
import android.os.UserHandle;
import android.os.Binder;
import android.util.Log;

import java.util.HashSet;
import java.util.Iterator;

public class LocationSettingsHelper {
    private static final String TAG = "LocationSettingsHelper";
    private static final boolean VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);
    private static LocationSettingsHelper sInstance = null;

    private final LocationManager mLocationManager;
    private final ActivityManager mActivityManager;
    private HashSet<ILocationSettingsCallback> mLocationSettingsCallbackSet =
            new HashSet<ILocationSettingsCallback>();
    private static final int FOREGROUND_IMPORTANCE_CUTOFF
            = ActivityManager.RunningAppProcessInfo.IMPORTANCE_FOREGROUND_SERVICE;
    public boolean mIsLocationSettingsOn;

    private static boolean isImportanceForeground(int importance) {
        return importance <= FOREGROUND_IMPORTANCE_CUTOFF;
    }
    public static synchronized LocationSettingsHelper getInstance(Context ctx) {
        if (sInstance == null) {
            sInstance = new LocationSettingsHelper(ctx);
        }

        return sInstance;
    }
    public interface ILocationSettingsCallback {
        void onLocationSettingsChange(boolean locationSettingsIsOn);
    }
    public interface IUidImportanceCallback {
        void onUidImportanceChange(int uid, boolean isInBackground);
    }
    public void registerForLocationSettingsCallback(
            ILocationSettingsCallback locationSettingsCallback) {
        mLocationSettingsCallbackSet.add(locationSettingsCallback);
    }

    public void deregisterForLocationSettingsCallback(
            ILocationSettingsCallback locationSettingsCallback) {
        mLocationSettingsCallbackSet.remove(locationSettingsCallback);
    }
    public void registerUidImportanceCallback(IUidImportanceCallback uidImportanceCallback) {
        ActivityManager.OnUidImportanceListener uidImportanceListener =
                new ActivityManager.OnUidImportanceListener() {
            public void onUidImportance(int uid, int importance) {
                boolean isForegroundUid = isImportanceForeground(importance);
                uidImportanceCallback.onUidImportanceChange(uid, isForegroundUid);
            }
        };
        final long origId = Binder.clearCallingIdentity();
        mActivityManager.addOnUidImportanceListener(uidImportanceListener,
                FOREGROUND_IMPORTANCE_CUTOFF);
        Binder.restoreCallingIdentity(origId);
    }

    private LocationSettingsHelper(Context ctx) {
        mLocationManager = (LocationManager) ctx.getSystemService(Context.LOCATION_SERVICE);
        mActivityManager = (ActivityManager) ctx.getSystemService(Context.ACTIVITY_SERVICE);
        ctx.registerReceiver(mLocationSettingsReceiver,
                             new IntentFilter(LocationManager.PROVIDERS_CHANGED_ACTION));
        mIsLocationSettingsOn = isLocationSettingsOn();
    }

    private boolean isLocationSettingsOn() {
        boolean isLocationSettingsOn =
                mLocationManager.isLocationEnabledForUser(Binder.getCallingUserHandle());

        if (VERBOSE) {
            Log.v(TAG, "isLocationSettingsOn: " + isLocationSettingsOn);
        }

        return isLocationSettingsOn;
    }

    private BroadcastReceiver mLocationSettingsReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            if (intent.getAction().equals("android.location.PROVIDERS_CHANGED")) {
                synchronized (this) {
                    boolean isLocationSettingsOnNow = isLocationSettingsOn();
                    if (VERBOSE) {
                        Log.v(TAG, "isLocationSettingsOnNow: " + isLocationSettingsOnNow +
                                " mIsLocationSettingsOn: " + mIsLocationSettingsOn);
                    }
                    if (isLocationSettingsOnNow != mIsLocationSettingsOn) {
                        mIsLocationSettingsOn = isLocationSettingsOnNow;
                        Iterator<ILocationSettingsCallback> it =
                                mLocationSettingsCallbackSet.iterator();
                        while (it.hasNext()) {
                            try {
                                ILocationSettingsCallback locationSettingsCallback = it.next();
                                locationSettingsCallback.onLocationSettingsChange(
                                        mIsLocationSettingsOn);
                            } catch (Exception e) {
                                Log.e(TAG, "Invalid locationSettingsCallback.");
                                it.remove();
                            }
                        }
                    }
                }
            }
        }
    };
}
