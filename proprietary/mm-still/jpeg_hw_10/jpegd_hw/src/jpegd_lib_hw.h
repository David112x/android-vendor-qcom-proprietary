/*============================================================================
   Copyright (c) 2012-2015 Qualcomm Technologies, Inc.  All Rights Reserved.
   Qualcomm Technologies Proprietary and Confidential.
============================================================================*/

#ifndef JPEGD_LIB_HW_H
#define JPEGD_LIB_HW_H

#include <unistd.h>
#include <media/msm_jpeg.h>
#include "jpegd_lib.h"
#include "jpegd_hw_reg.h"
#include "jpegd_hw_reg_ctrl.h"
#include "jpegd_dbg.h"
#include "jpegd.h"

#define JPEG_8974_V1 0x10000000
#define JPEG_8974_V2 0x10010000
#define JPEG_8974_PRO 0x10010001
#define JPEG_8994 0x10020000

/**
  * Macro to get major number of HW version
  *
  */
#define GET_MAJOR(v) ((v) >> 28)

/**
  * Macro to get minor number of HW version
  *
  */
#define GET_MINOR(v) (((v) >> 16) & 0x00000FFF)

#define IS_8974_V2(v) ((v) != JPEG_8974_V1)

/**
  * Macro to check if the HW version is for 8994
  */
#define IS_8994(v) ((v) == JPEG_8994)

/**
  * Macro to check if the HW version is 8994 or greater
  *
  */
#define IS_8994_OR_GREATER(v) \
  ((GET_MAJOR(v) > GET_MAJOR(JPEG_8994)) \
  || ((GET_MAJOR(v) == GET_MAJOR(JPEG_8994)) \
  && (GET_MINOR(v) >= GET_MINOR(JPEG_8994))))

#define CEILING16(X) (((X) + 0x000F) & 0xFFF0)
#define CEILING8(X)  (((X) + 0x0007) & 0xFFF8)
#define CEILING2(X)  (((X) + 0x0001) & 0xFFFE)


/*============================================================================
   FUNCTIONS
============================================================================*/
int jpegd_hw_reset(int);
//
int jpegd_hw_core_cfg(int, int);
//
int jpegd_hw_fe_cfg(int, jpegd_cmd_fe_cfg *, uint32_t);
//
int jpegd_hw_input_len_cfg(int, uint32_t);
//
int jpegd_hw_we_cfg(int, jpegd_cmd_we_cfg *,
  jpegd_image_info_t *, jpegd_scale_type_t, uint32_t);
//
int jpegd_hw_decode_cfg(int, uint32_t, uint32_t, uint32_t,
  jpegd_subsampling_t, uint32_t);
//
int jpegd_hw_dht_code_config(int, jpegd_cmd_huff_cfg_t*);
//
int jpegd_hw_huff_config(int, jpegd_cmd_huff_cfg_t*);
//
int jpegd_hw_jpeg_dqt(int, jpegd_cmd_quant_cfg_t*);
//
int jpegd_hw_decode(int);
//
int jpegd_hw_scaling_config(int, int);
//
int jpegd_hw_scale_core_cfg(int);
//
void jpegd_lib_hw_get_version (struct msm_jpeg_hw_cmd *p_hw_cmd);

#endif // JPEGD_LIB_HW_H
