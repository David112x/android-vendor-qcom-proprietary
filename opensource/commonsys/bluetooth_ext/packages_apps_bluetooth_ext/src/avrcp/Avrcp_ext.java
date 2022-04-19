/*
 * Copyright (C) 2017, The Linux Foundation. All rights reserved.
 * Not a Contribution.
 */
/*
 * Copyright (C) 2016 The Android Open Source Project
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

package com.android.bluetooth.avrcp;

import android.annotation.NonNull;
import android.annotation.Nullable;
import android.bluetooth.BluetoothA2dp;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothAvrcp;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothProfile;
import android.content.BroadcastReceiver;
import com.android.bluetooth.a2dp.A2dpService;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.ApplicationInfo;
import android.content.pm.PackageManager;
import android.content.pm.PackageManager.NameNotFoundException;
import android.content.pm.ResolveInfo;
import android.content.res.Resources;
import android.content.SharedPreferences;
import android.graphics.Color;
import android.media.AudioManager;
import android.media.AudioAttributes;
import android.media.AudioPlaybackConfiguration;
import android.media.MediaDescription;
import android.media.MediaMetadata;
import android.media.browse.MediaBrowser;
import android.media.session.MediaSession;
import android.media.session.MediaSession.QueueItem;
import android.media.session.MediaSessionManager;
import android.media.session.PlaybackState;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Looper;
import android.os.Message;
import android.os.SystemClock;
import android.os.SystemProperties;
import android.os.UserManager;
import android.util.Log;
import android.view.KeyEvent;
import com.android.bluetooth.Utils;
import android.app.Notification;
import android.app.NotificationManager;
import android.app.NotificationChannel;

import com.android.bluetooth.btservice.AdapterService;
import com.android.bluetooth.btservice.ProfileService;
import com.android.bluetooth.btservice.AbstractionLayer;
import com.android.bluetooth.ba.BATService;
import com.android.bluetooth.R;
import com.android.bluetooth.Utils;
import com.android.bluetooth.avrcpcontroller.AvrcpControllerService;
import com.android.bluetooth.ReflectionUtils;
import com.android.bluetooth.hearingaid.HearingAidService;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.SortedMap;
import java.util.TreeMap;
import java.util.Objects;
import com.android.bluetooth.hfp.HeadsetService;
import java.util.Arrays;
import java.util.concurrent.ConcurrentHashMap;
/******************************************************************************
 * support Bluetooth AVRCP profile. support metadata, play status, event
 * notifications, address player selection and browse feature implementation.
 ******************************************************************************/

public final class Avrcp_ext {
    private static final boolean DEBUG = true;
    private static final String TAG = "Avrcp_ext";
    private static final String ABSOLUTE_VOLUME_BLACKLIST = "absolute_volume_blacklist";
    private static final String AVRCP_VERSION_PROPERTY = "persist.bluetooth.avrcpversion";
    private static final String AVRCP_1_6_STRING = "avrcp16";
    private static final String AVRCP_NOTIFICATION_ID = "avrcp_notification";

    private static Avrcp_ext mAvrcp = null;
    private Context mContext;
    private final AudioManager mAudioManager;
    private AvrcpMessageHandler mHandler;
    private final BluetoothAdapter mAdapter;
    private A2dpService mA2dpService;
    private Handler mAudioManagerPlaybackHandler;
    private AudioManagerPlaybackListener mAudioManagerPlaybackCb;
    private MediaSessionManager mMediaSessionManager;
    private @Nullable MediaController mMediaController;
    private MediaControllerListener mMediaControllerCb;
    private MediaAttributes mMediaAttributes;
    private long mLastQueueId;
    private PackageManager mPackageManager;
    private int mTransportControlFlags;
    private int mA2dpState;
    private @NonNull PlaybackState mCurrentPlayerState;
    private long mLastStateUpdate;
    private boolean mAudioManagerIsPlaying;
    private int mPlayStatusChangedNT;
    private byte mReportedPlayStatus;
    private int mPlayerStatusChangeNT;
    private int mTrackChangedNT;
    private int mPlayPosChangedNT;
    private long mSongLengthMs;
    private int mAddrPlayerChangedNT;
    private int mNowPlayingListChangedNT;
    private long mPlaybackIntervalMs;
    private long mLastReportedPosition;
    private long mNextPosMs;
    private long mPrevPosMs;
    private int mFeatures;
    private int mRemoteVolume;
    private int mLastRemoteVolume;
    private int mInitialRemoteVolume;
    private NotificationManager mNotificationManager;
    /* Local volume in audio index 0-15 */
    private int mLocalVolume;
    private int mLastLocalVolume;

    private boolean mFastforward;
    private boolean mRewind;

    private String mAddress;

    private int mLastDirection;
    private final int mVolumeStep;
    private final int mAudioStreamMax;
    private boolean updateAbsVolume = false;
    private static final int NO_PLAYER_ID = 0;

    private boolean is_player_updated_for_browse;
    private String mCachedBrowsePlayer;
    private int mCurrAddrPlayerID;
    private int mCurrBrowsePlayerID;
    private int mLastUsedPlayerID;
    private AvrcpMediaRsp mAvrcpMediaRsp;
    private AvrcpPlayerAppSettingsRsp mAvrcpPlayerAppSettingsRsp;
    private int maxAvrcpConnections = 1; // Max Avrcp connections at any time
    private static final int INVALID_DEVICE_INDEX = 0xFF;
    private boolean pts_test = false;
    private boolean avrcp_playstatus_blacklist = false;
    private static final String [] BlacklistDeviceAddrToMediaAttr = {"00:17:53"/*Toyota Etios*/};
    private boolean ignore_play;
    private BluetoothDevice disconnectedActiveDevice;
    private byte changePathFolderType;
    private FolderItemsRsp_ext saveRspObj;
    private int changePathDepth;
    private byte changePathDirection;
    HashMap<BluetoothDevice, Integer> mVolumeMap = new HashMap();
    HashMap<BluetoothDevice, Boolean> mDeviceAbsVolMap = new HashMap();
    public static final String VOLUME_MAP = "bluetooth_volume_map";
    public static final String ABS_VOL_MAP = "bluetooth_ABS_VOL_map";
    private boolean isShoActive = false;
    private boolean twsShoEnabled = false;
    byte[] dummyaddr = {(byte)0xFA, (byte)0xCE, (byte)0xFA,
                        (byte)0xCE, (byte)0xFA, (byte)0xCE};
    private static final String playerStateUpdateBlackListedAddr[] = {
         "BC:30:7E", //bc-30-7e-5e-f6-27, Name: Porsche BT 0310; bc-30-7e-8c-22-cb, Name: Audi MMI 1193
         "2C:DC:AD", //2C-DC-AD-BB-2F-25, Name: PORSCHE
         "00:1E:43", //00-1e-43-14-f0-68, Name: Audi MMI 4365
         "9C:DF:03", //9C:DF:03:D3:C0:17, Name: Benz S600L
         "00:0A:08",  //00:0A:08:51:1E:E7, Name: BMW530
         "00:04:79", //00-04-79-00-06-bc, Name: radius HP-BTL01
         "28:A1:83", //28-A1-83-94-90-AE, Name: VW Radio
         "30:c3:d9", //30-c3-d9-8d-5b-83, Name: VW RadioBin
         "24:df:6a", //24-df-6a-f4-0a-7e, Name: HUAWEI WATCH
     };
    private static final String playerStateUpdateBlackListedNames[] = {
       "Audi",
       "Porsche"
    };


    private static final ArrayList<String> playerBrowseWhiteListDB =
       new ArrayList<String>(Arrays.asList(
               "com.google.android.music",
               "com.nhn.android.music",
               "com.gaana"
    ));

    private static final String nonMediaAppsBlacklistedNames[] = {
       "telecom",
       "skype"
    };

    /* UID counter to be shared across different files. */
    static short sUIDCounter = AvrcpConstants_ext.DEFAULT_UID_COUNTER;

    /* BTRC features */
    public static final int BTRC_FEAT_METADATA = 0x01;
    public static final int BTRC_FEAT_ABSOLUTE_VOLUME = 0x02;
    public static final int BTRC_FEAT_BROWSE = 0x04;
    public static final int BTRC_FEAT_COVER_ART = 0x08;
    public static final int BTRC_FEAT_AVRC_UI_UPDATE = 0x10;

    /* AVRC response codes, from avrc_defs */
    private static final int AVRC_RSP_NOT_IMPL = 8;
    private static final int AVRC_RSP_ACCEPT = 9;
    private static final int AVRC_RSP_REJ = 10;
    private static final int AVRC_RSP_IN_TRANS = 11;
    private static final int AVRC_RSP_IMPL_STBL = 12;
    private static final int AVRC_RSP_CHANGED = 13;
    private static final int AVRC_RSP_INTERIM = 15;

    /* AVRC request commands from Native */
    private static final int MSG_NATIVE_REQ_GET_RC_FEATURES = 1;
    private static final int MSG_NATIVE_REQ_GET_PLAY_STATUS = 2;
    private static final int MSG_NATIVE_REQ_GET_ELEM_ATTRS = 3;
    private static final int MSG_NATIVE_REQ_REGISTER_NOTIFICATION = 4;
    private static final int MSG_NATIVE_REQ_VOLUME_CHANGE = 5;
    private static final int MSG_NATIVE_REQ_GET_FOLDER_ITEMS = 6;
    private static final int MSG_NATIVE_REQ_SET_ADDR_PLAYER = 7;
    private static final int MSG_NATIVE_REQ_SET_BR_PLAYER = 8;
    private static final int MSG_NATIVE_REQ_CHANGE_PATH = 9;
    private static final int MSG_NATIVE_REQ_PLAY_ITEM = 10;
    private static final int MSG_NATIVE_REQ_GET_ITEM_ATTR = 11;
    private static final int MSG_NATIVE_REQ_GET_TOTAL_NUM_OF_ITEMS = 12;
    private static final int MSG_NATIVE_REQ_PASS_THROUGH = 13;

    /* other AVRC messages */
    private static final int MSG_PLAY_INTERVAL_TIMEOUT = 14;
    private static final int MSG_ADJUST_VOLUME = 15;
    private static final int MSG_SET_ABSOLUTE_VOLUME = 16;
    private static final int MSG_ABS_VOL_TIMEOUT = 17;
    private static final int MSG_SET_A2DP_AUDIO_STATE = 18;
    private static final int MSG_NOW_PLAYING_CHANGED_RSP = 19;
    private static final int MESSAGE_DEVICE_RC_CLEANUP = 21;
    private static final int MSG_PLAY_INTERVAL_TIMEOUT_2 = 22;
    private final static int MESSAGE_PLAYERSETTINGS_TIMEOUT = 23;
    private final static int MESSAGE_SET_MEDIA_SESSION = 24;
    private final static int MSG_SET_AVRCP_CONNECTED_DEVICE = 25;
    private final static int MESSAGE_UPDATE_ABS_VOLUME_STATUS = 31;
    private static final int MSG_PLAY_STATUS_CMD_TIMEOUT = 33;
    private final static int MESSAGE_START_SHO = 34;
    private static final int MSG_SET_ACTIVE_DEVICE = 35;

    private static final int CMD_TIMEOUT_DELAY = 2000;
    private static final int MAX_ERROR_RETRY_TIMES = 6;
    private static final int AVRCP_MAX_VOL = 127;
    private static final int AVRCP_BASE_VOLUME_STEP = 1;
    public static final int AVRC_ID_VOL_UP = 0x41;
    public static final int AVRC_ID_VOL_DOWN = 0x42;
    private static final int SET_MEDIA_SESSION_DELAY = 300;

    /* Communicates with MediaPlayer to fetch media content */
    private BrowsedMediaPlayer_ext mBrowsedMediaPlayer;

    /* Addressed player handling */
    private AddressedMediaPlayer_ext mAddressedMediaPlayer;

    /* List of Media player instances, useful for retrieving MediaPlayerList or MediaPlayerInfo */
    private SortedMap<Integer, MediaPlayerInfo_ext> mMediaPlayerInfoList;
    private boolean mAvailablePlayerViewChanged;

    private List<String> mPkgRequestedMBSConnect;

    /* List of media players which supports browse */
    private List<BrowsePlayerInfo_ext> mBrowsePlayerInfoList;

    /* Manage browsed players */
    private AvrcpBrowseManager mAvrcpBrowseManager;

    /* Player Application Settings */
    private AvrcpPlayerAppSettings mAvrcpPlayerAppSettings;

    /* BIP Responder */
    private static AvrcpBipRsp mAvrcpBipRsp;

    /* Broadcast receiver for device connections intent broadcasts */
    private final BroadcastReceiver mAvrcpReceiver = new AvrcpServiceBroadcastReceiver();
    private final BroadcastReceiver mBootReceiver = new AvrcpServiceBootReceiver();
    private final BroadcastReceiver mShutDownReceiver = new AvrcpServiceShutDownReceiver();

    /* Recording passthrough key dispatches */
    static private final int PASSTHROUGH_LOG_MAX_SIZE = DEBUG ? 50 : 10;
    private EvictingQueue_ext<MediaKeyLog> mPassthroughLogs; // Passthorugh keys dispatched
    private List<MediaKeyLog> mPassthroughPending; // Passthrough keys sent not dispatched yet
    private int mPassthroughDispatched; // Number of keys dispatched

    private BluetoothDevice mCurrentBrowsingDevice = null;
    private BluetoothDevice mBrowsingActiveDevice = null;

    private class MediaKeyLog {
        private long mTimeSent;
        private long mTimeProcessed;
        private String mPackage;
        private KeyEvent mEvent;

        public MediaKeyLog(long time, KeyEvent event) {
            mEvent = event;
            mTimeSent = time;
        }

        public boolean addDispatch(long time, KeyEvent event, String packageName) {
            if (mPackage != null) return false;
            if (event.getAction() != mEvent.getAction()) return false;
            if (event.getKeyCode() != mEvent.getKeyCode()) return false;
            mPackage = packageName;
            mTimeProcessed = time;
            return true;
        }

        public String toString() {
            StringBuilder sb = new StringBuilder();
            sb.append(android.text.format.DateFormat.format("MM-dd HH:mm:ss", mTimeSent));
            sb.append(" " + mEvent.toString());
            if (mPackage == null) {
                sb.append(" (undispatched)");
            } else {
                sb.append(" to " + mPackage);
                sb.append(" in " + (mTimeProcessed - mTimeSent) + "ms");
            }
            return sb.toString();
        }
    }

    // Device dependent registered Notification & Variables
    private class DeviceDependentFeature {
        private Context mContext;
        private BluetoothDevice mCurrentDevice;
        private @NonNull PlaybackState mCurrentPlayState;
        private int mPlayStatusChangedNT;
        private int mNowPlayingListChangedNT;
        private int mNowPlayingChangedNT;
        private int mTrackChangedNT;
        private long mNextPosMs;
        private long mPrevPosMs;
        private long mPlaybackIntervalMs;
        private int mPlayerStatusChangeNT;
        private long mLastReportedPosition;
        private int mPlayPosChangedNT;
        private int mFeatures;
        private int mLastDirection;
        private HashMap<Integer, Integer> mMusicAppCmdResponsePending;
        private int mAbsoluteVolume;
        private int mLastSetVolume;
        private boolean mVolCmdSetInProgress;
        private boolean mVolCmdAdjustInProgress;
        private boolean isAbsoluteVolumeSupportingDevice;
        private int mAbsVolRetryTimes;
        private int keyPressState;
        private long mTracksPlayed;
        private int mAvailablePlayersChangedNT;
        private int mAddrPlayerChangedNT;
        private long mLastStateUpdate;
        private int mUidsChangedNT;

        private int mRemoteVolume;
        private int mLastRemoteVolume;
        private int mInitialRemoteVolume;
        private int mLastRspPlayStatus;
        private boolean isActiveDevice;
        private int mLastRequestedVolume;
        private boolean isPlayStatusTimeOut;

        /* Local volume in audio index 0-15 */
        private int mLocalVolume;
        private int mLastLocalVolume;
        private int mBlackListVolume;
        private int mLastPassthroughcmd;
        private int mReportedPlayerID;
        private boolean mTwsPairDisconnected;
        private boolean cache_play_cmd;
        public DeviceDependentFeature(Context context) {
            mContext = context;
            mCurrentDevice = null;
            mCurrentPlayState = new PlaybackState.Builder().setState(PlaybackState.STATE_NONE, -1L, 0.0f).build();;
            mPlayStatusChangedNT = AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
            mNowPlayingListChangedNT = AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
            mAddrPlayerChangedNT = AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
            mTrackChangedNT = AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
            mPlayerStatusChangeNT = AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
            mNextPosMs = -1;
            mPrevPosMs = -1;
            mPlaybackIntervalMs = 0L;
            mLastReportedPosition = -1;
            mPlayPosChangedNT = AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
            mFeatures = 0;
            mLastDirection = 0;
            mAbsoluteVolume = -1;
            mLastSetVolume = -1;
            mTracksPlayed = 0;
            mVolCmdAdjustInProgress = false;
            mVolCmdSetInProgress = false;
            isAbsoluteVolumeSupportingDevice = false;
            mAbsVolRetryTimes = 0;
            keyPressState = AvrcpConstants_ext.KEY_STATE_RELEASE; //Key release state
            mRemoteVolume = -1;
            mMusicAppCmdResponsePending = new HashMap<Integer, Integer>();
            mAvailablePlayersChangedNT = AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
            mAddrPlayerChangedNT = AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
            mUidsChangedNT = AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
            mLastStateUpdate = -1;
            mInitialRemoteVolume = -1;
            mBlackListVolume = -1;
            isActiveDevice = false;
            isPlayStatusTimeOut = false;
            mLastRemoteVolume = -1;
            mLastRequestedVolume = -1;
            mLocalVolume = -1;
            mLastLocalVolume = -1;
            mAbsoluteVolume = -1;
            mLastRspPlayStatus = -1;
            mLastPassthroughcmd = KeyEvent.KEYCODE_UNKNOWN;
            mReportedPlayerID = NO_PLAYER_ID;
            mTwsPairDisconnected = false;
            cache_play_cmd = false;
        }
    };
    DeviceDependentFeature[] deviceFeatures;

    private static class SHOQueue {
        static BluetoothDevice device;
        static boolean PlayReq;
        static boolean isRetry;
    }

    static {
        classInitNative();
    }


    private Avrcp_ext(Context context, A2dpService svc, int maxConnections ) {
        if (DEBUG) Log.v(TAG, "Avrcp");
        mCachedBrowsePlayer = null;
        is_player_updated_for_browse = false;
        mAdapter = BluetoothAdapter.getDefaultAdapter();
        mMediaAttributes = new MediaAttributes(null, null);
        mLastQueueId = MediaSession.QueueItem.UNKNOWN_ID;
        mLastStateUpdate = -1L;
        mSongLengthMs = 0L;
        mCurrentPlayerState = new PlaybackState.Builder().setState(PlaybackState.STATE_NONE, -1L, 0.0f).build();
        mReportedPlayStatus = PLAYSTATUS_ERROR;
        mA2dpState = BluetoothA2dp.STATE_NOT_PLAYING;
        mAudioManagerIsPlaying = false;
        mPlayStatusChangedNT = AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
        mPlayerStatusChangeNT  = AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
        mTrackChangedNT = AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
        mPlayPosChangedNT = AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
        mAddrPlayerChangedNT = AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
        mNowPlayingListChangedNT = AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
        mPlaybackIntervalMs = 0L;
        mLastReportedPosition = -1;
        mNextPosMs = -1;
        mPrevPosMs = -1;
        mFeatures = 0;
        mRemoteVolume = -1;
        mInitialRemoteVolume = -1;
        mLastRemoteVolume = -1;
        mLastDirection = 0;
        mLocalVolume = -1;
        mLastLocalVolume = -1;
        mCurrAddrPlayerID = NO_PLAYER_ID;
        mCurrBrowsePlayerID = 0;
        mContext = context;
        mLastUsedPlayerID = 0;
        mAddressedMediaPlayer = null;
        mAvrcpBipRsp = null;
        mAvrcpPlayerAppSettings = null;
        mA2dpService = svc;
        maxAvrcpConnections = maxConnections;
        deviceFeatures = new DeviceDependentFeature[maxAvrcpConnections];
        for(int i = 0; i < maxAvrcpConnections; i++) {
            deviceFeatures[i] = new DeviceDependentFeature(mContext);
        }
        mFastforward = false;
        mRewind = false;
        mCurrentBrowsingDevice = null;
        mBrowsingActiveDevice = null;

        initNative(maxAvrcpConnections);
        String twsSho = SystemProperties.get("persist.vendor.btstack.enable.twsplussho");
        if (!twsSho.isEmpty() && twsSho.equals("true")) {
            twsShoEnabled = true;
        }
        mMediaSessionManager = (MediaSessionManager) context.getSystemService(
            Context.MEDIA_SESSION_SERVICE);
        mAudioManager = (AudioManager) context.getSystemService(Context.AUDIO_SERVICE);
        mAudioStreamMax = mAudioManager.getStreamMaxVolume(AudioManager.STREAM_MUSIC);
        mVolumeStep = Math.max(AVRCP_BASE_VOLUME_STEP, AVRCP_MAX_VOL/mAudioStreamMax);

        // Register for package removal intent broadcasts for media button receiver persistence
        IntentFilter pkgFilter = new IntentFilter();
        pkgFilter.addAction(Intent.ACTION_PACKAGE_REMOVED);
        pkgFilter.addAction(Intent.ACTION_PACKAGE_ADDED);
        pkgFilter.addAction(Intent.ACTION_PACKAGE_CHANGED);
        pkgFilter.addAction(Intent.ACTION_PACKAGE_DATA_CLEARED);
        pkgFilter.addDataScheme("package");
        context.registerReceiver(mAvrcpReceiver, pkgFilter);

        IntentFilter bootFilter = new IntentFilter();
        bootFilter.addAction(Intent.ACTION_USER_UNLOCKED);
        context.registerReceiver(mBootReceiver, bootFilter);
        pts_test = SystemProperties.getBoolean("vendor.bluetooth.avrcpct-passthrough.pts", false);
        avrcp_playstatus_blacklist = SystemProperties.getBoolean("persist.vendor.btstack.avrcp-playstatus.blacklist", false);

        IntentFilter shutdownFilter = new IntentFilter();
        shutdownFilter.addAction(Intent.ACTION_SHUTDOWN);
        context.registerReceiver(mShutDownReceiver, shutdownFilter);

        // create Notification channel.
        mNotificationManager = (NotificationManager)
                mContext.getSystemService(Context.NOTIFICATION_SERVICE);
        NotificationChannel mChannel = new NotificationChannel(AVRCP_NOTIFICATION_ID,
                mContext.getString(R.string.avrcp_notification_name), NotificationManager.IMPORTANCE_DEFAULT);
        mChannel.setDescription(mContext.getString(R.string.bluetooth_advanced_feat_description));
        // setDefaults(int defaults) This method was deprecated in API level 26. use
        // NotificationChannel.enableVibration(boolean) and
        // NotificationChannel.enableLights(boolean) and
        // NotificationChannel.setSound(Uri, AudioAttributes) instead.
        mChannel.enableLights(true);
        mChannel.enableVibration(false);
        mChannel.setSound(Uri.EMPTY, Notification.AUDIO_ATTRIBUTES_DEFAULT);
        mChannel.setLightColor(Color.GREEN);
        mNotificationManager.createNotificationChannel(mChannel);

    }

    private synchronized void start() {
        if (DEBUG) Log.v(TAG, "start");
        if (mHandler != null)
            mHandler = null;
        HandlerThread thread = new HandlerThread("BluetoothAvrcpHandler");
        thread.start();
        Looper looper = thread.getLooper();
        mHandler = new AvrcpMessageHandler(looper);
        mAudioManagerPlaybackHandler = new Handler(looper);
        mAudioManagerPlaybackCb = new AudioManagerPlaybackListener();
        mMediaControllerCb = new MediaControllerListener();
        mAvrcpMediaRsp = new AvrcpMediaRsp();
        mAvrcpPlayerAppSettingsRsp = new AvrcpPlayerAppSettingsRsp();
        mMediaPlayerInfoList = new TreeMap<Integer, MediaPlayerInfo_ext>();
        mAvailablePlayerViewChanged = false;
        mPkgRequestedMBSConnect = new ArrayList<String>();
        mBrowsePlayerInfoList = Collections.synchronizedList(new ArrayList<BrowsePlayerInfo_ext>());
        mPassthroughDispatched = 0;
        mPassthroughLogs = new EvictingQueue_ext<MediaKeyLog>(PASSTHROUGH_LOG_MAX_SIZE);
        mPassthroughPending = Collections.synchronizedList(new ArrayList<MediaKeyLog>());
        if (mMediaSessionManager != null) {
            mMediaSessionManager.addOnActiveSessionsChangedListener(mActiveSessionListener, null,
                    mHandler);
            mMediaSessionManager.setCallback(mButtonDispatchCallback, null);
        }
        mPackageManager = mContext.getApplicationContext().getPackageManager();

        boolean isCoverArtSupported = false;
        AdapterService adapterService = AdapterService.getAdapterService();
        if (adapterService != null) {
            if (adapterService.getProfileInfo(AbstractionLayer.AVRCP, AbstractionLayer.AVRCP_0103_SUPPORT))
            {
                isCoverArtSupported = false;
                if (DEBUG) Log.d(TAG, "isCoverArtSupported: " + isCoverArtSupported);
            } else if(adapterService.getProfileInfo(AbstractionLayer.AVRCP, AbstractionLayer.AVRCP_COVERART_SUPPORT)){
                isCoverArtSupported = true;
                if (DEBUG) Log.d(TAG, "isCoverArtSupported: " + isCoverArtSupported);
            }
        }

        String avrcpVersion = SystemProperties.get(AVRCP_VERSION_PROPERTY, AVRCP_1_6_STRING);
        if (DEBUG) Log.d(TAG, "avrcpVersion " + avrcpVersion);
        /* Enable Cover Art support is version is 1.6 and flag is set in config */
        if (isCoverArtSupported && avrcpVersion != null &&
            avrcpVersion.equals(AVRCP_1_6_STRING))
            mAvrcpBipRsp = new AvrcpBipRsp(mContext, maxAvrcpConnections);
        if (mAvrcpBipRsp != null) {
            if (DEBUG) Log.d(TAG, "Starting AVRCP BIP Responder Service");
            mAvrcpBipRsp.start();
        }

        mAvrcpPlayerAppSettings = new AvrcpPlayerAppSettings(mContext, mAvrcpPlayerAppSettingsRsp);
        if (mAvrcpPlayerAppSettings != null) {
            if (DEBUG) Log.d(TAG, "Starting AVRCP AvrcpPlayerAppSetting Service");
            mAvrcpPlayerAppSettings.start();
        }

        /* create object to communicate with addressed player */
        mAddressedMediaPlayer = new AddressedMediaPlayer_ext(mAvrcpMediaRsp, mAvrcp);

        /* initialize BrowseMananger which manages Browse commands and response */
        mAvrcpBrowseManager = new AvrcpBrowseManager(mContext, mAvrcpMediaRsp);

        initMediaPlayersList();

        BrowsedMediaPlayer_ext player =
                mAvrcpBrowseManager.getBrowsedMediaPlayer(dummyaddr);
        if (player != null)
            player.start();

        UserManager manager = UserManager.get(mContext);
        if (manager == null || manager.isUserUnlocked()) {
            if (DEBUG) Log.d(TAG, "User already unlocked, initializing player lists");
            // initialize browsable player list and build media player list
            buildBrowsablePlayerList();
        }

        mAudioManager.registerAudioPlaybackCallback(
                mAudioManagerPlaybackCb, mAudioManagerPlaybackHandler);
        changePathDepth = 0;
        changePathFolderType = 0;
        changePathDirection = 0;
        disconnectedActiveDevice = null;
        Avrcp_extVolumeManager();
        Log.v(TAG, "Exit start");
    }

    public static Avrcp_ext make(Context context, A2dpService svc,
                             int maxConnections) {
        if (DEBUG) Log.v(TAG, "make");
        Log.v(TAG, "mAvrcp = " + mAvrcp);
        //Avrcp_ext ar = new Avrcp_ext(context, svc, maxConnections);
        if(mAvrcp == null) {
            mAvrcp = new Avrcp_ext(context, svc, maxConnections);
            mAvrcp.start();
        }
        Log.v(TAG, "Exit make");
        return mAvrcp;
    }

    public static void clearAvrcpInstance () {
        Log.v(TAG, "clearing mAvrcp instatnce" + mAvrcp);
        mAvrcp = null;
        Log.v(TAG, "After clearing mAvrcp instatnce " + mAvrcp);
    }

    public synchronized void doQuit() {
        if (DEBUG) Log.d(TAG, "doQuit");
        if (mAudioManager != null)
            mAudioManager.unregisterAudioPlaybackCallback(mAudioManagerPlaybackCb);

        if (mMediaController != null) mMediaController.unregisterCallback(mMediaControllerCb);
        Message msg = mHandler.obtainMessage(MESSAGE_DEVICE_RC_CLEANUP, 0,
               0, null);
        mHandler.sendMessage(msg);
        if (mMediaSessionManager != null) {
            mMediaSessionManager.setCallback(null, null);
            mMediaSessionManager.removeOnActiveSessionsChangedListener(mActiveSessionListener);
        }

        if (mAvrcpPlayerAppSettings != null) {
            mAvrcpPlayerAppSettings.cleanup();
            mAvrcpPlayerAppSettings = null;
        }

        mAudioManagerPlaybackHandler.removeCallbacksAndMessages(null);
        mHandler.removeCallbacksAndMessages(null);
        Looper looper = mHandler.getLooper();
        if (looper != null) {
            looper.quit();
        }

        if (mAvrcpBipRsp != null) {
            mAvrcpBipRsp.stop();
            mAvrcpBipRsp = null;
        }

        mAudioManagerPlaybackHandler = null;
        mContext.unregisterReceiver(mAvrcpReceiver);
        mContext.unregisterReceiver(mBootReceiver);
        mContext.unregisterReceiver(mShutDownReceiver);

        mPkgRequestedMBSConnect.clear();
        mAddressedMediaPlayer.cleanup();
        mAvrcpBrowseManager.cleanup();
        mCurrentBrowsingDevice = null;
        mBrowsingActiveDevice = null;
        if (mNotificationManager != null )
            mNotificationManager.deleteNotificationChannel(AVRCP_NOTIFICATION_ID);
        changePathDepth = 0;
        changePathFolderType = 0;
        changePathDirection = 0;
        disconnectedActiveDevice = null;
        Log.d(TAG, "Exit doQuit");
    }

    public void clearDeviceDependentFeature() {
        Log.d(TAG, "Enter clearDeviceDependentFeature()");
        for (int i = 0; i < maxAvrcpConnections; i++) {
            deviceFeatures[i].keyPressState =
                AvrcpConstants_ext.KEY_STATE_RELEASE; //Key release state
            if (deviceFeatures[i].mMusicAppCmdResponsePending != null)
                deviceFeatures[i].mMusicAppCmdResponsePending.clear();
        }
        Log.d(TAG, "Exit clearDeviceDependentFeature()");
    }

    public void cleanup() {
        if (DEBUG) Log.d(TAG, "cleanup");
        cleanupNative();
        Log.d(TAG, "Exit cleanup()");
    }

    public boolean isCoverArtFeatureSupported(byte[] bdaddr) {
        Log.w(TAG, "isCoverArtFeatureSupported");
        String address = Utils.getAddressStringFromByte((byte[]) bdaddr);
        BluetoothDevice device = mAdapter.getRemoteDevice(address);
        if (device == null)
            return false;
        MediaPlayerInfo_ext player = mMediaPlayerInfoList.getOrDefault(mCurrAddrPlayerID, null);
        int index = getIndexForDevice(device);
        if ((index == INVALID_DEVICE_INDEX) || (player == null))
            return false;

        short[] featureBits = player.getFeatureBitMask();
        boolean playerSupportsCA = false;
        for (int i = 0; i < featureBits.length; i++) {
            if (featureBits[i] == AvrcpConstants_ext.AVRC_PF_COVER_ART_BIT_NO) {
                playerSupportsCA = true;
                break;
            }
        }
        boolean peerSupprtsCA = ((deviceFeatures[index].mFeatures & BTRC_FEAT_COVER_ART) != 0);
        Log.w(TAG, "playersupportCA " + playerSupportsCA + " peersupportCA " + peerSupprtsCA);
        return (peerSupprtsCA && playerSupportsCA);
    }

    private class AudioManagerPlaybackListener extends AudioManager.AudioPlaybackCallback {
        @Override
        public void onPlaybackConfigChanged(List<AudioPlaybackConfiguration> configs) {
            super.onPlaybackConfigChanged(configs);
            boolean isPlaying = false;
            for (AudioPlaybackConfiguration config : configs) {
                if (DEBUG) {
                    Log.d(TAG,
                            "AudioManager Player: "
                                    + AudioPlaybackConfiguration.toLogFriendlyString(config));
                }
                if (config.getPlayerState() == AudioPlaybackConfiguration.PLAYER_STATE_STARTED) {
                    isPlaying = true;
                    break;
                }
            }
            if (DEBUG) Log.d(TAG, "AudioManager isPlaying: " + isPlaying);
            if (mAudioManagerIsPlaying != isPlaying) {
                mAudioManagerIsPlaying = isPlaying;
                updateCurrentMediaState(null);
            }
        }
    }

    private class MediaControllerListener extends MediaController.Callback {
        @Override
        public void onMetadataChanged(MediaMetadata metadata) {
            if (DEBUG) Log.v(TAG, "onMetadataChanged");
            updateCurrentMediaState(null);
        }
        @Override
        public synchronized void onPlaybackStateChanged(PlaybackState state) {
            if (DEBUG) Log.v(TAG, "onPlaybackStateChanged: state " + ((state != null)? state.toString() : "NULL"));
            updateCurrentMediaState(null);
            Log.v(TAG, " Exit onPlaybackStateChanged");
        }

        @Override
        public void onSessionDestroyed() {
            Log.v(TAG, "MediaController session destroyed");
            synchronized (Avrcp_ext.this) {
                if (mMediaController != null)
                    removeMediaController(mMediaController.getWrappedInstance());
            }
            Log.d(TAG, "Exit onSessionDestroyed");
        }


        @Override
        public void onQueueChanged(List<MediaSession.QueueItem> queue) {
            if (queue == null) {
                Log.v(TAG, "onQueueChanged: received null queue");
                return;
            }

            Log.v(TAG, "onQueueChanged: NowPlaying list changed, Queue Size = "+ queue.size());
            mHandler.sendEmptyMessage(MSG_NOW_PLAYING_CHANGED_RSP);
            Log.d(TAG, "Exit onQueueChanged");
        }
    }
    private boolean isPlayerStateUpdateBlackListed(String address, String deviceName) {
        if (address == null) return false;
        for (int i = 0; i < playerStateUpdateBlackListedAddr.length; i++) {
            String addr = playerStateUpdateBlackListedAddr[i];
            if (address.toLowerCase().startsWith(addr.toLowerCase())) {
                Log.d(TAG, "AVRCP PlayerStateUpdateBlacklist Addr Matched:" + address);
                return true;
            }
        }
        if (deviceName == null) return false;
        for (int j = 0; j < playerStateUpdateBlackListedNames.length; j++) {
            String name = playerStateUpdateBlackListedNames[j];
            if (deviceName.toLowerCase().startsWith(name.toLowerCase()) ||
                deviceName.toLowerCase().equals(name.toLowerCase())) {
                Log.d(TAG, "AVRCP PlayerStateUpdateBlacklist Name Matched:" + deviceName);
                return true;
            }
        }
        return false;
    };

