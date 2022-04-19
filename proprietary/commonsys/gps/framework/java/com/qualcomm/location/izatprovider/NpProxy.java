/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2015-2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qualcomm.location.izatprovider;

import java.io.IOException;
import java.lang.NullPointerException;
import android.content.pm.PackageManager.NameNotFoundException;
import java.util.concurrent.atomic.AtomicBoolean;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.location.Location;
import android.location.LocationProvider;
import android.location.LocationListener;
import android.location.LocationManager;
import android.location.LocationRequest;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Looper;
import android.os.Message;
import android.os.SystemProperties;
import android.os.UserHandle;
import android.os.WorkSource;
import android.util.Log;
import com.android.internal.location.ProviderProperties;
import java.util.List;
import com.android.internal.location.ILocationProvider;
import com.android.internal.location.ProviderRequest;
import com.android.server.location.LocationProviderProxy;
import com.android.server.location.AbstractLocationProvider;
import com.qualcomm.location.izatprovider.IzatProvider.IzatProviderContext;
import com.qualcomm.location.R;
import com.qualcomm.location.izat.izatconfig.IzatConfigService;

// Wraps OSNP service
public class NpProxy implements Handler.Callback {

    static {
        nativeOnClassLoad();
    }

    int getStatus(Bundle extras) {
        try {
            return mActiveNpHandle.getStatus(extras);
        } catch (NullPointerException npe) {
            Log.w(TAG, npe);
        }
        return LocationProvider.TEMPORARILY_UNAVAILABLE;
    }

    long getStatusUpdateTime() {
        try {
            return mActiveNpHandle.getStatusUpdateTime();
        } catch (NullPointerException npe) {
            Log.w(TAG, npe);
        }
        return 0;
    }

    private int getNpImplPackage(int startIdx, String packageName) {
        for (int i = startIdx; i < mNpImplIds.length; i++) {
            if (mContext.getResources().getString(mNpImplIds[i][0]).equals(packageName)) {
                return i;
            }
        }
        return mNpImplIds.length;
    }

    private void setNpImplePackageNames() {

        String defaultServicePackageName = IzatConfigService.getOsnpPackagename(mContext);
        String regionServicePackageName = IzatConfigService.getRegionalNlpPackagename(mContext);
        int nlp_mode = IzatConfigService.getNlpMode(mContext);
        log("defaultServicePackageName = " + defaultServicePackageName);
        log("regionServicePackageName = " + regionServicePackageName);
        log("nlp_mode = " + nlp_mode);

        boolean osNlpOnly = ( OSNLP_ONLY == nlp_mode);
        mQnpOnly = ( QNP_ONLY == nlp_mode);

        mIzatProviderContext.setNlpConfig(osNlpOnly, mQnpOnly);

        mOsnlpResId = R.string.config_OSNP_PackageName;
        mOsnlpArrayResId = R.array.config_OSNP_NpPackageNames;
        mRegionalOsnlpResId = R.string.config_regional_OSNP_Baidu_PackageName;
        mRegionalOsnlpArrayResId = R.array.config_Baidu_NpPackageNames;

        int implIdx = mNpImplIds.length;
        if ((implIdx = getNpImplPackage(0, defaultServicePackageName)) < mNpImplIds.length) {
            mOsnlpResId = mNpImplIds[implIdx][0];
            mOsnlpArrayResId = mNpImplIds[implIdx][1];
            mAccuracyThreshold = mNpImplIds[implIdx][2];
        }

        if ((implIdx = getNpImplPackage(1, regionServicePackageName)) < mNpImplIds.length) {
            mRegionalOsnlpResId = mNpImplIds[implIdx][0];
            mRegionalOsnlpArrayResId = mNpImplIds[implIdx][1];
            mRegionalAccuracyThreshold = mNpImplIds[implIdx][2];
        }
    }

