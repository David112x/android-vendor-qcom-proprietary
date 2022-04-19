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
import android.widget.RelativeLayout;
import android.widget.TextView;

import com.qualcomm.qti.sva.utils.LogUtils;

public class TutorialFourActivity extends Activity {
    private final String TAG = TutorialFourActivity.class.getSimpleName();

    private RelativeLayout mLayoutBottomBar;
    private TextView mTvNext;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        LogUtils.d(TAG, "onCreate: enter");
        setContentView(R.layout.activity_tutorial_four);

        mLayoutBottomBar = findViewById(R.id.bottom_bar);
        mTvNext = mLayoutBottomBar.findViewById(R.id.tv_next);
        mTvNext.setText(R.string.done);
    }

    public void onBackOrNextClicked(View view) {
        if (view.getId() == R.id.tv_back) {
            LogUtils.d(TAG, "OnBackOrNextClicked: back is clicked");
            Intent intent = new Intent(TutorialFourActivity.this,
                    TutorialThreeActivity.class);
            startActivity(intent);
        } else if (view.getId() == R.id.tv_next) {
            LogUtils.d(TAG, "OnBackOrNextClicked: next is clicked");
            Intent intent = new Intent(TutorialFourActivity.this,
                    MainActivity.class);
            intent.setFlags(Intent.FLAG_ACTIVITY_CLEAR_TOP | Intent.FLAG_ACTIVITY_CLEAR_TASK);
            startActivity(intent);
        } else {
            LogUtils.d(TAG, "OnBackOrNextClicked: unknown view is clicked");
        }

        finish();
    }
}