    private boolean isAppBlackListedForMediaSessionUpdate(String appName) {
        if (appName == null) return false;
        for (int j = 0; j < nonMediaAppsBlacklistedNames.length; j++) {
            String name = nonMediaAppsBlacklistedNames[j];
            if (appName.toLowerCase().contains(name.toLowerCase())) {
                Log.d(TAG, "AVRCP non Media app BL for media session update: " + appName);
                return true;
            }
        }
        return false;
    };
    /** Handles Avrcp messages. */
    private final class AvrcpMessageHandler extends Handler {
        private AvrcpMessageHandler(Looper looper) {
            super(looper);
        }

        @Override
        public void handleMessage(Message msg) {
            int deviceIndex  = INVALID_DEVICE_INDEX;
            if (DEBUG) Log.v(TAG, "AvrcpMessageHandler: received message=" + msg.what);

            switch (msg.what) {
            case MESSAGE_PLAYERSETTINGS_TIMEOUT:
            {
                Log.e(TAG, "**MESSAGE_PLAYSTATUS_TIMEOUT: Addr: " +
                            (String)msg.obj + " Msg: " + msg.arg1);
                BluetoothDevice currdevice;
                currdevice = mAdapter.getRemoteDevice((String) msg.obj);
                deviceIndex = getIndexForDevice(currdevice);
                if (deviceIndex == INVALID_DEVICE_INDEX) {
                    Log.e(TAG,"Invalid device index for send response");
                    break;
                }
                int rsp = msg.arg1;
                CreateMusicSettingsAppCmdLookupOrUpdate(rsp, deviceIndex, false);
                mAvrcpPlayerAppSettings.handlerMsgTimeout(msg.arg1, currdevice);
                break;
            }
            case MESSAGE_UPDATE_ABS_VOLUME_STATUS:
            {
                deviceIndex = msg.arg1;
                int vol = msg.arg2;
                BluetoothDevice device = mA2dpService.getActiveDevice();
                if (device == null)
                    break;

                if (vol >= 0) {
                    int volume = convertToAvrcpVolume(vol);
                    int remVol = deviceFeatures[deviceIndex].mRemoteVolume;
                    if (remVol != -1) {
                        remVol = convertToAudioStreamVolume(remVol);
                    }
                    Log.d(TAG,"vol = " + vol + "remVol = " + remVol);
                    if (vol != remVol &&
                       deviceFeatures[deviceIndex].mCurrentDevice != null) {
                       setVolumeNative(volume , getByteAddress(deviceFeatures[deviceIndex].mCurrentDevice));
                       deviceFeatures[deviceIndex].mRemoteVolume = volume;
                       if (deviceFeatures[deviceIndex].mCurrentDevice.isTwsPlusDevice()) {
                           AdapterService adapterService = AdapterService.getAdapterService();
                           BluetoothDevice peer_device =
                            adapterService.getTwsPlusPeerDevice(deviceFeatures[deviceIndex].mCurrentDevice);
                           if (peer_device != null &&
                               getIndexForDevice(peer_device) != INVALID_DEVICE_INDEX) {
                               // Change volume to peer earbud as well
                               Log.d(TAG,"setting volume to TWS+ peer also");
                               setVolumeNative(volume, getByteAddress(peer_device));
                           }
                       }
                    }
                }
                break;
            }

            case MSG_NATIVE_REQ_GET_RC_FEATURES:
            {
                String address = (String) msg.obj;
                if (DEBUG)
                    Log.v(TAG, "MSG_GET_RC_FEATURES: address = " + address +
                            ", features = " + msg.arg1);
                BluetoothDevice device = mAdapter.getRemoteDevice(address);
                deviceIndex = getIndexForDevice(device);
                if (deviceIndex == INVALID_DEVICE_INDEX) {
                    Log.v(TAG,"device entry not present, bailing out");
                    return;
                }
                BluetoothDevice mDevice = mA2dpService.getActiveDevice();
                deviceFeatures[deviceIndex].mFeatures = msg.arg1;
                deviceFeatures[deviceIndex].mFeatures =
                    modifyRcFeatureFromBlacklist(deviceFeatures[deviceIndex].mFeatures,
                    address);
                Log.d(TAG, "avrcpct-passthrough pts_test = " + pts_test);
                if (pts_test) {
                    Log.v(TAG,"fake BTRC_FEAT_ABSOLUTE_VOLUME remote feat support for pts test");
                    deviceFeatures[deviceIndex].mFeatures =
                                 deviceFeatures[deviceIndex].mFeatures | BTRC_FEAT_ABSOLUTE_VOLUME;
                }
                deviceFeatures[deviceIndex].isAbsoluteVolumeSupportingDevice =
                        ((deviceFeatures[deviceIndex].mFeatures &
                        BTRC_FEAT_ABSOLUTE_VOLUME) != 0);
                //store Absolute volume support
                   Log.d(TAG,"absolute volume support device is present " + mDeviceAbsVolMap.containsKey(deviceFeatures[deviceIndex].mCurrentDevice));
                if (!mDeviceAbsVolMap.containsKey(deviceFeatures[deviceIndex].mCurrentDevice) ||
                    (mDeviceAbsVolMap.containsKey(deviceFeatures[deviceIndex].mCurrentDevice) &&
                   mDeviceAbsVolMap.get(deviceFeatures[deviceIndex].mCurrentDevice) !=
                   deviceFeatures[deviceIndex].isAbsoluteVolumeSupportingDevice)) {
                   SharedPreferences.Editor absVolumeMapEditor = getAbsVolumeMap().edit();
                   mDeviceAbsVolMap.put(deviceFeatures[deviceIndex].mCurrentDevice, (boolean)deviceFeatures[deviceIndex].isAbsoluteVolumeSupportingDevice);
                   absVolumeMapEditor.putBoolean(deviceFeatures[deviceIndex].mCurrentDevice.getAddress(), deviceFeatures[deviceIndex].isAbsoluteVolumeSupportingDevice);
                   // Always use apply() since it is asynchronous, otherwise the call can hang waiting for
                   // storage to be written.
                   absVolumeMapEditor.apply();
                   Log.d(TAG,"absolute volume support data saved");
                }
                BATService mBatService = BATService.getBATService();
                if ((mBatService != null) && mBatService.isBATActive()) {
                    Log.d(TAG,"MSG_NATIVE_REQ_GET_RC_FEATURES BA Active, update absvol support as true  ");
                    mAudioManager.avrcpSupportsAbsoluteVolume(device.getAddress(),
                            true);
                } else if (device.isTwsPlusDevice()) {
                    if (twsShoEnabled) {
                        //SHO is enabled, check if TWS+ device is active
                        int index = -1;
                        if (mDevice != null) index = getIndexForDevice(mDevice);
                        if (mDevice == null || (mDevice != null && (mDevice.isTwsPlusDevice() ||
                            (index != INVALID_DEVICE_INDEX && isAbsoluteVolumeSupported(index))))) {
                            Log.v(TAG,"TWS+ device, update abs vol as true in RC FEATURE handle");
                            mAudioManager.avrcpSupportsAbsoluteVolume(device.getAddress(), true);
                            if (updateAbsVolume) updateAbsVolume = false;
                        } else {
                            updateAbsVolume = true;
                            Log.d(TAG,"TWS+ is not active, set absVolume flag later");
                        }
                    } else
                        mAudioManager.avrcpSupportsAbsoluteVolume(device.getAddress(), true);
                } else if (mDevice != null && Objects.equals(mDevice, device)) {
                    mAudioManager.avrcpSupportsAbsoluteVolume(device.getAddress(),
                        isAbsoluteVolumeSupported(deviceIndex));
                    Log.v(TAG,"update audio manager for abs vol state = "
                            + isAbsoluteVolumeSupported(deviceIndex));
                }
                deviceFeatures[deviceIndex].mLastPassthroughcmd = KeyEvent.KEYCODE_UNKNOWN;

                if ((deviceFeatures[deviceIndex].mFeatures &
                        BTRC_FEAT_AVRC_UI_UPDATE) != 0)
                {
                    int NOTIFICATION_ID = android.R.drawable.stat_sys_data_bluetooth;
                    //Notification.Builder (Context context) constructor was deprecated in API level 26.
                    //use Notification.Builder (Context context,String channelId) instead.
                    Notification notification = new Notification.Builder(mContext, AVRCP_NOTIFICATION_ID)
                        .setContentTitle(mContext.getString(R.string.bluetooth_rc_feat_title))
                        .setContentText(mContext.getString(R.string.bluetooth_rc_feat_content))
                        .setSubText(mContext.getString(R.string.bluetooth_rc_feat_subtext))
                        .setSmallIcon(android.R.drawable.stat_sys_data_bluetooth)
                        .setChannelId(AVRCP_NOTIFICATION_ID)
                        .build();

                    if (mNotificationManager != null )
                        mNotificationManager.notify(NOTIFICATION_ID, notification);
                    else
                        Log.e(TAG,"mNotificationManager is null");
                    Log.v(TAG," update notification manager on remote repair request");
                }
                break;
            }

            case MSG_NATIVE_REQ_GET_PLAY_STATUS:
            {
                BluetoothDevice device;
                int playState = PLAYSTATUS_ERROR;
                int current_playState = PLAYSTATUS_ERROR;
                int position;

                String address = Utils.getAddressStringFromByte((byte[]) msg.obj);
                Log.v(TAG, "Event for device address " + address);

                device = mAdapter.getRemoteDevice(address);
                deviceIndex = getIndexForDevice(device);
                if (deviceIndex == INVALID_DEVICE_INDEX) {
                    Log.e(TAG,"Invalid device index for play status");
                    break;
                }
                playState = convertPlayStateToPlayStatus(deviceFeatures[deviceIndex].mCurrentPlayState);
                current_playState = playState;
                if (mFastforward) {
                    playState = PLAYSTATUS_FWD_SEEK;
                }
                if (mRewind) {
                    playState = PLAYSTATUS_REV_SEEK;
                }
                position = (int)getPlayPosition(device);
                if(avrcp_playstatus_blacklist)
                {
                    if ((deviceFeatures[deviceIndex].mCurrentDevice != null) &&
                        isPlayerStateUpdateBlackListed(
                            deviceFeatures[deviceIndex].mCurrentDevice.getAddress(),
                            deviceFeatures[deviceIndex].mCurrentDevice.getName()) &&
                            ((playState == PLAYSTATUS_PAUSED) ||
                            (playState == PLAYSTATUS_STOPPED))) {
                        if (mA2dpService.getConnectedDevices().size() > 0) {
                            if (mA2dpService.isA2dpPlaying(
                                deviceFeatures[deviceIndex].mCurrentDevice)) {
                                 Log.w(TAG, "A2dp playing, update playstatus to Carkit as PLAYING: " +
                                     deviceFeatures[deviceIndex].mCurrentDevice.getAddress() +
                                     " playState " + playState);
                                 playState = PLAYSTATUS_PLAYING;
                            }
                        } else {
                            if (mAudioManager.isMusicActive()) {
                                Log.w(TAG, "A2dp not connected, but Music active," +
                                     " update playstatus to Carkit as PLAYING: " +
                                     deviceFeatures[deviceIndex].mCurrentDevice.getAddress() +
                                     " playState " + playState);
                                playState = PLAYSTATUS_PLAYING;
                            }
                        }
                    }
                }
                if (position == -1) {
                    Log.v(TAG, "Force play postion to 0, for getPlayStatus Rsp");
                    position = 0;
                }
                if (avrcp_playstatus_blacklist && (playState == PLAYSTATUS_PLAYING) &&
                     ((current_playState == PLAYSTATUS_PAUSED) ||
                     (current_playState == PLAYSTATUS_STOPPED))) {
                    Log.v(TAG, "Send play status for avrcp blacklist device");
                } else {
                    if (deviceFeatures[deviceIndex].isPlayStatusTimeOut) {
                        Log.v(TAG, "Sending play status after timeout");
                        playState = convertPlayStateToPlayStatus(mCurrentPlayerState);
                    } else {
                 /* IOT fix as some remote device just depends on playback state in CHANGED response
                  * to update its playback status and trigger avrcp play/pause command. Somietimes,
                  * after foward or backward, DUT update PAUSED to remote in CHANGED response, then
                  * update PLAYING in get play status or playback INTERIM response, so that carkit
                  * display incorrect playback status or trigger incorrect PLAY/PAUSE command.
                  */
                        if (deviceFeatures[deviceIndex].mLastRspPlayStatus != playState &&
                              deviceFeatures[deviceIndex].mLastRspPlayStatus != -1) {
                            Log.w(TAG,"playback status has changed from last playback CHANGED response, " +
                                      "repsonse last CHANGED play status");
                            playState = deviceFeatures[deviceIndex].mLastRspPlayStatus;
                        }
                    }
                }
                if (DEBUG)
                    Log.v(TAG, "Play Status for : " + device.getName() +
                          " state: " + playState + " position: " + position);
                getPlayStatusRspNative(getByteAddress(device), playState, (int)mSongLengthMs, position);
                break;
            }

            case MSG_NATIVE_REQ_GET_ELEM_ATTRS:
            {
                String[] textArray;
                AvrcpCmd_ext.ElementAttrCmd elem = (AvrcpCmd_ext.ElementAttrCmd) msg.obj;
                byte numAttr = elem.mNumAttr;
                int[] attrIds = elem.mAttrIDs;
                byte[] remoteAddr = elem.mAddress;
                boolean blacklistAttr = false;
                if (DEBUG) Log.v(TAG, "MSG_NATIVE_REQ_GET_ELEM_ATTRS:numAttr=" + numAttr);
                textArray = new String[numAttr];
                for(int j=0;j< BlacklistDeviceAddrToMediaAttr.length;j++) {
                    String addr = BlacklistDeviceAddrToMediaAttr[j];
                    String device_addr = Utils.byteArrayToString(elem.mAddress);
                    device_addr = device_addr.replaceAll(" ", ":");
                    if(device_addr.toLowerCase().startsWith(addr.toLowerCase())) {
                        Log.d(TAG,"Blacklisted for set attribute as empty string");
                        blacklistAttr = true;
                        break;
                    }
                }
                StringBuilder responseDebug = new StringBuilder();
                responseDebug.append("getElementAttr response: ");
                BluetoothDevice device = null;
                if (mAvrcpBipRsp != null) {
                    device = mAvrcpBipRsp.getBluetoothDevice(remoteAddr);
                }
                for (int i = 0; i < numAttr; ++i) {
                    textArray[i] = mMediaAttributes.getString(device, attrIds[i]);
                    if(blacklistAttr) {
                        if(attrIds[i] == MediaAttributes.ATTR_MEDIA_NUMBER
                           && textArray[i].equals("0"))
                            textArray[i]= new String();
                        else if(attrIds[i] == MediaAttributes.ATTR_MEDIA_TOTAL_NUMBER
                                && textArray[i].equals("0"))
                            textArray[i]= new String();
                    }
                    responseDebug.append("[" + attrIds[i] + "=");
                    if (attrIds[i] == AvrcpConstants_ext.ATTRID_TITLE
                            || attrIds[i] == AvrcpConstants_ext.ATTRID_ARTIST
                            || attrIds[i] == AvrcpConstants_ext.ATTRID_ALBUM) {
                        responseDebug.append(Utils.ellipsize(textArray[i]) + "] ");
                    } else {
                        responseDebug.append(textArray[i] + "] ");
                    }
                }
                Log.v(TAG, responseDebug.toString());
                byte[] bdaddr = elem.mAddress;
                getElementAttrRspNative(bdaddr, numAttr, attrIds, textArray);
                break;
            }

            case MSG_NATIVE_REQ_REGISTER_NOTIFICATION:
                if (DEBUG) Log.v(TAG, "MSG_NATIVE_REQ_REGISTER_NOTIFICATION:event=" + msg.arg1 +
                        " param=" + msg.arg2);
                processRegisterNotification((byte[]) msg.obj, msg.arg1, msg.arg2);
                break;

            case MSG_NOW_PLAYING_CHANGED_RSP:
                if (DEBUG) Log.v(TAG, "MSG_NOW_PLAYING_CHANGED_RSP");
                removeMessages(MSG_NOW_PLAYING_CHANGED_RSP);
                updateCurrentMediaState(null);
                break;

            case MSG_PLAY_INTERVAL_TIMEOUT:
            case MSG_PLAY_INTERVAL_TIMEOUT_2:
                if (DEBUG) Log.v(TAG, "MSG_PLAY_INTERVAL_TIMEOUT");
                Log.v(TAG, "event for device address " + (BluetoothDevice)msg.obj);
                deviceIndex = getIndexForDevice((BluetoothDevice) msg.obj);
                if (deviceIndex == INVALID_DEVICE_INDEX) {
                    Log.e(TAG,"invalid index for device");
                    break;
                }
                sendPlayPosNotificationRsp(false, deviceIndex);
                break;

            case MSG_NATIVE_REQ_VOLUME_CHANGE:
            {
                Bundle data = msg.getData();
                byte[] bdaddr = data.getByteArray("BdAddress");
                String address = Utils.getAddressStringFromByte(bdaddr);
                BluetoothDevice device, activeDevice = null;
                Log.v(TAG, "event for device address " + address);
                deviceIndex = getIndexForDevice(mAdapter.getRemoteDevice(address));
                if (deviceIndex == INVALID_DEVICE_INDEX) {
                    Log.e(TAG,"invalid index for device");
                    break;
                }
                device = deviceFeatures[deviceIndex].mCurrentDevice;
                activeDevice = mA2dpService.getActiveDevice();
                byte absVol = (byte) ((byte) msg.arg1 & 0x7f); // discard MSB as it is RFD
                if (DEBUG) Log.v(TAG, "MSG_NATIVE_REQ_VOLUME_CHANGE addr: " + address);

                if (((!(activeDevice != null && (isTwsPlusPair(activeDevice, device)
                    || Objects.equals(device, activeDevice)))) &&
                    (deviceFeatures[deviceIndex].mInitialRemoteVolume != -1)) ||
                    (!deviceFeatures[deviceIndex].isAbsoluteVolumeSupportingDevice)) {
                        if (deviceFeatures[deviceIndex].isAbsoluteVolumeSupportingDevice) {
                           deviceFeatures[deviceIndex].mRemoteVolume = absVol;
                        }
                        if (DEBUG) Log.v(TAG, "MSG_NATIVE_REQ_VOLUME_CHANGE ignored");
                        break;
                }
                int absolutevol = absVol;
                if (DEBUG)
                    Log.v(TAG, "MSG_NATIVE_REQ_VOLUME_CHANGE: volume=" + absVol + " ctype="
                                    + msg.arg2 + " local vol " + deviceFeatures[deviceIndex].mLocalVolume +
                                    " remote vol " + deviceFeatures[deviceIndex].mRemoteVolume);
                Log.v(TAG, "last local Volume = " + deviceFeatures[deviceIndex].mLastLocalVolume +
                    " last remote Volume = " + deviceFeatures[deviceIndex].mLastRemoteVolume +
                    " last requested volume = " + deviceFeatures[deviceIndex].mLastRequestedVolume +
                    " cmd set = " + deviceFeatures[deviceIndex].mVolCmdSetInProgress +
                    " cmd adjust = " + deviceFeatures[deviceIndex].mVolCmdAdjustInProgress);
                if ((deviceFeatures[deviceIndex].mVolCmdSetInProgress ||
                        deviceFeatures[deviceIndex].mVolCmdAdjustInProgress) &&
                        (deviceFeatures[deviceIndex].mLastRequestedVolume != -1) &&
                        (deviceFeatures[deviceIndex].mLastRequestedVolume != absolutevol)) {
                    Log.v(TAG, "send cached lastreq vol = " +
                        deviceFeatures[deviceIndex].mLastRequestedVolume);
                    boolean isSetVol =
                        setVolumeNative(deviceFeatures[deviceIndex].mLastRequestedVolume,
                        getByteAddress(deviceFeatures[deviceIndex].mCurrentDevice));
                    if (isSetVol) {
                        removeMessages(MSG_ABS_VOL_TIMEOUT);
                        deviceFeatures[deviceIndex].mLastRemoteVolume =
                                deviceFeatures[deviceIndex].mLastRequestedVolume;
                        sendMessageDelayed(obtainMessage(MSG_ABS_VOL_TIMEOUT,
                                 0, 0, deviceFeatures[deviceIndex].mCurrentDevice),
                                 CMD_TIMEOUT_DELAY);
                        deviceFeatures[deviceIndex].mLastRequestedVolume = -1;
                        deviceFeatures[deviceIndex].mLocalVolume = convertToAudioStreamVolume(absVol);
                        Log.v(TAG, "Reset cached lastreq vol = " +
                            deviceFeatures[deviceIndex].mLastRequestedVolume);
                    }
                    break;
                }
                if (msg.arg2 == AVRC_RSP_ACCEPT || msg.arg2 == AVRC_RSP_REJ) {
                    if ((deviceFeatures[deviceIndex].mVolCmdAdjustInProgress == false) &&
                        (deviceFeatures[deviceIndex].mVolCmdSetInProgress == false)) {
                        if (deviceFeatures[deviceIndex].mCurrentDevice.isTwsPlusDevice()) {
                            Log.e(TAG,"Store volume for TWS+ pair for volume relays");
                            deviceFeatures[deviceIndex].mRemoteVolume = absVol;
                            deviceFeatures[deviceIndex].mLocalVolume = convertToAudioStreamVolume(absVol);
                            break;
                        }
                        Log.e(TAG, "Unsolicited response, ignored");
                        break;
                    }
                    removeMessages(MSG_ABS_VOL_TIMEOUT);

                    deviceFeatures[deviceIndex].mVolCmdAdjustInProgress = false;
                    deviceFeatures[deviceIndex].mVolCmdSetInProgress = false;
                    deviceFeatures[deviceIndex].mAbsVolRetryTimes = 0;
                }

                if (msg.arg2 == AVRC_RSP_INTERIM && areMultipleDevicesConnected() &&
                    !(activeDevice != null && Objects.equals(deviceFeatures[deviceIndex].mCurrentDevice, activeDevice)) &&
                    deviceFeatures[deviceIndex].mInitialRemoteVolume == -1 &&
                    deviceFeatures[deviceIndex].mCurrentDevice.isTwsPlusDevice()) {
                    device = deviceFeatures[deviceIndex].mCurrentDevice;
                    for(int i = 0; i < maxAvrcpConnections; i++) {
                        BluetoothDevice conn_dev = deviceFeatures[i].mCurrentDevice;
                        if (i != deviceIndex && deviceFeatures[i].mCurrentDevice != null &&
                            deviceFeatures[i].mInitialRemoteVolume != -1 &&
                            isTwsPlusPair(conn_dev, device)) {
                            Log.v(TAG,"TWS+ pair found at index " + i +
                               "mTwsPairDisconnected = " + deviceFeatures[i].mTwsPairDisconnected);
                            if (deviceFeatures[i].mTwsPairDisconnected) {
                                Log.v(TAG,"TWS+ pair was disconnected earlier");
                                Log.v(TAG,"TWS+ store this volume");
                                deviceFeatures[i].mTwsPairDisconnected = false;
                            } else {
                                Log.v(TAG,"volume already set for tws pair");
                                deviceFeatures[deviceIndex].mInitialRemoteVolume = absVol;
                                deviceFeatures[deviceIndex].mRemoteVolume = absVol;
                                deviceFeatures[deviceIndex].mLocalVolume = convertToAudioStreamVolume(absVol);
                                break;
                            }
                        }
                    }
                }
                // convert remote volume to local volume
                int volIndex = convertToAudioStreamVolume(absVol);
                boolean isShowUI = true;
                if (DEBUG) Log.v(TAG,"Volume Index = " + volIndex);
                if (deviceFeatures[deviceIndex].mInitialRemoteVolume == -1) {
                    //Don't show media UI when device connected.
                    isShowUI = false;
                    deviceFeatures[deviceIndex].mInitialRemoteVolume = absVol;
                    //Avoid fluctuation of volume during device added in blacklist
                    // use send setAbsolute volume for blacklisted volume
                    if (deviceFeatures[deviceIndex].mBlackListVolume != -1 &&
                       activeDevice != null &&
                       Objects.equals(activeDevice, deviceFeatures[deviceIndex].mCurrentDevice)) {
                        resetBlackList(address);
                        if (DEBUG) Log.v(TAG, "remote initial volume as audio stream volume : " +
                            deviceFeatures[deviceIndex].mBlackListVolume);
                        Message msg1 = mHandler.obtainMessage(MSG_SET_ABSOLUTE_VOLUME,
                        deviceFeatures[deviceIndex].mBlackListVolume, 0);
                        mHandler.sendMessage(msg1);
                        deviceFeatures[deviceIndex].mRemoteVolume = absVol;
                        /*copying blacklisted volume beacuse already streaming is going on same volume*/
                        deviceFeatures[deviceIndex].mLocalVolume = deviceFeatures[deviceIndex].mBlackListVolume;
                        deviceFeatures[deviceIndex].mBlackListVolume = -1;
                        break;
                    } else if (activeDevice != null &&
                          (Objects.equals(activeDevice, device) || isTwsPlusPair(activeDevice, device))) {
                        /*Avoid send set absolute volume for store volume untill volume registration
                        complete and making synchronization to send only one setAbsolute volume
                        during connection*/
                        int val = getVolume(deviceFeatures[deviceIndex].mCurrentDevice);
                        if (val != volIndex) {
                            deviceFeatures[deviceIndex].mRemoteVolume = absVol;
                            setAbsVolumeFlag(deviceFeatures[deviceIndex].mCurrentDevice);
                            break;
                        }
                    }
                }
                if (deviceFeatures[deviceIndex].mLocalVolume != volIndex &&
                                                 (msg.arg2 == AVRC_RSP_CHANGED ||
                                                 msg.arg2 == AVRC_RSP_INTERIM ||
                                                 msg.arg2 == AVRC_RSP_ACCEPT)) {
                    if (msg.arg2 == AVRC_RSP_ACCEPT){
                        Log.d(TAG, "Don't show media UI when slide volume bar");
                        isShowUI = false;
                    }
                    if (device.isTwsPlusDevice() &&
                        activeDevice != null && !activeDevice.isTwsPlusDevice()) {
                        Log.d(TAG,"TWS+ device is not active, ignore volume change type: " + msg.arg2);
                        break;
                    }
                    /* If the volume has successfully changed */
                    if (!(activeDevice != null &&
                        (isTwsPlusPair(activeDevice, device) ||
                        Objects.equals(activeDevice, device))) &&
                        (msg.arg2 == AVRC_RSP_CHANGED || msg.arg2 == AVRC_RSP_INTERIM)) {
                        Log.d(TAG, "Do not change volume from an inactive device");
                        break;
                    }

                    deviceFeatures[deviceIndex].mLocalVolume = volIndex;
                    deviceFeatures[deviceIndex].mLastRequestedVolume = -1;
                    if (deviceFeatures[deviceIndex].mLastLocalVolume != -1
                        && msg.arg2 == AVRC_RSP_ACCEPT) {
                        if (deviceFeatures[deviceIndex].mLastLocalVolume != volIndex) {
                            /* remote volume changed more than requested due to
                             * local and remote has different volume steps */
                            if (DEBUG) Log.d(TAG, "Remote returned volume does not match desired volume "
                                + deviceFeatures[deviceIndex].mLastLocalVolume + " vs "
                                + volIndex);
                            deviceFeatures[deviceIndex].mLastLocalVolume =
                                deviceFeatures[deviceIndex].mLocalVolume;
                        }
                    }
                    notifyVolumeChanged(deviceFeatures[deviceIndex].mLocalVolume, isShowUI);
                    deviceFeatures[deviceIndex].mRemoteVolume = absVol;
                    long pecentVolChanged = ((long)absVol * 100) / 0x7f;
                    Log.e(TAG, "percent volume changed: " + pecentVolChanged + "%");
                } else if (deviceFeatures[deviceIndex].mLastRemoteVolume > 0
                            && deviceFeatures[deviceIndex].mLastRemoteVolume < AVRCP_MAX_VOL &&
                            deviceFeatures[deviceIndex].mLocalVolume == volIndex &&
                            (msg.arg2 == AVRC_RSP_ACCEPT )) {
                    /* oops, the volume is still same, remote does not like the value
                     * retry a volume one step up/down */
                    if (DEBUG) Log.d(TAG, "Remote device didn't tune volume, let's try one more step.");
                    /* direction calculation for retry mechanism */
                    int retry_volume = Math.min(AVRCP_MAX_VOL,
                            Math.max(0, deviceFeatures[deviceIndex].mLastRemoteVolume +
                                        deviceFeatures[deviceIndex].mLastDirection));
                    if ((deviceFeatures[deviceIndex].mCurrentDevice != null) && setVolumeNative(retry_volume,
                            getByteAddress(deviceFeatures[deviceIndex].mCurrentDevice))) {
                        deviceFeatures[deviceIndex].mLastRemoteVolume = retry_volume;
                        sendMessageDelayed(obtainMessage(MSG_ABS_VOL_TIMEOUT,
                            0, 0, deviceFeatures[deviceIndex].mCurrentDevice), CMD_TIMEOUT_DELAY);
                        deviceFeatures[deviceIndex].mVolCmdAdjustInProgress = true;
                    }
                } else if (msg.arg2 == AVRC_RSP_REJ) {
                    if (DEBUG)
                        Log.v(TAG, "setAbsoluteVolume call rejected");
                }
            }   break;

            case MSG_SET_ABSOLUTE_VOLUME:
            {

                if (DEBUG) Log.v(TAG, "MSG_SET_ABSOLUTE_VOLUME");

                int avrcpVolume = convertToAvrcpVolume(msg.arg1);
                BluetoothDevice activeDevice = mA2dpService.getActiveDevice();
                avrcpVolume = Math.min(AVRCP_MAX_VOL, Math.max(0, avrcpVolume));
                for (int i = 0; i < maxAvrcpConnections; i++) {
                    if (deviceFeatures[i].mCurrentDevice != null && activeDevice != null &&
                        (isTwsPlusPair(activeDevice, deviceFeatures[i].mCurrentDevice) ||
                         Objects.equals(activeDevice, deviceFeatures[i].mCurrentDevice)) &&
                         deviceFeatures[i].isAbsoluteVolumeSupportingDevice) {

                          deviceIndex = i;

                          if ((deviceFeatures[deviceIndex].mVolCmdSetInProgress) ||
                                (deviceFeatures[deviceIndex].mVolCmdAdjustInProgress)){
                              deviceFeatures[deviceIndex].mLastRequestedVolume = avrcpVolume;
                              Log.w(TAG, "There is already a volume command in progress cache = " +
                                  deviceFeatures[deviceIndex].mLastRequestedVolume);
                              continue;
                          }
                          if (deviceFeatures[deviceIndex].mInitialRemoteVolume == -1) {
                              if (!deviceFeatures[deviceIndex].mCurrentDevice.isTwsPlusDevice()) {
                                  if (DEBUG) Log.d(TAG, "remote never tell us initial volume, black list it. vol: "+ msg.arg1);
                                  deviceFeatures[deviceIndex].mBlackListVolume = msg.arg1;
                                  blackListCurrentDevice(deviceIndex);
                                  break;
                              } else {
                                  if (DEBUG) Log.d(TAG, "TWS+ device, intial volume not notified yet");
                                  continue;
                              }
                          }
                          Log.v(TAG, "event for device address " + getByteAddress(deviceFeatures[deviceIndex].mCurrentDevice));
                          boolean isSetVol = setVolumeNative(avrcpVolume ,
                                getByteAddress(deviceFeatures[deviceIndex].mCurrentDevice));
                          if (isSetVol) {
                              sendMessageDelayed(obtainMessage(MSG_ABS_VOL_TIMEOUT,
                                   0, 0, deviceFeatures[deviceIndex].mCurrentDevice),
                                   CMD_TIMEOUT_DELAY);
                              deviceFeatures[deviceIndex].mVolCmdSetInProgress = true;
                              deviceFeatures[deviceIndex].mLastRemoteVolume = avrcpVolume;
                              if(deviceFeatures[deviceIndex].mLastLocalVolume >  msg.arg1)
                                  deviceFeatures[deviceIndex].mLastDirection = -1;
                              else
                                  deviceFeatures[deviceIndex].mLastDirection = 1;
                              deviceFeatures[deviceIndex].mLastLocalVolume = msg.arg1;
                              deviceFeatures[deviceIndex].mLastRequestedVolume = -1;
                         } else {
                            if (DEBUG) Log.d(TAG, "setVolumeNative failed");
                         }
                    }
                }
                break;
            }
            case MSG_ABS_VOL_TIMEOUT:
                if (DEBUG) Log.v(TAG, "MSG_ABS_VOL_TIMEOUT: Volume change cmd timed out.");
                deviceIndex = getIndexForDevice((BluetoothDevice) msg.obj);
                if (deviceIndex == INVALID_DEVICE_INDEX) {
                    Log.e(TAG,"invalid device index for abs vol timeout");
                    for (int i = 0; i < maxAvrcpConnections; i++) {
                        if (deviceFeatures[i].mVolCmdSetInProgress == true)
                            deviceFeatures[i].mVolCmdSetInProgress = false;
                        if (deviceFeatures[i].mVolCmdAdjustInProgress == true)
                            deviceFeatures[i].mVolCmdAdjustInProgress = false;
                    }
                    break;
                }
                deviceFeatures[deviceIndex].mVolCmdSetInProgress = false;
                deviceFeatures[deviceIndex].mVolCmdAdjustInProgress = false;
                Log.v(TAG, "event for device address " + (BluetoothDevice)msg.obj);
                if (deviceFeatures[deviceIndex].mAbsVolRetryTimes >= MAX_ERROR_RETRY_TIMES) {
                    deviceFeatures[deviceIndex].mAbsVolRetryTimes = 0;
                    blackListCurrentDevice(deviceIndex);
                } else {
                    deviceFeatures[deviceIndex].mAbsVolRetryTimes += 1;
                    boolean isSetVol = setVolumeNative(deviceFeatures[deviceIndex].mLastRemoteVolume ,
                            getByteAddress((BluetoothDevice) msg.obj));
                    if (isSetVol) {
                        sendMessageDelayed(obtainMessage(MSG_ABS_VOL_TIMEOUT,
                                0, 0, msg.obj), CMD_TIMEOUT_DELAY);
                        deviceFeatures[deviceIndex].mVolCmdSetInProgress = true;
                    }
                }
                break;

            case MSG_SET_A2DP_AUDIO_STATE: {
                if (DEBUG) Log.v(TAG, "MSG_SET_A2DP_AUDIO_STATE:" + msg.arg1);
                mA2dpState = msg.arg1;
                BluetoothDevice playStateChangeDevice = (BluetoothDevice)msg.obj;
                Log.v(TAG, "event for device address " + playStateChangeDevice.getAddress());

                if (mA2dpState == BluetoothA2dp.STATE_PLAYING) {
                    boolean shoComplete = false;
                    synchronized(Avrcp_ext.this) {
                        if(isShoActive) {
                            isShoActive = false;
                            shoComplete = true;
                            Log.e(TAG, "1: SHO complete");
                        }

                        if(mHandler.hasMessages(MESSAGE_START_SHO) && (!SHOQueue.isRetry)) {
                            mHandler.removeMessages(MESSAGE_START_SHO);
                            triggerSHO(SHOQueue.device, SHOQueue.PlayReq, false);
                        }
                    }
                    if(shoComplete == true) {
                        CompleteSHO();
                    }
                }

                deviceIndex = getIndexForDevice(playStateChangeDevice);
                if (deviceIndex == INVALID_DEVICE_INDEX) {
                    Log.e(TAG,"Set A2DP state: invalid index for device");
                    break;
                }
                // if BA streaming is ongoing, don't update AVRCP state based on A2DP State.
                // This is for some remote devices, which send PLAY/PAUSE based on AVRCP State.
                BATService mBatService = BATService.getBATService();
                if ((mBatService == null) || !mBatService.isA2dpSuspendFromBA()) {
                  // if this suspend was triggered by BA, then don't update AVRCP states
                  updateCurrentMediaState((BluetoothDevice)msg.obj);
                }

              }
              break;

            case MESSAGE_DEVICE_RC_CLEANUP:
                if (DEBUG)
                    Log.v(TAG, "MESSAGE_DEVICE_RC_CLEANUP");
                    clearDeviceDependentFeature();
                    for (int i = 0; i < maxAvrcpConnections; i++) {
                       cleanupDeviceFeaturesIndex(i);
                    }
                break;

            case MSG_NATIVE_REQ_GET_FOLDER_ITEMS: {
                AvrcpCmd_ext.FolderItemsCmd folderObj = (AvrcpCmd_ext.FolderItemsCmd) msg.obj;
                if (DEBUG) Log.v(TAG, "MSG_NATIVE_REQ_GET_FOLDER_ITEMS " + folderObj);
                switch (folderObj.mScope) {
                    case AvrcpConstants_ext.BTRC_SCOPE_PLAYER_LIST:
                        handleMediaPlayerListRsp(folderObj);
                        break;
                    case AvrcpConstants_ext.BTRC_SCOPE_FILE_SYSTEM:
                    case AvrcpConstants_ext.BTRC_SCOPE_NOW_PLAYING:
                        handleGetFolderItemBrowseResponse(folderObj, folderObj.mAddress);
                        break;
                    default:
                        Log.e(TAG, "unknown scope for getfolderitems. scope = "
                                + folderObj.mScope);
                        getFolderItemsRspNative(folderObj.mAddress,
                                AvrcpConstants_ext.RSP_INV_SCOPE, (short) 0, (byte) 0, 0,
                                null, null, null, null, null, null, null, null);
                }
                break;
            }

            case MSG_NATIVE_REQ_SET_ADDR_PLAYER:
                // object is bdaddr, argument 1 is the selected player id
                if (DEBUG) Log.v(TAG, "MSG_NATIVE_REQ_SET_ADDR_PLAYER id=" + msg.arg1);
                setAddressedPlayer((byte[]) msg.obj, msg.arg1);
                break;

            case MSG_NATIVE_REQ_GET_ITEM_ATTR:
                // msg object contains the item attribute object
                AvrcpCmd_ext.ItemAttrCmd cmd = (AvrcpCmd_ext.ItemAttrCmd) msg.obj;
                if (DEBUG) Log.v(TAG, "MSG_NATIVE_REQ_GET_ITEM_ATTR " + cmd);
                handleGetItemAttr(cmd);
                break;

            case MSG_NATIVE_REQ_SET_BR_PLAYER:
                // argument 1 is the selected player id
                if (DEBUG) Log.v(TAG, "MSG_NATIVE_REQ_SET_BR_PLAYER id=" + msg.arg1);
                setBrowsedPlayer((byte[]) msg.obj, msg.arg1);
                break;

            case MSG_NATIVE_REQ_CHANGE_PATH:
            {
                if (DEBUG) Log.v(TAG, "MSG_NATIVE_REQ_CHANGE_PATH" + " changePathDepth " +
                            changePathDepth + " changePathFolderType " + changePathFolderType);
                Bundle data = msg.getData();
                byte[] bdaddr = data.getByteArray("BdAddress");
                byte[] folderUid = data.getByteArray("folderUid");
                byte direction = data.getByte("direction");
                byte[] tempUid = new byte[AvrcpConstants_ext.UID_SIZE];
                if (mAvrcpBrowseManager.getBrowsedMediaPlayer(bdaddr) != null) {
                    changePathDirection = direction;
                    if(direction == 1)
                       changePathDepth++;
                    mAvrcpBrowseManager.getBrowsedMediaPlayer(bdaddr).changePath(folderUid,
                        direction);
                    if ((direction == 1) && (changePathDepth > 0)) {
                       for (int index=0; index<saveRspObj.mDisplayNames.length; index++) {
                            for (int size=0; size < AvrcpConstants_ext.UID_SIZE; size++)
                                tempUid[size] = saveRspObj.mItemUid[index* AvrcpConstants_ext.UID_SIZE + size];
                            if (Arrays.equals(folderUid, tempUid)) {
                                changePathFolderType = saveRspObj.mFolderTypes[index];
                                break;
                            }
                       }
                    }
                    else if ((direction == 0) && (changePathDepth>0)) {
                       changePathDepth--;
                       if (changePathDepth == 0)
                            changePathFolderType = 0;
                    }

                } else {
                    Log.e(TAG, "Remote requesting change path before setbrowsedplayer");
                    changePathRspNative(bdaddr, AvrcpConstants_ext.RSP_BAD_CMD, 0);
                }
                break;
            }

            case MSG_NATIVE_REQ_PLAY_ITEM:
            {
                Bundle data = msg.getData();
                byte[] bdaddr = data.getByteArray("BdAddress");
                byte[] uid = data.getByteArray("uid");
                byte scope = data.getByte("scope");
                if (DEBUG)
                    Log.v(TAG, "MSG_NATIVE_REQ_PLAY_ITEM scope=" + scope + " id="
                                    + Utils.byteArrayToString(uid));
                handlePlayItemResponse(bdaddr, uid, scope);
                break;
            }

            case MSG_NATIVE_REQ_GET_TOTAL_NUM_OF_ITEMS:
                if (DEBUG) Log.v(TAG, "MSG_NATIVE_REQ_GET_TOTAL_NUM_OF_ITEMS scope=" + msg.arg1);
                // argument 1 is scope, object is bdaddr
                handleGetTotalNumOfItemsResponse((byte[]) msg.obj, (byte) msg.arg1);
                break;

            case MSG_NATIVE_REQ_PASS_THROUGH:
                if (DEBUG)
                    Log.v(TAG, "MSG_NATIVE_REQ_PASS_THROUGH: id=" + msg.arg1 + " st=" + msg.arg2);
                // argument 1 is id, argument 2 is keyState
                Bundle data = msg.getData();
                byte[] bdaddr = data.getByteArray("BdAddress");
                String address = Utils.getAddressStringFromByte(bdaddr);
                Log.v(TAG, "MSG_NATIVE_REQ_PASS_THROUGH " + address);
                handlePassthroughCmd(bdaddr, msg.arg1, msg.arg2);
                break;

            case MESSAGE_SET_MEDIA_SESSION:
                android.media.session.MediaController mMediaController =
                    (android.media.session.MediaController)msg.obj;
                setActiveMediaSession(mMediaController);
                break;

            case MSG_SET_AVRCP_CONNECTED_DEVICE:
                BluetoothDevice device = (BluetoothDevice) msg.obj;
                if (msg.arg1 == BluetoothProfile.STATE_CONNECTED) {
                    setAvrcpConnectedDevice(device);
                } else {
                    if (DEBUG) Log.d(TAG," mAvrcpBipRsp :" + mAvrcpBipRsp);
                    if (mAvrcpBipRsp != null) {
                        mAvrcpBipRsp.disconnect(device);
                    }
                    setAvrcpDisconnectedDevice(device);
                }
                break;

            case MSG_PLAY_STATUS_CMD_TIMEOUT:
                deviceIndex = getIndexForDevice((BluetoothDevice) msg.obj);
                if (deviceIndex == INVALID_DEVICE_INDEX) {
                    Log.e(TAG,"invalid device index for Play status timeout");
                    break;
                }
                deviceFeatures[deviceIndex].isPlayStatusTimeOut = true;
                break;

            case MESSAGE_START_SHO:
                synchronized (Avrcp_ext.this) {
                    if(mHandler.hasMessages(MESSAGE_START_SHO)) {
                        Log.e(TAG, "Queue already has another SHO pending");
                        break;
                    }
                    isShoActive = true;
                    Log.d(TAG, "2: SHO started. PlayReq = " + msg.arg1 + "SHO Retry = " + msg.arg2);
                }

                BluetoothDevice dev = (BluetoothDevice)msg.obj;
                boolean PlayReq = (msg.arg1 == 1);
                boolean isRetry = (msg.arg2 == 1);
                boolean ret;
                BluetoothDevice prevActiveDevice = mA2dpService.getActiveDevice();
                if(Objects.equals(dev, prevActiveDevice)) {
                    isShoActive = false;
                    Log.w(TAG, dev + " device is already active");
                    break;
                }
                ret = mA2dpService.startSHO(dev);
                if(!ret) {
                    isShoActive = false;
                    if(isRetry) {
                        Log.e(TAG, "SHO failed");
                    } else {
                        triggerSHO(dev, PlayReq, true);
                    }
                    break;
                }

                if(!PlayReq) {
                    synchronized (Avrcp_ext.this) {
                        isShoActive = false;
                        Log.d(TAG, "3: SHO complete");
                        if (mHandler.hasMessages(MESSAGE_START_SHO) && (!SHOQueue.isRetry)) {
                            mHandler.removeMessages(MESSAGE_START_SHO);
                            triggerSHO(SHOQueue.device, SHOQueue.PlayReq, false);
                        }
                    }
                    CompleteSHO();
                }
                break;

            case MSG_SET_ACTIVE_DEVICE:
                boolean tws_switch = false;
                Log.d(TAG, "MSG_SET_ACTIVE_DEVICE");
                BluetoothDevice bt_device = (BluetoothDevice) msg.obj;
                if (bt_device == null) {
                    for (int i = 0; i < maxAvrcpConnections; i++) {
                        deviceFeatures[i].isActiveDevice = false;
                    }
                    break;
                }
                if (bt_device != null && bt_device.isTwsPlusDevice()) {
                    for (int i = 0; i < maxAvrcpConnections; i++) {
                        if (deviceFeatures[i].mCurrentDevice != null &&
                            deviceFeatures[i].mCurrentDevice.isTwsPlusDevice() &&
                            ((!deviceFeatures[i].isActiveDevice &&
                            !Objects.equals(bt_device, deviceFeatures[i].mCurrentDevice)) ||
                            (deviceFeatures[i].isActiveDevice &&
                             Objects.equals(bt_device, deviceFeatures[i].mCurrentDevice)))) {
                            /* If bt_device is already active then it is active device swithc
                            ** between TWS+ device.
                            ** If bt_device is not active and other TWS+ pair is not active then
                            ** it is a new connection
                            */
                            tws_switch = true;
                        }
                    }
                }
                deviceIndex = getIndexForDevice(bt_device);
                if (deviceIndex == INVALID_DEVICE_INDEX) {
                    Log.e(TAG,"Invalid device index for setActiveDevice");
                    for (int i = 0; i < maxAvrcpConnections; i++) {
                        deviceFeatures[i].isActiveDevice = false;
                    }
                    break;
                }
                deviceFeatures[deviceIndex].isActiveDevice = true;
                if (deviceFeatures[deviceIndex].cache_play_cmd) {
                  process_cached_play(deviceIndex);
                }

                if (mFastforward)  mFastforward = false;
                if (mRewind)  mRewind = false;

                if (mCurrentBrowsingDevice == null && mCachedBrowsePlayer != null &&
                        is_player_updated_for_browse == false) {
                    Log.w(TAG, "Calling SetBrowsePackage as part of device switch for "
                            + mCachedBrowsePlayer);
                    SetBrowsePackage(mCachedBrowsePlayer);
                }

                if (deviceFeatures[deviceIndex].mCurrentDevice.isTwsPlusDevice() &&
                        updateAbsVolume == true) {
                    Log.d(TAG,"setting absVolume flag for TWS+ device");
                    mAudioManager.avrcpSupportsAbsoluteVolume(bt_device.getAddress(),true);
                    AdapterService mAdapterService = AdapterService.getAdapterService();
                    BluetoothDevice peer_device = mAdapterService.
                            getTwsPlusPeerDevice(deviceFeatures[deviceIndex].mCurrentDevice);
                    if (peer_device != null &&
                            getIndexForDevice(peer_device) == INVALID_DEVICE_INDEX) {
                        Log.d(TAG,"Other TWS+ earbud not connected, reset updateAbsVolume flag");
                        updateAbsVolume = false;
                    }
                }
                if (bt_device.isTwsPlusDevice() && !tws_switch) {
                    Log.d(TAG,"Reseting mTwsPairDisconnected at index " + deviceIndex);
                    deviceFeatures[deviceIndex].mTwsPairDisconnected = false;
                    for (int i = 0; i < maxAvrcpConnections; i++) {
                        if (i != deviceIndex && deviceFeatures[i].mCurrentDevice != null &&
                            deviceFeatures[i].mCurrentDevice.isTwsPlusDevice()) {
                            deviceFeatures[i].mTwsPairDisconnected = false;
                            break;
                        }
                    }
                }
                if (maxAvrcpConnections > 1) {
                    for (int i = 0; i < maxAvrcpConnections; i++) {
                        if (deviceIndex != i && deviceFeatures[i].mCurrentDevice != null &&
                                deviceFeatures[i].mCurrentDevice.isTwsPlusDevice() &&
                                isTwsPlusPair(deviceFeatures[i].mCurrentDevice, bt_device)) {
                            Log.d(TAG,"TWS+ pair connected, keep both devices active");
                            deviceFeatures[i].isActiveDevice = true;
                            if (updateAbsVolume == true) {
                                Log.d(TAG,"Setting absVolume flag to TWS+ pair");
                                mAudioManager.avrcpSupportsAbsoluteVolume(
                                        bt_device.getAddress(), true);
                                updateAbsVolume = false;
                            }
                        } else {
                            if (deviceIndex != i)
                                deviceFeatures[i].isActiveDevice = false;
                        }
                    }
                }
                Log.e(TAG, "AVRCP isActive device index " + deviceIndex + " setActive addr " +
                            deviceFeatures[deviceIndex].mCurrentDevice.getAddress());

                //to keep volume copy for setting volume
                deviceFeatures[deviceIndex].mLocalVolume = getVolume(bt_device);
                if((maxAvrcpConnections > 1) && (deviceFeatures[deviceIndex].mCurrentDevice != null)
                        && (deviceFeatures[deviceIndex].mReportedPlayerID != mCurrAddrPlayerID)) {
                    Log.d(TAG,"Update cache browsing event to last active device, deviceFeatures[" +
                            deviceIndex + "].mReportedPlayerID: " +
                            deviceFeatures[deviceIndex].mReportedPlayerID +
                            ", mCurrAddrPlayerID: " + mCurrAddrPlayerID);
                    if (deviceFeatures[deviceIndex].mAvailablePlayersChangedNT ==
                            AvrcpConstants.NOTIFICATION_TYPE_INTERIM) {
                        registerNotificationRspAvalPlayerChangedNative(
                                AvrcpConstants.NOTIFICATION_TYPE_CHANGED,
                                getByteAddress(deviceFeatures[deviceIndex].mCurrentDevice));
                        mAvailablePlayerViewChanged = false;
                        deviceFeatures[deviceIndex].mAvailablePlayersChangedNT =
                                AvrcpConstants.NOTIFICATION_TYPE_CHANGED;
                    }
                    if (deviceFeatures[deviceIndex].mAddrPlayerChangedNT ==
                            AvrcpConstants.NOTIFICATION_TYPE_INTERIM) {
                        registerNotificationRspAddrPlayerChangedNative(
                                AvrcpConstants.NOTIFICATION_TYPE_CHANGED,
                                mCurrAddrPlayerID, sUIDCounter,
                                getByteAddress(deviceFeatures[deviceIndex].mCurrentDevice));
                        deviceFeatures[deviceIndex].mAddrPlayerChangedNT =
                                AvrcpConstants.NOTIFICATION_TYPE_CHANGED;
                        // send track change event becasue some carkits will refresh metadata
                        // while receive addressed player change event. Track change event to
                        // make remote get metadata correctly.
                        sendTrackChangedRsp(false, deviceFeatures[deviceIndex].mCurrentDevice);
                    }

                    deviceFeatures[deviceIndex].mReportedPlayerID = mCurrAddrPlayerID;
                    break;
                }

            default:
                Log.e(TAG, "unknown message! msg.what=" + msg.what);
                break;
            }
            Log.v(TAG, "Exit handleMessage");
        }
    }

