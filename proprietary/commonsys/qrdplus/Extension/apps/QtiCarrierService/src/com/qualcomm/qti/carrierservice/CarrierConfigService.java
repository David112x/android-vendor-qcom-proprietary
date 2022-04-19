/**
 * Copyright (c) 2020, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.carrierservice;

import android.app.IntentService;
import android.content.Context;
import android.content.Intent;
import android.os.IBinder;
import android.os.PowerManager;

import java.lang.ref.WeakReference;
import java.util.List;

public class CarrierConfigService extends IntentService {
    public static String TAG = CarrierConfigService.class.getSimpleName();

    public CarrierConfigService() {
        super("MyIntentService");
    }

    @Override
    protected void onHandleIntent(Intent intent) {

    }
}
