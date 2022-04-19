/* ======================================================================
*  Copyright (c) 2017 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*  ====================================================================*/

package com.qti.wifidbreceiver;

import java.com.qti.wifidbreceiver.APInfo;
import java.com.qti.wifidbreceiver.APInfoExt;
import java.util.List;
import android.location.Location;

oneway interface IWiFiDBReceiverResponseListener {

    // Decprecated, for backwards compatibility with static linked SDK
    void onAPListAvailable(in List<APInfo> apInfoList);

    void onStatusUpdate(in boolean isSuccess, in String error);

    void onServiceRequest();

    void onAPListAvailableExt(in List<APInfoExt> apInfoList, in int apStatus, in Location location);
    void onLookupRequest(in List<APInfoExt> apInfoList, in Location location);
}
