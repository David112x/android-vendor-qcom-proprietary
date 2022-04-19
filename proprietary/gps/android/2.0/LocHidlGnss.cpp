/*
 * Copyright (c) 2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#define LOG_TAG "LocSvc_HIDL_Gnss"
#define LOG_NDEBUG 0

#include "LocHidlGnss.h"
#include "LocHidlAGnss.h"
#include "LocHidlDebugReportService.h"
#include "LocHidlFlpService.h"
#include "LocHidlGeofenceService.h"
#include "LocHidlIzatProvider.h"
#include "LocHidlGnssNi.h"
#include "LocHidlIzatOsNpGlue.h"
#include "LocHidlIzatSubscription.h"
#include "LocHidlRilInfoMonitor.h"
#include "LocHidlXT.h"
#include "LocHidlWiFiDBReceiver.h"
#include "LocHidlWWANDBReceiver.h"
#include "LocHidlUtils.h"

#define LOC_HIDL_VERSION V2_0
#include <all-versions/default/LocHidlGnss.impl.h>
#undef LOC_HIDL_VERSION
