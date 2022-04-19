/* ======================================================================
*  Copyright (c) 2015, 2017 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*  ====================================================================*/

package com.qti.izat;

import android.os.IBinder;
import java.com.qti.flp.IFlpService;
import java.com.qti.flp.ITestService;
import java.com.qti.geofence.IGeofenceService;
import java.com.qti.debugreport.IDebugReportService;
import java.com.qti.wifidbreceiver.IWiFiDBReceiver;
import java.com.qti.wwandbreceiver.IWWANDBReceiver;
import java.com.qti.gnssconfig.IGnssConfigService;
import java.com.qti.wifidbprovider.IWiFiDBProvider;
import java.com.qti.wwandbprovider.IWWANDBProvider;

interface IIzatService {
    IFlpService getFlpService();
    ITestService getTestService();
    IGeofenceService getGeofenceService();
    String getVersion();
    IDebugReportService getDebugReportService();
    IWiFiDBReceiver getWiFiDBReceiver();
    IWWANDBReceiver getWWANDBReceiver();
    IGnssConfigService getGnssConfigService();
    IWiFiDBProvider getWiFiDBProvider();
    IWWANDBProvider getWWANDBProvider();
}