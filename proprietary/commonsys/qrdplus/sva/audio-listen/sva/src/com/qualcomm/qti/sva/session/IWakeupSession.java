/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva.session;

import android.hardware.soundtrigger.SoundTrigger;

import com.qualcomm.qti.sva.session.legacy.IWakeupListener;

public interface IWakeupSession {
    // SoundTrigger API rc code
    int RC_OK = SoundTrigger.STATUS_OK; // 0,Status code used when the operation succeeded
    int RC_ERROR = SoundTrigger.STATUS_ERROR;// Integer.MIN_VALUE -2147483648;
    int RC_PERMISSION_DENIED = SoundTrigger.STATUS_PERMISSION_DENIED;
    int RC_NO_INIT = SoundTrigger.STATUS_NO_INIT; // AudioServer not ready or no ST HAL
    int RC_BAD_VALUE = SoundTrigger.STATUS_BAD_VALUE;
    int RC_DEAD_OBJECT = SoundTrigger.STATUS_DEAD_OBJECT; // AudioServer binder no rsp
    int RC_INVALID_OPERATION = SoundTrigger.STATUS_INVALID_OPERATION;

    // App self define rc, which begin from -1000
    int RC_INVALID_PARAM = -1000;
    int RC_FILE_NOT_EXIST = -1001;
    int RC_WRONG_STATUS = -1002;
    int RC_FAILURE = -1003;

    // PendingIntent Const
    String ACTION_RECOGNITION = "com.qualcomm.qti.sva.ACTION_RECOGNITION";
    String KEY_EXTRA_SM_NAME = "smName";

    int establishSvaSession(String smName);

    int terminateSvaSession(String smName);

    void releaseAllSvaSessions();

    int loadSM(String smName);

    int startRecognition(String smName);

    int stopRecognition(String smName);

    int unloadSM(String smName);

    int restartRecognition(String smName);

    // new STService API only
    boolean isRecognitionActive(String smName);

    // legacy STModule API only
    void addWakeupListener(IWakeupListener listener);

    void removeWakeupListener(IWakeupListener listener);

}
