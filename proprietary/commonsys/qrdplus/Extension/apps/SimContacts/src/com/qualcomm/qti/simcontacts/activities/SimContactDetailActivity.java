/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
package com.qualcomm.qti.simcontacts.activities;

import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.res.ColorStateList;
import android.graphics.PorterDuff;
import android.graphics.PorterDuffColorFilter;
import android.graphics.drawable.BitmapDrawable;
import android.graphics.drawable.Drawable;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.support.v4.os.ResultReceiver;
import android.support.v7.app.AppCompatActivity;
import android.support.v7.widget.Toolbar;
import android.text.TextUtils;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.android.contacts.ContactPhotoManager;
import com.android.contacts.ContactPhotoManager.DefaultImageRequest;
import com.android.contacts.database.SimContactDao;
import com.android.contacts.lettertiles.LetterTileDrawable;
import com.android.contacts.model.SimCard;
import com.android.contacts.model.SimContact;
import com.android.contacts.util.MaterialColorMapUtils;
import com.android.contacts.util.MaterialColorMapUtils.MaterialPalette;
import com.qualcomm.qti.simcontacts.ContactSaveService;
import com.qualcomm.qti.simcontacts.R;
import com.qualcomm.qti.simcontacts.util.Constants;
import com.qualcomm.qti.simcontacts.widget.ContactCardView;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

public class SimContactDetailActivity extends AppCompatActivity {

