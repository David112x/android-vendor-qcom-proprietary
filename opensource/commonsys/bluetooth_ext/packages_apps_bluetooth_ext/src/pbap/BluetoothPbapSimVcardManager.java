/*
* Copyright (c) 2015, The Linux Foundation. All rights reserved.
* Not a Contribution.
* Copyright (C) 2014 Samsung System LSI
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
package com.android.bluetooth.pbap;

import com.android.bluetooth.R;

import android.content.ContentResolver;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteException;
import android.net.Uri;
import com.android.vcard.VCardBuilder;
import com.android.vcard.VCardConfig;
import com.android.vcard.VCardConstants;
import com.android.vcard.VCardUtils;

import android.content.ContentValues;
import android.provider.CallLog;
import android.provider.CallLog.Calls;
import android.text.TextUtils;
import android.text.format.Time;
import android.util.Log;
import android.provider.ContactsContract.Contacts;
import android.provider.ContactsContract.CommonDataKinds;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.CommonDataKinds.StructuredName;

import java.io.IOException;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;
import java.util.Collections;
import java.util.Comparator;

import javax.obex.Operation;
import javax.obex.ResponseCodes;
import javax.obex.ServerOperation;

/**
 * VCard composer especially for Call Log used in Bluetooth.
 */
public class BluetoothPbapSimVcardManager {
    private static final String TAG = "PbapSIMvCardComposer";

    private static final boolean V = BluetoothPbapService.VERBOSE;

    private static final String FAILURE_REASON_FAILED_TO_GET_DATABASE_INFO =
        "Failed to get database information";

    private static final String FAILURE_REASON_NO_ENTRY =
        "There's no exportable in the database";

    private static final String FAILURE_REASON_NOT_INITIALIZED =
        "The vCard composer object is not correctly initialized";

    /** Should be visible only from developers... (no need to translate, hopefully) */
    private static final String FAILURE_REASON_UNSUPPORTED_URI =
        "The Uri vCard composer received is not supported by the composer.";

    private static final String NO_ERROR = "No error";

    private final String SIM_URI = "content://icc/adn";

    private static final String[] SIM_PROJECTION = new String[] {
        Contacts.DISPLAY_NAME,
        CommonDataKinds.Phone.NUMBER,
        CommonDataKinds.Phone.TYPE,
        CommonDataKinds.Phone.LABEL
    };

    interface SimPaths {

        String SIM_PATH = "/SIM1/telecom";

        String SIM_ICH_PATH = "/SIM1/telecom/ich";

        String SIM_OCH_PATH = "/SIM1/telecom/och";

        String SIM_MCH_PATH = "/SIM1/telecom/mch";

        String SIM_CCH_PATH = "/SIM1/telecom/cch";

        String SIM_PB_PATH = "/SIM1/telecom/pb";

    }

    private static final int NAME_COLUMN_INDEX = 0;
    private static final int NUMBER_COLUMN_INDEX = 1;
    private static final int NUMBERTYPE_COLUMN_INDEX = 2;
    private static final int NUMBERLABEL_COLUMN_INDEX = 3;


    private final Context mContext;
    private ContentResolver mContentResolver;
    private Cursor mCursor;
    private boolean mTerminateIsCalled;
    private String mErrorReason = NO_ERROR;

    public BluetoothPbapSimVcardManager(final Context context) {
        mContext = context;
        mContentResolver = context.getContentResolver();
    }

    public boolean init(final Uri contentUri, final String selection,
            final String[] selectionArgs, final String sortOrder) {
            final Uri myUri = Uri.parse(SIM_URI);
        if (!myUri.equals(contentUri)) {

            mErrorReason = FAILURE_REASON_UNSUPPORTED_URI;
            return false;
        }

        //checkpoint Figure out if we can apply selection, projection and sort order.
        mCursor = mContentResolver.query(
                contentUri, SIM_PROJECTION, null, null,sortOrder);

        if (mCursor == null) {
            mErrorReason = FAILURE_REASON_FAILED_TO_GET_DATABASE_INFO;
            return false;
        }
        if (mCursor.getCount() == 0 || !mCursor.moveToFirst()) {
            try {
                mCursor.close();
            } catch (SQLiteException e) {
                Log.e(TAG, "SQLiteException on Cursor#close(): " + e.getMessage());
            } finally {
                mErrorReason = FAILURE_REASON_NO_ENTRY;
                mCursor = null;
            }
            return false;
        }
        return true;
    }

