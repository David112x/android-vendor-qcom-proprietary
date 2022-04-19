/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.android.contacts.database;

import android.annotation.TargetApi;
import android.content.ContentProviderOperation;
import android.content.ContentProviderResult;
import android.content.ContentResolver;
import android.content.ContentValues;
import android.content.Context;
import android.content.OperationApplicationException;
import android.content.pm.PackageManager;
import android.database.Cursor;
import android.net.Uri;
import android.os.Build;
import android.os.RemoteException;
import android.provider.BaseColumns;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.Data;
import android.provider.ContactsContract.RawContacts;
import android.telephony.PhoneNumberUtils;
import android.telephony.SubscriptionInfo;
import android.telephony.SubscriptionManager;
import android.telephony.TelephonyManager;
import android.text.TextUtils;
import android.util.SparseArray;

import com.android.contacts.compat.CompatUtils;
import com.android.contacts.model.SimCard;
import com.android.contacts.model.SimContact;
import com.android.contacts.model.account.AccountWithDataSet;
import com.android.contacts.util.PermissionsUtil;
import com.android.contacts.util.SharedPreferenceUtil;
import com.qualcomm.qti.simcontacts.R;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

/**
 * Provides data access methods for loading contacts from a SIM card and and migrating these
 * SIM contacts to a CP2 account.
 */
public class SimContactDaoImpl extends SimContactDao {
    private static final String TAG = "SimContactDao";

    // Maximum number of SIM contacts to import in a single ContentResolver.applyBatch call.
    // This is necessary to avoid TransactionTooLargeException when there are a large number of
    // contacts. This has been tested on Nexus 6 NME70B and is probably be conservative enough
    // to work on any phone.
    private static final int IMPORT_MAX_BATCH_SIZE = 300;

    // How many SIM contacts to consider in a single query. This prevents hitting the SQLite
    // query parameter limit.
    static final int QUERY_MAX_BATCH_SIZE = 100;

    public static final Uri ICC_CONTENT_URI = Uri.parse("content://icc/adn");

    public static String _ID = BaseColumns._ID;
    public static String NAME = "name";
    public static String NUMBER = "number";
    public static String EMAILS = "emails";
    public static String ANRS = "anrs";

    private static final String STR_TAG = "tag";
    private static final String STR_NEW_TAG = "newTag";
    private static final String STR_NEW_NUMBER = "newNumber";
    private static final String STR_NEW_EMAILS = "newEmails";
    private static final String STR_NEW_ANRS = "newAnrs";

    private final Context mContext;
    private final ContentResolver mResolver;
    private final TelephonyManager mTelephonyManager;

    public SimContactDaoImpl(Context context) {
        this(context, context.getContentResolver(),
                (TelephonyManager) context.getSystemService(Context.TELEPHONY_SERVICE));
    }

    public SimContactDaoImpl(Context context, ContentResolver resolver,
            TelephonyManager telephonyManager) {
        mContext = context;
        mResolver = resolver;
        mTelephonyManager = telephonyManager;
    }

    public Context getContext() {
        return mContext;
    }

    @Override
    public boolean canReadSimContacts() {
        // Require SIM_STATE_READY because the TelephonyManager methods related to SIM require
        // this state
        return hasTelephony() && hasPermissions() &&
                mTelephonyManager.getSimState() == TelephonyManager.SIM_STATE_READY;
    }

    @Override
    public List<SimCard> getSimCards() {
        if (!canReadSimContacts()) {
            return Collections.emptyList();
        }
        final List<SimCard> sims = CompatUtils.isMSIMCompatible() ?
                getSimCardsFromSubscriptions() :
                Collections.singletonList(SimCard.create(mTelephonyManager,
                        mContext.getString(R.string.single_sim_display_label)));
        return SharedPreferenceUtil.restoreSimStates(mContext, sims);
    }

    @Override
    public ArrayList<SimContact> loadContactsForSim(SimCard sim) {
        if (sim.hasValidSubscriptionId()) {
            return loadSimContacts(sim.getSubscriptionId());
        }
        return loadSimContacts();
    }

