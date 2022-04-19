/* ======================================================================
*  Copyright (c) 2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*  ====================================================================*/

package com.qti.wifidbprovider;

import android.location.Location;
import android.os.Parcel;
import android.os.Parcelable;

import com.qti.wwandbreceiver.BSInfo;

public final class APScan implements Parcelable {
    public String mMacAddress;
    public float mRssi;
    public int mDeltaTime;
    public byte[] mSSID;
    public int mChannelNumber;

    public APScan() {

    }

    public static final Creator<APScan> CREATOR =
        new Creator<APScan>() {
        public APScan createFromParcel(Parcel in) {
            return new APScan(in);
        }

        public APScan[] newArray(int size) {
            return new APScan[size];
        }
    };

    private APScan(Parcel in) {
        readFromParcel(in);
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeString(mMacAddress);
        out.writeFloat(mRssi);
        out.writeInt(mDeltaTime);
        out.writeByteArray(mSSID);
        out.writeInt(mChannelNumber);
    }

    public void readFromParcel(Parcel in) {
        mMacAddress = in.readString();
        mRssi = in.readFloat();
        mDeltaTime = in.readInt();
        mSSID = in.createByteArray();
        mChannelNumber = in.readInt();
    }

    public int describeContents() {
        return 0;
    }

}