    private boolean checkPackageExists(String packageName)
    {
        boolean exists = (packageName != null && !packageName.isEmpty());

        if (exists) {
            try {
                String info = mContext.getPackageManager()
                                      .getPackageInfo(packageName, 0)
                                      .versionName;
                exists = (info != null || !info.isEmpty());
            } catch (NameNotFoundException e) {
                exists = false;
            }
        }

        return exists;
    }

    private void initServiceBinding() {
        if (null == mOSNpHandle && !mQnpOnly) {
            mOSNpHandle = new LocationController();
            LocationProviderProxy OsNp =
                    LocationProviderProxy.createAndBind(mContext, mOSNpHandle,
                                                        NETWORK_LOCATION_SERVICE_ACTION,
                                                        R.bool.config_enableNpOverlay,
                                                        mOsnlpResId,
                                                        mOsnlpArrayResId);
            mOSNpHandle.attachProvider(OsNp);
        }

        if (null == mRegionOSNpHandle) {
            mRegionOSNpHandle = new LocationController();
            LocationProviderProxy RegionalNp =
                    LocationProviderProxy.createAndBind(mContext, mRegionOSNpHandle,
                                                        NETWORK_LOCATION_SERVICE_ACTION,
                                                        R.bool.config_enableNpOverlay,
                                                        mRegionalOsnlpResId,
                                                        mRegionalOsnlpArrayResId);
            mRegionOSNpHandle.attachProvider(RegionalNp);
        }

        updateServiceBinding();
    }

    private void updateServiceBinding() {
        boolean useRegional = mDeviceInRegion;
        boolean hasDefaultNp = checkPackageExists(mContext.getResources().getString(mOsnlpResId));
        boolean hasRegionNp = checkPackageExists(mContext.getResources()
                .getString(mRegionalOsnlpResId));

        log("hasRegionNp = " + hasRegionNp + "; hasDefaultNp = " +
            hasDefaultNp);

        // only having default NP doesn't mean we could use it. If device is in
        // the reagion where it can not be used, i.e if (useRegional == true),
        // having default NP does no good. So hasAndAllowDefaultNp means we have
        // default NP AND we can use it.
        // Also if configuration is QnpOnly, don't use it.
        boolean hasAndAllowDefaultNp = (hasDefaultNp && !mQnpOnly && !useRegional);

        // prefer default NP if we can use it
        if (hasAndAllowDefaultNp) {
            mActiveNpHandle = mOSNpHandle;
            mActiveAccuracyThreshold = mAccuracyThreshold;
            log("OS NLP is selected to run");
        } else if (hasRegionNp) {
            mActiveNpHandle = mRegionOSNpHandle;
            mActiveAccuracyThreshold = mRegionalAccuracyThreshold;
            log("Regional NLP is selected to run");
        } else {
            // no NLP we can use
            Log.w(TAG, "Device has no NLP service to use");
        }
    }

    private void setRequestTry() {
        if (null != mRequests) {
            try {
                mWorkSource.clear();
                if (mRequests.reportLocation) {
                    if (null != mIzatProviderContext.getNetworkProviderInner(mContext)) {
                        WorkSource ws = mIzatProviderContext
                                .getNetworkProviderInner(mContext).getWorkSource();
                        if (null != ws) {
                            mWorkSource.add(ws);
                        }
                    }
                }
                mActiveNpHandle.setRequest(mRequests, mWorkSource);
                if (!mRequests.reportLocation) {
                    mRequests = null;
                }
            } catch (NullPointerException npe) {
                Log.w(TAG, npe);
            }
        }
    }

    NpProxy(Context context, Looper looper, IzatProviderContext izatProviderContext) {
        mContext = context;
        mIzatProviderContext = izatProviderContext;
        mHandler = new Handler(looper, this);
        mHandler.sendEmptyMessage(MSG_INIT);
        mDeviceInRegion = false;
        mRequests = null;
    }

    void destroy() {
        mRequests = null;
        nativeOnInstanceDeinit();
    }

