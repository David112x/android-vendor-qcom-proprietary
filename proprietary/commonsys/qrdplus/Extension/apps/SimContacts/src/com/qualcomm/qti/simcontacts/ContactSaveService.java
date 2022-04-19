/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
package com.qualcomm.qti.simcontacts;

import android.app.IntentService;
import android.content.ContentUris;
import android.content.Context;
import android.content.Intent;
import android.content.OperationApplicationException;
import android.database.Cursor;
import android.net.Uri;
import android.os.Bundle;
import android.os.Handler;
import android.os.Looper;
import android.os.RemoteException;
import android.provider.ContactsContract.CommonDataKinds.Email;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.CommonDataKinds.StructuredName;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.Data;
import android.support.v4.os.ResultReceiver;
import android.text.TextUtils;
import android.util.Log;
import android.widget.Toast;

import com.android.contacts.database.SimContactDao;
import com.android.contacts.model.SimContact;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.Set;

public class ContactSaveService extends IntentService {

    public static final String ACTION_DELETE_MULTIPLE_CONTACTS = "deleteMultipleContacts";
    public static final String ACTION_IMPORT_CONTACTS_FROM_DEVICE = "importContactsFromDevice";
    public static final String ACTION_EXPORT_CONTACTS_TO_DEVICE = "exportContactsToDevice";
    public static final String ACTION_CREATE_NEW_CONTACT = "createNewContact";
    public static final String ACTION_EDIT_EXISTING_CONTACT = "editExistingContact";
    public static final String EXTRA_OLD_SIM_CONTACT = "oldSimContact";
    public static final String EXTRA_SIM_CONTACT = "simContact";
    public static final String EXTRA_CONTACTS = "contacts";
    public static final String EXTRA_SIM_CONTACTS = "simContacts";
    public static final String EXTRA_RESULT_RECEIVER = "resultReceiver";
    public static final String EXTRA_SIM_SUBSCRIPTION_ID = "subscriptionId";

    public static final int CONTACTS_DELETE_STARTED = 0;
    public static final int CONTACTS_DELETE_INCREMENT = 1;
    public static final int CONTACTS_DELETE_COMPLETE = 2;

    public static final int CONTACTS_IMPORT_STARTED = 0;
    public static final int CONTACTS_IMPORT_COMPLETE = 1;

    public static final int CONTACTS_EXPORT_STARTED = 0;
    public static final int CONTACTS_EXPORT_COMPLETE = 1;
    public static final int CONTACTS_EXPORT_FAILURE = 2;

    public static final int CONTACTS_OPERATION_STARTED = 0;
    public static final int CONTACTS_OPERATION_COMPLETE = 1;
    public static final int CONTACTS_OPERATION_FAILURE = 2;

    private static final String[] DATA_PROJECTION = new String[] {
            Contacts._ID, // 0
            Data.MIMETYPE, //1
            Data.DATA1, // 2 Phone.NUMBER, Email.address
    };

    private static final int DATA_MIMETYPE_COLUMN = 1;
    private static final int DATA_DATA1_COLUMN = 2;

    private static final String TAG = "ContactSaveService";

    private SimContactDao mSimContactDao;
    private Handler mUIHandler;

    public ContactSaveService(){
        super(TAG);
    }


    public ContactSaveService(String name) {
        super(name);
    }

    @Override
    public void onCreate() {
        super.onCreate();
        mSimContactDao = SimContactDao.create(this);
        mUIHandler = new Handler(Looper.getMainLooper());
    }

    @Override
    protected void onHandleIntent(Intent intent) {
        if (intent == null){
            return;
        }

        String action = intent.getAction();
        if (ACTION_DELETE_MULTIPLE_CONTACTS.equals(action)){
            deleteMultipleContacts(intent);
        }else if (ACTION_IMPORT_CONTACTS_FROM_DEVICE.equals(action)){
            importContactsFromDevice(intent);
        }else if (ACTION_EXPORT_CONTACTS_TO_DEVICE.equals(action)){
            exportContactsToDevice(intent);
        }else if (ACTION_CREATE_NEW_CONTACT.equals(action)){
            createNewContact(intent);
        }else if (ACTION_EDIT_EXISTING_CONTACT.equals(action)){
            editExistingContact(intent);
        }
    }

