/******************************************************************************
 *
 *  Copyright (c) 2016-2017, The Linux Foundation. All rights reserved.
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

#define LOG_TAG "bt_device_profile"

#include <assert.h>
#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <hardware/bluetooth.h>
#include <hardware/vendor.h>
#include <string.h>

#include "bt_types.h"
#include "osi/include/config.h"
#include "osi/include/log.h"
#include "osi/include/osi.h"
#include "osi/include/compat.h"
#include "osi/include/list.h"
#include "profile_config.h"
#include "osi/include/allocator.h"
#include "btcore/include/module.h"

#if defined(OS_GENERIC)
static const char *PROFILE_CONF_FILE_PATH = "bt_profile.conf";
#else  // !defined(OS_GENERIC)
static const char *PROFILE_CONF_BASE_FILE_PATH = "/etc/bluetooth/bt_profile.conf";
static const char *PROFILE_CONF_FILE_PATH = "/data/misc/bluedroid/bt_profile.conf";
#endif  // defined(OS_GENERIC)

list_t *profile_conf_list = NULL;
bool profile_db_is_initialized = false;
static config_t *config;

#define CASE_RETURN_STR(const) case const: return #const;

#define SECTION_MAX_LENGTH  (249)
#define KEY_MAX_LENGTH      (249)
#define VALUE_MAX_LENGTH    (6)

struct config_t {
  list_t *sections;
};

typedef struct {
  char *key;
  char *value;
} profile_entry_t;

typedef struct {
  char *name;
  list_t *entries;
} profile_section_t;

typedef struct {
  char avrcp_coverart_support[VALUE_MAX_LENGTH];
  char avrcp_0103_support[VALUE_MAX_LENGTH];
} avrcp_feature_t;

typedef struct {
  char use_sim_support[VALUE_MAX_LENGTH];
  char pbap_0102_support[VALUE_MAX_LENGTH];
} pbap_feature_t;

typedef struct {
   char map_email_support[VALUE_MAX_LENGTH];
   char map_0104_support[VALUE_MAX_LENGTH];
} map_feature_t;

typedef struct {
  profile_t profile_id;
  char *version;

  union {
    avrcp_feature_t avrcp_feature_entry;
    pbap_feature_t pbap_feature_entry;
    map_feature_t map_feature_entry;
    max_pow_feature_t max_pow_feature_entry;
  } profile_feature_type;

} profile_db_entry_t;

// Config realted functions
static void profile_config_cleanup(void);
static void profile_free_entry_(void *data);
static void profile_lazy_init_(void);
static bool load_to_database(int profile_id, char *key, char *value);

//This function is used to initialize the profile list and load the entries from file
static void load_config();

// Interface functions
bool profile_feature_fetch(const profile_t profile, profile_info_t feature_name);
max_pow_feature_t max_radiated_power_fetch(const profile_t profile);

static const char* profile_name_string_(const profile_t profile_name)
{
  switch (profile_name) {
    CASE_RETURN_STR(AVRCP_ID)
    CASE_RETURN_STR(PBAP_ID)
    CASE_RETURN_STR(MAP_ID)
    CASE_RETURN_STR(MAX_POW_ID)
    CASE_RETURN_STR(END_OF_PROFILE_LIST)
  }
  return "UNKNOWN";
}

static const char* profile_feature_string_(const profile_info_t feature)
{
  switch (feature) {
    CASE_RETURN_STR(VERSION)
    CASE_RETURN_STR(AVRCP_COVERART_SUPPORT)
    CASE_RETURN_STR(AVRCP_0103_SUPPORT)
    CASE_RETURN_STR(USE_SIM_SUPPORT)
    CASE_RETURN_STR(MAP_EMAIL_SUPPORT)
    CASE_RETURN_STR(MAP_0104_SUPPORT)
    CASE_RETURN_STR(PBAP_0102_SUPPORT)
    CASE_RETURN_STR(BR_MAX_POW_SUPPORT)
    CASE_RETURN_STR(EDR_MAX_POW_SUPPORT)
    CASE_RETURN_STR(BLE_MAX_POW_SUPPORT)
    CASE_RETURN_STR(END_OF_FEATURE_LIST)
  }
  return "UNKNOWN";
}

// Module life-cycle functions

static future_t *profile_conf_init(void)
{
  profile_lazy_init_();
  return future_new_immediate(FUTURE_SUCCESS);
}

static future_t *profile_conf_clean_up(void)
{
  list_free(profile_conf_list);
  profile_conf_list = NULL;
  profile_config_cleanup();

  return future_new_immediate(FUTURE_SUCCESS);
}

EXPORT_SYMBOL module_t profile_config_module = {
  .name = PROFILE_CONFIG_MODULE,
  .init = profile_conf_init,
  .start_up = NULL,
  .shut_down = NULL,
  .clean_up = profile_conf_clean_up,
  .dependencies = {NULL},
};

// Local functions

static void profile_free_entry_(void *data)
{
  profile_db_entry_t *entry = (profile_db_entry_t *)data;
  osi_free(entry);
}

static void profile_lazy_init_(void)
{
  if (profile_conf_list == NULL) {
    profile_conf_list = list_new(profile_free_entry_);
    load_config();
  }
}

// profile config related functions

static int profile_config_init(void)
{
  struct stat sts;

  if (!stat(PROFILE_CONF_FILE_PATH, &sts) && sts.st_size) {
    if(!(config = config_new(PROFILE_CONF_FILE_PATH))) {
      LOG_WARN(LOG_TAG, "%s unable to load config file for : %s",
         __func__, PROFILE_CONF_FILE_PATH);
    }
  } else if (!stat(PROFILE_CONF_BASE_FILE_PATH, &sts) && sts.st_size) {
    if(!(config = config_new(PROFILE_CONF_BASE_FILE_PATH))) {
      LOG_WARN(LOG_TAG, "%s unable to load config file for : %s",
         __func__, PROFILE_CONF_BASE_FILE_PATH);
    }else {
        config_save(config, PROFILE_CONF_FILE_PATH);
    }
  }
  if(!config  && !(config = config_new_empty())) {
    goto error;
  }

  LOG_WARN(LOG_TAG, "%s load config file succesful : %s",
         __func__, PROFILE_CONF_FILE_PATH);
  return 0;

error:
  config_free(config);
  config = NULL;
  return -1;
}

static int get_profile(char *section)
{
  int profile_name;

  for ( profile_name = AVRCP_ID;
        profile_name < END_OF_PROFILE_LIST; profile_name++ ) {
    if (!strncasecmp( section, profile_name_string_((profile_t)profile_name),
                      strlen(section)))
      return profile_name;
  }
  return -1;
}

static int get_feature(char *key)
{
  int feature_name;

  for ( feature_name = VERSION;
        feature_name < END_OF_FEATURE_LIST; feature_name++ ) {
    if (!strncasecmp( key, profile_feature_string_((profile_info_t)feature_name),
                      strlen(key)))
      return feature_name;
  }
  return -1;
}

static profile_db_entry_t* profile_entry_fetch(const profile_t profile)
{
  assert(profile);
  profile_db_entry_t *db_entry = NULL;
  if (profile_conf_list == NULL || list_length(profile_conf_list) == 0) {
    return db_entry;
  }

  const list_node_t *node = list_begin(profile_conf_list);

  while (node != list_end(profile_conf_list)) {
    db_entry = (profile_db_entry_t *)list_node(node);
    assert(db_entry);

    if (profile != db_entry->profile_id) {
      node = list_next(node);
      continue;
    } else if(profile == db_entry->profile_id) {
       return (profile_db_entry_t*)db_entry;
    }
  }
  return NULL;
}

static void profile_database_add_(profile_db_entry_t *db_entry)
{
  if (!profile_entry_fetch(db_entry->profile_id)) {
    if (profile_conf_list)
      list_append(profile_conf_list, db_entry);
  } else {
    //return as the entry is already present
    LOG_DEBUG(LOG_TAG, "Entry is already present in the list");
    return;
  }
}

max_pow_feature_t  max_radiated_power_fetch(const profile_t profile, profile_info_t feature_name)
{
  static max_pow_feature_t Tech_max_power = {0x80, 0x80, 0x80, false, false, false};
  assert(profile);
  LOG_WARN(LOG_TAG, "max_radiated_power_fetch:profile %d", profile);

  profile_db_entry_t *db_entry = profile_entry_fetch(profile);
  if (db_entry == NULL) {
    return Tech_max_power;
  }

  if (profile == MAX_POW_ID) {
    switch (feature_name) {
      case BR_MAX_POW_SUPPORT:
      {
        if (db_entry->profile_feature_type.max_pow_feature_entry.BR_max_pow_feature == true) {
          Tech_max_power.BR_max_pow_feature = true;
          Tech_max_power.BR_max_pow_support =
             db_entry->profile_feature_type.max_pow_feature_entry.BR_max_pow_support;
        }
      }
      break;
      case EDR_MAX_POW_SUPPORT:
      {
        if (db_entry->profile_feature_type.max_pow_feature_entry.EDR_max_pow_feature == true) {
          Tech_max_power.EDR_max_pow_feature = true;
          Tech_max_power.EDR_max_pow_support =
             db_entry->profile_feature_type.max_pow_feature_entry.EDR_max_pow_support;
        }
      }
      break;
      case BLE_MAX_POW_SUPPORT:
      {
        if (db_entry->profile_feature_type.max_pow_feature_entry.BLE_max_pow_feature == true) {
          Tech_max_power.BLE_max_pow_feature = true;
          Tech_max_power.BLE_max_pow_support =
             db_entry->profile_feature_type.max_pow_feature_entry.BLE_max_pow_support;
        }
      }
      break;
      default:
      {
        LOG_WARN(LOG_TAG, "profile_feature_fetch:profile = %d , feature %d not found" , profile, feature_name);
      }
      break;
    }
  } else
    LOG_WARN(LOG_TAG, "max_radiated_power_fetch:profile = %d" , profile);

  return Tech_max_power;
}

bool profile_feature_fetch(const profile_t profile, profile_info_t feature_name)
{

  assert(profile);
  LOG_WARN(LOG_TAG, "profile_feature_fetch:profile %d", profile);
  bool feature_set = false;

  profile_db_entry_t *db_entry = profile_entry_fetch(profile);
  if (db_entry == NULL)
    return feature_set;

  switch (profile) {
    case AVRCP_ID:
    {
      switch (feature_name) {
        case AVRCP_COVERART_SUPPORT:
        {
          if (strncasecmp("true", db_entry->profile_feature_type.avrcp_feature_entry.avrcp_coverart_support, strlen("true")) == 0)
            feature_set = true;
          LOG_WARN(LOG_TAG, "profile_feature_fetch:AVRCP_COVERART_SUPPORT found %d", feature_set);
        }
        break;
        case AVRCP_0103_SUPPORT:
        {
          if (strncasecmp("true", db_entry->profile_feature_type.avrcp_feature_entry.avrcp_0103_support, strlen("true")) == 0)
            feature_set = true;
          LOG_WARN(LOG_TAG, "profile_feature_fetch:AVRCP_0103_SUPPORT found %d" , feature_set);
        }
        break;
        default:
        {
          LOG_WARN(LOG_TAG, "profile_feature_fetch:profile = %d , feature %d not found" , profile, feature_name);

        }
        break;
      }
    }
    break;
    case PBAP_ID:
    {
      switch (feature_name) {
        case USE_SIM_SUPPORT:
        {
          if (strncasecmp("true", db_entry->profile_feature_type.pbap_feature_entry.use_sim_support, strlen("true")) == 0)
            feature_set = true;
          LOG_WARN(LOG_TAG, "profile_feature_fetch:USE_SIM_SUPPORT found %d" , feature_set);
        }
        break;
        case PBAP_0102_SUPPORT:
        {
          if (strncasecmp("true", db_entry->profile_feature_type.pbap_feature_entry.pbap_0102_support, strlen("true")) == 0)
            feature_set = true;
          LOG_WARN(LOG_TAG, "profile_feature_fetch:PBAP_0102_SUPPORT found %d" , feature_set);
        }
        break;
        default:
        {
          LOG_WARN(LOG_TAG, "profile_feature_fetch:profile = %d , feature %d not found" , profile, feature_name);
        }
        break;
      }
    }
    break;
    case MAP_ID:
    {
      switch (feature_name) {
        case  MAP_EMAIL_SUPPORT:
        {
          if (strncasecmp("true", db_entry->profile_feature_type.map_feature_entry.map_email_support, strlen("true")) == 0)
            feature_set = true;
          LOG_WARN(LOG_TAG, "profile_feature_fetch:MAP_EMAIL_SUPPORT found %d" , feature_set);
        }
        break;
        case MAP_0104_SUPPORT:
        {
          if (strncasecmp("true", db_entry->profile_feature_type.map_feature_entry.map_0104_support,
              strlen("true")) == 0)
            feature_set = true;
          LOG_WARN(LOG_TAG, "profile_feature_fetch:MAP_0104_SUPPORT found %d" , feature_set);
        }
        break;
        default:
        {
          LOG_WARN(LOG_TAG, "profile_feature_fetch:profile = %d , feature %d not found" , profile, feature_name);
        }
        break;
      }
    }
    break;
    default:
    {
      LOG_WARN(LOG_TAG,"%s() profile %d not found",__func__, profile);
    }
    break;
  }
  return feature_set;
}

static bool load_to_database(int profile_id, char *key, char *value)
{
  profile_db_entry_t *entry;

  switch (profile_id) {
    case AVRCP_ID:
    {
      LOG_WARN(LOG_TAG, " AVRCP_ID: key :: %s, value :: %s",
                      key, value);
      entry = profile_entry_fetch(AVRCP_ID);
      if (entry == NULL) {
        entry = (profile_db_entry_t *)osi_calloc(sizeof(profile_db_entry_t));
        entry->profile_id = (profile_t)profile_id;
      }
      switch (get_feature(key)) {
        case  AVRCP_0103_SUPPORT:
        {
          memset(&entry->profile_feature_type.avrcp_feature_entry.avrcp_0103_support, '\0', VALUE_MAX_LENGTH);
          memcpy(&entry->profile_feature_type.avrcp_feature_entry.avrcp_0103_support, value, strlen(value));
          LOG_WARN(LOG_TAG, " AVRCP_ID: entry->profile_feature_type.avrcp_feature_entry.avrcp_0103_support %s",
                      entry->profile_feature_type.avrcp_feature_entry.avrcp_0103_support);
        }
        break;
        case AVRCP_COVERART_SUPPORT:
        {
          memset(&entry->profile_feature_type.avrcp_feature_entry.avrcp_coverart_support, '\0', VALUE_MAX_LENGTH);
          memcpy(&entry->profile_feature_type.avrcp_feature_entry.avrcp_coverart_support, value, strlen(value));
          LOG_WARN(LOG_TAG, " AVRCP_ID: &entry->profile_feature_type.avrcp_feature_entry.avrcp_coverart_support %s",
                      entry->profile_feature_type.avrcp_feature_entry.avrcp_coverart_support);
        }
        break;
        default:
        {
          LOG_WARN(LOG_TAG,"%s is invalid key %s", __func__, key);
        }
        break;
      }
      profile_database_add_(entry);
    }
    break;
    case PBAP_ID:
    {
      LOG_WARN(LOG_TAG, " PBAP_ID: key :: %s, value :: %s",
                      key, value);
      entry = profile_entry_fetch(PBAP_ID);
      if (entry == NULL) {
        entry = (profile_db_entry_t *)osi_calloc(sizeof(profile_db_entry_t));
        entry->profile_id = (profile_t)profile_id;
      }
      switch (get_feature(key)) {
        case PBAP_0102_SUPPORT:
        {
          memset(&entry->profile_feature_type.pbap_feature_entry.pbap_0102_support, '\0', VALUE_MAX_LENGTH);
          memcpy(&entry->profile_feature_type.pbap_feature_entry.pbap_0102_support , value, strlen(value));
        }
        break;
        case USE_SIM_SUPPORT:
        {
          memset(&entry->profile_feature_type.pbap_feature_entry.use_sim_support, '\0', VALUE_MAX_LENGTH);
          memcpy(&entry->profile_feature_type.pbap_feature_entry.use_sim_support, value, strlen(value));
        }
        break;
        default:
        {
          LOG_WARN(LOG_TAG,"%s is invalid key %s", __func__, key);
        }
        break;
      }
      profile_database_add_(entry);
    }
    break;
    case MAP_ID:
    {
      LOG_WARN(LOG_TAG, " MAP_ID: key :: %s, value :: %s",
                    key, value);
      entry = profile_entry_fetch(MAP_ID);
      if (entry == NULL) {
        entry = (profile_db_entry_t *)osi_calloc(sizeof(profile_db_entry_t));
        entry->profile_id = (profile_t)profile_id;
      }
      switch (get_feature(key)) {
        case MAP_EMAIL_SUPPORT:
        {
          memset(&entry->profile_feature_type.map_feature_entry.map_email_support, '\0', VALUE_MAX_LENGTH);
          memcpy(&entry->profile_feature_type.map_feature_entry.map_email_support, value, strlen(value));
        }
        break;
        case MAP_0104_SUPPORT:
        {
          memset(&entry->profile_feature_type.map_feature_entry.map_0104_support,
              '\0', VALUE_MAX_LENGTH);
          memcpy(&entry->profile_feature_type.map_feature_entry.map_0104_support,
              value, strlen(value));
        }
        break;
        default:
        {
          LOG_WARN(LOG_TAG,"%s is invalid key %s", __func__, key);
        }
        break;
      }
      profile_database_add_(entry);
    }
    break;
    case MAX_POW_ID:
    {
      LOG_WARN(LOG_TAG, " MAX_POW_ID: key :: %s, value :: %s",
                    key, value);
      entry = profile_entry_fetch(MAX_POW_ID);
      if (entry == NULL) {
        entry = (profile_db_entry_t *)osi_calloc(sizeof(profile_db_entry_t));
        entry->profile_id = (profile_t)profile_id;
      }
      switch (get_feature(key)) {
        case BR_MAX_POW_SUPPORT:
        {
          uint8_t BR_pow;
          char *e;

          if ( strlen(key) != 18 ) {
            LOG_WARN(LOG_TAG,
            " ignoring %s due to invalid key in config file", key);
            return false;
          }

          BR_pow = (uint8_t)strtoul(value, &e, 16);
          LOG_WARN(LOG_TAG, " MAX_POW_ID: BR_pow :: %x",BR_pow);
          entry->profile_feature_type.max_pow_feature_entry.BR_max_pow_feature = true;
          entry->profile_feature_type.max_pow_feature_entry.BR_max_pow_support = BR_pow;
        }
        break;
        case EDR_MAX_POW_SUPPORT:
        {
          uint8_t EDR_pow;
          char *e;

          if ( strlen(key) != 19 ) {
            LOG_WARN(LOG_TAG,
            " ignoring %s due to invalid key in config file", key);
            return false;
          }

          EDR_pow = (uint8_t)strtoul(value, &e, 16);
          LOG_WARN(LOG_TAG, " MAX_POW_ID: EDR_pow :: %x",EDR_pow);
          entry->profile_feature_type.max_pow_feature_entry.EDR_max_pow_feature = true;
          entry->profile_feature_type.max_pow_feature_entry.EDR_max_pow_support = EDR_pow;
        }
        break;
        case BLE_MAX_POW_SUPPORT:
        {
          uint8_t BLE_pow;
          char *e;

          if ( strlen(key) != 19 ) {
            LOG_WARN(LOG_TAG,
            " ignoring %s due to invalid key in config file", key);
            return false;
          }

          BLE_pow = (uint8_t)strtoul(value, &e, 16);
          LOG_WARN(LOG_TAG, " MAX_POW_ID: BLE_pow :: %x",BLE_pow);
          entry->profile_feature_type.max_pow_feature_entry.BLE_max_pow_feature = true;
          entry->profile_feature_type.max_pow_feature_entry.BLE_max_pow_support = BLE_pow;

        }
        break;
        default:
        {
          LOG_WARN(LOG_TAG,"%s is invalid key %s", __func__, key);
        }
        break;
      }
      profile_database_add_(entry);
    }
    break;
    default:
    {
      LOG_WARN(LOG_TAG,"%s is invalid profile entry %s", __func__, key);
    }
    break;
  }
  return true;
}

static void load_config()
{
  if ( profile_config_init() != -1) {
    for (const list_node_t *node = list_begin(config->sections);
       node != list_end(config->sections); node = list_next(node)) {
      int profile_id = -1;
      profile_section_t *sec = (profile_section_t *)list_node(node);
      if ( (profile_id = get_profile(sec->name)) != -1 ) {
        for (const list_node_t *node_entry = list_begin(sec->entries);
           node_entry != list_end(sec->entries);
           node_entry = list_next(node_entry)) {
          profile_entry_t *entry = (profile_entry_t *)list_node(node_entry);
          load_to_database(profile_id, entry->key, entry->value);
        }
      }
    }
  }
  else {
    LOG_ERROR(LOG_TAG, "Error in initializing profile config file");
  }
}

static void profile_config_cleanup(void)
{
  config_free(config);
  config = NULL;
}
