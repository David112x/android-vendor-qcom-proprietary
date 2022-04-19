/* ======================================================================
*  Copyright (c) 2018 Qualcomm Technologies, Inc.
*  All Rights Reserved.
*  Confidential and Proprietary - Qualcomm Technologies, Inc.
*  ====================================================================*/

package com.qti.gnssconfig;

import java.com.qti.gnssconfig.IGnssConfigCallback;

interface IGnssConfigService {

    void registerCallback(in IGnssConfigCallback callback);
    void getGnssSvTypeConfig();
    void setGnssSvTypeConfig(in int[] disabledSvTypeList);
    void resetGnssSvTypeConfig();
}