    public static Intent createEditExistingContactIntent(Context context, SimContact oldSimContact,
                  SimContact newSimContact, int subscriptionId, ResultReceiver receiver){
        Intent intent = new Intent(context, ContactSaveService.class);
        intent.setAction(ContactSaveService.ACTION_EDIT_EXISTING_CONTACT);
        intent.putExtra(ContactSaveService.EXTRA_OLD_SIM_CONTACT, oldSimContact);
        intent.putExtra(ContactSaveService.EXTRA_SIM_CONTACT, newSimContact);
        intent.putExtra(ContactSaveService.EXTRA_SIM_SUBSCRIPTION_ID, subscriptionId);
        intent.putExtra(ContactSaveService.EXTRA_RESULT_RECEIVER, receiver);
        return intent;
    }

    public static Intent createCreateNewContactIntent(Context context, SimContact simContact,
                   int subscriptionId, ResultReceiver receiver){
        Intent intent = new Intent(context, ContactSaveService.class);
        intent.setAction(ContactSaveService.ACTION_CREATE_NEW_CONTACT);
        intent.putExtra(ContactSaveService.EXTRA_SIM_CONTACT, simContact);
        intent.putExtra(ContactSaveService.EXTRA_SIM_SUBSCRIPTION_ID, subscriptionId);
        intent.putExtra(ContactSaveService.EXTRA_RESULT_RECEIVER, receiver);
        return intent;
    }

    public static Intent createDeleteContactIntent(Context context,
                  ArrayList<SimContact> simContacts,
                  int subscriptionId, ResultReceiver receiver){
        Intent intent = new Intent(context, ContactSaveService.class);
        intent.setAction(ContactSaveService.ACTION_DELETE_MULTIPLE_CONTACTS);
        intent.putParcelableArrayListExtra(ContactSaveService.EXTRA_SIM_CONTACTS, simContacts);
        intent.putExtra(ContactSaveService.EXTRA_SIM_SUBSCRIPTION_ID, subscriptionId);
        intent.putExtra(ContactSaveService.EXTRA_RESULT_RECEIVER, receiver);
        return intent;
    }

    private void editExistingContact(Intent intent){
        final SimContact oldSimContact = intent.getParcelableExtra(
                EXTRA_OLD_SIM_CONTACT);
        final SimContact newSimContact = intent.getParcelableExtra(
                EXTRA_SIM_CONTACT);
        final int subscriptionId = intent.getIntExtra(EXTRA_SIM_SUBSCRIPTION_ID, -1);
        if (oldSimContact == null || newSimContact == null || subscriptionId == -1) {
            Log.e(TAG, "Invalid arguments for edit existing contact request");
            return;
        }
        final ResultReceiver receiver = intent.getParcelableExtra(
                ContactSaveService.EXTRA_RESULT_RECEIVER);
        notifyActionProgress(CONTACTS_OPERATION_STARTED, receiver);
        int result = mSimContactDao.editExistingSimContact(oldSimContact, newSimContact,
                subscriptionId);
        Log.d(TAG, "editExistingContact result = "+result);
        notifyActionProgress(result != 0 ? CONTACTS_OPERATION_COMPLETE :
                CONTACTS_OPERATION_FAILURE , receiver);
    }

    private void createNewContact(Intent intent){
        final SimContact simContact = intent.getParcelableExtra(
                EXTRA_SIM_CONTACT);
        final int subscriptionId = intent.getIntExtra(EXTRA_SIM_SUBSCRIPTION_ID, -1);
        if (simContact == null || subscriptionId == -1) {
            Log.e(TAG, "Invalid arguments for create new contact request");
            return;
        }
        final ResultReceiver receiver = intent.getParcelableExtra(
                ContactSaveService.EXTRA_RESULT_RECEIVER);
        notifyActionProgress(CONTACTS_OPERATION_STARTED, receiver);
        Uri result = mSimContactDao.createNewSimContact(simContact, subscriptionId);
        Log.d(TAG, "createNewContact result = "+result);
        notifyActionProgress(result != null ? CONTACTS_OPERATION_COMPLETE :
                CONTACTS_OPERATION_FAILURE , receiver);
    }

