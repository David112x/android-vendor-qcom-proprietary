/*
* Copyright (c) 2019 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc.
*/
package com.qualcomm.qti.simcontacts.model;

import android.net.Uri;
import android.os.Parcel;
import android.os.Parcelable;

public class Contact implements Parcelable{

    private long mId;
    private String mName;
    private String mNumber;
    private String mLookupKey;
    private String mType;
    private String mLabel;
    private long mContactId;
    private String mAccountType;
    private String mAccountName;

    private Uri mPhotoUri;
    private long mPhotoId;

    public Contact(long id, String lookupKey, String name, long contactId,
                   long photoId, Uri photoUri){
        this(id, lookupKey, name, "", "", "", contactId, photoId, photoUri);
    }

    public Contact(long id, String lookupKey, String name, String number,
                   String type, String label, long contactId, long photoId, Uri photoUri){
        this(id, lookupKey, name, number, type, label, contactId, "", "",photoId, photoUri);
    }

    public Contact(long id, String lookupKey, String name, String number,
                   String type, String label, long contactId,
                   String accountType, String accountName,
                   long photoId, Uri photoUri){
        mId = id;
        mLookupKey = lookupKey;
        mName = name;
        mNumber = number;
        mType = type;
        mLabel = label;
        mContactId = contactId;
        mAccountName = accountName;
        mAccountType = accountType;
        mPhotoId = photoId;
        mPhotoUri = photoUri;
    }

    public long getId(){
        return mId;
    }

    public String getName(){
        return mName;
    }

    public String getNumber(){
        return mNumber;
    }

    public String getLookupKey(){
        return mLookupKey;
    }

    public String getType(){
        return mType;
    }

    public String getLabel(){
        return mLabel;
    }

    public long getContactId(){
        return mContactId;
    }

    public String getAccountType(){
        return mAccountType;
    }

    public String getAccountName(){
        return mAccountName;
    }

    public long getPhotoId(){
        return mPhotoId;
    }

    public Uri getPhotoUri(){
        return mPhotoUri;
    }

    protected Contact(Parcel in) {
        mId = in.readLong();
        mName = in.readString();
        mNumber = in.readString();
        mLookupKey = in.readString();
        mType = in.readString();
        mLabel = in.readString();
        mContactId = in.readLong();
        mAccountType = in.readString();
        mAccountName = in.readString();
        mPhotoUri = in.readParcelable(Uri.class.getClassLoader());
        mPhotoId = in.readLong();
    }

    @Override
    public void writeToParcel(Parcel dest, int flags) {
        dest.writeLong(mId);
        dest.writeString(mName);
        dest.writeString(mNumber);
        dest.writeString(mLookupKey);
        dest.writeString(mType);
        dest.writeString(mLabel);
        dest.writeLong(mContactId);
        dest.writeString(mAccountType);
        dest.writeString(mAccountName);
        dest.writeParcelable(mPhotoUri, flags);
        dest.writeLong(mPhotoId);
    }

    @Override
    public int describeContents() {
        return 0;
    }

    public static final Creator<Contact> CREATOR = new Creator<Contact>() {
        @Override
        public Contact createFromParcel(Parcel in) {
            return new Contact(in);
        }

        @Override
        public Contact[] newArray(int size) {
            return new Contact[size];
        }
    };
}
