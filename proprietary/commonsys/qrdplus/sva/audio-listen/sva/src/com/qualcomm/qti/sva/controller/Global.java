/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva.controller;

import android.content.Context;
import android.os.Environment;
import android.provider.Settings;

import com.qualcomm.qti.sva.R;
import com.qualcomm.qti.sva.wrapper.SystemPropertiesWrapper;

public class Global {
    public final static String SVA_DEBUG_MODE_ENABLED = "sva_debug_mode_enabled";

    // define SVA app root directory and its subdirectory
    public final static String NAME_APP = "SVA";
    public final static String NAME_TRAINING_SUBDIR = "trainings";
    public final static String NAME_VOICE_REQUESTS_SUBDIR = "voiceRequests";
    public final static String SUFFIX_WAV_FILE = ".wav";
    public final static String NAME_LANGUAGE_MODEL = "default.lm";
    public final static String NAME_MERGED_LANGUAGE_MODEL = "merged_language_model.lat";

    //storage path
    public static String PATH_ROOT = null;
    public static String PATH_TRAINING_RECORDINGS = null;
    public static String PATH_TRAINING_RECORDING_TEMP_FILE = null;
    public static String PATH_VOICE_REQUESTS = null;

    public final static String SUFFIX_DATA_FILE = ".data";

    private final String TAG = Global.class.getSimpleName();

    private static Global mInstance;
    private AssetsFileMgr mAssetFileMgr;
    private RecordingsMgr mRecordingsMgr;
    private ExtendedSmMgr mExtendedSmMgr;

    private boolean mIsToggleXmlParam = false;

    private Global(Context context) {
        PATH_ROOT = context.getExternalFilesDir(null).getAbsolutePath();
        PATH_TRAINING_RECORDINGS = PATH_ROOT + "/"
            + NAME_TRAINING_SUBDIR;
        PATH_TRAINING_RECORDING_TEMP_FILE = PATH_TRAINING_RECORDINGS
            + "/" + "temp_recording.wav";
        PATH_VOICE_REQUESTS = PATH_ROOT + "/"
            + NAME_VOICE_REQUESTS_SUBDIR;
        mAssetFileMgr = new AssetsFileMgr();
        mRecordingsMgr = new RecordingsMgr();
        mExtendedSmMgr = new ExtendedSmMgr();
    }

    public static Global getInstance() {
        return mInstance;
    }

    public static void register(final Context context) {
        mInstance = new Global(context);
    }

    public static boolean isSvaDebugModeEnabled(Context context) {
        return Settings.Global.getInt(context.getContentResolver(),
                SVA_DEBUG_MODE_ENABLED, 0) != 0;
    }

    public AssetsFileMgr getAssetFileMgr() {
        return mAssetFileMgr;
    }

    public RecordingsMgr getRecordingsMgr() {
        return mRecordingsMgr;
    }

    public ExtendedSmMgr getExtendedSmMgr() {
        return mExtendedSmMgr;
    }

    public boolean getIsToggleXmlParam() {
        return mIsToggleXmlParam;
    }

    public void setIsToggleXmlParam(boolean bToggle) {
        mIsToggleXmlParam = bToggle;
    }
}