    public String createOneEntry(boolean vcardVer21) {
        if (mCursor == null || mCursor.isAfterLast()) {
            mErrorReason = FAILURE_REASON_NOT_INITIALIZED;
            return null;
        }
        try {
            return createOnevCardEntryInternal(vcardVer21);
        } finally {
            mCursor.moveToNext();
        }
    }

    private String createOnevCardEntryInternal(boolean vcardVer21) {
        final int vcardType = (vcardVer21 ? VCardConfig.VCARD_TYPE_V21_GENERIC :
                VCardConfig.VCARD_TYPE_V30_GENERIC) |
                VCardConfig.FLAG_REFRAIN_PHONE_NUMBER_FORMATTING;
        final VCardBuilder builder = new VCardBuilder(vcardType);
        String name = mCursor.getString(NAME_COLUMN_INDEX);
        if (TextUtils.isEmpty(name)) {
            name = mCursor.getString(NUMBER_COLUMN_INDEX);
        }
        final boolean needCharset = !(VCardUtils.containsOnlyPrintableAscii(name));
        // Create ContentValues for making name as Structured name
        List<ContentValues> contentValuesList = new ArrayList<ContentValues>();
        ContentValues nameContentValues = new ContentValues();
        nameContentValues.put(StructuredName.DISPLAY_NAME, name);
        contentValuesList.add(nameContentValues);
        builder.appendNameProperties(contentValuesList);

        String number = mCursor.getString(NUMBER_COLUMN_INDEX);
        if (TextUtils.isEmpty(number)) {
            // To avoid Spec violation and IOT issues, intialise with invalid number
            number = "000000";
        }
        if (number.equals("-1")) {
            number = mContext.getString(R.string.unknownNumber);
        }

        // checkpoint Figure out what are the type and label
        int type = mCursor.getInt(NUMBERTYPE_COLUMN_INDEX);
        String label = mCursor.getString(NUMBERLABEL_COLUMN_INDEX);
        if (type == 0) { // value for type is not present in db
            type = Phone.TYPE_MOBILE;
        }
        if (TextUtils.isEmpty(label)) {
            label = Integer.toString(type);
        }
        builder.appendTelLine(type, label, number, false);
        return builder.toString();
    }

    public void terminate() {
        if (mCursor != null) {
            try {
                mCursor.close();
            } catch (SQLiteException e) {
                Log.e(TAG, "SQLiteException on Cursor#close(): " + e.getMessage());
            }
            mCursor = null;
        }

        mTerminateIsCalled = true;
    }

    @Override
    public void finalize() {
        if (!mTerminateIsCalled) {
            terminate();
        }
    }

    public int getCount() {
        if (mCursor == null) {
            return 0;
        }
        return mCursor.getCount();
    }

    public boolean isAfterLast() {
        if (mCursor == null) {
            return false;
        }
        return mCursor.isAfterLast();
    }

    public void moveToPosition(final int position, boolean sortalpha) {
        if(mCursor == null) {
            return;
        }
        if(sortalpha) {
            setpositionbyalpha(position);
            return;
        }
        mCursor.moveToPosition(position);
    }

    public String getErrorReason() {
        return mErrorReason;
    }

    public void setpositionbyalpha(int position) {
        if(mCursor == null) {
            return;
        }
        ArrayList<String> nameList = new ArrayList<String>();
        for (mCursor.moveToFirst(); !mCursor.isAfterLast(); mCursor
                        .moveToNext()) {
            String name = mCursor.getString(NAME_COLUMN_INDEX);
            if (TextUtils.isEmpty(name)) {
                name = mContext.getString(android.R.string.unknownName);
            }
            nameList.add(name);
        }

        Collections.sort(nameList, new Comparator <String> () {
            @Override
            public int compare(String str1, String str2){
                return str1.compareToIgnoreCase(str2);
            }
        });

        for (mCursor.moveToFirst(); !mCursor.isAfterLast(); mCursor.moveToNext()) {
            if(mCursor.getString(NAME_COLUMN_INDEX).equals(nameList.get(position))) {
              break;
            }

        }
    }

