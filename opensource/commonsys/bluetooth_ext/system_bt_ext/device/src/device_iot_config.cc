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
#include "bt_target.h"
#if (BT_IOT_LOGGING_ENABLED == TRUE)

#define LOG_TAG "device_iot_config"
//#undef LOG_NDEBUG
//#define LOG_NDEBUG 0
#include "device_iot_config.h"

#include <base/logging.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <string>
#include <mutex>

#include "bt_types.h"
#include "btcore/include/module.h"
#include "btif/include/btif_api.h"
#include "btif/include/btif_util.h"
#include "osi/include/alarm.h"
#include "osi/include/allocator.h"
#include "osi/include/compat.h"
#include "osi/include/config.h"
#include "osi/include/log.h"
#include "osi/include/osi.h"

#include "osi/include/properties.h"

#define BT_IOT_CONFIG_SOURCE_TAG_NUM 1010003

#define INFO_SECTION "Info"
#define VERSION_KEY "Version"
#define FILE_CREATED_TIMESTAMP "TimeCreated"
#define FILE_MODIFIED_TIMESTAMP "TimeModified"
#define TIME_STRING_LENGTH sizeof("YYYY-MM-DD HH:MM:SS")
static const char* TIME_STRING_FORMAT = "%Y-%m-%d %H:%M:%S";

#ifndef DEVICES_MAX_NUM_IN_IOT_INFO_FILE
#define DEVICES_MAX_NUM_IN_IOT_INFO_FILE  40
#endif
#define DEVICES_NUM_MARGIN 5

#if (DEVICES_MAX_NUM_IN_IOT_INFO_FILE < DEVICES_NUM_MARGIN)
#undef DEVICES_MAX_NUM_IN_IOT_INFO_FILE
#define DEVICES_MAX_NUM_IN_IOT_INFO_FILE DEVICES_NUM_MARGIN
#endif

#define DEVICE_IOT_INFO_CURRENT_VERSION 1
#define DEVICE_IOT_INFO_FIRST_VERSION 1

#define IOT_CONFIG_FLUSH_EVT            0
#define IOT_CONFIG_SAVE_TIMER_FIRED_EVT 1

#if defined(OS_GENERIC)
static const char* IOT_CONFIG_FILE_PATH = "bt_remote_dev_info.conf";
static const char* IOT_CONFIG_BACKUP_PATH = "bt_remote_dev_info.bak";
#else  // !defined(OS_GENERIC)
static const char* IOT_CONFIG_FILE_PATH = "/data/misc/bluedroid/bt_remote_dev_info.conf";
static const char* IOT_CONFIG_BACKUP_PATH = "/data/misc/bluedroid/bt_remote_dev_info.bak";
#endif  // defined(OS_GENERIC)
static const period_ms_t CONFIG_SETTLE_PERIOD_MS = 12000;

static void timer_iot_config_save_cb(void* data);
static void device_iot_config_write(uint16_t event, char* p_param);
static config_t* device_iot_config_open(const char* filename);
static int device_iot_config_get_device_num(config_t* config);
static void device_iot_config_save(void);
static bool is_factory_reset(void);
static void delete_iot_config_files(void);

static enum ConfigSource {
  NOT_LOADED,
  ORIGINAL,
  BACKUP,
  NEW_FILE,
  RESET
} device_iot_config_source = NOT_LOADED;

static int device_iot_config_devices_loaded = -1;
static char device_iot_config_time_created[TIME_STRING_LENGTH];

static std::mutex config_lock;  // protects operations on |config|.
static config_t* config;
static alarm_t* config_timer;
static bool iot_logging_enabled = false;

#define CHECK_LOGGING_ENABLED(return_value) do { if (!iot_logging_enabled) return (return_value); } while(0)

