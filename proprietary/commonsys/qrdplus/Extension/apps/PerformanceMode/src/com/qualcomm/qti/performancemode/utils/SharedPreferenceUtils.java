/**
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.performancemode.utils;

import android.content.Context;
import android.content.SharedPreferences;
import android.os.Build;
import android.preference.PreferenceManager;

public class SharedPreferenceUtils {
    private static final String TAG = SharedPreferenceUtils.class.getSimpleName();
    private static final String PERFORMANCE_MODE_STATUS = "performance_mode_status";
    private static final String SESSION_HANDLE = "session_handle";

    /**
     * used for platform build compatible
     */
    private static boolean isAtLeastN() {
        return Build.VERSION.SDK_INT >= Build.VERSION_CODES.N;
    }

    private static SharedPreferences getDefaultSharedPreferences(Context context) {
        final Context storageContext;
        if (isAtLeastN()) {
            // All N devices have split storage areas. Migrate the existing preferences into the
            // new device encrypted storage area if that has not yet occurred.
            final String name = PreferenceManager.getDefaultSharedPreferencesName(context);
            storageContext = context.createDeviceProtectedStorageContext();
            if (!storageContext.moveSharedPreferencesFrom(context, name)) {
                LogUtils.e(TAG, "Failed to migrate shared preferences");
            }
        } else {
            storageContext = context;
        }
        return PreferenceManager.getDefaultSharedPreferences(storageContext);
    }

    public static boolean getPerformanceModeStatus(Context context) {
        SharedPreferences prefs = getDefaultSharedPreferences(context);
        return prefs.getBoolean(PERFORMANCE_MODE_STATUS, false);
    }

    public static void setPerformanceModeStatus(Context context, boolean isOn) {
        SharedPreferences prefs = getDefaultSharedPreferences(context);
        SharedPreferences.Editor editor = prefs.edit();
        editor.putBoolean(PERFORMANCE_MODE_STATUS, isOn);
        editor.apply();
    }

    public static int getSessionHandle(Context context) {
        SharedPreferences prefs = getDefaultSharedPreferences(context);
        return prefs.getInt(SESSION_HANDLE, -1);
    }

    public static void setSessionHandle(Context context, int handle){
        SharedPreferences prefs = getDefaultSharedPreferences(context);
        SharedPreferences.Editor editor = prefs.edit();
        editor.putInt(SESSION_HANDLE, handle);
        editor.apply();
    }
}
