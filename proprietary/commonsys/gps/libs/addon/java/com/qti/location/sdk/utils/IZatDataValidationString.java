/*====*====*====*====*====*====*====*====*====*====*====*====*====*====*====*
  Copyright (c) 2018 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.qti.location.sdk.utils;

import com.qti.location.sdk.utils.IZatValidationResults;
import com.qti.location.sdk.utils.IZatValidationResults.IZatDataTypes;

public class IZatDataValidationString {
    private static String TAG = "IZatDataValidation";

    public static boolean wifiIPV4MacValidate(String address) {
        String trueMacAddress = "[A-Fa-f0-9]{12}";

        return address.matches(trueMacAddress);
    }

    public static IZatValidationResults dataValidate(String str, IZatDataTypes types) {
        String failInfo = null;
        switch (types) {
            case WIFI_MAC_ADDRESS:
                if (wifiIPV4MacValidate(str)) {
                    return new IZatValidationResults(true);
                } else {
                    failInfo = "[WIFI_MAC_ADDRESS]Str with len 12, [0-F], current value: " + str;
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
