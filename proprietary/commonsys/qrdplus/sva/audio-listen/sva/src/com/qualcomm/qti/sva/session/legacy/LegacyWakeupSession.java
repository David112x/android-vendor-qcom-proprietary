/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva.session.legacy;

import android.content.Context;
import android.hardware.soundtrigger.SoundTrigger;
import android.hardware.soundtrigger.SoundTriggerModule;

import com.qualcomm.qti.sva.controller.ExtendedSmMgr;
import com.qualcomm.qti.sva.controller.Global;
import com.qualcomm.qti.sva.data.IExtendedSmModel;
import com.qualcomm.qti.sva.data.IKeyphraseModel;
import com.qualcomm.qti.sva.data.ISettingsModel;
import com.qualcomm.qti.sva.data.ISmModel;
import com.qualcomm.qti.sva.data.IUserModel;
import com.qualcomm.qti.sva.data.SettingsModel;
import com.qualcomm.qti.sva.session.IWakeupSession;
import com.qualcomm.qti.sva.session.OpaqueDataHelper;
import com.qualcomm.qti.sva.utils.FileUtils;
import com.qualcomm.qti.sva.utils.LogUtils;

import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;
import org.xmlpull.v1.XmlPullParserFactory;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

public class LegacyWakeupSession implements IWakeupSession, SoundTrigger.StatusListener {
    private final String TAG = LegacyWakeupSession.class.getSimpleName();

    private SoundTriggerModule mSTModule = null;
    private boolean mSTServiceEnabled = false;
    private List<IWakeupListener> mWakeupListeners = new ArrayList<>();
    private Context mContext;
    private UUID mVendorUuid = null;
    private UUID mVendorUuidFromHal = null;
    private ExtendedSmMgr mExtendedSmMgr;

    private UUID mUUID = null;
    private String mKeyphraseFromXml = null;
    private byte[] mOpaqueDataFromXml = null;

    public LegacyWakeupSession(Context context) {
        mContext = context;
        if (!attachModule()) {
            LogUtils.e(TAG, "LegacyWakeupSession: attachModule failed");
        }
        mExtendedSmMgr = Global.getInstance().getExtendedSmMgr();
    }

    @Override
    public int establishSvaSession(String smName) {
        int returnCode;
        LogUtils.d(TAG, "establishSvaSession: smName = " + smName);
        if (null == smName) {
            LogUtils.d(TAG, "establishSvaSession: invalid input param");
            returnCode = RC_INVALID_PARAM;
            return returnCode;
        }

        returnCode = loadSM(smName);
        if (RC_OK != returnCode) {
            LogUtils.d(TAG, "establishSvaSession: loadSM returnCode = " + returnCode);
            return returnCode;
        }

        returnCode = startRecognition(smName);
        if (RC_OK != returnCode) {
            LogUtils.d(TAG, "establishSvaSession: startRecognition returnCode = "
                    + returnCode);
            unloadSM(smName);
            return returnCode;
        }

        return returnCode;
    }

    @Override
    public int terminateSvaSession(String smName) {
        int returnCode;
        LogUtils.d(TAG, "terminateSvaSession: smName = " + smName);
        if (null == smName) {
            LogUtils.d(TAG, "terminateSvaSession: invalid input param");
            returnCode = RC_INVALID_PARAM;
            return returnCode;
        }

        returnCode = stopRecognition(smName);
        if (RC_OK != returnCode) {
            LogUtils.d(TAG, "terminateSvaSession: stopRecognition returnCode = "
                    + returnCode);
            if (RC_WRONG_STATUS != returnCode) {
                return returnCode;
            }
        }

        returnCode = unloadSM(smName);
        if (RC_OK != returnCode) {
            LogUtils.d(TAG, "terminateSvaSession: unloadSM returnCode = "
                    + returnCode);
            return returnCode;
        }

        return returnCode;
    }

