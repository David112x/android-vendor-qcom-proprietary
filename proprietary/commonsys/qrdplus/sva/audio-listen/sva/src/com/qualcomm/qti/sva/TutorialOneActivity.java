/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

package com.qualcomm.qti.sva;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;

import com.qualcomm.qti.sva.utils.LogUtils;

public class TutorialOneActivity extends Activity {
    private final String TAG = TutorialOneActivity.class.getSimpleName();

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_tutorial_one);
    }

    public void onBackOrNextClicked(View view) {
        if (view.getId() == R.id.tv_back) {
            LogUtils.d(TAG, "OnBackOrNextClicked: back is clicked");
        } else if (view.getId() == R.id.tv_next) {
            LogUtils.d(TAG, "OnBackOrNextClicked: next is clicked");
            Intent intent = new Intent(TutorialOneActivity.this,
                    TutorialTwoActivity.class);
            startActivity(intent);
        } else {
            LogUtils.d(TAG, "OnBackOrNextClicked: unknown view is clicked");
        }

        finish();
    }
}