    public final int getSIMContactsSize() {
        final Uri myUri = Uri.parse(SIM_URI);
        int size = 0;
        Cursor contactCursor = null;
        try {
            contactCursor = mContentResolver.query(myUri, SIM_PROJECTION, null,null, null);
            if (contactCursor != null) {
                size = contactCursor.getCount() +1;  //always has the 0.vcf
            }
        } finally {
            if (contactCursor != null) {
                contactCursor.close();
            }
        }
        return size;
    }

    public final ArrayList<String> getSIMPhonebookNameList(final int orderByWhat) {
        ArrayList<String> nameList = new ArrayList<String>();
        nameList.add(BluetoothPbapService.getLocalPhoneName());
        //Since owner card should always be 0.vcf, maintaing a separate list to avoid sorting
        ArrayList<String> allnames = new ArrayList<String>();
        final Uri myUri = Uri.parse(SIM_URI);
        Cursor contactCursor = null;
        try {
            contactCursor = mContentResolver.query(myUri, SIM_PROJECTION, null,null,null);
            if (contactCursor != null) {
                for (contactCursor.moveToFirst(); !contactCursor.isAfterLast(); contactCursor
                        .moveToNext()) {
                    String name = contactCursor.getString(NAME_COLUMN_INDEX);
                    if (TextUtils.isEmpty(name)) {
                        name = mContext.getString(android.R.string.unknownName);
                    }
                    allnames.add(name);
                }
            }
        } finally {
            if (contactCursor != null) {
                contactCursor.close();
            }
        }
        if (orderByWhat == BluetoothPbapObexServer.ORDER_BY_INDEXED) {
            if (V) Log.v(TAG, "getPhonebookNameList, order by index");
        } else if (orderByWhat == BluetoothPbapObexServer.ORDER_BY_ALPHABETICAL) {
            if (V) Log.v(TAG, "getPhonebookNameList, order by alpha");
            Collections.sort(allnames, new Comparator <String> () {
                @Override
                public int compare(String str1, String str2) {
                    return str1.compareToIgnoreCase(str2);
                }
            });
        }

        nameList.addAll(allnames);
        return nameList;

    }

    public final ArrayList<String> getSIMContactNamesByNumber(
            final String phoneNumber) {
        ArrayList<String> nameList = new ArrayList<String>();
        ArrayList<String> startNameList = new ArrayList<String>();
        Cursor contactCursor = null;
        final Uri uri = Uri.parse(SIM_URI);

        try {
            contactCursor = mContentResolver.query(uri, SIM_PROJECTION,
                    null, null, null);

            if (contactCursor != null) {
                for (contactCursor.moveToFirst(); !contactCursor.isAfterLast(); contactCursor
                        .moveToNext()) {
                    String number = contactCursor.getString(NUMBER_COLUMN_INDEX);
                    if (number == null) {
                        if (V) Log.v(TAG, "number is null");
                        continue;
                    }

                    if (V) Log.v(TAG, "number: " + number + " phoneNumber:" + phoneNumber);
                    if ((number.endsWith(phoneNumber)) || (number.startsWith(phoneNumber))) {
                        String name = contactCursor.getString(NAME_COLUMN_INDEX);
                        if (TextUtils.isEmpty(name)) {
                            name = mContext.getString(android.R.string.unknownName);
                        }
                        if (V) Log.v(TAG, "got name " + name + " by number " + phoneNumber);

                        if (number.endsWith(phoneNumber)) {
                            if (V) Log.v(TAG, "Adding to end name list");
                            nameList.add(name);
                        } else {
                            if (V) Log.v(TAG, "Adding to start name list");
                            startNameList.add(name);
                        }
                    }
                }
            }
        } finally {
            if (contactCursor != null) {
                contactCursor.close();
            }
        }
        int startListSize = startNameList.size();
        for (int index = 0; index < startListSize; index++) {
            String object = startNameList.get(index);
            if (!nameList.contains(object))
                nameList.add(object);
        }

        return nameList;
    }

