/*  Copyright (c) 2018 Qualcomm Technologies, Inc.
 *  All Rights Reserved.
 *  Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
package com.qti.gnssconfig;

oneway interface IGnssConfigCallback
{
    void getGnssSvTypeConfigCb(in int[] disabledSvTypeList);
}
