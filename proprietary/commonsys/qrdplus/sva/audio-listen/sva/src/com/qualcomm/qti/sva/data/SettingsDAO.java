/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva.data;

import android.content.Intent;
import android.content.SharedPreferences;

import com.qualcomm.qti.sva.utils.LogUtils;

import java.net.URISyntaxException;

final class SettingsDAO {
    private final static String TAG = SettingsDAO.class.getSimpleName();

    // setting item key definition
    private static final String KEY_GLOBAL_SM3_1ST_KEYPHRASE_CONFIDENCE_LEVEL
            = "global_sm3_1st_keyphrase_confidence_level";
    private static final String KEY_GLOBAL_SM2_1ST_KEYPHRASE_CONFIDENCE_LEVEL
            = "global_sm2_1st_keyphrase_confidence_level";
    private static final String KEY_GLOBAL_SM3_1ST_USER_CONFIDENCE_LEVEL
            = "global_sm3_1st_user_confidence_level";
    private static final String KEY_GLOBAL_SM2_1ST_USER_CONFIDENCE_LEVEL
            = "global_sm2_1st_user_confidence_level";
    private static final String KEY_GLOBAL_SM3_2ND_KEYPHRASE_CONFIDENCE_LEVEL
            = "global_sm3_2nd_keyphrase_confidence_level";
    private static final String KEY_GLOBAL_SM3_2ND_USER_CONFIDENCE_LEVEL
            = "global_sm3_2nd_user_confidence_level";
    private static final String KEY_GLOBAL_GMM_TRAINING_CONFIDENCE_LEVEL
            = "global_gmm_training_confidence_level";
    private static final String KEY_1ST_KEYPHRASE_CONFIDENCE_LEVEL
            = "1st_keyphrase_confidence_level";
    private static final String KEY_1ST_USER_CONFIDENCE_LEVEL
            = "1st_user_confidence_level";
    private static final String KEY_2ND_KEYPHRASE_CONFIDENCE_LEVEL
            = "2nd_keyphrase_confidence_level";
    private static final String KEY_2ND_USER_CONFIDENCE_LEVEL
            = "2nd_user_confidence_level";
    private static final String KEY_GLOBAL_DETECTION_TONE_ENABLED
            = "global_detection_tone_enabled";
    private static final String KEY_GLOBAL_IS_DISPLAY_ADVANCED_DETAILS
            = "global_is_display_advanced_details";
    private static final String KEY_USER_VERIFICATION_ENABLED
            = "user_verification_enabled";
    private static final String KEY_VOICE_REQUEST_ENABLED
            = "voice_request_enabled";
    private static final String KEY_VOICE_REQUEST_LENGTH_IN_MILLISEC
            = "voice_request_len_in_millisec";
    private static final String KEY_OPAQUE_DATA_TRANSFER_ENABLED
            = "opaque_data_transfer_enabled";
    private static final String KEY_HIST_BUFFER_TIME_IN_MILLI_SEC
            = "hist_buffer_time_in_milli_sec";
    private static final String KEY_PRE_ROLL_DURATION_IN_MILLI_SEC
            = "pre_roll_duration_in_milli_sec";
    private static final String KEY_ACTION_NAME = "action_name";
    private static final String KEY_ACTION_INTENT = "action_intent";

    // default NiHaoXiaoLong model is difficult to be re-trained,
    // need set this value to lower value
    // so change default training confidence level to 50
    private static final int DEFAULT_GLOBAL_GMM_TRAINING_CONFIDENCE_LEVEL = 50;
    private static final boolean DEFAULT_DETECTION_TONE_ENABLED = true;
    private static final boolean DEFAULT_IS_DISPLAY_ADVANCED_DETAILS = true;
    private static final boolean DEFAULT_USER_VERIFICATION_ENABLED = true;
    private static final boolean DEFAULT_VOICE_REQUEST_ENABLED = false;
    private static final int DEFAULT_VOICE_REQUEST_LENGTH = 3000;
    private static final boolean DEFAULT_OPAQUE_DATA_TRANSFER_ENABLED = false;
    private static final int DEFAULT_HIST_BUFFER_TIME = 2000;
    private static final int DEFAULT_PRE_ROLL_DURATION = 500;
    private static final String DEFAULT_ACTION_NAME = "None";
    private static final Intent DEFAULT_ACTION_INTENT = null;