    public final int composeAndSendSIMPhonebookVcards(Operation op,
            final int startPoint, final int endPoint, final boolean vcardType21,
            String ownerVCard) {
        if (startPoint < 1 || startPoint > endPoint) {
            Log.e(TAG, "internal error: startPoint or endPoint is not correct.");
            return ResponseCodes.OBEX_HTTP_INTERNAL_ERROR;
        }
        final Uri myUri = Uri.parse(SIM_URI);
        BluetoothPbapSimVcardManager composer = null;
        HandlerForStringBuffer buffer = null;
        try {
            composer = new BluetoothPbapSimVcardManager(mContext);
            buffer = new HandlerForStringBuffer(op, ownerVCard);

            if (!composer.init(myUri, null, null, null) || !buffer.onInit(mContext)) {
                return ResponseCodes.OBEX_HTTP_INTERNAL_ERROR;
            }
            composer.moveToPosition(startPoint -1, false);
            for (int count =startPoint -1; count < endPoint; count++) {
                if (BluetoothPbapObexServer.sIsAborted) {
                    ((ServerOperation)op).isAborted = true;
                    BluetoothPbapObexServer.sIsAborted = false;
                    break;
                }
                String vcard = composer.createOneEntry(vcardType21);
                if (vcard == null) {
                    Log.e(TAG, "Failed to read a contact. Error reason: "
                            + composer.getErrorReason());
                    return ResponseCodes.OBEX_HTTP_INTERNAL_ERROR;
                }
                buffer.onEntryCreated(vcard);
            }
        } finally {
            if (composer != null) {
                composer.terminate();
            }
            if (buffer != null) {
                buffer.onTerminate();
            }
        }
        return ResponseCodes.OBEX_HTTP_OK;
    }

    /**
     * Handler to emit vCards to PCE.
     */
    public class HandlerForStringBuffer {
        private Operation operation;

        private OutputStream outputStream;

        private String phoneOwnVCard = null;

        public HandlerForStringBuffer(Operation op, String ownerVCard) {
            operation = op;
            if (ownerVCard != null) {
                phoneOwnVCard = ownerVCard;
                if (V) Log.v(TAG, "phone own number vcard:");
                if (V) Log.v(TAG, phoneOwnVCard);
            }
        }

        private boolean write(String vCard) {
            try {
                if (vCard != null) {
                    outputStream.write(vCard.getBytes());
                    return true;
                }
            } catch (IOException e) {
                Log.e(TAG, "write outputstrem failed" + e.toString());
            }
            return false;
        }

        public boolean onInit(Context context) {
            try {
                outputStream = operation.openOutputStream();
                if (phoneOwnVCard != null) {
                    return write(phoneOwnVCard);
                }
                return true;
            } catch (IOException e) {
                Log.e(TAG, "open outputstrem failed" + e.toString());
            }
            return false;
        }

        public boolean onEntryCreated(String vcard) {
            return write(vcard);
        }

        public void onTerminate() {
            if (!BluetoothPbapObexServer.closeStream(outputStream, operation)) {
                if (V) Log.v(TAG, "CloseStream failed!");
            } else {
                if (V) Log.v(TAG, "CloseStream ok!");
            }
        }
    }

