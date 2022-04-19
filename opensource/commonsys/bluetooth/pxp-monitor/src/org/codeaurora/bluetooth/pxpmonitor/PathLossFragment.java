/*
 * Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *        * Redistributions of source code must retain the above copyright
 *            notice, this list of conditions and the following disclaimer.
 *        * Redistributions in binary form must reproduce the above copyright
 *            notice, this list of conditions and the following disclaimer in the
 *            documentation and/or other materials provided with the distribution.
 *        * Neither the name of The Linux Foundation nor
 *            the names of its contributors may be used to endorse or promote
 *            products derived from this software without specific prior written
 *            permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NON-INFRINGEMENT ARE DISCLAIMED.    IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

package org.codeaurora.bluetooth.pxpmonitor;

import android.app.Activity;
import android.app.Fragment;
import android.content.Context;
import android.os.Bundle;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.view.inputmethod.EditorInfo;
import android.view.inputmethod.InputMethodManager;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;
import android.widget.ToggleButton;

public class PathLossFragment extends Fragment implements View.OnClickListener {

    private static final String TAG = PathLossFragment.class.getSimpleName();

    private static final short TX_POWER_MIN = -100;

    private static final short TX_POWER_MAX = 20;

    private static final short RSSI_MIN = -127;

    private static final short RSSI_MAX = 20;

    private static final short THRESHOLD_MIN = TX_POWER_MIN - RSSI_MAX; // -120

    private static final short THRESHOLD_MAX = TX_POWER_MAX - RSSI_MIN; // 147

    private DeviceActivity mActivity;

    private ToggleButton mPathLossButtons[] = new ToggleButton[3];

    private EditText mMinValue;

    private EditText mMaxValue;

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.path_loss_fragment, null, false);

        mPathLossButtons[0] = (ToggleButton) view.findViewById(R.id.path_off);
        mPathLossButtons[1] = (ToggleButton) view.findViewById(R.id.path_mild);
        mPathLossButtons[2] = (ToggleButton) view.findViewById(R.id.path_high);
        mMinValue = (EditText) view.findViewById(R.id.threshold_min_value);
        mMaxValue = (EditText) view.findViewById(R.id.threshold_max_value);

        mMinValue.setOnEditorActionListener(new EditText.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {

                CharSequence text = null;

                int threshold = Integer.parseInt(mMinValue.getText().toString());

                if (threshold > THRESHOLD_MAX) {

                    text = "Threshold is too big. Changing to "
                            + Integer.toString(THRESHOLD_MAX);

                    threshold = THRESHOLD_MAX;
                    mMinValue.setText(String.valueOf(threshold));

                } else if (threshold < THRESHOLD_MIN) {

                    text = "Threshold is too small. Changing to "
                            + Integer.toString(THRESHOLD_MIN);

                    threshold = THRESHOLD_MIN;
                    mMinValue.setText(String.valueOf(threshold));
                }

                if (text != null) {
                    Context context = getActivity();

                    Toast toast = Toast.makeText(context, text, Toast.LENGTH_SHORT);
                    toast.setGravity(Gravity.CENTER, 0, 0);
                    toast.show();
                }

                mActivity.mPxpServiceProxy.setMinPathLossThreshold(mActivity.mLeDevice,
                        threshold);

                final InputMethodManager imm = (InputMethodManager) getActivity()
                        .getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.hideSoftInputFromWindow(getView().getWindowToken(), 0);

                return true;
            }
        });

        mMaxValue.setOnEditorActionListener(new EditText.OnEditorActionListener() {
            @Override
            public boolean onEditorAction(TextView v, int actionId, KeyEvent event) {

                CharSequence text = null;

                int threshold = Integer.parseInt(mMaxValue.getText().toString());

                if (threshold > THRESHOLD_MAX) {

                    text = "Threshold is too big. Changing to "
                            + Integer.toString(THRESHOLD_MAX);

                    threshold = THRESHOLD_MAX;
                    mMaxValue.setText(String.valueOf(threshold));

                } else if (threshold < THRESHOLD_MIN) {

                    text = "Threshold is too small. Changing to "
                            + Integer.toString(THRESHOLD_MIN);

                    threshold = THRESHOLD_MIN;
                    mMaxValue.setText(String.valueOf(threshold));
                }

                if (text != null) {
                    Context context = getActivity();

                    Toast toast = Toast.makeText(context, text, Toast.LENGTH_SHORT);
                    toast.setGravity(Gravity.CENTER, 0, 0);
                    toast.show();
                }

                mActivity.mPxpServiceProxy.setMaxPathLossThreshold(mActivity.mLeDevice,
                        threshold);

                final InputMethodManager imm = (InputMethodManager) getActivity()
                        .getSystemService(Context.INPUT_METHOD_SERVICE);
                imm.hideSoftInputFromWindow(getView().getWindowToken(), 0);

                return true;
            }
        });

        for (ToggleButton btn : mPathLossButtons) {
            btn.setEnabled(false);
            btn.setOnClickListener(this);
        }

        return view;
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);

        try {
            mActivity = (DeviceActivity) activity;
        } catch (ClassCastException e) {
            throw new ClassCastException(this.getClass().getSimpleName()
                    + " can be only attached to " + MainActivity.class.getSimpleName());
        }
    }

    public void init() {

        for (ToggleButton btn : mPathLossButtons) {
            btn.setEnabled(false);
        }

        mMinValue.setEnabled(false);
        mMaxValue.setEnabled(false);
    }

    public void onServiceFound(boolean llsService, boolean tpsService) {

        Log.v(TAG, "IAS service = " + llsService + " TPS service = " + tpsService);

        if (mActivity.mPxpServiceProxy.checkFailedReadTxPowerLevel(mActivity.mLeDevice)) {
            Context context = getActivity();
            CharSequence text = "TX Power level not found";

            Toast toast = Toast.makeText(context, text, Toast.LENGTH_SHORT);
            toast.setGravity(Gravity.CENTER, 0, 0);
            toast.show();

        } else if (llsService && tpsService) {

            mPathLossButtons[0].setEnabled(true);
            mPathLossButtons[1].setEnabled(true);
            mPathLossButtons[2].setEnabled(true);
            mMinValue.setEnabled(true);
            mMaxValue.setEnabled(true);

            int pathLossLevel;

            pathLossLevel = mActivity.mPxpServiceProxy
                    .getPathLossAlertLevel(mActivity.mLeDevice);

            for (int i = 0; i < mPathLossButtons.length; i++) {
                mPathLossButtons[i].setChecked(pathLossLevel == i);
            }

            int minThresholdValue;
            int maxThresholdValue;

            minThresholdValue = mActivity.mPxpServiceProxy
                    .getMinPathLossThreshold(mActivity.mLeDevice);

            mMinValue.setText(String.format("%d", minThresholdValue));

            maxThresholdValue = mActivity.mPxpServiceProxy
                    .getMaxPathLossThreshold(mActivity.mLeDevice);

            mMaxValue.setText(String.format("%d", maxThresholdValue));

        } else {
            init();
        }

    }

    @Override
    public void onClick(View v) {
        ToggleButton clickedButton = (ToggleButton) v;

        if (!mActivity.mPxpServiceProxy.getConnectionState(mActivity.mLeDevice)) {
            return;
        }

        int idx;

        switch (clickedButton.getId()) {
            case R.id.path_off:
                idx = 0;
                break;

            case R.id.path_mild:
                idx = 1;
                break;

            case R.id.path_high:
                idx = 2;
                break;

            default:
                return;
        }

        for (int i = 0; i < mPathLossButtons.length; i++) {
            ToggleButton btn = mPathLossButtons[i];

            if (mPathLossButtons[idx].isChecked()) {

                if (i != idx) {
                    btn.setChecked(false);
                }

            } else {
                btn.setChecked(i == 0);
                idx = 0;
            }
        }

        mActivity.mPxpServiceProxy.setPathLossAlertLevel(mActivity.mLeDevice, idx);
    }
}
