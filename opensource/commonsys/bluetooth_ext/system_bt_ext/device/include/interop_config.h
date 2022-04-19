/******************************************************************************
 *
 *  Copyright (c) 2016, The Linux Foundation. All rights reserved.
 *  Not a Contribution.
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

#include <stdbool.h>
#include <errno.h>
#include "device/include/interop_database.h"
#include "device/include/interop.h"

typedef uint16_t UINT16;

// API's for adding entries to dynamic interop database
void interop_database_add_addr(const uint16_t feature, const RawAddress *addr, size_t length);
void interop_database_add_name(const uint16_t feature, const char *name);
void interop_database_add_manufacturer(const interop_feature_t feature, uint16_t manufacturer);
void interop_database_add_vndr_prdt(const interop_feature_t feature, uint16_t vendor_id, uint16_t product_id);
void interop_database_add_addr_max_lat(const interop_feature_t feature, const RawAddress *addr,
          size_t length, uint16_t max_lat);
void interop_database_add_version(const interop_feature_t feature, uint16_t version);

// API's for removing entries from dynamic interop database
bool interop_database_remove_addr(const interop_feature_t feature, const RawAddress *addr);
bool interop_database_remove_name( const interop_feature_t feature, const char *name);
bool interop_database_remove_manufacturer( const interop_feature_t feature, uint16_t manufacturer);
bool interop_database_remove_vndr_prdt(const interop_feature_t feature, uint16_t vendor_id, uint16_t product_id);
bool interop_database_remove_addr_max_lat(const interop_feature_t feature,
          const RawAddress *addr, size_t length, uint16_t max_lat);
bool interop_database_remove_version(const interop_feature_t feature, uint16_t version);

// API's to match entries with in dynamic interop database
bool interop_database_match_addr(const interop_feature_t feature, const RawAddress *addr);
bool interop_database_match_name( const interop_feature_t feature, const char *name);
bool interop_database_match_manufacturer(const interop_feature_t feature, uint16_t manufacturer);
bool interop_database_match_vndr_prdt(const interop_feature_t feature, uint16_t vendor_id, uint16_t product_id);
bool interop_database_match_addr_get_max_lat(const interop_feature_t feature,
          const RawAddress *addr, uint16_t *max_lat);
bool interop_database_match_version(const interop_feature_t feature, uint16_t version);