    @Override
    public void releaseAllSvaSessions() {
        ArrayList<IExtendedSmModel> allSoundModelListList
                = mExtendedSmMgr.getAllSoundModelList();
        int returnCode;
        for (IExtendedSmModel smModel : allSoundModelListList) {
            String smName = smModel.getSoundModelFullFileName();
            LogUtils.d(TAG, "releaseAllSvaSessions: smName = " + smName);
            switch (smModel.getSessionStatus()) {
                case UNLOADED:
                    LogUtils.d(TAG, "releaseAllSvaSessions: unloaded status, do nothing");
                    break;
                case STARTED:
                    returnCode = terminateSvaSession(smName);
                    LogUtils.d(TAG, "releaseAllSvaSessions: started status returnCode = "
                            + returnCode);
                    break;
                case LOADED:
                    returnCode = unloadSM(smName);
                    LogUtils.d(TAG, "releaseAllSvaSessions: loaded status returnCode = "
                            + returnCode);
                    break;
                case STOPPED:
                    returnCode = unloadSM(smName);
                    LogUtils.d(TAG, "releaseAllSvaSessions: stopped status returnCode = "
                            + returnCode);
                    break;
                default:
                    LogUtils.d(TAG, "releaseAllSvaSessions: invalid status value");
                    break;
            }

            // reset to initial status: UNLOADED
            smModel.setSessionStatus(IExtendedSmModel.SessionStatus.UNLOADED);
        }

        LogUtils.d(TAG, "releaseAllSvaSessions: detachModule is called");
        detachModule();
    }

    @Override
    public void addWakeupListener(IWakeupListener listener) {
        if (mWakeupListeners.contains(listener)) {
            LogUtils.d(TAG, "addWakeupListener: listener already exists");
        } else {
            LogUtils.d(TAG, "addWakeupListener: add listener " + listener);
            mWakeupListeners.add(listener);
        }
    }

    @Override
    public void removeWakeupListener(IWakeupListener listener) {
        if (mWakeupListeners.contains(listener)) {
            LogUtils.d(TAG, "removeWakeupListener: remove listener " + listener);
            mWakeupListeners.remove(listener);
        } else {
            LogUtils.d(TAG, "removeWakeupListener: listener not exist, do nothing");
        }
    }

    @Override
    public int restartRecognition(String smName) {
        int returnCode;
        LogUtils.d(TAG, "restartRecognition: smName = " + smName);
        if (null == smName) {
            LogUtils.d(TAG, "restartRecognition: invalid input param");
            returnCode = RC_INVALID_PARAM;
            return returnCode;
        }

        IExtendedSmModel extendedSmModel = mExtendedSmMgr.getSoundModel(smName);

        if (IExtendedSmModel.SessionStatus.STARTED != extendedSmModel.getSessionStatus()) {
            LogUtils.d(TAG, "restartRecognition: failure for wrong status");
            returnCode = RC_FAILURE;
            return returnCode;
        }

        // call STM API
        returnCode = mSTModule.startRecognition(extendedSmModel.getSoundModelHandle(),
                extendedSmModel.getRecognitionConfig());
        if (RC_OK == returnCode) {
            LogUtils.d(TAG, "restartRecognition: STM restartRecognition success");
            extendedSmModel.setSessionStatus(IExtendedSmModel.SessionStatus.STARTED);
        }

        return returnCode;
    }

    @Override
    public void onRecognition(SoundTrigger.RecognitionEvent recognitionEvent) {
        LogUtils.d(TAG, "onRecognition: recognitionEvent = " + recognitionEvent);
        for (IWakeupListener listener : mWakeupListeners) {
            listener.onRecognition(recognitionEvent);
        }
    }

    @Override
    public void onSoundModelUpdate(SoundTrigger.SoundModelEvent soundModelEvent) {
        LogUtils.d(TAG, "onSoundModelUpdate is recalled, do nothing");
    }

    @Override
    public void onServiceStateChange(int state) {
        LogUtils.d(TAG, "onServiceStateChange: state = " + state);
        setSTServiceEnabled((SoundTrigger.SERVICE_STATE_ENABLED == state));
        for (IWakeupListener listener : mWakeupListeners) {
            listener.onServiceStatusChange(state);
        }
    }

    @Override
    public void onServiceDied() {
        LogUtils.d(TAG, "onServiceDied: ST Service died");
        for (IWakeupListener listener : mWakeupListeners) {
            listener.onServiceDied();
        }
    }

    private boolean getSTServiceEnabled() {
        return mSTServiceEnabled;
    }

    private void setSTServiceEnabled(boolean enabled) {
        mSTServiceEnabled = enabled;
    }

