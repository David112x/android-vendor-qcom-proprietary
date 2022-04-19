/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva.service;

import android.app.Service;
import android.content.Context;
import android.content.Intent;
import android.hardware.soundtrigger.SoundTrigger;
import android.media.soundtrigger.SoundTriggerManager;
import android.os.AsyncTask;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.Messenger;
import android.os.PowerManager;
import android.os.RemoteException;

import com.qualcomm.listen.ListenTypes;
import com.qualcomm.qti.sva.R;
import com.qualcomm.qti.sva.SvaNotifications;
import com.qualcomm.qti.sva.controller.DetectionEventContainer;
import com.qualcomm.qti.sva.controller.ExtendedSmMgr;
import com.qualcomm.qti.sva.controller.Global;
import com.qualcomm.qti.sva.controller.Msg;
import com.qualcomm.qti.sva.controller.MsgResponse;
import com.qualcomm.qti.sva.controller.TonePlayer;
import com.qualcomm.qti.sva.data.IExtendedSmModel;
import com.qualcomm.qti.sva.data.IKeyphraseModel;
import com.qualcomm.qti.sva.data.ISettingsModel;
import com.qualcomm.qti.sva.data.SettingsModel;
import com.qualcomm.qti.sva.session.IWakeupSession;
import com.qualcomm.qti.sva.session.legacy.IWakeupListener;
import com.qualcomm.qti.sva.session.legacy.LegacyWakeupSession;
import com.qualcomm.qti.sva.utils.FileUtils;
import com.qualcomm.qti.sva.utils.LogUtils;
import com.qualcomm.qti.sva.wrapper.WindowManagerGlobalWrapper;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.Map;
import java.util.Timer;
import java.util.TimerTask;


public class WakeupService extends Service implements IWakeupListener {
    private final static int DISPLAY_ON_DURATION = 5000;
    private final static int SOUND_TRIGGER_NOT_INIT = -1;

    private final String TAG = WakeupService.class.getSimpleName();

    private Map<Msg.ClientID, Messenger> mClients = new HashMap<>();
    private Messenger mServerMessenger;
    private IWakeupSession mWakeupSession;
    private ExtendedSmMgr mExtendedSmMgr;
    private PowerManager.WakeLock mWakeLock;
    private Timer mWakelockTimer;
    private LookAheadBufferTask mLABTask;
    private RestartRecognitionTask mRestartRecognitionTask;
    private TonePlayer mTonePlayer;

    private static int mSoundTriggerState = SOUND_TRIGGER_NOT_INIT;

    private Handler mHandler = new Handler(new Handler.Callback() {
        Messenger clientMessenger;

        @Override
        public boolean handleMessage(Message msg) {
            clientMessenger = msg.replyTo;
            switch (msg.what) {
                case Msg.MSG_REGISTER_CLIENT_REQ:
                    handleRegisterClientReq(clientMessenger, (Msg.ClientID) msg.obj);
                    break;
                case Msg.MSG_DE_REGISTER_CLIENT_REQ:
                    handleDeregisterClientReq(clientMessenger, (Msg.ClientID) msg.obj);
                    break;
                case Msg.MSG_ESTABLISH_SESSION_REQ:
                    handleEstablishSessionReq(clientMessenger, (String) msg.obj);
                    break;
                case Msg.MSG_TERMINATE_SESSION_REQ:
                    handleTerminateSessionReq(clientMessenger, (String) msg.obj);
                    break;
                case Msg.MSG_LOAD_SOUND_MODEL_REQ:
                    handleLoadSoundModelReq(clientMessenger, (String) msg.obj);
                    break;
                case Msg.MSG_START_RECOGNITION_REQ:
                    handleStartRecognitionReq(clientMessenger, (String) msg.obj);
                    break;
                case Msg.MSG_STOP_RECOGNITION_REQ:
                    handleStopRecognitionReq(clientMessenger, (String) msg.obj);
                    break;
                case Msg.MSG_UNLOAD_SOUND_MODEL_REQ:
                    handleUnloadSoundModelReq(clientMessenger, (String) msg.obj);
                    break;
                case Msg.MSG_RELEASE_ALL_SESSIONS_REQ:
                    handleReleaseAllSessionsReq(clientMessenger);
                    break;
                default:
                    break;
            }
            return true;
        }
    });