    @Override
    public boolean deleteSimContact(SimContact simContact, int subscriptionId) {
        Uri uri = ICC_CONTENT_URI.buildUpon()
                .appendPath("subId")
                .appendPath(String.valueOf(subscriptionId))
                .build();
        StringBuilder stringBuilder = simContact.SimContactToBuilder();
        return  mResolver.delete(uri,stringBuilder.toString(),null) > 0;
    }

    @Override
    public Uri createNewSimContact(SimContact simContact, int subscriptionId) {
        Uri uri = ICC_CONTENT_URI.buildUpon()
                .appendPath("subId")
                .appendPath(String.valueOf(subscriptionId))
                .build();
        ContentValues values = new ContentValues();
        values.put(STR_TAG, TextUtils.isEmpty(simContact.getName()) ? "" : simContact.getName());
        if (!TextUtils.isEmpty(simContact.getPhone())) {
            values.put(NUMBER, PhoneNumberUtils.stripSeparators(simContact.getPhone()));
        }
        values.put(EMAILS, simContact.getEmailsString());
        values.put(ANRS, simContact.getAnrsString().
                replaceAll("[^0123456789PWN\\,\\;\\*\\#\\+\\:]", ""));
        return mResolver.insert(uri,values);
    }

    public ArrayList<SimContact> loadSimContacts(int subscriptionId) {
        return loadFrom(ICC_CONTENT_URI.buildUpon()
                .appendPath("subId")
                .appendPath(String.valueOf(subscriptionId))
                .build());
    }

    public ArrayList<SimContact> loadSimContacts() {
        return loadFrom(ICC_CONTENT_URI);
    }

    @Override
    public ContentProviderResult[] exportContacts(List<SimContact> contacts)
            throws RemoteException, OperationApplicationException{
        return exportContacts(contacts, AccountWithDataSet.getNullAccount());
    }

    @Override
    public ContentProviderResult[] exportContacts(List<SimContact> contacts,
            AccountWithDataSet targetAccount)
            throws RemoteException, OperationApplicationException {
        if (contacts.size() < IMPORT_MAX_BATCH_SIZE) {
            return importBatch(contacts, targetAccount);
        }
        final List<ContentProviderResult> results = new ArrayList<>();
        for (int i = 0; i < contacts.size(); i += IMPORT_MAX_BATCH_SIZE) {
            results.addAll(Arrays.asList(importBatch(
                    contacts.subList(i, Math.min(contacts.size(), i + IMPORT_MAX_BATCH_SIZE)),
                    targetAccount)));
        }
        return results.toArray(new ContentProviderResult[results.size()]);
    }

    @Override
    public int editExistingSimContact(SimContact oldSimContact, SimContact newSimContact,
                                      int subscriptionId){
        Uri uri = ICC_CONTENT_URI.buildUpon()
                .appendPath("subId")
                .appendPath(String.valueOf(subscriptionId))
                .build();
        ContentValues values = new ContentValues();

        String newTag = newSimContact.getName();
        String newNumber = newSimContact.getPhone();
        String newAnrs = newSimContact.getAnrsString();
        String newEmails = newSimContact.getEmailsString();

        if (TextUtils.isEmpty(newTag) && TextUtils.isEmpty(newNumber)
                && TextUtils.isEmpty(newAnrs) && TextUtils.isEmpty(newEmails) ){
            return 0;
        }

        //******old sim contact data **//
        values.put(STR_TAG, TextUtils.isEmpty(oldSimContact.getName()) ? "" : oldSimContact.getName());
        if (!TextUtils.isEmpty(oldSimContact.getPhone())) {
            values.put(NUMBER, PhoneNumberUtils.stripSeparators(oldSimContact.getPhone()));
        }
        values.put(EMAILS, oldSimContact.getEmailsString());
        values.put(ANRS, oldSimContact.getAnrsString().
                replaceAll("[^0123456789PWN\\,\\;\\*\\#\\+\\:]", ""));
       //******new sim contact data **//
        values.put(STR_NEW_TAG, TextUtils.isEmpty(newTag) ? "" : newTag);
        if (!TextUtils.isEmpty(newNumber)) {
            values.put(STR_NEW_NUMBER, PhoneNumberUtils.stripSeparators(newNumber));
        }
        values.put(STR_NEW_EMAILS, newSimContact.getEmailsString());
        values.put(STR_NEW_ANRS, newSimContact.getAnrsString().
                replaceAll("[^0123456789PWN\\,\\;\\*\\#\\+\\:]", ""));
        return mResolver.update(uri,values,null,null);
    }

