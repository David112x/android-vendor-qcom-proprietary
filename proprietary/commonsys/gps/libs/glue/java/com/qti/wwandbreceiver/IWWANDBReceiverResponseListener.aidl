/* ======================================================================
*  Copyright (c) 2017 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*  ====================================================================*/

package com.qti.wwandbreceiver;

import java.com.qti.wwandbreceiver.BSInfo;
import java.util.List;
import android.location.Location;

oneway interface IWWANDBReceiverResponseListener {

    // Decprecated, for backwards compatibility with static linked SDK
    void onBSListAvailable(in List<BSInfo> bsInfoList);

    void onStatusUpdate(in boolean isSuccess, in String error);

    void onServiceRequest();

    void onBSListAvailableExt(in List<BSInfo> bsInfoList, in int apStatus, in Location location);
}