// Module lifecycle functions
static future_t* init(void) {
  char enabled[PROPERTY_VALUE_MAX] = {0};

  std::unique_lock<std::mutex> lock(config_lock);
  osi_property_get("persist.vendor.service.bt.iot.enablelogging", enabled, "false");
  iot_logging_enabled = strncmp(enabled, "true", 4) == 0;

  if (!iot_logging_enabled) {
    delete_iot_config_files();
    return future_new_immediate(FUTURE_SUCCESS);
  }

  LOG_INFO(LOG_TAG, "%s", __func__);

  if (is_factory_reset())
    delete_iot_config_files();

  config = device_iot_config_open(IOT_CONFIG_FILE_PATH);
  device_iot_config_source = ORIGINAL;
  if (!config) {
    LOG_WARN(LOG_TAG, "%s unable to load config file: %s; using backup.",
          __func__, IOT_CONFIG_FILE_PATH);
    config = device_iot_config_open(IOT_CONFIG_BACKUP_PATH);
    device_iot_config_source = BACKUP;
  }

  if (!config) {
    LOG_ERROR(LOG_TAG, "%s unable to load bak file; creating empty config.", __func__);
    config = config_new_empty();
    device_iot_config_source = NEW_FILE;
  }

  if (!config) {
    LOG_ERROR(LOG_TAG, "%s unable to allocate a config object.", __func__);
    goto error;
  }

  int version;
  if (device_iot_config_source == NEW_FILE) {
    version = DEVICE_IOT_INFO_CURRENT_VERSION;
    config_set_int(config, INFO_SECTION, VERSION_KEY, DEVICE_IOT_INFO_CURRENT_VERSION);
  } else {
    version = config_get_int(config, INFO_SECTION, VERSION_KEY, -1);
    if (version == -1) {
      version = DEVICE_IOT_INFO_FIRST_VERSION;
      config_set_int(config, INFO_SECTION, VERSION_KEY, DEVICE_IOT_INFO_FIRST_VERSION);
    }
  }

  if (version != DEVICE_IOT_INFO_CURRENT_VERSION) {
    LOG_INFO(LOG_TAG, "%s: version in file is %d, CURRENT_VERSION is %d ", __func__,
        version, DEVICE_IOT_INFO_CURRENT_VERSION);
    remove(IOT_CONFIG_FILE_PATH);
    remove(IOT_CONFIG_BACKUP_PATH);
    config_free(config);
    config = config_new_empty();
    if (!config) {
      LOG_ERROR(LOG_TAG, "%s unable to allocate a config object.", __func__);
      goto error;
    }
    config_set_int(config, INFO_SECTION, VERSION_KEY, DEVICE_IOT_INFO_CURRENT_VERSION);
    device_iot_config_source = NEW_FILE;
  }

  device_iot_config_devices_loaded = device_iot_config_get_device_num(config);

  // Read or set config file creation timestamp
  const char* time_str;
  time_str = config_get_string(config, INFO_SECTION, FILE_CREATED_TIMESTAMP, NULL);
  if (time_str != NULL) {
    strlcpy(device_iot_config_time_created, time_str, TIME_STRING_LENGTH);
  } else {
    time_t current_time = time(NULL);
    struct tm* time_created = localtime(&current_time);
    if (time_created) {
      strftime(device_iot_config_time_created, TIME_STRING_LENGTH,
              TIME_STRING_FORMAT, time_created);
      config_set_string(config, INFO_SECTION, FILE_CREATED_TIMESTAMP,
              device_iot_config_time_created);
    }
  }

  // TODO(sharvil): use a non-wake alarm for this once we have
  // API support for it. There's no need to wake the system to
  // write back to disk.
  config_timer = alarm_new("btif.iot.config");
  if (!config_timer) {
    LOG_ERROR(LOG_TAG, "%s unable to create alarm.", __func__);
    goto error;
  }

  LOG_EVENT_INT(BT_IOT_CONFIG_SOURCE_TAG_NUM, device_iot_config_source);

  return future_new_immediate(FUTURE_SUCCESS);

error:
  alarm_free(config_timer);
  config_free(config);
  config_timer = NULL;
  config = NULL;
  device_iot_config_source = NOT_LOADED;
  return future_new_immediate(FUTURE_FAIL);
}

static config_t* device_iot_config_open(const char* filename) {
  config_t* config = config_new(filename);
  if (!config)
    return NULL;

  return config;
}

