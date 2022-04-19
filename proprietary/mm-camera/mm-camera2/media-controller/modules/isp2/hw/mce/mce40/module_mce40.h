/* module_mce40.h
 *
 * Copyright (c) 2012-2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#ifndef __MODULE_MCE40_H__
#define __MODULE_MCE40_H__

/* std headers */
#include "pthread.h"

/* hal headers */
#include "cam_types.h"

/* isp headers */
#include "isp_common.h"
#include "isp_defs.h"
#include "mce_reg.h"
#include "chromatix.h"

/** mce40_t
 *  @mce_mix_cmd_2:
 *  @mce_cmd: register command for hardware settings
 *  @applied_mce_cmd: save applied settings for vfe diag (eztune)
 *  @prev_lux_idx:
 *  @cnt:
 **/
typedef struct{
  ISP_MCE_MIX_ConfigCmdType_2       mce_mix_cmd_2;
  ISP_MCE_ConfigCmdType             mce_cmd;
  ISP_MCE_ConfigCmdType             applied_mce_cmd;

  float                             prev_lux_idx;
  int                               cnt;
  aec_update_t                      aec_update;
  asd_update_t                      asd_update;
  cam_scene_mode_type               bestshot_mode;
} mce40_t;
#endif
