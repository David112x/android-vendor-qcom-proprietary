/**
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.performancemode.utils;

import android.util.Log;

public final class  LogUtils {

    private static final String APP_TAG = "PerformanceMode";
    private static final int DEBUG = Log.DEBUG;
    private static final int ERROR = Log.ERROR;

    private static boolean isEnabledLogging(int level){
        return Log.isLoggable(APP_TAG, level);
    }

    private static String combineLogMessage(String tag, String message){
        return tag + ": " + message;
    }

    public static void d(String tag, String msg) {
        if (isEnabledLogging(DEBUG)) {
            Log.d(APP_TAG, combineLogMessage(tag, msg));
        }
    }

    public static void e(String tag, String msg) {
        if (isEnabledLogging(ERROR)) {
            Log.e(APP_TAG, combineLogMessage(tag, msg));
        }
    }
}