    // setting item default value definition
    private static final int DEFAULT_GLOBAL_SM2_1ST_KEYPHRASE_CONFIDENCE_LEVEL = 69;
    private static final int DEFAULT_GLOBAL_SM2_1ST_USER_CONFIDENCE_LEVEL = 69;
    private static final int DEFAULT_GLOBAL_SM3_1ST_KEYPHRASE_CONFIDENCE_LEVEL = 40;
    private static final int DEFAULT_GLOBAL_SM3_1ST_USER_CONFIDENCE_LEVEL = 1;
    private static final int DEFAULT_GLOBAL_SM3_2ND_KEYPHRASE_CONFIDENCE_LEVEL = 40;
    private static final int DEFAULT_GLOBAL_SM3_2ND_USER_CONFIDENCE_LEVEL = 35;

    private static final int TYPE_1ST_KEYPHRASE= 1;
    private static final int TYPE_1ST_USER = 2;
    private static final int TYPE_2ND_KEYPHRASE = 3;
    private static final int TYPE_2ND_USER = 4;

    private SettingsDAO() {
    }

    static int getGlobal1stKeyphraseConfidenceLevel(SharedPreferences sharedPrefs,
                                                    ISmModel.MODEL_VERSION version) {
        return getGlobalConfidenceLevel(sharedPrefs, TYPE_1ST_KEYPHRASE, version);
    }

    static void setGlobal1stKeyphraseConfidenceLevel(SharedPreferences sharedPrefs, 
                                                    ISmModel.MODEL_VERSION version,int level) {
        setGlobalConfidenceLevel(sharedPrefs, TYPE_1ST_KEYPHRASE, version, level);
    }

    static int getGlobal1stUserConfidenceLevel(SharedPreferences sharedPrefs,
                                                ISmModel.MODEL_VERSION version) {
        return getGlobalConfidenceLevel(sharedPrefs, TYPE_1ST_USER, version);
    }

    static void setGlobal1stUserConfidenceLevel(SharedPreferences sharedPrefs,
                                                ISmModel.MODEL_VERSION version, int level) {
        setGlobalConfidenceLevel(sharedPrefs, TYPE_1ST_USER, version, level);
    }

    static int getGlobal2ndKeyphraseConfidenceLevel(SharedPreferences sharedPrefs,
                                                ISmModel.MODEL_VERSION version) {
        return getGlobalConfidenceLevel(sharedPrefs, TYPE_2ND_KEYPHRASE, version);
    }

    static void setGlobal2ndKeyphraseConfidenceLevel(SharedPreferences sharedPrefs,
                                                ISmModel.MODEL_VERSION version, int level) {
        setGlobalConfidenceLevel(sharedPrefs, TYPE_2ND_KEYPHRASE, version, level);
    }

    static int getGlobal2ndUserConfidenceLevel(SharedPreferences sharedPrefs,
                                                ISmModel.MODEL_VERSION version) {
        return getGlobalConfidenceLevel(sharedPrefs, TYPE_2ND_USER, version);
    }

    static void setGlobal2ndUserConfidenceLevel(SharedPreferences sharedPrefs,
                                                ISmModel.MODEL_VERSION version, int level) {
        setGlobalConfidenceLevel(sharedPrefs, TYPE_2ND_USER, version, level);
    }

