/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qti.location.sdk.utils;

import com.qti.location.sdk.utils.IZatValidationResults;
import com.qti.location.sdk.utils.IZatValidationResults.IZatDataTypes;
import java.util.HashSet;
import java.util.List;

public class IZatDataValidationInteger {
    private static String TAG = "IZatDataValidation";

    public static HashSet<Integer> getSetFromArray(int[] dataArray) {
        HashSet<Integer> dataSet = new HashSet<Integer>();
        for(int i : dataArray){
            dataSet.add(i);
        }

        return dataSet;
    }

    public static boolean dataSetValidate(int para, HashSet<Integer> dataRange) {
        for (int i : dataRange) {
            if (i == para)
                return true;
        }
        return false;
    }

    public static IZatValidationResults dataValidate(int para, IZatDataTypes types) {
        String failInfo = null;
        switch (types) {
            case FLP_STATUS:
                if (dataSetValidate(para,getSetFromArray(new int[]{0,1,2}))) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[FLP STATUS]Value set: 0,1,2, current value: " +
                               String.valueOf(para);
                }
                break;
            case FLP_DISTANCE_INTERVAL:
                if (para > 0) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[FLP_DISTANCE_INTERVAL]Range: >0, current value: " +
                               String.valueOf(para);
                }
                break;
            case FLP_POWER_MODE:
                if (dataSetValidate(para,getSetFromArray(new int[]{1,2,3,4,5}))) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[FLP_POWER_MODE]Value Set:1,2,3,4,5, current value: " +
                               String.valueOf(para);
                }
                break;
            case GEO_RESPONSIVENESS:
                if (para >= 1000 && para <= 65535000) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[GEO_RESPONSIVENESS]Range: [1000,65535000], current value: " +
                               String.valueOf(para);
                }
                break;
            case GEO_EVENT:
                if (dataSetValidate(para,getSetFromArray(new int[]{1,2,4,8,16}))) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[GEO_EVENT]Value set:1,2,4,8,16, current value: " +
                               String.valueOf(para);
                }
                break;
            case GEO_REQUEST_TYPE:
                if (dataSetValidate(para,getSetFromArray(new int[]{1,2,3,4,5}))) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[GEO_REQUEST_TYPE]Value set:1,2,3,4,5, current value: " +
                               String.valueOf(para);
                }
                break;
            case GEO_ERROR_CODE:
                if (dataSetValidate(para,getSetFromArray(new int[]{-100,-102,-103,-149}))) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[GEO_ERROR_CODE]Value set:-100,-102,-103,-149," +
                               "current value: " + String.valueOf(para);
                }
                break;
            case GEO_ENGINE_STATUS:
                if (dataSetValidate(para,getSetFromArray(new int[]{1,2,3,4}))) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[GEO_ENGINE_STATUS]Value set:1,2,3,4, current value: " +
                               String.valueOf(para);
                }
                break;
            case GEO_DWELL_TIME:
                if (para >= 0 && para < 65535) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[GEO_DWELL_TIME]Range: [0,65535), current value: " +
                               String.valueOf(para);
                }
                break;
            case WIFI_EXPIRE_DAYS:
                if (para >= 0) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[WIFI_EXPIRE_DAYS]Range: >=0, current value: " +
                               String.valueOf(para);
                }
                break;
            case WIFI_DAYS_VALID:
                if (para >= 0) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[WIFI_DAYS_VALID]Range: >=0, current value: " +
                               String.valueOf(para);
                }
                break;
            case WIFI_DELTA_TIME:
                if (para > 0) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[WIFI_DELTA_TIME]Range: >0, current value: " +
                               String.valueOf(para);
                }
                break;
            case WIFI_CHANNEL_NUM://need to further check
                if (para >= 0) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[WIFI_CHANNEL_NUM]Range: >=0, current value: " +
                               String.valueOf(para);
                }
                break;
            case WWAN_CELL_REGIONID1:
                if (para >= 0 && para <= 999) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[WWAN_CELL_REGIONID1]Range: [0,999], current value: " +
                               String.valueOf(para);
                }
                break;
            case WWAN_CELL_REGIONID2_CDMA:
                if (para >= 0 && para <= 32767) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[WWAN_CELL_REGIONID2_CDMA]Range: [0,32767], current value: " +
                               String.valueOf(para);
                }
                break;
            case WWAN_CELL_REGIONID2_OTHERS:
                if (para >= 0 && para <= 999) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[WWAN_CELL_REGIONID2_OTHERS]Range: [0,999], current value: " +
                               String.valueOf(para);
                }
                break;
            case WWAN_CELL_REGIONID3_CDMA_GSM:
                if (para >= 0 && para <= 65535) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[WWAN_CELL_REGIONID3_CDMA_GSM]Range: [0,65535], current value: " +
                               String.valueOf(para);
                    failInfo = "Cell ID3 out of range";
                }
                break;
            case WWAN_CELL_REGIONID3_WCDMA_LTE:
                if (para >= 1 && para <= 65535) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[WWAN_CELL_REGIONID3_WCDMA_LTE]Range: [1,65535], current value: " +
                               String.valueOf(para);
                }
            case WWAN_CELL_REGIONID4_CDMA_GSM:
                if (para >= 0 && para <= 65535) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[WWAN_CELL_REGIONID4_CDMA_GSM]Range: [0,65535], current value: " +
                               String.valueOf(para);
                }
                break;
            case WWAN_CELL_REGIONID4_WCDMA_LTE:
                if (para >= 0 && para <= 268435455) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[WWAN_CELL_REGIONID4_WCDMA_LTE]Range: [1,268435455]," +
                               " current value: " + String.valueOf(para);
                }
                break;
            default:
                failInfo = "Unknown type to be validate";
                break;

        }

        IZatValidationResults validateResult = new IZatValidationResults(false, failInfo);
        validateResult.processResult();
        return validateResult;
    }


    public static IZatValidationResults dataValidate(long para, IZatDataTypes types) {
        String failInfo = null;
        switch (types) {
            case LOCATION_TIME:
                if (para > 0) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[LOCATION_TIME]Range: >0, current value: " +
                               String.valueOf(para);
                }
                break;
            case FLP_TIME_INTERVAL:
                if (para > 0) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[FLP_TIME_INTERVAL]Range: >0, current value: " +
                               String.valueOf(para);
                }
                break;
            case FLP_TRIP_DISTANCE:
                if (para > 0) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[FLP_TRIP_DISTANCE]Range: >0, current value: " +
                               String.valueOf(para);
                }
                break;
            case FLP_TBM_MILLIS:
                if (para > 0) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[FLP_TBM_MILLIS]Range: >0, current value: " +
                               String.valueOf(para);
                }
                break;
            default:
                failInfo = "Unknown type to be validate";
                break;

        }
        IZatValidationResults validateResult = new IZatValidationResults(false, failInfo);
        validateResult.processResult();
        return validateResult;
    }
}
