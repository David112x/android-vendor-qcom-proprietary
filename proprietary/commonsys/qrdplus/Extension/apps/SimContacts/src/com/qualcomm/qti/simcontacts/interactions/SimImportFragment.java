/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
package com.qualcomm.qti.simcontacts.interactions;

import android.app.Activity;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.support.v4.os.ResultReceiver;

import com.qualcomm.qti.simcontacts.ContactSaveService;
import com.qualcomm.qti.simcontacts.R;
import com.qualcomm.qti.simcontacts.activities.ContactMultiSelectionActivity;

public class SimImportFragment extends Fragment {
    private static final String TAG = "SimExport";
    private static final String RESULT_KEY = "result";
    private int mSubscriptionId;
    private ProgressDialog mProgressDialog;

    public static SimImportFragment start(int subscriptionId, Fragment host){
        final FragmentManager fragmentManager = host.getFragmentManager();
        SimImportFragment fragment = (SimImportFragment) fragmentManager.findFragmentByTag(TAG);
        if (fragment == null) {
            fragment = new SimImportFragment();
            fragment.setSubscriptionId(subscriptionId);
            fragmentManager.beginTransaction().add(fragment, TAG)
                    .commitAllowingStateLoss();
        } else {
            fragment.setSubscriptionId(subscriptionId);
        }
        return fragment;
    }

    @Override
    public void onAttach(Context context){
        super.onAttach(context);
        startContactSelectionIntent();
    }

    @Override
    public void onDetach(){
        super.onDetach();
    }

    private void setSubscriptionId(int subscriptionId){
        mSubscriptionId = subscriptionId;
        if (isAdded()){
            startContactSelectionIntent();
        }
    }

    private void startContactSelectionIntent(){
        Intent intent = new Intent(getActivity(), ContactMultiSelectionActivity.class);
        intent.putExtra("importSubId", mSubscriptionId);
        startActivityForResult(intent, Activity.RESULT_FIRST_USER);
    }

    @Override
    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (resultCode != Activity.RESULT_OK) {
            return;
        }
        Bundle contacts = data.getBundleExtra(RESULT_KEY);
        if (contacts != null){
            startSimImportIntent(contacts);
        }
    }

    private void startSimImportIntent(Bundle contacts){
        ResultReceiver receiver = new ResultReceiver(new Handler()){
            @Override
            protected void onReceiveResult(int resultCode, Bundle resultData) {
                super.onReceiveResult(resultCode, resultData);
                switch (resultCode){
                    case ContactSaveService.CONTACTS_IMPORT_STARTED:
                        showImportProgressDialog();
                        break;
                    case ContactSaveService.CONTACTS_IMPORT_COMPLETE:
                        if (mProgressDialog != null){
                            mProgressDialog.dismiss();
                            mProgressDialog = null;
                        }
                        break;
                }
            }
        };
        getActivity().startService(ContactSaveService.createSimImportIntent(getActivity(),
                contacts, mSubscriptionId, receiver));
    }

    public void showImportProgressDialog(){
        mProgressDialog = new ProgressDialog(getActivity());
        mProgressDialog.setTitle(R.string.import_contacts_title);
        mProgressDialog.setMessage(getString(R.string.importing));
        mProgressDialog.setCanceledOnTouchOutside(false);
        mProgressDialog.show();
    }

    @Override
    public void onDestroy(){
        super.onDestroy();
        if (mProgressDialog != null){
            mProgressDialog.dismiss();
            mProgressDialog = null;
        }
    }
}