    // Methods Called by native code
    private void onSetRequest(long interval, float smallestDisplacement, int numUpdates) {
        LocationRequest locationRequest = LocationRequest
                .createFromDeprecatedProvider(LocationManager.NETWORK_PROVIDER,
                                              interval, smallestDisplacement,
                                              numUpdates == 1);
        ProviderRequest providerRequest = new ProviderRequest ();
        providerRequest.interval = interval;
        providerRequest.reportLocation = true;
        providerRequest.locationRequests.add (locationRequest);

        mHandler.obtainMessage(MSG_SET_REQUEST, providerRequest).sendToTarget();
    }

    private void onStopRequest() {
        mHandler.obtainMessage(MSG_SET_REQUEST, new ProviderRequest()).sendToTarget();
    }

    private boolean clearMarkerIfForScreening(Location location) {
        Bundle extras = location.getExtras();
        boolean marked = (extras != null &&
                          extras.containsKey ("com.qualcomm.location.nlp:screen"));

        if (marked) {
            extras.remove("com.qualcomm.location.nlp:screen");
        }

        return marked;
    }

    static private void log(String msg) {
        if (DEBUG) Log.d(TAG, msg);
    }

    // Message handler
    public boolean handleMessage(Message msg) {
        int msgID = msg.what;
        log("handleMessage what - " + msgID);

        switch (msgID) {
        case MSG_INIT:
        {
            setNpImplePackageNames();

            initServiceBinding();

            nativeOnInstanceInit();
            break;
        }
        case MSG_SET_REQUEST:
        {
            mRequests = (ProviderRequest)msg.obj;
            setRequestTry();
            break;
        }
        case MSG_LOCATION_CHANGED:
        {
            int source = -1;
            Location location = (Location)msg.obj;

            if (location != null) {

                log("Location: " + location.toString());

                Bundle extras = location.getExtras();
                if (extras != null) {
                    String strSource = extras.getString(IzatProvider.NLP_TYPE_KEY, "");
                    if (strSource != null) {
                        if (strSource.equalsIgnoreCase("cell")) {
                            source = 0;
                        } else if (strSource.equalsIgnoreCase("wifi")) {
                            source = 1;
                        }
                    }
                }

                nativeOnLocationChanged(true, location.getTime(),
                                        true, location.getElapsedRealtimeNanos(),
                                        true, location.getLatitude(),
                                        true, location.getLongitude(),
                                        location.hasAccuracy(),
                                        location.getAccuracy() < mActiveAccuracyThreshold?
                                        mActiveAccuracyThreshold : location.getAccuracy(),
                                        location.hasVerticalAccuracy(),
                                        location.getVerticalAccuracyMeters(),
                                        location.hasAltitude(), location.getAltitude(),
                                        location.hasBearing(), location.getBearing(),
                                        location.hasSpeed(), location.getSpeed(),
                                        source != -1, source);
            }
            break;
        }
        case MSG_REGIONAL_NP_UPDATE:
            boolean deviceInRegion = (msg.arg1 == 1);
            if (mDeviceInRegion != deviceInRegion) {
                mDeviceInRegion = deviceInRegion;
                updateServiceBinding();
            }
            break;
        default:
            Log.w(TAG, "Unhandled Message " + msg.what);
        }
        return true;
    }

    private class LocationController implements AbstractLocationProvider.LocationProviderManager {

        private AbstractLocationProvider mProvider;

        public void attachProvider(AbstractLocationProvider provider) {
            mProvider = provider;
        }

        public void setRequest(ProviderRequest request, WorkSource workSource) {
            try {
                mProvider.setRequest(request, workSource);
            } catch (NullPointerException npe) {
                Log.w(TAG, npe);
            }
        }

        public int getStatus(Bundle extras) {
            try {
                return mProvider.getStatus(extras);
            } catch (NullPointerException npe) {
                Log.w(TAG, npe);
            }
            return LocationProvider.TEMPORARILY_UNAVAILABLE;
        }

