/** @file IZatManager.java
*/

/* ======================================================================
*  Copyright (c) 2015 - 2017 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*  ====================================================================*/

package com.qti.location.sdk;

import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.util.Log;
import android.os.ServiceManager;
import android.os.RemoteException;
import android.os.Bundle;
import android.content.pm.ResolveInfo;
import android.location.Location;
import android.app.PendingIntent;

import java.lang.InterruptedException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.HashSet;

import com.qti.izat.IIzatService;
import com.qti.flp.IFlpService;
import com.qti.flp.ILocationCallback;
import com.qti.flp.ISessionStatusCallback;
import com.qti.flp.ITestService;
import com.qti.flp.IMaxPowerAllocatedCallback;
import com.qti.geofence.IGeofenceService;
import com.qti.geofence.IGeofenceCallback;
import com.qti.geofence.GeofenceData;
import com.qti.debugreport.IDebugReportService;
import com.qti.debugreport.IDebugReportCallback;
import com.qti.gnssconfig.IGnssConfigService;
import com.qti.gnssconfig.IGnssConfigCallback;

import com.qti.location.sdk.IZatWiFiDBReceiver.IZatAPInfo;
import com.qti.location.sdk.IZatWiFiDBReceiver.IZatAPInfoExtra;
import com.qti.location.sdk.IZatWiFiDBReceiver.IZatAPLocationData;
import com.qti.location.sdk.IZatWiFiDBReceiver.IZatAPSpecialInfo;
import com.qti.location.sdk.IZatWiFiDBReceiver.IZatWiFiDBReceiverResponseListener;
import com.qti.location.sdk.IZatWiFiDBReceiver.IZatWiFiDBReceiverResponseListenerExt;

import com.qti.location.sdk.IZatWiFiDBProvider.IZatWiFiDBProviderResponseListener;
import com.qti.location.sdk.IZatWiFiDBProvider.IZatAPObsLocData;
import com.qti.location.sdk.IZatWiFiDBProvider.IZatAPScan;

import com.qti.wifidbprovider.APScan;
import com.qti.wifidbreceiver.APInfo;
import com.qti.wifidbreceiver.APInfoExt;
import com.qti.wifidbreceiver.APLocationData;
import com.qti.wifidbreceiver.APSpecialInfo;
import com.qti.wifidbreceiver.IWiFiDBReceiver;
import com.qti.wifidbreceiver.IWiFiDBReceiverResponseListener;

import com.qti.wifidbprovider.APObsLocData;
import com.qti.wifidbprovider.IWiFiDBProvider;
import com.qti.wifidbprovider.IWiFiDBProviderResponseListener;

import com.qti.location.sdk.IZatDBCommon.IZatCellInfo;
import com.qti.location.sdk.IZatDBCommon.IZatAPSSIDInfo;
import com.qti.location.sdk.IZatDBCommon.IZatApBsListStatus;
import com.qti.location.sdk.IZatGnssConfigService.IZatGnssConfigCallback;
import com.qti.location.sdk.IZatGnssConfigService.IzatGnssSvType;
import com.qti.location.sdk.IZatWWANDBReceiver.IZatBSInfo;
import com.qti.location.sdk.IZatWWANDBReceiver.IZatBSLocationData;
import com.qti.location.sdk.IZatWWANDBReceiver.IZatBSLocationDataBase.IZatReliablityTypes;
import com.qti.location.sdk.IZatWWANDBReceiver.IZatBSSpecialInfo;
import com.qti.location.sdk.IZatWWANDBReceiver.IZatWWANDBReceiverResponseListener;
import com.qti.location.sdk.IZatWWANDBReceiver.IZatWWANDBReceiverResponseListenerExt;
import com.qti.location.sdk.IZatWWANDBProvider.IZatWWANDBProviderResponseListener;

import com.qti.location.sdk.IZatDBCommon.IZatCellInfo;
import com.qti.location.sdk.IZatDBCommon.IZatAPSSIDInfo;
import com.qti.location.sdk.IZatDBCommon.IZatApBsListStatus;

import com.qti.wwandbprovider.BSObsLocationData;
import com.qti.wwandbreceiver.BSInfo;
import com.qti.wwandbreceiver.BSLocationData;
import com.qti.wwandbreceiver.BSSpecialInfo;
import com.qti.wwandbreceiver.IWWANDBReceiver;
import com.qti.wwandbreceiver.IWWANDBReceiverResponseListener;

import com.qti.wwandbprovider.IWWANDBProvider;
import com.qti.wwandbprovider.IWWANDBProviderResponseListener;

import com.qti.location.sdk.IZatWWANDBProvider.IZatBSObsLocationData;

import com.qti.location.sdk.utils.IZatDataValidation;
import com.qti.location.sdk.utils.IZatValidationResults.IZatDataTypes;
import com.qti.location.sdk.utils.IZatValidationResults;

import java.io.UnsupportedEncodingException;

/** @addtogroup IZatManager
    @{ */

/**
 * The IZatManager class is the entry point to Qualcomm Location services.
 * It requires the permission of android.permission.ACCESS_FINE_LOCATION.
 *
 * <pre>
 * <code>
 *    // Sample Code
 *
 *    import android.app.Activity;
 *    import android.os.Bundle;
 *    import android.view.View;
 *    import android.widget.Button;
 *    import com.qti.location.sdk.IZatManager;
 *    import com.qti.location.sdk.IZatFlpService;
 *
 *    public class FullscreenActivity extends Activity {
 *
 *       private IZatManager mIzatMgr = null;
 *       private IZatFlpService mFlpSevice = null;
 *
 *       &#64Override
 *       protected void onCreate(Bundle savedInstanceState) {
 *
 *            ...
 *            // get the instance of IZatManager
 *            mIzatMgr = IZatManager.getInstance(getApplicationContext());
 *
 *            ...
 *            final Button connectButton = (Button)findViewById(R.id.connectButton);
 *            connectButton.setOnClickListener(new View.OnClickListener() {
 *                &#64Override
 *                public void onClick(View v) {
 *
 *                    // connect to IZatFlpService through IZatManager
 *                    if (mIzatMgr != null) {
 *                        <b>mFlpSevice = mIzatMgr.connectFlpService();</b>
 *                    }
 *                }
 *            });
 *            ...
 *        }
 *    }
 * </code>
 * </pre>
 */