    private void process_cached_play(int index) {
        Log.d(TAG,"process_cached_play");
        deviceFeatures[index].cache_play_cmd = false;
        handlePassthroughCmd(getByteAddress(deviceFeatures[index].mCurrentDevice),
                             BluetoothAvrcp.PASSTHROUGH_ID_PLAY,
                             AvrcpConstants_ext.KEY_STATE_PRESS);
        Message msg = mHandler.obtainMessage(MSG_NATIVE_REQ_PASS_THROUGH,
                                             BluetoothAvrcp.PASSTHROUGH_ID_PLAY,
                                             AvrcpConstants_ext.KEY_STATE_RELEASE);
        Bundle data = new Bundle();
        data.putByteArray("BdAddress", getByteAddress(deviceFeatures[index].mCurrentDevice));
        msg.setData(data);
        mHandler.sendMessageDelayed(msg,10);
    }

    private void updatePlayStatusForDevice(int deviceIndex, PlaybackState state) {
        if (state == null) {
            Log.i(TAG,"updatePlayStatusForDevice: device: state is =" + state);
            return;
        }
        Log.i(TAG,"updatePlayStatusForDevice: device: " +
                    deviceFeatures[deviceIndex].mCurrentDevice);

        byte newStatus = getBluetoothPlayState(state);

        /* update play status in global media player list */
        MediaPlayerInfo_ext player = getAddressedPlayerInfo();
        if (player != null) {
            player.setPlayStatus(newStatus);
        }

        int newPlayStatus = convertPlayStateToPlayStatus(state);
        int oldPlayStatus = convertPlayStateToPlayStatus(deviceFeatures[deviceIndex].mCurrentPlayState);

        if (mFastforward) {
            newPlayStatus = PLAYSTATUS_FWD_SEEK;
        }
        if (mRewind) {
            newPlayStatus = PLAYSTATUS_REV_SEEK;
        }
        if(avrcp_playstatus_blacklist)
        {
            if ((deviceFeatures[deviceIndex].mCurrentDevice != null) &&
                isPlayerStateUpdateBlackListed(
                    deviceFeatures[deviceIndex].mCurrentDevice.getAddress(),
                    deviceFeatures[deviceIndex].mCurrentDevice.getName()) &&
               ((newPlayStatus == PLAYSTATUS_PAUSED) ||
               (newPlayStatus == PLAYSTATUS_STOPPED))) {

                if (mA2dpService.getConnectedDevices().size() > 0) {
                    if (mA2dpService.isA2dpPlaying(
                        deviceFeatures[deviceIndex].mCurrentDevice)) {
                        Log.w(TAG, " A2dp Playing, do not update/save playstatus");
                        return;
                    }
                } else {
                    if(mAudioManager.isMusicActive()) {
                        Log.w(TAG, "A2dp disconnected, but music active, don't update/save playstatus");
                        return;
                    }
                }
            }
        }
        if (DEBUG) {
            Log.v(TAG, "updatePlaybackState (" + deviceFeatures[deviceIndex].mPlayStatusChangedNT + "): "+
                       "old=" + deviceFeatures[deviceIndex].mCurrentPlayState + "(" + oldPlayStatus + "), "+
                       "new=" + state + "(" + newPlayStatus + ")");
        }

        deviceFeatures[deviceIndex].mCurrentPlayState = state;
        deviceFeatures[deviceIndex].mLastPassthroughcmd = KeyEvent.KEYCODE_UNKNOWN;

        if ((deviceFeatures[deviceIndex].mPlayStatusChangedNT ==
                AvrcpConstants_ext.NOTIFICATION_TYPE_INTERIM) &&
               (oldPlayStatus != newPlayStatus) && deviceFeatures[deviceIndex].mCurrentDevice != null) {
            Log.w(TAG, "Sending PlayStatus CHANGED Rsp !!!");
            deviceFeatures[deviceIndex].mPlayStatusChangedNT =
                AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
            registerNotificationRspPlayStatusNative(
                    deviceFeatures[deviceIndex].mPlayStatusChangedNT,
                    newPlayStatus,
                    getByteAddress(deviceFeatures[deviceIndex].mCurrentDevice));
            deviceFeatures[deviceIndex].mLastRspPlayStatus = newPlayStatus;
            if (!deviceFeatures[deviceIndex].isPlayStatusTimeOut) {
                Message msg = mHandler.obtainMessage(MSG_PLAY_STATUS_CMD_TIMEOUT,
                                         0, 0, deviceFeatures[deviceIndex].mCurrentDevice);
                mHandler.sendMessageDelayed(msg, CMD_TIMEOUT_DELAY);
            }
        }
        Log.i(TAG,"Exit updatePlayStatusForDevice");
    }

    private boolean isPlayStateToBeUpdated(int deviceIndex) {
        Log.v(TAG, "isPlayStateTobeUpdated: device: "  +
                    deviceFeatures[deviceIndex].mCurrentDevice);
        if (maxAvrcpConnections < 2) {
            Log.v(TAG, "maxAvrcpConnections: " + maxAvrcpConnections);
            return true;
        /*} else if(mA2dpService.isMulticastFeatureEnabled()) {
            if (!areMultipleDevicesConnected()) {
                Log.v(TAG, "Single connection exists");
                return true;
            } else if (mA2dpService.isMulticastEnabled()) {
                Log.v(TAG, "Multicast is Enabled");
                return true;
            } else {
                Log.v(TAG, "Multiple connection exists, Multicast not enabled");
                if((deviceFeatures[deviceIndex].mCurrentDevice != null) &&
                   isDeviceActiveInHandOffNative(getByteAddress(
                            deviceFeatures[deviceIndex].mCurrentDevice))) {
                    Log.v(TAG, "Device Active in handoff scenario");
                    return true;
                } else {
                    Log.v(TAG, "Device Not Active in handoff scenario");
                    return false;
                }
            }*/
        } else {
            if (!areMultipleDevicesConnected()) {
                Log.v(TAG, "Single connection exists");
                if (deviceFeatures[deviceIndex].isActiveDevice) {
                   Log.v(TAG, "isPlayStateToBeUpdated device is active device");
                   return true;
                }
                return false;
            } else {
                Log.v(TAG, "Multiple connection exists in handoff");
                if((deviceFeatures[deviceIndex].mCurrentDevice != null) &&
                   isDeviceActiveInHandOffNative(getByteAddress(
                            deviceFeatures[deviceIndex].mCurrentDevice))) {
                    Log.v(TAG, "Device Active in handoff scenario");
                    return true;
                } else {
                    Log.v(TAG, "Device Not Active in handoff scenario");
                    return false;
                }
            }
        }
    }

    private boolean isPlayerPaused() {
        if (mCurrentPlayerState == null)
            return false;

        int state = mCurrentPlayerState.getState();
        Log.d(TAG, "isPlayerPaused: state=" + state);

        return (state == PlaybackState.STATE_PAUSED ||
            state == PlaybackState.STATE_STOPPED ||
            state == PlaybackState.STATE_NONE);
    }


    private boolean areMultipleDevicesConnected() {
        int connections = 0;
        for (int deviceIndex = 0; deviceIndex < maxAvrcpConnections; deviceIndex++) {
            if (deviceFeatures[deviceIndex].mCurrentDevice != null) {
                connections++;
            }
        }
        if (connections > 1) return true;
        else return false;
    }
    private void updatePlayerStateAndPosition(PlaybackState state) {
        if (DEBUG) Log.v(TAG, "updatePlayerStateAndPosition, old=" +
                            mCurrentPlayerState + ", state=" + state);
        if (state == null) {
            Log.i(TAG,"updatePlayerStateAndPosition: device: state = " + state);
            return;
        }

        if (DEBUG) Log.v(TAG, "old state = " + mCurrentPlayerState + ", new state= " + state);
        int oldPlayStatus = convertPlayStateToPlayStatus(mCurrentPlayerState);
        int newPlayStatus = convertPlayStateToPlayStatus(state);

        mCurrentPlayerState = state;
        mLastStateUpdate = SystemClock.elapsedRealtime();

        HeadsetService headsetService = HeadsetService.getHeadsetService();
        for (int deviceIndex = 0; deviceIndex < maxAvrcpConnections; deviceIndex++) {
            /*Discretion is required only when updating play state changed as playing*/
            boolean isInCall = headsetService != null && headsetService.isScoOrCallActive();
            if ((state.getState() != PlaybackState.STATE_PLAYING) ||
                                isPlayStateToBeUpdated(deviceIndex) && !isInCall) {
                updatePlayStatusForDevice(deviceIndex, state);
                deviceFeatures[deviceIndex].mLastStateUpdate = mLastStateUpdate;
            } else if ((state.getState() == PlaybackState.STATE_PLAYING) &&
                        deviceFeatures[deviceIndex].mLastRspPlayStatus == PLAYSTATUS_PLAYING &&
                        !isPlayStateToBeUpdated(deviceIndex)) {
                PlaybackState.Builder playState = new PlaybackState.Builder();
                playState.setState(PlaybackState.STATE_PAUSED,
                                          PlaybackState.PLAYBACK_POSITION_UNKNOWN, 1.0f);
                Log.i(TAG,"Updating playstatus PAUSE to inactive devices");
                updatePlayStatusForDevice(deviceIndex, playState.build());
                deviceFeatures[deviceIndex].mLastStateUpdate = mLastStateUpdate;
            }
        }
        if ((state.getState() == PlaybackState.STATE_PLAYING) &&
                (newPlayStatus != oldPlayStatus)) {
            Log.i(TAG,"No active device found, update playstate to stack");
            updatePlayStatusToStack(newPlayStatus);
        }
        for (int deviceIndex = 0; deviceIndex < maxAvrcpConnections; deviceIndex++) {
            sendPlayPosNotificationRsp(false, deviceIndex);
        }
        Log.v(TAG, "Exit updatePlayerStateAndPosition");
    }

    private void updatePlaybackState(PlaybackState state, BluetoothDevice device) {
        Log.v(TAG,"updatePlaybackState, state: " + state + " device: " + device);
        for (int i = 0; i < maxAvrcpConnections; i++) {
            Log.v(TAG,"Device: " + ((deviceFeatures[i].mCurrentDevice == null) ?
                "no name: " : deviceFeatures[i].mCurrentDevice.getName() +
                " : old state: " + deviceFeatures[i].mCurrentPlayState));
        }
        if (device == null) {
            /*Called because of player state change*/
            updatePlayerStateAndPosition(state);
            return;
        } else {
            int deviceIndex = getIndexForDevice(device);
            if (deviceIndex == INVALID_DEVICE_INDEX) {
                Log.w(TAG,"invalid device index" +
                        "Play status change for not connected device");
            } else {
                Log.v(TAG, "old state: " + deviceFeatures[deviceIndex].mCurrentPlayState
                            + " new state: " + state + " device: " +
                            device + " index: " + deviceIndex);
                updatePlayStatusForDevice(deviceIndex, state);
                for (int i = 0; i < maxAvrcpConnections; i++) {
                     if (i != deviceIndex && !deviceFeatures[i].isActiveDevice &&
                          deviceFeatures[i].mLastRspPlayStatus == PLAYSTATUS_PLAYING &&
                          (state != null && state.getState() == PlaybackState.STATE_PLAYING) &&
                          (!isTwsPlusPair(deviceFeatures[i].mCurrentDevice, deviceFeatures[deviceIndex].mCurrentDevice))) {
                         PlaybackState.Builder playState = new PlaybackState.Builder();
                         playState.setState(PlaybackState.STATE_PAUSED,
                                          PlaybackState.PLAYBACK_POSITION_UNKNOWN, 1.0f);
                         Log.i(TAG,"Updating PAUSE to inactive devices");
                         updatePlayStatusForDevice(i, playState.build());
                     }
                }
            }
        }
        Log.v(TAG,"Exit updatePlaybackState");
    }

    private void sendPlaybackStatus(int playStatusChangedNT, byte playbackState) {
        registerNotificationRspPlayStatusNative(playStatusChangedNT, (byte) playbackState, null);
        mPlayStatusChangedNT = playStatusChangedNT;
        mReportedPlayStatus = playbackState;
    }

    private void updateTransportControls(int transportControlFlags) {
        mTransportControlFlags = transportControlFlags;
    }

    class MediaAttributes {
        private boolean exists;
        private String title;
        private String artistName;
        private String albumName;
        private String mediaNumber;
        private String mediaTotalNumber;
        private String genre;
        private long playingTimeMs;
        private String coverArt;

        private static final int ATTR_TITLE = 1;
        private static final int ATTR_ARTIST_NAME = 2;
        private static final int ATTR_ALBUM_NAME = 3;
        private static final int ATTR_MEDIA_NUMBER = 4;
        private static final int ATTR_MEDIA_TOTAL_NUMBER = 5;
        private static final int ATTR_GENRE = 6;
        private static final int ATTR_PLAYING_TIME_MS = 7;
        private static final int ATTR_COVER_ART = 8;


        public MediaAttributes(BluetoothDevice device, MediaMetadata data) {
            synchronized (this) {
                exists = data != null;
                if (!exists) {
                    return;
                }
                String CurrentPackageName = (mMediaController != null) ? mMediaController.getPackageName():null;
                artistName = stringOrBlank(data.getString(MediaMetadata.METADATA_KEY_ARTIST));
                albumName = stringOrBlank(data.getString(MediaMetadata.METADATA_KEY_ALBUM));
                if (albumName.isEmpty()) {
                  albumName =
                      stringOrBlank(data.getString(MediaMetadata.METADATA_KEY_ALBUM_ARTIST));
                  Log.d(TAG,"overwrite album_artist :" + albumName + "if album is blank");
                }
                if (CurrentPackageName != null && !(CurrentPackageName.equals("com.android.music"))) {
                    mediaNumber = longStringOrBlank((data.getLong(MediaMetadata.METADATA_KEY_TRACK_NUMBER)));
                } else {
                    /* playlist starts with 0 for default player*/
                    mediaNumber = longStringOrBlank((data.getLong(MediaMetadata.METADATA_KEY_TRACK_NUMBER) + 1L));
                }
                mediaTotalNumber = longStringOrBlank(data.getLong(MediaMetadata.METADATA_KEY_NUM_TRACKS));
                genre = stringOrBlank(data.getString(MediaMetadata.METADATA_KEY_GENRE));
                playingTimeMs = data.getLong(MediaMetadata.METADATA_KEY_DURATION);
                Log.d(TAG," albumName :" + albumName + " device :" + device);
                if (mAvrcpBipRsp != null && device != null)
                    coverArt = stringOrBlank(mAvrcpBipRsp.getImgHandle(device, albumName));
                else coverArt = stringOrBlank(null);

                // Try harder for the title.
                title = data.getString(MediaMetadata.METADATA_KEY_TITLE);

                if (title == null) {
                    MediaDescription desc = data.getDescription();
                    if (desc != null) {
                        CharSequence val = desc.getDescription();
                        if (val != null)
                            title = val.toString();
                    }
                }

                if (title != null && CurrentPackageName != null &&
                        CurrentPackageName.equals("com.tencent.qqmusic")) {
                    title = title.trim();
                }

                if (title == null)
                    title = new String();
            }
        }

        public synchronized long getLength() {
            if (!exists) return 0L;
            return playingTimeMs;
        }

        public synchronized boolean equals(MediaAttributes other) {
            if (other == null)
                return false;

            if (exists != other.exists)
                return false;

            if (exists == false)
                return true;

            return (title.equals(other.title)) && (artistName.equals(other.artistName))
                    && (albumName.equals(other.albumName))
                    && (mediaNumber.equals(other.mediaNumber))
                    && (mediaTotalNumber.equals(other.mediaTotalNumber))
                    && (genre.equals(other.genre))
                    && (playingTimeMs == other.playingTimeMs);
        }

        public synchronized String getString(BluetoothDevice device, int attrId) {
            if (!exists)
                return new String();

            switch (attrId) {
                case ATTR_TITLE:
                    return title;
                case ATTR_ARTIST_NAME:
                    return artistName;
                case ATTR_ALBUM_NAME:
                    return albumName;
                case ATTR_MEDIA_NUMBER:
                    return mediaNumber;
                case ATTR_MEDIA_TOTAL_NUMBER:
                    return mediaTotalNumber;
                case ATTR_GENRE:
                    return genre;
                case ATTR_PLAYING_TIME_MS:
                    return Long.toString(playingTimeMs);
                case ATTR_COVER_ART:
                    if (mAvrcpBipRsp != null && device != null) {
                        /* Fetch coverArt Handle now in case OBEX channel is established just
                        * before retrieving get element attribute. */
                        coverArt = stringOrBlank(mAvrcpBipRsp.getImgHandle(device, albumName));
                    } else {
                        coverArt = stringOrBlank(null);
                    }
                    return coverArt;
                default:
                    return new String();
            }
        }

        private String stringOrBlank(String s) {
            return s == null ? new String() : s;
        }

        private String longStringOrBlank(Long s) {
            return s == null ? new String() : s.toString();
        }

        public String toString() {
            if (!exists) {
                return "[MediaAttributes: none]";
            }

            return "[MediaAttributes: " + title + " - " + albumName + " by " + artistName + " ("
                    + playingTimeMs + " " + mediaNumber + "/" + mediaTotalNumber + ") " + genre
                    + " - " + coverArt + "]";
        }

        public String toRedactedString() {
            if (!exists) {
                return "[MediaAttributes: none]";
            }

            return "[MediaAttributes: " + Utils.ellipsize(title) + " - "
                    + Utils.ellipsize(albumName) + " by " + Utils.ellipsize(artistName) + " ("
                    + playingTimeMs + " " + mediaNumber + "/" + mediaTotalNumber + ") " + genre
                    + "]";
        }
    }

