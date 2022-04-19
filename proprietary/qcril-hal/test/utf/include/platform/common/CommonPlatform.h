/*===========================================================================
 *
 *    Copyright (c) 2018 Qualcomm Technologies, Inc.
 *    All Rights Reserved.
 *    Confidential and Proprietary - Qualcomm Technologies, Inc.
 *
 *===========================================================================*/

#pragma once

#include "ril.h"

class CommonPlatform {
public:
    virtual void Register(RIL_RadioFunctions *callbacks) = 0;
    virtual int OnRequest(int request, void *data, size_t datalen,
        RIL_Token t) = 0;
};
void setPlatform(CommonPlatform* platform);
