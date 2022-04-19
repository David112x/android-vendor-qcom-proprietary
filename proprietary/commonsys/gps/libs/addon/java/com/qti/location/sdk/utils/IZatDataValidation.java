/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qti.location.sdk.utils;

import android.location.Location;
import com.qti.location.sdk.IZatFlpService;

import com.qti.location.sdk.IZatGeofenceService;
import com.qti.location.sdk.IZatWiFiDBProvider;
import com.qti.location.sdk.IZatWiFiDBReceiver;
import com.qti.location.sdk.IZatWWANDBProvider;
import com.qti.location.sdk.IZatWWANDBReceiver;
import com.qti.location.sdk.IZatDBCommon;
import com.qti.location.sdk.IZatIllegalArgumentException;
import com.qti.location.sdk.IZatStaleDataException;
import com.qti.location.sdk.IZatSecurityException;
import com.qti.location.sdk.utils.IZatValidationResults;
import com.qti.location.sdk.utils.IZatValidationResults.IZatDataTypes;
import com.qti.geofence.GeofenceData;
import java.util.HashSet;
import java.util.List;
import android.util.Log;

public class IZatDataValidation {
    private static String TAG = "IZatDataValidation";
    private static final boolean isValidationEnabled = Log.isLoggable (TAG, Log.DEBUG);

    public static IZatValidationResults dataValidate(int para, IZatDataTypes types) {
        if (!isValidationEnabled) {
            return new IZatValidationResults(true);
        } else {
            return IZatDataValidationInteger.dataValidate(para, types);
        }
    }


    public static IZatValidationResults dataValidate(long para, IZatDataTypes types) {
        if (!isValidationEnabled) {
            return new IZatValidationResults(true);
        } else {
            return IZatDataValidationInteger.dataValidate(para, types);
        }
    }

    public static IZatValidationResults dataValidate(float para, IZatDataTypes types) {
        if (!isValidationEnabled) {
            return new IZatValidationResults(true);
        } else {
            return IZatDataValidationFloat.dataValidate(para, types);
        }
    }

    public static IZatValidationResults dataValidate(String str, IZatDataTypes types) {
        if (!isValidationEnabled) {
            return new IZatValidationResults(true);
        } else {
            return IZatDataValidationString.dataValidate(str, types);
        }
    }


    public static IZatValidationResults dataValidate(Location location) {
        IZatValidationResults validateResult = new IZatValidationResults(true);

        if (!isValidationEnabled) {
            return validateResult;
        } else if (location == null) {
            validateResult.setResult(false);
            validateResult.setInfo("location object is null");
            return validateResult;
        }

        validateResult.append(dataValidate((float)location.getLatitude(),
                              IZatDataTypes.LOCATION_LATITUDE));
        validateResult.append(dataValidate((float)location.getLongitude(),
                              IZatDataTypes.LOCATION_LONGITUDE));
        if (location.hasSpeed())
            validateResult.append(dataValidate(location.getSpeed(),
                                  IZatDataTypes.LOCATION_SPEED));
        if (location.hasBearing())
            validateResult.append(dataValidate(location.getBearing(),
                                  IZatDataTypes.LOCATION_BEARING));
        if (location.hasAccuracy())
            validateResult.append(dataValidate(location.getAccuracy(),
                                  IZatDataTypes.LOCATION_ACCURACY));
        validateResult.append(dataValidate(location.getTime(), IZatDataTypes.LOCATION_TIME));

        return validateResult;
    }

    public static IZatValidationResults dataValidate(Location[] locations) {
        IZatValidationResults validateResult = new IZatValidationResults(true);

        if (!isValidationEnabled) {
            return validateResult;
        }

        for(Location loc : locations) {
            validateResult.append(dataValidate(loc));
        }
        return validateResult;
    }

