/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.simcontacts.activities;

import android.os.Bundle;
import android.support.v7.app.AppCompatActivity;

import com.android.contacts.activities.RequestPermissionsActivity;
import com.qualcomm.qti.simcontacts.R;

public class ContactMultiSelectionActivity extends AppCompatActivity  {

    private ContactSelectionFragment mContactsFragment;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (RequestPermissionsActivity.startPermissionActivityIfNeeded(this)) {
            return;
        }
        setContentView(R.layout.contact_selection_activity);
        mContactsFragment = (ContactSelectionFragment) getFragmentManager().
                findFragmentById(R.id.multiple_selection_fragment);
    }

    @Override
    public void onBackPressed() {
        if (mContactsFragment.isSearchMode()) {
            mContactsFragment.setSearchMode(false);
        } else {
            super.onBackPressed();
        }
    }
}
