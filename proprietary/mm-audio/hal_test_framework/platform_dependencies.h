/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#ifndef PLATFORM_DEPENDENCIES_H_
#define PLATFORM_DEPENDENCIES_H_


#ifndef DISABLE_QAHW_API
#include <hardware/audio.h>
#include <hardware/hardware.h>
#else
#include "hardware/audio.h"
#include "hardware/hardware.h"
#endif

#include "qahw_api.h"
#include "qahw_defs.h"

#include <iostream>

#endif // PLATFORM_DEPENDENCIES_H_