    public static IZatValidationResults dataValidate(IZatFlpService.IzatFlpRequest flpRequest) {
        IZatValidationResults validateResult = new IZatValidationResults(true);

        if (!isValidationEnabled) {
            return validateResult;
        } else if (flpRequest == null) {
            validateResult.setResult(false);
            validateResult.setInfo("flpRequest object is null");
            return validateResult;
        }

        if (!dataValidate(flpRequest.getDistanceInterval(),
                          IZatDataTypes.FLP_DISTANCE_INTERVAL).getResult() &&
            !dataValidate(flpRequest.getTripDistance(),
                          IZatDataTypes.FLP_TRIP_DISTANCE).getResult() &&
            !dataValidate(flpRequest.getTimeInterval(),
                          IZatDataTypes.FLP_TIME_INTERVAL).getResult()) {
            validateResult.setResult(false);
            validateResult.setIsException(true);
            validateResult.setInfo("None of FLP session interval is set");
        }

        validateResult.processResult();
        return validateResult;
    }


    public static IZatValidationResults dataValidate(IZatGeofenceService.IzatGeofence geofence) {
        IZatValidationResults validateResult = new IZatValidationResults(true);

        if (!isValidationEnabled) {
            return validateResult;
        } else if (geofence == null) {
            validateResult.setResult(false);
            validateResult.setInfo("geofence object is null");
            return validateResult;
        }

        validateResult.append(dataValidate((float)geofence.getLatitude(),
                                           IZatDataTypes.LOCATION_LATITUDE));
        validateResult.append(dataValidate((float)geofence.getLongitude(),
                                           IZatDataTypes.LOCATION_LONGITUDE));
        validateResult.append(dataValidate((float)geofence.getRadius(),
                                           IZatDataTypes.GEO_RADIUS));
        if ((geofence.getUpdatedFieldsMask() & GeofenceData.RESPONSIVENESS_IS_SET) != 0) {
            validateResult.append(dataValidate(geofence.getNotifyResponsiveness(),
                                               IZatDataTypes.GEO_RESPONSIVENESS));
        }

        if ((geofence.getUpdatedFieldsMask() & GeofenceData.DWELL_NOTIFY_IS_SET) != 0) {
            validateResult.append(dataValidate(geofence.getDwellNotify().getDwellTime(),
                                               IZatDataTypes.GEO_DWELL_TIME));
        }

        if (!validateResult.getResult()) {
            validateResult.setIsException(true);
            validateResult.processResult();
        }
        return validateResult;
    }

    public static IZatValidationResults dataValidate(IZatDBCommon.IZatCellInfo cellInfo) {
        IZatValidationResults validateResult = new IZatValidationResults(true);

        if (!isValidationEnabled) {
            return validateResult;
        } else if (cellInfo == null) {
            validateResult.setResult(false);
            validateResult.setInfo("cellInfo object is null");
            return validateResult;
        }

        validateResult.append(dataValidate(cellInfo.getRegionID1(),
                              IZatDataTypes.WWAN_CELL_REGIONID1));

        if (cellInfo.getType() == IZatDBCommon.IZatCellTypes.CDMA) {
            validateResult.append(dataValidate(cellInfo.getRegionID2(),
                                  IZatDataTypes.WWAN_CELL_REGIONID2_CDMA));
        } else {
            validateResult.append(dataValidate(cellInfo.getRegionID2(),
                                  IZatDataTypes.WWAN_CELL_REGIONID2_OTHERS));
        }

        if (cellInfo.getType() == IZatDBCommon.IZatCellTypes.CDMA ||
            cellInfo.getType() == IZatDBCommon.IZatCellTypes.GSM) {
            validateResult.append(dataValidate(cellInfo.getRegionID3(),
                                               IZatDataTypes.WWAN_CELL_REGIONID3_CDMA_GSM));
        } else {
            validateResult.append(dataValidate(cellInfo.getRegionID3(),
                                               IZatDataTypes.WWAN_CELL_REGIONID3_WCDMA_LTE));
        }

        if (cellInfo.getType() == IZatDBCommon.IZatCellTypes.CDMA ||
            cellInfo.getType() == IZatDBCommon.IZatCellTypes.GSM) {
            validateResult.append(dataValidate(cellInfo.getRegionID4(),
                                               IZatDataTypes.WWAN_CELL_REGIONID4_CDMA_GSM));
        } else {
            validateResult.append(dataValidate(cellInfo.getRegionID4(),
                                               IZatDataTypes.WWAN_CELL_REGIONID4_WCDMA_LTE));
        }

        return validateResult;
    }


