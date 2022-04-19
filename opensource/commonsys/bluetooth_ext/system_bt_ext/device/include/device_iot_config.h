/******************************************************************************
 *  Copyright (C) 2018, The Linux Foundation. All rights reserved.
 *
 *  Not a Contribution
 *****************************************************************************/
/******************************************************************************
 *
 *  Copyright (C) 2014 Google, Inc.
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

#pragma once

#include "bt_target.h"
#if (BT_IOT_LOGGING_ENABLED == TRUE)

#include <stdbool.h>
#include <stddef.h>

#include "bt_types.h"
#include "device_iot_conf_defs.h"
#include "raw_address.h"

static const char DEVICE_IOT_CONFIG_MODULE[] = "device_iot_config_module";

typedef struct device_iot_config_section_iter_t device_iot_config_section_iter_t;

bool device_iot_config_has_section(const char* section);
bool device_iot_config_exist(const char* section, const char* key);
bool device_iot_config_get_int(const char* section, const char* key, int* value);
bool device_iot_config_addr_get_int(const RawAddress& peer_addr, const char* key, int* value);
bool device_iot_config_set_int(const char* section, const char* key, int value);
bool device_iot_config_addr_set_int(const RawAddress& peer_addr, const char* key, int value);
bool device_iot_config_int_add_one(const char* section, const char* key);
bool device_iot_config_addr_int_add_one(const RawAddress& peer_addr, const char* key);

bool device_iot_config_get_hex(const char* section, const char* key, int* value);
bool device_iot_config_addr_get_hex(const RawAddress& peer_addr, const char* key, int* value);
bool device_iot_config_set_hex(const char* section, const char* key, int value, int byte_num);
bool device_iot_config_addr_set_hex(const RawAddress& peer_addr, const char* key, int value, int byte_num);
bool device_iot_config_addr_set_hex_if_greater(const RawAddress& peer_addr, const char* key, int value, int byte_num);
bool device_iot_config_get_str(const char* section, const char* key, char* value, int* size_bytes);
bool device_iot_config_set_str(const char* section, const char* key, const char* value);
bool device_iot_config_addr_set_str(const RawAddress& peer_addr, const char* key, const char* value);
bool device_iot_config_get_bin(const char* section, const char* key, uint8_t* value, size_t* length);
bool device_iot_config_set_bin(const char* section, const char* key, const uint8_t* value, size_t length);
bool device_iot_config_addr_set_bin(const RawAddress& peer_addr, const char* key, const uint8_t* value, size_t length);
bool device_iot_config_remove(const char* section, const char* key);

size_t device_iot_config_get_bin_length(const char* section, const char* key);

const device_iot_config_section_iter_t* device_iot_config_section_begin(void);
const device_iot_config_section_iter_t* device_iot_config_section_end(void);
const device_iot_config_section_iter_t* device_iot_config_section_next(const device_iot_config_section_iter_t* section);
const char* device_iot_config_section_name(const device_iot_config_section_iter_t* section);

void device_iot_config_flush(void);
bool device_iot_config_clear(void);

void device_debug_iot_config_dump(int fd);

#endif