    public final int composeAndSendSIMPhonebookOneVcard(Operation op,
            final int offset, final boolean vcardType21, String ownerVCard,
            int orderByWhat) {
        if (offset < 1) {
            Log.e(TAG, "Internal error: offset is not correct.");
            return ResponseCodes.OBEX_HTTP_INTERNAL_ERROR;
        }
        final Uri myUri = Uri.parse(SIM_URI);

        BluetoothPbapSimVcardManager composer = null;
        HandlerForStringBuffer buffer = null;
        try {
            composer = new BluetoothPbapSimVcardManager(mContext);
            buffer = new HandlerForStringBuffer(op, ownerVCard);
            if (!composer.init(myUri, null, null,null)||
                               !buffer.onInit(mContext)) {
                return ResponseCodes.OBEX_HTTP_INTERNAL_ERROR;
            }
            if (orderByWhat == BluetoothPbapObexServer.ORDER_BY_INDEXED) {
                if (V) Log.v(TAG, "getPhonebookNameList, order by index");
                composer.moveToPosition(offset -1, false);
            } else if (orderByWhat == BluetoothPbapObexServer.ORDER_BY_ALPHABETICAL) {
                if (V) Log.v(TAG, "getPhonebookNameList, order by alpha");
                composer.moveToPosition(offset -1, true);
            }
            if (BluetoothPbapObexServer.sIsAborted) {
                ((ServerOperation)op).isAborted = true;
                 BluetoothPbapObexServer.sIsAborted = false;
            }
            String vcard = composer.createOneEntry(vcardType21);
            if (vcard == null) {
                Log.e(TAG, "Failed to read a contact. Error reason: "
                            + composer.getErrorReason());
                return ResponseCodes.OBEX_HTTP_INTERNAL_ERROR;
            }
            buffer.onEntryCreated(vcard);
        } finally {
            if (composer != null) {
                composer.terminate();
            }
            if (buffer != null) {
                buffer.onTerminate();
            }
        }

        return ResponseCodes.OBEX_HTTP_OK;
    }

    protected boolean isSimPhoneBook(String name, String type, String PB,
            String SIM1, String TYPE_PB, String TYPE_LISTING, String  mCurrentPath) {

        return ((name.contains(PB.subSequence(0, PB.length()))
                && name.contains(SIM1.subSequence(0,
                        SIM1.length())))
                && (type.equals(TYPE_PB)))
                || (((name.contains(
                        PB.subSequence(0, PB.length())))
                        && (mCurrentPath.equals(SimPaths.SIM_PATH)))
                        && (type.equals(TYPE_LISTING)));
    }

    protected String getType(String searchAttr) {
        String type = "";
        if (searchAttr.equals("0")) {
            type = "name";
        } else if (searchAttr.equals("1")) {
            type = "number";
        }
        return type;
    }

    /* to request SIM phonebook */
    public int initiatePullSimPhonebook(int startPoint, int endPoint,
            boolean vcard21, Operation op, BluetoothPbapVcardManager mVcardManager,
            boolean ignorefilter, byte[] propertySelector, BluetoothPbapObexServer server) {
        if (startPoint == 0) {
            String ownerVcard = mVcardManager.getOwnerPhoneNumberVcard(vcard21,
                    propertySelector);
            if (endPoint == 0) {
                return server.pushBytes(op, ownerVcard);
            } else {
                return composeAndSendSIMPhonebookVcards(op, 1, endPoint,
                    vcard21, ownerVcard);
            }
        } else {
            return composeAndSendSIMPhonebookVcards(op, startPoint, endPoint, vcard21, null);
        }
    }

    /* to request SIM vCard entry*/
    protected int initiatePullSimVcardEntry(int intIndex, int size, boolean vcard21,
            int mOrderBy, String name, Operation op, BluetoothPbapVcardManager mVcardManager,
            boolean ignorefilter, byte[] propertySelector, BluetoothPbapObexServer server) {
        if (intIndex < 0 || intIndex >= size) {
            Log.w(TAG, "The requested vcard is not acceptable! name= " + name);
            return ResponseCodes.OBEX_HTTP_NOT_FOUND;
        } else if (intIndex == 0) {
            // For PB_PATH, 0.vcf is the phone number of this phone.
            String ownerVcard = mVcardManager.getOwnerPhoneNumberVcard(vcard21,
                    propertySelector);
            return server.pushBytes(op, ownerVcard);
        } else {
            return composeAndSendSIMPhonebookOneVcard(op, intIndex, vcard21, null, mOrderBy);
        }
    }

    /* to fetch contact names for the search value. */
    protected ArrayList<String> retrieveContactNamesByNumber(boolean isSim,
            BluetoothPbapVcardManager mVcardManager, String searchValue) {
        ArrayList<String> names = null;
        if (isSim) {
            return getSIMContactNamesByNumber(searchValue);
        } else {
            return mVcardManager.getContactNamesByNumber(searchValue);
        }
    }
}