    private void handleReleaseAllSessionsReq(Messenger clientMessenger) {
        LogUtils.d(TAG, "handleReleaseAllSessionsReq: clientMessenger = "
                + clientMessenger);
        if (null == clientMessenger) {
            LogUtils.d(TAG, "handleReleaseAllSessionsReq: invalid params");
            return;
        }

        mWakeupSession.releaseAllSvaSessions();
        LogUtils.d(TAG, "handleReleaseAllSessionsReq: releaseAllSvaSessions is called");
        sendResponse(clientMessenger, Msg.MSG_RELEASE_ALL_SESSIONS_RSP, null);

        // do not display SVA notification icon
        SvaNotifications.cancelNotification(getApplicationContext());
    }

    private void handleUnloadSoundModelReq(Messenger clientMessenger, String smFullName) {
        LogUtils.d(TAG, "handleUnloadSoundModelReq: smFullName = " + smFullName);
        if (null == clientMessenger || null == smFullName) {
            LogUtils.d(TAG, "handleUnloadSoundModelReq: invalid params");
            sendResponseToAllClients(Msg.MSG_UNLOAD_SOUND_MODEL_RSP, MsgResponse.Result.FAILURE);
            return;
        }

        int results = mWakeupSession.unloadSM(smFullName);
        LogUtils.d(TAG, "handleUnloadSoundModelReq: results = " + results);
        if (IWakeupSession.RC_OK == results) {
            sendResponseToAllClients(Msg.MSG_UNLOAD_SOUND_MODEL_RSP, MsgResponse.Result.SUCCESS);
        } else {
            sendResponseToAllClients(Msg.MSG_UNLOAD_SOUND_MODEL_RSP, MsgResponse.Result.FAILURE);
        }
    }

    private void handleStopRecognitionReq(Messenger clientMessenger, String smFullName) {
        LogUtils.d(TAG, "handleStopRecognitionReq: smFullName = " + smFullName);
        if (null == clientMessenger || null == smFullName) {
            LogUtils.d(TAG, "handleStopRecognitionReq: invalid params");
            sendResponseToAllClients(Msg.MSG_STOP_RECOGNITION_RSP, MsgResponse.Result.FAILURE);
            return;
        }

        int results = mWakeupSession.stopRecognition(smFullName);
        LogUtils.d(TAG, "handleStopRecognitionReq: results = " + results);
        if (IWakeupSession.RC_OK == results) {
            sendResponseToAllClients(Msg.MSG_STOP_RECOGNITION_RSP, MsgResponse.Result.SUCCESS);

            // if not active session now, stop SVA notification icon
            if (!mExtendedSmMgr.hasActiveSessions()) {
                SvaNotifications.cancelNotification(getApplicationContext());
            }
        } else {
            sendResponseToAllClients(Msg.MSG_STOP_RECOGNITION_RSP, MsgResponse.Result.FAILURE);
        }
    }

    private void handleStartRecognitionReq(Messenger clientMessenger, String smFullName) {
        LogUtils.d(TAG, "handleStartRecognitionReq: smFullName = " + smFullName);
        if (null == clientMessenger || null == smFullName) {
            LogUtils.d(TAG, "handleStartRecognitionReq: invalid params");
            sendResponseToAllClients(Msg.MSG_START_RECOGNITION_RSP, MsgResponse.Result.FAILURE);
            return;
        }

        int results = mWakeupSession.startRecognition(smFullName);
        LogUtils.d(TAG, "handleStartRecognitionReq: results = " + results);
        if (IWakeupSession.RC_OK == results) {
            sendResponseToAllClients(Msg.MSG_START_RECOGNITION_RSP, MsgResponse.Result.SUCCESS);

            // display notification in status bar
            SvaNotifications.notifySVAActive(getApplicationContext());
        } else {
            sendResponseToAllClients(Msg.MSG_START_RECOGNITION_RSP, MsgResponse.Result.FAILURE);
        }
    }

    private void handleLoadSoundModelReq(Messenger clientMessenger, String smFullName) {
        LogUtils.d(TAG, "handleLoadSoundModelReq: smFullName = " + smFullName);
        if (null == clientMessenger || null == smFullName) {
            LogUtils.d(TAG, "handleLoadSoundModelReq: invalid params");
            sendResponseToAllClients(Msg.MSG_LOAD_SOUND_MODEL_RSP, MsgResponse.Result.FAILURE);
            return;
        }

        int results = mWakeupSession.loadSM(smFullName);
        LogUtils.d(TAG, "handleLoadSoundModelReq: results = " + results);
        if (IWakeupSession.RC_OK == results) {
            sendResponseToAllClients(Msg.MSG_LOAD_SOUND_MODEL_RSP, MsgResponse.Result.SUCCESS);
        } else {
            sendResponseToAllClients(Msg.MSG_LOAD_SOUND_MODEL_RSP, MsgResponse.Result.FAILURE);
        }
    }