    public static IZatValidationResults dataValidate(IZatWiFiDBReceiver.IZatAPInfo apInfo) {
        IZatValidationResults validateResult = new IZatValidationResults(true);

        if (!isValidationEnabled) {
            return validateResult;
        } else if (apInfo == null) {
            validateResult.setResult(false);
            validateResult.setInfo("apInfo object is null");
            return validateResult;
        }

        validateResult.append(dataValidate(apInfo.getMacAddress(),
                                           IZatDataTypes.WIFI_MAC_ADDRESS));
        validateResult.append(dataValidate((long)apInfo.getTimestamp(),
                                           IZatDataTypes.LOCATION_TIME));

        if (apInfo.isExtraAvailable()) {
            validateResult.append(dataValidate(apInfo.getExtra().getCellInfo()));
        }

        return validateResult;
    }

    public static IZatValidationResults dataValidate(
                IZatWiFiDBReceiver.IZatAPLocationData apLocInfo) {
        IZatValidationResults validateResult = new IZatValidationResults(true);

        if (!isValidationEnabled) {
            return validateResult;
        } else if (apLocInfo == null) {
            validateResult.setResult(false);
            validateResult.setInfo("apLocInfo object is null");
            return validateResult;
        }

        validateResult.append(dataValidate(apLocInfo.getMacAddress(),
                                           IZatDataTypes.WIFI_MAC_ADDRESS));
        validateResult.append(dataValidate(apLocInfo.getLatitude(),
                                           IZatDataTypes.LOCATION_LATITUDE));
        validateResult.append(dataValidate(apLocInfo.getLongitude(),
                                           IZatDataTypes.LOCATION_LONGITUDE));
        try {
            validateResult.append(dataValidate(apLocInfo.getMaxAntenaRange(),
                                               IZatDataTypes.WIFI_MAX_ANTENARANGE));
        } catch (IZatStaleDataException sde) {
            Log.w(TAG, "MAR exception");
        }

        try {
            validateResult.append(dataValidate(apLocInfo.getHorizontalError(),
                                               IZatDataTypes.LOCATION_ACCURACY));
        } catch (IZatStaleDataException sde) {
            Log.w(TAG, "HEPE exception");
        }

        return validateResult;
    }

    public static IZatValidationResults dataValidate(
                IZatWiFiDBReceiver.IZatAPSpecialInfo apSpecialInfo) {
        IZatValidationResults validateResult = new IZatValidationResults(true);

        if (!isValidationEnabled) {
            return validateResult;
        } else if (apSpecialInfo == null) {
            validateResult.setResult(false);
            validateResult.setInfo("apSpecialInfo object is null");
            return validateResult;
        }

        validateResult.append(dataValidate(apSpecialInfo.mMacAddress,
                              IZatDataTypes.WIFI_MAC_ADDRESS));

        return validateResult;
    }

    public static IZatValidationResults dataValidate(IZatWiFiDBProvider.IZatAPScan apScan) {
        IZatValidationResults validateResult = new IZatValidationResults(true);

        if (!isValidationEnabled) {
            return validateResult;
        } else if (apScan == null) {
            validateResult.setResult(false);
            validateResult.setInfo("apScan object is null");
            return validateResult;
        }

        validateResult.append(dataValidate(apScan.getMacAddress(),
                              IZatDataTypes.WIFI_MAC_ADDRESS));
        validateResult.append(dataValidate(apScan.getRssi(),
                              IZatDataTypes.WIFI_RSSI));
        validateResult.append(dataValidate(apScan.getDeltatime(),
                              IZatDataTypes.WIFI_DELTA_TIME));
        validateResult.append(dataValidate(apScan.getAPChannelNumber(),
                              IZatDataTypes.WIFI_CHANNEL_NUM));

        return validateResult;
    }

    public static IZatValidationResults dataValidate(
                IZatWiFiDBProvider.IZatAPObsLocData apObsLocData) {
        IZatValidationResults validateResult = new IZatValidationResults(true);

        if (!isValidationEnabled) {
            return validateResult;
        } else if (apObsLocData == null) {
            validateResult.setResult(false);
            validateResult.setInfo("apObsLocData object is null");
            return validateResult;
        }

        validateResult.append(dataValidate(apObsLocData.getLocation()));
        validateResult.append(dataValidate(apObsLocData.getCellInfo()));

        validateResult.append(dataValidate(apObsLocData.getScanTimestamp(),
                                           IZatDataTypes.LOCATION_TIME));

        List<IZatWiFiDBProvider.IZatAPScan> apScanList = apObsLocData.getApScanList();

        for(IZatWiFiDBProvider.IZatAPScan apScan : apScanList) {
            validateResult.append(dataValidate(apScan));
        }

        return validateResult;
    }

