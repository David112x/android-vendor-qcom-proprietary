/* ======================================================================
*  Copyright (c) 2017 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*  ====================================================================*/

package com.qti.wifidbreceiver;

import java.com.qti.wifidbreceiver.IWiFiDBReceiverResponseListener;
import java.com.qti.wifidbreceiver.APLocationData;
import java.com.qti.wifidbreceiver.APSpecialInfo;
import java.util.List;
import android.app.PendingIntent;

interface IWiFiDBReceiver {

    boolean registerResponseListener(in IWiFiDBReceiverResponseListener callback);

    int requestAPList (in int expireInDays);

    int pushWiFiDB (in List<APLocationData> locationData,
                    in List<APSpecialInfo> specialData,
                    in int daysValid);

    void removeResponseListener(in IWiFiDBReceiverResponseListener callback);
    int requestScanList ();
    boolean registerResponseListenerExt(in IWiFiDBReceiverResponseListener callback,
            in PendingIntent notifyIntent);
    int pushLookupResult (in List<APLocationData> locationData,
                          in List<APSpecialInfo> specialData);
}