    private boolean attachModule() {
        ArrayList<SoundTrigger.ModuleProperties> moduleProperties
                = new ArrayList<>();
        SoundTrigger.listModules(moduleProperties);
        if (moduleProperties.size() < 1) {
            LogUtils.e(TAG, "attachModule: no available modules");
            return false;
        }

        mVendorUuidFromHal = moduleProperties.get(0).uuid;

        LogUtils.d(TAG, "attachModule: moduleProperties.get(0) = "
                + moduleProperties.get(0));
        mSTModule = SoundTrigger.attachModule(moduleProperties.get(0).id, this, null);
        if (null == mSTModule) {
            LogUtils.e(TAG, "attachModule: low layer error");
            return false;
        }

        return true;
    }

    public int loadSM(String smName) {
        int returnCode;
        LogUtils.d(TAG, "loadSM: smName = " + smName);
        if (null == smName) {
            LogUtils.d(TAG, "loadSM: invalid input param");
            returnCode = RC_INVALID_PARAM;
            return returnCode;
        }

        String filePath = mExtendedSmMgr.getSoundModelFullPath(smName);
        IExtendedSmModel extendedSmModel = mExtendedSmMgr.getSoundModel(smName);
        if (null == extendedSmModel || !FileUtils.isExist(filePath)) {
            LogUtils.d(TAG, "loadSM: sm file not exist");
            returnCode = RC_FILE_NOT_EXIST;
            return returnCode;
        }

        if (null == mSTModule) {
            LogUtils.d(TAG, "loadSM: mSTModule is null, need retry attachModule");
            if (!attachModule()) {
                LogUtils.e(TAG, "loadSM: retry attachModule fail");
                returnCode = RC_FAILURE;
                return returnCode;
            }
        }

        // if the sm have been loaded, return success directly
        if (IExtendedSmModel.SessionStatus.LOADED == extendedSmModel.getSessionStatus()
                || IExtendedSmModel.SessionStatus.STOPPED == extendedSmModel.getSessionStatus()) {
            LogUtils.d(TAG, "loadSM: already loaded, return success");
            returnCode = RC_OK;
            return returnCode;
        }

        // call STM API
        SoundTrigger.KeyphraseSoundModel keyphraseSoundModel = createSTKeyphraseSoundModel(smName);
        if (null == keyphraseSoundModel) {
            LogUtils.d(TAG, "loadSM: create KeyphraseSoundModel fail");
            returnCode = RC_FAILURE;
            return returnCode;
        }
        int[] smHandleArray = new int[20];
        returnCode = mSTModule.loadSoundModel(keyphraseSoundModel, smHandleArray);
        if (RC_OK == returnCode) {
            LogUtils.d(TAG, "loadSM: STM loadSoundModel success");
            extendedSmModel.setSessionStatus(IExtendedSmModel.SessionStatus.LOADED);
            extendedSmModel.setSoundModelHandle(smHandleArray[0]);
        }

        return returnCode;
    }

