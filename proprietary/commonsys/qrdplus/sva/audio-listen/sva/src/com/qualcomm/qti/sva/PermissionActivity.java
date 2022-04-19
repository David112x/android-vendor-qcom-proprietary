/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva;

import android.app.Activity;
import android.app.AlertDialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Bundle;
import android.provider.Settings;

import com.qualcomm.qti.sva.utils.AppPermissionUtils;
import com.qualcomm.qti.sva.utils.LogUtils;

public class PermissionActivity extends Activity {
    private static final String PACKAGE_URL_SCHEME = "package:";
    private static final int REQUEST_CODE_PERMISSION = 1000;

    private final String TAG = PermissionActivity.class.getSimpleName();

    private AlertDialog mMissingPermissionDialog;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        String[] requiredPermissions = getIntent().getStringArrayExtra(
                AppPermissionUtils.EXTRA_KEY_REQUIRED_PERMISSIONS);
        if (null == savedInstanceState && null != requiredPermissions) {
            String[] permissions = AppPermissionUtils.checkPermissions(this,
                    requiredPermissions);
            requestPermissions(permissions, REQUEST_CODE_PERMISSION);
        }
    }

    @Override
    protected void onDestroy() {
        if (null != mMissingPermissionDialog) {
            LogUtils.d(TAG, "onDestroy: dismiss mMissingPermissionDialog");
            mMissingPermissionDialog.dismiss();
        }
        super.onDestroy();
    }

    @Override
    public void onRequestPermissionsResult(int requestCode, String[] permissions,
                                           int[] grantResults) {
        boolean isAllPermissionGranted = true;
        switch (requestCode) {
            case REQUEST_CODE_PERMISSION:
                for (int result : grantResults) {
                    if (result != PackageManager.PERMISSION_GRANTED) {
                        isAllPermissionGranted = false;
                    }
                }
                break;
            default:
                super.onRequestPermissionsResult(requestCode, permissions, grantResults);
                break;
        }

        if (isAllPermissionGranted) {
            Intent intent = new Intent();
            intent.setClass(getApplicationContext(), MainActivity.class);
            startActivity(intent);
            finish();
        } else {
            mMissingPermissionDialog = showMissingPermissionDialog();
        }
    }

    private AlertDialog showMissingPermissionDialog() {
        AlertDialog.Builder builder = new AlertDialog.Builder(this);
        builder.setTitle(R.string.help);
        builder.setMessage(R.string.missing_permission_msg);
        builder.setNegativeButton(R.string.quit, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {
                finish();
            }
        });
        builder.setPositiveButton(R.string.settings, new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {
                Intent intent = new Intent(Settings.ACTION_APPLICATION_DETAILS_SETTINGS);
                intent.setData(Uri.parse(PACKAGE_URL_SCHEME + getPackageName()));
                startActivity(intent);
                finish();
            }
        });

        return builder.show();
    }
}
