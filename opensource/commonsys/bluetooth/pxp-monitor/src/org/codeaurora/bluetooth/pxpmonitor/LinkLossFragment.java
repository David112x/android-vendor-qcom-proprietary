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
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Toast;
import android.widget.ToggleButton;

public class LinkLossFragment extends Fragment implements View.OnClickListener {

    private static final String TAG = LinkLossFragment.class.getSimpleName();

    private DeviceActivity mActivity;

    private ToggleButton mLinkLossButtons[] = new ToggleButton[3];

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
            Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.link_loss_fragment, container, false);

        mLinkLossButtons[0] = (ToggleButton) view.findViewById(R.id.link_off);
        mLinkLossButtons[1] = (ToggleButton) view.findViewById(R.id.link_mild);
        mLinkLossButtons[2] = (ToggleButton) view.findViewById(R.id.link_high);

        for (ToggleButton btn : mLinkLossButtons) {
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

        for (ToggleButton btn : mLinkLossButtons) {
            btn.setEnabled(false);
        }
    }

    public void onServiceFound(boolean state) {
        Log.d(TAG, "LLS service" + state);

        if (state) {
            mLinkLossButtons[0].setEnabled(true);
            mLinkLossButtons[1].setEnabled(true);
            mLinkLossButtons[2].setEnabled(true);

            int linkLossLevel = 0;

            if (mActivity.mPxpServiceProxy.getLinkLossAlertLevel(mActivity.mLeDevice) < 0) {
                Log.w(TAG, "LLS Alert level not yet read");

            } else {
                linkLossLevel = mActivity.mPxpServiceProxy.
                        getLinkLossAlertLevel(mActivity.mLeDevice);
            }

            for (int i = 0; i < mLinkLossButtons.length; i++) {
                mLinkLossButtons[i].setChecked(linkLossLevel == i);
            }

        } else {
            init();
            Context context = getActivity();
            CharSequence text = "Link Loss Service not found";

            Toast toast = Toast.makeText(context, text, Toast.LENGTH_SHORT);
            toast.setGravity(Gravity.CENTER, 0, 0);
            toast.show();
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
            case R.id.link_off:
                idx = 0;
                break;

            case R.id.link_mild:
                idx = 1;
                break;

            case R.id.link_high:
                idx = 2;
                break;

            default:
                return;
        }

        for (int i = 0; i < mLinkLossButtons.length; i++) {
            ToggleButton btn = mLinkLossButtons[i];

            if (mLinkLossButtons[idx].isChecked()) {

                if (i != idx) {
                    btn.setChecked(false);
                }

            } else {
                btn.setChecked(i == 0);
                idx = 0;
            }
        }

        mActivity.mPxpServiceProxy.setLinkLossAlertLevel(mActivity.mLeDevice, idx);
    }
}
