/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.networksetting;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;
import android.util.Log;

public class NetworkStatusService extends Service {
    private static String LOG_TAG = NetworkStatusService.class.getSimpleName();
    private RoamingAlerter mRoamingAlerter;

    @Override
    public void onCreate() {
        Log.d(LOG_TAG, "onCreate...");
        mRoamingAlerter =  new RoamingAlerter(getApplicationContext());
        mRoamingAlerter.monitor();
    }

    @Override
    public IBinder onBind(Intent intent) {
        return null;
    }

    @Override
    public void onDestroy() {
        Log.d(LOG_TAG, "onDestroy...");
        if (mRoamingAlerter != null) {
            mRoamingAlerter.unmonitor();
        }
    }
}