static future_t* start_up(void) {
  CHECK_LOGGING_ENABLED(future_new_immediate(FUTURE_SUCCESS));

  LOG_INFO(LOG_TAG, "%s", __func__);
  device_iot_config_int_add_one(IOT_CONF_KEY_SECTION_ADAPTER, IOT_CONF_KEY_BT_ONOFF_COUNT);
  return future_new_immediate(FUTURE_SUCCESS);
}

static future_t* shut_down(void) {
  CHECK_LOGGING_ENABLED(future_new_immediate(FUTURE_SUCCESS));

  LOG_INFO(LOG_TAG, "%s", __func__);
  device_iot_config_flush();
  return future_new_immediate(FUTURE_SUCCESS);
}

static future_t* clean_up(void) {
  CHECK_LOGGING_ENABLED(future_new_immediate(FUTURE_SUCCESS));

  LOG_INFO(LOG_TAG, "%s", __func__);
  if (config_timer != NULL && alarm_is_scheduled(config_timer))
    device_iot_config_flush();

  alarm_free(config_timer);
  config_timer = NULL;

  std::unique_lock<std::mutex> lock(config_lock);
  config_free(config);
  config = NULL;
  return future_new_immediate(FUTURE_SUCCESS);
}

EXPORT_SYMBOL module_t device_iot_config_module = {
  .name = DEVICE_IOT_CONFIG_MODULE,
  .init = init,
  .start_up = start_up,
  .shut_down = shut_down,
  .clean_up = clean_up
};

bool device_iot_config_has_section(const char* section) {
  CHECK_LOGGING_ENABLED(false);

  CHECK(config != NULL);
  CHECK(section != NULL);

  std::unique_lock<std::mutex> lock(config_lock);
  return config_has_section(config, section);
}

bool device_iot_config_exist(const char* section, const char* key) {
  CHECK_LOGGING_ENABLED(false);

  CHECK(config != NULL);
  CHECK(section != NULL);
  CHECK(key != NULL);

  std::unique_lock<std::mutex> lock(config_lock);
  return config_has_key(config, section, key);
}

static bool device_iot_config_has_key_value(const char* section, const char* key, const char* value_str) {
  CHECK(config != NULL);
  CHECK(section != NULL);
  CHECK(key != NULL);
  CHECK(value_str != NULL);

  const char* stored_value = config_get_string(config, section, key, NULL);

  if (!stored_value || strcmp(value_str, stored_value) != 0)
    return false;

  return true;
}

bool device_iot_config_get_int(const char* section, const char* key, int* value) {
  CHECK_LOGGING_ENABLED(false);

  CHECK(config != NULL);
  CHECK(section != NULL);
  CHECK(key != NULL);
  CHECK(value != NULL);

  std::unique_lock<std::mutex> lock(config_lock);
  bool ret = config_has_key(config, section, key);
  if (ret)
    *value = config_get_int(config, section, key, *value);

  return ret;
}

bool device_iot_config_addr_get_int(const RawAddress& peer_addr, const char* key, int* value) {
  CHECK_LOGGING_ENABLED(false);

  std::string addrstr = peer_addr.ToString();
  const char* bdstr = addrstr.c_str();
  return device_iot_config_get_int(bdstr, key, value);
}

bool device_iot_config_set_int(const char* section, const char* key, int value) {
  CHECK_LOGGING_ENABLED(false);

  CHECK(config != NULL);
  CHECK(section != NULL);
  CHECK(key != NULL);

  LOG_VERBOSE(LOG_TAG, "%s: sec=%s, key=%s, val=%d", __func__, section, key, value);
  std::unique_lock<std::mutex> lock(config_lock);
  char value_str[32] = {0};
  snprintf(value_str, sizeof(value_str), "%d", value);
  if (device_iot_config_has_key_value(section, key, value_str))
    return true;

  config_set_string(config, section, key, value_str);
  device_iot_config_save();

  return true;
}

