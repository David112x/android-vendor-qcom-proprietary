/***************************************************************************
 * Copyright (c) 2010-2018 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 ***************************************************************************/
#ifndef __EZTUNE_H__
#define __EZTUNE_H__

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdarg.h>
#include <inttypes.h>
#include <sys/types.h>

#include "chromatix.h"
#include "chromatix_common.h"
#include "chromatix_cpp.h"
#include "chromatix_swpostproc.h"
#include "sensor_common.h"
#include "eztune_diagnostics.h"
#include "eztune_vfe_diagnostics.h"

#ifndef Q10
#define Q10 1024
#endif

#define EZTUNE_FORMAT_MAX 128

/* set-prop to make eztune settings sticky between sessions */
#define EZTUNE_PROP_STICKY_SETTINGS "persist.vendor.camera.eztune.sticky"
/* set-prop to store user-defined tune date  */
#define EZTUNE_PROP_TUNE_DATE       "persist.vendor.camera.eztune.tunedate"
/* set-prop to store user-defined tune name  */
#define EZTUNE_PROP_TUNE_NAME       "persist.vendor.camera.eztune.tunename"
/* set-prop to store user-defined tune author  */
#define EZTUNE_PROP_TUNE_AUTHOR     "persist.vendor.camera.eztune.tuneauthor"

typedef enum {
  EZTUNE_FORMAT_JPG = 0,
  EZTUNE_FORMAT_YUV_422,
  EZTUNE_FORMAT_YUV_420,
  EZTUNE_FORMAT_YVU_422,
  EZTUNE_FORMAT_YVU_420,
  EZTUNE_FORMAT_YCrCb_422,
  EZTUNE_FORMAT_YCrCb_420,
  EZTUNE_FORMAT_YCbCr_422,
  EZTUNE_FORMAT_YCbCr_420,
  EZTUNE_FORMAT_INVALID
} eztune_prev_format_t;

typedef enum {
  EZTUNE_FORMAT_BAYER_RGGB = 0,
  EZTUNE_FORMAT_BAYER_GRBG,
  EZTUNE_FORMAT_BAYER_BGGR,
  EZTUNE_FORMAT_BAYER_GBRG,
  EZTUNE_RAW_INVALID
} eztune_raw_format_t;

typedef enum {
  TUNING_SET_RELOAD_CHROMATIX,
  TUNING_SET_RELOAD_AFTUNE,
  TUNING_SET_AUTOFOCUS_TUNING,
  TUNING_SET_VFE_COMMAND,
  TUNING_SET_POSTPROC_COMMAND,
  TUNING_SET_3A_COMMAND,
  TUNING_SET_AEC_LOCK,
  TUNING_SET_AEC_UNLOCK,
  TUNING_SET_AWB_LOCK,
  TUNING_SET_AWB_UNLOCK,
  TUNING_SET_MAX
} tune_set_t;

typedef enum {
  EZTUNE_BUSMSG_SNAP_JPEG = 0, /* do not change value, hard-coded in HAL */
  EZTUNE_BUSMSG_SNAP_RAW  = 1, /* do not change value, hard-coded in HAL */
  EZTUNE_BUSMSG_FLASHMODE,
  EZTUNE_BUSMSG_MAX
} eztune_bus_msg_t;

typedef enum {
  EZTUNE_MISC_GET_VERSION,
  EZTUNE_MISC_APPLY_CHANGES,
  EZTUNE_MISC_WRITE_INI,
  EZTUNE_MISC_READ_INI,
  EZTUNE_MISC_LIST_INI,
} eztune_misc_message_t;

typedef enum {
  EZT_D_FLOAT = 1,
  EZT_D_INT8,
  EZT_D_INT16,
  EZT_D_INT32,
  EZT_D_UINT8,
  EZT_D_UINT16,
  EZT_D_UINT32,
  EZT_D_DOUBLE,
  EZT_D_INVALID
} eztune_item_format_t;

typedef enum {
    EZT_T_CHROMATIX,
    EZT_T_DIGANOSTIC,
    EZT_T_AUTOFOCUS,
    EZTUNE_TYPE_INVALID
} eztune_item_type_t;

typedef enum {
  EZT_WRITE_FLAG = 0,
  EZT_READ_FLAG = 1,
  EZT_ACTION_FLAG = (1<<2),
  EZT_CHROMATIX_FLAG = (1<<3),
  EZT_3A_FLAG = (1<<4),
  EZT_AUTOFOCUS_FLAG = (1<<5),
} eztune_item_reg_flag_t;

typedef enum {
  EZT_SIZE_DIAG,
  EZT_SIZE_ONE
} eztune_item_size;

typedef enum {
  PREV_CHROMATIX_PARMS_TYPE = 0,
  SNAP_CHROMATIX_PARMS_TYPE,
  PREV_CHROMATIX_VFE_COMMON_TYPE,
  SNAP_CHROMATIX_VFE_COMMON_TYPE,
  PREV_CHROMATIX_CPP_TYPE,
  SNAP_CHROMATIX_CPP_TYPE,
  PREV_CHROMATIX_SW_POSTPROC_TYPE,
  SNAP_CHROMATIX_SW_POSTPROC_TYPE,
  PREV_CHROMATIX_3A_PARMS_TYPE,
  SNAP_CHROMATIX_3A_PARMS_TYPE,

  CHROMATIX_MAX
} chromatix_base_type_t;

typedef struct {
  uint8_t* chromatixData;
  uint8_t* snap_chromatixData;
  uint8_t* common_chromatixData;
  uint8_t* cpp_chromatixData;
  uint8_t* snap_cpp_chromatixData;
  uint8_t* postproc_chromatixData;
  uint8_t* aaa_chromatixData;
} eztune_chromatix_t;

typedef struct {
  chromatix_parms_type *chromatixptr;
  chromatix_parms_type *snap_chromatixptr;
  chromatix_VFE_common_type *common_chromatixptr;
  chromatix_cpp_type *cpp_chromatixptr;
  chromatix_cpp_type *snap_cpp_chromatixptr;
  chromatix_sw_postproc_type *swpp_chromatixptr;
  chromatix_3a_parms_type *aaa_chromatixptr;

  actuator_driver_params_t *af_driver_ptr;
  ez_af_tuning_params_t *af_tuning_ptr;

  metadata_buffer_t *metadata;

  void (*tuning_set_vfe)(vfemodule_t module, optype_t optype, int32_t value);
  void (*tuning_set_pp)(pp_module_t module, optype_t optype, int32_t value);
  void (*tuning_set_3a)(aaa_set_optype_t optype, int32_t value);
  void (*tuning_set_focus)(void *eztune_t_ptr, aftuning_optype_t optype, int32_t value);
  void (*tuning_post_bus_msg)(eztune_bus_msg_t optype, int32_t msg);
} eztune_t;

typedef struct {
  int item_num;
  int table_index;
  char value_string[EZTUNE_FORMAT_MAX];
} eztune_set_val_t;

typedef struct {
  int32_t id;                           // enum value (eztune_parms_list_t)
  char name[EZTUNE_FORMAT_MAX];         // text name
  chromatix_base_type_t base_type;      // pointer type to dereference for chromatix
  eztune_item_format_t format;          // format for string conversion
  eztune_item_type_t type;              // chromaxix, diag, af
  eztune_item_reg_flag_t reg_flag;      // read/write
  uint32_t offset;                      // offset into struct
  uint16_t entry_count;                 // for arrays - entry count
  uint32_t step_size;                   // for arrays - ptr increment size
} eztune_item_t;

#endif /* __EZTUNE_H__ */