    public void persistSimState(SimCard sim) {
        SharedPreferenceUtil.persistSimStates(mContext, Collections.singletonList(sim));
    }

    @Override
    public void persistSimStates(List<SimCard> simCards) {
        SharedPreferenceUtil.persistSimStates(mContext, simCards);
    }

    @Override
    public SimCard getSimBySubscriptionId(int subscriptionId) {
        final List<SimCard> sims = SharedPreferenceUtil.restoreSimStates(mContext, getSimCards());
        if (subscriptionId == SimCard.NO_SUBSCRIPTION_ID && !sims.isEmpty()) {
            return sims.get(0);
        }
        for (SimCard sim : getSimCards()) {
            if (sim.getSubscriptionId() == subscriptionId) {
                return sim;
            }
        }
        return null;
    }

    private ContentProviderResult[] importBatch(List<SimContact> contacts,
            AccountWithDataSet targetAccount)
            throws RemoteException, OperationApplicationException {
        final ArrayList<ContentProviderOperation> ops =
                createImportOperations(contacts, targetAccount);
        return mResolver.applyBatch(ContactsContract.AUTHORITY, ops);
    }

    @TargetApi(Build.VERSION_CODES.LOLLIPOP_MR1)
    private List<SimCard> getSimCardsFromSubscriptions() {
        final SubscriptionManager subscriptionManager = (SubscriptionManager)
                mContext.getSystemService(Context.TELEPHONY_SUBSCRIPTION_SERVICE);
        final List<SubscriptionInfo> subscriptions = subscriptionManager
                .getActiveSubscriptionInfoList();
        final ArrayList<SimCard> result = new ArrayList<>();
        if (subscriptions != null) {
            for (SubscriptionInfo subscriptionInfo : subscriptions) {
                result.add(SimCard.create(subscriptionInfo));
            }
        }
        Collections.sort(result);
        return result;
    }

    private List<SimContact> getContactsForSim(SimCard sim) {
        final List<SimContact> contacts = sim.getContacts();
        return contacts != null ? contacts : loadContactsForSim(sim);
    }

    // See b/32831092
    // Sometimes the SIM contacts provider seems to get stuck if read from multiple threads
    // concurrently. So we just have a global lock around it to prevent potential issues.
    private static final Object SIM_READ_LOCK = new Object();
    private ArrayList<SimContact> loadFrom(Uri uri) {
        synchronized (SIM_READ_LOCK) {
            final Cursor cursor = mResolver.query(uri, null, null, null, null);
            if (cursor == null) {
                // Assume null means there are no SIM contacts.
                return new ArrayList<>(0);
            }

            try {
                return loadFromCursor(cursor);
            } finally {
                cursor.close();
            }
        }
    }

    private ArrayList<SimContact> loadFromCursor(Cursor cursor) {
        final int colId = cursor.getColumnIndex(_ID);
        final int colName = cursor.getColumnIndex(NAME);
        final int colNumber = cursor.getColumnIndex(NUMBER);
        final int colEmails = cursor.getColumnIndex(EMAILS);
        final int colAnrs = cursor.getColumnIndex(ANRS);
        final ArrayList<SimContact> result = new ArrayList<>();

        while (cursor.moveToNext()) {
            final long id = cursor.getLong(colId);
            final String name = cursor.getString(colName);
            final String number = cursor.getString(colNumber);
            final String emails = cursor.getString(colEmails);
            String anrs = "";
            if (colAnrs >=0 )
                anrs = cursor.getString(colAnrs);
            final SimContact contact = new SimContact(id, name, number, parseEmails(emails),
                    parseAnrs(anrs));
            // Only include contact if it has some useful data
            if (contact.hasName() || contact.hasPhone() || contact.hasEmails()
                    || contact.hasAnrs()) {
                result.add(contact);
            }
        }
        Collections.sort(result, SimContact.compareByItemLabel());
        return result;
    }

