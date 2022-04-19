/*
  * Copyright (c) 2019 Qualcomm Technologies, Inc.
  * All Rights Reserved.
  * Confidential and Proprietary - Qualcomm Technologies, Inc.
  */

package com.qualcomm.qti.internal.telephony;

import android.content.Context;
import android.provider.Settings;
import android.provider.Settings.SettingNotFoundException;
import android.telephony.SubscriptionInfo;
import android.util.Log;
import com.android.internal.telephony.GlobalSettingsHelper;
import com.android.internal.telephony.MultiSimSettingController;
import com.android.internal.telephony.SubscriptionController;
import com.android.internal.telephony.Phone;
import com.android.internal.telephony.PhoneFactory;

import java.util.List;
import java.util.stream.Collectors;

/*
 * Extending QtiMultiSimSettingController to override default
 * behavior for mobile data
 */
public class QtiMultiSimSettingController extends MultiSimSettingController {

    private static final String LOG_TAG = "QtiMultiSimSettingController";

    public static MultiSimSettingController init(Context context, SubscriptionController sc) {
        synchronized (QtiMultiSimSettingController.class) {
            if (sInstance == null) {
                sInstance = new QtiMultiSimSettingController(context,
                        SubscriptionController.getInstance());
            } else {
                Log.wtf(LOG_TAG, "init() called multiple times!  sInstance = " + sInstance);
            }
        }
        return sInstance;
    }

    private QtiMultiSimSettingController(Context context, SubscriptionController sc) {
        super(context, sc);
    }

    public static QtiMultiSimSettingController getInstance() {
        return (QtiMultiSimSettingController)sInstance;
    }

    @Override
    protected void disableDataForNonDefaultNonOpportunisticSubscriptions() {
        log("disableDataForNonDefaultNonOpportunisticSubscriptions - do nothing");
    }

    protected synchronized void onUserDataEnabled(int subId, boolean enable) {
        log("onUserDataEnabled");
        // Make sure MOBILE_DATA of subscriptions in same group are synced.
        setUserDataEnabledForGroup(subId, enable);
    }

    /**
     * Make sure MOBILE_DATA of subscriptions in the same group with the subId
     * are synced.
     */
    @Override
    protected synchronized void setUserDataEnabledForGroup(int subId, boolean enable) {
        log("setUserDataEnabledForGroup subId " + subId + " enable " + enable);
        List<SubscriptionInfo> infoList = mSubController.getSubscriptionsInGroup(
                mSubController.getGroupUuid(subId), mContext.getOpPackageName());

        if (infoList == null) return;

        for (SubscriptionInfo info : infoList) {
            int currentSubId = info.getSubscriptionId();
            if (currentSubId == subId) continue;
            // TODO: simplify when setUserDataEnabled becomes singleton
            if (mSubController.isActiveSubId(currentSubId)) {
                // For active subscription, call setUserDataEnabled through DataEnabledSettings.
                Phone phone = PhoneFactory.getPhone(mSubController.getPhoneId(currentSubId));
                if (phone != null) {
                    phone.getDataEnabledSettings().setUserDataEnabled(enable);
                }
            } else {
                // For inactive subscription, directly write into global settings.
                GlobalSettingsHelper.setBoolean(
                        mContext, Settings.Global.MOBILE_DATA, currentSubId, enable);
            }
        }
   }

   @Override
   protected void updateDefaults() {
        log("updateDefaults");

        // FIXME re-work on this method as per latest changes in parent class
   }

    protected void log(String msg) {
        Log.d(LOG_TAG, msg);
    }
}
