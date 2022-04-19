/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
package com.qualcomm.qti.simcontacts.activities;

import android.app.Fragment;
import android.app.LoaderManager;
import android.content.AsyncTaskLoader;
import android.content.Context;
import android.content.Intent;
import android.content.Loader;
import android.database.ContentObserver;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.text.TextUtils;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ListView;

import com.android.contacts.database.SimContactDao;
import com.android.contacts.database.SimContactDaoImpl;
import com.android.contacts.model.SimCard;
import com.android.contacts.model.SimContact;
import com.qualcomm.qti.simcontacts.R;
import com.qualcomm.qti.simcontacts.interactions.SimExportFragment;
import com.qualcomm.qti.simcontacts.interactions.SimImportFragment;
import com.qualcomm.qti.simcontacts.util.Constants;
import com.qualcomm.qti.simcontacts.util.SimContactsSortAlgorithm;
import com.qualcomm.qti.simcontacts.SimUpdateMonitor;
import com.qualcomm.qti.simcontacts.interactions.ContactMultiDeletion;
import com.qualcomm.qti.simcontacts.adapter.MultiSelectContactListAdapter;
import com.qualcomm.qti.simcontacts.adapter.SimContactBrowseListAdapter;
import com.qualcomm.qti.simcontacts.widget.DefaultToolbar;

import java.util.ArrayList;
import java.util.TreeSet;