    private void handleTerminateSessionReq(Messenger clientMessenger, String smFullName) {
        LogUtils.d(TAG, "handleTerminateSessionReq: smFullName = " + smFullName);
        MsgResponse response;
        if (null == clientMessenger || null == smFullName) {
            LogUtils.d(TAG, "handleTerminateSessionReq: invalid params");
            response = new MsgResponse(MsgResponse.Result.FAILURE, smFullName,
                    IWakeupSession.RC_INVALID_PARAM);
            sendResponse(clientMessenger, Msg.MSG_TERMINATE_SESSION_RSP, response);
            return;
        }

        int results = mWakeupSession.terminateSvaSession(smFullName);
        LogUtils.d(TAG, "handleTerminateSessionReq: results = " + results);
        if (IWakeupSession.RC_OK == results) {
            response = new MsgResponse(MsgResponse.Result.SUCCESS, smFullName, results);
            sendResponse(clientMessenger, Msg.MSG_TERMINATE_SESSION_RSP, response);

            // if not active session now, stop SVA notification icon
            if (!mExtendedSmMgr.hasActiveSessions()) {
                SvaNotifications.cancelNotification(getApplicationContext());
            }
        } else {
            response = new MsgResponse(MsgResponse.Result.FAILURE, smFullName, results);
            sendResponse(clientMessenger, Msg.MSG_TERMINATE_SESSION_RSP, response);
        }
    }

    private void handleEstablishSessionReq(Messenger clientMessenger, String smFullName) {
        LogUtils.d(TAG, "handleEstablishSessionReq: smFullName = " + smFullName);
        MsgResponse response;
        if (null == clientMessenger || null == smFullName) {
            LogUtils.d(TAG, "handleEstablishSessionReq: invalid params");
            response = new MsgResponse(MsgResponse.Result.FAILURE, smFullName,
                    IWakeupSession.RC_INVALID_PARAM);
            sendResponse(clientMessenger, Msg.MSG_ESTABLISH_SESSION_RSP, response);
            return;
        }

        int results = mWakeupSession.establishSvaSession(smFullName);
        LogUtils.d(TAG, "handleEstablishSessionReq: results = " + results);
        if (IWakeupSession.RC_OK == results) {
            response = new MsgResponse(MsgResponse.Result.SUCCESS, smFullName, results);
            sendResponse(clientMessenger, Msg.MSG_ESTABLISH_SESSION_RSP, response);

            // display notification in status bar
            SvaNotifications.notifySVAActive(getApplicationContext());
        } else {
            response = new MsgResponse(MsgResponse.Result.FAILURE, smFullName, results);
            sendResponse(clientMessenger, Msg.MSG_ESTABLISH_SESSION_RSP, response);
        }
    }

    private void handleRegisterClientReq(Messenger clientMessenger, Msg.ClientID clientId) {
        LogUtils.d(TAG, "handleRegisterClientReq: clientId = " + clientId);
        if (mClients.containsKey(clientId)) {
            mClients.replace(clientId, clientMessenger);
        } else {
            mClients.put(clientId, clientMessenger);
        }

        sendResponse(clientMessenger, Msg.MSG_REGISTER_CLIENT_RSP, MsgResponse.Result.SUCCESS);
    }

    private void handleDeregisterClientReq(Messenger clientMessenger, Msg.ClientID clientId) {
        if (mClients.containsValue(clientMessenger) && mClients.containsKey(clientId)) {
            LogUtils.d(TAG, "handleDeregisterClientReq: clientId = " + clientId);
            mClients.remove(clientId, clientMessenger);
        }
    }

    private void startForeground(){
        startForeground(SvaNotifications.SVA_FOREGROUND_NOTIFICATION_ID,
                SvaNotifications.buildWakeupServiceNotification(this));
    }

    private void stopForeground(){
        stopForeground(true);
    }

