/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */
#ifndef __CNSS_iot_ap_H_
#define __CNSS_iot_ap_H_

#include "qca-vendor_copy.h"
#include "wlan_msg.h"
#include "debug.h"

struct wlan_iot_ap_ps {
	char bssid[WLAN_MSG_MAX_BSSID_LEN];
};

#ifdef ENABLE_INTEROP_ISSUES_AP_DETECTION
int cnss_iot_ap_init(void);
int cnss_iot_ap_deinit(void);
int cnss_iot_ap_set_to_driver(void);
#else
static inline int cnss_iot_ap_init(void)
{
	wsvc_printf_err("interop issues ap is not enabled\n");
	return 0;
}

static inline int cnss_iot_ap_deinit(void) { return 0; }
static inline int cnss_iot_ap_set_to_driver(void) { return 0; }
#endif
#endif
