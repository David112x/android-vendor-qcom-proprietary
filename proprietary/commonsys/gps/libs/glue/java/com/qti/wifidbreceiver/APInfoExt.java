/* ======================================================================
*  Copyright (c) 2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*  ====================================================================*/

package com.qti.wifidbreceiver;

import android.os.Parcel;
import android.os.Parcelable;

public final class APInfoExt implements Parcelable {
    public String mMacAddress;
    // use 0 if cell info not available, fill 0 to all IDs
    public int mCellType;
    public int mCellRegionID1;
    public int mCellRegionID2;
    public int mCellRegionID3;
    public int mCellRegionID4;
    // use null if SSID not available
    public byte[] mSSID;
    public int mTimestamp;
    // NOT_IN_FDAL, IN_FDAL, BLACKLISTED
    public int mFdalStatus;

    public APInfoExt() {

    }

    public static final Creator<APInfoExt> CREATOR =
        new Creator<APInfoExt>() {
        public APInfoExt createFromParcel(Parcel in) {
            return new APInfoExt(in);
        }

        public APInfoExt[] newArray(int size) {
            return new APInfoExt[size];
        }
    };

    private APInfoExt(Parcel in) {
        readFromParcel(in);
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeString(mMacAddress);
        out.writeInt(mCellType);
        out.writeInt(mCellRegionID1);
        out.writeInt(mCellRegionID2);
        out.writeInt(mCellRegionID3);
        out.writeInt(mCellRegionID4);
        out.writeByteArray(mSSID);
        out.writeInt(mTimestamp);
        out.writeInt(mFdalStatus);
    }

    public void readFromParcel(Parcel in) {
        mMacAddress = in.readString();
        mCellType = in.readInt();
        mCellRegionID1 = in.readInt();
        mCellRegionID2 = in.readInt();
        mCellRegionID3 = in.readInt();
        mCellRegionID4 = in.readInt();
        mSSID = in.createByteArray();
        mTimestamp = in.readInt();
        mFdalStatus = in.readInt();
    }

    public int describeContents() {
        return 0;
    }

}
