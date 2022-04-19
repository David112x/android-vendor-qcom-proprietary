/*
 * Copyright (c) 2019 The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *        * Redistributions of source code must retain the above copyright
 *            notice, this list of conditions and the following disclaimer.
 *        * Redistributions in binary form must reproduce the above
 *            copyright notice, this list of conditions and the following
 *            disclaimer in the documentation and/or other materials provided
 *            with the distribution.
 *        * Neither the name of The Linux Foundation nor the names of its
 *            contributors may be used to endorse or promote products derived
 *            from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __BT_CONFIG_STORE__
#define __BT_CONFIG_STORE__

#include <btcommon_interface_defs.h>
#include <cutils/properties.h>
#include <vector>

#define BT_CONFIG_STORE_INTERFACE_STRING "btConfigStoreInterface"

typedef struct {
  uint32_t type;
  char value[PROPERTY_VALUE_MAX];
}vendor_property_t;

typedef struct {
  /** set to sizeof(bt_configstore_interface_t) */
  size_t size;

  /**
   *
   *To get vendor property
   */
  bool (*get_vendor_properties)(uint32_t propType,
        std::vector<vendor_property_t> &vPropList);

  /**
   *
   * To get BT add On features
   */
  bool (*get_add_on_features)(add_on_features_list_t *features_list);

  /**
   * To set ANT/BT/FM Vendor Properties
   */
  bool (*set_vendor_property)(uint32_t type, const char *value);

  /**
   * To convert soc name to soc type
   */
  bt_soc_type_t (*convert_bt_soc_name_to_soc_type)(const char *soc_name);

  /**
   * To convert prop type  to string format
   */
  const char * (*convert_prop_type_to_string_format)(uint32_t propType);

} bt_configstore_interface_t;

#endif