bool device_iot_config_addr_set_int(const RawAddress& peer_addr, const char* key, int value) {
  CHECK_LOGGING_ENABLED(false);

  std::string addrstr = peer_addr.ToString();
  const char* bdstr = addrstr.c_str();
  return device_iot_config_set_int(bdstr, key, value);
}

bool device_iot_config_int_add_one(const char* section, const char* key) {
  CHECK_LOGGING_ENABLED(false);

  CHECK(config != NULL);
  CHECK(section != NULL);
  CHECK(key != NULL);

  LOG_VERBOSE(LOG_TAG, "%s: sec=%s, key=%s", __func__, section, key);
  int result = 0;
  std::unique_lock<std::mutex> lock(config_lock);
  result = config_get_int(config, section, key, result);
  if (result >= 0) {
    result += 1;
  } else {
    result = 0;
  }
  config_set_int(config, section, key, result);
  device_iot_config_save();

  return true;
}

bool device_iot_config_addr_int_add_one(const RawAddress& peer_addr, const char* key) {
  CHECK_LOGGING_ENABLED(false);

  std::string addrstr = peer_addr.ToString();
  const char* bdstr = addrstr.c_str();
  return device_iot_config_int_add_one(bdstr, key);
}

bool device_iot_config_get_hex(const char* section, const char* key, int* value) {
  CHECK_LOGGING_ENABLED(false);

  CHECK(config != NULL);
  CHECK(section != NULL);
  CHECK(key != NULL);
  CHECK(value != NULL);

  int sscanf_ret, result = 0;
  std::unique_lock<std::mutex> lock(config_lock);
  const char* stored_value = config_get_string(config, section, key, NULL);
  if (!stored_value)
    return false;

  sscanf_ret = sscanf(stored_value, "%x", &result);
  if (sscanf_ret != 1)
    return false;

  *value = result;
  return true;
}

bool device_iot_config_addr_get_hex(const RawAddress& peer_addr, const char* key, int* value) {
  CHECK_LOGGING_ENABLED(false);

  std::string addrstr = peer_addr.ToString();
  const char* bdstr = addrstr.c_str();
  return device_iot_config_get_hex(bdstr, key, value);
}

bool device_iot_config_set_hex(const char* section, const char* key, int value, int byte_num) {
  CHECK_LOGGING_ENABLED(false);

  CHECK(config != NULL);
  CHECK(section != NULL);
  CHECK(key != NULL);

  LOG_VERBOSE(LOG_TAG, "%s: sec=%s, key=%s, val=0x%x", __func__, section, key, value);
  char value_str[32] = { 0 };
  if (byte_num == 1)
    snprintf(value_str, sizeof(value_str), "%02x", value);
  else if (byte_num == 2)
    snprintf(value_str, sizeof(value_str), "%04x", value);
  else if (byte_num == 3)
    snprintf(value_str, sizeof(value_str), "%06x", value);
  else if (byte_num == 4)
    snprintf(value_str, sizeof(value_str), "%08x", value);

  std::unique_lock<std::mutex> lock(config_lock);
  if (device_iot_config_has_key_value(section, key, value_str))
    return true;

  config_set_string(config, section, key, value_str);
  device_iot_config_save();

  return true;
}

bool device_iot_config_addr_set_hex(const RawAddress& peer_addr,
          const char* key, int value, int byte_num) {
  CHECK_LOGGING_ENABLED(false);

  std::string addrstr = peer_addr.ToString();
  const char* bdstr = addrstr.c_str();
  return device_iot_config_set_hex(bdstr, key, value, byte_num);
}

bool device_iot_config_addr_set_hex_if_greater(const RawAddress& peer_addr,
    const char* key, int value, int byte_num) {
  CHECK_LOGGING_ENABLED(false);

  int stored_value = 0;
  bool ret = device_iot_config_addr_get_hex(peer_addr, key, &stored_value);
  if (ret && stored_value >= value)
    return true;

  return device_iot_config_addr_set_hex(peer_addr, key, value, byte_num);
}

