/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva.data;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

public interface ISmModel {
    // valid language model and sound model file suffix
    String SUFFIX_LANGUAGE_MODEL = ".lm";
    String SUFFIX_MERGED_LANGUAGE_MODEL = ".lat";
    String SUFFIX_DEFAULT_SOUND_MODEL = ".uim";
    String SUFFIX_TRAINED_SOUND_MODEL = ".udm";

    int UNKNOWN_SOUND_MODEL_TYPE = 0;

    /**
     * SoundModel is Snapdragon Voice Activation format
     */
    int SVA_SOUND_MODEL_TYPE = 1;

    int VERSION_UNKNOWN = 0;
    /**
     * Version 1.0
     */
    int VERSION_0100 = 0x0100;
    /**
     * Version 2.0
     */
    int VERSION_0200 = 0x0200;
    /**
     * Version 3.0
     */
    int VERSION_0300 = 0x0300;
    /**
     * Version 4.0
     */
    int VERSION_0400 = 0x0400;


    UUID getSoundModelUUID();

    MODEL_VERSION getSoundModelVersion();

    void setSoundModelVersion(MODEL_VERSION version);

    int getSoundModelType();

    void setSoundModelType(int smType);

    String getSoundModelFullFileName();

    void setSoundModelFullFileName(String fullFileName);

    String getSoundModelPrettyFileName();

    void setSoundModelPrettyFileName(String prettyFileName);

    String getSoundModelSuffix();

    void setSoundModelSuffix(String suffix);

    String getSoundModelPrettyKeyphrase();

    void setSoundModelPrettyKeyphrase(String prettyKeyphrase);

    IKeyphraseModel getKeyphraseModel(String keyphrase);

    List<IKeyphraseModel> getSoundModelKeyphraseList();

    ArrayList<IUserModel> getAllUsers();

    boolean isUserExistForKeyphrase(String keyphrase, String userName);

    void addKeyphrase(String keyphrase, int keyphraseId);

    void addKeyphraseAndUser(String keyphrase, int keyphraseId,
                             String userName, int userId);

    void removeKeyphrase(String keyphrase);

    void removeUser(String keyphrase, String userName);

    boolean isUserTrainedSoundModel();

    boolean canAddUser();

    String getKeyphraseNameById(int id);

    boolean isUdkSm();

    enum MODEL_VERSION {
        VERSION_UNKNOWN, // non-SVA model
        VERSION_1_0,  // version 1.0
        VERSION_2_0,  // version 2.0
        VERSION_3_0,  // version 3.0
        VERSION_4_0,  // version 4.0
    }
}
