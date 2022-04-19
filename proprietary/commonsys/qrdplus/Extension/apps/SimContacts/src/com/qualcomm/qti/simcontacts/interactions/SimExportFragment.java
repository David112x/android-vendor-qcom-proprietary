/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
package com.qualcomm.qti.simcontacts.interactions;

import android.app.AlertDialog;
import android.app.Fragment;
import android.app.FragmentManager;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Handler;
import android.support.v4.os.ResultReceiver;
import android.util.Log;
import android.widget.Toast;

import com.android.contacts.model.SimContact;
import com.qualcomm.qti.simcontacts.ContactSaveService;
import com.qualcomm.qti.simcontacts.R;

import java.util.ArrayList;

public class SimExportFragment extends Fragment {

    private static final String TAG = "SimContactsExport";
    private ArrayList<SimContact> mSimContacts = new ArrayList<>();
    private AlertDialog mDialog;
    private Listener mListener;
    private ProgressDialog mProgressDialog;

    public static interface Listener{
        void onExportContactsStarted();
    }

    public static SimExportFragment start(ArrayList<SimContact> simContacts, Fragment host){
        final FragmentManager fragmentManager = host.getFragmentManager();
        SimExportFragment fragment = (SimExportFragment) fragmentManager.findFragmentByTag(TAG);
        if (fragment == null) {
            fragment = new SimExportFragment();
            fragment.setSelectedContacts(simContacts);
            fragmentManager.beginTransaction().add(fragment, TAG)
                    .commitAllowingStateLoss();
        } else {
            fragment.setSelectedContacts(simContacts);
        }
        return fragment;
    }

    public void setMultiExportListener(Listener listener){
        mListener = listener;
    }

    @Override
    public void onAttach(Context context){
        super.onAttach(context);
        showExportConfirmationDialog(mSimContacts);
    }

    @Override
    public void onDestroyView(){
        super.onDestroyView();
        if (mDialog != null){
            mDialog.dismiss();
            mDialog = null;
        }
    }

    private void setSelectedContacts(ArrayList<SimContact> simContacts){
        mSimContacts = simContacts;
        if (isAdded()){
            showExportConfirmationDialog(mSimContacts);
        }
    }

    private void showExportConfirmationDialog(final ArrayList<SimContact> simContacts){
        if (simContacts == null || simContacts.size() == 0){
            Log.e(TAG, "invalid request since no selected contact");
            return;
        }

        int messageId;
        if (simContacts.size() > 1){
            messageId = R.string.export_multiple_contacts_confirmation;
        }else {
            messageId = R.string.export_single_contact_confirmation;
        }

        mDialog = new AlertDialog.Builder(getActivity())
                .setIconAttribute(android.R.attr.alertDialogIcon)
                .setMessage(messageId)
                .setNegativeButton(android.R.string.cancel, null)
                .setPositiveButton(R.string.export_positive_button_Confirmation,
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int whichButton) {
                                //Export selected contacts to Device.
                                if (mListener != null){
                                    mListener.onExportContactsStarted();
                                }
                                startSimExportIntent(simContacts);
                            }
                        }
                )
                .create();

        mDialog.setOnDismissListener(new DialogInterface.OnDismissListener() {
            @Override
            public void onDismiss(DialogInterface dialog) {
                mDialog = null;
            }
        });
        mDialog.show();
    }

    private void startSimExportIntent(final ArrayList<SimContact> simContacts){
        ResultReceiver receiver = new ResultReceiver(new Handler()){
            @Override
            protected void onReceiveResult(int resultCode, Bundle resultData) {
                super.onReceiveResult(resultCode, resultData);
                switch (resultCode){
                    case ContactSaveService.CONTACTS_EXPORT_STARTED:
                        showExportProgressDialog();
                        break;
                    case ContactSaveService.CONTACTS_EXPORT_COMPLETE:
                        dismissProgressDialog();
                        showToastFeedback(getResources().getQuantityString(
                                R.plurals.sim_export_success_toast_fmt, simContacts.size(),
                                simContacts.size()));
                        break;
                    case ContactSaveService.CONTACTS_EXPORT_FAILURE:
                        dismissProgressDialog();
                        showToastFeedback(getString(R.string.sim_export_failed_toast));
                        break;
                }
            }
        };
        getActivity().startService(ContactSaveService.createSimExportIntent(getActivity(),
                simContacts, receiver));
    }

    public void showExportProgressDialog(){
        mProgressDialog = new ProgressDialog(getActivity());
        mProgressDialog.setTitle(R.string.export_contacts_title);
        mProgressDialog.setMessage(getString(R.string.exporting));
        mProgressDialog.setCanceledOnTouchOutside(false);
        mProgressDialog.show();
    }

    @Override
    public void onDestroy(){
        super.onDestroy();
        dismissProgressDialog();
        dismissExportConfirmationDialog();
    }

    private void dismissExportConfirmationDialog(){
        if (mDialog != null){
            mDialog.dismiss();
            mDialog = null;
        }
    }

    private void showToastFeedback(String message){
        Toast.makeText(getActivity(), message, Toast.LENGTH_LONG).show();
    }

    private void dismissProgressDialog(){
        if (mProgressDialog != null){
            mProgressDialog.dismiss();
            mProgressDialog = null;
        }
    }
}