bool device_iot_config_get_str(const char* section, const char* key, char* value, int* size_bytes) {
  CHECK_LOGGING_ENABLED(false);

  CHECK(config != NULL);
  CHECK(section != NULL);
  CHECK(key != NULL);
  CHECK(value != NULL);
  CHECK(size_bytes != NULL);

  std::unique_lock<std::mutex> lock(config_lock);
  const char* stored_value = config_get_string(config, section, key, NULL);

  if (!stored_value)
    return false;

  strlcpy(value, stored_value, *size_bytes);
  *size_bytes = strlen(value) + 1;

  return true;
}

bool device_iot_config_set_str(const char* section, const char* key, const char* value) {
  CHECK_LOGGING_ENABLED(false);

  CHECK(config != NULL);
  CHECK(section != NULL);
  CHECK(key != NULL);
  CHECK(value != NULL);

  LOG_VERBOSE(LOG_TAG, "%s: sec=%s, key=%s, val=%s", __func__, section, key, value);
  std::unique_lock<std::mutex> lock(config_lock);
  if (device_iot_config_has_key_value(section, key, value))
    return true;

  config_set_string(config, section, key, value);
  device_iot_config_save();

  return true;
}

bool device_iot_config_addr_set_str(const RawAddress& peer_addr,
          const char* key, const char* value) {
  CHECK_LOGGING_ENABLED(false);

  std::string addrstr = peer_addr.ToString();
  const char* bdstr = addrstr.c_str();
  return device_iot_config_set_str(bdstr, key, value);
}

bool device_iot_config_get_bin(const char* section, const char* key,
          uint8_t* value, size_t* length) {
  CHECK_LOGGING_ENABLED(false);

  CHECK(config != NULL);
  CHECK(section != NULL);
  CHECK(key != NULL);
  CHECK(value != NULL);
  CHECK(length != NULL);

  std::unique_lock<std::mutex> lock(config_lock);
  const char* value_str = config_get_string(config, section, key, NULL);

  if (!value_str)
    return false;

  size_t value_len = strlen(value_str);
  if ((value_len % 2) != 0 || *length < (value_len / 2))
    return false;

  for (size_t i = 0; i < value_len; ++i)
    if (!isxdigit(value_str[i]))
      return false;

  for (*length = 0; *value_str; value_str += 2, *length += 1)
    sscanf(value_str, "%02hhx", &value[*length]);

  return true;
}

size_t device_iot_config_get_bin_length(const char* section, const char* key) {
  CHECK_LOGGING_ENABLED(0);

  CHECK(config != NULL);
  CHECK(section != NULL);
  CHECK(key != NULL);

  std::unique_lock<std::mutex> lock(config_lock);
  const char* value_str = config_get_string(config, section, key, NULL);

  if (!value_str)
    return 0;

  size_t value_len = strlen(value_str);
  return ((value_len % 2) != 0) ? 0 : (value_len / 2);
}

bool device_iot_config_set_bin(const char* section, const char* key,
          const uint8_t* value, size_t length) {
  CHECK_LOGGING_ENABLED(false);

  const char* lookup = "0123456789abcdef";

  CHECK(config != NULL);
  CHECK(section != NULL);
  CHECK(key != NULL);

  LOG_VERBOSE(LOG_TAG, "%s: key = %s", __func__, key);
  if (length > 0)
    CHECK(value != NULL);

  char* str = (char* )osi_calloc(length * 2 + 1);
  if (str == NULL) {
    LOG_ERROR(LOG_TAG, "%s unable to allocate a str.", __func__);
    return false;
  }

  for (size_t i = 0; i < length; ++i) {
    str[(i * 2) + 0] = lookup[(value[i] >> 4) & 0x0F];
    str[(i * 2) + 1] = lookup[value[i] & 0x0F];
  }

  std::unique_lock<std::mutex> lock(config_lock);
  if (device_iot_config_has_key_value(section, key, str))
    return true;

  config_set_string(config, section, key, str);
  device_iot_config_save();

  osi_free(str);
  return true;
}