    private SoundTrigger.KeyphraseSoundModel createSTKeyphraseSoundModel(String smName) {
        if (null == smName) {
            LogUtils.d(TAG, "createSTKeyphraseSoundModel: invalid input param");
            return null;
        }

        // determine recognition mode
        int recognitionMode = SoundTrigger.RECOGNITION_MODE_VOICE_TRIGGER;
        boolean isUserTrainedSM = mExtendedSmMgr.getSoundModel(smName).isUserTrainedSoundModel();
        ISettingsModel settingsModel = new SettingsModel(mContext, smName);
        boolean isUserVerificationEnabled = settingsModel.getUserVerificationEnabled();
        if (isUserTrainedSM && isUserVerificationEnabled) {
            recognitionMode += SoundTrigger.RECOGNITION_MODE_USER_IDENTIFICATION;
        }

        ByteBuffer smBuffer = null;
        try {
            smBuffer = FileUtils.readFileToByteBuffer(
                    mExtendedSmMgr.getSoundModelFullPath(smName));
        } catch (IOException e) {
            e.printStackTrace();
        }

        if (null == smBuffer) {
            LogUtils.d(TAG, "createSTKeyphraseSoundModel: read sm data fail");
            return null;
        }

        IExtendedSmModel extendedSmModel = mExtendedSmMgr.getSoundModel(smName);
        SoundTrigger.KeyphraseRecognitionExtra[] stKpRecognitionExtraArray;
        SoundTrigger.Keyphrase[] stKpArray;
        if (Global.getInstance().getIsToggleXmlParam()) {
            // get params from xml file
            createStkSmInfoFromXml(smName);
            if (null == mKeyphraseFromXml) {
                LogUtils.d(TAG, "createSTKeyphraseSoundModel: mKeyphraseFromXml is null");
                return null;
            }

            int[] userIdArray = new int[0];
            SoundTrigger.Keyphrase stKeyphrase = new SoundTrigger.Keyphrase(
                    0, recognitionMode, "en_US", mKeyphraseFromXml, userIdArray);
            stKpArray = new SoundTrigger.Keyphrase[1];
            stKpArray[0] = stKeyphrase;

            SoundTrigger.ConfidenceLevel[] confLevels = new SoundTrigger.ConfidenceLevel[0];
            SoundTrigger.KeyphraseRecognitionExtra extraItem =
                    new SoundTrigger.KeyphraseRecognitionExtra(0, 1,
                            69, confLevels);
            stKpRecognitionExtraArray = new SoundTrigger.KeyphraseRecognitionExtra[1];
            stKpRecognitionExtraArray[0] = extraItem;
        } else {
            // resume the mVendorUuid to QTI UUID
            mVendorUuid = mVendorUuidFromHal;
            int keyphraseCount = extendedSmModel.getSoundModelKeyphraseList().size();
            stKpArray = new SoundTrigger.Keyphrase[keyphraseCount];
            stKpRecognitionExtraArray = new SoundTrigger.KeyphraseRecognitionExtra[keyphraseCount];
            for (int j = 0; j < keyphraseCount; j++) {
                IKeyphraseModel keyphraseModel
                        = extendedSmModel.getSoundModelKeyphraseList().get(j);

                // generate user id array
                int userCount = keyphraseModel.getUserList().size();
                int[] userIdList = new int[userCount];
                int index = 0;
                for (IUserModel item : keyphraseModel.getUserList()) {
                    userIdList[index++] = item.getUserId();
                }

                // create SoundTrigger.Keyphrase object
                SoundTrigger.Keyphrase stKeyphrase
                        = new SoundTrigger.Keyphrase(keyphraseModel.getKeyphraseId(),
                        recognitionMode, "en_US", keyphraseModel.getKeyphraseFullName(),
                        userIdList);
                stKpArray[j] = stKeyphrase;

                // value user confidence levels
                ISmModel.MODEL_VERSION version = extendedSmModel.getSoundModelVersion();
                SoundTrigger.ConfidenceLevel[] stUserConfidenceLevelArray
                        = new SoundTrigger.ConfidenceLevel[userCount];
                for (int k = 0; k < userCount; k++) {
                    int userConfidenceLevel;
                    if (ISmModel.MODEL_VERSION.VERSION_3_0 == version
                        || ISmModel.MODEL_VERSION.VERSION_4_0 == version) {
                        userConfidenceLevel = settingsModel.get2ndUserConfidenceLevel();
                    } else {
                        userConfidenceLevel = settingsModel.get1stUserConfidenceLevel();
                    }

                    SoundTrigger.ConfidenceLevel stConfidenceLevel
                            = new SoundTrigger.ConfidenceLevel(stKeyphrase.users[k],
                            userConfidenceLevel);
                    stUserConfidenceLevelArray[k] = stConfidenceLevel;
                }

                //value keyphrase confidence level
                int keyphraseConfidenceLevel;
                if (ISmModel.MODEL_VERSION.VERSION_3_0 == version
                    || ISmModel.MODEL_VERSION.VERSION_4_0 == version) {
                    keyphraseConfidenceLevel = settingsModel.get2ndKeyphraseConfidenceLevel();
                } else {
                    keyphraseConfidenceLevel = settingsModel.get1stKeyphraseConfidenceLevel();
                }

                // create KeyphraseRecognitionExtra
                SoundTrigger.KeyphraseRecognitionExtra stKpRecognitionExtra
                        = new SoundTrigger.KeyphraseRecognitionExtra(
                        keyphraseModel.getKeyphraseId(), recognitionMode,
                        keyphraseConfidenceLevel, stUserConfidenceLevelArray);
                stKpRecognitionExtraArray[j] = stKpRecognitionExtra;
            }
        }

        // save stKpRecognitionExtraArray which is used for startRecognition param
        extendedSmModel.setKeyphraseRecognitionExtra(stKpRecognitionExtraArray);

        smBuffer.rewind();
        byte[] smByteArray = new byte[smBuffer.remaining()];
        smBuffer.get(smByteArray);

        SoundTrigger.KeyphraseSoundModel stKpSm = new SoundTrigger.KeyphraseSoundModel(
                extendedSmModel.getSoundModelUUID(), mVendorUuid, smByteArray, stKpArray);

        return stKpSm;
    }

