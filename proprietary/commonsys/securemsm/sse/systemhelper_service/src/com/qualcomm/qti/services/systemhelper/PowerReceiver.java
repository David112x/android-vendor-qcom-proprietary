/*
 * Copyright (c) 2014, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.services.systemhelper;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;

/**
 * Intent receiver for the following:
 * ACTION_SHUTDOWN
 * */
public class PowerReceiver extends BroadcastReceiver {

    /*
     * @see android.content.BroadcastReceiver#onReceive(android.content.Context, android.content.Intent)
     */
    @Override
    public void onReceive(Context context, Intent intent) {
        String intentAction = intent.getAction();
        if ((intentAction != null) &&
              (intentAction.equals(Intent.ACTION_SHUTDOWN))) {
                SysHelperService.sendEventNotification(SysHelperService.ACTION_SHUTDOWN);
        }
    }

}