bool device_iot_config_addr_set_bin(const RawAddress& peer_addr,
          const char* key, const uint8_t* value, size_t length) {
  CHECK_LOGGING_ENABLED(false);

  std::string addrstr = peer_addr.ToString();
  const char* bdstr = addrstr.c_str();
  return device_iot_config_set_bin(bdstr, key, value, length);
}

const device_iot_config_section_iter_t* device_iot_config_section_begin(void) {
  CHECK_LOGGING_ENABLED(NULL);

  CHECK(config != NULL);
  return (const device_iot_config_section_iter_t* )config_section_begin(config);
}

const device_iot_config_section_iter_t* device_iot_config_section_end(void) {
  CHECK_LOGGING_ENABLED(NULL);

  CHECK(config != NULL);
  return (const device_iot_config_section_iter_t* )config_section_end(config);
}

const device_iot_config_section_iter_t* device_iot_config_section_next(const
          device_iot_config_section_iter_t* section) {
  CHECK_LOGGING_ENABLED(NULL);

  CHECK(config != NULL);
  CHECK(section != NULL);
  return (const device_iot_config_section_iter_t* )config_section_next((const
          config_section_node_t* )section);
}

const char* device_iot_config_section_name(const device_iot_config_section_iter_t* section) {
  CHECK_LOGGING_ENABLED(NULL);

  CHECK(config != NULL);
  CHECK(section != NULL);
  return config_section_name((const config_section_node_t* )section);
}

bool device_iot_config_remove(const char* section, const char* key) {
  CHECK_LOGGING_ENABLED(false);

  CHECK(config != NULL);
  CHECK(section != NULL);
  CHECK(key != NULL);

  std::unique_lock<std::mutex> lock(config_lock);
  return config_remove_key(config, section, key);
}

static void device_iot_config_save(void) {
  CHECK_LOGGING_ENABLED((void)0);

  CHECK(config != NULL);
  CHECK(config_timer != NULL);

  LOG_VERBOSE(LOG_TAG, "%s", __func__);
  alarm_set(config_timer, CONFIG_SETTLE_PERIOD_MS, timer_iot_config_save_cb, NULL);
}

void device_iot_config_flush(void) {
  CHECK_LOGGING_ENABLED((void)0);

  CHECK(config != NULL);
  CHECK(config_timer != NULL);

  int event = alarm_is_scheduled(config_timer) ? IOT_CONFIG_SAVE_TIMER_FIRED_EVT :
          IOT_CONFIG_FLUSH_EVT;
  LOG_VERBOSE(LOG_TAG, "%s: evt=%d", __func__, event);
  alarm_cancel(config_timer);
  device_iot_config_write(event, NULL);
}

bool device_iot_config_clear(void) {
  CHECK_LOGGING_ENABLED(false);

  CHECK(config != NULL);
  CHECK(config_timer != NULL);

  LOG_INFO(LOG_TAG, "%s", __func__);
  alarm_cancel(config_timer);

  std::unique_lock<std::mutex> lock(config_lock);
  config_free(config);

  config = config_new_empty();
  if (config == NULL) {
    return false;
  }

  bool ret = config_save(config, IOT_CONFIG_FILE_PATH);
  device_iot_config_source = RESET;
  return ret;
}

static void timer_iot_config_save_cb(UNUSED_ATTR void* data) {
  // Moving file I/O to btif context instead of timer callback because
  // it usually takes a lot of time to be completed, introducing
  // delays during A2DP playback causing blips or choppiness.
  LOG_VERBOSE(LOG_TAG, "%s", __func__);
  btif_transfer_context(device_iot_config_write, IOT_CONFIG_SAVE_TIMER_FIRED_EVT, NULL, 0, NULL);
}

static void set_modified_time() {
  time_t current_time = time(NULL);
  struct tm* time_modified = localtime(&current_time);
  char device_iot_config_time_modified[TIME_STRING_LENGTH];
  if (time_modified) {
    strftime(device_iot_config_time_modified, TIME_STRING_LENGTH,
            TIME_STRING_FORMAT, time_modified);
    config_set_string(config, INFO_SECTION, FILE_MODIFIED_TIMESTAMP,
            device_iot_config_time_modified);
  }
}