    private SimContact mSimContact;
    private MaterialColorMapUtils mMaterialColorMapUtils;
    private MaterialPalette mMaterialPalette;
    private int mSubscriptionId;
    private String mSimCardDisplayName;
    private SimContactDao mSimContactDao;
    private SimCard mSecondSimCard;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.sim_contact_detail_activity);
        mMaterialColorMapUtils = new MaterialColorMapUtils(getResources());
        Toolbar toolbar = findViewById(R.id.toolbar);
        setSupportActionBar(toolbar);
        mSimContactDao = SimContactDao.create(this);

        getSupportActionBar().setDisplayShowTitleEnabled(false);
        mSimContact = getIntent().getParcelableExtra(Constants.SIM_CONTACT_CONTENT);
        mSubscriptionId = getIntent().getIntExtra(Constants.SIM_CARD_SUBSCRIPTION_ID, -1);
        mSimCardDisplayName = getIntent().getStringExtra(Constants.SIM_CARD_DISPLAY_NAME);
        setupColors();
        drawContactPhoto();
        bindContactData();
        loadSecondSimCard();
    }

    private void setupColors() {
        DefaultImageRequest request;
        int contactType = ContactPhotoManager.TYPE_DEFAULT;
        if (TextUtils.isEmpty(mSimContact.getLookupKey())) {
            request = new ContactPhotoManager.DefaultImageRequest(null,
                    mSimContact.getItemLabel(), contactType, false /* isCircular */);
        } else {
            request = new DefaultImageRequest(mSimContact.getItemLabel(),
                    mSimContact.getLookupKey(), contactType, false /* isCircular */);
        }
        Drawable drawable = ContactPhotoManager.
                getDefaultAvatarDrawableForContact(getResources(), true, request);
        final int primaryColor = ((LetterTileDrawable) drawable).getColor();
        mMaterialPalette = mMaterialColorMapUtils.calculatePrimaryAndSecondaryColor(primaryColor);
    }

    private void drawContactPhoto() {
        ImageView imageView = findViewById(R.id.photo_view);
        final BitmapDrawable bitmapDrawable = (BitmapDrawable) getResources().getDrawable(
                R.drawable.person_white_540dp);
        imageView.setBackgroundColor(mMaterialPalette.mPrimaryColor);
        imageView.setImageDrawable(bitmapDrawable);

        getWindow().setStatusBarColor(mMaterialPalette.mSecondaryColor);
    }

    private void bindContactData() {
        ((TextView)findViewById(R.id.label)).setText(mSimContact.getItemLabel());
        if (mSimContact == null || (TextUtils.isEmpty(mSimContact.getPhone())
                && TextUtils.isEmpty(mSimContact.getEmailsString())
                && TextUtils.isEmpty(mSimContact.getAnrsString()))) {
            showNoContactDataCard();
            return;
        }
        showContactDataCard();
    }

    private void showNoContactDataCard() {
        List<List<ContactCardView.Entry>> entries = new ArrayList<>();
        List<ContactCardView.Entry> phones = new ArrayList<>();
        List<ContactCardView.Entry> emails = new ArrayList<>();
        ContactCardView noContactDataCard = findViewById(R.id.no_contact_data_card);
        ContactCardView.Entry entry1 = new ContactCardView.Entry(getString(
                R.string.add_phone_number), "",
                R.drawable.quantum_ic_phone_vd_theme_24,
                getEditContactIntent(), 0, null);
        phones.add(entry1);
        ContactCardView.Entry entry2 = new ContactCardView.Entry(
                getString(R.string.add_email), "",
                R.drawable.quantum_ic_email_vd_theme_24,
                getEditContactIntent(), 0, null);
        emails.add(entry2);
        entries.add(phones);
        entries.add(emails);
        findViewById(R.id.contact_data_card).setVisibility(View.GONE);
        noContactDataCard.setVisibility(View.VISIBLE);
        noContactDataCard.setupViews(entries);
    }

    private void showContactDataCard() {
        List<List<ContactCardView.Entry>> entries = new ArrayList<>();
        List<ContactCardView.Entry> phones = new ArrayList<>();
        List<ContactCardView.Entry> emails = new ArrayList<>();
        ContactCardView contactDataCard = findViewById(R.id.contact_data_card);

        String phone = mSimContact.getPhone();
        if (!TextUtils.isEmpty(phone)) {
            ContactCardView.Entry entry = new ContactCardView.Entry("", phone,
                    R.drawable.quantum_ic_phone_vd_theme_24, getDialerIntent(phone),
                    R.drawable.quantum_ic_message_vd_theme_24, getSendSmsIntent(phone));
            phones.add(entry);
        }

        if (mSimContact.getAnrs() != null) {
            for (String anr : mSimContact.getAnrs()) {
                ContactCardView.Entry entry = new ContactCardView.Entry("", anr,
                        R.drawable.quantum_ic_phone_vd_theme_24, getDialerIntent(anr),
                        R.drawable.quantum_ic_message_vd_theme_24, getSendSmsIntent(anr));
                phones.add(entry);
            }
        }

        if (mSimContact.getEmails() != null) {
            for (String email : mSimContact.getEmails()) {
                ContactCardView.Entry entry = new ContactCardView.Entry("", email,
                        R.drawable.quantum_ic_email_vd_theme_24, null,
                        0, null);
                emails.add(entry);
            }
        }

        if (phones.size() > 0) {
            entries.add(phones);
        }
        if (emails.size() > 0) {
            entries.add(emails);
        }
        findViewById(R.id.no_contact_data_card).setVisibility(View.GONE);
        contactDataCard.setVisibility(View.VISIBLE);
        contactDataCard.setupViews(entries);

        PorterDuffColorFilter colorFilter = new PorterDuffColorFilter(
                mMaterialPalette.mPrimaryColor, PorterDuff.Mode.SRC_ATOP);
        contactDataCard.setColorFilter(colorFilter);
    }

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.contact_details, menu);
        menu.findItem(R.id.menu_edit).setIconTintList(ColorStateList.valueOf(getColor(
                android.R.color.white)));
        return super.onCreateOptionsMenu(menu);
    }

    @Override
    public boolean onPrepareOptionsMenu(Menu menu) {
        if (mSecondSimCard == null){
            menu.findItem(R.id.menu_copy_to_sim).setVisible(false);
        }else {
            menu.findItem(R.id.menu_copy_to_sim).setVisible(true);
            menu.findItem(R.id.menu_copy_to_sim).setTitle(getString(
                    R.string.copy_to_sim) + mSecondSimCard.getDisplayName());
        }
        return super.onPrepareOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.menu_edit:
                startActivityForResult(getEditContactIntent(), 0);
                break;
            case R.id.menu_delete:
                showDeleteConfirmationDialog();
                break;
            case R.id.menu_copy_to_phone:
                copySimContactToPhone();
                break;
            case R.id.menu_copy_to_sim:
                copyToSimCard();
                break;
        }
        return super.onOptionsItemSelected(item);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (resultCode == RESULT_OK) {
            SimContact simContact = data.getParcelableExtra(Constants.SIM_CONTACT_CONTENT);
            if (simContact != null){
                mSimContact = simContact;
                setupColors();
                drawContactPhoto();
                bindContactData();
            }
        }
    }

    private void copyToSimCard(){
        final ResultReceiver receiver = new ResultReceiver(new Handler()){
            @Override
            protected void onReceiveResult(int resultCode, Bundle resultData) {
                super.onReceiveResult(resultCode, resultData);
                switch (resultCode){
                    case ContactSaveService.CONTACTS_OPERATION_COMPLETE:
                        Toast.makeText(SimContactDetailActivity.this,
                                R.string.copy_done, Toast.LENGTH_SHORT).show();
                        break;
                    case ContactSaveService.CONTACTS_OPERATION_FAILURE:
                        Toast.makeText(SimContactDetailActivity.this,
                                R.string.copy_failure, Toast.LENGTH_SHORT).show();
                        break;
                }
            }
        };
        startService(ContactSaveService.createCreateNewContactIntent(
                this, mSimContact, mSecondSimCard.getSubscriptionId(), receiver));
    }

    private void copySimContactToPhone(){
        ResultReceiver receiver = new ResultReceiver(new Handler()){
            @Override
            protected void onReceiveResult(int resultCode, Bundle resultData) {
                super.onReceiveResult(resultCode, resultData);
                switch (resultCode){
                    case ContactSaveService.CONTACTS_EXPORT_COMPLETE:
                        Toast.makeText(SimContactDetailActivity.this,
                                R.string.copy_done, Toast.LENGTH_SHORT).show();
                        break;
                    case ContactSaveService.CONTACTS_EXPORT_FAILURE:
                        Toast.makeText(SimContactDetailActivity.this,
                                R.string.copy_failure, Toast.LENGTH_SHORT).show();
                        break;
                }
            }
        };
        startService(ContactSaveService.createSimExportIntent(this,
                new ArrayList<>(Arrays.asList(mSimContact)), receiver));
    }

    private void showDeleteConfirmationDialog() {
        Dialog dialog = new AlertDialog.Builder(this)
                .setIconAttribute(android.R.attr.alertDialogIcon)
                .setMessage(R.string.delete_single_contact_confirmation)
                .setNegativeButton(android.R.string.cancel, null)
                .setPositiveButton(R.string.delete_positive_button_Confirmation,
                        new DialogInterface.OnClickListener() {
                            @Override
                            public void onClick(DialogInterface dialog, int whichButton) {
                                deleteContact();
                            }
                        }
                )
                .create();

        dialog.setOnDismissListener(new DialogInterface.OnDismissListener() {
            @Override
            public void onDismiss(DialogInterface dialog) {
                dialog = null;
            }
        });
        dialog.show();
    }

    private void deleteContact(){
        ResultReceiver receiver = new ResultReceiver(new Handler()){
            @Override
            protected void onReceiveResult(int resultCode, Bundle resultData) {
                super.onReceiveResult(resultCode, resultData);
                switch (resultCode){
                    case ContactSaveService.CONTACTS_DELETE_COMPLETE:
                        finish();
                        break;
                }
            }
        };

        startService(ContactSaveService.createDeleteContactIntent(this,
                new ArrayList<>(Arrays.asList(mSimContact)),
                mSubscriptionId, receiver));
    }

    private void loadSecondSimCard(){
        List<SimCard> simCards = mSimContactDao.getSimCards();
        if (simCards.size() > 1) {
            for (SimCard simCard : simCards) {
                if (simCard.getSubscriptionId() != mSubscriptionId) {
                    mSecondSimCard = simCard;
                    invalidateOptionsMenu();
                }
            }
        }
    }

    private Intent getEditContactIntent() {
        Intent intent = new Intent(this, SimContactEditorActivity.class);
        intent.putExtra(SimContactEditorActivity.ACTION_EDIT, true);
        intent.putExtra(Constants.SIM_CONTACT_CONTENT, mSimContact);
        intent.putExtra(Constants.SIM_CARD_SUBSCRIPTION_ID, mSubscriptionId);
        intent.putExtra(Constants.SIM_CARD_DISPLAY_NAME, mSimCardDisplayName);
        return intent;
    }

    private Intent getDialerIntent(String phone) {
        Intent intent = new Intent();
        intent.setAction(Intent.ACTION_CALL);
        intent.setData(Uri.parse("tel:" + phone));
        return intent;
    }

    private Intent getSendSmsIntent(String phone) {
        Intent intent = new Intent();
        intent.setAction(Intent.ACTION_SENDTO);
        intent.setData(Uri.parse("smsto:" + phone));
        return intent;
    }
}
