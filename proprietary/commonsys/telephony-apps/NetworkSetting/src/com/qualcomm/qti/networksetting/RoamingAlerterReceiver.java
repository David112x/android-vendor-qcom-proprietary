/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.networksetting;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.os.PersistableBundle;
import android.telephony.CarrierConfigManager;
import android.telephony.SubscriptionManager;
import android.util.Log;

public class RoamingAlerterReceiver extends BroadcastReceiver {
    private static final String TAG = "NetworkSetting RoamingAlerterReceiver";
    private boolean mHasLaunched = false;

    @Override
    public void onReceive(Context context, Intent intent) {
        Log.d(TAG, "Received action: " + intent.getAction());
        if (!mHasLaunched && CarrierConfigManager.ACTION_CARRIER_CONFIG_CHANGED
                .equals(intent.getAction())) {
            final int subId = intent.getIntExtra(SubscriptionManager.EXTRA_SUBSCRIPTION_INDEX,
                    SubscriptionManager.INVALID_SUBSCRIPTION_ID);
            if (SubscriptionManager.isValidSubscriptionId(subId)) {
                final PersistableBundle carrierConfig = context.getSystemService(
                        CarrierConfigManager.class).getConfigForSubId(subId);
                // Enable this feature with global config from carrier config
                if (carrierConfig != null
                        && carrierConfig.getBoolean("config_enabled_roaming_alerting")){
                    mHasLaunched = true;
                    context.startService(new Intent(context, NetworkStatusService.class));
                    Log.d(TAG, "Starting NetworkStatusService");
                }
            }
       }
    }
}