    public static IZatValidationResults dataValidate(
                IZatWWANDBProvider.IZatBSObsLocationData bsObsLocData) {
        IZatValidationResults validateResult = new IZatValidationResults(true);

        if (!isValidationEnabled) {
            return validateResult;
        } else if (bsObsLocData == null) {
            validateResult.setResult(false);
            validateResult.setInfo("bsObsLocData object is null");
            return validateResult;
        }

        try {
            validateResult.append(dataValidate(bsObsLocData.getLocation()));
        } catch (IZatStaleDataException sde) {
            Log.w(TAG, "LOCATION exception");
        }

        try {
            validateResult.append(dataValidate(bsObsLocData.getIZatCellInfo()));
        } catch (IZatStaleDataException sde) {
            Log.w(TAG, "CELL INFO exception");
        }

        try {
            validateResult.append(dataValidate(bsObsLocData.getTimestamp(),
                                               IZatDataTypes.LOCATION_TIME));
        } catch (IZatStaleDataException sde) {
            Log.w(TAG, "TIME exception");
        }

        return validateResult;
    }

    public static IZatValidationResults dataValidate(
                IZatWWANDBReceiver.IZatBSLocationData bsObsLocData) {
        IZatValidationResults validateResult = new IZatValidationResults(true);

        if (!isValidationEnabled) {
            return validateResult;
        } else if (bsObsLocData == null) {
            validateResult.setResult(false);
            validateResult.setInfo("bsObsLocData object is null");
            return validateResult;
        }

        try {
            validateResult.append(dataValidate(bsObsLocData.getLocation()));
        } catch (IZatStaleDataException sde) {
            Log.w(TAG, "LOCATION exception");
        }

        try {
            validateResult.append(dataValidate(bsObsLocData.getIZatCellInfo()));
        } catch (IZatStaleDataException sde) {
            Log.w(TAG, "CELL INFO exception");
        }

        try {
            validateResult.append(dataValidate(bsObsLocData.getHorizontalCoverageRadius(),
                                               IZatDataTypes.WWAN_HORIZONTAL_COV_RADIUS));
        } catch (IZatStaleDataException sde) {
            Log.w(TAG, "HORIZONTAL COV RADIUS exception");
        }

        return validateResult;
    }

    public static IZatValidationResults dataValidate(
                IZatWWANDBReceiver.IZatBSSpecialInfo bsSpecialInfo) {
        IZatValidationResults validateResult = new IZatValidationResults(true);

        if (!isValidationEnabled) {
            return validateResult;
        } else if (bsSpecialInfo == null) {
            validateResult.setResult(false);
            validateResult.setInfo("bsSpecialInfo object is null");
            return validateResult;
        }

        validateResult.append(dataValidate(bsSpecialInfo.getCellInfo()));

        return validateResult;
    }

    public static IZatValidationResults dataValidate(
                IZatWWANDBProvider.IZatBSSpecialInfo bsSpecialInfo) {
        IZatValidationResults validateResult = new IZatValidationResults(true);

        if (!isValidationEnabled) {
            return validateResult;
        } else if (bsSpecialInfo == null) {
            validateResult.setResult(false);
            validateResult.setInfo("bsSpecialInfo object is null");
            return validateResult;
        }

        validateResult.append(dataValidate(bsSpecialInfo.getCellInfo()));

        return validateResult;
    }

    public static IZatValidationResults dataValidate(IZatWWANDBReceiver.IZatBSInfo bsInfo) {
        IZatValidationResults validateResult = new IZatValidationResults(true);

        if (!isValidationEnabled) {
            return validateResult;
        } else if (bsInfo == null) {
            validateResult.setResult(false);
            validateResult.setInfo("bsInfo object is null");
            return validateResult;
        }

        validateResult.append(dataValidate((IZatDBCommon.IZatCellInfo)bsInfo.getCellInfo()));

        return validateResult;
    }

}
