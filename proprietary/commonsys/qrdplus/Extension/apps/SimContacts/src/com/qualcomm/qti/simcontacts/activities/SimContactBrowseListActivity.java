/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
package com.qualcomm.qti.simcontacts.activities;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;

import com.qualcomm.qti.simcontacts.R;

public class SimContactBrowseListActivity extends AppCompatActivity {

    private SimContactBrowseListFragment mContactsBrowserListFragment;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.sim_conatct_activity);

        mContactsBrowserListFragment = (SimContactBrowseListFragment) getFragmentManager().
                findFragmentById(R.id.sim_contact_fragment);
    }

    @Override
    public void onBackPressed() {
        if (mContactsBrowserListFragment.isSelectionMode()) {
            mContactsBrowserListFragment.setSelectionMode(false);
            return;
        }
        super.onBackPressed();
    }
}