    @Override
    public void onCreate() {
        mServerMessenger = new Messenger(mHandler);

        startForeground();

        // Create a wakeLock to turn the display on when detection occurs.
        PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
        if (null != pm) {
            mWakeLock = pm.newWakeLock(PowerManager.SCREEN_DIM_WAKE_LOCK |
                    PowerManager.ACQUIRE_CAUSES_WAKEUP, "svaLock");
            mWakeLock.setReferenceCounted(false);
        }

        // create directory first
        FileUtils.createDirIfNotExists(Global.PATH_ROOT);
        FileUtils.createDirIfNotExists(Global.PATH_TRAINING_RECORDINGS);
        FileUtils.createDirIfNotExists(Global.PATH_VOICE_REQUESTS);

        // copy sound model files to SdCard path
        Global.getInstance().getAssetFileMgr().copyAssetsIfNotExists(getApplicationContext(),
                Global.PATH_ROOT);

        // init sound model list according to sound model files
        mExtendedSmMgr = Global.getInstance().getExtendedSmMgr();
        mExtendedSmMgr.initSoundModels(getApplicationContext());

        Context appContext = getApplicationContext();
        LogUtils.d(TAG, "onCreate: based on legacy STModule API session impl");
        mWakeupSession = new LegacyWakeupSession(appContext);
        mWakeupSession.addWakeupListener(this);

        mTonePlayer = new TonePlayer(this);
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        LogUtils.d(TAG, "onStartCommand: intent = " + intent);
        return START_STICKY;
    }

    private void onReceivedRecognitionResumed(Intent intent) {
        LogUtils.d(TAG, "onReceivedRecognitionResumed: enter");
    }

    private void onReceivedRecognitionPaused(Intent intent) {
        LogUtils.d(TAG, "onReceivedRecognitionPaused: enter");

    }

    private void onReceivedRecognitionError(Intent intent) {
        int errorCode = intent.getIntExtra(SoundTriggerManager.EXTRA_STATUS,
                SoundTrigger.STATUS_ERROR);
        LogUtils.d(TAG, "onReceivedRecognitionError: errorCode = " + errorCode);
        if (SoundTrigger.STATUS_DEAD_OBJECT == errorCode) {
            LogUtils.d(TAG, "onReceivedRecognitionError");
            onServiceDied();
            return;
        } else {
            LogUtils.e(TAG, "onReceivedRecognitionError: other error, need process?");
        }
    }

    private void onReceivedRecognitionEvent(Intent intent) {
        String smName = intent.getStringExtra(IWakeupSession.KEY_EXTRA_SM_NAME);

        SoundTrigger.KeyphraseRecognitionEvent event = intent.getParcelableExtra(
                SoundTriggerManager.EXTRA_RECOGNITION_EVENT);
        LogUtils.d(TAG, "onReceivedRecognitionEvent: event = " + event
                + " smName = " + smName);

        // get smName by keyphraseId
        for (SoundTrigger.KeyphraseRecognitionExtra extraItem : event.keyphraseExtras) {
            int keyphraseId = extraItem.id;
            if (keyphraseId >= 0) {
                ExtendedSmMgr smMgr = Global.getInstance().getExtendedSmMgr();
                String temp = smMgr.getSmNameByKeyphraseId(keyphraseId);
                if (null != temp) {
                    smName = temp;
                }
                break;
            }
        }

        LogUtils.d(TAG, "onReceivedRecognitionEvent: finalized smName = " + smName);

        processRecognitionEvent(smName, event);
        if (!event.captureAvailable) {
            LogUtils.d(TAG, "onReceivedRecognitionEvent: no LAB, restart recognition");
            String[] params = new String[]{smName};
            mRestartRecognitionTask = new RestartRecognitionTask();
            mRestartRecognitionTask.execute(params);
        }
    }

