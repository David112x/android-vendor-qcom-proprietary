/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qti.location.sdk.utils;

import com.qti.location.sdk.utils.IZatValidationResults;
import com.qti.location.sdk.utils.IZatValidationResults.IZatDataTypes;

public class IZatDataValidationFloat {
    private static String TAG = "IZatDataValidation";

    public static IZatValidationResults dataValidate(float para, IZatDataTypes types) {
        String failInfo;
        switch (types) {
            case LOCATION_LATITUDE:
                if (para >= -90 && para <= 90 ) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[LOCATION_LATITUDE]Range: [-90,90], current value: " +
                               String.valueOf(para);
                }
                break;
            case LOCATION_LONGITUDE:
                if (para >= -180 && para <= 180 ) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[LOCATION_LONGITUDE]Range: [-180,180], current value: " +
                               String.valueOf(para);
                }
                break;
            case LOCATION_SPEED:
                if (para >= 0) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[LOCATION_SPEED]Range: >=0, current value: " +
                               String.valueOf(para);
                }
                break;
            case LOCATION_BEARING:
                if (para > 0 && para <= 360 ) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[LOCATION_BEARING]Range: (0,360], current value: " +
                               String.valueOf(para);
                }
                break;
            case LOCATION_ACCURACY:
                if (para > 0) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[LOCATION_ACCURACY]Range: >0, current value: " +
                               String.valueOf(para);
                }
                break;
            case GEO_RADIUS:
                if (para > 0 ) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[GEO_RADIUS]Range: >0, current value: " +
                               String.valueOf(para);
                }
                break;
            case WIFI_MAX_ANTENARANGE:
                if (para > 0) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[WIFI_MAX_ANTENARANGE]Range: >0, current value: " +
                               String.valueOf(para);
                    failInfo = "Wrong wifi max antena range";
                }
                break;
            case WIFI_RSSI:
                if (para > -128 && para < 127 ) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[WIFI_RSSI]Range: (-128,127), current value: " +
                               String.valueOf(para);
                }
                break;
            case WWAN_HORIZONTAL_COV_RADIUS:
                if (para > 0) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[WWAN_HORIZONTAL_COV_RADIUS]Range: >0, current value: " +
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
