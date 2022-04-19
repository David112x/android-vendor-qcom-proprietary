/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva;

import android.app.Application;
import com.qualcomm.qti.sva.controller.Global;

public class SvaApplication extends Application {

    @Override
    public void onCreate() {
        super.onCreate();
        Global.register(getApplicationContext());
    }
}
