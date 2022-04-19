/* eeprom_bin.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __EEPROM_BIN_H__
#define __EEPROM_BIN_H__

#include <linux/types.h>
#include <media/msm_cam_sensor.h>
#include "eebin_interface.h"
#include "eebin_lib.h"

typedef struct {
  uint32_t version;
  uint32_t (*parse)(bin_device_t *bin_device, char *buff, uint32_t size);
}eebin_parse_t;

typedef struct {
  int32_t         ready;
  bin_device_t    bin_device;
  uint32_t        version;
  uint32_t        version_crc;
  char            name[MAX_MODULE_NAME];
  char            lib_name[MAX_MODULE_NAME];
  char            path[PATH_SIZE_255];
  void           *eebin_parselib_hdl;
  void           *eebin_parselib_ptr;
  char           *out_buff;
  uint32_t        out_buff_size;
}eebin_hdl_t;

int32_t eebin_name_data(eebin_hdl_t *eebin_hdl,
  eebin_query_name_t *name_data);

int32_t eebin_dev_data(eebin_hdl_t *eebin_hdl,
  eebin_query_dev_t *dev_data);

int32_t eebin_num_dev(eebin_hdl_t *eebin_hdl,
  eebin_lib_t type, uint32_t *num);

int32_t eebin_get_bin_data(eebin_hdl_t *eebin_hdl);

int32_t eebin_lib_name_data(eebin_hdl_t *eebin_hdl,
  eebin_query_name_t *name_data);

#endif