static void restrict_device_num() {
  CHECK(config != NULL);

  int curr_num = device_iot_config_get_device_num(config);
  int removed_devices = 0;
  int need_remove_devices_num;

  if (curr_num <= DEVICES_MAX_NUM_IN_IOT_INFO_FILE) {
    return;
  }

  need_remove_devices_num = curr_num - DEVICES_MAX_NUM_IN_IOT_INFO_FILE + DEVICES_NUM_MARGIN;
  LOG_INFO(LOG_TAG, "%s: curr_num=%d, need_remove_num=%d", __func__,
     curr_num, need_remove_devices_num);

  const config_section_node_t* snode = config_section_begin(config);
  while (snode != config_section_end(config)) {
    const char* section = config_section_name(snode);
    if (RawAddress::IsValidAddress(section)) {
      snode = config_section_next(snode);
      config_remove_section(config, section);
      if (++removed_devices >= need_remove_devices_num) {
        break;
      }
    } else {
      snode = config_section_next(snode);
    }
  }
}

static int compare_key(const char* first, const char* second) {
  bool first_is_profile_key = strncasecmp(first, "Profile", 7) == 0;
  bool second_is_profile_key = strncasecmp(second, "Profile", 7) == 0;
  if (!first_is_profile_key && !second_is_profile_key) {
    return 0;
  } else if (first_is_profile_key && second_is_profile_key) {
    return strcasecmp(first, second);
  } else {
    return first_is_profile_key ? 1 : -1;
  }
}

static void device_iot_config_write(uint16_t event, UNUSED_ATTR char* p_param) {
  CHECK_LOGGING_ENABLED((void)0);

  CHECK(config != NULL);
  CHECK(config_timer != NULL);

  LOG_INFO(LOG_TAG, "%s: evt=%d", __func__, event);
  std::unique_lock<std::mutex> lock(config_lock);
  if (event == IOT_CONFIG_SAVE_TIMER_FIRED_EVT)
    set_modified_time();

  rename(IOT_CONFIG_FILE_PATH, IOT_CONFIG_BACKUP_PATH);
  restrict_device_num();
  config_sections_sort_by_entry_key(config, compare_key);
  config_save(config, IOT_CONFIG_FILE_PATH);
}

static int device_iot_config_get_device_num(config_t* conf) {
  CHECK_LOGGING_ENABLED(0);

  CHECK(conf != NULL);
  int devices = 0;

  const config_section_node_t* snode = config_section_begin(conf);
  while (snode != config_section_end(conf)) {
    const char* section = config_section_name(snode);
    if (RawAddress::IsValidAddress(section)) {
      devices++;
    }
    snode = config_section_next(snode);
  }
  return devices;
}

void device_debug_iot_config_dump(int fd) {
  CHECK_LOGGING_ENABLED((void)0);

  dprintf(fd, "\nBluetooth Iot Config:\n");

  dprintf(fd, "  Config Source: ");
  switch(device_iot_config_source) {
    case NOT_LOADED:
      dprintf(fd, "Not loaded\n");
      break;
    case ORIGINAL:
      dprintf(fd, "Original file\n");
      break;
    case BACKUP:
      dprintf(fd, "Backup file\n");
      break;
    case NEW_FILE:
      dprintf(fd, "New file\n");
      break;
    case RESET:
      dprintf(fd, "Reset file\n");
      break;
  }

  dprintf(fd, "  Devices loaded: %d\n", device_iot_config_devices_loaded);
  dprintf(fd, "  File created/tagged: %s\n", device_iot_config_time_created);
}

static bool is_factory_reset(void) {
  char factory_reset[PROPERTY_VALUE_MAX] = {0};
  osi_property_get("persist.bluetooth.factoryreset", factory_reset, "false");
  return strncmp(factory_reset, "true", 4) == 0;
}

static void delete_iot_config_files(void) {
  remove(IOT_CONFIG_FILE_PATH);
  remove(IOT_CONFIG_BACKUP_PATH);
}

#endif