    private boolean isStateTransitional(IExtendedSmModel.SessionStatus fromState,
                                        IExtendedSmModel.SessionStatus toState) {
        switch (toState) {
            case UNLOADED:
                if (IExtendedSmModel.SessionStatus.LOADED == fromState
                        || IExtendedSmModel.SessionStatus.STOPPED == fromState) {
                    return true;
                }
                break;
            case LOADED:
                if (IExtendedSmModel.SessionStatus.UNLOADED == fromState) {
                    return true;
                }
                break;
            case STARTED:
                if (IExtendedSmModel.SessionStatus.LOADED == fromState
                        || IExtendedSmModel.SessionStatus.STOPPED == fromState) {
                    return true;
                }
                break;
            case STOPPED:
                if (IExtendedSmModel.SessionStatus.STARTED == fromState) {
                    return true;
                }
                break;
            default:
                LogUtils.e("isStateTransitional: unrecognized state" + toState);
                break;
        }

        return false;
    }

    public int startRecognition(String smName) {
        int returnCode;
        LogUtils.d(TAG, "startRecognition: smName = " + smName);
        if (null == smName) {
            LogUtils.d(TAG, "startRecognition: invalid input param");
            returnCode = RC_INVALID_PARAM;
            return returnCode;
        }

        // judge the sm whether loaded or not
        IExtendedSmModel extendedSmModel = mExtendedSmMgr.getSoundModel(smName);
        if (IExtendedSmModel.SessionStatus.STARTED == extendedSmModel.getSessionStatus()) {
            LogUtils.d(TAG, "startRecognition: already started, return success directly");
            returnCode = RC_OK;
            return returnCode;
        }

        boolean bCanTransit = isStateTransitional(extendedSmModel.getSessionStatus(),
                IExtendedSmModel.SessionStatus.STARTED);
        if (!bCanTransit) {
            LogUtils.d(TAG, "startRecognition: failure for wrong status");
            returnCode = RC_FAILURE;
            return returnCode;
        }

        SoundTrigger.KeyphraseRecognitionExtra[] stKpRecognitionExtraArray
                = extendedSmModel.getKeyphraseRecognitionExtra();
        if (null == stKpRecognitionExtraArray) {
            LogUtils.d(TAG, "startRecognition: failure for stKpRecognitionExtraArray");
            returnCode = RC_FAILURE;
            return returnCode;
        }


        ISettingsModel settingsModel = new SettingsModel(mContext, smName);
        boolean bCaptureRequest = settingsModel.getVoiceRequestEnabled();
        boolean bAllowMultipleTriggers = false;

        // fill opaque data
        byte[] opaqueByteArray;
        OpaqueDataHelper opaqueDataHelper;
        boolean bOpaqueDataTransfer = settingsModel.getOpaqueDataTransferEnabled();
        int histBufferDuration = settingsModel.getHistBufferTime();
        int preRollDuration = settingsModel.getPreRollDuration();
        ISmModel.MODEL_VERSION version = extendedSmModel.getSoundModelVersion();
        if (bOpaqueDataTransfer) {
            if (ISmModel.MODEL_VERSION.VERSION_3_0 == version
                || ISmModel.MODEL_VERSION.VERSION_4_0 == version) {
                boolean hasKeywordIndicesParam = false;
                boolean hasTimeStampParam = false;
                opaqueDataHelper = new OpaqueDataHelper(smName,
                        bOpaqueDataTransfer, hasKeywordIndicesParam, hasTimeStampParam);
                int sKeyphraseLevel = settingsModel.get2ndKeyphraseConfidenceLevel();
                int sUserLevel = settingsModel.get2ndUserConfidenceLevel();
                int fKeyphraseLevel = settingsModel.get1stKeyphraseConfidenceLevel();
                int fUserLevel = settingsModel.get1stUserConfidenceLevel();
                opaqueByteArray = opaqueDataHelper.fillOpaqueDataByteBuffer(fKeyphraseLevel,
                        fUserLevel, sKeyphraseLevel, sUserLevel,
                        histBufferDuration, preRollDuration, 0, 0);
            } else {
                opaqueDataHelper = new OpaqueDataHelper(smName,
                        false, false,
                        false);
                opaqueByteArray = opaqueDataHelper.fillOpaqueDataByteBuffer(histBufferDuration,
                        preRollDuration);
            }

            if (Global.getInstance().getIsToggleXmlParam()) {
                opaqueByteArray = mOpaqueDataFromXml;
            }
        } else {
            opaqueByteArray = null;
        }

        SoundTrigger.RecognitionConfig stRecognitionConfig
                = new SoundTrigger.RecognitionConfig(bCaptureRequest, bAllowMultipleTriggers,
                stKpRecognitionExtraArray, opaqueByteArray);
        extendedSmModel.setRecognitionConfig(stRecognitionConfig);

        // call STM API
        returnCode = mSTModule.startRecognition(extendedSmModel.getSoundModelHandle(),
                stRecognitionConfig);
        if (RC_OK == returnCode) {
            LogUtils.d(TAG, "startRecognition: STM startRecognition success");
            extendedSmModel.setSessionStatus(IExtendedSmModel.SessionStatus.STARTED);
        }

        return returnCode;
    }

