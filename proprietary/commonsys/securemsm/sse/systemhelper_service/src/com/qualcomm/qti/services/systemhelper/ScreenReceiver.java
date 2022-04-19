/*
 * Copyright (c) 2014, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.services.systemhelper;

import android.app.KeyguardManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.PowerManager;
import android.util.Log;

/**
 * Intent receiver for the following:
 * ACTION_SCREEN_OFF
 * ACTION_SCREEN_ON
 * ACTION_USER_PRESENT --> lock screen passed
 * CAT_IDLE_SCREEN_ACTION --> back to home screen
 * */
public class ScreenReceiver extends BroadcastReceiver {

  /*
   * @see android.content.BroadcastReceiver#onReceive(android.content.Context,
   * android.content.Intent)
   */
  @Override
  public void onReceive(Context context, Intent intent) {
    String intentAction = intent.getAction();
    if (intentAction == null)
      return;
    KeyguardManager keyguard =
        (KeyguardManager)context.getSystemService(Context.KEYGUARD_SERVICE);
    if (intentAction.equals(Intent.ACTION_SCREEN_OFF)) {
      SysHelperService.sendEventNotification(SysHelperService.ACTION_SCREEN_OFF);
    } else if (intentAction.equals(Intent.ACTION_SCREEN_ON)) {
        SysHelperService.sendEventNotification(SysHelperService.ACTION_SCREEN_ON);
    } else if (intentAction.equals(Intent.ACTION_USER_PRESENT)) {
        SysHelperService.sendEventNotification(SysHelperService.ACTION_USER_PRESENT);
    }
  }

  public static boolean screenOn() {
    PowerManager powerManager =
        (PowerManager)SysHelperService.context.getSystemService(
            Context.POWER_SERVICE);
    KeyguardManager keyguard =
        (KeyguardManager)SysHelperService.context.getSystemService(
            Context.KEYGUARD_SERVICE);
    if (powerManager.isScreenOn()) {
      return true;
    }
    return false;
  }
}
