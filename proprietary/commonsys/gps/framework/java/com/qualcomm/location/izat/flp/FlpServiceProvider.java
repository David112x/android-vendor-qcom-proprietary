/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2015-2017 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qualcomm.location.izat.flp;

import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.location.Location;
import android.location.LocationManager;
import android.os.IBinder;
import android.os.Binder;
import android.os.RemoteCallbackList;
import android.os.RemoteException;
import android.os.ServiceManager;
import android.os.SystemClock;
import android.os.UserHandle;
import android.util.Log;
import android.util.Pair;
import android.os.Handler;
import android.os.Message;
import android.os.HandlerThread;
import android.os.Looper;

import android.app.AppOpsManager;
import android.app.ActivityManager;
import java.util.LinkedList;
import java.util.Queue;
import java.util.Map;
import java.util.HashMap;
import java.util.List;
import java.util.ArrayList;

import com.qti.flp.*;
import com.qualcomm.location.izat.CallbackData;
import com.qualcomm.location.izat.DataPerPackageAndUser;
import com.qualcomm.location.izat.LocationSettingsHelper;

public class FlpServiceProvider implements LocationSettingsHelper.ILocationSettingsCallback,
                                           LocationSettingsHelper.IUidImportanceCallback,
                                           Handler.Callback {
    private static final String TAG = "FlpServiceProvider";
    private static final boolean VERBOSE = Log.isLoggable(TAG, Log.VERBOSE);
    private static final int FLP_RESULT_SUCCESS = 0;
    private static final int FLP_RESULT_ERROR = -1;
    private static final int LOCATION_REPORT_ON_FULL_INDICATION = 1;
    private static final int LOCATION_REPORT_ON_FIX_INDICATION = 2;
    private static final int LOCATION_REPORT_ON_QUERY = 4;
    private static final int LOCATION_REPORT_ON_INDICATIONS = 8; // for legacy
    private static final int FLP_PASSIVE_LISTENER_HW_ID = -1;
    private static final Object sCallBacksLock = new Object();
    private static final Object sStatusCallbacksLock = new Object();
    private static final Object sLocationSettingsLock = new Object();
    private final Context mContext;
    private final LocationSettingsHelper mLocationSettingsHelper;
    private final DataPerPackageAndUser<FlpUserData> mDataPerPackageAndUser;
    private AppOpsManager mAppOpsMgr;
    private final Looper mLooper;
    private Handler mHandler;

    private static final int FLP_SESSION_BACKGROUND = 1;
    private static final int FLP_SESSION_FOREGROUND = 2;
    private static final int FLP_SESSION_PASSIVE = 4;

    private static final int FLP_BG_NOTIFICATION_ROUTINE = 1;
    private static final int FLP_BG_NOTIFICATION_OUTDOOR_TRIP = 3;

    private static final int BATCHING_STATUS_TRIP_COMPLETED = 0;
    private static final int BATCHING_STATUS_POSITION_AVAILABE = 1;
    private static final int BATCHING_STATUS_POSITION_UNAVAILABLE = 2;

    private static final int BATCHING_MODE_ROUTINE = 0;
    private static final int BATCHING_MODE_OUTDOOR_TRIP = 1;

    private static final int POWER_MODE_INVALID = 0;
    private static final int POWER_MODE_M1 = 0;
    private static final int POWER_MODE_M2 = 0;
    private static final int POWER_MODE_M3 = 0;
    private static final int POWER_MODE_M4 = 0;
    private static final int POWER_MODE_M5 = 0;

    private static final int MSG_UPDATE_HIGHPOWER_MONITOR = 0;
    private static final String ACCESS_BACKGROUND_LOCATION =
            android.Manifest.permission.ACCESS_BACKGROUND_LOCATION;

    // Maximum delta beetwen most recent location in batch and session start time
    private static final int MAX_STARTTIME_DELTA_MS = 60 * 1000;

    private static final long MilliToNano(long millis) {
        return (millis > (Long.MAX_VALUE/1000000)) ? Long.MAX_VALUE : millis*1000000;
    }

    private class BgSessionData {
        private int mBgNotificationType;
        private long mSessionStartTime;

        BgSessionData() {
            mBgNotificationType = FLP_BG_NOTIFICATION_ROUTINE;
            mSessionStartTime = 0;
        }
    }


    // Used for data transfer into native layer.
    // Note the period difference in units: nanoseconds in native layer, milliseconds above.
    private class FlpSessionData {
        private int mId;
        private int mFlags;
        private long mPeriodNs;
        private int mDistanceIntervalMps;
        private long mTripDistanceM;
        private int mPowerMode;
        private long mTbmMs;
        private int mCallingUid;
        private String mCallingPackage;
        private boolean mIsRunning;
        private boolean mIsAllowedInBackground;

        FlpSessionData(int id,
                       int flags,
                       long period_ms,
                       int distance_interval_mps,
                       long trip_distance_m,
                       int power_mode,
                       long tbm_ms,
                       int calling_uid,
                       String calling_package,
                       boolean isAllowedInBackground) {
            mId = id;
            mFlags = flags;
            mPeriodNs = MilliToNano(period_ms);
            mDistanceIntervalMps = distance_interval_mps;
            mTripDistanceM = trip_distance_m;
            mPowerMode = power_mode;
            mTbmMs = tbm_ms;
            mCallingUid = calling_uid;
            mCallingPackage = calling_package;
            mIsRunning = false;
            mIsAllowedInBackground = isAllowedInBackground;
        }
    }

    private RemoteCallbackList<ILocationCallback> mCallbacksForBg
            = new RemoteCallbackList<ILocationCallback>();
    private RemoteCallbackList<ILocationCallback> mCallbacksForFg
            = new RemoteCallbackList<ILocationCallback>();
    private RemoteCallbackList<ILocationCallback> mCallbacksForPassive
            = new RemoteCallbackList<ILocationCallback>();
    private RemoteCallbackList<IMaxPowerAllocatedCallback> mMaxPowerCallbacks
            = new RemoteCallbackList<IMaxPowerAllocatedCallback>();
    private RemoteCallbackList<ISessionStatusCallback> mCallbacksForStatus
            = new RemoteCallbackList<ISessionStatusCallback>();

    // Used to store FLP data related to each user.
    private class FlpUserData extends CallbackData {
        private Map<Long, BgSessionData> mBgSessionMap = new HashMap<Long, BgSessionData>();
        private Map<Long, FlpSessionData> mFlpSessionMap =
                new HashMap<Long, FlpSessionData>();

        private Queue<Pair<ILocationCallback,Long>> mQueryCbQueue
                = new LinkedList<Pair<ILocationCallback,Long>>();

        // Backup copy to allow batch register/deregister on the corresponding RemoteCallbackList
        private Map<ILocationCallback, Long> mCallbacksForBgMap =
                new HashMap<ILocationCallback, Long>();
        private Map<ILocationCallback, Long> mCallbacksForFgMap =
                new HashMap<ILocationCallback, Long>();
        private Map<ILocationCallback, Long> mCallbacksForPassiveMap =
                new HashMap<ILocationCallback, Long>();
        private Map<ISessionStatusCallback, Long> mCallbacksForStatusMap
                = new HashMap<ISessionStatusCallback, Long>();

        private List<IMaxPowerAllocatedCallback> mMaxPowerCallbacksList
                = new ArrayList<IMaxPowerAllocatedCallback>();
    }

    // For FLP only 1 instance per user.
    // Will be switched accordingly on user changes.
    private FlpUserData mFlpUserData = new FlpUserData();

    public boolean handleMessage(Message msg) {
        int msgID = msg.what;
        switch(msgID){
            case MSG_UPDATE_HIGHPOWER_MONITOR:
            {
                Log.d(TAG, "MSG_UPDATE_HIGHPOWER_MONITOR");
                ArrayList<Pair<Integer,String>> sList =
                        (ArrayList<Pair<Integer,String>>)msg.obj;
                boolean start = (msg.arg1 == 1) ? true:false;
                updateHighPowerLocationMonitoring(sList, start);
            }
            break;
            default:
                Log.w(TAG, "Unhandled Message " + msg.what);
        }
        return true;
    }

    private static FlpServiceProvider sInstance = null;
    public static FlpServiceProvider getInstance(Context ctx) {
        if (sInstance == null) {
            sInstance = new FlpServiceProvider(ctx);
        }
        return sInstance;
    }
    private int mFlpFeaturMasks = -1;
    private boolean mIsLocationSettingsOn;
    public void onUidImportanceChange(int uid, boolean isImportanceForeground) {
        ArrayList<Pair<Integer, String>> startList = new ArrayList<Pair<Integer, String>>();
        ArrayList<Pair<Integer, String>> stopList = new ArrayList<Pair<Integer, String>>();
        for (FlpSessionData flpSessionData : mFlpUserData.mFlpSessionMap.values()) {
            if (flpSessionData.mCallingUid == uid) {
                Log.d(TAG, "uid: " + uid + " pid: " + flpSessionData.mCallingUid +
                        ", session goes: " + (isImportanceForeground? "foreground": "background"));
                if (flpSessionData.mIsRunning) {
                    //stop background session if without ACCESS_BACKGROUND_LOCATION
                    if (!flpSessionData.mIsAllowedInBackground && !isImportanceForeground) {
                        native_flp_stop_session(flpSessionData.mId);
                        stopList.add(new Pair<Integer, String>(flpSessionData.mCallingUid,
                                    flpSessionData.mCallingPackage));
                        flpSessionData.mIsRunning = false;
                    }
                } else if (isImportanceForeground) {
                    //restart session when it changes to foreground
                    native_flp_start_session(flpSessionData.mId,
                            flpSessionData.mFlags,
                            flpSessionData.mPeriodNs,
                            flpSessionData.mDistanceIntervalMps,
                            flpSessionData.mTripDistanceM,
                            flpSessionData.mPowerMode,
                            flpSessionData.mTbmMs);
                    startList.add(new Pair<Integer, String>(flpSessionData.mCallingUid,
                                flpSessionData.mCallingPackage));
                    flpSessionData.mIsRunning = true;
                }
            }
        }
        if (!stopList.isEmpty()) {
            mHandler.obtainMessage(MSG_UPDATE_HIGHPOWER_MONITOR, 0, 1, stopList).sendToTarget();
        }
        if (!startList.isEmpty()) {
            mHandler.obtainMessage(MSG_UPDATE_HIGHPOWER_MONITOR, 1, 1, startList).sendToTarget();
        }
    }

    private FlpServiceProvider(Context ctx) {
        if (VERBOSE) {
            Log.d(TAG, "FlpServiceProvider construction");
        }
        mContext = ctx;
        if (native_flp_init() != FLP_RESULT_SUCCESS) {
            Log.e(TAG, "native flp init failed");
        }
        mLocationSettingsHelper = LocationSettingsHelper.getInstance(ctx);
        mLocationSettingsHelper.registerForLocationSettingsCallback(this);
        mLocationSettingsHelper.registerUidImportanceCallback(this);
        mIsLocationSettingsOn = mLocationSettingsHelper.mIsLocationSettingsOn;

        mDataPerPackageAndUser = new DataPerPackageAndUser<FlpUserData>(mContext,
                new UserChanged());
        mDataPerPackageAndUser.useCommonPackage();
        mDataPerPackageAndUser.setData(mFlpUserData);

        HandlerThread t = new HandlerThread(FlpServiceProvider.class.getSimpleName());
        t.start();
        mLooper = t.getLooper();
        mHandler = new Handler(mLooper, this);
        mAppOpsMgr = mContext.getSystemService(AppOpsManager.class);
    }

    public void onLocationSettingsChange(boolean locationSettingsIsOn) {
        synchronized (sLocationSettingsLock) {
            mIsLocationSettingsOn = locationSettingsIsOn;
            if (mIsLocationSettingsOn) {
                startFlpSessions();
            } else {
                stopFlpSessions();
            }
        }
    }

    public void updateHighPowerLocationMonitoring(ArrayList<Pair<Integer, String>> sList,
                                                  boolean start) {
        for(Pair<Integer,String> item : sList) {
            if (start) {
                Log.d(TAG, "startOpNoThrow");
                mAppOpsMgr.startOpNoThrow(AppOpsManager.OP_MONITOR_HIGH_POWER_LOCATION,
                                          item.first, item.second);
            } else {
                Log.d(TAG, "finishOp");
                mAppOpsMgr.finishOp(AppOpsManager.OP_MONITOR_HIGH_POWER_LOCATION,
                                    item.first, item.second);
            }
        }
        mContext.sendBroadcastAsUser(
                new Intent(LocationManager.HIGH_POWER_REQUEST_CHANGE_ACTION),
                UserHandle.ALL);
    }

    private void startFlpSessions() {
        ArrayList<Pair<Integer, String>> sList = new ArrayList<Pair<Integer, String>>();
        for (FlpSessionData flpSessionData : mFlpUserData.mFlpSessionMap.values()) {
            if (VERBOSE) {
                Log.d(TAG, "Starting flp session id: " + flpSessionData.mId);
            }
            if (!flpSessionData.mIsRunning) {
                native_flp_start_session(flpSessionData.mId,
                        flpSessionData.mFlags,
                        flpSessionData.mPeriodNs,
                        flpSessionData.mDistanceIntervalMps,
                        flpSessionData.mTripDistanceM,
                        flpSessionData.mPowerMode,
                        flpSessionData.mTbmMs);
                sList.add(new Pair<Integer, String>(flpSessionData.mCallingUid,
                                                    flpSessionData.mCallingPackage));
                flpSessionData.mIsRunning = true;
            }

        }
        mHandler.obtainMessage(MSG_UPDATE_HIGHPOWER_MONITOR, 1, 1, sList).sendToTarget();
    }

    private void stopFlpSessions() {
        ArrayList<Pair<Integer, String>> sList = new ArrayList<Pair<Integer, String>>();
        for (FlpSessionData flpSessionData : mFlpUserData.mFlpSessionMap.values()) {
            if (VERBOSE) {
                Log.d(TAG, "Stoping flp session id: " + flpSessionData.mId);
            }
            if (flpSessionData.mIsRunning) {
                native_flp_stop_session(flpSessionData.mId);
                sList.add(new Pair<Integer, String>(flpSessionData.mCallingUid,
                                                    flpSessionData.mCallingPackage));
                flpSessionData.mIsRunning = false;
            }
        }
        mHandler.obtainMessage(MSG_UPDATE_HIGHPOWER_MONITOR, 0, 1, sList).sendToTarget();
    }

    /* Remote binder */
    private final IFlpService.Stub mBinder = new IFlpService.Stub() {
        public void registerCallback(final int sessionType,
                                     final int id,
                                     final ILocationCallback cb,
                                     final long sessionStartTime) {
            if (VERBOSE) {
                Log.d(TAG, "in IFlpService.Stub(): registerCallback()," +
                           " sessionType is " + sessionType + "; id is " + id +
                           "; sessionStartTime is " + sessionStartTime +
                           "; cb:" + cb);
            }
            if (cb != null) {
                if (mCallbacksForBg != null &&
                    mCallbacksForFg != null &&
                    mCallbacksForPassive != null) {
                    switch (sessionType) {
                        case FLP_SESSION_BACKGROUND:
                            synchronized (sCallBacksLock) {
                                mCallbacksForBg.register(cb, (long) id);
                                mFlpUserData.mCallbacksForBgMap.put(cb, new Long(id));

                                BgSessionData bgSessData;
                                Long idVal = Long.valueOf(id);
                                if (mFlpUserData.mBgSessionMap.containsKey(idVal)) {
                                    bgSessData = mFlpUserData.mBgSessionMap.get(idVal);
                                } else {
                                    bgSessData = new BgSessionData();
                                }

                                bgSessData.mSessionStartTime = sessionStartTime;
                                mFlpUserData.mBgSessionMap.put(idVal, bgSessData);
                            }
                            try {
                                cb.asBinder().linkToDeath(new IBinder.DeathRecipient() {
                                    @Override
                                    public void binderDied() {
                                        synchronized(sCallBacksLock) {
                                            mCallbacksForBg.unregister(cb);
                                            mFlpUserData.mCallbacksForBgMap.remove(cb);
                                            mFlpUserData.mBgSessionMap.remove(Long.valueOf(id));
                                            stopFlpSession(id);
                                        }
                                    }
                                }, 0);
                            } catch (RemoteException e) {
                                throw new RuntimeException("Failed clean up flp sessions", e);
                            }
                            break;
                        case FLP_SESSION_FOREGROUND:
                            synchronized (sCallBacksLock) {
                                mCallbacksForFg.register(cb, sessionStartTime);
                                mFlpUserData.mCallbacksForFgMap.put(cb, sessionStartTime);
                            }
                            try {
                                cb.asBinder().linkToDeath(new IBinder.DeathRecipient() {
                                    @Override
                                    public void binderDied() {
                                        synchronized(sCallBacksLock) {
                                            mCallbacksForFg.unregister(cb);
                                            mFlpUserData.mCallbacksForFgMap.remove(cb);
                                            stopFlpSession(id);
                                        }
                                    }
                                }, 0);
                            } catch (RemoteException e) {
                                throw new RuntimeException("Failed clean up flp sessions", e);
                            }
                            break;
                        case FLP_SESSION_PASSIVE:
                            synchronized (sCallBacksLock) {
                                mCallbacksForPassive.register(cb, sessionStartTime);
                                mFlpUserData.mCallbacksForPassiveMap.put(cb, sessionStartTime);
                            }
                            try {
                                cb.asBinder().linkToDeath(new IBinder.DeathRecipient() {
                                    @Override
                                    public void binderDied() {
                                        synchronized(sCallBacksLock) {
                                            mCallbacksForPassive.unregister(cb);
                                            mFlpUserData.mCallbacksForPassiveMap.remove(cb);
                                            if (id != FLP_PASSIVE_LISTENER_HW_ID) {
                                                stopFlpSession(id);
                                            }
                                        }
                                    }
                                }, 0);
                            } catch (RemoteException e) {
                                throw new RuntimeException("Failed unregister passive cb", e);
                            }
                            break;
                        default:
                            Log.e(TAG, "unknown sessionType");
                            break;
                    }
                } else {
                    Log.e(TAG, "one of the callback list is not created");
                }
            } else {
                Log.e(TAG, "cb is null");
            }
        }

        public void unregisterCallback(final int sessionType,
                                       final ILocationCallback cb) {
            if (VERBOSE) {
                Log.d(TAG, "in IFlpService.Stub(): unregisterCallback(): cb:" + cb);
            }
            if (cb != null) {
                if (mCallbacksForBg != null &&
                    mCallbacksForFg != null &&
                    mCallbacksForPassive != null) {
                    synchronized (sCallBacksLock) {
                        switch (sessionType) {
                            case FLP_SESSION_BACKGROUND:
                                mCallbacksForBg.unregister(cb);
                                mFlpUserData.mCallbacksForBgMap.remove(cb);
                                break;
                            case FLP_SESSION_FOREGROUND:
                                mCallbacksForFg.unregister(cb);
                                mFlpUserData.mCallbacksForFgMap.remove(cb);
                                break;
                            case FLP_SESSION_PASSIVE:
                                mCallbacksForPassive.unregister(cb);
                                mFlpUserData.mCallbacksForPassiveMap.remove(cb);
                                break;
                            default:
                                Log.e(TAG, "unknown sessionType");
                                break;
                        }
                    }
                } else {
                    Log.e(TAG, "one of the callback list is not created");
                }
            } else {
                Log.e(TAG, "cb is null");
            }
        }

        public void registerForSessionStatus(final int id, final ISessionStatusCallback cb) {
            if (VERBOSE) {
                Log.d(TAG, "in IFlpService.Stub(): registerForSessionStatus() cb:" + cb);
            }
            if (cb != null) {
                if (mCallbacksForStatus != null) {
                    synchronized (sStatusCallbacksLock) {
                        mCallbacksForStatus.register(cb, (long) id);
                        mFlpUserData.mCallbacksForStatusMap.put(cb, new Long(id));
                    }
                    try {
                        cb.asBinder().linkToDeath(new IBinder.DeathRecipient() {
                            @Override
                            public void binderDied() {
                                synchronized(sStatusCallbacksLock) {
                                    mCallbacksForStatus.unregister(cb);
                                    mFlpUserData.mCallbacksForStatusMap.remove(cb);
                                }
                            }
                        }, 0);
                    } catch (RemoteException e) {
                        throw new RuntimeException("Failed clean up flp sessions", e);
                    }
                } else {
                      Log.e(TAG, "cb is null");
                }
            }
        }

        public void unregisterForSessionStatus(final ISessionStatusCallback cb) {
            if (VERBOSE) {
                Log.d(TAG, "in FlpService.Stub unregisterForSessionStatus() cb = : " + cb);
            }
            if (cb != null) {
                if (mCallbacksForStatus != null) {
                    synchronized(sStatusCallbacksLock) {
                        mCallbacksForStatus.unregister(cb);
                        mFlpUserData.mCallbacksForStatusMap.remove(cb);
                    }
                }
            } else {
                Log.e(TAG, "cb is null");
            }
        }

        public int getAllSupportedFeatures() {
            if (VERBOSE) {
                Log.d(TAG, "in IFlpService.Stub(): getAllSupportedFeatures()");
            }
            if (mFlpFeaturMasks == -1) {
                mFlpFeaturMasks = native_flp_get_all_supported_features();
            }
            return mFlpFeaturMasks;
        }

        public int startFlpSession(int id,
                                   int flags,
                                   long period_ms,
                                   int distance_interval_mps,
                                   long trip_distance_m) {

            return startFlpSessionWithPower(
                    // No power mode and tbm value
                    id, flags, period_ms, distance_interval_mps,
                    trip_distance_m, POWER_MODE_INVALID, 0);
        }

        public int startFlpSessionWithPower(int id,
                int flags,
                long period_ms,
                int distance_interval_mps,
                long trip_distance_m,
                int power_mode,
                long tbm_ms) {

            if (VERBOSE) {
                Log.d(TAG, "in IFlpService.Stub(): startFlpSession()" +
                           "; id is " + id +
                           "; period_ms is " + period_ms +
                           "; flags is " + flags +
                           "; distance interval is " + distance_interval_mps +
                           "; trip distance is " + trip_distance_m +
                           "; power mode is " + power_mode +
                           "; tbm is " + tbm_ms);
            }

            // Ensure mFlpUserData is from current user.
            // Calls to start from running Apps can arrive before onUserChange
            mFlpUserData = mDataPerPackageAndUser.getData();

            // BgSessionMap only cares about BG sessions i.e. routine / trip
            synchronized(sCallBacksLock) {
                if ((flags == FLP_BG_NOTIFICATION_ROUTINE) ||
                    (flags == FLP_BG_NOTIFICATION_OUTDOOR_TRIP)) {
                    BgSessionData bgSessData = mFlpUserData.mBgSessionMap.get(Long.valueOf(id));

                    if (bgSessData != null) {
                        bgSessData.mBgNotificationType = flags;
                    } else {
                        Log.e(TAG, "No registered callback for this session id.");
                    }
                }
            }

            int calling_uid = Binder.getCallingUid();
            int calling_pid = Binder.getCallingPid();
            String calling_package = mContext.getPackageManager().getNameForUid(calling_uid);
            boolean isAllowedInBackground = mContext.checkPermission(ACCESS_BACKGROUND_LOCATION,
                    calling_pid, calling_uid) == PackageManager.PERMISSION_GRANTED;

            FlpSessionData flpSessionData = new FlpSessionData(id,
                                                               flags,
                                                               period_ms,
                                                               distance_interval_mps,
                                                               trip_distance_m,
                                                               power_mode,
                                                               tbm_ms,
                                                               calling_uid,
                                                               calling_package,
                                                               isAllowedInBackground);
            int result;
            synchronized (sLocationSettingsLock) {
                mFlpUserData.mFlpSessionMap.put(Long.valueOf(id), flpSessionData);
                if (!mIsLocationSettingsOn || flpSessionData.mIsRunning) {
                    if (VERBOSE) {
                        Log.d(TAG, "Location setting is OFF or already running, no need to start.");
                    }
                    result = FLP_RESULT_SUCCESS;
                } else {
                    result = native_flp_start_session(flpSessionData.mId,
                                                      flpSessionData.mFlags,
                                                      flpSessionData.mPeriodNs,
                                                      flpSessionData.mDistanceIntervalMps,
                                                      flpSessionData.mTripDistanceM,
                                                      flpSessionData.mPowerMode,
                                                      flpSessionData.mTbmMs);
                    ArrayList<Pair<Integer, String>> sList = new ArrayList<Pair<Integer, String>>();
                    sList.add(new Pair<Integer, String>(flpSessionData.mCallingUid,
                                                        flpSessionData.mCallingPackage));
                    mHandler.obtainMessage(MSG_UPDATE_HIGHPOWER_MONITOR, 1, 1, sList)
                            .sendToTarget();
                    flpSessionData.mIsRunning = true;
                }
            }

            return result;
        }

        public int updateFlpSession(int id,
                int flags,
                long period_ms,
                int distance_interval_mps,
                long trip_distance_m) {

            return updateFlpSessionWithPower(id, flags, period_ms,
                    distance_interval_mps, trip_distance_m, POWER_MODE_INVALID, 0);
        }

        public int updateFlpSessionWithPower(int id,
                                    int flags,
                                    long period_ms,
                                    int distance_interval_mps,
                                    long trip_distance_m,
                                    int power_mode,
                                    long tbm_ms) {
            if (VERBOSE) {
                Log.d(TAG, "in IFlpService.Stub(): updateFlpSession()" +
                           "; id is " + id +
                           "; period_ms is " + period_ms +
                           "; flags is " + flags +
                           "; distance_ms is " + distance_interval_mps +
                           "; trip distance " + trip_distance_m +
                           "; power mode " + power_mode +
                           "; tbm_ms " + tbm_ms);
            }

            // BgSessionMap only cares about BG sessions i.e. routine / trip
            synchronized(sCallBacksLock) {
                Long idVal = Long.valueOf(id);
                BgSessionData bgSessData = mFlpUserData.mBgSessionMap.get(idVal);

                if ((flags == FLP_BG_NOTIFICATION_ROUTINE) ||
                    (flags == FLP_BG_NOTIFICATION_OUTDOOR_TRIP)) {
                    if (bgSessData != null) {
                        bgSessData.mBgNotificationType = flags;
                    } else {
                        // may be the update is happening from a foreground session,
                        // hence callback will be registered after the update call
                        bgSessData = new BgSessionData();
                        bgSessData.mBgNotificationType = flags;
                        mFlpUserData.mBgSessionMap.put(idVal, bgSessData);
                    }
                }
            }

            if (!mFlpUserData.mFlpSessionMap.containsKey(Long.valueOf(id))) {
                Log.e(TAG, "Invalid FlpSession id: " + id);
                return FLP_RESULT_ERROR;
            }

            int result;
            FlpSessionData flpSessionData = mFlpUserData.mFlpSessionMap.get(Long.valueOf(id));
            synchronized (sLocationSettingsLock) {
                flpSessionData.mId = id;
                flpSessionData.mFlags = flags;
                flpSessionData.mPeriodNs = MilliToNano(period_ms);
                flpSessionData.mDistanceIntervalMps = distance_interval_mps;
                flpSessionData.mTripDistanceM = trip_distance_m;
                flpSessionData.mPowerMode = power_mode;
                flpSessionData.mTbmMs = tbm_ms;

                if (!mIsLocationSettingsOn) {
                    result =  FLP_RESULT_SUCCESS;
                } else {
                    result =  native_flp_update_session(flpSessionData.mId,
                                                        flpSessionData.mFlags,
                                                        flpSessionData.mPeriodNs,
                                                        flpSessionData.mDistanceIntervalMps,
                                                        flpSessionData.mTripDistanceM,
                                                        flpSessionData.mPowerMode,
                                                        flpSessionData.mTbmMs);
                }
            }

            return result;
        }

        public int stopFlpSession(int id) {
            // Ensure mFlpUserData is from current user.
            // Calls to stop from destroyed Apps can arrive before onUserChange
            mFlpUserData = mDataPerPackageAndUser.getData();

            if (VERBOSE) {
                Log.d(TAG, "in IFlpService.Stub(): stopFlpSession(); id is " + id);
            }

            synchronized(sCallBacksLock) {
                mFlpUserData.mBgSessionMap.remove(Long.valueOf(id));
            }

            int result;
            Long idVal = Long.valueOf(id);
            if (!mFlpUserData.mFlpSessionMap.containsKey(idVal)) {
                Log.e(TAG, "Invalid FlpSession id: " + id);
                return FLP_RESULT_ERROR;
            }

            FlpSessionData flpSessionData = mFlpUserData.mFlpSessionMap.get(idVal);
            if (!flpSessionData.mIsRunning) {
                result = FLP_RESULT_SUCCESS;
            } else {
                result = native_flp_stop_session(id);
                ArrayList<Pair<Integer, String>> sList = new ArrayList<Pair<Integer, String>>();
                sList.add(new Pair<Integer, String>(flpSessionData.mCallingUid,
                                                    flpSessionData.mCallingPackage));
                mHandler.obtainMessage(MSG_UPDATE_HIGHPOWER_MONITOR, 0, 1, sList)
                            .sendToTarget();
            }

            mFlpUserData.mFlpSessionMap.remove(idVal);

            return result;
        }

        public int pullLocations(final ILocationCallback cb,
                                 final long sessionStartTime,
                                 final int id) {
            if (VERBOSE) {
                Log.d(TAG, "in IFlpService.Stub(): pullLocations(), sessionStartTime is "
                           + sessionStartTime);
            }
            if (cb == null) {
                Log.e(TAG, "in IFlpService.Stub(): cb is null.");
                return FLP_RESULT_ERROR;
            }

            if (!mFlpUserData.mFlpSessionMap.containsKey(Long.valueOf(id))) {
                Log.e(TAG, "Invalid FlpSession id: " + id);
                return FLP_RESULT_ERROR;
            }

            int result;
            synchronized (sLocationSettingsLock) {
                if (!mIsLocationSettingsOn) {
                    result = FLP_RESULT_SUCCESS;
                } else {
                    synchronized (sCallBacksLock) {
                        // save the cb
                        mFlpUserData.mQueryCbQueue.add(
                                new Pair<ILocationCallback, Long>(cb, sessionStartTime));
                    }
                    result = native_flp_get_all_locations(id);
                }
            }

            return result;
        }
    };

    /* Remote binder */
    private final ITestService.Stub mTestingBinder = new ITestService.Stub() {
        public void deleteAidingData(long flags) {
            if (VERBOSE) {
                Log.d(TAG, "in IFlpService.Stub(): deleteAidingData(). Flags: " + flags);
            }
            native_flp_delete_aiding_data(flags);
        }

        public void updateXtraThrottle(boolean enabled) {
            if (VERBOSE) {
                Log.d(TAG, "in IFlpService.Stub(): updateXtraThrottle(). enabled: " + enabled);
            }
            native_flp_update_xtra_throttle(enabled);
        }

        public void registerMaxPowerChangeCallback(final IMaxPowerAllocatedCallback cb) {
            if (VERBOSE) {
                Log.d(TAG, "in IFlpService.Stub(): registerMaxPowerChangeCallback()");
            }
            if (cb != null) {
                if (mMaxPowerCallbacks != null) {
                    mMaxPowerCallbacks.register(cb);
                    mFlpUserData.mMaxPowerCallbacksList.add(cb);

                    try {
                        cb.asBinder().linkToDeath(new IBinder.DeathRecipient() {
                            @Override
                            public void binderDied() {
                                mMaxPowerCallbacks.unregister(cb);
                                mFlpUserData.mMaxPowerCallbacksList.remove(cb);
                            }
                        }, 0);
                        native_flp_get_max_power_allocated_in_mw();
                    } catch (RemoteException e) {
                        throw new RuntimeException("Failed clean up", e);
                    }
                } else {
                    Log.e(TAG, "mMaxPowerCallbacks is null");
                }
            } else {
                Log.e(TAG, "cb is null");
            }
        }

        public void unregisterMaxPowerChangeCallback(IMaxPowerAllocatedCallback cb) {
            if (VERBOSE) {
                Log.d(TAG, "in IFlpService.Stub(): unregisterMaxPowerChangeCallback()");
            }
            if (cb != null) {
                if (mMaxPowerCallbacks != null) {
                    mMaxPowerCallbacks.unregister(cb);
                } else {
                    Log.e(TAG, "mMaxPowerCallbacks is null");
                }
            } else {
                Log.e(TAG, "cb is null");
            }
        }
    };

    private void onLocationReport(Location[] locations, int reportTrigger, int batchingMode) {
        if (VERBOSE) {
            Log.v(TAG, "entering onLocationReport() reportTrigger is " + reportTrigger +
                       "; Batching Mode is " + batchingMode +
                       "; and the first timestamp is " + locations[0].getTime());
        }
        long elapsedTime = SystemClock.elapsedRealtimeNanos();
        long timeStamp = locations[locations.length - 1].getTime();
        //Populate the elapasedTime (nanosec) for each fix based on the elapsedTime of the last fix
        //and the timestamp (millisec) of each fix
        for (int i = 0; i < locations.length; ++i) {
            locations[i].setElapsedRealtimeNanos(elapsedTime -
                    (timeStamp - locations[i].getTime()) * 1000000L);
        }

        if (reportTrigger == LOCATION_REPORT_ON_FULL_INDICATION) {
            // Broadcast to all batching callbacks the new value.
            synchronized (sCallBacksLock) {
                int index = mCallbacksForBg.beginBroadcast();
                for (int i = 0; i < index; i++) {
                    try {
                        Long sessionId = (Long) mCallbacksForBg.getBroadcastCookie(i);
                        BgSessionData bgSessData = mFlpUserData.mBgSessionMap.get(sessionId);
                        if (bgSessData == null) {
                            Log.e(TAG, "No Background session data found for id:" + index);
                            continue;
                        }
                        long sessionStartTime = (long) bgSessData.mSessionStartTime;
                        if (sessionStartTime >
                                 locations[locations.length-1].getTime() + MAX_STARTTIME_DELTA_MS) {
                            if (VERBOSE) {
                                Log.v(TAG, "Future start time detected, returning whole batch");
                            }
                            sessionStartTime = locations[0].getTime();
                        }

                        if (VERBOSE) {
                            Log.v(TAG, "in the mCallbacksForBg loop : " + i +
                                "; timestamp is " + sessionStartTime +
                                "; notification Type is " + bgSessData.mBgNotificationType);
                        }
                        if (((bgSessData.mBgNotificationType == FLP_BG_NOTIFICATION_ROUTINE) &&
                            (batchingMode == BATCHING_MODE_ROUTINE)) ||
                            (bgSessData.mBgNotificationType == FLP_BG_NOTIFICATION_OUTDOOR_TRIP) &&
                            (batchingMode == BATCHING_MODE_OUTDOOR_TRIP)) {
                            if (sessionStartTime<=locations[0].getTime()) {
                                // return the whole batch
                                if (VERBOSE) {
                                    Log.v(TAG, "return whole batch");
                                }
                                mCallbacksForBg.getBroadcastItem(i).onLocationAvailable(locations);
                            } else if (sessionStartTime > locations[locations.length-1].getTime()) {
                                if (VERBOSE) {
                                    Log.v(TAG, "no need to return");
                                }
                            } else {
                                // find the offset
                                int offset = getOffset(sessionStartTime, locations);
                                Location[] newResult = new Location[locations.length - offset];
                                System.arraycopy(locations,
                                                 offset,
                                                 newResult,
                                                 0,
                                                 locations.length - offset);
                                mCallbacksForBg.getBroadcastItem(i).onLocationAvailable(newResult);
                            }
                        }
                    } catch (RemoteException e) {
                    // The RemoteCallbackList will take care of removing
                    // the dead object.
                    }
                }
                mCallbacksForBg.finishBroadcast();
            }
         } else if (reportTrigger == LOCATION_REPORT_ON_FIX_INDICATION) {
            // Broadcast to all tracking callbacks the new value.
            synchronized (sCallBacksLock) {
                int index = mCallbacksForFg.beginBroadcast();
                for (int i = 0; i < index; i++) {
                    try {
                        long sessionStartTime = (long) mCallbacksForFg.getBroadcastCookie(i);
                        // Handle case of sessionStartTime in the future (device missconfiguration?)
                        if (sessionStartTime >
                                 locations[locations.length-1].getTime() + MAX_STARTTIME_DELTA_MS) {
                            if (VERBOSE) {
                                Log.v(TAG, "Future start time detected, returning whole batch");
                            }
                            sessionStartTime = locations[0].getTime();
                        }
                        if (VERBOSE) {
                            Log.v(TAG, "in the mCallbacksForFg loop : " + i
                                       + "; cd timestamp is" + sessionStartTime);
                        }
                        if (sessionStartTime <= locations[0].getTime()) {
                            // return the whole batch
                            if (VERBOSE) {
                                Log.v(TAG, "return whole batch");
                            }
                            mCallbacksForFg.getBroadcastItem(i).onLocationAvailable(locations);
                        } else if (sessionStartTime>locations[locations.length-1].getTime()) {
                            if (VERBOSE) {
                                Log.v(TAG, "no need to return");
                            }
                        } else {
                            // find the offset
                            int offset = getOffset(sessionStartTime, locations);
                            Location[] newResult = new Location[locations.length - offset];
                            System.arraycopy(locations,
                                             offset,
                                             newResult,
                                             0,
                                             locations.length - offset);
                            mCallbacksForFg.getBroadcastItem(i).onLocationAvailable(newResult);
                        }
                    } catch (RemoteException e) {
                        // The RemoteCallbackList will take care of removing
                        // the dead object.
                    }
                }
                mCallbacksForFg.finishBroadcast();
            }
        } else if (reportTrigger == LOCATION_REPORT_ON_QUERY) {
            synchronized (sCallBacksLock) {
                if (!mFlpUserData.mQueryCbQueue.isEmpty()) {
                    Pair<ILocationCallback, Long> cbPair = mFlpUserData.mQueryCbQueue.remove();
                    if (cbPair != null) {
                        try {
                            // check the timestamp, find the offset
                            ILocationCallback callback = cbPair.first;
                            long sessionStartTime = cbPair.second;
                            if (VERBOSE) {
                                Log.v(TAG, "calling callback for" +
                                           " pulling, sessionStartTime is " +
                                            sessionStartTime);
                            }
                            if (sessionStartTime <= locations[0].getTime()) {
                                // return the whole batch
                                if (VERBOSE) {
                                    Log.v(TAG, "return whole batch");
                                }
                                callback.onLocationAvailable(locations);
                            } else if (sessionStartTime>locations[locations.length-1].getTime()) {
                                if (VERBOSE) {
                                    Log.v(TAG, "no need to return");
                                }
                            } else {
                                int offset = getOffset(sessionStartTime, locations);
                                Location[] newResult = new Location[locations.length - offset];
                                System.arraycopy(locations,
                                                 offset,
                                                 newResult,
                                                 0,
                                                 locations.length - offset);
                                callback.onLocationAvailable(newResult);
                            }
                            // update the timestamp of the callback
                            Long sessionId = mFlpUserData.mCallbacksForBgMap.get(callback);
                            BgSessionData bgSessData = mFlpUserData.mBgSessionMap.get(sessionId);
                            if (bgSessData != null) {
                                bgSessData.mSessionStartTime = System.currentTimeMillis();
                            }
                            if (mCallbacksForFg.unregister(callback)) {
                                mCallbacksForFg.register(callback, System.currentTimeMillis());
                                mFlpUserData.mCallbacksForFgMap.put(callback,
                                        System.currentTimeMillis());
                            }
                            if (mCallbacksForPassive.unregister(callback)) {
                                mCallbacksForPassive.register(callback, System.currentTimeMillis());
                                mFlpUserData.mCallbacksForPassiveMap.put(callback,
                                        System.currentTimeMillis());
                            }
                        } catch (RemoteException e) {
                            // The RemoteCallbackList will take care of removing
                            // the dead object.
                        }
                    }
                } else {
                    Log.e(TAG, "no available callback on query");
                }
            }
        } else if (reportTrigger == LOCATION_REPORT_ON_INDICATIONS) {
            /*
               For legacy behaviour, the callback are in the passive
               listeners already, so do nothing here.
            */
        } else {
            Log.e(TAG, "unknown reportTrigger");
        }

        // passive listeners
        if (mCallbacksForPassive.getRegisteredCallbackCount() > 0) {
            // Broadcast to all passive listeners
            synchronized (sCallBacksLock) {
                int index = mCallbacksForPassive.beginBroadcast();
                for (int i = 0; i < index; i++) {
                    try {
                        long sessionStartTime = (long) mCallbacksForPassive.getBroadcastCookie(i);
                        // Handle case of sessionStartTime in the future (device missconfiguration?)
                        if (sessionStartTime >
                                 locations[locations.length-1].getTime() + MAX_STARTTIME_DELTA_MS) {
                            if (VERBOSE) {
                                Log.v(TAG, "Future start time detected, returning whole batch");
                            }
                            sessionStartTime = locations[0].getTime();
                        }
                        if (VERBOSE) {
                            Log.v(TAG, "in the mCallbacksForPassive loop : " + i
                                       + "; cd timestamp is" + sessionStartTime);
                        }
                        if (sessionStartTime<=locations[0].getTime()) {
                            // return the whole batch
                            if (VERBOSE) {
                                Log.v(TAG, "return whole batch");
                            }
                            mCallbacksForPassive.getBroadcastItem(i).onLocationAvailable(locations);
                        } else if (sessionStartTime>locations[locations.length-1].getTime()) {
                            if (VERBOSE) {
                                Log.v(TAG, "no need to return");
                            }
                        } else {
                            // find the offset
                            int offset = getOffset(sessionStartTime, locations);
                            Location[] newResult = new Location[locations.length - offset];
                            System.arraycopy(locations,
                                             offset,
                                             newResult,
                                             0,
                                             locations.length - offset);
                            mCallbacksForPassive.getBroadcastItem(i).onLocationAvailable(newResult);
                        }
                    } catch (RemoteException e) {
                        // The RemoteCallbackList will take care of removing
                        // the dead object.
                    }
                }
                mCallbacksForPassive.finishBroadcast();
            }
        }
    }

    private void onBatchingStatusCb(int batchingStatus, int[] completedTripClientIds) {
        if (VERBOSE) {
            Log.d(TAG, "entering onBatchingStatusCb batchingStatus :" + batchingStatus);
        }
        synchronized(sStatusCallbacksLock) {
            List<Long> completedTripClientList = null;

            if (completedTripClientIds != null) {
                completedTripClientList = new ArrayList<Long>(completedTripClientIds.length);
                for (int index = 0; index < completedTripClientIds.length; index++) {
                    completedTripClientList.add(Long.valueOf(completedTripClientIds[index]));
                }
            }

            int broadcastCount = mCallbacksForStatus.beginBroadcast();
            for (int broadcastIndex = 0; broadcastIndex < broadcastCount; broadcastIndex++) {
                try {
                    if (batchingStatus != BATCHING_STATUS_TRIP_COMPLETED) {
                        mCallbacksForStatus.getBroadcastItem(broadcastIndex).onBatchingStatusCb(
                                batchingStatus);
                    } else {
                        Long sessionId =
                                (Long) mCallbacksForStatus.getBroadcastCookie(broadcastIndex);
                        if ((completedTripClientList != null) &&
                                (completedTripClientList.contains(sessionId))) {
                            mCallbacksForStatus.getBroadcastItem(broadcastIndex).onBatchingStatusCb(
                                    batchingStatus);
                            mFlpUserData.mFlpSessionMap.remove(sessionId);
                        }
                    }
                } catch(RemoteException e) {
                  // The RemoteCallbackList will take care of removing
                  // the dead object.
                }
            }
            mCallbacksForStatus.finishBroadcast();
        }
    }

    private void onMaxPowerAllocatedChanged(int power_mW) {
        if (VERBOSE) {
            Log.d(TAG, "entering onMaxPowerAllocatedChanged() power: " + power_mW);
        }
        // Broadcast to all clients the new value.
        int index = mMaxPowerCallbacks.beginBroadcast();
        for (int i = 0; i < index; i++) {
            if (VERBOSE) {
                Log.v(TAG, "in the mMaxPowerCallbacks loop : " + i);
            }
            try {
                IMaxPowerAllocatedCallback cb =
                    mMaxPowerCallbacks.getBroadcastItem(i);
                cb.onMaxPowerAllocatedChanged((double)power_mW);
            } catch (RemoteException e) {
                // The RemoteCallbackList will take care of removing
                // the dead object.
            }
        }
        mMaxPowerCallbacks.finishBroadcast();
    }

    class UserChanged implements DataPerPackageAndUser.UserChangeListener<FlpUserData> {
        @Override
        public void onUserChange(Map<String, FlpUserData> prevUserData,
                                 Map<String, FlpUserData> currentUserData) {
            if (VERBOSE) {
                Log.d(TAG, "Active user has changed, updating FLP callbacks...");
            }

            synchronized (sLocationSettingsLock) {
                // New user switched may be first time running
                if (currentUserData.isEmpty()) {
                    Log.d(TAG, "FLP data for new user is empty, creating new instance.");
                    // mDataPerPackageAndUser.setData would dead-lock in this case
                    currentUserData.put(mDataPerPackageAndUser.getPackageName(null),
                            new FlpUserData());
                }
                FlpUserData currentFlpUserData =
                        currentUserData.get(mDataPerPackageAndUser.getPackageName(null)) ;

                FlpUserData prevFlpUserData =
                        prevUserData.get(mDataPerPackageAndUser.getPackageName(null)) ;
                // mFlpUserData may have been updated if start/stop was called before
                mFlpUserData = prevFlpUserData;

                synchronized (sCallBacksLock) {
                    // Unregister all previous callbacks
                    for (ILocationCallback callback : mFlpUserData.mCallbacksForFgMap.keySet()) {
                        mCallbacksForFg.unregister(callback);
                    }

                    for (ILocationCallback callback : mFlpUserData.mCallbacksForBgMap.keySet()) {
                        mCallbacksForBg.unregister(callback);
                    }

                    for (ILocationCallback callback :
                            mFlpUserData.mCallbacksForPassiveMap.keySet()) {
                        mCallbacksForPassive.unregister(callback);
                    }
                }

                synchronized (sStatusCallbacksLock) {
                    for (ISessionStatusCallback callback :
                            mFlpUserData.mCallbacksForStatusMap.keySet()) {
                        mCallbacksForStatus.unregister(callback);
                    }
                }

                for (IMaxPowerAllocatedCallback callback : mFlpUserData.mMaxPowerCallbacksList) {
                    mMaxPowerCallbacks.unregister(callback);
                }

                // Remove any pending callback queue. Wont make sense any more after user switch
                mFlpUserData.mQueryCbQueue.clear();

                if (mIsLocationSettingsOn) {
                    stopFlpSessions();
                }

                // Update to new current user
                mFlpUserData = currentFlpUserData;

                if (mIsLocationSettingsOn) {
                    startFlpSessions();
                }

                synchronized (sCallBacksLock) {
                    // Re-register all previous callbacks
                    for (ILocationCallback callback : mFlpUserData.mCallbacksForFgMap.keySet()) {
                        mCallbacksForFg.register(callback,
                                mFlpUserData.mCallbacksForFgMap.get(callback));
                    }

                    for (ILocationCallback callback : mFlpUserData.mCallbacksForBgMap.keySet()) {
                        mCallbacksForBg.register(callback,
                                mFlpUserData.mCallbacksForBgMap.get(callback));
                    }

                    for (ILocationCallback callback :
                            mFlpUserData.mCallbacksForPassiveMap.keySet()) {
                        mCallbacksForPassive.register(callback,
                                mFlpUserData.mCallbacksForPassiveMap.get(callback));
                    }
                }

                synchronized (sStatusCallbacksLock) {
                    for (ISessionStatusCallback callback :
                            mFlpUserData.mCallbacksForStatusMap.keySet()) {
                        mCallbacksForStatus.register(callback,
                                mFlpUserData.mCallbacksForPassiveMap.get(callback));
                    }
                }

                for (IMaxPowerAllocatedCallback callback : mFlpUserData.mMaxPowerCallbacksList) {
                    mMaxPowerCallbacks.register(callback);
                }
            }
        }

    }

    private int getOffset (long sessionStartTime, Location[] locations) {
        int offset = -1, left = 0, right = locations.length-1;
        while(left <= right) {
            int mid = (left+right)/2;
            // found the exactly match
            if (sessionStartTime ==
                locations[mid].getTime()) {
                offset = mid;
                break;
            }
            if (sessionStartTime >
                locations[mid].getTime()) {
                left = mid + 1;
            }
            if (sessionStartTime <
                locations[mid].getTime()) {
                right = mid - 1;
            }
        }
        if (offset == -1) {
            offset = left;
        }

        if (VERBOSE) {
           Log.v(TAG, "offset : " + offset);
        }
        return offset;
    }

    public IFlpService getFlpBinder() {
        return mBinder;
    }

    public ITestService getFlpTestingBinder() {
        return mTestingBinder;
    }

    /**
     * Called from native code to report HIDL death
     */
    private void reportFLPServiceDied() {
        if (VERBOSE) {
            Log.d(TAG, "FLPServiceDied, restarting sessions...");
        }
        stopFlpSessions();
        if (mIsLocationSettingsOn) {
            startFlpSessions();
        }
    }

    static {
        native_flp_class_init();
    }

    private static native void native_flp_class_init();
    private native int native_flp_init();
    private native int native_flp_get_all_supported_features();
    private native int native_flp_start_session(int id,
                                                int flags,
                                                long period_ns,
                                                int distance_interval_mps,
                                                long trip_distance_m,
                                                int power_mode,
                                                long tbm_ms);
    private native int native_flp_update_session(int id,
                                                 int flags,
                                                 long period_ns,
                                                 int distance_interval_mps,
                                                 long trip_distance_m,
                                                 int power_mode,
                                                 long tbm_ms);
    private native int native_flp_stop_session(int id);
    private native int native_flp_get_all_locations(int id);
    private native void native_flp_delete_aiding_data(long flags);
    private native void native_flp_update_xtra_throttle(boolean enabled);
    private native void native_flp_get_max_power_allocated_in_mw();
}
