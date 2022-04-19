/******************************************************************************
#  Copyright (c) 2013 Qualcomm Technologies, Inc.
#  All Rights Reserved.
#  Confidential and Proprietary - Qualcomm Technologies, Inc.
  ---------------------------------------------------------------------------

  @file    ril_utf_property.c
  @brief   RIL UTF property set and get function defs

  DESCRIPTION
  ---------------------------------------------------------------------------
******************************************************************************/
#include "ril_utf_common_includes.h"
#include "ril_utf_core.h"

struct qcril_test_property_type properties[QCRIL_TEST_MAX_PROPERTIES] = {};

/*===================================================================================
// property_get
//TBD: handle null input values
//TBD: support ERANGE, ULONG, and 0 ret val testing
//TBD: add ability to test return value of prop...get (this is not used anywhere now)
===================================================================================*/
extern "C" int property_get(const char *key, char *value, const char *default_value)
{
   int found = 0;
   int prop_idx=0;
   int prop_tbl_len = sizeof(properties) / sizeof(struct qcril_test_property_type);

   //RIL_UTF_DEBUG("\n QMI_RIL_UTL: property table len: %d", prop_tbl_len);

   //TBD uncomment below after qcril_qmi_sms code has been updated
   if(NULL == value)
     return E_FAILURE;

   *value ='\0'; // We don't now the size of value, so just initializing the first byte.
   while (!found && prop_idx <  prop_tbl_len) {
        if( strcmp( properties[prop_idx].key, key) == 0 ) {
            //RIL_UTF_DEBUG("\n QMI_RIL_UTL: Found property %s", key);
            //RIL_UTF_DEBUG("\n QMI_RIL_UTL: %s value:%s\n",
            //                  properties[prop_idx].key,
            //                  properties[prop_idx].prop_val);
             memset(value, '\0', strlen(properties[prop_idx].prop_val) +1);/* Caller of this function
                   has to allocate atleast (strlen(properties[prop_idx].prop_val) +1) memory for "value".*/
             strlcpy(value, properties[prop_idx].prop_val, strlen(properties[prop_idx].prop_val)+1);
             return E_SUCCESS;
        }
        else
        {
             prop_idx++;
        }
   }

  // If no entry is present and default value is provided we need to set default value
   if ( (default_value == NULL) || (strcmp(default_value, "") == 0) )
   {
     //RIL_UTF_DEBUG("\n QMI_RIL_UTL: Could not find property %s", key);
     return E_FAILURE;
   }
   else
   {
     strlcpy(value, default_value, QCRIL_TEST_PROPERTY_VALUE_LEN);
     return E_SUCCESS;
   }
}

