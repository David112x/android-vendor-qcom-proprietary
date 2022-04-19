/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#define LOG_TAG "LocSvc_HIDL_GnssNi"
#define LOG_NDEBUG 0

#include "LocHidlGnssNi.h"

#define LOC_HIDL_VERSION V2_0
#include <all-versions/default/location_api/LocHidlNiClient.impl.h>
#include <all-versions/default/LocHidlGnssNi.impl.h>
#undef LOC_HIDL_VERSION