public class IZatManager {
    private static String TAG = "IZatManager";
    private static final boolean VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);
    private static final String SDK_Version = "7.3.1";

    private static final int UNINITIALIZED_MASK = -1;
    private int mFlpFeaturMasks = UNINITIALIZED_MASK;
    private final int FEATURE_BIT_TIME_BASED_BATCHING_IS_SUPPORTED = 0x1;
    private final int FEATURE_BIT_DISTANCE_BASED_TRACKING_IS_SUPPORTED = 0x2;
    private final int FEATURE_BIT_ADAPTIVE_BATCHING_IS_SUPPORTED = 0x4;
    private final int FEATURE_BIT_DISTANCE_BASED_BATCHING_IS_SUPPORTED = 0x8;
    private final int FEATURE_BIT_OUTDOOR_TRIP_BATCHING_IS_SUPPORTED = 0x10;
    private final int FEATURE_BIT_AGPM_IS_SUPPORTED = 0x20;
    private final int FEATURE_BIT_CONSTELLATION_ENABLEMENT_IS_SUPPORTED = 0x40;

    private Map<IZatFlpServiceImpl.IZatSessionHandlerImpl, FlpRequestMapItem> mFlpRequestsMap
        = createIdMap();
    private Map<IZatFlpServiceImpl.IZatSessionHandlerImpl, FlpRequestMapItem> createIdMap() {
        Map<IZatFlpServiceImpl.IZatSessionHandlerImpl, FlpRequestMapItem> result =
            new HashMap<IZatFlpServiceImpl.IZatSessionHandlerImpl, FlpRequestMapItem>();
        return Collections.synchronizedMap(result);
    }

    private Map<IZatFlpService.IFlpLocationCallback,
                LocationCallbackWrapper> mFlpPassiveCbMap = createPassiveCbMap();
    private Map<IZatFlpService.IFlpLocationCallback,
                LocationCallbackWrapper> createPassiveCbMap() {
        Map<IZatFlpService.IFlpLocationCallback, LocationCallbackWrapper> result =
            new HashMap<IZatFlpService.IFlpLocationCallback, LocationCallbackWrapper>();
        return Collections.synchronizedMap(result);
    }

    private Map<IZatTestService.IFlpMaxPowerAllocatedCallback,
                MaxPowerAllocatedCallbackWrapper> mFlpMaxPowerCbMap = createMaxPowerCbMap();
    private Map<IZatTestService.IFlpMaxPowerAllocatedCallback,
                MaxPowerAllocatedCallbackWrapper> createMaxPowerCbMap() {
        Map<IZatTestService.IFlpMaxPowerAllocatedCallback,
            MaxPowerAllocatedCallbackWrapper> result =
            new HashMap<IZatTestService.IFlpMaxPowerAllocatedCallback,
                        MaxPowerAllocatedCallbackWrapper>();
        return Collections.synchronizedMap(result);
    }

    private Map<IZatGeofenceServiceImpl,
                IZatGeofenceService.IZatGeofenceCallback> mGeofenceClientCallbackMap
        = createGeofenceClientCallbackMap();
    private Map<IZatGeofenceServiceImpl,
                IZatGeofenceService.IZatGeofenceCallback> createGeofenceClientCallbackMap() {
        Map<IZatGeofenceServiceImpl, IZatGeofenceService.IZatGeofenceCallback> result =
            new HashMap<IZatGeofenceServiceImpl, IZatGeofenceService.IZatGeofenceCallback>();
        return Collections.synchronizedMap(result);
    }

    private Map<IZatGeofenceServiceImpl.IZatGeofenceHandleImpl,
                GeofenceMapItem> mGeofencesMap = createGeofencesMap();
    private Map<IZatGeofenceServiceImpl.IZatGeofenceHandleImpl,
                GeofenceMapItem> createGeofencesMap() {
        Map<IZatGeofenceServiceImpl.IZatGeofenceHandleImpl,
            GeofenceMapItem> result =
            new HashMap<IZatGeofenceServiceImpl.IZatGeofenceHandleImpl,
                        GeofenceMapItem>();
        return Collections.synchronizedMap(result);
    }

    private Map<IZatDebugReportingServiceImpl,
                IZatDebugReportingService.IZatDebugReportCallback> mDebugReportClientCallbackMap
        = createDebugReportClientCallbackMap();
    private Map<IZatDebugReportingServiceImpl,
                IZatDebugReportingService.IZatDebugReportCallback>
        createDebugReportClientCallbackMap() {
            Map<IZatDebugReportingServiceImpl,
                IZatDebugReportingService.IZatDebugReportCallback> result =
                new HashMap<IZatDebugReportingServiceImpl,
                            IZatDebugReportingService.IZatDebugReportCallback>();
            return Collections.synchronizedMap(result);
    }

    private Map<IZatGnssConfigServiceImpl,
                IZatGnssConfigService.IZatGnssConfigCallback> mGnssConfigClientCallbackMap
        = createGnssConfigClientCallbackMap();
    private Map<IZatGnssConfigServiceImpl,
                IZatGnssConfigService.IZatGnssConfigCallback>
        createGnssConfigClientCallbackMap() {
            Map<IZatGnssConfigServiceImpl,
                IZatGnssConfigService.IZatGnssConfigCallback> result =
                new HashMap<IZatGnssConfigServiceImpl,
                            IZatGnssConfigService.IZatGnssConfigCallback>();
            return Collections.synchronizedMap(result);
    }

    private IZatWiFiDBReceiverImpl mWiFiDBRecImpl = null;
    private IZatWWANDBReceiverImpl mWWANDBRecImpl = null;
    private IZatWiFiDBProviderImpl mWiFiDBProvImpl = null;
    private IZatWWANDBProviderImpl mWWANDBProvImpl = null;

    private static final int FLP_PASSIVE_LISTENER_HW_ID = -1;
    private static final int FLP_RESULT_SUCCESS = 0;
    private static final int FLP_RESULT_FAILURE = -1;

    private static final int FLP_SEESION_BACKGROUND = 1;
    private static final int FLP_SEESION_FOREGROUND = 2;
    private static final int FLP_SEESION_PASSIVE = 4;

    private static final int GEOFENCE_DWELL_TYPE_INSIDE = 1;
    private static final int GEOFENCE_DWELL_TYPE_OUTSIDE = 2;

    private static volatile int sFlpRequestsCnt = 0;
    private static final Object sFlpServiceLock = new Object();
    private static final Object sTestServiceLock = new Object();
    private static final Object sGeofenceServiceLock = new Object();
    private static final Object sGeofencesMapLock = new Object();
    private static final Object sGeofenceClientCallbackMapLock = new Object();
    private static final Object sDebugReportServiceLock = new Object();
    private static final Object sDebugReportCallbackMapLock = new Object();
    private static final Object sWiFiDBReceiverLock = new Object();
    private static final Object sWiFiDBProviderLock = new Object();
    private static final Object sWWANDBReceiverLock = new Object();
    private static final Object sGnssConfigServiceLock = new Object();
    private static final Object sGnssConfigCallbackMapLock = new Object();
    private static final Object sWWANDBProviderLock = new Object();

    private static final String REMOTE_IZAT_SERVICE_NAME = "com.qualcomm.location.izat.IzatService";
    private Context mContext;
    private static IZatManager sInstance = null;
    private static IIzatService sIzatService = null;
    private GeofenceStatusCallbackWrapper mGeofenceCbWrapper = new GeofenceStatusCallbackWrapper();
    private DebugReportCallbackWrapper mDebugReportCbWrapper = new DebugReportCallbackWrapper();
    private WiFiDBReceiverRespListenerWrapper mWiFiDBReceiverRespListenerWrapper =
            new WiFiDBReceiverRespListenerWrapper();
    private WiFiDBProviderRespListenerWrapper mWiFiDBProviderRespListenerWrapper =
            new WiFiDBProviderRespListenerWrapper();
    private WWANDBReceiverRespListenerWrapper mWWANDBReceiverRespListenerWrapper =
            new WWANDBReceiverRespListenerWrapper();
    private GnssConfigCallbackWrapper mGnssConfigCbWrapper =
            new GnssConfigCallbackWrapper();
    private WWANDBProviderRespListenerWrapper mWWANDBProviderRespListenerWrapper =
            new WWANDBProviderRespListenerWrapper();

    /**
     * Returns an instance of IZatManager.
     *
     * @param context Context object.
     *
     * @throws IZatIllegalArgumentException The context is NULL.
     * @return
     * The instance of IZatManager.
     */
    public static synchronized IZatManager getInstance(Context context)
        throws IZatIllegalArgumentException {
        if (null == context) {
            throw new IZatIllegalArgumentException("null argument");
        }

        if (null == sInstance) {
            sInstance = new IZatManager(context);
        }

        return sInstance;
    }

    private IZatManager(Context context) {
        // To avoid collisions with other process, use thread id + 1 byte for counter
        sFlpRequestsCnt = android.os.Process.myTid() << 8;
        if (VERBOSE) {
            Log.v(TAG, "IZatManager ctor sFlpRequestsCnt=" + sFlpRequestsCnt);
        }
        mContext = context;
    }

    /**
     * Gets the Qualcomm Location SDK version and the IZatService version.
     * <p>
     * This API requires that IZatService is present, otherwise
     * an {@link IZatServiceUnavailableException} is thrown.
     *
     * This returns version information in format X.Y.Z:A.B.C,
     * where XYZ are major, minor, and revision IDs for the SDK
     * and ABC are major, minor, and revision IDs for the service.
     * This API may take as long as several seconds to execute.
     * It is recommended not to call this API in the main thread.
     * </p>
     * @throws IZatServiceUnavailableException  The IZatService is not present.
     * @return
     * The versions of the SDK and the service.
     */
    public String getVersion()
        throws IZatServiceUnavailableException {
        String service_version;

        if (null == sIzatService) {
            connectIzatService();
        }

        try {
            service_version = sIzatService.getVersion();
        } catch (RemoteException e) {
            throw new RuntimeException("Failed to get IzatService version", e);
        }

        if (null == service_version) {
            service_version = "1.0.0";
        }

        String version = SDK_Version + ":" + service_version;

        return version;
    }

    private void setFeatureMask(IFlpService flpService) {
        if (null == flpService) {
            throw new IZatServiceUnavailableException("FLP Service is not available.");
        }
        synchronized (sFlpServiceLock) {
            if (UNINITIALIZED_MASK == mFlpFeaturMasks) {
                try {
                    for (int retries = 0; retries < 10; retries++) {
                        mFlpFeaturMasks = flpService.getAllSupportedFeatures();
                        if (UNINITIALIZED_MASK != mFlpFeaturMasks) {
                            break;
                        }
                        Thread.sleep(200);
                    }
                } catch (InterruptedException ie) {
                } catch (RemoteException e) {
                }
            }
        }
        Log.d(TAG, "got mFlpFeaturMasks" + mFlpFeaturMasks);
    }

    private synchronized void connectIzatService() {
        if (null == sIzatService) {
            if (VERBOSE) {
                Log.d(TAG, "Connecting to Izat service by name [" +
                      REMOTE_IZAT_SERVICE_NAME + "]");
            }

            // check if IZat service installed
            PackageManager pm = mContext.getPackageManager();
            ResolveInfo ri = pm.resolveService(new Intent(REMOTE_IZAT_SERVICE_NAME), 0);
            if (null == ri) {
                Log.e(TAG, "Izat service (" + REMOTE_IZAT_SERVICE_NAME + ") not installed");
                throw new IZatServiceUnavailableException("Izat service unavailable.");
            }

            // check if IZat service started
            if (null == ServiceManager.getService(REMOTE_IZAT_SERVICE_NAME)) {
                Log.e(TAG, "Izat service (" + REMOTE_IZAT_SERVICE_NAME + ") is not started");
                throw new IZatServiceUnavailableException("Izat service not started.");
            }

            sIzatService =
                IIzatService.Stub.asInterface(ServiceManager.getService(REMOTE_IZAT_SERVICE_NAME));
            if(null == sIzatService) {
                Log.e(TAG, "Izat service (" + REMOTE_IZAT_SERVICE_NAME + ") not started");
                throw new IZatServiceUnavailableException("Izat service unavailable.");
            }
        }
    }


    /**
     * Gets an IZatFlpService interface.
     *
     * This API may take several seconds to execute.
     * Qualcomm Technologies, Inc. recommends not to call this API in the main thread.
     *
     * @throws IZatServiceUnavailableException The FLP service is not present.
     *
     * @return
     * An IZatFlpService interface.
     *
     * @dependencies
     * This API requires that the IZat FLP service is present, otherwise
     * an IZatServiceUnavailableException is thrown.
     */
    public IZatFlpService connectFlpService()
        throws IZatServiceUnavailableException {

        if (null == sIzatService) {
            connectIzatService();
        }

        try {
            IFlpService flpService = (IFlpService)(sIzatService.getFlpService());
            if (null == flpService) {
                throw new IZatServiceUnavailableException("FLP Service is not available.");
            }
            setFeatureMask(flpService);

            if ((mFlpFeaturMasks & FEATURE_BIT_DISTANCE_BASED_TRACKING_IS_SUPPORTED)>0) {
                return new IZatFlpServiceImpl(flpService);
            } else {
                Log.e(TAG, "Izat FLP positioning is not supported on this device.");
                return null;
            }
        } catch (RemoteException e) {
            throw new RuntimeException("Failed to get IFlpService", e);
        }
    }

    /**
     * Disconnects form the specified IZatFlpService interface.
     *
     * @param service Handle object that is returned from
     * the IZatManager#connectFlpService().
     * @throws IZatIllegalArgumentException The input service is NULL.
     * @return
     * None.
     */
    public void disconnectFlpService(IZatFlpService service)
        throws IZatIllegalArgumentException {
        if (null == service || !(service instanceof IZatFlpServiceImpl))
            throw new IZatIllegalArgumentException();

        // stop all the undergoing sessions before disconnecting
        try {
            synchronized (sFlpServiceLock) {
                IFlpService flpService =
                   (IFlpService)(sIzatService.getFlpService());
                if (null == flpService) {
                    throw new IZatServiceUnavailableException("FLP Service is not available.");
                }
                FlpRequestMapItem mapItem = null;
                for (IZatFlpServiceImpl.IZatSessionHandlerImpl key : mFlpRequestsMap.keySet()) {
                    mapItem = mFlpRequestsMap.get(key);
                    // stop session
                    if (flpService.stopFlpSession(mapItem.getHwId()) !=
                            FLP_RESULT_SUCCESS) {
                        Log.e(TAG, "stopFlpSession failed in disconnecting");
                        return;
                    }
                    if ((mFlpFeaturMasks & FEATURE_BIT_DISTANCE_BASED_BATCHING_IS_SUPPORTED)>0) {
                        // un-register cb
                        if (NotificationType.NOTIFICATION_WHEN_BUFFER_IS_FULL ==
                                mapItem.getNotifyType()) {
                            flpService.unregisterCallback(FLP_SEESION_BACKGROUND,
                                                          mapItem.getCbWrapper());
                        }
                        if (NotificationType.NOTIFICATION_ON_EVERY_LOCATION_FIX ==
                                mapItem.getNotifyType()) {
                            flpService.unregisterCallback(FLP_SEESION_FOREGROUND,
                                                          mapItem.getCbWrapper());
                        }
                    } else {
                        flpService.unregisterCallback(FLP_SEESION_PASSIVE,
                                                      mapItem.getCbWrapper());
                    }

                    // unregister if any session status handlers are registered
                    if (mapItem.getStatusCallback() != null) {
                        flpService.unregisterForSessionStatus(mapItem.getStatusCallback());
                    }
                }
                mFlpRequestsMap.clear();

                // remove all passive listeners.
                LocationCallbackWrapper cbWrapper = null;
                for (IZatFlpService.IFlpLocationCallback key : mFlpPassiveCbMap.keySet()) {
                    cbWrapper = mFlpPassiveCbMap.get(key);
                    flpService.unregisterCallback(FLP_SEESION_PASSIVE,
                                                  cbWrapper);
                }
                mFlpPassiveCbMap.clear();
            }
        } catch (RemoteException e) {
            throw new RuntimeException("Failed stop all flp sessions", e);
        }
    }

    /**
     * Gets an {@link IZatTestService} interface.
     * <p>
     * This API requires that the IZatTest service is present, otherwise
     * an {@link IZatServiceUnavailableException} is thrown.<br/>
     * This API may take as long as several seconds to execute.
     * It is recommended not to call this API in main thread.
     * Only a system application is allowed to call this API.
     * </p>
     *
     * @throws IZatServiceUnavailableException The Test service is not present.
     * @throws IZatSecurityException The callling application is not a system app.
     * @return An {@link IZatTestService} interface.
     */
    public IZatTestService connectTestService()
        throws IZatServiceUnavailableException, IZatSecurityException {

        if (!mContext.getApplicationInfo().isSystemApp()) {
            throw new IZatSecurityException("Test Service is only available to system app.");
        }

        if (null == sIzatService) {
            connectIzatService();
        }

        try {
            ITestService testService = (ITestService)(sIzatService.getTestService());
            if (null == testService) {
                throw new IZatServiceUnavailableException("Test Service is not available.");
            }
            return new IZatTestServiceImpl(testService);
        } catch (RemoteException e) {
            throw new RuntimeException("Failed to get ITestService", e);
        }
    }

    /**
     * Disconnects from the specified {@link IZatTestService} interface.
     *
     * @param service Handle object that is returned from
     * {@link IZatManager#connectTestService()}.
     * @throws IZatIllegalArgumentException The input service is NULL.
     */
    public void disconnectTestService(IZatTestService service)
        throws IZatIllegalArgumentException {
        if (null == service || !(service instanceof IZatTestServiceImpl))
            throw new IZatIllegalArgumentException();

        synchronized (sTestServiceLock) {
            for (IZatTestService.IFlpMaxPowerAllocatedCallback key :
                     mFlpMaxPowerCbMap.keySet()) {
                service.deregisterForMaxPowerAllocatedChange(key);
            }
            mFlpMaxPowerCbMap.clear();
        }
    }

    /**
     * Gets an {@link IZatGeofenceService} interface.
     * <p>
     * This API requires that the Geofence service is present, otherwise
     * an {@link IZatServiceUnavailableException} is thrown.<br/>
     * This API may take as long as several seconds to execute.
     * It is recommended not to call this API in the main thread.
     * </p>
     *
     * @throws IZatServiceUnavailableException The Geofence service is not present.
     * @return An {@link IZatGeofenceService} interface.
     */
    public IZatGeofenceService connectGeofenceService()
        throws IZatServiceUnavailableException {
        if (null == sIzatService) {
            connectIzatService();
        }

        try {
            IGeofenceService geofenceService =
                (IGeofenceService)(sIzatService.getGeofenceService());
            if (null == geofenceService) {
                throw new IZatServiceUnavailableException("Geofence Service is not available.");
            }
            // register the geofence callback
            synchronized (sGeofenceServiceLock) {
                geofenceService.registerCallback(mGeofenceCbWrapper);
            }
            return new IZatGeofenceServiceImpl(geofenceService);
        } catch (RemoteException e) {
            throw new RuntimeException("Failed to get IGeofenceService", e);
        }
    }

   /**
     * Disconnects from the specified {@link IZatGeofenceService} interface.
     *
     * @param service Handle object that is returned from
     * {@link IZatManager#connectGeofenceService()}.
     * @throws IZatIllegalArgumentException The input service is NULL.
     */
    public void disconnectGeofenceService(IZatGeofenceService service)
        throws IZatIllegalArgumentException {
        if (null == service || !(service instanceof IZatGeofenceServiceImpl))
            throw new IZatIllegalArgumentException();

        try {
            synchronized (sGeofenceServiceLock) {
                IGeofenceService geofenceService =
                   (IGeofenceService)(sIzatService.getGeofenceService());
                if (null == geofenceService) {
                    throw new IZatServiceUnavailableException("Geofence Service is not available.");
                }
                // remove all geofence added.
                GeofenceMapItem mapItem = null;
                for (IZatGeofenceServiceImpl.IZatGeofenceHandleImpl key :
                         mGeofencesMap.keySet()) {
                    mapItem = mGeofencesMap.get(key);
                    geofenceService.removeGeofence(mapItem.getHWGeofenceId());
                }
                // un-register the geofence callback
                geofenceService.unregisterCallback(mGeofenceCbWrapper);
            }
            synchronized (sGeofencesMapLock) {
                mGeofencesMap.clear();
            }
            // delete the client callback
            synchronized (sGeofenceClientCallbackMapLock) {
                for (IZatGeofenceServiceImpl key : mGeofenceClientCallbackMap.keySet()) {
                    if (service == key) {
                        mGeofenceClientCallbackMap.remove(key);
                        break;
                    }
                }
            }
        } catch (RemoteException e) {
            throw new RuntimeException("Failed to remove all geofence added", e);
        }
    }

   /**
    * Gets an {@link IZatDebugReportingService} interface.
    * <p>
    * This API requires that the IZat debug reporting service is present, otherwise
    * an {@link IZatServiceUnavailableException} is thrown.<br/>
    * This API may take as long as several seconds to execute.
    * It is recommended not to call this API in the main thread.
    * </p>
    *
    * @throws IZatServiceUnavailableException The Debug Reporting Service is not present.
    * @return An {@link IZatDebugReportingService} interface.
    */
   public IZatDebugReportingService connectDebugReportingService()
        throws IZatServiceUnavailableException {
        if (null == sIzatService) {
            connectIzatService();
        }

        try {
            IDebugReportService debugReportService =
                (IDebugReportService)(sIzatService.getDebugReportService());
            if (null == debugReportService) {
                throw new IZatServiceUnavailableException("Debug Reporting Service is not available.");
            }
            synchronized (sDebugReportServiceLock) {
                debugReportService.registerForDebugReporting(mDebugReportCbWrapper);
            }
            return new IZatDebugReportingServiceImpl(debugReportService);
        } catch (RemoteException e) {
            throw new RuntimeException("Failed to get IDebugReportService", e);
        }
   }

   /**
     * Disconnects from the specified {@link IZatDebugReportingService} interface.
     *
     * @param service Handle object that is returned from
     * {@link IZatManager#connectDebugReportingService()}.
     * @throws IZatIllegalArgumentException The input service is NULL.
     */
   public void disconnectDebugReportingService(IZatDebugReportingService service) {
       if (null == service || !(service instanceof IZatDebugReportingServiceImpl))
            throw new IZatIllegalArgumentException();

       try {
           IDebugReportService debugReportService =
               (IDebugReportService)(sIzatService.getDebugReportService());
            if (null == debugReportService) {
                throw new IZatServiceUnavailableException("Debug Report Service is not available.");
            }
            synchronized (sDebugReportServiceLock) {
               debugReportService.unregisterForDebugReporting(mDebugReportCbWrapper);
            }
            synchronized (sDebugReportCallbackMapLock) {
                for(IZatDebugReportingServiceImpl key :
                        mDebugReportClientCallbackMap.keySet()) {
                    if (service == key) {
                        mDebugReportClientCallbackMap.remove(key);
                        break;
                    }
                }
            }
        } catch (RemoteException e) {
            throw new RuntimeException("Failed to disconnect DebugReportService", e);
        }
   }


    /**
     * Gets an {@link IZatWiFiDBReceiver} interface.
     * <p>
     * This API requires that the Wi-Fi DB receiver is present, otherwise
     * an {@link IZatServiceUnavailableException} is thrown.<br/>
     * This API may take as long as several seconds to execute.
     * It is recommended not to call this API in the main thread.
     * </p>
     *
     * @param listener Handle object that is returned from the {@link IZatWiFiDBReceiver} service.
     *                 Can not be null, or a IZatIllegalArgumentException will be thrown.
     * @param WiFiReceiverIntent Pending intent to handle onServiceRequest() calls from SDK.
     *                 Can not be null, or a IZatIllegalArgumentException will be thrown.
     * @throws IZatServiceUnavailableException The Wi-Fi DB
     *                                         receiver is not
     *                                         present.
     * @throws IZatIllegalArgumentException    One of the arguments is null.
     * @throws IZatSecurityException           The SDK client is not a system App.
     * @return an {@link IZatWiFiDBReceiver} interface.
     */
    @Deprecated
    public IZatWiFiDBReceiver connectToWiFiDBReceiver(
            IZatWiFiDBReceiverResponseListener listener)
            throws IZatServiceUnavailableException {
        if (null == sIzatService) {
            connectIzatService();
        }

        if (null == listener) {
            throw new IZatIllegalArgumentException("Listener can not be null.");
        }

        try {
            if(null == mWiFiDBRecImpl) {
                IWiFiDBReceiver wifiDBReceiver =
                        (IWiFiDBReceiver)(sIzatService.getWiFiDBReceiver());
                // register the callbacks
                synchronized (sWiFiDBReceiverLock) {
                    wifiDBReceiver.registerResponseListener(mWiFiDBReceiverRespListenerWrapper);
                }
                mWiFiDBRecImpl = new IZatWiFiDBReceiverImpl(wifiDBReceiver, listener);
            }
            return mWiFiDBRecImpl;

        } catch (RemoteException e) {
            throw new RuntimeException("Failed to get IWiFiDBReceiver", e);
        }
    }

    /**
     * Gets an {@link IZatWiFiDBReceiver} interface.
     * <p>
     * This API requires that the Wi-Fi DB receiver is present, otherwise
     * an {@link IZatServiceUnavailableException} is thrown.<br/>
     * This API may take as long as several seconds to execute.
     * It is recommended not to call this API in the main thread.
     * </p>
     *
     * @param listener Handle object that is returned from the {@link IZatWiFiDBReceiver} service.
     *                 Can not be null, or a IZatIllegalArgumentException will be thrown.
     * @param WiFiReceiverIntent Pending intent to handle onServiceRequest() calls from SDK.
     *                 Can not be null, or a IZatIllegalArgumentException will be thrown.
     * @throws IZatServiceUnavailableException The Wi-Fi DB
     *                                         receiver is not
     *                                         present.
     * @throws IZatIllegalArgumentException    One of the arguments is null.
     * @throws IZatSecurityException           The SDK client is not a system App.
     * @return an {@link IZatWiFiDBReceiver} interface.
     */
    public IZatWiFiDBReceiver connectToWiFiDBReceiver(
                IZatWiFiDBReceiverResponseListenerExt listener, PendingIntent WiFiReceiverIntent)
                throws IZatServiceUnavailableException, IZatIllegalArgumentException,
                IZatSecurityException {
        if (!mContext.getApplicationInfo().isSystemApp()) {
            throw new IZatSecurityException("WiFiDBReceiver is only available to system apps.");
        }

        if (null == sIzatService) {
            connectIzatService();
        }

        if (null == listener) {
            throw new IZatIllegalArgumentException("Listener can not be null.");
        }

        if (null == WiFiReceiverIntent) {
            throw new IZatIllegalArgumentException("Pending intent can not be null.");
        }

        try {
            if(null == mWiFiDBRecImpl) {
                IWiFiDBReceiver wifiDBReceiver =
                    (IWiFiDBReceiver)(sIzatService.getWiFiDBReceiver());
                // register the callbacks
                synchronized (sWiFiDBReceiverLock) {
                    wifiDBReceiver.registerResponseListenerExt(mWiFiDBReceiverRespListenerWrapper,
                            WiFiReceiverIntent);
                }
                mWiFiDBRecImpl = new IZatWiFiDBReceiverImpl(wifiDBReceiver, listener);
            }
            return mWiFiDBRecImpl;

        } catch (RemoteException e) {
            throw new RuntimeException("Failed to get IWiFiDBReceiver", e);
        }
    }

    /**
     * Disconnects the specified {@link IZatWiFiDBReceiver}
     * interface.
     *
     * @param receiver Handle object that is returned from
     * {@link IZatManager#connectToWiFiDBReceiver()}.
     * @throws IZatIllegalArgumentException The input receiver is NULL.
     * @throws IZatSecurityException           The SDK client is not a system App.
     *
     */
    public void disconnectFromWiFiDBReceiver(IZatWiFiDBReceiver receiver)
        throws IZatIllegalArgumentException, IZatSecurityException {
        if (!mContext.getApplicationInfo().isSystemApp()) {
            throw new IZatSecurityException("WWANDBProvider is only available to system app.");
        }
        if (null == receiver || !(receiver instanceof IZatWiFiDBReceiverImpl))
            throw new IZatIllegalArgumentException();
        try {
            IWiFiDBReceiver wifiDBReceiver =
                    (IWiFiDBReceiver)(sIzatService.getWiFiDBReceiver());
            synchronized (sWiFiDBReceiverLock) {
                wifiDBReceiver.removeResponseListener(mWiFiDBReceiverRespListenerWrapper);
            }
            mWiFiDBRecImpl = null;
        } catch (RemoteException e) {
            throw new RuntimeException("Failed to disconnect WiFiDBReceiver", e);
        }
    }

    /**
     * Gets an {@link IZatWWANDBReceiver} interface.
     * <p>
     * This API requires that WWAN DB Receiver is present, otherwise
     * an {@link IZatServiceUnavailableException}
     * is thrown.<br/>
     *
     * This API may take as long as several seconds to execute.
     * It is recommended not to call this API in the main thread.
     * </p>
     *
     * @param listener Handle object that is returned from
     *                     the {@link IZatWWANDBReceiver} service.
     *                 Can not be null, or a IZatIllegalArgumentException will be thrown.
     * @throws IZatServiceUnavailableException The WWAN DB
     *                                         Receiver is not
     *                                         present.
     * @return an {@link IZatWWANDBReceiver} interface.
     */
    public IZatWWANDBReceiver connectToWWANDBReceiver(
            IZatWWANDBReceiverResponseListener listener)
            throws IZatServiceUnavailableException{
        if (sIzatService == null) {
            connectIzatService();
        }

        try {
            if(null == mWWANDBRecImpl) {
                IWWANDBReceiver wwanDBReceiver =
                        (IWWANDBReceiver)(sIzatService.getWWANDBReceiver());
                // register the callbacks
                synchronized (sWWANDBReceiverLock) {
                    wwanDBReceiver.registerResponseListener(mWWANDBReceiverRespListenerWrapper);
                }
                mWWANDBRecImpl = new IZatWWANDBReceiverImpl(wwanDBReceiver, listener);
            }
            return mWWANDBRecImpl;

        } catch (RemoteException e) {
            throw new RuntimeException("Failed to get IWWANDBReceiver", e);
        }
    }

    /**
     * Gets an {@link IZatWWANDBReceiver} interface.
     * <p>
     * This API requires that WWAN DB Receiver is present, otherwise
     * an {@link IZatServiceUnavailableException}
     * is thrown.<br/>
     *
     * This API may take as long as several seconds to execute.
     * It is recommended not to call this API in the main thread.
     * </p>
     *
     * @param listener Handle object that is returned from
     *                     the {@link IZatWWANDBReceiver} service.
     *                 Can not be null, or a IZatIllegalArgumentException will be thrown.
     * @param WWANReceiverIntent Pending intent to handle onServiceRequest() calls from SDK.
     *                 Can not be null, or a IZatIllegalArgumentException will be thrown.
     * @throws IZatServiceUnavailableException The WWAN DB
     *                                         Receiver is not
     *                                         present.
     * @throws IZatIllegalArgumentException    One of the arguments is null.
     * @throws IZatSecurityException           The SDK client is not a system App.
     * @return an {@link IZatWWANDBReceiver} interface.
     */
    public IZatWWANDBReceiver connectToWWANDBReceiver(
            IZatWWANDBReceiverResponseListenerExt listener, PendingIntent WWANReceiverIntent)
            throws IZatServiceUnavailableException, IZatIllegalArgumentException,
            IZatServiceUnavailableException{

        if (!mContext.getApplicationInfo().isSystemApp()) {
            throw new IZatSecurityException("WWANDBReceiver is only available to system app.");
        }

        if (sIzatService == null) {
            connectIzatService();
        }

        if (null == listener) {
            throw new IZatIllegalArgumentException("Listener can not be null.");
        }

        if (null == WWANReceiverIntent) {
            throw new IZatIllegalArgumentException("Pending intent can not be null.");
        }

        try {
            if(null == mWWANDBRecImpl) {
                IWWANDBReceiver wwanDBReceiver =
                        (IWWANDBReceiver)(sIzatService.getWWANDBReceiver());
                // register the callbacks
                synchronized (sWWANDBReceiverLock) {
                    wwanDBReceiver.registerResponseListenerExt(mWWANDBReceiverRespListenerWrapper,
                                                               WWANReceiverIntent);
                }
                mWWANDBRecImpl = new IZatWWANDBReceiverImpl(wwanDBReceiver, listener);
            }
            return mWWANDBRecImpl;

        } catch (RemoteException e) {
            throw new RuntimeException("Failed to get IWWANDBReceiver", e);
        }
    }

    /**
     * Disconnects the specified {@link IZatWWANDBReceiver}
     * interface.
     *
     * @param receiver Handle object that is returned from
     * {@link IZatManager#connectToWWANDBReceiver()}.
     *
     * @throws IZatIllegalArgumentException The input receiver is NULL.
     * @throws IZatSecurityException        The SDK client is not a system App.
     */
    public void disconnectFromWWANDBReceiver(IZatWWANDBReceiver receiver)
            throws IZatIllegalArgumentException, IZatSecurityException {
        if (!mContext.getApplicationInfo().isSystemApp()) {
            throw new IZatSecurityException("WWANDBProvider is only available to system app.");
        }
        if (receiver == null || !(receiver instanceof IZatWWANDBReceiverImpl)){
                throw new IZatIllegalArgumentException();
        }
        try {
            IWWANDBReceiver wwanDBReceiver =
                    (IWWANDBReceiver)(sIzatService.getWWANDBReceiver());
            synchronized (sWWANDBReceiverLock) {
                wwanDBReceiver.removeResponseListener(mWWANDBReceiverRespListenerWrapper);
            }
            mWWANDBRecImpl = null;
        } catch (RemoteException e) {
            throw new RuntimeException("Failed to disconnect WWANDBReceiver", e);
        }
    }

    /**
     * Gets an {@link IZatWiFiDBProvider} interface.
     * <p>
     * This API requires that the Wi-Fi DB provider is present, otherwise
     * an {@link IZatServiceUnavailableException} is thrown.<br/>
     * This API may take as long as several seconds to execute.
     * It is recommended not to call this API in the main thread.
     * </p>
     *
     * @param listener Handle object that is returned from the {@link IZatWiFiDBProvider} service.
     *                 Can not be null, or a IZatIllegalArgumentException will be thrown.
     * @param WiFiProviderIntent Pending intent to handle onServiceRequest() calls from SDK.
     *                 Can not be null, or a IZatIllegalArgumentException will be thrown.
     * @throws IZatServiceUnavailableException The Wi-Fi DB
     *                                         provider is not
     *                                         present.
     * @throws IZatIllegalArgumentException    One of the arguments is null.
     * @throws IZatSecurityException           The SDK client is not a system App.
     *
     * @return an {@link IZatWiFiDBProvider} interface.
     */
    public IZatWiFiDBProvider connectToWiFiDBProvider(
            IZatWiFiDBProviderResponseListener listener, PendingIntent WiFiProviderIntent)
            throws IZatServiceUnavailableException, IZatIllegalArgumentException,
            IZatSecurityException {
        if (!mContext.getApplicationInfo().isSystemApp()) {
            throw new IZatSecurityException("WiFiDBProvider is only available to system apps.");
        }

        if (null == sIzatService) {
            connectIzatService();
        }
        if (null == listener) {
            throw new IZatIllegalArgumentException("Listener can not be null.");
        }

        if (null == WiFiProviderIntent) {
            throw new IZatIllegalArgumentException("Pending intent can not be null.");
        }

        try {
            if(null == mWiFiDBProvImpl) {
                IWiFiDBProvider wifiDBProvider =
                        (IWiFiDBProvider)(sIzatService.getWiFiDBProvider());
                // register the callbacks
                synchronized (sWiFiDBProviderLock) {
                    wifiDBProvider.registerResponseListener(mWiFiDBProviderRespListenerWrapper,
                            WiFiProviderIntent);
                }
                mWiFiDBProvImpl = new IZatWiFiDBProviderImpl(wifiDBProvider, listener);
            }
            return mWiFiDBProvImpl;

        } catch (RemoteException e) {
            throw new RuntimeException("Failed to get IWiFiDBReceiver", e);
        }
    }

    /**
     * Disconnects the specified {@link IZatWiFiDBProvider}
     * interface.
     *
     * @param provider Handle object that is returned from
     * {@link IZatManager#connectToWiFiDBProvider()}.
     * @throws IZatIllegalArgumentException The input provider is NULL.
     * @throws IZatSecurityException           The SDK client is not a system App.
     *
     */
    public void disconnectFromWiFiDBProvider(IZatWiFiDBProvider provider)
            throws IZatIllegalArgumentException, IZatSecurityException {
        if (!mContext.getApplicationInfo().isSystemApp()) {
            throw new IZatSecurityException("WWANDBProvider is only available to system app.");
        }
        if (null == provider || !(provider instanceof IZatWiFiDBProviderImpl))
            throw new IZatIllegalArgumentException();
        try {
            IWiFiDBProvider wifiDBProvider =
                    (IWiFiDBProvider)(sIzatService.getWiFiDBProvider());
            synchronized (sWiFiDBReceiverLock) {
                wifiDBProvider.removeResponseListener(mWiFiDBProviderRespListenerWrapper);
            }
            mWiFiDBProvImpl = null;
        } catch (RemoteException e) {
            throw new RuntimeException("Failed to disconnect WifiDBProvider", e);
        }
    }

    /**
     * Gets an {@link IZatWWANDBProvider} interface.
     * <p>
     * This API requires that WWAN DB Provider is present, otherwise
     * an {@link IZatServiceUnavailableException}
     * is thrown.<br/>
     *
     * This API may take as long as several seconds to execute.
     * It is recommended not to call this API in the main thread.
     * </p>
     *
     * @param listener Handle object that is returned from
     *                 the {@link IZatWWANDBProvider} service.
     *                 Can not be null, or a IZatIllegalArgumentException will be thrown.
     * @param WWANProviderIntent Pending intent to handle onServiceRequest() calls from SDK.
     *                 Can not be null, or a IZatIllegalArgumentException will be thrown.
     * @throws IZatServiceUnavailableException The WWAN DB
     *                                         Provider is not
     *                                         present.
     * @throws IZatIllegalArgumentException    One of the arguments is null.
     * @throws IZatSecurityException           The SDK client is not a system App.
     * @return an {@link IZatWWANDBProvider} interface.
     */
    public IZatWWANDBProvider connectToWWANDBProvider(
            IZatWWANDBProviderResponseListener listener, PendingIntent WWANProviderIntent)
            throws IZatServiceUnavailableException, IZatIllegalArgumentException,
            IZatServiceUnavailableException{

        if (!mContext.getApplicationInfo().isSystemApp()) {
            throw new IZatSecurityException("WWANDBProvider is only available to system app.");
        }

        if (sIzatService == null) {
            connectIzatService();
        }

        if (null == listener) {
            throw new IZatIllegalArgumentException("Listener can not be null.");
        }

        if (null == WWANProviderIntent) {
            throw new IZatIllegalArgumentException("Pending intent can not be null.");
        }

        try {
            if(null == mWWANDBProvImpl) {
                IWWANDBProvider wwanDBProvider =
                        (IWWANDBProvider)(sIzatService.getWWANDBProvider());
                // register the callbacks
                synchronized (sWWANDBProviderLock) {
                    wwanDBProvider.registerResponseListener(mWWANDBProviderRespListenerWrapper,
                            WWANProviderIntent);
                }
                mWWANDBProvImpl = new IZatWWANDBProviderImpl(wwanDBProvider, listener);
            }
            return mWWANDBProvImpl;

        } catch (RemoteException e) {
            throw new RuntimeException("Failed to get IWWANDBReceiver", e);
        }
    }

    /**
     * Disconnects the specified {@link IZatWWANDBProvider}
     * interface.
     *
     * @param provider Handle object that is returned from
     * {@link IZatManager#connectToWWANDBProvider()}.
     *
     * @throws IZatIllegalArgumentException The input provider is NULL.
     * @throws IZatSecurityException        The SDK client is not a system App.
     */
    public void disconnectFromWWANDBProvider(IZatWWANDBProvider provider)
            throws IZatIllegalArgumentException, IZatSecurityException {
        if (!mContext.getApplicationInfo().isSystemApp()) {
            throw new IZatSecurityException("WWANDBProvider is only available to system app.");
        }
        if (null == provider || !(provider instanceof IZatWWANDBProviderImpl))
            throw new IZatIllegalArgumentException();
        try {
            IWWANDBProvider wwanDBProvider =
                    (IWWANDBProvider)(sIzatService.getWWANDBProvider());
            synchronized (sWiFiDBReceiverLock) {
                wwanDBProvider.removeResponseListener(mWWANDBProviderRespListenerWrapper);
            }
            mWWANDBProvImpl = null;
        } catch (RemoteException e) {
            throw new RuntimeException("Failed to disconnect WWANDBProvider", e);
        }
    }

    /**
     * Gets an {@link IZatGnssConfigService} interface.
     * <p>
     * This API requires that the IZat Gnss Config service is present, otherwise
     * an {@link IZatServiceUnavailableException} is thrown.<br/>
     * This API may take as long as several seconds to execute.
     * It is recommended not to call this API in the main thread.
     * </p>
     *
     * @throws IZatServiceUnavailableException The Gnss Config Service is not present.
     * @return An {@link IZatGnssConfigService} interface.
     */
    public IZatGnssConfigService connectGnssConfigService()
         throws IZatServiceUnavailableException {
         if (null == sIzatService) {
             connectIzatService();
         }

         try {
             setFeatureMask((IFlpService)(sIzatService.getFlpService()));
             IGnssConfigService gnssConfigService =
                 (IGnssConfigService)(sIzatService.getGnssConfigService());
             if (null == gnssConfigService) {
                 throw new IZatServiceUnavailableException("Gnss Config Service is not available.");
             }
             if ((mFlpFeaturMasks & FEATURE_BIT_CONSTELLATION_ENABLEMENT_IS_SUPPORTED) > 0) {
                 return new IZatGnssConfigServiceImpl(gnssConfigService);
             } else {
                 Log.e(TAG, "Izat Gnss SV constellation config is not supported on this device.");
                 return null;
             }
         } catch (RemoteException e) {
             throw new RuntimeException("Failed to get IGnssConfigService", e);
         }
    }

    /**
      * Disconnects from the specified {@link IZatGnssConfigService} interface.
      *
      * @param service Handle object that is returned from
      * {@link IZatManager#connectGnssConfigService()}.
      * @throws IZatIllegalArgumentException The input service is NULL.
      */
    public void disconnectGnssConfigService(IZatGnssConfigService service) {
        if (null == service || !(service instanceof IZatGnssConfigServiceImpl))
             throw new IZatIllegalArgumentException();

        try {
            IGnssConfigService gnssConfigService =
                (IGnssConfigService)(sIzatService.getGnssConfigService());
             if (null == gnssConfigService) {
                 throw new IZatServiceUnavailableException("Gnss Config Service is not available.");
             }
             synchronized (sGnssConfigCallbackMapLock) {
                 for(IZatGnssConfigServiceImpl key :
                         mGnssConfigClientCallbackMap.keySet()) {
                     if (service == key) {
                         mGnssConfigClientCallbackMap.remove(key);
                         break;
                     }
                 }
             }
         } catch (RemoteException e) {
             throw new RuntimeException("Failed to disconnect GnssConfigService", e);
         }
    }

