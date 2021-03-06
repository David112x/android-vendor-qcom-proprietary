/*******************************************************************************
@file    QcrilMsgTunnelAutoboot.java
@brief   Waits for boot complete intent and starts QcrilMsgTunnelService

---------------------------------------------------------------------------
Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
Qualcomm Technologies Proprietary and Confidential.
---------------------------------------------------------------------------
 ******************************************************************************/

package com.qualcomm.qcrilmsgtunnel;

import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.os.SystemProperties;
import android.util.Log;

public class QcrilMsgTunnelAutoboot extends BroadcastReceiver {

    private static final String TAG = "QcrilMsgTunnelAutoboot";

    @Override
    public void onReceive(Context context, Intent intent) {
        boolean canStartService = SystemProperties.getBoolean("persist.radio.msgtunnel.start",
                true);
        if (Intent.ACTION_LOCKED_BOOT_COMPLETED.equals(intent.getAction()) && canStartService) {
            ComponentName comp = new ComponentName(context.getPackageName(),
                    QcrilMsgTunnelService.class.getName());
            ComponentName service = context.startService(new Intent().setComponent(comp));
            if (service == null) {
                Log.e(TAG, "Could not start service " + comp.toString());
            } else {
                Log.d(TAG, comp.toString() + " started successfully");
            }
        } else if (Intent.ACTION_USER_INITIALIZE.equals(intent.getAction())) {
            if (android.os.Process.myUserHandle().isOwner()) {
                Log.d(TAG, "switched to primary user");
                Log.d(TAG, "userId of primary user = " + android.os.Process.myUid());
            }
            else {
               Log.d(TAG, "Secondary user , userId = " + android.os.Process.myUid());
            }
        } else {
            Log.e(TAG, "Received Intent: " + intent.toString() + " canStartService = "
                    + canStartService);
        }
    }
}
