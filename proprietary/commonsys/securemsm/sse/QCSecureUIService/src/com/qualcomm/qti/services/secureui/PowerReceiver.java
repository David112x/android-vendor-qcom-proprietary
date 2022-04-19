/*
 * Copyright (c) 2014, 2019 Qualcomm Technologies, Inc.  All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

package com.qualcomm.qti.services.secureui;

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
        int ret;
        if ((intentAction != null) &&
              (intentAction.equals(Intent.ACTION_SHUTDOWN))) {
            ret = SecureUIService.sendNotification(SecureUIService.SUI_MSG_ID_ABORT, SecureUIService.SUI_MSG_RET_NOP, SecureUIService.TOUCH_LIB_ADDR);
            if (0 != ret)
                SecureUIService.sendNotification(SecureUIService.SUI_MSG_ID_ABORT, SecureUIService.SUI_MSG_RET_NOP, null);
        }
    }

}
