/*===========================================================================
 *
 *    Copyright (c) 2018 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *===========================================================================*/

#ifndef _PLATFORM_H
#define _PLATFORM_H

#include "ril.h"

void platform_register(const RIL_RadioFunctions *callbacks);
int platform_onRequest(int request, void *data, size_t datalen, RIL_Token t);

#endif