    private void processRecognitionEvent(String smName,
                                         SoundTrigger.KeyphraseRecognitionEvent event) {
        LogUtils.d(TAG, "processRecognitionEvent: event = " + event
                + " smName = " + smName);

        // save smHandle
        IExtendedSmModel extendedSmModel = mExtendedSmMgr.getSoundModel(smName);
        extendedSmModel.setSoundModelHandle(event.soundModelHandle);

        // If recognitionEvent.status is ABORT, discard the recognition because capture preempted
        // by another audio use case.
        if (SoundTrigger.RECOGNITION_STATUS_ABORT == event.status) {
            LogUtils.d(TAG, "processRecognitionEvent: abort status, discard recognition");
            return;
        }

        ISettingsModel settingsModel = new SettingsModel(getApplicationContext(), smName);
        boolean bVoiceRequestEnabled = settingsModel.getVoiceRequestEnabled();
        if (SoundTrigger.RECOGNITION_STATUS_SUCCESS == event.status
                && event.captureAvailable
                && bVoiceRequestEnabled) {
            LogUtils.d(TAG, "processRecognitionEvent: LAB");

            // Filepath for voice requests is approximately /sdcard/sva/voiceRequests/<time>.wav.
            String time = String.valueOf(System.currentTimeMillis());
            String filePath = Global.PATH_VOICE_REQUESTS + "/" + time + Global.SUFFIX_WAV_FILE;
            String smHandle = String.valueOf(event.soundModelHandle);
            String captureHandle = String.valueOf(event.captureSession);
            LogUtils.d(TAG, "processRecognitionEvent: filePath = " + filePath);
            String[] params = new String[]{filePath, captureHandle, smHandle, smName};

            mLABTask = new LookAheadBufferTask();
            mLABTask.execute(params);
        }

        if (Global.getInstance().getIsToggleXmlParam()) {
            LogUtils.d(TAG, "processRecognitionEvent: toggle xml mode");
            // playback detection tone
            boolean bDetectionTone = settingsModel.getGlobalDetectionToneEnabled();
            if (bDetectionTone && !bVoiceRequestEnabled) {
                switch (event.status) {
                    case SoundTrigger.RECOGNITION_STATUS_SUCCESS:
                        if (null != mTonePlayer) {
                            mTonePlayer.play(0);
                        }
                        break;
                    case SoundTrigger.RECOGNITION_STATUS_FAILURE:
                        if (null != mTonePlayer) {
                            mTonePlayer.play(1);
                        }
                        break;
                    default:
                        break;
                }
            }

            turnOnDisplay();
            dismissKeyguard();
            return;
        }

        ListenTypes.VoiceWakeupDetectionDataV2 vwuDetectionData
                = new ListenTypes.VoiceWakeupDetectionDataV2();
        // Loop through KeyphraseRecognitionExtras to find out how many non-zero confLevels so
        // arrays can be initialized to the correct size.
        int keyphraseLevelCount = 0;
        int userLevelCount = 0;
        for (SoundTrigger.KeyphraseRecognitionExtra keyphraseExtraItem : event.keyphraseExtras) {
            if (keyphraseExtraItem.coarseConfidenceLevel >= 0) {
                keyphraseLevelCount++;
            }

            for (SoundTrigger.ConfidenceLevel userLevelItem : keyphraseExtraItem.confidenceLevels) {
                if (userLevelItem.confidenceLevel >= 0) {
                    userLevelCount++;
                }
            }
        }

        // value keyphrase levels and user levels
        vwuDetectionData.nonzeroKWConfLevels
                = new ListenTypes.VWUKeywordConfLevel[keyphraseLevelCount];
        vwuDetectionData.nonzeroUserKWPairConfLevels =
                new ListenTypes.VWUUserKeywordPairConfLevel[userLevelCount];
        int userLevelIndex = 0;
        int keyphraseLevelIndex = 0;
        for (SoundTrigger.KeyphraseRecognitionExtra extraItem : event.keyphraseExtras) {
            int keyphraseId = extraItem.id;
            String keyphraseName = extendedSmModel.getKeyphraseNameById(keyphraseId);
            if (keyphraseLevelCount > 0) {
                vwuDetectionData.nonzeroKWConfLevels[keyphraseLevelIndex]
                        = new ListenTypes.VWUKeywordConfLevel();
                vwuDetectionData.nonzeroKWConfLevels[keyphraseLevelIndex].keyword
                        = keyphraseName;
                vwuDetectionData.nonzeroKWConfLevels[keyphraseLevelIndex].confLevel
                        = (short) extraItem.coarseConfidenceLevel;
                keyphraseLevelIndex++;
            }

            for (SoundTrigger.ConfidenceLevel userLevelItem : extraItem.confidenceLevels) {
                int userId = userLevelItem.userId;
                IKeyphraseModel keyphraseModel = extendedSmModel.getKeyphraseModel(keyphraseName);
                String userName = null;
                if (null != keyphraseModel) {
                    userName = keyphraseModel.getUserNameById(userId);
                } else {
                    LogUtils.d(TAG, "processRecognitionEvent: keyphraseModel is null");
                }

                vwuDetectionData.nonzeroUserKWPairConfLevels[userLevelIndex]
                        = new ListenTypes.VWUUserKeywordPairConfLevel();
                vwuDetectionData.nonzeroUserKWPairConfLevels[userLevelIndex].keyword
                        = keyphraseName;
                vwuDetectionData.nonzeroUserKWPairConfLevels[userLevelIndex].confLevel
                        = (short) userLevelItem.confidenceLevel;
                vwuDetectionData.nonzeroUserKWPairConfLevels[userLevelIndex].user = userName;
                userLevelIndex++;
            }
        }

        // playback detection tone
        boolean bDetectionTone = settingsModel.getGlobalDetectionToneEnabled();
        if (bDetectionTone && !bVoiceRequestEnabled) {
            switch (event.status) {
                case SoundTrigger.RECOGNITION_STATUS_SUCCESS:
                    if (null != mTonePlayer) {
                        mTonePlayer.play(0);
                    }
                    break;
                case SoundTrigger.RECOGNITION_STATUS_FAILURE:
                    if (null != mTonePlayer) {
                        mTonePlayer.play(1);
                    }
                    break;
                default:
                    break;
            }
        }

        // send intent when action is non NONE
        if (!extendedSmModel.getSoundModelActionName().equals(getString(R.string.none))) {
            boolean isDebugMode = false;
            for (Map.Entry item : mClients.entrySet()) {
                if (Msg.ClientID.CLIENT_ID_DEBUG_MAIN_ACTIVITY == item.getKey()
                        || Msg.ClientID.CLIENT_ID_DEBUG_MODEL_LIST_ACTIVITY == item.getKey()) {
                    isDebugMode = true;
                    break;
                }
            }

            if (!isDebugMode) {
                // send intent only for non debug mode
                Intent actionIntent = extendedSmModel.getSoundModelActionIntent();
                if (null != actionIntent) {
                    LogUtils.d(TAG, "processRecognitionEvent: startActivity Intent = "
                            + actionIntent);
                    actionIntent.putExtra("LaunchedByWho", "SVA");
                    actionIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
                    startActivity(actionIntent);
                }
            }
        }

        // send MSG_DETECTION_IND event
        DetectionEventContainer eventContainer = new DetectionEventContainer(
                ListenTypes.STATUS_SUCCESS, null,
                vwuDetectionData, extendedSmModel.getSessionId(), smName);
        sendResponseToAllClients(Msg.MSG_DETECTION_IND, eventContainer);
        turnOnDisplay();
        dismissKeyguard();
    }