    private void updateCurrentMediaState(BluetoothDevice device) {
        // Only do player updates when we aren't registering for track changes.
        MediaAttributes currentAttributes;
        boolean isPlaying = false;
        PlaybackState newState = new PlaybackState.Builder().setState(PlaybackState.STATE_NONE,
                                               PlaybackState.PLAYBACK_POSITION_UNKNOWN, 0.0f).build();
        boolean updateA2dpPlayState = false;
        Log.v(TAG,"updateCurrentMediaState: mMediaController: " + mMediaController);
        Log.v(TAG,"isMusicActive: " + mAudioManager.isMusicActive() + " getBluetoothPlayState: " + getBluetoothPlayState(mCurrentPlayerState));

        synchronized (this) {
            if (mMediaController == null ||
                device != null) { //Update playstate for a2dp play state change
                if (getBluetoothPlayState(mCurrentPlayerState) == PLAYSTATUS_PLAYING && (mA2dpState == BluetoothA2dp.STATE_PLAYING)) {
                    isPlaying = true;
                } else {
                    isPlaying = (mA2dpState == BluetoothA2dp.STATE_PLAYING) && mAudioManager.isMusicActive();
                }
                Log.v(TAG,"updateCurrentMediaState: isPlaying = " + isPlaying);
                // Use A2DP state if we don't have a MediaControlller
                PlaybackState.Builder builder = new PlaybackState.Builder();
                if (isPlaying) {
                    builder.setState(PlaybackState.STATE_PLAYING,
                            PlaybackState.PLAYBACK_POSITION_UNKNOWN, 1.0f);
                } else {
                    builder.setState(PlaybackState.STATE_PAUSED,
                            PlaybackState.PLAYBACK_POSITION_UNKNOWN, 0.0f);
                }
                if (mMediaController != null && mMediaController.getPlaybackState() != null) {
                    int mMediaPlayState = mMediaController.getPlaybackState().getState();
                    if (isPlaying) {
                        builder.setState(PlaybackState.STATE_PLAYING,
                                mMediaController.getPlaybackState().getPosition(), 1.0f);
                    } else {
                        builder.setState(PlaybackState.STATE_PAUSED,
                                mMediaController.getPlaybackState().getPosition(), 0.0f);
                    }
                }
                newState = builder.build();
                if (mMediaController == null)
                    currentAttributes = new MediaAttributes(device, null);
                else
                    currentAttributes = new MediaAttributes(device, mMediaController.getMetadata());
                for (int i = 0; i < maxAvrcpConnections; i++) {
                    if (device != null && deviceFeatures[i].mCurrentDevice != null) {
                        if ((isPlaying != isPlayingState(deviceFeatures[i].mCurrentPlayState)) &&
                            (Objects.equals(deviceFeatures[i].mCurrentDevice, device))) {
                            updateA2dpPlayState = true;
                            deviceFeatures[i].mLastStateUpdate = SystemClock.elapsedRealtime();
                        }
                    }
                }
            } else {
                newState = mMediaController.getPlaybackState();
                if (newState == null) {
                    Log.d(TAG, "updateCurrentMediaState: playState is null, mAudioManagerIsPlaying=" + mAudioManagerIsPlaying
                               + ", isMusicActive=" + mAudioManager.isMusicActive());
                    PlaybackState.Builder builder = new PlaybackState.Builder();
                    if (mAudioManagerIsPlaying && mAudioManager.isMusicActive()) {
                        builder.setState(PlaybackState.STATE_PLAYING,
                                PlaybackState.PLAYBACK_POSITION_UNKNOWN, 1.0f);
                        newState = builder.build();
                    } else if (!mAudioManagerIsPlaying) {
                        builder.setState(PlaybackState.STATE_PAUSED,
                                PlaybackState.PLAYBACK_POSITION_UNKNOWN, 0.0f);
                        newState = builder.build();
                    }
                }
                Log.v(TAG,"updateCurrentMediaState: get media attributes: ");
                currentAttributes = new MediaAttributes(device, mMediaController.getMetadata());
            }
        }

        if (mMediaController != null) {
            MediaMetadata data = mMediaController.getMetadata();
            mSongLengthMs = (data != null) ? (data.getLong(MediaMetadata.METADATA_KEY_DURATION)):0L;
        } else {
            mSongLengthMs = 0L;
        }

        byte newPlayStatus = getBluetoothPlayState(newState);

        byte[] addr = null;
        int index = INVALID_DEVICE_INDEX;
        if (device == null) {
            for (int i = 0; i < maxAvrcpConnections; i++) {
                if ((deviceFeatures[i].mCurrentDevice != null) &&
                     deviceFeatures[i].isActiveDevice) {
                    addr = getByteAddress(deviceFeatures[i].mCurrentDevice);
                    index = getIndexForDevice(deviceFeatures[i].mCurrentDevice);
                    Log.v(TAG,"updateCurrentMediaState: addr: " + addr);
                    break;
                }
            }
        } else {
            addr = getByteAddress(device);
            index = getIndexForDevice(device);
        }

        if (newState != null && newState.getState() != PlaybackState.STATE_BUFFERING
                 && newState.getState() != PlaybackState.STATE_NONE) {
            long newQueueId = MediaSession.QueueItem.UNKNOWN_ID;
            if (newState != null) newQueueId = newState.getActiveQueueItemId();
            Log.v(TAG, "Media update: id " + mLastQueueId + "➡" + newQueueId + "? "
                            + currentAttributes.toRedactedString() + " : "
                            + mMediaAttributes.toRedactedString());

            // Update available/addressed player for current active device, but cached the player
            // update for inactive device until device switch and inactive device become active.
            if (addr != null && index != INVALID_DEVICE_INDEX &&
                    deviceFeatures[index].mReportedPlayerID != mCurrAddrPlayerID) {
                Log.v(TAG, "Update player id: " + deviceFeatures[index].mReportedPlayerID +
                        "-> " + mCurrAddrPlayerID);
                String currPkg = getPackageName(mCurrAddrPlayerID);
                String prevPkg = getPackageName(deviceFeatures[index].mReportedPlayerID);
                if ((currPkg != null && !currPkg.isEmpty() &&
                        playerBrowseWhiteListDB.contains(currPkg))
                        || (prevPkg != null && !prevPkg.isEmpty() &&
                        playerBrowseWhiteListDB.contains(prevPkg))) {
                    Log.w(TAG, "Send change response as part of player switch");
                    if (deviceFeatures[index].mAvailablePlayersChangedNT ==
                                AvrcpConstants_ext.NOTIFICATION_TYPE_INTERIM) {
                        registerNotificationRspAvalPlayerChangedNative(
                                AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED, addr);
                        mAvailablePlayerViewChanged = false;
                        deviceFeatures[index].mAvailablePlayersChangedNT =
                                AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
                    }
                    if (deviceFeatures[index].mAddrPlayerChangedNT ==
                                AvrcpConstants_ext.NOTIFICATION_TYPE_INTERIM) {
                        registerNotificationRspAddrPlayerChangedNative(
                                AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED, mCurrAddrPlayerID,
                                sUIDCounter, addr);
                        deviceFeatures[index].mAddrPlayerChangedNT =
                                AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
                    }

                    // Update the now playing list without sending the notification
                    deviceFeatures[index].mNowPlayingListChangedNT =
                            AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
                    mAddressedMediaPlayer.updateNowPlayingList(mMediaController);
                    deviceFeatures[index].mNowPlayingListChangedNT =
                            AvrcpConstants_ext.NOTIFICATION_TYPE_INTERIM;
                }
                deviceFeatures[index].mReportedPlayerID = mCurrAddrPlayerID;
            }

            // Dont send now playing list changed if the player doesn't support browsing
            MediaPlayerInfo_ext info = getAddressedPlayerInfo();
            String pkg = (info != null) ? info.getPackageName():"";
            boolean isPlayerInBrowseList = (pkg != null) && (!pkg.isEmpty()) &&
                    playerBrowseWhiteListDB.contains(pkg);
            if (isPlayerInBrowseList) {
                Log.v(TAG, "Check if NowPlayingList is updated");
                mAddressedMediaPlayer.updateNowPlayingList(mMediaController);
            }
            Log.v(TAG, "newPlayStatus:" + newPlayStatus + "mReportedPlayStatus:" + mReportedPlayStatus);
            // Notify track changed if:
            //  - The CT is registered for the notification
            //  - Queue ID is UNKNOWN and MediaMetadata is different
            if (((newQueueId == -1 || newQueueId != mLastQueueId)
                    && !currentAttributes.equals(mMediaAttributes))) {
                Log.v(TAG, "Send track changed");
                mMediaAttributes = currentAttributes;
                mLastQueueId = newQueueId;
                for (int i = 0; i < maxAvrcpConnections; i++) {
                    if ((deviceFeatures[i].mCurrentDevice != null) &&
                        (deviceFeatures[i].mTrackChangedNT == AvrcpConstants_ext.NOTIFICATION_TYPE_INTERIM)) {
                         deviceFeatures[i].mTracksPlayed++;
                         Log.v(TAG,"sending track change for device " + i);
                         sendTrackChangedRsp(false, deviceFeatures[i].mCurrentDevice);
                    }
                }
            }
        } else {
            Log.i(TAG, "Skipping update due to invalid playback state");
        }

        // still send the updated play state if the playback state is none or buffering

        if (device == null || updateA2dpPlayState) {
            if (device == null && newState != null && (newState.getState() ==
                    PlaybackState.STATE_NONE) &&
                    (getBluetoothPlayState(mCurrentPlayerState) ==
                    PLAYSTATUS_PLAYING || mAudioManager.isMusicActive())
                    && (mA2dpState == BluetoothA2dp.STATE_PLAYING)) {
                Log.i(TAG, "Players updated current playback state is none," +
                            " skip updating playback state");
            } else if (device == null && newState != null &&
                    newState.getState() == PlaybackState.STATE_ERROR) {
                Log.i(TAG, "Players updated current playback state is error," +
                            " skip updating playback state");
            } else {
                updatePlaybackState(newState, device);
            }
        }

        if (updateA2dpPlayState && newState != null && newState.getState() == PlaybackState.STATE_PLAYING) {
            for (int i = 0; i < maxAvrcpConnections; i++) {
                if (device != null && (deviceFeatures[i].mCurrentDevice != null) &&
                          Objects.equals(deviceFeatures[i].mCurrentDevice, device))
                    sendPlayPosNotificationRsp(false, i);
            }
        }
    }

    private void getRcFeaturesRequestFromNative(byte[] address, int features) {
        if (DEBUG) Log.v(TAG, "getRcFeaturesRequestFromNative: address=" + address.toString());
        Message msg = mHandler.obtainMessage(MSG_NATIVE_REQ_GET_RC_FEATURES, features, 0,
                Utils.getAddressStringFromByte(address));
        mHandler.sendMessage(msg);
        Log.v(TAG, "Exit getRcFeaturesRequestFromNative");
    }

    private void getPlayStatusRequestFromNative(byte[] address) {
        if (DEBUG) Log.v(TAG, "getPlayStatusRequestFromNative: address" + address.toString());
        Message msg = mHandler.obtainMessage(MSG_NATIVE_REQ_GET_PLAY_STATUS);
        msg.obj = address;
        mHandler.sendMessage(msg);
        Log.v(TAG, "Exit getPlayStatusRequestFromNative");
    }

    private void getElementAttrRequestFromNative(byte[] address, byte numAttr, int[] attrs) {
        if (DEBUG) Log.v(TAG, "getElementAttrRequestFromNative: numAttr=" + numAttr);
        AvrcpCmd_ext avrcpCmdobj = new AvrcpCmd_ext();
        AvrcpCmd_ext.ElementAttrCmd elemAttr = avrcpCmdobj.new ElementAttrCmd(address, numAttr, attrs);
        Message msg = mHandler.obtainMessage(MSG_NATIVE_REQ_GET_ELEM_ATTRS);
        msg.obj = elemAttr;
        mHandler.sendMessage(msg);
        Log.v(TAG, "Exit getElementAttrRequestFromNative");
    }

    private void registerNotificationRequestFromNative(byte[] address,int eventId, int param) {
        if (DEBUG) Log.v(TAG, "registerNotificationRequestFromNative: eventId=" + eventId);
        Message msg = mHandler.obtainMessage(MSG_NATIVE_REQ_REGISTER_NOTIFICATION, eventId, param);
        msg.obj = address;
        mHandler.sendMessage(msg);
        Log.v(TAG, "Exit registerNotificationRequestFromNative");
    }

    private void processRegisterNotification(byte[] address, int eventId, int param) {

        BluetoothDevice device = mAdapter.getRemoteDevice(address);
        int deviceIndex = getIndexForDevice(device);
        Log.v(TAG, "Enter processRegisterNotification");
        if (deviceIndex == INVALID_DEVICE_INDEX) {
            Log.v(TAG,"device entry not present, bailing out");
            return;
        }

        int currPlayState = convertPlayStateToPlayStatus
                (deviceFeatures[deviceIndex].mCurrentPlayState);

        if (mFastforward) {
            currPlayState = PLAYSTATUS_FWD_SEEK;
        }
        if (mRewind) {
            currPlayState = PLAYSTATUS_REV_SEEK;
        }

        Log.v(TAG,"processRegisterNotification: eventId" + eventId);
        switch (eventId) {
            case EVT_PLAY_STATUS_CHANGED:
                if (deviceFeatures[deviceIndex].mCurrentDevice == null) {
                  Log.e(TAG,"ERROR!!! current device is null");
                  return;
                }
                deviceFeatures[deviceIndex].mPlayStatusChangedNT =
                        AvrcpConstants_ext.NOTIFICATION_TYPE_INTERIM;
                mHandler.removeMessages(MSG_PLAY_STATUS_CMD_TIMEOUT);
                deviceFeatures[deviceIndex].isPlayStatusTimeOut = false;
                if(avrcp_playstatus_blacklist && isPlayerStateUpdateBlackListed(
                    deviceFeatures[deviceIndex].mCurrentDevice.getAddress(),
                    deviceFeatures[deviceIndex].mCurrentDevice.getName()) &&
                    ((currPlayState == PLAYSTATUS_PAUSED) ||
                     (currPlayState == PLAYSTATUS_STOPPED))) {
                    if (mA2dpService.getConnectedDevices().size() > 0) {
                        if (mA2dpService.isA2dpPlaying(deviceFeatures[deviceIndex].mCurrentDevice)) {
                            Log.w(TAG, "A2dp playing, Interim update playstatus to" +
                                " carkit as PLAYING: " +
                                deviceFeatures[deviceIndex].mCurrentDevice.getAddress() +
                                " playState " + currPlayState);
                                currPlayState = PLAYSTATUS_PLAYING;
                        }
                    } else {
                        if (mAudioManager.isMusicActive()) {
                            Log.w(TAG, "A2dp not connected, but music active, Interim update" +
                                " playstatus to carkit as PLAYING: " +
                                deviceFeatures[deviceIndex].mCurrentDevice.getAddress() +
                                " playState " + currPlayState);
                            currPlayState = PLAYSTATUS_PLAYING;
                        }
                    }
                }
                if ((deviceFeatures[deviceIndex].mLastRspPlayStatus != currPlayState) &&
                    (deviceFeatures[deviceIndex].mLastRspPlayStatus != -1)) {
                    registerNotificationRspPlayStatusNative(
                                deviceFeatures[deviceIndex].mPlayStatusChangedNT,
                                deviceFeatures[deviceIndex].mLastRspPlayStatus,
                                getByteAddress(deviceFeatures[deviceIndex].mCurrentDevice));

                    Log.d(TAG, "playback Status has changed from last playstatus response " +
                                    "send CHANGED event with current playback status");
                    deviceFeatures[deviceIndex].mPlayStatusChangedNT =
                                        AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
                    if (!deviceFeatures[deviceIndex].isPlayStatusTimeOut) {
                        Message msg = mHandler.obtainMessage(MSG_PLAY_STATUS_CMD_TIMEOUT,
                                                 0, 0, deviceFeatures[deviceIndex].mCurrentDevice);
                        mHandler.sendMessageDelayed(msg, CMD_TIMEOUT_DELAY);
                    }
                }
                else if ((currPlayState == PLAYSTATUS_PLAYING) &&
                    (deviceFeatures[deviceIndex].mLastRspPlayStatus == -1)) {
                    registerNotificationRspPlayStatusNative(
                                deviceFeatures[deviceIndex].mPlayStatusChangedNT,
                                PLAYSTATUS_STOPPED,
                                getByteAddress(deviceFeatures[deviceIndex].mCurrentDevice));
                    Log.v(TAG, "Sending Stopped in INTERIM response when current_play_status is playing and device just got connected");
                    deviceFeatures[deviceIndex].mPlayStatusChangedNT =
                                        AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
                    if (!deviceFeatures[deviceIndex].isPlayStatusTimeOut) {
                        Message msg = mHandler.obtainMessage(MSG_PLAY_STATUS_CMD_TIMEOUT,
                                                 0, 0, deviceFeatures[deviceIndex].mCurrentDevice);
                        mHandler.sendMessageDelayed(msg, CMD_TIMEOUT_DELAY);
                    }
                }
                registerNotificationRspPlayStatusNative(
                        deviceFeatures[deviceIndex].mPlayStatusChangedNT,
                        currPlayState,
                        getByteAddress(deviceFeatures[deviceIndex].mCurrentDevice));
                deviceFeatures[deviceIndex].mLastRspPlayStatus = currPlayState;
                break;

            case EVT_TRACK_CHANGED:
                Log.v(TAG, "Track changed notification enabled");
                if (deviceFeatures[deviceIndex].mCurrentDevice == null) {
                  Log.e(TAG,"ERROR!!! current device is null");
                  return;
                }
                deviceFeatures[deviceIndex].mTrackChangedNT =
                        AvrcpConstants_ext.NOTIFICATION_TYPE_INTERIM;
                sendTrackChangedRsp(true, deviceFeatures[deviceIndex].mCurrentDevice);
                break;

            case EVT_PLAY_POS_CHANGED:
                if (param <= 0)
                   param = 1;

                long update_interval = 0L;
                update_interval = SystemProperties.getLong("persist.vendor.btstack.avrcp.pos_time", 3000L);
                deviceFeatures[deviceIndex].mPlayPosChangedNT =
                                             AvrcpConstants_ext.NOTIFICATION_TYPE_INTERIM;
                update_interval = Math.max((long)param * 1000L, update_interval);
                deviceFeatures[deviceIndex].mPlaybackIntervalMs = update_interval;
                sendPlayPosNotificationRsp(true, deviceIndex);
                Log.v(TAG,"mPlayPosChangedNT updated for index " +
                      deviceFeatures[deviceIndex].mPlayPosChangedNT +
                      " index " + deviceIndex);
                break;

            case EVT_APP_SETTINGS_CHANGED:
                deviceFeatures[deviceIndex].mPlayerStatusChangeNT =
                        AvrcpConstants_ext.NOTIFICATION_TYPE_INTERIM;
                mAvrcpPlayerAppSettings.sendPlayerAppChangedRsp(
                        deviceFeatures[deviceIndex].mPlayerStatusChangeNT, device);
                break;

            case EVT_AVBL_PLAYERS_CHANGED:
                /* Notify remote available players changed */
                if (DEBUG) Log.d(TAG, "Available Players notification enabled");
                if (deviceFeatures[deviceIndex].mCurrentDevice == null) {
                  Log.e(TAG,"ERROR!!! current device is null");
                  return;
                }
                deviceFeatures[deviceIndex].mAvailablePlayersChangedNT =
                        AvrcpConstants_ext.NOTIFICATION_TYPE_INTERIM;
                registerNotificationRspAvalPlayerChangedNative(
                        AvrcpConstants_ext.NOTIFICATION_TYPE_INTERIM,
                        getByteAddress(deviceFeatures[deviceIndex].mCurrentDevice));
                break;

            case EVT_ADDR_PLAYER_CHANGED:
                /* Notify remote addressed players changed */
                if (DEBUG) Log.d(TAG, "Addressed Player notification enabled");
                if (deviceFeatures[deviceIndex].mCurrentDevice == null) {
                  Log.e(TAG,"ERROR!!! current device is null");
                  return;
                }
                deviceFeatures[deviceIndex].mAddrPlayerChangedNT =
                                             AvrcpConstants_ext.NOTIFICATION_TYPE_INTERIM;
                registerNotificationRspAddrPlayerChangedNative(
                        AvrcpConstants_ext.NOTIFICATION_TYPE_INTERIM,
                        mCurrAddrPlayerID, sUIDCounter,
                        getByteAddress(deviceFeatures[deviceIndex].mCurrentDevice));
                deviceFeatures[deviceIndex].mReportedPlayerID = mCurrAddrPlayerID;
                break;

            case EVENT_UIDS_CHANGED:
                if (DEBUG) Log.d(TAG, "UIDs changed notification enabled");
                if (deviceFeatures[deviceIndex].mCurrentDevice == null) {
                  Log.e(TAG,"ERROR!!! current device is null");
                  return;
                }
                deviceFeatures[deviceIndex].mUidsChangedNT =
                                             AvrcpConstants_ext.NOTIFICATION_TYPE_INTERIM;
                registerNotificationRspUIDsChangedNative(
                        AvrcpConstants_ext.NOTIFICATION_TYPE_INTERIM, sUIDCounter,
                        getByteAddress(deviceFeatures[deviceIndex].mCurrentDevice));
                break;

            case EVENT_NOW_PLAYING_CONTENT_CHANGED:
                if (DEBUG) Log.d(TAG, "Now Playing List changed notification enabled");
                if (deviceFeatures[deviceIndex].mCurrentDevice == null) {
                  Log.e(TAG,"ERROR!!! current device is null");
                  return;
                }
                /* send interim response to remote device */
                mNowPlayingListChangedNT = AvrcpConstants_ext.NOTIFICATION_TYPE_INTERIM;
                if (!registerNotificationRspNowPlayingChangedNative(
                        AvrcpConstants_ext.NOTIFICATION_TYPE_INTERIM,
                        getByteAddress(deviceFeatures[deviceIndex].mCurrentDevice))) {
                    Log.e(TAG, "EVENT_NOW_PLAYING_CONTENT_CHANGED: " +
                            "registerNotificationRspNowPlayingChangedNative for Interim rsp failed!");
                }
                break;
        }
        Log.d(TAG, "Exit processRegisterNotification");
    }

    private void handlePassthroughCmdRequestFromNative(byte[] address, int id, int keyState) {
        Message msg = mHandler.obtainMessage(MSG_NATIVE_REQ_PASS_THROUGH, id, keyState);
        Bundle data = new Bundle();
        Log.d(TAG, "Enter handlePassthroughCmdRequestFromNative");
        data.putByteArray("BdAddress" , address);
        msg.setData(data);
        mHandler.sendMessage(msg);
        Log.d(TAG, "Exit handlePassthroughCmdRequestFromNative");
    }

    private void sendTrackChangedRsp(boolean registering, BluetoothDevice device) {
        int deviceIndex = getIndexForDevice(device);
        Log.d(TAG, "Enter sendTrackChangedRsp");
        if (deviceFeatures[deviceIndex].mTrackChangedNT != AvrcpConstants_ext.NOTIFICATION_TYPE_INTERIM
                && !registering) {
            if (DEBUG) Log.d(TAG, "sendTrackChangedRsp: Not registered or registering.");
            return;
        }

        deviceFeatures[deviceIndex].mTrackChangedNT = AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
        if (registering)
            deviceFeatures[deviceIndex].mTrackChangedNT = AvrcpConstants_ext.NOTIFICATION_TYPE_INTERIM;

        byte[] byteAddr = getByteAddress(deviceFeatures[deviceIndex].mCurrentDevice);
        MediaPlayerInfo_ext info = getAddressedPlayerInfo();
        String currPkg = (info != null) ? info.getPackageName():"";
        boolean isPlayerInBrowseList = (currPkg != null) && (!currPkg.isEmpty()) &&
                playerBrowseWhiteListDB.contains(currPkg);
        // for non-browsable players or no player
        if (!isPlayerInBrowseList ||
                (deviceFeatures[deviceIndex].mFeatures & BTRC_FEAT_BROWSE) == 0) {
            byte[] track = AvrcpConstants_ext.TRACK_IS_SELECTED;
            if (!mMediaAttributes.exists) track = AvrcpConstants_ext.NO_TRACK_SELECTED;
            registerNotificationRspTrackChangeNative(
                              deviceFeatures[deviceIndex].mTrackChangedNT,
                              track,
                              byteAddr);
            return;
        }

        mAddressedMediaPlayer.sendTrackChangeWithId(deviceFeatures[deviceIndex].mTrackChangedNT,
                mMediaController, byteAddr);
        Log.d(TAG, "Exit sendTrackChangedRsp");
    }

    private long getPlayPosition(BluetoothDevice device) {
        Log.d(TAG, "Enter getPlayPosition");
        long currPosition;
        if (device != null) {
            int deviceIndex = getIndexForDevice(device);
            if (deviceIndex == INVALID_DEVICE_INDEX) {
                Log.e(TAG,"Device index is not valid in getPlayPosition");
                return -1L;
            }

            if (deviceFeatures[deviceIndex].mCurrentPlayState == null) {
                Log.d(TAG, "getPlayPosition, deviceFeatures[" + deviceIndex + "].mCurrentPlayState is null");
                return -1L;
            }

            if (deviceFeatures[deviceIndex].mCurrentPlayState.getPosition() ==
                    PlaybackState.PLAYBACK_POSITION_UNKNOWN) {
                Log.d(TAG, "getPlayPosition, deviceFeatures[" + deviceIndex + "] currentPosition is unknown");
                if (isPlayingState(deviceFeatures[deviceIndex].mCurrentPlayState))
                    return 0L;
                else
                    return -1L;
            }

            if (isPlayingState(deviceFeatures[deviceIndex].mCurrentPlayState) && !isPlayerPaused()) {
                long sinceUpdate =
                     SystemClock.elapsedRealtime() - deviceFeatures[deviceIndex].mLastStateUpdate;
                currPosition = sinceUpdate + deviceFeatures[deviceIndex].mCurrentPlayState.getPosition();
            } else {
                if (isPlayingState(deviceFeatures[deviceIndex].mCurrentPlayState) &&
                    (avrcp_playstatus_blacklist && isPlayerStateUpdateBlackListed(
                     deviceFeatures[deviceIndex].mCurrentDevice.getAddress(),
                     deviceFeatures[deviceIndex].mCurrentDevice.getName()))) {
                 currPosition = mCurrentPlayerState.getPosition();
                 Log.d(TAG, "Remote is BLed for playstatus, So send playposition by fetching from "+
                                   "mCurrentPlayerState." + currPosition);
                } else {
                  currPosition = deviceFeatures[deviceIndex].mCurrentPlayState.getPosition();
                  Log.d(TAG, "getPlayPosition(): currPosition = " + currPosition);
                }
            }
        } else {
            if (mCurrentPlayerState == null) {
                Log.d(TAG, "getPlayPosition, mCurrentPlayState is null");
                return -1L;
            }

            if (mCurrentPlayerState.getPosition() == PlaybackState.PLAYBACK_POSITION_UNKNOWN) {
                Log.d(TAG, "getPlayPosition, currentPosition is unknown");
                if (isPlayingState(mCurrentPlayerState))
                    return 0L;
                else
                    return -1L;
            }
            if (isPlayingState(mCurrentPlayerState)) {
                long sinceUpdate =
                    (SystemClock.elapsedRealtime() - mCurrentPlayerState.getLastPositionUpdateTime());
                currPosition = SystemClock.elapsedRealtime() - mLastStateUpdate +
                       mCurrentPlayerState.getPosition();
            } else {
                currPosition = mCurrentPlayerState.getPosition();
            }

        }
        if (mMediaAttributes.playingTimeMs >= 0 && currPosition > mMediaAttributes.playingTimeMs)
            currPosition = mMediaAttributes.playingTimeMs;
        Log.d(TAG, "Exit getPlayPosition, position: " + currPosition);
        return currPosition;
    }

    private int convertPlayStateToPlayStatus(PlaybackState state) {
        int playStatus = PLAYSTATUS_ERROR;
        switch (state.getState()) {
            case PlaybackState.STATE_PLAYING:
                playStatus = PLAYSTATUS_PLAYING;
                break;

            case PlaybackState.STATE_STOPPED:
            case PlaybackState.STATE_CONNECTING:
            case PlaybackState.STATE_NONE:
                playStatus = PLAYSTATUS_STOPPED;
                break;

            case PlaybackState.STATE_PAUSED:
            case PlaybackState.STATE_BUFFERING:
                playStatus = PLAYSTATUS_PAUSED;
                break;

            case PlaybackState.STATE_FAST_FORWARDING:
            case PlaybackState.STATE_SKIPPING_TO_NEXT:
            case PlaybackState.STATE_SKIPPING_TO_QUEUE_ITEM:
                playStatus = PLAYSTATUS_FWD_SEEK;
                break;

            case PlaybackState.STATE_REWINDING:
            case PlaybackState.STATE_SKIPPING_TO_PREVIOUS:
                playStatus = PLAYSTATUS_REV_SEEK;
                break;

            case PlaybackState.STATE_ERROR:
                playStatus = PLAYSTATUS_ERROR;
                break;

        }
        return playStatus;
    }

    private boolean isPlayingState(@Nullable PlaybackState state) {
        if (state == null) return false;
        return (state != null) && (state.getState() == PlaybackState.STATE_PLAYING);
    }

    private boolean isPausedState(@Nullable PlaybackState state) {
        if (state == null) return false;
        return (state != null) && (state.getState() == PlaybackState.STATE_PAUSED);
    }
    /**
     * Sends a play position notification, or schedules one to be
     * sent later at an appropriate time. If |requested| is true,
     * does both because this was called in reponse to a request from the
     * TG.
     */
    private void sendPlayPosNotificationRsp(boolean requested, int i) {
        Log.d(TAG, "Enter sendPlayPosNotificationRsp");
        if (!requested && deviceFeatures[i].mPlayPosChangedNT != AvrcpConstants_ext.NOTIFICATION_TYPE_INTERIM) {
            if (DEBUG) Log.d(TAG, "sendPlayPosNotificationRsp: Not registered or requesting.");
            return;
        }
        long playPositionMs = getPlayPosition(deviceFeatures[i].mCurrentDevice);
        int currPlayStatus = convertPlayStateToPlayStatus(deviceFeatures[i].mCurrentPlayState);
        String debugLine = "sendPlayPosNotificationRsp: ";
        int currMsgPlayIntervalTimeout = (i == 0) ? MSG_PLAY_INTERVAL_TIMEOUT : MSG_PLAY_INTERVAL_TIMEOUT_2;

        // Some remote devices are going to bad state when sending play position
        // as ffff for non-playing state
        if (!requested && playPositionMs == -1L && currPlayStatus != PLAYSTATUS_PLAYING) {
           if (DEBUG) Log.d(TAG, " Don't send invalid play position notification for non-playing state");
           return;
        }

        // mNextPosMs is set to -1 when the previous position was invalid
        // so this will be true if the new position is valid & old was invalid.
        // mPlayPositionMs is set to -1 when the new position is invalid,
        // and the old mPrevPosMs is >= 0 so this is true when the new is invalid
        // and the old was valid.
        if (DEBUG) {
            debugLine += "(" + requested + ") " + deviceFeatures[i].mPrevPosMs + " <=? " + playPositionMs + " <=? "
                    + deviceFeatures[i].mNextPosMs + " mLastReportedPosition " + deviceFeatures[i].mLastReportedPosition;
            if (isPlayingState(deviceFeatures[i].mCurrentPlayState)) debugLine += " Playing";
            debugLine += " State: " + deviceFeatures[i].mCurrentPlayState.getState();
        }
        if (requested || ((deviceFeatures[i].mLastReportedPosition != playPositionMs) &&
             ((playPositionMs >= deviceFeatures[i].mNextPosMs) ||
             (playPositionMs <= deviceFeatures[i].mPrevPosMs))) && deviceFeatures[i].isActiveDevice) {
            if (!requested)
                deviceFeatures[i].mPlayPosChangedNT = AvrcpConstants.NOTIFICATION_TYPE_CHANGED;
            if (deviceFeatures[i].mCurrentDevice != null) {
                if (!registerNotificationRspPlayPosNative(deviceFeatures[i].mPlayPosChangedNT,
                        (int)playPositionMs, getByteAddress(deviceFeatures[i].mCurrentDevice))) {
                    Log.w(TAG,"Fail to send rsp to remote, restore to interim if change rsp fail");
                    if (!requested) {
                        deviceFeatures[i].mPlayPosChangedNT =
                                AvrcpConstants.NOTIFICATION_TYPE_INTERIM;
                        return;
                    }
                }
            }
            deviceFeatures[i].mLastReportedPosition = playPositionMs;
            if (playPositionMs != PlaybackState.PLAYBACK_POSITION_UNKNOWN) {
                deviceFeatures[i].mNextPosMs = playPositionMs + deviceFeatures[i].mPlaybackIntervalMs;
                deviceFeatures[i].mPrevPosMs = playPositionMs - deviceFeatures[i].mPlaybackIntervalMs;
            } else {
                deviceFeatures[i].mNextPosMs = -1;
                deviceFeatures[i].mPrevPosMs = -1;
            }
        }

        mHandler.removeMessages(currMsgPlayIntervalTimeout);
        if ((deviceFeatures[i].mCurrentDevice != null) &&
            (deviceFeatures[i].mPlayPosChangedNT == AvrcpConstants_ext.NOTIFICATION_TYPE_INTERIM) &&
            (isPlayingState(deviceFeatures[i].mCurrentPlayState)) &&
             !isPlayerPaused() && deviceFeatures[i].isActiveDevice) {
            Message msg = mHandler.obtainMessage(currMsgPlayIntervalTimeout, 0, 0,
                                                 deviceFeatures[i].mCurrentDevice);
            long delay = deviceFeatures[i].mPlaybackIntervalMs;
            if (deviceFeatures[i].mNextPosMs != -1) {
                delay = deviceFeatures[i].mNextPosMs - (playPositionMs > 0 ? playPositionMs : 0);
            }
            if (DEBUG) debugLine += " Timeout " + delay + "ms";
            mHandler.sendMessageDelayed(msg, delay);
        }
        if (DEBUG) Log.d(TAG, debugLine);
        Log.d(TAG, "Exit sendPlayPosNotificationRsp");
    }
    private boolean isTwsPlusDeviceConnected() {
        for (int i = 0; i < maxAvrcpConnections; i++) {
            if (deviceFeatures[i].mCurrentDevice != null &&
                deviceFeatures[i].mCurrentDevice.isTwsPlusDevice()) {
                Log.d(TAG,"TWS+ Device Connected");
                return true;
            }
        }
        return false;
    }
    /**
     * This is called from AudioService. It will return whether this device supports abs volume.
     * NOT USED AT THE MOMENT.
     */

 public boolean isAbsoluteVolumeSupported() {
        boolean status = false;
        BluetoothDevice activeDevice = mA2dpService.getActiveDevice();
        for (int i = 0; i < maxAvrcpConnections; i++) {
            if (deviceFeatures[i].mCurrentDevice != null && activeDevice != null &&
                (isTwsPlusPair(activeDevice, deviceFeatures[i].mCurrentDevice) ||
                Objects.equals(deviceFeatures[i].mCurrentDevice, activeDevice))) {
                if ((deviceFeatures[i].mFeatures &
                        BTRC_FEAT_ABSOLUTE_VOLUME) != 0) {
                    status = true;
                }
            }
        }
        Log.v(TAG, "Exit isAbsoluteVolumeSupported");
        return status;
 }



 public boolean isAbsoluteVolumeSupported(int index) {
        boolean status = false;
        Log.v(TAG, "Enter isAbsoluteVolumeSupported with index " + index);
        if (deviceFeatures[index].mCurrentDevice != null) {
            if (deviceFeatures[index].isAbsoluteVolumeSupportingDevice) {
                Log.v(TAG, "isAbsoluteVolumeSupported: yes, for dev: " + index);
                status = true;
            }
        }
        Log.v(TAG, "Exit isAbsoluteVolumeSupported with index " + index);
        return status;
    }

