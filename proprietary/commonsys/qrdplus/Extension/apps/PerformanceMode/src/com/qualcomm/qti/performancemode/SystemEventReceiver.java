/**
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.performancemode;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;

import com.qualcomm.qti.performancemode.api.PerformanceModeManager;
import com.qualcomm.qti.performancemode.utils.LogUtils;
import com.qualcomm.qti.performancemode.utils.SharedPreferenceUtils;

public class SystemEventReceiver extends BroadcastReceiver {
    private static final String TAG = SystemEventReceiver.class.getSimpleName();

    @Override
    public void onReceive(Context context, Intent intent) {
        String intentAction = intent.getAction();
        Context appContext = context.getApplicationContext();
        boolean isOnMode = SharedPreferenceUtils.getPerformanceModeStatus(appContext);

        LogUtils.d(TAG, "onReceive: intentAction = " + intentAction
                + " isOnMode = " + isOnMode);

        if (Intent.ACTION_BOOT_COMPLETED.equals(intentAction) ||
                Intent.ACTION_LOCKED_BOOT_COMPLETED.equals(intentAction)) {
            PerformanceModeManager manager = PerformanceModeManager.getInstance();
            boolean isSupport = manager.isPerformanceModeSupport();
            if (isSupport && isOnMode) {
                // the last saved performance mode is on
                // so need re-try to turn it on.
                tryTurnOnPerformanceMode(appContext);
            } else {
                SharedPreferenceUtils.setSessionHandle(appContext, -1);
            }
            enablePerformanceModeActivity(appContext, isSupport);
        } else if (Intent.ACTION_SHUTDOWN.equals(intentAction)) {
            if (isOnMode) {
                LogUtils.d(TAG, "onReceive: shutdown set session handle to -1");
                SharedPreferenceUtils.setSessionHandle(appContext, -1);
            }
        }
    }

    private void enablePerformanceModeActivity(Context context, boolean enable) {
        LogUtils.d(TAG, "enablePerformanceModeActivity: enable = " + enable);
        PackageManager pm = context.getPackageManager();
        ComponentName name = new ComponentName(context, PerformanceModeActivity.class);
        if (enable) {
            pm.setComponentEnabledSetting(name, PackageManager.COMPONENT_ENABLED_STATE_ENABLED,
                    PackageManager.DONT_KILL_APP);
        } else {
            // kill the app when performance mode feature is not supported
            pm.setComponentEnabledSetting(name,
                    PackageManager.COMPONENT_ENABLED_STATE_DISABLED, 0);
        }
    }

    private void tryTurnOnPerformanceMode(Context context) {
        PerformanceModeManager manager = PerformanceModeManager.getInstance();

        /* when user first enter settings app do turn on,
         * then received boot receiver, this case need try
         * once turn off request.
         */
        Context appContext = context.getApplicationContext();
        int lastHandle = SharedPreferenceUtils.getSessionHandle(appContext);
        if (-1 != lastHandle) {
            int result = manager.turnOffPerformanceMode(lastHandle);
            LogUtils.d(TAG, "onReceive:tryTurnOnPerformanceMode: lastHandle = "
                    + lastHandle + " turn off result = " + result);
        }

        // turn on performance mode
        int currentHandle = manager.turnOnPerformanceMode();
        LogUtils.d(TAG, "onReceive:tryTurnOnPerformanceMode: currentHandle = "
                + currentHandle);
        if (-1 != currentHandle) {
            SharedPreferenceUtils.setSessionHandle(appContext, currentHandle);
        } else {
            SharedPreferenceUtils.setSessionHandle(appContext, -1);
        }
    }
}
