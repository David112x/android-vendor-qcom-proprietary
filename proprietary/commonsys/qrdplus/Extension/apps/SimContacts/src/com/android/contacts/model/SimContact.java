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
package com.android.contacts.model;

import android.content.ContentProviderOperation;
import android.database.MatrixCursor;
import android.net.Uri;
import android.os.Parcel;
import android.os.Parcelable;
import android.provider.ContactsContract;
import android.provider.ContactsContract.CommonDataKinds.Email;
import android.provider.ContactsContract.CommonDataKinds.Phone;
import android.provider.ContactsContract.CommonDataKinds.StructuredName;
import android.telephony.PhoneNumberUtils;
import android.text.TextUtils;

import com.android.contacts.model.account.AccountWithDataSet;

import java.util.Arrays;
import java.util.Collection;
import java.util.Comparator;
import java.util.List;
import java.util.Objects;

/**
 * Holds data for contacts loaded from the SIM card.
 */
public class SimContact implements Parcelable {
    private final long mId;
    private final String mName;
    private final String mPhone;
    private final String[] mEmails;
    private final String[] mAnrs;

    public SimContact(long id, String name, String phone) {
        this(id, name, phone, null);
    }

    public SimContact(long id, String name, String phone, String[] emails) {
        this(id, name, phone, emails, null);
    }

    public SimContact(String name, String phone, String[] emails, String[] anrs) {
        this(-1, name, phone, emails, anrs);
    }

    public SimContact(long id, String name, String phone, String[] emails, String[] anrs) {
        mId = id;
        mName = name;
        mPhone = phone == null ? "" : phone.trim();
        mEmails = emails;
        mAnrs = anrs;
    }

    public SimContact(SimContact other) {
        this(other.mId, other.mName, other.mPhone, other.mEmails, other.mAnrs);
    }

    public long getId() {
        return mId;
    }

    public String getName() {
        return mName;
    }

    public String getPhone() {
        return mPhone;
    }

    public String[] getEmails() {
        return mEmails;
    }

    public String[] getAnrs() {
        return mAnrs;
    }

    public String getAnrsString(){
        if (mAnrs != null && mAnrs.length > 0){
            StringBuilder buffer = new StringBuilder();
            for (String anr : mAnrs){
                buffer.append(anr);
                buffer.append(":");
            }
            return buffer.toString();
        }
        return "";
    }

    public String getEmailsString(){
        if (mEmails != null && mEmails.length > 0){
            StringBuilder buffer = new StringBuilder();
            for (String email : mEmails){
                buffer.append(email);
                buffer.append(",");
            }
            return buffer.toString();
        }
        return "";
    }

    public String getItemLabel() {
        if (hasName()) {
            return getName();
        } else if (hasPhone()) {
            return getPhone();
        } else if (hasEmails()) {
            return getEmails()[0];
        } else if(hasAnrs()) {
            return getAnrs()[0];
        } else {
            // This isn't really possible because we skip empty SIM contacts during loading
            return "";
        }
    }

    public StringBuilder SimContactToBuilder(){
        StringBuilder buf = new StringBuilder();
        String number = null;
        String emails = ArrayToString(mEmails, ',');
        String anrs = ArrayToString(mAnrs, ':');
        if (!TextUtils.isEmpty(mPhone))
            number = PhoneNumberUtils.stripSeparators(mPhone);
        if (anrs != null)
            anrs = anrs.replaceAll("[^0123456789PWN\\,\\;\\*\\#\\+\\:]", "");
        if (!TextUtils.isEmpty(mName)) {
            buf.append("tag='");
            buf.append(mName);
            buf.append("'");
        }
        if (!TextUtils.isEmpty(number)) {
            buf.append(" AND number='");
            buf.append(number);
            buf.append("'");
        }
        if (!TextUtils.isEmpty(emails)) {
            buf.append(" AND emails='");
            buf.append(emails);
            buf.append("'");
        }
        if (!TextUtils.isEmpty(anrs)) {
            buf.append(" AND anrs='");
            buf.append(anrs);
            buf.append("'");
        }
        return buf;
    }

    private String ArrayToString(String[] array, char separator) {
        StringBuilder sb = new StringBuilder();
        if (array != null) {
            for (String string : array) {
                sb.append(string);
                sb.append(separator);
            }
        }
        return sb.toString();
    }

    public void appendCreateContactOperations(List<ContentProviderOperation> ops,
            AccountWithDataSet targetAccount) {
        // There is nothing to save so skip it.
        if (!hasName() && !hasPhone() && !hasEmails() && !hasAnrs()) return;

        final int rawContactOpIndex = ops.size();
        ops.add(ContentProviderOperation.newInsert(ContactsContract.RawContacts.CONTENT_URI)
                .withYieldAllowed(true)
                .withValue(ContactsContract.RawContacts.ACCOUNT_NAME, targetAccount.name)
                .withValue(ContactsContract.RawContacts.ACCOUNT_TYPE, targetAccount.type)
                .withValue(ContactsContract.RawContacts.DATA_SET, targetAccount.dataSet)
                .withValue(ContactsContract.RawContacts.AGGREGATION_MODE,
                        ContactsContract.RawContacts.AGGREGATION_MODE_DISABLED)
                .build());
        if (mName != null) {
            ops.add(createInsertOp(rawContactOpIndex, StructuredName.CONTENT_ITEM_TYPE,
                    StructuredName.DISPLAY_NAME, mName));
        }
        if (!mPhone.isEmpty()) {
            ops.add(createInsertOp(rawContactOpIndex, Phone.CONTENT_ITEM_TYPE,
                    Phone.NUMBER, mPhone));
        }
        if (mEmails != null) {
            for (String email : mEmails) {
                ops.add(createInsertOp(rawContactOpIndex, Email.CONTENT_ITEM_TYPE,
                        Email.ADDRESS, email));
            }
        }
        if (mAnrs != null) {
            for (String anr : mAnrs) {
                ops.add(createInsertOp(rawContactOpIndex, Phone.CONTENT_ITEM_TYPE,
                        Phone.NUMBER, anr));
            }
        }
    }

