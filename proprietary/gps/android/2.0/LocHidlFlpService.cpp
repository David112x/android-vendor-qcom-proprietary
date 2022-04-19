/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#define LOG_TAG "LocSvc_HIDL_FlpService"
#define LOG_NDEBUG 0

#include "LocHidlFlpService.h"

#define LOC_HIDL_VERSION V2_0
#include <all-versions/default/location_api/LocHidlFlpClient.impl.h>
#include <all-versions/default/LocHidlFlpService.impl.h>
#undef LOC_HIDL_VERSION
