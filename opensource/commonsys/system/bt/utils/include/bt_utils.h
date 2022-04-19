/******************************************************************************
 *
 *  Copyright (C) 2009-2012 Broadcom Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/

#ifndef BT_UTILS_H
#define BT_UTILS_H

#include "bt_types.h"

static const char BT_UTILS_MODULE[] = "bt_utils_module";

/*******************************************************************************
 *  Type definitions
 ******************************************************************************/

#define COD_AUDIO_DEVICE (0x200400)
#define COD_HID_MASK 0x0700
#define COD_HID_MAJOR 0x0500

typedef enum {
  TASK_HIGH_MEDIA = 0,
  TASK_UIPC_READ,
  TASK_HIGH_MAX
} tHIGH_PRIORITY_TASK;

/* Run-time configuration file to store AVRCP version info*/
#ifndef AVRC_PEER_VERSION_CONF_FILE
#define AVRC_PEER_VERSION_CONF_FILE "/data/misc/bluedroid/avrc_peer_entries.conf"
#endif
/* Runtime configuration file to store PCE version file*/
#ifndef PCE_PEER_VERSION_CONF_FILE
#define PCE_PEER_VERSION_CONF_FILE "/data/misc/bluedroid/pce_peer_entries.conf"
#endif
/* Runtime configuration file to store MCE version file*/
#ifndef MCE_PEER_VERSION_CONF_FILE
#define MCE_PEER_VERSION_CONF_FILE "/data/misc/bluedroid/mce_peer_entries.conf"
#endif

#define LOG_ID_STATS_A2DP      (0x06 << 8) | 0x00

/*******************************************************************************
 *  Functions
 ******************************************************************************/

void raise_priority_a2dp(tHIGH_PRIORITY_TASK high_task);
bool is_device_present(char* header, unsigned char* device_details);
uint32_t bt_devclass_to_uint(DEV_CLASS dev_class);
bool is_iot_info_report_enabled();

#endif /* BT_UTILS_H */