    /**
     * We get this call from AudioService. This will send a message to our handler object,
     * requesting our handler to call setVolumeNative()
     */
    public void adjustVolume(int direction) {
        Log.d(TAG, "MSG_ADJUST_VOLUME");
        Message msg = mHandler.obtainMessage(MSG_ADJUST_VOLUME, direction, 0);
        mHandler.sendMessage(msg);
    }

    public void setAbsoluteVolume(int volume) {
        Log.v(TAG, "Enter setAbsoluteVolume");
        if (volume == mLocalVolume) {
            if (DEBUG) Log.v(TAG, "setAbsoluteVolume is setting same index, ignore " + volume);
            return;
        }
        Log.d(TAG, "pts_test = " + pts_test + " volume = " + volume +
                " local volume = " + mLocalVolume);
        if (pts_test) {
            AvrcpControllerService avrcpCtrlService =
                    AvrcpControllerService.getAvrcpControllerService();
            if (avrcpCtrlService != null) {
                Log.d(TAG, "avrcpCtrlService not null");
                for (int i = 0; i < maxAvrcpConnections; i++) {
                    if (deviceFeatures[i].mCurrentDevice != null) {
                        Log.d(TAG, "SendPassThruPlay command sent for = "
                                + deviceFeatures[i].mCurrentDevice);
                        if (volume > mLocalVolume) {
                            Log.d(TAG, "Vol Passthrough Up");
                            avrcpCtrlService.sendPassThroughCommandNative(
                                Utils.getByteAddress(deviceFeatures[i].mCurrentDevice),
                                AVRC_ID_VOL_UP, AvrcpConstants_ext.KEY_STATE_PRESS);
                            avrcpCtrlService.sendPassThroughCommandNative(
                                Utils.getByteAddress(deviceFeatures[i].mCurrentDevice),
                                AVRC_ID_VOL_UP, AvrcpConstants_ext.KEY_STATE_RELEASE);
                        } else if (volume < mLocalVolume) {
                           Log.d(TAG, "Vol Passthrough Down");
                           avrcpCtrlService.sendPassThroughCommandNative(
                                Utils.getByteAddress(deviceFeatures[i].mCurrentDevice),
                                AVRC_ID_VOL_DOWN, AvrcpConstants_ext.KEY_STATE_PRESS);
                            avrcpCtrlService.sendPassThroughCommandNative(
                                Utils.getByteAddress(deviceFeatures[i].mCurrentDevice),
                                AVRC_ID_VOL_DOWN, AvrcpConstants_ext.KEY_STATE_RELEASE);
                        }
                        mLocalVolume = volume;
                    }
                }
            } else {
                Log.d(TAG, "passthru command not sent, connection unavailable");
            }
        } else {
            mHandler.removeMessages(MSG_ADJUST_VOLUME);
            Message msg = mHandler.obtainMessage(MSG_SET_ABSOLUTE_VOLUME, volume, 0);
            mHandler.sendMessage(msg);
            Log.v(TAG, "Exit setAbsoluteVolume");
        }
    }

    /* Called in the native layer as a btrc_callback to return the volume set on the carkit in the
     * case when the volume is change locally on the carkit. This notification is not called when
     * the volume is changed from the phone.
     *
     * This method will send a message to our handler to change the local stored volume and notify
     * AudioService to update the UI
     */
    private void volumeChangeRequestFromNative(byte[] address, int volume, int ctype) {
        Message msg = mHandler.obtainMessage(MSG_NATIVE_REQ_VOLUME_CHANGE, volume, ctype);
        Bundle data = new Bundle();
        Log.v(TAG, "Enter volumeChangeRequestFromNative");
        data.putByteArray("BdAddress" , address);
        msg.setData(data);
        mHandler.sendMessage(msg);
        Log.v(TAG, "Exit volumeChangeRequestFromNative");
    }

    private void getFolderItemsRequestFromNative(
        byte[] address, byte scope, long startItem, long endItem, byte numAttr, int[] attrIds) {
        if (DEBUG) Log.v(TAG, "getFolderItemsRequestFromNative: scope=" + scope + ", numAttr=" + numAttr);
        AvrcpCmd_ext avrcpCmdobj = new AvrcpCmd_ext();
        Log.v(TAG, "Enter getFolderItemsRequestFromNative");
        AvrcpCmd_ext.FolderItemsCmd folderObj = avrcpCmdobj.new FolderItemsCmd(address, scope,
                startItem, endItem, numAttr, attrIds);
        Message msg = mHandler.obtainMessage(MSG_NATIVE_REQ_GET_FOLDER_ITEMS, 0, 0);
        msg.obj = folderObj;
        mHandler.sendMessage(msg);
        Log.v(TAG, "Exit getFolderItemsRequestFromNative");
    }

    private void setAddressedPlayerRequestFromNative(byte[] address, int playerId) {
        if (DEBUG) Log.v(TAG, "setAddrPlayerRequestFromNative: playerId=" + playerId);
        Message msg = mHandler.obtainMessage(MSG_NATIVE_REQ_SET_ADDR_PLAYER, playerId, 0);
        msg.obj = address;
        mHandler.sendMessage(msg);
        Log.v(TAG, "Exit setAddressedPlayerRequestFromNative");
    }

    private void setBrowsedPlayerRequestFromNative(byte[] address, int playerId) {
        if (DEBUG) Log.v(TAG, "setBrPlayerRequestFromNative: playerId=" + playerId);
        Message msg = mHandler.obtainMessage(MSG_NATIVE_REQ_SET_BR_PLAYER, playerId, 0);
        msg.obj = address;
        mHandler.sendMessage(msg);
        Log.v(TAG, "Exit setBrowsedPlayerRequestFromNative");
    }

    private void changePathRequestFromNative(byte[] address, byte direction, byte[] folderUid) {
        if (DEBUG) Log.v(TAG, "changePathRequestFromNative: direction=" + direction);
        Bundle data = new Bundle();
        Message msg = mHandler.obtainMessage(MSG_NATIVE_REQ_CHANGE_PATH);
        data.putByteArray("BdAddress" , address);
        data.putByteArray("folderUid" , folderUid);
        data.putByte("direction" , direction);
        msg.setData(data);
        mHandler.sendMessage(msg);
        Log.v(TAG, "Exit changePathRequestFromNative");
    }

    private void getItemAttrRequestFromNative(byte[] address, byte scope, byte[] itemUid, int uidCounter,
            byte numAttr, int[] attrs) {
        AvrcpCmd_ext avrcpCmdobj = new AvrcpCmd_ext();
        Log.v(TAG, "Enter getItemAttrRequestFromNative");
        AvrcpCmd_ext.ItemAttrCmd itemAttr = avrcpCmdobj.new ItemAttrCmd(address, scope,
                itemUid, uidCounter, numAttr, attrs);
        Message msg = mHandler.obtainMessage(MSG_NATIVE_REQ_GET_ITEM_ATTR);
        msg.obj = itemAttr;
        mHandler.sendMessage(msg);
        Log.v(TAG, "Exit getItemAttrRequestFromNative");
    }

    private void searchRequestFromNative(byte[] address, int charsetId, byte[] searchStr) {
        /* Search is not supported */
        Log.w(TAG, "searchRequestFromNative: search is not supported");
        searchRspNative(address, AvrcpConstants_ext.RSP_SRCH_NOT_SPRTD, 0, 0);
    }

    private void playItemRequestFromNative(byte[] address, byte scope, int uidCounter, byte[] uid) {
        if (DEBUG) Log.v(TAG, "playItemRequestFromNative: scope=" + scope);
        Bundle data = new Bundle();
        Message msg = mHandler.obtainMessage(MSG_NATIVE_REQ_PLAY_ITEM);
        data.putByteArray("BdAddress" , address);
        data.putByteArray("uid" , uid);
        data.putInt("uidCounter" , uidCounter);
        data.putByte("scope" , scope);
        msg.setData(data);
        mHandler.sendMessage(msg);
        Log.v(TAG, "Exit playItemRequestFromNative");
    }

    private void addToPlayListRequestFromNative(byte[] address, byte scope, byte[] uid, int uidCounter) {
        /* add to NowPlaying not supported */
        Log.w(TAG, "addToPlayListRequestFromNative: not supported! scope=" + scope);
        addToNowPlayingRspNative(address, AvrcpConstants_ext.RSP_INTERNAL_ERR);
    }

    private void getTotalNumOfItemsRequestFromNative(byte[] address, byte scope) {
        if (DEBUG) Log.v(TAG, "getTotalNumOfItemsRequestFromNative: scope=" + scope);
        Bundle data = new Bundle();
        Message msg = mHandler.obtainMessage(MSG_NATIVE_REQ_GET_TOTAL_NUM_OF_ITEMS);
        msg.arg1 = scope;
        msg.obj = address;
        mHandler.sendMessage(msg);
        Log.v(TAG, "Exit getTotalNumOfItemsRequestFromNative");
    }

    private void notifyVolumeChanged(int volume, boolean isShowUI) {
        if (isShowUI) {
            mAudioManager.setStreamVolume(AudioManager.STREAM_MUSIC, volume,
                    AudioManager.FLAG_SHOW_UI | AudioManager.FLAG_BLUETOOTH_ABS_VOLUME);
        } else {
            mAudioManager.setStreamVolume(AudioManager.STREAM_MUSIC, volume,
                    AudioManager.FLAG_BLUETOOTH_ABS_VOLUME);
        }
    }

    private int convertToAudioStreamVolume(int volume) {
        // Rescale volume to match AudioSystem's volume
        return (int) Math.round((double) volume*mAudioStreamMax/AVRCP_MAX_VOL);
    }

    private int convertToAvrcpVolume(int volume) {
        return (int) Math.ceil((double) volume*AVRCP_MAX_VOL/mAudioStreamMax);
    }

    //PDU ID 0x11
    private void onListPlayerAttributeRequest(byte[] address) {
        if (DEBUG)
            Log.v(TAG, "onListPlayerAttributeRequest");
        int deviceIndex =
                getIndexForDevice(mAdapter.getRemoteDevice(
                        Utils.getAddressStringFromByte(address)));
        if (deviceIndex == INVALID_DEVICE_INDEX) {
            Log.e(TAG,"invalid index for device");
            return;
        }
        CreateMusicSettingsAppCmdLookupOrUpdate(AvrcpConstants_ext.GET_ATTRIBUTE_IDS,
                deviceIndex, true);
        mAvrcpPlayerAppSettings.onListPlayerAttributeRequest(address);
        SendPlayerSettingMsg(AvrcpConstants_ext.GET_ATTRIBUTE_IDS, address);
   }

    //PDU ID 0x12
    private void onListPlayerAttributeValues (byte attr, byte[] address) {
        if (DEBUG)Log.v(TAG, "onListPlayerAttributeValues");
        int deviceIndex =
                getIndexForDevice(mAdapter.getRemoteDevice(
                Utils.getAddressStringFromByte(address)));
        if (deviceIndex == INVALID_DEVICE_INDEX) {
            Log.e(TAG,"invalid index for device");
            return;
        }
        CreateMusicSettingsAppCmdLookupOrUpdate(AvrcpConstants_ext.GET_VALUE_IDS, deviceIndex, true);
        mAvrcpPlayerAppSettings.onListPlayerAttributeValues(attr, address);
        SendPlayerSettingMsg(AvrcpConstants_ext.GET_VALUE_IDS, address);
    }

    //PDU ID 0x13
    private void onGetPlayerAttributeValues (byte attr ,int[] arr ,
            byte[] address)
    {
        if (DEBUG)
            Log.v(TAG, "onGetPlayerAttributeValues: num of attrib " + attr );
        int deviceIndex =
                getIndexForDevice(mAdapter.getRemoteDevice(
                Utils.getAddressStringFromByte(address)));
        if (deviceIndex == INVALID_DEVICE_INDEX) {
            Log.e(TAG,"invalid index for device");
            return;
        }
        CreateMusicSettingsAppCmdLookupOrUpdate(AvrcpConstants_ext.GET_ATTRIBUTE_VALUES,
                deviceIndex, true);
        mAvrcpPlayerAppSettings.onGetPlayerAttributeValues(attr, arr, address);
        SendPlayerSettingMsg(AvrcpConstants_ext.GET_ATTRIBUTE_VALUES, address);
    }

    //PDU 0x14
    private void setPlayerAppSetting( byte num, byte [] attr_id, byte [] attr_val,
            byte[] address)
    {
        if (DEBUG)
            Log.v(TAG, "setPlayerAppSetting: number of attributes" + num );
        int deviceIndex =
                getIndexForDevice(mAdapter.getRemoteDevice(
                Utils.getAddressStringFromByte(address)));
        if (deviceIndex == INVALID_DEVICE_INDEX) {
            Log.e(TAG,"invalid index for device");
            return;
        }
        if (!deviceFeatures[deviceIndex].isActiveDevice) {
            SendSetPlayerAppRspNative(AvrcpConstants_ext.RSP_INTERNAL_ERR, address);
            Log.e(TAG,"Set Command from inactive device reject it");
            return;
        }

        CreateMusicSettingsAppCmdLookupOrUpdate(AvrcpConstants_ext.SET_ATTRIBUTE_VALUES,
                deviceIndex, true);
        mAvrcpPlayerAppSettings.setPlayerAppSetting(num, attr_id, attr_val, address);
        SendPlayerSettingMsg(AvrcpConstants_ext.SET_ATTRIBUTE_VALUES, address);
    }

    //PDU 0x15
    private void getplayerattribute_text(byte attr , byte [] attrIds,
            byte[] address)
    {
        if(DEBUG) Log.d(TAG, "getplayerattribute_text " + attr +" attrIDsNum "
                                                        + attrIds.length);
        int deviceIndex =
                getIndexForDevice(mAdapter.getRemoteDevice(
                Utils.getAddressStringFromByte(address)));
        if (deviceIndex == INVALID_DEVICE_INDEX) {
            Log.e(TAG,"invalid index for device");
            return;
        }
        CreateMusicSettingsAppCmdLookupOrUpdate(AvrcpConstants_ext.GET_ATTRIBUTE_TEXT,
                deviceIndex, true);
        mAvrcpPlayerAppSettings.getplayerattribute_text(attr, attrIds, address);
        SendPlayerSettingMsg(AvrcpConstants_ext.GET_ATTRIBUTE_TEXT, address);
    }

    //PDU 0x16
    private void getplayervalue_text(byte attr_id , byte num_value , byte [] value,
            byte[] address)
    {
        int deviceIndex =
                getIndexForDevice(mAdapter.getRemoteDevice(
                Utils.getAddressStringFromByte(address)));
        if (deviceIndex == INVALID_DEVICE_INDEX) {
            Log.e(TAG,"invalid index for device");
            return;
        }
        CreateMusicSettingsAppCmdLookupOrUpdate(AvrcpConstants_ext.GET_VALUE_TEXT, deviceIndex, true);
        mAvrcpPlayerAppSettings.getplayervalue_text(attr_id, num_value, value, address);
        SendPlayerSettingMsg(AvrcpConstants_ext.GET_VALUE_TEXT, address);
    }

    private void SendPlayerSettingMsg(Integer cmd, byte[] address) {
        long delay_interval = mAvrcpPlayerAppSettings.getPlayerAppSettingsCmdDelay();
        Message msg = mHandler.obtainMessage();
        msg.what = MESSAGE_PLAYERSETTINGS_TIMEOUT;
        msg.arg1 = cmd;
        msg.arg2 = 0;
        msg.obj = Utils.getAddressStringFromByte(address);
        mHandler.sendMessageDelayed(msg, delay_interval);
    }

    private void CreateMusicSettingsAppCmdLookupOrUpdate(Integer cmd,
            int deviceIndex, boolean entry_new) {
        if (deviceIndex == INVALID_DEVICE_INDEX) {
           Log.e(TAG,"invalid index for device");
           return;
        }
        Log.v(TAG,"Cmd = " + cmd + "on index = " + deviceIndex + "new entry" + entry_new);

        if (entry_new) {
            if (deviceFeatures[deviceIndex].mMusicAppCmdResponsePending.
                    containsKey(cmd)) {
                int cmdCount =
                        deviceFeatures[deviceIndex].mMusicAppCmdResponsePending.get(cmd);
                Log.v(TAG,"cmdCount = " + cmdCount + "for command type = " + cmd);
                deviceFeatures[deviceIndex].mMusicAppCmdResponsePending.put
                        (cmd, cmdCount + 1);
            } else {
                deviceFeatures[deviceIndex].mMusicAppCmdResponsePending.put
                        (cmd, 1);
            }
        } else {
            if (deviceFeatures[deviceIndex].mMusicAppCmdResponsePending.
                    containsKey(cmd)) {
                int PendingCmds =
                        deviceFeatures[deviceIndex].mMusicAppCmdResponsePending.get(cmd);
                Log.v(TAG,"PendingCmds = " + PendingCmds + "for resoponse type = " + cmd);
                if (PendingCmds > 1) {
                    deviceFeatures[deviceIndex].mMusicAppCmdResponsePending
                            .put(cmd, PendingCmds - 1);
                } else if (PendingCmds == 1) {
                    deviceFeatures[deviceIndex].mMusicAppCmdResponsePending.remove(cmd);
                } else {
                    Log.e(TAG,"Invalid Player Setting Cmd count entry in lookup");
                }
            }
        }
    }


    private void SetBrowsePackage(String PackageName) {
        String browseService = (PackageName != null)?getBrowseServiceName(PackageName):null;
        Log.w(TAG, "SetBrowsePackage for pkg " + PackageName + "svc" + browseService);
        if (browseService != null && !browseService.isEmpty()) {
            BluetoothDevice browse_active_device = mBrowsingActiveDevice;
            if (browse_active_device != null) {
                is_player_updated_for_browse = true;
                byte[] addr = getByteAddress(browse_active_device);
                if (mAvrcpBrowseManager.getBrowsedMediaPlayer(addr) != null) {
                    Log.w(TAG, "Addr Player update to Browse " + PackageName);
                    mAvrcpBrowseManager.getBrowsedMediaPlayer(addr).
                            setCurrentPackage(PackageName, browseService);
                }
            } else {
                Log.w(TAG, "SetBrowsePackage Active device not set yet cache " + PackageName +
                        "wait for connection");
                mCachedBrowsePlayer = PackageName;
            }
        }
    }

    private void blackListCurrentDevice(int i) {
        String mAddress = null;
        if (deviceFeatures[i].mCurrentDevice == null) {
            Log.v(TAG, "blackListCurrentDevice: Device is null");
            return;
        }
        mAddress  = deviceFeatures[i].mCurrentDevice.getAddress();
        deviceFeatures[i].mFeatures &= ~BTRC_FEAT_ABSOLUTE_VOLUME;
        BATService mBatService = BATService.getBATService();
        if ((mBatService != null) && mBatService.isBATActive()) {
            Log.d(TAG," blackListCurrentDevice BA Active, update absvol support as true  ");
            mAudioManager.avrcpSupportsAbsoluteVolume(mAddress, true);
        }
        else {
            mAudioManager.avrcpSupportsAbsoluteVolume(mAddress, false);
        }

        SharedPreferences pref = mContext.getSharedPreferences(ABSOLUTE_VOLUME_BLACKLIST,
                Context.MODE_PRIVATE);
        SharedPreferences.Editor editor = pref.edit();
        editor.putBoolean(mAddress, true);
        editor.commit();
        SharedPreferences.Editor absVolumeMapEditor = getAbsVolumeMap().edit();
        mDeviceAbsVolMap.put(deviceFeatures[i].mCurrentDevice, false);
        absVolumeMapEditor.putBoolean(mAddress, false);
        // Always use apply() since it is asynchronous, otherwise the call can hang waiting for
        // storage to be written.
        absVolumeMapEditor.apply();
        Log.v(TAG, "Exit blackListCurrentDevice");
    }


    private int modifyRcFeatureFromBlacklist(int feature, String address) {
        SharedPreferences pref = mContext.getSharedPreferences(ABSOLUTE_VOLUME_BLACKLIST,
                Context.MODE_PRIVATE);
        Log.v(TAG, "Enter modifyRcFeatureFromBlacklist");
        if (!pref.contains(address)) {
            return feature;
        }
        if (pref.getBoolean(address, false)) {
            feature &= ~BTRC_FEAT_ABSOLUTE_VOLUME;
        }
        Log.v(TAG, "Exit modifyRcFeatureFromBlacklist");
        return feature;
    }

    public void resetBlackList(String address) {
        SharedPreferences pref = mContext.getSharedPreferences(ABSOLUTE_VOLUME_BLACKLIST,
                Context.MODE_PRIVATE);
        if (!pref.contains(address)) {
            Log.v(TAG, "resetBlackList : device is not added in blacklist");
            return;
        }
        SharedPreferences.Editor editor = pref.edit();
        editor.remove(address);
        editor.apply();
        SharedPreferences.Editor absVolumeMapEditor = getAbsVolumeMap().edit();
        BluetoothDevice device = BluetoothAdapter.getDefaultAdapter().getRemoteDevice(address);
        if (device != null && device.getBondState() == BluetoothDevice.BOND_NONE) {
            Log.v(TAG, "resetBlackList : device state is unbonded");
            return;
        }
        mDeviceAbsVolMap.put(device, true);
        absVolumeMapEditor.putBoolean(address, true);
        // Always use apply() since it is asynchronous, otherwise the call can hang waiting for
        // storage to be written.
        absVolumeMapEditor.apply();
        //updating audio for absolute volume support
        mAudioManager.avrcpSupportsAbsoluteVolume(address, true);
    }

    /**
     * This is called from A2dpStateMachine to set A2dp audio state.
     */
    public void setA2dpAudioState(int state, BluetoothDevice device) {
        Message msg = mHandler.obtainMessage(MSG_SET_A2DP_AUDIO_STATE, state, 0, device);
        mHandler.sendMessage(msg);
    }
    private boolean isTwsPlusPair(BluetoothDevice target_dev, BluetoothDevice curr_dev) {
        if (target_dev.isTwsPlusDevice() && curr_dev.isTwsPlusDevice() &&
            curr_dev.getTwsPlusPeerAddress().equals(target_dev.getAddress())) {
            Log.i(TAG,"isTwsPlusPair = Yes");
            return true;
        }
        return false;
    }
    public void setAvrcpConnectedDevice(BluetoothDevice device) {
        boolean NeedCheckMusicActive = true;
        Log.i(TAG,"setAvrcpConnectedDevice, Device added is " + device);
        BluetoothDevice active_device = null;
        for (int i = 0; i < maxAvrcpConnections; i++) {
            if (deviceFeatures[i].mCurrentDevice != null) {
                if(device != null && Objects.equals(deviceFeatures[i].mCurrentDevice, device)) {
                    Log.v(TAG,"device is already added in connected list, ignore now");
                    return;
                }
                if(deviceFeatures[i].isActiveDevice == true) {
                   Log.i(TAG,"Device " + deviceFeatures[i].mCurrentDevice.getAddress() + " is active");
                   active_device = deviceFeatures[i].mCurrentDevice;
                }
            }
        }
        if(avrcp_playstatus_blacklist && isPlayerStateUpdateBlackListed(device.getAddress(),device.getName()))
           NeedCheckMusicActive = false;
        for (int i = 0; i < maxAvrcpConnections; i++ ) {
            if (deviceFeatures[i].mCurrentDevice == null) {
                deviceFeatures[i].mCurrentDevice = device;
                if ((device.isTwsPlusDevice()) &&
                     !twsShoEnabled) {
                    Log.v(TAG,"TWS+ device connected, set it to active");
                    deviceFeatures[i].isActiveDevice = true;
                    updateAbsVolume = false;
                }
                deviceFeatures[i].mInitialRemoteVolume = -1;
                deviceFeatures[i].mBlackListVolume = -1;

                /*Playstate is explicitly updated here to take care of cases
                        where play state update is missed because of that happening
                        even before Avrcp connects*/
                deviceFeatures[i].mCurrentPlayState = mCurrentPlayerState;
                if (isPlayingState(mCurrentPlayerState)) {
                /* In dual a2dp connection mode, if music is streaming on other device and
                ** avrcp connection was delayed to second device and is not in playing state
                ** check for playing device and update play status accordingly
                */
                    if (!isPlayStateToBeUpdated(i)) {
                        PlaybackState.Builder playState = new PlaybackState.Builder();
                        playState.setState(PlaybackState.STATE_PAUSED,
                                       PlaybackState.PLAYBACK_POSITION_UNKNOWN, 1.0f);
                        deviceFeatures[i].mCurrentPlayState = playState.build();
                    }
                }
                Log.i(TAG,"setAvrcpConnectedDevice, mCurrentPlayerState = " + mCurrentPlayerState +
                          "isMusicActive = " + mAudioManager.isMusicActive() +
                          "isA2dpPlaying = " + mA2dpService.isA2dpPlaying(device));
                if (!isPlayingState(mCurrentPlayerState) &&
                     (mA2dpService.isA2dpPlaying(device)) &&
                      ((NeedCheckMusicActive && mAudioManager.isMusicActive()) ||(!NeedCheckMusicActive))) {
                /*A2DP playstate updated for video playback scenario, where a2dp play status is
                    updated when avrcp connection was not up yet.*/
                    Log.i(TAG,"A2dp playing device found");
                    BluetoothDevice playingDevice = mA2dpService.getActiveDevice();
                    if (playingDevice != null && playingDevice.equals(device)) {
                        PlaybackState.Builder playState = new PlaybackState.Builder();
                        playState.setState(PlaybackState.STATE_PLAYING,
                                       PlaybackState.PLAYBACK_POSITION_UNKNOWN, 1.0f);
                        deviceFeatures[i].mCurrentPlayState = playState.build();
                    }
                } else if (isPlayingState(mCurrentPlayerState) &&
                           !mA2dpService.isA2dpPlaying(device)) {
                       PlaybackState.Builder playState = new PlaybackState.Builder();
                       playState.setState(PlaybackState.STATE_PAUSED,
                                     PlaybackState.PLAYBACK_POSITION_UNKNOWN, 1.0f);
                       deviceFeatures[i].mCurrentPlayState = playState.build();
                }
                Log.i(TAG,"play status updated on Avrcp connection as: " +
                                                    deviceFeatures[i].mCurrentPlayState);
                Log.i(TAG,"device added at " + i);
                Log.i(TAG,"Active device set to true at index =  " + i);
                break;
            }
        }

        for (int i = 0; i < maxAvrcpConnections; i++ ) {
            if (isPlayingState(mCurrentPlayerState)) {
                if (deviceFeatures[i].mCurrentDevice != null &&
                    !isPlayStateToBeUpdated(i) &&
                    deviceFeatures[i].isActiveDevice) {
                    deviceFeatures[i].isActiveDevice = false;
                    Log.i(TAG,"Active device set to false at index =  " + i);
                    if (isPlayingState(deviceFeatures[i].mCurrentPlayState)) {
                        PlaybackState.Builder playState = new PlaybackState.Builder();
                        playState.setState(PlaybackState.STATE_PAUSED,
                                       PlaybackState.PLAYBACK_POSITION_UNKNOWN, 1.0f);
                        updatePlaybackState(playState.build(), deviceFeatures[i].mCurrentDevice);
                    }
                }
            }
            else if (deviceFeatures[i].mCurrentDevice != null && device != null &&
                    !(Objects.equals(deviceFeatures[i].mCurrentDevice, device)) &&
                    deviceFeatures[i].isActiveDevice &&
                    !isTwsPlusPair(deviceFeatures[i].mCurrentDevice, device)) {
                deviceFeatures[i].isActiveDevice = false;
                Log.i(TAG,"Active device set to false at index =  " + i);
            }
        }
        BluetoothDevice mDevice = mA2dpService.getActiveDevice();
        //validating device is connected
        int index = getIndexForDevice(device);
        if (index != INVALID_DEVICE_INDEX && mDevice != null &&
            (Objects.equals(mDevice, deviceFeatures[index].mCurrentDevice) ||
             (mDevice.isTwsPlusDevice() && device.isTwsPlusDevice()))) {
            setActiveDevice(deviceFeatures[index].mCurrentDevice);
            //When A2dp playing on DUT and Remote got connected, send proper playstatus
            if (isPlayingState(mCurrentPlayerState) &&
                mA2dpService.isA2dpPlaying(device)) {
                deviceFeatures[index].mLastStateUpdate = mLastStateUpdate;
                deviceFeatures[index].mCurrentPlayState = mCurrentPlayerState;
                Log.i(TAG,"Send correct playstatus and song position to remote when it gets connected: " +
                                                      deviceFeatures[index].mCurrentPlayState);
            }
        }
        Log.i(TAG,"Exit setAvrcpConnectedDevice");
    }

    /**
     * This is called from A2dpStateMachine to set A2dp Connected device to null on disconnect.
     */
    public void setAvrcpDisconnectedDevice(BluetoothDevice device) {
        Log.i(TAG,"Enter setAvrcpDisconnectedDevice");
        int deviceIndex = INVALID_DEVICE_INDEX;
        for (int i = 0; i < maxAvrcpConnections; i++ ) {
            if (deviceFeatures[i].mCurrentDevice != null && device != null &&
                    Objects.equals(deviceFeatures[i].mCurrentDevice, device)) {
                deviceIndex = i;
                if (deviceFeatures[i].isActiveDevice &&
                      deviceFeatures[i].isAbsoluteVolumeSupportingDevice) {
                    storeVolumeForDevice(device);
                    disconnectedActiveDevice = device;
                }
            }
            if (deviceFeatures[i].mCurrentDevice != null && device != null &&
                    !(Objects.equals(deviceFeatures[i].mCurrentDevice, device))) {
                Log.i(TAG,"setAvrcpDisconnectedDevice : Active device changed to index = " + i);
                if (device.isTwsPlusDevice() &&
                    isTwsPlusPair(device,deviceFeatures[i].mCurrentDevice)) {
                    Log.i(TAG,"TWS+ pair got disconnected,update absVolume");
                    updateAbsVolume = true;
                    Log.i(TAG,"TWS+ pair disconnected, set mTwsPairDisconnected for index " + i);
                    deviceFeatures[i].mTwsPairDisconnected = true;
                }
            }
        }

        if (deviceIndex != INVALID_DEVICE_INDEX) {
            // initiate cleanup for all variables;
            cleanupDeviceFeaturesIndex(deviceIndex);
            Log.i(TAG,"Device removed is " + device);
            Log.i(TAG,"removed at " + deviceIndex);
        }

        BATService mBatService = BATService.getBATService();
        if ((mBatService != null) && mBatService.isBATActive()) {
            Log.d(TAG," setAvrcpDisconnectedDevice BA Active, update absvol support as true  ");
            mAudioManager.avrcpSupportsAbsoluteVolume(device.getAddress(), true);
        }

        if (mBrowsingActiveDevice != null && Objects.equals(mBrowsingActiveDevice, device)) {
            Log.w(TAG,"setAvrcpDisconnect: Browse active device disconned reset it");
            mBrowsingActiveDevice = null;
        }

        if ((mCurrentBrowsingDevice != null) &&
            (mCurrentBrowsingDevice.equals(device))) {
            Log.v(TAG,"BT device is matched with browsing device:");
            BrowsedMediaPlayer_ext player =
                    mAvrcpBrowseManager.getBrowsedMediaPlayer(getByteAddress(device));
            if (player != null)
               player.disconnect();
            mAvrcpBrowseManager.clearBrowsedMediaPlayer(getByteAddress(device));
            mCurrentBrowsingDevice = null;
            changePathDepth = 0;
            changePathFolderType = 0;
            is_player_updated_for_browse = false;
            Log.w(TAG,"Reset is_player_updated_for_browse on device disconnection");
        }
        Log.v(TAG,"Exit setAvrcpDisconnectedDevice");
    }