        public long getStatusUpdateTime() {
            try {
                return mProvider.getStatusUpdateTime();
            } catch (NullPointerException npe) {
                Log.w(TAG, npe);
            }
            return 0;
        }

        @Override
        public void onReportLocation(Location location) {
            if (LocationManager.NETWORK_PROVIDER.equals(location.getProvider())) {
                mHandler.obtainMessage(MSG_LOCATION_CHANGED, location).sendToTarget();
            }
        }

        @Override
        public void onReportLocation(List<Location> locations) {
        }

        @Override
        public void onSetEnabled(boolean enabled) {
            if (null != mIzatProviderContext.getNetworkProviderInner(mContext)) {
                if (null != mIzatProviderContext) {
                    mIzatProviderContext.setOsConsent(enabled);
                }
            }
        }

        @Override
        public void onSetProperties(ProviderProperties properties) {
        }
    }

    // native method declarations.
    private static native void nativeOnClassLoad();
    private native void nativeOnInstanceInit();
    private native void nativeOnInstanceDeinit();
    private native void nativeOnLocationChanged(boolean hasTime, long time,
                                                boolean hasTimeNanos, long timeNanos,
                                                boolean hasLatitude, double latitude,
                                                boolean hasLongitude, double longitude,
                                                boolean hasAccuracy, float accuracy,
                                                boolean hasVerticalAccuracy, float verticalAccuracy,
                                                boolean hasAltitude, double altitude,
                                                boolean hasBearing, float bearing,
                                                boolean hasSpeed, float speed,
                                                boolean hasSource, int source);


    // Constant data members
    private static final int        MSG_INIT                  = 1;
    private static final int        MSG_SET_REQUEST           = 3;
    private static final int        MSG_LOCATION_CHANGED      = 4;
    private static final int        MSG_REGIONAL_NP_UPDATE    = 5;

    private static final int        OSNLP_ONLY                = 1;
    private static final int        QNP_ONLY                  = 2;

    private static final String     TAG   = "NpProxy";
    private static final boolean    DEBUG = Log.isLoggable (TAG, Log.DEBUG);

    // static class members
    // data members
    private Handler                 mHandler;
    private Context                 mContext;
    private IzatProviderContext     mIzatProviderContext;
    private WorkSource              mWorkSource         = new WorkSource();
    private boolean                 mDeviceInRegion;
    private ProviderRequest         mRequests;

    private LocationController      mOSNpHandle;
    private LocationController      mRegionOSNpHandle;
    private LocationController      mActiveNpHandle;
    private float                   mAccuracyThreshold;
    private float                   mRegionalAccuracyThreshold;
    private float                   mActiveAccuracyThreshold;
    private boolean                 mQnpOnly;

    // initialized in the static clause, per izat.conf
    private static int              mRegionalOsnlpResId;
    private static int              mOsnlpResId;
    private static int              mRegionalOsnlpArrayResId;
    private static int              mOsnlpArrayResId;

    //3rd column is used to populate hardcoded threshold associated with that NLP implementation
    private static final int mNpImplIds[][] = {
            {R.string.config_OSNP_PackageName, R.array.config_OSNP_NpPackageNames, 0},
            {R.string.config_regional_OSNP_Baidu_PackageName, R.array.config_Baidu_NpPackageNames,
            1000},
            {R.string.config_regional_OSNP_AutoNavi_PackageName,
             R.array.config_AutoNavi_NpPackageNames, 0},
            {R.string.config_regional_OSNP_Tencent_PackageName,
             R.array.config_Tencent_NpPackageNames, 0},
            {R.string.config_regional_OSNP_AutoNavi_PackageName_1,
             R.array.config_AutoNavi_NpPackageNames_1, 0}};

    private static final String NETWORK_LOCATION_SERVICE_ACTION =
            "com.android.location.service.v3.NetworkLocationProvider";

    public void setRegionalNpRegulate(boolean regulated) {
        mHandler.obtainMessage(MSG_REGIONAL_NP_UPDATE, regulated ? 1 : 0, 0).sendToTarget();
    }
}
