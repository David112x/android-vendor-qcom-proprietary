/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
package com.qualcomm.qti.simcontacts.activities;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.os.Bundle;
import android.os.Handler;
import android.support.v4.os.ResultReceiver;
import android.support.v7.app.AppCompatActivity;
import android.text.TextUtils;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.inputmethod.InputMethodManager;
import android.widget.TextView;
import android.widget.Toast;

import com.android.contacts.model.SimContact;
import com.qualcomm.qti.simcontacts.ContactSaveService;
import com.qualcomm.qti.simcontacts.R;
import com.qualcomm.qti.simcontacts.util.Constants;
import com.qualcomm.qti.simcontacts.widget.EditorKindSectionView;

public class SimContactEditorActivity extends AppCompatActivity
        implements View.OnClickListener{
    public static final String ACTION_EDIT = "contact_edit";
    private boolean mEditContact = false;
    private EditorKindSectionView mNameEditor;
    private EditorKindSectionView mPhoneEditor;
    private EditorKindSectionView mAnrsEditor;
    private EditorKindSectionView mEmailsEditor;
    private int mSubscriptionId;
    private SimContact mSimContact;

    final ResultReceiver mOperationReceiver = new ResultReceiver(new Handler()){
        @Override
        protected void onReceiveResult(int resultCode, Bundle resultData) {
            super.onReceiveResult(resultCode, resultData);
            switch (resultCode){
                case ContactSaveService.CONTACTS_OPERATION_COMPLETE:
                    hideSoftKeyboard();
                    if (mEditContact){
                        Intent intent = new Intent();
                        intent.putExtra(Constants.SIM_CONTACT_CONTENT, buildNewSimContact());
                        setResult(RESULT_OK, intent);
                    }
                    onBackPressed();
                    break;
                case ContactSaveService.CONTACTS_OPERATION_FAILURE:
                    hideSoftKeyboard();
                    onBackPressed();
                    Toast.makeText(SimContactEditorActivity.this,
                            getString(R.string.create_new_or_edit_failure_message),
                            Toast.LENGTH_SHORT).show();
                    break;
            }
        }
    };

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.sim_contact_editor_activity);
        getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        getSupportActionBar().setHomeAsUpIndicator(R.drawable.quantum_ic_close_vd_theme_24);
        mEditContact = getIntent().getBooleanExtra(ACTION_EDIT, false);
        mSubscriptionId = getIntent().getIntExtra(Constants.SIM_CARD_SUBSCRIPTION_ID, -1);
        updateActionBarTitle();
        setupViews();
        if (mEditContact){
            applySimContactData();
        }
    }

    private void setupViews(){
        TextView accountName = findViewById(R.id.account_name);
        accountName.setText(getIntent().getStringExtra(Constants.SIM_CARD_DISPLAY_NAME));
        mNameEditor = findViewById(R.id.contact_name);
        mPhoneEditor = findViewById(R.id.contact_phone);
        mAnrsEditor = findViewById(R.id.contact_anrs);
        mEmailsEditor = findViewById(R.id.contact_emails);
    }

    private void applySimContactData(){
        mSimContact = getIntent().getParcelableExtra(Constants.SIM_CONTACT_CONTENT);
        mNameEditor.setInitialValue(mSimContact.getName());
        mPhoneEditor.setInitialValue(mSimContact.getPhone());
        String[] anrs = mSimContact.getAnrs();
        mAnrsEditor.setInitialValue(anrs == null || anrs.length == 0 ? null : anrs[0]);
        String[] emails = mSimContact.getEmails();
        mEmailsEditor.setInitialValue(emails == null || emails.length == 0 ? null : emails[0]);
    }

    private void updateActionBarTitle(){
        setTitle(mEditContact ? R.string.edit_existing_contact_title :
                R.string.create_new_contact_title);
    }

    private void createNewContact(){
        startService(ContactSaveService.createCreateNewContactIntent(
                this, buildNewSimContact(), mSubscriptionId, mOperationReceiver));
    }

    private void editExistingContact(){
        startService(ContactSaveService.createEditExistingContactIntent(
                this, mSimContact, buildNewSimContact(), mSubscriptionId, mOperationReceiver));
    }

    private SimContact buildNewSimContact(){
        String name = mNameEditor.getEditorTextContent();
        String phone = mPhoneEditor.getEditorTextContent();
        String anr = mAnrsEditor.getEditorTextContent();
        String email = mEmailsEditor.getEditorTextContent();
        return new SimContact(name, phone, TextUtils.isEmpty(email) ? null : new String[]{email},
                TextUtils.isEmpty(anr) ? null : new String[]{anr});
    }

    @Override
    public void onClick(View view) {
        if (hasPendingChanges()){
            if (mEditContact){
                editExistingContact();
            }else {
                createNewContact();
            }
        }else {
            hideSoftKeyboard();
            onBackPressed();
        }
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.contact_editor, menu);
        menu.findItem(R.id.menu_save).getActionView().setOnClickListener(this);
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()){
            case android.R.id.home:
                if (hasPendingChanges()){
                    showDiscardConfirmationDialog();
                }else {
                    hideSoftKeyboard();
                    onBackPressed();
                }
                break;
        }
        return super.onOptionsItemSelected(item);
    }

    private void hideSoftKeyboard() {
        // Hide soft keyboard, if visible
        InputMethodManager inputMethodManager = (InputMethodManager) getSystemService(
                Context.INPUT_METHOD_SERVICE);
        inputMethodManager.hideSoftInputFromWindow(getWindow().getDecorView().getWindowToken(), 0);
    }

    private boolean hasPendingChanges(){
        return mNameEditor.hasPendingChange()
                || mPhoneEditor.hasPendingChange()
                || mAnrsEditor.hasPendingChange()
                || mEmailsEditor.hasPendingChange();
    }

    private void showDiscardConfirmationDialog(){
        AlertDialog dialog = new AlertDialog.Builder(this)
                .setIconAttribute(android.R.attr.alertDialogIcon)
                .setMessage(R.string.cancel_confirmation_dialog_message)
                .setNegativeButton(android.R.string.cancel, null)
                .setPositiveButton(R.string.cancel_confirmation_dialog_cancel_editing_button,
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int whichButton) {
                                hideSoftKeyboard();
                                onBackPressed();
                            }
                        }
                ).create();
        dialog.show();
    }
}


