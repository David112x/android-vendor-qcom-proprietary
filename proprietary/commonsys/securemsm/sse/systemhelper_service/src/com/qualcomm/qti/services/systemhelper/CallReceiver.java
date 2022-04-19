/*
 * Copyright (c) 2014, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */


package com.qualcomm.qti.services.systemhelper;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.telephony.TelephonyManager;
import android.util.Log;

/**
 * Intent receiver for the following:
 * ACTION_PHONE_STATE_CHANGED
 * */
public class CallReceiver extends BroadcastReceiver {
  public static final String TAG = "SysHelperService.CallReceiver";

  @Override
  public void onReceive(Context context, Intent intent) {
    String action = intent.getAction();
    if ((action == null) ||
        (action.equals(TelephonyManager.ACTION_PHONE_STATE_CHANGED)) == false) {
      Log.w(TAG, "Unexpected intent");
      return;
    }
    String extra = intent.getStringExtra(TelephonyManager.EXTRA_STATE);
    if (extra == null) {
      Log.w(TAG, "No information attached to call intent");
      return;
    }

    if (extra.equals(TelephonyManager.EXTRA_STATE_RINGING)) {
      Log.d(TAG, "Ringing");
      SysHelperService.sendEventNotification(
          SysHelperService.PHONE_STATE_RINGING);
    } else if (extra.equals(TelephonyManager.EXTRA_STATE_OFFHOOK)) {
      Log.d(TAG, "Off Hook");
      SysHelperService.sendEventNotification(
          SysHelperService.PHONE_STATE_OFF_HOOK);
    } else if (extra.equals(TelephonyManager.EXTRA_STATE_IDLE)) {
      Log.d(TAG, "Phone Idle");
      SysHelperService.sendEventNotification(SysHelperService.PHONE_STATE_IDLE);
    } else {
      Log.w(TAG, "Unknown phone state: " + extra);
    }
  }

  public static boolean callActive() {
    TelephonyManager telephonyManager =
        (TelephonyManager)SysHelperService.context.getSystemService(
            SysHelperService.context.TELEPHONY_SERVICE);
    return (telephonyManager.getCallState() !=
            TelephonyManager.CALL_STATE_IDLE);
  }
}