    private ContentProviderOperation createInsertOp(int rawContactOpIndex, String mimeType,
            String column, String value) {
        return ContentProviderOperation.newInsert(ContactsContract.Data.CONTENT_URI)
                .withValueBackReference(ContactsContract.Data.RAW_CONTACT_ID, rawContactOpIndex)
                .withValue(ContactsContract.Data.MIMETYPE, mimeType)
                .withValue(column, value)
                .build();
    }

    public void appendAsContactRow(MatrixCursor cursor) {
        cursor.newRow().add(ContactsContract.Contacts._ID, mId)
                .add(ContactsContract.Contacts.DISPLAY_NAME_PRIMARY, mName)
                .add(ContactsContract.Contacts.LOOKUP_KEY, getLookupKey());
    }

    public boolean hasName() {
        return !isEmptyAfterTrim(mName);
    }

    public boolean hasPhone() {
        return !isEmptyAfterTrim(mPhone);
    }

    public boolean hasEmails() {
        return mEmails != null && mEmails.length > 0 && !isEmptyAfterTrim(mEmails[0]);
    }

    public boolean hasAnrs() {
        return mAnrs != null && mAnrs.length > 0 && !isEmptyAfterTrim(mAnrs[0]);
    }

    private boolean isEmptyAfterTrim(String str){
        return (str == null) || (str.trim().length() == 0);
    }

    /**
     * Generate a "fake" lookup key. This is needed because
     * {@link ContactPhotoManager} will only generate a letter avatar
     * if the contact has a lookup key.
     */
    public String getLookupKey() {
        if (mName != null) {
            return "sim-n-" + Uri.encode(mName);
        } else if (mPhone != null) {
            return "sim-p-" + Uri.encode(mPhone);
        } else {
            return null;
        }
    }

    @Override
    public String toString() {
        return "SimContact{" +
                "mId=" + mId +
                ", mName='" + mName + '\'' +
                ", mPhone='" + mPhone + '\'' +
                ", mEmails=" + Arrays.toString(mEmails) +
                ", mAnrs=" + Arrays.toString(mAnrs) +
                '}';
    }

    @Override
    public boolean equals(Object o) {
        if (this == o) return true;
        if (o == null || getClass() != o.getClass()) return false;

        final SimContact that = (SimContact) o;

        return mId == that.mId && Objects.equals(mName, that.mName) &&
                Objects.equals(mPhone, that.mPhone) && Arrays.equals(mEmails, that.mEmails)
                && Arrays.equals(mAnrs, that.mAnrs);
    }

    @Override
    public int hashCode() {
        int result = (int) (mId ^ (mId >>> 32));
        result = 31 * result + (mName != null ? mName.hashCode() : 0);
        result = 31 * result + (mPhone != null ? mPhone.hashCode() : 0);
        result = 31 * result + Arrays.hashCode(mEmails);
        result = 31 * result + Arrays.hashCode(mAnrs);
        return result;
    }

    @Override
    public int describeContents() {
        return 0;
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeLong(mId);
        dest.writeString(mName);
        dest.writeString(mPhone);
        dest.writeStringArray(mEmails);
        dest.writeStringArray(mAnrs);
    }

    public static final Creator<SimContact> CREATOR = new Creator<SimContact>() {
        @Override
        public SimContact createFromParcel(Parcel source) {
            final long id = source.readLong();
            final String name = source.readString();
            final String phone = source.readString();
            final String[] emails = source.createStringArray();
            final String[] anrs = source.createStringArray();
            return new SimContact(id, name, phone, emails, anrs);
        }

        @Override
        public SimContact[] newArray(int size) {
            return new SimContact[size];
        }
    };

    /**
     * Convert a collection of SIM contacts to a Cursor matching a query from
     * {@link ContactsContract.Contacts#CONTENT_URI} with the provided projection.
     *
     * This allows a collection of SIM contacts to be displayed using the existing adapters for
     * contacts.
     */
    public static final MatrixCursor convertToContactsCursor(Collection<SimContact> contacts,
            String[] projection) {
        final MatrixCursor result = new MatrixCursor(projection);
        for (SimContact contact : contacts) {
            contact.appendAsContactRow(result);
        }
        return result;
    }

    public static final Comparator<SimContact> compareById() {
        return new Comparator<SimContact>() {
            @Override
            public int compare(SimContact lhs, SimContact rhs) {
                // We assume ids are unique.
                return Long.compare(lhs.mId, rhs.mId);
            }
        };
    }

    public static final Comparator<SimContact> compareByItemLabel() {
        return new Comparator<SimContact>() {
            @Override
            public int compare(SimContact lhs, SimContact rhs) {
                return lhs.getItemLabel().toLowerCase().
                        compareTo(rhs.getItemLabel().toLowerCase());
            }
        };
    }
}