    private ArrayList<ContentProviderOperation> createImportOperations(List<SimContact> contacts,
            AccountWithDataSet targetAccount) {
        final ArrayList<ContentProviderOperation> ops = new ArrayList<>();
        for (SimContact contact : contacts) {
            contact.appendCreateContactOperations(ops, targetAccount);
        }
        return ops;
    }

    private String[] parseEmails(String emails) {
        return !TextUtils.isEmpty(emails) ? emails.split(",") : null;
    }

    private String[] parseAnrs(String anrs) {
        return !TextUtils.isEmpty(anrs) ? anrs.split(":") : null;
    }

    private boolean hasTelephony() {
        return mContext.getPackageManager().hasSystemFeature(PackageManager.FEATURE_TELEPHONY);
    }

    private boolean hasPermissions() {
        return PermissionsUtil.hasContactsPermissions(mContext) &&
                PermissionsUtil.hasPhonePermissions(mContext);
    }

    // TODO remove this class and the USE_FAKE_INSTANCE flag once this code is not under
    // active development or anytime after 3/1/2017
    public static class DebugImpl extends SimContactDaoImpl {

        private List<SimCard> mSimCards = new ArrayList<>();
        private SparseArray<SimCard> mCardsBySubscription = new SparseArray<>();

        public DebugImpl(Context context) {
            super(context);
        }

        public DebugImpl addSimCard(SimCard sim) {
            mSimCards.add(sim);
            mCardsBySubscription.put(sim.getSubscriptionId(), sim);
            return this;
        }

        @Override
        public List<SimCard> getSimCards() {
            return SharedPreferenceUtil.restoreSimStates(getContext(), mSimCards);
        }

        @Override
        public ArrayList<SimContact> loadContactsForSim(SimCard card) {
            return new ArrayList<>(card.getContacts());
        }

        @Override
        public boolean canReadSimContacts() {
            return true;
        }
    }

    // Query used for detecting existing contacts that may match a SimContact.
    private static final class DataQuery {

        public static final String[] PROJECTION = new String[] {
                Data.RAW_CONTACT_ID, Phone.NUMBER, Data.DISPLAY_NAME, Data.MIMETYPE
        };

        public static final int RAW_CONTACT_ID = 0;
        public static final int PHONE_NUMBER = 1;
        public static final int DISPLAY_NAME = 2;
        public static final int MIMETYPE = 3;

        public static long getRawContactId(Cursor cursor) {
            return cursor.getLong(RAW_CONTACT_ID);
        }

        public static String getPhoneNumber(Cursor cursor) {
            return isPhoneNumber(cursor) ? cursor.getString(PHONE_NUMBER) : null;
        }

        public static String getDisplayName(Cursor cursor) {
            return cursor.getString(DISPLAY_NAME);
        }

        public static boolean isPhoneNumber(Cursor cursor) {
            return Phone.CONTENT_ITEM_TYPE.equals(cursor.getString(MIMETYPE));
        }
    }

    private static final class AccountQuery {
        public static final String[] PROJECTION = new String[] {
                RawContacts._ID, RawContacts.ACCOUNT_NAME, RawContacts.ACCOUNT_TYPE,
                RawContacts.DATA_SET
        };

        public static long getId(Cursor cursor) {
            return cursor.getLong(0);
        }

        public static AccountWithDataSet getAccount(Cursor cursor) {
            return new AccountWithDataSet(cursor.getString(1), cursor.getString(2),
                    cursor.getString(3));
        }
    }
}