static unsigned int __serial = 0;
static pthread_cond_t serial_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t serial_mutex = PTHREAD_MUTEX_INITIALIZER;
unsigned int __system_property_wait_any(unsigned int serial)
{
    RIL_UTF_DEBUG("QMI_RIL_UTL: Waiting for property update. serial: %u, __serial: %u\n", serial, __serial);
    pthread_mutex_lock(&serial_mutex);
    do {
        pthread_cond_wait(&serial_cond, &serial_mutex);
    } while (__serial == serial);
    RIL_UTF_DEBUG("QMI_RIL_UTL: Property updated. serial: %u, __serial: %u\n", serial, __serial);
    serial = __serial;
    pthread_mutex_unlock(&serial_mutex);
    return serial;
}
/*===================================================================================
// property_set
//TBD: handle null input values
//TBD: support ERANGE, ULONG, and 0 ret val testing
//TBD: add ability to test return value of prop...get (this is not used anywhere now)
===================================================================================*/
extern "C" int property_set(const char *key, const char *value)
{

   int found = 0;
   int prop_idx=0;
   int prop_tbl_len = sizeof(properties) / sizeof(struct qcril_test_property_type);

   while (!found && prop_idx <  prop_tbl_len) {
        if( strcmp( properties[prop_idx].key, "") == 0 ) //If first index is empty, then we have to make an entry in first index.
        {
          break;
        }

        if( strcmp( properties[prop_idx].key, key) == 0 ) {
            RIL_UTF_DEBUG("\n QMI_RIL_UTL: Found property %s", key);
            RIL_UTF_DEBUG("\n QMI_RIL_UTL: updating %s to %s\n",
                              properties[prop_idx].key,
                              value);

             strlcpy(properties[prop_idx].prop_val, value, QCRIL_TEST_PROPERTY_VALUE_LEN);
             pthread_mutex_lock(&serial_mutex);
             __serial ++;
             pthread_cond_signal(&serial_cond);
             pthread_mutex_unlock(&serial_mutex);
             return E_SUCCESS;
        }
        else
        {
             prop_idx++;
        }
   }
   RIL_UTF_DEBUG("\n\nprop_idx %d\n\n", prop_idx);
   if ( prop_idx < QCRIL_TEST_MAX_PROPERTIES )
   {
     memset(properties[prop_idx].key,'\0',QCRIL_TEST_PROPERTY_KEY_LEN);
     memset(properties[prop_idx].prop_val,'\0',QCRIL_TEST_PROPERTY_VALUE_LEN);
     strlcpy(properties[prop_idx].key, key, QCRIL_TEST_PROPERTY_KEY_LEN);
     strlcpy(properties[prop_idx].prop_val, value, QCRIL_TEST_PROPERTY_VALUE_LEN);
     return E_SUCCESS;
   }
   else
   {
     RIL_UTF_DEBUG("\n QMI_RIL_UTL: Could not find property %s", key);
     return E_FAILURE;//fail
   }
}

extern "C" int8_t property_get_bool(const char *key, int8_t default_value) {
    return default_value;
}

/*===================================================================================
// property_init
//
// Resets all properties to pre boot state
===================================================================================*/
int utf_property_init()
{
   int i = 0;

   for(i=0; i<QCRIL_TEST_MAX_PROPERTIES; i++)
   {
      memset(properties[i].key,'\0',QCRIL_TEST_PROPERTY_KEY_LEN);
      memset(properties[i].prop_val,'\0',QCRIL_TEST_PROPERTY_VALUE_LEN);
   }

   property_set("persist.vendor.radio.adb_log_on", "0");
   property_set("persist.vendor.radio.eons.enabled", "true");
   property_set("ro.product.board", "MSM8660_SURF");
   property_set("ro.product.brand", "qti");
   property_set("ro.product.cpu.abi", "armeabi-v7a");
   property_set("ro.product.cpu.abi2", "armeabi");
   property_set("ro.product.device", "msm8660_surf");
   property_set("ro.product.locale.language", "en");
   property_set("ro.product.locale.region", "US");
   property_set("ro.product.manufacturer", "unknown");
   property_set("ro.product.model", "msm8660_surf");
   property_set("ro.product.name", "msm8660_surf");
   property_set("ro.qti.bluetooth.ftp", "true");
   property_set("ro.qti.cabl", "1");
   property_set("ro.ril.svlte1x", "false");
   property_set("ro.ril.svdo", "false");
   property_set("ro.baseband", "SVLTE1");
   property_set("persist.vendor.radio.sib16_support", "1");
   property_set("persist.vendor.radio.hw_mbn_update", "0");
   property_set("persist.vendor.radio.sw_mbn_update", "0");
   property_set("persist.vendor.radio.hidl_dev_service", "1");

  return E_SUCCESS;
}

/*===================================================================================
// print_all_properties
//
// print all currently defined properties
===================================================================================*/
void print_all_properties()
{
   int prop_idx=0;
   int prop_tbl_len = sizeof(properties) / sizeof(struct qcril_test_property_type);

   for (prop_idx = 0; prop_idx < prop_tbl_len; ++prop_idx)
   {
      if (strcmp( properties[prop_idx].key, "") != 0)
      {
        RIL_UTF_ESSENTIAL("\n QMI_RIL_UTF: Found property %s with value %s", properties[prop_idx].key, properties[prop_idx].prop_val);
      }
   }
}
