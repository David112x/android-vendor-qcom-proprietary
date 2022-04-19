/* ======================================================================
*  Copyright (c) 2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*  ====================================================================*/

package com.qti.wifidbprovider;

import android.os.Parcel;
import android.os.Parcelable;
import android.location.Location;
import com.qti.wwandbreceiver.BSInfo;
import com.qti.wifidbprovider.APScan;

public final class APObsLocData implements Parcelable {
    public Location mLocation;
    public BSInfo mCellInfo;
    public int mScanTimestamp;
    public APScan[] mScanList;

    public APObsLocData() {

    }

    public static final Parcelable.Creator<APObsLocData> CREATOR =
        new Parcelable.Creator<APObsLocData>() {
        public APObsLocData createFromParcel(Parcel in) {
            return new APObsLocData(in);
        }

        public APObsLocData[] newArray(int size) {
            return new APObsLocData[size];
        }
    };

    private APObsLocData(Parcel in) {
        readFromParcel(in);
    }

    public void writeToParcel(Parcel out, int flags) {
        out.writeTypedArray(mScanList, 0);
        out.writeByte((byte)(mLocation != null ? 1 : 0));
        if (null != mLocation) {
            mLocation.writeToParcel(out, flags);
        }
        out.writeByte((byte)(mCellInfo != null ? 1 : 0));
        if (null != mCellInfo) {
            mCellInfo.writeToParcel(out, flags);
        }
        out.writeInt(mScanTimestamp);
    }

    public void readFromParcel(Parcel in) {
        mScanList = in.createTypedArray(APScan.CREATOR);
        mLocation= (in.readByte() == 1) ? Location.CREATOR.createFromParcel(in) : null;
        mCellInfo= (in.readByte() == 1) ? BSInfo.CREATOR.createFromParcel(in) : null;
        mScanTimestamp = in.readInt();
    }

    public int describeContents() {
        return 0;
    }

}

