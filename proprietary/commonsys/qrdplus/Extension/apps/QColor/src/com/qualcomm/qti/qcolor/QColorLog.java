/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.qcolor;

public class QColorLog {
    private static String LOG_TAG = "QColorMode";

    private static boolean isEnableDebugLogging(){
        return true;//android.util.Log.isLoggable(LOG_TAG, android.util.Log.VERBOSE);
    }

    private static String combineLogMessage(String tag, String message){
        return tag + ": " + message;
    }

    public static void i(String tag, String message){
        if(isEnableDebugLogging()){
            android.util.Log.i(LOG_TAG, combineLogMessage(tag, message));
        }
    }

    public static void v(String tag, String message){
        if(isEnableDebugLogging()){
            android.util.Log.v(LOG_TAG, combineLogMessage(tag, message));
        }
    }

    public static void d(String tag, String message){
        if(isEnableDebugLogging()){
            android.util.Log.d(LOG_TAG, combineLogMessage(tag, message));
        }
    }

    public static void w(String tag, String message){
        android.util.Log.w(LOG_TAG, combineLogMessage(tag, message));
    }

    public static void e(String tag, String message){
        android.util.Log.e(LOG_TAG, combineLogMessage(tag, message));
    }
}
