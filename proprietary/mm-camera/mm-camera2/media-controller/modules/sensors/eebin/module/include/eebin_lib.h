/* eebin_lib.h
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __EEBIN_LIB_H__
#define __EEBIN_LIB_H__
#include <linux/types.h>
#include "eebin_dev.h"

typedef int32_t (*write_lib_file_f)(char*,char*,char*,uint32_t);
typedef int32_t (*eebin_parse_data_f)(bin_device_t *,
                                       char *, int,write_lib_file_f);

typedef struct {
  uint32_t version_num;
  uint32_t version_crc;
  char *name;
  eebin_parse_data_f parse_f;
} eebinparse_lib;
#endif
