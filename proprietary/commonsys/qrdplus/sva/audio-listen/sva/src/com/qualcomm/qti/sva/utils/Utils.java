/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva.utils;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.Context;
import android.os.Build;

import com.qualcomm.qti.sva.R;

public class Utils {
    private static final String TAG = Utils.class.getSimpleName();

    public static boolean isAtLeastN() {
        return Build.VERSION.SDK_INT >= 24;
    }

    public static void openAlertDialog(Context context, String title, String message) {
        if (null == context || null == message) {
            LogUtils.e(TAG, "openAlertDialog: invalid input params");
            return;
        }

        AlertDialog.Builder builder = new AlertDialog.Builder(context);
        if (null == title) {
            title = context.getResources().getString(R.string.app_name);
        }
        LogUtils.d(TAG, "openAlertDialog: title= " + title);
        LogUtils.d(TAG, "openAlertDialog: message= " + message);
        builder.setTitle(title)
                .setMessage(message)
                .setCancelable(true)
                .setNegativeButton(R.string.cancel, null);

        if (!((Activity) context).isFinishing()) {
            builder.show();
        }
    }
}
