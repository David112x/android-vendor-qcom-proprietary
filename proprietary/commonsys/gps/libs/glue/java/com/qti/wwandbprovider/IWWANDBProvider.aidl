/* ======================================================================
*  Copyright (c) 2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*  ====================================================================*/

package com.qti.wwandbprovider;

import java.com.qti.wwandbprovider.IWWANDBProviderResponseListener;
import android.app.PendingIntent;

interface IWWANDBProvider {

    boolean registerResponseListener(in IWWANDBProviderResponseListener callback,
            in PendingIntent notifyIntent);

    int requestBSObsLocData();

    void removeResponseListener(in IWWANDBProviderResponseListener callback);
}