public class SimContactBrowseListFragment extends Fragment
        implements LoaderManager.LoaderCallbacks<ArrayList<SimContact>>,
        AdapterView.OnItemClickListener, AdapterView.OnItemLongClickListener,
        MultiSelectContactListAdapter.ContactSelectionListener,
        SimUpdateMonitor.SimStateListener {
    private static final int SIM_CONTACT_LOADER_ID = 0x0010;
    private static final String TAG = "ContactsBrowserList";
    private static final boolean DEBUG = true;
    private SimContactBrowseListAdapter mContactListAdapter;
    private SimContactBrowseListActivity mActivity;
    private int mSubscriptionId;
    private ListView mSimContactList;
    private ContactMultiDeletion mContactMultiDeletion;
    private DefaultToolbar mToolbar;

    private DefaultToolbar.Listener listener = new DefaultToolbar.Listener() {
        @Override
        public void onBackPressed() {
            getActivity().onBackPressed();
        }

        @Override
        public void onSelectionModeBeginStopping() {
            mContactListAdapter.clearSelectedContactIds();
        }

        @Override
        public void onSelectionModeStopped() {
            mContactListAdapter.setDisplayCheckBox(false);
            getActivity().invalidateOptionsMenu();
        }

        @Override
        public void onSelectionModeStarted() {
            mContactListAdapter.setDisplayCheckBox(true);
        }
    };

    private ContentObserver mSimContactsObserver = new ContentObserver(new Handler()) {
        @Override
        public void onChange(boolean selfChange) {
            super.onChange(selfChange);
            getLoaderManager().initLoader(SIM_CONTACT_LOADER_ID, null,
                    SimContactBrowseListFragment.this);
        }
    };

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container, Bundle savedState) {
        View view = inflater.inflate(R.layout.sim_contacts_fragment, container, false);
        mSimContactList = view.findViewById(R.id.sim_contacts_list);
        mToolbar = view.findViewById(R.id.toolbar);
        mToolbar.setListener(listener);
        return view;
    }

    @Override
    public void onActivityCreated(Bundle savedInstanceState) {
        super.onActivityCreated(savedInstanceState);
        Bundle extras = getActivity().getIntent().getExtras();
        mSubscriptionId = extras.getInt(Constants.SIM_CARD_SUBSCRIPTION_ID);
        String displayName = extras.getString(Constants.SIM_CARD_DISPLAY_NAME);
        if (!TextUtils.isEmpty(displayName)) {
            getActivity().setTitle(displayName);
        }
        SimUpdateMonitor.getInstance(getActivity().getApplicationContext()).
                registerSimStateListener(this);
        mContactListAdapter = new SimContactBrowseListAdapter(getActivity());
        mContactListAdapter.setContactSelectionListener(this);
        mSimContactList.setAdapter(mContactListAdapter);
        mSimContactList.setOnItemClickListener(this);
        mSimContactList.setOnItemLongClickListener(this);

        registerContentObserver();

        getLoaderManager().initLoader(SIM_CONTACT_LOADER_ID, null, this);
        setHasOptionsMenu(true);

        mActivity = (SimContactBrowseListActivity) getActivity();
        mActivity.setSupportActionBar(mToolbar);
    }

    @Override
    public void onSimStateChanged(int subscriptionId, int state) {
        if (subscriptionId == mSubscriptionId){
            if (state == SimUpdateMonitor.SIM_STATE_ERROR){
                getActivity().finish();
            }
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        if (mContactMultiDeletion != null) {
            mContactMultiDeletion.onDestroy();
        }
        SimUpdateMonitor.getInstance(getActivity().getApplicationContext()).
                unRegisterSimStateListener(this);
        unRegisterContentObserver();
    }

    @Override
    public void onItemClick(AdapterView<?> adapterView, View view, int position, long id) {
        final long contactId = mContactListAdapter.getData().get(position).getId();
        if (isSelectionMode()) {
            mContactListAdapter.toggleSelectedContactIds(contactId);

            final int selectedCount = mContactListAdapter.getSelectedContactIds().size();
            if (selectedCount == 0) {
                setSelectionMode(false);
            }
        }else {
            Intent intent = new Intent(getActivity(), SimContactDetailActivity.class);
            intent.putExtra(Constants.SIM_CONTACT_CONTENT,
                    mContactListAdapter.getItem(position));
            intent.putExtra(Constants.SIM_CARD_SUBSCRIPTION_ID, mSubscriptionId);
            intent.putExtra(Constants.SIM_CARD_DISPLAY_NAME, mActivity.getSupportActionBar().getTitle());
            startActivity(intent);
        }
    }

    @Override
    public boolean onItemLongClick(AdapterView<?> adapterView, View view, int i, long l) {
        final long contactId = mContactListAdapter.getData().get(i).getId();
        if (!isSelectionMode()) {
            setSelectionMode(true);
            getActivity().invalidateOptionsMenu();
            mContactListAdapter.toggleSelectedContactIds(contactId);
        }
        return true;
    }

    @Override
    public void onSelectedContactsChanged() {
        mToolbar.updateSelectionCount(mContactListAdapter.getSelectedContactIds().size());
    }

    @Override
    public Loader<ArrayList<SimContact>> onCreateLoader(int i, Bundle bundle) {
        return new SimContactLoader(getContext(), mSubscriptionId);
    }

    @Override
    public void onLoadFinished(Loader<ArrayList<SimContact>> loader,
                ArrayList<SimContact> simContacts) {
        if (mContactListAdapter == null) {
            return;
        }
        Bundle sortExtra = SimContactsSortAlgorithm.defaultSort(simContacts);
        mContactListAdapter.setData(simContacts, sortExtra);
        mContactListAdapter.notifyDataSetChanged();
        getLoaderManager().destroyLoader(SIM_CONTACT_LOADER_ID);
        getActivity().invalidateOptionsMenu();
    }

    @Override
    public void onLoaderReset(Loader<ArrayList<SimContact>> loader) {
        //do nothing
    }

    @Override
    public void onCreateOptionsMenu(Menu menu, MenuInflater inflater) {
        super.onCreateOptionsMenu(menu, inflater);
        inflater.inflate(R.menu.sim_contact_option, menu);
        menu.findItem(R.id.menu_export).getActionView().
                setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                exportContactsToDevice();
            }
        });
    }

    @Override
    public void onPrepareOptionsMenu(Menu menu) {
        if (mToolbar.isSelectionMode()) {
            makeMenuItemVisible(menu, R.id.menu_add_contacts, false);
            makeMenuItemVisible(menu, R.id.menu_select_all, false);
            makeMenuItemVisible(menu, R.id.menu_import, false);
            makeMenuItemVisible(menu, R.id.menu_delete, true);
            makeMenuItemVisible(menu, R.id.menu_export, true);

        } else {
            makeMenuItemVisible(menu, R.id.menu_add_contacts, true);
            makeMenuItemVisible(menu, R.id.menu_select_all,
                    mContactListAdapter.getData().size() > 0);
            makeMenuItemVisible(menu, R.id.menu_export, false);
            makeMenuItemVisible(menu, R.id.menu_import, true);
            makeMenuItemVisible(menu, R.id.menu_delete, false);
        }
        mToolbar.updateMenuIconTintList();
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            case R.id.menu_select_all:
                selectAllContacts();
                break;
            case R.id.menu_delete:
                deleteSelectedContacts();
                break;
            case R.id.menu_add_contacts:
                createNewContact();
                break;
            case R.id.menu_import:
                SimImportFragment.start(mSubscriptionId, this);
                break;
        }
        return super.onOptionsItemSelected(item);

    }

    private void exportContactsToDevice(){
        ArrayList<SimContact> selectedContacts = new ArrayList<>();
        TreeSet<Long> selectContactIds = mContactListAdapter.getSelectedContactIds();
        for (SimContact simContact : mContactListAdapter.getData()){
            if (selectContactIds.contains(simContact.getId())){
                selectedContacts.add(simContact);
            }
        }
        SimExportFragment exportFragment = SimExportFragment.
                start(selectedContacts, this);
        exportFragment.setMultiExportListener(new SimExportFragment.Listener() {
            @Override
            public void onExportContactsStarted() {
                setSelectionMode(false);
            }
        });
    }

    private void makeMenuItemVisible(Menu menu, int itemId, boolean visible) {
        final MenuItem item = menu.findItem(itemId);
        if (item != null) {
            item.setVisible(visible);
        }
    }

    private void createNewContact(){
        Intent intent = new Intent(getActivity(), SimContactEditorActivity.class);
        intent.putExtra(SimContactEditorActivity.ACTION_EDIT, false);
        intent.putExtra(Constants.SIM_CARD_SUBSCRIPTION_ID, mSubscriptionId);
        intent.putExtra(Constants.SIM_CARD_DISPLAY_NAME,
                mActivity.getSupportActionBar().getTitle());
        startActivity(intent);
    }

    private void deleteSelectedContacts() {
        mContactMultiDeletion = ContactMultiDeletion.start(getActivity(),
                mContactListAdapter.getSelectedContactIds(),
                mContactListAdapter.getData(),
                mSubscriptionId);
        mContactMultiDeletion.setMultiDeleteListener(new ContactMultiDeletion.Listener() {
            @Override
            public void onDeletionFinished() {
                setSelectionMode(false);
            }
        });
    }

    private void selectAllContacts() {
        setSelectionMode(true);
        TreeSet<Long> allContactIds = new TreeSet<>();
        for (SimContact simContact : mContactListAdapter.getData()) {
            final long contactId = simContact.getId();
            if (contactId < 0) {
                continue;
            }
            allContactIds.add(contactId);
        }
        mContactListAdapter.setSelectedContactIds(allContactIds);
        getActivity().invalidateOptionsMenu();
    }

    public void setSelectionMode(boolean flag){
        mToolbar.setSelectionMode(flag);
    }

    public boolean isSelectionMode() {
        return mToolbar.isSelectionMode();
    }

    private void registerContentObserver() {
        Uri uri = SimContactDaoImpl.ICC_CONTENT_URI.buildUpon()
                .appendPath("subId")
                .appendPath(String.valueOf(mSubscriptionId))
                .build();
        getActivity().getContentResolver().registerContentObserver(uri, true,
                mSimContactsObserver);
    }

    private void unRegisterContentObserver() {
        getActivity().getContentResolver().unregisterContentObserver(mSimContactsObserver);

    }

    private static class SimContactLoader extends AsyncTaskLoader<ArrayList<SimContact>> {

        private final SimContactDao mDao;
        private final int mSubscriptionId;

        public SimContactLoader(Context context, int subscriptionId) {
            super(context);
            mDao = SimContactDao.create(context);
            mSubscriptionId = subscriptionId;
        }

        @Override
        protected void onStartLoading() {
            forceLoad();
        }

        @Override
        public ArrayList<SimContact> loadInBackground() {
            return loadFromSim();
        }

        private ArrayList<SimContact> loadFromSim() {
            final SimCard sim = mDao.getSimBySubscriptionId(mSubscriptionId);
            if (sim == null) {
                return new ArrayList<>();
            }
            return mDao.loadContactsForSim(sim);
        }
    }
}
