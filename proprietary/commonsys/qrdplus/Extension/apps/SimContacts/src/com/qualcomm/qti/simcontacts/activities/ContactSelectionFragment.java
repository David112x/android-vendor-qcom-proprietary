/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/

package com.qualcomm.qti.simcontacts.activities;

import android.app.Activity;
import android.app.ListFragment;
import android.content.AsyncQueryHandler;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.Directory;
import android.provider.ContactsContract.RawContacts;
import android.text.TextUtils;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ListView;
import android.widget.TextView;

import com.android.contacts.util.UriUtils;
import com.qualcomm.qti.simcontacts.R;
import com.qualcomm.qti.simcontacts.adapter.ContactListAdapter;
import com.qualcomm.qti.simcontacts.adapter.MultiSelectContactListAdapter;
import com.qualcomm.qti.simcontacts.model.Contact;
import com.qualcomm.qti.simcontacts.widget.ContactToolbar;

import java.util.ArrayList;
import java.util.TreeSet;

public class ContactSelectionFragment extends ListFragment
        implements MultiSelectContactListAdapter.ContactSelectionListener,
        View.OnClickListener{
    private final static String TAG = "MultiPickContact";
    private static final String RESULT_KEY = "result";

    static final String[] CONTACTS_SUMMARY_PROJECTION = new String[] {
            Contacts._ID, // 0
            Contacts.NAME_RAW_CONTACT_ID, // 1
            Contacts.LOOKUP_KEY, // 2
            Contacts.DISPLAY_NAME_PRIMARY, // 3
            Contacts.PHOTO_ID, // 4
            Contacts.PHOTO_THUMBNAIL_URI, // 5
    };

    private static final String[] DATA_PROJECTION = new String[] {
            Data._ID, // 0
            Data.CONTACT_ID, // 1
            Contacts.LOOKUP_KEY, // 2
            Data.DISPLAY_NAME, // 3
            Contacts.PHOTO_ID,// 4
            Contacts.PHOTO_THUMBNAIL_URI, // 5
            Data.DATA1, // 6 Phone.NUMBER, Email.address
            Data.DATA2, // 7 phone.type
            Data.DATA3, // 8 Phone.LABEL
            Data.MIMETYPE, //9
    };

    // contacts column
    private static final int SUMMARY_ID_COLUMN_INDEX = 0;
    private static final int SUMMARY_COLUMN_CONTACT_ID = 1;
    private static final int SUMMARY_LOOKUP_KEY_COLUMN_INDEX = 2;
    private static final int SUMMARY_DISPLAY_NAME_PRIMARY_COLUMN_INDEX = 3;
    private static final int SUMMARY_PHOTO_ID_COLUMN_INDEX = 4;
    private static final int SUMMARY_CONTACT_COLUMN_PHOTO_URI = 5;

    private static final int DATA_DATA1_COLUMN = 6;
    private static final int DATA_DATA2_COLUMN = 7;  //phone.type
    private static final int DATA_DATA3_COLUMN = 8; //Phone.LABEL

    private static final int QUERY_TOKEN = 43;

    private int mSubscriptionId = -1;
    private TextView mConfirmationButton;
    private QueryHandler mQueryHandler;
    private ContactMultiSelectionActivity mContext;
    private ContactListAdapter mContactListAdapter;
    private ContactMultiSelectionActivity mActivity;
    private ContactToolbar mToolbar;

    private ContactToolbar.Listener listener = new ContactToolbar.Listener() {
        @Override
        public void onSearchKeywordChanged(String searchKeyword) {
            doFilter(searchKeyword);
        }

        @Override
        public void onAllContactsSelectedOrDeselected() {
            setSelectedAll(mContactListAdapter.getSelectedContactIds().size() <
                    getListAdapter().getCount());
        }

        @Override
        public void onSearchModeStopped() {
            getActivity().invalidateOptionsMenu();
            startQuery();

        }
    };

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedState) {
        View view = inflater.inflate(R.layout.contact_selection_fragment, container, false);
        mToolbar = view.findViewById(R.id.toolbar);
        mToolbar.setListener(listener);
        mConfirmationButton = view.findViewById(R.id.confirmation_button);
        mConfirmationButton.setOnClickListener(this);
        return view;
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        Intent intent = getActivity().getIntent();
        mSubscriptionId = intent.getIntExtra("importSubId", -1);
        mContext = (ContactMultiSelectionActivity)getActivity();
        mContactListAdapter = new ContactListAdapter(mContext);
        mContactListAdapter.setContactSelectionListener(this);
        mContactListAdapter.setContactType(intent);
        mContactListAdapter.setDisplayCheckBox(true);
        setHasOptionsMenu(true);
        setListAdapter(mContactListAdapter);
        mQueryHandler = new QueryHandler(mContext);
        startQuery();

        mConfirmationButton.setText(mSubscriptionId != -1 ?
                getString(R.string.import_from_device_confirmation_text).toUpperCase() :
                getString(R.string.select_multiple_contacts_confirmation_text).toUpperCase());
        updateConfirmationButton();

        mActivity = (ContactMultiSelectionActivity) getActivity();
        mActivity.setSupportActionBar(mToolbar);
        mActivity.getSupportActionBar().setDisplayShowHomeEnabled(true);
        mActivity.getSupportActionBar().setDisplayHomeAsUpEnabled(true);
        mActivity.getSupportActionBar().setDisplayShowTitleEnabled(false);
        mActivity.getSupportActionBar().setHomeAsUpIndicator(
                R.drawable.quantum_ic_arrow_back_vd_theme_24);
    }

    @Override
    public void onListItemClick(ListView l, View v, int position, long id) {
        final long contactId = mContactListAdapter.getData().get(position).getId();
        mContactListAdapter.toggleSelectedContactIds(contactId);
        setSearchMode(false);
    }

    @Override
    public void onSelectedContactsChanged() {
        mToolbar.updateSelectionCount(mContactListAdapter.getSelectedContactIds().size(),
                getListAdapter().getCount());
        updateConfirmationButton();
    }

    @Override
    public void onDestroy() {
        mQueryHandler.removeCallbacksAndMessages(QUERY_TOKEN);
        super.onDestroy();
    }

    public boolean isSearchMode(){
        return mToolbar.isSearchMode();
    }

    public void setSearchMode(boolean searchMode){
        mToolbar.setSearchMode(searchMode);
        mContactListAdapter.setSearchMode(searchMode);
        updateConfirmationButton();
    }

    private void updateConfirmationButton(){
        mConfirmationButton.setVisibility(mToolbar.isSearchMode() ?  View.GONE : View.VISIBLE);
        if (0 == mContactListAdapter.getSelectedContactIds().size()) {
            mConfirmationButton.setEnabled(false);
            mConfirmationButton.setTextColor(
                    getActivity().getColor(R.color.ok_or_clear_button_disable_color));
        } else {
            mConfirmationButton.setEnabled(true);
            mConfirmationButton.setTextColor(
                    getActivity().getColor(R.color.ok_or_clear_button_normal_color));
        }
    }

    private void startQuery() {
        mQueryHandler.removeCallbacksAndMessages(QUERY_TOKEN);
        Uri uri = getContactUri();
        String[] projection = getProjection();
        mQueryHandler.startQuery(QUERY_TOKEN, null, uri, projection, null, null,
                RawContacts.SORT_KEY_PRIMARY);
    }

    private void doFilter(String searchKeyword) {
        if (TextUtils.isEmpty(searchKeyword)) {
            mContactListAdapter.setData(new ArrayList<Contact>(), new Bundle());
            return;
        }
        mContactListAdapter.updateSearchKeyword(searchKeyword);
        mQueryHandler.removeCallbacksAndMessages(QUERY_TOKEN);
        Uri uri = Uri.withAppendedPath(getContactFilterUri(), Uri.encode(searchKeyword));
        String[] projection = getProjection();
        mQueryHandler.startQuery(QUERY_TOKEN, null, uri, projection, null, null,
                RawContacts.SORT_KEY_PRIMARY);
    }

    private Uri getContactUri() {
        Uri uri;
        switch (mContactListAdapter.getContactType()) {
            case ContactListAdapter.MODE_DEFAULT_CONTACT:
                uri = Contacts.CONTENT_URI;
                break;
            case ContactListAdapter.MODE_DEFAULT_PHONE:
                uri = Phone.CONTENT_URI;
                uri = uri.buildUpon().appendQueryParameter(ContactsContract.DIRECTORY_PARAM_KEY,
                        String.valueOf(Directory.DEFAULT))
                        .appendQueryParameter(ContactsContract.REMOVE_DUPLICATE_ENTRIES, "true")
                        .build();
                break;
            default:
                uri = Contacts.CONTENT_URI;
        }
        return uri.buildUpon().appendQueryParameter(Contacts.EXTRA_ADDRESS_BOOK_INDEX, "true")
                .build();
    }

    private Uri getContactFilterUri() {
        switch (mContactListAdapter.getContactType()) {
            case ContactListAdapter.MODE_DEFAULT_CONTACT:
                return Contacts.CONTENT_FILTER_URI;
            case ContactListAdapter.MODE_DEFAULT_PHONE:
                return Phone.CONTENT_FILTER_URI;
            default:
                Log.e(TAG, "getFilterUri: Incorrect mode: "
                        + mContactListAdapter.getContactType());
        }
        return Contacts.CONTENT_FILTER_URI;
    }

    private String[] getProjection() {
        switch (mContactListAdapter.getContactType()) {
            case ContactListAdapter.MODE_DEFAULT_CONTACT:
                return CONTACTS_SUMMARY_PROJECTION;
            case ContactListAdapter.MODE_DEFAULT_PHONE:
                return DATA_PROJECTION;
            default:
                Log.e(TAG,"getProjectionForQuery: Incorrect mode: "
                        + mContactListAdapter.getContactType());
        }
        return CONTACTS_SUMMARY_PROJECTION;
    }

    private void setSelectedAll(boolean isSelectedAll) {
        if (isSelectedAll){
            TreeSet<Long> allContactIds = new TreeSet<>();
            for(Contact contact: mContactListAdapter.getData()){
                final long contactId = contact.getId();
                if (contactId < 0){
                    continue;
                }
                allContactIds.add(contactId);
            }
            mContactListAdapter.setSelectedContactIds(allContactIds);
        }else {
            mContactListAdapter.clearSelectedContactIds();
        }
    }

    @Override
    public void onClick(View view) {
        //pass selected contact data.
        final Bundle selectedContacts = new Bundle();
        ArrayList<Contact> contacts = mContactListAdapter.getData();
        for (Contact contact : contacts){
            if (mContactListAdapter.getSelectedContactIds().contains(contact.getId())){
                String[] value = null;
                if (mContactListAdapter.getContactType()
                        == ContactListAdapter.MODE_DEFAULT_CONTACT) {
                    value = new String[] {
                            contact.getLookupKey(),
                            String.valueOf(contact.getId()),
                            String.valueOf(contact.getContactId()),
                            contact.getPhotoUri() == null ? null : String
                                    .valueOf(contact.getPhotoUri()),
                            contact.getName()};
                } else if (mContactListAdapter.getContactType()
                        == ContactListAdapter.MODE_DEFAULT_PHONE) {
                    value = new String[] { contact.getName(),
                            contact.getNumber(),
                            contact.getType(),
                            contact.getLabel(),
                            String.valueOf(contact.getContactId())};
                }
                selectedContacts.putStringArray(String.valueOf(contact.getId()),value);
            }
        }
        Intent intent = new Intent();
        intent.putExtra(RESULT_KEY, selectedContacts);
        getActivity().setResult(Activity.RESULT_OK, intent);
        getActivity().finish();
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        super.onCreateOptionsMenu(menu, inflater);
        inflater.inflate(R.menu.contact_selection, menu);
    }

    @Override
    public void onPrepareOptionsMenu(Menu menu) {
        makeMenuItemVisible(menu, R.id.menu_search);
        if (mActivity != null) {
            mToolbar.updateHomeAsUpIndicator(mActivity.getSupportActionBar());
        }
        super.onPrepareOptionsMenu(menu);
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.menu_search:
                setSearchMode(true);
                getActivity().invalidateOptionsMenu();
                return true;
            case android.R.id.home:
                getActivity().onBackPressed();
                return true;
        }
        return super.onOptionsItemSelected(item);
    }

    private void makeMenuItemVisible(Menu menu, int itemId) {
        makeMenuItemVisible(menu, itemId, !mToolbar.isSearchMode());
    }

    private void makeMenuItemVisible(Menu menu, int itemId, boolean visible) {
        final MenuItem item = menu.findItem(itemId);
        if (item != null) {
            item.setVisible(visible);
        }
    }

    private class QueryHandler extends AsyncQueryHandler {
        public QueryHandler(Context context) {
            super(context.getContentResolver());
        }

        @Override
        protected void onQueryComplete(int token, Object cookie, Cursor cursor) {
            Bundle extras = cursor.getExtras();
            ArrayList<Contact> contacts = loadFromCursor(cursor);
            mContactListAdapter.setData(contacts, extras);
        }

        private ArrayList<Contact> loadFromCursor(Cursor cursor){
            if (cursor == null){
                return new ArrayList<>();
            }
            ArrayList<Contact> contacts = new ArrayList<>();
            try {
                while (cursor.moveToNext()) {
                    final long id = cursor.getLong(SUMMARY_ID_COLUMN_INDEX);
                    final String name = cursor.getString(SUMMARY_DISPLAY_NAME_PRIMARY_COLUMN_INDEX);
                    final String lookupKey = cursor.getString(SUMMARY_LOOKUP_KEY_COLUMN_INDEX);
                    final long photoId = cursor.getLong(SUMMARY_PHOTO_ID_COLUMN_INDEX);
                    final Uri photoUri = UriUtils.parseUriOrNull(cursor
                            .getString(SUMMARY_CONTACT_COLUMN_PHOTO_URI));
                    final long contactId = cursor.getLong(SUMMARY_COLUMN_CONTACT_ID);
                    if (mContactListAdapter.getContactType()
                            == ContactListAdapter.MODE_DEFAULT_CONTACT) {
                        Contact contact = new Contact(id, lookupKey, name,
                                contactId, photoId, photoUri);
                        contacts.add(contact);
                    }else if (mContactListAdapter.getContactType()
                            == ContactListAdapter.MODE_DEFAULT_PHONE){
                        final String number = cursor.getString(DATA_DATA1_COLUMN);
                        final String type = String.valueOf(cursor.getInt(DATA_DATA2_COLUMN));
                        final String label = cursor.getString(DATA_DATA3_COLUMN);
                        Contact contact = new Contact(id, lookupKey, name, number, type,
                                label, contactId, photoId, photoUri);
                        contacts.add(contact);
                    }
                }
            }finally {
                cursor.close();
            }
            return contacts;
        }
    }
}