    public int stopRecognition(String smName) {
        int returnCode;
        LogUtils.d(TAG, "stopRecognition: smName = " + smName);
        if (null == smName) {
            LogUtils.d(TAG, "stopRecognition: invalid input param");
            returnCode = RC_INVALID_PARAM;
            return returnCode;
        }

        // judge the sm whether stopped or not
        IExtendedSmModel extendedSmModel = mExtendedSmMgr.getSoundModel(smName);
        if (IExtendedSmModel.SessionStatus.STOPPED == extendedSmModel.getSessionStatus()
                || IExtendedSmModel.SessionStatus.LOADED == extendedSmModel.getSessionStatus()) {
            LogUtils.d(TAG, "stopRecognition: already stopped, return success directly");
            returnCode = RC_OK;
            return returnCode;
        }

        boolean bCanTransit = isStateTransitional(extendedSmModel.getSessionStatus(),
                IExtendedSmModel.SessionStatus.STOPPED);
        if (!bCanTransit) {
            LogUtils.d(TAG, "stopRecognition: failure for wrong status");
            returnCode = RC_WRONG_STATUS;
            return returnCode;
        }

        // call STM API
        returnCode = mSTModule.stopRecognition(extendedSmModel.getSoundModelHandle());
        if (RC_OK == returnCode) {
            LogUtils.d(TAG, "stopRecognition: STM stopRecognition success");
            extendedSmModel.setSessionStatus(IExtendedSmModel.SessionStatus.STOPPED);
        }

        return returnCode;
    }

    @Override
    public int unloadSM(String smName) {
        int returnCode;
        LogUtils.d(TAG, "unloadSM: smName = " + smName);
        if (null == smName) {
            LogUtils.d(TAG, "unloadSM: invalid input param");
            returnCode = RC_INVALID_PARAM;
            return returnCode;
        }

        // judge the sm whether unloaded or not
        IExtendedSmModel extendedSmModel = mExtendedSmMgr.getSoundModel(smName);
        if (IExtendedSmModel.SessionStatus.UNLOADED == extendedSmModel.getSessionStatus()) {
            LogUtils.d(TAG, "unloadSM: already unloaded, return success directly");
            returnCode = RC_OK;
            return returnCode;
        }

        boolean bCanTransit = isStateTransitional(extendedSmModel.getSessionStatus(),
                IExtendedSmModel.SessionStatus.UNLOADED);
        if (!bCanTransit) {
            LogUtils.d(TAG, "unloadSM: failure for wrong status");
            returnCode = RC_FAILURE;
            return returnCode;
        }

        // call STM API
        returnCode = mSTModule.unloadSoundModel(extendedSmModel.getSoundModelHandle());
        if (RC_OK == returnCode) {
            LogUtils.d(TAG, "unloadSM: STM unloadSoundModel success");
            extendedSmModel.setSessionStatus(IExtendedSmModel.SessionStatus.UNLOADED);
        }

        return returnCode;
    }

