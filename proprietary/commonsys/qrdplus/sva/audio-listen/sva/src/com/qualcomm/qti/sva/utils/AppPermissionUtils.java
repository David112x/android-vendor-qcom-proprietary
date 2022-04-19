/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva.utils;

import android.Manifest;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;

import com.qualcomm.qti.sva.PermissionActivity;

import java.util.ArrayList;

public class AppPermissionUtils {
    public final static String[] APP_REQUIRED_PERMISSIONS = {
            Manifest.permission.READ_PHONE_STATE,
            Manifest.permission.RECORD_AUDIO,
            Manifest.permission.ACCESS_COARSE_LOCATION,
            Manifest.permission.ACCESS_FINE_LOCATION,
    };

    public static final String EXTRA_KEY_REQUIRED_PERMISSIONS = "required_permissions";

    private final static String TAG = AppPermissionUtils.class.getSimpleName();

    public static boolean isAllPermissionsGranted(Context context, String[] permissions) {
        String[] requestPermissions = checkPermissions(context, permissions);
        if (0 == requestPermissions.length) {
            return true;
        }

        Intent intent = new Intent();
        intent.setClass(context.getApplicationContext(), PermissionActivity.class);
        intent.putExtra(EXTRA_KEY_REQUIRED_PERMISSIONS, permissions);
        context.startActivity(intent);
        ((Activity) context).finish();
        return false;
    }

    public static String[] checkPermissions(Context context, String[] permissionArr) {
        boolean isPermissionGranted;

        ArrayList<String> permissionList = new ArrayList<>();
        for (String item : permissionArr) {
            isPermissionGranted = (PackageManager.PERMISSION_GRANTED ==
                    context.checkSelfPermission(item));
            if (!isPermissionGranted) {
                permissionList.add(item);
            }
        }

        String[] requestPermissions = new String[permissionList.size()];
        permissionList.toArray(requestPermissions);
        return requestPermissions;
    }
}
