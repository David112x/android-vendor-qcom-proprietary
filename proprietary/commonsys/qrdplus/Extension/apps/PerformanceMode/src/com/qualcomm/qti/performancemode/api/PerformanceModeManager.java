/**
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.performancemode.api;

import android.util.BoostFramework;
import com.qualcomm.qti.performancemode.utils.LogUtils;

public class PerformanceModeManager {
    private static final String TAG = PerformanceModeManager.class.getSimpleName();
    private static final PerformanceModeManager ourInstance = new PerformanceModeManager();
    private static final String PACKAGE_NAME = "com.qualcomm.qti.performancemode";

    private static final String PROP_PERFORMANCE_MODE_SUPPORT
            = "vendor.perf.performancemode.support";
    // PERFORMANCE_MODE_BOOST_ID should be equal to the corresponding value in BoostFramework.java
    // public static final int VENDOR_HINT_PERFORMANCE_MODE = 0x00001091;
    private static final int PERFORMANCE_MODE_BOOST_ID = 0x00001091;
    private BoostFramework mBoostFramework;

    public static PerformanceModeManager getInstance() {
        return ourInstance;
    }

    private PerformanceModeManager() {
        mBoostFramework = new BoostFramework();
    }

    public int turnOnPerformanceMode() {
        int handle;
        handle = mBoostFramework.perfHint(PERFORMANCE_MODE_BOOST_ID,
                PACKAGE_NAME, Integer.MAX_VALUE, -1);
        LogUtils.d(TAG, "turnOnPerformanceMode: turn on handle = "
                + handle);
        return handle;
    }

    public int turnOffPerformanceMode(int handle) {
        int ret = -1;
        LogUtils.d(TAG, "turnOffPerformanceMode: handle = " + handle);
        if (-1 != handle) {
            ret = mBoostFramework.perfLockReleaseHandler(handle);
            LogUtils.d(TAG, "turnOffPerformanceMode: turn off ret = " + ret);
            if (-1 != ret) {
                LogUtils.d(TAG, "turnOffPerformanceMode: turn off success");
            } else {
                LogUtils.e(TAG, "turnOffPerformanceMode: turn off failure");
            }
        }

        LogUtils.d(TAG, "turnOffPerformanceMode: return ret = " + ret);
        return ret;
    }

    public boolean isPerformanceModeSupport(){
        String support = mBoostFramework.perfGetProp(PROP_PERFORMANCE_MODE_SUPPORT,
                "0");
        return "1".equalsIgnoreCase(support) || "true".equalsIgnoreCase(support);
    }
}
