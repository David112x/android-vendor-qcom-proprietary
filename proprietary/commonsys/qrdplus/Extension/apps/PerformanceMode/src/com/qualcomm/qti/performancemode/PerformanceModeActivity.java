/**
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.performancemode;

import android.app.ActionBar;
import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import com.qualcomm.qti.performancemode.api.PerformanceModeManager;
import com.qualcomm.qti.performancemode.utils.LogUtils;
import com.qualcomm.qti.performancemode.utils.SharedPreferenceUtils;

public class PerformanceModeActivity extends Activity implements View.OnClickListener {
    private static final String TAG = PerformanceModeActivity.class.getSimpleName();
    private boolean mIsPerformanceModeOn;
    private Button mButtonOn;
    private Button mButtonOff;
    private Context mAppContext;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_performance_mode);

        mAppContext = getApplicationContext();
        final ActionBar actionBar = getActionBar();
        if (actionBar != null) {
            actionBar.setDisplayHomeAsUpEnabled(true);
        }

        mButtonOn = findViewById(R.id.button_on);
        mButtonOn.setOnClickListener(this);
        mButtonOff = findViewById(R.id.button_off);
        mButtonOff.setOnClickListener(this);

        mIsPerformanceModeOn = SharedPreferenceUtils.getPerformanceModeStatus(mAppContext);
        setVisibility(mIsPerformanceModeOn);

        // if the performance mode feature is not supported, disable two buttons
        if (!PerformanceModeManager.getInstance().isPerformanceModeSupport()) {
            LogUtils.d(TAG, "onCreate: button is disabled");
            mButtonOn.setEnabled(false);
            mButtonOff.setEnabled(false);
        } else {
            // only turn on when user launches activity before received boot completed receiver.
            tryTurnOnPerformanceMode(mAppContext);
        }
    }

    @Override
    public boolean onNavigateUp() {
        if (!super.onNavigateUp()) {
            finish();
        }
        return true;
    }

    @Override
    public void onClick(View v) {
        final boolean turnOn = (v.getId() == R.id.button_on);
        if (turnOn) {
            // Do the turn on performance mode operation
            int handle = PerformanceModeManager.getInstance().turnOnPerformanceMode();
            if (-1 != handle) {
                setVisibility(true);
                SharedPreferenceUtils.setSessionHandle(mAppContext, handle);
                SharedPreferenceUtils.setPerformanceModeStatus(mAppContext, true);
            } else {
                Toast.makeText(mAppContext, R.string.turn_on_fail, Toast.LENGTH_SHORT).show();
            }
        } else {
            // Do the turn off performance mode operation
            int handle = SharedPreferenceUtils.getSessionHandle(mAppContext);
            if (-1 != handle) {
                int ret = PerformanceModeManager.getInstance().turnOffPerformanceMode(handle);
                if (-1 != ret) {
                    setVisibility(false);
                    SharedPreferenceUtils.setSessionHandle(mAppContext, -1);
                    SharedPreferenceUtils.setPerformanceModeStatus(mAppContext,
                            false);
                } else {
                    Toast.makeText(mAppContext, R.string.turn_off_fail,
                            Toast.LENGTH_SHORT).show();
                }
            } else {
                Toast.makeText(mAppContext, R.string.turn_off_invalid_handle,
                        Toast.LENGTH_SHORT).show();
            }
        }
    }

    public boolean isPerformanceModeOn() {
        return mIsPerformanceModeOn;
    }

    public void setVisibility(boolean isOn) {
        mIsPerformanceModeOn = isOn;
        if (isOn) {
            mButtonOn.setVisibility(View.GONE);
            mButtonOff.setVisibility(View.VISIBLE);
        } else {
            mButtonOn.setVisibility(View.VISIBLE);
            mButtonOff.setVisibility(View.GONE);
        }
    }

    private void tryTurnOnPerformanceMode(Context appContext) {
        int lastHandle = SharedPreferenceUtils.getSessionHandle(appContext);
        if (isPerformanceModeOn() && -1 == lastHandle) {
            /* User launch PerformanceModeActivity before this App
             * received boot completed receiver.
             */
            int handle = PerformanceModeManager.getInstance().turnOnPerformanceMode();
            LogUtils.d(TAG, "tryTurnOnPerformanceMode: turn on result handle = " + handle);
            if (-1 != handle) {
                SharedPreferenceUtils.setSessionHandle(appContext, handle);
            }
        }
    }
}