    @Override
    public void onRecognition(SoundTrigger.RecognitionEvent recognitionEvent) {
        LogUtils.d(TAG, "onRecognition: recognitionEvent = " + recognitionEvent);
        String smName = mExtendedSmMgr.getSmNameByHandle(recognitionEvent.soundModelHandle);
        if (null != smName) {
            processRecognitionEvent(smName,
                    (SoundTrigger.KeyphraseRecognitionEvent) recognitionEvent);
            if (!recognitionEvent.captureAvailable) {
                LogUtils.d(TAG, "onRecognition: no LAB, restart recognition");
                String[] params = new String[]{smName};
                mRestartRecognitionTask = new RestartRecognitionTask();
                mRestartRecognitionTask.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR, params);
            }
        }
    }

    @Override
    public void onServiceStatusChange(int state) {
        LogUtils.d(TAG, "onServiceStateChange: state= " + state);
        if (state == SoundTrigger.SERVICE_STATE_ENABLED
            && mSoundTriggerState == SoundTrigger.SERVICE_STATE_DISABLED) {
            //Restart all previously started SMs.
            ArrayList<IExtendedSmModel> models = mExtendedSmMgr.getAllSoundModelList();
            for (IExtendedSmModel item : models) {
                if (IExtendedSmModel.SessionStatus.STARTED == item.getSessionStatus()) {
                    String smName = item.getSoundModelFullFileName();
                    String[] params = new String[]{smName};
                    mRestartRecognitionTask = new RestartRecognitionTask();
                    mRestartRecognitionTask.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR, params);
                }
            }
        }
        mSoundTriggerState = state;
    }

    @Override
    public void onServiceDied() {
        LogUtils.d(TAG, "onServiceDied: enter");
        if (null != mWakeupSession) {
            mWakeupSession.releaseAllSvaSessions();
        }

        //stop Wakeup Service
        stopSelf();

        // notify all client to refresh UI
        sendResponseToAllClients(Msg.MSG_AUDIO_SERVER_DIED_IND, null);
    }

    @Override
    public IBinder onBind(Intent intent) {
        return mServerMessenger.getBinder();
    }

    @Override
    public void onDestroy() {
        LogUtils.v(TAG, "onDestroy: enter");
        if (mWakeLock != null && mWakeLock.isHeld()) {
            mWakeLock.release();
            mWakeLock = null;
        }

        if (null != mLABTask && !mLABTask.isCancelled()) {
            mLABTask.cancel(true);
            mLABTask = null;
        }

        if (null != mRestartRecognitionTask && !mRestartRecognitionTask.isCancelled()) {
            mRestartRecognitionTask.cancel(true);
            mRestartRecognitionTask = null;
        }

        if (null != mWakeupSession) {
            mWakeupSession.removeWakeupListener(this);
            mWakeupSession.releaseAllSvaSessions();
        }

        // cancel all SVA notification
        SvaNotifications.cancelNotification(getApplicationContext());

        if (null != mHandler) {
            mHandler.removeCallbacksAndMessages(null);
        }

        if (null != mTonePlayer) {
            mTonePlayer.release();
        }

        stopForeground();

        super.onDestroy();
    }

    private void sendResponse(Messenger clientMessenger, int what, int msgArg1, int msgArg2) {
        if (mClients.containsValue(clientMessenger)) {
            try {
                LogUtils.v(TAG, "sendResponse: clientMessenger = " + clientMessenger
                        + " what =  " + what + " msgArg1 = " + msgArg1
                        + " msgArg2 = " + msgArg2);
                Message msg = Message.obtain(null, what);
                msg.arg1 = msgArg1;
                msg.arg2 = msgArg2;
                clientMessenger.send(msg);
            } catch (RemoteException e) {
                LogUtils.v(TAG, "sendResponse: removing clientMessenger = "
                        + clientMessenger);
                e.printStackTrace();
                mClients.remove(clientMessenger);
            }
        } else {
            LogUtils.v(TAG, "sendResponse: invalid clientMessenger = " + clientMessenger);
        }
    }

    private void sendResponse(Messenger clientMessenger, int what, Object obj) {
        if (mClients.containsValue(clientMessenger)) {
            try {
                LogUtils.v(TAG, "sendResponse: clientMessenger = " + clientMessenger
                        + " what =  " + what + " obj = " + obj);
                Message msg = Message.obtain(null, what);
                msg.obj = obj;
                clientMessenger.send(msg);
            } catch (RemoteException e) {
                LogUtils.v(TAG, "sendResponse: removing clientMessenger = "
                        + clientMessenger);
                e.printStackTrace();
                mClients.remove(clientMessenger);
            }
        } else {
            LogUtils.v(TAG, "sendResponse: invalid clientMessenger = " + clientMessenger);
        }
    }

    private void sendResponseToAllClients(int what, int msgArg1, int msgArg2) {
        for (Msg.ClientID key : mClients.keySet()) {
            LogUtils.v(TAG, "sendResponseToAllClients: key = " + key
                    + " clientMessenger = " + mClients.get(key)
                    + " what = " + what + " msgArg1 = " + msgArg1 + " msgArg2 = " + msgArg2);
            sendResponse(mClients.get(key), what, msgArg1, msgArg2);
        }
    }

    private void sendResponseToAllClients(int what, Object obj) {
        for (Msg.ClientID key : mClients.keySet()) {
            LogUtils.v(TAG, "sendResponseToAllClients: key = " + key
                    + " clientMessenger = " + mClients.get(key)
                    + " what = " + what + " obj = " + obj);
            sendResponse(mClients.get(key), what, obj);
        }
    }

    private synchronized void turnOnDisplay() {
        LogUtils.d(TAG, "turnOnDisplay: enter");

        // If the wakeLock isn't held, that means no other detections
        // have happened in the last DISPLAY_ON_DURATION milliseconds.
        // It's safe to acquire the wakeLock.
        if (!mWakeLock.isHeld()) {
            mWakeLock.acquire();
        } else {
            PowerManager pm = (PowerManager) getSystemService(Context.POWER_SERVICE);
            if (null != pm && !pm.isInteractive()) {
                LogUtils.d(TAG, "turnOnDisplay: no interactive");
                mWakeLock.release();
                mWakeLock.acquire();
            }
        }

        // If no previous detection within 3 seconds, starts a new
        // timerTask and releases the wakeLock after 3 seconds.
        // If a timerTask already exists, cancels that task and
        // creates a new one to release the wakeLock after 3 seconds.
        startWakelockTimer();
    }

    // Turns off the display after DISPLAY_ON_DURATION milliseconds if no user interaction.
    private void startWakelockTimer() {
        LogUtils.d(TAG, "startWakelockTimer: enter");
        // To stop existing wakelockTimers
        stopWakelockTimer();
        mWakelockTimer = new Timer();
        mWakelockTimer.schedule(new TimerTask() {
            @Override
            public void run() {
                LogUtils.d(TAG, "startWakelockTimer:run: release wakelock");
                // To stop this wakelockTimer from repeating
                stopWakelockTimer();
                try {
                    if (mWakeLock.isHeld()) {
                        mWakeLock.release();
                    }
                } catch (RuntimeException re) {
                    LogUtils.e(TAG, "startWakelockTimer:run: wakelock under-locked");
                }
            }
        }, DISPLAY_ON_DURATION);
    }

    private void stopWakelockTimer() {
        if (null != mWakelockTimer) {
            // Cancel all scheduled tasks for the timer.
            mWakelockTimer.cancel();
            // Remove all canceled tasks from the queue so they can be garbaged collected.
            int numPurgedTasks = mWakelockTimer.purge();
            //wakelockTimer = null;
            LogUtils.d(TAG, "stopWakelockTimer: numPurgedTasks= " + numPurgedTasks);
        }
    }

    private synchronized void dismissKeyguard() {
        WindowManagerGlobalWrapper windowManager = new WindowManagerGlobalWrapper();
        windowManager.dismissKeyguard();
    }

    public class LookAheadBufferTask extends AsyncTask<String, Void, Integer> {
        private final int SUCCESS = 0;
        private final int FAILURE = -1;
        private int mAssociateSmHandle = -1;
        private String mFilePath;
        private String mSmName;

        @Override
        protected Integer doInBackground(String... strings) {
            mFilePath = strings[0];
            LogUtils.d(TAG, "LookAheadBufferTask:doInBackground: mFilePath = "
                    + mFilePath);
            int captureHandle = Integer.parseInt(strings[1]);
            mAssociateSmHandle = Integer.parseInt(strings[2]);
            mSmName = strings[3];

            // create directory first
            FileUtils.createDirIfNotExists(Global.PATH_ROOT);
            FileUtils.createDirIfNotExists(Global.PATH_VOICE_REQUESTS);

            boolean bSuccess = Global.getInstance().getRecordingsMgr().startHotWordRecording(
                    getApplicationContext(), mSmName, captureHandle, mFilePath);

            return bSuccess ? SUCCESS : FAILURE;
        }

        @Override
        protected void onPostExecute(Integer integer) {
            LogUtils.d(TAG, "LookAheadBufferTask:onPostExecute: enter");
            super.onPostExecute(integer);
            if (SUCCESS == integer) {
                sendResponseToAllClients(Msg.MSG_VOICE_REQUEST_IND, mFilePath);
            }

            if (-1 != mAssociateSmHandle) {
                String[] params = new String[]{mSmName};
                mRestartRecognitionTask = new RestartRecognitionTask();
                mRestartRecognitionTask.executeOnExecutor(AsyncTask.THREAD_POOL_EXECUTOR, params);
            }
        }
    }

    /*
     * Restart recognition task
     */
    public class RestartRecognitionTask extends AsyncTask<String, Void, Integer> {

        @Override
        protected Integer doInBackground(String... strings) {
            String smName = strings[0];
            int result = IWakeupSession.RC_OK;
            LogUtils.d(TAG, "RestartRecognitionTask:doInBackground: smName = " + smName);
            result = mWakeupSession.restartRecognition(smName);

            // display notification in status bar
            if (mExtendedSmMgr.hasActiveSessions()) {
                SvaNotifications.notifySVAActive(getApplicationContext());
            }

            LogUtils.d(TAG, "RestartRecognitionTask:doInBackground: "
                    + "restartRecognition result = " + result);

            return result;
        }
    }
}