    private void deleteMultipleContacts(Intent intent){
        final ArrayList<SimContact> simContacts = intent.getParcelableArrayListExtra(
                EXTRA_SIM_CONTACTS);
        final int subscriptionId = intent.getIntExtra(EXTRA_SIM_SUBSCRIPTION_ID, -1);
        if (simContacts == null || simContacts.size() == 0 || subscriptionId == -1) {
            Log.e(TAG, "Invalid arguments for deleteMultipleContacts request");
            return;
        }
        final ResultReceiver receiver = intent.getParcelableExtra(
                ContactSaveService.EXTRA_RESULT_RECEIVER);
        notifyActionProgress(CONTACTS_DELETE_STARTED, receiver);
        for (SimContact simContact :  simContacts){
            boolean success = mSimContactDao.deleteSimContact(simContact, subscriptionId);
            notifyActionProgress(CONTACTS_DELETE_INCREMENT, receiver);
            if (!success){
                Log.e(TAG," delete SIM contact ["+simContact.getItemLabel()+"] failure");
            }
        }
        notifyActionProgress(CONTACTS_DELETE_COMPLETE, receiver);
    }

    private void notifyActionProgress(int state, ResultReceiver receiver){
        if (receiver != null) {
            receiver.send(state, new Bundle());
        }
    }

    public static Intent createSimExportIntent(Context context, ArrayList<SimContact> contacts,
                         ResultReceiver receiver){
        Intent intent = new Intent(context, ContactSaveService.class);
        intent.setAction(ContactSaveService.ACTION_EXPORT_CONTACTS_TO_DEVICE);
        intent.putParcelableArrayListExtra(EXTRA_SIM_CONTACTS, contacts);
        intent.putExtra(ContactSaveService.EXTRA_RESULT_RECEIVER, receiver);
        return intent;
    }

    private void exportContactsToDevice(Intent intent){
        final ArrayList<SimContact> simContacts = intent.getParcelableArrayListExtra(
                EXTRA_SIM_CONTACTS);
        if (simContacts == null || simContacts.size() == 0) {
            Log.e(TAG, "Invalid arguments for exportContactsToDevice request");
            return;
        }
        final ResultReceiver receiver = intent.getParcelableExtra(
                ContactSaveService.EXTRA_RESULT_RECEIVER);
        notifyActionProgress(CONTACTS_EXPORT_STARTED, receiver);
        try {
            //default export to device.
            mSimContactDao.exportContacts(simContacts);
            notifyActionProgress(CONTACTS_EXPORT_COMPLETE, receiver);
        }catch (RemoteException |OperationApplicationException e) {
            e.printStackTrace();
            Log.e(TAG, "exportContactsToDevice "+ e.toString());
            notifyActionProgress(CONTACTS_EXPORT_FAILURE, receiver);
        }
    }

    public static Intent createSimImportIntent(Context context, Bundle contacts,
                    int subscriptionId, ResultReceiver receiver){
        Intent intent = new Intent(context, ContactSaveService.class);
        intent.setAction(ContactSaveService.ACTION_IMPORT_CONTACTS_FROM_DEVICE);
        intent.putExtra(ContactSaveService.EXTRA_CONTACTS, contacts);
        intent.putExtra(ContactSaveService.EXTRA_SIM_SUBSCRIPTION_ID, subscriptionId);
        intent.putExtra(ContactSaveService.EXTRA_RESULT_RECEIVER, receiver);
        return intent;
    }