    static int getGlobalGMMTrainingConfidenceLevel(SharedPreferences sharedPrefs) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "getGlobalGMMTrainingConfidenceLevel: invalid param");
            return DEFAULT_GLOBAL_GMM_TRAINING_CONFIDENCE_LEVEL;
        }

        return sharedPrefs.getInt(KEY_GLOBAL_GMM_TRAINING_CONFIDENCE_LEVEL,
                DEFAULT_GLOBAL_GMM_TRAINING_CONFIDENCE_LEVEL);
    }

    static void setGlobalGMMTrainingConfidenceLevel(SharedPreferences sharedPrefs, int level) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setGlobalGMMTrainingConfidenceLevel: invalid param");
            return;
        }
        sharedPrefs.edit().putInt(KEY_GLOBAL_GMM_TRAINING_CONFIDENCE_LEVEL, level).apply();
    }

    static int get1stKeyphraseConfidenceLevel(SharedPreferences sharedPrefs,
                                              SharedPreferences globalPrefs,
                                              ISmModel.MODEL_VERSION version) {
        return getModelConfidenceLevel(sharedPrefs, globalPrefs, TYPE_1ST_KEYPHRASE, version);
    }

    static void set1stKeyphraseConfidenceLevel(SharedPreferences sharedPrefs, int level) {
        setModelConfidenceLevel(sharedPrefs, TYPE_1ST_KEYPHRASE, level);
    }

    static int get1stUserConfidenceLevel(SharedPreferences sharedPrefs,
                                         SharedPreferences globalPrefs,
                                         ISmModel.MODEL_VERSION version) {
        return getModelConfidenceLevel(sharedPrefs, globalPrefs, TYPE_1ST_USER, version);
    }

    static void set1stUserConfidenceLevel(SharedPreferences sharedPrefs, int level) {
        setModelConfidenceLevel(sharedPrefs, TYPE_1ST_USER, level);
    }

    static int get2ndKeyphraseConfidenceLevel(SharedPreferences sharedPrefs,
                                              SharedPreferences globalPrefs,
                                              ISmModel.MODEL_VERSION version) {
        return getModelConfidenceLevel(sharedPrefs, globalPrefs, TYPE_2ND_KEYPHRASE, version);
    }

    static void set2ndKeyphraseConfidenceLevel(SharedPreferences sharedPrefs, int level) {
        setModelConfidenceLevel(sharedPrefs, TYPE_2ND_KEYPHRASE, level);
    }

    static int get2ndUserConfidenceLevel(SharedPreferences sharedPrefs,
                                         SharedPreferences globalPrefs,
                                         ISmModel.MODEL_VERSION version) {
        return getModelConfidenceLevel(sharedPrefs, globalPrefs, TYPE_2ND_USER, version);
    }

    static void set2ndUserConfidenceLevel(SharedPreferences sharedPrefs, int level) {
        setModelConfidenceLevel(sharedPrefs, TYPE_2ND_USER, level);
    }

    static boolean getGlobalDetectionToneEnabled(SharedPreferences sharedPrefs) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "getGlobalDetectionToneEnabled: invalid param");
            return DEFAULT_DETECTION_TONE_ENABLED;
        }

        return sharedPrefs.getBoolean(KEY_GLOBAL_DETECTION_TONE_ENABLED,
                DEFAULT_DETECTION_TONE_ENABLED);
    }

    static void setGlobalDetectionToneEnabled(SharedPreferences sharedPrefs, boolean enabled) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setGlobalDetectionToneEnabled: invalid param");
            return;
        }

        sharedPrefs.edit().putBoolean(KEY_GLOBAL_DETECTION_TONE_ENABLED, enabled).apply();
    }

    static boolean getGlobalIsDisplayAdvancedDetails(SharedPreferences sharedPrefs) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "getGlobalIsDisplayAdvancedDetails: invalid param");
            return DEFAULT_IS_DISPLAY_ADVANCED_DETAILS;
        }

        return sharedPrefs.getBoolean(KEY_GLOBAL_IS_DISPLAY_ADVANCED_DETAILS,
                DEFAULT_IS_DISPLAY_ADVANCED_DETAILS);
    }

    static void setGlobalIsDisplayAdvancedDetails(SharedPreferences sharedPrefs,
                                                  boolean bDisplay) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setGlobalIsDisplayAdvancedDetails: invalid param");
            return;
        }

        sharedPrefs.edit().putBoolean(KEY_GLOBAL_IS_DISPLAY_ADVANCED_DETAILS, bDisplay).apply();
    }

    static boolean getUserVerificationEnabled(SharedPreferences sharedPrefs) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "getUserVerificationEnabled: invalid param");
            return DEFAULT_USER_VERIFICATION_ENABLED;
        }

        return sharedPrefs.getBoolean(KEY_USER_VERIFICATION_ENABLED,
                DEFAULT_USER_VERIFICATION_ENABLED);
    }

    static void setUserVerificationEnabled(SharedPreferences sharedPrefs, boolean enabled) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setUserVerificationEnabled: invalid param");
            return;
        }

        sharedPrefs.edit().putBoolean(KEY_USER_VERIFICATION_ENABLED, enabled).apply();
    }

    static boolean getVoiceRequestEnabled(SharedPreferences sharedPrefs) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "getVoiceRequestEnabled: invalid param");
            return DEFAULT_VOICE_REQUEST_ENABLED;
        }

        return sharedPrefs.getBoolean(KEY_VOICE_REQUEST_ENABLED,
                DEFAULT_VOICE_REQUEST_ENABLED);
    }

    static void setVoiceRequestEnabled(SharedPreferences sharedPrefs, boolean enabled) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setVoiceRequestEnabled: invalid param");
            return;
        }

        sharedPrefs.edit().putBoolean(KEY_VOICE_REQUEST_ENABLED, enabled).apply();
    }

    static int getVoiceRequestLength(SharedPreferences sharedPrefs) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "getVoiceRequestLength: invalid param");
            return DEFAULT_VOICE_REQUEST_LENGTH;
        }
        return sharedPrefs.getInt(KEY_VOICE_REQUEST_LENGTH_IN_MILLISEC,
                DEFAULT_VOICE_REQUEST_LENGTH);
    }

    static void setVoiceRequestLength(SharedPreferences sharedPrefs, int len) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setVoiceRequestLength: invalid param");
            return;
        }
        if (len > 0) {
            sharedPrefs.edit().putInt(KEY_VOICE_REQUEST_LENGTH_IN_MILLISEC, len).apply();
        }
    }

    static boolean getOpaqueDataTransferEnabled(SharedPreferences sharedPrefs) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "getOpaqueDataTransferEnabled: invalid param");
            return DEFAULT_OPAQUE_DATA_TRANSFER_ENABLED;
        }

        return sharedPrefs.getBoolean(KEY_OPAQUE_DATA_TRANSFER_ENABLED,
                DEFAULT_OPAQUE_DATA_TRANSFER_ENABLED);
    }

    static void setOpaqueDataTransferEnabled(SharedPreferences sharedPrefs, boolean enabled) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setOpaqueDataTransferEnabled: invalid param");
            return;
        }

        sharedPrefs.edit().putBoolean(KEY_OPAQUE_DATA_TRANSFER_ENABLED, enabled).apply();
    }

    static int getHistBufferTime(SharedPreferences sharedPrefs) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "getHistBufferTime: invalid param");
            return DEFAULT_HIST_BUFFER_TIME;
        }

        return sharedPrefs.getInt(KEY_HIST_BUFFER_TIME_IN_MILLI_SEC,
                DEFAULT_HIST_BUFFER_TIME);
    }

    static void setHistBufferTime(SharedPreferences sharedPrefs, int len) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setHistBufferTime: invalid param");
            return;
        }

        if (len >= 0) {
            sharedPrefs.edit().putInt(KEY_HIST_BUFFER_TIME_IN_MILLI_SEC, len).apply();
        }
    }

    static int getPreRollDuration(SharedPreferences sharedPrefs) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "getPreRollDuration: invalid param");
            return DEFAULT_PRE_ROLL_DURATION;
        }

        return sharedPrefs.getInt(KEY_PRE_ROLL_DURATION_IN_MILLI_SEC,
                DEFAULT_PRE_ROLL_DURATION);
    }

    static void setPreRollDuration(SharedPreferences sharedPrefs, int len) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setPreRollDuration: invalid param");
            return;
        }

        if (len >= 0) {
            sharedPrefs.edit().putInt(KEY_PRE_ROLL_DURATION_IN_MILLI_SEC, len).apply();
        }
    }

    public static void setActionName(SharedPreferences sharedPrefs, String actionName) {
        LogUtils.d(TAG, "setActionName: actionName = " + actionName);
        if (null == sharedPrefs || null == actionName) {
            LogUtils.d(TAG, "setActionName: invalid param");
            return;
        }

        sharedPrefs.edit().putString(KEY_ACTION_NAME, actionName).apply();
    }

    public static String getActionName(SharedPreferences sharedPrefs) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "getActionName: invalid param");
            return DEFAULT_ACTION_NAME;
        }

        return sharedPrefs.getString(KEY_ACTION_NAME, DEFAULT_ACTION_NAME);
    }

    public static Intent getActionIntent(SharedPreferences sharedPrefs) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "getActionIntent: invalid param");
            return DEFAULT_ACTION_INTENT;
        }

        Intent result;
        try {
            String action = sharedPrefs.getString(KEY_ACTION_INTENT, null);
            if (null != action) {
                result = Intent.parseUri(action, 0);
            } else {
                result = null;
            }
        } catch (URISyntaxException e) {
            e.printStackTrace();
            result = null;
        }

        LogUtils.d(TAG, "getActionIntent: result = " + result);
        return result;
    }

    public static void setActionIntent(SharedPreferences sharedPrefs, Intent actionIntent) {
        LogUtils.d(TAG, "setActionIntent: actionIntent = " + actionIntent);
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setActionIntent: invalid param");
            return;
        }

        if (null != actionIntent) {
            sharedPrefs.edit().putString(KEY_ACTION_INTENT, actionIntent.toUri(0)).apply();
        } else {
            sharedPrefs.edit().putString(KEY_ACTION_INTENT, null).apply();
        }
    }

    private static int getDefaultGlobalConfidenceLevel(int type,
                                                       ISmModel.MODEL_VERSION version) {
        int confidenceLevel = -1;
        switch(type) {
            case TYPE_1ST_KEYPHRASE:
                if (ISmModel.MODEL_VERSION.VERSION_2_0 == version) {
                    confidenceLevel = DEFAULT_GLOBAL_SM2_1ST_KEYPHRASE_CONFIDENCE_LEVEL;
                } else if(ISmModel.MODEL_VERSION.VERSION_3_0 == version
                            || ISmModel.MODEL_VERSION.VERSION_4_0 == version) {
                    confidenceLevel = DEFAULT_GLOBAL_SM3_1ST_KEYPHRASE_CONFIDENCE_LEVEL;
                }
                break;
            case TYPE_1ST_USER:
                if (ISmModel.MODEL_VERSION.VERSION_2_0 == version) {
                    confidenceLevel = DEFAULT_GLOBAL_SM2_1ST_USER_CONFIDENCE_LEVEL;
                } else if(ISmModel.MODEL_VERSION.VERSION_3_0 == version
                            || ISmModel.MODEL_VERSION.VERSION_4_0 == version) {
                    confidenceLevel = DEFAULT_GLOBAL_SM3_1ST_USER_CONFIDENCE_LEVEL;
                }
                break;
            case TYPE_2ND_KEYPHRASE:
                confidenceLevel = DEFAULT_GLOBAL_SM3_2ND_KEYPHRASE_CONFIDENCE_LEVEL;
                break;
            case TYPE_2ND_USER:
                confidenceLevel = DEFAULT_GLOBAL_SM3_2ND_USER_CONFIDENCE_LEVEL;
                break;
            default:
                break;
        }
        return confidenceLevel;
    }

    private static String getGlobalKey(int type, ISmModel.MODEL_VERSION version) {
        String globalKey = null;
        switch(type) {
            case TYPE_1ST_KEYPHRASE:
                if (ISmModel.MODEL_VERSION.VERSION_2_0 == version) {
                    globalKey = KEY_GLOBAL_SM2_1ST_KEYPHRASE_CONFIDENCE_LEVEL;
                } else if (ISmModel.MODEL_VERSION.VERSION_3_0 == version
                             || ISmModel.MODEL_VERSION.VERSION_4_0 == version) {
                    globalKey = KEY_GLOBAL_SM3_1ST_KEYPHRASE_CONFIDENCE_LEVEL;
                }
                break;
            case TYPE_1ST_USER:
                if (ISmModel.MODEL_VERSION.VERSION_2_0 == version) {
                    globalKey = KEY_GLOBAL_SM2_1ST_USER_CONFIDENCE_LEVEL;
                } else if (ISmModel.MODEL_VERSION.VERSION_3_0 == version
                            || ISmModel.MODEL_VERSION.VERSION_4_0 == version) {
                    globalKey = KEY_GLOBAL_SM3_1ST_USER_CONFIDENCE_LEVEL;
                }
                break;
            case TYPE_2ND_KEYPHRASE:
                globalKey = KEY_GLOBAL_SM3_2ND_KEYPHRASE_CONFIDENCE_LEVEL;
                break;
            case TYPE_2ND_USER:
                globalKey = KEY_GLOBAL_SM3_2ND_USER_CONFIDENCE_LEVEL;
                break;
            default:
                break;
        }
        return globalKey;
    }

    private static int getGlobalConfidenceLevel(SharedPreferences sharedPrefs, int type,
                                                ISmModel.MODEL_VERSION version) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "getGlobalConfidenceLevel: invalid param");
            return getDefaultGlobalConfidenceLevel(type, version);
        }

        int confidenceLevel = 0;
        String globalKey = getGlobalKey(type, version);
        if (null == globalKey) {
            return getDefaultGlobalConfidenceLevel(type, version);
        }

        return sharedPrefs.getInt(globalKey, getDefaultGlobalConfidenceLevel(type, version));
    }

    private static void setGlobalConfidenceLevel(SharedPreferences sharedPrefs, int type,
                                                ISmModel.MODEL_VERSION version, int level) {
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setGlobalConfidenceLevel: invalid param");
            return;
        }

        String globalKey = getGlobalKey(type, version);
        if (null == globalKey) {
            return;
        }
        sharedPrefs.edit().putInt(globalKey, level).apply();
    }

    private static String getModelkey(int type) {
        String modelKey = null;
        switch(type) {
            case TYPE_1ST_KEYPHRASE:
                modelKey = KEY_1ST_KEYPHRASE_CONFIDENCE_LEVEL;
                break;
            case TYPE_1ST_USER:
                modelKey = KEY_1ST_USER_CONFIDENCE_LEVEL;
                break;
            case TYPE_2ND_KEYPHRASE:
                modelKey = KEY_2ND_KEYPHRASE_CONFIDENCE_LEVEL;
                break;
            case TYPE_2ND_USER:
                modelKey = KEY_2ND_USER_CONFIDENCE_LEVEL;
                break;
            default:
                break;
        }
        return modelKey;
    }

    private static int getModelConfidenceLevel(SharedPreferences sharedPrefs,
                                                SharedPreferences globalPrefs,
                                                int type,
                                                ISmModel.MODEL_VERSION version) {
        if (null == sharedPrefs && null == globalPrefs) {
            LogUtils.d(TAG, "setModelConfidenceLevel: invalid param");
            return getDefaultGlobalConfidenceLevel(type, version);
        }
        String key = getModelkey(type);
        if (null == key) {
            return getGlobalConfidenceLevel(globalPrefs, type, version);
        }
        return sharedPrefs.getInt(key, getGlobalConfidenceLevel(globalPrefs, type, version));
    }

    private static void setModelConfidenceLevel(SharedPreferences sharedPrefs, int type, int level){
        if (null == sharedPrefs) {
            LogUtils.d(TAG, "setModelConfidenceLevel: invalid param");
            return;
        }
        String key = getModelkey(type);
        if (null == key) {
            return;
        }
        sharedPrefs.edit().putInt(key, level).apply();
    }
}
