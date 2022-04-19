/**
 * Copyright (c) 2020, Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.carrierservice;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.telephony.TelephonyManager;
import android.util.Log;

/**
 * Receive ACTION_SIM_CARD_STATE_CHANGED broadcast and
 * start {@link CarrierConfigService}
 */
public class SimCardStatusChangeReceiver extends BroadcastReceiver {
    @Override
    public void onReceive(Context context, Intent intent) {
        if (TelephonyManager.ACTION_SIM_CARD_STATE_CHANGED.equals(intent.getAction())) {
            Log.d(CarrierConfigService.TAG, "onReceive ACTION_SIM_CARD_STATE_CHANGED");
        }
    }
}