    private void importContactsFromDevice(Intent intent){
        final Bundle contacts = intent.getBundleExtra(EXTRA_CONTACTS);
        final int subscriptionId = intent.getIntExtra(EXTRA_SIM_SUBSCRIPTION_ID, -1);
        if (contacts == null || contacts.size() == 0 || subscriptionId == -1){
            Log.e(TAG, "Invalid arguments for importContactsFromDevice request");
            return;
        }
        final ResultReceiver receiver = intent.getParcelableExtra(
                ContactSaveService.EXTRA_RESULT_RECEIVER);
        boolean isSimCardFull = false;
        int insertCount = 0;
        final AdnCapacityCache adnCapacityCache = new AdnCapacityCache(subscriptionId);
        Set<String> contactIds = contacts.keySet();
        Iterator<String> iterator = contactIds.iterator();
        notifyActionProgress(CONTACTS_EXPORT_STARTED, receiver);
        while (iterator.hasNext()){
            String name = "";
            ArrayList<String> numbers = new ArrayList<>();
            ArrayList<String> emails = new ArrayList<>();
            String contactId = String.valueOf(iterator.next());
            Cursor cursor = getContactDataCursor(contactId);
            try {
                if (cursor != null && cursor.moveToFirst()) {
                    do {
                        final String mimeType = cursor.getString(DATA_MIMETYPE_COLUMN);
                        if (StructuredName.CONTENT_ITEM_TYPE.equals(mimeType)) {
                            name = cursor.getString(DATA_DATA1_COLUMN);
                        }else if (Phone.CONTENT_ITEM_TYPE.equals(mimeType)) {
                            final String number = cursor.getString(DATA_DATA1_COLUMN);
                            if (!TextUtils.isEmpty(number) && adnCapacityCache.mEmptyNumber-- > 0) {
                                numbers.add(number);
                            }
                        }else if (adnCapacityCache.mCanSaveEmail
                                && Email.CONTENT_ITEM_TYPE.equals(mimeType)) {
                            final String email = cursor.getString(DATA_DATA1_COLUMN);
                            if (!TextUtils.isEmpty(email)
                                    && adnCapacityCache.mEmptyEmail-- > 0) {
                                emails.add(email);
                            }
                        }
                    } while (cursor.moveToNext());
                }
            } finally {
                if (cursor != null) {
                    cursor.close();
                }
            }

            if (adnCapacityCache.mFreeSimCount-- > 0 && 0 == numbers.size() && 0 == emails.size()) {
                showToastMessage(getString(R.string.contact_no_phone_or_email, name));
                continue;
            }
            int groupCount = adnCapacityCache.calculateCandidateAdnCount(name, numbers.size(),
                    emails.size());

            Uri result;
            for (int k = 0; k < groupCount; k++) {
                if (adnCapacityCache.mFreeSimCount > 0) {
                    String num = numbers.size() > 0 ? numbers.remove(0) : null;
                    ArrayList<String> anrsField = new ArrayList<>();
                    ArrayList<String> emailsFiled = new ArrayList<>();
                    if (adnCapacityCache.mCanSaveAnr) {
                        for (int j = 1; j < adnCapacityCache.mPhoneCountInOneSimContact; j++) {
                            if (numbers.size() > 0 && adnCapacityCache.mEmptyAnr-- > 0) {
                                anrsField.add(numbers.remove(0));
                            }
                        }
                    }
                    if (adnCapacityCache.mCanSaveEmail) {
                        for (int j = 0; j < adnCapacityCache.mEmailCountInOneSimContact; j++) {
                            if (emails.size() > 0) {
                                emailsFiled.add(emails.remove(0));
                            }
                        }
                    }

                    SimContact simContact = new SimContact(name, num,
                            emailsFiled.toArray(new String[emailsFiled.size()]),
                            anrsField.toArray(new String[anrsField.size()]));
                    result = mSimContactDao.createNewSimContact(simContact, subscriptionId);
                    if (null == result) {
                        adnCapacityCache.updateCacheFieldsAfterFailed(anrsField.size(),
                                emailsFiled.size(), num);
                        showToastMessage(getString(R.string.import_device_contact_failed,
                                TextUtils.isEmpty(name) ? "" : name,
                                TextUtils.isEmpty(num) ? "" : num,
                                emailsFiled.size() == 0 ? "" : emailsFiled.toString()));

                    }else {
                        Log.d(TAG, "Exported contact [" + name + ", " + contactId + "] " +
                                "to subId " + subscriptionId);
                        insertCount++;
                        adnCapacityCache.mFreeSimCount--;
                    }
                } else {
                    isSimCardFull = true;
                    showToastMessage(getString(R.string.sim_card_full, insertCount));
                    break;
                }
            }
            if (isSimCardFull) {
                break;
            }
        }
        if (!isSimCardFull) {
            showToastMessage(getString(R.string.import_finished));
        }
        notifyActionProgress(CONTACTS_EXPORT_COMPLETE, receiver);
    }

