/*
 * Copyright (c) 2016, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.callfeaturessetting.cdma.CdmaCallForwardWaiting;

import android.Manifest;
import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.ComponentName;
import android.content.Context;
import android.content.res.Configuration;
import android.content.res.Resources;
import android.net.Uri;
import android.provider.Settings;
import android.telecom.PhoneAccountHandle;
import android.telecom.TelecomManager;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.text.TextUtils;
import android.util.DisplayMetrics;
import android.util.Log;

import com.android.internal.telephony.CommandsInterface;
import com.android.internal.telephony.PhoneFactory;

import java.util.List;
import com.qualcomm.qti.callfeaturessetting.R;

public class GlobalUtils {
    public static final String LOG_TAG = "GlobalUtils";
    private static final String PACKAGE_URL_SCHEME = "package:";
    public static final int CALL_WAITING = 7;
    private static final int ACTIVATE = 0;
    private static final int DEACTIVATE = 1;
    public static final int REQUEST_PHONE_PERMISSIONS = 0;
    public final static String[] REQUIRED_PERMISSIONS = {
        Manifest.permission.CALL_PHONE,
        Manifest.permission.READ_PHONE_STATE
    };

    public static PhoneAccountHandle getPhoneAccountHandle(Context context, int subId) {
        SubscriptionInfo subInfo = null;
        try {
            SubscriptionManager subMgr = (SubscriptionManager) context
                .getSystemService(Context.TELEPHONY_SUBSCRIPTION_SERVICE);
            subInfo = subMgr.getActiveSubscriptionInfo(subId);
        } catch (Exception e) {
            Log.e(LOG_TAG, "Exception: " + e);
            return null;
        }
        String iccId = null;
        if (subInfo != null ) {
            iccId = subInfo.getIccId();
        }
        TelecomManager telecomManager = (TelecomManager) context.
            getSystemService(Context.TELECOM_SERVICE);
        List<PhoneAccountHandle> accounts = telecomManager.getCallCapablePhoneAccounts();
        if(iccId != null) {
            for (PhoneAccountHandle account : accounts) {
                if (iccId.equals(account.getId())) {
                    return account;
                }
            }
        }
        return null;
    }

    private static int getResId(int type, int state) {
        if (state == ACTIVATE) {
            switch (type) {
                case CommandsInterface.CF_REASON_UNCONDITIONAL:
                    return R.string.CFU_ACT;
                case CommandsInterface.CF_REASON_BUSY:
                    return R.string.CFB_ACT;
                case CommandsInterface.CF_REASON_NO_REPLY:
                    return R.string.CFNRY_ACT;
                case CommandsInterface.CF_REASON_NOT_REACHABLE:
                    return R.string.CFNRC_ACT;
                case CALL_WAITING:
                    return R.string.CW_ACT;
                default:
                    return R.string.CFU_ACT;
            }
        } else {
            switch (type) {
                case CommandsInterface.CF_REASON_UNCONDITIONAL:
                    return R.string.CFU_DEACT;
                case CommandsInterface.CF_REASON_BUSY:
                    return R.string.CFB_DEACT;
                case CommandsInterface.CF_REASON_NO_REPLY:
                    return R.string.CFNRY_DEACT;
                case CommandsInterface.CF_REASON_NOT_REACHABLE:
                    return R.string.CFNRC_DEACT;
                case CommandsInterface.CF_REASON_ALL:
                    return R.string.CFA_DEACT;
                case CALL_WAITING:
                    return R.string.CW_DEACT;
                default:
                    return R.string.CFU_DEACT;
            }
        }
    }

    public static String getActivateNumber(Context context, int subId, int type) {
        Resources subRes = getResPerSubId(context, subId);
        if (subRes != null)
            return subRes.getString(getResId(type, ACTIVATE));
        else
            return null;
    }

    public static String getDeactivateNumber(Context context, int subId, int type) {
        Resources subRes = getResPerSubId(context, subId);
        if (subRes != null)
            return subRes.getString(getResId(type, DEACTIVATE));
        else
            return null;
    }

    private static Resources getResPerSubId(Context context, int subId) {
        SubscriptionInfo subInfo = null;
        try {
            SubscriptionManager subMgr = (SubscriptionManager) context
                .getSystemService(Context.TELEPHONY_SUBSCRIPTION_SERVICE);
            subInfo = subMgr.getActiveSubscriptionInfo(subId);
        } catch (Exception e) {
            Log.e(LOG_TAG, "Exception: " + e);
            return null;
        }
        Configuration config = context.getResources().getConfiguration();
        Configuration newConfig = new Configuration();
        newConfig.setTo(config);
        if (subInfo != null) {
            newConfig.mcc = subInfo.getMcc();
            newConfig.mnc = subInfo.getMnc();
            if (newConfig.mnc == 0) {
                newConfig.mnc = Configuration.MNC_ZERO;
            }
        }
        DisplayMetrics metrics = context.getResources().getDisplayMetrics();
        DisplayMetrics newMetrics = new DisplayMetrics();
        newMetrics.setTo(metrics);
        return new Resources(context.getResources().getAssets(), newMetrics, newConfig);
    }

    public static AlertDialog showMissingPermissionDialog(Context context) {
        AlertDialog.Builder builder = new AlertDialog.Builder(context);
        builder.setTitle(R.string.help);
        builder.setMessage(R.string.missing_permission_msg);
        builder.setNegativeButton(R.string.quit, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {
                ((Activity) context).finish();
            }
        });
        builder.setPositiveButton(R.string.settings, new DialogInterface.OnClickListener() {
             @Override
             public void onClick(DialogInterface dialogInterface, int i) {
                Intent intent = new Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
                intent.setData(Uri.parse(PACKAGE_URL_SCHEME + context.getPackageName()));
                context.startActivity(intent);
                ((Activity) context).finish();
             }
        });
        return builder.show();
    }

    public static void requestPhonePermission(Context context) {
        Log.i(LOG_TAG, "Phone permission has NOT been granted. Requesting permission.");
        if (((Activity) context).shouldShowRequestPermissionRationale
                (Manifest.permission.READ_PHONE_STATE)
                || ((Activity) context).shouldShowRequestPermissionRationale
                (Manifest.permission.CALL_PHONE)) {
            // Provide an additional rationale to the user if the permission was not granted
            // and the user would benefit from additional context for the use of the permission.
            // For example if the user has previously denied the permission.
            showMissingPermissionDialog(context);
        } else {
            // Phone permission has not been granted yet. Request it directly.
            ((Activity) context)
                .requestPermissions(REQUIRED_PERMISSIONS, REQUEST_PHONE_PERMISSIONS);
        }
    }
}

