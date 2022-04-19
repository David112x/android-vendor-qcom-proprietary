/* ======================================================================
*  Copyright (c) 2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*  ====================================================================*/

package com.qti.wwandbprovider;

import android.location.Location;
import android.os.Parcel;
import android.os.Parcelable;

import com.qti.wwandbreceiver.BSInfo;

public final class BSObsLocationData implements Parcelable {
    public Location mLocation;
    public BSInfo mCellInfo;
    public int mScanTimestamp;

    public BSObsLocationData() {

    }

    public static final Parcelable.Creator<BSObsLocationData> CREATOR =
            new Parcelable.Creator<BSObsLocationData>() {
                public BSObsLocationData createFromParcel(Parcel in) {
                    return new BSObsLocationData(in);
                }

                public BSObsLocationData[] newArray(int size) {
                    return new BSObsLocationData[size];
                }
            };

    private BSObsLocationData(Parcel in) {
        readFromParcel(in);
    }

    public void writeToParcel(Parcel out, int flags) {
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
        mLocation= (in.readByte() == 1) ? Location.CREATOR.createFromParcel(in) : null;
        mCellInfo= (in.readByte() == 1) ? BSInfo.CREATOR.createFromParcel(in) : null;
        mScanTimestamp = in.readInt();
    }

    public int describeContents() {
        return 0;
    }

}
