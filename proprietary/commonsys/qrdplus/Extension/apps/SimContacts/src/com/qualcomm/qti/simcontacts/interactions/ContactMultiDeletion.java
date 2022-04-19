/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
package com.qualcomm.qti.simcontacts.interactions;

import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.os.Handler;
import android.support.v4.os.ResultReceiver;

import com.android.contacts.model.SimContact;
import com.qualcomm.qti.simcontacts.ContactSaveService;
import com.qualcomm.qti.simcontacts.R;

import java.util.ArrayList;
import java.util.TreeSet;

public class ContactMultiDeletion {

    //must use Activity Context.
    private final Context mContext;
    private final int mSubscriptionId;
    private AlertDialog mDialog;
    private ProgressDialog mProgressDialog;
    private Listener mListener;

    public interface Listener{

        void onDeletionFinished();
    }

    public static ContactMultiDeletion start(Context context, TreeSet<Long> selectedContactIds,
                  ArrayList<SimContact> simContacts, int subscriptionId){
        ArrayList<SimContact> selectedContacts = new ArrayList<>();
        for (SimContact simContact : simContacts){
            if (selectedContactIds.contains(simContact.getId())){
                selectedContacts.add(simContact);
            }
        }
        return new ContactMultiDeletion(context, selectedContacts, subscriptionId);
    }

    private ContactMultiDeletion(Context context, ArrayList<SimContact> selectedContacts,
                                 int subscriptionId){
        mContext = context;
        mSubscriptionId = subscriptionId;
        if (selectedContacts.size() <= 0){
            return;
        }
        final int messageId;
        if (selectedContacts.size() == 1){
            messageId = R.string.delete_single_contact_confirmation;
        }else if (selectedContacts.size() > 1){
            messageId = R.string.delete_multiple_contacts_confirmation;
        }else {
            messageId = R.string.delete_multiple_contacts_confirmation;
        }

        showDialog(messageId, selectedContacts);
    }

    private void showDialog(int messageId, final ArrayList<SimContact> simContacts) {
        mDialog = new AlertDialog.Builder(mContext)
                .setIconAttribute(android.R.attr.alertDialogIcon)
                .setMessage(messageId)
                .setNegativeButton(android.R.string.cancel, null)
                .setPositiveButton(R.string.delete_positive_button_Confirmation,
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int whichButton) {
                                if (mListener != null){
                                    mListener.onDeletionFinished();
                                }
                                deleteSelectedContact(simContacts);
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

    private void showProgressDialog(int contactsCount){
        CharSequence title = mContext.getString(R.string.delete_contacts_title);

        mProgressDialog = new ProgressDialog(mContext);
        mProgressDialog.setTitle(title);
        mProgressDialog.setProgressStyle(ProgressDialog.STYLE_HORIZONTAL);
        mProgressDialog.setProgress(0);
        mProgressDialog.setMax(contactsCount);
        mProgressDialog.setCancelable(false);
        mProgressDialog.show();
    }

    private void deleteSelectedContact(final ArrayList<SimContact> simContacts){
        ResultReceiver receiver = new ResultReceiver(new Handler()){
            @Override
            protected void onReceiveResult(int resultCode, Bundle resultData) {
                super.onReceiveResult(resultCode, resultData);
                switch (resultCode){
                    case ContactSaveService.CONTACTS_DELETE_STARTED:
                        showProgressDialog(simContacts.size());
                        break;
                    case ContactSaveService.CONTACTS_DELETE_INCREMENT:
                        if (mProgressDialog != null){
                            mProgressDialog.incrementProgressBy(1);
                        }
                        break;
                    case ContactSaveService.CONTACTS_DELETE_COMPLETE:
                        if (mProgressDialog != null){
                            mProgressDialog.dismiss();
                            mProgressDialog = null;
                        }
                        break;
                }
            }
        };

        mContext.startService(ContactSaveService.createDeleteContactIntent(mContext,
                simContacts, mSubscriptionId, receiver));
    }

    public void setMultiDeleteListener(Listener listener){
        mListener = listener;
    }

    public void onDestroy(){
        if (mProgressDialog != null){
            mProgressDialog.dismiss();
            mProgressDialog = null;
        }

        if (mDialog != null){
            mDialog.dismiss();
            mDialog = null;
        }
    }
}