    @Override
    public boolean isRecognitionActive(String smName) {
        LogUtils.d(TAG, "isRecognitionActive: not support for legacy STModule API");
        return false;
    }

    private void detachModule() {
        LogUtils.d(TAG, "detachModule: enter");
        if (null != mSTModule) {
            LogUtils.d(TAG, "detachModule: mSTModule.detach() is called");
            mSTModule.detach();
            mSTModule = null;
        }

        mExtendedSmMgr.resetAllSessionStatus();

        LogUtils.d(TAG, "detachModule: exit");
    }

    // This function requires a valid XML file exists on the device's file system named the same
    // name as the SM to be used.
    private void createStkSmInfoFromXml(String smName) {
        LogUtils.d(TAG, "createStkSmInfoFromXml: enter");
        XmlPullParserFactory xmlPullParserFactory;
        XmlPullParser parser;

        final String xmlFilePath = Global.PATH_ROOT + "/" + smName + ".xml";
        LogUtils.d(TAG, "createStkSmInfoFromXml: smName= " + smName
                + ", xmlFilePath= " + xmlFilePath);
        FileInputStream fileInputStream;
        try {
            fileInputStream = new FileInputStream(new File(xmlFilePath));
            xmlPullParserFactory = XmlPullParserFactory.newInstance();
            parser = xmlPullParserFactory.newPullParser();
            parser.setInput(fileInputStream, null);
            int eventType = parser.getEventType();
            String startTag;
            while (eventType != XmlPullParser.END_DOCUMENT) {
                switch (eventType) {
                    case XmlPullParser.START_DOCUMENT:
                        LogUtils.d(TAG, "createStkSmInfoFromXml: start_document "
                                + "parser.getName() = " + parser.getName());
                        break;
                    case XmlPullParser.START_TAG:
                        startTag = parser.getName();
                        LogUtils.d(TAG, "createStkSmInfoFromXml: start_tag = " + startTag);
                        if (startTag.equals("KeyphraseSoundModel")) {
                            // This can be ignored because it's only to signal the beginning of
                            //the XML file.
                            break;
                        } else if (startTag.equals("Uuid")) {
                            parser.next();
                            LogUtils.d(TAG, "createStkSmInfoFromXml: Uuid = "
                                    + parser.getText());
                            mUUID = UUID.fromString(parser.getText());
                        } else if (startTag.equals("VendorUuid")) {
                            parser.next();
                            LogUtils.d(TAG, "createStkSmInfoFromXml: VendorUuid = "
                                    + parser.getText());
                            mVendorUuid = UUID.fromString(parser.getText());
                        } else if (startTag.equals("Data")) {
                            parser.next();
                            LogUtils.d(TAG, "createStkSmInfoFromXml: Data= "
                                    + parser.getText());
                            if (parser.getText().equals("true")) {
                                String filePath = Global.PATH_ROOT + "/" + smName +
                                        Global.SUFFIX_DATA_FILE;
                                LogUtils.d(TAG, "createStkSmInfoFromXml: opaqueData "
                                        + " filepath = " + filePath);
                                mOpaqueDataFromXml = FileUtils.readFileToByteArray(filePath);
                            }
                        } else if (startTag.equals("Text")) {
                            parser.next();
                            LogUtils.d(TAG, "createStkSmInfoFromXml: Text = "
                                    + parser.getText());
                            mKeyphraseFromXml = parser.getText();
                        } else {
                            LogUtils.d(TAG, "createStkSmInfoFromXml: startTag = "
                                    + startTag + " not recognized.");
                            break;
                        }
                        break;
                    case XmlPullParser.END_TAG:
                        LogUtils.d(TAG, "createStkSmInfoFromXml: end_tag = "
                                + parser.getName());
                        break;
                }
                eventType = parser.next();
                LogUtils.d(TAG, "createStkSmInfoFromXml: parser.next() = "
                        + parser.getName());
            }

            if (eventType == XmlPullParser.END_DOCUMENT) {
                LogUtils.d(TAG, "createStkSmInfoFromXml: END_DOCUMENT");
                LogUtils.d(TAG, "createStkSmInfoFromXml: END_DOCUMENT parser.getName()= "
                        + parser.getName());
            } else {
                LogUtils.d(TAG, "createStkSmInfoFromXml: Could not reach end of document.");
            }

        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (XmlPullParserException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
