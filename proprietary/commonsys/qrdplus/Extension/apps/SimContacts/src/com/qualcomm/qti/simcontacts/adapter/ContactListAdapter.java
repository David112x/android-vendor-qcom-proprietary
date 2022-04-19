/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.simcontacts.adapter;

import android.content.Context;
import android.content.Intent;
import android.graphics.Typeface;
import android.os.Bundle;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Contacts;
import android.text.TextUtils;
import android.view.View;
import android.widget.CheckBox;
import android.widget.ImageView;
import android.widget.TextView;

import com.android.contacts.format.TextHighlighter;
import com.android.contacts.util.ContactsSectionIndexer;
import com.qualcomm.qti.simcontacts.R;
import com.qualcomm.qti.simcontacts.model.Contact;

import java.util.ArrayList;

public class ContactListAdapter extends MultiSelectContactListAdapter<Contact> {

    public static final int MODE_DEFAULT_CONTACT = 0;
    public static final int MODE_DEFAULT_PHONE = 1;
    private static final String ACTION_MULTI_PICK_CONTACT =
            "com.android.contacts.action.MULTI_PICK_CONTACT";
    private static final String ACTION_MULTI_PICK =
            "com.android.contacts.action.MULTI_PICK";
    private final TextHighlighter mTextHighlighter;

    private int mContactType;
    private boolean mSearchMode;
    private String mSearchKeyword;

    public ContactListAdapter(Context context){
        super(context);
        mTextHighlighter = new TextHighlighter(Typeface.BOLD);
    }

    public void setContactType(Intent intent){
        final String action = intent.getAction();
        if (ACTION_MULTI_PICK.equals(action)) {
            mContactType = MODE_DEFAULT_PHONE;
        }else if (ACTION_MULTI_PICK_CONTACT.equals(action)){
            mContactType = MODE_DEFAULT_CONTACT;
        }
    }

    public int getContactType(){
        return mContactType;
    }

    public void setSearchMode(boolean searchMode){
        mSearchMode = searchMode;
    }

    public void updateSearchKeyword(String searchKeyword){
        mSearchKeyword = searchKeyword;
    }

    @Override
    public void setData(ArrayList<Contact> contacts, Bundle bundle) {
        mContacts.clear();
        mContacts.addAll(contacts);

        if (bundle.containsKey(Contacts.EXTRA_ADDRESS_BOOK_INDEX_TITLES)
                && bundle.containsKey(Contacts.EXTRA_ADDRESS_BOOK_INDEX_COUNTS)) {
            String sections[] = bundle.getStringArray(Contacts.EXTRA_ADDRESS_BOOK_INDEX_TITLES);
            int counts[] = bundle.getIntArray(Contacts.EXTRA_ADDRESS_BOOK_INDEX_COUNTS);
            setIndexer(new ContactsSectionIndexer(sections, counts));
        } else {
            setIndexer(null);
        }
        notifyDataSetChanged();
    }

    @Override
    public ArrayList<Contact> getData() {
        return mContacts;
    }

    @Override
    public void bindView(View itemView, int position) {
        CheckBox checkBox = itemView.findViewById(R.id.contact_list_item_checkbox);
        ImageView photoView = itemView.findViewById(R.id.contact_list_item_photo);
        TextView name = itemView.findViewById(R.id.contact_list_item_name);
        TextView section = itemView.findViewById(R.id.section_index);


        bindCheckBox(checkBox, getItem(position).getId());
        bindPhotoView(photoView,getItem(position).getName(),
                getItem(position).getLookupKey(),
                getItem(position).getPhotoId(),
                getItem(position).getPhotoUri());
        bindContactName(name, getItem(position).getName());
        bindSectionHeader(section, position);
        bindLabelAndNumberIfNeeded(itemView, position);
        updateChildViewsVisibility(itemView, position);
        setHighLightIfNeed(itemView, position);
    }

    @Override
    public void bindSectionHeader(TextView section, int position){
        if (!mSearchMode){
            super.bindSectionHeader(section, position);
        }else {
            section.setVisibility(View.GONE);
        }
    }

    private void bindLabelAndNumberIfNeeded(View itemView, int position){
        TextView phone = itemView.findViewById(R.id.contact_list_item_number);
        TextView labelView = itemView.findViewById(R.id.contact_list_item_label);
        if (mContactType == MODE_DEFAULT_PHONE){
            phone.setText(mContacts.get(position).getNumber());
            labelView.setText(Phone.getTypeLabel(mContext.getResources(),
                    Integer.parseInt(mContacts.get(position).getType()),
                    mContacts.get(position).getLabel()));
        }
    }

    private void updateChildViewsVisibility(View itemView, int position){
        ImageView photoView = itemView.findViewById(R.id.contact_list_item_photo);
        if (mContactType == MODE_DEFAULT_CONTACT){
            itemView.findViewById(R.id.contact_list_item_number).setVisibility(View.GONE);
            itemView.findViewById(R.id.contact_list_item_label).setVisibility(View.INVISIBLE);
        }else if (mContactType == MODE_DEFAULT_PHONE){
            boolean isFirstEntry = true;
            long currentContactId = getItem(position).getContactId();
            if (position > 0) {
                final long previousContactId = getItem(position - 1).getContactId();
                if (currentContactId == previousContactId) {
                    isFirstEntry = false;
                }
            }

            if (isFirstEntry) {
                itemView.getLayoutParams().height = mContext.getResources()
                        .getDimensionPixelSize(R.dimen.default_contacts_item_height);
                photoView.setVisibility(View.VISIBLE);
                itemView.findViewById(R.id.contact_list_item_name).setVisibility(View.VISIBLE);
            } else {
                itemView.getLayoutParams().height = mContext.getResources()
                        .getDimensionPixelSize(R.dimen.small_contacts_item_height);
                photoView.setVisibility(View.INVISIBLE);
                itemView.findViewById(R.id.contact_list_item_name).setVisibility(View.GONE);
            }
        }
    }

    private void setHighLightIfNeed(View itemView, int position) {
        if (mSearchMode && !TextUtils.isEmpty(mSearchKeyword)) {
            setFilterHighLight(itemView, position);
        }
    }

    private void setFilterHighLight(View itemView, int position) {
        TextView nameView = itemView.findViewById(R.id.contact_list_item_name);
        CharSequence nameText = getItem(position).getName();
        nameText = mTextHighlighter.applyPrefixHighlight(nameText, mSearchKeyword.toUpperCase());
        nameView.setText(nameText);
        TextView numberView = itemView.findViewById(R.id.contact_list_item_number);
        CharSequence numberText = getItem(position).getNumber();
        numberText = mTextHighlighter.applyPrefixHighlight(numberText,
                mSearchKeyword.toUpperCase());
        numberView.setText(numberText);
    }
}
