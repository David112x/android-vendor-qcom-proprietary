/* ======================================================================
*  Copyright (c) 2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*  ====================================================================*/

package com.qti.wifidbprovider;

import java.com.qti.wifidbprovider.APObsLocData;
import java.util.List;
import android.location.Location;

oneway interface IWiFiDBProviderResponseListener {

    void onApObsLocDataAvailable(in List<APObsLocData> apObsList, in int apStatus);

    void onServiceRequest();
}
