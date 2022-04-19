/*=============================================================================
  Copyright (c) 2017, 2019 Qualcomm Technologies, Inc.
  All Rights Reserved.
  Confidential and Proprietary - Qualcomm Technologies, Inc.
=============================================================================*/

package com.example.connmgr;

import android.content.Context;
import com.qualcomm.qti.imscmservice.V2_2.IImsCmService;
import com.qualcomm.qti.imscmservice.V2_0.IImsCMConnection;
import com.qualcomm.qti.imscmservice.V2_0.configData;
import com.qualcomm.qti.imscmservice.V2_0.userConfig;
import com.qualcomm.qti.imscmservice.V2_0.deviceConfig;
import com.qualcomm.qti.imscmservice.V2_0.autoConfig;
import com.qualcomm.qti.imscmservice.V2_0.ConfigType;
import com.qualcomm.qti.imscmservice.V2_1.autoconfigTriggerReason;
import com.qualcomm.qti.imscmservice.V2_1.StatusCode;
import com.qualcomm.qti.imscmservice.V2_2.AutoConfigResponse;
import java.util.HashMap;


public class ConnectionGlobalData  {
    final static String TAG = "UI_ConnMgr_ConnectionGlobalData";
    public static final String SM_FEATURE_TAG =
        "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.largemsg\"";

    public static final String IM_FEATURE_TAG =
        "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.session\"";

    public static final String FT_FEATURE_TAG =
        "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.filetransfer\"";

    public static final String PRESENCE_FEATURE_TAG =
        "+g.3gpp.iari-ref=\"urn%3Aurn-7%3A3gpp-application.ims.iari.rcse.dp\"";

    public static final String  INVALID_FEATURE_TAG3 =
        "+g.3gpp.icsi-ref=\"urn%3Aurn-7%3A3gpp-service.ims.icsi.oma.cpm.anyinvalidtag\"";

    public static IImsCmService connMgr = null;
    public static boolean IMSPDNConnected = false;
    public static int connMgrInitialised = 0 ;
    public static int connSmInitialisedArray[] = {0, 0};
    public static int connImInitialisedArray[] = {0, 0};
    public static int connInvalidInitialisedArray[] = {0, 0};
    public static Context mContext = null;
    public static int userDataArray[] = {6789, 9560}; //random value for sub0 and sub1
    public static long serviceHandleArray[] = {0, 0};
    public static int activeWorkingSub = 0;
    public static configData configdata = null;
    public static AutoConfigResponse acsResponse = null;
    public static HashMap<Integer, String> userConfigData = new HashMap<Integer, String>();
    public static HashMap<Integer, String> deviceConfigData = new HashMap<Integer, String>();
    public static autoConfig autoConfigData = null;
    public static int configType = ConfigType.USER_CONFIG;
    public static int autoConfigType = ConfigType.AUTO_CONFIG;
    public static short responseCode = 404;

    public static HashMap<Integer, ConnectionData> connSmMap =
        new HashMap<Integer, ConnectionData>();
    public static HashMap<Integer, ConnectionData> connImMap =
        new HashMap<Integer, ConnectionData>();
    public static HashMap<Integer, ConnectionData> connInvalidMap =
        new HashMap<Integer, ConnectionData>();

    public static HashMap<Integer, ConnectionManagerListenerImpl> connMgrListenerMap =
        new HashMap<Integer, ConnectionManagerListenerImpl>();
    public static HashMap<Integer, ConnectionManagerListenerImpl_2_1> connMgrListenerMap_2_1 =
        new HashMap<Integer, ConnectionManagerListenerImpl_2_1>();
    public static HashMap<Integer, ConnectionManagerListenerImpl_2_0> connMgrListenerMap_2_0 =
        new HashMap<Integer, ConnectionManagerListenerImpl_2_0>();
    public static HashMap<Integer, Object> connMgrMap = new HashMap<Integer,Object>();
    public static HashMap<Integer, ConnectionListenerImpl> connSmListenerMap =
        new HashMap<Integer, ConnectionListenerImpl>();
    public static HashMap<Integer, ConnectionListenerImpl> connImListenerMap =
        new HashMap<Integer, ConnectionListenerImpl>();
    public static HashMap<Integer, ConnectionListenerImpl> connInvalidListenerMap =
        new HashMap<Integer, ConnectionListenerImpl>();
    public static int acsTriggerReason = autoconfigTriggerReason.AUTOCONFIG_INVALID_TOKEN;
    public static int oldClientV2_0 = 0;
    public static int oldClientV2_1 = 1;
    public static int newClient = 2;
    public static int currVersion;
    public static com.qualcomm.qti.imscmservice.V2_1.IImsCmService oldConnMgrV2_1;
    public static com.qualcomm.qti.imscmservice.V2_0.IImsCmService oldConnMgrV2_0;
    public static int unsupportedStatusCode = StatusCode.UNSUPPORTED;
}
