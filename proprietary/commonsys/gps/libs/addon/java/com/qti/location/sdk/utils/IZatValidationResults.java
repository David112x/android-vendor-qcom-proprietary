/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qti.location.sdk.utils;

import com.qti.location.sdk.IZatIllegalArgumentException;
import com.qti.location.sdk.IZatStaleDataException;
import com.qti.location.sdk.IZatSecurityException;
import android.util.Log;
import java.lang.StringBuilder;

public class IZatValidationResults {
    private static String TAG = "IZatDataValidation";
    private static final boolean DEBUG = Log.isLoggable (TAG, Log.DEBUG);
    private static final boolean VERBOSE = Log.isLoggable (TAG, Log.VERBOSE);

    public enum IZatDataTypes {
        //LOCATION values
        LOCATION_TIME,
        LOCATION_LATITUDE,
        LOCATION_LONGITUDE,
        LOCATION_ALTITUDE,
        LOCATION_SPEED,
        LOCATION_BEARING,
        LOCATION_ACCURACY,

        //FLP values
        FLP_STATUS,
        FLP_TIME_INTERVAL,
        FLP_DISTANCE_INTERVAL,
        FLP_TRIP_DISTANCE,
        FLP_POWER_MODE,
        FLP_TBM_MILLIS,

        //Geofence values
        GEO_RESPONSIVENESS,
        GEO_RADIUS,
        GEO_FIELDS_MASK,
        GEO_EVENT,
        GEO_REQUEST_TYPE,
        GEO_ERROR_CODE,
        GEO_ENGINE_STATUS,
        GEO_DWELL_TIME,

        //WiFi DB
        WIFI_EXPIRE_DAYS,
        WIFI_DAYS_VALID,
        WIFI_MAC_ADDRESS,
        WIFI_MAX_ANTENARANGE,
        WIFI_RSSI,
        WIFI_DELTA_TIME,
        WIFI_CHANNEL_NUM,

        //WWAN DB
        WWAN_CELL_REGIONID1,
        WWAN_CELL_REGIONID2_CDMA,
        WWAN_CELL_REGIONID2_OTHERS,
        WWAN_CELL_REGIONID3_CDMA_GSM,
        WWAN_CELL_REGIONID3_WCDMA_LTE,
        WWAN_CELL_REGIONID4_CDMA_GSM,
        WWAN_CELL_REGIONID4_WCDMA_LTE,
        WWAN_HORIZONTAL_COV_RADIUS;

    }

    private boolean mResult;
    private boolean mRaiseException = VERBOSE;
    private StringBuilder mInfo;

    public IZatValidationResults(boolean result) {
        this(result,"");
    }

    public IZatValidationResults(boolean result, String info) {
        mResult = result;
        mInfo = new StringBuilder(info);
    }

    public void setResult(boolean result) {
        mResult = result;
    }

    public boolean getResult() {
        return mResult;
    }
    public void setIsException(boolean isException) {
        mRaiseException = isException;
    }

    public boolean IsException() {
        return mRaiseException;
    }

    public void setInfo(String info) {
        mInfo.setLength(0);
        mInfo.append(info);
    }

    public String getInfo() {
        return mInfo.toString();
    }

    // Merge two validation results to one reuslt
    public void append(IZatValidationResults appendResult) {
        if (!appendResult.getResult()) {
            mResult = false;
            mInfo.append('\n');
            mInfo.append(appendResult.getInfo());
        }
    }

    /* Set log.tag.IZatDataValidation = VERBOSE in local.prop to enable the exception.
       Set log.tag.IZatDataValidation = DEBUG in local.prop to only enable the fail log print
       and disable the exception throw.*/
    public void processResult () {
        if (mResult) {
            return;
        }

        if (mRaiseException) {
            throw new IZatIllegalArgumentException(mInfo.toString());
        } else if (DEBUG) {
            Log.d(TAG, "Fail Info: " + mInfo.toString());
        }
    }
}