    private void showToastMessage(final String message){
        mUIHandler.post(new Runnable() {
            @Override
            public void run() {
                Toast.makeText(getApplicationContext(), message, Toast.LENGTH_SHORT).show();
            }
        });
    }

    private Cursor getContactDataCursor(String contactId){
        Uri dataUri = Uri.withAppendedPath(ContentUris.withAppendedId(Contacts.CONTENT_URI,
                Long.parseLong(contactId)), Contacts.Data.CONTENT_DIRECTORY);
        return getContentResolver().query(dataUri, DATA_PROJECTION, null, null, null);
    }

    private class AdnCapacityCache{
        private final boolean mCanSaveAnr;
        private final boolean mCanSaveEmail;
        private final int mEmailCountInOneSimContact;
        private final int mPhoneCountInOneSimContact;
        private int mFreeSimCount;
        private int mEmptyAnr;
        private int mEmptyEmail;
        private int mEmptyNumber;

        private AdnCapacityCache(int subscriptionId){
            mFreeSimCount = ContactUtils.getSimFreeCount(subscriptionId);
            mCanSaveAnr = ContactUtils.canSaveAnr(subscriptionId);
            mCanSaveEmail = ContactUtils.canSaveEmail(subscriptionId);
            mEmailCountInOneSimContact = ContactUtils.getOneSimEmailCount(subscriptionId);
            mPhoneCountInOneSimContact = ContactUtils.getOneSimAnrCount(
                    subscriptionId) + 1;
            mEmptyAnr = ContactUtils.getSpareAnrCount(subscriptionId);
            mEmptyEmail = ContactUtils.getSpareEmailCount(subscriptionId);
            mEmptyNumber = mFreeSimCount + mEmptyAnr;
        }

        private int calculateCandidateAdnCount(String name, int numberCount, int emailCount){
            int nameCount = (name != null && !name.equals("")) ? 1 : 0;
            int groupNumCount = (numberCount % mPhoneCountInOneSimContact) != 0 ?
                    (numberCount / mPhoneCountInOneSimContact + 1)
                    : (numberCount / mPhoneCountInOneSimContact);
            int groupEmailCount = mEmailCountInOneSimContact == 0 ? 0
                    : ((emailCount % mEmailCountInOneSimContact) != 0 ?
                    (emailCount / mEmailCountInOneSimContact + 1)
                    : (emailCount / mEmailCountInOneSimContact));
            // recalute the group when spare anr is not enough
            if (mCanSaveAnr && mEmptyAnr >= 0 && mEmptyAnr <= groupNumCount) {
                groupNumCount = numberCount - mEmptyAnr;
            }
            return Math.max(groupEmailCount, Math.max(nameCount, groupNumCount));
        }

        private void updateCacheFieldsAfterFailed(int anrsSize,
                     int emailsSize, String number){
            // Failed to insert to SIM card
            // reset emptyNumber and emptyAnr to the value before the insert operation
            mEmptyAnr += anrsSize;
            mEmptyNumber += anrsSize;
            if (!TextUtils.isEmpty(number)) {
                mEmptyNumber++;
            }
            // reset emptyEmail to the value before the insert operation
            mEmptyEmail += emailsSize;
        }
    }
}