    private class AvrcpServiceBootReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (action.equals(Intent.ACTION_USER_UNLOCKED)) {
                if (DEBUG) Log.d(TAG, "User unlocked, initializing player lists");
                /* initializing media player's list */
                buildBrowsablePlayerList();
            }
        }
    }

    private class AvrcpServiceBroadcastReceiver extends BroadcastReceiver {
        @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            if (DEBUG) Log.d(TAG, "AvrcpServiceBroadcastReceiver-> Action: " + action);

            if (action.equals(Intent.ACTION_PACKAGE_REMOVED)
                    || action.equals(Intent.ACTION_PACKAGE_DATA_CLEARED)) {
                if (!intent.getBooleanExtra(Intent.EXTRA_REPLACING, false)) {
                    // a package is being removed, not replaced
                    String packageName = intent.getData().getSchemeSpecificPart();
                    if (packageName != null) {
                        handlePackageModified(packageName, true);
                    }
                }
            } else if (action.equals(Intent.ACTION_PACKAGE_ADDED)
                    || action.equals(Intent.ACTION_PACKAGE_CHANGED)) {
                String packageName = intent.getData().getSchemeSpecificPart();
                if (DEBUG) Log.d(TAG,"AvrcpServiceBroadcastReceiver-> packageName: "
                        + packageName);
                if (packageName != null) {
                    handlePackageModified(packageName, false);
                }
            }
        }
    }

    private class AvrcpServiceShutDownReceiver extends BroadcastReceiver {
    @Override
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();
            BluetoothDevice device = null;
            if (DEBUG) Log.d(TAG, "AvrcpServiceShutdownReceiver-> Action: " + action);
            if (action.equals(Intent.ACTION_SHUTDOWN)) {
                int storeVolume =  mAudioManager.getStreamVolume(AudioManager.STREAM_MUSIC);
                if (DEBUG) Log.d(TAG, "AvrcpServiceShutdownReceiver store volume " + storeVolume);
                if (mA2dpService != null)
                   device =  mA2dpService.getActiveDevice();
                SharedPreferences.Editor pref = getVolumeMap().edit();
                if (device != null) {
                    pref.putInt(device.getAddress(), storeVolume);
                    pref.apply();
                }
            }
        }
    }

    private void handlePackageModified(String packageName, boolean removed) {
        if (DEBUG) Log.d(TAG, "packageName: " + packageName + " removed: " + removed);

        if (removed) {
            removeMediaPlayerInfo(packageName);
            // old package is removed, updating local browsable player's list
            if (isBrowseSupported(packageName)) {
                removePackageFromBrowseList(packageName);
            }
        } else {
            // new package has been added.
            if (isBrowsableListUpdated(packageName)) {
                // Rebuilding browsable players list
                buildBrowsablePlayerList();
            }
        }
        Log.d(TAG, "Exit handlePackageModified");
    }

    private boolean isBrowsableListUpdated(String newPackageName) {
        // getting the browsable media players list from package manager
        Intent intent = new Intent("android.media.browse.MediaBrowserService");
        List<ResolveInfo> resInfos = mPackageManager.queryIntentServices(intent,
                                         PackageManager.MATCH_ALL);
        for (ResolveInfo resolveInfo : resInfos) {
            if (resolveInfo.serviceInfo.packageName.equals(newPackageName)) {
                if (DEBUG)
                    Log.d(TAG,
                            "isBrowsableListUpdated: package includes MediaBrowserService, true");
                return true;
            }
        }

        // if list has different size
        if (resInfos.size() != mBrowsePlayerInfoList.size()) {
            if (DEBUG) Log.d(TAG, "isBrowsableListUpdated: browsable list size mismatch, true");
            return true;
        }

        Log.d(TAG, "isBrowsableListUpdated: false");
        return false;
    }

    private void removePackageFromBrowseList(String packageName) {
        if (DEBUG) Log.d(TAG, "removePackageFromBrowseList: " + packageName);
        synchronized (this) {
            synchronized (mBrowsePlayerInfoList) {
                int browseInfoID = getBrowseId(packageName);
                if (browseInfoID != -1) {
                    mBrowsePlayerInfoList.remove(browseInfoID);
                    BrowsedMediaPlayer_ext player =
                            mAvrcpBrowseManager.getBrowsedMediaPlayer(dummyaddr);
                    if (player != null)
                        player.updateBrowsablePlayerList(packageName);
                }
            }
        }
        Log.d(TAG, "Exit removePackageFromBrowseList");
    }

    /*
     * utility function to get the browse player index from global browsable
     * list. It may return -1 if specified package name is not in the list.
     */
    private int getBrowseId(String packageName) {
        boolean response = false;
        int browseInfoID = 0;
        synchronized (this) {
            synchronized (mBrowsePlayerInfoList) {
                for (BrowsePlayerInfo_ext info : mBrowsePlayerInfoList) {
                    if (info.packageName.equals(packageName)) {
                        response = true;
                        break;
                    }
                    browseInfoID++;
                }
            }
        }

        if (!response) {
            browseInfoID = -1;
        }

        if (DEBUG) Log.d(TAG, "getBrowseId for packageName: " + packageName +
                " , browseInfoID: " + browseInfoID);
        return browseInfoID;
    }

    private void setAddressedPlayer(byte[] bdaddr, int selectedId) {
        String functionTag = "setAddressedPlayer(" + selectedId + "): ";

        Log.d(TAG, "Enter setAddressedPlayer");
        synchronized (this) {
            synchronized (mMediaPlayerInfoList) {
                if (mMediaPlayerInfoList.isEmpty()) {
                    Log.w(TAG, functionTag + "no players, send no available players");
                    setAddressedPlayerRspNative(bdaddr, AvrcpConstants_ext.RSP_NO_AVBL_PLAY);
                    return;
                }
                if (selectedId == NO_PLAYER_ID) {
                    Log.w(TAG, functionTag + "Respond dummy pass response ");
                    setAddressedPlayerRspNative(bdaddr, AvrcpConstants_ext.RSP_NO_ERROR);
                    return;
                }
                if (!mMediaPlayerInfoList.containsKey(selectedId)) {
                    Log.w(TAG, functionTag + "invalid id, sending response back ");
                    setAddressedPlayerRspNative(bdaddr, AvrcpConstants_ext.RSP_INV_PLAYER);
                    return;
                }

                if (isPlayerAlreadyAddressed(selectedId)) {
                    MediaPlayerInfo_ext info = getAddressedPlayerInfo();
                    Log.i(TAG, functionTag + "player already addressed: " + info);
                    setAddressedPlayerRspNative(bdaddr, AvrcpConstants_ext.RSP_NO_ERROR);
                    return;
                }
                // register new Media Controller Callback and update the current IDs
                if (!updateCurrentController(selectedId, mCurrBrowsePlayerID)) {
                    Log.e(TAG, functionTag + "updateCurrentController failed!");
                    setAddressedPlayerRspNative(bdaddr, AvrcpConstants_ext.RSP_INTERNAL_ERR);
                    return;
                }
                // If we don't have a controller, try to launch the player
                MediaPlayerInfo_ext info = getAddressedPlayerInfo();
                if (info.getMediaController() == null) {
                    Intent launch = mPackageManager.getLaunchIntentForPackage(info.getPackageName());
                    Log.i(TAG, functionTag + "launching player " + launch);
                    mContext.startActivity(launch);
                }
            }
        }
        setAddressedPlayerRspNative(bdaddr, AvrcpConstants_ext.RSP_NO_ERROR);
        Log.d(TAG, "Exit setAddressedPlayer");
    }

    private void setBrowsedPlayer(byte[] bdaddr, int selectedId) {
        int status = AvrcpConstants_ext.RSP_NO_ERROR;

        Log.d(TAG, "Enter setBrowsedPlayer");
        String address = Utils.getAddressStringFromByte(bdaddr);
        BluetoothDevice prevBrowseDevice = mCurrentBrowsingDevice;
        mCurrentBrowsingDevice = mAdapter.getRemoteDevice(address);
        // checking for error cases
        BluetoothDevice device = BluetoothAdapter.getDefaultAdapter().getRemoteDevice(bdaddr);
        if (mBrowsingActiveDevice != null && !Objects.equals(mBrowsingActiveDevice, device)) {
            mCurrentBrowsingDevice = prevBrowseDevice;
            status = AvrcpConstants_ext.RSP_INTERNAL_ERR;
            Log.w(TAG, "setBrowsedPlayer: Cmd from browse inactive device reject it");
        } else if (mMediaPlayerInfoList.isEmpty()) {
            status = AvrcpConstants_ext.RSP_NO_AVBL_PLAY;
            Log.w(TAG, "setBrowsedPlayer: No available players! ");
        } else {
            // Workaround for broken controllers selecting ID 0
            if (selectedId == 0) {
                Log.w(TAG, "setBrowsedPlayer: workaround invalid id 0");
                selectedId = mCurrAddrPlayerID;
            }

            // update current browse player id and start browsing service
            updateNewIds(mCurrAddrPlayerID, selectedId);
            String browsedPackage = getPackageName(selectedId);

            if (!isPackageNameValid(browsedPackage)) {
                Log.w(TAG, " Invalid package for id:" + mCurrBrowsePlayerID);
                status = AvrcpConstants_ext.RSP_INV_PLAYER;
            } else if (!playerBrowseWhiteListDB.contains(browsedPackage)) {
                Log.w(TAG, "Browse unsupported for id:" + mCurrBrowsePlayerID
                        + ", packagename : " + browsedPackage);
                status = AvrcpConstants_ext.RSP_PLAY_NOT_BROW;
            } else if (!startBrowseService(bdaddr, browsedPackage)) {
                Log.e(TAG, "service cannot be started for browse player id:" + mCurrBrowsePlayerID
                        + ", packagename : " + browsedPackage);
                status = AvrcpConstants_ext.RSP_INTERNAL_ERR;
            }
        }

        if (status != AvrcpConstants_ext.RSP_NO_ERROR) {
            setBrowsedPlayerRspNative(bdaddr, status, (byte) 0x00, 0, null);
        }

        if (DEBUG) Log.d(TAG, "setBrowsedPlayer for selectedId: " + selectedId +
                " , status: " + status);
        Log.d(TAG, "Exit setBrowsedPlayer");
    }

    private MediaSessionManager.OnActiveSessionsChangedListener mActiveSessionListener =
            new MediaSessionManager.OnActiveSessionsChangedListener() {

                @Override
                public void onActiveSessionsChanged(
                        List<android.media.session.MediaController> newControllers) {
                    if (newControllers.size() > 0) {
                        HeadsetService mService = HeadsetService.getHeadsetService();
                        if (mService != null && mService.isScoOrCallActive()) {
                            Log.d(TAG, "Ignoring session changed update because of MT call in progress");
                            return;
                        }
                    }
                    Set<String> updatedPackages = new HashSet<String>();
                    // Update the current players
                    synchronized (Avrcp_ext.this) {
                        for (android.media.session.MediaController controller : newControllers) {
                            String packageName = controller.getPackageName();
                            if (DEBUG) Log.v(TAG, "ActiveSession: " + MediaControllerFactory.wrap(controller));
                        // Only use the first (highest priority) controller from each package
                            if (updatedPackages.contains(packageName)) continue;
                            addMediaPlayerController(controller);
                            updatedPackages.add(packageName);
                        }
                    }

                    if (newControllers.size() > 0 && getAddressedPlayerInfo() == null) {
                        if (DEBUG)
                            Log.v(TAG, "No addressed player but active sessions, taking first.");
                        Log.w(TAG,"Trigger setAddressedMediaSessionPkg frm onActiveSessionChanged");
                        setAddressedMediaSessionPackage(newControllers.get(0).getPackageName());
                    }
                    updateCurrentMediaState(null);
                }
            };

    private void setAddressedMediaSessionPackage(@Nullable String packageName) {
        if (packageName == null) {
            // Should only happen when there's no media players, reset to no available player.
            updateCurrentController(0, mCurrBrowsePlayerID);
            return;
        }
        if (isAppBlackListedForMediaSessionUpdate(packageName)) {
            Log.d(TAG, "Ignore addressed media session change to " + packageName);
            return;
        }
        // No change.
        if (getPackageName(mCurrAddrPlayerID).equals(packageName)) return;
        Log.w(TAG, "Changing addressed media session to " + packageName);
        // If the player doesn't exist, we need to add it.
        if (getMediaPlayerInfo(packageName) == null) {
            addMediaPlayerPackage(packageName);
            updateCurrentMediaState(null);
        }

        if (mCurrentBrowsingDevice == null) {
            Log.w(TAG, "Calling SetBrowsePackage as part of player switch for " + packageName);
            SetBrowsePackage(packageName);
        }

        synchronized (this) {
            synchronized (mMediaPlayerInfoList) {
                for (Map.Entry<Integer, MediaPlayerInfo_ext> entry : mMediaPlayerInfoList.entrySet()) {
                    if (entry.getValue().getPackageName().equals(packageName)) {
                        int newAddrID = entry.getKey();
                        if (DEBUG) Log.v(TAG, "Set addressed #" + newAddrID + " " + entry.getValue());
                        updateCurrentController(newAddrID, mCurrBrowsePlayerID);
                        updateCurrentMediaState(null);
                        return;
                    }
                }
            }
        }
        // We shouldn't ever get here.
        Log.e(TAG, "Player info for " + packageName + " doesn't exist!");
    }

    private void setActiveMediaSession(MediaSession.Token token) {
        android.media.session.MediaController activeController =
                new android.media.session.MediaController(mContext, token);
        if (isAppBlackListedForMediaSessionUpdate(activeController.getPackageName())) {
            Log.d(TAG, "Ignore active media session change to " + activeController.getPackageName());
            return;
        }

        if(mHandler.hasMessages(MESSAGE_SET_MEDIA_SESSION))
            mHandler.removeMessages(MESSAGE_SET_MEDIA_SESSION);

        if (DEBUG) Log.v(TAG, "Set active media session " + activeController.getPackageName());
        HeadsetService mService = HeadsetService.getHeadsetService();
        if (mService != null && mService.isScoOrCallActive()) {
            Log.w(TAG,"setActiveMediaSession: HF is in non CS call, delaying registration");
            Message msg = mHandler.obtainMessage(MESSAGE_SET_MEDIA_SESSION, activeController);
            mHandler.sendMessageDelayed(msg, SET_MEDIA_SESSION_DELAY);
            return;
        }
        synchronized (Avrcp_ext.this) {
            addMediaPlayerController(activeController);
            setAddressedMediaSessionPackage(activeController.getPackageName());
            mCachedBrowsePlayer = activeController.getPackageName();
            Log.w(TAG,"Trigger setAddressedMediaSessionPackage from setActiveMediaSession" +
                    mCachedBrowsePlayer);
        }
    }

    private void setActiveMediaSession(android.media.session.MediaController mController) {
        HeadsetService mService = HeadsetService.getHeadsetService();
        if (isAppBlackListedForMediaSessionUpdate(mController.getPackageName())) {
            if (mService != null && mService.isScoOrCallActive()) {
                Log.w(TAG, "Ignore media session during call");
                return;
            }
        }

        addMediaPlayerController(mController);
        setAddressedMediaSessionPackage(mController.getPackageName());
        mCachedBrowsePlayer = mController.getPackageName();
        Log.w(TAG,"Trigger setAddressedMediaSessionPkg from setActiveMediaSession" +
                mCachedBrowsePlayer);
    }

    private boolean startBrowseService(byte[] bdaddr, String packageName) {
        boolean status = true;

        /* creating new instance for Browse Media Player */
        String browseService = getBrowseServiceName(packageName);
        if (!browseService.isEmpty()) {
            mAvrcpBrowseManager.getBrowsedMediaPlayer(bdaddr).setBrowsed(
                    packageName, browseService);
        } else {
            Log.w(TAG, "No Browser service available for " + packageName);
            status = false;
        }

        if (DEBUG) Log.d(TAG, "startBrowseService for packageName: " + packageName +
                ", status = " + status);
        return status;
    }

    private String getBrowseServiceName(String packageName) {
        String browseServiceName = "";

        // getting the browse service name from browse player info
        synchronized (this) {
            synchronized (mBrowsePlayerInfoList) {
                int browseInfoID = getBrowseId(packageName);
                if (browseInfoID != -1) {
                    browseServiceName = mBrowsePlayerInfoList.get(browseInfoID).serviceClass;
                }
            }
        }

        if (DEBUG) Log.d(TAG, "getBrowseServiceName for packageName: " + packageName +
                ", browseServiceName = " + browseServiceName);
        return browseServiceName;
    }

    void buildBrowsablePlayerList() {
        synchronized (this) {
            synchronized (mBrowsePlayerInfoList) {
                mBrowsePlayerInfoList.clear();
                Log.d(TAG, "buildBrowsablePlayerList ");
                Intent intent = new Intent(android.service.media.MediaBrowserService.SERVICE_INTERFACE);
                List<ResolveInfo> playerList =
                        mPackageManager.queryIntentServices(intent, PackageManager.MATCH_ALL);

                for (ResolveInfo info : playerList) {
                    String serviceName = info.serviceInfo.name;
                    String packageName = info.serviceInfo.packageName;
                    Log.d(TAG, "Fetch the displayName of package - start");
                    String displayName = getAppLabel(packageName);
                    if (displayName == null) displayName = new String();
                    Log.d(TAG, "Fetch the displayName of package -" + displayName + " end");
                    Log.d(TAG, "svc " + serviceName + " pkg " + packageName);
                    BrowsePlayerInfo_ext currentPlayer =
                            new BrowsePlayerInfo_ext(packageName, displayName, serviceName);
                    mBrowsePlayerInfoList.add(currentPlayer);
                    MediaPlayerInfo_ext playerInfo = getMediaPlayerInfo(packageName);
                    MediaController controller =
                            (playerInfo == null) ? null : playerInfo.getMediaController();
                    // Refresh the media player entry so it notices we can browse
                    if (controller != null) {
                        addMediaPlayerController(controller.getWrappedInstance());
                    } else {
                        addMediaPlayerPackage(packageName);
                    }
                    Log.d(TAG, "Added MediaPlayerController/MediaPlayerPackage");
                }
                updateCurrentMediaState(null);
            }
        }
    }

    /* Initializes list of media players identified from session manager active sessions */
    private void initMediaPlayersList() {
        synchronized (this) {
            synchronized (mMediaPlayerInfoList) {
                // Clearing old browsable player's list
                mMediaPlayerInfoList.clear();

                if (mMediaSessionManager == null) {
                    if (DEBUG) Log.w(TAG, "initMediaPlayersList: no media session manager!");
                    return;
                }

                List<android.media.session.MediaController> controllers =
                        mMediaSessionManager.getActiveSessions(null);
                if (DEBUG)
                    Log.v(TAG, "initMediaPlayerInfoList: " + controllers.size() + " controllers");
                /* Initializing all media players */
                for (android.media.session.MediaController controller : controllers) {
                     addMediaPlayerController(controller);
                }

                updateCurrentMediaState(null);

                if (mMediaPlayerInfoList.size() > 0) {
                    // Set the first one as the Addressed Player
                    updateCurrentController(mMediaPlayerInfoList.firstKey(), -1);
                }
            }
        }
    }

    private List<android.media.session.MediaController> getMediaControllers() {
        List<android.media.session.MediaController> controllers =
                new ArrayList<android.media.session.MediaController>();
        synchronized (this) {
            synchronized (mMediaPlayerInfoList) {
                for (MediaPlayerInfo_ext info : mMediaPlayerInfoList.values()) {
                     MediaController controller = info.getMediaController();
                    if (controller != null) {
                        controllers.add(controller.getWrappedInstance());
                    }
                }
            }
        }
        return controllers;
    }

    /** Add (or update) a player to the media player list without a controller */
    private boolean addMediaPlayerPackage(String packageName) {
        MediaPlayerInfo_ext info = new MediaPlayerInfo_ext(null, AvrcpConstants_ext.PLAYER_TYPE_AUDIO,
                AvrcpConstants_ext.PLAYER_SUBTYPE_NONE, PLAYSTATUS_STOPPED,
                getFeatureBitMask(packageName), packageName, getAppLabel(packageName));
        return addMediaPlayerInfo(info);
    }

    /** Add (or update) a player to the media player list given an active controller */
    private boolean addMediaPlayerController(android.media.session.MediaController controller) {
        String packageName = controller.getPackageName();
        MediaPlayerInfo_ext info = new MediaPlayerInfo_ext(MediaControllerFactory.wrap(controller), /*MediaController not present*/
                AvrcpConstants_ext.PLAYER_TYPE_AUDIO, AvrcpConstants_ext.PLAYER_SUBTYPE_NONE,
                getBluetoothPlayState(controller.getPlaybackState()),
                getFeatureBitMask(packageName), controller.getPackageName(),
                getAppLabel(packageName));
        return addMediaPlayerInfo(info);
    }

    /** Add or update a player to the media player list given the MediaPlayerInfo object.
     *  @return true if an item was updated, false if it was added instead
     */
    private boolean addMediaPlayerInfo(MediaPlayerInfo_ext info) {
        int updateId = -1;
        boolean updated = false;
        boolean currentRemoved = false;
        if (isAppBlackListedForMediaSessionUpdate(info.getPackageName())) {
            Log.d(TAG, "Skip adding to the media player info list " + info.getPackageName());
            return updated;
        }
        synchronized (this) {
            synchronized (mMediaPlayerInfoList) {
                for (Map.Entry<Integer, MediaPlayerInfo_ext> entry : mMediaPlayerInfoList.entrySet()) {
                    MediaPlayerInfo_ext current = entry.getValue();
                    int id = entry.getKey();
                    if (info.getPackageName().equals(current.getPackageName())) {
                        if (!current.equalView(info)) {
                            // If we would present a different player, make it a new player
                            // so that controllers know whether a player is browsable or not.
                            mMediaPlayerInfoList.remove(id);
                            currentRemoved = (mCurrAddrPlayerID == id);
                            break;
                        }
                        updateId = id;
                        updated = true;
                        break;
                    }
                }
                if (updateId == -1) {
                    // New player
                    mLastUsedPlayerID++;
                    updateId = mLastUsedPlayerID;
                    mAvailablePlayerViewChanged = true;
                }
                mMediaPlayerInfoList.put(updateId, info);
            }
        }
        if (DEBUG) Log.d(TAG, (updated ? "update #" : "add #") + updateId + ":" + info.toString());
        if (currentRemoved || updateId == mCurrAddrPlayerID) {
            updateCurrentController(updateId, mCurrBrowsePlayerID);
        }
        return updated;
    }

    /** Remove all players related to |packageName| from the media player info list */
    private MediaPlayerInfo_ext removeMediaPlayerInfo(String packageName) {
        synchronized (this) {
            synchronized (mMediaPlayerInfoList) {
                int removeKey = -1;
                for (Map.Entry<Integer, MediaPlayerInfo_ext> entry : mMediaPlayerInfoList.entrySet()) {
                    if (entry.getValue().getPackageName().equals(packageName)) {
                        removeKey = entry.getKey();
                        break;
                    }
                }
                if (removeKey != -1) {
                    if (DEBUG)
                        Log.d(TAG, "remove #" + removeKey + ":" + mMediaPlayerInfoList.get(removeKey));
                    mAvailablePlayerViewChanged = true;
                    return mMediaPlayerInfoList.remove(removeKey);
                }

                return null;
            }
        }
    }

    /** Remove the controller referenced by |controller| from any player in the list */
    private void removeMediaController(@Nullable android.media.session.MediaController controller) {
        if (controller == null) return;
        synchronized (this) {
            synchronized (mMediaPlayerInfoList) {
                for (Map.Entry<Integer, MediaPlayerInfo_ext> entry : mMediaPlayerInfoList.entrySet()) {
                    MediaPlayerInfo_ext info = entry.getValue();
                    MediaController c = info.getMediaController();
                    if (c != null && c.equals(controller)) {
                        info.setMediaController(null);
                        if (entry.getKey() == mCurrAddrPlayerID) {
                            updateCurrentController(mCurrAddrPlayerID, mCurrBrowsePlayerID);
                        }
                    }
                }
            }
        }
    }

    /*
     * utility function to get the playback state of any media player through
     * media controller APIs.
     */
    private byte getBluetoothPlayState(PlaybackState pbState) {
        if (pbState == null) {
            Log.w(TAG, "playState object null, sending STOPPED");
            return PLAYSTATUS_STOPPED;
        }

        switch (pbState.getState()) {
            case PlaybackState.STATE_PLAYING:
                return PLAYSTATUS_PLAYING;

            case PlaybackState.STATE_BUFFERING:
            case PlaybackState.STATE_STOPPED:
            case PlaybackState.STATE_NONE:
            case PlaybackState.STATE_CONNECTING:
                return PLAYSTATUS_STOPPED;

            case PlaybackState.STATE_PAUSED:
                return PLAYSTATUS_PAUSED;

            case PlaybackState.STATE_FAST_FORWARDING:
            case PlaybackState.STATE_SKIPPING_TO_NEXT:
            case PlaybackState.STATE_SKIPPING_TO_QUEUE_ITEM:
                return PLAYSTATUS_FWD_SEEK;

            case PlaybackState.STATE_REWINDING:
            case PlaybackState.STATE_SKIPPING_TO_PREVIOUS:
                return PLAYSTATUS_REV_SEEK;

            case PlaybackState.STATE_ERROR:
            default:
                return PLAYSTATUS_ERROR;
        }
    }

    /*
     * utility function to get the feature bit mask of any media player through
     * package name
     */
    private short[] getFeatureBitMask(String packageName) {

        ArrayList<Short> featureBitsList = new ArrayList<Short>();

        /* adding default feature bits */
        featureBitsList.add(AvrcpConstants_ext.AVRC_PF_PLAY_BIT_NO);
        featureBitsList.add(AvrcpConstants_ext.AVRC_PF_STOP_BIT_NO);
        featureBitsList.add(AvrcpConstants_ext.AVRC_PF_PAUSE_BIT_NO);
        featureBitsList.add(AvrcpConstants_ext.AVRC_PF_REWIND_BIT_NO);
        featureBitsList.add(AvrcpConstants_ext.AVRC_PF_FAST_FWD_BIT_NO);
        featureBitsList.add(AvrcpConstants_ext.AVRC_PF_FORWARD_BIT_NO);
        featureBitsList.add(AvrcpConstants_ext.AVRC_PF_BACKWARD_BIT_NO);
        featureBitsList.add(AvrcpConstants_ext.AVRC_PF_ADV_CTRL_BIT_NO);

        /* Add/Modify browse player supported features. */
        if (isBrowseSupported(packageName)) {
            featureBitsList.add(AvrcpConstants_ext.AVRC_PF_BROWSE_BIT_NO);
            featureBitsList.add(AvrcpConstants_ext.AVRC_PF_UID_UNIQUE_BIT_NO);
            featureBitsList.add(AvrcpConstants_ext.AVRC_PF_NOW_PLAY_BIT_NO);
            featureBitsList.add(AvrcpConstants_ext.AVRC_PF_GET_NUM_OF_ITEMS_BIT_NO);
            if (mAvrcpBipRsp != null)
                featureBitsList.add(AvrcpConstants_ext.AVRC_PF_COVER_ART_BIT_NO);
        }

        // converting arraylist to array for response
        short[] featureBitsArray = new short[featureBitsList.size()];

        for (int i = 0; i < featureBitsList.size(); i++) {
            featureBitsArray[i] = featureBitsList.get(i).shortValue();
        }

        return featureBitsArray;
    }

    /**
     * Checks the Package name if it supports Browsing or not.
     *
     * @param packageName - name of the package to get the Id.
     * @return true if it supports browsing, else false.
     */
    private boolean isBrowseSupported(String packageName) {
        synchronized (this) {
            synchronized (mBrowsePlayerInfoList) {
                /* check if Browsable Player's list contains this package name */
                for (BrowsePlayerInfo_ext info : mBrowsePlayerInfoList) {
                    if (info.packageName.equals(packageName)) {
                        if (DEBUG) Log.v(TAG, "isBrowseSupported for " + packageName + ": true");
                        return true;
                    }
                }
            }
        }

        if (DEBUG) Log.v(TAG, "isBrowseSupported for " + packageName + ": false");
        return false;
    }

    private String getPackageName(int id) {
        MediaPlayerInfo_ext player = null;
        synchronized (this) {
            synchronized (mMediaPlayerInfoList) {
                player = mMediaPlayerInfoList.getOrDefault(id, null);
            }
        }

        if (player == null) {
            Log.w(TAG, "No package name for player (" + id + " not valid)");
            return "";
        }

        synchronized (this) {
            String packageName = player.getPackageName();
            if (DEBUG) Log.v(TAG, "Player " + id + " package: " + packageName);
            return packageName;
        }
    }

    /* from the global object, getting the current browsed player's package name */
    private String getCurrentBrowsedPlayer(byte[] bdaddr) {
        String browsedPlayerPackage = "";

        Map<String, BrowsedMediaPlayer_ext> connList = mAvrcpBrowseManager.getConnList();
        String bdaddrStr = new String(bdaddr);
        if(connList.containsKey(bdaddrStr)){
            browsedPlayerPackage = connList.get(bdaddrStr).getPackageName();
        }
        if (DEBUG) Log.v(TAG, "getCurrentBrowsedPlayerPackage: " + browsedPlayerPackage);
        return browsedPlayerPackage;
    }

    /* Returns the MediaPlayerInfo for the currently addressed media player */
    private MediaPlayerInfo_ext getAddressedPlayerInfo() {
        synchronized (this) {
            synchronized (mMediaPlayerInfoList) {
                return mMediaPlayerInfoList.getOrDefault(mCurrAddrPlayerID, null);
            }
        }
    }

    /*
     * Utility function to get the Media player info from package name returns
     * null if package name not found in media players list
     */
    private MediaPlayerInfo_ext getMediaPlayerInfo(String packageName) {
        synchronized (this) {
            synchronized (mMediaPlayerInfoList) {
                if (mMediaPlayerInfoList.isEmpty()) {
                    if (DEBUG) Log.v(TAG, "getMediaPlayerInfo: Media players list empty");
                    return null;
                }

                for (MediaPlayerInfo_ext info : mMediaPlayerInfoList.values()) {
                    if (packageName.equals(info.getPackageName())) {
                        if (DEBUG) Log.v(TAG, "getMediaPlayerInfo: Found " + packageName);
                        return info;
                    }
                }
                if (DEBUG) Log.w(TAG, "getMediaPlayerInfo: " + packageName + " not found");
                return null;
            }
        }
    }

    /* prepare media list & return the media player list response object */
    private MediaPlayerListRsp_ext prepareMediaPlayerRspObj() {
        synchronized (this) {
            synchronized (mMediaPlayerInfoList) {
                // TODO(apanicke): This hack will go away as soon as a developer
                // option to enable or disable player selection is created. Right
                // now this is needed to fix BMW i3 carkits and any other carkits
                // that might try to connect to a player that isnt the current
                // player based on this list
                int numPlayers = 1;

                int[] playerIds = new int[numPlayers];
                byte[] playerTypes = new byte[numPlayers];
                int[] playerSubTypes = new int[numPlayers];
                String[] displayableNameArray = new String[numPlayers];
                byte[] playStatusValues = new byte[numPlayers];
                short[] featureBitMaskValues =
                        new short[numPlayers * AvrcpConstants_ext.AVRC_FEATURE_MASK_SIZE];

                // Reserve the first spot for the currently addressed player if
                // we have one
                int players = mMediaPlayerInfoList.containsKey(mCurrAddrPlayerID) ? 1 : 0;
                for (Map.Entry<Integer, MediaPlayerInfo_ext> entry : mMediaPlayerInfoList.entrySet()) {
                    int idx = players;
                    if (entry.getKey() == mCurrAddrPlayerID)
                        idx = 0;
                    else
                        continue; // TODO(apanicke): Remove, see above note
                    MediaPlayerInfo_ext info = entry.getValue();
                    playerIds[idx] = entry.getKey();
                    playerTypes[idx] = info.getMajorType();
                    playerSubTypes[idx] = info.getSubType();
                    displayableNameArray[idx] = info.getDisplayableName();
                    playStatusValues[idx] = info.getPlayStatus();

                    short[] featureBits = info.getFeatureBitMask();
                    short[] featureBitsArray = {0x00, 0x00, 0x00, 0x00, 0x00, 0xb7, 0x01, 0x04,
                                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                    String browsedPackage = getPackageName(mCurrAddrPlayerID);
                    BrowsedMediaPlayer_ext player =
                            mAvrcpBrowseManager.getBrowsedMediaPlayer(dummyaddr);
                    String currPkg = (info != null) ? info.getPackageName():"";
                    if (playerBrowseWhiteListDB.contains(currPkg)) {
                        for (int numBit = 0; numBit < featureBits.length; numBit++) {
                            /* gives which octet this belongs to */
                            byte octet = (byte) (featureBits[numBit] / 8);
                            /* gives the bit position within the octet */
                            byte bit = (byte) (featureBits[numBit] % 8);
                            featureBitMaskValues[(idx * AvrcpConstants_ext.AVRC_FEATURE_MASK_SIZE) + octet] |=
                                    (1 << bit);
                        }
                    } else {
                         featureBitMaskValues =
                                 Arrays.copyOf(featureBitsArray, featureBitsArray.length);
                         Log.w(TAG, "sending bit mask for non Browsable Player");
                    }

                    /* printLogs */
                    if (DEBUG) {
                        Log.d(TAG, "Player " + playerIds[idx] + ": " + displayableNameArray[idx]
                                        + " type: " + playerTypes[idx] + ", " + playerSubTypes[idx]
                                        + " status: " + playStatusValues[idx]);
                    }

                    if (idx != 0) players++;
                }

                if (DEBUG) Log.d(TAG, "prepareMediaPlayerRspObj: numPlayers = " + numPlayers);

                return new MediaPlayerListRsp_ext(AvrcpConstants_ext.RSP_NO_ERROR, sUIDCounter, players,
                        AvrcpConstants_ext.BTRC_ITEM_PLAYER, playerIds, playerTypes, playerSubTypes,
                        playStatusValues, featureBitMaskValues, displayableNameArray);
            }
        }
    }

     /* build media player list and send it to remote. */
    private void handleMediaPlayerListRsp(AvrcpCmd_ext.FolderItemsCmd folderObj) {
        MediaPlayerListRsp_ext rspObj = null;
        synchronized (this) {
            synchronized (mMediaPlayerInfoList) {
                int numPlayers = mMediaPlayerInfoList.size();
                if (numPlayers == 0) {
                    mediaPlayerListRspNative(folderObj.mAddress, AvrcpConstants_ext.RSP_NO_AVBL_PLAY,
                            (short) 0, (byte) 0, 0, null, null, null, null, null, null);
                    return;
                }
                if (folderObj.mStartItem >= numPlayers || folderObj.mStartItem >= 1) {
                    Log.i(TAG, "handleMediaPlayerListRsp: start = " + folderObj.mStartItem
                                    + " > num of items = " + numPlayers);
                    mediaPlayerListRspNative(folderObj.mAddress, AvrcpConstants_ext.RSP_INV_RANGE,
                            (short) 0, (byte) 0, 0, null, null, null, null, null, null);
                    return;
                }
                if (mCurrAddrPlayerID == NO_PLAYER_ID) {
                    short[] featureBitsArray = {0x00, 0x00, 0x00, 0x00, 0x00, 0xb7, 0x01, 0x04,
                                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                    Log.i(TAG, "handleMediaPlayerListRsp: Send dummy player response");
                    mediaPlayerListRspNative(folderObj.mAddress, (int)AvrcpConstants_ext.RSP_NO_ERROR,
                            (int)sUIDCounter, AvrcpConstants_ext.BTRC_ITEM_PLAYER, 1, new int[] {0},
                            new byte[] {AvrcpConstants_ext.PLAYER_TYPE_AUDIO}, new int[] {1},
                            new byte[] {PLAYSTATUS_STOPPED}, featureBitsArray,
                            new String[] {"Dummy Player"});
                    return;
                }
                rspObj = prepareMediaPlayerRspObj();
            }
        }
        if (DEBUG) Log.d(TAG, "handleMediaPlayerListRsp: sending " + rspObj.mNumItems + " players");
        mediaPlayerListRspNative(folderObj.mAddress, rspObj.mStatus, rspObj.mUIDCounter,
                rspObj.mItemType, rspObj.mNumItems, rspObj.mPlayerIds, rspObj.mPlayerTypes,
                rspObj.mPlayerSubTypes, rspObj.mPlayStatusValues, rspObj.mFeatureBitMaskValues,
                rspObj.mPlayerNameList);
    }

    /* unregister to the old controller, update new IDs and register to the new controller */
    private boolean updateCurrentController(int addrId, int browseId) {
        boolean registerRsp = true;
        int preAddrId = mCurrAddrPlayerID;
        int preBrowseId = mCurrBrowsePlayerID;

        updateNewIds(addrId, browseId);

        MediaController newController = null;
        MediaPlayerInfo_ext info = getAddressedPlayerInfo();
        if (info != null) newController = info.getMediaController();

        if (DEBUG)
            Log.d(TAG, "updateCurrentController: " + mMediaController + " to " + newController);
        synchronized (this) {
            if (mMediaController == null || (!mMediaController.equals(newController))) {
                if (mMediaController != null) {
                    mMediaController.unregisterCallback(mMediaControllerCb);
                }
                mMediaController = newController;
                if (mMediaController != null) {
                    mMediaController.registerCallback(mMediaControllerCb, mHandler);
                } else {
                    registerRsp = false;
                    updateNewIds(preAddrId, preBrowseId);
                }
            }
        }
        updateCurrentMediaState(null);
        return registerRsp;
    }

    /* Handle getfolderitems for scope = VFS, Search, NowPlayingList */
    private void handleGetFolderItemBrowseResponse(AvrcpCmd_ext.FolderItemsCmd folderObj, byte[] bdaddr) {
        int status = AvrcpConstants_ext.RSP_NO_ERROR;
        BluetoothDevice device = BluetoothAdapter.getDefaultAdapter().getRemoteDevice(bdaddr);

        /* Browsed player is already set */
        if (folderObj.mScope == AvrcpConstants_ext.BTRC_SCOPE_FILE_SYSTEM) {
            if (mBrowsingActiveDevice != null && !Objects.equals(mBrowsingActiveDevice, device)) {
                Log.e(TAG, "handleGetFolderItemBrowse: Cmd from browse inactive device, reject it");
                getFolderItemsRspNative(bdaddr, AvrcpConstants_ext.RSP_INTERNAL_ERR, (short) 0,
                        (byte) 0x00, 0, null, null, null, null, null, null, null, null);
                return;
            }
            if (mAvrcpBrowseManager.getBrowsedMediaPlayer(bdaddr) == null) {
                Log.e(TAG, "handleGetFolderItemBrowseResponse: no browsed player set for "
                                + Utils.getAddressStringFromByte(bdaddr));
                getFolderItemsRspNative(bdaddr, AvrcpConstants_ext.RSP_INTERNAL_ERR, (short) 0,
                        (byte) 0x00, 0, null, null, null, null, null, null, null, null);
                return;
            }
            mAvrcpBrowseManager.getBrowsedMediaPlayer(bdaddr).getFolderItemsVFS(folderObj);
            return;
        }
        if (folderObj.mScope == AvrcpConstants_ext.BTRC_SCOPE_NOW_PLAYING) {
            mAddressedMediaPlayer.getFolderItemsNowPlaying(bdaddr, folderObj, mMediaController);
            return;
        }

        /* invalid scope */
        Log.e(TAG, "handleGetFolderItemBrowseResponse: unknown scope " + folderObj.mScope);
        getFolderItemsRspNative(bdaddr, AvrcpConstants_ext.RSP_INV_SCOPE, (short) 0, (byte) 0x00, 0,
                null, null, null, null, null, null, null, null);
    }

    /* utility function to update the global values of current Addressed and browsed player */
    private void updateNewIds(int addrId, int browseId) {
        if (DEBUG)
            Log.v(TAG, "updateNewIds: Addressed:" + mCurrAddrPlayerID + " to " + addrId
                            + ", Browse:" + mCurrBrowsePlayerID + " to " + browseId);
        mCurrAddrPlayerID = addrId;
        mCurrBrowsePlayerID = browseId;
    }

    /* Getting the application's displayable name from package name */
    private String getAppLabel(String packageName) {
        ApplicationInfo appInfo = null;
        try {
            appInfo = mPackageManager.getApplicationInfo(packageName, 0);
        } catch (NameNotFoundException e) {
            e.printStackTrace();
        }

        return (String) (appInfo != null ? mPackageManager
                .getApplicationLabel(appInfo) : "Unknown");
    }

    private void handlePlayItemResponse(byte[] bdaddr, byte[] uid, byte scope) {
        BluetoothDevice device = BluetoothAdapter.getDefaultAdapter().getRemoteDevice(bdaddr);
        HeadsetService mService = HeadsetService.getHeadsetService();
        if ((mService != null) && mService.isScoOrCallActive()) {
            Log.w(TAG, "Remote requesting play item while call is active");
            playItemRspNative(bdaddr, AvrcpConstants_ext.RSP_MEDIA_IN_USE);
            return;
        }

        if (mBrowsingActiveDevice != null && !Objects.equals(mBrowsingActiveDevice, device)) {
            Log.w(TAG, "play item Cmd from browse inactive device, reject it");
            playItemRspNative(bdaddr, AvrcpConstants_ext.RSP_INTERNAL_ERR);
            return;
        }

        if (scope == AvrcpConstants_ext.BTRC_SCOPE_NOW_PLAYING) {
            mAddressedMediaPlayer.playItem(bdaddr, uid, mMediaController);
        }
        else {
            if(!isAddrPlayerSameAsBrowsed(bdaddr)) {
                Log.w(TAG, "Remote requesting play item on uid which may not be recognized by" +
                        "current addressed player");
                playItemRspNative(bdaddr, AvrcpConstants_ext.RSP_INV_ITEM);
                return;
            }

            if (mAvrcpBrowseManager.getBrowsedMediaPlayer(bdaddr) != null) {
                mAvrcpBrowseManager.getBrowsedMediaPlayer(bdaddr).playItem(uid, scope);
            } else {
                Log.e(TAG, "handlePlayItemResponse: Remote requested playitem " +
                        "before setbrowsedplayer");
                playItemRspNative(bdaddr, AvrcpConstants_ext.RSP_INTERNAL_ERR);
            }
        }
    }

    private void handleGetItemAttr(AvrcpCmd_ext.ItemAttrCmd itemAttr) {
        if (itemAttr.mUidCounter != sUIDCounter) {
            itemAttr.mUidCounter = sUIDCounter;
            Log.e(TAG, "handleGetItemAttr: invalid uid counter, assign new value = " + itemAttr.mUidCounter);
        }
        BluetoothDevice device =
                BluetoothAdapter.getDefaultAdapter().getRemoteDevice(itemAttr.mAddress);
        if (mBrowsingActiveDevice != null && !Objects.equals(mBrowsingActiveDevice, device)) {
            Log.e(TAG, "Item attributes from browse inactive device, reject it");
            getItemAttrRspNative(
                    itemAttr.mAddress, AvrcpConstants_ext.RSP_INTERNAL_ERR, (byte) 0, null, null);
            return;
        }
        if (itemAttr.mScope == AvrcpConstants_ext.BTRC_SCOPE_NOW_PLAYING) {
            if (mCurrAddrPlayerID == NO_PLAYER_ID) {
                getItemAttrRspNative(
                        itemAttr.mAddress, AvrcpConstants_ext.RSP_NO_AVBL_PLAY, (byte) 0, null, null);
                return;
            }
            mAddressedMediaPlayer.getItemAttr(itemAttr.mAddress, itemAttr, mMediaController);
            return;
        }
        // All other scopes use browsed player
        if (mAvrcpBrowseManager.getBrowsedMediaPlayer(itemAttr.mAddress) != null) {
            mAvrcpBrowseManager.getBrowsedMediaPlayer(itemAttr.mAddress).getItemAttr(itemAttr);
        } else {
            Log.e(TAG, "Could not get attributes. mBrowsedMediaPlayer is null");
            getItemAttrRspNative(
                    itemAttr.mAddress, AvrcpConstants_ext.RSP_INTERNAL_ERR, (byte) 0, null, null);
        }
    }

    private void handleGetTotalNumOfItemsResponse(byte[] bdaddr, byte scope) {
        BluetoothDevice device = BluetoothAdapter.getDefaultAdapter().getRemoteDevice(bdaddr);
        if (mBrowsingActiveDevice != null && !Objects.equals(mBrowsingActiveDevice, device)) {
            getTotalNumOfItemsRspNative(bdaddr, AvrcpConstants_ext.RSP_INTERNAL_ERR, 0, 0);
            Log.w(TAG, "GetTotalNumOfItems: Cmd from browse inactive device reject it");
            return;
        }

        // for scope as media player list
        if (scope == AvrcpConstants_ext.BTRC_SCOPE_PLAYER_LIST) {
            int numPlayers = 0;
            synchronized(this) {
                synchronized (mMediaPlayerInfoList) {
                    numPlayers = mMediaPlayerInfoList.containsKey(mCurrAddrPlayerID) ? 1 : 0;
                }
            }
            if (DEBUG) Log.d(TAG, "handleGetTotalNumOfItemsResponse: " + numPlayers + " players.");
            getTotalNumOfItemsRspNative(bdaddr, AvrcpConstants_ext.RSP_NO_ERROR, 0, numPlayers);
        } else if (scope == AvrcpConstants_ext.BTRC_SCOPE_NOW_PLAYING) {
            if (mMediaController == null) {
                Log.e(TAG, "Could not get Total NumOfItems. mMediaController is null");
                getTotalNumOfItemsRspNative(bdaddr, AvrcpConstants_ext.RSP_NO_AVBL_PLAY, 0, 0);
                return;
            }
            mAddressedMediaPlayer.getTotalNumOfItems(bdaddr, mMediaController);
        } else {
            // for FileSystem browsing scopes as VFS, Now Playing
            if (mAvrcpBrowseManager.getBrowsedMediaPlayer(bdaddr) != null) {
                mAvrcpBrowseManager.getBrowsedMediaPlayer(bdaddr).getTotalNumOfItems(scope);
            } else {
                Log.e(TAG, "Could not get Total NumOfItems. mBrowsedMediaPlayer is null");
                getTotalNumOfItemsRspNative(bdaddr, AvrcpConstants_ext.RSP_INTERNAL_ERR, 0, 0);
            }
        }

    }

    /* check if browsed player and addressed player are same */
    private boolean isAddrPlayerSameAsBrowsed(byte[] bdaddr) {
        String browsedPlayer = getCurrentBrowsedPlayer(bdaddr);

        if (!isPackageNameValid(browsedPlayer)) {
            Log.w(TAG, "Browsed player name empty");
            return false;
        }

        MediaPlayerInfo_ext info = getAddressedPlayerInfo();
        String packageName = (info == null) ? "<none>" : info.getPackageName();
        if (info == null || !packageName.equals(browsedPlayer)) {
            if (DEBUG) Log.d(TAG, browsedPlayer + " is not addressed player " + packageName);
            return false;
        }
        return true;
    }

    /* checks if package name is not null or empty */
    private boolean isPackageNameValid(String browsedPackage) {
        boolean isValid = (browsedPackage != null && browsedPackage.length() > 0);
        if (DEBUG) Log.d(TAG, "isPackageNameValid: browsedPackage = " + browsedPackage +
                "isValid = " + isValid);
        return isValid;
    }

    /* checks if selected addressed player is already addressed */
    private boolean isPlayerAlreadyAddressed(int selectedId) {
        // checking if selected ID is same as the current addressed player id
        boolean isAddressed = (mCurrAddrPlayerID == selectedId);
        if (DEBUG) Log.d(TAG, "isPlayerAlreadyAddressed: isAddressed = " + isAddressed);
        return isAddressed;
    }

    private byte[] getByteAddress(BluetoothDevice device) {
        return Utils.getBytesFromAddress(device.getAddress());
    }

    public void cleanupDeviceFeaturesIndex (int index) {
        Log.i(TAG,"cleanupDeviceFeaturesIndex index:" + index);
        deviceFeatures[index].mCurrentDevice = null;
        deviceFeatures[index].mCurrentPlayState = new PlaybackState.Builder().setState(PlaybackState.STATE_NONE, -1L, 0.0f).build();;
        deviceFeatures[index].mNowPlayingListChangedNT = AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
        deviceFeatures[index].mPlayStatusChangedNT = AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
        deviceFeatures[index].mPlayerStatusChangeNT = AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
        deviceFeatures[index].mTrackChangedNT = AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
        deviceFeatures[index].mPlaybackIntervalMs = 0L;
        deviceFeatures[index].mPlayPosChangedNT = AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
        deviceFeatures[index].mFeatures = 0;
        deviceFeatures[index].mAbsoluteVolume = -1;
        deviceFeatures[index].mLastRspPlayStatus = -1;
        deviceFeatures[index].mLastSetVolume = -1;
        deviceFeatures[index].mLastDirection = 0;
        deviceFeatures[index].mVolCmdSetInProgress = false;
        deviceFeatures[index].mVolCmdAdjustInProgress = false;
        deviceFeatures[index].mAbsVolRetryTimes = 0;
        deviceFeatures[index].mAvailablePlayersChangedNT = AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
        deviceFeatures[index].isActiveDevice = false;
        deviceFeatures[index].mAddrPlayerChangedNT = AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
        deviceFeatures[index].mUidsChangedNT = AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
        deviceFeatures[index].mLastPassthroughcmd = KeyEvent.KEYCODE_UNKNOWN;
        deviceFeatures[index].isAbsoluteVolumeSupportingDevice = false;
        deviceFeatures[index].keyPressState = AvrcpConstants_ext.KEY_STATE_RELEASE; //Key release state
        deviceFeatures[index].mReportedPlayerID = NO_PLAYER_ID;
        deviceFeatures[index].isPlayStatusTimeOut = false;
        deviceFeatures[index].mInitialRemoteVolume = -1;
        deviceFeatures[index].mBlackListVolume = -1;
        deviceFeatures[index].mLastRemoteVolume = -1;
        deviceFeatures[index].mLastLocalVolume = -1;
        deviceFeatures[index].cache_play_cmd = false;
        deviceFeatures[index].mRemoteVolume = -1;
        deviceFeatures[index].mLocalVolume = -1;
    }

    private void onConnectionStateChanged(
            boolean rc_connected, boolean br_connected, byte[] address) {
        BluetoothDevice device = BluetoothAdapter.getDefaultAdapter().getRemoteDevice(address);
        Log.d(TAG, "onConnectionStateChanged " + rc_connected + " " + br_connected + " Addr:"
            + device);
        if (device == null) {
            Log.e(TAG, "onConnectionStateChanged Device is null");
            return;
        }
        if (br_connected == true) {
            mBrowsingActiveDevice = device;
            Log.w(TAG, "onConnectionStateChanged Set Active browse device" + mBrowsingActiveDevice);
            if (mCurrentBrowsingDevice == null && mCachedBrowsePlayer != null &&
                    is_player_updated_for_browse == false) {
                Log.w(TAG, "Calling SetBrowsePackage as part of connect for "+ mCachedBrowsePlayer);
                SetBrowsePackage(mCachedBrowsePlayer);
            }
            return;
        }
        int newState = (rc_connected ? BluetoothProfile.STATE_CONNECTED :
            BluetoothProfile.STATE_DISCONNECTED);
        Message msg = mHandler.obtainMessage(MSG_SET_AVRCP_CONNECTED_DEVICE, newState, 0, device);
        mHandler.sendMessage(msg);
        Log.v(TAG, "Exit onConnectionStateChanged");
    }

    public void dump(StringBuilder sb) {
        sb.append("AVRCP:\n");
        for (int i = 0; i < maxAvrcpConnections; i++) {
            Log.v(TAG,"for index " + i);
            ProfileService.println(sb, "mMediaAttributes: " + mMediaAttributes);
            ProfileService.println(sb, "mTransportControlFlags: " + mTransportControlFlags);
            ProfileService.println(sb, "mTracksPlayed: " + deviceFeatures[i].mTracksPlayed);
            ProfileService.println(sb, "mCurrentPlayState: " + deviceFeatures[i].mCurrentPlayState);
            ProfileService.println(sb, "mLastStateUpdate: " + mLastStateUpdate);
            ProfileService.println(sb, "mPlayStatusChangedNT: " + deviceFeatures[i].mPlayStatusChangedNT);
            ProfileService.println(sb, "mTrackChangedNT: " + deviceFeatures[i].mTrackChangedNT);
            ProfileService.println(sb, "mLastStateUpdate: " + mLastStateUpdate);
            ProfileService.println(sb, "mSongLengthMs: " + mSongLengthMs);
            ProfileService.println(sb, "mPlaybackIntervalMs: " + deviceFeatures[i].mPlaybackIntervalMs);
            ProfileService.println(sb, "mPlayPosChangedNT: " + deviceFeatures[i].mPlayPosChangedNT);
            ProfileService.println(sb, "mNextPosMs: " + deviceFeatures[i].mNextPosMs);
            ProfileService.println(sb, "mPrevPosMs: " + deviceFeatures[i].mPrevPosMs);
            ProfileService.println(sb, "mFeatures: " + deviceFeatures[i].mFeatures);
            ProfileService.println(sb, "mRemoteVolume: " + deviceFeatures[i].mRemoteVolume);
            ProfileService.println(sb, "mLastRemoteVolume: " + deviceFeatures[i].mLastRemoteVolume);
            ProfileService.println(sb, "mAbsoluteVolume: " + deviceFeatures[i].mAbsoluteVolume);
            ProfileService.println(sb, "mLastSetVolume: " + deviceFeatures[i].mLastSetVolume);
            ProfileService.println(sb, "mLastDirection: " + deviceFeatures[i].mLastDirection);
            ProfileService.println(sb, "mVolumeStep: " + mVolumeStep);
            ProfileService.println(sb, "mAudioStreamMax: " + mAudioStreamMax);
            ProfileService.println(sb, "mVolCmdSetInProgress: " + deviceFeatures[i].mVolCmdSetInProgress);
            ProfileService.println(sb, "mVolCmdAdjustInProgress: " + deviceFeatures[i].mVolCmdAdjustInProgress);
            ProfileService.println(sb, "mAbsVolRetryTimes: " + deviceFeatures[i].mAbsVolRetryTimes);

        }
        synchronized (this) {
            if (mMediaController != null)
                ProfileService.println(sb, "mMediaController: "
                                + mMediaController.getWrappedInstance() + " pkg "
                                + mMediaController.getPackageName());
        }
        ProfileService.println(sb, "");
        ProfileService.println(sb, "Media Players:");
        synchronized (this) {
            synchronized (mMediaPlayerInfoList) {
                for (Map.Entry<Integer, MediaPlayerInfo_ext> entry : mMediaPlayerInfoList.entrySet()) {
                    int key = entry.getKey();
                    ProfileService.println(sb, ((mCurrAddrPlayerID == key) ? " *#" : "  #")
                                    + entry.getKey() + ": " + entry.getValue());
                }
            }
        }

        ProfileService.println(sb, "");
        mAddressedMediaPlayer.dump(sb, mMediaController);

        ProfileService.println(sb, "");
        ProfileService.println(sb, mPassthroughDispatched + " passthrough operations: ");
        if (mPassthroughDispatched > mPassthroughLogs.size())
            ProfileService.println(sb, "  (last " + mPassthroughLogs.size() + ")");
        synchronized (mPassthroughLogs) {
            for (MediaKeyLog log : mPassthroughLogs) {
                ProfileService.println(sb, "  " + log);
            }
        }
        synchronized (mPassthroughPending) {
            for (MediaKeyLog log : mPassthroughPending) {
                ProfileService.println(sb, "  " + log);
            }
        }

        // Print the blacklisted devices (for absolute volume control)
        SharedPreferences pref =
                mContext.getSharedPreferences(ABSOLUTE_VOLUME_BLACKLIST, Context.MODE_PRIVATE);
        Map<String, ?> allKeys = pref.getAll();
        ProfileService.println(sb, "");
        ProfileService.println(sb, "Runtime Blacklisted Devices (absolute volume):");
        if (allKeys.isEmpty()) {
            ProfileService.println(sb, "  None");
        } else {
            for (String key : allKeys.keySet()) {
                ProfileService.println(sb, "  " + key);
            }
        }
    }

    public class AvrcpBrowseManager {
        Map<String, BrowsedMediaPlayer_ext> connList = new ConcurrentHashMap<String, BrowsedMediaPlayer_ext>();
        private AvrcpMediaRspInterface_ext mMediaInterface;
        private Context mContext;

        public AvrcpBrowseManager(Context context, AvrcpMediaRspInterface_ext mediaInterface) {
            mContext = context;
            mMediaInterface = mediaInterface;
        }

        public void cleanup() {
            Iterator entries = connList.entrySet().iterator();
            while (entries.hasNext()) {
                Map.Entry entry = (Map.Entry) entries.next();
                BrowsedMediaPlayer_ext browsedMediaPlayer = (BrowsedMediaPlayer_ext) entry.getValue();
                if (browsedMediaPlayer != null) {
                    browsedMediaPlayer.cleanup();
                }
            }
            // clean up the map
            connList.clear();
        }

        // get the a free media player interface based on the passed bd address
        // if the no items is found for the passed media player then it assignes a
        // available media player interface
        public BrowsedMediaPlayer_ext getBrowsedMediaPlayer(byte[] bdaddr) {
            BrowsedMediaPlayer_ext mediaPlayer;
            String bdaddrStr = new String(bdaddr);
            if (connList.containsKey(bdaddrStr)) {
                mediaPlayer = connList.get(bdaddrStr);
            } else {
                mediaPlayer = new BrowsedMediaPlayer_ext(bdaddr, mContext, mMediaInterface, mAvrcp);
                connList.put(bdaddrStr, mediaPlayer);
            }
            return mediaPlayer;
        }

        // clears the details pertaining to passed bdaddres
        public boolean clearBrowsedMediaPlayer(byte[] bdaddr) {
            String bdaddrStr = new String(bdaddr);
            if (connList.containsKey(bdaddrStr)) {
                connList.remove(bdaddrStr);
                return true;
            }
            return false;
        }

        public Map<String, BrowsedMediaPlayer_ext> getConnList() {
            return connList;
        }

        /* Helper function to convert colon separated bdaddr to byte string */
        private byte[] hexStringToByteArray(String s) {
            int len = s.length();
            byte[] data = new byte[len / 2];
            for (int i = 0; i < len; i += 2) {
                data[i / 2] = (byte) ((Character.digit(s.charAt(i), 16) << 4)
                        + Character.digit(s.charAt(i+1), 16));
            }
            return data;
        }
    }

    private class AvrcpPlayerAppSettingsRsp implements AvrcpPlayerAppSettingsRspInterface {
        private static final String TAG = "AvrcpPlayerAppSettingsRsp";

        public void getListPlayerappAttrRsp(byte attr, byte[] attrIds, byte[] address) {
            if (!getListPlayerappAttrRspNative(attr, attrIds, address)) {
                Log.e(TAG, "getListPlayerappAttrRspNative failed!");
            }
        }

        public void getPlayerAppValueRsp(byte numberattr, byte[]values, byte[] address) {
            if (!getPlayerAppValueRspNative(numberattr, values, address)) {
                Log.e(TAG, "getPlayerAppValueRspNative failed!");
            }
        }

        public void SendCurrentPlayerValueRsp(byte numberattr, byte[]attr, byte[] address) {
            if (!SendCurrentPlayerValueRspNative(numberattr, attr, address)) {
                Log.e(TAG, "SendCurrentPlayerValueRspNative failed!");
            }
        }

        public void SendSetPlayerAppRsp(int attr_status, byte[] address) {
            if (attr_status != AvrcpConstants_ext.RSP_INTERNAL_ERR) {
                for (int i = 0; i < maxAvrcpConnections; i++) {
                    if (deviceFeatures[i].mCurrentDevice != null &&
                        deviceFeatures[i].mPlayerStatusChangeNT ==
                            AvrcpConstants_ext.NOTIFICATION_TYPE_INTERIM) {
                        Log.v(TAG,"device has registered for mPlayerAppSettingStatusChangeNT");
                        deviceFeatures[i].mPlayerStatusChangeNT =
                                AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
                        mAvrcpPlayerAppSettings.sendPlayerAppChangedRsp(
                                deviceFeatures[i].mPlayerStatusChangeNT,
                                deviceFeatures[i].mCurrentDevice);
                    } else {
                        Log.v(TAG,"Drop Set Attr Val update from media player");
                    }
                }
            }
            if ((address != null) && (!SendSetPlayerAppRspNative(attr_status, address))) {
                Log.e(TAG, "SendSetPlayerAppRspNative failed!");
            }

        }

        public void sendSettingsTextRsp(int num_attr, byte[] attr, int length, String[] text,
                byte[] address) {
            if (!sendSettingsTextRspNative(num_attr, attr, length, text, address)) {
                Log.e(TAG, "sendSettingsTextRspNative failed!");
            }
        }

        public void sendValueTextRsp(int num_attr, byte[] attr, int length, String[] text,
                byte[] address) {
            if (!sendValueTextRspNative(num_attr, attr, length, text, address)) {
                Log.e(TAG, "sendValueTextRspNative failed!");
            }
        }

        public void registerNotificationPlayerAppRsp(int type, byte numberattr, byte[] attr,
                byte[] address) {
            if (!registerNotificationPlayerAppRspNative(type, numberattr, attr, address)) {
                Log.e(TAG, "registerNotificationPlayerAppRspNative failed!");
            }
        }

        public BluetoothDevice GetPlayerSettingCmdPendingDevice(Integer reponse) {
            BluetoothDevice device = null;
            mHandler.removeMessages(MESSAGE_PLAYERSETTINGS_TIMEOUT);
            for (int i = 0; i < maxAvrcpConnections; i++) {
                if ((deviceFeatures[i].mMusicAppCmdResponsePending
                        .containsKey(reponse))) {
                    CreateMusicSettingsAppCmdLookupOrUpdate(reponse, i, false);
                    device = deviceFeatures[i].mCurrentDevice;
                    break;
                }
            }
            return device;
        }

    }
    /*
     * private class which handles responses from AvrcpMediaManager. Maps responses to native
     * responses. This class implements the AvrcpMediaRspInterface interface.
     */
    private class AvrcpMediaRsp implements AvrcpMediaRspInterface_ext {
        private static final String TAG = "AvrcpMediaRsp";

        public void setAddrPlayerRsp(byte[] address, int rspStatus) {
            if (!setAddressedPlayerRspNative(address, rspStatus)) {
                Log.e(TAG, "setAddrPlayerRsp failed!");
            }
        }

        public void setBrowsedPlayerRsp(byte[] address, int rspStatus, byte depth, int numItems,
                String[] textArray) {
            if (!setBrowsedPlayerRspNative(address, rspStatus, depth, numItems, textArray)) {
                Log.e(TAG, "setBrowsedPlayerRsp failed!");
            }
            else {
                if (depth == 0) {
                    changePathDepth = 0;
                    changePathFolderType = 0;
                }
            }
        }

        public void mediaPlayerListRsp(byte[] address, int rspStatus, MediaPlayerListRsp_ext rspObj) {
            if (rspObj != null && rspStatus == AvrcpConstants_ext.RSP_NO_ERROR) {
                if (!mediaPlayerListRspNative(address, rspStatus, sUIDCounter, rspObj.mItemType,
                            rspObj.mNumItems, rspObj.mPlayerIds, rspObj.mPlayerTypes,
                            rspObj.mPlayerSubTypes, rspObj.mPlayStatusValues,
                            rspObj.mFeatureBitMaskValues, rspObj.mPlayerNameList))
                    Log.e(TAG, "mediaPlayerListRsp failed!");
            } else {
                Log.e(TAG, "mediaPlayerListRsp: rspObj is null");
                if (!mediaPlayerListRspNative(address, rspStatus, sUIDCounter, (byte) 0x00, 0, null,
                            null, null, null, null, null))
                    Log.e(TAG, "mediaPlayerListRsp failed!");
            }
        }

        public void folderItemsRsp(byte[] address, int rspStatus, FolderItemsRsp_ext rspObj) {
            if (rspObj != null && rspStatus == AvrcpConstants_ext.RSP_NO_ERROR) {
                String Album = new String("Albums");
                String Artist = new String("Artists");
                String Playlist = new String("Playlists");
                for (int index = 0; index < rspObj.mDisplayNames.length; index++) {
                    if (rspObj.mDisplayNames[index].equals(Album))
                        rspObj.mFolderTypes[index] = AvrcpConstants_ext.FOLDER_TYPE_ALBUMS;
                    else if (rspObj.mDisplayNames[index].equals(Artist))
                        rspObj.mFolderTypes[index] = AvrcpConstants_ext.FOLDER_TYPE_ARTISTS;
                    else if (rspObj.mDisplayNames[index].equals(Playlist))
                        rspObj.mFolderTypes[index] = AvrcpConstants_ext.FOLDER_TYPE_PLAYLISTS;
                    /*by default for every folder filling folder type Titles*/
                    else
                        if (changePathFolderType > 0)
                            rspObj.mFolderTypes[index] = changePathFolderType;
                        else
                            rspObj.mFolderTypes[index] = AvrcpConstants_ext.FOLDER_TYPE_TITLES;
                }
                Log.v(TAG, " changePathDepth " + changePathDepth +
                        " changePathFolderType " + changePathFolderType);
                if (changePathDepth == 0) {
                    saveRspObj = rspObj;
                    changePathFolderType = 0;
                }
                if (!getFolderItemsRspNative(address, rspStatus, sUIDCounter, rspObj.mScope,
                        rspObj.mNumItems, rspObj.mFolderTypes, rspObj.mPlayable, rspObj.mItemTypes,
                        rspObj.mItemUid, rspObj.mDisplayNames, rspObj.mAttributesNum,
                        rspObj.mAttrIds, rspObj.mAttrValues))
                    Log.e(TAG, "getFolderItemsRspNative failed!");
            } else {
                Log.e(TAG, "folderItemsRsp: rspObj is null or rspStatus is error:" + rspStatus);
                if (!getFolderItemsRspNative(address, rspStatus, sUIDCounter, (byte) 0x00, 0,
                        null, null, null, null, null, null, null, null))
                    Log.e(TAG, "getFolderItemsRspNative failed!");
            }

        }

        public void changePathRsp(byte[] address, int rspStatus, int numItems) {
            /*to handle changePath invalid uid scenario or any error sceanrio */
            if (rspStatus != AvrcpConstants_ext.RSP_NO_ERROR && changePathDepth>0) {
                if(changePathDirection == 1)
                    changePathDepth--;
                else
                    changePathDepth++;
            }
            if (!changePathRspNative(address, rspStatus, numItems))
                Log.e(TAG, "changePathRspNative failed!");
        }

        public void getItemAttrRsp(byte[] address, int rspStatus, ItemAttrRsp_ext rspObj) {
            if (rspObj != null && rspStatus == AvrcpConstants_ext.RSP_NO_ERROR) {
                if (!getItemAttrRspNative(address, rspStatus, rspObj.mNumAttr,
                        rspObj.mAttributesIds, rspObj.mAttributesArray))
                    Log.e(TAG, "getItemAttrRspNative failed!");
            } else {
                Log.e(TAG, "getItemAttrRsp: rspObj is null or rspStatus is error:" + rspStatus);
                if (!getItemAttrRspNative(address, rspStatus, (byte) 0x00, null, null))
                    Log.e(TAG, "getItemAttrRspNative failed!");
            }
        }

        public void playItemRsp(byte[] address, int rspStatus) {
            BluetoothDevice device;
            String bdaddr = Utils.getAddressStringFromByte(address);
            device = mAdapter.getRemoteDevice(bdaddr);
            int deviceIndex = getIndexForDevice(device);
            if (deviceIndex == INVALID_DEVICE_INDEX) {
                Log.e(TAG,"Invalid device index for playItemRsp");
                return;
            }

            if((rspStatus == AvrcpConstants_ext.RSP_NO_ERROR) && ((mA2dpService != null) &&
                    !Objects.equals(mA2dpService.getActiveDevice(), device))) {
                Log.d(TAG, "Trigger Handoff by playItem");
                startSHO(device, true);
            }
            if (!playItemRspNative(address, rspStatus)) {
                Log.e(TAG, "playItemRspNative failed!");
            }
        }

        public void getTotalNumOfItemsRsp(byte[] address, int rspStatus, int uidCounter,
                int numItems) {
            if (!getTotalNumOfItemsRspNative(address, rspStatus, sUIDCounter, numItems)) {
                Log.e(TAG, "getTotalNumOfItemsRspNative failed!");
            }
        }

        public void addrPlayerChangedRsp(int type, int playerId, int uidCounter) {
            byte[] addr = getActiveDeviceAddress();
            addrPlayerChangedRsp(type, playerId, uidCounter, addr);
        }

        public void addrPlayerChangedRsp(int type, int playerId, int uidCounter, byte[] address) {
            if (!registerNotificationRspAddrPlayerChangedNative(type, playerId, sUIDCounter, address)) {
                Log.e(TAG, "registerNotificationRspAddrPlayerChangedNative failed!");
            }
        }

        public void avalPlayerChangedRsp(byte[] address, int type) {
            if (!registerNotificationRspAvalPlayerChangedNative(type, address)) {
                Log.e(TAG, "registerNotificationRspAvalPlayerChangedNative failed!");
            }
        }

        public void uidsChangedRsp(int type) {
            byte[] addr = null;
            int index = INVALID_DEVICE_INDEX;
            for (int i = 0; i < maxAvrcpConnections; i++) {
                if ((deviceFeatures[i].mCurrentDevice != null) &&
                    deviceFeatures[i].isActiveDevice) {
                    addr = getByteAddress(deviceFeatures[i].mCurrentDevice);
                    index = i;
                    break;
                }
            }

            if ((index != INVALID_DEVICE_INDEX) &&
                    (deviceFeatures[index].mUidsChangedNT != AvrcpConstants_ext.NOTIFICATION_TYPE_INTERIM)) {
                if (DEBUG) Log.d(TAG, "uidsChangedRsp: Not registered or requesting.");
                return;
            }

            if (addr == null) {
                Log.e(TAG,"uidsChangedRsp:No active device found");
                return;
            }
            if (!registerNotificationRspUIDsChangedNative(type, sUIDCounter, addr)) {
                Log.e(TAG, "registerNotificationRspUIDsChangedNative failed!");
            }
            if (index != INVALID_DEVICE_INDEX)
                deviceFeatures[index].mUidsChangedNT = AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
        }

        public void nowPlayingChangedRsp(int type) {
            nowPlayingChangedRsp(type, null);
        }

        public void nowPlayingChangedRsp(int type, byte[] address) {
            byte[] addr = null;
            for (int i = 0; i < maxAvrcpConnections; i++) {
                if (deviceFeatures[i].isActiveDevice) {
                    addr = getByteAddress(deviceFeatures[i].mCurrentDevice);
                    deviceFeatures[i].mNowPlayingListChangedNT = type;
                    break;
                }
            }
            if (addr == null) {
                Log.e(TAG,"uidsChangedRsp:No active device found");
                return;
            }
            if (mNowPlayingListChangedNT != AvrcpConstants_ext.NOTIFICATION_TYPE_INTERIM) {
                if (DEBUG) Log.d(TAG, "NowPlayingListChanged: Not registered or requesting.");
                return;
            }

            if (!registerNotificationRspNowPlayingChangedNative(type, addr)) {
                Log.e(TAG, "registerNotificationRspNowPlayingChangedNative failed!");
            }
            mNowPlayingListChangedNT = AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
        }

        public void trackChangedRsp(int type, byte[] uid) {
            byte[] addr = getActiveDeviceAddress();
            trackChangedRsp(type, uid, addr);
        }

        public void trackChangedRsp(int type, byte[] uid, byte[] addr) {
            if (!registerNotificationRspTrackChangeNative(type, uid, addr)) {
                Log.e(TAG, "registerNotificationRspTrackChangeNative failed!");
            }
        }

        public byte[] getActiveDeviceAddress() {
            byte[] addr = null;
            for (int i = 0; i < maxAvrcpConnections; i++) {
                if (deviceFeatures[i].isActiveDevice) {
                    addr = getByteAddress(deviceFeatures[i].mCurrentDevice);
                    break;
                }
            }
            return addr;
        }
    }

    private int getIndexForDevice(BluetoothDevice device) {
        for (int i = 0; i < maxAvrcpConnections; i++) {
            if (deviceFeatures[i].mCurrentDevice != null && device != null &&
                    Objects.equals(deviceFeatures[i].mCurrentDevice, device)) {
                Log.i(TAG,"device found at index " + i);
                return i;
            }
        }
        Log.e(TAG, "returning invalid index");
        return INVALID_DEVICE_INDEX;
    }

    private int getActiveDeviceIndex() {
        for (int i = 0; i < maxAvrcpConnections; i++) {
            if (deviceFeatures[i].mCurrentDevice != null &&
                deviceFeatures[i].isActiveDevice) {
                Log.d(TAG,"Active device index: " + i);
                return i;
            }
        }
        return INVALID_DEVICE_INDEX;
    }

    public void removeVolumeForDevice(BluetoothDevice device) {
        if (device == null || device.getBondState() != BluetoothDevice.BOND_NONE)
            return;
        SharedPreferences.Editor pref = getVolumeMap().edit();
        SharedPreferences.Editor prefAbs = getAbsVolumeMap().edit();
        Log.i(TAG, "RemoveStoredVolume: Remove stored stream volume level and abs volume support for device " + device);
        mVolumeMap.remove(device);
        pref.remove(device.getAddress());
        // Always use apply() since it is asynchronous, otherwise the call can hang waiting for
        // storage to be written.
        pref.apply();
        //remove absoute volume support
        mDeviceAbsVolMap.remove(device);
        prefAbs.remove(device.getAddress());
        // Always use apply() since it is asynchronous, otherwise the call can hang waiting for
        // storage to be written.
        prefAbs.apply();
    }

    public void storeVolumeForDevice(BluetoothDevice device) {
        /*Due to race condtion between active device change and bond state change, if bond state
         change event handled first and active device change later than volume will not stored
         if bond state is not paired */
        if (device != null && device.getBondState() == BluetoothDevice.BOND_NONE) {
            Log.i(TAG, "storeVolume: not saved because bond state is none");
            return;
        }
        SharedPreferences.Editor pref = getVolumeMap().edit();
        int index = getIndexForDevice(device);
        int storeVolume =  mAudioManager.getStreamVolume(AudioManager.STREAM_MUSIC);
        Log.i(TAG, "storeVolume: Storing stream volume level for device " + device
                + " : " + storeVolume);

        List<BluetoothDevice> HAActiveDevices = null;
        HearingAidService mHaService = HearingAidService.getHearingAidService();
        if (mHaService != null) {
          HAActiveDevices = mHaService.getActiveDevices();
        }
        if (HAActiveDevices != null && (HAActiveDevices.get(0) != null
                || HAActiveDevices.get(1) != null)) {
          Log.d(TAG, "Do not store volume when Hearing Aid device is active");
          return;
        }

        if (index == INVALID_DEVICE_INDEX && disconnectedActiveDevice != null &&
            (disconnectedActiveDevice.equals(device)
            || isTwsPlusPair(disconnectedActiveDevice, device))) {
            Log.v(TAG, "No need to store volume again during avrcp disconnect volume is stored");
            disconnectedActiveDevice = null;
            return;
        }
        mVolumeMap.put(device, storeVolume);
        pref.putInt(device.getAddress(), storeVolume);
        if (device != null && device.isTwsPlusDevice()) {
            AdapterService mAdapterService = AdapterService.getAdapterService();
            BluetoothDevice peerDevice = mAdapterService.getTwsPlusPeerDevice(device);
            if (peerDevice != null && getIndexForDevice(peerDevice) != INVALID_DEVICE_INDEX)
                Log.d(TAG,"storeVolume to TWS+ pair device " + peerDevice + " : " + storeVolume);
                mVolumeMap.put(peerDevice, storeVolume);
                pref.putInt(peerDevice.getAddress(), storeVolume);
        }
        // Always use apply() since it is asynchronous, otherwise the call can hang waiting for
        // storage to be written.
        pref.apply();
        //below case to handle if to keep track of mLocal for not supported abs vol
        if(index != INVALID_DEVICE_INDEX &&
            !deviceFeatures[index].isAbsoluteVolumeSupportingDevice) {
            deviceFeatures[index].mLocalVolume = storeVolume;
        }
    }

    public boolean startSHO(BluetoothDevice device, boolean PlayReq) {
        HeadsetService headsetService = HeadsetService.getHeadsetService();
        boolean isInCall = headsetService != null && headsetService.isScoOrCallActive();
        boolean isFMActive = mAudioManager.getParameters("fm_status").contains("1");
        Log.d(TAG, "0: SHO Init: isInCall = " + isInCall + " isFMActive = " + isFMActive);
        synchronized (Avrcp_ext.this) {
            if (isShoActive) {
                mHandler.removeMessages (MESSAGE_START_SHO);
                Message msg = mHandler.obtainMessage(MESSAGE_START_SHO, PlayReq?1:0, 0, device);
                SHOQueue.device = device;
                SHOQueue.PlayReq = PlayReq;
                SHOQueue.isRetry = false;
                mHandler.sendMessageDelayed(msg, 3000);
                Log.d(TAG, "4: SHO Queued");
                return true;
            } else {
                isShoActive = true;
                Log.d(TAG, "5: SHO started: PlayReq = " + PlayReq);
            }
        }
        boolean ret = mA2dpService.startSHO(device);
        if(!ret) {
            isShoActive = false;
            if (device.isTwsPlusDevice()) {
                BluetoothDevice activeDevice = mA2dpService.getActiveDevice();
                if (activeDevice != null &&
                    isTwsPlusPair(device, activeDevice)) {
                    Log.e(TAG,"TWS+ switch ignored, do not retry sho");
                    CompleteSHO();
                    return ret;
                }
            }
            mHandler.removeMessages(MESSAGE_START_SHO);
            triggerSHO(device, PlayReq, true);
            return ret;
        }
        synchronized (Avrcp_ext.this) {
            if (!PlayReq || isInCall || isFMActive) {
                isShoActive = false;
                Log.d(TAG, "6: SHO complete");

                if (mHandler.hasMessages(MESSAGE_START_SHO)) {
                    mHandler.removeMessages(MESSAGE_START_SHO);
                    triggerSHO(SHOQueue.device, SHOQueue.PlayReq, false);
                }
            }
        }
        CompleteSHO();
        return ret;
    }

    private void triggerSHO(BluetoothDevice device, boolean PlayReq, boolean isRetry) {
        Message msg = mHandler.obtainMessage(MESSAGE_START_SHO, PlayReq?1:0, isRetry?1:0, device);
        if(isRetry) {
            SHOQueue.device = device;
            SHOQueue.PlayReq = PlayReq;
            SHOQueue.isRetry = true;
            mHandler.sendMessageDelayed(msg, 2000);
            Log.e(TAG, "Retry SHO after delay");
        } else {
            SHOQueue.isRetry = false;
            mHandler.sendMessage(msg);
        }
    }

    public void CompleteSHO() {
        /*For device setup after SHO*/
    }

    public void setActiveDevice(BluetoothDevice device) {
        Log.w(TAG, "setActiveDevice call for device " + device);
        Message msg = mHandler.obtainMessage(MSG_SET_ACTIVE_DEVICE, 0, 0, device);
        mHandler.sendMessage(msg);
    }

    private SharedPreferences getVolumeMap() {
        return mContext.getSharedPreferences(VOLUME_MAP, Context.MODE_PRIVATE);
    }

    private SharedPreferences getAbsVolumeMap() {
        return mContext.getSharedPreferences(ABS_VOL_MAP, Context.MODE_PRIVATE);
    }

    private void Avrcp_extVolumeManager() {
        // Load the stored volume preferences into a hash map since shared preferences are slow
        // to poll and update. If the device has been unbonded since last start remove it from
        // the map.
        Map<String, ?> allKeys = getVolumeMap().getAll();
        SharedPreferences.Editor volumeMapEditor = getVolumeMap().edit();
        for (Map.Entry<String, ?> entry : allKeys.entrySet()) {
            String key = entry.getKey();
            Object value = entry.getValue();
            BluetoothDevice d = BluetoothAdapter.getDefaultAdapter().getRemoteDevice(key);

            if (value instanceof Integer && d.getBondState() == BluetoothDevice.BOND_BONDED) {
                mVolumeMap.put(d, (Integer) value);
                Log.w(TAG, "address " + key + " from the volume map volume :" + value);
            } else {
                Log.w(TAG, "Removing " + key + " from the volume map");
                volumeMapEditor.remove(key);
            }
        }
        volumeMapEditor.apply();

        allKeys = getAbsVolumeMap().getAll();
        SharedPreferences.Editor absVolumeMapEditor = getAbsVolumeMap().edit();
        for (Map.Entry<String, ?> entry : allKeys.entrySet()) {
            String key = entry.getKey();
            Object value = entry.getValue();
            BluetoothDevice d = BluetoothAdapter.getDefaultAdapter().getRemoteDevice(key);

            if (value instanceof Boolean && d.getBondState() == BluetoothDevice.BOND_BONDED) {
                mDeviceAbsVolMap.put(d, (Boolean) value);
                Log.w(TAG, "Address " + key + " from the absolute volume map support :" + value);
            } else {
                Log.w(TAG, "Removing " + key + " from the absolute volume map support state" + d.getBondState());
                absVolumeMapEditor.remove(key);
            }
        }
        absVolumeMapEditor.apply();
    }

    public int getVolume(BluetoothDevice device) {
        if (!mVolumeMap.containsKey(device)) {
            Log.w(TAG, "getVolume: Couldn't find volume preference for device: " + device);
            return mAudioStreamMax/2;
        }
        Log.d(TAG, "getVolume: Returning volume " + mVolumeMap.get(device));
        return mVolumeMap.get(device);
    }

    public void setAbsVolumeFlag(BluetoothDevice device) {
        int deviceIndex = getIndexForDevice(device);
        int volume = getVolume(device);
        Log.d(TAG, " device allocated for abs volume support " + mDeviceAbsVolMap.containsKey(device));
        if (mDeviceAbsVolMap.containsKey(device)) {
            //updating abs volume supported or not to audio when active device change is success
            Log.d(TAG, " Returning abs volume support " + mDeviceAbsVolMap.get(device));
            mAudioManager.avrcpSupportsAbsoluteVolume(device.getAddress(), mDeviceAbsVolMap.get(device));
        } else {
            //absolute volume is not stored upadtion false by default
            Log.d(TAG, "Returning abs volume support false ");
            mAudioManager.avrcpSupportsAbsoluteVolume(device.getAddress(), false);
        }
        if (deviceIndex == INVALID_DEVICE_INDEX) {
            Log.e(TAG,"Invalid device index for setAbsVolumeFlag");
            return;
        }
        if(deviceFeatures[deviceIndex].isAbsoluteVolumeSupportingDevice == false) {
            Log.d(TAG,"isAbsoluteVolumeSupportingDevice is false or volume is not stored");
            return;
        }
        if(deviceFeatures[deviceIndex].mInitialRemoteVolume == -1) {
            Log.e(TAG,"intial volume is not updated or volume is not stored");
            return;
        }
        Message msg = mHandler.obtainMessage();
        msg.what = MESSAGE_UPDATE_ABS_VOLUME_STATUS;
        msg.arg1 = deviceIndex;
        msg.arg2 = volume;
        mHandler.sendMessage(msg);
        Log.d(TAG,"setAbsVolumeFlag = " + isAbsoluteVolumeSupported(deviceIndex));
        return;
    }

    /* getters for some private variables */
    public AvrcpBrowseManager getAvrcpBrowseManager() {
        return mAvrcpBrowseManager;
    }

    public void trackChangedAddressedRsp(int type, byte[] uid, byte[] addr) {
        if (!registerNotificationRspTrackChangeNative(type, uid, addr)) {
            Log.e(TAG, "registerNotificationRspTrackChangeNative failed!");
        }
    }

    /* PASSTHROUGH COMMAND MANAGEMENT */

    void handlePassthroughCmd(byte[] bdaddr, int op, int state) {
        int code = avrcpPassthroughToKeyCode(op);
        if (code == KeyEvent.KEYCODE_UNKNOWN) {
            Log.w(TAG, "Ignoring passthrough of unknown key " + op + " state " + state);
            return;
        }
        BluetoothDevice device;
        String address = Utils.getAddressStringFromByte(bdaddr);
        device = mAdapter.getRemoteDevice(address);
        int deviceIndex = getIndexForDevice(device);
        if (deviceIndex == INVALID_DEVICE_INDEX) {
            Log.e(TAG,"Invalid device index for passthrough command");
            return;
        }
        Log.d(TAG, "passthrough from device: " + address);
        if (mA2dpService != null)
            Log.d(TAG, "Active device: " + mA2dpService.getActiveDevice());

        int action = KeyEvent.ACTION_DOWN;
        if (state == AvrcpConstants_ext.KEY_STATE_RELEASE) action = KeyEvent.ACTION_UP;
        BluetoothDevice a2dp_active_device = null;
        boolean skip = false;;
        if (mA2dpService != null) a2dp_active_device = mA2dpService.getActiveDevice();
        if (a2dp_active_device != null) {
            if (a2dp_active_device.isTwsPlusDevice() &&
                (isTwsPlusPair(a2dp_active_device, device) ||
                Objects.equals(a2dp_active_device, device))) {
                Log.d(TAG,"Passthrough received from TWS+ Pair");
                skip = true;
            }
        }
        if (a2dp_active_device == null &&
            code == KeyEvent.KEYCODE_MEDIA_PLAY) {
            if (action == KeyEvent.ACTION_DOWN) {
                deviceFeatures[deviceIndex].cache_play_cmd = true;
            } else if (action == KeyEvent.ACTION_UP &&
                       deviceFeatures[deviceIndex].cache_play_cmd) {
                Log.d(TAG,"play cmd cached, ignore release");
            }
        }
        if (!skip && (mA2dpService != null) && !Objects.equals(a2dp_active_device, device)) {
            Log.w(TAG, "code " + code + " action " + action + " from inactive device");
            if (code == KeyEvent.KEYCODE_MEDIA_PLAY) {
                if (isPlayingState(mCurrentPlayerState) &&
                        mAudioManager.isMusicActive() &&
                        (mA2dpState == BluetoothA2dp.STATE_PLAYING)) {
                    ignore_play = true;
                    deviceFeatures[deviceIndex].cache_play_cmd = false;
                }
                if (action == KeyEvent.ACTION_DOWN) {
                    Log.d(TAG, "AVRCP Trigger Handoff");
                    startSHO(device, true);
                } else {
                    Log.d(TAG, "release for play PT from inactive device");
                }
            } else {
                Log.d(TAG, "Ignore passthrough from inactive device");
                return;
            }
        }

        Log.d(TAG, "ignore_play: " + ignore_play);
        if (ignore_play && code == KeyEvent.KEYCODE_MEDIA_PLAY) {
           if (action == KeyEvent.ACTION_UP) {
               ignore_play = false;
               Log.d(TAG, "ignore_play: " + ignore_play + " since play released is received");
           }
           return;
        }

        if (ignore_play) {
            ignore_play = false;
            Log.d(TAG, "ignore_play: " + ignore_play + " since another PT came before play release");
        }

        if (deviceFeatures[deviceIndex].cache_play_cmd) {
            Log.d(TAG,"cached play cmd exist, process it when a2dp device become active");
            return;
        }

        if (DEBUG) Log.d(TAG, "Avrcp current play state: " +
            convertPlayStateToPlayStatus(mCurrentPlayerState) +
            " isMusicActive: " + mAudioManager.isMusicActive() + " A2dp state: "  + mA2dpState +
            " Cached passthrough command: " + deviceFeatures[deviceIndex].mLastPassthroughcmd);
        if ((deviceFeatures[deviceIndex].mLastPassthroughcmd == KeyEvent.KEYCODE_UNKNOWN) ||
                    deviceFeatures[deviceIndex].mLastPassthroughcmd == code) {
            if (isPlayingState(mCurrentPlayerState) &&
                     (code == KeyEvent.KEYCODE_MEDIA_PLAY)) {
                 Log.w(TAG, "Ignoring passthrough command play" + op + " state " + state +
                         "in music playing");
                 return;
            }
            if ((!isPlayingState(mCurrentPlayerState)) &&
                    (!mAudioManager.isMusicActive()) &&
                    (mA2dpState == BluetoothA2dp.STATE_NOT_PLAYING) &&
                    (code == KeyEvent.KEYCODE_MEDIA_PAUSE)) {
                Log.w(TAG, "Ignoring passthrough command pause" + op + " state " + state +
                        "in music playing");
                return;
            }
        }

        BATService mBatService = BATService.getBATService();
        if ((mBatService != null) && mBatService.isBATActive()) {
            Log.d(TAG," BA-Active PassThrough cmd " + code + " isPlayerPlaying = "
                                                 +isPlayingState(mCurrentPlayerState));
            switch (code) {
                case KeyEvent.KEYCODE_MEDIA_PLAY:
                   if (isPlayingState(mCurrentPlayerState)) {
                        // we are not going to make a fix for device which depend on A2DP State to send
                        // AVRCP Command
                        return;
                    }
                    break;
                case KeyEvent.KEYCODE_MEDIA_PAUSE:
                    if (isPausedState(mCurrentPlayerState)) {
                        return;
                    }
                    break;
            }
        }

        KeyEvent event = new KeyEvent(action, code);

        if (!KeyEvent.isMediaSessionKey(code)) {
            Log.w(TAG, "Passthrough non-media key " + op + " (code " + code + ") state " + state);
        } else {
            if (code == KeyEvent.KEYCODE_MEDIA_FAST_FORWARD) {
                if ((state == deviceFeatures[deviceIndex].keyPressState) &&
                        (state == AvrcpConstants_ext.KEY_STATE_RELEASE)) {
                    Log.e(TAG, "Ignore fast forward key release event");
                    return;
                }
                if (action == KeyEvent.ACTION_DOWN) {
                    mFastforward = true;
                } else if (action == KeyEvent.ACTION_UP) {
                    mFastforward = false;
                }
                deviceFeatures[deviceIndex].keyPressState = state;
            } else if (code == KeyEvent.KEYCODE_MEDIA_REWIND) {
                if ((state == deviceFeatures[deviceIndex].keyPressState) &&
                        (state == AvrcpConstants_ext.KEY_STATE_RELEASE)) {
                    Log.e(TAG, "Ignore rewind key release event");
                    return;
                }
                if (action == KeyEvent.ACTION_DOWN) {
                    mRewind = true;
                } else if (action == KeyEvent.ACTION_UP) {
                    mRewind = false;
                }
                deviceFeatures[deviceIndex].keyPressState = state;
            } else {
                mFastforward = false;
                mRewind = false;
            }
        }

        /* IOT Fix as some remote recognise FF/Rewind state as non-playing hence send
         * changed response at the time of Release of Fast-Forward/Rewind Button */
        if ((code == KeyEvent.KEYCODE_MEDIA_FAST_FORWARD || code == KeyEvent.KEYCODE_MEDIA_REWIND)
                && (deviceFeatures[deviceIndex].mPlayStatusChangedNT ==
                AvrcpConstants_ext.NOTIFICATION_TYPE_INTERIM) && (action == KeyEvent.ACTION_UP)) {
            int currentPlayState =
                    convertPlayStateToPlayStatus(deviceFeatures[deviceIndex].mCurrentPlayState);
            Log.d(TAG, " currentPlayState: " + currentPlayState + " mLastRspPlayStatus: " +
                          deviceFeatures[deviceIndex].mLastRspPlayStatus);
            if (deviceFeatures[deviceIndex].mCurrentDevice != null &&
                    deviceFeatures[deviceIndex].mLastRspPlayStatus != currentPlayState) {
                deviceFeatures[deviceIndex].mPlayStatusChangedNT =
                                    AvrcpConstants_ext.NOTIFICATION_TYPE_CHANGED;
                registerNotificationRspPlayStatusNative(deviceFeatures[deviceIndex].mPlayStatusChangedNT
                       ,currentPlayState,
                        getByteAddress(deviceFeatures[deviceIndex].mCurrentDevice));
                deviceFeatures[deviceIndex].mLastRspPlayStatus = currentPlayState;
                Log.d(TAG, "Sending playback status CHANGED rsp on FF/Rewind key release");
            }
        }

        Log.d(TAG, "cached passthrough: " + deviceFeatures[deviceIndex].mLastPassthroughcmd +
              "current passthrough: " + code);
        if ((code == KeyEvent.KEYCODE_MEDIA_PAUSE) || (code == KeyEvent.KEYCODE_MEDIA_PLAY))
            deviceFeatures[deviceIndex].mLastPassthroughcmd = code;

        mMediaSessionManager.dispatchMediaKeyEvent(event);
        addKeyPending(event);
    }

    private int avrcpPassthroughToKeyCode(int operation) {
        switch (operation) {
            case BluetoothAvrcp.PASSTHROUGH_ID_UP:
                return KeyEvent.KEYCODE_DPAD_UP;
            case BluetoothAvrcp.PASSTHROUGH_ID_DOWN:
                return KeyEvent.KEYCODE_DPAD_DOWN;
            case BluetoothAvrcp.PASSTHROUGH_ID_LEFT:
                return KeyEvent.KEYCODE_DPAD_LEFT;
            case BluetoothAvrcp.PASSTHROUGH_ID_RIGHT:
                return KeyEvent.KEYCODE_DPAD_RIGHT;
            case BluetoothAvrcp.PASSTHROUGH_ID_RIGHT_UP:
                return KeyEvent.KEYCODE_DPAD_UP_RIGHT;
            case BluetoothAvrcp.PASSTHROUGH_ID_RIGHT_DOWN:
                return KeyEvent.KEYCODE_DPAD_DOWN_RIGHT;
            case BluetoothAvrcp.PASSTHROUGH_ID_LEFT_UP:
                return KeyEvent.KEYCODE_DPAD_UP_LEFT;
            case BluetoothAvrcp.PASSTHROUGH_ID_LEFT_DOWN:
                return KeyEvent.KEYCODE_DPAD_DOWN_LEFT;
            case BluetoothAvrcp.PASSTHROUGH_ID_0:
                return KeyEvent.KEYCODE_NUMPAD_0;
            case BluetoothAvrcp.PASSTHROUGH_ID_1:
                return KeyEvent.KEYCODE_NUMPAD_1;
            case BluetoothAvrcp.PASSTHROUGH_ID_2:
                return KeyEvent.KEYCODE_NUMPAD_2;
            case BluetoothAvrcp.PASSTHROUGH_ID_3:
                return KeyEvent.KEYCODE_NUMPAD_3;
            case BluetoothAvrcp.PASSTHROUGH_ID_4:
                return KeyEvent.KEYCODE_NUMPAD_4;
            case BluetoothAvrcp.PASSTHROUGH_ID_5:
                return KeyEvent.KEYCODE_NUMPAD_5;
            case BluetoothAvrcp.PASSTHROUGH_ID_6:
                return KeyEvent.KEYCODE_NUMPAD_6;
            case BluetoothAvrcp.PASSTHROUGH_ID_7:
                return KeyEvent.KEYCODE_NUMPAD_7;
            case BluetoothAvrcp.PASSTHROUGH_ID_8:
                return KeyEvent.KEYCODE_NUMPAD_8;
            case BluetoothAvrcp.PASSTHROUGH_ID_9:
                return KeyEvent.KEYCODE_NUMPAD_9;
            case BluetoothAvrcp.PASSTHROUGH_ID_DOT:
                return KeyEvent.KEYCODE_NUMPAD_DOT;
            case BluetoothAvrcp.PASSTHROUGH_ID_ENTER:
                return KeyEvent.KEYCODE_NUMPAD_ENTER;
            case BluetoothAvrcp.PASSTHROUGH_ID_CLEAR:
                return KeyEvent.KEYCODE_CLEAR;
            case BluetoothAvrcp.PASSTHROUGH_ID_CHAN_UP:
                return KeyEvent.KEYCODE_CHANNEL_UP;
            case BluetoothAvrcp.PASSTHROUGH_ID_CHAN_DOWN:
                return KeyEvent.KEYCODE_CHANNEL_DOWN;
            case BluetoothAvrcp.PASSTHROUGH_ID_PREV_CHAN:
                return KeyEvent.KEYCODE_LAST_CHANNEL;
            case BluetoothAvrcp.PASSTHROUGH_ID_INPUT_SEL:
                return KeyEvent.KEYCODE_TV_INPUT;
            case BluetoothAvrcp.PASSTHROUGH_ID_DISP_INFO:
                return KeyEvent.KEYCODE_INFO;
            case BluetoothAvrcp.PASSTHROUGH_ID_HELP:
                return KeyEvent.KEYCODE_HELP;
            case BluetoothAvrcp.PASSTHROUGH_ID_PAGE_UP:
                return KeyEvent.KEYCODE_PAGE_UP;
            case BluetoothAvrcp.PASSTHROUGH_ID_PAGE_DOWN:
                return KeyEvent.KEYCODE_PAGE_DOWN;
            case BluetoothAvrcp.PASSTHROUGH_ID_POWER:
                return KeyEvent.KEYCODE_POWER;
            case BluetoothAvrcp.PASSTHROUGH_ID_VOL_UP:
                return KeyEvent.KEYCODE_VOLUME_UP;
            case BluetoothAvrcp.PASSTHROUGH_ID_VOL_DOWN:
                return KeyEvent.KEYCODE_VOLUME_DOWN;
            case BluetoothAvrcp.PASSTHROUGH_ID_MUTE:
                return KeyEvent.KEYCODE_MUTE;
            case BluetoothAvrcp.PASSTHROUGH_ID_PLAY:
                return KeyEvent.KEYCODE_MEDIA_PLAY;
            case BluetoothAvrcp.PASSTHROUGH_ID_STOP:
                return KeyEvent.KEYCODE_MEDIA_STOP;
            case BluetoothAvrcp.PASSTHROUGH_ID_PAUSE:
                return KeyEvent.KEYCODE_MEDIA_PAUSE;
            case BluetoothAvrcp.PASSTHROUGH_ID_RECORD:
                return KeyEvent.KEYCODE_MEDIA_RECORD;
            case BluetoothAvrcp.PASSTHROUGH_ID_REWIND:
                return KeyEvent.KEYCODE_MEDIA_REWIND;
            case BluetoothAvrcp.PASSTHROUGH_ID_FAST_FOR:
                return KeyEvent.KEYCODE_MEDIA_FAST_FORWARD;
            case BluetoothAvrcp.PASSTHROUGH_ID_EJECT:
                return KeyEvent.KEYCODE_MEDIA_EJECT;
            case BluetoothAvrcp.PASSTHROUGH_ID_FORWARD:
                return KeyEvent.KEYCODE_MEDIA_NEXT;
            case BluetoothAvrcp.PASSTHROUGH_ID_BACKWARD:
                return KeyEvent.KEYCODE_MEDIA_PREVIOUS;
            case BluetoothAvrcp.PASSTHROUGH_ID_F1:
                return KeyEvent.KEYCODE_F1;
            case BluetoothAvrcp.PASSTHROUGH_ID_F2:
                return KeyEvent.KEYCODE_F2;
            case BluetoothAvrcp.PASSTHROUGH_ID_F3:
                return KeyEvent.KEYCODE_F3;
            case BluetoothAvrcp.PASSTHROUGH_ID_F4:
                return KeyEvent.KEYCODE_F4;
            case BluetoothAvrcp.PASSTHROUGH_ID_F5:
                return KeyEvent.KEYCODE_F5;
            // Fallthrough for all unknown key mappings
            case BluetoothAvrcp.PASSTHROUGH_ID_SELECT:
            case BluetoothAvrcp.PASSTHROUGH_ID_ROOT_MENU:
            case BluetoothAvrcp.PASSTHROUGH_ID_SETUP_MENU:
            case BluetoothAvrcp.PASSTHROUGH_ID_CONT_MENU:
            case BluetoothAvrcp.PASSTHROUGH_ID_FAV_MENU:
            case BluetoothAvrcp.PASSTHROUGH_ID_EXIT:
            case BluetoothAvrcp.PASSTHROUGH_ID_SOUND_SEL:
            case BluetoothAvrcp.PASSTHROUGH_ID_ANGLE:
            case BluetoothAvrcp.PASSTHROUGH_ID_SUBPICT:
            case BluetoothAvrcp.PASSTHROUGH_ID_VENDOR:
            default:
                return KeyEvent.KEYCODE_UNKNOWN;
        }
    }

    private void addKeyPending(KeyEvent event) {
        mPassthroughPending.add(new MediaKeyLog(System.currentTimeMillis(), event));
    }

    private void recordKeyDispatched(KeyEvent event, String packageName) {
        long time = System.currentTimeMillis();
        Log.v(TAG, "recordKeyDispatched: " + event + " dispatched to " + packageName);
        setAddressedMediaSessionPackage(packageName);
        mCachedBrowsePlayer = packageName;
        Log.w(TAG,"Trigger setAddressedMediaSessionPkg recordKeyDispatch" + mCachedBrowsePlayer);
        synchronized (mPassthroughPending) {
            Iterator<MediaKeyLog> pending = mPassthroughPending.iterator();
            while (pending.hasNext()) {
                MediaKeyLog log = pending.next();
                if (log.addDispatch(time, event, packageName)) {
                    mPassthroughDispatched++;
                    mPassthroughLogs.add(log);
                    pending.remove();
                    return;
                }
            }
            Log.w(TAG, "recordKeyDispatch: can't find matching log!");
        }
    }

    private final MediaSessionManager.Callback mButtonDispatchCallback =
            new MediaSessionManager.Callback() {
                @Override
                public void onMediaKeyEventDispatched(KeyEvent event, MediaSession.Token token) {
                    if (token != null) {
                        // Get the package name
                        android.media.session.MediaController controller =
                                new android.media.session.MediaController(mContext, token);
                        String targetPackage = controller.getPackageName();
                        recordKeyDispatched(event, targetPackage);
                    }
                }

                @Override
                public void onMediaKeyEventDispatched(KeyEvent event, ComponentName receiver) {
                    recordKeyDispatched(event, receiver.getPackageName());
                }

                @Override
                public void onAddressedPlayerChanged(MediaSession.Token token) {
                    setActiveMediaSession(token);
                }

                @Override
                public void onAddressedPlayerChanged(ComponentName receiver) {
                    if (receiver == null) {
                        // No active sessions, and no session to revive, give up.
                        setAddressedMediaSessionPackage(null);
                        return;
                    }
                    // We can still get a passthrough which will revive this player.
                    setAddressedMediaSessionPackage(receiver.getPackageName());
                    mCachedBrowsePlayer = receiver.getPackageName();
                    Log.w(TAG,"Trigger setAddressedMediaSessionPackage from onAddrPlayerChanged" +
                            mCachedBrowsePlayer);
                }
            };

    // Do not modify without updating the HAL bt_rc.h files.

    // match up with btrc_play_status_t enum of bt_rc.h
    final static byte PLAYSTATUS_STOPPED = 0;
    final static byte PLAYSTATUS_PLAYING = 1;
    final static byte PLAYSTATUS_PAUSED = 2;
    final static byte PLAYSTATUS_FWD_SEEK = 3;
    final static byte PLAYSTATUS_REV_SEEK = 4;
    final static byte PLAYSTATUS_ERROR = (byte) 255;

    // match up with btrc_media_attr_t enum of bt_rc.h
    final static int MEDIA_ATTR_TITLE = 1;
    final static int MEDIA_ATTR_ARTIST = 2;
    final static int MEDIA_ATTR_ALBUM = 3;
    final static int MEDIA_ATTR_TRACK_NUM = 4;
    final static int MEDIA_ATTR_NUM_TRACKS = 5;
    final static int MEDIA_ATTR_GENRE = 6;
    final static int MEDIA_ATTR_PLAYING_TIME = 7;

    // match up with btrc_event_id_t enum of bt_rc.h
    final static int EVT_PLAY_STATUS_CHANGED = 1;
    final static int EVT_TRACK_CHANGED = 2;
    final static int EVT_TRACK_REACHED_END = 3;
    final static int EVT_TRACK_REACHED_START = 4;
    final static int EVT_PLAY_POS_CHANGED = 5;
    final static int EVT_BATT_STATUS_CHANGED = 6;
    final static int EVT_SYSTEM_STATUS_CHANGED = 7;
    final static int EVT_APP_SETTINGS_CHANGED = 8;
    final static int EVENT_NOW_PLAYING_CONTENT_CHANGED = 9;
    final static int EVT_AVBL_PLAYERS_CHANGED = 0xa;
    final static int EVT_ADDR_PLAYER_CHANGED = 0xb;
    final static int EVENT_UIDS_CHANGED = 0x0c;

    private native static void classInitNative();
    private native void initNative(int maxConnections);
    private native void cleanupNative();
    private native boolean getPlayStatusRspNative(byte[] address, int playStatus, int songLen, int position);
    private native boolean updatePlayStatusToStack(int state);
    private native boolean getElementAttrRspNative(byte[] address, byte numAttr, int[] attrIds,
            String[] textArray);
    private native boolean registerNotificationRspPlayStatusNative(int type, int
            playStatus, byte[] address);
    private native boolean registerNotificationRspTrackChangeNative(int type, byte[]
            track, byte[] address);
    private native boolean registerNotificationRspPlayPosNative(int type, int
            playPos, byte[] address);
    private native boolean setVolumeNative(int volume, byte[] address);
    private native boolean sendPassThroughCommandNative(int keyCode, int keyState,
            byte[] address);
    private native boolean setAddressedPlayerRspNative(byte[] address, int rspStatus);
    private native boolean setBrowsedPlayerRspNative(byte[] address, int rspStatus, byte depth,
            int numItems, String[] textArray);
    private native boolean mediaPlayerListRspNative(byte[] address, int rsStatus, int uidCounter,
            byte item_type, int numItems, int[] playerIds, byte[] playerTypes, int[] playerSubTypes,
            byte[] playStatusValues, short[] featureBitMaskValues, String[] textArray);
    private native boolean getFolderItemsRspNative(byte[] address, int rspStatus, short uidCounter,
            byte scope, int numItems, byte[] folderTypes, byte[] playable, byte[] itemTypes,
            byte[] itemUidArray, String[] textArray, int[] AttributesNum, int[] AttributesIds,
            String[] attributesArray);
    private native boolean getListPlayerappAttrRspNative(byte attr,
            byte[] attrIds, byte[] address);
    private native boolean getPlayerAppValueRspNative(byte numberattr,
            byte[]values, byte[] address );
    private native boolean SendCurrentPlayerValueRspNative(byte numberattr,
            byte[]attr, byte[] address );
    private native boolean SendSetPlayerAppRspNative(int attr_status, byte[] address);
    private native boolean sendSettingsTextRspNative(int num_attr, byte[] attr,
        int length, String[]text, byte[] address);
    private native boolean sendValueTextRspNative(int num_attr, byte[] attr,
        int length, String[]text, byte[] address);
    private native boolean registerNotificationPlayerAppRspNative(int type,
        byte numberattr, byte[]attr, byte[] address);
    private native boolean changePathRspNative(byte[] address, int rspStatus, int numItems);
    private native boolean getItemAttrRspNative(byte[] address, int rspStatus, byte numAttr,
            int[] attrIds, String[] textArray);
    private native boolean playItemRspNative(byte[] address, int rspStatus);
    private native boolean getTotalNumOfItemsRspNative(byte[] address, int rspStatus,
            int uidCounter, int numItems);
    private native boolean isDeviceActiveInHandOffNative(byte[] address);
    private native boolean searchRspNative(byte[] address, int rspStatus, int uidCounter,
            int numItems);
    private native boolean addToNowPlayingRspNative(byte[] address, int rspStatus);
    private native boolean registerNotificationRspAddrPlayerChangedNative(int type,
            int playerId, int uidCounter, byte[] address);
    private native boolean registerNotificationRspAvalPlayerChangedNative(int type, byte[] address);
    private native boolean registerNotificationRspUIDsChangedNative(int type, int uidCounter,
            byte[] address);
    private native boolean registerNotificationRspNowPlayingChangedNative(int type,
            byte[] address);

    public static String getImgHandleFromTitle(String title) {
        if (DEBUG) Log.d(TAG, " getImgHandleFromTitle title:" + title + " return empty ");
        return "";
    }

    public static String getImgHandleFromTitle(byte[] bdaddr, String title) {
        String handle = null;
        if (DEBUG) Log.d(TAG, " getImgHandleFromTitle bdaddr:" + bdaddr + " title:" + title);
        if (mAvrcpBipRsp != null && title != null) {
            handle = mAvrcpBipRsp.getImgHandleFromTitle(bdaddr, title);
            return (handle != null && !handle.isEmpty()) ? handle:null;
        }
        return handle;
    }
}