/** @cond */

    /*
     * Implementation of interface IZatFlpService. (FOR INTERNAL USE ONLY)
     *
     * @hide
     */
    private class IZatFlpServiceImpl implements IZatFlpService {

        IFlpService mService;
        public IZatFlpServiceImpl(IFlpService service) {
            mService = service;
        }

        @Override
        public IZatFlpSessionHandle startFlpSession(IFlpLocationCallback callback,
                                                    IzatFlpRequest flpRequest)
                                                    throws IZatIllegalArgumentException {
            if (null == callback || null == flpRequest) {
                throw new IZatIllegalArgumentException("invalid input parameter");
            }
            if ((flpRequest.getTimeInterval() <= 0) &&
                (flpRequest.getDistanceInterval() <= 0) &&
                (flpRequest.getTripDistance() <= 0)) {
                throw new IZatIllegalArgumentException("Atleast one of the parameters " +
                        "in time, distance interval and trip distance must be valid");
            }
            IZatDataValidation.dataValidate(flpRequest);
            try {
                synchronized (sFlpServiceLock) {
                    // first to check if this request is already started.
                    NotificationType notifType =
                            NotificationType.NOTIFICATION_ON_EVERY_LOCATION_FIX;
                    if (flpRequest.mIsRunningInBackground) {

                        notifType = NotificationType.NOTIFICATION_WHEN_BUFFER_IS_FULL;

                        if (flpRequest instanceof IzatFlpBgRequest) {
                            // get the active mode
                            IzatFlpBgRequest.IzatFlpBgRequestMode activeMode =
                                    ((IzatFlpBgRequest)flpRequest).getActiveMode();

                            if (IzatFlpBgRequest.IzatFlpBgRequestMode.TRIP_MODE == activeMode) {
                                if (0 == (mFlpFeaturMasks &
                                          FEATURE_BIT_OUTDOOR_TRIP_BATCHING_IS_SUPPORTED)) {
                                    throw new IZatFeatureNotSupportedException (
                                            "Outdoor trip mode is not supported.");
                                } else if (flpRequest.getTripDistance() > 0) {
                                    notifType =
                                            NotificationType.NOTIFICATION_WHEN_TRIP_IS_COMPLETED;
                                }
                            }
                        }
                    }

                    for (FlpRequestMapItem mapItem: mFlpRequestsMap.values()) {
                        if ((mapItem.getCallback() == callback) &&
                            (mapItem.getNotifyType() == notifType) &&
                            (mapItem.getTimeInterval() == flpRequest.getTimeInterval()) &&
                            (mapItem.getDistanceInterval() == flpRequest.getDistanceInterval()) &&
                                    (mapItem.getTripDistance() == flpRequest.getTripDistance())){
                            throw new IZatIllegalArgumentException("this session" +
                                                                   " started already.");
                        }
                    }
                    int hwId = sFlpRequestsCnt++;
                    long sessionStartTime = System.currentTimeMillis();
                    // register the cb
                    LocationCallbackWrapper cbWrapper = new LocationCallbackWrapper(callback);

                    if ((mFlpFeaturMasks & FEATURE_BIT_DISTANCE_BASED_BATCHING_IS_SUPPORTED)>0) {
                        /* If the modem support LB2.0, then the batched fixes and live fix will be
                          returned independently to clients based on the request issued by
                          each client. */
                        if (flpRequest.mIsRunningInBackground) {
                            mService.registerCallback(FLP_SEESION_BACKGROUND,
                                                      hwId,
                                                      cbWrapper,
                                                      sessionStartTime);
                        } else {
                            mService.registerCallback(FLP_SEESION_FOREGROUND,
                                                      hwId,
                                                      cbWrapper,
                                                      sessionStartTime);
                        }
                    } else {
                            /* If the modem does not support LB2.0, then the batching client
                               will also receive the live fix, which is the legacy behaviour.*/
                            mService.registerCallback(FLP_SEESION_PASSIVE,
                                                      hwId,
                                                      cbWrapper,
                                                      sessionStartTime);
                    }
                    // start flp session
                    int result = mService.startFlpSessionWithPower(hwId,
                                    notifType.getCode(),
                                    flpRequest.getTimeInterval(),
                                    flpRequest.getDistanceInterval(),
                                    flpRequest.getTripDistance(),
                                    flpRequest.getPowerMode(),
                                    flpRequest.getTbmMillis());
                    if (VERBOSE) {
                        Log.v(TAG, "startFlpSession() returning : " + result);
                    }
                    if (FLP_RESULT_SUCCESS == result) {
                        IZatSessionHandlerImpl handle = new IZatSessionHandlerImpl();
                        mFlpRequestsMap.put(handle,
                                new FlpRequestMapItem(callback,
                                                      notifType,
                                                      flpRequest.getTimeInterval(),
                                                      flpRequest.getDistanceInterval(),
                                                      flpRequest.getTripDistance(),
                                                      cbWrapper,
                                                      hwId,
                                                      sessionStartTime,
                                                      flpRequest.getPowerMode(),
                                                      flpRequest.getTbmMillis()));
                        return handle;
                    } else {
                        if (0 < (mFlpFeaturMasks &
                                 FEATURE_BIT_DISTANCE_BASED_BATCHING_IS_SUPPORTED)) {
                            if (flpRequest.mIsRunningInBackground) {
                                mService.unregisterCallback(FLP_SEESION_BACKGROUND,
                                                            cbWrapper);
                            } else {
                                mService.unregisterCallback(FLP_SEESION_FOREGROUND,
                                                            cbWrapper);
                            }
                        } else {
                                mService.unregisterCallback(FLP_SEESION_PASSIVE,
                                                            cbWrapper);
                        }
                        sFlpRequestsCnt--;
                        return null;
                    }
                }
            } catch (RemoteException e) {
                throw new RuntimeException("Failed startFlpSession", e);
            }
        }

        @Override
        public void stopFlpSession(IZatFlpSessionHandle handler)
            throws IZatIllegalArgumentException {

            if (null == handler || !(handler instanceof IZatFlpServiceImpl.IZatSessionHandlerImpl)) {
                throw new IZatIllegalArgumentException("invalid input parameter");
            }
            try {
                synchronized (sFlpServiceLock) {
                    // first to check if this request is already started.
                    FlpRequestMapItem mapItem = mFlpRequestsMap.get(handler);
                    if (null == mapItem) {
                        Log.e(TAG, "this request ID is unknown.");
                        return;
                    }
                    if (mService.stopFlpSession(mapItem.getHwId()) != FLP_RESULT_SUCCESS){
                        Log.e(TAG, "stopFlpSession() failed. ");
                        return;
                    }
                    if ((mFlpFeaturMasks & FEATURE_BIT_DISTANCE_BASED_BATCHING_IS_SUPPORTED)>0) {
                        if ((NotificationType.NOTIFICATION_WHEN_BUFFER_IS_FULL ==
                                mapItem.getNotifyType()) ||
                            (NotificationType.NOTIFICATION_WHEN_TRIP_IS_COMPLETED ==
                                    mapItem.getNotifyType())) {
                            mService.unregisterCallback(FLP_SEESION_BACKGROUND,
                                                        mapItem.getCbWrapper());
                        } else if (NotificationType.NOTIFICATION_ON_EVERY_LOCATION_FIX == mapItem.getNotifyType()) {
                            mService.unregisterCallback(FLP_SEESION_FOREGROUND,
                                                        mapItem.getCbWrapper());
                        }
                    } else {
                        mService.unregisterCallback(FLP_SEESION_PASSIVE,
                                                    mapItem.getCbWrapper());
                    }
                    mFlpRequestsMap.remove(handler);
                }
            } catch (RemoteException e) {
                throw new RuntimeException("Failed stopFlpSession", e);
            }
        }

        @Override
        public void registerForPassiveLocations(IZatFlpService.IFlpLocationCallback callback)
            throws IZatIllegalArgumentException {
            if (null == callback) {
                throw new IZatIllegalArgumentException("invalid input parameter");
            }
            try {
                synchronized (sFlpServiceLock) {
                    if (null == mFlpPassiveCbMap.get(callback)) {
                        LocationCallbackWrapper cbWrapper = new LocationCallbackWrapper(callback);
                        mFlpPassiveCbMap.put(callback, cbWrapper);
                        mService.registerCallback(FLP_SEESION_PASSIVE,
                                                  FLP_PASSIVE_LISTENER_HW_ID,
                                                  cbWrapper,
                                                  System.currentTimeMillis());
                    } else {
                        Log.w(TAG, "this passive callback is already registered.");
                    }
                }
            } catch (RemoteException e) {
                throw new RuntimeException("Failed registerForPassiveLocations", e);
            }
        }

        @Override
        public void deregisterForPassiveLocations(IZatFlpService.IFlpLocationCallback callback)
            throws IZatIllegalArgumentException {
            if (null == callback) {
                throw new IZatIllegalArgumentException("invalid input parameter");
            }
            try {
                synchronized (sFlpServiceLock) {
                    LocationCallbackWrapper cbWrapper = mFlpPassiveCbMap.get(callback);
                    if (null == cbWrapper) {
                        Log.w(TAG, "this passive callback is not registered.");
                    } else {
                        mService.unregisterCallback(FLP_SEESION_PASSIVE,
                                                    cbWrapper);
                        mFlpPassiveCbMap.remove(callback);
                    }
                }
            } catch (RemoteException e) {
                throw new RuntimeException("Failed deregisterForPassiveLocations", e);
            }
        }

        private class IZatSessionHandlerImpl implements IZatFlpService.IZatFlpSessionHandle {
            @Override
            public void pullLocations() {

                try {
                    synchronized (sFlpServiceLock) {
                        FlpRequestMapItem mapItem = mFlpRequestsMap.get(this);
                        if (null == mapItem) {
                            Log.w(TAG, "no flp session undergoing");
                            return;
                        }
                        if (null == mapItem.getCbWrapper()) {
                            Log.w(TAG, "no available callback");
                            return;
                        }
                        int result = mService.pullLocations(mapItem.getCbWrapper(),
                                                            mapItem.getSessionStartTime(),
                                                            mapItem.getHwId());
                        if (FLP_RESULT_SUCCESS == result) {
                            mapItem.setSessionStartTime(System.currentTimeMillis());
                        }
                        if (VERBOSE) {
                            Log.v(TAG, "pullLocations() returning : " + result);
                        }
                    }
                } catch (RemoteException e) {
                    throw new RuntimeException("Failed pullLocations", e);
                }
            }

            @Override
            public void setToForeground() {
                try {
                    synchronized (sFlpServiceLock) {
                        FlpRequestMapItem mapItem = mFlpRequestsMap.get(this);
                        if (null == mapItem) {
                            Log.w(TAG, "no flp session undergoing");
                            return;
                        }
                        if (null == mapItem.getCbWrapper()) {
                            Log.w(TAG, "no available callback");
                            return;
                        }
                        int result = FLP_RESULT_SUCCESS;
                        if (NotificationType.NOTIFICATION_ON_EVERY_LOCATION_FIX !=
                                mapItem.getNotifyType()) {
                            result = mService.updateFlpSessionWithPower(mapItem.getHwId(),
                                NotificationType.NOTIFICATION_ON_EVERY_LOCATION_FIX.getCode(),
                                mapItem.getTimeInterval(),
                                mapItem.getDistanceInterval(), 0,
                                mapItem.getPowerMode(), mapItem.getTbmMillis());
                            if (FLP_RESULT_SUCCESS == result) {
                                // update the item flag in map
                                mapItem.updateNotifyType(
                                    NotificationType.NOTIFICATION_ON_EVERY_LOCATION_FIX);
                                mFlpRequestsMap.put(this, mapItem);
                                // move the callback form bg to fg
                                if ((mFlpFeaturMasks &
                                    FEATURE_BIT_DISTANCE_BASED_BATCHING_IS_SUPPORTED)>0) {
                                    mService.unregisterCallback(FLP_SEESION_BACKGROUND,
                                                                mapItem.getCbWrapper());
                                    mService.registerCallback(FLP_SEESION_FOREGROUND,
                                                              mapItem.getHwId(),
                                                              mapItem.getCbWrapper(),
                                                              mapItem.getSessionStartTime());
                                }
                            } else {
                                Log.v(TAG, "mService.updateFlpSession failed.");
                            }
                        }
                        if (VERBOSE)  {
                            Log.v(TAG, "setToForeground() returning : " + result);
                        }
                    }
                } catch (RemoteException e) {
                    throw new RuntimeException("Failed setToForeground", e);
                }
            }

            @Override
            public void setToBackground() {
                try {
                    synchronized (sFlpServiceLock) {
                        FlpRequestMapItem mapItem = mFlpRequestsMap.get(this);
                        if (null == mapItem) {
                            Log.w(TAG, "no flp session undergoing");
                            return;
                        }
                        if (null == mapItem.getCbWrapper()) {
                            Log.w(TAG, "no available callback");
                            return;
                        }
                        int result = FLP_RESULT_SUCCESS;
                        if (NotificationType.NOTIFICATION_WHEN_BUFFER_IS_FULL !=
                                mapItem.getNotifyType()) {
                            result = mService.updateFlpSessionWithPower(mapItem.getHwId(),
                                NotificationType.NOTIFICATION_WHEN_BUFFER_IS_FULL.getCode(),
                                mapItem.getTimeInterval(),
                                mapItem.getDistanceInterval(), 0,
                                mapItem.getPowerMode(), mapItem.getTbmMillis());
                            if (FLP_RESULT_SUCCESS == result) {
                                // update the item flag in map
                                mapItem.updateNotifyType(
                                    NotificationType.NOTIFICATION_WHEN_BUFFER_IS_FULL);
                                mFlpRequestsMap.put(this, mapItem);
                                if ((mFlpFeaturMasks &
                                    FEATURE_BIT_DISTANCE_BASED_BATCHING_IS_SUPPORTED)>0) {
                                    // move the callback form fg to bg
                                    mService.unregisterCallback(FLP_SEESION_FOREGROUND,
                                                                mapItem.getCbWrapper());
                                    mService.registerCallback(FLP_SEESION_BACKGROUND,
                                                              mapItem.getHwId(),
                                                              mapItem.getCbWrapper(),
                                                              mapItem.getSessionStartTime());
                                }
                            } else {
                                Log.v(TAG, "mService.updateFlpSession failed.");
                            }
                        }
                        if (VERBOSE) {
                            Log.v(TAG, "setToBackground() returning : " + result);
                        }
                    }
                } catch (RemoteException e) {
                    throw new RuntimeException("Failed setToBackground", e);
                }
            }

            @Override
            public void setToTripMode() throws
                    IZatFeatureNotSupportedException {
                if (0 == (mFlpFeaturMasks & FEATURE_BIT_OUTDOOR_TRIP_BATCHING_IS_SUPPORTED)) {
                    throw new IZatFeatureNotSupportedException (
                            "Outdoor trip mode is not supported.");
                }

                try {
                    synchronized (sFlpServiceLock) {
                        FlpRequestMapItem mapItem = mFlpRequestsMap.get(this);
                        if (null == mapItem) {
                            Log.w(TAG, "no flp session undergoing");
                            return;
                        }
                        if (null == mapItem.getCbWrapper()) {
                            Log.w(TAG, "no available callback");
                            return;
                        }

                        if (0 == (mFlpFeaturMasks &
                                  FEATURE_BIT_OUTDOOR_TRIP_BATCHING_IS_SUPPORTED)) {
                            Log.w(TAG, "Outdoor Trip mode not supported");
                            return;
                        }

                        int result = FLP_RESULT_SUCCESS;
                        if (NotificationType.NOTIFICATION_WHEN_TRIP_IS_COMPLETED !=
                                mapItem.getNotifyType()) {
                            result = mService.updateFlpSessionWithPower(mapItem.getHwId(),
                                    NotificationType.NOTIFICATION_WHEN_TRIP_IS_COMPLETED.getCode(),
                                    mapItem.getTimeInterval(),
                                    mapItem.getDistanceInterval(), mapItem.getTripDistance(),
                                    mapItem.getPowerMode(), mapItem.getTbmMillis());
                            if (FLP_RESULT_SUCCESS == result) {
                                // update the item flag in map
                                mapItem.updateNotifyType(
                                        NotificationType.NOTIFICATION_WHEN_TRIP_IS_COMPLETED);

                                // remember to switch back to previous state once
                                // trip is completed.
                                mapItem.setRestartOnTripCompleted(true);
                                mFlpRequestsMap.put(this, mapItem);
                                if ((mFlpFeaturMasks &
                                    FEATURE_BIT_DISTANCE_BASED_BATCHING_IS_SUPPORTED)>0) {
                                    if (NotificationType.NOTIFICATION_ON_EVERY_LOCATION_FIX ==
                                            mapItem.getPreviousNotifyType()) {
                                        // switching from foreground to trip mode
                                        mService.unregisterCallback(FLP_SEESION_FOREGROUND,
                                                                    mapItem.getCbWrapper());
                                        mService.registerCallback(FLP_SEESION_BACKGROUND,
                                                                  mapItem.getHwId(),
                                                                  mapItem.getCbWrapper(),
                                                                  mapItem.getSessionStartTime());
                                    }
                                }
                            } else {
                                Log.v(TAG, "mService.updateFlpSession failed.");
                            }
                        }
                        if (VERBOSE)  {
                            Log.v(TAG, "setToTripMode() returning : " + result);
                        }
                    }
                } catch (RemoteException e) {
                    throw new RuntimeException("Failed setToTripMode", e);
                }
            }

            @Override
            public void registerForSessionStatus(IFlpStatusCallback callback)
                    throws IZatIllegalArgumentException, IZatFeatureNotSupportedException {
                if (0 == (mFlpFeaturMasks & FEATURE_BIT_OUTDOOR_TRIP_BATCHING_IS_SUPPORTED)) {
                    throw new IZatFeatureNotSupportedException (
                            "Session status not supported.");
                }

                if (null == callback) {
                    throw new IZatIllegalArgumentException("invalid input parameter");
                }

                try {
                    synchronized (sFlpServiceLock) {
                        FlpRequestMapItem mapItem = mFlpRequestsMap.get(this);
                        if (null == mapItem) {
                            Log.w(TAG, "no flp session undergoing");
                            return;
                        }

                        FlpStatusCallbackWrapper cbWrapper =
                            new FlpStatusCallbackWrapper(callback, mService);
                        mapItem.setStatusCallback(cbWrapper);
                        mService.registerForSessionStatus(mapItem.getHwId(), cbWrapper);

                    }
                } catch (RemoteException e) {
                    throw new RuntimeException("Failed registerForSessionStatus", e);
                }
            }

            @Override
            public void unregisterForSessionStatus()
                    throws  IZatIllegalArgumentException, IZatFeatureNotSupportedException {

                    if (0 == (mFlpFeaturMasks & FEATURE_BIT_OUTDOOR_TRIP_BATCHING_IS_SUPPORTED)) {
                    throw new IZatFeatureNotSupportedException (
                            "Session status not supported.");
                }

                try {
                    synchronized (sFlpServiceLock) {
                        FlpRequestMapItem mapItem = mFlpRequestsMap.get(this);
                        if (null == mapItem) {
                            Log.w(TAG, "no flp session undergoing");
                            return;
                        }

                        FlpStatusCallbackWrapper cbWrapper = mapItem.getStatusCallback();
                        if (null == cbWrapper) {
                            Log.w(TAG, "no status callback wrapper is registered.");
                        } else {
                            mService.unregisterForSessionStatus(cbWrapper);
                            mapItem.setStatusCallback(null);
                        }
                    }
                } catch (RemoteException e) {
                    throw new RuntimeException("Failed unregisterForSessionStatus", e);
                }
        }
    }
  }

    /**
     * Implementation of interface IZatTestService. (FOR INTERNAL USE ONLY)
     *
     * @hide
     */
    private class IZatTestServiceImpl implements IZatTestService {

        ITestService mService;
        public IZatTestServiceImpl(ITestService service) {
            mService = service;
        }

        @Override
        public void deleteAidingData(long flags)
            throws IZatIllegalArgumentException {
            if (0 == flags) {
                throw new IZatIllegalArgumentException("invalid input parameter." +
                                                       " flags must be filled");
            }
            try {
                mService.deleteAidingData(flags);
            } catch (RemoteException e) {
                throw new RuntimeException("Failed deregisterForPassiveLocations", e);
            }
        }

        @Override
        public void updateXtraThrottle(boolean enabled) {
            try {
                mService.updateXtraThrottle(enabled);
            } catch (RemoteException e) {
                throw new RuntimeException("Failed updateXtraThrottle", e);
            }
        }

        @Override
        public void registerForMaxPowerAllocatedChange(
            IZatTestService.IFlpMaxPowerAllocatedCallback callback)
                throws IZatIllegalArgumentException {

            if (null == callback) {
                throw new IZatIllegalArgumentException("invalid input parameter");
            }
            try {
                synchronized (sTestServiceLock) {
                    if (null == mFlpMaxPowerCbMap.get(callback)) {
                        MaxPowerAllocatedCallbackWrapper cbWrapper =
                                    new MaxPowerAllocatedCallbackWrapper(callback);
                        mFlpMaxPowerCbMap.put(callback, cbWrapper);
                        mService.registerMaxPowerChangeCallback(cbWrapper);
                    } else {
                        Log.w(TAG, "this max power callback is already registered.");
                    }
                }
            } catch (RemoteException e) {
                throw new RuntimeException("Failed registerForMaxPowerAllocatedChange", e);
            }
        }

        @Override
        public void deregisterForMaxPowerAllocatedChange(
            IZatTestService.IFlpMaxPowerAllocatedCallback callback)
                throws IZatIllegalArgumentException {

            if (null == callback) {
                throw new IZatIllegalArgumentException("invalid input parameter");
            }
            try {
                synchronized (sTestServiceLock) {
                    MaxPowerAllocatedCallbackWrapper cbWrapper = mFlpMaxPowerCbMap.get(callback);
                    if (null == cbWrapper) {
                        Log.w(TAG, "this passive callback is not registered.");
                    } else {
                        mService.unregisterMaxPowerChangeCallback(cbWrapper);
                        mFlpMaxPowerCbMap.remove(callback);
                    }
                }
            } catch (RemoteException e) {
                throw new RuntimeException("Failed deregisterForMaxPowerAllocatedChange", e);
            }
        }
    }

    private class IZatWiFiDBReceiverImpl extends IZatWiFiDBReceiver {
        IWiFiDBReceiver mReceiver;
        public IZatWiFiDBReceiverImpl(IWiFiDBReceiver receiver,
                                      Object listener)
            throws IZatIllegalArgumentException {
            super(listener);
            if(null == receiver || null == listener) {
                throw new IZatIllegalArgumentException("IZatWiFiDBReceiverImpl:" +
                                                       " null receiver / listener");
            }
            mReceiver = receiver;
        }

        @Override
        public void requestAPList(int expire_in_days) {
            try {
                IZatDataValidation.dataValidate(expire_in_days, IZatDataTypes.WIFI_EXPIRE_DAYS);
                mReceiver.requestAPList(expire_in_days);
            } catch (RemoteException e) {
                throw new RuntimeException("Failed to request AP LIst", e);
            }
        }

        @Override
        public void requestScanList() {
            try {
                mReceiver.requestScanList();
            } catch (RemoteException e) {
                throw new RuntimeException("Failed to request AP LIst", e);
            }
        }

        @Override
        public void pushWiFiDB(List<IZatAPLocationData> izatLocationDataList,
                               List<IZatAPSpecialInfo> izatSpecialInfoList,
                               int days_valid) {

            if (IZatWiFiDBReceiver.MAXIMUM_INJECTION_ELEMENTS < izatLocationDataList.size()) {
                throw new RuntimeException(
                        "Exceeded maximum number of APs in izatLocationDataList: " +
                         IZatWiFiDBReceiver.MAXIMUM_INJECTION_ELEMENTS);
            }

            if (IZatWiFiDBReceiver.MAXIMUM_INJECTION_ELEMENTS < izatSpecialInfoList.size()) {
                throw new RuntimeException(
                        "Exceeded maximum number of APs in izatSpecialInfoList: " +
                        IZatWiFiDBReceiver.MAXIMUM_INJECTION_ELEMENTS);
            }

            List<APLocationData> apLocationDataList = new ArrayList<APLocationData>();
            List<APSpecialInfo> apSpecialInfoList = new ArrayList<APSpecialInfo>();

            for(IZatAPLocationData izatLocationData: izatLocationDataList) {
                if(null != izatLocationData) {
                    IZatDataValidation.dataValidate(izatLocationData);
                    APLocationData apLocationData = new APLocationData();
                    apLocationData.mMacAddress = izatLocationData.getMacAddress();
                    apLocationData.mLatitude = izatLocationData.getLatitude();
                    apLocationData.mLongitude = izatLocationData.getLongitude();
                    apLocationData.mValidBits = apLocationData.AP_LOC_WITH_LAT_LON;
                    try{
                        apLocationData.mMaxAntenaRange = izatLocationData.getMaxAntenaRange();
                        apLocationData.mValidBits |= apLocationData.AP_LOC_MAR_VALID;
                    }catch (IZatStaleDataException e){
                        Log.e(TAG, "MAR exception ");
                        //do nothing
                    }

                    try{
                        apLocationData.mHorizontalError = izatLocationData.getHorizontalError();
                        apLocationData.mValidBits |= apLocationData.AP_LOC_HORIZONTAL_ERR_VALID;
                    }catch (IZatStaleDataException e){
                        Log.e(TAG, "HE exception ");
                        //do nothing
                    }

                    try{
                        apLocationData.mReliability = izatLocationData.getReliability().ordinal();
                        apLocationData.mValidBits |= apLocationData.AP_LOC_RELIABILITY_VALID;
                    }catch (IZatStaleDataException e){
                        Log.e(TAG, "REL exception ");
                        //do nothing
                    }

                    apLocationDataList.add(apLocationData);
                }
            }

            for(IZatAPSpecialInfo spl_info: izatSpecialInfoList) {
                if(null != spl_info) {
                    IZatDataValidation.dataValidate(spl_info);
                    APSpecialInfo info = new APSpecialInfo();
                    info.mMacAddress = spl_info.mMacAddress;
                    info.mInfo = spl_info.mInfo.ordinal();
                    apSpecialInfoList.add(info);
                }
            }

            try {
                mReceiver.pushWiFiDB(apLocationDataList, apSpecialInfoList, days_valid);
            } catch (RemoteException e) {
                throw new RuntimeException("Failed to push WiFi DB", e);
            }
        }

        @Override
        public void pushLookupResult(List<IZatAPLocationData> izatLocationDataList,
                                     List<IZatAPSpecialInfo> izatSpecialInfoList) {
            List<APLocationData> apLocationDataList = new ArrayList<APLocationData>();
            List<APSpecialInfo> apSpecialInfoList = new ArrayList<APSpecialInfo>();

            for(IZatAPLocationData izatLocationData: izatLocationDataList) {
                if(null != izatLocationData) {
                    IZatDataValidation.dataValidate(izatLocationData);
                    APLocationData apLocationData = new APLocationData();
                    apLocationData.mMacAddress = izatLocationData.getMacAddress();
                    apLocationData.mLatitude = izatLocationData.getLatitude();
                    apLocationData.mLongitude = izatLocationData.getLongitude();
                    apLocationData.mValidBits = apLocationData.AP_LOC_WITH_LAT_LON;
                    try{
                        apLocationData.mMaxAntenaRange = izatLocationData.getMaxAntenaRange();
                        apLocationData.mValidBits |= apLocationData.AP_LOC_MAR_VALID;
                    }catch (IZatStaleDataException e){
                        Log.e(TAG, "MAR exception ");
                        //do nothing
                    }

                    try{
                        apLocationData.mHorizontalError = izatLocationData.getHorizontalError();
                        apLocationData.mValidBits |= apLocationData.AP_LOC_HORIZONTAL_ERR_VALID;
                    }catch (IZatStaleDataException e){
                        Log.e(TAG, "HE exception ");
                        //do nothing
                    }

                    try{
                        apLocationData.mReliability = izatLocationData.getReliability().ordinal();
                        apLocationData.mValidBits |= apLocationData.AP_LOC_RELIABILITY_VALID;
                    }catch (IZatStaleDataException e){
                        Log.e(TAG, "REL exception ");
                        //do nothing
                    }

                    apLocationDataList.add(apLocationData);
                }
            }

            for(IZatAPSpecialInfo spl_info: izatSpecialInfoList) {
                if(null != spl_info) {
                    IZatDataValidation.dataValidate(spl_info);
                    APSpecialInfo info = new APSpecialInfo();
                    info.mMacAddress = spl_info.mMacAddress;
                    info.mInfo = spl_info.mInfo.ordinal();
                    apSpecialInfoList.add(info);
                }
            }

            try {
                mReceiver.pushLookupResult(apLocationDataList, apSpecialInfoList);
            } catch (RemoteException e) {
                throw new RuntimeException("Failed to push WiFi DB", e);
            }
        }
    }

    private class IZatWiFiDBProviderImpl extends IZatWiFiDBProvider {
        IWiFiDBProvider mProvider;
        public IZatWiFiDBProviderImpl(IWiFiDBProvider provider,
                                      IZatWiFiDBProviderResponseListener listener)
                throws IZatIllegalArgumentException {
            super(listener);
            if(null == provider || null == listener) {
                throw new IZatIllegalArgumentException("IZatWiFiDBProviderImpl:" +
                        " null receiver / listener");
            }
            mProvider = provider;
        }

        @Override
        public void requestAPObsLocData() {
            try {
                mProvider.requestAPObsLocData();
            } catch (RemoteException e) {
                throw new RuntimeException("Failed to request AP Observation data", e);
            }
        }
    }
    private class IZatWWANDBReceiverImpl extends IZatWWANDBReceiver {
        IWWANDBReceiver mReceiver;
        public IZatWWANDBReceiverImpl(IWWANDBReceiver receiver,
                                      Object listener)
            throws IZatIllegalArgumentException {
            super(listener);
            if(null == receiver || null == listener) {
                throw new IZatIllegalArgumentException("IZatWWANDBReceiverImpl:" +
                                                       " null receiver / listener");
            }
            mReceiver = receiver;
        }

        @Override
        public void requestBSList(int expire_in_days) {
            try {
                IZatDataValidation.dataValidate(expire_in_days,
                                                IZatDataTypes.WIFI_EXPIRE_DAYS);
                mReceiver.requestBSList(expire_in_days);
            } catch (RemoteException e) {
                throw new RuntimeException("Failed to request BS LIst", e);
            }
        }

        @Override
        public void pushWWANDB(List<IZatBSLocationData> location_data,
                               List<IZatBSSpecialInfo> special_info,
                               int days_valid) {
            if (IZatWWANDBReceiver.MAXIMUM_INJECTION_ELEMENTS < location_data.size()) {
                throw new RuntimeException(
                        "Exceeded maximum number of BSs in location_data: " +
                         IZatWWANDBReceiver.MAXIMUM_INJECTION_ELEMENTS);
            }

            if (IZatWWANDBReceiver.MAXIMUM_INJECTION_ELEMENTS < special_info.size()) {
                throw new RuntimeException(
                        "Exceeded maximum number of BSs in special_info: " +
                        IZatWWANDBReceiver.MAXIMUM_INJECTION_ELEMENTS);
            }

            List<BSLocationData> locationData = new ArrayList<BSLocationData>();
            List<BSSpecialInfo> specialInfo = new ArrayList<BSSpecialInfo>();

            for(IZatBSLocationData loc_data: location_data) {
                if(null != loc_data) {
                    if (VERBOSE) {
                        log(loc_data);
                    }
                    IZatDataValidation.dataValidate(loc_data);
                    BSLocationData locData = new BSLocationData();
                    locData.mCellType = loc_data.getIZatCellInfo().getType().ordinal();
                    locData.mCellRegionID1 = loc_data.getIZatCellInfo().getRegionID1();
                    locData.mCellRegionID2 = loc_data.getIZatCellInfo().getRegionID2();
                    locData.mCellRegionID3 = loc_data.getIZatCellInfo().getRegionID3();
                    locData.mCellRegionID4 = loc_data.getIZatCellInfo().getRegionID4();

                    if (null != loc_data.getLocation()) {
                        locData.mLatitude = (float) loc_data.getLocation().getLatitude();
                        locData.mLongitude = (float) loc_data.getLocation().getLongitude();
                        locData.mValidBits = locData.BS_LOC_WITH_LAT_LON;

                        locData.mHorizontalCoverageRadius =
                                loc_data.getHorizontalCoverageRadius();
                        locData.mValidBits |= locData.BS_LOC_HORIZONTAL_COV_RADIUS_VALID;

                        locData.mAltitude = (float) loc_data.getLocation().getAltitude();
                        locData.mValidBits |= locData.BS_LOC_ALTITUDE_VALID;

                        locData.mAltitudeUncertainty =
                                loc_data.getLocation().getVerticalAccuracyMeters();
                        locData.mValidBits |= locData.BS_LOC_ALTITUDE_UNCERTAINTY_VALID;

                        locData.mHorizontalConfidence = Math.round(
                                loc_data.getHorizontalConfidence());
                        locData.mValidBits |= locData.BS_LOC_HORIZONTAL_CONFIDENCE_VALID;

                        locData.mAltitudeConfidence =
                                loc_data.getAltitudeConfidence();
                        locData.mValidBits |= locData.BS_LOC_ALTITUDE_CONFIDENCE_VALID;
                    }


                    try{
                        locData.mHorizontalReliability =
                                loc_data.getHorizontalReliability().ordinal();
                        locData.mValidBits |= locData.BS_LOC_HORIZONTAL_RELIABILITY_VALID;
                    }catch (IZatStaleDataException e){
                        Log.e(TAG, "HR exception ");
                        //do nothing
                    }

                    try{
                        locData.mAltitudeReliability = loc_data.getAltitudeReliability().ordinal();
                        locData.mValidBits |= locData.BS_LOC_ALTITUDE_RELIABILITY_VALID;
                    }catch (IZatStaleDataException e){
                        Log.e(TAG, "ALR exception ");
                        //do nothing
                    }

                    locationData.add(locData);
                }
            }

            for(IZatBSSpecialInfo spl_info: special_info) {
                if(null != spl_info) {
                    if (VERBOSE) {
                        log(spl_info);
                    }
                    IZatDataValidation.dataValidate(spl_info);

                    BSSpecialInfo info = new BSSpecialInfo();
                    info.mCellType = spl_info.getCellInfo().getType().ordinal();
                    info.mCellRegionID1 = spl_info.getCellInfo().getRegionID1();
                    info.mCellRegionID2 = spl_info.getCellInfo().getRegionID2();
                    info.mCellRegionID3 = spl_info.getCellInfo().getRegionID3();
                    info.mCellRegionID4 = spl_info.getCellInfo().getRegionID4();
                    info.mInfo = spl_info.getInfo().ordinal();
                    specialInfo.add(info);
                }
            }

            try {
                mReceiver.pushWWANDB(locationData, specialInfo, days_valid);
            } catch (RemoteException e) {
                throw new RuntimeException("Failed to push WWAN DB", e);
            }
        }
    }
    private class IZatWWANDBProviderImpl extends IZatWWANDBProvider {
        IWWANDBProvider mProvider;
        public IZatWWANDBProviderImpl(IWWANDBProvider provider,
                                      IZatWWANDBProviderResponseListener listener)
                throws IZatIllegalArgumentException {
            super(listener);
            if(null == provider || null == listener) {
                throw new IZatIllegalArgumentException("IZatWWANDBProviderImpl:" +
                        " null receiver / listener");
            }
            mProvider = provider;
        }

        @Override
        public void requestBSObsLocData() {
            try {
                mProvider.requestBSObsLocData();
            } catch (RemoteException e) {
                throw new RuntimeException("Failed to request BS Observation data", e);
            }
        }
    }

    private class IZatGeofenceServiceImpl implements IZatGeofenceService {
        IGeofenceService mService;
        public IZatGeofenceServiceImpl(IGeofenceService service) {
            mService = service;
        }

        @Override
        public void registerForGeofenceCallbacks(IZatGeofenceCallback statusCb)
                                                 throws IZatIllegalArgumentException {
            if (null == statusCb) {
                throw new IZatIllegalArgumentException("invalid input parameter");
            }
            /* save the callbacks bonding with the caller. If there is already a
               callback associated with the caller, then the new cb will override
               the existing cb. */
            synchronized (sGeofenceClientCallbackMapLock) {
                mGeofenceClientCallbackMap.put(this, statusCb);
            }
        }

        @Override
        public void deregisterForGeofenceCallbacks(IZatGeofenceCallback statusCb)
                                                   throws IZatIllegalArgumentException {
            if (null == statusCb) {
                throw new IZatIllegalArgumentException("invalid input parameter");
            }
            synchronized (sGeofenceClientCallbackMapLock) {
                mGeofenceClientCallbackMap.remove(this);
            }
        }

        @Override
        public void registerPendingIntent(PendingIntent geofenceIntent)
                                throws IZatIllegalArgumentException {
            if (null == geofenceIntent) {
                throw new IZatIllegalArgumentException("invalid input parameter");
            }

            synchronized (sGeofenceServiceLock) {
                try {
                    mService.registerPendingIntent(geofenceIntent);
                } catch (RemoteException e) {
                    throw new RuntimeException("Failed registerPendingIntent");
                }
            }
        }

        @Override
        public void unregisterPendingIntent(PendingIntent geofenceIntent)
                                throws IZatIllegalArgumentException {
            if (null == geofenceIntent) {
                throw new IZatIllegalArgumentException("invalid input parameter");
            }

            synchronized (sGeofenceServiceLock) {
                try {
                    mService.unregisterPendingIntent(geofenceIntent);
                } catch (RemoteException e) {
                    throw new RuntimeException("Failed unregisterPendingIntent");
                }
            }
        }

        @Override
        public Map<IZatGeofenceHandle, IzatGeofence> recoverGeofences() {
            IZatGeofenceCallback cb = mGeofenceClientCallbackMap.get(this);
            if (null == cb) {
                Log.e(TAG, "callback is not registered");
                return null;
            }

            Map<IZatGeofenceHandle, IzatGeofence> gfHandleDataMap =
                new HashMap<> ();
            ArrayList<GeofenceData> gfList = new ArrayList<GeofenceData> ();

            try {
                synchronized(sGeofenceServiceLock) {
                    mService.recoverGeofences(gfList);
                }
            } catch (RemoteException e) {
                throw new RuntimeException("Failed to recover geofences", e);
            }
            synchronized (sGeofencesMapLock) {
                for (GeofenceData geofence : gfList) {
                    // create a new IzatGeofence object to be returned
                    IZatGeofenceService.IzatGeofence gfObj =
                            getIzatGeofenceFromGeofenceData(geofence);

                    // check if IZatGeofenceHandle for this geofence id exists
                    // Update gfHandleDataMap
                    boolean handleExists = false;
                    for (Map.Entry<IZatGeofenceServiceImpl.IZatGeofenceHandleImpl,
                            GeofenceMapItem> entry :
                        mGeofencesMap.entrySet()) {
                        if (entry.getValue().getHWGeofenceId() == geofence.getGeofenceId()) {
                            handleExists = true;
                            gfHandleDataMap.put(entry.getKey(), gfObj);
                            break;
                        }
                    }

                    if (false == handleExists) {
                        IZatGeofenceHandleImpl handle = new IZatGeofenceHandleImpl();
                        String appTextData = geofence.getAppTextData();
                        Object appData;
                        if (appTextData != null) {
                            appData = appTextData;
                        } else {
                            appData = geofence.getAppBundleData();
                        }
                        mGeofencesMap.put(handle,
                            new GeofenceMapItem(appData,
                            geofence.getGeofenceId(), cb));
                        gfHandleDataMap.put(handle, gfObj);
                    }
                }
            }
            return gfHandleDataMap;
        }

        @Override
        public IZatGeofenceHandle addGeofence(Object context, IzatGeofence geofence)
                                              throws IZatIllegalArgumentException {
            if (null == geofence) {
                throw new IZatIllegalArgumentException("invalid null geofence");
            }
            if (geofence.getLatitude() < -90 || geofence.getLatitude() > 90) {
                throw new IZatIllegalArgumentException("invalid geofence latitude." +
                                                       " Expected in range -90..90.");
            }
            if (geofence.getLongitude() < -180 || geofence.getLongitude() > 180) {
                throw new IZatIllegalArgumentException("invalid geofence longitude." +
                                                       " Expected in range -180..180.");
            }
            IZatDataValidation.dataValidate(geofence);
            IZatGeofenceCallback cb = mGeofenceClientCallbackMap.get(this);
            int geofenceId;

            if (null == cb) {
                Log.e(TAG, "callback is not registered.");
                return null;
            }
            try {
                synchronized (sGeofenceServiceLock) {
                    double latitude = geofence.getLatitude();
                    double longitude = geofence.getLongitude();
                    double radius = geofence.getRadius();
                    int transitionType = geofence.getTransitionTypes().getValue();
                    int responsiveness = geofence.getNotifyResponsiveness();
                    int confidence = geofence.getConfidence().getValue();

                    int dwellTime = 0;
                    int dwellType = 0;
                    if (geofence.getDwellNotify() != null) {
                        dwellTime = geofence.getDwellNotify().getDwellTime();
                        dwellType = geofence.getDwellNotify().getDwellType();
                    }

                    if ((context != null) &&
                            ((context instanceof String) || (context instanceof Bundle))) {

                        String appTextData =
                                ((context instanceof String) ? context.toString() : null);
                        Bundle bundleObj =
                                ((context instanceof Bundle) ? (Bundle) context : null);

                        geofenceId = mService.addGeofenceObj(new GeofenceData(responsiveness,
                                                                              latitude,
                                                                              longitude,
                                                                              radius,
                                                                              transitionType,
                                                                              confidence,
                                                                              dwellType,
                                                                              dwellTime,
                                                                              appTextData,
                                                                              bundleObj,
                                                                              -1));
                    } else {
                        geofenceId = mService.addGeofence(latitude,
                                                          longitude,
                                                          radius,
                                                          transitionType,
                                                          responsiveness,
                                                          confidence,
                                                          dwellTime,
                                                          dwellType);
                    }
                }
            } catch (RemoteException e) {
                throw new RuntimeException("Failed addGeofence", e);
            }
            IZatGeofenceHandleImpl handle = new IZatGeofenceHandleImpl();
            synchronized (sGeofencesMapLock) {
                mGeofencesMap.put(handle,
                                  new GeofenceMapItem(context, geofenceId, cb));
            }
            return handle;
        }

        @Override
        public void removeGeofence(IZatGeofenceHandle handler)
                                   throws IZatIllegalArgumentException {
            if (null == handler ||
                !(handler instanceof IZatGeofenceServiceImpl.IZatGeofenceHandleImpl)) {
                throw new IZatIllegalArgumentException("invalid input parameter");
            }
            try {
                synchronized (sGeofenceServiceLock) {
                    GeofenceMapItem mapItem = mGeofencesMap.get(handler);
                    if (null == mapItem) {
                        Log.e(TAG, "this request ID is unknown.");
                        IZatGeofenceCallback cb =
                                mGeofenceClientCallbackMap.get(this);
                        if (cb != null) {
                            cb.onRequestFailed(handler,
                                    GEOFENCE_REQUEST_TYPE_REMOVE,
                                    GEOFENCE_RESULT_ERROR_ID_UNKNOWN);
                        } else {
                            throw new IZatIllegalArgumentException(
                                    "Invalid Geofence handle");
                        }
                        return;
                    }
                    mService.removeGeofence(mapItem.getHWGeofenceId());
                }
            } catch (RemoteException e) {
                throw new RuntimeException("Failed removeGeofence", e);
            }
            synchronized (sGeofencesMapLock) {
                mGeofencesMap.remove(handler);
            }
        }

        private GeofenceData recoverGeofenceDataById(int geofenceId)
                throws IZatIllegalArgumentException {
            ArrayList<GeofenceData> gfList = new ArrayList<GeofenceData> ();
            try {
                synchronized(sGeofenceServiceLock) {
                    mService.recoverGeofences(gfList);

                    for (GeofenceData geofence : gfList) {
                        if (geofenceId == geofence.getGeofenceId()) {
                            return geofence;
                        }
                    }
                    throw new IZatIllegalArgumentException("Invalid geofence id");
                }
            } catch (RemoteException e) {
                throw new RuntimeException("Failed to recover geofence", e);
            }
        }

        private IzatGeofence getIzatGeofenceFromGeofenceData(GeofenceData geofence) {
            IZatGeofenceService.IzatGeofence gfObj =
                    new IZatGeofenceService.IzatGeofence(geofence.getLatitude(),
                    geofence.getLongitude(), geofence.getRadius());
                    gfObj.setNotifyResponsiveness(geofence.getNotifyResponsiveness());
                    gfObj.setTransitionTypes(
                            IZatGeofenceService.IzatGeofenceTransitionTypes.values()[
                            geofence.getTransitionType().getValue()]);
                    gfObj.setConfidence(
                            IZatGeofenceService.IzatGeofenceConfidence.values()[
                            geofence.getConfidence().getValue() - 1]);

                    IZatGeofenceService.IzatDwellNotify dwellNotify =
                            new IZatGeofenceService.IzatDwellNotify(
                                    geofence.getDwellTime(),
                                    geofence.getDwellType().getValue());
                    gfObj.setDwellNotify(dwellNotify);

            return gfObj;
        }

        private class IZatGeofenceHandleImpl implements IZatGeofenceService.IZatGeofenceHandle {

            @Override
            public Object getContext() {
                GeofenceMapItem mapItem = mGeofencesMap.get(this);
                if (mapItem != null) {
                    return mapItem.getContext();
                }
                return null;
            }

            @Override
            @Deprecated
            public void update(IzatGeofenceTransitionTypes transitionTypes,
                               int notifyResponsiveness)
                               throws IZatIllegalArgumentException {
                if (null == transitionTypes || 0 >= notifyResponsiveness) {
                    throw new IZatIllegalArgumentException("invalid input parameter");
                }

                IzatGeofence gf = new IZatGeofenceService.IzatGeofence(null, null, null);
                gf.setTransitionTypes(transitionTypes);
                gf.setNotifyResponsiveness(notifyResponsiveness);

                update(gf);
            }

            @Override
            public void update(IzatGeofence geofence) throws IZatIllegalArgumentException {
                try {
                    synchronized (sGeofenceServiceLock) {
                        GeofenceMapItem mapItem = mGeofencesMap.get(this);
                        if (null == mapItem) {
                            Log.e(TAG, "this request ID is unknown.");
                            IZatGeofenceCallback cb =
                                    mGeofenceClientCallbackMap.get(this);
                            if (cb != null) {
                                cb.onRequestFailed(this,
                                        GEOFENCE_REQUEST_TYPE_UPDATE,
                                        GEOFENCE_RESULT_ERROR_ID_UNKNOWN);
                            } else {
                                throw new IZatIllegalArgumentException(
                                        "Invalid Geofence handle");
                            }
                            return;
                        }
                        mService.updateGeofenceData(mapItem.getHWGeofenceId(),
                                                    geofence.getLatitude(),
                                                    geofence.getLongitude(),
                                                    geofence.getRadius(),
                                                    geofence.getTransitionTypes().getValue(),
                                                    geofence.getNotifyResponsiveness(),
                                                    geofence.getConfidence().getValue(),
                                                    geofence.getDwellNotify().getDwellTime(),
                                                    geofence.getDwellNotify().getDwellType(),
                                                    geofence.getUpdatedFieldsMask());
                        geofence.resetUpdatedFieldsMask();
                    }
                } catch (RemoteException e) {
                    throw new RuntimeException("Failed update", e);
                }
            }

            @Override
            public void pause() throws IZatIllegalArgumentException {
                try {
                    synchronized (sGeofenceServiceLock) {
                        GeofenceMapItem mapItem = mGeofencesMap.get(this);
                        if (null == mapItem) {
                            Log.e(TAG, "this request ID is unknown.");
                            IZatGeofenceCallback cb =
                                    mGeofenceClientCallbackMap.get(this);
                            if (cb != null) {
                                cb.onRequestFailed(this,
                                        GEOFENCE_REQUEST_TYPE_PAUSE,
                                        GEOFENCE_RESULT_ERROR_ID_UNKNOWN);
                            } else {
                                throw new IZatIllegalArgumentException(
                                        "Invalid Geofence handle");
                            }
                            return;
                        }
                        mService.pauseGeofence(mapItem.getHWGeofenceId());
                    }
                } catch (RemoteException e) {
                    throw new RuntimeException("Failed pause", e);
                }
            }

            @Override
            public void resume() throws IZatIllegalArgumentException {
                try {
                    synchronized (sGeofenceServiceLock) {
                        GeofenceMapItem mapItem = mGeofencesMap.get(this);
                        if (null == mapItem) {
                            Log.e(TAG, "this request ID is unknown.");
                            IZatGeofenceCallback cb =
                                    mGeofenceClientCallbackMap.get(this);
                            if (cb != null) {
                                cb.onRequestFailed(this,
                                        GEOFENCE_REQUEST_TYPE_RESUME,
                                        GEOFENCE_RESULT_ERROR_ID_UNKNOWN);
                            } else {
                                throw new IZatIllegalArgumentException(
                                        "Invalid Geofence handle");
                            }
                            return;
                        }
                        mService.resumeGeofence(mapItem.getHWGeofenceId());
                    }
                } catch (RemoteException e) {
                    throw new RuntimeException("Failed resume", e);
                }
            }

            @Override
            public boolean isPaused() throws IZatIllegalArgumentException {
                synchronized (sGeofenceServiceLock) {
                    GeofenceMapItem mapItem = mGeofencesMap.get(this);
                    if (null == mapItem) {
                        Log.e(TAG, "this request ID is unknown.");
                        throw new IZatIllegalArgumentException(
                                "Invalid Geofence handle");
                    }
                    return recoverGeofenceDataById(mapItem.mHWGeofenceId).isPaused();
                }
            }
       }
    }

    public class IZatDebugReportingServiceImpl implements IZatDebugReportingService {
        IDebugReportService mService;

        public IZatDebugReportingServiceImpl(IDebugReportService service) {
            mService = service;
        }

        @Override
        public void registerForDebugReports(IZatDebugReportCallback reportCb)
                                                 throws IZatIllegalArgumentException {
            if (null == reportCb) {
                throw new IZatIllegalArgumentException("invalid input parameter");
            }

            synchronized (sDebugReportCallbackMapLock) {
                mDebugReportClientCallbackMap.put(this, reportCb);
            }

            synchronized(sDebugReportServiceLock) {
                if (!mDebugReportClientCallbackMap.isEmpty()) {
                    try {
                        mService.startReporting();
                    } catch (RemoteException e) {
                        throw new RuntimeException("Failed to register for debug reports");
                    }
                }
            }
        }

        @Override
        public void deregisterForDebugReports(IZatDebugReportCallback reportCb)
                                                   throws IZatIllegalArgumentException {
            if (null == reportCb) {
                throw new IZatIllegalArgumentException("invalid input parameter");
            }

            synchronized (sDebugReportCallbackMapLock) {
                IZatDebugReportCallback cb = mDebugReportClientCallbackMap.get(this);
                if (cb != null) {
                    mDebugReportClientCallbackMap.remove(this);
                }
            }

            synchronized(sDebugReportServiceLock) {
                if (mDebugReportClientCallbackMap.isEmpty()) {
                    try {
                        mService.stopReporting();
                    } catch (RemoteException e) {
                        throw new RuntimeException("Failed to deregister for debug reports");
                    }
                }
            }
        }

       @Override
       public Bundle getDebugReport()
           throws IZatIllegalArgumentException {
           synchronized(sDebugReportServiceLock) {
                Bundle bdlReportObj = null;

                try {
                    bdlReportObj = mService.getDebugReport();
                } catch (RemoteException e) {
                    throw new RuntimeException("Failed to get debug report");
                }

                return bdlReportObj;
           }
       }
    }

    public class IZatGnssConfigServiceImpl implements IZatGnssConfigService {
        IGnssConfigService mService;

        public IZatGnssConfigServiceImpl(IGnssConfigService service) {
            mService = service;
        }

        @Override
        public void getGnssSvTypeConfig(IZatGnssConfigCallback gnssConfigCb)
                                                 throws IZatIllegalArgumentException {
            if (null == gnssConfigCb) {
                throw new IZatIllegalArgumentException("Invalid gnss config cb");
            }

            synchronized (sGnssConfigCallbackMapLock) {
                mGnssConfigClientCallbackMap.put(this, gnssConfigCb);
            }

            synchronized(sGnssConfigServiceLock) {
                if (!mGnssConfigClientCallbackMap.isEmpty()) {
                    try {
                        mService.registerCallback(mGnssConfigCbWrapper);
                        mService.getGnssSvTypeConfig();
                    } catch (RemoteException e) {
                        throw new RuntimeException("Failed to get gnss sv type config");
                    }
                }
            }
        }

        @Override
        public void setGnssSvTypeConfig(Set<IzatGnssSvType> enabledSvTypeSet,
                                        Set<IzatGnssSvType> disabledSvTypeSet)
                                                   throws IZatIllegalArgumentException {
            if (null == enabledSvTypeSet && null == disabledSvTypeSet) {
                throw new IZatIllegalArgumentException("Both enabled/disabled sv type lists null");
            }

            synchronized(sGnssConfigServiceLock) {
                try {
                    // Convert List<IzatGnssSvType> to List<Integer>
                    int svTypeArrLen = 0;
                    if (null != enabledSvTypeSet) {
                        svTypeArrLen += enabledSvTypeSet.size();
                    }
                    if (null != disabledSvTypeSet) {
                        svTypeArrLen += disabledSvTypeSet.size();
                    }
                    int[] disabledSvTypeArray = new int[svTypeArrLen];
                    int idx = 0;
                    if (null != disabledSvTypeSet) {
                        for (IzatGnssSvType svType : disabledSvTypeSet) {
                            if (null != enabledSvTypeSet && enabledSvTypeSet.contains(svType)) {
                                throw new IZatIllegalArgumentException(
                                        "Both enabled/disabled sv type lists contain: " + svType);
                            }
                            disabledSvTypeArray[idx++] = svType.getValue();
                        }
                    }
                    if (null != enabledSvTypeSet) {
                        for (IzatGnssSvType svType : enabledSvTypeSet) {
                            disabledSvTypeArray[idx++] = ~(svType.getValue());
                        }
                    }
                    mService.setGnssSvTypeConfig(disabledSvTypeArray);
                } catch (RemoteException e) {
                    throw new RuntimeException("Failed to set gnss sv type config");
                }
            }
        }

        @Override
        public void resetGnssSvTypeConfig() {

            synchronized(sGnssConfigServiceLock) {
                try {
                    mService.resetGnssSvTypeConfig();
                } catch (RemoteException e) {
                    throw new RuntimeException("Failed to reset gnss sv type config");
                }
            }
        }
    }

    /**
     * Implementation of callback object. (FOR INTERNAL USE ONLY)
     *
     * @hide
     */
    private class LocationCallbackWrapper extends ILocationCallback.Stub {
        IZatFlpService.IFlpLocationCallback mCallback;
        public LocationCallbackWrapper(IZatFlpService.IFlpLocationCallback callback) {
            mCallback = callback;
        }
        public void onLocationAvailable(Location[] locations) {
            if (null == mCallback) {
                Log.w(TAG, "mCallback is NULL in LocationCallbackWrapper");
                return;
            }
            IZatDataValidation.dataValidate(locations);
            mCallback.onLocationAvailable(locations);
        }
    }

    /* Implementation of callback object. (FOR INTERNAL USE ONLY)
     *
     * @hide
     */
    private class FlpStatusCallbackWrapper extends ISessionStatusCallback.Stub {
        IZatFlpService.IFlpStatusCallback mCallback;
        IFlpService mService;

        public FlpStatusCallbackWrapper(IZatFlpService.IFlpStatusCallback callback,
            IFlpService flpService) {
            mCallback = callback;
            mService = flpService;
        }

        public void onBatchingStatusCb(int status) {
            if (null == mCallback) {
                Log.w(TAG, "mCallback is NULL in FlpStatusCallbackWrapper");
            }
            IZatDataValidation.dataValidate(status, IZatDataTypes.FLP_STATUS);

            try {
                synchronized(sFlpServiceLock) {
                    IZatFlpService.IzatFlpStatus batchStatus =
                            IZatFlpService.IzatFlpStatus.values()[status];

                    if (batchStatus != IZatFlpService.IzatFlpStatus.OUTDOOR_TRIP_COMPLETED) {
                        mCallback.onBatchingStatusCb(batchStatus);
                        return;
                    }

                    FlpRequestMapItem mapItem = null;
                    IZatFlpServiceImpl.IZatSessionHandlerImpl sessionHandler = null;
                    for (IZatFlpServiceImpl.IZatSessionHandlerImpl key : mFlpRequestsMap.keySet()) {
                        mapItem = mFlpRequestsMap.get(key);
                        if (mapItem.getStatusCallback() == this) {
                            sessionHandler = key;
                            break;
                        }
                    }

                    if (null == mapItem) {
                        Log.w(TAG, "no flp session undergoing");
                        return;
                    }

                    NotificationType notifType = mapItem.getNotifyType();
                    if ((IZatFlpService.IzatFlpStatus.OUTDOOR_TRIP_COMPLETED == batchStatus) &&
                        (NotificationType.NOTIFICATION_WHEN_TRIP_IS_COMPLETED == notifType)) {
                        mCallback.onBatchingStatusCb(batchStatus);

                        // update the FLP session previous notification mode
                        if (mapItem.getRestartOnTripCompleted()) {
                            int result = FLP_RESULT_SUCCESS;
                            result = mService.startFlpSession(mapItem.getHwId(),
                                    mapItem.getPreviousNotifyType().getCode(),
                                    mapItem.getTimeInterval(),
                                    mapItem.getDistanceInterval(), mapItem.getTripDistance());

                            if (FLP_RESULT_SUCCESS == result) {
                                // update the item flag in map
                                mapItem.updateNotifyType(mapItem.getPreviousNotifyType());
                                mapItem.setRestartOnTripCompleted(false);

                                if ((mFlpFeaturMasks &
                                    FEATURE_BIT_DISTANCE_BASED_BATCHING_IS_SUPPORTED)>0) {
                                    if (NotificationType.NOTIFICATION_ON_EVERY_LOCATION_FIX ==
                                            mapItem.getNotifyType()) {
                                        // switching from trip mode to foreground mode
                                        mService.unregisterCallback(FLP_SEESION_BACKGROUND,
                                                                    mapItem.getCbWrapper());
                                        mService.registerCallback(FLP_SEESION_FOREGROUND,
                                                                  mapItem.getHwId(),
                                                                  mapItem.getCbWrapper(),
                                                                  mapItem.getSessionStartTime());
                                    }
                                }
                            } else {
                                Log.v(TAG, "mService.updateFlpSession on trip completed failed.");
                            }
                        } else {
                            // remove the session handler from the cache as the trip ahs completed
                            mService.unregisterCallback(FLP_SEESION_BACKGROUND,
                                                        mapItem.getCbWrapper());
                            sessionHandler.unregisterForSessionStatus();
                            mFlpRequestsMap.remove(sessionHandler);
                        }
                    }
                }
            } catch (RemoteException e) {
                   throw new RuntimeException("Failed to handle onBatchingStatusCb for status:" +
                           status);
            }
        }
    }

    private enum NotificationType {
        NOTIFICATION_WHEN_BUFFER_IS_FULL(1),
        NOTIFICATION_ON_EVERY_LOCATION_FIX(2),
        NOTIFICATION_WHEN_TRIP_IS_COMPLETED(3);
        private final int mCode;
        private NotificationType(int c) {
            mCode = c;
        }

        public int getCode() {
            return mCode;
        }
    }

    private class FlpRequestMapItem {
        public IZatFlpService.IFlpLocationCallback mCallback = null;
        public NotificationType mNotiType = null;
        public LocationCallbackWrapper mCbWrapper = null;
        public FlpStatusCallbackWrapper mStatusCbWrapper = null;
        public long mMaxTime = -1;
        public int mMaxDistance = -1;
        public long mTripDistance = -1;
        public int mHwId = -1;
        private long mSessionStartTime = -1;
        private boolean mRestartOnTripCompleted = false;
        private NotificationType mPreviousNotifType = null;
        private int mPowerMode = 0;
        private long mTbmMillis = 0;
        public FlpRequestMapItem(IZatFlpService.IFlpLocationCallback callback,
                                 NotificationType notiType,
                                 long maxTime,
                                 int maxDistance,
                                 long tripDistance,
                                 LocationCallbackWrapper cbWrapper,
                                 int hwId,
                                 long sessionStartTime,
                                 int powerMode,
                                 long tbmMillis) {
            mCallback = callback;
            mNotiType = notiType;
            mPreviousNotifType = notiType;
            mMaxTime = maxTime;
            mMaxDistance = maxDistance;
            mTripDistance = tripDistance;
            mCbWrapper = cbWrapper;
            mHwId = hwId;
            mSessionStartTime = sessionStartTime;
            mRestartOnTripCompleted = false;
            mStatusCbWrapper = null;
            mPowerMode = powerMode;
            mTbmMillis = tbmMillis;
        }
        public IZatFlpService.IFlpLocationCallback getCallback() {
            return mCallback;
        }
        public void updateNotifyType(NotificationType type) {
            mNotiType = type;

            if (NotificationType.NOTIFICATION_WHEN_TRIP_IS_COMPLETED != type) {
                mPreviousNotifType = type;
            }
        }
        public NotificationType getNotifyType() {
            return mNotiType;
        }

        public NotificationType getPreviousNotifyType() {
            return mPreviousNotifType;
        }

        public long getTimeInterval() {
            return mMaxTime;
        }
        public int getDistanceInterval() {
            return mMaxDistance;
        }

        public long getTripDistance() {
            return mTripDistance;
        }

        public LocationCallbackWrapper getCbWrapper() {
            return mCbWrapper;
        }
        public int getHwId() {
            return mHwId;
        }
        public long getSessionStartTime() {
            return mSessionStartTime;
        }
        public void setSessionStartTime(long sessionStartTime) {
            mSessionStartTime = sessionStartTime;
        }

        public void setRestartOnTripCompleted(boolean restart) {
            mRestartOnTripCompleted = restart;
        }

        public boolean getRestartOnTripCompleted() {
            return mRestartOnTripCompleted;
        }

        public void setStatusCallback(FlpStatusCallbackWrapper cbWrapper) {
            mStatusCbWrapper = cbWrapper;
        }

        public FlpStatusCallbackWrapper getStatusCallback() {
            return mStatusCbWrapper;
        }

        public void setPowerMode(int powerMode) {
            if (0== (mFlpFeaturMasks & FEATURE_BIT_AGPM_IS_SUPPORTED)) {
                throw new IZatFeatureNotSupportedException ("AGPM is not supported");
            }
            mPowerMode = powerMode;
        }
        public int getPowerMode() {
            return mPowerMode;
        }

        public void setTbmMillis(long tbmMillis) {
            if (0== (mFlpFeaturMasks & FEATURE_BIT_AGPM_IS_SUPPORTED)) {
                throw new IZatFeatureNotSupportedException ("AGPM is not supported");
            }
            mTbmMillis = tbmMillis;
        }
        public long getTbmMillis() {
            return mTbmMillis;
        }
    }

    private class GeofenceMapItem {
        Object mContext;
        int mHWGeofenceId;
        IZatGeofenceService.IZatGeofenceCallback mCallback;

        public GeofenceMapItem(Object context,
                               int geofenceId,
                               IZatGeofenceService.IZatGeofenceCallback callback)
        {
            mContext = context;
            mHWGeofenceId = geofenceId;
            mCallback = callback;
        }

        public Object getContext() {
            return mContext;
        }
        public int getHWGeofenceId() {
            return mHWGeofenceId;
        }
        public IZatGeofenceService.IZatGeofenceCallback getCallback() {
            return mCallback;
        }
    }

    private class MaxPowerAllocatedCallbackWrapper
        extends IMaxPowerAllocatedCallback.Stub {

        IZatTestService.IFlpMaxPowerAllocatedCallback mCallback;
        public MaxPowerAllocatedCallbackWrapper(
            IZatTestService.IFlpMaxPowerAllocatedCallback callback) {
            mCallback = callback;
        }
        public void onMaxPowerAllocatedChanged(double power_mW) {
            if (null == mCallback) {
                Log.w(TAG, "mCallback is NULL in MaxPowerAllocatedCallbackWrapper");
                return;
            }
            mCallback.onMaxPowerAllocatedChanged(power_mW);
        }
    }

    private class GeofenceStatusCallbackWrapper
        extends IGeofenceCallback.Stub {

        public void onTransitionEvent(int geofenceHwId,
                                      int event,
                                      Location location) {
            if (VERBOSE) {
                Log.d(TAG, "onTransitionEvent - geofenceHwId is " +
                      geofenceHwId + "; event is " + event);
            }
            IZatDataValidation.dataValidate(location);
            IZatDataValidation.dataValidate(event, IZatDataTypes.GEO_EVENT);
            // find the callback through geofence hw id
            synchronized (sGeofencesMapLock) {
                GeofenceMapItem mapItem = null;
                for (IZatGeofenceServiceImpl.IZatGeofenceHandleImpl key :
                        mGeofencesMap.keySet()) {
                    mapItem = mGeofencesMap.get(key);
                    if (mapItem.getHWGeofenceId() == geofenceHwId) {
                        mapItem.getCallback().onTransitionEvent(key, event, location);
                        return;
                    }
                }
            }
        }

        public void onRequestResultReturned(int geofenceHwId,
                                            int requestType,
                                            int result) {
            if (VERBOSE) {
                Log.d(TAG, "onRequestResultReturned - geofenceHwId is " +
                      geofenceHwId + "; requestType is " + requestType +
                      "; result is " + result);
            }
            IZatDataValidation.dataValidate(requestType, IZatDataTypes.GEO_REQUEST_TYPE);
            IZatDataValidation.dataValidate(result, IZatDataTypes.GEO_ERROR_CODE);
            if (result == IZatGeofenceService.GEOFENCE_RESULT_SUCCESS) {
                return;
            }
            // find the callback through geofence hw id
            synchronized (sGeofencesMapLock) {
                GeofenceMapItem mapItem = null;
                for (IZatGeofenceServiceImpl.IZatGeofenceHandleImpl key :
                        mGeofencesMap.keySet()) {
                    mapItem = mGeofencesMap.get(key);
                    if (mapItem.getHWGeofenceId() == geofenceHwId) {
                        if (IZatGeofenceService.GEOFENCE_REQUEST_TYPE_ADD == requestType) {
                            mGeofencesMap.remove(key);
                        }
                        mapItem.getCallback().onRequestFailed(key, requestType, result);
                        return;
                    }
                }
            }
        }

        public void onEngineReportStatus(int status,
                                         Location location) {
            if (VERBOSE) {
                Log.d(TAG, "onEngineReportStatus - status is " + status);
            }
            IZatDataValidation.dataValidate(location);
            IZatDataValidation.dataValidate(status, IZatDataTypes.GEO_ENGINE_STATUS);
            // broadcast to all clients
            synchronized (sGeofenceClientCallbackMapLock) {
                for (IZatGeofenceServiceImpl key : mGeofenceClientCallbackMap.keySet()) {
                    mGeofenceClientCallbackMap.get(key).onEngineReportStatus(status,
                                                                             location);
                }
            }
        }
    }

    private class DebugReportCallbackWrapper
        extends IDebugReportCallback.Stub {

        public void onDebugDataAvailable(Bundle debugReport) {
            if (VERBOSE) {
                Log.v(TAG, "onDebugDataAvailable");
            }
            synchronized(sDebugReportCallbackMapLock) {
                // broadcast to all clients
                for (IZatDebugReportingServiceImpl key : mDebugReportClientCallbackMap.keySet()) {
                    mDebugReportClientCallbackMap.get(key).onDebugReportAvailable(debugReport);
                }
            }
        }
    }

    private class WiFiDBReceiverRespListenerWrapper extends IWiFiDBReceiverResponseListener.Stub {

        // Decprecated, for backwards compatibility with legacy SDK
        public void onAPListAvailable(List<APInfo> ap_info) {
            if (VERBOSE) {
                Log.d(TAG, "onAPListAvailable");
            }
            if(null != mWiFiDBRecImpl) {
                List<IZatAPInfo> apInfo = new ArrayList<IZatAPInfo>();
                for(APInfo ap: ap_info) {
                    IZatCellInfo cellInfo = null;

                    if (!(ap.mCellRegionID1 == 0 && ap.mCellRegionID2 == 0 && ap.mCellRegionID3 == 0
                            && ap.mCellRegionID4 == 0)) {
                        cellInfo = new IZatCellInfo(ap.mCellRegionID1,
                                ap.mCellRegionID2,
                                ap.mCellRegionID3,
                                ap.mCellRegionID4,
                                ap.mCellType);
                    }
                    IZatAPSSIDInfo ssidInfo = null;
                    if(null != ap.mSSID && ap.mSSID.length > 0) {
                        ssidInfo = new IZatAPSSIDInfo(ap.mSSID, (short)ap.mSSID.length);
                    }
                    apInfo.add(new IZatAPInfo(ap.mMacAddress, 0,
                            new IZatAPInfoExtra(cellInfo, ssidInfo)));
                }

                if (VERBOSE) {
                    log(apInfo, null, null);
                }
                for(IZatAPInfo item : apInfo) {
                    IZatDataValidation.dataValidate(item);
                }

                if (null != mWiFiDBRecImpl.mResponseListener) {
                    mWiFiDBRecImpl.mResponseListener.onAPListAvailable(apInfo);
                }
            }
        }

        private IZatAPInfo iZatAPInfoFromAPInfoExt(APInfoExt ap) {
            IZatCellInfo cellInfo = null;

            if (!(ap.mCellRegionID1 == 0 && ap.mCellRegionID2 == 0 && ap.mCellRegionID3 == 0
                    && ap.mCellRegionID4 == 0)) {
                cellInfo = new IZatCellInfo(ap.mCellRegionID1,
                        ap.mCellRegionID2,
                        ap.mCellRegionID3,
                        ap.mCellRegionID4,
                        ap.mCellType);
            }
            IZatAPSSIDInfo ssidInfo = null;
            if (null != ap.mSSID && ap.mSSID.length > 0) {
                ssidInfo = new IZatAPSSIDInfo(ap.mSSID, (short)ap.mSSID.length);
            }

            IZatAPInfoExtra extra = null;
            if (null != cellInfo && null != ssidInfo) {
               extra = new IZatAPInfoExtra(cellInfo, ssidInfo);
            }

            return new IZatAPInfo(ap.mMacAddress, ap.mTimestamp, extra);
        }

        public void onAPListAvailableExt(List<APInfoExt> ap_info, int status, Location location) {
            if (VERBOSE) {
                Log.d(TAG, "onAPListAvailable");
            }
            if(null != mWiFiDBRecImpl) {
                List<IZatAPInfo> iZatApInfoList = new ArrayList<IZatAPInfo>();
                for(APInfoExt ap: ap_info) {
                    IZatAPInfo iZatAPInfo = iZatAPInfoFromAPInfoExt(ap);
                    iZatApInfoList.add(iZatAPInfo);
                }

                if (VERBOSE) {
                    log(iZatApInfoList, IZatApBsListStatus.fromInt(status), location);
                }
                for(IZatAPInfo item : iZatApInfoList) {
                    IZatDataValidation.dataValidate(item);
                }

                if (null != mWiFiDBRecImpl.mResponseListenerExt) {
                    mWiFiDBRecImpl.mResponseListenerExt.onAPListAvailable(iZatApInfoList,
                            IZatApBsListStatus.fromInt(status), location);
                }
            }
        }

        public void onLookupRequest(List<APInfoExt> ap_info, Location location) {
            if (VERBOSE) {
                Log.d(TAG, "onLookupRequest");
            }
            if(null != mWiFiDBRecImpl) {
                List<IZatAPInfo> iZatApInfoList = new ArrayList<IZatAPInfo>();
                for(APInfoExt ap: ap_info) {
                    IZatAPInfo iZatAPInfo = iZatAPInfoFromAPInfoExt(ap);
                    if (VERBOSE) {
                        Log.d(TAG, "onLookupRequest ap.mFdalStatus: " + ap.mFdalStatus);
                    }
                    iZatAPInfo.setFdalStatus(ap.mFdalStatus);
                    iZatApInfoList.add(iZatAPInfo);
                }

                if (VERBOSE) {
                    log(iZatApInfoList, IZatApBsListStatus.STD_FINAL, location);
                }
                for(IZatAPInfo item : iZatApInfoList) {
                    IZatDataValidation.dataValidate(item);
                }
                IZatDataValidation.dataValidate(location);
                if (null != mWiFiDBRecImpl.mResponseListenerExt) {
                    mWiFiDBRecImpl.mResponseListenerExt.onLookupRequest(iZatApInfoList, location);
                }
            }
        }

        public void onStatusUpdate(boolean is_success, String error) {
            if (VERBOSE) {
                Log.d(TAG, "onStatusUpdate");
            }
            if(null != mWiFiDBRecImpl) {
                if (null != mWiFiDBRecImpl.mResponseListenerExt) {
                    mWiFiDBRecImpl.mResponseListenerExt.onStatusUpdate(is_success, error);
                } else if (null != mWiFiDBRecImpl.mResponseListener) {
                    mWiFiDBRecImpl.mResponseListener.onStatusUpdate(is_success, error);
                }

            }
        }
        public void onServiceRequest() {
            if (VERBOSE) {
                Log.d(TAG, "onServiceRequest");
            }
            if(null != mWiFiDBRecImpl) {
                if (null != mWiFiDBRecImpl.mResponseListenerExt) {
                    mWiFiDBRecImpl.mResponseListenerExt.onServiceRequest();
                } else if (null != mWiFiDBRecImpl.mResponseListener) {
                    mWiFiDBRecImpl.mResponseListener.onServiceRequest();
                }
            }
        }
    }

    private class WiFiDBProviderRespListenerWrapper extends IWiFiDBProviderResponseListener.Stub {

        public void onApObsLocDataAvailable(List<APObsLocData> apObsList, int apStatus) {
            if (VERBOSE) {
                Log.d(TAG, "onApObsLocDataAvailable");
            }
            if(null != mWiFiDBProvImpl) {
                List<IZatAPObsLocData> iZatAPObsList = new ArrayList<IZatAPObsLocData>();

                for(APObsLocData apObs: apObsList) {
                    List<IZatAPScan> mApScanList = new ArrayList<IZatAPScan>();
                    for (APScan apScan: apObs.mScanList) {
                        IZatAPSSIDInfo ssidInfo = null;
                        if(null != apScan.mSSID && apScan.mSSID.length > 0) {
                            ssidInfo = new IZatAPSSIDInfo(apScan.mSSID, (short)apScan.mSSID.length);
                        }
                        mApScanList.add(new IZatAPScan(apScan.mMacAddress,
                                apScan.mRssi,
                                apScan.mDeltaTime,
                                ssidInfo,
                                apScan.mChannelNumber));
                    }
                    IZatCellInfo cellInfo = null;

                    if (!(apObs.mCellInfo.mCellRegionID1 == 0 && apObs.mCellInfo.mCellRegionID2 == 0
                            && apObs.mCellInfo.mCellRegionID3 == 0
                            && apObs.mCellInfo.mCellRegionID4 == 0)) {
                        cellInfo = new IZatCellInfo(apObs.mCellInfo.mCellRegionID1,
                                                    apObs.mCellInfo.mCellRegionID2,
                                                    apObs.mCellInfo.mCellRegionID3,
                                                    apObs.mCellInfo.mCellRegionID4,
                                                    apObs.mCellInfo.mCellType);
                    }

                    iZatAPObsList.add(new IZatAPObsLocData(apObs.mLocation,
                                                           cellInfo,
                                                           apObs.mScanTimestamp,
                                                           mApScanList));
                }

                if (VERBOSE) {
                    logAp(iZatAPObsList, IZatApBsListStatus.values()[apStatus]);
                }
                for(IZatAPObsLocData item : iZatAPObsList) {
                    IZatDataValidation.dataValidate(item);
                }

                mWiFiDBProvImpl.mResponseListener.onApObsLocDataAvailable(iZatAPObsList,
                        IZatApBsListStatus.values()[apStatus]);
            }
        }
        public void onServiceRequest() {
            if (VERBOSE) {
                Log.d(TAG, "onServiceRequest");
            }
            if(null != mWiFiDBProvImpl) {
                mWiFiDBProvImpl.mResponseListener.onServiceRequest();
            }
        }
    }

    private class WWANDBReceiverRespListenerWrapper extends IWWANDBReceiverResponseListener.Stub {
        // Decprecated, for backwards compatibility with static linked SDK
        public void onBSListAvailable(List<BSInfo> bs_info) {
            if (VERBOSE) {
                Log.d(TAG, "onBSListAvailable legacy");
            }
            if(null != mWWANDBRecImpl) {


                List<IZatBSInfo> cellInfoList = new ArrayList<IZatBSInfo>();
                for(BSInfo bs: bs_info) {
                    cellInfoList.add(new IZatBSInfo(new IZatCellInfo(bs.mCellRegionID1,
                            bs.mCellRegionID2,
                            bs.mCellRegionID3,
                            bs.mCellRegionID4,
                            bs.mCellType)));
                }
                for(IZatBSInfo item : cellInfoList) {
                    IZatDataValidation.dataValidate(item);
                }
                if (null != mWWANDBRecImpl.mResponseListener) {
                    mWWANDBRecImpl.mResponseListener.onBSListAvailable(cellInfoList);
                }
            }
        }

        public void onBSListAvailableExt(List<BSInfo> bs_info, int status, Location location) {
            if (VERBOSE) {
                Log.d(TAG, "onBSListAvailable");
            }
            if(null != mWWANDBRecImpl) {
                List<IZatCellInfo> cellInfoList = new ArrayList<IZatCellInfo>();
                for(BSInfo bs: bs_info) {
                    IZatCellInfo cell = new IZatCellInfo(bs.mCellRegionID1,
                                                         bs.mCellRegionID2,
                                                         bs.mCellRegionID3,
                                                         bs.mCellRegionID4,
                                                         bs.mCellType);
                    cell.setCellLocalTimestamp(bs.mCellLocalTimestamp);
                    cellInfoList.add(cell);
                }
                for(IZatCellInfo item : cellInfoList) {
                    IZatDataValidation.dataValidate(item);
                }

                if (null != mWWANDBRecImpl.mResponseListenerExt) {
                    mWWANDBRecImpl.mResponseListenerExt.onBSListAvailable(cellInfoList,
                            IZatApBsListStatus.fromInt(status), location);
                }
            }
        }
        public void onStatusUpdate(boolean is_success, String error) {
            if (VERBOSE) {
                Log.d(TAG, "onStatusUpdate");
            }
            if (null != mWWANDBRecImpl) {
                if (null != mWWANDBRecImpl.mResponseListenerExt) {
                    mWWANDBRecImpl.mResponseListenerExt.onStatusUpdate(is_success, error);
                } else if (null != mWWANDBRecImpl.mResponseListener) {
                    mWWANDBRecImpl.mResponseListener.onStatusUpdate(is_success, error);
                }
            }
        }
        public void onServiceRequest() {
            if (VERBOSE) {
                Log.d(TAG, "onServiceRequest");
            }
            if (null != mWWANDBRecImpl) {
                if (null != mWWANDBRecImpl.mResponseListenerExt) {
                    mWWANDBRecImpl.mResponseListenerExt.onServiceRequest();
                } else if (null != mWWANDBRecImpl.mResponseListener) {
                    mWWANDBRecImpl.mResponseListener.onServiceRequest();
                }
            }
        }
    }

    private class GnssConfigCallbackWrapper
                  extends IGnssConfigCallback.Stub {

        public void getGnssSvTypeConfigCb(int[] disabledSvTypeArr) {
            if (VERBOSE) {
                Log.v(TAG, "getGnssSvTypeConfigCb");
            }
            synchronized(sGnssConfigCallbackMapLock) {
                // broadcast to all clients
                for (IZatGnssConfigServiceImpl key : mGnssConfigClientCallbackMap.keySet()) {
                    // Convert List<Integer> to List<IzatGnssSvType>
                    Set<IzatGnssSvType> enabledSvTypeSet = new HashSet<IzatGnssSvType>();
                    Set<IzatGnssSvType> disabledSvTypeSet = new HashSet<IzatGnssSvType>();
                    for (int svType : disabledSvTypeArr) {
                        IzatGnssSvType izatSvType = IzatGnssSvType.fromInt(svType);
                        if (null != izatSvType) {
                            disabledSvTypeSet.add(izatSvType);
                        } else {
                            izatSvType = IzatGnssSvType.fromInt((~svType) & 0xFF);
                            if (null != izatSvType) {
                                enabledSvTypeSet.add(izatSvType);
                            } else {
                                Log.e(TAG, "Invalid sv type: " + svType);
                            }
                        }
                    }
                    mGnssConfigClientCallbackMap.get(key).getGnssSvTypeConfigCb(
                            enabledSvTypeSet, disabledSvTypeSet);
                }
            }
        }
    }

    private class WWANDBProviderRespListenerWrapper extends IWWANDBProviderResponseListener.Stub {
        @Override
        public void onBsObsLocDataAvailable(List<BSObsLocationData> bsObsList, int bsStatus)
                throws RemoteException {
            if (VERBOSE) {
                Log.d(TAG, "onBsObsLocDataAvailable");
            }
            if(null != mWWANDBProvImpl) {
                List<IZatBSObsLocationData> izatBsObsList = new ArrayList<IZatBSObsLocationData>();
                for(BSObsLocationData bs: bsObsList) {
                    IZatCellInfo cellInfo = new IZatCellInfo(bs.mCellInfo.mCellRegionID1,
                                                             bs.mCellInfo.mCellRegionID2,
                                                             bs.mCellInfo.mCellRegionID3,
                                                             bs.mCellInfo.mCellRegionID4,
                                                             bs.mCellInfo.mCellType);
                    cellInfo.setCellLocalTimestamp(bs.mCellInfo.mCellLocalTimestamp);
                    IZatBSObsLocationData obs = new IZatBSObsLocationData(cellInfo,
                            bs.mLocation, IZatReliablityTypes.MEDIUM, IZatReliablityTypes.MEDIUM);
                    obs.setTimestamp(bs.mScanTimestamp);
                    izatBsObsList.add(obs);
                }

                if (VERBOSE) {
                    logBs(izatBsObsList,
                            IZatApBsListStatus.fromInt(bsStatus));
                }
                for(IZatBSObsLocationData item : izatBsObsList) {
                    IZatDataValidation.dataValidate(item);
                }

                if (null != mWWANDBProvImpl.mResponseListener) {
                    mWWANDBProvImpl.mResponseListener.onBSObsLocDataAvailable(izatBsObsList,
                            IZatApBsListStatus.fromInt(bsStatus));
                }
            }
        }

        @Override
        public void onServiceRequest() {
            if (VERBOSE) {
                Log.d(TAG, "onServiceRequest");
            }
            if (null != mWWANDBProvImpl) {
                if (null != mWWANDBProvImpl.mResponseListener) {
                    mWWANDBProvImpl.mResponseListener.onServiceRequest();
                }
            }
        }
    }


    /* ******************************************************************************** */
    /* Log helper functions                                                             */
    /* ******************************************************************************** */
    private void log(IZatBSLocationData bs_location_data) {
        Log.v(TAG, "Location data: ");
        log(bs_location_data.getLocation());
        Log.v(TAG, "Cell data: " + bs_location_data.getIZatCellInfo());
        log(bs_location_data.getIZatCellInfo());
    }

    private void log(IZatBSSpecialInfo bs_info) {
        Log.v(TAG, "Special info: " + bs_info.getInfo());
        Log.v(TAG, "Cell data: " + bs_info.getCellInfo());
        log(bs_info.getCellInfo());
    }

    private void logBs(List<IZatBSObsLocationData> izatBsObsList,
                       IZatApBsListStatus bsStatus) {
        Log.v(TAG, "BS ObsLocData Available size: " + izatBsObsList.size() + " status:" +
                bsStatus);

        int counter = 1;
        for (IZatBSObsLocationData ap_obs: izatBsObsList) {
            Log.v(TAG, "Entry IZatBSObsLocationData num. " + counter);
            Log.v(TAG, "Location data: ");
            log(ap_obs.getLocation());
            Log.v(TAG, "Cell data: " + ap_obs.getIZatCellInfo());
            log(ap_obs.getIZatCellInfo());
            Log.v(TAG, "Scan data: ");
            Log.v(TAG, "Scan timestamp: " + ap_obs.getTimestamp());
            counter++;
        }
    }

    private void logAp(List<IZatAPObsLocData> ap_obs_list,
                       IZatApBsListStatus ap_obs_status) {
        Log.v(TAG, "Ap ObsLocData Available size: " + ap_obs_list.size() + " status:" +
                ap_obs_status);

        int counter = 1;
        for (IZatAPObsLocData ap_obs: ap_obs_list) {
            Log.v(TAG, "Entry IZatAPObsLocData num. " + counter + " Location data:");
            log(ap_obs.getLocation());
            Log.v(TAG, "Cell data: " + ap_obs.getCellInfo());
            log(ap_obs.getCellInfo());
            Log.v(TAG, "Scan timestamp: " + ap_obs.getScanTimestamp() + "Number of APScan entries: "
                    + ap_obs.getApScanList().size());
            log(ap_obs.getApScanList());
            counter++;
        }
    }

    private void log(List<IZatAPInfo> ap_list,
                     IZatApBsListStatus ap_status,
                     Location location) {

        Log.v(TAG, "AP List size:" + ap_list.size() + " status: " + ap_status);
        log(location);

        for(IZatWiFiDBReceiver.IZatAPInfo ap_info : ap_list) {
            StringBuilder s = new StringBuilder("AP Info mac: ").append(ap_info.getMacAddress()).
                    append("AP Info timestamp: ").append(ap_info.getTimestamp());
            if (ap_info.isExtraAvailable())
            {
                IZatWiFiDBReceiver.IZatAPInfoExtra ap_extra = ap_info.getExtra();
                if (null != ap_extra.getSSID()) {
                    try {
                        s.append("SSID utf8: ").append(new String(ap_extra.getSSID().mSSID, "UTF-8"));
                    } catch (UnsupportedEncodingException e) {
                        s.append("SSID utf8: invalid utf8");
                    }
                } else {
                    s.append("SSID utf8: null");
                }
                log(ap_extra.getCellInfo());
            } else {
                s.append("AP info no extra data available.");
            }
            Log.v(TAG, s.toString());
        }
    }

    private void log(Location location) {
        if (null == location) {
            Log.v(TAG, "Location is null");
        } else {
            Log.v(TAG, "Provider: " + location.getProvider() +
                       "Longitude: " + location.getLongitude() +
                       "Latitude: " + location.getLatitude() +
                       "Altitude: " + (location.hasAltitude() ? location.getAltitude() : "EMPTY") +
                       "Accuracy: " + (location.hasAccuracy() ? location.getAccuracy() : "EMPTY") +
                       "loc. Timestamp: " + location.getTime() +
                       "VerticalAccuracy: " + (location.hasVerticalAccuracy() ?
                        location.getVerticalAccuracyMeters() : "EMPTY") );
        }
    }

    private void log(IZatCellInfo cellInfo) {
        if (null == cellInfo) {
            Log.v(TAG, "cellInfo is null");
        } else {
            StringBuilder s = new StringBuilder("RegionID1: ").append(cellInfo.getRegionID1()).
                    append("RegionID2: ").append(cellInfo.getRegionID2()).
                    append("RegionID3: ").append(cellInfo.getRegionID3()).
                    append("RegionID4: ").append(cellInfo.getRegionID4()).
                    append("Type:      ").append(cellInfo.getType());
            try {
                s.append(cellInfo.getCellLocalTimestamp());
            } catch (IZatStaleDataException e) {
                s.append("Timestamp: invalid");
            }

            Log.v(TAG, s.toString());
        }
    }

    private void log(List<IZatAPScan> ap_scan_list) {
        int counter = 1;
        for (IZatAPScan ap_scan: ap_scan_list) {

            StringBuilder s = new StringBuilder("===  Entry IZatAPScan num. ").append(counter).
                    append("Mac address: ").append(ap_scan.getMacAddress()).
                    append("Rssi: ").append(ap_scan.getRssi());
            if (null != ap_scan.getSSID()) {
                try {
                    s.append("SSID utf8: ").append(new String(ap_scan.getSSID().mSSID, "UTF-8"));
                } catch (UnsupportedEncodingException e) {
                    s.append("SSID utf8: invalid utf8");
                }
            } else {
                s.append("SSID utf8: null");
            }
            s.append("Channel number: ").append(ap_scan.getAPChannelNumber());
            s.append("Delta time:  ").append(ap_scan.getDeltatime());

            Log.v(TAG, s.toString());
            counter++;
        }
    }
/** @endcond */
}
/** @} */  /* end_addtogroup IZatManager */
