/*===========================================================================
 *
 *    Copyright (c) 2018 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *===========================================================================*/

#pragma once

#include "android/hardware/radio/1.2/IRadio.h"
#include "ril.h"

using namespace ::android::hardware::radio;

int convertDialInfotoHidl(void *data, size_t datalen, V1_0::Dial &dialInfo);

int convertCallForwardInfotoHidl(void *data, size_t datalen, V1_0::CallForwardInfo &info);
int convertCallForwardInfotoUtf(RIL_CallForwardInfo &out, const V1_0::CallForwardInfo &in);
