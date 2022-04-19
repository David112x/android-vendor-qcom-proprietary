/*
 * Copyright (c) 2014 - 2015, 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qualcomm.qti.services.secureui;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.util.Log;
import android.view.Window;
import android.view.WindowManager;
import android.view.Display;
import android.view.Surface;

public class OrientationActivity extends Activity {
  public static final String TAG = "SecUIOrientation";

  /** Called when activity is first created **/
  @Override
  public void onCreate(Bundle savedInstanceState) {
    Log.d(TAG, "onCreate+");
    super.onCreate(savedInstanceState);

    Display display = getWindowManager().getDefaultDisplay();
    int rotation = display.getRotation();
    switch (rotation) {
      case Surface.ROTATION_0: SecureUIService.setRotation(0x00); break;
      case Surface.ROTATION_90: SecureUIService.setRotation(0x10); break;
      case Surface.ROTATION_180: SecureUIService.setRotation(0x20); break;
      case Surface.ROTATION_270: SecureUIService.setRotation(0x30); break;
    }
    Log.d(TAG, "Final rotation: " + rotation);
    Log.d(TAG, "onCreate-");
  }

  @Override
  public void onNewIntent(Intent intent) {
    super.onNewIntent(intent);
  }

  @Override
  public void onPause() {
    Log.d(TAG, "onPause+");
    super.onPause();
    this.finish();
    Log.d(TAG, "onPause-");
  }

  @Override
  public void onResume() {
    Log.d(TAG, "onResume+");
    super.onResume();
    this.finish();
    Log.d(TAG, "onResume-");
  }

  @Override
  public void onDestroy() {
    Log.d(TAG, "onDestroy+");
    super.onDestroy();
  }
}
