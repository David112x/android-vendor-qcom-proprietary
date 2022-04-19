/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*

GENERAL DESCRIPTION

  IzatProviderBase - Base class for network and unified providers

  Copyright  (c) 2015, 2017 Qualcomm Technologies, Inc.
  All Rights Reserved. Qualcomm Technologies Proprietary and Confidential.

  Not a Contribution, Apache license notifications and
  license are retained for attribution purposes only.
=============================================================================*/
/*
 * Copyright (C) 2012 The Android Open Source Project
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

package com.qualcomm.location.izatprovider;

import java.lang.reflect.Field;
import java.util.HashSet;
import java.util.List;

import android.app.AppOpsManager;
import android.content.Context;
import android.content.Intent;
import android.location.Criteria;
import android.location.Location;
import android.location.LocationManager;
import android.location.LocationProvider;
import android.location.LocationRequest;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.WorkSource;
import android.os.RemoteException;
import android.os.WorkSource;
import android.os.UserHandle;
import android.os.ServiceManager;
import android.util.Log;

import com.android.location.provider.LocationProviderBase;
import com.android.location.provider.LocationRequestUnbundled;
import com.android.location.provider.ProviderPropertiesUnbundled;
import com.android.location.provider.ProviderRequestUnbundled;

import com.qualcomm.location.izat.IzatService;
import com.qualcomm.location.LocationService;
import com.qualcomm.location.LBSSystemMonitorService;
import com.qualcomm.location.osagent.OsAgent;

import com.qti.izat.XTProxy;

public class IzatProvider extends LocationProviderBase implements Handler.Callback {
    // member constants
    private static final int MSG_ENABLE_PROVIDER        = 111;
    private static final int MSG_DISABLE_PROVIDER       = 222;
    private static final int MSG_SET_REQUEST            = 333;
    private static final int MSG_LOCATION_CHANGED       = 444;
    private static final int MSG_STATUS_CHANGED         = 555;
    private static final int MSG_DESTROY_ENGINE         = 666;
    private static final int MSG_INIT                   = 777;

    private static final int GNSS_SESSION_BEGIN         = 4;
    private static final int GNSS_SESSION_END           = 5;
    private static final int GNSS_ENGINE_ON             = 6;
    private static final int GNSS_ENGINE_OFF            = 7;

    static final String NLP_TYPE_KEY                    = "networkLocationType";

    // These get set in JNI at the time of class laoding.
    private static int IZAT_STREAM_FUSED;
    private static int IZAT_STREAM_NETWORK;
    private static int IZAT_HORIZONTAL_FINE;
    private static int IZAT_HORIZONTAL_BLOCK;
    private static int IZAT_HORIZONTAL_NONE;

    private static final String XT_PREFS_FILE = "xt";
    private static final String XT_NPP_LOADED = "npp_loaded";

    // This is really like a place holder
    public static IzatProviderContext mProviderContext;

    private final boolean DBG;
    private final String TAG;
    private final String NAME;
    private final boolean mHighPowerCapable;
    private final int mIzatProviderType;
    private Handler mHandler;
    private AppOpsManager mAppOpsMgr;
    private WorkSource mActiveWorkSource;
    private boolean mNavigating;

    public void enable() {
        Log.d(TAG,"location provider enabled");
        mHandler.sendEmptyMessage(MSG_ENABLE_PROVIDER);
    }

    public void disable() {
        Log.d(TAG,"location provider disabled");
        mHandler.sendEmptyMessage(MSG_DISABLE_PROVIDER);
    }

    @Override
    public void onSetRequest(ProviderRequestUnbundled request, WorkSource source) {
        mHandler.obtainMessage(MSG_SET_REQUEST,
                               new RequestParams(request, source)).sendToTarget();
    }

    @Override
    public int onGetStatus(Bundle extras) {
        if (mIzatProviderType == IZAT_STREAM_NETWORK)
            return mProviderContext.mNpProxy.getStatus(extras);
        return LocationProvider.AVAILABLE;
    }

    @Override
    public long onGetStatusUpdateTime() {
        if (mIzatProviderType == IZAT_STREAM_NETWORK)
            return mProviderContext.mNpProxy.getStatusUpdateTime();
        return 0;
    }

    // called from jni
    private void onLocationChanged(long utcTime,
                                   long elapsedRealTimeNanos, double latitude,
                                   double longitude, boolean hasAltitude,
                                   double altitude, boolean hasVerticalUnc,
                                   float verticalUnc, boolean hasSpeed,
                                   float speed,boolean hasSpeedUnc,
                                   float speedUnc, boolean hasBearing,
                                   float bearing, boolean hasBearingUnc,
                                   float bearingUnc, boolean hasAccuracy,
                                   float accuracy, short positionSource) {
        String emptyStr = "";
        log("onLocationChanged - UTC Time: " + utcTime +
            "; Elapsed Real Time Nanos: " + elapsedRealTimeNanos +
            "; Latitude: " + latitude +"; Longitude: " + longitude +
            "; Accuracy: " + (hasAccuracy ? accuracy : emptyStr) +
            "; Altitude: " + (hasAltitude ? altitude : emptyStr) +
            "; Vertical Unc: " + (hasVerticalUnc ? verticalUnc: emptyStr) +
            "; Speed: " + (hasSpeed ? speed : emptyStr) +
            "; Speed Unc " + (hasSpeedUnc ? speedUnc: emptyStr) +
            "; Bearing: " + (hasBearing ? bearing : emptyStr) +
            "; Bearing Unc " + (hasBearingUnc ? bearingUnc: emptyStr) +
            "; positionSource: " + emptyStr +
            "; providerName: " + NAME);

        Location location = new Location(NAME);
        location.setTime(utcTime);
        location.setElapsedRealtimeNanos(elapsedRealTimeNanos);
        location.setLatitude(latitude);
        location.setLongitude(longitude);
        if (hasAltitude) location.setAltitude(altitude);
        if (hasSpeed) location.setSpeed(speed);
        if (hasBearing) location.setBearing(bearing);
        if (hasAccuracy) location.setAccuracy(accuracy);
        if (hasVerticalUnc) location.setVerticalAccuracyMeters(verticalUnc);
        if (hasSpeedUnc) location.setSpeedAccuracyMetersPerSecond(speedUnc);
        if (hasBearingUnc) location.setBearingAccuracyDegrees(bearingUnc);
        location.makeComplete();

        Bundle extras = new Bundle();
        switch (positionSource) {
        case 0:
            extras.putString(NLP_TYPE_KEY, "cell");
            break;
        case 1:
            extras.putString(NLP_TYPE_KEY, "wifi");
            break;
        }
        location.setExtras(extras);

        if (mIzatProviderType == IZAT_STREAM_NETWORK) {
            // Network locations must have a noGPSLocation extra parameter
            location.setExtraLocation(Location.EXTRA_NO_GPS_LOCATION, new Location(location));
        } else {
            // else we need add an extras bundle to this location. It is a little wasting this
            // way because Location makes a copy of the pass in bundle. So this Bundle is to
            // be dereferenced as soon as the call is done.
            location.setExtras(new Bundle());
        }

        mHandler.obtainMessage(MSG_LOCATION_CHANGED, location).sendToTarget();
    }

    // called from jni
    private void onStatusChanged (int status) {
        mHandler.obtainMessage(MSG_STATUS_CHANGED, status).sendToTarget();
    }

    static IzatProvider getNetworkProvider(Context ctx) {
        synchronized(IzatProvider.class) {
            if (mProviderContext == null || mProviderContext.mNetwkProvider == null) {
                IzatProvider tmp = new IzatProvider(
                        ctx, IZAT_STREAM_NETWORK, LocationManager.NETWORK_PROVIDER,
                        ProviderPropertiesUnbundled.create(true, false, true, false, false, false, false,
                                                           Criteria.POWER_LOW, Criteria.ACCURACY_COARSE));
                mProviderContext.mNetwkProvider = tmp;
            }
        }
        return mProviderContext.mNetwkProvider;
    }

    private IzatProvider(Context ctx, int providerType, String providerName,
                         ProviderPropertiesUnbundled properties) {
        super("IzatProvider_"+providerName, properties);
        IzatProviderContext ipc = null;
        TAG = "IzatProvider_"+providerName;
        DBG = Log.isLoggable(TAG, Log.DEBUG);
        NAME = providerName;
        mIzatProviderType = providerType;
        mHighPowerCapable = (mIzatProviderType == IZAT_STREAM_FUSED);
        if (mProviderContext == null) {
            mProviderContext = new IzatProviderContext(ctx.getApplicationContext());
            ipc = mProviderContext;
        }
        mHandler = new Handler(mProviderContext.mLooper, this);
        mHandler.obtainMessage(MSG_INIT, ipc).sendToTarget();
    }

    @Override
    public boolean handleMessage (Message msg) {
        int msgID = msg.what;
        log("handleMessage what - " + msgID);

        switch (msgID) {
        case MSG_INIT:
        {
            mAppOpsMgr = mProviderContext.mAppContext.getSystemService(AppOpsManager.class);
            mActiveWorkSource = new WorkSource();
            mNavigating = false;

            // A workaround for doing the below only once, since mProviderContext
            // being static will only be passed here once.
            if (msg.obj != null) {
                IzatProviderContext ipc = (IzatProviderContext)msg.obj;
                ipc.mNpProxy = new NpProxy(ipc.mAppContext, ipc.mLooper, ipc);
                ipc.mOsAgent = OsAgent.GetInstance(ipc.mAppContext, ipc.mLooper, ipc);

                ipc.mAppContext.startServiceAsUser(
                    new Intent(ipc.mAppContext, LBSSystemMonitorService.class),
                    UserHandle.OWNER);
                ipc.mAppContext.startServiceAsUser(
                    new Intent(ipc.mAppContext, LocationService.class),
                    UserHandle.OWNER);
                ipc.mAppContext.startServiceAsUser(
                    new Intent(ipc.mAppContext, IzatService.class)
                    .setAction("com.qualcomm.location.izat.IzatService"),
                    UserHandle.OWNER);
                ipc.mAppContext.startServiceAsUser(
                    new Intent()
                    .setClassName("com.qualcomm.location.XT",
                                  "com.qualcomm.location.XT.XTSrv")
                    .setAction(Intent.ACTION_LOCKED_BOOT_COMPLETED),
                    UserHandle.CURRENT);

                // Mark QNP_ACTIVE flag to show opt-in app
                try {
                    Context context =
                            ipc.mAppContext.createPackageContext("com.qualcomm.location.XT", 0);
                    XTProxy.setNpProxyLoaded(context);
                } catch (Exception e) {
                    Log.e(TAG, "Exception Saving setting XT_NPP_LOADED: ", e);
                }

                // This is not class load because it requires an instance be created
                // and a msg handling thread is in place, but we really just need to
                // call this once per process'es life cycle. So it is very similar to
                // class load, but it can not be because the calling thread may be a
                // main thread, and the processing in the native woudl involve HIDL
                // handshake, which may be too heavy for main thread. So the static
                // load to as something in between the class load and instance init.
                nativeOnStaticLoad();
            }
            break;
        }
        case MSG_ENABLE_PROVIDER:
        {
            nativeOnEnableProvider(mIzatProviderType);
            break;
        }
        case MSG_DISABLE_PROVIDER:
        {
            nativeOnDisableProvider(mIzatProviderType);
            break;
        }
        case MSG_SET_REQUEST:
        {
            RequestParams params = (RequestParams)msg.obj;
            handleSetRequest(params);
            break;
        }
        case MSG_LOCATION_CHANGED:
        {
            Location location = (Location)msg.obj;
            log("MSG_LOCATION_CHANGED: " + location.getProvider());
            reportLocation(location);
            break;
        }
        case MSG_STATUS_CHANGED:
        {
            int status = (int)msg.obj;
            log("MSG_STATUS_CHANGED: Status: " + status);
            boolean wasNavigating = mNavigating;
            if (status == GNSS_SESSION_BEGIN) {
                mNavigating = true;
            } else if (status ==  GNSS_SESSION_END ||
                       status ==  GNSS_ENGINE_OFF) {
                mNavigating = false;
            }

            if (wasNavigating != mNavigating) {
                // Send an intent to toggle the GPS icon
                updateHighPowerLocationMonitoringOnClientUids(
                        mActiveWorkSource, mNavigating);
                mProviderContext.mAppContext.sendBroadcastAsUser(
                        new Intent(LocationManager.HIGH_POWER_REQUEST_CHANGE_ACTION),
                        UserHandle.ALL);
            }
            break;
        }
        }

        return true;
    }

    WorkSource getWorkSource(){
        return mActiveWorkSource;
    }

    private void handleSetRequest(RequestParams wrapper) {
        log("handleSetRequest");

        try {
            WorkSource[] changes = mActiveWorkSource.setReturningDiffs(wrapper.source);
            if (null != changes && mNavigating && mHighPowerCapable) {
                // start for new work, true for start
                updateHighPowerLocationMonitoringOnClientUids(changes[0], true);
                // finish for gone work, false for stop
                updateHighPowerLocationMonitoringOnClientUids(changes[1], false);
            }

            if (wrapper.request.getReportLocation()) {
                nativeOnAddRequest(mIzatProviderType,
                                   Integer.MAX_VALUE,
                                   wrapper.request.getInterval(),
                                   0,
                                   IZAT_HORIZONTAL_BLOCK);
            } else {
                nativeOnRemoveRequest(mIzatProviderType,
                                      Integer.MAX_VALUE,
                                      wrapper.request.getInterval(),
                                      0,
                                      IZAT_HORIZONTAL_BLOCK);
            }

        } catch (Exception e) {
            Log.w(TAG, "Exception ", e);
        }
    }

    private void updateHighPowerLocationMonitoringOnClientUids(WorkSource newWork, boolean start) {
        if (newWork != null) {
            for (int i=0; i<newWork.size(); i++) {
                int uid = newWork.get(i);
                String packageName = newWork.getName(i);
                if (start) {
                    mAppOpsMgr.startOpNoThrow(AppOpsManager.OP_GPS, uid, packageName);
                } else {
                    mAppOpsMgr.finishOp(AppOpsManager.OP_GPS, uid, packageName);
                }
                log(String.format("%sOp - uid: %d; packageName: %s",
                                  (start ? "start" : "finish"), uid, packageName));
            }
        }
    }

    static int mapAccuracy(int locationRequestAccuracy) {
        switch (locationRequestAccuracy) {
        case LocationRequest.ACCURACY_FINE:
        case LocationRequest.POWER_HIGH:
            return IZAT_HORIZONTAL_FINE;
        case LocationRequest.ACCURACY_BLOCK:
        case LocationRequest.POWER_LOW:
            return IZAT_HORIZONTAL_BLOCK;
        default:
            return IZAT_HORIZONTAL_NONE;
        }
    }

    private void log(String log) {
        if (DBG) Log.d(TAG, log);
    }


    // Nested classes
    public static class IzatProviderContext {
        private IzatProvider mNetwkProvider;
        private final Context mAppContext;
        private final Looper mLooper;
        private OsAgent mOsAgent;
        private NpProxy mNpProxy;

        private boolean mOsNlpOnly = false;
        private boolean mQnpOnly = false;

        private boolean mUserConsent = false;
        private boolean mOsConsent = false;

        private IzatProviderContext(Context app) {
            mAppContext = app;
            HandlerThread t = new HandlerThread(IzatProvider.class.getSimpleName());
            t.start();
            mLooper = t.getLooper();
        }

        public IzatProvider getNetworkProviderInner(Context ctx) {
            return getNetworkProvider(ctx);
        }

        // Called from OsAgent on region change
        public void setRegionalNpRegulate(boolean regulated) {
            mNpProxy.setRegionalNpRegulate(regulated);
        }

        // Let NpProxy parse the settings and feed them back
        public void setNlpConfig(boolean osNlpOnly, boolean qnpOnly) {
            mOsNlpOnly = osNlpOnly;
            mQnpOnly = qnpOnly;
        }

        // Called from NpProxy if OSNP is atached
        public void setOsConsent(boolean consentAccepted) {
            // For QNP only this can't happen (= no GMS loaded)
            if (!mQnpOnly && mOsConsent != consentAccepted) {
                mOsConsent = consentAccepted;
                // For combo, combine OsConsent OR UserConsent
                // Note that mUserConsent is default false, for mOsNlpOnly case will remain so
                getNetworkProviderInner(mAppContext).setEnabled(mOsConsent | mUserConsent);
            }
        }

        // Called from OsAgent Opt-in UI user consent
        public void setUserConsent(boolean consentAccepted) {
            // For OSnp only this can't happen (opt-in not shown)
            if (!mOsNlpOnly && mUserConsent != consentAccepted) {
                mUserConsent = consentAccepted;
                // For combo, combine OsConsent OR UserConsent
                // Note that mOsConsent is default false, for mQnpOnly case will remain so
                getNetworkProviderInner(mAppContext).setEnabled(mOsConsent | mUserConsent);
            }
        }
    }

    private static class RequestParams {
        public ProviderRequestUnbundled request;
        public WorkSource source;
        public RequestParams(ProviderRequestUnbundled request,
                             WorkSource source) {
            this.request = request;
            this.source = source;
        }
    }

    native private static void nativeOnClassLoad();
    native private static void nativeOnStaticLoad();
    native private void nativeOnEnableProvider(int providerType);
    native private void nativeOnDisableProvider(int providerType);
    native private void nativeOnAddRequest(int providerType, int numFixes, long tbf, float displacement, int accuracy);
    native private void nativeOnRemoveRequest(int providerType, int numFixes, long tbf, float displacement, int accuracy);

    static {
        System.loadLibrary("locationservice_jni");
        // This sets below constants based on the enum values in the native
        // IZAT_STREAM_NETWORK / IZAT_STREAM_FUSED
        // IZAT_HORIZONTAL_FINE / IZAT_HORIZONTAL_BLOCK / IZAT_HORIZONTAL_NONE
        nativeOnClassLoad();
    }
}
