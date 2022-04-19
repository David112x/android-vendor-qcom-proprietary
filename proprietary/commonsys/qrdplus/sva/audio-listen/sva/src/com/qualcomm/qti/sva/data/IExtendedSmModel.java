/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva.data;

import android.content.Intent;
import android.hardware.soundtrigger.SoundTrigger;

public interface IExtendedSmModel extends ISmModel {
    SessionStatus getSessionStatus();

    void setSessionStatus(SessionStatus status);

    String getSoundModelActionName();

    void setSoundModelActionName(String actionName);

    Intent getSoundModelActionIntent();

    void setSoundModelActionIntent(Intent actionIntent);

    SoundTrigger.KeyphraseRecognitionExtra[] getKeyphraseRecognitionExtra();

    void setKeyphraseRecognitionExtra(SoundTrigger.KeyphraseRecognitionExtra[] extraArray);

    SoundTrigger.RecognitionConfig getRecognitionConfig();

    void setRecognitionConfig(SoundTrigger.RecognitionConfig config);

    int getSoundModelHandle();

    void setSoundModelHandle(int handle);

    // these are used for debug mode dynamic view match
    int getSessionId();

    void setSessionId(int id);

    enum SessionStatus {
        UNLOADED,
        LOADED,
        STARTED,
        STOPPED
    }

}
